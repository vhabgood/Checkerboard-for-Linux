#include <QtTest/QtTest>
#include <QObject>
#include "GameManager.h"
#include "checkers_types.h"
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include
#include "checkers_c_types.h" // Direct include

class TestGameLogic : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Called before the first test function is executed
        GameManager::initLogging();
        GameManager::log(LogLevel::Info, "TestGameLogic: Starting test suite.");
    }

    void cleanupTestCase() {
        // Called after the last test function is executed
        GameManager::log(LogLevel::Info, "TestGameLogic: Test suite finished.");
        GameManager::closeLogging();
    }

    void init() {
        // Called before each test function is executed
        // Reset GameManager for each test
        m_gameManager = new GameManager(this);
        m_gameManager->setOptions(CBoptions()); // Reset options to default
        GameManager::log(LogLevel::Info, "TestGameLogic: Test function started.");
    }

    void cleanup() {
        // Called after each test function is executed
        delete m_gameManager;
        m_gameManager = nullptr;
        GameManager::log(LogLevel::Info, "TestGameLogic: Test function finished.");
    }

    void testNewGame_data() {
        QTest::addColumn<int>("gameType");
        QTest::newRow("English Checkers") << GT_ENGLISH;
        // Add other game types if supported
    }

    void testNewGame() {
        QFETCH(int, gameType);
        m_gameManager->newGame(gameType);

        // Verify initial board state (e.g., number of pieces, correct placement)
        Board8x8 board = m_gameManager->getCurrentBoard();
        int whitePieces = 0;
        int blackPieces = 0;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                int piece = board.board[r][c];
                if (piece & CB_WHITE) {
                    whitePieces++;
                } else if (piece & CB_BLACK) {
                    blackPieces++;
                }
            }
        }
        QCOMPARE(whitePieces, 12);
        QCOMPARE(blackPieces, 12);
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_BLACK); // Black starts
        QCOMPARE(m_gameManager->getTotalMoves(), 0); // No moves made yet
    }

    void testMakeMove() {
        m_gameManager->newGame(GT_ENGLISH);

        pos currentPos;
        Board8x8 currentBoard = m_gameManager->getCurrentBoard();
        boardtobitboard(&currentBoard, &currentPos);
        CBmove legalMoves[MAXMOVES];
        int nmoves_val = 0;
        int isjump_val = 0;
        bool dummy_can_continue_multijump = false;
        get_legal_moves_c(&currentPos, m_gameManager->getCurrentPlayer(), legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

        GameManager::log(LogLevel::Debug, QString("testMakeMove: Generated %1 legal moves.").arg(nmoves_val));
        for (int i = 0; i < nmoves_val; ++i) {
            GameManager::log(LogLevel::Debug, QString("testMakeMove: Legal move %1: from (%2,%3) to (%4,%5)").arg(i).arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y));
        }

        // Find the move from (0,5) to (1,4) (square 12 to 16)
        CBmove move_to_make = {0};
        bool moveFound = false;
        for (int i = 0; i < nmoves_val; ++i) {
            if (legalMoves[i].from.x == 0 && legalMoves[i].from.y == 5 &&
                legalMoves[i].to.x == 1 && legalMoves[i].to.y == 4) {
                move_to_make = legalMoves[i];
                moveFound = true;
                break;
            }
        }
        GameManager::log(LogLevel::Debug, QString("testMakeMove: Searching for move from (0,5) to (1,4). Found: %1").arg(moveFound));
        QVERIFY(moveFound);

        bool isCapture = m_gameManager->makeMove(move_to_make);
        m_gameManager->switchTurn(); // Manually switch turn for testing
        QVERIFY(!isCapture);
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_WHITE); // Turn should switch
        QCOMPARE(m_gameManager->getTotalMoves(), 1); // One move made
        QCOMPARE(m_gameManager->getCurrentBoard().board[5][0], CB_EMPTY); // Old position empty (0,5)
        QCOMPARE(m_gameManager->getCurrentBoard().board[4][1], (CB_BLACK | CB_MAN)); // New position has piece (1,4)
    }

    void testMakeCaptureMove() {
        m_gameManager->newGame(GT_ENGLISH);
        // Set up a capture scenario manually
        Board8x8 board;
        // Start with default board, then modify for capture scenario
        m_gameManager->newGame(GT_ENGLISH);
        board = m_gameManager->getCurrentBoard(); // Get the initial board
        // Clear some pieces to set up a capture
        board.board[2][0] = CB_EMPTY; // Clear black piece at 11
        board.board[3][1] = CB_EMPTY; // Clear black piece at 15
        board.board[4][2] = (CB_WHITE | CB_MAN); // Place white piece at 19
        board.board[5][3] = (CB_BLACK | CB_MAN); // Place black piece at 23
        m_gameManager->setCurrentBoard(board);
        m_gameManager->setCurrentColorToMove(CB_BLACK);

        // Get legal moves for black (should include the capture)
        pos currentPos;
        Board8x8 currentBoard = m_gameManager->getCurrentBoard();
        boardtobitboard(&currentBoard, &currentPos);
        CBmove legalMoves[MAXMOVES];
        int nmoves_val = 0;
        int isjump_val = 0;
        bool dummy_can_continue_multijump = false;
        get_legal_moves_c(&currentPos, m_gameManager->getCurrentPlayer(), legalMoves, &nmoves_val, &isjump_val, NULL, &dummy_can_continue_multijump);

        GameManager::log(LogLevel::Debug, QString("testMakeCaptureMove: Generated %1 legal moves.").arg(nmoves_val));
        for (int i = 0; i < nmoves_val; ++i) {
            GameManager::log(LogLevel::Debug, QString("testMakeCaptureMove: Legal move %1: from (%2,%3) to (%4,%5), is_capture: %6").arg(i).arg(legalMoves[i].from.x).arg(legalMoves[i].from.y).arg(legalMoves[i].to.x).arg(legalMoves[i].to.y).arg(legalMoves[i].is_capture));
        }

        // Find the capture move (e.g., 23-14 capturing 19)
        // Square 23 is (x=3, y=5)
        // Square 14 is (x=1, y=3)
        // Captured piece at 19 is (x=2, y=4)
        CBmove move_to_make = {0};
        bool moveFound = false;
        for (int i = 0; i < nmoves_val; ++i) {
            if (legalMoves[i].from.x == 3 && legalMoves[i].from.y == 5 && // Corrected from.x
                legalMoves[i].to.x == 1 && legalMoves[i].to.y == 3 &&
                legalMoves[i].is_capture) {
                move_to_make = legalMoves[i];
                moveFound = true;
                break;
            }
        }
        GameManager::log(LogLevel::Debug, QString("testMakeCaptureMove: Searching for move from (3,5) to (1,3) with capture. Found: %1").arg(moveFound));
        QVERIFY(moveFound);

        bool isCapture = m_gameManager->makeMove(move_to_make);
        m_gameManager->switchTurn(); // Manually switch turn for testing
        QVERIFY(isCapture);
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_WHITE); // Turn should switch
        QCOMPARE(m_gameManager->getTotalMoves(), 2); // Move added to history
        QCOMPARE(m_gameManager->getCurrentBoard().board[5][2], CB_EMPTY); // Old position empty (23)
        QCOMPARE(m_gameManager->getCurrentBoard().board[3][1], (CB_BLACK | CB_MAN)); // New position has piece (14)
        QCOMPARE(m_gameManager->getCurrentBoard().board[4][2], CB_EMPTY); // Captured piece removed (19)
    }

    void testInvalidMoves() {
        m_gameManager->newGame(GT_ENGLISH);
        // Scenario 1: Move to an occupied square
        CBmove invalidMove1 = {0};
        invalidMove1.from.x = 0; invalidMove1.from.y = 2; // Square 11 (Black man)
        invalidMove1.to.x = 0; invalidMove1.to.y = 1;   // Square 9 (Black man) - occupied
        QVERIFY(!m_gameManager->isLegalMove(invalidMove1));

        // Scenario 2: Move off the board
        CBmove invalidMove2 = {0};
        invalidMove2.from.x = 0; invalidMove2.from.y = 2; // Square 11 (Black man)
        invalidMove2.to.x = -1; invalidMove2.to.y = 3;  // Off board
        QVERIFY(!m_gameManager->isLegalMove(invalidMove2));

        // Scenario 3: Move the wrong color piece
        m_gameManager->setCurrentColorToMove(CB_BLACK); // Ensure it's black's turn
        CBmove invalidMove3 = {0};
        invalidMove3.from.x = 0; invalidMove3.from.y = 5; // Square 21 (White man)
        invalidMove3.to.x = 1; invalidMove3.to.y = 4;   // Valid move for white, but wrong turn
        QVERIFY(!m_gameManager->isLegalMove(invalidMove3));

        // Scenario 4: Non-capture move when capture is available
        // Set up a board where a capture is forced
        Board8x8 board;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                board.board[r][c] = CB_EMPTY;
            }
        }
        board.board[2][2] = (CB_BLACK | CB_MAN); // Black man at 14
        board.board[3][3] = (CB_WHITE | CB_MAN); // White man at 19
        board.board[4][4] = CB_EMPTY;            // Empty square at 24 for capture
        m_gameManager->setCurrentBoard(board);
        m_gameManager->setCurrentColorToMove(CB_BLACK);

        CBmove nonCaptureMove = {0};
        nonCaptureMove.from.x = 2; nonCaptureMove.from.y = 2; // Black man at 14
        nonCaptureMove.to.x = 3; nonCaptureMove.to.y = 1;   // Invalid non-capture move
        QVERIFY(!m_gameManager->isLegalMove(nonCaptureMove));

        // Scenario 5: Move a piece that doesn't exist
        CBmove invalidMove5 = {0};
        invalidMove5.from.x = 0; invalidMove5.from.y = 0; // Empty square
        invalidMove5.to.x = 1; invalidMove5.to.y = 1;
        QVERIFY(!m_gameManager->isLegalMove(invalidMove5));
    }

    void testGoBackAndForward() {
        m_gameManager->newGame(GT_ENGLISH);
        // Make a few moves
        CBmove move1 = {0}; move1.from.x = 0; move1.from.y = 2; move1.to.x = 1; move1.to.y = 3;
        m_gameManager->makeMove(move1); // Black moves 11-15
        m_gameManager->switchTurn(); // Manually switch turn for testing
        CBmove move2 = {0}; move2.from.x = 1; move2.from.y = 5; move2.to.x = 0; move2.to.y = 4;
        m_gameManager->makeMove(move2); // White moves 22-18
        m_gameManager->switchTurn(); // Manually switch turn for testing

        QCOMPARE(m_gameManager->getTotalMoves(), 2); // 2 moves made

        m_gameManager->goBack();
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_WHITE); // Should be white's turn again
        QCOMPARE(m_gameManager->getTotalMoves(), 2); // History size doesn't change, index does

        m_gameManager->goBack();
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_BLACK); // Should be black's turn again

        m_gameManager->goForward();
        QCOMPARE(m_gameManager->getCurrentPlayer(), CB_WHITE); // Should be white's turn again
    }

    void testDrawDetection_50MoveRule() {
        m_gameManager->newGame(GT_ENGLISH);
        // Simulate 50 non-capture moves (100 half-moves)
        for (int i = 0; i < 100; ++i) {
            // Make a dummy non-capture move
            CBmove move = {0};
            move.from.x = 0; move.from.y = 2;
            move.to.x = 1; move.to.y = 3;
            m_gameManager->makeMove(move);
            m_gameManager->switchTurn(); // Manually switch turn for dummy moves
        }
        QCOMPARE(m_gameManager->getHalfMoveCount(), 100);

        // Connect to gameIsOver signal
        bool gameOverSignalReceived = false;
        int gameResult = -1;
        connect(m_gameManager, &GameManager::gameIsOver, [&](int result){
            gameOverSignalReceived = true;
            gameResult = result;
        });

        m_gameManager->detectDraws();
        QVERIFY(gameOverSignalReceived);
        QCOMPARE(gameResult, CB_DRAW);
    }

    void testDrawDetection_ThreeFoldRepetition() {
        m_gameManager->newGame(GT_ENGLISH);
        // Simulate a position repeating three times
        // Initial position is already in history
        QString initialFen = m_gameManager->getFenPosition();

        // Make two moves and return to initial position
        CBmove move1 = {0}; move1.from.x = 0; move1.from.y = 2; move1.to.x = 1; move1.to.y = 3;
        m_gameManager->makeMove(move1);
        m_gameManager->switchTurn();
        CBmove move2 = {0}; move2.from.x = 1; move2.from.y = 5; move2.to.x = 0; move2.to.y = 4;
        m_gameManager->makeMove(move2);
        m_gameManager->switchTurn();

        // Go back to initial position
        m_gameManager->goBackAll();
        QCOMPARE(m_gameManager->getFenPosition(), initialFen);

        // Make the same two moves again
        m_gameManager->makeMove(move1);
        m_gameManager->switchTurn();
        m_gameManager->makeMove(move2);
        m_gameManager->switchTurn();

        // Go back to initial position again
        m_gameManager->goBackAll();
        QCOMPARE(m_gameManager->getFenPosition(), initialFen);

        // Connect to gameIsOver signal
        bool gameOverSignalReceived = false;
        int gameResult = -1;
        connect(m_gameManager, &GameManager::gameIsOver, [&](int result){
            gameOverSignalReceived = true;
            gameResult = result;
        });

        m_gameManager->detectDraws();
        QVERIFY(gameOverSignalReceived);
        QCOMPARE(gameResult, CB_DRAW);
    }

    void testWinLoss_NoLegalMoves() {
        m_gameManager->newGame(GT_ENGLISH);
        // Manually set up a board where black has no legal moves
        Board8x8 board;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                board.board[r][c] = CB_EMPTY;
            }
        }
        // Place a white piece that blocks all black pieces
        board.board[0][0] = (CB_WHITE | CB_KING); // White king at 1
        board.board[1][1] = (CB_BLACK | CB_MAN); // Black man at 5 (trapped)
        m_gameManager->setCurrentBoard(board);
        m_gameManager->setCurrentColorToMove(CB_BLACK); // It's black's turn

        // Connect to gameIsOver signal
        bool gameOverSignalReceived = false;
        int gameResult = -1;
        connect(m_gameManager, &GameManager::gameIsOver, [&](int result){
            gameOverSignalReceived = true;
            gameResult = result;
        });

        // Trigger playMove, which should detect no legal moves for black
        m_gameManager->playMove(); // This will call get_legal_moves_c internally

        QVERIFY(gameOverSignalReceived);
        QCOMPARE(gameResult, CB_LOSS); // Black loses
    }

private:
    GameManager *m_gameManager;
};