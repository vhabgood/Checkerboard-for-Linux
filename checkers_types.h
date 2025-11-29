#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h> // For memset, memcpy etc.

#include <QList>
#include <QColor>
#include <QAtomicInt>
#include <QString>
#include <QMetaType>

#include "ai_state.h"

// --- C-Compatible Type Definitions --- //

// Coordinate struct
typedef struct {
    int x, y;
} coor;

// Board representation (8x8 array)
typedef struct {
    int board[8][8];
} Board8x8;
Q_DECLARE_METATYPE(Board8x8)

// Bitboard position (for internal engine use and EGDB)
typedef struct {
    unsigned int bm; // Black men
    unsigned int bk; // Black kings
    unsigned int wm; // White men
    unsigned int wk; // White kings
    int color; // Side to move for EGDB, usually DB_BLACK or DB_WHITE
} pos;

// Define int32 for compatibility (used in dblookup.cpp)
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
    char comment[256];
} CBmove;
Q_DECLARE_METATYPE(CBmove)

// User book entry
typedef struct userbookentry {
	pos position;
	CBmove move;
} userbookentry;

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
} PDNgame;

// Engine match statistics
typedef struct emstats_t {
    long engine1_wins;
    long engine2_wins;
    long draws;
    long total_games;
} emstats_t;

// definition of a structure for compressed databases (from dblookup.h)
typedef struct compresseddatabase
    {
    int ispresent;			// does db exist?
    int numberofblocks;		// how many disk blocks does this db need?
    int blockoffset;		// offset to calculate unique block id for the blocks of this db
    int firstblock;			// where is the first block in it's db?
    int startbyte;			// at this byte
    int databasesize;		// index range for this database
    int value;				// WIN/LOSS/DRAW if single value, UNKNOWN == 0 else
    int *idx;				// pointer to an array of index numbers, array is 
                            // allocated dynamically, to size n
    int fp;					// which file is it in?
    } cprsubdb;

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

// Define PlayerType enum for configuring AI vs. Human or AI vs. AI
enum PlayerType {
    PLAYER_HUMAN,
    PLAYER_AI
};


// --- C-Compatible Macros --- //

// Generic
#define CB_EMPTY 0         // Represents an empty square
#define CB_UNKNOWN 0       // General unknown value
#define MAXMOVES 256       // Max moves in a move list
#define MAX_PATH_FIXED 512 // Max path length for files (increased from 260)
#define MAXUSERBOOK 1024   // Max entries in user book

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
#define GT_3MOVE 5

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

// EGDB related macros
#define PRELOAD
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
#define MAXFP 50
#define SKIPS 58
#define MAXSKIP 10000

// --- Program Status Word Flags (32-bit hexadecimal) ---

// Application & Initialization (Bits 0-7)
#define STATUS_APP_START                    (1U << 0)   // 0x00000001
#define STATUS_BOARD_INIT_OK                (1U << 1)   // 0x00000002
#define STATUS_EGDB_INIT_START              (1U << 2)   // 0x00000004
#define STATUS_EGDB_INIT_OK                 (1U << 3)   // 0x00000008
#define STATUS_EGDB_INIT_FAIL               (1U << 4)   // 0x00000010
#define STATUS_ENGINE_LOAD_OK               (1U << 5)   // 0x00000020
#define STATUS_ENGINE_LOAD_FAIL             (1U << 6)   // 0x00000040
#define STATUS_NEW_GAME_OK                  (1U << 7)   // 0x00000080
#define STATUS_GAME_LOAD_PDN_OK             (1U << 8)   // 0x00000100
#define STATUS_GAME_SAVE_PDN_OK             (1U << 9)   // 0x00000200

// Game State (Bits 8-15)
#define STATUS_GAMEMANAGER_INIT_START       (1U << 10)  // 0x00000400
#define STATUS_TURN_BIT_POS                 11          // Position for turn field
#define STATUS_TURN_MASK                    (1U << STATUS_TURN_BIT_POS) // Mask for turn field
#define STATUS_TURN_BLACK                   (0U << STATUS_TURN_BIT_POS)
#define STATUS_TURN_WHITE                   (1U << STATUS_TURN_BIT_POS)

#define STATUS_GAMETYPE_BIT_POS             12          // Position for game type field
#define STATUS_GAMETYPE_MASK                (1U << STATUS_GAMETYPE_BIT_POS) // Mask for game type field
#define STATUS_GAMETYPE_NORMAL              (0U << STATUS_GAMETYPE_BIT_POS)
#define STATUS_GAMETYPE_3MOVE               (1U << STATUS_GAMETYPE_BIT_POS)

#define STATUS_WHITE_PLAYER_BIT_POS         13          // Position for white player type field (2 bits)
#define STATUS_WHITE_PLAYER_MASK            (3U << STATUS_WHITE_PLAYER_BIT_POS) // Mask for 2 bits
#define STATUS_WHITE_PLAYER_NONE            (0U << STATUS_WHITE_PLAYER_BIT_POS)
#define STATUS_WHITE_PLAYER_HUMAN           (1U << STATUS_WHITE_PLAYER_BIT_POS)
#define STATUS_WHITE_PLAYER_AI              (2U << STATUS_WHITE_PLAYER_BIT_POS)

#define STATUS_BLACK_PLAYER_BIT_POS         15          // Position for black player type field (2 bits)
#define STATUS_BLACK_PLAYER_MASK            (3U << STATUS_BLACK_PLAYER_BIT_POS) // Mask for 2 bits
#define STATUS_BLACK_PLAYER_NONE            (0U << STATUS_BLACK_PLAYER_BIT_POS)
#define STATUS_BLACK_PLAYER_HUMAN           (1U << STATUS_BLACK_PLAYER_BIT_POS)
#define STATUS_BLACK_PLAYER_AI              (2U << STATUS_BLACK_PLAYER_BIT_POS)

