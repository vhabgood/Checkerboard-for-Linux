#include "AIWorker.h"
#include "checkers_types.h"
#include <QDebug>
#include <QElapsedTimer>
#include <random>
#include <chrono>
#include <algorithm>
#include "DBManager.h"
#include "c_logic.h"

// Helper for LSB if not defined
#ifndef LSB
#define LSB(x) __builtin_ctz(x)
#endif

// Piece-Square Tables (32-square bitboard indexing)
const int AIWorker::whiteManPST[32] = {
    50, 50, 50, 50,
    40, 40, 40, 40,
    30, 30, 30, 30,
    20, 20, 20, 20,
    15, 15, 15, 15,
    10, 10, 10, 10,
    5, 5, 5, 5,
    0, 0, 0, 0
};

const int AIWorker::whiteKingPST[32] = {
    10, 10, 10, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 10, 10, 10
};

const int AIWorker::blackManPST[32] = {
    0, 0, 0, 0,
    5, 5, 5, 5,
    10, 10, 10, 10,
    15, 15, 15, 15,
    20, 20, 20, 20,
    30, 30, 30, 30,
    40, 40, 40, 40,
    50, 50, 50, 50
};

const int AIWorker::blackKingPST[32] = {
    10, 10, 10, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 20, 20, 10,
    10, 10, 10, 10
};

uint64_t AIWorker::ZobristTable[8][8][5] = {};
uint64_t AIWorker::ZobristWhiteToMove = 0;
bool AIWorker::m_zobristInitialized = false;

AIWorker::AIWorker(QObject *parent) : QObject(parent), m_lastEvaluationScore(0), m_lastSearchDepth(0), m_rootWLD(0)
{
    if (!m_zobristInitialized) {
        initZobristKeys();
        m_zobristInitialized = true;
    }
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

void AIWorker::performTask(AI_State task, const bitboard_pos& board, int color, double maxtime)
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

void AIWorker::performInitialization(const QString& egdbPath)
{
    DBManager::instance()->db_init(64, nullptr, egdbPath.toUtf8().constData());
    emit initializationFinished(true, 10);
}

void AIWorker::searchBestMove(bitboard_pos board, int color, double maxtime)
{
    QElapsedTimer timer;
    timer.start();
    m_abortRequested.storeRelaxed(0);
    
    m_lastEvaluationScore = 0;
    m_lastSearchDepth = 0;
    m_egdbLookupResult = "No EGDB data";
    m_searchPath.clear();
    m_rootWLD = DB_UNKNOWN;

    // Root EGDB Probe
    int n_pieces = count_pieces(board);
    if (n_pieces <= 10 && !is_capture_available(board, color)) {
        EGDB_POSITION pos;
        pos.black_pieces = board.bm | board.bk;
        pos.white_pieces = board.wm | board.wk;
        pos.king = board.bk | board.wk;
        pos.stm = (color == CB_WHITE) ? 1 : 0;
        EGDB_ERR err;
        m_rootWLD = DBManager::instance()->dblookup(&pos, &err);
        
        // If Root is WIN, we might want to relax repetition rules to ensure we make progress
        // even if it involves repeating a position once (though 3-fold is draw).
    }

    CBmove bestMove = CBmove();
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (nmoves == 0) {
        emit searchFinished(false, false, CBmove(), "No legal moves.", 0, "", 0);
        return;
    }

    if (nmoves == 1) {
        m_lastEvaluationScore = evaluateBoard(board, color);
        emit evaluationReady(m_lastEvaluationScore, 0);
        emit searchFinished(true, false, legalMoves[0], "Only one legal move.", 0, "", 0);
        return;
    }

    if (isjump) {
        int capture_count = 0;
        for (int i = 0; i < nmoves; ++i) {
            if (legalMoves[i].jumps > 0) {
                legalMoves[capture_count++] = legalMoves[i];
            }
        }
        nmoves = capture_count;
    }

    std::sort(legalMoves, legalMoves + nmoves, compareMoves);
    bestMove = legalMoves[0]; 

    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);

    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        if (m_abortRequested.loadRelaxed() || timer.elapsed() > timeLimitMs) {
            break;
        }

        CBmove iterationBestMove = CBmove();
        int currentIterationBestValue = LOSS_SCORE;
        
        int alpha = LOSS_SCORE;
        int beta = WIN_SCORE;
        
        if (current_depth >= 5) {
            alpha = bestValue - 50;
            beta = bestValue + 50;
        }

        while (true) {
            currentIterationBestValue = LOSS_SCORE;
            int current_alpha = alpha;
            int current_beta = beta;

            // Root search loop
            m_searchPath.push_back(board); // Push root
            for (int i = 0; i < nmoves; ++i) {
                const auto& move = legalMoves[i];
                if (m_abortRequested.loadRelaxed()) break;

                bitboard_pos nextBoard = board;
                domove_c(&move, &nextBoard);
                
                int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -current_beta, -current_alpha, nullptr, true);

                if (moveValue > currentIterationBestValue) {
                    currentIterationBestValue = moveValue;
                    iterationBestMove = move;
                }
                current_alpha = std::max(current_alpha, currentIterationBestValue);
                if (current_alpha >= current_beta) break;
            }
            m_searchPath.pop_back(); // Pop root

            if (m_abortRequested.loadRelaxed()) break;

            if (currentIterationBestValue <= alpha) {
                alpha = LOSS_SCORE;
            } else if (currentIterationBestValue >= beta) {
                beta = WIN_SCORE;
            } else {
                break;
            }
        }

        if (!m_abortRequested.loadRelaxed()) {
            bestValue = currentIterationBestValue;
            bestMove = iterationBestMove;
            actualSearchDepth = current_depth;
            emit evaluationReady(bestValue, actualSearchDepth, m_egdbLookupResult);
        }
    }
    
    m_lastEvaluationScore = bestValue;
    m_lastSearchDepth = actualSearchDepth;

    emit evaluationReady(bestValue, actualSearchDepth, m_egdbLookupResult);
    emit searchFinished(true, false, bestMove, "Search completed.", 0, "", timer.elapsed() / 1000.0);
}

