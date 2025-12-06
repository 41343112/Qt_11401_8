#include "qt_chess.h"
#include "ui_qt_chess.h"
#include "chessengine.h"
#include "soundsettingsdialog.h"
#include "pieceiconsettingsdialog.h"
#include "boardcolorsettingsdialog.h"
#include <QMessageBox>
#include <QFont>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QResizeEvent>
#include <QPointer>
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSlider>
#include <QTimer>
#include <QGroupBox>
#include <QFileDialog>
#include <QDate>
#include <QTextStream>
#include <QClipboard>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QApplication>
#include <QRandomGenerator>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QTextEdit>
#include <algorithm>

namespace {
const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: rgba(255, 80, 80, 0.85); border: 2px solid #FF3333; }";
const int DEFAULT_ICON_SIZE = 40; // 預設圖示大小（像素）
const int MAX_TIME_LIMIT_SECONDS = 1800; // 最大時間限制：30 分鐘
const int MAX_SLIDER_POSITION = 31; // 滑桿範圍：0（無限制）、1（30秒）、2-31（1-30 分鐘）
const int MAX_MINUTES = 30; // 最大時間限制（分鐘）
const QString GAME_ENDED_TEXT = "遊戲結束"; // 遊戲結束時顯示的文字
const int UPDATE_CHECK_DELAY_MS = 3000; // 啟動後檢查更新的延遲時間（毫秒）
const int RELEASE_NOTES_PREVIEW_LENGTH = 200; // 更新說明預覽的字元數

// 上一步移動高亮顏色 - 現代科技風格的青色/霓虹色調
const QString LAST_MOVE_LIGHT_COLOR = "#7FDBDB";  // 淺色格子的高亮（科技青色）
const QString LAST_MOVE_DARK_COLOR = "#4ECDC4";   // 深色格子的高亮（霓虹青色）

// ===== 現代科技風格主題顏色 =====
const QString THEME_BG_DARK = "#1A1A2E";           // 深色背景
const QString THEME_BG_MEDIUM = "#16213E";         // 中等深度背景
const QString THEME_BG_PANEL = "#0F3460";          // 面板背景
const QString THEME_ACCENT_PRIMARY = "#00D9FF";    // 主要強調色（霓虹青色）
const QString THEME_ACCENT_SECONDARY = "#E94560";  // 次要強調色（霓虹粉色）
const QString THEME_ACCENT_SUCCESS = "#00FF88";    // 成功色（霓虹綠色）
const QString THEME_ACCENT_WARNING = "#FFD93D";    // 警告色（霓虹黃色）
const QString THEME_TEXT_PRIMARY = "#EAEAEA";      // 主要文字顏色
const QString THEME_BORDER = "#2A4066";            // 邊框顏色

// 視窗大小的佈局常數
const int PANEL_SPACING = 10;          // 面板之間的間距
const int BASE_MARGINS =   20;           // 基本佈局邊距（不包括棋盤容器的 2*BOARD_CONTAINER_MARGIN）
const int TIME_LABEL_SPACING = 0;     // 時間標籤周圍的間距（已禁用）
const int BOARD_CONTAINER_MARGIN = 0;  // 棋盤容器每側的邊距（已禁用）

// UI 元素的縮放常數
const int MIN_SQUARE_SIZE = 40;        // 棋盤格子的最小大小
const int MAX_SQUARE_SIZE = 150;       // 棋盤格子的最大大小
const int MIN_UI_FONT_SIZE = 10;       // UI 元素的最小字體大小
const int MAX_UI_FONT_SIZE = 20;       // UI 元素的最大字體大小
const int UI_FONT_SCALE_DIVISOR = 5;   // 根據格子大小縮放 UI 字體的除數
const int MIN_TIME_LABEL_HEIGHT = 30;  // 時間標籤的最小高度
const int MAX_TIME_LABEL_HEIGHT = 50;  // 時間標籤的最大高度
const int MIN_TIME_LABEL_WIDTH = 0;  // 時間標籤的最小寬度（水平定位）

// 時間控制 UI 縮放常數
const int TIME_CONTROL_FONT_DIVISOR = 6;     // 縮放時間控制標籤字體的除數
const int BUTTON_FONT_DIVISOR = 5;           // 縮放按鈕字體的除數
const int SLIDER_HEIGHT_DIVISOR = 3;         // 縮放滑桿高度的除數
const int MIN_TIME_CONTROL_FONT = 8;         // 時間控制標籤的最小字體大小
const int MAX_TIME_CONTROL_FONT = 20;        // 時間控制標籤的最大字體大小
const int MIN_BUTTON_FONT = 10;              // 按鈕的最小字體大小
const int MAX_BUTTON_FONT = 20;              // 按鈕的最大字體大小
const int MIN_SLIDER_HEIGHT = 20;            // 滑桿的最小高度
const int MAX_SLIDER_HEIGHT = 80;            // 滑桿的最大高度
const int SLIDER_HANDLE_EXTRA = 10;          // 滑桿手柄的額外空間
const int LOW_TIME_THRESHOLD_MS = 10000;     // 低時間警告的閾值（10 秒）
const int MIN_PANEL_WIDTH = 50;              // 左右面板的最小寬度（像素）
const int MAX_PANEL_WIDTH = 600;              // 左右面板的最大寬度（像素）

// 遊戲結束佈局常數
const int ENDGAME_PANEL_FALLBACK_WIDTH = 200;   // 遊戲結束面板的後備寬度
const int ENDGAME_PANEL_FALLBACK_HEIGHT = 50;   // 遊戲結束面板的後備高度
const int NORMAL_PANEL_FALLBACK_WIDTH = 30;     // 正常面板的後備寬度
const int NORMAL_PANEL_FALLBACK_HEIGHT = 100;   // 正常面板的後備高度

// PGN 格式常數
const int PGN_MOVES_PER_LINE = 6;            // PGN 檔案中每行的移動回合數

// ELO 評分常數（用於難度顯示）
const int ELO_BASE = 250;                    // 最低 ELO 評分（對應 Skill Level 0）
const int ELO_PER_LEVEL = 150;               // 每級增加的 ELO 分數（確保結果能被50整除）

// 計算 ELO 評分的輔助函數
static int calculateElo(int skillLevel) {
    return ELO_BASE + skillLevel * ELO_PER_LEVEL;
}

// 根據難度等級取得中文難度名稱
static QString getDifficultyName(int skillLevel) {
    if (skillLevel <= 4) {        // Level 0-4
        return "初學";
    } else if (skillLevel <= 8) { // Level 5-8
        return "簡單";
    } else if (skillLevel <= 12) { // Level 9-12
        return "中等";
    } else if (skillLevel <= 16) { // Level 13-16
        return "困難";
    } else {                       // Level 17-20
        return "大師";
    }
}

// 獲取面板的實際寬度，如果尚未渲染則使用後備值的輔助函數
static int getPanelWidth(QWidget* panel) {
    if (!panel) return 0;

    int width = panel->width();
    if (width <= 0) {
        width = panel->sizeHint().width();
        if (width <= 0) {
            width = MIN_PANEL_WIDTH;
        }
    }
    return width;
}
}

Qt_Chess::Qt_Chess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Qt_Chess)
    , m_selectedSquare(-1, -1)
    , m_pieceSelected(false)
    , m_isDragging(false)
    , m_dragStartSquare(-1, -1)
    , m_dragLabel(nullptr)
    , m_wasSelectedBeforeDrag(false)
    , m_boardWidget(nullptr)
    , m_menuBar(nullptr)
    , m_gameStarted(false)
    , m_isBoardFlipped(false)
    , m_lastMoveFrom(-1, -1)
    , m_lastMoveTo(-1, -1)
    , m_whiteTimeLimitSlider(nullptr)
    , m_whiteTimeLimitLabel(nullptr)
    , m_whiteTimeLimitTitleLabel(nullptr)
    , m_blackTimeLimitSlider(nullptr)
    , m_blackTimeLimitLabel(nullptr)
    , m_blackTimeLimitTitleLabel(nullptr)
    , m_incrementSlider(nullptr)
    , m_incrementLabel(nullptr)
    , m_incrementTitleLabel(nullptr)
    , m_whiteTimeLabel(nullptr)
    , m_blackTimeLabel(nullptr)
    , m_whiteTimeProgressBar(nullptr)
    , m_blackTimeProgressBar(nullptr)
    , m_startButton(nullptr)
    , m_gameTimer(nullptr)
    , m_whiteTimeMs(0)
    , m_blackTimeMs(0)
    , m_whiteInitialTimeMs(0)
    , m_blackInitialTimeMs(0)
    , m_incrementMs(0)
    , m_timeControlEnabled(false)
    , m_timerStarted(false)
    , m_serverTimeOffset(0)
    , m_gameStartLocalTime(0)
    , m_currentTurnStartTime(0)
    , m_serverTimeA(0)
    , m_serverTimeB(0)
    , m_serverCurrentPlayer("White")
    , m_serverLastSwitchTime(0)
    , m_useServerTimer(false)
    , m_lastServerUpdateTime(0)
    , m_boardContainer(nullptr)
    , m_timeControlPanel(nullptr)
    , m_contentLayout(nullptr)
    , m_rightStretchIndex(-1)
    , m_moveListWidget(nullptr)
    , m_exportPGNButton(nullptr)
    , m_copyPGNButton(nullptr)
    , m_moveListPanel(nullptr)
    , m_capturedWhitePanel(nullptr)
    , m_capturedBlackPanel(nullptr)
    , m_whiteScoreDiffLabel(nullptr)
    , m_blackScoreDiffLabel(nullptr)
    , m_rightTimePanel(nullptr)
    , m_topEndGamePanel(nullptr)
    , m_bottomEndGamePanel(nullptr)
    , m_replayTitle(nullptr)
    , m_replayFirstButton(nullptr)
    , m_replayPrevButton(nullptr)
    , m_replayNextButton(nullptr)
    , m_replayLastButton(nullptr)
    , m_isReplayMode(false)
    , m_replayMoveIndex(-1)
    , m_savedCurrentPlayer(PieceColor::White)
    , m_chessEngine(nullptr)
    , m_humanModeButton(nullptr)
    , m_computerModeButton(nullptr)
    , m_gameModeStatusLabel(nullptr)
    , m_currentGameMode(GameMode::HumanVsHuman)
    , m_colorSelectionWidget(nullptr)
    , m_whiteButton(nullptr)
    , m_randomButton(nullptr)
    , m_blackButton(nullptr)
    , m_isRandomColorSelected(false)
    , m_difficultySlider(nullptr)
    , m_difficultyLabel(nullptr)
    , m_difficultyValueLabel(nullptr)
    , m_thinkingLabel(nullptr)
    , m_networkManager(nullptr)
    , m_onlineModeButton(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_roomInfoLabel(nullptr)
    , m_isOnlineGame(false)
    , m_waitingForOpponent(false)
    , m_onlineHostSelectedColor(PieceColor::White)
    , m_newGameAction(nullptr)
    , m_bgmPlayer(nullptr)
    , m_bgmEnabled(true)
    , m_bgmVolume(30)
    , m_lastBgmIndex(-1)
    , m_animationOverlay(nullptr)
    , m_animationLabel(nullptr)
    , m_animationSubLabel(nullptr)
    , m_animationTimer(nullptr)
    , m_animationStep(0)
    , m_pendingGameStart(false)
    , m_startupAnimationTimer(nullptr)
    , m_startupAnimationStep(0)
    , m_fadeAnimation(nullptr)
    , m_scaleAnimation(nullptr)
    , m_opacityEffect(nullptr)
    , m_updateChecker(nullptr)
    , m_manualUpdateCheck(false)
{
    ui->setupUi(this);
    setWindowTitle("♔ 國際象棋 - 科技對弈 ♚");
    resize(900, 660);  // 增加寬度以容納時間控制面板

    // 設置最小視窗大小以確保所有內容都能完整顯示而不被裁切
    // 最小寬度：允許棋盤至少 8*MIN_SQUARE_SIZE 加上面板和邊距
    // 最小高度：允許棋盤和控制項合理顯示
    setMinimumSize(814, 420);

    setMouseTracking(true);
    
    // 應用現代科技風格全局樣式表
    applyModernStylesheet();

    loadSoundSettings();
    initializeSounds();
    initializeBackgroundMusic();  // 初始化背景音樂
    loadPieceIconSettings();
    loadBoardColorSettings();
    loadBoardFlipSettings();
    loadPieceIconsToCache(); // 載入設定後將圖示載入快取
    setupMenuBar();
    setupUI();
    loadTimeControlSettings();  // 在 setupUI() 之後載入以確保元件存在
    loadEngineSettings();  // 載入引擎設定
    initializeEngine();  // 初始化棋局引擎
    initializeNetwork(); // 初始化網路管理器
    updateBoard();
    updateStatus();
    updateTimeDisplays();
    updateReplayButtons();  // 設置回放按鈕初始狀態
    updateCapturedPiecesDisplay();  // 初始化被吃掉棋子顯示
    
    // 初始化更新檢查器
    m_updateChecker = new UpdateChecker(this);
    connect(m_updateChecker, &UpdateChecker::updateCheckFinished, 
            this, &Qt_Chess::onUpdateCheckFinished);
    connect(m_updateChecker, &UpdateChecker::updateCheckFailed, 
            this, &Qt_Chess::onUpdateCheckFailed);
    
    // 啟動後自動檢查更新（延遲以免干擾啟動動畫）
    QTimer::singleShot(UPDATE_CHECK_DELAY_MS, this, [this]() {
        m_updateChecker->checkForUpdates();
    });
    
    // 在視窗顯示後播放啟動動畫
    QTimer::singleShot(100, this, &Qt_Chess::playStartupAnimation);
}

Qt_Chess::~Qt_Chess()
{
    // 停止並清理背景音樂
    if (m_bgmPlayer) {
        m_bgmPlayer->stop();
        delete m_bgmPlayer;
        m_bgmPlayer = nullptr;
    }
    
    // 停止並清理棋局引擎
    if (m_chessEngine) {
        m_chessEngine->stopEngine();
        delete m_chessEngine;
        m_chessEngine = nullptr;
    }
    delete ui;
}

void Qt_Chess::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 為棋盤和時間控制創建水平佈局
    m_contentLayout = new QHBoxLayout();

    // 左側棋譜面板
    m_moveListPanel = new QWidget(this);
    m_moveListPanel->setMinimumWidth(MIN_PANEL_WIDTH);  // 限制最小寬度
    m_moveListPanel->setMaximumWidth(MAX_PANEL_WIDTH);  // 限制最大寬度
    QVBoxLayout* moveListLayout = new QVBoxLayout(m_moveListPanel);
    moveListLayout->setContentsMargins(5, 5, 5, 5);

    // 棋譜標題 - 現代科技風格
    QLabel* moveListTitle = new QLabel("📜 棋譜記錄", m_moveListPanel);
    moveListTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    moveListTitle->setFont(titleFont);
    moveListTitle->setStyleSheet(QString(
        "QLabel { "
        "  color: %1; "
        "  padding: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 transparent, stop:0.5 rgba(0, 217, 255, 0.2), stop:1 transparent); "
        "  border-bottom: 2px solid %1; "
        "}"
    ).arg(THEME_ACCENT_PRIMARY));
    moveListLayout->addWidget(moveListTitle);

    m_moveListWidget = new QListWidget(m_moveListPanel);
    m_moveListWidget->setAlternatingRowColors(true);
    connect(m_moveListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int row = m_moveListWidget->row(item);
        const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
        // 每行包含兩步（白方和黑方），點擊某行會跳到該行的最後一步
        int moveIndex = row * 2 + 1;
        // 確保索引不超出範圍
        if (moveIndex >= static_cast<int>(moveHistory.size())) {
            moveIndex = moveHistory.size() - 1;
        }
        enterReplayMode();
        replayToMove(moveIndex);
    });
    moveListLayout->addWidget(m_moveListWidget);

    // 匯出PGN按鈕（初始隱藏）- 現代科技風格
    m_exportPGNButton = new QPushButton("📤 匯出 PGN", m_moveListPanel);
    m_exportPGNButton->hide();
    connect(m_exportPGNButton, &QPushButton::clicked, this, &Qt_Chess::onExportPGNClicked);
    moveListLayout->addWidget(m_exportPGNButton);

    // 複製棋譜按鈕（初始隱藏）- 現代科技風格
    m_copyPGNButton = new QPushButton("📋 複製棋譜", m_moveListPanel);
    m_copyPGNButton->hide();
    connect(m_copyPGNButton, &QPushButton::clicked, this, &Qt_Chess::onCopyPGNClicked);
    moveListLayout->addWidget(m_copyPGNButton);

    // 回放控制按鈕標題 - 現代科技風格
    m_replayTitle = new QLabel("🎬 回放控制", m_moveListPanel);
    m_replayTitle->setAlignment(Qt::AlignCenter);
    QFont replayFont;
    replayFont.setPointSize(11);
    replayFont.setBold(true);
    m_replayTitle->setFont(replayFont);
    m_replayTitle->setStyleSheet(QString(
        "QLabel { "
        "  color: %1; "
        "  padding: 6px; "
        "  margin-top: 5px; "
        "}"
    ).arg(THEME_ACCENT_WARNING));
    moveListLayout->addWidget(m_replayTitle);

    // 回放按鈕容器
    QWidget* replayButtonContainer = new QWidget(m_moveListPanel);
    QGridLayout* replayButtonLayout = new QGridLayout(replayButtonContainer);
    replayButtonLayout->setContentsMargins(0, 0, 0, 0);
    replayButtonLayout->setSpacing(4);

    // 回放按鈕樣式 - 現代科技風格霓虹效果
    QString replayButtonStyle = QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2); "
        "  color: %3; "
        "  border: 2px solid %4; "
        "  border-radius: 6px; "
        "  padding: 6px 10px; "
        "  font-size: 14px; "
        "  font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:0.5 rgba(255, 217, 61, 0.3), stop:1 %2); "
        "  border-color: %5; "
        "  color: %5; "
        "}"
        "QPushButton:pressed { "
        "  background: %5; "
        "  color: %2; "
        "}"
        "QPushButton:disabled { "
        "  background: rgba(30, 30, 50, 0.5); "
        "  color: #555; "
        "  border-color: #333; "
        "}"
    ).arg(THEME_BG_PANEL, THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_BORDER, THEME_ACCENT_WARNING);

    m_replayFirstButton = new QPushButton("⏮", replayButtonContainer);
    m_replayFirstButton->setToolTip("第一步");
    m_replayFirstButton->setEnabled(false);  // 初始停用
    m_replayFirstButton->setStyleSheet(replayButtonStyle);
    connect(m_replayFirstButton, &QPushButton::clicked, this, &Qt_Chess::onReplayFirstClicked);
    replayButtonLayout->addWidget(m_replayFirstButton, 0, 0);

    m_replayPrevButton = new QPushButton("◀", replayButtonContainer);
    m_replayPrevButton->setToolTip("上一步");
    m_replayPrevButton->setEnabled(false);  // 初始停用
    m_replayPrevButton->setStyleSheet(replayButtonStyle);
    connect(m_replayPrevButton, &QPushButton::clicked, this, &Qt_Chess::onReplayPrevClicked);
    replayButtonLayout->addWidget(m_replayPrevButton, 0, 1);

    m_replayNextButton = new QPushButton("▶", replayButtonContainer);
    m_replayNextButton->setToolTip("下一步");
    m_replayNextButton->setEnabled(false);  // 初始停用
    m_replayNextButton->setStyleSheet(replayButtonStyle);
    connect(m_replayNextButton, &QPushButton::clicked, this, &Qt_Chess::onReplayNextClicked);
    replayButtonLayout->addWidget(m_replayNextButton, 0, 2);

    m_replayLastButton = new QPushButton("⏭", replayButtonContainer);
    m_replayLastButton->setToolTip("最後一步");
    m_replayLastButton->setEnabled(false);  // 初始停用
    m_replayLastButton->setStyleSheet(replayButtonStyle);
    connect(m_replayLastButton, &QPushButton::clicked, this, &Qt_Chess::onReplayLastClicked);
    replayButtonLayout->addWidget(m_replayLastButton, 0, 3);

    moveListLayout->addWidget(replayButtonContainer);

    // 左側棋譜面板 - 固定寬度，不參與水平伸展
    m_contentLayout->addWidget(m_moveListPanel, 1);  // 固定寬度不伸展

    // 添加左側伸展以保持棋盤居中並吸收多餘空間
    m_contentLayout->addStretch(0);

    // 棋盤容器 - 使用垂直佈局以在棋盤上方和下方放置被吃棋子
    m_boardContainer = new QWidget(this);
    m_boardContainer->setMouseTracking(true);
    QVBoxLayout* boardContainerVLayout = new QVBoxLayout(m_boardContainer);;
    boardContainerVLayout->setContentsMargins(BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN,
                                             BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN);
    boardContainerVLayout->setSpacing(5);

    // 遊戲結束時對方的時間和吃子紀錄面板（棋盤上方，初始隱藏）
    m_topEndGamePanel = new QWidget(m_boardContainer);
    m_topEndGamePanel->setMaximumHeight(50);  // 最大高度，容納時間標籤（40px）加上邊距
    m_topEndGamePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QHBoxLayout* topEndGameLayout = new QHBoxLayout(m_topEndGamePanel);
    topEndGameLayout->setContentsMargins(5, 5, 5, 5);
    topEndGameLayout->setSpacing(10);
    topEndGameLayout->setAlignment(Qt::AlignLeft);  // 靠左對齊
    m_topEndGamePanel->hide();  // 初始隱藏
    boardContainerVLayout->addWidget(m_topEndGamePanel, 0);

    // 國際象棋棋盤（水平佈局以保持居中）
    QHBoxLayout* boardHLayout = new QHBoxLayout();
    boardHLayout->setContentsMargins(0, 0, 0, 0);
    boardHLayout->setSpacing(0);

    m_boardWidget = new QWidget(m_boardContainer);
    m_boardWidget->setMouseTracking(true);
    QGridLayout* gridLayout = new QGridLayout(m_boardWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(2, 2, 2, 2);  // 所有邊添加 2px 邊距以防止邊框被裁切

    m_squares.resize(8, std::vector<QPushButton*>(8));

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = new QPushButton(m_boardWidget);
            square->setMinimumSize(MIN_SQUARE_SIZE, MIN_SQUARE_SIZE);  // 設置最小大小以匹配 updateSquareSizes()
            square->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            square->setMouseTracking(true);

            QFont buttonFont;
            buttonFont.setPointSize(36);
            square->setFont(buttonFont);

            m_squares[row][col] = square;
            gridLayout->addWidget(square, row, col);

            // 儲存按鈕坐標以便在 eventFilter 中高效查找
            m_buttonCoordinates[square] = QPoint(col, row);

            // 安裝事件過濾器以捕獲拖放的滑鼠事件
            square->installEventFilter(this);

            connect(square, &QPushButton::clicked, this, [this, row, col]() {
                onSquareClicked(row, col);
            });

            updateSquareColor(row, col);
        }
    }

    // 將棋盤添加到水平佈局
    boardHLayout->addWidget(m_boardWidget, 1, Qt::AlignCenter);
    boardContainerVLayout->addLayout(boardHLayout, 1);

    // 遊戲結束時我方的時間和吃子紀錄面板（棋盤下方，初始隱藏）
    m_bottomEndGamePanel = new QWidget(m_boardContainer);
    m_bottomEndGamePanel->setMaximumHeight(50);  // 最大高度，容納時間標籤（40px）加上邊距
    m_bottomEndGamePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QHBoxLayout* bottomEndGameLayout = new QHBoxLayout(m_bottomEndGamePanel);
    bottomEndGameLayout->setContentsMargins(5, 5, 5, 5);
    bottomEndGameLayout->setSpacing(10);
    bottomEndGameLayout->setAlignment(Qt::AlignLeft);  // 靠左對齊
    m_bottomEndGamePanel->hide();  // 初始隱藏
    boardContainerVLayout->addWidget(m_bottomEndGamePanel, 0);

    // 將棋盤容器添加到內容佈局
    // 使用較大的伸展因子(3)使棋盤在水平放大時優先擴展
    m_contentLayout->addWidget(m_boardContainer, 2, Qt::AlignCenter);

    // 右側時間顯示面板（在棋盤和時間控制之間）
    // 佈局順序：對方吃子紀錄（上方垂直往下）-> 時間顯示區 -> 我方吃子紀錄（從時間垂直往下）
    m_rightTimePanel = new QWidget(this);
    m_rightTimePanel->setMinimumWidth(100);
    m_rightTimePanel->setMaximumWidth(150);
    QVBoxLayout* rightTimePanelLayout = new QVBoxLayout(m_rightTimePanel);
    rightTimePanelLayout->setContentsMargins(5, 5, 5, 5);
    rightTimePanelLayout->setSpacing(5);

    // 對方的吃子紀錄從右側棋盤上方垂直往下（白子被黑方吃掉）
    m_capturedWhitePanel = new QWidget(m_rightTimePanel);
    m_capturedWhitePanel->setMinimumWidth(30);
    m_capturedWhitePanel->setMinimumHeight(100);
    rightTimePanelLayout->addWidget(m_capturedWhitePanel, 1);

    // 時間顯示字體
    QFont timeFont;
    timeFont.setPointSize(14);
    timeFont.setBold(true);

    // 黑方時間進度條 - 放在時間標籤上方，初始隱藏
    m_blackTimeProgressBar = new QProgressBar(m_rightTimePanel);
    m_blackTimeProgressBar->setMinimum(0);
    m_blackTimeProgressBar->setMaximum(100);
    m_blackTimeProgressBar->setValue(100);
    m_blackTimeProgressBar->setTextVisible(false);
    m_blackTimeProgressBar->setFixedWidth(100);  // 與時間標籤同寬
    m_blackTimeProgressBar->setMaximumHeight(8);
    m_blackTimeProgressBar->setStyleSheet(QString(
        "QProgressBar { border: 2px solid %1; border-radius: 4px; background-color: %2; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "  stop:0 %3, stop:1 %4); border-radius: 2px; }"
    ).arg(THEME_BORDER, THEME_BG_DARK, THEME_ACCENT_PRIMARY, THEME_ACCENT_SUCCESS));
    m_blackTimeProgressBar->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_blackTimeProgressBar, 0, Qt::AlignCenter);

    // 黑方時間標籤 - 現代科技風格數位顯示
    m_blackTimeLabel = new QLabel("--:--", m_rightTimePanel);
    m_blackTimeLabel->setFont(timeFont);
    m_blackTimeLabel->setAlignment(Qt::AlignCenter);
    m_blackTimeLabel->setStyleSheet(QString(
        "QLabel { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(15, 52, 96, 0.95), stop:1 rgba(26, 26, 46, 0.95)); "
        "  color: %1; "
        "  padding: 10px; "
        "  border: 2px solid %2; "
        "  border-radius: 8px; "
        "  font-family: 'Consolas', 'Monaco', monospace; "
        "}"
    ).arg(THEME_ACCENT_PRIMARY, THEME_BORDER));
    m_blackTimeLabel->setFixedSize(110, 45);  // 稍大的固定大小
    m_blackTimeLabel->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_blackTimeLabel, 0, Qt::AlignCenter);

    // 白方時間標籤 - 現代科技風格數位顯示
    m_whiteTimeLabel = new QLabel("--:--", m_rightTimePanel);
    m_whiteTimeLabel->setFont(timeFont);
    m_whiteTimeLabel->setAlignment(Qt::AlignCenter);
    m_whiteTimeLabel->setStyleSheet(QString(
        "QLabel { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(15, 52, 96, 0.95), stop:1 rgba(26, 26, 46, 0.95)); "
        "  color: %1; "
        "  padding: 10px; "
        "  border: 2px solid %2; "
        "  border-radius: 8px; "
        "  font-family: 'Consolas', 'Monaco', monospace; "
        "}"
    ).arg(THEME_ACCENT_SUCCESS, THEME_BORDER));
    m_whiteTimeLabel->setFixedSize(110, 45);  // 稍大的固定大小
    m_whiteTimeLabel->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_whiteTimeLabel, 0, Qt::AlignCenter);

    // 白方時間進度條 - 現代科技風格漸變
    m_whiteTimeProgressBar = new QProgressBar(m_rightTimePanel);
    m_whiteTimeProgressBar->setMinimum(0);
    m_whiteTimeProgressBar->setMaximum(100);
    m_whiteTimeProgressBar->setValue(100);
    m_whiteTimeProgressBar->setTextVisible(false);
    m_whiteTimeProgressBar->setFixedWidth(100);  // 與時間標籤同寬
    m_whiteTimeProgressBar->setMaximumHeight(8);
    m_whiteTimeProgressBar->setStyleSheet(QString(
        "QProgressBar { border: 2px solid %1; border-radius: 4px; background-color: %2; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "  stop:0 %3, stop:1 %4); border-radius: 2px; }"
    ).arg(THEME_BORDER, THEME_BG_DARK, THEME_ACCENT_SUCCESS, THEME_ACCENT_PRIMARY));
    m_whiteTimeProgressBar->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_whiteTimeProgressBar, 0, Qt::AlignCenter);

    // 我方的吃子紀錄從時間垂直往下（黑子被白方吃掉）
    m_capturedBlackPanel = new QWidget(m_rightTimePanel);
    m_capturedBlackPanel->setMinimumWidth(30);
    m_capturedBlackPanel->setMinimumHeight(100);
    rightTimePanelLayout->addWidget(m_capturedBlackPanel, 1);

    // 將右側時間面板添加到內容佈局
    m_contentLayout->addWidget(m_rightTimePanel, 0);

    // 添加右側伸展以保持棋盤居中並吸收多餘空間
    m_rightStretchIndex = m_contentLayout->count();  // 記錄伸展項的索引
    m_contentLayout->addStretch(0);

    // 時間控制的右側面板 - 固定寬度，不參與水平伸展
    m_timeControlPanel = new QWidget(this);
    m_timeControlPanel->setMinimumWidth(MIN_PANEL_WIDTH);  // 限制最小寬度
    m_timeControlPanel->setMaximumWidth(MAX_PANEL_WIDTH);  // 限制最大寬度
    QVBoxLayout* rightPanelLayout = new QVBoxLayout(m_timeControlPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    setupTimeControlUI(rightPanelLayout);
    m_contentLayout->addWidget(m_timeControlPanel, 1);  // 固定寬度不伸展

    mainLayout->addLayout(m_contentLayout);

    setCentralWidget(centralWidget);
}

