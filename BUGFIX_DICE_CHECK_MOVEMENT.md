# Bug Fix: Dice Mode Check Interruption - Both Sides Cannot Move

## Issue Report

**Reporter**: @41343112  
**Issue**: 修正當對方王被攻擊時兩邊都不能動 (Fix: Both sides cannot move when opponent's king is attacked)

## Problem Analysis

When the check interruption feature was implemented, a critical bug occurred:

### Symptoms
- When Player A puts Player B's king in check during dice mode
- Player B cannot select or move any pieces
- Player A also becomes unable to move after the interruption
- Game becomes stuck/locked

### Root Cause
1. **Attacker's dice state cleared**: When check interruption occurs, the code clears `m_rolledPieceTypes` and sets `m_diceMovesRemaining = 0`
2. **Opponent has no dice**: The opponent (who needs to respond to check) has no dice rolled because dice are only rolled at the start of a turn
3. **Dice restriction still active**: The `isPieceTypeInRolledList()` function returns `false` for all pieces when `m_rolledPieceTypes` is empty
4. **Cannot select pieces**: Lines 2455, 2626, and 3500 check `isPieceTypeInRolledList()` before allowing piece selection, blocking all moves

## Solution

### Implementation
Added a new state flag `m_diceRespondingToCheck` to bypass dice restrictions when a player is responding to check.

### Changes Made

#### 1. Header File (src/qt_chess.h)
```cpp
bool m_diceRespondingToCheck;  // 當前玩家是否正在應對將軍（允許移動任何棋子）
```

#### 2. Initialization (src/qt_chess.cpp)
- Constructor: Initialize to `false`
- Reset functions: Clear flag on game reset

#### 3. Set Flag on Check Interruption
When check interruption occurs (lines 2589, 3730):
```cpp
// 設置對手正在應對將軍標記（允許對手移動任何棋子）
m_diceRespondingToCheck = true;
```

#### 4. Clear Flag After Response
When player moves to escape check (lines 2565, 3710):
```cpp
if (m_diceRespondingToCheck) {
    qDebug() << "[Qt_Chess] Player responded to check, clearing responding flag";
    m_diceRespondingToCheck = false;
}
```

#### 5. Bypass Dice Check (line 8961)
Modified `isPieceTypeInRolledList()`:
```cpp
// 如果玩家正在應對將軍，允許移動任何棋子
if (m_diceRespondingToCheck) {
    return true;
}
```

#### 6. Clear Flag on Turn Restoration (line 6304)
When interrupted player's turn is restored:
```cpp
// 清除中斷標記和應對將軍標記
m_diceCheckInterrupted = false;
m_diceInterruptedPlayer = PieceColor::None;
m_diceRespondingToCheck = false;
```

## Testing Verification

### Test Scenario
1. Start online game with dice mode
2. Player A rolls dice and moves pieces
3. Player A puts Player B's king in check (not checkmate) mid-turn
4. **Verify**: Player B can select and move any piece
5. Player B moves to escape check
6. **Verify**: Turn returns to Player A
7. **Verify**: Player A can continue with remaining dice moves

### Expected Behavior After Fix
- ✅ Player B can move any piece to escape check (not restricted by dice)
- ✅ Player A's interrupted turn is properly restored
- ✅ Both players can move throughout the game
- ✅ Dice restrictions apply normally except when responding to check

## Commit Information

**Commit Hash**: 9f22488  
**Commit Message**: Fix: Allow piece movement when responding to check in dice mode

Add m_diceRespondingToCheck flag to allow players to move any piece when responding to check, not restricted by dice rolls. This fixes the issue where both sides couldn't move after a check occurred.

## Files Modified

1. **src/qt_chess.h**: Added `m_diceRespondingToCheck` member variable
2. **src/qt_chess.cpp**: 
   - Initialize flag in constructor
   - Set flag on check interruption
   - Clear flag after response
   - Modify `isPieceTypeInRolledList()` to bypass check
   - Reset flag in cleanup functions

**Total Changes**: 31 lines added, 1 line modified

## Related Documentation

- [DICE_MODE_CHECK_RULE.md](DICE_MODE_CHECK_RULE.md) - Original feature documentation
- [IMPLEMENTATION_SUMMARY_DICE_CHECK.md](IMPLEMENTATION_SUMMARY_DICE_CHECK.md) - Implementation summary

## Notes

This bug highlights the importance of considering state transitions in game logic. The original implementation correctly saved and restored dice state but didn't account for the fact that the responding player needed unrestricted movement to escape check. The fix maintains the intended behavior while allowing necessary freedom of movement.
