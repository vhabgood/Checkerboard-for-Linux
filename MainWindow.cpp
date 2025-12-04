#include <QDebug>
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
#include "Dialogs.h" // Include for custom dialogs

#include "GeminiAI.h"

// Helper function



MainWindow::MainWindow(GameManager *gameManager, QWidget *parent) : m_gameManager(gameManager), QMainWindow(parent) {

    setWindowTitle("Checkerboard for Linux");

    resize(640, 700);



    m_boardWidget = new BoardWidget(this);

    setCentralWidget(m_boardWidget);



    qDebug() << "MainWindow initialized."; // Re-enable qInfo



    // Initialize AI here since m_egdbPath is needed from m_options

    // Load settings first to get m_options.EGTBdirectory

    loadSettings();

    // Force normal mode on startup, overriding saved settings
    m_options.white_player_type = PLAYER_AI;
    m_options.black_player_type = PLAYER_HUMAN;







    createMenus();

    createToolBars();



    m_evaluationLabel = new QLabel(this);

    m_depthLabel = new QLabel(this);

    statusBar()->addPermanentWidget(m_evaluationLabel);

    statusBar()->addPermanentWidget(m_depthLabel);



        connect(m_gameManager, &GameManager::boardUpdated, this, &MainWindow::handleBoardUpdated);



        connect(m_boardWidget, &BoardWidget::squareClicked, m_gameManager, &GameManager::handleSquareClick);



        connect(this, &MainWindow::setPrimaryEnginePath, m_gameManager->getAi(), &GeminiAI::setExternalEnginePath, Qt::QueuedConnection);



        connect(this, &MainWindow::setSecondaryEnginePath, m_gameManager->getAi(), &GeminiAI::setSecondaryExternalEnginePath, Qt::QueuedConnection);



        connect(this, &MainWindow::setEgdbPath, m_gameManager->getAi(), &GeminiAI::setEgdbPath, Qt::QueuedConnection); // Connect the new signal
        connect(m_gameManager, &GameManager::requestEngineSearch, m_gameManager->getAi(), &GeminiAI::requestMove, Qt::QueuedConnection); // Connect GameManager's request to AI's move request



        connect(this, &MainWindow::appStateChangeRequested, this, &MainWindow::onAppStateChangeRequested, Qt::QueuedConnection);
        connect(m_gameManager, &GameManager::requestClearSelectedPiece, m_boardWidget, &BoardWidget::clearSelectedPiece);

        QTimer::singleShot(0, this, &MainWindow::startGame);



    }

void MainWindow::startGame()
{
    m_gameManager->newGame(GT_ENGLISH);
}

MainWindow::~MainWindow()
{
}

// /**
//  * @brief Changes the application's current operational state and updates the UI accordingly.
//  *
//  * This function is central to managing the application's behavior, as different states
//  * (e.g., normal play, setup mode, analysis mode) require different UI elements to be
//  * enabled or disabled. It ensures that only relevant actions are available to the user
//  * in a given context.
//  *
//  * @param newState The `AppState` enum value representing the new state to transition to.
//  *
//  * @output
//  *   - Updates the `m_currentState` member to `newState`.
//  *   - Iterates through various `QAction` objects and `QMenu` objects, enabling or disabling
//  *     them based on the `newState`.
//  *   - Emits `qDebug()` messages to log state transitions.
//  */

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
            qWarning() << QString("Unknown AppState: %1. Mapping to Idle").arg(appState);
            return Idle;
    }
}

void MainWindow::changeAppState(AppState newState)
{
    emit appStateChangeRequested(newState);
}

void MainWindow::onAppStateChangeRequested(AppState newState)
{
    m_currentState = newState;
    m_gameManager->getAi()->setMode(mapAppStatetoAIState(newState));

    // Defer the UI update to a separate event loop cycle
    QTimer::singleShot(0, this, &MainWindow::updateUiForState);
}

void MainWindow::updateUiForState()
{
    // Use the now-stable m_currentState to update the UI
    AppState currentState = m_currentState;

    // Define state groups
    bool isNormal = (currentState == STATE_NORMAL);
    bool isEngineThinking = (currentState == STATE_ENGINE_THINKING);
    bool isSetup = (currentState == STATE_SETUP);
    bool isAnalysis = (currentState == STATE_ANALYZEGAME || currentState == STATE_ANALYZEPDN);
    bool isEngineMatch = (currentState == STATE_ENGINE_MATCH);
    bool isRunTestSet = (currentState == STATE_RUNTESTSET);
    bool isBookMode = (currentState == STATE_BOOKADD || currentState == STATE_BOOKDELETE || currentState == STATE_BOOKVIEW);
    bool is2Player = (currentState == STATE_2PLAYER);
    bool isAutoplay = (currentState == STATE_AUTOPLAY);

    bool engineIsActive = isEngineThinking || isAnalysis || isEngineMatch || isRunTestSet;
    bool gameInProgress = isNormal || isEngineThinking || isAnalysis || isEngineMatch || is2Player || isAutoplay;

    // File Menu Actions
    qDebug() << "Updating File Menu Actions...";
    m_newGameAction->setEnabled(isNormal || isSetup);
    m_game3MoveAction->setEnabled(isNormal);
    m_gameLoadAction->setEnabled(isNormal);
    m_gameSaveAction->setEnabled(isNormal || isAnalysis);

                    // Game Menu Actions

                    qDebug() << "Updating Game Menu Actions...";

                    // m_gameAnalyzeAction->setEnabled(isNormal); // This was commented out in previous step

                    m_gameCopyAction->setEnabled(gameInProgress || isSetup || isAnalysis);    m_gameCopyAction->setEnabled(gameInProgress || isSetup || isAnalysis);
    m_gamePasteAction->setEnabled(isNormal || isSetup);
    m_gameFenToClipboardAction->setEnabled(gameInProgress || isSetup || isAnalysis);
    m_gameFenFromClipboardAction->setEnabled(isNormal || isSetup);
    m_gameAnalyzePdnAction->setEnabled(isNormal);

    // Moves Menu Actions
    qDebug() << "Updating Moves Menu Actions...";
    m_movesPlayAction->setEnabled(isNormal);
    m_movesBackAction->setEnabled(gameInProgress || isAnalysis);
    m_movesForwardAction->setEnabled(gameInProgress || isAnalysis);
    m_movesBackAllAction->setEnabled(gameInProgress || isAnalysis);
    m_movesForwardAllAction->setEnabled(gameInProgress || isAnalysis);
    m_movesCommentAction->setEnabled(gameInProgress || isAnalysis);
    m_interruptEngineAction->setEnabled(engineIsActive);
    m_abortEngineAction->setEnabled(engineIsActive);

    // Options Menu Actions
    qDebug() << "Updating Options Menu Actions...";
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
    qDebug() << "Updating Engine Menu Actions...";
    m_engineSelectAction->setEnabled(isNormal);
    m_engineOptionsAction->setEnabled(isNormal);
    m_engineEvalAction->setEnabled(isNormal);
    m_cmEngineMatchAction->setEnabled(isNormal);
    m_cmAddCommentAction->setEnabled(gameInProgress || isAnalysis);
    m_cmEngineCommandAction->setEnabled(true);
    m_cmRunTestSetAction->setEnabled(true);
    
        // Ponder and other actions can be enabled/disabled based on engine capabilities
    
        // m_engineMenu->addAction(tr("Ponder"), this, &MainWindow::enginePonder);
    
        qDebug() << "Updating Setup Menu Actions...";
    
    }
    
    void MainWindow::helpAbout() {
    qInfo() << "Help About action triggered.";
    QMessageBox::about(this, tr("About Checkerboard"),
                       tr("Checkerboard for Linux\n\n" 
                          "Version 1.0\n\n" 
                          "A cross-platform checkers application.\n\n" 
                          "Based on the original Windows Checkerboard by Martin Fierz."));
}


