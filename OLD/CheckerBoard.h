#pragma once

#include "checkers_types.h" // Includes CBconsts.h implicitly
#include <vector>
#include <string>

// --- Configuration ---
#define PLACE "Qt Port" // Indicate this is the Qt version
#define SLEEPTIME 50    // Milliseconds for short sleeps
#define AUTOSLEEPTIME 10 // Milliseconds for AutoThread loop sleep


// --- Enums ---
// (Already defined in checkers_types.h or moved to MainWindow.h)
// enum state { ... };

// Callback for status updates
typedef void (*StatusCallback)(const char* text);
extern "C" void set_status_callback(StatusCallback callback);
extern "C" void update_status_bar_c(const char* format, ...);


#include <QObject>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QTextEdit>
#include <QStatusBar>
#include <QCloseEvent>
#include "cb_interface.h"

#include "checkers_types.h"

extern "C" {
    // C-style function declarations
    void forward_to_game_end_c(PdnGameWrapper* game, Board8x8 board, int* color);
    void move4tonotation(const CBmove &m, char s[80]);
    void InitCheckerBoard(Board8x8 board);
    void ClearCheckerBoard(Board8x8 b);
}

class CheckerBoard : public QMainWindow
{
    Q_OBJECT

public:
    CheckerBoard(QWidget *parent = 0);
    ~CheckerBoard();
    void addmovetogame_c(PDNgame* game, Board8x8 board, int color, const CBmove &move, char *pdn);
    void forward_to_game_end_c(PDNgame* game, Board8x8 board, int* color);
    int start3move_c(PDNgame* game, Board8x8 board, int* color, int opening_index, int gametype);

    // Engine Loading (accepts parameters)
    void loadengines(const char* primaryEngineFile, const char* secondaryEngineFile);
    

    

    // Board / FEN / PDN Helpers (state independent or accept params)
    void InitCheckerBoard(Board8x8 board);
    void ClearCheckerBoard(Board8x8 b);
    bool move_to_pdn_english(Board8x8 board8, int color, CBmove *move, char *pdn, int gametype);
    bool islegal_check(Board8x8 board8, int color, Squarelist &squares, CBmove *move, int gametype);


    void move4tonotation(const CBmove &m, char s[80]);
    void board8toFEN(const Board8x8 board, char *fenstr, int color, int gametype);
    int FENtoboard8(Board8x8 board, const char *buf, int *poscolor, int gametype);
    bool doload(PDNgame *game, const char *gamestring, int *color, Board8x8 board8, std::string &errormsg);

    // Other Helpers
    int coorstonumber(int x, int y, int gametype);
    void numbertocoors(int n, int *x, int *y, int gametype);
    // void coorstocoors(int *x, int *y, bool invert, bool mirror); // Declared in checkers_types.h
    // bool is_valid_board8_square(int x, int y); // Declared in checkers_types.h
    char *pdn_result_to_string(PDN_RESULT result, int gametype);

    

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newGame();
    void about();
    void options();
    void engineSelect();
    void engineOptions();
    void engineCommand();
    void gameDatabase();
    void loadGame();
    void saveGame();
    void gameInfo();
    void findPosition();
    void findCR();
    void commentMove();
    void pieceSet();
    void help();
    void homepage();
    void problemOfTheDay();
    void onlineUpgrade();
    void checkersInANutshell();
    void exitApplication();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();

    QMenu *fileMenu;
    QMenu *gameMenu;
    QMenu *movesMenu;
    QMenu *optionsMenu;
    QMenu *engineMenu;
    QMenu *setupMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *gameToolBar;
    QToolBar *movesToolBar;

    QAction *newGameAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *optionsAct;
    QAction *engineSelectAct;
    QAction *engineOptionsAct;
    QAction *engineCommandAct;
    QAction *gameDatabaseAct;
    QAction *loadGameAct;
    QAction *saveGameAct;
    QAction *gameInfoAct;
    QAction *findPositionAct;
    QAction *findCRAct;
    QAction *commentMoveAct;
    QAction *pieceSetAct;
    QAction *helpAct;
    QAction *homepageAct;
    QAction *problemOfTheDayAct;
    QAction *onlineUpgradeAct;
    QAction *checkersInANutshellAct;

    // Other UI elements
    QTextEdit *textEdit;
    QStatusBar *statusBar;

    // Game logic and engine
    // CheckerBoardWidget *checkerBoardWidget;
    // Engine *engine;
};
    



