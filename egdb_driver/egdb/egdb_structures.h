#pragma once

#include <cstdint>
#include <cstdio> // For FILE_HANDLE
#include "platform.h" // For FILE_HANDLE

#include "egdb.h" // For EGDB_TYPE

namespace egdb_interface {

// WLD specific sub-database structure (from OLD/egdb_wld_runlen.cpp)
struct CPRSUBDB {
    uint32_t numberofblocks;	/* number of blocks in the database. */
    uint32_t firstblock;	/* first block ID */
    uint32_t data_offset;		/* offset of data for this subdb in file */
    uint32_t min_block_len;     /* minimum block length */
    uint32_t max_block_len;     /* maximum block length */
    unsigned char* data;        /* pointer to loaded data if memory mapped or preloaded */

    // Additional members required by DBManager.cpp
    FILE_HANDLE file;           // File handle for this sub-database
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
    FILE_HANDLE file;           // File handle for this sub-database
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

// Checkpoint structure for WLD and MTC
struct Checkpoint {
    uint64_t index;
    uint64_t file_offset;
    uint8_t initial_byte;
};

// INDEX_REC struct based on usage in egdb_wld_runlen.cpp
typedef struct INDEX_REC {
    struct INDEX_REC *next;
    FILE *file;
    uint64_t offset_in_file;
    int num_dbs;
    int num_bmen;
    int num_bkings;
    int num_wmen;
    int num_wkings;
    int side_to_move;
    int bmrank;
    int wmrank;
    int file_num;
    unsigned int first_block_id;
    unsigned int startbyte;
    Checkpoint *checkpoints;
    int num_checkpoints;
    int initial_value; // Added for MTC
} INDEX_REC;

} // namespace egdb_interface