//
//	compressmain.c
//	main file of my database compressor
//
//

//#define DEBUG

#include <windows.h>
#include "checkers.h"
#include "compressmain.h"
#include "bool.h"
#include "min_movegen.h"


#include <time.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>


double start,substart,progstart;
int *currentdb_b, *currentdb_w; // pointer to the databases which are currently being computed.
FILE *logfp;

FILE *fp_idx; // currently open idx file
FILE *fp_cpr; // currently open db file

unsigned char block[1024]; // current compressed db block;
int blockindex; // current index into block array.
int blocknumber; // current block number
int maxdbsize = 0;
int totaldbsize = 0;
int symmetricsize = 0;

// subdatabase records: hold info on a database slice, such as 2222.66
int conversions=0;
int iterations=0;

double totalbytes,totalbytescompressed,totalblacksize;
int totalcompbytes,totalliteralbytes;

// from lookup.c
//extern subdb subdatabase[MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][MAXPIECE+1][7][7][2];
int bicoef[33][33];
int transposedbicoef[33][33];

//for fast lookup:
int gbmrange,gwmrange,gbkrange,gwkrange;
int gmultiplier1,gmultiplier2;
int gbm,gbk,gwm,gwk,gbmrank,gwmrank;


int meminuse = 0;


#define SKIPS 58        // the number of skips.
#define MAXSKIP 10000   // the largest skip value
#define MAXIDX 100000   // this means that no compressed file can be larger than 100'000 1K blocks - 
						// so no larger than 100MB. the largest single sized file i have would be
						// about 340 MB uncompressed - and compression ratios are typically 1:10
						// so this should be ok.



// skip numbers range from 5...MAXSKIP defined above, there are SKIPS of them.
// original 
int skip[SKIPS]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,40,44,48,52,56,60,70,80,90,100,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,MAXSKIP};

// testing different:
// ~0.2% better on 6-piece db.
//int skip[SKIPS]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,32,36,40,44,48,52,56,60,70,80,90,100,110,120,130,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,MAXSKIP};
int rle[MAXSKIP+1];
int skipindex[MAXSKIP+1]; // holds the index of the next smaller or equal skip.

main()
	{
	int i,j,n;
	FILE *fp;

	printf("\nDatabase Compressor by Martin Fierz");

	// delete old log files
	fp = fopen("compress.txt","w");
	fclose(fp);
	fp = fopen("compresslog.txt","w");
	fclose(fp);
	fp = fopen("winlog.txt","w");
	fclose(fp);
	meminuse = 0;
	
	// initialize bool.c module
	initbool();

	// initialize lookup data
	initbicoef();

	// initialize skipindex
	for(i=0;i<=MAXSKIP;i++) // for i=0 to MAXSKIP
		{
		for(j=SKIPS-1;j>=0;j--)
			{
			if(skip[j]<=i)
				{
				skipindex[i]=j;
				break;
				}
			}
		}
	progstart = clock();

	//for(n=2;n<=MAXPIECES;n++)
	for(n=2;n<=6;n++)
		{
		j=builddb(n);
		printf("\ndb %i done",n);
		printf("\ntime: %.1f seconds\n",(clock()-progstart)/CLK_TCK);
		}

	printf("\nmaximal db size is %ii\n\n", maxdbsize);
	printf("\ntotal db size is %i\n\n", totaldbsize);
	printf("\ntotal black size is %i",totalblacksize);
	printf("\ntotal symmetric db size is %i\n\n", symmetricsize);
	
	exit(0);
	}


int builddb(int n)
	{
	// build all databases with i stones
	// do this by building all subdatabases with n stones organized into their bm bk wm wk category
	// first, it has to do the ones with the kings, and only later the ones with the stones.

	// code as is should work - first does strange databases, but that's ok i think. 

	int bm=0,bk=0,wm=0,wk=0;
	int div1, div2, div3;
	FILE *fp;
	char filename[256];
	int i;
	
	// set rle array to 0 - this array holds the db run length histogram
	for(i=0;i<=MAXSKIP;i++)
		rle[i]=0;

	//for logfile: keep a record of the sizes of the compressed db	
	totalbytes=0;
	totalbytescompressed=0;
	totalblacksize = 0;
	totalcompbytes = 0;
	totalliteralbytes = 0;
	
	if(n<SPLITSIZE)
		{
		// we build one database for every number of stones.
		// open files for this
		sprintf(filename,"db%i.idx",n);
#ifdef BLACKONLY
		sprintf(filename,"bb%i.idx",n);
#endif
		fp_idx = fopen(filename,"w");
		sprintf(filename,"db%i.cpr",n);
#ifdef BLACKONLY
		sprintf(filename,"bb%i.cpr",n);
#endif
		fp_cpr = fopen(filename,"wb");
		// blockindex is the index of the byte counter in the current block,
		// blocknumber is the number of the current block
		blockindex=0;
		blocknumber = 0;
		if(fp_cpr==0 || fp_idx==0)
			{
			printf("\ncould not open files: %s",filename);
			getch();
			exit(0);
			}
		}

	for(div1=0;div1<=n;div1++)
		{
		for(div2=div1;div2<=n;div2++)
			{
			for(div3=div2;div3<=n;div3++)
				{
				bm=div1;
				bk=div2-div1;
				wm=div3-div2;
				wk=n-div3;
				buildsubdb(bm,bk,wm,wk);
				}
			}
		}

	if(n<SPLITSIZE)
		{
		// now, there is probably an unfinished block in memory. the
		// index file has been done, but the block still has to go into
		// the compressed file:
		if(blockindex>1024 || blockindex <0)
			{
			printf("\nblockindex is %i",blockindex);
			getch();
			exit(0);
			}
		fwrite(block,blockindex,1,fp_cpr);
		
		fclose(fp_idx);
		fclose(fp_cpr);
		}

	fp=fopen("compresslog.txt","a");
	if(fp!=0)
		{
		fprintf(fp, "\n%i-piece database: t:%.1fs bytes: %.0f  compressed: %.0f  black: %.0f  ratio %.3f  compressed %i literal %i",n,(clock()-progstart)/CLK_TCK,totalbytes, totalbytescompressed, totalblacksize,totalbytescompressed/totalbytes, totalcompbytes, totalliteralbytes);
		fclose(fp);
		}

	sprintf(filename, "dbhist%i.txt",n);
	fp=fopen(filename,"w");
	if(fp!=0)
			{
		for(i=0;i<=MAXSKIP;i++)
			{
			fprintf(fp,"%i\t%i\n",i,rle[i]);
			}
		fclose(fp);
		}
	return 0;
	}

