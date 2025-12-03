#ifndef NETWORKGAMEDIALOG_H
#define NETWORKGAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>
#include "chesspiece.h"

class NetworkGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkGameDialog(QWidget *parent = nullptr);
    ~NetworkGameDialog();

    enum class DialogResult {
        Cancelled,
        HostGame,
        JoinGame
    };

    DialogResult getResult() const { return m_result; }
    QString getServerAddress() const;
    quint16 getServerPort() const;
    PieceColor getPlayerColor() const { return m_selectedColor; }
    QString getPlayerName() const;
    QString getRoomNumber() const;

private slots:
    void onHostGameClicked();
    void onJoinGameClicked();
    void onCancelClicked();
    void onHostModeSelected();
    void onJoinModeSelected();

private:
    void setupUI();
    void updateUIState();
    QString generateRoomNumber();  // 生成房間號碼
    void parseRoomNumber(const QString& roomNumber, QString& ip, quint16& port) const;  // 解析房間號碼

    QRadioButton* m_hostRadio;
    QRadioButton* m_joinRadio;
    QButtonGroup* m_modeButtonGroup;

    // 主機模式控制項
    QGroupBox* m_hostGroup;
    QRadioButton* m_whiteRadio;
    QRadioButton* m_blackRadio;
    QButtonGroup* m_colorButtonGroup;
    QLabel* m_colorLabel;
    QLabel* m_roomNumberDisplayLabel;  // 顯示生成的房間號碼

    // 加入模式控制項
    QGroupBox* m_joinGroup;
    QLineEdit* m_roomNumberEdit;  // 房間號碼輸入
    QLabel* m_roomNumberLabel;

    // 玩家名稱
    QLineEdit* m_playerNameEdit;
    QLabel* m_playerNameLabel;

    // 按鈕
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    DialogResult m_result;
    PieceColor m_selectedColor;
    QString m_generatedRoomNumber;  // 保存生成的房間號碼
};

#endif // NETWORKGAMEDIALOG_H
