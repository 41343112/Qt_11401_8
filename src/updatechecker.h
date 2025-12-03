#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVersionNumber>

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    // 檢查更新
    void checkForUpdates();
    
    // 取得當前版本
    static QString getCurrentVersion();
    
    // 取得最新版本資訊
    QString getLatestVersion() const { return m_latestVersion; }
    QString getReleaseUrl() const { return m_releaseUrl; }
    QString getReleaseNotes() const { return m_releaseNotes; }

signals:
    // 當檢查完成時發出信號
    void updateCheckFinished(bool updateAvailable);
    
    // 當檢查失敗時發出信號
    void updateCheckFailed(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
    QString m_latestVersion;
    QString m_releaseUrl;
    QString m_releaseNotes;
    
    bool compareVersions(const QString& version1, const QString& version2);
};

#endif // UPDATECHECKER_H
