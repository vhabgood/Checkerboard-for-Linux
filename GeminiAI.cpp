#include <QDebug>
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
    qInfo() << QString("GeminiAI: Initializing EGDB with path: %1").arg(m_egdbPath);
    char db_init_msg_buffer[256]; // Declare a buffer for db_init messages
    db_init_msg_buffer[0] = '\0'; // Ensure it's null-terminated initially
    int initializedPieces = db_init(256, db_init_msg_buffer, m_egdbPath.toUtf8().constData());
    if (initializedPieces > 0) { // Check if any databases were initialized
        m_egdbInitialized = true;
        m_maxEGDBPieces = initializedPieces;
        qInfo() << QString("GeminiAI: EGDB initialized successfully. Max pieces: %1. Message: %2").arg(m_maxEGDBPieces).arg(db_init_msg_buffer);
    } else {
        m_egdbInitialized = false;
        m_maxEGDBPieces = 0;
        qWarning() << QString("GeminiAI: Failed to initialize EGDB with path: %1. EGDB will be disabled. Message: %2").arg(m_egdbPath).arg(db_init_msg_buffer);
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
    qDebug() << QString("GeminiAI: Mode set to %1").arg(mode);
}

void GeminiAI::setHandicap(int handicap)
{
    m_handicap = handicap;
    qDebug() << QString("GeminiAI: Handicap set to %1").arg(handicap);
}

bool GeminiAI::sendCommand(const QString& command, QString& reply)
{
    if (m_useExternalEngine && m_primaryExternalEngine) {
        return m_primaryExternalEngine->sendCommand(command, reply);
    } else {
        qDebug() << QString("GeminiAI: sendCommand called with: %1 (internal AI, no action taken)").arg(command);
        reply = "Internal AI: Command received, but no external engine is active.";
        return true;
    }
}

void GeminiAI::addMoveToUserBook(const Board8x8& board, const CBmove& move)
{
    Q_UNUSED(board);
    Q_UNUSED(move);
    // qDebug() << "GeminiAI: addMoveToUserBook (placeholder)."; // Removed verbose log
}

void GeminiAI::deleteCurrentEntry()
{
    // qDebug() << "GeminiAI: deleteCurrentEntry (placeholder)."; // Removed verbose log
}

void GeminiAI::deleteAllEntriesFromUserBook()
{
    // qDebug() << "GeminiAI: deleteAllEntriesFromUserBook (placeholder)."; // Removed verbose log
}

void GeminiAI::navigateToNextEntry()
{
    // qDebug() << "GeminiAI: navigateToNextEntry (placeholder)."; // Removed verbose log
}

void GeminiAI::navigateToPreviousEntry()
{
    // qDebug() << "GeminiAI: navigateToPreviousEntry (placeholder)."; // Removed verbose log
}

void GeminiAI::resetNavigation()
{
    // qDebug() << "GeminiAI: resetNavigation (placeholder)."; // Removed verbose log
}

void GeminiAI::loadUserBook(const QString& filename)
{
    Q_UNUSED(filename);
    // qDebug() << "GeminiAI: loadUserBook (placeholder)."; // Removed verbose log
}

void GeminiAI::saveUserBook(const QString& filename)
{
    Q_UNUSED(filename);
    // qDebug() << "GeminiAI: saveUserBook (placeholder)."; // Removed verbose log
}

void GeminiAI::setExternalEnginePath(const QString& path)
{
    m_externalEnginePath = path;
    // qDebug() << QString("GeminiAI: External engine path set to: %1").arg(path); // Removed verbose log
}

void GeminiAI::setSecondaryExternalEnginePath(const QString& path)
{
    m_secondaryExternalEnginePath = path;
    // qDebug() << QString("GeminiAI: Secondary external engine path set to: %1").arg(path); // Removed verbose log
}

void GeminiAI::setEgdbPath(const QString& path)
{
    qInfo() << QString("GeminiAI: EGDB path set to: %1").arg(path);
    if (m_egdbInitialized) {
        db_exit(); // Close existing EGDB before reinitializing
    }
    m_egdbPath = path;
    init(); // Reinitialize EGDB with the new path
}

