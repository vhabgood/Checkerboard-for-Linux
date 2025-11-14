#include "engine_wrapper.h"
#include "GameManager.h" // For logging
#include <QDebug>
#include <QEventLoop>

ExternalEngine::ExternalEngine(const QString& enginePath, QObject *parent)
    : QObject(parent),
    m_enginePath(enginePath),
    m_process(new QProcess(this)),
    m_responseTimer(new QTimer(this)),
    m_currentReply(nullptr),
    m_eventLoop(nullptr),
    m_isReady(false),
    m_expectingBestMove(false)
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ExternalEngine::readStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &ExternalEngine::readStandardError);
    connect(m_process, &QProcess::started, this, &ExternalEngine::processStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExternalEngine::processFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ExternalEngine::processErrorOccurred);

    m_responseTimer->setSingleShot(true);
    connect(m_responseTimer, &QTimer::timeout, this, [this]() {
        GameManager::log(LogLevel::Warning, QString("ExternalEngine: Command timed out: %1").arg(m_lastCommand));
        if (m_eventLoop && m_eventLoop->isRunning()) {
            m_eventLoop->exit(-1); // Exit with an error code
        }
    });
}

ExternalEngine::~ExternalEngine()
{
    stopEngine();
}

bool ExternalEngine::startEngine()
{
    if (m_enginePath.isEmpty()) {
        GameManager::log(LogLevel::Error, "ExternalEngine: Engine path is empty. Cannot start engine.");
        return false;
    }

    if (m_process->state() == QProcess::NotRunning) {
        m_process->start(m_enginePath, QStringList()); // Use the recommended overload
        if (!m_process->waitForStarted(5000)) { // Wait up to 5 seconds for the engine to start
            GameManager::log(LogLevel::Error, QString("ExternalEngine: Failed to start engine: %1").arg(m_process->errorString()));
            return false;
        }
        GameManager::log(LogLevel::Info, QString("ExternalEngine: Engine started: %1").arg(m_enginePath));
        return true;
    }
    return false;
}

void ExternalEngine::stopEngine()
{
    if (m_process->state() == QProcess::Running) {
        m_process->write("quit\n");
        m_process->waitForBytesWritten(1000);
        m_process->close(); // Close the process
        m_process->waitForFinished(2000); // Wait for it to finish
        GameManager::log(LogLevel::Info, "ExternalEngine: Engine stopped.");
    }
}

bool ExternalEngine::sendCommand(const QString& command, QString& reply)
{
    if (m_process->state() != QProcess::Running) {
        GameManager::log(LogLevel::Error, "ExternalEngine: Cannot send command, engine is not running.");
        reply = "Error: Engine not running.";
        return false;
    }

    m_outputBuffer.clear();
    m_lastCommand = command;
    m_currentReply = &reply; // Set the reply buffer for this command
    m_isReady = false; // Assume not ready until confirmed

    GameManager::log(LogLevel::Debug, QString("ExternalEngine: Sending command: %1").arg(command));
    m_process->write((command + "\n").toUtf8());
    m_process->waitForBytesWritten(1000);

    // Use an event loop to wait for a response
    m_eventLoop = new QEventLoop(this);
    m_responseTimer->start(5000); // 5 second timeout for response
    int result = m_eventLoop->exec(); // Blocks until m_eventLoop->exit() is called
    m_responseTimer->stop();
    delete m_eventLoop;
    m_eventLoop = nullptr;

    if (result == -1) { // Timeout occurred
        reply = "Error: Command timed out.";
        return false;
    }

    // Concatenate buffered output into reply
    reply = m_outputBuffer.join("\n");
    return true;
}

void ExternalEngine::setEnginePath(const QString& path)
{
    m_enginePath = path;
}

void ExternalEngine::readStandardOutput()
{
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        GameManager::log(LogLevel::Debug, QString("ExternalEngine StdOut: %1").arg(line));
        emit engineOutput(line);

        // Buffer output for synchronous reply
        if (m_currentReply) {
            m_outputBuffer.append(line);
        }

        // Basic parsing for common engine responses
        if (line == "readyok") {
            m_isReady = true;
            emit engineReady();
            if (m_eventLoop && m_eventLoop->isRunning()) {
                m_eventLoop->exit(0);
            }
        } else if (line.startsWith("bestmove")) {
            // Parse bestmove line (e.g., "bestmove e2e4 ponder e7e5")
            // For now, just log and emit a dummy move
            QStringList parts = line.split(" ");
            if (parts.size() >= 2) {
                // TODO: Parse actual move from parts[1]
                CBmove dummyMove = {0};
                emit bestMoveFound(dummyMove);
            }
            m_expectingBestMove = false;
            if (m_eventLoop && m_eventLoop->isRunning()) {
                m_eventLoop->exit(0);
            }
        } else if (line.startsWith("info")) {
            // Parse info line for evaluation and depth
            // Example: "info depth 5 score cp 200 nodes 1000 pv e2e4 e7e5"
            int depth = 0;
            int score = 0;
            QRegExp depthRx("depth (\\d+)");
            QRegExp scoreRx("score cp (-?\\d+)"); // Centipawn score

            if (depthRx.indexIn(line) != -1) {
                depth = depthRx.cap(1).toInt();
            }
            if (scoreRx.indexIn(line) != -1) {
                score = scoreRx.cap(1).toInt();
            }
            if (depth > 0 || score != 0) { // Only emit if we found some meaningful info
                emit evaluationUpdate(score, depth);
            }
        }

        // If we are waiting for a reply and the line indicates the end of a response
        // (e.g., "readyok", "bestmove", or a blank line after a command that doesn't expect specific output)
        // we can exit the event loop. This part might need refinement based on actual engine protocols.
        if (m_currentReply && (line.isEmpty() || line == "readyok" || line.startsWith("bestmove"))) {
            if (m_eventLoop && m_eventLoop->isRunning()) {
                m_eventLoop->exit(0);
            }
        }
    }
}

void ExternalEngine::readStandardError()
{
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        GameManager::log(LogLevel::Error, QString("ExternalEngine StdErr: %1").arg(line));
        emit engineError(line);
    }
}

void ExternalEngine::processStarted()
{
    GameManager::log(LogLevel::Info, "ExternalEngine: Process started.");
    // Send initial commands to set up the engine (e.g., "uci" or "xboard")
    // For now, let's assume a simple "uci" protocol if it's a UCI-like engine.
    // This might need to be configurable.
    m_process->write("uci\n");
    m_process->waitForBytesWritten(1000);
    m_process->write("isready\n");
    m_process->waitForBytesWritten(1000);
}

void ExternalEngine::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    GameManager::log(LogLevel::Info, QString("ExternalEngine: Process finished with exit code %1, status %2.").arg(exitCode).arg(exitStatus));
    m_isReady = false;
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->exit(-1); // Indicate an error if process finishes unexpectedly
    }
}

void ExternalEngine::processErrorOccurred(QProcess::ProcessError error)
{
    GameManager::log(LogLevel::Critical, QString("ExternalEngine: Process error occurred: %1").arg(error));
    m_isReady = false;
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->exit(-1); // Indicate an error
    }
}