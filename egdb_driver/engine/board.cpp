#include "engine/board.h"
#include "engine/bool.h"

namespace egdb_interface {

// For an 8x8 board, the mapping from square index (0-31) to bit number (0-31) is direct.
char square0_to_bitnum_table[NUMSQUARES] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

BITBOARD square0_to_bitboard_table[NUMSQUARES] = {
	SQ1,   SQ2,  SQ3,  SQ4,  SQ5,  SQ6,  SQ7,  SQ8,  SQ9, SQ10,
	SQ11, SQ12, SQ13, SQ14, SQ15, SQ16, SQ17, SQ18, SQ19, SQ20,
	SQ21, SQ22, SQ23, SQ24, SQ25, SQ26, SQ27, SQ28, SQ29, SQ30,
	SQ31, SQ32
};

// For an 8x8 board, bit numbers are 0-31.
char bitnum_to_square0_table[NUM_BITBOARD_BITS] = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

// For an 8x8 board with 4 squares per rank.
int rank_to_rank0_shift_table[] = {
	0, 4, 8, 12, 16, 20, 24, 28
};

void init_board(void) {
    // Tables are statically initialized, so nothing to do here for now.
    // This function exists to match the declaration in board.h and
    // calls from egdb_open.cpp.
}

bool has_move_black(const uint64_t *board, BOARD *b) {
    // Placeholder: return false (no capture) for now to allow WLD lookups.
    // In a full implementation, this would detect captures for side-to-move.
    return false;
}

bool has_move_white(const uint64_t *board, BOARD *b) {
    return false;
}

int get_subdb_index(int num_pieces, unsigned int bm, unsigned int bk, unsigned int wm, unsigned int wk) {
    // Placeholder: return 0 for now. This is used by MTC logic to index into sub-databases.
    return 0;
}

}	// namespace egdb_interface