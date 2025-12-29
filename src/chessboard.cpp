#include "chessboard.h"
#include <QRandomGenerator>
#include <algorithm>

ChessBoard::ChessBoard()
    : m_board(8, std::vector<ChessPiece>(8)), m_currentPlayer(PieceColor::White), m_enPassantTarget(-1, -1), m_gameResult(GameResult::InProgress), m_bombModeEnabled(false), m_lastMoveTriggeredMine(false)
{
    initializeBoard();
}

void ChessBoard::initializeBoard() {
    // 初始化空棋盤
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_board[row][col] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
    
    // 設置黑色棋子（第 0 和 1 行）
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
    
    // 設置白色棋子（第 6 和 7 行）
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
    
    m_currentPlayer = PieceColor::White;
    m_enPassantTarget = QPoint(-1, -1);
    m_moveHistory.clear();
    m_gameResult = GameResult::InProgress;
    clearCapturedPieces();
    
    // 清除地雷
    m_minePositions.clear();
}

const ChessPiece& ChessBoard::getPiece(int row, int col) const {
    return m_board[row][col];
}

ChessPiece& ChessBoard::getPiece(int row, int col) {
    return m_board[row][col];
}

void ChessBoard::setPiece(int row, int col, const ChessPiece& piece) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        m_board[row][col] = piece;
    }
}

QPoint ChessBoard::findKing(PieceColor color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getType() == PieceType::King && piece.getColor() == color) {
                return QPoint(col, row);
            }
        }
    }
    return QPoint(-1, -1);
}