int buildsubdb(int bm,int bk,int wm,int wk)
	{
	int bmrank, wmrank; // maximal rank of the white and black men
	char filename[256];
	int n = bm+bk+wm+wk;

	// is this a valid database?
	// one side has no pieces:
	if((bm+bk==0) || (wm+wk==0)) return 0;

#ifdef REVERSE
	// white has more pieces than black
	if(bm+bk<wm+wk)
		return 0;

	// absurd databases: if maxpieces = 8, we don't do 5-3, 6-2, 7-1 or 6-1 or 5-1 etc.
	// this depends on MAXPIECE of course!
	if(bm+bk>MAXPIECE)
		return 0;

	// more bk than wk in case of equal number of pieces
	if(wk+wm==bk+bm)
		{
		if(wk>bk)
			return 0;
		}
#endif

	if(n>=SPLITSIZE)
		{
		// we build one database for every database slice like 4men-4men 
		// or 3men1king-4men etc. for the 8-piece db a single file would
		// be 4GB in size - much too much!
		// open database files for writing: fp_idx and fp_cpr are file pointers
		// to these two files
		sprintf(filename,"db%i_%i%i%i%i.idx",n,bm,bk,wm,wk);
#ifdef BLACKONLY
		sprintf(filename,"bb%i_%i%i%i%i.idx",n,bm,bk,wm,wk);
#endif
		fp_idx = fopen(filename,"w");
		sprintf(filename,"db%i_%i%i%i%i.cpr",n,bm,bk,wm,wk);
#ifdef BLACKONLY
		sprintf(filename,"bb%i_%i%i%i%i.cpr",n,bm,bk,wm,wk);
#endif
		fp_cpr = fopen(filename,"wb");
		
		// reset block index and block number of the database
		blockindex=0;
		blocknumber = 0;
		if(fp_cpr==0 || fp_idx==0)
			{
			printf("\ncould not open files");
			getch();
			exit(0);
			}
		}



	if(bm==0 && wm==0)
		buildsubdbslice(bm,bk,wm,wk,0,0);
	
	if(bm!=0 && wm==0)
		{
		for(bmrank=6;bmrank>=((bm-1)/4);bmrank--)
			buildsubdbslice(bm,bk,wm,wk,bmrank,0);
		}
	
	if(wm!=0 && bm==0)
		{
		for(wmrank=6;wmrank>=((wm-1)/4);wmrank--)
			buildsubdbslice(bm,bk,wm,wk,0,wmrank);
		}

	if(bm!=0 && wm!=0)
		{
		for(bmrank=6;bmrank>=((bm-1)/4);bmrank--)
			{
			for(wmrank=6;wmrank>=((wm-1)/4);wmrank--)
				buildsubdbslice(bm,bk,wm,wk,bmrank,wmrank);
			}
		}

	if(n>=SPLITSIZE)
		{
		// now, there is probably an unfinished block in memory. the
		// index file has been done, but the block still has to go into
		// it:
		if(blockindex>1024 || blockindex <0)
			{
			printf("\nblockindex is %i",blockindex);
			getch();
			exit(0);
			}
		fwrite(block,blockindex,1,fp_cpr);
		
		fclose(fp_idx);
		fclose(fp_cpr);
		}


	return 1;
	}

