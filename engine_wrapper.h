#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QEventLoop> // Added for QEventLoop

#include "checkers_types.h"

class ExternalEngine : public QObject
{
    Q_OBJECT

public:
    explicit ExternalEngine(const QString& enginePath, QObject *parent = nullptr);
    ~ExternalEngine();

    bool startEngine();
    void stopEngine();
    bool sendCommand(const QString& command, QString& reply);
    void setEnginePath(const QString& path);
    QString getEnginePath() const { return m_enginePath; }

signals:
    void engineOutput(const QString& output);
    void engineError(const QString& error);
    void engineReady();
    void bestMoveFound(const CBmove& move);
    void evaluationUpdate(int score, int depth);

private slots:
    void readStandardOutput();
    void readStandardError();
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorOccurred(QProcess::ProcessError error);

private:
    QProcess *m_process;
    QString m_enginePath;
    QStringList m_outputBuffer; // Buffer for engine output
    QTimer *m_responseTimer; // Timer to wait for engine responses
    QString m_lastCommand; // Store the last command sent
    QString *m_currentReply; // Pointer to the reply string for the current command
    QEventLoop *m_eventLoop; // Event loop for synchronous command sending
    bool m_isReady; // Flag to indicate if the engine is ready
    bool m_expectingBestMove; // Flag to indicate if we are expecting a best move
};