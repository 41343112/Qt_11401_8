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
#include <QSet>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <QRandomGenerator>
#include <QGroupBox>
#include <QListWidget>
#include <QProgressBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <vector>
#include "chessboard.h"
#include "chessengine.h"
#include "soundsettingsdialog.h"
#include "pieceiconsettingsdialog.h"
#include "boardcolorsettingsdialog.h"
#include "updatechecker.h"
#include "networkmanager.h"
#include "onlinedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Qt_Chess;
}
QT_END_NAMESPACE

// Constants for online mode
constexpr qint64 DRAW_REQUEST_COOLDOWN_MS = 3000;  // 3 seconds cooldown for draw requests
constexpr int ROOM_NUMBER_MIN = 1000;              // Minimum room number
constexpr int ROOM_NUMBER_MAX = 9999;              // Maximum room number
constexpr int ROOM_NUMBER_LENGTH = 4;              // Room number length

// Game mode identifiers (used in network messages and UI)
constexpr const char* GAME_MODE_FOG_OF_WAR = "霧戰";
constexpr const char* GAME_MODE_GRAVITY = "地吸引力";
constexpr const char* GAME_MODE_TELEPORT = "傳送陣";
constexpr const char* GAME_MODE_DICE = "骰子";
constexpr const char* GAME_MODE_BOMB = "踩地雷";

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
    void onResignClicked();       // 認輸按鈕點擊
    void onRequestDrawClicked();  // 請求和棋按鈕點擊
    void onExitClicked();         // 退出遊戲按鈕點擊
    void onSoundSettingsClicked();
    void onPieceIconSettingsClicked();
    void onBoardColorSettingsClicked();
    void onFlipBoardClicked();
    void onToggleFullScreenClicked();
    void onStartButtonClicked();
    void onExportPGNClicked();
    void onCopyPGNClicked();
    void onToggleBackgroundMusicClicked();
    void onCheckForUpdatesClicked();
    void onUpdateCheckFinished(bool updateAvailable);
    void onUpdateCheckFailed(const QString& error);
    
    // 主選單按鈕點擊
    void onMainMenuLocalPlayClicked();
    void onMainMenuComputerPlayClicked();
    void onMainMenuOnlinePlayClicked();
    void onMainMenuSettingsClicked();
    void onBackToMainMenuClicked();

