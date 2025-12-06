# 設定面板整合實作說明

## 概述

本次更新將原本以對話框（Dialog）方式顯示的設定功能改為直接在主視窗中以面板形式顯示，提供更流暢的使用者體驗。

## 問題描述

**原問題**：當點擊設定時不是小視窗，直接一個畫面

**解決方案**：將三個設定對話框（音效、棋子圖標、棋盤顏色）整合為統一的設定面板，在主視窗中切換顯示。

## 主要變更

### 1. 架構變更

#### qt_chess.h
- 添加 `QStackedWidget`、`QTabWidget` 和 `QScrollArea` 相關的包含檔案
- 新增成員變量：
  - `m_stackedWidget`: 用於在主遊戲視圖和設定視圖之間切換
  - `m_mainGameView`: 主遊戲視圖容器
  - `m_settingsView`: 設定視圖容器
  - `m_settingsTabWidget`: 設定標籤頁控件
  - `m_soundSettingsWidget`: 音效設定 widget
  - `m_pieceIconSettingsWidget`: 棋子圖標設定 widget
  - `m_boardColorSettingsWidget`: 棋盤顏色設定 widget
  - `m_backFromSettingsButton`: 返回按鈕

- 新增成員函數：
  - `setupSettingsPanel()`: 設置設定面板
  - `showSettingsPanel(int tabIndex)`: 顯示設定面板並切換到指定標籤
  - `hideSettingsPanel()`: 隱藏設定面板，返回主遊戲視圖
  - `createSoundSettingsWidget()`: 創建音效設定 widget
  - `createPieceIconSettingsWidget()`: 創建棋子圖標設定 widget
  - `createBoardColorSettingsWidget()`: 創建棋盤顏色設定 widget

#### qt_chess.cpp

**UI 結構變更**：
- 修改 `setupUI()` 函數，使用 `QStackedWidget` 作為中央 widget
- 將原有的遊戲 UI 放入 `m_mainGameView` 頁面
- 在 `setupUI()` 末尾調用 `setupSettingsPanel()` 創建設定視圖

**設定面板實現**：
```cpp
void Qt_Chess::setupSettingsPanel() {
    // 創建設定視圖，包含標題、返回按鈕和標籤頁
    m_settingsView = new QWidget(m_stackedWidget);
    
    // 創建標籤頁 widget，包含三個設定標籤
    m_settingsTabWidget = new QTabWidget(m_settingsView);
    
    // 創建各個設定 widget 並添加到標籤頁
    m_settingsTabWidget->addTab(createSoundSettingsWidget(), "🔊 音效設定");
    m_settingsTabWidget->addTab(createPieceIconSettingsWidget(), "♟️ 棋子圖標");
    m_settingsTabWidget->addTab(createBoardColorSettingsWidget(), "🎨 棋盤顏色");
}
```

**設定 Widget 特點**：

1. **音效設定 Widget** (`createSoundSettingsWidget()`):
   - 使用滾動區域容納所有控件
   - 提供全部音效總開關
   - 每個音效類型都有獨立的控制組：
     - 啟用/停用複選框
     - 檔案路徑顯示和瀏覽按鈕
     - 音量滑桿
     - 預覽和重設按鈕
   - 設定更改立即生效（即時應用）
   - 提供重設所有音效為預設值按鈕

2. **棋子圖標設定 Widget** (`createPieceIconSettingsWidget()`):
   - 棋子大小縮放滑桿（60%-100%）
   - 圖標集選擇（Unicode 或自訂）
   - 簡化的界面，聚焦於主要功能
   - 設定更改立即應用到棋盤
   - 提供重設為預設值按鈕

3. **棋盤顏色設定 Widget** (`createBoardColorSettingsWidget()`):
   - 2×2 預覽網格實時顯示顏色變化
   - 七種預設配色方案快速選擇
   - 自訂淺色和深色方格顏色
   - 顏色選擇使用 QColorDialog
   - 設定更改立即應用到棋盤
   - 提供重設為預設值按鈕

