#include "boardcolorsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QFrame>
#include <QVector>
#include<QSettings>
#include <QLineEdit>
// Constants
namespace {
    const QString HOVER_BORDER_COLOR = "#4A90E2";
    const QString HOVER_BACKGROUND_COLOR = "#F0F0F0";
    const QString PRESSED_BACKGROUND_COLOR = "#E0E0E0";
    const int MAX_CUSTOM_SLOTS = 7;
    
    // Preset color schemes (not including custom slots)
    const QVector<BoardColorSettingsDialog::ColorScheme> PRESET_SCHEMES = {
        BoardColorSettingsDialog::ColorScheme::Classic,
        BoardColorSettingsDialog::ColorScheme::BlueGray,
        BoardColorSettingsDialog::ColorScheme::GreenWhite
    };
    
    // Helper function to generate settings key prefix for custom slots
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
    
    // Color scheme selection
    QGroupBox* schemeGroup = new QGroupBox("預設配色方案", this);
    QVBoxLayout* schemeLayout = new QVBoxLayout(schemeGroup);
    
    // Preset previews layout - first row with 3 presets
    QHBoxLayout* presetsLayout = new QHBoxLayout();
    presetsLayout->addWidget(createPresetPreview(ColorScheme::Classic, "經典"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::BlueGray, "藍灰"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::GreenWhite, "綠白"));
    presetsLayout->addStretch();
    schemeLayout->addLayout(presetsLayout);
    
    mainLayout->addWidget(schemeGroup);
    
    // Custom color slots - multiple rows with 7 custom slots
    QGroupBox* customSlotsGroup = new QGroupBox("自訂配色方案", this);
    QVBoxLayout* customSlotsLayout = new QVBoxLayout(customSlotsGroup);
    
    // First row: Custom 1-4
    QHBoxLayout* customRow1 = new QHBoxLayout();
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom1, "自訂1"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom2, "自訂2"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom3, "自訂3"));
    customRow1->addWidget(createPresetPreview(ColorScheme::Custom4, "自訂4"));
    customRow1->addStretch();
    customSlotsLayout->addLayout(customRow1);
    
    // Second row: Custom 5-7

    
    mainLayout->addWidget(customSlotsGroup);
    
    // Custom color selection (initially hidden)
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
    
    m_customColorWidget->setVisible(false);  // Initially hidden
    mainLayout->addWidget(m_customColorWidget);
    
    // Preview section
    QGroupBox* previewGroup = new QGroupBox("預覽 (2×2)", this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewWidget = new QWidget(this);
    QGridLayout* previewGrid = new QGridLayout(m_previewWidget);
    previewGrid->setSpacing(0);
    previewGrid->setContentsMargins(0, 0, 0, 0);
    
    // Create 2x2 preview grid
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
    
    // Buttons
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
    
    // Initialize UI state
    updateColorButtons();
    updatePreview();
}

