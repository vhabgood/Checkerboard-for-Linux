#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Checkerboard");
    setFixedSize(640, 640); // Match the CheckerBoardWidget size for now

    checkerBoardWidget = new CheckerBoardWidget(this);
    setCentralWidget(checkerBoardWidget);

    createMenus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    gameMenu = menuBar()->addMenu(tr("&Game"));
    movesMenu = menuBar()->addMenu(tr("&Moves"));
    optionsMenu = menuBar()->addMenu(tr("&Options"));
    engineMenu = menuBar()->addMenu(tr("&Engine"));
    setupMenu = menuBar()->addMenu(tr("&Setup"));
    helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *newGameAction = new QAction(tr("&New Game"), this);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::newGame);
    fileMenu->addAction(newGameAction);

    QAction *exitAction = new QAction(tr("E&xit"), this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(exitAction);

    // Game Menu Actions
    game3MoveAction = new QAction(tr("3-&Move Opening"), this);
    connect(game3MoveAction, &QAction::triggered, this, &MainWindow::game3Move);
    gameMenu->addAction(game3MoveAction);

    gameLoadAction = new QAction(tr("&Load Game..."), this);
    connect(gameLoadAction, &QAction::triggered, this, &MainWindow::gameLoad);
    gameMenu->addAction(gameLoadAction);

    gameSaveAction = new QAction(tr("&Save Game..."), this);
    connect(gameSaveAction, &QAction::triggered, this, &MainWindow::gameSave);
    gameMenu->addAction(gameSaveAction);

    gameMenu->addSeparator();

    gameInfoAction = new QAction(tr("Game &Info..."), this);
    connect(gameInfoAction, &QAction::triggered, this, &MainWindow::gameInfo);
    gameMenu->addAction(gameInfoAction);

    gameAnalyzeAction = new QAction(tr("&Analyze Game..."), this);
    connect(gameAnalyzeAction, &QAction::triggered, this, &MainWindow::gameAnalyze);
    gameMenu->addAction(gameAnalyzeAction);

    gameMenu->addSeparator();

    gameCopyAction = new QAction(tr("&Copy"), this);
    connect(gameCopyAction, &QAction::triggered, this, &MainWindow::gameCopy);
    gameMenu->addAction(gameCopyAction);

    gamePasteAction = new QAction(tr("&Paste"), this);
    connect(gamePasteAction, &QAction::triggered, this, &MainWindow::gamePaste);
    gameMenu->addAction(gamePasteAction);

    gameMenu->addSeparator();

    gameDatabaseAction = new QAction(tr("Game &Database..."), this);
    connect(gameDatabaseAction, &QAction::triggered, this, &MainWindow::gameDatabase);
    gameMenu->addAction(gameDatabaseAction);

    gameFindAction = new QAction(tr("&Find..."), this);
    connect(gameFindAction, &QAction::triggered, this, &MainWindow::gameFind);
    gameMenu->addAction(gameFindAction);

    gameFindCRAction = new QAction(tr("Find &CR..."), this);
    connect(gameFindCRAction, &QAction::triggered, this, &MainWindow::gameFindCR);
    gameMenu->addAction(gameFindCRAction);

    gameFindThemeAction = new QAction(tr("Find &Theme..."), this);
    connect(gameFindThemeAction, &QAction::triggered, this, &MainWindow::gameFindTheme);
    gameMenu->addAction(gameFindThemeAction);

    gameMenu->addSeparator();

    gameSaveAsHtmlAction = new QAction(tr("Save As &HTML..."), this);
    connect(gameSaveAsHtmlAction, &QAction::triggered, this, &MainWindow::gameSaveAsHtml);
    gameMenu->addAction(gameSaveAsHtmlAction);

    gameDiagramAction = new QAction(tr("&Diagram..."), this);
    connect(gameDiagramAction, &QAction::triggered, this, &MainWindow::gameDiagram);
    gameMenu->addAction(gameDiagramAction);

    gameFindPlayerAction = new QAction(tr("Find &Player..."), this);
    connect(gameFindPlayerAction, &QAction::triggered, this, &MainWindow::gameFindPlayer);
    gameMenu->addAction(gameFindPlayerAction);

    gameMenu->addSeparator();

    gameFenToClipboardAction = new QAction(tr("FEN to &Clipboard"), this);
    connect(gameFenToClipboardAction, &QAction::triggered, this, &MainWindow::gameFenToClipboard);
    gameMenu->addAction(gameFenToClipboardAction);

    gameFenFromClipboardAction = new QAction(tr("FEN from C&lipboard"), this);
    connect(gameFenFromClipboardAction, &QAction::triggered, this, &MainWindow::gameFenFromClipboard);
    gameMenu->addAction(gameFenFromClipboardAction);

    gameMenu->addSeparator();

    gameSelectUserBookAction = new QAction(tr("Select &User Book..."), this);
    connect(gameSelectUserBookAction, &QAction::triggered, this, &MainWindow::gameSelectUserBook);
    gameMenu->addAction(gameSelectUserBookAction);

    gameReSearchAction = new QAction(tr("&Re-Search"), this);
    connect(gameReSearchAction, &QAction::triggered, this, &MainWindow::gameReSearch);
    gameMenu->addAction(gameReSearchAction);

    gameLoadNextAction = new QAction(tr("Load &Next Game"), this);
    connect(gameLoadNextAction, &QAction::triggered, this, &MainWindow::gameLoadNext);
    gameMenu->addAction(gameLoadNextAction);

    gameLoadPreviousAction = new QAction(tr("Load &Previous Game"), this);
    connect(gameLoadPreviousAction, &QAction::triggered, this, &MainWindow::gameLoadPrevious);
    gameMenu->addAction(gameLoadPreviousAction);

    gameAnalyzePdnAction = new QAction(tr("Analyze &PDN..."), this);
    connect(gameAnalyzePdnAction, &QAction::triggered, this, &MainWindow::gameAnalyzePdn);
    gameMenu->addAction(gameAnalyzePdnAction);

    gameSampleDiagramAction = new QAction(tr("Sample &Diagram"), this);
    connect(gameSampleDiagramAction, &QAction::triggered, this, &MainWindow::gameSampleDiagram);
    gameMenu->addAction(gameSampleDiagramAction);

    // Moves Menu Actions
    movesPlayAction = new QAction(tr("&Play"), this);
    connect(movesPlayAction, &QAction::triggered, this, &MainWindow::movesPlay);
    movesMenu->addAction(movesPlayAction);

    movesBackAction = new QAction(tr("&Back"), this);
    connect(movesBackAction, &QAction::triggered, this, &MainWindow::movesBack);
    movesMenu->addAction(movesBackAction);

    movesForwardAction = new QAction(tr("&Forward"), this);
    connect(movesForwardAction, &QAction::triggered, this, &MainWindow::movesForward);
    movesMenu->addAction(movesForwardAction);

    movesBackAllAction = new QAction(tr("Back &All"), this);
    connect(movesBackAllAction, &QAction::triggered, this, &MainWindow::movesBackAll);
    movesMenu->addAction(movesBackAllAction);

    movesForwardAllAction = new QAction(tr("Forward A&ll"), this);
    connect(movesForwardAllAction, &QAction::triggered, this, &MainWindow::movesForwardAll);
    movesMenu->addAction(movesForwardAllAction);

    movesMenu->addSeparator();

    movesCommentAction = new QAction(tr("&Comment..."), this);
    connect(movesCommentAction, &QAction::triggered, this, &MainWindow::movesComment);
    movesMenu->addAction(movesCommentAction);

    movesMenu->addSeparator();

    interruptEngineAction = new QAction(tr("&Interrupt Engine"), this);
    connect(interruptEngineAction, &QAction::triggered, this, &MainWindow::interruptEngine);
    movesMenu->addAction(interruptEngineAction);

    abortEngineAction = new QAction(tr("A&bort Engine"), this);
    connect(abortEngineAction, &QAction::triggered, this, &MainWindow::abortEngine);
    movesMenu->addAction(abortEngineAction);

    // Options Menu Actions
    levelMenu = optionsMenu->addMenu(tr("&Level"));
    levelExactAction = new QAction(tr("&Exact"), this);
    connect(levelExactAction, &QAction::triggered, this, &MainWindow::levelExact);
    levelMenu->addAction(levelExactAction);

    levelInstantAction = new QAction(tr("&Instant"), this);
    connect(levelInstantAction, &QAction::triggered, this, &MainWindow::levelInstant);
    levelMenu->addAction(levelInstantAction);

    levelMenu->addSeparator();

    level01SAction = new QAction(tr("0.1 &Seconds"), this);
    connect(level01SAction, &QAction::triggered, this, &MainWindow::level01S);
    levelMenu->addAction(level01SAction);

    level02SAction = new QAction(tr("0.2 Seconds"), this);
    connect(level02SAction, &QAction::triggered, this, &MainWindow::level02S);
    levelMenu->addAction(level02SAction);

    level05SAction = new QAction(tr("0.5 Seconds"), this);
    connect(level05SAction, &QAction::triggered, this, &MainWindow::level05S);
    levelMenu->addAction(level05SAction);

    level1SAction = new QAction(tr("1 Second"), this);
    connect(level1SAction, &QAction::triggered, this, &MainWindow::level1S);
    levelMenu->addAction(level1SAction);

    level2SAction = new QAction(tr("2 Seconds"), this);
    connect(level2SAction, &QAction::triggered, this, &MainWindow::level2S);
    levelMenu->addAction(level2SAction);

    level5SAction = new QAction(tr("5 Seconds"), this);
    connect(level5SAction, &QAction::triggered, this, &MainWindow::level5S);
    levelMenu->addAction(level5SAction);

    level10SAction = new QAction(tr("10 Seconds"), this);
    connect(level10SAction, &QAction::triggered, this, &MainWindow::level10S);
    levelMenu->addAction(level10SAction);

    level15SAction = new QAction(tr("15 Seconds"), this);
    connect(level15SAction, &QAction::triggered, this, &MainWindow::level15S);
    levelMenu->addAction(level15SAction);

    level30SAction = new QAction(tr("30 Seconds"), this);
    connect(level30SAction, &QAction::triggered, this, &MainWindow::level30S);
    levelMenu->addAction(level30SAction);

    level1MAction = new QAction(tr("1 &Minute"), this);
    connect(level1MAction, &QAction::triggered, this, &MainWindow::level1M);
    levelMenu->addAction(level1MAction);

    level2MAction = new QAction(tr("2 Minutes"), this);
    connect(level2MAction, &QAction::triggered, this, &MainWindow::level2M);
    levelMenu->addAction(level2MAction);

    level5MAction = new QAction(tr("5 Minutes"), this);
    connect(level5MAction, &QAction::triggered, this, &MainWindow::level5M);
    levelMenu->addAction(level5MAction);

    level15MAction = new QAction(tr("15 Minutes"), this);
    connect(level15MAction, &QAction::triggered, this, &MainWindow::level15M);
    levelMenu->addAction(level15MAction);

    level30MAction = new QAction(tr("30 Minutes"), this);
    connect(level30MAction, &QAction::triggered, this, &MainWindow::level30M);
    levelMenu->addAction(level30MAction);

    levelMenu->addSeparator();

    levelInfiniteAction = new QAction(tr("&Infinite"), this);
    connect(levelInfiniteAction, &QAction::triggered, this, &MainWindow::levelInfinite);
    levelMenu->addAction(levelInfiniteAction);

    levelIncrementAction = new QAction(tr("&Increment..."), this);
    connect(levelIncrementAction, &QAction::triggered, this, &MainWindow::levelIncrement);
    levelMenu->addAction(levelIncrementAction);

    levelAddTimeAction = new QAction(tr("Add &Time..."), this);
    connect(levelAddTimeAction, &QAction::triggered, this, &MainWindow::levelAddTime);
    levelMenu->addAction(levelAddTimeAction);

    levelSubtractTimeAction = new QAction(tr("Subtract &Time..."), this);
    connect(levelSubtractTimeAction, &QAction::triggered, this, &MainWindow::levelSubtractTime);
    levelMenu->addAction(levelSubtractTimeAction);

    optionsMenu->addSeparator();

    pieceSetAction = new QAction(tr("&Piece Set..."), this);
    connect(pieceSetAction, &QAction::triggered, this, &MainWindow::pieceSet);
    optionsMenu->addAction(pieceSetAction);

    optionsHighlightAction = new QAction(tr("&Highlight Moves"), this);
    optionsHighlightAction->setCheckable(true);
    connect(optionsHighlightAction, &QAction::triggered, this, &MainWindow::optionsHighlight);
    optionsMenu->addAction(optionsHighlightAction);

    optionsSoundAction = new QAction(tr("&Sound"), this);
    optionsSoundAction->setCheckable(true);
    connect(optionsSoundAction, &QAction::triggered, this, &MainWindow::optionsSound);
    optionsMenu->addAction(optionsSoundAction);

    optionsPriorityAction = new QAction(tr("&Priority..."), this);
    connect(optionsPriorityAction, &QAction::triggered, this, &MainWindow::optionsPriority);
    optionsMenu->addAction(optionsPriorityAction);

    options3MoveAction = new QAction(tr("3-Move &Openings..."), this);
    connect(options3MoveAction, &QAction::triggered, this, &MainWindow::options3Move);
    optionsMenu->addAction(options3MoveAction);

    optionsDirectoriesAction = new QAction(tr("&Directories..."), this);
    connect(optionsDirectoriesAction, &QAction::triggered, this, &MainWindow::optionsDirectories);
    optionsMenu->addAction(optionsDirectoriesAction);

    optionsUserBookAction = new QAction(tr("&User Book..."), this);
    connect(optionsUserBookAction, &QAction::triggered, this, &MainWindow::optionsUserBook);
    optionsMenu->addAction(optionsUserBookAction);

    QMenu *languageMenu = optionsMenu->addMenu(tr("&Language"));
    optionsLanguageEnglishAction = new QAction(tr("&English"), this);
    optionsLanguageEnglishAction->setCheckable(true);
    connect(optionsLanguageEnglishAction, &QAction::triggered, this, &MainWindow::optionsLanguageEnglish);
    languageMenu->addAction(optionsLanguageEnglishAction);

    optionsLanguageEspanolAction = new QAction(tr("&Español"), this);
    optionsLanguageEspanolAction->setCheckable(true);
    connect(optionsLanguageEspanolAction, &QAction::triggered, this, &MainWindow::optionsLanguageEspanol);
    languageMenu->addAction(optionsLanguageEspanolAction);

    optionsLanguageItalianoAction = new QAction(tr("&Italiano"), this);
    optionsLanguageItalianoAction->setCheckable(true);
    connect(optionsLanguageItalianoAction, &QAction::triggered, this, &MainWindow::optionsLanguageItaliano);
    languageMenu->addAction(optionsLanguageItalianoAction);

    optionsLanguageDeutschAction = new QAction(tr("&Deutsch"), this);
    optionsLanguageDeutschAction->setCheckable(true);
    connect(optionsLanguageDeutschAction, &QAction::triggered, this, &MainWindow::optionsLanguageDeutsch);
    languageMenu->addAction(optionsLanguageDeutschAction);

    optionsLanguageFrancaisAction = new QAction(tr("&Français"), this);
    optionsLanguageFrancaisAction->setCheckable(true);
    connect(optionsLanguageFrancaisAction, &QAction::triggered, this, &MainWindow::optionsLanguageFrancais);
    languageMenu->addAction(optionsLanguageFrancaisAction);

    optionsMenu->addSeparator();

    displayInvertAction = new QAction(tr("&Invert Board"), this);
    displayInvertAction->setCheckable(true);
    connect(displayInvertAction, &QAction::triggered, this, &MainWindow::displayInvert);
    optionsMenu->addAction(displayInvertAction);

    displayNumbersAction = new QAction(tr("Display &Numbers"), this);
    displayNumbersAction->setCheckable(true);
    connect(displayNumbersAction, &QAction::triggered, this, &MainWindow::displayNumbers);
    optionsMenu->addAction(displayNumbersAction);

    displayMirrorAction = new QAction(tr("Display &Mirror"), this);
    displayMirrorAction->setCheckable(true);
    connect(displayMirrorAction, &QAction::triggered, this, &MainWindow::displayMirror);
    optionsMenu->addAction(displayMirrorAction);

    optionsMenu->addSeparator();

    cmNormalAction = new QAction(tr("&Normal Mode"), this);
    cmNormalAction->setCheckable(true);
    connect(cmNormalAction, &QAction::triggered, this, &MainWindow::cmNormal);
    optionsMenu->addAction(cmNormalAction);

    cmAnalysisAction = new QAction(tr("&Analysis Mode"), this);
    cmAnalysisAction->setCheckable(true);
    connect(cmAnalysisAction, &QAction::triggered, this, &MainWindow::cmAnalysis);
    optionsMenu->addAction(cmAnalysisAction);

    gotoNormalAction = new QAction(tr("&Go To Normal Mode"), this);
    connect(gotoNormalAction, &QAction::triggered, this, &MainWindow::gotoNormal);
    optionsMenu->addAction(gotoNormalAction);

    cmAutoplayAction = new QAction(tr("&Autoplay Mode"), this);
    cmAutoplayAction->setCheckable(true);
    connect(cmAutoplayAction, &QAction::triggered, this, &MainWindow::cmAutoplay);
    optionsMenu->addAction(cmAutoplayAction);

    cm2PlayerAction = new QAction(tr("&2 Player Mode"), this);
    cm2PlayerAction->setCheckable(true);
    connect(cm2PlayerAction, &QAction::triggered, this, &MainWindow::cm2Player);
    optionsMenu->addAction(cm2PlayerAction);

    engineVsEngineAction = new QAction(tr("Engine &vs. Engine"), this);
    engineVsEngineAction->setCheckable(true);
    connect(engineVsEngineAction, &QAction::triggered, this, &MainWindow::engineVsEngine);
    optionsMenu->addAction(engineVsEngineAction);

    optionsMenu->addSeparator();

    colorBoardNumbersAction = new QAction(tr("Color &Board Numbers..."), this);
    connect(colorBoardNumbersAction, &QAction::triggered, this, &MainWindow::colorBoardNumbers);
    optionsMenu->addAction(colorBoardNumbersAction);

    colorHighlightAction = new QAction(tr("Color &Highlight..."), this);
    connect(colorHighlightAction, &QAction::triggered, this, &MainWindow::colorHighlight);
    optionsMenu->addAction(colorHighlightAction);

    optionsMenu->addSeparator();

    QMenu *bookModeMenu = optionsMenu->addMenu(tr("&Book Mode"));
    bookModeViewAction = new QAction(tr("&View"), this);
    bookModeViewAction->setCheckable(true);
    connect(bookModeViewAction, &QAction::triggered, this, &MainWindow::bookModeView);
    bookModeMenu->addAction(bookModeViewAction);

    bookModeAddAction = new QAction(tr("&Add"), this);
    bookModeAddAction->setCheckable(true);
    connect(bookModeAddAction, &QAction::triggered, this, &MainWindow::bookModeAdd);
    bookModeMenu->addAction(bookModeAddAction);

    bookModeDeleteAction = new QAction(tr("&Delete"), this);
    bookModeDeleteAction->setCheckable(true);
    connect(bookModeDeleteAction, &QAction::triggered, this, &MainWindow::bookModeDelete);
    bookModeMenu->addAction(bookModeDeleteAction);

    // Engine Menu Actions
    engineSelectAction = new QAction(tr("&Select Engine..."), this);
    connect(engineSelectAction, &QAction::triggered, this, &MainWindow::engineSelect);
    engineMenu->addAction(engineSelectAction);

    engineAboutAction = new QAction(tr("&About Engine..."), this);
    connect(engineAboutAction, &QAction::triggered, this, &MainWindow::engineAbout);
    engineMenu->addAction(engineAboutAction);

    engineHelpAction = new QAction(tr("&Engine Help..."), this);
    connect(engineHelpAction, &QAction::triggered, this, &MainWindow::engineHelp);
    engineMenu->addAction(engineHelpAction);

    engineOptionsAction = new QAction(tr("Engine &Options..."), this);
    connect(engineOptionsAction, &QAction::triggered, this, &MainWindow::engineOptions);
    engineMenu->addAction(engineOptionsAction);

    engineMenu->addSeparator();

    cmEngineMatchAction = new QAction(tr("Engine &Match..."), this);
    connect(cmEngineMatchAction, &QAction::triggered, this, &MainWindow::cmEngineMatch);
    engineMenu->addAction(cmEngineMatchAction);

    cmAddCommentAction = new QAction(tr("Add &Comment..."), this);
    connect(cmAddCommentAction, &QAction::triggered, this, &MainWindow::cmAddComment);
    engineMenu->addAction(cmAddCommentAction);

    engineEvalAction = new QAction(tr("Engine &Eval"), this);
    connect(engineEvalAction, &QAction::triggered, this, &MainWindow::engineEval);
    engineMenu->addAction(engineEvalAction);

    cmEngineCommandAction = new QAction(tr("Engine &Command..."), this);
    connect(cmEngineCommandAction, &QAction::triggered, this, &MainWindow::cmEngineCommand);
    engineMenu->addAction(cmEngineCommandAction);

    cmRunTestSetAction = new QAction(tr("Run &Test Set..."), this);
    connect(cmRunTestSetAction, &QAction::triggered, this, &MainWindow::cmRunTestSet);
    engineMenu->addAction(cmRunTestSetAction);

    bookModeDeleteAction = new QAction(tr("&Delete"), this);
    bookModeDeleteAction->setCheckable(true);
    connect(bookModeDeleteAction, &QAction::triggered, this, &MainWindow::bookModeDelete);
    bookModeMenu->addAction(bookModeDeleteAction);

    // Engine Menu Actions
    engineSelectAction = new QAction(tr("&Select Engine..."), this);
    connect(engineSelectAction, &QAction::triggered, this, &MainWindow::engineSelect);
    engineMenu->addAction(engineSelectAction);

    engineAboutAction = new QAction(tr("&About Engine..."), this);
    connect(engineAboutAction, &QAction::triggered, this, &MainWindow::engineAbout);
    engineMenu->addAction(engineAboutAction);

    engineHelpAction = new QAction(tr("&Engine Help..."), this);
    connect(engineHelpAction, &QAction::triggered, this, &MainWindow::engineHelp);
    engineMenu->addAction(engineHelpAction);

    engineOptionsAction = new QAction(tr("Engine &Options..."), this);
    connect(engineOptionsAction, &QAction::triggered, this, &MainWindow::engineOptions);
    engineMenu->addAction(engineOptionsAction);

    engineMenu->addSeparator();

    cmEngineMatchAction = new QAction(tr("Engine &Match..."), this);
    connect(cmEngineMatchAction, &QAction::triggered, this, &MainWindow::cmEngineMatch);
    engineMenu->addAction(cmEngineMatchAction);

    cmAddCommentAction = new QAction(tr("Add &Comment..."), this);
    connect(cmAddCommentAction, &QAction::triggered, this, &MainWindow::cmAddComment);
    engineMenu->addAction(cmAddCommentAction);

    engineEvalAction = new QAction(tr("Engine &Eval"), this);
    connect(engineEvalAction, &QAction::triggered, this, &MainWindow::engineEval);
    engineMenu->addAction(engineEvalAction);

    cmEngineCommandAction = new QAction(tr("Engine &Command..."), this);
    connect(cmEngineCommandAction, &QAction::triggered, this, &MainWindow::cmEngineCommand);
    engineMenu->addAction(cmEngineCommandAction);

    cmRunTestSetAction = new QAction(tr("Run &Test Set..."), this);
    connect(cmRunTestSetAction, &QAction::triggered, this, &MainWindow::cmRunTestSet);
    engineMenu->addAction(cmRunTestSetAction);

    cmHandicapAction = new QAction(tr("&Handicap..."), this);
    connect(cmHandicapAction, &QAction::triggered, this, &MainWindow::cmHandicap);
    engineMenu->addAction(cmHandicapAction);

    // Setup Menu Actions
    setupModeAction = new QAction(tr("&Setup Mode"), this);
    setupModeAction->setCheckable(true);
    connect(setupModeAction, &QAction::triggered, this, &MainWindow::setupMode);
    setupMenu->addAction(setupModeAction);

    setupClearAction = new QAction(tr("&Clear Board"), this);
    connect(setupClearAction, &QAction::triggered, this, &MainWindow::setupClear);
    setupMenu->addAction(setupClearAction);

    setupBlackAction = new QAction(tr("Add &Black Piece"), this);
    connect(setupBlackAction, &QAction::triggered, this, &MainWindow::setupBlack);
    setupMenu->addAction(setupBlackAction);

    setupWhiteAction = new QAction(tr("Add &White Piece"), this);
    connect(setupWhiteAction, &QAction::triggered, this, &MainWindow::setupWhite);
    setupMenu->addAction(setupWhiteAction);

    setupCcAction = new QAction(tr("Set &Color to Move..."), this);
    connect(setupCcAction, &QAction::triggered, this, &MainWindow::setupCc);
    setupMenu->addAction(setupCcAction);

    // Help Menu Actions
    helpHelpAction = new QAction(tr("&Help"), this);
    connect(helpHelpAction, &QAction::triggered, this, &MainWindow::helpHelp);
    helpMenu->addAction(helpHelpAction);

    helpAboutAction = new QAction(tr("&About..."), this);
    connect(helpAboutAction, &QAction::triggered, this, &MainWindow::helpAbout);
    helpMenu->addAction(helpAboutAction);

    helpCheckersInANutshellAction = new QAction(tr("Checkers in a &Nutshell"), this);
    connect(helpCheckersInANutshellAction, &QAction::triggered, this, &MainWindow::helpCheckersInANutshell);
    helpMenu->addAction(helpCheckersInANutshellAction);

    helpHomepageAction = new QAction(tr("&Homepage"), this);
    connect(helpHomepageAction, &QAction::triggered, this, &MainWindow::helpHomepage);
    helpMenu->addAction(helpHomepageAction);

    helpProblemOfTheDayAction = new QAction(tr("&Problem of the Day"), this);
    connect(helpProblemOfTheDayAction, &QAction::triggered, this, &MainWindow::helpProblemOfTheDay);
    helpMenu->addAction(helpProblemOfTheDayAction);

    helpOnlineUpgradeAction = new QAction(tr("&Online Upgrade"), this);
    connect(helpOnlineUpgradeAction, &QAction::triggered, this, &MainWindow::helpOnlineUpgrade);
    helpMenu->addAction(helpOnlineUpgradeAction);
}