int buildsubdbslice(int bm, int bk, int wm, int wk, int bmrank, int wmrank)
	{
	///////////////////////////////////////////////////////////////////////////////////

	int symmetric = 0;
	int compressedsize=0;
	int index,i;
	int color;
	int32 free;
	int32 memsize; // will not work for databases which require more than 2-4GB ram.
	int dominantvalue;
	int done=0, pass=0;
	int32 wins=0, draws=0, losses=0, unknowns=0;
//	int resultb[4],resultw[4];
	int loadblack = 0, loadwhite = 0;
	int singlevalue; // is whole db single value?
	int dbsize;
	int nondominantvalue;
	position p;
	float ratio;
	char dbname[256];
	char c[5]=" +-=";
	FILE *fp;

#ifdef REVERSE
	/////////////////////////////////////////////////////////////////////////
	// check for a valid db - is it's reverse already computed?:
	//
	if(wm==bm && wk==bk)
		{
		if(wmrank>bmrank)
			return 0;
		}
#endif

	////////////////////////////////////////////////////////////////////////
	// get the size of our database and do some housekeeping

	dbsize = getdatabasesize(bm, bk, wm, wk, bmrank, wmrank);
	totalbytes+=dbsize/4;
	

	// check if the database is symmetric
	if(bm==wm && bk==wk && bmrank==wmrank)
		symmetric = 1;
	
	// load black database
	sprintf(dbname,"raw\\db%i%i%i%i-%i%ib.dat", bm,bk,wm,wk,bmrank,wmrank);
	printf("\nloading %s... ",dbname);
	fp = fopen(dbname,"rb");
	if(fp!=NULL)
		{
		memsize = pad16(dbsize/4);
		currentdb_b = VirtualAlloc(0, memsize, MEM_COMMIT, PAGE_READWRITE);
		if(currentdb_b == NULL)
			{
			printf("\nerror on mem alloc for black db!");
			getch();
			exit(0);
			}
		fread(currentdb_b, 1, memsize, fp);
		fclose(fp);
		}
	printf("done");
	
// set globals to speed things up
			// extract the indexes for the piece types from the total index:
	// get multiplier for wkindex:
	// possibly not using the "if" is faster - because if can lead to
	// a branch misprediction
	gbmrange=1;
	gbkrange=1;
	gwmrange=1;
	gwkrange=1;
	if(bm)
		gbmrange = bicoef[4*(bmrank+1)][bm] - bicoef[4*bmrank][bm];
	if(wm)
		gwmrange = bicoef[4*(wmrank+1)][wm] - bicoef[4*wmrank][wm];
	if(bk)
		gbkrange = bicoef[32-bm-wm][bk];
	if(wk)
		gwkrange = bicoef[32-bm-wm-bk][wk];
	gmultiplier1 = gbmrange*gwmrange*gbkrange;
	gmultiplier2 = gbmrange*gwmrange;
	gbm=bm;
	gbk=bk;
	gwm=wm;
	gwk=wk;
	gbmrank=bmrank;
	gwmrank=wmrank;
	
		
	// if it's symmetric, also load white db. reason for this is that the capture test
	// is expensive and we can do it simultaneously on the two.




	if(!symmetric)
		{
		// load white database
		sprintf(dbname,"raw\\db%i%i%i%i-%i%iw.dat", bm,bk,wm,wk,bmrank,wmrank);
		printf("\nloading %s... ",dbname);
		fp = fopen(dbname,"rb");
		if(fp!=NULL)
			{
			currentdb_w = VirtualAlloc(0, memsize, MEM_COMMIT, PAGE_READWRITE);
			if(currentdb_w == NULL)
				{
				printf("\nerror on mem alloc for white db!");
				getch();
				exit(0);
				}
			fread(currentdb_w, 1, memsize, fp);
			fclose(fp);
			}
		printf("done");
		}

	sprintf(dbname,"db%i%i%i%i-%i%i", bm,bk,wm,wk,bmrank,wmrank);
	printf("\ncompressing %s black",dbname);
	
	///////////////////////////////////////////////
	// step 1: set capture positions to unknown
	printf("\n  floating captures... ");


	if(symmetric)
		{
		for(index=0;index<dbsize;index++)
			{
			fastindextoposition(index, &p, bm, bk, wm, wk, bmrank, wmrank, &color);
			if(p.bm & p.wm)
				{
				setdatabasevalue(currentdb_b, index, UNKNOWN);
				continue;
				}
//			resultb[getdatabasevalue(currentdb_b,index)]++;
			// if it's a capture, float the value to unknown
			// two-step test: if there are black and white pieces 
			// adjacent to each other we do the testcapture. else
			// not. this should be a bit faster than testcapture alone.
			if( (forward(p.bm|p.bk) | backward(p.bm|p.bk)) & (p.wm|p.wk))
				{
				if(testcapture(&p, BLACK) || testcapture(&p, WHITE))
					{
					setdatabasevalue(currentdb_b, index, UNKNOWN);
					}
				}
			// float impossible positions. an impossible position is one
			// where both sides have no unmove. 
			/*free = ~(p.bm|p.bk|p.wm|p.bk);
			if( ((backward(p.bm|p.bk)|forward(p.bk)) & free) == 0)
				{
				if( ((forward(p.wm|p.wk)|backward(p.wk)) & free) == 0)
					{// this position is impossible!
					printf("!");
					setdatabasevalue(currentdb_b, index, UNKNOWN);
					}
				}*/
			}
		}
	else
		{
		for(index=0;index<dbsize;index++)
			{
			fastindextoposition(index, &p, bm, bk, wm, wk, bmrank, wmrank, &color);
			if(p.bm & p.wm)
				{
				setdatabasevalue(currentdb_b, index, UNKNOWN);
				setdatabasevalue(currentdb_w, index, UNKNOWN);
				continue;
				}
//			resultb[getdatabasevalue(currentdb_b,index)]++;
//			resultw[getdatabasevalue(currentdb_w,index)]++;
			if( (forward(p.bm|p.bk) | backward(p.bm|p.bk)) & (p.wm|p.wk))
				{
				if(testcapture(&p, BLACK) || testcapture(&p, WHITE))
					{
					setdatabasevalue(currentdb_b, index, UNKNOWN);
					setdatabasevalue(currentdb_w, index, UNKNOWN);
					}
				}
			// float impossible positions. an impossible position is one
			// where both sides have no unmove. 
			/*free = ~(p.bm|p.bk|p.wm|p.bk);
			if( ( (backward(p.bm) | backward(p.bk) | forward(p.bk)) & free) == 0) 
				{
				if( ( (forward(p.wm) | forward(p.wk) | backward(p.wk)) & free) == 0) 
					// this position is impossible!
					{
					printf("!");
					setdatabasevalue(currentdb_b, index, UNKNOWN);
					setdatabasevalue(currentdb_w, index, UNKNOWN);
					}
				}
			*/
			}
		}


	printf("\n  finding dominant value in db");
	dominantvalue = finddominantvalue(currentdb_b, dbsize, &wins, &draws, &losses);

	// check for single-valued database:
	singlevalue=UNKNOWN;
	switch(dominantvalue)
		{
		case WIN:
			if(draws+losses==0)
				singlevalue=WIN;
			nondominantvalue = DRAW;
			break;
		case DRAW:
			if(wins+losses==0)
				singlevalue=DRAW;
			nondominantvalue = LOSS;
			break;
		case LOSS:
			if(wins+draws==0)
				singlevalue=LOSS;
			nondominantvalue = WIN;
			break;
		}

	if(singlevalue==UNKNOWN)
		{
		// set unknown values in a complicated way to improve run lengths!
		//floatnondominant(currentdb_b,dbsize,dominantvalue,nondominantvalue);
		printf("\n  maximizing run length");
		/*for(i=0;i<dbsize;i++)
			{
			if(getdatabasevalue(currentdb_b, i) == UNKNOWN)
				unknowns++;
			}
		printf("\nunknowns before %i", unknowns);
		unknowns=0;*/
		//runlengthoptimize(currentdb_b, dbsize, dominantvalue);
		runlengthmaximize(currentdb_b, dbsize, dominantvalue);
		/*for(i=0;i<dbsize;i++)
			{
			if(getdatabasevalue(currentdb_b, i) == UNKNOWN)
				{
				unknowns++;
				printf("\nunknown at index %i of %i", i, dbsize);
				}
			}
		printf("\nunknowns before %i", unknowns);*/
		fprintf(fp_idx,"BASE%i,%i,%i,%i,%i,%i,b:%i/%i\n",bm,bk,wm,wk,bmrank,wmrank,blocknumber,blockindex);
		sprintf(dbname,"db%i%i%i%i-%i%ib", bm,bk,wm,wk,bmrank,wmrank);
		printf("\n  compressing... ");
		compressedsize = compressdb(currentdb_b, dbsize, dbname);
		printf("\ndone");
		}
	else
		{
		fprintf(fp_idx,"BASE%i,%i,%i,%i,%i,%i,b:%c\n",bm,bk,wm,wk,bmrank,wmrank,c[singlevalue]);
		}

	totalbytescompressed+=compressedsize;
	totalblacksize += compressedsize;
	ratio = (float)compressedsize/(dbsize/4);
	
	if(compressedsize!=0)
		{
		printf("\nsize: uncompressed %i compressed %i  ratio %.3f overall %.3f",(int) dbsize/4, compressedsize,ratio,totalbytescompressed/totalbytes);
		fp = fopen("compress.txt","a");
		fprintf(fp,"\n%s:",dbname);
		fprintf(fp,"size: uncmpr %i cmpr %i  ratio %.3f overall %.3f",(int) dbsize/4, compressedsize,ratio,totalbytescompressed/totalbytes);
		fclose(fp);
		}
	if(symmetric)
		{
		printf(" free");
		VirtualFree(currentdb_b , 0, MEM_RELEASE);
		return 1;
		}

#ifdef BLACKONLY
	// produce only black database?
	VirtualFree(currentdb_b , 0, MEM_RELEASE);
	if(!symmetric)
		VirtualFree(currentdb_w , 0, MEM_RELEASE);
	return 1;
#endif

	//-------------------------------------------------------------------------------------------------
	// 
	// compress the white database
	//
	//

	totalbytes+=dbsize/4;

	printf("\ncompressing %s white",dbname);
	compressedsize=0;

	dominantvalue = finddominantvalue(currentdb_w, dbsize,&wins,&draws,&losses);

	// check for single-valued database:
	// check for single-valued database:
	singlevalue=UNKNOWN;
	switch(dominantvalue)
		{
		case WIN:
			if(draws+losses==0)
				singlevalue=WIN;
			nondominantvalue = DRAW;
			break;
		case DRAW:
			if(wins+losses==0)
				singlevalue=DRAW;
			nondominantvalue = LOSS;
			break;
		case LOSS:
			if(wins+draws==0)
				singlevalue=LOSS;
			nondominantvalue = WIN;
			break;
		}


	if(singlevalue==UNKNOWN)
		{
		// set unknown values in a complicated way to improve run lengths!
		//floatnondominant(currentdb_w,dbsize,dominantvalue,nondominantvalue);
		//runlengthoptimize(currentdb_w, dbsize, dominantvalue);
		runlengthmaximize(currentdb_w, dbsize, dominantvalue);
		sprintf(dbname,"db%i%i%i%i-%i%iw", bm,bk,wm,wk,bmrank,wmrank);
		fprintf(fp_idx,"BASE%i,%i,%i,%i,%i,%i,w:%i/%i\n",bm,bk,wm,wk,bmrank,wmrank,blocknumber,blockindex);
		compressedsize = compressdb(currentdb_w, dbsize, dbname);
		}
	else
		{
		fprintf(fp_idx,"BASE%i,%i,%i,%i,%i,%i,w:%c\n",bm,bk,wm,wk,bmrank,wmrank,c[singlevalue]);
		}

	totalbytescompressed+=compressedsize;
	ratio = (float)compressedsize/(dbsize/4);
	if(compressedsize!=0)
		printf("\nsize: uncompressed %i compressed %i, ratio %.3f overall %.3f", (int)dbsize/4, compressedsize,ratio,totalbytescompressed/totalbytes);

	VirtualFree(currentdb_b , 0, MEM_RELEASE);
	VirtualFree(currentdb_w , 0, MEM_RELEASE);
	return 1;
	}


