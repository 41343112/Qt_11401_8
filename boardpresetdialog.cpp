#include "boardpresetdialog.h"
#include <QGroupBox>
#include <QDebug>

BoardPresetDialog::BoardPresetDialog(QWidget *parent)
    : QDialog(parent), m_selectedPreset(BoardPreset::Standard)
{
    setWindowTitle("選擇棋盤佈局");
    setModal(true);
    setupUI();
    resize(400, 300);
}

BoardPresetDialog::~BoardPresetDialog()
{
}

void BoardPresetDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Title label
    QLabel* titleLabel = new QLabel("請選擇棋盤佈局：", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(10);
    
    // Create button group for radio buttons
    m_buttonGroup = new QButtonGroup(this);
    
    // Group box for presets
    QGroupBox* presetGroupBox = new QGroupBox("預設棋盤", this);
    QVBoxLayout* presetLayout = new QVBoxLayout(presetGroupBox);
    
    // Standard preset
    m_standardRadio = new QRadioButton("標準開局", this);
    QLabel* standardDesc = new QLabel("  經典的國際象棋開局位置，所有棋子在初始位置", this);
    standardDesc->setWordWrap(true);
    standardDesc->setStyleSheet("color: gray; font-size: 10pt;");
    m_standardRadio->setChecked(true);
    presetLayout->addWidget(m_standardRadio);
    presetLayout->addWidget(standardDesc);
    presetLayout->addSpacing(10);
    m_buttonGroup->addButton(m_standardRadio, 0);
    
    // Mid-game preset
    m_midGameRadio = new QRadioButton("中局練習", this);
    QLabel* midGameDesc = new QLabel("  中局戰術位置，適合練習中盤戰術和策略", this);
    midGameDesc->setWordWrap(true);
    midGameDesc->setStyleSheet("color: gray; font-size: 10pt;");
    presetLayout->addWidget(m_midGameRadio);
    presetLayout->addWidget(midGameDesc);
    presetLayout->addSpacing(10);
    m_buttonGroup->addButton(m_midGameRadio, 1);
    
    // Endgame preset
    m_endGameRadio = new QRadioButton("殘局練習", this);
    QLabel* endGameDesc = new QLabel("  王兵殘局，適合練習殘局技巧和兵的升變", this);
    endGameDesc->setWordWrap(true);
    endGameDesc->setStyleSheet("color: gray; font-size: 10pt;");
    presetLayout->addWidget(m_endGameRadio);
    presetLayout->addWidget(endGameDesc);
    m_buttonGroup->addButton(m_endGameRadio, 2);
    
    mainLayout->addWidget(presetGroupBox);
    mainLayout->addStretch();
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* okButton = new QPushButton("確定", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    
    okButton->setDefault(true);
    okButton->setMinimumWidth(80);
    cancelButton->setMinimumWidth(80);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(okButton, &QPushButton::clicked, this, &BoardPresetDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &BoardPresetDialog::onCancelClicked);
}

void BoardPresetDialog::onOkClicked()
{
    // Determine which preset was selected
    int checkedId = m_buttonGroup->checkedId();
    switch (checkedId) {
        case 0:
            m_selectedPreset = BoardPreset::Standard;
            break;
        case 1:
            m_selectedPreset = BoardPreset::MidGame;
            break;
        case 2:
            m_selectedPreset = BoardPreset::EndGame;
            break;
        default:
            // This should never happen with controlled radio button selection
            // Log error and default to Standard preset
            qWarning() << "BoardPresetDialog: Invalid button ID" << checkedId << "- defaulting to Standard preset";
            m_selectedPreset = BoardPreset::Standard;
            break;
    }
    
    accept();
}

void BoardPresetDialog::onCancelClicked()
{
    reject();
}
