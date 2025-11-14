#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "checkers_types.h" // Include all common types and structs



// Function pointer types for engine interaction
typedef int (*CB_GETMOVE)(Board8x8 board, int color, double maxtime, char statusBuffer[1024], QAtomicInt *playnow, int info, int moreinfo, CBmove *bestMove);
typedef int (*CB_ISLEGAL)(Board8x8 board8, int color, int from, int to, int gametype, CBmove *move);
typedef int (*CB_ENGINECOMMAND)(const char *command, char reply[ENGINECOMMAND_REPLY_SIZE]);
typedef int (*CB_GETSTRING)(char Lstr[MAXNAME]);
typedef int (*CB_GETGAMETYPE)(void);
