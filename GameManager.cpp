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
#include "checkers_c_types.h" // Direct include

extern LogLevel s_minLogLevel;

// Static member definitions
QFile GameManager::m_logFile;
QTextStream GameManager::m_logStream;
QMutex GameManager::m_logMutex;

void GameManager::initLogging()
{
    m_logFile.setFileName("/home/victor/Desktop/checkers/Programs/checkers_project/ResourceFiles/app.log");
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_logStream.setDevice(&m_logFile);
        // Direct write to file to test if QFile is working
        m_logFile.write("Direct write test: Logging initialized.\n");
        log(LogLevel::Info, "Logging initialized.");
    } else {
        GameManager::log(LogLevel::Error, "Failed to open log file.");
    }
}

void GameManager::closeLogging()
{
    log(LogLevel::Info, "Logging finished.");
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

static QString levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

extern LogLevel s_minLogLevel;

void GameManager::log(LogLevel level, const QString& message)
{
    if (level < s_minLogLevel) {
        return;
    }
    QMutexLocker locker(&m_logMutex);
    if (m_logFile.isOpen()) {
        m_logStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " [" << levelToString(level) << "]: " << message << Qt::endl;
    }
}

// C-compatible logging function
extern "C" void log_c(int level, const char* message)
{
    GameManager::log(static_cast<LogLevel>(level), QString::fromUtf8(message));
}

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


static int PDNparseMove(char *token, Squarelist &squares)
{
    squares.clear();
    const char *p = token;
    int num = 0;
    bool num_found = false;

    while (*p) {
        if (isdigit(*p)) {
            num = num * 10 + (*p - '0');
            num_found = true;
        } else if (num_found) {
            squares.append(num);
            num = 0;
            num_found = false;
        }
        p++;
    }

    if (num_found) {
        squares.append(num);
    }
    return squares.size(); // Add this line to return a value
}

static void PDNgametoPDNstring(PdnGameWrapper &game, std::string &pdnstring, const char *lineterm) {
    pdnstring.clear();

    pdnstring += "[Event \"" + std::string(game.game.event) + "]" + lineterm;
    pdnstring += "[Site \"" + std::string(game.game.site) + "]" + lineterm;
    pdnstring += "[Date \"" + std::string(game.game.date) + "]" + lineterm;
    pdnstring += "[Round \"" + std::string(game.game.round) + "]" + lineterm;
    pdnstring += "[White \"" + std::string(game.game.white) + "]" + lineterm;
    pdnstring += "[Black \"" + std::string(game.game.black) + "]" + lineterm;
    pdnstring += "[Result \"" + std::string(game.game.resultstring) + "]" + lineterm;

    if (strlen(game.game.FEN) > 0) {
        pdnstring += "[FEN \"" + std::string(game.game.FEN) + "]" + lineterm;
    }

    pdnstring += lineterm;

    for (size_t i = 0; i < game.moves.size(); ++i) {
        if (i % 2 == 0) {
            pdnstring += std::to_string((i / 2) + 1) + ". ";
        }
        // Reconstruct board state up to this move to determine capture and jumps
        Board8x8 tempBoard;
        newgame(&tempBoard); // Start with initial board
        for (size_t j = 0; j < i; ++j) {
            CBmove prevMove;
            numbertocoors(game.moves[j].from_square, &prevMove.from.x, &prevMove.from.y, game.game.gametype);
            numbertocoors(game.moves[j].to_square, &prevMove.to.x, &prevMove.to.y, game.game.gametype);
            // For previous moves, we assume is_capture and jumps were correctly determined or are not needed for board reconstruction
            prevMove.is_capture = false; // Placeholder for now
            prevMove.jumps = 0; // Placeholder for now
            domove_c(&prevMove, &tempBoard);
        }

        // Now, determine is_capture and jumps for the current move
        CBmove currentCbMove;
        numbertocoors(game.moves[i].from_square, &currentCbMove.from.x, &currentCbMove.from.y, game.game.gametype);
        numbertocoors(game.moves[i].to_square, &currentCbMove.to.x, &currentCbMove.to.y, game.game.gametype);

        CBmove legalMoves[MAXMOVES];
        int nmoves_val = 0;
        int isjump_val = 0;
        pos currentPos;
        boardtobitboard(&tempBoard, &currentPos);
        bool dummy_can_continue_multijump = false;
        get_legal_moves_c(&currentPos, (i % 2 == 0) ? CB_BLACK : CB_WHITE, legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

        bool moveFound = false;
        for (int k = 0; k < nmoves_val; ++k) {
            if (legalMoves[k].from.x == currentCbMove.from.x &&
                legalMoves[k].from.y == currentCbMove.from.y &&
                legalMoves[k].to.x == currentCbMove.to.x &&
                legalMoves[k].to.y == currentCbMove.to.y) {
                currentCbMove.is_capture = legalMoves[k].is_capture;
                currentCbMove.jumps = legalMoves[k].jumps;
                moveFound = true;
                break;
            }
        }
        if (!moveFound) {
            GameManager::log(LogLevel::Warning, QString("PDNgametoPDNstring: Could not find legal move for PDN move %1-%2. Defaulting is_capture=false, jumps=0.").arg(game.moves[i].from_square).arg(game.moves[i].to_square));
        }

        char move_notation[80];
        move4tonotation(&currentCbMove, move_notation);
        pdnstring += std::string(move_notation) + " ";

        if (strlen(game.moves[i].comment) > 0) {
            pdnstring += "{" + std::string(game.moves[i].comment) + "} ";
        }
    }
} // Closing brace for PDNgametoPDNstring

static int PDNparseGetnumberofgames(char *filename);

void GameManager::parsePdnGameString(char* game_str, PdnGameWrapper& game) {
    char game_str_copy[10000]; // Create a copy as strtok modifies the string
    strncpy(game_str_copy, game_str, sizeof(game_str_copy) - 1);
    game_str_copy[sizeof(game_str_copy) - 1] = '\0';

    char* line = strtok(game_str_copy, "\n");
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
                        if (strcmp(key_start, "Event") == 0) strcpy(game.game.event, value_start);
                        else if (strcmp(key_start, "Site") == 0) strcpy(game.game.site, value_start);
                        else if (strcmp(key_start, "Date") == 0) strcpy(game.game.date, value_start);
                        else if (strcmp(key_start, "Round") == 0) strcpy(game.game.round, value_start);
                        else if (strcmp(key_start, "White") == 0) strcpy(game.game.white, value_start);
                        else if (strcmp(key_start, "Black") == 0) strcpy(game.game.black, value_start);
                        else if (strcmp(key_start, "Result") == 0) strcpy(game.game.resultstring, value_start);
                        else if (strcmp(key_start, "FEN") == 0) strcpy(game.game.FEN, value_start);
                    }
                }
            }
        } else if (isdigit(line[0])) {
            // Parse moves
            char* move_token = strtok(line, " ");
            while (move_token != NULL) {
                if (strchr(move_token, '.') == NULL) { // Not a move number
                    PDNmove pdnMove;
                    char* dash = strchr(move_token, '-');
                    char* comment_start = strchr(move_token, '{');
                    char* comment_end = nullptr;

                    if (comment_start) {
                        *comment_start = '\0'; // Temporarily null-terminate to parse move before comment
                        comment_start++; // Move past '{'
                        comment_end = strchr(comment_start, '}');
                        if (comment_end) {
                            *comment_end = '\0'; // Null-terminate comment
                            strncpy(pdnMove.comment, comment_start, sizeof(pdnMove.comment) - 1);
                            pdnMove.comment[sizeof(pdnMove.comment) - 1] = '\0';
                        }
                    } else {
                        pdnMove.comment[0] = '\0'; // No comment
                    }

                    if (dash) {
                        *dash = '\0';
                        pdnMove.from_square = atoi(move_token);
                        pdnMove.to_square = atoi(dash + 1);
                        game.moves.push_back(pdnMove);
                    }
                }
                move_token = strtok(NULL, " ");
            }
        }
        line = strtok(NULL, "\n");
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

