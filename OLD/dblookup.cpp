#include "checkers_types.h"
#include "log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdbool>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cerrno>
#include <climits>
#include <cctype>

#define SKIPS 58
#define MAXSKIP 10000

// Local static global variables for dblookup.cpp
static cprsubdb cprsubdatabase[MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][7][7][2]; 
static unsigned char **blockpointer;	
static unsigned char *cachebaseaddress; 
static struct bi
	{
	int uniqueid; 
	int next;     
	int prev;     
	} *blockinfo;
static int head,tail; 
static const int skip[SKIPS]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,40,44,48,52,56,60,70,80,90,100,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,MAXSKIP};
static int runlength[256]; 
static int value[256];	
static int bicoef[33][33]; 
static int maxblockid = 0;  
static FILE *dbfp[MAXFP]; 
static char dbnames[MAXFP][256]; 
static char DBpath[256];

static int maxblocknum;
static int maxidx;
static int maxpieces;
static int maxpiece;
static int cachesize;
static int bytesallocated = 0;

static char dbinfo[1024] = "";
static int decode_table[81][4];

struct DB_CONFIG {
    int maxidx;
    int maxblocknum;
    int maxpieces;
    int maxpiece;
};

static const DB_CONFIG db_configs[] = {
    {0, 0, 0, 0}, // 0 pieces
    {0, 0, 0, 0}, // 1 piece
    {MAXIDX4, BLOCKNUM4, 2, 2}, // 2 pieces
    {MAXIDX4, BLOCKNUM4, 3, 3}, // 3 pieces
    {MAXIDX4, BLOCKNUM4, 4, 3}, // 4 pieces
    {MAXIDX5, BLOCKNUM5, 5, 3}, // 5 pieces
    {MAXIDX6, BLOCKNUM6, 6, 3}, // 6 pieces
    {MAXIDX7, BLOCKNUM7, 7, 4}, // 7 pieces
    {MAXIDX8, BLOCKNUM8, 8, 4}  // 8 pieces
};

// Helper function local to this file
static int choose(int n, int k)
	{
	int result = 1;
	int i;
	i=k;
	while(i)
		{
		result *= (n-i+1);
		i--;
		}
	i=k;
	while(i)
		{
		result /=i;
		i--;
		}
	return result;
	}

static int parseindexfile(const char* EGTBdirectory, char idxfilename[256],int blockoffset,int fpcount);
    
int db_getcachesize(void)
	{
	return cachesize;
	}

static uint32_t calculate_lsb_index(uint32_t pieces, uint32_t occupied_mask) {
    uint32_t index = 0;
    uint32_t y = pieces;
    uint32_t x;
    int i = 1;
    while (y) {
        x = LSB(y);
        y ^= (1 << x);
        if (occupied_mask) {
            x -= recbitcount(occupied_mask & ((1 << x) - 1));
        }
        index += bicoef[x][i];
        i++;
    }
    return index;
}

static uint32_t calculate_msb_index(uint32_t pieces) {
    uint32_t index = 0;
    uint32_t y = pieces;
    uint32_t x;
    int i = 1;
    while (y) {
        x = MSB(y);
        y ^= (1 << x);
        x = 31 - x;
        index += bicoef[x][i];
        i++;
    }
    return index;
}

static uint32_t calculate_index(const pos& p, int bm, int bk, int wm, int wk, int bmrank, int wmrank) {
    uint32_t bmindex = calculate_lsb_index(p.bm, 0);
    uint32_t wmindex = calculate_msb_index(p.wm);
    uint32_t bkindex = calculate_lsb_index(p.bk, p.bm | p.wm);
    uint32_t wkindex = calculate_lsb_index(p.wk, p.bm | p.bk | p.wm);

    uint32_t bmrange = 1, wmrange = 1, bkrange = 1;
    
    if (bm)
        bmrange = bicoef[4 * (bmrank + 1)][bm] - bicoef[4 * bmrank][bm];
    if (wm)
        wmrange = bicoef[4 * (wmrank + 1)][wm] - bicoef[4 * wmrank][wm];
    if (bk)
        bkrange = bicoef[32 - bm - wm][bk];

    if (bmrank)
        bmindex -= bicoef[4 * bmrank][bm];
    if (wmrank)
        wmindex -= bicoef[4 * wmrank][wm];

    return bmindex + wmindex * bmrange + bkindex * bmrange * wmrange + wkindex * bmrange * wmrange * bkrange;
}