int floatnondominant(int *database,int dbsize, int dominantvalue,int nondominantvalue)
	{
	int index;
	int value;

	for(index=0;index<dbsize;index++)
		{
		value=getdatabasevalue(database, index);
		if(value != dominantvalue && value != UNKNOWN && value!=nondominantvalue)
			{
			//printf("\nsetting value");
			setdatabasevalue(database,index,nondominantvalue);
			}
		}
	return 1;
	}

int finddominantvalue(int *database, int dbsize, int *w, int *d, int *l)
	{
	// returns the dominant value in a database:
	// sets wins draws and losses in *w, *d, *l
	int32 wins, draws, losses;
	int32 result[4];
	int32 index;
	int value, dominantvalue;

	wins=0;draws=0;losses=0;
	result[0]=0;result[1]=0;result[2]=0;result[3]=0;

	for(index=0;index<dbsize;index++)
		{
		value = getdatabasevalue(database, index);
		result[value]++;
		}


	wins = result[1];
	draws = result[3];
	losses = result[2];

	*w = wins;
	*d = draws;
	*l = losses;
	dominantvalue = WIN;
	if(draws>wins)
		dominantvalue = DRAW;
	if(losses>wins && losses>draws)
		dominantvalue = LOSS;
	printf("  %i wins, %i draws, %i losses", wins, draws, losses);

	return dominantvalue;
	}

