#include "onlinedialog.h"
#include "qt_chess.h"  // For game mode constants
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

OnlineDialog::OnlineDialog(QWidget *parent)
    : QDialog(parent)
    , m_standardModeCheckbox(nullptr)
    , m_rapidModeCheckbox(nullptr)
    , m_blitzModeCheckbox(nullptr)
    , m_handicapModeCheckbox(nullptr)
    , m_customRulesCheckbox(nullptr)
{
    setupUI();
    setWindowTitle(tr("選擇遊戲模式"));
    resize(400, 350);
}

OnlineDialog::~OnlineDialog()
{
}

void OnlineDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 遊戲模式選擇
    QGroupBox* gameModeGroup = new QGroupBox(tr("🎯 選擇遊戲模式（可複選）"), this);
    gameModeGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #2196F3; }");
    QVBoxLayout* gameModeLayout = new QVBoxLayout(gameModeGroup);
    
    m_standardModeCheckbox = new QCheckBox(tr("🌫️ 霧戰"), this);
    m_standardModeCheckbox->setStyleSheet("QCheckBox { font-size: 10pt; padding: 3px; color: white; }");
    m_standardModeCheckbox->setChecked(true); // 默認勾選
    
    m_rapidModeCheckbox = new QCheckBox(tr("🌍 地吸引力"), this);
    m_rapidModeCheckbox->setStyleSheet("QCheckBox { font-size: 10pt; padding: 3px; color: white; }");
    
    m_blitzModeCheckbox = new QCheckBox(tr("🔮 傳送陣"), this);
    m_blitzModeCheckbox->setStyleSheet("QCheckBox { font-size: 10pt; padding: 3px; color: white; }");
    
    m_handicapModeCheckbox = new QCheckBox(tr("🎲 骰子"), this);
    m_handicapModeCheckbox->setStyleSheet("QCheckBox { font-size: 10pt; padding: 3px; color: white; }");
    
    m_customRulesCheckbox = new QCheckBox(tr("💣 踩地雷"), this);
    m_customRulesCheckbox->setStyleSheet("QCheckBox { font-size: 10pt; padding: 3px; color: white; }");
    
    gameModeLayout->addWidget(m_standardModeCheckbox);
    gameModeLayout->addWidget(m_rapidModeCheckbox);
    gameModeLayout->addWidget(m_blitzModeCheckbox);
    gameModeLayout->addWidget(m_handicapModeCheckbox);
    gameModeLayout->addWidget(m_customRulesCheckbox);
    
    mainLayout->addWidget(gameModeGroup);
    
    mainLayout->addStretch();
    
    // 按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("確定"), this);
    m_okButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 10px 20px; font-weight: bold; }");
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_cancelButton->setStyleSheet("QPushButton { padding: 10px 20px; }");
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 連接信號
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QMap<QString, bool> OnlineDialog::getGameModes() const
{
    QMap<QString, bool> gameModes;
    
    if (m_standardModeCheckbox) {
        gameModes[GAME_MODE_FOG_OF_WAR] = m_standardModeCheckbox->isChecked();
    }
    if (m_rapidModeCheckbox) {
        gameModes[GAME_MODE_GRAVITY] = m_rapidModeCheckbox->isChecked();
    }
    if (m_blitzModeCheckbox) {
        gameModes[GAME_MODE_TELEPORT] = m_blitzModeCheckbox->isChecked();
    }
    if (m_handicapModeCheckbox) {
        gameModes[GAME_MODE_DICE] = m_handicapModeCheckbox->isChecked();
    }
    if (m_customRulesCheckbox) {
        gameModes[GAME_MODE_BOMB] = m_customRulesCheckbox->isChecked();
    }
    
    return gameModes;
}
