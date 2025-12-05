#ifndef ONLINEDIALOG_H
#define ONLINEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>

class OnlineDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode {
        None,
        CreateRoom,
        JoinRoom
    };

    explicit OnlineDialog(QWidget *parent = nullptr);
    ~OnlineDialog();

    Mode getMode() const { return m_mode; }
    QString getRoomCode() const;  // For relay mode
    bool useRelayServer() const { return true; }  // Always use relay

private slots:
    void onCreateRoomClicked();
    void onJoinRoomClicked();

private:
    void setupUI();

    Mode m_mode;
    
    QRadioButton* m_createRoomRadio;
    QRadioButton* m_joinRoomRadio;
    
    QWidget* m_joinRoomWidget;
    
    // Relay mode widgets
    QLineEdit* m_roomCodeEdit;
    
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    QLabel* m_instructionLabel;
};

#endif // ONLINEDIALOG_H