GameManager::GameManager(QObject *parent) : QObject(parent),
    m_pieceSelected(false),
    m_selectedX(-1),
    m_selectedY(-1),
    m_forcedCapturePending(false), // Initialize new flag
    m_engineColor(CB_WHITE), // Initialize AI engine color to WHITE
    m_whiteTime(0.0), // Initialize white's time
    m_blackTime(0.0), // Initialize black's time
    m_halfMoveCount(0), // Initialize half-move count
    m_gameTimer(new QTimer(this)) // Initialize game timer
{
    GameManager::log(LogLevel::Info, "GameManager: Initializing.");
    connect(m_gameTimer, &QTimer::timeout, this, &GameManager::handleTimerTimeout);
}


GameManager::~GameManager()
{
}

void GameManager::newGame(int gameType)
{
    GameManager::log(LogLevel::Info, QString("GameManager: Starting new game of type %1").arg(gameType));
    // Call C function to set up initial board
    newgame(&m_currentBoard);
    m_currentColorToMove = CB_BLACK; // Black typically starts
    m_halfMoveCount = 0; // Reset half-move count for new game
    m_boardHistory.clear(); // Clear board history for new game
    m_boardHistory.append(getFenPosition()); // Add initial position to history

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

    emit boardUpdated(m_currentBoard);
    emit gameMessage("New game started!");

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

bool GameManager::makeMove(const CBmove& move)
{
    GameManager::log(LogLevel::Debug, QString("GameManager: Making move from %1,%2 to %3,%4").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y));

    // Check if the move is a capture or a pawn move for 50-move rule
    bool isCapture = move.jumps > 0;
    bool isPawnMove = (move.oldpiece == (CB_WHITE | CB_MAN) || move.oldpiece == (CB_BLACK | CB_MAN));

    // 2. Apply the move
    domove_c(&move, &m_currentBoard);
    m_lastMove = move; // Update m_lastMove

    // If we are not at the end of the move history (i.e., some moves were undone),
    // discard all "future" moves before adding the new one.
    if (m_currentPdnGame.game.movesindex < m_currentPdnGame.moves.size()) {
        m_currentPdnGame.moves.erase(m_currentPdnGame.moves.begin() + m_currentPdnGame.game.movesindex, m_currentPdnGame.moves.end());
    }

    // Create a PDNmove from the CBmove
    PDNmove pdnMove;
    pdnMove.from_square = coorstonumber(move.from.x, move.from.y, m_currentPdnGame.game.gametype);
    pdnMove.to_square = coorstonumber(move.to.x, move.to.y, m_currentPdnGame.game.gametype);
    pdnMove.comment[0] = '\0'; // Initialize comment as empty

    // Add the move to the PDN game history
    m_currentPdnGame.moves.push_back(pdnMove);
    m_currentPdnGame.game.movesindex++;

    // Update half-move count for 50-move rule
    if (isCapture || isPawnMove) {
        m_halfMoveCount = 0;
    } else {
        m_halfMoveCount++;
    }

    // Add current board FEN to history for draw detection
    m_boardHistory.append(getFenPosition());

    // Stop and restart the timer for the next player
    if (m_options.enable_game_timer) {
        m_gameTimer->stop();
        m_gameTimer->start(1000);
    }

    // 4. Emit boardUpdated
    emit boardUpdated(m_currentBoard);

    return isCapture;
}

