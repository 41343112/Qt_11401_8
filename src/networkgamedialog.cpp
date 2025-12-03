#include "networkgamedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>

NetworkGameDialog::NetworkGameDialog(QWidget *parent)
    : QDialog(parent)
    , m_result(DialogResult::Cancelled)
    , m_selectedColor(PieceColor::White)
{
    setupUI();
    setWindowTitle(tr("網路對戰"));
    resize(450, 350);
}

NetworkGameDialog::~NetworkGameDialog()
{
}

void NetworkGameDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 玩家名稱輸入
    QHBoxLayout* nameLayout = new QHBoxLayout();
    m_playerNameLabel = new QLabel(tr("玩家名稱:"), this);
    m_playerNameEdit = new QLineEdit(this);
    m_playerNameEdit->setPlaceholderText(tr("輸入您的名稱"));
    m_playerNameEdit->setText(tr("玩家"));
    nameLayout->addWidget(m_playerNameLabel);
    nameLayout->addWidget(m_playerNameEdit);
    mainLayout->addLayout(nameLayout);
    
    mainLayout->addSpacing(10);
    
    // 模式選擇
    QGroupBox* modeGroup = new QGroupBox(tr("連線模式"), this);
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    
    m_modeButtonGroup = new QButtonGroup(this);
    m_hostRadio = new QRadioButton(tr("主機模式（等待其他玩家連線）"), this);
    m_joinRadio = new QRadioButton(tr("加入模式（連接到其他玩家）"), this);
    m_hostRadio->setChecked(true);
    
    m_modeButtonGroup->addButton(m_hostRadio, 0);
    m_modeButtonGroup->addButton(m_joinRadio, 1);
    
    modeLayout->addWidget(m_hostRadio);
    modeLayout->addWidget(m_joinRadio);
    
    mainLayout->addWidget(modeGroup);
    
    // 主機模式設定
    m_hostGroup = new QGroupBox(tr("主機設定"), this);
    QFormLayout* hostLayout = new QFormLayout(m_hostGroup);
    
    m_colorLabel = new QLabel(tr("執棋顏色:"), this);
    QHBoxLayout* colorLayout = new QHBoxLayout();
    m_colorButtonGroup = new QButtonGroup(this);
    m_whiteRadio = new QRadioButton(tr("執白"), this);
    m_blackRadio = new QRadioButton(tr("執黑"), this);
    m_whiteRadio->setChecked(true);
    
    m_colorButtonGroup->addButton(m_whiteRadio, static_cast<int>(PieceColor::White));
    m_colorButtonGroup->addButton(m_blackRadio, static_cast<int>(PieceColor::Black));
    
    colorLayout->addWidget(m_whiteRadio);
    colorLayout->addWidget(m_blackRadio);
    hostLayout->addRow(m_colorLabel, colorLayout);
    
    // 生成並顯示房間號碼
    m_generatedRoomNumber = generateRoomNumber();
    QLabel* roomLabel = new QLabel(tr("房間號碼:"), this);
    
    // 創建一個水平布局來放置房間號碼和複製按鈕
    QHBoxLayout* roomLayout = new QHBoxLayout();
    m_roomNumberDisplayLabel = new QLabel(m_generatedRoomNumber, this);
    m_roomNumberDisplayLabel->setStyleSheet("QLabel { font-size: 16pt; font-weight: bold; color: #00D9FF; }");
    m_roomNumberDisplayLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    QPushButton* copyButton = new QPushButton(tr("複製"), this);
    copyButton->setMaximumWidth(60);
    connect(copyButton, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_roomNumberDisplayLabel->text());
        QMessageBox::information(this, tr("已複製"), tr("房間號碼已複製到剪貼簿！"));
    });
    
    roomLayout->addWidget(m_roomNumberDisplayLabel);
    roomLayout->addWidget(copyButton);
    roomLayout->addStretch();
    
    hostLayout->addRow(roomLabel, roomLayout);
    
    // 顯示本機 IP 位址（供參考）
    QLabel* ipLabel = new QLabel(tr("本機 IP:"), this);
    QString localIp = tr("獲取中...");
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : ipAddressesList) {
        if (address != QHostAddress::LocalHost && address.toIPv4Address()) {
            localIp = address.toString();
            break;
        }
    }
    QLabel* ipValueLabel = new QLabel(localIp, this);
    ipValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ipValueLabel->setStyleSheet("QLabel { font-size: 9pt; color: #888; }");
    hostLayout->addRow(ipLabel, ipValueLabel);
    
    mainLayout->addWidget(m_hostGroup);
    
    // 加入模式設定
    m_joinGroup = new QGroupBox(tr("加入設定"), this);
    QFormLayout* joinLayout = new QFormLayout(m_joinGroup);
    
    m_roomNumberLabel = new QLabel(tr("房間號碼:"), this);
    m_roomNumberEdit = new QLineEdit(this);
    m_roomNumberEdit->setPlaceholderText(tr("輸入房間號碼，例如: ABCD1234"));
    m_roomNumberEdit->setStyleSheet("QLineEdit { font-size: 14pt; }");
    joinLayout->addRow(m_roomNumberLabel, m_roomNumberEdit);
    
    mainLayout->addWidget(m_joinGroup);
    
    // 按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("開始"), this);
    m_cancelButton = new QPushButton(tr("取消"), this);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 連接信號
    connect(m_hostRadio, &QRadioButton::toggled, this, &NetworkGameDialog::onHostModeSelected);
    connect(m_joinRadio, &QRadioButton::toggled, this, &NetworkGameDialog::onJoinModeSelected);
    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        if (m_hostRadio->isChecked()) {
            onHostGameClicked();
        } else {
            onJoinGameClicked();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, &NetworkGameDialog::onCancelClicked);
    
    // 初始化 UI 狀態
    updateUIState();
}

