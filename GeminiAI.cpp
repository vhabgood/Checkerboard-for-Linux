#include <QDebug>
#include "GeminiAI.h"
#include <limits>
#include <QElapsedTimer>
#include "GameManager.h"

extern "C" {
#include "checkers_c_types.h"
#include "c_logic.h"
#include "dblookup.h"
}


const int GeminiAI::whiteManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::whiteKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::blackManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  7, 12, 12,  7,  2,  0},
    { 0,  2,  5,  7,  7,  5,  2,  0},
    { 0,  0,  2,  2,  2,  2,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::blackKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 15, 30, 40, 40, 30, 15,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};


// Static member initialization for ZobristTable
uint64_t GeminiAI::ZobristTable[8][8][5] = {}; // Initialize with zeros
uint64_t GeminiAI::ZobristWhiteToMove = 0; // Initialize with zero

void GeminiAI::initZobristKeys() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count()); // Seed with current time
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 5; ++k) { // 0: empty, 1: white man, 2: white king, 3: black man, 4: black king
                ZobristTable[i][j][k] = rng();
            }
        }
    }
    ZobristWhiteToMove = rng(); // Initialize the white to move hash
}

void GeminiAI::init()
{
    // Initialize EGDB
    qInfo() << "GeminiAI: Initializing EGDB with path: " << m_egdbPath;
    char db_init_msg_buffer[256]; // Declare a buffer for db_init messages
    db_init_msg_buffer[0] = '\0'; // Ensure it's null-terminated initially
    int initializedPieces = db_init(256, db_init_msg_buffer, m_egdbPath.toUtf8().constData());
    if (initializedPieces > 0) { // Check if any databases were initialized
        m_egdbInitialized = true;
        m_maxEGDBPieces = initializedPieces;
        qInfo() << "GeminiAI: EGDB initialized successfully. Max pieces: " << m_maxEGDBPieces << " Message: " << db_init_msg_buffer;
    } else {
        m_egdbInitialized = false;
        m_maxEGDBPieces = 0;
        qWarning() << "GeminiAI: Failed to initialize EGDB with path: " << m_egdbPath << ". EGDB will be disabled. Message: " << db_init_msg_buffer;
    }
}

GeminiAI::GeminiAI(const QString& egdbPath, QObject *parent) : QObject(parent),
    m_egdbPath(egdbPath),
    m_lastEvaluationScore(0), // Initialize the new member
    m_lastSearchDepth(0), // Initialize the new member
    m_egdbInitialized(false), // Initialize EGDB status
    m_maxEGDBPieces(0), // Initialize max EGDB pieces
    m_mode(Idle), // Initialize AI mode
    m_handicap(0), // Initialize handicap
    m_primaryExternalEngine(nullptr),
    m_secondaryExternalEngine(nullptr),
    m_useExternalEngine(false),
    m_externalEnginePath(""),
    m_secondaryExternalEnginePath("")
{
    // Call the static Zobrist key initialization once
    initZobristKeys();

    // Initialize killer moves
    memset(m_killerMoves, 0, sizeof(m_killerMoves));

    // Initialize history table
    memset(m_historyTable, 0, sizeof(m_historyTable));
}

GeminiAI::~GeminiAI()
{
    if (m_primaryExternalEngine) {
        delete m_primaryExternalEngine;
        m_primaryExternalEngine = nullptr;
    }
    if (m_secondaryExternalEngine) {
        delete m_secondaryExternalEngine;
        m_secondaryExternalEngine = nullptr;
    }
    db_exit();
}

void GeminiAI::requestAbort()
{
    m_abortRequested.storeRelaxed(1);
}

void GeminiAI::setMode(AI_State mode)
{
    m_mode = mode;
    qDebug() << "GeminiAI: Mode set to " << mode;
}

void GeminiAI::setHandicap(int handicap)
{
    m_handicap = handicap;
    qDebug() << "GeminiAI: Handicap set to " << handicap;
}

void GeminiAI::setOptions(const CBoptions& options)
{
    this->m_options = options;
    qDebug() << "GeminiAI: Options set. White Player Type: " << this->m_options.white_player_type << ", Black Player Type: " << this->m_options.black_player_type;
}