void GeminiAI::requestMove(Board8x8 board, int colorToMove, double timeLimit)
{
    qDebug() << QString("GeminiAI: requestMove called. Color: %1, Time Limit: %2").arg(colorToMove).arg(timeLimit);

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
                qWarning() << QString("GeminiAI: Could not parse bestmove string: %1").arg(bestMoveStr);
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
    // qDebug() << "GeminiAI: doWork (placeholder)."; // Removed verbose log
}

void GeminiAI::initEngineProcess()
{
    // qDebug() << "GeminiAI: initEngineProcess called."; // Removed verbose log
    if (!m_externalEnginePath.isEmpty()) {
        m_primaryExternalEngine = new ExternalEngine(m_externalEnginePath, this);
        if (m_primaryExternalEngine->startEngine()) {
            qInfo() << QString("GeminiAI: Primary external engine started: %1").arg(m_externalEnginePath);
            m_useExternalEngine = true;
        } else {
            qCritical() << QString("GeminiAI: Failed to start primary external engine: %1").arg(m_externalEnginePath);
            m_useExternalEngine = false;
        }
    }
    if (!m_secondaryExternalEnginePath.isEmpty()) {
        m_secondaryExternalEngine = new ExternalEngine(m_secondaryExternalEnginePath, this);
        if (m_secondaryExternalEngine->startEngine()) {
            qInfo() << QString("GeminiAI: Secondary external engine started: %1").arg(m_secondaryExternalEnginePath);
        } else {
            qCritical() << QString("GeminiAI: Failed to start secondary external engine: %1").arg(m_secondaryExternalEnginePath);
        }
    }
}

void GeminiAI::quitEngineProcess()
{
    // qDebug() << "GeminiAI: quitEngineProcess called."; // Removed verbose log
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
    // qDebug() << "GeminiAI: startAnalyzeGame called."; // Removed verbose log
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
    // qDebug() << "GeminiAI: startAutoplay (placeholder)."; // Removed verbose log
}

void GeminiAI::startEngineMatch(int numGames, const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    // qDebug() << QString("GeminiAI: startEngineMatch (placeholder) for %1 games.").arg(numGames); // Removed verbose log
}

void GeminiAI::startRunTestSet(const Board8x8& board, int colorToMove)
{
    Q_UNUSED(board);
    Q_UNUSED(colorToMove);
    // qDebug() << "GeminiAI: startRunTestSet (placeholder)."; // Removed verbose log
}