void MainWindow::newGame()
{
    qDebug() << "New Game action triggered";
    newgame(); // Call the C function to reset the board state
    checkerBoardWidget->setBoard(cbboard8); // Update the GUI with the new board state
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

// Game Menu Slots
void MainWindow::game3Move()
{
    qDebug() << "3-Move Opening action triggered";
    QMessageBox::information(this, tr("3-Move Opening"), tr("3-Move Opening logic will be implemented here."));
    // TODO: Implement actual 3-move opening logic here
}

void MainWindow::gameLoad()
{
    qDebug() << "Load Game action triggered";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Game"), "", tr("PDN Files (*.pdn);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, tr("Load Game"), tr("Loading game from: %1").arg(fileName));
        // TODO: Implement actual game loading logic here
    } else {
        qDebug() << "Load Game cancelled";
    }
}

void MainWindow::gameSave()
{
    qDebug() << "Save Game action triggered";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Game"), "", tr("PDN Files (*.pdn);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, tr("Save Game"), tr("Saving game to: %1").arg(fileName));
        // TODO: Implement actual game saving logic here
    } else {
        qDebug() << "Save Game cancelled";
    }
}

void MainWindow::gameInfo()
{
    qDebug() << "Game Info action triggered";
    QMessageBox::information(this, tr("Game Information"), tr("Game information will be displayed here.\nVersion: %1\nDate: %2").arg(VERSION).arg(PLACE));
    // TODO: Populate with actual game information
}

