#include "AI.h"
#include "GeminiAI.h"
#include <cmath>
#include <QThread>
#include <cstring> // For memcpy, memset
#include <QElapsedTimer>
#include <QCoreApplication> // Added for applicationDirPath
#include <QFile> // Added for QFile
#include <QTextStream> // Added for QTextStream
#include <QDir> // Added for QDir
#include <QDataStream>
#include <algorithm> // Required for std::sort
#include <limits> // Required for std::numeric_limits
#include <sys/resource.h> // For setpriority
#include <errno.h> // For strerror
#include <unistd.h> // Required for getpid()
#include "engine_wrapper.h" // Include engine_wrapper.h
#include "GameManager.h" // Include GameManager for logging
#include "dblookup.h" // Re-explicitly include dblookup.h

#define AUTOSLEEPTIME 10 // Milliseconds
#define SLEEPTIME 10 // Milliseconds (from AutoThreadWorker.cpp)



void AI::setPriority(int priority)
{
    qDebug() << QString("AI: Setting AI process priority to: %1").arg(priority));
    pid_t pid = getpid(); // Get the PID of the current process (AI thread's process)
    if (setpriority(PRIO_PROCESS, pid, priority) == -1) {
        qCritical() << QString("AI: Failed to set AI process priority: %1").arg(strerror(errno)));
    } else {
        qDebug() << QString("AI: AI process priority set to %1").arg(priority));
    }

    if (m_engineProcess && m_engineProcess->state() == QProcess::Running) {
        pid_t enginePid = m_engineProcess->processId();
    if (setpriority(PRIO_PROCESS, m_engineProcess->processId(), priority) == -1) {
        qCritical() << QString("AI: Failed to set engine process priority: %1").arg(strerror(errno)));
    } else {
        qDebug() << QString("AI: Engine process priority set to %1").arg(priority));
    }
    }
}

void AI::setHandicap(int handicapDepth)
{
    m_handicapDepth = handicapDepth;
    qDebug() << QString("AI: AI handicap depth set to: %1").arg(m_handicapDepth));
}

bool AI::isGameOver(Board8x8 board, int color)
{
    CBmove movelist[MAXMOVES];
    int numMoves = 0;
    pos currentPos;
    boardtobitboard(&board, &currentPos);
    int isjump = 0;
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&currentPos, color, movelist, &numMoves, &isjump, NULL, &dummy_can_continue_multijump);

    return numMoves == 0; // If no legal moves, game is over
}

static inline int cbcolor_to_getmovelistcolor_internal(int cbcolor)
{
    if (cbcolor == CB_BLACK)
        return(1);
    else
        return(-1);
}

void AI::board8toboard12(Board8x8 board8, int board12[12][12])
{
    int i, j;
    for (i = 0; i <= 11; i++) {
        for (j = 0; j <= 11; j++)
            board12[i][j] = 10;
    }

    for (i = 2; i <= 9; i++) {
        for (j = 2; j <= 9; j++)
            board12[i][j] = 0;
    }

    for (i = 0; i <= 7; i++) {
        for (j = 0; j <= 7; j++) {
            if (board8.board[i][j] == (CB_BLACK | CB_MAN))
                board12[i + 2][j + 2] = 1;
            if (board8.board[i][j] == (CB_BLACK | CB_KING))
                board12[i + 2][j + 2] = 2;
            if (board8.board[i][j] == (CB_WHITE | CB_MAN))
                board12[i + 2][j + 2] = -1;
            if (board8.board[i][j] == (CB_WHITE | CB_KING))
                board12[i + 2][j + 2] = -2;
        }
    }
}

