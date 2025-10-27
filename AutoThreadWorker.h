#ifndef AUTOTHREADWORKER_H
#define AUTOTHREADWORKER_H

#include <QObject>
#include <QMutex>
#include <QAtomicInt>
#include "checkers_types.h" // For game state enums etc.

// Forward declaration if MainWindow includes this
// class MainWindow;

class AutoThreadWorker : public QObject
{
    Q_OBJECT

public:
    explicit AutoThreadWorker(QObject *parent = nullptr);
    ~AutoThreadWorker();

    void requestStop(); // Method to signal the worker to stop

public slots:
    void doWork(); // Main execution function, replaces AutoThreadFunc
    // Add slots here if the main thread needs to command the worker
    // e.g., void startEngineMatch(); void startAnalysis();

signals:
    // Signals to communicate back to the main thread (MainWindow)
    void updateStatus(const QString &message);
    void updateWindowTitle(const QString &title);
    void requestEngineMove(); // Replaces PostMessage(hwnd, WM_COMMAND, MOVESPLAY, 0)
    void requestMoveForwardAll(); // Replaces PostMessage(hwnd, WM_COMMAND, MOVESFORWARDALL, 0)
    void requestMoveBack();       // Replaces PostMessage(hwnd, WM_COMMAND, MOVESBACK, 0)
    void requestNewGame();        // Replaces PostMessage(hwnd, WM_COMMAND, GAMENEW, 0)
    void requestStart3Move(int openingIndex); // Replaces PostMessage(hwnd, WM_COMMAND, START3MOVE, opening_index)
    void requestSaveGame(const QString& filename); // Replaces SendMessage(hwnd, WM_COMMAND, DOSAVE, 0) after setting global filename
    void changeStateRequest(int newState); // To request state change back in main thread
    void analysisComplete(const QString& analysisFilename); // Signal when analysis is done
    void matchFinished(const QString& finalResult); // Signal when engine match is done

private:
    // Member variables to hold state previously static in AutoThreadFunc or global
    // Need proper initialization
    int m_gamenumber = 0;
    int m_movecount = 0;
    bool m_startmatch = false; // Flag to indicate start of match/analysis
    emstats_t m_emstats;       // Engine match statistics
    // ... other state variables needed ...

    QAtomicInt m_stopRequested; // Flag to safely stop the thread loop

    // --- Placeholder functions for logic moved from CheckerBoard.c ---
    // These would ideally live elsewhere (e.g., GameLogic class) but put here for now
    // They need access to shared game state (cbgame, cbboard8, cbcolor, cboptions) via mutexes or signals
    void updateMatchStats(int result, int movecount, int gamenumber, emstats_t *stats);
    void makeAnalysisFile(const QString& filename);
    int loadNextGame(); // Needs refactoring to signal MainWindow to load
    void startUserBallot(int ballotIndex); // Needs refactoring
    void quickSearchBothEngines(); // Needs refactoring to signal engine searches
    void resetMatchStats(); // Needs careful implementation
    int numBallots(); // Needs access to cboptions/user_ballots
    int game0ToBallot0(int game0);
    int game0ToMatch0(int game0);
    QString engineName(int engineNum); // Wrapper to get engine name
    int getCurrentEngine(); // Access current engine index
    void setCurrentEngine(int engineNum); // Set current engine index
    void toggleCurrentEngine(); // Toggle engine

    // Functions related to file names - should use QDir
    QString emStatsFilename();
    QString emProgressFilename();
    QString emPdnFilename();
    QString emLogFilename();

    // Helper to write to files using Qt
    bool writeToFile(const QString& filename, const QString& content, bool append = true);

     // Access shared state safely (examples - implementation needed)
    // Needs pointers to mutexes and shared data passed in constructor or set via methods
    // QMutex* m_gameStateMutex;
    // PDNgame* m_cbgame;
    // Board8x8* m_cbboard8;
    // int* m_cbcolor;
    // CBoptions* m_cboptions;
    // bool* m_gameover; // Use QAtomicInt or mutex
    // bool* m_animationBusy; // Use QAtomicInt or mutex
    // bool* m_engineBusy; // Use QAtomicInt or mutex
    // bool* m_engineStarting; // Use QAtomicInt or mutex
    // --- End Placeholder Functions ---

};

#endif // AUTOTHREADWORKER_H