void MainWindow::gameAnalyze()
{
    qDebug() << "Analyze Game action triggered";
    QMessageBox::information(this, tr("Analyze Game"), tr("Game analysis functionality will be implemented here."));
    // TODO: Implement actual game analysis logic here
}

void MainWindow::gameCopy()
{
    qDebug() << "Copy action triggered";
    QMessageBox::information(this, tr("Copy"), tr("Copy functionality will be implemented here."));
    // TODO: Implement actual copy logic here
}

void MainWindow::gamePaste()
{
    qDebug() << "Paste action triggered";
    QMessageBox::information(this, tr("Paste"), tr("Paste functionality will be implemented here."));
    // TODO: Implement actual paste logic here
}

void MainWindow::gameDatabase()
{
    qDebug() << "Game Database action triggered";
    QMessageBox::information(this, tr("Game Database"), tr("Game database functionality will be implemented here."));
    // TODO: Implement actual game database logic here
}

void MainWindow::gameFind()
{
    qDebug() << "Find action triggered";
    QMessageBox::information(this, tr("Find"), tr("Find functionality will be implemented here."));
    // TODO: Implement actual find logic here
}

void MainWindow::gameFindCR()
{
    qDebug() << "Find CR action triggered";
    QMessageBox::information(this, tr("Find CR"), tr("Find CR functionality will be implemented here."));
    // TODO: Implement actual find CR logic here
}

