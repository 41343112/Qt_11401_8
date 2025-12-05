#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>

// Network constants
static const int MIN_ROOM_NUMBER = 1000;
static const int MAX_ROOM_NUMBER = 9999;
static const int PORT_BASE = 10000;
static const int MIN_AUTO_PORT = 10000;
static const int MAX_AUTO_PORT = 20000;

// Default relay server settings (can be configured)
// For local testing, use "localhost" or "127.0.0.1"
// For production, change to your actual relay server address
static const QString DEFAULT_RELAY_SERVER = "localhost";
static const int DEFAULT_RELAY_PORT = 8080;

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_clientSocket(nullptr)
    , m_relayClient(nullptr)
    , m_relayServerUrl(DEFAULT_RELAY_SERVER)
    , m_relayServerPort(DEFAULT_RELAY_PORT)
    , m_connectionMode(ConnectionMode::Relay)  // Default to relay mode
    , m_role(NetworkRole::None)
    , m_status(ConnectionStatus::Disconnected)
    , m_port(0)
    , m_playerColor(PieceColor::None)
    , m_opponentColor(PieceColor::None)
{
    // Initialize relay client
    m_relayClient = new RelayClient(this);
    connect(m_relayClient, &RelayClient::connected, this, &NetworkManager::onRelayConnected);
    connect(m_relayClient, &RelayClient::disconnected, this, &NetworkManager::onRelayDisconnected);
    connect(m_relayClient, &RelayClient::error, this, &NetworkManager::onRelayError);
    connect(m_relayClient, &RelayClient::roomCreated, this, &NetworkManager::onRelayRoomCreated);
    connect(m_relayClient, &RelayClient::roomJoined, this, &NetworkManager::onRelayRoomJoined);
    connect(m_relayClient, &RelayClient::opponentJoined, this, &NetworkManager::onRelayOpponentJoined);
    connect(m_relayClient, &RelayClient::opponentLeft, this, &NetworkManager::onRelayOpponentLeft);
    connect(m_relayClient, &RelayClient::gameMessageReceived, this, &NetworkManager::onRelayGameMessageReceived);
}

NetworkManager::~NetworkManager()
{
    closeConnection();
}

bool NetworkManager::createRoom(quint16 port)
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    // 創建服務器
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
    connect(m_server, &QTcpServer::acceptError, this, &NetworkManager::onServerError);
    
    // 生成房號
    m_roomNumber = generateRoomNumber();
    
    // 根據房號計算端口（PORT_BASE + 房號）
    port = PORT_BASE + m_roomNumber.toInt();
    
    // 監聽所有網絡接口
    if (!m_server->listen(QHostAddress::Any, port)) {
        emit connectionError(tr("無法創建房間: ") + m_server->errorString());
        delete m_server;
        m_server = nullptr;
        return false;
    }
    
    m_port = m_server->serverPort();
    m_role = NetworkRole::Server;
    m_status = ConnectionStatus::Connected;
    m_playerColor = PieceColor::White;  // 房主執白
    m_opponentColor = PieceColor::Black;
    
    emit roomCreated(m_roomNumber, m_port);
    return true;
}

bool NetworkManager::joinRoom(const QString& hostAddress, quint16 port)
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    m_connectionMode = ConnectionMode::Direct;
    
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onError);
    
    m_role = NetworkRole::Client;
    m_status = ConnectionStatus::Connecting;
    m_playerColor = PieceColor::Black;  // 加入者執黑
    m_opponentColor = PieceColor::White;
    
    m_socket->connectToHost(hostAddress, port);
    return true;
}

bool NetworkManager::createRoomViaRelay()
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    m_connectionMode = ConnectionMode::Relay;
    m_role = NetworkRole::Server;
    m_status = ConnectionStatus::Connecting;
    m_playerColor = PieceColor::White;  // 房主執白
    m_opponentColor = PieceColor::Black;
    
    // Connect to relay server first
    if (!m_relayClient->isConnected()) {
        if (!m_relayClient->connectToRelay(m_relayServerUrl, m_relayServerPort)) {
            emit connectionError(tr("無法連接到中繼伺服器"));
            m_status = ConnectionStatus::Disconnected;
            return false;
        }
    } else {
        // Already connected, create room immediately
        m_relayClient->createRoom();
    }
    
    return true;
}

bool NetworkManager::joinRoomViaRelay(const QString& roomCode)
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    m_connectionMode = ConnectionMode::Relay;
    m_role = NetworkRole::Client;
    m_status = ConnectionStatus::Connecting;
    m_playerColor = PieceColor::Black;  // 加入者執黑
    m_opponentColor = PieceColor::White;
    
    m_roomNumber = roomCode;
    
    // Connect to relay server first
    if (!m_relayClient->isConnected()) {
        if (!m_relayClient->connectToRelay(m_relayServerUrl, m_relayServerPort)) {
            emit connectionError(tr("無法連接到中繼伺服器"));
            m_status = ConnectionStatus::Disconnected;
            return false;
        }
    } else {
        // Already connected, join room immediately
        m_relayClient->joinRoom(roomCode);
    }
    
    return true;
}

