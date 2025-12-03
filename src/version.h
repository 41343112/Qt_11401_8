#ifndef VERSION_H
#define VERSION_H

#include <QString>

// 應用程式版本資訊
// 在發布新版本時更新此檔案
#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0

#define APP_VERSION "1.0.0"
#define APP_NAME "Qt_Chess"

// GitHub 儲存庫資訊
#define GITHUB_OWNER "41343112"
#define GITHUB_REPO "Qt_Chess"

// GitHub API URL 構造函數
inline QString getGitHubApiReleasesUrl() {
    return QString("https://api.github.com/repos/%1/%2/releases/latest")
        .arg(GITHUB_OWNER)
        .arg(GITHUB_REPO);
}

#define GITHUB_API_RELEASES_URL getGitHubApiReleasesUrl().toUtf8().constData()

#endif // VERSION_H
