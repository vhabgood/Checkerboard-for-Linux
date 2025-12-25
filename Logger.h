#ifndef LOGGER_H
#define LOGGER_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QString>
#include <QWaitCondition>
#include <QFile>
#include <QTextStream>

class Logger : public QThread
{
    Q_OBJECT

public:
    static Logger* instance();
    static void cleanup();
    void log(const QString &message);
    void stop();

protected:
    void run() override;

private:
    Logger(QObject *parent = nullptr);
    ~Logger();

    static Logger* m_instance;
    QQueue<QString> m_messageQueue;
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_quit = false;

    QFile m_logFile;
    QTextStream m_logStream;
};

#endif // LOGGER_H
