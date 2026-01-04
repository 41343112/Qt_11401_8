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
    BlackResigns,    // 黑方認輸
    WhiteTimeout,    // 白方超時
    BlackTimeout     // 黑方超時
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
    
    // 地雷功能 (Bomb Chess Mode)
    void enableBombMode(bool enable);
    bool isBombModeEnabled() const { return m_bombModeEnabled; }
    bool isMineAt(const QPoint& pos) const;
    const std::vector<QPoint>& getMinePositions() const { return m_minePositions; }
    void placeMines();
    void setMinePositions(const std::vector<QPoint>& positions);
    bool lastMoveTriggeredMine() const { return m_lastMoveTriggeredMine; }
    static std::vector<QPoint> generateRandomMinePositions();  // 生成隨機地雷位置（靜態工具方法）
    
private:
    std::vector<std::vector<ChessPiece>> m_board;
    PieceColor m_currentPlayer;
    QPoint m_enPassantTarget; // 可以進行吃過路兵的位置（如果沒有則為 -1, -1）
    std::vector<MoveRecord> m_moveHistory; // 棋步歷史記錄
    GameResult m_gameResult; // 遊戲結果
    std::vector<ChessPiece> m_capturedWhite; // 被吃掉的白色棋子
    std::vector<ChessPiece> m_capturedBlack; // 被吃掉的黑色棋子
    
    // 地雷模式相關變量
    bool m_bombModeEnabled; // 地雷模式是否啟用
    std::vector<QPoint> m_minePositions; // 地雷位置
    bool m_lastMoveTriggeredMine; // 上一步移動是否觸發了地雷

    
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
