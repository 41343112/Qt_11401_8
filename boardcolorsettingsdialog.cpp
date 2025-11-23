#include "boardcolorsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QFrame>
#include <QVector>
#include<QSettings>
#include <QLineEdit>

// 常數
namespace {
    const QString HOVER_BORDER_COLOR = "#4A90E2";
    const QString HOVER_BACKGROUND_COLOR = "#F0F0F0";
    const QString PRESSED_BACKGROUND_COLOR = "#E0E0E0";
    const int MAX_CUSTOM_SLOTS = 7;
    
    // 預設配色方案（不包括自訂插槽）
    const QVector<BoardColorSettingsDialog::ColorScheme> PRESET_SCHEMES = {
        BoardColorSettingsDialog::ColorScheme::Classic,
        BoardColorSettingsDialog::ColorScheme::BlueGray,
        BoardColorSettingsDialog::ColorScheme::GreenWhite,
        BoardColorSettingsDialog::ColorScheme::PurplePink,
        BoardColorSettingsDialog::ColorScheme::WoodDark,
        BoardColorSettingsDialog::ColorScheme::OceanBlue,
        BoardColorSettingsDialog::ColorScheme::LightTheme
    };
    
    // 生成自訂插槽設定鍵前綴的輔助函數
    QString getCustomSlotPrefix(int slotIndex) {
        return QString("customSlot%1_").arg(slotIndex + 1);
    }
}

BoardColorSettingsDialog::BoardColorSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("棋盤顏色設定");
    setModal(true);
    resize(650, 500);
    
    m_settings = getDefaultSettings();
    loadCustomSlots();
    setupUI();
}

BoardColorSettingsDialog::~BoardColorSettingsDialog()
{
}

void BoardColorSettingsDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 配色方案選擇
    QGroupBox* schemeGroup = new QGroupBox("預設配色方案", this);
    QVBoxLayout* schemeLayout = new QVBoxLayout(schemeGroup);
    
    // 預設預覽佈局 - 所有預設在水平佈局中
    QHBoxLayout* presetsLayout = new QHBoxLayout();
    presetsLayout->addWidget(createPresetPreview(ColorScheme::Classic, "經典"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::BlueGray, "藍灰"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::GreenWhite, "綠白"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::PurplePink, "紫粉"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::WoodDark, "木紋深色"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::OceanBlue, "海洋藍"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::LightTheme, "淺色系"));
    presetsLayout->addStretch();
    schemeLayout->addLayout(presetsLayout);
    
    mainLayout->addWidget(schemeGroup);
    
    // 自訂顏色插槽 - 多行包含 7 個自訂插槽
    QGroupBox* customSlotsGroup = new QGroupBox("自訂配色方案", this);
    QVBoxLayout* customSlotsLayout = new QVBoxLayout(customSlotsGroup);
    
    // 第一行：自訂 1-4
    QHBoxLayout* customRow1 = new QHBoxLayout();
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom1, "自訂1"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom2, "自訂2"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom3, "自訂3"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom4, "自訂4"));


    customRow1->addStretch();
    customSlotsLayout->addLayout(customRow1);
    
    // 第二行：自訂 5-7

    
    mainLayout->addWidget(customSlotsGroup);
    
    // 自訂顏色選擇（初始隱藏）
    m_customColorWidget = new QWidget(this);
    QGroupBox* customGroup = new QGroupBox("自訂顏色", m_customColorWidget);
    QVBoxLayout* customLayout = new QVBoxLayout(customGroup);
    
    QLabel* instructionLabel = new QLabel("使用下方按鈕選擇顏色", this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    customLayout->addWidget(instructionLabel);
    
    QHBoxLayout* lightColorLayout = new QHBoxLayout();
    lightColorLayout->addWidget(new QLabel("淺色方格:", this));
    m_lightColorButton = new QPushButton(this);
    m_lightColorButton->setMinimumSize(80, 30);
    connect(m_lightColorButton, &QPushButton::clicked, this, &BoardColorSettingsDialog::onLightColorClicked);
    lightColorLayout->addWidget(m_lightColorButton);
    lightColorLayout->addStretch();
    customLayout->addLayout(lightColorLayout);
    
    QHBoxLayout* darkColorLayout = new QHBoxLayout();
    darkColorLayout->addWidget(new QLabel("深色方格:", this));
    m_darkColorButton = new QPushButton(this);
    m_darkColorButton->setMinimumSize(80, 30);
    connect(m_darkColorButton, &QPushButton::clicked, this, &BoardColorSettingsDialog::onDarkColorClicked);
    darkColorLayout->addWidget(m_darkColorButton);
    darkColorLayout->addStretch();
    customLayout->addLayout(darkColorLayout);
    
    QVBoxLayout* customWidgetLayout = new QVBoxLayout(m_customColorWidget);
    customWidgetLayout->setContentsMargins(0, 0, 0, 0);
    customWidgetLayout->addWidget(customGroup);
    
    m_customColorWidget->setVisible(false);  // 初始隱藏
    mainLayout->addWidget(m_customColorWidget);
    
    // 預覽區域
    QGroupBox* previewGroup = new QGroupBox("預覽 (2×2)", this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewWidget = new QWidget(this);
    QGridLayout* previewGrid = new QGridLayout(m_previewWidget);
    previewGrid->setSpacing(0);
    previewGrid->setContentsMargins(0, 0, 0, 0);
    
    // 創建 2x2 預覽格線
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            m_previewSquares[row][col] = new QLabel(this);
            m_previewSquares[row][col]->setMinimumSize(80, 80);
            m_previewSquares[row][col]->setMaximumSize(80, 80);
            m_previewSquares[row][col]->setFrameStyle(QFrame::Box);
            m_previewSquares[row][col]->setLineWidth(1);
            previewGrid->addWidget(m_previewSquares[row][col], row, col);
        }
    }
    
    previewLayout->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    mainLayout->addWidget(previewGroup);
    
    // 按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* resetButton = new QPushButton("重設為預設值", this);
    connect(resetButton, &QPushButton::clicked, this, &BoardColorSettingsDialog::onResetToDefaults);
    buttonLayout->addWidget(resetButton);
    
    buttonLayout->addStretch();
    
    QPushButton* okButton = new QPushButton("確定", this);
    connect(okButton, &QPushButton::clicked, this, &BoardColorSettingsDialog::onAccept);
    buttonLayout->addWidget(okButton);
    
    QPushButton* cancelButton = new QPushButton("取消", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 初始化 UI 狀態
    updateColorButtons();
    updatePreview();
}

