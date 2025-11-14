#ifndef EGDBMANAGER_H
#define EGDBMANAGER_H

#include <QString>
#include "checkers_types.h"

class EgdbManager
{
public:
    EgdbManager();
    ~EgdbManager();

    // Initializes the EGDB with the given path
    bool init(const QString& egdbPath);

    // Performs an EGDB lookup for the given FEN position
    PDN_RESULT lookup(const QString& fenPosition);

private:
    // Helper function to parse FEN string into position struct and determine side to move
    int fen_to_position(const char* fen_position, pos* p, int* side_to_move);
};

#endif // EGDBMANAGER_H
