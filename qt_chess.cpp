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
    const int DEFAULT_ICON_SIZE = 40; // Default fallback icon size in pixels
    const int MAX_TIME_LIMIT_SECONDS = 3600; // Maximum time limit: 60 minutes
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
    , m_timeLimitSlider(nullptr)
    , m_timeLimitLabel(nullptr)
    , m_incrementSlider(nullptr)
    , m_incrementLabel(nullptr)
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
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(900, 660);  // Increased width to accommodate time control panel
    
    // Set minimum window size: increased width for right panel
    setMinimumSize(520, 380);
    
    setMouseTracking(true);
    
    loadSoundSettings();
    initializeSounds();
    loadPieceIconSettings();
    loadBoardColorSettings();
    loadBoardFlipSettings();
    loadPieceIconsToCache(); // Load icons to cache after loading settings
    setupMenuBar();
    setupUI();
    loadTimeControlSettings();  // Load after setupUI() to ensure widgets exist
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
    
    // Create horizontal layout for board and time controls
    QHBoxLayout* contentLayout = new QHBoxLayout();
    
    // Chess board container with overlaid time displays
    m_boardContainer = new QWidget(this);
    m_boardContainer->setMouseTracking(true);
    QVBoxLayout* boardContainerLayout = new QVBoxLayout(m_boardContainer);
    boardContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    // Chess board
    m_boardWidget = new QWidget(m_boardContainer);
    m_boardWidget->setMouseTracking(true);
    QGridLayout* gridLayout = new QGridLayout(m_boardWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(2, 2, 2, 2);  // Add 2px margin on all sides to prevent border clipping
    
    m_squares.resize(8, std::vector<QPushButton*>(8));
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = new QPushButton(m_boardWidget);
            square->setMinimumSize(40, 40);  // Set a reasonable minimum size
            square->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            square->setMouseTracking(true);
            
            QFont buttonFont;
            buttonFont.setPointSize(36);
            square->setFont(buttonFont);
            
            m_squares[row][col] = square;
            gridLayout->addWidget(square, row, col);
            
            // Store button coordinates for efficient lookup in eventFilter
            m_buttonCoordinates[square] = QPoint(col, row);
            
            // Install event filter to capture mouse events for drag-and-drop
            square->installEventFilter(this);
            
            connect(square, &QPushButton::clicked, this, [this, row, col]() {
                onSquareClicked(row, col);
            });
            
            updateSquareColor(row, col);
        }
    }
    
    boardContainerLayout->addWidget(m_boardWidget, 0, Qt::AlignCenter);
    contentLayout->addWidget(m_boardContainer, 1);
    
    // Right panel for time controls
    QVBoxLayout* rightPanelLayout = new QVBoxLayout();
    setupTimeControlUI(rightPanelLayout);
    contentLayout->addLayout(rightPanelLayout, 0);
    
    mainLayout->addLayout(contentLayout);
    
    // New game button
    m_newGameButton = new QPushButton("新遊戲", this);
    m_newGameButton->setMinimumHeight(40);
    QFont buttonFont;
    buttonFont.setPointSize(14);
    buttonFont.setBold(true);
    m_newGameButton->setFont(buttonFont);
    connect(m_newGameButton, &QPushButton::clicked, this, &Qt_Chess::onNewGameClicked);
    mainLayout->addWidget(m_newGameButton);
    
    setCentralWidget(centralWidget);
}

void Qt_Chess::setupMenuBar() {
    m_menuBar = menuBar();
    
    // Settings menu
    QMenu* settingsMenu = m_menuBar->addMenu("設定");
    
    // Sound settings action
    QAction* soundSettingsAction = new QAction("音效設定", this);
    connect(soundSettingsAction, &QAction::triggered, this, &Qt_Chess::onSoundSettingsClicked);
    settingsMenu->addAction(soundSettingsAction);
    
    // Piece icon settings action
    QAction* pieceIconSettingsAction = new QAction("棋子圖標設定", this);
    connect(pieceIconSettingsAction, &QAction::triggered, this, &Qt_Chess::onPieceIconSettingsClicked);
    settingsMenu->addAction(pieceIconSettingsAction);
    
    // Board color settings action
    QAction* boardColorSettingsAction = new QAction("棋盤顏色設定", this);
    connect(boardColorSettingsAction, &QAction::triggered, this, &Qt_Chess::onBoardColorSettingsClicked);
    settingsMenu->addAction(boardColorSettingsAction);
    
    settingsMenu->addSeparator();
    
    // Flip board action
    QAction* flipBoardAction = new QAction("反轉棋盤", this);
    connect(flipBoardAction, &QAction::triggered, this, &Qt_Chess::onFlipBoardClicked);
    settingsMenu->addAction(flipBoardAction);
}

