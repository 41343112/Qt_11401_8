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

private slots:
    void onHostGameClicked();
    void onJoinGameClicked();
    void onCancelClicked();
    void onHostModeSelected();
    void onJoinModeSelected();

private:
    void setupUI();
    void updateUIState();

    QRadioButton* m_hostRadio;
    QRadioButton* m_joinRadio;
    QButtonGroup* m_modeButtonGroup;

    // 主機模式控制項
    QGroupBox* m_hostGroup;
    QSpinBox* m_portSpinBox;
    QRadioButton* m_whiteRadio;
    QRadioButton* m_blackRadio;
    QButtonGroup* m_colorButtonGroup;
    QLabel* m_portLabel;
    QLabel* m_colorLabel;

    // 加入模式控制項
    QGroupBox* m_joinGroup;
    QLineEdit* m_addressEdit;
    QSpinBox* m_joinPortSpinBox;
    QLabel* m_addressLabel;
    QLabel* m_joinPortLabel;

    // 玩家名稱
    QLineEdit* m_playerNameEdit;
    QLabel* m_playerNameLabel;

    // 按鈕
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    DialogResult m_result;
    PieceColor m_selectedColor;
};

#endif // NETWORKGAMEDIALOG_H
