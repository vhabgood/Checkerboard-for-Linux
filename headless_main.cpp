#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "c_logic.h"
#include "checkers_types.h"
#include "AIWorker.h"
#include "DBManager.h"
#include "log.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread> // Include QThread header
#include "main_controller.h" // Include the new MainController header

// Function to read FEN from a file
std::string read_fen_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
    }
    std::string fen;
    std::getline(file, fen);
    return fen;
}

void print_move(const CBmove& move) {
    char notation[80];
    move4tonotation(&move, notation);
    log_c(LOG_LEVEL_INFO, "Move: %s", notation);
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    Logger::instance()->start();

    std::string fen_string;
    std::vector<std::string> args(argv + 1, argv + argc);

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--file" && i + 1 < args.size()) {
            fen_string = read_fen_from_file(args[++i]);
        } else if (args[i] == "--fen" && i + 1 < args.size()) {
            fen_string = args[++i];
        }
    }

    if (fen_string.empty()) {
        log_c(LOG_LEVEL_FATAL, "No FEN string provided. Use --file <filename> or --fen <fen_string>");
        Logger::instance()->stop();
        Logger::instance()->wait();
        return 1;
    }

    // Removed direct DBManager::db_init call here as it's handled by aiWorker->performInitialization
    // log_c(LOG_LEVEL_INFO, "Initializing DBManager...");
    // char db_out[256];
    // DBManager::instance()->db_init(0, db_out, "db/");

    log_c(LOG_LEVEL_INFO, "Initializing AI worker...");

    // Create a thread for the AI worker
    QThread aiThread;
    AIWorker *aiWorker = new AIWorker();
    aiWorker->moveToThread(&aiThread);

    // Create the MainController to emit signals to the AI worker
    MainController controller;
    
    // Connect signals/slots for AIWorker communication
    QObject::connect(&controller, &MainController::requestAiSearch, aiWorker, &AIWorker::performTask, Qt::QueuedConnection);

    // Explicitly start the AI thread
    aiThread.start();
    
    // Initialize EGDB path for the AIWorker once after the thread starts
    aiWorker->performInitialization("db/");
    
    // The thread cleanup is now handled at the end of main(), so this connect is no longer needed.
    // QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [&]() {
    //     aiWorker->requestAbort();
    //     aiThread.quit();
    //     aiThread.wait();
    //     delete aiWorker;
    // });

    // Dummy board for initialization of some parts of GameManager
    bitboard_pos board;
    int color_to_move;
    if (!FENtobitboard_pos(&board, fen_string.c_str(), &color_to_move, GT_ENGLISH)) {
        log_c(LOG_LEVEL_FATAL, "Failed to parse FEN string.");
        Logger::instance()->stop();
        Logger::instance()->wait();
        return 1;
    }
    board.color = color_to_move;

    int move_count = 0;
    const int max_moves = 100;

    CBmove best_move;
    bool move_found = false;
    int final_score = 0;
    int final_depth = 0;
    QString egdb_result_str;

    QObject::connect(aiWorker, &AIWorker::evaluationReady, [&](int score, int depth, const QString& egdbInfo) {
        // Log evaluation updates (optional, for debugging)
        log_c(LOG_LEVEL_DEBUG, "AIWorker: Evaluation update - Score: %d, Depth: %d, EGDB: %s", score, depth, egdbInfo.toUtf8().constData());
        final_score = score;
        final_depth = depth;
    });

    while (move_count < max_moves) {
        log_c(LOG_LEVEL_INFO, "Move #%d, Color to move: %s", move_count + 1, (board.color == CB_WHITE) ? "White" : "Black");
        
        CBmove movelist[MAXMOVES];
        int nmoves, isjump;
        get_legal_moves_c(board, board.color, movelist, nmoves, isjump, nullptr, nullptr);

        if (nmoves == 0) {
            log_c(LOG_LEVEL_INFO, "Game over. %s has no moves.", (board.color == CB_WHITE) ? "White" : "Black");
            break;
        }

        move_found = false;
        QEventLoop loop;

        QObject::connect(aiWorker, &AIWorker::searchFinished, [&](bool found, bool aborted, const CBmove& move, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime) {
            Q_UNUSED(statusText);
            Q_UNUSED(gameResult);
            Q_UNUSED(pdnMoveText);
            Q_UNUSED(elapsedTime);
            if (found && !aborted) {
                best_move = move;
                move_found = true;
                final_score = aiWorker->getLastEvaluationScore();
                final_depth = aiWorker->getLastSearchDepth();
                egdb_result_str = aiWorker->getEgdbLookupResult();
            }
            loop.quit();
        });
        
        // Feed current board to history for repetition detection
        uint64_t boardKey = aiWorker->generateZobristKey(board, board.color);
        aiWorker->addHistoryKey(boardKey);

        emit controller.requestAiSearch(Autoplay, board, board.color, 1.0);
        
        loop.exec(); // Block until loop.quit() is called

        if (move_found) {
            log_c(LOG_LEVEL_INFO, "[GAME] Move %d (%s): %s (Eval: %.2f, Depth: %d, EGDB: %s)", 
                  move_count + 1, 
                  (board.color == CB_WHITE) ? "White" : "Black",
                  [best_move]() { char n[80]; move4tonotation(&best_move, n); return std::string(n); }().c_str(),
                  final_score / 100.0,
                  final_depth,
                  egdb_result_str.isEmpty() ? "None" : egdb_result_str.toUtf8().constData());
            
            log_c(LOG_LEVEL_DEBUG, "Board state BEFORE move: bm=0x%X, bk=0x%X, wm=0x%X, wk=0x%X", board.bm, board.bk, board.wm, board.wk);
            domove_c(&best_move, &board);
            log_c(LOG_LEVEL_DEBUG, "Board state AFTER move: bm=0x%X, bk=0x%X, wm=0x%X, wk=0x%X", board.bm, board.bk, board.wm, board.wk);
        } else {
            log_c(LOG_LEVEL_ERROR, "AI failed to find a move. Game ends.");
            // Determine winner based on who couldn't move
            log_c(LOG_LEVEL_INFO, "Game over. %s has no legal moves. %s wins!",
                   (board.color == CB_WHITE) ? "White" : "Black",
                   (board.color == CB_WHITE) ? "Black" : "White");
            break;
        }

        move_count++;
        // Toggle color for the next move
        board.color = (board.color == CB_WHITE) ? CB_BLACK : CB_WHITE;

        // Check for immediate game over (no pieces for opponent) - now for the current board.color
        CBmove current_player_movelist[MAXMOVES];
        int current_player_nmoves = 0;
        int current_player_isjump = 0;
        get_legal_moves_c(board, board.color, current_player_movelist, current_player_nmoves, current_player_isjump, nullptr, nullptr);
        if (current_player_nmoves == 0) {
            log_c(LOG_LEVEL_INFO, "Game over. %s has no legal moves. %s wins!",
                   (board.color == CB_WHITE) ? "White" : "Black",
                   (board.color == CB_WHITE) ? "Black" : "White");
            break;
        }
    }

    if (move_count >= max_moves) {
        log_c(LOG_LEVEL_INFO, "Game drawn due to %d move limit.", max_moves);
    }

    log_c(LOG_LEVEL_INFO, "Headless game finished.");
    Logger::instance()->stop();
    Logger::instance()->wait();

    // Proper thread cleanup
    aiWorker->requestAbort();
    aiThread.quit();
    aiThread.wait();
    delete aiWorker;
    
    return 0;
}
