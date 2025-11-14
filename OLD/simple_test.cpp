#include "c_logic.h"
#include <stdio.h>
#include <string.h>

int main() {
    // Initialize the EGDB with the path to the database files.
    if (egdb_wrapper_init("./db") != 0) {
        fprintf(stderr, "Failed to initialize Kingsrow EGDB.\n");
        return 1;
    }

    // Test a 2-piece position - Draw
    char fen1[] = "W:K1:K32";
    Board8x8 board1;
    pos position1;
    int color1;

    // Convert FEN to Board8x8
    FENtoboard8(board1, fen1, &color1, 0); // Assuming gametype 0 for now

    // Convert Board8x8 to pos struct
    boardtobitboard(board1, &position1);

    int result1 = egdb_wrapper_lookup(&position1, color1);
    printf("Result for FEN %s: %d\n", fen1, result1);

    return 0;
}
