#pragma once

#include <QWidget>
#include "checkers_types.h"

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);
    ~BoardWidget(); // Add destructor declaration
    void setBoard(const bitboard_pos& board);
    void updatePiece(int square, int piece);
    void clearHighlights();
    void highlightSquare(int square, const QColor& color);

    // New functions for piece selection
    void setSelectedPiece(int x, int y);
    void clearSelectedPiece();

    // New setters for configuration and display
    void setHighlight(bool highlight);
    void setInverted(bool inverted);
    void setShowCoordinates(bool show);
    void setMirror(bool mirrored);

    // New setters for setup mode
    void setSetupPieceType(int pieceType);
    void setTogglePieceColorMode(bool toggle);

    // New setter for piece set
    void setPieceSet(const QString& pieceSet);

signals:
    void squareClicked(int x, int y); // Modified to emit x and y coordinates

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void loadPiecePixmaps();
    QPoint boardToScreen(int x, int y) const;
    QPoint screenToBoard(const QPoint& screenPos) const;
    // Board state
    bitboard_pos m_board;
    QVector<QColor> m_highlights; // For highlighting squares

    // Configuration and display
    int m_squareSize;
    bool m_inverted;
    bool m_showCoordinates;
    bool m_mirrored;
    bool m_highlight; // Global highlight toggle

    QColor m_coordinateColor;
    QColor m_highlightColor;

    // Piece drawing
    QMap<int, QPixmap> m_piecePixmaps; // Stores pixmaps for different piece types
    QString m_pieceSet; // Current piece set being used (e.g., "standard", "marble")

    // Selection and setup mode
    int m_selectedX;
    int m_selectedY;
    bool m_pieceSelected;
    int m_currentSetupPieceType; // Used in setup mode
    bool m_isTogglePieceColorMode; // Used in setup mode
};