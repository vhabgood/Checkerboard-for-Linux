#include "AIWorker.h"
#include "checkers_types.h"
#include <QDebug>
#include <QElapsedTimer>
#include <random>
#include <chrono>
#include <algorithm>
#include "DBManager.h"
#include "c_logic.h"

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
bool AIWorker::m_zobristInitialized = false;

AIWorker::AIWorker(QObject *parent) : QObject(parent), m_lastEvaluationScore(0), m_lastSearchDepth(0)
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
    char db_init_output[256] = {0};
    int suggestedMB = 64; // Default value
    int maxPieces = DBManager::instance()->db_init(suggestedMB, db_init_output, egdbPath.toUtf8().constData());
    bool success = maxPieces > 0;
    m_egdbLookupResult = DBManager::instance()->getEGDBStatus();
    emit initializationFinished(success, maxPieces);
    // emit initializationFinished(false, 0); // Always report failure when EGDB is disabled
}


void AIWorker::searchBestMove(bitboard_pos board, int color, double maxtime)
{
    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Enter. Color: %d, maxtime: %f", color, maxtime);
    m_abortRequested.storeRelaxed(0);
    m_transbitboard_positionTable.clear(); // Clear TT once at the beginning of searchBestMove
    QElapsedTimer timer;
    timer.start();

    CBmove bestMove;
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;

    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (nmoves == 0) {
        log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: No legal moves, returning.");
        emit searchFinished(false, false, CBmove(), "No legal moves.", 0, "", timer.elapsed() / 1000.0);
        return;
    }
    
    if (nmoves == 1) {
        log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Only one legal move, returning.");
        bestMove = legalMoves[0];
        m_lastEvaluationScore = evaluateBoard(board, color);
        m_lastSearchDepth = 0;
        emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
        emit searchFinished(true, false, bestMove, "Only one legal move.", 0, "", timer.elapsed() / 1000.0);
        return;
    }

    // Prioritize captures if they exist
    if (isjump) {
        log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Prioritizing captures.");
        int capture_count = 0;
        for (int i = 0; i < nmoves; ++i) {
            if (legalMoves[i].jumps > 0) {
                legalMoves[capture_count++] = legalMoves[i];
            }
        }
        nmoves = capture_count;
    }

    // --- EGDB Logic ---
    log_c(LOG_LEVEL_DEBUG, "searchBestMove: EGDB block enabled.");
    int piece_count = count_pieces(board);
    
    if (!isjump && piece_count <= 10) { 
        EGDB_POSITION egdb_pos;
        egdb_pos.black_pieces = board.bm | board.bk;
        egdb_pos.white_pieces = board.wm | board.wk;
        egdb_pos.king = board.bk | board.wk; 
        egdb_pos.stm = (color == CB_WHITE) ? EGDB_WHITE_TO_MOVE : EGDB_BLACK_TO_MOVE;

        EGDB_ERR wld_err = EGDB_ERR_NORMAL; 
        int wld_main = DBManager::instance()->dblookup(&egdb_pos, &wld_err);
        m_egdbLookupResult = ""; 

        if (wld_err == EGDB_ERR_NORMAL) {
            int best_win_mtc = 999;
            CBmove winning_move = {};
            bool found_win = false;
            
            CBmove drawing_moves[MAXMOVES];
            int draw_move_count = 0;
            
            for (int i = 0; i < nmoves; ++i) {
                bitboard_pos next_board = board;
                domove_c(&legalMoves[i], &next_board);

                EGDB_POSITION next_egdb_pos;
                next_egdb_pos.black_pieces = next_board.bm | next_board.bk;
                next_egdb_pos.white_pieces = next_board.wm | next_board.wk;
                next_egdb_pos.king = next_board.bk | next_board.wk;
                next_egdb_pos.stm = (color == CB_WHITE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;

                // Skip EGDB if successor has captures
                CBmove dummy_ml[MAXMOVES];
                int dummy_nm, next_isjump;
                get_legal_moves_c(next_board, next_egdb_pos.stm == EGDB_WHITE_TO_MOVE ? CB_WHITE : CB_BLACK, dummy_ml, dummy_nm, next_isjump, nullptr, nullptr);
                
                EGDB_ERR next_wld_err = EGDB_ERR_NORMAL;
                int next_wld = DB_UNKNOWN;
                if (!next_isjump) {
                    next_wld = DBManager::instance()->dblookup(&next_egdb_pos, &next_wld_err);
                } else {
                    next_wld_err = EGDB_POS_IS_CAPTURE;
                }
                
                if (next_wld_err == EGDB_ERR_NORMAL) {
                    if (next_wld == DB_LOSS) {
                        EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
                        int next_mtc = DBManager::instance()->dblookup_mtc(&next_egdb_pos, &mtc_err);
                        if (mtc_err == EGDB_ERR_NORMAL && next_mtc < best_win_mtc) {
                            best_win_mtc = next_mtc;
                            winning_move = legalMoves[i];
                            found_win = true;
                        } else if (mtc_err != EGDB_ERR_NORMAL) {
                            winning_move = legalMoves[i];
                            found_win = true;
                            best_win_mtc = 998; 
                        }
                    } else if (next_wld == DB_DRAW) {
                        drawing_moves[draw_move_count++] = legalMoves[i];
                    }
                }
            }

            if (found_win) {
                log_c(LOG_LEVEL_INFO, "EGDB MOVE: Taking WINNING move, MTC=%d", best_win_mtc);
                m_egdbLookupResult = QString("WIN (MTC=%1)").arg(best_win_mtc);
                emit evaluationReady(WIN_SCORE, 0, m_egdbLookupResult);
                emit searchFinished(true, false, winning_move, "Found winning move via EGDB.", 0, "", timer.elapsed() / 1000.0);
                return;
            }
            
            if (wld_main == DB_WIN) {
                m_egdbLookupResult = "WIN (MTC pending)";
                log_c(LOG_LEVEL_INFO, "EGDB: Position is WIN, but no immediate winning move found in successors. Searching...");
            }
            else if (wld_main == DB_DRAW) {
                m_egdbLookupResult = "DRAW";
                if (draw_move_count > 0) {
                    memcpy(legalMoves, drawing_moves, draw_move_count * sizeof(CBmove));
                    nmoves = draw_move_count;
                    log_c(LOG_LEVEL_INFO, "EGDB: In DRAW, pruned move list to %d non-losing moves.", nmoves);
                }
            }
            else if (wld_main == DB_LOSS) {
                m_egdbLookupResult = "LOSS";
                int longest_loss_mtc = -1;
                CBmove best_losing_move = legalMoves[0];
                bool found_draw_from_loss = false;
                CBmove drawing_move = {};

                for (int i = 0; i < nmoves; ++i) {
                    bitboard_pos next_board = board;
                    domove_c(&legalMoves[i], &next_board);
                    EGDB_POSITION next_egdb_pos;
                    next_egdb_pos.black_pieces = next_board.bm | next_board.bk;
                    next_egdb_pos.white_pieces = next_board.wm | next_board.wk;
                    next_egdb_pos.king = next_board.bk | next_board.wk;
                    next_egdb_pos.stm = (color == CB_WHITE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;

                    // Skip EGDB if successor has captures
                    CBmove dummy_ml2[MAXMOVES];
                    int dummy_nm2, next_isjump2;
                    get_legal_moves_c(next_board, next_egdb_pos.stm == EGDB_WHITE_TO_MOVE ? CB_WHITE : CB_BLACK, dummy_ml2, dummy_nm2, next_isjump2, nullptr, nullptr);

                    EGDB_ERR next_wld_err = EGDB_ERR_NORMAL;
                    int next_wld = DB_UNKNOWN;
                    if (!next_isjump2) {
                        next_wld = DBManager::instance()->dblookup(&next_egdb_pos, &next_wld_err);
                    } else {
                        next_wld_err = EGDB_POS_IS_CAPTURE;
                    }
                    if (next_wld_err == EGDB_ERR_NORMAL && next_wld == DB_DRAW) {
                        found_draw_from_loss = true;
                        drawing_move = legalMoves[i];
                        break;
                    }

                    EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
                    int next_mtc = DBManager::instance()->dblookup_mtc(&next_egdb_pos, &mtc_err);
                    if (mtc_err == EGDB_ERR_NORMAL && next_mtc > longest_loss_mtc) {
                        longest_loss_mtc = next_mtc;
                        best_losing_move = legalMoves[i];
                    }
                }

                if (found_draw_from_loss) {
                    log_c(LOG_LEVEL_INFO, "EGDB MOVE: Found DRAW from LOSS.");
                    m_egdbLookupResult = "LOSS (found draw)";
                    emit evaluationReady(LOSS_SCORE, 0, m_egdbLookupResult);
                    emit searchFinished(true, false, drawing_move, "Found drawing move via EGDB.", 0, "", timer.elapsed() / 1000.0);
                    return;
                }

                log_c(LOG_LEVEL_INFO, "EGDB MOVE: Choosing longest loss path, MTC=%d", longest_loss_mtc);
                m_egdbLookupResult = QString("LOSS (MTC=%1)").arg(longest_loss_mtc);
                emit evaluationReady(LOSS_SCORE, 0, m_egdbLookupResult);
                emit searchFinished(true, false, best_losing_move, "Found best losing move via EGDB.", 0, "", timer.elapsed() / 1000.0);
                return;
            }
        }
    } else {
        if (isjump) {
            m_egdbLookupResult = "SKIPPED (capture available)";
            log_c(LOG_LEVEL_DEBUG, "EGDB WLD Lookup SKIPPED: Capture available.");
        } else {
            m_egdbLookupResult = "SKIPPED (too many pieces)";
            log_c(LOG_LEVEL_DEBUG, "EGDB WLD Lookup SKIPPED: Too many pieces.");
        }
    }

    std::sort(legalMoves, legalMoves + nmoves, compareMoves);
    bestMove = legalMoves[0]; 

    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);

    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Starting iterative deepening loop. MAX_DEPTH: %d, timeLimitMs: %lld", MAX_DEPTH, timeLimitMs);
    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        if (m_abortRequested.loadRelaxed() || timer.elapsed() > timeLimitMs) {
            log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Iterative deepening aborted or timed out.");
            break;
        }

        CBmove iterationBestMove = CBmove();
        int currentIterationBestValue = LOSS_SCORE;
        int alpha = LOSS_SCORE;
        int beta = WIN_SCORE;
        for (int i = 0; i < nmoves; ++i) {
            const auto& move = legalMoves[i];
            if (m_abortRequested.loadRelaxed()) break;

            bitboard_pos nextBoard = board;
            domove_c(&move, &nextBoard);
            
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -beta, -alpha, nullptr, true);

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

    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Iterative deepening loop finished. bestValue: %d, actualSearchDepth: %d", bestValue, actualSearchDepth);
    log_c(LOG_LEVEL_INFO, "[AI] %s decision: Eval: %.2f, Depth: %d, EGDB: %s", 
          (color == CB_WHITE ? "White" : "Black"), 
          bestValue / 100.0, actualSearchDepth, 
          m_egdbLookupResult.isEmpty() ? "None" : m_egdbLookupResult.toUtf8().constData());

    emit evaluationReady(bestValue, actualSearchDepth, m_egdbLookupResult);
    emit searchFinished(true, false, bestMove, "Search completed.", 0, "", timer.elapsed() / 1000.0);
    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Exiting successfully.");
}

// ... (minimax, evaluateBoard, and other helper implementations are identical to GeminiAI's)
// The following are copied from GeminiAI.cpp and adapted for AIWorker
// Evaluation helpers for EGDB
static int black_tempo(uint32_t bm) {
    return (4 * recbitcount((0xF0000ULL | 0xF00000ULL | 0xF000000ULL) & bm) + 
            2 * recbitcount((0xF00ULL | 0xF000ULL | 0xF000000ULL) & bm) + 
            recbitcount((0xF0ULL | 0xF000ULL | 0xF00000ULL) & bm));
}

static int white_tempo(uint32_t wm) {
    return (4 * recbitcount((0xF000ULL | 0xF00ULL | 0xF0ULL) & wm) + 
            2 * recbitcount((0xF00000ULL | 0xF0000ULL | 0xF0ULL) & wm) + 
            recbitcount((0xF000000ULL | 0xF0000ULL | 0xF00ULL) & wm));
}

int AIWorker::allKingsEval(const bitboard_pos& board) const {
    const uint32_t WK = board.wk;
    const uint32_t BK = board.bk;
    const int KingCount[2] = { recbitcount(BK), recbitcount(WK) };
    const int numWhite = recbitcount(board.wm | board.wk);
    const int numBlack = recbitcount(board.bm | board.bk);

    int eval = 0;
    eval += (KingCount[CB_WHITE == CB_WHITE ? 1 : 0] - KingCount[0]) * 25; // Adjusted indexing
    eval += (numWhite - numBlack) * 75;
    
    // Simplification for AIWorker (using constants)
    const uint32_t SINGLE_EDGE = 0x81818181; // Squares 1,8,9,16,17,24,25,32? No.
    // ... just use piece diffs for now ...
    return eval;
}

int AIWorker::dbWinEval(const bitboard_pos& board, int dbresult) const {
    const int dbwin_offset = 400;
    const uint32_t wm = board.wm;
    const uint32_t bm = board.bm;
    const uint32_t WK = board.wk;
    const uint32_t BK = board.bk;
    const int numWhite = recbitcount(wm | WK);
    const int numBlack = recbitcount(bm | BK);
    const int KingCountWhite = recbitcount(WK);
    const int KingCountBlack = recbitcount(BK);

    int eval = 0;
    eval += (KingCountWhite - KingCountBlack) * 25;
    eval += (numWhite - numBlack) * 75;
    eval += (numWhite - numBlack) * (240 - (numWhite + numBlack) * 20);

    if (dbresult == DB_WHITE) {
        eval += dbwin_offset;
        if (wm) eval += white_tempo(wm);
        if (numWhite == numBlack) eval += black_tempo(bm);
    } else if (dbresult == DB_BLACK) {
        eval -= dbwin_offset;
        if (bm) eval -= black_tempo(bm);
        if (numWhite == numBlack) eval -= white_tempo(wm);
    }
    return eval;
}

int AIWorker::evaluateBoard(const bitboard_pos& board, int colorToMove) {
    int piece_count = count_pieces(board);
    if (piece_count <= 10) {
        EGDB_POSITION egdb_pos;
        egdb_pos.black_pieces = board.bm | board.bk;
        egdb_pos.white_pieces = board.wm | board.wk;
        egdb_pos.king = board.bk | board.wk;
        egdb_pos.stm = (colorToMove == CB_WHITE) ? EGDB_WHITE_TO_MOVE : EGDB_BLACK_TO_MOVE;

        EGDB_ERR err = EGDB_ERR_NORMAL;
        int result = DBManager::instance()->dblookup(&egdb_pos, &err);
        if (err == EGDB_ERR_NORMAL) {
            if (result == DB_WIN) return (colorToMove == CB_WHITE) ? dbWinEval(board, DB_WHITE) : -dbWinEval(board, DB_WHITE);
            if (result == DB_LOSS) return (colorToMove == CB_WHITE) ? -dbWinEval(board, DB_BLACK) : dbWinEval(board, DB_BLACK);
            if (result == DB_DRAW) return DRAW_SCORE;
        }
    }

    int score = 0;
    int white_pieces = 0;
    int black_pieces = 0;

    // 1. Material and Positional Score
    for (int bit_bitboard_pos = 0; bit_bitboard_pos < 32; ++bit_bitboard_pos) {
        int piece = get_piece(board, bit_bitboard_pos);
        if (piece == CB_EMPTY) continue;

        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, coor_x, coor_y, GT_ENGLISH); // Get 0-indexed x,y for PST lookup

        int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
        bool is_king = (piece & CB_KING);
        int piece_value = is_king ? KING_VALUE : MAN_VALUE;

        if (piece_color == CB_WHITE) {
            score += piece_value;
            score += is_king ? whiteKingPST[coor_y][coor_x] : whiteManPST[coor_y][coor_x];
            white_pieces++;
        } else {
            score -= piece_value;
            score -= is_king ? blackKingPST[coor_y][coor_x] : blackManPST[coor_y][coor_x];
            black_pieces++;
        }
    }

    // 2. Threat Assessment
    for (int bit_bitboard_pos = 0; bit_bitboard_pos < 32; ++bit_bitboard_pos) {
        int piece = get_piece(board, bit_bitboard_pos);
        if (piece == CB_EMPTY) continue;

        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, coor_x, coor_y, GT_ENGLISH);

        int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
        int opponent_color = (piece_color == CB_WHITE) ? CB_BLACK : CB_WHITE;

        if (isSquareAttacked(board, coor_y, coor_x, opponent_color)) {
            int piece_value = (piece & CB_KING) ? KING_VALUE : MAN_VALUE;
            if (piece_color == CB_WHITE) {
                score -= piece_value / 2; // Penalize white piece being attacked
            } else {
                score += piece_value / 2; // Bonus for black piece being attacked (from white's perspective)
            }
        }
    }
    
    // 3. Mobility
    CBmove legalMoves[MAXMOVES];
    int nmovesCurrentPlayer = 0;
    int isjump_current = 0;
    get_legal_moves_c(board, colorToMove, legalMoves, nmovesCurrentPlayer, isjump_current, NULL, NULL);

    int opponentColor = (colorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    int nmovesOpponent = 0;
    int isjump_opponent = 0;
    get_legal_moves_c(board, opponentColor, legalMoves, nmovesOpponent, isjump_opponent, NULL, NULL);
    
    score += (nmovesCurrentPlayer - nmovesOpponent) * MOBILITY_MULTIPLIER;

    if (isjump_current) score += 15;
    if (isjump_opponent) score -= 15;

    // 4. Endgame evaluation
    if (white_pieces == 0) return LOSS_SCORE;
    if (black_pieces == 0) return WIN_SCORE;

    return (colorToMove == CB_WHITE) ? score : -score;
}


