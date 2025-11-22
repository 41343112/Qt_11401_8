#include "chessboard.h"

ChessBoard::ChessBoard()
    : m_board(8, std::vector<ChessPiece>(8)), m_currentPlayer(PieceColor::White), m_enPassantTarget(-1, -1)
{
    initializeBoard();
}

void ChessBoard::initializeBoard() {
    initializeBoard(BoardPreset::Standard);
}

void ChessBoard::initializeBoard(BoardPreset preset) {
    clearBoard();
    
    switch (preset) {
        case BoardPreset::Standard:
            initializeStandardBoard();
            break;
        case BoardPreset::MidGame:
            initializeMidGameBoard();
            break;
        case BoardPreset::EndGame:
            initializeEndGameBoard();
            break;
    }
    
    m_currentPlayer = PieceColor::White;
    m_enPassantTarget = QPoint(-1, -1);
}

void ChessBoard::clearBoard() {
    // Initialize empty board
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_board[row][col] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
}

void ChessBoard::initializeStandardBoard() {
    // Setup black pieces (row 0 and 1)
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
    
    // Setup white pieces (row 6 and 7)
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
}

void ChessBoard::initializeMidGameBoard() {
    // Mid-game tactical position - Sicilian Defense-like position
    // This is a common mid-game position with both sides having active play
    
    // Black pieces
    m_board[0][0] = ChessPiece(PieceType::Rook, PieceColor::Black);
    m_board[0][4] = ChessPiece(PieceType::King, PieceColor::Black);
    m_board[0][7] = ChessPiece(PieceType::Rook, PieceColor::Black);
    m_board[1][0] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[1][1] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[1][5] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[1][6] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[1][7] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[2][2] = ChessPiece(PieceType::Bishop, PieceColor::Black);
    m_board[2][3] = ChessPiece(PieceType::Queen, PieceColor::Black);
    m_board[3][2] = ChessPiece(PieceType::Knight, PieceColor::Black);
    m_board[3][4] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[4][3] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[4][5] = ChessPiece(PieceType::Bishop, PieceColor::Black);
    
    // White pieces
    m_board[7][0] = ChessPiece(PieceType::Rook, PieceColor::White);
    m_board[7][4] = ChessPiece(PieceType::King, PieceColor::White);
    m_board[7][7] = ChessPiece(PieceType::Rook, PieceColor::White);
    m_board[6][0] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[6][1] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[6][5] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[6][6] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[6][7] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[5][2] = ChessPiece(PieceType::Bishop, PieceColor::White);
    m_board[5][3] = ChessPiece(PieceType::Queen, PieceColor::White);
    m_board[4][2] = ChessPiece(PieceType::Knight, PieceColor::White);
    m_board[4][4] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[3][3] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[3][5] = ChessPiece(PieceType::Bishop, PieceColor::White);
    
    // Mark pieces as moved (since this is mid-game)
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (m_board[row][col].getType() != PieceType::None) {
                m_board[row][col].setMoved(true);
            }
        }
    }
}