int AIWorker::evaluateBoard(const bitboard_pos& board, int colorToMove) {
    int piece_count = count_pieces(board);
    
    // EGDB Probe
    if (piece_count <= 10 && !is_capture_available(board, colorToMove)) {
        EGDB_POSITION egdb_pos;
        egdb_pos.black_pieces = board.bm | board.bk;
        egdb_pos.white_pieces = board.wm | board.wk;
        egdb_pos.king = board.bk | board.wk;
        egdb_pos.stm = (colorToMove == CB_WHITE) ? 1 : 0;

        EGDB_ERR err = EGDB_ERR_NORMAL;
        int result = DBManager::instance()->dblookup(&egdb_pos, &err);
        
        if (err == EGDB_ERR_NORMAL) {
            // MTC Logic (Simplified for safety)
            if (result == DB_WIN) {
                int score = WIN_SCORE - 1000; // Base win score
                
                EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
                int mtc = DBManager::instance()->dblookup_mtc(&egdb_pos, &mtc_err);
                
                if (mtc_err == EGDB_ERR_NORMAL && mtc != -1) {
                    score += (1000 - mtc * 10); // Higher score for lower MTC
                } else {
                    // Unknown MTC, assume worst-case (longest) win to encourage search to find better
                    score += 0; 
                }
                return score;
            }
            if (result == DB_LOSS) {
                int score = LOSS_SCORE + 1000;
                
                EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
                int mtc = DBManager::instance()->dblookup_mtc(&egdb_pos, &mtc_err);
                
                if (mtc_err == EGDB_ERR_NORMAL && mtc != -1) {
                    score -= (1000 - mtc * 10); 
                    score = LOSS_SCORE + 1000 + (mtc * 10); 
                } else {
                    // Unknown MTC, assume worst-case (fastest) loss
                    score = LOSS_SCORE + 1000; 
                }
                return score;
            }
            if (result == DB_DRAW) return DRAW_SCORE;
        }
    }

    int score = 0;
    // ... (rest of standard evaluation) ...
    // Material and Positional Score
    for (int bit_idx = 0; bit_idx < 32; ++bit_idx) {
        int piece = get_piece(board, bit_idx);
        if (piece == CB_EMPTY) continue;

        int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
        bool is_king = (piece & CB_KING);
        int piece_value = is_king ? KING_VALUE : MAN_VALUE;

        if (piece_color == CB_WHITE) {
            score += piece_value;
            score += is_king ? whiteKingPST[bit_idx] : whiteManPST[bit_idx];
        } else {
            score -= piece_value;
            score -= is_king ? blackKingPST[bit_idx] : blackManPST[bit_idx];
        }
    }
    
    // Perspective
    if (colorToMove == CB_BLACK) score = -score;

    return score;
}