QPushButton* BoardColorSettingsDialog::createPresetPreview(ColorScheme scheme, const QString& label) {
    QPushButton* button = new QPushButton(this);
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    
    // 創建內容元件
    QWidget* contentWidget = new QWidget(button);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // 獲取預設顏色或使用自訂插槽顏色
    BoardColorSettings presetSettings;
    QString displayLabel = label;
    if (isCustomSlot(scheme)) {
        int slotIndex = static_cast<int>(scheme) - static_cast<int>(ColorScheme::Custom1);
        presetSettings = m_customSlots[slotIndex];
        // 如果設置則使用自訂名稱，否則使用預設標籤
        if (!presetSettings.customName.isEmpty()) {
            displayLabel = presetSettings.customName;
        }
    } else {
        presetSettings = getPresetSettings(scheme);
    }
    
    // 創建 2x2 迷你預覽
    QWidget* previewWidget = new QWidget(contentWidget);
    QGridLayout* grid = new QGridLayout(previewWidget);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    // 對於所有方案（包括自訂插槽），顯示實際顏色
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            QLabel* square = new QLabel(contentWidget);
            square->setMinimumSize(PREVIEW_SQUARE_SIZE, PREVIEW_SQUARE_SIZE);
            square->setMaximumSize(PREVIEW_SQUARE_SIZE, PREVIEW_SQUARE_SIZE);
            square->setFrameStyle(QFrame::Box);
            square->setLineWidth(1);
            
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? presetSettings.lightSquareColor : presetSettings.darkSquareColor;
            square->setStyleSheet(QString("QLabel { background-color: %1; }").arg(color.name()));
            
            grid->addWidget(square, row, col);
        }
    }
    
    layout->addWidget(previewWidget, 0, Qt::AlignCenter);
    
    QLabel* nameLabel = new QLabel(displayLabel, contentWidget);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);  // 允許較長名稱的文字換行
    layout->addWidget(nameLabel);
    
    // 設置內容元件幾何以填充按鈕
    contentWidget->setGeometry(0, 0, PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    
    // 添加懸停效果
    button->setStyleSheet(
        QString("QPushButton { border: 2px solid transparent; border-radius: 5px; background-color: transparent; }"
                "QPushButton:hover { border: 2px solid %1; background-color: %2; }"
                "QPushButton:pressed { background-color: %3; }")
        .arg(HOVER_BORDER_COLOR, HOVER_BACKGROUND_COLOR, PRESSED_BACKGROUND_COLOR)
    );
    
    // 連接點擊事件
    connect(button, &QPushButton::clicked, this, [this, scheme]() {
        onPresetPreviewClicked(scheme);
    });
    
    return button;
}

