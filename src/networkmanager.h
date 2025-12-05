#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QPoint>
#include <QTimer>
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
    StartGame,          // 房主通知開始遊戲
    TimeSettings,       // 房主更新時間設定
    Move,               // 棋步
    GameOver,           // 遊戲結束
    Surrender,          // 投降
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
    bool discoverAndJoinRoom(const QString& roomNumber);  // 使用房號探索並加入房間（本地網路）
    void closeConnection();
    
    QString getRoomNumber() const { return m_roomNumber; }
    quint16 getPort() const { return m_port; }
    NetworkRole getRole() const { return m_role; }
    ConnectionStatus getStatus() const { return m_status; }
    
    // 遊戲同步
    void sendMove(const QPoint& from, const QPoint& to, PieceType promotionType = PieceType::None);
    void sendGameStart(PieceColor playerColor);
    void sendStartGame(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor);  // 房主通知開始遊戲（包含時間設定和顏色選擇）
    void sendTimeSettings(int whiteTimeMs, int blackTimeMs, int incrementMs);  // 房主發送時間設定更新
    void sendSurrender();  // 發送投降訊息
    void sendGameOver(const QString& result);
    void sendChat(const QString& message);
    
    // 玩家顏色管理
    PieceColor getOpponentColor() const { return m_opponentColor; }
    PieceColor getPlayerColor() const { return m_playerColor; }
    void setPlayerColors(PieceColor playerColor);  // 設定玩家顏色（同時設定對手顏色為相反顏色）


signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void roomCreated(const QString& roomNumber, quint16 port);
    void roomDiscovered(const QString& hostAddress, quint16 port);  // 探索到房間
    void opponentJoined();
    void opponentMove(const QPoint& from, const QPoint& to, PieceType promotionType);
    void gameStartReceived(PieceColor playerColor);
    void startGameReceived(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor);  // 收到開始遊戲通知（包含時間設定和房主顏色）
    void timeSettingsReceived(int whiteTimeMs, int blackTimeMs, int incrementMs);  // 收到時間設定更新
    void surrenderReceived();  // 收到投降訊息
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
    void onDiscoveryReadyRead();  // UDP 探索訊息接收
    void onDiscoveryTimeout();     // UDP 探索超時

private:
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    QTcpSocket* m_clientSocket;  // 用於服務器端連接的客戶端套接字
    
    // UDP 探索
    QUdpSocket* m_discoverySocket;
    QTimer* m_discoveryTimer;
    QString m_searchingRoomNumber;
    
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
    
    // UDP 探索相關
    void startDiscovery();
    void stopDiscovery();
    void sendDiscoveryRequest(const QString& roomNumber);
    void sendDiscoveryResponse(const QHostAddress& sender, quint16 senderPort);
};

#endif // NETWORKMANAGER_H
