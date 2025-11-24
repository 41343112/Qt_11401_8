# Implementation Summary: Clear Move List and Show Unlimited Time

## Problem Statement (Chinese)
按下開始時清空棋譜 當只有一個有設時間另一個時間顯示為無限制

Translation:
1. Clear the chess record/move list when the start button is pressed
2. When only one side has a set time, the other time displays as unlimited

## Implementation

### 1. Clear Move List on Start Button Click

**Location**: `qt_chess.cpp` - `onStartButtonClicked()` function (lines 675-677 and 722-724)

**Changes**:
```cpp
// 清空棋譜列表
if (m_moveListWidget) {
    m_moveListWidget->clear();
}
```

**Details**:
- Added move list widget clearing in both branches of the start button handler:
  - When time control is enabled (`m_timeControlEnabled && !m_timerStarted`)
  - When time control is disabled (`!m_timeControlEnabled && !m_gameStarted`)
- The move list widget (`m_moveListWidget`) displays the game moves in the left panel
- The underlying move history (`m_moveHistory`) is already cleared by `m_chessBoard.initializeBoard()` called in `resetBoardState()`
- This ensures both the visual display and internal data structure are cleared when starting a new game

### 2. Display "無限制" (Unlimited) for Zero Time

**Location**: `qt_chess.cpp` - `updateTimeDisplays()` function (lines 1873-1875)

**Changes**:
```cpp
auto formatTime = [](int ms) -> QString {
    if (ms <= 0) {  // Changed from: if (ms < 0)
        return "無限制";  // Changed from: return "不限時";
    }
    // ... rest of the function
};
```

**Details**:
- Changed the condition from `ms < 0` to `ms <= 0` to handle the case when time is exactly 0 (unlimited)
- Changed the display text from "不限時" to "無限制" to match the requirement
- When a player has time set to 0 milliseconds (slider at position 0), they now see "無限制" instead of "00:00"
- This makes it clear that the player has unlimited time rather than 0 seconds remaining

### Timer Logic

The timer countdown logic already properly handles unlimited time:

**Location**: `qt_chess.cpp` - `onGameTimerTick()` function (lines 1948 and 1962)

```cpp
// 僅當白方有時間限制（非無限制）時才減少時間
if (m_whiteTimeMs > 0) {
    m_whiteTimeMs -= 100; // 減少 100ms（計時器每 100ms 觸發一次）
    // ...
}
```

The timer only counts down when time is greater than 0, so players with unlimited time (0 ms) won't have their time decrease.

## Testing

The application builds successfully:
```bash
qmake Qt_Chess.pro
make
```

Resulting executable: `Qt_Chess` (1.1M)

## Behavior

### Scenario 1: Both Players with Time Limits
- White: 5 minutes → Displays "05:00"
- Black: 3 minutes → Displays "03:00"
- Both timers count down during gameplay

### Scenario 2: One Player with Time Limit
- White: 5 minutes → Displays "05:00"
- Black: Unlimited (0) → Displays "無限制"
- Only white's timer counts down
- Black has unlimited time to make moves

### Scenario 3: Both Players Unlimited
- White: Unlimited (0) → Displays "無限制"
- Black: Unlimited (0) → Displays "無限制"
- No timers run, game is untimed

### Move List Behavior
- When "開始" (Start) button is clicked, the move list in the left panel is cleared
- As the game progresses, new moves are recorded and displayed in the move list
- The move list shows moves in algebraic notation (e.g., "1. e4 e5")

## Files Modified

- `qt_chess.cpp`: Main game logic file
  - `onStartButtonClicked()`: Added move list clearing
  - `updateTimeDisplays()`: Modified time formatting for unlimited time
