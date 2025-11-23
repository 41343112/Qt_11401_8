# Replay Timer Fix: Keep Counting for Same Player

## 問題描述 (Problem Description)

當在遊戲進行中進入回放模式時，計時器會繼續倒數，但是當在回放中瀏覽不同的棋步時，計時器會在白方和黑方之間切換，這是不正確的行為。

When entering replay mode during an active game, the timer continues counting down, but as we navigate through different moves in the replay, the timer switches between White and Black players, which is incorrect behavior.

## 預期行為 (Expected Behavior)

當看回放時：
- 時間保持倒數 ✓
- **但是不要換人倒數** ✓ (NEW FIX)

When viewing replay:
- Time keeps counting down ✓
- **But don't switch the person counting down** ✓ (NEW FIX)

計時器應該繼續為進入回放模式時的當前玩家倒數，而不是隨著回放中的棋步切換而改變。

The timer should continue counting down for the player whose turn it was when entering replay mode, and not change as we step through the move history.

## 根本原因 (Root Cause)

在回放模式中，當使用者瀏覽棋譜時，`m_chessBoard.getCurrentPlayer()` 會根據棋盤上重播的棋步狀態而改變。例如：

In replay mode, when the user navigates through the move history, `m_chessBoard.getCurrentPlayer()` changes based on the replayed board state. For example:

```
進入回放 (Enter replay):
- 實際遊戲狀態: 白方回合 (Actual game: White's turn)
- 白方計時器倒數 (White's timer counting down)

回放到第10步 (Replay to move 10):
- 棋盤狀態: 黑方回合 (Board state: Black's turn)
- 錯誤: 切換到黑方計時器倒數 ❌ (BUG: Switches to Black's timer)

回放到第11步 (Replay to move 11):
- 棋盤狀態: 白方回合 (Board state: White's turn)
- 錯誤: 又切換回白方計時器倒數 ❌ (BUG: Switches back to White's timer)
```

這導致在回放過程中，計時器不斷在兩個玩家之間切換，而不是持續為同一個玩家倒數。

This causes the timer to continuously switch between players during replay, instead of consistently counting down for the same player.

## 解決方案 (Solution)

利用已存在的 `m_savedCurrentPlayer` 成員變數，該變數在進入回放模式時通過 `saveBoardState()` 儲存當前玩家。

Utilize the existing `m_savedCurrentPlayer` member variable, which is saved when entering replay mode via `saveBoardState()`.

### 程式碼變更 (Code Changes)

#### 1. onGameTimerTick() 函數

**之前 (Before):**
```cpp
void Qt_Chess::onGameTimerTick() {
    if (!m_timeControlEnabled) return;
    
    // 減少當前玩家的時間
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();  // ❌ 會隨回放切換
    if (currentPlayer == PieceColor::White) {
        // ...
    }
}
```

**之後 (After):**
```cpp
void Qt_Chess::onGameTimerTick() {
    if (!m_timeControlEnabled) return;
    
    // 減少當前玩家的時間
    // 在回放模式中，使用進入回放時儲存的玩家，而不是棋盤上的當前玩家
    // 這樣可以確保在回放過程中時間只從同一個玩家倒數，不會隨著棋步切換
    PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();  // ✓ 回放時使用儲存的玩家
    if (currentPlayer == PieceColor::White) {
        // ...
    }
}
```

#### 2. updateTimeDisplays() 函數

**之前 (Before):**
```cpp
void Qt_Chess::updateTimeDisplays() {
    // ...
    // 根據當前回合和剩餘時間確定背景顏色
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();  // ❌ 會隨回放切換
    
    // 設定白方/黑方標籤的背景顏色（綠色/紅色/灰色）
    if (currentPlayer == PieceColor::White) {
        // 白方標籤為綠色或紅色
    } else {
        // 白方標籤為灰色
    }
}
```

