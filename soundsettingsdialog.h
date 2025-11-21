#ifndef SOUNDSETTINGSDIALOG_H
#define SOUNDSETTINGSDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QSoundEffect>

class SoundSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SoundSettingsDialog(QWidget *parent = nullptr);
    ~SoundSettingsDialog();

    struct SoundSettings {
        QString moveSound;
        QString captureSound;
        QString castlingSound;
        QString checkSound;
        QString checkmateSound;
        double moveVolume;
        double captureVolume;
        double castlingVolume;
        double checkVolume;
        double checkmateVolume;
        bool moveSoundEnabled;
        bool captureSoundEnabled;
        bool castlingSoundEnabled;
        bool checkSoundEnabled;
        bool checkmateSoundEnabled;
        bool allSoundsEnabled;
    };

    SoundSettings getSettings() const;
    void setSettings(const SoundSettings& settings);
    
    static SoundSettings getDefaultSettings();

private slots:
    void onBrowseMove();
    void onBrowseCapture();
    void onBrowseCastling();
    void onBrowseCheck();
    void onBrowseCheckmate();
    void onPreviewMove();
    void onPreviewCapture();
    void onPreviewCastling();
    void onPreviewCheck();
    void onPreviewCheckmate();
    void onAllSoundsToggled(bool enabled);
    void onResetToDefaults();
    void onAccept();

private:
    SoundSettings m_settings;
    
    QCheckBox* m_allSoundsCheckBox;
    
    QLineEdit* m_moveSoundEdit;
    QCheckBox* m_moveSoundCheckBox;
    QSlider* m_moveVolumeSlider;
    QLabel* m_moveVolumeLabel;
    QPushButton* m_moveBrowseButton;
    QPushButton* m_movePreviewButton;
    
    QLineEdit* m_captureSoundEdit;
    QCheckBox* m_captureSoundCheckBox;
    QSlider* m_captureVolumeSlider;
    QLabel* m_captureVolumeLabel;
    QPushButton* m_captureBrowseButton;
    QPushButton* m_capturePreviewButton;
    
    QLineEdit* m_castlingSoundEdit;
    QCheckBox* m_castlingSoundCheckBox;
    QSlider* m_castlingVolumeSlider;
    QLabel* m_castlingVolumeLabel;
    QPushButton* m_castlingBrowseButton;
    QPushButton* m_castlingPreviewButton;
    
    QLineEdit* m_checkSoundEdit;
    QCheckBox* m_checkSoundCheckBox;
    QSlider* m_checkVolumeSlider;
    QLabel* m_checkVolumeLabel;
    QPushButton* m_checkBrowseButton;
    QPushButton* m_checkPreviewButton;
    
    QLineEdit* m_checkmateSoundEdit;
    QCheckBox* m_checkmateSoundCheckBox;
    QSlider* m_checkmateVolumeSlider;
    QLabel* m_checkmateVolumeLabel;
    QPushButton* m_checkmateBrowseButton;
    QPushButton* m_checkmatePreviewButton;
    
    QSoundEffect m_previewSound;
    
    struct SoundControlWidgets {
        QLineEdit* soundEdit;
        QCheckBox* soundCheckBox;
        QSlider* volumeSlider;
        QLabel* volumeLabel;
        QPushButton* browseButton;
        QPushButton* previewButton;
    };
    
    void setupUI();
    void createSoundRow(QGridLayout* gridLayout, int& row, const QString& label,
                       SoundControlWidgets& widgets,
                       void (SoundSettingsDialog::*browseSlot)(),
                       void (SoundSettingsDialog::*previewSlot)());
    void updateAllSoundsCheckBox();
    QString browseForSoundFile();
    void previewSound(const QString& soundFile, double volume);
};

#endif // SOUNDSETTINGSDIALOG_H
