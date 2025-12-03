#include "updatechecker.h"
#include "version.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_currentVersion(APP_VERSION)
{
}

UpdateChecker::~UpdateChecker()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

QString UpdateChecker::getCurrentVersion() const
{
    return m_currentVersion;
}

QString UpdateChecker::getLatestVersion() const
{
    return m_latestVersion;
}

QString UpdateChecker::getDownloadUrl() const
{
    return m_downloadUrl;
}

bool UpdateChecker::isUpdateAvailable() const
{
    if (m_latestVersion.isEmpty()) {
        return false;
    }
    return compareVersions(m_currentVersion, m_latestVersion);
}

void UpdateChecker::checkForUpdates()
{
    // 如果已有進行中的請求，取消它
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl(GITHUB_API_RELEASES_URL));
    request.setHeader(QNetworkRequest::UserAgentHeader, APP_NAME);
    
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UpdateChecker::onCheckReplyFinished);
}

void UpdateChecker::onCheckReplyFinished()
{
    if (!m_currentReply) {
        return;
    }

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_currentReply->errorString();
        qDebug() << "Update check failed:" << errorMsg;
        emit checkFailed(errorMsg);
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    QByteArray responseData = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        emit checkFailed(tr("無法解析 GitHub API 回應"));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    
    // 取得版本標籤
    QString tagName = jsonObj["tag_name"].toString();
    if (tagName.isEmpty()) {
        emit checkFailed(tr("無法取得版本標籤"));
        return;
    }

    // 從標籤中提取版本號
    m_latestVersion = extractVersionFromTag(tagName);
    
    // 取得發布說明
    m_releaseNotes = jsonObj["body"].toString();
    
    // 取得下載連結（尋找第一個資產檔案）
    QJsonArray assets = jsonObj["assets"].toArray();
    m_downloadUrl.clear();
    
    if (!assets.isEmpty()) {
        // 優先尋找適合當前平台的資產
        QString platform;
#ifdef Q_OS_WIN
        platform = "windows";
#elif defined(Q_OS_MAC)
        platform = "macos";
#elif defined(Q_OS_LINUX)
        platform = "linux";
#endif
        
        for (const QJsonValue& asset : assets) {
            QJsonObject assetObj = asset.toObject();
            QString name = assetObj["name"].toString().toLower();
            QString downloadUrl = assetObj["browser_download_url"].toString();
            
            if (!downloadUrl.isEmpty() && name.contains(platform)) {
                m_downloadUrl = downloadUrl;
                break;
            }
        }
        
        // 如果沒有找到平台特定的資產，使用第一個
        if (m_downloadUrl.isEmpty()) {
            QJsonObject firstAsset = assets[0].toObject();
            m_downloadUrl = firstAsset["browser_download_url"].toString();
        }
    }
    
    // 如果沒有資產，使用發布頁面的 HTML URL
    if (m_downloadUrl.isEmpty()) {
        m_downloadUrl = jsonObj["html_url"].toString();
    }

    // 比較版本
    if (compareVersions(m_currentVersion, m_latestVersion)) {
        qDebug() << "Update available:" << m_latestVersion << "Current:" << m_currentVersion;
        emit updateAvailable(m_latestVersion, m_downloadUrl, m_releaseNotes);
    } else {
        qDebug() << "No update available. Current:" << m_currentVersion << "Latest:" << m_latestVersion;
        emit noUpdateAvailable();
    }
}

void UpdateChecker::startDownload()
{
    if (m_downloadUrl.isEmpty()) {
        emit downloadFailed(tr("沒有可用的下載連結"));
        return;
    }

    // 如果已有進行中的請求，取消它
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(m_downloadUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, APP_NAME);
    
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &UpdateChecker::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::finished, this, &UpdateChecker::onDownloadFinished);
}

void UpdateChecker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateChecker::onDownloadFinished()
{
    if (!m_currentReply) {
        return;
    }

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_currentReply->errorString();
        qDebug() << "Download failed:" << errorMsg;
        emit downloadFailed(errorMsg);
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    // 儲存下載的檔案
    QByteArray data = m_currentReply->readAll();
    QString fileName = QUrl(m_downloadUrl).fileName();
    if (fileName.isEmpty()) {
        fileName = "Qt_Chess_update";
    }
    
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir downloadDir(downloadPath);
    QString filePath = downloadDir.absoluteFilePath(fileName);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit downloadFailed(tr("無法儲存檔案: ") + file.errorString());
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }
    
    file.write(data);
    file.close();
    
    qDebug() << "Download completed:" << filePath;
    emit downloadFinished(filePath);
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

bool UpdateChecker::compareVersions(const QString& version1, const QString& version2) const
{
    // 解析版本號 (格式: x.y.z)
    QStringList v1Parts = version1.split('.');
    QStringList v2Parts = version2.split('.');
    
    // 確保版本號有三個部分
    while (v1Parts.size() < 3) v1Parts.append("0");
    while (v2Parts.size() < 3) v2Parts.append("0");
    
    // 比較每個部分
    for (int i = 0; i < 3; i++) {
        int num1 = v1Parts[i].toInt();
        int num2 = v2Parts[i].toInt();
        
        if (num2 > num1) {
            return true;  // version2 較新
        } else if (num2 < num1) {
            return false; // version1 較新或相同
        }
    }
    
    return false; // 版本相同
}

QString UpdateChecker::extractVersionFromTag(const QString& tag) const
{
    // 從標籤中提取版本號 (例如: "v1.0.0" -> "1.0.0")
    QString version = tag;
    QRegularExpression re("^v?([0-9]+\\.[0-9]+\\.[0-9]+)");
    QRegularExpressionMatch match = re.match(tag);
    
    if (match.hasMatch()) {
        version = match.captured(1);
    }
    
    return version;
}
