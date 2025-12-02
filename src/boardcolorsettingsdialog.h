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

    // UI 常數
    static constexpr int PREVIEW_BUTTON_WIDTH = 80;
    static constexpr int PREVIEW_BUTTON_HEIGHT = 95;
    static constexpr int PREVIEW_SQUARE_SIZE = 30;
    
public:
    explicit BoardColorSettingsDialog(QWidget *parent = nullptr);
    ~BoardColorSettingsDialog();

    enum class ColorScheme {
        Classic,      // 經典棕色/米色（預設）
        BlueGray,     // 藍色和灰色
        GreenWhite,   // 綠色和白色
        PurplePink,   // 紫色和粉色
        WoodDark,     // 木色和深色
        OceanBlue,    // 海藍色
        LightTheme,   // 淺色柔和色
        Custom1,      // 自訂使用者選擇的顏色插槽 1
        Custom2,      // 自訂使用者選擇的顏色插槽 2
        Custom3,      // 自訂使用者選擇的顏色插槽 3
        Custom4,      // 自訂使用者選擇的顏色插槽 4
        Custom5,      // 自訂使用者選擇的顏色插槽 5
        Custom6,      // 自訂使用者選擇的顏色插槽 6
        Custom7       // 自訂使用者選擇的顏色插槽 7
    };

    struct BoardColorSettings {
        QColor lightSquareColor;
        QColor darkSquareColor;
        ColorScheme scheme;
        QString customName;  // 自訂配色方案的名稱
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
    QLabel* m_previewSquares[2][2];  // 2x2 預覽格線
    QWidget* m_customColorWidget;  // 包含自訂顏色控制的元件
    
    // 自訂顏色插槽儲存
    BoardColorSettings m_customSlots[7];
    
    void setupUI();
    void applyModernDialogStyle();  // 應用現代科技風格
    void updatePreview();
    void updateColorButtons();
    void applyPresetColorScheme(ColorScheme scheme);
    QPushButton* createPresetPreview(ColorScheme scheme, const QString& label);
    void loadCustomSlots();
    void saveCustomSlots();
    bool isCustomSlot(ColorScheme scheme) const;
};

#endif // BOARDCOLORSETTINGSDIALOG_H
