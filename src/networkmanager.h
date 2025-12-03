#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QPoint>
#include "chesspiece.h"

enum class NetworkRole {
    None,
    Server,    // 房主
    Client     // 加入者
};

enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class MessageType {
    RoomCreated,        // 房間已創建
    JoinRoom,           // 加入房間
    JoinAccepted,       // 加入被接受
    JoinRejected,       // 加入被拒絕
    GameStart,          // 遊戲開始
    Move,               // 棋步
    GameOver,           // 遊戲結束
    Chat,               // 聊天訊息
    PlayerDisconnected, // 玩家斷線
    Ping,               // 心跳包
    Pong                // 心跳回應
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 房間管理
    bool createRoom(quint16 port = 0);  // 創建房間，返回房號
    bool joinRoom(const QString& hostAddress, quint16 port);
    void closeConnection();
    
    QString getRoomNumber() const { return m_roomNumber; }
    quint16 getPort() const { return m_port; }
    NetworkRole getRole() const { return m_role; }
    ConnectionStatus getStatus() const { return m_status; }
    
    // 遊戲同步
    void sendMove(const QPoint& from, const QPoint& to, PieceType promotionType = PieceType::None);
    void sendGameStart(PieceColor playerColor);
    void sendGameOver(const QString& result);
    void sendChat(const QString& message);
    
    // 對手顏色
    PieceColor getOpponentColor() const { return m_opponentColor; }
    PieceColor getPlayerColor() const { return m_playerColor; }

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void roomCreated(const QString& roomNumber, quint16 port);
    void opponentJoined();
    void opponentMove(const QPoint& from, const QPoint& to, PieceType promotionType);
    void gameStartReceived(PieceColor playerColor);
    void gameOverReceived(const QString& result);
    void chatReceived(const QString& message);
    void opponentDisconnected();

private slots:
    void onNewConnection();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void onServerError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    QTcpSocket* m_clientSocket;  // 用於服務器端連接的客戶端套接字
    
    NetworkRole m_role;
    ConnectionStatus m_status;
    QString m_roomNumber;
    quint16 m_port;
    
    PieceColor m_playerColor;
    PieceColor m_opponentColor;
    
    QByteArray m_receiveBuffer;
    
    void sendMessage(const QJsonObject& message);
    void processMessage(const QJsonObject& message);
    QString generateRoomNumber() const;
    MessageType stringToMessageType(const QString& type) const;
    QString messageTypeToString(MessageType type) const;
    QTcpSocket* getActiveSocket() const;
};

#endif // NETWORKMANAGER_H