void Qt_Chess::updateSquareColor(int displayRow, int displayCol) {
    // Calculate logical coordinates to determine the correct light/dark pattern
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
    
    // Highlight king in red if in check
    applyCheckHighlight();
    // Reapply highlights if a piece is selected
    if (m_pieceSelected) {
        highlightValidMoves();
    }
}

void Qt_Chess::updateStatus() {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";
    
    if (m_chessBoard.isCheckmate(currentPlayer)) {
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        QMessageBox::information(this, "遊戲結束", QString("將死！%1獲勝！").arg(winner));
    } else if (m_chessBoard.isStalemate(currentPlayer)) {
        QMessageBox::information(this, "遊戲結束", "逼和！對局和棋。");
    } else if (m_chessBoard.isInsufficientMaterial()) {
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
    
    // Re-apply red background to king if in check
    applyCheckHighlight();
}

void Qt_Chess::highlightValidMoves() {
    clearHighlights();
    
    if (!m_pieceSelected) return;
    
    // Highlight selected square (m_selectedSquare is in logical coordinates)
    int displayRow = getDisplayRow(m_selectedSquare.y());
    int displayCol = getDisplayCol(m_selectedSquare.x());
    m_squares[displayRow][displayCol]->setStyleSheet(
        "QPushButton { background-color: #7FC97F; border: 2px solid #00FF00; }"
    );
    
    // Highlight valid moves
    for (int logicalRow = 0; logicalRow < 8; ++logicalRow) {
        for (int logicalCol = 0; logicalCol < 8; ++logicalCol) {
            QPoint targetSquare(logicalCol, logicalRow);
            if (m_chessBoard.isValidMove(m_selectedSquare, targetSquare)) {
                bool isCapture = isCaptureMove(m_selectedSquare, targetSquare);
                int displayRow = getDisplayRow(logicalRow);
                int displayCol = getDisplayCol(logicalCol);
                // Use logical coordinates to determine light/dark square
                bool isLight = (logicalRow + logicalCol) % 2 == 0;
                
                if (isCapture) {
                    // Highlight capture moves in red
                    QString color = isLight ? "#FFB3B3" : "#FF8080";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FF0000; }").arg(color)
                    );
                } else {
                    // Highlight non-capture moves in orange
                    QString color = isLight ? "#FFE4B5" : "#DEB887";
                    m_squares[displayRow][displayCol]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FFA500; }").arg(color)
                    );
                }
            }
        }
    }
    
    // Re-apply red background to king if in check and king is not the selected piece
    applyCheckHighlight(m_selectedSquare);
}

