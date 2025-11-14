#ifndef CHECKERBOARDWIDGET_H
#define CHECKERBOARDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QPoint>
#include "checkers_types.h" // For Board8x8, Squarelist, coorstonumber
#include "coordinates.h"    // For coorstonumber, numbertocoors (if needed later)
#include "CBconsts.h"       // For GT_ENGLISH and other constants


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

public slots:
    // Slot to update the board state and display options from MainWindow
    void setBoardState(const Board8x8 board, bool invert, bool mirror, int gametype, bool showNumbers);
    // Slot to clear the current click sequence (called by MainWindow after move processed)
    void clearClicks();
    // Slot to set the color for board numbers
    void setBoardNumberColor(QRgb color);
    // Slot to set the color for highlights
    void setHighlightColor(QRgb color);

signals:
    // Signal emitted when the user completes a click sequence that might be a move
    void moveSequenceAttempted(const Squarelist& clicks);

protected:
    // We will need these to draw the board and handle clicks
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override; // To recalculate square size

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

    // Board state and display options
    Board8x8 m_board; // Current board state
    bool m_invertBoard = false;
    bool m_mirrorBoard = false;
    bool m_showNumbers = true; // Control visibility of board numbers
    int m_gametype = ENGLISH; // Store current gametype
    int m_squareSize = 0;       // Size of each square in pixels
    QPoint m_boardOrigin;       // Top-left corner of the board area within the widget
    QRgb m_boardNumberColor; // Color for board numbers
    QRgb m_highlightColor;   // Color for highlights

    Squarelist m_clicks; // Stores the sequence of clicked squares for move input

    // Helper functions
    void calculateBoardLayout(); // Calculate m_squareSize and m_boardOrigin
    // Converts logical board coordinates (0-7, 0-7) potentially applying invert/mirror
    QPoint boardToWidgetCoords(int boardX, int boardY) const;
    // Converts widget pixel coordinates back to logical board coordinates (0-7, 0-7)
    // Returns true if conversion is valid and sets boardX, boardY
    bool widgetToBoardCoords(const QPoint& widgetPos, int& boardX, int& boardY) const;
    // Applies inversion/mirroring logic to logical board coordinates
    void applyDisplayOptions(int& x, int& y) const;
    // Reverses inversion/mirroring logic from logical board coords to display coords
    void reverseDisplayOptions(int& x, int& y) const;


};

#endif // CHECKERBOARDWIDGET_H

