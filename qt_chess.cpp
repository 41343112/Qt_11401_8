#include "qt_chess.h"
#include "ui_qt_chess.h"
#include <QMessageBox>
#include <QFont>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace {
    const QString CHECK_HIGHLIGHT_STYLE = "QPushButton { background-color: #FF6B6B; border: 2px solid #FF0000; }";
}

Qt_Chess::Qt_Chess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Qt_Chess)
    , m_selectedSquare(-1, -1)
    , m_pieceSelected(false)
{
    ui->setupUi(this);
    setWindowTitle("國際象棋 - 雙人對弈");
    resize(700, 750);
    
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
    
    // Turn and status labels
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
    
    // Chess board
    QWidget* boardWidget = new QWidget(this);
    QGridLayout* gridLayout = new QGridLayout(boardWidget);
    gridLayout->setSpacing(0);
    
    m_squares.resize(8, std::vector<QPushButton*>(8));
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QPushButton* square = new QPushButton(boardWidget);
            square->setMinimumSize(80, 80);
            square->setMaximumSize(80, 80);
            
            QFont buttonFont;
            buttonFont.setPointSize(36);
            square->setFont(buttonFont);
            
            m_squares[row][col] = square;
            gridLayout->addWidget(square, row, col);
            
            connect(square, &QPushButton::clicked, this, [this, row, col]() {
                onSquareClicked(row, col);
            });
            
            updateSquareColor(row, col);
        }
    }
    
    mainLayout->addWidget(boardWidget, 0, Qt::AlignCenter);
    
    // New game button
    m_newGameButton = new QPushButton("新遊戲", this);
    m_newGameButton->setMinimumHeight(40);
    m_newGameButton->setFont(font);
    connect(m_newGameButton, &QPushButton::clicked, this, &Qt_Chess::onNewGameClicked);
    mainLayout->addWidget(m_newGameButton);
    
    setCentralWidget(centralWidget);
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
    
    // Highlight king in red if in check
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

void Qt_Chess::applyCheckHighlight() {
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (m_chessBoard.isInCheck(currentPlayer)) {
        QPoint kingPos = m_chessBoard.findKing(currentPlayer);
        if (kingPos.x() >= 0 && kingPos.y() >= 0) {
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
            if (m_chessBoard.isValidMove(m_selectedSquare, QPoint(col, row))) {
                bool isLight = (row + col) % 2 == 0;
                QString color = isLight ? "#FFE4B5" : "#DEB887";
                m_squares[row][col]->setStyleSheet(
                    QString("QPushButton { background-color: %1; border: 2px solid #FFA500; }").arg(color)
                );
            }
        }
    }
    
    // Re-apply red background to king if in check and king is not the selected piece
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (m_chessBoard.isInCheck(currentPlayer)) {
        QPoint kingPos = m_chessBoard.findKing(currentPlayer);
        if (kingPos.x() >= 0 && kingPos.y() >= 0 && kingPos != m_selectedSquare) {
            m_squares[kingPos.y()][kingPos.x()]->setStyleSheet(CHECK_HIGHLIGHT_STYLE);
        }
    }
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
