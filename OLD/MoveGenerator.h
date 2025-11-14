#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "checkers_types.h"
#include "c_logic.h"

class MoveGenerator
{
public:
    MoveGenerator();

    // Generates a list of legal moves for the given position and color
    void generateMoves(pos *p, int color, CBmove M[MAXMOVES], int *n, bool onlyCaptures = false);
    bool isLegalMove(const Board8x8 board, int color, int fromX, int fromY, int toX, int toY, CBmove *move, int gametype);

private:
    // Internal helper functions, moved from CB_movegen.c
    int makemovelist(int color, CBmove movelist[MAXMOVES], int b[12][12], int *isjump, int *n);
    void board8toboard12(Board8x8 board, int board12[12][12]);
    void whitecapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void blackcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void whitekingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void blackkingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    inline int cbcolor_to_getmovelistcolor(int cbcolor);
};

#endif // MOVEGENERATOR_H
