#include <QApplication>
#include "MainWindow.h"
#include "GameManager.h" // Include GameManager for logging
#include <QMessageBox>
#include <QSharedMemory>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include "checkers_types.h" // Include the new Qt-specific types header
#include <stdio.h>

// Custom message handler
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    LogLevel level;
    QString logMessage;
    switch (type) {
    case QtDebugMsg:
        logMessage = QString("Debug: %1").arg(msg);
        level = LogLevel::Debug;
        break;
    case QtInfoMsg:
        logMessage = QString("Info: %1").arg(msg);
        level = LogLevel::Info;
        break;
    case QtWarningMsg:
        logMessage = QString("Warning: %1").arg(msg);
        level = LogLevel::Warning;
        break;
    case QtCriticalMsg:
        logMessage = QString("Critical: %1").arg(msg);
        level = LogLevel::Critical;
        break;
    case QtFatalMsg:
        logMessage = QString("Fatal: %1").arg(msg);
        level = LogLevel::Critical; // Fatal messages are also critical
        break;
    }

    // Include context information if available
    if (context.file) {
        logMessage += QString(" in %1, line %2").arg(context.file).arg(context.line);
    }

    GameManager::log(level, logMessage);
}


int main(int argc, char *argv[])
{
    // For debugging: try to remove any leftover shared memory segment from previous crashes
    QSharedMemory cleanupSharedMemory;
    cleanupSharedMemory.setKey("CheckerBoardAppKey");
    if (cleanupSharedMemory.attach()) {
        cleanupSharedMemory.detach();
    }
    cleanupSharedMemory.create(1); // Re-create to ensure it's clean for this run
    cleanupSharedMemory.detach(); // Detach immediately, the main sharedMemory will handle it

    GameManager::initLogging(); // Initialize logging

    qInstallMessageHandler(myMessageHandler); // Install the message handler

    QApplication a(argc, argv);

    qRegisterMetaType<Board8x8>();

    qRegisterMetaType<CBmove>();

    GameManager::log(LogLevel::Info, "Application started.");

    QSharedMemory sharedMemory;

    sharedMemory.setKey("CheckerBoardAppKey");

    if (sharedMemory.attach()) {
        QMessageBox::warning(nullptr, "Application Already Running", "Another instance of CheckerBoard is already running.");
        return 1;
    }

    if (!sharedMemory.create(1)) {
        // This case should ideally not be reached if attach() failed, but for robustness
        QMessageBox::critical(nullptr, "Error", "Could not create shared memory segment.");
        return 1;
    }

    Q_INIT_RESOURCE(resources);

    GameManager gameManager;

    MainWindow w(&gameManager);

    w.show();

    int result = a.exec();

    if (sharedMemory.isAttached()) {
        sharedMemory.detach();
    }

    GameManager::log(LogLevel::Info, "Application finished."); // Log application shutdown
    GameManager::closeLogging(); // Close logging

    return result;
}
