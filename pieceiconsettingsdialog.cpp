#include "pieceiconsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPixmap>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>

PieceIconSettingsDialog::PieceIconSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("棋子圖標設定");
    resize(700, 600);
    setupUI();
}

PieceIconSettingsDialog::~PieceIconSettingsDialog()
{
}

void PieceIconSettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Icon Set Selection Group with visual previews
    QGroupBox* iconSetGroup = new QGroupBox("圖標集選擇 - 點擊預覽圖選擇", this);
    QVBoxLayout* iconSetLayout = new QVBoxLayout(iconSetGroup);
    
    // Create button group for radio buttons
    m_iconSetButtonGroup = new QButtonGroup(this);
    connect(m_iconSetButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PieceIconSettingsDialog::onIconSetButtonClicked);
    
    // Create a grid layout for icon set previews
    QGridLayout* previewGrid = new QGridLayout();
    previewGrid->setSpacing(15);
    
    // Row 1: Unicode and Preset1
    QWidget* unicodeWidget = createIconSetPreviewWidget(IconSetType::Unicode, "Unicode 符號 (預設)");
    previewGrid->addWidget(unicodeWidget, 0, 0);
    
    QWidget* preset1Widget = createIconSetPreviewWidget(IconSetType::Preset1, "預設圖標集 1");
    previewGrid->addWidget(preset1Widget, 0, 1);
    
    // Row 2: Preset2 and Preset3
    QWidget* preset2Widget = createIconSetPreviewWidget(IconSetType::Preset2, "預設圖標集 2");
    previewGrid->addWidget(preset2Widget, 1, 0);
    
    QWidget* preset3Widget = createIconSetPreviewWidget(IconSetType::Preset3, "預設圖標集 3");
    previewGrid->addWidget(preset3Widget, 1, 1);
    
    // Row 3: Custom (spans both columns)
    QWidget* customWidget = createIconSetPreviewWidget(IconSetType::Custom, "自訂圖標");
    previewGrid->addWidget(customWidget, 2, 0, 1, 2);
    
    iconSetLayout->addLayout(previewGrid);
    mainLayout->addWidget(iconSetGroup);
    
    // Keep combo box hidden but functional for backward compatibility
    m_iconSetComboBox = new QComboBox(this);
    m_iconSetComboBox->addItem("Unicode 符號 (預設)", static_cast<int>(IconSetType::Unicode));
    m_iconSetComboBox->addItem("預設圖標集 1", static_cast<int>(IconSetType::Preset1));
    m_iconSetComboBox->addItem("預設圖標集 2", static_cast<int>(IconSetType::Preset2));
    m_iconSetComboBox->addItem("預設圖標集 3", static_cast<int>(IconSetType::Preset3));
    m_iconSetComboBox->addItem("自訂圖標", static_cast<int>(IconSetType::Custom));
    m_iconSetComboBox->setVisible(false);  // Hidden, using visual interface instead
    connect(m_iconSetComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PieceIconSettingsDialog::onIconSetTypeChanged);
    
    // Use custom icons checkbox (legacy, for backward compatibility)
    m_useCustomIconsCheckBox = new QCheckBox("使用自訂圖標 (舊版相容)", this);
    m_useCustomIconsCheckBox->setChecked(m_settings.useCustomIcons);
    m_useCustomIconsCheckBox->setVisible(false); // Hidden by default, icon set combo handles this now
    connect(m_useCustomIconsCheckBox, &QCheckBox::toggled, this, &PieceIconSettingsDialog::onUseCustomIconsToggled);
    mainLayout->addWidget(m_useCustomIconsCheckBox);
    
    // Scroll area for piece settings
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    
    // White pieces group
    QGroupBox* whiteGroup = new QGroupBox("白方棋子", this);
    QGridLayout* whiteGridLayout = new QGridLayout(whiteGroup);
    int whiteRow = 0;
    
    createPieceRow(whiteGridLayout, whiteRow, "國王 (♔)", m_whiteKingEdit, 
                   m_whiteKingBrowseButton, m_whiteKingPreviewButton, m_whiteKingResetButton,
                   &PieceIconSettingsDialog::onBrowseWhiteKing,
                   &PieceIconSettingsDialog::onPreviewWhiteKing,
                   &PieceIconSettingsDialog::onResetWhiteKing);
    
    createPieceRow(whiteGridLayout, whiteRow, "皇后 (♕)", m_whiteQueenEdit,
                   m_whiteQueenBrowseButton, m_whiteQueenPreviewButton, m_whiteQueenResetButton,
                   &PieceIconSettingsDialog::onBrowseWhiteQueen,
                   &PieceIconSettingsDialog::onPreviewWhiteQueen,
                   &PieceIconSettingsDialog::onResetWhiteQueen);
    
    createPieceRow(whiteGridLayout, whiteRow, "城堡 (♖)", m_whiteRookEdit,
                   m_whiteRookBrowseButton, m_whiteRookPreviewButton, m_whiteRookResetButton,
                   &PieceIconSettingsDialog::onBrowseWhiteRook,
                   &PieceIconSettingsDialog::onPreviewWhiteRook,
                   &PieceIconSettingsDialog::onResetWhiteRook);
    
    createPieceRow(whiteGridLayout, whiteRow, "主教 (♗)", m_whiteBishopEdit,
                   m_whiteBishopBrowseButton, m_whiteBishopPreviewButton, m_whiteBishopResetButton,
                   &PieceIconSettingsDialog::onBrowseWhiteBishop,
                   &PieceIconSettingsDialog::onPreviewWhiteBishop,
                   &PieceIconSettingsDialog::onResetWhiteBishop);
    
    createPieceRow(whiteGridLayout, whiteRow, "騎士 (♘)", m_whiteKnightEdit,
                   m_whiteKnightBrowseButton, m_whiteKnightPreviewButton, m_whiteKnightResetButton,
                   &PieceIconSettingsDialog::onBrowseWhiteKnight,
                   &PieceIconSettingsDialog::onPreviewWhiteKnight,
                   &PieceIconSettingsDialog::onResetWhiteKnight);
    
    createPieceRow(whiteGridLayout, whiteRow, "兵 (♙)", m_whitePawnEdit,
                   m_whitePawnBrowseButton, m_whitePawnPreviewButton, m_whitePawnResetButton,
                   &PieceIconSettingsDialog::onBrowseWhitePawn,
                   &PieceIconSettingsDialog::onPreviewWhitePawn,
                   &PieceIconSettingsDialog::onResetWhitePawn);
    
    scrollLayout->addWidget(whiteGroup);
    
    // Black pieces group
    QGroupBox* blackGroup = new QGroupBox("黑方棋子", this);
    QGridLayout* blackGridLayout = new QGridLayout(blackGroup);
    int blackRow = 0;
    
    createPieceRow(blackGridLayout, blackRow, "國王 (♚)", m_blackKingEdit,
                   m_blackKingBrowseButton, m_blackKingPreviewButton, m_blackKingResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackKing,
                   &PieceIconSettingsDialog::onPreviewBlackKing,
                   &PieceIconSettingsDialog::onResetBlackKing);
    
    createPieceRow(blackGridLayout, blackRow, "皇后 (♛)", m_blackQueenEdit,
                   m_blackQueenBrowseButton, m_blackQueenPreviewButton, m_blackQueenResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackQueen,
                   &PieceIconSettingsDialog::onPreviewBlackQueen,
                   &PieceIconSettingsDialog::onResetBlackQueen);
    
    createPieceRow(blackGridLayout, blackRow, "城堡 (♜)", m_blackRookEdit,
                   m_blackRookBrowseButton, m_blackRookPreviewButton, m_blackRookResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackRook,
                   &PieceIconSettingsDialog::onPreviewBlackRook,
                   &PieceIconSettingsDialog::onResetBlackRook);
    
    createPieceRow(blackGridLayout, blackRow, "主教 (♝)", m_blackBishopEdit,
                   m_blackBishopBrowseButton, m_blackBishopPreviewButton, m_blackBishopResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackBishop,
                   &PieceIconSettingsDialog::onPreviewBlackBishop,
                   &PieceIconSettingsDialog::onResetBlackBishop);
    
    createPieceRow(blackGridLayout, blackRow, "騎士 (♞)", m_blackKnightEdit,
                   m_blackKnightBrowseButton, m_blackKnightPreviewButton, m_blackKnightResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackKnight,
                   &PieceIconSettingsDialog::onPreviewBlackKnight,
                   &PieceIconSettingsDialog::onResetBlackKnight);
    
    createPieceRow(blackGridLayout, blackRow, "兵 (♟)", m_blackPawnEdit,
                   m_blackPawnBrowseButton, m_blackPawnPreviewButton, m_blackPawnResetButton,
                   &PieceIconSettingsDialog::onBrowseBlackPawn,
                   &PieceIconSettingsDialog::onPreviewBlackPawn,
                   &PieceIconSettingsDialog::onResetBlackPawn);
    
    scrollLayout->addWidget(blackGroup);
    scrollLayout->addStretch();
    
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* resetAllButton = new QPushButton("重設為預設值", this);
    connect(resetAllButton, &QPushButton::clicked, this, &PieceIconSettingsDialog::onResetToDefaults);
    buttonLayout->addWidget(resetAllButton);
    
    buttonLayout->addStretch();
    
    QPushButton* okButton = new QPushButton("確定", this);
    connect(okButton, &QPushButton::clicked, this, &PieceIconSettingsDialog::onAccept);
    buttonLayout->addWidget(okButton);
    
    QPushButton* cancelButton = new QPushButton("取消", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Set default selection to Unicode
    QAbstractButton* defaultButton = m_iconSetButtonGroup->button(static_cast<int>(IconSetType::Unicode));
    if (defaultButton) {
        defaultButton->setChecked(true);
    }
}