static void move_cache_block_to_head(int block_index)
{
    if (block_index == head) {
        return;
    }

    int prev = blockinfo[block_index].prev;
    int next = blockinfo[block_index].next;

    if (next != -1) {
        blockinfo[next].prev = prev;
    } else { // It was the tail
        tail = prev;
    }

    if (prev != -1) {
        blockinfo[prev].next = next;
    }

    blockinfo[block_index].next = head;
    blockinfo[block_index].prev = -1;
    
    if (head != -1) {
        blockinfo[head].prev = block_index;
    }

    head = block_index;
}

static unsigned char* get_disk_block(int uniqueblockid, cprsubdb* dbpointer, int blocknumber, int cl)
{
    unsigned char* diskblock;
    int newhead;

    if (blockpointer[uniqueblockid] != nullptr) {
        diskblock = blockpointer[uniqueblockid];
        newhead = (diskblock - cachebaseaddress) >> 10;
        move_cache_block_to_head(newhead);
    } else { // Not in cache, try to load from disk
        if (tail == -1) {
            log_c(LOG_LEVEL_ERROR, "dblookup: cache is full, tail is -1.");
            g_programStatusWord |= STATUS_CRITICAL_ERROR;
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

        if (dbfp[dbpointer->fp] == nullptr) {
            return nullptr;
        }
        int fseek_result = fseek(dbfp[dbpointer->fp], (blocknumber + dbpointer->firstblock) << 10, SEEK_SET);
        if (fseek_result != 0) {
            log_c(LOG_LEVEL_ERROR, "dblookup: fseek failed.");
            g_programStatusWord |= STATUS_EGDB_LOOKUP_MISS;
            return nullptr;
        }
        
        size_t itemsRead = fread(diskblock, 1024, 1, dbfp[dbpointer->fp]);
        if (itemsRead != 1) {
            log_c(LOG_LEVEL_ERROR, "dblookup: fread failed to read a full block.");
            g_programStatusWord |= STATUS_EGDB_LOOKUP_MISS;
            blockpointer[uniqueblockid] = nullptr;
            return nullptr;
        }
    }
    return diskblock;
}

static int decode_value(unsigned char* diskblock, uint32_t index, cprsubdb* dbpointer, int blocknumber)
{
    log_c(LOG_LEVEL_DEBUG, "dblookup: Entering decode_value.");
    int i;
    int n;
    int returnvalue = DB_UNKNOWN;
    unsigned char byte;
    bool reverse = false;
    int *idx = dbpointer->idx;

    if (dbpointer->numberofblocks > blocknumber + 1) {
        if (idx[blocknumber + 1] - index < index - idx[blocknumber]) {
            reverse = true;
        }
    }

    if (reverse) {
        n = idx[blocknumber + 1];
        i = 1023;
        while ((unsigned int)n > index) {
            if (i < 0) {
                log_c(LOG_LEVEL_ERROR, "dblookup: decode_value reverse search failed.");
                g_programStatusWord |= STATUS_FILE_IO_ERROR;
                return DB_UNKNOWN;
            }
            n -= runlength[diskblock[i]];
            i--;
        }
        i++;
    } else {
        n = idx[blocknumber];
        i = (blocknumber == 0) ? dbpointer->startbyte : 0;

        while ((unsigned int)n <= index) {
            if (i >= 1024) {
                log_c(LOG_LEVEL_ERROR, "dblookup: decode_value forward search failed.");
                g_programStatusWord |= STATUS_FILE_IO_ERROR;
                return DB_UNKNOWN;
            }
            n += runlength[diskblock[i]];
            i++;
        }
        i--;
        n -= runlength[diskblock[i]];
    }

    if (i < 0 || i >= 1024) {
        log_c(LOG_LEVEL_ERROR, "dblookup: decode_value index out of bounds.");
        g_programStatusWord |= STATUS_FILE_IO_ERROR;
        return DB_UNKNOWN;
    }

    byte = diskblock[i];
    if (byte > 80) {
        returnvalue = value[byte];
    } else {
        i = index - n;
        if (i >= 0 && i < 4) {
            returnvalue = decode_table[byte][i];
        } else {
            log_c(LOG_LEVEL_ERROR, "dblookup: decode_value switch default case hit.");
            g_programStatusWord |= STATUS_FILE_IO_ERROR;
            return DB_UNKNOWN;
        }
    }
    char log_msg_buffer[256];
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: Raw value from EGDB: %d (byte: %d, i: %d)", returnvalue, byte, i);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
    returnvalue++;
    g_programStatusWord |= STATUS_EGDB_LOOKUP_HIT; // Set status flag for EGDB lookup hit
    return returnvalue;
}

int dblookup(pos *q,int cl)
	{
    // Clear previous EGDB lookup status flags
    g_programStatusWord &= ~STATUS_EGDB_LOOKUP_HIT;
    g_programStatusWord &= ~STATUS_EGDB_LOOKUP_MISS;

    uint32_t index;
    int bm, bk, wm, wk, bmrank=0, wmrank=0;
    int i;
    uint32_t x,y;
    uint32_t bmindex=0,bkindex=0,wmindex=0,wkindex=0;
    uint32_t bmrange=1, wmrange=1, bkrange=1;
    int blocknumber;	
	int uniqueblockid;
	int reverse = 0;
	int returnvalue=DB_UNKNOWN;
	unsigned char *diskblock;
	int newhead,prev, next;
	int n;
	int *idx;
	unsigned char byte;
	pos revpos;
	pos p;
	cprsubdb *dbpointer;
	
p=*q;
	p.color = q->color & 1;

	bm = recbitcount(p.bm);
	bk = recbitcount(p.bk);
	wm = recbitcount(p.wm);
	wk = recbitcount(p.wk);
	
	if( (bm+wm+wk+bk>maxpieces) || (bm+bk>maxpiece) || (wm+wk>maxpiece)) {
        log_c(LOG_LEVEL_DEBUG, "dblookup: Early exit - piece count exceeds maxpieces. Returning DB_UNKNOWN.");
		return DB_UNKNOWN;
    } 	
	if(bm)
		bmrank = MSB(p.bm)/4;
	if(wm)
		wmrank = (31-LSB(p.wm))/4;	
	
	if (( ((wm+wk-bm-bk)<<16) + ((wk-bk)<<8) + ((wmrank-bmrank)<<4) + p.color) > 0)
		reverse = 1;

	if (reverse)
		{
		revpos.bm = revert(p.wm);
		revpos.bk = revert(p.wk);
		revpos.wm = revert(p.bm);
		revpos.wk = revert(p.bk);
		revpos.color = p.color^1;
		p = revpos;
		
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
dbpointer = &cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][p.color];
	if(dbpointer->ispresent == 0 || (dbfp[dbpointer->fp] == NULL)) {
        log_c(LOG_LEVEL_DEBUG, "dblookup: Early exit - dbpointer not present or file not open. Returning DB_UNKNOWN.");
		return DB_UNKNOWN;
    }
	if(dbpointer->value != DB_UNKNOWN)
		return dbpointer->value;

	index = calculate_index(p, bm, bk, wm, wk, bmrank, wmrank);
	idx = dbpointer->idx;
	n= dbpointer->numberofblocks;
	
	blocknumber=0;
	while((blocknumber < n - 1) && ((unsigned int)idx[blocknumber + 1] <= index) )
		blocknumber++;

	uniqueblockid = dbpointer->blockoffset+
					dbpointer->firstblock +
					blocknumber;
	diskblock = get_disk_block(uniqueblockid, dbpointer, blocknumber, cl);

	if (diskblock == nullptr) {
		int final_result = DB_NOT_LOOKED_UP; // Changed to DB_NOT_LOOKED_UP
		char log_msg_buffer[256];
		snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: Final result from dblookup: %d (diskblock is nullptr, returning DB_NOT_LOOKED_UP)", final_result);
		log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
		return final_result;
	}		
	int final_result = decode_value(diskblock, index, dbpointer, blocknumber);
	char log_msg_buffer[256];
	snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: Final result from dblookup: %d (from decode_value)", final_result);
	log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
	return final_result;
}
int64_t getdatabasesize(int bm, int bk, int wm, int wk, int bmrank, int wmrank)
	{
	        int64_t dbsize = 1;
	if(bm)
		dbsize *= bicoef[4*(bmrank+1)][bm] - bicoef[4*bmrank][bm];
  
	if(wm)
		dbsize *= bicoef[4*(wmrank+1)][wm] - bicoef[4*wmrank][wm];

	if(bk)
		dbsize *= bicoef[32-bm-wm][bk];

	if(wk)
		dbsize *= bicoef[32-bm-wm-bk][wk];

	return dbsize;
	}

int db_exit(void)
	{
	int i, bm, bk, wm, wk, bmrank, wmrank, color;

	    free(cachebaseaddress);
	    free(blockpointer);
	    free(blockinfo); 	
	for(i=0;i<50;i++)
		{
		if(dbfp[i] != nullptr)
			fclose(dbfp[i]);	
		}

    for (bm = 0; bm <= MAXPIECE; ++bm) {
        for (bk = 0; bk <= MAXPIECE; ++bk) {
            for (wm = 0; wm <= MAXPIECE; ++wm) {
                for (wk = 0; wk <= MAXPIECE; ++wk) {
                    for (bmrank = 0; bmrank < 7; ++bmrank) {
                        for (wmrank = 0; wmrank < 7; ++wmrank) {
                            for (color = 0; color < 2; ++color) {
                                if (cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx != nullptr) {
                                    free(cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx);
                                    cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx = nullptr;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

	return 1;
	}

static char* read_entire_file(const char* fullpath, long* file_size) {
    FILE *fp = fopen(fullpath, "rb");
    if (fp == nullptr) {
        char log_msg_buffer_fail_open[256];
        snprintf(log_msg_buffer_fail_open, sizeof(log_msg_buffer_fail_open), "dblookup: read_entire_file - Failed to open file: %s (errno: %d)", fullpath, errno);
        log_c(LOG_LEVEL_ERROR, log_msg_buffer_fail_open);
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    *file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = static_cast<char*>(malloc(*file_size + 1));
    if (content == nullptr) {
        log_c(LOG_LEVEL_ERROR, "dblookup: read_entire_file - malloc failed for file content!");
        g_programStatusWord |= STATUS_CRITICAL_ERROR | STATUS_EGDB_INIT_FAIL;
        fclose(fp);
        return nullptr;
    }

    if (fread(content, 1, *file_size, fp) != *file_size) {
        char log_msg_buffer_fread_fail[256];
        snprintf(log_msg_buffer_fread_fail, sizeof(log_msg_buffer_fread_fail), "dblookup: read_entire_file - fread failed to read entire file (expected %ld bytes, got less)!", *file_size);
        log_c(LOG_LEVEL_ERROR, log_msg_buffer_fread_fail);
        g_programStatusWord |= STATUS_FILE_IO_ERROR | STATUS_EGDB_INIT_FAIL;
        free(content);
        fclose(fp);
        return nullptr;
    }
    fclose(fp);
    content[*file_size] = '\0';
    return content;
}

static bool parse_single_base_entry(char **ptr_ref, int blockoffset, int fpcount, int* total_blocks_ptr, char* file_content) {
    char *ptr = *ptr_ref;
    char *original_ptr = ptr; // Keep original ptr to advance if sscanf fails

    if (strncmp(ptr, " BASE", 5) == 0) {
        ptr += 5;
        int bm, bk, wm, wk, bmrank, wmrank, color, singlevalue, startbyte, firstblock;
        char colorchar;
        int n_read = 0;
        int stat = sscanf(ptr, "%i,%i,%i,%i,%i,%i,%c:%n", &bm, &bk, &wm, &wk, &bmrank, &wmrank, &colorchar, &n_read);
        ptr += n_read;

        if (stat < 7) {
            log_c(LOG_LEVEL_DEBUG, "parse_single_base_entry: sscanf failed to parse full BASE entry. Skipping.");
            *ptr_ref = original_ptr + 1; // Advance ptr to avoid infinite loop
            return false;
        }
        char log_msg_base_entry[256];
        snprintf(log_msg_base_entry, sizeof(log_msg_base_entry), "parse_single_base_entry: Found BASE entry: bm=%d, bk=%d, wm=%d, wk=%d, bmrank=%d, wmrank=%d, colorchar=%c", bm, bk, wm, wk, bmrank, wmrank, colorchar);
        log_c(LOG_LEVEL_DEBUG, log_msg_base_entry);

        color = (colorchar == 'b') ? DB_BLACK : DB_WHITE;
        cprsubdb *dbpointer = &cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color];

        if (isdigit(*ptr)) {
            char *next_ptr;
            firstblock = strtol(ptr, &next_ptr, 10);
            if (*next_ptr != '/') {
                log_c(LOG_LEVEL_ERROR, "parse_single_base_entry: Expected '/' after firstblock. Skipping entry.");
                *ptr_ref = original_ptr + 1; // Advance ptr to avoid infinite loop
                return false;
            }
            ptr = next_ptr + 1;
            startbyte = strtol(ptr, &next_ptr, 10);
            ptr = next_ptr;

            char log_msg_block_info[256];
            snprintf(log_msg_block_info, sizeof(log_msg_block_info), "parse_single_base_entry: Block info: firstblock=%d, startbyte=%d", firstblock, startbyte);
            log_c(LOG_LEVEL_DEBUG, log_msg_block_info);

            dbpointer->firstblock = firstblock;
            dbpointer->blockoffset = blockoffset;
            dbpointer->ispresent = 1;
            dbpointer->value = 0;
            dbpointer->startbyte = startbyte;
            dbpointer->fp = fpcount;

            int current_idx_count = 1; // Start with 1 for idx[0] = 0
            char *temp_ptr = ptr;
            while (*temp_ptr) {
                while (*temp_ptr && isspace(*temp_ptr)) temp_ptr++;
                if (!*temp_ptr || !isdigit(*temp_ptr)) break;
                strtol(temp_ptr, &next_ptr, 10);
                temp_ptr = next_ptr;
                current_idx_count++;
            }

            char log_msg_num_idx_count[256];
            snprintf(log_msg_num_idx_count, sizeof(log_msg_num_idx_count), "parse_single_base_entry: Number of indices counted: %d", current_idx_count);
            log_c(LOG_LEVEL_DEBUG, log_msg_num_idx_count);

            dbpointer->numberofblocks = current_idx_count;
            dbpointer->idx = static_cast<int*>(malloc(current_idx_count * sizeof(int)));
            if (dbpointer->idx == nullptr) {
                log_c(LOG_LEVEL_ERROR, "dblookup: malloc failed in parse_single_base_entry for dbpointer->idx!");
                g_programStatusWord |= STATUS_CRITICAL_ERROR | STATUS_EGDB_INIT_FAIL;
                free(file_content); // Free parent's file_content on malloc failure
                return false;
            }
            bytesallocated += current_idx_count * sizeof(int);

            // Now re-parse and populate the dynamically allocated array
            int num = 1;
            dbpointer->idx[0] = 0; // The first index is always 0
            while (*ptr) {
                while (*ptr && isspace(*ptr)) ptr++;
                if (!*ptr || !isdigit(*ptr)) break;
                
                dbpointer->idx[num] = strtol(ptr, &next_ptr, 10);
                ptr = next_ptr;
                num++;
                if (num >= current_idx_count) break; // Should not overflow now
            }

            char log_msg_num_idx_parsed[256];
            snprintf(log_msg_num_idx_parsed, sizeof(log_msg_num_idx_parsed), "parse_single_base_entry: Number of indices parsed: %d", num);
            log_c(LOG_LEVEL_DEBUG, log_msg_num_idx_parsed);

            if (firstblock + current_idx_count > *total_blocks_ptr) { // Use current_idx_count here
                *total_blocks_ptr = firstblock + current_idx_count;
            }

        } else {
            switch (*ptr) {
                case '+': singlevalue = DB_WIN; break;
                case '=': singlevalue = DB_DRAW; break;
                case '-': singlevalue = DB_LOSS; break;
                default: singlevalue = DB_UNKNOWN; break;
            }
            char log_msg_single_value[256];
            snprintf(log_msg_single_value, sizeof(log_msg_single_value), "parse_single_base_entry: Single value entry: %d (char: %c)", singlevalue, *ptr);
            log_c(LOG_LEVEL_DEBUG, log_msg_single_value);
            dbpointer->blockoffset = 0;
            dbpointer->firstblock = 0;
            dbpointer->idx = nullptr;
            dbpointer->ispresent = 1;
            dbpointer->numberofblocks = 0;
            dbpointer->value = singlevalue;
            dbpointer->fp = fpcount;
            ptr++; // Advance ptr past the single value char
        }
        *ptr_ref = ptr; // Update the main ptr
        return true;
    }
    *ptr_ref = original_ptr + 1; // Advance ptr to avoid infinite loop if not " BASE"
    return false;
}

static int parseindexfile(const char* EGTBdirectory, char idxfilename[256], int blockoffset, int fpcount) {
    char fullpath[MAX_PATH_FIXED];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, idxfilename);
    char log_msg_buffer[256];
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: parseindexfile - Attempting to open IDX file: %s", fullpath);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);

    long fsize = 0;
    char *file_content = read_entire_file(fullpath, &fsize);
    if (file_content == nullptr) {
        char log_msg_buffer_fail_read[256];
        snprintf(log_msg_buffer_fail_read, sizeof(log_msg_buffer_fail_read), "dblookup: parseindexfile - Failed to read IDX file: %s", fullpath);
        log_c(LOG_LEVEL_ERROR, log_msg_buffer_fail_read);
        g_programStatusWord |= STATUS_FILE_IO_ERROR | STATUS_EGDB_INIT_FAIL;
        return -1;
    }
    char log_msg_buffer_success_read[256];
    snprintf(log_msg_buffer_success_read, sizeof(log_msg_buffer_success_read), "dblookup: parseindexfile - Successfully read IDX file: %s (size: %ld)", fullpath, fsize);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer_success_read);

    char *ptr = file_content;
    int total_blocks = 0;

    while (*ptr) {
        if (!parse_single_base_entry(&ptr, blockoffset, fpcount, &total_blocks, file_content)) {
            // parse_single_base_entry advances ptr, so just continue
            // This case handles non-" BASE" lines or errors within parsing, ptr already advanced
        }
    }

    free(file_content);
    return total_blocks;
}

int db_init(int suggestedMB, char out[256], const char* EGTBdirectory)
{
    for (int byte_val = 0; byte_val < 81; ++byte_val) {
        decode_table[byte_val][0] = byte_val % 3;
        decode_table[byte_val][1] = (byte_val / 3) % 3;
        decode_table[byte_val][2] = (byte_val / 9) % 3;
        decode_table[byte_val][3] = (byte_val / 27) % 3;
    }

    g_programStatusWord |= STATUS_EGDB_INIT_START; // Mark EGDB initialization start
    FILE *fp;
    char dbname[256];
    char fullpath[MAX_PATH_FIXED];
    int i,j,n,nb,nw;
    int bm,bk,wm,wk;
    int blockoffset = 0;
    int autoloadnum = 0;
    int cprFileCount = 0;
    int pifreturnvalue;
    int pieces=0;
    int memsize;
    char str[256];

    strncpy(DBpath, EGTBdirectory, sizeof(DBpath) - 1);
    DBpath[sizeof(DBpath) - 1] = '\0';
	
	for(i=0;i<33;i++)
		{
		for(j=1;j<=i;j++)
			{
			bicoef[i][j] = choose(i,j);
			}
		bicoef[i][0] = 1;
		}

	for(i=1;i<33;i++)
		bicoef[0][i]=0;

	for(i=0;i<81;i++)
		runlength[i]=4;

	for(i=81;i<256;i++)
		{
		runlength[i]= skip[(i-81)%SKIPS];
		value[i]= ((i-81)/SKIPS);
		}

	memset(cprsubdatabase, 0, (MAXPIECE+1)*(MAXPIECE+1)*(MAXPIECE+1)*(MAXPIECE+1)*98*sizeof(cprsubdb));
	
    for(n=2;n<=MAXPIECE+1;n++)
    {
        char log_msg_buffer[256];
		snprintf(dbname, sizeof(dbname), "db%i.idx",n);
        snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, dbname);
        snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: db_init - Attempting to open IDX file for pieces %d: %s", n, fullpath);
        log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
		fp = fopen(fullpath,"rb");
		if(fp)
		{
            snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: db_init - Successfully opened IDX file for pieces %d: %s", n, fullpath);
            log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
			if (n > pieces) {
			    pieces = n;
			}
            fclose(fp);
		}
		else
		{
            snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: db_init - Failed to open IDX file for pieces %d: %s (errno: %d)", n, fullpath, errno);
            log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
			continue;
		}
    }

    for(n=SPLITSIZE;n<=8;n++)
    {
        for(nb=4;nb<=4;nb++)
        {
            nw=n-nb;
            if(nw>nb)
                continue;
            for(bk=0;bk<=nb;bk++)
            {
                bm=nb-bk;
                for(wk=0;wk<=nw;wk++)
                {
                    wm=nw-wk;
                    if(bm+bk==wm+wk && wk>bk)
                        continue;
                    snprintf(dbname, sizeof(dbname), "db%i_%i%i%i%i.cpr",bm+bk+wm+wk,bm,bk,wm,wk);
                    snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, dbname);
                    fp = fopen(fullpath,"rb");
                    if(fp)
                    {
                        if (n > pieces) {
                            pieces = n;
                        }
                        fclose(fp);
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
    }
		    if (pieces > 0 && pieces < sizeof(db_configs) / sizeof(db_configs[0])) {
		        const DB_CONFIG* config = &db_configs[pieces];
		        maxidx = config->maxidx;
		        maxblocknum = config->maxblocknum;
		        maxpieces = config->maxpieces;
		        maxpiece = config->maxpiece;
		    } else {
		        // Handle error or default case
		    }

    cachesize = suggestedMB << 10;
    if(maxpieces <6)
        cachesize = 2000;
    if(maxpieces == 6)
        cachesize = 42000;
    if(maxpieces > 6)
    {
        cachesize = (cachesize > MINCACHESIZE ? cachesize : MINCACHESIZE);
    }
    
    blockoffset = 0;

    for(n=2; n<=maxpieces; n++)
    {
        if(n>=SPLITSIZE)
            continue;

        char log_msg_buffer[256];
        snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: db_init - Constructing dbname: db%d.cpr for n=%d", n, n);
        log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
        snprintf(dbname, sizeof(dbname), "db%i.cpr", n);
        snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, dbname);
        snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: db_init - Attempting to open CPR file: %s", fullpath);
        log_c(LOG_LEVEL_DEBUG, log_msg_buffer);

            if (cprFileCount < MAXFP) {
                dbfp[cprFileCount] = fopen(fullpath, "rb");
                if (dbfp[cprFileCount] != nullptr) {
                    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: Successfully opened CPR file: %s", fullpath);
                    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
                    sprintf(dbnames[cprFileCount], "%s", fullpath);
                    snprintf(dbname, sizeof(dbname), "db%i.idx", n);
                    char log_msg_buffer_idx_call[256];
                    snprintf(log_msg_buffer_idx_call, sizeof(log_msg_buffer_idx_call), "dblookup: Calling parseindexfile for IDX file: %s", dbname);
                    log_c(LOG_LEVEL_DEBUG, log_msg_buffer_idx_call);
                    pifreturnvalue = parseindexfile(EGTBdirectory, dbname, blockoffset, cprFileCount);
                    if (pifreturnvalue >= 0) {
                        blockoffset += pifreturnvalue;
                        cprFileCount++;
                    } else {
                        log_c(LOG_LEVEL_ERROR, "dblookup: parseindexfile failed!");
                        g_programStatusWord |= STATUS_EGDB_INIT_FAIL;
                        fclose(dbfp[cprFileCount]);
                        dbfp[cprFileCount] = nullptr;
                    }
                } else {
                    char log_msg_buffer[256];
                    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "dblookup: Failed to open CPR file: %s (errno: %d)", fullpath, errno);
                    log_c(LOG_LEVEL_ERROR, log_msg_buffer);
                    g_programStatusWord |= STATUS_EGDB_INIT_FAIL;
                }
            }    }

    for (n=SPLITSIZE; n<=maxpieces; n++)
    {
        for (nb=maxpieces-maxpiece; nb<=maxpiece; nb++)
        {
            nw=n-nb;
            if (nw > nb)
                continue;
            for (bk=0; bk<=nb; bk++)
            {
                bm=nb-bk;
                for (wk=0; wk<=nw; wk++)
                {
                    wm=nw-wk;
                    if (bm+bk==wm+wk && wk>bk)
                        continue;

                    snprintf(dbname, sizeof(dbname), "db%i_%i%i%i%i.cpr", bm+bk+wm+wk, bm, bk, wm, wk);
                    snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, dbname);
                    if (cprFileCount < MAXFP) {
                        char log_msg_buffer_split_cpr_attempt[256];
                    snprintf(log_msg_buffer_split_cpr_attempt, sizeof(log_msg_buffer_split_cpr_attempt), "dblookup: Attempting to open split CPR file: %s", fullpath);
                    log_c(LOG_LEVEL_DEBUG, log_msg_buffer_split_cpr_attempt);
                        dbfp[cprFileCount] = fopen(fullpath, "rb");
                        if (dbfp[cprFileCount] != nullptr) {
                            char log_msg_buffer_split_cpr_success[256];
                            snprintf(log_msg_buffer_split_cpr_success, sizeof(log_msg_buffer_split_cpr_success), "dblookup: Successfully opened split CPR file: %s", fullpath);
                            log_c(LOG_LEVEL_DEBUG, log_msg_buffer_split_cpr_success);
                            sprintf(dbnames[cprFileCount], "%s", fullpath);
                            snprintf(dbname, sizeof(dbname), "db%i_%i%i%i%i.idx", bm+bk+wm+wk, bm, bk, wm, wk);
                            char log_msg_buffer_split_idx_call[256];
                            snprintf(log_msg_buffer_split_idx_call, sizeof(log_msg_buffer_split_idx_call), "dblookup: Calling parseindexfile for split IDX file: %s", dbname);
                            log_c(LOG_LEVEL_DEBUG, log_msg_buffer_split_idx_call);
                            pifreturnvalue = parseindexfile(EGTBdirectory, dbname, blockoffset, cprFileCount);
                            if (pifreturnvalue >= 0) {
                                blockoffset += pifreturnvalue;
                                cprFileCount++;
                            } else {
                                fclose(dbfp[cprFileCount]);
                                g_programStatusWord |= STATUS_EGDB_INIT_FAIL;
                                dbfp[cprFileCount] = nullptr;
                                                        }
                                                    } else {
                                                        char log_msg_buffer_split_cpr_fail[256];
                        snprintf(log_msg_buffer_split_cpr_fail, sizeof(log_msg_buffer_split_cpr_fail), "dblookup: Failed to open split CPR file: %s", fullpath);
                        log_c(LOG_LEVEL_ERROR, log_msg_buffer_split_cpr_fail);
                                                        g_programStatusWord |= STATUS_EGDB_INIT_FAIL;
                                                    }
                                                }            }
        }
    }

	memsize = cachesize << 10;
    cachebaseaddress = static_cast<unsigned char*>(malloc(memsize));
    
    if(cachebaseaddress == nullptr && memsize!=0)
    {
        g_programStatusWord |= STATUS_EGDB_INIT_FAIL; // Set status flag for EGDB init failure
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for cachebaseaddress failed!");
        return 0;
    }
    
    memsize = maxblocknum*sizeof(unsigned char*);
    blockpointer = static_cast<unsigned char**>(malloc(memsize));
    if(blockpointer == nullptr && memsize != 0 )
    {
        g_programStatusWord |= STATUS_EGDB_INIT_FAIL; // Set status flag for EGDB init failure
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for blockpointer failed!");
        free(cachebaseaddress);
        return 0;
    }
    for(i=0;i<maxblocknum;i++)
        blockpointer[i]=nullptr;
        
    memsize = cachesize*sizeof(struct bi);
    blockinfo = static_cast<struct bi*>(malloc(memsize));
    if(blockinfo == nullptr && memsize!=0)
    {
        g_programStatusWord |= STATUS_EGDB_INIT_FAIL; // Set status flag for EGDB init failure
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for blockinfo failed!");
        free(cachebaseaddress);
        free(blockpointer);
        return 0;
    }
    
    autoloadnum = preload(out, dbfp, cprFileCount);                                
    
    autoloadnum=0;
    for(i=autoloadnum;i<cachesize;i++)
    {
        blockinfo[i].next = i+1;
        blockinfo[i].prev = i-1;
        blockinfo[i].uniqueid = i;
    }
    blockinfo[cachesize-1].next=-1;
    blockinfo[autoloadnum].prev=-1;
    
    head=autoloadnum;
    tail = cachesize-1;
    
    g_programStatusWord |= STATUS_EGDB_INIT_OK; // Set status flag for successful EGDB initialization
	return maxpieces;
}
	return maxpieces;
}

int preload(char out[256], FILE *db_fp[], int fp_count) {
	unsigned char *diskblock;
	int cachepointer = 0;
	int autoloadnum = 0;
    char dbname_to_use[256];

    for (int i = 0; i < fp_count; ++i) {
        FILE *fp = db_fp[i];
        if (fp == nullptr) {
            continue;
        }

        rewind(fp);

        strncpy(dbname_to_use, dbnames[i], sizeof(dbname_to_use) - 1);
        dbname_to_use[sizeof(dbname_to_use) - 1] = '\0';
		while(!feof(fp)){
			diskblock = cachebaseaddress + (cachepointer << 10);
            memset(diskblock, 0, 1024);
			blockpointer[cachepointer] = diskblock;
			blockinfo[cachepointer].uniqueid = cachepointer;
			
			if(!(cachepointer%1024))
				{
				}

		            size_t bytesRead = fread(diskblock,1024,1,fp);
		                        if (bytesRead != 1) {
                                        if (!feof(fp)) {
                                        }
                                        break;
		                        }			
		            cachepointer++;
			if(cachepointer == cachesize)
				return 1;
		}
    }
	return autoloadnum;
}

void db_infostring(char *str)
	{
	sprintf(str,"\nDatabase Details:\n%s",dbinfo);
	return;
	}
