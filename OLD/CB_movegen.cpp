#include "enginedefs.h"

void domove(int b[46], move2 *move)
{
	int square, after;
	int i;

	for (i = 0; i < move->n; i++) {
		square = (move->m[i] % 256);
		after = ((move->m[i] >> 16) % 256);
		b[square] = after;
	}
}

void undomove(int b[46], move2 *move)
{
	int square, before;
	int i;

	for (i = move->n - 1; i >= 0; --i) {
		square = (move->m[i] % 256);
		before = ((move->m[i] >> 8) % 256);
		b[square] = before;
	}
}