# Qt_Chess Dice Mode Code Documentation
## Comprehensive Technical Presentation

---

## Table of Contents

1. Dice Mode Overview
2. Core Data Structures
3. Main Functions
4. Dice Rolling Process
5. Move Validation Mechanism
6. Display Update System
7. Check Interruption Rule
8. Network Synchronization
9. Code Architecture
10. Practical Examples

---

## 1. Dice Mode Overview

### Feature Description
Dice Mode is a special game mode in Qt_Chess that adds randomness and strategic depth to the game.

**Core Mechanics:**
- Roll 3 dice at the start of each turn
- Each die represents a piece type (King, Queen, Rook, Bishop, Knight, Pawn)
- Players must move each of the 3 piece types once
- Can only move the rolled piece types

**Special Rules:**
- King can only be rolled once per turn (avoid over-reliance on King)
- Check Interruption: When putting opponent in check, they must respond first, then attacker continues remaining moves
- Dynamic Display: Dice show greyscale when pieces don't exist or have no legal moves

---

## 2. Core Data Structures

### 2.1 Class Member Variables (qt_chess.h: 279-294)

```cpp
// Dice Mode Related
bool m_diceModeEnabled;              // Whether dice mode is enabled
std::vector<PieceType> m_rolledPieceTypes;      // Rolled piece types this turn
std::vector<int> m_rolledPieceTypeCounts;       // Remaining moves for each type
QWidget* m_diceDisplayPanel;         // Dice display panel
QLabel* m_diceDisplayTitle;          // Panel title (shows whose turn)
QList<QLabel*> m_diceDisplayLabels;  // Labels showing rolled pieces (3)
int m_diceMovesRemaining;            // Remaining dice moves this turn

// Dice Mode Check Interruption Related
bool m_diceCheckInterrupted;         // Is dice mode interrupted due to check
PieceColor m_diceInterruptedPlayer;  // Color of interrupted player
bool m_diceRespondingToCheck;        // Is current player responding to check
std::vector<PieceType> m_diceSavedPieceTypes;      // Saved dice types before interruption
std::vector<int> m_diceSavedPieceTypeCounts;       // Saved counts before interruption
int m_diceSavedMovesRemaining;       // Saved remaining moves before interruption
```

### 2.2 Data Structure Explanation

| Variable Name | Type | Purpose |
|--------------|------|---------|
| `m_rolledPieceTypes` | `vector<PieceType>` | Stores the 3 rolled piece types |
| `m_rolledPieceTypeCounts` | `vector<int>` | Remaining moves for each type (initially 1) |
| `m_diceMovesRemaining` | `int` | Total remaining moves (initially 3) |
| `m_diceCheckInterrupted` | `bool` | Flag for check interruption |
| `m_diceSavedPieceTypes` | `vector<PieceType>` | Backup of dice state during interruption |

---

## 3. Main Functions

### 3.1 Function List

| Function Name | Lines | Description |
|--------------|-------|-------------|
| `rollDiceForTurn()` | 8933-8971 | Roll 3 piece types for current turn |
| `onDiceRolled()` | 8974-9085 | Process dice results from server |
| `onDiceStateReceived()` | 9088-9149 | Process dice state updates |
| `updateDiceDisplay()` | 9152-9229 | Update dice display panel UI |
| `isPieceTypeInRolledList()` | 9232-9249 | Check if piece type is in rolled list |
| `markPieceTypeAsMoved()` | 9252-9266 | Mark rolled piece type as moved |
| `allRolledPiecesMoved()` | 9269-9297 | Check if all dice are used |
| `canPieceTypeMove()` | 8917-8930 | Check if piece type can move |
| `canRollPiece()` | 9300-9317 | Check if piece at position can be rolled |

---

## 4. Dice Rolling Process

### 4.1 Flow Diagram

```
[Turn Starts] 
    ↓
[rollDiceForTurn()] - Client requests dice
    ↓
[requestDiceRoll()] - Send network request to server
    ↓
[Server generates random numbers]
    ↓
[onDiceRolled()] - Receive server's dice results
    ↓
[Process dice results and update local state]
    ↓
[updateDiceDisplay()] - Update UI display
    ↓
[Player starts moving pieces]
```

### 4.2 Core Code: rollDiceForTurn()

**File:** `src/qt_chess.cpp` (Lines 8933-8971)

