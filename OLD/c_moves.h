#ifndef C_MOVES_H
#define C_MOVES_H

#include "checkers_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void domove_c(const CBmove *m, Board8x8* board);
int undomove_c(CBmove *m, Board8x8* board);

#ifdef __cplusplus
}
#endif

#endif // C_MOVES_H
