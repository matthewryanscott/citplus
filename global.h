// --------------------------------------------------------------------------
// Citadel: Global.H
//
// Many of the global variables in Citadel.


// --------------------------------------------------------------------------
// initialized variables

// ------------------------------------------------------------ version stuff
extern char version[];
extern long NumericVer;
extern char programName[];

extern ushort BoardNameHash;

extern char Author[];
extern char cmpDate[];
extern char cmpTime[];

#ifdef MAIN
// ----------------------------------------------------- file names and stuff
EXTERN char etcTab[]        = "ETC.TAB";
EXTERN char hallDat[]       = "HALL.DAT";
EXTERN char hall2Dat[]      = "HALL2.DAT";
EXTERN char grpDat[]        = "GRP.DAT";
EXTERN char roomDat[]       = "ROOM.DAT";
EXTERN char msgDat[]        = "MSG.DAT";
EXTERN char logDat[]        = "LOG.DAT";
EXTERN char log2Dat[]       = "LOG2.DAT";
EXTERN char log3Dat[]       = "LOG3.DAT";
EXTERN char log4Dat[]       = "LOG4.DAT";
EXTERN char log5Dat[]       = "LOG5.DAT";
EXTERN char log6Dat[]       = "LOG6.DAT";
EXTERN char roomposDat[]    = "ROOMPOS.DAT";
EXTERN char cronTab[]       = "CRON.TAB";
EXTERN char lgTab[]         = "LOG.TAB";
EXTERN char rmTab[]         = "ROOM.TAB";
EXTERN char roomreqIn[]     = "ROOMREQ.IN";
EXTERN char roomreqOut[]    = "ROOMREQ.OUT";
EXTERN char roomreqTmp[]    = "ROOMREQ.TMP";
EXTERN char roomdataIn[]    = "ROOMDATA.IN";
EXTERN char roomdataOut[]   = "ROOMDATA.OUT";
EXTERN char messageApl[]    = "MESSAGE.APL";
EXTERN char ctdlDat[]       = "CTDL.DAT";
EXTERN char mesgTmp[]       = "MESG.TMP";
EXTERN char roomStar[]      = "ROOM.*";
EXTERN char outputApl[]     = "OUTPUT.APL";
EXTERN char inputApl[]      = "INPUT.APL";
EXTERN char readmeApl[]     = "README.APL";
EXTERN char consoleApl[]    = "CONSOLE.APL";
EXTERN char starstar[]      = "*.*";
EXTERN char ctdlMnu[]       = "CTDL.MNU";
EXTERN char ctdlMna[]       = "CTDL.MN@";
EXTERN char bordersDat[]    = "BORDERS.DAT";

EXTERN char sbs[]           = "%s\\%s";

EXTERN char FO_A[]          = "a";
EXTERN char FO_AB[]         = "ab";
EXTERN char FO_AP[]         = "a+";
EXTERN char FO_R[]          = "r";
EXTERN char FO_W[]          = "w";
EXTERN char FO_WB[]         = "wb";
EXTERN char FO_RB[]         = "rb";
EXTERN char FO_RPB[]        = "r+b";
EXTERN char FO_WPB[]        = "w+b";