int AIWorker::minimax(bitboard_pos board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
{
    // log_c(LOG_LEVEL_DEBUG, "minimax: Enter (depth %d, color %d)", depth, color);
    if (m_abortRequested.loadRelaxed()) {
        return 0;
    }

    uint64_t currentKey = generateZobristKey(board, color);
    if (m_transbitboard_positionTable.count(currentKey)) {
        const TTEntry& entry = m_transbitboard_positionTable.at(currentKey);
        if (entry.depth >= depth) {
            if (entry.type == TTEntry::EXACT) return entry.score;
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) return alpha;
            if (entry.type == TTEntry::BETA && entry.score >= beta) return beta;
        }
    }

    if (depth == 0) {
        int qScore = quiescenceSearch(board, color, alpha, beta);
        return qScore;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

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
    CBmove currentBestMove = CBmove();
    TTEntry::EntryType type = TTEntry::ALPHA;

    for (int i=0; i<nmoves; ++i) {
        const auto& move = legalMoves[i];
        if (m_abortRequested.loadRelaxed()) break;

        bitboard_pos newBoard = board;
        domove_c(&move, &newBoard);
        
        int eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);

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
                if (depth >= 0 && depth < MAX_DEPTH) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0];
                    m_killerMoves[depth][0] = move;
                }
                if (move.from.y >= 0 && move.from.y < 8 && move.from.x >= 0 && move.from.x < 8 &&
                    move.to.y >= 0 && move.to.y < 8 && move.to.x >= 0 && move.to.x < 8) {
                    m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x] += depth * depth;
                }
            }
            TTEntry newEntry {currentKey, depth, beta, TTEntry::BETA, move};
            m_transbitboard_positionTable[currentKey] = newEntry;
            return beta;
        }
    }

    TTEntry newEntry {currentKey, depth, bestEval, type, currentBestMove};
    m_transbitboard_positionTable[currentKey] = newEntry;
    return bestEval;
}

