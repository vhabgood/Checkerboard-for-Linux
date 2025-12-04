#include "GeminiAI.h"
#include "DBManager.h"
#include "core_types.h"
#include <QDebug>
#include <QCoreApplication>
#include "log.h"

GeminiAI::GeminiAI(const QString& egdbPath, QObject *parent)
    : QObject(parent),
      m_aiThread(new QThread(this)),
      m_worker(new AIWorker()),
      m_egdbPath(egdbPath),
      m_egdbInitialized(false),
      m_maxEGDBPieces(0),
      m_mode(Autoplay),
      m_handicap(0),
      m_primaryExternalEngine(nullptr),
      m_secondaryExternalEngine(nullptr),
      m_useExternalEngine(false),
      m_lastEvaluationScore(0),
      m_lastSearchDepth(0),
      m_pendingMoveRequest(false)
{
    m_worker->moveToThread(m_aiThread);
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

}

void GeminiAI::init()
{
    qDebug() << "GeminiAI: Initializing EGDB with path: " << m_egdbPath;
    updateProgramStatusWord(STATUS_EGDB_INIT_START);

    char db_init_output[256];
    int suggestedMB = 64; // Default value, can be made configurable later if needed

    int success = DBManager::instance()->db_init(suggestedMB, db_init_output, m_egdbPath.toUtf8().constData());

    if (success) {
        m_egdbInitialized = true;
        updateProgramStatusWord(STATUS_EGDB_INIT_OK);
        // Assuming DBManager::db_init populates its internal m_maxpieces variable
        // which can then be retrieved or is available through other means.
        // For now, m_maxEGDBPieces will remain at its default of 0 unless updated by a separate mechanism.
        // This should be verified by checking DBManager.cpp.
        qDebug() << "GeminiAI: EGDB initialized successfully. Output: " << db_init_output;
    } else {
        m_egdbInitialized = false;
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL);
        qWarning() << "GeminiAI: EGDB initialization failed. Output: " << db_init_output;
    }

    // The logic to process pending move requests should not re-call requestMove here,
    // as requestMove now directly emits requestWorkerTask without deferring based on EGDB status.
    // The m_pendingMoveRequest flag is effectively no longer needed in this context.
    // However, if the intent was to ensure a move request is re-triggered if it was deferred *before*
    // EGDB init completed, this block is where that logic would live if deferral was still active.
    // For now, we'll keep it simple as requestMove now directly emits.
    // m_pendingMoveRequest = false; // Reset pending request if it was used for deferred calls
}



void GeminiAI::requestMove(bitboard_pos board, int colorToMove, double timeLimit)
{
    emit requestWorkerTask(m_mode, board, colorToMove, timeLimit);
}

void GeminiAI::abortSearch()
{
    QMetaObject::invokeMethod(m_worker, "requestAbort", Qt::QueuedConnection);
}

void GeminiAI::startAnalyzeGame(const bitboard_pos& board, int colorToMove)
{
    emit requestWorkerTask(AnalyzeGame, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startAutoplay(const bitboard_pos& board, int colorToMove)
{
    emit requestWorkerTask(Autoplay, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startEngineMatch(int numGames, const bitboard_pos& board, int colorToMove)
{
    // numGames is not directly used here but could be passed to the worker if needed
    emit requestWorkerTask(EngineMatch, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startRunTestSet(const bitboard_pos& board, int colorToMove)
{
    emit requestWorkerTask(RunTestSet, board, colorToMove, m_options.time_per_move);
}

void GeminiAI::startAnalyzePdn(const bitboard_pos& board, int colorToMove)
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
void GeminiAI::addMoveToUserBook(const bitboard_pos& board, const CBmove& move) { Q_UNUSED(board); Q_UNUSED(move); }
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