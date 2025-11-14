#include "GeminiAI.h"
#include "c_logic.h"
#include <limits>
#include <QElapsedTimer> // Added for QElapsedTimer
#include "GameManager.h" // Added to resolve compilation errors for GameManager::log

extern "C" {
#include "dblookup.h" // Re-explicitly include dblookup.h
}
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include

const int GeminiAI::whiteManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  5,  5,  5,  5,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  5, 15, 25, 25, 15,  5,  0},
    { 0,  5, 15, 25, 25, 15,  5,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  5,  5,  5,  5,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::whiteKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 20, 40, 60, 60, 40, 20,  0},
    { 0, 30, 60, 80, 80, 60, 30,  0},
    { 0, 30, 60, 80, 80, 60, 30,  0},
    { 0, 20, 40, 60, 60, 40, 20,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::blackManPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  5,  5,  5,  5,  0,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  5, 15, 25, 25, 15,  5,  0},
    { 0,  5, 15, 25, 25, 15,  5,  0},
    { 0,  5, 10, 15, 15, 10,  5,  0},
    { 0,  0,  5,  5,  5,  5,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int GeminiAI::blackKingPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
    { 0, 20, 40, 60, 60, 40, 20,  0},
    { 0, 30, 60, 80, 80, 60, 30,  0},
    { 0, 30, 60, 80, 80, 60, 30,  0},
    { 0, 20, 40, 60, 60, 40, 20,  0},
    { 0, 10, 20, 30, 30, 20, 10,  0},
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
    GameManager::log(LogLevel::Info, QString("GeminiAI: Initializing EGDB with path: %1").arg(m_egdbPath));
    char db_init_msg_buffer[256]; // Declare a buffer for db_init messages
    int initializedPieces = db_init(256, db_init_msg_buffer, m_egdbPath.toUtf8().constData());
    if (initializedPieces > 0) { // Check if any databases were initialized
        m_egdbInitialized = true;
        m_maxEGDBPieces = initializedPieces;
        GameManager::log(LogLevel::Info, QString("GeminiAI: EGDB initialized successfully. Max pieces: %1").arg(m_maxEGDBPieces));
    } else {
        m_egdbInitialized = false;
        m_maxEGDBPieces = 0;
        GameManager::log(LogLevel::Warning, QString("GeminiAI: Failed to initialize EGDB with path: %1. EGDB will be disabled.").arg(m_egdbPath));
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
    GameManager::log(LogLevel::Debug, QString("GeminiAI: Mode set to %1").arg(mode));
}

void GeminiAI::setHandicap(int handicap)
{
    m_handicap = handicap;
    GameManager::log(LogLevel::Debug, QString("GeminiAI: Handicap set to %1").arg(handicap));
}

bool GeminiAI::sendCommand(const QString& command, QString& reply)
{
    if (m_useExternalEngine && m_primaryExternalEngine) {
        return m_primaryExternalEngine->sendCommand(command, reply);
    } else {
        GameManager::log(LogLevel::Debug, QString("GeminiAI: sendCommand called with: %1 (internal AI, no action taken)").arg(command));
        reply = "Internal AI: Command received, but no external engine is active.";
        return true;
    }
}

void GeminiAI::addMoveToUserBook(const Board8x8& board, const CBmove& move)
{
    Q_UNUSED(board);
    Q_UNUSED(move);
    // GameManager::log(LogLevel::Debug, "GeminiAI: addMoveToUserBook (placeholder)."); // Removed verbose log
}

void GeminiAI::deleteCurrentEntry()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: deleteCurrentEntry (placeholder)."); // Removed verbose log
}

void GeminiAI::deleteAllEntriesFromUserBook()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: deleteAllEntriesFromUserBook (placeholder)."); // Removed verbose log
}

void GeminiAI::navigateToNextEntry()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: navigateToNextEntry (placeholder)."); // Removed verbose log
}

void GeminiAI::navigateToPreviousEntry()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: navigateToPreviousEntry (placeholder)."); // Removed verbose log
}

void GeminiAI::resetNavigation()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: resetNavigation (placeholder)."); // Removed verbose log
}

void GeminiAI::loadUserBook(const QString& filename)
{
    Q_UNUSED(filename);
    // GameManager::log(LogLevel::Debug, "GeminiAI: loadUserBook (placeholder)."); // Removed verbose log
}

void GeminiAI::saveUserBook(const QString& filename)
{
    Q_UNUSED(filename);
    // GameManager::log(LogLevel::Debug, "GeminiAI: saveUserBook (placeholder)."); // Removed verbose log
}

void GeminiAI::setExternalEnginePath(const QString& path)
{
    m_externalEnginePath = path;
    // GameManager::log(LogLevel::Debug, QString("GeminiAI: External engine path set to: %1").arg(path)); // Removed verbose log
}

