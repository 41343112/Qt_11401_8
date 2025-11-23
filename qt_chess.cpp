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

namespace {
    const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
    const int DEFAULT_ICON_SIZE = 40; // 預設圖示大小（像素）
    const int MAX_TIME_LIMIT_SECONDS = 1800; // 最大時間限制：30 分鐘
    const int MAX_SLIDER_POSITION = 31; // 滑桿範圍：0（無限制）、1（30秒）、2-31（1-30 分鐘）
    const int MAX_MINUTES = 30; // 最大時間限制（分鐘）
    const QString GAME_ENDED_TEXT = "遊戲結束"; // 遊戲結束時顯示的文字
    
    // 視窗大小的佈局常數
    const int LEFT_PANEL_MAX_WIDTH = 200;  // 新遊戲按鈕面板的最大寬度
    const int RIGHT_PANEL_MAX_WIDTH = 600; // 時間控制面板的最大寬度
    const int PANEL_SPACING = 0;          // 面板之間的間距
    const int BASE_MARGINS = 0;           // 基本佈局邊距（不包括棋盤容器的 2*BOARD_CONTAINER_MARGIN）
    const int TIME_LABEL_SPACING = 10;     // 時間標籤周圍的間距
    const int BOARD_CONTAINER_MARGIN = 0;  // 棋盤容器每側的邊距（總水平：2*5=10px）
    
    // UI 元素的縮放常數
    const int MIN_SQUARE_SIZE = 40;        // 棋盤格子的最小大小
    const int MAX_SQUARE_SIZE = 180;       // 棋盤格子的最大大小
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
    const int MAX_TIME_CONTROL_FONT = 14;        // 時間控制標籤的最大字體大小
    const int MIN_BUTTON_FONT = 10;              // 按鈕的最小字體大小
    const int MAX_BUTTON_FONT = 14;              // 按鈕的最大字體大小
    const int MIN_SLIDER_HEIGHT = 20;            // 滑桿的最小高度
    const int MAX_SLIDER_HEIGHT = 40;            // 滑桿的最大高度
    const int SLIDER_HANDLE_EXTRA = 10;          // 滑桿手柄的額外空間
    const int LOW_TIME_THRESHOLD_MS = 10000;     // 低時間警告的閾值（10 秒）
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
    , m_startButton(nullptr)
    , m_gameTimer(nullptr)
    , m_whiteTimeMs(0)
    , m_blackTimeMs(0)
    , m_incrementMs(0)
    , m_timeControlEnabled(false)
    , m_timerStarted(false)
    , m_boardContainer(nullptr)
    , m_timeControlPanel(nullptr)
    , m_gameStarted(false)
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(900, 660);  // 增加寬度以容納時間控制面板
    
    // 設置最小視窗大小以確保所有內容都能完整顯示而不被裁切
    // 計算：LEFT_PANEL_MAX_WIDTH (200) + 最小棋盤 (8*MIN_SQUARE_SIZE+4=244) + 
    //       RIGHT_PANEL_MAX_WIDTH (300) + 2*PANEL_SPACING (40) + BASE_MARGINS (30) + 
    //       棋盤容器邊距 (2*BOARD_CONTAINER_MARGIN=10) = 824
    // 高度：棋盤 (244) + 時間標籤 (~80) + 間距 (~60) = ~384，使用 420 以舒適調整大小
    setMinimumSize(900, 660);
    
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
}

Qt_Chess::~Qt_Chess()
{
    delete ui;
}

