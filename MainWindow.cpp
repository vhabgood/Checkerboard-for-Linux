#include "MainWindow.h"
#include "GameManager.h"
#include "BoardWidget.h"
#include "checkers_types.h"
#include "engine_wrapper.h"

#include <QInputDialog>
#include <QColorDialog>
#include <QSettings>
#include <QMutexLocker>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices> // Required for QDesktopServices

#include "GeminiAI.h"


// Helper function

MainWindow::MainWindow(GameManager *gameManager, QWidget *parent)
    : QMainWindow(parent), m_gameManager(gameManager),
      m_currentState(STATE_NORMAL),
      m_isAnalyzing(false),
      m_isPondering(false),
      m_isInfiniteAnalyzing(false),
      m_setupPieceType(CB_EMPTY),
      m_togglePieceColorMode(false), // Initialize toggle piece color mode
      m_currentPdnGameIndex(-1) // Initialize PDN game index
{
    setWindowTitle("Checkerboard for Linux");
    resize(640, 700);

    // Create the new components
    m_boardWidget = new BoardWidget(this);
    setCentralWidget(m_boardWidget);

    // AI setup
    m_aiThread = new QThread(this);
    QSettings settings("Checkerboard", "Checkerboard");
    QString egdbPath = settings.value("Options/EGTBDirectory", QCoreApplication::applicationDirPath() + "/db/").toString();
    m_ai = new GeminiAI(egdbPath); // Pass EGDB path to AI constructor
    m_ai->moveToThread(m_aiThread);
    connect(this, &MainWindow::setPrimaryEnginePath, m_ai, &GeminiAI::setExternalEnginePath);
    connect(this, &MainWindow::setSecondaryEnginePath, m_ai, &GeminiAI::setSecondaryExternalEnginePath);
    // Load settings after AI setup
    loadSettings();
    GameManager::log(LogLevel::Debug, QString("MainWindow: EGTBdirectory after loadSettings(): %1").arg(m_options.EGTBdirectory));
    m_gameManager->setOptions(m_options); // Ensure GameManager also has updated options

    // Connect AI signals to MainWindow slots
    connect(m_ai, &GeminiAI::searchFinished, this, &MainWindow::handleSearchFinished);
    connect(m_ai, &GeminiAI::engineError, this, &MainWindow::setStatusBarText);
    connect(m_ai, &GeminiAI::requestNewGame, m_gameManager, &GameManager::newGame);
    connect(m_ai, &GeminiAI::updateStatus, this, &MainWindow::setStatusBarText);
    connect(m_ai, &GeminiAI::changeState, this, &MainWindow::changeAppState);

    // Connect thread started signal to AI's doWork slot and initEngineProcess slot
    connect(m_aiThread, &QThread::started, m_ai, &GeminiAI::doWork);
    connect(m_aiThread, &QThread::started, m_ai, &GeminiAI::initEngineProcess);
    connect(m_aiThread, &QThread::started, m_ai, &GeminiAI::init); // Connect init() slot
    connect(m_aiThread, &QThread::finished, m_ai, &GeminiAI::quitEngineProcess);



    // Start the AI thread
    m_aiThread->start();

    // Setup UI
    createMenus();
    createToolBars();

    // Create clock labels
    m_blackClockLabel = new QLabel(" B: 0.0 ", this);
    m_whiteClockLabel = new QLabel(" W: 0.0 ", this);
    m_evaluationLabel = new QLabel(" Eval: 0 ", this); // Initialize evaluation label
    statusBar()->addPermanentWidget(m_blackClockLabel);
    statusBar()->addPermanentWidget(m_whiteClockLabel);
    statusBar()->addPermanentWidget(m_evaluationLabel); // Add evaluation label to status bar

    // m_gameManager->setOptions(m_options); // Already called above
    m_boardWidget->setPieceSet(m_options.piece_set); // Set initial piece set
    restoreGeometry(settings.value("Window/Geometry").toByteArray());

    // Connect signals from UI components to GameManager/AI
    connect(m_boardWidget, &BoardWidget::squareClicked, this, [this](int x, int y){
        if (m_currentState == STATE_NORMAL) {
            m_gameManager->handleSquareClick(x, y);
        } else if (m_currentState == STATE_SETUP) {
            if (m_boardWidget->isTogglePieceColorMode()) {
                m_gameManager->togglePieceColor(x, y);
            } else {
                m_gameManager->setPiece(x, y, m_boardWidget->getSetupPieceType());
            }
        }
    });
    // Connect signals from GameManager to MainWindow's slots
    connect(m_gameManager, &GameManager::boardUpdated, this, &MainWindow::handleBoardUpdated);
    connect(m_gameManager, &GameManager::gameMessage, this, &MainWindow::setStatusBarText);
    connect(m_gameManager, &GameManager::gameIsOver, this, &MainWindow::handleGameOver);
    connect(m_gameManager, &GameManager::pieceSelected, m_boardWidget, &BoardWidget::setSelectedPiece);
    connect(m_gameManager, &GameManager::pieceDeselected, m_boardWidget, &BoardWidget::clearSelectedPiece);
    connect(m_gameManager, &GameManager::humanTurn, this, [this]() {
        changeAppState(STATE_NORMAL);
        setStatusBarText("Your turn.");
    });
    // ----- THIS IS THE FIXED CODE -----
    connect(m_gameManager, &GameManager::requestEngineSearch, m_ai, &GeminiAI::requestMove);

    // Connect signals from AI to MainWindow/GameManager
    connect(m_ai, &GeminiAI::engineError, this, &MainWindow::setStatusBarText);
    connect(m_ai, &GeminiAI::requestNewGame, m_gameManager, &GameManager::newGame);

    // Connect signals for evaluation display
    connect(m_ai, &GeminiAI::evaluationReady, m_gameManager, &GameManager::handleEvaluationUpdate);
    connect(m_gameManager, &GameManager::evaluationUpdated, this, &MainWindow::updateEvaluationDisplay);

    // UI Update Timer
    m_uiUpdateTimer = new QTimer(this);
    m_uiUpdateTimer->start(100);

    GameManager::log(LogLevel::Info, "MainWindow initialized.");
    m_gameManager->newGame(GT_ENGLISH); // Start a new game on launch
}

void MainWindow::handleSearchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime)
{
    // GameManager::log(LogLevel::Debug, QString("handleSearchFinished called. Current state: %1").arg(m_currentState)); // Removed verbose log
    Q_UNUSED(aborted);
    Q_UNUSED(gameResult);
    Q_UNUSED(pdnMoveText);
    Q_UNUSED(elapsedTime);

    setStatusBarText(statusText);
    if (moveFound) {
        m_gameManager->makeMove(bestMove);
        m_gameManager->switchTurn();
    }

    // If still in autoplay mode, request the next move
    if (m_currentState == STATE_AUTOPLAY) {
        // GameManager::log(LogLevel::Debug, "handleSearchFinished: In autoplay mode."); // Removed verbose log
        // Check if the game is over before requesting the next move
        // This check should ideally be done in GameManager after makeMove
        // For now, we\'ll rely on GameManager::makeMove to emit gameIsOver if applicable.
        // If game is not over, request next move.
        // A small delay might be desirable for visual feedback during autoplay.
        QTimer::singleShot(500, this, [this]() {
            if (m_currentState == STATE_AUTOPLAY) { // Re-check state in case it changed during delay
                m_gameManager->playMove();
            }
        });
    } else {
        // GameManager::log(LogLevel::Debug, "handleSearchFinished: Not in autoplay mode."); // Removed verbose log
    }
}

MainWindow::~MainWindow()
{
    m_ai->requestAbort(); // Signal the AI thread to stop
    m_aiThread->quit();
    if (!m_aiThread->wait(2000)) { // Wait for 2 seconds
        GameManager::log(LogLevel::Warning, "AI thread did not stop, terminating...");
        m_aiThread->terminate();
        m_aiThread->wait();
    }
    delete m_aiThread;
    delete m_ai;
}

/**
 * @brief Changes the application's current operational state and updates the UI accordingly.
 *
 * This function is central to managing the application's behavior, as different states
 * (e.g., normal play, setup mode, analysis mode) require different UI elements to be
 * enabled or disabled. It ensures that only relevant actions are available to the user
 * in a given context.
 *
 * @param newState The `AppState` enum value representing the new state to transition to.
 *
 * @output
 *   - Updates the `m_currentState` member to `newState`.
 *   - Iterates through various `QAction` objects and `QMenu` objects, enabling or disabling
 *     them based on the `newState`.
 *   - Emits `qDebug()` messages to log state transitions.
 */
AI_State MainWindow::mapAppStatetoAIState(AppState appState)
{
    switch (appState) {
        case STATE_IDLE:
        case STATE_PLAYING:
        case STATE_BOOKADD:
        case STATE_GAMEOVER:
        case STATE_NORMAL:
        case STATE_SETUP:
        case STATE_ENGINE_THINKING: // AI is thinking, but its mode is not explicitly set to "thinking" in AI_State, it's handled by doSearch and runStateMachine
        case STATE_2PLAYER:
        case STATE_BOOKDELETE:
                    return Idle;        case STATE_ANALYSIS:
        case STATE_ANALYZEGAME:
            return AnalyzeGame;
        case STATE_ENGINE_MATCH:
            return EngineMatch;
        case STATE_AUTOPLAY:
            return Autoplay;
        case STATE_RUNTESTSET:
            return RunTestSet;
        case STATE_ANALYZEPDN:
            return AnalyzePdn;
        default:
            GameManager::log(LogLevel::Warning, QString("Unknown AppState: %1. Mapping to Idle").arg(appState));
            return Idle;
    }
}

