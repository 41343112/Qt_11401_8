# Code Location and Description Guide

This document explains the location and functionality of the "Settings" and "Chess Piece Judgment" features in the chess game.

## Table of Contents

1. [Settings Features](#settings-features)
   - [Sound Settings](#sound-settings)
   - [Piece Icon Settings](#piece-icon-settings)
   - [Board Color Settings](#board-color-settings)
2. [Chess Piece Judgment Code](#chess-piece-judgment-code)
   - [Piece Class Definition](#piece-class-definition)
   - [Move Validation Logic](#move-validation-logic)
   - [Board Game Rules](#board-game-rules)

---

## Settings Features

Settings features allow players to customize the game's appearance and sound effects. There are three main settings dialogs:

### Sound Settings

**File Location:**
- Header file: `src/soundsettingsdialog.h`
- Implementation file: `src/soundsettingsdialog.cpp`

**Key Functionality:**

The `SoundSettingsDialog` class provides a dialog that allows users to customize various sound effects in the game.

**Code Structure:**

```cpp
class SoundSettingsDialog : public QDialog
{
    struct SoundSettings {
        QString moveSound;           // Move sound file path
        QString captureSound;        // Capture sound file path
        QString castlingSound;       // Castling sound file path
        QString checkSound;          // Check sound file path
        QString checkmateSound;      // Checkmate sound file path
        double moveVolume;           // Move sound volume (0.0-1.0)
        double captureVolume;        // Capture sound volume
        double castlingVolume;       // Castling sound volume
        double checkVolume;          // Check sound volume
        double checkmateVolume;      // Checkmate sound volume
        bool moveSoundEnabled;       // Enable move sound
        bool captureSoundEnabled;    // Enable capture sound
        bool castlingSoundEnabled;   // Enable castling sound
        bool checkSoundEnabled;      // Enable check sound
        bool checkmateSoundEnabled;  // Enable checkmate sound
        bool allSoundsEnabled;       // Enable all sounds
    };
    
    SoundSettings getSettings() const;
    void setSettings(const SoundSettings& settings);
    static SoundSettings getDefaultSettings();
};
```

**Key Methods:**
- `getSettings()`: Get current sound settings
- `setSettings()`: Apply saved settings
- `getDefaultSettings()`: Provide default sound settings
- Browse button slots (e.g., `onBrowseMove()`): Let users select custom sound files
- Preview button slots (e.g., `onPreviewMove()`): Play sounds for preview
- Reset button slots (e.g., `onResetMove()`): Reset individual sounds to defaults

---

### Piece Icon Settings

**File Location:**
- Header file: `src/pieceiconsettingsdialog.h`
- Implementation file: `src/pieceiconsettingsdialog.cpp`

**Key Functionality:**

The `PieceIconSettingsDialog` class provides a dialog for customizing chess piece icons.

**Code Structure:**

```cpp
class PieceIconSettingsDialog : public QDialog
{
    enum class IconSetType {
        Unicode,      // Default Unicode symbols (♔, ♕, ♖, etc.)
        Preset1,      // Preset icon set 1
        Preset2,      // Preset icon set 2
        Preset3,      // Preset icon set 3
        Custom        // Custom user-selected icons
    };

    struct PieceIconSettings {
        QString whiteKingIcon;     // White king icon path
        QString whiteQueenIcon;    // White queen icon path
        QString whiteRookIcon;     // White rook icon path
        QString whiteBishopIcon;   // White bishop icon path
        QString whiteKnightIcon;   // White knight icon path
        QString whitePawnIcon;     // White pawn icon path
        QString blackKingIcon;     // Black king icon path
        QString blackQueenIcon;    // Black queen icon path
        QString blackRookIcon;     // Black rook icon path
        QString blackBishopIcon;   // Black bishop icon path
        QString blackKnightIcon;   // Black knight icon path
        QString blackPawnIcon;     // Black pawn icon path
        bool useCustomIcons;       // Use custom icons
        IconSetType iconSetType;   // Icon set type
        int pieceScale;            // Piece scale percentage (60-100)
    };
};
```

**Key Features:**
- Icon set selection: Choose from Unicode symbols, preset icon sets, or fully custom
- Individual piece customization: Set icons for all 12 piece types (6 white + 6 black)
- Scaling functionality: Adjust piece size (60%-100%) using a slider
- Preview functionality: Preview icons before applying
- Save/Load: Save custom icon sets and load them later

---

### Board Color Settings

**File Location:**
- Header file: `src/boardcolorsettingsdialog.h`
- Implementation file: `src/boardcolorsettingsdialog.cpp`

**Key Functionality:**

The `BoardColorSettingsDialog` class provides a dialog for customizing board colors.

**Code Structure:**

```cpp
class BoardColorSettingsDialog : public QDialog
{
    enum class ColorScheme {
        Classic,      // Classic brown/beige (default)
        BlueGray,     // Blue and gray
        GreenWhite,   // Green and white
        PurplePink,   // Purple and pink
        WoodDark,     // Wood and dark colors
        OceanBlue,    // Ocean blue
        LightTheme,   // Light pastel colors
        Custom1,      // Custom user-selected color slot 1
        Custom2,      // Custom user-selected color slot 2
        Custom3,      // Custom user-selected color slot 3
        Custom4,      // Custom user-selected color slot 4
        Custom5,      // Custom user-selected color slot 5
        Custom6,      // Custom user-selected color slot 6
        Custom7       // Custom user-selected color slot 7
    };

    struct BoardColorSettings {
        QColor lightSquareColor;  // Light square color
        QColor darkSquareColor;   // Dark square color
        ColorScheme scheme;       // Color scheme
        QString customName;       // Custom scheme name
    };
};
```

**Key Features:**
- Preset color schemes: 7 preset schemes (Classic, Blue-Gray, Green-White, Purple-Pink, Wood-Dark, Ocean-Blue, Light)
- Custom colors: Use color picker to select any color
- 2×2 preview grid: Real-time display of board color effects
- Thumbnail previews: Visual previews of all preset color schemes
- Custom slots: Save up to 7 custom color schemes

---

## Chess Piece Judgment Code

Chess piece judgment is responsible for checking if piece moves are legal and enforcing chess game rules.

### Piece Class Definition

**File Location:**
- Header file: `src/chesspiece.h`
- Implementation file: `src/chesspiece.cpp`

**Key Functionality:**

The `ChessPiece` class defines piece types, colors, and movement rules.

**Code Structure:**

```cpp
// Piece type enumeration
enum class PieceType {
    None,     // No piece (empty square)
    Pawn,     // Pawn
    Rook,     // Rook
    Knight,   // Knight
    Bishop,   // Bishop
    Queen,    // Queen
    King      // King
};

// Piece color enumeration
enum class PieceColor {
    None,     // No color (empty square)
    White,    // White
    Black     // Black
};

class ChessPiece {
public:
    ChessPiece(PieceType type = PieceType::None, PieceColor color = PieceColor::None);
    
    PieceType getType() const;      // Get piece type
    PieceColor getColor() const;    // Get piece color
    bool hasMoved() const;          // Has the piece moved
    void setMoved(bool moved);      // Set moved flag
    
    QString getSymbol() const;      // Get Unicode symbol (♔, ♕, etc.)
    
    // Check if move to target position is valid (basic piece movement rules)
    bool isValidMove(const QPoint& from, const QPoint& to, 
                     const std::vector<std::vector<ChessPiece>>& board,
                     const QPoint& enPassantTarget = QPoint(-1, -1)) const;
    
private:
    PieceType m_type;    // Piece type
    PieceColor m_color;  // Piece color
    bool m_hasMoved;     // Has moved (for castling and pawn double move)
    
    // Movement validation functions for each piece type
    bool isValidPawnMove(...) const;
    bool isValidRookMove(...) const;
    bool isValidKnightMove(...) const;
    bool isValidBishopMove(...) const;
    bool isValidQueenMove(...) const;
    bool isValidKingMove(...) const;
    bool isPathClear(...) const;
};
```

---

### Move Validation Logic

**Core Validation Method: `isValidMove()`**

This method is the core of piece move validation, located in `src/chesspiece.cpp`.

**Movement Rules for Each Piece:**

#### 1. Pawn Movement Rules
- Can only move forward
- First move can be 1 or 2 squares
- Captures diagonally only
- Supports en passant special rule

#### 2. Rook Movement Rules
- Moves horizontally or vertically only
- Path must be clear

#### 3. Knight Movement Rules
- Moves in an "L" shape (2+1 or 1+2)
- Can jump over other pieces

#### 4. Bishop Movement Rules
- Moves diagonally only
- Path must be clear

#### 5. Queen Movement Rules
- Combines rook and bishop movements
- Can move horizontally, vertically, or diagonally

#### 6. King Movement Rules
- Can move one square in any direction
- Castling is handled by the `ChessBoard` class

---

### Board Game Rules

**File Location:**
- Header file: `src/chessboard.h`
- Implementation file: `src/chessboard.cpp`

**Key Functionality:**

The `ChessBoard` class manages the entire board state and game rules.

**Code Structure:**

```cpp
class ChessBoard {
public:
    ChessBoard();
    
    void initializeBoard();  // Initialize board to starting state
    
    // Piece access
    const ChessPiece& getPiece(int row, int col) const;
    ChessPiece& getPiece(int row, int col);
    void setPiece(int row, int col, const ChessPiece& piece);
    
    // Movement and validation
    bool movePiece(const QPoint& from, const QPoint& to);
    bool isValidMove(const QPoint& from, const QPoint& to) const;
    
    // Game state queries
    PieceColor getCurrentPlayer() const;
    void setCurrentPlayer(PieceColor player);
    bool isInCheck(PieceColor color) const;        // Is in check
    bool isCheckmate(PieceColor color) const;      // Is checkmate
    bool isStalemate(PieceColor color) const;      // Is stalemate
    bool isInsufficientMaterial() const;           // Insufficient material
    
    QPoint findKing(PieceColor color) const;       // Find king position
    QPoint getEnPassantTarget() const;
    
    // Promotion
    bool needsPromotion(const QPoint& to) const;
    void promotePawn(const QPoint& pos, PieceType newType);
    
private:
    std::vector<std::vector<ChessPiece>> m_board;  // 8x8 board
    PieceColor m_currentPlayer;                    // Current player
    QPoint m_enPassantTarget;                      // En passant target
    std::vector<MoveRecord> m_moveHistory;         // Move history
    GameResult m_gameResult;                       // Game result
    
    void switchPlayer();
    bool wouldBeInCheck(...) const;
    bool hasAnyValidMoves(PieceColor color) const;
    bool canPieceMove(const QPoint& pos) const;
    bool canCastle(const QPoint& from, const QPoint& to) const;
};
```

**Key Methods:**

#### 1. `isValidMove()` - Complete Move Validation
Checks not only basic piece movement rules but also advanced rules like:
- Basic boundary checks
- Checking if it's the correct player's turn
- Checking piece-specific movement rules
- Special case: castling
- Ensuring the move doesn't put own king in check

#### 2. `isInCheck()` - Check Detection
- Iterates through all opponent pieces
- Checks if any piece can capture the king
- Used to validate if moves are legal

#### 3. `isCheckmate()` - Checkmate Detection
- Must be in check
- No legal moves available to escape check
- Game over, opponent wins

#### 4. `isStalemate()` - Stalemate Detection
- Not in check
- But no legal moves available
- Game over, draw

#### 5. `canCastle()` - Castling Check
Castling is a special move requiring multiple conditions:
- Must be a king
- King and rook must not have moved
- King must not be in check
- Path between king and rook must be clear
- King cannot pass through or land in check

---

## Summary

### Settings Features
- **Sound Settings**: `src/soundsettingsdialog.h/cpp` - Manages game sound customization and playback
- **Piece Icon Settings**: `src/pieceiconsettingsdialog.h/cpp` - Manages chess piece icon appearance
- **Board Color Settings**: `src/boardcolorsettingsdialog.h/cpp` - Manages board color schemes

### Chess Piece Judgment Features
- **Piece Class**: `src/chesspiece.h/cpp` - Defines piece types and basic movement rules
- **Board Class**: `src/chessboard.h/cpp` - Manages game state and advanced rules (check, checkmate, castling, etc.)

These classes together implement a fully-functional chess game with all standard rules and rich customization options.