void Qt_Chess::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 為棋盤和時間控制創建水平佈局
    QHBoxLayout* contentLayout = new QHBoxLayout();
    
    // 移除左側面板 - 沒有新遊戲按鈕後不再需要
    
    // 棋盤容器，左右兩側顯示時間
    m_boardContainer = new QWidget(this);
    m_boardContainer->setMouseTracking(true);
    QHBoxLayout* boardContainerLayout = new QHBoxLayout(m_boardContainer);
    boardContainerLayout->setContentsMargins(BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN, 
                                             BOARD_CONTAINER_MARGIN, BOARD_CONTAINER_MARGIN);
    boardContainerLayout->setSpacing(TIME_LABEL_SPACING);  // 元素之間的一致間距
    
    // 時間顯示字體
    QFont timeFont;
    timeFont.setPointSize(14);
    timeFont.setBold(true);
    
    // 黑方時間標籤（左側 - 對手的時間）- 初始隱藏
    m_blackTimeLabel = new QLabel("--:--", m_boardContainer);
    m_blackTimeLabel->setFont(timeFont);
    m_blackTimeLabel->setAlignment(Qt::AlignCenter);
    m_blackTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_blackTimeLabel->setMinimumSize(100, 40);
    m_blackTimeLabel->hide();  // 初始隱藏
    boardContainerLayout->addWidget(m_blackTimeLabel, 0, Qt::AlignTop);
    
    // 國際象棋棋盤
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
    
    // 將棋盤添加到容器佈局
    // 伸展因子 1 允許棋盤擴展並填充可用的水平空間
    // 而時間標籤（伸展因子 0）保持其最小大小
    boardContainerLayout->addWidget(m_boardWidget, 1, Qt::AlignCenter);
    
    // 白方時間標籤（右側 - 玩家的時間）- 初始隱藏
    m_whiteTimeLabel = new QLabel("--:--", m_boardContainer);
    m_whiteTimeLabel->setFont(timeFont);
    m_whiteTimeLabel->setAlignment(Qt::AlignCenter);
    m_whiteTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_whiteTimeLabel->setMinimumSize(100, 40);
    m_whiteTimeLabel->hide();  // 初始隱藏
    boardContainerLayout->addWidget(m_whiteTimeLabel, 0, Qt::AlignBottom);
    
    contentLayout->addWidget(m_boardContainer, 2);  // 給棋盤更多空間（2:1 比例）
    contentLayout->setAlignment(m_boardContainer, Qt::AlignCenter);  // 將棋盤容器置中
    
    // 時間控制的右側面板
    m_timeControlPanel = new QWidget(this);
    m_timeControlPanel->setMaximumWidth(RIGHT_PANEL_MAX_WIDTH);  // 限制面板寬度
    QVBoxLayout* rightPanelLayout = new QVBoxLayout(m_timeControlPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    setupTimeControlUI(rightPanelLayout);
    contentLayout->addWidget(m_timeControlPanel, 1);  // 控制面板佔較少空間
    
    mainLayout->addLayout(contentLayout);
    
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
}

void Qt_Chess::updateStatus() {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";
    
    if (m_chessBoard.isCheckmate(currentPlayer)) {
        handleGameEnd();
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        QMessageBox::information(this, "遊戲結束", QString("將死！%1獲勝！").arg(winner));
    } else if (m_chessBoard.isStalemate(currentPlayer)) {
        handleGameEnd();
        QMessageBox::information(this, "遊戲結束", "逼和！對局和棋。");
    } else if (m_chessBoard.isInsufficientMaterial()) {
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
    
    // 隱藏時間顯示
    if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
    if (m_blackTimeLabel) m_blackTimeLabel->hide();
    
    // 隱藏放棄按鈕
    if (m_giveUpButton) m_giveUpButton->hide();
    
    // 根據滑桿值重置時間
    if (m_whiteTimeLimitSlider) {
        int value = m_whiteTimeLimitSlider->value();
        if (value == 0) {
            m_whiteTimeMs = 0;
        } else if (value == 1) {
            m_whiteTimeMs = 30 * 1000;
        } else {
            m_whiteTimeMs = (value - 1) * 60 * 1000;
        }
    }
    
    if (m_blackTimeLimitSlider) {
        int value = m_blackTimeLimitSlider->value();
        if (value == 0) {
            m_blackTimeMs = 0;
        } else if (value == 1) {
            m_blackTimeMs = 30 * 1000;
        } else {
            m_blackTimeMs = (value - 1) * 60 * 1000;
        }
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
        // 停止遊戲
        m_gameStarted = false;
        stopTimer();
        m_timerStarted = false;
        
        // 顯示時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->show();
        }
        
        // 隱藏時間顯示
        if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
        if (m_blackTimeLabel) m_blackTimeLabel->hide();
        
        // 隱藏放棄按鈕
        if (m_giveUpButton) m_giveUpButton->hide();
        
        // 重新啟用開始按鈕
        if (m_startButton) {
            m_startButton->setText("開始");
            m_startButton->setEnabled(true);
        }
        
        // 顯示放棄者的訊息
        PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
        QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        QMessageBox::information(this, "遊戲結束", QString("%1放棄！%2獲勝！").arg(playerName).arg(winner));
    }
}

