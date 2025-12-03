#include "updatechecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

// 當前版本號
#define APP_VERSION "1.0.0"

// GitHub repository 資訊
#define GITHUB_REPO_OWNER "41343112"
#define GITHUB_REPO_NAME "Qt_Chess"

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onReplyFinished);
}

UpdateChecker::~UpdateChecker()
{
}

QString UpdateChecker::getCurrentVersion()
{
    return APP_VERSION;
}

void UpdateChecker::checkForUpdates()
{
    // GitHub API URL for latest release
    QString apiUrl = QString("https://api.github.com/repos/%1/%2/releases/latest")
                        .arg(GITHUB_REPO_OWNER, GITHUB_REPO_NAME);
    QUrl url(apiUrl);
    QNetworkRequest request(url);
    
    // 設定 User-Agent header (GitHub API 需要，包含版本號以便除錯)
    QString userAgent = QString("Qt_Chess/%1").arg(APP_VERSION);
    request.setRawHeader("User-Agent", userAgent.toUtf8());
    
    m_networkManager->get(request);
}

void UpdateChecker::onReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        if (reply->error() == QNetworkReply::ContentNotFoundError) {
            errorMsg = "沒有找到可用的發行版本";
        } else {
            errorMsg = QString("網路錯誤: %1").arg(reply->errorString());
        }
        emit updateCheckFailed(errorMsg);
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (!doc.isObject()) {
        emit updateCheckFailed("無法解析伺服器回應");
        return;
    }
    
    QJsonObject obj = doc.object();
    m_latestVersion = obj["tag_name"].toString();
    
    // 移除版本號前的 "v" 字首（如果存在）
    if (m_latestVersion.startsWith("v") || m_latestVersion.startsWith("V")) {
        m_latestVersion = m_latestVersion.mid(1);
    }
    
    m_releaseUrl = obj["html_url"].toString();
    m_releaseNotes = obj["body"].toString();
    
    // 比較版本
    QString currentVersion = getCurrentVersion();
    bool updateAvailable = compareVersions(m_latestVersion, currentVersion);
    
    emit updateCheckFinished(updateAvailable);
}

bool UpdateChecker::compareVersions(const QString& version1, const QString& version2)
{
    // 使用 QVersionNumber 進行版本比較
    QVersionNumber v1 = QVersionNumber::fromString(version1);
    QVersionNumber v2 = QVersionNumber::fromString(version2);
    
    return QVersionNumber::compare(v1, v2) > 0;
}
