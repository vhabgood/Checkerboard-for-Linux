#include "AIWorker.h"
#include "checkers_types.h"
#include <QDebug>
#include <QElapsedTimer>
#include <random>
#include <chrono>
#include <algorithm>
#include "DBManager.h"
#include "c_logic.h"

// Piece-Square Tables (32-square bitboard indexing)
const int AIWorker::whiteManPST[32] = {
    0, 0, 0, 0,
    2, 2, 2, 2,
    2, 5, 7, 7,
    5, 2, 2, 7,
    12, 12, 7, 2,
    2, 7, 12, 12,
    7, 2, 2, 5,
    7, 7, 5, 2
};

const int AIWorker::whiteKingPST[32] = {
    0, 10, 20, 30,
    30, 20, 10, 0,
    20, 40, 60, 60,
    40, 20, 0, 30,
    60, 80, 80, 60,
    30, 0, 30, 60,
    80, 80, 60, 30,
    0, 20, 40, 60
};

const int AIWorker::blackManPST[32] = {
    2, 5, 7, 7,
    2, 7, 12, 12,
    7, 2, 2, 7,
    12, 12, 7, 2,
    2, 5, 7, 7,
    5, 2, 2, 2,
    2, 2, 0, 0,
    0, 0, 0, 0
};

const int AIWorker::blackKingPST[32] = {
    60, 40, 20, 0,
    30, 60, 80, 80,
    60, 30, 0, 30,
    60, 80, 80, 60,
    30, 0, 20, 40,
    60, 60, 40, 20,
    0, 10, 20, 30,
    30, 20, 10, 0
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
    m_egdbLookupResult = "";

    CBmove bestMove = CBmove();
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (nmoves == 0) {
        log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: No legal moves, returning.");
        emit searchFinished(false, false, CBmove(), "No legal moves.", 0, "", 0);
        return;
    }

    if (nmoves == 1) {
        log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Only one legal move, returning.");
        m_lastEvaluationScore = evaluateBoard(board, color);
        emit evaluationReady(m_lastEvaluationScore, 0);
        emit searchFinished(true, false, legalMoves[0], "Only one legal move.", 0, "", 0);
        return;
    }

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
            
            auto is_repetition = [&](const bitboard_pos& next_b, int next_c) {
                uint64_t next_key = generateZobristKey(next_b, next_c);
                for (uint64_t hKey : m_gameHistory) {
                    if (hKey == next_key) return true;
                }
                return false;
            };

            for (int i = 0; i < nmoves; ++i) {
                bitboard_pos next_board = board;
                domove_c(&legalMoves[i], &next_board);
                int next_color = (color == CB_WHITE) ? CB_BLACK : CB_WHITE;

                if (is_repetition(next_board, next_color)) {
                    continue;
                }

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
                
                for (int i=0; i<nmoves; ++i) {
                    bitboard_pos nBoard = board;
                    domove_c(&legalMoves[i], &nBoard);
                    EGDB_POSITION nPos;
                    nPos.black_pieces = nBoard.bm | nBoard.bk;
                    nPos.white_pieces = nBoard.wm | nBoard.wk;
                    nPos.king = nBoard.bk | nBoard.wk;
                    nPos.stm = (color == CB_WHITE) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;
                    
                    EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
                    int mtc = DBManager::instance()->dblookup_mtc(&nPos, &mtc_err);
                    if (mtc_err == EGDB_ERR_NORMAL && mtc > longest_loss_mtc) {
                        longest_loss_mtc = mtc;
                        best_losing_move = legalMoves[i];
                    }
                }
                
                if (longest_loss_mtc != -1) {
                    log_c(LOG_LEVEL_INFO, "EGDB MOVE: Choosing longest loss path, MTC=%d", longest_loss_mtc);
                    m_egdbLookupResult = QString("LOSS (MTC=%1)").arg(longest_loss_mtc);
                    emit evaluationReady(LOSS_SCORE, 0, m_egdbLookupResult);
                    emit searchFinished(true, false, best_losing_move, "Choosing longest losing path via EGDB.", 0, "", timer.elapsed() / 1000.0);
                    return;
                }
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
        
        // 3. Aspiration Window
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
            
            // Override EGDB text if search finds a forced win/loss (score > 90% of max)
            int absScore = (bestValue > 0) ? bestValue : -bestValue;
            if (absScore > 900000) {
                 if (bestValue > 0) m_egdbLookupResult = "WIN (Search)";
                 else m_egdbLookupResult = "LOSS (Search)";
                 log_c(LOG_LEVEL_DEBUG, "AIWorker: Overriding EGDB result to %s", m_egdbLookupResult.toUtf8().constData());
            }
            
            emit evaluationReady(bestValue, actualSearchDepth, m_egdbLookupResult);
        }
    }

    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Iterative deepening loop finished. bestValue: %d, actualSearchDepth: %d", bestValue, actualSearchDepth);
    
    m_lastEvaluationScore = bestValue;
    m_lastSearchDepth = actualSearchDepth;

    log_c(LOG_LEVEL_INFO, "[AI] %s decision: Eval: %.2f, Depth: %d, EGDB: %s", 
          (color == CB_WHITE ? "White" : "Black"), 
          bestValue / 100.0, actualSearchDepth, 
          m_egdbLookupResult.isEmpty() ? "None" : m_egdbLookupResult.toUtf8().constData());

    emit evaluationReady(bestValue, actualSearchDepth, m_egdbLookupResult);
    emit searchFinished(true, false, bestMove, "Search completed.", 0, "", timer.elapsed() / 1000.0);
    log_c(LOG_LEVEL_DEBUG, "AIWorker::searchBestMove: Exiting successfully.");
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
            if (result == DB_WIN) {
                int winner = (colorToMove == CB_WHITE) ? DB_WHITE : DB_BLACK;
                return dbWinEval(board, winner); 
            }
            if (result == DB_LOSS) {
                int winner = (colorToMove == CB_WHITE) ? DB_BLACK : DB_WHITE;
                return -dbWinEval(board, winner);
            }
            if (result == DB_DRAW) return DRAW_SCORE;
        }
    }

    int score = 0;
    int white_pieces = 0;
    int black_pieces = 0;

    // 1. Material and Positional Score
    for (int bit_idx = 0; bit_idx < 32; ++bit_idx) {
        int piece = get_piece(board, bit_idx);
        if (piece == CB_EMPTY) continue;

        int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
        bool is_king = (piece & CB_KING);
        int piece_value = is_king ? KING_VALUE : MAN_VALUE;

        if (piece_color == CB_WHITE) {
            score += piece_value;
            score += is_king ? whiteKingPST[bit_idx] : whiteManPST[bit_idx];
            white_pieces++;
            
            if (is_king) {
                // King Mobility Freedom check
                int mobility = 0;
                int dr[] = {-1, -1, 1, 1};
                int dc[] = {-1, 1, -1, 1};
                int rx, ry;
                numbertocoors(bit_idx + 1, rx, ry, GT_ENGLISH);
                for (int d = 0; d < 4; ++d) {
                    int tx = rx + dc[d], ty = ry + dr[d];
                    if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                        int sn = coorstonumber(tx, ty, GT_ENGLISH);
                        if (sn > 0 && get_piece(board, sn - 1) == CB_EMPTY) mobility++;
                    }
                }
                score += mobility * 5; // Bonus for freedom
                if (mobility <= 1) score -= 50; // Trapped penalty
            }
        } else {
            score -= piece_value;
            score -= is_king ? blackKingPST[bit_idx] : blackManPST[bit_idx];
            black_pieces++;

            if (is_king) {
                int mobility = 0;
                int dr[] = {-1, -1, 1, 1};
                int dc[] = {-1, 1, -1, 1};
                int rx, ry;
                numbertocoors(bit_idx + 1, rx, ry, GT_ENGLISH);
                for (int d = 0; d < 4; ++d) {
                    int tx = rx + dc[d], ty = ry + dr[d];
                    if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                        int sn = coorstonumber(tx, ty, GT_ENGLISH);
                        if (sn > 0 && get_piece(board, sn - 1) == CB_EMPTY) mobility++;
                    }
                }
                score -= mobility * 5; // Negative score is good for black
                if (mobility <= 1) score += 50; // Trapped penalty for black
            }
        }
    }

    // 2. Threat Assessment
    for (int bit_idx = 0; bit_idx < 32; ++bit_idx) {
        int piece = get_piece(board, bit_idx);
        if (piece == CB_EMPTY) continue;

        int coor_x, coor_y;
        numbertocoors(bit_idx + 1, coor_x, coor_y, GT_ENGLISH);

        int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
        int opponent_color = (piece_color == CB_WHITE) ? CB_BLACK : CB_WHITE;

        if (isSquareAttacked(board, coor_y, coor_x, opponent_color)) {
            int piece_value = (piece & CB_KING) ? KING_VALUE : MAN_VALUE;
            if (piece_color == CB_WHITE) {
                score -= piece_value; // FULL penalty, not half
            } else {
                score += piece_value;
            }
        }
    }
    
    /* 3. Mobility (TEMPORARILY DISABLED FOR PERFORMANCE)
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
    */

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

    // Repetition check
    for (uint64_t historyKey : m_gameHistory) {
        if (historyKey == currentKey) {
            return DRAW_SCORE;
        }
    }

    if (m_transbitboard_positionTable.count(currentKey)) {
        const TTEntry& entry = m_transbitboard_positionTable.at(currentKey);
        // CRITICAL: Stored depth must be >= current depth to use EXACT/ALPHA/BETA scores
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

    // 1. Null Move Pruning
    if (allowNull && depth >= 3 && !isSquareAttacked(board, -1, -1, (color == CB_WHITE ? CB_BLACK : CB_WHITE))) {
        int nm_eval = -minimax(board, (color == CB_WHITE ? CB_BLACK : CB_WHITE), depth - 3, -beta, -beta + 1, nullptr, false);
        if (nm_eval >= beta) return beta;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(board, color, legalMoves, nmoves, isjump, NULL, NULL);

    if (nmoves == 0) {
        // Return a score that favors delaying loss (further depth = higher score)
        return LOSS_SCORE + (MAX_DEPTH - depth);
    }
    
    std::sort(legalMoves, legalMoves + nmoves, [&](const CBmove& a, const CBmove& b){
        int scoreA = 0;
        int scoreB = 0;
        if(a.is_capture) scoreA += 100000;
        if(b.is_capture) scoreB += 100000;
        
        // Killer moves
        if (depth < MAX_DEPTH) {
            if (a.from.x == m_killerMoves[depth][0].from.x && a.from.y == m_killerMoves[depth][0].from.y &&
                a.to.x == m_killerMoves[depth][0].to.x && a.to.y == m_killerMoves[depth][0].to.y) scoreA += 50000;
            if (b.from.x == m_killerMoves[depth][0].from.x && b.from.y == m_killerMoves[depth][0].from.y &&
                b.to.x == m_killerMoves[depth][0].to.x && b.to.y == m_killerMoves[depth][0].to.y) scoreB += 50000;
            if (a.from.x == m_killerMoves[depth][1].from.x && a.from.y == m_killerMoves[depth][1].from.y &&
                a.to.x == m_killerMoves[depth][1].to.x && a.to.y == m_killerMoves[depth][1].to.y) scoreA += 40000;
            if (b.from.x == m_killerMoves[depth][1].from.x && b.from.y == m_killerMoves[depth][1].from.y &&
                b.to.x == m_killerMoves[depth][1].to.x && b.to.y == m_killerMoves[depth][1].to.y) scoreB += 40000;
        }

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
        
        int eval;
        // 2. Late Move Reductions (LMR)
        if (i >= 4 && depth >= 3 && !move.is_capture) {
            eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 2, -alpha - 1, -alpha, nullptr, true);
            if (eval > alpha) {
                eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);
            }
        } else {
            eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);
        }

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
    
    int eval = 0;
    if (dbresult == DB_WHITE) {
        eval = dbwin_offset + 10 * white_tempo(wm) + 20 * recbitcount(WK);
    } else {
        eval = dbwin_offset + 10 * black_tempo(bm) + 20 * recbitcount(BK);
    }
    return eval;
}
