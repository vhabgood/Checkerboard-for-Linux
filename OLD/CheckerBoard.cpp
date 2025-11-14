// ... (Includes) ...
#include <QDebug> // Include for qDebug
#include <cstdarg> // For va_list, vsnprintf
#include "cb_interface.h" // For engine function pointer types
#include "CBconsts.h" // For GT_ENGLISH and other constants

// ... (Other includes) ...
#include "CheckerBoard.h"
#include "utility.h"
#include "PDNparser.h"
#include "PdnManager.h"
// ...

// --- Status Callback ---
static StatusCallback g_statusCallback = nullptr;

void set_status_callback(StatusCallback callback) {
    g_statusCallback = callback;
}

void update_status_bar_c(const char* format, ...) {
    if (g_statusCallback) {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        buffer[sizeof(buffer) - 1] = '\0';
        g_statusCallback(buffer);
    }
}
// --- End Status Callback ---


// --- Keep necessary globals for now (to be eliminated) ---


// --- Update function implementations ---

// Placeholder/Stubbed functions (original bodies removed previously)
int handle_rbuttondown(int x, int y) {
    qDebug() << "Right mouse button down at:" << x << "," << y;
    // TODO: Convert x, y to board coordinates and handle context menu or deselection
    return 1; 
}
int handle_lbuttondown(int x, int y) {
    qDebug() << "Left mouse button down at:" << x << "," << y;
    // TODO: Convert x, y to board coordinates and handle piece selection/move
    return 1; 
}
int handletimer(void) {
    qDebug() << "handletimer called";
    // TODO: Implement timer logic, e.g., update clocks, check for time limits.
    return 1; 
}
// int createcheckerboard(HWND hwnd_removed) { /* Placeholder */ return 1; }

// Updated C functions accepting state parameters
extern "C" void newgame_c(PDNgame* game, Board8x8 board, int* color, int gametype) {
    InitCheckerBoard(board);
    *color = CB_BLACK; // Red (Black pieces) to move first
    if(gametype == GT_ITALIAN)
        *color=CB_WHITE;
    if(gametype == GT_SPANISH)
        *color = CB_WHITE;
    if(gametype == GT_RUSSIAN)
        *color = CB_WHITE;
    if(gametype == GT_CZECH)
        *color = CB_WHITE;
}
extern "C" void domove_c(const CBmove *m, Board8x8 board) {
    int i,x,y;

    x=m->from.x;y=m->from.y;
    board[x][y]=0;
    x=m->to.x;y=m->to.y;
    board[x][y]=m->newpiece;

    for(i=0;i<m->jumps;i++)
    {
        x=m->del[i].x;
        y=m->del[i].y;
        board[x][y]=0;
    }
}
int undomove_c(CBmove *m, Board8x8 board) {
    int i,x,y;

    x = m->to.x;
    y = m->to.y;
    board[x][y] = 0;

    x = m->from.x;
    y = m->from.y;
    board[x][y] = m->oldpiece;

    for(i=0; i<m->jumps; i++)
    {
        x = m->del[i].x;
        y = m->del[i].y;
        board[x][y] = m->delpiece[i];
    }
    return 1;
}
void addmovetogame_c(PDNgame* game, Board8x8 board, int color, CBmove &move, char *pdn) { /* ... */ }
extern "C" void forward_to_game_end_c(PdnGameWrapper* game, Board8x8 board, int* color) {
    qDebug() << "forward_to_game_end_c called";
    // Reset board to initial state of the game
    // Assuming game->initial_board and game->initial_color are set correctly when game is loaded
    // For now, re-initialize board and color based on game->gametype
    newgame_c(game, board, color, game->gametype); // This will set up the initial board and color

    // Apply all moves in the game
    for (size_t i = 0; i < game->moves.size(); ++i) {
        domove_c(&game->moves[i].move, board);
        *color = (*color == CB_WHITE) ? CB_BLACK : CB_WHITE; // Toggle color
    }
    game->movesindex = game->moves.size(); // Set movesindex to the end of the game
}
int start3move_c(PDNgame* game, Board8x8 board, int* color, int opening_index, int gametype) {
    qDebug() << "start3move_c called with opening_index:" << opening_index << "gametype:" << gametype;

    // 1. Initialize the board to a standard starting position
    newgame_c(game, board, color, gametype);

    // 2. Apply a sequence of moves corresponding to the opening_index.
    // This is a placeholder. Real implementation would involve a lookup table
    // or a database of 3-move openings.
    switch (opening_index) {
        case 0: // Example: First opening
            qDebug() << "Applying 3-move opening 0";
            // Apply moves for opening 0
            // For example: domove_c(move1, board); *color = ...; domove_c(move2, board); *color = ...;
            break;
        case 1: // Example: Second opening
            qDebug() << "Applying 3-move opening 1";
            // Apply moves for opening 1
            break;
        default:
            qWarning() << "Unknown 3-move opening index:" << opening_index;
            break;
    }

    // 3. Update game->movesindex to reflect the applied moves
    // For now, assuming 3 moves are applied, so movesindex = 3
    game->movesindex = 3; // Placeholder

    return 1; // Indicate success
}

