#pragma once

#include "checkers_types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // For FILE, etc.
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


void log_c(int level, const char* message); // Declare log_c here



// From bitboard.h
void boardtocrbitboard(const Board8x8* b, pos *position);
void bitboardtoboard8(pos *p, Board8x8* b);
int count_pieces(const Board8x8* board);
bool is_valid_board8_square(int x, int y);

// From coordinates.h

void coorstocoors(int *x, int *y, bool invert, bool mirror);
void coor_to_notation(int x, int y, char* s, int gametype);
int coorstonumber(int x, int y, int gametype);
void numbertocoors(int n, int *x, int *y, int gametype);

// From crc.h
unsigned int crc_calc(const char *buf, int len);
unsigned int file_crc_calc(FILE *fp);
int fname_crc_calc(const char *name, unsigned int *crc);

// From fen.h
int is_fen(const char *buf);
int FENtoboard8(Board8x8* board, const char *buf, int *poscolor, int gametype);
void board8toFEN(const Board8x8* board, char *fenstr, int color, int gametype);

// From game_logic.h
void newgame(Board8x8* board);




// From dialogs.h
int extract_filename(const char *filespec, char *name);

// From saveashtml.h
int stripquotes(const char *str, char *stripped);
int PDNgametostartposition(PDNgame *game, int b[64]);
int coortohtml(coor c, int gametype);
void move4tonotation(const CBmove *m, char s[80]);

// From utility.h
char *strncpy_terminated(char *dest, const char *src, size_t n);
uint32_t get_sum_squares(CBmove *move);
int get_startcolor(int gametype);
enum PDN_RESULT string_to_pdn_result(const char *resultstr, int gametype);
char* read_text_file_c(const char* filename, enum READ_TEXT_FILE_ERROR_TYPE* etype);

void start3move_c(Board8x8* board, int opening_index);
void domove_c(const CBmove *move, Board8x8* board);
void unmake_move_c(const CBmove *move, Board8x8* board);
void find_captures_recursive(const Board8x8* board, CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n, int color, int is_king, const int* visited_parent);
void makemovelist(const Board8x8* board, int color, CBmove movelist[MAXMOVES], int *isjump, int *n);



#ifdef __cplusplus
}
#endif