void Qt_Chess::setupMenuBar() {
    m_menuBar = menuBar();

    // 遊戲選單
    QMenu* gameMenu = m_menuBar->addMenu("🎮 遊戲");

    // 新遊戲動作
    m_newGameAction = new QAction("🔄 新遊戲", this);
    connect(m_newGameAction, &QAction::triggered, this, &Qt_Chess::onNewGameClicked);
    gameMenu->addAction(m_newGameAction);

    gameMenu->addSeparator();

    // 放棄動作
    QAction* giveUpAction = new QAction("🏳 放棄", this);
    connect(giveUpAction, &QAction::triggered, this, &Qt_Chess::onGiveUpClicked);
    gameMenu->addAction(giveUpAction);

    // 設定選單
    QMenu* settingsMenu = m_menuBar->addMenu("⚙ 設定");

    // 音效設定動作
    QAction* soundSettingsAction = new QAction("🔊 音效設定", this);
    connect(soundSettingsAction, &QAction::triggered, this, &Qt_Chess::onSoundSettingsClicked);
    settingsMenu->addAction(soundSettingsAction);

    // 棋子圖標設定動作
    QAction* pieceIconSettingsAction = new QAction("♟ 棋子圖標設定", this);
    connect(pieceIconSettingsAction, &QAction::triggered, this, &Qt_Chess::onPieceIconSettingsClicked);
    settingsMenu->addAction(pieceIconSettingsAction);

    // 棋盤顏色設定動作
    QAction* boardColorSettingsAction = new QAction("🎨 棋盤顏色設定", this);
    connect(boardColorSettingsAction, &QAction::triggered, this, &Qt_Chess::onBoardColorSettingsClicked);
    settingsMenu->addAction(boardColorSettingsAction);

    settingsMenu->addSeparator();

    // 反轉棋盤動作
    QAction* flipBoardAction = new QAction("🔃 反轉棋盤", this);
    connect(flipBoardAction, &QAction::triggered, this, &Qt_Chess::onFlipBoardClicked);
    settingsMenu->addAction(flipBoardAction);

    // 切換全螢幕動作
    QAction* toggleFullScreenAction = new QAction("📺 切換全螢幕", this);
    connect(toggleFullScreenAction, &QAction::triggered, this, &Qt_Chess::onToggleFullScreenClicked);
    settingsMenu->addAction(toggleFullScreenAction);
    
    settingsMenu->addSeparator();
    
    // 背景音樂開關動作
    QAction* toggleBgmAction = new QAction("🎵 背景音樂", this);
    toggleBgmAction->setCheckable(true);
    toggleBgmAction->setChecked(m_bgmEnabled);
    connect(toggleBgmAction, &QAction::triggered, this, &Qt_Chess::onToggleBackgroundMusicClicked);
    settingsMenu->addAction(toggleBgmAction);
    
    // 說明選單
    QMenu* helpMenu = m_menuBar->addMenu("❓ 說明");
    
    // 檢查更新動作
    QAction* checkUpdateAction = new QAction("🔄 檢查更新", this);
    connect(checkUpdateAction, &QAction::triggered, this, &Qt_Chess::onCheckForUpdatesClicked);
    helpMenu->addAction(checkUpdateAction);
}

void Qt_Chess::updateSquareColor(int displayRow, int displayCol) {
    // 計算邏輯坐標以確定正確的淺色/深色模式
    int logicalRow = getLogicalRow(displayRow);
    int logicalCol = getLogicalCol(displayCol);
    bool isLight = (logicalRow + logicalCol) % 2 == 0;
    QColor color = isLight ? m_boardColorSettings.lightSquareColor : m_boardColorSettings.darkSquareColor;
    // 現代科技風格 - 帶有微妙的內發光邊框效果
    m_squares[displayRow][displayCol]->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid rgba(0, 217, 255, 0.3); }").arg(color.name())
        );
}

void Qt_Chess::updateBoard() {
    for (int logicalRow = 0; logicalRow < 8; ++logicalRow) {
        for (int logicalCol = 0; logicalCol < 8; ++logicalCol) {
            int displayRow = getDisplayRow(logicalRow);
            int displayCol = getDisplayCol(logicalCol);
            const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
            displayPieceOnSquare(m_squares[displayRow][displayCol], piece);
            updateSquareColor(displayRow, displayCol);
        }
    }

    // 高亮上一步移動的格子
    applyLastMoveHighlight();
    // 如果被將軍，將國王高亮為紅色
    applyCheckHighlight();
    // 如果選擇了棋子，重新應用高亮
    if (m_pieceSelected) {
        highlightValidMoves();
    }
    
    // 更新被吃掉的棋子顯示
    updateCapturedPiecesDisplay();
}

void Qt_Chess::updateStatus() {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";

    if (m_chessBoard.isCheckmate(currentPlayer)) {
        // 記錄將死結果
        if (currentPlayer == PieceColor::White) {
            m_chessBoard.setGameResult(GameResult::BlackWins);
        } else {
            m_chessBoard.setGameResult(GameResult::WhiteWins);
        }
        handleGameEnd();
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        QMessageBox::information(this, "遊戲結束", QString("將死！%1獲勝！").arg(winner));
    } else if (m_chessBoard.isStalemate(currentPlayer)) {
        m_chessBoard.setGameResult(GameResult::Draw);
        handleGameEnd();
        QMessageBox::information(this, "遊戲結束", "逼和！對局和棋。");
    } else if (m_chessBoard.isInsufficientMaterial()) {
        m_chessBoard.setGameResult(GameResult::Draw);
        handleGameEnd();
        QMessageBox::information(this, "遊戲結束", "子力不足以將死！對局和棋。");
    }
}

void Qt_Chess::applyCheckHighlight(const QPoint& excludeSquare) {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (m_chessBoard.isInCheck(currentPlayer)) {
        QPoint kingPos = m_chessBoard.findKing(currentPlayer);
        if (kingPos.x() >= 0 && kingPos.y() >= 0 && kingPos != excludeSquare) {
            int logicalRow = kingPos.y();
            int logicalCol = kingPos.x();
            int displayRow = getDisplayRow(logicalRow);
            int displayCol = getDisplayCol(logicalCol);
            m_squares[displayRow][displayCol]->setStyleSheet(CHECK_HIGHLIGHT_STYLE);
        }
    }
}

void Qt_Chess::applyLastMoveHighlight() {
    // 如果沒有上一步移動，則不高亮
    if (m_lastMoveFrom.x() < 0 || m_lastMoveTo.x() < 0) {
        return;
    }
    
    // 高亮「從」格子（黃色）
    int fromDisplayRow = getDisplayRow(m_lastMoveFrom.y());
    int fromDisplayCol = getDisplayCol(m_lastMoveFrom.x());
    bool fromIsLight = (m_lastMoveFrom.y() + m_lastMoveFrom.x()) % 2 == 0;
    QString fromColor = fromIsLight ? LAST_MOVE_LIGHT_COLOR : LAST_MOVE_DARK_COLOR;
    m_squares[fromDisplayRow][fromDisplayCol]->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid #333; }").arg(fromColor)
    );
    
    // 高亮「到」格子（黃色）
    int toDisplayRow = getDisplayRow(m_lastMoveTo.y());
    int toDisplayCol = getDisplayCol(m_lastMoveTo.x());
    bool toIsLight = (m_lastMoveTo.y() + m_lastMoveTo.x()) % 2 == 0;
    QString toColor = toIsLight ? LAST_MOVE_LIGHT_COLOR : LAST_MOVE_DARK_COLOR;
    m_squares[toDisplayRow][toDisplayCol]->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid #333; }").arg(toColor)
    );
}

void Qt_Chess::clearHighlights() {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            updateSquareColor(row, col);
        }
    }

    // 重新應用上一步移動的高亮
    applyLastMoveHighlight();
    // 如果被將軍，重新應用國王的紅色背景
    applyCheckHighlight();
}

void Qt_Chess::highlightValidMoves() {
    clearHighlights();

    if (!m_pieceSelected) return;

    // 高亮選中的格子（m_selectedSquare 是邏輯坐標）- 現代科技風格霓虹綠
    int displayRow = getDisplayRow(m_selectedSquare.y());
    int displayCol = getDisplayCol(m_selectedSquare.x());
    m_squares[displayRow][displayCol]->setStyleSheet(
        QString("QPushButton { background-color: rgba(0, 255, 136, 0.6); border: 3px solid %1; }").arg(THEME_ACCENT_SUCCESS)
        );

    // 高亮有效的移動
    for (int logicalRow = 0; logicalRow < 8; ++logicalRow) {
        for (int logicalCol = 0; logicalCol < 8; ++logicalCol) {
            QPoint targetSquare(logicalCol, logicalRow);
            if (m_chessBoard.isValidMove(m_selectedSquare, targetSquare)) {
                bool isCapture = isCaptureMove(m_selectedSquare, targetSquare);
                int displayRow = getDisplayRow(logicalRow);
                int displayCol = getDisplayCol(logicalCol);
                // 使用邏輯坐標確定淺色/深色格子
                bool isLight = (logicalRow + logicalCol) % 2 == 0;

                if (isCapture) {
                    // 將吃子移動高亮為霓虹紅/粉色
                    QString color = isLight ? "rgba(255, 100, 120, 0.7)" : "rgba(233, 69, 96, 0.8)";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 3px solid %2; }").arg(color, THEME_ACCENT_SECONDARY)
                        );
                } else {
                    // 將非吃子移動高亮為霓虹黃色
                    QString color = isLight ? "rgba(255, 217, 61, 0.5)" : "rgba(255, 217, 61, 0.7)";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 3px solid %2; }").arg(color, THEME_ACCENT_WARNING)
                        );
                }
            }
        }
    }

    // 如果被將軍且國王不是選中的棋子，重新應用國王的紅色背景
    applyCheckHighlight(m_selectedSquare);
}

void Qt_Chess::onSquareClicked(int displayRow, int displayCol) {
    qDebug() << "[Qt_Chess::onSquareClicked] Square clicked at display position (" << displayRow << "," << displayCol << ")"
             << "| m_gameStarted:" << m_gameStarted
             << "| m_isOnlineGame:" << m_isOnlineGame;
    
    // 如果在回放模式中，不允許移動
    if (m_isReplayMode) {
        qDebug() << "[Qt_Chess::onSquareClicked] In replay mode, ignoring click";
        return;
    }

    // 如果遊戲尚未開始，不允許移動
    if (!m_gameStarted) {
        qDebug() << "[Qt_Chess::onSquareClicked] Game not started, ignoring click";
        return;
    }

    // 如果是電腦的回合，玩家不能移動
    if (isComputerTurn()) {
        qDebug() << "[Qt_Chess::onSquareClicked] Computer's turn, ignoring click";
        return;
    }
    
    // 如果是線上模式且不是本地玩家回合，不能移動
    if (m_isOnlineGame && !isOnlineTurn()) {
        qDebug() << "[Qt_Chess::onSquareClicked] Not player's turn in online game"
                 << "| Current player:" << (int)m_chessBoard.getCurrentPlayer()
                 << "| Network player color:" << (m_networkManager ? (int)m_networkManager->getPlayerColor() : -1);
        return;
    }

    // 將顯示坐標轉換為邏輯坐標
    int logicalRow = getLogicalRow(displayRow);
    int logicalCol = getLogicalCol(displayCol);
    QPoint clickedSquare(logicalCol, logicalRow);

    if (!m_pieceSelected) {
        // 嘗試選擇一個棋子
        const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
        if (piece.getType() != PieceType::None &&
            piece.getColor() == m_chessBoard.getCurrentPlayer() &&
            isPlayerPiece(piece.getColor())) {  // 檢查是否為玩家的棋子
            m_selectedSquare = clickedSquare;
            m_pieceSelected = true;
            highlightValidMoves();
        }
    } else {
        // 在執行移動之前檢測移動類型
        bool isCapture = isCaptureMove(m_selectedSquare, clickedSquare);
        bool isCastling = isCastlingMove(m_selectedSquare, clickedSquare);

        // 嘗試移動選中的棋子
        if (m_chessBoard.movePiece(m_selectedSquare, clickedSquare)) {
            // 記錄上一步移動用於高亮顯示
            m_lastMoveFrom = m_selectedSquare;
            m_lastMoveTo = clickedSquare;
            
            m_pieceSelected = false;
            
            // 記錄 UCI 格式的移動
            PieceType promType = PieceType::None;
            
            // 檢查是否為第一步棋，如果是且計時器未啟動，則啟動計時器
            bool isFirstMove = m_uciMoveHistory.isEmpty();
            if (isFirstMove && m_timeControlEnabled && !m_timerStarted) {
                m_timerStarted = true;
                m_gameStartLocalTime = QDateTime::currentMSecsSinceEpoch();  // 記錄遊戲開始時間
                m_currentTurnStartTime = m_gameStartLocalTime;  // 記錄當前回合開始時間
                startTimer();
                qDebug() << "[Qt_Chess] Timer started after first move";
            }
            
            // 為剛完成移動的玩家應用時間增量
            applyIncrement();

            updateBoard();

            // 檢查是否需要兵升變
            if (m_chessBoard.needsPromotion(clickedSquare)) {
                const ChessPiece& piece = m_chessBoard.getPiece(clickedSquare.y(), clickedSquare.x());
                PieceType promotionType = showPromotionDialog(piece.getColor());
                m_chessBoard.promotePawn(clickedSquare, promotionType);
                promType = promotionType;
                updateBoard();
            }
            
            // 記錄 UCI 移動
            QString uciMove = ChessEngine::moveToUCI(m_selectedSquare, clickedSquare, promType);
            m_uciMoveHistory.append(uciMove);

            // 更新棋譜列表
            updateMoveList();

            // 播放適當的音效
            playSoundForMove(isCapture, isCastling);

            // 更新時間顯示（計時器僅在已啟動時運行）
            updateTimeDisplays();

            updateStatus();
            
            // 如果是線上模式，發送移動給對手
            if (m_isOnlineGame && m_networkManager) {
                qDebug() << "[Qt_Chess] Sending move to opponent: from" << m_lastMoveFrom << "to" << m_lastMoveTo;
                m_networkManager->sendMove(m_lastMoveFrom, m_lastMoveTo, promType);
            }
            
            // 如果現在是電腦的回合，請求引擎走棋
            if (isComputerTurn() && m_gameStarted) {
                // 使用短暫延遲讓 UI 更新
                QTimer::singleShot(100, this, &Qt_Chess::requestEngineMove);
            }
        } else if (clickedSquare == m_selectedSquare) {
            // 取消選擇棋子
            m_pieceSelected = false;
            clearHighlights();
        } else {
            // 嘗試選擇相同顏色的另一個棋子
            const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
            if (piece.getType() != PieceType::None &&
                piece.getColor() == m_chessBoard.getCurrentPlayer() &&
                isPlayerPiece(piece.getColor())) {  // 檢查是否為玩家的棋子
                m_selectedSquare = clickedSquare;
                highlightValidMoves();
            }
        }
    }
}

void Qt_Chess::onNewGameClicked() {
    // 如果在回放模式中，先退出
    if (m_isReplayMode) {
        exitReplayMode();
    }

    m_chessBoard.initializeBoard();
    m_pieceSelected = false;
    m_gameStarted = false;  // 重置遊戲開始狀態
    m_uciMoveHistory.clear();  // 清空 UCI 移動歷史
    
    // 停止背景音樂（遊戲未開始）
    stopBackgroundMusic();
    
    // 重置上一步移動高亮
    m_lastMoveFrom = QPoint(-1, -1);
    m_lastMoveTo = QPoint(-1, -1);

    // 重置時間控制
    stopTimer();
    m_timerStarted = false;
    
    // 停止引擎思考
    if (m_chessEngine) {
        m_chessEngine->stop();
        m_chessEngine->newGame();
    }

    // 將時間和吃子紀錄恢復到右側面板
    restoreWidgetsFromGameEnd();

    // 顯示時間控制面板
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }

    // 隱藏時間顯示和進度條
    if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
    if (m_blackTimeLabel) m_blackTimeLabel->hide();
    if (m_whiteTimeProgressBar) m_whiteTimeProgressBar->hide();
    if (m_blackTimeProgressBar) m_blackTimeProgressBar->hide();

    // 隱藏放棄按鈕
    if (m_giveUpButton) m_giveUpButton->hide();

    // 隱藏匯出 PGN 按鈕和複製棋譜按鈕
    if (m_exportPGNButton) m_exportPGNButton->hide();
    if (m_copyPGNButton) m_copyPGNButton->hide();

    // 清空棋譜列表
    if (m_moveListWidget) m_moveListWidget->clear();

    // 根據滑桿值重置時間
    if (m_whiteTimeLimitSlider) {
        m_whiteTimeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
    }

    if (m_blackTimeLimitSlider) {
        m_blackTimeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
    }

    // 檢查是否啟用時間控制
    m_timeControlEnabled = (m_whiteTimeMs > 0 || m_blackTimeMs > 0);

    // 重置棋盤後啟用開始按鈕
    if (m_startButton) {
        m_startButton->setEnabled(true);
        m_startButton->setText("開始");
    }

    updateBoard();
    updateStatus();
    updateTimeDisplays();

    // 更新回放按鈕狀態（新遊戲沒有移動歷史）
    updateReplayButtons();

    // 當遊戲還沒開始時，將右側伸展設為 0
    setRightPanelStretch(0);

    // 清除任何殘留的高亮顯示（例如選中的棋子、有效移動、將軍警告）
    clearHighlights();
}

void Qt_Chess::onGiveUpClicked() {
    // 顯示確認對話框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "確認放棄",
        "你確定要放棄這局遊戲嗎？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // 在線上模式下，通知對手投降
        if (m_isOnlineGame && m_networkManager) {
            m_networkManager->sendSurrender();
        }
        
        // 記錄認輸結果
        PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
        if (currentPlayer == PieceColor::White) {
            m_chessBoard.setGameResult(GameResult::WhiteResigns);
        } else {
            m_chessBoard.setGameResult(GameResult::BlackResigns);
        }

        // 處理遊戲結束的通用邏輯
        handleGameEnd();

        // 顯示放棄者的訊息
        QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        QMessageBox::information(this, "遊戲結束", QString("%1放棄！%2獲勝！").arg(playerName).arg(winner));

        // 不再自動進入回放模式，用戶可以根據需要點擊回放按鈕
    }
}

void Qt_Chess::onStartButtonClicked() {
    // 檢查是否在線上模式且尚未連接
    if (m_isOnlineGame && m_waitingForOpponent) {
        QMessageBox::warning(this, "無法開始", "請等待對手連接後再開始遊戲");
        return;
    }
    
    // 在線上模式下，通知對手開始遊戲（包含時間設定）
    if (m_isOnlineGame && m_networkManager) {
        // 獲取當前的時間設定
        int whiteTimeMs = 0;
        int blackTimeMs = 0;
        int incrementMs = 0;
        
        if (m_whiteTimeLimitSlider) {
            whiteTimeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
        }
        if (m_blackTimeLimitSlider) {
            blackTimeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
        }
        if (m_incrementSlider) {
            incrementMs = m_incrementSlider->value() * 1000;  // 轉換為毫秒
        }
        
        // 房主需要在發送開始遊戲訊息前設定自己的玩家顏色
        // 這樣 isPlayerPiece() 才能正確判斷哪些棋子可以移動
        if (m_networkManager->getRole() == NetworkRole::Host) {
            m_networkManager->setPlayerColors(m_onlineHostSelectedColor);
        }
        
        m_networkManager->sendStartGame(whiteTimeMs, blackTimeMs, incrementMs, m_onlineHostSelectedColor);
        
        qDebug() << "[Qt_Chess::onStartButtonClicked] Host sending StartGame to server"
                 << "| Host color:" << (m_onlineHostSelectedColor == PieceColor::White ? "White" : "Black")
                 << "| Waiting for server gameStart broadcast...";
        
        // 房主也等待伺服器廣播的 gameStart 訊息，確保與房客同步開始
        // 不再在這裡直接啟動遊戲，而是等待 onStartGameReceived() 被觸發
        m_gameStarted = false;
        m_timerStarted = false;
        
        // 隱藏開始按鈕，顯示等待訊息
        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("等待同步開始...");
        }
        
        // 不繼續執行下面的初始化，等待伺服器回應
        return;
    }
    
    // 清空 UCI 移動歷史
    m_uciMoveHistory.clear();
    
    // 通知引擎開始新遊戲
    if (m_chessEngine) {
        m_chessEngine->newGame();
    }
    
    // 根據選擇的遊戲模式決定是否翻轉棋盤
    // 當玩家執黑時（ComputerVsHuman 或 線上模式中房主執黑），棋盤應該翻轉使黑棋在下方
    GameMode mode = getCurrentGameMode();
    bool shouldFlip = (mode == GameMode::ComputerVsHuman) || 
                      (mode == GameMode::OnlineGame && 
                       m_networkManager && m_networkManager->getRole() == NetworkRole::Host &&
                       m_onlineHostSelectedColor == PieceColor::Black);
    if (m_isBoardFlipped != shouldFlip) {
        m_isBoardFlipped = shouldFlip;
        saveBoardFlipSettings();
    }
    
    // 確保時間和吃子紀錄在右側面板（遊戲開始時恢復到正常位置）
    restoreWidgetsFromGameEnd();
    
    // 顯示右側時間面板
    if (m_rightTimePanel) {
        m_rightTimePanel->show();
    }
    
    if (m_timeControlEnabled && !m_timerStarted) {
        // 重置棋盤到初始狀態
        resetBoardState();

        // 清空棋譜列表
        if (m_moveListWidget) {
            m_moveListWidget->clear();
        }

        // 根據滑桿值重置時間
        if (m_whiteTimeLimitSlider) {
            m_whiteTimeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
            m_whiteInitialTimeMs = m_whiteTimeMs;  // 記錄初始時間用於進度條
        }

        if (m_blackTimeLimitSlider) {
            m_blackTimeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
            m_blackInitialTimeMs = m_blackTimeMs;  // 記錄初始時間用於進度條
        }

        // 不在這裡啟動計時器，等待第一步棋走出後再開始計時
        // m_timerStarted 保持為 false，直到第一步棋走出
        m_gameStarted = true;  // 非線上模式立即啟動（線上模式不會執行到這裡）
        
        // 不調用 startTimer()，等待第一步棋

        // 隱藏時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }

        // 在棋盤左右兩側顯示時間和進度條
        if (m_whiteTimeLabel && m_blackTimeLabel) {
            m_whiteTimeLabel->show();
            m_blackTimeLabel->show();
        }
        if (m_whiteTimeProgressBar && m_blackTimeProgressBar) {
            m_whiteTimeProgressBar->show();
            m_blackTimeProgressBar->show();
        }

        // 顯示放棄按鈕
        if (m_giveUpButton) {
            m_giveUpButton->show();
        }
        
        // 在線上模式下顯示退出房間按鈕
        if (m_isOnlineGame && m_exitRoomButton) {
            m_exitRoomButton->show();
        }

        updateTimeDisplays();

        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("進行中");
        }

        // 更新回放按鈕狀態（遊戲開始時停用）
        updateReplayButtons();

        // 當遊戲開始時，將右側伸展設為 1
        setRightPanelStretch(1);
        
        // 播放遊戲開始動畫
        m_pendingGameStart = isComputerTurn();  // 標記是否需要在動畫後請求引擎走棋
        playGameStartAnimation();
    } else if (!m_timeControlEnabled && !m_gameStarted) {
        // 重置棋盤到初始狀態（即使沒有時間控制）
        resetBoardState();

        // 清空棋譜列表
        if (m_moveListWidget) {
            m_moveListWidget->clear();
        }

        // 重置時間值為 0（無限制）
        m_whiteTimeMs = 0;
        m_blackTimeMs = 0;

        m_gameStarted = true;  // 非線上模式立即啟動（線上模式不會執行到這裡）

        // 隱藏時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }

        // 顯示放棄按鈕
        if (m_giveUpButton) {
            m_giveUpButton->show();
        }
        
        // 在線上模式下顯示退出房間按鈕
        if (m_isOnlineGame && m_exitRoomButton) {
            m_exitRoomButton->show();
        }

        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("進行中");
        }

        // 更新回放按鈕狀態（遊戲開始時停用）
        updateReplayButtons();

        // 當遊戲開始時，將右側伸展設為 1
        setRightPanelStretch(1);
        
        // 播放遊戲開始動畫
        m_pendingGameStart = isComputerTurn();  // 標記是否需要在動畫後請求引擎走棋
        playGameStartAnimation();
    } else if (m_isOnlineGame) {
        // 線上模式但不符合上述條件時（例如 m_timerStarted 已為 true），
        // 需要重置棋盤並延遲啟用走棋以確保對手已準備好接收移動
        
        // 重置棋盤到初始狀態
        resetBoardState();
        
        // 清空棋譜列表
        if (m_moveListWidget) {
            m_moveListWidget->clear();
        }
        
        // 如果有時間控制，根據滑桿值重置時間
        if (m_timeControlEnabled) {
            if (m_whiteTimeLimitSlider) {
                m_whiteTimeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
                m_whiteInitialTimeMs = m_whiteTimeMs;
            }
            if (m_blackTimeLimitSlider) {
                m_blackTimeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
                m_blackInitialTimeMs = m_blackTimeMs;
            }
            
            // 啟動計時器
            startTimer();
            
            // 顯示時間和進度條
            if (m_whiteTimeLabel && m_blackTimeLabel) {
                m_whiteTimeLabel->show();
                m_blackTimeLabel->show();
            }
            if (m_whiteTimeProgressBar && m_blackTimeProgressBar) {
                m_whiteTimeProgressBar->show();
                m_blackTimeProgressBar->show();
            }
        } else {
            // 無時間控制，重置時間值為 0
            m_whiteTimeMs = 0;
            m_blackTimeMs = 0;
        }
        
        // 隱藏時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }
        
        // 顯示放棄按鈕
        if (m_giveUpButton) {
            m_giveUpButton->show();
        }
        
        // 顯示退出房間按鈕
        if (m_exitRoomButton) {
            m_exitRoomButton->show();
        }
        
        // 延遲啟用走棋
        QTimer::singleShot(200, this, [this]() {
            m_gameStarted = true;
            qDebug() << "[Qt_Chess] Host: m_gameStarted set to true (timer path 3 - edge case fix)";
        });
        
        updateTimeDisplays();
        
        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("進行中");
        }
        
        // 更新回放按鈕狀態（遊戲開始時停用）
        updateReplayButtons();
        
        // 當遊戲開始時，將右側伸展設為 1
        setRightPanelStretch(1);
        
        // 播放遊戲開始動畫
        m_pendingGameStart = isComputerTurn();
        playGameStartAnimation();
    }
}

void Qt_Chess::onSoundSettingsClicked() {
    SoundSettingsDialog dialog(this);
    dialog.setSettings(m_soundSettings);

    if (dialog.exec() == QDialog::Accepted) {
        m_soundSettings = dialog.getSettings();
        applySoundSettings();
    }
}

void Qt_Chess::onPieceIconSettingsClicked() {
    PieceIconSettingsDialog dialog(this);
    dialog.setSettings(m_pieceIconSettings);

    if (dialog.exec() == QDialog::Accepted) {
        m_pieceIconSettings = dialog.getSettings();
        applyPieceIconSettings();
    }
}

void Qt_Chess::onBoardColorSettingsClicked() {
    BoardColorSettingsDialog dialog(this);
    dialog.setSettings(m_boardColorSettings);

    if (dialog.exec() == QDialog::Accepted) {
        m_boardColorSettings = dialog.getSettings();
        applyBoardColorSettings();
    }
}

PieceType Qt_Chess::showPromotionDialog(PieceColor color) {
    QDialog dialog(this);
    dialog.setWindowTitle("兵升變");
    dialog.setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* label = new QLabel("選擇升變的棋子：", &dialog);
    QFont font = label->font();
    font.setPointSize(12);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // 為每個升變選項創建按鈕
    struct PromotionOption {
        PieceType type;
        QString symbol;
    };

    std::vector<PromotionOption> options = {
        {PieceType::Queen, color == PieceColor::White ? "♕" : "♛"},
        {PieceType::Rook, color == PieceColor::White ? "♖" : "♜"},
        {PieceType::Bishop, color == PieceColor::White ? "♗" : "♝"},
        {PieceType::Knight, color == PieceColor::White ? "♘" : "♞"}
    };

    PieceType selectedType = PieceType::Queen; // 預設為后

    for (const auto& option : options) {
        QPushButton* button = new QPushButton(option.symbol, &dialog);
        button->setMinimumSize(80, 80);
        QFont buttonFont;
        buttonFont.setPointSize(36);
        button->setFont(buttonFont);

        connect(button, &QPushButton::clicked, [&dialog, &selectedType, option]() {
            selectedType = option.type;
            dialog.accept();
        });

        buttonLayout->addWidget(button);
    }

    layout->addLayout(buttonLayout);

    dialog.exec();
    return selectedType;
}

QPoint Qt_Chess::getSquareAtPosition(const QPoint& pos) const {
    if (!m_boardWidget) return QPoint(-1, -1);

    QPoint boardPos = m_boardWidget->mapFrom(this, pos);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = m_squares[row][col];
            if (square->geometry().contains(boardPos)) {
                return QPoint(col, row);
            }
        }
    }

    return QPoint(-1, -1);
}

void Qt_Chess::restorePieceToSquare(const QPoint& logicalSquare) {
    if (logicalSquare.x() >= 0 && logicalSquare.y() >= 0 && logicalSquare.x() < 8 && logicalSquare.y() < 8) {
        const ChessPiece& piece = m_chessBoard.getPiece(logicalSquare.y(), logicalSquare.x());
        int displayRow = getDisplayRow(logicalSquare.y());
        int displayCol = getDisplayCol(logicalSquare.x());
        displayPieceOnSquare(m_squares[displayRow][displayCol], piece);
    }
}

bool Qt_Chess::eventFilter(QObject *obj, QEvent *event) {
    // 檢查事件是否來自我們的棋盤格子按鈕之一
    QPushButton* button = qobject_cast<QPushButton*>(obj);
    if (!button) {
        return QMainWindow::eventFilter(obj, event);
    }

    // 使用高效的映射查找檢查此按鈕是否為我們的棋盤格子之一
    if (!m_buttonCoordinates.contains(button)) {
        return QMainWindow::eventFilter(obj, event);
    }

    // 轉發滑鼠事件以啟用拖放
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // 將按鈕的位置映射到主視窗的坐標系統
        QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
        QPoint windowPos = mapFromGlobal(globalPos);
        QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(),
                                mouseEvent->buttons(), mouseEvent->modifiers());
        mousePressEvent(&mappedEvent);
        // 不要完全接受事件 - 如果沒有開始拖動，讓按鈕仍然處理點擊
        if (m_isDragging) {
            return true; // 事件已處理，開始拖動
        }
    } else if (event->type() == QEvent::MouseMove) {
        if (m_isDragging) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
            QPoint windowPos = mapFromGlobal(globalPos);
            QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(),
                                    mouseEvent->buttons(), mouseEvent->modifiers());
            mouseMoveEvent(&mappedEvent);
            return true; // 事件已處理
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (m_isDragging) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
            QPoint windowPos = mapFromGlobal(globalPos);
            QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(),
                                    mouseEvent->buttons(), mouseEvent->modifiers());
            mouseReleaseEvent(&mappedEvent);
            return true; // 事件已處理
        }
    }

    // 將事件傳遞給父類進行標準處理
    return QMainWindow::eventFilter(obj, event);
}

