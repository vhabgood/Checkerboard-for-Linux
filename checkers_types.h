






#pragma once







#include <stdbool.h>



#include <stdint.h>



#include <time.h>



#include "CBconsts.h"







#ifdef __cplusplus



#include <string>



#include <vector>



#endif







// Cross-platform type definitions



typedef void* HWND;



typedef void* HMENU;



typedef void* HINSTANCE;



typedef unsigned long DWORD;



typedef unsigned int UINT;



typedef uintptr_t WPARAM;



typedef intptr_t LPARAM;



typedef int (*DLGPROC)(HWND, UINT, WPARAM, LPARAM);




typedef unsigned long COLORREF;



typedef struct tagRECT {



    long left;



    long top;



    long right;



    long bottom;



} RECT;



typedef struct tagCHOOSECOLOR {



    DWORD lStructSize;



    HWND hwndOwner;



    HINSTANCE hInstance;



    COLORREF rgbResult;



    COLORREF* lpCustColors;



    DWORD Flags;



    LPARAM lCustData;



    void* lpfnHook;



    const char* lpTemplateName;



} CHOOSECOLOR, *LPCHOOSECOLOR;







#define CC_RGBINIT 0x00000001



#define CC_FULLOPEN 0x00000002



#define LPSTR char*



#define LPTHREAD_START_ROUTINE void*



#define TBADDBITMAP 0



#define INITCOMMONCONTROLSEX 0



#define STD_FILENEW 0



#define STD_FILESAVE 0



#define STD_FILEOPEN 0



#define STD_FIND 0



#define STD_UNDO 0



#define STD_REDOW 0



#define TBSTYLE_BUTTON 0



#define TBSTYLE_SEP 0



#define ICC_BAR_CLASSES 0



#define TOOLBARCLASSNAME ""



#define WS_CHILD 0



#define WS_BORDER 0



#define WS_VISIBLE 0



#define TBSTYLE_TOOLTIPS 0



#define CCS_ADJUSTABLE 0



#define TBSTYLE_FLAT 0



#define TB_BUTTONSTRUCTSIZE 0



#define TBBUTTON 0



#define TBSTATE_ENABLED 0



#define BYTE unsigned char



#define TB_ADDBITMAP 0



#define HINST_COMMCTRL 0



#define IDB_STD_SMALL_COLOR 0



#define TB_ADDBUTTONS 0



#define LPTBBUTTON 0



#define TB_AUTOSIZE 0



#define SW_SHOW 5



#define ERROR_FILE_NOT_FOUND 2



#define SE_ERR_NOASSOC 31



#define WM_USER 0x0400



#define TB_CHANGEBITMAP (WM_USER + 43)



#define CB_RESET_MOVES 0



#define CB_EXACT_TIME 0



#define CLK_TCK 1000



#define SND_FILENAME 0



#define SND_ASYNC 0















typedef struct tagTOOLTIPTEXT {







    UINT cbSize;







    UINT uFlags;







    HWND hwnd;







    uintptr_t uId;







    RECT rect;







    HINSTANCE hinst;







    LPSTR lpszText;







    LPARAM lParam;







} TOOLTIPTEXT, *LPTOOLTIPTEXT;



















#define MAX_PATH 260



#define MB_OK 0x00000000L



#define TRUE 1



#define FALSE 0



#define WM_COMMAND 0x0111



#define WM_USER 0x0400



#define TB_CHECKBUTTON (WM_USER + 2)



#define MAKELONG(a, b) ((long)(((unsigned short)(a)) | ((unsigned int)((unsigned short)(b))) << 16))



#define MF_UNCHECKED 0x00000000L



#define MF_CHECKED 0x00000008L



#define SW_SHOW 5



#define ERROR_FILE_NOT_FOUND 2



#define SE_ERR_NOASSOC 31







#define MAXMOVES 256



#define COMMENTLENGTH 1024



#define MAXNAME 256



