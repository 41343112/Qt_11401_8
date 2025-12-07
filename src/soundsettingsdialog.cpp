#include "soundsettingsdialog.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

SoundSettingsDialog::SoundSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("🔊 音效設定");
    setModal(true);
    setMinimumWidth(600);
    
    // 應用現代科技風格
    applyModernDialogStyle();
    
    setupUI();
    
    // 載入已儲存的設定或使用預設值
    SoundSettings defaults = getDefaultSettings();
    QSettings settings("QtChess", "SoundSettings");
    m_settings.moveSound = settings.value("moveSound", defaults.moveSound).toString();
    m_settings.captureSound = settings.value("captureSound", defaults.captureSound).toString();
    m_settings.castlingSound = settings.value("castlingSound", defaults.castlingSound).toString();
    m_settings.checkSound = settings.value("checkSound", defaults.checkSound).toString();
    m_settings.checkmateSound = settings.value("checkmateSound", defaults.checkmateSound).toString();
    
    m_settings.moveVolume = settings.value("moveVolume", defaults.moveVolume).toDouble();
    m_settings.captureVolume = settings.value("captureVolume", defaults.captureVolume).toDouble();
    m_settings.castlingVolume = settings.value("castlingVolume", defaults.castlingVolume).toDouble();
    m_settings.checkVolume = settings.value("checkVolume", defaults.checkVolume).toDouble();
    m_settings.checkmateVolume = settings.value("checkmateVolume", defaults.checkmateVolume).toDouble();
    
    m_settings.moveSoundEnabled = settings.value("moveSoundEnabled", defaults.moveSoundEnabled).toBool();
    m_settings.captureSoundEnabled = settings.value("captureSoundEnabled", defaults.captureSoundEnabled).toBool();
    m_settings.castlingSoundEnabled = settings.value("castlingSoundEnabled", defaults.castlingSoundEnabled).toBool();
    m_settings.checkSoundEnabled = settings.value("checkSoundEnabled", defaults.checkSoundEnabled).toBool();
    m_settings.checkmateSoundEnabled = settings.value("checkmateSoundEnabled", defaults.checkmateSoundEnabled).toBool();
    m_settings.allSoundsEnabled = settings.value("allSoundsEnabled", defaults.allSoundsEnabled).toBool();
    
    setSettings(m_settings);
}

SoundSettingsDialog::~SoundSettingsDialog()
{
}

void SoundSettingsDialog::createSoundRow(QGridLayout* gridLayout, int& row, const QString& label,
                                         QLineEdit*& soundEdit, QCheckBox*& soundCheckBox,
                                         QSlider*& volumeSlider, QLabel*& volumeLabel,
                                         QPushButton*& browseButton, QPushButton*& previewButton,
                                         QPushButton*& resetButton,
                                         void (SoundSettingsDialog::*browseSlot)(),
                                         void (SoundSettingsDialog::*previewSlot)(),
                                         void (SoundSettingsDialog::*resetSlot)())
{
    // 標籤
    QLabel* nameLabel = new QLabel(label, this);
    gridLayout->addWidget(nameLabel, row, 0);
    
    // 啟用核取方塊
    soundCheckBox = new QCheckBox("啟用", this);
    soundCheckBox->setChecked(true);
    gridLayout->addWidget(soundCheckBox, row, 1);
    
    // 檔案路徑編輯
    soundEdit = new QLineEdit(this);
    soundEdit->setReadOnly(true);
    gridLayout->addWidget(soundEdit, row, 2);
    
    // 瀏覽按鈕
    browseButton = new QPushButton("瀏覽...", this);
    connect(browseButton, &QPushButton::clicked, this, browseSlot);
    gridLayout->addWidget(browseButton, row, 3);
    
    // 音量標籤
    QLabel* volLabel = new QLabel("音量:", this);
    gridLayout->addWidget(volLabel, row, 4);
    
    // 音量滑桿
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setFixedWidth(100);
    gridLayout->addWidget(volumeSlider, row, 5);
    
    // 音量百分比標籤
    volumeLabel = new QLabel("50%", this);
    volumeLabel->setFixedWidth(40);
    gridLayout->addWidget(volumeLabel, row, 6);
    
    // 連接滑桿以更新標籤
    connect(volumeSlider, &QSlider::valueChanged, [volumeLabel](int value) {
        volumeLabel->setText(QString("%1%").arg(value));
    });
    
    // 預覽按鈕
    previewButton = new QPushButton("預覽", this);
    connect(previewButton, &QPushButton::clicked, this, previewSlot);
    gridLayout->addWidget(previewButton, row, 7);
    
    // 重置按鈕
    resetButton = new QPushButton("重設", this);
    connect(resetButton, &QPushButton::clicked, this, resetSlot);
    gridLayout->addWidget(resetButton, row, 8);
    
    row++;
}

void SoundSettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 添加關於支援格式的資訊標籤
    QLabel* infoLabel = new QLabel(
        "支援的音檔格式：WAV (推薦)、MP3、OGG\n"
        "注意：WAV 格式具有最佳的相容性，建議使用 WAV 格式的音效檔案。", 
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #555; font-size: 10pt; padding: 5px; background-color: #f0f0f0; border-radius: 3px; }");
    mainLayout->addWidget(infoLabel);
    
    // 主啟用/禁用核取方塊
    m_allSoundsCheckBox = new QCheckBox("啟用所有音效", this);
    m_allSoundsCheckBox->setChecked(true);
    connect(m_allSoundsCheckBox, &QCheckBox::toggled, this, &SoundSettingsDialog::onAllSoundsToggled);
    mainLayout->addWidget(m_allSoundsCheckBox);
    
    // 為所有音效設定創建群組框
    QGroupBox* soundsGroupBox = new QGroupBox("音效設定", this);
    QGridLayout* gridLayout = new QGridLayout(soundsGroupBox);
    
    int row = 0;
    
    // 為每種音效類型創建行
    createSoundRow(gridLayout, row, "移動音效:", m_moveSoundEdit, m_moveSoundCheckBox, 
                   m_moveVolumeSlider, m_moveVolumeLabel, m_moveBrowseButton, m_movePreviewButton,
                   m_moveResetButton,
                   &SoundSettingsDialog::onBrowseMove, &SoundSettingsDialog::onPreviewMove,
                   &SoundSettingsDialog::onResetMove);
    
    createSoundRow(gridLayout, row, "吃子音效:", m_captureSoundEdit, m_captureSoundCheckBox,
                   m_captureVolumeSlider, m_captureVolumeLabel, m_captureBrowseButton, m_capturePreviewButton,
                   m_captureResetButton,
                   &SoundSettingsDialog::onBrowseCapture, &SoundSettingsDialog::onPreviewCapture,
                   &SoundSettingsDialog::onResetCapture);
    
    createSoundRow(gridLayout, row, "王車易位:", m_castlingSoundEdit, m_castlingSoundCheckBox,
                   m_castlingVolumeSlider, m_castlingVolumeLabel, m_castlingBrowseButton, m_castlingPreviewButton,
                   m_castlingResetButton,
                   &SoundSettingsDialog::onBrowseCastling, &SoundSettingsDialog::onPreviewCastling,
                   &SoundSettingsDialog::onResetCastling);
    
    createSoundRow(gridLayout, row, "將軍音效:", m_checkSoundEdit, m_checkSoundCheckBox,
                   m_checkVolumeSlider, m_checkVolumeLabel, m_checkBrowseButton, m_checkPreviewButton,
                   m_checkResetButton,
                   &SoundSettingsDialog::onBrowseCheck, &SoundSettingsDialog::onPreviewCheck,
                   &SoundSettingsDialog::onResetCheck);
    
    createSoundRow(gridLayout, row, "將死音效:", m_checkmateSoundEdit, m_checkmateSoundCheckBox,
                   m_checkmateVolumeSlider, m_checkmateVolumeLabel, m_checkmateBrowseButton, m_checkmatePreviewButton,
                   m_checkmateResetButton,
                   &SoundSettingsDialog::onBrowseCheckmate, &SoundSettingsDialog::onPreviewCheckmate,
                   &SoundSettingsDialog::onResetCheckmate);
    
    mainLayout->addWidget(soundsGroupBox);
    
    // 底部的按鈕
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* resetButton = new QPushButton("重設為預設值", this);
    connect(resetButton, &QPushButton::clicked, this, &SoundSettingsDialog::onResetToDefaults);
    buttonLayout->addWidget(resetButton);
    
    QPushButton* okButton = new QPushButton("確定", this);
    connect(okButton, &QPushButton::clicked, this, &SoundSettingsDialog::onAccept);
    buttonLayout->addWidget(okButton);
    
    QPushButton* cancelButton = new QPushButton("取消", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

SoundSettingsDialog::SoundSettings SoundSettingsDialog::getSettings() const
{
    return m_settings;
}

void SoundSettingsDialog::setSettings(const SoundSettings& settings)
{
    m_settings = settings;
    
    m_allSoundsCheckBox->setChecked(m_settings.allSoundsEnabled);
    
    m_moveSoundEdit->setText(m_settings.moveSound);
    m_moveSoundCheckBox->setChecked(m_settings.moveSoundEnabled);
    m_moveVolumeSlider->setValue(static_cast<int>(m_settings.moveVolume * 100));
    
    m_captureSoundEdit->setText(m_settings.captureSound);
    m_captureSoundCheckBox->setChecked(m_settings.captureSoundEnabled);
    m_captureVolumeSlider->setValue(static_cast<int>(m_settings.captureVolume * 100));
    
    m_castlingSoundEdit->setText(m_settings.castlingSound);
    m_castlingSoundCheckBox->setChecked(m_settings.castlingSoundEnabled);
    m_castlingVolumeSlider->setValue(static_cast<int>(m_settings.castlingVolume * 100));
    
    m_checkSoundEdit->setText(m_settings.checkSound);
    m_checkSoundCheckBox->setChecked(m_settings.checkSoundEnabled);
    m_checkVolumeSlider->setValue(static_cast<int>(m_settings.checkVolume * 100));
    
    m_checkmateSoundEdit->setText(m_settings.checkmateSound);
    m_checkmateSoundCheckBox->setChecked(m_settings.checkmateSoundEnabled);
    m_checkmateVolumeSlider->setValue(static_cast<int>(m_settings.checkmateVolume * 100));
}

void SoundSettingsDialog::onBrowseMove()
{
    QString file = browseForSoundFile();
    if (!file.isEmpty()) {
        m_moveSoundEdit->setText(file);
    }
}

void SoundSettingsDialog::onBrowseCapture()
{
    QString file = browseForSoundFile();
    if (!file.isEmpty()) {
        m_captureSoundEdit->setText(file);
    }
}

void SoundSettingsDialog::onBrowseCastling()
{
    QString file = browseForSoundFile();
    if (!file.isEmpty()) {
        m_castlingSoundEdit->setText(file);
    }
}

void SoundSettingsDialog::onBrowseCheck()
{
    QString file = browseForSoundFile();
    if (!file.isEmpty()) {
        m_checkSoundEdit->setText(file);
    }
}

void SoundSettingsDialog::onBrowseCheckmate()
{
    QString file = browseForSoundFile();
    if (!file.isEmpty()) {
        m_checkmateSoundEdit->setText(file);
    }
}

void SoundSettingsDialog::onPreviewMove()
{
    previewSound(m_moveSoundEdit->text(), m_moveVolumeSlider->value() / 100.0);
}

void SoundSettingsDialog::onPreviewCapture()
{
    previewSound(m_captureSoundEdit->text(), m_captureVolumeSlider->value() / 100.0);
}

void SoundSettingsDialog::onPreviewCastling()
{
    previewSound(m_castlingSoundEdit->text(), m_castlingVolumeSlider->value() / 100.0);
}

void SoundSettingsDialog::onPreviewCheck()
{
    previewSound(m_checkSoundEdit->text(), m_checkVolumeSlider->value() / 100.0);
}

void SoundSettingsDialog::onPreviewCheckmate()
{
    previewSound(m_checkmateSoundEdit->text(), m_checkmateVolumeSlider->value() / 100.0);
}

void SoundSettingsDialog::onAllSoundsToggled(bool enabled)
{
    m_moveSoundCheckBox->setEnabled(enabled);
    m_captureSoundCheckBox->setEnabled(enabled);
    m_castlingSoundCheckBox->setEnabled(enabled);
    m_checkSoundCheckBox->setEnabled(enabled);
    m_checkmateSoundCheckBox->setEnabled(enabled);
}

void SoundSettingsDialog::onResetToDefaults()
{
    m_settings = getDefaultSettings();
    setSettings(m_settings);
}

void SoundSettingsDialog::onAccept()
{
    // 從 UI 收集設定
    m_settings.allSoundsEnabled = m_allSoundsCheckBox->isChecked();
    
    m_settings.moveSound = m_moveSoundEdit->text();
    m_settings.moveSoundEnabled = m_moveSoundCheckBox->isChecked();
    m_settings.moveVolume = m_moveVolumeSlider->value() / 100.0;
    
    m_settings.captureSound = m_captureSoundEdit->text();
    m_settings.captureSoundEnabled = m_captureSoundCheckBox->isChecked();
    m_settings.captureVolume = m_captureVolumeSlider->value() / 100.0;
    
    m_settings.castlingSound = m_castlingSoundEdit->text();
    m_settings.castlingSoundEnabled = m_castlingSoundCheckBox->isChecked();
    m_settings.castlingVolume = m_castlingVolumeSlider->value() / 100.0;
    
    m_settings.checkSound = m_checkSoundEdit->text();
    m_settings.checkSoundEnabled = m_checkSoundCheckBox->isChecked();
    m_settings.checkVolume = m_checkVolumeSlider->value() / 100.0;
    
    m_settings.checkmateSound = m_checkmateSoundEdit->text();
    m_settings.checkmateSoundEnabled = m_checkmateSoundCheckBox->isChecked();
    m_settings.checkmateVolume = m_checkmateVolumeSlider->value() / 100.0;
    
    // 儲存設定
    QSettings settings("QtChess", "SoundSettings");
    settings.setValue("moveSound", m_settings.moveSound);
    settings.setValue("captureSound", m_settings.captureSound);
    settings.setValue("castlingSound", m_settings.castlingSound);
    settings.setValue("checkSound", m_settings.checkSound);
    settings.setValue("checkmateSound", m_settings.checkmateSound);
    
    settings.setValue("moveVolume", m_settings.moveVolume);
    settings.setValue("captureVolume", m_settings.captureVolume);
    settings.setValue("castlingVolume", m_settings.castlingVolume);
    settings.setValue("checkVolume", m_settings.checkVolume);
    settings.setValue("checkmateVolume", m_settings.checkmateVolume);
    
    settings.setValue("moveSoundEnabled", m_settings.moveSoundEnabled);
    settings.setValue("captureSoundEnabled", m_settings.captureSoundEnabled);
    settings.setValue("castlingSoundEnabled", m_settings.castlingSoundEnabled);
    settings.setValue("checkSoundEnabled", m_settings.checkSoundEnabled);
    settings.setValue("checkmateSoundEnabled", m_settings.checkmateSoundEnabled);
    settings.setValue("allSoundsEnabled", m_settings.allSoundsEnabled);
    
    accept();
}

QString SoundSettingsDialog::browseForSoundFile()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        "選擇音效檔案",
        QString(),
        "WAV 音效檔案 (*.wav);;所有音效檔案 (*.wav *.mp3 *.ogg);;所有檔案 (*.*)"
    );
    return file;
}

