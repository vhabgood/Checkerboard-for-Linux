#ifndef BOARD_INDEXING_H
#define BOARD_INDEXING_H

#include "egdb/egdb.h"
#include "egdb/egdb_common.h"

namespace egdb_interface {

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

} // namespace egdb_interface

#endif // BOARD_INDEXING_H