void Qt_Chess::onStartButtonClicked() {
    if (m_timeControlEnabled && !m_timerStarted) {
        m_timerStarted = true;
        m_gameStarted = true;  // 標記遊戲已開始
        startTimer();
        
        // 隱藏時間控制面板
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }
        
        // 在棋盤左右兩側顯示時間
        if (m_whiteTimeLabel && m_blackTimeLabel) {
            m_whiteTimeLabel->show();
            m_blackTimeLabel->show();
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
    } else if (!m_timeControlEnabled && !m_gameStarted) {
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
    if (m_isDragging && m_dragLabel) {
        m_dragLabel->move(event->pos() - QPoint(m_dragLabel->width() / 2, m_dragLabel->height() / 2));
    }
    
    QMainWindow::mouseMoveEvent(event);
}

void Qt_Chess::mouseReleaseEvent(QMouseEvent *event) {
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
                    // Restore the piece to the original square first
                    restorePieceToSquare(m_dragStartSquare);
                    m_selectedSquare = logicalDropSquare;
                    m_pieceSelected = true;
                    highlightValidMoves();
                } else {
                    // Invalid move and not selecting another piece
                    // Restore the piece to the original square
                    restorePieceToSquare(m_dragStartSquare);
                    m_pieceSelected = false;
                    clearHighlights();
                }
            }
        } else {
            // Dropped outside board - cancel
            // Restore the piece to the original square
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
    
    // Reapply highlights after resize
    if (m_pieceSelected) {
        highlightValidMoves();
    }
}