void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    m_newGameAction = fileMenu->addAction(tr("&New Game"), this, &MainWindow::newGame);
    qDebug() << "m_newGameAction created: " << (m_newGameAction != nullptr);
    fileMenu->addAction(tr("E&xit"), this, &MainWindow::exitApplication);

    QMenu *gameMenu = menuBar()->addMenu(tr("&Game"));
    m_game3MoveAction = gameMenu->addAction(tr("&3-Move"), this, &MainWindow::game3Move);
    qDebug() << "m_game3MoveAction created: " << (m_game3MoveAction != nullptr);
    m_gameLoadAction = gameMenu->addAction(tr("&Load"), this, &MainWindow::gameLoad);
    qDebug() << "m_gameLoadAction created: " << (m_gameLoadAction != nullptr);
    m_gameSaveAction = gameMenu->addAction(tr("&Save"), this, &MainWindow::gameSave);
    qDebug() << "m_gameSaveAction created: " << (m_gameSaveAction != nullptr);
    gameMenu->addSeparator();
    m_gameAnalyzeAction = gameMenu->addAction(tr("&Analyze"), this, &MainWindow::gameAnalyze);
    qDebug() << "m_gameAnalyzeAction created: " << (m_gameAnalyzeAction != nullptr);
    m_gameCopyAction = gameMenu->addAction(tr("&Copy"), this, &MainWindow::gameCopy);
    qDebug() << "m_gameCopyAction created: " << (m_gameCopyAction != nullptr);
    m_gamePasteAction = gameMenu->addAction(tr("&Paste"), this, &MainWindow::gamePaste);
    qDebug() << "m_gamePasteAction created: " << (m_gamePasteAction != nullptr);
    m_gameFenToClipboardAction = gameMenu->addAction(tr("FEN to Clipboard"), this, &MainWindow::gameFenToClipboard);
    qDebug() << "m_gameFenToClipboardAction created: " << (m_gameFenToClipboardAction != nullptr);
    m_gameFenFromClipboardAction = gameMenu->addAction(tr("FEN from Clipboard"), this, &MainWindow::gameFenFromClipboard);
    qDebug() << "m_gameFenFromClipboardAction created: " << (m_gameFenFromClipboardAction != nullptr);
    m_gameAnalyzePdnAction = gameMenu->addAction(tr("&Analyze PDN"), this, &MainWindow::gameAnalyzePdn);
    qDebug() << "m_gameAnalyzePdnAction created: " << (m_gameAnalyzePdnAction != nullptr);

    QMenu *movesMenu = menuBar()->addMenu(tr("&Moves"));
    m_movesPlayAction = movesMenu->addAction(tr("&Play"), this, &MainWindow::movesPlay);
    qDebug() << "m_movesPlayAction created: " << (m_movesPlayAction != nullptr);
    m_movesBackAction = movesMenu->addAction(tr("&Back"), this, &MainWindow::movesBack);
    qDebug() << "m_movesBackAction created: " << (m_movesBackAction != nullptr);
    m_movesForwardAction = movesMenu->addAction(tr("&Forward"), this, &MainWindow::movesForward);
    qDebug() << "m_movesForwardAction created: " << (m_movesForwardAction != nullptr);
    m_movesBackAllAction = movesMenu->addAction(tr("Back &All"), this, &MainWindow::movesBackAll);
    qDebug() << "m_movesBackAllAction created: " << (m_movesBackAllAction != nullptr);
    m_movesForwardAllAction = movesMenu->addAction(tr("Forward A&ll"), this, &MainWindow::movesForwardAll);
    qDebug() << "m_movesForwardAllAction created: " << (m_movesForwardAllAction != nullptr);
    m_movesCommentAction = movesMenu->addAction(tr("&Comment"), this, &MainWindow::movesComment);
    qDebug() << "m_movesCommentAction created: " << (m_movesCommentAction != nullptr);
    movesMenu->addSeparator();
    m_interruptEngineAction = movesMenu->addAction(tr("&Interrupt Engine"), this, &MainWindow::interruptEngine);
    qDebug() << "m_interruptEngineAction created: " << (m_interruptEngineAction != nullptr);
    m_abortEngineAction = movesMenu->addAction(tr("A&bort Engine"), this, &MainWindow::abortEngine);
    qDebug() << "m_abortEngineAction created: " << (m_abortEngineAction != nullptr);

    QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
    m_optionsHighlightAction = optionsMenu->addAction(tr("&Highlight"), this, &MainWindow::optionsHighlight);
    qDebug() << "m_optionsHighlightAction created: " << (m_optionsHighlightAction != nullptr);
    m_optionsSoundAction = optionsMenu->addAction(tr("&Sound"), this, &MainWindow::optionsSound);
    qDebug() << "m_optionsSoundAction created: " << (m_optionsSoundAction != nullptr);
    QMenu *displayMenu = optionsMenu->addMenu(tr("&Display"));
    m_displayInvertAction = displayMenu->addAction(tr("&Invert Board"), this, &MainWindow::displayInvert);
    qDebug() << "m_displayInvertAction created: " << (m_displayInvertAction != nullptr);
    m_displayNumbersAction = displayMenu->addAction(tr("&Numbers"), this, &MainWindow::displayNumbers);
    qDebug() << "m_displayNumbersAction created: " << (m_displayNumbersAction != nullptr);
    m_displayMirrorAction = displayMenu->addAction(tr("&Mirror"), this, &MainWindow::displayMirror);
    qDebug() << "m_displayMirrorAction created: " << (m_displayMirrorAction != nullptr);
    QMenu *modeMenu = optionsMenu->addMenu(tr("&Mode"));
    m_cmNormalAction = modeMenu->addAction(tr("&Normal"), this, &MainWindow::cmNormal);
    qDebug() << "m_cmNormalAction created: " << (m_cmNormalAction != nullptr);
    m_cmAnalysisAction = modeMenu->addAction(tr("&Analysis"), this, &MainWindow::cmAnalysis);
    qDebug() << "m_cmAnalysisAction created: " << (m_cmAnalysisAction != nullptr);
    m_cmAutoplayAction = modeMenu->addAction(tr("A&utoplay"), this, &MainWindow::cmAutoplay);
    qDebug() << "m_cmAutoplayAction created: " << (m_cmAutoplayAction != nullptr);
    m_cm2PlayerAction = modeMenu->addAction(tr("&2 Player"), this, &MainWindow::cm2Player);
    qDebug() << "m_cm2PlayerAction created: " << (m_cm2PlayerAction != nullptr);
    m_engineVsEngineAction = modeMenu->addAction(tr("&Engine vs Engine"), this, &MainWindow::engineVsEngine);
    qDebug() << "m_engineVsEngineAction created: " << (m_engineVsEngineAction != nullptr);
    QMenu *bookMenu = optionsMenu->addMenu(tr("&Book"));
    m_bookModeViewAction = bookMenu->addAction(tr("&View"), this, &MainWindow::bookModeView);
    qDebug() << "m_bookModeViewAction created: " << (m_bookModeViewAction != nullptr);
    m_bookModeAddAction = bookMenu->addAction(tr("&Add"), this, &MainWindow::bookModeAdd);
    qDebug() << "m_bookModeAddAction created: " << (m_bookModeAddAction != nullptr);
    m_bookModeDeleteAction = bookMenu->addAction(tr("&Delete"), this, &MainWindow::bookModeDelete);
    qDebug() << "m_bookModeDeleteAction created: " << (m_bookModeDeleteAction != nullptr);
    m_bookModeDeleteAllAction = bookMenu->addAction(tr("Delete &All"), this, &MainWindow::bookModeDeleteAll);
    qDebug() << "m_bookModeDeleteAllAction created: " << (m_bookModeDeleteAllAction != nullptr);
    
    m_engineMenu = menuBar()->addMenu(tr("&Engine"));
    m_engineSelectAction = m_engineMenu->addAction(tr("&Select"), this, &MainWindow::engineSelect);
    qDebug() << "m_engineSelectAction created: " << (m_engineSelectAction != nullptr);
    m_engineOptionsAction = m_engineMenu->addAction(tr("&Options"), this, &MainWindow::engineOptions);
    qDebug() << "m_engineOptionsAction created: " << (m_engineOptionsAction != nullptr);
    m_engineEvalAction = m_engineMenu->addAction(tr("&Eval"), this, &MainWindow::engineEval);
    qDebug() << "m_engineEvalAction created: " << (m_engineEvalAction != nullptr);
    m_cmEngineMatchAction = m_engineMenu->addAction(tr("Eng&ine Match"), this, &MainWindow::cmEngineMatch);
    qDebug() << "m_cmEngineMatchAction created: " << (m_cmEngineMatchAction != nullptr);
    m_cmAddCommentAction = m_engineMenu->addAction(tr("Add &Comment"), this, &MainWindow::cmAddComment);
    qDebug() << "m_cmAddCommentAction created: " << (m_cmAddCommentAction != nullptr);
    m_cmEngineCommandAction = m_engineMenu->addAction(tr("Engine &Command"), this, &MainWindow::cmEngineCommand);
    qDebug() << "m_cmEngineCommandAction created: " << (m_cmEngineCommandAction != nullptr);
    m_cmRunTestSetAction = m_engineMenu->addAction(tr("&Run Test Set"), this, &MainWindow::cmRunTestSet);
    qDebug() << "m_cmRunTestSetAction created: " << (m_cmRunTestSetAction != nullptr);
    m_engineMenu->addAction(tr("&Handicap"), this, &MainWindow::cmHandicap);
    m_engineMenu->addSeparator();
    m_engineMenu->addAction(tr("&About"), this, &MainWindow::engineAbout);
    m_engineMenu->addAction(tr("&Help"), this, &MainWindow::engineHelp);
    m_engineMenu->addSeparator();
    m_engineMenu->addAction(tr("A&nalyze"), this, &MainWindow::engineAnalyze);
    m_engineMenu->addAction(tr("&Infinite"), this, &MainWindow::engineInfinite);
    m_engineMenu->addAction(tr("&Resign"), this, &MainWindow::engineResign);
    m_engineMenu->addAction(tr("&Draw"), this, &MainWindow::engineDraw);
    m_engineMenu->addSeparator();
    m_engineMenu->addAction(tr("&Undo Move"), this, &MainWindow::engineUndoMove);
    m_engineMenu->addAction(tr("&Ponder"), this, &MainWindow::enginePonder);
    
    QMenu *setupMenu = menuBar()->addMenu(tr("&Setup"));
    m_setupModeAction = setupMenu->addAction(tr("Setup &Mode"), this, &MainWindow::setupMode);
    qDebug() << "m_setupModeAction created: " << (m_setupModeAction != nullptr);
    setupMenu->addSeparator();
    m_setupClearAction = setupMenu->addAction(tr("&Clear"), this, &MainWindow::setupClear);
    qDebug() << "m_setupClearAction created: " << (m_setupClearAction != nullptr);
    m_setupBlackAction = setupMenu->addAction(tr("&Black"), this, &MainWindow::setupBlack);
    qDebug() << "m_setupBlackAction created: " << (m_setupBlackAction != nullptr);
    m_setupWhiteAction = setupMenu->addAction(tr("&White"), this, &MainWindow::setupWhite);
    qDebug() << "m_setupWhiteAction created: " << (m_setupWhiteAction != nullptr);
    m_setupCcAction = setupMenu->addAction(tr("&Change Color"), this, &MainWindow::setupCc);
    qDebug() << "m_setupCcAction created: " << (m_setupCcAction != nullptr);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("Help"), this, &MainWindow::helpHelp);
    helpMenu->addAction(tr("Checkers In A Nutshell"), this, &MainWindow::helpCheckersInANutshell);
    helpMenu->addAction(tr("Homepage"), this, &MainWindow::helpHomepage);
    helpMenu->addAction(tr("Problem Of The Day"), this, &MainWindow::helpProblemOfTheDay);
    helpMenu->addAction(tr("Online Upgrade"), this, &MainWindow::helpOnlineUpgrade);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("About"), this, &MainWindow::helpAbout);
    helpMenu->addAction(tr("About Qt"), this, &MainWindow::helpAboutQt);
    helpMenu->addAction(tr("Contents"), this, &MainWindow::helpContents);
}

