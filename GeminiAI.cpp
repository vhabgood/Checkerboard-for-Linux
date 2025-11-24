#include <QDebug>
#include "GeminiAI.h"
#include <limits>
#include <QElapsedTimer>
#include "GameManager.h"

extern "C" {
#include "checkers_c_types.h" // For common C types and defines
#include "c_logic.h"          // For domove_c, board8toFEN, get_legal_moves_c etc.
#include "dblookup.h"         // For db_init, db_exit, dblookup
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
    // qDebug() << "GeminiAI: External engine path set to: " << path; // Removed verbose log
}

void GeminiAI::setSecondaryExternalEnginePath(const QString& path)
{
    m_secondaryExternalEnginePath = path;
    // qDebug() << "GeminiAI: Secondary external engine path set to: " << path; // Removed verbose log
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
    // qDebug() << "GeminiAI: doWork (placeholder)."; // Removed verbose log
}

void GeminiAI::initEngineProcess()
{
    // qDebug() << "GeminiAI: initEngineProcess called."; // Removed verbose log
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
    // qDebug() << "GeminiAI: startEngineMatch (placeholder) for " << numGames << " games."; // Removed verbose log
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
    qDebug() << "GeminiAI::getBestMove: Entering with maxtime: " << maxtime << ", color: " << (color == CB_WHITE ? "WHITE" : "BLACK") << ", FEN: " << fen_c;

    CBmove bestMove = {0};
    int bestValue = LOSS_SCORE;
    int actualSearchDepth = 0; // Initialize here
    int current_depth; // Loop variable for iterative deepening

    QElapsedTimer timer;
    timer.start();

    qint64 timeLimitMs = static_cast<qint64>(maxtime * 1000);
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
    if (m_egdbInitialized && totalPieces <= m_maxEGDBPieces && !hasCaptures(board, color)) {
        current_pos_egdb = boardToPosition(board, color); // Assign here
        egdb_result = dblookup(&current_pos_egdb, 0);

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            CBmove legalMoves[MAXMOVES];
            int nmoves = 0;
            int isjump = 0;
            bool dummy_can_continue_multijump = false;
            get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);

            if (nmoves == 0) {
                m_lastEvaluationScore = 0; // No moves, draw or loss
                m_lastSearchDepth = 0; // No search performed
                qWarning() << "GeminiAI::getBestMove: EGDB lookup - No legal moves, returning default.";
                CBmove emptyMove = {0};
                return emptyMove;
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
                        return legalMoves[i];
                    }
                } else if (egdb_result == DB_DRAW) {
                    if (next_egdb_result == DB_DRAW) {
                        m_lastEvaluationScore = 0; // Drawing move
                        m_lastSearchDepth = 0; // No search performed
                        return legalMoves[i];
                    }
                }
            }
        }
    }
    
    // Get all legal moves once before iterative deepening to avoid recalculating
    // and to ensure bestMove is initialized with a valid move.
    CBmove initialLegalMoves[MAXMOVES];
    int initialNmoves = 0;
    int initialIsjump = 0;
    bool dummy_can_continue_multijump = false; // This variable is not used after this point
    get_legal_moves_c(&board, color, initialLegalMoves, &initialNmoves, &initialIsjump, NULL, &dummy_can_continue_multijump);

    if (initialNmoves == 0) {
        m_lastEvaluationScore = LOSS_SCORE;
        m_lastSearchDepth = 0;
        qWarning() << "GeminiAI::getBestMove: No legal moves at root. Returning default empty move.";
        CBmove emptyMove = {0};
        return emptyMove;
    }
    
    std::vector<CBmove> moves_to_consider;
    if (initialIsjump) {
        for (int i = 0; i < initialNmoves; ++i) {
            if (initialLegalMoves[i].is_capture) {
                moves_to_consider.push_back(initialLegalMoves[i]);
            }
        }
    } else {
        for (int i = 0; i < initialNmoves; ++i) {
            moves_to_consider.push_back(initialLegalMoves[i]);
        }
    }

    if (moves_to_consider.size() == 1) {
        qInfo() << "GeminiAI::getBestMove: Only one legal move, returning it immediately.";
        m_lastEvaluationScore = evaluateBoard(board, color);
        m_lastSearchDepth = 0;
        return moves_to_consider[0];
    }

    // Sort moves to prioritize captures (already done by the jump check, but good for non-jumps)
    std::sort(moves_to_consider.begin(), moves_to_consider.end(), compareMoves);
    
    // Initialize bestMove and bestValue with the first legal move
    bestMove = moves_to_consider[0];
    Board8x8 tempBoardForInitialEval = board;
    domove_c(&bestMove, &tempBoardForInitialEval);
    // Use depth 0 for initial evaluation to just get quiescence search result
    bestValue = -minimax(tempBoardForInitialEval, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, 0, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), nullptr, true);


    // Iterative Deepening
    for (current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        if (m_abortRequested.loadRelaxed()) {
            qInfo() << "GeminiAI::getBestMove: Abort requested during iterative deepening.";
            break; // Abort if requested
        }

        qint64 elapsed = timer.elapsed();
        if (elapsed > timeLimitMs && current_depth > 1) { // Only break if not the first iteration
            qWarning() << "GeminiAI::getBestMove: Time limit exceeded (" << elapsed << "ms > " << timeLimitMs << "ms), breaking iterative deepening.";
            break;
        }
        // Ensure first iteration gets at least MIN_TIME_FOR_FIRST_ITERATION_MS
        if (current_depth == 1 && elapsed > timeLimitMs - MIN_TIME_FOR_FIRST_ITERATION_MS && timeLimitMs > MIN_TIME_FOR_FIRST_ITERATION_MS) {
             qWarning() << "GeminiAI::getBestMove: Not enough time for first iteration, elapsed: " << elapsed << "ms, timeLimitMs: " << timeLimitMs << "ms";
             break;
        }

        CBmove iterationBestMove = {0}; // Best move for this iteration
        int currentIterationBestValue = LOSS_SCORE; // Also initialize this here

        // Separate captures from non-captures for current depth using initialLegalMoves
        std::vector<CBmove> captureMoves;
        std::vector<CBmove> nonCaptureMoves;
        for (int i = 0; i < initialNmoves; ++i) { 
            if (initialLegalMoves[i].is_capture) {
                captureMoves.push_back(initialLegalMoves[i]);
            } else {
                nonCaptureMoves.push_back(initialLegalMoves[i]);
            }
        }

        std::vector<CBmove>* movesToConsider = &nonCaptureMoves;
        if (!captureMoves.empty()) {
            movesToConsider = &captureMoves;
            qDebug() << "GeminiAI::getBestMove: " << captureMoves.size() << " capture moves available. Prioritizing captures.";
        } else {
            qDebug() << "GeminiAI::getBestMove: No capture moves available. Considering " << nonCaptureMoves.size() << " non-capture moves.";
        }

        if (movesToConsider->empty()) {
            qWarning() << "GeminiAI::getBestMove: No moves to consider for depth " << current_depth << ". Skipping iteration.";
            continue; // No moves to consider for this iteration, move to next depth
        }
        
        // Initialize currentIterationBestMove for this iteration with the first move to consider
        iterationBestMove = movesToConsider->at(0);
        // Evaluate the first move to set a baseline for currentIterationBestValue for this iteration
        Board8x8 tempBoard = board;
        domove_c(&iterationBestMove, &tempBoard);
        currentIterationBestValue = -minimax(tempBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), nullptr, true);


        int alpha = std::numeric_limits<int>::min(); // Reset alpha for this iteration
        int beta = std::numeric_limits<int>::max();  // Reset beta for this iteration

        for (const auto& move : *movesToConsider) {
            if (m_abortRequested.loadRelaxed()) {
                qInfo() << "GeminiAI::getBestMove: Abort requested during move iteration at depth " << current_depth;
                break; // Abort if requested
            }

            Board8x8 nextBoard = board;
            domove_c(&move, &nextBoard); // Make the move
            char next_fen_c[256];
            board8toFEN(&nextBoard, next_fen_c, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, GT_ENGLISH);
            qDebug() << "GeminiAI::getBestMove: Considering move from(" << move.from.x << "," << move.from.y << ") to(" << move.to.x << "," << move.to.y << "). Board FEN after move: " << next_fen_c;

            // Call minimax with the current iteration depth
            int moveValue = -minimax(nextBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, current_depth - 1, -beta, -alpha, nullptr, true);
            qDebug() << "GeminiAI::getBestMove: Depth " << current_depth << ", Move from(" << move.from.x << "," << move.from.y << ") to(" << move.to.x << "," << move.to.y << "), Value: " << moveValue;


            if (moveValue > currentIterationBestValue) {
                currentIterationBestValue = moveValue;
                iterationBestMove = move;
            }
            alpha = std::max(alpha, currentIterationBestValue);
        }

        // If aborted during an iteration, break out of iterative deepening
        if (m_abortRequested.loadRelaxed()) {
            qInfo() << "GeminiAI::getBestMove: Abort requested after move iteration.";
            break;
        }

        // If a move was found in this iteration, update the overall best move
        if (currentIterationBestValue > std::numeric_limits<int>::min()) {
            bestValue = currentIterationBestValue;
            bestMove = iterationBestMove;
            actualSearchDepth = current_depth; // Update the actual depth reached
            // qInfo() << "GeminiAI::getBestMove: Depth " << current_depth << " - Best move found: from(" << bestMove.from.x << "," << bestMove.from.y << ") to(" << bestMove.to.x << "," << bestMove.to.y << "), value: " << bestValue; // Removed verbose log
        } else {
            qWarning() << "GeminiAI::getBestMove: Depth " << current_depth << " - No valid move found in this iteration.";
        }
    }

    m_lastEvaluationScore = bestValue; // Store the final evaluation score
    m_lastSearchDepth = actualSearchDepth; // Store the final search depth
    emit evaluationReady(m_lastEvaluationScore, m_lastSearchDepth); // Emit the signal with the final results
    qInfo() << "--- getBestMove: End ---";
    qInfo() << "Final best move: from(" << bestMove.from.x << "," << bestMove.from.y << ") to(" << bestMove.to.x << "," << bestMove.to.y << "), score: " << m_lastEvaluationScore << ", depth: " << m_lastSearchDepth;
    return bestMove;
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
            int piece_value = is_king ? KING_VALUE : MAN_VALUE;

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
                int piece_value = (piece & CB_KING) ? KING_VALUE : MAN_VALUE;
                int penalty = piece_value; // Default penalty is piece value

                if (!defended) {
                    penalty *= THREAT_PENALTY_MULTIPLIER; // Significantly increased penalty if undefended
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
    CBmove legalMoves[MAXMOVES];
    int nmovesCurrentPlayer = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmovesCurrentPlayer, &isjump, NULL, &dummy_can_continue_multijump);

    int nmovesOpponent = 0;
    get_legal_moves_c(&board, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, legalMoves, &nmovesOpponent, &isjump, NULL, &dummy_can_continue_multijump);
    evaluation += (nmovesCurrentPlayer - nmovesOpponent) * MOBILITY_MULTIPLIER;

    evaluation += evaluateKingSafety(board, CB_WHITE);
    evaluation -= evaluateKingSafety(board, CB_BLACK);
    evaluation += evaluatePassedMen(board);
    evaluation += evaluateBridge(board);
    evaluation += evaluateCenterControl(board);

    if (white_pieces == 0) {
        evaluation = LOSS_SCORE;
    } else if (black_pieces == 0) {
        evaluation = WIN_SCORE;
    }

    if (color == CB_BLACK) {
        return -evaluation;
    }
    return evaluation;
}

