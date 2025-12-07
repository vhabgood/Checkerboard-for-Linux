#include "GameManager.h"
#include "c_logic.h"
#include <QDebug>

GameManager::GameManager(QObject *parent)
    : QObject(parent),
      m_currentColorToMove(CB_WHITE),
      m_whitePlayer(PlayerType::PLAYER_HUMAN),
      m_blackPlayer(PlayerType::PLAYER_AI),
      m_isGameOver(false),
      m_isAITurn(false)
{
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameManager::handleTimerTimeout);
}

GameManager::~GameManager()
{
}

void GameManager::newGame(int gameType)
{

    m_currentBoard = get_initial_board(gameType);
    m_currentColorToMove = CB_WHITE;
    m_isGameOver = false;
    emit boardUpdated(m_currentBoard);
    requestAiMove();
}

void GameManager::makeMove(const CBmove& move)
{
    if (m_isGameOver) {
        return;
    }

    domove_c(&move, &m_currentBoard);
    switchPlayer();
    emit boardUpdated(m_currentBoard);

    if (!m_isGameOver) {
        requestAiMove();
    }
}

void GameManager::onMoveSelected(const CBmove& move)
{
    if (isLegalMove(move)) {
        makeMove(move);
    }
}

void GameManager::switchPlayer()
{
    m_currentColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
}

void GameManager::requestAiMove()
{


    if ((m_currentColorToMove == CB_WHITE && m_whitePlayer == PlayerType::PLAYER_AI) ||
        (m_currentColorToMove == CB_BLACK && m_blackPlayer == PlayerType::PLAYER_AI)) {
        m_isAITurn = true;
        emit aiThinking(true);

        emit requestEngineSearch(Autoplay, m_currentBoard, m_currentColorToMove, 1.0); // Assuming 1.0 is a placeholder for timeLimit
    } else {
        m_isAITurn = false;
        emit userInputRequested();

    }
}

bool GameManager::isLegalMove(const CBmove& move)
{
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    get_legal_moves_c(m_currentBoard, m_currentColorToMove, legalMoves, nmoves, isjump, NULL, NULL);

    for (int i = 0; i < nmoves; ++i) {
        // Compare moves using coor.x and coor.y, as coor.field is not consistently used
        if (legalMoves[i].from.x == move.from.x && legalMoves[i].from.y == move.from.y &&
            legalMoves[i].to.x == move.to.x && legalMoves[i].to.y == move.to.y) {
            return true;
        }
    }
    return false;
}

char *GameManager::read_text_file_qt(const QString &filename, READ_TEXT_FILE_ERROR_TYPE &etype) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (file.exists()) {
            etype = READ_TEXT_FILE_OTHER_ERROR;
        } else {
            etype = READ_TEXT_FILE_NOT_FOUND;
        }
        return nullptr;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Convert QString to char*
    QByteArray ba = content.toUtf8();
    char *c_str = static_cast<char *>(malloc(ba.length() + 1));
    if (c_str) {
        strcpy(c_str, ba.constData());
        etype = READ_TEXT_FILE_NO_ERROR;
    } else {
        etype = READ_TEXT_FILE_COULD_NOT_OPEN; // Treat malloc failure as could not open
    }
    return c_str;
}

