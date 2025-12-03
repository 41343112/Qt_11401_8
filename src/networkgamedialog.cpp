#include "networkgamedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHostInfo>
#include <QNetworkInterface>

NetworkGameDialog::NetworkGameDialog(QWidget *parent)
    : QDialog(parent)
    , m_result(DialogResult::Cancelled)
    , m_selectedColor(PieceColor::White)
{
    setupUI();
    setWindowTitle(tr("網路對戰"));
    resize(450, 400);
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
    
    m_portLabel = new QLabel(tr("端口:"), this);
    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(8888);
    hostLayout->addRow(m_portLabel, m_portSpinBox);
    
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
    
    // 顯示本機 IP 位址
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
    hostLayout->addRow(ipLabel, ipValueLabel);
    
    mainLayout->addWidget(m_hostGroup);
    
    // 加入模式設定
    m_joinGroup = new QGroupBox(tr("加入設定"), this);
    QFormLayout* joinLayout = new QFormLayout(m_joinGroup);
    
    m_addressLabel = new QLabel(tr("伺服器位址:"), this);
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setPlaceholderText(tr("輸入 IP 位址，例如: 192.168.1.100"));
    joinLayout->addRow(m_addressLabel, m_addressEdit);
    
    m_joinPortLabel = new QLabel(tr("端口:"), this);
    m_joinPortSpinBox = new QSpinBox(this);
    m_joinPortSpinBox->setRange(1024, 65535);
    m_joinPortSpinBox->setValue(8888);
    joinLayout->addRow(m_joinPortLabel, m_joinPortSpinBox);
    
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
    
    // 驗證伺服器位址
    if (m_addressEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("請輸入伺服器位址！"));
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
    return m_addressEdit->text().trimmed();
}

quint16 NetworkGameDialog::getServerPort() const
{
    if (m_hostRadio->isChecked()) {
        return static_cast<quint16>(m_portSpinBox->value());
    } else {
        return static_cast<quint16>(m_joinPortSpinBox->value());
    }
}

QString NetworkGameDialog::getPlayerName() const
{
    return m_playerNameEdit->text().trimmed();
}
