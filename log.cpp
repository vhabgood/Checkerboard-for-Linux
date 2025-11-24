#include "log.h"
#include <QDateTime>
#include <QDebug>
#include <cstdio>

LogLevel s_minLogLevel = LOG_LEVEL_DEBUG;

void init_logging()
{
}

void close_logging()
{
}


void log_c(int level, const char* message)
{
    if (level < s_minLogLevel)
        return;

    QString levelStr;
    switch (level) {
    case LOG_LEVEL_TRACE: levelStr = "TRACE"; break;
    case LOG_LEVEL_DEBUG: levelStr = "DEBUG"; break;
    case LOG_LEVEL_INFO: levelStr = "INFO"; break;
    case LOG_LEVEL_WARNING: levelStr = "WARNING"; break;
    case LOG_LEVEL_ERROR: levelStr = "ERROR"; break;
    case LOG_LEVEL_FATAL: levelStr = "FATAL"; break;
    default: levelStr = "UNKNOWN"; break;
    }

    QString formattedMessage = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                               + "  [" + levelStr + "]  " + QString::fromUtf8(message);

    printf("%s\n", formattedMessage.toLocal8Bit().constData());
    fflush(stdout);
}


void log_qstring(int level, const QString& message)
{
    log_c(level, message.toUtf8().constData());
}