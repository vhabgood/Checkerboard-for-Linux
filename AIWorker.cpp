#include "AIWorker.h"
#include <QDebug>
#include <QElapsedTimer>
#include <random>
#include <chrono>
#include <algorithm>
#include "GeminiAI.h"

extern "C" {
#include "c_logic.h"
}

// Piece-Square Tables and other constants from GeminiAI
const int AIWorker::whiteManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int AIWorker::whiteKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int AIWorker::blackManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int AIWorker::blackKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

uint64_t AIWorker::ZobristTable[8][8][5] = {};
uint64_t AIWorker::ZobristWhiteToMove = 0;

AIWorker::AIWorker(QObject *parent) : QObject(parent), m_lastEvaluationScore(0), m_lastSearchDepth(0)
{
    initZobristKeys();
    memset(m_killerMoves, 0, sizeof(m_killerMoves));
    memset(m_historyTable, 0, sizeof(m_historyTable));
}

AIWorker::~AIWorker()
{
}

void AIWorker::initZobristKeys() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 5; ++k) {
                ZobristTable[i][j][k] = rng();
            }
        }
    }
    ZobristWhiteToMove = rng();
}

void AIWorker::requestAbort()
{
    m_abortRequested.storeRelaxed(1);
}

void AIWorker::performTask(AI_State task, const Board8x8& board, int color, double maxtime)
{
    switch(task) {
        case Autoplay:
        case AnalyzeGame:
        case RunTestSet:
        case AnalyzePdn:
        default:
            searchBestMove(board, color, maxtime);
            break;
    }
}

void AIWorker::searchBestMove(Board8x8 board, int color, double maxtime)
{
    m_abortRequested.storeRelaxed(0);
    m_transpositionTable.clear();
    QElapsedTimer timer;
    timer.start();

    char fen_c[256];
    board8toFEN(&board, fen_c, color, GT_ENGLISH);
    qDebug() << "AIWorker::searchBestMove: Starting search for FEN: " << fen_c;

    CBmove bestMove = {0};
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, NULL);

    if (nmoves == 0) {
        emit searchFinished(false, false, {0}, "No legal moves.", 0, "", timer.elapsed() / 1000.0);
        return;
    }
    
    if (nmoves == 1) {
        bestMove = legalMoves[0];
        m_lastEvaluationScore = evaluateBoard(board, color, DB_UNKNOWN);
        m_lastSearchDepth = 0;
        emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
        emit searchFinished(true, false, bestMove, "Only one legal move.", 0, "", timer.elapsed() / 1000.0);
        return;
    }

    std::sort(legalMoves, legalMoves + nmoves, compareMoves);
    bestMove = legalMoves[0]; 

    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);

    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        if (m_abortRequested.loadRelaxed() || timer.elapsed() > timeLimitMs) {
            break;
        }

        CBmove iterationBestMove = {0};
        int currentIterationBestValue = LOSS_SCORE;
        int alpha = LOSS_SCORE;
        int beta = WIN_SCORE;

        for (int i = 0; i < nmoves; ++i) {
            const auto& move = legalMoves[i];
            if (m_abortRequested.loadRelaxed()) break;

            Board8x8 nextBoard = board;
            domove_c(&move, &nextBoard);
            
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -beta, -alpha, nullptr, true, DB_UNKNOWN);

            if (moveValue > currentIterationBestValue) {
                currentIterationBestValue = moveValue;
                iterationBestMove = move;
            }
            alpha = std::max(alpha, currentIterationBestValue);
        }

        if (!m_abortRequested.loadRelaxed()) {
            bestValue = currentIterationBestValue;
            bestMove = iterationBestMove;
            actualSearchDepth = current_depth;
        }
    }

    m_lastEvaluationScore = bestValue;
    m_lastSearchDepth = actualSearchDepth;
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
    
    bool aborted = m_abortRequested.loadRelaxed();
    emit searchFinished(true, aborted, bestMove, "Search complete.", 0, "", timer.elapsed() / 1000.0);
}

// ... (minimax, evaluateBoard, and other helper implementations are identical to GeminiAI's)
// The following are copied from GeminiAI.cpp and adapted for AIWorker
int AIWorker::evaluateBoard(const Board8x8& board, int colorToMove, int egdb_context) {
    int score = 0;
    int white_pieces = 0;
    int black_pieces = 0;

    // 1. Material and Positional Score
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            bool is_king = (piece & CB_KING);
            int piece_value = is_king ? KING_VALUE : MAN_VALUE;

            if (piece_color == CB_WHITE) {
                score += piece_value;
                score += is_king ? whiteKingPST[r][c] : whiteManPST[r][c];
                white_pieces++;
            } else {
                score -= piece_value;
                score -= is_king ? blackKingPST[r][c] : blackManPST[r][c];
                black_pieces++;
            }
        }
    }

    // 2. Threat Assessment
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            int opponent_color = (piece_color == CB_WHITE) ? CB_BLACK : CB_WHITE;

            if (isSquareAttacked(board, r, c, opponent_color)) {
                int piece_value = (piece & CB_KING) ? KING_VALUE : MAN_VALUE;
                if (piece_color == CB_WHITE) {
                    score -= piece_value / 2; // Penalize white piece being attacked
                } else {
                    score += piece_value / 2; // Bonus for black piece being attacked (from white's perspective)
                }
            }
        }
    }
    
    // 3. Mobility
    CBmove legalMoves[MAXMOVES];
    int nmovesCurrentPlayer = 0;
    int isjump_current = 0;
    get_legal_moves_c(&board, colorToMove, legalMoves, &nmovesCurrentPlayer, &isjump_current, NULL, NULL);

    int opponentColor = (colorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    int nmovesOpponent = 0;
    int isjump_opponent = 0;
    get_legal_moves_c(&board, opponentColor, legalMoves, &nmovesOpponent, &isjump_opponent, NULL, NULL);
    
    score += (nmovesCurrentPlayer - nmovesOpponent) * MOBILITY_MULTIPLIER;

    if (isjump_current) score += 15;
    if (isjump_opponent) score -= 15;

    // 4. Endgame evaluation
    if (white_pieces == 0) return LOSS_SCORE;
    if (black_pieces == 0) return WIN_SCORE;

    return (colorToMove == CB_WHITE) ? score : -score;
}


