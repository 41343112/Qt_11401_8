#include "boardcolorsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QFrame>

// UI Style Constants
namespace {
    const QString HOVER_BORDER_COLOR = "#4A90E2";
    const QString HOVER_BACKGROUND_COLOR = "#F0F0F0";
    const QString PRESSED_BACKGROUND_COLOR = "#E0E0E0";
    
    // Custom preview gradient colors
    const QColor CUSTOM_COLOR_TOP_LEFT = QColor("#FF6B6B");     // Red
    const QColor CUSTOM_COLOR_TOP_RIGHT = QColor("#4ECDC4");    // Cyan
    const QColor CUSTOM_COLOR_BOTTOM_LEFT = QColor("#95E1D3");  // Light green
    const QColor CUSTOM_COLOR_BOTTOM_RIGHT = QColor("#FFE66D"); // Yellow
}

BoardColorSettingsDialog::BoardColorSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("棋盤顏色設定");
    setModal(true);
    resize(550, 500);
    
    m_settings = getDefaultSettings();
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
    
    m_colorSchemeComboBox = new QComboBox(this);
    m_colorSchemeComboBox->addItem("經典棕褐色", static_cast<int>(ColorScheme::Classic));
    m_colorSchemeComboBox->addItem("藍灰配色", static_cast<int>(ColorScheme::BlueGray));
    m_colorSchemeComboBox->addItem("綠白配色", static_cast<int>(ColorScheme::GreenWhite));
    m_colorSchemeComboBox->addItem("自訂顏色", static_cast<int>(ColorScheme::Custom));
    connect(m_colorSchemeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoardColorSettingsDialog::onColorSchemeChanged);
    schemeLayout->addWidget(m_colorSchemeComboBox);
    
    // Preset previews layout
    QHBoxLayout* presetsLayout = new QHBoxLayout();
    presetsLayout->addWidget(createPresetPreview(ColorScheme::Classic, "經典"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::BlueGray, "藍灰"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::GreenWhite, "綠白"));
    presetsLayout->addWidget(createPresetPreview(ColorScheme::Custom, "自訂"));
    presetsLayout->addStretch();
    schemeLayout->addLayout(presetsLayout);
    
    mainLayout->addWidget(schemeGroup);
    
    // Custom color selection
    QGroupBox* customGroup = new QGroupBox("自訂顏色", this);
    QVBoxLayout* customLayout = new QVBoxLayout(customGroup);
    
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
    
    mainLayout->addWidget(customGroup);
    
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
    
    // Get preset colors or use custom indicator
    BoardColorSettings presetSettings = getPresetSettings(scheme);
    
    // Create 2x2 mini preview
    QWidget* previewWidget = new QWidget(contentWidget);
    QGridLayout* grid = new QGridLayout(previewWidget);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    if (scheme == ColorScheme::Custom) {
        // For custom scheme, show a color palette icon or gradient
        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 2; ++col) {
                QLabel* square = new QLabel(contentWidget);
                square->setMinimumSize(PREVIEW_SQUARE_SIZE, PREVIEW_SQUARE_SIZE);
                square->setMaximumSize(PREVIEW_SQUARE_SIZE, PREVIEW_SQUARE_SIZE);
                square->setFrameStyle(QFrame::Box);
                square->setLineWidth(1);
                
                // Create a colorful pattern for custom option
                QColor color;
                if (row == 0 && col == 0) color = CUSTOM_COLOR_TOP_LEFT;
                else if (row == 0 && col == 1) color = CUSTOM_COLOR_TOP_RIGHT;
                else if (row == 1 && col == 0) color = CUSTOM_COLOR_BOTTOM_LEFT;
                else color = CUSTOM_COLOR_BOTTOM_RIGHT;
                
                square->setStyleSheet(QString("QLabel { background-color: %1; }").arg(color.name()));
                grid->addWidget(square, row, col);
            }
        }
    } else {
        // For preset schemes, show the actual colors
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
    }
    
    layout->addWidget(previewWidget, 0, Qt::AlignCenter);
    
    QLabel* nameLabel = new QLabel(label, contentWidget);
    nameLabel->setAlignment(Qt::AlignCenter);
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

