#include "qt_chess.h"
#include "ui_qt_chess.h"
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
#include <QSettings>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <QGroupBox>
#include <QFileDialog>
#include <QDate>
#include <QTextStream>
#include <QClipboard>
#include <QApplication>
#include <algorithm>

namespace {
const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
const int DEFAULT_ICON_SIZE = 40; // 預設圖示大小（像素）
const int MAX_TIME_LIMIT_SECONDS = 1800; // 最大時間限制：30 分鐘
const int MAX_SLIDER_POSITION = 31; // 滑桿範圍：0（無限制）、1（30秒）、2-31（1-30 分鐘）
const int MAX_MINUTES = 30; // 最大時間限制（分鐘）
const QString GAME_ENDED_TEXT = "遊戲結束"; // 遊戲結束時顯示的文字

// 視窗大小的佈局常數
const int PANEL_SPACING = 10;          // 面板之間的間距
const int BASE_MARGINS =   20;           // 基本佈局邊距（不包括棋盤容器的 2*BOARD_CONTAINER_MARGIN）
const int TIME_LABEL_SPACING = 0;     // 時間標籤周圍的間距（已禁用）
const int BOARD_CONTAINER_MARGIN = 0;  // 棋盤容器每側的邊距（已禁用）

// UI 元素的縮放常數
const int MIN_SQUARE_SIZE = 40;        // 棋盤格子的最小大小
const int MAX_SQUARE_SIZE = 170;       // 棋盤格子的最大大小
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

// PGN 格式常數
const int PGN_MOVES_PER_LINE = 6;            // PGN 檔案中每行的移動回合數

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
    , m_replayTitle(nullptr)
    , m_replayFirstButton(nullptr)
    , m_replayPrevButton(nullptr)
    , m_replayNextButton(nullptr)
    , m_replayLastButton(nullptr)
    , m_isReplayMode(false)
    , m_replayMoveIndex(-1)
    , m_savedCurrentPlayer(PieceColor::White)
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(900, 660);  // 增加寬度以容納時間控制面板

    // 設置最小視窗大小以確保所有內容都能完整顯示而不被裁切
    // 最小寬度：允許棋盤至少 8*MIN_SQUARE_SIZE 加上面板和邊距
    // 最小高度：允許棋盤和控制項合理顯示
    setMinimumSize(814, 420);

    setMouseTracking(true);

    loadSoundSettings();
    initializeSounds();
    loadPieceIconSettings();
    loadBoardColorSettings();
    loadBoardFlipSettings();
    loadPieceIconsToCache(); // 載入設定後將圖示載入快取
    setupMenuBar();
    setupUI();
    loadTimeControlSettings();  // 在 setupUI() 之後載入以確保元件存在
    updateBoard();
    updateStatus();
    updateTimeDisplays();
    updateReplayButtons();  // 設置回放按鈕初始狀態
    updateCapturedPiecesDisplay();  // 初始化被吃掉棋子顯示
}