void GameManager::handleTimerTimeout()
{
    GameManager::log(LogLevel::Debug, "GameManager::handleTimerTimeout called.");
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

void GameManager::loadPdnGame(const QString &filename)
{
    std::vector<PdnGameWrapper> loadedGames;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        GameManager::log(LogLevel::Error, QString("Failed to open file: %1 %2").arg(filename).arg(file.errorString()));
        emit gameMessage(QString("Failed to load PDN file: %1").arg(filename));
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    QByteArray ba = fileContent.toUtf8();
    char* buffer = ba.data();
    char* current_pos = buffer;

    loadedGames.clear();

    char game_str[10000];
    while (PDNparseGetnextgame(&current_pos, game_str, sizeof(game_str))) {
        PdnGameWrapper game; // Create a new PdnGameWrapper object for each game
        parsePdnGameString(game_str, game); // Parse game_str into game object
        loadedGames.push_back(game);
    }
    GameManager::log(LogLevel::Info, QString("GameManager: Parsed %1 games from PDN file.").arg(loadedGames.size()));

    if (!loadedGames.empty()) {
        m_currentPdnGame = loadedGames[0]; // Load the first game
        GameManager::log(LogLevel::Info, QString("GameManager: Loading first game: %1").arg(m_currentPdnGame.game.event));

        // Apply starting FEN to the board
        if (strlen(m_currentPdnGame.game.FEN) > 0) {
            loadFenPosition(QString::fromUtf8(m_currentPdnGame.game.FEN));
        } else {
            // If no FEN, start a new standard game
            newGame(m_currentPdnGame.game.gametype);
        }

        // Apply all moves in the loaded game
        Board8x8 tempBoard; // Use a temporary board to reconstruct state for move validation
        newgame(&tempBoard); // Start with initial board
        if (strlen(m_currentPdnGame.game.FEN) > 0) {
            int color_to_move; // Dummy variable
            FENtoboard8(&tempBoard, m_currentPdnGame.game.FEN, &color_to_move, m_currentPdnGame.game.gametype);
        }

        int currentColor = CB_BLACK; // Assuming black starts for PDN moves unless FEN specifies otherwise
        if (strlen(m_currentPdnGame.game.FEN) > 0) {
            int color_to_move_from_fen; // Dummy variable
            FENtoboard8(&tempBoard, m_currentPdnGame.game.FEN, &color_to_move_from_fen, m_currentPdnGame.game.gametype);
            currentColor = color_to_move_from_fen;
        }

        for (const auto& pdnMove : m_currentPdnGame.moves) {
            CBmove cbMove;
            numbertocoors(pdnMove.from_square, &cbMove.from.x, &cbMove.from.y, m_currentPdnGame.game.gametype);
            numbertocoors(pdnMove.to_square, &cbMove.to.x, &cbMove.to.y, m_currentPdnGame.game.gametype);

            CBmove legalMoves[MAXMOVES];
            int nmoves_val = 0;
            int isjump_val = 0;
            pos currentPos;
            boardtobitboard(&tempBoard, &currentPos);
            bool dummy_can_continue_multijump = false;
            get_legal_moves_c(&currentPos, currentColor, legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

            bool moveFound = false;
            for (int k = 0; k < nmoves_val; ++k) {
                if (legalMoves[k].from.x == cbMove.from.x &&
                    legalMoves[k].from.y == cbMove.from.y &&
                    legalMoves[k].to.x == cbMove.to.x &&
                    legalMoves[k].to.y == cbMove.to.y) {
                    cbMove.is_capture = legalMoves[k].is_capture;
                    cbMove.jumps = legalMoves[k].jumps;
                    moveFound = true;
                    break;
                }
            }
            if (!moveFound) {
                GameManager::log(LogLevel::Warning, QString("loadPdnGame: Could not find legal move for PDN move %1-%2. Defaulting is_capture=false, jumps=0.").arg(pdnMove.from_square).arg(pdnMove.to_square));
            }

            domove_c(&cbMove, &tempBoard);
            currentColor = (currentColor == CB_WHITE) ? CB_BLACK : CB_WHITE;
        }
        m_currentBoard = tempBoard; // Set the main board to the final state of the loaded game
        m_currentColorToMove = currentColor; // Set the current color to move
        m_currentPdnGame.game.movesindex = m_currentPdnGame.moves.size(); // Set history to the end
        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Loaded game: %1").arg(m_currentPdnGame.game.event));
    }
    else {
        GameManager::log(LogLevel::Error, QString("GameManager: Failed to load PDN file: %1").arg(filename));
        emit gameMessage(QString("Failed to load PDN file: %1").arg(filename));
    }
}


int GameManager::read_match_stats_internal() {
    QString filename = emstats_filename_internal();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        GameManager::log(LogLevel::Error, QString("Could not open match stats file for reading: %1").arg(filename));
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
        GameManager::log(LogLevel::Error, QString("Could not open match stats file for resetting: %1").arg(filename));
        return;
    }
    file.close();
    GameManager::log(LogLevel::Info, QString("Match stats file reset: %1").arg(filename));
}
void GameManager::update_match_stats_internal(int result, int movecount, int gamenumber, emstats_t *stats) {
    QString filename = emstats_filename_internal();
    QFile file(filename);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        GameManager::log(LogLevel::Error, QString("Could not open match stats file for appending: %1").arg(filename));
        return;
    }

    QTextStream out(&file);
    out << "GameNumber: " << gamenumber << ", Result: " << result << ", Moves: " << movecount << "\n";
    file.close();
    GameManager::log(LogLevel::Info, QString("Match stats updated for game: %1").arg(gamenumber));
}
int GameManager::num_ballots_internal() {
     GameManager::log(LogLevel::Debug, "GameManager: num_ballots_internal called (placeholder)");
     return 174; // Default 3-move count
}
int GameManager::game0_to_ballot0_internal(int game0) {
     GameManager::log(LogLevel::Debug, QString("GameManager: game0_to_ballot0_internal called (placeholder) for game: %1").arg(game0));
     return game0 % this->num_ballots_internal(); // Simple placeholder: cycle through ballots
}
int GameManager::game0_to_match0_internal(int game0) {
     GameManager::log(LogLevel::Debug, QString("GameManager: game0_to_match0_internal called (placeholder) for game: %1").arg(game0));
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
    GameManager::log(LogLevel::Debug, "GameManager: quick_search_both_engines_internal called.");
    // This would typically involve emitting signals to the AI class(es) to initiate a search.
    // For now, let's assume we want to request a move for the current board state and color.
    // The time limit here is a placeholder and should be determined by game options.
    emit requestEngineSearch(m_currentBoard, m_currentColorToMove, m_options.time_per_move); // Use m_options.time_per_move
}

