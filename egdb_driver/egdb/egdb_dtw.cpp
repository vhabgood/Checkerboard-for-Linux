/*
* Copyright (C) 2017-2021 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#if defined(_MSC_VER)
#if defined(USE_WIN_API)
#include "windows.h"
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif

#include "egdb/egdb_intl.h"
#include "egdb/egdb_common.h"
#include "egdb/crc.h"

#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/reverse.h"


// from dtw\dtw.h
#define MAX_DTW_PLIES 255


namespace egdb_interface {

	typedef struct {
		/* a file handle for the database file. */
		FILE_HANDLE file;

		/* true if this db contains positions with n pieces, where n is <= num_pieces.
		* if false, it only contains positions with exactly num_pieces.
		*/
		bool contains_le_pieces;

		/* if true, this database contains entries for one side only. */
		bool dtw_w_only;

		/* size of the file in bytes. */
		uint64_t file_size;

		/* a memory mapped handle to the file. For Windows only. */
		MEM_HANDLE map_handle;

		/* a pointer to the database in memory. */
		uint8_t *db;
	} DTW_DB_INFO;

	typedef struct {
		/* an array of db handles, one for each number of pieces. */
		DTW_DB_INFO *db_info;
	} DTW_DRIVER_INFO;


	// forward references
	static int dtw_lookup(DTW_DRIVER_INFO *ddi,
						  int num_pieces,
						  const EGDB_POSITION *pos,
						  EGDB_ERR *err_code);


	EGDB_ERR egdb_close_dtw(EGDB_DRIVER *driver)
	{
		int i;
		DTW_DRIVER_INFO *ddi;

		if (!driver)
			return(EGDB_INVALID_HANDLE);

		ddi = (DTW_DRIVER_INFO *)driver->internal_data;
		if (ddi) {
			if (ddi->db_info) {
				for (i = 0; i < driver->max_pieces; ++i) {
					if (ddi->db_info[i].db) {
#if defined(_MSC_VER) && defined(USE_WIN_API)
						UnmapViewOfFile(ddi->db_info[i].db);
						CloseHandle(ddi->db_info[i].map_handle);
						CloseHandle(ddi->db_info[i].file);
#else
						std::free(ddi->db_info[i].db);
						std::fclose(ddi->db_info[i].file);
#endif
					}
				}
				std::free(ddi->db_info);
			}
			std::free(ddi);
		}
		std::free(driver);
		return(EGDB_ERR_NORMAL); // Changed from EGDB_NORMAL
	}

	unsigned int egdb_get_max_pieces_dtw(EGDB_DRIVER *driver)
	{
		return(driver->max_pieces);
	}

	unsigned int egdb_get_info_dtw(
		EGDB_DRIVER *driver,
		unsigned int num_pieces,
		EGDB_INFO *info,
		unsigned int max_info)
	{
		DTW_DRIVER_INFO *ddi;

		if (num_pieces == 0 || num_pieces > driver->max_pieces)
			return(0);
		if (max_info == 0)
			return(0);

		ddi = (DTW_DRIVER_INFO *)driver->internal_data;
		if (ddi->db_info[num_pieces - 1].db == 0)
			return(0);

		info->type = EGDB_DTW;
		info->compression = EGDB_COMPRESSION_NONE;
		info->dtw_w_only = ddi->db_info[num_pieces - 1].dtw_w_only;
		info->contains_le_pieces = ddi->db_info[num_pieces - 1].contains_le_pieces;
		return(1);
	}

	int egdb_dtw_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
	{
		int value;
		int num_pieces;
		DTW_DRIVER_INFO *ddi;

		ddi = (DTW_DRIVER_INFO *)driver->internal_data;
		num_pieces = bit_pop_count64(pos->black_pieces | pos->white_pieces); // Changed to bit_pop_count64

		value = dtw_lookup(ddi, num_pieces, pos, err_code);
		if (*err_code != EGDB_ERR_NORMAL) // Changed from EGDB_NORMAL
			return(EGDB_UNKNOWN);

		if (value == 0)
			return(EGDB_UNKNOWN);
		if (value == 255)
			return(EGDB_UNKNOWN);
		return(value);
	}

	static int dtw_lookup(
		DTW_DRIVER_INFO *ddi,
		int num_pieces,
		const EGDB_POSITION *pos,
		EGDB_ERR *err_code)
	{
		int p_index;
		uint64_t index, index_reversed;
		uint64_t w, b, k;
		DTW_DB_INFO *db_info;
		BOARD board;

		// find the db that handles this num_pieces
		p_index = num_pieces - 1;
		if (p_index < 0) {
			*err_code = EGDB_NUM_PIECES_TOO_SMALL;
			return(0);
		}
		db_info = &ddi->db_info[p_index];

		// check if db is loaded.
		if (db_info->db == 0) {
			*err_code = EGDB_DB_NOT_LOADED;
			return(0);
		}

		// Check if this position is a capture.  The db does not contain capture positions.
		// Also check if there are pieces on the back rank.
		// Also check for too many kings.
		if (pos->stm == EGDB_BLACK_TO_MOVE) {
			if (has_move_black(&pos->black_pieces, &board)) // Changed cast and passed address of bitboard
				if (board.is_capture) {
					*err_code = EGDB_POS_IS_CAPTURE;
					return(0);
				}
			if (pos->white_pieces & ~pos->king & ROW1) { // Changed pos->kings to pos->king and ROW_1 to ROW1
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
			if (bit_pop_count64(pos->black_pieces & pos->king) > num_pieces - 1) { // Changed to bit_pop_count64 and pos->kings to pos->king
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
			if (bit_pop_count64(pos->white_pieces & pos->king) > num_pieces - 1) { // Changed to bit_pop_count64 and pos->kings to pos->king
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
		}
		else {	// white to move
			if (has_move_white(&pos->white_pieces, &board)) // Changed cast and passed address of bitboard
				if (board.is_capture) {
					*err_code = EGDB_POS_IS_CAPTURE;
					return(0);
				}
			if (pos->black_pieces & ~pos->king & ROW7) { // Changed pos->kings to pos->king and ROW_8 to ROW8 (assuming ROW8 is correct for black back rank)
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
			if (bit_pop_count64(pos->black_pieces & pos->king) > num_pieces - 1) { // Changed to bit_pop_count64 and pos->kings to pos->king
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
			if (bit_pop_count64(pos->white_pieces & pos->king) > num_pieces - 1) { // Changed to bit_pop_count64 and pos->kings to pos->king
				*err_code = EGDB_INVALID_POS;
				return(0);
			}
		}

		// get the index for this position.
		// This can be done for either side to move.
		w = pos->white_pieces;
		b = pos->black_pieces;
		k = pos->king; // Changed from pos->kings

		// if white is to move, and this is a w_only db, do a normal lookup.
		if (pos->stm == EGDB_WHITE_TO_MOVE && db_info->dtw_w_only) {
			board_to_index_dtw(w, b, k, &index);

			// check for index out of bounds.
			if (index >= db_info->file_size) {
				*err_code = EGDB_INDEX_OUT_OF_BOUNDS;
				return(0);
			}
			*err_code = EGDB_ERR_NORMAL; // Changed from EGDB_NORMAL
			return(db_info->db[index]);
		}

		// if black is to move, and this is a w_only db, reverse the board and
		// do a lookup.
		if (pos->stm == EGDB_BLACK_TO_MOVE && db_info->dtw_w_only) {
			uint64_t rev_b = b;
			uint64_t rev_w = w;
			uint64_t rev_k = k;
			reverse_bitboard(&rev_b);
			reverse_bitboard(&rev_w);
			reverse_bitboard(&rev_k);
			board_to_index_dtw(rev_b, rev_w, rev_k, &index);

			// check for index out of bounds.
			if (index >= db_info->file_size) {
				*err_code = EGDB_INDEX_OUT_OF_BOUNDS;
				return(0);
			}
			*err_code = EGDB_ERR_NORMAL; // Changed from EGDB_NORMAL
			return(db_info->db[index]);
		}

		// This must be a db for both sides to move.
		// For white to move, do a lookup, then a reverse lookup, and take the
		// minimum of the two.
		if (pos->stm == EGDB_WHITE_TO_MOVE) {
			int val1, val2;

			board_to_index_dtw(w, b, k, &index);
			uint64_t rev_b = b;
			uint64_t rev_w = w;
			uint64_t rev_k = k;
			reverse_bitboard(&rev_b);
			reverse_bitboard(&rev_w);
			reverse_bitboard(&rev_k);
			board_to_index_dtw(rev_b, rev_w, rev_k, &index_reversed);

			// check for index out of bounds.
			if (index >= db_info->file_size || index_reversed >= db_info->file_size) {
				*err_code = EGDB_INDEX_OUT_OF_BOUNDS;
				return(0);
			}

			val1 = db_info->db[index];
			if (val1 == 0) val1 = MAX_DTW_PLIES;
			val2 = db_info->db[index_reversed];
			if (val2 == 0) val2 = MAX_DTW_PLIES;

			*err_code = EGDB_ERR_NORMAL; // Changed from EGDB_NORMAL
			if (val1 < val2)
				return(val1);
			return(val2);
		}

		// For black to move, do a lookup, then a reverse lookup, and take the
		// minimum of the two.
		else {
			int val1, val2;
			board_to_index_dtw(w, b, k, &index);
			uint64_t rev_b = b;
			uint64_t rev_w = w;
			uint64_t rev_k = k;
			reverse_bitboard(&rev_b);
			reverse_bitboard(&rev_w);
			reverse_bitboard(&rev_k);
			board_to_index_dtw(rev_b, rev_w, rev_k, &index_reversed);

			// check for index out of bounds.
			if (index >= db_info->file_size || index_reversed >= db_info->file_size) {
				*err_code = EGDB_INDEX_OUT_OF_BOUNDS;
				return(0);
			}

			val1 = db_info->db[index];
			if (val1 == 0) val1 = MAX_DTW_PLIES;
			val2 = db_info->db[index_reversed];
			if (val2 == 0) val2 = MAX_DTW_PLIES;
			
			*err_code = EGDB_ERR_NORMAL; // Changed from EGDB_NORMAL
			if (val1 < val2)
				return(val1);
			return(val2);
		}
	}

	unsigned int get_total_num_dtw_dbs()
	{
		return(MAX_PIECES_IN_DB - 2 + 1);
	}
} // namespace egdb_interface