```cpp
void Qt_Chess::rollDiceForTurn() {
    if (!m_diceModeEnabled || !m_isOnlineGame) {
        return;  // Only used in online dice mode
    }
    
    PieceColor currentColor = m_chessBoard.getCurrentPlayer();
    qDebug() << "[Qt_Chess::rollDiceForTurn] Rolling dice for" 
             << (currentColor == PieceColor::White ? "White" : "Black");
    
    // Step 1: Get all movable pieces
    std::vector<QPoint> movablePieces = getMovablePieces(currentColor);
    
    if (movablePieces.empty()) {
        // No movable pieces, clear dice state
        m_rolledPieceTypes.clear();
        m_rolledPieceTypeCounts.clear();
        m_diceMovesRemaining = 0;
        updateDiceDisplay();
        return;
    }
    
    // Step 2: Count each piece type
    std::map<PieceType, int> pieceTypeCounts;
    for (const auto& pos : movablePieces) {
        const ChessPiece& piece = m_chessBoard.getPiece(pos.y(), pos.x());
        pieceTypeCounts[piece.getType()]++;
    }
    
    // Step 3: Create list of available piece types
    std::vector<PieceType> availableTypes;
    for (const auto& pair : pieceTypeCounts) {
        availableTypes.push_back(pair.first);
    }
    
    // Step 4: Request server to generate dice
    if (m_networkManager) {
        m_networkManager->requestDiceRoll(
            static_cast<int>(availableTypes.size())
        );
    }
}
```

---

## 5. Move Validation Mechanism

### 5.1 Validation Flow

```
[Player attempts to move piece]
    ↓
[isPieceTypeInRolledList()] - Check if piece type is in rolled list
    ↓
    YES → [Allow move] → [markPieceTypeAsMoved()] - Mark as moved
    ↓                        ↓
    NO → [Reject move]    [Decrease counter] → [updateDiceDisplay()]
```

### 5.2 Core Code: isPieceTypeInRolledList()

**File:** `src/qt_chess.cpp` (Lines 9232-9249)

```cpp
bool Qt_Chess::isPieceTypeInRolledList(PieceType type) const {
    // If dice mode not enabled, all pieces can move
    if (!m_diceModeEnabled) {
        return true;
    }
    
    // If player is responding to check, allow moving any piece
    if (m_diceRespondingToCheck) {
        return true;
    }
    
    // Check if type is in dice list with remaining moves
    for (size_t i = 0; i < m_rolledPieceTypes.size(); ++i) {
        if (m_rolledPieceTypes[i] == type && 
            m_rolledPieceTypeCounts[i] > 0) {
            return true;
        }
    }
    
    return false;  // Not in list or used up
}
```

### 5.3 Core Code: markPieceTypeAsMoved()

**File:** `src/qt_chess.cpp` (Lines 9252-9266)

```cpp
void Qt_Chess::markPieceTypeAsMoved(PieceType type) {
    if (!m_diceModeEnabled) {
        return;
    }
    
    // Find corresponding die and decrease count
    for (size_t i = 0; i < m_rolledPieceTypes.size(); ++i) {
        if (m_rolledPieceTypes[i] == type && 
            m_rolledPieceTypeCounts[i] > 0) {
            m_rolledPieceTypeCounts[i]--;  // Decrease type count
            m_diceMovesRemaining--;        // Decrease total moves
            
            qDebug() << "[markPieceTypeAsMoved] Marked dice" << (i + 1) 
                     << "as moved. Remaining:" << m_diceMovesRemaining;
            
            updateDiceDisplay();  // Update UI (may turn grey)
            return;
        }
    }
}
```

---

## 6. Display Update System

### 6.1 UI Components

Dice display panel includes:
- **Title Label** (`m_diceDisplayTitle`): Shows "🎲 My Turn" or "⏸️ Opponent's Turn"
- **3 Dice Labels** (`m_diceDisplayLabels`): Show rolled piece types

### 6.2 Display States

| State | Appearance | Condition |
|-------|-----------|-----------|
| **Colored (Available)** | Blue gradient background | Remaining moves > 0 AND piece type can move |
| **Greyscale (Unavailable)** | Grey background | Remaining moves = 0 OR piece type cannot move |
| **Empty** | Shows "--" | No piece rolled for this die position |

### 6.3 Core Code: updateDiceDisplay()

**File:** `src/qt_chess.cpp` (Lines 9152-9229)

