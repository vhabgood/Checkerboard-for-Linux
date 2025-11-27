#pragma once

// Basic C Standard Library Includes
#include <stdbool.h>
#include <stdint.h>
#include <time.h>


// --- C-Compatible Type Definitions and Macros --- //

// Coordinate struct
typedef struct {
    int x, y;
} coor;

// Board representation (8x8 array)
typedef struct {
    int board[8][8];
} Board8x8;

// Bitboard position (for internal engine use)
typedef struct {
    unsigned int bm; // Black men
    unsigned int bk; // Black kings
    unsigned int wm; // White men
    unsigned int wk; // White kings
    int color; // Side to move
} pos;



// Define int32 for compatibility (used in dblookup.c)
typedef int int32;

// Chess move representation
typedef struct {
    coor from;
    coor to;
    coor path[12];
    coor del[12];
    int delpiece[12];
    int jumps;
    int oldpiece; 
    int newpiece; 
    bool is_capture;
} CBmove;

// Generic
#define CB_EMPTY 0         // Represents an empty square
#define CB_UNKNOWN 0       // General unknown value
#define MAXMOVES 256       // Max moves in a move list
#define MAX_PATH_FIXED 512 // Max path length for files

#ifdef __cplusplus
extern "C" {
#endif
    void log_c(int level, const char* message);
    void boardtobitboard(const Board8x8* board8x8, pos* position); // Added this line
    int get_legal_moves_c(const Board8x8* board, int color, CBmove movelist[MAXMOVES], int *nmoves, int *isjump, const CBmove *last_move, bool *can_continue_multijump);
#ifdef __cplusplus
}
#endif

#define MAXUSERBOOK 1024 // Define MAXUSERBOOK here, as it's specific to userbook

typedef struct userbookentry {
	pos position;
	CBmove move;
} userbookentry;

// PDN Move data
typedef struct {
    int from_square;
    int to_square;
    char comment[256];
} PDNmove;

// PDN Game structure (C-compatible)
typedef struct {
    char event[256];
    char site[256];
    char date[256];
    char round[256];
    char white[256];
    char black[256];
    char resultstring[16];
    int result; // Using int for C-compatible storage of PDN_RESULT_ENUM
    char FEN[256];
    int gametype;
    int num_moves;
    int movesindex;
    PDNmove *moves; // Pointer to dynamically allocated moves (managed by C++ wrapper)
}
PDNgame;

// Engine match statistics
typedef struct emstats_t {
    long engine1_wins;
    long engine2_wins;
    long draws;
    long total_games;
} emstats_t;

// --- C-Compatible Enums --- //

enum PDN_RESULT {
    PDN_RESULT_UNKNOWN,
    PDN_RESULT_WIN,
    PDN_RESULT_LOSS,
    PDN_RESULT_DRAW,
    PDN_RESULT_UNAVAILABLE
};

enum Language {
    LANG_ENGLISH,
    LANG_ESPANOL,
    LANG_ITALIANO,
    LANG_DEUTSCH,
    LANG_FRANCAIS
};

enum AppState {
    STATE_IDLE,
    STATE_PLAYING,
    STATE_ANALYSIS,
    STATE_ENGINE_MATCH,
    STATE_BOOKADD,
    STATE_GAMEOVER,
    STATE_NORMAL,
    STATE_SETUP,
    STATE_ENGINE_THINKING,
    STATE_ANALYZEGAME,
    STATE_AUTOPLAY,
    STATE_2PLAYER,
    STATE_RUNTESTSET,
    STATE_ANALYZEPDN,
    STATE_BOOKDELETE,
    STATE_BOOKVIEW
};


// Log Levels (C-compatible)
enum LogLevel {
    LOG_LEVEL_TRACE = -1,
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4
};

enum GuiAction {
    GUI_UPDATE_BOARD,
    GUI_SHOW_MESSAGE,
    GUI_ENABLE_CONTROLS,
    GUI_DISABLE_CONTROLS
};