Qt_Chess::~Qt_Chess()
{
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
    moveListLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* moveListTitle = new QLabel("棋譜", m_moveListPanel);
    moveListTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    moveListTitle->setFont(titleFont);
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

    // 匯出PGN按鈕（初始隱藏）
    m_exportPGNButton = new QPushButton("匯出 PGN", m_moveListPanel);
    m_exportPGNButton->hide();
    connect(m_exportPGNButton, &QPushButton::clicked, this, &Qt_Chess::onExportPGNClicked);
    moveListLayout->addWidget(m_exportPGNButton);

    // 複製棋譜按鈕（初始隱藏）
    m_copyPGNButton = new QPushButton("複製棋譜", m_moveListPanel);
    m_copyPGNButton->hide();
    connect(m_copyPGNButton, &QPushButton::clicked, this, &Qt_Chess::onCopyPGNClicked);
    moveListLayout->addWidget(m_copyPGNButton);

    // 回放控制按鈕（始終可見）
    m_replayTitle = new QLabel("回放控制", m_moveListPanel);
    m_replayTitle->setAlignment(Qt::AlignCenter);
    QFont replayFont;
    replayFont.setPointSize(10);
    replayFont.setBold(true);
    m_replayTitle->setFont(replayFont);
    moveListLayout->addWidget(m_replayTitle);

    QWidget* replayButtonContainer = new QWidget(m_moveListPanel);
    QGridLayout* replayButtonLayout = new QGridLayout(replayButtonContainer);
    replayButtonLayout->setContentsMargins(0, 0, 0, 0);
    replayButtonLayout->setSpacing(5);

    m_replayFirstButton = new QPushButton("⏮", replayButtonContainer);
    m_replayFirstButton->setToolTip("第一步");
    m_replayFirstButton->setEnabled(false);  // 初始停用
    connect(m_replayFirstButton, &QPushButton::clicked, this, &Qt_Chess::onReplayFirstClicked);
    replayButtonLayout->addWidget(m_replayFirstButton, 0, 0);

    m_replayPrevButton = new QPushButton("◀", replayButtonContainer);
    m_replayPrevButton->setToolTip("上一步");
    m_replayPrevButton->setEnabled(false);  // 初始停用
    connect(m_replayPrevButton, &QPushButton::clicked, this, &Qt_Chess::onReplayPrevClicked);
    replayButtonLayout->addWidget(m_replayPrevButton, 0, 1);

    m_replayNextButton = new QPushButton("▶", replayButtonContainer);
    m_replayNextButton->setToolTip("下一步");
    m_replayNextButton->setEnabled(false);  // 初始停用
    connect(m_replayNextButton, &QPushButton::clicked, this, &Qt_Chess::onReplayNextClicked);
    replayButtonLayout->addWidget(m_replayNextButton, 0, 2);

    m_replayLastButton = new QPushButton("⏭", replayButtonContainer);
    m_replayLastButton->setToolTip("最後一步");
    m_replayLastButton->setEnabled(false);  // 初始停用
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
    QVBoxLayout* boardContainerVLayout = new QVBoxLayout(m_boardContainer);
    boardContainerVLayout->setContentsMargins(BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN,
                                             BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN);
    boardContainerVLayout->setSpacing(5);

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
    m_blackTimeProgressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #333; border-radius: 3px; background-color: #444; }"
        "QProgressBar::chunk { background-color: #4CAF50; border-radius: 2px; }"
    );
    m_blackTimeProgressBar->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_blackTimeProgressBar, 0, Qt::AlignCenter);

    // 黑方時間標籤 - 初始隱藏
    m_blackTimeLabel = new QLabel("--:--", m_rightTimePanel);
    m_blackTimeLabel->setFont(timeFont);
    m_blackTimeLabel->setAlignment(Qt::AlignCenter);
    m_blackTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_blackTimeLabel->setFixedSize(100, 40);  // 固定大小
    m_blackTimeLabel->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_blackTimeLabel, 0, Qt::AlignCenter);

    // 白方時間標籤 - 初始隱藏
    m_whiteTimeLabel = new QLabel("--:--", m_rightTimePanel);
    m_whiteTimeLabel->setFont(timeFont);
    m_whiteTimeLabel->setAlignment(Qt::AlignCenter);
    m_whiteTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_whiteTimeLabel->setFixedSize(100, 40);  // 固定大小
    m_whiteTimeLabel->hide();  // 初始隱藏
    rightTimePanelLayout->addWidget(m_whiteTimeLabel, 0, Qt::AlignCenter);

    // 白方時間進度條 - 放在時間標籤下方，初始隱藏
    m_whiteTimeProgressBar = new QProgressBar(m_rightTimePanel);
    m_whiteTimeProgressBar->setMinimum(0);
    m_whiteTimeProgressBar->setMaximum(100);
    m_whiteTimeProgressBar->setValue(100);
    m_whiteTimeProgressBar->setTextVisible(false);
    m_whiteTimeProgressBar->setFixedWidth(100);  // 與時間標籤同寬
    m_whiteTimeProgressBar->setMaximumHeight(8);
    m_whiteTimeProgressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #333; border-radius: 3px; background-color: #444; }"
        "QProgressBar::chunk { background-color: #4CAF50; border-radius: 2px; }"
    );
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
    QMenu* gameMenu = m_menuBar->addMenu("遊戲");

    // 新遊戲動作
    QAction* newGameAction = new QAction("新遊戲", this);
    connect(newGameAction, &QAction::triggered, this, &Qt_Chess::onNewGameClicked);
    gameMenu->addAction(newGameAction);

    gameMenu->addSeparator();

    // 放棄動作
    QAction* giveUpAction = new QAction("放棄", this);
    connect(giveUpAction, &QAction::triggered, this, &Qt_Chess::onGiveUpClicked);
    gameMenu->addAction(giveUpAction);

    // 設定選單
    QMenu* settingsMenu = m_menuBar->addMenu("設定");

    // 音效設定動作
    QAction* soundSettingsAction = new QAction("音效設定", this);
    connect(soundSettingsAction, &QAction::triggered, this, &Qt_Chess::onSoundSettingsClicked);
    settingsMenu->addAction(soundSettingsAction);

    // 棋子圖標設定動作
    QAction* pieceIconSettingsAction = new QAction("棋子圖標設定", this);
    connect(pieceIconSettingsAction, &QAction::triggered, this, &Qt_Chess::onPieceIconSettingsClicked);
    settingsMenu->addAction(pieceIconSettingsAction);

    // 棋盤顏色設定動作
    QAction* boardColorSettingsAction = new QAction("棋盤顏色設定", this);
    connect(boardColorSettingsAction, &QAction::triggered, this, &Qt_Chess::onBoardColorSettingsClicked);
    settingsMenu->addAction(boardColorSettingsAction);

    settingsMenu->addSeparator();

    // 反轉棋盤動作
    QAction* flipBoardAction = new QAction("反轉棋盤", this);
    connect(flipBoardAction, &QAction::triggered, this, &Qt_Chess::onFlipBoardClicked);
    settingsMenu->addAction(flipBoardAction);
}

