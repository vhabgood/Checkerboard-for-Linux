#include "Logger.h"
#include <QDateTime>

Logger* Logger::m_instance = nullptr;

Logger* Logger::instance()
{
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

Logger::Logger(QObject *parent) : QThread(parent)
{
    m_logFile.setFileName("log.txt");
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        m_logStream << "--- Log Started: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n";
        m_logStream.flush();
    }
}

Logger::~Logger()
{
    stop();
    wait();
    if (m_logFile.isOpen()) {
        m_logStream << "--- Log Ended: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n";
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::stop()
{
    QMutexLocker locker(&m_mutex);
    m_quit = true;
    m_condition.wakeOne();
}

void Logger::log(const QString& message)
{
    QMutexLocker locker(&m_mutex);
    m_messageQueue.enqueue(message);
    m_condition.wakeOne();
}

void Logger::run()
{
    forever {
        QMutexLocker locker(&m_mutex);
        if (m_quit && m_messageQueue.isEmpty()) {
            break;
        }

        if (m_messageQueue.isEmpty()) {
            m_condition.wait(&m_mutex);
        }

        if (!m_messageQueue.isEmpty()) {
            QString message = m_messageQueue.dequeue();
            if (m_logFile.isOpen()) {
                m_logStream << message << "\n";
                m_logStream.flush();
            }
        }
    }
}
