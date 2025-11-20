// dblookup.h
// all you need.

#include "checkers_types.h"

// include 



// choose which db you want to use


#ifdef __cplusplus
extern "C" {
#endif

//int initdblookup(char str[256]);
int db_init(int suggestedMB, char str[256], const char* EGTBdirectory);
int db_exit(void);
int dblookup(pos *p,int cl);
int revert(int32 n);
int db_getcachesize(void);
void db_infostring(char *str);
int LSB(int32 x);
int MSB(int32 x);

#ifdef __cplusplus
}
#endif