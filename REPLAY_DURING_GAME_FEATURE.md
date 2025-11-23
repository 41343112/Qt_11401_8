# 遊戲進行中回放功能 / Replay During Active Game Feature

## 概述 / Overview

此功能實現了在遊戲進行中使用回放功能的能力，允許玩家在對局期間回顧之前的棋步，而不會影響當前遊戲狀態或計時器。

This feature implements the ability to use replay functionality during an active game, allowing players to review previous moves during a match without affecting the current game state or timer.

## 問題陳述 / Problem Statement

原始問題：**遊戲中也可以使用回放功能**

Original Issue: Replay functionality should also be available during the game.

之前的實現僅允許在遊戲結束後使用回放功能。這限制了玩家在對局期間分析和學習的能力。

The previous implementation only allowed replay functionality after the game ended. This limited players' ability to analyze and learn during an ongoing match.

## 解決方案 / Solution

### 核心改變 / Core Changes

1. **移除遊戲狀態檢查 / Removed Game State Checks**
   - 移除了 `updateReplayButtons()` 中禁用回放按鈕的 `m_gameStarted` 檢查
   - 移除了所有回放按鈕處理器中的 `m_gameStarted` 檢查
   - 移除了棋譜列表雙擊處理器中的 `m_gameStarted` 檢查

2. **計時器管理 / Timer Management**
   - 新增 `m_savedTimerWasActive` 成員變數來保存計時器狀態
   - 進入回放模式時自動暫停計時器
   - 退出回放模式時自動恢復計時器
   - 正確檢查 `m_timerStarted` 和 `m_gameTimer->isActive()` 兩個條件

3. **退出回放按鈕 / Exit Replay Button**
   - 重新加入 `m_exitReplayButton` 成員
   - 只在遊戲進行中回放時顯示
   - 允許玩家快速返回當前遊戲狀態

### 技術實現細節 / Technical Implementation Details

#### 修改的函數 / Modified Functions

**enterReplayMode()**
```cpp
void Qt_Chess::enterReplayMode() {
    if (m_isReplayMode) return;
    
    m_isReplayMode = true;
    
    // 儲存當前棋盤狀態
    saveBoardState();
    
    // 如果遊戲正在進行且計時器正在運行，暫停它
    // 檢查 m_timerStarted 和 m_gameTimer->isActive() 以確保計時器真的在運行
    m_savedTimerWasActive = (m_timerStarted && m_gameTimer && m_gameTimer->isActive());
    if (m_savedTimerWasActive) {
        stopTimer();
    }
    
    // 如果遊戲正在進行，顯示退出回放按鈕
    if (m_gameStarted && m_exitReplayButton) {
        m_exitReplayButton->show();
    }
}
```

**exitReplayMode()**
```cpp
void Qt_Chess::exitReplayMode() {
    if (!m_isReplayMode) return;
    
    m_isReplayMode = false;
    m_replayMoveIndex = -1;
    
    // 恢復棋盤狀態
    restoreBoardState();
    
    // 如果計時器之前正在運行，恢復它
    if (m_savedTimerWasActive) {
        startTimer();
        m_savedTimerWasActive = false;
    }
    
    // 隱藏退出回放按鈕
    if (m_exitReplayButton) {
        m_exitReplayButton->hide();
    }
    
    // 取消棋譜列表的選擇
    m_moveListWidget->clearSelection();
    
    // 更新回放按鈕狀態
    updateReplayButtons();
}
```

**updateReplayButtons()**
```cpp
void Qt_Chess::updateReplayButtons() {
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
    
    // 如果沒有棋步歷史，停用所有按鈕
    if (moveHistory.empty()) {
        if (m_replayFirstButton) m_replayFirstButton->setEnabled(false);
        if (m_replayPrevButton) m_replayPrevButton->setEnabled(false);
        if (m_replayNextButton) m_replayNextButton->setEnabled(false);
        if (m_replayLastButton) m_replayLastButton->setEnabled(false);
        return;
    }
    
    // 在回放模式中，根據當前位置啟用/停用按鈕
    if (m_isReplayMode) {
        // ... 位置相關的按鈕狀態邏輯
    } else {
        // 不在回放模式且有棋步歷史，啟用所有按鈕以允許進入回放
        // 不論遊戲是否正在進行
        if (m_replayFirstButton) m_replayFirstButton->setEnabled(true);
        if (m_replayPrevButton) m_replayPrevButton->setEnabled(true);
        if (m_replayNextButton) m_replayNextButton->setEnabled(true);
        if (m_replayLastButton) m_replayLastButton->setEnabled(true);
    }
}
```

### 新增成員變數 / New Member Variables