void PieceIconSettingsDialog::createPieceRow(QGridLayout* gridLayout, int& row, const QString& label,
                                            QLineEdit*& iconEdit,
                                            QPushButton*& browseButton, QPushButton*& previewButton,
                                            QPushButton*& resetButton,
                                            void (PieceIconSettingsDialog::*browseSlot)(),
                                            void (PieceIconSettingsDialog::*previewSlot)(),
                                            void (PieceIconSettingsDialog::*resetSlot)())
{
    QLabel* nameLabel = new QLabel(label, this);
    gridLayout->addWidget(nameLabel, row, 0);
    
    iconEdit = new QLineEdit(this);
    iconEdit->setReadOnly(true);
    gridLayout->addWidget(iconEdit, row, 1);
    
    browseButton = new QPushButton("瀏覽", this);
    connect(browseButton, &QPushButton::clicked, this, browseSlot);
    gridLayout->addWidget(browseButton, row, 2);
    
    previewButton = new QPushButton("預覽", this);
    connect(previewButton, &QPushButton::clicked, this, previewSlot);
    gridLayout->addWidget(previewButton, row, 3);
    
    resetButton = new QPushButton("重設", this);
    connect(resetButton, &QPushButton::clicked, this, resetSlot);
    gridLayout->addWidget(resetButton, row, 4);
    
    row++;
}

