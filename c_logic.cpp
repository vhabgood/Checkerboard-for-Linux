#include "c_logic.h"
#include "dblookup.h"
#include "checkers_c_types.h" // Direct include
#include "log.h"



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
int FENtoboard8(Board8x8 *board, const char *FEN, int *color, int gametype)
{
    int square, square2, s;
    int piece_color, piece_type;
    int i, j;
    const char *lastp;
    char *p;

    // Create a mutable copy of FEN
    char FEN_copy[256];
    strncpy(FEN_copy, FEN, 255);
    FEN_copy[255] = '\0';

    char *token;
    char *saveptr;

    // Parse color to move
    token = strtok_r(FEN_copy, ":", &saveptr);
    if (!token) return 0;

    if (toupper(token[0]) == 'B')
        *color = CB_BLACK;
    else if (toupper(token[0]) == 'W')
        *color = CB_WHITE;
    else
        return (0);
    
    for (i = 0; i < 8; ++i)
        for (j = 0; j < 8; ++j)
            board->board[i][j] = 0;

    // Now, parse the piece list
    token = strtok_r(NULL, "", &saveptr); // Get the rest of the string
    if (!token) return 0;
    
    p = token; // Set 'p' to point to the beginning of the piece list
    const char *fen_end = FEN_copy + strlen(FEN_copy); // Calculate the end of the FEN string within the buffer

    while (p < fen_end && *p) { // Loop while p is within bounds and not null terminator
        piece_type = CB_MAN;
        if (p < fen_end && *p == '"') {
            ++p;
            continue;
        }
        if (p < fen_end && toupper(*p) == 'W') {
            piece_color = CB_WHITE;
            ++p;
            continue;
        }
        if (p < fen_end && toupper(*p) == 'B') {
            piece_color = CB_BLACK;
            ++p;
            continue;
        }
        if (p < fen_end && toupper(*p) == 'K') {
            piece_type = CB_KING;
            ++p;
        }
        for (square = 0; p < fen_end && isdigit(*p); ++p)
            square = 10 * square + (*p - '0');
        square2 = square;
        if (p < fen_end && (*p == ',' || *p == ':'))
            ++p;
        else if (p < fen_end && *p == '-' && p + 1 < fen_end && isdigit(p[1])) {
            ++p;
            for (square2 = 0; p < fen_end && isdigit(*p); ++p)
                square2 = 10 * square2 + (*p - '0');
            if (p < fen_end && (*p == ',' || *p == ':'))
                ++p;
        }
        if (square && square <= square2) {
            for (s = square; s <= square2; ++s) {
                numbertocoors(s, &i, &j, gametype);
                if (i >= 0 && i < 8 && j >= 0 && j < 8) { // Added bounds check for board
                    board->board[j][i] = piece_type | piece_color;
                }
            }
        }
        if (p < fen_end && *p == ',')
            ++p;
    }
    return (1);
}
void board8toFEN(const Board8x8 *board, char *FEN, int color, int gametype)
{
    char board_str[256] = {0};
    char *ptr = board_str;
    int remaining = sizeof(board_str);
    log_c(LOG_LEVEL_DEBUG, "--- board8toFEN: Start ---");
    for (int y = 0; y < 8; ++y) {
        int written = snprintf(ptr, remaining, "%d %d %d %d %d %d %d %d\n",
                board->board[y][0], board->board[y][1], board->board[y][2], board->board[y][3],
                board->board[y][4], board->board[y][5], board->board[y][6], board->board[y][7]);
        if (written > 0 && written < remaining) {
            ptr += written;
            remaining -= written;
        }
    }
    log_c(LOG_LEVEL_DEBUG, board_str);


    char white_pieces[100] = "";
    char black_pieces[100] = "";
    char temp[10];
    int first_white = 1;
    int first_black = 1;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((y % 2) != (x % 2)) {
                int piece = board->board[y][x];
                int square = coorstonumber(x, y, gametype);
                if ((piece & CB_WHITE) != 0) {
                    if (!first_white) {
                        strncat(white_pieces, ",", sizeof(white_pieces) - strlen(white_pieces) - 1);
                    }
                    if ((piece & CB_KING) != 0) {
                        strncat(white_pieces, "K", sizeof(white_pieces) - strlen(white_pieces) - 1);
                    }
                    snprintf(temp, sizeof(temp), "%d", square);
                    strncat(white_pieces, temp, sizeof(white_pieces) - strlen(white_pieces) - 1);
                    first_white = 0;
                } else if ((piece & CB_BLACK) != 0) {
                    if (!first_black) {
                        strncat(black_pieces, ",", sizeof(black_pieces) - strlen(black_pieces) - 1);
                    }
                    if ((piece & CB_KING) != 0) {
                        strncat(black_pieces, "K", sizeof(black_pieces) - strlen(black_pieces) - 1);
                    }
                    snprintf(temp, sizeof(temp), "%d", square);
                    strncat(black_pieces, temp, sizeof(black_pieces) - strlen(black_pieces) - 1);
                    first_black = 0;
                }
            }
        }
    }

    if (color == CB_WHITE) {
        snprintf(FEN, 256, "W:W%s:B%s", white_pieces, black_pieces); // Use 256 for FEN buffer size
    } else {
        snprintf(FEN, 256, "B:W%s:B%s", white_pieces, black_pieces); // Use 256 for FEN buffer size
    }
    log_c(LOG_LEVEL_DEBUG, "--- board8toFEN: End ---");
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
            int board[32];
            for (int k = 0; k < 12; k++) { //black pieces
                board[k] = (CB_BLACK | CB_MAN);
            }
            for (int k = 12; k < 20; k++) { //empty squares
                board[k] = CB_EMPTY;
            }
            for (int k = 20; k < 32; k++) { //white pieces
                board[k] = (CB_WHITE | CB_MAN);
            }

            for (int k = 0; k < 32; k++) {
                int r, c;
                numbertocoors(k + 1, &c, &r, game->gametype);
                b8.board[r][c] = board[k];
            }
        } else {
            int board[32];
            for (int k = 0; k < 12; k++) { //white pieces
                board[k] = (CB_WHITE | CB_MAN);
            }
            for (int k = 12; k < 20; k++) { //empty squares
                board[k] = CB_EMPTY;
            }
            for (int k = 20; k < 32; k++) { //black pieces
                board[k] = (CB_BLACK | CB_MAN);
            }

            for (int k = 0; k < 32; k++) {
                int r, c;
                numbertocoors(k + 1, &c, &r, game->gametype);
                b8.board[r][c] = board[k];
            }
        }
        
    } else {
        // setup
        FENtoboard8(&b8, game->FEN, &returncolor, game->gametype);
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            b[8 * (7 - j) + i] = b8.board[i][j];
            if (game->gametype == 21)
                b[8 * (7 - j) + i] = b8.board[i][7 - j];
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
    if (gametype == GT_ITALIAN || gametype == GT_SPANISH || GT_RUSSIAN || gametype == GT_CZECH) // Fixed comma
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

// Forward declarations for internal move generation functions
// These are now declared in c_logic.h and implemented below.

int get_legal_moves_c(const Board8x8* board, int color, CBmove movelist[MAXMOVES], int *nmoves, int *isjump, const CBmove *last_move, bool *can_continue_multijump)
{
    CBmove all_moves[MAXMOVES];
    int all_nmoves = 0;
    int all_isjump = 0;

    if (can_continue_multijump) {
        *can_continue_multijump = false;
    }

    makemovelist(board, color, all_moves, &all_isjump, &all_nmoves);

    log_c(LOG_LEVEL_DEBUG, "--- get_legal_moves_c: Start ---");
    log_c(LOG_LEVEL_DEBUG, "Finding legal moves for color");

    if (last_move != NULL && last_move->jumps > 0 && all_isjump) {
        int filtered_count = 0;
        for (int i = 0; i < all_nmoves; ++i) {
            if (all_moves[i].from.x == last_move->to.x &&
                all_moves[i].from.y == last_move->to.y) {
                movelist[filtered_count++] = all_moves[i];
                if (can_continue_multijump) { // Add check here
                    *can_continue_multijump = true;
                }
            }
        }

        if (filtered_count > 0) {
            *nmoves = filtered_count;
            *isjump = 1;
        } else {
            *nmoves = 0;
            *isjump = 0;
            if (can_continue_multijump) { // Add check here
                *can_continue_multijump = false;
            }
        }
    } else {
        memcpy(movelist, all_moves, all_nmoves * sizeof(CBmove));
        *nmoves = all_nmoves;
        *isjump = all_isjump;
        if (can_continue_multijump) { // Add check here
            *can_continue_multijump = false;
        }
    }

    char log_msg[100];
    sprintf(log_msg, "Found %d legal moves. Is jump: %d", *nmoves, *isjump);
    log_c(LOG_LEVEL_DEBUG, log_msg);

    return *nmoves;
}



extern "C" void find_captures_recursive(const Board8x8* board, CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n, int color, int is_king, int* visited)
{
    char log_msg[250];
    sprintf(log_msg, "find_captures_recursive: Entry. board_ptr=%p, x=%d, y=%d, d=%d, color=%d, is_king=%d", board, x, y, d, color, is_king);
    log_c(LOG_LEVEL_DEBUG, log_msg);

    int i;

    // Safety check: if recursion depth exceeds the maximum allowed jumps, stop.
    // MAX_JUMPS_PER_MOVE is effectively 11 (since del array is size 12 and del[11] is max index)
    if (d >= 11) { // If d is 11, d+1 would be 12, out of bounds for del[12]
        return;
    }

    // Define movement vectors based on color
    int fwd = (color == CB_WHITE) ? 1 : -1; // White moves y+1, Black moves y-1

    // Directions: 0:fwd-left, 1:fwd-right, 2:back-left, 3:back-right
    int dx[] = {-1, 1, -1, 1};
    int temp_dy[4];
    temp_dy[0] = fwd;
    temp_dy[1] = fwd;
    temp_dy[2] = -fwd;
    temp_dy[3] = -fwd;
    int* dy = temp_dy;

    int start_dir = 0;
    int end_dir = is_king ? 4 : 2; // Men only check the first 2 (forward) directions

    log_c(LOG_LEVEL_DEBUG, "find_captures_recursive: Entering loop. start_dir=0, end_dir=2"); // Log before the loop for better context

    for (i = start_dir; i < end_dir; ++i) {
        int jump_x = x + dx[i];
        int jump_y = y + dy[i];
        int land_x = x + 2 * dx[i];
        int land_y = y + 2 * dy[i];
        sprintf(log_msg, "find_captures_recursive: Checking jump from (%d,%d) to (%d,%d) over (%d,%d). Depth: %d", x, y, land_x, land_y, jump_x, jump_y, d);
        log_c(LOG_LEVEL_DEBUG, log_msg);

        // Check bounds for 8x8 board
        if (jump_x >= 0 && jump_x < 8 && jump_y >= 0 && jump_y < 8 && land_x >= 0 && land_x < 8 && land_y >= 0 && land_y < 8) {
            // Check for a valid capture in this direction
            int opponent_piece = board->board[jump_y][jump_x];
            if (opponent_piece == CB_EMPTY) {
                continue;
            }
            int landing_square = board->board[land_y][land_x];

            // An opponent piece has the opposite color
            bool is_opponent = false;
            if (color == CB_WHITE) { // Current piece is White
                if ((opponent_piece & CB_BLACK) != 0) { // Opponent is Black
                    is_opponent = true;
                }
            } else { // Current piece is Black
                if ((opponent_piece & CB_WHITE) != 0) { // Opponent is White
                    is_opponent = true;
                }
            }

            if (is_opponent && landing_square == CB_EMPTY) {
                CBmove mm = m;  // Copy the current move sequence
                // Update move details for this jump
                mm.to.x = land_x;
                mm.to.y = land_y;
                mm.path[d + 1].x = land_x;
                mm.path[d + 1].y = land_y;
                mm.del[d].x = jump_x;
                mm.del[d].y = jump_y;
                mm.delpiece[d] = opponent_piece;
                
                // Only set sentinel if there is space
                if (d + 1 < 12) { // 12 is the size of del array
                    mm.del[d + 1].x = -1; // Sentinel
                }
                mm.jumps = d + 1;

                // Determine the piece type after the move (promotion)
                int becomes_king = 0;
                if (!is_king) {
                    if (color == CB_WHITE && land_y == 7) becomes_king = 1; // White reaches back rank
                    if (color == CB_BLACK && land_y == 0) becomes_king = 1; // Black reaches back rank
                }
                int next_is_king = is_king || becomes_king;
                int new_piece_type = next_is_king ? (color == CB_WHITE ? (CB_WHITE | CB_KING) : (CB_BLACK | CB_KING)) : m.oldpiece;
                mm.newpiece = new_piece_type;
                movelist[*n] = mm;
                (*n)++;
            }
        }
    }
}


void makemovelist(const Board8x8* board, int color, CBmove movelist[MAXMOVES], int *isjump, int *n)
{
    // char log_msg[200]; // Removed
    // sprintf(log_msg, "makemovelist: Entry. Finding moves for color: %d", color); // Removed
    // log_c(LOG_LEVEL_DEBUG, log_msg); // Removed

    int i;
    CBmove m = {0};
    *isjump = 0;

    for (i = 0; i < MAXMOVES; i++) {
        movelist[i].jumps = 0;
        movelist[i].is_capture = false;
    }
    *n = 0;

    // --- 1. Find all possible captures first ---
    CBmove captures[MAXMOVES];
    int ncaptures = 0;
    int visited[33] = {0};
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board->board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            int is_king = (piece & CB_KING) ? 1 : 0;

            if (piece_color == color) {
                m.from.x = c;
                m.from.y = r;
                m.path[0].x = c;
                m.path[0].y = r;
                m.oldpiece = piece;
                find_captures_recursive(board, captures, m, c, r, 0, &ncaptures, color, is_king, visited);
            }
        }
    }

    if (ncaptures > 0) {
        // sprintf(log_msg, "makemovelist: %d captures found. Prioritizing captures.", ncaptures); // Removed
        // log_c(LOG_LEVEL_DEBUG, log_msg); // Removed
        *isjump = 1;
        int max_jumps = 0;
        for (i = 0; i < ncaptures; ++i) {
            if (captures[i].jumps > max_jumps) {
                max_jumps = captures[i].jumps;
            }
        }

        for (i = 0; i < ncaptures; ++i) {
            if (captures[i].jumps == max_jumps) {
                movelist[*n] = captures[i];
                (*n)++;
            }
        }
        for(i=0; i<*n; ++i) movelist[i].is_capture = true;
        return;
    }

    // sprintf(log_msg, "makemovelist: No captures found. Total ncaptures: %d.", ncaptures); // Removed
    // log_c(LOG_LEVEL_DEBUG, log_msg); // Removed
    // log_c(LOG_LEVEL_DEBUG, "makemovelist: Entering regular move generation phase (no captures found)."); // Removed

    // --- 2. If no captures, find all regular moves ---
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board->board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            int is_king = (piece & CB_KING) ? 1 : 0;

            if (piece_color == color) {
                int fwd = (color == CB_WHITE) ? 1 : -1; // White moves y+1, Black moves y-1
                int dx[] = {-1, 1};
                
                // Men moves (forward only)
                if (!is_king) {
                    for (i = 0; i < 2; ++i) {
                        int to_c = c + dx[i];
                        int to_r = r + fwd;
                        if (to_c >= 0 && to_c < 8 && to_r >= 0 && to_r < 8 && board->board[to_r][to_c] == CB_EMPTY) {
                            movelist[*n].jumps = 0;
                            movelist[*n].from.x = c;
                            movelist[*n].from.y = r;
                            movelist[*n].to.x = to_c;
                            movelist[*n].to.y = to_r;
                            movelist[*n].path[0].x = c;
                            movelist[*n].path[0].y = r;
                            movelist[*n].path[1].x = to_c;
                            movelist[*n].path[1].y = to_r;
                            movelist[*n].del[0].x = -1;
                            movelist[*n].oldpiece = piece;
                            if ((color == CB_WHITE && to_r == 7) || (color == CB_BLACK && to_r == 0)) {
                                movelist[*n].newpiece = piece | CB_KING;
                            } else {
                                movelist[*n].newpiece = piece;
                            }
                            (*n)++;
                        }
                    }
                }
                // King moves (all 4 diagonal directions)
                else {
                    int king_dy[] = {1, 1, -1, -1};
                    int king_dx[] = {-1, 1, -1, 1};
                    for (i = 0; i < 4; ++i) {
                        int to_c = c + king_dx[i];
                        int to_r = r + king_dy[i];
                        if (to_c >= 0 && to_c < 8 && to_r >= 0 && to_r < 8 && board->board[to_r][to_c] == CB_EMPTY) {
                            movelist[*n].jumps = 0;
                            movelist[*n].from.x = c;
                            movelist[*n].from.y = r;
                            movelist[*n].to.x = to_c;
                            movelist[*n].to.y = to_r;
                            movelist[*n].path[0].x = c;
                            movelist[*n].path[0].y = r;
                            movelist[*n].path[1].x = to_c;
                            movelist[*n].path[1].y = to_r;
                            movelist[*n].del[0].x = -1;
                            movelist[*n].oldpiece = piece;
                            movelist[*n].newpiece = piece; // Kings don't change type on non-capture moves
                            (*n)++;
                        }
                    }
                }
            }
        }
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

    char* dash = strchr(mutable_move_str, '-');
    if (!dash) {
        free(mutable_move_str); // Free the copy before returning
        return 0; // Invalid format
    }

    *dash = '\0'; // Temporarily null-terminate to read 'from'
    *from_square = atoi(mutable_move_str);
    *to_square = atoi(dash + 1);
    *dash = '-'; // Restore the dash

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

