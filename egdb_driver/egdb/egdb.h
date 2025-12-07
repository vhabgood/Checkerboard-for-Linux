#pragma once

#include <cstdint> // For uint64_t

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

/* This is Kingsrow's definition of a checkers position.
 * The bitboards use bit0 for square 1, bit 1 for square 2, ...,
 * For Italian checkers, the rows are reversed, so that bit 0 represents
 * Italian square 4, bit 1 represents square 3, bit 2 represents square 2, etc.
 */
typedef struct {
	uint64_t black; // Changed from unsigned int
	uint64_t white; // Changed from unsigned int
	uint64_t king;	// Changed from unsigned int
} EGDB_NORMAL_BITBOARD;

/* This is Cake's definition of a board position.
 * The bitboards use bit 3 for square 1, bit 2, for square 2, ...
 * This is repeated on each row of squares, thus bit7 for square 5, 
 * bit6 for square 6, bit 5 for square 7, ...
 * For Italian checkers, the rows are reversed, so that bit 0 represents
 * Italian square 1, bit 1 represents square 2, bit 2 represents square 3, etc.
 */
typedef struct {
	uint64_t black_man; // Changed from unsigned int
	uint64_t black_king; // Changed from unsigned int
	uint64_t white_man; // Changed from unsigned int
	uint64_t white_king; // Changed from unsigned int
} EGDB_ROW_REVERSED_BITBOARD;

typedef union {
	EGDB_NORMAL_BITBOARD normal;
	EGDB_ROW_REVERSED_BITBOARD row_reversed;
} EGDB_BITBOARD;

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
	EGDB_BITBOARD_TYPE bitboard_type;
	void (*msg_fn)(char *);
	EGDB_DRIVER_HANDLE handle;
	struct egdb_driver *next;
};


/* Open an endgame database driver. */
EGDB_API EGDB_DRIVER *egdb_open(EGDB_BITBOARD_TYPE bitboard_type,
								int pieces, 
								int cache_mb,
								const char *directory,
								void (*msg_fn)(char *));

/*
 * Identify which type of database is present, and the maximum number of pieces
 * for which it has data.
 * The return value of the function is 0 if a database is found, and its
 * database type and piece info are written using the pointer arguments.
 * The return value is non-zero if no database is found.  In this case the values
 * for egdb_type and max_pieces are undefined on return.
 */
EGDB_API int egdb_identify(const char *directory, EGDB_TYPE *egdb_type, int *max_pieces);

EGDB_API unsigned int egdb_version;



#define EGDB_DEFAULT_CACHE_SIZE (16 * 1024 * 1024)
#define MAX_PIECES_IN_DB 11 // Support up to 10 pieces