void GeminiAI::setSecondaryExternalEnginePath(const QString& path)
{
    m_secondaryExternalEnginePath = path;
    // GameManager::log(LogLevel::Debug, QString("GeminiAI: Secondary external engine path set to: %1").arg(path)); // Removed verbose log
}

void GeminiAI::requestMove(Board8x8 board, int colorToMove, double timeLimit)
{
    // GameManager::log(LogLevel::Debug, QString("GeminiAI: requestMove called. Color: %1, Time Limit: %2").arg(colorToMove).arg(timeLimit)); // Removed verbose log

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
                pos currentPos;
                boardtobitboard(&board, &currentPos);
                CBmove legalMoves[MAXMOVES];
                int nmoves = 0;
                int isjump = 0;
                bool dummy_can_continue_multijump = false;
                get_legal_moves_c(&currentPos, colorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

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
                GameManager::log(LogLevel::Warning, QString("GeminiAI: Could not parse bestmove string: %1").arg(bestMoveStr));
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
    // GameManager::log(LogLevel::Debug, "GeminiAI: doWork (placeholder)."); // Removed verbose log
}

void GeminiAI::initEngineProcess()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: initEngineProcess called."); // Removed verbose log
    if (!m_externalEnginePath.isEmpty()) {
        m_primaryExternalEngine = new ExternalEngine(m_externalEnginePath, this);
        if (m_primaryExternalEngine->startEngine()) {
            GameManager::log(LogLevel::Info, QString("GeminiAI: Primary external engine started: %1").arg(m_externalEnginePath));
            m_useExternalEngine = true;
        } else {
            GameManager::log(LogLevel::Error, QString("GeminiAI: Failed to start primary external engine: %1").arg(m_externalEnginePath));
            m_useExternalEngine = false;
        }
    }
    if (!m_secondaryExternalEnginePath.isEmpty()) {
        m_secondaryExternalEngine = new ExternalEngine(m_secondaryExternalEnginePath, this);
        if (m_secondaryExternalEngine->startEngine()) {
            GameManager::log(LogLevel::Info, QString("GeminiAI: Secondary external engine started: %1").arg(m_secondaryExternalEnginePath));
        } else {
            GameManager::log(LogLevel::Error, QString("GeminiAI: Failed to start secondary external engine: %1").arg(m_secondaryExternalEnginePath));
        }
    }
}

void GeminiAI::quitEngineProcess()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: quitEngineProcess called."); // Removed verbose log
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
    // GameManager::log(LogLevel::Debug, "GeminiAI: startAnalyzeGame called."); // Removed verbose log
    // For analysis, we can run a deeper search or a search with a fixed time limit.
    // For now, let's use a default time limit for analysis.
    double analysisTimeLimit = 10.0; // 10 seconds for analysis

    // Perform the search
    getBestMove(board, colorToMove, analysisTimeLimit);

    // Emit the evaluation result
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
}

void GeminiAI::startAutoplay(const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    // GameManager::log(LogLevel::Debug, "GeminiAI: startAutoplay (placeholder)."); // Removed verbose log
}

void GeminiAI::startEngineMatch(int numGames, const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    // GameManager::log(LogLevel::Debug, QString("GeminiAI: startEngineMatch (placeholder) for %1 games.").arg(numGames)); // Removed verbose log
}

void GeminiAI::startRunTestSet(const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    // GameManager::log(LogLevel::Debug, "GeminiAI: startRunTestSet (placeholder)."); // Removed verbose log
}

void GeminiAI::startAnalyzePdn(const Board8x8& board, int colorToMove)
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: startAnalyzePdn called."); // Removed verbose log
    // For PDN analysis, we can run a deeper search or a search with a fixed time limit.
    // For now, let's use a default time limit for analysis.
    double analysisTimeLimit = 10.0; // 10 seconds for analysis

    // Perform the search
    getBestMove(board, colorToMove, analysisTimeLimit);

    // Emit the evaluation result
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth);
}

void GeminiAI::abortSearch()
{
    // GameManager::log(LogLevel::Debug, "GeminiAI: abortSearch (placeholder)."); // Removed verbose log
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

    // For non-captures, a more advanced heuristic could be used.
    // For now, we'll just return false (no strong preference for non-captures)
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

            // XOR with piece type at current square
            hash ^= ZobristTable[r][c][pieceType];
        }
    }

    // XOR in the side to move
    if (colorToMove == CB_WHITE) {
        hash ^= ZobristWhiteToMove;
    }

    return hash;
}

