#include "onlinedialog.h"
#include "qt_chess.h"  // For game mode constants
#include "theme.h"
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
    
    // 應用歐式古典風格
    applyClassicalDialogStyle();
}

OnlineDialog::~OnlineDialog()
{
}

void OnlineDialog::applyClassicalDialogStyle()
{
    // 歐式古典風格對話框樣式表
    QString styleSheet = QString(
        "QDialog { "
        "  background-color: %1; "
        "}"
        "QGroupBox { "
        "  font-weight: bold; "
        "  color: %2; "
        "  border: 2px solid %3; "
        "  border-radius: 6px; "
        "  margin-top: 12px; "
        "  padding-top: 12px; "
        "  background-color: %4; "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  subcontrol-position: top left; "
        "  padding: 4px 12px; "
        "  color: %2; "
        "  background-color: %4; "
        "  border: 1px solid %3; "
        "  border-radius: 4px; "
        "  left: 10px; "
        "}"
        "QCheckBox { "
        "  color: %5; "
        "  font-size: 10pt; "
        "  padding: 3px; "
        "}"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QCheckBox::indicator:unchecked { "
        "  border: 2px solid %3; border-radius: 3px; background-color: %1; "
        "}"
        "QCheckBox::indicator:checked { "
        "  border: 2px solid %2; border-radius: 3px; "
        "  background-color: %6; "
        "}"
        "QPushButton { "
        "  background-color: %4; "
        "  color: %5; "
        "  border: 2px solid %3; "
        "  border-radius: 6px; "
        "  padding: 10px 20px; "
        "  font-weight: 500; "
        "}"
        "QPushButton:hover { "
        "  background-color: %7; "
        "  border-color: %2; "
        "  color: %2; "
        "}"
        "QPushButton:pressed { background-color: %3; }"
    ).arg(Theme::BG_MEDIUM, Theme::ACCENT_PRIMARY, Theme::BORDER, 
          Theme::BG_PANEL, Theme::TEXT_PRIMARY, Theme::ACCENT_SECONDARY, 
          Theme::BG_DARK);
    
    setStyleSheet(styleSheet);
}

void OnlineDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 遊戲模式選擇
    QGroupBox* gameModeGroup = new QGroupBox(tr("🎯 選擇遊戲模式（可複選）"), this);
    QVBoxLayout* gameModeLayout = new QVBoxLayout(gameModeGroup);
    
    m_standardModeCheckbox = new QCheckBox(tr("🌫️ 霧戰"), this);
    m_standardModeCheckbox->setChecked(true); // 默認勾選
    
    m_rapidModeCheckbox = new QCheckBox(tr("🌍 地吸引力"), this);
    
    m_blitzModeCheckbox = new QCheckBox(tr("🔮 傳送陣"), this);
    
    m_handicapModeCheckbox = new QCheckBox(tr("🎲 骰子"), this);
    
    m_customRulesCheckbox = new QCheckBox(tr("💣 踩地雷"), this);
    
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
    m_cancelButton = new QPushButton(tr("取消"), this);
    
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
