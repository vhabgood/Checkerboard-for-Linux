#pragma once

#include "cb_interface.h" // Includes checkers_types.h and CBstructs.h

#ifdef __cplusplus
extern "C" {
#endif

void boardtobitboard(Board8x8 board, pos *position);
void boardtocrbitboard(Board8x8 board, pos *position);
void bitboardtoboard8(pos *p, Board8x8 board);

#ifdef __cplusplus
}
#endif

