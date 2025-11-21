#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "chesspiece.h"
#include <QPoint>
#include <vector>

// 移動資訊結構
struct MoveInfo {
    bool isCapture;      // 是否吃子
    bool isCastling;     // 是否王車易位
    bool isCheck;        // 移動後是否將軍
    bool isCheckmate;    // 移動後是否將死
    
    MoveInfo() : isCapture(false), isCastling(false), isCheck(false), isCheckmate(false) {}
};

// 棋盤類別 - 管理棋盤狀態和遊戲規則
class ChessBoard {
public:
    ChessBoard();
    
    void initializeBoard();
    const ChessPiece& getPiece(int row, int col) const;
    ChessPiece& getPiece(int row, int col);
    
    bool movePiece(const QPoint& from, const QPoint& to, MoveInfo* moveInfo = nullptr);
    bool isValidMove(const QPoint& from, const QPoint& to) const;
    
    PieceColor getCurrentPlayer() const { return m_currentPlayer; }
    bool isInCheck(PieceColor color) const;
    bool isCheckmate(PieceColor color) const;
    bool isStalemate(PieceColor color) const;
    bool isInsufficientMaterial() const;
    
    QPoint findKing(PieceColor color) const;
    QPoint getEnPassantTarget() const { return m_enPassantTarget; }
    
    // 升變 - 當兵需要升變時回傳 true
    bool needsPromotion(const QPoint& to) const;
    void promotePawn(const QPoint& pos, PieceType newType);
    
private:
    std::vector<std::vector<ChessPiece>> m_board;
    PieceColor m_currentPlayer;
    QPoint m_enPassantTarget; // 可進行吃過路兵捕獲的位置（若無則為 -1, -1）
    
    void switchPlayer();
    bool wouldBeInCheck(const QPoint& from, const QPoint& to, PieceColor color) const;
    bool hasAnyValidMoves(PieceColor color) const;
    bool canPieceMove(const QPoint& pos) const;
    bool canCastle(const QPoint& from, const QPoint& to) const;
};

#endif // CHESSBOARD_H