**菜單處理變更**：
```cpp
void Qt_Chess::onSoundSettingsClicked() {
    showSettingsPanel(0);  // 顯示音效設定標籤
}

void Qt_Chess::onPieceIconSettingsClicked() {
    showSettingsPanel(1);  // 顯示棋子圖標設定標籤
}

void Qt_Chess::onBoardColorSettingsClicked() {
    showSettingsPanel(2);  // 顯示棋盤顏色設定標籤
}
```

**鍵盤快捷鍵**：
- 添加 ESC 鍵支持：在設定面板時按 ESC 鍵返回主遊戲視圖
- 修改 `keyPressEvent()` 函數，優先檢查是否在設定視圖

### 2. 樣式設計

所有設定面板使用現代科技風格（Modern Tech Style），與主遊戲 UI 保持一致：
- 霓虹青色 (THEME_ACCENT_PRIMARY: #00D9FF) 作為主要強調色
- 深色背景 (THEME_BG_DARK, THEME_BG_PANEL)
- 圓角按鈕和組件
- 漸變效果和懸停動畫
- 統一的字體大小和間距

### 3. 用戶體驗改進

**即時更新**：
- 所有設定更改立即應用到遊戲，無需點擊確定或應用按鈕
- 音量滑桿、顏色選擇等操作立即生效

**返回機制**：
- 左上角的「← 返回遊戲」按鈕
- ESC 鍵快捷鍵
- 無需保存操作，設定自動持久化

**視覺反饋**：
- 棋盤顏色設定提供 2×2 預覽網格
- 音效設定提供預覽按鈕
- 所有按鈕提供懸停效果

## 技術細節

### 堆疊 Widget 切換
```cpp
void Qt_Chess::showSettingsPanel(int tabIndex) {
    if (tabIndex >= 0 && tabIndex < m_settingsTabWidget->count()) {
        m_settingsTabWidget->setCurrentIndex(tabIndex);
    }
    m_stackedWidget->setCurrentWidget(m_settingsView);
}

void Qt_Chess::hideSettingsPanel() {
    m_stackedWidget->setCurrentWidget(m_mainGameView);
}
```

### 設定持久化
- 所有設定更改通過現有的 `applySoundSettings()`、`applyPieceIconSettings()` 和 `applyBoardColorSettings()` 函數應用
- 這些函數內部會調用 QSettings 保存設定到持久化存儲

### 記憶體管理
- 所有 widget 由父 widget 管理生命週期
- 使用智能指針避免記憶體洩漏
- 重設功能會重新創建 widget 以確保 UI 狀態正確

## 測試建議

1. **功能測試**：
   - 測試從菜單打開各個設定標籤
   - 驗證設定更改立即應用到遊戲
   - 測試返回按鈕和 ESC 鍵
   - 驗證設定在程式重啟後保持

2. **UI 測試**：
   - 檢查所有控件在不同視窗大小下正常顯示
   - 驗證滾動區域在內容過多時正常工作
   - 檢查顏色預覽正確更新

3. **邊界測試**：
   - 測試音量滑桿的最小和最大值
   - 測試棋子大小縮放的邊界值
   - 測試顏色選擇對話框取消操作

## 向後兼容性

- 保留原有的設定對話框類（SoundSettingsDialog、PieceIconSettingsDialog、BoardColorSettingsDialog）
- 保留所有設定相關的數據結構和函數
- 僅修改 UI 呈現方式，不影響設定的存儲和讀取邏輯

## 未來改進建議

1. **動畫效果**：添加面板切換的淡入淡出動畫
2. **搜索功能**：在設定面板添加搜索框快速找到設定項
3. **快捷鍵顯示**：在設定面板顯示相關的鍵盤快捷鍵
4. **設定匯入/匯出**：允許用戶匯出和匯入設定配置

## 結論

此次更新成功地將對話框式設定改為嵌入式面板設定，提供了更現代、更流暢的用戶體驗。所有設定功能保持完整，並且通過即時更新和視覺反饋提升了可用性。
