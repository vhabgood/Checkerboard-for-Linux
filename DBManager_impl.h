#pragma once
#include "DBManager.h"
#include "core_types.h"
#include <type_traits>
#include "log.h"

template <typename T>
bool DBManager::parse_single_base_entry_generic(char **ptr_ref, int blockoffset, int fpcount, int* total_blocks_ptr, char* file_content) {
    char *ptr = *ptr_ref;

    // Use a temporary pointer to scan past leading whitespace without modifying the original pointer yet.
    char *temp_ptr = ptr;
    while (*temp_ptr == ' ' || *temp_ptr == '\t') {
        temp_ptr++;
    }

    // Check for "BASE" at the new position.
    if (temp_ptr[0] != 'B' || temp_ptr[1] != 'A' || temp_ptr[2] != 'S' || temp_ptr[3] != 'E') {
        return false; // Not a "BASE" line. The original ptr_ref is unmodified, so the caller's line-skipping logic remains correct.
    }

    // If we found "BASE", update the main pointer to this new position.
    ptr = temp_ptr;

    ptr += 4; // Consume "BASE"
    int bm, bk, wm, wk, bmrank, wmrank, color;
    char colorchar;
    int n_read = 0;
    int stat = sscanf(ptr, "%i,%i,%i,%i,%i,%i,%c:%n", &bm, &bk, &wm, &wk, &bmrank, &wmrank, &colorchar, &n_read);
    
    if (stat < 7) {
        return false; // Malformed line
    }

    // Bounds checking to prevent heap corruption
    if (bm < 0 || bm > MAXPIECE || bk < 0 || bk > MAXPIECE ||
        wm < 0 || wm > MAXPIECE || wk < 0 || wk > MAXPIECE ||
        bmrank < 0 || bmrank >= MAX_RANK_COUNT ||
        wmrank < 0 || wmrank >= MAX_RANK_COUNT) {
        log_c(LOG_LEVEL_ERROR, "Error parsing index file: index out of bounds. bm=%d, bk=%d, wm=%d, wk=%d, bmrank=%d, wmrank=%d", bm, bk, wm, wk, bmrank, wmrank);
        return false; // Malformed or malicious line
    }

    ptr += n_read;

    color = (colorchar == 'b') ? DB_BLACK : DB_WHITE;
    T *dbpointer;
    if constexpr (std::is_same<T, cprsubdb>::value) {
        dbpointer = &cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color];
    } else {
        dbpointer = &cprsubdatabase_mtc[bm][bk][wm][wk][bmrank][wmrank][color];
    }
    
    dbpointer->haspartials = 0;

    if (isdigit(*ptr)) {
        char *next_ptr;
        int firstblock = strtol(ptr, &next_ptr, 10);
        if (*next_ptr != '/') {
            return false; // Malformed line
        }
        ptr = next_ptr + 1;
        int startbyte = strtol(ptr, &next_ptr, 10);
        ptr = next_ptr;

        dbpointer->first_block_id = firstblock;
        dbpointer->blockoffset = blockoffset;
        dbpointer->ispresent = 1;
        dbpointer->value = 0;
        dbpointer->startbyte = startbyte;
        dbpointer->fp = fpcount;
        
        int current_idx_count = 1; 
        char *temp_ptr = ptr;
        while (*temp_ptr) {
            while (*temp_ptr && isspace(*temp_ptr)) temp_ptr++;
            if (!*temp_ptr || !isdigit(*temp_ptr)) break;
            strtol(temp_ptr, &next_ptr, 10);
            temp_ptr = next_ptr;
            current_idx_count++;
        }

        int db_blocks_count = 0;
        if (firstblock != -1) { 
            db_blocks_count = (current_idx_count == 1) ? 1 : current_idx_count - 1;
        }
        dbpointer->num_blocks = db_blocks_count;
        dbpointer->idx_size = current_idx_count;
        dbpointer->idx = (int*)malloc(current_idx_count * sizeof(int));
        if (dbpointer->idx == nullptr) {
            log_c(LOG_LEVEL_FATAL, "Failed to allocate memory for EGDB index.");
            return false;
        }
        bytesallocated += current_idx_count * sizeof(int);

        int num = 1;
        dbpointer->idx[0] = 0; 
        while (*ptr) {
            while (*ptr && isspace(*ptr)) ptr++;
            if (!*ptr || !isdigit(*ptr)) break;
            
            dbpointer->idx[num] = strtol(ptr, &next_ptr, 10);
            ptr = next_ptr;
            num++;
            if (num >= current_idx_count) break; 
        }

        if (firstblock + current_idx_count > *total_blocks_ptr) { 
            *total_blocks_ptr = firstblock + current_idx_count;
        }

    } else {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            int singlevalue;
            switch (*ptr) {
                case '+': singlevalue = DB_WIN; break;
                case '=': singlevalue = DB_DRAW; break;
                case '-': singlevalue = DB_LOSS; break;
                default: singlevalue = DB_UNKNOWN; break;
            }
            dbpointer->blockoffset = 0;
            dbpointer->first_block_id = 0;
            dbpointer->ispresent = 1;
            dbpointer->num_blocks = 0;
            dbpointer->value = singlevalue;
            dbpointer->fp = fpcount;
            ptr++;
        } else {
            dbpointer->ispresent = 1;
            dbpointer->value = strtol(ptr, &ptr, 10);
        }
    }
    
    // On success, update the caller's pointer
    *ptr_ref = ptr; 
    return true;
}