// AI & Time Control (Bits 17-23)
#define STATUS_AI_TIME_SETTING_BIT_POS      17          // Position for AI time setting field (4 bits)
#define STATUS_AI_TIME_SETTING_MASK         (0xFU << STATUS_AI_TIME_SETTING_BIT_POS) // Mask for 4 bits
// Values for this field would correspond to LEVEL_ constants (0-15)

// Events & Errors (Bits 24-31)
#define STATUS_EGDB_LOOKUP_HIT              (1U << 24)  // 0x01000000
#define STATUS_EGDB_LOOKUP_MISS             (1U << 25)  // 0x02000000
#define STATUS_ENGINE_MOVE_RECEIVED         (1U << 26)  // 0x04000000
#define STATUS_INVALID_MOVE                 (1U << 28)  // 0x10000000
#define STATUS_FILE_IO_ERROR                (1U << 29)  // 0x20000000
#define STATUS_CRITICAL_ERROR               (1U << 31)  // 0x80000000

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

// --- C++ specific options struct ---
typedef QList<int> Squarelist;

struct CBoptions {
    bool sound;
    bool highlight;
    bool invert_board;
    bool show_coordinates;
    double time_per_move;
    int gametype;
    QString engine_path;
    QString secondary_engine_path;
    QString engine_name;
    QString engine_options;
    QString book_path;
    char userdirectory[MAX_PATH_FIXED];
    char matchdirectory[MAX_PATH_FIXED];
    char EGTBdirectory[MAX_PATH_FIXED];
    int level;
    bool exact_time;
    bool use_incremental_time;
    int initial_time;
    int time_increment;
    bool invert;
    bool numbers;
    bool mirror;
    bool userbook;
    bool op_crossboard;
    bool op_mailplay;
    bool op_barred;
    int priority;
    Language language;
    int current_engine;
    QString piece_set;
    int three_move_option;
    LogLevel min_log_level;
    bool enable_game_timer;
    int white_player_type;
    int black_player_type;

    // Default constructor
    CBoptions() :
        sound(true),
        highlight(true),
        invert_board(false),
        show_coordinates(true),
        time_per_move(2.0),
        gametype(GT_ENGLISH),
        engine_path(""),
        secondary_engine_path(""),
        engine_name(""),
        engine_options(""),
        book_path(""),
        level(LEVEL_5S),
        exact_time(false),
        use_incremental_time(false),
        initial_time(0),
        time_increment(0),
        invert(false),
        numbers(false),
        mirror(false),
        userbook(false),
        op_crossboard(false),
        op_mailplay(false),
        op_barred(false),
        priority(0),
        language(LANG_ENGLISH),
        current_engine(0),
        piece_set("standard"),
        three_move_option(0),
        min_log_level(LOG_LEVEL_INFO),
        enable_game_timer(false),
        white_player_type(PLAYER_HUMAN),
        black_player_type(PLAYER_HUMAN)
    {
        memset(userdirectory, 0, sizeof(userdirectory));
        memset(matchdirectory, 0, sizeof(matchdirectory));
        memset(EGTBdirectory, 0, sizeof(EGTBdirectory));
    }
};
Q_DECLARE_METATYPE(CBoptions)

// Global program status word
extern uint32_t g_programStatusWord;

// C++ wrapper for PDNgame
struct PdnGameWrapper {
    PDNgame game; // The underlying C-style PDNgame struct
    QList<CBmove> moves; // C++ friendly storage for moves

    // Constructor to initialize
    PdnGameWrapper() {
        memset(&game, 0, sizeof(PDNgame)); // Initialize C-struct to zeros
    }

    // Default copy/assignment/destructor are sufficient now
    PdnGameWrapper(const PdnGameWrapper& other) = default;
    PdnGameWrapper& operator=(const PdnGameWrapper& other) = default;
    ~PdnGameWrapper() = default;
};
Q_DECLARE_METATYPE(PdnGameWrapper)


// --- Function Pointer Types (C-compatible) ---
typedef int (*CB_GETMOVE)(Board8x8 board, int color, double maxtime, char statusBuffer[1024], int *playnow, int info, int moreinfo, CBmove *bestMove);
typedef int (*CB_ISLEGAL)(Board8x8 board8, int color, int from, int to, int gametype, CBmove *move);
typedef int (*CB_ENGINECOMMAND)(const char *command, char reply[ENGINECOMMAND_REPLY_SIZE]);
typedef int (*CB_GETSTRING)(char Lstr[MAX_PATH_FIXED]);
typedef int (*CB_GETGAMETYPE)(void);

// For c_logic.cpp: LSB, MSB, revert helpers
#define hiword(x) (((x)&0xFFFF0000)>>16)
#define loword(x) ((x)&0xFFFF)

#ifdef __cplusplus
extern "C" {
#endif
    // Function prototypes for C functions
    void log_c(int level, const char* message);
    void boardtobitboard(const Board8x8* board8x8, pos* position);
    int get_legal_moves_c(const Board8x8* board, int color, CBmove movelist[MAXMOVES], int *nmoves, int *isjump, const CBmove *last_move, bool *can_continue_multijump);
    int LSB(int32 x);
    int MSB(int32 x);
    int revert(int32 n);
    int recbitcount(int32 n);
        int preload(char out[256], FILE *db_fp[], int fp_count);
    int db_getcachesize(void);
    void db_infostring(char *str);
    int64_t getdatabasesize(int bm, int bk, int wm, int wk, int bmrank, int wmrank);
#ifdef __cplusplus
}
#endif