void MainWindow::gameFindTheme()
{
    qDebug() << "Find Theme action triggered";
    QMessageBox::information(this, tr("Find Theme"), tr("Find Theme functionality will be implemented here."));
    // TODO: Implement actual find theme logic here
}

void MainWindow::gameSaveAsHtml()
{
    qDebug() << "Save As HTML action triggered";
    QMessageBox::information(this, tr("Save As HTML"), tr("Save game as HTML functionality will be implemented here."));
    // TODO: Implement actual save as HTML logic here
}

void MainWindow::gameDiagram()
{
    qDebug() << "Diagram action triggered";
    QMessageBox::information(this, tr("Diagram"), tr("Diagram functionality will be implemented here."));
    // TODO: Implement actual diagram logic here
}

void MainWindow::gameFindPlayer()
{
    qDebug() << "Find Player action triggered";
    QMessageBox::information(this, tr("Find Player"), tr("Find Player functionality will be implemented here."));
    // TODO: Implement actual find player logic here
}

void MainWindow::gameFenToClipboard()
{
    qDebug() << "FEN to Clipboard action triggered";
    QMessageBox::information(this, tr("FEN to Clipboard"), tr("FEN to Clipboard functionality will be implemented here."));
    // TODO: Implement actual FEN to clipboard logic here
}