void NetworkGameDialog::updateUIState()
{
    bool isHostMode = m_hostRadio->isChecked();
    m_hostGroup->setEnabled(isHostMode);
    m_joinGroup->setEnabled(!isHostMode);
}

void NetworkGameDialog::onHostModeSelected()
{
    updateUIState();
}

void NetworkGameDialog::onJoinModeSelected()
{
    updateUIState();
}

void NetworkGameDialog::onHostGameClicked()
{
    // 驗證玩家名稱
    if (m_playerNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("請輸入玩家名稱！"));
        return;
    }
    
    // 設定選擇的顏色
    if (m_whiteRadio->isChecked()) {
        m_selectedColor = PieceColor::White;
    } else {
        m_selectedColor = PieceColor::Black;
    }
    
    m_result = DialogResult::HostGame;
    accept();
}

void NetworkGameDialog::onJoinGameClicked()
{
    // 驗證玩家名稱
    if (m_playerNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("請輸入玩家名稱！"));
        return;
    }
    
    // 驗證房間號碼
    if (m_roomNumberEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("請輸入房間號碼！"));
        return;
    }
    
    // 驗證房間號碼格式
    QString roomNumber = m_roomNumberEdit->text().trimmed().toUpper();
    if (roomNumber.length() != 8) {
        QMessageBox::warning(this, tr("警告"), tr("房間號碼格式不正確！應為8個字符。"));
        return;
    }
    
    m_result = DialogResult::JoinGame;
    accept();
}

void NetworkGameDialog::onCancelClicked()
{
    m_result = DialogResult::Cancelled;
    reject();
}

QString NetworkGameDialog::getServerAddress() const
{
    if (m_hostRadio->isChecked()) {
        // 主機模式：返回本機 IP
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (const QHostAddress& address : ipAddressesList) {
            if (address != QHostAddress::LocalHost && address.toIPv4Address()) {
                return address.toString();
            }
        }
        return "127.0.0.1";
    } else {
        // 加入模式：從房間號碼解析 IP
        QString ip;
        quint16 port;
        parseRoomNumber(m_roomNumberEdit->text().trimmed().toUpper(), ip, port);
        return ip;
    }
}

quint16 NetworkGameDialog::getServerPort() const
{
    if (m_hostRadio->isChecked()) {
        // 主機模式：從房間號碼中提取端口
        QString ip;
        quint16 port;
        parseRoomNumber(m_generatedRoomNumber, ip, port);
        return port;
    } else {
        // 加入模式：從輸入的房間號碼解析端口
        QString ip;
        quint16 port;
        parseRoomNumber(m_roomNumberEdit->text().trimmed().toUpper(), ip, port);
        return port;
    }
}

QString NetworkGameDialog::getPlayerName() const
{
    return m_playerNameEdit->text().trimmed();
}

QString NetworkGameDialog::getRoomNumber() const
{
    return m_generatedRoomNumber;
}

QString NetworkGameDialog::generateRoomNumber()
{
    // 生成一個簡單的房間號碼
    // 格式：4個字母 + 4個數字（例如：ABCD1234）
    QString roomNumber;
    
    // 生成4個隨機字母（A-Z）
    for (int i = 0; i < 4; ++i) {
        roomNumber += QChar('A' + QRandomGenerator::global()->bounded(26));
    }
    
    // 生成4個隨機數字（0-9）
    for (int i = 0; i < 4; ++i) {
        roomNumber += QString::number(QRandomGenerator::global()->bounded(10));
    }
    
    return roomNumber;
}

void NetworkGameDialog::parseRoomNumber(const QString& roomNumber, QString& ip, quint16& port) const
{
    // 從房間號碼解析出 IP 和端口
    // 這是一個簡化的實現，實際應用中應該使用服務器來管理房間
    
    // 對於本地測試，我們將房間號碼映射到端口
    // 使用房間號碼的哈希值來生成端口號（8000-9000範圍）
    uint hash = qHash(roomNumber);
    port = 8000 + (hash % 1000);
    
    // 獲取本機 IP 作為服務器地址
    ip = "127.0.0.1";  // 默認為本地
    
    // 嘗試獲取局域網 IP
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : ipAddressesList) {
        if (address != QHostAddress::LocalHost && address.toIPv4Address()) {
            QString ipStr = address.toString();
            // 如果是局域網 IP（192.168.x.x 或 10.x.x.x）
            if (ipStr.startsWith("192.168.") || ipStr.startsWith("10.")) {
                ip = ipStr;
                break;
            }
        }
    }
}
