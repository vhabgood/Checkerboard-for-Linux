// 
// bool.c - performs some boolean operations
//

// must be initialized with a call to initbool()

#include "checkers.h"
#include "bool.h"

static char LSBarray[256];
static char MSBarray[256];
static char bitsinword[65536];
static int32 revword[65536];
//unsigned char tab[256];

/*>(3) They did not try to remove poorly predicted branch from their implementation
>of the LOOKUP algorithm. For example, on x86 there is no branches in the
>following variant:
>
>  extern unsigned char tab[256];
>
>  unsigned Lookup8 (unsigned i)
>  {
>      unsigned b, n;
>
>      b = ((i & 0xFFFF0000) != 0);
>      n = b * 16;
>      i >>= b * 16;
>      b = ((i & 0x0000FF00) != 0);
>      n += b * 8;
>      i >>= b * 8;
>      return n + tab[i];
>  }
>
>Eugene
*/
/*
unsigned lookup(unsigned i)
  {
      unsigned b, n;

      b = ((i & 0xFFFF0000) != 0);
      n = b * 16;
      i >>= b * 16;
      b = ((i & 0x0000FF00) != 0);
      n += b * 8;
      i >>= b * 8;
      return n + tab[i];
  }
*/
int LSB(int32 x)
	{
	/* returns the position of the least significant bit in x */
	/* or -1 if not found */
	if(x&0x000000FF)
		return(LSBarray[x&0x000000FF]);
	if(x&0x0000FF00)
		return(LSBarray[(x>>8)&0x000000FF]+8);
	if(x&0x00FF0000)
		return(LSBarray[(x>>16)&0x000000FF]+16);
	return(LSBarray[(x>>24)&0x000000FF]+24);
	}

int MSB(int32 x)
	{
	/* returns the position of the most significant bit in x */
	/* or -1 if not found */
	if(x&0xFF000000)
		return(MSBarray[(x>>24)&0xFF]+24);
	if(x&0x00FF0000)
		return(MSBarray[(x>>16)&0xFF]+16);
	if(x&0x0000FF00)
		return(MSBarray[(x>>8)&0xFF]+8);
	return(MSBarray[x&0xFF]);
	}


void initbool(void)
	{
	int i,j;
	/* initialize array for "LSB" */
	for(i=0;i<256;i++)
		{
		if(i&1) {LSBarray[i]=0;continue;}
		if(i&2) {LSBarray[i]=1;continue;}
		if(i&4) {LSBarray[i]=2;continue;}
		if(i&8) {LSBarray[i]=3;continue;}
		if(i&16) {LSBarray[i]=4;continue;}
		if(i&32) {LSBarray[i]=5;continue;}
		if(i&64) {LSBarray[i]=6;continue;}
		if(i&128) {LSBarray[i]=7;continue;}
		LSBarray[i]=-1;
		}
	//for(i=0;i<256;i++)
	//	tab[i]=LSBarray[i];
	/* initialize array for "MSB" */
	for(i=0;i<256;i++)
		{
		if(i&128) {MSBarray[i]=7;continue;}
		if(i&64) {MSBarray[i]=6;continue;}
		if(i&32) {MSBarray[i]=5;continue;}
		if(i&16) {MSBarray[i]=4;continue;}
		if(i&8) {MSBarray[i]=3;continue;}
		if(i&4) {MSBarray[i]=2;continue;}
		if(i&2) {MSBarray[i]=1;continue;}
		if(i&1) {MSBarray[i]=0;continue;}
		MSBarray[i]=-1;
		}

	// initialize bitsinword, the number of bits in a word
	for(i=0;i<65536;i++)
		bitsinword[i]=recbitcount((int32)i);

	// initialize revword, the reverse of a word.
	for(i=0;i<65536;i++)
		{
		revword[i]=0;
		for(j=0;j<16;j++)
			{
			if(i&(1<<j))
				revword[i] +=1<<(15-j);
			}
		}
	}

int revert(int32 n)
	// reverses a 4-byte integer
	{
	int32 value = 0;
	value = revword[hiword(n)];
	value += (revword[loword(n)]<<16);
	return value;
	}

int recbitcount(int32 n)
	/* counts & returns the number of bits which are set in a 32-bit integer
		slower than a table-based bitcount if many bits are
		set. used to make the table for the table-based bitcount on initialization
	*/
	{
	int r=0;
	while(n)
		{
		n=n&(n-1);
		r++;
		}
	return r;
	}

/* table-lookup bitcount */

int bitcount(int32 n)
	/* returns the number of bits set in the 32-bit integer n */
	{
	return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
	}


	
