#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QDebug>

// Server configuration
static const QString SERVER_URL = "wss://chess-server-mjg6.onrender.com";

// Room number constants
static const int MIN_ROOM_NUMBER = 1000;
static const int MAX_ROOM_NUMBER = 9999;

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
    
    // 生成房號（4位數字）
    m_roomNumber = generateRoomNumber();
    
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
    qDebug() << "[NetworkManager] Generated room number:" << m_roomNumber;
    
    // 立即發送房號給UI顯示
    emit roomCreated(m_roomNumber);
    
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
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::Move);
    message["roomNumber"] = m_roomNumber;
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
    message["type"] = messageTypeToString(MessageType::StartGame);
    message["roomNumber"] = m_roomNumber;
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
        // 房主請求創建房間，發送客戶端生成的房號
        QJsonObject message;
        message["type"] = messageTypeToString(MessageType::CreateRoom);
        message["roomNumber"] = m_roomNumber;  // 發送客戶端生成的房號
        sendMessage(message);
        qDebug() << "[NetworkManager] Sent CreateRoom request with room number:" << m_roomNumber;
    } else if (m_role == NetworkRole::Guest) {
        // 加入者請求加入房間
        QJsonObject message;
        message["type"] = messageTypeToString(MessageType::JoinRoom);
        message["roomNumber"] = m_roomNumber;
        sendMessage(message);
        qDebug() << "[NetworkManager] Sent JoinRoom request for room:" << m_roomNumber;
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
    QString typeStr = message["type"].toString();
    MessageType type = stringToMessageType(typeStr);
    
    qDebug() << "[NetworkManager::processMessage] Received message:" << typeStr 
             << "| Role:" << (m_role == NetworkRole::Host ? "Host" : "Guest");
    
    switch (type) {
    case MessageType::RoomCreated:
        // 服務器確認創建的房間號（如果服務器返回不同的房號則更新）
        if (m_role == NetworkRole::Host) {
            QString serverRoomNumber = message["roomNumber"].toString();
            if (!serverRoomNumber.isEmpty() && serverRoomNumber != m_roomNumber) {
                qDebug() << "[NetworkManager] Server assigned different room number:" << serverRoomNumber;
                m_roomNumber = serverRoomNumber;
                emit roomCreated(m_roomNumber);
            } else {
                qDebug() << "[NetworkManager] Room created confirmed with number:" << m_roomNumber;
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

QString NetworkManager::generateRoomNumber() const
{
    // 生成4位數字房號 (MIN_ROOM_NUMBER to MAX_ROOM_NUMBER inclusive)
    int roomNum = QRandomGenerator::global()->bounded(MIN_ROOM_NUMBER, MAX_ROOM_NUMBER + 1);
    return QString::number(roomNum);
}