void domove_c(const CBmove *move, Board8x8 *board)
{
    char log_msg[200];
    sprintf(log_msg, "--- domove_c: Start --- Move from (%d,%d) to (%d,%d). Piece: %d. Jumps: %d. Before: from_sq=%d, to_sq=%d",
            move->from.x, move->from.y, move->to.x, move->to.y, move->oldpiece, move->jumps,
            board->board[move->from.y][move->from.x], board->board[move->to.y][move->to.x]);
    log_c(LOG_LEVEL_DEBUG, log_msg);

    int from_x = move->from.x;
    int from_y = move->from.y;
    int to_x = move->to.x;
    int to_y = move->to.y;

    sprintf(log_msg, "domove_c: Accessing board at from=(%d,%d) and to=(%d,%d)", from_x, from_y, to_x, to_y);
    log_c(LOG_LEVEL_DEBUG, log_msg);

    // Check bounds just in case
    if (from_x < 0 || from_x >= 8 || from_y < 0 || from_y >= 8 || to_x < 0 || to_x >= 8 || to_y < 0 || to_y >= 8) {
        log_c(LOG_LEVEL_ERROR, "domove_c: Out of bounds coordinates!");
        return;
    }


    int x, y, i;

    board->board[move->to.y][move->to.x] = board->board[move->from.y][move->from.x];
    board->board[move->from.y][move->from.x] = CB_EMPTY;

    if (move->jumps > 0) {
        for (i = 0; i < move->jumps; i++) {
            board->board[move->del[i].y][move->del[i].x] = CB_EMPTY;
        }
    }

    /* promote to king if necessary */
    if (move->to.y == 0 && board->board[move->to.y][move->to.x] == (CB_BLACK | CB_MAN))
        board->board[move->to.y][move->to.x] = (CB_BLACK | CB_KING);
    if (move->to.y == 7 && board->board[move->to.y][move->to.x] == (CB_WHITE | CB_MAN))
        board->board[move->to.y][move->to.x] = (CB_WHITE | CB_KING);
    
    sprintf(log_msg, "--- domove_c: End --- After: from_sq=%d, to_sq=%d",
            board->board[move->from.y][move->from.x], board->board[move->to.y][move->to.x]);
    log_c(LOG_LEVEL_DEBUG, log_msg);
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
    pos egdb_pos;
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
