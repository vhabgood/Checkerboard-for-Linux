#include "checkers_types.h" // Direct include
#include <QDebug>
#include "GameManager.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDate> // Added for QDate

#include <QDateTime>
#include <cstring> // For strtok, strncpy, strcmp
#include <cstdlib> // For atoi
extern "C" {
#include "c_logic.h" // Added for C functions like newgame, domove_c, board8toFEN, FENtoboard8
}
#include "checkers_types.h" // Direct include

extern LogLevel s_minLogLevel;

// Static member definitions






static inline bool is_pdnquote(uint8_t c)
{
    if (c == '"')
        return(true);
    return(false);
}

int GameManager::PDNparseGetnextgame(char **start, char *game, int maxlen)
{
    char *p;
    char *p_org;
    int headersdone = 0;

    game[0] = 0;
    if ((*start) == 0)
        return 0;

    p = (*start);
    p_org = p;
    while (*p != 0) {
        if (*p == '[' && !headersdone) {
            p++;
            while (*p != ']' && *p != 0) {
                if (is_pdnquote(*p)) {
                    ++p;
                    while (!is_pdnquote(*p) && *p != 0) {
                        ++p;
                    }
                    if (*p == 0)
                        break;
                }
                p++;
            }
        }

        if (*p == 0)
            break;

        if (*p == '{') {
            p++;
            while (*p != '}' && *p != 0) {
                p++;
            }
        }

        if (*p == 0)
            break;

        if (isdigit((uint8_t) *p))
            headersdone = 1;

        if (p[0] == '[' && headersdone) {
            p--;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '1' && p[1] == '-' && p[2] == '0') {
            p += 3;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '0' && p[1] == '-' && p[2] == '1' && !isdigit((uint8_t) p[3])) {
            p += 3;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '*') {
            p++;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '1' && p[1] == '/' && p[2] == '2' && p[3] == '-' && p[4] == '1' && p[5] == '/' && p[6] == '2') {
            p += 7;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        p++;
    }

    if (headersdone) {
        strncpy(game, *start, p - *start);
        game[p-*start] = 0;
        *start = p;
        return (int)(p - p_org);
    }
    return 0; // Added return statement for the case where no game is found
}


void GameManager::parsePdnGameString(char* game_str, PdnGameWrapper& game) {
    char game_str_copy[10000]; 
    strncpy(game_str_copy, game_str, sizeof(game_str_copy) - 1);
    game_str_copy[sizeof(game_str_copy) - 1] = '\0';

    char* line = strtok(game_str_copy, "\n\r");
    while (line != NULL) {
        if (line[0] == '[') {
            // Parse header
            char* key_start = line + 1;
            char* key_end = strchr(key_start, ' ');
            if (key_end) {
                *key_end = '\0';
                char* value_start = strchr(key_end + 1, '"');
                if (value_start) {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        *value_end = '\0';
                        if (strcmp(key_start, "Event") == 0) strncpy(game.game.event, value_start, sizeof(game.game.event)-1);
                        else if (strcmp(key_start, "Site") == 0) strncpy(game.game.site, value_start, sizeof(game.game.site)-1);
                        else if (strcmp(key_start, "Date") == 0) strncpy(game.game.date, value_start, sizeof(game.game.date)-1);
                        else if (strcmp(key_start, "Round") == 0) strncpy(game.game.round, value_start, sizeof(game.game.round)-1);
                        else if (strcmp(key_start, "White") == 0) strncpy(game.game.white, value_start, sizeof(game.game.white)-1);
                        else if (strcmp(key_start, "Black") == 0) strncpy(game.game.black, value_start, sizeof(game.game.black)-1);
                        else if (strcmp(key_start, "Result") == 0) strncpy(game.game.resultstring, value_start, sizeof(game.game.resultstring)-1);
                        else if (strcmp(key_start, "FEN") == 0) strncpy(game.game.FEN, value_start, sizeof(game.game.FEN)-1);
                    }
                }
            }
        } else if (isdigit(line[0]) || line[0] == '{') {
            // Parse moves
            char* context = nullptr;
            char* move_token = strtok_r(line, " ", &context);
            while (move_token != NULL) {
                if (strchr(move_token, '.') != NULL) { // Is a move number, skip it
                     move_token = strtok_r(NULL, " ", &context);
                     continue;
                }
                
                CBmove temp_move = {0}; // Create a temporary move
                if (move_token[0] == '{') { 
                    // This is a comment, find the end and store it
                    char* end_comment = strchr(move_token, '}');
                    if(end_comment) *end_comment = '\0';
                    strncpy(temp_move.comment, move_token + 1, sizeof(temp_move.comment) - 1);
                    // We need to associate this with the PREVIOUS move
                    if (!game.moves.isEmpty()) {
                        strcpy(game.moves.last().comment, temp_move.comment);
                    }
                     move_token = strtok_r(NULL, " ", &context);
                     continue;
                }

                int from = 0, to = 0;
                sscanf(move_token, "%d", &from);
                char* sep = strpbrk(move_token, "-x");
                if (sep) {
                    sscanf(sep + 1, "%d", &to);
                }
                
                numbertocoors(from, &temp_move.from.x, &temp_move.from.y, GT_ENGLISH);
                numbertocoors(to, &temp_move.to.x, &temp_move.to.y, GT_ENGLISH);
                game.moves.push_back(temp_move);
                
                move_token = strtok_r(NULL, " ", &context);
            }
        }
        line = strtok(NULL, "\n\r");
    }
}

int GameManager::PDNparseGetnumberofgames(char *filename)
{
    char *buffer;
    char game_str_buffer[10000]; // Use a buffer for game string
    char *p;
    int ngames;
    READ_TEXT_FILE_ERROR_TYPE etype;

    buffer = read_text_file_c(filename, &etype);
    if (buffer == NULL)
        return -1;

    p = buffer;
    ngames = 0;
    while (GameManager::PDNparseGetnextgame(&p, game_str_buffer, sizeof(game_str_buffer)))
        ++ngames;

    free(buffer);
    return(ngames);
}