void Qt_Chess::mousePressEvent(QMouseEvent *event) {
    // 如果在回放模式中，左鍵點擊棋盤會退出回放模式
    if (m_isReplayMode) {
        if (event->button() == Qt::LeftButton) {
            QPoint displaySquare = getSquareAtPosition(event->pos());
            // 檢查點擊是否在棋盤範圍內
            if (displaySquare.x() >= 0 && displaySquare.y() >= 0 &&
                displaySquare.x() < 8 && displaySquare.y() < 8) {
                exitReplayMode();
                return;
            }
        }
        QMainWindow::mousePressEvent(event);
        return;
    }

    QPoint displaySquare = getSquareAtPosition(event->pos());

    // 右鍵點擊 - 取消任何當前動作
    if (event->button() == Qt::RightButton) {
        if (m_isDragging) {
            // 取消拖動並將棋子返回原位
            m_isDragging = false;
            if (m_dragLabel) {
                m_dragLabel->hide();
                m_dragLabel->deleteLater();
                m_dragLabel = nullptr;
            }
            // 將棋子恢復到原始格子
            restorePieceToSquare(m_dragStartSquare);
            m_dragStartSquare = QPoint(-1, -1);
            clearHighlights();
        } else if (m_pieceSelected) {
            // 如果選擇了棋子則取消選擇
            m_pieceSelected = false;
            clearHighlights();
        }
        return;
    }

    // 如果遊戲尚未開始，不允許拖動
    if (!m_gameStarted) {
        return;
    }

    // 左鍵點擊 - 開始拖動
    if (event->button() == Qt::LeftButton && displaySquare.x() >= 0 && displaySquare.y() >= 0 &&
        displaySquare.x() < 8 && displaySquare.y() < 8) {
        // 轉換 display coordinates to logical coordinates
        int logicalRow = getLogicalRow(displaySquare.y());
        int logicalCol = getLogicalCol(displaySquare.x());
        QPoint logicalSquare(logicalCol, logicalRow);

        const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
        if (piece.getType() != PieceType::None &&
            piece.getColor() == m_chessBoard.getCurrentPlayer() &&
            isPlayerPiece(piece.getColor())) {  // 檢查是否為玩家的棋子

            // 追蹤這個棋子在拖動前是否已被選中
            m_wasSelectedBeforeDrag = (m_pieceSelected && m_selectedSquare == logicalSquare);

            m_isDragging = true;
            m_dragStartSquare = logicalSquare;
            m_selectedSquare = logicalSquare;
            m_pieceSelected = true;

            // 創建 drag label
            m_dragLabel = new QLabel(this);

            // 使用自訂圖示或 Unicode 符號
            if (m_pieceIconSettings.useCustomIcons) {
                QPixmap pixmap = getCachedPieceIcon(piece.getType(), piece.getColor());
                if (!pixmap.isNull()) {
                    QPushButton* squareButton = m_squares[displaySquare.y()][displaySquare.x()];
                    int iconSize = calculateIconSize(squareButton);
                    m_dragLabel->setPixmap(pixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    m_dragLabel->setText(piece.getSymbol());
                    QFont font;
                    font.setPointSize(36);
                    m_dragLabel->setFont(font);
                }
            } else {
                m_dragLabel->setText(piece.getSymbol());
                QFont font;
                font.setPointSize(36);
                m_dragLabel->setFont(font);
            }

            m_dragLabel->setStyleSheet("QLabel { background-color: transparent; border: none; }");
            m_dragLabel->adjustSize();
            m_dragLabel->move(event->pos() - QPoint(m_dragLabel->width() / 2, m_dragLabel->height() / 2));
            m_dragLabel->show();
            m_dragLabel->raise();

            // 隱藏 the piece from the original square during drag
            m_squares[displaySquare.y()][displaySquare.x()]->setText("");
            m_squares[displaySquare.y()][displaySquare.x()]->setIcon(QIcon());

            highlightValidMoves();
        }
    }

    QMainWindow::mousePressEvent(event);
}

void Qt_Chess::mouseMoveEvent(QMouseEvent *event) {
    // 如果在回放模式中，不處理拖動
    if (m_isReplayMode) {
        QMainWindow::mouseMoveEvent(event);
        return;
    }

    if (m_isDragging && m_dragLabel) {
        m_dragLabel->move(event->pos() - QPoint(m_dragLabel->width() / 2, m_dragLabel->height() / 2));
    }

    QMainWindow::mouseMoveEvent(event);
}

void Qt_Chess::mouseReleaseEvent(QMouseEvent *event) {
    // 如果在回放模式中，不處理滑鼠事件
    if (m_isReplayMode) {
        QMainWindow::mouseReleaseEvent(event);
        return;
    }

    // 右鍵點擊 - 已在 mousePressEvent 中處理
    if (event->button() == Qt::RightButton) {
        QMainWindow::mouseReleaseEvent(event);
        return;
    }

    // 左鍵釋放 - 完成拖動
    if (event->button() == Qt::LeftButton && m_isDragging) {
        QPoint displayDropSquare = getSquareAtPosition(event->pos());

        // 清理拖動標籤
        if (m_dragLabel) {
            m_dragLabel->hide();
            m_dragLabel->deleteLater();
            m_dragLabel = nullptr;
        }

        m_isDragging = false;

        if (displayDropSquare.x() >= 0 && displayDropSquare.y() >= 0) {
            // 轉換 display coordinates to logical coordinates
            int logicalRow = getLogicalRow(displayDropSquare.y());
            int logicalCol = getLogicalCol(displayDropSquare.x());
            QPoint logicalDropSquare(logicalCol, logicalRow);

            // 在執行移動之前檢測移動類型
            bool isCapture = isCaptureMove(m_dragStartSquare, logicalDropSquare);
            bool isCastling = isCastlingMove(m_dragStartSquare, logicalDropSquare);

            // 嘗試移動棋子
            if (m_chessBoard.movePiece(m_dragStartSquare, logicalDropSquare)) {
                // 記錄上一步移動用於高亮顯示
                m_lastMoveFrom = m_dragStartSquare;
                m_lastMoveTo = logicalDropSquare;
                
                m_pieceSelected = false;
                
                // 記錄 UCI 格式的移動
                PieceType promType = PieceType::None;

                // 應用 time increment for the player who just moved
                applyIncrement();

                updateBoard();

                // 檢查 pawn promotion is needed
                if (m_chessBoard.needsPromotion(logicalDropSquare)) {
                    const ChessPiece& piece = m_chessBoard.getPiece(logicalDropSquare.y(), logicalDropSquare.x());
                    PieceType promotionType = showPromotionDialog(piece.getColor());
                    m_chessBoard.promotePawn(logicalDropSquare, promotionType);
                    promType = promotionType;
                    updateBoard();
                }
                
                // 記錄 UCI 移動
                QString uciMove = ChessEngine::moveToUCI(m_dragStartSquare, logicalDropSquare, promType);
                m_uciMoveHistory.append(uciMove);

                // 更新棋譜列表
                updateMoveList();

                // 播放適當的音效
                playSoundForMove(isCapture, isCastling);

                // 更新 time displays (timer only runs if already started)
                updateTimeDisplays();

                updateStatus();
                clearHighlights();
                
                // 如果是線上模式，發送移動給對手
                if (m_isOnlineGame && m_networkManager) {
                    qDebug() << "[Qt_Chess] Sending move to opponent (drag): from" << m_lastMoveFrom << "to" << m_lastMoveTo;
                    m_networkManager->sendMove(m_lastMoveFrom, m_lastMoveTo, promType);
                }
                
                // 如果現在是電腦的回合，請求引擎走棋
                if (isComputerTurn() && m_gameStarted) {
                    // 使用短暫延遲讓 UI 更新
                    QTimer::singleShot(100, this, &Qt_Chess::requestEngineMove);
                }
            } else if (logicalDropSquare == m_dragStartSquare) {
                // 放在同一格子上 - 切換選擇
                // 將棋子恢復到原始格子
                restorePieceToSquare(m_dragStartSquare);

                if (m_wasSelectedBeforeDrag) {
                    // 已經被選中，所以取消選擇
                    m_pieceSelected = false;
                    clearHighlights();
                } else {
                    // 未被選中，所以保持選中並顯示高亮
                    m_selectedSquare = m_dragStartSquare;
                    m_pieceSelected = true;
                    highlightValidMoves();
                }
            } else {
                // 無效的移動 - 嘗試選擇不同的棋子
                const ChessPiece& piece = m_chessBoard.getPiece(logicalDropSquare.y(), logicalDropSquare.x());
                if (piece.getType() != PieceType::None &&
                    piece.getColor() == m_chessBoard.getCurrentPlayer()) {
                    // 首先將棋子恢復到原始格子
                    restorePieceToSquare(m_dragStartSquare);
                    m_selectedSquare = logicalDropSquare;
                    m_pieceSelected = true;
                    highlightValidMoves();
                } else {
                    // 無效移動且未選擇另一個棋子
                    // 將棋子恢復到原始格子
                    restorePieceToSquare(m_dragStartSquare);
                    m_pieceSelected = false;
                    clearHighlights();
                }
            }
        } else {
            // 放在棋盤外 - 取消
            // 將棋子恢復到原始格子
            restorePieceToSquare(m_dragStartSquare);
            m_pieceSelected = false;
            clearHighlights();
        }

        m_dragStartSquare = QPoint(-1, -1);
    }

    QMainWindow::mouseReleaseEvent(event);
}

void Qt_Chess::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateSquareSizes();
    updateTimeControlSizes();  // 更新 time control UI sizes when window is resized

    // 調整大小後重新應用高亮
    if (m_pieceSelected) {
        highlightValidMoves();
    }
    
    // 如果動畫疊加層正在顯示，更新其大小以匹配新視窗大小
    if (m_animationOverlay && m_animationOverlay->isVisible()) {
        QRect windowRect = rect();
        m_animationOverlay->setGeometry(windowRect);
        if (m_animationLabel) {
            m_animationLabel->setGeometry(0, 0, windowRect.width(), windowRect.height());
        }
    }
}

void Qt_Chess::keyPressEvent(QKeyEvent *event) {
    // ESC 鍵：退出全螢幕
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            showNormal();
            event->accept();
            return;
        }
    }

    // 檢查是否在回放模式或有棋譜可回放
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
    if (moveHistory.empty()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    // 處理左右箭頭鍵
    if (event->key() == Qt::Key_Left) {
        // 左箭頭：上一步（只有在按鈕啟用時才處理）
        if (m_replayPrevButton && m_replayPrevButton->isEnabled()) {
            onReplayPrevClicked();
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Right) {
        // 右箭頭：下一步（只有在按鈕啟用時才處理）
        if (m_replayNextButton && m_replayNextButton->isEnabled()) {
            onReplayNextClicked();
            event->accept();
            return;
        }
    }

    // 其他按鍵傳遞給基類處理
    QMainWindow::keyPressEvent(event);
}

void Qt_Chess::updateSquareSizes() {
    if (!m_boardWidget || m_squares.empty()) return;

    // 獲取 the central widget
    QWidget* central = centralWidget();
    if (!central) return;

    // 計算 available space for the board
    // 考慮帶有左右面板和面板間距的水平佈局
    int reservedWidth = 0;
    int reservedHeight = 0;

    // 考慮左側面板的實際寬度（棋譜面板）- 總是可見
    reservedWidth += getPanelWidth(m_moveListPanel);

    // 如果可見則考慮右側面板的實際寬度（時間控制面板）
    if (m_timeControlPanel && m_timeControlPanel->isVisible()) {
        reservedWidth += getPanelWidth(m_timeControlPanel);
    }

    // 考慮右側時間面板的寬度（時間和被吃棋子面板）
    if (m_rightTimePanel && m_rightTimePanel->isVisible()) {
        reservedWidth += getPanelWidth(m_rightTimePanel);
    }

    // 添加佈局間距和邊距
    reservedWidth += BASE_MARGINS * 4;  // 適度的邊距

    // 為佈局邊距和間距添加一些填充
    reservedHeight += BASE_MARGINS * 2;  // 上下各一邊的邊距

    // 考慮選單欄高度（如果存在）以防止全螢幕時棋盤被裁切
    if (m_menuBar && m_menuBar->isVisible()) {
        reservedHeight += m_menuBar->height();
    }

    int availableWidth = central->width() - reservedWidth;
    int availableHeight = central->height() - reservedHeight;

    // 計算 the size for each square (use the smaller dimension to keep squares square)
    int squareSize = qMin(availableWidth, availableHeight) / 8;

    // 確保最小和合理的最大大小
    squareSize = qMax(squareSize, MIN_SQUARE_SIZE);  // 使用常數作為最小大小
    squareSize = qMin(squareSize, MAX_SQUARE_SIZE);  // 限制在合理的最大值

    // 計算 font size based on square size (approximately 45% of square size)
    int fontSize = squareSize * 9 / 20;  // 這對於 80px 的格子大約給出 36pt
    fontSize = qMax(fontSize, 12);  // 確保最小可讀字體大小
    fontSize = qMin(fontSize, 54);  // 限制非常大棋盤的字體大小

    // 更新 all squares
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = m_squares[row][col];
            square->setFixedSize(squareSize, squareSize);

            QFont font = square->font();
            font.setPointSize(fontSize);
            square->setFont(font);

            // 更新 icon size if using custom icons
            if (m_pieceIconSettings.useCustomIcons && !square->icon().isNull()) {
                // 確保縮放在有效範圍內（60-100）
                int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
                int iconSize = static_cast<int>(squareSize * scale / 100.0);
                square->setIconSize(QSize(iconSize, iconSize));
            }
        }
    }

    // 更新 the board widget size to fit the squares exactly
    // 添加 4 個額外像素（每側 2px）以防止格子高亮時邊框被裁切
    m_boardWidget->setFixedSize(squareSize * 8 + 4, squareSize * 8 + 4);

    // 更新 time label font sizes to scale with board size
    if (m_whiteTimeLabel && m_blackTimeLabel) {
        int timeFontSize = qMax(MIN_UI_FONT_SIZE, qMin(MAX_UI_FONT_SIZE, squareSize / UI_FONT_SCALE_DIVISOR));
        QFont timeFont = m_whiteTimeLabel->font();
        timeFont.setPointSize(timeFontSize);
        timeFont.setBold(true);
        m_whiteTimeLabel->setFont(timeFont);
        m_blackTimeLabel->setFont(timeFont);

        // 更新 time label minimum heights proportionally
        int timeLabelHeight = qMax(MIN_TIME_LABEL_HEIGHT, qMin(MAX_TIME_LABEL_HEIGHT, squareSize / 2));
        m_whiteTimeLabel->setMinimumHeight(timeLabelHeight);
        m_blackTimeLabel->setMinimumHeight(timeLabelHeight);
    }
}

void Qt_Chess::updateTimeControlSizes() {
    if (!m_boardWidget || m_squares.empty()) return;

    // 獲取 a reference square size to base scaling on
    int squareSize = m_squares[0][0]->width();
    if (squareSize <= 0) {
        squareSize = m_squares[0][0]->minimumWidth();
        if (squareSize <= 0) {
            squareSize = MIN_SQUARE_SIZE;
        }
    }

    // 計算 font sizes based on square size
    int controlLabelFontSize = qMax(MIN_TIME_CONTROL_FONT, qMin(MAX_TIME_CONTROL_FONT, squareSize / TIME_CONTROL_FONT_DIVISOR));

    // 更新 time control panel label fonts
    QFont controlLabelFont;
    controlLabelFont.setPointSize(controlLabelFontSize);

    if (m_whiteTimeLimitTitleLabel) m_whiteTimeLimitTitleLabel->setFont(controlLabelFont);
    if (m_whiteTimeLimitLabel) m_whiteTimeLimitLabel->setFont(controlLabelFont);
    if (m_blackTimeLimitTitleLabel) m_blackTimeLimitTitleLabel->setFont(controlLabelFont);
    if (m_blackTimeLimitLabel) m_blackTimeLimitLabel->setFont(controlLabelFont);
    if (m_incrementTitleLabel) m_incrementTitleLabel->setFont(controlLabelFont);
    if (m_incrementLabel) m_incrementLabel->setFont(controlLabelFont);

    // 更新 slider heights based on square size
    int sliderHeight = qMax(MIN_SLIDER_HEIGHT, qMin(MAX_SLIDER_HEIGHT, squareSize / SLIDER_HEIGHT_DIVISOR));

    // 設置滑桿高度的輔助 lambda
    auto setSliderHeight = [sliderHeight](QSlider* slider) {
        if (slider) {
            slider->setMinimumHeight(sliderHeight);
            slider->setMaximumHeight(sliderHeight + SLIDER_HANDLE_EXTRA);
        }
    };

    setSliderHeight(m_whiteTimeLimitSlider);
    setSliderHeight(m_blackTimeLimitSlider);
    setSliderHeight(m_incrementSlider);

    // 更新 button fonts
    int buttonFontSize = qMax(MIN_BUTTON_FONT, qMin(MAX_BUTTON_FONT, squareSize / BUTTON_FONT_DIVISOR));
    QFont buttonFont;
    buttonFont.setPointSize(buttonFontSize);
    buttonFont.setBold(true);

    if (m_startButton) m_startButton->setFont(buttonFont);
    if (m_giveUpButton) m_giveUpButton->setFont(buttonFont);
}

void Qt_Chess::initializeSounds() {
    applySoundSettings();
}

void Qt_Chess::loadSoundSettings() {
    SoundSettingsDialog::SoundSettings defaults = SoundSettingsDialog::getDefaultSettings();
    QSettings settings("QtChess", "SoundSettings");

    m_soundSettings.moveSound = settings.value("moveSound", defaults.moveSound).toString();
    m_soundSettings.captureSound = settings.value("captureSound", defaults.captureSound).toString();
    m_soundSettings.castlingSound = settings.value("castlingSound", defaults.castlingSound).toString();
    m_soundSettings.checkSound = settings.value("checkSound", defaults.checkSound).toString();
    m_soundSettings.checkmateSound = settings.value("checkmateSound", defaults.checkmateSound).toString();

    m_soundSettings.moveVolume = settings.value("moveVolume", defaults.moveVolume).toDouble();
    m_soundSettings.captureVolume = settings.value("captureVolume", defaults.captureVolume).toDouble();
    m_soundSettings.castlingVolume = settings.value("castlingVolume", defaults.castlingVolume).toDouble();
    m_soundSettings.checkVolume = settings.value("checkVolume", defaults.checkVolume).toDouble();
    m_soundSettings.checkmateVolume = settings.value("checkmateVolume", defaults.checkmateVolume).toDouble();

    m_soundSettings.moveSoundEnabled = settings.value("moveSoundEnabled", defaults.moveSoundEnabled).toBool();
    m_soundSettings.captureSoundEnabled = settings.value("captureSoundEnabled", defaults.captureSoundEnabled).toBool();
    m_soundSettings.castlingSoundEnabled = settings.value("castlingSoundEnabled", defaults.castlingSoundEnabled).toBool();
    m_soundSettings.checkSoundEnabled = settings.value("checkSoundEnabled", defaults.checkSoundEnabled).toBool();
    m_soundSettings.checkmateSoundEnabled = settings.value("checkmateSoundEnabled", defaults.checkmateSoundEnabled).toBool();
    m_soundSettings.allSoundsEnabled = settings.value("allSoundsEnabled", defaults.allSoundsEnabled).toBool();
}

void Qt_Chess::setSoundSource(QSoundEffect& sound, const QString& path) {
    // 設置音效來源並正確處理 URL 的輔助函數
    // - 對於 Qt 資源路徑（qrc:），直接使用 QUrl 建構函數
    // - 對於本地檔案路徑，使用 QUrl::fromLocalFile 進行正確轉換
    if (path.startsWith("qrc:")) {
        sound.setSource(QUrl(path));
    } else {
        sound.setSource(QUrl::fromLocalFile(path));
    }
}

void Qt_Chess::applySoundSettings() {
    // 初始化 sound effects with settings
    setSoundSource(m_moveSound, m_soundSettings.moveSound);
    m_moveSound.setVolume(m_soundSettings.moveVolume);

    setSoundSource(m_captureSound, m_soundSettings.captureSound);
    m_captureSound.setVolume(m_soundSettings.captureVolume);

    setSoundSource(m_castlingSound, m_soundSettings.castlingSound);
    m_castlingSound.setVolume(m_soundSettings.castlingVolume);

    setSoundSource(m_checkSound, m_soundSettings.checkSound);
    m_checkSound.setVolume(m_soundSettings.checkVolume);

    setSoundSource(m_checkmateSound, m_soundSettings.checkmateSound);
    m_checkmateSound.setVolume(m_soundSettings.checkmateVolume);
}

bool Qt_Chess::isCaptureMove(const QPoint& from, const QPoint& to) const {
    const ChessPiece& movingPiece = m_chessBoard.getPiece(from.y(), from.x());
    const ChessPiece& destinationPiece = m_chessBoard.getPiece(to.y(), to.x());

    // 檢查常規吃子
    if (destinationPiece.getType() != PieceType::None &&
        destinationPiece.getColor() != movingPiece.getColor()) {
        return true;
    }

    // 檢查吃過路兵
    if (movingPiece.getType() == PieceType::Pawn &&
        to == m_chessBoard.getEnPassantTarget() &&
        m_chessBoard.getEnPassantTarget().x() >= 0) {
        return true;
    }

    return false;
}

bool Qt_Chess::isCastlingMove(const QPoint& from, const QPoint& to) const {
    const ChessPiece& movingPiece = m_chessBoard.getPiece(from.y(), from.x());

    // 檢查 the moving piece is a king moving 2 squares horizontally
    if (movingPiece.getType() != PieceType::King || abs(to.x() - from.x()) != 2) {
        return false;
    }

    // 驗證移動在正確的起始行（黑方第 0 行，白方第 7 行）
    if (movingPiece.getColor() == PieceColor::White && from.y() == 7 && to.y() == 7) {
        return true;
    }
    if (movingPiece.getColor() == PieceColor::Black && from.y() == 0 && to.y() == 0) {
        return true;
    }

    return false;
}

void Qt_Chess::stopAllSounds() {
    m_moveSound.stop();
    m_captureSound.stop();
    m_castlingSound.stop();
    m_checkSound.stop();
    m_checkmateSound.stop();
}

void Qt_Chess::playSoundForMove(bool isCapture, bool isCastling) {
    // 檢查 all sounds are disabled
    if (!m_soundSettings.allSoundsEnabled) {
        return;
    }

    // 停止 any currently playing sound before playing the new one
    stopAllSounds();

    // 注意：movePiece() 之後，回合已切換，所以 currentPlayer 現在是對手
    PieceColor opponentColor = m_chessBoard.getCurrentPlayer();
    bool opponentInCheck = m_chessBoard.isInCheck(opponentColor);
    bool opponentCheckmate = m_chessBoard.isCheckmate(opponentColor);

    if (opponentCheckmate && m_soundSettings.checkmateSoundEnabled) {
        m_checkmateSound.play();
    } else if (opponentInCheck && m_soundSettings.checkSoundEnabled) {
        m_checkSound.play();
    } else if (isCastling && m_soundSettings.castlingSoundEnabled) {
        m_castlingSound.play();
    } else if (isCapture && m_soundSettings.captureSoundEnabled) {
        m_captureSound.play();
    } else if (m_soundSettings.moveSoundEnabled) {
        m_moveSound.play();
    }
}

void Qt_Chess::loadPieceIconSettings() {
    QSettings settings("Qt_Chess", "ChessGame");

    m_pieceIconSettings.useCustomIcons = settings.value("PieceIcons/useCustomIcons", false).toBool();
    m_pieceIconSettings.iconSetType = static_cast<PieceIconSettingsDialog::IconSetType>(
        settings.value("PieceIcons/iconSetType", static_cast<int>(PieceIconSettingsDialog::IconSetType::Unicode)).toInt()
        );
    // 載入 and validate piece scale (ensure it's within valid range 60-100)
    int loadedScale = settings.value("PieceIcons/pieceScale", 80).toInt();
    m_pieceIconSettings.pieceScale = qBound(60, loadedScale, 100);
    m_pieceIconSettings.whiteKingIcon = settings.value("PieceIcons/whiteKingIcon", "").toString();
    m_pieceIconSettings.whiteQueenIcon = settings.value("PieceIcons/whiteQueenIcon", "").toString();
    m_pieceIconSettings.whiteRookIcon = settings.value("PieceIcons/whiteRookIcon", "").toString();
    m_pieceIconSettings.whiteBishopIcon = settings.value("PieceIcons/whiteBishopIcon", "").toString();
    m_pieceIconSettings.whiteKnightIcon = settings.value("PieceIcons/whiteKnightIcon", "").toString();
    m_pieceIconSettings.whitePawnIcon = settings.value("PieceIcons/whitePawnIcon", "").toString();
    m_pieceIconSettings.blackKingIcon = settings.value("PieceIcons/blackKingIcon", "").toString();
    m_pieceIconSettings.blackQueenIcon = settings.value("PieceIcons/blackQueenIcon", "").toString();
    m_pieceIconSettings.blackRookIcon = settings.value("PieceIcons/blackRookIcon", "").toString();
    m_pieceIconSettings.blackBishopIcon = settings.value("PieceIcons/blackBishopIcon", "").toString();
    m_pieceIconSettings.blackKnightIcon = settings.value("PieceIcons/blackKnightIcon", "").toString();
    m_pieceIconSettings.blackPawnIcon = settings.value("PieceIcons/blackPawnIcon", "").toString();
}

void Qt_Chess::applyPieceIconSettings() {
    QSettings settings("Qt_Chess", "ChessGame");

    settings.setValue("PieceIcons/useCustomIcons", m_pieceIconSettings.useCustomIcons);
    settings.setValue("PieceIcons/iconSetType", static_cast<int>(m_pieceIconSettings.iconSetType));
    // 驗證並儲存棋子縮放（確保在有效範圍 60-100 內）
    int validatedScale = qBound(60, m_pieceIconSettings.pieceScale, 100);
    settings.setValue("PieceIcons/pieceScale", validatedScale);
    settings.setValue("PieceIcons/whiteKingIcon", m_pieceIconSettings.whiteKingIcon);
    settings.setValue("PieceIcons/whiteQueenIcon", m_pieceIconSettings.whiteQueenIcon);
    settings.setValue("PieceIcons/whiteRookIcon", m_pieceIconSettings.whiteRookIcon);
    settings.setValue("PieceIcons/whiteBishopIcon", m_pieceIconSettings.whiteBishopIcon);
    settings.setValue("PieceIcons/whiteKnightIcon", m_pieceIconSettings.whiteKnightIcon);
    settings.setValue("PieceIcons/whitePawnIcon", m_pieceIconSettings.whitePawnIcon);
    settings.setValue("PieceIcons/blackKingIcon", m_pieceIconSettings.blackKingIcon);
    settings.setValue("PieceIcons/blackQueenIcon", m_pieceIconSettings.blackQueenIcon);
    settings.setValue("PieceIcons/blackRookIcon", m_pieceIconSettings.blackRookIcon);
    settings.setValue("PieceIcons/blackBishopIcon", m_pieceIconSettings.blackBishopIcon);
    settings.setValue("PieceIcons/blackKnightIcon", m_pieceIconSettings.blackKnightIcon);
    settings.setValue("PieceIcons/blackPawnIcon", m_pieceIconSettings.blackPawnIcon);

    settings.sync();

    // 載入 icons to cache for improved performance
    loadPieceIconsToCache();

    // 更新 the board to reflect the new settings
    updateBoard();
}

QString Qt_Chess::getPieceIconPath(PieceType type, PieceColor color) const {
    if (type == PieceType::None || color == PieceColor::None) {
        return "";
    }

    if (color == PieceColor::White) {
        switch (type) {
        case PieceType::King:   return m_pieceIconSettings.whiteKingIcon;
        case PieceType::Queen:  return m_pieceIconSettings.whiteQueenIcon;
        case PieceType::Rook:   return m_pieceIconSettings.whiteRookIcon;
        case PieceType::Bishop: return m_pieceIconSettings.whiteBishopIcon;
        case PieceType::Knight: return m_pieceIconSettings.whiteKnightIcon;
        case PieceType::Pawn:   return m_pieceIconSettings.whitePawnIcon;
        default: return "";
        }
    } else {
        switch (type) {
        case PieceType::King:   return m_pieceIconSettings.blackKingIcon;
        case PieceType::Queen:  return m_pieceIconSettings.blackQueenIcon;
        case PieceType::Rook:   return m_pieceIconSettings.blackRookIcon;
        case PieceType::Bishop: return m_pieceIconSettings.blackBishopIcon;
        case PieceType::Knight: return m_pieceIconSettings.blackKnightIcon;
        case PieceType::Pawn:   return m_pieceIconSettings.blackPawnIcon;
        default: return "";
        }
    }
}

void Qt_Chess::displayPieceOnSquare(QPushButton* square, const ChessPiece& piece) {
    if (!square) return;

    // 清除 previous content
    square->setText("");
    square->setIcon(QIcon());

    // 使用圖示或符號顯示棋子
    if (m_pieceIconSettings.useCustomIcons) {
        QPixmap pixmap = getCachedPieceIcon(piece.getType(), piece.getColor());
        if (!pixmap.isNull()) {
            QIcon icon(pixmap);
            square->setIcon(icon);
            // 設置 icon size based on square size
            int iconSize = calculateIconSize(square);
            square->setIconSize(QSize(iconSize, iconSize));
        } else {
            // 如果圖示無法載入或不在快取中則回退到符號
            square->setText(piece.getSymbol());
        }
    } else {
        // 使用 Unicode 符號
        square->setText(piece.getSymbol());
    }
}

