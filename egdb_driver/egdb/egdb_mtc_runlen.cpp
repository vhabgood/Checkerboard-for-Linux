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
#include <vector>
#include <dirent.h>
#include <algorithm>

#include "egdb/egdb_common.h"
#include "egdb/egdb_intl.h"
#include "egdb/db_io.h"
#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/reverse.h"
#include "engine/bool.h"
#include "builddb/compression_tables.h"
#include "builddb/indexing.h"
#include "engine/board_indexing.h"
#include "egdb/egdb_types.h"
#include "log.h"

namespace egdb_interface {

    const int MTC_UNKNOWN_VAL = -1;

	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num);
	static int dblookup_mtc_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err);
	static DBHANDLE initdblookup_mtc(const char *egtb_dir, unsigned int cache_size_bytes);
	static void exitdblookup_mtc(DBHANDLE h);
    unsigned int get_total_num_mtc_dbs();

	int egdb_close_mtc(EGDB_DRIVER *driver)
	{
		if (!driver) return(EGDB_INVALID_HANDLE);
		exitdblookup_mtc((DBHANDLE)driver->internal_data);
		free(driver);
		return(0);
	}

	unsigned int egdb_get_max_pieces_mtc(EGDB_DRIVER *driver)
	{
		return(driver->max_pieces);
	}

	unsigned int egdb_get_info_mtc(EGDB_DRIVER *driver, unsigned int num_pieces, EGDB_INFO *info, unsigned int max_info)
	{
		DBHANDLE h;
		if (num_pieces < 2 || num_pieces > driver->max_pieces) return(0);
		if (max_info == 0) return(0);
		h = (DBHANDLE)driver->internal_data;
		if (!h->cprsubdb[num_pieces - 2].ispresent) return(0);
		info->type = driver->db_type;
		info->num_pieces = num_pieces;
		info->compression = EGDB_COMPRESSION_RUNLEN;
		info->dtw_w_only = false;
		info->contains_le_pieces = true;
		return(1);
	}

    int egdb_mtc_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
    {
        EGDB_BITBOARD bitboard;
        bitboard.normal.black = pos->black_pieces;
        bitboard.normal.white = pos->white_pieces;
        bitboard.normal.king = pos->king;
        return driver->lookup(driver, &bitboard, pos->stm, 0);
    }

    static int dblookup_mtc_wrapper(EGDB_DRIVER *driver, EGDB_BITBOARD *position, int color, int cl)
    {
        DBHANDLE h = (DBHANDLE)driver->internal_data;
        EGDB_POSITION pos;
        pos.black_pieces = position->normal.black;
        pos.white_pieces = position->normal.white;
        pos.king = position->normal.king;
        pos.stm = color;
        EGDB_ERR err;
        return dblookup_mtc_internal(h, &pos, &err);
    }

	EGDB_ERR egdb_open_mtc_runlen(EGDB_DRIVER *driver)
	{
		DBHANDLE h = initdblookup_mtc(driver->path, driver->cache_size);
		if (!h) return(EGDB_DB_NOT_LOADED);
		driver->internal_data = h;
		driver->lookup = dblookup_mtc_wrapper;
		driver->close = egdb_close_mtc;
		driver->max_pieces = h->max_pieces;
		return(EGDB_ERR_NORMAL);
	}

	static DBHANDLE initdblookup_mtc(const char *egtb_dir, unsigned int cache_size_bytes)
	{
		DBHANDLE h;
		CPRSUBDB *csdb;
		unsigned int i, max_num_dbs, num_dbs_loaded;
		int file_num;

		h = (DBHANDLE)calloc(1, sizeof(DB_HANDLE_T));
		if (!h) return(0);

		strncpy(h->path, egtb_dir, sizeof(h->path) - 1);
		h->cache_size = (cache_size_bytes < (4 * 1024 * 1024)) ? (4 * 1024 * 1024) : cache_size_bytes;

		h->cache_base = (uint8_t *)malloc(h->cache_size);
		if (!h->cache_base) { free(h); return(0); }

		h->cache_block_ptr = (uint8_t **)malloc(MAXCACHEDBLOCKS * sizeof(uint8_t *));
		h->cache_block_info = (BLOCK_INFO *)malloc(MAXCACHEDBLOCKS * sizeof(BLOCK_INFO));
		if (!h->cache_block_ptr || !h->cache_block_info) { free(h->cache_base); free(h); return(0); }

		for (i = 0; i < MAXCACHEDBLOCKS; i++) {
			h->cache_block_ptr[i] = h->cache_base + (i * 4096);
			h->cache_block_info[i].prev = i - 1;
			h->cache_block_info[i].next = i + 1;
			h->cache_block_info[i].unique_id_64 = (uint64_t)-1;
            h->cache_block_info[i].bytes_in_block = 0;
		}
		h->cache_block_info[MAXCACHEDBLOCKS - 1].next = -1;
		h->cache_head = HEAD; h->cache_tail = TAIL;

		max_num_dbs = get_total_num_mtc_dbs();
		h->cprsubdb = (CPRSUBDB *)calloc(max_num_dbs, sizeof(CPRSUBDB));
		if (!h->cprsubdb) { free(h->cache_block_info); free(h->cache_base); free(h); return(0); }

		file_num = 0; num_dbs_loaded = 0;
		for (i = 0; i < max_num_dbs; ++i) {
			int n_pieces = i + 2;
			csdb = &h->cprsubdb[i];
			csdb->db_type = EGDB_MTC_RUNLEN;
			csdb->compression = EGDB_COMPRESSION_RUNLEN;
			csdb->num_pieces = n_pieces;

            std::vector<std::string> idx_files;
            DIR *dir = opendir(h->path);
            if (dir) {
                struct dirent *ent;
                std::string prefix = "db" + std::to_string(n_pieces);
                while ((ent = readdir(dir)) != nullptr) {
                    std::string fname = ent->d_name;
                    if (fname.size() >= prefix.size() + 4 &&
                        fname.compare(0, prefix.size(), prefix) == 0 &&
                        (fname.find(".idx_mtc") != std::string::npos || 
                         (fname.find(".idx") != std::string::npos && fname.find("mtc") != std::string::npos))) {
                        if (fname.size() > prefix.size() && isdigit(fname[prefix.size()])) continue;
                        idx_files.push_back(fname);
                    }
                }
                closedir(dir);
            }

            for (const std::string& idx_f_str : idx_files) {
                if (parseindexfile_mtc(h, idx_f_str.c_str(), file_num) == 0) {
                    csdb->ispresent = true;
                    num_dbs_loaded++;
                    file_num++;
                }
            }
			if (csdb->ispresent) h->max_pieces = n_pieces;
		}
		if (num_dbs_loaded == 0) { exitdblookup_mtc(h); return(0); }
		return(h);
	}

	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num)
	{
		FILE *fp, *cpr_fp = nullptr;
		char buffer[4096], fullpath[512], cpr_fname[256];
		std::string s;
		char *p;
		int bm, bk, wm, wk, rank;
        char colorchar;
		INDEX_REC *idx_rec = nullptr;

		s = h->path; s += "/";
        std::string base_path = s; s += idx_fname;
		strncpy(fullpath, s.c_str(), sizeof(fullpath) - 1);
		fp = fopen(fullpath, "r");
		if (!fp) return(1);

        strncpy(cpr_fname, idx_fname, sizeof(cpr_fname) - 1);
        std::string cpr_f_str = cpr_fname;
        size_t idx_pos = cpr_f_str.find(".idx");
        if (idx_pos != std::string::npos) cpr_f_str.replace(idx_pos, 4, ".cpr");
        std::string cpr_path = base_path + cpr_f_str;
        cpr_fp = fopen(cpr_path.c_str(), "rb");
        if (!cpr_fp) {
            if (idx_pos != std::string::npos) cpr_f_str.replace(idx_pos, 4, ".mtc");
            cpr_path = base_path + cpr_f_str;
            cpr_fp = fopen(cpr_path.c_str(), "rb");
        }
        if (!cpr_fp) { fclose(fp); return(1); }

        CPRSUBDB *csdb = nullptr;
        uint64_t current_file_offset = 0;

		while (fgets(buffer, sizeof(buffer), fp)) {
			p = buffer; while (*p && isspace(*p)) p++;
            if (*p == '\0') continue;

			if (strncmp(p, "BASE", 4) == 0) {
				if (sscanf(p, "BASE%d,%d,%d,%d,%d,%c:", &rank, &bm, &bk, &wm, &wk, &colorchar) == 6) {
                    int n_pieces = bm + bk + wm + wk;
                    csdb = &h->cprsubdb[n_pieces - 2];
                    idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                    idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                    idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                    idx_rec->side_to_move = (colorchar == 'w') ? EGDB_WHITE_TO_MOVE : EGDB_BLACK_TO_MOVE;
                    idx_rec->bmrank = (bm > 0) ? rank : 0;
                    idx_rec->wmrank = (wm > 0) ? rank : 0;
                    idx_rec->file_num = file_num;
                    
                    p = strchr(p, ':');
                    if (p) {
                        p++;
                        unsigned int block_id, start_byte;
                        if (sscanf(p, "%u/%u", &block_id, &start_byte) == 2) {
                            idx_rec->first_block_id = block_id;
                            idx_rec->startbyte = start_byte;
                            current_file_offset = (uint64_t)block_id * 4096 + start_byte;
                        }
                    }

                    auto get_next_num = [](char **ptr) -> int64_t {
                        while (**ptr && !isdigit(**ptr) && **ptr != '-') (*ptr)++;
                        if (**ptr == '\0') return -1;
                        char *end;
                        int64_t val = strtoll(*ptr, &end, 10);
                        *ptr = end;
                        return val;
                    };

                    int64_t skip0 = get_next_num(&p);
                    int64_t val0 = get_next_num(&p);
                    if (skip0 != -1 && val0 != -1) {
                        current_file_offset += (uint64_t)skip0;
                        idx_rec->checkpoints = (Checkpoint*)malloc(sizeof(Checkpoint));
                        idx_rec->checkpoints[0] = {0, current_file_offset, (uint8_t)val0};
                        idx_rec->num_checkpoints = 1;
                        int dist = MTC_DECODE((uint8_t)val0);
                        if (dist == 0 && (uint8_t)val0 >= MTC_SKIPS) dist = 1;
                        idx_rec->initial_value = dist;
                    }
                    idx_rec->file = cpr_fp;
                    idx_rec->next = csdb->index_list;
                    csdb->index_list = idx_rec;
				}
			} else if (isdigit(*p)) {
                if (idx_rec) {
                    char *end; int64_t skip = strtoll(p, &end, 10); p = end;
                    int64_t val = strtoll(p, &end, 10);
                    if (end != p) {
                        current_file_offset += (uint64_t)skip;
                        idx_rec->num_checkpoints++;
                        idx_rec->checkpoints = (Checkpoint*)realloc(idx_rec->checkpoints, idx_rec->num_checkpoints * sizeof(Checkpoint));
                        idx_rec->checkpoints[idx_rec->num_checkpoints - 1] = {0, current_file_offset, (uint8_t)val};
                    }
                }
            }
		}
		fclose(fp);
		return(0);
	}

	static int dblookup_mtc_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err)
	{
		CPRSUBDB *csdb;
        int num_pieces;
        INDEX_REC *idx_rec;
        uint8_t *diskblock;
        uint64_t current_index;

        *err = EGDB_ERR_NORMAL;
        num_pieces = bit_pop_count64(pos->black_pieces | pos->white_pieces);
        if (num_pieces < 2 || num_pieces > h->max_pieces) {
            *err = EGDB_NUM_PIECES_OUT_OF_BOUNDS;
            return(MTC_UNKNOWN_VAL);
        }
        csdb = &h->cprsubdb[num_pieces - 2];
        if (!csdb->ispresent) { *err = EGDB_DB_NOT_LOADED; return(MTC_UNKNOWN_VAL); }

        unsigned int num_bmen = bit_pop_count64(pos->black_pieces & ~pos->king);
        unsigned int num_bkings = bit_pop_count64(pos->black_pieces & pos->king);
        unsigned int num_wmen = bit_pop_count64(pos->white_pieces & ~pos->king);
        unsigned int num_wkings = bit_pop_count64(pos->white_pieces & pos->king);

        int bmrank = 0, wmrank = 0;
        if (num_bmen) bmrank = msb_(pos->black_pieces & ~pos->king) / 4;
        if (num_wmen) wmrank = (31 - lsb_(pos->white_pieces & ~pos->king)) / 4;

        bool reverse_board = false;
        if ((((int)(num_wmen + num_wkings - num_bmen - num_bkings) << 16) + 
             ((int)(num_wkings - num_bkings) << 8) + 
             ((int)(wmrank - bmrank) << 4) + 
             (pos->stm == EGDB_WHITE_TO_MOVE ? 1 : 0)) > 0) {
            reverse_board = true;
        }

        EGDB_POSITION lookup_pos = *pos;
        if (reverse_board) {
            lookup_pos.black_pieces = pos->white_pieces;
            lookup_pos.white_pieces = pos->black_pieces;
            lookup_pos.king = pos->king;
            reverse_bitboard(&lookup_pos.black_pieces);
            reverse_bitboard(&lookup_pos.white_pieces);
            reverse_bitboard(&lookup_pos.king);
            lookup_pos.stm = (pos->stm == EGDB_WHITE_TO_MOVE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
            num_bmen = bit_pop_count64(lookup_pos.black_pieces & ~lookup_pos.king);
            num_bkings = bit_pop_count64(lookup_pos.black_pieces & lookup_pos.king);
            num_wmen = bit_pop_count64(lookup_pos.white_pieces & ~lookup_pos.king);
            num_wkings = bit_pop_count64(lookup_pos.white_pieces & lookup_pos.king);
            if (num_bmen) bmrank = msb_(lookup_pos.black_pieces & ~lookup_pos.king) / 4; else bmrank = 0;
            if (num_wmen) wmrank = (31 - lsb_(lookup_pos.white_pieces & ~lookup_pos.king)) / 4; else wmrank = 0;
        }

        auto find_record_mtc = [&](const EGDB_POSITION *p, int bmen, int bkings, int wmen, int wkings, int b_rank, int w_rank) -> INDEX_REC* {
            INDEX_REC *rec = csdb->index_list;
            INDEX_REC *catch_all = nullptr;
            while (rec) {
                if (rec->num_bmen == bmen && rec->num_bkings == bkings &&
                    rec->num_wmen == wmen && rec->num_wkings == wkings &&
                    rec->side_to_move == p->stm) {
                    if (bmen > 0) {
                        if (rec->bmrank == b_rank) return rec;
                        if (rec->bmrank == 0) catch_all = rec;
                    } else if (wmen > 0) {
                        if (rec->wmrank == w_rank) return rec;
                        if (rec->wmrank == 0) catch_all = rec;
                    } else return rec;
                }
                rec = rec->next;
            }
            return catch_all;
        };

        idx_rec = find_record_mtc(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings, bmrank, wmrank);
        if (!idx_rec) {
            EGDB_POSITION flipped = lookup_pos;
            flipped.black_pieces = lookup_pos.white_pieces;
            flipped.white_pieces = lookup_pos.black_pieces;
            flipped.king = lookup_pos.king;
            reverse_bitboard(&flipped.black_pieces);
            reverse_bitboard(&flipped.white_pieces);
            reverse_bitboard(&flipped.king);
            flipped.stm = (lookup_pos.stm == EGDB_WHITE_TO_MOVE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
            int f_bmen = bit_pop_count64(flipped.black_pieces & ~flipped.king);
            int f_bkings = bit_pop_count64(flipped.black_pieces & flipped.king);
            int f_wmen = bit_pop_count64(flipped.white_pieces & ~flipped.king);
            int f_wkings = bit_pop_count64(flipped.white_pieces & flipped.king);
            int f_bm_rank = 0, f_wm_rank = 0;
            if (f_bmen) f_bm_rank = msb_(flipped.black_pieces & ~flipped.king) / 4;
            if (f_wmen) f_wm_rank = (31 - lsb_(flipped.white_pieces & ~flipped.king)) / 4;
            idx_rec = find_record_mtc(&flipped, f_bmen, f_bkings, f_wmen, f_wkings, f_bm_rank, f_wm_rank);
            if (idx_rec) {
                lookup_pos = flipped; num_bmen = f_bmen; num_bkings = f_bkings; num_wmen = f_wmen; num_wkings = f_wkings;
                bmrank = f_bm_rank; wmrank = f_wm_rank;
            }
        }

        if (!idx_rec) {
            if (num_bmen + num_bkings == 0) return (pos->stm == EGDB_BLACK) ? 0 : 999;
            if (num_wmen + num_wkings == 0) return (pos->stm == EGDB_WHITE) ? 0 : 999;
            *err = EGDB_DB_NOT_LOADED; return(MTC_UNKNOWN_VAL);
        }
        
        current_index = (uint64_t)position_to_index_slice(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings);
        uint64_t absolute_file_offset, decompressed_index; uint8_t start_byte_val;

        if (idx_rec->num_checkpoints > 0) {
            int best_cp_idx = -1;
            for (int i = 0; i < idx_rec->num_checkpoints; ++i) {
                if (current_index >= idx_rec->checkpoints[i].index) best_cp_idx = i;
                else break;
            }
            if (best_cp_idx == -1) { *err = EGDB_INDEX_OUT_OF_BOUNDS; return MTC_UNKNOWN_VAL; }
            Checkpoint& cp = idx_rec->checkpoints[best_cp_idx];
            absolute_file_offset = cp.file_offset; decompressed_index = cp.index; start_byte_val = cp.initial_byte;
        } else {
            absolute_file_offset = (uint64_t)idx_rec->first_block_id * 4096 + idx_rec->startbyte;
            decompressed_index = 0; start_byte_val = 0;
        }

        int blocknumber = (int)(absolute_file_offset / 4096);
        int offset = (int)(absolute_file_offset % 4096);
        int cache_idx;
        if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) { *err = EGDB_FILE_READ_ERROR; return(MTC_UNKNOWN_VAL); }
		
        int current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
        uint64_t target_index = current_index;
        int current_dist = (start_byte_val >= MTC_SKIPS) ? MTC_DECODE(start_byte_val) : idx_rec->initial_value;
        if (current_dist == 0 && start_byte_val >= MTC_SKIPS) current_dist = 1;

        while (decompressed_index <= target_index) {
            uint32_t run = 0;
            while (diskblock[offset] < MTC_SKIPS) {
                run += diskblock[offset]; offset++;
                if (offset >= current_block_size) {
                    if (current_block_size < 4096) return current_dist;
                    blocknumber++;
                    if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) return current_dist; 
                    current_block_size = h->cache_block_info[cache_idx].bytes_in_block; offset = 0;
                }
            }
            uint8_t dist_byte = diskblock[offset];
            current_dist = MTC_DECODE(dist_byte);
            if (current_dist == 0 && dist_byte >= MTC_SKIPS) current_dist = 1;
            if (target_index < decompressed_index + run + 1) return current_dist;
            decompressed_index += (run + 1); offset++;
            if (offset >= current_block_size) {
                if (decompressed_index > target_index) break;
                if (current_block_size < 4096) break;
                blocknumber++;
                if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) break;
                current_block_size = h->cache_block_info[cache_idx].bytes_in_block; offset = 0;
            }
        }
        return current_dist;
	}

	static void exitdblookup_mtc(DBHANDLE h)
	{
		int i; INDEX_REC *idx_rec, *idx_rec_next;
		if (h) {
			if (h->cprsubdb) {
				for (i = 0; i < (int)get_total_num_mtc_dbs(); ++i) {
					idx_rec = h->cprsubdb[i].index_list; FILE *last_file = nullptr;
					while (idx_rec) {
						idx_rec_next = idx_rec->next;
                        if (idx_rec->file && idx_rec->file != last_file) { fclose(idx_rec->file); last_file = idx_rec->file; }
                        if (idx_rec->checkpoints) free(idx_rec->checkpoints);
                        free(idx_rec); idx_rec = idx_rec_next;
					}
				}
				free(h->cprsubdb);
			}
			free(h->cache_block_info);
            if (h->cache_block_ptr) free(h->cache_block_ptr);
			if (h->cache_base) free(h->cache_base);
			free(h);
		}
	}

	unsigned int get_total_num_mtc_dbs()
	{
		return(MAXPIECE - 2 + 1);
	}

} // namespace egdb_interface