void BoardColorSettingsDialog::updatePreview() {
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? m_settings.lightSquareColor : m_settings.darkSquareColor;
            m_previewSquares[row][col]->setStyleSheet(
                QString("QLabel { background-color: %1; }").arg(color.name())
            );
        }
    }
}

void BoardColorSettingsDialog::updateColorButtons() {
    m_lightColorButton->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 2px solid #333; }")
        .arg(m_settings.lightSquareColor.name())
    );
    
    m_darkColorButton->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 2px solid #333; }")
        .arg(m_settings.darkSquareColor.name())
    );
}



void BoardColorSettingsDialog::applyPresetColorScheme(ColorScheme scheme) {
    BoardColorSettings presetSettings = getPresetSettings(scheme);
    m_settings.lightSquareColor = presetSettings.lightSquareColor;
    m_settings.darkSquareColor = presetSettings.darkSquareColor;
    m_settings.scheme = scheme;
    
    updateColorButtons();
    updatePreview();
}



void BoardColorSettingsDialog::onLightColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.lightSquareColor, this, "選擇淺色方格顏色");
    if (color.isValid()) {
        m_settings.lightSquareColor = color;
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onDarkColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.darkSquareColor, this, "選擇深色方格顏色");
    if (color.isValid()) {
        m_settings.darkSquareColor = color;
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onResetToDefaults() {
    m_settings = getDefaultSettings();
    
    updateColorButtons();
    updatePreview();
}

void BoardColorSettingsDialog::onAccept() {
    // 檢查當前顏色是否匹配任何預設或自訂插槽
    bool matchesExistingScheme = false;
    
    // 檢查所有預設
    for (ColorScheme scheme : PRESET_SCHEMES) {
        BoardColorSettings preset = getPresetSettings(scheme);
        if (m_settings.lightSquareColor == preset.lightSquareColor && 
            m_settings.darkSquareColor == preset.darkSquareColor) {
            matchesExistingScheme = true;
            m_settings.scheme = scheme;
            break;
        }
    }
    
    // 檢查自訂插槽
    if (!matchesExistingScheme) {
        for (int i = 0; i < MAX_CUSTOM_SLOTS; ++i) {
            if (m_settings.lightSquareColor == m_customSlots[i].lightSquareColor && 
                m_settings.darkSquareColor == m_customSlots[i].darkSquareColor) {
                matchesExistingScheme = true;
                m_settings.scheme = static_cast<ColorScheme>(static_cast<int>(ColorScheme::Custom1) + i);
                m_settings.customName = m_customSlots[i].customName;
                break;
            }
        }
    }
    
    // 如果我們已經選擇了自訂插槽，直接儲存到它
    if (isCustomSlot(m_settings.scheme)) {
        int slotIndex = static_cast<int>(m_settings.scheme) - static_cast<int>(ColorScheme::Custom1);
        
        // 詢問此自訂插槽的名稱
        QDialog nameDialog(this);
        nameDialog.setWindowTitle("命名自訂配色");
        nameDialog.setModal(true);
        
        QVBoxLayout* layout = new QVBoxLayout(&nameDialog);
        
        QLabel* label = new QLabel("為這個自訂配色命名（選填）：", &nameDialog);
        layout->addWidget(label);
        
        QLineEdit* nameEdit = new QLineEdit(&nameDialog);
        nameEdit->setPlaceholderText("例如：我的最愛、夜間模式等");
        nameEdit->setText(m_customSlots[slotIndex].customName);
        nameEdit->setMaxLength(20);
        layout->addWidget(nameEdit);
        
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* okButton = new QPushButton("確定", &nameDialog);
        QPushButton* cancelButton = new QPushButton("取消", &nameDialog);
        
        connect(okButton, &QPushButton::clicked, &nameDialog, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, &nameDialog, &QDialog::reject);
        
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);
        
        if (nameDialog.exec() == QDialog::Accepted) {
            // 儲存到選定的自訂插槽
            m_settings.customName = nameEdit->text().trimmed();
            m_customSlots[slotIndex] = m_settings;
            
            saveCustomSlots();
        } else {
            // 使用者取消，不儲存
            return;
        }
    }
    // 如果顏色不匹配任何現有方案且我們尚未在自訂插槽中，詢問要儲存到哪個插槽
    else if (!matchesExistingScheme) {
        QDialog saveDialog(this);
        saveDialog.setWindowTitle("儲存自訂顏色");
        saveDialog.setModal(true);
        
        QVBoxLayout* layout = new QVBoxLayout(&saveDialog);
        
        QLabel* label = new QLabel("選擇要儲存到哪個自訂槽位：", &saveDialog);
        layout->addWidget(label);
        
        QVector<QRadioButton*> slotRadios(MAX_CUSTOM_SLOTS);
        for (int i = 0; i < MAX_CUSTOM_SLOTS; ++i) {
            QString slotLabel = QString("自訂%1").arg(i + 1);
            if (!m_customSlots[i].customName.isEmpty()) {
                slotLabel += QString(" (%1)").arg(m_customSlots[i].customName);
            }
            slotRadios[i] = new QRadioButton(slotLabel, &saveDialog);
            if (i == 0) slotRadios[i]->setChecked(true);
            layout->addWidget(slotRadios[i]);
        }
        
        QLabel* nameLabel = new QLabel("為這個配色命名（選填）：", &saveDialog);
        layout->addWidget(nameLabel);
        
        QLineEdit* nameEdit = new QLineEdit(&saveDialog);
        nameEdit->setPlaceholderText("例如：我的最愛、夜間模式等");
        nameEdit->setMaxLength(20);
        layout->addWidget(nameEdit);
        
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* okButton = new QPushButton("確定", &saveDialog);
        QPushButton* cancelButton = new QPushButton("取消", &saveDialog);
        
        connect(okButton, &QPushButton::clicked, &saveDialog, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, &saveDialog, &QDialog::reject);
        
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);
        
        if (saveDialog.exec() == QDialog::Accepted) {
            int slotIndex = 0;
            for (int i = 0; i < MAX_CUSTOM_SLOTS; ++i) {
                if (slotRadios[i]->isChecked()) {
                    slotIndex = i;
                    break;
                }
            }
            
            // 儲存到選定的自訂插槽
            m_settings.scheme = static_cast<ColorScheme>(static_cast<int>(ColorScheme::Custom1) + slotIndex);
            m_settings.customName = nameEdit->text().trimmed();
            m_customSlots[slotIndex] = m_settings;
            
            saveCustomSlots();
        } else {
            // 使用者取消，不儲存
            return;
        }
    }
    
    accept();
}

