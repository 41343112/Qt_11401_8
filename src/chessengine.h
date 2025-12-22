#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QPoint>
#include <QTimer>
#include "chesspiece.h"

class ChessBoard;

// 引擎難度等級
enum class EngineDifficulty {
    VeryEasy = 1,   // 等級 1 - 非常簡單
    Easy = 5,       // 等級 5 - 簡單
    Medium = 10,    // 等級 10 - 中等
    Hard = 15,      // 等級 15 - 困難
    VeryHard = 20   // 等級 20 - 非常困難
};

// 遊戲模式
enum class GameMode {
    HumanVsHuman,       // 雙人對弈
    HumanVsComputer,    // 人機對弈（玩家執白）
    ComputerVsHuman,    // 人機對弈（玩家執黑）
    OnlineGame,         // 線上對戰
    Minesweeper         // 地雷模式
};

class ChessEngine : public QObject
{
    Q_OBJECT

public:
    explicit ChessEngine(QObject *parent = nullptr);
    ~ChessEngine();

    // 引擎控制
    bool startEngine(const QString& enginePath);
    void stopEngine();
    bool isEngineRunning() const;

    // 遊戲模式和難度設定
    void setGameMode(GameMode mode);
    GameMode getGameMode() const { return m_gameMode; }
    
    void setDifficulty(int level);  // 0-20
    int getDifficulty() const { return m_skillLevel; }
    
    void setThinkingTime(int milliseconds);  // 設定思考時間
    int getThinkingTime() const { return m_thinkingTimeMs; }
    
    void setSearchDepth(int depth);  // 設定搜尋深度（1-30）
    int getSearchDepth() const { return m_searchDepth; }

    // 棋局控制
    void newGame();
    void setPosition(const QString& fen);
    void setPositionFromMoves(const QStringList& moves);
    void requestMove();
    void stop();

    // UCI 相關
    QString getBestMove() const { return m_bestMove; }

    // 工具函數 - 將棋盤狀態轉換為 FEN 格式
    static QString boardToFEN(const ChessBoard& board);
    static QString moveToUCI(const QPoint& from, const QPoint& to, PieceType promotionType = PieceType::None);
    static void uciToMove(const QString& uci, QPoint& from, QPoint& to, PieceType& promotionType);

signals:
    void engineReady();
    void bestMoveFound(const QString& move);
    void engineError(const QString& error);
    void thinkingStarted();
    void thinkingStopped();

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onEngineFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onEngineError(QProcess::ProcessError error);

private:
    QProcess* m_process;
    QString m_enginePath;
    QString m_bestMove;
    QString m_currentPosition;  // 當前 FEN 或移動列表
    
    GameMode m_gameMode;
    int m_skillLevel;           // 0-20
    int m_thinkingTimeMs;       // 思考時間（毫秒）
    int m_searchDepth;          // 搜尋深度（1-30）
    
    bool m_isReady;
    bool m_isThinking;
    
    void sendCommand(const QString& command);
    void parseOutput(const QString& line);
    void configureEngine();
};

#endif // CHESSENGINE_H