bool GeminiAI::isMoveSafe(const Board8x8& board_after_move, int color)
{
    // The board passed in should ALREADY have the move applied.
    int opponent_color = (color == CB_WHITE) ? CB_BLACK : CB_WHITE;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board_after_move.board[r][c];
            if (piece == CB_EMPTY) continue;

            // Check only current player's pieces
            if ((piece & color) == 0) continue;

            // If the piece is attacked by the opponent and is not defended, the move is unsafe
            if (isSquareAttacked(board_after_move, r, c, opponent_color) && !isPieceDefended(board_after_move, r, c, color)) {
                // For now, any undefended attacked piece makes the move unsafe.
                // A more advanced check would consider material balance after the exchange.
                qDebug() << "GeminiAI::isMoveSafe: Unsafe move detected. Piece at (" << r << "," << c << ") is attacked and undefended after move.";
                return false;
            }
        }
    }
    return true; // No immediate undefended attacked pieces found
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
                    safetyScore -= (MAN_VALUE / 5); // Penalize if a square around the king is attacked (20 points for MAN_VALUE=100)
                }
                // Check for friendly pawns (men) forming a shield
                int piece_at_neighbor = board.board[r_neighbor][c_neighbor];
                if ((piece_at_neighbor & color) && (piece_at_neighbor & CB_MAN)) {
                    // Reward pawns in front of the king
                    if (color == CB_WHITE && r_neighbor > king_r) { // White pawns moving "up" the board
                        safetyScore += (MAN_VALUE / 10); // 10 points for MAN_VALUE=100
                    } else if (color == CB_BLACK && r_neighbor < king_r) { // Black pawns moving "down" the board
                        safetyScore += (MAN_VALUE / 10); // 10 points for MAN_VALUE=100
                    }
                }
            }
        }
    }

    return safetyScore;
}