void MainWindow::changeAppState(AppState newState)
{
    // GameManager::log(LogLevel::Debug, QString("changeAppState called with newState: %1").arg(newState)); // Removed verbose log
    m_currentState = newState;
    m_ai->setMode(mapAppStatetoAIState(newState));

    // Define state groups
    bool isNormal = (newState == STATE_NORMAL);
    bool isEngineThinking = (newState == STATE_ENGINE_THINKING);
    bool isSetup = (newState == STATE_SETUP);
    bool isAnalysis = (newState == STATE_ANALYZEGAME || newState == STATE_ANALYZEPDN);
    bool isEngineMatch = (newState == STATE_ENGINE_MATCH);
    bool isRunTestSet = (newState == STATE_RUNTESTSET);
    bool isBookMode = (newState == STATE_BOOKADD || newState == STATE_BOOKDELETE || newState == STATE_BOOKVIEW);
    bool is2Player = (newState == STATE_2PLAYER);
    bool isAutoplay = (newState == STATE_AUTOPLAY);

    bool engineIsActive = isEngineThinking || isAnalysis || isEngineMatch || isRunTestSet;
    bool gameInProgress = isNormal || isEngineThinking || isAnalysis || isEngineMatch || is2Player || isAutoplay;

    // File Menu Actions
    m_newGameAction->setEnabled(isNormal || isSetup);
    m_game3MoveAction->setEnabled(isNormal);
    m_gameLoadAction->setEnabled(isNormal);
    m_gameSaveAction->setEnabled(isNormal || isAnalysis);

    // Game Menu Actions
    m_gameAnalyzeAction->setEnabled(isNormal);
    m_gameCopyAction->setEnabled(gameInProgress || isSetup || isAnalysis);
    m_gamePasteAction->setEnabled(isNormal || isSetup);
    m_gameDatabaseAction->setEnabled(isNormal);
    m_gameFenToClipboardAction->setEnabled(gameInProgress || isSetup || isAnalysis);
    m_gameFenFromClipboardAction->setEnabled(isNormal || isSetup);
    m_gameAnalyzePdnAction->setEnabled(isNormal);

    // Moves Menu Actions
    m_movesPlayAction->setEnabled(isNormal);
    m_movesBackAction->setEnabled(gameInProgress || isAnalysis);
    m_movesForwardAction->setEnabled(gameInProgress || isAnalysis);
    m_movesBackAllAction->setEnabled(gameInProgress || isAnalysis);
    m_movesForwardAllAction->setEnabled(gameInProgress || isAnalysis);
    m_movesCommentAction->setEnabled(gameInProgress || isAnalysis);
    m_interruptEngineAction->setEnabled(engineIsActive);
    m_abortEngineAction->setEnabled(engineIsActive);

    // Options Menu Actions
    m_optionsHighlightAction->setEnabled(true);
    m_optionsSoundAction->setEnabled(true);
    m_displayInvertAction->setEnabled(true);
    m_displayNumbersAction->setEnabled(true);

    m_cmNormalAction->setEnabled(!isNormal);
    m_cmAnalysisAction->setEnabled(!isAnalysis);
    m_cmAutoplayAction->setEnabled(!isAutoplay);
    m_cm2PlayerAction->setEnabled(!is2Player);
    m_engineVsEngineAction->setEnabled(!isEngineMatch);

    m_bookModeViewAction->setEnabled(isNormal || isBookMode);
    m_bookModeAddAction->setEnabled(isNormal || isBookMode);
    m_bookModeDeleteAction->setEnabled(isNormal || isBookMode);

    // Engine Menu Actions
    m_engineSelectAction->setEnabled(isNormal);
    m_engineOptionsAction->setEnabled(isNormal);
    m_engineEvalAction->setEnabled(isNormal);
    m_cmEngineMatchAction->setEnabled(isNormal);
    m_cmAddCommentAction->setEnabled(gameInProgress || isAnalysis);
    m_cmEngineCommandAction->setEnabled(isNormal);
    m_cmRunTestSetAction->setEnabled(isNormal);

    // Setup Menu Actions
    m_setupModeAction->setEnabled(isNormal);
    m_setupClearAction->setEnabled(isSetup);
    m_setupBlackAction->setEnabled(isSetup);
    m_setupWhiteAction->setEnabled(isSetup);
    m_setupCcAction->setEnabled(isSetup);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    GameManager::log(LogLevel::Info, "MainWindow::closeEvent triggered.");
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::handleBoardUpdated(const Board8x8& board)
{
    m_boardWidget->setBoard(board);
}

void MainWindow::handleGameMessage(const QString& message)
{
    setStatusBarText(message);
}

void MainWindow::handleGameOver(int result)
{
    GameManager::log(LogLevel::Info, QString("MainWindow::handleGameOver triggered with result: %1").arg(result));
    QString message;
    switch(result) {
        case CB_WIN: message = "Game Over: You Win!"; break;
        case CB_LOSS: message = "Game Over: You Lose!"; break;
        case CB_DRAW: message = "Game Over: Draw!"; break;
        default: message = "Game Over!"; break;
    }
    QMessageBox::information(this, "Game Over", message);
    setStatusBarText(message);
}

// --- Private Methods ---
void MainWindow::createMenus()
{
    // File Menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    m_newGameAction = fileMenu->addAction(tr("&New Game"), this, &MainWindow::newGame);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &MainWindow::exitApplication);

    // Game Menu
    QMenu *gameMenu = menuBar()->addMenu(tr("&Game"));
    m_game3MoveAction = gameMenu->addAction(tr("3-Move Game"), this, &MainWindow::game3Move);
    m_gameLoadAction = gameMenu->addAction(tr("Load Game"), this, &MainWindow::gameLoad);
    m_gameSaveAction = gameMenu->addAction(tr("Save Game"), this, &MainWindow::gameSave);
    m_gameAnalyzeAction = gameMenu->addAction(tr("Analyze Game"), this, &MainWindow::gameAnalyze);
    m_gameCopyAction = gameMenu->addAction(tr("Copy Game"), this, &MainWindow::gameCopy);
    m_gamePasteAction = gameMenu->addAction(tr("Paste Game"), this, &MainWindow::gamePaste);
    m_gameDatabaseAction = gameMenu->addAction(tr("Game Database"), this, &MainWindow::gameDatabase);
    m_gameFenToClipboardAction = gameMenu->addAction(tr("FEN to Clipboard"), this, &MainWindow::gameFenToClipboard);
    m_gameFenFromClipboardAction = gameMenu->addAction(tr("FEN from Clipboard"), this, &MainWindow::gameFenFromClipboard);
    m_gameAnalyzePdnAction = gameMenu->addAction(tr("Analyze PDN"), this, &MainWindow::gameAnalyzePdn);
    gameMenu->addAction(tr("Game Info"), this, &MainWindow::gameInfo);
    gameMenu->addSeparator();
    gameMenu->addAction(tr("Find"), this, &MainWindow::gameFind);
    gameMenu->addAction(tr("Find CR"), this, &MainWindow::gameFindCR);
    gameMenu->addAction(tr("Find Theme"), this, &MainWindow::gameFindTheme);
    gameMenu->addAction(tr("Find Player"), this, &MainWindow::gameFindPlayer);
    gameMenu->addSeparator();
    gameMenu->addAction(tr("Save As HTML"), this, &MainWindow::gameSaveAsHtml);
    gameMenu->addAction(tr("Diagram"), this, &MainWindow::gameDiagram);
    gameMenu->addAction(tr("Sample Diagram"), this, &MainWindow::gameSampleDiagram);
    gameMenu->addSeparator();
    gameMenu->addAction(tr("Select User Book"), this, &MainWindow::gameSelectUserBook);
    gameMenu->addAction(tr("Re-Search"), this, &MainWindow::gameReSearch);
    gameMenu->addAction(tr("Load Next"), this, &MainWindow::gameLoadNext);
    gameMenu->addAction(tr("Load Previous"), this, &MainWindow::gameLoadPrevious);

    // Moves Menu
    QMenu *movesMenu = menuBar()->addMenu(tr("&Moves"));
    m_movesPlayAction = movesMenu->addAction(tr("Play"), this, &MainWindow::movesPlay);
    m_movesBackAction = movesMenu->addAction(tr("Back"), this, &MainWindow::movesBack);
    m_movesForwardAction = movesMenu->addAction(tr("Forward"), this, &MainWindow::movesForward);
    m_movesBackAllAction = movesMenu->addAction(tr("Back All"), this, &MainWindow::movesBackAll);
    m_movesForwardAllAction = movesMenu->addAction(tr("Forward All"), this, &MainWindow::movesForwardAll);
    m_movesCommentAction = movesMenu->addAction(tr("Comment"), this, &MainWindow::movesComment);
    movesMenu->addSeparator();
    m_interruptEngineAction = movesMenu->addAction(tr("Interrupt Engine"), this, &MainWindow::interruptEngine);
    m_abortEngineAction = movesMenu->addAction(tr("Abort Engine"), this, &MainWindow::abortEngine);

    // Options Menu
    QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));

    // Level Submenu
    QMenu *levelMenu = optionsMenu->addMenu(tr("&Level"));
    levelMenu->addAction(tr("Exact"), this, &MainWindow::levelExact);
    levelMenu->addAction(tr("Instant"), this, &MainWindow::levelInstant);
    levelMenu->addSeparator();
    levelMenu->addAction(tr("0.1s"), this, &MainWindow::level01S);
    levelMenu->addAction(tr("0.2s"), this, &MainWindow::level02S);
    levelMenu->addAction(tr("0.5s"), this, &MainWindow::level05S);
    levelMenu->addAction(tr("1s"), this, &MainWindow::level1S);
    levelMenu->addAction(tr("2s"), this, &MainWindow::level2S);
    levelMenu->addAction(tr("5s"), this, &MainWindow::level5S);
    levelMenu->addAction(tr("10s"), this, &MainWindow::level10S);
    levelMenu->addAction(tr("15s"), this, &MainWindow::level15S);
    levelMenu->addAction(tr("30s"), this, &MainWindow::level30S);
    levelMenu->addAction(tr("1m"), this, &MainWindow::level1M);
    levelMenu->addAction(tr("2m"), this, &MainWindow::level2M);
    levelMenu->addAction(tr("5m"), this, &MainWindow::level5M);
    levelMenu->addAction(tr("15m"), this, &MainWindow::level15M);
    levelMenu->addAction(tr("30m"), this, &MainWindow::level30M);
    levelMenu->addSeparator();
    levelMenu->addAction(tr("Infinite"), this, &MainWindow::levelInfinite);
    levelMenu->addAction(tr("Increment"), this, &MainWindow::levelIncrement);
    levelMenu->addAction(tr("Add Time"), this, &MainWindow::levelAddTime);
    levelMenu->addAction(tr("Subtract Time"), this, &MainWindow::levelSubtractTime);

    // Other Options
    optionsMenu->addSeparator();
    optionsMenu->addAction(tr("Piece Set"), this, &MainWindow::pieceSet);
    m_optionsHighlightAction = optionsMenu->addAction(tr("Highlight"), this, &MainWindow::optionsHighlight);
    m_optionsSoundAction = optionsMenu->addAction(tr("Sound"), this, &MainWindow::optionsSound);
    optionsMenu->addAction(tr("Priority"), this, &MainWindow::optionsPriority);
    optionsMenu->addAction(tr("3-Move"), this, &MainWindow::options3Move);
    optionsMenu->addAction(tr("Directories"), this, &MainWindow::optionsDirectories);
    optionsMenu->addAction(tr("User Book"), this, &MainWindow::optionsUserBook);

    // Language Submenu
    QMenu *languageMenu = optionsMenu->addMenu(tr("&Language"));
    languageMenu->addAction(tr("English"), this, &MainWindow::optionsLanguageEnglish);
    languageMenu->addAction(tr("Español"), this, &MainWindow::optionsLanguageEspanol);
    languageMenu->addAction(tr("Italiano"), this, &MainWindow::optionsLanguageItaliano);
    languageMenu->addAction(tr("Deutsch"), this, &MainWindow::optionsLanguageDeutsch);
    languageMenu->addAction(tr("Français"), this, &MainWindow::optionsLanguageFrancais);

    // Display Submenu
    QMenu *displayMenu = optionsMenu->addMenu(tr("&Display"));
    m_displayInvertAction = displayMenu->addAction(tr("Invert"), this, &MainWindow::displayInvert);
    m_displayNumbersAction = displayMenu->addAction(tr("Numbers"), this, &MainWindow::displayNumbers);
    m_displayMirrorAction = displayMenu->addAction(tr("Mirror"), this, &MainWindow::displayMirror);

    // Mode Submenu
    QMenu *modeMenu = optionsMenu->addMenu(tr("&Mode"));
    m_cmNormalAction = modeMenu->addAction(tr("Normal"), this, &MainWindow::cmNormal);
    m_cmAnalysisAction = modeMenu->addAction(tr("Analysis"), this, &MainWindow::cmAnalysis);
    modeMenu->addAction(tr("Go To Normal"), this, &MainWindow::gotoNormal);
    m_cmAutoplayAction = modeMenu->addAction(tr("Autoplay"), this, &MainWindow::cmAutoplay);
    m_cm2PlayerAction = modeMenu->addAction(tr("2-Player"), this, &MainWindow::cm2Player);
    m_engineVsEngineAction = modeMenu->addAction(tr("Engine vs Engine"), this, &MainWindow::engineVsEngine);

    // Colors Submenu
    QMenu *colorsMenu = optionsMenu->addMenu(tr("&Colors"));
    colorsMenu->addAction(tr("Board Numbers"), this, &MainWindow::colorBoardNumbers);
    colorsMenu->addAction(tr("Highlight"), this, &MainWindow::colorHighlight);

    // Book Mode Submenu
    QMenu *bookModeMenu = optionsMenu->addMenu(tr("&Book Mode"));
    m_bookModeViewAction = bookModeMenu->addAction(tr("View"), this, &MainWindow::bookModeView);
    m_bookModeAddAction = bookModeMenu->addAction(tr("Add"), this, &MainWindow::bookModeAdd);
    m_bookModeDeleteAction = bookModeMenu->addAction(tr("Delete"), this, &MainWindow::bookModeDelete);
    m_bookModeDeleteAllAction = bookModeMenu->addAction(tr("Delete All"), this, &MainWindow::bookModeDeleteAll);
    m_bookModeDeleteAllAction = bookModeMenu->addAction(tr("Delete All"), this, &MainWindow::bookModeDeleteAll);

    // Engine Menu
    QMenu *engineMenu = menuBar()->addMenu(tr("&Engine"));
    m_engineSelectAction = engineMenu->addAction(tr("Select"), this, &MainWindow::engineSelect);
    m_engineOptionsAction = engineMenu->addAction(tr("Options"), this, &MainWindow::engineOptions);
    m_engineEvalAction = engineMenu->addAction(tr("Eval"), this, &MainWindow::engineEval);
    engineMenu->addSeparator();
    m_cmEngineMatchAction = engineMenu->addAction(tr("Engine Match"), this, &MainWindow::cmEngineMatch);
    m_cmAddCommentAction = engineMenu->addAction(tr("Add Comment"), this, &MainWindow::cmAddComment);
    m_cmEngineCommandAction = engineMenu->addAction(tr("Engine Command"), this, &MainWindow::cmEngineCommand);
    m_cmRunTestSetAction = engineMenu->addAction(tr("Run Test Set"), this, &MainWindow::cmRunTestSet);
    engineMenu->addAction(tr("Handicap"), this, &MainWindow::cmHandicap);
    engineMenu->addSeparator();
    engineMenu->addAction(tr("About"), this, &MainWindow::engineAbout);
    engineMenu->addAction(tr("Help"), this, &MainWindow::engineHelp);
    engineMenu->addSeparator();
    engineMenu->addAction(tr("Analyze"), this, &MainWindow::engineAnalyze);
    engineMenu->addAction(tr("Infinite"), this, &MainWindow::engineInfinite);
    engineMenu->addAction(tr("Resign"), this, &MainWindow::engineResign);
    engineMenu->addAction(tr("Draw"), this, &MainWindow::engineDraw);
    engineMenu->addSeparator();
    engineMenu->addAction(tr("Undo Move"), this, &MainWindow::engineUndoMove);
    engineMenu->addAction(tr("Ponder"), this, &MainWindow::enginePonder);

    // Setup Menu
    QMenu *setupMenu = menuBar()->addMenu(tr("&Setup"));
    m_setupModeAction = setupMenu->addAction(tr("Setup Mode"), this, &MainWindow::setupMode);
    setupMenu->addSeparator();
    m_setupClearAction = setupMenu->addAction(tr("Clear"), this, &MainWindow::setupClear);
    m_setupBlackAction = setupMenu->addAction(tr("Black"), this, &MainWindow::setupBlack);
    m_setupWhiteAction = setupMenu->addAction(tr("White"), this, &MainWindow::setupWhite);
    m_setupCcAction = setupMenu->addAction(tr("Change Color"), this, &MainWindow::setupCc);

    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("Help"), this, &MainWindow::helpHelp);
