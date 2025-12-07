#include "log.h"
#include "Logger.h"
#include <cstdarg>
#include <cstdio>
#include <QDateTime>
#include <QString>

static int s_minLogLevel = LOG_LEVEL_INFO;

void log_c(int level, const char* format, ...)
{
    if (level > s_minLogLevel) { // Invert the logic here
        return;
    }

    // Format the message using vsnprintf
    char buffer[2048]; // Increased buffer size for safety
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Determine the log level string
    const char* levelStr;
    switch (level) {
        case LOG_LEVEL_FATAL:   levelStr = "FATAL";   break;
        case LOG_LEVEL_ERROR:   levelStr = "ERROR";   break;
        case LOG_LEVEL_WARNING: levelStr = "WARNING"; break;
        case LOG_LEVEL_INFO:    levelStr = "INFO";    break;
        case LOG_LEVEL_DEBUG:   levelStr = "DEBUG";   break;
        default:                levelStr = "UNKNOWN"; break;
    }

    // Get current timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // Construct the final log message
    QString finalMessage = QString("%1  [%2]  %3").arg(timestamp).arg(levelStr).arg(buffer);

    // Send the message to the thread-safe logger
    Logger::instance()->log(finalMessage);
}