bool GeminiAI::isManPassed(const Board8x8& board, int r, int c) {
    int piece = board.board[r][c];
    int pieceColor = (piece & CB_WHITE) ? CB_WHITE : CB_BLACK;
    int opponentColor = (pieceColor == CB_WHITE) ? CB_BLACK : CB_WHITE;

    if (pieceColor == CB_WHITE) {
        // A white piece at (r, c) is passed if no black pieces exist in rows > r.
        for (int row = r + 1; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if ((board.board[row][col] & opponentColor)) {
                    return false;
                }
            }
        }
    } else { // Black piece
        // A black piece at (r, c) is passed if no white pieces exist in rows < r.
        for (int row = 0; row < r; ++row) {
            for (int col = 0; col < 8; ++col) {
                if ((board.board[row][col] & opponentColor)) {
                    return false;
                }
            }
        }
    }
    return true;
}

int GeminiAI::evaluatePassedMen(const Board8x8& board)
{
    int score = 0;
    const int PASSED_MAN_MULTIPLIER = 3;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            if (piece == CB_EMPTY || (piece & CB_KING)) {
                continue;
            }

            int piece_value = 0;
            bool is_passed = isManPassed(board, r, c);

            if ((piece & CB_WHITE)) { // White man
                if (r >= 4) { // On opponent's side
                    piece_value = PASSED_MAN_BONUS * (r - 3);
                    if (is_passed) {
                        piece_value *= PASSED_MAN_MULTIPLIER;
                    }
                }
                score += piece_value;
            } else { // Black man
                if (r <= 3) { // On opponent's side
                    piece_value = PASSED_MAN_BONUS * (4 - r);
                    if (is_passed) {
                        piece_value *= PASSED_MAN_MULTIPLIER;
                    }
                }
                score -= piece_value;
            }
        }
    }
    return score;
}