void GeminiAI::startAnalyzePdn(const Board8x8& board, int colorToMove)
{
    // qDebug() << "GeminiAI: startAnalyzePdn called."; // Removed verbose log
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
    // qDebug() << "GeminiAI: abortSearch (placeholder)."; // Removed verbose log
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
    m_transpositionTable.clear(); // Clear transposition table for each new search

    char fen_c[256];
    board8toFEN(&board, fen_c, color, GT_ENGLISH);
    qDebug() << QString("GeminiAI::getBestMove: Entering with maxtime: %1, color: %2, FEN: %3").arg(maxtime).arg(color == CB_WHITE ? "WHITE" : "BLACK").arg(fen_c);

    CBmove bestMoveRoot = {0}; // Stores the best move found by iterative deepening
    int bestValueRoot = LOSS_SCORE;
    CBmove currentIterationBestMove = {0};
    int currentIterationBestValue = LOSS_SCORE;
    int actualSearchDepth = 0; // To store the depth reached

    QElapsedTimer timer;
    timer.start();

    // Convert maxtime from seconds to milliseconds for QElapsedTimer
    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);
    // qDebug() << QString("GeminiAI::getBestMove: maxtime: %1s, timeLimitMs: %2ms").arg(maxtime).arg(timeLimitMs); // Removed verbose log
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

    pos current_pos_egdb; // Declare here
    int egdb_result = DB_UNKNOWN; // Declare and initialize here

    // EGDB Lookup if in endgame (existing logic)
    if (m_egdbInitialized && totalPieces <= m_maxEGDBPieces) {
        current_pos_egdb = boardToPosition(board, color); // Assign here
        egdb_result = dblookup(&current_pos_egdb, color == CB_WHITE ? DB_WHITE : DB_BLACK);

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            CBmove legalMoves[MAXMOVES];
            int nmoves = 0;
            int isjump = 0;
            bool dummy_can_continue_multijump = false;
            get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

            // qDebug() << QString("GeminiAI::getBestMove: EGDB lookup - Legal moves found: %1").arg(nmoves); // Removed verbose log

            if (nmoves == 0) {
                m_lastEvaluationScore = 0; // No moves, draw or loss
                m_lastSearchDepth = 0; // No search performed
                qWarning() << "GeminiAI::getBestMove: EGDB lookup - No legal moves, returning default.";
                CBmove bestMoveRoot;
                bestMoveRoot.from.x = bestMoveRoot.from.y = bestMoveRoot.to.x = bestMoveRoot.to.y = 0;
                bestMoveRoot.is_capture = 0;
                return bestMoveRoot;
            }

            for (int i = 0; i < nmoves; ++i) {
                Board8x8 nextBoard = board;
                domove_c(&legalMoves[i], &nextBoard);

                pos next_pos_egdb = boardToPosition(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE);
                int next_egdb_result = dblookup(&next_pos_egdb, (color == CB_WHITE) ? DB_WHITE : DB_BLACK);

                if (egdb_result == DB_WIN) {
                    if (next_egdb_result == DB_LOSS) {
                        m_lastEvaluationScore = std::numeric_limits<int>::max(); // Winning move
                        m_lastSearchDepth = 0; // No search performed
                        // qDebug() << QString("GeminiAI::getBestMove: EGDB lookup - Found winning move: from(%1,%2) to(%3,%4)").arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y); // Removed verbose log
                        return legalMoves[i];
                    }
                } else if (egdb_result == DB_DRAW) {
                    if (next_egdb_result == DB_DRAW) {
                        m_lastEvaluationScore = 0; // Drawing move
                        m_lastSearchDepth = 0; // No search performed
                        // qDebug() << QString("GeminiAI::getBestMove: EGDB lookup - Found drawing move: from(%1,%2) to(%3,%4)").arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y); // Removed verbose log
                        return legalMoves[i];
                    }
                }
            }
        }
    }

    // Iterative Deepening
    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        // qDebug() << QString("GeminiAI::getBestMove: Starting iteration with depth: %1").arg(current_depth); // Removed verbose log
        if (m_abortRequested.loadRelaxed()) {
            qInfo() << "GeminiAI::getBestMove: Abort requested during iterative deepening.";
            break; // Abort if requested
        }

        // Check if enough time remains for the next iteration
        qint64 elapsed = timer.elapsed();
        // qDebug() << QString("GeminiAI::getBestMove: Elapsed time: %1ms").arg(elapsed); // Removed verbose log
        if (elapsed > timeLimitMs && current_depth > 1) { // Only break if not the first iteration
            qWarning() << QString("GeminiAI::getBestMove: Time limit exceeded (%1ms > %2ms), breaking iterative deepening.").arg(elapsed).arg(timeLimitMs);
            break;
        }
        // For the first iteration, ensure at least MIN_TIME_FOR_FIRST_ITERATION_MS is available
        if (current_depth == 1 && elapsed > timeLimitMs - MIN_TIME_FOR_FIRST_ITERATION_MS && timeLimitMs > MIN_TIME_FOR_FIRST_ITERATION_MS) {
             qWarning() << QString("GeminiAI::getBestMove: Not enough time for first iteration, elapsed: %1ms, timeLimitMs: %2ms").arg(elapsed).arg(timeLimitMs);
             break;
        }


    CBmove moves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, moves, &nmoves, &isjump, NULL, NULL);

    if (nmoves == 0) {
        // No legal moves, game over. This should ideally be handled before iterative deepening.
        // But as a safeguard, if no moves, return the default bestMoveRoot.
        m_lastEvaluationScore = LOSS_SCORE;
        m_lastSearchDepth = 0; // Store the depth reached
        qWarning() << "GeminiAI::getBestMove: No legal moves at root, returning default bestMoveRoot.";
        CBmove bestMoveRoot;
        bestMoveRoot.from.x = bestMoveRoot.from.y = bestMoveRoot.to.x = bestMoveRoot.to.y = 0;
        bestMoveRoot.is_capture = 0;
        return bestMoveRoot;
    }

    // Separate captures from non-captures
    std::vector<CBmove> captureMoves;
    std::vector<CBmove> nonCaptureMoves;
    for (int i = 0; i < nmoves; ++i) {
        if (moves[i].is_capture) {
            captureMoves.push_back(moves[i]);
        } else {
            nonCaptureMoves.push_back(moves[i]);
        }
    }

    // If there are captures, only consider those moves
    std::vector<CBmove>* movesToConsider = &nonCaptureMoves;
    if (!captureMoves.empty()) {
        movesToConsider = &captureMoves;
        qDebug() << QString("GeminiAI::getBestMove: %1 capture moves available. Prioritizing captures.").arg(captureMoves.size());
    } else {
        qDebug() << QString("GeminiAI::getBestMove: No capture moves available. Considering %1 non-capture moves.").arg(nonCaptureMoves.size());
    }

    // Handle immediate forced moves (single capture or single non-capture move)
    if (movesToConsider->size() == 1) {
        qInfo() << "GeminiAI::getBestMove: Only one legal move (capture or non-capture), returning it immediately.";
        m_lastEvaluationScore = evaluateBoard(board, color); // Evaluate current board for score
        m_lastSearchDepth = 0;
        return movesToConsider->at(0);
    }

    // Sort moves to prioritize captures (already done by the captureMoves/nonCaptureMoves separation)
    // For non-captures, a more advanced heuristic could be used.
    // For now, we'll just sort them by a simple heuristic if needed, or rely on the search.
    // std::sort(movesToConsider->begin(), movesToConsider->end(), compareMoves); // compareMoves is for CBmove array, not vector

        currentIterationBestValue = std::numeric_limits<int>::min();
        CBmove iterationBestMove = {0}; // Best move for this iteration

        for (const auto& move : *movesToConsider) {
            if (m_abortRequested.loadRelaxed()) {
                qInfo() << QString("GeminiAI::getBestMove: Abort requested during move iteration at depth %1.").arg(current_depth);
                break; // Abort if requested
            }

            Board8x8 nextBoard = board;
            domove_c(&move, &nextBoard); // Make the move
            char next_fen_c[256];
            board8toFEN(&nextBoard, next_fen_c, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, GT_ENGLISH);
            qDebug() << QString("GeminiAI::getBestMove: Considering move from(%1,%2) to(%3,%4). Board FEN after move: %5").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(next_fen_c);

            // Call minimax with the current iteration depth
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), nullptr, true);
            qDebug() << QString("GeminiAI::getBestMove: Depth %1, Move from(%2,%3) to(%4,%5), Value: %6").arg(current_depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(moveValue);

            if (moveValue > currentIterationBestValue) {
                currentIterationBestValue = moveValue;
                iterationBestMove = move;
            }
        }

        // If aborted during an iteration, break out of iterative deepening
        if (m_abortRequested.loadRelaxed()) {
            qInfo() << "GeminiAI::getBestMove: Abort requested after move iteration.";
            break;
        }

        // If a move was found in this iteration, update the overall best move
        if (currentIterationBestValue > std::numeric_limits<int>::min()) {
            bestValueRoot = currentIterationBestValue;
            bestMoveRoot = iterationBestMove;
            actualSearchDepth = current_depth; // Update the actual depth reached
            // qDebug() << QString("GeminiAI::getBestMove: Depth %1 - Best move found: from(%2,%3) to(%4,%5), value: %6").arg(current_depth).arg(bestMoveRoot.from.x).arg(bestMoveRoot.from.y).arg(bestMoveRoot.to.x).arg(bestMoveRoot.to.y).arg(bestValueRoot); // Removed verbose log
        } else {
            qWarning() << QString("GeminiAI::getBestMove: Depth %1 - No valid move found in this iteration.").arg(current_depth);
        }
    }

    m_lastEvaluationScore = bestValueRoot; // Store the final evaluation score
    m_lastSearchDepth = actualSearchDepth; // Store the final search depth
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth); // Emit the signal with the final results
    qInfo() << QString("--- getBestMove: End ---");
    qInfo() << QString("Final best move: from(%1,%2) to(%3,%4), score: %5, depth: %6").arg(bestMoveRoot.from.x).arg(bestMoveRoot.from.y).arg(bestMoveRoot.to.x).arg(bestMoveRoot.to.y).arg(m_lastEvaluationScore).arg(m_lastSearchDepth);
    return bestMoveRoot;
}