void Qt_Chess::updateSquareColor(int displayRow, int displayCol) {
    // 計算邏輯坐標以確定正確的淺色/深色模式
    int logicalRow = getLogicalRow(displayRow);
    int logicalCol = getLogicalCol(displayCol);
    bool isLight = (logicalRow + logicalCol) % 2 == 0;
    QColor color = isLight ? m_boardColorSettings.lightSquareColor : m_boardColorSettings.darkSquareColor;
    m_squares[displayRow][displayCol]->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid #333; }").arg(color.name())
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

void Qt_Chess::clearHighlights() {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            updateSquareColor(row, col);
        }
    }

    // 如果被將軍，重新應用國王的紅色背景
    applyCheckHighlight();
}

void Qt_Chess::highlightValidMoves() {
    clearHighlights();

    if (!m_pieceSelected) return;

    // 高亮選中的格子（m_selectedSquare 是邏輯坐標）
    int displayRow = getDisplayRow(m_selectedSquare.y());
    int displayCol = getDisplayCol(m_selectedSquare.x());
    m_squares[displayRow][displayCol]->setStyleSheet(
        "QPushButton { background-color: #7FC97F; border: 2px solid #00FF00; }"
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
                    // 將吃子移動高亮為紅色
                    QString color = isLight ? "#FFB3B3" : "#FF8080";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FF0000; }").arg(color)
                        );
                } else {
                    // 將非吃子移動高亮為橙色
                    QString color = isLight ? "#FFE4B5" : "#DEB887";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FFA500; }").arg(color)
                        );
                }
            }
        }
    }

    // 如果被將軍且國王不是選中的棋子，重新應用國王的紅色背景
    applyCheckHighlight(m_selectedSquare);
}