void ChessBoard::initializeEndGameBoard() {
    // King and pawn endgame - classic endgame practice position
    // White has a passed pawn and slight advantage
    
    // Black pieces
    m_board[0][4] = ChessPiece(PieceType::King, PieceColor::Black);
    m_board[3][1] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    m_board[3][6] = ChessPiece(PieceType::Pawn, PieceColor::Black);
    
    // White pieces
    m_board[7][4] = ChessPiece(PieceType::King, PieceColor::White);
    m_board[5][2] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[5][3] = ChessPiece(PieceType::Pawn, PieceColor::White);
    m_board[5][5] = ChessPiece(PieceType::Pawn, PieceColor::White);
    
    // Mark all pieces as moved
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (m_board[row][col].getType() != PieceType::None) {
                m_board[row][col].setMoved(true);
            }
        }
    }
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
    
    // Check if any opponent piece can capture the king
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
    // Make a copy of the board and simulate the move
    std::vector<std::vector<ChessPiece>> tempBoard = m_board;
    
    // Perform the move on the temporary board
    // Note: QPoint(x, y) maps to board[y][x] since x=col, y=row
    tempBoard[to.y()][to.x()] = tempBoard[from.y()][from.x()];
    tempBoard[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
    // Find king position after the move
    QPoint kingPos(-1, -1);
    if (tempBoard[to.y()][to.x()].getType() == PieceType::King) {
        kingPos = to;
    } else {
        // Search for the king
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
    
    // If king not found, consider it as check (defensive programming)
    if (kingPos.x() < 0) return true;
    
    PieceColor opponentColor = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    
    // Check if any opponent piece can capture the king
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
    
    // Check if there's a piece at from position
    if (piece.getType() == PieceType::None) return false;
    
    // Check if it's the current player's piece
    if (piece.getColor() != m_currentPlayer) return false;
    
    // Special handling for castling
    if (piece.getType() == PieceType::King && abs(to.x() - from.x()) == 2) {
        return canCastle(from, to);
    }
    
    // Check if the move is valid for this piece type
    if (!piece.isValidMove(from, to, m_board, m_enPassantTarget)) return false;
    
    // Check if the move would put own king in check
    if (wouldBeInCheck(from, to, m_currentPlayer)) return false;
    
    return true;
}

bool ChessBoard::movePiece(const QPoint& from, const QPoint& to) {
    if (!isValidMove(from, to)) return false;
    
    ChessPiece& piece = m_board[from.y()][from.x()];
    PieceType pieceType = piece.getType();
    PieceColor pieceColor = piece.getColor();
    
    // Handle castling
    if (pieceType == PieceType::King && abs(to.x() - from.x()) == 2) {
        // Kingside castling
        if (to.x() > from.x()) {
            // Move rook from h-file to f-file
            ChessPiece& rook = m_board[from.y()][7];
            m_board[from.y()][5] = rook;
            m_board[from.y()][5].setMoved(true);
            m_board[from.y()][7] = ChessPiece(PieceType::None, PieceColor::None);
        }
        // Queenside castling
        else {
            // Move rook from a-file to d-file
            ChessPiece& rook = m_board[from.y()][0];
            m_board[from.y()][3] = rook;
            m_board[from.y()][3].setMoved(true);
            m_board[from.y()][0] = ChessPiece(PieceType::None, PieceColor::None);
        }
    }
    
    // Handle en passant capture
    if (pieceType == PieceType::Pawn && to == m_enPassantTarget && m_enPassantTarget.x() >= 0) {
        // Remove the captured pawn
        int capturedPawnRow = (pieceColor == PieceColor::White) ? to.y() + 1 : to.y() - 1;
        m_board[capturedPawnRow][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
    }
    
    // Clear en passant target before setting new one
    m_enPassantTarget = QPoint(-1, -1);
    
    // Track pawn double moves for en passant
    if (pieceType == PieceType::Pawn && abs(to.y() - from.y()) == 2) {
        // Set en passant target to the square the pawn skipped over (middle square)
        // For example, if white pawn moves from row 6 to row 4, target is row 5
        int targetRow = (from.y() + to.y()) / 2;
        m_enPassantTarget = QPoint(from.x(), targetRow);
    }
    
    // Perform the move
    m_board[to.y()][to.x()] = piece;
    m_board[to.y()][to.x()].setMoved(true);
    m_board[from.y()][from.x()] = ChessPiece(PieceType::None, PieceColor::None);
    
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
    
    // Must be a king that hasn't moved
    if (king.getType() != PieceType::King || king.hasMoved()) return false;
    
    // Must move exactly 2 squares horizontally
    if (abs(to.x() - from.x()) != 2 || to.y() != from.y()) return false;
    
    // King must not be in check
    if (isInCheck(king.getColor())) return false;
    
    // Determine rook position and check path
    int rookCol = (to.x() > from.x()) ? 7 : 0; // Kingside or queenside
    const ChessPiece& rook = m_board[from.y()][rookCol];
    
    // Rook must exist and not have moved
    if (rook.getType() != PieceType::Rook || rook.hasMoved()) return false;
    
    // Check if path is clear between king and rook
    // Verify all squares between king's current position and rook are empty
    int direction = (to.x() > from.x()) ? 1 : -1;
    for (int col = from.x() + direction; col != rookCol; col += direction) {
        if (m_board[from.y()][col].getType() != PieceType::None) return false;
    }
    
    // Check that king doesn't move through check
    // King moves 2 squares, so check the intermediate square
    QPoint intermediate(from.x() + direction, from.y());
    if (wouldBeInCheck(from, intermediate, king.getColor())) return false;
    
    // Check that king doesn't end up in check
    if (wouldBeInCheck(from, to, king.getColor())) return false;
    
    return true;
}

bool ChessBoard::needsPromotion(const QPoint& to) const {
    const ChessPiece& piece = m_board[to.y()][to.x()];
    if (piece.getType() != PieceType::Pawn) return false;
    
    // White pawn reaches row 0, black pawn reaches row 7
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
    // Count pieces on the board
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
                // Pawn, Rook, or Queen - sufficient material
                hasOtherPieces = true;
            }
        }
    }
    
    // Safety check: both kings must be present
    if (!hasWhiteKing || !hasBlackKing) return false;
    
    // If there are pawns, rooks, or queens, material is sufficient
    if (hasOtherPieces) return false;
    
    // King vs King
    if (whiteKnights == 0 && blackKnights == 0 && 
        whiteBishops == 0 && blackBishops == 0) {
        return true;
    }
    
    // King and Knight vs King
    if ((whiteKnights == 1 && blackKnights == 0 && whiteBishops == 0 && blackBishops == 0) ||
        (blackKnights == 1 && whiteKnights == 0 && whiteBishops == 0 && blackBishops == 0)) {
        return true;
    }
    
    // King and Bishop vs King
    if ((whiteBishops == 1 && blackBishops == 0 && whiteKnights == 0 && blackKnights == 0) ||
        (blackBishops == 1 && whiteBishops == 0 && whiteKnights == 0 && blackKnights == 0)) {
        return true;
    }
    
    // King and Bishop vs King and Bishop with bishops on same color squares
    if (whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0) {
        // Both bishops must be on the same color squares (both on even or both on odd)
        if ((whiteBishopsOnEvenSquares > 0 && blackBishopsOnEvenSquares > 0) ||
            (whiteBishopsOnOddSquares > 0 && blackBishopsOnOddSquares > 0)) {
            return true;
        }
    }
    
    return false;
}
