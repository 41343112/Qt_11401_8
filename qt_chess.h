#ifndef QT_CHESS_H
#define QT_CHESS_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMap>
#include <QSoundEffect>
#include <vector>
#include "chessboard.h"

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

private slots:
    void onSquareClicked(int row, int col);
    void onNewGameClicked();

private:
    Ui::Qt_Chess *ui;
    ChessBoard m_chessBoard;
    std::vector<std::vector<QPushButton*>> m_squares;
    QMap<QPushButton*, QPoint> m_buttonCoordinates; // For efficient button-to-coordinate lookup
    QPoint m_selectedSquare;
    bool m_pieceSelected;
    
    // Drag-and-drop state
    bool m_isDragging;
    QPoint m_dragStartSquare;
    QLabel* m_dragLabel;
    
    QPushButton* m_newGameButton;
    QWidget* m_boardWidget;
    
    // Sound effects
    QSoundEffect m_moveSound;
    QSoundEffect m_captureSound;
    QSoundEffect m_castlingSound;
    QSoundEffect m_checkSound;
    QSoundEffect m_checkmateSound;
    
    void setupUI();
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
};
#endif // QT_CHESS_H
