#include "AIWorker.h"
#include "checkers_types.h"
#include <QDebug>
#include <QElapsedTimer>
#include <random>
#include <chrono>
#include <algorithm>
#include "GeminiAI.h"
#include "DBManager.h"

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

void AIWorker::searchBestMove(bitboard_pos board, int color, double maxtime)
{
    m_abortRequested.storeRelaxed(0);
    m_transbitboard_positionTable.clear();
    QElapsedTimer timer;
    timer.start();

    char fen_c[256];
    bitboard_postoFEN(&board, fen_c, color, GT_ENGLISH);
    qDebug() << "AIWorker::searchBestMove: Starting search for FEN: " << fen_c;

    CBmove bestMove = {0};
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;
    int initial_egdb_score = DB_UNKNOWN; // Declare and initialize here
    int mtc_score = 0; // Declare mtc_score in a higher scope

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;

    // --- DEBUG: Log FEN before getting legal moves ---
    char fen_before_moves[256];
    bitboard_postoFEN(&board, fen_before_moves, color, GT_ENGLISH);
    qDebug() << "AIWorker::searchBestMove: Board FEN before get_legal_moves_c:" << fen_before_moves;
    // --- END DEBUG ---

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

    // Prioritize captures if they exist
    if (isjump) {
        // Filter for only capture moves
        int capture_count = 0;
        for (int i = 0; i < nmoves; ++i) {
            if (legalMoves[i].jumps > 0) {
                legalMoves[capture_count++] = legalMoves[i];
            }
        }
        nmoves = capture_count;
    } else {
        // No captures, check EGDB
        int piece_count = count_pieces(&board);
        if (piece_count <= 8 && !isjump) {
            char fen_for_log[256];
            bitboard_postoFEN(&board, fen_for_log, color, GT_ENGLISH);
            char log_msg[512];
            snprintf(log_msg, sizeof(log_msg), "EGDB LOOKUP: FEN: %s", fen_for_log);
            log_c(LOG_LEVEL_INFO, log_msg);

            initial_egdb_score = DBManager::instance()->dblookup(&board, color);
            mtc_score = DBManager::instance()->dblookup_mtc(&board);
            
            const char* interpretation = "Unknown";
            if (initial_egdb_score == DB_WIN) interpretation = "Win";
            else if (initial_egdb_score == DB_LOSS) interpretation = "Loss";
            else if (initial_egdb_score == DB_DRAW) interpretation = "Draw";
            snprintf(log_msg, sizeof(log_msg), "EGDB RESULT: WLD_score=%d (Interpretation: %s), MTC_score=%d", initial_egdb_score, interpretation, mtc_score);
            log_c(LOG_LEVEL_INFO, log_msg);
            
            if (initial_egdb_score == DB_WIN || initial_egdb_score == DB_LOSS || initial_egdb_score == DB_DRAW) {
                for (int i = 0; i < nmoves; ++i) {
                    CBmove currentMove = legalMoves[i];
                    bitboard_pos next_board = board;
                    domove_c(&currentMove, &next_board);
                    int next_egdb_score = DBManager::instance()->dblookup(&next_board, (color == CB_WHITE) ? CB_BLACK : CB_WHITE);
                    
                    if (initial_egdb_score == DB_WIN && next_egdb_score == DB_LOSS) {
                        char log_msg[512];
                        snprintf(log_msg, sizeof(log_msg), "EGDB MOVE: Found winning move: %d-%d", 
                                 coorstonumber(currentMove.from.x, currentMove.from.y, GT_ENGLISH), 
                                 coorstonumber(currentMove.to.x, currentMove.to.y, GT_ENGLISH));
                        log_c(LOG_LEVEL_INFO, log_msg);
                        
                        emit searchFinished(true, false, currentMove, "Found winning move via EGDB.", 0, "", timer.elapsed() / 1000.0);
                        return;
                    }
                }
            }
        }
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

            bitboard_pos nextBoard = board;
            domove_c(&move, &nextBoard);
            
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -beta, -alpha, nullptr, true, initial_egdb_score, mtc_score);

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
int AIWorker::evaluateBoard(const bitboard_pos& board, int colorToMove, int egdb_context) {
    // If EGDB context is available and definitive, use it directly
    if (egdb_context == DB_WIN) {
        return WIN_SCORE - 1; // Return a score just below the absolute max for WIN
    }
    if (egdb_context == DB_LOSS) {
        return LOSS_SCORE + 1; // Return a score just above the absolute min for LOSS
    }
    if (egdb_context == DB_DRAW) {
        // For a draw, the heuristic evaluation can still be useful to find a "better" draw
        // or one that puts more pressure, so we let the rest of the function run.
        // We could return 0 here, but it would remove subtlety.
    }

    int score = 0;
    int white_pieces = 0;
    int black_pieces = 0;

    // 1. Material and Positional Score
    for (int bit_bitboard_pos = 0; bit_bitboard_pos < 32; ++bit_bitboard_pos) {
        int piece = get_piece(&board, bit_bitboard_pos);
        if (piece == CB_EMPTY) continue;

        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, &coor_x, &coor_y, GT_ENGLISH); // Get 0-indexed x,y for PST lookup

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
        int piece = get_piece(&board, bit_bitboard_pos);
        if (piece == CB_EMPTY) continue;

        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, &coor_x, &coor_y, GT_ENGLISH);

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


int AIWorker::minimax(bitboard_pos board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull, int egdb_context, int mtc_score)
{
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

    // Check MTC score from Endgame Database (MTC)
    if (mtc_score != MTC_UNKNOWN_VALUE) {
        if (mtc_score > 0 && mtc_score <= MTC_MAX_MOVES_TO_CONSIDER) {
            // This is a forced win for the current player, with 'mtc_score' moves to convert.
            // A smaller mtc_score means a quicker win, so a higher evaluation.
            // We scale this to fit within our score range, and ensure it beats regular search but isn't max WIN_SCORE
            return MTC_WIN_VALUE_BASE - mtc_score;
        } 
        // If mtc_score indicates a loss for the current side (e.g., opponent wins in N moves, MTC value could be negative or
        // represent a win for the other side when viewed from the current side's perspective)
        // For simplicity with current dblookup_mtc that returns 0 for unknown/loss, we might handle losses differently or rely on WLD.
        // Assuming positive mtc_score means current player to move wins.
    }

    if (depth == 0) {
        return quiescenceSearch(board, color, alpha, beta, egdb_context, mtc_score);
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
        bitboard_pos newBoard = board;
        domove_c(&move, &newBoard);
        
        int eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true, egdb_context, mtc_score);

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
            m_transbitboard_positionTable[currentKey] = newEntry;
            return beta;
        }
    }

    TTEntry newEntry {currentKey, depth, bestEval, type, currentBestMove};
    m_transbitboard_positionTable[currentKey] = newEntry;

    return bestEval;
}

int AIWorker::quiescenceSearch(bitboard_pos board, int color, int alpha, int beta, int egdb_context, int mtc_score)
{
    // Check MTC score from Endgame Database (MTC)
    if (mtc_score != MTC_UNKNOWN_VALUE) {
        if (mtc_score > 0 && mtc_score <= MTC_MAX_MOVES_TO_CONSIDER) {
            return MTC_WIN_VALUE_BASE - mtc_score;
        }
    }
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
            bitboard_pos newBoard = board;
            domove_c(&legalMoves[i], &newBoard);

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha, egdb_context, mtc_score);

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
                int attacker_piece = get_piece(&board, attacker_square_num - 1);
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
            int landing_square_num = coorstonumber(landing_c, landing_c, GT_ENGLISH); // This seems wrong
            
            if (attacker_square_num == 0 || landing_square_num == 0) continue;

            int attacker_piece = get_piece(&board, attacker_square_num - 1);
            int landing_piece = get_piece(&board, landing_square_num - 1);

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
        int piece = get_piece(&board, bit_bitboard_pos);
        int coor_x, coor_y;
        numbertocoors(bit_bitboard_pos + 1, &coor_x, &coor_y, GT_ENGLISH); // Get 0-indexed x,y for ZobristTable lookup

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
