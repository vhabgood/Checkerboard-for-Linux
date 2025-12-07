#pragma once
#include <cstdint> // Required for uint32_t
#include <vector>  // Required for std::vector

#ifdef __cplusplus
#include <QMetaType> // Required for Q_DECLARE_METATYPE
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Define constants for EGDB and general engine use
#define MAXPIECE 7     // Max pieces per side (from OLD/builddb4/checkers.h, used in dblookup)
#define SKIPS 58       // Number of skips in Tunstall coding (from OLD/dblookup.cpp)
#define MAXFP 12       // Max number of open file pointers for EGDB (a reasonable default)
#define MAXSKIP 7500   // Max skip value (from OLD/dblookup.cpp)

// Program Status Word (PSW) bit flags
#define STATUS_CRITICAL_ERROR      (1 << 0)  // General critical error, usually leads to shutdown
#define STATUS_FILE_IO_ERROR       (1 << 1)  // Error during file read/write
#define STATUS_EGDB_INIT_START     (1 << 2)  // EGDB initialization has started
#define STATUS_EGDB_INIT_OK        (1 << 3)  // EGDB initialization completed successfully
#define STATUS_EGDB_INIT_FAIL      (1 << 4)  // EGDB initialization failed
#define STATUS_EGDB_LOOKUP_ATTEMPT (1 << 5)  // An EGDB lookup was attempted
#define STATUS_EGDB_LOOKUP_HIT     (1 << 6)  // An EGDB lookup resulted in a hit (found a value)
#define STATUS_EGDB_LOOKUP_MISS    (1 << 7)  // An EGDB lookup resulted in a miss (no value found)
#define STATUS_EGDB_UNEXPECTED_VALUE (1 << 8) // EGDB returned an unexpected numerical value
#define STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS (1 << 9) // EGDB lookup failed due to piece count out of bounds
#define STATUS_EGDB_LOOKUP_NOT_PRESENT (1 << 10) // EGDB lookup failed because sub-database was not present
#define STATUS_EGDB_LOOKUP_INVALID_INDEX (1 << 11) // EGDB lookup failed due to invalid index calculation
#define STATUS_EGDB_SINGLE_VALUE_HIT (1 << 12) // EGDB lookup returned a pre-calculated single value
#define STATUS_EGDB_WIN_RESULT     (1 << 13) // EGDB lookup resulted in a Win
#define STATUS_EGDB_LOSS_RESULT    (1 << 14) // EGDB lookup resulted in a Loss
#define STATUS_EGDB_DRAW_RESULT    (1 << 15) // EGDB lookup resulted in a Draw
#define STATUS_EGDB_UNKNOWN_RESULT (1 << 16) // EGDB lookup resulted in an Unknown value (after full processing)
#define STATUS_EGDB_DISK_READ_ERROR (1 << 17) // Error reading a disk block for EGDB
#define STATUS_EGDB_DECODE_ERROR (1 << 18) // Error during EGDB data decoding
#define STATUS_APP_START           (1 << 19)  // Application has started

// Getter and setter for g_programStatusWord
uint32_t getProgramStatusWord();
void setProgramStatusWord(uint32_t value);
void updateProgramStatusWord(uint32_t flags_to_set);
void clearProgramStatusWordFlags(uint32_t flags_to_clear);

// EGDB specific constants for results and colors, aligning with CB_* where possible
#define DB_BLACK        8       // Aligns with CB_BLACK
#define DB_WHITE        4       // Aligns with CB_WHITE
#define DB_WIN          1       // Aligns with CB_WIN
#define DB_LOSS         2       // Aligns with CB_LOSS
#define DB_DRAW         3       // Aligns with CB_DRAW
#define DB_UNKNOWN      4       // Aligns with CB_UNKNOWN

// Additional EGDB specific constants
#define DB_NOT_LOOKED_UP -1     // Indicates EGDB lookup did not return a value

// AI Scoring Constants
#define WIN_SCORE        1000000
#define LOSS_SCORE      -1000000
#define DRAW_SCORE             0

// MTC specific constants
#define MTC_UNKNOWN_VALUE      0   // dblookup_mtc returns 0 if no MTC value is found
#define MTC_MAX_MOVES_TO_CONSIDER 20 // Max moves to win/loss to consider for direct influence on score
#define MTC_WIN_VALUE_BASE     900000 // Base score for MTC win, adjusted by moves
#define MTC_LOSS_VALUE_BASE   -900000 // Base score for MTC loss, adjusted by moves


// Max path length for cross-platform compatibility
#define MAX_PATH_FIXED 1024

// EGDB specific constants from old DBLookup.cpp
#define MINCACHESIZE (1 << 16)  // Minimum size for EGDB cache
#define SPLITSIZE 6             // Piece count threshold for splitting EGDB files

// EGDB configuration for different piece counts
// These are derived from analysis of old build scripts and EGDB generation logic
#define MAXIDX2 1
#define BLOCKNUM2 1
#define MAXIDX3 1
#define BLOCKNUM3 1
#define MAXIDX4 1
#define BLOCKNUM4 1
#define MAXIDX5 1
#define BLOCKNUM5 1
#define MAXIDX6 1
#define BLOCKNUM6 1
#define MAXIDX7 1
#define BLOCKNUM7 1
#define MAXIDX8 1
#define BLOCKNUM8 1

