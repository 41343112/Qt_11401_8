#include "chessengine.h"
#include "chessboard.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

ChessEngine::ChessEngine(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_gameMode(GameMode::HumanVsHuman)
    , m_skillLevel(10)  // 預設中等難度
    , m_thinkingTimeMs(1000)  // 預設 1 秒思考時間
    , m_isReady(false)
    , m_isThinking(false)
{
}

ChessEngine::~ChessEngine()
{
    stopEngine();
}

bool ChessEngine::startEngine(const QString& enginePath)
{
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
    
    connect(m_process, &QProcess::readyReadStandardOutput, 
            this, &ChessEngine::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, 
            this, &ChessEngine::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ChessEngine::onEngineFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &ChessEngine::onEngineError);

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

void ChessEngine::stopEngine()
{
    if (m_process) {
        if (m_isThinking) {
            sendCommand("stop");
            m_isThinking = false;
        }
        sendCommand("quit");
        
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

bool ChessEngine::isEngineRunning() const
{
    return m_process && m_process->state() == QProcess::Running && m_isReady;
}

void ChessEngine::setGameMode(GameMode mode)
{
    m_gameMode = mode;
}

void ChessEngine::setDifficulty(int level)
{
    m_skillLevel = qBound(0, level, 20);
    configureEngine();
}

void ChessEngine::setThinkingTime(int milliseconds)
{
    m_thinkingTimeMs = qBound(50, milliseconds, 30000);
}

void ChessEngine::newGame()
{
    if (!isEngineRunning()) return;
    
    sendCommand("ucinewgame");
    m_currentPosition.clear();
    m_bestMove.clear();
}

void ChessEngine::setPosition(const QString& fen)
{
    if (!isEngineRunning()) return;
    
    m_currentPosition = fen;
    sendCommand(QString("position fen %1").arg(fen));
}

void ChessEngine::setPositionFromMoves(const QStringList& moves)
{
    if (!isEngineRunning()) return;
    
    if (moves.isEmpty()) {
        sendCommand("position startpos");
    } else {
        sendCommand(QString("position startpos moves %1").arg(moves.join(" ")));
    }
}

void ChessEngine::requestMove()
{
    if (!isEngineRunning()) return;
    
    m_isThinking = true;
    m_bestMove.clear();
    emit thinkingStarted();
    
    // 使用 movetime 限制思考時間
    sendCommand(QString("go movetime %1").arg(m_thinkingTimeMs));
}

void ChessEngine::stop()
{
    if (m_isThinking && m_process) {
        sendCommand("stop");
    }
}

void ChessEngine::sendCommand(const QString& command)
{
    if (m_process && m_process->state() == QProcess::Running) {
        QString cmd = command + "\n";
        m_process->write(cmd.toUtf8());
        m_process->waitForBytesWritten(100);
    }
}

void ChessEngine::onReadyReadStandardOutput()
{
    if (!m_process) return;
    
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        parseOutput(line);
    }
}

void ChessEngine::onReadyReadStandardError()
{
    if (!m_process) return;
    
    QString error = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    if (!error.isEmpty()) {
        qWarning() << "Chess engine error:" << error;
    }
}

void ChessEngine::onEngineFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    
    m_isReady = false;
    m_isThinking = false;
}

void ChessEngine::onEngineError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "引擎無法啟動";
            break;
        case QProcess::Crashed:
            errorMsg = "引擎崩潰";
            break;
        case QProcess::Timedout:
            errorMsg = "引擎超時";
            break;
        case QProcess::WriteError:
            errorMsg = "寫入引擎時發生錯誤";
            break;
        case QProcess::ReadError:
            errorMsg = "讀取引擎輸出時發生錯誤";
            break;
        default:
            errorMsg = "未知錯誤";
            break;
    }
    
    emit engineError(errorMsg);
    m_isReady = false;
    m_isThinking = false;
}

void ChessEngine::parseOutput(const QString& line)
{
    if (line.isEmpty()) return;
    
    // UCI 協議回應
    if (line == "uciok") {
        // UCI 初始化完成，配置引擎
        configureEngine();
        sendCommand("isready");
    }
    else if (line == "readyok") {
        m_isReady = true;
        emit engineReady();
    }
    else if (line.startsWith("bestmove")) {
        // 解析最佳走法
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            m_bestMove = parts[1];
            m_isThinking = false;
            emit thinkingStopped();
            emit bestMoveFound(m_bestMove);
        }
    }
    // 可以添加更多解析，如 info 資訊用於顯示引擎思考
}

void ChessEngine::configureEngine()
{
    if (!m_process || m_process->state() != QProcess::Running) return;
    
    // 設定 Stockfish 的技能等級（1-20）
    // 注意：此選項名稱為 Stockfish 專用，其他引擎可能使用不同名稱
    sendCommand(QString("setoption name Skill Level value %1").arg(m_skillLevel));
    
    // 設定較小的 hash 表（16MB）以節省記憶體
    // 對於休閒遊戲來說足夠，同時不會佔用太多系統資源
    sendCommand("setoption name Hash value 16");
    
    // 禁用 Ponder（預測對手走法）以簡化遊戲體驗
    sendCommand("setoption name Ponder value false");
}

