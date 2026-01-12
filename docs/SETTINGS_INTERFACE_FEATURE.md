# 設定界面功能 (Settings Interface Feature)

## 概述 (Overview)

此功能將設定從對話框模式轉換為全屏界面面板，與主選單的設計風格保持一致。這提供了更流暢的使用者體驗和更一致的視覺設計。

This feature converts the settings from a dialog-based approach to a full-screen interface panel, maintaining consistency with the main menu design. This provides a smoother user experience and more consistent visual design.

## 功能特色 (Features)

### 主要改進 (Main Improvements)

1. **全屏設定面板**
   - 不再使用彈出式對話框
   - 採用與主選單相同的全屏界面設計
   - 保持歐式古典風格主題

2. **流暢的導航**
   - 點擊主選單的「⚙️ 設定」按鈕進入設定面板
   - 設定面板提供「← 返回主選單」按鈕
   - 所有三個設定選項都可直接訪問

3. **一致的設計語言**
   - 使用與主選單相同的按鈕樣式
   - 保持相同的顏色主題和字體
   - 響應式佈局適應不同視窗大小

## 使用方式 (Usage)

### 進入設定面板 (Entering Settings Panel)

1. 從主選單點擊「⚙️ 設定」按鈕
2. 設定面板將以全屏方式顯示

### 可用的設定選項 (Available Settings Options)

設定面板提供三個主要設定選項：

1. **🔊 音效設定**
   - 調整遊戲音效
   - 設定音量
   - 啟用/停用音效

2. **♟ 棋子圖標設定**
   - 選擇棋子圖標集
   - 調整棋子大小
   - 自訂棋子圖片

3. **🎨 棋盤顏色設定**
   - 選擇棋盤配色方案
   - 自訂顏色
   - 預覽棋盤外觀

### 返回主選單 (Returning to Main Menu)

點擊設定面板底部的「← 返回主選單」按鈕即可返回主選單。

## 技術實現 (Technical Implementation)

### 架構變更 (Architecture Changes)

1. **新增 UI 元件**
   ```cpp
   QWidget* m_settingsWidget;           // 設定面板容器
   QPushButton* m_settingsSoundButton;  // 音效設定按鈕
   QPushButton* m_settingsPieceIconButton; // 棋子圖標設定按鈕
   QPushButton* m_settingsBoardColorButton; // 棋盤顏色設定按鈕
   QPushButton* m_settingsBackButton;   // 返回主選單按鈕
   ```

2. **新增函數**
   - `setupSettingsPanel()`: 初始化設定面板 UI
   - `showSettingsPanel()`: 顯示設定面板並隱藏其他面板

3. **導航邏輯**
   - `showMainMenu()`: 更新以隱藏設定面板
   - `showGameContent()`: 更新以隱藏設定面板
   - `onMainMenuSettingsClicked()`: 簡化為調用 `showSettingsPanel()`

### 樣式設計 (Styling)

設定面板使用與主選單相同的歐式古典風格主題：
- 背景色：`THEME_BG_PANEL`
- 文字色：`THEME_TEXT_PRIMARY`
- 邊框色：`THEME_BORDER`
- 強調色：`THEME_ACCENT_PRIMARY`

## 向後兼容性 (Backward Compatibility)

- 所有現有的設定對話框功能保持不變
- 設定選項的實際功能沒有變更
- 只是改變了訪問設定的方式

## 未來改進 (Future Enhancements)

可能的改進方向：

1. **直接編輯設定**
   - 在設定面板上直接顯示設定控制項
   - 減少需要打開的對話框數量

2. **設定預覽**
   - 在設定面板上顯示當前設定的預覽
   - 例如顯示當前棋盤顏色或棋子圖標

3. **設定分類**
   - 將設定分為「遊戲設定」、「外觀設定」、「音效設定」等類別
   - 使用標籤頁或側邊欄導航

## 相關文件 (Related Documentation)

- [音效設定](SOUND_SETTINGS.md)
- [棋子圖標設定](ADDING_ICON_SETS.md)
- [棋盤顏色設定](BOARD_COLOR_QUICK_START.md)
