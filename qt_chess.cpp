#include "qt_chess.h"
#include "ui_qt_chess.h"
#include <QMessageBox>
#include <QFont>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QResizeEvent>

namespace {
    const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
}

namespace {
    const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
}

Qt_Chess::Qt_Chess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Qt_Chess)
    , m_selectedSquare(-1, -1)
    , m_pieceSelected(false)
    , m_isDragging(false)
    , m_dragStartSquare(-1, -1)
    , m_dragLabel(nullptr)
    , m_boardWidget(nullptr)
    , m_moveSound(nullptr)
    , m_captureSound(nullptr)
    , m_checkSound(nullptr)
    , m_castlingSound(nullptr)
    , m_checkmateSound(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(700, 750);
    setMouseTracking(true);
    
    setupUI();
    setupSounds();
    updateBoard();
    updateStatus();
}

Qt_Chess::~Qt_Chess()
{
    delete m_moveSound;
    delete m_captureSound;
    delete m_checkSound;
    delete m_castlingSound;
    delete m_checkmateSound;
    delete ui;
}

void Qt_Chess::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 回合和狀態標籤
    m_turnLabel = new QLabel("當前回合：白方", this);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    QFont font = m_turnLabel->font();
    font.setPointSize(14);
    font.setBold(true);
    m_turnLabel->setFont(font);
    mainLayout->addWidget(m_turnLabel);
    
    m_statusLabel = new QLabel("對局進行中", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFont(font);
    mainLayout->addWidget(m_statusLabel);
    
    // 棋盤
    m_boardWidget = new QWidget(this);
    m_boardWidget->setMouseTracking(true);
    QGridLayout* gridLayout = new QGridLayout(m_boardWidget);
    gridLayout->setSpacing(0);
    
    m_squares.resize(8, std::vector<QPushButton*>(8));
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = new QPushButton(m_boardWidget);
            square->setMinimumSize(40, 40);  // 設置合理的最小尺寸
            square->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            square->setMouseTracking(true);
            
            QFont buttonFont;
            buttonFont.setPointSize(36);
            square->setFont(buttonFont);
            
            m_squares[row][col] = square;
            gridLayout->addWidget(square, row, col);
            
            // 儲存按鈕座標以便在 eventFilter 中高效查找
            m_buttonCoordinates[square] = QPoint(col, row);
            
            // 安裝事件過濾器以捕獲拖放的滑鼠事件
            square->installEventFilter(this);
            
            connect(square, &QPushButton::clicked, this, [this, row, col]() {
                onSquareClicked(row, col);
            });
            
            updateSquareColor(row, col);
        }
    }
    
    mainLayout->addWidget(m_boardWidget, 0, Qt::AlignCenter);
    
    // 新遊戲按鈕
    m_newGameButton = new QPushButton("新遊戲", this);
    m_newGameButton->setMinimumHeight(40);
    m_newGameButton->setFont(font);
    connect(m_newGameButton, &QPushButton::clicked, this, &Qt_Chess::onNewGameClicked);
    mainLayout->addWidget(m_newGameButton);
    
    setCentralWidget(centralWidget);
}

void Qt_Chess::setupSounds() {
    // 初始化音效物件
    m_moveSound = new QSoundEffect(this);
    m_moveSound->setSource(QUrl("qrc:/sounds/sounds/move.wav"));
    m_moveSound->setVolume(0.5);
    
    m_captureSound = new QSoundEffect(this);
    m_captureSound->setSource(QUrl("qrc:/sounds/sounds/capture.wav"));
    m_captureSound->setVolume(0.5);
    
    m_checkSound = new QSoundEffect(this);
    m_checkSound->setSource(QUrl("qrc:/sounds/sounds/check.wav"));
    m_checkSound->setVolume(0.5);
    
    m_castlingSound = new QSoundEffect(this);
    m_castlingSound->setSource(QUrl("qrc:/sounds/sounds/castling.wav"));
    m_castlingSound->setVolume(0.5);
    
    m_checkmateSound = new QSoundEffect(this);
    m_checkmateSound->setSource(QUrl("qrc:/sounds/sounds/checkmate.wav"));
    m_checkmateSound->setVolume(0.5);
}