void Qt_Chess::onSquareClicked(int displayRow, int displayCol) {
    // Convert display coordinates to logical coordinates
    int logicalRow = getLogicalRow(displayRow);
    int logicalCol = getLogicalCol(displayCol);
    QPoint clickedSquare(logicalCol, logicalRow);
    
    if (!m_pieceSelected) {
        // Try to select a piece
        const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
        if (piece.getType() != PieceType::None && 
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
            m_selectedSquare = clickedSquare;
            m_pieceSelected = true;
            highlightValidMoves();
        }
    } else {
        // Detect move type before executing the move
        bool isCapture = isCaptureMove(m_selectedSquare, clickedSquare);
        bool isCastling = isCastlingMove(m_selectedSquare, clickedSquare);
        
        // Try to move the selected piece
        if (m_chessBoard.movePiece(m_selectedSquare, clickedSquare)) {
            m_pieceSelected = false;
            
            // Apply time increment for the player who just moved
            applyIncrement();
            
            updateBoard();
            
            // Check if pawn promotion is needed
            if (m_chessBoard.needsPromotion(clickedSquare)) {
                const ChessPiece& piece = m_chessBoard.getPiece(clickedSquare.y(), clickedSquare.x());
                PieceType promotionType = showPromotionDialog(piece.getColor());
                m_chessBoard.promotePawn(clickedSquare, promotionType);
                updateBoard();
            }
            
            // Play appropriate sound effect
            playSoundForMove(isCapture, isCastling);
            
            // Update time displays (timer only runs if already started)
            updateTimeDisplays();
            
            updateStatus();
        } else if (clickedSquare == m_selectedSquare) {
            // Deselect the piece
            m_pieceSelected = false;
            clearHighlights();
        } else {
            // Try to select a different piece of the same color
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
    
    // Reset time control
    stopTimer();
    m_timerStarted = false;
    
    if (m_timeControlEnabled && m_timeLimitSlider) {
        int seconds = m_timeLimitSlider->value();
        if (seconds > 0) {
            m_whiteTimeMs = seconds * 1000;
            m_blackTimeMs = seconds * 1000;
        }
        // Enable start button when time control is active
        if (m_startButton) {
            m_startButton->setEnabled(true);
            m_startButton->setText("開始");
        }
    } else {
        // Disable start button when time control is not active
        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("開始");
        }
    }
    
    updateBoard();
    updateStatus();
    updateTimeDisplays();
}

void Qt_Chess::onStartButtonClicked() {
    if (m_timeControlEnabled && !m_timerStarted) {
        m_timerStarted = true;
        startTimer();
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
    
    // Create buttons for each promotion option
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
    
    PieceType selectedType = PieceType::Queen; // Default to Queen
    
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
    // Check if the event is from one of our chess square buttons
    QPushButton* button = qobject_cast<QPushButton*>(obj);
    if (!button) {
        return QMainWindow::eventFilter(obj, event);
    }
    
    // Check if this button is one of our chess squares using efficient map lookup
    if (!m_buttonCoordinates.contains(button)) {
        return QMainWindow::eventFilter(obj, event);
    }
    
    // Forward mouse events to enable drag-and-drop
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // Map the button's position to the main window's coordinate system
        QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
        QPoint windowPos = mapFromGlobal(globalPos);
        QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(), 
                               mouseEvent->buttons(), mouseEvent->modifiers());
        mousePressEvent(&mappedEvent);
        // Don't accept the event completely - let the button still handle clicks if no drag started
        if (m_isDragging) {
            return true; // Event handled, start dragging
        }
    } else if (event->type() == QEvent::MouseMove) {
        if (m_isDragging) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
            QPoint windowPos = mapFromGlobal(globalPos);
            QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(), 
                                   mouseEvent->buttons(), mouseEvent->modifiers());
            mouseMoveEvent(&mappedEvent);
            return true; // Event handled
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (m_isDragging) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
            QPoint windowPos = mapFromGlobal(globalPos);
            QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(), 
                                   mouseEvent->buttons(), mouseEvent->modifiers());
            mouseReleaseEvent(&mappedEvent);
            return true; // Event handled
        }
    }
    
    // Pass the event to the parent class for standard processing
    return QMainWindow::eventFilter(obj, event);
}

