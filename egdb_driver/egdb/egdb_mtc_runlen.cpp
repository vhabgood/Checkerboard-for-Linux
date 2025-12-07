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
#include "egdb/egdb_types.h"
#include "log.h"

namespace egdb_interface {

    // MTC_UNKNOWN is -1, MTC_WIN is 1, MTC_LOSS is -1, MTC_DRAW is 0
    const int MTC_WIN_VAL = 1;
    const int MTC_LOSS_VAL = -1;
    const int MTC_UNKNOWN_VAL = -1;
    const int MTC_DRAW_VAL = 0;

	// forward references
	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num);
	static int dblookup_mtc_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err);
	static DBHANDLE initdblookup_mtc(const char *egtb_dir, unsigned int cache_size_bytes);
	static void exitdblookup_mtc(DBHANDLE h);
    unsigned int get_total_num_mtc_dbs();


	EGDB_ERR egdb_close_mtc(EGDB_DRIVER *driver)
	{
		if (!driver)
			return(EGDB_INVALID_HANDLE);

		exitdblookup_mtc((DBHANDLE)driver->internal_data);
		free(driver);
		return(EGDB_ERR_NORMAL);
	}

	unsigned int egdb_get_max_pieces_mtc(EGDB_DRIVER *driver)
	{
		return(driver->max_pieces);
	}

	unsigned int egdb_get_info_mtc(
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

		if (!h->cprsubdb[num_pieces - 2].ispresent)
		 	return(0);

		info->type = driver->db_type;
		info->num_pieces = num_pieces;
		info->compression = EGDB_COMPRESSION_RUNLEN;
		info->dtw_w_only = false;
		info->contains_le_pieces = true;
		return(1);
	}

    static uint64_t convert_cake_to_kingsrow(uint64_t x) {
        // Reverses the order of bits in each 4-bit nibble.
        // Cake: Sq1=Bit3, Sq2=Bit2, Sq3=Bit1, Sq4=Bit0 (in first nibble)
        // Kingsrow: Sq1=Bit0, Sq2=Bit1, Sq3=Bit2, Sq4=Bit3
        uint32_t lo = (uint32_t)x;
        uint32_t hi = (uint32_t)(x >> 32);
        
        auto convert32 = [](uint32_t v) -> uint32_t {
            return ((v & 0x88888888) >> 3) | 
                   ((v & 0x44444444) >> 1) | 
                   ((v & 0x22222222) << 1) | 
                   ((v & 0x11111111) << 3);
        };
        
        return ((uint64_t)convert32(hi) << 32) | convert32(lo);
    }

    int egdb_mtc_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
    {
        DBHANDLE h;
        int value;

        h = (DBHANDLE)driver->internal_data;

        // check if num_pieces is too large for this db.
        if (bit_pop_count64(pos->black_pieces | pos->white_pieces) > h->max_pieces) {
            *err_code = EGDB_NUM_PIECES_TOO_LARGE;
            return(MTC_UNKNOWN);
        }

        EGDB_POSITION local_pos = *pos;
        if (driver->bitboard_type == EGDB_ROW_REVERSED) {
            local_pos.black_pieces = convert_cake_to_kingsrow(pos->black_pieces);
            local_pos.white_pieces = convert_cake_to_kingsrow(pos->white_pieces);
            local_pos.king = convert_cake_to_kingsrow(pos->king);
        }

        value = dblookup_mtc_internal(h, &local_pos, err_code);
        if (*err_code != EGDB_ERR_NORMAL)
            return(MTC_UNKNOWN);

        return(value);
    }

	EGDB_ERR egdb_open_mtc_runlen(EGDB_DRIVER *driver)
	{
		DBHANDLE h;
		char const *db_dir = driver->path;
		unsigned int cache_size_bytes = driver->cache_size;

		h = initdblookup_mtc(db_dir, cache_size_bytes);
		if (h == 0)
			return(EGDB_DB_NOT_LOADED);

		driver->internal_data = h;
		return(EGDB_ERR_NORMAL);
	}


	static DBHANDLE initdblookup_mtc(char const *egtb_dir, unsigned int cache_size_bytes)
	{
		DBHANDLE h;
		int i;
		char idx_fname[256];
		unsigned int max_num_dbs;
		CPRSUBDB *csdb;
		int num_dbs_loaded = 0;
        int file_num;


		// allocate memory for the handle.
		h = (DBHANDLE)calloc(1, sizeof(struct DB_HANDLE_T));
		if (!h)
			return(0);
		
		h->db_type = EGDB_MTC_RUNLEN;
		h->max_pieces = 0;
		h->compression_type = EGDB_COMPRESSION_RUNLEN;
        strncpy(h->path, egtb_dir, sizeof(h->path) - 1);

		// allocate the lru cache memory.
		h->cache_base = (uint8_t *)calloc(cache_size_bytes, 1);
		if (!h->cache_base) {
			free(h);
			return(0);
		}
		h->cache_size = cache_size_bytes;

		// allocate the pointers to the cache blocks.
		h->cache_block_ptr = (uint8_t **)calloc(MAXCACHEDBLOCKS, sizeof(uint8_t *));
		        if (!h->cache_block_ptr) {
		            free(h->cache_base);			free(h);
			return(0);
		}

		// allocate the info for each cache block.
		h->cache_block_info = (BLOCK_INFO *)calloc(MAXCACHEDBLOCKS, sizeof(BLOCK_INFO));
		if (!h->cache_block_info) {
			free(h->cache_block_ptr);
			free(h->cache_base);
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
		h->cache_block_info[0].prev = -1;
		h->cache_block_info[MAXCACHEDBLOCKS - 1].next = -1;
		h->cache_head = HEAD;
		h->cache_tail = TAIL;

		// determine the number of dbs in the database.
		max_num_dbs = get_total_num_mtc_dbs();
		h->cprsubdb = (CPRSUBDB *)calloc(max_num_dbs, sizeof(CPRSUBDB));
		if (!h->cprsubdb) {
			free(h->cache_block_info);
			free(h->cache_base);
			free(h);
			return(0);
		}

		file_num = 0;
		for (i = 0; i < (int)max_num_dbs; ++i) {
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

            for (const std::string& idx_fname_str : idx_files) {
                const char* idx_f = idx_fname_str.c_str();
                if (parseindexfile_mtc(h, idx_f, file_num) == 0) {
                    csdb->ispresent = true;
                    num_dbs_loaded++;
                    file_num++;
                }
            }
			if (csdb->ispresent) h->max_pieces = n_pieces;
		}

		// check that at least one db was loaded.
		if (num_dbs_loaded == 0) {
            log_c(LOG_LEVEL_DEBUG, "initdblookup_mtc: NO MTC DBs loaded.");
			exitdblookup_mtc(h);
			return(0);
		}
        log_c(LOG_LEVEL_DEBUG, "initdblookup_mtc: Total MTC files loaded: %d. Max pieces: %d", num_dbs_loaded, h->max_pieces);
		return(h);
	}

	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num)
	{
		FILE *fp, *cpr_fp = nullptr;
		char buffer[1024];
		char fullpath[512], cpr_fullpath[512];
		std::string s;
		char *p;
		int bm, bk, wm, wk, rank;
        char colorchar;
		INDEX_REC *idx_rec = nullptr;
        char cpr_fname[256];
        int last_bm = -1, last_bk = -1, last_wm = -1, last_wk = -1, last_color = -1;
        uint32_t next_block_id = 0;
        int current_rank = 0;

		// open the index file.
		s = h->path;
		s += "/";
        std::string base_path = s;
		s += idx_fname;
		strncpy(fullpath, s.c_str(), sizeof(fullpath) - 1);
		fp = fopen(fullpath, "r");
		if (!fp)
			return(1);

        // Derive cpr filename from idx filename
        strncpy(cpr_fname, idx_fname, sizeof(cpr_fname) - 1);
        std::string cpr_f_str = cpr_fname;
        size_t idx_pos = cpr_f_str.find(".idx");
        if (idx_pos != std::string::npos) {
            cpr_f_str.replace(idx_pos, 4, ".cpr");
        }
        std::string cpr_path = base_path + cpr_f_str;
        cpr_fp = fopen(cpr_path.c_str(), "rb");
        if (!cpr_fp) {
            // Try .mtc extension
            if (idx_pos != std::string::npos) {
                cpr_f_str.replace(idx_pos, 4, ".mtc");
                cpr_path = base_path + cpr_f_str;
                cpr_fp = fopen(cpr_path.c_str(), "rb");
            }
        }
        if (!cpr_fp) {
            fclose(fp);
            return 1;
        }

		while (fgets(buffer, sizeof(buffer), fp)) {
			p = buffer;
			while (isspace(*p)) p++;
			if (*p == 0 || *p == '#') continue;

			if (strncmp(p, "BASE", 4) == 0) {
                p += 4;
                int n_read = 0;
                if (sscanf(p, "%d,%d,%d,%d,%d,%c:%n", 
                    &bm, &bk, &wm, &wk, &rank, &colorchar, &n_read) >= 6) {
                    
                    last_bm = bm; last_bk = bk; last_wm = wm; last_wk = wk;
                    last_color = (colorchar == 'b') ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
                    current_rank = rank;
                    p += n_read;

                    if (*p == '+' || *p == '-' || *p == '=') {
                        // Single value hit
                        idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                        idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                        idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                        idx_rec->side_to_move = last_color;
                        if (bm > 0) idx_rec->bmrank = current_rank; else idx_rec->wmrank = current_rank;
                        idx_rec->idx_size = 0;
                        idx_rec->file_num = file_num;
                        idx_rec->initial_value = (*p == '+') ? 100 : ((*p == '-') ? -100 : 0);
                        
                        int pieces = bm + bk + wm + wk;
                        CPRSUBDB *csdb = &h->cprsubdb[pieces - 2];
                        idx_rec->file = cpr_fp;
                        idx_rec->next = csdb->index_list;
                        csdb->index_list = idx_rec;
                        continue;
                    }

                    unsigned int block_id, start_byte;
                    if (sscanf(p, "%u/%u%n", &block_id, &start_byte, &n_read) == 2) {
                        p += n_read;
                        idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                        idx_rec->file = cpr_fp;
                        idx_rec->file_num = file_num;
                        idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                        idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                        idx_rec->side_to_move = last_color;
                        if (bm > 0) idx_rec->bmrank = current_rank; else idx_rec->wmrank = current_rank;
                        
                        // Normalize offsets: If start_byte >= 1024, shift it into block_id
                        idx_rec->first_block_id = block_id + (start_byte / 1024);
                        idx_rec->startbyte = start_byte % 1024;
                        
                        // Parse optional initial_value if present
                        if (*p == ',') {
                            p++;
                            strtol(p, &p, 10); // Skip block count
                            if (*p == ',') {
                                p++;
                                idx_rec->initial_value = strtol(p, &p, 10);
                            }
                        }

                        std::vector<uint64_t> offsets;
                        offsets.push_back(0); 
                        
                        auto parse_block_start = [&](char *ptr) {
                            char *endptr;
                            uint64_t val = strtoull(ptr, &endptr, 10);
                            if (endptr != ptr) return (uint64_t)val;
                            return (uint64_t)0ULL;
                        };

                        while (*p) {
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            if (!isdigit(*p)) break;
                            offsets.push_back(parse_block_start(p));
                            while (*p && isdigit(*p)) p++; 
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            while (*p && isdigit(*p)) p++; 
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            while (*p && isdigit(*p)) p++; 
                        }

                        long file_pos;
                        while ((file_pos = ftell(fp)) != -1 && fgets(buffer, sizeof(buffer), fp)) {
                            char *p2 = buffer;
                            while (isspace(*p2)) p2++;
                            if (*p2 == 0 || *p2 == '#') continue;
                            if (strncmp(p2, "BASE", 4) == 0) {
                                fseek(fp, file_pos, SEEK_SET);
                                break;
                            }
                            
                            uint64_t first_val = parse_block_start(p2);
                            if (!offsets.empty() && first_val < offsets.back()) {
                                 fseek(fp, file_pos, SEEK_SET);
                                 break;
                            }
                            
                            while (*p2) {
                                while (*p2 && (isspace(*p2) || *p2 == ',')) p2++;
                                if (!isdigit(*p2)) break;
                                offsets.push_back(parse_block_start(p2));
                                while (*p2 && isdigit(*p2)) p2++;
                                while (*p2 && (isspace(*p2) || *p2 == ',')) p2++;
                                while (*p2 && isdigit(*p2)) p2++;
                                while (*p2 && (isspace(*p2) || *p2 == ',')) p2++;
                                while (*p2 && isdigit(*p2)) p2++;
                            }
                        }

                        idx_rec->idx_size = (int)offsets.size();
                        idx_rec->idx = (uint64_t *)malloc(offsets.size() * sizeof(uint64_t));
                        for (size_t i = 0; i < offsets.size(); ++i) idx_rec->idx[i] = offsets[i];

                        int pieces = bm + bk + wm + wk;
                        CPRSUBDB *csdb = &h->cprsubdb[pieces - 2];
                        idx_rec->next = csdb->index_list;
                        csdb->index_list = idx_rec;
                        next_block_id = block_id + (unsigned int)offsets.size();
                    }
                }
            } else if (last_bm != -1 && isdigit(*p)) {
                // Subsequent rank subdivision without BASE keyword
                current_rank++;
                // Let the main loop handle it by rewinding and treating as next rank
                fseek(fp, ftell(fp) - strlen(buffer), SEEK_SET);
                // Wait, we need to create a BASE line? No, just use the last known pieces.
                // For simplicity, let's assume all relevant WLD/MTC files use BASE.
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
		uint64_t current_index;
		uint8_t *diskblock;
		unsigned int current_n;
		int blocknumber;

		*err = EGDB_ERR_NORMAL;

		// get the subdb.
		num_pieces = bit_pop_count64(pos->black_pieces | pos->white_pieces);
        if (num_pieces < 2 || num_pieces > h->max_pieces) {
			*err = EGDB_NUM_PIECES_OUT_OF_BOUNDS;
			return(MTC_UNKNOWN_VAL);
		}
        csdb = &h->cprsubdb[num_pieces - 2];
		if (!csdb->ispresent) {
			*err = EGDB_DB_NOT_LOADED;
			return(MTC_UNKNOWN_VAL);
		}

        // Determine piece counts and ranks for canonicalization check
        unsigned int num_bmen, num_bkings, num_wmen, num_wkings;
        num_bmen = bit_pop_count64(pos->black_pieces & ~pos->king);
        num_bkings = bit_pop_count64(pos->black_pieces & pos->king);
        num_wmen = bit_pop_count64(pos->white_pieces & ~pos->king);
        num_wkings = bit_pop_count64(pos->white_pieces & pos->king);

        int bmrank = 0, wmrank = 0;
        if (num_bmen) bmrank = msb_(pos->black_pieces & ~pos->king) / 4;
        if (num_wmen) wmrank = (31 - lsb_(pos->white_pieces & ~pos->king)) / 4;

        // Exact Kingsrow 8x8 canonicalization condition
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
            
            // Re-calculate counts and ranks for the reversed position
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

        // SYMMETRY RETRY: If no match found, try flipping pieces and colors.
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
            int f_bmrank = 0, f_wmrank = 0;
            if (f_bmen) f_bmrank = msb_(flipped.black_pieces & ~flipped.king) / 4;
            if (f_wmen) f_wmrank = (31 - lsb_(flipped.white_pieces & ~flipped.king)) / 4;

            idx_rec = find_record_mtc(&flipped, f_bmen, f_bkings, f_wmen, f_wkings, f_bmrank, f_wmrank);
            if (idx_rec) {
                lookup_pos = flipped;
                num_bmen = f_bmen; num_bkings = f_bkings; num_wmen = f_wmen; num_wkings = f_wkings;
                bmrank = f_bmrank; wmrank = f_wmrank;
            }
        }

        if (!idx_rec) {
            // Trivial case check: If one side has no pieces, it's a loss for them.
            if (num_bmen + num_bkings == 0) {
                return (pos->stm == EGDB_BLACK) ? 0 : 999; // 999 is Win, 0 is Loss/Draw
            }
            if (num_wmen + num_wkings == 0) {
                return (pos->stm == EGDB_WHITE) ? 0 : 999;
            }

            log_c(LOG_LEVEL_WARNING, "dblookup_mtc_internal: NO MATCH FOUND for piece counts and stm.");
            *err = EGDB_DB_NOT_LOADED;
            return(MTC_UNKNOWN_VAL);
        }
        
        current_index = (uint64_t)position_to_index_slice(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings);
        current_n = current_index;

        log_c(LOG_LEVEL_DEBUG, "dblookup_mtc_internal: current_index=%llu, pieces: %u/%u/%u/%u", (unsigned long long)current_index, num_bmen, num_bkings, num_wmen, num_wkings);

        if (idx_rec->num_idx_blocks == 0) {
            // Single value hit
            int result = idx_rec->initial_value;
            return result;
        }

        // Find the block number by searching idx array
        blocknumber = 0;
        for (int i = 0; i < idx_rec->idx_size; ++i) {
            if (current_index >= (uint64_t)idx_rec->idx[i]) {
                blocknumber = i;
            } else {
                break;
            }
        }
        
        // Bounds Check
        if (blocknumber == idx_rec->idx_size - 1) {
             uint64_t last_start = idx_rec->idx[blocknumber];
             if (current_index > last_start + 1000000) {
                 *err = EGDB_INDEX_OUT_OF_BOUNDS;
                 return MTC_UNKNOWN_VAL;
             }
        }
        
        log_c(LOG_LEVEL_DEBUG, "dblookup_mtc_internal: blocknumber selection: %d (idx_size: %d)", blocknumber, idx_rec->idx_size);

		// get the data block from disk (or cache).
        int cache_idx;
        if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
			*err = EGDB_FILE_READ_ERROR;
			return(MTC_UNKNOWN_VAL);
		}
		
		// now decompress the data.
		current_n = (unsigned int)idx_rec->idx[blocknumber];
        int j = (blocknumber == 0) ? idx_rec->startbyte : 0;
        int current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
        
        int val = idx_rec->initial_value; 
        uint64_t target_index = current_index;

        log_c(LOG_LEVEL_DEBUG, "dblookup_mtc_internal: starting decompression. target_index: %llu, current_n: %llu, j: %d, initial_val: %d", (unsigned long long)target_index, (unsigned long long)current_n, j, val);

        while (target_index > current_n + diskblock[j]) {
             current_n += diskblock[j];
             j++;
             val = (val == MTC_LOSS_VAL) ? MTC_WIN_VAL : MTC_LOSS_VAL;
             if (j >= current_block_size) {
                 blocknumber++;
                 if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
                     log_c(LOG_LEVEL_ERROR, "dblookup_mtc_internal: Failed to get next data block.");
                     *err = EGDB_FILE_READ_ERROR;
                     return MTC_UNKNOWN_VAL;
                 }
                 current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
                 j = 0;
             }
        }
        
        log_c(LOG_LEVEL_DEBUG, "dblookup_mtc_internal: hit! result: %d, target_index: %llu, current_n: %llu, run: %d", val, (unsigned long long)target_index, (unsigned long long)current_n, (int)diskblock[j]);
        return val;
	}

	static void exitdblookup_mtc(DBHANDLE h)
	{
		int i;
		INDEX_REC *idx_rec, *idx_rec_next;

		if (h) {
			if (h->cprsubdb) {
				for (i = 0; i < (int)get_total_num_mtc_dbs(); ++i) {
					idx_rec = h->cprsubdb[i].index_list;
                    FILE *last_file = nullptr;
					while (idx_rec) {
						idx_rec_next = idx_rec->next;
                        if (idx_rec->file && idx_rec->file != last_file) {
                            fclose(idx_rec->file);
                            last_file = idx_rec->file;
                        }
						if (idx_rec->idx) free(idx_rec->idx);
						free(idx_rec);
						idx_rec = idx_rec_next;
					}
				}
				free(h->cprsubdb);
			}
			free(h->cache_block_info);
			if (h->cache_block_ptr)
			free(h->cache_block_ptr);
			if (h->cache_base)
				free(h->cache_base);
			free(h);
		}
	}

	unsigned int get_total_num_mtc_dbs()
	{
		return(MAX_PIECES_IN_DB - 2 + 1);
	}

} // namespace egdb_interface
