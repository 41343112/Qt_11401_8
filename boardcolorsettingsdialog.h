#ifndef BOARDCOLORSETTINGSDIALOG_H
#define BOARDCOLORSETTINGSDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QColor>
#include <QButtonGroup>
#include <QRadioButton>

class BoardColorSettingsDialog : public QDialog
{
    Q_OBJECT

    // UI Constants
    static constexpr int PREVIEW_BUTTON_WIDTH = 80;
    static constexpr int PREVIEW_BUTTON_HEIGHT = 95;
    static constexpr int PREVIEW_SQUARE_SIZE = 30;
    
public:
    explicit BoardColorSettingsDialog(QWidget *parent = nullptr);
    ~BoardColorSettingsDialog();

    enum class ColorScheme {
        Classic,      // Classic brown/beige (default)
        BlueGray,     // Blue and gray
        GreenWhite,   // Green and white
        Custom1,      // Custom user-selected colors slot 1
        Custom2,      // Custom user-selected colors slot 2
        Custom3,      // Custom user-selected colors slot 3
        Custom4,      // Custom user-selected colors slot 4
        Custom5,      // Custom user-selected colors slot 5
        Custom6,      // Custom user-selected colors slot 6
        Custom7       // Custom user-selected colors slot 7
    };

    struct BoardColorSettings {
        QColor lightSquareColor;
        QColor darkSquareColor;
        ColorScheme scheme;
        QString customName;  // Name for custom color schemes
    };

    BoardColorSettings getSettings() const;
    void setSettings(const BoardColorSettings& settings);
    
    static BoardColorSettings getDefaultSettings();
    static BoardColorSettings getPresetSettings(ColorScheme scheme);

private slots:
    void onLightColorClicked();
    void onDarkColorClicked();
    void onResetToDefaults();
    void onAccept();
    void onPresetPreviewClicked(ColorScheme scheme);

private:
    BoardColorSettings m_settings;
    
    QPushButton* m_lightColorButton;
    QPushButton* m_darkColorButton;
    QWidget* m_previewWidget;
    QLabel* m_previewSquares[2][2];  // 2x2 preview grid
    QWidget* m_customColorWidget;  // Widget containing custom color controls
    
    // Custom color slots storage
    BoardColorSettings m_customSlots[7];
    
    void setupUI();
    void updatePreview();
    void updateColorButtons();
    void applyPresetColorScheme(ColorScheme scheme);
    QPushButton* createPresetPreview(ColorScheme scheme, const QString& label);
    void loadCustomSlots();
    void saveCustomSlots();
    bool isCustomSlot(ColorScheme scheme) const;
};

#endif // BOARDCOLORSETTINGSDIALOG_H