CBmove GeminiAI::getBestMove(Board8x8 board, int color, double maxtime)
{
    m_abortRequested.storeRelaxed(0); // Reset abort flag

    // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: Entering with maxtime: %1, color: %2").arg(maxtime).arg(color)); // Removed verbose log

    CBmove bestMoveRoot = {0}; // Stores the best move found by iterative deepening
    int bestValueRoot = LOSS_SCORE;
    CBmove currentIterationBestMove = {0};
    int currentIterationBestValue = LOSS_SCORE;
    int actualSearchDepth = 0; // To store the depth reached

    QElapsedTimer timer;
    timer.start();

    // Convert maxtime from seconds to milliseconds for QElapsedTimer
    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: maxtime: %1s, timeLimitMs: %2ms").arg(maxtime).arg(timeLimitMs)); // Removed verbose log
    const qint64 MIN_TIME_FOR_FIRST_ITERATION_MS = 100; // Ensure at least 100ms for first iteration

    // Count pieces for EGDB lookup (existing logic)
    int totalPieces = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board.board[r][c] != CB_EMPTY) {
                totalPieces++;
            }
        }
    }

    // EGDB Lookup if in endgame (existing logic)
    if (m_egdbInitialized && totalPieces <= m_maxEGDBPieces) { // Check if EGDB is initialized and within piece threshold
        POSITION current_pos_egdb = boardToPosition(board, color);
        int egdb_result = dblookup(&current_pos_egdb, color == CB_WHITE ? DB_WHITE : DB_BLACK);

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            CBmove legalMoves[MAXMOVES];
            int nmoves = 0;
            int isjump = 0;
            pos currentPosBitboard;
            boardtobitboard(&board, &currentPosBitboard);
            bool dummy_can_continue_multijump = false;
            get_legal_moves_c(&currentPosBitboard, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

            // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: EGDB lookup - Legal moves found: %1").arg(nmoves)); // Removed verbose log

            if (nmoves == 0) {
                m_lastEvaluationScore = 0; // No moves, draw or loss
                m_lastSearchDepth = 0; // No search performed
                GameManager::log(LogLevel::Warning, "GeminiAI::getBestMove: EGDB lookup - No legal moves, returning default.");
                return bestMoveRoot;
            }

            for (int i = 0; i < nmoves; ++i) {
                Board8x8 nextBoard = board;
                domove_c(&legalMoves[i], &nextBoard);

                POSITION next_pos_egdb = boardToPosition(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE);
                int next_egdb_result = dblookup(&next_pos_egdb, (color == CB_WHITE) ? DB_WHITE : DB_BLACK);

                if (egdb_result == DB_WIN) {
                    if (next_egdb_result == DB_LOSS) {
                        m_lastEvaluationScore = std::numeric_limits<int>::max(); // Winning move
                        m_lastSearchDepth = 0; // No search performed
                        // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: EGDB lookup - Found winning move: from(%1,%2) to(%3,%4)").arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y)); // Removed verbose log
                        return legalMoves[i];
                    }
                } else if (egdb_result == DB_DRAW) {
                    if (next_egdb_result == DB_DRAW) {
                        m_lastEvaluationScore = 0; // Drawing move
                        m_lastSearchDepth = 0; // No search performed
                        // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: EGDB lookup - Found drawing move: from(%1,%2) to(%3,%4)").arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y)); // Removed verbose log
                        return legalMoves[i];
                    }
                }
            }
        }
    }

    // Iterative Deepening
    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: Starting iteration with depth: %1").arg(current_depth)); // Removed verbose log
        if (m_abortRequested.loadRelaxed()) {
            GameManager::log(LogLevel::Info, "GeminiAI::getBestMove: Abort requested during iterative deepening.");
            break; // Abort if requested
        }

        // Check if enough time remains for the next iteration
        qint64 elapsed = timer.elapsed();
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: Elapsed time: %1ms").arg(elapsed)); // Removed verbose log
        if (elapsed > timeLimitMs && current_depth > 1) { // Only break if not the first iteration
            GameManager::log(LogLevel::Warning, QString("GeminiAI::getBestMove: Time limit exceeded (%1ms > %2ms), breaking iterative deepening.").arg(elapsed).arg(timeLimitMs));
            break;
        }
        // For the first iteration, ensure at least MIN_TIME_FOR_FIRST_ITERATION_MS is available
        if (current_depth == 1 && elapsed > timeLimitMs - MIN_TIME_FOR_FIRST_ITERATION_MS && timeLimitMs > MIN_TIME_FOR_FIRST_ITERATION_MS) {
             GameManager::log(LogLevel::Warning, QString("GeminiAI::getBestMove: Not enough time for first iteration, elapsed: %1ms, timeLimitMs: %2ms").arg(elapsed).arg(timeLimitMs));
             break;
        }


    CBmove moves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, color, moves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    if (nmoves == 0) {
        // No legal moves, game over. This should ideally be handled before iterative deepening.
        // But as a safeguard, if no moves, return the default bestMoveRoot.
        m_lastEvaluationScore = LOSS_SCORE;
        m_lastSearchDepth = 0; // Store the depth reached
        GameManager::log(LogLevel::Warning, "GeminiAI::getBestMove: No legal moves at root, returning default bestMoveRoot.");
        CBmove bestMoveRoot;
        bestMoveRoot.from.x = bestMoveRoot.from.y = bestMoveRoot.to.x = bestMoveRoot.to.y = 0;
        bestMoveRoot.is_capture = 0;
        return bestMoveRoot;
    }

    // Check for forced moves (jumps)
    std::vector<CBmove> jumps;
    for (int i = 0; i < nmoves; ++i) {
        if (moves[i].is_capture) {
            jumps.push_back(moves[i]);
        }
    }

    if (jumps.size() == 1) {
        GameManager::log(LogLevel::Info, "GeminiAI::getBestMove: Only one legal jump, returning it immediately.");
        m_lastEvaluationScore = evaluateBoard(board, color);
        m_lastSearchDepth = 0;
        return jumps[0];
    }

    if (nmoves == 1) {
        GameManager::log(LogLevel::Info, "GeminiAI::getBestMove: Only one legal move, returning it immediately.");
        m_lastEvaluationScore = evaluateBoard(board, color);
        m_lastSearchDepth = 0;
        return moves[0];
    }

    // If there are jumps, only consider those moves
    if (!jumps.empty()) {
        nmoves = jumps.size();
        for (int i = 0; i < nmoves; ++i) {
            moves[i] = jumps[i];
        }
    }

    // Sort moves to prioritize captures (already done by the jump check, but good for non-jumps)
    std::sort(moves, moves + nmoves, compareMoves);


        currentIterationBestValue = std::numeric_limits<int>::min();
        CBmove iterationBestMove = {0}; // Best move for this iteration

        for (int i = 0; i < nmoves; ++i) {
            if (m_abortRequested.loadRelaxed()) {
                GameManager::log(LogLevel::Info, QString("GeminiAI::getBestMove: Abort requested during move iteration at depth %1.").arg(current_depth));
                break; // Abort if requested
            }

            Board8x8 nextBoard = board;
            domove_c(&moves[i], &nextBoard); // Make the move

            // Call minimax with the current iteration depth
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), nullptr, true);

            if (moveValue > currentIterationBestValue) {
                currentIterationBestValue = moveValue;
                iterationBestMove = moves[i];
            }
        }

        // If aborted during an iteration, break out of iterative deepening
        if (m_abortRequested.loadRelaxed()) {
            GameManager::log(LogLevel::Info, "GeminiAI::getBestMove: Abort requested after move iteration.");
            break;
        }

        // If a move was found in this iteration, update the overall best move
        if (currentIterationBestValue > std::numeric_limits<int>::min()) {
            bestValueRoot = currentIterationBestValue;
            bestMoveRoot = iterationBestMove;
            actualSearchDepth = current_depth; // Update the actual depth reached
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::getBestMove: Depth %1 - Best move found: from(%2,%3) to(%4,%5), value: %6").arg(current_depth).arg(bestMoveRoot.from.x).arg(bestMoveRoot.from.y).arg(bestMoveRoot.to.x).arg(bestMoveRoot.to.y).arg(bestValueRoot)); // Removed verbose log
        } else {
            GameManager::log(LogLevel::Warning, QString("GeminiAI::getBestMove: Depth %1 - No valid move found in this iteration.").arg(current_depth));
        }
    }

    m_lastEvaluationScore = bestValueRoot; // Store the final evaluation score
    m_lastSearchDepth = actualSearchDepth; // Store the final search depth
    GameManager::log(LogLevel::Info, QString("GeminiAI::getBestMove: Final best move: from(%1,%2) to(%3,%4), score: %5, depth: %6").arg(bestMoveRoot.from.x).arg(bestMoveRoot.from.y).arg(bestMoveRoot.to.x).arg(bestMoveRoot.to.y).arg(m_lastEvaluationScore).arg(m_lastSearchDepth));
    return bestMoveRoot;
}