void Qt_Chess::onSquareClicked(int displayRow, int displayCol) {
    // 如果在回放模式中，不允許移動
    if (m_isReplayMode) {
        return;
    }

    // 如果遊戲尚未開始，不允許移動
    if (!m_gameStarted) {
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
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
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
            m_pieceSelected = false;

            // 為剛完成移動的玩家應用時間增量
            applyIncrement();

            updateBoard();

            // 檢查是否需要兵升變
            if (m_chessBoard.needsPromotion(clickedSquare)) {
                const ChessPiece& piece = m_chessBoard.getPiece(clickedSquare.y(), clickedSquare.x());
                PieceType promotionType = showPromotionDialog(piece.getColor());
                m_chessBoard.promotePawn(clickedSquare, promotionType);
                updateBoard();
            }

            // 更新棋譜列表
            updateMoveList();

            // 播放適當的音效
            playSoundForMove(isCapture, isCastling);

            // 更新時間顯示（計時器僅在已啟動時運行）
            updateTimeDisplays();

            updateStatus();
        } else if (clickedSquare == m_selectedSquare) {
            // 取消選擇棋子
            m_pieceSelected = false;
            clearHighlights();
        } else {
            // 嘗試選擇相同顏色的另一個棋子
            const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
            if (piece.getType() != PieceType::None &&
                piece.getColor() == m_chessBoard.getCurrentPlayer()) {
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

    // 重置時間控制
    stopTimer();
    m_timerStarted = false;

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

        m_timerStarted = true;
        m_gameStarted = true;  // 標記遊戲已開始
        startTimer();

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

        updateTimeDisplays();

        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("進行中");
        }

        // 更新回放按鈕狀態（遊戲開始時停用）
        updateReplayButtons();

        // 當遊戲開始時，將右側伸展設為 1
        setRightPanelStretch(1);
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

        // 即使沒有時間控制也允許遊戲開始
        m_gameStarted = true;

        // 隱藏時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }

        // 顯示放棄按鈕
        if (m_giveUpButton) {
            m_giveUpButton->show();
        }

        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("進行中");
        }

        // 更新回放按鈕狀態（遊戲開始時停用）
        updateReplayButtons();

        // 當遊戲開始時，將右側伸展設為 1
        setRightPanelStretch(1);
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
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {

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
                m_pieceSelected = false;

                // 應用 time increment for the player who just moved
                applyIncrement();

                updateBoard();

                // 檢查 pawn promotion is needed
                if (m_chessBoard.needsPromotion(logicalDropSquare)) {
                    const ChessPiece& piece = m_chessBoard.getPiece(logicalDropSquare.y(), logicalDropSquare.x());
                    PieceType promotionType = showPromotionDialog(piece.getColor());
                    m_chessBoard.promotePawn(logicalDropSquare, promotionType);
                    updateBoard();
                }

                // 更新棋譜列表
                updateMoveList();

                // 播放適當的音效
                playSoundForMove(isCapture, isCastling);

                // 更新 time displays (timer only runs if already started)
                updateTimeDisplays();

                updateStatus();
                clearHighlights();
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
}

void Qt_Chess::keyPressEvent(QKeyEvent *event) {
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

void Qt_Chess::setupTimeControlUI(QVBoxLayout* timeControlPanelLayout) {
    // 時間控制群組框
    QGroupBox* timeControlGroup = new QGroupBox("時間控制", this);
    QVBoxLayout* timeControlLayout = new QVBoxLayout(timeControlGroup);

    QFont labelFont;
    labelFont.setPointSize(10);

    // 白方時間標籤和滑桿
    m_whiteTimeLimitTitleLabel = new QLabel("白方時間:", this);
    m_whiteTimeLimitTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_whiteTimeLimitTitleLabel);

    m_whiteTimeLimitLabel = new QLabel("不限時", this);
    m_whiteTimeLimitLabel->setFont(labelFont);
    m_whiteTimeLimitLabel->setAlignment(Qt::AlignCenter);
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

    // 黑方時間標籤和滑桿
    m_blackTimeLimitTitleLabel = new QLabel("黑方時間:", this);
    m_blackTimeLimitTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_blackTimeLimitTitleLabel);

    m_blackTimeLimitLabel = new QLabel("不限時", this);
    m_blackTimeLimitLabel->setFont(labelFont);
    m_blackTimeLimitLabel->setAlignment(Qt::AlignCenter);
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

    // 增量標籤和滑桿
    m_incrementTitleLabel = new QLabel("每著加秒:", this);
    m_incrementTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_incrementTitleLabel);

    m_incrementLabel = new QLabel("0秒", this);
    m_incrementLabel->setFont(labelFont);
    m_incrementLabel->setAlignment(Qt::AlignCenter);
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

    // 添加伸展以填充群組框中的剩餘空間
    timeControlLayout->addStretch();

    timeControlPanelLayout->addWidget(timeControlGroup, 1);

    // 開始 button - placed at the bottom of the time control panel, outside the group box
    m_startButton = new QPushButton("開始", this);
    m_startButton->setMinimumHeight(40);
    QFont startButtonFont;
    startButtonFont.setPointSize(12);
    startButtonFont.setBold(true);
    m_startButton->setFont(startButtonFont);
    m_startButton->setEnabled(true);  // 始終啟用以允許開始遊戲
    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
    timeControlPanelLayout->addWidget(m_startButton, 0);  // 伸展因子 0 以保持按鈕高度

    // 放棄按鈕 - 放在開始按鈕下方，初始隱藏
    m_giveUpButton = new QPushButton("放棄", this);
    m_giveUpButton->setMinimumHeight(40);
    QFont giveUpButtonFont;
    giveUpButtonFont.setPointSize(12);
    giveUpButtonFont.setBold(true);
    m_giveUpButton->setFont(giveUpButtonFont);
    m_giveUpButton->hide();  // 初始隱藏
    connect(m_giveUpButton, &QPushButton::clicked, this, &Qt_Chess::onGiveUpClicked);
    timeControlPanelLayout->addWidget(m_giveUpButton, 0);  // 伸展因子 0 以保持按鈕高度

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

    // 開始 button is always enabled
    if (m_startButton) {
        m_startButton->setEnabled(true);
        m_startButton->setText("開始");
    }

    updateTimeDisplays();
    saveTimeControlSettings();
}

