#include "core_types.h"
#include "DBManager.h"
#include <QMutexLocker>
#include <QDebug>
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
#include <algorithm> // For std::min and std::max
#include "c_logic.h"



DBManager* DBManager::m_instance = nullptr;

const int DBManager::skip[SKIPS]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,40,44,48,52,56,60,70,80,90,100,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,MAXSKIP};

const DBManager::DB_CONFIG DBManager::db_configs[] = {
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

DBManager* DBManager::instance()
{
    if (!m_instance) {
        m_instance = new DBManager();
    }
    return m_instance;
}

DBManager::DBManager(QObject *parent) : QObject(parent),
    blockpointer(nullptr),
    cachebaseaddress(nullptr),
    blockinfo(nullptr),
    head(-1), tail(-1),
    maxblockid(0),
    maxblocknum(0),
    maxidx(0),
    maxpieces(0),
    maxpiece(0),
    cachesize(0),
    bytesallocated(0)
{
    memset(cprsubdatabase, 0, sizeof(cprsubdatabase));
    memset(cprsubdatabase_mtc, 0, sizeof(cprsubdatabase_mtc));
    memset(dbfp, 0, sizeof(dbfp));
    memset(dbfp_mtc, 0, sizeof(dbfp_mtc));
    memset(dbnames, 0, sizeof(dbnames));
    memset(dbnames_mtc, 0, sizeof(dbnames_mtc));
    strncpy(DBpath, "", sizeof(DBpath));
    memset(dbinfo, 0, sizeof(dbinfo));

    // Initialize runlength and value arrays dynamically
    for(int i=0;i<81;i++)
        runlength[i]=4;

    for(int i=81;i<256;i++)
    {
        runlength[i]= skip[(i-81)%SKIPS];
        value[i]= ((i-81)/SKIPS);
    }
}

DBManager::~DBManager()
{
    db_exit();
}

bool DBManager::processEgdbFiles(const char* EGTBdirectory, int nPieces, int bm, int bk, int wm, int wk, int& blockOffset, int& cprFileCount, char* outBuffer) {
    return processEgdbFiles_generic<cprsubdb>(EGTBdirectory, nPieces, bm, bk, wm, wk, blockOffset, cprFileCount, outBuffer);
}

bool DBManager::processEgdbFiles_mtc(const char* EGTBdirectory, int nPieces, int bm, int bk, int wm, int wk, int& blockOffset, int& cprFileCount, char* outBuffer) {
    return processEgdbFiles_generic<cprsubdb_mtc>(EGTBdirectory, nPieces, bm, bk, wm, wk, blockOffset, cprFileCount, outBuffer);
}


void DBManager::initDecodeTable() {
    for (int byte_val = 0; byte_val < DBManager::DECODE_TABLE_SIZE; ++byte_val) {
        decode_table[byte_val][0] = byte_val % 3;
        decode_table[byte_val][1] = (byte_val / 3) % 3;
        decode_table[byte_val][2] = (byte_val / 9) % 3;
        decode_table[byte_val][3] = (byte_val / 27) % 3;
    }
}

void DBManager::initBicoefTable() {
    for(int i=0;i<33;i++)
		{
		for(int j=1;j<=i;j++)
			{
			bicoef[i][j] = choose(i,j);
			}
		bicoef[i][0] = 1;
		}

	for(int i=1;i<33;i++)
		bicoef[0][i]=0;
}

int DBManager::choose(int n, int k)
{
    int result = 1;
    int i = k;
    while (i) {
        result *= (n - i + 1);
        i--;
    }
    i = k;
    while (i) {
        result /= i;
        i--;
    }
    return result;
}

uint32_t DBManager::calculate_lsb_index(uint32_t pieces, uint32_t occupied_mask) {
    uint32_t index = 0;
    uint32_t y = pieces;
    int x;
    int i = 1;
    while (y) {
        x = LSB(y);
        y ^= (1 << x);
        if (occupied_mask) {
            x -= recbitcount(occupied_mask & ((1 << x) - 1));
        }
        if (x < i - 1) {
            return UINT32_MAX;
        }
        index += bicoef[x][i];
        i++;
    }
    return index;
}

uint32_t DBManager::calculate_msb_index(uint32_t pieces) {
    uint32_t index = 0;
    uint32_t y = pieces;
    int x;
    int i = 1;
    while (y) {
        x = MSB(y);
        y ^= (1U << x);
        x = 31 - x;
        index += bicoef[x][i];
        i++;
    }
    return index;
}

uint32_t DBManager::calculate_index(const bitboard_pos& p, int bm, int bk, int wm, int wk, int bmrank, int wmrank) {
    int64_t bmindex = calculate_lsb_index(p.bm, 0);
    if (bmindex == UINT32_MAX) return UINT32_MAX;
    
    uint32_t reversed_wm = revert(p.wm);
    uint32_t reversed_bm = revert(p.bm); 
    int64_t wmindex = calculate_lsb_index(reversed_wm, reversed_bm);
    if (wmindex == UINT32_MAX) return UINT32_MAX;

    int64_t bkindex = calculate_lsb_index(p.bk, p.bm | p.wm);
    if (bkindex == UINT32_MAX) return UINT32_MAX;

    int64_t wkindex = calculate_lsb_index(p.wk, p.bm | p.bk | p.wm);
    if (wkindex == UINT32_MAX) return UINT32_MAX;

    int64_t bmrange = 1, wmrange = 1, bkrange = 1;
    
    if (bm)
        bmrange = bicoef[4 * (bmrank + 1)][bm] - bicoef[4 * bmrank][bm];
    if (wm)
        wmrange = bicoef[4 * (wmrank + 1)][wm] - bicoef[4 * wmrank][wm];
    if (bk)
        bkrange = bicoef[32 - bm - wm][bk];

    if (bmrank)
        bmindex -= bicoef[4 * bmrank][bm];

    return bmindex + wmindex * bmrange + bkindex * bmrange * wmrange + wkindex * bmrange * wmrange * bkrange;
}

void DBManager::move_cache_block_to_head(int block_index)
{
    if (block_index == head) {
        return;
    }

    int prev = blockinfo[block_index].prev;
    int next = blockinfo[block_index].next;

    if (next != -1) {
        blockinfo[next].prev = prev;
    } else { 
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

unsigned char* DBManager::get_disk_block(int uniqueblockid, cprsubdb* dbpointer, int blocknumber, int cl)
{
    // The 'cl' parameter is no longer used in the generic version but is kept for API compatibility for now.
    // It was determined to be unused in the original code.
    return get_disk_block_generic<cprsubdb>(uniqueblockid, dbpointer, blocknumber);
}

unsigned char* DBManager::get_disk_block_mtc(int uniqueblockid, cprsubdb_mtc* dbpointer, int blocknumber)
{
    return get_disk_block_generic<cprsubdb_mtc>(uniqueblockid, dbpointer, blocknumber);
}


int DBManager::decode_value(unsigned char* diskblock, uint32_t index, cprsubdb* dbpointer, int blocknumber, int cl)
{
    // The 'cl' parameter is no longer used in the generic version but is kept for API compatibility for now.
    return decode_value_generic<cprsubdb>(diskblock, index, dbpointer, blocknumber);
}

char* DBManager::read_entire_file(const char* fullpath, long* file_size) {
    FILE *fp = fopen(fullpath, "rb");
    if (fp == nullptr) {
        log_c(LOG_LEVEL_WARNING, "read_entire_file: Failed to open file: %s", fullpath);
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size == 0) {
        fclose(fp);
        *file_size = 0;
        // Return a valid empty string to prevent crashes in the caller
        char* empty_content = static_cast<char*>(malloc(1));
        if (empty_content) {
            empty_content[0] = '\0';
        }
        return empty_content;
    }

    char* content = static_cast<char*>(malloc(size + 1));
    if (content == nullptr) {
        log_c(LOG_LEVEL_FATAL, "read_entire_file: Failed to malloc %ld bytes.", size + 1);
        fclose(fp);
        return nullptr;
    }

    if (fread(content, 1, size, fp) != size) {
        log_c(LOG_LEVEL_ERROR, "read_entire_file: Failed to read %ld bytes from %s.", size, fullpath);
        free(content);
        fclose(fp);
        return nullptr;
    }
    fclose(fp);
    content[size] = '\0';

    // Perform in-place sanitization
    long write_idx = 0;
    for (long read_idx = 0; read_idx < size; ++read_idx) {
        if (content[read_idx] == '\r' && (read_idx + 1 < size) && content[read_idx + 1] == '\n') {
            continue; // Skip the '\r'
        }
        content[write_idx++] = content[read_idx];
    }
    content[write_idx] = '\0';
    *file_size = write_idx;

    // Optional: shrink the allocation to the sanitized size to save memory.
    // realloc might return the same pointer or a new one.
    char* resized_content = static_cast<char*>(realloc(content, write_idx + 1));
    if (resized_content == nullptr) {
        log_c(LOG_LEVEL_WARNING, "read_entire_file: realloc failed, returning original potentially oversized buffer.");
        return content; // If realloc fails, return the original buffer
    }

    return resized_content;
}

bool DBManager::parse_single_base_entry(char **ptr_ref, int blockoffset, int fpcount, int* total_blocks_ptr, char* file_content) {
    return parse_single_base_entry_generic<cprsubdb>(ptr_ref, blockoffset, fpcount, total_blocks_ptr, file_content);
}

bool DBManager::parse_single_base_entry_mtc(char **ptr_ref, int blockoffset, int fpcount, int* total_blocks_ptr, char* file_content) {
    return parse_single_base_entry_generic<cprsubdb_mtc>(ptr_ref, blockoffset, fpcount, total_blocks_ptr, file_content);
}

int DBManager::parseindexfile_mtc(const char* EGTBdirectory, char idxfilename[256], int blockoffset, int fpcount) {
    return parseindexfile_generic<cprsubdb_mtc>(EGTBdirectory, idxfilename, blockoffset, fpcount);
}

int DBManager::parseindexfile(const char* EGTBdirectory, char idxfilename[256], int blockoffset, int fpcount) {
    return parseindexfile_generic<cprsubdb>(EGTBdirectory, idxfilename, blockoffset, fpcount);
}

int DBManager::internal_preload(char out[256], FILE *db_fp[], int fp_count) {
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

int DBManager::db_init(int suggestedMB, char out[256], const char* EGTBdirectory)
{
    QMutexLocker locker(&m_mutex);
    
    // Clear any previous EGDB status flags and set INIT_START
    clearProgramStatusWordFlags(0xFFFFFFFF); // Clear all flags to start fresh
    updateProgramStatusWord(STATUS_APP_START | STATUS_EGDB_INIT_START);
    log_c(LOG_LEVEL_INFO, "DBManager::db_init: Initializing EGDB from directory: %s", EGTBdirectory);

    initDecodeTable();

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

    strncpy(DBpath, EGTBdirectory, sizeof(DBpath) - 1);
    DBpath[sizeof(DBpath) - 1] = '\0';
	
    initBicoefTable();



    memset(cprsubdatabase, 0, DBManager::CPR_SUBDATABASE_TOTAL_SIZE);
    memset(cprsubdatabase_mtc, 0, sizeof(cprsubdatabase_mtc));
	
    for(n=2;n<=8;n++) // Check up to 8 pieces for maxpieces
    {
		snprintf(dbname, sizeof(dbname), "db%i.idx",n);
        snprintf(fullpath, sizeof(fullpath), "%s/%s", EGTBdirectory, dbname);
		fp = fopen(fullpath,"rb");
		if(fp)
		{
			if (n > pieces) {
			    pieces = n;
			}
            fclose(fp);
		}
    }

    this->maxpieces = pieces;
    log_c(LOG_LEVEL_INFO, "DBManager::db_init: Determined maxpieces = %d.", this->maxpieces);
    if (this->maxpieces > 0) {
        log_c(LOG_LEVEL_INFO, "EGDB found. Max pieces = %d.", this->maxpieces);
    } else {
        log_c(LOG_LEVEL_WARNING, "No EGDB files found.");
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL);
        return 0;
    }
    
    if (this->maxpieces > 8) this->maxpieces = 8;

    this->cachesize = suggestedMB << 10;
    if(this->maxpieces <6)
        this->cachesize = 2000;
    if(this->maxpieces == 6)
        this->cachesize = 42000;
    if(this->maxpieces > 6)
    {
        this->cachesize = (this->cachesize > MINCACHESIZE ? this->cachesize : MINCACHESIZE);
    }

    if (this->maxpieces > 0 && this->maxpieces < (int)(sizeof(db_configs) / sizeof(db_configs[0]))) {
        const DB_CONFIG* config = &db_configs[this->maxpieces];
        this->maxidx = config->maxidx;
        this->maxpiece = config->maxpiece;
        this->maxblocknum = this->cachesize; // Set maxblocknum to cachesize to prevent out-of-bounds access
    }
    
    blockoffset = 0;
    int cprFileCount_mtc = 0;
    int blockoffset_mtc = 0;

    for(n=2; n<=this->maxpieces; n++)
    {
        if(n>=SPLITSIZE)
            continue;
        processEgdbFiles(EGTBdirectory, n, 0, 0, 0, 0, blockoffset, cprFileCount, out);
        processEgdbFiles_mtc(EGTBdirectory, n, 0, 0, 0, 0, blockoffset_mtc, cprFileCount_mtc, out);
    }

    for (n=SPLITSIZE; n<=this->maxpieces; n++)
    {
        for (nb=this->maxpieces-this->maxpiece; nb<=this->maxpiece; nb++)
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
                    processEgdbFiles(EGTBdirectory, n, bm, bk, wm, wk, blockoffset, cprFileCount, out);
                    processEgdbFiles_mtc(EGTBdirectory, n, bm, bk, wm, wk, blockoffset_mtc, cprFileCount_mtc, out);
                }
            }
        }
    }

	memsize = this->cachesize << 10;
    cachebaseaddress = static_cast<unsigned char*>(malloc(memsize));
    
    if(cachebaseaddress == nullptr && memsize!=0)
    {
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL | STATUS_CRITICAL_ERROR); 
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for cachebaseaddress failed!");
        return 0;
    }
    
    memsize = this->maxblocknum*sizeof(unsigned char*);
    blockpointer = static_cast<unsigned char**>(malloc(memsize));
    if(blockpointer == nullptr && memsize != 0 )
    {
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL | STATUS_CRITICAL_ERROR); 
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for blockpointer failed!");
        free(cachebaseaddress);
        return 0;
    }
    for(i=0;i<this->maxblocknum;i++)
        blockpointer[i]=nullptr;
    
    memsize = this->cachesize*sizeof(struct bi);
    blockinfo = static_cast<struct bi*>(malloc(memsize));
    if(blockinfo == nullptr && memsize!=0)
    {
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL | STATUS_CRITICAL_ERROR); 
        log_c(LOG_LEVEL_ERROR, "dblookup: malloc for blockinfo failed!");
        free(cachebaseaddress);
        free(blockpointer);
        return 0;
    }
    
    autoloadnum = internal_preload(out, dbfp, cprFileCount);                                
    
    autoloadnum=0;
    for(i=autoloadnum;i<this->cachesize;i++)
    {
        blockinfo[i].next = i+1;
        blockinfo[i].prev = i-1;
        blockinfo[i].uniqueid = i;
    }
    if (this->cachesize > 0) {
        blockinfo[this->cachesize-1].next=-1;
        blockinfo[autoloadnum].prev=-1;
    }
    
    head=autoloadnum;
    tail = this->cachesize-1;
    
    updateProgramStatusWord(STATUS_EGDB_INIT_OK);
    log_c(LOG_LEVEL_INFO, "DBManager::db_init: EGDB initialization successful (PSW: 0x%08X). Returning maxpieces = %d.", getProgramStatusWord(), this->maxpieces);

	return this->maxpieces;
}