// --------------------------------------------------------------- TERM_STUFF
// CTRL_A Stuff
EXTERN char MCICodes[][3] =
    {
    "rn",   // MCI_REALNAME
    "ln",   // MCI_LASTNAME
    "pn",   // MCI_PHONENUM
    "ad",   // MCI_ADDR
    "lc",   // MCI_LASTCALLD
    "ab",   // MCI_ACCTBAL
    "rm",   // MCI_ROOMNAME
    "hn",   // MCI_HALLNAME
    "nn",   // MCI_NODENAME
    "np",   // MCI_NODEPHONE
    "df",   // MCI_DISKFREE
    "nm",   // MCI_NUMMSGS
    "cl",   // MCI_CLS
    "cn",   // MCI_CALLNUM
    "sn",   // MCI_SYSOPNAME
    "cr",   // MCI_CONNRATE
    "pr",   // MCI_PORTRATE
    "mr",   // MCI_MSGROOM
    "bp",   // MCI_BEEPNUM
    "gc",   // MCI_GETCHAR
    "gs",   // MCI_GETSTR
    "pc",   // MCI_PUTCHAR
    "ps",   // MCI_PUTSTR
    "bs",   // MCI_BSNUM
    "os",   // MCI_OUTSPEC
    "sl",   // MCI_SLOW
    "pw",   // MCI_PASSWORD
    "in",   // MCI_INITIALS
    "go",   // MCI_GOTO
    "lb",   // MCI_LABEL
    "cm",   // MCI_COMPARE
    "gt",   // MCI_GT
    "lt",   // MCI_LT
    "eq",   // MCI_EQ
    "rd",   // MCI_RANDOM
    "as",   // MCI_ASSIGN
    "++",   // MCI_ADD
    "--",   // MCI_SUBTRACT
    "**",   // MCI_TIMES
    "//",   // MCI_DIVIDE
    "an",   // MCI_ASGNNEXT
    "hi",   // MCI_HANGINGINDENT
    "hf",   // MCI_HELPFILE
    };

EXTERN char TERM_NORMAL[]       = "0";
EXTERN char TERM_BLINK[]        = "1";
EXTERN char TERM_REVERSE[]      = "2";
EXTERN char TERM_BOLD[]         = "3";
EXTERN char TERM_UNDERLINE[]    = "4";
EXTERN char TERM_FMT_OFF[]      = "5";
EXTERN char TERM_FMT_ON[]       = "6";
EXTERN char TERM_SPLCHK_OFF[]   = "7";
EXTERN char TERM_SPLCHK_ON[]    = "8";
EXTERN char TERM_USERNAME[]     = "N";
EXTERN char TERM_FIRSTNAME[]    = "n";
EXTERN char TERM_TIME[]         = "t";
EXTERN char TERM_DATE[]         = "T";
EXTERN char TERM_POOP[]         = "P";
EXTERN char TERM_RND_BACK[]     = "R";
EXTERN char TERM_RND_FORE[]     = "r";

// CTRL_B Stuff
EXTERN char TERM_BS[]           = "B";
EXTERN char TERM_IMPERV[]       = "I";
EXTERN char TERM_PAUSE[]        = "P";
EXTERN char TERM_HANGUP[]       = "H";
EXTERN char TERM_CONT[]         = "C";
EXTERN char TERM_MCI_ON[]       = "M";
EXTERN char TERM_MCI_DEF[]      = "m";
EXTERN char TERM_REPLACE_ON[]   = "R";
EXTERN char TERM_REPLACE_OFF[]  = "r";

// ----------------------------------------------------------------- messages
EXTERN char spcts[]     = " %s";
EXTERN char pctd[]      = "%d";
EXTERN char pcthd[]     = "%hd";
EXTERN char pctld[]     = "%ld";
EXTERN char pctlu[]     = "%lu";
EXTERN char pct2d[]     = "%2d";
EXTERN char pctc[]      = "%c";
EXTERN char pctss[]     = "%s ";
EXTERN char sqst[]      = " ?";
EXTERN char bn[]        = "\n";
EXTERN char br[]        = "\r";
EXTERN char spcspc[]    = "  ";


// -------------------------------------------------------------------- other
EXTERN int          scrollpos   = 23;
EXTERN uint cdecl   conRows     = 24;
EXTERN uint cdecl   conCols     = 80;
EXTERN int cdecl    conMode     = -1;
EXTERN PortSpeedE   slv_baud    = PS_ERROR;
EXTERN int          cmd_mdata   = CERROR;

EXTERN long bauds[] =
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400l, 57600l, 115200l};

EXTERN long connectbauds[] =
    {300, 600, 1200, 2400, 4800, 7200, 9600, 12000, 14400, 16800, 19200,
    21600, 24000, 26400, 28800, 38400l, 57600l, 115200l, 230400l};

