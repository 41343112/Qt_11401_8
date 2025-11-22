#ifndef QT_CHESS_H
#define QT_CHESS_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMap>
#include <QSoundEffect>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <QGroupBox>
#include <vector>
#include "chessboard.h"
#include "soundsettingsdialog.h"
#include "pieceiconsettingsdialog.h"
#include "boardcolorsettingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Qt_Chess;
}
QT_END_NAMESPACE

class Qt_Chess : public QMainWindow
{
    Q_OBJECT

public:
    Qt_Chess(QWidget *parent = nullptr);
    ~Qt_Chess();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSquareClicked(int row, int col);
    void onNewGameClicked();
    void onSoundSettingsClicked();
    void onPieceIconSettingsClicked();
    void onBoardColorSettingsClicked();
    void onFlipBoardClicked();
    void onStartButtonClicked();

private:
    Ui::Qt_Chess *ui;
    ChessBoard m_chessBoard;
    std::vector<std::vector<QPushButton*>> m_squares;
    QMap<QPushButton*, QPoint> m_buttonCoordinates; // For efficient button-to-coordinate lookup
    QPoint m_selectedSquare;
    bool m_pieceSelected;
    
    // Drag-and-drop state
    bool m_isDragging;
    QPoint m_dragStartSquare;
    QLabel* m_dragLabel;
    bool m_wasSelectedBeforeDrag;
    
    QPushButton* m_newGameButton;
    QWidget* m_boardWidget;
    QMenuBar* m_menuBar;
    
    // Sound effects
    QSoundEffect m_moveSound;
    QSoundEffect m_captureSound;
    QSoundEffect m_castlingSound;
    QSoundEffect m_checkSound;
    QSoundEffect m_checkmateSound;
    SoundSettingsDialog::SoundSettings m_soundSettings;
    
    // Piece icon settings
    PieceIconSettingsDialog::PieceIconSettings m_pieceIconSettings;
    QMap<QString, QPixmap> m_pieceIconCache; // Cache for loaded piece icons
    
    // Board color settings
    BoardColorSettingsDialog::BoardColorSettings m_boardColorSettings;
    
    // Board flip state
    bool m_isBoardFlipped;
    
    // Time control
    QComboBox* m_timeLimitCombo;
    QSlider* m_incrementSlider;
    QLabel* m_incrementLabel;
    QLabel* m_whiteTimeLabel;
    QLabel* m_blackTimeLabel;
    QPushButton* m_startButton;
    QTimer* m_gameTimer;
    int m_whiteTimeMs;  // White's remaining time in milliseconds
    int m_blackTimeMs;  // Black's remaining time in milliseconds
    int m_incrementMs;  // Increment per move in milliseconds
    bool m_timeControlEnabled;
    bool m_timerStarted;  // Track if timer has been manually started
    QWidget* m_boardContainer;  // Container for board with overlaid time displays
    
    void setupUI();
    void setupMenuBar();
    void updateBoard();
    void updateSquareColor(int row, int col);
    void updateStatus();
    void highlightValidMoves();
    void clearHighlights();
    void applyCheckHighlight(const QPoint& excludeSquare = QPoint(-1, -1));
    void restorePieceToSquare(const QPoint& square);
    PieceType showPromotionDialog(PieceColor color);
    QPoint getSquareAtPosition(const QPoint& pos) const;
    void updateSquareSizes();
    void initializeSounds();
    void loadSoundSettings();
    void applySoundSettings();
    void setSoundSource(QSoundEffect& sound, const QString& path);
    void stopAllSounds();
    bool isCaptureMove(const QPoint& from, const QPoint& to) const;
    bool isCastlingMove(const QPoint& from, const QPoint& to) const;
    void playSoundForMove(bool isCapture, bool isCastling);
    void loadPieceIconSettings();
    void applyPieceIconSettings();
    QString getPieceIconPath(PieceType type, PieceColor color) const;
    void displayPieceOnSquare(QPushButton* square, const ChessPiece& piece);
    void loadPieceIconsToCache();
    void clearPieceIconCache();
    QPixmap getCachedPieceIcon(PieceType type, PieceColor color) const;
    int calculateIconSize(QPushButton* square) const;
    void loadBoardColorSettings();
    void applyBoardColorSettings();
    void loadBoardFlipSettings();
    void saveBoardFlipSettings();
    int getDisplayRow(int logicalRow) const;
    int getDisplayCol(int logicalCol) const;
    int getLogicalRow(int displayRow) const;
    int getLogicalCol(int displayCol) const;
    void setupTimeControlUI(QVBoxLayout* rightPanelLayout);
    void updateTimeDisplays();
    void onTimeLimitChanged(int index);
    void onIncrementChanged(int value);
    void onGameTimerTick();
    void startTimer();
    void stopTimer();
    void applyIncrement();
    void loadTimeControlSettings();
    void saveTimeControlSettings();
    void positionTimeDisplaysOnBoard();
};
#endif // QT_CHESS_H