int GeminiAI::countLegalMoves(const Board8x8& board, int color)
{
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    return nmoves;
}

int GeminiAI::evaluateBoard(const Board8x8& board, int color)
{
    int evaluation = 0;
    int white_pieces = 0;
    int black_pieces = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            bool is_king = (piece & CB_KING);
            int piece_value = is_king ? 400 : 200;

            if (piece_color == CB_WHITE) {
                evaluation += piece_value;
                evaluation += is_king ? whiteKingPST[r][c] : whiteManPST[r][c];
                white_pieces++;
            } else {
                evaluation -= piece_value;
                evaluation -= is_king ? blackKingPST[r][c] : blackManPST[r][c];
                black_pieces++;
            }
        }
    }

    // Now, check for threats separately. This is cleaner.
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if (piece == CB_EMPTY) continue;

            int piece_color = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
            int opponent_color = (piece_color == CB_WHITE) ? CB_BLACK : CB_WHITE;

            if (isSquareAttacked(board, r, c, opponent_color)) {
                bool defended = isPieceDefended(board, r, c, piece_color);
                int piece_value = (piece & CB_KING) ? 400 : 200;
                int penalty = piece_value; // Default penalty is piece value

                if (!defended) {
                    penalty *= 2; // Double penalty if undefended
                    qDebug() << QString("GeminiAI::evaluateBoard: Undefended piece attacked at (%1,%2). Penalty: %3").arg(r).arg(c).arg(penalty);
                } else {
                    qDebug() << QString("GeminiAI::evaluateBoard: Defended piece attacked at (%1,%2). Penalty: %3").arg(r).arg(c).arg(penalty);
                }

                if (piece_color == CB_WHITE) {
                    evaluation -= penalty; // Penalty for white piece being attacked
                } else {
                    evaluation += penalty; // "Bonus" for black piece being attacked (from white's perspective)
                }
            }
        }
    }

    // Add a bonus for material advantage
    // Mobility: Award points for having more legal moves
    int whiteMobility = countLegalMoves(board, CB_WHITE);
    int blackMobility = countLegalMoves(board, CB_BLACK);

    evaluation += (whiteMobility - blackMobility) * 10; // Adjust multiplier as needed

    // Add king safety to evaluation
    evaluation += evaluateKingSafety(board, CB_WHITE);
    evaluation -= evaluateKingSafety(board, CB_BLACK);


    if (white_pieces == 0) return LOSS_SCORE;
    if (black_pieces == 0) return WIN_SCORE;

    if (color == CB_BLACK) {
        return -evaluation;
    }
    return evaluation;
}