void GameManager::savePdnGame(const QString &filename) {
    GameManager::log(LogLevel::Info, QString("GameManager: Saving PDN game to: %1").arg(filename));
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        GameManager::log(LogLevel::Error, QString("GameManager: Could not open file for writing: %1 %2").arg(filename).arg(file.errorString()));
        emit gameMessage(QString("Failed to save game to %1.").arg(filename));
        return;
    }

    QTextStream out(&file);
    // Get actual game data (headers, moves) from current game state
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

    // Format moves into PDN string
    int moveNumber = 1;
    Board8x8 tempBoard; // Use a temporary board to reconstruct state for move validation
    newgame(&tempBoard); // Start with initial board
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        int color_to_move; // Dummy variable
        FENtoboard8(&tempBoard, m_currentPdnGame.game.FEN, &color_to_move, m_currentPdnGame.game.gametype);
    }

    int currentColor = CB_BLACK; // Assuming black starts for PDN moves unless FEN specifies otherwise
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        int color_to_move_from_fen; // Dummy variable
        FENtoboard8(&tempBoard, m_currentPdnGame.game.FEN, &color_to_move_from_fen, m_currentPdnGame.game.gametype);
        currentColor = color_to_move_from_fen;
    }

    for (size_t i = 0; i < m_currentPdnGame.moves.size(); ++i) {
        if (i % 2 == 0) { // White's move
            out << moveNumber << ". ";
            moveNumber++;
        }
        char pdn_c[40];
        CBmove cbMove;
        numbertocoors(m_currentPdnGame.moves[i].from_square, &cbMove.from.x, &cbMove.from.y, m_currentPdnGame.game.gametype);
        numbertocoors(m_currentPdnGame.moves[i].to_square, &cbMove.to.x, &cbMove.to.y, m_currentPdnGame.game.gametype);

        CBmove legalMoves[MAXMOVES];
        int nmoves_val = 0;
        int isjump_val = 0;
        pos currentPos;
        boardtobitboard(&tempBoard, &currentPos);
        bool dummy_can_continue_multijump = false;
        get_legal_moves_c(&currentPos, currentColor, legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

        bool moveFound = false;
        for (int k = 0; k < nmoves_val; ++k) {
            if (legalMoves[k].from.x == cbMove.from.x &&
                legalMoves[k].from.y == cbMove.from.y &&
                legalMoves[k].to.x == cbMove.to.x &&
                legalMoves[k].to.y == cbMove.to.y) {
                cbMove.is_capture = legalMoves[k].is_capture;
                cbMove.jumps = legalMoves[k].jumps;
                moveFound = true;
                break;
            }
        }
        if (!moveFound) {
            GameManager::log(LogLevel::Warning, QString("savePdnGame: Could not find legal move for PDN move %1-%2. Defaulting is_capture=false, jumps=0.").arg(m_currentPdnGame.moves[i].from_square).arg(m_currentPdnGame.moves[i].to_square));
        }

        char move_notation[80];
        move4tonotation(&cbMove, pdn_c);
        out << pdn_c << " "; // Corrected output

        if (strlen(m_currentPdnGame.moves[i].comment) > 0) {
            out << "{" << m_currentPdnGame.moves[i].comment << "} ";
        }
        domove_c(&cbMove, &tempBoard); // Apply move to tempBoard for next iteration
        currentColor = (currentColor == CB_WHITE) ? CB_BLACK : CB_WHITE; // Update color for next iteration
    }
    out << m_currentPdnGame.game.resultstring << "\n";

    file.close();
    emit gameMessage(QString("Game saved to %1.").arg(filename));
    GameManager::log(LogLevel::Info, "GameManager: PDN game saved.");
}