**之後 (After):**
```cpp
void Qt_Chess::updateTimeDisplays() {
    // ...
    // 根據當前回合和剩餘時間確定背景顏色
    // 規則：不是自己的回合時顯示灰色，是自己的回合時根據剩餘時間決定（< 10 秒紅色，否則綠色）
    // 在回放模式中，使用進入回放時儲存的玩家，以保持視覺一致性
    PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();  // ✓ 回放時使用儲存的玩家
    
    // 設定白方/黑方標籤的背景顏色（綠色/紅色/灰色）
    if (currentPlayer == PieceColor::White) {
        // 白方標籤為綠色或紅色（一致顯示白方計時器在運行）
    } else {
        // 白方標籤為灰色
    }
}
```

## 使用情境 (Usage Scenario)

### 範例：白方回合時進入回放 (Example: Entering Replay During White's Turn)

```
1. 實際遊戲狀態 (Actual Game State)
   ┌─────────────────────┐
   │ 白方: 5:30 [綠色]    │ ← 白方回合，計時器在倒數
   │ 黑方: 6:00 [灰色]    │
   └─────────────────────┘

2. 點擊回放按鈕，瀏覽到第10步（黑方回合的狀態）
   ┌─────────────────────┐
   │ 白方: 5:28 [綠色]    │ ← 繼續為白方倒數 ✓
   │ 黑方: 6:00 [灰色]    │ ← 不切換到黑方 ✓
   │ [回放: 第10步]       │
   └─────────────────────┘
   棋盤顯示第10步的位置（黑方回合），但計時器繼續為白方倒數

3. 繼續瀏覽到第15步（白方回合的狀態）
   ┌─────────────────────┐
   │ 白方: 5:26 [綠色]    │ ← 仍然為白方倒數 ✓
   │ 黑方: 6:00 [灰色]    │
   │ [回放: 第15步]       │
   └─────────────────────┘
   棋盤顯示第15步的位置（白方回合），計時器一直為白方倒數

4. 退出回放，返回實際遊戲
   ┌─────────────────────┐
   │ 白方: 5:25 [綠色]    │ ← 回到實際遊戲狀態
   │ 黑方: 6:00 [灰色]    │
   └─────────────────────┘
   計時器繼續為白方倒數（因為實際遊戲狀態仍然是白方回合）
```

### 範例：黑方回合時進入回放 (Example: Entering Replay During Black's Turn)

```
1. 實際遊戲狀態 (Actual Game State)
   ┌─────────────────────┐
   │ 白方: 5:30 [灰色]    │
   │ 黑方: 4:45 [綠色]    │ ← 黑方回合，計時器在倒數
   └─────────────────────┘

2. 進入回放，瀏覽各個棋步
   ┌─────────────────────┐
   │ 白方: 5:30 [灰色]    │
   │ 黑方: 4:43 [綠色]    │ ← 始終為黑方倒數 ✓
   │ [回放中]             │
   └─────────────────────┘
   無論棋盤顯示哪一步（白方回合或黑方回合），計時器都只為黑方倒數
```

## 技術細節 (Technical Details)

### 資料流程 (Data Flow)