void Qt_Chess::mousePressEvent(QMouseEvent *event) {
    QPoint displaySquare = getSquareAtPosition(event->pos());
    
    // Right click - cancel any current action
    if (event->button() == Qt::RightButton) {
        if (m_isDragging) {
            // Cancel drag and return piece to original position
            m_isDragging = false;
            if (m_dragLabel) {
                m_dragLabel->hide();
                m_dragLabel->deleteLater();
                m_dragLabel = nullptr;
            }
            // Restore the piece to the original square
            restorePieceToSquare(m_dragStartSquare);
            m_dragStartSquare = QPoint(-1, -1);
            clearHighlights();
        } else if (m_pieceSelected) {
            // Deselect piece if one is selected
            m_pieceSelected = false;
            clearHighlights();
        }
        return;
    }
    
    // Left click - start drag
    if (event->button() == Qt::LeftButton && displaySquare.x() >= 0 && displaySquare.y() >= 0 && 
        displaySquare.x() < 8 && displaySquare.y() < 8) {
        // Convert display coordinates to logical coordinates
        int logicalRow = getLogicalRow(displaySquare.y());
        int logicalCol = getLogicalCol(displaySquare.x());
        QPoint logicalSquare(logicalCol, logicalRow);
        
        const ChessPiece& piece = m_chessBoard.getPiece(logicalRow, logicalCol);
        if (piece.getType() != PieceType::None && 
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
            
            // Track if this piece was already selected before the drag
            m_wasSelectedBeforeDrag = (m_pieceSelected && m_selectedSquare == logicalSquare);
            
            m_isDragging = true;
            m_dragStartSquare = logicalSquare;
            m_selectedSquare = logicalSquare;
            m_pieceSelected = true;
            
            // Create drag label
            m_dragLabel = new QLabel(this);
            
            // Use custom icon or Unicode symbol
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
            
            // Hide the piece from the original square during drag
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
    // Right click - already handled in mousePressEvent
    if (event->button() == Qt::RightButton) {
        QMainWindow::mouseReleaseEvent(event);
        return;
    }
    
    // Left click release - complete drag
    if (event->button() == Qt::LeftButton && m_isDragging) {
        QPoint displayDropSquare = getSquareAtPosition(event->pos());
        
        // Clean up drag label
        if (m_dragLabel) {
            m_dragLabel->hide();
            m_dragLabel->deleteLater();
            m_dragLabel = nullptr;
        }
        
        m_isDragging = false;
        
        if (displayDropSquare.x() >= 0 && displayDropSquare.y() >= 0) {
            // Convert display coordinates to logical coordinates
            int logicalRow = getLogicalRow(displayDropSquare.y());
            int logicalCol = getLogicalCol(displayDropSquare.x());
            QPoint logicalDropSquare(logicalCol, logicalRow);
            
            // Detect move type before executing the move
            bool isCapture = isCaptureMove(m_dragStartSquare, logicalDropSquare);
            bool isCastling = isCastlingMove(m_dragStartSquare, logicalDropSquare);
            
            // Try to move the piece
            if (m_chessBoard.movePiece(m_dragStartSquare, logicalDropSquare)) {
                m_pieceSelected = false;
                
                // Apply time increment for the player who just moved
                applyIncrement();
                
                updateBoard();
                
                // Check if pawn promotion is needed
                if (m_chessBoard.needsPromotion(logicalDropSquare)) {
                    const ChessPiece& piece = m_chessBoard.getPiece(logicalDropSquare.y(), logicalDropSquare.x());
                    PieceType promotionType = showPromotionDialog(piece.getColor());
                    m_chessBoard.promotePawn(logicalDropSquare, promotionType);
                    updateBoard();
                }
                
                // Play appropriate sound effect
                playSoundForMove(isCapture, isCastling);
                
                // Update time displays (timer only runs if already started)
                updateTimeDisplays();
                
                updateStatus();
                clearHighlights();
            } else if (logicalDropSquare == m_dragStartSquare) {
                // Dropped on same square - toggle selection
                // Restore the piece to the original square
                restorePieceToSquare(m_dragStartSquare);
                
                if (m_wasSelectedBeforeDrag) {
                    // Was already selected, so deselect
                    m_pieceSelected = false;
                    clearHighlights();
                } else {
                    // Was not selected, so keep it selected with highlights
                    m_selectedSquare = m_dragStartSquare;
                    m_pieceSelected = true;
                    highlightValidMoves();
                }
            } else {
                // Invalid move - try to select a different piece
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
    
    // Reapply highlights after resize
    if (m_pieceSelected) {
        highlightValidMoves();
    }
}

void Qt_Chess::updateSquareSizes() {
    if (!m_boardWidget || m_squares.empty()) return;
    
    // Get the central widget
    QWidget* central = centralWidget();
    if (!central) return;
    
    // Calculate available space for the board
    // Account for the new game button
    int reservedHeight = 0;
    if (m_newGameButton) reservedHeight += m_newGameButton->minimumHeight();
    
    // Add some padding for layout margins and spacing (estimate ~50px)
    reservedHeight += 50;
    
    int availableWidth = central->width();
    int availableHeight = central->height() - reservedHeight;
    
    // Calculate the size for each square (use the smaller dimension to keep squares square)
    int squareSize = qMin(availableWidth, availableHeight) / 8;
    
    // Ensure minimum and reasonable maximum size
    squareSize = qMax(squareSize, 35);
    squareSize = qMin(squareSize, 180);  // Cap at a reasonable maximum
    
    // Calculate font size based on square size (approximately 45% of square size)
    int fontSize = squareSize * 9 / 20;  // This gives roughly 36pt for 80px squares
    fontSize = qMax(fontSize, 12);  // Ensure minimum readable font size
    fontSize = qMin(fontSize, 54);  // Cap font size for very large boards
    
    // Update all squares
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = m_squares[row][col];
            square->setFixedSize(squareSize, squareSize);
            
            QFont font = square->font();
            font.setPointSize(fontSize);
            square->setFont(font);
            
            // Update icon size if using custom icons
            if (m_pieceIconSettings.useCustomIcons && !square->icon().isNull()) {
                // Ensure scale is within valid range (60-100)
                int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
                int iconSize = static_cast<int>(squareSize * scale / 100.0);
                square->setIconSize(QSize(iconSize, iconSize));
            }
        }
    }
    
    // Update the board widget size to fit the squares exactly
    // Add 4 extra pixels (2px on each side) to prevent border clipping when squares are highlighted
    m_boardWidget->setFixedSize(squareSize * 8 + 4, squareSize * 8 + 4);
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
    // Initialize sound effects with settings
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
    
    // Check if the moving piece is a king moving 2 squares horizontally
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
    // Check if all sounds are disabled
    if (!m_soundSettings.allSoundsEnabled) {
        return;
    }
    
    // Stop any currently playing sound before playing the new one
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
    // Load and validate piece scale (ensure it's within valid range 60-100)
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
    
    // Load icons to cache for improved performance
    loadPieceIconsToCache();
    
    // Update the board to reflect the new settings
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
    
    // Clear previous content
    square->setText("");
    square->setIcon(QIcon());
    
    // Display piece with icon or symbol
    if (m_pieceIconSettings.useCustomIcons) {
        QPixmap pixmap = getCachedPieceIcon(piece.getType(), piece.getColor());
        if (!pixmap.isNull()) {
            QIcon icon(pixmap);
            square->setIcon(icon);
            // Set icon size based on square size
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
    
    // Load all piece icons into cache
    auto loadIconToCache = [this](const QString& iconPath) {
        if (!iconPath.isEmpty() && !m_pieceIconCache.contains(iconPath) && QFile::exists(iconPath)) {
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                m_pieceIconCache.insert(iconPath, pixmap);
            }
        }
    };
    
    // Load white pieces
    loadIconToCache(m_pieceIconSettings.whiteKingIcon);
    loadIconToCache(m_pieceIconSettings.whiteQueenIcon);
    loadIconToCache(m_pieceIconSettings.whiteRookIcon);
    loadIconToCache(m_pieceIconSettings.whiteBishopIcon);
    loadIconToCache(m_pieceIconSettings.whiteKnightIcon);
    loadIconToCache(m_pieceIconSettings.whitePawnIcon);
    
    // Load black pieces
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
    // Apply the user-configured scale factor (default 80%)
    // Ensure scale is within valid range (60-100)
    int scale = qBound(60, m_pieceIconSettings.pieceScale, 100);
    return static_cast<int>(squareWidth * scale / 100.0);
}

void Qt_Chess::loadBoardColorSettings() {
    QSettings settings("Qt_Chess", "BoardColorSettings");
    
    // Load color scheme type with validation
    int schemeInt = settings.value("colorScheme", static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic)).toInt();
    
    // Validate scheme is within valid range
    if (schemeInt < static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic) ||
        schemeInt > static_cast<int>(BoardColorSettingsDialog::ColorScheme::Custom7)) {
        schemeInt = static_cast<int>(BoardColorSettingsDialog::ColorScheme::Classic);
    }
    
    BoardColorSettingsDialog::ColorScheme scheme = static_cast<BoardColorSettingsDialog::ColorScheme>(schemeInt);
    
    // Load colors
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
    // Save settings
    QSettings settings("Qt_Chess", "BoardColorSettings");
    settings.setValue("colorScheme", static_cast<int>(m_boardColorSettings.scheme));
    settings.setValue("lightSquareColor", m_boardColorSettings.lightSquareColor.name());
    settings.setValue("darkSquareColor", m_boardColorSettings.darkSquareColor.name());
    
    // Update all squares on the board
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

void Qt_Chess::setupTimeControlUI(QVBoxLayout* rightPanelLayout) {
    // Time control group box
    QGroupBox* timeControlGroup = new QGroupBox("時間控制", this);
    QVBoxLayout* timeControlLayout = new QVBoxLayout(timeControlGroup);
    
    // Total time label and slider
    QLabel* timeLimitTitleLabel = new QLabel("總時間:", this);
    QFont labelFont;
    labelFont.setPointSize(10);
    timeLimitTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(timeLimitTitleLabel);
    
    m_timeLimitLabel = new QLabel("不限時", this);
    m_timeLimitLabel->setFont(labelFont);
    m_timeLimitLabel->setAlignment(Qt::AlignCenter);
    timeControlLayout->addWidget(m_timeLimitLabel);
    
    m_timeLimitSlider = new QSlider(Qt::Horizontal, this);
    m_timeLimitSlider->setMinimum(0);  // 0 = unlimited
    m_timeLimitSlider->setMaximum(MAX_TIME_LIMIT_SECONDS);  // Max 60 minutes
    m_timeLimitSlider->setValue(0);
    m_timeLimitSlider->setTickPosition(QSlider::TicksBelow);
    m_timeLimitSlider->setTickInterval(300);  // Tick every 5 minutes
    connect(m_timeLimitSlider, &QSlider::valueChanged, this, &Qt_Chess::onTimeLimitChanged);
    timeControlLayout->addWidget(m_timeLimitSlider);
    
    // Increment label and slider
    QLabel* incrementTitleLabel = new QLabel("每著加秒:", this);
    incrementTitleLabel->setFont(labelFont);
    timeControlLayout->addWidget(incrementTitleLabel);
    
    m_incrementLabel = new QLabel("0秒", this);
    m_incrementLabel->setFont(labelFont);
    m_incrementLabel->setAlignment(Qt::AlignCenter);
    timeControlLayout->addWidget(m_incrementLabel);
    
    m_incrementSlider = new QSlider(Qt::Horizontal, this);
    m_incrementSlider->setMinimum(0);
    m_incrementSlider->setMaximum(60);
    m_incrementSlider->setValue(0);
    m_incrementSlider->setTickPosition(QSlider::TicksBelow);
    m_incrementSlider->setTickInterval(5);
    connect(m_incrementSlider, &QSlider::valueChanged, this, &Qt_Chess::onIncrementChanged);
    timeControlLayout->addWidget(m_incrementSlider);
    
    // Black time display
    QLabel* blackTimeTitle = new QLabel("黑方時間:", this);
    blackTimeTitle->setFont(labelFont);
    timeControlLayout->addWidget(blackTimeTitle);
    
    m_blackTimeLabel = new QLabel("--:--", this);
    QFont timeFont;
    timeFont.setPointSize(14);
    timeFont.setBold(true);
    m_blackTimeLabel->setFont(timeFont);
    m_blackTimeLabel->setAlignment(Qt::AlignCenter);
    m_blackTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_blackTimeLabel->setMinimumHeight(40);
    timeControlLayout->addWidget(m_blackTimeLabel);
    
    // White time display
    QLabel* whiteTimeTitle = new QLabel("白方時間:", this);
    whiteTimeTitle->setFont(labelFont);
    timeControlLayout->addWidget(whiteTimeTitle);
    
    m_whiteTimeLabel = new QLabel("--:--", this);
    m_whiteTimeLabel->setFont(timeFont);
    m_whiteTimeLabel->setAlignment(Qt::AlignCenter);
    m_whiteTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    m_whiteTimeLabel->setMinimumHeight(40);
    timeControlLayout->addWidget(m_whiteTimeLabel);
    
    // Start button
    m_startButton = new QPushButton("開始", this);
    m_startButton->setMinimumHeight(40);
    QFont startButtonFont;
    startButtonFont.setPointSize(12);
    startButtonFont.setBold(true);
    m_startButton->setFont(startButtonFont);
    m_startButton->setEnabled(false);  // Initially disabled
    connect(m_startButton, &QPushButton::clicked, this, &Qt_Chess::onStartButtonClicked);
    timeControlLayout->addWidget(m_startButton);
    
    // Add stretch to push everything to the top
    timeControlLayout->addStretch();
    
    rightPanelLayout->addWidget(timeControlGroup);
    
    // Initialize game timer
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &Qt_Chess::onGameTimerTick);
}

void Qt_Chess::onTimeLimitChanged(int value) {
    if (!m_timeLimitSlider || !m_timeLimitLabel) return;
    
    if (value == 0) {
        // Unlimited time
        m_timeLimitLabel->setText("不限時");
        m_timeControlEnabled = false;
        stopTimer();
        m_timerStarted = false;
        if (m_startButton) {
            m_startButton->setEnabled(false);
            m_startButton->setText("開始");
        }
    } else {
        // Set time limit
        // Format the label based on the value
        QString timeText;
        if (value < 60) {
            timeText = QString("%1秒").arg(value);
        } else if (value % 60 == 0) {
            timeText = QString("%1分鐘").arg(value / 60);
        } else {
            int minutes = value / 60;
            int seconds = value % 60;
            timeText = QString("%1分%2秒").arg(minutes).arg(seconds);
        }
        m_timeLimitLabel->setText(timeText);
        
        m_timeControlEnabled = true;
        m_whiteTimeMs = value * 1000;
        m_blackTimeMs = value * 1000;
        m_timerStarted = false;
        if (m_startButton) {
            m_startButton->setEnabled(true);
            m_startButton->setText("開始");
        }
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
    
    // Convert milliseconds to minutes:seconds
    auto formatTime = [](int ms) -> QString {
        int totalSeconds = ms / 1000;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    };
    
    m_whiteTimeLabel->setText(formatTime(m_whiteTimeMs));
    m_blackTimeLabel->setText(formatTime(m_blackTimeMs));
    
    // Highlight the active player's timer
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (currentPlayer == PieceColor::White) {
        m_whiteTimeLabel->setStyleSheet("QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
        m_blackTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    } else {
        m_blackTimeLabel->setStyleSheet("QLabel { background-color: rgba(76, 175, 80, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
        m_whiteTimeLabel->setStyleSheet("QLabel { background-color: rgba(51, 51, 51, 200); color: #FFF; padding: 8px; border-radius: 5px; }");
    }
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
        m_whiteTimeMs -= 100; // Decrease by 100ms (timer ticks every 100ms)
        if (m_whiteTimeMs <= 0) {
            m_whiteTimeMs = 0;
            updateTimeDisplays();
            stopTimer();
            m_timerStarted = false;  // Reset timer state
            if (m_startButton) {
                m_startButton->setText("開始");
                m_startButton->setEnabled(true);
            }
            QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
            return;
        }
    } else {
        m_blackTimeMs -= 100;
        if (m_blackTimeMs <= 0) {
            m_blackTimeMs = 0;
            updateTimeDisplays();
            stopTimer();
            m_timerStarted = false;  // Reset timer state
            if (m_startButton) {
                m_startButton->setText("開始");
                m_startButton->setEnabled(true);
            }
            QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
            return;
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

void Qt_Chess::loadTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    
    int timeLimitSeconds = settings.value("timeLimitSeconds", 0).toInt();
    int incrementSeconds = settings.value("incrementSeconds", 0).toInt();
    
    // Validate and normalize time limit (convert legacy -1 to 0 for unlimited)
    if (timeLimitSeconds < 0) {
        timeLimitSeconds = 0;  // Normalize legacy unlimited setting
    }
    // Clamp to valid range [0, MAX_TIME_LIMIT_SECONDS]
    timeLimitSeconds = qBound(0, timeLimitSeconds, MAX_TIME_LIMIT_SECONDS);
    
    // Set the time limit slider
    if (m_timeLimitSlider) {
        m_timeLimitSlider->setValue(timeLimitSeconds);
    }
    
    // Set increment
    if (m_incrementSlider) {
        m_incrementSlider->setValue(incrementSeconds);
    }
    
    m_incrementMs = incrementSeconds * 1000;
    
    if (timeLimitSeconds == 0) {
        m_timeControlEnabled = false;
    } else {
        m_timeControlEnabled = true;
        m_whiteTimeMs = timeLimitSeconds * 1000;
        m_blackTimeMs = timeLimitSeconds * 1000;
    }
}

void Qt_Chess::saveTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    
    if (m_timeLimitSlider) {
        settings.setValue("timeLimitSeconds", m_timeLimitSlider->value());
    }
    
    if (m_incrementSlider) {
        settings.setValue("incrementSeconds", m_incrementSlider->value());
    }
    
    settings.sync();
}