QString GameManager::getFenPosition() {
    char fen_c[256];
    board8toFEN(&m_currentBoard, fen_c, m_currentColorToMove, GT_ENGLISH); // Assuming GT_ENGLISH for now
    QString fenString = QString::fromUtf8(fen_c);
    GameManager::log(LogLevel::Debug, QString("GameManager: Generated FEN: %1").arg(fenString));
    return fenString;
}

void GameManager::loadFenPosition(const QString& fen) {
    GameManager::log(LogLevel::Info, QString("GameManager: Loading FEN position: %1").arg(fen));
    int color_to_move;
    if (FENtoboard8(&m_currentBoard, fen.toUtf8().constData(), &color_to_move, GT_ENGLISH) == 1) {
        m_currentColorToMove = color_to_move;
        emit boardUpdated(m_currentBoard);
        emit gameMessage("FEN position loaded successfully.");
    } else {
        emit gameMessage("Failed to load FEN position.");
        GameManager::log(LogLevel::Error, QString("GameManager: Failed to load FEN position: %1").arg(fen));
    }
}

void GameManager::start3MoveGame(int opening_index) {
    GameManager::log(LogLevel::Info, QString("GameManager: start3MoveGame called with opening index: %1").arg(opening_index));
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
    GameManager::log(LogLevel::Info, "GameManager: Clearing board.");
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
    GameManager::log(LogLevel::Info, QString("GameManager: Setting piece at %1,%2 to %3").arg(x).arg(y).arg(pieceType));
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        m_currentBoard.board[y][x] = pieceType;
        emit boardUpdated(m_currentBoard);
        emit gameMessage(QString("Piece set at %1,%2 to %3.").arg(x).arg(y).arg(pieceType));
    } else {
        GameManager::log(LogLevel::Warning, QString("GameManager: Invalid coordinates for setPiece: %1,%2").arg(x).arg(y));
    }
}