void GameManager::loadPdnGame(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Failed to open PDN file:" << filename;
        g_programStatusWord |= STATUS_FILE_IO_ERROR;
        return;
    }
    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    QByteArray ba = fileContent.toUtf8();
    char* buffer = ba.data();
    
    char game_str[10000];
    if (PDNparseGetnextgame(&buffer, game_str, sizeof(game_str))) {
        PdnGameWrapper tempGame;
        parsePdnGameString(game_str, tempGame);
        
        m_currentPdnGame.game = tempGame.game;
        m_currentPdnGame.moves.clear();

        Board8x8 tempBoard;
        int currentTurn;

        if (strlen(m_currentPdnGame.game.FEN) > 0) {
            FENtoboard8(&tempBoard, m_currentPdnGame.game.FEN, &currentTurn, m_currentPdnGame.game.gametype);
        } else {
            newgame(&tempBoard);
            currentTurn = CB_BLACK;
        }

        for (const CBmove& simpleMove : tempGame.moves) {
            CBmove legalMoves[MAXMOVES];
            int nmoves = 0;
            int isjump = 0;
            bool can_continue_multijump = false;
            get_legal_moves_c(&tempBoard, currentTurn, legalMoves, &nmoves, &isjump, NULL, &can_continue_multijump);

            bool moveFound = false;
            for (int i = 0; i < nmoves; ++i) {
                if (legalMoves[i].from.x == simpleMove.from.x &&
                    legalMoves[i].from.y == simpleMove.from.y &&
                    legalMoves[i].to.x == simpleMove.to.x &&
                    legalMoves[i].to.y == simpleMove.to.y) {
                    
                    m_currentPdnGame.moves.push_back(legalMoves[i]);
                    domove_c(&legalMoves[i], &tempBoard);
                    currentTurn = (currentTurn == CB_WHITE) ? CB_BLACK : CB_WHITE;
                    moveFound = true;
                    break;
                }
            }
            if (!moveFound) {
                qWarning() << "Could not reconstruct legal move from PDN. History may be incomplete.";
                break;
            }
        }

        m_currentBoard = tempBoard;
        m_currentColorToMove = currentTurn;
        m_currentPdnGame.game.movesindex = m_currentPdnGame.moves.size();
        
        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Loaded game: %1").arg(m_currentPdnGame.game.event));
        g_programStatusWord |= STATUS_GAME_LOAD_PDN_OK;
    } else {
        qCritical() << "Failed to parse any games from PDN file:" << filename;
    }
}

GameManager::GameManager(QObject *parent) : QObject(parent),
    m_pieceSelected(false),
    m_selectedX(-1),
    m_selectedY(-1),
    m_forcedCapturePending(false),
    m_engineColor(CB_WHITE),
    m_whiteTime(0.0),
    m_blackTime(0.0),
    m_halfMoveCount(0),
    m_gameTimer(new QTimer(this))
{
    g_programStatusWord |= STATUS_GAMEMANAGER_INIT_START;
    qDebug() << "GameManager: Initializing.";
    // connect(m_gameTimer, &QTimer::timeout, this, &GameManager::handleTimerTimeout);
    connect(this, &GameManager::gameIsOver, this, &GameManager::handleGameOverResult); // Connect gameIsOver to handleGameOverResult
}


GameManager::~GameManager()
{
}

void GameManager::newGame(int gameType)
{
    qDebug() << QString("GameManager: Starting new game of type %1").arg(gameType);
    // Call C function to set up initial board
    newgame(&m_currentBoard);
    g_programStatusWord |= STATUS_BOARD_INIT_OK; // Set status flag for board initialization
    m_currentColorToMove = CB_BLACK; // Black typically starts
    // Set turn flag
    g_programStatusWord &= ~STATUS_TURN_MASK; // Clear existing turn bits
    if (m_currentColorToMove == CB_WHITE) {
        g_programStatusWord |= (1U << STATUS_TURN_BIT_POS); // Set bit for White's turn
    } else {
        g_programStatusWord |= (0U << STATUS_TURN_BIT_POS); // Clear bit for Black's turn
    }
    m_halfMoveCount = 0; // Reset half-move count for new game
    m_boardHistory.clear(); // Clear board history for new game
    QString fen = getFenPosition();
    m_boardHistory.append(fen); // Add initial position to history
    qDebug() << "GameManager::newGame, FEN:" << fen;

    // Determine m_engineColor based on options
    if (m_options.white_player_type == PLAYER_AI && m_options.black_player_type == PLAYER_AI) {
        m_engineColor = m_currentColorToMove; // Both are AI, so AI moves for current player
    } else if (m_options.white_player_type == PLAYER_AI) {
        m_engineColor = CB_WHITE;
    } else if (m_options.black_player_type == PLAYER_AI) {
        m_engineColor = CB_BLACK;
    } else {
        m_engineColor = CB_EMPTY; // No AI playing
    }

    // Set game type flag
    g_programStatusWord &= ~STATUS_GAMETYPE_MASK; // Clear existing game type bits
    if (gameType == GT_3MOVE) {
        g_programStatusWord |= (1U << STATUS_GAMETYPE_BIT_POS); // Set bit 1 for 3-move game
    } else {
        g_programStatusWord |= (0U << STATUS_GAMETYPE_BIT_POS); // Clear bit for normal game
    }

    // Set white player type flag
    g_programStatusWord &= ~STATUS_WHITE_PLAYER_MASK; // Clear existing white player type bits
    if (m_options.white_player_type == PLAYER_HUMAN) {
        g_programStatusWord |= (1U << STATUS_WHITE_PLAYER_BIT_POS); // Set bit 1 for human
    } else if (m_options.white_player_type == PLAYER_AI) {
        g_programStatusWord |= (2U << STATUS_WHITE_PLAYER_BIT_POS); // Set bit 2 for AI
    } else {
        g_programStatusWord |= (0U << STATUS_WHITE_PLAYER_BIT_POS); // Set bit 0 for None
    }

    // Set black player type flag
    g_programStatusWord &= ~STATUS_BLACK_PLAYER_MASK; // Clear existing black player type bits
    if (m_options.black_player_type == PLAYER_HUMAN) {
        g_programStatusWord |= (1U << STATUS_BLACK_PLAYER_BIT_POS); // Set bit 1 for human
    } else if (m_options.black_player_type == PLAYER_AI) {
        g_programStatusWord |= (2U << STATUS_BLACK_PLAYER_BIT_POS); // Set bit 2 for AI
    } else {
        g_programStatusWord |= (0U << STATUS_BLACK_PLAYER_BIT_POS); // Set bit 0 for None
    }

    qDebug() << QString("GameManager: newGame - m_options.white_player_type: %1, m_options.black_player_type: %2")
                 .arg(m_options.white_player_type)
                 .arg(m_options.black_player_type);

    emit boardUpdated(m_currentBoard);
    emit gameMessage("New game started!");
    g_programStatusWord |= STATUS_NEW_GAME_OK; // Set status flag for new game start

    if (m_options.enable_game_timer) {
        m_gameTimer->start(1000); // Tick every second
    }

    // If the current player is an AI, trigger its move
    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
        playMove();
    } else {
        emit humanTurn();
    }
}

