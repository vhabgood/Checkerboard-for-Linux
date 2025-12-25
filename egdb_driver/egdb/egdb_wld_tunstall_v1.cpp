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
#include <string>
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
#include "core_types.h"
#include "engine/bicoef.h"
#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/reverse.h"
#include "engine/bool.h"
#include "builddb/tunstall_decompress.h"
#include "builddb/indexing.h"
#include "db_stats.h"


// for the lru cache for the database.
#define TAIL 0
#define HEAD (MAXCACHEDBLOCKS - 1)


namespace egdb_interface {

	// forward references
	static unsigned int parseindexfile(DBHANDLE h, char const *idx_fname, int file_num);
	static int dblookup(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err);
	static DBHANDLE initdblookup(const char *egtb_dir, unsigned int cache_size_bytes);
	static void exitdblookup(DBHANDLE h);
	unsigned int get_total_num_wld_dbs();
    int get_db_data_block(DBHANDLE h, int file_num, uint64_t offset, int num_dbs, uint8_t **data_block, unsigned int *len_bytes);
    void get_board_index(const EGDB_POSITION *pos, uint64_t *index, unsigned int *num_bmen, unsigned int *num_bkings, unsigned int *num_wmen, unsigned int *num_wkings);


	EGDB_ERR egdb_close_wld_tun_v1(EGDB_DRIVER *driver)
	{
		if (!driver)
			return(EGDB_INVALID_HANDLE);

		exitdblookup((DBHANDLE)driver->internal_data);
		std::free(driver);
		return(EGDB_ERR_NORMAL);
	}

	unsigned int egdb_get_max_pieces_wld_tun_v1(EGDB_DRIVER *driver)
	{
		return(driver->max_pieces);
	}

	unsigned int egdb_get_info_wld_tun_v1(
		EGDB_DRIVER *driver,
		unsigned int num_pieces,
		EGDB_INFO *info,
		unsigned int max_info)
	{
		DBHANDLE h;

		if (num_pieces == 0 || num_pieces > driver->max_pieces)
			return(0);
		if (max_info == 0)
			return(0);

		h = (DBHANDLE)driver->internal_data;

		if (!h->cprsubdb[num_pieces - 1].ispresent)
			return(0);

		info->type = driver->db_type;
		info->num_pieces = num_pieces;
		info->compression = h->cprsubdb[num_pieces - 1].compression;
		info->dtw_w_only = false;
		info->contains_le_pieces = h->cprsubdb[num_pieces - 1].contains_le_pieces;
		return(1);
	}

	int egdb_wld_tun_v1_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
	{
		DBHANDLE h;
		int value;
		
		h = (DBHANDLE)driver->internal_data;

		// check if num_pieces is too large for this db.
		if (bit_pop_count64(pos->black_pieces | pos->white_pieces) > h->max_pieces) {
			*err_code = EGDB_NUM_PIECES_TOO_LARGE;
			return(EGDB_UNKNOWN);
		}

		value = dblookup(h, pos, err_code);
		if (*err_code != EGDB_ERR_NORMAL)
			return(EGDB_UNKNOWN);

		return(value);
	}

	EGDB_ERR egdb_open_wld_tun_v1(EGDB_DRIVER *driver)
	{
		DBHANDLE h;
		char const *db_dir = driver->path;
		unsigned int cache_size_bytes = driver->cache_size;

		h = initdblookup(db_dir, cache_size_bytes);
		if (h == 0)
			return(EGDB_DB_NOT_LOADED);

		driver->internal_data = h;
		return(EGDB_ERR_NORMAL);
	}