void Qt_Chess::loadPieceIconsToCache() {
    clearPieceIconCache();

    if (!m_pieceIconSettings.useCustomIcons) {
        return;
    }

    // 載入 all piece icons into cache
    auto loadIconToCache = [this](const QString& iconPath) {
        if (!iconPath.isEmpty() && !m_pieceIconCache.contains(iconPath) && QFile::exists(iconPath)) {
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                m_pieceIconCache.insert(iconPath, pixmap);
            }
        }
    };

    // 載入 white pieces
    loadIconToCache(m_pieceIconSettings.whiteKingIcon);
    loadIconToCache(m_pieceIconSettings.whiteQueenIcon);
    loadIconToCache(m_pieceIconSettings.whiteRookIcon);
    loadIconToCache(m_pieceIconSettings.whiteBishopIcon);
    loadIconToCache(m_pieceIconSettings.whiteKnightIcon);
    loadIconToCache(m_pieceIconSettings.whitePawnIcon);

    // 載入 black pieces
    loadIconToCache(m_pieceIconSettings.blackKingIcon);
    loadIconToCache(m_pieceIconSettings.blackQueenIcon);
    loadIconToCache(m_pieceIconSettings.blackRookIcon);
    loadIconToCache(m_pieceIconSettings.blackBishopIcon);
    loadIconToCache(m_pieceIconSettings.blackKnightIcon);
    loadIconToCache(m_pieceIconSettings.blackPawnIcon);
}

void Qt_Chess::clearPieceIconCache() {
    m_pieceIconCache.clear();
}

QPixmap Qt_Chess::getCachedPieceIcon(PieceType type, PieceColor color) const {
    QString iconPath = getPieceIconPath(type, color);
    if (!iconPath.isEmpty() && m_pieceIconCache.contains(iconPath)) {
        return m_pieceIconCache.value(iconPath);
    }
    return QPixmap();
}

int Qt_Chess::calculateIconSize(QPushButton* square) const {
    if (!square) return DEFAULT_ICON_SIZE;
    int squareWidth = square->width();
    if (squareWidth <= 0) {
        squareWidth = square->minimumWidth();
        if (squareWidth <= 0) {
            return DEFAULT_ICON_SIZE;
        }
    }
    // 應用 the user-configured scale factor (default 80%)
    // 確保縮放在有效範圍內（60-100）
    int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
    return static_cast<int>(squareWidth * scale / 100.0);
}

void Qt_Chess::loadBoardColorSettings() {
    QSettings settings("Qt_Chess", "BoardColorSettings");

    // 載入 color scheme type with validation
    int schemeInt = settings.value("colorScheme", static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic)).toInt();

    // 驗證方案在有效範圍內
    if (schemeInt < static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic) ||
        schemeInt > static_cast<int>(BoardColorSettingsDialog::ColorScheme::Custom7)) {
        schemeInt = static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic);
    }

    BoardColorSettingsDialog::ColorScheme scheme = static_cast<BoardColorSettingsDialog::ColorScheme>(schemeInt);

    // 載入 colors
    QString lightColorStr = settings.value("lightSquareColor", "#F0D9B5").toString();
    QString darkColorStr = settings.value("darkSquareColor", "#B58863").toString();

    m_boardColorSettings.scheme = scheme;
    m_boardColorSettings.lightSquareColor = QColor(lightColorStr);
    m_boardColorSettings.darkSquareColor = QColor(darkColorStr);

    // 驗證顏色
    if (!m_boardColorSettings.lightSquareColor.isValid()) {
        m_boardColorSettings.lightSquareColor = QColor("#F0D9B5");
    }
    if (!m_boardColorSettings.darkSquareColor.isValid()) {
        m_boardColorSettings.darkSquareColor = QColor("#B58863");
    }
}

void Qt_Chess::applyBoardColorSettings() {
    // 儲存 settings
    QSettings settings("Qt_Chess", "BoardColorSettings");
    settings.setValue("colorScheme", static_cast<int>(m_boardColorSettings.scheme));
    settings.setValue("lightSquareColor", m_boardColorSettings.lightSquareColor.name());
    settings.setValue("darkSquareColor", m_boardColorSettings.darkSquareColor.name());

    // 更新 all squares on the board
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            updateSquareColor(row, col);
        }
    }

    // 如果需要則重新應用高亮
    if (m_pieceSelected) {
        highlightValidMoves();
    }

    // 如果被將軍則重新應用將軍高亮
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (m_chessBoard.isInCheck(currentPlayer)) {
        applyCheckHighlight();
    }
}

void Qt_Chess::loadBoardFlipSettings() {
    QSettings settings("Qt_Chess", "ChessGame");
    m_isBoardFlipped = settings.value("boardFlipped", false).toBool();
}

void Qt_Chess::saveBoardFlipSettings() {
    QSettings settings("Qt_Chess", "ChessGame");
    settings.setValue("boardFlipped", m_isBoardFlipped);
}

int Qt_Chess::getDisplayRow(int logicalRow) const {
    return m_isBoardFlipped ? (7 - logicalRow) : logicalRow;
}

int Qt_Chess::getDisplayCol(int logicalCol) const {
    return m_isBoardFlipped ? (7 - logicalCol) : logicalCol;
}

int Qt_Chess::getLogicalRow(int displayRow) const {
    return m_isBoardFlipped ? (7 - displayRow) : displayRow;
}

int Qt_Chess::getLogicalCol(int displayCol) const {
    return m_isBoardFlipped ? (7 - displayCol) : displayCol;
}

void Qt_Chess::onFlipBoardClicked() {
    m_isBoardFlipped = !m_isBoardFlipped;
    saveBoardFlipSettings();
    updateBoard();
}

void Qt_Chess::onToggleFullScreenClicked() {
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void Qt_Chess::setupTimeControlUI(QVBoxLayout* timeControlPanelLayout) {
    // 時間控制群組框 - 現代科技風格
    QGroupBox* timeControlGroup = new QGroupBox("⏱ 遊戲設定", this);
    QVBoxLayout* timeControlLayout = new QVBoxLayout(timeControlGroup);

    QFont labelFont;
    labelFont.setPointSize(10);

    // 白方時間標籤和滑桿 - 現代科技風格
    m_whiteTimeLimitTitleLabel = new QLabel("♔ 白方時間:", this);
    m_whiteTimeLimitTitleLabel->setFont(labelFont);
    m_whiteTimeLimitTitleLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(THEME_ACCENT_SUCCESS));
    timeControlLayout->addWidget(m_whiteTimeLimitTitleLabel);

    m_whiteTimeLimitLabel = new QLabel("不限時", this);
    m_whiteTimeLimitLabel->setFont(labelFont);
    m_whiteTimeLimitLabel->setAlignment(Qt::AlignCenter);
    m_whiteTimeLimitLabel->setStyleSheet(QString(
        "QLabel { color: %1; padding: 4px; background: rgba(0, 255, 136, 0.1); border-radius: 4px; }"
    ).arg(THEME_ACCENT_SUCCESS));
    timeControlLayout->addWidget(m_whiteTimeLimitLabel);

    // 白方時間的水平滑桿 - 離散值
    // 滑桿位置：0=無限制，1=30秒，2-31=1-30分鐘
    m_whiteTimeLimitSlider = new QSlider(Qt::Horizontal, this);
    m_whiteTimeLimitSlider->setMinimum(0);  // 0 = 無限制
    m_whiteTimeLimitSlider->setMaximum(MAX_SLIDER_POSITION);  // 0（無限制），1（30秒），2-31（1-30分鐘）
    m_whiteTimeLimitSlider->setValue(0);
    m_whiteTimeLimitSlider->setTickPosition(QSlider::TicksBelow);
    m_whiteTimeLimitSlider->setTickInterval(1);
    connect(m_whiteTimeLimitSlider, &QSlider::valueChanged, this, &Qt_Chess::onWhiteTimeLimitChanged);
    timeControlLayout->addWidget(m_whiteTimeLimitSlider);

    // 黑方時間標籤和滑桿 - 現代科技風格
    m_blackTimeLimitTitleLabel = new QLabel("♚ 黑方時間:", this);
    m_blackTimeLimitTitleLabel->setFont(labelFont);
    m_blackTimeLimitTitleLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(THEME_ACCENT_PRIMARY));
    timeControlLayout->addWidget(m_blackTimeLimitTitleLabel);

    m_blackTimeLimitLabel = new QLabel("不限時", this);
    m_blackTimeLimitLabel->setFont(labelFont);
    m_blackTimeLimitLabel->setAlignment(Qt::AlignCenter);
    m_blackTimeLimitLabel->setStyleSheet(QString(
        "QLabel { color: %1; padding: 4px; background: rgba(0, 217, 255, 0.1); border-radius: 4px; }"
    ).arg(THEME_ACCENT_PRIMARY));
    timeControlLayout->addWidget(m_blackTimeLimitLabel);

    // 黑方時間的水平滑桿 - 離散值
    m_blackTimeLimitSlider = new QSlider(Qt::Horizontal, this);
    m_blackTimeLimitSlider->setMinimum(0);  // 0 = 無限制
    m_blackTimeLimitSlider->setMaximum(MAX_SLIDER_POSITION);  // 0（無限制），1（30秒），2-31（1-30分鐘）
    m_blackTimeLimitSlider->setValue(0);
    m_blackTimeLimitSlider->setTickPosition(QSlider::TicksBelow);
    m_blackTimeLimitSlider->setTickInterval(1);
    connect(m_blackTimeLimitSlider, &QSlider::valueChanged, this, &Qt_Chess::onBlackTimeLimitChanged);
    timeControlLayout->addWidget(m_blackTimeLimitSlider);

    // 增量標籤和滑桿 - 現代科技風格
    m_incrementTitleLabel = new QLabel("⏳ 每著加秒:", this);
    m_incrementTitleLabel->setFont(labelFont);
    m_incrementTitleLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(THEME_ACCENT_SECONDARY));
    timeControlLayout->addWidget(m_incrementTitleLabel);

    m_incrementLabel = new QLabel("0秒", this);
    m_incrementLabel->setFont(labelFont);
    m_incrementLabel->setAlignment(Qt::AlignCenter);
    m_incrementLabel->setStyleSheet(QString(
        "QLabel { color: %1; padding: 4px; background: rgba(233, 69, 96, 0.1); border-radius: 4px; }"
    ).arg(THEME_ACCENT_SECONDARY));
    timeControlLayout->addWidget(m_incrementLabel);

    // 增量的水平滑桿 - 填充可用寬度
    m_incrementSlider = new QSlider(Qt::Horizontal, this);
    m_incrementSlider->setMinimum(0);
    m_incrementSlider->setMaximum(60);
    m_incrementSlider->setValue(0);
    m_incrementSlider->setTickPosition(QSlider::TicksBelow);
    m_incrementSlider->setTickInterval(5);
    connect(m_incrementSlider, &QSlider::valueChanged, this, &Qt_Chess::onIncrementChanged);
    timeControlLayout->addWidget(m_incrementSlider);

    // 對弈模式 - 直接接在時間控制下方（同一個群組框內）
    QLabel* gameModeLabel = new QLabel("⚔ 對弈模式:", this);
    gameModeLabel->setFont(labelFont);
    gameModeLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(THEME_ACCENT_PRIMARY));
    timeControlLayout->addWidget(gameModeLabel);
    
    // 使用方塊按鈕選擇雙人或電腦對弈 - 現代科技風格
    QHBoxLayout* gameModeButtonsLayout = new QHBoxLayout();
    
    // 現代科技風格按鈕樣式 - 雙人模式（霓虹綠色）
    QString humanModeStyle = QString(
        "QPushButton { "
        "  border: 2px solid %1; border-radius: 8px; padding: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:1 %3); "
        "  color: %4; font-weight: bold; "
        "}"
        "QPushButton:checked { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %5, stop:1 rgba(0, 255, 136, 0.6)); "
        "  color: %3; border-color: %5; "
        "}"
        "QPushButton:hover { "
        "  border-color: %5; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:0.5 rgba(0, 255, 136, 0.2), stop:1 %3); "
        "}"
    ).arg(THEME_BORDER, THEME_BG_PANEL, THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_ACCENT_SUCCESS);
    
    // 現代科技風格按鈕樣式 - 電腦模式（霓虹青色）
    QString computerModeStyle = QString(
        "QPushButton { "
        "  border: 2px solid %1; border-radius: 8px; padding: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:1 %3); "
        "  color: %4; font-weight: bold; "
        "}"
        "QPushButton:checked { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %5, stop:1 rgba(0, 217, 255, 0.6)); "
        "  color: %3; border-color: %5; "
        "}"
        "QPushButton:hover { "
        "  border-color: %5; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:0.5 rgba(0, 217, 255, 0.2), stop:1 %3); "
        "}"
    ).arg(THEME_BORDER, THEME_BG_PANEL, THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_ACCENT_PRIMARY);
    
    // 雙人對弈按鈕
    m_humanModeButton = new QPushButton("👥 雙人", this);
    m_humanModeButton->setFont(labelFont);
    m_humanModeButton->setCheckable(true);
    m_humanModeButton->setChecked(true);
    m_humanModeButton->setMinimumSize(70, 45);
    m_humanModeButton->setStyleSheet(humanModeStyle);
    connect(m_humanModeButton, &QPushButton::clicked, this, &Qt_Chess::onHumanModeClicked);
    gameModeButtonsLayout->addWidget(m_humanModeButton);
    
    // 電腦對弈按鈕
    m_computerModeButton = new QPushButton("🤖 電腦", this);
    m_computerModeButton->setFont(labelFont);
    m_computerModeButton->setCheckable(true);
    m_computerModeButton->setMinimumSize(70, 45);
    m_computerModeButton->setStyleSheet(computerModeStyle);
    connect(m_computerModeButton, &QPushButton::clicked, this, &Qt_Chess::onComputerModeClicked);
    gameModeButtonsLayout->addWidget(m_computerModeButton);
    
    // 線上對戰按鈕
    m_onlineModeButton = new QPushButton("🌐 線上", this);
    m_onlineModeButton->setFont(labelFont);
    m_onlineModeButton->setCheckable(true);
    m_onlineModeButton->setMinimumSize(70, 45);
    m_onlineModeButton->setStyleSheet(computerModeStyle);
    connect(m_onlineModeButton, &QPushButton::clicked, this, &Qt_Chess::onOnlineModeClicked);
    gameModeButtonsLayout->addWidget(m_onlineModeButton);
    
    timeControlLayout->addLayout(gameModeButtonsLayout);
    
    // 選邊按鈕容器（電腦模式時顯示）
    m_colorSelectionWidget = new QWidget(this);
    QHBoxLayout* colorButtonsLayout = new QHBoxLayout(m_colorSelectionWidget);
    colorButtonsLayout->setContentsMargins(0, 5, 0, 5);
    
    // 統一的按鈕樣式 - 現代科技風格（霓虹粉色選中效果）
    QString colorButtonStyle = QString(
        "QPushButton { "
        "  border: 2px solid %1; border-radius: 6px; padding: 6px; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:1 %3); "
        "  color: %4; font-weight: bold; "
        "}"
        "QPushButton:checked { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %5, stop:1 rgba(233, 69, 96, 0.6)); "
        "  color: white; border-color: %5; "
        "}"
        "QPushButton:hover { "
        "  border-color: %5; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:0.5 rgba(233, 69, 96, 0.2), stop:1 %3); "
        "}"
    ).arg(THEME_BORDER, THEME_BG_PANEL, THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_ACCENT_SECONDARY);
    
    // 執白按鈕
    m_whiteButton = new QPushButton("♔ 執白", this);
    m_whiteButton->setFont(labelFont);
    m_whiteButton->setCheckable(true);
    m_whiteButton->setMinimumSize(55, 38);
    m_whiteButton->setStyleSheet(colorButtonStyle);
    connect(m_whiteButton, &QPushButton::clicked, this, &Qt_Chess::onWhiteColorClicked);
    colorButtonsLayout->addWidget(m_whiteButton);
    
    // 隨機按鈕
    m_randomButton = new QPushButton("🎲 隨機", this);
    m_randomButton->setFont(labelFont);
    m_randomButton->setCheckable(true);
    m_randomButton->setMinimumSize(55, 38);
    m_randomButton->setStyleSheet(colorButtonStyle);
    connect(m_randomButton, &QPushButton::clicked, this, &Qt_Chess::onRandomColorClicked);
    colorButtonsLayout->addWidget(m_randomButton);
    
    // 執黑按鈕
    m_blackButton = new QPushButton("♚ 執黑", this);
    m_blackButton->setFont(labelFont);
    m_blackButton->setCheckable(true);
    m_blackButton->setMinimumSize(55, 38);
    m_blackButton->setStyleSheet(colorButtonStyle);
    connect(m_blackButton, &QPushButton::clicked, this, &Qt_Chess::onBlackColorClicked);
    colorButtonsLayout->addWidget(m_blackButton);
    
    m_colorSelectionWidget->hide();  // 初始隱藏
    timeControlLayout->addWidget(m_colorSelectionWidget);
    
    // 顯示當前選擇的標籤（電腦模式時顯示執白/執黑）
    m_gameModeStatusLabel = new QLabel("", this);
    m_gameModeStatusLabel->setFont(labelFont);
    m_gameModeStatusLabel->setAlignment(Qt::AlignCenter);
    m_gameModeStatusLabel->hide();  // 初始隱藏
    timeControlLayout->addWidget(m_gameModeStatusLabel);
    
    // 連線狀態標籤（線上模式時顯示）
    m_connectionStatusLabel = new QLabel("", this);
    m_connectionStatusLabel->setFont(labelFont);
    m_connectionStatusLabel->setAlignment(Qt::AlignCenter);
    m_connectionStatusLabel->setStyleSheet(QString("QLabel { color: %1; padding: 5px; border-radius: 4px; }").arg(THEME_TEXT_PRIMARY));
    m_connectionStatusLabel->hide();  // 初始隱藏
    timeControlLayout->addWidget(m_connectionStatusLabel);
    
    // 房間資訊標籤（線上模式時顯示）
    m_roomInfoLabel = new QLabel("", this);
    m_roomInfoLabel->setFont(labelFont);
    m_roomInfoLabel->setAlignment(Qt::AlignCenter);
    m_roomInfoLabel->setWordWrap(true);
    m_roomInfoLabel->setStyleSheet(QString("QLabel { color: %1; background: rgba(103, 232, 249, 0.15); "
        "padding: 8px; border-radius: 4px; font-weight: bold; }").arg(THEME_ACCENT_PRIMARY));
    m_roomInfoLabel->hide();  // 初始隱藏
    timeControlLayout->addWidget(m_roomInfoLabel);
    
    // 難度設定
    m_difficultyLabel = new QLabel("🎯 電腦難度:", this);
    m_difficultyLabel->setFont(labelFont);
    m_difficultyLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(THEME_ACCENT_WARNING));
    timeControlLayout->addWidget(m_difficultyLabel);
    
    // 初始值為 0（初學者），顯示 ELO 和中文難度名稱
    int initialElo = calculateElo(0);
    QString initialDiffName = getDifficultyName(0);
    m_difficultyValueLabel = new QLabel(QString("%1 (ELO %2)").arg(initialDiffName).arg(initialElo), this);
    m_difficultyValueLabel->setFont(labelFont);
    m_difficultyValueLabel->setAlignment(Qt::AlignCenter);
    m_difficultyValueLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; padding: 4px; "
        "background: rgba(255, 217, 61, 0.15); border-radius: 4px; }").arg(THEME_ACCENT_WARNING));
    timeControlLayout->addWidget(m_difficultyValueLabel);
    
    m_difficultySlider = new QSlider(Qt::Horizontal, this);
    m_difficultySlider->setMinimum(0);
    m_difficultySlider->setMaximum(20);
    m_difficultySlider->setValue(0);
    m_difficultySlider->setTickPosition(QSlider::TicksBelow);
    m_difficultySlider->setTickInterval(1);
    connect(m_difficultySlider, &QSlider::valueChanged, this, &Qt_Chess::onDifficultyChanged);
    timeControlLayout->addWidget(m_difficultySlider);
    
    // 電腦思考中的提示標籤（初始隱藏）- 現代科技風格動畫效果
    m_thinkingLabel = new QLabel("🔄 電腦思考中...", this);
    m_thinkingLabel->setFont(labelFont);
    m_thinkingLabel->setAlignment(Qt::AlignCenter);
    m_thinkingLabel->setStyleSheet(QString(
        "QLabel { "
        "  color: %1; "
        "  font-weight: bold; "
        "  padding: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(233, 69, 96, 0.3), stop:0.5 rgba(0, 217, 255, 0.3), stop:1 rgba(233, 69, 96, 0.3)); "
        "  border: 2px solid %1; "
        "  border-radius: 8px; "
        "}"
    ).arg(THEME_ACCENT_SECONDARY));
    m_thinkingLabel->hide();
    timeControlLayout->addWidget(m_thinkingLabel);
    
    // 根據初始模式設定難度控制的可見性（預設為雙人模式，隱藏難度控制）
    bool isVsComputer = (m_currentGameMode != GameMode::HumanVsHuman);
    m_colorSelectionWidget->setVisible(isVsComputer);
    m_difficultyLabel->setVisible(isVsComputer);
    m_difficultyValueLabel->setVisible(isVsComputer);
    m_difficultySlider->setVisible(isVsComputer);

    // 添加伸展以填充群組框中的剩餘空間
    timeControlLayout->addStretch();

    timeControlPanelLayout->addWidget(timeControlGroup, 1);

    // 開始按鈕 - 現代科技風格霓虹效果
    m_startButton = new QPushButton("▶ 開始對弈", this);
    m_startButton->setMinimumHeight(50);
    QFont startButtonFont;
    startButtonFont.setPointSize(14);
    startButtonFont.setBold(true);
    m_startButton->setFont(startButtonFont);
    m_startButton->setEnabled(true);  // 始終啟用以允許開始遊戲
    m_startButton->setStyleSheet(QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
        "  color: %2; "
        "  border: 3px solid %1; "
        "  border-radius: 12px; "
        "  padding: 10px; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
        "  border-color: white; "
        "}"
        "QPushButton:pressed { "
        "  background: %1; "
        "}"
        "QPushButton:disabled { "
        "  background: rgba(50, 50, 70, 0.6); "
        "  color: #666; "
        "  border-color: #444; "
        "}"
    ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
    timeControlPanelLayout->addWidget(m_startButton, 0);  // 伸展因子 0 以保持按鈕高度

    // 放棄按鈕 - 現代科技風格紅色警告效果
    m_giveUpButton = new QPushButton("🏳 放棄認輸", this);
    m_giveUpButton->setMinimumHeight(45);
    QFont giveUpButtonFont;
    giveUpButtonFont.setPointSize(12);
    giveUpButtonFont.setBold(true);
    m_giveUpButton->setFont(giveUpButtonFont);
    m_giveUpButton->setStyleSheet(QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %1, stop:0.5 rgba(233, 69, 96, 0.7), stop:1 %1); "
        "  color: %2; "
        "  border: 3px solid %3; "
        "  border-radius: 10px; "
        "  padding: 8px; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %3, stop:0.5 rgba(255, 100, 120, 0.9), stop:1 %3); "
        "  border-color: #FF6B6B; "
        "}"
        "QPushButton:pressed { "
        "  background: %3; "
        "}"
    ).arg(THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_ACCENT_SECONDARY));
    m_giveUpButton->hide();  // 初始隱藏
    connect(m_giveUpButton, &QPushButton::clicked, this, &Qt_Chess::onGiveUpClicked);
    timeControlPanelLayout->addWidget(m_giveUpButton, 0);  // 伸展因子 0 以保持按鈕高度

    // 退出房間按鈕 - 現代科技風格橙色警告效果
    m_exitRoomButton = new QPushButton("🚪 退出房間", this);
    m_exitRoomButton->setMinimumHeight(45);
    QFont exitRoomButtonFont;
    exitRoomButtonFont.setPointSize(12);
    exitRoomButtonFont.setBold(true);
    m_exitRoomButton->setFont(exitRoomButtonFont);
    m_exitRoomButton->setStyleSheet(QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %1, stop:0.5 rgba(255, 140, 0, 0.7), stop:1 %1); "
        "  color: %2; "
        "  border: 3px solid %3; "
        "  border-radius: 10px; "
        "  padding: 8px; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %3, stop:0.5 rgba(255, 160, 50, 0.9), stop:1 %3); "
        "  border-color: #FFA500; "
        "}"
        "QPushButton:pressed { "
        "  background: %3; "
        "}"
    ).arg(THEME_BG_DARK, THEME_TEXT_PRIMARY, THEME_ACCENT_WARNING));
    m_exitRoomButton->hide();  // 初始隱藏
    connect(m_exitRoomButton, &QPushButton::clicked, this, &Qt_Chess::onExitRoomClicked);
    timeControlPanelLayout->addWidget(m_exitRoomButton, 0);  // 伸展因子 0 以保持按鈕高度

    // 初始化 game timer
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &Qt_Chess::onGameTimerTick);
}

void Qt_Chess::onWhiteTimeLimitChanged(int value) {
    if (!m_whiteTimeLimitSlider || !m_whiteTimeLimitLabel) return;

    m_whiteTimeMs = calculateTimeFromSliderValue(value);
    m_whiteTimeLimitLabel->setText(getTimeTextFromSliderValue(value));

    // 更新 time control enabled state
    m_timeControlEnabled = (m_whiteTimeMs > 0 || m_blackTimeMs > 0);
    m_timerStarted = false;

    // 開始 button is always enabled, but don't change text if in online mode
    if (m_startButton && !m_isOnlineGame) {
        m_startButton->setEnabled(true);
        m_startButton->setText("▶ 開始對弈");
    }

    updateTimeDisplays();
    saveTimeControlSettings();
    
    // 如果是線上模式且是房主，並且對手已加入，發送時間設定更新
    if (m_isOnlineGame && m_networkManager && 
        m_networkManager->getRole() == NetworkRole::Host && 
        m_networkManager->getStatus() == ConnectionStatus::Connected) {
        m_networkManager->sendTimeSettings(m_whiteTimeMs, m_blackTimeMs, m_incrementMs);
    }
}

void Qt_Chess::onBlackTimeLimitChanged(int value) {
    if (!m_blackTimeLimitSlider || !m_blackTimeLimitLabel) return;

    m_blackTimeMs = calculateTimeFromSliderValue(value);
    m_blackTimeLimitLabel->setText(getTimeTextFromSliderValue(value));

    // 更新 time control enabled state
    m_timeControlEnabled = (m_whiteTimeMs > 0 || m_blackTimeMs > 0);
    m_timerStarted = false;

    // 開始 button is always enabled, but don't change text if in online mode
    if (m_startButton && !m_isOnlineGame) {
        m_startButton->setEnabled(true);
        m_startButton->setText("▶ 開始對弈");
    }

    updateTimeDisplays();
    saveTimeControlSettings();
    
    // 如果是線上模式且是房主，並且對手已加入，發送時間設定更新
    if (m_isOnlineGame && m_networkManager && 
        m_networkManager->getRole() == NetworkRole::Host && 
        m_networkManager->getStatus() == ConnectionStatus::Connected) {
        m_networkManager->sendTimeSettings(m_whiteTimeMs, m_blackTimeMs, m_incrementMs);
    }
}