void GameManager::makeMove(const CBmove& move)
{
    qDebug() << QString("GameManager: Moving (%1-%2)").arg(coorstonumber(move.from.x, move.from.y, GT_ENGLISH)).arg(coorstonumber(move.to.x, move.to.y, GT_ENGLISH));

    // Check if the move is a capture or a pawn move for 50-move rule
    bool isCapture = move.jumps > 0;
    bool isPawnMove = (move.oldpiece == (CB_WHITE | CB_MAN) || move.oldpiece == (CB_BLACK | CB_MAN));

    // 2. Apply the move
    Board8x8 tempBoard = m_currentBoard;
    domove_c(&move, &tempBoard);
    m_currentBoard = tempBoard;
    m_lastMove = move; // Update m_lastMove

    // If we are not at the end of the move history (i.e., some moves were undone),
    // discard all "future" moves before adding the new one.
    if (m_currentPdnGame.game.movesindex < m_currentPdnGame.moves.size()) {
        m_currentPdnGame.moves.erase(m_currentPdnGame.moves.begin() + m_currentPdnGame.game.movesindex, m_currentPdnGame.moves.end());
    }

    // Add the move to the game history
    m_currentPdnGame.moves.push_back(move);
    m_currentPdnGame.game.movesindex++;

    // Update half-move count for 50-move rule
    if (isCapture || isPawnMove) {
        m_halfMoveCount = 0;
    } else {
        m_halfMoveCount++;
    }

    // Add current board FEN to history for draw detection
    QString fen = getFenPosition();
    m_boardHistory.append(fen);

    // Stop and restart the timer for the next player
    if (m_options.enable_game_timer) {
        m_gameTimer->stop();
        m_gameTimer->start(1000);
    }

    // 4. Emit boardUpdated
    emit boardUpdated(m_currentBoard);

    qDebug() << QString("GameManager: Board state after move: %1").arg(fen);

}

void GameManager::handleTimerTimeout()
{
    qDebug() << "GameManager::handleTimerTimeout called.";
    if (m_currentColorToMove == CB_WHITE) {
        m_whiteTime -= 1.0;
        if (m_whiteTime <= 0) {
            m_whiteTime = 0;
            m_gameTimer->stop();
            emit gameIsOver(CB_LOSS); // White loses on time
            emit gameMessage("White ran out of time!");
        }
    } else {
        m_blackTime -= 1.0;
        if (m_blackTime <= 0) {
            m_blackTime = 0;
            m_gameTimer->stop();
            emit gameIsOver(CB_WIN); // Black loses on time
            emit gameMessage("Black ran out of time!");
        }
    }
    emit updateClockDisplay(m_whiteTime, m_blackTime);
}

void GameManager::handleGameOverResult(int result)
{
    qInfo() << QString("GameManager: Game Over result received: %1").arg(result);
    switch (result) {
        case CB_WIN:
            strncpy(m_currentPdnGame.game.resultstring, "1-0", sizeof(m_currentPdnGame.game.resultstring) - 1);
            break;
        case CB_LOSS:
            strncpy(m_currentPdnGame.game.resultstring, "0-1", sizeof(m_currentPdnGame.game.resultstring) - 1);
            break;
        case CB_DRAW:
            strncpy(m_currentPdnGame.game.resultstring, "1/2-1/2", sizeof(m_currentPdnGame.game.resultstring) - 1);
            break;
        default:
            strncpy(m_currentPdnGame.game.resultstring, "*", sizeof(m_currentPdnGame.game.resultstring) - 1); // Unknown or ongoing
            break;
    }
    m_currentPdnGame.game.resultstring[sizeof(m_currentPdnGame.game.resultstring) - 1] = '\0';
    qDebug() << QString("GameManager: Updated PDN game result string to: %1").arg(m_currentPdnGame.game.resultstring);
}