	/**
	* Given an EGDB directory path, initializes the egdb lookup library.
	* @param egtb_dir a path to the directory that contains the egdb files.
	* @param cache_size_bytes the size in bytes to use for the lru cache.
	* @return a handle if successful, else 0.
	*/
	static DBHANDLE initdblookup(char const *egtb_dir, unsigned int cache_size_bytes)
	{
		DBHANDLE h;
		int i;
		char cpr_fname[256];
		char idx_fname[256];
		char fullpath[512];
		std::string s;
		unsigned int max_num_dbs;
		        int file_num;
		        CPRSUBDB *csdb;		int num_dbs_loaded = 0;


		// allocate memory for the handle.
		h = (DBHANDLE)calloc(1, sizeof(struct DB_HANDLE_T));
		if (!h)
			return(0);
		
		h->db_type = EGDB_WLD_TUN_V1;
		h->max_pieces = 0;
		h->compression_type = EGDB_COMPRESSION_TUNSTALL_V1;

		// allocate the lru cache memory.
		h->cache_base = (uint8_t *)calloc(cache_size_bytes, 1);
		        if (!h->cache_base) {
		            std::free(h);
		            return(0);
		        }
		// allocate the info for each cache block.
		h->cache_block_info = (BLOCK_INFO *)calloc(MAXCACHEDBLOCKS, sizeof(BLOCK_INFO));
		if (!h->cache_block_info) {
							std::free(h->cache_block_ptr);			free(h->cache_base);
			free(h);
			return(0);
		}

		// initialize the cache.
		for (i = 0; i < MAXCACHEDBLOCKS; ++i) {
			h->cache_block_ptr[i] = 0;
			h->cache_block_info[i].prev = i - 1;
			h->cache_block_info[i].next = i + 1;
			h->cache_block_info[i].unique_id = -1;
		}
		h->cache_head = HEAD;
		h->cache_tail = TAIL;

		// determine the number of dbs in the database.
		max_num_dbs = get_total_num_wld_dbs();
		h->cprsubdb = (CPRSUBDB *)std::calloc(max_num_dbs, sizeof(CPRSUBDB));
		if (!h->cprsubdb) {
							std::free(h->cache_block_info);			free(h->cache_block_ptr);
							std::free(h->cache_base);			free(h);
			return(0);
		}

		// for each number of pieces, open the database.
		file_num = 0;
		for (i = 0; i < max_num_dbs; ++i) {
			csdb = &h->cprsubdb[i];
			csdb->db_type = EGDB_WLD_TUN_V1;
			csdb->compression = EGDB_COMPRESSION_TUNSTALL_V1;
			csdb->num_pieces = i + 2;

			// open the cpr file.
			if (csdb->num_pieces < SPLIT_POINT_TUNSTALL_V1) {
				sprintf(cpr_fname, "db%d.cpr", csdb->num_pieces);
			}
			else {
				// We don't have split dbs for wld tunstall v1.
				csdb->ispresent = false;
				continue;
			}
			s = egtb_dir;
			s += "/";
			s += cpr_fname;
			strncpy(fullpath, s.c_str(), sizeof(fullpath) - 1);
			csdb->file = (FILE_HANDLE)std::fopen(fullpath, "rb");
			if (!csdb->file) {
				// File does not exist, or could not be opened.  Do not treat as an error.
				csdb->ispresent = false;
				continue;
			}

			// open the idx file.
			if (csdb->num_pieces < SPLIT_POINT_TUNSTALL_V1) {
				sprintf(idx_fname, "db%d.idx", csdb->num_pieces);
			}
			else {
				csdb->ispresent = false;
				std::fclose((FILE*)csdb->file);
				continue;
			}

			            if (parseindexfile(h, idx_fname, file_num)) {
			                csdb->ispresent = false;
			                std::fclose((FILE*)csdb->file);				continue;
			}

			csdb->ispresent = true;
			h->max_pieces = csdb->num_pieces;
			num_dbs_loaded++;
			file_num++;
		}

		// check that at least one db was loaded.
		if (num_dbs_loaded == 0) {
			exitdblookup(h);
			return(0);
		}

		return(h);
	}

