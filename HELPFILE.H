#ifdef MAIN
char *hlpnames[] =
	{
#ifdef WINCIT
    "Accounting",       "Aide",             "ANSI",
	"Configuration",	"DoHelp",			"Doors",
	"Files",			"Groups",			"Halls",
	"Help",				"Introduction", 	"Login",
	"Messages",			"Network",			"Queue",
	"Rooms",			"Room System",		"Special",
    "Spell Checker",    "Sysop",            "Chat",
#else
	"ACCOUNT",			"AIDE",				"ANSI",
	"CONFIG",			"DOHELP",			"DOORS",
	"FILES",			"GROUPS",			"HALLS",
	"HELP",				"INTRO",			"LOGIN",
	"MESSAGES",			"NETWORK",			"QUEUE",
	"ROOMS",			"ROOMSYS",			"SPECIAL",
	"SPELL",			"SYSOP",
#endif

	NULL
	};
#else
extern char *hlpnames[];
#endif