void Qt_Chess::updateTimeDisplays() {
    if (!m_whiteTimeLabel || !m_blackTimeLabel) return;

    if (!m_timeControlEnabled) {
        m_whiteTimeLabel->setText("--:--");
        m_blackTimeLabel->setText("--:--");
        // 隱藏進度條當無時間控制時
        if (m_whiteTimeProgressBar) m_whiteTimeProgressBar->hide();
        if (m_blackTimeProgressBar) m_blackTimeProgressBar->hide();
        return;
    }

    // 轉換 milliseconds to minutes:seconds or show unlimited
    // 當時間 < 10 秒時，顯示格式為 0:秒.小數（例如 "0:9.8"）
    auto formatTime = [](int ms) -> QString {
        if (ms <= 0) {
            return "無限制";
        }

        // 如果少於 LOW_TIME_THRESHOLD_MS（10 秒），顯示格式為秒.小數（例如：9.8）
        if (ms < LOW_TIME_THRESHOLD_MS) {
            double seconds = ms / 1000.0;
            return QString("%1").arg(seconds, 0, 'f', 1);  // 格式：9.8
        }

        // 否則顯示分鐘:秒格式
        int totalSeconds = ms / 1000;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    };

    m_whiteTimeLabel->setText(formatTime(m_whiteTimeMs));
    m_blackTimeLabel->setText(formatTime(m_blackTimeMs));

    // 進度條樣式生成輔助函數
    auto getProgressBarStyle = [](bool isLowTime) -> QString {
        QString chunkColor = isLowTime ? "#DC3545" : "#4CAF50";
        return QString("QProgressBar { border: 1px solid #333; border-radius: 3px; background-color: #444; }"
                       "QProgressBar::chunk { background-color: %1; border-radius: 2px; }").arg(chunkColor);
    };

    // 更新進度條
    if (m_whiteTimeProgressBar && m_whiteInitialTimeMs > 0) {
        int whiteProgress = static_cast<int>((static_cast<double>(m_whiteTimeMs) / m_whiteInitialTimeMs) * 100);
        whiteProgress = qBound(0, whiteProgress, 100);
        m_whiteTimeProgressBar->setValue(whiteProgress);
        bool isLowTime = m_whiteTimeMs > 0 && m_whiteTimeMs < LOW_TIME_THRESHOLD_MS;
        m_whiteTimeProgressBar->setStyleSheet(getProgressBarStyle(isLowTime));
    }

    if (m_blackTimeProgressBar && m_blackInitialTimeMs > 0) {
        int blackProgress = static_cast<int>((static_cast<double>(m_blackTimeMs) / m_blackInitialTimeMs) * 100);
        blackProgress = qBound(0, blackProgress, 100);
        m_blackTimeProgressBar->setValue(blackProgress);
        bool isLowTime = m_blackTimeMs > 0 && m_blackTimeMs < LOW_TIME_THRESHOLD_MS;
        m_blackTimeProgressBar->setStyleSheet(getProgressBarStyle(isLowTime));
    }

    // 根據當前回合和剩餘時間確定背景顏色
    // 規則：不是自己的回合時顯示灰色，是自己的回合時根據剩餘時間決定（< 10 秒紅色，否則綠色）
    // 在回放模式中，使用進入回放時儲存的玩家，而不是棋盤上的當前玩家
    // 這樣可以確保計時器高亮顯示與實際倒數的玩家保持一致，不會隨著回放的棋步切換
    PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();

    QString whiteStyle, blackStyle;

    // 確定白方標籤樣式
    // 當不是自己的回合時，顯示灰色，即使時間少於 10 秒
    if (currentPlayer == PieceColor::White) {
        // 白方回合：如果時間少於 10 秒顯示紅色，否則顯示綠色
        if (m_whiteTimeMs > 0 && m_whiteTimeMs < LOW_TIME_THRESHOLD_MS) {
            whiteStyle = "QLabel { background-color: rgba(220, 53, 69, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
        } else {
            whiteStyle = "QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
        }
    } else {
        // 不是白方回合：顯示灰色（無論剩餘時間多少）
        whiteStyle = "QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    }

    // 確定黑方標籤樣式
    // 當不是自己的回合時，顯示灰色，即使時間少於 10 秒
    if (currentPlayer == PieceColor::Black) {
        // 黑方回合：如果時間少於 10 秒顯示紅色，否則顯示綠色
        if (m_blackTimeMs > 0 && m_blackTimeMs < LOW_TIME_THRESHOLD_MS) {
            blackStyle = "QLabel { background-color: rgba(220, 53, 69, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
        } else {
            blackStyle = "QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
        }
    } else {
        // 不是黑方回合：顯示灰色（無論剩餘時間多少）
        blackStyle = "QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    }

    m_whiteTimeLabel->setStyleSheet(whiteStyle);
    m_blackTimeLabel->setStyleSheet(blackStyle);
}

void Qt_Chess::onIncrementChanged(int value) {
    m_incrementMs = value * 1000;
    m_incrementLabel->setText(QString("%1秒").arg(value));
    saveTimeControlSettings();
    
    // 如果是線上模式且是房主，並且對手已加入，發送時間設定更新
    if (m_isOnlineGame && m_networkManager && 
        m_networkManager->getRole() == NetworkRole::Host && 
        m_networkManager->getStatus() == ConnectionStatus::Connected) {
        m_networkManager->sendTimeSettings(m_whiteTimeMs, m_blackTimeMs, m_incrementMs);
    }
}

void Qt_Chess::onGameTimerTick() {
    if (!m_timeControlEnabled) return;

    // 如果使用伺服器控制的計時器，從伺服器狀態更新顯示
    if (m_useServerTimer && m_isOnlineGame) {
        updateTimeDisplaysFromServer();
        return;
    }

    // 在線上模式中，使用伺服器同步的時間計算
    // 這確保兩位玩家看到相同的經過時間，即使他們的本地時鐘不同步
    if (m_isOnlineGame && m_gameStartLocalTime > 0) {
        // 獲取當前同步時間（使用伺服器時間偏移）
        qint64 currentSyncTime = QDateTime::currentMSecsSinceEpoch() + m_serverTimeOffset;
        qint64 gameStartSyncTime = m_gameStartLocalTime + m_serverTimeOffset;
        
        // 如果這是當前回合的第一個 tick，記錄回合開始時間
        if (m_currentTurnStartTime == 0) {
            m_currentTurnStartTime = currentSyncTime;
        }
        
        // 計算當前玩家在這個回合已用的時間
        qint64 turnElapsedMs = currentSyncTime - m_currentTurnStartTime;
        
        // 更新當前玩家的剩餘時間
        PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();
        if (currentPlayer == PieceColor::White) {
            if (m_whiteTimeMs > 0 && m_whiteInitialTimeMs > 0) {  // 檢查初始時間也 > 0（非無限制）
                // 基於實際經過時間更新，而不是固定減少100ms
                int newWhiteTime = m_whiteInitialTimeMs - static_cast<int>(turnElapsedMs);
                
                // 防止時間跳躍（如果計算出的時間比當前剩餘時間多，保持當前時間）
                if (newWhiteTime < m_whiteTimeMs) {
                    m_whiteTimeMs = newWhiteTime;
                }
                
                if (m_whiteTimeMs <= 0) {
                    m_whiteTimeMs = 0;
                    updateTimeDisplays();
                    stopTimer();
                    m_timerStarted = false;
                    showTimeControlAfterTimeout();
                    QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
                    return;
                }
            }
        } else {
            if (m_blackTimeMs > 0 && m_blackInitialTimeMs > 0) {  // 檢查初始時間也 > 0（非無限制）
                int newBlackTime = m_blackInitialTimeMs - static_cast<int>(turnElapsedMs);
                
                if (newBlackTime < m_blackTimeMs) {
                    m_blackTimeMs = newBlackTime;
                }
                
                if (m_blackTimeMs <= 0) {
                    m_blackTimeMs = 0;
                    updateTimeDisplays();
                    stopTimer();
                    m_timerStarted = false;
                    showTimeControlAfterTimeout();
                    QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
                    return;
                }
            }
        }
    } else {
        // 非線上模式：使用原本的遞減邏輯
        PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();
        if (currentPlayer == PieceColor::White) {
            if (m_whiteTimeMs > 0) {
                m_whiteTimeMs -= 100;
                if (m_whiteTimeMs <= 0) {
                    m_whiteTimeMs = 0;
                    updateTimeDisplays();
                    stopTimer();
                    m_timerStarted = false;
                    showTimeControlAfterTimeout();
                    QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
                    return;
                }
            }
        } else {
            if (m_blackTimeMs > 0) {
                m_blackTimeMs -= 100;
                if (m_blackTimeMs <= 0) {
                    m_blackTimeMs = 0;
                    updateTimeDisplays();
                    stopTimer();
                    m_timerStarted = false;
                    showTimeControlAfterTimeout();
                    QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
                    return;
                }
            }
        }
    }

    updateTimeDisplays();
}

void Qt_Chess::updateTimeDisplaysFromServer() {
    if (!m_networkManager) return;
    
    // 獲取當前 UNIX 毫秒數
    qint64 currentUnixTimeMs = QDateTime::currentMSecsSinceEpoch();
    
    // 計算距離最後切換經過的時間（毫秒）
    // m_serverLastSwitchTime 是秒數，需要轉換為毫秒
    // 如果 lastSwitchTime 為 0，表示計時器尚未啟動（等待第一步棋）
    qint64 elapsedMs = 0;
    if (m_serverLastSwitchTime > 0) {
        qint64 lastSwitchTimeMs = m_serverLastSwitchTime * 1000;
        elapsedMs = currentUnixTimeMs - lastSwitchTimeMs;
    }
    
    // 確定我是玩家 A (房主) 還是玩家 B (房客)
    bool isPlayerA = (m_networkManager->getRole() == NetworkRole::Host);
    
    // 計算白方和黑方的實際顯示時間
    qint64 whiteTime, blackTime;
    
    if (m_serverCurrentPlayer == "White") {
        // 白方正在走棋，從白方時間扣除 elapsed
        if (isPlayerA) {
            // 我是房主，房主是白方還是黑方？檢查我的顏色
            if (m_networkManager->getPlayerColor() == PieceColor::White) {
                // 房主是白方 (whiteIsA = true)
                whiteTime = m_serverTimeA - elapsedMs;
                blackTime = m_serverTimeB;
            } else {
                // 房主是黑方 (whiteIsA = false)
                whiteTime = m_serverTimeB - elapsedMs;
                blackTime = m_serverTimeA;
            }
        } else {
            // 我是房客
            if (m_networkManager->getPlayerColor() == PieceColor::White) {
                // 房客是白方 (whiteIsA = false)
                whiteTime = m_serverTimeB - elapsedMs;
                blackTime = m_serverTimeA;
            } else {
                // 房客是黑方 (whiteIsA = true)
                whiteTime = m_serverTimeA - elapsedMs;
                blackTime = m_serverTimeB;
            }
        }
    } else {
        // 黑方正在走棋，從黑方時間扣除 elapsed
        if (isPlayerA) {
            if (m_networkManager->getPlayerColor() == PieceColor::White) {
                // 房主是白方 (whiteIsA = true)
                whiteTime = m_serverTimeA;
                blackTime = m_serverTimeB - elapsedMs;
            } else {
                // 房主是黑方 (whiteIsA = false)
                whiteTime = m_serverTimeB;
                blackTime = m_serverTimeA - elapsedMs;
            }
        } else {
            if (m_networkManager->getPlayerColor() == PieceColor::White) {
                // 房客是白方 (whiteIsA = false)
                whiteTime = m_serverTimeB;
                blackTime = m_serverTimeA - elapsedMs;
            } else {
                // 房客是黑方 (whiteIsA = true)
                whiteTime = m_serverTimeA;
                blackTime = m_serverTimeB - elapsedMs;
            }
        }
    }
    
    // 確保時間不為負數
    whiteTime = qMax(static_cast<qint64>(0), whiteTime);
    blackTime = qMax(static_cast<qint64>(0), blackTime);
    
    // 防止時間跳躍：只允許時間遞減，不允許時間回跳
    // 當雙方顯示都歸0時，由伺服器（裁判）判斷超時
    int newWhiteTime = static_cast<int>(whiteTime);
    int newBlackTime = static_cast<int>(blackTime);
    
    // 只在時間減少或相等時更新，完全防止時間增加（回跳）
    if (newWhiteTime <= m_whiteTimeMs) {
        m_whiteTimeMs = newWhiteTime;
    }
    
    if (newBlackTime <= m_blackTimeMs) {
        m_blackTimeMs = newBlackTime;
    }
    
    // 更新顯示
    updateTimeDisplays();
    
    // 檢查超時（只檢查有設定時間限制的玩家）
    if (m_whiteTimeMs <= 0 && m_timeControlEnabled && m_whiteInitialTimeMs > 0) {
        stopTimer();
        m_timerStarted = false;
        showTimeControlAfterTimeout();
        QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
    } else if (m_blackTimeMs <= 0 && m_timeControlEnabled && m_blackInitialTimeMs > 0) {
        stopTimer();
        m_timerStarted = false;
        showTimeControlAfterTimeout();
        QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
    }
}

void Qt_Chess::startTimer() {
    if (m_timeControlEnabled && m_timerStarted && m_gameTimer && !m_gameTimer->isActive()) {
        m_gameTimer->start(100); // 每 100ms 觸發一次以平滑倒計時
    }
}

void Qt_Chess::stopTimer() {
    if (m_gameTimer && m_gameTimer->isActive()) {
        m_gameTimer->stop();
    }
}

void Qt_Chess::applyIncrement() {
    if (!m_timeControlEnabled || m_incrementMs <= 0) return;

    // 為剛完成移動的玩家添加增量
    // 注意：getCurrentPlayer() 在移動後返回對手
    PieceColor playerWhoMoved = (m_chessBoard.getCurrentPlayer() == PieceColor::White)
                                    ? PieceColor::Black : PieceColor::White;

    if (playerWhoMoved == PieceColor::White) {
        m_whiteTimeMs += m_incrementMs;
    } else {
        m_blackTimeMs += m_incrementMs;
    }
    
    // 在線上模式中，重置回合計時器，因為現在輪到新玩家了
    if (m_isOnlineGame && m_gameStartLocalTime > 0) {
        m_currentTurnStartTime = QDateTime::currentMSecsSinceEpoch() + m_serverTimeOffset;
        // 更新當前回合的初始時間（用於計算經過時間）
        PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
        if (currentPlayer == PieceColor::White) {
            m_whiteInitialTimeMs = m_whiteTimeMs;
        } else {
            m_blackInitialTimeMs = m_blackTimeMs;
        }
    }
}

void Qt_Chess::handleGameEnd() {
    // 停止 timer when game ends
    stopTimer();
    m_timerStarted = false;
    m_gameStarted = false;  // 標記遊戲已結束
    
    // 停止背景音樂（遊戲已結束）
    stopBackgroundMusic();

    // 隱藏放棄按鈕
    if (m_giveUpButton) {
        m_giveUpButton->hide();
    }

    // 顯示時間控制面板
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }

    // 重新啟用開始按鈕以允許開始新遊戲
    if (m_startButton) {
        m_startButton->setText("開始");
        m_startButton->setEnabled(true);
    }

    // 將時間和吃子紀錄移動到棋盤上下方
    moveWidgetsForGameEnd();

    // 顯示匯出 PGN 按鈕和複製棋譜按鈕
    if (m_exportPGNButton) {
        m_exportPGNButton->show();
    }
    if (m_copyPGNButton) {
        m_copyPGNButton->show();
    }

    // 更新回放按鈕狀態（遊戲結束後可以回放）
    updateReplayButtons();

    // 當遊戲結束時，將右側伸展設為 0
    setRightPanelStretch(0);
}

void Qt_Chess::moveWidgetsForGameEnd() {
    // 將對方的時間標籤和吃子紀錄移動到棋盤上方
    // 將我方的時間標籤和吃子紀錄移動到棋盤下方
    // 根據棋盤是否翻轉來決定誰是對方、誰是我方
    
    if (!m_topEndGamePanel || !m_bottomEndGamePanel) return;
    
    // 獲取上下方面板的佈局
    QHBoxLayout* topLayout = qobject_cast<QHBoxLayout*>(m_topEndGamePanel->layout());
    QHBoxLayout* bottomLayout = qobject_cast<QHBoxLayout*>(m_bottomEndGamePanel->layout());
    if (!topLayout || !bottomLayout) return;
    
    // 清空現有佈局中的 widgets（但不刪除它們）
    while (topLayout->count() > 0) {
        topLayout->takeAt(0);
    }
    while (bottomLayout->count() > 0) {
        bottomLayout->takeAt(0);
    }
    
    // 根據棋盤翻轉狀態決定對方和我方
    // 當棋盤翻轉時（玩家執黑），黑方在下方，白方在上方
    // 當棋盤不翻轉時（玩家執白），白方在下方，黑方在上方
    
    QLabel* opponentTimeLabel = nullptr;
    QProgressBar* opponentProgressBar = nullptr;
    QWidget* opponentCapturedPanel = nullptr;
    QLabel* myTimeLabel = nullptr;
    QProgressBar* myProgressBar = nullptr;
    QWidget* myCapturedPanel = nullptr;
    
    if (m_isBoardFlipped) {
        // 棋盤翻轉：玩家執黑，對方（白方）在上方，我方（黑方）在下方
        opponentTimeLabel = m_whiteTimeLabel;
        opponentProgressBar = m_whiteTimeProgressBar;
        opponentCapturedPanel = m_capturedBlackPanel;  // 對方（白方）吃掉的我方棋子（黑子）
        myTimeLabel = m_blackTimeLabel;
        myProgressBar = m_blackTimeProgressBar;
        myCapturedPanel = m_capturedWhitePanel;  // 我方（黑方）吃掉的對方棋子（白子）
    } else {
        // 棋盤不翻轉：玩家執白，對方（黑方）在上方，我方（白方）在下方
        opponentTimeLabel = m_blackTimeLabel;
        opponentProgressBar = m_blackTimeProgressBar;
        opponentCapturedPanel = m_capturedWhitePanel;  // 對方（黑方）吃掉的我方棋子（白子）
        myTimeLabel = m_whiteTimeLabel;
        myProgressBar = m_whiteTimeProgressBar;
        myCapturedPanel = m_capturedBlackPanel;  // 我方（白方）吃掉的對方棋子（黑子）
    }
    
    // 將 widgets 重新設定父物件並添加到新佈局
    // 對方的時間和吃子紀錄放在棋盤上方
    if (opponentTimeLabel) {
        opponentTimeLabel->setParent(m_topEndGamePanel);
        opponentTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        topLayout->addWidget(opponentTimeLabel);
        opponentTimeLabel->show();
    }
    // 遊戲結束時不顯示進度條
    if (opponentProgressBar) {
        opponentProgressBar->hide();
    }
    if (opponentCapturedPanel) {
        opponentCapturedPanel->setParent(m_topEndGamePanel);
        topLayout->addWidget(opponentCapturedPanel, 1);
        opponentCapturedPanel->show();
    }
    
    // 我方的時間和吃子紀錄放在棋盤下方
    if (myTimeLabel) {
        myTimeLabel->setParent(m_bottomEndGamePanel);
        myTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        bottomLayout->addWidget(myTimeLabel);
        myTimeLabel->show();
    }
    // 遊戲結束時不顯示進度條
    if (myProgressBar) {
        myProgressBar->hide();
    }
    if (myCapturedPanel) {
        myCapturedPanel->setParent(m_bottomEndGamePanel);
        bottomLayout->addWidget(myCapturedPanel, 1);
        myCapturedPanel->show();
    }
    
    // 隱藏右側面板
    if (m_rightTimePanel) {
        m_rightTimePanel->hide();
    }
    
    // 顯示上下方面板
    m_topEndGamePanel->show();
    m_bottomEndGamePanel->show();
    
    // 更新被吃掉的棋子顯示
    updateCapturedPiecesDisplay();
}

void Qt_Chess::restoreWidgetsFromGameEnd() {
    // 將時間和吃子紀錄恢復到右側面板
    if (!m_rightTimePanel) return;
    
    QVBoxLayout* rightLayout = qobject_cast<QVBoxLayout*>(m_rightTimePanel->layout());
    if (!rightLayout) return;
    
    // 隱藏上下方面板
    if (m_topEndGamePanel) {
        m_topEndGamePanel->hide();
    }
    if (m_bottomEndGamePanel) {
        m_bottomEndGamePanel->hide();
    }
    
    // 清空右側佈局（但不刪除 widgets）
    while (rightLayout->count() > 0) {
        rightLayout->takeAt(0);
    }
    
    // 將 widgets 重新設定父物件並添加到右側佈局
    // 順序根據 m_isBoardFlipped 調整：對方在上方，我方在下方
    
    // 根據棋盤翻轉狀態決定上下方的吃子面板和時間標籤
    QWidget* topCapturedPanel = nullptr;
    QProgressBar* topProgressBar = nullptr;
    QLabel* topTimeLabel = nullptr;
    QLabel* bottomTimeLabel = nullptr;
    QProgressBar* bottomProgressBar = nullptr;
    QWidget* bottomCapturedPanel = nullptr;
    
    if (m_isBoardFlipped) {
        // 玩家執黑：對方（白方）在上方，我方（黑方）在下方
        topCapturedPanel = m_capturedBlackPanel;    // 對方（白方）吃掉的我方棋子（黑子）
        topProgressBar = m_whiteTimeProgressBar;
        topTimeLabel = m_whiteTimeLabel;
        bottomTimeLabel = m_blackTimeLabel;
        bottomProgressBar = m_blackTimeProgressBar;
        bottomCapturedPanel = m_capturedWhitePanel; // 我方（黑方）吃掉的對方棋子（白子）
    } else {
        // 玩家執白：對方（黑方）在上方，我方（白方）在下方
        topCapturedPanel = m_capturedWhitePanel;    // 對方（黑方）吃掉的我方棋子（白子）
        topProgressBar = m_blackTimeProgressBar;
        topTimeLabel = m_blackTimeLabel;
        bottomTimeLabel = m_whiteTimeLabel;
        bottomProgressBar = m_whiteTimeProgressBar;
        bottomCapturedPanel = m_capturedBlackPanel; // 我方（白方）吃掉的對方棋子（黑子）
    }
    
    // 按順序添加：對方吃子紀錄 -> 對方時間進度條 -> 對方時間 -> 我方時間 -> 我方時間進度條 -> 我方吃子紀錄
    if (topCapturedPanel) {
        topCapturedPanel->setParent(m_rightTimePanel);
        rightLayout->addWidget(topCapturedPanel, 1);
    }
    
    if (topProgressBar) {
        topProgressBar->setParent(m_rightTimePanel);
        rightLayout->addWidget(topProgressBar, 0, Qt::AlignCenter);
    }
    
    if (topTimeLabel) {
        topTimeLabel->setParent(m_rightTimePanel);
        topTimeLabel->setAlignment(Qt::AlignCenter);
        rightLayout->addWidget(topTimeLabel, 0, Qt::AlignCenter);
    }
    
    if (bottomTimeLabel) {
        bottomTimeLabel->setParent(m_rightTimePanel);
        bottomTimeLabel->setAlignment(Qt::AlignCenter);
        rightLayout->addWidget(bottomTimeLabel, 0, Qt::AlignCenter);
    }
    
    if (bottomProgressBar) {
        bottomProgressBar->setParent(m_rightTimePanel);
        rightLayout->addWidget(bottomProgressBar, 0, Qt::AlignCenter);
    }
    
    if (bottomCapturedPanel) {
        bottomCapturedPanel->setParent(m_rightTimePanel);
        rightLayout->addWidget(bottomCapturedPanel, 1);
    }
    
    // 更新被吃掉的棋子顯示
    updateCapturedPiecesDisplay();
}

void Qt_Chess::loadTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");

    // 載入 white time
    int whiteTimeLimitSeconds = settings.value("whiteTimeLimitSeconds", 0).toInt();
    int whiteSliderPosition = 0;

    if (whiteTimeLimitSeconds <= 0) {
        whiteSliderPosition = 0;  // 無限制
    } else if (whiteTimeLimitSeconds == 30) {
        whiteSliderPosition = 1;  // 30 秒
    } else if (whiteTimeLimitSeconds < 60) {
        // 舊版值 < 60 秒（非 30）-> 映射到 30 秒作為最接近的有效選項
        whiteSliderPosition = 1;
    } else {
        // 轉換 minutes to slider position (2-31 = 1-30 minutes)
        int minutes = whiteTimeLimitSeconds / 60;
        if (minutes > MAX_MINUTES) minutes = MAX_MINUTES;  // 限制在最大分鐘數
        whiteSliderPosition = minutes + 1;
    }

    // 載入 black time
    int blackTimeLimitSeconds = settings.value("blackTimeLimitSeconds", 0).toInt();
    int blackSliderPosition = 0;

    if (blackTimeLimitSeconds <= 0) {
        blackSliderPosition = 0;  // 無限制
    } else if (blackTimeLimitSeconds == 30) {
        blackSliderPosition = 1;  // 30 秒
    } else if (blackTimeLimitSeconds < 60) {
        blackSliderPosition = 1;
    } else {
        int minutes = blackTimeLimitSeconds / 60;
        if (minutes > MAX_MINUTES) minutes = MAX_MINUTES;
        blackSliderPosition = minutes + 1;
    }

    int incrementSeconds = settings.value("incrementSeconds", 0).toInt();

    // 設置 the time limit sliders
    if (m_whiteTimeLimitSlider) {
        m_whiteTimeLimitSlider->setValue(whiteSliderPosition);
    }

    if (m_blackTimeLimitSlider) {
        m_blackTimeLimitSlider->setValue(blackSliderPosition);
    }

    // 設置 increment
    if (m_incrementSlider) {
        m_incrementSlider->setValue(incrementSeconds);
    }

    m_incrementMs = incrementSeconds * 1000;

    // 時間控制啟用狀態將由 onWhiteTimeLimitChanged 和 onBlackTimeLimitChanged 設置
    // 這些由上面的 setValue 觸發
}

void Qt_Chess::saveTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");

    // 儲存 white time (轉換毫秒為秒)
    if (m_whiteTimeLimitSlider) {
        int timeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
        int seconds = timeMs / 1000;
        settings.setValue("whiteTimeLimitSeconds", seconds);
    }

    // 儲存 black time (轉換毫秒為秒)
    if (m_blackTimeLimitSlider) {
        int timeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
        int seconds = timeMs / 1000;
        settings.setValue("blackTimeLimitSeconds", seconds);
    }

    if (m_incrementSlider) {
        settings.setValue("incrementSeconds", m_incrementSlider->value());
    }

    settings.sync();
}

void Qt_Chess::showTimeControlAfterTimeout() {
    // 標記遊戲已結束
    m_gameStarted = false;
    
    // 停止背景音樂（超時結束）
    stopBackgroundMusic();

    // 顯示時間控制面板 so user can adjust settings
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }

    // 隱藏放棄按鈕
    if (m_giveUpButton) {
        m_giveUpButton->hide();
    }

    // 重新啟用開始按鈕
    if (m_startButton) {
        m_startButton->setText("開始");
        m_startButton->setEnabled(true);
    }

    // 將時間和吃子紀錄移動到棋盤上下方
    moveWidgetsForGameEnd();

    // 當遊戲超時結束時，將右側伸展設為 0
    setRightPanelStretch(0);
}

void Qt_Chess::resetBoardState() {
    // 重置棋盤到初始狀態
    m_chessBoard.initializeBoard();
    m_pieceSelected = false;
    
    // 重置上一步移動高亮
    m_lastMoveFrom = QPoint(-1, -1);
    m_lastMoveTo = QPoint(-1, -1);
    
    updateBoard();
    clearHighlights();
}

int Qt_Chess::calculateTimeFromSliderValue(int value) const {
    // 根據滑桿值計算時間（毫秒）
    // 滑桿位置：0=無限制，1=30秒，2-31=1-30分鐘

    // 驗證輸入範圍
    if (value < 0 || value > MAX_SLIDER_POSITION) {
        return 0;  // 無效輸入，返回無限制
    }

    if (value == 0) {
        return 0;  // 無限制
    } else if (value == 1) {
        return 30 * 1000;  // 30 秒
    } else {
        // 值 2-31 代表 1-30 分鐘
        return (value - 1) * 60 * 1000;
    }
}

QString Qt_Chess::getTimeTextFromSliderValue(int value) const {
    // 根據滑桿值取得顯示文字
    // 滑桿位置：0=無限制，1=30秒，2-31=1-30分鐘

    // 驗證輸入範圍（與 calculateTimeFromSliderValue 一致）
    if (value < 0 || value > MAX_SLIDER_POSITION) {
        return "不限時";  // 無效輸入，返回不限時
    }

    if (value == 0) {
        return "不限時";
    } else if (value == 1) {
        return "30秒";
    } else {
        // 值 2-31 代表 1-30 分鐘
        int minutes = value - 1;
        return QString("%1分鐘").arg(minutes);
    }
}

void Qt_Chess::setRightPanelStretch(int stretch) {
    // 設置右側面板伸展因子的輔助函數
    // 當遊戲還沒開始時設為 0（緊湊佈局），當遊戲開始時設為 1（擴展間距）
    if (m_contentLayout && m_rightStretchIndex >= 0 && m_rightStretchIndex < m_contentLayout->count()) {
        m_contentLayout->setStretch(m_rightStretchIndex, stretch);
    }
}

// 棋譜功能實現
void Qt_Chess::updateMoveList() {
    if (!m_moveListWidget) return;

    m_moveListWidget->clear();
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();

    // 每兩步組合成一行（白方和黑方）
    for (size_t i = 0; i < moveHistory.size(); i += 2) {
        int moveNumber = (i / 2) + 1;
        QString moveText = QString("%1. %2").arg(moveNumber).arg(moveHistory[i].algebraicNotation);

        // 如果有黑方的移動，添加到同一行
        if (i + 1 < moveHistory.size()) {
            moveText += QString(" %1").arg(moveHistory[i + 1].algebraicNotation);
        }

        m_moveListWidget->addItem(moveText);
    }

    // 自動捲動到最新的移動
    m_moveListWidget->scrollToBottom();

    // 更新回放按鈕狀態
    updateReplayButtons();
}

void Qt_Chess::onExportPGNClicked() {
    exportPGN();
}

void Qt_Chess::exportPGN() {
    QString pgn = generatePGN();

    // 使用文件對話框讓用戶選擇保存位置
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "匯出 PGN",
                                                    "game.pgn",
                                                    "PGN 檔案 (*.pgn);;所有檔案 (*)");

    if (fileName.isEmpty()) {
        return;  // 用戶取消
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "錯誤", "無法保存檔案");
        return;
    }

    QTextStream out(&file);
    out << pgn;
    file.close();

    QMessageBox::information(this, "成功", "PGN 已成功匯出");
}

QString Qt_Chess::generatePGN() const {
    QString pgn;

    // PGN 標頭
    QDate currentDate = QDate::currentDate();
    pgn += QString("[Event \"雙人對弈\"]\n");
    pgn += QString("[Site \"Qt_Chess\"]\n");
    pgn += QString("[Date \"%1\"]\n").arg(currentDate.toString("yyyy.MM.dd"));
    pgn += QString("[Round \"-\"]\n");
    pgn += QString("[White \"白方\"]\n");
    pgn += QString("[Black \"黑方\"]\n");

    // 結果
    QString result = m_chessBoard.getGameResultString();

    // 如果遊戲結果還未確定，根據當前棋盤狀態檢查
    if (result == "*") {
        PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
        if (m_chessBoard.isCheckmate(currentPlayer)) {
            result = (currentPlayer == PieceColor::White) ? "0-1" : "1-0";
        } else if (m_chessBoard.isStalemate(currentPlayer) || m_chessBoard.isInsufficientMaterial()) {
            result = "1/2-1/2";
        }
    }
    pgn += QString("[Result \"%1\"]\n\n").arg(result);

    // 移動列表
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
    int moveNumber = 1;
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        if (i % 2 == 0) {
            // 白方移動
            if (i > 0) pgn += " ";
            pgn += QString("%1. %2").arg(moveNumber).arg(moveHistory[i].algebraicNotation);
        } else {
            // 黑方移動
            pgn += QString(" %1").arg(moveHistory[i].algebraicNotation);
            moveNumber++;

            // 每 PGN_MOVES_PER_LINE 個回合換行以提高可讀性
            if (moveNumber > 1 && (moveNumber - 1) % PGN_MOVES_PER_LINE == 0 && i + 1 < moveHistory.size()) {
                pgn += "\n";
            }
        }
    }

    // 添加結果
    if (!moveHistory.empty()) {
        pgn += " ";
    }
    pgn += result + "\n";

    return pgn;
}

void Qt_Chess::onCopyPGNClicked() {
    copyPGN();
}

void Qt_Chess::copyPGN() {
    QString pgn = generatePGN();

    // 複製到剪貼簿
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(pgn);

    QMessageBox::information(this, "成功", "棋譜已複製到剪貼簿");
}

int Qt_Chess::getPieceValue(PieceType type) const {
    // 標準國際象棋棋子分值
    switch (type) {
        case PieceType::None:   return 0;  // 空格不計分
        case PieceType::Pawn:   return 1;
        case PieceType::Knight: return 3;
        case PieceType::Bishop: return 3;
        case PieceType::Rook:   return 5;
        case PieceType::Queen:  return 9;
        case PieceType::King:   return 0;  // 國王不計分
    }
    return 0;  // 防禦性返回
}