void MainWindow::gameFenFromClipboard()
{
    qDebug() << "FEN from Clipboard action triggered";
    QMessageBox::information(this, tr("FEN from Clipboard"), tr("FEN from Clipboard functionality will be implemented here."));
    // TODO: Implement actual FEN from clipboard logic here
}

void MainWindow::gameSelectUserBook()
{
    qDebug() << "Select User Book action triggered";
    QMessageBox::information(this, tr("Select User Book"), tr("Select User Book functionality will be implemented here."));
    // TODO: Implement actual select user book logic here
}

void MainWindow::gameReSearch()
{
    qDebug() << "Re-Search action triggered";
    QMessageBox::information(this, tr("Re-Search"), tr("Re-Search functionality will be implemented here."));
    // TODO: Implement actual re-search logic here
}

void MainWindow::gameLoadNext()
{
    qDebug() << "Load Next Game action triggered";
    QMessageBox::information(this, tr("Load Next Game"), tr("Load Next Game functionality will be implemented here."));
    // TODO: Implement actual load next game logic here
}

void MainWindow::gameLoadPrevious()
{
    qDebug() << "Load Previous Game action triggered";
    QMessageBox::information(this, tr("Load Previous Game"), tr("Load Previous Game functionality will be implemented here."));
    // TODO: Implement actual load previous game logic here
}