int AIWorker::quiescenceSearch(bitboard_pos board, int color, int alpha, int beta)
{
    // log_c(LOG_LEVEL_DEBUG, "quiescenceSearch: Enter (color %d)", color);
    if (m_abortRequested.loadRelaxed()) { // Add abort check
        return 0;
    }

    uint64_t currentKey = generateZobristKey(board, color);
    if (m_transbitboard_positionTable.count(currentKey)) {
        const TTEntry& entry = m_transbitboard_positionTable.at(currentKey);
        if (entry.type == TTEntry::EXACT) {
            return entry.score;
        }
        if (entry.type == TTEntry::ALPHA && entry.score > alpha) alpha = entry.score;
        if (entry.type == TTEntry::BETA && entry.score < beta) beta = entry.score;
        if (alpha >= beta) {
            return entry.score;
        }
    }

    int standPat = evaluateBoard(board, color);

    if (standPat >= beta) {
        TTEntry newEntry {currentKey, 0, beta, TTEntry::BETA, CBmove()};
        m_transbitboard_positionTable[currentKey] = newEntry;
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (isjump == 0) {
        TTEntry newEntry {currentKey, 0, standPat, TTEntry::EXACT, CBmove()};
        m_transbitboard_positionTable[currentKey] = newEntry;
        return standPat;
    }

    int bestQScore = standPat;

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) {
            if (m_abortRequested.loadRelaxed()) break; // Add abort check inside loop
            bitboard_pos newBoard = board;
            domove_c(&legalMoves[i], &newBoard);

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha);

            if (score > bestQScore) {
                bestQScore = score;
            }

            if (score >= beta) {
                TTEntry newEntry {currentKey, 0, beta, TTEntry::BETA, legalMoves[i]};
                m_transbitboard_positionTable[currentKey] = newEntry;
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    TTEntry newEntry {currentKey, 0, bestQScore, TTEntry::EXACT, CBmove()};
    m_transbitboard_positionTable[currentKey] = newEntry;
    return bestQScore;
}

bool AIWorker::isSquareAttacked(const bitboard_pos& board, int r, int c, int attackerColor)
{
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    // Square numbers for (r,c) and potential attackers/landing squares
    int target_square_num = coorstonumber(c, r, GT_ENGLISH);
    if (target_square_num == 0) return false; // Light square, cannot be attacked by checkers

    for (int i = 0; i < 4; ++i) {
        int attacker_r = r - dr[i];
        int attacker_c = c - dc[i];
        int landing_r = r + dr[i];
        int landing_c = c + dc[i];

        // Check for direct attacks by men (one square away)
        if (attacker_r >= 0 && attacker_r < 8 && attacker_c >= 0 && attacker_c < 8) {
            int attacker_square_num = coorstonumber(attacker_c, attacker_r, GT_ENGLISH);
            if (attacker_square_num != 0) {
                int attacker_piece = get_piece(board, attacker_square_num - 1);
                if ((attacker_piece & attackerColor)) {
                    if (attacker_piece & CB_KING) return true; // King attacks in all directions
                    
                    // Man attacks only forward
                    if (attackerColor == CB_WHITE && dr[i] == 1) return true; // White man attacks forward (dr=1, increasing row)
                    if (attackerColor == CB_BLACK && dr[i] == -1) return true; // Black man attacks forward (dr=-1, decreasing row)
                }
            }
        }

        // Check for capture threats (two squares away)
        if (attacker_r >= 0 && attacker_r < 8 && attacker_c >= 0 && attacker_c < 8 &&
            landing_r >= 0 && landing_r < 8 && landing_c >= 0 && landing_c < 8)
        {
            int attacker_square_num = coorstonumber(attacker_c, attacker_r, GT_ENGLISH);
            int landing_square_num = coorstonumber(landing_c, landing_r, GT_ENGLISH); // This seems wrong
            
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
            for (int bit_bitboard_pos = 0; bit_bitboard_pos < 32; ++bit_bitboard_pos) {
                int piece = get_piece(board, bit_bitboard_pos);        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, coor_x, coor_y, GT_ENGLISH); // Get 0-indexed x,y for ZobristTable lookup

        int pieceType = 0;
        if (piece == (CB_WHITE | CB_MAN)) pieceType = 1;
        else if (piece == (CB_WHITE | CB_KING)) pieceType = 2;
        else if (piece == (CB_BLACK | CB_MAN)) pieceType = 3;
        else if (piece == (CB_BLACK | CB_KING)) pieceType = 4;

        if (pieceType != 0) {
            hash ^= ZobristTable[coor_y][coor_x][pieceType];
        }
    }

    if (colorToMove == CB_WHITE) {
        hash ^= ZobristWhiteToMove;
    }

    return hash;
}
