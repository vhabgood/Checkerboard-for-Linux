#include "SearchThreadWorker.h"
#include <QThread>
#include <QDebug>
#include <cstring> // For memcpy, memset
#include <QtCore/qmath.h> // For qMin, qMax if needed (instead of algorithm min/max)

// Temporary includes - dependencies to resolve
#include "utility.h"      // For cblog, writefile (needs replacing)
#include "bitboard.h"     // For boardtobitboard
#include "checkerboard.h" // For global state access (BAD)


SearchThreadWorker::SearchThreadWorker(QObject *parent) : QObject(parent)
{
    m_abortRequested.store(0);
}

void SearchThreadWorker::setSearchParameters(const Board8x8 board, int color, double maxtime,
                                           int info, int moreinfo,
                                           CB_GETMOVE engineGetMoveFunc,
                                           CB_ENGINECOMMAND engineCommandFunc,
                                           int gametype)
{
    memcpy(m_board, board, sizeof(Board8x8));
    m_color = color;
    m_maxtime = maxtime; // TODO: Review how maxtime is calculated and used (absolute vs iterative budget)
    m_info = info;
    m_moreinfo = moreinfo;
    m_engineGetMoveFunc = engineGetMoveFunc;
    m_engineCommandFunc = engineCommandFunc;
    m_gametype = gametype;
    m_abortRequested.store(0); // Reset abort flag
    m_playnow_shim = 0; // Reset shim

    // TODO: Copy necessary options (like userbook enabled) or pass them
    // TODO: Query userbook *before* calling this, pass relevant move if found
}

void SearchThreadWorker::requestAbort()
{
    m_abortRequested.store(1);
    m_playnow_shim = 1; // Set the shim variable the engine checks
}

