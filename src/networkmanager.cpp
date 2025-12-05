#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

// Server configuration
static const QString SERVER_URL = "wss://chess-server-mjg6.onrender.com";

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_webSocket(nullptr)
    , m_role(NetworkRole::None)
    , m_status(ConnectionStatus::Disconnected)
    , m_serverUrl(SERVER_URL)
    , m_playerColor(PieceColor::None)
    , m_opponentColor(PieceColor::None)
{
}

NetworkManager::~NetworkManager()
{
    closeConnection();
}

bool NetworkManager::createRoom()
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    // 不生成臨時房號，完全依賴伺服器分配
    m_roomNumber.clear();
    
    // 創建 WebSocket 連接
    m_webSocket = new QWebSocket();
    connect(m_webSocket, &QWebSocket::connected, this, &NetworkManager::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
            this, &NetworkManager::onError);
    
    m_role = NetworkRole::Host;
    m_status = ConnectionStatus::Connecting;
    m_playerColor = PieceColor::White;  // 房主執白
    m_opponentColor = PieceColor::Black;
    
    qDebug() << "[NetworkManager] Connecting to server:" << m_serverUrl;
    qDebug() << "[NetworkManager] Waiting for server to assign room number";
    
    // 等待伺服器回應分配房號
    
    m_webSocket->open(QUrl(m_serverUrl));
    return true;
}

bool NetworkManager::joinRoom(const QString& roomNumber)
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    // 創建 WebSocket 連接
    m_webSocket = new QWebSocket();
    connect(m_webSocket, &QWebSocket::connected, this, &NetworkManager::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
            this, &NetworkManager::onError);
    
    m_role = NetworkRole::Guest;
    m_status = ConnectionStatus::Connecting;
    m_roomNumber = roomNumber;
    m_playerColor = PieceColor::Black;  // 加入者執黑
    m_opponentColor = PieceColor::White;
    
    qDebug() << "[NetworkManager] Connecting to server:" << m_serverUrl << "to join room:" << roomNumber;
    m_webSocket->open(QUrl(m_serverUrl));
    return true;
}

void NetworkManager::closeConnection()
{
    // 發送斷線通知（如果有連接）
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject message;
        message["type"] = messageTypeToString(MessageType::PlayerDisconnected);
        sendMessage(message);
        
        // 確保訊息發送完成
        m_webSocket->flush();
    }
    
    if (m_webSocket) {
        m_webSocket->close();
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }
    
    // 完整重置所有狀態
    m_role = NetworkRole::None;
    m_status = ConnectionStatus::Disconnected;
    m_roomNumber.clear();
    m_playerColor = PieceColor::None;
    m_opponentColor = PieceColor::None;
}

void NetworkManager::sendMove(const QPoint& from, const QPoint& to, PieceType promotionType)
{
    qDebug() << "[NetworkManager::sendMove] Sending move from" << from << "to" << to 
             << "| Role:" << (m_role == NetworkRole::Host ? "Host" : "Guest")
             << "| Socket connected:" << (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState);
    
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendMove] ERROR: Room number is empty, cannot send move";
        return;
    }
    
    // 使用伺服器期望的格式
    QJsonObject message;
    message["action"] = "move";
    message["room"] = m_roomNumber;
    message["fromRow"] = from.y();
    message["fromCol"] = from.x();
    message["toRow"] = to.y();
    message["toCol"] = to.x();
    
    if (promotionType != PieceType::None) {
        message["promotion"] = static_cast<int>(promotionType);
    }
    
    sendMessage(message);
    
    qDebug() << "[NetworkManager::sendMove] Move sent successfully";
}

void NetworkManager::sendGameStart(PieceColor playerColor)
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendGameStart] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::GameStart);
    message["roomNumber"] = m_roomNumber;
    message["playerColor"] = static_cast<int>(playerColor);
    
    sendMessage(message);
}

void NetworkManager::sendStartGame(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor)
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendStartGame] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["action"] = "startGame";  // Server expects "action" field
    message["room"] = m_roomNumber;  // Server expects "room" field
    message["whiteTimeMs"] = whiteTimeMs;
    message["blackTimeMs"] = blackTimeMs;
    message["incrementMs"] = incrementMs;
    message["hostColor"] = (hostColor == PieceColor::White) ? "White" : "Black";
    sendMessage(message);
}

void NetworkManager::sendTimeSettings(int whiteTimeMs, int blackTimeMs, int incrementMs)
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendTimeSettings] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::TimeSettings);
    message["roomNumber"] = m_roomNumber;
    message["whiteTimeMs"] = whiteTimeMs;
    message["blackTimeMs"] = blackTimeMs;
    message["incrementMs"] = incrementMs;
    sendMessage(message);
}

