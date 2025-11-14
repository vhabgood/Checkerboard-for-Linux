#define COMMENTLENGTH 1024

typedef struct
	{
	int win;
	int loss;
	int draw;
	} RESULT;



struct coor 				/* coordinate structure for board coordinates */
	{
	// was ints
	int x;
	int y;
	};



struct CBmove					/* all the information you need about a move */
	{
	// was all ints. made char because i save CBmoves in user book - this saves
	// me a factor 4 - still, they are much too larg of course...

	int jumps;				/* how many jumps are there in this move? */
	int newpiece;			/* what type of piece appears on to */
	int oldpiece;			/* what disappears on from */
	struct coor from,to; /* coordinates of the piece - in 8x8 notation!*/
	struct coor path[12];/* intermediate path coordinates of the moving piece */
	struct coor del[12]; /* squares whose pieces are deleted after the move */
	int delpiece[12]; 	/* what is on these squares */
	};



struct listentry			/* doubly linked list which holds the game */
	{
	struct CBmove move;					// move
	struct listentry *next;				// pointer to next in list
	struct listentry *alternatenext;	// pointer to alternate next -> variations
	struct listentry *last;				// pointer to previous in list
	char PDN[64];						// PDN of move, e.g. 8-11 or 8x15
	char comment[COMMENTLENGTH];		// user comment
	char analysis[COMMENTLENGTH];		// engine analysis comment - separate from above so they can coexist
	};

struct PDNgame
	{
	// structure for a PDN game
	// standard 7-tag-roster
	char event[256];
	char site[256];
	char date[256];
	char round[256];
	char black[256];
	char white[256];
	char resultstring[256];
	/* support 2 more tags for setup */
	char setup[256];
	char FEN[256];
	/* internal conversion to integers */
	int result;
	int gametype;
	/* head of the linked list */
	struct listentry *head;
	};

/*
struct package
	{
	char name[256];			//package name
	char version[64];		// package version
	char URL[256]; 			//package location
	char description[256];
	char size[256];			// size in KB
	char status[256]; 		// new package, new version, or up to date*
	char publisher[256];
	char filename[256];		//filename to write the file to. if this is ZIP, winzip is invoked
	}; */

struct gamedatabase{   // MAXGAMES of these get allocated! - nearly 20MB for 40'000 MAXGAMES
	char black[64];
	char white[64];
	char result[10];
	char event[128];
	char date[32];
	char PDN[256];
	};

struct pos
	{
	unsigned int bm;
	unsigned int bk;
	unsigned int wm;
	unsigned int wk;
	};

struct userbookentry {
	// position
	struct pos position;
	// move
	struct CBmove move;
	};

