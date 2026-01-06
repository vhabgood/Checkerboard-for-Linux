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
    if (block_index == h->cache_head) return;

    // Unlink
    int prev = h->cache_block_info[block_index].prev;
    int next = h->cache_block_info[block_index].next;

    if (block_index == h->cache_tail) {
        h->cache_tail = next;
        if (h->cache_tail != -1) h->cache_block_info[h->cache_tail].prev = -1;
    } else {
        if (prev != -1) h->cache_block_info[prev].next = next;
        if (next != -1) h->cache_block_info[next].prev = prev;
    }

    // Link at Head
    h->cache_block_info[block_index].next = -1;
    h->cache_block_info[block_index].prev = h->cache_head;
    
    if (h->cache_head != -1) h->cache_block_info[h->cache_head].next = block_index;
    h->cache_head = block_index;
    
    // Handle empty list case (though strictly shouldn't happen here if initialized)
    if (h->cache_tail == -1) h->cache_tail = block_index;
}

int get_db_data_block(DBHANDLE h, struct INDEX_REC *idx_rec, int blocknumber, uint8_t **db_data_block, int *cache_index)
{
    // blocknumber is absolute 4096-byte block number in the file.
    // Use a 64-bit unique ID to prevent overflow and collisions.
    uint64_t unique_id = ((uint64_t)idx_rec->file_num << 32) | (uint32_t)blocknumber;
    
    // Search in cache
    for (int i = 0; i < MAXCACHEDBLOCKS; ++i) {
        if (h->cache_block_info[i].unique_id_64 == unique_id) {
            *db_data_block = h->cache_base + (i * 4096);
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

    uint8_t *diskblock = h->cache_base + (victim_index * 4096);
    
    if (!idx_rec->file) {
        return 1;
    }

    // Seek and read 4096 bytes
    if (fseek(idx_rec->file, (long long)blocknumber * 4096, SEEK_SET) != 0) {
        log_c(LOG_LEVEL_ERROR, "get_db_data_block: fseek failed.");
        return 1;
    }

    size_t bytes_read = fread(diskblock, 1, 4096, idx_rec->file);
    if (bytes_read == 0) {
        // log_c(LOG_LEVEL_ERROR, "get_db_data_block: fread read 0 bytes (EOF?).");
        return 1;
    }
    
    h->cache_block_info[victim_index].unique_id_64 = unique_id;
    h->cache_block_info[victim_index].bytes_in_block = (int)bytes_read;
    *db_data_block = diskblock;
    if (cache_index) *cache_index = victim_index;
    move_cache_block_to_head(h, victim_index);

    return 0; // Success
}

} // namespace egdb_interface
