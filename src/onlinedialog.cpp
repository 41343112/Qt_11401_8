#include "onlinedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QClipboard>
#include <QApplication>

OnlineDialog::OnlineDialog(QWidget *parent)
    : QDialog(parent)
    , m_mode(Mode::None)
    , m_connectionInfoEdit(nullptr)
    , m_pasteButton(nullptr)
{
    setupUI();
    setWindowTitle(tr("線上對戰 - 簡易連線"));
    resize(500, 420);
}

OnlineDialog::~OnlineDialog()
{
}

void OnlineDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 指示標籤 - 更友善的說明
    m_instructionLabel = new QLabel(tr("🎮 歡迎使用線上對戰！\n請選擇您的角色："), this);
    QFont instructionFont = m_instructionLabel->font();
    instructionFont.setPointSize(12);
    instructionFont.setBold(true);
    m_instructionLabel->setFont(instructionFont);
    m_instructionLabel->setStyleSheet("QLabel { color: #2196F3; padding: 10px; }");
    mainLayout->addWidget(m_instructionLabel);
    
    mainLayout->addSpacing(10);
    
    // 模式選擇
    QGroupBox* modeGroup = new QGroupBox(tr("選擇角色"), this);
    modeGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    
    m_createRoomRadio = new QRadioButton(tr("📱 我要創建房間（給朋友連線碼）"), this);
    m_createRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    m_joinRoomRadio = new QRadioButton(tr("🔗 我有連線碼（加入朋友的房間）"), this);
    m_joinRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    
    modeLayout->addWidget(m_createRoomRadio);
    modeLayout->addWidget(m_joinRoomRadio);
    
    mainLayout->addWidget(modeGroup);
    
    mainLayout->addSpacing(10);
    
    // 加入房間的輸入區域 - 簡化版
    m_joinRoomWidget = new QWidget(this);
    QVBoxLayout* joinMainLayout = new QVBoxLayout(m_joinRoomWidget);
    
    // 簡易連線區域（使用連線碼）
    QGroupBox* easyConnectGroup = new QGroupBox(tr("📋 方法一：使用連線碼（推薦）"), this);
    easyConnectGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #4CAF50; }");
    QVBoxLayout* easyLayout = new QVBoxLayout(easyConnectGroup);
    
    QLabel* easyLabel = new QLabel(tr("請朋友將連線碼複製後，貼到下方："), this);
    easyLabel->setWordWrap(true);
    easyLayout->addWidget(easyLabel);
    
    m_connectionInfoEdit = new QTextEdit(this);
    m_connectionInfoEdit->setPlaceholderText(tr("在此貼上朋友給您的連線碼\n格式如：192.168.1.100:1234 或 example.com:1234"));
    m_connectionInfoEdit->setMaximumHeight(60);
    easyLayout->addWidget(m_connectionInfoEdit);
    
    m_pasteButton = new QPushButton(tr("📋 從剪貼簿貼上"), this);
    m_pasteButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; font-weight: bold; }");
    connect(m_pasteButton, &QPushButton::clicked, this, &OnlineDialog::onPasteConnectionInfo);
    easyLayout->addWidget(m_pasteButton);
    
    joinMainLayout->addWidget(easyConnectGroup);
    
    // 手動輸入區域
    QGroupBox* manualGroup = new QGroupBox(tr("✏️ 方法二：手動輸入"), this);
    manualGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout* manualLayout = new QFormLayout(manualGroup);
    
    m_hostAddressEdit = new QLineEdit(this);
    m_hostAddressEdit->setPlaceholderText(tr("例如: 192.168.1.100 或 example.com"));
    
    m_roomNumberEdit = new QLineEdit(this);
    m_roomNumberEdit->setPlaceholderText(tr("4位數字（可單獨使用於本地網路）"));
    m_roomNumberEdit->setMaxLength(4);
    QIntValidator* validator = new QIntValidator(1000, 9999, this);
    m_roomNumberEdit->setValidator(validator);
    
    manualLayout->addRow(tr("IP地址/域名:"), m_hostAddressEdit);
    manualLayout->addRow(tr("房號:"), m_roomNumberEdit);
    
    joinMainLayout->addWidget(manualGroup);
    
    m_joinRoomWidget->setEnabled(false);
    mainLayout->addWidget(m_joinRoomWidget);
    
    // 添加簡化的說明文字
    QLabel* tipLabel = new QLabel(
        tr("💡 <b>簡單說明：</b><br>"
           "• <b>創建房間</b>：系統會給您一個連線碼，複製後傳給朋友<br>"
           "• <b>加入房間</b>：把朋友給的連線碼貼上即可<br>"
           "• <b>快速加入</b>：如在同一WiFi，直接輸入4位數房號即可<br>"
           "• 房主（創建者）執白棋先走，加入者執黑棋"), this);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("QLabel { color: #666; font-size: 10pt; padding: 10px; background-color: #f5f5f5; border-radius: 5px; }");
    mainLayout->addWidget(tipLabel);
    
    mainLayout->addStretch();
    
    // 按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("✓ 開始"), this);
    m_okButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 10px 20px; font-weight: bold; }");
    m_cancelButton = new QPushButton(tr("✗ 取消"), this);
    m_cancelButton->setStyleSheet("QPushButton { padding: 10px 20px; }");
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 連接信號
    connect(m_createRoomRadio, &QRadioButton::toggled, this, &OnlineDialog::onCreateRoomClicked);
    connect(m_joinRoomRadio, &QRadioButton::toggled, this, &OnlineDialog::onJoinRoomClicked);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 默認選中創建房間
    m_createRoomRadio->setChecked(true);
}

