#ifndef BITCOUNT_H
#define BITCOUNT_H

#include "engine/project.h"
#include <cstdint> // For uint64_t

namespace egdb_interface {

extern unsigned int bitsinword[65536];
extern unsigned int bitcount_table_built;


void init_bitcount(void);
extern int bit_pop_count64(uint64_t x); // Declaration for bit_pop_count64
}	// namespace egdb_interface

#endif