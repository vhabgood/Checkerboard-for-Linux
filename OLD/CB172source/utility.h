// utility.h

// Function prototypes for non-GUI related utility functions

// Example: log to file function (if not GUI dependent)
void logtofile(char *str);

// FEN related functions (if not GUI dependent)
int FENtoboard8(char *FENstring, int board8[8][8], int *color);
void board8toFEN(int board8[8][8], int color, char *FENstring);
int is_fen(char *str);

// Path extraction (if not GUI dependent)
void extract_path(char *name, char *path);

// Other utility functions that are not GUI dependent
// ...