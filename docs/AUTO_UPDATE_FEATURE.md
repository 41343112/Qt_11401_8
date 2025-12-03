# 自動更新功能

## 概述

Qt_Chess 現在支援自動檢查 GitHub Releases 上的更新。當有新版本可用時，應用程式會通知使用者並提供下載連結。

## 功能特點

### 自動檢查
- 應用程式啟動後 2 秒自動檢查更新
- 不會影響啟動動畫或使用者體驗
- 自動檢查失敗時不會打擾使用者

### 手動檢查
- 透過「幫助」→「檢查更新」選單手動檢查
- 顯示檢查進度對話框
- 如果已是最新版本，會顯示確認訊息

### 更新通知
當發現新版本時：
- 顯示版本號
- 顯示更新說明（最多 300 字元）
- 提供「是/否」選項
- 選擇「是」會在瀏覽器中打開下載頁面

## 技術實現

### UpdateChecker 類別
位於 `src/updatechecker.h` 和 `src/updatechecker.cpp`

**主要方法：**
- `checkForUpdates()` - 從 GitHub API 檢查最新版本
- `startDownload()` - 開始下載更新檔案（備用功能）
- `compareVersions()` - 比較版本號（支援語義版本控制）

**信號：**
- `updateAvailable(version, downloadUrl, releaseNotes)` - 發現新版本
- `noUpdateAvailable()` - 已是最新版本
- `checkFailed(error)` - 檢查失敗

### 版本管理
版本資訊集中管理於 `src/version.h`：
```cpp
#define APP_VERSION "1.0.0"
#define GITHUB_OWNER "41343112"
#define GITHUB_REPO "Qt_Chess"
```

發布新版本時，只需要：
1. 更新 `src/version.h` 中的版本號
2. 在 GitHub 上創建對應的 Release

### API 端點
使用 GitHub REST API v3：
```
https://api.github.com/repos/{owner}/{repo}/releases/latest
```

## 版本比較邏輯

支援標準語義版本格式（x.y.z）：
- 比較主版本號、次版本號和修訂號
- 支援帶 'v' 前綴的標籤（例如：v1.0.0）
- 版本號不足三個部分時自動補 0

## 平台支援

- ✅ Windows
- ✅ macOS
- ✅ Linux

自動檢測當前平台並優先選擇對應的下載資產。

## 依賴項

- Qt5 Network 模組（已加入 Qt_Chess.pro）
- 網路連線（用於 API 請求）

## 未來改進

可考慮的增強功能：
1. 應用程式內下載更新
2. 自動安裝更新（需要提升權限）
3. 更新進度條
4. 差異更新（僅下載變更的檔案）
5. 更新歷史紀錄
6. 設定更新檢查頻率

## 測試

要測試更新功能：
1. 在 GitHub 上創建一個新的 Release（版本號大於 1.0.0）
2. 執行應用程式
3. 等待 2 秒或手動點擊「檢查更新」
4. 應該會顯示更新通知

## 注意事項

- 更新檢查需要網路連線
- GitHub API 有速率限制（未認證：60 次/小時）
- 失敗的檢查不會影響應用程式運行
- 使用者可以選擇忽略更新通知
