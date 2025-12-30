#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QPoint>
#include "chesspiece.h"

enum class NetworkRole {
    None,
    Host,      // 房主
    Guest      // 加入者
};

enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class MessageType {
    CreateRoom,         // 創建房間
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
    bool createRoom();  // 創建房間，通過服務器生成房號
    bool joinRoom(const QString& roomNumber);  // 通過房號加入房間
    void leaveRoom();  // 明確離開房間（通知伺服器和對手）
    void closeConnection();
    
    QString getRoomNumber() const { return m_roomNumber; }
    NetworkRole getRole() const { return m_role; }
    ConnectionStatus getStatus() const { return m_status; }
    
    // 遊戲同步
    void sendMove(const QPoint& from, const QPoint& to, PieceType promotionType = PieceType::None, QPoint finalPosition = QPoint(-1, -1), PieceType movedPieceType = PieceType::None);
    void sendGameStart(PieceColor playerColor);
    void sendStartGame(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor, const QMap<QString, bool>& gameModes = QMap<QString, bool>(), const std::vector<QPoint>& minePositions = std::vector<QPoint>());  // 房主通知開始遊戲（包含時間設定、顏色選擇、遊戲模式和地雷位置）
    void sendTimeSettings(int whiteTimeMs, int blackTimeMs, int incrementMs);  // 房主發送時間設定更新
    void sendSurrender();  // 發送投降訊息
    void sendDrawOffer();  // 發送和棋請求
    void sendDrawResponse(bool accepted);  // 回應和棋請求（接受或拒絕）
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
    void roomCreated(const QString& roomNumber);
    void opponentJoined();
    void playerLeft();  // 對手在遊戲開始前離開房間
    void promotedToHost();  // 房主離開，自己被提升為新房主
    void opponentMove(const QPoint& from, const QPoint& to, PieceType promotionType, QPoint finalPosition);
    void gameStartReceived(PieceColor playerColor);
    void startGameReceived(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor, qint64 serverTimeOffset, const QMap<QString, bool>& gameModes, const std::vector<QPoint>& minePositions);  // 收到開始遊戲通知（包含時間設定、房主顏色、伺服器時間偏移、遊戲模式和地雷位置）
    void timeSettingsReceived(int whiteTimeMs, int blackTimeMs, int incrementMs);  // 收到時間設定更新
    void timerStateReceived(qint64 timeA, qint64 timeB, const QString& currentPlayer, qint64 lastSwitchTime);  // 收到伺服器計時器狀態更新
    void diceStateReceived(const std::vector<PieceType>& diceRolled, const std::vector<bool>& diceUsed);  // 收到骰子狀態更新
    void surrenderReceived();  // 收到投降訊息
    void drawOfferReceived();  // 收到和棋請求
    void drawResponseReceived(bool accepted);  // 收到和棋回應（接受或拒絕）
    void gameOverReceived(const QString& result);
    void chatReceived(const QString& message);
    void opponentDisconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError socketError);

private:
    QWebSocket* m_webSocket;
    
    NetworkRole m_role;
    ConnectionStatus m_status;
    QString m_roomNumber;
    QString m_serverUrl;
    
    PieceColor m_playerColor;
    PieceColor m_opponentColor;
    
    void sendMessage(const QJsonObject& message);
    std::vector<QPoint> parseMinePositions(const QJsonObject& message) const;  // 解析地雷位置的輔助方法
    std::pair<std::vector<PieceType>, std::vector<bool>> parseDiceState(const QJsonObject& message) const;  // 解析骰子狀態的輔助方法
    void processMessage(const QJsonObject& message);
    MessageType stringToMessageType(const QString& type) const;
    QString messageTypeToString(MessageType type) const;
};

#endif // NETWORKMANAGER_H
