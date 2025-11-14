#pragma once

#include "GeminiAI.h"
#include <QProcess>
#include <QTimer>
#include "checkers_types.h"
#include "OLD/c_declarations.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "c_logic.h" // Provides declarations for egdb_wrapper functions and other C utilities
#ifdef __cplusplus
}
#endif


#define SEARCH_DEPTH 3 // Define the search depth for minimax





enum AI_State {
    Idle,
    Autoplay,
    EngineMatch,
    RunTestSet,
    EngineGame,
    AnalyzeGame,
    AnalyzePdn,
    ObserveGame
};



class AI : public QObject
{
    Q_OBJECT

signals:
    void searchFinished(bool moveFound, bool aborted, CBmove bestMove, const QString& status, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void engineOutput(const QString& output);
    void engineError(const QString& error);
    void moveFound(CBmove bestMove);
    void updateToolbarIcon(int iconType, int iconIndex);
    void logEngineOutput(const QString& engineName, const QString& pdnMoveText, double totalTime, double maxTimeSent, double timeUsed, const QString& analysis);
    void captureSoundRequest();
    void playSoundRequest();
    void requestNewGame(int gametype);
    void updateStatus(const QString& status);
    void changeState(AppState newState);
    void evaluationReady(int score, int depth); // New signal to emit evaluation score and depth

public:
    explicit AI(const QString& egdbPath, QObject *parent = nullptr);
    ~AI();

    // Public functions for User Book
    void loadUserBook(const QString& filename);
    void saveUserBook(const QString& filename);
    void addMoveToUserBook(const Board8x8 board, const CBmove& move);
    void deleteCurrentEntry();
    void navigateToNextEntry();
    void navigateToPreviousEntry();
    void resetNavigation();
    void deleteAllEntriesFromUserBook();
    const userbookentry* getCurrentEntry() const;
    bool lookupMove(const Board8x8 board, int color, int gametype, CBmove* bookMove) const;

    void setSearchParameters(Board8x8 board, int color, double maxtime,
                             int info, int moreinfo,
                             int (*engineGetMoveFunc)(Board8x8, int, double, char*, int*, int, int, CBmove*),
                             CB_ENGINECOMMAND engineCommandFunc,
                             int gametype);
public slots:
    void setMode(AI_State mode);
    void doWork();
    void requestAbort();
    void loadEngine(const QString& enginePath);
    void requestMove(const Board8x8& board, int colorToMove, double timeLimit);
    void abortSearch();
    bool internalGetMove(Board8x8 board, int color, double maxtime, char statusBuffer[1024], QAtomicInt *playnow, int info, int moreinfo, CBmove *bestMove);
    void startAutoplay(const Board8x8& board, int color);
    void startEngineMatch(int totalGames);
    void startRunTestSet();
    void startEngineGame();
    void startAnalyzeGame();
    void startAnalyzePdn();
    void startObserveGame();
    bool sendCommand(const QString& command, QString& reply);
    void initEngineProcess(); // New slot to initialize QProcess in the AI's thread
    void quitEngineProcess(); // New slot to quit and clean up QProcess
    void handleAutoplayState();
    void handleEngineMatchState();
    void handleRunTestSetState();
    void handleEngineGameState();
    void handleAnalyzeGameState();
    void handleAnalyzePdnState();
    void handleObserveGameState();
    void move_to_pdn_english_from_list(int nmoves, CBmove* movelist, const CBmove* move, char* pdn_c, int gametype);
    void setPriority(int priority);
    void setHandicap(int handicapDepth);

private:
    void runStateMachine();
    QTimer *m_stateMachineTimer;
    QAtomicInt m_abortRequested;
    Board8x8 m_board;
    int m_color;
    double m_maxtime;
    int m_info;
    int m_moreinfo;
public:
    CB_GETMOVE m_engineGetMoveFunc = nullptr;
    CB_ENGINECOMMAND m_engineCommandFunc = nullptr;
    int m_gametype;
    QProcess* m_engineProcess = nullptr;
    CBmove m_bestMove; // Added to store the best move found by calculateInternalMove
    bool m_useInternalAI = true; // New member to control internal AI usage
    int *m_playnow_shim; // Shim for playnow flag from external engine interface
    int m_maxEGDBPieces; // To store the maxpieces returned by db_init

    userbookentry m_userbook[MAXUSERBOOK];
    int m_userbooknum;
    int m_userbookcur;
    AI_State m_mode;
    bool m_startMatchFlag;
    int m_matchGameNumber;
    int m_matchMoveCount;
    bool m_matchGameOver;
    bool m_egdbInitialized; // New member to track EGDB initialization status
    int m_totalMatchGames;
    int m_handicapDepth; // New member to store handicap depth
    GeminiAI m_geminiAI; // Instance of the GeminiAI engine

    // Private helper functions for move generation (moved from MoveGenerator)
    int makemovelist(int color, CBmove movelist[MAXMOVES], int b[12][12], int *isjump, int *n); // Corrected return type to int
    void board8toboard12(Board8x8 board, int board12[12][12]);
    void whitecapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void blackcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void whitekingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);
    void blackkingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n);


    // Private helper functions
    CBmove calculateInternalMove(const Board8x8& board, int colorToMove, CBmove movelist[MAXMOVES], int& numMoves); // Moved from public
    PDN_RESULT lookupEgdb(const QString& fenPosition);
    CBmove parseMoveString(const QString& moveString);
    bool isGameOver(Board8x8 board, int color);
    void parseEngineOutput(const QString& output);
    void move_to_pdn_english(const Board8x8& board, int color, const CBmove* move, char* pdn_c, int gametype);
    void get_movelist_from_engine(const Board8x8& board, int color, CBmove* movelist, int* nmoves, int* iscapture);
    // Piece-Square Tables (PSTs) for evaluation
    static const int whiteManPST[8][8];
    static const int whiteKingPST[8][8];
    static const int blackManPST[8][8];
    static const int blackKingPST[8][8];

    int evaluate(const Board8x8& board);
    static void testMoveGeneration(); // Declare as static member function

};