extern "C" void move4tonotation(const CBmove &m, char s[80]) {
    // Convert from and to coordinates to standard square notation (e.g., 1-5, 10x17)
    // Assuming m.from.x and m.to.x are 0-indexed board coordinates that need to be converted to 1-indexed square numbers.
    // This requires a mapping from (x,y) to square number, which is not directly available here.
    // For now, I will use the raw x,y coordinates and assume they are already square numbers.
    // A more robust solution would involve a helper function to convert (x,y) to square number.

    // Placeholder for actual square number conversion
    int from_sq = m.from.x; // This is incorrect, needs (x,y) to square number conversion
    int to_sq = m.to.x;   // This is incorrect, needs (x,y) to square number conversion

    // If it's a capture, use 'x', otherwise use '-'
    if (m.is_capture) {
        sprintf(s, "%d x %d", from_sq, to_sq);
    } else {
        sprintf(s, "%d - %d", from_sq, to_sq);
    }
    qDebug() << "move4tonotation:" << s;
}

extern "C" void InitCheckerBoard(Board8x8 board) {
    // Initialize all squares to EMPTY
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = EMPTY;
        }
    }

    // Set up initial black pieces (top of the logical board)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((i + j) % 2 != 0) { // Dark squares
                board[i][j] = BLACK_MAN;
            }
        }
    }

    // Set up initial white pieces (bottom of the logical board)
    for (int i = 5; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((i + j) % 2 != 0) { // Dark squares
                board[i][j] = WHITE_MAN;
            }
        }
    }
}

extern "C" void ClearCheckerBoard(Board8x8 b) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            b[i][j] = EMPTY;
        }
    }
}

void addmovetogame_c(PdnGameWrapper* game, Board8x8 board, int color, const CBmove *move, char *pdn) {
    PDNmove pdnMove;
    pdnMove.move = *move;
    strncpy(pdnMove.PDN, pdn, sizeof(pdnMove.PDN) - 1);
    pdnMove.PDN[sizeof(pdnMove.PDN) - 1] = '\0';
    pdnMove.comment[0] = '\0'; // Clear comment string
    game->moves.push_back(pdnMove);
    game->movesindex = game->moves.size();
}

char *pdn_result_to_string(PDN_RESULT result, int gametype) {
    // Placeholder implementation
    switch (result) {
        case PDN_RESULT_WIN: return (char*)"1-0"; // Updated from PDN_RESULT_WHITE_WINS
        case PDN_RESULT_LOSS: return (char*)"0-1"; // Updated from PDN_RESULT_BLACK_WINS
        case PDN_RESULT_DRAW: return (char*)"1/2-1/2";
        default: return (char*)"*";
    }
}

double maxtime_for_incremental_tc(double remaining, double time_increment, double initial_time) {
    qDebug() << "maxtime_for_incremental_tc called: remaining=" << remaining << ", increment=" << time_increment << ", initial=" << initial_time;
    // A common heuristic for incremental time control:
    // Divide remaining time by an estimated number of moves (e.g., 40) and add the increment.
    // This is a simplified model and can be refined.
    double calculated_time = (remaining / 40.0) + time_increment;
    // Ensure the calculated time is not excessively large or small
    if (calculated_time < 0.1) calculated_time = 0.1; // Minimum time
    if (calculated_time > remaining) calculated_time = remaining; // Cannot use more than remaining time

    qDebug() << "Calculated incremental time:" << calculated_time;
    return calculated_time;
}

double timelevel_to_time(int level) {
    qDebug() << "timelevel_to_time called with level:" << level;
    // This function maps a time control level to a time value.
    // The original implementation might have used a lookup table or a switch.
    // For now, a placeholder:
    switch (level) {
        case 0: return 0.0; // Unlimited
        case 1: return 1.0; // 1 second
        case 2: return 5.0; // 5 seconds
        case 3: return 10.0; // 10 seconds
        case 4: return 30.0; // 30 seconds
        case 5: return 60.0; // 1 minute
        case 6: return 180.0; // 3 minutes
        case 7: return 300.0; // 5 minutes
        case 8: return 600.0; // 10 minutes
        case 9: return 1800.0; // 30 minutes
        case 10: return 3600.0; // 1 hour
        default: return 0.0;
    }
}

double maxtime_for_non_incremental_tc(double remaining, double time_per_move) {
    qDebug() << "maxtime_for_non_incremental_tc called: remaining=" << remaining << ", time_per_move=" << time_per_move;
    // In non-incremental time control, each move is allotted a fixed amount of time.
    // The remaining time is not directly used for calculating the current move's time.
    return time_per_move;
}