int GeminiAI::evaluateBoard(const Board8x8& board, int color)
{
    int evaluation = 0;
    int opponentColor = (color == CB_WHITE) ? CB_BLACK : CB_WHITE;

    int whiteMen = 0;
    int whiteKings = 0;
    int blackMen = 0;
    int blackKings = 0;

    CBmove legalMoves[MAXMOVES];
    int nmovesCurrentPlayer = 0;
    int nmovesOpponent = 0;
    int isjump = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, color, legalMoves, &nmovesCurrentPlayer, &isjump, NULL, &dummy_can_continue_multijump);

    // Material and PST evaluation
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            int pieceValue = 0;

            switch (piece) {
                case (CB_WHITE | CB_MAN):
                    pieceValue = 200;
                    evaluation += pieceValue;
                    evaluation += whiteManPST[r][c];
                    whiteMen++;
                    if (r == 5) evaluation += 20;
                    if (r == 6) evaluation += 40;
                    break;
                case (CB_WHITE | CB_KING):
                    pieceValue = 400;
                    evaluation += pieceValue;
                    evaluation += whiteKingPST[r][c];
                    whiteKings++;
                    break;
                case (CB_BLACK | CB_MAN):
                    pieceValue = 200;
                    evaluation -= pieceValue;
                    evaluation -= blackManPST[r][c];
                    blackMen++;
                    if (r == 2) evaluation -= 20;
                    if (r == 1) evaluation -= 40;
                    break;
                case (CB_BLACK | CB_KING):
                    pieceValue = 400;
                    evaluation -= pieceValue;
                    evaluation -= blackKingPST[r][c];
                    blackKings++;
                    break;
                default:
                    break;
            }

            // Tactical considerations: Penalize undefended pieces that are attacked
            if (piece != CB_EMPTY) {
                int pieceColor = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
                if (pieceColor == color) { // Only consider current player's pieces
                    if (isSquareAttacked(board, r, c, opponentColor)) {
                        bool isDefended = false;
                        
                        // New defense check: See if a recapture is possible
                        CBmove opponentCaptures[MAXMOVES];
                        int nOpponentCaptures = 0;
                        int isOpponentJump = 0;
                        pos opponentPos;
                        boardtobitboard(&board, &opponentPos);
                        bool dummy_multijump = false;
                        get_legal_moves_c(&opponentPos, opponentColor, opponentCaptures, &nOpponentCaptures, &isOpponentJump, NULL, &dummy_multijump);

                        if(isOpponentJump) {
                            for(int i = 0; i < nOpponentCaptures; ++i) {
                                // Check if this capture move attacks our piece at (r, c)
                                bool attacksOurPiece = false;
                                for(int j = 0; j < opponentCaptures[i].jumps; ++j) {
                                    int capturedPieceY = opponentCaptures[i].del[j].y - 2;
                                    int capturedPieceX = opponentCaptures[i].del[j].x - 2;
                                    if(capturedPieceY == r && capturedPieceX == c) {
                                        attacksOurPiece = true;
                                        break;
                                    }
                                }

                                if(attacksOurPiece) {
                                    // If the opponent makes this capture, is their piece then vulnerable?
                                    Board8x8 boardAfterCapture = board;
                                    domove_c(&opponentCaptures[i], &boardAfterCapture);
                                    if(isSquareAttacked(boardAfterCapture, opponentCaptures[i].to.y - 2, opponentCaptures[i].to.x - 2, color)) {
                                        isDefended = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (!isDefended) {
                            if (pieceColor == CB_WHITE) {
                                evaluation -= (pieceValue / 2);
                            } else {
                                evaluation += (pieceValue / 2);
                            }
                        }
                    }
                }
            }
        }
    }

    // Passed Men evaluation
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];

            // Check for White Passed Men
            if (piece == (CB_WHITE | CB_MAN)) {
                bool isPassed = true;
                // Check columns c-1, c, c+1 in front of the white man
                for (int check_r = r + 1; check_r < 8; ++check_r) {
                    for (int check_c_offset = -1; check_c_offset <= 1; ++check_c_offset) {
                        int check_c = c + check_c_offset;
                        if (check_c >= 0 && check_c < 8) {
                            int opponent_piece = board.board[check_r][check_c];
                            if (opponent_piece == (CB_BLACK | CB_MAN) || opponent_piece == (CB_BLACK | CB_KING)) {
                                isPassed = false;
                                break;
                            }
                        }
                    }
                    if (!isPassed) break;
                }
                if (isPassed) {
                    evaluation += 50 + (r * 5); // Bonus for passed man, scaled by how far advanced it is
                }
            }
            // Check for Black Passed Men
            else if (piece == (CB_BLACK | CB_MAN)) {
                bool isPassed = true;
                // Check columns c-1, c, c+1 in front of the black man
                for (int check_r = r - 1; check_r >= 0; --check_r) {
                    for (int check_c_offset = -1; check_c_offset <= 1; ++check_c_offset) {
                        int check_c = c + check_c_offset;
                        if (check_c >= 0 && check_c < 8) {
                            int opponent_piece = board.board[check_r][check_c];
                            if (opponent_piece == (CB_WHITE | CB_MAN) || opponent_piece == (CB_WHITE | CB_KING)) {
                                isPassed = false;
                                break;
                            }
                        }
                    }
                    if (!isPassed) break;
                }
                if (isPassed) {
                    evaluation -= (50 + ((7 - r) * 5)); // Penalty for opponent's passed man, scaled by how far advanced it is
                }
            }
        }
    }

    // King Safety: Penalize isolated kings
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];

            if (piece == (CB_WHITE | CB_KING)) {
                bool isIsolated = true;
                // Check 4 diagonal neighbors for friendly pieces
                int dr[] = {-1, -1, 1, 1};
                int dc[] = {-1, 1, -1, 1};
                for (int i = 0; i < 4; ++i) {
                    int nr = r + dr[i];
                    int nc = c + dc[i];
                    if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                        int neighbor_piece = board.board[nr][nc];
                        if (neighbor_piece == (CB_WHITE | CB_MAN) || neighbor_piece == (CB_WHITE | CB_KING)) {
                            isIsolated = false;
                            break;
                        }
                    }
                }
                if (isIsolated) {
                    evaluation -= 30; // Penalty for isolated white king
                }
            } else if (piece == (CB_BLACK | CB_KING)) {
                bool isIsolated = true;
                // Check 4 diagonal neighbors for friendly pieces
                int dr[] = {-1, -1, 1, 1};
                int dc[] = {-1, 1, -1, 1};
                for (int i = 0; i < 4; ++i) {
                    int nr = r + dr[i];
                    int nc = c + dc[i];
                    if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                        int neighbor_piece = board.board[nr][nc];
                        if (neighbor_piece == (CB_BLACK | CB_MAN) || neighbor_piece == (CB_BLACK | CB_KING)) {
                            isIsolated = false;
                            break;
                        }
                    }
                }
                if (isIsolated) {
                    evaluation += 30; // Penalty for isolated black king (positive for black's perspective)
                }
            }
        }
    }

    // Mobility
    evaluation += nmovesCurrentPlayer * 5;

    // Opponent mobility
    get_legal_moves_c(&currentPos, opponentColor, legalMoves, &nmovesOpponent, &isjump, NULL, &dummy_can_continue_multijump);
    evaluation -= nmovesOpponent * 5;

    // Center Control
    for (int r = 2; r <= 5; ++r) {
        for (int c = 2; c <= 5; ++c) {
            int piece = board.board[r][c];
            if (piece == (CB_WHITE | CB_MAN) || piece == (CB_WHITE | CB_KING)) {
                evaluation += 10;
            } else if (piece == (CB_BLACK | CB_MAN) || piece == (CB_BLACK | CB_KING)) {
                evaluation -= 10;
            }
        }
    }

    // Tempo/Turn Bonus
    evaluation += 10;

    if (color == CB_BLACK) {
        evaluation = -evaluation;
    }

    return evaluation;
}

