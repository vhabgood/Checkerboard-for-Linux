#ifndef SEARCHTHREADWORKER_H
#define SEARCHTHREADWORKER_H

#include <QObject>
#include <QElapsedTimer>
#include <QMutex>
#include <QAtomicInt>
#include <QString>
#include "checkers_types.h" // For Board8x8, CBmove, game state etc.
#include "cb_interface.h"   // For engine function pointer types (CB_GETMOVE etc.)

class SearchThreadWorker : public QObject
{
    Q_OBJECT

public:
    explicit SearchThreadWorker(QObject *parent = nullptr);

    // Function to set necessary data before starting the search
    void setSearchParameters(const Board8x8 board, int color, double maxtime,
                              int info, int moreinfo,
                              CB_GETMOVE engineGetMoveFunc,
                              CB_ENGINECOMMAND engineCommandFunc, // Needed for get_movelist_from_engine
                              int gametype); // Need gametype for movelist generation/parsing

    void requestAbort(); // Method for main thread to signal abortion

public slots:
    void doSearch(); // Main execution slot

signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText);
    void updateSearchStatus(const QString& statusText); // For intermediate updates
    void playSoundRequest();
    void requestAnimation(const CBmove& move);
    void logEngineOutput(const QString& engineName, const QString& pdnMove, double totalTime, double maxTimeSent, double timeUsed, const QString& analysis);
    void updateToolbarIcon(int commandId, int bitmapIndex); // To change play icon state

private:
    Board8x8 m_board;
    int m_color;
    double m_maxtime; // This might need adjustment based on timing logic (absolute vs iterative)
    int m_info;
    int m_moreinfo;
    CB_GETMOVE m_engineGetMoveFunc = nullptr;
    CB_ENGINECOMMAND m_engineCommandFunc = nullptr; // For get_movelist
    int m_gametype; // Store gametype

    QAtomicInt m_abortRequested; // Flag for graceful termination
    int m_playnow_shim = 0; // Shim variable to pass its address to getmove

    // --- Helper functions moved/adapted from CheckerBoard.c ---
    // These need safe access to shared state if they depend on it outside search params
    int get_movelist_from_engine(Board8x8 board8, int color, CBmove movelist[], int *nmoves, int *iscapture);
    bool move_to_pdn_english(int nmoves, CBmove movelist[MAXMOVES], CBmove *move, char *pdn, int gametype);
    bool move_to_pdn_english(Board8x8 board8, int color, CBmove *move, char *pdn, int gametype);
    void addMoveToGameLogically(CBmove &move, const QString& pdn); // Placeholder for game logic update signal
    void detectNonConversionDraws(PDNgame &game, bool *is_draw_by_repetition, bool *is_draw_by_40move_rule); // Needs cbgame access (problematic here)
    void send_game_history(PDNgame &game, const Board8x8 board, int color); // Needs cbgame access
    void format_time_args(double increment, double remaining, uint32_t *info, uint32_t *moreinfo);
    double maxtime_for_incremental_tc(double remaining);
    double maxtime_for_non_incremental_tc(double remaining, double increment);
     void save_time_stats(int enginenum, double maxtime, double elapsed); // Needs static/shared state

    // Temporary struct to hold user book data if needed during search
    // This should ideally be queried *before* starting the thread
    struct TempUserBookEntry {
        pos position;
        CBmove move;
    };
    QList<TempUserBookEntry> m_userbook; // Or pass relevant entries if found before search

     // Access shared state (Needs proper mechanism - signals/mutexes or passed data)
     // CBoptions* m_cboptions;
     // time_ctrl_t* m_time_ctrl;
     // PDNgame* m_cbgame; // Very problematic to access directly here
     // int* m_userbooknum;
     // userbookentry* m_userbook_global; // Pointer to global userbook
     // bool* m_reset_move_history; // Needs atomic/mutex
     // bool* m_addcomment; // Needs atomic/mutex
     // bool* m_gameover; // Needs atomic/mutex
     // int* m_currentengine; // Needs atomic/mutex
     // --- End Shared State ---

};

#endif // SEARCHTHREADWORKER_H

