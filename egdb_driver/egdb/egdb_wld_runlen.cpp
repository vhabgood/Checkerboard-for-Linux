#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

#include "egdb/egdb_common.h"
#include "egdb/egdb_intl.h"
#include "egdb/crc.h"
#include "engine/bicoef.h"
#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/reverse.h"
#include "engine/bool.h"
#include "builddb/compression_tables.h"
#include "builddb/tunstall_decompress.h"
#include "builddb/indexing.h"
#include "egdb/db_io.h"
#include "log.h"

// for the lru cache for the database.
#define TAIL 0
#define HEAD (MAXCACHEDBLOCKS - 1)

namespace egdb_interface {

    extern int64_t man_index_base[MAXPIECE + 1][MAXPIECE + 1][ROWSIZE + 1];

    // forward references
    static unsigned int parseindexfile_wld(DBHANDLE h, char const *idx_fname, int file_num);
    static int dblookup_wld_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err);
    static DBHANDLE initdblookup(const char *egtb_dir, unsigned int cache_size_bytes);
    static void exitdblookup(DBHANDLE h);

    static unsigned int get_total_num_wld_dbs() {
         return MAX_PIECES_IN_DB - 2 + 1;
    }

    int egdb_close_wld(EGDB_DRIVER *driver)
    {
        if (!driver)
            return(EGDB_INVALID_HANDLE);

        exitdblookup((DBHANDLE)driver->internal_data);
        free(driver);
        return(0);
    }

    static int dblookup_wld_wrapper(EGDB_DRIVER *driver, EGDB_BITBOARD *position, int color, int cl)
    {
        DBHANDLE h = (DBHANDLE)driver->internal_data;
        EGDB_POSITION pos;
        pos.black_pieces = position->normal.black;
        pos.white_pieces = position->normal.white;
        pos.king = position->normal.king;
        pos.stm = color;
        EGDB_ERR err;
        return dblookup_wld_internal(h, &pos, &err);
    }

    int egdb_wld_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
    {
        EGDB_BITBOARD bitboard;
        bitboard.normal.black = pos->black_pieces;
        bitboard.normal.white = pos->white_pieces;
        bitboard.normal.king = pos->king;
        return driver->lookup(driver, &bitboard, pos->stm, 0);
    }

    unsigned int egdb_get_max_pieces_wld(EGDB_DRIVER *driver)
    {
        return(driver->max_pieces);
    }

    unsigned int egdb_get_info_wld(
        EGDB_DRIVER *driver,
        unsigned int num_pieces,
        EGDB_INFO *info,
        unsigned int max_info)
    {
        DBHANDLE h = (DBHANDLE)driver->internal_data;
        if (num_pieces < 2 || num_pieces > driver->max_pieces) return 0;
        if (!h->cprsubdb[num_pieces - 2].ispresent) return 0;

        info->type = driver->db_type;
        info->num_pieces = num_pieces;
        info->compression = h->cprsubdb[num_pieces - 2].compression;
        info->dtw_w_only = false;
        info->contains_le_pieces = true;
        return 1;
    }

    EGDB_ERR egdb_open_wld_runlen(EGDB_DRIVER *driver)
    {
        DBHANDLE h;
        char const *db_dir = driver->path;
        unsigned int cache_size_bytes = driver->cache_size;

        h = initdblookup(db_dir, cache_size_bytes);
        if (h == 0)
            return(EGDB_DB_NOT_LOADED);

        driver->internal_data = h;
        driver->lookup = dblookup_wld_wrapper;
        driver->close = egdb_close_wld;
        driver->max_pieces = h->max_pieces;
        return(EGDB_ERR_NORMAL);
    }

    static std::string egtb_dir_path(DBHANDLE h) {
         return h->path;
    }

