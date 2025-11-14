#ifndef C_DECLARATIONS_H
#define C_DECLARATIONS_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "checkers_types.h" // Include checkers_types.h for common types

// Macros
#define MAXMOVES 256
#define COMMENTLENGTH 1024
#define MAXNAME 256
#define MAX_PATH_FIXED 260
#define MAXUSERBOOK 1024
#define MAXPIECESET 10
#define ENGINECOMMAND_REPLY_SIZE 2048
#define MAXCOMMANDLENGTH 256
#define MAXMESSAGELENGTH 256

// Typedefs
typedef unsigned int UINT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;

#endif