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
#include <QProgressBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <vector>
#include "chessboard.h"
#include "chessengine.h"
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
    QProgressBar* m_whiteTimeProgressBar;  // 白方時間進度條
    QProgressBar* m_blackTimeProgressBar;  // 黑方時間進度條
    QPushButton* m_startButton;
    QTimer* m_gameTimer;
    int m_whiteTimeMs;  // 白方剩餘時間（毫秒）
    int m_blackTimeMs;  // 黑方剩餘時間（毫秒）
    int m_whiteInitialTimeMs;  // 白方初始時間（毫秒），用於進度條計算
    int m_blackInitialTimeMs;  // 黑方初始時間（毫秒），用於進度條計算
    int m_incrementMs;  // 每步的增量（毫秒）
    bool m_timeControlEnabled;
    bool m_timerStarted;  // 追蹤計時器是否已手動啟動
    QWidget* m_boardContainer;  // 帶有疊加時間顯示的棋盤容器
    QWidget* m_timeControlPanel;  // 時間控制設定面板
    QHBoxLayout* m_contentLayout;  // 主內容佈局，用於調整伸展因子
    int m_rightStretchIndex;  // 右側伸展項的索引
    
    // 棋譜面板
    QListWidget* m_moveListWidget;
    QPushButton* m_exportPGNButton;
    QPushButton* m_copyPGNButton;
    QWidget* m_moveListPanel;
    
    // 被吃掉的棋子面板和右側時間/吃子顯示面板
    QWidget* m_capturedWhitePanel;
    QWidget* m_capturedBlackPanel;
    QList<QLabel*> m_capturedWhiteLabels;
    QList<QLabel*> m_capturedBlackLabels;
    QLabel* m_whiteScoreDiffLabel;  // 白方分差標籤
    QLabel* m_blackScoreDiffLabel;  // 黑方分差標籤
    QWidget* m_rightTimePanel;  // 右側時間和被吃棋子的容器
    
    // 遊戲結束時的上下方面板
    QWidget* m_topEndGamePanel;     // 遊戲結束時對方的時間和吃子紀錄（棋盤上方）
    QWidget* m_bottomEndGamePanel;  // 遊戲結束時我方的時間和吃子紀錄（棋盤下方）
    
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
    
    // 電腦對弈引擎
    ChessEngine* m_chessEngine;
    QPushButton* m_humanModeButton;      // 雙人對弈按鈕
    QPushButton* m_computerModeButton;   // 電腦對弈按鈕
    QLabel* m_gameModeStatusLabel;       // 顯示電腦模式時的執白/執黑狀態
    GameMode m_currentGameMode;          // 當前遊戲模式
    // 選邊按鈕（電腦模式時顯示）
    QWidget* m_colorSelectionWidget;     // 選邊按鈕容器
    QPushButton* m_whiteButton;          // 執白按鈕
    QPushButton* m_randomButton;         // 隨機按鈕
    QPushButton* m_blackButton;          // 執黑按鈕
    bool m_isRandomColorSelected;        // 是否選擇隨機顏色
    QSlider* m_difficultySlider;
    QLabel* m_difficultyLabel;
    QLabel* m_difficultyValueLabel;
    QLabel* m_thinkingLabel;  // 顯示「電腦思考中...」
    QStringList m_uciMoveHistory;  // UCI 格式的移動歷史
    
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
    void moveWidgetsForGameEnd();  // 將時間和吃子紀錄移動到棋盤上下方
    void restoreWidgetsFromGameEnd();  // 將時間和吃子紀錄恢復到右側面板
    void showTimeControlAfterTimeout();  // 時間到期後顯示時間控制面板的輔助函數
    void updateTimeControlSizes();  // 根據視窗大小更新時間控制 UI 元素大小的輔助函數
    void resetBoardState();  // 重置棋盤到初始狀態的輔助函數
    int calculateTimeFromSliderValue(int value) const;  // 根據滑桿值計算時間（毫秒）的輔助函數
    QString getTimeTextFromSliderValue(int value) const;  // 根據滑桿值取得顯示文字的輔助函數
    void setRightPanelStretch(int stretch);  // 設置右側面板伸展因子的輔助函數
    
    // 棋譜功能
    void updateMoveList();
    void exportPGN();
    void copyPGN();
    QString generatePGN() const;
    
    // 被吃掉的棋子顯示
    void updateCapturedPiecesDisplay();
    int getPieceValue(PieceType type) const;  // 取得棋子分值的輔助函數
    
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
    
    // 電腦對弈功能
    void setupEngineUI(QVBoxLayout* layout);
    void initializeEngine();
    void onHumanModeClicked();           // 雙人模式按鈕點擊
    void onComputerModeClicked();        // 電腦模式按鈕點擊
    void onWhiteColorClicked();          // 執白按鈕點擊
    void onRandomColorClicked();         // 隨機按鈕點擊
    void onBlackColorClicked();          // 執黑按鈕點擊
    void onDifficultyChanged(int value);
    void onEngineBestMove(const QString& move);
    void onEngineReady();
    void onEngineError(const QString& error);
    void requestEngineMove();
    bool isComputerTurn() const;
    bool isPlayerPiece(PieceColor pieceColor) const;  // 檢查棋子是否為玩家的
    GameMode getCurrentGameMode() const;  // 取得當前遊戲模式
    void loadEngineSettings();
    void saveEngineSettings();
    QString getEnginePath() const;
    void updateGameModeUI();             // 更新遊戲模式 UI 狀態
};
#endif // QT_CHESS_H
