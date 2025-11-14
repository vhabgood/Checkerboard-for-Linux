#include "c_logic.h"
#include "dblookup.h"
#include "checkers_c_types.h" // Direct include


#include <string.h> // For strcpy, strcat, strlen, strchr
#include <stdlib.h> // For atoi
#include <ctype.h> // For isspace, toupper, isdigit
#include <stdarg.h> // For va_list, va_start, vfprintf, va_end
#include <QString> // Added for QString
// Forward declaration for logging function from GameManager
void log_c(int level, const char* message);
#include <stdio.h> // For FILE, etc.

int bitcount(unsigned int n) {
    int c = 0;
    while (n > 0) { n &= (n - 1); c++; }
    return c;
}

// From bitboard.h
void boardtobitboard(const Board8x8* b, pos *position)
{
    int i, j, bit_pos;
    position->bm = 0; position->bk = 0; position->wm = 0; position->wk = 0;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if ((i + j) % 2 == 1) { // Only dark squares
                bit_pos = i * 4 + j / 2;
                switch (b->board[i][j]) {
                    case (CB_BLACK | CB_MAN):
                        position->bm |= (1 << bit_pos);
                        break;
                    case (CB_BLACK | CB_KING):
                        position->bk |= (1 << bit_pos);
                        break;
                    case (CB_WHITE | CB_MAN):
                        position->wm |= (1 << bit_pos);
                        break;
                    case (CB_WHITE | CB_KING):
                        position->wk |= (1 << bit_pos);
                        break;
                }
            }
        }
    }
}

void boardtocrbitboard(const Board8x8* b, pos *position)
{
    int i, board[32];
    board[0] = b->board[0][0]; board[1] = b->board[2][0]; board[2] = b->board[4][0]; board[3] = b->board[6][0];
    board[4] = b->board[1][1]; board[5] = b->board[3][1]; board[6] = b->board[5][1]; board[7] = b->board[7][1];
    board[8] = b->board[0][2]; board[9] = b->board[2][2]; board[10] = b->board[4][2]; board[11] = b->board[6][2];
    board[12] = b->board[1][3]; board[13] = b->board[3][3]; board[14] = b->board[5][3]; board[15] = b->board[7][3];
    board[16] = b->board[0][4]; board[17] = b->board[2][4]; board[18] = b->board[4][4]; board[19] = b->board[6][4];
    board[20] = b->board[1][5]; board[21] = b->board[3][5]; board[22] = b->board[5][5]; board[23] = b->board[7][5];
    board[24] = b->board[0][6]; board[25] = b->board[2][6]; board[26] = b->board[4][6]; board[27] = b->board[6][6];
    board[28] = b->board[1][7]; board[29] = b->board[3][7]; board[30] = b->board[5][7]; board[31] = b->board[7][7];

    position->bm = 0; position->bk = 0; position->wm = 0; position->wk = 0;
    for (i = 0; i < 32; i++) {
        switch (board[i]) {
        case CB_BLACK | CB_MAN: position->wm |= (1 << (31 - i)); break;
        case CB_BLACK | CB_KING: position->wk |= (1 << (31 - i)); break;
        case CB_WHITE | CB_MAN: position->bm |= (1 << (31 - i)); break;
        case CB_WHITE | CB_KING: position->wk = position->wk | (1 << (31 - i)); break;
        }
    }
}

void bitboardtoboard8(pos *p, Board8x8* b)
{
    int i, r, c, bit_pos;

    // Initialize board to empty
    for (r = 0; r < 8; r++) {
        for (c = 0; c < 8; c++) {
            b->board[r][c] = CB_EMPTY;
        }
    }

    for (r = 0; r < 8; r++) {
        for (c = 0; c < 8; c++) {
            if ((r + c) % 2 == 1) { // Only dark squares
                bit_pos = r * 4 + c / 2;
                if (p->bm & (1 << bit_pos))
                    b->board[r][c] = CB_BLACK | CB_MAN;
                else if (p->bk & (1 << bit_pos))
                    b->board[r][c] = CB_BLACK | CB_KING;
                else if (p->wm & (1 << bit_pos))
                    b->board[r][c] = CB_WHITE | CB_MAN;
                else if (p->wk & (1 << bit_pos))
                    b->board[r][c] = CB_WHITE | CB_KING;
            }
        }
    }
}

int count_pieces(const Board8x8* board)
{
    pos currentPos;
    boardtobitboard(board, &currentPos);
    return bitcount(currentPos.bm) + bitcount(currentPos.bk) + bitcount(currentPos.wm) + bitcount(currentPos.wk);
}

// From coordinates.h
int coorstonumber(int x, int y, int gametype) {
    // Checkers numbering: 1-32, starting from bottom-right (y=7, x=6) and increasing to the left, then up row by row.
    // Only dark squares (x+y is odd) have numbers.

    // Adjust y to be 0-indexed from the bottom (0-7)
    int adjusted_y = 7 - y;

    if ((x + y) % 2 == 0) { // Light square, no number
        return 0; // Or some other indicator for no number
    }

    int number = 0;
    if (adjusted_y % 2 == 0) { // Even adjusted_y (original y=7,5,3,1) -> dark squares x=0,2,4,6
        // For right-to-left numbering, map x=6 to 0, x=4 to 1, x=2 to 2, x=0 to 3
        number = (adjusted_y * 4) + ((6 - x) / 2) + 1;
    } else { // Odd adjusted_y (original y=6,4,2,0) -> dark squares x=1,3,5,7
        // For right-to-left numbering, map x=7 to 0, x=5 to 1, x=3 to 2, x=1 to 3
        number = (adjusted_y * 4) + ((7 - x) / 2) + 1;
    }
    return number;
}

void numbertocoors(int n, int *x, int *y, int gametype) {
    // Convert 1-32 board number back to 0-7 (x,y) coordinates
    if (n < 1 || n > 32) {
        *x = -1; *y = -1; // Invalid number
        return;
    }

    int adjusted_y = (n - 1) / 4;
    int offset_in_row = (n - 1) % 4;

    if (adjusted_y % 2 == 0) { // Original y=7,5,3,1
        *x = 6 - (offset_in_row * 2);
    } else { // Original y=6,4,2,0
        *x = 7 - (offset_in_row * 2 + 1);
    }
    *y = 7 - adjusted_y;
}
void coorstocoors(int *x, int *y, bool invert, bool mirror) { if (invert) { *x = 7 - *x; *y = 7 - *y; } if (mirror) *x = 7 - *x; }
bool is_valid_board8_square(int x, int y) { return((x + y) % 2 != 0); }

void coor_to_notation(int x, int y, char* s, int gametype) {
    int square_num = coorstonumber(x, y, gametype);
    sprintf(s, "%d", square_num);
}

