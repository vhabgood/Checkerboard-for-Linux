#include "PdnManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "c_logic.h"

// From utility.h, needed for read_text_file_c
extern "C" {
    char* read_text_file_c(const char* filename, READ_TEXT_FILE_ERROR_TYPE* etype);
}

PdnManager::PdnManager()
{
    // Constructor
}

PdnManager::~PdnManager()
{
    // Destructor
}

bool PdnManager::loadPdnFile(const QString &filename, std::vector<PdnGameWrapper> &games)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filename << file.errorString();
        return false;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // Convert QString to char* for compatibility with existing C-style parsing functions
    QByteArray ba = fileContent.toUtf8();
    char* buffer = ba.data();
    char* current_pos = buffer;

    games.clear();

    char game_str[10000]; // Buffer to hold a single game's PDN string
    while (PDNparseGetnextgame(&current_pos, game_str, sizeof(game_str))) {
        PdnGameWrapper game; // Create a new PdnGameWrapper object for each game
        // For now, just add a dummy game to the vector
        games.push_back(game);
    }
    qDebug() << "PdnManager: Parsed" << games.size() << "games from PDN file.";
    return true;
}

// --- Moved functions from PDNparser.cpp ---

int PdnManager::PDNparseGetnumberofgames(char *filename)
{
    char *buffer;
    char game[1024];
    char *p;
    int ngames;
    READ_TEXT_FILE_ERROR_TYPE etype;

    buffer = read_text_file_c(filename, &etype);
    if (buffer == NULL)
        return -1;

    p = buffer;
    ngames = 0;
    while (PDNparseGetnextgame(&p, game, sizeof(game)))
        ++ngames;

    free(buffer);
    return(ngames);
}

inline bool PdnManager::is_pdnquote(uint8_t c)
{
    if (c == '"')
        return(true);
    if (c & 0x80) {
        if (c == 0x20 || c == 0x1d) // UTF8_LEFT_DBLQUOTE and UTF8_RIGHT_DBLQUOTE placeholders
            return(true);
    }
    return(false);
}