// 靜態工具函數

QString ChessEngine::boardToFEN(const ChessBoard& board)
{
    QString fen;
    
    // 棋盤位置
    for (int row = 0; row < 8; ++row) {
        int emptyCount = 0;
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = board.getPiece(row, col);
            if (piece.getType() == PieceType::None) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += QString::number(emptyCount);
                    emptyCount = 0;
                }
                
                char pieceChar = ' ';
                switch (piece.getType()) {
                    case PieceType::King:   pieceChar = 'k'; break;
                    case PieceType::Queen:  pieceChar = 'q'; break;
                    case PieceType::Rook:   pieceChar = 'r'; break;
                    case PieceType::Bishop: pieceChar = 'b'; break;
                    case PieceType::Knight: pieceChar = 'n'; break;
                    case PieceType::Pawn:   pieceChar = 'p'; break;
                    default: break;
                }
                
                if (piece.getColor() == PieceColor::White) {
                    pieceChar = QChar(pieceChar).toUpper().toLatin1();
                }
                fen += pieceChar;
            }
        }
        if (emptyCount > 0) {
            fen += QString::number(emptyCount);
        }
        if (row < 7) {
            fen += '/';
        }
    }
    
    // 當前玩家
    fen += ' ';
    fen += (board.getCurrentPlayer() == PieceColor::White) ? 'w' : 'b';
    
    // 王車易位權利（簡化：檢查國王和車是否移動過）
    QString castling;
    const ChessPiece& whiteKing = board.getPiece(7, 4);
    const ChessPiece& blackKing = board.getPiece(0, 4);
    const ChessPiece& whiteRookK = board.getPiece(7, 7);
    const ChessPiece& whiteRookQ = board.getPiece(7, 0);
    const ChessPiece& blackRookK = board.getPiece(0, 7);
    const ChessPiece& blackRookQ = board.getPiece(0, 0);
    
    if (whiteKing.getType() == PieceType::King && !whiteKing.hasMoved()) {
        if (whiteRookK.getType() == PieceType::Rook && !whiteRookK.hasMoved()) {
            castling += 'K';
        }
        if (whiteRookQ.getType() == PieceType::Rook && !whiteRookQ.hasMoved()) {
            castling += 'Q';
        }
    }
    if (blackKing.getType() == PieceType::King && !blackKing.hasMoved()) {
        if (blackRookK.getType() == PieceType::Rook && !blackRookK.hasMoved()) {
            castling += 'k';
        }
        if (blackRookQ.getType() == PieceType::Rook && !blackRookQ.hasMoved()) {
            castling += 'q';
        }
    }
    
    fen += ' ';
    fen += castling.isEmpty() ? "-" : castling;
    
    // 吃過路兵目標
    fen += ' ';
    QPoint enPassant = board.getEnPassantTarget();
    if (enPassant.x() >= 0 && enPassant.y() >= 0) {
        char file = 'a' + enPassant.x();
        char rank = '8' - enPassant.y();
        fen += file;
        fen += rank;
    } else {
        fen += '-';
    }
    
    // 半步計數和全步計數（簡化）
    fen += " 0 1";
    
    return fen;
}

QString ChessEngine::moveToUCI(const QPoint& from, const QPoint& to, PieceType promotionType)
{
    // UCI 格式：e2e4 或帶升變 e7e8q
    QString uci;
    uci += QChar('a' + from.x());  // 起始列
    uci += QChar('8' - from.y());  // 起始行
    uci += QChar('a' + to.x());    // 目標列
    uci += QChar('8' - to.y());    // 目標行
    
    // 升變
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

void ChessEngine::uciToMove(const QString& uci, QPoint& from, QPoint& to, PieceType& promotionType)
{
    if (uci.length() < 4) {
        from = QPoint(-1, -1);
        to = QPoint(-1, -1);
        promotionType = PieceType::None;
        return;
    }
    
    // 解析起始位置
    int fromCol = uci[0].toLatin1() - 'a';
    int fromRow = '8' - uci[1].toLatin1();
    from = QPoint(fromCol, fromRow);
    
    // 解析目標位置
    int toCol = uci[2].toLatin1() - 'a';
    int toRow = '8' - uci[3].toLatin1();
    to = QPoint(toCol, toRow);
    
    // 解析升變
    promotionType = PieceType::None;
    if (uci.length() >= 5) {
        QChar promChar = uci[4].toLower();
        if (promChar == 'q') promotionType = PieceType::Queen;
        else if (promChar == 'r') promotionType = PieceType::Rook;
        else if (promChar == 'b') promotionType = PieceType::Bishop;
        else if (promChar == 'n') promotionType = PieceType::Knight;
    }
}
