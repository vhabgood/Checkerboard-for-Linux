// 
//
// checkers.h - standard include file for checkers projects, with all structures and constants. 
//
//

#undef MOCKENCODE

// use already present database or start from scratch?

//#define USEOLD
#define REVERSE
#define SYMMETRIC
//#define TESTSIZE
//#define PROFILE
#undef NOIO

/////////////////
// constants

#define MAXMOVES 28

#define BLACK 0
#define WHITE 1

// number of pieces up to which we build databases.
#define MAXPIECES 6 
// maximal number of a single piece
#define MAXPIECE ((MAXPIECES+1)/2)

// database scores 
#define UNKNOWN 0
#define WIN 1
#define LOSS 2
#define DRAW 3

/////////////////
// structures

typedef unsigned int int32;


typedef struct pos
	{
	int32 bm;
	int32 bk;
	int32 wm;
	int32 wk;
	} position;

typedef struct sub
	{
	int bm;
	int bk;
	int wm;
	int wk;
	int maxbm;
	int maxwm;
	int databasesize;
	int *database;
	} subdb;


typedef position move;

#define pad16(x) (((x)/16+1)*16)

#define hiword(x) (((x)&0xFFFF0000)>>16)
#define loword(x) ((x)&0xFFFF)