template <typename T>
int DBManager::parseindexfile_generic(const char* EGTBdirectory, char idxfilename[256], int blockoffset, int fpcount) {
    char fullpath[MAX_PATH_FIXED];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, idxfilename);
    
    long fsize = 0;
    char *file_content = read_entire_file(fullpath, &fsize);
    if (file_content == nullptr) {
        updateProgramStatusWord(STATUS_FILE_IO_ERROR | STATUS_EGDB_INIT_FAIL);
        return -1;
    }

    char *ptr = file_content;
    int total_blocks = 0;

    while (*ptr) {
        char* current_line_start = ptr;
        if (parse_single_base_entry_generic<T>(&ptr, blockoffset, fpcount, &total_blocks, file_content)) {
            // Success, ptr was advanced by the callee. Loop continues.
        } else {
            // Failure, not a BASE line or malformed. Advance to next line.
            char* next_line = strchr(current_line_start, '\n');
            if (next_line) {
                ptr = next_line + 1;
            } else {
                // No more newlines, must be end of file.
                break;
            }
        }
    }

    free(file_content);
    return total_blocks;
}

template <typename T>
bool DBManager::processEgdbFiles_generic(const char* EGTBdirectory, int nPieces, int bm, int bk, int wm, int wk, int& blockOffset, int& cprFileCount, char* outBuffer) {
    char cpr_dbname[256], idx_dbname[256], fullpath[MAX_PATH_FIXED];
    FILE *fp_cpr = nullptr;
    int pifReturn = -1;

    const char* cpr_suffix = "";
    const char* idx_suffix = "";
    const char* db_type_str = "";
    FILE** dbfp_array = nullptr;
    char (*dbnames_array)[256] = nullptr;

    if constexpr (std::is_same<T, cprsubdb>::value) {
        cpr_suffix = ".cpr";
        idx_suffix = ".idx";
        db_type_str = "WLD";
        dbfp_array = dbfp;
        dbnames_array = dbnames;
    } else {
        cpr_suffix = ".cpr_mtc";
        idx_suffix = ".idx_mtc";
        db_type_str = "MTC";
        dbfp_array = dbfp_mtc;
        dbnames_array = dbnames_mtc;
    }

    if (nPieces < SPLITSIZE) {
        snprintf(cpr_dbname, sizeof(cpr_dbname), "db%i%s", nPieces, cpr_suffix);
        snprintf(idx_dbname, sizeof(idx_dbname), "db%i%s", nPieces, idx_suffix);
    } else {
        snprintf(cpr_dbname, sizeof(cpr_dbname), "db%i_%i%i%i%i%s", nPieces, bm, bk, wm, wk, cpr_suffix);
        snprintf(idx_dbname, sizeof(idx_dbname), "db%i_%i%i%i%i%s", nPieces, bm, bk, wm, wk, idx_suffix);
    }
    
    snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, cpr_dbname);

    fp_cpr = fopen(fullpath, "rb");
    if (fp_cpr) {
        if (cprFileCount < MAXFP) {
            dbfp_array[cprFileCount] = fp_cpr;
            sprintf(dbnames_array[cprFileCount], "%s", fullpath);
            pifReturn = parseindexfile_generic<T>(EGTBdirectory, idx_dbname, blockOffset, cprFileCount);

            if (pifReturn >= 0) {
                blockOffset += pifReturn;
                cprFileCount++;
                return true;
            } else {
                fclose(fp_cpr);
                dbfp_array[cprFileCount] = nullptr;
                return false;
            }
        } else {
            log_c(LOG_LEVEL_WARNING, "MAXFP limit reached, cannot open more %s CPR files.", db_type_str);
            fclose(fp_cpr);
            return false;
        }
    } else {
        return false;
    }
}