// From crc.h
#define CRCSEED 0xffffffff
#define XOROT    0xffffffff
unsigned int CRC32tab[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
unsigned int crc_calc(const char *buf, int len) { int i; unsigned int crc = CRCSEED; for (i = 0; i < len; ++i) crc = CRC32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8); crc ^= XOROT; return(crc); }
unsigned int file_crc_calc(FILE *fp) { int i; unsigned int crc = CRCSEED; int bytes_read; char buf[16384]; do { bytes_read = (int)fread(buf, 1, sizeof(buf), fp); if (bytes_read <= 0) break; for (i = 0; i < bytes_read; ++i) crc = CRC32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8); } while (bytes_read > 0); crc ^= XOROT; return(crc); }
int fname_crc_calc(const char *name, unsigned int *crc) { FILE *fp = fopen(name, "rb"); if (!fp) return(1); *crc = file_crc_calc(fp); fclose(fp); return(0); }

// From fen.h
int is_fen(const char *buf) { while (*buf) { if (isspace(*buf)) ++buf; else if (*buf == '"') ++buf; else break; } if ((toupper(*buf) == 'B' || toupper(*buf) == 'W') && buf[1] == ':') return(1); else return(0); }
int FENtoboard8(Board8x8* board, const char *buf, int *poscolor, int gametype) { int square, square2, s; int color, piece_type; int i, j; const char *lastp; lastp = strchr(buf, ':'); if (!lastp || lastp == buf) return(0); buf = lastp - 1; if (toupper(*buf) == 'B') *poscolor = CB_BLACK; else if (toupper(*buf) == 'W') *poscolor = CB_WHITE; else return(0); for (i = 0; i < 8; ++i) for (j = 0; j < 8; ++j) board->board[i][j] = 0; ++buf; if (*buf != ':') return(0); ++buf; lastp = buf; while (*buf) { piece_type = CB_MAN; if (*buf == '"') { ++buf; continue; } if (toupper(*buf) == 'W') { color = CB_WHITE; ++buf; continue; } if (toupper(*buf) == 'B') { color = CB_BLACK; ++buf; continue; } if (toupper(*buf) == 'K') { piece_type = CB_KING; ++buf; } for (square = 0; isdigit(*buf); ++buf) square = 10 * square + (*buf - '0'); square2 = square; if (*buf == ',' || *buf == ':') ++buf; else if (*buf == '-' && isdigit(buf[1])) { ++buf; for (square2 = 0; isdigit(*buf); ++buf) square2 = 10 * square2 + (*buf - '0'); if (*buf == ',' || *buf == ':') ++buf; } if (square && square <= square2) for (s = square; s <= square2; ++s) { numbertocoors(s, &i, &j, gametype); board->board[j][i] = piece_type | color; } if (*buf == ',') ++buf; if (lastp == buf) break; lastp = buf; } return(1); } 
void board8toFEN(const Board8x8* board, char *fenstr, int color, int gametype) {
    char w_pieces[128] = "";
    char b_pieces[128] = "";
    char temp[8];

    for (int s = 1; s <= 32; ++s) {
        int r, c;
        numbertocoors(s, &c, &r, gametype);
        int piece = board->board[r][c];

        if (piece == (CB_WHITE | CB_MAN)) {
            sprintf(temp, "%d,", s);
            strcat(w_pieces, temp);
        } else if (piece == (CB_WHITE | CB_KING)) {
            sprintf(temp, "K%d,", s);
            strcat(w_pieces, temp);
        } else if (piece == (CB_BLACK | CB_MAN)) {
            sprintf(temp, "%d,", s);
            strcat(b_pieces, temp);
        } else if (piece == (CB_BLACK | CB_KING)) {
            sprintf(temp, "K%d,", s);
            strcat(b_pieces, temp);
        }
    }

    // Remove trailing commas
    if (strlen(w_pieces) > 0) w_pieces[strlen(w_pieces) - 1] = '\0';
    if (strlen(b_pieces) > 0) b_pieces[strlen(b_pieces) - 1] = '\0';

    sprintf(fenstr, "%c:W%s:B%s", (color == CB_BLACK ? 'B' : 'W'), w_pieces, b_pieces);
}

// From game_logic.h
void newgame(Board8x8* board) {
    char initialBoard[8][8] = { {CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN}, {CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY}, {CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN, CB_EMPTY, CB_WHITE | CB_MAN}, {CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY}, {CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY, CB_EMPTY}, {CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY}, {CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN}, {CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY, CB_BLACK | CB_MAN, CB_EMPTY} };
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            board->board[r][c] = initialBoard[r][c];
        }
    }
}

// From dialogs.h
int extract_filename(const char *filespec, char *name)
{
    int i, len;

    len = (int)strlen(filespec);
    for (i = len - 1; i >= 0; --i)
        if (filespec[i] == '\'' || filespec[i] == '/' || filespec[i] == ':')
            break;

    strcpy(name, filespec + i + 1);
    return(0);
}

// From saveashtml.h

int stripquotes(const char *str, char *stripped)
{
    int i = 0;

    stripped[0] = '\0';
    while (str[i] != 0 && i < 1024) {
        if (str[i] != '"')
            stripped[i] = str[i];
        else
            stripped[i] = ' ';
        i++;
    }

    stripped[i] = 0;
    return 1;
}

int PDNgametostartposition(PDNgame *game, int b[64])
{
    // fills the array b with the pieces in the starting position of a PDN game
    int i, j;
    int returncolor;
    Board8x8 b8;

    for (i = 0; i < 64; i++)
        b[i] = 0;

    if (strcmp(game->FEN, "") == 0) {

    // no setup
        if (game->gametype == 21) {
            for (i = 0; i < 12; i++) {
                j = 2 * i;
                if (i >= 4 && i < 8)
                    j++;
                b[j] = CB_BLACK | CB_MAN;}

            for (i = 20; i < 32; i++) {
                j = 2 * i + 1;
                if (i >= 24 && i < 28)
                    j--;
                b[j] = CB_WHITE | CB_MAN;}
        }
        else {
            for (i = 0; i < 12; i++) {
                j = 2 * i + 1;
                if (i >= 4 && i < 8)
                    j--;
                b[j] = CB_WHITE | CB_MAN;
            }

            for (i = 20; i < 32; i++) {
                j = 2 * i;
                if (i >= 24 && i < 28)
                    j++;
                b[j] = CB_BLACK | CB_MAN;
            }
        }
    }
    else {

        // setup
        FENtoboard8(&b8, game->FEN, &returncolor, game->gametype);
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                b[8 * (7 - j) + i] = b8.board[i][j];
                if (game->gametype == 21)
                    b[8 * (7 - j) + i] = b8.board[i][7 - j];
            }
        }
    }

    return 1;
}