int GeminiAI::evaluateBridge(const Board8x8& board)
{
    int score = 0;
    
    // Check for White's bridge (squares 29, 31)
    if ((board.board[0][4] & CB_WHITE) && (board.board[0][6] & CB_WHITE)) {
        score += BRIDGE_BONUS;
    }
    
    // Check for Black's bridge (squares 1, 3)
    if ((board.board[7][0] & CB_BLACK) && (board.board[7][2] & CB_BLACK)) {
        score -= BRIDGE_BONUS;
    }

    return score;
}

int GeminiAI::evaluateCenterControl(const Board8x8& board)
{
    int score = 0;
    // Center squares for an 8x8 board: 10,11,14,15,18,19,22,23
    // (2,3),(2,5),(3,2),(3,4),(4,3),(4,5),(5,2),(5,4)
    int center_squares[8][2] = {{2,3},{2,5},{3,2},{3,4},{4,3},{4,5},{5,2},{5,4}};

    for(int i=0; i<8; ++i) {
        int r = center_squares[i][0];
        int c = center_squares[i][1];
        int piece = board.board[r][c];
        if (piece != CB_EMPTY) {
            if ((piece & CB_WHITE)) {
                score += CENTER_BONUS;
            } else {
                score -= CENTER_BONUS;
            }
        }
    }
    return score;
}

