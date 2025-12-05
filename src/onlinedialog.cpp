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
    m_instructionLabel = new QLabel(tr("🎮 歡迎使用線上對戰！\n使用中央伺服器連線，無需設定網路"), this);
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
    
    m_createRoomRadio = new QRadioButton(tr("📱 我要創建房間（分享房號給朋友）"), this);
    m_createRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    m_joinRoomRadio = new QRadioButton(tr("🔗 我有房號（加入朋友的房間）"), this);
    m_joinRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    
    modeLayout->addWidget(m_createRoomRadio);
    modeLayout->addWidget(m_joinRoomRadio);
    
    mainLayout->addWidget(modeGroup);
    
    mainLayout->addSpacing(10);
    
    // 加入房間的輸入區域 - 簡化版
    m_joinRoomWidget = new QWidget(this);
    QVBoxLayout* joinMainLayout = new QVBoxLayout(m_joinRoomWidget);
    
    // 簡易連線區域（使用房號）
    QGroupBox* easyConnectGroup = new QGroupBox(tr("📋 方法一：使用房號（推薦）"), this);
    easyConnectGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #4CAF50; }");
    QVBoxLayout* easyLayout = new QVBoxLayout(easyConnectGroup);
    
    QLabel* easyLabel = new QLabel(tr("請朋友將房號複製後，貼到下方："), this);
    easyLabel->setWordWrap(true);
    easyLayout->addWidget(easyLabel);
    
    m_connectionInfoEdit = new QTextEdit(this);
    m_connectionInfoEdit->setPlaceholderText(tr("在此貼上朋友給您的房號\n格式如：1234"));
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
    
    m_roomNumberEdit = new QLineEdit(this);
    m_roomNumberEdit->setPlaceholderText(tr("例如: 1234"));
    m_roomNumberEdit->setMaxLength(4);
    QIntValidator* validator = new QIntValidator(1000, 9999, this);
    m_roomNumberEdit->setValidator(validator);
    
    manualLayout->addRow(tr("房號:"), m_roomNumberEdit);
    
    joinMainLayout->addWidget(manualGroup);
    
    m_joinRoomWidget->setEnabled(false);
    mainLayout->addWidget(m_joinRoomWidget);
    
    // 添加簡化的說明文字
    QLabel* tipLabel = new QLabel(
        tr("💡 <b>簡單說明：</b><br>"
           "• <b>創建房間</b>：系統會給您一個房號，複製後傳給朋友<br>"
           "• <b>加入房間</b>：把朋友給的房號貼上即可<br>"
           "• 房主（創建者）執白棋先走，加入者執黑棋<br>"
           "• 使用中央伺服器，無需設定路由器或防火牆"), this);
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
        m_instructionLabel->setText(tr("🎮 您選擇了「創建房間」\n點擊「開始」後，系統會給您一個房號"));
    }
}

void OnlineDialog::onJoinRoomClicked()
{
    if (m_joinRoomRadio->isChecked()) {
        m_mode = Mode::JoinRoom;
        m_joinRoomWidget->setEnabled(true);
        m_instructionLabel->setText(tr("🎮 您選擇了「加入房間」\n請貼上朋友給您的房號，或手動輸入"));
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
        QMessageBox::information(this, tr("提示"), tr("剪貼簿是空的，請先複製房號"));
    }
}

void OnlineDialog::parseConnectionInfo(const QString& info)
{
    QString text = info.trimmed();
    
    // 移除可能的前綴文字
    if (text.contains("房號") || text.contains("Room")) {
        int colonPos = text.lastIndexOf(':');
        if (colonPos > 0) {
            text = text.mid(colonPos + 1).trimmed();
        } else {
            // 如果沒有冒號，嘗試提取最後一個單詞
            QStringList parts = text.split(' ', Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                text = parts.last();
            }
        }
    }
    
    // 清理可能的空白字符
    text = text.simplified();
    
    // 驗證房號格式（應該是4位數字）
    if (text.length() == 4) {
        bool ok;
        int roomNum = text.toInt(&ok);
        if (ok && roomNum >= 1000 && roomNum <= 9999) {
            m_roomNumberEdit->setText(text);
            QMessageBox::information(this, tr("成功"), 
                tr("已自動填入房號: %1").arg(text));
            return;
        }
    }
    
    QMessageBox::warning(this, tr("格式錯誤"), 
        tr("無法識別房號格式\n\n房號必須是4位數字（1000-9999）"));
}

QString OnlineDialog::getRoomNumber() const
{
    // 優先從文字框取得
    QString roomNumber = m_roomNumberEdit->text().trimmed();
    
    // 如果文字框為空，嘗試從貼上區域取得
    if (roomNumber.isEmpty()) {
        roomNumber = m_connectionInfoEdit->toPlainText().trimmed();
    }
    
    return roomNumber;
}
