# AI 引擎程式碼說明 (AI Engine Code Documentation)

## 目錄 (Table of Contents)

1. [概述 (Overview)](#概述-overview)
2. [Stockfish 引擎整合 (Stockfish Integration)](#stockfish-引擎整合-stockfish-integration)
3. [UCI 協議 (UCI Protocol)](#uci-協議-uci-protocol)
4. [難度設定 (Difficulty Settings)](#難度設定-difficulty-settings)
5. [移動生成 (Move Generation)](#移動生成-move-generation)
6. [引擎控制 (Engine Control)](#引擎控制-engine-control)
7. [FEN 與 UCI 轉換 (FEN and UCI Conversion)](#fen-與-uci-轉換-fen-and-uci-conversion)
8. [效能優化 (Performance Optimization)](#效能優化-performance-optimization)

---

## 概述 (Overview)

### 中文說明

Qt_Chess 使用 Stockfish 西洋棋引擎實現 AI 對弈功能。`ChessEngine` 類別（`src/chessengine.h` 和 `src/chessengine.cpp`）負責：

- 引擎進程管理
- UCI 協議通訊
- 難度等級設定
- 移動請求和接收
- 棋盤狀態轉換（FEN 格式）

Stockfish 是世界上最強的開源西洋棋引擎之一，支援多種難度級別和搜尋深度設定。

### English Description

Qt_Chess uses the Stockfish chess engine for AI gameplay. The `ChessEngine` class (`src/chessengine.h` and `src/chessengine.cpp`) is responsible for:

- Engine process management
- UCI protocol communication
- Difficulty level configuration
- Move requests and responses
- Board state conversion (FEN format)

Stockfish is one of the strongest open-source chess engines in the world, supporting multiple difficulty levels and search depth settings.

---

## Stockfish 引擎整合 (Stockfish Integration)

### 類別定義 (Class Definition)

```cpp
class ChessEngine : public QObject {
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
    GameMode getGameMode() const;
    void setDifficulty(int level);  // 0-20
    void setThinkingTime(int milliseconds);
    void setSearchDepth(int depth);  // 1-30
    
    // 棋局控制
    void newGame();
    void setPosition(const QString& fen);
    void setPositionFromMoves(const QStringList& moves);
    void requestMove();
    void stop();
    
signals:
    void engineReady();
    void bestMoveFound(const QString& move);
    void engineError(const QString& error);
    void thinkingStarted();
    void thinkingStopped();
    
private:
    QProcess* m_process;
    QString m_enginePath;
    QString m_bestMove;
    GameMode m_gameMode;
    int m_skillLevel;      // 0-20
    int m_thinkingTimeMs;  // 思考時間（毫秒）
    int m_searchDepth;     // 搜尋深度（1-30）
    bool m_isReady;
    bool m_isThinking;
};
```

### 遊戲模式 (Game Modes)

```cpp
enum class GameMode {
    HumanVsHuman,       // 雙人對弈
    HumanVsComputer,    // 人機對弈（玩家執白）
    ComputerVsHuman,    // 人機對弈（玩家執黑）
    OnlineGame          // 線上對戰
};
```

### 引擎初始化 (Engine Initialization)

```cpp
ChessEngine::ChessEngine(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_gameMode(GameMode::HumanVsHuman)
    , m_skillLevel(0)       // 預設初學者難度
    , m_thinkingTimeMs(50)  // 預設 50ms
    , m_searchDepth(1)      // 預設深度 1
    , m_isReady(false)
    , m_isThinking(false)
{
}
```

---

## UCI 協議 (UCI Protocol)

### UCI 簡介 (UCI Introduction)

UCI (Universal Chess Interface) 是標準的西洋棋引擎通訊協議。它使用文字命令進行通訊。

**主要命令**：
- `uci` - 初始化引擎
- `isready` - 檢查引擎是否就緒
- `ucinewgame` - 開始新遊戲
- `position` - 設定棋盤位置
- `go` - 開始計算
- `stop` - 停止計算
- `quit` - 退出引擎

### 啟動引擎 (Start Engine)

```cpp
bool ChessEngine::startEngine(const QString& enginePath) {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        stopEngine();
    }
    
    // 檢查引擎檔案是否存在
    QFileInfo engineInfo(enginePath);
    if (!engineInfo.exists()) {
        emit engineError(QString("引擎檔案不存在：%1").arg(enginePath));
        return false;
    }
    
    m_enginePath = enginePath;
    m_process = new QProcess(this);
    
    // 連接訊號
    connect(m_process, &QProcess::readyReadStandardOutput, 
            this, &ChessEngine::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, 
            this, &ChessEngine::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ChessEngine::onEngineFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &ChessEngine::onEngineError);
    
    // 啟動引擎進程
    m_process->start(enginePath, QStringList());
    
    if (!m_process->waitForStarted(5000)) {
        emit engineError(QString("無法啟動引擎：%1").arg(m_process->errorString()));
        delete m_process;
        m_process = nullptr;
        return false;
    }
    
    // 初始化 UCI 協議
    sendCommand("uci");
    
    return true;
}
```

### 發送命令 (Send Command)

```cpp
void ChessEngine::sendCommand(const QString& command) {
    if (m_process && m_process->state() == QProcess::Running) {
        qDebug() << "[Engine] Sending:" << command;
        m_process->write((command + "\n").toUtf8());
        m_process->waitForBytesWritten();
    }
}
```

### 接收輸出 (Receive Output)

```cpp
void ChessEngine::onReadyReadStandardOutput() {
    if (!m_process) return;
    
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        qDebug() << "[Engine] Received:" << line;
        parseOutput(line);
    }
}

void ChessEngine::parseOutput(const QString& line) {
    // UCI 初始化完成
    if (line == "uciok") {
        sendCommand("isready");
        return;
    }
    
    // 引擎就緒
    if (line == "readyok") {
        if (!m_isReady) {
            m_isReady = true;
            configureEngine();  // 配置引擎參數
            emit engineReady();
        }
        return;
    }
    
    // 最佳移動
    if (line.startsWith("bestmove")) {
        QStringList parts = line.split(" ");
        if (parts.size() >= 2) {
            m_bestMove = parts[1];
            m_isThinking = false;
            emit thinkingStopped();
            emit bestMoveFound(m_bestMove);
        }
        return;
    }
    
    // 引擎資訊（可選）
    if (line.startsWith("info")) {
        // 可以解析搜尋資訊、評估值等
        // 例如：info depth 10 score cp 50 nodes 12345 ...
    }
}
```

### 配置引擎 (Configure Engine)

```cpp
void ChessEngine::configureEngine() {
    if (!m_isReady) return;
    
    // 設定技能等級（0-20）
    sendCommand(QString("setoption name Skill Level value %1").arg(m_skillLevel));
    
    // 設定多執行緒（可選）
    sendCommand("setoption name Threads value 1");
    
    // 設定記憶體（可選）
    sendCommand("setoption name Hash value 128");  // 128 MB
    
    qDebug() << "[Engine] Configured with skill level:" << m_skillLevel;
}
```

---

## 難度設定 (Difficulty Settings)

### 技能等級 (Skill Level)

Stockfish 支援 0-20 的技能等級：

```cpp
enum class EngineDifficulty {
    VeryEasy = 1,    // 等級 1 - 非常簡單
    Easy = 5,        // 等級 5 - 簡單
    Medium = 10,     // 等級 10 - 中等
    Hard = 15,       // 等級 15 - 困難
    VeryHard = 20    // 等級 20 - 非常困難
};

void ChessEngine::setDifficulty(int level) {
    m_skillLevel = qBound(0, level, 20);
    configureEngine();
}
```

### 思考時間 (Thinking Time)

```cpp
void ChessEngine::setThinkingTime(int milliseconds) {
    m_thinkingTimeMs = qMax(10, milliseconds);  // 最少 10ms
}
```

### 搜尋深度 (Search Depth)

```cpp
void ChessEngine::setSearchDepth(int depth) {
    m_searchDepth = qBound(1, depth, 30);  // 1-30 層
}
```

### 難度與參數關係 (Difficulty vs Parameters)

| 技能等級 | 思考時間 | 搜尋深度 | 棋力描述 |
|---------|---------|---------|---------|
| 0-5     | 10-50ms | 1-3     | 初學者 |
| 6-10    | 50-200ms| 3-6     | 中級玩家 |
| 11-15   | 200-500ms| 6-10   | 高級玩家 |
| 16-20   | 500ms+  | 10-20   | 專家級 |

---

## 移動生成 (Move Generation)

### 請求移動 (Request Move)

```cpp
void ChessEngine::requestMove() {
    if (!m_isReady || m_isThinking) {
        return;
    }
    
    m_isThinking = true;
    emit thinkingStarted();
    
    // 構建 go 命令
    QString goCommand = "go";
    
    // 根據設定選擇搜尋方式
    if (m_searchDepth > 0) {
        // 使用固定深度
        goCommand += QString(" depth %1").arg(m_searchDepth);
    } else {
        // 使用時間限制
        goCommand += QString(" movetime %1").arg(m_thinkingTimeMs);
    }
    
    sendCommand(goCommand);
}
```

### 設定棋盤位置 (Set Position)

#### 使用 FEN 格式

```cpp
void ChessEngine::setPosition(const QString& fen) {
    if (!m_isReady) return;
    
    m_currentPosition = fen;
    sendCommand(QString("position fen %1").arg(fen));
}
```

#### 使用移動列表

```cpp
void ChessEngine::setPositionFromMoves(const QStringList& moves) {
    if (!m_isReady) return;
    
    QString command = "position startpos";
    if (!moves.isEmpty()) {
        command += " moves " + moves.join(" ");
    }
    
    sendCommand(command);
}
```

### 新遊戲 (New Game)

```cpp
void ChessEngine::newGame() {
    if (!m_isReady) return;
    
    sendCommand("ucinewgame");
    sendCommand("position startpos");
    m_currentPosition = "startpos";
}
```

---

## 引擎控制 (Engine Control)

### 停止思考 (Stop Thinking)

```cpp
void ChessEngine::stop() {
    if (m_isThinking) {
        sendCommand("stop");
        m_isThinking = false;
        emit thinkingStopped();
    }
}
```

### 停止引擎 (Stop Engine)

```cpp
void ChessEngine::stopEngine() {
    if (m_process) {
        if (m_isThinking) {
            sendCommand("stop");
            m_isThinking = false;
        }
        
        sendCommand("quit");
        
        // 等待引擎正常退出
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
        
        delete m_process;
        m_process = nullptr;
    }
    
    m_isReady = false;
    m_isThinking = false;
}

ChessEngine::~ChessEngine() {
    stopEngine();
}
```

### 錯誤處理 (Error Handling)

```cpp
void ChessEngine::onEngineError(QProcess::ProcessError error) {
    QString errorStr;
    switch (error) {
        case QProcess::FailedToStart:
            errorStr = "引擎啟動失敗";
            break;
        case QProcess::Crashed:
            errorStr = "引擎崩潰";
            break;
        case QProcess::Timedout:
            errorStr = "引擎超時";
            break;
        case QProcess::WriteError:
            errorStr = "引擎寫入錯誤";
            break;
        case QProcess::ReadError:
            errorStr = "引擎讀取錯誤";
            break;
        default:
            errorStr = "引擎未知錯誤";
            break;
    }
    
    emit engineError(errorStr);
}

void ChessEngine::onEngineFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit) {
        emit engineError(QString("引擎異常退出，退出碼：%1").arg(exitCode));
    }
    m_isReady = false;
    m_isThinking = false;
}
```

---

## FEN 與 UCI 轉換 (FEN and UCI Conversion)

### FEN 格式 (FEN Format)

FEN (Forsyth-Edwards Notation) 是棋盤位置的文字表示法：

```
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

**組成部分**：
1. 棋盤位置（從第 8 橫列到第 1 橫列）
2. 當前玩家（w=白方，b=黑方）
3. 王車易位權利（KQkq）
4. 吃過路兵目標方格
5. 半回合計數（50步規則）
6. 完整回合數

### 棋盤轉 FEN (Board to FEN)

```cpp
QString ChessEngine::boardToFEN(const ChessBoard& board) {
    QString fen;
    
    // 1. 棋盤位置
    for (int row = 0; row < 8; ++row) {
        int emptyCount = 0;
        
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = board.getPiece(row, col);
            
            if (piece.getType() == PieceType::None) {
                emptyCount++;
            } else {
                // 先寫出空格數量
                if (emptyCount > 0) {
                    fen += QString::number(emptyCount);
                    emptyCount = 0;
                }
                
                // 寫出棋子符號
                char symbol = pieceToFENChar(piece);
                fen += symbol;
            }
        }
        
        // 行尾的空格
        if (emptyCount > 0) {
            fen += QString::number(emptyCount);
        }
        
        // 除了最後一行，加上斜線
        if (row < 7) {
            fen += "/";
        }
    }
    
    // 2. 當前玩家
    fen += (board.getCurrentPlayer() == PieceColor::White) ? " w " : " b ";
    
    // 3. 王車易位權利
    QString castling;
    if (canWhiteKingsideCastle(board)) castling += "K";
    if (canWhiteQueensideCastle(board)) castling += "Q";
    if (canBlackKingsideCastle(board)) castling += "k";
    if (canBlackQueensideCastle(board)) castling += "q";
    fen += castling.isEmpty() ? "-" : castling;
    fen += " ";
    
    // 4. 吃過路兵目標
    QPoint enPassant = board.getEnPassantTarget();
    if (enPassant.x() >= 0 && enPassant.y() >= 0) {
        fen += squareToAlgebraic(enPassant);
    } else {
        fen += "-";
    }
    
    // 5. 半回合計數和完整回合數
    fen += " 0 1";  // 簡化版本
    
    return fen;
}

char ChessEngine::pieceToFENChar(const ChessPiece& piece) {
    char symbol;
    switch (piece.getType()) {
        case PieceType::King:   symbol = 'k'; break;
        case PieceType::Queen:  symbol = 'q'; break;
        case PieceType::Rook:   symbol = 'r'; break;
        case PieceType::Bishop: symbol = 'b'; break;
        case PieceType::Knight: symbol = 'n'; break;
        case PieceType::Pawn:   symbol = 'p'; break;
        default:                symbol = ' '; break;
    }
    
    // 白方用大寫
    if (piece.getColor() == PieceColor::White) {
        symbol = toupper(symbol);
    }
    
    return symbol;
}
```

### UCI 移動格式 (UCI Move Format)

UCI 使用長代數記譜法：`e2e4`, `e7e5`, `e1g1` (王車易位), `e7e8q` (兵升變)

```cpp
QString ChessEngine::moveToUCI(const QPoint& from, const QPoint& to, 
                                PieceType promotionType) {
    QString uci;
    
    // 起始方格（例如 e2）
    uci += QChar('a' + from.x());
    uci += QChar('1' + (7 - from.y()));
    
    // 目標方格（例如 e4）
    uci += QChar('a' + to.x());
    uci += QChar('1' + (7 - to.y()));
    
    // 兵升變
    if (promotionType != PieceType::None) {
        switch (promotionType) {
            case PieceType::Queen:  uci += 'q'; break;
            case PieceType::Rook:   uci += 'r'; break;
            case PieceType::Bishop: uci += 'b'; break;
            case PieceType::Knight: uci += 'n'; break;
            default: break;
        }
    }
    
    return uci;
}
```

### UCI 轉移動 (UCI to Move)

```cpp
void ChessEngine::uciToMove(const QString& uci, QPoint& from, QPoint& to, 
                             PieceType& promotionType) {
    if (uci.length() < 4) {
        from = QPoint(-1, -1);
        to = QPoint(-1, -1);
        promotionType = PieceType::None;
        return;
    }
    
    // 解析起始方格（例如 e2）
    from.setX(uci[0].toLatin1() - 'a');
    from.setY(7 - (uci[1].toLatin1() - '1'));
    
    // 解析目標方格（例如 e4）
    to.setX(uci[2].toLatin1() - 'a');
    to.setY(7 - (uci[3].toLatin1() - '1'));
    
    // 解析升變（如果有）
    promotionType = PieceType::None;
    if (uci.length() >= 5) {
        QChar promotion = uci[4].toLower();
        switch (promotion.toLatin1()) {
            case 'q': promotionType = PieceType::Queen;  break;
            case 'r': promotionType = PieceType::Rook;   break;
            case 'b': promotionType = PieceType::Bishop; break;
            case 'n': promotionType = PieceType::Knight; break;
            default:  break;
        }
    }
}
```

---

## 效能優化 (Performance Optimization)

### 1. 非同步處理 (Asynchronous Processing)

使用 Qt 訊號/槽機制實現非同步通訊：

```cpp
// 在 Qt_Chess 中
void Qt_Chess::onComputerTurn() {
    if (!m_chessEngine->isEngineRunning()) {
        return;
    }
    
    // 設定當前棋盤位置
    QString fen = ChessEngine::boardToFEN(m_chessBoard);
    m_chessEngine->setPosition(fen);
    
    // 請求移動（非阻塞）
    m_chessEngine->requestMove();
    
    // 顯示思考提示
    m_statusLabel->setText("電腦思考中...");
}

void Qt_Chess::onBestMoveFound(const QString& move) {
    // 解析 UCI 移動
    QPoint from, to;
    PieceType promotion;
    ChessEngine::uciToMove(move, from, to, promotion);
    
    // 執行移動
    bool success = m_chessBoard.movePiece(from, to);
    
    if (success) {
        // 處理兵升變
        if (m_chessBoard.needsPromotion(to)) {
            m_chessBoard.promotePawn(to, promotion);
        }
        
        // 更新 UI
        update();
        updateStatus();
    }
}
```

### 2. 引擎快取 (Engine Caching)

保持引擎運行，避免重複啟動：

```cpp
// 在應用程式啟動時初始化引擎
void Qt_Chess::initializeEngine() {
    QString enginePath = QCoreApplication::applicationDirPath() + "/engine/stockfish";
    
    #ifdef Q_OS_WIN
        enginePath += ".exe";
    #endif
    
    if (m_chessEngine->startEngine(enginePath)) {
        qDebug() << "引擎初始化成功";
    } else {
        qWarning() << "引擎初始化失敗";
    }
}

// 只在應用程式關閉時停止引擎
Qt_Chess::~Qt_Chess() {
    m_chessEngine->stopEngine();
}
```

### 3. 難度自適應 (Adaptive Difficulty)

根據玩家水平自動調整：

```cpp
void Qt_Chess::adjustDifficulty(bool playerWon) {
    int currentDifficulty = m_chessEngine->getDifficulty();
    
    if (playerWon) {
        // 玩家贏了，提高難度
        m_chessEngine->setDifficulty(qMin(20, currentDifficulty + 2));
    } else {
        // 玩家輸了，降低難度
        m_chessEngine->setDifficulty(qMax(0, currentDifficulty - 1));
    }
}
```

### 4. 記憶體管理 (Memory Management)

```cpp
void ChessEngine::configureEngine() {
    if (!m_isReady) return;
    
    // 根據系統記憶體調整 Hash 大小
    int hashSize = 128;  // MB
    
    #ifdef Q_OS_WIN
        // Windows 可以使用更多記憶體
        hashSize = 256;
    #endif
    
    sendCommand(QString("setoption name Hash value %1").arg(hashSize));
    
    // 限制執行緒數（避免影響 UI）
    sendCommand("setoption name Threads value 1");
}
```

---

## 使用範例 (Usage Examples)

### 基本使用 (Basic Usage)

```cpp
// 1. 創建引擎實例
ChessEngine* engine = new ChessEngine(this);

// 2. 連接訊號
connect(engine, &ChessEngine::engineReady, 
        this, &Qt_Chess::onEngineReady);
connect(engine, &ChessEngine::bestMoveFound, 
        this, &Qt_Chess::onBestMoveFound);
connect(engine, &ChessEngine::engineError, 
        this, &Qt_Chess::onEngineError);

// 3. 啟動引擎
QString enginePath = "/path/to/stockfish";
engine->startEngine(enginePath);

// 4. 設定難度
engine->setDifficulty(10);  // 中等難度

// 5. 開始新遊戲
engine->newGame();

// 6. 設定棋盤位置
QString fen = ChessEngine::boardToFEN(chessBoard);
engine->setPosition(fen);

// 7. 請求移動
engine->requestMove();

// 8. 接收最佳移動（在槽函數中）
void Qt_Chess::onBestMoveFound(const QString& move) {
    // 解析並執行移動
    QPoint from, to;
    PieceType promotion;
    ChessEngine::uciToMove(move, from, to, promotion);
    chessBoard.movePiece(from, to);
}
```

### 高級使用 (Advanced Usage)

```cpp
// 自訂搜尋參數
engine->setSearchDepth(15);     // 搜尋 15 層
engine->setThinkingTime(5000);  // 最多思考 5 秒

// 使用移動歷史設定位置
QStringList moves;
moves << "e2e4" << "e7e5" << "g1f3" << "b8c6";
engine->setPositionFromMoves(moves);

// 立即停止思考
engine->stop();

// 檢查引擎狀態
if (engine->isEngineRunning()) {
    qDebug() << "引擎運行中";
}
```

---

## 疑難排解 (Troubleshooting)

### 常見問題 (Common Issues)

1. **引擎啟動失敗**
   - 檢查引擎檔案是否存在
   - 檢查檔案權限（Linux/Mac 需要執行權限）
   - 檢查路徑是否正確

2. **引擎無回應**
   - 檢查是否發送了 `uci` 命令
   - 檢查是否收到了 `uciok` 回應
   - 使用 `isready` 命令確認引擎狀態

3. **移動解析錯誤**
   - 檢查 UCI 格式是否正確
   - 檢查座標轉換是否正確
   - 確認 FEN 字串格式正確

4. **效能問題**
   - 降低搜尋深度
   - 減少思考時間
   - 限制 Hash 大小
   - 使用單執行緒模式

---

## 相關文件 (Related Documentation)

- [遊戲規則文件](GAME_RULES_DOCUMENTATION.md)
- [骰子模式文件](DICE_MODE_DOCUMENTATION.md)
- [伺服器文件](SERVER_DOCUMENTATION.md)
- [Stockfish 官方文件](https://stockfishchess.org/)
- [UCI 協議規範](http://wbec-ridderkerk.nl/html/UCIProtocol.html)
