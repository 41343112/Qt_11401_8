#include "networkmanager.h"
#include <QDataStream>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_role(NetworkRole::None)
    , m_localPlayerColor(PieceColor::None)
{
}

NetworkManager::~NetworkManager()
{
    stopServer();
    disconnectFromServer();
}

bool NetworkManager::startServer(quint16 port)
{
    if (m_server && m_server->isListening()) {
        return true;
    }

    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        QString error = m_server->errorString();
        delete m_server;
        m_server = nullptr;
        emit connectionError(tr("無法啟動伺服器: %1").arg(error));
        return false;
    }

    m_role = NetworkRole::Server;
    qDebug() << "伺服器已啟動，監聽端口:" << port;
    return true;
}

void NetworkManager::stopServer()
{
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    m_role = NetworkRole::None;
}

quint16 NetworkManager::getServerPort() const
{
    if (m_server && m_server->isListening()) {
        return m_server->serverPort();
    }
    return 0;
}

bool NetworkManager::connectToServer(const QString& host, quint16 port)
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        return true;
    }

    if (m_socket) {
        m_socket->deleteLater();
    }

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onSocketError);

    m_role = NetworkRole::Client;
    m_socket->connectToHost(host, port);
    
    qDebug() << "正在連接到伺服器:" << host << ":" << port;
    return true;
}

void NetworkManager::disconnectFromServer()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QTcpSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    if (m_role == NetworkRole::Client) {
        m_role = NetworkRole::None;
    }
}

QString NetworkManager::getRemoteAddress() const
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        return m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());
    }
    return QString();
}

void NetworkManager::onNewConnection()
{
    if (!m_server) return;

    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    if (!clientSocket) return;

    // 如果已經有連接，拒絕新連接
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        qDebug() << "拒絕新連接，已有客戶端連接";
        clientSocket->close();
        clientSocket->deleteLater();
        return;
    }

    // 接受新連接
    if (m_socket) {
        m_socket->deleteLater();
    }

    m_socket = clientSocket;
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onSocketError);

    qDebug() << "客戶端已連接:" << m_socket->peerAddress().toString();
    emit connected();
}

void NetworkManager::onReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    m_readBuffer.append(QString::fromUtf8(data));

    // 處理所有完整的訊息（以換行符分隔）
    int newlinePos;
    while ((newlinePos = m_readBuffer.indexOf('\n')) != -1) {
        QString message = m_readBuffer.left(newlinePos);
        m_readBuffer = m_readBuffer.mid(newlinePos + 1);
        
        if (!message.isEmpty()) {
            processMessage(message);
        }
    }
}

void NetworkManager::onDisconnected()
{
    qDebug() << "連接已斷開";
    emit disconnected();
    
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void NetworkManager::onSocketError(QAbstractSocket::SocketError error)
{
    if (m_socket) {
        QString errorString = m_socket->errorString();
        qDebug() << "Socket 錯誤:" << errorString;
        emit connectionError(errorString);
    }
}

void NetworkManager::sendMessage(const NetworkMessage& message)
{
    if (!m_socket || m_socket->state() != QTcpSocket::ConnectedState) {
        qDebug() << "無法發送訊息：未連接";
        return;
    }

    QString encoded = encodeMessage(message);
    QByteArray data = encoded.toUtf8() + "\n";
    
    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        qDebug() << "發送訊息失敗:" << m_socket->errorString();
    } else {
        m_socket->flush();
    }
}

QString NetworkManager::encodeMessage(const NetworkMessage& message) const
{
    QJsonObject json;
    
    // 編碼訊息類型
    QString typeStr;
    switch (message.type) {
        case NetworkMessageType::Move:
            typeStr = "move";
            break;
        case NetworkMessageType::GameStart:
            typeStr = "game_start";
            break;
        case NetworkMessageType::GameEnd:
            typeStr = "game_end";
            break;
        case NetworkMessageType::Chat:
            typeStr = "chat";
            break;
        case NetworkMessageType::Resign:
            typeStr = "resign";
            break;
        case NetworkMessageType::NewGame:
            typeStr = "new_game";
            break;
        case NetworkMessageType::Handshake:
            typeStr = "handshake";
            break;
        case NetworkMessageType::Ping:
            typeStr = "ping";
            break;
        case NetworkMessageType::PlayerInfo:
            typeStr = "player_info";
            break;
    }
    
    json["type"] = typeStr;
    json["data"] = message.data;
    
    return QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
}