int compressdb(int *database, int dbsize, char dbname[256])
	{
	int index, i, j, lastvalue;
	unsigned char byteout;
	int compressedsize = 0;
	int idxnumber=0;
	int compbytes=0;
	int literalbytes=0;
	

	for(index=0;index<dbsize;i=0)
		{
		i=0;
		// get next value
		lastvalue = getdatabasevalue(database, index);
		// try how far it remains the same
		while((index+i+1<dbsize) && (getdatabasevalue(database, index+i+1)==lastvalue) && (i<MAXSKIP-1))
			i++;
		
		// now, we know that databasevalues from index up to index+i are all the same.
		// change i to i+1, to say that there are i+1 same values in a row:
		i++;

#ifdef DEBUG
		if(i>MAXSKIP)
			{
			printf("\ni, %i, is larger than MAXSKIP",i);
			getch();
			}
#endif

		rle[i]++;

		// get byte to write:
		if(i>4)
			{
			// we use a compressed byte here:
			// now, we find the largest skip[x] which is smallerorequal to i:
			// to facilitate this, we initialized an array at the program start!

#ifndef MOCKENCODE
			j=skipindex[i];
			// j is smaller or equal than i. 
			// e.g. for i=7; j is also 7.
			// now, skip[j] is the run length we can encode
			i=skip[j];
#else
			j=0;
#endif
	
			index+=i;
			byteout=81+(lastvalue-1)*58+j;
			compbytes++;
#ifdef DEBUG
			if(byteout>255 || byteout<81)
				{
				printf("compression error on RLE byteout is %i",byteout);
				getch();
				}
#endif
			}
		else
			{
			if(index+3<dbsize)
				{
				byteout=  getdatabasevalue(database,index)-1+3*(getdatabasevalue(database,index+1)-1) + 
						9*(getdatabasevalue(database,index+2)-1)+27*(getdatabasevalue(database,index+3)-1);
				}
			else
				{
				byteout = getdatabasevalue(database,index)-1;
				if(index+1<dbsize)
					byteout+=3*(getdatabasevalue(database,index+1)-1);
				if(index+2<dbsize)
					byteout+=9*(getdatabasevalue(database,index+2)-1);
				}

			if(byteout<0 || byteout>80)
				{
				printf("compression error on literal byteout is %i",byteout);
				getch();
				}
			index+=4;
			literalbytes++;
			}
		//-------------------------------------------------------------
		// done getting the compressed byte!
		// now we have a byte to write in variable byteout
		compressedsize++;
		block[blockindex]=byteout;
		blockindex++;
		blockindex%=1024;
		if(blockindex==0)
			{
			// block is full, write to disk:
			fwrite(block,1024,1,fp_cpr);
			blocknumber++;
			// write index 
			fprintf(fp_idx,"%i\n",index);
			}
		}
	totalcompbytes += compbytes;
	totalliteralbytes += literalbytes;
	return compressedsize;
	}