int AI::makemovelist(int color, CBmove movelist[MAXMOVES], int board[12][12], int *isjump, int *n)
{
    coor wk[12], bk[12], ws[12], bs[12];
    int nwk = 0, nbk = 0, nws = 0, nbs = 0;
    int i, j;
    int x, y;
    CBmove m;
    *isjump = 0;

    for (i = 0; i < MAXMOVES; i++)
        movelist[i].jumps = 0;
    *n = 0;

    for (j = 2; j <= 8; j += 2) {
        for (i = 2; i <= 8; i += 2) {
            if (board[i][j] == 0)
                continue;
            if (board[i][j] == 1) {
                ws[nws].x = i;
                ws[nws].y = j;
                nws++;
                continue;
            }

            if (board[i][j] == 2) {
                wk[nwk].x = i;
                wk[nwk].y = j;
                nwk++;
                continue;
            }

            if (board[i][j] == -1) {
                bs[nbs].x = i;
                bs[nbs].y = j;
                nbs++;
                continue;
            }

            if (board[i][j] == -2) {
                bk[nbk].x = i;
                bk[nbk].y = j;
                nbk++;
                continue;
            }
        }
    }

    for (j = 3; j <= 9; j += 2) {
        for (i = 3; i <= 9; i += 2) {
            if (board[i][j] == 0)
                continue;
            if (board[i][j] == 1) {
                ws[nws].x = i;
                ws[nws].y = j;
                nws++;
                continue;
            }

            if (board[i][j] == 2) {
                wk[nwk].x = i;
                wk[nwk].y = j;
                nwk++;
                continue;
            }

            if (board[i][j] == -1) {
                bs[nbs].x = i;
                bs[nbs].y = j;
                nbs++;
                continue;
            }

            if (board[i][j] == -2) {
                bk[nbk].x = i;
                bk[nbk].y = j;
                nbk++;
                continue;
            }
        }
    }

    if (color == CB_WHITE) {

        /* search for captures with white kings*/
        if (nwk > 0) {
            for (i = 0; i < nwk; i++) {
                x = wk[i].x;
                y = wk[i].y;
                if
                (
                    (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == 0) ||
                    (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == 0) ||
                    (board[x + 1][y - 1] < 0 && board[x + 2][y - 2] == 0) ||
                    (board[x - 1][y - 1] < 0 && board[x - 2][y - 2] == 0)
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    whitekingcapture(board,movelist,m,x,y,0,n);
                }
            }
        }

        /* search for captures with white stones */
        if (nws > 0) {
            for (i = 0; i < nws; i++) {
                x = ws[i].x;
                y = ws[i].y;
                if
                (
                    (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == 0) ||
                    (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == 0)
                ) {
                    m.from.x = x;
                    m.from.y = y;
                    m.path[0].x = x;
                    m.path[0].y = y;
                    whitecapture(board,movelist,m,x,y,0,n);
                }
            }
        }

        /* if there are capture moves return. */
                if (*n > 0) {
                    *isjump = 1;
                    return 0;
                }
                    /* search for moves with white kings */
                    if (nwk > 0) {
                        for (i = 0; i < nwk; i++) {
                            x = wk[i].x;
                            y = wk[i].y;
                            if (board[x + 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = 2;
                                movelist[*n].oldpiece = 2;
                                (*n)++;
                            }

                            if (board[x + 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = 2;
                                movelist[*n].oldpiece = 2;
                                (*n)++;
                            }

                            if (board[x - 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = 2;
                                movelist[*n].oldpiece = 2;
                                (*n)++;
                            }

                            if (board[x - 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = 2;
                                movelist[*n].oldpiece = 2;
                                (*n)++;
                            }
                        }
                    }

                    /* search for moves with white stones */
                    if (nws > 0) {
                        for (i = nws - 1; i >= 0; i--) {
                            x = ws[i].x;
                            y = ws[i].y;
                            if (board[x + 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                if (y == 8) {
                                    movelist[*n].newpiece = 2;
                                }
                                else
                                    movelist[*n].newpiece = 1;
                                movelist[*n].oldpiece = 1;
                                (*n)++;
                            }

                            if (board[x - 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                if (y == 8) {
                                    movelist[*n].newpiece = 2;
                                }
                                else
                                    movelist[*n].newpiece = 1;
                                movelist[*n].oldpiece = 1;
                                (*n)++;
                            }
                        }
                    }

                    if (*n > 0)
                        return *n;
                }
                else {

                    /* search for captures with black kings*/
                    *n = 0;
                    if (nbk > 0) {
                        for (i = 0; i < nbk; i++) {
                            x = bk[i].x;
                            y = bk[i].y;
                            if
                            (
                                ((board[x + 1][y + 1] > 0) && (board[x + 1][y + 1] < 3) && (board[x + 2][y + 2] == 0)) ||
                                ((board[x - 1][y + 1] > 0) && (board[x - 1][y + 1] < 3) && (board[x - 2][y + 2] == 0)) ||
                                ((board[x + 1][y - 1] > 0) && (board[x + 1][y - 1] < 3) && (board[x + 2][y - 2] == 0)) ||
                                ((board[x - 1][y - 1] > 0) && (board[x - 1][y - 1] < 3) && (board[x - 2][y - 2] == 0))
                            ) {
                                m.from.x = x;
                                m.from.y = y;
                                m.path[0].x = x;
                                m.path[0].y = y;
                                //blackkingcapture(board, movelist, m, x, y, 0, n);
                            }
                        }
                    }

                    /* search for captures with black stones */
                    if (nbs > 0) {
                        for (i = nbs - 1; i >= 0; i--) {
                            x = bs[i].x;
                            y = bs[i].y;
                            if
                            (
                                (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == 0) ||
                                (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == 0)
                            ) {
                                m.from.x = x;
                                m.from.y = y;
                                m.path[0].x = x;
                                m.path[0].y = y;
                                //blackcapture(board,movelist,m,x,y,0,n);
                            }
                        }
                    }

                    /* search for moves with black kings*/
        if (*n > 0) {
            *isjump = 1;
            return 0;
        }

                    if (nbk > 0) {
                        for (i = 0; i < nbk; i++) {
                            x = bk[i].x;
                            y = bk[i].y;
                            if (board[x + 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = -2;
                                movelist[*n].oldpiece = -2;
                                (*n)++;
                            }

                            if (board[x + 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = -2;
                                movelist[*n].oldpiece = -2;
                                (*n)++;
                            }

                            if (board[x - 1][y + 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y + 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y + 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = -2;
                                movelist[*n].oldpiece = -2;
                                (*n)++;
                            }

                            if (board[x - 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                movelist[*n].newpiece = -2;
                                movelist[*n].oldpiece = -2;
                                (*n)++;
                            }
                        }
                    }

                    /* search for moves with black stones */
                    if (nbs > 0) {
                        for (i = 0; i < nbs; i++) {
                            x = bs[i].x;
                            y = bs[i].y;
                            if (board[x + 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x + 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x + 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                if (y == 3) {
                                    movelist[*n].newpiece = -2;
                                }
                                else
                                    movelist[*n].newpiece = -1;
                                movelist[*n].oldpiece = -1;
                                (*n)++;
                            }

                            if (board[x - 1][y - 1] == 0) {
                                movelist[*n].jumps = 0;
                                movelist[*n].from.x = x;
                                movelist[*n].from.y = y;
                                movelist[*n].to.x = x - 1;
                                movelist[*n].to.y = y - 1;
                                movelist[*n].path[0].x = x;
                                movelist[*n].path[0].y = y;
                                movelist[*n].path[1].x = x - 1;
                                movelist[*n].path[1].y = y - 1;
                                movelist[*n].del[0].x = -1;
                                if (y == 3) {
                                    movelist[*n].newpiece = -2;
                                }
                                else
                                    movelist[*n].newpiece = -1;
                                movelist[*n].oldpiece = -1;
                                (*n)++;
                            }
                        }
                    }
                }
    return 0; // Added return statement to satisfy non-void function requirement
}

void AI::whitecapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    int b[12][12];
    CBmove mm;
    int end = 1;

    mm = m;
    if (y < 8) {
        if (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == 0) {
            memcpy(b, board, 144 * sizeof(int));
            b[x][y] = 0;
            b[x + 1][y + 1] = 0;
            b[x + 2][y + 2] = 1;
            mm.to.x = x + 2;
            mm.to.y = y + 2;
            mm.path[d + 1].x = x + 2;
            mm.path[d + 1].y = y + 2;
            mm.del[d].x = x + 1;
            mm.del[d].y = y + 1;
            mm.delpiece[d] = board[x + 1][y + 1];
            mm.del[d + 1].x = -1;
            if (y == 7)
                mm.newpiece = 2;
            else
                mm.newpiece = 1;

            whitecapture(b,movelist,mm,x+2,y+2,d+1,n);
            end = 0;
        }

        mm = m;
        if (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == 0) {
            memcpy(b, board, 144 * sizeof(int));
            b[x][y] = 0;
            b[x - 1][y + 1] = 0;
            b[x - 2][y + 2] = 1;
            mm.to.x = x - 2;
            mm.to.y = y + 2;
            mm.path[d + 1].x = x - 2;
            mm.path[d + 1].y = y + 2;
            mm.del[d].x = x - 1;
            mm.del[d].y = y + 1;
            mm.del[d + 1].x = -1;
            mm.delpiece[d] = board[x - 1][y + 1];
            if (y == 7)
                mm.newpiece = 2;
            else
                mm.newpiece = 1;

            whitecapture(b,movelist,mm,x-2,y+2,d+1,n);
            end = 0;
        }
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = 1;
        (*n)++;
    }
}

void AI::whitekingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    int b[12][12];
    CBmove mm;
    int end = 1;

    mm = m;
    if (board[x + 1][y + 1] < 0 && board[x + 2][y + 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x + 1][y + 1] = 0;
        b[x + 2][y + 2] = 2;
        mm.to.x = x + 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y + 1;
        mm.del[d + 1].x = -1;
        mm.delpiece[d] = board[x + 1][y + 1];
        mm.newpiece = 2;

        whitekingcapture(b,movelist,mm,x+2,y+2,d+1,n);
        end = 0;
    }

    mm = m;
    if (board[x - 1][y + 1] < 0 && board[x - 2][y + 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x - 1][y + 1] = 0;
        b[x - 2][y + 2] = 2;
        mm.to.x = x - 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x - 1][y + 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = 2;

        whitekingcapture(b,movelist,mm,x-2,y+2,d+1,n);
        end = 0;
    }

    if (board[x + 1][y - 1] < 0 && board[x + 2][y - 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x + 1][y - 1] = 0;
        b[x + 2][y - 2] = 2;
        mm.to.x = x + 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x + 1][y - 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = 2;

        whitekingcapture(b,movelist,mm,x+2,y-2,d+1,n);
        end = 0;
    }

    mm = m;
    if (board[x - 1][y - 1] < 0 && board[x - 2][y - 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x - 1][y - 1] = 0;
        b[x - 2][y - 2] = 2;
        mm.to.x = x - 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x - 1][y - 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = 2;

        whitekingcapture(b,movelist,mm,x-2,y-2,d+1,n);
        end = 0;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = 2;
        (*n)++;
    }
}

void AI::blackcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    int b[12][12];
    CBmove mm;
    int end = 1;

    mm = m;
    if (y > 3) {
        if (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == 0) {
            memcpy(b, board, 144 * sizeof(int));
            b[x][y] = 0;
            b[x + 1][y - 1] = 0;
            b[x + 2][y - 2] = -1;
            mm.to.x = x + 2;
            mm.to.y = y - 2;
            mm.path[d + 1].x = x + 2;
            mm.path[d + 1].y = y - 2;
            mm.del[d].x = x + 1;
            mm.del[d].y = y - 1;
            mm.delpiece[d] = board[x + 1][y - 1];
            mm.del[d + 1].x = -1;
            if (y == 4)
                mm.newpiece = -2;
            else
                mm.newpiece = -1;

            blackcapture(b, movelist, mm, x + 2, y - 2, d + 1, n);
            end = 0;
        }

        mm = m;
        if (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == 0) {
            memcpy(b, board, 144 * sizeof(int));
            b[x][y] = 0;
            b[x - 1][y - 1] = 0;
            b[x - 2][y - 2] = -1;
            mm.to.x = x - 2;
            mm.to.y = y - 2;
            mm.path[d + 1].x = x - 2;
            mm.path[d + 1].y = y - 2;
            mm.del[d].x = x - 1;
            mm.del[d].y = y - 1;
            mm.delpiece[d] = board[x - 1][y - 1];
            mm.del[d + 1].x = -1;
            if (y == 4)
                mm.newpiece = -2;
            else
                mm.newpiece = -1;

            blackcapture(b, movelist, mm, x - 2, y - 2, d + 1, n);
            end = 0;
        }
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = -1;
        (*n)++;
    }
}

void AI::blackkingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
{
    int b[12][12];
    CBmove mm;
    int end = 1;

    mm = m;
    if (board[x + 1][y + 1] > 0 && board[x + 2][y + 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x + 1][y + 1] = 0;
        b[x + 2][y + 2] = 2;
        mm.to.x = x + 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x + 1][y + 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = -2;

        blackkingcapture(b, movelist, mm, x + 2, y + 2, d + 1, n);
        end = 0;
    }

    mm = m;
    if (board[x - 1][y + 1] > 0 && board[x - 2][y + 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x - 1][y + 1] = 0;
        b[x - 2][y + 2] = 2;
        mm.to.x = x - 2;
        mm.to.y = y + 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y + 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y + 1;
        mm.delpiece[d] = board[x - 1][y + 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = -2;

        blackkingcapture(b, movelist, mm, x - 2, y + 2, d + 1, n);
        end = 0;
    }

    if (board[x + 1][y - 1] > 0 && board[x + 2][y - 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x + 1][y - 1] = 0;
        b[x + 2][y - 2] = 2;
        mm.to.x = x + 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x + 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x + 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x + 1][y - 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = -2;

        blackkingcapture(b, movelist, mm, x + 2, y - 2, d + 1, n);
        end = 0;
    }

    mm = m;
    if (board[x - 1][y - 1] > 0 && board[x - 2][y - 2] == 0) {
        memcpy(b, board, 144 * sizeof(int));
        b[x][y] = 0;
        b[x - 1][y - 1] = 0;
        b[x - 2][y - 2] = 2;
        mm.to.x = x - 2;
        mm.to.y = y - 2;
        mm.path[d + 1].x = x - 2;
        mm.path[d + 1].y = y - 2;
        mm.del[d].x = x - 1;
        mm.del[d].y = y - 1;
        mm.delpiece[d] = board[x - 1][y - 1];
        mm.del[d + 1].x = -1;
        mm.newpiece = -2;

        blackkingcapture(b, movelist, mm, x - 2, y - 2, d + 1, n);
        end = 0;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = -2;
        (*n)++;
    }
}

void AI::loadUserBook(const QString& filename)
{
    qInfo() << QString("AI: Loading user book from: %1").arg(filename));
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << QString("AI: Failed to open user book file for reading: %1").arg(filename));
        return;
    }

    QDataStream in(&file);
    m_userbooknum = 0; // Reset existing book
    in.setVersion(QDataStream::Qt_5_12); // Set data stream version
    while (!in.atEnd() && m_userbooknum < MAXUSERBOOK) {
        userbookentry entry;
        int bytesRead = in.readRawData(reinterpret_cast<char*>(&entry), sizeof(userbookentry));
        if (bytesRead != sizeof(userbookentry) || in.status() != QDataStream::Ok) {
            qCritical() << QString("AI: Error reading user book entry from file: %1, Bytes read: %2/%3, Status: %4").arg(filename).arg(bytesRead).arg(sizeof(userbookentry)).arg(in.status()));
            if (bytesRead == -1 && in.status() != QDataStream::Ok) {
                 qWarning() << "AI: Attempting to re-read with a different stream version or handle corrupted data.");
            }
            break;
        }
        m_userbook[m_userbooknum++] = entry;
    }
    file.close();
    m_userbookcur = 0; // Reset navigation
    qInfo() << QString("AI: Loaded %1 entries from user book: %2").arg(m_userbooknum).arg(filename));
}

void AI::saveUserBook(const QString& filename)
{
    qInfo() << QString("AI: Saving user book to: %1").arg(filename));
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << QString("AI: Failed to open user book file for writing: %1").arg(filename));
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12); // Set data stream version
    for (int i = 0; i < m_userbooknum; ++i) {
        int bytesWritten = out.writeRawData(reinterpret_cast<const char*>(&m_userbook[i]), sizeof(userbookentry));
        if (bytesWritten != sizeof(userbookentry) || out.status() != QDataStream::Ok) {
            qCritical() << QString("AI: Error writing user book entry to file: %1, Bytes written: %2/%3, Status: %4").arg(filename).arg(bytesWritten).arg(sizeof(userbookentry)).arg(out.status()));
            break;
        }
    }
    file.close();
    qInfo() << QString("AI: Saved %1 entries to user book: %2").arg(m_userbooknum).arg(filename));
}

