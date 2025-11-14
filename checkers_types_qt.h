#pragma once

#include <string>
#include <vector>
#include <QList> // For Squarelist Qt version
#include <QColor> // For QRgb type
#include <QAtomicInt> // Required for CB_GETMOVE
#include <QString> // Required for QString members in CBoptions
#include <QMetaType> // Required for Q_DECLARE_METATYPE

#include "checkers_types_c.h" // Include the C-compatible types

// Declare meta-types for C-style structs used in signals/slots
Q_DECLARE_METATYPE(Board8x8)
Q_DECLARE_METATYPE(pos)
Q_DECLARE_METATYPE(POSITION)
Q_DECLARE_METATYPE(CBmove)
Q_DECLARE_METATYPE(PDNgame) // If PDNgame is used directly in signals/slots

// Function pointer types for engine interaction
typedef int (*CB_GETMOVE)(Board8x8 board, int color, double maxtime, char statusBuffer[1024], QAtomicInt *playnow, int info, int moreinfo, CBmove *bestMove);
typedef int (*CB_ENGINECOMMAND)(const char *command, char reply[ENGINECOMMAND_REPLY_SIZE]);

typedef QList<int> Squarelist;

// C++ specific options struct
struct CBoptions {
    bool sound;
    bool highlight;
    bool invert_board;
    bool show_coordinates;
    double time_per_move = 2.0; // Added for AI time control
    int gametype;
    QString engine_path;
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
    int priority;
    Language language; // New member for language setting
    int current_engine; // 0 for none, 1 for primary, 2 for secondary
};
Q_DECLARE_METATYPE(CBoptions)

// The C++ specific PDNgame struct wrapper
struct PdnGameWrapper : public PDNgame {
    std::vector<PDNmove> moves; // C++ specific: Use std::vector for moves

    PdnGameWrapper& operator=(const PDNgame& other) {
        if (static_cast<const PDNgame*>(this) != &other) {
            // copy all the members from PDNgame
            strncpy(this->event, other.event, sizeof(this->event) - 1);
            this->event[sizeof(this->event) - 1] = '\0';
            strncpy(this->site, other.site, sizeof(this->site) - 1);
            this->site[sizeof(this->site) - 1] = '\0';
            strncpy(this->date, other.date, sizeof(this->date) - 1);
            this->date[sizeof(this->date) - 1] = '\0';
            strncpy(this->round, other.round, sizeof(this->round) - 1);
            this->round[sizeof(this->round) - 1] = '\0';
            strncpy(this->white, other.white, sizeof(this->white) - 1);
            this->white[sizeof(this->white) - 1] = '\0';
            strncpy(this->black, other.black, sizeof(this->black) - 1);
            this->black[sizeof(this->black) - 1] = '\0';
            strncpy(this->resultstring, other.resultstring, sizeof(this->resultstring) - 1);
            this->resultstring[sizeof(this->resultstring) - 1] = '\0';
            strncpy(this->FEN, other.FEN, sizeof(this->FEN) - 1);
            this->FEN[sizeof(this->FEN) - 1] = '\0';
            this->result = other.result;
            this->gametype = other.gametype;
            this->num_moves = other.num_moves;
            this->movesindex = other.movesindex;
        }
        return *this;
    }
};
Q_DECLARE_METATYPE(PdnGameWrapper)
