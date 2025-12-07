#ifndef BOARD_H
#define BOARD_H

#include "egdb/egdb.h"
#include "egdb/egdb_common.h"
#include "engine/project.h"


// board squares.
#define SQ1 0x00000001ULL
#define SQ2 0x00000002ULL
#define SQ3 0x00000004ULL
#define SQ4 0x00000008ULL
#define SQ5 0x00000010ULL
#define SQ6 0x00000020ULL
#define SQ7 0x00000040ULL
#define SQ8 0x00000080ULL
#define SQ9 0x00000100ULL
#define SQ10 0x00000200ULL
#define SQ11 0x00000400ULL
#define SQ12 0x00000800ULL
#define SQ13 0x00001000ULL
#define SQ14 0x00002000ULL
#define SQ15 0x00004000ULL
#define SQ16 0x00008000ULL
#define SQ17 0x00010000ULL
#define SQ18 0x00020000ULL
#define SQ19 0x00040000ULL
#define SQ20 0x00080000ULL
#define SQ21 0x00100000ULL
#define SQ22 0x00200000ULL
#define SQ23 0x00400000ULL
#define SQ24 0x00800000ULL
#define SQ25 0x01000000ULL
#define SQ26 0x02000000ULL
#define SQ27 0x04000000ULL
#define SQ28 0x08000000ULL
#define SQ29 0x10000000ULL
#define SQ30 0x20000000ULL
#define SQ31 0x40000000ULL
#define SQ32 0x80000000ULL


// rows in bitboard.
#define ROW0 (SQ1 | SQ2 | SQ3 | SQ4)
#define ROW1 (SQ5 | SQ6 | SQ7 | SQ8)
#define ROW2 (SQ9 | SQ10 | SQ11 | SQ12)
#define ROW3 (SQ13 | SQ14 | SQ15 | SQ16)
#define ROW4 (SQ17 | SQ18 | SQ19 | SQ20)
#define ROW5 (SQ21 | SQ22 | SQ23 | SQ24)
#define ROW6 (SQ25 | SQ26 | SQ27 | SQ28)
#define ROW7 (SQ29 | SQ30 | SQ31 | SQ32)


// columns in bitboard.
#define COL0 (SQ4 | SQ8 | SQ12 | SQ16 | SQ20 | SQ24 | SQ28 | SQ32)
#define COL1 (SQ3 | SQ7 | SQ11 | SQ15 | SQ19 | SQ23 | SQ27 | SQ31)
#define COL2 (SQ2 | SQ6 | SQ10 | SQ14 | SQ18 | SQ22 | SQ26 | SQ30)
#define COL3 (SQ1 | SQ5 | SQ9 | SQ13 | SQ17 | SQ21 | SQ25 | SQ29)


// other masks.
#define EDGE_SQUARES (ROW0 | ROW7 | COL0 | COL3)
#define GHOSTS 0x80000000ULL

#define ALL_SQUARES 0xFFFFFFFF


namespace egdb_interface {

	typedef struct {
		bool is_capture;
		int move_type;
	} BOARD;


	extern unsigned int square_to_bitboard[NUM_SQUARES];
    extern BITBOARD square0_to_bitboard_table[NUMSQUARES];

	void init_board(void);
	bool has_move_black(const uint64_t *board, BOARD *b);
	bool has_move_white(const uint64_t *board, BOARD *b);
	uint64_t get_board_index(const EGDB_POSITION *pos,
							 uint64_t *index,
							 unsigned int *bm,
							 unsigned int *bk,
							 unsigned int *wm,
							 unsigned int *wk);
	uint64_t get_board_index_mtc(const EGDB_POSITION *pos,
								 uint64_t *index,
								 unsigned int *bm,
								 unsigned int *bk,
								 unsigned int *wm,
								 unsigned int *wk);
	uint64_t board_to_index_dtw(uint64_t w, uint64_t b, uint64_t k, uint64_t *index);
}	// namespace egdb_interface

#endif
