#include <algorithm>
#include <cassert>
#include <list>
#include "bitbuf.h"
#include "huffman.h"


#include "bitbuf.h"
#include "huffman.h"

static int compare_codetable_entries(const void *p1, const void *p2)
{
    const Huffman::Codetable *entry1 = (const Huffman::Codetable *)p1;
    const Huffman::Codetable *entry2 = (const Huffman::Codetable *)p2;

    if (entry1->codelength > entry2->codelength) {
        return -1;
    } else if (entry1->codelength < entry2->codelength) {
        return 1;
    } else {
        // Code lengths are equal, compare by symbol values in increasing order
        if (entry1->value < entry2->value) {
            return -1;
        } else if (entry1->value > entry2->value) {
            return 1;
        } else {
            return 0;
        }
    }
}



void Huffman::get_frequencies(std::vector<Value> &input)
{
	Value maxvalue = 0;
	frequencies.resize(MAX_VALUES, 0);

	for (auto i = input.begin(); i != input.end(); ++i) {
		assert(*i < MAX_VALUES);
		++frequencies[*i];
		maxvalue = std::max(maxvalue, *i);
	}
	frequencies.resize(maxvalue + 1);
}

static void dbg(){}
std::list<Huffman::Huffnode *>::iterator Huffman::get_lowest_freq(std::list<Huffnode *> &list)
{
	std::list<Huffman::Huffnode *>::iterator lowest, i;
	
	assert(list.size() > 0);
	lowest = list.begin();
	i = lowest;
	for (++i; i != list.end(); ++i) {
		if ((*i)->freq < (*lowest)->freq || ((*i)->freq == (*lowest)->freq && (*i)->depth < (*lowest)->depth)) {
			if ((*i)->freq == (*lowest)->freq && (*i)->depth < (*lowest)->depth)
				dbg();
			lowest = i;
		}

	}
	return(lowest);
}


void Huffman::build_tree(void)
{
	Huffnode *node = nullptr;
	std::list<Huffnode *> nodes;

	for (auto i = frequencies.begin(); i != frequencies.end(); ++i) {
		if (*i == 0)
			continue;
		node = (Huffnode *)malloc(sizeof(Huffnode));
		assert(node != nullptr);
		node->value = (Value)(i - frequencies.begin());
		node->depth = 0;
		node->freq = *i;
		node->left = nullptr;
		node->right = nullptr;
		node->parent = nullptr;
		nodes.push_back(node);
	}

	while (nodes.size() > 1) {
		std::list<Huffnode *>::iterator low1, low2;
		Huffnode *n1, *n2;

		low1 = get_lowest_freq(nodes);
		n1 = *low1;
		nodes.erase(low1);
		low2 = get_lowest_freq(nodes);
		n2 = *low2;
		nodes.erase(low2);
		node = (Huffnode *)malloc(sizeof(Huffnode));
		assert(node != nullptr);
		node->value = INTERNAL_NODE;
		node->depth = 1 + std::max(n1->depth, n2->depth);
		node->freq = n1->freq + n2->freq;
		node->left = n1;
		n1->parent = node;
		node->right = n2;
		n2->parent = node;
		node->parent = nullptr;
		nodes.push_back(node);
	}
	tree = node;
}


void Huffman::get_codelengths(Huffnode *node, int depth)
{
	Codetable entry;

	if (!node)
		return;

	if (node->left != nullptr)
		get_codelengths(node->left, depth + 1);

	if (node->right != nullptr)
		get_codelengths(node->right, depth + 1);
	
	if (node->left == nullptr || node->right == nullptr || node->value != INTERNAL_NODE) {
		assert(node->value != INTERNAL_NODE);
		entry.codelength = std::max(depth, 1);	/* Handle degenerate case of 1 code. */
		entry.huffcode = 0;
		entry.value = node->value;
		entry.freq = frequencies[node->value];
		codetable.push_back(entry);
	}
}


void Huffman::get_codelengths(void)
{
	get_codelengths(tree, 0);
}


/*
 * Sort the table by code lengths and symbol values.
 * Code lengths in decreasing order.
 * Symbol values in increasing order (when code lengths are equal).
 */