void MainWindow::gameAnalyzePdn()
{
    qDebug() << "Analyze PDN action triggered";
    QMessageBox::information(this, tr("Analyze PDN"), tr("Analyze PDN functionality will be implemented here."));
    // TODO: Implement actual analyze PDN logic here
}

void MainWindow::gameSampleDiagram()
{
    qDebug() << "Sample Diagram action triggered";
    QMessageBox::information(this, tr("Sample Diagram"), tr("Sample Diagram functionality will be implemented here."));
    // TODO: Implement actual sample diagram logic here
}

// Moves Menu Slots
void MainWindow::movesPlay()
{
    qDebug() << "Play action triggered";
    QMessageBox::information(this, tr("Play Move"), tr("Play move functionality will be implemented here."));
    // TODO: Implement actual play move logic here
}

void MainWindow::movesBack()
{
    qDebug() << "Back action triggered";
    QMessageBox::information(this, tr("Move Back"), tr("Move back functionality will be implemented here."));
    // TODO: Implement actual move back logic here
}

void MainWindow::movesForward()
{
    qDebug() << "Forward action triggered";
    QMessageBox::information(this, tr("Move Forward"), tr("Move forward functionality will be implemented here."));
    // TODO: Implement actual move forward logic here
}

void MainWindow::movesBackAll()
{
    qDebug() << "Back All action triggered";
    QMessageBox::information(this, tr("Move Back All"), tr("Move back all functionality will be implemented here."));
    // TODO: Implement actual move back all logic here
}

