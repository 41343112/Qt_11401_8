#include "qt_chess.h"
#include "ui_qt_chess.h"
#include "soundsettingsdialog.h"
#include "pieceiconsettingsdialog.h"
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

namespace {
    const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
    const int DEFAULT_ICON_SIZE = 40; // Default fallback icon size in pixels
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
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(700, 660);
    
    // Set minimum window size: width 320px (8 squares × 40px), height 380px (320px board + ~60px UI elements)
    setMinimumSize(320, 380);
    
    setMouseTracking(true);
    
    loadSoundSettings();
    initializeSounds();
    loadPieceIconSettings();
    loadPieceIconsToCache(); // Load icons to cache after loading settings
    setupMenuBar();
    setupUI();
    updateBoard();
    updateStatus();
}

Qt_Chess::~Qt_Chess()
{
    delete ui;
}

void Qt_Chess::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Chess board
    m_boardWidget = new QWidget(this);
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
    
    mainLayout->addWidget(m_boardWidget, 0, Qt::AlignCenter);
    
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
}

void Qt_Chess::updateSquareColor(int row, int col) {
    bool isLight = (row + col) % 2 == 0;
    QString color = isLight ? "#F0D9B5" : "#B58863";
    m_squares[row][col]->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid #333; }").arg(color)
    );
}

void Qt_Chess::updateBoard() {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            displayPieceOnSquare(m_squares[row][col], piece);
            updateSquareColor(row, col);
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
            int row = kingPos.y();
            int col = kingPos.x();
            m_squares[row][col]->setStyleSheet(CHECK_HIGHLIGHT_STYLE);
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
    
    // Highlight selected square
    m_squares[m_selectedSquare.y()][m_selectedSquare.x()]->setStyleSheet(
        "QPushButton { background-color: #7FC97F; border: 2px solid #00FF00; }"
    );
    
    // Highlight valid moves
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPoint targetSquare(col, row);
            if (m_chessBoard.isValidMove(m_selectedSquare, targetSquare)) {
                bool isCapture = isCaptureMove(m_selectedSquare, targetSquare);
                bool isLight = (row + col) % 2 == 0;
                
                if (isCapture) {
                    // Highlight capture moves in red
                    QString color = isLight ? "#FFB3B3" : "#FF8080";
                    m_squares[row][col]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FF0000; }").arg(color)
                    );
                } else {
                    // Highlight non-capture moves in orange
                    QString color = isLight ? "#FFE4B5" : "#DEB887";
                    m_squares[row][col]->setStyleSheet(
                        QString("QPushButton { background-color: %1; border: 2px solid #FFA500; }").arg(color)
                    );
                }
            }
        }
    }
    
    // Re-apply red background to king if in check and king is not the selected piece
    applyCheckHighlight(m_selectedSquare);
}

void Qt_Chess::onSquareClicked(int row, int col) {
    QPoint clickedSquare(col, row);
    
    if (!m_pieceSelected) {
        // Try to select a piece
        const ChessPiece& piece = m_chessBoard.getPiece(row, col);
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
            
            updateStatus();
        } else if (clickedSquare == m_selectedSquare) {
            // Deselect the piece
            m_pieceSelected = false;
            clearHighlights();
        } else {
            // Try to select a different piece of the same color
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
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
    updateBoard();
    updateStatus();
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

void Qt_Chess::restorePieceToSquare(const QPoint& square) {
    if (square.x() >= 0 && square.y() >= 0 && square.x() < 8 && square.y() < 8) {
        const ChessPiece& piece = m_chessBoard.getPiece(square.y(), square.x());
        displayPieceOnSquare(m_squares[square.y()][square.x()], piece);
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
    QPoint square = getSquareAtPosition(event->pos());
    
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
    if (event->button() == Qt::LeftButton && square.x() >= 0 && square.y() >= 0 && 
        square.x() < 8 && square.y() < 8) {
        const ChessPiece& piece = m_chessBoard.getPiece(square.y(), square.x());
        if (piece.getType() != PieceType::None && 
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
            
            // Track if this piece was already selected before the drag
            m_wasSelectedBeforeDrag = (m_pieceSelected && m_selectedSquare == square);
            
            m_isDragging = true;
            m_dragStartSquare = square;
            m_selectedSquare = square;
            m_pieceSelected = true;
            
            // Create drag label
            m_dragLabel = new QLabel(this);
            
            // Use custom icon or Unicode symbol
            if (m_pieceIconSettings.useCustomIcons) {
                QPixmap pixmap = getCachedPieceIcon(piece.getType(), piece.getColor());
                if (!pixmap.isNull()) {
                    QPushButton* squareButton = m_squares[square.y()][square.x()];
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
            m_squares[square.y()][square.x()]->setText("");
            m_squares[square.y()][square.x()]->setIcon(QIcon());
            
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
        QPoint dropSquare = getSquareAtPosition(event->pos());
        
        // Clean up drag label
        if (m_dragLabel) {
            m_dragLabel->hide();
            m_dragLabel->deleteLater();
            m_dragLabel = nullptr;
        }
        
        m_isDragging = false;
        
        if (dropSquare.x() >= 0 && dropSquare.y() >= 0) {
            // Detect move type before executing the move
            bool isCapture = isCaptureMove(m_dragStartSquare, dropSquare);
            bool isCastling = isCastlingMove(m_dragStartSquare, dropSquare);
            
            // Try to move the piece
            if (m_chessBoard.movePiece(m_dragStartSquare, dropSquare)) {
                m_pieceSelected = false;
                updateBoard();
                
                // Check if pawn promotion is needed
                if (m_chessBoard.needsPromotion(dropSquare)) {
                    const ChessPiece& piece = m_chessBoard.getPiece(dropSquare.y(), dropSquare.x());
                    PieceType promotionType = showPromotionDialog(piece.getColor());
                    m_chessBoard.promotePawn(dropSquare, promotionType);
                    updateBoard();
                }
                
                // Play appropriate sound effect
                playSoundForMove(isCapture, isCastling);
                
                updateStatus();
                clearHighlights();
            } else if (dropSquare == m_dragStartSquare) {
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
                const ChessPiece& piece = m_chessBoard.getPiece(dropSquare.y(), dropSquare.x());
                if (piece.getType() != PieceType::None && 
                    piece.getColor() == m_chessBoard.getCurrentPlayer()) {
                    // Restore the piece to the original square first
                    restorePieceToSquare(m_dragStartSquare);
                    m_selectedSquare = dropSquare;
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
                int iconSize = static_cast<int>(squareSize * m_pieceIconSettings.pieceScale / 100.0);
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
    m_pieceIconSettings.pieceScale = settings.value("PieceIcons/pieceScale", 80).toInt();
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
    settings.setValue("PieceIcons/pieceScale", m_pieceIconSettings.pieceScale);
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
    return static_cast<int>(squareWidth * m_pieceIconSettings.pieceScale / 100.0);
}
