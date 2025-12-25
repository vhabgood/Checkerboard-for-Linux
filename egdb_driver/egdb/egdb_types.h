#pragma once

#include <cstdint>  // uint64_t
#include <cstdio>   // FILE
#include "core_types.h"

#include "egdb.h" // For EGDB_TYPE
#include "egdb_structures.h" // Added this include

namespace egdb_interface {

    typedef struct {
        int prev;
        int next;
        uint64_t unique_id_64;
        int bytes_in_block; // Track actual bytes read
    } BLOCK_INFO;

    typedef struct DB_HANDLE_T {
        EGDB_TYPE db_type;
        int max_pieces;
        int compression_type;
        uint8_t *cache_base;
        unsigned int cache_size;
        uint8_t **cache_block_ptr;
        BLOCK_INFO *cache_block_info;
        int cache_head;
        int cache_tail;
        struct CPRSUBDB *cprsubdb;
        char path[256]; // For the path to the EGDB files
        FILE* files[MAXFP]; // Array of open file pointers
        int num_files;      // Number of open files
    } DB_HANDLE_T;

    typedef struct DB_HANDLE_T* DBHANDLE;

    // Missing defines
    #define MAXCACHEDBLOCKS 1024
    #define SPLIT_POINT_RUNLEN 11
    #define SPLIT_POINT_TUNSTALL_V1 6 // Added this line

    #define TAIL 0
    #define HEAD (MAXCACHEDBLOCKS - 1)

}	// namespace egdb_interface