void GameManager::togglePieceColor(int x, int y) {
    GameManager::log(LogLevel::Info, QString("GameManager: Toggling piece color at %1,%2").arg(x).arg(y));
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
        GameManager::log(LogLevel::Warning, QString("GameManager: Invalid coordinates for togglePieceColor: %1,%2").arg(x).arg(y));
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
    // If it's an AI's turn, ignore human clicks
    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
        GameManager::log(LogLevel::Debug, "GameManager: Ignoring human click during AI turn.");
        emit gameMessage("It's the AI's turn to move.");
        return;
    }

    int square = coorstonumber(x, y, GT_ENGLISH);
    GameManager::log(LogLevel::Debug, QString("GameManager: Square clicked: %1").arg(square));

    // Get all legal moves for the current player to determine if a capture is mandatory
    CBmove allLegalMoves[MAXMOVES];
    int all_nmoves = 0;
    int all_isjump = 0;
    pos currentPosForMandatoryCheck;
    boardtobitboard(&m_currentBoard, &currentPosForMandatoryCheck);
    bool dummy_can_continue_multijump_check = false;
    get_legal_moves_c(&currentPosForMandatoryCheck, m_currentColorToMove, allLegalMoves, &all_nmoves, &all_isjump, NULL, &dummy_can_continue_multijump_check);

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
            // Clicked the same piece again, deselect it (only if not in a forced capture sequence)
            if (!m_forcedCapturePending) {
                m_pieceSelected = false;
                m_selectedX = -1;
                m_selectedY = -1;
                emit gameMessage("Piece deselected.");
                emit pieceDeselected();
            }
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
            pos currentPos;
            boardtobitboard(&m_currentBoard, &currentPos);
            bool can_continue_multijump = false;

            // If a forced capture is pending, we can only make jumps with the piece that just moved.
            // The legal move generator needs the last move to correctly identify subsequent jumps.
            if (m_forcedCapturePending) {
                get_legal_moves_c(&currentPos, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, &m_lastMove, &can_continue_multijump);
            } else {
                get_legal_moves_c(&currentPos, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, NULL, &can_continue_multijump);
            }

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
                bool isCapture = makeMove(chosenMove);

                // Check if a promotion happened
                bool promotionOccurred = (chosenMove.oldpiece & CB_MAN) && (chosenMove.newpiece & CB_KING);

                if (isCapture && !promotionOccurred) {
                    // After a capture, check if the same piece can make another jump
                    pos posAfterMove;
                    boardtobitboard(&m_currentBoard, &posAfterMove);
                    CBmove multiJumpMoves[MAXMOVES];
                    int n_multijump_moves = 0;
                    int is_multijump = 0;
                    bool can_continue_this_jump = false;
                    get_legal_moves_c(&posAfterMove, m_currentColorToMove, multiJumpMoves, &n_multijump_moves, &is_multijump, &m_lastMove, &can_continue_this_jump);

                    if (can_continue_this_jump) {
                        // Multi-jump is possible and mandatory.
                        m_forcedCapturePending = true;
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
                pos finalPos;
                boardtobitboard(&m_currentBoard, &finalPos);
                CBmove nextPlayerMoves[MAXMOVES];
                int n_next_moves = 0;
                int is_next_jump = 0;
                bool dummy_can_continue = false;
                get_legal_moves_c(&finalPos, nextColorToMove, nextPlayerMoves, &n_next_moves, &is_next_jump, NULL, &dummy_can_continue);

                if (n_next_moves == 0) {
                    emit gameIsOver(CB_WIN); // Current player wins
                    emit gameMessage("Game Over!");
                } else {
                    // Switch turns
                    m_currentColorToMove = nextColorToMove;
                    emit gameMessage("Move made.");

                    // Trigger AI if it's its turn
                    if ((m_currentColorToMove == CB_WHITE && m_options.white_player_type == PLAYER_AI) ||
                        (m_currentColorToMove == CB_BLACK && m_options.black_player_type == PLAYER_AI)) {
                        playMove();
                    } else {
                        emit humanTurn();
                    }
                }

            } else {
                // Move was not found in the legal list
                emit gameMessage("Illegal move. Please try again.");
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
    // Safeguard: Ensure it's an AI's turn based on options
    bool isWhiteAI = (m_options.white_player_type == PLAYER_AI);
    bool isBlackAI = (m_options.black_player_type == PLAYER_AI);

    if (!((m_currentColorToMove == CB_WHITE && isWhiteAI) || (m_currentColorToMove == CB_BLACK && isBlackAI))) {
        GameManager::log(LogLevel::Warning, QString("GameManager: playMove() called, but it is not an AI's turn. Current color: %1, White AI: %2, Black AI: %3. Ignoring.").arg(m_currentColorToMove).arg(isWhiteAI).arg(isBlackAI));
        return;
    }

    // Check if AI has any legal moves
    pos currentPos;
    boardtobitboard(&m_currentBoard, &currentPos);
    CBmove legalMovesForAI[MAXMOVES];
    int nmoves_for_ai = 0;
    int isjump_for_ai = 0;
    bool dummy_can_continue_multijump_for_ai = false;
    get_legal_moves_c(&currentPos, m_currentColorToMove, legalMovesForAI, &nmoves_for_ai, &isjump_for_ai, NULL, &dummy_can_continue_multijump_for_ai);

    if (nmoves_for_ai == 0) {
        // AI has no legal moves, human wins (or other AI wins in AI vs AI)
        int result;
        if (m_currentColorToMove == CB_WHITE) { // White AI has no moves
            result = (isBlackAI) ? CB_WIN : CB_LOSS; // If Black is AI, Black AI wins. Else, Human (Black) wins.
        } else { // Black AI has no moves
            result = (isWhiteAI) ? CB_WIN : CB_LOSS; // If White is AI, White AI wins. Else, Human (White) wins.
        }
        emit gameIsOver(result);
        emit gameMessage("Game Over! No legal moves for current player.");
        m_forcedCapturePending = false;
        return;
    }

    GameManager::log(LogLevel::Debug, "GameManager: playMove called. Requesting AI move.");
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
    GameManager::log(LogLevel::Debug, QString("Reconstructing board state to move index %1").arg(move_index));

    // 1. Reset board to initial state
    newgame(&m_currentBoard);
    int initial_color = CB_BLACK;
    if (strlen(m_currentPdnGame.game.FEN) > 0) {
        FENtoboard8(&m_currentBoard, m_currentPdnGame.game.FEN, &initial_color, m_currentPdnGame.game.gametype);
    }
    m_currentColorToMove = initial_color;

    // 2. Apply moves up to the target index
    for (int i = 0; i < move_index; ++i) {
        const PDNmove& pdnMove = m_currentPdnGame.moves[i];
        CBmove cbMove;
        numbertocoors(pdnMove.from_square, &cbMove.from.x, &cbMove.from.y, m_currentPdnGame.game.gametype);
        numbertocoors(pdnMove.to_square, &cbMove.to.x, &cbMove.to.y, m_currentPdnGame.game.gametype);

        // We need to determine if the move was a capture to apply it correctly.
        // This requires getting legal moves for the state *before* this move.
        CBmove legalMoves[MAXMOVES];
        int nmoves_val = 0;
        int isjump_val = 0;
        pos currentPos;
        bool dummy_can_continue_multijump = false;
        boardtobitboard(&m_currentBoard, &currentPos);
        get_legal_moves_c(&currentPos, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

        bool moveFound = false;
        for (int k = 0; k < nmoves_val; ++k) {
            if (legalMoves[k].from.x == cbMove.from.x &&
                legalMoves[k].from.y == cbMove.from.y &&
                legalMoves[k].to.x == cbMove.to.x &&
                legalMoves[k].to.y == cbMove.to.y) {
                cbMove = legalMoves[k]; // Use the full move data
                moveFound = true;
                break;
            }
        }
        if (!moveFound) {
             GameManager::log(LogLevel::Warning, QString("reconstructBoardState: Could not find legal move for PDN move %1-%2. Applying as non-capture.").arg(pdnMove.from_square).arg(pdnMove.to_square));
             cbMove.is_capture = 0;
             cbMove.jumps = 0;
        }

        domove_c(&cbMove, &m_currentBoard);
        m_currentColorToMove = (m_currentColorToMove == CB_WHITE) ? CB_BLACK : CB_WHITE;
    }

    // 3. Update m_currentPdnGame.game.movesindex to reflect the reconstructed state
    m_currentPdnGame.game.movesindex = move_index;

    // 4. Update the UI
    emit boardUpdated(m_currentBoard);
}

void GameManager::goBack() {
    GameManager::log(LogLevel::Debug, "GameManager: goBack called.");
    if (m_currentPdnGame.game.movesindex > 0) {
        m_currentPdnGame.game.movesindex--;
        reconstructBoardState(m_currentPdnGame.game.movesindex);
        emit gameMessage(QString("Moved back to move %1.").arg(m_currentPdnGame.game.movesindex));
    } else {
        emit gameMessage("Already at the beginning of the game.");
    }
}

void GameManager::goForward() {
    GameManager::log(LogLevel::Debug, "GameManager: goForward called.");
    if (m_currentPdnGame.game.movesindex < m_currentPdnGame.moves.size()) { 
        m_currentPdnGame.game.movesindex++;
        reconstructBoardState(m_currentPdnGame.game.movesindex);
        emit gameMessage(QString("Moved forward to move %1.").arg(m_currentPdnGame.game.movesindex));
    } else {
        emit gameMessage("Already at the end of the game.");
    }
}

void GameManager::goBackAll() {
    GameManager::log(LogLevel::Debug, "GameManager: goBackAll called.");
    m_currentPdnGame.game.movesindex = 0;
    reconstructBoardState(m_currentPdnGame.game.movesindex);
    emit gameMessage("Moved to the beginning of the game.");
}

void GameManager::goForwardAll() {
    GameManager::log(LogLevel::Debug, "GameManager: goForwardAll called.");
    m_currentPdnGame.game.movesindex = m_currentPdnGame.moves.size();
    reconstructBoardState(m_currentPdnGame.game.movesindex);
    emit gameMessage("Moved to the end of the game.");
}

void GameManager::sendGameHistory() {
    GameManager::log(LogLevel::Debug, "GameManager: sendGameHistory called.");
    // This function would typically format the game history (moves)
    // and send it to the AI engine, possibly via a signal.
    // For now, we'll just log the moves and send a placeholder command.
    std::string pdn_history_str;
    PDNgametoPDNstring(m_currentPdnGame, pdn_history_str, "\n");
    QString history = QString::fromStdString(pdn_history_str);
    GameManager::log(LogLevel::Debug, history);

    // Placeholder: Send a generic "gamehistory" command to the AI engine
    // A more robust implementation would involve a specific AI command for this.
    // QString reply;
    // if (m_ai->sendCommand(QString("gamehistory %1").arg(history), reply)) {
    //     emit gameMessage(QString("Game history sent to engine. Reply: %1").arg(reply));
    // } else {
    //     emit gameMessage("Failed to send game history to engine.");
    // }
    emit gameMessage("Game history sent (logged to debug and placeholder command).");
}

void GameManager::detectDraws() {
    GameManager::log(LogLevel::Debug, "GameManager: detectDraws called.");

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
    GameManager::log(LogLevel::Debug, QString("GameManager: addComment called with: %1").arg(comment));
    if (m_currentPdnGame.game.movesindex > 0 && m_currentPdnGame.game.movesindex <= m_currentPdnGame.moves.size()) {
        // Add comment to the current move
        strncpy(m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1].comment,
                comment.toUtf8().constData(),
                sizeof(m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1].comment) - 1);
        m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1].comment[sizeof(m_currentPdnGame.moves[m_currentPdnGame.game.movesindex - 1].comment) - 1] = '\0';
        emit gameMessage("Comment added to current move.");
    } else {
        emit gameMessage("Cannot add comment: No move selected or game not started.");
    }
}