int GameManager::read_match_stats_internal() { return 0; }
void GameManager::reset_match_stats_internal() {}
void GameManager::update_match_stats_internal(int result, int movecount, int gamenumber, emstats_t *stats) { Q_UNUSED(result); Q_UNUSED(movecount); Q_UNUSED(gamenumber); Q_UNUSED(stats); }
int GameManager::num_ballots_internal() { return 0; }
int GameManager::game0_to_ballot0_internal(int game0) { Q_UNUSED(game0); return 0; }
int GameManager::game0_to_match0_internal(int game0) { Q_UNUSED(game0); return 0; }
QString GameManager::emstats_filename_internal() { return QString(); }
QString GameManager::emprogress_filename_internal() { return QString(); }
QString GameManager::empdn_filename_internal() { return QString(); }
QString GameManager::emlog_filename_internal() { return QString(); }
void GameManager::quick_search_both_engines_internal() {}
void GameManager::start3MoveGame(int opening_index) {

    start3move_c(&m_currentBoard, opening_index);
    m_currentColorToMove = CB_BLACK; // Black always starts 3-move openings
    m_isGameOver = false;
    m_boardHistory.clear();
    m_boardHistory.append(getFenPosition());
    m_halfMoveCount = 0;
    emit boardUpdated(m_currentBoard);
    requestAiMove();
}
void GameManager::savePdnGame(const QString &filename) { Q_UNUSED(filename); }
QString GameManager::getFenPosition() {
    char fen_c_str[256];
    bitboard_postoFEN(&m_currentBoard, fen_c_str, m_currentColorToMove, GT_ENGLISH);
    return QString(fen_c_str);
}
void GameManager::loadFenPosition(const QString& fen) {

    int color_to_move;
    int success = FENtobitboard_pos(&m_currentBoard, fen.toUtf8().constData(), &color_to_move, GT_ENGLISH);
    if (success) {
        m_currentColorToMove = color_to_move;
        m_isGameOver = false;
        m_boardHistory.clear();
        m_boardHistory.append(getFenPosition());
        m_halfMoveCount = 0;
        emit boardUpdated(m_currentBoard);
        requestAiMove();
    } else {
    
    }
}
void GameManager::setCurrentPdnGame(const PDNgame& gameData) {
    m_currentPdnGame.pdn = gameData;
    m_currentBoard = get_initial_board(gameData.gametype); // Reinitialize board based on game type
    // Apply moves from PDNgame to m_currentBoard
    for (int i = 0; i < gameData.nmoves; ++i) {
        domove_c(&gameData.moves[i], &m_currentBoard);
    }
    m_currentColorToMove = get_startcolor(gameData.gametype);
    emit boardUpdated(m_currentBoard);
}

const PdnGameWrapper& GameManager::getCurrentPdnGame() const { return m_currentPdnGame; }
void GameManager::setOptions(const CBoptions& options) {
    m_options = options;
    // Apply options that directly affect GameManager's state
    m_whitePlayer = m_options.white_player_type;
    m_blackPlayer = m_options.black_player_type;
    m_gameTimer->setInterval(1000); // 1 second interval
    if (m_options.enable_game_timer) {
        m_gameTimer->start();
    } else {
        m_gameTimer->stop();
    }

}

int GameManager::getGameType() const { return m_options.gametype; }
int GameManager::getTotalMoves() const { return m_boardHistory.size() - 1; } // Exclude initial position
CBmove GameManager::getLastMove() const { return m_lastMove; }
void GameManager::handleSquareClick(int x, int y) {
    // This is a placeholder. Actual logic will involve move validation, piece selection, etc.

    // For human player, if a piece is selected, try to make a move.
    // If no piece selected, select the piece at (x,y) if it belongs to current player.
    if (m_isAITurn) {

        return;
    }
    
    // Convert to square number (1-32)
    int clicked_square_num = coorstonumber(x, y, GT_ENGLISH);
    if (clicked_square_num == 0) return; // Clicked on a light square

    int piece_at_clicked_square = get_piece(m_currentBoard, clicked_square_num - 1);
    
    if (!m_pieceSelected) {
        // No piece selected, try to select one
        if (piece_at_clicked_square != CB_EMPTY && (piece_at_clicked_square & m_currentColorToMove)) {
            m_selectedX = x;
            m_selectedY = y;
            m_pieceSelected = true;
            emit pieceSelected(x, y);

        }
    } else {
        // A piece is already selected, try to make a move
        CBmove proposed_move;
        proposed_move.from.x = m_selectedX;
        proposed_move.from.y = m_selectedY;
        proposed_move.to.x = x;
        proposed_move.to.y = y;
        proposed_move.is_capture = false; // Will be determined by isLegalMove

        if (isLegalMove(proposed_move)) {
            makeMove(proposed_move);
            emit requestClearSelectedPiece();
            emit pieceDeselected();
        } else {
            // Invalid move, deselect piece
            emit requestClearSelectedPiece();
            emit pieceDeselected();

        }
    }
}