bool ChessBoard::isInCheck(PieceColor color) const {
    QPoint kingPos = findKing(color);
    if (kingPos.x() < 0) return false;
    
    PieceColor opponentColor = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    
    // 檢查是否有任何對手的棋子可以吃掉國王
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getColor() == opponentColor && piece.getType() != PieceType::None) {
                if (piece.isValidMove(QPoint(col, row), kingPos, m_board, QPoint(-1, -1))) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool ChessBoard::wouldBeInCheck(const QPoint& from, const QPoint& to, PieceColor color) const {
    // 複製棋盤並模擬移動
    std::vector<std::vector<ChessPiece>> tempBoard = m_board;
    
    // 在臨時棋盤上執行移動
    // 注意：QPoint(x, y) 對應到 board[y][x]，因為 x=列，y=行
    tempBoard[to.y()][to.x()] = tempBoard[from.y()][from.x()];
    tempBoard[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
    // 在移動後找到國王的位置
    QPoint kingPos(-1, -1);
    if (tempBoard[to.y()][to.x()].getType() == PieceType::King) {
        kingPos = to;
    } else {
        // 尋找國王
        bool found = false;
        for (int row = 0; row < 8 && !found; ++row) {
            for (int col = 0; col < 8; ++col) {
                const ChessPiece& piece = tempBoard[row][col];
                if (piece.getType() == PieceType::King && piece.getColor() == color) {
                    kingPos = QPoint(col, row);
                    found = true;
                    break;
                }
            }
        }
    }
    
    // 如果找不到國王，認為它被將軍（防禦性程式設計）
    if (kingPos.x() < 0) return true;
    
    PieceColor opponentColor = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    
    // 檢查是否有任何對手的棋子可以吃掉國王
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = tempBoard[row][col];
            if (piece.getColor() == opponentColor && piece.getType() != PieceType::None) {
                if (piece.isValidMove(QPoint(col, row), kingPos, tempBoard, QPoint(-1, -1))) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool ChessBoard::isValidMove(const QPoint& from, const QPoint& to) const {
    const ChessPiece& piece = m_board[from.y()][from.x()];
    
    // 檢查起始位置是否有棋子
    if (piece.getType() == PieceType::None) return false;
    
    // 檢查是否為當前玩家的棋子
    if (piece.getColor() != m_currentPlayer) return false;
    
    // 特殊處理王車易位
    if (piece.getType() == PieceType::King && abs(to.x() - from.x()) == 2) {
        return canCastle(from, to);
    }
    
    // 檢查該棋子類型的移動是否有效
    if (!piece.isValidMove(from, to, m_board, m_enPassantTarget)) return false;
    
    // 檢查移動是否會使自己的國王被將軍
    if (wouldBeInCheck(from, to, m_currentPlayer)) return false;
    
    return true;
}

bool ChessBoard::movePiece(const QPoint& from, const QPoint& to) {
    if (!isValidMove(from, to)) return false;
    
    // 重置地雷觸發標誌
    m_lastMoveTriggeredMine = false;
    
    ChessPiece& piece = m_board[from.y()][from.x()];
    PieceType pieceType = piece.getType();
    PieceColor pieceColor = piece.getColor();
    
    // 檢查是否為吃子、王車易位或吃過路兵
    bool isCapture = m_board[to.y()][to.x()].getType() != PieceType::None;
    bool isCastling = (pieceType == PieceType::King && abs(to.x() - from.x()) == 2);
    bool isEnPassant = (pieceType == PieceType::Pawn && to == m_enPassantTarget && m_enPassantTarget.x() >= 0);
    
    if (isEnPassant) {
        isCapture = true;  // 吃過路兵也是吃子
    }
    
    // 追蹤被吃掉的棋子（在移動之前儲存）
    if (isCapture && !isEnPassant) {
        // 常規吃子：儲存目標位置的棋子
        const ChessPiece& capturedPiece = m_board[to.y()][to.x()];
        if (capturedPiece.getColor() == PieceColor::White) {
            m_capturedWhite.push_back(capturedPiece);
        } else if (capturedPiece.getColor() == PieceColor::Black) {
            m_capturedBlack.push_back(capturedPiece);
        }
    }
    
    // 處理王車易位
    if (isCastling) {
        // 王翼易位
        if (to.x() > from.x()) {
            // 將車從 h 列移到 f 列
            ChessPiece& rook = m_board[from.y()][7];
            m_board[from.y()][5] = rook;
            m_board[from.y()][5].setMoved(true);
            m_board[from.y()][7] = ChessPiece(PieceType::None, PieceColor::None);
        }
        // 后翼易位
        else {
            // 將車從 a 列移到 d 列
            ChessPiece& rook = m_board[from.y()][0];
            m_board[from.y()][3] = rook;
            m_board[from.y()][3].setMoved(true);
            m_board[from.y()][0] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
    
    // 處理吃過路兵
    if (isEnPassant) {
        // 追蹤被吃掉的兵（吃過路兵時，被吃的兵在不同位置）
        int capturedPawnRow = (pieceColor == PieceColor::White) ? to.y() + 1 : to.y() - 1;
        const ChessPiece& capturedPawn = m_board[capturedPawnRow][to.x()];
        if (capturedPawn.getColor() == PieceColor::White) {
            m_capturedWhite.push_back(capturedPawn);
        } else if (capturedPawn.getColor() == PieceColor::Black) {
            m_capturedBlack.push_back(capturedPawn);
        }
        // 移除被吃掉的兵
        m_board[capturedPawnRow][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
    }
    
    // 在設置新的吃過路兵目標之前清除舊的
    m_enPassantTarget = QPoint(-1, -1);
    
    // 追蹤兵的雙格移動以便吃過路兵
    if (pieceType == PieceType::Pawn && abs(to.y() - from.y()) == 2) {
        // 將吃過路兵目標設置為兵跳過的格子（中間格子）
        // 例如，如果白兵從第 6 行移到第 4 行，目標為第 5 行
        int targetRow = (from.y() + to.y()) / 2;
        m_enPassantTarget = QPoint(from.x(), targetRow);
    }
    
    // 執行移動
    m_board[to.y()][to.x()] = piece;
    m_board[to.y()][to.x()].setMoved(true);
    m_board[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
    // 檢查地雷爆炸
    if (m_bombModeEnabled && isMineAt(to)) {
        // 踩到地雷：棋子被摧毀（從棋盤上移除）
        ChessPiece explodedPiece = m_board[to.y()][to.x()];
        m_board[to.y()][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
        
        // 將被炸毀的棋子加入被吃掉的棋子列表（用於顯示）
        if (explodedPiece.getColor() == PieceColor::White) {
            m_capturedWhite.push_back(explodedPiece);
        } else if (explodedPiece.getColor() == PieceColor::Black) {
            m_capturedBlack.push_back(explodedPiece);
        }
        
        m_lastMoveTriggeredMine = true;
        
        // 地雷爆炸後移除該地雷
        auto it = std::find(m_minePositions.begin(), m_minePositions.end(), to);
        if (it != m_minePositions.end()) {
            m_minePositions.erase(it);
        }
        
        // 如果國王被炸毀，遊戲立即結束，爆炸方輸
        if (explodedPiece.getType() == PieceType::King) {
            if (explodedPiece.getColor() == PieceColor::White) {
                m_gameResult = GameResult::BlackWins;
            } else {
                m_gameResult = GameResult::WhiteWins;
            }
        }
    }
    
    // 記錄移動（在切換玩家之前，因為 recordMove 需要檢查對手是否被將軍）
    recordMove(from, to, isCapture, isCastling, isEnPassant);
    
    switchPlayer();
    return true;
}

void ChessBoard::switchPlayer() {
    m_currentPlayer = (m_currentPlayer == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

bool ChessBoard::canPieceMove(const QPoint& pos) const {
    const ChessPiece& piece = m_board[pos.y()][pos.x()];
    if (piece.getType() == PieceType::None) return false;
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPoint to(col, row);
            if (piece.isValidMove(pos, to, m_board, m_enPassantTarget)) {
                if (!wouldBeInCheck(pos, to, piece.getColor())) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ChessBoard::hasAnyValidMoves(PieceColor color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getColor() == color && piece.getType() != PieceType::None) {
                if (canPieceMove(QPoint(col, row))) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ChessBoard::isCheckmate(PieceColor color) const {
    return isInCheck(color) && !hasAnyValidMoves(color);
}

bool ChessBoard::isStalemate(PieceColor color) const {
    return !isInCheck(color) && !hasAnyValidMoves(color);
}

bool ChessBoard::canCastle(const QPoint& from, const QPoint& to) const {
    const ChessPiece& king = m_board[from.y()][from.x()];
    
    // 必須是未移動過的國王
    if (king.getType() != PieceType::King || king.hasMoved()) return false;
    
    // 必須正好水平移動 2 格
    if (abs(to.x() - from.x()) != 2 || to.y() != from.y()) return false;
    
    // 國王不能處於被將軍狀態
    if (isInCheck(king.getColor())) return false;
    
    // 確定車的位置並檢查路徑
    int rookCol = (to.x() > from.x()) ? 7 : 0; // 王翼或后翼
    const ChessPiece& rook = m_board[from.y()][rookCol];
    
    // 車必須存在且未移動過
    if (rook.getType() != PieceType::Rook || rook.hasMoved()) return false;
    
    // 檢查國王和車之間的路徑是否暢通
    // 驗證國王當前位置和車之間的所有格子都是空的
    int direction = (to.x() > from.x()) ? 1 : -1;
    for (int col = from.x() + direction; col != rookCol; col += direction) {
        if (m_board[from.y()][col].getType() != PieceType::None) return false;
    }
    
    // 檢查國王不會經過被將軍的格子
    // 國王移動 2 格，所以檢查中間的格子
    QPoint intermediate(from.x() + direction, from.y());
    if (wouldBeInCheck(from, intermediate, king.getColor())) return false;
    
    // 檢查國王不會在目標位置被將軍
    if (wouldBeInCheck(from, to, king.getColor())) return false;
    
    return true;
}

bool ChessBoard::needsPromotion(const QPoint& to) const {
    const ChessPiece& piece = m_board[to.y()][to.x()];
    if (piece.getType() != PieceType::Pawn) return false;
    
    // 白兵到達第 0 行，黑兵到達第 7 行
    if (piece.getColor() == PieceColor::White && to.y() == 0) return true;
    if (piece.getColor() == PieceColor::Black && to.y() == 7) return true;
    
    return false;
}

void ChessBoard::promotePawn(const QPoint& pos, PieceType newType) {
    ChessPiece& piece = m_board[pos.y()][pos.x()];
    if (piece.getType() != PieceType::Pawn) return;
    
    PieceColor color = piece.getColor();
    m_board[pos.y()][pos.x()] = ChessPiece(newType, color);
    m_board[pos.y()][pos.x()].setMoved(true);
    
    // 更新最後一個移動記錄以包含升變信息
    if (!m_moveHistory.empty()) {
        MoveRecord& lastMove = m_moveHistory.back();
        lastMove.isPromotion = true;
        lastMove.promotionType = newType;
        lastMove.algebraicNotation = generateAlgebraicNotation(lastMove);
    }
}

bool ChessBoard::isInsufficientMaterial() const {
    // 計算棋盤上的棋子
    int whiteKnights = 0, blackKnights = 0;
    int whiteBishops = 0, blackBishops = 0;
    int whiteBishopsOnEvenSquares = 0, whiteBishopsOnOddSquares = 0;
    int blackBishopsOnEvenSquares = 0, blackBishopsOnOddSquares = 0;
    bool hasWhiteKing = false, hasBlackKing = false;
    bool hasOtherPieces = false;
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            PieceType type = piece.getType();
            PieceColor color = piece.getColor();
            
            if (type == PieceType::None) continue;
            
            if (type == PieceType::King) {
                if (color == PieceColor::White) hasWhiteKing = true;
                else hasBlackKing = true;
            } else if (type == PieceType::Knight) {
                if (color == PieceColor::White) whiteKnights++;
                else blackKnights++;
            } else if (type == PieceType::Bishop) {
                bool isEvenSquare = (row + col) % 2 == 0;
                if (color == PieceColor::White) {
                    whiteBishops++;
                    if (isEvenSquare) whiteBishopsOnEvenSquares++;
                    else whiteBishopsOnOddSquares++;
                } else {
                    blackBishops++;
                    if (isEvenSquare) blackBishopsOnEvenSquares++;
                    else blackBishopsOnOddSquares++;
                }
            } else {
                // 兵、車或后 - 材料足夠
                hasOtherPieces = true;
            }
        }
    }
    
    // 安全檢查：雙方國王必須存在
    if (!hasWhiteKing || !hasBlackKing) return false;
    
    // 如果有兵、車或后，材料足夠
    if (hasOtherPieces) return false;
    
    // 王對王
    if (whiteKnights == 0 && blackKnights == 0 && 
        whiteBishops == 0 && blackBishops == 0) {
        return true;
    }
    
    // 王和馬對王
    if ((whiteKnights == 1 && blackKnights == 0 && whiteBishops == 0 && blackBishops == 0) ||
        (blackKnights == 1 && whiteKnights == 0 && whiteBishops == 0 && blackBishops == 0)) {
        return true;
    }
    
    // 王和象對王
    if ((whiteBishops == 1 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0) ||
        (blackBishops == 1 && whiteBishops == 0 && whiteKnights == 0 && blackKnights == 0)) {
        return true;
    }
    
    // 王和象對王和象，且雙方象在同色格上
    if (whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0) {
        // 兩個象必須在同色格上（都在偶數格或都在奇數格）
        if ((whiteBishopsOnEvenSquares > 0 && blackBishopsOnEvenSquares > 0) ||
            (whiteBishopsOnOddSquares > 0 && blackBishopsOnOddSquares > 0)) {
            return true;
        }
    }
    
    return false;
}

// 棋譜記錄輔助函數實現
void ChessBoard::clearMoveHistory() {
    m_moveHistory.clear();
}

void ChessBoard::setMoveHistory(const std::vector<MoveRecord>& history) {
    m_moveHistory = history;
}

QString ChessBoard::getMoveNotation(int moveIndex) const {
    if (moveIndex < 0 || moveIndex >= static_cast<int>(m_moveHistory.size())) {
        return QString();
    }
    return m_moveHistory[moveIndex].algebraicNotation;
}

QStringList ChessBoard::getAllMoveNotations() const {
    QStringList notations;
    for (size_t i = 0; i < m_moveHistory.size(); ++i) {
        notations.append(m_moveHistory[i].algebraicNotation);
    }
    return notations;
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
    // 將坐標轉換為代數記法（例如 e4）
    // QPoint(x, y) 其中 x = 列（0-7），y = 行（0-7）
    // a-h 對應於列 0-7，1-8 對應於行 7-0（行 0 是第 8 行）
    char file = 'a' + square.x();
    char rank = '8' - square.y();
    return QString("%1%2").arg(file).arg(rank);
}

bool ChessBoard::isAmbiguousMove(const QPoint& from, const QPoint& to) const {
    // 檢查是否有多個同類型的棋子可以移動到同一目標
    const ChessPiece& movingPiece = m_board[from.y()][from.x()];
    PieceType pieceType = movingPiece.getType();
    PieceColor pieceColor = movingPiece.getColor();
    
    // 兵、國王移動通常不會模糊
    if (pieceType == PieceType::Pawn || pieceType == PieceType::King) {
        return false;
    }
    
    // 檢查是否有其他相同類型和顏色的棋子可以移動到同一目標
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPoint otherPos(col, row);
            if (otherPos == from) continue;
            
            const ChessPiece& piece = m_board[row][col];
            if (piece.getType() == pieceType && piece.getColor() == pieceColor) {
                // 檢查這個棋子是否也可以移動到目標
                if (piece.isValidMove(otherPos, to, m_board, m_enPassantTarget) &&
                    !wouldBeInCheck(otherPos, to, pieceColor)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void ChessBoard::recordMove(const QPoint& from, const QPoint& to, bool isCapture, 
                           bool isCastling, bool isEnPassant, bool isPromotion,
                           PieceType promotionType) {
    const ChessPiece& piece = m_board[to.y()][to.x()];
    PieceColor opponentColor = (piece.getColor() == PieceColor::White) ? 
                                PieceColor::Black : PieceColor::White;
    
    MoveRecord record;
    record.from = from;
    record.to = to;
    record.pieceType = piece.getType();
    record.pieceColor = piece.getColor();
    record.isCapture = isCapture;
    record.isCastling = isCastling;
    record.isEnPassant = isEnPassant;
    record.isPromotion = isPromotion;
    record.promotionType = promotionType;
    record.isCheck = isInCheck(opponentColor);
    record.isCheckmate = isCheckmate(opponentColor);
    record.algebraicNotation = generateAlgebraicNotation(record);
    
    m_moveHistory.push_back(record);
}

QString ChessBoard::generateAlgebraicNotation(const MoveRecord& move) const {
    QString notation;
    
    // 王車易位的特殊記法
    if (move.isCastling) {
        // 判斷是王翼還是后翼易位
        if (move.to.x() > move.from.x()) {
            notation = "O-O";  // 王翼易位
        } else {
            notation = "O-O-O";  // 后翼易位
        }
    } else {
        // 棋子類型（兵不標註）
        QString pieceNotation = pieceTypeToNotation(move.pieceType);
        notation += pieceNotation;
        
        // 處理消歧義
        if (!pieceNotation.isEmpty() && isAmbiguousMove(move.from, move.to)) {
            // 如果列不同，加上起始列
            bool addFile = true;
            bool addRank = false;
            
            // 檢查是否需要加上行
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    QPoint otherPos(col, row);
                    if (otherPos == move.from) continue;
                    
                    const ChessPiece& piece = m_board[row][col];
                    if (piece.getType() == move.pieceType && piece.getColor() == move.pieceColor) {
                        if (piece.isValidMove(otherPos, move.to, m_board, m_enPassantTarget) &&
                            !wouldBeInCheck(otherPos, move.to, move.pieceColor)) {
                            // 如果在同一列，需要加上行號
                            if (col == move.from.x()) {
                                addRank = true;
                            }
                        }
                    }
                }
            }
            
            if (addFile) {
                notation += QString(QChar('a' + move.from.x()));
            }
            if (addRank) {
                notation += QString(QChar('8' - move.from.y()));
            }
        }
        
        // 兵吃子需要標註起始列
        if (move.pieceType == PieceType::Pawn && move.isCapture) {
            notation += QString(QChar('a' + move.from.x()));
        }
        
        // 吃子標記
        if (move.isCapture) {
            notation += "x";
        }
        
        // 目標位置
        notation += squareToNotation(move.to);
        
        // 升變標記
        if (move.isPromotion) {
            notation += "=" + pieceTypeToNotation(move.promotionType);
        }
    }
    
    // 將軍或將死標記
    if (move.isCheckmate) {
        notation += "#";
    } else if (move.isCheck) {
        notation += "+";
    }
    
    return notation;
}

QString ChessBoard::getGameResultString() const {
    switch (m_gameResult) {
        case GameResult::WhiteWins:
            return "1-0";
        case GameResult::BlackWins:
            return "0-1";
        case GameResult::Draw:
            return "1/2-1/2";
        case GameResult::WhiteResigns:
            return "0-1";  // 白方認輸，黑方獲勝
        case GameResult::BlackResigns:
            return "1-0";  // 黑方認輸，白方獲勝
        case GameResult::InProgress:
        default:
            return "*";
    }
}

const std::vector<ChessPiece>& ChessBoard::getCapturedPieces(PieceColor color) const {
    if (color == PieceColor::White) {
        return m_capturedWhite;
    }
    return m_capturedBlack;
}

void ChessBoard::clearCapturedPieces() {
    m_capturedWhite.clear();
    m_capturedBlack.clear();
}

// 地雷模式實現 (Bomb Chess Mode Implementation)
void ChessBoard::enableBombMode(bool enable) {
    m_bombModeEnabled = enable;
    if (!enable) {
        m_minePositions.clear();
    }
    // 不再自動生成地雷位置 - 必須透過 placeMines() 或 setMinePositions() 明確設定
}

bool ChessBoard::isMineAt(const QPoint& pos) const {
    if (!m_bombModeEnabled) {
        return false;
    }
    
    for (const QPoint& mine : m_minePositions) {
        if (mine == pos) {
            return true;
        }
    }
    return false;
}

void ChessBoard::placeMines() {
    m_minePositions.clear();
    
    // 地雷區域：第3-6行（索引2-5），第a-h列（索引0-7）
    // 隨機放置4-5個地雷
    std::vector<QPoint> availablePositions;
    for (int row = 2; row <= 5; ++row) {
        for (int col = 0; col < 8; ++col) {
            availablePositions.push_back(QPoint(col, row));
        }
    }
    
    // 使用Qt的隨機數生成器
    QRandomGenerator *rng = QRandomGenerator::global();
    int numMines = 4 + (rng->bounded(2)); // 4或5個地雷
    
    // 隨機打亂位置列表
    for (int i = availablePositions.size() - 1; i > 0; --i) {
        int j = rng->bounded(i + 1);
        std::swap(availablePositions[i], availablePositions[j]);
    }
    
    // 選取前numMines個位置作為地雷
    for (int i = 0; i < numMines && i < static_cast<int>(availablePositions.size()); ++i) {
        m_minePositions.push_back(availablePositions[i]);
    }
}

void ChessBoard::setMinePositions(const std::vector<QPoint>& positions) {
    m_minePositions = positions;
}
