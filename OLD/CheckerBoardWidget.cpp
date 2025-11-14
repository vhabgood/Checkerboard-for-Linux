#include "CheckerBoardWidget.h"
#include "c_functions.h"
#include "c_logic.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDir>
#include <QDebug>
#include <cstring> // For memcpy
#include <algorithm> // For std::min

// Assuming coordinates.c functions are available (need linkage)
// extern "C" {
// int coorstonumber(int x, int y, int gametype);
// void numbertocoors(int n, int *x, int *y, int gametype); // May need later
// void coorstocoors(int *x, int *y, int invert, int mirror); // Logic moved here
// }
// Or include coordinates.h if it declares them extern "C"

CheckerBoardWidget::CheckerBoardWidget(QWidget *parent) :
    QWidget(parent)
{
    // Initialize board to empty or starting state if preferred
    memset(&m_board, 0, sizeof(m_board)); // Initialize to empty
    // Or call a function to set starting position

    setMinimumSize(200, 200); // Set a reasonable minimum size
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // Allow resizing
    calculateBoardLayout();
    setMouseTracking(true); // Needed if we want hover effects later
} // End of constructor

CheckerBoardWidget::~CheckerBoardWidget()
{
}

void CheckerBoardWidget::loadPieceBitmaps(const QString &pieceSetDir)
{
    qDebug() << "Loading bitmaps from resource prefix:" << pieceSetDir;

    bool success = true;

    // Construct full resource paths and attempt to load each pixmap.
    // The pieceSetDir (e.g., ":/standard") acts as the base prefix.

    QString manMaskPath = pieceSetDir + "/bmp/standard/manmask.bmp";
    qDebug() << "Attempting to load:" << manMaskPath;
    if (!m_manMaskPixmap.load(manMaskPath)) {
        qWarning() << "Failed to load image:" << manMaskPath << ", isNull:" << m_manMaskPixmap.isNull() << ", cacheKey:" << m_manMaskPixmap.cacheKey(); success = false;
    }

    QString kingMaskPath = pieceSetDir + "/bmp/standard/kingmask.bmp";
    qDebug() << "Attempting to load:" << kingMaskPath;
    if (!m_kingMaskPixmap.load(kingMaskPath)) {
        qWarning() << "Failed to load image:" << kingMaskPath << ", isNull:" << m_kingMaskPixmap.isNull() << ", cacheKey:" << m_kingMaskPixmap.cacheKey(); success = false;
    }

    QString bmPath = pieceSetDir + "/bmp/standard/bm.bmp";
    qDebug() << "Attempting to load:" << bmPath;
    if (!m_blackManPixmap.load(bmPath)) {
        qWarning() << "Failed to load image:" << bmPath << ", isNull:" << m_blackManPixmap.isNull() << ", cacheKey:" << m_blackManPixmap.cacheKey(); success = false;
    }

    QString bkPath = pieceSetDir + "/bmp/standard/bk.bmp";
    qDebug() << "Attempting to load:" << bkPath;
    if (!m_blackKingPixmap.load(bkPath)) {
        qWarning() << "Failed to load image:" << bkPath << ", isNull:" << m_blackKingPixmap.isNull() << ", cacheKey:" << m_blackKingPixmap.cacheKey(); success = false;
    }

    QString wmPath = pieceSetDir + "/bmp/standard/wm.bmp";
    qDebug() << "Attempting to load:" << wmPath;
    if (!m_whiteManPixmap.load(wmPath)) {
        qWarning() << "Failed to load image:" << wmPath << ", isNull:" << m_whiteManPixmap.isNull() << ", cacheKey:" << m_whiteManPixmap.cacheKey(); success = false;
    }

    QString wkPath = pieceSetDir + "/bmp/standard/wk.bmp";
    qDebug() << "Attempting to load:" << wkPath;
    if (!m_whiteKingPixmap.load(wkPath)) {
        qWarning() << "Failed to load image:" << wkPath << ", isNull:" << m_whiteKingPixmap.isNull() << ", cacheKey:" << m_whiteKingPixmap.cacheKey(); success = false;
    }

    QString lightPath = pieceSetDir + "/bmp/standard/light.bmp";
    qDebug() << "Attempting to load:" << lightPath;
    if (!m_lightSquarePixmap.load(lightPath)) {
        qWarning() << "Failed to load image:" << lightPath << ", isNull:" << m_lightSquarePixmap.isNull() << ", cacheKey:" << m_lightSquarePixmap.cacheKey(); success = false;
    }

    QString darkPath = pieceSetDir + "/bmp/standard/dark.bmp";
    qDebug() << "Attempting to load:" << darkPath;
    if (!m_darkSquarePixmap.load(darkPath)) {
        qWarning() << "Failed to load image:" << darkPath << ", isNull:" << m_darkSquarePixmap.isNull() << ", cacheKey:" << m_darkSquarePixmap.cacheKey(); success = false;
    }

    if(success) {
        qInfo() << "Successfully loaded piece set from:" << pieceSetDir;
    } else {
        qWarning() << "Errors occurred loading piece set from:" << pieceSetDir;
    }

    calculateBoardLayout();
    update();
}

