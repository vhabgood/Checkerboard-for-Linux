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

    EGDB_ERR egdb_close_wld(EGDB_DRIVER *driver)
    {
        if (!driver)
            return(EGDB_INVALID_HANDLE);

        exitdblookup((DBHANDLE)driver->internal_data);
        free(driver);
        return(EGDB_ERR_NORMAL);
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

    int egdb_wld_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code)
    {
        // printf("DEBUG: Entering egdb_wld_lookup\n"); fflush(stdout);
        DBHANDLE h;
        int value;

        h = (DBHANDLE)driver->internal_data;

        // check if num_pieces is too large for this db.
        if (bit_pop_count64(pos->black_pieces | pos->white_pieces) > h->max_pieces) {
            *err_code = EGDB_NUM_PIECES_TOO_LARGE;
            return(EGDB_UNKNOWN);
        }

        EGDB_POSITION local_pos = *pos;
        if (driver->bitboard_type == EGDB_ROW_REVERSED) {
            local_pos.black_pieces = convert_cake_to_kingsrow(pos->black_pieces);
            local_pos.white_pieces = convert_cake_to_kingsrow(pos->white_pieces);
            local_pos.king = convert_cake_to_kingsrow(pos->king);
        }

        value = dblookup_wld_internal(h, &local_pos, err_code);
        if (*err_code != EGDB_ERR_NORMAL)
            return(EGDB_UNKNOWN);

        return(value);
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
            h->cache_block_info[i].unique_id = -1;
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
                    if (fname.size() >= prefix.size() + 4 &&
                        fname.compare(0, prefix.size(), prefix) == 0 &&
                        fname.compare(fname.size() - 4, 4, ".idx") == 0) {
                        
                        // Ensure it's not a different piece count (e.g. db10 doesn't match db1 prefix)
                        if (fname.size() > prefix.size() && isdigit(fname[prefix.size()])) continue;

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
        char buffer[1024];
        char fullpath[512];
        std::string s;
        char *p;
        int current_rank = 0;
        int last_bm = 0, last_bk = 0, last_wm = 0, last_wk = 0;
        int last_color = 0;
        CPRSUBDB *csdb;
        INDEX_REC *idx_rec;
        unsigned int n_read;
        unsigned int next_block_id = 0;

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
        char *dot = strrchr(cpr_fname, '.');
        if (dot) strcpy(dot, ".cpr");
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
                         idx_rec->idx_size = 0;
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
                        idx_rec = (INDEX_REC *)calloc(1, sizeof(INDEX_REC));
                        idx_rec->file_num = file_num;
                        idx_rec->num_bmen = bm; idx_rec->num_bkings = bk;
                        idx_rec->num_wmen = wm; idx_rec->num_wkings = wk;
                        idx_rec->side_to_move = last_color;
                        if (bm > 0) idx_rec->bmrank = current_rank; else idx_rec->wmrank = current_rank;
                        
                        // Normalize offsets: If start_byte >= 1024, shift it into block_id
                        idx_rec->first_block_id = block_id + (start_byte / 1024);
                        idx_rec->startbyte = start_byte % 1024;
                        
                        std::vector<uint64_t> offsets;
                        offsets.push_back(0); // Block 0 starts at relative index 0
                        
                        auto parse_block_start = [&](char *ptr) {
                            char *endptr;
                            uint64_t val = strtoull(ptr, &endptr, 10);
                            if (endptr != ptr) return (uint64_t)val;
                            return (uint64_t)0ULL;
                        };

                        // Parse rest of the BASE line
                        while (*p) {
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            if (!isdigit(*p)) break;
                            offsets.push_back(parse_block_start(p));
                            while (*p && isdigit(*p)) p++; // Skip digits
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            while (*p && isdigit(*p)) p++; // Skip start_byte
                            while (*p && (isspace(*p) || *p == ',')) p++;
                            while (*p && isdigit(*p)) p++; // Skip checkpoint
                        }

                        // Continue reading continuation lines
                        long file_pos;
                        while ((file_pos = ftell(fp)) != -1 && fgets(buffer, sizeof(buffer), fp)) {
                            char *p2 = buffer;
                            while (isspace(*p2)) p2++;
                            if (*p2 == 0 || *p2 == '#') continue;
                            if (strncmp(p2, "BASE", 4) == 0) {
                                fseek(fp, file_pos, SEEK_SET);
                                break;
                            }
                            
                            // Check if this is a new rank (non-monotonic index)
                            uint64_t first_val = parse_block_start(p2);
                            if (!offsets.empty() && first_val < offsets.back()) {
                                 // Likely a rank change.
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
                        idx_rec->num_idx_blocks = (int)offsets.size();
                        idx_rec->idx = (uint64_t *)malloc(offsets.size() * sizeof(uint64_t));
                        for(size_t i=0; i<offsets.size(); ++i) idx_rec->idx[i] = offsets[i];
                        
                        int pieces = bm + bk + wm + wk;
                        CPRSUBDB *csdb = &h->cprsubdb[pieces - 2];
                        idx_rec->file = cpr_fp;
                        idx_rec->next = csdb->index_list;
                        csdb->index_list = idx_rec;
                        next_block_id = block_id + (unsigned int)offsets.size();
                    }
                }
            } else if (last_bm != -1 && isdigit(*p)) {
                 // Implicit next rank
                 current_rank++;
                 // We'll recurse or loop to create a new INDEX_REC? 
                 // Actually, if we hit this, we should rewind and let the main loop handle it?
                 // No, the main loop expects "BASE". 
                 // Let's implement the same BASE-less logic as MTC.
                 // Actually, for WLD, BASE-less lines seem rare. 
                 // If we hit one, let's just skip for now or treat as BASE.
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
        int value;
        INDEX_REC *idx_rec;
        uint64_t current_index, current_n;
        uint8_t *diskblock;
        unsigned int db_len_bytes;
        int blocknumber = 0;
        unsigned int num_bmen, num_bkings, num_wmen, num_wkings;

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

        // SYMMETRY RETRY: If no match found, try flipping pieces and colors.
        // Many database slices are only stored from one perspective (canonicalization).
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
            }
        }

        if (!idx_rec) {
            // Trivial case check: If one side has no pieces, it's a loss for them.
            if (num_bmen + num_bkings == 0) {
                return (pos->stm == EGDB_BLACK) ? EGDB_LOSS : EGDB_WIN;
            }
            if (num_wmen + num_wkings == 0) {
                return (pos->stm == EGDB_WHITE) ? EGDB_LOSS : EGDB_WIN;
            }

            if (num_pieces <= 7) {
                log_c(LOG_LEVEL_WARNING, "dblookup_wld_internal: NO MATCH FOUND for piece counts and stm.");
            }
            *err = EGDB_DB_NOT_LOADED;
            return(EGDB_UNKNOWN);
        }
        
        // Handle single-value hits
        if (idx_rec->num_idx_blocks == 0) {
             int res = idx_rec->initial_value;
             if (lookup_pos.stm != pos->stm) {
                 if (res == EGDB_WIN) res = EGDB_LOSS;
                 else if (res == EGDB_LOSS) res = EGDB_WIN;
             }
             return res;
        }
        
        current_index = (uint64_t)position_to_index_slice(&lookup_pos, (int)num_bmen, (int)num_bkings, (int)num_wmen, (int)num_wkings);
        current_n = current_index;

        // Find block
        // idx_rec->idx is array of start_indices.
        // We need k where idx[k] <= current_index < idx[k+1]
        // Binary search or linear.
        int best_block = -1;
        
        for (int k = 0; k < idx_rec->num_idx_blocks; ++k) {
             if (idx_rec->idx[k] <= current_index) {
                 best_block = k;
             } else {
                 break;
             }
        }
        
        // Sanity Check: If current_index is wildly beyond the start of the last block,
        // it means we are using a catch-all index for a position that resides in a missing slice.
        // A single 4KB block can hold at most ~262k positions (assuming extremely high compression).
        // We use a safe margin of 1,000,000.
        if (best_block == idx_rec->num_idx_blocks - 1) {
            uint64_t last_start = idx_rec->idx[best_block];
            if (current_index > last_start + 1000000) {
                 // log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Index %llu too far past last block start %llu", (unsigned long long)current_index, (unsigned long long)last_start);
                 *err = EGDB_INDEX_OUT_OF_BOUNDS;
                 return EGDB_UNKNOWN;
            }
        }
        
        if (best_block == -1) {
             *err = EGDB_INDEX_OUT_OF_BOUNDS;
             return EGDB_UNKNOWN;
        }
        
        blocknumber = best_block;
        current_n = idx_rec->idx[blocknumber]; // Start index of this block

        // Handle Small File Packing: If the file is smaller than 1024 bytes,
        // it only has block 0. All index checkpoints must be in block 0.
        if (idx_rec->file) {
            long current_fpos = ftell(idx_rec->file);
            fseek(idx_rec->file, 0, SEEK_END);
            long file_size = ftell(idx_rec->file);
            fseek(idx_rec->file, current_fpos, SEEK_SET);
            if (file_size < 1024) {
                blocknumber = 0;
                current_n = idx_rec->idx[0];
            }
        }

        int cache_idx;
        if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
            *err = EGDB_FILE_READ_ERROR;
            return(EGDB_UNKNOWN);
        }

        // Decompress
        int offset = (blocknumber == 0) ? idx_rec->startbyte : 0;
        uint64_t decompressed_index = current_n;
        int current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
        
        // log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Decompressing. Target: %llu, Start: %llu, Block: %d, Offset: %d", (unsigned long long)current_index, (unsigned long long)decompressed_index, blocknumber, offset);
        
        int loop_safety = 0;
        // log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Starting decompression loop. Target: %llu, Start: %llu, Block: %d, Offset: %d", (unsigned long long)current_index, (unsigned long long)decompressed_index, blocknumber, offset);
        while (decompressed_index <= current_index) {
             if (loop_safety++ > 5000000) {
                 log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Loop safety break at Idx: %llu", (unsigned long long)decompressed_index);
                 break; 
             }
             
             uint8_t byte = diskblock[offset];
             int run = compression_tables.runlength_table[byte];
             int val_base = compression_tables.value_table[byte];
             
             /*
             if (loop_safety < 10) {
                 log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Loop %d: Byte=%d Run=%d ValBase=%d Idx=%llu", loop_safety, byte, run, val_base, (unsigned long long)decompressed_index);
             }
             */

             // Check if we found it
             if (decompressed_index + run > current_index) {
                  // Found!
                  int final_val;
                  if (byte < 81) {
                       int run_offset = (int)(current_index - decompressed_index);
                       final_val = compression_tables.decode_table[byte][run_offset];
                  } else {
                       final_val = val_base;
                  }
                  
                  // log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Hit! Val=%d at Idx=%llu", final_val, (unsigned long long)current_index);

                  // Standard Kingsrow Mapping (0=LOSS, 1=DRAW, 2=WIN)
                  int res = EGDB_UNKNOWN;
                  if (final_val == 0) res = EGDB_LOSS; 
                  else if (final_val == 1) res = EGDB_DRAW; 
                  else if (final_val == 2) res = EGDB_WIN;  
                  
                  return res;
             }
             
             decompressed_index += run;
             offset++;
             if (offset >= current_block_size) {
                 if (current_block_size < 1024) {
                      log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: EOF reached in short block %d at Idx: %llu", blocknumber, (unsigned long long)decompressed_index);
                      break;
                 }
                 blocknumber++;
                 if (get_db_data_block(h, idx_rec, blocknumber, &diskblock, &cache_idx)) {
                     log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Failed to get next block %d", blocknumber);
                     *err = EGDB_FILE_READ_ERROR;
                     return EGDB_UNKNOWN;
                 }
                 current_block_size = h->cache_block_info[cache_idx].bytes_in_block;
                 offset = 0;
             }
        }
        
        log_c(LOG_LEVEL_DEBUG, "dblookup_wld_internal: Decompression FAILED. Final Idx: %llu", (unsigned long long)decompressed_index);

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

}   // namespace egdb_interface