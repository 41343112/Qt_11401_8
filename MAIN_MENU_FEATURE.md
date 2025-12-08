# 主選單功能 (Main Menu Feature)

## 概述

此功能為國際象棋遊戲添加了主選單畫面，使用者在啟動應用程式時首先看到主選單，可以選擇不同的遊戲模式。

## 功能說明

### 主選單畫面

應用程式啟動後，會顯示主選單畫面，包含以下四個按鈕：

1. **🎮 本地遊玩** - 啟動本地雙人對戰模式
2. **🤖 與電腦對戰** - 啟動與 AI 電腦對弈模式
3. **🌐 線上遊玩** - 啟動線上對戰模式
4. **⚙️ 設定** - 開啟遊戲設定

### 主要特點

- **現代科技風格設計**：主選單採用霓虹電路板主題，與遊戲整體風格一致
- **大按鈕設計**：按鈕大小適中，易於點擊
- **清晰的視覺回饋**：滑鼠懸停和點擊時有視覺效果
- **返回主選單功能**：在遊戲中可以通過選單欄返回主選單

## 技術實現

### 新增的 UI 元件

```cpp
// 主選單相關成員變數
QWidget* m_mainMenuWidget;                 // 主選單容器
QPushButton* m_mainMenuLocalPlayButton;    // 本地遊玩按鈕
QPushButton* m_mainMenuComputerPlayButton; // 與電腦對戰按鈕
QPushButton* m_mainMenuOnlinePlayButton;   // 線上遊玩按鈕
QPushButton* m_mainMenuSettingsButton;     // 設定按鈕
QWidget* m_gameContentWidget;              // 遊戲內容容器
```

### 主要函數

1. **setupMainMenu()** - 創建並配置主選單 UI
2. **showMainMenu()** - 顯示主選單，隱藏遊戲內容
3. **showGameContent()** - 隱藏主選單，顯示遊戲內容
4. **onMainMenuLocalPlayClicked()** - 處理本地遊玩按鈕點擊
5. **onMainMenuComputerPlayClicked()** - 處理電腦對戰按鈕點擊
6. **onMainMenuOnlinePlayClicked()** - 處理線上遊玩按鈕點擊
7. **onMainMenuSettingsClicked()** - 處理設定按鈕點擊
8. **onBackToMainMenuClicked()** - 返回主選單

### UI 結構變更

原有的 UI 結構：
```
CentralWidget
└── MainLayout (遊戲內容)
    ├── ContentLayout (棋盤和控制面板)
    └── ...
```

新的 UI 結構：
```
CentralWidget
└── RootLayout
    ├── MainMenuWidget (主選單)
    │   └── MenuLayout
    │       ├── TitleLabel
    │       ├── LocalPlayButton
    │       ├── ComputerPlayButton
    │       ├── OnlinePlayButton
    │       └── SettingsButton
    └── GameContentWidget (遊戲內容)
        └── MainLayout
            ├── ContentLayout
            └── ...
```

## 使用流程

### 1. 啟動應用程式

- 應用程式啟動後顯示主選單畫面
- 遊戲內容和選單欄初始隱藏

### 2. 選擇遊戲模式

#### 本地遊玩
- 點擊「本地遊玩」按鈕
- 切換到遊戲畫面
- 自動設置為雙人對戰模式
- 開始新遊戲

#### 與電腦對戰
- 點擊「與電腦對戰」按鈕
- 切換到遊戲畫面
- 自動設置為電腦對戰模式
- 使用者可以選擇執白或執黑

#### 線上遊玩
- 點擊「線上遊玩」按鈕
- 切換到遊戲畫面
- 顯示線上對戰連線對話框

#### 設定
- 點擊「設定」按鈕
- 切換到遊戲畫面
- 自動開啟音效設定對話框

### 3. 返回主選單

在遊戲畫面中：
- 點擊選單欄的「遊戲」→「返回主選單」
- 如果遊戲進行中會詢問確認
- 確認後返回主選單畫面

## 樣式設計

主選單使用與遊戲一致的現代科技風格：

```cpp
// 主題顏色
THEME_BG_DARK = "#0A1628"           // 深色背景
THEME_BG_MEDIUM = "#0D1F3C"         // 中等深度背景
THEME_BG_PANEL = "#0F2940"          // 面板背景
THEME_ACCENT_PRIMARY = "#00FFFF"    // 主要強調色（霓虹青色）
THEME_ACCENT_SUCCESS = "#00D9FF"    // 成功色
THEME_TEXT_PRIMARY = "#E8F4F8"      // 主要文字顏色
```

## 行為說明

### 遊戲狀態保護

當從遊戲中返回主選單時：
- 如果遊戲正在進行且未結束，會顯示確認對話框
- 使用者可以選擇繼續遊戲或返回主選單
- 返回主選單會重置遊戲狀態

### 選單欄可見性

- 主選單畫面：隱藏選單欄
- 遊戲畫面：顯示選單欄

## 未來改進

可能的改進方向：
1. 添加遊戲說明/教學按鈕
2. 添加遊戲統計/歷史記錄
3. 添加快速重新開始按鈕
4. 改進主選單動畫效果
5. 添加主選單背景動畫或圖片
