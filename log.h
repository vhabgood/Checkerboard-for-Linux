#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
#include <QString>
#include <QtGlobal>
extern "C" {
#endif

#define LOG_LEVEL_FATAL   0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

void log_c(int level, const char* format, ...);

#ifdef __cplusplus
}
void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#endif

#endif // LOG_H