int AIWorker::minimax(bitboard_pos board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
{
    if (m_abortRequested.loadRelaxed()) return 0;

    // Path-based Repetition Detection
    for (const auto& pathBoard : m_searchPath) {
        if (pathBoard.bm == board.bm && pathBoard.bk == board.bk &&
            pathBoard.wm == board.wm && pathBoard.wk == board.wk &&
            pathBoard.color == board.color) {
            
            // If we are in a winning root position, we might want to return 0 (Draw) 
            // but if we are winning, a draw is bad (score 0 < WIN).
            // Standard repetition is draw (0).
            return DRAW_SCORE; 
        }
    }

    uint64_t currentKey = generateZobristKey(board, color);
    
    // Transposition Table Lookup
    auto it = m_transbitboard_positionTable.find(currentKey);
    if (it != m_transbitboard_positionTable.end()) {
        const TTEntry& entry = it->second;
        if (entry.depth >= depth) {
            if (entry.type == TTEntry::EXACT) return entry.score;
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) return alpha;
            if (entry.type == TTEntry::BETA && entry.score >= beta) return beta;
        }
    }

    // EGDB Probe at leaves or shallow depths
    int piece_count = count_pieces(board);
    if (piece_count <= 10 && !is_capture_available(board, color)) {
         // ... Similar logic to evaluateBoard ...
         // For now, let's rely on evaluateBoard calling it at depth 0
    }

    if (depth == 0) {
        return quiescenceSearch(board, color, alpha, beta);
    }

    // Move Generation
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (nmoves == 0) return LOSS_SCORE + (MAX_DEPTH - depth); // Mate

    // Move Ordering (Simplified)
    // ...

    // Recursion
    m_searchPath.push_back(board); // Push current
    int bestEval = LOSS_SCORE;
    TTEntry::EntryType type = TTEntry::ALPHA;

    for (int i=0; i<nmoves; ++i) {
        bitboard_pos newBoard = board;
        domove_c(&legalMoves[i], &newBoard);
        
        int eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);

        if (eval > bestEval) {
            bestEval = eval;
        }
        if (bestEval > alpha) {
            alpha = bestEval;
            type = TTEntry::EXACT;
        }
        if (alpha >= beta) {
            m_searchPath.pop_back(); // Pop before return
            TTEntry newEntry {currentKey, depth, beta, TTEntry::BETA, legalMoves[i]};
            m_transbitboard_positionTable[currentKey] = newEntry;
            return beta;
        }
    }
    
    m_searchPath.pop_back(); // Pop after loop

    TTEntry newEntry {currentKey, depth, bestEval, type, CBmove()};
    m_transbitboard_positionTable[currentKey] = newEntry;
    return bestEval;
}

