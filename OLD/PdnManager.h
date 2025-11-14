#ifndef PDNMANAGER_H
#define PDNMANAGER_H

#include "checkers_types.h"
#include <vector>
#include <QString>

class PdnManager
{
public:
    PdnManager();
    ~PdnManager();

    // Loads PDN games from a file and populates the provided vector
    bool loadPdnFile(const QString &filename, std::vector<PdnGameWrapper> &games);
    static int PDNparseMove(char *token, Squarelist &squares);

private:
    // Private helper functions for parsing, moved from PDNparser.cpp
    int PDNparseGetnumberofgames(char *filename);
    int PDNparseGetnextgame(char **start, char *game, int maxlen);
    int PDNparseGetnextheader(char **start, char *header, int maxlen);
    int PDNparseGetnexttag(char **start, char *tag, int maxlen);
    int PDNparseGetnexttoken(char **start, char *token, int maxlen);
    void PDNgametoPDNstring(PdnGameWrapper &game, std::string &pdnstring, const char *lineterm);
    inline bool is_pdnquote(uint8_t c);
};

#endif // PDNMANAGER_H