int GeminiAI::minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
{
    int bestEval; // Declare bestEval here
    // qDebug() << QString("GeminiAI::minimax: Depth %1, Color %2, Alpha %3, Beta %4").arg(depth).arg(color).arg(alpha).arg(beta); // Removed verbose log
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << QString("GeminiAI::minimax: Abort requested at depth %1.").arg(depth);
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
        pos current_pos_egdb = boardToPosition(board, color);
        int egdb_result = dblookup(&current_pos_egdb, color == CB_WHITE ? DB_WHITE : DB_BLACK);

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            if (egdb_result == DB_WIN) {
                qDebug() << QString("GeminiAI::minimax: EGDB WIN at depth %1. Score: %2").arg(depth).arg(WIN_SCORE - depth);
                return WIN_SCORE - depth; // Win for current player, adjusted for depth
            } else if (egdb_result == DB_LOSS) {
                qDebug() << QString("GeminiAI::minimax: EGDB LOSS at depth %1. Score: %2").arg(depth).arg(LOSS_SCORE + depth);
                return LOSS_SCORE + depth; // Loss for current player, adjusted for depth
            } else if (egdb_result == DB_DRAW) {
                qDebug() << QString("GeminiAI::minimax: EGDB DRAW at depth %1. Score: 0").arg(depth);
                return 0; // Draw
            }
        }
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, NULL);
    if (allowNull && depth >= 3 && totalPieces > 8 && !isjump) {
        // Make a null move (pass the turn) and search with reduced depth
        int nullMoveEval = -minimax(board, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1 - R, -beta, -beta + 1, nullptr, false);

        if (nullMoveEval >= beta) {
            // If the null move search causes a beta cutoff, we can prune this branch
            // qDebug() << QString("GeminiAI::minimax: Null move cutoff at depth %1. Score: %2").arg(depth).arg(beta);
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
                // qDebug() << QString("GeminiAI::minimax: TT hit (EXACT) at depth %1, score: %2").arg(depth).arg(entry.score); // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) {
                // qDebug() << QString("GeminiAI::minimax: TT hit (ALPHA) at depth %1, score: %2").arg(depth).arg(entry.score); // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::BETA && entry.score >= beta) {
                // qDebug() << QString("GeminiAI::minimax: TT hit (BETA) at depth %1, score: %2").arg(depth).arg(entry.score); // Removed verbose log
                return entry.score;
            }
        }
        // If a best move is stored, use it for move ordering
        if (bestMove && entry.bestMove.from.x != 0) { // Assuming {0} is a null move
            *bestMove = entry.bestMove;
            // qDebug() << QString("GeminiAI::minimax: TT best move for ordering: from(%1,%2) to(%3,%4)").arg(bestMove->from.x).arg(bestMove->from.y).arg(bestMove->to.x).arg(bestMove->to.y); // Removed verbose log
        }
    }

    if (depth == 0) {
        int q_score = quiescenceSearch(board, color, alpha, beta);
        // qDebug() << QString("GeminiAI::minimax: Depth 0, Quiescence Search score: %1").arg(q_score); // Removed verbose log
        return q_score; // Call quiescence search at depth 0
    }

    // qDebug() << QString("GeminiAI::minimax: Depth %1, Color %2, Legal moves found: %3").arg(depth).arg(color).arg(nmoves); // Removed verbose log

    if (nmoves == 0) {
        // If no legal moves, the current player loses.
        // Return a score that is worse the sooner the loss is
        // qDebug() << QString("GeminiAI::minimax: Depth %1, Color %2, No legal moves, returning losing score.").arg(depth).arg(color); // Removed verbose log
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
        int eval; // Declare eval here

        if (m_abortRequested.loadRelaxed()) {
            qInfo() << QString("GeminiAI::minimax: Abort requested during move iteration at depth %1.").arg(depth);
            break; // Abort if requested
        }

        Board8x8 newBoard = board;
        domove_c(&move, &newBoard); // Make the move

        // Safety check: If the move immediately leads to an undefended piece being attacked, penalize it heavily
        if (!isMoveSafe(board, color, move)) { // Pass original board and move for context
            eval = LOSS_SCORE + depth; // Assign a very low score
            qDebug() << QString("GeminiAI::minimax: Unsafe move detected and penalized: from(%1,%2) to(%3,%4)").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y);
        } else {
            // Recursive call with negated alpha and beta for the opponent
            eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);
        }

        // qDebug() << QString("GeminiAI::minimax: Depth %1, Move from(%2,%3) to(%4,%5), Eval: %6").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(eval); // Removed verbose log

        if (eval > bestEval) {
            bestEval = eval;
            currentBestMove = move;
            if (bestMove) { // Only update bestMove if it's not nullptr (i.e., in the top-level call)
                *bestMove = move;
                // qDebug() << QString("GeminiAI::minimax: Depth %1, New best move: from(%2,%3) to(%4,%5), Eval: %6").arg(depth).arg(bestMove->from.x).arg(bestMove->from.y).arg(bestMove->to.x).arg(bestMove->to.y).arg(bestEval); // Removed verbose log
            }
        }

        alpha = std::max(alpha, eval); // Update alpha for the current node
        if (alpha >= beta) {
            // qDebug() << QString("GeminiAI::minimax: Alpha-beta cut-off. Alpha: %2, Beta: %3").arg(alpha).arg(beta); // Removed verbose log
            // Store killer move
            if (depth < MAX_DEPTH) {
                if (!(m_killerMoves[depth][0].from.x == move.from.x && m_killerMoves[depth][0].from.y == move.from.y &&
                      m_killerMoves[depth][0].to.x == move.to.x && m_killerMoves[depth][0].to.y == move.to.y)) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0]; // Shift existing killer to second slot
                    m_killerMoves[depth][0] = move; // Store new killer move
                    // qDebug() << QString("GeminiAI::minimax: Depth %1, Stored killer move: from(%2,%3) to(%4,%5)").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y); // Removed verbose log
                }
            }
            // Update history table
            m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x] += depth;
            // qDebug() << QString("GeminiAI::minimax: Depth %1, Updated history for move: from(%2,%3) to(%4,%5), new score: %6").arg(depth).arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x]); // Removed verbose log

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
    // qDebug() << QString("GeminiAI::minimax: Storing to TT: Depth %1, Score %2, Type %3, BestMove from(%4,%5) to(%6,%7)").arg(depth).arg(bestEval).arg(newEntry.type).arg(newEntry.bestMove.from.x).arg(newEntry.bestMove.from.y).arg(newEntry.bestMove.to.x).arg(newEntry.bestMove.to.y); // Removed verbose log

    return bestEval;
}

