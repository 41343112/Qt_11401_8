# 遊戲規則程式碼說明 (Game Rules Code Documentation)

## 目錄 (Table of Contents)

1. [概述 (Overview)](#概述-overview)
2. [棋盤架構 (Board Architecture)](#棋盤架構-board-architecture)
3. [標準西洋棋規則 (Standard Chess Rules)](#標準西洋棋規則-standard-chess-rules)
4. [特殊走法 (Special Moves)](#特殊走法-special-moves)
5. [將軍與將死 (Check and Checkmate)](#將軍與將死-check-and-checkmate)
6. [遊戲模式 (Game Modes)](#遊戲模式-game-modes)
7. [棋譜記錄 (Move Recording)](#棋譜記錄-move-recording)
8. [遊戲結果 (Game Results)](#遊戲結果-game-results)

---

## 概述 (Overview)

### 中文說明

Qt_Chess 的遊戲規則實現在 `ChessBoard` 類別中（`src/chessboard.h` 和 `src/chessboard.cpp`）。此類別負責：

- 棋盤狀態管理
- 移動驗證
- 規則執行
- 將軍/將死偵測
- 特殊走法（王車易位、吃過路兵、兵升變）
- 遊戲模式（霧戰、骰子、地雷）
- 棋譜記錄

### English Description

Qt_Chess game rules are implemented in the `ChessBoard` class (`src/chessboard.h` and `src/chessboard.cpp`). This class is responsible for:

- Board state management
- Move validation
- Rule enforcement
- Check/checkmate detection
- Special moves (castling, en passant, pawn promotion)
- Game modes (fog of war, dice, bomb chess)
- Move recording

---

## 棋盤架構 (Board Architecture)

### 資料結構 (Data Structures)

#### 1. 棋盤表示 (Board Representation)

```cpp
class ChessBoard {
private:
    std::vector<std::vector<ChessPiece>> m_board;  // 8x8 棋盤
    PieceColor m_currentPlayer;                     // 當前玩家
    QPoint m_enPassantTarget;                       // 吃過路兵目標位置
    std::vector<MoveRecord> m_moveHistory;          // 移動歷史
    GameResult m_gameResult;                        // 遊戲結果
};
```

**棋盤座標系統**：
- Row 0 = 黑方底線（第 8 橫列）
- Row 7 = 白方底線（第 1 橫列）
- Col 0-7 = A-H 縱列

#### 2. 棋子定義 (Piece Definition)

```cpp
enum class PieceType {
    None,    // 空格
    King,    // 國王
    Queen,   // 皇后
    Rook,    // 城堡
    Bishop,  // 主教
    Knight,  // 騎士
    Pawn     // 兵
};

enum class PieceColor {
    None,    // 無
    White,   // 白方
    Black    // 黑方
};

class ChessPiece {
private:
    PieceType m_type;
    PieceColor m_color;
    bool m_hasMoved;  // 是否已移動（用於王車易位）
};
```

#### 3. 移動記錄 (Move Record)

```cpp
struct MoveRecord {
    QPoint from;                    // 起始位置
    QPoint to;                      // 目標位置
    PieceType pieceType;            // 棋子類型
    PieceColor pieceColor;          // 棋子顏色
    bool isCapture;                 // 是否吃子
    bool isCastling;                // 是否王車易位
    bool isEnPassant;               // 是否吃過路兵
    bool isPromotion;               // 是否兵升變
    PieceType promotionType;        // 升變類型
    bool isCheck;                   // 是否將軍
    bool isCheckmate;               // 是否將死
    QString algebraicNotation;      // 代數記譜法
};
```

### 初始化 (Initialization)

```cpp
void ChessBoard::initializeBoard() {
    // 清空棋盤
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_board[row][col] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
    
    // 設置黑色棋子（Row 0-1）
    m_board[0][0] = ChessPiece(PieceType::Rook, PieceColor::Black);
    m_board[0][1] = ChessPiece(PieceType::Knight, PieceColor::Black);
    m_board[0][2] = ChessPiece(PieceType::Bishop, PieceColor::Black);
    m_board[0][3] = ChessPiece(PieceType::Queen, PieceColor::Black);
    m_board[0][4] = ChessPiece(PieceType::King, PieceColor::Black);
    m_board[0][5] = ChessPiece(PieceType::Bishop, PieceColor::Black);
    m_board[0][6] = ChessPiece(PieceType::Knight, PieceColor::Black);
    m_board[0][7] = ChessPiece(PieceType::Rook, PieceColor::Black);
    
    for (int col = 0; col < 8; ++col) {
        m_board[1][col] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    }
    
    // 設置白色棋子（Row 6-7）
    for (int col = 0; col < 8; ++col) {
        m_board[6][col] = ChessPiece(PieceType::Pawn, PieceColor::White);
    }
    
    m_board[7][0] = ChessPiece(PieceType::Rook, PieceColor::White);
    m_board[7][1] = ChessPiece(PieceType::Knight, PieceColor::White);
    m_board[7][2] = ChessPiece(PieceType::Bishop, PieceColor::White);
    m_board[7][3] = ChessPiece(PieceType::Queen, PieceColor::White);
    m_board[7][4] = ChessPiece(PieceType::King, PieceColor::White);
    m_board[7][5] = ChessPiece(PieceType::Bishop, PieceColor::White);
    m_board[7][6] = ChessPiece(PieceType::Knight, PieceColor::White);
    m_board[7][7] = ChessPiece(PieceType::Rook, PieceColor::White);
    
    // 初始化狀態
    m_currentPlayer = PieceColor::White;
    m_enPassantTarget = QPoint(-1, -1);
    m_moveHistory.clear();
    m_gameResult = GameResult::InProgress;
}
```

---

## 標準西洋棋規則 (Standard Chess Rules)

### 移動驗證 (Move Validation)

主要驗證函數：

```cpp
bool ChessBoard::isValidMove(const QPoint& from, const QPoint& to) const {
    // 1. 邊界檢查
    if (from.x() < 0 || from.x() >= 8 || from.y() < 0 || from.y() >= 8 ||
        to.x() < 0 || to.x() >= 8 || to.y() < 0 || to.y() >= 8) {
        return false;
    }
    
    // 2. 起始位置必須有棋子
    const ChessPiece& piece = getPiece(from.y(), from.x());
    if (piece.getType() == PieceType::None) {
        return false;
    }
    
    // 3. 必須是當前玩家的棋子
    if (piece.getColor() != m_currentPlayer) {
        return false;
    }
    
    // 4. 不能吃自己的棋子
    const ChessPiece& targetPiece = getPiece(to.y(), to.x());
    if (targetPiece.getColor() == m_currentPlayer) {
        return false;
    }
    
    // 5. 檢查棋子類型的移動規則
    if (!piece.isValidMove(from, to, targetPiece.getType() != PieceType::None)) {
        return false;
    }
    
    // 6. 檢查路徑是否有阻擋（除了騎士）
    if (piece.getType() != PieceType::Knight) {
        if (!isPathClear(from, to)) {
            return false;
        }
    }
    
    // 7. 檢查是否會導致自己被將軍
    if (wouldBeInCheck(from, to, m_currentPlayer)) {
        return false;
    }
    
    return true;
}
```

### 各棋子移動規則 (Piece Movement Rules)

#### 1. 兵 (Pawn)

```cpp
bool ChessPiece::isValidPawnMove(const QPoint& from, const QPoint& to, bool isCapture) const {
    int direction = (m_color == PieceColor::White) ? -1 : 1;  // 白方往上，黑方往下
    int startRow = (m_color == PieceColor::White) ? 6 : 1;
    
    int rowDiff = to.y() - from.y();
    int colDiff = to.x() - from.x();
    
    // 向前一格
    if (colDiff == 0 && rowDiff == direction && !isCapture) {
        return true;
    }
    
    // 首次移動可前進兩格
    if (colDiff == 0 && rowDiff == 2 * direction && 
        from.y() == startRow && !isCapture) {
        return true;
    }
    
    // 斜向吃子
    if (abs(colDiff) == 1 && rowDiff == direction && isCapture) {
        return true;
    }
    
    return false;
}
```

#### 2. 城堡 (Rook)

```cpp
bool ChessPiece::isValidRookMove(const QPoint& from, const QPoint& to) const {
    // 必須在同一行或同一列
    return (from.x() == to.x() || from.y() == to.y());
}
```

#### 3. 騎士 (Knight)

```cpp
bool ChessPiece::isValidKnightMove(const QPoint& from, const QPoint& to) const {
    int rowDiff = abs(to.y() - from.y());
    int colDiff = abs(to.x() - from.x());
    
    // L形移動：2+1 或 1+2
    return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
}
```

#### 4. 主教 (Bishop)

```cpp
bool ChessPiece::isValidBishopMove(const QPoint& from, const QPoint& to) const {
    int rowDiff = abs(to.y() - from.y());
    int colDiff = abs(to.x() - from.x());
    
    // 必須在對角線上
    return rowDiff == colDiff && rowDiff > 0;
}
```

#### 5. 皇后 (Queen)

```cpp
bool ChessPiece::isValidQueenMove(const QPoint& from, const QPoint& to) const {
    // 皇后 = 城堡 + 主教
    return isValidRookMove(from, to) || isValidBishopMove(from, to);
}
```

#### 6. 國王 (King)

```cpp
bool ChessPiece::isValidKingMove(const QPoint& from, const QPoint& to) const {
    int rowDiff = abs(to.y() - from.y());
    int colDiff = abs(to.x() - from.x());
    
    // 只能移動一格（除了王車易位）
    return rowDiff <= 1 && colDiff <= 1 && (rowDiff + colDiff > 0);
}
```

### 路徑檢查 (Path Checking)

```cpp
bool ChessBoard::isPathClear(const QPoint& from, const QPoint& to) const {
    int rowStep = (to.y() > from.y()) ? 1 : (to.y() < from.y()) ? -1 : 0;
    int colStep = (to.x() > from.x()) ? 1 : (to.x() < from.x()) ? -1 : 0;
    
    int row = from.y() + rowStep;
    int col = from.x() + colStep;
    
    while (row != to.y() || col != to.x()) {
        if (getPiece(row, col).getType() != PieceType::None) {
            return false;  // 路徑被阻擋
        }
        row += rowStep;
        col += colStep;
    }
    
    return true;
}
```

---

## 特殊走法 (Special Moves)

### 1. 王車易位 (Castling)

**條件**：
1. 國王和城堡都沒有移動過
2. 國王和城堡之間沒有棋子
3. 國王不在被將軍狀態
4. 國王不會經過或進入被將軍的位置

```cpp
bool ChessBoard::canCastle(const QPoint& from, const QPoint& to) const {
    const ChessPiece& king = getPiece(from.y(), from.x());
    
    // 必須是國王
    if (king.getType() != PieceType::King) {
        return false;
    }
    
    // 國王不能移動過
    if (king.hasMoved()) {
        return false;
    }
    
    // 國王不能在被將軍狀態
    if (isInCheck(m_currentPlayer)) {
        return false;
    }
    
    int row = from.y();
    int colDiff = to.x() - from.x();
    
    // 必須水平移動兩格
    if (abs(colDiff) != 2 || to.y() != row) {
        return false;
    }
    
    // 確定城堡位置（王翼或后翼）
    int rookCol = (colDiff > 0) ? 7 : 0;  // 右側或左側
    const ChessPiece& rook = getPiece(row, rookCol);
    
    // 必須是城堡且未移動過
    if (rook.getType() != PieceType::Rook || rook.hasMoved()) {
        return false;
    }
    
    // 檢查中間路徑是否清空
    int start = std::min(from.x(), rookCol);
    int end = std::max(from.x(), rookCol);
    for (int col = start + 1; col < end; ++col) {
        if (getPiece(row, col).getType() != PieceType::None) {
            return false;
        }
    }
    
    // 檢查國王移動路徑是否經過被將軍的位置
    int step = (colDiff > 0) ? 1 : -1;
    for (int col = from.x(); col != to.x() + step; col += step) {
        if (wouldBeInCheck(from, QPoint(col, row), m_currentPlayer)) {
            return false;
        }
    }
    
    return true;
}
```

**執行王車易位**：

```cpp
// 在 movePiece() 中
if (canCastle(from, to)) {
    // 移動國王
    m_board[to.y()][to.x()] = piece;
    m_board[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
    // 移動城堡
    int rookFromCol = (to.x() > from.x()) ? 7 : 0;
    int rookToCol = (to.x() > from.x()) ? to.x() - 1 : to.x() + 1;
    
    m_board[to.y()][rookToCol] = m_board[to.y()][rookFromCol];
    m_board[to.y()][rookFromCol] = ChessPiece(PieceType::None, PieceColor::None);
    
    isCastling = true;
}
```

### 2. 吃過路兵 (En Passant)

**條件**：
1. 對手的兵從起始位置前進兩格
2. 己方的兵在對手兵的旁邊
3. 必須在下一步立即吃子

```cpp
// 檢查是否為吃過路兵
bool isEnPassant = false;
if (piece.getType() == PieceType::Pawn) {
    if (to == m_enPassantTarget) {
        isEnPassant = true;
        
        // 移除被吃的兵
        int capturedRow = from.y();  // 在同一行
        m_board[capturedRow][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
    }
}

// 設定新的吃過路兵目標
m_enPassantTarget = QPoint(-1, -1);  // 重置
if (piece.getType() == PieceType::Pawn) {
    int rowDiff = abs(to.y() - from.y());
    if (rowDiff == 2) {
        // 兵前進兩格，設定吃過路兵目標
        int direction = (piece.getColor() == PieceColor::White) ? -1 : 1;
        m_enPassantTarget = QPoint(to.x(), to.y() + direction);
    }
}
```

### 3. 兵升變 (Pawn Promotion)

**條件**：兵到達對方底線

```cpp
bool ChessBoard::needsPromotion(const QPoint& to) const {
    const ChessPiece& piece = getPiece(to.y(), to.x());
    
    if (piece.getType() != PieceType::Pawn) {
        return false;
    }
    
    // 白方到達 Row 0，黑方到達 Row 7
    if (piece.getColor() == PieceColor::White && to.y() == 0) {
        return true;
    }
    if (piece.getColor() == PieceColor::Black && to.y() == 7) {
        return true;
    }
    
    return false;
}

void ChessBoard::promotePawn(const QPoint& pos, PieceType newType) {
    ChessPiece& piece = getPiece(pos.y(), pos.x());
    
    if (piece.getType() == PieceType::Pawn && needsPromotion(pos)) {
        // 升變為選擇的棋子類型
        piece = ChessPiece(newType, piece.getColor());
        piece.setMoved(true);
    }
}
```

---

## 將軍與將死 (Check and Checkmate)

### 將軍檢查 (Check Detection)

```cpp
bool ChessBoard::isInCheck(PieceColor color) const {
    QPoint kingPos = findKing(color);
    if (kingPos.x() < 0) return false;
    
    PieceColor opponentColor = (color == PieceColor::White) ? 
                                PieceColor::Black : PieceColor::White;
    
    // 檢查是否有對手的棋子可以吃掉國王
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getColor() == opponentColor) {
                QPoint from(col, row);
                if (piece.isValidMove(from, kingPos, true)) {
                    // 檢查路徑（騎士除外）
                    if (piece.getType() == PieceType::Knight || 
                        isPathClear(from, kingPos)) {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}
```

### 將死檢查 (Checkmate Detection)

```cpp
bool ChessBoard::isCheckmate(PieceColor color) const {
    // 必須先在被將軍狀態
    if (!isInCheck(color)) {
        return false;
    }
    
    // 檢查是否有任何合法移動可以解除將軍
    return !hasAnyValidMoves(color);
}

bool ChessBoard::hasAnyValidMoves(PieceColor color) const {
    for (int fromRow = 0; fromRow < 8; ++fromRow) {
        for (int fromCol = 0; fromCol < 8; ++fromCol) {
            const ChessPiece& piece = m_board[fromRow][fromCol];
            if (piece.getColor() == color) {
                // 嘗試所有可能的目標位置
                for (int toRow = 0; toRow < 8; ++toRow) {
                    for (int toCol = 0; toCol < 8; ++toCol) {
                        QPoint from(fromCol, fromRow);
                        QPoint to(toCol, toRow);
                        if (isValidMove(from, to)) {
                            return true;  // 找到至少一個合法移動
                        }
                    }
                }
            }
        }
    }
    return false;  // 沒有合法移動
}
```

### 僵局檢查 (Stalemate Detection)

```cpp
bool ChessBoard::isStalemate(PieceColor color) const {
    // 不在被將軍狀態，但沒有合法移動
    if (isInCheck(color)) {
        return false;
    }
    
    return !hasAnyValidMoves(color);
}
```

### 子力不足和局 (Insufficient Material Draw)

```cpp
bool ChessBoard::isInsufficientMaterial() const {
    std::vector<PieceType> whitePieces;
    std::vector<PieceType> blackPieces;
    
    // 收集所有棋子
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getType() != PieceType::None && 
                piece.getType() != PieceType::King) {
                if (piece.getColor() == PieceColor::White) {
                    whitePieces.push_back(piece.getType());
                } else {
                    blackPieces.push_back(piece.getType());
                }
            }
        }
    }
    
    // 只有國王 vs 國王
    if (whitePieces.empty() && blackPieces.empty()) {
        return true;
    }
    
    // 國王+騎士 vs 國王 或 國王+主教 vs 國王
    if (whitePieces.size() == 1 && blackPieces.empty()) {
        return (whitePieces[0] == PieceType::Knight || 
                whitePieces[0] == PieceType::Bishop);
    }
    if (blackPieces.size() == 1 && whitePieces.empty()) {
        return (blackPieces[0] == PieceType::Knight || 
                blackPieces[0] == PieceType::Bishop);
    }
    
    return false;
}
```

---

## 遊戲模式 (Game Modes)

### 1. 霧戰模式 (Fog of War)

霧戰模式在 `Qt_Chess` 類別中實現，不在 `ChessBoard` 中：

```cpp
// 在 Qt_Chess 中
bool m_fogOfWarEnabled;

// 計算可見方格
std::set<QPoint> Qt_Chess::calculateVisibleSquares(PieceColor playerColor) {
    std::set<QPoint> visible;
    
    // 玩家的所有棋子
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            if (piece.getColor() == playerColor) {
                QPoint pos(col, row);
                visible.insert(pos);  // 棋子位置可見
                
                // 添加該棋子的所有可能移動位置
                for (int targetRow = 0; targetRow < 8; ++targetRow) {
                    for (int targetCol = 0; targetCol < 8; ++targetCol) {
                        QPoint target(targetCol, targetRow);
                        if (m_chessBoard.isValidMove(pos, target)) {
                            visible.insert(target);
                        }
                    }
                }
            }
        }
    }
    
    return visible;
}
```

### 2. 骰子模式 (Dice Mode)

```cpp
// 在 Qt_Chess 中
bool m_diceMode;
std::vector<PieceType> m_rolledPieceTypes;      // 骰出的棋子類型
std::vector<int> m_rolledPieceTypeCounts;       // 每種類型的剩餘次數
int m_diceMovesRemaining;                        // 剩餘移動次數

void Qt_Chess::rollDice() {
    // 獲取可移動的棋子類型
    std::vector<PieceType> movablePieces = getMovablePieceTypes(m_myColor);
    
    if (movablePieces.empty()) {
        return;
    }
    
    // 擲3個骰子
    m_rolledPieceTypes.clear();
    m_rolledPieceTypeCounts.clear();
    
    for (int i = 0; i < 3; i++) {
        int index = QRandomGenerator::global()->bounded(movablePieces.size());
        PieceType type = movablePieces[index];
        
        // 檢查是否已有此類型
        auto it = std::find(m_rolledPieceTypes.begin(), 
                           m_rolledPieceTypes.end(), type);
        if (it != m_rolledPieceTypes.end()) {
            int idx = std::distance(m_rolledPieceTypes.begin(), it);
            m_rolledPieceTypeCounts[idx]++;
        } else {
            m_rolledPieceTypes.push_back(type);
            m_rolledPieceTypeCounts.push_back(1);
        }
    }
    
    m_diceMovesRemaining = 3;
}
```

### 3. 地雷模式 (Bomb Chess)

```cpp
class ChessBoard {
private:
    bool m_bombModeEnabled;
    std::vector<QPoint> m_minePositions;
    bool m_lastMoveTriggeredMine;
    
public:
    void enableBombMode(bool enable) {
        m_bombModeEnabled = enable;
        if (enable) {
            placeMines();
        } else {
            m_minePositions.clear();
        }
    }
    
    void placeMines() {
        m_minePositions = generateRandomMinePositions();
    }
    
    static std::vector<QPoint> generateRandomMinePositions() {
        std::vector<QPoint> positions;
        
        // 在中間區域（Row 2-5）隨機放置 8 個地雷
        while (positions.size() < 8) {
            int row = 2 + QRandomGenerator::global()->bounded(4);  // 2-5
            int col = QRandomGenerator::global()->bounded(8);      // 0-7
            
            QPoint pos(col, row);
            if (std::find(positions.begin(), positions.end(), pos) == 
                positions.end()) {
                positions.push_back(pos);
            }
        }
        
        return positions;
    }
    
    bool isMineAt(const QPoint& pos) const {
        return std::find(m_minePositions.begin(), 
                        m_minePositions.end(), 
                        pos) != m_minePositions.end();
    }
};

// 在移動時檢查地雷
bool ChessBoard::movePiece(const QPoint& from, const QPoint& to) {
    // ... 正常移動邏輯 ...
    
    // 檢查是否踩到地雷
    if (m_bombModeEnabled && isMineAt(to)) {
        m_lastMoveTriggeredMine = true;
        
        // 移除移動的棋子
        m_board[to.y()][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
        
        // 移除周圍 3x3 的所有棋子（除了國王）
        for (int row = to.y() - 1; row <= to.y() + 1; ++row) {
            for (int col = to.x() - 1; col <= to.x() + 1; ++col) {
                if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                    ChessPiece& piece = m_board[row][col];
                    if (piece.getType() != PieceType::King) {
                        piece = ChessPiece(PieceType::None, PieceColor::None);
                    }
                }
            }
        }
        
        // 移除這個地雷
        m_minePositions.erase(
            std::remove(m_minePositions.begin(), m_minePositions.end(), to),
            m_minePositions.end()
        );
    } else {
        m_lastMoveTriggeredMine = false;
    }
    
    return true;
}
```

---

## 棋譜記錄 (Move Recording)

### 代數記譜法 (Algebraic Notation)

```cpp
void ChessBoard::recordMove(const QPoint& from, const QPoint& to, 
                            bool isCapture, bool isCastling, 
                            bool isEnPassant, bool isPromotion,
                            PieceType promotionType) {
    MoveRecord record;
    record.from = from;
    record.to = to;
    
    const ChessPiece& piece = getPiece(to.y(), to.x());
    record.pieceType = piece.getType();
    record.pieceColor = piece.getColor();
    record.isCapture = isCapture;
    record.isCastling = isCastling;
    record.isEnPassant = isEnPassant;
    record.isPromotion = isPromotion;
    record.promotionType = promotionType;
    
    // 檢查將軍和將死
    PieceColor opponentColor = (m_currentPlayer == PieceColor::White) ? 
                                PieceColor::Black : PieceColor::White;
    record.isCheck = isInCheck(opponentColor);
    record.isCheckmate = isCheckmate(opponentColor);
    
    // 生成代數記譜法
    record.algebraicNotation = generateAlgebraicNotation(record);
    
    m_moveHistory.push_back(record);
}

QString ChessBoard::generateAlgebraicNotation(const MoveRecord& move) const {
    QString notation;
    
    // 王車易位
    if (move.isCastling) {
        if (move.to.x() > move.from.x()) {
            notation = "O-O";  // 短易位（王翼）
        } else {
            notation = "O-O-O";  // 長易位（后翼）
        }
    } else {
        // 棋子符號（兵除外）
        if (move.pieceType != PieceType::Pawn) {
            notation += pieceTypeToNotation(move.pieceType);
            
            // 檢查是否需要消歧義
            if (isAmbiguousMove(move.from, move.to)) {
                notation += squareToNotation(move.from);
            }
        }
        
        // 吃子符號
        if (move.isCapture) {
            if (move.pieceType == PieceType::Pawn) {
                notation += QChar('a' + move.from.x());  // 兵吃子要寫出縱列
            }
            notation += "x";
        }
        
        // 目標方格
        notation += squareToNotation(move.to);
        
        // 兵升變
        if (move.isPromotion) {
            notation += "=";
            notation += pieceTypeToNotation(move.promotionType);
        }
        
        // 吃過路兵標記
        if (move.isEnPassant) {
            notation += " e.p.";
        }
    }
    
    // 將軍和將死符號
    if (move.isCheckmate) {
        notation += "#";
    } else if (move.isCheck) {
        notation += "+";
    }
    
    return notation;
}

QString ChessBoard::pieceTypeToNotation(PieceType type) const {
    switch (type) {
        case PieceType::King:   return "K";
        case PieceType::Queen:  return "Q";
        case PieceType::Rook:   return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        case PieceType::Pawn:   return "";
        default:                return "";
    }
}

QString ChessBoard::squareToNotation(const QPoint& square) const {
    char file = 'a' + square.x();
    char rank = '8' - square.y();
    return QString("%1%2").arg(file).arg(rank);
}
```

---

## 遊戲結果 (Game Results)

### 結果類型 (Result Types)

```cpp
enum class GameResult {
    InProgress,      // 遊戲進行中
    WhiteWins,       // 白方獲勝
    BlackWins,       // 黑方獲勝
    Draw,            // 和局
    WhiteResigns,    // 白方認輸
    BlackResigns,    // 黑方認輸
    WhiteTimeout,    // 白方超時
    BlackTimeout     // 黑方超時
};

QString ChessBoard::getGameResultString() const {
    switch (m_gameResult) {
        case GameResult::WhiteWins:
            return "1-0";
        case GameResult::BlackWins:
            return "0-1";
        case GameResult::Draw:
            return "1/2-1/2";
        case GameResult::WhiteResigns:
            return "0-1 (白方認輸)";
        case GameResult::BlackResigns:
            return "1-0 (黑方認輸)";
        case GameResult::WhiteTimeout:
            return "0-1 (白方超時)";
        case GameResult::BlackTimeout:
            return "1-0 (黑方超時)";
        default:
            return "*";
    }
}
```

---

## 相關文件 (Related Documentation)

- [AI 引擎文件](AI_ENGINE_DOCUMENTATION.md)
- [骰子模式文件](DICE_MODE_DOCUMENTATION.md)
- [伺服器文件](SERVER_DOCUMENTATION.md)
- [線上對戰功能](ONLINE_MODE_FEATURE.md)