void Qt_Chess::updateCapturedPiecesDisplay() {
    // 清除現有的被吃掉棋子標籤
    for (QLabel* label : m_capturedWhiteLabels) {
        delete label;
    }
    m_capturedWhiteLabels.clear();

    for (QLabel* label : m_capturedBlackLabels) {
        delete label;
    }
    m_capturedBlackLabels.clear();

    // 計算雙方被吃掉棋子的總分值
    const std::vector<ChessPiece>& capturedWhite = m_chessBoard.getCapturedPieces(PieceColor::White);
    const std::vector<ChessPiece>& capturedBlack = m_chessBoard.getCapturedPieces(PieceColor::Black);
    
    int whiteCapturedValue = 0;  // 被吃掉的白色棋子總值（黑方得分）
    int blackCapturedValue = 0;  // 被吃掉的黑色棋子總值（白方得分）
    
    for (const ChessPiece& piece : capturedWhite) {
        whiteCapturedValue += getPieceValue(piece.getType());
    }
    for (const ChessPiece& piece : capturedBlack) {
        blackCapturedValue += getPieceValue(piece.getType());
    }
    
    // 計算分差：正值表示該方領先
    // 白方分差 = 白方得分（吃掉的黑子）- 黑方得分（吃掉的白子）
    int whiteDiff = blackCapturedValue - whiteCapturedValue;
    int blackDiff = -whiteDiff;  // 黑方分差與白方分差相反

    // 檢查是否處於遊戲結束狀態（面板已移動到上下方）
    bool isEndGameLayout = m_topEndGamePanel && m_topEndGamePanel->isVisible();

    // 被吃掉棋子的大小和間距設定
    const int pieceSize = 24;  // 每個棋子標籤的大小
    const int horizontalOffset = pieceSize / 4;  // 相同類型棋子的水平重疊偏移量
    const int verticalOffset = pieceSize;  // 不同類型棋子之間的垂直間距
    const int topMargin = isEndGameLayout ? 5 : 38;  // 頂部邊距：遊戲結束時較小
    const int leftMargin = 5;  // 左邊距

    // 按棋子類型分組並顯示的輔助函數
    // 根據是否為遊戲結束佈局使用不同的排列方式
    // 返回最終的位置以便放置分差標籤
    auto displayCapturedPieces = [this, pieceSize, horizontalOffset, verticalOffset, topMargin, leftMargin, isEndGameLayout](
        QWidget* panel, const std::vector<ChessPiece>& capturedPieces, QList<QLabel*>& labels) -> int {
        if (!panel) return 0;
        if (capturedPieces.empty()) return 0;

        // 複製並按棋子分值由小到大排序，確保相同分值的棋子放在一起
        std::vector<ChessPiece> sortedPieces = capturedPieces;
        std::sort(sortedPieces.begin(), sortedPieces.end(), [this](const ChessPiece& a, const ChessPiece& b) {
            int valueA = getPieceValue(a.getType());
            int valueB = getPieceValue(b.getType());
            if (valueA != valueB) {
                return valueA < valueB;  // 按分值由小到大排序
            }
            // 分值相同時，按類型排序以保持穩定性
            return static_cast<int>(a.getType()) < static_cast<int>(b.getType());
        });

        int panelWidth = panel->width();
        int panelHeight = panel->height();
        // 如果面板寬度尚未計算（初始設置期間），使用最小寬度
        if (panelWidth <= 0) {
            panelWidth = panel->minimumWidth();
            if (panelWidth <= 0) panelWidth = isEndGameLayout ? ENDGAME_PANEL_FALLBACK_WIDTH : NORMAL_PANEL_FALLBACK_WIDTH;
        }
        if (panelHeight <= 0) {
            panelHeight = panel->minimumHeight();
            if (panelHeight <= 0) panelHeight = isEndGameLayout ? ENDGAME_PANEL_FALLBACK_HEIGHT : NORMAL_PANEL_FALLBACK_HEIGHT;
        }

        int xPos = leftMargin;
        int yPos = topMargin;
        PieceType lastType = PieceType::None;

        for (size_t i = 0; i < sortedPieces.size(); ++i) {
            const ChessPiece& piece = sortedPieces[i];

            // 先計算下一個棋子的位置
            int nextYPos = yPos;
            int nextXPos = xPos;

            // 如果不是第一個棋子，根據類型決定位置
            if (lastType != PieceType::None) {
                if (piece.getType() == lastType) {
                    // 相同類型的棋子水平重疊
                    int newXPos = xPos + horizontalOffset;
                    // 檢查是否超出面板寬度，如果超出則換行
                    if (newXPos + pieceSize > panelWidth) {
                        if (isEndGameLayout) {
                            // 遊戲結束佈局時，超出寬度就不再顯示更多棋子
                            break;
                        } else {
                            // 正常佈局時換行
                            nextYPos += verticalOffset;
                            nextXPos = leftMargin;
                        }
                    } else {
                        nextXPos = newXPos;
                    }
                } else {
                    if (isEndGameLayout) {
                        // 遊戲結束時，不同類型棋子也水平排列，只是間距較大
                        int newXPos = xPos + pieceSize;
                        if (newXPos + pieceSize > panelWidth) {
                            break;  // 超出寬度就不再顯示
                        }
                        nextXPos = newXPos;
                    } else {
                        // 正常佈局：不同類型的棋子垂直排列（換行）
                        nextYPos += verticalOffset;
                        nextXPos = leftMargin;  // 重置 x 位置
                    }
                }
            }

            // 檢查是否超出面板高度，如果超出則停止顯示
            if (nextYPos + pieceSize > panelHeight) {
                break;  // 停止處理更多棋子
            }

            // 更新位置
            yPos = nextYPos;
            xPos = nextXPos;

            // 創建並放置棋子標籤
            QLabel* label = new QLabel(panel);
            label->setText(piece.getSymbol());
            QFont pieceFont;
            pieceFont.setPointSize(16);
            label->setFont(pieceFont);
            label->setFixedSize(pieceSize, pieceSize);
            label->setAlignment(Qt::AlignCenter);
            label->move(xPos, yPos);
            lastType = piece.getType();

            label->show();
            labels.append(label);
        }
        
        // 返回最終的位置（用於放置分差標籤）
        if (isEndGameLayout) {
            return xPos + pieceSize;  // 返回 x 位置
        } else {
            return yPos + pieceSize;  // 返回 y 位置
        }
    };

    // 更新分差標籤的輔助函數
    auto updateScoreDiffLabel = [isEndGameLayout](QLabel*& label, QWidget* panel, int scoreDiff, int position) {
        if (!panel) return;
        
        // 如果標籤不存在，創建它
        if (!label) {
            label = new QLabel(panel);
            QFont scoreFont;
            scoreFont.setPointSize(12);
            scoreFont.setBold(true);
            label->setFont(scoreFont);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        
        // 只有當該方領先時才顯示分差
        if (scoreDiff > 0) {
            label->setText(QString("+%1").arg(scoreDiff));
            label->setStyleSheet("QLabel { color: #4CAF50; }");  // 綠色表示領先
            if (isEndGameLayout) {
                // 遊戲結束佈局：分差標籤在棋子右側
                label->move(position + 5, 5);
            } else {
                // 正常佈局：分差標籤在棋子下方
                label->move(5, position + 5);
            }
            label->adjustSize();
            label->show();
        } else {
            label->hide();
        }
    };

    // 根據棋盤是否翻轉來決定哪個面板顯示哪方的被吃棋子
    // 上方面板（m_capturedWhitePanel）應該顯示對方被吃掉的棋子
    // 下方面板（m_capturedBlackPanel）應該顯示我方吃掉的對方棋子
    
    int topPanelEndY = 0;
    int bottomPanelEndY = 0;
    
    if (m_isBoardFlipped) {
        // 棋盤翻轉：玩家執黑
        // 上方面板顯示被吃掉的黑子（對方白方吃掉的我方黑子）
        // 下方面板顯示被吃掉的白子（我方黑方吃掉的對方白子）
        if (m_capturedWhitePanel) {
            topPanelEndY = displayCapturedPieces(m_capturedWhitePanel, capturedBlack, m_capturedWhiteLabels);
            updateScoreDiffLabel(m_blackScoreDiffLabel, m_capturedWhitePanel, whiteDiff, topPanelEndY);
        }
        if (m_capturedBlackPanel) {
            bottomPanelEndY = displayCapturedPieces(m_capturedBlackPanel, capturedWhite, m_capturedBlackLabels);
            updateScoreDiffLabel(m_whiteScoreDiffLabel, m_capturedBlackPanel, blackDiff, bottomPanelEndY);
        }
    } else {
        // 棋盤不翻轉：玩家執白
        // 上方面板顯示被吃掉的白子（對方黑方吃掉的我方白子）
        // 下方面板顯示被吃掉的黑子（我方白方吃掉的對方黑子）
        if (m_capturedWhitePanel) {
            topPanelEndY = displayCapturedPieces(m_capturedWhitePanel, capturedWhite, m_capturedWhiteLabels);
            updateScoreDiffLabel(m_blackScoreDiffLabel, m_capturedWhitePanel, blackDiff, topPanelEndY);
        }
        if (m_capturedBlackPanel) {
            bottomPanelEndY = displayCapturedPieces(m_capturedBlackPanel, capturedBlack, m_capturedBlackLabels);
            updateScoreDiffLabel(m_whiteScoreDiffLabel, m_capturedBlackPanel, whiteDiff, bottomPanelEndY);
        }
    }
}

void Qt_Chess::enterReplayMode() {
    if (m_isReplayMode) return;

    m_isReplayMode = true;

    // 儲存當前棋盤狀態
    saveBoardState();

    // 在回放模式中，不再禁用時間控制滑桿
}

void Qt_Chess::exitReplayMode() {
    if (!m_isReplayMode) return;

    m_isReplayMode = false;
    m_replayMoveIndex = -1;

    // 恢復棋盤狀態
    restoreBoardState();

    // 取消棋譜列表的選擇
    m_moveListWidget->clearSelection();

    // 更新回放按鈕狀態
    updateReplayButtons();
}

void Qt_Chess::replayToMove(int moveIndex) {
    // 保存移動歷史的副本，因為 initializeBoard() 會清除它
    std::vector<MoveRecord> moveHistory = m_chessBoard.getMoveHistory();

    // 限制索引範圍
    if (moveIndex < -1) moveIndex = -1;
    if (moveIndex >= static_cast<int>(moveHistory.size())) {
        moveIndex = moveHistory.size() - 1;
    }

    m_replayMoveIndex = moveIndex;

    // 重新初始化棋盤
    m_chessBoard.initializeBoard();

    // 重播棋步直到指定的移動
    for (int i = 0; i <= moveIndex; ++i) {
        const MoveRecord& move = moveHistory[i];
        m_chessBoard.movePiece(move.from, move.to);

        // 處理升變
        if (move.isPromotion) {
            m_chessBoard.promotePawn(move.to, move.promotionType);
        }
    }

    // 恢復移動歷史，因為 movePiece 會記錄新的移動
    // 我們需要保持原始的移動歷史用於回放
    m_chessBoard.setMoveHistory(moveHistory);

    // 更新顯示
    updateBoard();
    clearHighlights();
    updateReplayButtons();

    // 高亮當前移動在棋譜列表中
    if (moveIndex >= 0) {
        int row = moveIndex / 2;
        m_moveListWidget->setCurrentRow(row);
    } else {
        m_moveListWidget->clearSelection();
    }

    // 不再自動退出回放模式，即使已經在最新一步
    // 允許用戶留在回放模式中查看最終狀態
}

void Qt_Chess::onReplayFirstClicked() {
    // 如果尚未進入回放模式，先進入
    if (!m_isReplayMode) {
        enterReplayMode();
    }

    replayToMove(-1);  // 初始狀態
}

void Qt_Chess::onReplayPrevClicked() {
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();

    // 如果尚未進入回放模式，從最新的一步開始往上
    if (!m_isReplayMode) {
        enterReplayMode();
        if (!moveHistory.empty()) {
            // 從最新一步往上倒退一步
            int targetIndex = static_cast<int>(moveHistory.size()) - 2;
            replayToMove(targetIndex);  // 如果只有一步，會顯示初始狀態（-1）
        }
    } else {
        replayToMove(m_replayMoveIndex - 1);
    }
}

void Qt_Chess::onReplayNextClicked() {
    // 如果尚未進入回放模式，先進入
    if (!m_isReplayMode) {
        enterReplayMode();
    }

    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
    int targetIndex = m_replayMoveIndex + 1;

    // 如果下一步是最後一步，回放到該步後自動退出回放模式
    if (!moveHistory.empty() && targetIndex >= static_cast<int>(moveHistory.size()) - 1) {
        replayToMove(moveHistory.size() - 1);
        exitReplayMode();
    } else {
        replayToMove(targetIndex);
    }
}

void Qt_Chess::onReplayLastClicked() {
    // 如果尚未進入回放模式，先進入
    if (!m_isReplayMode) {
        enterReplayMode();
    }

    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();
    if (!moveHistory.empty()) {
        // 跳到最後一步後自動退出回放模式
        replayToMove(moveHistory.size() - 1);
        exitReplayMode();
    }
}

void Qt_Chess::updateReplayButtons() {
    const std::vector<MoveRecord>& moveHistory = m_chessBoard.getMoveHistory();

    // 如果沒有棋步歷史，停用所有按鈕
    if (moveHistory.empty()) {
        if (m_replayFirstButton) m_replayFirstButton->setEnabled(false);
        if (m_replayPrevButton) m_replayPrevButton->setEnabled(false);
        if (m_replayNextButton) m_replayNextButton->setEnabled(false);
        if (m_replayLastButton) m_replayLastButton->setEnabled(false);
        return;
    }

    // 在回放模式中，根據當前位置啟用/停用按鈕
    if (m_isReplayMode) {
        if (m_replayFirstButton) {
            m_replayFirstButton->setEnabled(m_replayMoveIndex >= 0);
        }
        if (m_replayPrevButton) {
            m_replayPrevButton->setEnabled(m_replayMoveIndex >= 0);
        }
        if (m_replayNextButton) {
            m_replayNextButton->setEnabled(m_replayMoveIndex < static_cast<int>(moveHistory.size()) - 1);
        }
        if (m_replayLastButton) {
            m_replayLastButton->setEnabled(m_replayMoveIndex < static_cast<int>(moveHistory.size()) - 1);
        }
    } else {
        // 不在回放模式，已經在當前棋盤狀態
        // 啟用「第一步」和「上一步」按鈕以允許進入回放
        if (m_replayFirstButton) m_replayFirstButton->setEnabled(true);
        if (m_replayPrevButton) m_replayPrevButton->setEnabled(true);
        // 停用「下一步」和「最後一步」按鈕，因為已經在最新狀態
        if (m_replayNextButton) m_replayNextButton->setEnabled(false);
        if (m_replayLastButton) m_replayLastButton->setEnabled(false);
    }
}

void Qt_Chess::saveBoardState() {
    // 儲存當前棋盤狀態
    m_savedBoardState.clear();
    m_savedBoardState.resize(8);
    for (int row = 0; row < 8; ++row) {
        m_savedBoardState[row].resize(8);
        for (int col = 0; col < 8; ++col) {
            m_savedBoardState[row][col] = m_chessBoard.getPiece(row, col);
        }
    }
    m_savedCurrentPlayer = m_chessBoard.getCurrentPlayer();
}

void Qt_Chess::restoreBoardState() {
    // 恢復棋盤狀態
    if (m_savedBoardState.size() != 8) return;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_chessBoard.setPiece(row, col, m_savedBoardState[row][col]);
        }
    }

    // 恢復當前玩家
    m_chessBoard.setCurrentPlayer(m_savedCurrentPlayer);

    // 更新顯示
    updateBoard();
    clearHighlights();
}

// 電腦對弈功能實現
// 注意：setupEngineUI 的功能已整合到 setupTimeControlUI 中
void Qt_Chess::setupEngineUI(QVBoxLayout* layout) {
    Q_UNUSED(layout);
    // 此函數已被棄用，所有遊戲模式 UI 現在都在 setupTimeControlUI 中設置
}

void Qt_Chess::initializeEngine() {
    m_chessEngine = new ChessEngine(this);
    
    connect(m_chessEngine, &ChessEngine::engineReady, this, &Qt_Chess::onEngineReady);
    connect(m_chessEngine, &ChessEngine::bestMoveFound, this, &Qt_Chess::onEngineBestMove);
    connect(m_chessEngine, &ChessEngine::engineError, this, &Qt_Chess::onEngineError);
    connect(m_chessEngine, &ChessEngine::thinkingStarted, this, [this]() {
        if (m_thinkingLabel) m_thinkingLabel->show();
    });
    connect(m_chessEngine, &ChessEngine::thinkingStopped, this, [this]() {
        if (m_thinkingLabel) m_thinkingLabel->hide();
    });
    
    // 嘗試啟動引擎
    QString enginePath = getEnginePath();
    if (!enginePath.isEmpty() && QFile::exists(enginePath)) {
        m_chessEngine->startEngine(enginePath);
    }
}

QString Qt_Chess::getEnginePath() const {
    // 優先尋找與執行檔同目錄的引擎
    QString appDir = QCoreApplication::applicationDirPath();
    
    // 檢查多種可能的引擎檔案名稱
    QStringList engineNames;
#ifdef Q_OS_WIN
    engineNames << "stockfish.exe" << "stockfish-windows-x86-64-avx2.exe" 
                << "stockfish-windows.exe" << "engine/stockfish.exe"
                << "engine/stockfish-windows-x86-64-avx2.exe";
#else
    engineNames << "stockfish" << "stockfish-linux" << "stockfish-ubuntu-x86-64-avx2"
                << "engine/stockfish" << "engine/stockfish-linux";
#endif
    
    // 在應用程式目錄搜尋
    for (const QString& name : engineNames) {
        QString path = appDir + "/" + name;
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    // 在專案的 engine 目錄搜尋（開發時使用）
    QString projectEngineDir = appDir + "/../engine";
    for (const QString& name : engineNames) {
        QString path = projectEngineDir + "/" + name;
        if (QFile::exists(path)) {
            return path;
        }
        // 移除 engine/ 前綴
        QString baseName = name;
        if (baseName.startsWith("engine/")) {
            baseName = baseName.mid(7);
        }
        path = projectEngineDir + "/" + baseName;
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    // 在原始碼的 engine 目錄搜尋（源碼目錄）
    QString srcEngineDir = QString(QCoreApplication::applicationDirPath() + "/../../engine");
    for (const QString& name : engineNames) {
        QString baseName = name;
        if (baseName.startsWith("engine/")) {
            baseName = baseName.mid(7);
        }
        QString path = srcEngineDir + "/" + baseName;
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    return QString();
}

void Qt_Chess::onHumanModeClicked() {
    m_currentGameMode = GameMode::HumanVsHuman;
    m_isOnlineGame = false;  // 確保切換到離線模式
    updateGameModeUI();
    
    // 更新引擎的遊戲模式
    if (m_chessEngine) {
        m_chessEngine->setGameMode(m_currentGameMode);
    }
    
    // 儲存設定
    saveEngineSettings();
}

void Qt_Chess::onComputerModeClicked() {
    // 切換到電腦模式，顯示選邊按鈕
    // 預設選擇執白（如果尚未選擇）
    if (m_currentGameMode == GameMode::HumanVsHuman) {
        m_currentGameMode = GameMode::HumanVsComputer;
    }
    
    m_isOnlineGame = false;  // 確保切換到離線模式
    updateGameModeUI();
    
    // 更新引擎的遊戲模式
    if (m_chessEngine) {
        m_chessEngine->setGameMode(m_currentGameMode);
    }
    
    // 儲存設定
    saveEngineSettings();
}

void Qt_Chess::onWhiteColorClicked() {
    m_isRandomColorSelected = false;  // 清除隨機選擇標記
    
    // 使顏色選擇按鈕互斥（一次只能選一個）
    if (m_whiteButton) m_whiteButton->setChecked(true);
    if (m_randomButton) m_randomButton->setChecked(false);
    if (m_blackButton) m_blackButton->setChecked(false);
    
    // 線上模式：記錄房主選擇的顏色
    if (m_isOnlineGame) {
        m_onlineHostSelectedColor = PieceColor::White;
    } else {
        // 電腦模式
        m_currentGameMode = GameMode::HumanVsComputer;
        updateGameModeUI();
        
        if (m_chessEngine) {
            m_chessEngine->setGameMode(m_currentGameMode);
        }
        saveEngineSettings();
    }
}

void Qt_Chess::onRandomColorClicked() {
    // 設定隨機選擇標記
    m_isRandomColorSelected = true;
    
    // 使顏色選擇按鈕互斥（一次只能選一個）
    if (m_whiteButton) m_whiteButton->setChecked(false);
    if (m_randomButton) m_randomButton->setChecked(true);
    if (m_blackButton) m_blackButton->setChecked(false);
    
    // 線上模式：隨機選擇顏色
    if (m_isOnlineGame) {
        if (QRandomGenerator::global()->bounded(2) == 0) {
            m_onlineHostSelectedColor = PieceColor::White;
        } else {
            m_onlineHostSelectedColor = PieceColor::Black;
        }
    } else {
        // 電腦模式：隨機選擇執白或執黑
        if (QRandomGenerator::global()->bounded(2) == 0) {
            m_currentGameMode = GameMode::HumanVsComputer;
        } else {
            m_currentGameMode = GameMode::ComputerVsHuman;
        }
        updateGameModeUI();
        
        if (m_chessEngine) {
            m_chessEngine->setGameMode(m_currentGameMode);
        }
        saveEngineSettings();
    }
}

void Qt_Chess::onBlackColorClicked() {
    m_isRandomColorSelected = false;  // 清除隨機選擇標記
    
    // 使顏色選擇按鈕互斥（一次只能選一個）
    if (m_whiteButton) m_whiteButton->setChecked(false);
    if (m_randomButton) m_randomButton->setChecked(false);
    if (m_blackButton) m_blackButton->setChecked(true);
    
    // 線上模式：記錄房主選擇的顏色
    if (m_isOnlineGame) {
        m_onlineHostSelectedColor = PieceColor::Black;
    } else {
        // 電腦模式
        m_currentGameMode = GameMode::ComputerVsHuman;
        updateGameModeUI();
        
        if (m_chessEngine) {
            m_chessEngine->setGameMode(m_currentGameMode);
        }
        saveEngineSettings();
    }
}

void Qt_Chess::updateGameModeUI() {
    bool isHumanMode = (m_currentGameMode == GameMode::HumanVsHuman);
    
    // 更新按鈕選中狀態
    if (m_humanModeButton) {
        m_humanModeButton->setChecked(isHumanMode);
    }
    if (m_computerModeButton) {
        m_computerModeButton->setChecked(!isHumanMode);
    }
    
    // 更新選邊按鈕
    if (m_colorSelectionWidget) {
        m_colorSelectionWidget->setVisible(!isHumanMode);
    }
    if (m_whiteButton) {
        // 如果是隨機選擇，不高亮執白按鈕
        m_whiteButton->setChecked(!m_isRandomColorSelected && m_currentGameMode == GameMode::HumanVsComputer);
    }
    if (m_randomButton) {
        // 如果是隨機選擇，保持隨機按鈕高亮
        m_randomButton->setChecked(m_isRandomColorSelected);
    }
    if (m_blackButton) {
        // 如果是隨機選擇，不高亮執黑按鈕
        m_blackButton->setChecked(!m_isRandomColorSelected && m_currentGameMode == GameMode::ComputerVsHuman);
    }
    
    // 隱藏狀態標籤（不顯示執白/執黑）
    if (m_gameModeStatusLabel) {
        m_gameModeStatusLabel->hide();
    }
    
    // 更新難度控制的可見性
    if (m_difficultyLabel) m_difficultyLabel->setVisible(!isHumanMode);
    if (m_difficultyValueLabel) m_difficultyValueLabel->setVisible(!isHumanMode);
    if (m_difficultySlider) m_difficultySlider->setVisible(!isHumanMode);
}

void Qt_Chess::onDifficultyChanged(int value) {
    if (!m_difficultyValueLabel || !m_chessEngine) return;
    
    // 使用輔助函數計算 ELO 評分和中文難度名稱
    int elo = calculateElo(value);
    QString diffName = getDifficultyName(value);
    
    // 更新顯示的難度值（顯示中文難度名稱和 ELO）
    QString diffText = QString("%1 (ELO %2)").arg(diffName).arg(elo);
    m_difficultyValueLabel->setText(diffText);
    
    // 更新引擎難度
    m_chessEngine->setDifficulty(value);
    
    // 根據難度調整思考時間
    // 較低難度：較短思考時間（最小50ms）；較高難度：較長思考時間
    int thinkingTime = 50 + (value * 125);  // 50ms 到 2550ms
    m_chessEngine->setThinkingTime(thinkingTime);
    
    // 根據難度調整搜尋深度（與難度綁定）
    // Level 0 (ELO 250) = depth 1, Level 20 (ELO 3250) = depth 21
    int depth = 1 + value;  // depth 1-21
    m_chessEngine->setSearchDepth(depth);
    
    // 儲存設定
    saveEngineSettings();
}

void Qt_Chess::onEngineBestMove(const QString& move) {
    if (move.isEmpty() || !m_gameStarted || m_isReplayMode) return;
    
    // 解析 UCI 格式的移動
    QPoint from, to;
    PieceType promotionType;
    ChessEngine::uciToMove(move, from, to, promotionType);
    
    if (from.x() < 0 || to.x() < 0) {
        qWarning() << "Invalid engine move:" << move;
        return;
    }
    
    // 在執行移動之前檢測移動類型
    bool isCapture = isCaptureMove(from, to);
    bool isCastling = isCastlingMove(from, to);
    
    // 執行引擎的移動
    if (m_chessBoard.movePiece(from, to)) {
        // 記錄上一步移動用於高亮顯示
        m_lastMoveFrom = from;
        m_lastMoveTo = to;
        
        // 記錄 UCI 格式的移動
        m_uciMoveHistory.append(move);
        
        // 為剛完成移動的玩家應用時間增量
        applyIncrement();
        
        updateBoard();
        
        // 處理升變
        if (m_chessBoard.needsPromotion(to)) {
            // 引擎的升變類型已經包含在移動中
            if (promotionType != PieceType::None) {
                m_chessBoard.promotePawn(to, promotionType);
            } else {
                // 預設升變為后
                m_chessBoard.promotePawn(to, PieceType::Queen);
            }
            updateBoard();
        }
        
        // 更新棋譜列表
        updateMoveList();
        
        // 播放適當的音效
        playSoundForMove(isCapture, isCastling);
        
        // 更新時間顯示
        updateTimeDisplays();
        
        updateStatus();
    }
}

void Qt_Chess::onEngineReady() {
    // 引擎已準備好，可以開始遊戲
    if (m_chessEngine) {
        GameMode mode = getCurrentGameMode();
        m_chessEngine->setGameMode(mode);
        
        if (m_difficultySlider) {
            m_chessEngine->setDifficulty(m_difficultySlider->value());
        }
    }
}

void Qt_Chess::onEngineError(const QString& error) {
    // 顯示引擎錯誤訊息，但不阻止遊戲進行（可以繼續雙人對弈）
    qWarning() << "Chess engine error:" << error;
    
    // 如果引擎無法使用，切換回雙人對弈模式
    if (m_currentGameMode != GameMode::HumanVsHuman) {
        // 切換回雙人對弈模式
        m_currentGameMode = GameMode::HumanVsHuman;
        updateGameModeUI();
        QMessageBox::warning(this, "引擎錯誤", 
            QString("無法啟動棋譜引擎：%1\n\n已切換為雙人對弈模式。").arg(error));
    }
}

void Qt_Chess::requestEngineMove() {
    if (!m_chessEngine || !m_chessEngine->isEngineRunning()) return;
    if (!m_gameStarted || m_isReplayMode) return;
    
    // 使用移動歷史設定當前位置
    m_chessEngine->setPositionFromMoves(m_uciMoveHistory);
    
    // 請求引擎計算最佳走法
    m_chessEngine->requestMove();
}

bool Qt_Chess::isComputerTurn() const {
    if (!m_chessEngine) return false;
    
    GameMode mode = getCurrentGameMode();
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    
    switch (mode) {
        case GameMode::HumanVsComputer:
            // 人執白，電腦執黑
            return currentPlayer == PieceColor::Black;
        case GameMode::ComputerVsHuman:
            // 電腦執白，人執黑
            return currentPlayer == PieceColor::White;
        case GameMode::OnlineGame:
        case GameMode::HumanVsHuman:
        default:
            return false;
    }
}

GameMode Qt_Chess::getCurrentGameMode() const {
    return m_currentGameMode;
}

bool Qt_Chess::isPlayerPiece(PieceColor pieceColor) const {
    GameMode mode = getCurrentGameMode();
    
    switch (mode) {
        case GameMode::HumanVsComputer:
            // 人執白，電腦執黑
            return pieceColor == PieceColor::White;
        case GameMode::ComputerVsHuman:
            // 電腦執白，人執黑
            return pieceColor == PieceColor::Black;
        case GameMode::OnlineGame:
            // 線上對戰，只有本地玩家的顏色是玩家的
            if (m_networkManager) {
                return pieceColor == m_networkManager->getPlayerColor();
            }
            return true;
        case GameMode::HumanVsHuman:
        default:
            // 雙人對弈，任何顏色都是玩家的
            return true;
    }
}

void Qt_Chess::loadEngineSettings() {
    QSettings settings("Qt_Chess", "ChessEngine");
    
    int gameMode = settings.value("gameMode", static_cast<int>(GameMode::HumanVsHuman)).toInt();
    int difficulty = settings.value("difficulty", 0).toInt();  // 預設初學者
    
    // 設定遊戲模式
    m_currentGameMode = static_cast<GameMode>(gameMode);
    updateGameModeUI();
    
    if (m_difficultySlider) {
        m_difficultySlider->setValue(difficulty);
        onDifficultyChanged(difficulty);  // 更新顯示（同時設定搜尋深度）
    }
}

void Qt_Chess::saveEngineSettings() {
    QSettings settings("Qt_Chess", "ChessEngine");
    
    settings.setValue("gameMode", static_cast<int>(m_currentGameMode));
    
    if (m_difficultySlider) {
        settings.setValue("difficulty", m_difficultySlider->value());
    }
    
    settings.sync();
}

void Qt_Chess::applyModernStylesheet() {
    // 現代科技風格全局樣式表
    QString styleSheet = QString(
        // 主視窗背景
        "QMainWindow { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 %1, stop:0.5 %2, stop:1 %1); "
        "}"
        
        // 中央部件
        "QWidget#centralwidget { "
        "  background: transparent; "
        "}"
        
        // 選單欄
        "QMenuBar { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %3, stop:1 %1); "
        "  color: %4; "
        "  border-bottom: 2px solid %5; "
        "  padding: 4px 8px; "
        "  font-weight: bold; "
        "}"
        "QMenuBar::item { "
        "  padding: 6px 12px; "
        "  background: transparent; "
        "  border-radius: 4px; "
        "}"
        "QMenuBar::item:selected { "
        "  background: rgba(0, 217, 255, 0.3); "
        "  color: %5; "
        "}"
        "QMenuBar::item:pressed { "
        "  background: rgba(0, 217, 255, 0.5); "
        "}"
        
        // 下拉選單
        "QMenu { "
        "  background-color: %1; "
        "  border: 2px solid %5; "
        "  border-radius: 8px; "
        "  padding: 4px; "
        "}"
        "QMenu::item { "
        "  padding: 8px 24px; "
        "  color: %4; "
        "  border-radius: 4px; "
        "}"
        "QMenu::item:selected { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(0, 217, 255, 0.4), stop:1 rgba(233, 69, 96, 0.4)); "
        "  color: white; "
        "}"
        "QMenu::separator { "
        "  height: 2px; "
        "  background: %6; "
        "  margin: 4px 8px; "
        "}"
        
        // 群組框
        "QGroupBox { "
        "  font-weight: bold; "
        "  color: %5; "
        "  border: 2px solid %6; "
        "  border-radius: 10px; "
        "  margin-top: 12px; "
        "  padding-top: 10px; "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 rgba(15, 52, 96, 0.9), stop:1 rgba(26, 26, 46, 0.9)); "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  subcontrol-position: top left; "
        "  padding: 4px 12px; "
        "  color: %5; "
        "  background: %3; "
        "  border: 1px solid %5; "
        "  border-radius: 6px; "
        "  left: 10px; "
        "}"
        
        // 按鈕
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %3, stop:1 %1); "
        "  color: %4; "
        "  border: 2px solid %6; "
        "  border-radius: 8px; "
        "  padding: 8px 16px; "
        "  font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %3, stop:0.5 rgba(0, 217, 255, 0.3), stop:1 %1); "
        "  border: 2px solid %5; "
        "  color: %5; "
        "}"
        "QPushButton:pressed { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %1, stop:1 %3); "
        "  border: 2px solid %7; "
        "}"
        "QPushButton:disabled { "
        "  background: rgba(30, 30, 50, 0.6); "
        "  color: #666; "
        "  border: 2px solid #444; "
        "}"
        "QPushButton:checked { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %5, stop:1 rgba(0, 217, 255, 0.6)); "
        "  color: %1; "
        "  border: 2px solid %5; "
        "}"
        
        // 標籤
        "QLabel { "
        "  color: %4; "
        "  font-weight: normal; "
        "}"
        
        // 滑桿
        "QSlider::groove:horizontal { "
        "  border: 1px solid %6; "
        "  height: 8px; "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %1, stop:1 %3); "
        "  border-radius: 4px; "
        "}"
        "QSlider::handle:horizontal { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %5, stop:1 rgba(0, 217, 255, 0.7)); "
        "  border: 2px solid %5; "
        "  width: 18px; "
        "  margin: -6px 0; "
        "  border-radius: 9px; "
        "}"
        "QSlider::handle:horizontal:hover { "
        "  background: %5; "
        "  border: 2px solid white; "
        "}"
        "QSlider::sub-page:horizontal { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %5, stop:1 %7); "
        "  border-radius: 4px; "
        "}"
        
        // 列表視窗
        "QListWidget { "
        "  background-color: rgba(26, 26, 46, 0.95); "
        "  border: 2px solid %6; "
        "  border-radius: 8px; "
        "  color: %4; "
        "  alternate-background-color: rgba(15, 52, 96, 0.5); "
        "}"
        "QListWidget::item { "
        "  padding: 6px; "
        "  border-radius: 4px; "
        "}"
        "QListWidget::item:selected { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(0, 217, 255, 0.5), stop:1 rgba(233, 69, 96, 0.3)); "
        "  color: white; "
        "}"
        "QListWidget::item:hover { "
        "  background: rgba(0, 217, 255, 0.2); "
        "}"
        
        // 進度條
        "QProgressBar { "
        "  border: 2px solid %6; "
        "  border-radius: 6px; "
        "  background-color: rgba(26, 26, 46, 0.9); "
        "  text-align: center; "
        "  color: %4; "
        "}"
        "QProgressBar::chunk { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %8, stop:1 %5); "
        "  border-radius: 4px; "
        "}"
        
        // 狀態欄
        "QStatusBar { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 %1, stop:1 %3); "
        "  color: %4; "
        "  border-top: 2px solid %5; "
        "}"
        
        // 訊息框
        "QMessageBox { "
        "  background-color: %1; "
        "}"
        "QMessageBox QLabel { "
        "  color: %4; "
        "}"
        
        // 對話框
        "QDialog { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 %1, stop:0.5 %2, stop:1 %1); "
        "}"
        
        // 滾動條
        "QScrollBar:vertical { "
        "  border: none; "
        "  background: %1; "
        "  width: 12px; "
        "  margin: 0; "
        "  border-radius: 6px; "
        "}"
        "QScrollBar::handle:vertical { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 %6, stop:1 %5); "
        "  min-height: 30px; "
        "  border-radius: 6px; "
        "}"
        "QScrollBar::handle:vertical:hover { "
        "  background: %5; "
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "  height: 0; "
        "}"
    ).arg(THEME_BG_DARK, THEME_BG_MEDIUM, THEME_BG_PANEL, THEME_TEXT_PRIMARY, 
          THEME_ACCENT_PRIMARY, THEME_BORDER, THEME_ACCENT_SECONDARY, THEME_ACCENT_SUCCESS);
    
    setStyleSheet(styleSheet);
}

