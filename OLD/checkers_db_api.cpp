#include "checkers_db_api.h"
#include "checkers_types.h"
#include <stdio.h>
#include <string.h>

extern "C" {
#include "dblookup.h"
}

// Implementation for Kingsrow EGDB initialization
int egdb_init(const char* egdb_path) {
    printf("EGDB: Initializing with path: %s\n", egdb_path);
    // Call the actual Kingsrow EGDB initialization function.
    // wld_cache and mtc_cache are set to 0 for now, assuming they are flags or simple integer values.
    return db_init(64, (char*)egdb_path); // Assuming 64MB as a default suggested size
}

// Helper function to parse FEN string into position struct and determine side to move
// Returns 0 on success, non-zero on failure
int fen_to_position(const char* fen_position, position* p, int* side_to_move) {
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
            // Convert 1-32 board notation to 0-31 bitboard index
            // Kingsrow uses 0-31 for bitboards, where 0 is bottom-left (A1) and 31 is top-right (H8)
            // Standard checkers notation (1-32) is usually from black's perspective (bottom to top, left to right)
            // Need to map this correctly. For now, assuming direct mapping to 0-31.
            // This might need adjustment based on actual Kingsrow bitboard representation.
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

// Implementation for Kingsrow EGDB lookup
int egdb_lookup(const char* fen_position) {
    printf("EGDB: Looking up FEN: %s\n", fen_position);

    position p;
    int side_to_move;
    if (fen_to_position(fen_position, &p, &side_to_move) != 0) {
        fprintf(stderr, "Error parsing FEN position: %s\n", fen_position);
        return PDN_RESULT_UNKNOWN;
    }

    int info; // This will hold additional info from db_lookup, if any
    int result = dblookup(&p, side_to_move);

    // Map db_lookup results to PDN_RESULT
    switch (result) {
        case DB_WIN:
            return PDN_RESULT_WIN;
        case DB_LOSS:
            return PDN_RESULT_LOSS;
        case DB_DRAW:
            return PDN_RESULT_DRAW;
        case DB_UNAVAILABLE:
            return PDN_RESULT_UNKNOWN; // Or a specific UNAVAILABLE result if defined
        case DB_UNKNOWN:
        default:
            return PDN_RESULT_UNKNOWN;
    }
}