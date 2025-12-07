#ifndef REPAIR_H
#define REPAIR_H

#include <cstdint>


namespace egdb_interface {

double get_entropy(unsigned int *frequencies, unsigned int max_value, unsigned int total_symbols);
unsigned int get_symbol_length(unsigned int value);

}	// namespace egdb_interface

#endif