void GameManager::loadPdnGame(const QString &filename) { Q_UNUSED(filename); }
void GameManager::resumePlay() {

    requestAiMove(); // Request AI move if it's AI's turn
}
void GameManager::clearBoard() {
    newgame(&m_currentBoard);
    emit boardUpdated(m_currentBoard);
}
void GameManager::setPiece(int x, int y, int pieceType) {
    int square_num = coorstonumber(x, y, GT_ENGLISH);
    if (square_num != 0) {
        set_piece(&m_currentBoard, square_num - 1, pieceType);
        emit boardUpdated(m_currentBoard);
    }
}
void GameManager::togglePieceColor(int x, int y) { Q_UNUSED(x); Q_UNUSED(y); }
void GameManager::playMove() {
    // This function typically plays the current move from history/PDN.
    // Assuming m_boardHistory and m_currentMoveIndex exist.
    // For now, it could be a placeholder or call goForward()
    goForward();
}
void GameManager::goBack() {
    if (m_boardHistory.size() > 1) {
        m_boardHistory.removeLast(); // Remove current bitboard_position
        loadFenPosition(m_boardHistory.last()); // Load previous bitboard_position
        m_currentColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE; // Toggle color
        emit boardUpdated(m_currentBoard);
    }
}
void GameManager::goForward() {
    // This would typically involve replaying moves from a PDN or history.
    // Needs more complex logic related to PDN parsing and move application.
}
void GameManager::goBackAll() {
    if (m_boardHistory.size() > 1) {
        loadFenPosition(m_boardHistory.first()); // Load initial position
        m_currentColorToMove = CB_WHITE; // Assuming white starts
        m_boardHistory.clear();
        m_boardHistory.append(getFenPosition());
        emit boardUpdated(m_currentBoard);
    }
}
void GameManager::goForwardAll() { /* Similar to goForward, but to end of game */ }
void GameManager::sendGameHistory() { /* Emit signal with game history */ }
void GameManager::detectDraws() { /* Logic to detect draws */ }
void GameManager::addComment(const QString& comment) { Q_UNUSED(comment); /* Add to PDN data */ }
int GameManager::PDNparseGetnextgame(char **start, char *game, int maxlen) { Q_UNUSED(start); Q_UNUSED(game); Q_UNUSED(maxlen); return 0; }
void GameManager::parsePdnGameString(char* game_str, PdnGameWrapper& game) { Q_UNUSED(game_str); Q_UNUSED(game); }
int GameManager::PDNparseGetnumberofgames(char *filename) { Q_UNUSED(filename); return 0; }
void GameManager::setTimeContol(int level, bool exact_time, bool use_incremental_time, int initial_time, int time_increment) {
    Q_UNUSED(level); Q_UNUSED(exact_time); Q_UNUSED(use_incremental_time); Q_UNUSED(initial_time); Q_UNUSED(time_increment);

}
void GameManager::addTimeToClock(int seconds) { Q_UNUSED(seconds); }
void GameManager::subtractFromClock(int seconds) { Q_UNUSED(seconds); }
bitboard_pos GameManager::getCurrentBoard() const { return m_currentBoard; }
void GameManager::setCurrentBoard(const bitboard_pos& board) {
    m_currentBoard = board;
    emit boardUpdated(m_currentBoard);
}
int GameManager::getCurrentPlayer() const { return m_currentColorToMove; }
void GameManager::setCurrentColorToMove(int color) {
    m_currentColorToMove = color;
    emit boardUpdated(m_currentBoard);
}
int GameManager::getHalfMoveCount() const { return m_halfMoveCount; }
CBoptions GameManager::getOptions() const { return m_options; }
void GameManager::setSoundEnabled(bool enabled) { Q_UNUSED(enabled); }
void GameManager::handleEvaluationUpdate(int score, int depth) {
    // Simply re-emit the signal
    emit evaluationUpdated(score, depth);
}
void GameManager::handleTimerTimeout() {
    // Logic for handling timer ticks
    if (!m_isGameOver && !m_isAITurn) {
        if (m_currentColorToMove == CB_WHITE) {
            m_whiteTime -= m_gameTimer->interval() / 1000.0;
        } else {
            m_blackTime -= m_gameTimer->interval() / 1000.0;
        }
        emit updateClockDisplay(m_whiteTime, m_blackTime);

        if (m_whiteTime <= 0 || m_blackTime <= 0) {
            m_isGameOver = true;
            emit gameIsOver(m_whiteTime <= 0 ? CB_BLACK : CB_WHITE); // Player whose time ran out loses
        }
    }
}
void GameManager::handleGameOverResult(int result) {
    m_isGameOver = true;
    emit gameIsOver(result);

}

void GameManager::handleAiMove(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime)
{
    Q_UNUSED(pdnMoveText); // This might be used later for PDN saving
    Q_UNUSED(elapsedTime); // This might be used for stats




    
    m_isAITurn = false;
    emit aiThinking(false);



    if (moveFound) {
        // Apply the AI's best move
        makeMove(bestMove); // This will handle switching player, updating board, and requesting next AI move if needed
    } else {
        // AI found no move. Game Over or other condition. Result: gameResult
        m_isGameOver = true;
        // The gameResult from the AI (CB_WIN, CB_LOSS, CB_DRAW) should be used here
        emit gameIsOver(gameResult); 
    }
}