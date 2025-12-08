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

// 地形類型（類似踩地雷遊戲）
enum class TerrainType {
    None = 0,        // 無地形
    Mine = 1,        // 地雷：棋子踩到會被消滅
    Safe = 2,        // 安全區：顯示周圍地雷數量
    Bonus = 3,       // 獎勵格：額外移動一次
    Shield = 4,      // 護盾格：免疫下一次地雷傷害
    Teleport = 5,    // 傳送格：隨機傳送到其他位置
    Revealed = 6     // 已揭露格子（顯示數字）
};

// 創新玩法變體選項（地形玩法 - 類似踩地雷）
enum class GameVariant {
    ClassicMines = 0,    // 經典地雷：標準踩地雷規則
    SafeZones = 1,       // 安全區域：顯示周圍地雷數量
    BonusMoves = 2,      // 獎勵移動：踩到獎勵格可額外移動
    ShieldPower = 3,     // 護盾能力：踩到護盾格可免疫地雷
    TeleportChaos = 4,   // 傳送混亂：踩到傳送格會隨機傳送
    ProgressiveMines = 5 // 漸進地雷：地雷數量隨遊戲進行增加
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
    
    // 創新玩法變體（地形玩法）
    void setGameVariant(GameVariant variant, bool enabled);
    bool isGameVariantEnabled(GameVariant variant) const;
    void clearAllGameVariants();
    int getMoveCount() const { return static_cast<int>(m_moveHistory.size()); }
    
    // 地形系統（類似踩地雷）
    TerrainType getTerrain(int row, int col) const;
    void setTerrain(int row, int col, TerrainType terrain);
    void clearAllTerrain();
    void generateMineField(GameVariant variant, int mineCount);  // 生成地雷場
    bool isTerrainRevealed(int row, int col) const;  // 檢查地形是否已揭露
    void revealTerrain(int row, int col);  // 揭露地形
    int countAdjacentMines(int row, int col) const;  // 計算周圍地雷數量
    void applyTerrainEffect(const QPoint& pos, PieceColor color);  // 應用地形效果
    bool hasPieceShield(const QPoint& pos) const;  // 檢查棋子是否有護盾
    void setPieceShield(const QPoint& pos, bool hasShield);  // 設置棋子護盾
    bool canPieceHaveExtraMove(PieceColor color) const;  // 檢查是否可以額外移動
    void setExtraMove(PieceColor color, bool hasExtra);  // 設置額外移動
    
private:
    std::vector<std::vector<ChessPiece>> m_board;
    PieceColor m_currentPlayer;
    QPoint m_enPassantTarget; // 可以進行吃過路兵的位置（如果沒有則為 -1, -1）
    std::vector<MoveRecord> m_moveHistory; // 棋步歷史記錄
    GameResult m_gameResult; // 遊戲結果
    std::vector<ChessPiece> m_capturedWhite; // 被吃掉的白色棋子
    std::vector<ChessPiece> m_capturedBlack; // 被吃掉的黑色棋子
    bool m_gameVariants[6]; // 6種創新玩法變體的啟用狀態
    
    // 地形系統（類似踩地雷）
    std::vector<std::vector<TerrainType>> m_terrain; // 棋盤地形（隱藏）
    std::vector<std::vector<bool>> m_terrainRevealed; // 地形是否已揭露
    std::vector<std::vector<int>> m_adjacentMines; // 周圍地雷數量
    std::vector<std::vector<bool>> m_pieceShields; // 棋子護盾狀態
    bool m_whiteExtraMove; // 白方是否有額外移動
    bool m_blackExtraMove; // 黑方是否有額外移動
    
    void switchPlayer();
    bool wouldBeInCheck(const QPoint& from, const QPoint& to, PieceColor color) const;
    bool hasAnyValidMoves(PieceColor color) const;
    bool canPieceMove(const QPoint& pos) const;
    bool canCastle(const QPoint& from, const QPoint& to) const;
    
    // 地形檢查函數（類似踩地雷）
    void autoRevealSafeTerrain(int row, int col);  // 自動揭露安全區域（類似踩地雷的連鎖揭露）
    void addProgressiveMines();  // 漸進式添加地雷（隨遊戲進行）
    
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
