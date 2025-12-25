#pragma once
#include <cstdint>

typedef struct {
	unsigned short *runlength_table;
	unsigned short *value_runs;
} DECOMPRESS_CATALOG;

namespace egdb_interface {

void decompress_wld_tunstall_v1(uint8_t *cpr_block,
								unsigned int block_len,
								uint64_t lsb_index,
								uint64_t msb_index,
								int *value);

void decompress_wld_tunstall_v2(uint8_t *cpr_block,
								unsigned int block_len,
								uint64_t lsb_index,
								uint64_t msb_index,
								int *value);

} // namespace egdb_interface