void SearchThreadWorker::doSearch()
{
    qDebug() << "SearchThreadWorker starting search...";
    QElapsedTimer timer;
    timer.start();

    // --- State Initialization ---
    CBmove bestMove;
    memset(&bestMove, 0, sizeof(bestMove));
    char statusBuffer[1024] = {0}; // Buffer for engine's status string
    QString pdnMoveText = "";
    bool moveFound = false;
    int gameResult = CB_UNKNOWN;
    int nmoves = 0;
    CBmove movelist[MAXMOVES];
    bool have_valid_movelist = false;
    int iscapture = 0;
    double calculated_maxtime = m_maxtime; // Use adjusted maxtime based on logic below
    double elapsed_time = 0;


    // Reset abort flag just in case
    m_abortRequested.store(0);
    m_playnow_shim = 0;

    // --- Adjust Thread Priority (Example) ---
    // This should ideally be set on the QThread object *before* starting it
    // QThread::currentThread()->setPriority(QThread::LowPriority); // Example


    // --- TODO: Handle Timing Logic (Incremental vs Fixed) ---
    // This logic depends heavily on shared state (cboptions, time_ctrl, CBstate)
    // It needs to be refactored to either:
    // 1. Pass all necessary timing info into setSearchParameters
    // 2. Have the main thread calculate the correct maxtime and pass *that*
    // For now, using the passed m_maxtime directly, which might be incorrect for some modes.
    // calculated_maxtime = ... calculate based on m_info, m_moreinfo, game state ...


    // --- Check for Forced Moves / No Moves ---
    // Needs engineCommandFunc for get_movelist_from_engine
    bool canUseGetMoveList = (m_engineCommandFunc != nullptr); // Basic check

    if (canUseGetMoveList) {
         if (get_movelist_from_engine(m_board, m_color, movelist, &nmoves, &iscapture) == 0) {
             have_valid_movelist = true;
         } else {
             // Fallback for English checkers if get_movelist fails?
             if (m_gametype == GT_ENGLISH) {
                 nmoves = getmovelist(m_color, movelist, m_board, &iscapture); // Assumes getmovelist is available
                 have_valid_movelist = true; // Built-in generator is valid for English
             } else {
                 nmoves = -1; // Indicate we couldn't get moves
                 have_valid_movelist = false;
             }
         }
    } else if (m_gametype == GT_ENGLISH) {
         nmoves = getmovelist(m_color, movelist, m_board, &iscapture); // Assumes getmovelist is available
         have_valid_movelist = true;
    } else {
         nmoves = -1; // Cannot determine moves without engine command or built-in gen
         have_valid_movelist = false;
    }


    if (nmoves == 0) {
        qDebug() << "No legal moves found.";
        // TODO: Handle game over logic based on application state (e.g., ENGINEMATCH)
        // This requires access to CBstate and potentially setting gameover flag
        // if (CBstate == ENGINEMATCH || ...) { gameover = true; gameResult = CB_LOSS; }
        emit searchFinished(false, false, bestMove, "No legal moves", CB_LOSS, ""); // Assume loss if no moves
        return;
    }
    if (nmoves == 1 && have_valid_movelist) {
        qDebug() << "Forced move found.";
        bestMove = movelist[0];
        moveFound = true;
        // Generate PDN for the forced move
        char pdn_c[40] = {0};
        if (m_gametype == GT_ENGLISH || have_valid_movelist) { // Use detailed PDN if possible
             move_to_pdn_english(m_board, m_color, &bestMove, pdn_c, m_gametype);
        } else {
             move4tonotation(bestMove, pdn_c); // Fallback
        }
        pdnMoveText = QString::fromUtf8(pdn_c);
        emit searchFinished(true, false, bestMove, "Forced move", CB_UNKNOWN, pdnMoveText);
        return;
    }

    // --- TODO: Check User Book ---
    // This should ideally happen *before* the thread starts.
    // If it must happen here, it needs safe access to userbook data.
    // bool foundInUserBook = false;
    // if (m_cboptions->userbook) { ... check m_userbook ... }
    // if (foundInUserBook) { emit searchFinished(...); return; }


    // --- Call Engine's getmove ---
    if (m_engineGetMoveFunc) {
        Board8x8 boardBeforeMove;
        memcpy(boardBeforeMove, m_board, sizeof(Board8x8)); // Save original board

        // Update toolbar icon to "thinking" state
        emit updateToolbarIcon(MOVESPLAY, 19); // Assuming 19 is the red/thinking icon index

        // --- TODO: send_game_history ---
        // This needs access to the global cbgame, which is problematic.
        // The history string should ideally be generated by MainWindow and passed
        // via setSearchParameters or another method.
        // send_game_history(*m_cbgame, m_board, m_color);

        // Call the engine
        qDebug() << "Calling engine getmove...";
        timer.restart(); // Start timing the engine call itself

        // Pass address of our shim variable
        gameResult = m_engineGetMoveFunc(m_board, m_color, calculated_maxtime, statusBuffer,
                                         &m_playnow_shim, m_info, m_moreinfo, &bestMove);

        elapsed_time = timer.elapsed() / 1000.0; // Get elapsed time in seconds
        qDebug() << "Engine getmove returned after" << elapsed_time << "s. Result:" << gameResult;

        // Update toolbar icon back to normal state
        emit updateToolbarIcon(MOVESPLAY, 2); // Assuming 2 is the normal icon index

        // --- Process Engine Result ---

        // TODO: Update timing stats (incremental/fixed) - needs access to time_ctrl/cboptions
        // save_time_stats(currentEngine, calculated_maxtime, elapsed_time);

        if (m_abortRequested.load()) {
            qDebug() << "Search aborted by request.";
             emit searchFinished(false, true, bestMove, "Search aborted", gameResult, "");
            return;
        }

        // --- Determine the move made ---
        // If the engine modified m_board directly (as per API doc)
        if (have_valid_movelist) {
            bool foundMatch = false;
            for (int i = 0; i < nmoves; ++i) {
                Board8x8 tempBoard;
                memcpy(tempBoard, boardBeforeMove, sizeof(Board8x8));
                domove(movelist[i], tempBoard); // Assuming domove is accessible
                if (memcmp(m_board, tempBoard, sizeof(Board8x8)) == 0) {
                    bestMove = movelist[i];
                    moveFound = true;
                    // Generate PDN text
                    char pdn_c[40] = {0};
                    move_to_pdn_english(nmoves, movelist, &bestMove, pdn_c, m_gametype);
                    pdnMoveText = QString::fromUtf8(pdn_c);
                    qDebug() << "Move identified:" << pdnMoveText;
                    break;
                }
            }
            if (!foundMatch) {
                qWarning() << "Engine returned board doesn't match any generated legal move!";
                // Restore board? Signal error?
                 emit searchFinished(false, false, bestMove, "Engine move mismatch", gameResult, "");
                 return;
            }
        } else {
            // Engine should have filled bestMove struct (for non-English or no get_movelist)
            // Verify basic validity?
            moveFound = true; // Assume engine returned a valid move struct
             // Generate basic PDN
             char pdn_c[40] = {0};
             move4tonotation(bestMove, pdn_c); // Assumes move4tonotation is accessible
             pdnMoveText = QString::fromUtf8(pdn_c);
             qDebug() << "Move from engine struct:" << pdnMoveText;
             // We still need to apply this move to m_board if getmove didn't modify it
             // memcpy(m_board, boardBeforeMove, sizeof(Board8x8)); // Restore if needed
             // domove(bestMove, m_board); // Apply the move from struct
        }

    } else {
        qCritical() << "Engine getmove function pointer is null!";
        sprintf(statusBuffer, "Error: Engine not loaded!");
        emit searchFinished(false, false, bestMove, statusBuffer, CB_UNKNOWN, "");
        return;
    }


    // --- Logging and Final Steps ---
    QString statusText = QString::fromUtf8(statusBuffer);

    // TODO: Handle logging based on state (ANALYZEGAME, ENGINEMATCH)
    // This requires access to CBstate, cbgame, etc.
    // Example for Engine Match logging:
    // if (CBstate == ENGINEMATCH) {
    //    QString engineName = ... // Get current engine name
    //    emit logEngineOutput(engineName, pdnMoveText, time_ctrl.total, calculated_maxtime, elapsed_time, statusText);
    // }

    // TODO: Handle adding comment based on addcomment flag / gameover conditions
    // Requires access to cbgame, addcomment flag, gameover flag

    // TODO: Detect draws (repetition, 40-move) - requires access to cbgame history

    // Request sound playback if option is enabled (needs cboptions access)
    // if (m_cboptions->sound) {
    //    emit playSoundRequest();
    // }

    // Signal search completion
    qDebug() << "Search finished. Found:" << moveFound << "PDN:" << pdnMoveText << "Status:" << statusText;
    emit searchFinished(moveFound, false, bestMove, statusText, gameResult, pdnMoveText);

}