void AI::deleteCurrentEntry()
{
    if (m_userbooknum == 0 || m_userbookcur < 0 || m_userbookcur >= m_userbooknum) {
        qWarning() << "AI: No entry to delete or invalid current index.");
        return;
    }

    for (int i = m_userbookcur; i < m_userbooknum - 1; ++i) {
        m_userbook[i] = m_userbook[i+1];
    }
    m_userbooknum--;

    if (m_userbookcur >= m_userbooknum && m_userbooknum > 0) {
        m_userbookcur = m_userbooknum - 1;
    }
    else if (m_userbooknum == 0) {
        m_userbookcur = 0;
    }
}

void AI::navigateToNextEntry()
{
    if (m_userbooknum > 0) {
        m_userbookcur = (m_userbookcur + 1) % m_userbooknum;
    }
}

void AI::navigateToPreviousEntry()
{
    if (m_userbooknum > 0) {
        m_userbookcur = (m_userbookcur - 1 + m_userbooknum) % m_userbooknum;
    }
}

void AI::resetNavigation()
{
    m_userbookcur = 0;
}

void AI::deleteAllEntriesFromUserBook()
{
    qInfo() << "AI: Deleting all entries from user book.");
    m_userbooknum = 0; // Simply reset the count to 0
    m_userbookcur = 0; // Reset navigation
    qInfo() << "AI: All entries deleted from user book.");
}