// Bit manipulation utility functions/macros
// Using GCC/Clang builtins for performance where available

// Count set bits (Population Count)
#ifdef __GNUC__
#define recbitcount(x) __builtin_popcount(x)
#else
// Fallback for other compilers (simple loop, less performant)
static inline int recbitcount_fallback(uint32_t n) {
    int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}
#define recbitcount(x) recbitcount_fallback(x)
#endif

// Revert bits (reverse order of bits)
static inline uint32_t revert(uint32_t n) {
    uint32_t reversed_n = 0;
    for (int i = 0; i < 32; ++i) {
        if ((n >> i) & 1) {
            reversed_n |= (1 << (31 - i));
        }
    }
    return reversed_n;
}

// Least Significant Bit (position of first set bit, 0-indexed)
#ifdef __GNUC__
#define LSB(x) ((x) == 0 ? -1 : __builtin_ctz(x))
#else
// Fallback (simple loop)
static inline int LSB_fallback(uint32_t n) {
    if (n == 0) return -1;
    int count = 0;
    while ((n & 1) == 0) {
        n >>= 1;
        count++;
    }
    return count;
}
#define LSB(x) LSB_fallback(x)
#endif

// Most Significant Bit (position of last set bit, 0-indexed)
#ifdef __GNUC__
#define MSB(x) ((x) == 0 ? -1 : (31 - __builtin_clz(x))) // For 32-bit int
#else
// Fallback (simple loop)
static inline int MSB_fallback(uint32_t n) {
    if (n == 0) return -1;
    int count = 0;
    while (n > 1) {
        n >>= 1;
        count++;
    }
    return count;
}
#define MSB(x) MSB_fallback(x)
#endif


// Coordinate struct (C++ style)
struct coor {
    int x, y;
    int field;

    // Default constructor
    coor() : x(0), y(0), field(0) {}

    // Parameterized constructor
    coor(int new_x, int new_y, int new_field) : x(new_x), y(new_y), field(new_field) {}
};

// Bitboard position (for internal engine use and EGDB)
typedef struct {
    unsigned int bm; // Black men
    unsigned int bk; // Black kings
    unsigned int wm; // White men
    unsigned int wk; // White kings
    int color; // Side to move for EGDB, usually DB_BLACK or DB_WHITE
} bitboard_pos;

// Board representation (8x8 array)
typedef struct {
    int board[8][8];
} Board8x8;

// WLD specific sub-database structure (from OLD/egdb_wld_runlen.cpp)
typedef struct {
    uint32_t num_blocks;		/* number of blocks in the database. */
    uint32_t first_block_id;	/* first block ID */
    uint32_t data_offset;		/* offset of data for this subdb in file */
    uint32_t min_block_len;     /* minimum block length */
    uint32_t max_block_len;     /* maximum block length */
    unsigned char* data;        /* pointer to loaded data if memory mapped or preloaded */

    // Additional members required by DBManager.cpp
    int fp;                     // File pointer index for this sub-database
    int* idx;                   // Vector of indices into the block data
    int idx_size;               // Size of the idx array
    int blockoffset;            // Offset in the block
    int value;                  // Value if the sub-database is a single value (e.g., all draws)
    int ispresent;              // Flag if the sub-database is present
    uint32_t startbyte;         // Starting byte in the file for this sub-database's data
    int haspartials;            // Flag indicating if this sub-database has partials
} cprsubdb;

// MTC specific sub-database structure (from OLD/egdb_dtw.cpp)
typedef struct {
    uint32_t num_blocks;
    uint32_t first_block_id;
    uint32_t data_offset;
    uint32_t min_block_len;
    uint32_t max_block_len;
    unsigned char* data;
    uint32_t min_value; // Minimum MTC value
    uint32_t max_value; // Maximum MTC value

    // Additional members required by DBManager.cpp
    int fp;                     // File pointer index for this sub-database
    int* idx;                   // Vector of indices into the block data
    int idx_size;               // Size of the idx array
    int blockoffset;            // Offset in the block
    int value;                  // Value if the sub-database is a single value
    int ispresent;              // Flag if the sub-database is present
    uint32_t startbyte;         // Starting byte in the file for this sub-database's data
    int haspartials;            // Flag indicating if this sub-database has partials
} cprsubdb_mtc;


#ifdef __cplusplus
}
#endif

// New namespace for bitboard-related constants
namespace BitboardConstants {
    extern char bitsinword[65536];
    extern uint32_t revword[65536];
} // namespace BitboardConstants

#ifdef __cplusplus
// Q_DECLARE_METATYPE declarations must be outside extern "C" blocks
Q_DECLARE_METATYPE(coor)
Q_DECLARE_METATYPE(bitboard_pos)
Q_DECLARE_METATYPE(Board8x8)
Q_DECLARE_METATYPE(cprsubdb)
Q_DECLARE_METATYPE(cprsubdb_mtc)
#endif