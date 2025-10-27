#include "checkers_types.h"
#include <stdlib.h> // For free
#include <ctype.h> // For isdigit
#include <string.h> // For strncpy

#define NEMESIS // enables detection of comments in round braces ( )

int PDNparseGetnumberofgames(char *filename)
{
	// returns the number of games in a PDN file
	char *buffer;
	char game[1024]; // Simplified from std::string
	char *p;
	int ngames;
	READ_TEXT_FILE_ERROR_TYPE etype;

	buffer = read_text_file(filename, &etype);
	if (buffer == NULL)
		return -1;

	p = buffer;
	ngames = 0;
	while (PDNparseGetnextgame(&p, game, sizeof(game)))
		++ngames;

	free(buffer);
	return(ngames);
}

inline bool is_pdnquote(uint8_t c)
{
	if (c == '"')
		return(true);
	if (c & 0x80) {
		if (c == 0x20 || c == 0x1d) // UTF8_LEFT_DBLQUOTE and UTF8_RIGHT_DBLQUOTE placeholders
			return(true);
	}

	return(false);
}


int PDNparseGetnextgame(char **start, char *game, int maxlen)
{

	/* searches a game in buffer, starting at **start. a 
		game is defined as everything between **start and
		the first occurrence of one of the four game 
		terminators (1-0 0-1 1/2-1/2 *). since the game 
		terminators also appear in headers [HEADER], 
		getnextgame skips headers. it also skips comments {COMMENT}
		if the function succeeds, **start points to the next character
		after the game returned in *game.
		*/

	// new 15. 8. 2002: try to recognize the next set of headers as terminators.
	// new 6.9. 2002: the way it was up to now, pdnparsenextgame would just
	// run infinitely on the last game!
	char *p;
	char *p_org;
	int headersdone = 0;

	game[0] = 0;
	if ((*start) == 0)
		return 0;

	p = (*start);
	p_org = p;
	while (*p != 0) {

		/* skip headers */
		if (*p == '[' && !headersdone) {
			p++;
			while (*p != ']' && *p != 0) {

				/* Ignore anything inside quotes (e.g. ']') within headers. */
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

		/* skip comments */
		if (*p == '{') {
			p++;
			while (*p != '}' && *p != 0) {
				p++;
			}
		}

#ifdef NEMESIS
		// skip comments, nemesis style
		if (*p == '(') {
			p++;
			while (*p != ')' && *p != 0) {
				p++;
			}
		}
#endif
		if (*p == 0)
			break;

		// try to detect whether we are through with the headers
		if (isdigit((uint8_t) *p))
			headersdone = 1;

		/* check for game terminators*/
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

int PDNparseGetnextheader(const char **start, char *header, int maxlen)
{
	/* getnextheader */

	/* searches a header in buffer, starting at **start. a header
	is defined as the next complete string which is enclosed 
	between square brackets [ HEADER ]. 
	if no header is found, getnextheader
	returns 0. 
	if a header is found, getnextheader sets **start
	to the next character after the header.
	the header is returned in *header */
	const char *p, *q;
	int i, quotecount;

	if (*start == 0)
		return 0;
	p = *start;
	while (*p != '[' && *p != 0)
		p++;

	/* if no opening brace is found... */
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

	// terminate header with a 0
	header[i < maxlen - 1 ? i : maxlen - 1] = 0;

	/* if no closing brace is found */
	if (*q == 0)
		return 0;

	/* ok, we have found a header it is written to *header, now 
		we set the start pointer which tells where to continue searching*/
	*start = q + 1;
	return 1;
}

int PDNparseGetnexttag(const char **start, char *tag, int maxlen)
{
	/* getnexttag */

	/* searches a tag in buffer, starting at **start. a tag
	is defined as the next complete string which is enclosed 
	between "s - "TAG". 
	if no tag is found, getnexttag
	returns 0. 
	if a tag is found, getnexttag sets **start
	to the next character after the header.
	the tag is returned in *tag */
	const char *p, *q;
	int i;

	if ((*start) == 0)
		return 0;
	p = (*start);
	while (!is_pdnquote(*p) && *p != 0)
		p++;

	/* if no opening " is found... */
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

	/* if no closing " is found */
	if (*q == 0)
		return 0;

	/* ok, we have found a tag, it is written to *tag, now 
		we set the start pointer */
	(*start) = q + 1;
	return 1;
}