int GeminiAI::quiescenceSearch(Board8x8 board, int color, int alpha, int beta)
{
    // qDebug() << QString("GeminiAI::quiescenceSearch: Color %1, Alpha %2, Beta %3").arg(color).arg(alpha).arg(beta); // Removed verbose log
    // Evaluate the current board from the perspective of the current player
    int standPat = evaluateBoard(board, color);
    // qDebug() << QString("GeminiAI::quiescenceSearch: Stand pat score: %1").arg(standPat); // Removed verbose log

    if (standPat >= beta) {
        // qDebug() << QString("GeminiAI::quiescenceSearch: Beta cutoff. Returning beta: %1").arg(beta); // Removed verbose log
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, NULL);

    // If there are no captures, return the static evaluation
    if (isjump == 0) {
        // qDebug() << QString("GeminiAI::quiescenceSearch: No captures. Returning stand_pat: %1").arg(standPat); // Removed verbose log
        return standPat;
    }

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) { // Only consider captures in quiescence search
            Board8x8 newBoard = board;
            domove_c(&legalMoves[i], &newBoard); // Make the move

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha); // Recursively call quiescence search for the opponent
            // qDebug() << QString("GeminiAI::quiescenceSearch: Move %1, Score %2").arg(i).arg(score); // Removed verbose log

            if (score >= beta) {
                // qDebug() << QString("GeminiAI::quiescenceSearch: Beta cutoff after move. Returning beta: %1").arg(beta); // Removed verbose log
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    // qDebug() << QString("GeminiAI::quiescenceSearch: Returning alpha: %1").arg(alpha); // Removed verbose log
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
    get_legal_moves_c(&board, colorToMove, legalMoves, &nmoves, &isjump, NULL, NULL);
    return isjump == 1;
}

