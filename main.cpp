#include <QApplication>
#include <QCoreApplication> // Added for QCoreApplication
#include "MainWindow.h"
#include <QMessageBox>
#include <QSharedMemory>
#include "checkers_types.h" // Include the new Qt-specific types header
#include <QMetaType>
#include <QDebug>
#include <QTimer>
#include "log.h"
#include <cstdio>
#include "DBManager.h"
#include "core_types.h" // Include core_types.h for global extern declarations

// Extern declarations for global variables
extern uint32_t g_programStatusWord;
extern int g_num_egdb_pieces;

// Global program status word definition


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    log_qstring(type, msg);
}


#include <cstdio>

int main(int argc, char *argv[])
{
    qRegisterMetaType<AI_State>("AI_State");

    updateProgramStatusWord(STATUS_APP_START);

    QApplication a(argc, argv);

    // Set application name for QStandardPaths
    QCoreApplication::setApplicationName("CheckerBoard");

    qRegisterMetaType<Board8x8>();

    qRegisterMetaType<CBmove>();

    qRegisterMetaType<CBoptions>("CBoptions");
    qRegisterMetaType<AppState>("AppState");

    qDebug() << "Application started.";

    QSharedMemory sharedMemory;

    sharedMemory.setKey("CheckerBoardAppKey");

    if (sharedMemory.attach()) {
        // Detach from the stale segment to allow the next run to succeed.
        sharedMemory.detach();
        QMessageBox::warning(nullptr, "Application Already Running", "Another instance of CheckerBoard is already running.");
        return 1;
    }

    if (!sharedMemory.create(1)) {
        // This case can be reached if another instance was created between the attach() call and this create() call.
        sharedMemory.detach();
        QMessageBox::critical(nullptr, "Error", "Could not create shared memory segment.");
        updateProgramStatusWord(STATUS_CRITICAL_ERROR); // Set status flag for critical error
        return 1;
    }

    Q_INIT_RESOURCE(resources);

    GameManager gameManager;

    MainWindow *w = new MainWindow(&gameManager);
    w->setAttribute(Qt::WA_DeleteOnClose); // Ensure the window is deleted when closed

    w->show();

    int result = a.exec();

    if (sharedMemory.isAttached()) {
        sharedMemory.detach();
    }

    qDebug() << "Application finished.";

    fprintf(stdout, "Program Status Word: 0x%08X\n", getProgramStatusWord());
    fprintf(stderr, "Program Status Word: 0x%08X\n", getProgramStatusWord());

    delete DBManager::instance();
    // close_logging();
    return result;
}


