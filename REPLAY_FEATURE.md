# 遊戲回放功能說明

## 概述

本文件說明 Qt_Chess 中新增的遊戲回放功能和認輸記錄功能。

## 認輸記錄功能

### 功能描述

當玩家在遊戲中選擇「放棄」時，系統現在會正確記錄認輸結果，並在 PGN 輸出中反映出來。

### 實作細節

1. **遊戲結果追蹤**：新增 `GameResult` 枚舉來追蹤所有可能的遊戲結果
   - `InProgress` - 遊戲進行中
   - `WhiteWins` - 白方獲勝（將死黑方）
   - `BlackWins` - 黑方獲勝（將死白方）
   - `Draw` - 和局（逼和或子力不足）
   - `WhiteResigns` - 白方認輸
   - `BlackResigns` - 黑方認輸

2. **PGN 輸出**：
   - 白方認輸：結果為 `0-1`
   - 黑方認輸：結果為 `1-0`
   - 結果會顯示在 PGN 標頭的 `[Result]` 欄位中

3. **使用方式**：
   - 遊戲進行中點擊「放棄」按鈕
   - 確認對話框中選擇「是」
   - 系統自動記錄認輸並結束遊戲
   - 匯出 PGN 時會包含認輸結果

## 遊戲回放功能

### 功能描述

遊戲結束後，玩家可以回放整局對弈，逐步查看每一步棋的演變過程。

### 主要特點

1. **進入回放模式**：
   - 遊戲結束後，雙擊棋譜列表中的任一步
   - 自動進入回放模式並跳到該步

2. **導航控制**：
   - **⏮ (第一步)**：跳到棋局開始的初始狀態
   - **◀ (上一步)**：回退到上一步
   - **▶ (下一步)**：前進到下一步
   - **⏭ (最後一步)**：跳到棋局的最後一步

3. **視覺反饋**：
   - 當前回放的棋步在棋譜列表中會被高亮顯示
   - 棋盤顯示對應棋步的狀態

4. **安全機制**：
   - 回放模式中棋盤互動被完全停用
   - 防止在回放時意外修改棋局
   - 所有滑鼠事件（點擊、拖動）都會被忽略

5. **退出回放**：
   - 點擊「退出回放」按鈕
   - 棋盤恢復到遊戲結束時的最終狀態
   - 所有回放控制按鈕隱藏
   - 匯出和複製功能恢復可用

### 技術實作

#### 狀態管理

```cpp
// 回放模式狀態
bool m_isReplayMode;              // 是否在回放模式
int m_replayMoveIndex;            // 當前回放的棋步索引
std::vector<std::vector<ChessPiece>> m_savedBoardState;  // 儲存的棋盤狀態
PieceColor m_savedCurrentPlayer;  // 儲存的當前玩家
```

#### 核心函數

1. **enterReplayMode()**：進入回放模式
   - 儲存當前棋盤狀態
   - 顯示回放控制按鈕
   - 隱藏遊戲控制按鈕

2. **exitReplayMode()**：退出回放模式
   - 恢復棋盤狀態
   - 隱藏回放控制按鈕
   - 顯示遊戲控制按鈕

3. **replayToMove(int moveIndex)**：跳到指定棋步
   - 重新初始化棋盤
   - 重播所有棋步直到指定位置
   - 更新顯示和按鈕狀態

4. **saveBoardState()**：儲存棋盤狀態
   - 複製所有棋子位置
   - 儲存當前玩家

5. **restoreBoardState()**：恢復棋盤狀態
   - 恢復所有棋子位置
   - 恢復當前玩家
   - 更新顯示

### 使用範例

1. **基本回放流程**：
   ```
   1. 完成一局遊戲（將死、和局或認輸）
   2. 在左側棋譜面板中雙擊任一步棋
   3. 進入回放模式，看到回放控制按鈕
   4. 使用導航按鈕查看每一步
   5. 點擊「退出回放」返回
   ```

2. **快速導航**：
   ```
   - 點擊 ⏮ 查看棋局開始
   - 點擊 ⏭ 查看棋局結束
   - 使用 ◀ ▶ 逐步查看
   ```

### 錯誤處理

1. **索引邊界檢查**：
   - 移動索引始終在有效範圍內
   - 空棋譜歷史的特殊處理

2. **狀態一致性**：
   - 使用 `setPiece()` 方法進行安全的棋子設置
   - 包含邊界檢查（0-7 行列）

3. **UI 狀態**：
   - 按鈕根據當前位置啟用/停用
   - 第一步時「上一步」和「第一步」按鈕停用
   - 最後一步時「下一步」和「最後一步」按鈕停用

## 程式碼結構

### ChessBoard 類別新增

```cpp
// 遊戲結果枚舉
enum class GameResult {
    InProgress, WhiteWins, BlackWins, Draw, WhiteResigns, BlackResigns
};

// 新增方法
GameResult getGameResult() const;
void setGameResult(GameResult result);
QString getGameResultString() const;
void setCurrentPlayer(PieceColor player);
void setPiece(int row, int col, const ChessPiece& piece);
```

### Qt_Chess 類別新增

```cpp
// 回放相關成員
QLabel* m_replayTitle;
QPushButton* m_replayFirstButton;
QPushButton* m_replayPrevButton;
QPushButton* m_replayNextButton;
QPushButton* m_replayLastButton;
QPushButton* m_exitReplayButton;
bool m_isReplayMode;
int m_replayMoveIndex;
std::vector<std::vector<ChessPiece>> m_savedBoardState;
PieceColor m_savedCurrentPlayer;

// 回放相關方法
void enterReplayMode();
void exitReplayMode();
void replayToMove(int moveIndex);
void onReplayFirstClicked();
void onReplayPrevClicked();
void onReplayNextClicked();
void onReplayLastClicked();
void onExitReplayClicked();
void updateReplayButtons();
void saveBoardState();
void restoreBoardState();
```

## 測試建議

1. **認輸功能測試**：
   - 白方認輸測試
   - 黑方認輸測試
   - 檢查 PGN 輸出結果

2. **回放功能測試**：
   - 短局回放（<10 步）
   - 長局回放（>50 步）
   - 邊界情況（只有 1 步）
   - 導航到各個位置

3. **UI 測試**：
   - 按鈕顯示/隱藏
   - 棋譜列表高亮
   - 回放模式防止互動

4. **邊界測試**：
   - 空棋譜歷史
   - 超出範圍的索引
   - 快速連續點擊

## 未來改進方向

1. **性能優化**：
   - 對於很長的遊戲，考慮增量式回放而非每次重新初始化

2. **額外功能**：
   - 回放速度控制（自動播放）
   - 跳到特定移動號
   - 顯示被吃掉的棋子歷史

3. **用戶體驗**：
   - 鍵盤快捷鍵支援
   - 觸控螢幕手勢支援
   - 回放時顯示移動註解

## 授權

本功能是 Qt_Chess 專案的一部分，遵循專案的授權條款。
