#include <QApplication>
#include "MainWindow.h"
#include "GameManager.h" // Include GameManager
#include "Logger.h" // Include the new Logger
#include "core_types.h" // Include for bitboard_pos

int main(int argc, char *argv[])
{
    // Start the logger thread. This should be the first thing.
    Logger::instance()->start();

    QApplication a(argc, argv);

    // Register custom types for signal/slot connections across threads
    qRegisterMetaType<AppState>("AppState");
    qRegisterMetaType<AI_State>("AI_State");
    qRegisterMetaType<CBmove>("CBmove");
    qRegisterMetaType<bitboard_pos>("bitboard_pos");

    GameManager gameManager;
    MainWindow w(&gameManager); // Pass GameManager to MainWindow constructor
    w.show();

    int result = a.exec();

    // Stop the logger thread cleanly
    Logger::instance()->stop();
    Logger::instance()->wait(); // Wait for the thread to finish writing messages

    return result;
}