```cpp
// qt_chess.h
bool m_savedTimerWasActive;        // 儲存進入回放前計時器是否在運行
QPushButton* m_exitReplayButton;   // 退出回放按鈕（遊戲進行中回放時顯示）
```

### 新增/恢復的方法 / New/Restored Methods

```cpp
void onExitReplayClicked();  // 處理退出回放按鈕點擊
```

## 使用者體驗 / User Experience

### 遊戲進行中回放 / Replay During Active Game

1. **進入回放** / **Entering Replay**
   - 玩家在對局期間點擊任一回放按鈕（⏮ ◀ ▶ ⏭）
   - 或在棋譜列表中雙擊任一步
   - 計時器自動暫停
   - 「退出回放」按鈕出現

2. **導航回放** / **Navigating Replay**
   - 使用回放按鈕瀏覽之前的棋步
   - 棋盤顯示對應的歷史狀態
   - 棋譜列表高亮當前回放位置
   - 棋盤互動被禁用以防意外修改

3. **退出回放** / **Exiting Replay**
   - 點擊「退出回放」按鈕
   - 棋盤立即恢復到當前遊戲狀態
   - 計時器自動恢復運行
   - 玩家可以繼續對弈

### 遊戲結束後回放 / Replay After Game Ends

- 功能與之前相同
- 不顯示「退出回放」按鈕（因為沒有活動遊戲可返回）
- 開始新遊戲時自動退出回放模式

## 優勢 / Benefits

1. **學習與分析** / **Learning and Analysis**
   - 玩家可以在對局期間回顧之前的棋步
   - 有助於理解對手的策略和自己的失誤

2. **公平性** / **Fairness**
   - 計時器自動暫停，確保回放不會消耗玩家時間
   - 自動恢復計時器，確保遊戲流程不中斷

3. **易用性** / **Usability**
   - 簡單直觀的「退出回放」按鈕
   - 無需額外步驟即可返回當前遊戲

4. **向後兼容** / **Backward Compatibility**
   - 遊戲結束後的回放功能保持不變
   - 不影響現有的遊戲流程和功能

## 測試建議 / Testing Recommendations

### 功能測試 / Functional Tests

1. **基本回放** / **Basic Replay**
   - 在遊戲進行中點擊回放按鈕
   - 驗證能夠進入回放模式
   - 驗證能夠瀏覽歷史棋步

2. **計時器行為** / **Timer Behavior**
   - 啟用時間控制開始遊戲
   - 進入回放模式，驗證計時器暫停
   - 退出回放模式，驗證計時器恢復
   - 確認時間沒有流失

3. **退出回放** / **Exit Replay**
   - 點擊「退出回放」按鈕
   - 驗證棋盤恢復到當前狀態
   - 驗證可以繼續移動棋子

4. **邊界情況** / **Edge Cases**
   - 沒有棋步歷史時回放按鈕應該被禁用
   - 在回放模式中不能移動棋子
   - 遊戲結束後不顯示「退出回放」按鈕

### 性能測試 / Performance Tests

1. **長局回放** / **Long Game Replay**
   - 測試超過 50 步的遊戲
   - 驗證回放流暢性
   - 驗證狀態恢復正確性

2. **快速切換** / **Rapid Switching**
   - 快速進入和退出回放模式
   - 驗證沒有記憶體洩漏
   - 驗證狀態一致性

## 已知限制 / Known Limitations

1. 回放模式中不能進行遊戲操作（這是設計特性，確保安全性）
2. 退出回放按鈕只在遊戲進行中顯示（遊戲結束後無需此按鈕）

## 未來改進方向 / Future Enhancements

1. **鍵盤快捷鍵** / **Keyboard Shortcuts**
   - 使用方向鍵導航回放
   - 使用 Esc 鍵退出回放

2. **自動播放** / **Auto-Play**
   - 添加自動播放功能，按固定間隔自動前進
   - 可調整播放速度

3. **註解功能** / **Annotation Feature**
   - 在回放時顯示棋步註解
   - 允許用戶添加註解

## 相關文件 / Related Documents

- [REPLAY_FEATURE.md](REPLAY_FEATURE.md) - 完整的回放功能文檔
- [REPLAY_BUTTONS_FEATURE.md](REPLAY_BUTTONS_FEATURE.md) - 回放按鈕功能文檔
- [README.md](README.md) - 專案整體文檔

## 版本歷史 / Version History

- **v1.0** - 初始實現遊戲進行中回放功能
  - 計時器自動暫停/恢復
  - 退出回放按鈕
  - 移除遊戲狀態限制

## 授權 / License

本功能是 Qt_Chess 專案的一部分，遵循專案的授權條款。

This feature is part of the Qt_Chess project and follows the project's license terms.