int coortohtml(coor c, int gametype)
{
    // return html number for a coordinate
    // html:
    // 0  1  2  3  4  5  6  7
    // 8  9 10 11 12 13 14 15
    // 16 etc.
    // coors
    //
    //
    switch (gametype) {
    case 21:
        return c.x + 8 * (7 - c.y);
        break;

    case 22:
        return c.x + 8 * c.y;
        break;

    default:
        return c.x + 8 * (7 - c.y);
        break;
    }
}



char *strncpy_terminated(char *dest, const char *src, size_t n) {
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0'; // Ensure null termination
    return dest;
}

uint32_t get_sum_squares(CBmove *move) {
    uint32_t sum = 0;
    if (move) {
        sum += move->from.x + move->from.y;
        sum += move->to.x + move->to.y;
    }
    return sum;
}

int get_startcolor(int gametype)
{
    if (gametype == GT_ITALIAN || gametype == GT_SPANISH || gametype == GT_RUSSIAN || gametype == GT_CZECH)
        return CB_WHITE;
    return CB_BLACK;
}

enum PDN_RESULT string_to_pdn_result(const char *resultstr, int gametype)
{
    if (strcmp(resultstr, "1-0") == 0)
        return PDN_RESULT_WIN;
    if (strcmp(resultstr, "0-1") == 0)
        return PDN_RESULT_LOSS;
    if (strcmp(resultstr, "1/2-1/2") == 0)
        return PDN_RESULT_DRAW;
    return PDN_RESULT_UNKNOWN;
}

// --- Move Generation (adapted from CB_movegen.c) ---

// Internal representation for move generation (12x12 board)
#define MG_EMPTY 0
#define MG_WHITE_MAN 1
#define MG_WHITE_KING 2
#define MG_BLACK_MAN -1
#define MG_BLACK_KING -2
#define MG_INVALID 10 // Represents off-board squares

// Forward declarations for internal move generation functions
// These are now declared in c_logic.h and implemented below.

int get_legal_moves_c(pos *p, int color, CBmove movelist[MAXMOVES], int *nmoves, int *isjump, const CBmove *last_move, bool *can_continue_multijump)
{
    int i, j;
    int board12[12][12];
    Board8x8 b8;
    CBmove all_moves[MAXMOVES];
    int all_nmoves = 0;
    int all_isjump = 0;

    *can_continue_multijump = false;

    bitboardtoboard8(p, &b8); // Convert pos to Board8x8
    board8toboard12(&b8, board12);

    // Initialize nmoves to 0 before calling makemovelist
    all_nmoves = 0;

    // Replace cbcolor_to_getmovelistcolor directly
    int color_to_use = (color == CB_BLACK) ? -1 : 1;
    makemovelist(color_to_use, all_moves, board12, &all_isjump, &all_nmoves);

    // Filter moves if a multi-jump is in progress
    if (last_move != NULL && last_move->jumps > 0) {
        int filtered_count = 0;
        for (i = 0; i < all_nmoves; ++i) {
            // Check if the move starts from the 'to' square of the last move
            // and if it's a jump (jumps > 0)
            if (all_moves[i].from.x == last_move->to.x + 2 && // +2 because makemovelist operates on 12x12 board
                all_moves[i].from.y == last_move->to.y + 2 && // +2 because makemovelist operates on 12x12 board
                all_moves[i].jumps > 0) {
                movelist[filtered_count++] = all_moves[i];
                *can_continue_multijump = true;
            }
        }
        *nmoves = filtered_count;
        *isjump = *can_continue_multijump; // If we can continue a multijump, it's a jump
    } else {
        // No multi-jump in progress, copy all moves
        for (i = 0; i < all_nmoves; ++i) {
            movelist[i] = all_moves[i];
        }
        *nmoves = all_nmoves;
        *isjump = all_isjump;
    }

    // now: do something to the coordinates, so that the moves are in a 8x8-format
    for (i = 0; i < *nmoves; i++) { // Use *nmoves for the loop limit
        movelist[i].from.x -= 2;
        movelist[i].to.x -= 2;
        movelist[i].from.y -= 2;
        movelist[i].to.y -= 2;
        for (j = 0; j < 11; j++) {
            movelist[i].path[j].x -= 2;
            movelist[i].path[j].y -= 2;
            movelist[i].del[j].x -= 2;
            movelist[i].del[j].y -= 2;
        }
    }

    // and set the pieces to CB-format
    for (i = 0; i < *nmoves; i++) { // Use *nmoves for the loop limit
        switch (movelist[i].oldpiece) {
        case MG_WHITE_KING:
            movelist[i].oldpiece = (CB_WHITE | CB_KING);
            break;
        case MG_WHITE_MAN:
            movelist[i].oldpiece = (CB_WHITE | CB_MAN);
            break;
        case MG_BLACK_MAN:
            movelist[i].oldpiece = (CB_BLACK | CB_MAN);
            break;
        case MG_BLACK_KING:
            movelist[i].oldpiece = (CB_BLACK | CB_KING);
            break;
        }

        switch (movelist[i].newpiece) {
        case MG_WHITE_KING:
            movelist[i].newpiece = (CB_WHITE | CB_KING);
            break;
        case MG_WHITE_MAN:
            movelist[i].newpiece = (CB_WHITE | CB_MAN);
            break;
        case MG_BLACK_MAN:
            movelist[i].newpiece = (CB_BLACK | CB_MAN);
            break;
        case MG_BLACK_KING:
            movelist[i].newpiece = (CB_BLACK | CB_KING);
            break;
        }

        for (j = 0; j < movelist[i].jumps; j++) {
            switch (movelist[i].delpiece[j]) {
            case MG_WHITE_KING:
                movelist[i].delpiece[j] = (CB_WHITE | CB_KING);
                break;
            case MG_WHITE_MAN:
                movelist[i].delpiece[j] = (CB_WHITE | CB_MAN);
                break;
            case MG_BLACK_MAN:
                movelist[i].delpiece[j] = (CB_BLACK | CB_MAN);
                break;
            case MG_BLACK_KING:
                movelist[i].delpiece[j] = (CB_BLACK | CB_KING);
                break;
            }
        }
    }
    return *nmoves;
}