int GeminiAI::minimax(Board8x8 board, int color, int depth, int alpha, int beta, CBmove *bestMove, bool allowNull)
{
    int bestEval; // Declare bestEval here
    // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Color " << color << ", Alpha " << alpha << ", Beta " << beta; // Removed verbose log
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "GeminiAI::minimax: Abort requested at depth " << depth;
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

    if (m_egdbInitialized && totalPieces <= m_maxEGDBPieces && !hasCaptures(board, color)) {
        char fen_c[256];
        board8toFEN(&board, fen_c, color, GT_ENGLISH);
        qDebug() << "GeminiAI::minimax: EGDB lookup. Pieces: " << totalPieces << ", FEN: " << fen_c << ", EGDB initialized: " << m_egdbInitialized;
        pos current_pos_egdb = boardToPosition(board, color);
        int egdb_result = dblookup(&current_pos_egdb, 1);
        qDebug() << "GeminiAI::minimax: EGDB result: " << egdb_result;

        if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
            if (egdb_result == DB_WIN) {
                qDebug() << "GeminiAI::minimax: EGDB WIN at depth " << depth << ". Score: " << WIN_SCORE - depth;
                return WIN_SCORE - depth; // Win for current player, adjusted for depth
            } else if (egdb_result == DB_LOSS) {
                qDebug() << "GeminiAI::minimax: EGDB LOSS at depth " << depth << ". Score: " << LOSS_SCORE + depth;
                return LOSS_SCORE + depth; // Loss for current player, adjusted for depth
            } else if (egdb_result == DB_DRAW) {
                qDebug() << "GeminiAI::minimax: EGDB DRAW at depth " << depth << ". Score: 0";
                return 0; // Draw
            }
        }
    }

    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, color, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    if (allowNull && depth >= 3 && totalPieces > 8 && !isjump) {
        // Make a null move (pass the turn) and search with reduced depth
        int nullMoveEval = -minimax(board, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1 - R, -beta, -beta + 1, nullptr, false);

        if (nullMoveEval >= beta) {
            // If the null move search causes a beta cutoff, we can prune this branch
            // qDebug() << "GeminiAI::minimax: Null move cutoff at depth " << depth << ". Score: " << beta;
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
                // qDebug() << "GeminiAI::minimax: TT hit (EXACT) at depth " << depth << ", score: " << entry.score; // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::ALPHA && entry.score <= alpha) {
                // qDebug() << "GeminiAI::minimax: TT hit (ALPHA) at depth " << depth << ", score: " << entry.score; // Removed verbose log
                return entry.score;
            }
            if (entry.type == TTEntry::BETA && entry.score >= beta) {
                // qDebug() << "GeminiAI::minimax: TT hit (BETA) at depth " << depth << ", score: " << entry.score; // Removed verbose log
                return entry.score;
            }
        }
        // If a best move is stored, use it for move ordering
        if (bestMove && entry.bestMove.from.x != 0) { // Assuming {0} is a null move
            *bestMove = entry.bestMove;
            // qDebug() << "GeminiAI::minimax: TT best move for ordering: from(" << bestMove->from.x << "," << bestMove->from.y << ") to(" << bestMove->to.x << "," << bestMove->to.y << ")"; // Removed verbose log
        }
    }

    if (depth == 0) {
        int q_score = quiescenceSearch(board, color, alpha, beta);
        // qDebug() << "GeminiAI::minimax: Depth 0, Quiescence Search score: " << q_score; // Removed verbose log
        return q_score; // Call quiescence search at depth 0
    }

    // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Color " << color << ", Legal moves found: " << nmoves; // Removed verbose log

    if (nmoves == 0) {
        // If no legal moves, the current player loses.
        // Return a score that is worse the sooner the loss is
        // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Color " << color << ", No legal moves, returning losing score."; // Removed verbose log
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
            qInfo() << "GeminiAI::minimax: Abort requested during move iteration at depth " << depth;
            break; // Abort if requested
        }

        Board8x8 newBoard = board;
        domove_c(&move, &newBoard); // Make the move

        if (!move.is_capture && !isMoveSafe(newBoard, color)) {
            eval = LOSS_SCORE; // Heavily penalize unsafe non-capture moves
            qDebug() << "GeminiAI::minimax: Unsafe non-capture move (" << move.from.x << "," << move.from.y << ")->(" << move.to.x << "," << move.to.y << ") at depth " << depth << ". Assigning LOSS_SCORE.";
        } else {
            eval = -minimax(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, depth - 1, -beta, -alpha, nullptr, true);
        }


        // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Move from(" << move.from.x << "," << move.from.y << ") to(" << move.to.x << "," << move.to.y << "), Eval: " << eval; // Removed verbose log

        if (eval > bestEval) {
            bestEval = eval;
            currentBestMove = move;
            if (bestMove) { // Only update bestMove if it's not nullptr (i.e., in the top-level call)
                *bestMove = move;
                // qDebug() << "GeminiAI::minimax: Depth " << depth << ", New best move: from(" << bestMove->from.x << "," << bestMove->from.y << ") to(" << bestMove->to.x << "," << bestMove->to.y << "), Eval: " << bestEval; // Removed verbose log
            }
        }

        alpha = std::max(alpha, eval); // Update alpha for the current node
        if (alpha >= beta) {
            // qDebug() << "GeminiAI::minimax: Alpha-beta cut-off. Alpha: " << alpha << ", Beta: " << beta; // Removed verbose log
            // Store killer move
            if (depth < MAX_DEPTH) {
                if (!(m_killerMoves[depth][0].from.x == move.from.x && m_killerMoves[depth][0].from.y == move.from.y &&
                      m_killerMoves[depth][0].to.x == move.to.x && m_killerMoves[depth][0].to.y == move.to.y)) {
                    m_killerMoves[depth][1] = m_killerMoves[depth][0]; // Shift existing killer to second slot
                    m_killerMoves[depth][0] = move; // Store new killer move
                    // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Stored killer move: from(" << move.from.x << "," << move.from.y << ") to(" << move.to.x << "," << move.to.y << ")"; // Removed verbose log
                }
            }
            // Update history table
            m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x] += depth;
            // qDebug() << "GeminiAI::minimax: Depth " << depth << ", Updated history for move: from(" << move.from.x << "," << move.from.y << ") to(" << move.to.x << "," << move.to.y << "), new score: " << m_historyTable[move.from.y][move.from.x][move.to.y][move.to.x]; // Removed verbose log

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
    // qDebug() << "GeminiAI::minimax: Storing to TT: Depth " << depth << ", Score " << bestEval << ", Type " << newEntry.type << ", BestMove from(" << newEntry.bestMove.from.x << "," << newEntry.bestMove.from.y << ") to(" << newEntry.bestMove.to.x << "," << newEntry.bestMove.to.y << ")"; // Removed verbose log

    return bestEval;
}

