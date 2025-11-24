# Test Verification Plan

## Requirements
1. **按下開始時清空棋譜** (Clear chess record when start button is pressed)
2. **當只有一個有設時間另一個時間顯示為無限制** (When only one side has time set, the other displays as unlimited)

## Test Cases

### Test Case 1: Clear Move List on Start (With Time Control)
**Steps:**
1. Launch the application
2. Set white time to 5 minutes (slider position 6)
3. Set black time to 3 minutes (slider position 4)
4. Click "開始" (Start) button
5. Make several moves (e.g., e4, e5, Nf3, Nc6)
6. Verify moves appear in the left panel move list
7. Click "遊戲" → "新遊戲" to reset
8. Click "開始" again

**Expected Result:**
- Move list should be empty after clicking "開始" in step 8
- Game board should be reset to initial position
- Time should reset to the slider values

### Test Case 2: Clear Move List on Start (Without Time Control)
**Steps:**
1. Launch the application
2. Set both white and black time sliders to position 0 (無限制/unlimited)
3. Click "開始" (Start) button
4. Make several moves
5. Verify moves appear in the move list
6. Click "遊戲" → "新遊戲" to reset
7. Click "開始" again

**Expected Result:**
- Move list should be empty after clicking "開始" in step 7
- Game board should be reset to initial position
- No time controls displayed

### Test Case 3: Display "無限制" for White Unlimited
**Steps:**
1. Launch the application
2. Set white time slider to position 0 (無限制)
3. Set black time slider to position 6 (5 minutes)
4. Click "開始" (Start) button
5. Observe the time displays on both sides of the board

**Expected Result:**
- White time label (right side) should display "無限制"
- Black time label (left side) should display "05:00"
- During gameplay, only black's timer should count down
- White's timer should remain at "無限制"

### Test Case 4: Display "無限制" for Black Unlimited
**Steps:**
1. Launch the application
2. Set white time slider to position 6 (5 minutes)
3. Set black time slider to position 0 (無限制)
4. Click "開始" (Start) button
5. Observe the time displays on both sides of the board

**Expected Result:**
- White time label (right side) should display "05:00"
- Black time label (left side) should display "無限制"
- During gameplay, only white's timer should count down
- Black's timer should remain at "無限制"

### Test Case 5: Both Players Unlimited
**Steps:**
1. Launch the application
2. Set both white and black time sliders to position 0 (無限制)
3. Click "開始" (Start) button
4. Observe the time displays

**Expected Result:**
- Both time labels should display "無限制"
- No timers should count down
- Game can continue indefinitely without time pressure

### Test Case 6: 30 Seconds Option
**Steps:**
1. Launch the application
2. Set white time slider to position 1 (30 seconds)
3. Set black time slider to position 0 (無限制)
4. Click "開始" (Start) button
5. Wait for timer to count down

**Expected Result:**
- White time should display "00:30" initially
- Black time should display "無限制"
- White's timer should count down
- When white's time goes below 10 seconds, format changes to "0:9.8", "0:9.7", etc.

### Test Case 7: Timer Behavior with Unlimited Time
**Steps:**
1. Set white to unlimited (position 0)
2. Set black to 1 minute (position 2)
3. Click "開始"
4. Make a move as white (white's turn ends)
5. Observe black's timer
6. Make a move as black (black's turn ends)
7. Observe white's "timer"

**Expected Result:**
- Black's timer counts down during black's turn
- White's display remains at "無限制" and never changes
- Game continues normally with mixed time controls

## Code Review Points

✅ **Move list clearing**:
- Added in both branches of `onStartButtonClicked()`
- Properly checks for null pointer before clearing
- Synchronized with internal board state reset

✅ **Unlimited time display**:
- Changed condition from `ms < 0` to `ms <= 0`
- Changed display text to "無限制"
- Consistent with Chinese UI throughout the app

✅ **Timer logic**:
- Already handles 0 ms correctly (no countdown)
- No need to modify timer countdown logic
- Time-out detection only triggers when time > 0

✅ **Edge cases handled**:
- Null pointer checks for widgets
- Both time control enabled and disabled scenarios
- Replay mode doesn't interfere with time display

## Manual Testing Checklist

- [ ] Build application successfully
- [ ] Test Case 1: Clear move list with time control
- [ ] Test Case 2: Clear move list without time control
- [ ] Test Case 3: White unlimited, black timed
- [ ] Test Case 4: White timed, black unlimited
- [ ] Test Case 5: Both unlimited
- [ ] Test Case 6: 30 seconds option
- [ ] Test Case 7: Timer behavior with mixed controls
- [ ] Verify no regressions in existing features
- [ ] Take screenshots of key scenarios

## Screenshots Needed

1. Initial state with time control sliders
2. One slider at 0 (無限制), one at non-zero value
3. Game in progress showing "無限制" for unlimited side
4. Move list populated with several moves
5. After clicking start - move list cleared
6. Both sides showing "無限制"
