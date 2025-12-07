#ifndef LOG_H
#define LOG_H

// Log levels
#define LOG_LEVEL_FATAL   0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// The primary logging function, now thread-safe.
void log_c(int level, const char* format, ...);

#endif // LOG_H