template <typename T>
unsigned char* DBManager::get_disk_block_generic(int uniqueblockid, T* dbpointer, int blocknumber)
{
    unsigned char* diskblock;
    int newhead;

    if (blockpointer[uniqueblockid] != nullptr) {
        diskblock = blockpointer[uniqueblockid];
        newhead = (diskblock - cachebaseaddress) >> 10;
        move_cache_block_to_head(newhead);
    } else { 
        if (tail == -1) {
            if constexpr (std::is_same<T, cprsubdb>::value) {
                log_c(LOG_LEVEL_ERROR, "dblookup: cache is full, tail is -1.");
                updateProgramStatusWord(STATUS_CRITICAL_ERROR);
            }
            return nullptr;
        }

        diskblock = cachebaseaddress + (tail << 10);
        blockpointer[uniqueblockid] = diskblock;
        if (blockinfo[tail].uniqueid != -1) {
            blockpointer[blockinfo[tail].uniqueid] = nullptr;
        }

        newhead = tail;
        tail = blockinfo[tail].prev;
        if (tail != -1) {
            blockinfo[tail].next = -1;
        }
        blockinfo[newhead].uniqueid = uniqueblockid;
        blockinfo[newhead].next = head;
        blockinfo[newhead].prev = -1;

        if (head != -1) {
            blockinfo[head].prev = newhead;
        }
        head = newhead;

        FILE** dbfp_array = nullptr;
        if constexpr (std::is_same<T, cprsubdb>::value) {
            dbfp_array = dbfp;
        } else {
            dbfp_array = dbfp_mtc;
        }

        if (dbfp_array[dbpointer->fp] == nullptr) {
            return nullptr;
        }
        int fseek_result = fseek(dbfp_array[dbpointer->fp], (blocknumber + dbpointer->first_block_id) << 10, SEEK_SET);
        if (fseek_result != 0) {
            if constexpr (std::is_same<T, cprsubdb>::value) {
                log_c(LOG_LEVEL_ERROR, "dblookup: fseek failed.");
                updateProgramStatusWord(STATUS_EGDB_LOOKUP_MISS | STATUS_EGDB_DISK_READ_ERROR);
            }
            return nullptr;
        }
        
        size_t itemsRead = fread(diskblock, 1024, 1, dbfp_array[dbpointer->fp]);
        if (itemsRead != 1) {
            if constexpr (std::is_same<T, cprsubdb>::value) {
                log_c(LOG_LEVEL_ERROR, "dblookup: fread failed to read a full block.");
                updateProgramStatusWord(STATUS_EGDB_LOOKUP_MISS | STATUS_EGDB_DISK_READ_ERROR);
            }
            blockpointer[uniqueblockid] = nullptr;
            return nullptr;
        }
    }
    return diskblock;
}

