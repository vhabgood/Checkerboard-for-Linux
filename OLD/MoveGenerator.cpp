#include "MoveGenerator.h"
#include "c_logic.h"
#include <assert.h>
#include <string.h>

MoveGenerator::MoveGenerator()
{
    // Constructor
}

static inline int cbcolor_to_getmovelistcolor_internal(int cbcolor)
{
    if (cbcolor == CB_BLACK)
        return(1);
    else
        return(-1);
}

void MoveGenerator::generateMoves(pos *p, int color, CBmove M[MAXMOVES], int *n, bool onlyCaptures)
{
    int i, j;
    int board12[12][12];
    Board8x8 b8;
    int isjump_local = 0; // Initialize isjump_local

    bitboardtoboard8(p, &b8); // Convert pos to Board8x8
    board8toboard12(b8, board12);

    assert(color == CB_BLACK || color == CB_WHITE);

    *n = 0; // Initialize n to 0

    // Call makemovelist
    makemovelist(cbcolor_to_getmovelistcolor_internal(color), M, board12, &isjump_local, n);

    if (onlyCaptures && isjump_local == 0) {
        // If only captures are requested, but no jumps were found, clear the movelist
        *n = 0;
        return;
    }
    // If onlyCaptures is true and isjump_local is 1, then movelist already contains only captures.
    // If onlyCaptures is false, movelist contains all moves (captures first, then non-captures).

    // now: do something to the coordinates, so that the moves are in a 8x8-format
    for (i = 0; i < *n; i++) { // Use *n for the loop limit
        M[i].from.x -= 2;
        M[i].to.x -= 2;
        M[i].from.y -= 2;
        M[i].to.y -= 2;
        for (j = 0; j < 11; j++) {
            M[i].path[j].x -= 2;
            M[i].path[j].y -= 2;
            M[i].del[j].x -= 2;
            M[i].del[j].y -= 2;
        }
    }

    // and set the pieces to CB-format
    for (i = 0; i < *n; i++) { // Use *n for the loop limit
        switch (M[i].oldpiece) {
        case -2:
            M[i].oldpiece = (CB_WHITE | CB_KING);
            break;

        case -1:
            M[i].oldpiece = (CB_WHITE | CB_MAN);
            break;

        case 1:
            M[i].oldpiece = (CB_BLACK | CB_MAN);
            break;

        case 2:
            M[i].oldpiece = (CB_BLACK | CB_KING);
            break;
        }

        switch (M[i].newpiece) {
        case -2:
            M[i].newpiece = (CB_WHITE | CB_KING);
            break;

        case -1:
            M[i].newpiece = (CB_WHITE | CB_MAN);
            break;

        case 1:
            M[i].newpiece = (CB_BLACK | CB_MAN);
            break;

        case 2:
            M[i].newpiece = (CB_BLACK | CB_KING);
            break;
        }

        for (j = 0; j < M[i].jumps; j++) {
            switch (M[i].delpiece[j]) {
            case -2:
                M[i].delpiece[j] = (CB_WHITE | CB_KING);
                break;

            case -1:
                M[i].delpiece[j] = (CB_WHITE | CB_MAN);
                break;

            case 1:
                M[i].delpiece[j] = (CB_BLACK | CB_MAN);
                break;

            case 2:
                M[i].delpiece[j] = (CB_BLACK | CB_KING);
                break;
            }
        }
    }
}

void MoveGenerator::board8toboard12(Board8x8 board8, int board12[12][12])
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

int MoveGenerator::makemovelist(int color, CBmove movelist[MAXMOVES], int board[12][12], int *isjump, int *n)
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

                    /* search for moves with black kings */
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

void MoveGenerator::whitecapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
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

            //whitecapture(b,movelist,mm,x+2,y+2,d+1,n);
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

            //whitecapture(b,movelist,mm,x-2,y+2,d+1,n);
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

void MoveGenerator::whitekingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
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

        //whitekingcapture(b,movelist,mm,x+2,y+2,d+1,n);
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

        //whitekingcapture(b,movelist,mm,x-2,y+2,d+1,n);
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

        //whitekingcapture(b,movelist,mm,x+2,y-2,d+1,n);
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

        //whitekingcapture(b,movelist,mm,x-2,y-2,d+1,n);
        end = 0;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = 2;
        (*n)++;
    }
}

void MoveGenerator::blackcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
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

            //blackcapture(b, movelist, mm, x + 2, y - 2, d + 1, n);
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

            //blackcapture(b, movelist, mm, x - 2, y - 2, d + 1, n);
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

void MoveGenerator::blackkingcapture(int board[12][12], CBmove movelist[MAXMOVES], CBmove m, int x, int y, int d, int *n)
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

        //blackkingcapture(b, movelist, mm, x + 2, y + 2, d + 1, n);
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

        //blackkingcapture(b, movelist, mm, x - 2, y + 2, d + 1, n);
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

        //blackkingcapture(b, movelist, mm, x + 2, y - 2, d + 1, n);
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

        //blackkingcapture(b, movelist, mm, x - 2, y - 2, d + 1, n);
        end = 0;
    }

    if (end) {
        m.jumps = d;
        movelist[*n] = m;
        movelist[*n].oldpiece = -2;
        (*n)++;
    }
}

bool MoveGenerator::isLegalMove(const Board8x8 board, int color, int fromX, int fromY, int toX, int toY, CBmove *move, int gametype)
{
    CBmove movelist[MAXMOVES];
    int n;
    pos p;
    boardtobitboard(&board, &p);

    generateMoves(&p, color, movelist, &n);

    for (int i = 0; i < n; i++)
    {
        if (movelist[i].from.x == fromX && movelist[i].from.y == fromY)
        {
            if (movelist[i].to.x == toX && movelist[i].to.y == toY)
            {
                *move = movelist[i];
                return true;
            }
        }
    }

    return false;
}