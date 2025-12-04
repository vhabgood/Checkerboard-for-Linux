#include "c_logic.h"
#include "checkers_types.h" // Includes checkers_c_types.h via conditional logic
#include "log.h"
#include "DBManager.h"

// Global variables defined here, declared extern in checkers_c_types.h
char bitsinword[65536]; 
uint32_t revword[65536]; 

// Implementations for bit manipulation functions are now in core_types.h

int count_pieces(const bitboard_pos* board) {
    return recbitcount(board->bm) + recbitcount(board->bk) +
           recbitcount(board->wm) + recbitcount(board->wk);
}

void boardtobitboard(const Board8x8* b, bitboard_pos *bitboard_position)
{
    int i, j, bit_bitboard_pos;
    bitboard_position->bm = 0; bitboard_position->bk = 0; bitboard_position->wm = 0; bitboard_position->wk = 0;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if ((i + j) % 2 == 1) { // Only dark squares
                bit_bitboard_pos = coorstonumber(j, i, GT_ENGLISH) - 1; // Convert to 0-indexed bit bitboard_position
                if (bit_bitboard_pos >= 0 && bit_bitboard_pos < 32) { // Ensure bit_bitboard_pos is valid
                    switch (b->board[i][j]) {
                        case (CB_BLACK | CB_MAN):
                            bitboard_position->bm |= (1 << bit_bitboard_pos);
                            break;
                        case (CB_BLACK | CB_KING):
                            bitboard_position->bk |= (1 << bit_bitboard_pos);
                            break;
                        case (CB_WHITE | CB_MAN):
                            bitboard_position->wm |= (1 << bit_bitboard_pos);
                            break;
                        case (CB_WHITE | CB_KING):
                            bitboard_position->wk |= (1 << bit_bitboard_pos);
                            break;
                    }
                }
            }
        }
    }
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
        *x = 7 - (offset_in_row * 2);
    }
    *y = 7 - adjusted_y;
    // char log_msg_buffer[256];
    // snprintf(log_msg_buffer, sizeof(log_msg_buffer), "numbertocoors: n=%d -> x=%d, y=%d", n, *x, *y);
    // log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
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
int FENtobitboard_pos(bitboard_pos* bitboard_position, const char *FEN, int *color, int gametype)
{
    char log_msg_buffer[2048];
    log_c(LOG_LEVEL_DEBUG, "FENtobitboard_pos: Entering function.");
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Received FEN: %s", FEN);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);

    // Create a mutable copy of FEN
    char FEN_copy[1024];
    strncpy(FEN_copy, FEN, 1023);
    FEN_copy[1023] = '\0';

    char *p = FEN_copy;
    char *token;
    char *saveptr;

    // 1. Parse color to move from the very beginning
    token = strtok_r(p, ":", &saveptr);
    if (!token) {
        log_c(LOG_LEVEL_ERROR, "FENtobitboard_pos: Failed to parse color token.");
        return 0;
    }
    if (toupper(token[0]) == 'B') {
        *color = CB_BLACK;
    } else if (toupper(token[0]) == 'W') {
        *color = CB_WHITE;
    } else {
        snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Invalid color character in FEN: %c", token[0]);
        log_c(LOG_LEVEL_ERROR, log_msg_buffer);
        return 0;
    }
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Parsed color to move: %d", *color);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);

    // 2. Initialize bitboards to empty
    bitboard_position->bm = 0; bitboard_position->bk = 0; bitboard_position->wm = 0; bitboard_position->wk = 0;

    // 3. Loop through the remaining color sections (W:... and B:...)
    while ((token = strtok_r(NULL, ":", &saveptr)) != NULL) {
        int piece_color;
        
        if (toupper(token[0]) == 'W') {
            piece_color = CB_WHITE;
        } else if (toupper(token[0]) == 'B') {
            piece_color = CB_BLACK;
        } else {
            snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Invalid piece color specifier: %c", token[0]);
            log_c(LOG_LEVEL_WARNING, log_msg_buffer);
            continue; // Skip to next token
        }

        // Now parse the squares for this color
        char* squares_ptr = token + 1; // Move past 'W' or 'B'
        char* square_token;
        char* square_saveptr;

        square_token = strtok_r(squares_ptr, ",", &square_saveptr);
        while(square_token != NULL) {
            int piece_type = CB_MAN;
            if (toupper(square_token[0]) == 'K') {
                piece_type = CB_KING;
                square_token++; // Move past 'K'
            }

            int square_num = atoi(square_token);
            if (square_num >= 1 && square_num <= 32) {
                // Convert 1-indexed square_num to 0-indexed bit bitboard_position
                int bit_bitboard_pos = square_num - 1; 
                set_piece(bitboard_position, bit_bitboard_pos, piece_type | piece_color);

                snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Placed piece at square %d (bit_bitboard_pos:%d)", square_num, bit_bitboard_pos);
                log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
            } else {
                snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Invalid square number %d", square_num);
                log_c(LOG_LEVEL_WARNING, log_msg_buffer);
            }

            square_token = strtok_r(NULL, ",", &square_saveptr);
        }
    }

    // After parsing, before returning
    char board_fen_after_parse[1024];
    bitboard_postoFEN(bitboard_position, board_fen_after_parse, *color, gametype); // Generate FEN from the parsed board
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "FENtobitboard_pos: Board state after parsing: %s", board_fen_after_parse);
    log_c(LOG_LEVEL_DEBUG, log_msg_buffer);
    
    log_c(LOG_LEVEL_DEBUG, "FENtobitboard_pos: Exiting function successfully.");
    return (1);
}
void bitboard_postoFEN(const bitboard_pos* bitboard_position, char *FEN, int color, int gametype)
{
    char white_pieces[100] = "";
    char black_pieces[100] = "";
    char temp[10];
    int first_white = 1;
    int first_black = 1;

    for (int square_num = 1; square_num <= 32; ++square_num) {
        int bit_bitboard_pos = square_num - 1; // 0-indexed bit bitboard_position
        int piece = get_piece(bitboard_position, bit_bitboard_pos); // Use get_piece helper

        if ((piece & CB_WHITE) != 0) {
            if (!first_white) {
                strncat(white_pieces, ",", sizeof(white_pieces) - strlen(white_pieces) - 1);
            }
            if ((piece & CB_KING) != 0) {
                strncat(white_pieces, "K", sizeof(white_pieces) - strlen(white_pieces) - 1);
            }
            snprintf(temp, sizeof(temp), "%d", square_num);
            strncat(white_pieces, temp, sizeof(white_pieces) - strlen(white_pieces) - 1);
            first_white = 0;
        } else if ((piece & CB_BLACK) != 0) {
            if (!first_black) {
                strncat(black_pieces, ",", sizeof(black_pieces) - strlen(black_pieces) - 1);
            }
            if ((piece & CB_KING) != 0) {
                strncat(black_pieces, "K", sizeof(black_pieces) - strlen(black_pieces) - 1);
            }
            snprintf(temp, sizeof(temp), "%d", square_num);
            strncat(black_pieces, temp, sizeof(black_pieces) - strlen(black_pieces) - 1);
            first_black = 0;
        }
    }

    if (color == CB_WHITE) {
        snprintf(FEN, 1024, "W:W%s:B%s", white_pieces, black_pieces);
    } else {
        snprintf(FEN, 1024, "B:W%s:B%s", white_pieces, black_pieces);
    }
}