void board8toboard12(const Board8x8* b, int board12[12][12])
{
    // checkerboard uses a 8x8 board representation, the move generator a 12x12
    // this routine converts a 8x8 to a 12x12 board
    int i, j;
    for (i = 0; i <= 11; i++) {
        for (j = 0; j <= 11; j++)
            board12[i][j] = MG_INVALID;
    }

    for (i = 2; i <= 9; i++) {
        for (j = 2; j <= 9; j++)
            board12[i][j] = MG_EMPTY;
    }

    for (i = 0; i <= 7; i++) { // i is row in Board8x8
        for (j = 0; j <= 7; j++) { // j is col in Board8x8
            if (b->board[i][j] == (CB_BLACK | CB_MAN))
                board12[j + 2][i + 2] = MG_BLACK_MAN;
            if (b->board[i][j] == (CB_BLACK | CB_KING))
                board12[j + 2][i + 2] = MG_BLACK_KING;
            if (b->board[i][j] == (CB_WHITE | CB_MAN))
                board12[j + 2][i + 2] = MG_WHITE_MAN;
            if (b->board[i][j] == (CB_WHITE | CB_KING))
                board12[j + 2][i + 2] = MG_WHITE_KING;
        }
    }

    // Log the 12x12 board for debugging
    log_c(LOG_LEVEL_DEBUG, "board8toboard12: Populated 12x12 board:");
    for (int row = 0; row < 12; ++row) {
        QString row_str;
        for (int col = 0; col < 12; ++col) {
            row_str += QString("%1 ").arg(board12[col][row], 3); // Print with padding
        }
        log_c(LOG_LEVEL_DEBUG, row_str.toUtf8().constData());
    }
} // This is the correct closing brace for the function.