int GeminiAI::minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
{
    int bestEval; // Declare bestEval here
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Color %2, Alpha %3, Beta %4").arg(depth).arg(color).arg(alpha).arg(beta)); // Removed verbose log
    if (m_abortRequested.loadRelaxed()) {
        GameManager::log(LogLevel::Info, QString("GeminiAI::minimax: Abort requested at depth %1.").arg(depth));
        return 0; // Return a neutral value if aborted
    }

    // EGDB Lookup at current node (conditional lookup)
    // Only perform lookup if EGDB is initialized and within piece threshold
    int totalPieces = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board.board[r][c] != CB_EMPTY) {
                totalPieces++;
            }
        }
    }

    if (m_egdbInitialized && totalPieces <= m_maxEGDBPieces) {
        POSITION current_pos_egdb = boardToPosition(board, color);
        int egdb_result = dblookup(&current_pos_egdb, 1); // Use conditional lookup (cl=1)

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            if (egdb_result == DB_WIN) {
                GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: EGDB WIN at depth %1. Score: %2").arg(depth).arg(WIN_SCORE - depth));
                return WIN_SCORE - depth; // Win for current player, adjusted for depth
            } else if (egdb_result == DB_LOSS) {
                GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: EGDB LOSS at depth %1. Score: %2").arg(depth).arg(LOSS_SCORE + depth));
                return LOSS_SCORE + depth; // Loss for current player, adjusted for depth
            } else if (egdb_result == DB_DRAW) {
                GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: EGDB DRAW at depth %1. Score: 0").arg(depth));
                return 0; // Draw
            }
        }
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    // Null Move Pruning
    const int R = 2; // Depth reduction factor
    if (allowNull && depth >= 3 && totalPieces > 8 && !isjump) {
        // Make a null move (pass the turn) and search with reduced depth
        int nullMoveEval = -minimax(board, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1 - R, -beta, -beta + 1, nullptr, false);

        if (nullMoveEval >= beta) {
            // If the null move search causes a beta cutoff, we can prune this branch
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Null move cutoff at depth %1. Score: %2").arg(depth).arg(beta));
            return beta;
        }
    }

    // Generate Zobrist key for the current position
    uint64_t currentKey = generateZobristKey(board, color);

    // Probe transposition table
    if (m_transpositionTable.count(currentKey)) {
        const TTEntry& entry = m_transpositionTable.at(currentKey);
        if (entry.depth >= depth) {
            if (entry.type == TTEntry::EXACT) {
                // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: TT hit (EXACT) at depth %1, score: %2").arg(depth).arg(entry.score)); // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) {
                // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: TT hit (ALPHA) at depth %1, score: %2").arg(depth).arg(entry.score)); // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::BETA && entry.score >= beta) {
                // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: TT hit (BETA) at depth %1, score: %2").arg(depth).arg(entry.score)); // Removed verbose log
                return entry.score;
            }
        }
        // If a best move is stored, use it for move ordering
        if (bestMove && entry.bestMove.from.x != 0) { // Assuming {0} is a null move
            *bestMove = entry.bestMove;
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: TT best move for ordering: from(%1,%2) to(%3,%4)").arg(bestMove->from.x).arg(bestMove->from.y).arg(bestMove->to.x).arg(bestMove->to.y)); // Removed verbose log
        }
    }

    if (depth == 0) {
        int q_score = quiescenceSearch(board, color, alpha, beta);
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth 0, Quiescence Search score: %1").arg(q_score)); // Removed verbose log
        return q_score; // Call quiescence search at depth 0
    }

    // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Color %2, Legal moves found: %3").arg(depth).arg(color).arg(nmoves)); // Removed verbose log

    if (nmoves == 0) {
        // If no legal moves, the current player loses.
        // Return a score that is worse the sooner the loss is
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Color %2, No legal moves, returning losing score.").arg(depth).arg(color)); // Removed verbose log
        return LOSS_SCORE + depth;
    }

    // Move ordering: Store moves and their scores
    std::vector<std::pair<CBmove, int>> scoredMoves;
    for (int i = 0; i < nmoves; ++i) {
        int score = 0;
        // Prioritize TT best move
        if (bestMove && legalMoves[i].from.x == bestMove->from.x && legalMoves[i].from.y == bestMove->from.y &&
            legalMoves[i].to.x == bestMove->to.x && legalMoves[i].to.y == bestMove->to.y) {
            score += 100000; // High bonus for TT move
        }
        // Prioritize killer moves
        if (depth < MAX_DEPTH) {
            if (legalMoves[i].from.x == m_killerMoves[depth][0].from.x && legalMoves[i].from.y == m_killerMoves[depth][0].from.y &&
                legalMoves[i].to.x == m_killerMoves[depth][0].to.x && legalMoves[i].to.y == m_killerMoves[depth][0].to.y) {
                score += 90000; // Bonus for first killer move
            } else if (legalMoves[i].from.x == m_killerMoves[depth][1].from.x && legalMoves[i].from.y == m_killerMoves[depth][1].from.y &&
                       legalMoves[i].to.x == m_killerMoves[depth][1].to.x && legalMoves[i].to.y == m_killerMoves[depth][1].to.y) {
                score += 80000; // Bonus for second killer move
            }
        }
        // Add history score
        score += m_historyTable[legalMoves[i].from.y][legalMoves[i].from.x][legalMoves[i].to.y][legalMoves[i].to.x];
        scoredMoves.push_back({legalMoves[i], score});
    }

    // Sort moves based on their scores
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const std::pair<CBmove, int>& a, const std::pair<CBmove, int>& b) {
        return a.second > b.second; // Sort in descending order of score
    });

    bestEval = LOSS_SCORE;
    CBmove currentBestMove = {0}; // Initialize with a null move

    for (const auto& scoredMove : scoredMoves) {
        const CBmove& move = scoredMove.first;

        if (m_abortRequested.loadRelaxed()) {
            GameManager::log(LogLevel::Info, QString("GeminiAI::minimax: Abort requested during move iteration at depth %1.").arg(depth));
            break; // Abort if requested
        }

        Board8x8 newBoard = board;
        domove_c(&move, &newBoard); // Make the move

        // Recursive call with negated alpha and beta for the opponent
        int eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);

        // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Move from(%2,%3) to(%4,%5), Eval: %6").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(eval)); // Removed verbose log

        if (eval > bestEval) {
            bestEval = eval;
            currentBestMove = move;
            if (bestMove) { // Only update bestMove if it's not nullptr (i.e., in the top-level call)
                *bestMove = move;
                // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, New best move: from(%2,%3) to(%4,%5), Eval: %6").arg(depth).arg(bestMove->from.x).arg(bestMove->from.y).arg(bestMove->to.x).arg(bestMove->to.y).arg(bestEval)); // Removed verbose log
            }
        }

        alpha = std::max(alpha, eval); // Update alpha for the current node
        if (alpha >= beta) {
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Alpha-beta cut-off. Alpha: %2, Beta: %3").arg(alpha).arg(beta)); // Removed verbose log
            // Store killer move
            if (depth < MAX_DEPTH) {
                if (!(m_killerMoves[depth][0].from.x == move.from.x && m_killerMoves[depth][0].from.y == move.from.y &&
                      m_killerMoves[depth][0].to.x == move.to.x && m_killerMoves[depth][0].to.y == move.to.y)) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0]; // Shift existing killer to second slot
                    m_killerMoves[depth][0] = move; // Store new killer move
                    // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Stored killer move: from(%2,%3) to(%4,%5)").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y)); // Removed verbose log
                }
            }
            // Update history table
            m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x] += depth;
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Depth %1, Updated history for move: from(%2,%3) to(%4,%5), new score: %6").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x])); // Removed verbose log

            break; // Alpha-beta cut-off
        }
    }

    // Store result in transposition table
    TTEntry newEntry;
    newEntry.key = currentKey;
    newEntry.depth = depth;
    newEntry.score = bestEval;
    if (bestEval <= alpha) {
        newEntry.type = TTEntry::BETA;
    } else if (bestEval >= beta) {
        newEntry.type = TTEntry::ALPHA;
    } else {
        newEntry.type = TTEntry::EXACT;
    }
    newEntry.bestMove = currentBestMove;
    m_transpositionTable[currentKey] = newEntry;
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::minimax: Storing to TT: Depth %1, Score %2, Type %3, BestMove from(%4,%5) to(%6,%7)").arg(depth).arg(bestEval).arg(newEntry.type).arg(newEntry.bestMove.from.x).arg(newEntry.bestMove.from.y).arg(newEntry.bestMove.to.x).arg(newEntry.bestMove.to.y)); // Removed verbose log

    return bestEval;
}

