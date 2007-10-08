// --------------------------------------------------------------------------
// Citadel: Version.CPP
//
// This defines the version, name, and other stuff relating to the program.

#include "ctdl.h"

#define MAJORVERSION	"/066"
#define REVISION        "73"

// --------------------------------------------------------------------------
// This has to be in the format of MAJORVERSION then REVISION as three
// digits. This has to always become a higher number as time goes on.
// It is used to know if datafiles need to be cleaned up in some way. For
// example, MorePrompt in the log buffer started to be used in version 
// 65074; if the last time a user called the version was less than 65074,
// then we set the default moreprompt.

long NumericVer = 66073l;

#if VERSION == ALPHA
	#define MINORVERSION	"à" REVISION
#elif VERSION == BETA
	#define MINORVERSION	"á" REVISION
#elif VERSION == PUBLICTEST
	#if defined(AUXMEM) || defined(WINCIT)
		#define MINORVERSION	"pà" REVISION
	#else
		#define MINORVERSION	"pá" REVISION
	#endif
#elif VERSION == RELEASE
	#define MINORVERSION
#else
	#error VERSION not defined properly.
#endif

char version[]		= MAJORVERSION MINORVERSION;

// --------------------------------------------------------------------------
// For limited releases, set this to the result of hash() on the board name.
// not super secure, but secure enough. 0 disables checking.
//
// hash("The Laboratory") == 14024u
// hash("Slumberland") == 15418u
// hash("starship inconnu") == 701u
// hash("vulgar display of power") == 7861u
// hash("the ghetto") == 13866u
// hash("milliways") == 12652u
// hash("memory alpha") == 9935u
// hash("the asylum") == 15919u
// hash("time out of mind") == 2988u
// hash("raspberry") == 12869u
// hash("freezing rain") == 14454u
// hash("starfleet academy") == 1358u
// hash("fish lair") == 15562u
// ni 2 = 460u
// harry's house = 488u
// xorcit one = 8898u

ushort BoardNameHash = 0u;


#ifdef WINCIT
char programName[]	= "Citadel+/Win32";
#else
char programName[]	= "Citadel+";
#endif

char Author[] = "By Anticlimactic Teleservices";
char cmpDate[]	= __DATE__;
char cmpTime[]	= __TIME__;