void SoundSettingsDialog::previewSound(const QString& soundFile, double volume)
{
    if (soundFile.isEmpty()) {
        QMessageBox::warning(this, "錯誤", "未選擇音效檔案");
        return;
    }
    
    m_previewSound.stop();
    
    // 處理 qrc: URL 和檔案路徑
    if (soundFile.startsWith("qrc:")) {
        m_previewSound.setSource(QUrl(soundFile));
    } else {
        m_previewSound.setSource(QUrl::fromLocalFile(soundFile));
    }
    
    m_previewSound.setVolume(volume);
    m_previewSound.play();
}

SoundSettingsDialog::SoundSettings SoundSettingsDialog::getDefaultSettings()
{
    SoundSettings defaults;
    
    defaults.moveSound = "qrc:/resources/sounds/move.wav";
    defaults.captureSound = "qrc:/resources/sounds/capture.wav";
    defaults.castlingSound = "qrc:/resources/sounds/castling.wav";
    defaults.checkSound = "qrc:/resources/sounds/check.wav";
    defaults.checkmateSound = "qrc:/resources/sounds/checkmate.wav";
    
    defaults.moveVolume = 0.5;
    defaults.captureVolume = 0.5;
    defaults.castlingVolume = 0.5;
    defaults.checkVolume = 0.5;
    defaults.checkmateVolume = 0.6;
    
    defaults.moveSoundEnabled = true;
    defaults.captureSoundEnabled = true;
    defaults.castlingSoundEnabled = true;
    defaults.checkSoundEnabled = true;
    defaults.checkmateSoundEnabled = true;
    defaults.allSoundsEnabled = true;
    
    return defaults;
}

