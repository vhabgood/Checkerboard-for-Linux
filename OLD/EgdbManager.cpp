#include "EgdbManager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <QDebug>

extern "C" {
#include "dblookup.h"
}

#include "checkers_types.h"

EgdbManager::EgdbManager()
{
    // Constructor
}

EgdbManager::~EgdbManager()
{
    // Destructor
}

bool EgdbManager::init(const QString& egdbPath)
{
    qDebug() << "EGDB: Initializing with path:" << egdbPath;
    QByteArray ba = egdbPath.toUtf8();
    // Call the actual Kingsrow EGDB initialization function.
    // wld_cache and mtc_cache are set to 0 for now, assuming they are flags or simple integer values.
    return db_init(64, ba.data()) == 0; // Assuming 64MB as a default suggested size
}

int EgdbManager::fen_to_position(const char* fen_position, pos* p, int* side_to_move)
{
    if (!fen_position || !p || !side_to_move) {
        return -1; // Invalid input
    }

    p->bm = 0;
    p->bk = 0;
    p->wm = 0;
    p->wk = 0;

    char fen_copy[256];
    strncpy(fen_copy, fen_position, sizeof(fen_copy) - 1);
    fen_copy[sizeof(fen_copy) - 1] = '\0';

    char* token = strtok(fen_copy, ":");
    if (!token) return -1; // Missing side to move

    // Parse side to move
    if (strcmp(token, "W") == 0) {
        *side_to_move = WHITE;
    } else if (strcmp(token, "B") == 0) {
        *side_to_move = BLACK;
    } else {
        return -1; // Invalid side to move
    }

    // Parse white pieces
    token = strtok(NULL, ":");
    if (!token) return -1; // Missing white pieces
    char* piece_token = strtok(token, ",");
    while (piece_token) {
        int square = atoi(piece_token);
        if (square >= 1 && square <= 32) {
            if (piece_token[0] == 'K') { // King
                p->wk |= (1 << (square - 1));
            } else { // Man
                p->wm |= (1 << (square - 1));
            }
        }
        piece_token = strtok(NULL, ",");
    }

    // Parse black pieces
    token = strtok(NULL, ":");
    if (!token) return -1; // Missing black pieces
    piece_token = strtok(token, ",");
    while (piece_token) {
        int square = atoi(piece_token);
        if (square >= 1 && square <= 32) {
            if (piece_token[0] == 'K') { // King
                p->bk |= (1 << (square - 1));
            } else { // Man
                p->bm |= (1 << (square - 1));
            }
        }
        piece_token = strtok(NULL, ",");
    }

    return 0; // Success
}

PDN_RESULT EgdbManager::lookup(const QString& fenPosition)
{
    qDebug() << "EGDB: Looking up FEN:" << fenPosition;

    pos p;
    int side_to_move;
    QByteArray ba = fenPosition.toUtf8();

    if (fen_to_position(ba.data(), &p, &side_to_move) != 0) {
        qWarning() << "Error parsing FEN position:" << fenPosition;
        return PDN_RESULT_UNKNOWN;
    }

    int result = dblookup((POSITION*)&p, side_to_move);

    switch (result) {
        case DB_WIN:
            return PDN_RESULT_WIN;
        case DB_LOSS:
            return PDN_RESULT_LOSS;
        case DB_DRAW:
            return PDN_RESULT_DRAW;
        case DB_UNAVAILABLE:
            return PDN_RESULT_UNAVAILABLE;
        case DB_UNKNOWN:
        default:
            return PDN_RESULT_UNKNOWN;
    }
}