void Qt_Chess::playGameStartAnimation() {
    // 創建動畫疊加層（如果尚未創建）
    if (!m_animationOverlay) {
        m_animationOverlay = new QWidget(this);
        m_animationOverlay->setObjectName("animationOverlay");
        m_animationOverlay->setStyleSheet(
            "QWidget#animationOverlay { "
            "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
            "    stop:0 rgba(26, 26, 46, 0.95), "
            "    stop:0.5 rgba(15, 52, 96, 0.95), "
            "    stop:1 rgba(26, 26, 46, 0.95)); "
            "}"
        );
    }
    
    // 創建動畫標籤（如果尚未創建）
    if (!m_animationLabel) {
        m_animationLabel = new QLabel(m_animationOverlay);
        m_animationLabel->setAlignment(Qt::AlignCenter);
    }
    
    // 初始化動畫計時器（只在構造時連接一次）
    if (!m_animationTimer) {
        m_animationTimer = new QTimer(this);
        connect(m_animationTimer, &QTimer::timeout, this, &Qt_Chess::onAnimationStep);
    }
    
    // 每次播放動畫時更新疊加層大小以匹配當前視窗大小
    QRect windowRect = rect();
    m_animationOverlay->setGeometry(windowRect);
    m_animationLabel->setGeometry(0, 0, windowRect.width(), windowRect.height());
    
    // 開始動畫
    m_animationStep = 0;
    m_animationOverlay->raise();
    m_animationOverlay->show();
    
    // 顯示第一幀
    onAnimationStep();
    
    // 啟動動畫計時器（每 800ms 更新一次）
    m_animationTimer->start(800);
}

void Qt_Chess::onAnimationStep() {
    if (!m_animationLabel || !m_animationOverlay) return;
    
    QString text;
    QString style;
    
    // 根據動畫步驟設置不同的文字和樣式
    // 已移除 3-2-1 倒數，直接顯示開始訊息
    switch (m_animationStep) {
        case 0:
            text = "⚔ 對弈開始 ⚔";
            style = QString(
                "QLabel { "
                "  color: %1; "
                "  font-size: 48px; "
                "  font-weight: bold; "
                "  font-family: 'Arial', sans-serif; "
                "  background: transparent; "
                "}"
            ).arg(THEME_ACCENT_SUCCESS);
            break;
        default:
            // 動畫結束
            m_animationTimer->stop();
            finishGameStartAnimation();
            return;
    }
    
    m_animationLabel->setText(text);
    m_animationLabel->setStyleSheet(style);
    m_animationStep++;
}

void Qt_Chess::finishGameStartAnimation() {
    // 隱藏動畫疊加層
    if (m_animationOverlay) {
        m_animationOverlay->hide();
    }
    
    // 遊戲開始動畫結束後開始播放背景音樂（只在遊戲進行中播放）
    if (m_gameStarted) {
        startBackgroundMusic();
    }
    
    // 如果有待處理的遊戲開始，現在執行它
    if (m_pendingGameStart) {
        m_pendingGameStart = false;
        
        // 執行實際的遊戲開始邏輯（在動畫期間被延遲）
        // 如果是電腦先走（玩家執黑），請求引擎走棋
        if (isComputerTurn()) {
            QTimer::singleShot(300, this, &Qt_Chess::requestEngineMove);
        }
    }
}

void Qt_Chess::playStartupAnimation() {
    // 創建動畫疊加層（如果尚未創建）
    if (!m_animationOverlay) {
        m_animationOverlay = new QWidget(this);
        m_animationOverlay->setObjectName("animationOverlay");
        m_animationOverlay->setStyleSheet(
            "QWidget#animationOverlay { "
            "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
            "    stop:0 rgba(26, 26, 46, 0.98), "
            "    stop:0.5 rgba(15, 52, 96, 0.98), "
            "    stop:1 rgba(26, 26, 46, 0.98)); "
            "}"
        );
    }
    
    // 創建動畫標籤（如果尚未創建）
    if (!m_animationLabel) {
        m_animationLabel = new QLabel(m_animationOverlay);
        m_animationLabel->setAlignment(Qt::AlignCenter);
    }
    
    // 創建副標籤（用於顯示副標題）
    if (!m_animationSubLabel) {
        m_animationSubLabel = new QLabel(m_animationOverlay);
        m_animationSubLabel->setAlignment(Qt::AlignCenter);
    }
    
    // 初始化啟動動畫計時器（只在構造時連接一次）
    if (!m_startupAnimationTimer) {
        m_startupAnimationTimer = new QTimer(this);
        connect(m_startupAnimationTimer, &QTimer::timeout, this, &Qt_Chess::onStartupAnimationStep);
    }
    
    // 每次播放動畫時更新疊加層大小以匹配當前視窗大小
    QRect windowRect = rect();
    m_animationOverlay->setGeometry(windowRect);
    m_animationLabel->setGeometry(0, windowRect.height() / 4, windowRect.width(), windowRect.height() / 2);
    m_animationSubLabel->setGeometry(0, windowRect.height() * 2 / 3, windowRect.width(), windowRect.height() / 4);
    
    // 開始動畫
    m_startupAnimationStep = 0;
    m_animationOverlay->raise();
    m_animationOverlay->show();
    
    // 顯示第一幀並開始淡入效果
    onStartupAnimationStep();
    
    // 啟動動畫計時器（每 700ms 更新一次，更流暢）
    m_startupAnimationTimer->start(700);
}

void Qt_Chess::playStartupTextAnimation(QLabel* label, const QString& text, const QString& color, int fontSize) {
    if (!label) return;
    
    // 設置文字和樣式
    label->setText(text);
    label->setStyleSheet(QString(
        "QLabel { "
        "  color: %1; "
        "  font-size: %2px; "
        "  font-weight: bold; "
        "  font-family: 'Arial', sans-serif; "
        "  background: transparent; "
        "}"
    ).arg(color).arg(fontSize));
}

void Qt_Chess::onStartupAnimationStep() {
    if (!m_animationLabel || !m_animationOverlay) return;
    
    // 動態啟動動畫序列：多階段視覺效果
    switch (m_startupAnimationStep) {
        case 0:
            // 第一幀：白色棋子符號，從上方滑入
            playStartupTextAnimation(m_animationLabel, "♔ ♕ ♖ ♗ ♘ ♙", THEME_ACCENT_PRIMARY, 80);
            if (m_animationSubLabel) m_animationSubLabel->setText("");
            break;
        case 1:
            // 第二幀：標題「科技對弈」放大顯示
            playStartupTextAnimation(m_animationLabel, "⚡ 科技對弈 ⚡", THEME_ACCENT_WARNING, 72);
            if (m_animationSubLabel) {
                m_animationSubLabel->setStyleSheet(QString(
                    "QLabel { color: %1; font-size: 24px; background: transparent; }"
                ).arg(THEME_ACCENT_PRIMARY));
                m_animationSubLabel->setText("TECH CHESS BATTLE");
            }
            break;
        case 2:
            // 第三幀：黑色棋子符號
            playStartupTextAnimation(m_animationLabel, "♚ ♛ ♜ ♝ ♞ ♟", THEME_ACCENT_SECONDARY, 80);
            if (m_animationSubLabel) m_animationSubLabel->setText("");
            break;
        case 3:
            // 第四幀：準備就緒
            playStartupTextAnimation(m_animationLabel, "🎮 準備就緒 🎮", THEME_ACCENT_SUCCESS, 64);
            if (m_animationSubLabel) {
                m_animationSubLabel->setStyleSheet(QString(
                    "QLabel { color: %1; font-size: 20px; background: transparent; }"
                ).arg(THEME_ACCENT_SUCCESS));
                m_animationSubLabel->setText("READY TO PLAY");
            }
            break;
        default:
            // 動畫結束
            m_startupAnimationTimer->stop();
            finishStartupAnimation();
            return;
    }
    
    m_startupAnimationStep++;
}

void Qt_Chess::finishStartupAnimation() {
    // 隱藏動畫疊加層
    if (m_animationOverlay) {
        m_animationOverlay->hide();
    }
    
    // 啟動動畫結束後不播放背景音樂
    // 背景音樂只在遊戲開始時播放
}

void Qt_Chess::initializeBackgroundMusic() {
    // 創建背景音樂播放器 (Qt6 API)
    m_bgmPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_bgmPlayer->setAudioOutput(m_audioOutput);
    
    // 設定音量 (Qt6 使用 0.0-1.0 浮點數)
    m_audioOutput->setVolume(m_bgmVolume / 100.0);
    
    // 初始化背景音樂列表 - 使用 resources/backgroundsounds 中的5首音樂
    m_bgmList.clear();
    m_bgmList << "qrc:/resources/backgroundsounds/backgroundsound_1.mp3"
              << "qrc:/resources/backgroundsounds/backgroundsound_2.mp3"
              << "qrc:/resources/backgroundsounds/backgroundsound_3.mp3"
              << "qrc:/resources/backgroundsounds/backgroundsound_4.mp3"
              << "qrc:/resources/backgroundsounds/backgroundsound_5.mp3";
    
    // 設定循環播放 - 當媒體結束時重新播放 (Qt6 使用 playbackStateChanged)
    connect(m_bgmPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState && m_bgmEnabled && m_gameStarted) {
            // 媒體播放完畢，重新開始（只在遊戲進行中才循環播放）
            m_bgmPlayer->setPosition(0);
            m_bgmPlayer->play();
        }
    });
}

void Qt_Chess::startBackgroundMusic() {
    if (!m_bgmPlayer || !m_bgmEnabled || m_bgmList.isEmpty()) return;
    
    // 隨機選擇一首背景音樂，但不能與上一次相同
    int newIndex;
    if (m_bgmList.size() == 1) {
        newIndex = 0;
    } else {
        do {
            newIndex = QRandomGenerator::global()->bounded(m_bgmList.size());
        } while (newIndex == m_lastBgmIndex);
    }
    
    m_lastBgmIndex = newIndex;
    QString bgmPath = m_bgmList[newIndex];
    
    // 設定並播放背景音樂 (Qt6 使用 setSource)
    m_bgmPlayer->setSource(QUrl(bgmPath));
    m_bgmPlayer->play();
}

void Qt_Chess::stopBackgroundMusic() {
    if (m_bgmPlayer) {
        m_bgmPlayer->stop();
    }
}

void Qt_Chess::toggleBackgroundMusic() {
    m_bgmEnabled = !m_bgmEnabled;
    if (m_bgmEnabled && m_gameStarted) {
        // 只有在遊戲進行中才啟動背景音樂
        startBackgroundMusic();
    } else {
        stopBackgroundMusic();
    }
}

void Qt_Chess::onToggleBackgroundMusicClicked() {
    toggleBackgroundMusic();
}

void Qt_Chess::setBackgroundMusicVolume(int volume) {
    m_bgmVolume = qBound(0, volume, 100);
    if (m_audioOutput) {
        m_audioOutput->setVolume(m_bgmVolume / 100.0);
    }
}

void Qt_Chess::onCheckForUpdatesClicked() {
    // 標記為手動檢查
    m_manualUpdateCheck = true;
    
    // 顯示檢查中訊息（使用 QPointer 防止懸空指標）
    QPointer<QMessageBox> checkingBox = new QMessageBox(this);
    checkingBox->setWindowTitle("檢查更新");
    checkingBox->setText("正在檢查更新...");
    checkingBox->setStandardButtons(QMessageBox::NoButton);
    checkingBox->setModal(false);
    checkingBox->setAttribute(Qt::WA_DeleteOnClose); // 確保關閉時自動刪除
    checkingBox->show();
    
    // 開始檢查更新
    m_updateChecker->checkForUpdates();
    
    // 當檢查完成時關閉訊息框（使用 SingleShot 連接避免重複連接）
    // 使用 QPointer 檢查對話框是否仍然有效
    connect(m_updateChecker, &UpdateChecker::updateCheckFinished, this, [checkingBox]() {
        if (checkingBox) {
            checkingBox->close();
        }
    }, Qt::SingleShotConnection);
    connect(m_updateChecker, &UpdateChecker::updateCheckFailed, this, [checkingBox]() {
        if (checkingBox) {
            checkingBox->close();
        }
    }, Qt::SingleShotConnection);
}

void Qt_Chess::onUpdateCheckFinished(bool updateAvailable) {
    if (updateAvailable) {
        QString currentVersion = UpdateChecker::getCurrentVersion();
        QString latestVersion = m_updateChecker->getLatestVersion();
        QString releaseUrl = m_updateChecker->getReleaseUrl();
        QString releaseNotes = m_updateChecker->getReleaseNotes();
        
        // 格式化更新說明（如果太長則截斷）
        QString formattedNotes;
        if (releaseNotes.isEmpty()) {
            formattedNotes = "無更新說明";
        } else if (releaseNotes.length() > RELEASE_NOTES_PREVIEW_LENGTH) {
            formattedNotes = releaseNotes.left(RELEASE_NOTES_PREVIEW_LENGTH) + "...";
        } else {
            formattedNotes = releaseNotes;
        }
        
        // 建立訊息內容
        QString message = QString(
            "發現新版本！\n\n"
            "目前版本：%1\n"
            "最新版本：%2\n\n"
            "更新說明：\n%3\n\n"
            "是否前往下載頁面？"
        ).arg(currentVersion, latestVersion, formattedNotes);
        
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("有可用更新");
        msgBox.setText(message);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, "前往下載");
        msgBox.setButtonText(QMessageBox::No, "稍後再說");
        
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            // 開啟瀏覽器到 GitHub 發行頁面
            QDesktopServices::openUrl(QUrl(releaseUrl));
        }
    } else if (m_manualUpdateCheck) {
        // 只有在手動檢查時才顯示「已是最新版本」訊息
        QMessageBox::information(this, "已是最新版本", 
            QString("您目前使用的是最新版本 %1").arg(UpdateChecker::getCurrentVersion()));
    }
    
    // 重設手動檢查標記
    m_manualUpdateCheck = false;
}

void Qt_Chess::onUpdateCheckFailed(const QString& error) {
    // 只有在手動檢查時才顯示錯誤訊息
    if (m_manualUpdateCheck) {
        QMessageBox::warning(this, "檢查更新失敗", 
            QString("無法檢查更新：%1").arg(error));
    }
    
    // 重設手動檢查標記
    m_manualUpdateCheck = false;
}

// ==================== 線上對戰功能 ====================

void Qt_Chess::initializeNetwork() {
    m_networkManager = new NetworkManager(this);
    
    // 連接信號
    connect(m_networkManager, &NetworkManager::connected, this, &Qt_Chess::onNetworkConnected);
    connect(m_networkManager, &NetworkManager::disconnected, this, &Qt_Chess::onNetworkDisconnected);
    connect(m_networkManager, &NetworkManager::connectionError, this, &Qt_Chess::onNetworkError);
    connect(m_networkManager, &NetworkManager::roomCreated, this, &Qt_Chess::onRoomCreated);
    connect(m_networkManager, &NetworkManager::opponentJoined, this, &Qt_Chess::onOpponentJoined);
    connect(m_networkManager, &NetworkManager::playerLeft, this, &Qt_Chess::onPlayerLeft);
    connect(m_networkManager, &NetworkManager::promotedToHost, this, &Qt_Chess::onPromotedToHost);
    connect(m_networkManager, &NetworkManager::opponentMove, this, &Qt_Chess::onOpponentMove);
    connect(m_networkManager, &NetworkManager::gameStartReceived, this, &Qt_Chess::onGameStartReceived);
    connect(m_networkManager, &NetworkManager::startGameReceived, this, &Qt_Chess::onStartGameReceived);
    connect(m_networkManager, &NetworkManager::timeSettingsReceived, this, &Qt_Chess::onTimeSettingsReceived);
    connect(m_networkManager, &NetworkManager::timerStateReceived, this, &Qt_Chess::onTimerStateReceived);
    connect(m_networkManager, &NetworkManager::surrenderReceived, this, &Qt_Chess::onSurrenderReceived);
    connect(m_networkManager, &NetworkManager::opponentDisconnected, this, &Qt_Chess::onOpponentDisconnected);
}

void Qt_Chess::onOnlineModeClicked() {
    if (!m_onlineModeButton->isChecked()) {
        m_onlineModeButton->setChecked(true);
        return;
    }
    
    // 如果已在線上模式，先關閉連線
    if (m_isOnlineGame) {
        m_networkManager->closeConnection();
        m_isOnlineGame = false;
        m_waitingForOpponent = false;
    }
    
    // 取消其他模式
    m_humanModeButton->setChecked(false);
    m_computerModeButton->setChecked(false);
    
    // 隱藏電腦模式相關UI（但保留顏色選擇widget用於線上模式）
    m_difficultyLabel->hide();
    m_difficultyValueLabel->hide();
    m_difficultySlider->hide();
    m_gameModeStatusLabel->hide();
    
    // 停止引擎
    if (m_chessEngine) {
        m_chessEngine->stopEngine();
    }
    
    // 顯示線上對戰對話框
    OnlineDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        OnlineDialog::Mode mode = dialog.getMode();
        
        if (mode == OnlineDialog::Mode::CreateRoom) {
            // 創建房間
            if (m_networkManager->createRoom()) {
                m_currentGameMode = GameMode::OnlineGame;
                m_isOnlineGame = true;
                m_waitingForOpponent = true;
                
                m_connectionStatusLabel->setText("🔄 等待對手加入...");
                m_connectionStatusLabel->show();
                m_roomInfoLabel->show();
                
                // 顯示顏色選擇widget讓房主選擇要執的顏色
                if (m_colorSelectionWidget) {
                    m_colorSelectionWidget->show();
                }
                
                // 停用新遊戲功能
                if (m_newGameAction) {
                    m_newGameAction->setEnabled(false);
                }
                
                // 停用雙人和電腦模式按鈕（連線上後不能切換模式）
                if (m_humanModeButton) m_humanModeButton->setEnabled(false);
                if (m_computerModeButton) m_computerModeButton->setEnabled(false);
                
                // 修改開始按鈕為取消功能（紅色）
                if (m_startButton) {
                    m_startButton->setText("✗ 取消等待");
                    m_startButton->setEnabled(true);
                    m_startButton->setStyleSheet(QString(
                        "QPushButton { "
                        "  background-color: #f44336; "
                        "  color: white; "
                        "  border: 3px solid #d32f2f; "
                        "  border-radius: 10px; "
                        "  padding: 8px; "
                        "  font-weight: bold; "
                        "  min-height: 45px; "
                        "}"
                        "QPushButton:hover { "
                        "  background-color: #d32f2f; "
                        "}"
                        "QPushButton:pressed { "
                        "  background-color: #c62828; "
                        "}"
                    ));
                    disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
                    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
                }
                
                // 不要立即開始遊戲，等待對手加入
            } else {
                QMessageBox::warning(this, "創建房間失敗", "無法創建房間，請稍後再試");
                m_onlineModeButton->setChecked(false);
                m_humanModeButton->setChecked(true);
            }
        } else if (mode == OnlineDialog::Mode::JoinRoom) {
            // 加入房間
            QString roomNumber = dialog.getRoomNumber();
            
            if (roomNumber.isEmpty()) {
                QMessageBox::warning(this, "輸入錯誤", "請輸入有效的房間號碼");
                m_onlineModeButton->setChecked(false);
                m_humanModeButton->setChecked(true);
                return;
            }
            
            if (m_networkManager->joinRoom(roomNumber)) {
                m_currentGameMode = GameMode::OnlineGame;
                m_isOnlineGame = true;
                
                m_connectionStatusLabel->setText("🔄 正在連接...");
                m_connectionStatusLabel->show();
                
                // 房客不顯示顏色選擇widget
                if (m_colorSelectionWidget) {
                    m_colorSelectionWidget->hide();
                }
                
                // 房客：禁用時間控制（只有房主可以設定時間）
                if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(false);
                if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(false);
                if (m_incrementSlider) m_incrementSlider->setEnabled(false);
                
                // 停用新遊戲功能
                if (m_newGameAction) {
                    m_newGameAction->setEnabled(false);
                }
                
                // 停用雙人和電腦模式按鈕（連線上後不能切換模式）
                if (m_humanModeButton) m_humanModeButton->setEnabled(false);
                if (m_computerModeButton) m_computerModeButton->setEnabled(false);
                
                // 修改開始按鈕為取消功能（紅色）
                if (m_startButton) {
                    m_startButton->setText("✗ 取消連接");
                    m_startButton->setEnabled(true);
                    m_startButton->setStyleSheet(QString(
                        "QPushButton { "
                        "  background-color: #f44336; "
                        "  color: white; "
                        "  border: 3px solid #d32f2f; "
                        "  border-radius: 10px; "
                        "  padding: 8px; "
                        "  font-weight: bold; "
                        "  min-height: 45px; "
                        "}"
                        "QPushButton:hover { "
                        "  background-color: #d32f2f; "
                        "}"
                        "QPushButton:pressed { "
                        "  background-color: #c62828; "
                        "}"
                    ));
                    disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
                    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
                }
            } else {
                QMessageBox::warning(this, "加入失敗", "無法加入房間");
                m_onlineModeButton->setChecked(false);
                m_humanModeButton->setChecked(true);
            }
        }
    } else {
        // 用戶取消，返回雙人模式
        m_onlineModeButton->setChecked(false);
        m_humanModeButton->setChecked(true);
    }
}

void Qt_Chess::onNetworkConnected() {
    m_connectionStatusLabel->setText("✅ 已連接");
    updateConnectionStatus();
}

void Qt_Chess::onNetworkDisconnected() {
    m_connectionStatusLabel->setText("❌ 已斷線");
    m_isOnlineGame = false;
    m_waitingForOpponent = false;
    
    // 恢復新遊戲功能
    if (m_newGameAction) {
        m_newGameAction->setEnabled(true);
    }
    
    // 隱藏顏色選擇widget
    if (m_colorSelectionWidget) {
        m_colorSelectionWidget->hide();
    }
    
    updateConnectionStatus();
}

