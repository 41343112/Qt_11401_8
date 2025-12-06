# Timer Start Behavior Fix

## Issue
In offline mode (Two-player and Computer modes), the timer would only start after White made the first move. This caused confusion because:
1. Users couldn't see the timer counting down when the game started
2. It was unclear when Black's timer would begin
3. The behavior was inconsistent with online mode and standard chess timing

## Root Cause
The timer initialization logic was located in the move handler (`onSquareClicked`), checking for the first move:
```cpp
// Old code - timer started after first move
bool isFirstMove = m_uciMoveHistory.isEmpty();
if (isFirstMove && m_timeControlEnabled && !m_timerStarted) {
    m_timerStarted = true;
    m_gameStartLocalTime = QDateTime::currentMSecsSinceEpoch();
    m_currentTurnStartTime = m_gameStartLocalTime;
    startTimer();
}
```

## Solution
Moved timer initialization to `onStartButtonClicked`, so the timer starts immediately when the game begins:
```cpp
// New code - timer starts when game begins
m_gameStarted = true;
m_timerStarted = true;
m_gameStartLocalTime = QDateTime::currentMSecsSinceEpoch();
m_currentTurnStartTime = m_gameStartLocalTime;
startTimer();
```

## Behavior After Fix

### Offline Mode (Two-player and Computer)
1. User sets time controls and clicks "Start"
2. **Timer starts immediately** - White's clock begins counting down
3. White makes a move
4. Black's clock begins counting down (White's stops)
5. Black makes a move
6. White's clock resumes counting down (Black's stops)

### Consistency with Online Mode
The fix makes offline mode behavior consistent with online mode, which already started the timer at game begin (in `onStartGameReceived`).

## Files Modified
- `src/qt_chess.cpp`:
  - Added timer start logic in `onStartButtonClicked` (lines 1224-1229)
  - Removed timer start logic from `onSquareClicked` (removed lines 949-957)

## Benefits
1. **Clear timing**: Users can immediately see the timer counting when game starts
2. **Standard behavior**: Matches standard chess timing rules (clock starts at game begin)
3. **Consistency**: All modes (offline and online) now have identical timer behavior
4. **Better UX**: No confusion about when Black's timer will start

## Testing Considerations
- Verify timer starts counting immediately when Start button is clicked
- Verify White's timer counts down before first move
- Verify Black's timer starts after White makes first move
- Verify timer behavior is same for Two-player and Computer modes
- Verify unlimited time still works (timer doesn't start if time is 0)