//    helpMenu->addAction(tr("Checkers In A Nutshell"), this, &MainWindow::helpCheckersInANutshell);
// not yet implemented    helpMenu->addAction(tr("Homepage"), this, &MainWindow::helpHomepage);
//    helpMenu->addAction(tr("Problem Of The Day"), this, &MainWindow::helpProblemOfTheDay);
 //   helpMenu->addAction(tr("Online Upgrade"), this, &MainWindow::helpOnlineUpgrade);
    helpMenu->addSeparator();
//    helpMenu->addAction(tr("About"), this, &MainWindow::helpAbout);
    helpMenu->addAction(tr("About Qt"), this, &MainWindow::helpAboutQt);
    helpMenu->addAction(tr("Contents"), this, &MainWindow::helpContents);
}
void MainWindow::helpAbout() {
    GameManager::log(LogLevel::Info, "Help About action triggered.");
    QMessageBox::about(this, tr("About Checkerboard"),
                       tr("Checkerboard for Linux\n\n" 
                          "Version 1.0\n\n" 
                          "A cross-platform checkers application.\n\n" 
                          "Based on the original Windows Checkerboard by Martin Fierz."));
}

void MainWindow::createToolBars()
{
    m_mainToolBar = addToolBar(tr("Main Toolbar"));
    m_mainToolBar->addAction(tr("New Game"), this, &MainWindow::newGame);
    // Add other common actions
}

