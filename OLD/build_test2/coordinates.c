#include "../checkers_types.h"
#include <stdio.h>
#include <string.h>

/*
 *******************************************************************************
 *
 * coortonumber
 *
 *******************************************************************************
 */
int coortonumber(int x, int y, int gametype)
{
  if (gametype == 21) {
    /* italian rules */
    if (y & 1) { /* y is odd */
      return (4 * (7 - y) + (8 - x) / 2);
    } else { /* y is even */
      return (4 * (7 - y) + (7 - x) / 2);
    }
  } else {
    /* english/american rules */
    if (y & 1) { /* y is odd */
      return (4 * y + x / 2 + 1);
    } else { /* y is even */
      return (4 * y + (x + 1) / 2);
    }
  }
}

/*
 *******************************************************************************
 *
 * coorstocoors
 *
 *******************************************************************************
 */
void coorstocoors(int *x, int *y, int invert, int mirror)
{
  if (mirror)
    *x = 7 - *x;
  if (invert) {
    *x = 7 - *x;
    *y = 7 - *y;
  }
}

/*
 *******************************************************************************
 *
 * is_valid_board8_square
 *
 *******************************************************************************
 */
int is_valid_board8_square(int x, int y)
{
  if (x < 0 || x > 7 || y < 0 || y > 7)
    return (0);

  if ((y & 1) == (x & 1))
    return (0);

  return (1);
}