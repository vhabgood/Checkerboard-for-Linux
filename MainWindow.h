#pragma once

#include "GeminiAI.h"
#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QSettings>
#include <QMutex>
#include <QMenuBar>
#include <QMenu>
#include <QDate>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFile>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>



#include "GameManager.h"
#include "BoardWidget.h"

#include "Dialogs.h"

#include "checkers_types.h"



#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include <QList>
#include <QVector>

class GameManager;
class BoardWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GameManager *gameManager, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void startGame();
    void changeAppState(AppState newState); 
    void setStatusBarText(const QString& text); 
    void updateEvaluationDisplay(int score, int depth); 

private slots: 


    // --- Menu Actions --- 
    // File Menu 
    void newGame(); 
    void exitApplication(); 
    // // Game Menu 
    void game3Move(); 
    void gameLoad(); 
    void gameSave(); 
    void gameInfo(); 
    void gameAnalyze(); 
    void gameCopy(); 
    void gamePaste(); 

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
    // // Moves Menu 
    void movesPlay(); 
    void movesBack(); 
    void movesForward(); 
    void movesBackAll(); 
    void movesForwardAll(); 
    void movesComment(); 
    void interruptEngine(); 
    void abortEngine(); 
    // // Options Menu - Level 
    void levelExact(); 
    void levelInstant(); 
    void level01S(); void level02S(); void level05S(); 
    void level1S(); void level2S(); void level5S(); 
    void level10S(); void level15S(); void level30S(); 
    void level1M(); void level2M(); void level5M(); 
    void level15M(); void level30M(); 
    void levelInfinite(); 
    // void levelIncrement(); 
    // void levelAddTime(); 
    // void levelSubtractTime(); 
    // Options Menu - Other 
    void pieceSet(); 
    void optionsHighlight(); 
    void optionsSound(); 
    void optionsPriority(); 
    void options3Move(); 
    void optionsDirectories(); 
    void optionsUserBook(); 
    // // Options Menu - Language (Basic Placeholders) 
    void optionsLanguageEnglish(); 
    void optionsLanguageEspanol(); 
    void optionsLanguageItaliano(); 
    void optionsLanguageDeutsch(); 
    void optionsLanguageFrancais(); 
    // // Options Menu - Display 
    void displayInvert(); 
    void displayNumbers(); 
    void displayMirror(); 
    // // Options Menu - Mode 
    void cmNormal(); 
    void cmAnalysis(); 
    // void gotoNormal(); 
    void cmAutoplay(); 
    void cm2Player(); 
    void engineVsEngine(); // Same as cmEngineMatch? 
    // // Options Menu - Colors 
    void colorBoardNumbers(); 
    void colorHighlight(); 
    void bookModeView(); 
    void bookModeAdd(); 
    void bookModeDelete(); 
    // // Engine Menu 
    void engineSelect(); 
    void engineAbout(); 
    void engineHelp(); 
    void engineOptions(); 
    void engineAnalyze(); 
    void engineInfinite(); 
    void engineResign(); 
    void engineDraw(); 
    void engineUndoMove(); 
    void enginePonder(); 
    void cmEngineMatch(); 
    void cmAddComment(); 
    void engineEval(); 
    void cmEngineCommand(); 
    void cmRunTestSet(); 
    void cmHandicap(); 
    // // Setup Menu 
    void setupMode(); 
    void setupClear(); 
    void setupBlack(); 
    void setupWhite(); 
    void setupCc(); // Change Color 
    // // Help Menu 
    void helpHelp(); 
    void helpAbout(); 
    void helpCheckersInANutshell(); 
    void helpHomepage(); 
    void helpProblemOfTheDay(); 
    void helpOnlineUpgrade(); 
    void helpAboutQt(); // Added declaration 
    void helpContents(); // Added declaration 
    // --- End Menu Actions --- 

    // Game Logic Slots 
    void handleBoardUpdated(const Board8x8& board); 
    void handleGameMessage(const QString& message); 
    void handleGameOver(int result); 

signals:
    void appStateChangeRequested(AppState newState);
    void setPrimaryEnginePath(const QString& path);
    void setSecondaryEnginePath(const QString& path);
    void setEgdbPath(const QString& path); // Add this new signal
    void setAiOptions(const CBoptions& options); // New signal to send options to AI

private slots:
    void onAppStateChangeRequested(AppState newState);

private:
    // Setup Methods
    void createMenus();
    void createToolBars();
    void loadSettings();
    void saveSettings();

    GameManager *m_gameManager; 

    // AI Management 
    GeminiAI *m_ai; 
    QThread *m_aiThread; 



    // UI Elements 
    BoardWidget *m_boardWidget;
    QToolBar *m_mainToolBar;
    QLabel *m_blackClockLabel; 
    QLabel *m_whiteClockLabel; 
    QLabel *m_evaluationLabel; 
    QLabel *m_depthLabel; 
    QSettings settings; 
    QTimer *m_uiUpdateTimer; 
    bool m_pieceSelected; 
    AppState m_currentState; 
    bool m_isAnalyzing; 
    bool m_isPondering; 
    bool m_isInfiniteAnalyzing; 
    int m_setupPieceType; 
    bool m_togglePieceColorMode; 

    QMutex m_optionsMutex; 
    CBoptions m_options; 

    QList<QString> m_pdnGameHistory; 
    int m_currentPdnGameIndex; 

    // Actions for state management 
    QAction *m_newGameAction; 
    QAction *m_game3MoveAction; 
    QAction *m_gameLoadAction; 
    QAction *m_gameSaveAction; 
    QAction *m_movesPlayAction; 
    QAction *m_interruptEngineAction; 
    QAction *m_abortEngineAction; 
    QAction *m_setupModeAction; 
    QAction *m_setupClearAction; 
    QAction *m_setupBlackAction; 
    QAction *m_setupWhiteAction; 
    QAction *m_setupCcAction; 

    // Game Menu Actions 
    QAction *m_gameAnalyzeAction; 
    QAction *m_gameCopyAction; 
    QAction *m_gamePasteAction; 

    QAction *m_gameFenToClipboardAction; 
    QAction *m_gameFenFromClipboardAction; 
    QAction *m_gameAnalyzePdnAction; 

    // Moves Menu Actions 
    QAction *m_movesBackAction; 
    QAction *m_movesForwardAction; 
    QAction *m_movesBackAllAction; 
    QAction *m_movesForwardAllAction; 
    QAction *m_movesCommentAction; 

    // Options Menu Actions 
    QAction *m_optionsHighlightAction; 
    QAction *m_optionsSoundAction; 
    QAction *m_displayInvertAction; 
    QAction *m_displayNumbersAction; 
    QAction *m_displayMirrorAction; 
    QAction *m_cmNormalAction; 
    QAction *m_cmAnalysisAction; 
    QAction *m_cmAutoplayAction; 
    QAction *m_cm2PlayerAction; 
    QAction *m_engineVsEngineAction; 
    QAction *m_bookModeViewAction; 
    QAction *m_bookModeAddAction; 
    QAction *m_bookModeDeleteAction; 
    QAction *m_bookModeDeleteAllAction; 

    // Book Mode Slots 
    void bookModeDeleteAll(); 

    // Engine Menu Actions 
    QAction *m_engineSelectAction; 
    QAction *m_engineOptionsAction; 
    QAction *m_engineEvalAction; 
    QAction *m_cmEngineMatchAction; 
    QAction *m_cmAddCommentAction; 
    QAction *m_cmEngineCommandAction; 
    QAction *m_cmRunTestSetAction; 
    QMenu *m_engineMenu;

    AI_State mapAppStatetoAIState(AppState appState); 
};