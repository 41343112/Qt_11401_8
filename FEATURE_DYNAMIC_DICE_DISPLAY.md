# Feature: Dynamic Dice Display Updates

## Overview

**Chinese**: 骰子模式 每次移動後動態更新骰子顯示，檢查該類型棋子是否還存在，檢查該類型棋子是否還有合法移動，如果不能動就將骰子灰階顯示

**English Translation**: In dice mode, dynamically update the dice display after each move. Check if pieces of that type still exist, check if they have legal moves, and display in grayscale if they cannot move.

## Problem Description

### Previous Behavior
The dice display panel would only show a dice slot in grayscale when the remaining move count reached 0. This meant that even if:
- All pieces of that type were captured
- All pieces of that type were blocked and had no legal moves

...the dice slot would still show as active (colored) if the move count hadn't been used yet.

### Issue Scenarios

1. **Piece Captured**: A player rolls a Knight, but the opponent captures the Knight before the player can move it. The dice slot still shows the Knight as available even though no Knights remain on the board.

2. **Blocked Pieces**: A player rolls a Rook, but all Rooks are blocked by other pieces with no legal moves. The dice slot shows the Rook as available even though it cannot be moved.

3. **Confusion**: Players see active dice slots for pieces they cannot actually move, leading to confusion and wasted time trying to select unmovable pieces.

## Solution

### Implementation

Added dynamic checking to the dice display system that updates after each move:

1. **New Helper Function**: `canPieceTypeMove()`
   - Checks if any piece of the specified type and color exists on the board
   - Verifies if at least one piece of that type has a legal move available
   - Returns `true` only if both conditions are met

2. **Enhanced Display Logic**: Modified `updateDiceDisplay()`
   - Now checks two conditions for grayscale display:
     - Remaining moves count <= 0 (original condition)
     - Piece type cannot move (new condition)
   - Updates dynamically whenever the display is refreshed

3. **Automatic Updates**
   - The display is already updated after each move via `markPieceTypeAsMoved()`
   - The new check is integrated into the existing update flow
   - No additional update calls needed

## Technical Details

### Files Modified

1. **src/qt_chess.h** (1 line added)
   - Added function declaration: `bool canPieceTypeMove(PieceType type, PieceColor color) const;`

2. **src/qt_chess.cpp** (32 lines added)
   - Implemented `canPieceTypeMove()` function (26 lines)
   - Modified `updateDiceDisplay()` function (6 lines)

### Code Changes

#### New Function: `canPieceTypeMove()`

```cpp
bool Qt_Chess::canPieceTypeMove(PieceType type, PieceColor color) const {
    // Iterate through the board to find pieces of specified type and color
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            // Check if this is the target piece type and color
            if (piece.getType() == type && piece.getColor() == color) {
                QPoint from(col, row);
                
                // Check if this piece has any legal moves
                for (int toRow = 0; toRow < 8; ++toRow) {
                    for (int toCol = 0; toCol < 8; ++toCol) {
                        QPoint to(toCol, toRow);
                        if (m_chessBoard.isValidMove(from, to)) {
                            return true;  // Found a legal move
                        }
                    }
                }
            }
        }
    }
    
    return false;  // No pieces found or no legal moves available
}
```

#### Modified: `updateDiceDisplay()`

**Before:**
```cpp
// If piece has been moved, display in grayscale
if (remainingMoves <= 0) {
    label->setStyleSheet(/* grayscale style */);
}
```

**After:**
```cpp
// Determine if should display in grayscale:
// 1. Remaining moves count is exhausted, OR
// 2. Piece type no longer exists or has no legal moves
PieceColor diceOwnerColor = m_chessBoard.getCurrentPlayer();
bool canMove = canPieceTypeMove(type, diceOwnerColor);

if (remainingMoves <= 0 || !canMove) {
    label->setStyleSheet(/* grayscale style */);
}
```

## Behavior After Implementation

### Dice Display States

A dice slot will now show as **grayscale (disabled)** when ANY of these conditions are true:
1. The move count for that slot has been exhausted (original behavior)
2. No pieces of that type exist on the board (new behavior)
3. All pieces of that type are blocked with no legal moves (new behavior)

A dice slot will show as **colored (active)** only when ALL of these conditions are true:
1. The move count for that slot is > 0
2. At least one piece of that type exists on the board
3. At least one piece of that type has a legal move available

### Example Scenarios

