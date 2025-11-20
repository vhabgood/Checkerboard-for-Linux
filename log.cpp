#include "log.h"
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QStandardPaths> // Added for QStandardPaths
#include <QDir>         // Added for QDir
#include <QDebug>       // Added for qWarning()

static QFile m_logFile;
static QTextStream m_logStream;
static QMutex m_logMutex;
LogLevel s_minLogLevel = LOG_LEVEL_DEBUG;

void init_logging()
{
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_logFile.setFileName(appDataLocation + "/app.txt");
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        // Fallback to current directory if appDataLocation is not writable
        m_logFile.setFileName("app.txt");
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qWarning() << "Failed to open log file in appDataLocation or current directory. Logging will be disabled.";
            return;
        }
    }
    m_logStream.setDevice(&m_logFile);
}

void close_logging()
{
    m_logFile.close();
}

void log_c(int level, const char* message)
{
    if (level < s_minLogLevel)
        return;

    QMutexLocker locker(&m_logMutex);
    QString levelStr;
    switch (level) {
    case LOG_LEVEL_DEBUG:
        levelStr = "DEBUG";
        break;
    case LOG_LEVEL_INFO:
        levelStr = "INFO";
        break;
    case LOG_LEVEL_WARNING:
        levelStr = "WARNING";
        break;
    case LOG_LEVEL_ERROR:
        levelStr = "ERROR";
        break;
    case LOG_LEVEL_FATAL:
        levelStr = "FATAL";
        break;
    }
    m_logStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                << " [" << levelStr << "] " << message << Qt::endl;
}

void log_qstring(int level, const QString& message)
{
    log_c(level, message.toUtf8().constData());
}
