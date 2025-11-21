#ifndef BOARDCOLORSETTINGSDIALOG_H
#define BOARDCOLORSETTINGSDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QColor>
#include <QComboBox>
#include <QButtonGroup>
#include <QRadioButton>

class BoardColorSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BoardColorSettingsDialog(QWidget *parent = nullptr);
    ~BoardColorSettingsDialog();

    enum class ColorScheme {
        Classic,      // Classic brown/beige (default)
        BlueGray,     // Blue and gray
        GreenWhite,   // Green and white
        Custom        // Custom user-selected colors
    };

    struct BoardColorSettings {
        QColor lightSquareColor;
        QColor darkSquareColor;
        ColorScheme scheme;
    };

    BoardColorSettings getSettings() const;
    void setSettings(const BoardColorSettings& settings);
    
    static BoardColorSettings getDefaultSettings();
    static BoardColorSettings getPresetSettings(ColorScheme scheme);

private slots:
    void onColorSchemeChanged(int index);
    void onLightColorClicked();
    void onDarkColorClicked();
    void onResetToDefaults();
    void onAccept();

private:
    BoardColorSettings m_settings;
    
    QComboBox* m_colorSchemeComboBox;
    QPushButton* m_lightColorButton;
    QPushButton* m_darkColorButton;
    QWidget* m_previewWidget;
    QLabel* m_previewSquares[2][2];  // 2x2 preview grid
    
    void setupUI();
    void updatePreview();
    void updateColorButtons();
    void applyPresetColorScheme(ColorScheme scheme);
    QWidget* createPresetPreview(ColorScheme scheme, const QString& label);
};

#endif // BOARDCOLORSETTINGSDIALOG_H