	static unsigned int parseindexfile(DBHANDLE h, char const *idx_fname, int file_num)
	{
		FILE *fp;
		char buffer[1024];
		char fullpath[512];
		std::string s;
		char *p;
		int bm, bk, wm, wk, color;
		uint64_t index;
		unsigned int num_lines;
		int num_bmen, num_bkings, num_wmen, num_wkings;
		        int num_dbs_for_this_piece_cnt;
		        CPRSUBDB *csdb;		INDEX_REC *idx_rec;
		int i;


		// open the index file.
		s = h->path;
		s += "/";
		s += idx_fname;
		strncpy(fullpath, s.c_str(), sizeof(fullpath) - 1);
		fp = std::fopen(fullpath, "r");
		if (!fp)
			return(1);

		// read the index file into memory.
		// first, count the number of lines.
		num_lines = 0;
		while (fgets(buffer, sizeof(buffer), fp)) {
			// skip blank lines.
			p = buffer;
			while (isspace(*p))
				p++;
			if (*p == 0)
				continue;

			// skip comment lines.
			if (*p == '#')
				continue;
			
			// a valid line.
			num_lines++;
		}
		rewind(fp);

		// now read all the lines.
		for (i = 0; i < num_lines; ++i) {
			if (!fgets(buffer, sizeof(buffer), fp))
				break;

			p = buffer;
			while (isspace(*p))
				p++;
			if (*p == 0)
				continue;
			if (*p == '#')
				continue;

			// parse the line.
			// e.g. "2,0,1,0,0:14470,265"
			if (sscanf(p, "%d,%d,%d,%d,%d:%llu,%d", &bm, &bk, &wm, &wk, &color,
					   (long long unsigned int *)&index, &num_dbs_for_this_piece_cnt) != 7) {
				fclose(fp);
				return(1);
			}

			// create the subdb.
			num_bmen = bm;
			num_bkings = bk;
			num_wmen = wm;
			num_wkings = wk;
			csdb = &h->cprsubdb[num_bmen + num_bkings + num_wmen + num_wkings - 2];

			// set up the index record.
			idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
			if (!idx_rec) {
				fclose(fp);
				return(1);
			}
			idx_rec->file_num = file_num;
			idx_rec->offset_in_file = index;
			idx_rec->num_dbs = num_dbs_for_this_piece_cnt;
			idx_rec->num_bmen = num_bmen;
			idx_rec->num_bkings = num_bkings;
			idx_rec->num_wmen = num_wmen;
			idx_rec->num_wkings = num_wkings;
			idx_rec->side_to_move = (color == 0) ? EGDB_WHITE_TO_MOVE : EGDB_BLACK_TO_MOVE;

			// add the index record to the list.
			idx_rec->next = csdb->index_list;
			csdb->index_list = idx_rec;
		}

		fclose(fp);
		return(0);
	}