void makemovelist(int color, CBmove movelist[MAXMOVES], int board[12][12], int *isjump, int *n)
{
    log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Called for color: %1").arg(color)).toUtf8().constData());
    // produces a movelist for color to move on board
    coor wk[12], bk[12], ws[12], bs[12];
    int nwk = 0, nbk = 0, nws = 0, nbs = 0;
    int i, j;
    int x, y;
    CBmove m;
    *isjump = 0;

    for (i = 0; i < MAXMOVES; i++) {
        movelist[i].jumps = 0;
        movelist[i].is_capture = false; // Initialize is_capture to false
    }
    *n = 0;


    // initialize list of stones
    for (j = 2; j <= 9; j++) { // Iterate through all rows (y-coordinates)
        for (i = 2; i <= 9; i++) { // Iterate through all columns (x-coordinates)
            if ((i + j) % 2 == 0) { // Only consider dark squares (i+j is odd for dark squares in 0-7, but 12x12 board is shifted by 2, so i+j is even for dark squares)
                continue;
            }
            if (board[i][j] == MG_EMPTY)
                continue;
            if (color == 1 && board[i][j] == MG_WHITE_MAN) {
                ws[nws].x = i;
                ws[nws].y = j;
                nws++;
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Found WHITE_MAN at 12x12 (%1,%2)").arg(i).arg(j)).toUtf8().constData());
                continue;
            }

            if (color == 1 && board[i][j] == MG_WHITE_KING) {
                wk[nwk].x = i;
                wk[nwk].y = j;
                nwk++;
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Found WHITE_KING at 12x12 (%1,%2)").arg(i).arg(j)).toUtf8().constData());
                continue;
            }

            if (color == -1 && board[i][j] == MG_BLACK_MAN) {
                bs[nbs].x = i;
                bs[nbs].y = j;
                nbs++;
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Found BLACK_MAN at 12x12 (%1,%2)").arg(i).arg(j)).toUtf8().constData());
                continue;
            }

            if (color == -1 && board[i][j] == MG_BLACK_KING) {
                bk[nbk].x = i;
                bk[nbk].y = j;
                nbk++;
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Found BLACK_KING at 12x12 (%1,%2)").arg(i).arg(j)).toUtf8().constData());
                continue;
            }
        }
    }

    log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Piece counts - White Men: %1, White Kings: %2, Black Men: %3, Black Kings: %4").arg(nws).arg(nwk).arg(nbs).arg(nbk)).toUtf8().constData());

    if (color == 1) { // White
        /* search for captures with white kings*/
        if (nwk > 0) {
            for (i = 0; i < nwk; i++) {
                x = wk[i].x;
                y = wk[i].y;
                if
                (
                    (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == MG_EMPTY) ||
                    (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == MG_EMPTY) ||
                    (board[x + 1][y - 1] < 0 && board[x + 2][y - 2] == MG_EMPTY) ||
                    (board[x - 1][y - 1] < 0 && board[x - 2][y - 2] == MG_EMPTY)
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    whitekingcapture(board, movelist, m, x, y, 0, n);
                }
            }
        }

        /* search for captures with white stones */
        if (nws > 0) {
            for (i = 0; i < nws; i++) {
                x = ws[i].x;
                y = ws[i].y;
                if
                (
                    (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == MG_EMPTY) ||
                    (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == MG_EMPTY)
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    whitecapture(board, movelist, m, x, y, 0, n);
                }
            }
        }

        /* if there are capture moves return. */
        if (*n > 0) {
            *isjump = 1;
            return;
        }

        /* search for moves with white kings */
        if (nwk > 0) {
            for (i = 0; i < nwk; i++) {
                x = wk[i].x;
                y = wk[i].y;
                if (board[x + 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_WHITE_KING;
                    movelist[*n].oldpiece = MG_WHITE_KING;
                    (*n)++;
                }

                if (board[x + 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_WHITE_KING;
                    movelist[*n].oldpiece = MG_WHITE_KING;
                    (*n)++;
                }

                if (board[x - 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_WHITE_KING;
                    movelist[*n].oldpiece = MG_WHITE_KING;
                    (*n)++;
                }

                if (board[x - 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_WHITE_KING;
                    movelist[*n].oldpiece = MG_WHITE_KING;
                    (*n)++;
                }
            }
        }

        /* search for moves with white stones */
        if (nws > 0) {
            for (i = 0; i < nws; i++) {
                x = ws[i].x;
                y = ws[i].y;
                if (board[x + 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    if (y == 8) {
                        movelist[*n].newpiece = MG_WHITE_KING;
                    }
                    else
                        movelist[*n].newpiece = MG_WHITE_MAN;
                    movelist[*n].oldpiece = MG_WHITE_MAN;
                    (*n)++;
                }

                if (board[x - 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    if (y == 8) {
                        movelist[*n].newpiece = MG_WHITE_KING;
                    }
                    else
                        movelist[*n].newpiece = MG_WHITE_MAN;
                    movelist[*n].oldpiece = MG_WHITE_MAN;
                    (*n)++;
                }
            }
        }
    }
    else { // Black
        /* search for captures with black kings*/
        if (nbk > 0) {
            for (i = 0; i < nbk; i++) {
                x = bk[i].x;
                y = bk[i].y;
                if
                (
                    ((board[x + 1][y + 1] > 0) && (board[x + 1][y + 1] < MG_INVALID) && (board[x + 2][y + 2] == MG_EMPTY)) ||
                    ((board[x - 1][y + 1] > 0) && (board[x - 1][y + 1] < MG_INVALID) && (board[x - 2][y + 2] == MG_EMPTY)) ||
                    ((board[x + 1][y - 1] > 0) && (board[x + 1][y - 1] < MG_INVALID) && (board[x + 2][y - 2] == MG_EMPTY)) ||
                    ((board[x - 1][y - 1] > 0) && (board[x - 1][y - 1] < MG_INVALID) && (board[x - 2][y - 2] == MG_EMPTY))
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    blackkingcapture(board, movelist, m, x, y, 0, n);
                }
            }
        }

        /* search for captures with black stones */
        if (nbs > 0) {
            for (i = nbs - 1; i >= 0; i--) {
                x = bs[i].x;
                y = bs[i].y;
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Checking black stone at 12x12 (%1,%2)").arg(x).arg(y)).toUtf8().constData());
                bool cond1 = (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == MG_EMPTY);
                bool cond2 = (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == MG_EMPTY);
                log_c(LOG_LEVEL_DEBUG, (QString("makemovelist: Cond1: %1, Cond2: %2").arg(cond1).arg(cond2)).toUtf8().constData());
                if
                (
                    cond1 || cond2
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    blackcapture(board, movelist, m, x, y, 0, n);
                }
            }
        }

        /* if there are capture moves return. */
        if (*n > 0) {
            *isjump = 1;
            return;
        }

        /* search for moves with black kings */
        if (nbk > 0) {
            for (i = 0; i < nbk; i++) {
                x = bk[i].x;
                y = bk[i].y;
                if (board[x + 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_BLACK_KING;
                    movelist[*n].oldpiece = MG_BLACK_KING;
                    (*n)++;
                }

                if (board[x + 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_BLACK_KING;
                    movelist[*n].oldpiece = MG_BLACK_KING;
                    (*n)++;
                }

                if (board[x - 1][y + 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y + 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y + 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_BLACK_KING;
                    movelist[*n].oldpiece = MG_BLACK_KING;
                    (*n)++;
                }

                if (board[x - 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    movelist[*n].newpiece = MG_BLACK_KING;
                    movelist[*n].oldpiece = MG_BLACK_KING;
                    (*n)++;
                }
            }
        }

        /* search for moves with black stones */
        if (nbs > 0) {
            for (i = nbs - 1; i >= 0; i--) {
                x = bs[i].x;
                y = bs[i].y;
                if (board[x + 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x + 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x + 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    if (y == 3) {
                        movelist[*n].newpiece = MG_BLACK_KING;
                    }
                    else
                        movelist[*n].newpiece = MG_BLACK_MAN;
                    movelist[*n].oldpiece = MG_BLACK_MAN;
                    (*n)++;
                }

                if (board[x - 1][y - 1] == MG_EMPTY) {
                    movelist[*n].jumps = 0;
                    movelist[*n].from.x = x;
                    movelist[*n].from.y = y;
                    movelist[*n].to.x = x - 1;
                    movelist[*n].to.y = y - 1;
                    movelist[*n].path[0].x = x;
                    movelist[*n].path[0].y = y;
                    movelist[*n].path[1].x = x - 1;
                    movelist[*n].path[1].y = y - 1;
                    movelist[*n].del[0].x = -1;
                    if (y == 3) {
                        movelist[*n].newpiece = MG_BLACK_KING;
                    }
                    else
                        movelist[*n].newpiece = MG_BLACK_MAN;
                    movelist[*n].oldpiece = MG_BLACK_MAN;
                    (*n)++;
                }
            }
        }
    }
}

void whitecapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    CBmove mm;
    int end = 1;

    if (d >= 11) { // Check to prevent stack overflow
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_WHITE_MAN;
        (*n)++;
        return;
    }

    // Try capturing diagonally up-right
    if (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x + 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_MAN;
        mm.newpiece = (y + 2 == 9) ? MG_WHITE_KING : MG_WHITE_MAN; // Handle kinging
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y + 1] = MG_EMPTY;
        board[x + 2][y + 2] = mm.newpiece;

        whitecapture(board, movelist, mm, x + 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y + 1] = original_captured_piece;
        board[x + 2][y + 2] = MG_EMPTY;
    }

    // Try capturing diagonally up-left
    if (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x - 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_MAN;
        mm.newpiece = (y + 2 == 9) ? MG_WHITE_KING : MG_WHITE_MAN; // Handle kinging
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y + 1] = MG_EMPTY;
        board[x - 2][y + 2] = mm.newpiece;

        whitecapture(board, movelist, mm, x - 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y + 1] = original_captured_piece;
        board[x - 2][y + 2] = MG_EMPTY;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_WHITE_MAN;
        movelist[*n].newpiece = (m.to.y == 9) ? MG_WHITE_KING : MG_WHITE_MAN; // Final kinging check
        movelist[*n].is_capture = true; // Set is_capture to true
        (*n)++;
    }
}

void whitekingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    CBmove mm;
    int end = 1;

    if (d >= 11) { // Check to prevent stack overflow
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_WHITE_KING;
        (*n)++;
        return;
    }

    // Try capturing diagonally up-right
    if (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x + 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_KING;
        mm.newpiece = MG_WHITE_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y + 1] = MG_EMPTY;
        board[x + 2][y + 2] = mm.newpiece;

        whitekingcapture(board, movelist, mm, x + 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y + 1] = original_captured_piece;
        board[x + 2][y + 2] = MG_EMPTY;
    }

    // Try capturing diagonally up-left
    if (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x - 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_KING;
        mm.newpiece = MG_WHITE_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y + 1] = MG_EMPTY;
        board[x - 2][y + 2] = mm.newpiece;

        whitekingcapture(board, movelist, mm, x - 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y + 1] = original_captured_piece;
        board[x - 2][y + 2] = MG_EMPTY;
    }

    // Try capturing diagonally down-right
    if (board[x + 1][y - 1] < 0 && board[x + 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x + 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_KING;
        mm.newpiece = MG_WHITE_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y - 1] = MG_EMPTY;
        board[x + 2][y - 2] = mm.newpiece;

        whitekingcapture(board, movelist, mm, x + 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y - 1] = original_captured_piece;
        board[x + 2][y - 2] = MG_EMPTY;
    }

    // Try capturing diagonally down-left
    if (board[x - 1][y - 1] < 0 && board[x - 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x - 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_WHITE_KING;
        mm.newpiece = MG_WHITE_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y - 1] = MG_EMPTY;
        board[x - 2][y - 2] = mm.newpiece;

        whitekingcapture(board, movelist, mm, x - 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y - 1] = original_captured_piece;
        board[x - 2][y - 2] = MG_EMPTY;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_WHITE_KING;
        movelist[*n].newpiece = MG_WHITE_KING; // Kings remain kings
        movelist[*n].is_capture = true; // Set is_capture to true
        (*n)++;
    }
}

void blackcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    CBmove mm;
    int end = 1;

    if (d >= 11) { // Check to prevent stack overflow
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_BLACK_MAN;
        (*n)++;
        return;
    }

    // Try capturing diagonally down-right
    if (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x + 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_MAN;
        mm.newpiece = (y - 2 == 2) ? MG_BLACK_KING : MG_BLACK_MAN; // Handle kinging
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y - 1] = MG_EMPTY;
        board[x + 2][y - 2] = mm.newpiece;

        blackcapture(board, movelist, mm, x + 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y - 1] = original_captured_piece;
        board[x + 2][y - 2] = MG_EMPTY;
    }

    // Try capturing diagonally down-left
    if (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x - 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_MAN;
        mm.newpiece = (y - 2 == 2) ? MG_BLACK_KING : MG_BLACK_MAN; // Handle kinging
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y - 1] = MG_EMPTY;
        board[x - 2][y - 2] = mm.newpiece;

        blackcapture(board, movelist, mm, x - 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y - 1] = original_captured_piece;
        board[x - 2][y - 2] = MG_EMPTY;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_BLACK_MAN;
        movelist[*n].newpiece = (m.to.y == 2) ? MG_BLACK_KING : MG_BLACK_MAN; // Final kinging check
        movelist[*n].is_capture = true; // Set is_capture to true
        (*n)++;
    }
}

void blackkingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    CBmove mm;
    int end = 1;

    if (d >= 11) { // Check to prevent stack overflow
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_BLACK_KING;
        (*n)++;
        return;
    }

    // Try capturing diagonally down-right
    if (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x + 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_KING;
        mm.newpiece = MG_BLACK_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y - 1] = MG_EMPTY;
        board[x + 2][y - 2] = mm.newpiece;

        blackkingcapture(board, movelist, mm, x + 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y - 1] = original_captured_piece;
        board[x + 2][y - 2] = MG_EMPTY;
    }

    // Try capturing diagonally down-left
    if (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x - 1][y - 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_KING;
        mm.newpiece = MG_BLACK_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y - 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y - 1] = MG_EMPTY;
        board[x - 2][y - 2] = mm.newpiece;

        blackkingcapture(board, movelist, mm, x - 2, y - 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y - 1] = original_captured_piece;
        board[x - 2][y - 2] = MG_EMPTY;
    }

    // Try capturing diagonally up-right
    if (board[x + 1][y + 1] > 0 && board[x + 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x + 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x + 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_KING;
        mm.newpiece = MG_BLACK_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x + 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x + 1][y + 1] = MG_EMPTY;
        board[x + 2][y + 2] = mm.newpiece;

        blackkingcapture(board, movelist, mm, x + 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x + 1][y + 1] = original_captured_piece;
        board[x + 2][y + 2] = MG_EMPTY;
    }

    // Try capturing diagonally up-left
    if (board[x - 1][y + 1] > 0 && board[x - 2][y + 2] == MG_EMPTY) {
        mm = m; // Copy current move state
        mm.from.x = x;
        mm.from.y = y;
        mm.to.x = x - 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x - 1][y + 1]; // Store captured piece
        mm.del[d + 1].x = -1; // Mark end of captured pieces for this path
        mm.oldpiece = MG_BLACK_KING;
        mm.newpiece = MG_BLACK_KING;
        mm.is_capture = true; // Set is_capture to true for recursive calls

        // Temporarily make the move on the current board
        int original_from_piece = board[x][y];
        int original_captured_piece = board[x - 1][y + 1];
        board[x][y] = MG_EMPTY;
        board[x - 1][y + 1] = MG_EMPTY;
        board[x - 2][y + 2] = mm.newpiece;

        blackkingcapture(board, movelist, mm, x - 2, y + 2, d + 1, n);
        end = 0;

        // Unmake the move
        board[x][y] = original_from_piece;
        board[x - 1][y + 1] = original_captured_piece;
        board[x - 2][y + 2] = MG_EMPTY;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = MG_BLACK_KING;
        movelist[*n].newpiece = MG_BLACK_KING; // Kings remain kings
        movelist[*n].is_capture = true; // Set is_capture to true
        (*n)++;
    }
}

