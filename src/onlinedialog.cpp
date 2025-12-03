#include "onlinedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QIntValidator>

OnlineDialog::OnlineDialog(QWidget *parent)
    : QDialog(parent)
    , m_mode(Mode::None)
{
    setupUI();
    setWindowTitle(tr("線上對戰"));
    resize(400, 300);
}

OnlineDialog::~OnlineDialog()
{
}

void OnlineDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 指示標籤
    m_instructionLabel = new QLabel(tr("選擇連線模式："), this);
    QFont instructionFont = m_instructionLabel->font();
    instructionFont.setPointSize(12);
    m_instructionLabel->setFont(instructionFont);
    mainLayout->addWidget(m_instructionLabel);
    
    mainLayout->addSpacing(10);
    
    // 模式選擇
    QGroupBox* modeGroup = new QGroupBox(tr("連線模式"), this);
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    
    m_createRoomRadio = new QRadioButton(tr("創建房間（作為房主）"), this);
    m_joinRoomRadio = new QRadioButton(tr("加入房間"), this);
    
    modeLayout->addWidget(m_createRoomRadio);
    modeLayout->addWidget(m_joinRoomRadio);
    
    mainLayout->addWidget(modeGroup);
    
    // 加入房間的輸入區域
    m_joinRoomWidget = new QWidget(this);
    QFormLayout* joinLayout = new QFormLayout(m_joinRoomWidget);
    
    m_hostAddressEdit = new QLineEdit(this);
    m_hostAddressEdit->setPlaceholderText(tr("例如: 192.168.1.100"));
    
    m_roomNumberEdit = new QLineEdit(this);
    m_roomNumberEdit->setPlaceholderText(tr("4位數字房號"));
    m_roomNumberEdit->setMaxLength(4);
    QIntValidator* validator = new QIntValidator(1000, 9999, this);
    m_roomNumberEdit->setValidator(validator);
    
    joinLayout->addRow(tr("對方IP地址:"), m_hostAddressEdit);
    joinLayout->addRow(tr("房間號碼:"), m_roomNumberEdit);
    
    m_joinRoomWidget->setEnabled(false);
    mainLayout->addWidget(m_joinRoomWidget);
    
    // 添加說明文字
    QLabel* tipLabel = new QLabel(
        tr("提示：\n"
           "• 創建房間後，系統會生成4位數字房號\n"
           "• 房主需要將自己的IP地址和房號告訴對方\n"
           "• 對方輸入IP地址和房號即可加入房間\n"
           "• 房主執白棋先走，加入者執黑棋"), this);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("QLabel { color: #666; font-size: 10pt; padding: 10px; }");
    mainLayout->addWidget(tipLabel);
    
    mainLayout->addStretch();
    
    // 按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("確定"), this);
    m_cancelButton = new QPushButton(tr("取消"), this);
    
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
    }
}

void OnlineDialog::onJoinRoomClicked()
{
    if (m_joinRoomRadio->isChecked()) {
        m_mode = Mode::JoinRoom;
        m_joinRoomWidget->setEnabled(true);
    }
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
        return 10000 + roomNumber.toInt();
    }
    return 0;
}
