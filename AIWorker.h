#pragma once

#include <QObject>
#include <unordered_map>
#include "checkers_types.h"
#include <QAtomicInt>

#define MAX_DEPTH 20

// Transbitboard_position Table Entry
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
    void performTask(AI_State task, const bitboard_pos& board, int color, double maxtime);
    void requestAbort();

signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void evaluationReady(int score, int depth);

private:
    void searchBestMove(bitboard_pos board, int color, double maxtime);
    int evaluateBoard(const bitboard_pos& board, int color, int egdb_context);
    int minimax(bitboard_pos board, int color, int depth, int alpha, int beta, CBmove* bestMove, bool allowNull, int egdb_context, int mtc_score);
    int quiescenceSearch(bitboard_pos board, int color, int alpha, int beta, int egdb_context, int mtc_score);
    bool isSquareAttacked(const bitboard_pos& board, int r, int c, int attackerColor);
    static bool compareMoves(const CBmove& a, const CBmove& b);
    uint64_t generateZobristKey(const bitboard_pos& board, int colorToMove);
    static void initZobristKeys();

    QAtomicInt m_abortRequested;
    std::unordered_map<uint64_t, TTEntry> m_transbitboard_positionTable;
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
