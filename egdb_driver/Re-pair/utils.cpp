#include <cmath>
#include "repair.h"


namespace egdb_interface {

double get_entropy(unsigned int *frequencies, unsigned int max_value, unsigned int total_symbols)
{
	double entropy = 0.0;
	double probability;

	for (unsigned int i = 0; i < max_value; ++i) {
		if (frequencies[i] == 0)
			continue;
		probability = (double)frequencies[i] / (double)total_symbols;
		entropy -= probability * std::log2(probability);
	}
	return(entropy);
}


unsigned int get_symbol_length(unsigned int value)
{
	if (value <= 255)
		return(1);
	return(2);
}

}	// namespace egdb_interface
