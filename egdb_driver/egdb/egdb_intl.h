#pragma once

#include <cstdint>  // uint64_t
#include <cstdio>   // FILE

#include "egdb.h" // For EGDB_TYPE
// #include "egdb/egdb_common.h" // Removed this include

#include "egdb_types.h" // Added this include

namespace egdb_interface {
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
}	// namespace egdb_interface