void SoundSettingsDialog::applyModernDialogStyle()
{
    // 現代科技風格對話框樣式表
    QString styleSheet = QString(
        "QDialog { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 %1, stop:0.5 %2, stop:1 %1); "
        "}"
        "QGroupBox { "
        "  font-weight: bold; "
        "  color: %3; "
        "  border: 2px solid %4; "
        "  border-radius: 10px; "
        "  margin-top: 12px; "
        "  padding-top: 10px; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 rgba(15, 41, 64, 0.9), stop:1 rgba(10, 22, 40, 0.9)); "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  subcontrol-position: top left; "
        "  padding: 4px 12px; "
        "  color: %3; "
        "  background: %5; "
        "  border: 1px solid %3; "
        "  border-radius: 6px; "
        "  left: 10px; "
        "}"
        "QLabel { color: %6; }"
        "QCheckBox { color: %6; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QCheckBox::indicator:unchecked { "
        "  border: 2px solid %4; border-radius: 4px; background: %1; "
        "}"
        "QCheckBox::indicator:checked { "
        "  border: 2px solid %3; border-radius: 4px; "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %3, stop:1 %7); "
        "}"
        "QLineEdit { "
        "  background: %1; color: %6; border: 2px solid %4; "
        "  border-radius: 6px; padding: 4px 8px; "
        "}"
        "QLineEdit:focus { border-color: %3; }"
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %5, stop:1 %1); "
        "  color: %6; border: 2px solid %4; border-radius: 8px; "
        "  padding: 6px 12px; font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "  border-color: %3; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %5, stop:0.5 rgba(0, 255, 255, 0.3), stop:1 %1); "
        "}"
        "QPushButton:pressed { background: %3; color: %1; }"
        "QSlider::groove:horizontal { "
        "  border: 1px solid %4; height: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %5); "
        "  border-radius: 4px; "
        "}"
        "QSlider::handle:horizontal { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %3, stop:1 rgba(0, 255, 255, 0.7)); "
        "  border: 2px solid %3; width: 16px; margin: -5px 0; border-radius: 8px; "
        "}"
        "QSlider::sub-page:horizontal { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %3, stop:1 %7); "
        "  border-radius: 4px; "
        "}"
    ).arg(Theme::BG_DARK, Theme::BG_MEDIUM, Theme::ACCENT_PRIMARY, 
          Theme::BORDER, Theme::BG_PANEL, Theme::TEXT_PRIMARY, 
          Theme::ACCENT_SECONDARY);
    
    setStyleSheet(styleSheet);
}

