#include "chessboard.h"

ChessBoard::ChessBoard()
    : m_board(8, std::vector<ChessPiece>(8)), m_currentPlayer(PieceColor::White), m_enPassantTarget(-1, -1)
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
    
    // 設置黑方棋子（第 0 和 1 行）
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
    
    // 設置白方棋子（第 6 和 7 行）
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
}

const ChessPiece& ChessBoard::getPiece(int row, int col) const {
    return m_board[row][col];
}

ChessPiece& ChessBoard::getPiece(int row, int col) {
    return m_board[row][col];
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
    
    // 檢查是否有任何對手的棋子可以捕獲王
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
    
    // 找出移動後王的位置
    QPoint kingPos(-1, -1);
    if (tempBoard[to.y()][to.x()].getType() == PieceType::King) {
        kingPos = to;
    } else {
        // 搜尋王的位置
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
    
    // 如果找不到王，視為被將軍（防禦性程式設計）
    if (kingPos.x() < 0) return true;
    
    PieceColor opponentColor = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    
    // 檢查是否有任何對手的棋子可以捕獲王
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
    
    // 王車易位的特殊處理
    if (piece.getType() == PieceType::King && abs(to.x() - from.x()) == 2) {
        return canCastle(from, to);
    }
    
    // 檢查此棋子類型的移動是否有效
    if (!piece.isValidMove(from, to, m_board, m_enPassantTarget)) return false;
    
    // 檢查移動是否會使己方王處於被將軍狀態
    if (wouldBeInCheck(from, to, m_currentPlayer)) return false;
    
    return true;
}

bool ChessBoard::movePiece(const QPoint& from, const QPoint& to, MoveInfo* moveInfo) {
    if (!isValidMove(from, to)) return false;
    
    ChessPiece& piece = m_board[from.y()][from.x()];
    PieceType pieceType = piece.getType();
    PieceColor pieceColor = piece.getColor();
    
    // 檢測是否吃子（目標位置有棋子）
    bool isCapture = m_board[to.y()][to.x()].getType() != PieceType::None;
    
    // 檢測是否王車易位
    bool isCastling = (pieceType == PieceType::King && abs(to.x() - from.x()) == 2);
    
    // 處理王車易位
    if (isCastling) {
        // 王翼易位
        if (to.x() > from.x()) {
            // 將車從 h 列移至 f 列
            ChessPiece& rook = m_board[from.y()][7];
            m_board[from.y()][5] = rook;
            m_board[from.y()][5].setMoved(true);
            m_board[from.y()][7] = ChessPiece(PieceType::None, PieceColor::None);
        }
        // 后翼易位
        else {
            // 將車從 a 列移至 d 列
            ChessPiece& rook = m_board[from.y()][0];
            m_board[from.y()][3] = rook;
            m_board[from.y()][3].setMoved(true);
            m_board[from.y()][0] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
    
    // 處理吃過路兵
    if (pieceType == PieceType::Pawn && to == m_enPassantTarget && m_enPassantTarget.x() >= 0) {
        // 移除被捕獲的兵
        int capturedPawnRow = (pieceColor == PieceColor::White) ? to.y() + 1 : to.y() - 1;
        m_board[capturedPawnRow][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
        isCapture = true; // 吃過路兵也算吃子
    }
    
    // 在設置新目標前清除吃過路兵目標
    m_enPassantTarget = QPoint(-1, -1);
    
    // 追蹤兵的雙格移動以供吃過路兵判斷
    if (pieceType == PieceType::Pawn && abs(to.y() - from.y()) == 2) {
        // 將吃過路兵目標設置為兵跳過的格子（中間格）
        // 例如，白兵從第 6 行移至第 4 行，目標為第 5 行
        int targetRow = (from.y() + to.y()) / 2;
        m_enPassantTarget = QPoint(from.x(), targetRow);
    }
    
    // 執行移動
    m_board[to.y()][to.x()] = piece;
    m_board[to.y()][to.x()].setMoved(true);
    m_board[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
    switchPlayer();
    
    // 檢查移動後是否將軍或將死
    PieceColor opponentColor = m_currentPlayer; // 已切換玩家，所以當前玩家是對手
    bool moveResultsInCheck = isInCheck(opponentColor);
    bool moveResultsInCheckmate = false;
    if (moveResultsInCheck) {
        moveResultsInCheckmate = isCheckmate(opponentColor);
    }
    
    // 填充移動資訊
    if (moveInfo) {
        moveInfo->isCapture = isCapture;
        moveInfo->isCastling = isCastling;
        moveInfo->isCheck = moveResultsInCheck;
        moveInfo->isCheckmate = moveResultsInCheckmate;
    }
    
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
    
    // 必須是未移動過的王
    if (king.getType() != PieceType::King || king.hasMoved()) return false;
    
    // 必須橫向移動正好 2 格
    if (abs(to.x() - from.x()) != 2 || to.y() != from.y()) return false;
    
    // 王不能處於被將軍狀態
    if (isInCheck(king.getColor())) return false;
    
    // 判斷車的位置並檢查路徑
    int rookCol = (to.x() > from.x()) ? 7 : 0; // 王翼或后翼
    const ChessPiece& rook = m_board[from.y()][rookCol];
    
    // 車必須存在且未移動過
    if (rook.getType() != PieceType::Rook || rook.hasMoved()) return false;
    
    // 檢查王和車之間的路徑是否暢通
    // 驗證王的當前位置和車之間的所有格子都是空的
    int direction = (to.x() > from.x()) ? 1 : -1;
    for (int col = from.x() + direction; col != rookCol; col += direction) {
        if (m_board[from.y()][col].getType() != PieceType::None) return false;
    }
    
    // 檢查王移動過程中是否經過被將軍的格子
    // 王移動 2 格，所以檢查中間格
    QPoint intermediate(from.x() + direction, from.y());
    if (wouldBeInCheck(from, intermediate, king.getColor())) return false;
    
    // 檢查王最終是否處於被將軍狀態
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
}

bool ChessBoard::isInsufficientMaterial() const {
    // 計算棋盤上的棋子數量
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
                // 兵、車或后 - 子力充足
                hasOtherPieces = true;
            }
        }
    }
    
    // 安全檢查：雙方的王都必須存在
    if (!hasWhiteKing || !hasBlackKing) return false;
    
    // 如果有兵、車或后，子力充足
    if (hasOtherPieces) return false;
    
    // 王對王
    if (whiteKnights == 0 && blackKnights == 0 && 
        whiteBishops == 0 && blackBishops == 0) {
        return true;
    }
    
    // 王與馬對王
    if ((whiteKnights == 1 && blackKnights == 0 && whiteBishops == 0 && blackBishops == 0) ||
        (blackKnights == 1 && whiteKnights == 0 && whiteBishops == 0 && blackBishops == 0)) {
        return true;
    }
    
    // 王與象對王
    if ((whiteBishops == 1 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0) ||
        (blackBishops == 1 && whiteBishops == 0 && whiteKnights == 0 && blackKnights == 0)) {
        return true;
    }
    
    // 王與象對王與象，且兩象在同色格上
    if (whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0) {
        // 兩個象必須在同色格上（都在偶數格或都在奇數格）
        if ((whiteBishopsOnEvenSquares > 0 && blackBishopsOnEvenSquares > 0) ||
            (whiteBishopsOnOddSquares > 0 && blackBishopsOnOddSquares > 0)) {
            return true;
        }
    }
    
    return false;
}
