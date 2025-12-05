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
static const quint16 DISCOVERY_PORT = 45678;  // UDP 探索端口
static const int DISCOVERY_TIMEOUT_MS = 3000;  // 探索超時時間（3秒）

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_clientSocket(nullptr)
    , m_discoverySocket(nullptr)
    , m_discoveryTimer(nullptr)
    , m_role(NetworkRole::None)
    , m_status(ConnectionStatus::Disconnected)
    , m_port(0)
    , m_playerColor(PieceColor::None)
    , m_opponentColor(PieceColor::None)
{
    // 初始化 UDP 探索套接字
    m_discoverySocket = new QUdpSocket(this);
    m_discoverySocket->bind(QHostAddress::Any, DISCOVERY_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(m_discoverySocket, &QUdpSocket::readyRead, this, &NetworkManager::onDiscoveryReadyRead);
    
    // 初始化探索超時計時器
    m_discoveryTimer = new QTimer(this);
    m_discoveryTimer->setSingleShot(true);
    connect(m_discoveryTimer, &QTimer::timeout, this, &NetworkManager::onDiscoveryTimeout);
}

NetworkManager::~NetworkManager()
{
    closeConnection();
    stopDiscovery();
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

// UDP 探索相關實現
bool NetworkManager::discoverAndJoinRoom(const QString& roomNumber)
{
    if (m_status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    qDebug() << "[NetworkManager::discoverAndJoinRoom] Discovering room:" << roomNumber;
    
    m_searchingRoomNumber = roomNumber;
    m_status = ConnectionStatus::Connecting;
    
    // 發送探索請求
    sendDiscoveryRequest(roomNumber);
    
    // 啟動超時計時器
    m_discoveryTimer->start(DISCOVERY_TIMEOUT_MS);
    
    return true;
}

void NetworkManager::sendDiscoveryRequest(const QString& roomNumber)
{
    QJsonObject request;
    request["type"] = "DISCOVER_ROOM";
    request["roomNumber"] = roomNumber;
    
    QJsonDocument doc(request);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    // 發送廣播到本地網路
    m_discoverySocket->writeDatagram(data, QHostAddress::Broadcast, DISCOVERY_PORT);
    
    qDebug() << "[NetworkManager::sendDiscoveryRequest] Sent discovery request for room:" << roomNumber;
}

void NetworkManager::sendDiscoveryResponse(const QHostAddress& sender, quint16 senderPort)
{
    QJsonObject response;
    response["type"] = "ROOM_FOUND";
    response["roomNumber"] = m_roomNumber;
    response["port"] = m_port;
    
    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    // 發送回應給請求者
    m_discoverySocket->writeDatagram(data, sender, senderPort);
    
    qDebug() << "[NetworkManager::sendDiscoveryResponse] Sent discovery response to" << sender.toString();
}

void NetworkManager::onDiscoveryReadyRead()
{
    while (m_discoverySocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_discoverySocket->pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        
        m_discoverySocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
        QJsonDocument doc = QJsonDocument::fromJson(datagram);
        if (!doc.isObject()) {
            continue;
        }
        
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();
        
        if (type == "DISCOVER_ROOM") {
            // 收到探索請求
            QString requestedRoom = obj["roomNumber"].toString();
            
            qDebug() << "[NetworkManager::onDiscoveryReadyRead] Received discovery request for room:" << requestedRoom 
                     << "from" << sender.toString();
            
            // 如果是我們的房間，回應
            if (m_role == NetworkRole::Server && requestedRoom == m_roomNumber) {
                qDebug() << "[NetworkManager::onDiscoveryReadyRead] Room matches! Sending response.";
                sendDiscoveryResponse(sender, senderPort);
            }
        }
        else if (type == "ROOM_FOUND") {
            // 收到房間回應
            QString foundRoom = obj["roomNumber"].toString();
            quint16 foundPort = obj["port"].toInt();
            
            qDebug() << "[NetworkManager::onDiscoveryReadyRead] Received room found response:"
                     << "room=" << foundRoom << "port=" << foundPort << "from" << sender.toString();
            
            // 如果是我們要找的房間
            if (foundRoom == m_searchingRoomNumber) {
                qDebug() << "[NetworkManager::onDiscoveryReadyRead] Found the room we're looking for!";
                
                stopDiscovery();
                
                // 發送探索成功信號
                emit roomDiscovered(sender.toString(), foundPort);
                
                // 自動連接到找到的房間
                if (joinRoom(sender.toString(), foundPort)) {
                    qDebug() << "[NetworkManager::onDiscoveryReadyRead] Joining discovered room...";
                } else {
                    qDebug() << "[NetworkManager::onDiscoveryReadyRead] Failed to join discovered room!";
                    emit connectionError(tr("無法連接到探索到的房間"));
                }
            }
        }
    }
}

void NetworkManager::onDiscoveryTimeout()
{
    qDebug() << "[NetworkManager::onDiscoveryTimeout] Discovery timeout for room:" << m_searchingRoomNumber;
    
    stopDiscovery();
    m_status = ConnectionStatus::Disconnected;
    
    emit connectionError(tr("找不到房號 %1 的房間\n\n請確認：\n1. 房主已創建房間\n2. 雙方在同一WiFi網路\n3. 房號輸入正確").arg(m_searchingRoomNumber));
}

void NetworkManager::startDiscovery()
{
    // 確保 UDP socket 已綁定
    if (!m_discoverySocket->isOpen()) {
        m_discoverySocket->bind(QHostAddress::Any, DISCOVERY_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    }
}

void NetworkManager::stopDiscovery()
{
    if (m_discoveryTimer->isActive()) {
        m_discoveryTimer->stop();
    }
    m_searchingRoomNumber.clear();
}