int GameManager::read_match_stats_internal() {
    QString filename = emstats_filename_internal();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << QString("Could not open match stats file for reading: %1").arg(filename);
        return 0;
    }

    QTextStream in(&file);
    int lastGameNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("GameNumber:")) {
            lastGameNumber = line.section(':', 1).trimmed().toInt();
        }
    }
    file.close();
    return lastGameNumber;
}
void GameManager::reset_match_stats_internal() {
    QString filename = emstats_filename_internal();
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qCritical() << QString("Could not open match stats file for resetting: %1").arg(filename);
        return;
    }
    file.close();
    qInfo() << QString("Match stats file reset: %1").arg(filename);
}
void GameManager::update_match_stats_internal(int result, int movecount, int gamenumber, emstats_t *stats) {
    QString filename = emstats_filename_internal();
    QFile file(filename);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qCritical() << QString("Could not open match stats file for appending: %1").arg(filename);
        return;
    }

    QTextStream out(&file);
    out << "GameNumber: " << gamenumber << ", Result: " << result << ", Moves: " << movecount << "\n";
    file.close();
    qInfo() << QString("Match stats updated for game: %1").arg(gamenumber);
}
int GameManager::num_ballots_internal() {
     qDebug() << "GameManager: num_ballots_internal called (placeholder)";
     return 174; // Default 3-move count
}
int GameManager::game0_to_ballot0_internal(int game0) {
     qDebug() << QString("GameManager: game0_to_ballot0_internal called (placeholder) for game: %1").arg(game0);
     return game0 % this->num_ballots_internal(); // Simple placeholder: cycle through ballots
}
int GameManager::game0_to_match0_internal(int game0) {
     qDebug() << QString("GameManager: game0_to_match0_internal called (placeholder) for game: %1").arg(game0);
     return game0 / 2; // Simple placeholder: every two games is a match
}
QString GameManager::emstats_filename_internal() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataLocation + "/match_stats.txt";
}
QString GameManager::emprogress_filename_internal() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataLocation + "/match_progress.txt";
}
QString GameManager::empdn_filename_internal() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataLocation + "/match.pdn";
}
QString GameManager::emlog_filename_internal() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataLocation + "/match_log.txt";
}
void GameManager::quick_search_both_engines_internal() {
    qDebug() << "GameManager: quick_search_both_engines_internal called.";
    // This would typically involve emitting signals to the AI class(es) to initiate a search.
    // For now, let's assume we want to request a move for the current board state and color.
    // The time limit here is a placeholder and should be determined by game options.
    emit requestEngineSearch(m_currentBoard, m_currentColorToMove, m_options.time_per_move); // Use m_options.time_per_move
}

void GameManager::savePdnGame(const QString &filename) {
    qInfo() << QString("GameManager: Saving PDN game to: %1").arg(filename);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << QString("GameManager: Could not open file for writing: %1 %2").arg(filename).arg(file.errorString());
        emit gameMessage(QString("Failed to save game to %1.").arg(filename));
        g_programStatusWord |= STATUS_FILE_IO_ERROR;
        return;
    }

    QTextStream out(&file);
    out << "[Event \"" << m_currentPdnGame.game.event << "]\n";
    out << "[Site \"" << m_currentPdnGame.game.site << "]\n";
    out << "[Date \"" << m_currentPdnGame.game.date << "]\n";
    out << "[Round \"" << m_currentPdnGame.game.round << "]\n";
    out << "[White \"" << m_currentPdnGame.game.white << "]\n";
    out << "[Black \"" << m_currentPdnGame.game.black << "]\n";
    out << "[Result \"" << m_currentPdnGame.game.resultstring << "]\n";
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        out << "[FEN \"" << m_currentPdnGame.game.FEN << "]\n";
    }
    out << "\n";

    int moveNumber = 1;
    
    // Determine starting color from FEN, default to Black if no FEN
    int turn = CB_BLACK;
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        if (toupper(m_currentPdnGame.game.FEN[0]) == 'W') {
            turn = CB_WHITE;
        }
    }

    for (const CBmove& cbMove : m_currentPdnGame.moves) {
        if (turn == CB_BLACK) {
            out << moveNumber << ". ";
        }

        char move_notation[80];
        move4tonotation(&cbMove, move_notation);
        out << move_notation << " ";

        if (strlen(cbMove.comment) > 0) {
            out << "{" << cbMove.comment << "} ";
        }

        if (turn == CB_WHITE) {
            moveNumber++;
        }
        turn = (turn == CB_BLACK) ? CB_WHITE : CB_BLACK;
    }
    out << m_currentPdnGame.game.resultstring << "\n";

    file.close();
    emit gameMessage(QString("Game saved to %1.").arg(filename));
    qInfo() << "GameManager: PDN game saved.";
    g_programStatusWord |= STATUS_GAME_SAVE_PDN_OK;
}

QString GameManager::getFenPosition() {
    char fen_c[256];
    board8toFEN(&m_currentBoard, fen_c, m_currentColorToMove, GT_ENGLISH); // Assuming GT_ENGLISH for now
    QString fenString = QString::fromUtf8(fen_c);
    return fenString;
}

void GameManager::loadFenPosition(const QString& fen) {
    qInfo() << QString("GameManager: Loading FEN position: %1").arg(fen);
    int color_to_move;
    QString trimmedFen = fen.trimmed(); // Trim whitespace
    QByteArray ba = trimmedFen.toUtf8();
    char* fen_copy = ba.data();
    if (FENtoboard8(&m_currentBoard, fen_copy, &color_to_move, GT_ENGLISH) == 1) {
        m_currentColorToMove = color_to_move;
        emit boardUpdated(m_currentBoard);
        emit gameMessage("FEN position loaded successfully.");
    } else {
        emit gameMessage("Failed to load FEN position.");
        qCritical() << QString("GameManager: Failed to load FEN position: %1").arg(trimmedFen);
    }
}

void GameManager::start3MoveGame(int opening_index) {
    qInfo() << QString("GameManager: start3MoveGame called with opening index: %1").arg(opening_index);
    start3move_c(&m_currentBoard, opening_index);
    m_currentColorToMove = CB_WHITE; // Assuming white always starts after 3-move opening
    emit boardUpdated(m_currentBoard);
    emit gameMessage(QString("Started 3-move game with opening %1.").arg(opening_index));
}

void GameManager::setCurrentPdnGame(const PDNgame& gameData) {
    m_currentPdnGame.game = gameData;
}

const PdnGameWrapper& GameManager::getCurrentPdnGame() const {
    return m_currentPdnGame;
}

void GameManager::setOptions(const CBoptions& options) {
    m_options = options;
}

void GameManager::setSoundEnabled(bool enabled)
{
    m_options.sound = enabled;
    // No other actions needed here, as the MainWindow handles playing sounds
    // based on this option.
}

void GameManager::setCurrentBoard(const Board8x8& board) {
    m_currentBoard = board;
    emit boardUpdated(m_currentBoard);
}