void BoardColorSettingsDialog::onPresetPreviewClicked(ColorScheme scheme) {
    if (isCustomSlot(scheme)) {
        // 當點擊自訂插槽時，應用該插槽中儲存的顏色
        int slotIndex = static_cast<int>(scheme) - static_cast<int>(ColorScheme::Custom1);
        m_settings = m_customSlots[slotIndex];
        m_settings.scheme = scheme;
        
        // 顯示自訂顏色元件以進行編輯
        m_customColorWidget->setVisible(true);
    } else {
        // 應用預設配色方案
        applyPresetColorScheme(scheme);
        
        // 隱藏自訂顏色元件
        m_customColorWidget->setVisible(false);
    }
    
    updateColorButtons();
    updatePreview();
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getSettings() const {
    return m_settings;
}

void BoardColorSettingsDialog::setSettings(const BoardColorSettings& settings) {
    m_settings = settings;
    
    updateColorButtons();
    updatePreview();
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getDefaultSettings() {
    BoardColorSettings settings;
    settings.lightSquareColor = QColor("#F0D9B5");  // 經典淺米色
    settings.darkSquareColor = QColor("#B58863");   // 經典棕色
    settings.scheme = ColorScheme::Classic;
    return settings;
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getPresetSettings(ColorScheme scheme) {
    BoardColorSettings settings;
    
    switch (scheme) {
        case ColorScheme::Classic:
            settings.lightSquareColor = QColor("#F0D9B5");  // 經典淺米色
            settings.darkSquareColor = QColor("#B58863");   // 經典棕色
            break;
            
        case ColorScheme::BlueGray:
            settings.lightSquareColor = QColor("#DEE3E6");  // 淺灰藍色
            settings.darkSquareColor = QColor("#8CA2AD");   // 深灰藍色
            break;
            
        case ColorScheme::GreenWhite:
            settings.lightSquareColor = QColor("#FFFFDD");  // 淺奶油色/白色
            settings.darkSquareColor = QColor("#86A666");   // 森林綠
            break;
            
        case ColorScheme::PurplePink:
            settings.lightSquareColor = QColor("#E8C4E8");  // 淺紫色/粉色
            settings.darkSquareColor = QColor("#9B6B9B");   // 深紫色
            break;
            
        case ColorScheme::WoodDark:
            settings.lightSquareColor = QColor("#D4A574");  // 淺木色
            settings.darkSquareColor = QColor("#6B4423");   // 深木色/棕色
            break;
            
        case ColorScheme::OceanBlue:
            settings.lightSquareColor = QColor("#A8D8EA");  // 淺海藍色
            settings.darkSquareColor = QColor("#2E5B6D");   // 深海藍色
            break;
            
        case ColorScheme::LightTheme:
            settings.lightSquareColor = QColor("#FEFEFE");  // 極淺灰色（幾乎白色）
            settings.darkSquareColor = QColor("#E0E0E0");   // 淺灰色
            break;
            
        case ColorScheme::Custom1:
        case ColorScheme::Custom2:
        case ColorScheme::Custom3:
        case ColorScheme::Custom4:
        case ColorScheme::Custom5:
        case ColorScheme::Custom6:
        case ColorScheme::Custom7:
            // 對於自訂插槽，返回預設顏色作為佔位符
            settings.lightSquareColor = QColor("#F0D9B5");
            settings.darkSquareColor = QColor("#B58863");
            break;
    }
    
    settings.scheme = scheme;
    return settings;
}

void BoardColorSettingsDialog::loadCustomSlots() {
    QSettings settings("Qt_Chess", "BoardColorSettings");
    
    for (int i = 0; i < MAX_CUSTOM_SLOTS; ++i) {
        QString prefix = getCustomSlotPrefix(i);
        
        QString lightColorStr = settings.value(prefix + "lightColor", "#F0D9B5").toString();
        QString darkColorStr = settings.value(prefix + "darkColor", "#B58863").toString();
        QString name = settings.value(prefix + "name", "").toString();
        
        m_customSlots[i].lightSquareColor = QColor(lightColorStr);
        m_customSlots[i].darkSquareColor = QColor(darkColorStr);
        m_customSlots[i].customName = name;
        
        // 驗證顏色
        if (!m_customSlots[i].lightSquareColor.isValid()) {
            m_customSlots[i].lightSquareColor = QColor("#F0D9B5");
        }
        if (!m_customSlots[i].darkSquareColor.isValid()) {
            m_customSlots[i].darkSquareColor = QColor("#B58863");
        }
        
        m_customSlots[i].scheme = static_cast<ColorScheme>(static_cast<int>(ColorScheme::Custom1) + i);
    }
}

void BoardColorSettingsDialog::saveCustomSlots() {
    QSettings settings("Qt_Chess", "BoardColorSettings");
    
    for (int i = 0; i < MAX_CUSTOM_SLOTS; ++i) {
        QString prefix = getCustomSlotPrefix(i);
        
        settings.setValue(prefix + "lightColor", m_customSlots[i].lightSquareColor.name());
        settings.setValue(prefix + "darkColor", m_customSlots[i].darkSquareColor.name());
        settings.setValue(prefix + "name", m_customSlots[i].customName);
    }
}

bool BoardColorSettingsDialog::isCustomSlot(ColorScheme scheme) const {
    return scheme == ColorScheme::Custom1 || 
           scheme == ColorScheme::Custom2 || 
           scheme == ColorScheme::Custom3 ||
           scheme == ColorScheme::Custom4 ||
           scheme == ColorScheme::Custom5 ||
           scheme == ColorScheme::Custom6 ||
           scheme == ColorScheme::Custom7;
}