void OnlineDialog::onCreateRoomClicked()
{
    if (m_createRoomRadio->isChecked()) {
        m_mode = Mode::CreateRoom;
        m_joinRoomWidget->setEnabled(false);
        m_instructionLabel->setText(tr("🎮 您選擇了「創建房間」\n點擊「開始」後，系統會給您一個連線碼"));
    }
}

void OnlineDialog::onJoinRoomClicked()
{
    if (m_joinRoomRadio->isChecked()) {
        m_mode = Mode::JoinRoom;
        m_joinRoomWidget->setEnabled(true);
        m_instructionLabel->setText(tr("🎮 您選擇了「加入房間」\n請貼上朋友給您的連線碼，或手動輸入"));
    }
}

void OnlineDialog::onPasteConnectionInfo()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString clipText = clipboard->text().trimmed();
    
    if (!clipText.isEmpty()) {
        m_connectionInfoEdit->setPlainText(clipText);
        parseConnectionInfo(clipText);
    } else {
        QMessageBox::information(this, tr("提示"), tr("剪貼簿是空的，請先複製連線碼"));
    }
}

void OnlineDialog::parseConnectionInfo(const QString& info)
{
    // 支援多種格式：
    // 1. IP:房號 格式 (192.168.1.100:1234)
    // 2. IP 房號 格式 (192.168.1.100 1234)
    // 3. 連線碼：IP:房號 格式
    // 4. 僅房號格式 (1234) - 用於本地網路探索
    
    QString text = info.trimmed();
    
    // 檢查是否僅為4位數字（房號）
    if (text.length() == 4) {
        bool ok;
        int roomNum = text.toInt(&ok);
        if (ok && roomNum >= 1000 && roomNum <= 9999) {
            m_hostAddressEdit->clear();  // 清空IP，表示使用探索模式
            m_roomNumberEdit->setText(text);
            QMessageBox::information(this, tr("房號模式"), 
                tr("已填入房號：%1\n\n系統將在本地網路中搜尋此房間").arg(text));
            return;
        }
    }
    
    // 移除可能的前綴文字
    if (text.contains("連線碼") || text.contains("Connection")) {
        int colonPos = text.lastIndexOf(':');
        if (colonPos > 0) {
            text = text.mid(colonPos + 1).trimmed();
        }
    }
    
    // 嘗試解析 IP:房號 格式
    QStringList parts;
    if (text.contains(':')) {
        parts = text.split(':');
    } else if (text.contains(' ')) {
        parts = text.split(' ', Qt::SkipEmptyParts);
    }
    
    if (parts.size() >= 2) {
        QString ip = parts[0].trimmed();
        QString room = parts[parts.size() - 1].trimmed();
        
        // 驗證房號是4位數字
        if (room.length() == 4) {
            bool ok;
            int roomNum = room.toInt(&ok);
            if (ok && roomNum >= 1000 && roomNum <= 9999) {
                m_hostAddressEdit->setText(ip);
                m_roomNumberEdit->setText(room);
                QMessageBox::information(this, tr("成功"), 
                    tr("已自動填入：\nIP: %1\n房號: %2").arg(ip, room));
                return;
            }
        }
    }
    
    QMessageBox::warning(this, tr("格式錯誤"), 
        tr("無法識別連線碼格式\n\n正確格式範例：\n• 192.168.1.100:1234\n• example.com:1234\n• 192.168.1.100 1234\n• 1234（僅房號，本地網路）"));
}

QString OnlineDialog::getHostAddress() const
{
    return m_hostAddressEdit->text().trimmed();
}

quint16 OnlineDialog::getPort() const
{
    // 從房號計算端口（10000 + 房號）
    QString roomNumber = m_roomNumberEdit->text().trimmed();
    if (roomNumber.length() == 4) {
        bool ok;
        int roomNum = roomNumber.toInt(&ok);
        if (ok && roomNum >= 1000 && roomNum <= 9999) {
            return 10000 + roomNum;
        }
    }
    return 0;
}

QString OnlineDialog::getRoomNumber() const
{
    return m_roomNumberEdit->text().trimmed();
}

bool OnlineDialog::isRoomNumberOnly() const
{
    // 檢查是否只提供了房號，沒有提供IP地址
    QString hostAddress = m_hostAddressEdit->text().trimmed();
    QString roomNumber = m_roomNumberEdit->text().trimmed();
    
    return hostAddress.isEmpty() && !roomNumber.isEmpty() && roomNumber.length() == 4;
}
