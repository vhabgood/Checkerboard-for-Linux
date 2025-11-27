#include <QApplication>
#include <QCoreApplication> // Added for QCoreApplication
#include "MainWindow.h"
#include <QMessageBox>
#include <QSharedMemory>
#include "checkers_types.h" // Include the new Qt-specific types header

#include <QDebug>
#include <QTimer>
#include "log.h"
#include <cstdio>

// Global program status word definition
uint32_t g_programStatusWord = 0;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    log_qstring(type, msg);
}


#include <cstdio>

int main(int argc, char *argv[])
{
    g_programStatusWord |= STATUS_APP_START;

    // Set application name for QStandardPaths
    QCoreApplication::setApplicationName("CheckerBoard");
    // init_logging();
    // qInstallMessageHandler(messageHandler);

    QApplication a(argc, argv);

    qRegisterMetaType<Board8x8>();

    qRegisterMetaType<CBmove>();

    qDebug() << "Application started.";

    QSharedMemory sharedMemory;

    sharedMemory.setKey("CheckerBoardAppKey");

    if (sharedMemory.attach()) {
        QMessageBox::warning(nullptr, "Application Already Running", "Another instance of CheckerBoard is already running.");
        return 1;
    }

    if (!sharedMemory.create(1)) {
        // This case should ideally not be reached if attach() failed, but for robustness
        QMessageBox::critical(nullptr, "Error", "Could not create shared memory segment.");
        g_programStatusWord |= STATUS_CRITICAL_ERROR; // Set status flag for critical error
        return 1;
    }

    Q_INIT_RESOURCE(resources);

    GameManager gameManager;

    MainWindow w(&gameManager);

    w.show();

    int result = a.exec();

    // if (sharedMemory.isAttached()) {
    //     sharedMemory.detach();
    // }

    qDebug() << "Application finished.";

    fprintf(stdout, "Program Status Word: 0x%08X\n", g_programStatusWord);
    fprintf(stderr, "Program Status Word: 0x%08X\n", g_programStatusWord);

    // close_logging();
    return result;
}