bool GeminiAI::isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor)
{
    // Check the 4 diagonal directions for an attack
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        // Square of the potential attacker
        int attacker_r = r + dr[i];
        int attacker_c = c + dc[i];

        // Square where the attacker would land
        int landing_r = r - dr[i];
        int landing_c = c - dc[i];

        // Check bounds
        if (attacker_r < 0 || attacker_r >= 8 || attacker_c < 0 || attacker_c >= 8 ||
            landing_r < 0 || landing_r >= 8 || landing_c < 0 || landing_c >= 8) {
            continue;
        }

        // Check if landing square is empty
        if (board.board[landing_r][landing_c] != CB_EMPTY) {
            continue;
        }

        int attacker_piece = board.board[attacker_r][attacker_c];

        // If there's no piece, or it's the wrong color, it can't attack
        if (attacker_piece == CB_EMPTY || (attacker_piece & attackerColor) == 0) {
            continue;
        }

        // Now check if the piece can legally make that capture
        if ((attacker_piece & CB_KING)) {
            // Kings can always capture from any diagonal
            return true;
        } else { // It's a man
            // White men can only capture "forward" (victim is at a higher row index)
            if (attackerColor == CB_WHITE && dr[i] < 0) { // Attacker is at r-1, victim at r
                return true;
            }
            // Black men can only capture "forward" (victim is at a lower row index)
            if (attackerColor == CB_BLACK && dr[i] > 0) { // Attacker is at r+1, victim at r
                return true;
            }
        }
    }

    return false;
}

