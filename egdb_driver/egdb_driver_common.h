#pragma once

#include <cstdint> // For uint64_t
#include "core_types.h"

/* Prevent name mangling of exported dll publics. */
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* Dont define EGDB_EXPORTS when using this library.
 * Only define it if you are actually compiling the library source code.
 */
#define EGDB_API EXTERNC

namespace egdb_interface {

typedef struct egdb_driver EGDB_DRIVER;

typedef EGDB_DRIVER *EGDB_DRIVER_HANDLE;

typedef enum {
    EGDB_ERR_NORMAL,
    EGDB_INVALID_HANDLE,
    EGDB_DB_NOT_LOADED,
    EGDB_NUM_PIECES_TOO_LARGE,
	EGDB_NUM_PIECES_TOO_SMALL, // Added this
    EGDB_NUM_PIECES_OUT_OF_BOUNDS,
    EGDB_INVALID_POS,
	EGDB_POS_IS_CAPTURE, // Added this
    EGDB_INDEX_OUT_OF_BOUNDS,
    EGDB_DECOMPRESSION_FAILED,
    EGDB_FILE_READ_ERROR,
    EGDB_UNKNOWN_DB_TYPE,
    EGDB_NO_MEM,
    EGDB_INVALID_DB_TYPE
} EGDB_ERR;


/* Color definitions. */
enum EGDB_COLOR {
	EGDB_BLACK = 0,
	EGDB_WHITE = 1
};

#define EGDB_BLACK_TO_MOVE 0 // Added this
#define EGDB_WHITE_TO_MOVE 1 // Added this


/* Values returned by handle->lookup(). */
enum LOOKUP_VALUE {
	EGDB_SUBDB_UNAVAILABLE = -2,/* this slice is not being used */
	EGDB_NOT_IN_CACHE = -1,		/* conditional lookup and position not in cache. */
	EGDB_UNKNOWN = 0,			/* value not in the database. */
	EGDB_WIN = 1,
	EGDB_LOSS = 2,
	EGDB_DRAW = 3,
	EGDB_DRAW_OR_LOSS = 4,
	EGDB_WIN_OR_DRAW = 5,
};

enum MTC_VALUE {
	MTC_UNKNOWN = 0,
	MTC_LESS_THAN_THRESHOLD = 1,
	MTC_THRESHOLD = 10,
};

enum WLD_VALUE {
	WLD_SUBDB_UNAVAILABLE = -2,/* this slice is not being used */
	WLD_NOT_IN_CACHE = -1,		/* conditional lookup and position not in cache. */
	WLD_UNKNOWN = 0,			/* value not in the database. */
	WLD_WIN = 1,
	WLD_LOSS = 2,
	WLD_DRAW = 3,
	WLD_DRAW_OR_LOSS = 4,
	WLD_WIN_OR_DRAW = 5,
};

enum EGDB_COMPRESSION {
	EGDB_COMPRESSION_NONE,
	EGDB_COMPRESSION_RUNLEN,
	EGDB_COMPRESSION_HUFFMAN,
	EGDB_COMPRESSION_TUNSTALL_V1,
	EGDB_COMPRESSION_TUNSTALL_V2
};

typedef enum {
	EGDB_NONE = -1, // Added this
	EGDB_KINGSROW_WLD = 0,			/* obsolete. */
	EGDB_KINGSROW_MTC,				/* obsolete. */
	EGDB_CAKE_WLD,
	EGDB_CHINOOK_WLD,
	EGDB_KINGSROW32_WLD,
	EGDB_KINGSROW32_MTC,
	EGDB_CHINOOK_ITALIAN_WLD,
	EGDB_KINGSROW32_ITALIAN_WLD,
	EGDB_KINGSROW32_ITALIAN_MTC,
	EGDB_KINGSROW32_WLD_TUN,
	EGDB_KINGSROW32_ITALIAN_WLD_TUN,
	EGDB_KINGSROW_DTW,
	EGDB_KINGSROW_ITALIAN_DTW,
	EGDB_WLD_RUNLEN,
	EGDB_MTC_RUNLEN,
	EGDB_WLD_HUFFMAN,
	EGDB_WLD_TUN_V1,
	EGDB_WLD_TUN_V2,
	EGDB_DTW,
} EGDB_TYPE;

typedef enum {
	EGDB_NORMAL = 0,
	EGDB_ROW_REVERSED
} EGDB_BITBOARD_TYPE;

/* for database lookup stats. */
typedef struct {
	unsigned int lru_cache_hits;
	unsigned int lru_cache_loads;
	unsigned int autoload_hits;
	unsigned int db_requests;				/* total egdb requests. */
	unsigned int db_returns;				/* total egdb w/l/d returns. */
	unsigned int db_not_present_requests;	/* requests for positions not in the db */
} EGDB_STATS;

typedef uint64_t BITBOARD; // Changed from EGDB_BITBOARD

struct EGDB_POSITION {
	BITBOARD black_pieces; // Changed from EGDB_BITBOARD black
	BITBOARD white_pieces; // Changed from EGDB_BITBOARD white
	BITBOARD king;         // Changed from EGDB_BITBOARD king
	int stm;
};

typedef struct {
	EGDB_TYPE type;
	unsigned int num_pieces;
	int compression;
	bool dtw_w_only;
	bool contains_le_pieces;
} EGDB_INFO;

/* The driver handle type */
struct egdb_driver {
	int (*lookup)(struct egdb_driver *handle, EGDB_BITBOARD *position, int color, int cl);
	void (*reset_stats)(struct egdb_driver *handle);
	EGDB_STATS *(*get_stats)(struct egdb_driver *handle);
	int (*verify)(struct egdb_driver *handle);
	int (*close)(struct egdb_driver *handle);
	void *internal_data;
	EGDB_TYPE db_type;
	int max_pieces;
	char path[256];
	unsigned int cache_size;
	EGDB_DRIVER_HANDLE handle;
	struct egdb_driver *next;
};


/* Open an endgame database driver. */
EGDB_API EGDB_DRIVER *egdb_open(EGDB_BITBOARD_TYPE bitboard_type,
								int pieces, 
								int cache_mb,
								const char *directory,
								void (*msg_fn)(char *));

EGDB_API int egdb_identify(const char *directory, EGDB_TYPE *egdb_type, int *max_pieces);

EGDB_API unsigned int egdb_version;

#define EGDB_DEFAULT_CACHE_SIZE (16 * 1024 * 1024)
#define MAX_PIECES_IN_DB 8 // Added this

// from egdb_intl.h
EGDB_DRIVER_HANDLE egdb_open(
    const char *db_path,
    unsigned int cache_size_mb,
    EGDB_TYPE db_type,
    EGDB_ERR *err_code);
EGDB_ERR egdb_close(EGDB_DRIVER_HANDLE handle);
int egdb_lookup(
    EGDB_DRIVER_HANDLE handle,
    const EGDB_POSITION *pos,
    EGDB_ERR *err_code);
unsigned int egdb_get_max_pieces(EGDB_DRIVER_HANDLE handle);
unsigned int egdb_get_info(
    EGDB_DRIVER_HANDLE handle,
    unsigned int num_pieces,
    EGDB_INFO *info,
    unsigned int max_info);

// from egdb_types.h moved to egdb/egdb_types.h

// from egdb_structures.h
// WLD specific sub-database structure (from OLD/egdb_wld_runlen.cpp)
struct CPRSUBDB {
    uint32_t numberofblocks;	/* number of blocks in the database. */
    uint32_t firstblock;	/* first block ID */
    uint32_t data_offset;		/* offset of data for this subdb in file */
    uint32_t min_block_len;     /* minimum block length */
    uint32_t max_block_len;     /* maximum block length */
    unsigned char* data;        /* pointer to loaded data if memory mapped or preloaded */