void move4tonotation(const CBmove *m, char s[80]) {
    // Ensure the output string is always null-terminated.
    s[0] = '\0';
    s[79] = '\0';

    int from_square = coorstonumber(m->from.x, m->from.y, GT_ENGLISH);
    
    if (m->jumps > 0) {
        char temp[16];
        snprintf(s, 80, "%d", from_square);
        for (int i = 0; i < m->jumps; ++i) {
            if (i + 1 >= 12) break; // Safety break for path array
            int next_square = coorstonumber(m->path[i + 1].x, m->path[i + 1].y, GT_ENGLISH);
            snprintf(temp, 16, "x%d", next_square);
            strncat(s, temp, 79 - strlen(s));
        }
    } else {
        int to_square = coorstonumber(m->to.x, m->to.y, GT_ENGLISH);
        snprintf(s, 80, "%d-%d", from_square, to_square);
    }
}

static int parse_move_string(const char* move_str, int* from_square, int* to_square) {
    char* mutable_move_str = strdup(move_str); // Create a mutable copy
    if (!mutable_move_str) {
        return 0; // Memory allocation failed
    }

    char* dash_pos = strchr(mutable_move_str, '-');
    if (!dash_pos) {
        free(mutable_move_str); // Free the copy before returning
        return 0; // Invalid format
    }

    *dash_pos = '\0'; // Temporarily null-terminate to read 'from'
    *from_square = atoi(mutable_move_str);
    *to_square = atoi(dash_pos + 1);
    *dash_pos = '-'; // Restore the dash

    free(mutable_move_str); // Free the mutable copy
    if (*from_square == 0 || *to_square == 0) {
        return 0; // Invalid square number
    }
    return 1;
}