void MainWindow::loadSettings()
{
    GameManager::log(LogLevel::Info, "Loading settings.");
    QSettings settings("Checkerboard", "Checkerboard");

    // Restore window geometry and state
    restoreGeometry(settings.value("Window/Geometry").toByteArray());
    restoreState(settings.value("Window/State").toByteArray());

    QMutexLocker locker(&m_optionsMutex); // Assuming m_optionsMutex exists

    // Load various game options into m_options
    m_options.sound = settings.value("Options/Sound", true).toBool();
    m_options.highlight = settings.value("Options/Highlight", true).toBool();
    m_options.invert_board = settings.value("Options/InvertBoard", false).toBool();
    m_options.show_coordinates = settings.value("Options/ShowCoordinates", true).toBool();
    m_options.gametype = settings.value("Options/GameType", GT_ENGLISH).toInt();
    m_options.engine_path = settings.value("Options/EnginePath", "").toString();
    m_options.secondary_engine_path = settings.value("Options/SecondaryEnginePath", "").toString();
    m_options.engine_name = settings.value("Options/EngineName", "Default Engine").toString();
    m_options.engine_options = settings.value("Options/EngineOptions", "").toString();
    m_options.book_path = settings.value("Options/BookPath", "").toString();
    m_options.language = static_cast<Language>(settings.value("Options/Language", LANG_ENGLISH).toInt()); // Load language
    m_options.current_engine = settings.value("Options/CurrentEngine", 0).toInt(); // Load current engine
    m_options.piece_set = settings.value("Options/PieceSet", "standard").toString(); // Load piece set
    m_options.priority = settings.value("Options/Priority", 0).toInt(); // Load priority
    m_options.three_move_option = settings.value("Options/ThreeMoveOption", 0).toInt();
    m_options.mirror = settings.value("Options/Mirror", false).toBool(); // Load mirror setting
// ----- THIS IS THE FIXED CODE -----
    m_options.enable_game_timer = settings.value("Options/EnableGameTimer", false).toBool();
    m_options.enable_game_timer = false; // <-- ADD THIS LINE to force it off.
    m_options.initial_time = settings.value("Options/InitialTime", 300).toInt(); // Default to 5 minutes (300 seconds)
    m_options.time_increment = settings.value("Options/TimeIncrement", 0).toInt(); // Default to 0 increment
    m_options.enable_game_timer = false; // Force disable the game timer for now
    m_options.white_player_type = static_cast<PlayerType>(settings.value("Options/WhitePlayerType", PLAYER_AI).toInt());
    m_options.white_player_type = PLAYER_AI; // <-- ADD THIS LINE to force it.
    m_options.black_player_type = static_cast<PlayerType>(settings.value("Options/BlackPlayerType", PLAYER_HUMAN).toInt());


    // Apply loaded settings to UI and other components
    strncpy(m_options.userdirectory, settings.value("Options/UserDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString().toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.userdirectory[MAX_PATH_FIXED - 1] = '\0';
    strncpy(m_options.matchdirectory, settings.value("Options/MatchDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString().toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.matchdirectory[MAX_PATH_FIXED - 1] = '\0';
    // Determine the preferred default EGTB directory (application's db folder)
    QString defaultEgdbPath = QCoreApplication::applicationDirPath() + "/db/";

    // Load the stored EGTB directory from settings. If not found, use the application's Documents location as a temporary default.
    QString storedEgdbPath = settings.value("Options/EGTBDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

    // The final path will be the default app path, unless a valid stored path overrides it.
    QString finalEgdbPath = defaultEgdbPath;

    // Check if the stored path is valid and exists. If so, use it.
    if (!storedEgdbPath.isEmpty() && QDir(storedEgdbPath).exists()) {
        finalEgdbPath = storedEgdbPath;
        GameManager::log(LogLevel::Info, QString("Using stored EGTB directory: %1").arg(finalEgdbPath));
    } else {
        GameManager::log(LogLevel::Warning, QString("Stored EGTB directory '%1' is invalid or does not exist. Using default: %2").arg(storedEgdbPath).arg(defaultEgdbPath));
    }
    
    // Copy the chosen path into the C-style string struct member
    strncpy(m_options.EGTBdirectory, finalEgdbPath.toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.EGTBdirectory[MAX_PATH_FIXED - 1] = '\0';
            // ... load other options as needed
    
        // Set external engine paths in AI
        emit setPrimaryEnginePath(m_options.engine_path);
        emit setSecondaryEnginePath(m_options.secondary_engine_path);
    
            GameManager::log(LogLevel::Info, "Settings loaded.");}

void MainWindow::saveSettings()
{
    GameManager::log(LogLevel::Info, "Saving settings.");
    QSettings settings("Checkerboard", "Checkerboard");

    // Save window geometry and state
    settings.setValue("Window/Geometry", saveGeometry());
    settings.setValue("Window/State", saveState());

    QMutexLocker locker(&m_optionsMutex); // Assuming m_optionsMutex exists

    // Save various game options from m_options
    settings.setValue("Options/Sound", m_options.sound);
    settings.setValue("Options/Highlight", m_options.highlight);
    settings.setValue("Options/InvertBoard", m_options.invert_board);
    settings.setValue("Options/ShowCoordinates", m_options.show_coordinates);
    settings.setValue("Options/TimePerMove", m_options.time_per_move);
    settings.setValue("Options/EnginePath", m_options.engine_path);
    settings.setValue("Options/SecondaryEnginePath", m_options.secondary_engine_path);
    settings.setValue("Options/EngineName", m_options.engine_name);
    settings.setValue("Options/EngineOptions", m_options.engine_options);
    settings.setValue("Options/BookPath", m_options.book_path);
    settings.setValue("Options/Language", static_cast<int>(m_options.language)); // Save language
    settings.setValue("Options/CurrentEngine", m_options.current_engine); // Save current engine
    settings.setValue("Options/PieceSet", m_options.piece_set); // Save piece set
    settings.setValue("Options/Priority", m_options.priority); // Save priority
    settings.setValue("Options/ThreeMoveOption", m_options.three_move_option);
    settings.setValue("Options/Mirror", m_options.mirror); // Save mirror setting
    settings.setValue("Options/WhitePlayerType", static_cast<int>(m_options.white_player_type));
    settings.setValue("Options/BlackPlayerType", static_cast<int>(m_options.black_player_type));

    settings.setValue("Options/UserDirectory", QString(m_options.userdirectory));
    settings.setValue("Options/MatchDirectory", QString(m_options.matchdirectory));
    settings.setValue("Options/EGTBDirectory", QString(m_options.EGTBdirectory));
    // ... save other options as needed

    GameManager::log(LogLevel::Info, "Settings saved.");
}
/**
 * @brief Loads application settings from persistent storage (QSettings).
 *
 * This function is called during `MainWindow` initialization to restore user preferences
 * and application state from the last session. It reads settings related to window
 * geometry, window state, and various game options stored in the `m_options` struct.
 *
 * @input None.
 *
 * @output
 *   - Restores `MainWindow`'s geometry and state using `restoreGeometry()` and `restoreState()`.
 *   - Populates the `m_options` (CBoptions struct) with saved values for sound, display, time control,
 *     directories, and engine paths. Default values are used if a setting is not found.
 *   - Acquires a `QMutexLocker` to ensure thread-safe access to `m_options`.
 */




void MainWindow::newGame()
{
    GameManager::log(LogLevel::Info, "New Game action triggered.");
    m_gameManager->newGame(GT_ENGLISH); // Or get type from a dialog
}

void MainWindow::exitApplication()
{
    GameManager::log(LogLevel::Info, "Exit Application action triggered.");
    close();
}

// --- Stubs for other menu actions ---
void MainWindow::game3Move() {
    GameManager::log(LogLevel::Info, "Game 3-Move action triggered.");
    // Prompt user for opening index
    bool ok;
    int opening_index = QInputDialog::getInt(this, tr("Select 3-Move Opening"),
                                             tr("Opening Index (0-173):"), 0, 0, 173, 1, &ok);
    if (ok) {
        m_gameManager->start3MoveGame(opening_index);
        setStatusBarText(QString("Started 3-move game with opening %1.").arg(opening_index));
    } else {
        setStatusBarText("3-Move opening selection cancelled.");
    }
}
void MainWindow::gameLoad() {
    GameManager::log(LogLevel::Info, "Game Load action triggered.");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load PDN Game"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("PDN Files (*.pdn);;All Files (*)"));
    if (!fileName.isEmpty()) {
        m_gameManager->loadPdnGame(fileName);
        // Add to history and update index
        int existingIndex = m_pdnGameHistory.indexOf(fileName);
        if (existingIndex != -1) {
            m_currentPdnGameIndex = existingIndex;
        } else {
            m_pdnGameHistory.append(fileName);
            m_currentPdnGameIndex = m_pdnGameHistory.size() - 1;
        }
        setStatusBarText(QString("Loaded game from %1.").arg(fileName));
    } else {
        setStatusBarText("Game loading cancelled.");
    }
}
void MainWindow::gameSave() {
    GameManager::log(LogLevel::Info, "Game Save action triggered.");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save PDN Game"),
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/game.pdn",
                                                   tr("PDN Files (*.pdn);;All Files (*)"));
    if (!fileName.isEmpty()) {
        m_gameManager->savePdnGame(fileName);
        setStatusBarText(QString("Saved game to %1.").arg(fileName));
    } else {
        setStatusBarText("Game saving cancelled.");
    }
}
void MainWindow::gameAnalyze() {
    GameManager::log(LogLevel::Info, "Game Analyze action triggered.");
    if (!m_isAnalyzing) {
        // Start analysis
        m_ai->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
        m_isAnalyzing = true;
        setStatusBarText("Engine analysis started.");
    } else {
        // Stop analysis
        m_ai->abortSearch();
        m_isAnalyzing = false;
        setStatusBarText("Engine analysis stopped.");
    }
}



void MainWindow::gameCopy() {
    GameManager::log(LogLevel::Info, "Game Copy action triggered.");
    QString fen = m_gameManager->getFenPosition();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fen);
    setStatusBarText("Current FEN copied to clipboard.");
}
void MainWindow::gamePaste() {
    GameManager::log(LogLevel::Info, "Game Paste action triggered.");
    QClipboard *clipboard = QApplication::clipboard();
    QString fen = clipboard->text();
    if (!fen.isEmpty()) {
        m_gameManager->loadFenPosition(fen);
        setStatusBarText("FEN position loaded from clipboard.");
    } else {
        setStatusBarText("Clipboard is empty or does not contain valid FEN.");
    }
}
void MainWindow::gameDatabase() {
    GameManager::log(LogLevel::Info, "Game Database action triggered.");
    GameDatabaseDialog dialog(m_gameManager, this);
    dialog.exec();
}
void MainWindow::gameFind() {
    GameManager::log(LogLevel::Info, "Game Find action triggered.");
    FindPositionDialog dialog(this);
    dialog.exec();
}
void MainWindow::gameFindCR() {
    GameManager::log(LogLevel::Info, "Game Find CR action triggered.");
    FindCRDialog dialog(this);
    dialog.exec();
}
void MainWindow::gameFindTheme() {
    GameManager::log(LogLevel::Info, "Game Find Theme action triggered.");
    bool ok;
    QString themeName = QInputDialog::getText(this, tr("Find Theme"),
                                            tr("Theme Name:"), QLineEdit::Normal,
                                            "", &ok);
    if (ok && !themeName.isEmpty()) {
        QMessageBox::information(this, tr("Find Theme"),
                                 tr("Searching for theme '%1'. Theme management is not yet fully implemented. "
                                    "This feature will allow users to search for and apply different visual themes for the checkerboard and pieces.").arg(themeName));
        setStatusBarText(QString("Searching for theme: %1.").arg(themeName));
    } else {
        setStatusBarText("Theme search cancelled.");
    }
}
void MainWindow::gameSaveAsHtml() {
    GameManager::log(LogLevel::Info, "Game Save As HTML action triggered.");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Game as HTML"),
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/game.html",
                                                   tr("HTML Files (*.html);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, tr("Save Game as HTML"),
                                 tr("Game saved as HTML to '%1' (functionality not yet fully implemented). "
                                    "This feature will generate an HTML file containing the game notation and possibly an interactive board.").arg(fileName));
        setStatusBarText(QString("Saving game as HTML to %1.").arg(fileName));
    } else {
        setStatusBarText("Saving game as HTML cancelled.");
    }
}
void MainWindow::gameDiagram() {
    GameManager::log(LogLevel::Info, "Game Diagram action triggered.");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Game Diagram"),
                                                   QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/game_diagram.png",
                                                   tr("PNG Images (*.png);;JPG Images (*.jpg);;BMP Images (*.bmp);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QPixmap pixmap = m_boardWidget->grab(); // Capture the BoardWidget as a QPixmap
        if (pixmap.save(fileName)) {
            setStatusBarText(QString("Game diagram saved to %1.").arg(fileName));
        } else {
            setStatusBarText(QString("Failed to save game diagram to %1.").arg(fileName));
        }
    } else {
        setStatusBarText("Saving game diagram cancelled.");
    }
}
void MainWindow::gameFindPlayer() {
    GameManager::log(LogLevel::Info, "Game Find Player action triggered.");
    bool ok;
    QString playerName = QInputDialog::getText(this, tr("Find Player"),
                                             tr("Player Name:"), QLineEdit::Normal,
                                             "", &ok);
    if (ok && !playerName.isEmpty()) {
        QMessageBox::information(this, tr("Find Player"),
                                 tr("Searching for player '%1'. Player search functionality is not yet fully implemented. "
                                    "This feature will allow searching through loaded PDN games for specific player names.").arg(playerName));
        setStatusBarText(QString("Searching for player: %1.").arg(playerName));
    } else {
        setStatusBarText("Player search cancelled.");
    }
}
void MainWindow::gameFenToClipboard() {
    GameManager::log(LogLevel::Info, "Game FEN to Clipboard action triggered.");
    QString fen = m_gameManager->getFenPosition();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fen);
    setStatusBarText("Current FEN copied to clipboard.");
}
void MainWindow::gameFenFromClipboard() {
    GameManager::log(LogLevel::Info, "Game FEN from Clipboard action triggered.");
    QClipboard *clipboard = QApplication::clipboard();
    QString fen = clipboard->text();
    if (!fen.isEmpty()) {
        m_gameManager->loadFenPosition(fen);
        setStatusBarText("FEN position loaded from clipboard.");
    } else {
        setStatusBarText("Clipboard is empty or does not contain valid FEN.");
    }
}
void MainWindow::gameSelectUserBook() {
    GameManager::log(LogLevel::Info, "Game Select User Book action triggered.");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select User Book"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("User Book Files (*.ubk);;All Files (*)"));
    if (!fileName.isEmpty()) {
        m_options.book_path = fileName;
        m_gameManager->setOptions(m_options);
        setStatusBarText(QString("User book selected: %1").arg(fileName));
    } else {
        setStatusBarText("User book selection cancelled.");
    }
}
void MainWindow::gameReSearch() {
    GameManager::log(LogLevel::Info, "Game Re-Search action triggered.");
    m_ai->requestMove(m_gameManager->getCurrentBoard(),
                      m_gameManager->getCurrentPlayer(),
                      m_gameManager->getOptions().time_per_move);
    setStatusBarText("Engine re-search initiated.");
}
void MainWindow::gameLoadNext() {
    GameManager::log(LogLevel::Info, "Game Load Next action triggered.");
    if (m_currentPdnGameIndex != -1 && m_currentPdnGameIndex < m_pdnGameHistory.size() - 1) {
        m_currentPdnGameIndex++;
        QString fileName = m_pdnGameHistory.at(m_currentPdnGameIndex);
        m_gameManager->loadPdnGame(fileName);
        setStatusBarText(QString("Loaded next game from %1.").arg(fileName));
    } else {
        setStatusBarText("No next game to load.");
    }
}
void MainWindow::gameLoadPrevious() {
    GameManager::log(LogLevel::Info, "Game Load Previous action triggered.");
    if (m_currentPdnGameIndex > 0) {
        m_currentPdnGameIndex--;
        QString fileName = m_pdnGameHistory.at(m_currentPdnGameIndex);
        m_gameManager->loadPdnGame(fileName);
        setStatusBarText(QString("Loaded previous game from %1.").arg(fileName));
    } else {
        setStatusBarText("No previous game to load.");
    }
}
void MainWindow::gameAnalyzePdn() {
    GameManager::log(LogLevel::Info, "Game Analyze PDN action triggered.");
    // Set AI parameters for PDN analysis mode
    m_ai->startAnalyzePdn(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer()); // Start the AI PDN analysis
    setStatusBarText("Game Analyze PDN: Analysis started.");
}

void MainWindow::gameInfo() {
    GameManager::log(LogLevel::Info, "Game Info action triggered.");
    QString info = QString("Current Game Type: %1\nTotal Moves: %2").arg(m_gameManager->getGameType()).arg(m_gameManager->getTotalMoves());
    QMessageBox::information(this, tr("Game Information"), info);
}
void MainWindow::gameSampleDiagram() {
    GameManager::log(LogLevel::Info, "Game Sample Diagram action triggered.");
    gameDiagram(); // Re-use the gameDiagram functionality
    setStatusBarText("Sample diagram saved (if location selected).");
}
void MainWindow::movesPlay() {
    GameManager::log(LogLevel::Info, "Moves Play action triggered.");
    m_gameManager->playMove();
    setStatusBarText("Moves Play action triggered.");
}
void MainWindow::movesBack() {
    GameManager::log(LogLevel::Info, "Moves Back action triggered.");
    m_gameManager->goBack();
    setStatusBarText("Moves Back action triggered.");
}
void MainWindow::movesForward() {
    GameManager::log(LogLevel::Info, "Moves Forward action triggered.");
    m_gameManager->goForward();
    setStatusBarText("Moves Forward action triggered.");
}
void MainWindow::movesBackAll() {
    GameManager::log(LogLevel::Info, "Moves Back All action triggered.");
    m_gameManager->goBackAll();
    setStatusBarText("Moves Back All action triggered.");
}
void MainWindow::movesForwardAll() {
    GameManager::log(LogLevel::Info, "Moves Forward All action triggered.");
    m_gameManager->goForwardAll();
    setStatusBarText("Moves Forward All action triggered.");
}
void MainWindow::movesComment() {
    GameManager::log(LogLevel::Info, "Moves Comment action triggered.");
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Comment"),
                                         tr("Comment:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        m_gameManager->addComment(text);
        setStatusBarText("Comment added.");
    } else {
        setStatusBarText("Add Comment cancelled.");
    }
}
void MainWindow::interruptEngine() { m_ai->requestAbort(); }
void MainWindow::abortEngine() { m_ai->requestAbort(); }
void MainWindow::levelExact() {
    GameManager::log(LogLevel::Info, "Level Exact action triggered.");
    m_gameManager->setTimeContol(LEVEL_INSTANT, true, false, 0, 0); // Example values
    setStatusBarText("Time control set to Exact.");
}
void MainWindow::levelInstant() {
    GameManager::log(LogLevel::Info, "Level Instant action triggered.");
    m_gameManager->setTimeContol(LEVEL_INSTANT, false, false, 0, 0); // Example values
    setStatusBarText("Time control set to Instant.");
}
void MainWindow::level01S() { 
    GameManager::log(LogLevel::Info, "Level 0.1S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_01S, false, false, 0.1, 0); 
    setStatusBarText("Time control set to 0.1 second."); 
}
void MainWindow::level02S() { 
    GameManager::log(LogLevel::Info, "Level 0.2S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_02S, false, false, 0.2, 0); 
    setStatusBarText("Time control set to 0.2 second."); 
}
void MainWindow::level05S() { 
    GameManager::log(LogLevel::Info, "Level 0.5S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_05S, false, false, 0.5, 0); 
    setStatusBarText("Time control set to 0.5 second."); 
}
void MainWindow::level1S() {
    GameManager::log(LogLevel::Info, "Level 1S action triggered.");
    m_gameManager->setTimeContol(LEVEL_1S, false, false, 1, 0);
    setStatusBarText("Time control set to 1 second.");
}
void MainWindow::level2S() { 
    GameManager::log(LogLevel::Info, "Level 2S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_2S, false, false, 2, 0); 
    setStatusBarText("Time control set to 2 seconds."); 
}
void MainWindow::level5S() { 
    GameManager::log(LogLevel::Info, "Level 5S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_5S, false, false, 5, 0); 
    setStatusBarText("Time control set to 5 seconds."); 
}
void MainWindow::level10S() { 
    GameManager::log(LogLevel::Info, "Level 10S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_10S, false, false, 10, 0); 
    setStatusBarText("Time control set to 10 seconds."); 
}
void MainWindow::level15S() { 
    GameManager::log(LogLevel::Info, "Level 15S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_15S, false, false, 15, 0); 
    setStatusBarText("Time control set to 15 seconds."); 
}
void MainWindow::level30S() { 
    GameManager::log(LogLevel::Info, "Level 30S action triggered."); 
    m_gameManager->setTimeContol(LEVEL_30S, false, false, 30, 0); 
    setStatusBarText("Time control set to 30 seconds."); 
}
void MainWindow::level1M() { 
    GameManager::log(LogLevel::Info, "Level 1M action triggered."); 
    m_gameManager->setTimeContol(LEVEL_1M, false, false, 60, 0); 
    setStatusBarText("Time control set to 1 minute."); 
}
void MainWindow::level2M() { 
    GameManager::log(LogLevel::Info, "Level 2M action triggered."); 
    m_gameManager->setTimeContol(LEVEL_2M, false, false, 120, 0); 
    setStatusBarText("Time control set to 2 minutes."); 
}
void MainWindow::level5M() { 
    GameManager::log(LogLevel::Info, "Level 5M action triggered."); 
    m_gameManager->setTimeContol(LEVEL_5M, false, false, 300, 0); 
    setStatusBarText("Time control set to 5 minutes."); 
}
void MainWindow::level15M() { 
    GameManager::log(LogLevel::Info, "Level 15M action triggered."); 
    m_gameManager->setTimeContol(LEVEL_15M, false, false, 900, 0); 
    setStatusBarText("Time control set to 15 minutes."); 
}
void MainWindow::level30M() {
    GameManager::log(LogLevel::Info, "Level 30M action triggered.");
    m_gameManager->setTimeContol(LEVEL_30M, false, false, 1800, 0);
    setStatusBarText("Time control set to 30 minutes.");
}
void MainWindow::levelInfinite() {
    GameManager::log(LogLevel::Info, "Level Infinite action triggered.");
    m_gameManager->setTimeContol(LEVEL_INFINITE, false, false, 0, 0); // Example values
    setStatusBarText("Time control set to Infinite.");
}

void MainWindow::engineInfinite() {
    GameManager::log(LogLevel::Info, "Engine Infinite action triggered.");
    QString reply;
    if (!m_isInfiniteAnalyzing) {
        if (m_ai->sendCommand("infinite on", reply)) {
            m_isInfiniteAnalyzing = true;
            setStatusBarText(QString("Engine infinite analysis enabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to enable engine infinite analysis.");
        }
    } else {
        if (m_ai->sendCommand("infinite off", reply)) {
            m_isInfiniteAnalyzing = false;
            setStatusBarText(QString("Engine infinite analysis disabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to disable engine infinite analysis.");
        }
    }
}
void MainWindow::levelIncrement() {
    GameManager::log(LogLevel::Info, "Level Increment action triggered.");
    bool ok1;
    int initialTime = QInputDialog::getInt(this, tr("Set Incremental Time: Initial Time"),
                                           tr("Initial Time (seconds):"), 600, 0, 36000, 1, &ok1);
    if (!ok1) { setStatusBarText("Incremental time setting cancelled."); return; }

    bool ok2;
    int incrementTime = QInputDialog::getInt(this, tr("Set Incremental Time: Increment"),
                                            tr("Increment (seconds):"), 5, 0, 600, 1, &ok2);
    if (!ok2) { setStatusBarText("Incremental time setting cancelled."); return; }

    m_options.initial_time = initialTime; // Update options
    m_options.time_increment = incrementTime; // Update options

    m_gameManager->setTimeContol(LEVEL_INFINITE, false, true, initialTime, incrementTime); // Assuming LEVEL_INFINITE for incremental
    setStatusBarText(QString("Incremental time control set: Initial %1s, Increment %2s.").arg(initialTime).arg(incrementTime));
}
void MainWindow::levelAddTime() {
    GameManager::log(LogLevel::Info, "Level Add Time action triggered.");
    bool ok;
    int timeToAdd = QInputDialog::getInt(this, tr("Add Time"),
                                         tr("Time to add (seconds):"), 60, 0, 3600, 1, &ok);
    if (ok) {
        // Placeholder: GameManager needs a method to add time
        m_gameManager->addTimeToClock(timeToAdd);
        setStatusBarText(QString("Added %1 seconds to clock (functionality not fully implemented).").arg(timeToAdd));
    } else {
        setStatusBarText("Add time cancelled.");
    }
}void MainWindow::levelSubtractTime() {
    GameManager::log(LogLevel::Info, "Level Subtract Time action triggered.");
    bool ok;
    int timeToSubtract = QInputDialog::getInt(this, tr("Subtract Time"),
                                              tr("Time to subtract (seconds):"), 60, 0, 3600, 1, &ok);
    if (ok) {
        // Placeholder: GameManager needs a method to subtract time
        m_gameManager->subtractFromClock(timeToSubtract);
        setStatusBarText(QString("Subtracted %1 seconds from clock (functionality not fully implemented).").arg(timeToSubtract));
    } else {
        setStatusBarText("Subtract time cancelled.");
    }
}
void MainWindow::pieceSet() {
    GameManager::log(LogLevel::Info, "Piece Set action triggered.");
    PieceSetDialog dialog(m_options.piece_set, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.piece_set = dialog.getSelectedPieceSet();
        m_gameManager->setOptions(m_options); // Update GameManager with new options
        m_boardWidget->setPieceSet(m_options.piece_set); // Update BoardWidget with new piece set
        setStatusBarText(QString("Piece set changed to %1.").arg(m_options.piece_set));
    } else {
        setStatusBarText("Piece set selection cancelled.");
    }
}
void MainWindow::optionsHighlight() {
    GameManager::log(LogLevel::Info, "Options Highlight action triggered.");
    m_options.highlight = !m_options.highlight;
    m_boardWidget->setHighlight(m_options.highlight);
    m_optionsHighlightAction->setChecked(m_options.highlight);
    setStatusBarText(QString("Highlighting %1.").arg(m_options.highlight ? "enabled" : "disabled"));
    m_gameManager->setOptions(m_options); // Update GameManager with new options
}
void MainWindow::optionsSound() {
    GameManager::log(LogLevel::Info, "Options Sound action triggered.");
    m_options.sound = !m_options.sound;
    setStatusBarText(QString("Sound %1.").arg(m_options.sound ? "enabled" : "disabled"));
    m_gameManager->setOptions(m_options); // Update GameManager with new options
}
void MainWindow::optionsPriority() {
    GameManager::log(LogLevel::Info, "Options Priority action triggered.");
    PriorityDialog dialog(m_options.priority, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.priority = dialog.getSelectedPriority();
        m_gameManager->setOptions(m_options); // Update GameManager with new options

        setStatusBarText(QString("Engine priority set to %1.").arg(m_options.priority));
    } else {
        setStatusBarText("Priority selection cancelled.");
    }
}
void MainWindow::options3Move() {
    GameManager::log(LogLevel::Info, "Options 3-Move action triggered.");
    ThreeMoveOptionsDialog dialog(m_options.three_move_option, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.three_move_option = dialog.getSelectedThreeMoveOption();
        m_gameManager->setOptions(m_options); // Update GameManager with new options
        setStatusBarText(QString("3-Move option set to %1.").arg(m_options.three_move_option));
    } else {
        setStatusBarText("3-Move option selection cancelled.");
    }
}
void MainWindow::optionsDirectories() {
    GameManager::log(LogLevel::Info, "Options Directories action triggered.");
    DirectoriesDialog dialog(QString(m_options.userdirectory), QString(m_options.matchdirectory), QString(m_options.EGTBdirectory), this);
    if (dialog.exec() == QDialog::Accepted) {
        strncpy(m_options.userdirectory, dialog.getUserDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.userdirectory[MAX_PATH_FIXED - 1] = '\0';
        strncpy(m_options.matchdirectory, dialog.getMatchDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.matchdirectory[MAX_PATH_FIXED - 1] = '\0';
        strncpy(m_options.EGTBdirectory, dialog.getEGTBDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.EGTBdirectory[MAX_PATH_FIXED - 1] = '\0';
        m_gameManager->setOptions(m_options); // Update GameManager with new options
        setStatusBarText("Directories configured.");
    } else {
        setStatusBarText("Directory configuration cancelled.");
    }
}
void MainWindow::optionsUserBook() {
    GameManager::log(LogLevel::Info, "Options User Book action triggered.");
    UserBookDialog dialog(m_options.book_path, this);

    connect(&dialog, &UserBookDialog::loadUserBookRequested, m_ai, &GeminiAI::loadUserBook);
    connect(&dialog, &UserBookDialog::saveUserBookRequested, m_ai, &GeminiAI::saveUserBook);
    connect(&dialog, &UserBookDialog::addMoveToUserBookRequested, this, [this]() {
        m_ai->addMoveToUserBook(m_gameManager->getCurrentBoard(), m_gameManager->getLastMove());
    });
    connect(&dialog, &UserBookDialog::deleteCurrentEntryRequested, m_ai, &GeminiAI::deleteCurrentEntry);
    connect(&dialog, &UserBookDialog::navigateToNextEntryRequested, m_ai, &GeminiAI::navigateToNextEntry);
    connect(&dialog, &UserBookDialog::navigateToPreviousEntryRequested, m_ai, &GeminiAI::navigateToPreviousEntry);
    connect(&dialog, &UserBookDialog::resetNavigationRequested, m_ai, &GeminiAI::resetNavigation);

    if (dialog.exec() == QDialog::Accepted) {
        // If a new book was loaded/saved, update the path in options
        if (!dialog.getSelectedUserBookPath().isEmpty()) {
            m_options.book_path = dialog.getSelectedUserBookPath();
            m_gameManager->setOptions(m_options); // Update GameManager with new options
            setStatusBarText(QString("User book path updated to %1.").arg(m_options.book_path));
        }
        setStatusBarText("User book options applied.");
    } else {
        setStatusBarText("User book options cancelled.");
    }
}
void MainWindow::optionsLanguageEnglish() {
    GameManager::log(LogLevel::Info, "Options Language English action triggered.");
    m_options.language = LANG_ENGLISH;
    setStatusBarText("Language set to English.");
    m_gameManager->setOptions(m_options);
}

void MainWindow::optionsLanguageEspanol() {
    GameManager::log(LogLevel::Info, "Options Language Espanol action triggered.");
    m_options.language = LANG_ESPANOL;
    setStatusBarText("Language set to Español.");
    m_gameManager->setOptions(m_options);
}

void MainWindow::optionsLanguageItaliano() {
    GameManager::log(LogLevel::Info, "Options Language Italiano action triggered.");
    m_options.language = LANG_ITALIANO;
    setStatusBarText("Language set to Italiano.");
    m_gameManager->setOptions(m_options);
}

void MainWindow::optionsLanguageDeutsch() {
    GameManager::log(LogLevel::Info, "Options Language Deutsch action triggered.");
    m_options.language = LANG_DEUTSCH;
    setStatusBarText("Language set to Deutsch.");
    m_gameManager->setOptions(m_options);
}

void MainWindow::optionsLanguageFrancais() {
    GameManager::log(LogLevel::Info, "Options Language Francais action triggered.");
    m_options.language = LANG_FRANCAIS;
    setStatusBarText("Language set to Français.");
    m_gameManager->setOptions(m_options);
}
void MainWindow::displayInvert() {
    GameManager::log(LogLevel::Info, "Display Invert action triggered.");
    m_options.invert_board = !m_options.invert_board;
    setStatusBarText(QString("Board inversion %1.").arg(m_options.invert_board ? "enabled" : "disabled"));
    m_gameManager->setOptions(m_options); // Update GameManager with new options
    m_boardWidget->setInverted(m_options.invert_board); // Update BoardWidget
}
void MainWindow::displayNumbers() {
    GameManager::log(LogLevel::Info, "Display Numbers action triggered.");
    m_options.show_coordinates = !m_options.show_coordinates;
    m_boardWidget->setShowCoordinates(m_options.show_coordinates);
    m_displayNumbersAction->setChecked(m_options.show_coordinates);
    setStatusBarText(QString("Show coordinates set to %1.").arg(m_options.show_coordinates ? "on" : "off"));
}
void MainWindow::displayMirror() {
    GameManager::log(LogLevel::Info, "Display Mirror action triggered.");
    m_options.mirror = !m_options.mirror;
    m_boardWidget->setMirror(m_options.mirror);
    m_displayMirrorAction->setChecked(m_options.mirror);
    setStatusBarText(QString("Board mirroring set to %1.").arg(m_options.mirror ? "on" : "off"));
}
void MainWindow::cmNormal() {
    GameManager::log(LogLevel::Info, "cmNormal action triggered.");
    changeAppState(STATE_NORMAL);
    setStatusBarText("Mode set to Normal.");
}
void MainWindow::cmAnalysis() {
    GameManager::log(LogLevel::Info, "cmAnalysis action triggered.");
    changeAppState(STATE_ANALYZEGAME);
    setStatusBarText("Mode set to Analysis.");
}
void MainWindow::gotoNormal() {
    GameManager::log(LogLevel::Info, "gotoNormal action triggered.");
    changeAppState(STATE_NORMAL);
    setStatusBarText("Mode set to Normal.");
}
void MainWindow::cmAutoplay() {
    GameManager::log(LogLevel::Info, "cmAutoplay action triggered.");
    changeAppState(STATE_AUTOPLAY);
    m_ai->startAutoplay(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
    setStatusBarText("Mode set to Autoplay.");
}
void MainWindow::cm2Player() {
    GameManager::log(LogLevel::Info, "cm2Player action triggered.");
    changeAppState(STATE_2PLAYER);
    setStatusBarText("Mode set to 2-Player.");
}
void MainWindow::engineVsEngine() {
    GameManager::log(LogLevel::Info, "engineVsEngine action triggered.");
    changeAppState(STATE_ENGINE_MATCH);
    setStatusBarText("Mode set to Engine vs Engine.");
}
void MainWindow::colorBoardNumbers() {
    GameManager::log(LogLevel::Info, "Color Board Numbers action triggered.");
    QColor initialColor = m_boardWidget->getCoordinateColor(); // Assuming BoardWidget has a getter for this
    QColor color = QColorDialog::getColor(initialColor, this, tr("Select Board Number Color"));
    if (color.isValid()) {
        // m_options.board_number_color = color; // Assuming CBoptions has a QColor member
        // m_gameManager->setOptions(m_options);
        m_boardWidget->setCoordinateColor(color); // Assuming BoardWidget has a setter for this
        setStatusBarText("Board number color updated.");
    } else {
        setStatusBarText("Board number color selection cancelled.");
    }
}
void MainWindow::colorHighlight() {
    GameManager::log(LogLevel::Info, "Color Highlight action triggered.");
    QColor initialColor = m_boardWidget->getHighlightColor(); // Assuming BoardWidget has a getter for this
    QColor color = QColorDialog::getColor(initialColor, this, tr("Select Highlight Color"));
    if (color.isValid()) {
        // m_options.highlight_color = color; // Assuming CBoptions has a QColor member
        // m_gameManager->setOptions(m_options);
        m_boardWidget->setHighlightColor(color); // Assuming BoardWidget has a setter for this
        setStatusBarText("Highlight color updated.");
    } else {
        setStatusBarText("Highlight color selection cancelled.");
    }
}
void MainWindow::bookModeView() {
    GameManager::log(LogLevel::Info, "Book Mode View action triggered.");
    UserBookDialog dialog(m_options.book_path, true, this); // true for readOnly

    // Connect dialog signals to AI slots (even in read-only mode, navigation might be useful)
    connect(&dialog, &UserBookDialog::loadUserBookRequested, m_ai, &GeminiAI::loadUserBook);
    connect(&dialog, &UserBookDialog::saveUserBookRequested, m_ai, &GeminiAI::saveUserBook);
    connect(&dialog, &UserBookDialog::addMoveToUserBookRequested, this, [this]() {
        m_ai->addMoveToUserBook(m_gameManager->getCurrentBoard(), m_gameManager->getLastMove());
    });
    connect(&dialog, &UserBookDialog::deleteCurrentEntryRequested, m_ai, &GeminiAI::deleteCurrentEntry);
    connect(&dialog, &UserBookDialog::navigateToNextEntryRequested, m_ai, &GeminiAI::navigateToNextEntry);
    connect(&dialog, &UserBookDialog::navigateToPreviousEntryRequested, m_ai, &GeminiAI::navigateToPreviousEntry);
    connect(&dialog, &UserBookDialog::resetNavigationRequested, m_ai, &GeminiAI::resetNavigation);

    dialog.exec();
    setStatusBarText("User book view closed.");
}
void MainWindow::bookModeAdd() {
    GameManager::log(LogLevel::Info, "Book Mode Add action triggered.");
    m_ai->addMoveToUserBook(m_gameManager->getCurrentBoard(), m_gameManager->getLastMove());
    setStatusBarText("Current position and move added to user book.");
}
void MainWindow::bookModeDelete() {
    GameManager::log(LogLevel::Info, "Book Mode Delete action triggered.");
    m_ai->deleteCurrentEntry();
    setStatusBarText("Current entry deleted from user book.");
}
void MainWindow::engineSelect() {
    GameManager::log(LogLevel::Info, "Engine Select action triggered.");
    EngineSelectDialog dialog(m_options.engine_path, m_options.secondary_engine_path, m_options.current_engine, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.engine_path = dialog.primaryEnginePath();
        m_options.secondary_engine_path = dialog.secondaryEnginePath();
        m_options.current_engine = dialog.selectedEngineIndex();
        m_gameManager->setOptions(m_options);
        // Set external engine paths in AI
        emit setPrimaryEnginePath(m_options.engine_path);
        emit setSecondaryEnginePath(m_options.secondary_engine_path);
        setStatusBarText(QString("Engine selected: %1").arg(dialog.selectedEngineName()));
    } else {
        setStatusBarText("Engine selection cancelled.");
    }
}

void MainWindow::engineOptions() {
    GameManager::log(LogLevel::Info, "Engine Options action triggered.");
    EngineOptionsDialog dialog(m_options.engine_name, m_options.engine_options, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.engine_name = dialog.engineName();
        m_options.engine_options = dialog.engineOptions();
        m_gameManager->setOptions(m_options);
        setStatusBarText(QString("Engine options set for %1.").arg(m_options.engine_name));
    } else {
        setStatusBarText("Engine options cancelled.");
    }
}
void MainWindow::cmEngineMatch() {
    GameManager::log(LogLevel::Info, "Engine Match action triggered.");
    bool ok;
    int numGames = QInputDialog::getInt(this, tr("Engine Match"),
                                        tr("Number of games:"), 1, 1, 1000, 1, &ok);
    if (ok) {
        changeAppState(STATE_ENGINE_MATCH);
        // Start the AI engine match
        m_ai->startEngineMatch(numGames, m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
        setStatusBarText(QString("Engine match started for %1 games.").arg(numGames));
    } else {
        setStatusBarText("Engine match cancelled.");
    }
}
void MainWindow::cmAddComment() {
    GameManager::log(LogLevel::Info, "Add Comment action triggered.");
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Engine Comment"),
                                         tr("Comment:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        m_gameManager->addComment(text);
        setStatusBarText("Engine comment added.");
    } else {
        setStatusBarText("Add Engine Comment cancelled.");
    }
}
void MainWindow::engineEval() {
    GameManager::log(LogLevel::Info, "Engine Eval action triggered.");
    changeAppState(STATE_ANALYZEGAME); // Or a specific EVAL state if needed
    // Start the AI evaluation
    m_ai->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
    setStatusBarText("Engine evaluation started.");
}

void MainWindow::engineAnalyze() {
    GameManager::log(LogLevel::Info, "Engine Analyze action triggered.");
    if (!m_isAnalyzing) {
        changeAppState(STATE_ANALYZEGAME);
        m_ai->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
        m_isAnalyzing = true;
        setStatusBarText("Engine analysis started.");
    } else {
        m_ai->abortSearch();
        m_isAnalyzing = false;
        setStatusBarText("Engine analysis stopped.");
    }
}

void MainWindow::cmEngineCommand() {
    GameManager::log(LogLevel::Info, "Engine Command action triggered.");
    bool ok;
    QString command = QInputDialog::getText(this, tr("Send Engine Command"),
                                            tr("Command:"), QLineEdit::Normal,
                                            "", &ok);
    if (ok && !command.isEmpty()) {
        QString reply;
        if (m_ai->sendCommand(command, reply)) {
            setStatusBarText(QString("Engine command sent. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to send engine command.");
        }
    } else {
        setStatusBarText("Engine command cancelled.");
    }
}
void MainWindow::cmRunTestSet() {
    GameManager::log(LogLevel::Info, "Run Test Set action triggered.");
    changeAppState(STATE_RUNTESTSET);
    // Start the AI test set run
    m_ai->startRunTestSet(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
    setStatusBarText("Running test set.");
}
void MainWindow::engineAbout() {
    GameManager::log(LogLevel::Info, "Engine About action triggered.");
    QMessageBox::information(this, tr("About Engine"),
                             tr("This is a placeholder for information about the currently selected AI engine.\n\n"
                                "Future development will include displaying engine name, version, author, and capabilities."));
}
void MainWindow::helpOnlineUpgrade() {
    GameManager::log(LogLevel::Info, "Help Online Upgrade action triggered.");
    QDesktopServices::openUrl(QUrl("https://www.example.com/upgrade")); // Placeholder URL
    setStatusBarText("Opened online upgrade page.");
}

void MainWindow::engineResign() {
    GameManager::log(LogLevel::Info, "Engine Resign action triggered.");
    QString reply;
    if (m_ai->sendCommand("resign", reply)) {
        setStatusBarText(QString("Engine resign command sent. Reply: %1").arg(reply));
    } else {
        setStatusBarText("Failed to send engine resign command.");
    }
}

void MainWindow::engineDraw() {
    GameManager::log(LogLevel::Info, "Engine Draw action triggered.");
    QString reply;
    if (m_ai->sendCommand("draw", reply)) {
        setStatusBarText(QString("Engine draw command sent. Reply: %1").arg(reply));
    } else {
        setStatusBarText("Failed to send engine draw command.");
    }
}

void MainWindow::cmHandicap() {
    GameManager::log(LogLevel::Info, "Handicap action triggered.");
    bool ok;
    int handicapValue = QInputDialog::getInt(this, tr("Set Engine Handicap"),
                                             tr("Handicap (moves to subtract from search depth):"),
                                             0, -MAX_DEPTH, MAX_DEPTH, 1, &ok);
    if (ok) {
        m_ai->setHandicap(handicapValue);
        setStatusBarText(QString("Engine handicap set to %1.").arg(handicapValue));
    } else {
        setStatusBarText("Engine handicap setting cancelled.");
    }
}
void MainWindow::setupMode() {
    GameManager::log(LogLevel::Info, "Setup Mode action triggered.");
    changeAppState(STATE_SETUP);
    setStatusBarText("Mode set to Setup.");
}
void MainWindow::setupClear() {
    GameManager::log(LogLevel::Info, "Setup Clear action triggered.");
    m_gameManager->clearBoard();
    setStatusBarText("Board cleared for setup.");
}

void MainWindow::setupBlack() {
    GameManager::log(LogLevel::Info, "Setup Black action triggered.");
    m_setupPieceType = CB_BLACK | CB_MAN;
    m_boardWidget->setSetupPieceType(m_setupPieceType);
    m_boardWidget->setTogglePieceColorMode(false);
    setStatusBarText("Click on a square to place a black man.");
}

void MainWindow::setupWhite() {
    GameManager::log(LogLevel::Info, "Setup White action triggered.");
    m_setupPieceType = CB_WHITE | CB_MAN;
    m_boardWidget->setSetupPieceType(m_setupPieceType);
    m_boardWidget->setTogglePieceColorMode(false);
    setStatusBarText("Click on a square to place a white man.");
}

void MainWindow::setupCc() {
    GameManager::log(LogLevel::Info, "Setup Change Color action triggered.");
    m_setupPieceType = CB_EMPTY; // Neutral value, as it's not for placing a specific piece
    m_boardWidget->setSetupPieceType(m_setupPieceType); // Inform BoardWidget about the piece type (or lack thereof)
    m_boardWidget->setTogglePieceColorMode(true);
    setStatusBarText("Click on a piece to change its color.");
}

void MainWindow::helpHelp() {
    GameManager::log(LogLevel::Info, "Help Help action triggered.");
    QString helpFilePath = QCoreApplication::applicationDirPath() + "/help.html";
    QDesktopServices::openUrl(QUrl::fromLocalFile(helpFilePath));
    setStatusBarText("Opened help documentation.");
}
void MainWindow::helpAboutQt() {
    GameManager::log(LogLevel::Info, "Help About Qt action triggered.");
    QMessageBox::aboutQt(this);
    setStatusBarText("Displayed About Qt dialog.");
}
void MainWindow::helpCheckersInANutshell() {
    GameManager::log(LogLevel::Info, "Help Checkers In A Nutshell action triggered.");
    QMessageBox::information(this, tr("Checkers In A Nutshell"),
                             tr("Checkers is a strategy board game played on an 8x8 checkered board.\n\n" 
                                "Rules:\n" 
                                "- Pieces move diagonally forward one square.\n" 
                                "- To capture, a piece jumps over an opponent's piece to an empty square beyond it.\n" 
                                "- Multiple captures are mandatory and must be completed in a single turn.\n" 
                                "- When a piece reaches the opponent's back row, it becomes a king.\n" 
                                "- Kings can move and capture diagonally forward and backward.\n" 
                                "- The game ends when a player has no legal moves or all their pieces are captured."));
}
void MainWindow::helpHomepage() {
    GameManager::log(LogLevel::Info, "Help Homepage action triggered.");
    QDesktopServices::openUrl(QUrl("https://www.example.com")); // Placeholder URL
    setStatusBarText("Opened project homepage.");
}
void MainWindow::helpProblemOfTheDay() {
    GameManager::log(LogLevel::Info, "Help Problem Of The Day action triggered.");
    QDesktopServices::openUrl(QUrl("https://www.example.com/problem_of_the_day")); // Placeholder URL
    setStatusBarText("Opened problem of the day.");
}

void MainWindow::engineHelp() {
    GameManager::log(LogLevel::Info, "Engine Help action triggered.");
    QString reply;
    if (m_ai->sendCommand("help", reply)) {
        QMessageBox::information(this, tr("Engine Help"), reply);
        setStatusBarText("Engine help displayed.");
    } else {
        setStatusBarText("Failed to get engine help.");
    }
}

void MainWindow::enginePonder() {
    GameManager::log(LogLevel::Info, "Engine Ponder action triggered.");
    QString reply;
    if (!m_isPondering) {
        if (m_ai->sendCommand("ponder on", reply)) {
            m_isPondering = true;
            setStatusBarText(QString("Engine pondering enabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to enable engine pondering.");
        }
    } else {
        if (m_ai->sendCommand("ponder off", reply)) {
            m_isPondering = false;
            setStatusBarText(QString("Engine pondering disabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to disable engine pondering.");
        }
    }
}

void MainWindow::helpContents() {
    GameManager::log(LogLevel::Info, "Help Contents action triggered.");
    QString helpFilePath = QCoreApplication::applicationDirPath() + "/help.html";
    QDesktopServices::openUrl(QUrl::fromLocalFile(helpFilePath));
    setStatusBarText("Opened help contents documentation.");
}

void MainWindow::engineUndoMove() {
    GameManager::log(LogLevel::Info, "Engine Undo Move action triggered.");
    QString reply;
    if (m_ai->sendCommand("undo", reply)) {
        setStatusBarText(QString("Engine undo command sent. Reply: %1").arg(reply));
    } else {
        setStatusBarText("Failed to send engine undo command.");
    }
}

void MainWindow::bookModeDeleteAll() {
    GameManager::log(LogLevel::Info, "Book Mode Delete All action triggered.");
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete All User Book Entries"),
                                  tr("Are you sure you want to delete ALL entries from the user book?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_ai->deleteAllEntriesFromUserBook();
        setStatusBarText("All entries deleted from user book.");
    } else {
        setStatusBarText("Deletion of all user book entries cancelled.");
    }
}

void MainWindow::setStatusBarText(const QString& text)
{
    statusBar()->showMessage(text);
    GameManager::log(LogLevel::Info, QString("Status bar text set to: %1").arg(text));
}

void MainWindow::updateEvaluationDisplay(int score)
{
    m_evaluationLabel->setText(QString(" Eval: %1 ").arg(score));
}
