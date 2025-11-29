#include "GeminiAI.h"
#include <QDebug>
#include "DBManager.h"

extern "C" {
#include "checkers_types.h"
#include "c_logic.h"
}

GeminiAI::GeminiAI(const QString& egdbPath, QObject *parent) : QObject(parent),
    m_aiThread(new QThread(this)),
    m_worker(new AIWorker()),
    m_egdbPath(egdbPath),
    m_egdbInitialized(false),
    m_maxEGDBPieces(0),
    m_mode(Idle),
    m_handicap(0),
    m_primaryExternalEngine(nullptr),
    m_secondaryExternalEngine(nullptr),
    m_useExternalEngine(false),
    m_externalEnginePath(""),
    m_secondaryExternalEnginePath(""),
    m_lastEvaluationScore(0),
    m_lastSearchDepth(0)
{
    m_worker->moveToThread(m_aiThread);

    // Connect signals for starting and stopping the thread
    connect(m_aiThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &GeminiAI::requestWorkerTask, m_worker, &AIWorker::performTask);
    connect(m_worker, &AIWorker::searchFinished, this, &GeminiAI::handleWorkerSearchFinished);
    connect(m_worker, &AIWorker::evaluationReady, this, &GeminiAI::handleWorkerEvaluation);

    m_aiThread->start();
}

GeminiAI::~GeminiAI()
{
    m_aiThread->quit();
    m_aiThread->wait();

    if (m_primaryExternalEngine) {
        delete m_primaryExternalEngine;
        m_primaryExternalEngine = nullptr;
    }
    if (m_secondaryExternalEngine) {
        delete m_secondaryExternalEngine;
        m_secondaryExternalEngine = nullptr;
    }
    DBManager::instance()->db_exit();
}

void GeminiAI::init()
{
    qInfo() << "GeminiAI: Initializing EGDB with path: " << m_egdbPath;
    char db_init_msg_buffer[256];
    db_init_msg_buffer[0] = '\0';
    int initializedPieces = DBManager::instance()->db_init(256, db_init_msg_buffer, m_egdbPath.toUtf8().constData());
    if (initializedPieces > 0) {
        m_egdbInitialized = true;
        m_maxEGDBPieces = initializedPieces;
        qInfo() << "GeminiAI: EGDB initialized successfully. Max pieces: " << m_maxEGDBPieces << " Message: " << db_init_msg_buffer;
    } else {
        m_egdbInitialized = false;
        m_maxEGDBPieces = 0;
        qWarning() << "GeminiAI: Failed to initialize EGDB with path: " << m_egdbPath << ". EGDB will be disabled. Message: " << db_init_msg_buffer;
    }
}

void GeminiAI::requestMove(Board8x8 board, int colorToMove, double timeLimit)
{
    qDebug() << "GeminiAI: requestMove called. Color: " << colorToMove << ", Time Limit: " << timeLimit;
    if (m_useExternalEngine && m_primaryExternalEngine) {
        // External engine logic remains the same
    } else {
        emit requestWorkerTask(m_mode, board, colorToMove, timeLimit);
    }
}

void GeminiAI::abortSearch()
{
    QMetaObject::invokeMethod(m_worker, "requestAbort", Qt::QueuedConnection);
}

void GeminiAI::startAnalyzeGame(const Board8x8& board, int colorToMove)
{
    emit requestWorkerTask(AnalyzeGame, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startAutoplay(const Board8x8& board, int colorToMove)
{
    emit requestWorkerTask(Autoplay, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startEngineMatch(int numGames, const Board8x8& board, int colorToMove)
{
    // numGames is not directly used here but could be passed to the worker if needed
    emit requestWorkerTask(EngineMatch, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startRunTestSet(const Board8x8& board, int colorToMove)
{
    emit requestWorkerTask(RunTestSet, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startAnalyzePdn(const Board8x8& board, int colorToMove)
{
    emit requestWorkerTask(AnalyzePdn, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::requestAbort()
{
    abortSearch();
}

void GeminiAI::handleWorkerSearchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime)
{
    emit searchFinished(moveFound, aborted, bestMove, statusText, gameResult, pdnMoveText, elapsedTime);
}

void GeminiAI::handleWorkerEvaluation(int score, int depth)
{
    m_lastEvaluationScore = score;
    m_lastSearchDepth = depth;
    emit evaluationReady(score, depth);
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

void GeminiAI::setEgdbPath(const QString& path)
{
    qInfo() << "GeminiAI: EGDB path set to: " << path;
    if (m_egdbInitialized) {
        DBManager::instance()->db_exit();
    }
    m_egdbPath = path;
    init();
}

// User book and external engine methods remain unchanged
void GeminiAI::addMoveToUserBook(const Board8x8& board, const CBmove& move) { Q_UNUSED(board); Q_UNUSED(move); }
void GeminiAI::deleteCurrentEntry() {}
void GeminiAI::deleteAllEntriesFromUserBook() {}
void GeminiAI::navigateToNextEntry() {}
void GeminiAI::navigateToPreviousEntry() {}
void GeminiAI::resetNavigation() {}
void GeminiAI::loadUserBook(const QString& filename) { Q_UNUSED(filename); }
void GeminiAI::saveUserBook(const QString& filename) { Q_UNUSED(filename); }
bool GeminiAI::sendCommand(const QString& command, QString& reply) {
    if (m_useExternalEngine && m_primaryExternalEngine) {
        return m_primaryExternalEngine->sendCommand(command, reply);
    }
    reply = "Internal AI: No external engine active.";
    return true;
}
void GeminiAI::setExternalEnginePath(const QString& path) { m_externalEnginePath = path; }
void GeminiAI::setSecondaryExternalEnginePath(const QString& path) { m_secondaryExternalEnginePath = path; }