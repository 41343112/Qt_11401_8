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
    , m_roomCodeEdit(nullptr)
{
    setupUI();
    setWindowTitle(tr("線上對戰 - 簡易連線"));
    resize(500, 400);
}

OnlineDialog::~OnlineDialog()
{
}

void OnlineDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 指示標籤 - 更友善的說明
    m_instructionLabel = new QLabel(tr("🎮 歡迎使用線上對戰！\n只需要房號就能跨網域連線！"), this);
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
    
    m_createRoomRadio = new QRadioButton(tr("📱 我要創建房間（取得房號給朋友）"), this);
    m_createRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    m_joinRoomRadio = new QRadioButton(tr("🔗 我有房號（加入朋友的房間）"), this);
    m_joinRoomRadio->setStyleSheet("QRadioButton { font-size: 11pt; padding: 5px; }");
    
    modeLayout->addWidget(m_createRoomRadio);
    modeLayout->addWidget(m_joinRoomRadio);
    
    mainLayout->addWidget(modeGroup);
    
    mainLayout->addSpacing(10);
    
    // 加入房間的輸入區域（只有中繼伺服器模式）
    m_joinRoomWidget = new QWidget(this);
    QVBoxLayout* joinMainLayout = new QVBoxLayout(m_joinRoomWidget);
    
    // 中繼伺服器模式（簡單版 - 只需房號）
    QGroupBox* relayConnectGroup = new QGroupBox(tr("📋 輸入房號"), this);
    relayConnectGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #4CAF50; }");
    QVBoxLayout* relayLayout = new QVBoxLayout(relayConnectGroup);
    
    QLabel* relayLabel = new QLabel(tr("只需要輸入4位數字的房號："), this);
    relayLabel->setWordWrap(true);
    relayLayout->addWidget(relayLabel);
    
    m_roomCodeEdit = new QLineEdit(this);
    m_roomCodeEdit->setPlaceholderText(tr("例如：1234"));
    m_roomCodeEdit->setMaxLength(4);
    QIntValidator* roomValidator = new QIntValidator(1000, 9999, this);
    m_roomCodeEdit->setValidator(roomValidator);
    QFont roomCodeFont = m_roomCodeEdit->font();
    roomCodeFont.setPointSize(14);
    m_roomCodeEdit->setFont(roomCodeFont);
    relayLayout->addWidget(m_roomCodeEdit);
    
    joinMainLayout->addWidget(relayConnectGroup);
    
    m_joinRoomWidget->setEnabled(false);
    mainLayout->addWidget(m_joinRoomWidget);
    
    // 添加簡化的說明文字
    QLabel* tipLabel = new QLabel(
        tr("💡 <b>簡單說明：</b><br>"
           "• <b>創建房間</b>：系統會給您一個4位數字房號，傳給朋友即可<br>"
           "• <b>加入房間</b>：輸入朋友給的4位數字房號即可連線<br>"
           "• 房主（創建者）執白棋先走，加入者執黑棋<br>"
           "• 使用中繼伺服器，可跨網域連線（不同WiFi也可以）"), this);
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
        m_instructionLabel->setText(tr("🎮 您選擇了「創建房間」\n點擊「開始」後，系統會給您一個4位數字房號"));
    }
}

void OnlineDialog::onJoinRoomClicked()
{
    if (m_joinRoomRadio->isChecked()) {
        m_mode = Mode::JoinRoom;
        m_joinRoomWidget->setEnabled(true);
        m_instructionLabel->setText(tr("🎮 您選擇了「加入房間」\n請輸入朋友給您的4位數字房號"));
    }
}

QString OnlineDialog::getRoomCode() const
{
    return m_roomCodeEdit ? m_roomCodeEdit->text().trimmed() : QString();
}
