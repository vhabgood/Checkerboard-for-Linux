#pragma once

// Include the C-compatible types


// C++ Standard Library Includes
#include <string>
#include <vector>

// Qt Specific Includes
#include <QList>
#include <QColor>
#include <QAtomicInt>
#include <QString>
#include <QMetaType> // Required for Q_DECLARE_METATYPE

// --- Q_DECLARE_METATYPE for C-style structs used in Qt signals/slots --- //
Q_DECLARE_METATYPE(coor)
Q_DECLARE_METATYPE(Board8x8)
Q_DECLARE_METATYPE(pos)
Q_DECLARE_METATYPE(CBmove)
Q_DECLARE_METATYPE(PDNgame)
typedef QList<int> Squarelist;

// --- C++ Specific Type Definitions --- //

extern LogLevel s_minLogLevel;

// Define PlayerType enum for configuring AI vs. Human or AI vs. AI
enum PlayerType {
    PLAYER_HUMAN,
    PLAYER_AI
};
Q_DECLARE_METATYPE(PlayerType)

// C++ specific options struct (using QString for paths)
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

// C++ wrapper for PDNgame
struct PdnGameWrapper {
    PDNgame game; // The underlying C-style PDNgame struct
    QList<PDNmove> moves; // C++ friendly storage for moves

    // Constructor to initialize from a PDNgame
    PdnGameWrapper() {
        memset(&game, 0, sizeof(PDNgame)); // Initialize C-struct to zeros
        game.moves = nullptr; // Ensure the pointer is null
    }

    // Copy constructor
    PdnGameWrapper(const PdnGameWrapper& other) : game(other.game), moves(other.moves) {
        // Deep copy the C-style moves array if it exists
        if (other.game.moves && other.game.num_moves > 0) {
            game.moves = new PDNmove[other.game.num_moves];
            memcpy(game.moves, other.game.moves, other.game.num_moves * sizeof(PDNmove));
        } else {
            game.moves = nullptr;
        }
    }

    // Assignment operator
    PdnGameWrapper& operator=(const PdnGameWrapper& other) {
        if (this != &other) {
            // Clean up existing moves if any
            if (game.moves) {
                delete[] game.moves;
            }

            game = other.game;
            moves = other.moves;

            // Deep copy the C-style moves array if it exists
            if (other.game.moves && other.game.num_moves > 0) {
                game.moves = new PDNmove[other.game.num_moves];
                memcpy(game.moves, other.game.moves, other.game.num_moves * sizeof(PDNmove));
            } else {
                game.moves = nullptr;
            }
        }
        return *this;
    }

    // Destructor
    ~PdnGameWrapper() {
        if (game.moves) {
            delete[] game.moves;
            game.moves = nullptr;
        }
    }
};
Q_DECLARE_METATYPE(PdnGameWrapper)