int setdatabasevalue(int *database, int index, int value)
	// sets a value in the database
	{
	int32 tmp;
	int32 mask;

	tmp = database[index/16];
	mask = 3 << (2*(index%16));
	mask = ~mask;
	tmp &= mask;

	tmp+= value << (2*(index%16));
	database[index/16] = tmp;
	
	return 1;
	}


int runlengthmaximize(int *database, int dbsize, int dominantvalue)
	// maximizes the run lengths in the db sprinkled with UNKNOWNS 
	{
	int lastvalue, index, value;

	lastvalue = UNKNOWN;
	index=0;
	while (lastvalue==UNKNOWN && index<dbsize)
		{
		lastvalue = getdatabasevalue(database, index);
		index++;
		}

	for(index=0;index<dbsize;index++)
		{
		value = getdatabasevalue(database, index);
		if(value == UNKNOWN && lastvalue==dominantvalue)
			setdatabasevalue(database, index, lastvalue);
		else
			lastvalue=value;
		}

	lastvalue = UNKNOWN;
	index=dbsize-1;
	while (lastvalue==UNKNOWN && index>=0)
		{
		lastvalue = getdatabasevalue(database, index);
		index--;
		}

	for(index=dbsize-1;index>=0;index--)
		{
		value = getdatabasevalue(database, index);
		if(value == UNKNOWN && lastvalue==dominantvalue)
			setdatabasevalue(database, index, lastvalue);
		else
			lastvalue=value;
		}


	// in second pass, just set unknown values to neighboring values
	lastvalue = UNKNOWN;
	index=0;
	while (lastvalue==UNKNOWN && index<dbsize)
		{
		lastvalue = getdatabasevalue(database, index);
		index++;
		}

	for(index=0;index<dbsize;index++)
		{
		value = getdatabasevalue(database, index);
		if(value == UNKNOWN)
			setdatabasevalue(database, index, lastvalue);
		else
			lastvalue=value;
		}
	return 1;
	}

int runlengthoptimize(int *database, int dbsize, int dominantvalue)
	// optimizes the run lengths in the db sprinkled with UNKNOWNS 
	{
	int lastvalue, index, value, nextvalue;
	int i,j,k;
	int runlength;
	int encodelength;
	// "seed pass - make sure the left edge of the db is "anchored"
	// get first value in db
	lastvalue = UNKNOWN;
	index=0;
	while (lastvalue==UNKNOWN && index<dbsize)
		{
		lastvalue = getdatabasevalue(database, index);
		index++;
		}
	// and spread it to the left.
	for(i=0;i<index;i++)
		setdatabasevalue(database, i, lastvalue);

	// "seed pass 2 - make sure the right edge of the db is "anchored"
	// get first value in db
	lastvalue = UNKNOWN;
	index=dbsize-1;
	while (lastvalue==UNKNOWN && index>=0)
		{
		lastvalue = getdatabasevalue(database, index);
		index--;
		}
	// and spread it to the left.
	for(i=index+1;i<dbsize;i++)
		setdatabasevalue(database, i, lastvalue);


	// next, fill gaps like 111uuuu1111 with 1's.
	for(index=0;index<dbsize;index++)
		{
		//search an unknown
		lastvalue = getdatabasevalue(database, index);
		if(lastvalue!=UNKNOWN)
			continue;
		
		lastvalue = getdatabasevalue(database, index-1);
		value=UNKNOWN;
		// we get here means: at "index" an unknown run starts.
		i=index;
		while(value == UNKNOWN)
			{
			i++;
			value = getdatabasevalue(database, i);
			}
		// when we get out here, the unknown run stops at i-1
		// fill it in, if value==lastvalue
		if(value!=lastvalue)
			{
			index=i;
			continue;
			}

		//we get here: we fill in the unknowns
		for(j=index;j<i;j++)
			setdatabasevalue(database,j,value);
		}

	// ok, obvious values are done now.

	// now, move from left to right through db, until we hit a transition from
	// know to unknown. then, count the number of consecutive known values to the
	// left. next, count the number of unknowns to the right. 
	for(index=0;index<dbsize;index++)
		{
		value = getdatabasevalue(database, index);
		if(value!=UNKNOWN)
			continue;
		lastvalue=value;
		// come here: an unknown run starts at "index".
		// check how long it is.
		i=index;
		while(value==UNKNOWN)
			{
			i++;
			value=getdatabasevalue(database,i);
			}
		// when we drop out here, an unknown run starts at index. it finishes at i-1.
		// the left value is lastvalue. the right value is value. 
		// now, add run length of known run to the left:
		for(j=index-1;j>=0;j--)
			{
			if(getdatabasevalue(database,j)!=lastvalue)
				break;
			}

		// example:
		// w w w d d u u u u u u u u u u l l l
		//     ^     ^                   ^
		//     j   index                 i
		//     lastvalue d               value l
		// => run length of total run we want to encode is j-j-1;
		runlength = i-j-1;
		// find length that could be encoded in one byte:
		if(runlength>4)
			encodelength = skip[skipindex[runlength]];
		else
			encodelength = 0;
		// now, there are two possible cases:
		// encodelength is smaller than the run of known values. that would be bad. 
		
		if (encodelength < index-j-1)
			{
			// bad case - just fill run with left value
			for(j=index;j<i;j++)
				setdatabasevalue(database,j,lastvalue);
			}
		else
			{
			// good case. 
			for(k=index; k<i;k++)
				{
				if(k<j+encodelength+1)
					setdatabasevalue(database,k,lastvalue);
				else
					setdatabasevalue(database,k,value);
				}
			}

		index = i;
		}

	
	return 1;
	}





