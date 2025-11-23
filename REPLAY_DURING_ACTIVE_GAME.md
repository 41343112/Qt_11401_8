# 遊戲進行中回放功能 / Replay During Active Game Feature

## 概述 / Overview

此功能更新允許玩家在遊戲進行中使用回放功能來回顧之前的棋步，然後繼續遊戲。這是一個常見的國際象棋應用功能，讓玩家可以在對局中分析自己的位置。

This feature update allows players to use the replay functionality during an active game to review previous moves, then continue playing. This is a common chess application feature that lets players analyze their position during a match.

## 問題陳述 / Problem Statement

**中文**: 正在下棋的時候也可以使用回放

**English**: Allow using replay/playback while playing chess

## 變更前 / Before Changes

- 回放按鈕在遊戲進行中被停用
- 玩家只能在遊戲結束後使用回放功能
- 無法在遊戲中回顧之前的棋步

- Replay buttons were disabled during an active game
- Players could only use replay after the game ended
- Could not review previous moves during the game

## 變更後 / After Changes

- 回放按鈕在遊戲進行中只要有棋步歷史就會啟用
- 玩家可以在任何時候進入回放模式回顧棋步
- 新增「退出回放」按鈕用於返回當前遊戲狀態
- 視窗標題顯示「(回放模式)」以指示正在回放

- Replay buttons are enabled during an active game when there are moves to review
- Players can enter replay mode at any time to review moves
- Added "Exit Replay" button to return to current game state
- Window title shows "(回放模式)" to indicate replay mode

## 使用方式 / How to Use

### 開始回放 / Starting Replay

在遊戲進行中，有三種方式進入回放模式：

During an active game, there are three ways to enter replay mode:

1. **點擊回放導航按鈕** / **Click Replay Navigation Buttons**
   - 點擊 ⏮ (第一步)、◀ (上一步)、▶ (下一步) 或 ⏭ (最後一步)
   - Click ⏮ (First), ◀ (Previous), ▶ (Next), or ⏭ (Last)

2. **雙擊棋譜列表** / **Double-click Move List**
   - 在左側棋譜面板中雙擊任一棋步
   - Double-click any move in the left move list panel

3. **自動進入** / **Automatic Entry**
   - 系統會自動進入回放模式並跳到選定的位置
   - System automatically enters replay mode and jumps to the selected position

### 回放中導航 / Navigating During Replay

- **⏮ 第一步** / **⏮ First**: 跳到棋局開始的初始狀態
- **◀ 上一步** / **◀ Previous**: 回到前一個棋步
- **▶ 下一步** / **▶ Next**: 前進到下一個棋步  
- **⏭ 最後一步** / **⏭ Last**: 跳到已下的最後一步（但不是當前遊戲狀態）

### 退出回放 / Exiting Replay

在遊戲進行中的回放模式，有兩種方式退出：

When in replay mode during an active game, there are two ways to exit:

1. **點擊「退出回放」按鈕** / **Click "Exit Replay" Button**
   - 此按鈕只在遊戲進行中的回放時顯示
   - This button only appears during replay in an active game
   - 點擊後恢復到當前遊戲狀態並繼續遊戲
   - Click to restore current game state and continue playing

2. **開始新遊戲** / **Start New Game**
   - 點擊「新遊戲」會自動退出回放模式
   - Clicking "New Game" automatically exits replay mode

### 視覺提示 / Visual Indicators

1. **視窗標題** / **Window Title**
   - 回放模式中：「國際象棋 - 雙人對弈 (回放模式)」
   - During replay: "國際象棋 - 雙人對弈 (回放模式)"
   - 正常遊戲：「國際象棋 - 雙人對弈」
   - Normal game: "國際象棋 - 雙人對弈"

2. **退出回放按鈕** / **Exit Replay Button**
   - 只在遊戲進行中的回放時顯示
   - Only visible during replay in an active game
   - 遊戲結束後的回放不顯示此按鈕
   - Not shown during replay after game ends

3. **回放按鈕狀態** / **Replay Button State**
   - 根據當前回放位置啟用/停用
   - Enabled/disabled based on current replay position
   - 在初始狀態時「第一步」和「上一步」停用
   - "First" and "Previous" disabled at initial state
   - 在最後一步時「下一步」和「最後一步」停用
   - "Next" and "Last" disabled at last move

## 安全機制 / Safety Mechanisms

1. **棋盤互動停用** / **Board Interaction Disabled**
   - 回放模式中所有棋盤點擊和拖動被忽略
   - All board clicks and drags are ignored during replay mode
   - 防止在回放時意外修改棋局
   - Prevents accidental modification of the game during replay

2. **狀態保存與恢復** / **State Save and Restore**
   - 進入回放時自動保存當前棋盤狀態
   - Current board state is automatically saved when entering replay
   - 退出回放時完整恢復狀態
   - State is fully restored when exiting replay
   - 包括所有棋子位置和當前玩家
   - Includes all piece positions and current player

3. **按鈕狀態管理** / **Button State Management**
   - 回放按鈕只在有棋步歷史時啟用
   - Replay buttons only enabled when there are moves to review
   - 防止在空棋局時進入回放
   - Prevents entering replay with empty game