static const struct { int from; int to; } three_moves_data[156][3] = {
    {{9, 14}, {23, 18}, {14, 23}},
    {{9, 13}, {22, 18}, {10, 14}},
    {{9, 13}, {22, 18}, {11, 15}},
    {{10, 15}, {22, 17}, {15, 19}},
    {{11, 16}, {23, 19}, {16, 23}},
    {{10, 14}, {22, 17}, {9, 13}},
    {{10, 14}, {22, 18}, {12, 16}},
    {{9, 13}, {24, 20}, {11, 16}},
    {{11, 16}, {21, 17}, {7, 11}},
    {{10, 15}, {21, 17}, {7, 10}},
    {{9, 13}, {22, 18}, {6, 9}},
    {{10, 15}, {21, 17}, {9, 13}},
    {{10, 15}, {22, 17}, {6, 10}},
    {{9, 13}, {23, 18}, {11, 16}},
    {{10, 14}, {22, 18}, {7, 10}},
    {{9, 13}, {22, 18}, {11, 16}},
    {{11, 16}, {22, 17}, {9, 13}},
    {{11, 16}, {22, 18}, {7, 11}},
    {{10, 14}, {22, 18}, {6, 10}},
    {{10, 14}, {22, 17}, {14, 18}},
    {{11, 15}, {23, 18}, {12, 16}},
    {{9, 13}, {23, 18}, {6, 9}},
    {{9, 13}, {24, 19}, {10, 14}},
    {{9, 13}, {22, 18}, {10, 15}},
    {{10, 14}, {24, 19}, {14, 18}},
    {{10, 15}, {21, 17}, {6, 10}},
    {{12, 16}, {24, 20}, {10, 15}},
    {{10, 15}, {23, 18}, {6, 10}},
    {{9, 14}, {22, 18}, {10, 15}},
    {{10, 14}, {22, 17}, {11, 16}},
    {{9, 14}, {23, 19}, {14, 18}},
    {{11, 16}, {24, 19}, {7, 11}},
    {{10, 15}, {23, 19}, {11, 16}},
    {{10, 15}, {22, 17}, {9, 13}},
    {{10, 14}, {24, 19}, {11, 16}},
    {{9, 13}, {23, 18}, {11, 15}},
    {{10, 14}, {23, 18}, {14, 23}},
    {{9, 13}, {22, 18}, {12, 16}},
    {{10, 14}, {23, 19}, {11, 15}},
    {{9, 14}, {22, 17}, {6, 9}},
    {{9, 13}, {23, 19}, {10, 15}},
    {{11, 16}, {23, 18}, {7, 11}},
    {{11, 16}, {21, 17}, {8, 11}},
    {{10, 15}, {22, 17}, {7, 10}},
    {{10, 15}, {23, 18}, {9, 14}},
    {{10, 14}, {23, 19}, {7, 10}},
    {{10, 14}, {22, 17}, {11, 15}},
    {{10, 14}, {24, 20}, {7, 10}},
    {{11, 16}, {22, 17}, {7, 11}},
    {{9, 13}, {24, 19}, {5, 9}},
    {{9, 13}, {24, 20}, {10, 14}},
    {{11, 15}, {21, 17}, {9, 13}},
    {{9, 14}, {22, 18}, {11, 16}},
    {{9, 14}, {22, 17}, {5, 9}},
    {{9, 13}, {23, 18}, {10, 15}},
    {{10, 14}, {23, 19}, {6, 10}},
    {{12, 16}, {22, 18}, {16, 19}},
    {{9, 13}, {24, 19}, {6, 9}},
    {{9, 13}, {24, 20}, {6, 9}},
    {{11, 15}, {24, 20}, {12, 16}},
    {{11, 15}, {21, 17}, {15, 19}},
    {{9, 13}, {23, 19}, {6, 9}},
    {{9, 13}, {24, 20}, {5, 9}},
    {{11, 15}, {24, 19}, {15, 24}},
    {{9, 14}, {22, 18}, {11, 15}},
    {{9, 14}, {24, 20}, {10, 15}},
    {{9, 14}, {24, 19}, {11, 16}},
    {{11, 16}, {22, 18}, {16, 19}},
    {{11, 16}, {24, 20}, {7, 11}},
    {{10, 15}, {21, 17}, {15, 18}},
    {{10, 15}, {23, 18}, {11, 16}},
    {{10, 14}, {24, 19}, {7, 10}},
    {{10, 14}, {24, 20}, {14, 18}},
    {{12, 16}, {24, 20}, {8, 12}},
    {{12, 16}, {21, 17}, {16, 19}},
    {{9, 13}, {24, 19}, {11, 16}},
    {{11, 16}, {23, 18}, {8, 11}},
    {{10, 14}, {24, 19}, {6, 10}},
    {{9, 13}, {23, 19}, {5, 9}},
    {{10, 14}, {23, 19}, {14, 18}},
    {{9, 13}, {23, 18}, {12, 16}},
    {{9, 13}, {23, 19}, {10, 14}},
    {{9, 13}, {21, 17}, {5, 9}},
    {{11, 15}, {22, 18}, {15, 22}},
    {{11, 16}, {23, 18}, {10, 14}},
    {{10, 14}, {22, 18}, {11, 16}},
    {{10, 14}, {24, 20}, {11, 16}},
    {{9, 13}, {23, 19}, {11, 16}},
    {{11, 15}, {23, 18}, {8, 11}},
    {{11, 15}, {24, 20}, {8, 11}},
    {{11, 16}, {22, 18}, {16, 20}},
    {{11, 16}, {23, 18}, {9, 14}},
    {{11, 16}, {22, 17}, {16, 20}},
    {{11, 16}, {21, 17}, {16, 20}},
    {{10, 15}, {21, 17}, {11, 16}},
    {{12, 16}, {23, 18}, {16, 19}},
    {{12, 16}, {21, 17}, {9, 14}},
    {{9, 13}, {24, 19}, {11, 15}},
    {{11, 15}, {23, 19}, {8, 11}},
    {{11, 15}, {22, 17}, {8, 11}},
    {{9, 14}, {22, 17}, {11, 16}},
    {{9, 14}, {24, 20}, {11, 16}},
    {{11, 16}, {22, 18}, {8, 11}},
    {{11, 16}, {21, 17}, {9, 14}},
    {{11, 16}, {24, 20}, {16, 19}},
    {{10, 15}, {24, 20}, {15, 19}},
    {{12, 16}, {22, 18}, {16, 20}},
    {{11, 15}, {22, 17}, {15, 19}},
    {{11, 15}, {22, 17}, {15, 18}},
    {{11, 15}, {23, 18}, {10, 14}},
    {{9, 14}, {24, 20}, {11, 15}},
    {{11, 16}, {23, 18}, {16, 20}},
    {{11, 16}, {24, 19}, {16, 20}},
    {{10, 15}, {22, 17}, {11, 16}},
    {{10, 15}, {23, 18}, {7, 10}},
    {{10, 15}, {24, 19}, {15, 24}},
    {{10, 15}, {24, 20}, {7, 10}},
    {{10, 14}, {22, 18}, {11, 15}},
    {{12, 16}, {23, 18}, {16, 20}},
    {{12, 16}, {24, 19}, {16, 20}},
    {{12, 16}, {21, 17}, {9, 13}},
    {{9, 13}, {23, 18}, {5, 9}},
    {{9, 13}, {24, 20}, {10, 15}},
    {{9, 13}, {21, 17}, {6, 9}},
    {{11, 15}, {23, 19}, {9, 14}},
    {{11, 15}, {23, 18}, {9, 14}},
    {{11, 15}, {23, 18}, {15, 19}},
    {{11, 15}, {21, 17}, {8, 11}},
    {{9, 14}, {24, 19}, {11, 15}},
    {{9, 14}, {23, 19}, {11, 16}},
    {{11, 16}, {24, 19}, {8, 11}},
    {{11, 16}, {21, 17}, {9, 13}},
    {{10, 15}, {22, 18}, {15, 22}},
    {{10, 15}, {23, 19}, {7, 10}},
    {{10, 15}, {24, 20}, {6, 10}},
    {{10, 14}, {22, 17}, {7, 10}},
    {{10, 14}, {24, 20}, {11, 15}},
    {{10, 14}, {24, 20}, {6, 10}},
    {{12, 16}, {22, 17}, {16, 20}},
    {{9, 13}, {22, 17}, {13, 22}},
    {{11, 15}, {23, 19}, {9, 13}},
    {{11, 15}, {22, 17}, {9, 13}},
    {{11, 15}, {24, 20}, {15, 18}},
    {{11, 15}, {21, 17}, {9, 14}},
    {{9, 14}, {22, 18}, {5, 9}},
    {{9, 14}, {22, 17}, {11, 15}},
    {{9, 14}, {24, 20}, {5, 9}},
    {{9, 14}, {24, 19}, {5, 9}},
    {{9, 14}, {23, 19}, {5, 9}},
    {{11, 16}, {22, 17}, {8, 11}},
    {{10, 15}, {23, 18}, {12, 16}},
    {{10, 15}, {23, 19}, {6, 10}},
    {{10, 14}, {23, 19}, {11, 16}},
    {{12, 16}, {22, 17}, {16, 19}},
    {{12, 16}, {21, 17}, {16, 20}},
    {{9, 13}, {24, 20}, {11, 15}}
};