bool GeminiAI::sendCommand(const QString& command, QString& reply)
{
    if (m_useExternalEngine && m_primaryExternalEngine) {
        return m_primaryExternalEngine->sendCommand(command, reply);
    } else {
        qDebug() << "GeminiAI: sendCommand called with: " << command << " (internal AI, no action taken)";
        reply = "Internal AI: Command received, but no external engine is active.";
        return true;
    }
}

void GeminiAI::addMoveToUserBook(const Board8x8& board, const CBmove& move)
{
    Q_UNUSED(board);
    Q_UNUSED(move);
}

void GeminiAI::deleteCurrentEntry()
{
}

void GeminiAI::deleteAllEntriesFromUserBook()
{
}

void GeminiAI::navigateToNextEntry()
{
}

void GeminiAI::navigateToPreviousEntry()
{
}

void GeminiAI::resetNavigation()
{
}

void GeminiAI::loadUserBook(const QString& filename)
{
    Q_UNUSED(filename);
}

void GeminiAI::saveUserBook(const QString& filename)
{
    Q_UNUSED(filename);
}

void GeminiAI::setExternalEnginePath(const QString& path)
{
    m_externalEnginePath = path;
}

void GeminiAI::setSecondaryExternalEnginePath(const QString& path)
{
    m_secondaryExternalEnginePath = path;
}

void GeminiAI::setEgdbPath(const QString& path)
{
    qInfo() << "GeminiAI: EGDB path set to: " << path;
    if (m_egdbInitialized) {
        db_exit(); // Close existing EGDB before reinitializing
    }
    m_egdbPath = path;
    init(); // Reinitialize EGDB with the new path
}

