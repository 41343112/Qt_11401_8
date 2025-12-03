#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QNetworkInterface>
#include <QHostAddress>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_clientSocket(nullptr)
    , m_role(NetworkRole::None)
    , m_status(ConnectionStatus::Disconnected)
    , m_port(0)
    , m_playerColor(PieceColor::None)
    , m_opponentColor(PieceColor::None)
{
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
    
    // 如果未指定端口，使用隨機端口
    if (port == 0) {
        port = QRandomGenerator::global()->bounded(10000, 60000);
    }
    
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
    m_roomNumber = generateRoomNumber();
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
    if (m_server) {
        m_server->close();
        delete m_server;
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
    
    m_role = NetworkRole::None;
    m_status = ConnectionStatus::Disconnected;
    m_roomNumber.clear();
    m_port = 0;
}

void NetworkManager::sendMove(const QPoint& from, const QPoint& to, PieceType promotionType)
{
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
}

void NetworkManager::sendGameStart(PieceColor playerColor)
{
    QJsonObject message;
    message["type"] = messageTypeToString(MessageType::GameStart);
    message["playerColor"] = static_cast<int>(playerColor);
    
    sendMessage(message);
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
    
    // 發送遊戲開始消息
    sendGameStart(m_playerColor);
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
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    
    socket->write(data);
    socket->flush();
}

void NetworkManager::processMessage(const QJsonObject& message)
{
    QString typeStr = message["type"].toString();
    MessageType type = stringToMessageType(typeStr);
    
    switch (type) {
    case MessageType::JoinRoom:
        // 服務器收到加入請求
        if (m_role == NetworkRole::Server) {
            QJsonObject response;
            response["type"] = messageTypeToString(MessageType::JoinAccepted);
            sendMessage(response);
        }
        break;
        
    case MessageType::JoinAccepted:
        // 客戶端收到加入接受
        emit opponentJoined();
        break;
        
    case MessageType::GameStart: {
        PieceColor opponentColor = static_cast<PieceColor>(message["playerColor"].toInt());
        // 對手的顏色就是我們的對手顏色
        m_opponentColor = opponentColor;
        m_playerColor = (opponentColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        emit gameStartReceived(m_playerColor);
        break;
    }
        
    case MessageType::Move: {
        int fromRow = message["fromRow"].toInt();
        int fromCol = message["fromCol"].toInt();
        int toRow = message["toRow"].toInt();
        int toCol = message["toCol"].toInt();
        
        QPoint from(fromCol, fromRow);
        QPoint to(toCol, toRow);
        
        PieceType promotionType = PieceType::None;
        if (message.contains("promotion")) {
            promotionType = static_cast<PieceType>(message["promotion"].toInt());
        }
        
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
    // 生成4位數字房號
    int roomNum = QRandomGenerator::global()->bounded(1000, 10000);
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
        {"Move", MessageType::Move},
        {"GameOver", MessageType::GameOver},
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
        {MessageType::Move, "Move"},
        {MessageType::GameOver, "GameOver"},
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