const userbookentry* AI::getCurrentEntry() const
{
    if (m_userbooknum > 0 && m_userbookcur >= 0 && m_userbookcur < m_userbooknum) {
        return &m_userbook[m_userbookcur];
    }
    return nullptr;
}

bool AI::lookupMove(const Board8x8 board, int color, int gametype, CBmove* bookMove) const
{
    Q_UNUSED(gametype); // gametype is not used in this implementation

    pos current_pos;
    boardtobitboard(&board, &current_pos);

    for (int i = 0; i < m_userbooknum; ++i) {
        if (memcmp(&m_userbook[i].position, &current_pos, sizeof(pos)) == 0) {
            // Check if the color matches. The book move should be for the current player.
            // This simple check assumes the book is consistent.
            // A more robust implementation might store the color to move with the position.
            if (m_color == color) {
                *bookMove = m_userbook[i].move;
                return true;
            }
        }
    }

    return false; // No move found in the book
}

void AI::addMoveToUserBook(const Board8x8 board, const CBmove& move)
{
    if (m_userbooknum >= MAXUSERBOOK) {
        qWarning() << "User book is full. Cannot add new entry.");
        return;
    }

    userbookentry newEntry;
    boardtobitboard(&board, &newEntry.position); // Convert Board8x8 to pos
    newEntry.move = move;
    m_userbook[m_userbooknum++] = newEntry;
    qInfo() << QString("Added move to user book. Total entries: %1").arg(m_userbooknum));
}


void AI::loadEngine(const QString& enginePath)
{
    qInfo() << QString("AI: Attempting to load engine: %1").arg(enginePath));
    if (m_engineProcess) {
        m_engineProcess->close();
        // m_engineProcess->deleteLater(); // QProcess will be deleted by its parent QObject (AI) when AI is destroyed
    }

    // Ensure m_engineProcess is created in the correct thread context
    if (!m_engineProcess) {
        initEngineProcess();
    }

    m_engineProcess->setProgram(enginePath);
    m_engineProcess->start();
    if (m_engineProcess->waitForStarted()) {
    qInfo() << QString("AI: Starting engine: %1 %2").arg(m_engineProcess->program()).arg(m_engineProcess->arguments().join(" ")));
        emit engineOutput("Engine loaded successfully.");
    } else {
        qCritical() << QString("AI: Failed to start engine: %1").arg(enginePath));
        emit engineError(QString("Failed to start engine: %1").arg(enginePath));
    }
}

void AI::requestMove(const Board8x8& board, int colorToMove, double timeLimit)
{
    qDebug() << QString("AI::requestMove: Received request with timeLimit: %1").arg(timeLimit));
    // Set the board and color for the AI to work with
    memcpy(&m_board, &board, sizeof(Board8x8));
    m_color = colorToMove;
    m_maxtime = timeLimit;
    m_abortRequested.storeRelaxed(0); // Reset abort flag

    if (m_useInternalAI) {
        qDebug() << "AI: Queuing move request for internal AI.");
        QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection);
            } else {
                if (!m_engineProcess || m_engineProcess->state() != QProcess::Running) {
                    emit engineError("Engine not loaded or not running.");
                    return;
                }
                qDebug() << QString("AI: Requesting move using external engine."));
                char fen_c[256];
                board8toFEN(&board, fen_c, colorToMove, m_gametype);
                            QString command = QString("position fen %1\ngo movetime %2\n").arg(fen_c).arg(timeLimit * 1000);
                            qDebug() << QString("AI: Sending command to engine: %1").arg(command.trimmed()));
                            qint64 bytesWritten = m_engineProcess->write(command.toUtf8());
                                        if (bytesWritten == -1) {
                                            qCritical() << QString("AI: Failed to write command to engine: %1").arg(m_engineProcess->errorString()));
                                                    } else if (bytesWritten != command.toUtf8().length()) {
                                                        qWarning() << QString("AI: Incomplete command written to engine. Expected %1 bytes, wrote %2 bytes.").arg(command.toUtf8().length()).arg(bytesWritten));
                                                                } else {
                                                                    qDebug() << QString("AI: Successfully wrote %1 bytes to engine.").arg(bytesWritten));
                                                                }            }}

