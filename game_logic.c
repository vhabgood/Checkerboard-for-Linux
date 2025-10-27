#include "checkers_types.h"
#include "CBconsts.h"

// Initialize the global board state with a standard checkers starting position
Board8x8 cbboard8 = {
    {EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN},
    {BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY},
    {EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY},
    {EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN},
    {WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY}
};

void newgame(void)
{
    // Reset the board to the initial state
    Board8x8 initialBoard = {
        {EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN},
        {BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY},
        {EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN, EMPTY, BLACK_MAN},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY},
        {EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN},
        {WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY, WHITE_MAN, EMPTY}
    };
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            cbboard8[r][c] = initialBoard[r][c];
        }
    }
}
