# Qt_Chess

A fully functional player-vs-player chess game built with Qt5.

## Features

- **Complete Chess Implementation**: All pieces (King, Queen, Rook, Bishop, Knight, Pawn) with proper movement rules
- **Special Moves**: 
  - **Castling** (王車易位): King can castle with rook (kingside or queenside) under appropriate conditions
  - **Pawn Promotion** (兵升變): Pawns are promoted when reaching the opposite end of the board, with a dialog to choose the promotion piece (Queen, Rook, Bishop, or Knight)
  - **En Passant** (吃過路兵): Special pawn capture when an opponent's pawn moves two squares forward
- **Turn-based Gameplay**: Alternating turns between White and Black players
- **Move Validation**: Only legal moves are allowed
- **Check Detection**: The game detects when a king is in check
- **Checkmate & Stalemate Detection**: Game automatically detects end conditions
- **Visual Highlights**: Selected pieces and valid moves are highlighted
- **Clean UI**: Classic chessboard with Unicode chess piece symbols
- **New Game**: Reset the board to start a new game at any time

## Building

### Requirements
- Qt5 (qt5-qtbase-devel or qtbase5-dev)
- C++17 compatible compiler
- qmake

### Build Instructions
```bash
qmake Qt_Chess.pro
make
```

### Run
```bash
./Qt_Chess
```

## How to Play

1. **Select a Piece**: Click on one of your pieces (White starts first)
2. **Valid Moves**: Valid destination squares will be highlighted in orange
3. **Move**: Click on a highlighted square to move the piece
4. **Deselect**: Click the selected piece again to deselect it
5. **Turn Indicator**: The current player's turn is displayed at the top
6. **Game Status**: Check, checkmate, and stalemate are automatically detected and displayed
7. **New Game**: Click "New Game" button to reset the board

## Game Rules

- White always moves first
- Players alternate turns
- Each piece moves according to standard chess rules:
  - **Pawn**: Moves forward one square (two squares on first move), captures diagonally
  - **Rook**: Moves horizontally or vertically any number of squares
  - **Knight**: Moves in an "L" shape (2 squares in one direction, 1 square perpendicular)
  - **Bishop**: Moves diagonally any number of squares
  - **Queen**: Moves horizontally, vertically, or diagonally any number of squares
  - **King**: Moves one square in any direction
- **Special Moves**:
  - **Castling**: King and rook can castle if neither piece has moved, there are no pieces between them, the king is not in check, and the king doesn't move through or into check
  - **Pawn Promotion**: When a pawn reaches the opposite end of the board (row 0 for White, row 7 for Black), it must be promoted to a Queen, Rook, Bishop, or Knight
  - **En Passant**: If an opponent's pawn moves two squares forward from its starting position and lands beside your pawn, you can capture it "in passing" on your next move
- You cannot move into check
- The game ends when checkmate or stalemate is reached

## Project Structure

- `main.cpp` - Application entry point
- `qt_chess.h/cpp` - Main window and UI logic
- `chessboard.h/cpp` - Game board logic and rules
- `chesspiece.h/cpp` - Chess piece definitions and movement validation
- `qt_chess.ui` - Qt UI designer file

## License

This is an educational project for learning Qt and game development.