enum PDN_PARSE_STATE {
    PDN_IDLE,
    PDN_READING_FROM,
    PDN_WAITING_SEP,
    PDN_WAITING_TO,
    PDN_READING_TO,
    PDN_WAITING_OPTIONAL_SEP,
    PDN_WAITING_OPTIONAL_TO,
    PDN_CURLY_COMMENT,
    PDN_NEMESIS_COMMENT,
    PDN_FLUFF,
    PDN_DONE
};

enum READ_TEXT_FILE_ERROR_TYPE {
    READ_TEXT_FILE_NO_ERROR,
    READ_TEXT_FILE_DOES_NOT_EXIST,
    READ_TEXT_FILE_COULD_NOT_OPEN,
    READ_TEXT_FILE_OTHER_ERROR
};

// Piece types (bit flags)
#define CB_BLACK 8
#define CB_WHITE 16
#define CB_MAN 1
#define CB_KING 2

// Game results
#define CB_WIN 1
#define CB_LOSS 2
#define CB_DRAW 3

// Game types
#define GT_ENGLISH 0
#define GT_AMERICAN 0
#define GT_ITALIAN 1
#define GT_SPANISH 2
#define GT_RUSSIAN 3
#define GT_CZECH 4

// Time control levels
#define LEVEL_INSTANT 0
#define LEVEL_1S 1
#define LEVEL_01S 12
#define LEVEL_02S 13
#define LEVEL_05S 14
#define LEVEL_2S 2
#define LEVEL_5S 3
#define LEVEL_10S 4
#define LEVEL_15S 5
#define LEVEL_30S 6
#define LEVEL_1M 7
#define LEVEL_2M 8
#define LEVEL_5M 9
#define LEVEL_15M 10
#define LEVEL_30M 11
#define LEVEL_INFINITE 99

// EGDB Score values
#define EGDB_WIN_SCORE 1000000
#define EGDB_LOSS_SCORE -1000000
#define EGDB_DRAW_SCORE 0

// EGDB related macros (from dblookup.h)
#define PRELOAD // preload (parts) of db in cache?
#define AUTOLOADSIZE 0
#define DB_BLACK 0
#define DB_WHITE 1
#define DB_UNKNOWN 0
#define DB_WIN 1
#define DB_LOSS 2
#define DB_DRAW 3
#define DB_NOT_LOOKED_UP 4
#define SPLITSIZE 8
#define BLOCKNUM4 250
#define BLOCKNUM5 2000
#define BLOCKNUM6 42000
#define BLOCKNUM7 400000
#define BLOCKNUM8 4600000
#define MAXIDX4 100
#define MAXIDX5 500
#define MAXIDX6 1000
#define MAXIDX7 31000
#define MAXIDX8 44000
#define MAXPIECE 4
#define MINCACHESIZE 65536

#define ENGINECOMMAND_REPLY_SIZE 2048
#define MOVESPLAY 1

// --- AI Evaluation Constants ---
#define MAN_VALUE 100
#define KING_VALUE 133
#define PASSED_MAN_BONUS 15
#define BRIDGE_BONUS 20
#define MOBILITY_MULTIPLIER 10
#define THREAT_PENALTY_MULTIPLIER 5
#define CENTER_BONUS 5
// --- End AI Evaluation Constants ---

// Function pointer types for engine interaction
typedef int (*CB_GETMOVE)(Board8x8 board, int color, double maxtime, char statusBuffer[1024], int *playnow, int info, int moreinfo, CBmove *bestMove);
typedef int (*CB_ISLEGAL)(Board8x8 board8, int color, int from, int to, int gametype, CBmove *move);
typedef int (*CB_ENGINECOMMAND)(const char *command, char reply[ENGINECOMMAND_REPLY_SIZE]);
typedef int (*CB_GETSTRING)(char Lstr[MAX_PATH_FIXED]); // Using MAX_PATH_FIXED for consistency
typedef int (*CB_GETGAMETYPE)(void);