void AI::abortSearch()
{
    m_abortRequested.storeRelaxed(1);
    if (m_engineProcess && m_engineProcess->state() == QProcess::Running) {
        m_engineProcess->write("stop\n");
        qInfo() << "AI: Abort requested. Stop command sent to engine.");
    }
}

void AI::requestAbort()
{
    m_abortRequested.storeRelaxed(1);
    m_geminiAI.requestAbort(); // Request abort in the GeminiAI instance
}

void AI::parseEngineOutput(const QString& output)
{
    qDebug() << QString("AI: Engine Raw Output: %1").arg(output.trimmed()));

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.startsWith("bestmove")) {
            QString moveString = line.section(' ', 1, 1).trimmed();
            if (!moveString.isEmpty()) {
                qDebug() << QString("AI: Parsed bestmove string: %1").arg(moveString));
                CBmove parsedMove = parseMoveString(moveString);
                emit moveFound(parsedMove);
                return;
            }
        } else if (line.startsWith("info")) {
            // Example info line: "info depth 5 score cp 123 nodes 1000"
            int depth = 0;
            int score = 0;

            QRegExp depthRx("depth (\\d+)");
            if (depthRx.indexIn(line) != -1) {
                depth = depthRx.cap(1).toInt();
            }

            QRegExp scoreCpRx("score cp (-?\\d+)");
            QRegExp scoreMateRx("score mate (-?\\d+)");

            if (scoreCpRx.indexIn(line) != -1) {
                score = scoreCpRx.cap(1).toInt();
            } else if (scoreMateRx.indexIn(line) != -1) {
                // Mate scores are typically in moves, convert to a large CP value
                int mateMoves = scoreMateRx.cap(1).toInt();
                score = (mateMoves > 0) ? 100000 - mateMoves : -100000 - mateMoves;
            }
            
            if (depth > 0 || score != 0) { // Only emit if we found meaningful info
                emit evaluationReady(score, depth);
            }
        }
    }
}

CBmove AI::parseMoveString(const QString& moveString)
{
    qDebug() << QString("AI: Parsing move string: %1").arg(moveString));
    CBmove move = {false}; // Initialize with false for is_capture, and zeros for the rest

    QStringList parts;
    if (moveString.contains('-')) {
        parts = moveString.split('-');
    } else if (moveString.contains('x')) {
        parts = moveString.split('x');
    }

            if (parts.size() == 2) {
                int from_square = parts[0].toInt();
                int to_square = parts[1].toInt();
                qDebug() << QString("AI: Parsed from_square: %1, to_square: %2").arg(from_square).arg(to_square));
        int from_x, from_y;
        numbertocoors(from_square, &from_x, &from_y, m_gametype);
        move.from.x = from_x;
        move.from.y = from_y;

        int to_x, to_y;
        numbertocoors(to_square, &to_x, &to_y, m_gametype);
        move.to.x = to_x;
        move.to.y = to_y;

        // Determine if it's a capture based on the 'x' separator or distance
        move.is_capture = moveString.contains('x');

        // For now, assume 1 jump if it's a capture
        if (move.is_capture) {
            move.jumps = 1;
        }
        qDebug() << QString("AI: Parsed CBmove: from(%1,%2) to(%3,%4) is_capture: %5").arg(move.from.x).arg(move.from.y).arg(move.to.x).arg(move.to.y).arg(move.is_capture));
    } else {
        qWarning() << QString("AI: Could not parse move string: %1").arg(moveString));
    }

    return move;
}

bool AI::internalGetMove(Board8x8 board, int colorToMove, double maxtime, char statusBuffer[1024], QAtomicInt *playnow, int info, int moreinfo, CBmove *bestMove)
{
    qDebug() << QString("AI::internalGetMove: Entering with maxtime: %1, color: %2").arg(maxtime).arg(colorToMove));

    // Pass the abort flag to the GeminiAI instance
    if (m_abortRequested.loadRelaxed()) {
        m_geminiAI.requestAbort();
    }

    // Check for EGDB lookup
            int num_pieces = count_pieces(&board);
            if (m_egdbInitialized && num_pieces <= m_maxEGDBPieces) {
                pos current_pos;
                boardtobitboard(&board, &current_pos);
                int egdb_result = egdb_wrapper_lookup(&current_pos, colorToMove);
                if (egdb_result != DB_UNKNOWN && egdb_result != DB_NOT_LOOKED_UP) {
                    // If EGDB provides a definitive result, we can potentially use it.
                    // For now, we'll just log it and proceed with search.
                    qDebug() << QString("AI::internalGetMove: EGDB lookup for %1 pieces returned: %2").arg(num_pieces).arg(egdb_result));
                    // A more sophisticated implementation would use this to guide the search or make an immediate move.
                }
            }
    *bestMove = m_geminiAI.getBestMove(board, colorToMove, maxtime);

    if (m_abortRequested.loadRelaxed()) {
        qInfo() << QString("AI::internalGetMove: Abort requested. Stopping search."));
        return false; // Aborted, no move found
    }
    
    return true;
}

void AI::handleAutoplayState() {
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Autoplay aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }

    // Request a move from the internal AI
    CBmove bestMove;
    char statusBuffer[1024] = {0};
    bool moveFound = this->internalGetMove(m_board, m_color, m_maxtime, statusBuffer, &m_abortRequested, m_info, m_moreinfo, &bestMove);

    if (moveFound) {
        // Emit the move to GameManager to apply it
        emit searchFinished(true, false, bestMove, QString(statusBuffer), 0, "", 0.0);
        // GameManager will then switch turns and potentially call playMove again if AI's turn
    } else {
        qWarning() << "AI: Autoplay: No move found or search aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
    }
}

void AI::handleEngineMatchState() {
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Engine match aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }

    if (m_matchGameOver) {
        m_matchGameNumber++;
        if (m_matchGameNumber >= m_totalMatchGames) {
            qInfo() << "AI: Engine match finished.");
            setMode(Idle);
            emit changeState(STATE_NORMAL);
            return;
        } else {
            qInfo() << QString("AI: Starting game %1 of %2 in engine match.").arg(m_matchGameNumber + 1).arg(m_totalMatchGames));
            emit requestNewGame(GT_ENGLISH); // Request a new game
            m_matchGameOver = false;
            m_matchMoveCount = 0;
            // Reset board and color for the new game
            // This will be handled by GameManager emitting boardUpdated and then AI receiving requestMove
            return;
        }
    }

    // Request a move from the internal AI
    CBmove bestMove;
    char statusBuffer[1024] = {0};
    bool moveFound = this->internalGetMove(m_board, m_color, m_maxtime, statusBuffer, &m_abortRequested, m_info, m_moreinfo, &bestMove);

    if (moveFound) {
        // Emit the move to GameManager to apply it
        emit searchFinished(true, false, bestMove, QString(statusBuffer), 0, "", 0.0);
        m_matchMoveCount++;
        // Check for game over after the move is applied by GameManager
        // For now, we'll assume GameManager will emit gameIsOver if the game ends.
    } else {
        qWarning() << "AI: Engine match: No move found or search aborted.");
        m_matchGameOver = true; // Consider game over if no move found
    }
}

