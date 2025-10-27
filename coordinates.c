// coordinates.c
//
// part of checkerboard
//
// this module takes care of all kinds of coordinate transformations:
// from board number to x,y coordinates and vice versa, taking into
// account what kind of game type is being played.

#include "checkers_types.h"

int coorstonumber(int x, int y, int gametype)
{
	// takes coordinates x and y, gametype, and returns the associated board number
	return(y * 4 + (x / 2) + 1);
}

void numbertocoors(int n, int *x, int *y, int gametype) {
    int j = (n - 1) / 4;
    int i = 2 * ((n - 1) % 4) + (j & 1);
    *x = i;
    *y = j;
}

void coorstocoors(int *x, int *y, int invert, int mirror)
{
	// given coordinates x and y on the screen, this function converts them to internal
	// representation of the board based on whether the board is inverted or mirrored
	if (invert) {
		*x = 7 - *x;
		*y = 7 - *y;
	}

	if (mirror)
		*x = 7 - *x;
}

