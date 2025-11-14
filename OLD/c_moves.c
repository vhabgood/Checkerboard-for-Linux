#include "c_moves.h"
#include "checkers_types.h"
#include <string.h> // For memcpy

// domove_c: Applies a move to the board.
void domove_c(const CBmove *m, Board8x8* board) {
    // Update the board according to the move m.
    int i;

    board->board[m->from.y][m->from.x] = CB_EMPTY;

    // Crowning of a man
    board->board[m->to.y][m->to.x] = m->newpiece;

    // If it is a jump, remove the jumped pieces.
    if (m->jumps > 0) {
        for (i = 0; i < m->jumps; i++) {
            board->board[m->del[i].y][m->del[i].x] = CB_EMPTY;
        }
    }
}

// undomove_c: Undoes a move on the board.
int undomove_c(CBmove *m, Board8x8* board) {
    // Undoes a move on the board.
    int i;

    board->board[m->from.y][m->from.x] = m->oldpiece;

    // Crowning of a man
    board->board[m->to.y][m->to.x] = CB_EMPTY;

    // If it is a jump, remove the jumped pieces.
    if (m->jumps > 0) {
        for (i = 0; i < m->jumps; i++) {
            board->board[m->del[i].y][m->del[i].x] = m->delpiece[i];
        }
    }
    return 1;
}