void GameManager::setCurrentColorToMove(int color) {
    m_currentColorToMove = color;
}

Board8x8 GameManager::getCurrentBoard() const { return m_currentBoard; }
int GameManager::getCurrentPlayer() const { return m_currentColorToMove; }
int GameManager::getHalfMoveCount() const { return m_halfMoveCount; }
CBoptions GameManager::getOptions() const { return m_options; }

void GameManager::clearBoard() {
    qInfo() << "GameManager: Clearing board.";
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            m_currentBoard.board[y][x] = CB_EMPTY;
        }
    }
    m_currentColorToMove = CB_WHITE;
    emit boardUpdated(m_currentBoard);
    emit gameMessage("Board cleared.");
}

void GameManager::setPiece(int x, int y, int pieceType) {
    qInfo() << QString("GameManager: Setting piece at %1,%2 to %3").arg(x).arg(y).arg(pieceType);
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        m_currentBoard.board[y][x] = pieceType;
        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Piece set at %1,%2 to %3.").arg(x).arg(y).arg(pieceType));
    } else {
        qWarning() << QString("GameManager: Invalid coordinates for setPiece: %1,%2").arg(x).arg(y);
    }
}

void GameManager::togglePieceColor(int x, int y) {
    qInfo() << QString("GameManager: Toggling piece color at %1,%2").arg(x).arg(y);
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        int piece = m_currentBoard.board[y][x];
        if (piece == (CB_WHITE | CB_MAN)) {
            m_currentBoard.board[y][x] = (CB_BLACK | CB_MAN);
        } else if (piece == (CB_WHITE | CB_KING)) {
            m_currentBoard.board[y][x] = (CB_BLACK | CB_KING);
        } else if (piece == (CB_BLACK | CB_MAN)) {
            m_currentBoard.board[y][x] = (CB_WHITE | CB_MAN);
        } else if (piece == (CB_BLACK | CB_KING)) {
            m_currentBoard.board[y][x] = (CB_WHITE | CB_KING);
        } else {
            emit gameMessage(QString("No piece to toggle at %1,%2.").arg(x).arg(y));
            return;
        }
        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Piece color toggled at %1,%2.").arg(x).arg(y));
    } else {
        qWarning() << QString("GameManager: Invalid coordinates for togglePieceColor: %1,%2").arg(x).arg(y);
    }
}

int GameManager::getGameType() const
{
    return m_options.gametype;
}

int GameManager::getTotalMoves() const
{
    return m_currentPdnGame.moves.size();
}

CBmove GameManager::getLastMove() const
{
    return m_lastMove;
}

