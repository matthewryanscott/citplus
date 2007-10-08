// --------------------------------------------------------------------------
// Citadel: FileRdWr.H
//
// General-purpose file read/write function prototypes

Bool GetStr(FILE *fl, char *str, int mlen);
Bool net69_GetStr(FILE *fl, char *str, int mlen);
void net69_PutStr(FILE *fl, const char *str);
Bool PutStr(FILE *fl, const char *str);