QString PieceIconSettingsDialog::browseForIconFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "選擇圖標檔案",
        QString(),
        "圖像檔案 (*.png *.jpg *.jpeg *.svg *.bmp);;所有檔案 (*.*)"
    );
    return fileName;
}

void PieceIconSettingsDialog::previewIcon(const QString& iconFile)
{
    if (iconFile.isEmpty()) {
        QMessageBox::information(this, "預覽", "未設定圖標");
        return;
    }
    
    QPixmap pixmap(iconFile);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "預覽", "無法載入圖標檔案");
        return;
    }
    
    QDialog previewDialog(this);
    previewDialog.setWindowTitle("圖標預覽");
    QVBoxLayout* layout = new QVBoxLayout(&previewDialog);
    
    QLabel* imageLabel = new QLabel(&previewDialog);
    imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(imageLabel);
    
    QPushButton* closeButton = new QPushButton("關閉", &previewDialog);
    connect(closeButton, &QPushButton::clicked, &previewDialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    previewDialog.exec();
}

// Browse slots
void PieceIconSettingsDialog::onBrowseWhiteKing() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whiteKingEdit->setText(file);
        m_settings.whiteKingIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseWhiteQueen() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whiteQueenEdit->setText(file);
        m_settings.whiteQueenIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseWhiteRook() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whiteRookEdit->setText(file);
        m_settings.whiteRookIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseWhiteBishop() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whiteBishopEdit->setText(file);
        m_settings.whiteBishopIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseWhiteKnight() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whiteKnightEdit->setText(file);
        m_settings.whiteKnightIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseWhitePawn() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_whitePawnEdit->setText(file);
        m_settings.whitePawnIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackKing() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackKingEdit->setText(file);
        m_settings.blackKingIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackQueen() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackQueenEdit->setText(file);
        m_settings.blackQueenIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackRook() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackRookEdit->setText(file);
        m_settings.blackRookIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackBishop() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackBishopEdit->setText(file);
        m_settings.blackBishopIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackKnight() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackKnightEdit->setText(file);
        m_settings.blackKnightIcon = file;
    }
}