void Qt_Chess::updateSquareSizes() {
    if (!m_boardWidget || m_squares.empty()) return;
    
    // 獲取 the central widget
    QWidget* central = centralWidget();
    if (!central) return;
    
    // 計算 available space for the board
    // Account for the horizontal layout with right panel and spacing between panels
    int reservedWidth = 0;
    int reservedHeight = 0;
    
    // Account for right panel width if visible (time control panel)
    if (m_timeControlPanel && m_timeControlPanel->isVisible()) {
        reservedWidth += RIGHT_PANEL_MAX_WIDTH;  // Reserve full width for time control panel
        reservedWidth += PANEL_SPACING;          // Spacing before right panel
    }
    
    // Add base margins for layout spacing (board container margins are part of board widget size)
    reservedWidth += BASE_MARGINS;
    
    // Account for time labels width if visible, plus spacing (now positioned horizontally)
    if (m_whiteTimeLabel && m_whiteTimeLabel->isVisible()) {
        reservedWidth += m_whiteTimeLabel->minimumWidth() + TIME_LABEL_SPACING;
    }
    if (m_blackTimeLabel && m_blackTimeLabel->isVisible()) {
        reservedWidth += m_blackTimeLabel->minimumWidth() + TIME_LABEL_SPACING;
    }
    
    // Add some padding for layout margins and spacing
    reservedHeight += BASE_MARGINS;
    
    int availableWidth = central->width() - reservedWidth;
    int availableHeight = central->height() - reservedHeight;
    
    // 計算 the size for each square (use the smaller dimension to keep squares square)
    int squareSize = qMin(availableWidth, availableHeight) / 8;
    
    // Ensure minimum and reasonable maximum size
    squareSize = qMax(squareSize, MIN_SQUARE_SIZE);  // Use constant for minimum size
    squareSize = qMin(squareSize, MAX_SQUARE_SIZE);  // Cap at a reasonable maximum
    
    // 計算 font size based on square size (approximately 45% of square size)
    int fontSize = squareSize * 9 / 20;  // This gives roughly 36pt for 80px squares
    fontSize = qMax(fontSize, 12);  // Ensure minimum readable font size
    fontSize = qMin(fontSize, 54);  // Cap font size for very large boards
    
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
                // Ensure scale is within valid range (60-100)
                int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
                int iconSize = static_cast<int>(squareSize * scale / 100.0);
                square->setIconSize(QSize(iconSize, iconSize));
            }
        }
    }
    
    // 更新 the board widget size to fit the squares exactly
    // Add 4 extra pixels (2px on each side) to prevent border clipping when squares are highlighted
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
        
        // 設置 minimum width for horizontal positioning (ensure time text fits)
        int timeLabelWidth = qMax(MIN_TIME_LABEL_WIDTH, squareSize);  // At least MIN_TIME_LABEL_WIDTH or square size
        m_whiteTimeLabel->setMinimumWidth(timeLabelWidth);
        m_blackTimeLabel->setMinimumWidth(timeLabelWidth);
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
    
    // Helper lambda to set slider height
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
    // Helper function to set sound source with proper URL handling
    // - For Qt resource paths (qrc:), use QUrl constructor directly
    // - For local file paths, use QUrl::fromLocalFile for proper conversion
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
    
    // Check for regular capture
    if (destinationPiece.getType() != PieceType::None && 
        destinationPiece.getColor() != movingPiece.getColor()) {
        return true;
    }
    
    // Check for en passant capture
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
    
    // Verify the move is on the correct starting rank (row 0 for black, row 7 for white)
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
    
    // Note: After movePiece(), the turn has switched, so currentPlayer is now the opponent
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
    // Validate and save piece scale (ensure it's within valid range 60-100)
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
    
    // Display piece with icon or symbol
    if (m_pieceIconSettings.useCustomIcons) {
        QPixmap pixmap = getCachedPieceIcon(piece.getType(), piece.getColor());
        if (!pixmap.isNull()) {
            QIcon icon(pixmap);
            square->setIcon(icon);
            // 設置 icon size based on square size
            int iconSize = calculateIconSize(square);
            square->setIconSize(QSize(iconSize, iconSize));
        } else {
            // Fallback to symbol if icon can't be loaded or not in cache
            square->setText(piece.getSymbol());
        }
    } else {
        // Use Unicode symbols
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
    // Ensure scale is within valid range (60-100)
    int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
    return static_cast<int>(squareWidth * scale / 100.0);
}