template <typename T>
int DBManager::decode_value_generic(unsigned char* diskblock, uint32_t index, T* dbpointer, int blocknumber)
{
    int i = -1;
    int n = 0;
    int returnvalue = DB_UNKNOWN;
    unsigned char byte;
    bool reverse = false;
    auto& idx = dbpointer->idx;

    if (dbpointer->num_blocks > blocknumber + 1) {
        if (idx[blocknumber + 1] - index < index - idx[blocknumber]) {
            reverse = true;
        }
    }

    if (reverse) {
        n = idx[blocknumber + 1];
        for (i = 1023; i >= 0; --i) {
            if ((unsigned int)n <= index) break;
            n -= runlength[diskblock[i]];
        }
        if (i < 1023) i++;
    } else {
        n = idx[blocknumber];
        i = (blocknumber == 0) ? dbpointer->startbyte : 0;
        
        while ((unsigned int)n <= index) {
            int current_runlength = runlength[diskblock[i]];
            if ((unsigned int)(n + current_runlength) > index) {
                break;
            }
            n += current_runlength;
            i++;
            if (i >= 1024) { 
                blocknumber++;
                int new_uniqueblockid = dbpointer->blockoffset + dbpointer->first_block_id + blocknumber;
                diskblock = get_disk_block_generic<T>(new_uniqueblockid, dbpointer, blocknumber);
                if (diskblock == nullptr) {
                    if constexpr (std::is_same<T, cprsubdb>::value) {
                        updateProgramStatusWord(STATUS_EGDB_DECODE_ERROR);
                    }
                    return DB_UNKNOWN;
                }
                i = 0;
            }
        }
    }

    if (i < 0 || i >= 1024) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_DECODE_ERROR);
        }
        return DB_UNKNOWN;
    }

    byte = diskblock[i];
    if (byte > 80) {
        returnvalue = value[byte];
    } else {
        int val_idx = index - n;
        if (val_idx >= 0 && val_idx < 4) {
            returnvalue = decode_table[byte][val_idx];
        } else {
            if constexpr (std::is_same<T, cprsubdb>::value) {
                updateProgramStatusWord(STATUS_EGDB_DECODE_ERROR);
            }
            return DB_UNKNOWN;
        }
    }
    
    if constexpr (std::is_same<T, cprsubdb>::value) {
        // Log the unexpected value
        if (returnvalue == 3) { // raw value 3 becomes 4 after increment
             updateProgramStatusWord(STATUS_EGDB_UNEXPECTED_VALUE);
        }
        returnvalue++; 
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_HIT); 
    }

    return returnvalue;
}

