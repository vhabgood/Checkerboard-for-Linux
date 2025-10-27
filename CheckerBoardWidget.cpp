#include "CheckerBoardWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDir>
#include <QDebug>

CheckerBoardWidget::CheckerBoardWidget(QWidget *parent) :
    QWidget(parent)
{
    // We will set the proper size policies and mouse tracking later
}

CheckerBoardWidget::~CheckerBoardWidget()
{
}

void CheckerBoardWidget::loadPieceBitmaps(const QString &pieceSetDir)
{
    // Replaces initbmp()
    // Use QDir::filePath to correctly join paths (e.g., "bmp/default" + "bm.bmp")
    
    QString manMaskPath = QDir::filePath(pieceSetDir, "manmask.bmp");
    if (!m_manMaskPixmap.load(manMaskPath)) {
        qWarning() << "Failed to load image:" << manMaskPath;
    }

    QString kingMaskPath = QDir::filePath(pieceSetDir, "kingmask.bmp");
    if (!m_kingMaskPixmap.load(kingMaskPath)) {
        qWarning() << "Failed to load image:" << kingMaskPath;
    }

    QString bmPath = QDir::filePath(pieceSetDir, "bm.bmp");
    if (!m_blackManPixmap.load(bmPath)) {
        qWarning() << "Failed to load image:" << bmPath;
    }

    QString bkPath = QDir::filePath(pieceSetDir, "bk.bmp");
    if (!m_blackKingPixmap.load(bkPath)) {
        qWarning() << "Failed to load image:" << bkPath;
    }

    QString wmPath = QDir::filePath(pieceSetDir, "wm.bmp");
    if (!m_whiteManPixmap.load(wmPath)) {
        qWarning() << "Failed to load image:" << wmPath;
    }

    QString wkPath = QDir::filePath(pieceSetDir, "wk.bmp");
    if (!m_whiteKingPixmap.load(wkPath)) {
        qWarning() << "Failed to load image:" << wkPath;
    }

    QString lightPath = QDir::filePath(pieceSetDir, "light.bmp");
    if (!m_lightSquarePixmap.load(lightPath)) {
        qWarning() << "Failed to load image:" << lightPath;
    }

    QString darkPath = QDir::filePath(pieceSetDir, "dark.bmp");
    if (!m_darkSquarePixmap.load(darkPath)) {
        qWarning() << "Failed to load image:" << darkPath;
    }

    // Force a repaint now that we have new images
    update();
}

const QPixmap& CheckerBoardWidget::getCBPixmap(PixmapType type) const
{
    // Replaces getCBbitmap()
    switch (type) {
        case BMP_BLACK_MAN:
            return m_blackManPixmap;
        case BMP_BLACK_KING:
            return m_blackKingPixmap;
        case BMP_WHITE_MAN:
            return m_whiteManPixmap;
        case BMP_WHITE_KING:
            return m_whiteKingPixmap;
        case BMP_LIGHT_SQUARE:
            return m_lightSquarePixmap;
        case BMP_DARK_SQUARE:
            return m_darkSquarePixmap;
        case BMP_MAN_MASK:
            return m_manMaskPixmap;
        case BMP_KING_MASK:
            return m_kingMaskPixmap;
        default:
            // Return a default pixmap to avoid crashes
            return m_lightSquarePixmap;
    }
}

void CheckerBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // This is where all drawing logic from graphics.c will go.
    // For example, to draw the light square at top-left:
    // painter.drawPixmap(0, 0, m_lightSquarePixmap);
    
    // Placeholder: just fill the background
    painter.fillRect(rect(), Qt::gray);
}

void CheckerBoardWidget::mousePressEvent(QMouseEvent *event)
{
    // This is where click handling logic from CheckerBoard.c (handle_lbuttondown, etc.) will go.
    // We can get coordinates like this:
    // QPoint clickPos = event->pos();
    
    // We'll implement the logic to convert clickPos to board coordinates later.
}

