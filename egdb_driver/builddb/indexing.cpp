#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "builddb/indexing.h"
#include "engine/bicoef.h"
#include "engine/board.h"
#include "engine/bitcount.h"
#include "engine/reverse.h"
#include "engine/bool.h"
#include "log.h"

namespace egdb_interface {

#define RANK0MAX 4
#define ROWSIZE 4
#define NUMSQUARES 32

static bool did_build_man_index_base = false;

uint32_t index_pieces_1_type(BITBOARD pieces, int num_pieces, BITBOARD occupied)
{
	int piece, logical_square;
	uint32_t index;

	index = 0;
	for (piece = 1; pieces; ++piece) {
		logical_square = lsb64_(pieces);
		pieces ^= ((BITBOARD)1 << logical_square);
		logical_square -= bit_pop_count64(occupied & (((BITBOARD)1 << logical_square) - 1));
		index += (uint32_t)choose(logical_square, piece);
	}
	return(index);
}


uint32_t index_pieces_1_type_reverse(BITBOARD pieces, int num_pieces, BITBOARD occupied)
{
	int piece, logical_square;
	uint32_t index;

	index = 0;
	for (piece = 1; pieces; ++piece) {
		logical_square = msb64_(pieces);
		pieces ^= ((BITBOARD)1 << logical_square);
		logical_square = (NUMSQUARES - 1) - logical_square;
		logical_square -= bit_pop_count64(occupied & ~(((BITBOARD)1 << ((NUMSQUARES - 1) - logical_square)) - 1));
		index += (uint32_t)choose(logical_square, piece);
	}
	return(index);
}

int64_t man_index_base[MAXPIECE + 1][MAXPIECE + 1][RANK0MAX + 1];

void build_man_index_base(void)
{
	int bm, wm, bm0;
	int64_t size;

	if (did_build_man_index_base)
		return;

	for (bm = 0; bm <= MAXPIECE; ++bm) {
		for (wm = 0; wm <= MAXPIECE; ++wm) {
			if (bm + wm > MAXPIECE)
				continue;
			size = 0;
			for (bm0 = (std::min)(bm, ROWSIZE); bm0 >= 0; --bm0) {
				man_index_base[bm][wm][bm0] = size;
				size += (int64_t)choose(NUMSQUARES - 2 * ROWSIZE, bm - bm0) * (int64_t)choose(ROWSIZE, bm0) * (int64_t)choose(NUMSQUARES - ROWSIZE - (bm - bm0), wm);
			}
		}
	}
	did_build_man_index_base = true;
}

int64_t position_to_index_slice(EGDB_POSITION const *p, int bm, int bk, int wm, int wk)
{
    BITBOARD bm0_mask;
    BITBOARD bmmask, bkmask, wmmask, wkmask;
    int bm0;
    uint32_t bmindex, bkindex, wmindex, wkindex, bm0index;
    uint32_t bmrange, bm0range, bkrange, wkrange;
    int64_t index64;
    int64_t checker_index_base, checker_index;

    bmmask = p->black_pieces & ~p->king;
    bm0_mask = bmmask & ROW0;
    bm0 = bit_pop_count64(bm0_mask);

    checker_index_base = man_index_base[bm][wm][bm0];

    bmindex = index_pieces_1_type(bmmask ^ bm0_mask, bm - bm0, 0);
    bm0index = index_pieces_1_type(bm0_mask, bm0, 0);

    wmmask = p->white_pieces & ~p->king;
    wmindex = index_pieces_1_type_reverse(wmmask, wm, bmmask);

    bkmask = p->black_pieces & p->king;
    bkindex = index_pieces_1_type(bkmask, bk, bmmask | wmmask);

    wkmask = p->white_pieces & p->king;
    wkindex = index_pieces_1_type(wkmask, wk, bmmask | wmmask | bkmask);

    bmrange = choose(NUMSQUARES - 2 * ROWSIZE, bm - bm0);
    bm0range = choose(ROWSIZE, bm0);
    bkrange = choose(NUMSQUARES - bm - wm, bk);
    wkrange = choose(NUMSQUARES - bm - wm - bk, wk);

    checker_index = bm0index + checker_index_base +
                    (int64_t)bmindex * bm0range +
                    (int64_t)wmindex * (int64_t)bm0range * (int64_t)bmrange;

    index64 = (int64_t)wkindex + 
              (int64_t)bkindex * (int64_t)wkrange +
              (int64_t)checker_index * (int64_t)bkrange * (int64_t)wkrange;

    return index64;
}

int64_t mirror_position_to_index_slice(EGDB_POSITION const *p, int bm, int bk, int wm, int wk)
{
	BITBOARD wm0_mask;
	BITBOARD bmmask, bkmask, wmmask, wkmask;
	int wm0;
	uint32_t bmindex, bkindex, wmindex, wkindex, wm0index;
	uint32_t wmrange, wm0range, bkrange, wkrange;
	int64_t index64;
	int64_t checker_index_base, checker_index;

	wmmask = p->white_pieces & ~p->king;
	wm0_mask = wmmask & ROW7;
	wm0 = bit_pop_count64(wm0_mask);

	checker_index_base = man_index_base[wm][bm][wm0];

	wmindex = index_pieces_1_type_reverse(wmmask ^ wm0_mask, wm - wm0, 0);
	wm0index = index_pieces_1_type_reverse(wm0_mask, wm0, 0);

	bmmask = p->black_pieces & ~p->king;
	bmindex = index_pieces_1_type(bmmask, bm, wmmask);

	wkmask = p->white_pieces & p->king;
	wkindex = index_pieces_1_type_reverse(wkmask, wk, bmmask | wmmask);

	bkmask = p->black_pieces & p->king;
	bkindex = index_pieces_1_type_reverse(bkmask, bk, bmmask | wmmask | wkmask);

	wmrange = choose(NUMSQUARES - 2 * ROWSIZE, wm - wm0);
	wm0range = choose(ROWSIZE, wm0);
	wkrange = choose(NUMSQUARES - bm - wm, wk);
	bkrange = choose(NUMSQUARES - bm - wm - wk, bk);

	checker_index = wm0index + checker_index_base +
					wmindex * wm0range +
					(int64_t)bmindex * (int64_t)wm0range * (int64_t)wmrange;

	index64 = (int64_t)bkindex + 
				(int64_t)wkindex * (int64_t)bkrange +
				(int64_t)(checker_index) * (int64_t)wkrange * (int64_t)bkrange;
	return(index64);
}

} // namespace egdb_interface