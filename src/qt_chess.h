#ifndef QT_CHESS_H
#define QT_CHESS_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMap>
#include <QSoundEffect>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <QGroupBox>
#include <QListWidget>
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
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSquareClicked(int row, int col);
    void onNewGameClicked();
    void onGiveUpClicked();
    void onSoundSettingsClicked();
    void onPieceIconSettingsClicked();
    void onBoardColorSettingsClicked();
    void onFlipBoardClicked();
    void onStartButtonClicked();
    void onExportPGNClicked();
    void onCopyPGNClicked();

private:
    Ui::Qt_Chess *ui;
    ChessBoard m_chessBoard;
    std::vector<std::vector<QPushButton*>> m_squares;
    QMap<QPushButton*, QPoint> m_buttonCoordinates; // 用於高效的按鈕到坐標查找
    QPoint m_selectedSquare;
    bool m_pieceSelected;
    
    // 拖放狀態
    bool m_isDragging;
    QPoint m_dragStartSquare;
    QLabel* m_dragLabel;
    bool m_wasSelectedBeforeDrag;
    
    QPushButton* m_newGameButton;
    QPushButton* m_giveUpButton;
    QWidget* m_boardWidget;
    QMenuBar* m_menuBar;
    bool m_gameStarted;  // 追蹤遊戲是否已開始
    
    // 音效
    QSoundEffect m_moveSound;
    QSoundEffect m_captureSound;
    QSoundEffect m_castlingSound;
    QSoundEffect m_checkSound;
    QSoundEffect m_checkmateSound;
    SoundSettingsDialog::SoundSettings m_soundSettings;
    
    // 棋子圖示設定
    PieceIconSettingsDialog::PieceIconSettings m_pieceIconSettings;
    QMap<QString, QPixmap> m_pieceIconCache; // 已載入棋子圖示的快取
    
    // 棋盤顏色設定
    BoardColorSettingsDialog::BoardColorSettings m_boardColorSettings;
    
    // 棋盤翻轉狀態
    bool m_isBoardFlipped;
    
    // 時間控制
    QSlider* m_whiteTimeLimitSlider;  // 白方時間的獨立滑桿
    QLabel* m_whiteTimeLimitLabel;
    QLabel* m_whiteTimeLimitTitleLabel;  // 白方時間的標題標籤
    QSlider* m_blackTimeLimitSlider;  // 黑方時間的獨立滑桿
    QLabel* m_blackTimeLimitLabel;
    QLabel* m_blackTimeLimitTitleLabel;  // 黑方時間的標題標籤
    QSlider* m_incrementSlider;
    QLabel* m_incrementLabel;
    QLabel* m_incrementTitleLabel;  // 增量的標題標籤
    QLabel* m_whiteTimeLabel;
    QLabel* m_blackTimeLabel;
    QPushButton* m_startButton;
    QTimer* m_gameTimer;
    int m_whiteTimeMs;  // 白方剩餘時間（毫秒）
    int m_blackTimeMs;  // 黑方剩餘時間（毫秒）
    int m_incrementMs;  // 每步的增量（毫秒）
    bool m_timeControlEnabled;
    bool m_timerStarted;  // 追蹤計時器是否已手動啟動
    QWidget* m_boardContainer;  // 帶有疊加時間顯示的棋盤容器
    QWidget* m_timeControlPanel;  // 時間控制設定面板
    QHBoxLayout* m_contentLayout;  // 主內容佈局，用於調整伸展因子
    int m_rightStretchIndex;  // 右側伸展項的索引
    
    // 吃子顯示
    QLabel* m_whiteCapturedLabel;  // 黑方吃掉的子（在黑方時間下方）
    QLabel* m_blackCapturedLabel;  // 白方吃掉的子（在白方時間上方）
    
    // 棋譜面板
    QListWidget* m_moveListWidget;
    QPushButton* m_exportPGNButton;
    QPushButton* m_copyPGNButton;
    QWidget* m_moveListPanel;
    
    // 回放控制
    QLabel* m_replayTitle;
    QPushButton* m_replayFirstButton;
    QPushButton* m_replayPrevButton;
    QPushButton* m_replayNextButton;
    QPushButton* m_replayLastButton;
    bool m_isReplayMode;
    int m_replayMoveIndex;  // 當前回放的棋步索引（-1 表示初始狀態）
    std::vector<std::vector<ChessPiece>> m_savedBoardState;  // 儲存進入回放前的棋盤狀態
    PieceColor m_savedCurrentPlayer;  // 儲存進入回放前的當前玩家
    
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
    void setupTimeControlUI(QVBoxLayout* timeControlPanelLayout);
    void updateTimeDisplays();
    void onWhiteTimeLimitChanged(int value);
    void onBlackTimeLimitChanged(int value);
    void onIncrementChanged(int value);
    void onGameTimerTick();
    void startTimer();
    void stopTimer();
    void applyIncrement();
    void loadTimeControlSettings();
    void saveTimeControlSettings();
    void handleGameEnd();  // 處理遊戲結束狀態的輔助函數
    void showTimeControlAfterTimeout();  // 時間到期後顯示時間控制面板的輔助函數
    void updateTimeControlSizes();  // 根據視窗大小更新時間控制 UI 元素大小的輔助函數
    void resetBoardState();  // 重置棋盤到初始狀態的輔助函數
    int calculateTimeFromSliderValue(int value) const;  // 根據滑桿值計算時間（毫秒）的輔助函數
    QString getTimeTextFromSliderValue(int value) const;  // 根據滑桿值取得顯示文字的輔助函數
    void setRightPanelStretch(int stretch);  // 設置右側面板伸展因子的輔助函數
    
    // 吃子顯示功能
    void updateCapturedPiecesDisplay();  // 更新吃子顯示
    int getPieceValue(PieceType type) const;  // 取得棋子價值
    QString renderCapturedPieces(const std::vector<PieceType>& pieces, PieceColor capturedColor, int& materialAdvantage) const;  // 渲染吃子顯示
    
    // 棋譜功能
    void updateMoveList();
    void exportPGN();
    void copyPGN();
    QString generatePGN() const;
    
    // 回放功能
    void enterReplayMode();
    void exitReplayMode();
    void replayToMove(int moveIndex);
    void onReplayFirstClicked();
    void onReplayPrevClicked();
    void onReplayNextClicked();
    void onReplayLastClicked();
    void updateReplayButtons();
    void saveBoardState();
    void restoreBoardState();
};
#endif // QT_CHESS_H
