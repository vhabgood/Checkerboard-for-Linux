// This file contains functions for reversing bitboards.
#pragma once

#include <cstdint>
#include "egdb/egdb_types.h"
#include "log.h"
#include "engine/project.h"

namespace egdb_interface {

	extern uint8_t ReverseByte[256];

	void init_reverse(void);

	static inline int mirrored(int square) {
		return 31 - square;
	}

	// reverses a bitboard for the 32-square American checkers board.
	inline void reverse_bitboard(uint64_t *board)
	{
		// log_c(LOG_LEVEL_DEBUG, "reverse_bitboard: input=0x%llx", (unsigned long long)*board);
		uint64_t original_board = *board;
		*board = 0; // Clear the board to build the reversed one

		for (int i = 0; i < NUMSQUARES; ++i) { // NUMSQUARES is 32 for American Checkers
			if ((original_board >> i) & 1) { // If there's a piece at square 'i' (0-indexed)
				*board |= (1ULL << mirrored(i)); // Place it at the mirrored square
			}
		}
		// log_c(LOG_LEVEL_DEBUG, "reverse_bitboard: output=0x%llx", (unsigned long long)*board);
	}

	inline void reverse(EGDB_POSITION *dest, const EGDB_POSITION *src)
	{
		uint64_t temp;
		temp = src->black_pieces;
		dest->white_pieces = temp;
		temp = src->white_pieces;
		dest->black_pieces = temp;
		temp = src->king;
		dest->king = temp;
		dest->stm = src->stm;
		reverse_bitboard(&dest->black_pieces);
		reverse_bitboard(&dest->white_pieces);
		reverse_bitboard(&dest->king);
	}


}	// namespace egdb_interface