int AIWorker::minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull, int egdb_context)
{
    if (m_abortRequested.loadRelaxed()) {
        return 0;
    }

    uint64_t currentKey = generateZobristKey(board, color);
    if (m_transpositionTable.count(currentKey)) {
        const TTEntry& entry = m_transpositionTable.at(currentKey);
        if (entry.depth >= depth) {
            if (entry.type == TTEntry::EXACT) return entry.score;
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) return alpha;
            if (entry.type == TTEntry::BETA && entry.score >= beta) return beta;
        }
    }

    if (depth == 0) {
        return quiescenceSearch(board, color, alpha, beta, egdb_context);
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, NULL);

    if (nmoves == 0) {
        return LOSS_SCORE + depth;
    }
    
    std::sort(legalMoves, legalMoves + nmoves, [&](const CBmove& a, const CBmove& b){
        int scoreA = 0;
        int scoreB = 0;
        if(a.is_capture) scoreA += 10000;
        if(b.is_capture) scoreB += 10000;
        scoreA += m_historyTable[a.from.y][a.from.x][a.to.y][a.to.x];
        scoreB += m_historyTable[b.from.y][b.from.x][b.to.y][b.to.x];
        return scoreA > scoreB;
    });

    int bestEval = LOSS_SCORE;
    CBmove currentBestMove = {0};
    TTEntry::EntryType type = TTEntry::ALPHA;

    for (int i=0; i<nmoves; ++i) {
        const auto& move = legalMoves[i];
        Board8x8 newBoard = board;
        domove_c(&move, &newBoard);
        
        int eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true, egdb_context);

        if (eval > bestEval) {
            bestEval = eval;
            currentBestMove = move;
        }

        if (bestEval > alpha) {
            alpha = bestEval;
            type = TTEntry::EXACT;
        }

        if (alpha >= beta) {
            if(!isjump) {
                m_killerMoves[depth][1] = m_killerMoves[depth][0];
                m_killerMoves[depth][0] = move;
                m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x] += depth * depth;
            }
            TTEntry newEntry {currentKey, depth, beta, TTEntry::BETA, move};
            m_transpositionTable[currentKey] = newEntry;
            return beta;
        }
    }

    TTEntry newEntry {currentKey, depth, bestEval, type, currentBestMove};
    m_transpositionTable[currentKey] = newEntry;

    return bestEval;
}

int AIWorker::quiescenceSearch(Board8x8 board, int color, int alpha, int beta, int egdb_context)
{
    int standPat = evaluateBoard(board, color, egdb_context);

    if (standPat >= beta) {
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, NULL);

    if (isjump == 0) {
        return standPat;
    }

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) {
            Board8x8 newBoard = board;
            domove_c(&legalMoves[i], &newBoard);

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha, egdb_context);

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    return alpha;
}

bool AIWorker::isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor)
{
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int attacker_r = r - dr[i];
        int attacker_c = c - dc[i];
        int landing_r = r + dr[i];
        int landing_c = c + dc[i];

        if (attacker_r >= 0 && attacker_r < 8 && attacker_c >= 0 && attacker_c < 8 &&
            landing_r >= 0 && landing_r < 8 && landing_c >= 0 && landing_c < 8)
        {
            int attacker_piece = board.board[attacker_r][attacker_c];
            if (board.board[landing_r][landing_c] == CB_EMPTY && (attacker_piece & attackerColor)) {
                if (attacker_piece & CB_KING) return true;
                if (attackerColor == CB_WHITE && landing_r > attacker_r) return true;
                if (attackerColor == CB_BLACK && landing_r < attacker_r) return true;
            }
        }
    }

    int forward = (attackerColor == CB_BLACK) ? 1 : -1;
    for(int i = 2; i<4; ++i) {
         int threatening_r = r + forward;
         int threatening_c = c + dc[i];
         if(threatening_r >= 0 && threatening_r < 8 && threatening_c >= 0 && threatening_c < 8) {
             int threatening_piece = board.board[threatening_r][threatening_c];
             if((threatening_piece & attackerColor) && !(threatening_piece & CB_KING)) {
                 return true;
             }
         }
    }

    return false;
}

bool AIWorker::compareMoves(const CBmove& a, const CBmove& b)
{
    if (a.is_capture && !b.is_capture) return true;
    if (!a.is_capture && b.is_capture) return false;
    if (a.is_capture && b.is_capture) return a.jumps > b.jumps;
    return false;
}

uint64_t AIWorker::generateZobristKey(const Board8x8& board, int colorToMove)
{
    uint64_t hash = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            int pieceType = 0;

            if (piece == (CB_WHITE | CB_MAN)) pieceType = 1;
            else if (piece == (CB_WHITE | CB_KING)) pieceType = 2;
            else if (piece == (CB_BLACK | CB_MAN)) pieceType = 3;
            else if (piece == (CB_BLACK | CB_KING)) pieceType = 4;

            hash ^= ZobristTable[r][c][pieceType];
        }
    }

    if (colorToMove == CB_WHITE) {
        hash ^= ZobristWhiteToMove;
    }

    return hash;
}