bool GeminiAI::isPieceDefended(const Board8x8& board, int r, int c, int pieceColor)
{
    // Check the 4 diagonal directions for a friendly piece that could defend (r, c)
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        // Square of the potential defender
        int defender_r = r + dr[i];
        int defender_c = c + dc[i];

        // Check bounds
        if (defender_r < 0 || defender_r >= 8 || defender_c < 0 || defender_c >= 8) {
            continue;
        }

        int defender_piece = board.board[defender_r][defender_c];

        // If there's no piece, or it's not a friendly piece, it can't defend
        if (defender_piece == CB_EMPTY || (defender_piece & pieceColor) == 0) {
            continue;
        }

        // Now check if the piece can legally move to (r, c) if (r, c) were empty
        // This is a simplified check, assuming (r,c) is an empty square for the defender to move into
        // For a true defense, the defender would need to be able to move to (r,c)
        // and (r,c) would need to be attacked by an opponent.
        // For now, we check if a friendly piece is adjacent diagonally.

        bool is_king = (defender_piece & CB_KING);

        if (is_king) {
            return true; // Kings can defend in all diagonal directions
        } else { // It's a man
            // White men can defend "backward" (from their perspective)
            if (pieceColor == CB_WHITE && dr[i] > 0) { // Defender is at r+1, piece at r
                return true;
            }
            // Black men can defend "backward" (from their perspective)
            if (pieceColor == CB_BLACK && dr[i] < 0) { // Defender is at r-1, piece at r
                return true;
            }
        }
    }

    return false;
}

bool GeminiAI::isMoveSafe(const Board8x8& board, int color, const CBmove& move)
{
    Board8x8 temp_board = board;
    domove_c(&move, &temp_board); // Apply the move to a temporary board

    int opponent_color = (color == CB_WHITE) ? CB_BLACK : CB_WHITE;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = temp_board.board[r][c];
            if (piece == CB_EMPTY) continue;

            // Check only current player's pieces
            if ((piece & color) == 0) continue;

            // If the piece is attacked by the opponent and is not defended, the move is unsafe
            if (isSquareAttacked(temp_board, r, c, opponent_color) && !isPieceDefended(temp_board, r, c, color)) {
                // For now, any undefended attacked piece makes the move unsafe.
                // A more advanced check would consider material balance after the exchange.
                qDebug() << QString("GeminiAI::isMoveSafe: Unsafe move detected. Piece at (%1,%2) is attacked and undefended after move.").arg(r).arg(c);
                return false;
            }
        }
    }
    return true; // No immediate undefended attacked pieces found
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

int GeminiAI::evaluateKingSafety(const Board8x8& board, int color)
{
    int safetyScore = 0;
    int king_r = -1, king_c = -1;

    // Find the king of the given color
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if ((piece & color) && (piece & CB_KING)) {
                king_r = r;
                king_c = c;
                break;
            }
        }
        if (king_r != -1) break;
    }

    if (king_r == -1) {
        // No king found (e.g., in an endgame where kings are captured or not on board yet)
        return 0;
    }

    int opponent_color = (color == CB_WHITE) ? CB_BLACK : CB_WHITE;

    // Evaluate squares around the king
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue; // Skip the king's own square

            int r_neighbor = king_r + dr;
            int c_neighbor = king_c + dc;

            if (r_neighbor >= 0 && r_neighbor < 8 && c_neighbor >= 0 && c_neighbor < 8) {
                // Check if the square is attacked by an opponent piece
                if (isSquareAttacked(board, r_neighbor, c_neighbor, opponent_color)) {
                    safetyScore -= 20; // Penalize if a square around the king is attacked
                }
                // Check for friendly pawns (men) forming a shield
                int piece_at_neighbor = board.board[r_neighbor][c_neighbor];
                if ((piece_at_neighbor & color) && (piece_at_neighbor & CB_MAN)) {
                    // Reward pawns in front of the king
                    if (color == CB_WHITE && r_neighbor > king_r) { // White pawns moving "up" the board
                        safetyScore += 10;
                    } else if (color == CB_BLACK && r_neighbor < king_r) { // Black pawns moving "down" the board
                        safetyScore += 10;
                    }
                }
            }
        }
    }

    return safetyScore;
}