void GeminiAI::requestMove(Board8x8 board, int colorToMove, double timeLimit)
{
    qDebug() << "GeminiAI: requestMove called. Color: " << colorToMove << ", Time Limit: " << timeLimit;

    if (m_useExternalEngine && m_primaryExternalEngine) {
        // Convert Board8x8 to FEN
        char fen_c[256];
        board8toFEN(&board, fen_c, colorToMove, GT_ENGLISH); // Assuming GT_ENGLISH for now
        QString fenString = QString::fromUtf8(fen_c);

        QString reply;
        // Send position command
        if (!m_primaryExternalEngine->sendCommand(QString("position fen %1").arg(fenString), reply)) {
            emit engineError("Failed to set position on external engine.");
            emit searchFinished(false, true, {0}, "Engine Error", 0, "", 0.0);
            return;
        }

        // Send go command
        if (!m_primaryExternalEngine->sendCommand(QString("go movetime %1").arg(static_cast<int>(timeLimit * 1000)), reply)) {
            emit engineError("Failed to request move from external engine.");
            emit searchFinished(false, true, {0}, "Engine Error", 0, "", 0.0);
            return;
        }

        // Parse reply for bestmove
        // The reply should contain "bestmove <move>"
        QString bestMoveStr;
        QStringList lines = reply.split("\n");
        for (const QString& line : lines) {
            if (line.startsWith("bestmove")) {
                bestMoveStr = line.section("bestmove ", 1).section(" ", 0, 0);
                break;
            }
        }

        if (!bestMoveStr.isEmpty()) {
            // Convert bestMoveStr (e.g., "11-15") to CBmove
            CBmove bestMove = {0};
            int from_square, to_square;
            // Assuming bestMoveStr is in format "from-to" or "fromxfromy-tox_toy"
            // For now, let's assume "from-to" (e.g., "11-15")
            QStringList moveParts = bestMoveStr.split("-");
            if (moveParts.size() == 2) {
                from_square = moveParts[0].toInt();
                to_square = moveParts[1].toInt();
                numbertocoors(from_square, &bestMove.from.x, &bestMove.from.y, GT_ENGLISH);
                numbertocoors(to_square, &bestMove.to.x, &bestMove.to.y, GT_ENGLISH);
                // Need to determine is_capture and jumps by re-generating legal moves
                CBmove legalMoves[MAXMOVES];
                int nmoves = 0;
                int isjump = 0;
                bool dummy_can_continue_multijump = false;
                get_legal_moves_c(&board, colorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

                for (int i = 0; i < nmoves; ++i) {
                    if (legalMoves[i].from.x == bestMove.from.x &&
                        legalMoves[i].from.y == bestMove.from.y &&
                        legalMoves[i].to.x == bestMove.to.x &&
                        legalMoves[i].to.y == bestMove.to.y) {
                        bestMove = legalMoves[i]; // Use the full move data
                        break;
                    }
                }
            } else {
                qWarning() << "GeminiAI: Could not parse bestmove string: " << bestMoveStr;
                emit searchFinished(false, true, {0}, "Engine Error: Invalid move format.", 0, "", 0.0);
                return;
            }

            emit searchFinished(true, false, bestMove, "External engine move found.", 0, "", 0.0);
        } else {
            emit engineError("External engine did not return a bestmove.");
            emit searchFinished(false, true, {0}, "Engine Error", 0, "", 0.0);
        }
    } else {
        // Fallback to internal AI
        CBmove bestMove = getBestMove(board, colorToMove, timeLimit);
        emit searchFinished(true, false, bestMove, "Internal AI move found.", 0, "", 0.0);
    }
}

void GeminiAI::doWork()
{
}

void GeminiAI::initEngineProcess()
{
    if (!m_externalEnginePath.isEmpty()) {
        m_primaryExternalEngine = new ExternalEngine(m_externalEnginePath, this);
        if (m_primaryExternalEngine->startEngine()) {
            qInfo() << "GeminiAI: Primary external engine started: " << m_externalEnginePath;
            m_useExternalEngine = true;
        } else {
            qCritical() << "GeminiAI: Failed to start primary external engine: " << m_externalEnginePath;
            m_useExternalEngine = false;
        }
    }
    if (!m_secondaryExternalEnginePath.isEmpty()) {
        m_secondaryExternalEngine = new ExternalEngine(m_secondaryExternalEnginePath, this);
        if (m_secondaryExternalEngine->startEngine()) {
            qInfo() << "GeminiAI: Secondary external engine started: " << m_secondaryExternalEnginePath;
        } else {
            qCritical() << "GeminiAI: Failed to start secondary external engine: " << m_secondaryExternalEnginePath;
        }
    }
}

void GeminiAI::quitEngineProcess()
{
    if (m_primaryExternalEngine) {
        m_primaryExternalEngine->stopEngine();
        delete m_primaryExternalEngine;
        m_primaryExternalEngine = nullptr;
    }
    if (m_secondaryExternalEngine) {
        m_secondaryExternalEngine->stopEngine();
        delete m_secondaryExternalEngine;
        m_secondaryExternalEngine = nullptr;
    }
    m_useExternalEngine = false;
}

void GeminiAI::startAnalyzeGame(const Board8x8& board, int colorToMove)
{
    double analysisTimeLimit = 10.0; // 10 seconds for analysis
    getBestMove(board, colorToMove, analysisTimeLimit);
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
}

void GeminiAI::startAutoplay(const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
}

void GeminiAI::startEngineMatch(int numGames, const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    Q_UNUSED(numGames);
}

void GeminiAI::startRunTestSet(const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
}

void GeminiAI::startAnalyzePdn(const Board8x8& board, int colorToMove)
{
    double analysisTimeLimit = 10.0; // 10 seconds for analysis
    getBestMove(board, colorToMove, analysisTimeLimit);
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
}

void GeminiAI::abortSearch()
{
    requestAbort();
}

bool GeminiAI::compareMoves(const CBmove& a, const CBmove& b)
{
    // Prioritize captures over non-captures
    if (a.is_capture && !b.is_capture) {
        return true;
    }
    if (!a.is_capture && b.is_capture) {
        return false;
    }

    // If both are captures, prioritize multi-jumps (more jumps is better)
    if (a.is_capture && b.is_capture) {
        return a.jumps > b.jumps;
    }

    return false;
}

uint64_t GeminiAI::generateZobristKey(const Board8x8& board, int colorToMove)
{
    uint64_t hash = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            int pieceType = 0; // 0: empty

            if (piece == (CB_WHITE | CB_MAN)) {
                pieceType = 1;
            } else if (piece == (CB_WHITE | CB_KING)) {
                pieceType = 2;
            } else if (piece == (CB_BLACK | CB_MAN)) {
                pieceType = 3;
            } else if (piece == (CB_BLACK | CB_KING)) {
                pieceType = 4;
            }

            hash ^= ZobristTable[r][c][pieceType];
        }
    }

    if (colorToMove == CB_WHITE) {
        hash ^= ZobristWhiteToMove;
    }

    return hash;
}

