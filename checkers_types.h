#ifndef CHECKERS_TYPES_H
#define CHECKERS_TYPES_H

#include "core_types.h"
#ifdef __cplusplus
#include <QString>
#include <QVariant>
#endif


// Define constants for piece types
#define CB_MAN			1
#define CB_KING			2
#define CB_WHITE		4
#define CB_BLACK		8
#define CB_EMPTY		16
#define CB_REMOVED		32

// Define constants for game types
#define GT_ENGLISH		1
#define GT_ITALIAN		2
#define GT_SPANISH		3
#define GT_GERMAN		4
#define GT_CZECH		5
#define GT_POOLS		6
#define GT_RUSSIAN		7
#define GT_BRAZILIAN	8

// Define constants for game results
#define CB_WIN			1
#define CB_LOSS			2
#define CB_DRAW			3
#define CB_UNKNOWN		4

// Enum for PDN game results
enum PDN_RESULT {
    PDN_RESULT_UNKNOWN = 0,
    PDN_RESULT_WIN,
    PDN_RESULT_LOSS,
    PDN_RESULT_DRAW
};

#define MAXMOVES        28
#define MAX_MOVES_PDN 1024


struct CBmove // Changed from typedef struct to struct
{
    coor from;
    coor to;
    int jumps;
    int captures[MAXMOVES];
    bool is_capture;
    // Missing members from c_logic.cpp errors
    coor path[MAXMOVES]; // Stores the path of a jump sequence
    coor del[MAXMOVES];  // Stores the squares of captured pieces
    int delpiece[MAXMOVES]; // Stores the types of captured pieces
    int oldpiece;       // The piece before the move
    int newpiece;       // The piece after the move (e.g., kinged)
    char comment[256];  // Comment associated with the move


}; // Removed typedef here, CBmove is now a proper struct
Q_DECLARE_METATYPE(CBmove)

// Enum for application states
enum AppState {
    STATE_IDLE,
    STATE_PLAYING,
    STATE_BOOKADD,
    STATE_GAMEOVER,
    STATE_NORMAL,
    STATE_SETUP,
    STATE_ENGINE_THINKING,
    STATE_2PLAYER,
    STATE_BOOKDELETE,
    STATE_ANALYSIS,
    STATE_ANALYZEGAME,
    STATE_ENGINE_MATCH,
    STATE_AUTOPLAY,
    STATE_RUNTESTSET,
    STATE_ANALYZEPDN,
    STATE_BOOKVIEW
};

enum AI_State {
    Idle,
    AnalyzeGame,
    EngineMatch,
    Autoplay,
    RunTestSet,
    AnalyzePdn
};
#ifdef __cplusplus
Q_DECLARE_METATYPE(AI_State)
#endif

// Add this to your checkers_types.h or a relevant header
typedef struct {
    int wins;
    int losses;
    int draws;
    int unfinished;
} emstats_t;

typedef enum {
    READ_TEXT_FILE_NO_ERROR,
    READ_TEXT_FILE_NOT_FOUND,
    READ_TEXT_FILE_EMPTY,
    READ_TEXT_FILE_OTHER_ERROR,
    READ_TEXT_FILE_COULD_NOT_OPEN,
    READ_TEXT_FILE_DOES_NOT_EXIST = READ_TEXT_FILE_NOT_FOUND // Alias
} READ_TEXT_FILE_ERROR_TYPE;

// --- New Enums and Structs to be added ---

// Enum for Player Type
enum PlayerType {
    PLAYER_HUMAN,
    PLAYER_AI
};

// Enum for Languages (placeholder)
enum Language {
    LANG_ENGLISH,
    LANG_ESPANOL,
    LANG_ITALIANO,
    LANG_DEUTSCH,
    LANG_FRANCAIS
};

// Enum for Time Control Levels
enum TimeLevel {
    LEVEL_EXACT, LEVEL_INSTANT, LEVEL_01S, LEVEL_02S, LEVEL_05S,
    LEVEL_1S, LEVEL_2S, LEVEL_5S, LEVEL_10S, LEVEL_15S, LEVEL_30S,
    LEVEL_1M, LEVEL_2M, LEVEL_5M, LEVEL_15M, LEVEL_30M, LEVEL_INFINITE
};

// Struct to hold game options
struct CBoptions {
    bool sound;
    bool highlight;
    bool invert_board;
    bool show_coordinates;
    double time_per_move;
    QString engine_path;
    QString secondary_engine_path;
    QString engine_name;
    QString engine_options;
    QString book_path;
    Language language;
    int current_engine;
    QString piece_set;
    int priority;
    int three_move_option;
    bool mirror;
    bool enable_game_timer;
    int initial_time;
    int time_increment;
    PlayerType white_player_type;
    PlayerType black_player_type;
    char userdirectory[MAX_PATH_FIXED];
    char matchdirectory[MAX_PATH_FIXED];
    char EGTBdirectory[MAX_PATH_FIXED];
    int gametype;
};

// PDN Game struct
struct PDNgame {
    char event[64];
    char site[64];
    char date[64];
    char round[32];
    char black[64];
    char white[64];
    char result[16];
    char FEN[200];
    int gametype;
    int nmoves;
    CBmove moves[MAX_MOVES_PDN];
};

// A wrapper for PDN game data to be used with QVariant
struct PdnGameWrapper {
    PDNgame pdn;
};

Q_DECLARE_METATYPE(PdnGameWrapper)

// AI Evaluation constants
#define KING_VALUE 300
#define MAN_VALUE 100
#define MOBILITY_MULTIPLIER 5

#endif // CHECKERS_TYPES_H