void GameManager::handleSquareClick(int x, int y)
{
    // qDebug() << QString("GameManager: User clicked on (%1) Current player: %2, Piece selected: %3, Forced capture: %4")
    //              .arg(coorstonumber(x, y, GT_ENGLISH))
    //              .arg((m_currentColorToMove == CB_WHITE) ? "white" : "black")
    //              .arg(m_pieceSelected ? "true" : "false")
    //              .arg(m_forcedCapturePending ? "true" : "false");

    // If it's an AI's turn, ignore human clicks
    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
        qDebug() << "GameManager: Ignoring human click during AI turn.";
        emit gameMessage("It's the AI's turn to move.");
        return;
    }

    int square = coorstonumber(x, y, GT_ENGLISH);
    // qDebug() << QString("GameManager: Square clicked: %1").arg(square);

    // Get all legal moves for the current player to determine if a capture is mandatory
    CBmove allLegalMoves[MAXMOVES];
    int all_nmoves = 0;
    int all_isjump = 0;
    bool dummy_can_continue_multijump_check = false;
    get_legal_moves_c(&m_currentBoard, m_currentColorToMove, allLegalMoves, &all_nmoves, &all_isjump, NULL, &dummy_can_continue_multijump_check);

    // Get the piece at the clicked square
    int piece = m_currentBoard.board[y][x];
    bool isCurrentPlayersPiece = false;
    if (m_currentColorToMove == CB_WHITE) {
        isCurrentPlayersPiece = (piece == (CB_WHITE | CB_MAN) || piece == (CB_WHITE | CB_KING));
    }
    else {
        isCurrentPlayersPiece = (piece == (CB_BLACK | CB_MAN) || piece == (CB_BLACK | CB_KING));
    }

    if (!m_pieceSelected) {
        // No piece selected yet, try to select one
        if (isCurrentPlayersPiece) {
            // If there are mandatory jumps, ensure the selected piece can make one
            if (all_isjump) {
                bool selectedPieceCanCapture = false;
                for (int i = 0; i < all_nmoves; ++i) {
                    if (allLegalMoves[i].from.x == x && allLegalMoves[i].from.y == y && allLegalMoves[i].is_capture) {
                        selectedPieceCanCapture = true;
                        break;
                    }
                }
                if (!selectedPieceCanCapture) {
                    emit gameMessage("You must select a piece that can make a capture!");
                    return;
                }
            }
            m_selectedX = x;
            m_selectedY = y;
            m_pieceSelected = true;
            emit gameMessage(QString("Piece selected at square %1").arg(square));
            emit pieceSelected(x, y);
        } else {
            emit gameMessage("Please select your own piece.");
        }
    } else {
        // A piece is already selected, this click is for a destination
        if (x == m_selectedX && y == m_selectedY) {
            // Clicked the same piece again, deselect it
            m_pieceSelected = false;
            m_selectedX = -1;
            m_selectedY = -1;
            emit gameMessage("Piece deselected.");
            emit pieceDeselected();
        } else {
            // Attempt to make a move
            CBmove potentialMove;
            potentialMove.from.x = m_selectedX;
            potentialMove.from.y = m_selectedY;
            potentialMove.to.x = x;
            potentialMove.to.y = y;

            CBmove legalMoves[MAXMOVES];
            int nmoves_val = 0;
            int isjump_val = 0;
            bool can_continue_multijump = false;
            
            CBmove *last_move_ptr = m_forcedCapturePending ? &m_lastMove : NULL;
            get_legal_moves_c(&m_currentBoard, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, last_move_ptr, &can_continue_multijump);

            bool moveFound = false;
            CBmove chosenMove;
            for (int i = 0; i < nmoves_val; ++i) {
                if (legalMoves[i].from.x == potentialMove.from.x &&
                    legalMoves[i].from.y == potentialMove.from.y &&
                    legalMoves[i].to.x == potentialMove.to.x &&
                    legalMoves[i].to.y == potentialMove.to.y) {
                    chosenMove = legalMoves[i];
                    moveFound = true;
                    break;
                }
            }


            if (moveFound) {
                qDebug() << "GameManager: User selected move " << QString("(%1-%2)").arg(coorstonumber(chosenMove.from.x, chosenMove.from.y, GT_ENGLISH)).arg(coorstonumber(chosenMove.to.x, chosenMove.to.y, GT_ENGLISH));
                bool isCapture = chosenMove.jumps > 0;
                makeMove(chosenMove);

                if (isCapture) {
                    // After a capture, check if the same piece can make another jump
                    CBmove multiJumpMoves[MAXMOVES];
                    int n_multijump_moves = 0;
                    int is_multijump = 0;
                    bool can_continue_this_jump = false;
                    get_legal_moves_c(&m_currentBoard, m_currentColorToMove, multiJumpMoves, &n_multijump_moves, &is_multijump, &chosenMove, &can_continue_this_jump);

                    if (can_continue_this_jump) {
                        // Multi-jump is possible and mandatory.
                        m_pieceSelected = true; // Keep piece selected
                        m_selectedX = x;        // Update selection to the new square
                        m_selectedY = y;
                        emit pieceSelected(x, y); // Highlight the piece for the next jump
                        emit gameMessage("Forced capture! Make your next jump.");
                        return; // End turn processing here, wait for next click
                    }
                }

                // If we reach here, the move sequence is over (either not a capture or no more multi-jumps).
                m_forcedCapturePending = false;
                m_pieceSelected = false;
                m_selectedX = -1;
                m_selectedY = -1;
                emit pieceDeselected();

                int nextColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;

                // Check for game over
                // qDebug() << "GameManager: Before get_legal_moves_c for next player.";
                CBmove nextPlayerMoves[MAXMOVES];
                int n_next_moves = 0;
                int is_next_jump = 0;
                get_legal_moves_c(&m_currentBoard, nextColorToMove, nextPlayerMoves, &n_next_moves, &is_next_jump, NULL, NULL);
                // qDebug() << QString("GameManager: After get_legal_moves_c for next player. n_next_moves: %1").arg(n_next_moves);

                if (n_next_moves == 0) {
                    emit gameIsOver(CB_WIN); // Current player wins
                    emit gameMessage("Game Over!");
                } else {
                    // Switch turns
                    // qDebug() << "Switching turns. Current color:" << m_currentColorToMove;
                    m_currentColorToMove = nextColorToMove;
                    // qDebug() << "New current color:" << m_currentColorToMove;
                    emit gameMessage("Move made.");

                    // Trigger AI if it's its turn
                    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
                        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
                        playMove();
                    } else {
                        qDebug() << "It's human's turn.";
                        emit humanTurn();
                    }
                }

            } else {
                // Move was not found in the legal list
                emit gameMessage("Illegal move. Please try again.");
                g_programStatusWord |= STATUS_INVALID_MOVE; // Set status flag for invalid move
                // Do not deselect the piece if a capture is forced, to guide the user.
                if (!m_forcedCapturePending) {
                    m_pieceSelected = false;
                    m_selectedX = -1;
                    m_selectedY = -1;
                    emit pieceDeselected();
                }
            }
        }
    }
}

void GameManager::playMove() {
    qDebug() << "GameManager: AI move requested.";
    // Emit a signal to the AI to request a move
    emit requestEngineSearch(m_currentBoard, m_currentColorToMove, m_options.time_per_move);
    emit gameMessage("AI is thinking...");
}

void GameManager::switchTurn()
{
    m_currentColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    if (m_currentColorToMove != m_engineColor) {
        emit humanTurn();
    }
}

void GameManager::reconstructBoardState(int move_index) {
    qDebug() << QString("Reconstructing board state to move index %1").arg(move_index);

    // 1. Reset board to initial state
    newgame(&m_currentBoard);
    int initial_color = CB_BLACK;
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        FENtoboard8(&m_currentBoard, m_currentPdnGame.game.FEN, &initial_color, m_currentPdnGame.game.gametype);
    }
    m_currentColorToMove = initial_color;

    // 2. Apply moves up to the target index
    for (int i = 0; i < move_index; ++i) {
        const CBmove& cbMove = m_currentPdnGame.moves[i];
        domove_c(&cbMove, &m_currentBoard);
        m_currentColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    }

    // 3. Update m_currentPdnGame.game.movesindex to reflect the reconstructed state
    m_currentPdnGame.game.movesindex = move_index;

    // 4. Update the UI
    emit boardUpdated(m_currentBoard);
}

void GameManager::goBack() {
    qDebug() << "GameManager: goBack called.";
    if (m_currentPdnGame.game.movesindex > 0) {
        // Get the last move from the history
        const CBmove& lastMove = m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1];

        // Unmake the move on the current board
        unmake_move_c(&lastMove, &m_currentBoard);

        // Decrement the move index
        m_currentPdnGame.game.movesindex--;

        // Update whose turn it is by looking at the color of the piece that was moved
        m_currentColorToMove = (lastMove.oldpiece & CB_WHITE) ? CB_WHITE : CB_BLACK;

        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Moved back to move %1.").arg(m_currentPdnGame.game.movesindex));
    } else {
        emit gameMessage("Already at the beginning of the game.");
    }
}