#include <dirent.h>

    // ... (forward refs)

    static DBHANDLE initdblookup(char const *egtb_dir, unsigned int cache_size_bytes)
    {
        DBHANDLE h;
        int i;
        char fullpath[512];
        std::string s;
        unsigned int max_num_dbs;
        int file_num;
        CPRSUBDB *csdb;
        int num_dbs_loaded = 0;

        // allocate memory for the handle.
        h = (DBHANDLE)calloc(1, sizeof(struct DB_HANDLE_T));
        if (!h) return(0);

        // Ensure compression tables are initialized
        static bool tables_initialized = false;
        if (!tables_initialized) {
            init_compression_tables();
            tables_initialized = true;
        }

        strncpy(h->path, egtb_dir, sizeof(h->path) - 1);

        h->db_type = EGDB_WLD_RUNLEN;
        h->max_pieces = 0;
        h->compression_type = EGDB_COMPRESSION_RUNLEN;

        if (cache_size_bytes < MAXCACHEDBLOCKS * 4096) cache_size_bytes = MAXCACHEDBLOCKS * 4096;
        h->cache_base = (uint8_t *)calloc(cache_size_bytes, 1);
        if (!h->cache_base) {
            free(h);
            return(0);
        }
        h->cache_size = cache_size_bytes;

        h->cache_block_info = (BLOCK_INFO *)calloc(MAXCACHEDBLOCKS, sizeof(BLOCK_INFO));
        if (!h->cache_block_info) {
            free(h->cache_base); free(h);
            return(0);
        }

        for (i = 0; i < MAXCACHEDBLOCKS; ++i) {
            h->cache_block_info[i].prev = i - 1;
            h->cache_block_info[i].next = i + 1;
            h->cache_block_info[i].unique_id_64 = 0xFFFFFFFFFFFFFFFFULL;
        }
        h->cache_block_info[0].prev = -1;
        h->cache_block_info[MAXCACHEDBLOCKS - 1].next = -1;
        h->cache_head = HEAD;
        h->cache_tail = TAIL;

        max_num_dbs = get_total_num_wld_dbs();
        h->cprsubdb = (CPRSUBDB *)calloc(max_num_dbs, sizeof(CPRSUBDB));
        if (!h->cprsubdb) {
            free(h->cache_block_info); free(h->cache_base); free(h);
            return(0);
        }

        file_num = 0;
        		for (i = 0; i < (int)max_num_dbs; ++i) {
        			int n_pieces = i + 2;
        			csdb = &h->cprsubdb[i];
        
            csdb->db_type = EGDB_WLD_RUNLEN;
            csdb->compression = EGDB_COMPRESSION_RUNLEN;
            csdb->num_pieces = n_pieces;

            std::vector<std::string> idx_files;
            DIR *dir = opendir(h->path);
            if (dir) {
                struct dirent *ent;
                std::string prefix = "db" + std::to_string(n_pieces);
                while ((ent = readdir(dir)) != nullptr) {
                    std::string fname = ent->d_name;
                    bool is_idx = (fname.size() >= 4 && fname.compare(fname.size() - 4, 4, ".idx") == 0);
                    bool is_idx1 = (fname.size() >= 5 && fname.compare(fname.size() - 5, 5, ".idx1") == 0);

                    if (fname.size() >= prefix.size() &&
                        fname.compare(0, prefix.size(), prefix) == 0 &&
                        (is_idx || is_idx1)) {
                        
                        // Ensure it's not a different piece count (e.g. db10 doesn't match db1 prefix)
                        if (fname.size() > prefix.size() && isdigit(fname[prefix.size()]) && fname[prefix.size()] != '-') continue;

                        idx_files.push_back(fname);
                    }
                }
                closedir(dir);
            }

            for (const std::string& idx_fname_str : idx_files) {
                const char* idx_f = idx_fname_str.c_str();
                if (parseindexfile_wld(h, idx_f, file_num)) {
                    log_c(LOG_LEVEL_DEBUG, "initdblookup: parseindexfile_wld FAILED for %s", idx_f);
                    continue;
                }
                csdb->ispresent = true;
                if (idx_fname_str.find(".idx1") != std::string::npos) {
                    csdb->compression = EGDB_COMPRESSION_TUNSTALL_V2;
                } else {
                    csdb->compression = EGDB_COMPRESSION_TUNSTALL_V1;
                }
                num_dbs_loaded++;
                file_num++;
            }
            if (csdb->ispresent) h->max_pieces = n_pieces;
        }

        if (num_dbs_loaded == 0) {
            log_c(LOG_LEVEL_DEBUG, "initdblookup: NO DBs loaded.");
            exitdblookup(h);
            return(0);
        }
        log_c(LOG_LEVEL_DEBUG, "initdblookup: Total files loaded: %d. Max pieces: %d", num_dbs_loaded, h->max_pieces);
        return(h);
    }

    static unsigned int parseindexfile_wld(DBHANDLE h, char const *idx_fname, int file_num)
    {
        FILE *fp;
        char buffer[4096];
        char fullpath[512];
        std::string s;
        char *p;
        int current_rank = 0;
        int last_bm = -1, last_bk = -1, last_wm = -1, last_wk = -1;
        int last_color = -1;
        INDEX_REC *idx_rec = nullptr;

        s = egtb_dir_path(h); 
        s += "/";
        std::string base_path = s;
        s += idx_fname;
        strncpy(fullpath, s.c_str(), sizeof(fullpath) - 1);
        fp = fopen(fullpath, "r");
        if (!fp) return(1);

        // Determine corresponding CPR filename
        char cpr_fname[256];
        strcpy(cpr_fname, idx_fname);
        char *ext = strstr(cpr_fname, ".idx1");
        if (ext) memcpy(ext, ".cpr1", 5);
        else {
            ext = strstr(cpr_fname, ".idx");
            if (ext) memcpy(ext, ".cpr", 4);
        }
        
        std::string cpr_path = base_path + cpr_fname;
        FILE *cpr_fp = fopen(cpr_path.c_str(), "rb");
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
                int bm, bk, wm, wk;
                char colorchar;
                int rank;
                int n_read = 0;
                int items_parsed = sscanf(p, "%d,%d,%d,%d,%d,%c:%n", &bm, &bk, &wm, &wk, &rank, &colorchar, &n_read);
                if (items_parsed >= 6) {
                    last_bm = bm; last_bk = bk; last_wm = wm; last_wk = wk;
                    last_color = (colorchar == 'b') ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
                    current_rank = rank;
                    p += n_read;

                    if (*p == '+' || *p == '-' || *p == '=') {
                         idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                         idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                         idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                         idx_rec->side_to_move = last_color;
                         idx_rec->file_num = file_num;
                         if (bm > 0) idx_rec->bmrank = current_rank; else idx_rec->wmrank = current_rank;
                         idx_rec->initial_value = (*p == '+') ? EGDB_WIN : ((*p == '-') ? EGDB_LOSS : EGDB_DRAW);
                         
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
                        // log_c(LOG_LEVEL_DEBUG, "parseindexfile_wld: BASE %d,%d,%d,%d,%d STM %d. Start: %u/%u", bm, bk, wm, wk, rank, last_color, block_id, start_byte);
                        idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                        if (!idx_rec) {
                            fclose(fp);
                            return 1;
                        }
                        idx_rec->file_num = file_num;
                        idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                        idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                        idx_rec->side_to_move = last_color;
                        if (bm > 0) idx_rec->bmrank = current_rank; else idx_rec->wmrank = current_rank;
                        
                        // Absolute starting position: block_id is in 4096-byte units
                        uint64_t absolute_slice_start = (uint64_t)block_id * 4096 + start_byte;
                        idx_rec->first_block_id = 0; // We'll use absolute offsets in blocknumber
                        
                        std::vector<Checkpoint> checkpoints;
                        uint64_t current_file_offset = absolute_slice_start;

                        // Helper to parse numbers from current line and subsequent lines
                        auto get_next_num = [&](char **ptr_ref) -> int64_t {
                            char *ptr = *ptr_ref;
                            while (true) {
                                while (*ptr && !isdigit(*ptr) && *ptr != '-' && *ptr != '+' && *ptr != '#' && *ptr != 'B') ptr++;
                                if (!*ptr || *ptr == '#' || strncmp(ptr, "BASE", 4) == 0) return -1;
                                char *endptr;
                                int64_t val = strtoll(ptr, &endptr, 10);
                                if (endptr != ptr) {
                                    *ptr_ref = endptr;
                                    return val;
                                }
                                ptr++;
                            }
                        };

                        // The first two numbers after colon are BlockOffset and InitialByte for Index 0
                        int64_t skip0 = get_next_num(&p);
                        int64_t val0 = get_next_num(&p);
                        if (skip0 != -1 && val0 != -1) {
                            current_file_offset += (uint64_t)skip0;
                            checkpoints.push_back({0, current_file_offset, (uint8_t)val0});
                        }

                        // Collect all remaining numbers as triplets (Index, SkipBlocks, InitialByte)
                        while (true) {
                            int64_t idx = get_next_num(&p);
                            if (idx == -1) {
                                // Try next line
                                long fpos = ftell(fp);
                                if (fgets(buffer, sizeof(buffer), fp)) {
                                    p = buffer;
                                    idx = get_next_num(&p);
                                    if (idx == -1 || (!checkpoints.empty() && idx < (int64_t)checkpoints.back().index)) {
                                        fseek(fp, fpos, SEEK_SET);
                                        break;
                                    }
                                } else break;
                            }
                            int64_t skip = get_next_num(&p);
                            int64_t val = get_next_num(&p);
                            if (skip == -1 || val == -1) break;

                            current_file_offset += (uint64_t)skip;
                            checkpoints.push_back({(uint64_t)idx, current_file_offset, (uint8_t)val});
                        }

                        idx_rec->num_checkpoints = (int)checkpoints.size();
                        if (idx_rec->num_checkpoints > 0) {
                            idx_rec->checkpoints = (Checkpoint *)malloc(checkpoints.size() * sizeof(Checkpoint));
                            if (idx_rec->checkpoints) {
                                for(size_t i=0; i<checkpoints.size(); ++i) idx_rec->checkpoints[i] = checkpoints[i];
                            }
                        }
                        
                        int pieces = bm + bk + wm + wk;
                        CPRSUBDB *csdb = &h->cprsubdb[pieces - 2];
                        idx_rec->file = cpr_fp;
                        idx_rec->next = csdb->index_list;
                        csdb->index_list = idx_rec;
                    }
                }
            }
        }
        fclose(fp);
        return 0;
    }

    static int dblookup_wld_internal(DBHANDLE h, const EGDB_POSITION *pos, EGDB_ERR *err)
    {
        // printf("DEBUG: Entering dblookup_wld_internal\n"); fflush(stdout);
        CPRSUBDB *csdb;
        int num_pieces;
        INDEX_REC *idx_rec;
        uint8_t *diskblock;
        unsigned int num_bmen, num_bkings, num_wmen, num_wkings;
        uint64_t current_index;

        *err = EGDB_ERR_NORMAL;

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

        // Determine piece counts and ranks for canonicalization check
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
        }

        // Calculate ranks for slicing
        int bm_rank = 0, wm_rank = 0;
        if (num_bmen) bm_rank = msb_(lookup_pos.black_pieces & ~lookup_pos.king) / 4;
        if (num_wmen) wm_rank = (31 - lsb_(lookup_pos.white_pieces & ~lookup_pos.king)) / 4;

        auto find_record = [&](const EGDB_POSITION *p, int bmen, int bkings, int wmen, int wkings, int b0, int w0) -> INDEX_REC* {
            INDEX_REC *rec = csdb->index_list;
            INDEX_REC *catch_all = nullptr;
            while (rec) {
                if (rec->num_bmen == bmen && rec->num_bkings == bkings &&
                    rec->num_wmen == wmen && rec->num_wkings == wkings &&
                    rec->side_to_move == p->stm) {
                    
                    if (bmen > 0) {
                        if (rec->bmrank == b0) return rec;
                        if (rec->bmrank == 0) catch_all = rec;
                    } else if (wmen > 0) {
                        if (rec->wmrank == w0) return rec;
                        if (rec->wmrank == 0) catch_all = rec;
                    } else return rec;
                }
                rec = rec->next;
            }
            return catch_all;
        };

        idx_rec = find_record(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings, bm_rank, wm_rank);

        // SYMMETRY RETRY
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

            idx_rec = find_record(&flipped, f_bmen, f_bkings, f_wmen, f_wkings, f_bm_rank, f_wm_rank);
            if (idx_rec) {
                lookup_pos = flipped;
                num_bmen = f_bmen; num_bkings = f_bkings; num_wmen = f_wmen; num_wkings = f_wkings;
                bm_rank = f_bm_rank; wm_rank = f_wm_rank;
            }
        }

        if (!idx_rec) {
            if (num_bmen + num_bkings == 0) return (pos->stm == EGDB_BLACK) ? EGDB_LOSS : EGDB_WIN;
            if (num_wmen + num_wkings == 0) return (pos->stm == EGDB_WHITE) ? EGDB_LOSS : EGDB_WIN;
            *err = EGDB_DB_NOT_LOADED;
            return(EGDB_UNKNOWN);
        }
        
        current_index = (uint64_t)position_to_index_slice(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings);

        if (csdb->compression == EGDB_COMPRESSION_TUNSTALL_V1 || csdb->compression == EGDB_COMPRESSION_TUNSTALL_V2) {
            uint64_t absolute_file_offset = (uint64_t)idx_rec->first_block_id * 4096 + idx_rec->startbyte;
            int blocknumber = (int)(absolute_file_offset / 4096);
            int cache_idx;
            if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
                *err = EGDB_FILE_READ_ERROR;
                return(EGDB_UNKNOWN);
            }
            
            int value;
            if (csdb->compression == EGDB_COMPRESSION_TUNSTALL_V1) {
                decompress_wld_tunstall_v1(diskblock, h->cache_block_info[cache_idx].bytes_in_block, current_index, 0, &value);
            } else {
                decompress_wld_tunstall_v2(diskblock, h->cache_block_info[cache_idx].bytes_in_block, lsb64_(current_index), msb64_(current_index), &value);
            }
            
            if (value == 0) return EGDB_WIN;
            if (value == 1) return EGDB_LOSS;
            if (value == 2) return EGDB_DRAW;
            return EGDB_UNKNOWN;
        }

        // Brute force RLE logic for RUNLEN compression types
        if (idx_rec->num_checkpoints == 0) return idx_rec->initial_value;

        // Use the first checkpoint (index 0)
        Checkpoint& cp = idx_rec->checkpoints[0];
        uint64_t absolute_file_offset = cp.file_offset;
        uint64_t decompressed_index = 0;

        int blocknumber = (int)(absolute_file_offset / 4096);
        int offset = (int)(absolute_file_offset % 4096);

        int cache_idx;
        if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
            *err = EGDB_FILE_READ_ERROR;
            return(EGDB_UNKNOWN);
        }

        int current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
        int final_val = EGDB_UNKNOWN;
        bool found = false;

        while (decompressed_index <= current_index) {
             if (offset >= current_block_size) {
                 if (current_block_size < 4096) break;
                 blocknumber++;
                 if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) break;
                 current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
                 offset = 0;
             }

             uint8_t byte = diskblock[offset];
             int run;
             int val_base = 0;
             bool is_block = false;

             if (byte < 81) {
                  run = 4;
                  is_block = true;
             } else {
                  run = compression_tables.runlength_table[byte];
                  val_base = compression_tables.value_table[byte];
             }

             if (decompressed_index + run > current_index) {
                  if (is_block) {
                       int run_offset = (int)(current_index - decompressed_index);
                       final_val = compression_tables.decode_table[byte][run_offset];
                  } else {
                       final_val = val_base;
                  }
                  found = true;
                  break;
             }
             decompressed_index += run;
             offset++;
        }
        
        if (found) {
            if (final_val == 0) return EGDB_LOSS;
            if (final_val == 1) return EGDB_DRAW;
            if (final_val == 2) return EGDB_WIN;
            return EGDB_UNKNOWN;
        }

        // log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Brute force FAILED for %d,%d,%d,%d STM %d. Final Idx: %llu, Target: %llu", 
        //      num_bmen, num_bkings, num_wmen, num_wkings, lookup_pos.stm, (unsigned long long)decompressed_index, (unsigned long long)current_index);
        *err = EGDB_DECOMPRESSION_FAILED;
        return(EGDB_UNKNOWN);
    }


    static void exitdblookup(DBHANDLE h)
    {
        int i;
        INDEX_REC *idx_rec, *idx_rec_next;

        if (h) {
            if (h->cprsubdb) {
                for (i = 0; i < (int)get_total_num_wld_dbs(); ++i) {
                    // Close the main file pointer if it exists
                    if (h->cprsubdb[i].file) {
                        fclose(h->cprsubdb[i].file);
                        h->cprsubdb[i].file = nullptr;
                    }
                    
                    idx_rec = h->cprsubdb[i].index_list;
                    FILE *last_closed = nullptr;
                    while (idx_rec) {
                        idx_rec_next = idx_rec->next;
                        // Close unique slice file pointers
                        if (idx_rec->file && idx_rec->file != last_closed) {
                            // Check if this file pointer is already closed by main pointer
                            bool already_closed = false;
                            for (int j=0; j<(int)get_total_num_wld_dbs(); ++j) {
                                if (h->cprsubdb[j].file == idx_rec->file) already_closed = true;
                            }
                            if (!already_closed) {
                                fclose(idx_rec->file);
                                last_closed = idx_rec->file;
                            }
                        }
                        if (idx_rec->checkpoints) free(idx_rec->checkpoints);
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

}   // namespace egdb_interface