int getdatabasevalue(int *database, int index)
	// reads a value in the database
	{
	int32 value=UNKNOWN;
	//printf("%i ", index);
	// there are 16 entries per integer, therefore:
	value = database[index/16];

	value = (value >> (2*(index%16)) ) & 3;

	return value;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
// initialization stuff below



int getdatabasesize(int bm, int bk, int wm, int wk, int bmrank, int wmrank)
	{
	// returns the range of database indices for this database.
	// needs binomial coefficients in the array bicoef[][] = choose from n, k
	int dbsize = 1;

	// number of bm configurations:
	// there are bm black men subject to the constraint that one of them is on 
	// the rank bmrank

	if(bm)
		dbsize *= bicoef[4*(bmrank+1)][bm] - bicoef[4*bmrank][bm];
  
	if(wm)
		dbsize *= bicoef[4*(wmrank+1)][wm] - bicoef[4*wmrank][wm];

	// number of bk configurations
	if(bk)
		dbsize *= bicoef[32-bm-wm][bk];

	// number of wk configurations
	if(wk)
		dbsize *= bicoef[32-bm-wm-bk][wk];


	return dbsize;
	}


int initbicoef(void)
	{
	// initializes the database records needed later for efficient access
	// of these databases
	// subdatabase[bm][bk][wm][wk][maxbm][maxwm]

	int i,j;

	// initialize binomial coefficients
	// bicoef[n][k] is supposed to be the number of ways you can choose k from n
	for(i=0;i<33;i++)
		{
		for(j=1;j<=i;j++)
			{
			// choose j from i:
			bicoef[i][j] = choose(i,j);
			}
		// define choosing 0 for simplicity 
		bicoef[i][0] = 1;
		}

	// choosing n from 0: bicoef = 0
	for(i=1;i<33;i++)
		bicoef[0][i]=0;
	for(i=0;i<33;i++)
	{
		for(j=0;j<33;j++)
			transposedbicoef[i][j]=bicoef[j][i];
	}


	return 1;
	}


int choose(int n, int k)
	{
	int result = 1;
	int i;

	i=k;
	while(i)
		{
		result *= (n-i+1);
		i--;
		}

	i=k;
	while(i)
		{
		result /=i;
		i--;
		}

	return result;
	}


void indextoposition(int32 index, position *p, int bm, int bk, int wm, int wk, int bmrank, int wmrank, int *color)
	{
	// reverse the position to index thing

	//int halfsize;
	int32 bmindex=0, bkindex=0, wmindex=0, wkindex=0;
	int32 bmrange=1, bkrange=1, wmrange=1, wkrange=1;
	
	int multiplier;
	int32 square_one=1;
	int32 occupied;
	int i,j,k,f;
	int bkpos[MAXPIECE],wkpos[MAXPIECE];
	
	p->bm = 0;
	p->bk = 0;
	p->wm = 0;
	p->wk = 0;
	

	// extract the indexes for the piece types from the total index:
	// get multiplier for wkindex:
	// possibly not using the "if" is faster - because if can lead to
	// a branch misprediction
	if(bm)
		bmrange = bicoef[4*(bmrank+1)][bm] - bicoef[4*bmrank][bm];
		
	if(wm)
		wmrange = bicoef[4*(wmrank+1)][wm] - bicoef[4*wmrank][wm];
		
	if(bk)
		bkrange = bicoef[32-bm-wm][bk];

	if(wk)
		wkrange = bicoef[32-bm-wm-bk][wk];
		
	
	multiplier = bmrange*wmrange*bkrange;
	wkindex = index / multiplier;
	index -= wkindex*multiplier;

	multiplier = bmrange*wmrange;
	bkindex = index / multiplier;
	index -= bkindex*multiplier;

	wmindex = index / bmrange;
	index -= wmindex*bmrange;

	bmindex = index;
	

	// add the rank index

	if(bm)
		bmindex += bicoef[4*bmrank][bm];
	if(wm)
		wmindex += bicoef[4*wmrank][wm];

	// now that we know the index numbers, we extract the pieces 
	// extract black men directly
	i=27;
	j=bm;
	// this stuff here takes lots of time. how can it be improved??
	// ed's idea: change the search from linear to binary.
	// my idea: create bicoeftransposed[i][j] which is bicoef[j][i].
	// looping over the second index should be faster!
	while(j)
		{
		//while(bicoef[i][j]>bmindex)
		while(transposedbicoef[j][i]>bmindex)
			i--;
		//bmindex-=bicoef[i][j];
		bmindex-=transposedbicoef[j][i];
		p->bm |= square_one<<i;
		j--;
		}

	// extract white men directly
	i=27;
	j=wm;
	while(j)
		{
		//while(bicoef[i][j]>wmindex)
		while(transposedbicoef[j][i]>wmindex)
			i--;
		//wmindex-=bicoef[i][j];
		wmindex-=transposedbicoef[j][i];
		p->wm |= square_one<<(31-i);
		j--;
		}

	// extract positions of black kings
	i=31;
	for(j=bk;j>0;j--)
		{
		//while(bicoef[i][j]>bkindex)
		while(transposedbicoef[j][i]>bkindex)
			i--;
		//bkindex-=bicoef[i][j];
		bkindex-=transposedbicoef[j][i];
		bkpos[j-1]=i;
		}
	
	// extract positions of white kings
	i=31;
	for(j=wk;j>0;j--)
		{
		//while(bicoef[i][j]>wkindex)
		while(transposedbicoef[j][i]>wkindex)
			i--;
		//wkindex-=bicoef[i][j];
		wkindex-=transposedbicoef[j][i];
		wkpos[j-1]=i;
		}
	
	// now, put black kings on the board. we know: bkpos[0]...bkpos[bk-1] is ordered
	// with bkpos[0] being the smallest one, same goes for wkpos;

	occupied = p->bm|p->wm;
	k=0;
	f=0;
	for(i=0;i<32,k<bk;i++)
		{
		if(occupied & (square_one<<i))
			continue;
		if(bkpos[k] == f)
			{
			p->bk|=(square_one<<i);
			k++;
			}
		f++;
		}
	
	occupied = p->bm|p->wm|p->bk;
	k=0;
	f=0;
	for(i=0;i<32,k<wk;i++)
		{
		if(occupied & (square_one<<i))
			continue;
		if(wkpos[k] == f)
			{
			p->wk|=(square_one<<i);
			k++;
			}
		f++;
		}
	
	}

	
void fastindextoposition(int32 index, position *p, int bm, int bk, int wm, int wk, int bmrank, int wmrank, int *color)
	{
	// reverse the position to index thing

	//int halfsize;
	int32 bmindex=0, bkindex=0, wmindex=0, wkindex=0;
	
	int32 square_one=1;
	int32 occupied;
	int i,j,k,f;
	int bkpos[MAXPIECE],wkpos[MAXPIECE];


	p->bm = 0;
	p->bk = 0;
	p->wm = 0;
	p->wk = 0;
	

	// extract the indexes for the piece types from the total index:
	// get multiplier for wkindex:
	// possibly not using the "if" is faster - because if can lead to
	// a branch misprediction
	/*if(bm)
		bmrange = bicoef[4*(bmrank+1)][bm] - bicoef[4*bmrank][bm];
		
	if(wm)
		wmrange = bicoef[4*(wmrank+1)][wm] - bicoef[4*wmrank][wm];
		
	if(bk)
		bkrange = bicoef[32-bm-wm][bk];

	if(wk)
		wkrange = bicoef[32-bm-wm-bk][wk];*/
		
	
	//multiplier = bmrange*wmrange*bkrange;
	wkindex = index / gmultiplier1;
	index -= wkindex*gmultiplier1;

	//multiplier = bmrange*wmrange;
	bkindex = index / gmultiplier2;
	index -= bkindex*gmultiplier2;

	wmindex = index / gbmrange;
	index -= wmindex*gbmrange;

	bmindex = index;
	

	// add the rank index

	if(bm)
		bmindex += bicoef[4*bmrank][bm];
	if(wm)
		wmindex += bicoef[4*wmrank][wm];

	// now that we know the index numbers, we extract the pieces 
	// extract black men directly
	i=27;
	j=bm;
	// this stuff here takes lots of time. how can it be improved??
	// ed's idea: change the search from linear to binary.
	// my idea: create bicoeftransposed[i][j] which is bicoef[j][i].
	// looping over the second index should be faster!
	while(j)
		{
		//while(bicoef[i][j]>bmindex)
		while(transposedbicoef[j][i]>bmindex)
			i--;
		//bmindex-=bicoef[i][j];
		bmindex-=transposedbicoef[j][i];
		p->bm |= square_one<<i;
		j--;
		}


	// extract white men directly

	i=27;
	j=wm;
	while(j)
		{
		//while(bicoef[i][j]>wmindex)
		while(transposedbicoef[j][i]>wmindex)
			i--;
		//wmindex-=bicoef[i][j];
		wmindex-=transposedbicoef[j][i];
		p->wm |= square_one<<(31-i);
		j--;
		}

	// extract positions of black kings
	i=31;
	for(j=bk;j>0;j--)
		{
		//while(bicoef[i][j]>bkindex)
		while(transposedbicoef[j][i]>bkindex)
			i--;
		//bkindex-=bicoef[i][j];
		bkindex-=transposedbicoef[j][i];
		bkpos[j-1]=i;
		}
	
	// extract positions of white kings
	i=31;
	for(j=wk;j>0;j--)
		{
		//while(bicoef[i][j]>wkindex)
		while(transposedbicoef[j][i]>wkindex)
			i--;
		//wkindex-=bicoef[i][j];
		wkindex-=transposedbicoef[j][i];
		wkpos[j-1]=i;
		}
	
	// now, put black kings on the board. we know: bkpos[0]...bkpos[bk-1] is ordered
	// with bkpos[0] being the smallest one, same goes for wkpos;

	occupied = p->bm|p->wm;
	k=0;
	f=0;
	for(i=0;i<32,k<bk;i++)
		{
		if(occupied & (square_one<<i))
			continue;
		if(bkpos[k] == f)
			{
			p->bk|=(square_one<<i);
			k++;
			}
		f++;
		}
	
	occupied = p->bm|p->wm|p->bk;
	k=0;
	f=0;
	for(i=0;i<32,k<wk;i++)
		{
		if(occupied & (square_one<<i))
			continue;
		if(wkpos[k] == f)
			{
			p->wk|=(square_one<<i);
			k++;
			}
		f++;
		}
	
	}