void GameManager::goForward() {
    qDebug() << "GameManager: goForward called.";
    if (m_currentPdnGame.game.movesindex < m_currentPdnGame.moves.size()) {
        // Get the next move from the history
        const CBmove& nextMove = m_currentPdnGame.moves[m_currentPdnGame.game.movesindex];

        // Apply the move
        domove_c(&nextMove, &m_currentBoard);

        // Increment the move index
        m_currentPdnGame.game.movesindex++;

        // Update whose turn it is
        m_currentColorToMove = (nextMove.oldpiece & CB_WHITE) ? CB_BLACK : CB_WHITE;

        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Moved forward to move %1.").arg(m_currentPdnGame.game.movesindex));
    } else {
        emit gameMessage("Already at the end of the game.");
    }
}

void GameManager::goBackAll() {
    qDebug() << "GameManager: goBackAll called.";
    m_currentPdnGame.game.movesindex = 0;
    reconstructBoardState(m_currentPdnGame.game.movesindex);
    emit gameMessage("Moved to the beginning of the game.");
}

void GameManager::goForwardAll() {
    qDebug() << "GameManager: goForwardAll called.";
    m_currentPdnGame.game.movesindex = m_currentPdnGame.moves.size();
    reconstructBoardState(m_currentPdnGame.game.movesindex);
    emit gameMessage("Moved to the end of the game.");
}

void GameManager::sendGameHistory() {
    qDebug() << "GameManager: sendGameHistory called.";
    std::string pdn_history_str;
    // PDNgametoPDNstring(m_currentPdnGame, pdn_history_str, "\n");
    QString history = QString::fromStdString(pdn_history_str);
    qDebug() << history;

    // Emit the PDN history as a command to the engine.
    // The MainWindow will connect this signal to the AI's sendCommand slot.
    emit sendEngineCommand(QString("setgamerecord %1").arg(history)); // Corrected emit statement
    emit gameMessage("Game history sent (logged to debug and sent to engine via signal).");
}

void GameManager::detectDraws() {
    qDebug() << "GameManager: detectDraws called.";

    // Three-fold repetition check
    QString currentFen = getFenPosition();
    int repetitionCount = 0;
    for (const QString& fen : m_boardHistory) {
        if (fen == currentFen) {
            repetitionCount++;
        }
    }

    if (repetitionCount >= 3) {
        emit gameMessage("Draw by three-fold repetition!");
        emit gameIsOver(CB_DRAW);
        return;
    }

    // 50-move rule
    if (m_halfMoveCount >= 100) { // 100 half-moves = 50 full moves
        emit gameMessage("Draw by 50-move rule!");
        emit gameIsOver(CB_DRAW);
        return;
    }

    // Insufficient material (basic check: only kings remain)
    int whitePieces = 0;
    int blackPieces = 0;
    bool whiteHasMan = false;
    bool blackHasMan = false;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = m_currentBoard.board[r][c];
            if (piece != CB_EMPTY) {
                if (piece & CB_WHITE) {
                    whitePieces++;
                    if (piece & CB_MAN) whiteHasMan = true;
                } else {
                    blackPieces++;
                    if (piece & CB_MAN) blackHasMan = true;
                }
            }
        }
    }

    // If only kings are left, it's a draw
    if (!whiteHasMan && !blackHasMan && whitePieces > 0 && blackPieces > 0) {
        emit gameMessage("Draw by insufficient material (only kings remain)!");
        emit gameIsOver(CB_DRAW);
        return;
    }

    emit gameMessage("Draw detection logic applied (three-fold repetition, 50-move rule, basic insufficient material).");
}

void GameManager::addComment(const QString& comment) {
    qDebug() << QString("GameManager: addComment called with: %1").arg(comment);
    if (m_currentPdnGame.game.movesindex > 0 && m_currentPdnGame.game.movesindex <= m_currentPdnGame.moves.size()) {
        // Get a reference to the move to modify it
        CBmove &move_to_comment = m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1];

        // Add comment to the current move
        strncpy(move_to_comment.comment,
                comment.toUtf8().constData(),
                sizeof(move_to_comment.comment) - 1);
        move_to_comment.comment[sizeof(move_to_comment.comment) - 1] = '\0';
        emit gameMessage("Comment added to current move.");
    } else {
        emit gameMessage("Cannot add comment: No move selected or game not started.");
    }
}

void GameManager::setTimeContol(int level, bool exact_time, bool use_incremental_time, int initial_time, int time_increment) {
    qInfo() << QString("GameManager: Setting time control - Level: %1, Exact Time: %2, Incremental: %3, Initial Time: %4, Increment: %5. AI Time per move set to: %6").arg(level).arg(exact_time).arg(use_incremental_time).arg(initial_time).arg(time_increment).arg(m_options.time_per_move);

    m_options.level = level;
    m_options.exact_time = exact_time;
    m_options.use_incremental_time = use_incremental_time;
    m_options.initial_time = initial_time;
    m_options.time_increment = time_increment;

    // Clear existing AI time setting bits and set new ones
    g_programStatusWord &= ~STATUS_AI_TIME_SETTING_MASK;
    g_programStatusWord |= ((uint32_t)level << STATUS_AI_TIME_SETTING_BIT_POS);

    // Set AI's time_per_move based on the level
    switch (level) {
        case LEVEL_INSTANT: m_options.time_per_move = 0.001; break; // Effectively instant
        case LEVEL_1S: m_options.time_per_move = 1.0; break;
        case LEVEL_01S: m_options.time_per_move = 0.1; break;
        case LEVEL_02S: m_options.time_per_move = 0.2; break;
        case LEVEL_05S: m_options.time_per_move = 0.5; break;
        case LEVEL_2S: m_options.time_per_move = 2.0; break;
        case LEVEL_5S: m_options.time_per_move = 5.0; break;
        case LEVEL_10S: m_options.time_per_move = 10.0; break;
        case LEVEL_15S: m_options.time_per_move = 15.0; break;
        case LEVEL_30S: m_options.time_per_move = 30.0; break;
        case LEVEL_1M: m_options.time_per_move = 60.0; break;
        case LEVEL_2M: m_options.time_per_move = 120.0; break;
        case LEVEL_5M: m_options.time_per_move = 300.0; break;
        case LEVEL_15M: m_options.time_per_move = 900.0; break;
        case LEVEL_30M: m_options.time_per_move = 1800.0; break;
        case LEVEL_INFINITE: m_options.time_per_move = 3600.0; break; // A very large number for "infinite"
        default: m_options.time_per_move = 2.0; break; // Default to 2 seconds
    }

    qInfo() << QString("GameManager: AI Time per move set to: %1").arg(m_options.time_per_move);
    emit gameMessage("Time control settings updated.");
}