// --- Helper Implementations ---
// These are placeholders or need significant refactoring for thread safety / dependencies

int SearchThreadWorker::get_movelist_from_engine(Board8x8 board8, int color, CBmove movelist[], int *nmoves, int *iscapture)
{
    // Basic implementation - needs error handling and parsing refinement
    qDebug() << "SearchThreadWorker::get_movelist_from_engine called";
    if (!m_engineCommandFunc) return 1;

    char fen_c[256];
    char command_c[300];
    char reply_c[ENGINECOMMAND_REPLY_SIZE] = {0};

    board8toFEN(board8, fen_c, color, m_gametype); // Assumes board8toFEN is accessible
    sprintf(command_c, "get movelist %s", fen_c);

    if (!m_engineCommandFunc(command_c, reply_c)) {
        qWarning() << "Engine command 'get movelist' failed or not supported.";
        return 1;
    }
     qDebug() << "Engine reply:" << reply_c;

    // TODO: Implement the parsing logic from CheckerBoard.c::get_movelist_from_engine
    // using reply_c and filling movelist, nmoves, iscapture
    // This parsing is complex and needs careful porting.
    *nmoves = 0; // Placeholder
    *iscapture = 0; // Placeholder
    qWarning() << "Move list parsing from engine reply is NOT IMPLEMENTED.";


    // Example check based on Simplech's reply format (may not apply to others)
    if (strncmp(reply_c, "movelist 0", 10) == 0) {
        *nmoves = 0;
        *iscapture = 0;
        return 0; // Success, no moves
    }
    // Add full parsing here...


    return 1; // Return error until parsing is done
}


bool SearchThreadWorker::move_to_pdn_english(int nmoves, CBmove movelist[MAXMOVES], CBmove *move, char *pdn, int gametype) {
    qDebug() << "Placeholder: move_to_pdn_english (list version) called";
    // TODO: Port logic from CheckerBoard.c
    // This relies on num_moves_matching_fromto, coortonumber, etc.
    move4tonotation(*move, pdn); // Fallback
    return false;
}

bool SearchThreadWorker::move_to_pdn_english(Board8x8 board8, int color, CBmove *move, char *pdn, int gametype) {
     qDebug() << "Placeholder: move_to_pdn_english (board version) called";
    // TODO: Port logic from CheckerBoard.c
    // Needs get_movelist_from_engine or getmovelist
    move4tonotation(*move, pdn); // Fallback
    return false;
}

void SearchThreadWorker::addMoveToGameLogically(CBmove &move, const QString& pdn) {
     qDebug() << "Placeholder: addMoveToGameLogically called - Main thread should handle game state update.";
     // This worker should NOT modify the main game state (cbgame) directly.
     // It signals the result, and MainWindow updates cbgame upon receiving the signal.
}

void SearchThreadWorker::detectNonConversionDraws(PDNgame &game, bool *is_draw_by_repetition, bool *is_draw_by_40move_rule) {
     qDebug() << "Placeholder: detectNonConversionDraws - Cannot access cbgame safely here.";
     *is_draw_by_repetition = false;
     *is_draw_by_40move_rule = false;
     // This logic MUST run in the main thread with access to the full game history.
}

void SearchThreadWorker::send_game_history(PDNgame &game, const Board8x8 board, int color) {
     qDebug() << "Placeholder: send_game_history - Cannot access cbgame safely here.";
     // History string should be generated in main thread and passed to engine command.
}

void SearchThreadWorker::format_time_args(double increment, double remaining, uint32_t *info, uint32_t *moreinfo) {
    qDebug() << "Placeholder: format_time_args called";
    // TODO: Port logic from CheckerBoard.c
    *info = 0;
    *moreinfo = 0;
}

double SearchThreadWorker::maxtime_for_incremental_tc(double remaining) {
     qDebug() << "Placeholder: maxtime_for_incremental_tc called - Needs cboptions access";
    // TODO: Port logic from CheckerBoard.c, needs safe cboptions access
    return qMax(0.1, remaining / 10.0); // Simple placeholder
}
double SearchThreadWorker::maxtime_for_non_incremental_tc(double remaining, double increment) {
     qDebug() << "Placeholder: maxtime_for_non_incremental_tc called";
    // TODO: Port logic from CheckerBoard.c
     return increment; // Simple placeholder
}

void SearchThreadWorker::save_time_stats(int enginenum, double maxtime, double elapsed) {
    qDebug() << "Placeholder: save_time_stats called - Needs shared state";
    // TODO: Port logic, needs access to static/shared counters & logging
}