void Qt_Chess::onNetworkError(const QString& error) {
    QMessageBox::warning(this, "網路錯誤", error);
    m_connectionStatusLabel->setText("❌ 連線錯誤");
    m_isOnlineGame = false;
    m_waitingForOpponent = false;
    
    // 隱藏退出房間按鈕
    if (m_exitRoomButton) {
        m_exitRoomButton->hide();
    }
    
    // 恢復開始按鈕的原始功能和樣式
    if (m_startButton) {
        m_startButton->show();  // 確保按鈕顯示
        disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
        connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
        m_startButton->setText("▶ 開始對弈");
        m_startButton->setEnabled(true);
        m_startButton->setStyleSheet(QString(
            "QPushButton { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
            "  color: %2; "
            "  border: 3px solid %1; "
            "  border-radius: 12px; "
            "  padding: 10px; "
            "}"
            "QPushButton:hover { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
            "  border-color: white; "
            "}"
            "QPushButton:pressed { "
            "  background: %1; "
            "}"
            "QPushButton:disabled { "
            "  background: rgba(50, 50, 70, 0.6); "
            "  color: #666; "
            "  border-color: #444; "
            "}"
        ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
    }
    
    // 恢復時間控制
    if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
    if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
    if (m_incrementSlider) m_incrementSlider->setEnabled(true);
    
    // 返回雙人模式
    m_onlineModeButton->setChecked(false);
    m_humanModeButton->setChecked(true);
    m_currentGameMode = GameMode::HumanVsHuman;
    m_connectionStatusLabel->hide();
    m_roomInfoLabel->hide();
}

void Qt_Chess::onRoomCreated(const QString& roomNumber) {
    showRoomInfoDialog(roomNumber);
    
    // 房間創建成功後，立即顯示退出房間按鈕和等待狀態
    m_waitingForOpponent = true;
    
    // 更新狀態標籤
    m_connectionStatusLabel->setText("⏳ 等待對手加入...");
    
    // 顯示退出房間按鈕（而非開始按鈕）
    if (m_startButton) {
        m_startButton->hide();
    }
    if (m_exitRoomButton) {
        m_exitRoomButton->show();
    }
}

void Qt_Chess::onOpponentJoined() {
    m_waitingForOpponent = false;
    
    // 檢查角色：只有房主有開始按鈕，房客等待房主開始
    bool isHost = (m_networkManager->getRole() == NetworkRole::Host);
    
    if (isHost) {
        // 房主：對手已加入，可以開始遊戲
        m_connectionStatusLabel->setText("✅ 對手已加入，請按開始鍵開始遊戲");
        
        if (m_startButton) {
            m_startButton->setText("▶ 開始");
            m_startButton->setEnabled(true);
            m_startButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");  // 綠色開始按鈕
            m_startButton->show();  // 確保按鈕顯示
            
            // 重新連接到開始遊戲功能
            disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
            connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
        }
        
        // 房主顯示退出房間按鈕
        if (m_exitRoomButton) {
            m_exitRoomButton->show();
        }
    } else {
        // 房客：成功加入房間，等待房主開始
        m_connectionStatusLabel->setText("✅ 已加入房間，等待房主開始遊戲...");
        
        // 房客隱藏開始按鈕，只顯示退出按鈕
        if (m_startButton) {
            m_startButton->hide();
        }
        
        // 房客顯示退出房間按鈕
        if (m_exitRoomButton) {
            m_exitRoomButton->show();
        }
    }
    
    // 房主：等待客戶端確認連線後再開始遊戲
    // 遊戲將在收到 gameStartReceived 信號時開始
}

void Qt_Chess::onPlayerLeft() {
    // 對手在遊戲開始前離開房間
    qDebug() << "[Qt_Chess::onPlayerLeft] Opponent left the room before game started";
    
    // 只有房主會收到這個通知（因為只有房主在等待對手）
    if (m_networkManager->getRole() == NetworkRole::Host) {
        m_waitingForOpponent = true;
        
        // 更新狀態標籤
        m_connectionStatusLabel->setText("⏳ 對手已離開，等待新對手加入...");
        
        // 隱藏開始按鈕
        if (m_startButton) {
            m_startButton->hide();
        }
        
        // 保持退出房間按鈕可見
        if (m_exitRoomButton) {
            m_exitRoomButton->show();
        }
    }
}

void Qt_Chess::onPromotedToHost() {
    // 房主離開，自己被提升為新房主
    qDebug() << "[Qt_Chess::onPromotedToHost] Promoted from Guest to Host";
    
    // 獲取房號用於顯示
    QString roomNumber = m_networkManager ? m_networkManager->getRoomNumber() : QString();
    
    // 更新狀態為等待對手
    m_waitingForOpponent = true;
    
    // 更新狀態標籤顯示角色變更和等待狀態
    m_connectionStatusLabel->setText(tr("👑 您已成為房主 | 原房主已離開，等待新對手加入..."));
    
    // 更新房間資訊標籤顯示房號（與初始創建房間時一致）
    if (m_roomInfoLabel && !roomNumber.isEmpty()) {
        m_roomInfoLabel->setText(QString("🎮 房號: %1").arg(roomNumber));
        m_roomInfoLabel->show();
    }
    
    // 顯示顏色選擇widget讓新房主選擇要執的顏色（與初始房主一致）
    if (m_colorSelectionWidget) {
        m_colorSelectionWidget->show();
    }
    
    // 啟用時間控制滑桿（新房主可以調整時間設定）
    if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
    if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
    if (m_incrementSlider) m_incrementSlider->setEnabled(true);
    
    // 隱藏開始按鈕（因為還沒有對手）
    if (m_startButton) {
        m_startButton->hide();
    }
    
    // 顯示退出房間按鈕
    if (m_exitRoomButton) {
        m_exitRoomButton->show();
    }
}

void Qt_Chess::onOpponentMove(const QPoint& from, const QPoint& to, PieceType promotionType) {
    qDebug() << "[Qt_Chess::onOpponentMove] Received opponent move: from" << from << "to" << to;
    
    // 對手的移動 - 直接執行移動，movePiece 會自動切換回合
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    
    qDebug() << "[Qt_Chess::onOpponentMove] Current player before move:" << (int)currentPlayer;
    
    // 直接移動對手的棋子，movePiece 會驗證並自動切換回合
    if (m_chessBoard.movePiece(from, to)) {
        qDebug() << "[Qt_Chess::onOpponentMove] Move successful, current player after move:" << (int)m_chessBoard.getCurrentPlayer();
        
        // 檢查是否需要升變
        if (promotionType != PieceType::None && m_chessBoard.needsPromotion(to)) {
            m_chessBoard.promotePawn(to, promotionType);
        }
        
        updateBoard();
        updateStatus();
        updateMoveList();
        updateCapturedPiecesDisplay();
        
        // 強制更新和重繪UI，確保棋盤變化立即顯示
        if (m_boardWidget) {
            m_boardWidget->update();
            m_boardWidget->repaint();
        }
        update();
        repaint();
        QApplication::processEvents();
        
        // 播放音效
        bool isCapture = isCaptureMove(from, to);
        bool isCastling = isCastlingMove(from, to);
        playSoundForMove(isCapture, isCastling);
        
        // 檢查將軍
        if (m_chessBoard.isInCheck(m_chessBoard.getCurrentPlayer())) {
            m_checkSound.play();
        }
        
        // 應用時間增量
        if (m_timeControlEnabled && m_timerStarted) {
            applyIncrement();
        }
    } else {
        qDebug() << "[Qt_Chess::onOpponentMove] Move failed!";
    }
}

void Qt_Chess::onGameStartReceived(PieceColor playerColor) {
    m_connectionStatusLabel->setText("✅ 連線成功！遊戲開始");
    
    // 恢復開始按鈕的原始功能和樣式
    if (m_startButton) {
        disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
        connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
        m_startButton->setText("▶ 開始對弈");
        m_startButton->setEnabled(true);
        m_startButton->setStyleSheet(QString(
            "QPushButton { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
            "  color: %2; "
            "  border: 3px solid %1; "
            "  border-radius: 12px; "
            "  padding: 10px; "
            "}"
            "QPushButton:hover { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
            "  border-color: white; "
            "}"
            "QPushButton:pressed { "
            "  background: %1; "
            "}"
            "QPushButton:disabled { "
            "  background: rgba(50, 50, 70, 0.6); "
            "  color: #666; "
            "  border-color: #444; "
            "}"
        ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
        
        // 房主和房客的UI狀態
        if (m_networkManager->getRole() == NetworkRole::Host) {
            // 房主：啟用開始按鈕和時間控制
            m_startButton->setEnabled(true);
            if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
            if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
            if (m_incrementSlider) m_incrementSlider->setEnabled(true);
        } else {
            // 房客：隱藏開始按鈕，禁用時間控制
            m_startButton->hide();
            m_connectionStatusLabel->setText("✅ 連線成功！等待房主開始遊戲...");
            if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(false);
            if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(false);
            if (m_incrementSlider) m_incrementSlider->setEnabled(false);
        }
    }
    
    // 顯示遊戲開始訊息
    QString roleMsg = (m_networkManager->getRole() == NetworkRole::Host) ? 
        "已成功連線到對手，點擊「開始對弈」開始遊戲！" : 
        "已成功連線到對手，等待房主開始遊戲...";
    QMessageBox::information(this, "連線成功", roleMsg);
    
    // 如果玩家執黑，翻轉棋盤
    if (playerColor == PieceColor::Black && !m_isBoardFlipped) {
        m_isBoardFlipped = true;
        updateBoard();
    }
    
    // 不再自動開始遊戲，改由房主點擊開始按鈕
}

void Qt_Chess::onOpponentDisconnected() {
    // 獲取房號用於顯示
    QString roomNumber = m_networkManager ? m_networkManager->getRoomNumber() : QString();
    
    // 檢查遊戲是否已開始或正在進行中，如果是則自動結束遊戲並退出棋盤
    // 即使遊戲剛開始還沒有走任何一步，也要結束遊戲
    if (m_gameStarted || m_timerStarted) {
        // 更新連線狀態標籤顯示對手退出和遊戲結束
        m_connectionStatusLabel->setText(QString("❌ 對手已退出遊戲 | 遊戲自動結束"));
        
        // 結束遊戲並更新狀態
        handleGameEnd();
        updateStatus();
        
        // 確保遊戲完全重置到初始狀態
        resetBoardState();
    } else {
        // 更新連線狀態標籤顯示對手斷線
        m_connectionStatusLabel->setText(QString("❌ 對手已斷開連接"));
    }
    
    // 更新房間資訊標籤顯示房號
    if (m_roomInfoLabel && !roomNumber.isEmpty()) {
        m_roomInfoLabel->setText(QString("🎮 房號: %1").arg(roomNumber));
        m_roomInfoLabel->show();
    }
    
    m_isOnlineGame = false;
    m_waitingForOpponent = false;
    
    // 隱藏退出房間按鈕
    if (m_exitRoomButton) {
        m_exitRoomButton->hide();
    }
    
    // 恢復開始按鈕的原始功能和樣式
    if (m_startButton) {
        m_startButton->show();  // 確保按鈕顯示
        disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
        connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
        m_startButton->setText("▶ 開始對弈");
        m_startButton->setEnabled(true);
        m_startButton->setStyleSheet(QString(
            "QPushButton { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
            "  color: %2; "
            "  border: 3px solid %1; "
            "  border-radius: 12px; "
            "  padding: 10px; "
            "}"
            "QPushButton:hover { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
            "  border-color: white; "
            "}"
            "QPushButton:pressed { "
            "  background: %1; "
            "}"
            "QPushButton:disabled { "
            "  background: rgba(50, 50, 70, 0.6); "
            "  color: #666; "
            "  border-color: #444; "
            "}"
        ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
    }
    
    // 恢復時間控制
    if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
    if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
    if (m_incrementSlider) m_incrementSlider->setEnabled(true);
    
    // 恢復模式選擇按鈕
    if (m_humanModeButton) m_humanModeButton->setEnabled(true);
    if (m_computerModeButton) m_computerModeButton->setEnabled(true);
    
    // 返回雙人模式
    m_onlineModeButton->setChecked(false);
    m_humanModeButton->setChecked(true);
    m_currentGameMode = GameMode::HumanVsHuman;
    m_connectionStatusLabel->hide();
    m_roomInfoLabel->hide();
}

void Qt_Chess::onCancelRoomClicked() {
    // 用戶取消等待或連接
    QString message = m_waitingForOpponent ? "確定要取消等待對手加入嗎？" : "確定要取消連接嗎？";
    
    int response = QMessageBox::question(this, "取消", 
        message, 
        QMessageBox::Yes | QMessageBox::No);
    
    if (response == QMessageBox::Yes) {
        // 使用 leaveRoom 明確通知對手
        m_networkManager->leaveRoom();
        
        m_isOnlineGame = false;
        m_waitingForOpponent = false;
        
        // 恢復開始按鈕的原始功能和樣式
        if (m_startButton) {
            m_startButton->show();  // 確保按鈕顯示
            disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
            connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
            m_startButton->setText("▶ 開始對弈");
            m_startButton->setEnabled(true);
            m_startButton->setStyleSheet(QString(
                "QPushButton { "
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
                "  color: %2; "
                "  border: 3px solid %1; "
                "  border-radius: 12px; "
                "  padding: 10px; "
                "}"
                "QPushButton:hover { "
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
                "  border-color: white; "
                "}"
                "QPushButton:pressed { "
                "  background: %1; "
                "}"
                "QPushButton:disabled { "
                "  background: rgba(50, 50, 70, 0.6); "
                "  color: #666; "
                "  border-color: #444; "
                "}"
            ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
        }
        
        // 恢復時間控制
        if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
        if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
        if (m_incrementSlider) m_incrementSlider->setEnabled(true);
        
        // 恢復模式選擇按鈕
        if (m_humanModeButton) m_humanModeButton->setEnabled(true);
        if (m_computerModeButton) m_computerModeButton->setEnabled(true);
        
        // 恢復新遊戲功能
        if (m_newGameAction) {
            m_newGameAction->setEnabled(true);
        }
        
        // 隱藏顏色選擇widget
        if (m_colorSelectionWidget) {
            m_colorSelectionWidget->hide();
        }
        
        // 返回雙人模式
        m_onlineModeButton->setChecked(false);
        m_humanModeButton->setChecked(true);
        m_currentGameMode = GameMode::HumanVsHuman;
        m_connectionStatusLabel->hide();
        m_roomInfoLabel->hide();
        
        QMessageBox::information(this, "已取消", "已取消連線，返回雙人模式");
    }
}

void Qt_Chess::onExitRoomClicked() {
    // 在遊戲進行中退出房間 - 移除確認對話框以減少延遲
    // int response = QMessageBox::question(this, "退出房間", 
    //     "確定要退出線上對戰嗎？這將結束當前遊戲。", 
    //     QMessageBox::Yes | QMessageBox::No);
    
    // if (response == QMessageBox::Yes) {
        // 首先停止計時器，避免計時器在清理過程中觸發
        stopTimer();
        m_timerStarted = false;
        
        // 設定標記，表示正在退出線上模式
        bool wasOnlineGame = m_isOnlineGame;
        
        // 隱藏退出房間按鈕
        if (m_exitRoomButton) {
            m_exitRoomButton->hide();
        }
        
        // 隱藏線上UI元素
        if (m_connectionStatusLabel) {
            m_connectionStatusLabel->hide();
        }
        if (m_roomInfoLabel) {
            m_roomInfoLabel->hide();
        }
        
        // 恢復開始按鈕
        if (m_startButton) {
            m_startButton->show();
            disconnect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onCancelRoomClicked);
            connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
            m_startButton->setText("▶ 開始對弈");
            m_startButton->setEnabled(true);
            m_startButton->setStyleSheet(QString(
                "QPushButton { "
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "    stop:0 %1, stop:0.5 rgba(0, 255, 136, 0.8), stop:1 %1); "
                "  color: %2; "
                "  border: 3px solid %1; "
                "  border-radius: 12px; "
                "  padding: 10px; "
                "}"
                "QPushButton:hover { "
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "    stop:0 %1, stop:0.3 rgba(0, 255, 136, 0.9), stop:0.7 rgba(0, 217, 255, 0.9), stop:1 %1); "
                "  border-color: white; "
                "}"
                "QPushButton:pressed { "
                "  background: %1; "
                "}"
                "QPushButton:disabled { "
                "  background: rgba(50, 50, 70, 0.6); "
                "  color: #666; "
                "  border-color: #444; "
                "}"
            ).arg(THEME_ACCENT_SUCCESS, THEME_BG_DARK));
        }
        
        // 恢復時間控制
        if (m_whiteTimeLimitSlider) m_whiteTimeLimitSlider->setEnabled(true);
        if (m_blackTimeLimitSlider) m_blackTimeLimitSlider->setEnabled(true);
        if (m_incrementSlider) m_incrementSlider->setEnabled(true);
        
        // 恢復模式選擇按鈕
        if (m_humanModeButton) m_humanModeButton->setEnabled(true);
        if (m_computerModeButton) m_computerModeButton->setEnabled(true);
        
        // 恢復新遊戲功能
        if (m_newGameAction) {
            m_newGameAction->setEnabled(true);
        }
        
        // 隱藏顏色選擇widget
        if (m_colorSelectionWidget) {
            m_colorSelectionWidget->hide();
        }
        
        // 返回雙人模式
        if (m_onlineModeButton) m_onlineModeButton->setChecked(false);
        if (m_humanModeButton) m_humanModeButton->setChecked(true);
        m_currentGameMode = GameMode::HumanVsHuman;
        
        // 關閉網路連線（在重置遊戲狀態之前關閉，確保訊息處理完成）
        if (m_networkManager) {
            m_networkManager->leaveRoom();  // 使用 leaveRoom 明確通知對手
        }
        
        // 重置線上模式標記（在關閉連接後）
        m_isOnlineGame = false;
        m_waitingForOpponent = false;
        
        // 只有在確實是線上遊戲時才重置棋盤
        if (wasOnlineGame) {
            onNewGameClicked();
        }
        
        // 移除對話框以減少延遲
        // QMessageBox::information(this, "已退出", "已退出線上對戰，返回雙人模式");
    // } // 移除 if (response == QMessageBox::Yes) 的結束括號
}

void Qt_Chess::onStartGameReceived(int whiteTimeMs, int blackTimeMs, int incrementMs, PieceColor hostColor, qint64 serverTimeOffset) {
    qDebug() << "[Qt_Chess::onStartGameReceived] Client received StartGame"
             << "| Host color:" << (hostColor == PieceColor::White ? "White" : "Black")
             << "| whiteTimeMs:" << whiteTimeMs
             << "| blackTimeMs:" << blackTimeMs
             << "| serverTimeOffset:" << serverTimeOffset << "ms";
    
    // 儲存伺服器時間偏移和遊戲開始時間，用於線上模式的時間同步
    m_serverTimeOffset = serverTimeOffset;
    m_gameStartLocalTime = QDateTime::currentMSecsSinceEpoch();
    m_currentTurnStartTime = m_gameStartLocalTime + m_serverTimeOffset;  // 初始化當前回合開始時間（使用同步時間）
    
    // 收到房主的開始遊戲通知，設定時間後客戶端自動開始遊戲
    
    // 設定時間值（房主設定的時間）
    m_whiteTimeMs = whiteTimeMs;
    m_blackTimeMs = blackTimeMs;
    m_whiteInitialTimeMs = whiteTimeMs;
    m_blackInitialTimeMs = blackTimeMs;
    m_incrementMs = incrementMs;  // 設定增量值
    
    // 設定增量值（僅用於顯示，房客滑桿已停用）
    if (m_incrementSlider) {
        m_incrementSlider->setValue(incrementMs / 1000);  // 轉換為秒
    }
    
    // 檢查是否啟用時間控制
    m_timeControlEnabled = (whiteTimeMs > 0 || blackTimeMs > 0);
    
    qDebug() << "[Qt_Chess::onStartGameReceived] m_timeControlEnabled:" << m_timeControlEnabled;
    
    // ===== 直接初始化棋盤，不呼叫 onNewGameClicked() =====
    // 因為 onNewGameClicked() 會重置 m_gameStarted = false 和從滑桿讀取時間
    
    // 如果在回放模式中，先退出
    if (m_isReplayMode) {
        exitReplayMode();
    }
    
    // 初始化棋盤
    m_chessBoard.initializeBoard();
    m_pieceSelected = false;
    m_uciMoveHistory.clear();
    
    // 停止背景音樂
    stopBackgroundMusic();
    
    // 重置上一步移動高亮
    m_lastMoveFrom = QPoint(-1, -1);
    m_lastMoveTo = QPoint(-1, -1);
    
    // 停止引擎思考（線上模式不使用引擎）
    if (m_chessEngine) {
        m_chessEngine->stop();
        m_chessEngine->newGame();
    }
    
    // 根據房主選擇的顏色決定棋盤翻轉和玩家顏色
    // 如果房主選擇黑色，則房主的棋盤翻轉，房客的棋盤不翻轉
    // 如果房主選擇白色，則房主的棋盤不翻轉，房客的棋盤翻轉
    if (m_networkManager && m_networkManager->getRole() == NetworkRole::Guest) {
        // 房客的棋盤翻轉與房主相反
        m_isBoardFlipped = (hostColor == PieceColor::White);
        saveBoardFlipSettings();
    } else if (m_networkManager && m_networkManager->getRole() == NetworkRole::Host) {
        // 房主根據自己的選擇決定是否翻轉（執黑則翻轉）
        m_isBoardFlipped = (hostColor == PieceColor::Black);
        saveBoardFlipSettings();
        
        // 確保房主選擇的顏色與本地記錄一致
        m_onlineHostSelectedColor = hostColor;
    }
    
    // 將時間和吃子紀錄恢復到右側面板
    restoreWidgetsFromGameEnd();
    
    // 顯示右側時間面板
    if (m_rightTimePanel) {
        m_rightTimePanel->show();
    }
    
    // 隱藏時間控制面板
    if (m_timeControlPanel) {
        m_timeControlPanel->hide();
    }
    
    // 清空棋譜列表
    if (m_moveListWidget) m_moveListWidget->clear();
    
    // ===== 啟動遊戲 =====
    m_gameStarted = true;  // 設定為 true，允許走棋
    m_timerStarted = true;
    
    qDebug() << "[Qt_Chess::onStartGameReceived] Game starting synchronously for both players"
             << "| My role:" << (m_networkManager->getRole() == NetworkRole::Host ? "Host" : "Guest")
             << "| Player color:" << (m_networkManager ? (int)m_networkManager->getPlayerColor() : -1);
    
    // 顯示放棄按鈕和退出房間按鈕（無論是否有時間控制）
    if (m_giveUpButton) {
        m_giveUpButton->show();
    }
    if (m_exitRoomButton) {
        m_exitRoomButton->show();
    }
    
    // 更新開始按鈕
    if (m_startButton) {
        if (m_networkManager->getRole() == NetworkRole::Host) {
            // 房主顯示「進行中」
            m_startButton->setText("進行中");
            m_startButton->setEnabled(false);
            m_startButton->show();
        } else {
            // 房客隱藏開始按鈕
            m_startButton->hide();
        }
    }
    
    // 更新回放按鈕狀態（遊戲開始時停用）
    updateReplayButtons();
    
    // 當遊戲開始時，將右側伸展設為 1
    setRightPanelStretch(1);
    
    // 更新棋盤和狀態
    updateBoard();
    updateStatus();
    updateTimeDisplays();
    
    // 如果啟用了時間控制，啟動計時器並顯示時間
    if (m_timeControlEnabled) {
        startTimer();
        
        // 在棋盤左右兩側顯示時間和進度條（必須在 updateTimeDisplays 之後）
        if (m_whiteTimeLabel) {
            m_whiteTimeLabel->show();
        }
        if (m_blackTimeLabel) {
            m_blackTimeLabel->show();
        }
        if (m_whiteTimeProgressBar) {
            m_whiteTimeProgressBar->show();
        }
        if (m_blackTimeProgressBar) {
            m_blackTimeProgressBar->show();
        }
    } else {
        // 隱藏時間標籤和進度條
        if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
        if (m_blackTimeLabel) m_blackTimeLabel->hide();
        if (m_whiteTimeProgressBar) m_whiteTimeProgressBar->hide();
        if (m_blackTimeProgressBar) m_blackTimeProgressBar->hide();
    }
    
    // 播放遊戲開始動畫
    m_pendingGameStart = isComputerTurn();
    playGameStartAnimation();
    
    // 強制更新UI，確保時間標籤和棋盤正確顯示
    if (m_boardWidget) {
        m_boardWidget->update();
        m_boardWidget->repaint();
    }
    
    if (m_timeControlEnabled) {
        if (m_whiteTimeLabel) {
            m_whiteTimeLabel->update();
            m_whiteTimeLabel->repaint();
        }
        if (m_blackTimeLabel) {
            m_blackTimeLabel->update();
            m_blackTimeLabel->repaint();
        }
        if (m_whiteTimeProgressBar) {
            m_whiteTimeProgressBar->update();
            m_whiteTimeProgressBar->repaint();
        }
        if (m_blackTimeProgressBar) {
            m_blackTimeProgressBar->update();
            m_blackTimeProgressBar->repaint();
        }
    }
    
    // 強制處理所有待處理的UI事件
    QApplication::processEvents();
    
    // 清除任何殘留的高亮顯示
    clearHighlights();
    
    // 移除對話框以減少延遲
    // QMessageBox::information(this, "遊戲開始", "對手已開始遊戲！");
}

void Qt_Chess::onTimerStateReceived(qint64 timeA, qint64 timeB, const QString& currentPlayer, qint64 lastSwitchTime) {
    qDebug() << "[Qt_Chess::onTimerStateReceived] Received timer state"
             << "| timeA:" << timeA
             << "| timeB:" << timeB
             << "| currentPlayer:" << currentPlayer
             << "| lastSwitchTime:" << lastSwitchTime;
    
    // 儲存伺服器計時器狀態
    m_serverTimeA = timeA;
    m_serverTimeB = timeB;
    m_serverCurrentPlayer = currentPlayer;
    m_serverLastSwitchTime = lastSwitchTime;
    m_useServerTimer = true;  // 啟用伺服器計時器模式
    m_lastServerUpdateTime = QDateTime::currentMSecsSinceEpoch();  // 記錄更新時間
    
    // 立即更新顯示
    updateTimeDisplaysFromServer();
}

void Qt_Chess::onTimeSettingsReceived(int whiteTimeMs, int blackTimeMs, int incrementMs) {
    // 房客收到房主的時間設定更新
    // 只有房客才需要更新（房主自己已經設定好了）
    if (m_networkManager && m_networkManager->getRole() == NetworkRole::Guest) {
        // 更新時間變數
        m_whiteTimeMs = whiteTimeMs;
        m_blackTimeMs = blackTimeMs;
        m_whiteInitialTimeMs = whiteTimeMs;
        m_blackInitialTimeMs = blackTimeMs;
        
        // 更新增量
        m_incrementMs = incrementMs;
        
        // 更新時間控制啟用狀態
        m_timeControlEnabled = (whiteTimeMs > 0 || blackTimeMs > 0);
        
        // 更新滑桿顯示（僅用於顯示，房客的滑桿已被停用）
        // 更新白方時間滑桿
        if (m_whiteTimeLimitSlider) {
            m_whiteTimeLimitSlider->blockSignals(true);
            int sliderValue;
            if (whiteTimeMs == 0) {
                sliderValue = 0;  // 無限制
            } else if (whiteTimeMs == 30000) {
                sliderValue = 1;  // 30秒
            } else {
                // 分鐘數：value = (minutes + 1)，反推 minutes = value - 1
                sliderValue = (whiteTimeMs / 60000) + 1;
            }
            m_whiteTimeLimitSlider->setValue(sliderValue);
            m_whiteTimeLimitSlider->blockSignals(false);
        }
        
        // 更新黑方時間滑桿
        if (m_blackTimeLimitSlider) {
            m_blackTimeLimitSlider->blockSignals(true);
            int sliderValue;
            if (blackTimeMs == 0) {
                sliderValue = 0;  // 無限制
            } else if (blackTimeMs == 30000) {
                sliderValue = 1;  // 30秒
            } else {
                // 分鐘數：value = (minutes + 1)，反推 minutes = value - 1
                sliderValue = (blackTimeMs / 60000) + 1;
            }
            m_blackTimeLimitSlider->setValue(sliderValue);
            m_blackTimeLimitSlider->blockSignals(false);
        }
        
        // 更新增量滑桿
        if (m_incrementSlider && m_incrementLabel) {
            m_incrementSlider->blockSignals(true);
            m_incrementSlider->setValue(incrementMs / 1000);
            m_incrementLabel->setText(QString("%1秒").arg(incrementMs / 1000));
            m_incrementSlider->blockSignals(false);
        }
        
        // 更新時間顯示標籤
        if (m_whiteTimeLimitLabel) {
            int minutes = whiteTimeMs / 60000;
            if (minutes == 0) {
                m_whiteTimeLimitLabel->setText("無限制");
            } else {
                m_whiteTimeLimitLabel->setText(QString("%1分鐘").arg(minutes));
            }
        }
        
        if (m_blackTimeLimitLabel) {
            int minutes = blackTimeMs / 60000;
            if (minutes == 0) {
                m_blackTimeLimitLabel->setText("無限制");
            } else {
                m_blackTimeLimitLabel->setText(QString("%1分鐘").arg(minutes));
            }
        }
        
        // 如果遊戲尚未開始，更新時間顯示
        if (!m_gameStarted) {
            updateTimeDisplays();
        }
    }
}

void Qt_Chess::onSurrenderReceived() {
    // 收到對手投降訊息
    PieceColor opponentColor = m_networkManager->getOpponentColor();
    
    // 設置遊戲結果
    if (opponentColor == PieceColor::White) {
        m_chessBoard.setGameResult(GameResult::WhiteResigns);
    } else {
        m_chessBoard.setGameResult(GameResult::BlackResigns);
    }
    
    // 處理遊戲結束
    handleGameEnd();
    
    // 顯示訊息
    QString opponentName = (opponentColor == PieceColor::White) ? "白方" : "黑方";
    QString winner = (opponentColor == PieceColor::White) ? "黑方" : "白方";
    QMessageBox::information(this, "對手投降", QString("%1投降！%2獲勝！").arg(opponentName).arg(winner));
}

void Qt_Chess::updateConnectionStatus() {
    if (m_isOnlineGame) {
        m_connectionStatusLabel->show();
    } else {
        m_connectionStatusLabel->hide();
    }
}

bool Qt_Chess::isOnlineTurn() const {
    if (!m_isOnlineGame) {
        return true;  // 非線上模式，總是可以移動
    }
    
    if (m_waitingForOpponent) {
        return false;  // 等待對手加入
    }
    
    // 檢查是否輪到本地玩家
    PieceColor playerColor = m_networkManager->getPlayerColor();
    return m_chessBoard.getCurrentPlayer() == playerColor;
}

void Qt_Chess::showRoomInfoDialog(const QString& roomNumber) {
    // 創建自訂對話框
    QDialog dialog(this);
    dialog.setWindowTitle(tr("🎉 房間已創建！"));
    dialog.setMinimumWidth(450);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // 標題
    QLabel* titleLabel = new QLabel(tr("<h2>✅ 房間創建成功！</h2>"), &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: #4CAF50; padding: 10px; }");
    layout->addWidget(titleLabel);
    
    // 說明文字
    QLabel* instructionLabel = new QLabel(
        tr("<p><b>📱 請將以下房號傳給您的朋友：</b></p>"), &dialog);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("QLabel { font-size: 11pt; padding: 5px; }");
    layout->addWidget(instructionLabel);
    
    // 房號顯示（大字體，可選取）
    QTextEdit* codeEdit = new QTextEdit(&dialog);
    codeEdit->setPlainText(roomNumber);
    codeEdit->setReadOnly(true);
    codeEdit->setMaximumHeight(60);
    codeEdit->setAlignment(Qt::AlignCenter);
    QFont codeFont = codeEdit->font();
    codeFont.setPointSize(16);
    codeFont.setBold(true);
    codeEdit->setFont(codeFont);
    codeEdit->setStyleSheet("QTextEdit { background-color: #E3F2FD; border: 2px solid #2196F3; border-radius: 5px; padding: 10px; }");
    layout->addWidget(codeEdit);
    
    // 複製按鈕
    QPushButton* copyButton = new QPushButton(tr("📋 複製房號"), &dialog);
    copyButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; font-size: 12pt; font-weight: bold; border-radius: 5px; }");
    connect(copyButton, &QPushButton::clicked, [roomNumber]() {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(roomNumber);
        // 移除對話框以減少延遲，直接複製
        // QMessageBox::information(nullptr, tr("已複製"), 
        //     tr("房號已複製到剪貼簿！\n\n請用通訊軟體（如LINE、WeChat）傳給朋友"));
    });
    layout->addWidget(copyButton);
    
    layout->addSpacing(10);
    
    // 詳細資訊 - 顯示房號和連線說明
    QLabel* detailLabel = new QLabel(
        tr("<p><b>房間資訊：</b><br>"
           "房間號碼：<span style='color: #2196F3; font-weight: bold;'>%1</span></p>"
           "<p style='color: #666; font-size: 9pt;'>"
           "💡 朋友收到房號後，選擇「加入房間」並貼上即可<br>"
           "🌐 使用中央伺服器，無需設定網路或防火牆</p>").arg(roomNumber), &dialog);
    detailLabel->setWordWrap(true);
    detailLabel->setStyleSheet("QLabel { padding: 10px; background-color: #f5f5f5; border-radius: 5px; }");
    layout->addWidget(detailLabel);
    
    layout->addSpacing(10);
    
    // 關閉按鈕
    QPushButton* closeButton = new QPushButton(tr("知道了"), &dialog);
    closeButton->setStyleSheet("QPushButton { padding: 8px; font-size: 11pt; }");
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    // 更新房間資訊標籤顯示房號
    m_roomInfoLabel->setText(QString("🎮 房號: %1").arg(roomNumber));
    
    dialog.exec();
}
