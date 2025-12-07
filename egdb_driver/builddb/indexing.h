#ifndef INDEXING_H
#define INDEXING_H

#include "egdb/egdb_common.h"
#include <cstdint>


namespace egdb_interface {

BITBOARD index_pieces_1_type(BITBOARD pieces, unsigned int num_pieces, BITBOARD occupied_mask);
BITBOARD index_pieces_1_type_reverse(BITBOARD pieces, unsigned int num_pieces, BITBOARD occupied_mask);

void build_man_index_base();
void indextoposition_slice(int64_t index, EGDB_POSITION *p, int bm, int bk, int wm, int wk);
int64_t position_to_index_slice(EGDB_POSITION const *p, int bm, int bk, int wm, int wk);
int64_t mirror_position_to_index_slice(EGDB_POSITION const *p, int bm, int bk, int wm, int wk);
int64_t getdatabasesize_slice(int bm, int bk, int wm, int wk);
int64_t getslicesize_gaps(int bm, int bk, int wm, int wk);
int get_subdb_index(int num_pieces, unsigned int bm, unsigned int bk, unsigned int wm, unsigned int wk); // Added this

BITBOARD free_square_bitboard_fwd(int logical_square, BITBOARD occupied);
BITBOARD free_square_bitboard_rev(int logical_square, BITBOARD occupied);
BITBOARD index2bitboard_fwd(unsigned int index, int num_squares, int first_square, int num_pieces);
BITBOARD index2bitboard_fwd(unsigned int index, int num_squares, int num_pieces, BITBOARD occupied);
BITBOARD index2bitboard_rev(unsigned int index, int num_squares, int num_pieces, BITBOARD occupied);

}	// namespace egdb_interface

#endif