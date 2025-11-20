#include <QApplication>
#include "MainWindow.h"
#include <QMessageBox>
#include <QSharedMemory>
#include "checkers_types.h" // Include the new Qt-specific types header

#include <QDebug>
#include <QTimer>
#include "log.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    log_qstring(type, msg);
}


int main(int argc, char *argv[])
{
    init_logging();
    qInstallMessageHandler(messageHandler);

    // For debugging: try to remove any leftover shared memory segment from previous crashes
    QSharedMemory cleanupSharedMemory;
    cleanupSharedMemory.setKey("CheckerBoardAppKey");
    if (cleanupSharedMemory.attach()) {
        cleanupSharedMemory.detach();
    }
    cleanupSharedMemory.create(1); // Re-create to ensure it's clean for this run
    cleanupSharedMemory.detach(); // Detach immediately, the main sharedMemory will handle it

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

    qDebug() << "Application finished.";

    close_logging();
    return result;
}