void GameManager::setTimeContol(int level, bool exact_time, bool use_incremental_time, int initial_time, int time_increment) {
    GameManager::log(LogLevel::Info, QString("GameManager: Setting time control - Level: %1, Exact Time: %2, Incremental: %3, Initial Time: %4, Increment: %5. AI Time per move set to: %6").arg(level).arg(exact_time).arg(use_incremental_time).arg(initial_time).arg(time_increment).arg(m_options.time_per_move));

    m_options.level = level;
    m_options.exact_time = exact_time;
    m_options.use_incremental_time = use_incremental_time;
    m_options.initial_time = initial_time;
    m_options.time_increment = time_increment;

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
        default: m_options.time_per_move = 5.0; break; // Default to 5 seconds
    }

    GameManager::log(LogLevel::Info, QString("GameManager: AI Time per move set to: %1").arg(m_options.time_per_move));
    emit gameMessage("Time control settings updated.");
}

void GameManager::addTimeToClock(int seconds) {
    GameManager::log(LogLevel::Debug, QString("GameManager: Adding %1 seconds to clock.").arg(seconds));
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
    GameManager::log(LogLevel::Debug, QString("GameManager: Subtracting %1 seconds from clock.").arg(seconds));
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

bool GameManager::isLegalMove(const CBmove& move) {
    GameManager::log(LogLevel::Debug, QString("GameManager: Checking if move from %1,%2 to %3,%4 is legal.").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y));

    pos currentPos;
    boardtobitboard(&m_currentBoard, &currentPos);

    CBmove legalMoves[MAXMOVES];
    int nmoves_val = 0;
    int isjump_val = 0;
    bool dummy_can_continue_multijump = false;

    get_legal_moves_c(&currentPos, m_currentColorToMove, legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

    for (int i = 0; i < nmoves_val; ++i) {
        if (legalMoves[i].from.x == move.from.x &&
            legalMoves[i].from.y == move.from.y &&
            legalMoves[i].to.x == move.to.x &&
            legalMoves[i].to.y == move.to.y) {
            GameManager::log(LogLevel::Debug, "GameManager: Move is legal.");
            return true;
        }
    }

    GameManager::log(LogLevel::Debug, "GameManager: Move is NOT legal.");
    return false;
}