int GeminiAI::quiescenceSearch(Board8x8 board, int color, int alpha, int beta)
{
    // qDebug() << "GeminiAI::quiescenceSearch: Color " << color << ", Alpha " << alpha << ", Beta " << beta; // Removed verbose log
    // Evaluate the current board from the perspective of the current player
    int standPat = evaluateBoard(board, color);
    // qDebug() << "GeminiAI::quiescenceSearch: Stand pat score: " << standPat; // Removed verbose log

    if (standPat >= beta) {
        // qDebug() << "GeminiAI::quiescenceSearch: Beta cutoff. Returning beta: " << beta; // Removed verbose log
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

    // If there are no captures, return the static evaluation
    if (isjump == 0) {
        // qDebug() << "GeminiAI::quiescenceSearch: No captures. Returning stand_pat: " << standPat; // Removed verbose log
        return standPat;
    }

    for (int i = 0; i < nmoves; ++i) {
        if (legalMoves[i].is_capture) { // Only consider captures in quiescence search
            Board8x8 newBoard = board;
            domove_c(&legalMoves[i], &newBoard); // Make the move

            int score = -quiescenceSearch(newBoard, (color == CB_WHITE) ? CB_BLACK : CB_WHITE, -beta, -alpha); // Recursively call quiescence search for the opponent
            // qDebug() << "GeminiAI::quiescenceSearch: Move " << i << ", Score " << score; // Removed verbose log

            if (score >= beta) {
                // qDebug() << "GeminiAI::quiescenceSearch: Beta cutoff after move. Returning beta: " << beta; // Removed verbose log
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    // qDebug() << "GeminiAI::quiescenceSearch: Returning alpha: " << alpha; // Removed verbose log
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
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&board, colorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    return isjump == 1;
}

bool GeminiAI::isSquareAttacked(const Board8x8& board, int r, int c, int attackerColor)
{
    // This function checks if a piece at (r, c) is attacked by a piece of attackerColor.
    // The piece at (r,c) is assumed to be of the opposite color.

    // Check for jumps over the square (r, c)
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        // Attacker's position
        int attacker_r = r - dr[i];
        int attacker_c = c - dc[i];

        // Landing position
        int landing_r = r + dr[i];
        int landing_c = c + dc[i];

        if (attacker_r < 0 || attacker_r >= 8 || attacker_c < 0 || attacker_c >= 8 ||
            landing_r < 0 || landing_r >= 8 || landing_c < 0 || landing_c >= 8) {
            continue;
        }

        int attacker_piece = board.board[attacker_r][attacker_c];
        
        // If the landing spot is empty and there's an attacker piece in the right spot
        if (board.board[landing_r][landing_c] == CB_EMPTY && (attacker_piece & attackerColor)) {
            // It's a potential jump. Now, check if the piece can legally make that jump.
            if (attacker_piece & CB_KING) {
                return true; // Kings can jump in any direction
            } else { // It's a man
                // White men jump "down" the board (increasing row index)
                if (attackerColor == CB_WHITE && landing_r > attacker_r) {
                    return true;
                }
                // Black men jump "up" the board (decreasing row index)
                if (attackerColor == CB_BLACK && landing_r < attacker_r) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GeminiAI::isPieceDefended(const Board8x8& board, int r, int c, int pieceColor)
{
    int opponentColor = (pieceColor == CB_WHITE) ? CB_BLACK : CB_WHITE;

    // Check the 4 diagonal directions for a potential capture of the piece at (r, c)
    int dr[] = {-1, -1, 1, 1};
    int dc[] = {-1, 1, -1, 1};

    for (int i = 0; i < 4; ++i) {
        // The square an opponent might jump FROM
        int attacker_r = r + dr[i];
        int attacker_c = c + dc[i];

        // The square the opponent would land ON
        int landing_r = r - dr[i];
        int landing_c = c - dc[i];

        // Check if the jump is within board boundaries
        if (attacker_r < 0 || attacker_r >= 8 || attacker_c < 0 || attacker_c >= 8 ||
            landing_r < 0 || landing_r >= 8 || landing_c < 0 || landing_c >= 8) {
            continue;
        }

        // If the landing square isn't empty, it's not a valid jump
        if (board.board[landing_r][landing_c] != CB_EMPTY) {
            continue;
        }

        int attacker_piece = board.board[attacker_r][attacker_c];

        // Check if there is an opponent piece that can perform the jump
        if ((attacker_piece & opponentColor)) {
            bool is_valid_capture = false;
            if (attacker_piece & CB_KING) {
                // Kings can always capture
                is_valid_capture = true;
            } else {
                // Check man capture direction
                // A white man at attacker_r captures a piece at r, must move to a lower row index (landing_r < attacker_r)
                if (opponentColor == CB_WHITE && landing_r < attacker_r) {
                    is_valid_capture = true;
                }
                // A black man at attacker_r captures a piece at r, must move to a higher row index (landing_r > attacker_r)
                else if (opponentColor == CB_BLACK && landing_r > attacker_r) {
                    is_valid_capture = true;
                }
            }

            if (is_valid_capture) {
                // If the capture is valid, simulate it and see if we can recapture the attacker
                Board8x8 board_after_capture = board;
                board_after_capture.board[r][c] = CB_EMPTY; // Piece is captured
                board_after_capture.board[attacker_r][attacker_c] = CB_EMPTY; // Attacker moves
                board_after_capture.board[landing_r][landing_c] = attacker_piece; // Attacker lands

                // Now, check if the landing square is attacked by one of our pieces (a recapture)
                if (isSquareAttacked(board_after_capture, landing_r, landing_c, pieceColor)) {
                    return true; // It's defended because we can trade back.
                }
            }
        }
    }

    // If no potential capture can be immediately avenged, the piece is not defended.
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
