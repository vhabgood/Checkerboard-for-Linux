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

    printf("main: Before GameManager::initLogging()\n");
    GameManager::initLogging(); // Initialize logging
    printf("main: After GameManager::initLogging()\n");

    printf("main: Before qInstallMessageHandler()\n");
    qInstallMessageHandler(myMessageHandler); // Install the message handler
    printf("main: After qInstallMessageHandler()\n");

    printf("main: Before QApplication a(argc, argv)\n");
    QApplication a(argc, argv);
    printf("main: After QApplication a(argc, argv)\n");

    printf("main: Before qRegisterMetaType<Board8x8>()\n");
    qRegisterMetaType<Board8x8>();
    printf("main: After qRegisterMetaType<Board8x8>()\n");

    printf("main: Before qRegisterMetaType<CBmove>()\n");
    qRegisterMetaType<CBmove>();
    printf("main: After qRegisterMetaType<CBmove>()\n");

    GameManager::log(LogLevel::Info, "Application started.");

    printf("main: Before QSharedMemory sharedMemory\n");
    QSharedMemory sharedMemory;
    printf("main: After QSharedMemory sharedMemory\n");

    printf("main: Before sharedMemory.setKey()\n");
    sharedMemory.setKey("CheckerBoardAppKey");
    printf("main: After sharedMemory.setKey()\n");

    printf("main: Before sharedMemory.attach()\n");
    if (sharedMemory.attach()) {
        printf("main: sharedMemory.attach() returned true. Another instance running.\n");
        QMessageBox::warning(nullptr, "Application Already Running", "Another instance of CheckerBoard is already running.");
        return 1;
    }
    printf("main: After sharedMemory.attach()\n");

    printf("main: Before sharedMemory.create(1)\n");
    if (!sharedMemory.create(1)) {
        printf("main: sharedMemory.create(1) failed.\n");
        // This case should ideally not be reached if attach() failed, but for robustness
        QMessageBox::critical(nullptr, "Error", "Could not create shared memory segment.");
        return 1;
    }
    printf("main: After sharedMemory.create(1)\n");

    printf("main: Before Q_INIT_RESOURCE(resources)\n");
    Q_INIT_RESOURCE(resources);
    printf("main: After Q_INIT_RESOURCE(resources)\n");

    printf("main: Before GameManager gameManager\n");
    GameManager gameManager;
    printf("main: After GameManager gameManager\n");

    printf("main: Before MainWindow w(&gameManager)\n");
    MainWindow w(&gameManager);
    printf("main: After MainWindow w(&gameManager)\n");

    printf("main: Before w.show()\n");
    w.show();
    printf("main: After w.show()\n");

    printf("main: Before a.exec()\n");
    int result = a.exec();
    printf("main: After a.exec()\n");

    printf("main: Before sharedMemory.isAttached()\n");
    if (sharedMemory.isAttached()) {
        printf("main: sharedMemory is attached. Detaching.\n");
        sharedMemory.detach();
    }
    printf("main: After sharedMemory.isAttached()\n");

    GameManager::log(LogLevel::Info, "Application finished."); // Log application shutdown
    printf("main: Before GameManager::closeLogging()\n");
    GameManager::closeLogging(); // Close logging
    printf("main: After GameManager::closeLogging()\n");

    return result;
}