QPushButton* BoardColorSettingsDialog::createPresetPreview(ColorScheme scheme, const QString& label) {
    QPushButton* button = new QPushButton(this);
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    
    // Create content widget
    QWidget* contentWidget = new QWidget(button);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // Get preset colors or use custom slot colors
    BoardColorSettings presetSettings;
    QString displayLabel = label;
    if (isCustomSlot(scheme)) {
        int slotIndex = static_cast<int>(scheme) - static_cast<int>(ColorScheme::Custom1);
        presetSettings = m_customSlots[slotIndex];
        // Use custom name if set, otherwise use default label
        if (!presetSettings.customName.isEmpty()) {
            displayLabel = presetSettings.customName;
        }
    } else {
        presetSettings = getPresetSettings(scheme);
    }
    
    // Create 2x2 mini preview
    QWidget* previewWidget = new QWidget(contentWidget);
    QGridLayout* grid = new QGridLayout(previewWidget);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    // For all schemes (including custom slots), show the actual colors
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
    nameLabel->setWordWrap(true);  // Allow text wrapping for longer names
    layout->addWidget(nameLabel);
    
    // Set content widget geometry to fill button
    contentWidget->setGeometry(0, 0, PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    
    // Add hover effect
    button->setStyleSheet(
        QString("QPushButton { border: 2px solid transparent; border-radius: 5px; background-color: transparent; }"
                "QPushButton:hover { border: 2px solid %1; background-color: %2; }"
                "QPushButton:pressed { background-color: %3; }")
        .arg(HOVER_BORDER_COLOR, HOVER_BACKGROUND_COLOR, PRESSED_BACKGROUND_COLOR)
    );
    
    // Connect click event
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
    // Check if the current colors match any preset or custom slot
    bool matchesExistingScheme = false;
    
    // Check all presets
    for (ColorScheme scheme : PRESET_SCHEMES) {
        BoardColorSettings preset = getPresetSettings(scheme);
        if (m_settings.lightSquareColor == preset.lightSquareColor && 
            m_settings.darkSquareColor == preset.darkSquareColor) {
            matchesExistingScheme = true;
            m_settings.scheme = scheme;
            break;
        }
    }
    
    // Check custom slots
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
    
    // If we already selected a custom slot, save directly to it
    if (isCustomSlot(m_settings.scheme)) {
        int slotIndex = static_cast<int>(m_settings.scheme) - static_cast<int>(ColorScheme::Custom1);
        
        // Ask for a name for this custom slot
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
            // Save to the selected custom slot
            m_settings.customName = nameEdit->text().trimmed();
            m_customSlots[slotIndex] = m_settings;
            
            saveCustomSlots();
        } else {
            // User cancelled, don't save
            return;
        }
    }
    // If colors don't match any existing scheme and we're not in a custom slot already, ask which slot to save to
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
            
            // Save to the selected custom slot
            m_settings.scheme = static_cast<ColorScheme>(static_cast<int>(ColorScheme::Custom1) + slotIndex);
            m_settings.customName = nameEdit->text().trimmed();
            m_customSlots[slotIndex] = m_settings;
            
            saveCustomSlots();
        } else {
            // User cancelled, don't save
            return;
        }
    }
    
    accept();
}

void BoardColorSettingsDialog::onPresetPreviewClicked(ColorScheme scheme) {
    if (isCustomSlot(scheme)) {
        // When custom slot is clicked, apply the saved colors from that slot
        int slotIndex = static_cast<int>(scheme) - static_cast<int>(ColorScheme::Custom1);
        m_settings = m_customSlots[slotIndex];
        m_settings.scheme = scheme;
        
        // Show custom color widget for editing
        m_customColorWidget->setVisible(true);
    } else {
        // Apply the preset color scheme
        applyPresetColorScheme(scheme);
        
        // Hide custom color widget
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
    settings.lightSquareColor = QColor("#F0D9B5");  // Classic light beige
    settings.darkSquareColor = QColor("#B58863");   // Classic brown
    settings.scheme = ColorScheme::Classic;
    return settings;
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getPresetSettings(ColorScheme scheme) {
    BoardColorSettings settings;
    
    switch (scheme) {
        case ColorScheme::Classic:
            settings.lightSquareColor = QColor("#F0D9B5");  // Classic light beige
            settings.darkSquareColor = QColor("#B58863");   // Classic brown
            break;
            
        case ColorScheme::BlueGray:
            settings.lightSquareColor = QColor("#DEE3E6");  // Light gray-blue
            settings.darkSquareColor = QColor("#8CA2AD");   // Dark gray-blue
            break;
            
        case ColorScheme::GreenWhite:
            settings.lightSquareColor = QColor("#FFFFDD");  // Light cream/white
            settings.darkSquareColor = QColor("#86A666");   // Forest green
            break;
            
        case ColorScheme::Custom1:
        case ColorScheme::Custom2:
        case ColorScheme::Custom3:
        case ColorScheme::Custom4:
        case ColorScheme::Custom5:
        case ColorScheme::Custom6:
        case ColorScheme::Custom7:
            // For Custom slots, return default colors as placeholder
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
        
        // Validate colors
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