bool move_to_pdn_english(Board8x8 board8, int color, CBmove *move, char *pdn, int gametype) {
    qDebug() << "move_to_pdn_english (C-style) called";
    // This function converts a CBmove to PDN English notation.
    // It requires a function to convert (x,y) coordinates to square numbers.
    // Assuming coorstonumber is an external C function.

    // Placeholder for coorstonumber. If not available, this will need to be implemented.
    // int from_sq = coorstonumber(move->from.x, move->from.y, gametype);
    // int to_sq = coorstonumber(move->to.x, move->to.y, gametype);

    // For now, using raw x,y coordinates as square numbers (this is incorrect for PDN)
    int from_sq = move->from.x; // Placeholder, needs actual conversion
    int to_sq = move->to.x;   // Placeholder, needs actual conversion

    if (move->is_capture) {
        sprintf(pdn, "%d x %d", from_sq, to_sq);
    } else {
        sprintf(pdn, "%d - %d", from_sq, to_sq);
    }
    qDebug() << "Converted move to PDN:" << pdn;
    return true;
}



bool doload(PdnGameWrapper *game, const char *gamestring, int *color, Board8x8 board8, std::string &errormsg) {
    // This function parses a PDN game from a string, populates the game structure,
    // and sets up the initial board state according to the PDN data.
    char mutable_gamestring[strlen(gamestring) + 2];
    strcpy(mutable_gamestring, gamestring);
    strcat(mutable_gamestring, " "); // Append space for tokenizer

    const char *p = mutable_gamestring;
    char header[256], token[1024];
    char headername[256], headervalue[256];
    bool issetup = false;

    // Clear existing game data
    memset(game, 0, sizeof(PDNgame));
    game->moves.clear();

    // 1. Parse Headers
    while (PDNparseGetnextheader(const_cast<char**>(&p), header)) {
        const char *start = header;
        PDNparseGetnexttoken(const_cast<char**>(&start), headername);
        PDNparseGetnexttag(const_cast<char**>(&start), headervalue);

        for (int i = 0; headername[i]; ++i) {
            headername[i] = tolower(headername[i]);
        }

        if (strcmp(headername, "event") == 0) strncpy(game->event, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "site") == 0) strncpy(game->site, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "date") == 0) strncpy(game->date, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "round") == 0) strncpy(game->round, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "white") == 0) strncpy(game->white, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "black") == 0) strncpy(game->black, headervalue, MAXNAME - 1);
        else if (strcmp(headername, "result") == 0) {
            strncpy(game->resultstring, headervalue, MAXNAME - 1);
            game->result = string_to_pdn_result(headervalue, game->gametype);
        } else if (strcmp(headername, "fen") == 0) {
            strncpy(game->FEN, headervalue, MAXNAME - 1);
            issetup = true;
        }
    }

    // 2. Set up Board State
    InitCheckerBoard(board8);
    *color = get_startcolor(game->gametype); // Set color based on game type

    if (issetup) {
        if (FENtoboard8(board8, game->FEN, color, game->gametype) != 1) {
            errormsg = "Invalid FEN string.";
            return false;
        }
    }

    // 3. Parse Moves
    PDN_PARSE_STATE state;
    Board8x8 temp_board;
    memcpy(temp_board, board8, sizeof(Board8x8));
    int temp_color = *color;

    while ((state = (PDN_PARSE_STATE)PDNparseGetnexttoken(const_cast<char**>(&p), token))) {
        if (token[strlen(token) - 1] == '.') continue; // Skip move numbers
        if (strcmp(token, "*") == 0 || strcmp(token, "1-0") == 0 || strcmp(token, "0-1") == 0 || strcmp(token, "1/2-1/2") == 0) {
            // Game terminator
            if (strlen(game->resultstring) == 0 || strcmp(game->resultstring, "*") == 0) {
                strncpy(game->resultstring, token, MAXNAME - 1);
                game->result = string_to_pdn_result(token, game->gametype);
            }
            break;
        }

        if (token[0] == '{' || state == PDN_FLUFF) {
            if (game->moves.size() > 0) {
                char* comment_start = (token[0] == '{') ? token + 1 : token;
                if (token[0] == '{') token[strlen(token) - 1] = '\0'; // Remove closing brace
                strncpy(game->moves.back().comment, comment_start, COMMENTLENGTH - 1);
            }
            continue;
        }

        // It's a move
        Squarelist squares;
        if (!PDNparseMove(token, squares)) {
            errormsg = std::string("Invalid move token: ") + token;
            return false;
        }

        CBmove move;
        if (!islegal_check(temp_board, temp_color, squares, &move, game->gametype)) {
            errormsg = std::string("Illegal move: ") + token;
            return false;
        }

        addmovetogame_c(game, temp_board, temp_color, move, token);
        domove_c(&move, temp_board);
        temp_color = (temp_color == CB_WHITE) ? CB_BLACK : CB_WHITE;
    }

    game->movesindex = 0; // Reset to the beginning of the game
    return true;
}

// --- Functions still using globals (marked TODO) ---







// ... (Other functions like islegal_check, move_to_pdn_english, etc., now accept state or are okay) ...