void start3move_c(Board8x8* board, int opening_index) {
    newgame(board); // Start with a fresh board

    if (opening_index >= 0 && opening_index < 156) {
        int current_color = CB_BLACK; // 3-move openings typically start with Black

        for (int i = 0; i < 3; ++i) {
            int from_square = three_moves_data[opening_index][i].from;
            int to_square = three_moves_data[opening_index][i].to;

            int from_x, from_y, to_x, to_y;
            numbertocoors(from_square, &from_x, &from_y, GT_ENGLISH);
            numbertocoors(to_square, &to_x, &to_y, GT_ENGLISH);

            CBmove move;
            move.from.x = from_x;
            move.from.y = from_y;
            move.to.x = to_x;
            move.to.y = to_y;
            move.jumps = 0; // Assuming no jumps in opening moves for simplicity

            // Determine the piece type and color based on the current board state
            // This is a simplification; a full implementation would use getmovelist
            // to find the exact move, including potential captures.
            // For 3-move openings, we assume simple moves.
            int piece = board->board[from_y][from_x];
            move.oldpiece = piece;
            move.newpiece = piece; // Piece type doesn't change unless kinged, handled by domove_c

            domove_c(&move, board);

            // Toggle color for the next move
            current_color = (current_color == CB_BLACK) ? CB_WHITE : CB_BLACK;
        }
    }
}

void domove_c(const CBmove *move, Board8x8* board) {

    int piece = board->board[move->from.y][move->from.x];



    // Move the piece

    board->board[move->to.y][move->to.x] = move->newpiece;

    board->board[move->from.y][move->from.x] = CB_EMPTY;



    // Handle captures

    if (move->jumps > 0) {

        for (int i = 0; i < move->jumps; ++i) {

            board->board[move->del[i].y][move->del[i].x] = CB_EMPTY;

        }

    }

}

void unmake_move_c(const CBmove *move, Board8x8* board) {
    // Move the piece back from 'to' to 'from'
    board->board[move->from.y][move->from.x] = move->oldpiece;
    board->board[move->to.y][move->to.x] = CB_EMPTY;

    // Restore captured pieces
    if (move->jumps > 0) {
        for (int i = 0; i < move->jumps; ++i) {
            board->board[move->del[i].y][move->del[i].x] = move->delpiece[i];
        }
    }
}

// Content from egdb_wrapper.c

// Include the original dblookup.h for db_init/dblookup declarations


// Implementation for Kingsrow EGDB initialization
int egdb_wrapper_init(const char* egdb_path) {
    char path_buffer[256];
    char log_msg[256];
    sprintf(log_msg, "EGDB Wrapper: Initializing with path: %s", egdb_path);
    log_c(LOG_LEVEL_INFO, log_msg);
    // Safely copy the path to a mutable buffer
    strncpy(path_buffer, egdb_path, sizeof(path_buffer));
    path_buffer[sizeof(path_buffer) - 1] = '\0'; // Ensure null-termination
    return db_init(64, path_buffer, egdb_path); // Pass egdb_path as the third argument
}

// Implementation for Kingsrow EGDB lookup
int egdb_wrapper_lookup(pos* p, int side_to_move) {
    char log_msg[256];
    sprintf(log_msg, "EGDB Wrapper: Lookup for position: bm=%u, bk=%u, wm=%u, wk=%u, side_to_move=%d", p->bm, p->bk, p->wm, p->wk, side_to_move);
    log_c(LOG_LEVEL_DEBUG, log_msg);
    // Convert our 'pos' struct to the EGDB's 'POSITION' struct
    POSITION egdb_pos;
    egdb_pos.bm = p->bm;
    egdb_pos.bk = p->bk;
    egdb_pos.wm = p->wm;
    egdb_pos.wk = p->wk;

    // Call the actual Kingsrow EGDB lookup function
    int result = dblookup(&egdb_pos, side_to_move);
    // If dblookup returns DB_NOT_LOOKED_UP, it means the position was not in cache during conditional lookup.
    // We don't log this as an error, but rather as debug info.
    log_c(LOG_LEVEL_DEBUG, log_msg);
    return result;
}

// Implementation for Kingsrow EGDB exit
int egdb_wrapper_exit() {
    log_c(LOG_LEVEL_INFO, "EGDB Wrapper: Exiting EGDB.");
    return db_exit();
}

char* read_text_file_c(const char* filename, enum READ_TEXT_FILE_ERROR_TYPE* etype) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        if (etype) *etype = READ_TEXT_FILE_DOES_NOT_EXIST;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length == -1) {
        fclose(file);
        if (etype) *etype = READ_TEXT_FILE_OTHER_ERROR;
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(file);
        if (etype) *etype = READ_TEXT_FILE_COULD_NOT_OPEN;
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    fclose(file);

    if (bytesRead != length) {
        free(buffer);
        if (etype) *etype = READ_TEXT_FILE_OTHER_ERROR;
        return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the string
    if (etype) *etype = READ_TEXT_FILE_NO_ERROR;
    return buffer;
}