void BoardColorSettingsDialog::onColorSchemeChanged(int index) {
    ColorScheme scheme = static_cast<ColorScheme>(m_colorSchemeComboBox->itemData(index).toInt());
    
    if (scheme != ColorScheme::Custom) {
        applyPresetColorScheme(scheme);
    }
    
    m_settings.scheme = scheme;
    
    // Enable/disable custom color buttons
    bool isCustom = (scheme == ColorScheme::Custom);
    m_lightColorButton->setEnabled(isCustom);
    m_darkColorButton->setEnabled(isCustom);
}

void BoardColorSettingsDialog::applyPresetColorScheme(ColorScheme scheme) {
    BoardColorSettings presetSettings = getPresetSettings(scheme);
    m_settings.lightSquareColor = presetSettings.lightSquareColor;
    m_settings.darkSquareColor = presetSettings.darkSquareColor;
    m_settings.scheme = scheme;
    
    updateColorButtons();
    updatePreview();
}

void BoardColorSettingsDialog::setComboBoxScheme(ColorScheme scheme) {
    int index = m_colorSchemeComboBox->findData(static_cast<int>(scheme));
    if (index != -1) {
        m_colorSchemeComboBox->setCurrentIndex(index);
    }
}

void BoardColorSettingsDialog::onLightColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.lightSquareColor, this, "選擇淺色方格顏色");
    if (color.isValid()) {
        m_settings.lightSquareColor = color;
        m_settings.scheme = ColorScheme::Custom;
        
        setComboBoxScheme(ColorScheme::Custom);
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onDarkColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.darkSquareColor, this, "選擇深色方格顏色");
    if (color.isValid()) {
        m_settings.darkSquareColor = color;
        m_settings.scheme = ColorScheme::Custom;
        
        setComboBoxScheme(ColorScheme::Custom);
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onResetToDefaults() {
    m_settings = getDefaultSettings();
    
    setComboBoxScheme(m_settings.scheme);
    
    updateColorButtons();
    updatePreview();
}

void BoardColorSettingsDialog::onAccept() {
    accept();
}

void BoardColorSettingsDialog::onPresetPreviewClicked(ColorScheme scheme) {
    if (scheme == ColorScheme::Custom) {
        // When custom preview is clicked, show color picker for light square first
        QColor lightColor = QColorDialog::getColor(m_settings.lightSquareColor, this, "選擇淺色方格顏色");
        if (lightColor.isValid()) {
            // Then show color picker for dark square
            QColor darkColor = QColorDialog::getColor(m_settings.darkSquareColor, this, "選擇深色方格顏色");
            if (darkColor.isValid()) {
                // Only apply changes if both colors were selected
                m_settings.lightSquareColor = lightColor;
                m_settings.darkSquareColor = darkColor;
                m_settings.scheme = ColorScheme::Custom;
                
                setComboBoxScheme(ColorScheme::Custom);
                updateColorButtons();
                updatePreview();
            }
            // If user cancels dark color selection, don't apply any changes
        }
    } else {
        // Apply the preset color scheme
        applyPresetColorScheme(scheme);
        setComboBoxScheme(scheme);
    }
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getSettings() const {
    return m_settings;
}

void BoardColorSettingsDialog::setSettings(const BoardColorSettings& settings) {
    m_settings = settings;
    
    setComboBoxScheme(settings.scheme);
    
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
            
        case ColorScheme::Custom:
            // For Custom scheme, return default colors as placeholder
            // Actual custom colors are managed by the dialog state
            settings.lightSquareColor = QColor("#F0D9B5");
            settings.darkSquareColor = QColor("#B58863");
            break;
    }
    
    settings.scheme = scheme;
    return settings;
}
