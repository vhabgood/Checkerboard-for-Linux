#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "egdb/db_io.h"
#include "egdb/egdb_common.h"
#include "egdb/egdb_structures.h"
#include "egdb/egdb_types.h"
#include "log.h"

namespace egdb_interface {

static void move_cache_block_to_head(DBHANDLE h, int block_index)
{
    if (block_index == h->cache_head) {
        return;
    }

    int prev = h->cache_block_info[block_index].prev;
    int next = h->cache_block_info[block_index].next;

    if (next != -1) {
        h->cache_block_info[next].prev = prev;
    } else { // It was the tail
        h->cache_tail = prev;
    }

    if (prev != -1) {
        h->cache_block_info[prev].next = next;
    }

    h->cache_block_info[block_index].next = h->cache_head;
    h->cache_block_info[block_index].prev = -1;
    
    if (h->cache_head != -1) {
        h->cache_block_info[h->cache_head].prev = block_index;
    }

    h->cache_head = block_index;
}

int get_db_data_block(DBHANDLE h, struct INDEX_REC *idx_rec, int blocknumber, uint8_t **db_data_block, int *cache_index)
{
    // Use file_num + first_block_id + blocknumber as a unique ID.
    // file_num is shifted to ensure no collision between files.
    int unique_id = (idx_rec->file_num << 20) | (idx_rec->first_block_id + blocknumber);
    
    // Search in cache
    for (int i = 0; i < MAXCACHEDBLOCKS; ++i) {
        if (h->cache_block_info[i].unique_id == unique_id) {
            *db_data_block = h->cache_base + (i * 1024);
            if (cache_index) *cache_index = i;
            move_cache_block_to_head(h, i);
            return 0; // Cache hit
        }
    }

    // Cache miss - use the tail block
    int victim_index = h->cache_tail;
    if (victim_index == -1) {
        log_c(LOG_LEVEL_ERROR, "get_db_data_block: cache is full and tail is -1.");
        return 1;
    }

    uint8_t *diskblock = h->cache_base + (victim_index * 1024);
    
    if (!idx_rec->file) {
        return 1;
    }

    // Seek and read 1024 bytes (one block)
    // Offset is (first_block_id + blocknumber) * 1024
    if (fseek(idx_rec->file, (long long)(idx_rec->first_block_id + blocknumber) * 1024, SEEK_SET) != 0) {
        log_c(LOG_LEVEL_ERROR, "get_db_data_block: fseek failed.");
        return 1;
    }

    size_t bytes_read = fread(diskblock, 1, 1024, idx_rec->file);
    if (bytes_read == 0) {
        log_c(LOG_LEVEL_ERROR, "get_db_data_block: fread read 0 bytes (EOF?).");
        return 1;
    }
    
    h->cache_block_info[victim_index].unique_id = unique_id;
    h->cache_block_info[victim_index].bytes_in_block = (int)bytes_read;
    *db_data_block = diskblock;
    if (cache_index) *cache_index = victim_index;
    move_cache_block_to_head(h, victim_index);

    return 0; // Success
}

} // namespace egdb_interface