void Qt_Chess::playMoveSound(const MoveInfo& moveInfo) {
    // 根據移動類型播放相應的音效
    // 優先級: 將死 > 王車易位 > 將軍 > 吃子 > 一般移動
    if (moveInfo.isCheckmate) {
        m_checkmateSound->play();
    } else if (moveInfo.isCastling) {
        m_castlingSound->play();
    } else if (moveInfo.isCheck) {
        m_checkSound->play();
    } else if (moveInfo.isCapture) {
        m_captureSound->play();
    } else {
        m_moveSound->play();
    }
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
            m_squares[row][col]->setText(piece.getSymbol());
            updateSquareColor(row, col);
        }
    }
    
    // 如果王被將軍則以紅色高亮顯示
    applyCheckHighlight();
}

void Qt_Chess::updateStatus() {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    QString playerName = (currentPlayer == PieceColor::White) ? "白方" : "黑方";
    
    m_turnLabel->setText(QString("當前回合：%1").arg(playerName));
    
    if (m_chessBoard.isCheckmate(currentPlayer)) {
        QString winner = (currentPlayer == PieceColor::White) ? "黑方" : "白方";
        m_statusLabel->setText(QString("將死！%1獲勝！").arg(winner));
        m_statusLabel->setStyleSheet("QLabel { color: red; }");
        QMessageBox::information(this, "遊戲結束", QString("將死！%1獲勝！").arg(winner));
    } else if (m_chessBoard.isStalemate(currentPlayer)) {
        m_statusLabel->setText("逼和！對局和棋。");
        m_statusLabel->setStyleSheet("QLabel { color: orange; }");
        QMessageBox::information(this, "遊戲結束", "逼和！對局和棋。");
    } else if (m_chessBoard.isInsufficientMaterial()) {
        m_statusLabel->setText("子力不足！對局和棋。");
        m_statusLabel->setStyleSheet("QLabel { color: orange; }");
        QMessageBox::information(this, "遊戲結束", "子力不足以將殺！對局和棋。");
    } else if (m_chessBoard.isInCheck(currentPlayer)) {
        m_statusLabel->setText(QString("%1被將軍！").arg(playerName));
        m_statusLabel->setStyleSheet("QLabel { color: red; }");
    } else {
        m_statusLabel->setText("對局進行中");
        m_statusLabel->setStyleSheet("QLabel { color: black; }");
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
    
    // 如果王在被將軍狀態，重新套用紅色背景
    applyCheckHighlight();
}

void Qt_Chess::highlightValidMoves() {
    clearHighlights();
    
    if (!m_pieceSelected) return;
    
    // 高亮選中的格子
    m_squares[m_selectedSquare.y()][m_selectedSquare.x()]->setStyleSheet(
        "QPushButton { background-color: #7FC97F; border: 2px solid #00FF00; }"
    );
    
    // 高亮有效的移動
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (m_chessBoard.isValidMove(m_selectedSquare, QPoint(col, row))) {
                bool isLight = (row + col) % 2 == 0;
                QString color = isLight ? "#FFE4B5" : "#DEB887";
                m_squares[row][col]->setStyleSheet(
                    QString("QPushButton { background-color: %1; border: 2px solid #FFA500; }").arg(color)
                );
            }
        }
    }
    
    // 如果王在被將軍狀態且王不是選中的棋子，重新套用紅色背景
    applyCheckHighlight(m_selectedSquare);
}