private:
    // ========================================
    // 核心遊戲狀態 (Core Game State)
    // ========================================
    Ui::Qt_Chess *ui;
    ChessBoard m_chessBoard;
    GameMode m_currentGameMode;          // 當前遊戲模式
    bool m_gameStarted;                  // 追蹤遊戲是否已開始
    QPoint m_selectedSquare;
    bool m_pieceSelected;
    QPoint m_lastMoveFrom;               // 上一步移動的高亮
    QPoint m_lastMoveTo;
    
    // ========================================
    // UI 元件 - 基礎佈局 (UI Components - Basic Layout)
    // ========================================
    QWidget* m_boardWidget;
    QWidget* m_boardContainer;           // 帶有疊加時間顯示的棋盤容器
    QHBoxLayout* m_contentLayout;        // 主內容佈局，用於調整伸展因子
    int m_rightStretchIndex;             // 右側伸展項的索引
    QMenuBar* m_menuBar;
    
    // ========================================
    // UI 元件 - 棋盤 (UI Components - Chess Board)
    // ========================================
    std::vector<std::vector<QPushButton*>> m_squares;
    QMap<QPushButton*, QPoint> m_buttonCoordinates; // 用於高效的按鈕到坐標查找
    
    // 拖放狀態
    bool m_isDragging;
    QPoint m_dragStartSquare;
    QLabel* m_dragLabel;
    bool m_wasSelectedBeforeDrag;
    
    // ========================================
    // UI 元件 - 遊戲控制按鈕 (UI Components - Game Control Buttons)
    // ========================================
    QPushButton* m_newGameButton;
    QPushButton* m_resignButton;         // 認輸按鈕
    QPushButton* m_requestDrawButton;    // 請求和棋按鈕
    QPushButton* m_exitButton;           // 退出遊戲按鈕
    QWidget* m_boardButtonPanel;         // 棋盤下方按鈕面板
    QPushButton* m_startButton;
    
    // ========================================
    // UI 元件 - 主選單 (UI Components - Main Menu)
    // ========================================
    QWidget* m_mainMenuWidget;           // 主選單容器
    QPushButton* m_mainMenuLocalPlayButton;    // 本地遊玩按鈕
    QPushButton* m_mainMenuComputerPlayButton; // 與電腦對戰按鈕
    QPushButton* m_mainMenuOnlinePlayButton;   // 線上遊玩按鈕
    QPushButton* m_mainMenuSettingsButton;     // 設定按鈕
    QWidget* m_gameContentWidget;        // 遊戲內容容器
    QPushButton* m_backToMenuButton;     // 返回主選單按鈕
    
    // ========================================
    // 時間控制系統 (Time Control System)
    // ========================================
    QWidget* m_timeControlPanel;         // 時間控制設定面板
    QSlider* m_whiteTimeLimitSlider;     // 白方時間的獨立滑桿
    QLabel* m_whiteTimeLimitLabel;
    QLabel* m_whiteTimeLimitTitleLabel;  // 白方時間的標題標籤
    QSlider* m_blackTimeLimitSlider;     // 黑方時間的獨立滑桿
    QLabel* m_blackTimeLimitLabel;
    QLabel* m_blackTimeLimitTitleLabel;  // 黑方時間的標題標籤
    QSlider* m_incrementSlider;
    QLabel* m_incrementLabel;
    QLabel* m_incrementTitleLabel;       // 增量的標題標籤
    QLabel* m_whiteTimeLabel;
    QLabel* m_blackTimeLabel;
    QProgressBar* m_whiteTimeProgressBar;  // 白方時間進度條
    QProgressBar* m_blackTimeProgressBar;  // 黑方時間進度條
    QTimer* m_gameTimer;
    int m_whiteTimeMs;                   // 白方剩餘時間（毫秒）
    int m_blackTimeMs;                   // 黑方剩餘時間（毫秒）
    int m_whiteInitialTimeMs;            // 白方初始時間（毫秒），用於進度條計算
    int m_blackInitialTimeMs;            // 黑方初始時間（毫秒），用於進度條計算
    int m_incrementMs;                   // 每步的增量（毫秒）
    bool m_timeControlEnabled;
    bool m_timerStarted;                 // 追蹤計時器是否已手動啟動
    qint64 m_serverTimeOffset;           // 伺服器時間偏移（ms）：serverTime - localTime
    qint64 m_gameStartLocalTime;         // 遊戲開始時的本地時間戳（ms）
    qint64 m_currentTurnStartTime;       // 當前回合開始的時間戳（ms）
    
    // 伺服器控制的計時器狀態
    qint64 m_serverTimeA;                // 玩家 A (房主) 剩餘時間（毫秒）
    qint64 m_serverTimeB;                // 玩家 B (房客) 剩餘時間（毫秒）
    QString m_serverCurrentPlayer;       // 伺服器端當前玩家 ("White" or "Black")
    qint64 m_serverLastSwitchTime;       // 伺服器最後切換時間（UNIX 秒數）
    bool m_useServerTimer;               // 是否使用伺服器控制的計時器
    qint64 m_lastServerUpdateTime;       // 最後一次收到伺服器更新的本地時間（毫秒）
    
    // 遊戲結束時的上下方面板
    QWidget* m_topEndGamePanel;          // 遊戲結束時對方的時間和吃子紀錄（棋盤上方）
    QWidget* m_bottomEndGamePanel;       // 遊戲結束時我方的時間和吃子紀錄（棋盤下方）
    
    // ========================================
    // 棋譜系統 (Move History System)
    // ========================================
    QListWidget* m_moveListWidget;
    QPushButton* m_exportPGNButton;
    QPushButton* m_copyPGNButton;
    QWidget* m_moveListPanel;
    
    // ========================================
    // 被吃棋子顯示系統 (Captured Pieces Display System)
    // ========================================
    QWidget* m_capturedWhitePanel;
    QWidget* m_capturedBlackPanel;
    QList<QLabel*> m_capturedWhiteLabels;
    QList<QLabel*> m_capturedBlackLabels;
    QLabel* m_whiteScoreDiffLabel;       // 白方分差標籤
    QLabel* m_blackScoreDiffLabel;       // 黑方分差標籤
    QWidget* m_rightTimePanel;           // 右側時間和被吃棋子的容器
    
    // ========================================
    // 回放系統 (Replay System)
    // ========================================
    QLabel* m_replayTitle;
    QPushButton* m_replayFirstButton;
    QPushButton* m_replayPrevButton;
    QPushButton* m_replayNextButton;
    QPushButton* m_replayLastButton;
    bool m_isReplayMode;
    int m_replayMoveIndex;               // 當前回放的棋步索引（-1 表示初始狀態）
    std::vector<std::vector<ChessPiece>> m_savedBoardState;  // 儲存進入回放前的棋盤狀態
    PieceColor m_savedCurrentPlayer;     // 儲存進入回放前的當前玩家
    
    // ========================================
    // 電腦對弈系統 (Computer Chess Engine System)
    // ========================================
    ChessEngine* m_chessEngine;
    QPushButton* m_humanModeButton;      // 雙人對弈按鈕
    QPushButton* m_computerModeButton;   // 電腦對弈按鈕
    QLabel* m_gameModeStatusLabel;       // 顯示電腦模式時的執白/執黑狀態
    QWidget* m_colorSelectionWidget;     // 選邊按鈕容器
    QPushButton* m_whiteButton;          // 執白按鈕
    QPushButton* m_randomButton;         // 隨機按鈕
    QPushButton* m_blackButton;          // 執黑按鈕
    bool m_isRandomColorSelected;        // 是否選擇隨機顏色
    QSlider* m_difficultySlider;
    QLabel* m_difficultyLabel;
    QLabel* m_difficultyValueLabel;
    QLabel* m_thinkingLabel;             // 顯示「電腦思考中...」
    QStringList m_uciMoveHistory;        // UCI 格式的移動歷史
    
    // ========================================
    // 線上對戰系統 (Online Game System)
    // ========================================
    NetworkManager* m_networkManager;    // 網路管理器
    QPushButton* m_onlineModeButton;     // 線上對戰按鈕
    QPushButton* m_exitRoomButton;       // 退出房間按鈕
    QPushButton* m_createRoomButton;     // 創建房間按鈕（右側面板）
    QPushButton* m_joinRoomButton;       // 加入房間按鈕（右側面板）
    QWidget* m_onlineButtonsWidget;      // 線上模式按鈕容器
    QLabel* m_connectionStatusLabel;     // 連線狀態標籤
    QLabel* m_roomInfoLabel;             // 房間資訊標籤
    bool m_isOnlineGame;                 // 是否為線上對戰
    bool m_waitingForOpponent;           // 等待對手
    PieceColor m_onlineHostSelectedColor;  // 房主選擇的顏色（線上模式）
    qint64 m_lastDrawRequestTime;        // 上次請求和棋的時間（毫秒）
    QMap<QString, bool> m_selectedGameModes;  // 選擇的遊戲模式
    
    // 霧戰模式相關 (Fog of War Mode)
    bool m_fogOfWarEnabled;              // 是否啟用霧戰模式
    std::vector<std::vector<bool>> m_visibleSquares;  // 可見方格（8x8）
    
    // 地吸引力模式相關 (Gravity Mode)
    bool m_gravityModeEnabled;           // 是否啟用地吸引力模式
    
    // 傳送陣模式相關 (Teleportation Mode)
    bool m_teleportModeEnabled;          // 是否啟用傳送陣模式
    QPoint m_teleportPortal1;            // 傳送門位置1
    QPoint m_teleportPortal2;            // 傳送門位置2
    QPixmap m_teleportIconCache;         // 傳送門圖示快取
    
    // 地雷爆炸動畫 (Mine Explosion Animation)
    QSet<QPushButton*> m_explodingSquares;  // 正在顯示爆炸動畫的方格
    
    // ========================================
    // 音效系統 (Sound System)
    // ========================================
    QSoundEffect m_moveSound;
    QSoundEffect m_captureSound;
    QSoundEffect m_castlingSound;
    QSoundEffect m_checkSound;
    QSoundEffect m_checkmateSound;
    QSoundEffect m_explosionSound;  // 地雷爆炸音效
    SoundSettingsDialog::SoundSettings m_soundSettings;
    
    // 背景音樂
    QMediaPlayer* m_bgmPlayer;
    QAudioOutput* m_audioOutput;         // Qt6: 音量控制, Qt5: nullptr
    bool m_bgmEnabled;
    int m_bgmVolume;                     // 0-100
    QStringList m_bgmList;               // 背景音樂列表
    int m_lastBgmIndex;                  // 上一次播放的音樂索引，避免重複
    
    // ========================================
    // 動畫系統 (Animation System)
    // ========================================
    QWidget* m_animationOverlay;         // 動畫疊加層
    QLabel* m_animationLabel;            // 動畫文字標籤
    QLabel* m_animationSubLabel;         // 動畫副標籤（用於多行動畫）
    QTimer* m_animationTimer;            // 動畫計時器
    int m_animationStep;                 // 動畫當前步驟
    bool m_pendingGameStart;             // 是否有待處理的遊戲開始
    
    // 啟動動畫
    QTimer* m_startupAnimationTimer;     // 啟動動畫計時器
    int m_startupAnimationStep;          // 啟動動畫當前步驟
    QPropertyAnimation* m_fadeAnimation; // 淡入淡出動畫
    QPropertyAnimation* m_scaleAnimation; // 縮放動畫
    QGraphicsOpacityEffect* m_opacityEffect; // 透明度效果
    
    // ========================================
    // 設定系統 (Settings System)
    // ========================================
    // 棋子圖示設定
    PieceIconSettingsDialog::PieceIconSettings m_pieceIconSettings;
    QMap<QString, QPixmap> m_pieceIconCache; // 已載入棋子圖示的快取
    
    // 棋盤顏色設定
    BoardColorSettingsDialog::BoardColorSettings m_boardColorSettings;
    
    // 棋盤翻轉狀態
    bool m_isBoardFlipped;
    
    // ========================================
    // 更新檢查系統 (Update Checker System)
    // ========================================
    UpdateChecker* m_updateChecker;      // 更新檢查器
    bool m_manualUpdateCheck;            // 是否為手動檢查更新
    
    // ========================================
    // UI 設置與佈局 (UI Setup and Layout)
    // ========================================
    void setupUI();
    void setupMenuBar();
    void setupMainMenu();
    void setupTimeControlUI(QVBoxLayout* timeControlPanelLayout);
    void setupEngineUI(QVBoxLayout* layout);
    void updateSquareSizes();
    void updateTimeControlSizes();
    void applyModernStylesheet();
    
    // ========================================
    // 主選單管理 (Main Menu Management)
    // ========================================
    void showMainMenu();
    void showGameContent();
    void resetGameState();
    
    // ========================================
    // 棋盤顯示與更新 (Board Display and Update)
    // ========================================
    void updateBoard();
    void updateSquareColor(int row, int col);
    void updateStatus();
    void displayPieceOnSquare(QPushButton* square, const ChessPiece& piece);
    QString getPieceTextColor(int logicalRow, int logicalCol) const;
    
    // 高亮顯示
    void highlightValidMoves();
    void clearHighlights();
    void applyCheckHighlight(const QPoint& excludeSquare = QPoint(-1, -1));
    void applyLastMoveHighlight();
    
    // 棋盤座標轉換
    int getDisplayRow(int logicalRow) const;
    int getDisplayCol(int logicalCol) const;
    int getLogicalRow(int displayRow) const;
    int getLogicalCol(int displayCol) const;
    QPoint getSquareAtPosition(const QPoint& pos) const;
    
    // 棋盤輔助功能
    void restorePieceToSquare(const QPoint& square);
    void resetBoardState();
    PieceType showPromotionDialog(PieceColor color);
    void handleMineExplosion(const QPoint& logicalPosition, bool isOpponentMove = false);
    
    // ========================================
    // 時間控制系統 (Time Control System)
    // ========================================
    void updateTimeDisplays();
    void updateTimeDisplaysFromServer();
    void onWhiteTimeLimitChanged(int value);
    void onBlackTimeLimitChanged(int value);
    void onIncrementChanged(int value);
    void onGameTimerTick();
    void startTimer();
    void stopTimer();
    void applyIncrement();
    void loadTimeControlSettings();
    void saveTimeControlSettings();
    void handleGameEnd();
    void moveWidgetsForGameEnd();
    void restoreWidgetsFromGameEnd();
    void showTimeControlAfterTimeout();
    int calculateTimeFromSliderValue(int value) const;
    QString getTimeTextFromSliderValue(int value) const;
    void setRightPanelStretch(int stretch);
    
    // ========================================
    // 棋譜管理系統 (Move History Management)
    // ========================================
    void updateMoveList();
    void exportPGN();
    void copyPGN();
    QString generatePGN() const;
    
    // ========================================
    // 被吃棋子顯示系統 (Captured Pieces Display)
    // ========================================
    void updateCapturedPiecesDisplay();
    int getPieceValue(PieceType type) const;
    
    // ========================================
    // 回放系統 (Replay System)
    // ========================================
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
    
    // ========================================
    // 電腦對弈系統 (Computer Chess Engine)
    // ========================================
    void initializeEngine();
    void onHumanModeClicked();
    void onComputerModeClicked();
    void onWhiteColorClicked();
    void onRandomColorClicked();
    void onBlackColorClicked();
    void onDifficultyChanged(int value);
    void onEngineBestMove(const QString& move);
    void onEngineReady();
    void onEngineError(const QString& error);
    void requestEngineMove();
    bool isComputerTurn() const;
    bool isPlayerPiece(PieceColor pieceColor) const;
    GameMode getCurrentGameMode() const;
    void loadEngineSettings();
    void saveEngineSettings();
    QString getEnginePath() const;
    void updateGameModeUI();
    
    // ========================================
    // 線上對戰系統 (Online Game System)
    // ========================================
    void initializeNetwork();
    void onOnlineModeClicked();
    void onCreateRoomButtonClicked();
    void onJoinRoomButtonClicked();
    void onNetworkConnected();
    void onNetworkDisconnected();
    void onNetworkError(const QString& error);
    void onRoomCreated(const QString& roomNumber);
    void onOpponentJoined();
    void onPlayerLeft();
    void onPromotedToHost();
    void onOpponentMove(const QPoint& from, const QPoint& to, PieceType promotionType, QPoint finalPosition);
    void onGameStartReceived(PieceColor playerColor);
    void onStartGameReceived(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor, qint64 serverTimeOffset, const QMap<QString, bool>& gameModes, const std::vector<QPoint>& minePositions);
    void onTimeSettingsReceived(int whiteTimeMs, int blackTimeMs, int incrementMs);
    void onTimerStateReceived(qint64 timeA, qint64 timeB, const QString& currentPlayer, qint64 lastSwitchTime);
    void onSurrenderReceived();
    void onDrawOfferReceived();
    void onDrawResponseReceived(bool accepted);
    void onOpponentDisconnected();
    void onCancelRoomClicked();
    void onExitRoomClicked();
    void updateConnectionStatus();
    bool isOnlineTurn() const;
    void showRoomInfoDialog(const QString& roomNumber);
    
    // 霧戰模式 (Fog of War Mode)
    void updateVisibleSquares();         // 更新可見方格
    bool isSquareVisible(int row, int col) const;  // 檢查方格是否可見
    void calculateVisibleSquares(PieceColor playerColor);  // 計算玩家可見的方格
    
    // 地吸引力模式 (Gravity Mode)
    void applyGravity();                 // 應用地吸引力讓棋子下落
    void rotateBoardDisplay(bool rotate);  // 旋轉棋盤UI顯示
    
    // 傳送陣模式 (Teleportation Mode)
    void initializeTeleportPortals();    // 初始化傳送門位置
    void resetTeleportPortals();         // 重置傳送門位置
    bool isTeleportPortal(int row, int col) const;  // 檢查方格是否為傳送門
    void handleTeleportation(const QPoint& from, const QPoint& to);  // 處理傳送
    bool performTeleportationMove(const QPoint& from, const QPoint& to);  // 執行傳送動作（不重置傳送門）
    QPoint handleTeleportationAndGetFinalPosition(const QPoint& from, const QPoint& to);  // 處理傳送並返回最終位置
    void applyFinalPosition(const QPoint& to, const QPoint& finalPosition);  // 應用最終位置（用於接收對手的傳送結果）
    
    // ========================================
    // 音效系統 (Sound System)
    // ========================================
    void initializeSounds();
    void loadSoundSettings();
    void applySoundSettings();
    void setSoundSource(QSoundEffect& sound, const QString& path);
    void stopAllSounds();
    bool isCaptureMove(const QPoint& from, const QPoint& to) const;
    bool isCastlingMove(const QPoint& from, const QPoint& to) const;
    void playSoundForMove(bool isCapture, bool isCastling);
    
    // 背景音樂
    void initializeBackgroundMusic();
    void startBackgroundMusic();
    void stopBackgroundMusic();
    void toggleBackgroundMusic();
    void setBackgroundMusicVolume(int volume);
    
    // ========================================
    // 動畫系統 (Animation System)
    // ========================================
    void playGameStartAnimation();
    void onAnimationStep();
    void finishGameStartAnimation();
    void playStartupAnimation();
    void onStartupAnimationStep();
    void finishStartupAnimation();
    void playStartupTextAnimation(QLabel* label, const QString& text, const QString& color, int fontSize);
    
    // ========================================
    // 設定系統 (Settings System)
    // ========================================
    // 棋子圖示設定
    void loadPieceIconSettings();
    void applyPieceIconSettings();
    QString getPieceIconPath(PieceType type, PieceColor color) const;
    void loadPieceIconsToCache();
    void clearPieceIconCache();
    QPixmap getCachedPieceIcon(PieceType type, PieceColor color) const;
    int calculateIconSize(QPushButton* square) const;
    
    // 棋盤顏色設定
    void loadBoardColorSettings();
    void applyBoardColorSettings();
    
    // 棋盤翻轉設定
    void loadBoardFlipSettings();
    void saveBoardFlipSettings();
};
#endif // QT_CHESS_H