CBmove GeminiAI::getBestMove(Board8x8 board, int color, double maxtime)
{
    m_abortRequested.storeRelaxed(0);
    m_transpositionTable.clear();

    char fen_c[256];
    board8toFEN(&board, fen_c, color, GT_ENGLISH);
    qDebug() << "GeminiAI::getBestMove: Entering with maxtime: " << maxtime << ", color: " << (color == CB_WHITE ? "WHITE" : "BLACK") << ", FEN: " << fen_c;

    CBmove bestMove = {0};
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0;

    QElapsedTimer timer;
    timer.start();

    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);

    // Get legal moves
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    if (nmoves == 0) {
        m_lastEvaluationScore = LOSS_SCORE;
        m_lastSearchDepth = 0;
        qWarning() << "GeminiAI::getBestMove: No legal moves at root.";
        return {0};
    }
    
    if (nmoves == 1) {
        qInfo() << "GeminiAI::getBestMove: Only one legal move, returning it immediately.";
        m_lastEvaluationScore = evaluateBoard(board, color);
        m_lastSearchDepth = 0;
        return legalMoves[0];
    }
    
    // Sort moves to prioritize captures
    std::sort(legalMoves, legalMoves + nmoves, compareMoves);
    bestMove = legalMoves[0]; // Initialize with the first legal move

    // Iterative Deepening
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

    m_lastEvaluationScore = bestValue;
    m_lastSearchDepth = actualSearchDepth;
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
    qInfo() << "--- getBestMove: End ---";
    qInfo() << "Final best move: from(" << bestMove.from.x << "," << bestMove.from.y << ") to(" << bestMove.to.x << "," << bestMove.to.y << "), score: " << m_lastEvaluationScore << ", depth: " << m_lastSearchDepth;
    return bestMove;
}

int GeminiAI::evaluateBoard(const Board8x8& board, int colorToMove) {
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
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, colorToMove, legalMoves, &nmovesCurrentPlayer, &isjump_current, NULL, &dummy_can_continue_multijump);

    int opponentColor = (colorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    int nmovesOpponent = 0;
    int isjump_opponent = 0;
    get_legal_moves_c(&board, opponentColor, legalMoves, &nmovesOpponent, &isjump_opponent, NULL, &dummy_can_continue_multijump);
    
    score += (nmovesCurrentPlayer - nmovesOpponent) * MOBILITY_MULTIPLIER;

    if (isjump_current) score += 15;
    if (isjump_opponent) score -= 15;


    // 4. Endgame evaluation
    if (white_pieces == 0) return LOSS_SCORE;
    if (black_pieces == 0) return WIN_SCORE;

    return (colorToMove == CB_WHITE) ? score : -score;
}


int GeminiAI::minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
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
        return quiescenceSearch(board, color, alpha, beta);
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    if (nmoves == 0) {
        return LOSS_SCORE + depth;
    }
    
    // Move ordering
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

int GeminiAI::quiescenceSearch(Board8x8 board, int color, int alpha, int beta)
{
    int standPat = evaluateBoard(board, color);

    if (standPat >= beta) {
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    if (isjump == 0) {
        return standPat;
    }

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) {
            Board8x8 newBoard = board;
            domove_c(&legalMoves[i], &newBoard);

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha);

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

bool GeminiAI::hasCaptures(const Board8x8& board, int colorToMove)
{
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, colorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    return isjump == 1;
}

bool GeminiAI::isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor)
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

    // Check for simple non-capture threats
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

pos GeminiAI::boardToPosition(const Board8x8& board, int colorToMove)
{
    pos p;
    pos bitboard_pos;
    boardtobitboard(&board, &bitboard_pos);

    p.bm = bitboard_pos.bm;
    p.bk = bitboard_pos.bk;
    p.wm = bitboard_pos.wm;
    p.wk = bitboard_pos.wk;
    p.color = (colorToMove == CB_WHITE) ? DB_WHITE : DB_BLACK;

    return p;
}