#### Scenario 1: Piece Captured
1. White rolls: Knight, Rook, Pawn
2. White moves Rook (dice shows: Knight✓, Rook✗, Pawn✓)
3. Black captures White's last Knight
4. **Result**: Knight dice slot now shows grayscale because no Knights remain

#### Scenario 2: Blocked Pieces
1. White rolls: Rook, Bishop, King
2. White's Rooks are both blocked with no legal moves
3. **Result**: Rook dice slot shows grayscale even though move count is 1

#### Scenario 3: Freed Pieces
1. White rolls: Knight, Rook, Pawn
2. White's Knights are initially blocked (Knight slot shows grayscale)
3. Black moves a piece, freeing a Knight
4. **Result**: If it's still White's turn and updateDiceDisplay() is called, Knight slot becomes active

## Performance Considerations

### Complexity
- `canPieceTypeMove()` is O(64 * 64) = O(4096) in worst case
- Called once per dice slot (max 3) per display update
- Total worst case: O(12,288) operations per update

### Optimization
The function uses early return:
- Returns `true` immediately when first legal move is found
- Typical case is much faster than worst case
- Most pieces will have legal moves, so returns quickly

### Update Frequency
- Called only when `updateDiceDisplay()` is invoked
- This happens after moves, not continuously
- Performance impact is negligible for typical gameplay

## Testing Recommendations

### Manual Testing Scenarios

1. **Basic Functionality**
   - Start online game in dice mode
   - Roll dice and observe initial display (all active)
   - Make a move and verify display updates
   - Capture opponent's piece of rolled type
   - Verify that dice slot becomes grayscale

2. **Blocked Pieces Test**
   - Create a board position where a piece type is blocked
   - Roll that piece type
   - Verify dice slot shows grayscale
   - Free the piece by moving blockers
   - Verify dice slot becomes active (may need to trigger update)

3. **All Pieces Captured**
   - Roll a piece type
   - Capture all pieces of that type
   - Verify dice slot becomes grayscale
   - Verify slot remains grayscale for rest of turn

4. **Multiple Dice Slots**
   - Roll 3 different piece types
   - Create scenarios where each becomes unavailable
   - Verify each slot independently shows correct state

5. **Turn Switching**
   - Verify display updates correctly when turn switches
   - Check both player perspectives (if testing with two clients)

### Edge Cases

1. **King in Check**: Verify blocked pieces still show correctly when king is in check
2. **Pawn Promotion**: Test behavior before/after pawn promotes to another type
3. **Castling**: Verify King/Rook availability when castling is the only legal move
4. **En Passant**: Test Pawn availability when en passant is the only legal move

## Related Features

### Existing Dice Mode Features
- Dice rolling and piece type selection
- Move count tracking
- Dice display panel UI
- Turn-based dice updates

### Related Check Interruption
- When player is responding to check (`m_diceRespondingToCheck`)
- Check interruption and turn restoration
- See: `BUGFIX_DICE_CHECK_MOVEMENT.md`

## Future Enhancements

### Potential Improvements

1. **Visual Feedback**
   - Add tooltip showing why a dice slot is disabled
   - Show message: "No pieces available" vs "No legal moves"
   - Animate transition between active and grayscale

2. **Optimization**
   - Cache piece positions by type for faster lookup
   - Only recheck affected piece types after moves
   - Lazy evaluation on first access

3. **Server Synchronization**
   - Server could send piece availability status
   - Reduce client-side computation
   - Ensure both clients see same state

4. **AI Integration**
   - Help AI prioritize available piece types
   - Avoid wasting time checking unavailable pieces

## Notes

- This feature integrates seamlessly with existing dice mode logic
- No changes needed to server communication protocol
- Compatible with check interruption feature
- Works with both click-move and drag-drop interfaces
- Display updates automatically through existing code paths

## Commit Information

**Commit Hash**: 7ffe1cb  
**Branch**: `copilot/update-dice-display-on-move`  
**Commit Message**: Add dynamic dice display updates to check piece availability

Implement canPieceTypeMove() to check if piece type exists and has legal moves.
Update updateDiceDisplay() to show grayscale when pieces cannot move.
Checks are performed after each move to dynamically update the display.

## Documentation

- This file: `FEATURE_DYNAMIC_DICE_DISPLAY.md`
- Related: `BUGFIX_DICE_CHECK_MOVEMENT.md`
- Related: `IMPLEMENTATION_SUMMARY_DICE_CHECK.md`
- Related: `docs/DICE_MODE_CHECK_RULE.md`

---

**Total Changes**: 2 files modified, 34 lines added  
**Implementation Date**: 2026-01-03