EXTERN int iso_clr[]        = {0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7};

#else

// --------------------------------------------------------------- file names
EXTERN char etcTab[];
EXTERN char hallDat[];
EXTERN char hall2Dat[];
EXTERN char grpDat[];
EXTERN char roomDat[];
EXTERN char msgDat[];
EXTERN char logDat[];
EXTERN char log2Dat[];
EXTERN char log3Dat[];
EXTERN char log4Dat[];
EXTERN char log5Dat[];
EXTERN char log6Dat[];
EXTERN char roomposDat[];
EXTERN char cronTab[];
EXTERN char lgTab[];
EXTERN char rmTab[];
EXTERN char roomreqIn[];
EXTERN char roomreqOut[];
EXTERN char roomreqTmp[];
EXTERN char roomdataIn[];
EXTERN char roomdataOut[];
EXTERN char messageApl[];
EXTERN char dohelp[];
EXTERN char ctdlDat[];
EXTERN char mesgTmp[];
EXTERN char roomStar[];
EXTERN char outputApl[];
EXTERN char inputApl[];
EXTERN char readmeApl[];
EXTERN char consoleApl[];
EXTERN char starstar[];
EXTERN char ctdlMnu[];
EXTERN char ctdlMna[];
EXTERN char bordersDat[];

EXTERN char sbs[];

EXTERN char FO_A[];
EXTERN char FO_AB[];
EXTERN char FO_AP[];
EXTERN char FO_R[];
EXTERN char FO_W[];
EXTERN char FO_WB[];
EXTERN char FO_RB[];
EXTERN char FO_RPB[];
EXTERN char FO_WPB[];

// --------------------------------------------------------------- TERM_STUFF
EXTERN char MCICodes[][3];

// CTRL_A Stuff
EXTERN char TERM_NORMAL[];
EXTERN char TERM_BLINK[];
EXTERN char TERM_REVERSE[];
EXTERN char TERM_BOLD[];
EXTERN char TERM_UNDERLINE[];
EXTERN char TERM_FMT_OFF[];
EXTERN char TERM_FMT_ON[];
EXTERN char TERM_SPLCHK_OFF[];
EXTERN char TERM_SPLCHK_ON[];
EXTERN char TERM_USERNAME[];
EXTERN char TERM_FIRSTNAME[];
EXTERN char TERM_TIME[];
EXTERN char TERM_DATE[];
EXTERN char TERM_POOP[];
EXTERN char TERM_RND_BACK[];
EXTERN char TERM_RND_FORE[];

// CTRL_B Stuff
EXTERN char TERM_BS[];
EXTERN char TERM_IMPERV[];
EXTERN char TERM_PAUSE[];
EXTERN char TERM_HANGUP[];
EXTERN char TERM_CONT[];
EXTERN char TERM_MCI_ON[];
EXTERN char TERM_MCI_DEF[];
EXTERN char TERM_REPLACE_ON[];
EXTERN char TERM_REPLACE_OFF[];

// ----------------------------------------------------------------- messages
EXTERN char spcts[];
EXTERN char pctd[];
EXTERN char pcthd[];
EXTERN char pctld[];
EXTERN char pctlu[];
EXTERN char pct2d[];
EXTERN char pctc[];
EXTERN char pctss[];
EXTERN char sqst[];
EXTERN char bn[];
EXTERN char br[];
EXTERN char spcspc[];

// -------------------------------------------------------------------- other
EXTERN int          scrollpos;
EXTERN uint cdecl   conRows;
EXTERN uint cdecl   conCols;
EXTERN int cdecl    conMode;
EXTERN PortSpeedE   slv_baud;
EXTERN int          cmd_mdata;

EXTERN long bauds[];
EXTERN long connectbauds[];
EXTERN int  iso_clr[];

#endif