    // Additional members required by DBManager.cpp
    FILE* file;           // File handle for this sub-database
    int fp;                     // File pointer index for this sub-database
    int* idx;                   // Vector of indices into the block data
    int blockoffset;            // Offset in the block
    int value;                  // Value if the sub-database is a single value (e.g., all draws)
    bool ispresent;             // Flag if the sub-database is present (corrected from is_present)
    uint32_t startbyte;         // Starting byte in the file for this sub-database's data

    // Members from errors
    EGDB_TYPE db_type;          // From egdb_get_info_wld, initdblookup
    int compression;            // From egdb_get_info_wld, initdblookup
    bool contains_le_pieces;    // From egdb_get_info_wld
    int num_pieces;             // From initdblookup
    struct INDEX_REC *index_list; // From parseindexfile and dblookup
    bool haspartials;           // From dblookup
};

// MTC specific sub-database structure (from OLD/egdb_dtw.cpp)
struct CPRSUBDB_MTC {
    uint32_t numberofblocks;
    uint32_t firstblock;
    uint32_t data_offset;
    uint32_t min_block_len;
    uint32_t max_block_len;
    unsigned char* data;
    uint32_t min_value; // Minimum MTC value
    uint32_t max_value; // Maximum MTC value

    // Additional members required by DBManager.cpp
    FILE* file;           // File handle for this sub-database
    int fp;                     // File pointer index for this sub-database
    int* idx;                   // Vector of indices into the block data
    int blockoffset;            // Offset in the block
    int value;                  // Value if the sub-database is a single value
    bool ispresent;             // Flag if the sub-database is present
    uint32_t startbyte;         // Starting byte in the file for this sub-database's data

    // Members from errors (assuming similar to CPRSUBDB for MTC)
    EGDB_TYPE db_type;
    int compression;
    bool contains_le_pieces;
    int num_pieces;
    struct INDEX_REC *index_list;
    bool haspartials;
};

// INDEX_REC struct based on usage in egdb_wld_runlen.cpp
struct INDEX_REC {
    int file_num;
    uint64_t offset_in_file;
    int num_dbs;
    int num_bmen;
    int num_bkings;
    int num_wmen;
    int num_wkings;
    int side_to_move;
    INDEX_REC *next;
};

} // namespace egdb_interface