void PieceIconSettingsDialog::onBrowseBlackPawn() {
    QString file = browseForIconFile();
    if (!file.isEmpty()) {
        m_blackPawnEdit->setText(file);
        m_settings.blackPawnIcon = file;
    }
}

// Preview slots
void PieceIconSettingsDialog::onPreviewWhiteKing() { previewIcon(m_settings.whiteKingIcon); }
void PieceIconSettingsDialog::onPreviewWhiteQueen() { previewIcon(m_settings.whiteQueenIcon); }
void PieceIconSettingsDialog::onPreviewWhiteRook() { previewIcon(m_settings.whiteRookIcon); }
void PieceIconSettingsDialog::onPreviewWhiteBishop() { previewIcon(m_settings.whiteBishopIcon); }
void PieceIconSettingsDialog::onPreviewWhiteKnight() { previewIcon(m_settings.whiteKnightIcon); }
void PieceIconSettingsDialog::onPreviewWhitePawn() { previewIcon(m_settings.whitePawnIcon); }
void PieceIconSettingsDialog::onPreviewBlackKing() { previewIcon(m_settings.blackKingIcon); }
void PieceIconSettingsDialog::onPreviewBlackQueen() { previewIcon(m_settings.blackQueenIcon); }
void PieceIconSettingsDialog::onPreviewBlackRook() { previewIcon(m_settings.blackRookIcon); }
void PieceIconSettingsDialog::onPreviewBlackBishop() { previewIcon(m_settings.blackBishopIcon); }
void PieceIconSettingsDialog::onPreviewBlackKnight() { previewIcon(m_settings.blackKnightIcon); }
void PieceIconSettingsDialog::onPreviewBlackPawn() { previewIcon(m_settings.blackPawnIcon); }

