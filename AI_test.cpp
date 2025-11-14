#include "AI.h"
#include "c_logic.h"
#include <iostream>
#include <cassert>
#include <string>
#include <QCoreApplication>
#include <QDir>

// Helper to create a dummy QAtomicInt for internalGetMove
QAtomicInt dummyPlaynow;

// Helper function to convert FEN to Board8x8 and color
bool setupBoardFromFEN(const std::string& fen, Board8x8* board, int* color, int gametype) {
    return FENtoboard8(board, fen.c_str(), color, gametype) == 1;
}

void testEgdbLookup() {
    std::cout << "Running EGDB Lookup Tests..." << std::endl;

    // Initialize the EGDB
    QString egdbPath = QCoreApplication::applicationDirPath() + QDir::separator() + "db";
    int maxEGDBPieces = egdb_wrapper_init(egdbPath.toUtf8().constData());
    if (maxEGDBPieces == 0) {
        std::cerr << "Warning: Failed to initialize EGDB or no EGDB files found at " << egdbPath.toStdString() << std::endl;
        // Continue tests, but note EGDB tests might fail
    } else {
        std::cout << "EGDB initialized successfully. Max pieces: " << maxEGDBPieces << std::endl;
    }

    // Test 1: Known Draw position (FEN from simple_test.cpp)
    Board8x8 board1;
    pos position1;
    int color1;
    std::string fen1 = "W:K1:K32"; // Example FEN for a draw
    if (setupBoardFromFEN(fen1, &board1, &color1, GT_ENGLISH)) {
        boardtobitboard(&board1, &position1);
        int result1 = egdb_wrapper_lookup(&position1, color1);
        std::cout << "FEN: " << fen1 << ", Expected: DRAW, Got: " << result1 << std::endl;
        if (maxEGDBPieces == 0) {
            assert(result1 == CB_UNKNOWN); // If EGDB not loaded, result should be UNKNOWN
        } else {
            assert(result1 == CB_DRAW);
        }
    } else {
        std::cerr << "Failed to setup board from FEN: " << fen1 << std::endl;
    }

    // Test 2: Known Win position (replace with actual FEN)
    Board8x8 board2;
    pos position2;
    int color2;
    std::string fen2 = "W:W1,K2:B3"; // Example FEN for a white win (replace with a real win FEN)
    if (setupBoardFromFEN(fen2, &board2, &color2, GT_ENGLISH)) {
        boardtobitboard(&board2, &position2);
        int result2 = egdb_wrapper_lookup(&position2, color2);
        std::cout << "FEN: " << fen2 << ", Expected: WIN, Got: " << result2 << std::endl;
        if (maxEGDBPieces == 0) {
            assert(result2 == CB_UNKNOWN); // If EGDB not loaded, result should be UNKNOWN
        } else {
            assert(result2 == CB_WIN);
        }
    } else {
        std::cerr << "Failed to setup board from FEN: " << fen2 << std::endl;
    }

    egdb_wrapper_exit();
    std::cout << "EGDB Lookup Tests Complete." << std::endl;
}

void testAIInternalMove() {
    std::cout << "Running AI Internal Move Tests..." << std::endl;
    AI ai_instance; // Create an AI instance
    
    CBmove bestMove;
    char statusBuffer[1024];

    // Test 1: Simple move for white
    Board8x8 board1;
    setupBoardFromFEN("W:W21:B12", &board1, nullptr, GT_ENGLISH); // White 21, Black 12
    ai_instance.internalGetMove(board1, CB_WHITE, 1.0, statusBuffer, &dummyPlaynow, 0, 0, &bestMove);
    std::cout << "Board 1 (White to move), Best Move: " << coorstonumber(bestMove.from.x, bestMove.from.y, GT_ENGLISH) << "-" << coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) << std::endl;
    // Expected move: 21-17 or 21-18
    assert((coorstonumber(bestMove.from.x, bestMove.from.y, GT_ENGLISH) == 21 &&
            (coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) == 17 ||
             coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) == 18)));

    // Test 2: Simple capture for black
    Board8x8 board2;
    setupBoardFromFEN("B:W18:B22", &board2, nullptr, GT_ENGLISH); // White 18, Black 22
    ai_instance.internalGetMove(board2, CB_BLACK, 1.0, statusBuffer, &dummyPlaynow, 0, 0, &bestMove);
    std::cout << "Board 2 (Black to move), Best Move: " << coorstonumber(bestMove.from.x, bestMove.from.y, GT_ENGLISH) << "x" << coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) << std::endl;
    // Expected move: 22x15
    assert(coorstonumber(bestMove.from.x, bestMove.from.y, GT_ENGLISH) == 22 &&
           coorstonumber(bestMove.to.x, bestMove.to.y, GT_ENGLISH) == 15 &&
           bestMove.is_capture);

    std::cout << "AI Internal Move Tests Complete." << std::endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv); // Initialize QCoreApplication for QDir and QCoreApplication::applicationDirPath()

    testEgdbLookup();
    testAIInternalMove();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