void Qt_Chess::onSquareClicked(int row, int col) {
    QPoint clickedSquare(col, row);
    
    if (!m_pieceSelected) {
        // 嘗試選擇棋子
        const ChessPiece& piece = m_chessBoard.getPiece(row, col);
        if (piece.getType() != PieceType::None && 
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
            m_selectedSquare = clickedSquare;
            m_pieceSelected = true;
            highlightValidMoves();
        }
    } else {
        // 嘗試移動選中的棋子
        MoveInfo moveInfo;
        if (m_chessBoard.movePiece(m_selectedSquare, clickedSquare, &moveInfo)) {
            m_pieceSelected = false;
            updateBoard();
            
            // 檢查是否需要兵升變
            if (m_chessBoard.needsPromotion(clickedSquare)) {
                const ChessPiece& piece = m_chessBoard.getPiece(clickedSquare.y(), clickedSquare.x());
                PieceType promotionType = showPromotionDialog(piece.getColor());
                m_chessBoard.promotePawn(clickedSquare, promotionType);
                updateBoard();
            }
            
            // 播放移動音效
            playMoveSound(moveInfo);
            
            updateStatus();
        } else if (clickedSquare == m_selectedSquare) {
            // 取消選擇棋子
            m_pieceSelected = false;
            clearHighlights();
        } else {
            // 嘗試選擇相同顏色的另一個棋子
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
    
    // 為每個升變選項建立按鈕
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

void Qt_Chess::restorePieceToSquare(const QPoint& square) {
    if (square.x() >= 0 && square.y() >= 0 && square.x() < 8 && square.y() < 8) {
        const ChessPiece& piece = m_chessBoard.getPiece(square.y(), square.x());
        m_squares[square.y()][square.x()]->setText(piece.getSymbol());
    }
}

bool Qt_Chess::eventFilter(QObject *obj, QEvent *event) {
    // 檢查事件是否來自我們的棋盤格按鈕
    QPushButton* button = qobject_cast<QPushButton*>(obj);
    if (!button) {
        return QMainWindow::eventFilter(obj, event);
    }
    
    // 使用高效的映射查找檢查此按鈕是否為我們的棋盤格之一
    if (!m_buttonCoordinates.contains(button)) {
        return QMainWindow::eventFilter(obj, event);
    }
    
    // 轉發滑鼠事件以啟用拖放功能
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // 將按鈕的位置映射到主視窗的座標系統
        QPoint globalPos = button->mapToGlobal(mouseEvent->pos());
        QPoint windowPos = mapFromGlobal(globalPos);
        QMouseEvent mappedEvent(mouseEvent->type(), windowPos, mouseEvent->button(), 
                               mouseEvent->buttons(), mouseEvent->modifiers());
        mousePressEvent(&mappedEvent);
        // 不完全接受事件 - 如果沒有開始拖動，讓按鈕仍然處理點擊
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
    
    // 將事件傳遞給父類別進行標準處理
    return QMainWindow::eventFilter(obj, event);
}

void Qt_Chess::mousePressEvent(QMouseEvent *event) {
    QPoint square = getSquareAtPosition(event->pos());
    
    // 右鍵點擊 - 取消當前動作
    if (event->button() == Qt::RightButton) {
        if (m_isDragging) {
            // 取消拖動並將棋子歸位
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
            // 如果已選中棋子，取消選擇
            m_pieceSelected = false;
            clearHighlights();
        }
        return;
    }
    
    // 左鍵點擊 - 開始拖動
    if (event->button() == Qt::LeftButton && square.x() >= 0 && square.y() >= 0 && 
        square.x() < 8 && square.y() < 8) {
        const ChessPiece& piece = m_chessBoard.getPiece(square.y(), square.x());
        if (piece.getType() != PieceType::None && 
            piece.getColor() == m_chessBoard.getCurrentPlayer()) {
            
            m_isDragging = true;
            m_dragStartSquare = square;
            m_selectedSquare = square;
            m_pieceSelected = true;
            
            // 建立拖動標籤
            m_dragLabel = new QLabel(this);
            m_dragLabel->setText(piece.getSymbol());
            QFont font;
            font.setPointSize(36);
            m_dragLabel->setFont(font);
            m_dragLabel->setStyleSheet("QLabel { background-color: transparent; border: none; }");
            m_dragLabel->adjustSize();
            m_dragLabel->move(event->pos() - QPoint(m_dragLabel->width() / 2, m_dragLabel->height() / 2));
            m_dragLabel->show();
            m_dragLabel->raise();
            
            // 拖動期間從原始格子隱藏棋子
            m_squares[square.y()][square.x()]->setText("");
            
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
        QPoint dropSquare = getSquareAtPosition(event->pos());
        
        // 清理拖動標籤
        if (m_dragLabel) {
            m_dragLabel->hide();
            m_dragLabel->deleteLater();
            m_dragLabel = nullptr;
        }
        
        m_isDragging = false;
        
        if (dropSquare.x() >= 0 && dropSquare.y() >= 0) {
            // 嘗試移動棋子
            MoveInfo moveInfo;
            if (m_chessBoard.movePiece(m_dragStartSquare, dropSquare, &moveInfo)) {
                m_pieceSelected = false;
                updateBoard();
                
                // 檢查是否需要兵升變
                if (m_chessBoard.needsPromotion(dropSquare)) {
                    const ChessPiece& piece = m_chessBoard.getPiece(dropSquare.y(), dropSquare.x());
                    PieceType promotionType = showPromotionDialog(piece.getColor());
                    m_chessBoard.promotePawn(dropSquare, promotionType);
                    updateBoard();
                }
                
                // 播放移動音效
                playMoveSound(moveInfo);
                
                updateStatus();
                clearHighlights();
            } else if (dropSquare == m_dragStartSquare) {
                // 放在同一格上 - 取消選擇
                // 將棋子恢復到原始格子
                restorePieceToSquare(m_dragStartSquare);
                m_pieceSelected = false;
                clearHighlights();
            } else {
                // 無效移動 - 嘗試選擇另一個棋子
                const ChessPiece& piece = m_chessBoard.getPiece(dropSquare.y(), dropSquare.x());
                if (piece.getType() != PieceType::None && 
                    piece.getColor() == m_chessBoard.getCurrentPlayer()) {
                    // 先將棋子恢復到原始格子
                    restorePieceToSquare(m_dragStartSquare);
                    m_selectedSquare = dropSquare;
                    m_pieceSelected = true;
                    highlightValidMoves();
                } else {
                    // 無效移動且沒有選擇另一個棋子
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
}

void Qt_Chess::updateSquareSizes() {
    if (!m_boardWidget || m_squares.empty()) return;
    
    // 取得中央小工具
    QWidget* central = centralWidget();
    if (!central) return;
    
    // 計算棋盤的可用空間
    // 考慮回合標籤、狀態標籤和新遊戲按鈕
    int reservedHeight = 0;
    if (m_turnLabel) reservedHeight += m_turnLabel->sizeHint().height();
    if (m_statusLabel) reservedHeight += m_statusLabel->sizeHint().height();
    if (m_newGameButton) reservedHeight += m_newGameButton->minimumHeight();
    
    // 為佈局邊距和間距添加一些填充（估計約 50px）
    reservedHeight += 50;
    
    int availableWidth = central->width();
    int availableHeight = central->height() - reservedHeight;
    
    // 計算每個格子的大小（使用較小的維度以保持格子為正方形）
    int squareSize = qMin(availableWidth, availableHeight) / 8;
    
    // 確保最小和合理的最大尺寸
    squareSize = qMax(squareSize, 40);
    squareSize = qMin(squareSize, 120);  // 限制在合理的最大值
    
    // 根據格子大小計算字體大小（約為格子大小的 45%）
    int fontSize = squareSize * 9 / 20;  // 對於 80px 的格子約為 36pt
    fontSize = qMax(fontSize, 12);  // 確保最小可讀字體大小
    fontSize = qMin(fontSize, 54);  // 限制非常大棋盤的字體大小
    
    // 更新所有格子
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = m_squares[row][col];
            square->setFixedSize(squareSize, squareSize);
            
            QFont font = square->font();
            font.setPointSize(fontSize);
            square->setFont(font);
        }
    }
    
    // 更新棋盤小工具的大小以精確適應格子
    m_boardWidget->setFixedSize(squareSize * 8, squareSize * 8);
}
