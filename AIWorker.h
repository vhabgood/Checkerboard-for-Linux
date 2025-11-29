#pragma once

#include <QObject>
#include <unordered_map>
#include "checkers_types.h"
#include <QAtomicInt>

#define MAX_DEPTH 20
#define WIN_SCORE 100000
#define LOSS_SCORE -100000

// Transposition Table Entry
struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    enum EntryType { EXACT, ALPHA, BETA } type;
    CBmove bestMove;
};

class AIWorker : public QObject
{
    Q_OBJECT

public:
    explicit AIWorker(QObject *parent = nullptr);
    ~AIWorker();

public slots:
    void performTask(AI_State task, const Board8x8& board, int color, double maxtime);
    void requestAbort();

signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void evaluationReady(int score, int depth);

private:
    void searchBestMove(Board8x8 board, int color, double maxtime);
    int evaluateBoard(const Board8x8& board, int color, int egdb_context);
    int minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull, int egdb_context);
    int quiescenceSearch(Board8x8 board, int color, int alpha, int beta, int egdb_context);
    bool isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor);
    static bool compareMoves(const CBmove& a, const CBmove& b);
    uint64_t generateZobristKey(const Board8x8& board, int colorToMove);
    static void initZobristKeys();

    QAtomicInt m_abortRequested;
    std::unordered_map<uint64_t, TTEntry> m_transpositionTable;
    CBmove m_killerMoves[MAX_DEPTH][2];
    int m_historyTable[8][8][8][8];
    int m_lastEvaluationScore;
    int m_lastSearchDepth;

    // Piece-Square Tables (PSTs) for evaluation
    static const int whiteManPST[8][8];
    static const int whiteKingPST[8][8];
    static const int blackManPST[8][8];
    static const int blackKingPST[8][8];

    // Zobrist Hashing
    static uint64_t ZobristTable[8][8][5];
    static uint64_t ZobristWhiteToMove;
};
