#pragma once

#include <QObject>
#include <unordered_map>
#include "checkers_types.h"
#include <QAtomicInt>
#include "egdb_driver/egdb/egdb_intl.h" // For egdb_interface types
#include "log.h" // For log_c and LOG_LEVEL_DEBUG/INFO

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

    int getLastEvaluationScore() const { return m_lastEvaluationScore; }
    int getLastSearchDepth() const { return m_lastSearchDepth; }
    QString getEgdbLookupResult() const { return m_egdbLookupResult; }
    uint64_t generateZobristKey(const bitboard_pos& board, int colorToMove);

public slots:
    void performTask(AI_State task, const bitboard_pos& board, int color, double maxtime);
    void performInitialization(const QString& egdbPath);
    void requestAbort();
    void searchBestMove(bitboard_pos board, int color, double maxtime);
    void clearHistory() { m_gameHistory.clear(); }
    void addHistoryKey(uint64_t key) { m_gameHistory.push_back(key); }

private slots:
    int evaluateBoard(const bitboard_pos& board, int colorToMove);
    int allKingsEval(const bitboard_pos& board) const;
    int dbWinEval(const bitboard_pos& board, int dbresult) const;
    int minimax(bitboard_pos board, int color, int depth, int alpha, int beta, CBmove* bestMove, bool isMaximizing);
    int quiescenceSearch(bitboard_pos board, int color, int alpha, int beta);
    bool isSquareAttacked(const bitboard_pos& board, int r, int c, int attackerColor);

signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void evaluationReady(int score, int depth, const QString& egdbInfo = QString());
    void initializationFinished(bool success, int maxPieces);

private:
    static bool compareMoves(const CBmove& a, const CBmove& b);
    static void initZobristKeys();

    QAtomicInt m_abortRequested;
    std::vector<uint64_t> m_gameHistory;
    std::unordered_map<uint64_t, TTEntry> m_transbitboard_positionTable;
    CBmove m_killerMoves[MAX_DEPTH][2];
    int m_historyTable[8][8][8][8];
    int m_lastEvaluationScore;
    int m_lastSearchDepth;
    QString m_egdbLookupResult;

    // Piece-Square Tables (PSTs) for evaluation
    static const int whiteManPST[32];
    static const int whiteKingPST[32];
    static const int blackManPST[32];
    static const int blackKingPST[32];

    // Zobrist Hashing
    static uint64_t ZobristTable[8][8][5];
    static uint64_t ZobristWhiteToMove;
    static bool m_zobristInitialized;
};