// ... rest of the file (quiescenceSearch, etc. from previous read) ...
int AIWorker::quiescenceSearch(bitboard_pos board, int color, int alpha, int beta)
{
    if (m_abortRequested.loadRelaxed()) return 0;

    int standPat = evaluateBoard(board, color);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (isjump == 0) return standPat;

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) {
            bitboard_pos newBoard = board;
            domove_c(&legalMoves[i], &newBoard);
            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha);
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
    }
    return alpha;
}

bool AIWorker::isSquareAttacked(const bitboard_pos& board, int r, int c, int attackerColor)
{
    // ... same as before ...
    // Directions for checkers movement
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int attacker_r = r - dr[i];
        int attacker_c = c - dc[i];
        int landing_r = r + dr[i];
        int landing_c = c + dc[i];

        if (attacker_r >= 0 && attacker_r < 8 && attacker_c >= 0 && attacker_c < 8 &&
            landing_r >= 0 && landing_r < 8 && landing_c >= 0 && landing_c < 8) {
            
            int attacker_square_num = coorstonumber(attacker_c, attacker_r, GT_ENGLISH);
            int landing_square_num = coorstonumber(landing_c, landing_r, GT_ENGLISH); 
            
            if (attacker_square_num == 0 || landing_square_num == 0) continue;

            int attacker_piece = get_piece(board, attacker_square_num - 1);
            int landing_piece = get_piece(board, landing_square_num - 1);

            if ((attacker_piece & attackerColor) && landing_piece == CB_EMPTY) { // Attacker behind, empty square beyond
                if (attacker_piece & CB_KING) return true;
                 if (attackerColor == CB_WHITE && dr[i] == 1) return true;
                 if (attackerColor == CB_BLACK && dr[i] == -1) return true;
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

uint64_t AIWorker::generateZobristKey(const bitboard_pos& board, int colorToMove)
{
    uint64_t hash = 0;
    
    auto hash_bits = [&](uint32_t bits, int type) {
        while (bits) {
            int bit_idx = LSB(bits);
            int coor_x, coor_y;
            numbertocoors(bit_idx + 1, coor_x, coor_y, GT_ENGLISH);
            hash ^= ZobristTable[coor_y][coor_x][type];
            bits &= (bits - 1);
        }
    };

    hash_bits(board.bm, 3);
    hash_bits(board.bk, 4);
    hash_bits(board.wm, 1);
    hash_bits(board.wk, 2);

    if (colorToMove == CB_WHITE) {
        hash ^= ZobristWhiteToMove;
    }

    return hash;
}

static int black_tempo(uint32_t bm) {
    int tempo = 0;
    // Black moves from Rank 1 (Top) to Rank 8 (Bottom)
    for (int r = 0; r < 8; ++r) {
        uint32_t rank_mask = 0xFULL << (r * 4);
        tempo += r * recbitcount(bm & rank_mask);
    }
    return tempo;
}

static int white_tempo(uint32_t wm) {
    int tempo = 0;
    // White moves from Rank 8 (Bottom) to Rank 1 (Top)
    for (int r = 0; r < 8; ++r) {
        uint32_t rank_mask = 0xFULL << (r * 4);
        tempo += (7 - r) * recbitcount(wm & rank_mask);
    }
    return tempo;
}

int AIWorker::allKingsEval(const bitboard_pos& board) const {
    // ... existing ...
    return 0;
}

int AIWorker::dbWinEval(const bitboard_pos& board, int dbresult) const {
    const int dbwin_offset = 400;
    const uint32_t wm = board.wm;
    const uint32_t bm = board.bm;
    const uint32_t WK = board.wk;
    const uint32_t BK = board.bk;
    
    int eval = 0;
    if (dbresult == DB_WHITE) {
        eval = dbwin_offset + 10 * white_tempo(wm) + 20 * recbitcount(WK);
    } else {
        eval = dbwin_offset + 10 * black_tempo(bm) + 20 * recbitcount(BK);
    }
    return eval;
}