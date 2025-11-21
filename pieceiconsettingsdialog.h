#ifndef PIECEICONSETTINGSDIALOG_H
#define PIECEICONSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QButtonGroup>
#include <QRadioButton>
#include "chesspiece.h"

class PieceIconSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PieceIconSettingsDialog(QWidget *parent = nullptr);
    ~PieceIconSettingsDialog();

    enum class IconSetType {
        Unicode,      // Default Unicode symbols
        Preset1,      // Preset icon set 1
        Preset2,      // Preset icon set 2
        Preset3,      // Preset icon set 3
        Custom        // Custom user-selected icons
    };

    struct PieceIconSettings {
        QString whiteKingIcon;
        QString whiteQueenIcon;
        QString whiteRookIcon;
        QString whiteBishopIcon;
        QString whiteKnightIcon;
        QString whitePawnIcon;
        QString blackKingIcon;
        QString blackQueenIcon;
        QString blackRookIcon;
        QString blackBishopIcon;
        QString blackKnightIcon;
        QString blackPawnIcon;
        bool useCustomIcons;
        IconSetType iconSetType;
    };

    PieceIconSettings getSettings() const;
    void setSettings(const PieceIconSettings& settings);
    
    static PieceIconSettings getDefaultSettings();
    static PieceIconSettings getPresetSettings(IconSetType setType);

private slots:
    void onIconSetTypeChanged(int index);
    void onIconSetButtonClicked(int id);
    void onBrowseWhiteKing();
    void onBrowseWhiteQueen();
    void onBrowseWhiteRook();
    void onBrowseWhiteBishop();
    void onBrowseWhiteKnight();
    void onBrowseWhitePawn();
    void onBrowseBlackKing();
    void onBrowseBlackQueen();
    void onBrowseBlackRook();
    void onBrowseBlackBishop();
    void onBrowseBlackKnight();
    void onBrowseBlackPawn();
    void onPreviewWhiteKing();
    void onPreviewWhiteQueen();
    void onPreviewWhiteRook();
    void onPreviewWhiteBishop();
    void onPreviewWhiteKnight();
    void onPreviewWhitePawn();
    void onPreviewBlackKing();
    void onPreviewBlackQueen();
    void onPreviewBlackRook();
    void onPreviewBlackBishop();
    void onPreviewBlackKnight();
    void onPreviewBlackPawn();
    void onResetWhiteKing();
    void onResetWhiteQueen();
    void onResetWhiteRook();
    void onResetWhiteBishop();
    void onResetWhiteKnight();
    void onResetWhitePawn();
    void onResetBlackKing();
    void onResetBlackQueen();
    void onResetBlackRook();
    void onResetBlackBishop();
    void onResetBlackKnight();
    void onResetBlackPawn();
    void onUseCustomIconsToggled(bool enabled);
    void onResetToDefaults();
    void onAccept();

private:
    PieceIconSettings m_settings;
    
    QComboBox* m_iconSetComboBox;
    QCheckBox* m_useCustomIconsCheckBox;
    QButtonGroup* m_iconSetButtonGroup;
    
    // White pieces
    QLineEdit* m_whiteKingEdit;
    QPushButton* m_whiteKingBrowseButton;
    QPushButton* m_whiteKingPreviewButton;
    QPushButton* m_whiteKingResetButton;
    
    QLineEdit* m_whiteQueenEdit;
    QPushButton* m_whiteQueenBrowseButton;
    QPushButton* m_whiteQueenPreviewButton;
    QPushButton* m_whiteQueenResetButton;
    
    QLineEdit* m_whiteRookEdit;
    QPushButton* m_whiteRookBrowseButton;
    QPushButton* m_whiteRookPreviewButton;
    QPushButton* m_whiteRookResetButton;
    
    QLineEdit* m_whiteBishopEdit;
    QPushButton* m_whiteBishopBrowseButton;
    QPushButton* m_whiteBishopPreviewButton;
    QPushButton* m_whiteBishopResetButton;
    
    QLineEdit* m_whiteKnightEdit;
    QPushButton* m_whiteKnightBrowseButton;
    QPushButton* m_whiteKnightPreviewButton;
    QPushButton* m_whiteKnightResetButton;
    
    QLineEdit* m_whitePawnEdit;
    QPushButton* m_whitePawnBrowseButton;
    QPushButton* m_whitePawnPreviewButton;
    QPushButton* m_whitePawnResetButton;
    
    // Black pieces
    QLineEdit* m_blackKingEdit;
    QPushButton* m_blackKingBrowseButton;
    QPushButton* m_blackKingPreviewButton;
    QPushButton* m_blackKingResetButton;
    
    QLineEdit* m_blackQueenEdit;
    QPushButton* m_blackQueenBrowseButton;
    QPushButton* m_blackQueenPreviewButton;
    QPushButton* m_blackQueenResetButton;
    
    QLineEdit* m_blackRookEdit;
    QPushButton* m_blackRookBrowseButton;
    QPushButton* m_blackRookPreviewButton;
    QPushButton* m_blackRookResetButton;
    
    QLineEdit* m_blackBishopEdit;
    QPushButton* m_blackBishopBrowseButton;
    QPushButton* m_blackBishopPreviewButton;
    QPushButton* m_blackBishopResetButton;
    
    QLineEdit* m_blackKnightEdit;
    QPushButton* m_blackKnightBrowseButton;
    QPushButton* m_blackKnightPreviewButton;
    QPushButton* m_blackKnightResetButton;
    
    QLineEdit* m_blackPawnEdit;
    QPushButton* m_blackPawnBrowseButton;
    QPushButton* m_blackPawnPreviewButton;
    QPushButton* m_blackPawnResetButton;
    
    void setupUI();
    void createPieceRow(QGridLayout* gridLayout, int& row, const QString& label,
                       QLineEdit*& iconEdit,
                       QPushButton*& browseButton, QPushButton*& previewButton,
                       QPushButton*& resetButton,
                       void (PieceIconSettingsDialog::*browseSlot)(),
                       void (PieceIconSettingsDialog::*previewSlot)(),
                       void (PieceIconSettingsDialog::*resetSlot)());
    QWidget* createIconSetPreviewWidget(IconSetType setType, const QString& label);
    QString browseForIconFile();
    void previewIcon(const QString& iconFile);
    void updateCustomIconsControls();
    void applyPresetIconSet(IconSetType setType);
    static QString getSetDirectoryName(IconSetType setType);
};

#endif // PIECEICONSETTINGSDIALOG_H