void SoundSettingsDialog::onResetMove()
{
    SoundSettings defaults = getDefaultSettings();
    m_moveSoundEdit->setText(defaults.moveSound);
    m_moveSoundCheckBox->setChecked(defaults.moveSoundEnabled);
    m_moveVolumeSlider->setValue(static_cast<int>(defaults.moveVolume * 100));
}

void SoundSettingsDialog::onResetCapture()
{
    SoundSettings defaults = getDefaultSettings();
    m_captureSoundEdit->setText(defaults.captureSound);
    m_captureSoundCheckBox->setChecked(defaults.captureSoundEnabled);
    m_captureVolumeSlider->setValue(static_cast<int>(defaults.captureVolume * 100));
}

void SoundSettingsDialog::onResetCastling()
{
    SoundSettings defaults = getDefaultSettings();
    m_castlingSoundEdit->setText(defaults.castlingSound);
    m_castlingSoundCheckBox->setChecked(defaults.castlingSoundEnabled);
    m_castlingVolumeSlider->setValue(static_cast<int>(defaults.castlingVolume * 100));
}

void SoundSettingsDialog::onResetCheck()
{
    SoundSettings defaults = getDefaultSettings();
    m_checkSoundEdit->setText(defaults.checkSound);
    m_checkSoundCheckBox->setChecked(defaults.checkSoundEnabled);
    m_checkVolumeSlider->setValue(static_cast<int>(defaults.checkVolume * 100));
}

void SoundSettingsDialog::onResetCheckmate()
{
    SoundSettings defaults = getDefaultSettings();
    m_checkmateSoundEdit->setText(defaults.checkmateSound);
    m_checkmateSoundCheckBox->setChecked(defaults.checkmateSoundEnabled);
    m_checkmateVolumeSlider->setValue(static_cast<int>(defaults.checkmateVolume * 100));
}