void NetworkManager::setRelayServer(const QString& serverUrl, quint16 port)
{
    m_relayServerUrl = serverUrl;
    m_relayServerPort = port;
}

void NetworkManager::closeConnection()
{
    // 發送斷線通知（如果有連接）
    if (m_clientSocket || m_socket) {
        QJsonObject message;
        message["type"] = messageTypeToString(MessageType::PlayerDisconnected);
        sendMessage(message);
        
        // 確保訊息發送完成
        if (m_clientSocket) {
            m_clientSocket->flush();
        }
        if (m_socket) {
            m_socket->flush();
        }
    }
    
    // Close relay connection if using relay mode
    if (m_connectionMode == ConnectionMode::Relay && m_relayClient) {
        m_relayClient->leaveRoom();
        m_relayClient->disconnectFromRelay();
    }
    
    // 先關閉並刪除 server，確保不再接受新連接
    if (m_server) {
        m_server->close();
        m_server->deleteLater();  // 使用 deleteLater 確保安全刪除
        m_server = nullptr;
    }
    
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    // 完整重置所有狀態
    m_role = NetworkRole::None;
    m_status = ConnectionStatus::Disconnected;
    m_roomNumber.clear();
    m_port = 0;
    m_playerColor = PieceColor::None;      // 重置玩家顏色
    m_opponentColor = PieceColor::None;    // 重置對手顏色
}

void NetworkManager::sendMove(const QPoint& from, const QPoint& to, PieceType promotionType)
{
    qDebug() << "[NetworkManager::sendMove] Sending move from" << from << "to" << to 
             << "| Role:" << (m_role == NetworkRole::Server ? "Server" : "Client")
             << "| Socket connected:" << (getActiveSocket() && getActiveSocket()->state() == QAbstractSocket::ConnectedState);
    
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::Move);
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
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::GameStart);
    message["playerColor"] = static_cast<int>(playerColor);
    
    sendMessage(message);
}

void NetworkManager::sendStartGame(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor)
{
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::StartGame);
    message["whiteTimeMs"] = whiteTimeMs;
    message["blackTimeMs"] = blackTimeMs;
    message["incrementMs"] = incrementMs;
    message["hostColor"] = (hostColor == PieceColor::White) ? "White" : "Black";
    sendMessage(message);
}

void NetworkManager::sendTimeSettings(int whiteTimeMs, int blackTimeMs, int incrementMs)
{
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::TimeSettings);
    message["whiteTimeMs"] = whiteTimeMs;
    message["blackTimeMs"] = blackTimeMs;
    message["incrementMs"] = incrementMs;
    sendMessage(message);
}

void NetworkManager::sendSurrender()
{
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::Surrender);
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
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::GameOver);
    message["result"] = result;
    
    sendMessage(message);
}

void NetworkManager::sendChat(const QString& message)
{
    QJsonObject jsonMessage;
    jsonMessage["type"] = messageTypeToString(MessageType::Chat);
    jsonMessage["message"] = message;
    
    sendMessage(jsonMessage);
}

void NetworkManager::onNewConnection()
{
    if (!m_server || m_clientSocket) {
        return;  // 已經有連接
    }
    
    m_clientSocket = m_server->nextPendingConnection();
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_clientSocket, &QTcpSocket::errorOccurred, this, &NetworkManager::onError);
    
    emit opponentJoined();
    
    // 等待客戶端發送 JoinRoom 消息，然後在 processMessage 中處理
}

void NetworkManager::onConnected()
{
    m_status = ConnectionStatus::Connected;
    emit connected();
    
    // 客戶端發送加入房間請求
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::JoinRoom);
    sendMessage(message);
}

void NetworkManager::onDisconnected()
{
    emit disconnected();
    emit opponentDisconnected();
    
    if (m_role == NetworkRole::Client) {
        m_status = ConnectionStatus::Disconnected;
    }
}

void NetworkManager::onReadyRead()
{
    QTcpSocket* socket = getActiveSocket();
    if (!socket) {
        return;
    }
    
    m_receiveBuffer.append(socket->readAll());
    
    // 處理所有完整的消息
    while (true) {
        int messageEnd = m_receiveBuffer.indexOf('\n');
        if (messageEnd == -1) {
            break;  // 沒有完整的消息
        }
        
        QByteArray messageData = m_receiveBuffer.left(messageEnd);
        m_receiveBuffer.remove(0, messageEnd + 1);
        
        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (!doc.isNull() && doc.isObject()) {
            processMessage(doc.object());
        }
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        emit connectionError(socket->errorString());
    }
    m_status = ConnectionStatus::Error;
}

void NetworkManager::onServerError(QAbstractSocket::SocketError socketError)
{
    if (m_server) {
        emit connectionError(m_server->errorString());
    }
}