void AI::handleRunTestSetState() {
    // Placeholder implementation for handleRunTestSetState
    qInfo() << "AI: handleRunTestSetState called (placeholder). Full implementation will involve loading test sets, iterating positions, requesting AI moves, comparing to expected moves, and recording results.");
    // For now, just transition to Idle after a short delay to avoid busy-waiting
    QTimer::singleShot(100, this, [this](){
        setMode(Idle);
        emit changeState(STATE_NORMAL);
    });
}

void AI::handleEngineGameState() {
    // In this state, the AI is primarily waiting for the external engine to send its move.
    // The `requestMove` function already sends the position to the engine.
    // The `parseEngineOutput` slot handles the engine's response.
    // This handler ensures the AI is in the correct mode to process that.
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Engine game aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }

    if (!m_useInternalAI && (!m_engineProcess || m_engineProcess->state() != QProcess::Running)) {
        qWarning() << "AI: External engine not running in EngineGame state. Transitioning to Idle.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }
    // qDebug("AI: handleEngineGameState: Waiting for external engine move.");
}

void AI::handleAnalyzeGameState() {
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Analyze game aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }

    // Request analysis from the internal AI
    // The internalGetMove function already performs a search and updates evaluation.
    // We just need to call it repeatedly.
    CBmove bestMove;
    char statusBuffer[1024] = {0};
    // For continuous analysis, we might want to set a very short time limit or a specific depth.
    // For now, let's use the current m_maxtime.
    bool moveFound = this->internalGetMove(m_board, m_color, m_maxtime, statusBuffer, &m_abortRequested, m_info, m_moreinfo, &bestMove);

    // Emit the evaluation score and search depth
    emit evaluationReady(m_geminiAI.getLastEvaluationScore(), m_geminiAI.getLastSearchDepth());

    if (!moveFound && m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Analyze game: Search aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
    }
    // If moveFound is false but not aborted, it means no legal moves, which should be handled by GameManager.
}

void AI::handleAnalyzePdnState() {
    // Placeholder implementation for handleAnalyzePdnState
    qInfo() << "AI: handleAnalyzePdnState called (placeholder). Full implementation will involve loading PDN games, iterating through moves, analyzing positions, and emitting evaluation.");
    // For now, just transition to Idle after a short delay to avoid busy-waiting
    QTimer::singleShot(100, this, [this](){
        setMode(Idle);
        emit changeState(STATE_NORMAL);
    });
}

void AI::handleObserveGameState() {
    if (m_abortRequested.loadRelaxed()) {
        qInfo() << "AI: Observe game aborted.");
        setMode(Idle);
        emit changeState(STATE_NORMAL);
        return;
    }

    // In observe mode, the AI doesn't make moves, but it can analyze the current board.
    // We can reuse the analysis logic.
    CBmove bestMove;
    char statusBuffer[1024] = {0};
    // For continuous analysis, we might want to set a very short time limit or a specific depth.
    // For now, let's use the current m_maxtime.
    this->internalGetMove(m_board, m_color, m_maxtime, statusBuffer, &m_abortRequested, m_info, m_moreinfo, &bestMove);

    // Emit the evaluation score and search depth
    emit evaluationReady(m_geminiAI.getLastEvaluationScore(), m_geminiAI.getLastSearchDepth());

    // qDebug("AI: handleObserveGameState: Observing game and providing analysis.");
}

void AI::setMode(AI_State newMode) {
    m_mode = newMode;
    qDebug() << QString("AI: Mode set to %1").arg(newMode));
}

AI::AI(const QString& egdbPath, QObject *parent) : QObject(parent),
    m_useInternalAI(true),
    m_engineProcess(nullptr),
    m_abortRequested(0),
    m_playnow_shim(0),
    m_gametype(GT_ENGLISH),
    m_egdbInitialized(false), // This will be set by GeminiAI's constructor
    m_maxEGDBPieces(0), // This will be set by GeminiAI's constructor
    m_stateMachineTimer(new QTimer(this)), // Initialize the timer
    m_mode(Idle),
    m_startMatchFlag(false),
    m_matchGameNumber(0),
    m_matchMoveCount(0),
    m_matchGameOver(false),
    m_totalMatchGames(0),
    m_geminiAI(egdbPath, this) // Initialize m_geminiAI with egdbPath
{
    qDebug() << "AI: Constructor called.");
    connect(m_stateMachineTimer, &QTimer::timeout, this, &AI::runStateMachine);
    m_stateMachineTimer->start(100); // Call runStateMachine every 100ms

    // Run move generation tests
    AI::testMoveGeneration();
}

AI::~AI()
{
    qDebug() << "AI: Destructor called.");
    // Cleanup EGDB if initialized
    // egdb_wrapper_exit(); // Example
    if (m_stateMachineTimer) {
        m_stateMachineTimer->stop();
        // m_stateMachineTimer is a child of AI, so it will be deleted automatically
        // delete m_stateMachineTimer; // No need to delete manually if parent is set
        m_stateMachineTimer = nullptr;
    }
    if (m_engineProcess) {
        m_engineProcess->terminate();
        m_engineProcess->waitForFinished(1000);
        delete m_engineProcess;
        m_engineProcess = nullptr;
    }
}

void AI::move_to_pdn_english(const Board8x8& board, int color, const CBmove* move, char* pdn_c, int gametype) {
    // This is a placeholder. A full implementation would convert the move to PDN English notation.
    // For now, we'll just use the simple move4tonotation.
    move4tonotation(move, pdn_c);
    qDebug() << QString("AI: move_to_pdn_english called (placeholder). Move: %1").arg(pdn_c));
}

void AI::get_movelist_from_engine(const Board8x8& board, int color, CBmove* movelist, int* nmoves, int* iscapture) {
    // This is a placeholder. A full implementation would query the external engine for legal moves.
    // For now, we'll just use the internal move generation.
    pos p;
    boardtobitboard(&board, &p);
    bool dummy_can_continue_multijump = false;
    get_legal_moves_c(&p, color, movelist, nmoves, iscapture, NULL, &dummy_can_continue_multijump);
    qDebug() << QString("AI: get_movelist_from_engine called (placeholder). Found %1 moves.").arg(*nmoves));
}