int PdnManager::PDNparseGetnextgame(char **start, char *game, int maxlen)
{
    char *p;
    char *p_org;
    int headersdone = 0;

    game[0] = 0;
    if ((*start) == 0)
        return 0;

    p = (*start);
    p_org = p;
    while (*p != 0) {
        if (*p == '[' && !headersdone) {
            p++;
            while (*p != ']' && *p != 0) {
                if (is_pdnquote(*p)) {
                    ++p;
                    while (!is_pdnquote(*p) && *p != 0) {
                        ++p;
                    }
                    if (*p == 0)
                        break;
                }
                p++;
            }
        }

        if (*p == 0)
            break;

        if (*p == '{') {
            p++;
            while (*p != '}' && *p != 0) {
                p++;
            }
        }

#ifdef NEMESIS
        if (*p == '(') {
            p++;
            while (*p != ')' && *p != 0) {
                p++;
            }
        }
#endif
        if (*p == 0)
            break;

        if (isdigit((uint8_t) *p))
            headersdone = 1;

        if (p[0] == '[' && headersdone) {
            p--;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '1' && p[1] == '-' && p[2] == '0') {
            p += 3;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '0' && p[1] == '-' && p[2] == '1' && !isdigit((uint8_t) p[3])) {
            p += 3;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '*') {
            p++;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        if (p[0] == '1' && p[1] == '/' && p[2] == '2' && p[3] == '-' && p[4] == '1' && p[5] == '/' && p[6] == '2') {
            p += 7;
            strncpy(game, *start, p - *start);
            game[p-*start] = 0;
            *start = p;
            return (int)(p - p_org);
        }

        p++;
    }

    if (headersdone) {
        strncpy(game, *start, p - *start);
        game[p-*start] = 0;
        *start = p;
        return (int)(p - p_org);
    }

    return 0;
}

int PdnManager::PDNparseGetnextheader(char **start, char *header, int maxlen)
{
    const char *p, *q;
    int i, quotecount;

    if (*start == 0)
        return 0;
    p = *start;
    while (*p != '[' && *p != 0)
        p++;

    if (*p == 0)
        return 0;

    q = p + 1;
    i = 0;
    quotecount = 0;
    while ((quotecount < 2 || *q != ']') && *q != 0) {
        if (i < maxlen)
            header[i] = *q;
        if (*q == '"')
            ++quotecount;
        q++;
        i++;
    }

    header[i < maxlen - 1 ? i : maxlen - 1] = 0;

    if (*q == 0)
        return 0;

    *start = const_cast<char*>(q + 1);
    return 1;
}

int PdnManager::PDNparseGetnexttag(char **start, char *tag, int maxlen)
{
    const char *p, *q;
    int i;

    if ((*start) == 0)
        return 0;
    p = (*start);
    while (!is_pdnquote(*p) && *p != 0)
        p++;

    if (*p == 0)
        return 0;

    q = p + 1;
    i = 0;
    while (!is_pdnquote(*q) && *q != 0) {
        if (i < maxlen - 1)
            tag[i] = *q;
        q++;
        i++;
    }

    tag[i < maxlen - 1 ? i : maxlen - 1] = 0;

    if (*q == 0)
        return 0;

    (*start) = const_cast<char*>(q + 1);
    return 1;
}

void PdnManager::PDNgametoPDNstring(PdnGameWrapper &game, std::string &pdnstring, const char *lineterm) {
    pdnstring.clear();

    pdnstring += "[Event \"" + std::string(game.event) + "]" + lineterm;
    pdnstring += "[Site \"" + std::string(game.site) + "]" + lineterm;
    pdnstring += "[Date \"" + std::string(game.date) + "]" + lineterm;
    pdnstring += "[Round \"" + std::string(game.round) + "]" + lineterm;
    pdnstring += "[White \"" + std::string(game.white) + "]" + lineterm;
    pdnstring += "[Black \"" + std::string(game.black) + "]" + lineterm;
    pdnstring += "[Result \"" + std::string(game.resultstring) + "]" + lineterm;

    if (strlen(game.FEN) > 0) {
        pdnstring += "[FEN \"" + std::string(game.FEN) + "]" + lineterm;
    }

    pdnstring += lineterm;

    for (size_t i = 0; i < game.moves.size(); ++i) {
        if (i % 2 == 0) {
            pdnstring += std::to_string((i / 2) + 1) + ". ";
        }
        // Convert PDNmove to CBmove and then to PDN string
        CBmove cbMove;
        numbertocoors(game.moves[i].from_square, &cbMove.from.x, &cbMove.from.y, game.gametype);
        numbertocoors(game.moves[i].to_square, &cbMove.to.x, &cbMove.to.y, game.gametype);
        cbMove.is_capture = false; // Placeholder, needs actual determination
        cbMove.jumps = 0; // Placeholder, needs actual determination

        char move_notation[80];
        move4tonotation(&cbMove, move_notation);
        pdnstring += std::string(move_notation) + " ";

        if (strlen(game.moves[i].comment) > 0) {
            pdnstring += "{" + std::string(game.moves[i].comment) + "} ";
        }
    }
    pdnstring += std::string(game.resultstring) + lineterm;
}

int PdnManager::PDNparseGetnexttoken(char **start, char *token, int maxlen) {
    int len = 0;
    PDN_PARSE_STATE state;
    int tokentype = PDN_DONE;
    const char *p, *possible_end, *tok_start;

    p = (*start);
    while (isspace(*p)) ++p;

    if (!*p) return 0;

    state = PDN_IDLE;
    possible_end = 0;
    tok_start = p;
    while (*p && state != PDN_DONE) {
        switch (state) {
            case PDN_IDLE:
                if (isdigit(*p)) state = PDN_READING_FROM;
                else if (*p == '{') state = PDN_CURLY_COMMENT;
                else if (*p == '(') state = PDN_NEMESIS_COMMENT;
                else if (!isspace(*p)) state = PDN_FLUFF;
                ++p;
                break;

            case PDN_FLUFF:
                tokentype = PDN_FLUFF;
                if (isdigit(*p) || *p == '{' || *p == '(' || *p == '"') {
                    state = PDN_DONE;
                    len = p - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(p);
                } else {
                    ++p;
                }
                break;

            case PDN_CURLY_COMMENT:
                if (*p == '}') {
                    state = PDN_DONE;
                    ++p;
                    len = p - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(p);
                } else ++p;
                break;

            case PDN_NEMESIS_COMMENT:
                if (*p == ')') {
                    state = PDN_DONE;
                    ++p;
                    len = p - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(p);
                } else ++p;
                break;

            case PDN_READING_FROM:
                if (isdigit(*p)) ++p;
                else if (*p == '/' && strncmp(p - 1, "1/2-1/2", 7) == 0) {
                    state = PDN_DONE;
                    len = 7;
                    memcpy(token, p - 1, len);
                    token[len] = 0;
                    *start = const_cast<char*>(p + 6);
                } else state = PDN_WAITING_SEP;
                break;

            case PDN_WAITING_SEP:
                if (isspace(*p)) ++p;
                else if (*p == '-' || tolower(*p) == 'x') {
                    ++p;
                    state = PDN_WAITING_TO;
                } else state = PDN_FLUFF;
                break;

            case PDN_WAITING_TO:
                if (isdigit(*p)) {
                    ++p;
                    state = PDN_READING_TO;
                } else if (isspace(*p)) ++p;
                else state = PDN_FLUFF;
                break;

            case PDN_READING_TO:
                if (isdigit(*p)) ++p;
                else if (*p == '-' || tolower(*p) == 'x') {
                    possible_end = p;
                    ++p;
                    state = PDN_WAITING_OPTIONAL_TO;
                } else if (isspace(*p)) {
                    possible_end = p;
                    ++p;
                    state = PDN_WAITING_OPTIONAL_SEP;
                } else {
                    state = PDN_DONE;
                    len = p - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(p);
                }
                break;

            case PDN_WAITING_OPTIONAL_SEP:
                if (isspace(*p)) ++p;
                else if (*p == '-' || tolower(*p) == 'x') {
                    ++p;
                    state = PDN_WAITING_OPTIONAL_TO;
                } else {
                    state = PDN_DONE;
                    len = possible_end - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(possible_end);
                }
                break;

            case PDN_WAITING_OPTIONAL_TO:
                if (isdigit(*p)) {
                    possible_end = 0;
                    ++p;
                    state = PDN_READING_TO;
                } else if (isspace(*p)) ++p;
                else {
                    state = PDN_DONE;
                    len = possible_end - tok_start;
                    memcpy(token, tok_start, len);
                    token[len] = 0;
                    *start = const_cast<char*>(possible_end);
                }
                break;
            default:
                state = PDN_DONE;
                break;
        }
    }

    if (!*p) {
        if (state == PDN_WAITING_SEP || state == PDN_WAITING_TO || state == PDN_WAITING_OPTIONAL_SEP || state == PDN_WAITING_OPTIONAL_TO) {
            if (possible_end) {
                len = possible_end - tok_start;
                memcpy(token, tok_start, len);
                token[len] = 0;
                *start = const_cast<char*>(possible_end);
            } else {
                len = p - tok_start;
                memcpy(token, tok_start, len);
                token[len] = 0;
                *start = const_cast<char*>(p);
            }
        } else {
            len = p - tok_start;
            memcpy(token, tok_start, len);
            token[len] = 0;
            *start = const_cast<char*>(p);
        }
    }
    return tokentype;
}

int PdnManager::PDNparseMove(char *token, Squarelist &squares)
{
    squares.clear();
    const char *p = token;
    int num = 0;
    bool num_found = false;

    while (*p) {
        if (isdigit(*p)) {
            num = num * 10 + (*p - '0');
            num_found = true;
        } else if (num_found) {
            squares.append(num);
            num = 0;
            num_found = false;
        }
        p++;
    }

    if (num_found) {
        squares.append(num);
    }

    return squares.size() >= 2;
}
