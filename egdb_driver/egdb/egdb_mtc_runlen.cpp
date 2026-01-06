#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <dirent.h>
#include <algorithm>
#include <mutex>
#include <set>
#include <iostream>

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
    #undef MTC_SKIPS
    #undef MTC_DECODE
    const int MTC_SKIPS = 250;
    #define MTC_DECODE(x) ((x) >= MTC_SKIPS ? (x) - MTC_SKIPS : (x))

	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num);
	extern "C" int dblookup_mtc_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err);
	static DBHANDLE initdblookup_mtc(const char *egtb_dir, unsigned int cache_size_bytes);
	static void exitdblookup_mtc(DBHANDLE h);

	unsigned int get_total_num_mtc_dbs() { return MAX_PIECES_IN_DB - 2 + 1; }

	int egdb_close_mtc(EGDB_DRIVER *driver) {
		if (!driver) return(EGDB_INVALID_HANDLE);
		exitdblookup_mtc((DBHANDLE)driver->internal_data);
		free(driver); return(0);
	}

	unsigned int egdb_get_max_pieces_mtc(EGDB_DRIVER *driver) { return(driver->max_pieces); }

	unsigned int egdb_get_info_mtc(EGDB_DRIVER *driver, unsigned int num_pieces, EGDB_INFO *info, unsigned int max_info) {
		if (num_pieces < 2 || num_pieces > (unsigned int)driver->max_pieces) return(0);
		DBHANDLE h = (DBHANDLE)driver->internal_data; if (!h->cprsubdb[num_pieces - 2].ispresent) return(0);
		info->type = driver->db_type; info->num_pieces = num_pieces; info->compression = EGDB_COMPRESSION_RUNLEN;
		info->dtw_w_only = false; info->contains_le_pieces = true; return(1);
	}

    int egdb_mtc_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code) {
        EGDB_BITBOARD bb; bb.normal.black = pos->black_pieces; bb.normal.white = pos->white_pieces; bb.normal.king = pos->king;
        return driver->lookup(driver, &bb, pos->stm, 0);
    }

    int dblookup_mtc_wrapper(EGDB_DRIVER *driver, EGDB_BITBOARD *position, int color, int cl) {
        DBHANDLE h = (DBHANDLE)driver->internal_data;
        EGDB_POSITION pos;
        pos.black_pieces = position->normal.black; pos.white_pieces = position->normal.white; pos.king = position->normal.king; pos.stm = color;
        EGDB_ERR err; return dblookup_mtc_internal(h, &pos, &err);
    }

	EGDB_ERR egdb_open_mtc_runlen(EGDB_DRIVER *driver) {
		DBHANDLE h = initdblookup_mtc(driver->path, driver->cache_size); if (!h) return(EGDB_DB_NOT_LOADED);
		driver->internal_data = h; driver->lookup = dblookup_mtc_wrapper; driver->close = egdb_close_mtc;
		driver->max_pieces = h->max_pieces; return(EGDB_ERR_NORMAL);
	}

	static DBHANDLE initdblookup_mtc(char const *egtb_dir, unsigned int cache_size_bytes) {
		DBHANDLE h = (DBHANDLE)calloc(1, sizeof(struct DB_HANDLE_T)); if (!h) return(0);
		h->cache_mutex = new std::mutex();
		static bool init = false; if (!init) { init_compression_tables(); build_man_index_base(); init = true; }
		strncpy(h->path, egtb_dir, sizeof(h->path) - 1);
		if (cache_size_bytes < MAXCACHEDBLOCKS * 4096) cache_size_bytes = MAXCACHEDBLOCKS * 4096;
		h->cache_base = (uint8_t *)calloc(cache_size_bytes, 1); if (!h->cache_base) { delete h->cache_mutex; free(h); return(0); }
		h->cache_size = cache_size_bytes; h->cache_block_info = (BLOCK_INFO *)calloc(MAXCACHEDBLOCKS, sizeof(BLOCK_INFO));
		for (int i = 0; i < MAXCACHEDBLOCKS; ++i) { h->cache_block_info[i].prev = i - 1; h->cache_block_info[i].next = i + 1; h->cache_block_info[i].unique_id_64 = 0xFFFFFFFFFFFFFFFFULL; }
		h->cache_block_info[0].prev = -1; h->cache_block_info[MAXCACHEDBLOCKS - 1].next = -1; h->cache_head = HEAD; h->cache_tail = TAIL;
		unsigned int max_dbs = get_total_num_mtc_dbs(); h->cprsubdb = (CPRSUBDB *)calloc(max_dbs, sizeof(CPRSUBDB));
		int file_num = 0;
		for (int i = 0; i < (int)max_dbs; ++i) {
			int n_pcs = i + 2; CPRSUBDB *csdb = &h->cprsubdb[i]; csdb->num_pieces = n_pcs;
			DIR *dir = opendir(h->path); if (dir) {
				struct dirent *ent; std::string prefix = "db" + std::to_string(n_pcs);
				while ((ent = readdir(dir)) != nullptr) {
					std::string fname = ent->d_name;
					if (fname.size() >= prefix.size() + 8 && fname.compare(0, prefix.size(), prefix) == 0 && fname.find(".idx_mtc") != std::string::npos) {
						if (fname.size() > prefix.size() && isdigit(fname[prefix.size()]) && fname[prefix.size()] != '-') continue;
						if (parseindexfile_mtc(h, fname.c_str(), file_num) > 0) { csdb->ispresent = true; file_num++; }
					}
				}
				closedir(dir);
			}
			if (csdb->ispresent) h->max_pieces = n_pcs;
		}
		return h;
	}

	static unsigned int parseindexfile_mtc(DBHANDLE h, char const *idx_fname, int file_num) {
		char buffer[4096]; std::string full_idx = std::string(h->path) + "/" + idx_fname;
		FILE *fp = fopen(full_idx.c_str(), "r"); if (!fp) return 0;
		char cpr_fname[256]; strcpy(cpr_fname, idx_fname); char *ext = strstr(cpr_fname, ".idx_mtc"); if (ext) memcpy(ext, ".cpr_mtc", 8);
		std::string full_cpr = std::string(h->path) + "/" + cpr_fname; FILE *cpr_fp = fopen(full_cpr.c_str(), "rb");
		if (!cpr_fp) { fclose(fp); return 0; }
		unsigned int rec_count = 0;
		while (fgets(buffer, sizeof(buffer), fp)) {
			char *p = buffer; while (isspace(*p)) p++; if (*p == 0 || *p == '#') continue;
			if (strncmp(p, "BASE", 4) == 0) {
				p += 4; int bm, bk, wm, wk, rnk; char cchar; int n_rd = 0;
				if (sscanf(p, "%d,%d,%d,%d,%d,%c:%n", &bm, &bk, &wm, &wk, &rnk, &cchar, &n_rd) >= 6) {
					rec_count++; int stm = (cchar == 'b') ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE; p += n_rd;
					unsigned int b_id, s_byte; if (sscanf(p, "%u/%u%n", &b_id, &s_byte, &n_rd) == 2) {
						p += n_rd; INDEX_REC *idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
						idx_rec->num_bmen = bm; idx_rec->num_bkings = bk; idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
						idx_rec->side_to_move = stm; idx_rec->file_num = file_num; idx_rec->file = cpr_fp;
						if (bm > 0) idx_rec->bmrank = rnk; else idx_rec->wmrank = rnk;
						idx_rec->first_block_id = b_id; idx_rec->startbyte = s_byte;
						uint64_t f_off = (uint64_t)b_id * 4096 + s_byte; std::vector<Checkpoint> cps;
						auto get_n = [&](char **pr) -> int64_t {
							char *t = *pr; 
                            while (true) {
                                while (*t && !isdigit(*t) && *t != '-' && *t != '+') t++;
                                if (!*t) return -1; 
                                char *e; int64_t v = strtoll(t, &e, 10); 
                                if (e != t) { *pr = e; return v; }
                                t++;
                            }
						};
                        
                        auto get_n_multiline = [&](char **pr, FILE *f) -> int64_t {
                            int64_t val = get_n(pr);
                            if (val == -1) {
                                long fpos = ftell(f);
                                if (fgets(buffer, sizeof(buffer), f)) {
                                    *pr = buffer;
                                    val = get_n(pr);
                                    if (val == -1 || strncmp(*pr, "BASE", 4) == 0) { // Safety: Don't eat next BASE line
                                        fseek(f, fpos, SEEK_SET); // Rewind if not a number or is new record
                                        return -1;
                                    }
                                }
                            }
                            return val;
                        };

						int64_t s0 = get_n_multiline(&p, fp), v0 = get_n_multiline(&p, fp);
						if (s0 != -1 && v0 != -1) { 
                            f_off += s0; Checkpoint cp = {0, f_off, (uint8_t)v0}; cps.push_back(cp); 
                        }
                        
						while (true) {
							int64_t id = get_n_multiline(&p, fp); if (id == -1) break;
							int64_t sk = get_n_multiline(&p, fp), vl = get_n_multiline(&p, fp); if (sk == -1 || vl == -1) break;
							f_off += sk; Checkpoint cp = {(uint64_t)id, f_off, (uint8_t)vl}; cps.push_back(cp);
						}
						idx_rec->num_checkpoints = (int)cps.size();
						if (idx_rec->num_checkpoints > 0) {
							idx_rec->checkpoints = (Checkpoint *)malloc(cps.size() * sizeof(Checkpoint));
							for(size_t i=0; i<cps.size(); ++i) idx_rec->checkpoints[i] = cps[i];
						}
						CPRSUBDB *csdb = &h->cprsubdb[bm+bk+wm+wk-2]; idx_rec->next = csdb->index_list; csdb->index_list = idx_rec;
					}
				}
			}
		}
		fclose(fp); return rec_count;
	}

    static INDEX_REC* find_rec_mtc(CPRSUBDB *index_start, const EGDB_POSITION *p, int bmen, int bkings, int wmen, int wkings, int b0, int w0) {
        INDEX_REC *rec = index_start->index_list, *catch_all = nullptr;
        while (rec) {
            if (rec->num_bmen == bmen && rec->num_bkings == bkings && rec->num_wmen == wmen && rec->num_wkings == wkings && rec->side_to_move == p->stm) {
                if (bmen > 0) { if (rec->bmrank == b0) return rec; if (rec->bmrank == 0) catch_all = rec; }
                else if (wmen > 0) { if (rec->wmrank == w0) return rec; if (rec->wmrank == 0) catch_all = rec; }
                else return rec;
            }
            rec = rec->next;
        }
        return catch_all;
    }

	extern "C" int dblookup_mtc_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err) {
		log_c(LOG_LEVEL_DEBUG, "MTC: Lookup start. STM=%d", pos->stm);
		*err = EGDB_ERR_NORMAL; 
        auto my_pop = [](uint64_t b) { int c=0; while(b) { if(b&1) c++; b>>=1; } return c; };
        uint64_t bb_total = pos->black_pieces | pos->white_pieces;
        int n_pcs = my_pop(bb_total);
        log_c(LOG_LEVEL_DEBUG, "MTC: Pieces=%d Max=%d", n_pcs, h->max_pieces);

		if (n_pcs < 2 || n_pcs > h->max_pieces) { *err = EGDB_NUM_PIECES_OUT_OF_BOUNDS; return(MTC_UNKNOWN_VAL); }
		CPRSUBDB *csdb = &h->cprsubdb[n_pcs - 2]; if (!csdb->ispresent) { *err = EGDB_DB_NOT_LOADED; return(MTC_UNKNOWN_VAL); }
		bool rev = false;
		int bm = my_pop(pos->black_pieces & ~pos->king), bk = my_pop(pos->black_pieces & pos->king);
		int wm = my_pop(pos->white_pieces & ~pos->king), wk = my_pop(pos->white_pieces & pos->king);
		if ((wm+wk) > (bm+bk) || ((wm+wk) == (bm+bk) && (wk > bk || (wk == bk && pos->stm == EGDB_WHITE_TO_MOVE)))) rev = true;
		EGDB_POSITION l_pos = *pos;
		if (rev) {
			l_pos.black_pieces = pos->white_pieces; l_pos.white_pieces = pos->black_pieces; l_pos.king = pos->king;
			reverse_bitboard(&l_pos.black_pieces); reverse_bitboard(&l_pos.white_pieces); reverse_bitboard(&l_pos.king);
			l_pos.stm = (pos->stm == EGDB_WHITE_TO_MOVE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
		}
        
        log_c(LOG_LEVEL_DEBUG, "MTC: Initial rec search. Rev=%d", rev);
		int b0 = (l_pos.black_pieces & ~l_pos.king) ? msb64_(l_pos.black_pieces & ~l_pos.king)/4 : 0;
		int w0 = (l_pos.white_pieces & ~l_pos.king) ? (31 - lsb64_(l_pos.white_pieces & ~l_pos.king))/4 : 0;
		int bm_l = my_pop(l_pos.black_pieces & ~l_pos.king), bk_l = my_pop(l_pos.black_pieces & l_pos.king);
		int wm_l = my_pop(l_pos.white_pieces & ~l_pos.king), wk_l = my_pop(l_pos.white_pieces & l_pos.king);
		INDEX_REC *idx_rec = find_rec_mtc(csdb, &l_pos, bm_l, bk_l, wm_l, wk_l, b0, w0);
		if (!idx_rec) {
            log_c(LOG_LEVEL_DEBUG, "MTC: idx_rec not found. Trying reverse.");
			rev = !rev; l_pos = *pos;
			if (rev) {
				l_pos.black_pieces = pos->white_pieces; l_pos.white_pieces = pos->black_pieces; l_pos.king = pos->king;
				reverse_bitboard(&l_pos.black_pieces); reverse_bitboard(&l_pos.white_pieces); reverse_bitboard(&l_pos.king);
				l_pos.stm = (pos->stm == EGDB_WHITE_TO_MOVE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
			}
			b0 = (l_pos.black_pieces & ~l_pos.king) ? msb64_(l_pos.black_pieces & ~l_pos.king)/4 : 0;
			w0 = (l_pos.white_pieces & ~l_pos.king) ? (31 - lsb64_(l_pos.white_pieces & ~l_pos.king))/4 : 0;
			bm_l = my_pop(l_pos.black_pieces & ~l_pos.king); bk_l = my_pop(l_pos.black_pieces & l_pos.king);
			wm_l = my_pop(l_pos.white_pieces & ~l_pos.king); wk_l = my_pop(l_pos.white_pieces & l_pos.king);
			idx_rec = find_rec_mtc(csdb, &l_pos, bm_l, bk_l, wm_l, wk_l, b0, w0);
		}
		if (!idx_rec) { log_c(LOG_LEVEL_DEBUG, "MTC: Still no idx_rec. Out of bounds."); *err = EGDB_INDEX_OUT_OF_BOUNDS; return MTC_UNKNOWN_VAL; }
        
                // log_c(LOG_LEVEL_DEBUG, "MTC: idx_rec found. Calculating cur_idx.");
        		uint64_t cur_idx = (uint64_t)position_to_index_slice(&l_pos, idx_rec->num_bmen, idx_rec->num_bkings, idx_rec->num_wmen, idx_rec->num_wkings);
                // log_c(LOG_LEVEL_DEBUG, "MTC: cur_idx=%llu", cur_idx);
                
                // log_c(LOG_LEVEL_DEBUG, "MTC: Checking checkpoints. Num=%d Ptr=%p", idx_rec->num_checkpoints, (void*)idx_rec->checkpoints);
                if (idx_rec->num_checkpoints == 0 || idx_rec->checkpoints == nullptr) {
                     log_c(LOG_LEVEL_ERROR, "MTC: No checkpoints found! Num=%d Ptr=%p", idx_rec->num_checkpoints, (void*)idx_rec->checkpoints);
                     *err = EGDB_FILE_READ_ERROR; return MTC_UNKNOWN_VAL;
                }
		int cp_i = 0; 
        for (int i = 1; i < idx_rec->num_checkpoints; ++i) {
             // log_c(LOG_LEVEL_DEBUG, "MTC: Checkpoint %d index=%llu", i, idx_rec->checkpoints[i].index); // Careful printing this inside loop
             if (idx_rec->checkpoints[i].index <= cur_idx) cp_i = i; else break;
        }
        
        // log_c(LOG_LEVEL_DEBUG, "MTC: Selected Checkpoint %d", cp_i);
		Checkpoint& cp = idx_rec->checkpoints[cp_i]; uint64_t d_idx = cp.index, off = cp.file_offset;
		int b_num = (int)(off/4096), o = (int)(off%4096); uint8_t *db; int c_idx; 
        
        // log_c(LOG_LEVEL_DEBUG, "MTC: Reading data block. b_num=%d", b_num);
        if (get_db_data_block(h, idx_rec, b_num, &db, &c_idx)) { log_c(LOG_LEVEL_ERROR, "MTC: Read failed."); *err = EGDB_FILE_READ_ERROR; return MTC_UNKNOWN_VAL; }
        // log_c(LOG_LEVEL_DEBUG, "MTC: Read success.");

		int b_sz = h->cache_block_info[c_idx].bytes_in_block; int dist = MTC_DECODE(cp.initial_byte);
		while (d_idx <= cur_idx) {
			uint32_t run = 0;
			while (db[o] < MTC_SKIPS) {
				run += db[o]; o++;
				if (o >= b_sz) { 
                    if (b_sz < 4096) return dist; 
                    b_num++; 
                    if (get_db_data_block(h, idx_rec, b_num, &db, &c_idx)) return dist; 
                    b_sz = h->cache_block_info[c_idx].bytes_in_block; o = 0; 
                }
			}
			dist = MTC_DECODE(db[o]); if (cur_idx < d_idx + run + 1) break; d_idx += (run + 1); o++;
			if (o >= b_sz) { 
                if (b_sz < 4096) break; 
                b_num++; 
                if (get_db_data_block(h, idx_rec, b_num, &db, &c_idx)) break; 
                b_sz = h->cache_block_info[c_idx].bytes_in_block; o = 0; 
            }
		}
		// log_c(LOG_LEVEL_INFO, "MTC_INTERNAL_RES: result=%d", dist); 
        return dist;
	}

	static void exitdblookup_mtc(DBHANDLE h) {
		if (h) {
			std::set<FILE*> closed; if (h->cprsubdb) {
				for (int i = 0; i < (int)get_total_num_mtc_dbs(); ++i) {
					INDEX_REC *r = h->cprsubdb[i].index_list;
					while (r) { INDEX_REC *n = r->next; if (r->file && closed.find(r->file) == closed.end()) { fclose(r->file); closed.insert(r->file); } if (r->checkpoints) free(r->checkpoints); free(r); r = n; }
				}
				free(h->cprsubdb);
			}
			free(h->cache_block_info); if (h->cache_block_ptr) free(h->cache_block_ptr); if (h->cache_base) free(h->cache_base);
			if (h->cache_mutex) delete h->cache_mutex; free(h);
		}
	}

} // namespace egdb_interface