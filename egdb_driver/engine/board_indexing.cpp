#include "board_indexing.h"
#include "log.h"
#include "egdb/egdb_intl.h"
#include "engine/bicoef.h"
#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/bool.h"
#include "engine/project.h"
#include "builddb/indexing.h" // For index_pieces_1_type and index_pieces_1_type_reverse
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace egdb_interface {

extern int64_t man_index_base[MAXPIECE + 1][MAXPIECE + 1][RANK0MAX + 1];
extern bool did_build_man_index_base;

uint64_t calculate_lsb_index(uint32_t pieces, uint32_t occupied_mask) {
    uint64_t index = 0;
    uint32_t y = pieces;
    int32_t x;
    int i = 1;
    while (y) {
        x = lsb_(y);
        y ^= (1 << x);
        if (occupied_mask) {
            x -= bit_pop_count(occupied_mask & ((1 << x) - 1));
        }
		if (x < 0) {
			// This indicates a logic error or corrupt board state, but we
			// guard here to prevent a crash from out-of-bounds access in choose().
			continue;
		}
        index += choose(x, i);
        i++;
    }
    return index;
}

uint64_t calculate_msb_index(uint32_t pieces) {
    uint32_t index = 0;
    uint32_t y = pieces;
    uint32_t x;
    int i = 1;
    while (y) {
        x = msb_(y);
        y ^= (1 << x);
        x = 31 - x;
        index += choose(x, i);
        i++;
    }
    return index;
}

uint64_t get_board_index(const EGDB_POSITION *pos,
                         uint64_t *index,
                         unsigned int *bm,
                         unsigned int *bk,
                         unsigned int *wm,
                         unsigned int *wk)
{
    uint32_t p_bm = (uint32_t)pos->black_pieces & ~(uint32_t)pos->king;
    uint32_t p_bk = (uint32_t)pos->black_pieces & (uint32_t)pos->king;
    uint32_t p_wm = (uint32_t)pos->white_pieces & ~(uint32_t)pos->king;
    uint32_t p_wk = (uint32_t)pos->white_pieces & (uint32_t)pos->king;

    *bm = bit_pop_count(p_bm);
    *bk = bit_pop_count(p_bk);
    *wm = bit_pop_count(p_wm);
    *wk = bit_pop_count(p_wk);

    int bmrank = 0;
    int wmrank = 0;

    if (p_bm)
        bmrank = msb_(p_bm) / 4;
    if (p_wm)
        wmrank = (31 - lsb_(p_wm)) / 4;

    uint32_t bmindex = calculate_lsb_index(p_bm, 0);
    uint32_t wmindex = calculate_msb_index(p_wm);
    uint32_t bkindex = calculate_lsb_index(p_bk, p_bm | p_wm);
    uint32_t wkindex = calculate_lsb_index(p_wk, p_bm | p_bk | p_wm);

    uint32_t bmrange = 1, wmrange = 1, bkrange = 1, wkrange = 1;

    if (!did_initbicoef) {
        initbicoef();
    }

    if (*bm)
        bmrange = choose(4 * (bmrank + 1), *bm) - choose(4 * bmrank, *bm);
    if (*wm)
        wmrange = choose(4 * (wmrank + 1), *wm) - choose(4 * wmrank, *wm);
    if (*bk)
        bkrange = choose(32 - *bm - *wm, *bk);
    if (*wk)
        wkrange = choose(32 - *bm - *wm - *bk, *wk);

    if (bmrank)
        bmindex -= choose(4 * bmrank, *bm);
    if (wmrank)
        wmindex -= choose(4 * wmrank, *wm);

    // WLD piece order (most to least significant): wm, bm, bk, wk
    *index = (uint64_t)wkindex + 
             (uint64_t)bkindex * wkrange + 
             (uint64_t)bmindex * wkrange * bkrange + 
             (uint64_t)wmindex * wkrange * bkrange * bmrange;
    return *index;
}

uint64_t get_board_index_mtc(const EGDB_POSITION *pos,
                             uint64_t *index,
                             unsigned int *bm,
                             unsigned int *bk,
                             unsigned int *wm,
                             unsigned int *wk)
{
    log_c(LOG_LEVEL_DEBUG, "get_board_index_mtc: black=0x%llx, white=0x%llx, king=0x%llx", (unsigned long long)pos->black_pieces, (unsigned long long)pos->white_pieces, (unsigned long long)pos->king);
    uint32_t p_bm = (uint32_t)pos->black_pieces & ~(uint32_t)pos->king;
    uint32_t p_bk = (uint32_t)pos->black_pieces & (uint32_t)pos->king;
    uint32_t p_wm = (uint32_t)pos->white_pieces & ~(uint32_t)pos->king;
    uint32_t p_wk = (uint32_t)pos->white_pieces & (uint32_t)pos->king;

    *bm = bit_pop_count(p_bm);
    *bk = bit_pop_count(p_bk);
    *wm = bit_pop_count(p_wm);
    *wk = bit_pop_count(p_wk);

    int bmrank = 0;
    int wmrank = 0;

    if (p_bm)
        bmrank = msb_(p_bm) / 4;
    if (p_wm)
        wmrank = (31 - lsb_(p_wm)) / 4;

    uint32_t bmindex = calculate_lsb_index(p_bm, 0);
    uint32_t wmindex = calculate_lsb_index(p_wm, 0);
    uint32_t bkindex = calculate_lsb_index(p_bk, p_bm | p_wm);
    uint32_t wkindex = calculate_lsb_index(p_wk, p_bm | p_bk | p_wm);

    uint32_t bmrange = 1, wmrange = 1, bkrange = 1, wkrange = 1;

    if (!did_initbicoef) {
        initbicoef();
    }

    if (*bm)
        bmrange = choose(4 * (bmrank + 1), *bm) - choose(4 * bmrank, *bm);
    if (*wm)
        wmrange = choose(4 * (wmrank + 1), *wm) - choose(4 * wmrank, *wm);
    if (*bk)
        bkrange = choose(32 - *bm - *wm, *bk);
    if (*wk)
        wkrange = choose(32 - *bm - *wm - *bk, *wk);

    if (bmrank)
        bmindex -= choose(4 * bmrank, *bm);
    if (wmrank)
        wmindex -= choose(4 * wmrank, *wm);

    // MTC piece order (most to least significant): wm, wk, bm, bk
    *index = (uint64_t)bkindex + 
             (uint64_t)bmindex * bkrange + 
             (uint64_t)wkindex * bkrange * bmrange + 
             (uint64_t)wmindex * bkrange * bmrange * wkrange;
    
    return *index;
}

uint64_t board_to_index_dtw(uint64_t w, uint64_t b, uint64_t k, uint64_t *index)
{
    // This is a simplified version for DTW which only uses total pieces, not men/kings explicitly.
    // Needs to be implemented based on DTW specific indexing if different from WLD.
    // For now, return a dummy value.
    *index = 0; // Placeholder
    return 0;
}

} // namespace egdb_interface