const QPixmap& CheckerBoardWidget::getCBPixmap(PixmapType type) const
{
    // Replaces getCBbitmap()
    switch (type) {
        case BMP_BLACK_MAN:     return m_blackManPixmap;
        case BMP_BLACK_KING:    return m_blackKingPixmap;
        case BMP_WHITE_MAN:     return m_whiteManPixmap;
        case BMP_WHITE_KING:    return m_whiteKingPixmap;
        case BMP_LIGHT_SQUARE:  return m_lightSquarePixmap;
        case BMP_DARK_SQUARE:   return m_darkSquarePixmap;
        case BMP_MAN_MASK:      return m_manMaskPixmap;
        case BMP_KING_MASK:     return m_kingMaskPixmap;
        default:
             qWarning() << "getCBPixmap called with invalid type:" << type;
            // Return a default pixmap to avoid crashes, maybe light square?
            return m_lightSquarePixmap;
    }
}

void CheckerBoardWidget::setBoardState(const Board8x8 board, bool invert, bool mirror, int gametype, bool showNumbers)
{
    memcpy(&m_board, &board, sizeof(m_board));
    bool optionsChanged = (m_invertBoard != invert || m_mirrorBoard != mirror || m_gametype != gametype || m_showNumbers != showNumbers);
    m_invertBoard = invert;
    m_mirrorBoard = mirror;
    m_gametype = gametype; // Update gametype
    m_showNumbers = showNumbers; // Update showNumbers
    if (optionsChanged) {
        m_clicks.clear(); // Clear clicks if display options change
    }
    update(); // Trigger repaint
}

void CheckerBoardWidget::clearClicks()
{
    if (m_clicks.size() > 0) {
        m_clicks.clear();
        update(); // Redraw to remove highlights
    }
}

void CheckerBoardWidget::setBoardNumberColor(QRgb color)
{
    m_boardNumberColor = color;
    update(); // Trigger repaint to use the new color
}

void CheckerBoardWidget::setHighlightColor(QRgb color)
{
    m_highlightColor = color;
    update(); // Trigger repaint to use the new color
}


void CheckerBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // Smoother drawing

    calculateBoardLayout(); // Ensure layout is up-to-date

    if (m_squareSize <= 0) {
        painter.fillRect(rect(), Qt::darkGray);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "Resize window");
        return; // Not ready to draw board yet
    }

    // Draw the board squares and pieces
    for (int y = 0; y < 8; ++y) {
        QString rowDebug;
        for (int x = 0; x < 8; ++x) {
            // Determine screen coordinates for this square (after applying display options)
            QPoint topLeft = boardToWidgetCoords(x, y);

            // Determine if it's a light or dark square (based on logical coordinates)
            bool isDark = ((x + y) % 2 != 0);

            // Draw background square
            painter.fillRect(QRect(topLeft, QSize(m_squareSize, m_squareSize)), isDark ? Qt::darkGray : Qt::lightGray);

            // Draw piece if present
            char piece = m_board.board[y][x];
            rowDebug += QString::number(piece) + " ";
            if (piece != CB_EMPTY) {
                PixmapType piecePixmapType;
                switch (piece) {
                    case (CB_BLACK | CB_MAN):  piecePixmapType = BMP_BLACK_MAN; break;
                    case (CB_BLACK | CB_KING): piecePixmapType = BMP_BLACK_KING; break;
                    case (CB_WHITE | CB_MAN):  piecePixmapType = BMP_WHITE_MAN; break;
                    case (CB_WHITE | CB_KING): piecePixmapType = BMP_WHITE_KING; break;
                    default: continue; // Should not happen
                }
                const QPixmap& piecePixmap = getCBPixmap(piecePixmapType);
                if (!piecePixmap.isNull()) {
                    // Scale pixmap if necessary (consider doing this once when piece set loads)
                    QPixmap scaledPixmap = piecePixmap.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    painter.drawPixmap(topLeft, scaledPixmap);
                }
            }

            // Draw board numbers if enabled and on a dark square
            if (m_showNumbers && isDark) {
                painter.setPen(QColor::fromRgb(m_boardNumberColor));
                // Adjust font size dynamically based on square size
                QFont font = painter.font();
                font.setPixelSize(m_squareSize / 4); // Example: 1/4th of square size
                painter.setFont(font);

                // Calculate the logical square number (1-32)
                int logicalX = x;
                int logicalY = y;
                reverseDisplayOptions(logicalX, logicalY); // Get logical coords from display coords
                int squareNumber = coorstonumber(logicalX, logicalY, m_gametype);

                // Draw the number in the corner of the square
                // Adjust position for better centering or corner placement
                QRect textRect(topLeft.x(), topLeft.y(), m_squareSize, m_squareSize);
                painter.drawText(textRect, Qt::AlignBottom | Qt::AlignRight, QString::number(squareNumber));
            }

            // Draw highlights for clicked squares
            if (m_clicks.size() > 0) {
                for (int i = 0; i < m_clicks.size(); ++i) {
                    int clickedSquareNum = m_clicks.at(i);
                    int clickedX, clickedY;
                    numbertocoors(clickedSquareNum, &clickedX, &clickedY, m_gametype);

                    // Apply display options to get the visual coordinates of the clicked square
                    int displayClickedX = clickedX;
                    int displayClickedY = clickedY;
                    applyDisplayOptions(displayClickedX, displayClickedY);

                    if (displayClickedX == x && displayClickedY == y) {
                        painter.setPen(QPen(QColor::fromRgb(m_highlightColor), 3)); // Yellow pen, 3 pixels wide
                        painter.drawRect(topLeft.x(), topLeft.y(), m_squareSize, m_squareSize);
                    }
                }
            }
        }
        qDebug() << "Board row" << y << ":" << rowDebug;
    }
}

void CheckerBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_squareSize <= 0) return; // Board not ready

    if (event->button() == Qt::LeftButton) {
        int boardX, boardY;
        bool onBoard = widgetToBoardCoords(event->pos(), boardX, boardY);

        if (onBoard) {
            // boardX, boardY are logical coordinates (0-7)
            qDebug() << "Left click on logical board coords:" << boardX << "," << boardY;

            

            // In normal play mode, only dark squares are interactive
            if (!is_valid_board8_square(boardX, boardY)) {
                qDebug() << "Clicked on non-playable (light) square.";
                // Clicking on a light square could be used to cancel a move sequence
                if (m_clicks.size() > 0) {
                    m_clicks.clear();
                    update(); // Redraw to remove highlights
                    emit moveSequenceAttempted(m_clicks); // Notify MainWindow that sequence is cancelled
                }
                return; // Ignore clicks on light squares
            }

            // It's a valid dark square, proceed.
            int squareNum = coorstonumber(boardX, boardY, m_gametype);
            qDebug() << "Clicked on playable square number:" << squareNum;

            // Basic click management:
            // This is a simplified version. MainWindow will have the full game logic.
            // The goal here is just to collect a sequence of square numbers.

            // If the square is already in the sequence, maybe the user wants to remove it?
            // For now, we prevent adding the same square more than twice (for king jumps).
            if (m_clicks.count(squareNum) > 0 && m_clicks.count(squareNum) >= 2) {
                qDebug() << "Ignoring third click on same square:" << squareNum;
                return;
            }

            // Add the clicked square to the sequence
            m_clicks.append(squareNum);
            update(); // Redraw to show the new highlight

            // Notify MainWindow about the updated move sequence
            emit moveSequenceAttempted(m_clicks);
        } else {
             qDebug() << "Clicked outside board area.";
             // Optionally clear clicks if clicking outside board cancels selection
             // m_clicks.clear();
             // update();
        }
    }
    
    else if (event->button() == Qt::RightButton) {
         int boardX, boardY;
         bool onBoard = widgetToBoardCoords(event->pos(), boardX, boardY);
         if (onBoard) {
             qDebug() << "Right click on logical board coords:" << boardX << "," << boardY;
             
         }
    }
}

void CheckerBoardWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateBoardLayout(); // Recalculate when widget size changes
    update(); // Trigger repaint with new layout
}

void CheckerBoardWidget::calculateBoardLayout()
{
    // Calculate the largest possible square size that fits
    int w = width();
    int h = height();
    if (w < 8 || h < 8) {
        m_squareSize = 0;
        return;
    }
    m_squareSize = std::min(w / 8, h / 8);

    // Calculate the top-left origin of the board area to center it
    int boardWidth = m_squareSize * 8;
    int boardHeight = m_squareSize * 8;
    m_boardOrigin.setX((w - boardWidth) / 2);
    m_boardOrigin.setY((h - boardHeight) / 2);

    // Optional: Scale pixmaps if they don't match m_squareSize
    // This can be slow if done frequently. Consider doing it only when piece set changes.
    // if (!m_blackManPixmap.isNull() && m_blackManPixmap.width() != m_squareSize) {
    //     m_blackManPixmap = m_blackManPixmap.scaled(m_squareSize, m_squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //     // Scale other pixmaps...
    // }
}


// Converts logical board coordinates (0-7 for x, 0-7 for y) to widget pixel coordinates.
QPoint CheckerBoardWidget::boardToWidgetCoords(int boardX, int boardY) const
{
    // Apply display options first to get the *visual* position
    int displayX = boardX;
    int displayY = boardY;
    applyDisplayOptions(displayX, displayY); // Apply only if input is logical - NO, input IS display coord

    int pixelX = m_boardOrigin.x() + displayX * m_squareSize;
    int pixelY = m_boardOrigin.y() + displayY * m_squareSize;
    return QPoint(pixelX, pixelY);
}

// Converts widget pixel coordinates back to logical board coordinates (0-7 for x, 0-7 for y).
// Returns true if conversion is valid (point is within board bounds).
bool CheckerBoardWidget::widgetToBoardCoords(const QPoint& widgetPos, int& boardX, int& boardY) const
{
    if (m_squareSize <= 0) return false;

    // Get coordinates relative to the board origin
    int relX = widgetPos.x() - m_boardOrigin.x();
    int relY = widgetPos.y() - m_boardOrigin.y();

    // Check if within bounds
    if (relX < 0 || relY < 0 || relX >= m_squareSize * 8 || relY >= m_squareSize * 8) {
        return false;
    }

    // Calculate display coordinates (0-7)
    int displayX = relX / m_squareSize;
    int displayY = relY / m_squareSize;

    // Convert display coordinates back to logical coordinates
    boardX = displayX;
    boardY = displayY;
    reverseDisplayOptions(boardX, boardY);

    return true;
}

// Applies inversion/mirroring logic based on current options
// Modifies x, y IN PLACE. Input/Output are logical coordinates (0-7).
void CheckerBoardWidget::applyDisplayOptions(int& x, int& y) const
{
     // This function takes LOGICAL coords and transforms them to DISPLAY coords
    if (m_invertBoard) {
        x = 7 - x;
        y = 7 - y;
    }
    if (m_mirrorBoard) {
        x = 7 - x;
    }
}

// Reverses inversion/mirroring logic
// Modifies x, y IN PLACE. Input are DISPLAY coords (0-7), output are LOGICAL coords (0-7).
void CheckerBoardWidget::reverseDisplayOptions(int& x, int& y) const
{
    // Apply reverse transformations in opposite order
    if (m_mirrorBoard) {
        x = 7 - x; // Mirror reverses itself
    }
     if (m_invertBoard) {
        x = 7 - x; // Invert reverses itself
        y = 7 - y;
    }
}