void AI::move_to_pdn_english_from_list(int nmoves, CBmove* movelist, const CBmove* move, char* pdn_c, int gametype) {
    // This is a placeholder. A full implementation would convert the move to PDN English notation from a list.
    // For now, we'll just use the simple move4tonotation.
    move4tonotation(move, pdn_c);
    qDebug() << QString("AI: move_to_pdn_english_from_list called (placeholder). Move: %1").arg(pdn_c));
}

// --- Stubs for missing AI functions ---

void AI::initEngineProcess()
{
    qDebug() << "AI::initEngineProcess: Initializing engine process.");
    if (!m_engineProcess) {
        m_engineProcess = new QProcess();
        connect(m_engineProcess, &QProcess::readyReadStandardOutput, this, [this](){
            parseEngineOutput(m_engineProcess->readAllStandardOutput());
        });
        connect(m_engineProcess, &QProcess::readyReadStandardError, this, [this](){
            qCritical() << QString("AI: Engine Error: %1").arg(QString(m_engineProcess->readAllStandardError())));
            emit engineError(m_engineProcess->readAllStandardError());
        });
        connect(m_engineProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus){
            qInfo() << QString("AI: Engine process finished with exit code %1, status %2.").arg(exitCode).arg(exitStatus));
            if (exitStatus == QProcess::CrashExit) {
                emit engineError("Engine process crashed.");
            }
        });
    }
}

void AI::quitEngineProcess()
{
    qInfo() << "AI::quitEngineProcess: Quitting engine process.");
    if (m_engineProcess) {
        m_engineProcess->terminate();
        m_engineProcess->waitForFinished(1000); // Give it some time to terminate
        if (m_engineProcess->state() != QProcess::NotRunning) {
            m_engineProcess->kill(); // Force kill if it didn't terminate
        }
        // m_engineProcess is deleted in the destructor, no need to delete here
        // m_engineProcess = nullptr; // Set to nullptr to avoid dangling pointer if not deleted
    }
}

void AI::doWork()
{
    qDebug() << "AI::doWork: Starting search.");
    QElapsedTimer totalTimer;
    totalTimer.start();

    CBmove bestMove;
    char statusBuffer[1024] = {0};
    bool moveFound = this->internalGetMove(m_board, m_color, m_maxtime, statusBuffer, &m_abortRequested, m_info, m_moreinfo, &bestMove);

    // Emit the evaluation score and search depth
    emit evaluationReady(m_geminiAI.getLastEvaluationScore(), m_geminiAI.getLastSearchDepth());

    double elapsedTime = totalTimer.elapsed() / 1000.0;
    int gameResult = 0; // Placeholder, determine actual game result if applicable

    if (moveFound) {
        qInfo() << QString("AI::doSearch: Search finished. Best move found: from(%1,%2) to(%3,%4)").arg(bestMove.from.x).arg(bestMove.from.y).arg(bestMove.to.x).arg(bestMove.to.y));
        emit searchFinished(true, m_abortRequested.loadRelaxed(), bestMove, QString(statusBuffer), gameResult, "", elapsedTime);
    } else {
        qWarning() << "AI::doSearch: Search finished. No move found or aborted.");
        emit searchFinished(false, m_abortRequested.loadRelaxed(), bestMove, QString(statusBuffer), gameResult, "", elapsedTime);
    }
}





void AI::setSearchParameters(Board8x8 board, int color, double maxtime,
                             int info, int moreinfo,
                             int (*engineGetMoveFunc)(Board8x8, int, double, char*, int*, int, int, CBmove*),
                             CB_ENGINECOMMAND engineCommandFunc,
                             int gametype)
{
    memcpy(&m_board, &board, sizeof(Board8x8));
    m_color = color;
    m_maxtime = maxtime;
    m_info = info;
    m_moreinfo = moreinfo;
    m_engineGetMoveFunc = engineGetMoveFunc;
    m_engineCommandFunc = engineCommandFunc;
    m_gametype = gametype;
}







bool AI::sendCommand(const QString& command, QString& reply)
{
    if (!m_engineProcess || m_engineProcess->state() != QProcess::Running) {
        reply = "Error: Engine not loaded or not running.";
        qCritical() << QString("AI: sendCommand failed: %1").arg(reply));
        return false;
    }

    qDebug() << QString("AI: Sending command to engine: %1").arg(command.trimmed()));
    m_engineProcess->write(command.toUtf8() + "\n");
    m_engineProcess->waitForBytesWritten();

    // Wait for a response, or a timeout
    if (m_engineProcess->waitForReadyRead(5000)) { // 5 second timeout
        reply = m_engineProcess->readAllStandardOutput();
        qDebug() << QString("AI: Engine response: %1").arg(reply.trimmed()));
        return true;
    } else {
        reply = "Error: No response from engine within timeout.";
        qCritical() << QString("AI: sendCommand failed: %1").arg(reply));
        return false;
    }
}

void AI::runStateMachine()
{
    // This function will be called periodically by a QTimer
    // to manage the AI's different operational modes.
    switch (m_mode) {
        case Autoplay:
            handleAutoplayState();
            break;
        case EngineMatch:
            handleEngineMatchState();
            break;
        case RunTestSet:
            handleRunTestSetState();
            break;
        case EngineGame:
            handleEngineGameState();
            break;
        case AnalyzeGame:
            handleAnalyzeGameState();
            break;
        case AnalyzePdn:
            handleAnalyzePdnState();
            break;
        case ObserveGame:
            handleObserveGameState();
            break;
        case Idle:
        default:
            // Do nothing or log unexpected call
            // qDebug("AI::runStateMachine: Called in Idle or unknown mode.");
            break;
    }
}

void AI::startAnalyzeGame()
{
    qInfo() << "AI: Starting Analyze Game mode.");
    setMode(AnalyzeGame);
}

void AI::startAnalyzePdn()
{
    qInfo() << "AI: Starting Analyze PDN mode.");
    setMode(AnalyzePdn);
}

void AI::startAutoplay(const Board8x8& board, int color)
{
    qInfo() << "AI: Starting Autoplay mode.");
    m_board = board;
    m_color = color;
    setMode(Autoplay);
}

void AI::startEngineMatch(int totalGames)
{
    qInfo() << QString("AI: Starting Engine Match mode for %1 games.").arg(totalGames));
    m_totalMatchGames = totalGames;
    m_matchGameNumber = 0;
    m_matchMoveCount = 0;
    m_matchGameOver = false;
    setMode(EngineMatch);
}

void AI::startRunTestSet()
{
    qInfo() << "AI: Starting Run Test Set mode.");
    setMode(RunTestSet);
}

void AI::startEngineGame()
{
    qInfo() << "AI: Starting Engine Game mode.");
    setMode(EngineGame);
}

