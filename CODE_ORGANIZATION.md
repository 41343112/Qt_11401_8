# 程式碼組織說明 (Code Organization)

本文件說明 Qt_Chess 專案的程式碼組織結構。

## 檔案結構概覽

### 核心檔案
- **qt_chess.h / qt_chess.cpp** - 主要應用程式類別，包含 UI 和遊戲邏輯
- **chessboard.h / chessboard.cpp** - 棋盤邏輯和規則引擎
- **chesspiece.h / chesspiece.cpp** - 棋子定義
- **chessengine.h / chessengine.cpp** - 電腦對弈引擎（UCI 協議）

### 網路功能
- **networkmanager.h / networkmanager.cpp** - 線上對戰網路管理
- **onlinedialog.h / onlinedialog.cpp** - 線上對戰對話框

### 設定對話框
- **soundsettingsdialog.h / soundsettingsdialog.cpp** - 音效設定
- **pieceiconsettingsdialog.h / pieceiconsettingsdialog.cpp** - 棋子圖示設定
- **boardcolorsettingsdialog.h / boardcolorsettingsdialog.cpp** - 棋盤顏色設定

### 其他工具
- **updatechecker.h / updatechecker.cpp** - 更新檢查功能
- **theme.h** - 主題和樣式定義

## qt_chess.h 組織結構

### 成員變數分類

#### 1. 核心遊戲狀態 (Core Game State)
- `m_chessBoard` - 棋盤實例
- `m_currentGameMode` - 當前遊戲模式
- `m_gameStarted` - 遊戲是否已開始
- `m_selectedSquare`, `m_pieceSelected` - 選中的棋子
- `m_lastMoveFrom`, `m_lastMoveTo` - 上一步移動

#### 2. UI 元件 (UI Components)
- **基礎佈局**: 主要容器、佈局管理器
- **棋盤**: 棋盤方格按鈕、拖放狀態
- **遊戲控制按鈕**: 新遊戲、認輸、和棋等
- **主選單**: 主選單相關按鈕和容器

#### 3. 時間控制系統 (Time Control System)
- 時間設定滑桿和標籤
- 計時器和進度條
- 本地和伺服器時間同步相關變數

#### 4. 棋譜系統 (Move History System)
- 棋譜列表顯示
- PGN 匯出/複製按鈕

#### 5. 被吃棋子顯示系統 (Captured Pieces Display)
- 被吃棋子面板和標籤
- 分數差異顯示

#### 6. 回放系統 (Replay System)
- 回放控制按鈕
- 回放狀態和索引
- 保存的棋盤狀態

#### 7. 電腦對弈系統 (Computer Chess Engine)
- 引擎實例和控制按鈕
- 難度設定
- 顏色選擇相關

#### 8. 線上對戰系統 (Online Game System)
- 網路管理器
- 連線狀態和房間資訊
- 線上遊戲相關標誌

#### 9. 音效系統 (Sound System)
- 音效實例（移動、吃子、將軍等）
- 背景音樂播放器
- 音效設定

#### 10. 動畫系統 (Animation System)
- 遊戲開始動畫
- 啟動動畫
- 動畫疊加層和計時器

#### 11. 設定系統 (Settings System)
- 棋子圖示設定和快取
- 棋盤顏色設定
- 棋盤翻轉狀態

#### 12. 更新檢查系統 (Update Checker System)
- 更新檢查器實例
- 檢查狀態標誌

### 方法分類

#### 1. UI 設置與佈局 (UI Setup and Layout)
初始化和設置各種 UI 元件的方法。

#### 2. 主選單管理 (Main Menu Management)
主選單顯示/隱藏和導航相關方法。

#### 3. 棋盤顯示與更新 (Board Display and Update)
- 棋盤顯示更新
- 高亮顯示（有效移動、將軍、上一步移動）
- 座標轉換（邏輯座標 ↔ 顯示座標）
- 棋盤輔助功能（升變對話框等）

#### 4. 事件處理 (Event Handlers)
- 滑鼠事件（點擊、拖放）
- 鍵盤事件
- 視窗調整事件
- 按鈕點擊事件

#### 5. 時間控制系統 (Time Control System)
時間顯示、計時器控制、時間設定載入/儲存等。

#### 6. 棋譜管理系統 (Move History Management)
棋譜列表更新、PGN 匯出/複製。

#### 7. 被吃棋子顯示系統 (Captured Pieces Display)
被吃棋子的顯示和分數計算。

#### 8. 回放系統 (Replay System)
回放模式的進入/退出、回放控制。

#### 9. 電腦對弈系統 (Computer Chess Engine)
引擎初始化、模式切換、難度調整、移動請求和回應處理。

#### 10. 線上對戰系統 (Online Game System)
網路連線管理、房間管理、遊戲同步、訊息處理。

#### 11. 音效系統 (Sound System)
音效初始化、播放、背景音樂控制。

#### 12. 動畫系統 (Animation System)
各種動畫效果的播放和控制。

#### 13. 設定系統 (Settings System)
- 棋子圖示：載入、應用、快取管理
- 棋盤顏色：載入、應用
- 棋盤翻轉：載入、儲存

#### 14. 更新檢查系統 (Update Checker System)
更新檢查相關的回調處理。

## qt_chess.cpp 組織結構

實作檔案的方法按照與標頭檔相同的順序組織，每個功能區塊都有清楚的分隔線標記：

```cpp
// ============================================================================
// 功能區塊名稱 (English Name)
// ============================================================================
```

這種組織方式使得：
1. 相關功能的程式碼集中在一起，易於維護
2. 新功能可以按類別加入相應區塊
3. 查找特定功能的實作更加容易
4. 程式碼審查和重構更有效率

## 其他檔案組織

### chessboard.h/cpp
- 棋盤狀態管理
- 移動驗證
- 遊戲規則（將軍、將死、和局等）
- 棋譜記錄

### chessengine.h/cpp
- UCI 協議通訊
- 引擎程序管理
- 難度和思考時間設定
- 棋盤狀態轉換（FEN 格式）

### networkmanager.h/cpp
- WebSocket 連線管理
- 訊息序列化/反序列化
- 房間管理
- 遊戲狀態同步

## 編碼規範

### 命名規則
- **成員變數**: `m_` 前綴，駝峰式命名（例如：`m_chessBoard`）
- **方法**: 駝峰式命名（例如：`updateBoard()`）
- **槽函數**: `on` 前綴（例如：`onSquareClicked()`）
- **常數**: 全大寫加底線（例如：`MAX_TIME_LIMIT_SECONDS`）

### 註解
- 使用中文註解說明複雜邏輯
- 重要的設計決策應該記錄
- 公開 API 應該有清楚的說明

## 維護指南

### 新增功能
1. 在 `qt_chess.h` 的適當區塊新增成員變數和方法宣告
2. 在 `qt_chess.cpp` 的對應區塊實作方法
3. 保持區塊內的組織一致性
4. 更新本文件說明新功能的位置

### 重構建議
- 如果某個區塊變得過大（>1000 行），考慮提取為獨立的類別
- 保持方法簡短（建議 <100 行）
- 複雜功能應該分解為多個小方法

### 程式碼審查重點
- 檢查新增程式碼是否在正確的功能區塊
- 確認相關功能是否集中在一起
- 驗證命名規則是否一致
- 確保適當的註解說明

## 結論

良好的程式碼組織是專案可維護性的關鍵。本專案採用功能導向的組織方式，將相關的程式碼集中在一起，使得開發和維護工作更加高效。