#define MAX_PATH_FIXED 260 /* Replacement for Windows-specific MAX_PATH */



#define ENGINECOMMAND_REPLY_SIZE 2048






typedef char Board8x8[8][8];

enum PieceType {
    EMPTY = 0,
    BLACK_MAN = CB_BLACK | CB_MAN,
    WHITE_MAN = CB_WHITE | CB_MAN,
    BLACK_KING = CB_BLACK | CB_KING,
    WHITE_KING = CB_WHITE | CB_KING
};

typedef struct coor {
    int x;
    int y;
} coor;

/* A checkers move. */
typedef struct CBmove {
    coor from;
    coor to;
    coor path[12]; /* Max 12 steps in a jump sequence */
    coor del[12];  /* Max 12 captured pieces */
    int delpiece[12]; /* The type of piece deleted */
    int oldpiece;     /* The type of piece before the move */
    int newpiece;     /* The type of piece after the move */
    int jumps;        /* Number of jumps in the move */
} CBmove;

extern Board8x8 cbboard8; // Global board state

typedef struct pos {
    unsigned int bm; // Black men
    unsigned int bk; // Black kings
    unsigned int wm; // White men
    unsigned int wk; // White kings
} pos;

typedef enum PDN_RESULT {
    PDN_RESULT_UNKNOWN,
    PDN_RESULT_WHITE_WINS,
    PDN_RESULT_BLACK_WINS,
    PDN_RESULT_DRAW
} PDN_RESULT;

typedef enum EM_START_POSITIONS {
    START_POS_3MOVE, START_POS_FROM_FILE
} EM_START_POSITIONS;

struct CBoptions {
	/* holds all options of CB.
	 * the point is that it is much easier to store one struct in the registry
	 * than to save every value separately. */
	unsigned int crc;					/* The crc is calculated on the whole struct using the sizeof(CBoptions) in the crc field. */
	char userdirectory[256];
	char matchdirectory[256];	
	char EGTBdirectory[256];
	char primaryenginestring[64];
	char secondaryenginestring[64];
	char start_pos_filename[MAX_PATH_FIXED]; /* Using fixed size */
	unsigned int colors[5]; /* Replaced COLORREF */
	int userbook;
	int sound;
	int invert;
	int mirror;
	int numbers;
	int highlight;
	int priority;
	int exact_time; /* bool */
	int use_incremental_time; /* bool */
	int early_game_adjudication; /* bool */
	int handicap_enable;		/* enable handicap multiplier for engine 2's time in engine matches. */
	double handicap_mult;		/* eninge 2's time multiplier when handicap match is enabled. */
	EM_START_POSITIONS em_start_positions;
	int level;
	double initial_time;		/* incremental time control settings. */
	double time_increment;
	int match_repeat_count;
	int op_crossboard;
	int op_mailplay;
	int op_barred;
	int window_x;
	int window_y;
	int window_width;
	int window_height;
	int addoffset;
	int language;
	int piecesetindex;
};

struct BALLOT_INFO {
	int color;
	Board8x8 board;
#ifdef __cplusplus
	std::string event;
#endif
};

struct RESULT_COUNTS {
	int black_wins;
	int white_wins;
	int draws;
	int unknowns;
};

/* A game move with associated move text, comments, and analysis text. */
struct gamebody_entry {
	CBmove move;						/* move */
	char PDN[64];						/* PDN of move, eg. 8-11 or 8x15 */
	char comment[COMMENTLENGTH];		/* user comment */
	char analysis[COMMENTLENGTH];		/* engine analysis comment - separate from above so they can coexist */
};

struct PDNgame {
	/* structure for a PDN game
	 * standard 7-tag-roster */
	char event[MAXNAME];
	char site[MAXNAME];
	char date[MAXNAME];
	char round[MAXNAME];
	char black[MAXNAME];
	char white[MAXNAME];
	char resultstring[MAXNAME];
	char FEN[MAXNAME];
	PDN_RESULT result;
	int gametype;
	int movesindex;						/* Current index in moves[]. */
#ifdef __cplusplus
	std::vector<gamebody_entry> moves;		/* Moves and comments in the game body. */
#endif
};