	/**
	* Looks up a board position in the database.
	* @param h a handle to the database.
	* @param pos a pointer to a board position.
	* @param err if an error occurs, this will be set to one of EGDB_ERR. If no
	* error occurs, this will be set to EGDB_NORMAL.
	* @return the value of the board position, one of EGDB_WIN, EGDB_LOSS, EGDB_DRAW,
	* or EGDB_UNKNOWN.
	*/
	static int dblookup(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err)
	{
		cprsubdb *csdb;
		int num_pieces;
		int value;
		INDEX_REC *idx_rec;
		uint64_t index;
		uint8_t *db_data_block;
		unsigned int db_len_bytes;
		unsigned int num_bmen, num_bkings, num_wmen, num_wkings;
		uint64_t lsb_index, msb_index;


		*err = EGDB_ERR_NORMAL;

		// get the subdb.
		num_pieces = bit_pop_count64(pos->black_pieces | pos->white_pieces);
		if (num_pieces < 2 || num_pieces > h->max_pieces) {
			*err = EGDB_NUM_PIECES_OUT_OF_BOUNDS;
			return(EGDB_UNKNOWN);
		}
		csdb = &h->cprsubdb[num_pieces - 2];
		if (!csdb->ispresent) {
			*err = EGDB_DB_NOT_LOADED;
			return(EGDB_UNKNOWN);
		}

		// check for too many kings.
		if (pos->stm == EGDB_BLACK_TO_MOVE) {
			if (bit_pop_count64(pos->white_pieces & pos->king) > num_pieces - 1) {
				*err = EGDB_INVALID_POS;
				return(EGDB_UNKNOWN);
			}
		}
		else {	// white to move
			if (bit_pop_count64(pos->black_pieces & pos->king) > num_pieces - 1) {
				*err = EGDB_INVALID_POS;
				return(EGDB_UNKNOWN);
			}
			if (bit_pop_count64(pos->white_pieces & pos->king) > num_pieces - 1) {
				*err = EGDB_INVALID_POS;
				return(EGDB_UNKNOWN);
			}
		}

		// get the index for this position.
		get_board_index(pos, &index, &num_bmen, &num_bkings, &num_wmen, &num_wkings);

		// find the correct index record.
		// The index record is based on num_bmen, num_bkings, num_wmen, num_wkings,
		// and side to move.
		idx_rec = csdb->index_list;
		while (idx_rec) {
			if ((idx_rec->num_bmen == num_bmen) &&
				(idx_rec->num_bkings == num_bkings) &&
				(idx_rec->num_wmen == num_wmen) &&
				(idx_rec->num_wkings == num_wkings) &&
				(idx_rec->side_to_move == pos->stm)) {
				break;
			}
			idx_rec = idx_rec->next;
		}
		if (!idx_rec) {
			*err = EGDB_DB_NOT_LOADED;
			return(EGDB_UNKNOWN);
		}

		// get the data block from disk (or cache).
		if (get_db_data_block(h, idx_rec->file_num, idx_rec->offset_in_file,
							  idx_rec->num_dbs, &db_data_block, &db_len_bytes)) {
			*err = EGDB_FILE_READ_ERROR;
			return(EGDB_UNKNOWN);
		}
		
		// sanity check.
		if (index >= db_len_bytes) {
			*err = EGDB_INDEX_OUT_OF_BOUNDS;
			return(EGDB_UNKNOWN);
		}

		// now decompress the data.
		// These dbs are split by piece count (4 and 5 pieces).
		// We pass the raw index to the decompressor.
		decompress_wld_tunstall_v1(db_data_block, db_len_bytes,
								   index, 0, &value);
		
		// The egdb stores values as:
		// 0 = white win, 1 = black win, 2 = draw.
		// We want:
		// EGDB_WIN = white win, EGDB_LOSS = black win, EGDB_DRAW = draw.
		if (value == 0) {
			return(EGDB_WIN);
		} else if (value == 1) {
			return(EGDB_LOSS);
		} else if (value == 2) {
			return(EGDB_DRAW);
		}
		
		*err = EGDB_DECOMPRESSION_FAILED;
		return(EGDB_UNKNOWN);
	}

	/**
	* Frees up memory allocated by initdblookup().
	* @param h a handle to the database.
	*/
	static void exitdblookup(DBHANDLE h)
	{
		int i;
		INDEX_REC *idx_rec, *idx_rec_next;

		if (h) {
			if (h->cprsubdb) {
				for (i = 0; i < get_total_num_wld_dbs(); ++i) {
					                    if (h->cprsubdb[i].file)
					                        std::fclose((FILE*)h->cprsubdb[i].file);					idx_rec = h->cprsubdb[i].index_list;
					while (idx_rec) {
						idx_rec_next = idx_rec->next;
						std::free(idx_rec);
						idx_rec = idx_rec_next;
					}
				}
				std::free(h->cprsubdb);
			}
			if (h->cache_block_info)
				std::free(h->cache_block_info);
			if (h->cache_block_ptr)
				std::free(h->cache_block_ptr);
			if (h->cache_base)
				std::free(h->cache_base);
			free(h);
		}
	}

}	// namespace egdb_interface