#define pcts    (spcts + 1)
#define justs   (spcts + 2)
#define ns      (spcts + 3)
#define qst     (sqst + 1)
#define spc     (spcspc + 1)

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//                          uninitialized variables
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// ------------------------------------------------------- EXTERNAL.CIT stuff
EXTERN protocols    *extProtList;       // our external protocols
EXTERN doors        *extDoorList;       // our doors
EXTERN censor       *censorList;        // things we censor
EXTERN pairedStrings *replaceList;      // things we replace
EXTERN userapps     *userAppList;       // user apps
EXTERN holidays     *holidayList;       // all the holidays
EXTERN events       *eventList;         // events
EXTERN intEvents    *intEventList;      // Internal evnets
EXTERN archivers    *arcList;           // archivers, dude
EXTERN pairedStrings *netCmdList;       // #NETCOMMANDs, even.

// -------------------------------------------------------- screensaver stuff
EXTERN ScreenSaverC ScreenSaver;

// -------------------------------------------------------------------- FILEs
EXTERN FILE         *TrapFile;          // trap file descriptor
EXTERN FILE         *RoomFile;          // Room file descriptor

// --------------------------------------------------------------- structures
EXTERN char         *borders;           // pointer to borders...
EXTERN AccountInfo  *AccountingData;    // From GRPDATA.CIT


#ifndef WINCIT
// ------------------------------------------------------------- Screen stuff
EXTERN Bool cdecl (*charattr)(char, uchar, Bool);
EXTERN void cdecl (*stringattr)(uint, const char *, uchar, Bool);
#endif

// -------------------------------------------------------------------- other
EXTERN r_slot       *roomPos;           // room position table

EXTERN char         etcpath[64];        // Path ETC.TAB in
EXTERN Bool         sysReq;             // Sysop wants system
EXTERN long         uptimestamp;        // when the bbs was put up
EXTERN Bool         eventkey;           // Alt-E pressed
EXTERN Bool         forceevent;         // Alt-F pressed
EXTERN Bool         repeatevent;        // Alt-R pressed

EXTERN char         fullExePath[80];    // Full path to the .EXE file

EXTERN Bool         debug;              // print debugging messages?
EXTERN Bool         readconfigcit;      // Force read of config.cit?

EXTERN CronC        Cron;               // Cron poop.

EXTERN Bool         roomBuild;          // Rebuild room-file?
EXTERN Bool         hallBuild;          // Rebuild hall-file?

EXTERN Bool         slv_door;           // citadel as a slave?
EXTERN Bool         slv_net;
EXTERN label        slv_node;
EXTERN int          return_code;

EXTERN Bool         login_user;         // user name from command line
EXTERN Bool         login_pw;           // login user using in and pw

EXTERN char         cmd_login[LABELSIZE+LABELSIZE+2];
EXTERN char         cmd_script[80];     // Script to run on startup.

EXTERN int          cmd_nobells;
EXTERN Bool         cmd_nochat;

EXTERN Bool         reconfig;           // Ctdl -C hit for reconfigure
EXTERN Bool         batchmode;          // Don't need -C

EXTERN Bool         sysMail;            // waiting mail for sysop...

EXTERN Bool         CitWindowsVid;      // Have to worry about video?
EXTERN Bool         CitWindowsKbd;      // Have to worry about keyboard?
EXTERN Bool         CitWindowsCsr;      // Have to worry about cursor?

EXTERN int THREAD   hufOrig, hufNew;    // size of compressed stuff
EXTERN uchar THREAD *encbuf;            // huffman stuff

EXTERN long         newmessagek;        // New size values
EXTERN l_slot       newmaxlogtab;       // New size values
EXTERN h_slot       newmaxhalls;        // New size values
EXTERN g_slot       newmaxgroups;       // New size values
EXTERN r_slot       newmaxrooms;        // New size values

EXTERN statRecord   *statList;          // the f4 list
EXTERN int          sl_start;           // where valid data starts...
EXTERN int          sl_end;             // ...and ends
EXTERN Bool         updateStatus;       // need to update f4 list