void NetworkManager::sendSurrender()
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendSurrender] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::Surrender);
    message["roomNumber"] = m_roomNumber;
    sendMessage(message);
}

void NetworkManager::setPlayerColors(PieceColor playerColor)
{
    // 設定玩家顏色和對手顏色
    m_playerColor = playerColor;
    m_opponentColor = (playerColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

void NetworkManager::sendGameOver(const QString& result)
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendGameOver] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::GameOver);
    message["roomNumber"] = m_roomNumber;
    message["result"] = result;
    
    sendMessage(message);
}

void NetworkManager::sendChat(const QString& chatMessage)
{
    if (m_roomNumber.isEmpty()) {
        qDebug() << "[NetworkManager::sendChat] ERROR: Room number is empty";
        return;
    }
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::Chat);
    message["roomNumber"] = m_roomNumber;
    message["message"] = chatMessage;
    
    sendMessage(message);
}

void NetworkManager::onConnected()
{
    qDebug() << "[NetworkManager] Connected to server";
    m_status = ConnectionStatus::Connected;
    emit connected();
    
    // 根據角色發送相應的請求
    if (m_role == NetworkRole::Host) {
        // 房主請求創建房間
        QJsonObject message;
        message["action"] = "createRoom";  // 使用伺服器期望的格式
        sendMessage(message);
        qDebug() << "[NetworkManager] Sent createRoom request";
    } else if (m_role == NetworkRole::Guest) {
        // 加入者請求加入房間
        QJsonObject message;
        message["action"] = "joinRoom";  // 使用伺服器期望的格式
        message["room"] = m_roomNumber;  // 使用 "room" 而不是 "roomNumber"
        sendMessage(message);
        qDebug() << "[NetworkManager] Sent joinRoom request for room:" << m_roomNumber;
    }
}

void NetworkManager::onDisconnected()
{
    qDebug() << "[NetworkManager] Disconnected from server";
    emit disconnected();
    emit opponentDisconnected();
    
    m_status = ConnectionStatus::Disconnected;
}

void NetworkManager::onTextMessageReceived(const QString& message)
{
    qDebug() << "[NetworkManager] Received message:" << message;
    
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        processMessage(doc.object());
    } else {
        qDebug() << "[NetworkManager] Failed to parse message as JSON";
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString;
    if (m_webSocket) {
        errorString = m_webSocket->errorString();
    } else {
        errorString = tr("WebSocket error: %1").arg(socketError);
    }
    
    qDebug() << "[NetworkManager] Socket error:" << errorString;
    emit connectionError(errorString);
    m_status = ConnectionStatus::Error;
}

void NetworkManager::sendMessage(const QJsonObject& message)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[NetworkManager::sendMessage] ERROR: Cannot send message, socket not connected"
                 << "| Socket:" << m_webSocket 
                 << "| State:" << (m_webSocket ? m_webSocket->state() : -1);
        return;
    }
    
    QJsonDocument doc(message);
    QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    
    qDebug() << "[NetworkManager::sendMessage] Sending message:" << message["type"].toString();
    
    m_webSocket->sendTextMessage(jsonString);
    m_webSocket->flush();
}

