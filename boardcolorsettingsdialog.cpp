#include "boardcolorsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QFrame>

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

QWidget* BoardColorSettingsDialog::createPresetPreview(ColorScheme scheme, const QString& label) {
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(5, 5, 5, 5);
    
    BoardColorSettings presetSettings = getPresetSettings(scheme);
    
    // Create 2x2 mini preview
    QWidget* previewWidget = new QWidget(this);
    QGridLayout* grid = new QGridLayout(previewWidget);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            QLabel* square = new QLabel(this);
            square->setMinimumSize(30, 30);
            square->setMaximumSize(30, 30);
            square->setFrameStyle(QFrame::Box);
            square->setLineWidth(1);
            
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? presetSettings.lightSquareColor : presetSettings.darkSquareColor;
            square->setStyleSheet(QString("QLabel { background-color: %1; }").arg(color.name()));
            
            grid->addWidget(square, row, col);
        }
    }
    
    layout->addWidget(previewWidget, 0, Qt::AlignCenter);
    
    QLabel* nameLabel = new QLabel(label, this);
    nameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(nameLabel);
    
    return container;
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

void BoardColorSettingsDialog::onLightColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.lightSquareColor, this, "選擇淺色方格顏色");
    if (color.isValid()) {
        m_settings.lightSquareColor = color;
        m_settings.scheme = ColorScheme::Custom;
        
        // Update combo box to show Custom
        for (int i = 0; i < m_colorSchemeComboBox->count(); ++i) {
            if (static_cast<ColorScheme>(m_colorSchemeComboBox->itemData(i).toInt()) == ColorScheme::Custom) {
                m_colorSchemeComboBox->setCurrentIndex(i);
                break;
            }
        }
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onDarkColorClicked() {
    QColor color = QColorDialog::getColor(m_settings.darkSquareColor, this, "選擇深色方格顏色");
    if (color.isValid()) {
        m_settings.darkSquareColor = color;
        m_settings.scheme = ColorScheme::Custom;
        
        // Update combo box to show Custom
        for (int i = 0; i < m_colorSchemeComboBox->count(); ++i) {
            if (static_cast<ColorScheme>(m_colorSchemeComboBox->itemData(i).toInt()) == ColorScheme::Custom) {
                m_colorSchemeComboBox->setCurrentIndex(i);
                break;
            }
        }
        
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onResetToDefaults() {
    m_settings = getDefaultSettings();
    
    // Update combo box
    for (int i = 0; i < m_colorSchemeComboBox->count(); ++i) {
        if (static_cast<ColorScheme>(m_colorSchemeComboBox->itemData(i).toInt()) == m_settings.scheme) {
            m_colorSchemeComboBox->setCurrentIndex(i);
            break;
        }
    }
    
    updateColorButtons();
    updatePreview();
}

void BoardColorSettingsDialog::onAccept() {
    accept();
}

BoardColorSettingsDialog::BoardColorSettings BoardColorSettingsDialog::getSettings() const {
    return m_settings;
}

void BoardColorSettingsDialog::setSettings(const BoardColorSettings& settings) {
    m_settings = settings;
    
    // Update combo box
    for (int i = 0; i < m_colorSchemeComboBox->count(); ++i) {
        if (static_cast<ColorScheme>(m_colorSchemeComboBox->itemData(i).toInt()) == settings.scheme) {
            m_colorSchemeComboBox->setCurrentIndex(i);
            break;
        }
    }
    
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
            // Return current custom colors or default
            settings.lightSquareColor = QColor("#F0D9B5");
            settings.darkSquareColor = QColor("#B58863");
            break;
    }
    
    settings.scheme = scheme;
    return settings;
}
