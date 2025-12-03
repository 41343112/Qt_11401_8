#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QPoint>
#include "chesspiece.h"

enum class NetworkRole {
    None,
    Server,  // 主機
    Client   // 客戶端
};

enum class NetworkMessageType {
    Move,           // 棋步移動
    GameStart,      // 遊戲開始
    GameEnd,        // 遊戲結束
    Chat,           // 聊天訊息
    Resign,         // 認輸
    NewGame,        // 新遊戲請求
    Handshake,      // 握手確認連線
    Ping,           // 心跳檢測
    PlayerInfo      // 玩家資訊
};

struct NetworkMessage {
    NetworkMessageType type;
    QString data;
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 伺服器功能
    bool startServer(quint16 port = 8888);
    void stopServer();
    bool isServerRunning() const { return m_server != nullptr && m_server->isListening(); }
    quint16 getServerPort() const;

    // 客戶端功能
    bool connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const { return m_socket != nullptr && m_socket->state() == QTcpSocket::ConnectedState; }

    // 訊息發送
    void sendMove(const QPoint& from, const QPoint& to, PieceType promotionType = PieceType::None);
    void sendGameStart(PieceColor playerColor);
    void sendGameEnd(const QString& result);
    void sendChat(const QString& message);
    void sendResign();
    void sendNewGameRequest();
    void sendPlayerInfo(const QString& playerName);

    // 狀態查詢
    NetworkRole getRole() const { return m_role; }
    QString getRemoteAddress() const;
    PieceColor getLocalPlayerColor() const { return m_localPlayerColor; }
    void setLocalPlayerColor(PieceColor color) { m_localPlayerColor = color; }

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void moveReceived(const QPoint& from, const QPoint& to, PieceType promotionType);
    void gameStartReceived(PieceColor remotePlayerColor);
    void gameEndReceived(const QString& result);
    void chatReceived(const QString& message);
    void resignReceived();
    void newGameRequested();
    void playerInfoReceived(const QString& playerName);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    NetworkRole m_role;
    PieceColor m_localPlayerColor;
    QString m_readBuffer;

    void sendMessage(const NetworkMessage& message);
    void processMessage(const QString& messageData);
    QString encodeMessage(const NetworkMessage& message) const;
    NetworkMessage decodeMessage(const QString& data) const;
};

#endif // NETWORKMANAGER_H
