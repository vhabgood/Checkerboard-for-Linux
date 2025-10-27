#pragma once

#include "checkers_types.h" // Consolidated common types

#define MAX_MOVES 256

int getmovelist(int color, CBmove movelist[MAXMOVES], Board8x8 board, int *isjump);
