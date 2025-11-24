#pragma once

#include "checkers_types.h"
#include <QObject>
#include <random> // For Zobrist key generation
#include <unordered_map> // For transposition table
#include "engine_wrapper.h" // Include ExternalEngine definition
#include "dblookup.h"

#define MAX_DEPTH 20 // Define the maximum search depth for iterative deepening
#define WIN_SCORE 100000
#define LOSS_SCORE -100000
#define R 2

// Transposition Table Entry
struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    enum NodeType { EXACT, ALPHA, BETA } type;
    CBmove bestMove;
};

// Define AI_State enum for controlling AI operational modes
enum AI_State {
    Idle,
    AnalyzeGame,
    EngineMatch,
    Autoplay,
    RunTestSet,
    AnalyzePdn
};
Q_DECLARE_METATYPE(AI_State)

class GeminiAI : public QObject

{

    Q_OBJECT

public:

    explicit GeminiAI(const QString& egdbPath, QObject *parent = nullptr);

    ~GeminiAI();

    CBmove getBestMove(Board8x8 board, int color, double maxtime);
    void requestAbort();
    int getLastEvaluationScore() const { return m_lastEvaluationScore; } // New getter
    int getLastSearchDepth() const { return m_lastSearchDepth; } // New getter
    void setMode(AI_State mode);
    void setHandicap(int handicap);
    bool sendCommand(const QString& command, QString& reply);
    void addMoveToUserBook(const Board8x8& board, const CBmove& move);
    void deleteCurrentEntry();
    void deleteAllEntriesFromUserBook();
    void navigateToNextEntry();
    void navigateToPreviousEntry();
    void resetNavigation();
    void loadUserBook(const QString& filename);
    void saveUserBook(const QString& filename);

public slots:
    void init();
    void requestMove(Board8x8 board, int colorToMove, double timeLimit);
    void doWork();
    void initEngineProcess();
    void quitEngineProcess();
    void startAnalyzeGame(const Board8x8& board, int colorToMove);
    void startAutoplay(const Board8x8& board, int colorToMove);
    void startEngineMatch(int numGames, const Board8x8& board, int colorToMove);
    void startRunTestSet(const Board8x8& board, int colorToMove);
    void startAnalyzePdn(const Board8x8& board, int colorToMove);
    void abortSearch();
    void setExternalEnginePath(const QString& path);
    void setSecondaryExternalEnginePath(const QString& path);
    void setOptions(const CBoptions& options);
    void setEgdbPath(const QString& path);

signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void engineError(const QString& errorMessage);
    void requestNewGame(int gameType);
    void updateStatus(const QString& statusMessage);
    void changeState(AppState newState);
    void evaluationReady(int score, int depth);

private:
    int evaluateBoard(const Board8x8& board, int color);
    int minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull = true);
    int quiescenceSearch(Board8x8 board, int color, int alpha, int beta);
    bool isKingTrapped(const Board8x8& board, int r, int c, const CBmove* legalMoves, int nmoves);
    bool hasCaptures(const Board8x8& board, int colorToMove);
    bool isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor);
    bool isPieceDefended(const Board8x8& board, int r, int c, int pieceColor);
    bool isMoveSafe(const Board8x8& board_after_move, int color);
    int evaluateKingSafety(const Board8x8& board, int color);
    int evaluatePassedMen(const Board8x8& board);
    int evaluateBridge(const Board8x8& board);
    int evaluateCenterControl(const Board8x8& board);
    bool isManPassed(const Board8x8& board, int r, int c);
    static bool compareMoves(const CBmove& a, const CBmove& b);
    pos boardToPosition(const Board8x8& board, int colorToMove);

    // Zobrist Hashing
    static uint64_t ZobristTable[8][8][5]; // [row][col][piece_type: empty, white_man, white_king, black_man, black_king]
    static uint64_t ZobristWhiteToMove; // Hash for white to move
    static void initZobristKeys(); // Static helper to initialize Zobrist keys
    uint64_t generateZobristKey(const Board8x8& board, int colorToMove);

    // Transposition Table
    std::unordered_map<uint64_t, TTEntry> m_transpositionTable;

    QAtomicInt m_abortRequested;
    QString m_egdbPath; // Member to store the EGDB path
    int m_lastEvaluationScore; // New member to store the last evaluation score
    int m_lastSearchDepth; // New member to store the last search depth
    bool m_egdbInitialized; // New member to track EGDB initialization status
    int m_maxEGDBPieces; // To store the maxpieces returned by db_init
    AI_State m_mode; // Current operational mode of the AI
    int m_handicap; // Handicap in terms of search depth reduction

    ExternalEngine *m_primaryExternalEngine; // Primary external engine
    ExternalEngine *m_secondaryExternalEngine; // Secondary external engine
    bool m_useExternalEngine; // Flag to indicate if an external engine should be used
    QString m_externalEnginePath; // Path to the currently selected external engine
    QString m_secondaryExternalEnginePath; // Path to the secondary external engine

    // Piece-Square Tables (PSTs) for evaluation
    static const int whiteManPST[8][8];
    static const int whiteKingPST[8][8];
    static const int blackManPST[8][8];
    static const int blackKingPST[8][8];

    // Killer Moves for move ordering
    CBmove m_killerMoves[MAX_DEPTH][2];

    // History Table for move ordering
    int m_historyTable[8][8][8][8];

    CBoptions m_options; // The AI's copy of options
};

