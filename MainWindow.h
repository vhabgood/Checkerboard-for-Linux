#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "CheckerBoardWidget.h"
#include "CheckerBoard.h" // Include for newgame() function

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File Menu Actions (already existing)
    void newGame();
    void exitApplication();

    // Game Menu Actions
    void game3Move();
    void gameLoad();
    void gameSave();
    void gameInfo();
    void gameAnalyze();
    void gameCopy();
    void gamePaste();
    void gameDatabase();
    void gameFind();
    void gameFindCR();
    void gameFindTheme();
    void gameSaveAsHtml();
    void gameDiagram();
    void gameFindPlayer();
    void gameFenToClipboard();
    void gameFenFromClipboard();
    void gameSelectUserBook();
    void gameReSearch();
    void gameLoadNext();
    void gameLoadPrevious();
    void gameAnalyzePdn();
    void gameSampleDiagram();

    // Moves Menu Actions
    void movesPlay();
    void movesBack();
    void movesForward();
    void movesBackAll();
    void movesForwardAll();
    void movesComment();
    void interruptEngine();
    void abortEngine();

    // Options Menu Actions
    void levelExact();
    void levelInstant();
    void level01S();
    void level02S();
    void level05S();
    void level1S();
    void level2S();
    void level5S();
    void level10S();
    void level15S();
    void level30S();
    void level1M();
    void level2M();
    void level5M();
    void level15M();
    void level30M();
    void levelInfinite();
    void levelIncrement();
    void levelAddTime();
    void levelSubtractTime();
    void pieceSet();
    void optionsHighlight();
    void optionsSound();
    void optionsPriority();
    void options3Move();
    void optionsDirectories();
    void optionsUserBook();
    void optionsLanguageEnglish();
    void optionsLanguageEspanol();
    void optionsLanguageItaliano();
    void optionsLanguageDeutsch();
    void optionsLanguageFrancais();
    void displayInvert();
    void displayNumbers();
    void displayMirror();
    void cmNormal();
    void cmAnalysis();
    void gotoNormal();
    void cmAutoplay();
    void cm2Player();
    void engineVsEngine();
    void colorBoardNumbers();
    void colorHighlight();
    void bookModeView();
    void bookModeAdd();
    void bookModeDelete();

    // Engine Menu Actions
    void engineSelect();
    void engineAbout();
    void engineHelp();
    void engineOptions();
    void cmEngineMatch();
    void cmAddComment();
    void engineEval();
    void cmEngineCommand();
    void cmRunTestSet();
    void cmHandicap();

    // Setup Menu Actions
    void setupMode();
    void setupClear();
    void setupBlack();
    void setupWhite();
    void setupCc();

    // Help Menu Actions
    void helpHelp();
    void helpAbout();
    void helpCheckersInANutshell();
    void helpHomepage();
    void helpProblemOfTheDay();
    void helpOnlineUpgrade();

private:
    void createMenus();

    CheckerBoardWidget *checkerBoardWidget;

    // Menus
    QMenu *gameMenu;
    QMenu *movesMenu;
    QMenu *optionsMenu;
    QMenu *engineMenu;
    QMenu *setupMenu;
    QMenu *helpMenu;

    // Actions (grouped by menu for clarity)

    // Game Menu Actions
    QAction *game3MoveAction;
    QAction *gameLoadAction;
    QAction *gameSaveAction;
    QAction *gameInfoAction;
    QAction *gameAnalyzeAction;
    QAction *gameCopyAction;
    QAction *gamePasteAction;
    QAction *gameDatabaseAction;
    QAction *gameFindAction;
    QAction *gameFindCRAction;
    QAction *gameFindThemeAction;
    QAction *gameSaveAsHtmlAction;
    QAction *gameDiagramAction;
    QAction *gameFindPlayerAction;
    QAction *gameFenToClipboardAction;
    QAction *gameFenFromClipboardAction;
    QAction *gameSelectUserBookAction;
    QAction *gameReSearchAction;
    QAction *gameLoadNextAction;
    QAction *gameLoadPreviousAction;
    QAction *gameAnalyzePdnAction;
    QAction *gameSampleDiagramAction;

    // Moves Menu Actions
    QAction *movesPlayAction;
    QAction *movesBackAction;
    QAction *movesForwardAction;
    QAction *movesBackAllAction;
    QAction *movesForwardAllAction;
    QAction *movesCommentAction;
    QAction *interruptEngineAction;
    QAction *abortEngineAction;

    // Options Menu Actions
    QMenu *levelMenu;
    QAction *levelExactAction;
    QAction *levelInstantAction;
    QAction *level01SAction;
    QAction *level02SAction;
    QAction *level05SAction;
    QAction *level1SAction;
    QAction *level2SAction;
    QAction *level5SAction;
    QAction *level10SAction;
    QAction *level15SAction;
    QAction *level30SAction;
    QAction *level1MAction;
    QAction *level2MAction;
    QAction *level5MAction;
    QAction *level15MAction;
    QAction *level30MAction;
    QAction *levelInfiniteAction;
    QAction *levelIncrementAction;
    QAction *levelAddTimeAction;
    QAction *levelSubtractTimeAction;
    QAction *pieceSetAction;
    QAction *optionsHighlightAction;
    QAction *optionsSoundAction;
    QAction *optionsPriorityAction;
    QAction *options3MoveAction;
    QAction *optionsDirectoriesAction;
    QAction *optionsUserBookAction;
    QAction *optionsLanguageEnglishAction;
    QAction *optionsLanguageEspanolAction;
    QAction *optionsLanguageItalianoAction;
    QAction *optionsLanguageDeutschAction;
    QAction *optionsLanguageFrancaisAction;
    QAction *displayInvertAction;
    QAction *displayNumbersAction;
    QAction *displayMirrorAction;
    QAction *cmNormalAction;
    QAction *cmAnalysisAction;
    QAction *gotoNormalAction;
    QAction *cmAutoplayAction;
    QAction *cm2PlayerAction;
    QAction *engineVsEngineAction;
    QAction *colorBoardNumbersAction;
    QAction *colorHighlightAction;
    QAction *bookModeViewAction;
    QAction *bookModeAddAction;
    QAction *bookModeDeleteAction;

    // Engine Menu Actions
    QAction *engineSelectAction;
    QAction *engineAboutAction;
    QAction *engineHelpAction;
    QAction *engineOptionsAction;
    QAction *cmEngineMatchAction;
    QAction *cmAddCommentAction;
    QAction *engineEvalAction;
    QAction *cmEngineCommandAction;
    QAction *cmRunTestSetAction;
    QAction *cmHandicapAction;

    // Setup Menu Actions
    QAction *setupModeAction;
    QAction *setupClearAction;
    QAction *setupBlackAction;
    QAction *setupWhiteAction;
    QAction *setupCcAction;

    // Help Menu Actions
    QAction *helpHelpAction;
    QAction *helpAboutAction;
    QAction *helpCheckersInANutshellAction;
    QAction *helpHomepageAction;
    QAction *helpProblemOfTheDayAction;
    QAction *helpOnlineUpgradeAction;
};