void GameManager::addTimeToClock(int seconds) {
    qDebug() << QString("GameManager: Adding %1 seconds to clock.").arg(seconds);
    if (m_currentColorToMove == CB_WHITE) {
        m_whiteTime += seconds;
    } else {
        m_blackTime += seconds;
    }
    emit gameMessage(QString("Added %1 seconds to %2's clock. White: %3s, Black: %4s.")
                     .arg(seconds)
                     .arg((m_currentColorToMove == CB_WHITE) ? "White" : "Black")
                     .arg(m_whiteTime)
                     .arg(m_blackTime));
}

void GameManager::subtractFromClock(int seconds) {
    qDebug() << QString("GameManager: Subtracting %1 seconds from clock.").arg(seconds);
    if (m_currentColorToMove == CB_WHITE) {
        m_whiteTime -= seconds;
        if (m_whiteTime < 0) m_whiteTime = 0;
    } else {
        m_blackTime -= seconds;
        if (m_blackTime < 0) m_blackTime = 0;
    }
    emit gameMessage(QString("Subtracted %1 seconds from %2's clock. White: %3s, Black: %4s.")
                     .arg(seconds)
                     .arg((m_currentColorToMove == CB_WHITE) ? "White" : "Black")
                     .arg(m_whiteTime)
                     .arg(m_blackTime));
}

void GameManager::handleEvaluationUpdate(int score, int depth)
{
    emit evaluationUpdated(score, depth);
}

void GameManager::handleAIMoveFound(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime)
{
    qDebug() << "GameManager::handleAIMoveFound - Move found:" << moveFound << ", Aborted:" << aborted << ", Best Move:" << bestMove.from.x << "," << bestMove.from.y << " to " << bestMove.to.x << "," << bestMove.to.y << ", Status:" << statusText;
    qDebug() << "GameManager::handleAIMoveFound - Received bestMove: from(" << coorstonumber(bestMove.from.x, bestMove.from.y, GT_ENGLISH) << ") to(" << coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) << ") is_capture:" << bestMove.is_capture << " jumps:" << bestMove.jumps;
    
    if (moveFound && !aborted) {
        // Apply the AI's best move
        makeMove(bestMove);
        detectDraws(); // Check for draws after AI's move
        
        // After making the move, switch turn and trigger AI if it's still AI's turn (e.g., multi-jump)
        // Or if it's the other AI's turn (in AI vs AI mode)
        int nextColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;

        CBmove nextPlayerMoves[MAXMOVES];
        int n_next_moves = 0;
        int is_next_jump = 0;
        bool dummy_can_continue_multijump = false;
        get_legal_moves_c(&m_currentBoard, nextColorToMove, nextPlayerMoves, &n_next_moves, &is_next_jump, NULL, &dummy_can_continue_multijump);

        qDebug() << "GameManager::handleAIMoveFound - Next player (" << nextColorToMove << ") has " << n_next_moves << " legal moves. Is jump: " << is_next_jump;


        if (n_next_moves == 0) {
            emit gameIsOver(CB_WIN); // Current player wins, as opponent has no moves
            emit gameMessage("Game Over! No legal moves for opponent.");
        } else {
            m_currentColorToMove = nextColorToMove;
            emit gameMessage("AI made its move.");

            if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
                (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
                // Use a timer to yield to the event loop, keeping the GUI responsive
                QTimer::singleShot(50, this, &GameManager::playMove);
            } else {
                emit humanTurn(); // It's human's turn now
            }
        }
    } else if (aborted) {
        gameMessage("AI search was aborted.");
    } else {
        gameMessage("AI could not find a move. Game might be over or an error occurred.");
        gameMessage("AI could not find a move. Game might be over or an error occurred.");
    }
}

bool GameManager::isLegalMove(const CBmove& move) {
    qDebug() << QString("GameManager: Checking if move from %1,%2 to %3,%4 is legal.").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y);

    CBmove legalMoves[MAXMOVES];
    int nmoves_val = 0;
    int isjump_val = 0;
    get_legal_moves_c(&m_currentBoard, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, NULL, NULL);

    for (int i = 0; i < nmoves_val; ++i) {
        if (legalMoves[i].from.x == move.from.x &&
            legalMoves[i].from.y == move.from.y &&
            legalMoves[i].to.x == move.to.x &&
            legalMoves[i].to.y == move.to.y) {
            qDebug() << "GameManager: Move is legal.";
            return true;
        }
    }

    qDebug() << "GameManager: Move is NOT legal.";
    return false;
}

void GameManager::resumePlay()
{
    qDebug() << "GameManager: Resuming play...";

    // Check if game is already over
    CBmove legalMoves[MAXMOVES];
    int nmoves = 0;
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&m_currentBoard, m_currentColorToMove, legalMoves, &nmoves, &isjump, NULL, &dummy_can_continue_multijump);
    if (nmoves == 0) {
        qDebug() << "GameManager::resumePlay - No legal moves. Game is over.";
        // Depending on whose turn it is, the other player wins.
        // For simplicity, we can just announce it. A more robust implementation
        // would emit gameIsOver with the correct winner.
        emit gameMessage("Game over - no legal moves.");
        return;
    }

    // Check if it's an AI's turn to move
    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
        qDebug() << "GameManager::resumePlay - It is an AI's turn. Triggering playMove().";
        playMove();
    } else {
        qDebug() << "GameManager::resumePlay - It is a human's turn.";
        emit humanTurn();
    }
}