EXTERN discardable  *msgs;              // general messages
EXTERN const char   **msgsI;            // Index

EXTERN discardable  *dateInfo;          // months, days...
EXTERN const char   **monthTab;         // Jan, Feb, Mar...
EXTERN const char   **fullmnts;         // January, February, March...
EXTERN const char   **days;             // Sun, Mon, Tue...
EXTERN const char   **fulldays;         // Sunday, Monday, Tuesday...
EXTERN const char   *amPtr, *pmPtr;     // AM and PM

EXTERN StatusLineC  StatusLine;         // The status line.

EXTERN uint         s_conRows;

EXTERN ulong        last_console_login_callno;

EXTERN int          oPhys;

EXTERN label        c86_there;
EXTERN char         cmdLine[130];

#ifndef WINCIT
EXTERN char cdecl   *physScreen;        // physical address of screen
EXTERN char cdecl   *dgLogiScreen;      // Copy from ti. This is a Kludge.
EXTERN int cdecl    dgLogiCol;          // Kludge.
EXTERN int cdecl    dgLogiRow;          // Kludge.

EXTERN RLMlist      *RLMmod;            // Initialized RLMs
EXTERN RLMfuncs     *RLMdo;             // Callable RLM functions

EXTERN OSTypeE      OSType;             // What are we running on?
EXTERN int          OSMajor;            // Major version number.
EXTERN int          OSMinor;            // Minor version number.

EXTERN uint cdecl   mouseFlags;
EXTERN uint cdecl   mouseButtons;
EXTERN uint cdecl   mouseX;
EXTERN uint cdecl   mouseY;
EXTERN Bool cdecl   mouseEvent;
#endif

EXTERN label THREAD modem_result;       // the result from dialing out

EXTERN l_slot THREAD SlotForGroupTester;
EXTERN const LogEntry2 THREAD *L2FGT;              // Why can't __thread variables be pointers? I cannot think of any
EXTERN const char THREAD *GNFNT;                   // reason for such silliness.

#define Log2ForGroupTester L2FGT
#define GroupNameForNameTester GNFNT

#ifdef WINCIT
EXTERN SequencerC	ConnSeq;
#endif

#ifndef WINCIT
// ------------------------------------------------------------------ UPS Poo
EXTERN void *upsDriver;
EXTERN label upsDesc;
EXTERN void cdecl (*initups)(void);
EXTERN void cdecl (*deinitups)(void);
EXTERN int cdecl (*statups)(void);

// ------------------------------------------------------------- Keyboard Poo
EXTERN void *kbdDriver;
EXTERN label kdDesc;
EXTERN void cdecl (*initkbd)(void);
EXTERN void cdecl (*deinitkbd)(void);
EXTERN int cdecl (*statcon)(void);
EXTERN int cdecl (*getcon)(void);
EXTERN Bool cdecl (*sp_press)(void);

// ---------------------------------------------------------------- Video Poo
EXTERN void *vidDriver;
EXTERN label vdDesc;

// ---------------------------------------------------------------- Sound Poo
EXTERN void *sndDriver;
EXTERN label sdDesc;
EXTERN int cdecl (*init_sound)(void);
EXTERN int cdecl (*close_sound)(void);
EXTERN int cdecl (*get_version)(void);
EXTERN int cdecl (*query_drivers)(void);
EXTERN int cdecl (*query_status)(void);
EXTERN int cdecl (*start_snd_src)(int which, const void *what);
EXTERN int cdecl (*play_sound)(int which);
EXTERN int cdecl (*stop_sound)(int which);
EXTERN int cdecl (*pause_sound)(int which);
EXTERN int cdecl (*resume_sound)(int which);
EXTERN int cdecl (*read_snd_stat)(int which);
EXTERN int cdecl (*set_midi_map)(int map);
EXTERN int cdecl (*get_src_vol)(int which);
EXTERN int cdecl (*set_src_vol)(int which, int vol);
EXTERN int cdecl (*set_fade_pan)(void *what);
EXTERN int cdecl (*strt_fade_pan)(void);
EXTERN int cdecl (*stop_fade_pan)(int which);
EXTERN int cdecl (*pse_fade_pan)(void);
EXTERN int cdecl (*res_fade_pan)(void);
EXTERN int cdecl (*read_fade_pan)(int which);
EXTERN int cdecl (*get_pan_pos)(int which);
EXTERN int cdecl (*set_pan_pos)(int which, int pos);
EXTERN int cdecl (*say_ascii)(const char *what, int len);
#endif

