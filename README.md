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
- **Visual Highlights**: Selected pieces and valid moves are highlighted (orange for normal moves, red for capture moves)
- **Clean UI**: Classic chessboard with Unicode chess piece symbols
- **New Game**: Reset the board to start a new game at any time
- **Sound Effects**: Different sounds for moves, captures, castling, check, and checkmate
- **Customizable Sound Settings**: 
  - Configure custom sound files for each event type
  - Adjust volume levels for each sound independently
  - Enable/disable individual sounds or all sounds
  - Reset individual sound effects to default values independently
  - Preview sounds before applying settings
  - Settings are saved persistently between sessions
- **Customizable Piece Icons**:
  - Choose from multiple icon set options:
    - Default Unicode chess symbols
    - Three preset icon sets (when images are provided)
    - Fully custom user-selected icons
  - **Adjustable Piece Size**: Scale piece icons from 60% to 100% of square size using a convenient slider
  - Configure custom icons for each of the 12 piece types (6 white + 6 black)
  - Support for common image formats (PNG, JPG, SVG, BMP)
  - Preview icons before applying
  - Easy switching between different icon sets
  - Reset to default Unicode symbols at any time
  - Settings are saved persistently between sessions
  - See [ADDING_ICON_SETS.md](ADDING_ICON_SETS.md) for instructions on adding preset icon images
- **Customizable Board Colors**:
  - Choose from multiple color scheme options:
    - Classic brown/beige (經典棕褐色) - Traditional chess board appearance
    - Blue/gray scheme (藍灰配色) - Modern cool-toned look
    - Green/white scheme (綠白配色) - Fresh natural appearance
    - Fully custom colors using color picker
  - **2×2 Preview Grid**: See exactly how your board will look before applying
  - **Preset Thumbnails**: Visual previews of all three preset color schemes
  - Pick any custom colors for light and dark squares
  - Reset to default colors at any time
  - Settings are saved persistently between sessions
  - See [BOARD_COLOR_QUICK_START.md](BOARD_COLOR_QUICK_START.md) for user guide

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
2. **Drag and Drop**: Click and drag a piece to move it to a valid square
3. **Cancel Move**: Right-click at any time during dragging to cancel and return the piece to its original position
4. **Valid Moves**: Valid destination squares will be highlighted (orange for normal moves, red for capture moves)
5. **Move**: Click on a highlighted square to move the piece, or drag and drop
6. **Deselect**: Click the selected piece again or right-click to deselect it
7. **Turn Indicator**: The current player's turn is displayed at the top
8. **Game Status**: Check, checkmate, and stalemate are automatically detected and displayed
9. **New Game**: Click "New Game" button to reset the board
10. **Sound Settings**: Click "設定" (Settings) menu > "音效設定" (Sound Settings) to:
    - Choose custom sound files for each event (move, capture, castling, check, checkmate)
    - Adjust volume levels using sliders
    - Enable/disable individual sounds or all sounds
    - Reset individual sound effects to their default values using the "重設" (Reset) button next to each sound
    - Reset all sound effects at once using the "重設為預設值" (Reset to Defaults) button
    - Preview sounds before applying settings
    - **Supported audio formats**: WAV (recommended), MP3, OGG
      - WAV format is recommended for best compatibility across all platforms
11. **Piece Icon Settings**: Click "設定" (Settings) menu > "棋子圖標設定" (Piece Icon Settings) to:
    - Choose from multiple icon set options:
      - Default Unicode chess symbols (♔, ♕, ♖, etc.)
      - Three preset icon sets (when available)
      - Custom user-selected icons
    - **Adjust piece size**: Use the slider to scale pieces from 60% to 100% of square size (default 80%)
    - Upload custom image files for each piece type (white and black)
    - Preview icons before applying
    - Reset individual piece icons to default Unicode symbols using the "重設" (Reset) button
    - Reset all piece icons at once using the "重設為預設值" (Reset to Defaults) button
    - **Supported image formats**: PNG, JPG, SVG, BMP
      - PNG format with transparent background is recommended for best appearance
12. **Board Color Settings**: Click "設定" (Settings) menu > "棋盤顏色設定" (Board Color Settings) to:
    - Choose from three preset color schemes:
      - 經典棕褐色 (Classic) - Traditional brown and beige colors
      - 藍灰配色 (Blue/Gray) - Modern blue and gray tones
      - 綠白配色 (Green/White) - Fresh green and white combination
    - Create custom color schemes using the color picker
    - See a 2×2 preview grid showing exactly how your board will look
    - View thumbnail previews of all preset color schemes
    - Reset to default colors at any time
    - All color settings are saved automatically

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
- `soundsettingsdialog.h/cpp` - Sound settings dialog for customizing audio
- `pieceiconsettingsdialog.h/cpp` - Piece icon settings dialog for customizing piece graphics
- `boardcolorsettingsdialog.h/cpp` - Board color settings dialog for customizing board colors
- `qt_chess.ui` - Qt UI designer file
- `resources.qrc` - Qt resource file for embedded sound files

## License

This is an educational project for learning Qt and game development.