void AI::startObserveGame()
{
    qInfo() << "AI: Starting Observe Game mode.");
    setMode(ObserveGame);
}

// Piece-Square Tables (PSTs) for evaluation
const int AI::whiteManPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {0, 10, 10, 10, 10, 10, 10,  0},
    {0, 10, 20, 20, 20, 20, 10,  0},
    {0, 10, 20, 30, 30, 20, 10,  0},
    {0, 10, 20, 30, 30, 20, 10,  0},
    {0, 10, 20, 20, 20, 20, 10,  0},
    {0, 10, 10, 10, 10, 10, 10,  0},
    {0,  0,  0,  0,  0,  0,  0,  0}
};

const int AI::whiteKingPST[8][8] = {
    {0, 30, 30, 30, 30, 30, 30,  0},
    {0, 30, 40, 40, 40, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 40, 40, 40, 30,  0},
    {0, 30, 30, 30, 30, 30, 30,  0},
    {0,  0,  0,  0,  0,  0,  0,  0}
};

const int AI::blackManPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {0, 10, 10, 10, 10, 10, 10,  0},
    {0, 10, 20, 20, 20, 20, 10,  0},
    {0, 10, 20, 30, 30, 20, 10,  0},
    {0, 10, 20, 30, 30, 20, 10,  0},
    {0, 10, 20, 20, 20, 20, 10,  0},
    {0, 10, 10, 10, 10, 10, 10,  0},
    {0,  0,  0,  0,  0,  0,  0,  0}
};

const int AI::blackKingPST[8][8] = {
    {0, 30, 30, 30, 30, 30, 30,  0},
    {0, 30, 40, 40, 40, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 50, 50, 40, 30,  0},
    {0, 30, 40, 40, 40, 40, 30,  0},
    {0, 30, 30, 30, 30, 30, 30,  0},
    {0,  0,  0,  0,  0,  0,  0,  0}
};

int AI::evaluate(const Board8x8& board)
{
    int score = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int piece = board.board[r][c];
            switch (piece) {
                case (CB_WHITE | CB_MAN):
                    score += 100; // Material value
                    score += whiteManPST[r][c]; // PST value
                    break;
                case (CB_WHITE | CB_KING):
                    score += 300; // Material value
                    score += whiteKingPST[r][c]; // PST value
                    break;
                case (CB_BLACK | CB_MAN):
                    score -= 100; // Material value
                    score -= blackManPST[r][c]; // PST value
                    break;
                case (CB_BLACK | CB_KING):
                    score -= 300; // Material value
                    score -= blackKingPST[r][c]; // PST value
                    break;
                default:
                    break;
            }
        }
    }
    return score;
}

// Standalone function to test move generation
void AI::testMoveGeneration() {
    qInfo() << "--- Starting Move Generation Tests ---");

    struct TestPosition {
        const char* fen;
        int colorToMove;
        QList<QString> expectedMoves;
    };

    QList<TestPosition> testPositions;

    // Test 1: Initial position, Black to move
    testPositions.append({
        "B:W21,22,23,24,25,26,27,28,29,30,31,32:B1,2,3,4,5,6,7,8,9,10,11,12",
        CB_BLACK,
        {"9-13", "9-14", "10-14", "10-15", "11-15", "11-16", "12-16"}
    });

    // Test 2: Simple White move
    testPositions.append({
        "W:W13,14,15,16,17,18,19,20,21,22,23,24:B1,2,3,4,5,6,7,8,9,10,11,12",
        CB_WHITE,
        {"21-17", "21-18", "22-17", "22-18", "22-19", "23-18", "23-19", "23-20", "24-19", "24-20"}
    });

    // Test 3: Simple Black capture
    testPositions.append({
        "B:W14:B10",
        CB_BLACK,
        {"10x17"} // Assuming 10x17 is the only legal move
    });

    // Test 4: Simple White capture
    testPositions.append({
        "W:W10:B14",
        CB_WHITE,
        {"10x17"} // Assuming 10x17 is the only legal move
    });

    // Test 5: Multi-jump for Black
    testPositions.append({
        "B:W14,15,18:B10",
        CB_BLACK,
        {"10x17x24"} // Assuming 10x17x24 is the only legal move
    });

    // Test 6: Multi-jump for White
    testPositions.append({
        "W:W10:B14,15,18",
        CB_WHITE,
        {"10x17x24"} // Assuming 10x17x24 is the only legal move
    });

    int passedTests = 0;
    for (int i = 0; i < testPositions.size(); ++i) {
        const TestPosition& tp = testPositions.at(i);
        qDebug() << QString("Test %1: Position FEN: %2, Color: %3").arg(i + 1).arg(tp.fen).arg(tp.colorToMove == CB_BLACK ? "Black" : "White"));

        Board8x8 board;
        int colorToMove;
        FENtoboard8(&board, tp.fen, &colorToMove, GT_ENGLISH);

        pos currentPos;
        boardtobitboard(&board, &currentPos);
        CBmove movelist[MAXMOVES];
        int numMoves = 0;
        int isjump = 0;
        bool can_continue_multijump = false;
        get_legal_moves_c(&currentPos, colorToMove, movelist, &numMoves, &isjump, NULL, &can_continue_multijump);

        QList<QString> generatedMoves;
        for (int k = 0; k < numMoves; ++k) {
            char move_notation[80];
            move4tonotation(&movelist[k], move_notation);
            generatedMoves.append(QString(move_notation));
        }
        std::vector<QString> generatedMovesVec(generatedMoves.begin(), generatedMoves.end());
        std::sort(generatedMovesVec.begin(), generatedMovesVec.end());
        generatedMoves = QList<QString>(generatedMovesVec.begin(), generatedMovesVec.end());

        QList<QString> mutableExpectedMoves = tp.expectedMoves; // Create a mutable copy
        std::vector<QString> expectedMovesVec(mutableExpectedMoves.begin(), mutableExpectedMoves.end());
        std::sort(expectedMovesVec.begin(), expectedMovesVec.end());
        mutableExpectedMoves = QList<QString>(expectedMovesVec.begin(), expectedMovesVec.end());

        qDebug() << QString("  Generated Moves: %1").arg(generatedMoves.join(", ")));
        qDebug() << QString("  Expected Moves: %1").arg(mutableExpectedMoves.join(", ")));

        if (generatedMoves == mutableExpectedMoves) {
            qInfo() << "  Test Passed!");
            passedTests++;
        } else {
            qCritical() << "  Test FAILED!");
        }
    }

    qInfo() << QString("--- Move Generation Tests Finished: %1/%2 Passed ---").arg(passedTests).arg(testPositions.size()));
}