void NetworkManager::processMessage(const QJsonObject& message)
{
    // 檢查是伺服器格式 (action) 還是舊格式 (type)
    QString actionStr = message["action"].toString();
    
    qDebug() << "[NetworkManager::processMessage] Received message with action:" << actionStr 
             << "| Role:" << (m_role == NetworkRole::Host ? "Host" : "Guest");
    
    // 處理伺服器的回應格式
    if (actionStr == "roomCreated") {
        // 伺服器確認創建的房間號
        if (m_role == NetworkRole::Host) {
            QString serverRoomNumber = message["room"].toString();
            if (!serverRoomNumber.isEmpty()) {
                qDebug() << "[NetworkManager] Server created room with number:" << serverRoomNumber;
                m_roomNumber = serverRoomNumber;
                emit roomCreated(m_roomNumber);
            } else {
                qDebug() << "[NetworkManager] Server response missing room number";
            }
        }
    }
    else if (actionStr == "playerJoined") {
        // 房主收到玩家加入通知
        if (m_role == NetworkRole::Host) {
            qDebug() << "[NetworkManager] Host notified: player joined room";
            emit opponentJoined();
        }
    }
    else if (actionStr == "joinedRoom") {
        // 加入房間成功
        qDebug() << "[NetworkManager] Joined room successfully";
        emit opponentJoined();
        
        // 發送遊戲開始確認（使用舊協議格式）
        sendGameStart(m_playerColor);
    }
    else if (actionStr == "gameStart") {
        // 伺服器廣播遊戲開始（同步開始）
        qDebug() << "[NetworkManager] Server broadcast gameStart";
        
        int whiteTimeMs = message["whiteTimeMs"].toInt();
        int blackTimeMs = message["blackTimeMs"].toInt();
        int incrementMs = message["incrementMs"].toInt();
        QString hostColorStr = message["hostColor"].toString();
        PieceColor hostColor = (hostColorStr == "White") ? PieceColor::White : PieceColor::Black;
        qint64 serverTimestamp = message["serverTimestamp"].toVariant().toLongLong();
        
        // 根據房主的顏色選擇更新玩家顏色
        if (m_role == NetworkRole::Host) {
            m_playerColor = hostColor;
            m_opponentColor = (hostColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        } else if (m_role == NetworkRole::Guest) {
            m_playerColor = (hostColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            m_opponentColor = hostColor;
        }
        
        qDebug() << "[NetworkManager] Game starting with server timestamp:" << serverTimestamp
                 << "| Host color:" << hostColorStr
                 << "| My role:" << (m_role == NetworkRole::Host ? "Host" : "Guest")
                 << "| My color:" << (m_playerColor == PieceColor::White ? "White" : "Black");
        
        emit startGameReceived(whiteTimeMs, blackTimeMs, incrementMs, hostColor);
    }
    else if (actionStr == "error") {
        // 伺服器錯誤
        QString errorMsg = message["message"].toString();
        qDebug() << "[NetworkManager] Server error:" << errorMsg;
        emit connectionError(tr("伺服器錯誤: ") + errorMsg);
        closeConnection();
    }
    else if (actionStr == "move") {
        // 收到對手的移動
        QString roomId = message["room"].toString();
        int fromRow = message["fromRow"].toInt();
        int fromCol = message["fromCol"].toInt();
        int toRow = message["toRow"].toInt();
        int toCol = message["toCol"].toInt();
        
        qDebug() << "[NetworkManager::processMessage] Move in room" << roomId 
                 << ": from (" << fromCol << "," << fromRow 
                 << ") to (" << toCol << "," << toRow << ")";
        
        QPoint from(fromCol, fromRow);
        QPoint to(toCol, toRow);
        
        PieceType promotionType = PieceType::None;
        if (message.contains("promotion")) {
            promotionType = static_cast<PieceType>(message["promotion"].toInt());
        }
        
        qDebug() << "[NetworkManager::processMessage] Emitting opponentMove signal";
        emit opponentMove(from, to, promotionType);
    }
    else {
        // 處理舊格式訊息（type 欄位）
        QString typeStr = message["type"].toString();
        if (typeStr.isEmpty()) {
            qDebug() << "[NetworkManager] Unknown message format:" << message;
            return;
        }
        
        MessageType type = stringToMessageType(typeStr);
        
        qDebug() << "[NetworkManager::processMessage] Received old format message:" << typeStr;
        
        switch (type) {
        case MessageType::RoomCreated:
            // 舊格式：伺服器確認創建的房間號
            if (m_role == NetworkRole::Host) {
                QString serverRoomNumber = message["roomNumber"].toString();
                if (!serverRoomNumber.isEmpty()) {
                    qDebug() << "[NetworkManager] Server created room with number:" << serverRoomNumber;
                    m_roomNumber = serverRoomNumber;
                    emit roomCreated(m_roomNumber);
                } else {
                    qDebug() << "[NetworkManager] Server response missing room number";
                }
            }
            break;
            
        case MessageType::JoinAccepted:
            // 加入房間成功
            qDebug() << "[NetworkManager] Join accepted";
            emit opponentJoined();
            
            // 發送遊戲開始確認
            sendGameStart(m_playerColor);
            break;
            
        case MessageType::JoinRejected:
            // 加入房間失敗
            {
                QString reason = message["reason"].toString();
                qDebug() << "[NetworkManager] Join rejected:" << reason;
                emit connectionError(tr("無法加入房間: ") + reason);
                closeConnection();
            }
            break;
    
    case MessageType::GameStart: {
        PieceColor opponentColor = static_cast<PieceColor>(message["playerColor"].toInt());
        m_opponentColor = opponentColor;
        m_playerColor = (opponentColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        emit gameStartReceived(m_playerColor);
        break;
    }
    
    case MessageType::StartGame:
        // 收到房主的開始遊戲通知（包含時間設定和顏色選擇）
        {
            int whiteTimeMs = message["whiteTimeMs"].toInt();
            int blackTimeMs = message["blackTimeMs"].toInt();
            int incrementMs = message["incrementMs"].toInt();
            QString hostColorStr = message["hostColor"].toString();
            PieceColor hostColor = (hostColorStr == "White") ? PieceColor::White : PieceColor::Black;
            
            // 根據房主的顏色選擇更新玩家顏色
            if (m_role == NetworkRole::Host) {
                m_playerColor = hostColor;
                m_opponentColor = (hostColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            } else if (m_role == NetworkRole::Guest) {
                m_playerColor = (hostColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
                m_opponentColor = hostColor;
            }
            
            emit startGameReceived(whiteTimeMs, blackTimeMs, incrementMs, hostColor);
        }
        break;
    
    case MessageType::TimeSettings:
        // 收到房主的時間設定更新
        {
            int whiteTimeMs = message["whiteTimeMs"].toInt();
            int blackTimeMs = message["blackTimeMs"].toInt();
            int incrementMs = message["incrementMs"].toInt();
            emit timeSettingsReceived(whiteTimeMs, blackTimeMs, incrementMs);
        }
        break;
    
    case MessageType::Surrender:
        // 收到對手投降訊息
        emit surrenderReceived();
        break;
        
    case MessageType::Move: {
        int fromRow = message["fromRow"].toInt();
        int fromCol = message["fromCol"].toInt();
        int toRow = message["toRow"].toInt();
        int toCol = message["toCol"].toInt();
        
        qDebug() << "[NetworkManager::processMessage] Move message: from (" << fromCol << "," << fromRow 
                 << ") to (" << toCol << "," << toRow << ")";
        
        QPoint from(fromCol, fromRow);
        QPoint to(toCol, toRow);
        
        PieceType promotionType = PieceType::None;
        if (message.contains("promotion")) {
            promotionType = static_cast<PieceType>(message["promotion"].toInt());
        }
        
        qDebug() << "[NetworkManager::processMessage] Emitting opponentMove signal";
        emit opponentMove(from, to, promotionType);
        break;
    }
        
    case MessageType::GameOver: {
        QString result = message["result"].toString();
        emit gameOverReceived(result);
        break;
    }
        
    case MessageType::Chat: {
        QString chatMessage = message["message"].toString();
        emit chatReceived(chatMessage);
        break;
    }
    
    case MessageType::PlayerDisconnected:
        // 對手斷線
        qDebug() << "[NetworkManager] Opponent disconnected";
        emit opponentDisconnected();
        break;
        
    default:
        qDebug() << "[NetworkManager] Unknown message type:" << typeStr;
        break;
        }
    }
}

MessageType NetworkManager::stringToMessageType(const QString& type) const
{
    static QMap<QString, MessageType> typeMap = {
        {"CreateRoom", MessageType::CreateRoom},
        {"RoomCreated", MessageType::RoomCreated},
        {"JoinRoom", MessageType::JoinRoom},
        {"JoinAccepted", MessageType::JoinAccepted},
        {"JoinRejected", MessageType::JoinRejected},
        {"GameStart", MessageType::GameStart},
        {"StartGame", MessageType::StartGame},
        {"TimeSettings", MessageType::TimeSettings},
        {"Move", MessageType::Move},
        {"GameOver", MessageType::GameOver},
        {"Surrender", MessageType::Surrender},
        {"Chat", MessageType::Chat},
        {"PlayerDisconnected", MessageType::PlayerDisconnected},
        {"Ping", MessageType::Ping},
        {"Pong", MessageType::Pong}
    };
    
    return typeMap.value(type, MessageType::Ping);
}

QString NetworkManager::messageTypeToString(MessageType type) const
{
    static QMap<MessageType, QString> stringMap = {
        {MessageType::CreateRoom, "CreateRoom"},
        {MessageType::RoomCreated, "RoomCreated"},
        {MessageType::JoinRoom, "JoinRoom"},
        {MessageType::JoinAccepted, "JoinAccepted"},
        {MessageType::JoinRejected, "JoinRejected"},
        {MessageType::GameStart, "GameStart"},
        {MessageType::StartGame, "StartGame"},
        {MessageType::TimeSettings, "TimeSettings"},
        {MessageType::Move, "Move"},
        {MessageType::GameOver, "GameOver"},
        {MessageType::Surrender, "Surrender"},
        {MessageType::Chat, "Chat"},
        {MessageType::PlayerDisconnected, "PlayerDisconnected"},
        {MessageType::Ping, "Ping"},
        {MessageType::Pong, "Pong"}
    };
    
    return stringMap.value(type, "Unknown");
}
