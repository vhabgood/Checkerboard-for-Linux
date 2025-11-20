#ifndef LOG_H
#define LOG_H

#include <QString>

#include "checkers_c_types.h"

void init_logging();
void close_logging();
void log_c(int level, const char* message);
void log_qstring(int level, const QString& message);

extern LogLevel s_minLogLevel;

#endif // LOG_H
