#ifndef LOG_H
#define LOG_H

#include <QString>

#include "checkers_types.h"

enum LogLevel {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

void init_logging();
void close_logging();
extern "C" {
void log_c(int level, const char* message);
void log_qstring(int level, const QString& message);
}

extern LogLevel s_minLogLevel;

#endif // LOG_H