int GeminiAI::quiescenceSearch(Board8x8 board, int color, int alpha, int beta)
{
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Color %1, Alpha %2, Beta %3").arg(color).arg(alpha).arg(beta)); // Removed verbose log
    // Evaluate the current board from the perspective of the current player
    int standPat = evaluateBoard(board, color);
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Stand pat score: %1").arg(standPat)); // Removed verbose log

    if (standPat >= beta) {
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Beta cutoff. Returning beta: %1").arg(beta)); // Removed verbose log
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

    // If there are no captures, return the static evaluation
    if (isjump == 0) {
        // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: No captures. Returning stand_pat: %1").arg(standPat)); // Removed verbose log
        return standPat;
    }

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) { // Only consider captures in quiescence search
            Board8x8 newBoard = board;
            domove_c(&legalMoves[i], &newBoard); // Make the move

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha); // Recursively call quiescence search for the opponent
            // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Move %1, Score %2").arg(i).arg(score)); // Removed verbose log

            if (score >= beta) {
                // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Beta cutoff after move. Returning beta: %1").arg(beta)); // Removed verbose log
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    // GameManager::log(LogLevel::Debug, QString("GeminiAI::quiescenceSearch: Returning alpha: %1").arg(alpha)); // Removed verbose log
    return alpha;
}
bool GeminiAI::isKingTrapped(const Board8x8& board, int r, int c, const CBmove* legalMoves, int nmoves)
{
    for (int i = 0; i < nmoves; i++) {
        if (legalMoves[i].from.x == c && legalMoves[i].from.y == r) {
            return false;
        }
    }

    return true;
}

bool GeminiAI::hasCaptures(const Board8x8& board, int colorToMove)
{
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, colorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    return isjump == 1;
}

bool GeminiAI::isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor)
{
    // Check for attacking men
    int man = (attackerColor == CB_WHITE) ? (CB_WHITE | CB_MAN) : (CB_BLACK | CB_MAN);
    int king = (attackerColor == CB_WHITE) ? (CB_WHITE | CB_KING) : (CB_BLACK | CB_KING);

    // Directions for men attacks (relative to attacker)
    // White men attack diagonally forward (rows decrease for white)
    // Black men attack diagonally forward (rows increase for black)
    int r_dir_man = (attackerColor == CB_WHITE) ? -1 : 1;

    // Check for men attacks
    // Check top-left diagonal for white man, bottom-left for black man
    if (r + r_dir_man >= 0 && r + r_dir_man < 8 && c - 1 >= 0 && c - 1 < 8) {
        if (board.board[r + r_dir_man][c - 1] == man || board.board[r + r_dir_man][c - 1] == king) {
            return true;
        }
    }
    // Check top-right diagonal for white man, bottom-right for black man
    if (r + r_dir_man >= 0 && r + r_dir_man < 8 && c + 1 >= 0 && c + 1 < 8) {
        if (board.board[r + r_dir_man][c + 1] == man || board.board[r + r_dir_man][c + 1] == king) {
            return true;
        }
    }

    // Check for king attacks (all four diagonal directions)
    int r_dirs[] = {-1, -1, 1, 1};
    int c_dirs[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + r_dirs[i];
        int nc = c + c_dirs[i];
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (board.board[nr][nc] == king) {
                return true;
            }
        }
    }

    return false;
}

POSITION GeminiAI::boardToPosition(const Board8x8& board, int colorToMove)
{
    POSITION p;
    pos bitboard_pos;
    boardtobitboard(&board, &bitboard_pos);

    p.bm = bitboard_pos.bm;
    p.bk = bitboard_pos.bk;
    p.wm = bitboard_pos.wm;
    p.wk = bitboard_pos.wk;
    p.color = (colorToMove == CB_WHITE) ? DB_WHITE : DB_BLACK;

    return p;
}