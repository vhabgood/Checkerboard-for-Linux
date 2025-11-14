#pragma once
#include "dblookup.h" // Include the Kingsrow EGDB API

#ifdef __cplusplus
extern "C" {
#endif

// Function to initialize the Kingsrow EGDB
// Returns 0 on success, non-zero on failure
int egdb_init(const char* egdb_path);

// Function to perform an EGDB lookup
// Returns a result code (e.g., win, loss, draw)
int egdb_lookup(const char* fen_position);

#ifdef __cplusplus
}
#endif
