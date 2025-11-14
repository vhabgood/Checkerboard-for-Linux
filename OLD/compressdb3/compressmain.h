int getdatabasesize(bm, bk, wm, wk, maxbm, maxwm);
int initbuilddb(void);
int builddb(int n);
int buildsubdb(int bm,int bk,int wm,int wk);
int buildsubdbslice(int bm, int bk, int wm, int wk, int bmrank, int wmrank);
int setdatabasevalue(int *database, int index, int value);
int memorypanic(void);
int unloadall(void);
int runlengthmaximize(int *currentdb, int dbsize, int dominantvalue);
int runlengthoptimize(int *currentdb, int dbsize, int dominantvalue);
int compressdb(int *currentdb, int dbsize, char dbname[256]);
int finddominantvalue(int *database, int dbsize, int *wins, int *draws, int *losses);
int choose(int n, int k);
int getdatabasevalue(int *database, int index);
int initbicoef(void);
void indextoposition(int32 index, position *p, int bm, int bk, int wm, int wk, int bmrank, int wmrank, int *color);
void fastindextoposition(int32 index, position *p, int bm, int bk, int wm, int wk, int bmrank, int wmrank, int *color);


// compile switches

#define SPLITSIZE 8 // dbs with SPLITSIZE stones or more will be divided into smaller parts. recommend 8
//#define BLACKONLY // only put in dbs with black to move?


// macros for fast testcapture:

/* bitboard masks for moves in various directions */
/* here "1" means the squares in the columns 1357 and "2" in 2468.*/
#define RF1  0x0F0F0F0F
#define RF2  0x00707070
#define LF1  0x0E0E0E0E
#define LF2  0x00F0F0F0
#define RB1  0x0F0F0F00
#define RB2  0x70707070
#define LB1  0x0E0E0E00
#define LB2  0xF0F0F0F0
/* bitboard masks for jumps in various directions */
#define RFJ1  0x00070707
#define RFJ2  0x00707070
#define LFJ1  0x000E0E0E
#define LFJ2  0x00E0E0E0
#define RBJ1  0x07070700
#define RBJ2  0x70707000
#define LBJ1  0x0E0E0E00
#define LBJ2  0xE0E0E000


#define forward(x) ( (((x)&LF1)<<3) | (((x)&LF2)<<4) | (((x)&RF1)<<4) | (((x)&RF2)<<5) )

#define leftforward(x) ( (((x)&LF1)<<3) | (((x)&LF2)<<4)  )
	// returns the squares left forward of x

#define rightforward(x) ( (((x)&RF1)<<4) | (((x)&RF2)<<5) )
	
#define backward(x) ( (((x)&LB1)>>5) | (((x)&LB2)>>4) | (((x)&RB1)>>4) | (((x)&RB2)>>3) )

#define leftbackward(x) ( (((x)&LB1)>>5) | (((x)&LB2)>>4)  )

#define rightbackward(x) ( (((x)&RB1)>>4) | (((x)&RB2)>>3) )