template <typename T>
int DBManager::dblookup_generic(bitboard_pos *q)
{
    QMutexLocker locker(&m_mutex);
    
    if constexpr (std::is_same<T, cprsubdb>::value) {
        clearProgramStatusWordFlags(STATUS_EGDB_LOOKUP_HIT | STATUS_EGDB_LOOKUP_MISS |
                                    STATUS_EGDB_WIN_RESULT | STATUS_EGDB_LOSS_RESULT |
                                    STATUS_EGDB_DRAW_RESULT | STATUS_EGDB_UNKNOWN_RESULT |
                                    STATUS_EGDB_UNEXPECTED_VALUE | STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS |
                                    STATUS_EGDB_LOOKUP_NOT_PRESENT | STATUS_EGDB_LOOKUP_INVALID_INDEX |
                                    STATUS_EGDB_SINGLE_VALUE_HIT | STATUS_EGDB_DISK_READ_ERROR |
                                    STATUS_EGDB_DECODE_ERROR);
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_ATTEMPT);
    }

    uint32_t index;
    int bm, bk, wm, wk, bmrank=0, wmrank=0;
    int blocknumber;    
	int uniqueblockid;
	int reverse = 0;
	unsigned char *diskblock;
	int n;
	bitboard_pos revbitboard_pos;
	bitboard_pos p;
	T *dbpointer;
	
    p=*q;
	p.color = q->color & 1;

	bm = recbitcount(p.bm);
	bk = recbitcount(p.bk);
	wm = recbitcount(p.wm);
	wk = recbitcount(p.wk);
	
	if( (bm+wm+wk+bk>maxpieces) || (bm+bk>maxpiece) || (wm+wk>maxpiece)) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS);
            return DB_UNKNOWN;
        } else {
            return 0;
        }
    }
	if(bm)
		bmrank = MSB(p.bm)/4;
	if(wm)
		wmrank = (31-LSB(p.wm))/4;    
	
	if (( ((wm+wk-bm-bk)<<16) + ((wk-bk)<<8) + ((wmrank-bmrank)<<4) + p.color) > 0)
		reverse = 1;

	if (reverse)
		{
		revbitboard_pos.bm = revert(p.wm);
		revbitboard_pos.bk = revert(p.wk);
		revbitboard_pos.wm = revert(p.bm);
		revbitboard_pos.wk = revert(p.bk);
		revbitboard_pos.color = p.color^1;
		p = revbitboard_pos;
		
		reverse = bm;
		bm = wm;
		wm = reverse;
		
		reverse = bk;
		bk = wk;
		wk = reverse;
		
		reverse = bmrank;
		bmrank = wmrank;
		wmrank = reverse;
		
		reverse = 1;
		}

    if constexpr (std::is_same<T, cprsubdb>::value) {
        dbpointer = &cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][p.color];
    } else {
        dbpointer = &cprsubdatabase_mtc[bm][bk][wm][wk][bmrank][wmrank][p.color];
    }

	if(dbpointer->ispresent == 0) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_LOOKUP_NOT_PRESENT);
            return DB_UNKNOWN;
        } else {
            return 0;
        }
    }
	if(dbpointer->value != DB_UNKNOWN) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_SINGLE_VALUE_HIT | STATUS_EGDB_LOOKUP_HIT);
            if (dbpointer->value == DB_WIN) updateProgramStatusWord(STATUS_EGDB_WIN_RESULT);
            else if (dbpointer->value == DB_LOSS) updateProgramStatusWord(STATUS_EGDB_LOSS_RESULT);
            else if (dbpointer->value == DB_DRAW) updateProgramStatusWord(STATUS_EGDB_DRAW_RESULT);
            else updateProgramStatusWord(STATUS_EGDB_UNKNOWN_RESULT); // Should not happen for single values
        }
		return dbpointer->value;
    }

	index = calculate_index(p, bm, bk, wm, wk, bmrank, wmrank);
    if (index == UINT32_MAX) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_LOOKUP_INVALID_INDEX);
            return DB_UNKNOWN;
        } else {
            return 0;
        }
    }

    auto& idx = dbpointer->idx;
	n= dbpointer->num_blocks;
	
	blocknumber=0;
	while((blocknumber < n - 1) && ((unsigned int)idx[blocknumber + 1] <= index) )
		blocknumber++;

	uniqueblockid = dbpointer->blockoffset+
					dbpointer->first_block_id +
					blocknumber;
    
	diskblock = get_disk_block_generic<T>(uniqueblockid, dbpointer, blocknumber); 

	if (diskblock == nullptr) {
        if constexpr (std::is_same<T, cprsubdb>::value) {
            updateProgramStatusWord(STATUS_EGDB_LOOKUP_MISS);
	 	    return DB_NOT_LOOKED_UP;
        } else {
            return 0;
        }
	} else 
	{
	 	int result = decode_value_generic<T>(diskblock, index, dbpointer, blocknumber);
        if constexpr (std::is_same<T, cprsubdb>::value) {
            // Interpret result and set appropriate status flags
            if (result == DB_WIN) updateProgramStatusWord(STATUS_EGDB_WIN_RESULT);
            else if (result == DB_LOSS) updateProgramStatusWord(STATUS_EGDB_LOSS_RESULT);
            else if (result == DB_DRAW) updateProgramStatusWord(STATUS_EGDB_DRAW_RESULT);
            else if (result == DB_UNKNOWN) updateProgramStatusWord(STATUS_EGDB_UNKNOWN_RESULT);
            else updateProgramStatusWord(STATUS_EGDB_UNEXPECTED_VALUE); // For the '4' case
        }
        return result;
	}
}
