# Board Flip Feature - Visual Guide

## Feature Overview

The board flip feature (反轉棋盤功能) allows players to rotate the chess board 180 degrees, enabling both players in a face-to-face game to view the board from their own perspective.

## How It Works

### Normal View (Default)
```
  a  b  c  d  e  f  g  h
8 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜  8   <- Black pieces at top
7 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟  7
6 .  .  .  .  .  .  .  . 6
5 .  .  .  .  .  .  .  . 5
4 .  .  .  .  .  .  .  . 4
3 .  .  .  .  .  .  .  . 3
2 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙  2
1 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖  1   <- White pieces at bottom
  a  b  c  d  e  f  g  h
```

### Flipped View (After clicking "反轉棋盤")
```
  h  g  f  e  d  c  b  a
1 ♖ ♘ ♗ ♔ ♕ ♗ ♘ ♖  1   <- White pieces at top (flipped)
2 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙  2
3 .  .  .  .  .  .  .  . 3
4 .  .  .  .  .  .  .  . 4
5 .  .  .  .  .  .  .  . 5
6 .  .  .  .  .  .  .  . 6
7 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟  7
8 ♜ ♞ ♝ ♚ ♛ ♝ ♞ ♜  8   <- Black pieces at bottom (flipped)
  h  g  f  e  d  c  b  a
```

## Key Features

✅ **Instant Toggle**: Board flips immediately when menu item is clicked
✅ **Preserved Gameplay**: All moves work correctly in both orientations
✅ **Correct Highlights**: Selected pieces and valid moves highlight properly
✅ **Drag-and-Drop**: Full drag-and-drop support in flipped mode
✅ **Persistent State**: Board orientation saved between sessions
✅ **Correct Colors**: Checkerboard pattern remains correct when flipped

## Use Cases

1. **Two-Player Local Games**: Perfect when players sit across from each other
2. **Teaching**: Instructor can flip board to match student's perspective
3. **Analysis**: Review games from both players' viewpoints
4. **Personal Preference**: Some players prefer seeing their pieces at the bottom

## Technical Details

### Coordinate Transformation
- **Normal**: Display row/col = Logical row/col
- **Flipped**: Display row/col = 7 - Logical row/col

### What Gets Transformed
- ✅ Piece positions on board
- ✅ Square colors (maintains checkerboard)
- ✅ Click positions
- ✅ Drag-and-drop start/end positions
- ✅ Valid move highlights
- ✅ Check/checkmate indicators
- ✅ Selected piece highlights

### What Stays the Same
- ✅ Game logic (moves, rules, validation)
- ✅ Turn order
- ✅ Move history
- ✅ Special moves (castling, en passant, promotion)

## Accessing the Feature

1. Launch Qt_Chess
2. Click "設定" (Settings) in the menu bar
3. Click "反轉棋盤" (Flip Board)
4. Board rotates instantly
5. Click again to flip back

## Notes

- The logical game state never changes - only the visual representation
- All standard chess rules remain in effect
- Both Unicode symbols and custom piece icons work correctly when flipped
- Coordinates in the code are carefully managed to ensure correctness