```cpp
void Qt_Chess::updateDiceDisplay() {
    if (!m_diceDisplayPanel || m_diceDisplayLabels.isEmpty()) {
        return;
    }
    
    // Show panel when dice mode is enabled
    if (m_diceModeEnabled && m_isOnlineGame) {
        m_diceDisplayPanel->show();
        
        // Update turn indicator
        if (m_diceDisplayTitle) {
            QString turnText = isOnlineTurn() ? 
                "🎲 My Turn" : "⏸️ Opponent's Turn";
            m_diceDisplayTitle->setText(turnText);
        }
        
        // Update each dice label
        for (int i = 0; i < 3 && i < m_diceDisplayLabels.size(); ++i) {
            QLabel* label = m_diceDisplayLabels[i];
            
            if (i < static_cast<int>(m_rolledPieceTypes.size())) {
                PieceType type = m_rolledPieceTypes[i];
                int remainingMoves = m_rolledPieceTypeCounts[i];
                
                // Get piece type name
                QString pieceTypeName;
                switch (type) {
                    case PieceType::King:   pieceTypeName = "King"; break;
                    case PieceType::Queen:  pieceTypeName = "Queen"; break;
                    case PieceType::Rook:   pieceTypeName = "Rook"; break;
                    case PieceType::Bishop: pieceTypeName = "Bishop"; break;
                    case PieceType::Knight: pieceTypeName = "Knight"; break;
                    case PieceType::Pawn:   pieceTypeName = "Pawn"; break;
                    default: pieceTypeName = "?"; break;
                }
                
                label->setText(pieceTypeName);
                
                // Determine if should display in greyscale
                PieceColor diceOwnerColor = m_chessBoard.getCurrentPlayer();
                bool canMove = canPieceTypeMove(type, diceOwnerColor);
                
                if (remainingMoves <= 0 || !canMove) {
                    // Greyscale style
                    label->setStyleSheet(
                        "QLabel { "
                        "  background: qlineargradient(...); "
                        "  color: #808080; "
                        "  border: 2px solid #606060; "
                        "}"
                    );
                } else {
                    // Colored style (available)
                    label->setStyleSheet(
                        "QLabel { "
                        "  background: qlineargradient(...); "
                        "  color: #2196F3; "
                        "  border: 2px solid #2196F3; "
                        "}"
                    );
                }
            } else {
                label->setText("--");  // Empty die position
            }
        }
    } else {
        m_diceDisplayPanel->hide();  // Hide when not in dice mode
    }
}
```

---

## 7. Check Interruption Rule

### 7.1 Rule Description

When a player puts opponent's king in check (but not checkmate) during their dice turn:
1. System saves current player's remaining dice moves
2. Turn immediately switches to opponent
3. Opponent must move first to escape check
4. After opponent moves, turn automatically switches back to interrupted player
5. Interrupted player can continue with remaining dice moves

### 7.2 State Save & Restore Flow

```
[Player moves piece] 
    ↓
[Detect: Is opponent in check?] 
    ↓ YES (and not checkmate, and has remaining dice)
[Save dice state]
    - m_diceSavedPieceTypes = m_rolledPieceTypes
    - m_diceSavedMovesRemaining = m_diceMovesRemaining
    ↓
[Clear current dice state]
    - m_rolledPieceTypes.clear()
    - m_diceMovesRemaining = 0
    ↓
[Set interruption flag]
    - m_diceCheckInterrupted = true
    - m_diceInterruptedPlayer = currentPlayer
    ↓
[Switch turn to opponent] - Opponent responds to check
    ↓
[Opponent moves to escape check]
    ↓
[onOpponentMove() detects interruption restore]
    ↓
[Restore dice state]
    - m_rolledPieceTypes = m_diceSavedPieceTypes
    - m_diceMovesRemaining = m_diceSavedMovesRemaining
    ↓
[Switch turn back to interrupted player]
    ↓
[Interrupted player continues moving remaining dice]
```

---

## 8. Network Synchronization

### 8.1 Network Communication Flow

```
[Client A] ─────────> [Server] <───────── [Client B]
    │                     │                   │
    │ requestDiceRoll()   │                   │
    ├──────────────────> │                   │
    │                     │ Generate random   │
    │                     ├──────────────────>│
    │ <───────────────────┤ diceRolled msg    │
    │   onDiceRolled()    │                   │
    │                     │                   │
    │ sendMove()          │                   │
    ├──────────────────> │                   │
    │                     ├──────────────────>│
    │                     │ move message      │
    │                     │ onOpponentMove()  │
```

### 8.2 Key Network Functions

#### requestDiceRoll() - Request Dice

**File:** `src/networkmanager.cpp` (Lines 281-294)

