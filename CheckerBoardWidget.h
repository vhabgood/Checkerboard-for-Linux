#ifndef CHECKERBOARDWIDGET_H
#define CHECKERBOARDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QString>

// Define the bitmap types, replacing the old bmp.h defines
enum PixmapType {
    BMP_BLACK_MAN,
    BMP_BLACK_KING,
    BMP_WHITE_MAN,
    BMP_WHITE_KING,
    BMP_LIGHT_SQUARE,
    BMP_DARK_SQUARE,
    BMP_MAN_MASK,
    BMP_KING_MASK
};

class CheckerBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CheckerBoardWidget(QWidget *parent = nullptr);
    ~CheckerBoardWidget();

    // Replaces initbmp() - Loads all pixmaps from a specified piece set directory
    void loadPieceBitmaps(const QString &pieceSetDir);

    // Replaces getCBbitmap() - Returns a const reference to a loaded pixmap
    const QPixmap& getCBPixmap(PixmapType type) const;

protected:
    // We will need these to draw the board and handle clicks
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Member variables to hold our loaded images
    QPixmap m_blackManPixmap;
    QPixmap m_blackKingPixmap;
    QPixmap m_whiteManPixmap;
    QPixmap m_whiteKingPixmap;
    QPixmap m_lightSquarePixmap;
    QPixmap m_darkSquarePixmap;
    QPixmap m_manMaskPixmap;
    QPixmap m_kingMaskPixmap;
};

#endif // CHECKERBOARDWIDGET_H