NetworkMessage NetworkManager::decodeMessage(const QString& data) const
{
    NetworkMessage message;
    
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "無效的 JSON 訊息";
        return message;
    }
    
    QJsonObject json = doc.object();
    QString typeStr = json["type"].toString();
    
    // 解碼訊息類型
    if (typeStr == "move") {
        message.type = NetworkMessageType::Move;
    } else if (typeStr == "game_start") {
        message.type = NetworkMessageType::GameStart;
    } else if (typeStr == "game_end") {
        message.type = NetworkMessageType::GameEnd;
    } else if (typeStr == "chat") {
        message.type = NetworkMessageType::Chat;
    } else if (typeStr == "resign") {
        message.type = NetworkMessageType::Resign;
    } else if (typeStr == "new_game") {
        message.type = NetworkMessageType::NewGame;
    } else if (typeStr == "handshake") {
        message.type = NetworkMessageType::Handshake;
    } else if (typeStr == "ping") {
        message.type = NetworkMessageType::Ping;
    } else if (typeStr == "player_info") {
        message.type = NetworkMessageType::PlayerInfo;
    }
    
    message.data = json["data"].toString();
    
    return message;
}

void NetworkManager::processMessage(const QString& messageData)
{
    NetworkMessage message = decodeMessage(messageData);
    
    switch (message.type) {
        case NetworkMessageType::Move: {
            // 解析移動資料: "fromRow,fromCol,toRow,toCol,promotionType"
            QStringList parts = message.data.split(',');
            if (parts.size() >= 4) {
                QPoint from(parts[0].toInt(), parts[1].toInt());
                QPoint to(parts[2].toInt(), parts[3].toInt());
                PieceType promotionType = PieceType::None;
                
                if (parts.size() >= 5) {
                    promotionType = static_cast<PieceType>(parts[4].toInt());
                }
                
                emit moveReceived(from, to, promotionType);
            }
            break;
        }
        
        case NetworkMessageType::GameStart: {
            // 解析遊戲開始資料: "playerColor"
            int colorInt = message.data.toInt();
            PieceColor remoteColor = static_cast<PieceColor>(colorInt);
            emit gameStartReceived(remoteColor);
            break;
        }
        
        case NetworkMessageType::GameEnd:
            emit gameEndReceived(message.data);
            break;
            
        case NetworkMessageType::Chat:
            emit chatReceived(message.data);
            break;
            
        case NetworkMessageType::Resign:
            emit resignReceived();
            break;
            
        case NetworkMessageType::NewGame:
            emit newGameRequested();
            break;
            
        case NetworkMessageType::PlayerInfo:
            emit playerInfoReceived(message.data);
            break;
            
        case NetworkMessageType::Handshake:
        case NetworkMessageType::Ping:
            // 處理握手和心跳
            break;
    }
}

void NetworkManager::sendMove(const QPoint& from, const QPoint& to, PieceType promotionType)
{
    NetworkMessage message;
    message.type = NetworkMessageType::Move;
    message.data = QString("%1,%2,%3,%4,%5")
                       .arg(from.x())
                       .arg(from.y())
                       .arg(to.x())
                       .arg(to.y())
                       .arg(static_cast<int>(promotionType));
    sendMessage(message);
}

void NetworkManager::sendGameStart(PieceColor playerColor)
{
    NetworkMessage message;
    message.type = NetworkMessageType::GameStart;
    message.data = QString::number(static_cast<int>(playerColor));
    sendMessage(message);
}

void NetworkManager::sendGameEnd(const QString& result)
{
    NetworkMessage message;
    message.type = NetworkMessageType::GameEnd;
    message.data = result;
    sendMessage(message);
}

void NetworkManager::sendChat(const QString& chatMessage)
{
    NetworkMessage message;
    message.type = NetworkMessageType::Chat;
    message.data = chatMessage;
    sendMessage(message);
}

void NetworkManager::sendResign()
{
    NetworkMessage message;
    message.type = NetworkMessageType::Resign;
    sendMessage(message);
}

void NetworkManager::sendNewGameRequest()
{
    NetworkMessage message;
    message.type = NetworkMessageType::NewGame;
    sendMessage(message);
}

void NetworkManager::sendPlayerInfo(const QString& playerName)
{
    NetworkMessage message;
    message.type = NetworkMessageType::PlayerInfo;
    message.data = playerName;
    sendMessage(message);
}