```
進入回放模式 (Enter Replay Mode):
┌──────────────────────────────────────┐
│ enterReplayMode()                    │
│  └─> saveBoardState()                │
│       └─> m_savedCurrentPlayer =     │
│           m_chessBoard.getCurrentPlayer() │
└──────────────────────────────────────┘
            ↓
儲存當前玩家（例如：White）

回放過程中 (During Replay):
┌──────────────────────────────────────┐
│ Timer Tick (每100ms)                 │
│  └─> onGameTimerTick()               │
│       └─> currentPlayer =            │
│           m_isReplayMode ?           │
│           m_savedCurrentPlayer :     │ ← 使用儲存的玩家
│           m_chessBoard.getCurrentPlayer() │
│       └─> 減少 currentPlayer 的時間  │
└──────────────────────────────────────┘
            ↓
始終為同一個玩家（White）倒數

┌──────────────────────────────────────┐
│ Display Update                       │
│  └─> updateTimeDisplays()            │
│       └─> currentPlayer =            │
│           m_isReplayMode ?           │
│           m_savedCurrentPlayer :     │ ← 使用儲存的玩家
│           m_chessBoard.getCurrentPlayer() │
│       └─> 設定對應玩家的標籤為綠色/紅色 │
└──────────────────────────────────────┘
            ↓
始終高亮同一個玩家（White）的計時器

退出回放模式 (Exit Replay Mode):
┌──────────────────────────────────────┐
│ exitReplayMode()                     │
│  └─> restoreBoardState()             │
│       └─> 恢復實際遊戲狀態            │
│  └─> m_isReplayMode = false         │
└──────────────────────────────────────┘
            ↓
計時器繼續為實際遊戲的當前玩家倒數
```

## 優點 (Benefits)

1. **公平性 (Fairness)**: 回放不會讓任何一方獲得時間優勢
2. **一致性 (Consistency)**: 視覺顯示（計時器高亮）與實際倒數的玩家一致
3. **直覺性 (Intuitiveness)**: 使用者清楚知道哪個玩家的時間在流逝
4. **程式碼簡潔 (Code Simplicity)**: 利用現有的變數，無需引入新的狀態

## 測試建議 (Testing Recommendations)

### 測試案例 1: 白方回合時回放
1. 開始遊戲，走幾步棋，確保當前是白方回合
2. 記錄白方當前時間（例如：5:30）
3. 進入回放模式
4. 瀏覽到黑方回合的棋步（例如：第10步）
5. ✓ 驗證白方計時器繼續倒數（例如：5:28）
6. ✓ 驗證白方計時器標籤保持綠色/紅色高亮
7. ✓ 驗證黑方計時器不變且標籤為灰色
8. 瀏覽到其他棋步
9. ✓ 驗證白方計時器持續倒數
10. 退出回放
11. ✓ 驗證白方計時器繼續正常倒數

### 測試案例 2: 黑方回合時回放
1. 開始遊戲，走幾步棋，確保當前是黑方回合
2. 記錄黑方當前時間（例如：4:45）
3. 進入回放模式
4. 瀏覽到白方回合的棋步（例如：第5步）
5. ✓ 驗證黑方計時器繼續倒數（例如：4:43）
6. ✓ 驗證黑方計時器標籤保持綠色/紅色高亮
7. ✓ 驗證白方計時器不變且標籤為灰色
8. 瀏覽到其他棋步
9. ✓ 驗證黑方計時器持續倒數
10. 退出回放
11. ✓ 驗證黑方計時器繼續正常倒數

### 測試案例 3: 低時間警告（< 10秒）
1. 設定時間控制為較短時間（例如：20秒）
2. 等待白方時間降至10秒以下
3. ✓ 驗證白方計時器標籤變為紅色
4. 進入回放模式
5. 瀏覽各個棋步
6. ✓ 驗證白方計時器標籤保持紅色
7. ✓ 驗證白方時間繼續倒數直到0秒
8. ✓ 如果時間到0，驗證顯示「白方超時」訊息

## 相關檔案 (Related Files)

- `qt_chess.cpp`: 主要變更檔案
  - `onGameTimerTick()`: 計時器更新邏輯
  - `updateTimeDisplays()`: 顯示更新邏輯
  - `enterReplayMode()`: 進入回放模式（呼叫 saveBoardState）
  - `saveBoardState()`: 儲存當前玩家到 m_savedCurrentPlayer
- `qt_chess.h`: 成員變數定義
  - `m_isReplayMode`: 回放模式標誌
  - `m_savedCurrentPlayer`: 儲存的當前玩家

## 實作日期 (Implementation Date)
2024-11-23

## 狀態 (Status)
✅ **完成 (COMPLETE)** - 程式碼變更已實作並提交
