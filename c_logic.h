#pragma once

#include "checkers_types.h"
#include "egdb_driver/egdb/egdb_intl.h" // For egdb_interface types

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // For FILE, etc.
#include <time.h>
#include <string.h>

// Note: Removed extern "C" block for C++ conversion. All functions declared here will have C++ linkage.

// From bitboard.h
void boardtocrbitboard(const Board8x8* b, bitboard_pos *position);
void bitboardtoboard8(bitboard_pos *p, Board8x8* b);
int count_pieces(const bitboard_pos& board);
bool is_valid_board8_square(int x, int y);

// Bitboard manipulation helpers
int get_piece(const bitboard_pos& board, int square_num);
void set_piece(bitboard_pos* board, int square_num, int piece);
void clear_square(bitboard_pos* board, int square_num);


// From coordinates.h

void coorstocoors(int& x, int& y, bool invert, bool mirror);
void coor_to_notation(int x, int y, char* s, int gametype);
int coorstonumber(int x, int y, int gametype);
void numbertocoors(int n, int& x, int& y, int gametype);

// From crc.h
unsigned int crc_calc(const char *buf, int len);
unsigned int file_crc_calc(FILE *fp);
int fname_crc_calc(const char *name, unsigned int *crc);

// From fen.h
int is_fen(const char *buf);
int FENtobitboard_pos(bitboard_pos* position, const char *buf, int *poscolor, int gametype);
void bitboard_postoFEN(const bitboard_pos* position, char *fenstr, int color, int gametype);

// From game_logic.h
void newgame(bitboard_pos* board);
bitboard_pos get_initial_board(int gameType);




// From dialogs.h
int extract_filename(const char *filespec, char *name);

// From saveashtml.h
int stripquotes(const char *str, char *stripped);

int coortohtml(const coor& c, int gametype);
void move4tonotation(const CBmove *m, char s[80]);

// From utility.h
char *strncpy_terminated(char *dest, const char *src, size_t n);
uint32_t get_sum_squares(CBmove *move);
int get_startcolor(int gametype);
enum PDN_RESULT string_to_pdn_result(const char *resultstr, int gametype);
char* read_text_file_c(const char* filename, READ_TEXT_FILE_ERROR_TYPE* etype);

void domove_c(const CBmove *move, bitboard_pos* board);
void dummy_function_test(); // NEW
void unmake_move_c(const CBmove *move, bitboard_pos* board);
void find_captures_recursive(const bitboard_pos& board, CBmove* movelist, CBmove* m, int x, int y, int d, int& n, int color, int is_king, const int* visited_parent);
void makemovelist(const bitboard_pos& board, int color, CBmove* movelist, int& isjump, int& n);
int get_legal_moves_c(const bitboard_pos& board, int color, CBmove* movelist, int& nmoves, int& isjump, const CBmove *last_move, bool *can_continue_multijump);

void start3move_c(bitboard_pos* board, int opening_index);