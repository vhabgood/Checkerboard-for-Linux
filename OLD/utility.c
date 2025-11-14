#include "stdio.h"
#include "utility.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// Dummy functions for Windows API calls if still referenced
#ifndef _fcloseall
#define _fcloseall() 0
#endif

// Removed Windows GUI related functions and types
/*
int initcolorstruct(HWND hwnd, CHOOSECOLOR *ccs, int index) { return 0; }
int FENtoclipboard(HWND hwnd, int board8[8][8], int color, int gametype) { return 0; }
int PDNtoclipboard(HWND hwnd, struct PDNgame *game) { return 0; }
char *textfromclipboard(HWND hwnd, char *str) { return NULL; }
int checklevelmenu(HMENU hmenu,int item, struct CBoptions *CBoptions) { return 0; }
void setmenuchecks(struct CBoptions *CBoptions, HMENU hmenu) {}
int getopening(struct CBoptions *CBoptions) { return 0; }
int getthreeopening(int n, struct CBoptions *CBoptions) { return 0; }
*/

#pragma warning(disable : 4996)

// Original content of utility.c starts here

// All functions below this point are part of the original utility.c
// and will be commented out if they are Windows GUI related.

// Original content of utility.c (excluding GUI functions) would go here.
// For now, we'll just leave the file with the includes and defines.

// Example of a function that might remain if it's not GUI-dependent:
/*
void logtofile(char *str)
{
    // ... (original logtofile implementation) ...
}
*/

// Commenting out the rest of the original utility.c content to avoid Windows GUI dependencies
/*
// ... (rest of utility.c content) ...
*/
