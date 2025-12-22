#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "chesspiece.h"
#include <QPoint>
#include <vector>
#include <QString>
#include <QStringList>

enum class GameResult {
    InProgress,      // 遊戲進行中
    WhiteWins,       // 白方獲勝
    BlackWins,       // 黑方獲勝
    Draw,            // 和局
    WhiteResigns,    // 白方認輸
    BlackResigns     // 黑方認輸
};

struct MoveRecord {
    QPoint from;
    QPoint to;
    PieceType pieceType;
    PieceColor pieceColor;
    bool isCapture;
    bool isCastling;
    bool isEnPassant;
    bool isPromotion;
    PieceType promotionType;
    bool isCheck;
    bool isCheckmate;
    QString algebraicNotation;
};

class ChessBoard {
public:
    ChessBoard();
    
    void initializeBoard();
    const ChessPiece& getPiece(int row, int col) const;
    ChessPiece& getPiece(int row, int col);
    void setPiece(int row, int col, const ChessPiece& piece);  // 安全地設置棋子
    
    bool movePiece(const QPoint& from, const QPoint& to);
    bool isValidMove(const QPoint& from, const QPoint& to) const;
    
    PieceColor getCurrentPlayer() const { return m_currentPlayer; }
    void setCurrentPlayer(PieceColor player) { m_currentPlayer = player; }
    bool isInCheck(PieceColor color) const;
    bool isCheckmate(PieceColor color) const;
    bool isStalemate(PieceColor color) const;
    bool isInsufficientMaterial() const;
    
    QPoint findKing(PieceColor color) const;
    QPoint getEnPassantTarget() const { return m_enPassantTarget; }
    
    // 升變 - 如果需要兵升變則返回 true
    bool needsPromotion(const QPoint& to) const;
    void promotePawn(const QPoint& pos, PieceType newType);
    
    // 棋譜記錄
    const std::vector<MoveRecord>& getMoveHistory() const { return m_moveHistory; }
    void clearMoveHistory();
    void setMoveHistory(const std::vector<MoveRecord>& history);
    QString getMoveNotation(int moveIndex) const;
    QStringList getAllMoveNotations() const;
    
    // 遊戲結果管理
    GameResult getGameResult() const { return m_gameResult; }
    void setGameResult(GameResult result) { m_gameResult = result; }
    QString getGameResultString() const;
    
    // 被吃掉的棋子追蹤
    const std::vector<ChessPiece>& getCapturedPieces(PieceColor color) const;
    void clearCapturedPieces();
    
    // 地雷模式功能
    void initializeMinesweeper();  // 初始化地雷模式（放置4-5個地雷）
    bool hasMine(int row, int col) const;  // 檢查該位置是否有地雷
    int getMineCount(int row, int col) const;  // 獲取周圍地雷數量
    bool isMinesweeperSquare(int row, int col) const;  // 檢查是否在地雷區域（3-6行，a-h列）
    void revealMineCount(int row, int col);  // 顯示周圍地雷數量
    bool checkMineExplosion(const QPoint& pos);  // 檢查是否踩到地雷，如果是則移除棋子
    
private:
    std::vector<std::vector<ChessPiece>> m_board;
    PieceColor m_currentPlayer;
    QPoint m_enPassantTarget; // 可以進行吃過路兵的位置（如果沒有則為 -1, -1）
    std::vector<MoveRecord> m_moveHistory; // 棋步歷史記錄
    GameResult m_gameResult; // 遊戲結果
    std::vector<ChessPiece> m_capturedWhite; // 被吃掉的白色棋子
    std::vector<ChessPiece> m_capturedBlack; // 被吃掉的黑色棋子
    
    // 地雷模式數據
    std::vector<std::vector<bool>> m_mineField;  // 地雷位置（8x8網格，true表示有地雷）
    std::vector<std::vector<int>> m_mineCounts;  // 每個格子周圍的地雷數量
    std::vector<std::vector<bool>> m_mineRevealed;  // 地雷數量是否已顯示
    
    void switchPlayer();
    bool wouldBeInCheck(const QPoint& from, const QPoint& to, PieceColor color) const;
    bool hasAnyValidMoves(PieceColor color) const;
    bool canPieceMove(const QPoint& pos) const;
    bool canCastle(const QPoint& from, const QPoint& to) const;
    
    // 棋譜記錄輔助函數
    void recordMove(const QPoint& from, const QPoint& to, bool isCapture, 
                   bool isCastling, bool isEnPassant, bool isPromotion = false,
                   PieceType promotionType = PieceType::None);
    QString generateAlgebraicNotation(const MoveRecord& move) const;
    QString pieceTypeToNotation(PieceType type) const;
    QString squareToNotation(const QPoint& square) const;
    bool isAmbiguousMove(const QPoint& from, const QPoint& to) const;
};

#endif // CHESSBOARD_H
