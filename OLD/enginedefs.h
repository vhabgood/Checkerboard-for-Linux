#pragma once

#include "checkers_types.h"



int generatemovelist(int b[46], move2 movelist[MAXMOVES], int color);
int generatecapturelist(int b[46], move2 movelist[MAXMOVES], int color);
void domove(int b[46], move2 *move);
void undomove(int b[46], move2 *move);
