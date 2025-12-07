#include <QApplication>
#include "MainWindow.h"
#include "GameManager.h" // Include GameManager
#include "Logger.h" // Include the new Logger
#include "core_types.h" // Include for bitboard_pos
#include "log.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Start the logger thread.
    Logger::instance()->start();
    
    // Redirect all Qt debug/info/warning messages to our log.txt
    qInstallMessageHandler(qtMessageHandler);

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
    Logger::cleanup();

    return result;
}