int DBManager::dblookup(bitboard_pos *q, int cl)
{
    // The 'cl' parameter is no longer used in the generic version but is kept for API compatibility for now.
    return dblookup_generic<cprsubdb>(q);
}

int DBManager::db_exit()
{
    QMutexLocker locker(&m_mutex);
    if (m_shutdown_has_been_called) {
        return 1;
    }
    m_shutdown_has_been_called = true;
    
    int i, bm, bk, wm, wk, bmrank, wmrank, color;

	    free(cachebaseaddress);
	    free(blockpointer);
	    free(blockinfo);     
	for(i=0;i<50;i++)
		{
		if(dbfp[i] != nullptr)
			fclose(dbfp[i]);
        if(dbfp_mtc[i] != nullptr)
            fclose(dbfp_mtc[i]);
		}

    // Free the idx arrays that were allocated with malloc
	for(i=0;i<50;i++)
		{
		if(dbfp[i] != nullptr)
			fclose(dbfp[i]);
        if(dbfp_mtc[i] != nullptr)
            fclose(dbfp_mtc[i]);
		}

    // Free the idx arrays that were allocated with malloc
    for (bm = 0; bm <= MAXPIECE; ++bm) {
        for (bk = 0; bk <= MAXPIECE; ++bk) {
            for (wm = 0; wm <= MAXPIECE; ++wm) {
                for (wk = 0; wk <= MAXPIECE; ++wk) {
                    for (bmrank = 0; bmrank < MAX_RANK_COUNT; ++bmrank) {
                        for (wmrank = 0; wmrank < MAX_RANK_COUNT; ++wmrank) {
                            for (color = 0; color < 2; ++color) {
                                if (cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx != nullptr) {
                                    free(cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx);
                                    cprsubdatabase[bm][bk][wm][wk][bmrank][wmrank][color].idx = nullptr; // Avoid double free
                                }
                                if (cprsubdatabase_mtc[bm][bk][wm][wk][bmrank][wmrank][color].idx != nullptr) {
                                    free(cprsubdatabase_mtc[bm][bk][wm][wk][bmrank][wmrank][color].idx);
                                    cprsubdatabase_mtc[bm][bk][wm][wk][bmrank][wmrank][color].idx = nullptr; // Avoid double free
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

int DBManager::getCacheSize()
{
    QMutexLocker locker(&m_mutex);
    return cachesize;
}

int DBManager::decode_value_mtc(unsigned char* diskblock, uint32_t index, cprsubdb_mtc* dbpointer, int blocknumber)
{
    return decode_value_generic<cprsubdb_mtc>(diskblock, index, dbpointer, blocknumber);
}

int DBManager::dblookup_mtc(bitboard_pos *q)
{
    return dblookup_generic<cprsubdb_mtc>(q);
}

void DBManager::getInfoString(char *str)
{
    QMutexLocker locker(&m_mutex);
    sprintf(str, "\nDatabase Details:\n%s", dbinfo);
}

int64_t DBManager::getDatabaseSize(int bm, int bk, int wm, int wk, int bmrank, int wmrank)
{
    QMutexLocker locker(&m_mutex);
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
	
	QString DBManager::getEGDBStatus() const {
	    QString status;
	    uint32_t psw = getProgramStatusWord();	
	    if (psw & STATUS_CRITICAL_ERROR) status += "CRITICAL_ERROR; ";
	    if (psw & STATUS_FILE_IO_ERROR) status += "FILE_IO_ERROR; ";
	
	    if (psw & STATUS_EGDB_INIT_START) status += "EGDB_INIT_START; ";
	    if (psw & STATUS_EGDB_INIT_OK) status += "EGDB_INIT_OK; ";
	    if (psw & STATUS_EGDB_INIT_FAIL) status += "EGDB_INIT_FAIL; ";
	
	    if (psw & STATUS_EGDB_LOOKUP_ATTEMPT) status += "EGDB_LOOKUP_ATTEMPT; ";
	    if (psw & STATUS_EGDB_LOOKUP_HIT) status += "EGDB_LOOKUP_HIT; ";
	    if (psw & STATUS_EGDB_LOOKUP_MISS) status += "EGDB_LOOKUP_MISS; ";
	    if (psw & STATUS_EGDB_UNEXPECTED_VALUE) status += "EGDB_UNEXPECTED_VALUE; ";
	    if (psw & STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS) status += "EGDB_LOOKUP_OUT_OF_BOUNDS; ";
	    if (psw & STATUS_EGDB_LOOKUP_NOT_PRESENT) status += "EGDB_LOOKUP_NOT_PRESENT; ";
	    if (psw & STATUS_EGDB_LOOKUP_INVALID_INDEX) status += "EGDB_LOOKUP_INVALID_INDEX; ";
	    if (psw & STATUS_EGDB_SINGLE_VALUE_HIT) status += "EGDB_SINGLE_VALUE_HIT; ";
	    if (psw & STATUS_EGDB_WIN_RESULT) status += "EGDB_WIN_RESULT; ";
	    if (psw & STATUS_EGDB_LOSS_RESULT) status += "EGDB_LOSS_RESULT; ";
	    if (psw & STATUS_EGDB_DRAW_RESULT) status += "EGDB_DRAW_RESULT; ";
	    if (psw & STATUS_EGDB_UNKNOWN_RESULT) status += "EGDB_UNKNOWN_RESULT; ";
	    if (psw & STATUS_EGDB_DISK_READ_ERROR) status += "EGDB_DISK_READ_ERROR; ";
	    if (psw & STATUS_EGDB_DECODE_ERROR) status += "EGDB_DECODE_ERROR; ";
	    if (psw & STATUS_APP_START) status += "APP_START; ";
	
	    if (status.isEmpty()) {
	        return "No specific EGDB status flags set.";
	    } else {
	        return status.trimmed(); // Remove trailing space if any
	    }
	}
	