void Qt_Chess::loadBoardColorSettings() {
    QSettings settings("Qt_Chess", "BoardColorSettings");
    
    // 載入 color scheme type with validation
    int schemeInt = settings.value("colorScheme", static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic)).toInt();
    
    // Validate scheme is within valid range
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
    
    // Validate colors
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
    
    // Re-apply highlights if needed
    if (m_pieceSelected) {
        highlightValidMoves();
    }
    
    // Re-apply check highlight if in check
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
    // Time control group box
    QGroupBox* timeControlGroup = new QGroupBox("時間控制", this);
    QVBoxLayout* timeControlLayout = new QVBoxLayout(timeControlGroup);
    
    QFont labelFont;
    labelFont.setPointSize(10);
    
    // White time label and slider
    m_whiteTimeLimitTitleLabel = new QLabel("白方時間:", this);
    m_whiteTimeLimitTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_whiteTimeLimitTitleLabel);
    
    m_whiteTimeLimitLabel = new QLabel("不限時", this);
    m_whiteTimeLimitLabel->setFont(labelFont);
    m_whiteTimeLimitLabel->setAlignment(Qt::AlignCenter);
    timeControlLayout->addWidget(m_whiteTimeLimitLabel);
    
    // Horizontal slider for white time - discrete values
    // Slider positions: 0=Unlimited, 1=30sec, 2-31=1-30min
    m_whiteTimeLimitSlider = new QSlider(Qt::Horizontal, this);
    m_whiteTimeLimitSlider->setMinimum(0);  // 0 = unlimited
    m_whiteTimeLimitSlider->setMaximum(MAX_SLIDER_POSITION);  // 0 (unlimited), 1 (30s), 2-31 (1-30 min)
    m_whiteTimeLimitSlider->setValue(0);
    m_whiteTimeLimitSlider->setTickPosition(QSlider::TicksBelow);
    m_whiteTimeLimitSlider->setTickInterval(1);
    connect(m_whiteTimeLimitSlider, &QSlider::valueChanged, this, &Qt_Chess::onWhiteTimeLimitChanged);
    timeControlLayout->addWidget(m_whiteTimeLimitSlider);
    
    // Black time label and slider
    m_blackTimeLimitTitleLabel = new QLabel("黑方時間:", this);
    m_blackTimeLimitTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_blackTimeLimitTitleLabel);
    
    m_blackTimeLimitLabel = new QLabel("不限時", this);
    m_blackTimeLimitLabel->setFont(labelFont);
    m_blackTimeLimitLabel->setAlignment(Qt::AlignCenter);
    timeControlLayout->addWidget(m_blackTimeLimitLabel);
    
    // Horizontal slider for black time - discrete values
    m_blackTimeLimitSlider = new QSlider(Qt::Horizontal, this);
    m_blackTimeLimitSlider->setMinimum(0);  // 0 = unlimited
    m_blackTimeLimitSlider->setMaximum(MAX_SLIDER_POSITION);  // 0 (unlimited), 1 (30s), 2-31 (1-30 min)
    m_blackTimeLimitSlider->setValue(0);
    m_blackTimeLimitSlider->setTickPosition(QSlider::TicksBelow);
    m_blackTimeLimitSlider->setTickInterval(1);
    connect(m_blackTimeLimitSlider, &QSlider::valueChanged, this, &Qt_Chess::onBlackTimeLimitChanged);
    timeControlLayout->addWidget(m_blackTimeLimitSlider);
    
    // Increment label and slider
    m_incrementTitleLabel = new QLabel("每著加秒:", this);
    m_incrementTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(m_incrementTitleLabel);
    
    m_incrementLabel = new QLabel("0秒", this);
    m_incrementLabel->setFont(labelFont);
    m_incrementLabel->setAlignment(Qt::AlignCenter);
    timeControlLayout->addWidget(m_incrementLabel);
    
    // Horizontal slider for increment - fills available width
    m_incrementSlider = new QSlider(Qt::Horizontal, this);
    m_incrementSlider->setMinimum(0);
    m_incrementSlider->setMaximum(60);
    m_incrementSlider->setValue(0);
    m_incrementSlider->setTickPosition(QSlider::TicksBelow);
    m_incrementSlider->setTickInterval(5);
    connect(m_incrementSlider, &QSlider::valueChanged, this, &Qt_Chess::onIncrementChanged);
    timeControlLayout->addWidget(m_incrementSlider);
    
    // Add stretch to fill remaining space in the group box
    timeControlLayout->addStretch();
    
    timeControlPanelLayout->addWidget(timeControlGroup, 1);
    
    // 開始 button - placed at the bottom of the time control panel, outside the group box
    m_startButton = new QPushButton("開始", this);
    m_startButton->setMinimumHeight(40);
    QFont startButtonFont;
    startButtonFont.setPointSize(12);
    startButtonFont.setBold(true);
    m_startButton->setFont(startButtonFont);
    m_startButton->setEnabled(true);  // Always enabled to allow starting the game
    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
    timeControlPanelLayout->addWidget(m_startButton, 0);  // Stretch factor 0 to maintain button height
    
    // Give Up button - placed below Start button, initially hidden
    m_giveUpButton = new QPushButton("放棄", this);
    m_giveUpButton->setMinimumHeight(40);
    QFont giveUpButtonFont;
    giveUpButtonFont.setPointSize(12);
    giveUpButtonFont.setBold(true);
    m_giveUpButton->setFont(giveUpButtonFont);
    m_giveUpButton->hide();  // Initially hidden
    connect(m_giveUpButton, &QPushButton::clicked, this, &Qt_Chess::onGiveUpClicked);
    timeControlPanelLayout->addWidget(m_giveUpButton, 0);  // Stretch factor 0 to maintain button height
    
    // 初始化 game timer
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &Qt_Chess::onGameTimerTick);
}

