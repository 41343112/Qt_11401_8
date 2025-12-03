# 自動更新功能

## 功能概述

Qt_Chess 現在支援自動更新檢查功能，能夠：

1. **自動檢查更新**：應用程式啟動後自動檢查 GitHub 是否有新版本
2. **手動檢查更新**：使用者可以透過選單手動檢查更新
3. **版本比較**：智慧比較版本號，確保檢測到最新版本
4. **一鍵下載**：發現新版本時，可以直接開啟瀏覽器前往下載頁面

## 使用方式

### 自動檢查更新

應用程式啟動後，會在 3 秒後自動檢查 GitHub 上的最新版本。如果有新版本可用：

- 會顯示更新通知對話框
- 顯示目前版本和最新版本
- 顯示更新說明（如果有）
- 提供「前往下載」和「稍後再說」兩個選項

如果已是最新版本，則不會顯示任何訊息（避免打擾使用者）。

### 手動檢查更新

1. 點擊選單列的「❓ 說明」
2. 選擇「🔄 檢查更新」
3. 系統會顯示「正在檢查更新...」訊息框
4. 檢查完成後：
   - 如果有新版本：顯示更新通知對話框
   - 如果已是最新版本：顯示「已是最新版本」訊息
   - 如果檢查失敗：顯示錯誤訊息

## 技術細節

### 版本檢查機制

- 使用 GitHub REST API (`https://api.github.com/repos/41343112/Qt_Chess/releases/latest`)
- 透過 Qt Network 模組進行非同步網路請求
- 使用 `QVersionNumber` 進行版本比較

### 版本號格式

當前應用程式版本號定義在 `src/updatechecker.cpp` 中：

```cpp
#define APP_VERSION "1.0.0"
```

版本號遵循語義化版本規範（Semantic Versioning）：`主版本號.次版本號.修訂號`

### UpdateChecker 類別

`UpdateChecker` 類別負責處理所有更新檢查邏輯：

- `checkForUpdates()`：發起更新檢查
- `getCurrentVersion()`：取得當前應用程式版本
- `getLatestVersion()`：取得最新版本號
- `getReleaseUrl()`：取得發行頁面 URL
- `getReleaseNotes()`：取得更新說明

### 信號與槽

UpdateChecker 使用 Qt 的信號槽機制：

- `updateCheckFinished(bool updateAvailable)`：檢查完成時發出
- `updateCheckFailed(const QString& error)`：檢查失敗時發出

## 開發者指南

### 更新版本號

發布新版本時，需要更新以下位置的版本號：

1. `src/updatechecker.cpp` 中的 `APP_VERSION` 定義
2. 在 GitHub 建立對應的 Release Tag（例如：`v1.0.0`）

### 測試更新功能

1. 修改 `APP_VERSION` 為較舊的版本號（例如 `0.9.0`）
2. 重新編譯應用程式
3. 執行應用程式，應該會收到更新通知

### 自訂更新行為

如果需要自訂更新檢查行為，可以修改 `Qt_Chess::Qt_Chess()` 建構函式中的定時器設定：

```cpp
// 調整延遲時間（毫秒）
QTimer::singleShot(3000, this, [this]() {
    m_updateChecker->checkForUpdates();
});
```

## 注意事項

1. **網路連線**：更新檢查需要網路連線，如果無法連線會靜默失敗（自動檢查時）
2. **GitHub API 限制**：GitHub API 對未認證請求有速率限制（每小時 60 次）
3. **隱私**：更新檢查僅向 GitHub 發送 API 請求，不會收集任何使用者資料

## 未來改進

可能的功能增強：

- [ ] 增加「不再提醒此版本」選項
- [ ] 支援自動下載和安裝（需要額外權限處理）
- [ ] 顯示完整的更新日誌
- [ ] 支援測試版本的檢查與下載
- [ ] 新增更新檢查頻率設定（每日/每週/手動）