// From game_logic.h
void newgame(bitboard_pos* board) {
    // Standard starting bitboard_position in bitboards
    board->bm = 0x00000FFF; // Squares 1-12 (Black Men)
    board->bk = 0x00000000;
    board->wm = 0xFFF00000; // Squares 21-32 (White Men)
    board->wk = 0x00000000;
    board->color = CB_BLACK; // Black to move
}

bitboard_pos get_initial_board(int gameType) {
    bitboard_pos board;
    newgame(&board); // Use the existing newgame function to set up the board
    // Optionally, you could add logic here to customize initial board based on gameType
    return board;
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

int get_legal_moves_c(const bitboard_pos* board, int color, CBmove movelist[MAXMOVES], int *nmoves, int *isjump, const CBmove *last_move, bool *can_continue_multijump)
{
    // --- DEBUG: Log FEN upon entering function ---
    char fen_in_get_legal[256];
    bitboard_postoFEN(board, fen_in_get_legal, color, GT_ENGLISH);
    log_c(LOG_LEVEL_DEBUG, (QString("get_legal_moves_c: Received board with FEN: ") + fen_in_get_legal).toUtf8().constData());
    // --- END DEBUG ---

    CBmove all_moves[MAXMOVES];
    int all_nmoves = 0;
    int all_isjump = 0;

    if (can_continue_multijump) {
        *can_continue_multijump = false;
    }

    makemovelist(board, color, all_moves, &all_isjump, &all_nmoves); // makemovelist now takes bitboard_pos*

    if (last_move != NULL && last_move->jumps > 0 && all_isjump) {
        int filtered_count = 0;
        for (int i = 0; i < all_nmoves; ++i) {
            if (all_moves[i].from.x == last_move->to.x &&
                all_moves[i].from.y == last_move->to.y) {
                movelist[filtered_count++] = all_moves[i];
                if (can_continue_multijump) {
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
            if (can_continue_multijump) {
                *can_continue_multijump = false;
            }
        }
    } else {
        memcpy(movelist, all_moves, all_nmoves * sizeof(CBmove));
        *nmoves = all_nmoves;
        *isjump = all_isjump;
        if (can_continue_multijump) {
            *can_continue_multijump = false;
        }
    }

    return *nmoves;
}



extern "C" void find_captures_recursive(const bitboard_pos* board, CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n, int color, int is_king, const int* visited_parent)
{
    char log_msg[512];
//    sprintf(log_msg, "find_captures_recursive: x: %d, y: %d, d: %d, color: %d, is_king: %d", x, y, d, color, is_king);
//    log_c(LOG_LEVEL_DEBUG, log_msg);

    int i;
    int fwd = (color == CB_WHITE) ? 1 : -1;
    int dx[] = {-1, 1, -1, 1};
    int temp_dy[4];
    temp_dy[0] = fwd;
    temp_dy[1] = fwd;
    temp_dy[2] = -fwd;
    temp_dy[3] = -fwd;
    int* dy = temp_dy;

    int start_dir = 0;
    int end_dir = is_king ? 4 : 2;

    bool found_another_jump = false;

    // Safety check: if recursion depth exceeds the maximum allowed jumps, stop.
    if (d >= 11) { // If d is 11, it's the last valid index for del[11], path[11] is valid for storage.
        return;
    }
    
    int visited[33];
    memcpy(visited, visited_parent, sizeof(visited));

    int current_square_num = coorstonumber(x, y, GT_ENGLISH);
    if (current_square_num == 0) {
        log_c(LOG_LEVEL_ERROR, "find_captures_recursive: current_square_num is 0!");
        return;
    }
    int current_square_bit_bitboard_pos = current_square_num - 1;

    if(visited[current_square_num])
        return;
    visited[current_square_num] = 1;


    for (i = start_dir; i < end_dir; ++i) {
        int jump_x = x + dx[i];
        int jump_y = y + dy[i];
        int land_x = x + 2 * dx[i];
        int land_y = y + 2 * dy[i];

        if (jump_x >= 0 && jump_x < 8 && jump_y >= 0 && jump_y < 8 && land_x >= 0 && land_x < 8 && land_y >= 0 && land_y < 8) {
            int jump_square_num = coorstonumber(jump_x, jump_y, GT_ENGLISH);
            int land_square_num = coorstonumber(land_x, land_y, GT_ENGLISH);

            if (jump_square_num == 0 || land_square_num == 0) continue; // Invalid squares

            int opponent_piece = get_piece(board, jump_square_num - 1);
            int landing_square_piece = get_piece(board, land_square_num - 1);
            
            bool is_opponent = (opponent_piece != CB_EMPTY) && ((opponent_piece & color) == 0);

            if (is_opponent && landing_square_piece == CB_EMPTY) {
                found_another_jump = true;

                CBmove mm = m;
                mm.to.x = land_x;
                mm.to.y = land_y;
                mm.path[d + 1].x = land_x;
                mm.path[d + 1].y = land_y;
                mm.del[d].x = jump_x;
                mm.del[d].y = jump_y;
                mm.delpiece[d] = opponent_piece;
                if (d < 11) {
                    mm.del[d + 1].x = -1;
                }
                mm.jumps = d + 1;

                int becomes_king = (!is_king) && ((color == CB_WHITE && land_y == 7) || (color == CB_BLACK && land_y == 0));
                int next_is_king = is_king || becomes_king;
                mm.newpiece = next_is_king ? (m.oldpiece | CB_KING) : m.oldpiece;

                if (becomes_king) {
                    // This jump makes a king. This is the last move in the sequence. Add it.
                    if (*n < MAXMOVES) {
                        movelist[*n] = mm;
                        (*n)++;
                    } else {
                        // qWarning() << "MAXMOVES exceeded in find_captures_recursive (becomes_king)"; // Re-enable custom logging first
                    }
                } else {
                    // Continue searching for more jumps in the sequence
                    bitboard_pos next_board = *board;
                    clear_square(&next_board, jump_square_num - 1);
                    find_captures_recursive(&next_board, movelist, mm, land_x, land_y, d + 1, n, color, next_is_king, visited);
                }
            }
        }
    }

    if (!found_another_jump && d > 0) {
        // char log_msg[512];
        // sprintf(log_msg, "find_captures_recursive: adding move, from: (%d,%d), to: (%d,%d), jumps: %d", m.from.x, m.from.y, m.to.x, m.to.y, m.jumps);
        // log_c(LOG_LEVEL_DEBUG, log_msg);
        if (*n < MAXMOVES) {
            movelist[*n] = m;
            (*n)++;
        } else {
            // qWarning() << "MAXMOVES exceeded in find_captures_recursive (not found_another_jump)"; // Re-enable custom logging first
        }
    }
}


void makemovelist(const bitboard_pos* board, int color, CBmove movelist[MAXMOVES], int *isjump, int *n)
{
    int i;
    CBmove m = {0};
    *isjump = 0;

    for (i = 0; i < MAXMOVES; i++) {
        movelist[i].jumps = 0;
        movelist[i].is_capture = false;
        movelist[i].comment[0] = '\0'; // Initialize comment
    }
    *n = 0;

    // --- 1. Find all bitboard_possible captures first ---
    CBmove captures[MAXMOVES];
    int ncaptures = 0;
    int visited[33] = {0};
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int square_num = coorstonumber(c, r, GT_ENGLISH);
            if (square_num == 0) continue; // Skip light squares

            int piece = get_piece(board, square_num - 1); // Get piece from bitboard
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
        *isjump = 1;
        int max_jumps = 0;
        for (i = 0; i < ncaptures; ++i) {
            if (captures[i].jumps > max_jumps) {
                max_jumps = captures[i].jumps;
            }
        }

        for (i = 0; i < ncaptures; ++i) {
            if (captures[i].jumps == max_jumps) {
                if (*n < MAXMOVES) {
                    movelist[*n] = captures[i];
                    (*n)++;
                }
            }
        }
        for(i=0; i<*n; ++i) movelist[i].is_capture = true;
        return;
    }

    // --- 2. If no captures, find all regular moves ---
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int square_num = coorstonumber(c, r, GT_ENGLISH);
            if (square_num == 0) continue; // Skip light squares

            int piece = get_piece(board, square_num - 1); // Get piece from bitboard
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            int is_king = (piece & CB_KING) ? 1 : 0;

            if (piece_color == color) {
                int fwd = (color == CB_WHITE) ? 1 : -1; 
                int dx[] = {-1, 1};
                
                if (!is_king) {
                    for (i = 0; i < 2; ++i) {
                        int to_c = c + dx[i];
                        int to_r = r + fwd;
                        int to_square_num = coorstonumber(to_c, to_r, GT_ENGLISH);

                        if (to_c >= 0 && to_c < 8 && to_r >= 0 && to_r < 8 && to_square_num != 0 && get_piece(board, to_square_num - 1) == CB_EMPTY) {
                            if (*n < MAXMOVES) {
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
                }
                else { // is_king
                    int king_dy[] = {1, 1, -1, -1};
                    int king_dx[] = {-1, 1, -1, 1};
                    for (i = 0; i < 4; ++i) {
                        int to_c = c + king_dx[i];
                        int to_r = r + king_dy[i];
                        int to_square_num = coorstonumber(to_c, to_r, GT_ENGLISH);

                        if (to_c >= 0 && to_c < 8 && to_r >= 0 && to_r < 8 && to_square_num != 0 && get_piece(board, to_square_num - 1) == CB_EMPTY) {
                            if (*n < MAXMOVES) {
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
                                movelist[*n].newpiece = piece;
                                (*n)++;
                            }
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

void start3move_c(bitboard_pos* board, int opening_index) {
    newgame(board); // Start with a fresh bitboard

    if (opening_index >= 0 && opening_index < 156) {
        int current_color = CB_BLACK; // 3-move openings typically start with Black

        for (int i = 0; i < 3; ++i) {
            int from_square_num = three_moves_data[opening_index][i].from;
            int to_square_num = three_moves_data[opening_index][i].to;

            int from_x, from_y, to_x, to_y;
            numbertocoors(from_square_num, &from_x, &from_y, GT_ENGLISH);
            numbertocoors(to_square_num, &to_x, &to_y, GT_ENGLISH);

            CBmove move;
            move.from.x = from_x;
            move.from.y = from_y;
            move.to.x = to_x;
            move.to.y = to_y;
            move.jumps = 0; // Assuming no jumps in opening moves for simplicity

            int piece = get_piece(board, from_square_num - 1); // Get piece from bitboard
            move.oldpiece = piece;
            // Determine newpiece, accounting for bitboard_possible kinging
            int new_piece_val = piece;
            if ((piece & CB_MAN) && ((current_color == CB_WHITE && to_y == 7) || (current_color == CB_BLACK && to_y == 0))) {
                new_piece_val |= CB_KING;
            }
            move.newpiece = new_piece_val;

            domove_c(&move, board);

            // Toggle color for the next move
            current_color = (current_color == CB_BLACK) ? CB_WHITE : CB_BLACK;
        }
    }
}

int get_piece(const bitboard_pos* board, int square_num) {
    unsigned int bit = (1U << square_num);
    if (board->bm & bit) return CB_BLACK | CB_MAN;
    if (board->bk & bit) return CB_BLACK | CB_KING;
    if (board->wm & bit) return CB_WHITE | CB_MAN;
    if (board->wk & bit) return CB_WHITE | CB_KING;
    return CB_EMPTY;
}

void set_piece(bitboard_pos* board, int square_num, int piece) {
    unsigned int bit = (1U << square_num);
    board->bm &= ~bit; // Clear existing piece
    board->bk &= ~bit;
    board->wm &= ~bit;
    board->wk &= ~bit;

    if (piece & CB_BLACK) {
        if (piece & CB_KING) board->bk |= bit;
        else board->bm |= bit;
    } else if (piece & CB_WHITE) {
        if (piece & CB_KING) board->wk |= bit;
        else board->wm |= bit;
    }
}

void clear_square(bitboard_pos* board, int square_num) {
    unsigned int bit = (1U << square_num);
    board->bm &= ~bit;
    board->bk &= ~bit;
    board->wm &= ~bit;
    board->wk &= ~bit;
}

void domove_c(const CBmove *move, bitboard_pos* board)
{
    int from_square = coorstonumber(move->from.x, move->from.y, GT_ENGLISH) - 1; // 0-indexed
    int to_square = coorstonumber(move->to.x, move->to.y, GT_ENGLISH) - 1;   // 0-indexed

    // 1. Move the piece from `from_square` and place its `newpiece` type at `to_square`
    clear_square(board, from_square);
    set_piece(board, to_square, move->newpiece); // Use newpiece which already accounts for kinging

    // 2. Remove captured pieces
    if (move->jumps > 0) {
        for (int i = 0; i < move->jumps; i++) {
            int del_square = coorstonumber(move->del[i].x, move->del[i].y, GT_ENGLISH) - 1;
            clear_square(board, del_square);
        }
    }
}

void unmake_move_c(const CBmove *move, bitboard_pos* board) {
    int from_square = coorstonumber(move->from.x, move->from.y, GT_ENGLISH) - 1;
    int to_square = coorstonumber(move->to.x, move->to.y, GT_ENGLISH) - 1;

    // 1. Move the piece back from 'to' to 'from'
    clear_square(board, to_square); // Clear the destination square
    set_piece(board, from_square, move->oldpiece); // Restore original piece at source

    // 2. Restore captured pieces
    if (move->jumps > 0) {
        for (int i = 0; i < move->jumps; ++i) {
            int del_square = coorstonumber(move->del[i].x, move->del[i].y, GT_ENGLISH) - 1;
            set_piece(board, del_square, move->delpiece[i]);
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
    return DBManager::instance()->db_init(64, path_buffer, egdb_path); // Pass egdb_path as the third argument
}

// Implementation for Kingsrow EGDB lookup
int egdb_wrapper_lookup(bitboard_pos* p, int side_to_move) {
    char log_msg[256];
    sprintf(log_msg, "EGDB Wrapper: Lookup for bitboard_position: bm=%u, bk=%u, wm=%u, wk=%u, side_to_move=%d", p->bm, p->bk, p->wm, p->wk, side_to_move);
    log_c(LOG_LEVEL_DEBUG, log_msg);
    // Convert our 'bitboard_pos' struct to the EGDB's 'POSITION' struct
    bitboard_pos egdb_bitboard_pos;
    egdb_bitboard_pos.bm = p->bm;
    egdb_bitboard_pos.bk = p->bk;
    egdb_bitboard_pos.wm = p->wm;
    egdb_bitboard_pos.wk = p->wk;

    // Call the actual Kingsrow EGDB lookup function
    int result = DBManager::instance()->dblookup(&egdb_bitboard_pos, side_to_move);
    // If dblookup returns DB_NOT_LOOKED_UP, it means the bitboard_position was not in cache during conditional lookup.
    // We don't log this as an error, but rather as debug info.
    log_c(LOG_LEVEL_DEBUG, log_msg);
    return result;
}



char* read_text_file_c(const char* filename, READ_TEXT_FILE_ERROR_TYPE* etype) {
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
