#pragma once

#include <QObject>
#include <QMutex>
#include "checkers_types.h"

class DBManager : public QObject
{
    Q_OBJECT

public:
    static DBManager* instance();
    ~DBManager();

    int db_init(int suggestedMB, char out[256], const char* EGTBdirectory);
    int dblookup(pos *q, int cl);
    int db_exit();
    int64_t getDatabaseSize(int bm, int bk, int wm, int wk, int bmrank, int wmrank);
    void getInfoString(char *str);
    int getCacheSize();


private:
    explicit DBManager(QObject *parent = nullptr);
    bool processEgdbFiles(const char* EGTBdirectory, int nPieces, int bm, int bk, int wm, int wk, int& blockOffset, int& cprFileCount, char* outBuffer);

    void initDecodeTable();
    void initBicoefTable();
    void initRunLengthAndValueTables();
    static DBManager* m_instance;
    QMutex m_mutex;

    // All the static global variables from dblookup.cpp are now private members
    cprsubdb cprsubdatabase[MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][7][7][2]; 
    unsigned char **blockpointer;	
    unsigned char *cachebaseaddress; 
    struct bi
    {
        int uniqueid; 
        int next;     
        int prev;     
    } *blockinfo;
    int head, tail; 
    
    static const int skip[SKIPS]; // Declared as static const
    int runlength[256]; 
    int value[256];	
    int bicoef[33][33]; 
    int maxblockid;  
    FILE *dbfp[MAXFP]; 
    char dbnames[MAXFP][256]; 
    char DBpath[256];

    int maxblocknum;
    int maxidx;
    int maxpieces;
    int maxpiece;
    int cachesize;
    int bytesallocated;

    char dbinfo[1024];
    int decode_table[81][4];

    struct DB_CONFIG {
        int maxidx;
        int maxblocknum;
        int maxpieces;
        int maxpiece;
    };

    static const DB_CONFIG db_configs[];

    // Helper functions are now private methods
    int choose(int n, int k);
    int parseindexfile(const char* EGTBdirectory, char idxfilename[256], int blockoffset, int fpcount);
    uint32_t calculate_lsb_index(uint32_t pieces, uint32_t occupied_mask);
    uint32_t calculate_msb_index(uint32_t pieces);
    uint32_t calculate_index(const pos& p, int bm, int bk, int wm, int wk, int bmrank, int wmrank);
    void move_cache_block_to_head(int block_index);
    unsigned char* get_disk_block(int uniqueblockid, cprsubdb* dbpointer, int blocknumber, int cl);
    int decode_value(unsigned char* diskblock, uint32_t index, cprsubdb* dbpointer, int blocknumber);
    int internal_preload(char out[256], FILE *db_fp[], int fp_count);
    char* read_entire_file(const char* fullpath, long* file_size);
    bool parse_single_base_entry(char **ptr_ref, int blockoffset, int fpcount, int* total_blocks_ptr, char* file_content);
    
    // Named constants for magic numbers
    static const int MAX_RANK_COUNT = 7;
    static const int DECODE_TABLE_SIZE = 81;
    static const int DECODE_TABLE_ENTRY_SIZE = 4;
    static const int CPR_SUBDATABASE_MULTIPLIER = 98; // Multiplier for memset size calculation
    static const size_t CPR_SUBDATABASE_TOTAL_SIZE = (MAXPIECE+1ULL)*(MAXPIECE+1ULL)*(MAXPIECE+1ULL)*(MAXPIECE+1ULL)*DBManager::CPR_SUBDATABASE_MULTIPLIER*sizeof(cprsubdb);
};