void Qt_Chess::onWhiteTimeLimitChanged(int value) {
    if (!m_whiteTimeLimitSlider || !m_whiteTimeLimitLabel) return;
    
    int actualSeconds = 0;
    QString timeText;
    
    if (value == 0) {
        // Unlimited time
        timeText = "不限時";
        actualSeconds = 0;
        m_whiteTimeMs = 0;
    } else if (value == 1) {
        // 30 seconds
        timeText = "30秒";
        actualSeconds = 30;
        m_whiteTimeMs = actualSeconds * 1000;
    } else {
        // value 2-31 represents 1-30 minutes
        int minutes = value - 1;
        timeText = QString("%1分鐘").arg(minutes);
        actualSeconds = minutes * 60;
        m_whiteTimeMs = actualSeconds * 1000;
    }
    
    m_whiteTimeLimitLabel->setText(timeText);
    
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
    
    int actualSeconds = 0;
    QString timeText;
    
    if (value == 0) {
        // Unlimited time
        timeText = "不限時";
        actualSeconds = 0;
        m_blackTimeMs = 0;
    } else if (value == 1) {
        // 30 seconds
        timeText = "30秒";
        actualSeconds = 30;
        m_blackTimeMs = actualSeconds * 1000;
    } else {
        // value 2-31 represents 1-30 minutes
        int minutes = value - 1;
        timeText = QString("%1分鐘").arg(minutes);
        actualSeconds = minutes * 60;
        m_blackTimeMs = actualSeconds * 1000;
    }
    
    m_blackTimeLimitLabel->setText(timeText);
    
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
        return;
    }
    
    // 轉換 milliseconds to minutes:seconds or show unlimited
    // When time < 10 seconds, show decimal points (e.g., "9.8")
    auto formatTime = [](int ms) -> QString {
        if (ms < 0) {
            return "不限時";
        }
        
        // If less than LOW_TIME_THRESHOLD_MS (10 seconds), show decimal format
        if (ms < LOW_TIME_THRESHOLD_MS) {
            double seconds = ms / 1000.0;
            return QString::number(seconds, 'f', 1);  // 顯示 1 decimal place
        }
        
        // Otherwise show minutes:seconds format
        int totalSeconds = ms / 1000;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    };
    
    m_whiteTimeLabel->setText(formatTime(m_whiteTimeMs));
    m_blackTimeLabel->setText(formatTime(m_blackTimeMs));
    
    // Determine background colors based on time remaining
    // When time < 10 seconds, use red background
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    
    QString whiteStyle, blackStyle;
    
    // Determine white label style
    if (m_whiteTimeMs > 0 && m_whiteTimeMs < LOW_TIME_THRESHOLD_MS) {  // Less than 10 seconds
        whiteStyle = "QLabel { background-color: rgba(220, 53, 69, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    } else if (currentPlayer == PieceColor::White) {
        whiteStyle = "QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    } else {
        whiteStyle = "QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    }
    
    // Determine black label style
    if (m_blackTimeMs > 0 && m_blackTimeMs < LOW_TIME_THRESHOLD_MS) {  // Less than 10 seconds
        blackStyle = "QLabel { background-color: rgba(220, 53, 69, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    } else if (currentPlayer == PieceColor::Black) {
        blackStyle = "QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }";
    } else {
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
    
    // Decrease time for current player
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (currentPlayer == PieceColor::White) {
        // Only decrease time if white has a time limit (not unlimited)
        if (m_whiteTimeMs > 0) {
            m_whiteTimeMs -= 100; // Decrease by 100ms (timer ticks every 100ms)
            if (m_whiteTimeMs <= 0) {
                m_whiteTimeMs = 0;
                updateTimeDisplays();
                stopTimer();
                m_timerStarted = false;  // Reset timer state
                showTimeControlAfterTimeout();
                QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
                return;
            }
        }
    } else {
        // Only decrease time if black has a time limit (not unlimited)
        if (m_blackTimeMs > 0) {
            m_blackTimeMs -= 100;
            if (m_blackTimeMs <= 0) {
                m_blackTimeMs = 0;
                updateTimeDisplays();
                stopTimer();
                m_timerStarted = false;  // Reset timer state
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
        m_gameTimer->start(100); // Tick every 100ms for smooth countdown
    }
}

void Qt_Chess::stopTimer() {
    if (m_gameTimer && m_gameTimer->isActive()) {
        m_gameTimer->stop();
    }
}

void Qt_Chess::applyIncrement() {
    if (!m_timeControlEnabled || m_incrementMs <= 0) return;
    
    // Add increment to the player who just moved
    // Note: getCurrentPlayer() returns the opponent after the move
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
    m_gameStarted = false;  // Mark game as ended
    
    // 隱藏放棄按鈕
    if (m_giveUpButton) {
        m_giveUpButton->hide();
    }
    
    // 顯示時間控制面板
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }
    
    // Re-enable start button to allow starting a new game
    if (m_startButton) {
        m_startButton->setText("開始");
        m_startButton->setEnabled(true);
    }
    
    // 隱藏時間顯示
    if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
    if (m_blackTimeLabel) m_blackTimeLabel->hide();
}

void Qt_Chess::loadTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    
    // 載入 white time
    int whiteTimeLimitSeconds = settings.value("whiteTimeLimitSeconds", 0).toInt();
    int whiteSliderPosition = 0;
    
    if (whiteTimeLimitSeconds <= 0) {
        whiteSliderPosition = 0;  // Unlimited
    } else if (whiteTimeLimitSeconds == 30) {
        whiteSliderPosition = 1;  // 30 seconds
    } else if (whiteTimeLimitSeconds < 60) {
        // Legacy values < 60 seconds (not 30) -> map to 30 seconds as closest valid option
        whiteSliderPosition = 1;
    } else {
        // 轉換 minutes to slider position (2-31 = 1-30 minutes)
        int minutes = whiteTimeLimitSeconds / 60;
        if (minutes > MAX_MINUTES) minutes = MAX_MINUTES;  // Cap at maximum minutes
        whiteSliderPosition = minutes + 1;
    }
    
    // 載入 black time
    int blackTimeLimitSeconds = settings.value("blackTimeLimitSeconds", 0).toInt();
    int blackSliderPosition = 0;
    
    if (blackTimeLimitSeconds <= 0) {
        blackSliderPosition = 0;  // Unlimited
    } else if (blackTimeLimitSeconds == 30) {
        blackSliderPosition = 1;  // 30 seconds
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
    
    // Time control enabled state will be set by onWhiteTimeLimitChanged and onBlackTimeLimitChanged
    // which are triggered by setValue above
}

void Qt_Chess::saveTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    
    // 儲存 white time
    if (m_whiteTimeLimitSlider) {
        int sliderValue = m_whiteTimeLimitSlider->value();
        int seconds = 0;
        
        if (sliderValue == 0) {
            seconds = 0;  // Unlimited
        } else if (sliderValue == 1) {
            seconds = 30;  // 30 seconds
        } else {
            // sliderValue 2-31 = 1-30 minutes
            seconds = (sliderValue - 1) * 60;
        }
        
        settings.setValue("whiteTimeLimitSeconds", seconds);
    }
    
    // 儲存 black time
    if (m_blackTimeLimitSlider) {
        int sliderValue = m_blackTimeLimitSlider->value();
        int seconds = 0;
        
        if (sliderValue == 0) {
            seconds = 0;  // Unlimited
        } else if (sliderValue == 1) {
            seconds = 30;  // 30 seconds
        } else {
            // sliderValue 2-31 = 1-30 minutes
            seconds = (sliderValue - 1) * 60;
        }
        
        settings.setValue("blackTimeLimitSeconds", seconds);
    }
    
    if (m_incrementSlider) {
        settings.setValue("incrementSeconds", m_incrementSlider->value());
    }
    
    settings.sync();
}

void Qt_Chess::showTimeControlAfterTimeout() {
    // Mark game as ended
    m_gameStarted = false;
    
    // 顯示時間控制面板 so user can adjust settings
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }
    
    // 隱藏放棄按鈕
    if (m_giveUpButton) {
        m_giveUpButton->hide();
    }
    
    // Re-enable start button
    if (m_startButton) {
        m_startButton->setText("開始");
        m_startButton->setEnabled(true);
    }
    
    // 隱藏時間顯示 since game is over
    if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
    if (m_blackTimeLabel) m_blackTimeLabel->hide();
}