void Huffman::sort_codetable_by_length(void)
{
	qsort(&codetable[0], codetable.size(), sizeof(Codetable), compare_codetable_entries);
}


void Huffman::delete_tree(Huffnode *node)
{
	if (node == nullptr)
		return;
	if (node->left != nullptr)
		delete_tree(node->left);
	if (node->right != nullptr)
		delete_tree(node->right);
	free(node);
}


std::string Huffman::binary_format(uint32_t code, int length, int formatted_width)
{
	int i;
	uint32_t mask;
	std::string result;

	code = left_justify32(code, length);
	for (i = 0; i < formatted_width; ++i) {
		mask = 1 << (i + 32 - formatted_width);
		if (mask & code)
			result = "1" + result;
		else
			result = "0" + result;
	}
	return(result);
}


void Huffman::print_codetable(void)
{
	for (size_t i = 0; i < codetable.size(); ++i)
		std::printf("sym %4d,  codelength %2d,  code 0x%08x (%s),  freq %10u\n", 
					codetable[i].value,
					codetable[i].codelength, 
					left_justify32(codetable[i].huffcode, codetable[i].codelength), 
					binary_format(codetable[i].huffcode, codetable[i].codelength, codetable[0].codelength).c_str(),
					codetable[i].freq);
}


/*
 * Assign Huffman code values to the huffcode field of codetable[].
 * codetable[] has been sorted by decreasing code lengths, and by increasing symbol values
 * when the lengths are equal.
 */
void Huffman::generate_codes(void)
{
	int last_length, length, code;

	code = 0;
	last_length = codetable[0].codelength;
	for (size_t i = 0; i < codetable.size(); ++i, ++code) {
		length = codetable[i].codelength;
		code >>= (last_length - length);
		last_length = length;
		codetable[i].huffcode = code;
	}
}


/*
 * Make a table that has the lowest huffman code of each length.
 * Used to decode huffman codes during decompression.
 */
void Huffman::build_length_table(void)
{
	int i;
	Lengthtable entry;

	/* codetable has the longest symbols first, so walk the table in reverse order. */
	entry.codelength = codetable[codetable.size() - 1].codelength;
	for (i = (int)codetable.size() - 2; i >= 0; --i) {
		if (codetable[i].codelength > entry.codelength) {
			entry.huffcode = left_justify32(codetable[i + 1].huffcode, entry.codelength);
			entry.codetable_index = i + 1;
			lengthtable.push_back(entry);
			entry.codelength = codetable[i].codelength;
		}

	}
	entry.huffcode = left_justify32(codetable[0].huffcode, entry.codelength);
	entry.codetable_index = 0;
	lengthtable.push_back(entry);
}


void uncompress_block(std::vector<uint32_t> &block, uint32_t ncodes, std::vector<Huffman::Value> &uncompressed,
					std::vector<Huffman::Lengthtable> &lengthtable, std::vector<Huffcode> &codetable)
{
	int i, symidx;
	uint32_t codes_read;
	Words32_64 codebuf;
	uint64_t temp;
	int bits_in_codebuf, block_index;

	uncompressed.clear();
	block_index = 0;
	codebuf.word32[1] = block[block_index++];
	codebuf.word32[0] = block[block_index++];
	bits_in_codebuf = 64;
	codes_read = 0;
	while (codes_read < ncodes) {
		if (bits_in_codebuf < 32 && block_index < (int)block.size()) {
			temp = block[block_index++];
			temp <<= (32 - bits_in_codebuf);
			codebuf.word64 |= temp;
			bits_in_codebuf += 32;
		}

		for (i = 0; codebuf.word32[1] < lengthtable[i].huffcode; ++i)
			;

		symidx = (codebuf.word32[1] - lengthtable[i].huffcode) >> (32 - lengthtable[i].codelength);
		symidx += lengthtable[i].codetable_index;
		uncompressed.push_back(codetable[symidx].value);
		++codes_read;
		codebuf.word64 <<= lengthtable[i].codelength;
		bits_in_codebuf -= lengthtable[i].codelength;
	}
}