void MainWindow::movesForwardAll()
{
    qDebug() << "Forward All action triggered";
    QMessageBox::information(this, tr("Move Forward All"), tr("Move forward all functionality will be implemented here."));
    // TODO: Implement actual move forward all logic here
}

void MainWindow::movesComment()
{
    qDebug() << "Comment action triggered";
    QMessageBox::information(this, tr("Add Comment"), tr("Add comment functionality will be implemented here."));
    // TODO: Implement actual add comment logic here
}

void MainWindow::interruptEngine()
{
    qDebug() << "Interrupt Engine action triggered";
    QMessageBox::information(this, tr("Interrupt Engine"), tr("Interrupt engine functionality will be implemented here."));
    // TODO: Implement actual interrupt engine logic here
}

void MainWindow::abortEngine()
{
    qDebug() << "Abort Engine action triggered";
    QMessageBox::information(this, tr("Abort Engine"), tr("Abort engine functionality will be implemented here."));
    // TODO: Implement actual abort engine logic here
}

// Options Menu Slots
void MainWindow::levelExact()
{
    qDebug() << "Level Exact action triggered";
    QMessageBox::information(this, tr("Level"), tr("Exact level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::levelInstant()
{
    qDebug() << "Level Instant action triggered";
    QMessageBox::information(this, tr("Level"), tr("Instant level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level01S()
{
    qDebug() << "Level 0.1 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("0.1 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level02S()
{
    qDebug() << "Level 0.2 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("0.2 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level05S()
{
    qDebug() << "Level 0.5 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("0.5 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level1S()
{
    qDebug() << "Level 1 Second action triggered";
    QMessageBox::information(this, tr("Level"), tr("1 Second level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level2S()
{
    qDebug() << "Level 2 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("2 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level5S()
{
    qDebug() << "Level 5 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("5 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level10S()
{
    qDebug() << "Level 10 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("10 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level15S()
{
    qDebug() << "Level 15 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("15 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level30S()
{
    qDebug() << "Level 30 Seconds action triggered";
    QMessageBox::information(this, tr("Level"), tr("30 Seconds level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level1M()
{
    qDebug() << "Level 1 Minute action triggered";
    QMessageBox::information(this, tr("Level"), tr("1 Minute level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level2M()
{
    qDebug() << "Level 2 Minutes action triggered";
    QMessageBox::information(this, tr("Level"), tr("2 Minutes level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level5M()
{
    qDebug() << "Level 5 Minutes action triggered";
    QMessageBox::information(this, tr("Level"), tr("5 Minutes level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level15M()
{
    qDebug() << "Level 15 Minutes action triggered";
    QMessageBox::information(this, tr("Level"), tr("15 Minutes level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::level30M()
{
    qDebug() << "Level 30 Minutes action triggered";
    QMessageBox::information(this, tr("Level"), tr("30 Minutes level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::levelInfinite()
{
    qDebug() << "Level Infinite action triggered";
    QMessageBox::information(this, tr("Level"), tr("Infinite level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::levelIncrement()
{
    qDebug() << "Level Increment action triggered";
    QMessageBox::information(this, tr("Level"), tr("Increment level functionality will be implemented here."));
    // TODO: Implement actual level logic here
}

void MainWindow::levelAddTime()
{
    qDebug() << "Level Add Time action triggered";
    QMessageBox::information(this, tr("Level"), tr("Add Time functionality will be implemented here."));
    // TODO: Implement actual add time logic here
}

void MainWindow::levelSubtractTime()
{
    qDebug() << "Level Subtract Time action triggered";
    QMessageBox::information(this, tr("Level"), tr("Subtract Time functionality will be implemented here."));
    // TODO: Implement actual subtract time logic here
}

void MainWindow::pieceSet()
{
    qDebug() << "Piece Set action triggered";
}

void MainWindow::optionsHighlight()
{
    qDebug() << "Highlight Moves action triggered";
}

void MainWindow::optionsSound()
{
    qDebug() << "Sound action triggered";
}

void MainWindow::optionsPriority()
{
    qDebug() << "Priority action triggered";
}

void MainWindow::options3Move()
{
    qDebug() << "3-Move Openings action triggered";
}

void MainWindow::optionsDirectories()
{
    qDebug() << "Directories action triggered";
}

void MainWindow::optionsUserBook()
{
    qDebug() << "User Book action triggered";
}

void MainWindow::optionsLanguageEnglish()
{
    qDebug() << "Language English action triggered";
}

void MainWindow::optionsLanguageEspanol()
{
    qDebug() << "Language Español action triggered";
}

void MainWindow::optionsLanguageItaliano()
{
    qDebug() << "Language Italiano action triggered";
}

void MainWindow::optionsLanguageDeutsch()
{
    qDebug() << "Language Deutsch action triggered";
}

void MainWindow::optionsLanguageFrancais()
{
    qDebug() << "Language Français action triggered";
}

void MainWindow::displayInvert()
{
    qDebug() << "Invert Board action triggered";
}

void MainWindow::displayNumbers()
{
    qDebug() << "Display Numbers action triggered";
}

void MainWindow::displayMirror()
{
    qDebug() << "Display Mirror action triggered";
}

void MainWindow::cmNormal()
{
    qDebug() << "Normal Mode action triggered";
}

void MainWindow::cmAnalysis()
{
    qDebug() << "Analysis Mode action triggered";
}

void MainWindow::gotoNormal()
{
    qDebug() << "Go To Normal Mode action triggered";
}

void MainWindow::cmAutoplay()
{
    qDebug() << "Autoplay Mode action triggered";
}

void MainWindow::cm2Player()
{
    qDebug() << "2 Player Mode action triggered";
}

void MainWindow::engineVsEngine()
{
    qDebug() << "Engine vs. Engine action triggered";
}

void MainWindow::colorBoardNumbers()
{
    qDebug() << "Color Board Numbers action triggered";
}

void MainWindow::colorHighlight()
{
    qDebug() << "Color Highlight action triggered";
}

void MainWindow::bookModeView()
{
    qDebug() << "Book Mode View action triggered";
}

void MainWindow::bookModeAdd()
{
    qDebug() << "Book Mode Add action triggered";
}

void MainWindow::bookModeDelete()
{
    qDebug() << "Book Mode Delete action triggered";
}

// Engine Menu Slots
void MainWindow::engineSelect()
{
    qDebug() << "Select Engine action triggered";
}

void MainWindow::engineAbout()
{
    qDebug() << "About Engine action triggered";
}

void MainWindow::engineHelp()
{
    qDebug() << "Engine Help action triggered";
}

void MainWindow::engineOptions()
{
    qDebug() << "Engine Options action triggered";
}

void MainWindow::cmEngineMatch()
{
    qDebug() << "Engine Match action triggered";
}

void MainWindow::cmAddComment()
{
    qDebug() << "Add Comment action triggered";
}

void MainWindow::engineEval()
{
    qDebug() << "Engine Eval action triggered";
}

void MainWindow::cmEngineCommand()
{
    qDebug() << "Engine Command action triggered";
}

void MainWindow::cmRunTestSet()
{
    qDebug() << "Run Test Set action triggered";
}

void MainWindow::cmHandicap()
{
    qDebug() << "Handicap action triggered";
}

// Setup Menu Slots
void MainWindow::setupMode()
{
    qDebug() << "Setup Mode action triggered";
}

void MainWindow::setupClear()
{
    qDebug() << "Clear Board action triggered";
}

void MainWindow::setupBlack()
{
    qDebug() << "Add Black Piece action triggered";
}

void MainWindow::setupWhite()
{
    qDebug() << "Add White Piece action triggered";
}

void MainWindow::setupCc()
{
    qDebug() << "Set Color to Move action triggered";
}

// Help Menu Slots
void MainWindow::helpHelp()
{
    qDebug() << "Help action triggered";
}

void MainWindow::helpAbout()
{
    qDebug() << "About action triggered";
}

void MainWindow::helpCheckersInANutshell()
{
    qDebug() << "Checkers in a Nutshell action triggered";
}

void MainWindow::helpHomepage()
{
    qDebug() << "Homepage action triggered";
}

void MainWindow::helpProblemOfTheDay()
{
    qDebug() << "Problem of the Day action triggered";
}

void MainWindow::helpOnlineUpgrade()
{
    qDebug() << "Online Upgrade action triggered";
}