// wincit specific data
#ifdef WINCIT
EXTERN HINSTANCE hInstance;

EXTERN HWND hCreateDlg;
EXTERN HWND hConfigDlg;

EXTERN HANDLE hConsoleOutput;
EXTERN HANDLE hConsoleInput;

EXTERN HANDLE RoomFileMutex;

EXTERN SMALL_RECT ScreenUpdateRect;
EXTERN HANDLE ScreenUpdateMutex;

EXTERN HWND MainDlgH;
EXTERN Bool ViewingTerm;

EXTERN HANDLE MenuMutex;
EXTERN HANDLE TrapMutex;
EXTERN HANDLE CronMutex;

EXTERN SerialPortListS *SerialPorts;
#endif

#ifdef MULTI
EXTERN LoginListC LoginList;
#endif



// Stuff and things
#ifdef MAIN
char *blbnames[] =
	{
	"BULLETIN",         "CHAT",             "CLOSESYS",
	"ENTRY",            "GOODBYE",          "HELLO",
	"LOGOUT",           "NEWMSG",           "NEWQUEST",
	"NEWROOM",          "NOCHAT",           "NOLOGIN",
	"PASSWORD",         "TEXTUP",           "TOOLOW",
	"USERINFO",         "VERIFIED",         "FILENSRT",
	"WCDOWN",           "WCUP",             "NOROUTE",
	"CHATTED",          "CENSOR",           "RESUME",
	"DISCLAIM",         "AD",               "NOANSWER",
	"TOOMANY",          "MOREPRMP",         "REALNAME",
	"PHONENUM",         "ADDRESS",          "NETPREFX",
	"TIME",             "PROMPT",           "MSGNYM",
	"COLORS",           "ENTERNYM",         "ENCRYPT",
	"WOWCOUNT",         "CALLIMIT",         "WIDTH",
	"LENGTH",           "NULLS",            "NUMUSERS",
	"UNLINK",           "DATESET",          "TIMESET",
	"SIGNATUR",         "SUBJECT",          "LINKAPP",
	"MNAMECHG",         "DIALOUT",          "TITLESUR",
	"NEWQMSG",          "KNOWNGRP",         "YN",
	"YNA",              "YNAC",             "BA",
	"YNK",              "MFU",				"KNWNKYWD",
	"PWRFAIL",			"NEWQFNGR",			"AIDEQA",
	"BANPORT",			"BANNED",

	NULL
	};

char *menunames[] =
	{
	"ENTOPT",			"KNOWN",			"READOPT",
	"GRPGLOB",			"EDIT",				"TERMINATE",
	"SYSNET",			"SYSGROUP", 		"SYSHALL",
	"SYSENTER", 		"AIDE",				"AIDEQUEUE",
	"MAINDOT",			"DOOR",				"CRON",
	"SYSOP",			"MAINOPT",			"VOLKSWAGEN",
	"BULLETINS",		"INVITE",			"FILEQUEUE",
	"HELP",				"PERSONAL", 		"PERSONALADD",
	"FINGER",			"EDITTEXT",			"SYSTABLE",
	"READWC",			"ENTERWC",			"UPLOAD",
	"DOWNLOAD",			"RESPONSEDOWNLOAD",	"WHO",
	"CHAT",

	NULL
	};

label CommandOverrides[UC_NUM];
#else
extern char *blbnames[];
extern char *menunames[];
extern label CommandOverrides[UC_NUM];
#endif
