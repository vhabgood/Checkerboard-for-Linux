#include <QDebug>
#include "BoardWidget.h"
#include "GameManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDir>
#include "log.h"

extern "C" {
#include "c_logic.h"
}


#include "GameManager.h"

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent)
{
    m_squareSize = 60;
    m_inverted = false;
    m_showCoordinates = true;
    m_coordinateColor = Qt::blue;
    m_highlightColor = Qt::yellow;
    m_selectedX = -1;
    m_selectedY = -1;
    m_pieceSelected = false;
    m_currentSetupPieceType = CB_EMPTY;
    m_isTogglePieceColorMode = false;
    m_pieceSet = "standard";
    m_mirrored = false;
    m_highlight = true;
    m_piecePixmaps = QMap<int, QPixmap>();

    setFixedSize(8 * m_squareSize, 8 * m_squareSize);
    loadPiecePixmaps();
}

void BoardWidget::setSetupPieceType(int pieceType) {
    m_currentSetupPieceType = pieceType;
}

void BoardWidget::setTogglePieceColorMode(bool toggle) {
    m_isTogglePieceColorMode = toggle;
}

BoardWidget::~BoardWidget()
{
}

void BoardWidget::setSelectedPiece(int x, int y) {
    m_selectedX = x;
    m_selectedY = y;
    m_pieceSelected = true;
    update();
}

void BoardWidget::clearSelectedPiece() {
    m_selectedX = -1;
    m_selectedY = -1;
    m_pieceSelected = false;
    update();
}

void BoardWidget::setBoard(const bitboard_pos& board)
{
    // qDebug() << "BoardWidget::setBoard called";
    m_board = board;
    update(); // Request a repaint
}

void BoardWidget::setInverted(bool inverted) {
    if (m_inverted != inverted) {
        m_inverted = inverted;
        update(); // Request repaint
    }
}

void BoardWidget::setShowCoordinates(bool show) {
    if (m_showCoordinates != show) {
        m_showCoordinates = show;
        update(); // Request repaint
    }
}



void BoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            // Calculate the logical board coordinates for this square
            int logicalRow = r;
            int logicalCol = c;

            // Get the screen bitboard_position for the logical coordinates
            QPoint screenPos = boardToScreen(logicalCol, logicalRow);

            // Draw square
            if ((r + c) % 2 == 0) { // Use original r, c for checkerboard pattern
                painter.fillRect(screenPos.x(), screenPos.y(), m_squareSize, m_squareSize, Qt::lightGray);
            } else {
                painter.fillRect(screenPos.x(), screenPos.y(), m_squareSize, m_squareSize, Qt::black);
            }

            // Draw highlight for selected piece
            // m_selectedX and m_selectedY are already in logical board coordinates
            if (m_highlight && m_pieceSelected && m_selectedX == logicalCol && m_selectedY == logicalRow) {
                painter.setPen(QPen(m_highlightColor, 3)); // Yellow border, 3 pixels thick
                painter.drawRect(screenPos.x(), screenPos.y(), m_squareSize, m_squareSize);
            }

            // Draw piece if present
            int square_num = coorstonumber(c, r, GT_ENGLISH); // Get 1-indexed square number
            if (square_num != 0) { // Only dark squares have pieces
                int piece = get_piece(&m_board, square_num - 1); // Get piece from bitboard (0-indexed)
                if (piece != CB_EMPTY && m_piecePixmaps.contains(piece)) {
                    painter.drawPixmap(screenPos.x(), screenPos.y(), m_piecePixmaps.value(piece));
                }
            }

            // Draw coordinates if enabled
            if (m_showCoordinates && ((r + c) % 2 != 0)) { // Use original r, c for checkerboard pattern
                painter.setPen(m_coordinateColor);
                int board_num = coorstonumber(c, r, GT_ENGLISH); // Use original c, r for board number
                painter.drawText(screenPos.x() + 5, screenPos.y() + m_squareSize - 5, QString::number(board_num));
            }
        }
    }
}

void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint boardPos = screenToBoard(event->pos());
        if (boardPos.x() >= 0 && boardPos.x() < 8 && boardPos.y() >= 0 && boardPos.y() < 8) {
            emit squareClicked(boardPos.x(), boardPos.y());
        }
    }
}

void BoardWidget::setPieceSet(const QString& pieceSet)
{
    if (m_pieceSet != pieceSet) {
        m_pieceSet = pieceSet;
        loadPiecePixmaps();
        update();
    }
}

void BoardWidget::setMirror(bool mirrored)
{
    if (m_mirrored != mirrored) {
        m_mirrored = mirrored;
        update(); // Trigger a repaint
    }
}

void BoardWidget::setHighlight(bool highlight)
{
    if (m_highlight != highlight) {
        m_highlight = highlight;
        update(); // Trigger a repaint
    }
}

void BoardWidget::loadPiecePixmaps()
{
    m_piecePixmaps.clear();
    QString basePath = ":/" + m_pieceSet + "/bmp/" + m_pieceSet + "/";

    QPixmap bm(basePath + "bm.bmp");
    QPixmap bk(basePath + "bk.bmp");
    QPixmap wm(basePath + "wm.bmp");
    QPixmap wk(basePath + "wk.bmp");

    m_piecePixmaps.insert(CB_BLACK | CB_MAN, bm.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_piecePixmaps.insert(CB_BLACK | CB_KING, bk.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_piecePixmaps.insert(CB_WHITE | CB_MAN, wm.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_piecePixmaps.insert(CB_WHITE | CB_KING, wk.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Check if pixmaps loaded successfully
    for (int key : m_piecePixmaps.keys()) {
        if (m_piecePixmaps.value(key).isNull()) {
            log_c(LOG_LEVEL_ERROR, QString("Failed to load pixmap for piece type: %1 from path: %2").arg(key).arg(basePath + (key == (CB_BLACK | CB_MAN) ? "bm.bmp" : (key == (CB_BLACK | CB_KING) ? "bk.bmp" : (key == (CB_WHITE | CB_MAN) ? "wm.bmp" : "wk.bmp")))).toUtf8().constData());
        } else {
        }
    }
}

QPoint BoardWidget::boardToScreen(int x, int y) const
{
    int screenX = x;
    int screenY = y;

    if (m_inverted) {
        screenY = 7 - screenY;
    }
    if (m_mirrored) {
        screenX = 7 - screenX;
    }

    return QPoint(screenX * m_squareSize, screenY * m_squareSize);
}

QPoint BoardWidget::screenToBoard(const QPoint& screenPos) const
{
    int x = screenPos.x() / m_squareSize;
    int y = screenPos.y() / m_squareSize;

    if (m_inverted) {
        y = 7 - y;
    }
    if (m_mirrored) {
        x = 7 - x;
    }

    return QPoint(x, y);
}