## 技術細節 / Technical Details

### 修改的檔案 / Modified Files

1. **qt_chess.h**
   - 恢復 `m_exitReplayButton` 成員變數
   - 恢復 `onExitReplayClicked()` 槽函數聲明

2. **qt_chess.cpp**
   - 移除所有回放按鈕處理器中的 `if (m_gameStarted) return;` 檢查
   - 更新 `updateReplayButtons()` 在遊戲進行中啟用按鈕
   - 移除棋譜列表雙擊處理器中的 `if (!m_gameStarted)` 檢查
   - 新增「退出回放」按鈕的創建和處理
   - 更新 `enterReplayMode()` 和 `exitReplayMode()` 以處理活躍遊戲

### 核心函數變更 / Core Function Changes

1. **onReplayFirstClicked() 等**
   - 移除：`if (m_gameStarted) return;`
   - 保留：自動進入回放模式的邏輯

2. **updateReplayButtons()**
   - 移除：遊戲進行中停用所有按鈕的邏輯
   - 保留：根據回放位置和棋步歷史啟用/停用按鈕

3. **enterReplayMode()**
   - 新增：檢查 `m_gameStarted` 決定是否顯示退出按鈕
   - 新增：更新視窗標題以指示回放模式

4. **exitReplayMode()**
   - 新增：檢查 `m_gameStarted` 恢復視窗標題
   - 新增：隱藏退出回放按鈕

5. **onExitReplayClicked()**
   - 新增：處理退出回放按鈕點擊
   - 功能：調用 `exitReplayMode()` 恢復遊戲狀態

## 測試建議 / Testing Recommendations

### 基本功能測試 / Basic Functionality Tests

1. **進入回放** / **Entering Replay**
   - ✓ 在遊戲進行中點擊回放按鈕
   - ✓ 在遊戲進行中雙擊棋譜列表
   - ✓ 確認進入回放模式並顯示正確狀態

2. **回放導航** / **Replay Navigation**
   - ✓ 測試所有四個導航按鈕
   - ✓ 確認棋盤顯示正確的歷史狀態
   - ✓ 確認按鈕根據位置正確啟用/停用

3. **退出回放** / **Exiting Replay**
   - ✓ 點擊「退出回放」按鈕
   - ✓ 確認恢復到正確的當前遊戲狀態
   - ✓ 確認可以繼續下棋

### 邊界情況測試 / Edge Case Tests

1. **第一步棋後** / **After First Move**
   - ✓ 只有一步棋時進入回放
   - ✓ 測試導航到初始狀態和第一步

2. **遊戲結束後** / **After Game Ends**
   - ✓ 確認「退出回放」按鈕不顯示
   - ✓ 確認回放功能仍然正常工作

3. **快速切換** / **Rapid Switching**
   - ✓ 快速進入和退出回放多次
   - ✓ 確認狀態保存和恢復正確

### 整合測試 / Integration Tests

1. **時間控制** / **Time Control**
   - ✓ 在有時間控制的遊戲中使用回放
   - ✓ 確認計時器在回放時正確處理

2. **特殊移動** / **Special Moves**
   - ✓ 回放包含吃子的棋步
   - ✓ 回放包含王車易位的棋步
   - ✓ 回放包含將軍的棋步

## 相容性 / Compatibility

- **向後相容** / **Backward Compatible**: ✅
  - 現有的回放功能在遊戲結束後仍然正常工作
  - Existing replay functionality after game ends still works normally

- **破壞性變更** / **Breaking Changes**: ❌
  - 沒有破壞性變更
  - No breaking changes

- **新增依賴** / **New Dependencies**: ❌
  - 沒有新增外部依賴
  - No new external dependencies

## 已知限制 / Known Limitations

1. **無法在回放中下棋** / **Cannot Move During Replay**
   - 設計如此：必須先退出回放才能繼續下棋
   - By design: must exit replay before continuing to play

2. **計時器暫停** / **Timer Paused**
   - 回放時計時器行為取決於實作
   - Timer behavior during replay depends on implementation
   - 建議測試以確認預期行為
   - Recommended to test to confirm expected behavior

## 未來改進 / Future Improvements

1. **鍵盤快捷鍵** / **Keyboard Shortcuts**
   - 新增方向鍵支援回放導航
   - Add arrow key support for replay navigation

2. **自動播放** / **Auto-play**
   - 新增自動播放功能以指定速度回放
   - Add auto-play feature to replay at specified speed

3. **回放標記** / **Replay Markers**
   - 在棋盤上標記最後一步移動
   - Mark the last move on the board

4. **回放註解** / **Replay Annotations**
   - 在回放時顯示移動註解
   - Show move annotations during replay

## 授權 / License

本功能是 Qt_Chess 專案的一部分，遵循專案的授權條款。

This feature is part of the Qt_Chess project and follows the project's license terms.

---

**版本** / **Version**: 1.0  
**日期** / **Date**: 2025-11-23  
**作者** / **Author**: GitHub Copilot
