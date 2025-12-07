#ifndef DB_IO_H
#define DB_IO_H

#include "egdb/egdb_types.h"
#include <cstdint>

namespace egdb_interface {

int get_db_data_block(DBHANDLE h, struct INDEX_REC *idx_rec, int blocknumber, uint8_t **db_data_block, int *cache_index = nullptr);

} // namespace egdb_interface

#endif