void Qt_Chess::onBlackTimeLimitChanged(int value) {
    if (!m_blackTimeLimitSlider || !m_blackTimeLimitLabel) return;

    m_blackTimeMs = calculateTimeFromSliderValue(value);
    m_blackTimeLimitLabel->setText(getTimeTextFromSliderValue(value));

    // 更新 time control enabled state
    m_timeControlEnabled = (m_whiteTimeMs > 0 || m_blackTimeMs > 0);
    m_timerStarted = false;

    // 開始 button is always enabled
    if (m_startButton) {
        m_startButton->setEnabled(true);
        m_startButton->setText("開始");
    }

    updateTimeDisplays();
    saveTimeControlSettings();
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

        // 如果少於 LOW_TIME_THRESHOLD_MS（10 秒），顯示格式為 0:秒.小數
        if (ms < LOW_TIME_THRESHOLD_MS) {
            double seconds = ms / 1000.0;
            return QString("0:%1").arg(seconds, 0, 'f', 1);  // 格式：0:9.8
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
}

void Qt_Chess::onGameTimerTick() {
    if (!m_timeControlEnabled) return;

    // 減少當前玩家的時間
    // 在回放模式中，使用進入回放時儲存的玩家，而不是棋盤上的當前玩家
    // 這樣可以確保在回放過程中時間只從同一個玩家倒數，不會隨著棋步切換
    PieceColor currentPlayer = m_isReplayMode ? m_savedCurrentPlayer : m_chessBoard.getCurrentPlayer();
    if (currentPlayer == PieceColor::White) {
        // 僅當白方有時間限制（非無限制）時才減少時間
        if (m_whiteTimeMs > 0) {
            m_whiteTimeMs -= 100; // 減少 100ms（計時器每 100ms 觸發一次）
            if (m_whiteTimeMs <= 0) {
                m_whiteTimeMs = 0;
                updateTimeDisplays();
                stopTimer();
                m_timerStarted = false;  // 重置計時器狀態
                showTimeControlAfterTimeout();
                QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
                return;
            }
        }
    } else {
        // 僅當黑方有時間限制（非無限制）時才減少時間
        if (m_blackTimeMs > 0) {
            m_blackTimeMs -= 100;
            if (m_blackTimeMs <= 0) {
                m_blackTimeMs = 0;
                updateTimeDisplays();
                stopTimer();
                m_timerStarted = false;  // 重置計時器狀態
                showTimeControlAfterTimeout();
                QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
                return;
            }
        }
    }

    updateTimeDisplays();
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
}

void Qt_Chess::handleGameEnd() {
    // 停止 timer when game ends
    stopTimer();
    m_timerStarted = false;
    m_gameStarted = false;  // 標記遊戲已結束

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

    // 保持時間顯示可見，以便玩家可以看到遊戲結束時的最終時間
    // 時間標籤在遊戲結束時不再隱藏

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

    // 保持時間顯示可見，以便玩家可以看到超時時的最終時間
    // 時間標籤在超時時不再隱藏

    // 當遊戲超時結束時，將右側伸展設為 0
    setRightPanelStretch(0);
}

void Qt_Chess::resetBoardState() {
    // 重置棋盤到初始狀態
    m_chessBoard.initializeBoard();
    m_pieceSelected = false;
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

    // 被吃掉棋子的大小和間距設定
    // 相同類型棋子水平重疊顯示，不同類型棋子垂直排列
    const int pieceSize = 24;  // 每個棋子標籤的大小
    const int horizontalOffset = pieceSize / 4;  // 相同類型棋子的水平重疊偏移量
    const int verticalOffset = pieceSize;  // 不同類型棋子之間的垂直間距

    // 按棋子類型分組並顯示的輔助函數
    // 相同類型棋子水平重疊，不同類型棋子垂直排列
    // 返回最終的 y 位置以便放置分差標籤
    auto displayCapturedPieces = [pieceSize, horizontalOffset, verticalOffset](
        QWidget* panel, const std::vector<ChessPiece>& capturedPieces, QList<QLabel*>& labels) -> int {
        if (!panel) return 0;
        if (capturedPieces.empty()) return 0;

        // 複製並按棋子類型排序，確保相同類型的棋子放在一起
        std::vector<ChessPiece> sortedPieces = capturedPieces;
        std::sort(sortedPieces.begin(), sortedPieces.end(), [](const ChessPiece& a, const ChessPiece& b) {
            return static_cast<int>(a.getType()) < static_cast<int>(b.getType());
        });

        int yPos = 0;  // 起始 y 位置，與棋盤頂部貼齊
        int panelWidth = panel->width();
        // 如果面板寬度尚未計算（初始設置期間），使用最小寬度
        if (panelWidth <= 0) {
            panelWidth = panel->minimumWidth();
            if (panelWidth <= 0) panelWidth = 30;  // 後備最小寬度
        }
        int baseXPos = 5;  // 起始 x 位置（左對齊，留最小邊距）
        int xPos = baseXPos;
        int panelHeight = panel->height();
        // 如果面板高度尚未計算，使用最小高度
        if (panelHeight <= 0) {
            panelHeight = panel->minimumHeight();
            if (panelHeight <= 0) panelHeight = 100;  // 後備最小高度
        }
        PieceType lastType = PieceType::None;

        for (size_t i = 0; i < sortedPieces.size(); ++i) {
            const ChessPiece& piece = sortedPieces[i];
            QLabel* label = new QLabel(panel);
            label->setText(piece.getSymbol());
            QFont pieceFont;
            pieceFont.setPointSize(16);
            label->setFont(pieceFont);
            label->setFixedSize(pieceSize, pieceSize);
            label->setAlignment(Qt::AlignCenter);

            // 如果不是第一個棋子，根據類型決定位置
            if (lastType != PieceType::None) {
                if (piece.getType() == lastType) {
                    // 相同類型的棋子水平重疊
                    int newXPos = xPos + horizontalOffset;
                    // 檢查是否超出面板寬度，如果超出則換行
                    if (newXPos + pieceSize > panelWidth) {
                        yPos += verticalOffset;
                        xPos = baseXPos;
                    } else {
                        xPos = newXPos;
                    }
                } else {
                    // 不同類型的棋子垂直排列（換行）
                    yPos += verticalOffset;
                    xPos = baseXPos;  // 重置 x 位置
                }
            }

            // 放置棋子標籤
            label->move(xPos, yPos);
            lastType = piece.getType();

            label->show();
            labels.append(label);
        }
        
        // 返回最終的 y 位置（加上最後一行的高度）
        return yPos + pieceSize;
    };

    // 更新分差標籤的輔助函數
    auto updateScoreDiffLabel = [](QLabel*& label, QWidget* panel, int scoreDiff, int yPosition) {
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
            label->move(5, yPosition + 5);  // 在棋子下方顯示
            label->adjustSize();
            label->show();
        } else {
            label->hide();
        }
    };

    // 顯示被吃掉的白色棋子（對方吃子紀錄，從上往下，與棋盤頂部貼齊）
    // 這裡顯示的是黑方吃掉的白子，所以顯示黑方的分差
    int whitePanelEndY = 0;
    if (m_capturedWhitePanel) {
        whitePanelEndY = displayCapturedPieces(m_capturedWhitePanel, capturedWhite, m_capturedWhiteLabels);
        updateScoreDiffLabel(m_blackScoreDiffLabel, m_capturedWhitePanel, blackDiff, whitePanelEndY);
    }

    // 顯示被吃掉的黑色棋子（我方吃子紀錄，從上往下）
    // 這裡顯示的是白方吃掉的黑子，所以顯示白方的分差
    int blackPanelEndY = 0;
    if (m_capturedBlackPanel) {
        blackPanelEndY = displayCapturedPieces(m_capturedBlackPanel, capturedBlack, m_capturedBlackLabels);
        updateScoreDiffLabel(m_whiteScoreDiffLabel, m_capturedBlackPanel, whiteDiff, blackPanelEndY);
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
