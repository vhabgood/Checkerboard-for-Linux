#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>
#include "checkers_types.h"




class GameManager : public QObject
{
    Q_OBJECT

public:
    explicit GameManager(QObject *parent = nullptr);
    ~GameManager();

    // Public API for game management
    void newGame(int gameType);
    void makeMove(const CBmove& move);
    static char *read_text_file_qt(const QString &filename, READ_TEXT_FILE_ERROR_TYPE &etype);
    static int read_match_stats_internal();
    static void reset_match_stats_internal();
    static void update_match_stats_internal(int result, int movecount, int gamenumber, emstats_t *stats);
    int num_ballots_internal();
    int game0_to_ballot0_internal(int game0);
    int game0_to_match0_internal(int game0);
    static QString emstats_filename_internal();
    static QString emprogress_filename_internal();
    static QString empdn_filename_internal();
    static QString emlog_filename_internal();
    void quick_search_both_engines_internal();
    void start3MoveGame(int opening_index);
    void savePdnGame(const QString &filename);
    QString getFenPosition();
    void loadFenPosition(const QString& fen);
    bool isLegalMove(const CBmove& move); // New: Checks if a move is legal
    void setCurrentPdnGame(const PDNgame& gameData);
    const PdnGameWrapper& getCurrentPdnGame() const;
    void setOptions(const CBoptions& options);
    int getGameType() const;
    int getTotalMoves() const;
    CBmove getLastMove() const;
    void handleSquareClick(int x, int y);
    void loadPdnGame(const QString &filename);



    // Setup Mode functions
    void clearBoard();
    void setPiece(int x, int y, int pieceType);
    void togglePieceColor(int x, int y);

    // Game Navigation functions
    void playMove();
    void switchTurn();
    void goBack();
    void goForward();
    void goBackAll();
    void goForwardAll();
    void sendGameHistory();
    void detectDraws();
    void addComment(const QString& comment);

    // PDN Parsing functions (made public for GameDatabaseDialog)
    static int PDNparseGetnextgame(char **start, char *game, int maxlen);
    static void parsePdnGameString(char* game_str, PdnGameWrapper& game);
    static int PDNparseGetnumberofgames(char *filename);

    // Time Control functions
    void setTimeContol(int level, bool exact_time, bool use_incremental_time, int initial_time, int time_increment);
    void addTimeToClock(int seconds);
    void subtractFromClock(int seconds);

    // Public getters for AI to access game state
    Board8x8 getCurrentBoard() const;
    void setCurrentBoard(const Board8x8& board);
    int getCurrentPlayer() const;
    void setCurrentColorToMove(int color);
    int getHalfMoveCount() const;
    CBoptions getOptions() const;
    void setSoundEnabled(bool enabled);

public slots:
    void handleEvaluationUpdate(int score, int depth); // New slot to receive evaluation from AI and re-emit
    void handleTimerTimeout(); // Slot to handle timer timeout
    void handleGameOverResult(int result); // New slot to handle game over results
    void handleAIMoveFound(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);

signals:
    // Signals to communicate with the GUI (e.g., update board, display message)
    void boardUpdated(const Board8x8& board);
    void gameMessage(const QString& message);
    void gameIsOver(int result);
    void requestEngineSearch(const Board8x8& board, int colorToMove, double timeLimit);
    void pieceSelected(int x, int y);
    void pieceDeselected();
    void humanTurn();
    void evaluationUpdated(int score, int depth); // New signal to emit evaluation score and depth
    void updateClockDisplay(double whiteTime, double blackTime); // New signal to update clock display
    void sendEngineCommand(const QString& command); // Corrected signal signature

private:
    void reconstructBoardState(int move_index);
    Board8x8 m_currentBoard;
    int m_currentColorToMove;
    PdnGameWrapper m_currentPdnGame;
    bool m_pieceSelected;
    int m_selectedX;
    int m_selectedY;
    QList<QString> m_boardHistory; // Stores FEN strings of past board positions for draw detection
    CBmove m_lastMove; // Stores the last move made
    bool m_forcedCapturePending;
    int m_engineColor; // Added to store the AI's color
    CBoptions m_options; // Added to store game options
    double m_whiteTime;
    double m_blackTime;
    int m_halfMoveCount; // New member for 50-move rule
    QTimer *m_gameTimer; // New member for game timer
};