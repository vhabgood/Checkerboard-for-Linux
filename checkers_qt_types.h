#pragma once

// Include the C-compatible types
#include "checkers_c_types.h"

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
Q_DECLARE_METATYPE(POSITION)
Q_DECLARE_METATYPE(CBmove)
Q_DECLARE_METATYPE(PDNgame)
typedef QList<int> Squarelist;

// --- C++ Specific Type Definitions --- //

// Define LogLevel enum for controlling logging verbosity
enum LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    Fatal
};
Q_DECLARE_METATYPE(LogLevel)

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
    double time_per_move = 5.0; // Added for AI time control, initialized to 5.0 seconds
    int gametype;
    QString engine_path;
    QString secondary_engine_path; // New member for secondary engine path
    QString engine_name; // New member for engine name
    QString engine_options; // New member for engine options
    QString book_path;
    // Added members from OptionsDialog.cpp
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
    int priority; // New member for engine process priority
    Language language; // New member for language setting
    int current_engine; // 0 for none, 1 for primary, 2 for secondary
    QString piece_set; // New member for selected piece set
    int three_move_option; // New member for 3-move game options
    LogLevel min_log_level; // New member for minimum log level
    bool enable_game_timer; // New member to enable/disable the game timer
    int white_player_type; // New member to define white player type (Human/AI)
    int black_player_type; // New member to define black player type (Human/AI)
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