```cpp
void NetworkManager::requestDiceRoll(int numMovablePieces) {
    if (m_roomNumber.isEmpty()) {
        return;
    }
    
    QJsonObject message;
    message["type"] = "requestDiceRoll";
    message["room"] = m_roomNumber;
    message["numPieces"] = numMovablePieces;
    
    sendMessage(message);
}
```

---

## 9. Code Architecture

```
┌─────────────────────────────────────────────┐
│           Qt_Chess Main Class               │
│          (qt_chess.h/cpp)                   │
└─────────────────────────────────────────────┘
                    │
    ┌───────────────┼───────────────┐
    │               │               │
    ▼               ▼               ▼
┌─────────┐   ┌──────────┐   ┌─────────┐
│  Dice   │   │   Move   │   │   UI    │
│  State  │   │Validation│   │ Display │
│         │   │          │   │         │
│• Variables│ │• Check   │   │• Panel  │
│• Save    │  │• Mark    │   │• Grey   │
│• Restore │  │• Verify  │   │• Style  │
└─────────┘   └──────────┘   └─────────┘
                    │
                    ▼
        ┌──────────────────┐
        │   ChessBoard     │
        │ (chessboard.cpp) │
        │                  │
        │• isValidMove()   │
        │• isInCheck()     │
        │• isCheckmate()   │
        └──────────────────┘
```

---

## 10. Practical Examples

### Example 1: Normal Dice Turn

```
Turn starts: White
├─ rollDiceForTurn()
├─ Server returns dice: [Knight, Rook, Pawn]
├─ onDiceRolled() processes result
├─ updateDiceDisplay() shows: Knight(Blue) Rook(Blue) Pawn(Blue)
│
├─ Player moves Knight
│   ├─ isPieceTypeInRolledList(Knight) → true ✓
│   ├─ movePiece() executes move
│   ├─ markPieceTypeAsMoved(Knight)
│   └─ updateDiceDisplay() shows: Knight(Grey) Rook(Blue) Pawn(Blue)
│
├─ Player moves Rook
│   └─ Display: Knight(Grey) Rook(Grey) Pawn(Blue)
│
├─ Player moves Pawn
│   └─ Display: Knight(Grey) Rook(Grey) Pawn(Grey)
│
└─ Switch turn to Black
```

### Example 2: Check Interruption

```
Turn starts: White
├─ Dice: [Queen, Bishop, Knight]
├─ Display: Queen(Blue) Bishop(Blue) Knight(Blue)
│
├─ Player moves Queen
│   ├─ movePiece() executes
│   ├─ Black King in check! (not checkmate)
│   ├─ markPieceTypeAsMoved(Queen)
│   ├─ Display: Queen(Grey) Bishop(Blue) Knight(Blue)
│   │
│   ├─ Detect interruption conditions:
│   │   ├─ opponentInCheck = true ✓
│   │   ├─ opponentInCheckmate = false ✓
│   │   └─ m_diceMovesRemaining - 1 = 2 > 0 ✓
│   │
│   └─ Send move with interruption flag
│
├─ Switch to Black (respond to check)
│   ├─ Black moves King to escape
│   └─ Send move to server
│
├─ onOpponentMove() receives Black's move
│   ├─ Detect: m_diceCheckInterrupted = true
│   ├─ Detect: stillInCheck = false (check resolved)
│   ├─ Restore White's dice state
│   ├─ Switch turn back to White
│   └─ Display: Queen(Grey) Bishop(Blue) Knight(Blue)
│
└─ White continues with Bishop and Knight
```

---

## Summary

### Key Features

1. **Complete State Management**
   - Vector-based storage for dice types and counts
   - Clear interruption save/restore mechanism
   - Multi-level validation checks

2. **Dynamic UI Updates**
   - Real-time piece state reflection
   - Automatic greyscale for unavailable dice
   - Beautiful gradient styles

3. **Network Synchronization**
   - Server-generated random dice for fairness
   - Bidirectional state sync
   - Interruption info transmission support

4. **Check Interruption Rule**
   - Complex logic handling
   - State save and restore
   - Ensures game fairness

### Code Statistics

| Item | Count |
|------|-------|
| Core Functions | 9 |
| State Variables | 13 |
| Total Lines of Code | ~600+ |
| Documentation Files | 4 MD files |

### Technical Highlights

✅ Modular Design  
✅ Clear Separation of Concerns  
✅ Comprehensive Error Handling  
✅ Detailed Debug Logging  
✅ Elegant UI/UX  
✅ Extensible Architecture  

---

*Document Date: 2026-01-04*  
*Version: 1.0*