void MainWindow::createToolBars()
{
    m_mainToolBar = addToolBar(tr("Main Toolbar"));
    m_mainToolBar->setObjectName("Main Toolbar");
    m_mainToolBar->addAction(tr("New Game"), this, &MainWindow::newGame);
    // Add other common actions
}


void MainWindow::loadSettings()
{
    qDebug() << "Loading settings.";
    QSettings settings("Checkerboard", "Checkerboard");

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
    m_options.initial_time = settings.value("Options/InitialTime", 120).toInt(); // Default to 2 minutes (120 seconds)
    m_options.time_increment = settings.value("Options/TimeIncrement", 0).toInt(); // Default to 0 increment
    m_options.enable_game_timer = false; // Force disable the game timer for now
    m_options.white_player_type = static_cast<PlayerType>(settings.value("Options/WhitePlayerType", PLAYER_AI).toInt());
    m_options.black_player_type = static_cast<PlayerType>(settings.value("Options/BlackPlayerType", PLAYER_HUMAN).toInt());

    qDebug() << QString("MainWindow: Settings loaded - White Player Type: %1, Black Player Type: %2")
                 .arg(m_options.white_player_type)
                 .arg(m_options.black_player_type);


    // Apply loaded settings to UI and other components
    strncpy(m_options.userdirectory, settings.value("Options/UserDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString().toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.userdirectory[MAX_PATH_FIXED - 1] = '\0';
    strncpy(m_options.matchdirectory, settings.value("Options/MatchDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString().toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.matchdirectory[MAX_PATH_FIXED - 1] = '\0';
    // Determine the preferred default EGTB directory (application's db folder)
    QString defaultEgdbPath = QCoreApplication::applicationDirPath() + "/db";

    // Load the stored EGTB directory from settings.
    QString storedEgdbPath = settings.value("Options/EGTBDirectory").toString();

    QString finalEgdbPath;

    // Check if the stored path is a valid directory containing the DB files
    if (!storedEgdbPath.isEmpty() && QDir(storedEgdbPath).exists() && QFile(storedEgdbPath + "/db2.idx").exists()) {
        finalEgdbPath = storedEgdbPath;
        qDebug() << "Using valid stored EGTB directory:" << finalEgdbPath;
    } else {
        // If stored path is invalid, fall back to the default path
        finalEgdbPath = defaultEgdbPath;
        qWarning() << "Stored EGTB directory was invalid or missing files. Falling back to default:" << finalEgdbPath;
        // Overwrite the bad setting with the correct default for next time
        settings.setValue("Options/EGTBDirectory", finalEgdbPath);
    }
    
    // Copy the chosen path into the C-style string struct member
    strncpy(m_options.EGTBdirectory, finalEgdbPath.toUtf8().constData(), MAX_PATH_FIXED - 1);
    m_options.EGTBdirectory[MAX_PATH_FIXED - 1] = '\0';
            // ... load other options as needed
    
        // Set external engine paths in AI
        emit setPrimaryEnginePath(m_options.engine_path);
        emit setSecondaryEnginePath(m_options.secondary_engine_path);
    
    qDebug() << "Settings loaded.";
}

void MainWindow::saveSettings()
{
    qInfo() << "Saving settings.";
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

    qInfo() << "Settings saved.";
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

// /**
//  * @brief Loads application settings from persistent storage (QSettings).
//  *
//  * This function is called during `MainWindow` initialization to restore user preferences
//  * and application state from the last session. It reads settings related to window
//  * geometry, window state, and various game options stored in the `m_options` struct.
//  *
//  * @input None.
//  *
//  * @output
//  *   - Restores `MainWindow`'s geometry and state using `restoreGeometry()` and `restoreState()`.
//  *   - Populates the `m_options` (CBoptions struct) with saved values for sound, display, time control,
//  *     directories, and engine paths. Default values are used if a setting is not found.
//  *   - Acquires a `QMutexLocker` to ensure thread-safe access to `m_options`.
//  */






void MainWindow::setStatusBarText(const QString& text)
{
    statusBar()->showMessage(text);
}

void MainWindow::updateEvaluationDisplay(int score, int depth)
{
    m_evaluationLabel->setText(QString("Eval: %1").arg(score / 100.0, 0, 'f', 2));
    m_depthLabel->setText(QString("Depth: %1").arg(depth));
}

void MainWindow::optionsHighlight()
{
    m_options.highlight = !m_options.highlight; // Toggle the highlight option
    m_boardWidget->setHighlight(m_options.highlight); // Apply to BoardWidget
    m_gameManager->setOptions(m_options); // Update GameManager
    m_optionsHighlightAction->setChecked(m_options.highlight); // Update menu action check state
    setStatusBarText(QString("Highlighting turned %1.").arg(m_options.highlight ? "on" : "off"));
}
void MainWindow::optionsSound()
{
    m_options.sound = !m_options.sound; // Toggle the sound option
    m_gameManager->setSoundEnabled(m_options.sound); // Update GameManager
    m_optionsSoundAction->setChecked(m_options.sound); // Update menu action check state
    setStatusBarText(QString("Sound turned %1.").arg(m_options.sound ? "on" : "off"));
    saveSettings(); // Save the updated setting
}

void MainWindow::displayInvert()
{
    m_options.invert_board = !m_options.invert_board; // Toggle the invert board option
    m_boardWidget->setInverted(m_options.invert_board); // Apply to BoardWidget
    m_gameManager->setOptions(m_options); // Update GameManager
    m_displayInvertAction->setChecked(m_options.invert_board); // Update menu action check state
    setStatusBarText(QString("Board inversion turned %1.").arg(m_options.invert_board ? "on" : "off"));
    saveSettings(); // Save the updated setting
}
void MainWindow::displayNumbers()
{
    m_options.show_coordinates = !m_options.show_coordinates; // Toggle the show coordinates option
    m_boardWidget->setShowCoordinates(m_options.show_coordinates); // Apply to BoardWidget
    m_gameManager->setOptions(m_options); // Update GameManager
    m_displayNumbersAction->setChecked(m_options.show_coordinates); // Update menu action check state
    setStatusBarText(QString("Board numbers turned %1.").arg(m_options.show_coordinates ? "on" : "off"));
    saveSettings(); // Save the updated setting
}

void MainWindow::displayMirror()
{
    m_options.mirror = !m_options.mirror; // Toggle the mirror option
    m_boardWidget->setMirror(m_options.mirror); // Apply to BoardWidget
    m_gameManager->setOptions(m_options); // Update GameManager
    m_displayMirrorAction->setChecked(m_options.mirror); // Update menu action check state
    setStatusBarText(QString("Board mirroring turned %1.").arg(m_options.mirror ? "on" : "off"));
    saveSettings(); // Save the updated setting
}
void MainWindow::cmNormal()
{
    m_options.white_player_type = PLAYER_AI;
    m_options.black_player_type = PLAYER_HUMAN;
    m_gameManager->setOptions(m_options);
    emit setAiOptions(m_options);
    changeAppState(STATE_NORMAL);
    setStatusBarText("Mode changed to Normal.");
    m_gameManager->resumePlay();
}
void MainWindow::cmAnalysis()
{
    changeAppState(STATE_ANALYSIS);
    setStatusBarText("Mode changed to Analysis.");
}
void MainWindow::cmAutoplay()
{
    m_options.white_player_type = PLAYER_AI;
    m_options.black_player_type = PLAYER_AI;
    m_gameManager->setOptions(m_options);
    emit setAiOptions(m_options);
    changeAppState(STATE_AUTOPLAY);
    setStatusBarText("Mode changed to Autoplay.");
    m_gameManager->resumePlay();
}
void MainWindow::cm2Player()
{
    m_options.white_player_type = PLAYER_HUMAN;
    m_options.black_player_type = PLAYER_HUMAN;
    m_gameManager->setOptions(m_options);
    emit setAiOptions(m_options);
    changeAppState(STATE_2PLAYER);
    setStatusBarText("Mode changed to 2-Player.");
    m_gameManager->resumePlay();
}
void MainWindow::engineVsEngine()
{
    m_options.white_player_type = PLAYER_AI;
    m_options.black_player_type = PLAYER_AI;
    m_gameManager->setOptions(m_options);
    emit setAiOptions(m_options);
    changeAppState(STATE_ENGINE_MATCH);
    setStatusBarText("Mode changed to Engine vs. Engine.");
    m_gameManager->resumePlay();
}

void MainWindow::bookModeView()
{
    UserBookDialog dialog(m_options.book_path, true, this); // 'true' for readOnly
    dialog.exec();
    setStatusBarText("User book viewed.");
}

void MainWindow::bookModeAdd()
{
    m_gameManager->getAi()->addMoveToUserBook(m_gameManager->getCurrentBoard(), m_gameManager->getLastMove());
    setStatusBarText("Current move added to user book.");
}
void MainWindow::bookModeDelete()
{
    m_gameManager->getAi()->deleteCurrentEntry();
    setStatusBarText("Current entry deleted from user book.");
}

void MainWindow::bookModeDeleteAll()
{
    if (QMessageBox::question(this, tr("Delete All User Book Entries"),
                              tr("Are you sure you want to delete ALL entries from the user book? This action cannot be undone."),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_gameManager->getAi()->deleteAllEntriesFromUserBook();
        setStatusBarText("All entries deleted from user book.");
    } else {
        setStatusBarText("Deletion of all user book entries cancelled.");
    }
}

void MainWindow::engineSelect()
{
    EngineSelectDialog dialog(m_options.engine_path, m_options.secondary_engine_path, m_options.current_engine, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.engine_path = dialog.primaryEnginePath();
        m_options.secondary_engine_path = dialog.secondaryEnginePath();
        m_options.current_engine = dialog.selectedEngineIndex();

        m_gameManager->setOptions(m_options);
        emit setPrimaryEnginePath(m_options.engine_path);
        emit setSecondaryEnginePath(m_options.secondary_engine_path);

        setStatusBarText(QString("Engine selection updated. Primary: %1, Secondary: %2, Current: %3")
                             .arg(m_options.engine_path)
                             .arg(m_options.secondary_engine_path)
                             .arg(m_options.current_engine));
        saveSettings();
    } else {
        setStatusBarText("Engine selection cancelled.");
    }
}
void MainWindow::engineAbout()
{
    QString reply;
    if (m_gameManager->getAi()->sendCommand("about", reply)) {
        QMessageBox::information(this, tr("About Engine"), reply);
        setStatusBarText("Engine information displayed.");
    } else {
        setStatusBarText("Could not get engine information.");
    }
}
void MainWindow::engineHelp()
{
    QString reply;
    if (m_gameManager->getAi()->sendCommand("help", reply)) {
        QMessageBox::information(this, tr("Engine Help"), reply);
        setStatusBarText("Engine help displayed.");
    } else {
        setStatusBarText("Could not get engine help.");
    }
}

void MainWindow::engineOptions()
{
    EngineOptionsDialog dialog(m_options.engine_name, m_options.engine_options, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.engine_options = dialog.getNewOptions();
        m_gameManager->setOptions(m_options);
        // Potentially inform AI about new options if needed
        setStatusBarText(QString("Engine options updated for %1.").arg(m_options.engine_name));
        saveSettings();
    } else {
        setStatusBarText("Engine options not changed.");
    }
}

void MainWindow::engineAnalyze()
{
    if (!m_isAnalyzing) {
        m_gameManager->getAi()->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
        m_isAnalyzing = true;
        setStatusBarText("Engine analysis started.");
    } else {
        m_gameManager->getAi()->abortSearch();
        m_isAnalyzing = false;
        setStatusBarText("Engine analysis stopped.");
    }
}

void MainWindow::engineResign()
{
    if (QMessageBox::question(this, tr("Resign Game"),
                              tr("Are you sure you want to resign the current game?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit m_gameManager->gameIsOver(CB_LOSS); // Current player resigns, so it's a loss
        setStatusBarText("Game resigned.");
    } else {
        setStatusBarText("Resignation cancelled.");
    }
}
void MainWindow::engineDraw()
{
    if (QMessageBox::question(this, tr("Offer/Accept Draw"),
                              tr("Do you want to offer or accept a draw?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit m_gameManager->gameIsOver(CB_DRAW);
        setStatusBarText("Draw declared.");
    } else {
        setStatusBarText("Draw offer/acceptance cancelled.");
    }
}
void MainWindow::engineUndoMove()
{
    m_gameManager->goBack();
    setStatusBarText("Last move undone.");
}
void MainWindow::enginePonder()
{
    QString reply;
    if (!m_isPondering) {
        if (m_gameManager->getAi()->sendCommand("ponder on", reply)) {
            m_isPondering = true;
            setStatusBarText(QString("Engine pondering enabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to enable engine pondering.");
        }
    } else {
        if (m_gameManager->getAi()->sendCommand("ponder off", reply)) {
            m_isPondering = false;
            setStatusBarText(QString("Engine pondering disabled. Reply: %1").arg(reply));
        } else {
            setStatusBarText("Failed to disable engine pondering.");
        }
    }
}
void MainWindow::cmEngineMatch()
{
    engineVsEngine(); // Delegate to existing function
    setStatusBarText("Engine vs. Engine match initiated.");
}
void MainWindow::cmAddComment()
{
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
void MainWindow::engineEval()
{
    if (!m_isAnalyzing) {
        m_gameManager->getAi()->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
        m_isAnalyzing = true;
        setStatusBarText("Engine evaluation started.");
    } else {
        setStatusBarText("Engine is already analyzing. Use Abort or Interrupt to stop.");
    }
}
void MainWindow::cmEngineCommand()
{
    bool ok;
    QString command = QInputDialog::getText(this, tr("Send Engine Command"),
                                            tr("Command:"), QLineEdit::Normal,
                                            "", &ok);
    if (ok && !command.isEmpty()) {
        QString reply;
        if (m_gameManager->getAi()->sendCommand(command, reply)) {
            QMessageBox::information(this, tr("Engine Reply"), reply);
            setStatusBarText("Engine command sent.");
        } else {
            setStatusBarText("Failed to send engine command.");
        }
    } else {
        setStatusBarText("Sending engine command cancelled.");
    }
}
void MainWindow::cmRunTestSet()
{
    m_gameManager->getAi()->startRunTestSet(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());
    setStatusBarText("Engine test set run initiated.");
}
void MainWindow::cmHandicap()
{
    bool ok;
    int handicap = QInputDialog::getInt(this, tr("Set Engine Handicap"),
                                        tr("Handicap (search depth reduction):"), 0, -20, 20, 1, &ok);
    if (ok) {
        m_gameManager->getAi()->setHandicap(handicap);
        setStatusBarText(QString("Engine handicap set to %1.").arg(handicap));
    } else {
        setStatusBarText("Setting engine handicap cancelled.");
    }
}

void MainWindow::setupMode()
{
    changeAppState(STATE_SETUP);
    setStatusBarText("Mode changed to Setup.");
}
void MainWindow::setupClear()
{
    m_gameManager->clearBoard();
    setStatusBarText("Board cleared.");
}
void MainWindow::setupBlack()
{
    m_setupPieceType = CB_BLACK | CB_MAN;
    m_togglePieceColorMode = false;
    m_boardWidget->setSetupPieceType(m_setupPieceType);
    m_boardWidget->setTogglePieceColorMode(m_togglePieceColorMode);
    setStatusBarText("Placing Black Men. Click squares to place/remove.");
}
void MainWindow::setupWhite()
{
    m_setupPieceType = CB_WHITE | CB_MAN;
    m_togglePieceColorMode = false;
    m_boardWidget->setSetupPieceType(m_setupPieceType);
    m_boardWidget->setTogglePieceColorMode(m_togglePieceColorMode);
    setStatusBarText("Placing White Men. Click squares to place/remove.");
}
void MainWindow::setupCc()
{
    m_togglePieceColorMode = true;
    m_boardWidget->setTogglePieceColorMode(m_togglePieceColorMode);
    setStatusBarText("Toggle Piece Color mode. Click a piece to change its color.");
}

void MainWindow::helpHelp()
{
    QMessageBox::information(this, tr("Help"),
                             tr("Welcome to Checkerboard for Linux!\n\n"
                                "This is a cross-platform checkers application. "
                                "Use the menus above to control game-play, analysis, and settings."));
    setStatusBarText("Help information displayed.");
}

void MainWindow::helpCheckersInANutshell()
{
    QUrl url("https://www.google.com/search?q=checkers+in+a+nutshell"); // Placeholder URL
    QDesktopServices::openUrl(url);
    setStatusBarText("Opening 'Checkers In A Nutshell' in browser.");
}

void MainWindow::helpHomepage()
{
    QUrl url("https://www.google.com/search?q=checkerboard+homepage"); // Placeholder URL
    QDesktopServices::openUrl(url);
    setStatusBarText("Opening application homepage in browser.");
}

void MainWindow::helpProblemOfTheDay()
{
    QUrl url("https://www.google.com/search?q=checkers+problem+of+the+day"); // Placeholder URL
    QDesktopServices::openUrl(url);
    setStatusBarText("Opening 'Problem of the Day' in browser.");
}
void MainWindow::helpOnlineUpgrade()
{
    QUrl url("https://www.google.com/search?q=checkerboard+online+upgrade"); // Placeholder URL
    QDesktopServices::openUrl(url);
    setStatusBarText("Opening 'Online Upgrade' in browser.");
}
void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this);
    setStatusBarText("Displaying About Qt information.");
}

void MainWindow::helpContents()
{
    QUrl url("https://www.google.com/search?q=checkerboard+help+contents"); // Placeholder URL
    QDesktopServices::openUrl(url);
    setStatusBarText("Opening help contents in browser.");
}

void MainWindow::handleBoardUpdated(const bitboard_pos& board)
{
    m_boardWidget->setBoard(board);
}
void MainWindow::handleGameMessage(const QString& message)
{
    qInfo() << "handleGameMessage";
}
void MainWindow::handleGameOver(int result)
{
    qInfo() << "handleGameOver";
}

void MainWindow::handleClearSelectedPiece()
{
    m_boardWidget->clearSelectedPiece();
}

void MainWindow::pieceSet()
{
    PieceSetDialog dialog(m_options.piece_set, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.piece_set = dialog.getSelectedPieceSet();
        m_boardWidget->setPieceSet(m_options.piece_set); // Apply to BoardWidget
        m_gameManager->setOptions(m_options); // Update GameManager
        setStatusBarText(QString("Piece set changed to '%1'.").arg(m_options.piece_set));
        saveSettings();
    } else {
        setStatusBarText("Piece set selection cancelled.");
    }
}

void MainWindow::optionsPriority()
{
    PriorityDialog dialog(m_options.priority, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.priority = dialog.getSelectedPriority();
        m_gameManager->setOptions(m_options);
        // Potentially inform AI to adjust its process priority
        setStatusBarText(QString("Engine priority set to %1.").arg(m_options.priority));
        saveSettings();
    } else {
        setStatusBarText("Priority selection cancelled.");
    }
}

void MainWindow::options3Move()
{
    ThreeMoveOptionsDialog dialog(m_options.three_move_option, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_options.three_move_option = dialog.getSelectedThreeMoveOption();
        m_gameManager->setOptions(m_options);
        setStatusBarText(QString("3-Move option set to %1.").arg(m_options.three_move_option));
        saveSettings();
    } else {
        setStatusBarText("3-Move option selection cancelled.");
    }
}

void MainWindow::optionsDirectories()
{
    DirectoriesDialog dialog(
        QString(m_options.userdirectory),
        QString(m_options.matchdirectory),
        QString(m_options.EGTBdirectory),
        this
    );
    if (dialog.exec() == QDialog::Accepted) {
        strncpy(m_options.userdirectory, dialog.getUserDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.userdirectory[MAX_PATH_FIXED - 1] = '\0';
        strncpy(m_options.matchdirectory, dialog.getMatchDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.matchdirectory[MAX_PATH_FIXED - 1] = '\0';
        strncpy(m_options.EGTBdirectory, dialog.getEGTBDirectory().toUtf8().constData(), MAX_PATH_FIXED - 1);
        m_options.EGTBdirectory[MAX_PATH_FIXED - 1] = '\0';

        m_gameManager->setOptions(m_options);
        emit setEgdbPath(QString(m_options.EGTBdirectory)); // Inform AI about the new EGDB path

        setStatusBarText(QString("Directories updated. User: %1, Match: %2, EGTB: %3")
                             .arg(m_options.userdirectory)
                             .arg(m_options.matchdirectory)
                             .arg(m_options.EGTBdirectory));
        saveSettings();
    } else {
        setStatusBarText("Directory selection cancelled.");
    }
}
void MainWindow::optionsUserBook()
{
    UserBookDialog dialog(m_options.book_path, false, this); // 'false' for readOnly
    if (dialog.exec() == QDialog::Accepted) {
        m_options.book_path = dialog.getSelectedUserBookPath();
        m_gameManager->getAi()->loadUserBook(m_options.book_path); // Inform AI about the new user book
        m_gameManager->setOptions(m_options);
        setStatusBarText(QString("User book set to '%1'.").arg(m_options.book_path));
        saveSettings();
    } else {
        setStatusBarText("User book selection cancelled.");
    }
}
void MainWindow::optionsLanguageEnglish()
{
    m_options.language = LANG_ENGLISH;
    m_gameManager->setOptions(m_options);
    saveSettings();
    setStatusBarText(tr("Language set to English. Restart for full effect."));
    QMessageBox::information(this, tr("Language Change"), tr("Language set to English. Please restart the application for the changes to take full effect."));
}
void MainWindow::optionsLanguageEspanol()
{
    m_options.language = LANG_ESPANOL;
    m_gameManager->setOptions(m_options);
    saveSettings();
    setStatusBarText(tr("Language set to Español. Restart for full effect."));
    QMessageBox::information(this, tr("Language Change"), tr("Language set to Español. Please restart the application for the changes to take full effect."));
}

void MainWindow::optionsLanguageItaliano()
{
    m_options.language = LANG_ITALIANO;
    m_gameManager->setOptions(m_options);
    saveSettings();
    setStatusBarText(tr("Language set to Italiano. Restart for full effect."));
    QMessageBox::information(this, tr("Language Change"), tr("Language set to Italiano. Please restart the application for the changes to take full effect."));
}
void MainWindow::optionsLanguageDeutsch()
{
    m_options.language = LANG_DEUTSCH;
    m_gameManager->setOptions(m_options);
    saveSettings();
    setStatusBarText(tr("Language set to Deutsch. Restart for full effect."));
    QMessageBox::information(this, tr("Language Change"), tr("Language set to Deutsch. Please restart the application for the changes to take full effect."));
}
void MainWindow::optionsLanguageFrancais()
{
    m_options.language = LANG_FRANCAIS;
    m_gameManager->setOptions(m_options);
    saveSettings();
    setStatusBarText(tr("Language set to Français. Restart for full effect."));
    QMessageBox::information(this, tr("Language Change"), tr("Language set to Français. Please restart the application for the changes to take full effect."));
}
void MainWindow::colorBoardNumbers()
{
    qInfo() << "colorBoardNumbers";
}
void MainWindow::colorHighlight()
{
    qInfo() << "colorHighlight";
}

void MainWindow::newGame()
{
    qInfo() << "New Game action triggered.";
    m_gameManager->newGame(GT_ENGLISH); // Or get type from a dialog
}

void MainWindow::exitApplication()
{
    qInfo() << "Exit Application action triggered.";
    close();
}

// --- Stubs for other menu actions ---
void MainWindow::game3Move() {
    qInfo() << "Game 3-Move action triggered.";
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
    qInfo() << "Game Load action triggered.";
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
    qInfo() << "Game Save action triggered.";
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
    qInfo() << "Game Analyze action triggered.";
    if (!m_isAnalyzing) {
                m_gameManager->getAi()->startAnalyzeGame(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer());        m_isAnalyzing = true;
        setStatusBarText("Engine analysis started.");
    } else {
        // Stop analysis
        m_gameManager->getAi()->abortSearch();
        m_isAnalyzing = false;
        setStatusBarText("Engine analysis stopped.");
    }
}



void MainWindow::gameCopy() {
    qInfo() << "Game Copy action triggered.";
    QString fen = m_gameManager->getFenPosition();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fen);
    setStatusBarText("Current FEN copied to clipboard.");
}
void MainWindow::gamePaste() {
    qInfo() << "Game Paste action triggered.";
    QClipboard *clipboard = QApplication::clipboard();
    QString fen = clipboard->text();
    if (!fen.isEmpty()) {
        m_gameManager->loadFenPosition(fen);
        setStatusBarText("FEN bitboard_position loaded from clipboard.");
    } else {
        setStatusBarText("Clipboard is empty or does not contain valid FEN.");
    }
}

void MainWindow::gameFind() {
    qInfo() << "Game Find action triggered.";
    FindPositionDialog dialog(this);
    dialog.exec();
}
void MainWindow::gameFindCR() {
    qInfo() << "Game Find CR action triggered.";
    FindCRDialog dialog(this);
    dialog.exec();
}
void MainWindow::gameFindTheme() {
    qInfo() << "Game Find Theme action triggered.";
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
    qInfo() << "Game Save As HTML action triggered.";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Game as HTML"),
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/game.html",
                                                   tr("HTML Files (*.html);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, tr("Save Game as HTML"),
                                 tr("Game saved as HTML to '%1' (functionality not yet fully implemented). "
                                    "This feature will generate an HTML file containing the game notation and bitboard_possibly an interactive board.").arg(fileName));
        setStatusBarText(QString("Saving game as HTML to %1.").arg(fileName));
    } else {
        setStatusBarText("Saving game as HTML cancelled.");
    }
}
void MainWindow::gameDiagram() {
    qInfo() << "Game Diagram action triggered.";
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
    qInfo() << "Game Find Player action triggered.";
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
    qInfo() << "Game FEN to Clipboard action triggered.";
    QString fen = m_gameManager->getFenPosition();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fen);
    setStatusBarText("Current FEN copied to clipboard.");
}
void MainWindow::gameFenFromClipboard() {
    qInfo() << "Game FEN from Clipboard action triggered.";
    bool ok;
    QString fen = QInputDialog::getText(this, tr("Load FEN"),
                                        tr("FEN:"), QLineEdit::Normal,
                                        "", &ok);
    if (ok && !fen.isEmpty()) {
        m_gameManager->loadFenPosition(fen);
        setStatusBarText("FEN bitboard_position loaded from input dialog.");
    } else {
        setStatusBarText("FEN loading cancelled.");
    }
}
void MainWindow::gameSelectUserBook() {
    qInfo() << "Game Select User Book action triggered.";
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
    qInfo() << "Game Re-Search action triggered.";
    m_gameManager->getAi()->requestMove(m_gameManager->getCurrentBoard(),
                      m_gameManager->getCurrentPlayer(),
                      m_gameManager->getOptions().time_per_move);
    setStatusBarText("Engine re-search initiated.");
}
void MainWindow::gameLoadNext() {
    qInfo() << "Game Load Next action triggered.";
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
    qInfo() << "Game Load Previous action triggered.";
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
    qInfo() << "Game Analyze PDN action triggered.";
    // Set AI parameters for PDN analysis mode
    m_gameManager->getAi()->startAnalyzePdn(m_gameManager->getCurrentBoard(), m_gameManager->getCurrentPlayer()); // Start the AI PDN analysis
    setStatusBarText("Game Analyze PDN: Analysis started.");
}

void MainWindow::gameInfo() {
    qInfo() << "Game Info action triggered.";
    QString info = QString("Current Game Type: %1\nTotal Moves: %2").arg(m_gameManager->getGameType()).arg(m_gameManager->getTotalMoves());
    QMessageBox::information(this, tr("Game Information"), info);
}
void MainWindow::gameSampleDiagram() {
    qInfo() << "Game Sample Diagram action triggered.";
    gameDiagram(); // Re-use the gameDiagram functionality
    setStatusBarText("Sample diagram saved (if location selected).");
}
void MainWindow::movesPlay() {
    qInfo() << "Moves Play action triggered.";
    m_gameManager->playMove();
    setStatusBarText("Moves Play action triggered.");
}
void MainWindow::movesBack() {
    qInfo() << "Moves Back action triggered.";
    m_gameManager->goBack();
    setStatusBarText("Moves Back action triggered.");
}
void MainWindow::movesForward() {
    qInfo() << "Moves Forward action triggered.";
    m_gameManager->goForward();
    setStatusBarText("Moves Forward action triggered.");
}
void MainWindow::movesBackAll() {
    qInfo() << "Moves Back All action triggered.";
    m_gameManager->goBackAll();
    setStatusBarText("Moves Back All action triggered.");
}
void MainWindow::movesForwardAll() {
    qInfo() << "Moves Forward All action triggered.";
    m_gameManager->goForwardAll();
    setStatusBarText("Moves Forward All action triggered.");
}
void MainWindow::movesComment() {
    qInfo() << "Moves Comment action triggered.";
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
void MainWindow::interruptEngine() { m_gameManager->getAi()->requestAbort(); }
void MainWindow::abortEngine() { m_gameManager->getAi()->requestAbort(); }
void MainWindow::levelExact() {
    qInfo() << "Level Exact action triggered.";
    m_gameManager->setTimeContol(LEVEL_INSTANT, true, false, 0, 0); // Example values
    setStatusBarText("Time control set to Exact.");
}
void MainWindow::levelInstant() {
    qInfo() << "Level Instant action triggered.";
    m_gameManager->setTimeContol(LEVEL_INSTANT, false, false, 0, 0); // Example values
    setStatusBarText("Time control set to Instant.");
}
void MainWindow::level01S() { 
    qInfo() << "Level 0.1S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_01S, false, false, 0.1, 0); 
    setStatusBarText("Time control set to 0.1 second."); 
}
void MainWindow::level02S() { 
    qInfo() << "Level 0.2S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_02S, false, false, 0.2, 0); 
    setStatusBarText("Time control set to 0.2 second."); 
}
void MainWindow::level05S() { 
    qInfo() << "Level 0.5S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_05S, false, false, 0.5, 0); 
    setStatusBarText("Time control set to 0.5 second."); 
}
void MainWindow::level1S() {
    qInfo() << "Level 1S action triggered.";
    m_gameManager->setTimeContol(LEVEL_1S, false, false, 1, 0);
    setStatusBarText("Time control set to 1 second.");
}
void MainWindow::level2S() { 
    qInfo() << "Level 2S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_2S, false, false, 2, 0); 
    setStatusBarText("Time control set to 2 seconds."); 
}
void MainWindow::level5S() { 
    qInfo() << "Level 5S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_5S, false, false, 5, 0); 
    setStatusBarText("Time control set to 5 seconds."); 
}
void MainWindow::level10S() { 
    qInfo() << "Level 10S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_10S, false, false, 10, 0); 
    setStatusBarText("Time control set to 10 seconds."); 
}
void MainWindow::level15S() { 
    qInfo() << "Level 15S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_15S, false, false, 15, 0); 
    setStatusBarText("Time control set to 15 seconds."); 
}
void MainWindow::level30S() { 
    qInfo() << "Level 30S action triggered."; 
    m_gameManager->setTimeContol(LEVEL_30S, false, false, 30, 0); 
    setStatusBarText("Time control set to 30 seconds."); 
}
void MainWindow::level1M() { 
    qInfo() << "Level 1M action triggered."; 
    m_gameManager->setTimeContol(LEVEL_1M, false, false, 60, 0); 
    setStatusBarText("Time control set to 1 minute."); 
}
void MainWindow::level2M() { 
    qInfo() << "Level 2M action triggered."; 
    m_gameManager->setTimeContol(LEVEL_2M, false, false, 120, 0); 
    setStatusBarText("Time control set to 2 minutes."); 
}
void MainWindow::level5M() { 
    qInfo() << "Level 5M action triggered."; 
    m_gameManager->setTimeContol(LEVEL_5M, false, false, 300, 0); 
    setStatusBarText("Time control set to 5 minutes."); 
}
void MainWindow::level15M() { 
    qInfo() << "Level 15M action triggered."; 
    m_gameManager->setTimeContol(LEVEL_15M, false, false, 900, 0); 
    setStatusBarText("Time control set to 15 minutes."); 
}
void MainWindow::level30M() {
    qInfo() << "Level 30M action triggered.";
    m_gameManager->setTimeContol(LEVEL_30M, false, false, 1800, 0);
    setStatusBarText("Time control set to 30 minutes.");
}
void MainWindow::levelInfinite() {
    qInfo() << "Level Infinite action triggered.";
    m_gameManager->setTimeContol(LEVEL_INFINITE, false, false, 0, 0); // Example values
    setStatusBarText("Time control set to Infinite.");
}

void MainWindow::engineInfinite() {
    qInfo() << "Engine Infinite action triggered.";
    QString reply;
    if (!m_isInfiniteAnalyzing) {
        if (m_gameManager->getAi()->sendCommand("infinite on", reply)) { // Assuming "infinite on" is the correct command
            m_isInfiniteAnalyzing = true;
            setStatusBarText(QString("Engine infinite analysis enabled. Reply: %1").arg(reply));
        }
        else {
            setStatusBarText("Failed to enable engine infinite analysis.");
        }
    }
    else {
        if (m_gameManager->getAi()->sendCommand("infinite off", reply)) { // Assuming "infinite off" is the correct command
            m_isInfiniteAnalyzing = false;
            setStatusBarText(QString("Engine infinite analysis disabled. Reply: %1").arg(reply));
        }
        else {
            setStatusBarText("Failed to disable engine infinite analysis.");
        }
    }
}