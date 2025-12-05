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
    QString getHostAddress() const;
    quint16 getPort() const;
    QString getRoomNumber() const;  // 取得房號（用於僅房號連線）
    bool isRoomNumberOnly() const;  // 檢查是否僅提供房號

private slots:
    void onCreateRoomClicked();
    void onJoinRoomClicked();
    void onPasteConnectionInfo();

private:
    void setupUI();
    void parseConnectionInfo(const QString& info);

    Mode m_mode;
    
    QRadioButton* m_createRoomRadio;
    QRadioButton* m_joinRoomRadio;
    
    QWidget* m_joinRoomWidget;
    QLineEdit* m_hostAddressEdit;
    QLineEdit* m_roomNumberEdit;
    QTextEdit* m_connectionInfoEdit;
    QPushButton* m_pasteButton;
    
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    QLabel* m_instructionLabel;
};

#endif // ONLINEDIALOG_H
