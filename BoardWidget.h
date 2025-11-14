#pragma once

#include <QWidget>
#include <QPixmap>
#include <QMap>
#include "checkers_types.h"

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);
    ~BoardWidget();

    void setBoard(const Board8x8& board);
    void setInverted(bool inverted);
    void setShowCoordinates(bool show);
    void setSetupPieceType(int pieceType);
    void setTogglePieceColorMode(bool toggle);
    void setPieceSet(const QString& pieceSet);
    void setMirror(bool mirrored);
    void setHighlight(bool highlight);

    QColor getCoordinateColor() const { return m_coordinateColor; }
    void setCoordinateColor(const QColor& color) { m_coordinateColor = color; update(); }

    QColor getHighlightColor() const { return m_highlightColor; }
    void setHighlightColor(const QColor& color) { m_highlightColor = color; update(); }

    int getSetupPieceType() const { return m_currentSetupPieceType; }
    bool isTogglePieceColorMode() const { return m_isTogglePieceColorMode; }

public slots:
    void setSelectedPiece(int x, int y);
    void clearSelectedPiece();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Board8x8 m_board;
    QMap<int, QPixmap> m_piecePixmaps;
    int m_squareSize;
    bool m_inverted; // New member to store inversion state
    bool m_showCoordinates; // New member to store coordinate display state
    QColor m_coordinateColor; // Color for board coordinates
    QColor m_highlightColor;  // Color for highlighting moves/pieces
    int m_selectedX; // X-coordinate of the selected piece
    int m_selectedY; // Y-coordinate of the selected piece
    bool m_pieceSelected; // Flag to indicate if a piece is selected
    int m_currentSetupPieceType; // Stores the currently selected piece type for setup mode
    bool m_isTogglePieceColorMode; // New member to track if in toggle piece color mode
    QString m_pieceSet; // New member to store the current piece set
    bool m_mirrored; // New member to store mirroring state
    bool m_highlight; // New member to store highlight state

    void loadPiecePixmaps();
    QPoint boardToScreen(int x, int y) const;
    QPoint screenToBoard(const QPoint& screenPos) const;

signals:
    void squareClicked(int x, int y);
};