// Reset slots
void PieceIconSettingsDialog::onResetWhiteKing() { m_whiteKingEdit->clear(); m_settings.whiteKingIcon.clear(); }
void PieceIconSettingsDialog::onResetWhiteQueen() { m_whiteQueenEdit->clear(); m_settings.whiteQueenIcon.clear(); }
void PieceIconSettingsDialog::onResetWhiteRook() { m_whiteRookEdit->clear(); m_settings.whiteRookIcon.clear(); }
void PieceIconSettingsDialog::onResetWhiteBishop() { m_whiteBishopEdit->clear(); m_settings.whiteBishopIcon.clear(); }
void PieceIconSettingsDialog::onResetWhiteKnight() { m_whiteKnightEdit->clear(); m_settings.whiteKnightIcon.clear(); }
void PieceIconSettingsDialog::onResetWhitePawn() { m_whitePawnEdit->clear(); m_settings.whitePawnIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackKing() { m_blackKingEdit->clear(); m_settings.blackKingIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackQueen() { m_blackQueenEdit->clear(); m_settings.blackQueenIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackRook() { m_blackRookEdit->clear(); m_settings.blackRookIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackBishop() { m_blackBishopEdit->clear(); m_settings.blackBishopIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackKnight() { m_blackKnightEdit->clear(); m_settings.blackKnightIcon.clear(); }
void PieceIconSettingsDialog::onResetBlackPawn() { m_blackPawnEdit->clear(); m_settings.blackPawnIcon.clear(); }

void PieceIconSettingsDialog::onUseCustomIconsToggled(bool enabled)
{
    m_settings.useCustomIcons = enabled;
}

void PieceIconSettingsDialog::onResetToDefaults()
{
    m_settings = getDefaultSettings();
    setSettings(m_settings);
}

void PieceIconSettingsDialog::onAccept()
{
    accept();
}

PieceIconSettingsDialog::PieceIconSettings PieceIconSettingsDialog::getSettings() const
{
    return m_settings;
}

void PieceIconSettingsDialog::setSettings(const PieceIconSettings& settings)
{
    m_settings = settings;
    
    // Set icon set type in combo box
    int index = m_iconSetComboBox->findData(static_cast<int>(settings.iconSetType));
    if (index >= 0) {
        m_iconSetComboBox->setCurrentIndex(index);
    }
    
    // Set the radio button for the icon set
    QAbstractButton* button = m_iconSetButtonGroup->button(static_cast<int>(settings.iconSetType));
    if (button) {
        button->setChecked(true);
    }
    
    m_useCustomIconsCheckBox->setChecked(settings.useCustomIcons);
    
    m_whiteKingEdit->setText(settings.whiteKingIcon);
    m_whiteQueenEdit->setText(settings.whiteQueenIcon);
    m_whiteRookEdit->setText(settings.whiteRookIcon);
    m_whiteBishopEdit->setText(settings.whiteBishopIcon);
    m_whiteKnightEdit->setText(settings.whiteKnightIcon);
    m_whitePawnEdit->setText(settings.whitePawnIcon);
    
    m_blackKingEdit->setText(settings.blackKingIcon);
    m_blackQueenEdit->setText(settings.blackQueenIcon);
    m_blackRookEdit->setText(settings.blackRookIcon);
    m_blackBishopEdit->setText(settings.blackBishopIcon);
    m_blackKnightEdit->setText(settings.blackKnightIcon);
    m_blackPawnEdit->setText(settings.blackPawnIcon);
    
    updateCustomIconsControls();
}

PieceIconSettingsDialog::PieceIconSettings PieceIconSettingsDialog::getDefaultSettings()
{
    PieceIconSettings defaults;
    defaults.whiteKingIcon = "";
    defaults.whiteQueenIcon = "";
    defaults.whiteRookIcon = "";
    defaults.whiteBishopIcon = "";
    defaults.whiteKnightIcon = "";
    defaults.whitePawnIcon = "";
    defaults.blackKingIcon = "";
    defaults.blackQueenIcon = "";
    defaults.blackRookIcon = "";
    defaults.blackBishopIcon = "";
    defaults.blackKnightIcon = "";
    defaults.blackPawnIcon = "";
    defaults.useCustomIcons = false;
    defaults.iconSetType = IconSetType::Unicode;
    return defaults;
}

PieceIconSettingsDialog::PieceIconSettings PieceIconSettingsDialog::getPresetSettings(IconSetType setType)
{
    PieceIconSettings settings;
    
    if (setType == IconSetType::Unicode) {
        // Use default Unicode symbols
        settings = getDefaultSettings();
        settings.iconSetType = setType;  // Ensure iconSetType is preserved
    } else if (setType == IconSetType::Preset1 || setType == IconSetType::Preset2 || setType == IconSetType::Preset3) {
        settings.iconSetType = setType;
        settings.useCustomIcons = true;  // Enable custom icons for preset sets
        QString setDir = getSetDirectoryName(setType);
        QString basePath = ":/resources/icons/" + setDir + "/";
        settings.whiteKingIcon = basePath + "white_king.png";
        settings.whiteQueenIcon = basePath + "white_queen.png";
        settings.whiteRookIcon = basePath + "white_rook.png";
        settings.whiteBishopIcon = basePath + "white_biship.png";
        settings.whiteKnightIcon = basePath + "white_knight.png";
        settings.whitePawnIcon = basePath + "white_pawn.png";
        settings.blackKingIcon = basePath + "black_king.png";
        settings.blackQueenIcon = basePath + "black_queen.png";
        settings.blackRookIcon = basePath + "black_rook.png";
        settings.blackBishopIcon = basePath + "black_biship.png";
        settings.blackKnightIcon = basePath + "black_knight.png";
        settings.blackPawnIcon = basePath + "black_pawn.png";
    } else {
        // Custom - return empty settings structure (user will populate paths via browse)
        // All QString members are already empty by default initialization
        settings.iconSetType = setType;
        settings.useCustomIcons = true;
    }
    
    return settings;
}

void PieceIconSettingsDialog::onIconSetTypeChanged(int index)
{
    IconSetType selectedType = static_cast<IconSetType>(m_iconSetComboBox->itemData(index).toInt());
    m_settings.iconSetType = selectedType;
    
    if (selectedType == IconSetType::Custom) {
        // Enable custom icon editing
        m_settings.useCustomIcons = true;
    } else {
        // Apply preset or Unicode
        applyPresetIconSet(selectedType);
    }
    
    updateCustomIconsControls();
}

void PieceIconSettingsDialog::updateCustomIconsControls()
{
    bool enableCustomEditing = (m_settings.iconSetType == IconSetType::Custom);
    
    // Enable/disable all browse, preview, and reset buttons
    m_whiteKingBrowseButton->setEnabled(enableCustomEditing);
    m_whiteQueenBrowseButton->setEnabled(enableCustomEditing);
    m_whiteRookBrowseButton->setEnabled(enableCustomEditing);
    m_whiteBishopBrowseButton->setEnabled(enableCustomEditing);
    m_whiteKnightBrowseButton->setEnabled(enableCustomEditing);
    m_whitePawnBrowseButton->setEnabled(enableCustomEditing);
    
    m_blackKingBrowseButton->setEnabled(enableCustomEditing);
    m_blackQueenBrowseButton->setEnabled(enableCustomEditing);
    m_blackRookBrowseButton->setEnabled(enableCustomEditing);
    m_blackBishopBrowseButton->setEnabled(enableCustomEditing);
    m_blackKnightBrowseButton->setEnabled(enableCustomEditing);
    m_blackPawnBrowseButton->setEnabled(enableCustomEditing);
    
    m_whiteKingResetButton->setEnabled(enableCustomEditing);
    m_whiteQueenResetButton->setEnabled(enableCustomEditing);
    m_whiteRookResetButton->setEnabled(enableCustomEditing);
    m_whiteBishopResetButton->setEnabled(enableCustomEditing);
    m_whiteKnightResetButton->setEnabled(enableCustomEditing);
    m_whitePawnResetButton->setEnabled(enableCustomEditing);
    
    m_blackKingResetButton->setEnabled(enableCustomEditing);
    m_blackQueenResetButton->setEnabled(enableCustomEditing);
    m_blackRookResetButton->setEnabled(enableCustomEditing);
    m_blackBishopResetButton->setEnabled(enableCustomEditing);
    m_blackKnightResetButton->setEnabled(enableCustomEditing);
    m_blackPawnResetButton->setEnabled(enableCustomEditing);
}

void PieceIconSettingsDialog::applyPresetIconSet(IconSetType setType)
{
    PieceIconSettings presetSettings = getPresetSettings(setType);
    
    m_settings.whiteKingIcon = presetSettings.whiteKingIcon;
    m_settings.whiteQueenIcon = presetSettings.whiteQueenIcon;
    m_settings.whiteRookIcon = presetSettings.whiteRookIcon;
    m_settings.whiteBishopIcon = presetSettings.whiteBishopIcon;
    m_settings.whiteKnightIcon = presetSettings.whiteKnightIcon;
    m_settings.whitePawnIcon = presetSettings.whitePawnIcon;
    m_settings.blackKingIcon = presetSettings.blackKingIcon;
    m_settings.blackQueenIcon = presetSettings.blackQueenIcon;
    m_settings.blackRookIcon = presetSettings.blackRookIcon;
    m_settings.blackBishopIcon = presetSettings.blackBishopIcon;
    m_settings.blackKnightIcon = presetSettings.blackKnightIcon;
    m_settings.blackPawnIcon = presetSettings.blackPawnIcon;
    m_settings.useCustomIcons = presetSettings.useCustomIcons;
    
    // Update UI to show the new paths
    m_whiteKingEdit->setText(m_settings.whiteKingIcon);
    m_whiteQueenEdit->setText(m_settings.whiteQueenIcon);
    m_whiteRookEdit->setText(m_settings.whiteRookIcon);
    m_whiteBishopEdit->setText(m_settings.whiteBishopIcon);
    m_whiteKnightEdit->setText(m_settings.whiteKnightIcon);
    m_whitePawnEdit->setText(m_settings.whitePawnIcon);
    m_blackKingEdit->setText(m_settings.blackKingIcon);
    m_blackQueenEdit->setText(m_settings.blackQueenIcon);
    m_blackRookEdit->setText(m_settings.blackRookIcon);
    m_blackBishopEdit->setText(m_settings.blackBishopIcon);
    m_blackKnightEdit->setText(m_settings.blackKnightIcon);
    m_blackPawnEdit->setText(m_settings.blackPawnIcon);
}

QString PieceIconSettingsDialog::getSetDirectoryName(IconSetType setType)
{
    switch (setType) {
        case IconSetType::Preset1: 
            return "set1";
        case IconSetType::Preset2: 
            return "set2";
        case IconSetType::Preset3: 
            return "set3";
        case IconSetType::Unicode:
        case IconSetType::Custom:
        default: 
            return "";
    }
}

QWidget* PieceIconSettingsDialog::createIconSetPreviewWidget(IconSetType setType, const QString& label)
{
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Create a frame with border
    QFrame* frame = new QFrame(widget);
    frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    frame->setLineWidth(2);
    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    
    // Radio button for selection
    QRadioButton* radioButton = new QRadioButton(label, frame);
    radioButton->setStyleSheet("QRadioButton { font-weight: bold; font-size: 11pt; }");
    m_iconSetButtonGroup->addButton(radioButton, static_cast<int>(setType));
    frameLayout->addWidget(radioButton);
    
    // Preview images layout
    QHBoxLayout* previewLayout = new QHBoxLayout();
    previewLayout->setSpacing(5);
    
    if (setType == IconSetType::Unicode) {
        // Show Unicode symbols
        QLabel* unicodeLabel = new QLabel("♔ ♕ ♖ ♗ ♘ ♙", frame);
        unicodeLabel->setStyleSheet("QLabel { font-size: 36pt; }");
        unicodeLabel->setAlignment(Qt::AlignCenter);
        previewLayout->addWidget(unicodeLabel);
    } else if (setType == IconSetType::Custom) {
        // Show text description for custom
        QLabel* customLabel = new QLabel("使用您自己的圖標檔案\n點擊下方「瀏覽」按鈕選擇", frame);
        customLabel->setStyleSheet("QLabel { font-size: 10pt; color: #666; }");
        customLabel->setAlignment(Qt::AlignCenter);
        customLabel->setWordWrap(true);
        previewLayout->addWidget(customLabel);
    } else {
        // Show preset icon previews
        PieceIconSettings presetSettings = getPresetSettings(setType);
        
        // Display a few key pieces as preview
        QStringList previewPieces;
        previewPieces << presetSettings.whiteKingIcon 
                      << presetSettings.whiteQueenIcon 
                      << presetSettings.whiteRookIcon
                      << presetSettings.blackKingIcon 
                      << presetSettings.blackQueenIcon 
                      << presetSettings.blackRookIcon;
        
        for (const QString& iconPath : previewPieces) {
            QLabel* iconLabel = new QLabel(frame);
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                iconLabel->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                iconLabel->setText("?");
                iconLabel->setStyleSheet("QLabel { font-size: 24pt; }");
            }
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setFixedSize(50, 50);
            previewLayout->addWidget(iconLabel);
        }
    }
    
    frameLayout->addLayout(previewLayout);
    layout->addWidget(frame);
    
    return widget;
}

void PieceIconSettingsDialog::onIconSetButtonClicked(int id)
{
    IconSetType selectedType = static_cast<IconSetType>(id);
    m_settings.iconSetType = selectedType;
    
    // Update the hidden combo box to stay in sync
    int comboIndex = m_iconSetComboBox->findData(static_cast<int>(selectedType));
    if (comboIndex >= 0) {
        m_iconSetComboBox->setCurrentIndex(comboIndex);
    }
    
    if (selectedType == IconSetType::Custom) {
        // Enable custom icon editing
        m_settings.useCustomIcons = true;
    } else {
        // Apply preset or Unicode
        applyPresetIconSet(selectedType);
    }
    
    updateCustomIconsControls();
}