/* This type is used to display game previews in the game select dialog. */
struct gamepreview {
	int game_index;		/* index of game into the current pdn database. */
	char black[64];
	char white[64];
	char result[10];
	char event[128];
	char date[32];
	char PDN[256];
};

struct userbookentry {
	pos position;
	CBmove move;
};

/* A mapping between different time constants. */
struct timemap {
	int level;		/* cboptions setting. */
	int token;		/* resource token of Windows control. */
	double time;	/* search time. */
};

struct time_ctrl_t {
	int clock_paused; /* bool */
	clock_t starttime;
	double black_time_remaining;
	double white_time_remaining;
	double cumulative_time_used[3];		/* Indexed by engine number, 1 or 2. */
	int searchcount;
};

struct emstats_t {
	int wins;
	int draws;
	int losses;
	int unknowns;
	int blackwins;
	int blacklosses;
	int games;
	int opening_index;	/* index into 3-move table, 1 less than the ACF ballot number. */

#ifdef __cplusplus
	inline bool is_odd(int n) {return((n & 1) == 1);}

	/*
	 * Given an engine match game number (1..N), return true if engine 1 plays black for that game.
	 * Engine 1 plays black in odd numbered games.
	 */
	inline bool engine1_plays_black(int gamenum) {return((is_odd(gamenum)));}

	/*
	 * Given an engine match game number (1..N), and a side-to-move color (CB_BLACK or CB_WHITE), 
	 * return the engine number (1 or 2) that should select the next move.
	 */
	inline int get_enginenum(int gamenum, int color) {
		if (is_odd(gamenum + color))
			return(1);
		else
			return(2);
	}
#endif
};

#ifdef __cplusplus
class Squarelist {
public:
	Squarelist(void) {clear();}
	void clear(void) {m_size = 0;}
	int first(void) {return(squares[0]);}
	int last(void) {return(squares[m_size - 1]);}
	int size(void) {return(m_size);}
	int read(int index) {return(squares[index]);}
	void append(int square) {
		if (m_size < sizeof(squares) / sizeof(squares[0])) {
			squares[m_size] = square;
			++m_size;
		}
	}
	int frequency(int square) {
		int count = 0;
		for (int i = 0; i < m_size; ++i)
			if (squares[i] == square)
				++count;
		return(count);
	}
	void reverse_color(void) {
		for (int i = 0; i < m_size; ++i)
			squares[i] = 33 - squares[i];
	}
	void reverse_rows(void) {
		for (int i = 0; i < m_size; ++i) {
			int sq0 = squares[i] - 1;
			int row = sq0 / 4;
			squares[i] = 1 + (4 * row) + (3 - (sq0 & 3));
		}
	}

private:
	char m_size;
	char squares[15];
};
#endif

// Function prototypes (these are not definitions, so they can remain)
extern "C" int getmovelist(int color, CBmove movelist[MAXMOVES], Board8x8 board, int *isjump);
void board8toFEN(const Board8x8 board, char *fenstr, int color, int gametype);
int FENtoboard8(Board8x8 board, const char *buf, int *poscolor, int gametype);

unsigned int crc_calc(char *buf, int len);
int coorstonumber(int x, int y, int gametype);
void numbertocoors(int n, int *x, int *y, int gametype);

typedef enum {
    RTF_NO_ERROR,
    RTF_FILE_ERROR,
    RTF_MALLOC_ERROR
} READ_TEXT_FILE_ERROR_TYPE;

char *read_text_file(char *filename, READ_TEXT_FILE_ERROR_TYPE *etype);
int PDNparseGetnextgame(char **start, char *game, int maxlen);