void NetworkManager::sendMessage(const QJsonObject& message)
{
    // Use relay if in relay mode
    if (m_connectionMode == ConnectionMode::Relay) {
        if (m_relayClient && m_relayClient->isConnected()) {
            qDebug() << "[NetworkManager::sendMessage] Sending via relay:" << message["type"].toString();
            m_relayClient->sendGameMessage(message);
        } else {
            qDebug() << "[NetworkManager::sendMessage] ERROR: Relay not connected";
        }
        return;
    }
    
    // Direct connection mode
    QTcpSocket* socket = getActiveSocket();
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[NetworkManager::sendMessage] ERROR: Cannot send message, socket not connected"
                 << "| Socket:" << socket 
                 << "| State:" << (socket ? socket->state() : -1);
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    
    qDebug() << "[NetworkManager::sendMessage] Sending message:" << message["type"].toString();
    
    socket->write(data);
    socket->flush();
}

void NetworkManager::processMessage(const QJsonObject& message)
{
    QString typeStr = message["type"].toString();
    MessageType type = stringToMessageType(typeStr);
    
    qDebug() << "[NetworkManager::processMessage] Received message:" << typeStr 
             << "| Role:" << (m_role == NetworkRole::Server ? "Server" : "Client");
    
    switch (type) {
    case MessageType::JoinRoom:
        // 服務器收到加入請求
        if (m_role == NetworkRole::Server) {
            QJsonObject response;
            response["type"] = messageTypeToString(MessageType::JoinAccepted);
            sendMessage(response);
            
            // 發送遊戲開始消息給客戶端
            sendGameStart(m_playerColor);
        }
        break;
        
    case MessageType::JoinAccepted:
        // 客戶端收到加入接受
        emit opponentJoined();
        
        // 客戶端也發送遊戲開始確認給服務器
        sendGameStart(m_playerColor);
        break;
        
    case MessageType::GameStart: {
        PieceColor opponentColor = static_cast<PieceColor>(message["playerColor"].toInt());
        // 對手的顏色就是我們的對手顏色
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
            if (m_role == NetworkRole::Server) {
                // 房主：使用自己選擇的顏色
                m_playerColor = hostColor;
                m_opponentColor = (hostColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            } else if (m_role == NetworkRole::Client) {
                // 房客：使用與房主相反的顏色
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
        
    default:
        break;
    }
}

QString NetworkManager::generateRoomNumber() const
{
    // 生成4位數字房號 (MIN_ROOM_NUMBER to MAX_ROOM_NUMBER inclusive)
    int roomNum = QRandomGenerator::global()->bounded(MIN_ROOM_NUMBER, MAX_ROOM_NUMBER + 1);
    return QString::number(roomNum);
}

MessageType NetworkManager::stringToMessageType(const QString& type) const
{
    static QMap<QString, MessageType> typeMap = {
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

QTcpSocket* NetworkManager::getActiveSocket() const
{
    if (m_role == NetworkRole::Server) {
        return m_clientSocket;
    } else {
        return m_socket;
    }
}

// Relay server slot implementations

void NetworkManager::onRelayConnected()
{
    qDebug() << "[NetworkManager] Connected to relay server";
    
    // If we were trying to create or join a room, do it now
    if (m_role == NetworkRole::Server) {
        m_relayClient->createRoom();
    } else if (m_role == NetworkRole::Client && !m_roomNumber.isEmpty()) {
        m_relayClient->joinRoom(m_roomNumber);
    }
}

void NetworkManager::onRelayDisconnected()
{
    qDebug() << "[NetworkManager] Disconnected from relay server";
    emit disconnected();
}

void NetworkManager::onRelayError(const QString& error)
{
    qDebug() << "[NetworkManager] Relay error:" << error;
    emit connectionError(error);
    m_status = ConnectionStatus::Error;
}

void NetworkManager::onRelayRoomCreated(const QString& roomCode)
{
    qDebug() << "[NetworkManager] Room created via relay:" << roomCode;
    m_roomNumber = roomCode;
    m_status = ConnectionStatus::Connected;
    m_port = 0;  // No port for relay mode
    emit roomCreated(m_roomNumber, m_port);
}

void NetworkManager::onRelayRoomJoined(const QString& roomCode)
{
    qDebug() << "[NetworkManager] Joined room via relay:" << roomCode;
    m_roomNumber = roomCode;
    m_status = ConnectionStatus::Connected;
    emit connected();
    
    // Send join request to start game protocol
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::JoinRoom);
    sendMessage(message);
}

void NetworkManager::onRelayOpponentJoined(const QString& opponentId)
{
    qDebug() << "[NetworkManager] Opponent joined via relay:" << opponentId;
    emit opponentJoined();
    
    // Server sends game start message
    if (m_role == NetworkRole::Server) {
        QJsonObject response;
        response["type"] = messageTypeToString(MessageType::JoinAccepted);
        sendMessage(response);
        
        // Send game start message
        sendGameStart(m_playerColor);
    }
}

void NetworkManager::onRelayOpponentLeft()
{
    qDebug() << "[NetworkManager] Opponent left via relay";
    emit opponentDisconnected();
}

void NetworkManager::onRelayGameMessageReceived(const QJsonObject& message)
{
    qDebug() << "[NetworkManager] Game message received via relay";
    processMessage(message);
}
