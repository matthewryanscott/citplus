// --------------------------------------------------------------------------
// Citadel: CTDL.H
//
// #include file for all Citadel CPP files.

#ifdef MINGW
#define VISUALC
#endif

#ifdef VISUALC
#define strncmpi strnicmp

struct time
{
    unsigned char   ti_min;     /* Minutes */
    unsigned char   ti_hour;    /* Hours */
    unsigned char   ti_hund;    /* Hundredths of seconds */
    unsigned char   ti_sec;     /* Seconds */
};

struct date {
  int da_year;     /* current year */
  char da_day;     /* day of the month */
  char da_mon;     /* month (1 = Jan) */
};

#define EXTRA_OPEN_BRACE {
#define EXTRA_CLOSE_BRACE }
#else
#define EXTRA_OPEN_BRACE
#define EXTRA_CLOSE_BRACE
#endif

#ifdef VISUALC
#define FA_RDONLY 0x00000001
#define FA_HIDDEN 0x00000002
#endif

#define ALPHA       1
#define BETA        2
#define PUBLICTEST  3
#define RELEASE     4

#define VERSION PUBLICTEST

// Only for debugging versions: 0 = VerifyHeap() only at critical times
//                              1 = VerifyHeap() always
#define DEBUGLEVEL  0


#define CFGVER      4


#ifdef WINCIT
#define MULTI
#endif

#ifdef MINGW
#define cdecl
#endif

#ifdef MULTI
    #ifdef MINGW
        #define THREAD
        #define _USERENTRY __cdecl
    #else
        #ifdef VISUALC
            #define THREAD __declspec( thread )
            #define _USERENTRY __cdecl
        #else
            #define THREAD __thread
        #endif
    #endif
#else
    #define THREAD
#endif


#if VERSION == ALPHA || (VERSION == PUBLICTEST && (defined(AUXMEM) || defined(WINCIT)))
    // Include assertions
#else
    #define NDEBUG
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

// Don't stack check release code. If it dies, it dies.
// Also, turn off -x (C++ exceptions) and -RT (run-time type identification)
// if using Borland C++ 4.00 or later
#ifdef __BORLANDC__
    #if VERSION == RELEASE
        #pragma option -N-
    #else
        #pragma option -N
    #endif

    #if __BORLANDC__ >= 1106
        #pragma option -x-
        #pragma option -RT-
    #endif
#endif

#ifdef WINCIT
#include <windows.h>
#include <commctrl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <direct.h>
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>
#ifndef WINCIT
#include <bios.h>
#endif
#include <direct.h>
#include <io.h>
#include <share.h>
#include <malloc.h>
#include <process.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <sys\stat.h>
#include <errno.h>

#include "pragmas.h"
#include "enums.h"

#ifdef __BORLANDC__
    // Modify's BC++'s Assert to save some DGROUP...

    #if !defined( __DEFS_H )
    #include <_defs.h>
    #endif

    extern "C" void _Cdecl _FARFUNC __assertfail( char _FAR *__msg, char _FAR *__cond, char _FAR *__file, int __line);

    #undef assert

    #ifdef NDEBUG
        #define assert(p)   ((void)0)
        #define DebugTrap()
    #else
        #ifdef MAIN
            #ifdef WINCIT
                char *AssertFail = "Error:%s %s:%d";
            #else
                char *AssertFail = "Error:%s %s:%d\n";
            #endif
            char *DebugTrap = "Debug: %s:%d";
        #else
            extern char *AssertFail;
            extern char *DebugTrap;
        #endif

        #define assert(p) ((p) ? (void)0 : (void) __assertfail(AssertFail, "", __FILE__, __LINE__))
        #define DebugTrap() trap(T_DEBUG, DebugTrap, __FILE__, __LINE__)
    #endif
#else
    #include <assert.h>
#endif

#undef toupper
#undef tolower
#undef strcmpi
#undef strcmp

// Miscelaneous #defines
#define TRUE            1
#define FALSE           0
#define CERROR          -1
#define ULONG_ERROR     0xFFFFFFFFL
#define UINT_ERROR      0xFFFF

#define SAMESTRING      0       // value for strcmp() & friend
#define SameString(a,b) (strcmpi((a), (b)) == SAMESTRING)

#define SCROLL_SAVE     1       // for cls()
#define SCROLL_NOSAVE   0

#define CTRL_A  (1)
#define CTRL_B  (2)
#define CTRL_D  (4)
#define BELL    (7)
#define LF      (10)
#define CR      (13)
#define ESC     (27)

typedef int (__cdecl *QSORT_CMP_FNP)(const void *, const void *);


// #defines to override standard library functions
#define strcmp(a, b)    deansi_str_cmp(a, b)
#define stricmp(a, b)   deansi_str_cmpi(a, b)
#define strcmpi(a, b)   deansi_str_cmpi(a, b)
#define strftime        strcitftime
#define fopen           CitFopen
#define unlink          CitUnlink
#define rename          CitRename
#define strdup          CitStrdup

#undef memcpy
#undef strcpy
#undef memset

#ifndef NDEBUG

#ifndef WINCIT
#pragma intrinsic memcpy
#pragma intrinsic strcpy
#pragma intrinsic memset
#endif

    // debugging overrides
    inline void *CITmemcpy(void *dest, const void *src, size_t len)
        {
        assert(dest != NULL);
        assert(src != NULL);
        assert(len != 0);

        return (memcpy(dest, src, len));
        }

    inline char *CITstrcpy(char *dest, const char *src)
        {
        assert(dest != NULL);
        assert(src != NULL);

        return (strcpy(dest, src));
        }

    inline void *CITmemset(void *dest, int what, size_t len)
        {
        assert(dest != NULL);
        assert(len != 0);

        return (memset(dest, what, len));
        }

#ifndef WINCIT
#pragma intrinsic -memcpy
#pragma intrinsic -strcpy
#pragma intrinsic -memset
#endif

    #define memcpy(x,y,z)   CITmemcpy(x, y, z)
    #define strcpy(x,y)     CITstrcpy(x, y)
    #define memset(x,y,z)   CITmemset(x, y, z)
#else
    // standard overrides
    #define memcpy(x,y,z)   _fmemcpy(x, y, z)
    #define strcpy(x,y)     _fstrcpy(x, y)
    #define memset(x,y,z)   _fmemset(x, y, z)
#endif

// copy a string into a buffer of a known length
inline void CopyString2Buffer(char *dest, const char *src, size_t Length)
    {
    assert(dest != NULL);
    assert(src != NULL);
    assert(Length);

    strncpy(dest, src, Length - 1);
    dest[Length - 1] = 0;
    }

#define CopyStringToBuffer(dest, src) \
    CopyString2Buffer(dest, src, sizeof(dest))

#ifdef MSC
#define time(a) CitadelTime(a)
#endif

// The following are easier to type and make it easier to format code.
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;
typedef unsigned char   uchar;
typedef unsigned int    Bool;
typedef unsigned short  BoolS;
typedef void *          CITHANDLE;

typedef uint bit;
typedef ushort word;
typedef uchar byte;

#ifdef AUXMEM
    #define AUXMEMCODE(x)   x
#else
    #define AUXMEMCODE(x)
#endif

#ifdef WINCIT
    #define WINCODE(x)  x
#else
    #define WINCODE(x)
#endif

#if defined(AUXMEM) || defined(WINCIT)
    #define AUXMEMWINCODE(x)    x
#else
    #define AUXMEMWINCODE(x)
#endif

#ifdef NDEBUG
    #define DEBUGCODE(x)
#else
    #define DEBUGCODE(x)    x
#endif


// for citOpen stuff
enum CitOpenCodes
    {
    CO_A = 1,   CO_AB,  CO_AP,  CO_R,   CO_W,   CO_WB,
    CO_RB,      CO_RPB, CO_WPB,
    };


// These define the sizes of some Citadel things.
typedef short   r_slot;             // what a room is
typedef short   h_slot;             // what a hall is
typedef short   l_slot;             // what a log table entry is
typedef short   l_index;            // what a log file entry is
typedef short   g_slot;             // what a group is

typedef long    m_index;
#define M_INDEX_ERROR -1

#if defined(AUXMEM) || defined(WINCIT)
    typedef long    m_slot;
#else
    typedef short   m_slot;
#endif
#define M_SLOT_ERROR    -1

#define LABELSIZE       30          // length of room names
#define BORDERSIZE      80          // length of borders.
#define PATHSIZE        256         // length of msg paths
#define MAXTEXT         8192        // cheating (message buffer size)
#define MAXDIRS         6           // number of directions
#define MAXWORDLEN      256         // maximum length of a word
#define DOSPATHSIZE     63          // wow.
#define ANSILEN         300         // Max length of ANSI/Music string
#define BOOL_COMPLEX    25          // how complex we can make them
#define KEYBUFSIZE      101         // How big our keyboard buffer is + 1.

typedef char            label[LABELSIZE+1];
typedef char            fpath[DOSPATHSIZE+1];
typedef ushort          BoolExpr[BOOL_COMPLEX + 1];


enum PortSpeedE
    {
    PS_300,         PS_600,         PS_1200,        PS_2400,
    PS_4800,        PS_9600,        PS_19200,       PS_38400,
    PS_57600,       PS_115200,

    PS_NUM,         PS_ERROR
    };

enum ModemSpeedE
    {
    MS_300,         MS_600,         MS_1200,        MS_2400,
    MS_4800,        MS_7200,        MS_9600,        MS_12000,
    MS_14400,       MS_16800,       MS_19200,       MS_21600,
    MS_24000,       MS_26400,       MS_28800,       MS_38400,
    MS_57600,       MS_115200,      MS_230400,

    MS_NUM,         MS_ERROR
    };

// it's amazing how often you need this. well, perhaps not "amazing."
#define SECSINDAY   86400l

// This next part defines the structures of the data files and tables
// used to index them.

/* -------------------------------------------------------------------- */
/*                                                                      */
/*              !   !  !!!   !!!!!  !!!!!   !!                          */
/*              !!  ! !   !    !    !       !!                          */
/*              ! ! ! !   !    !    !!!     !!                          */
/*              !  !! !   !    !    !                                   */
/*              !   !  !!!     !    !!!!!   !!                          */
/*                                                                      */
/* Citadel uses readTables() and writeTables() to write images          */
/* of the external variables in RAM to disk, and later restore          */
/* them.  The images are stored in Etc.tab and all the .TAB files.      */
/* Ctdl.Exe will automatically reconstruct these files the hard way     */
/* when necessary and write them out when finished.                     */
/* Etc.tab and the .TAB files are always destroyed after reading, to    */
/* minimize the possibility of reading out-of-date versions.  In        */
/* general, this technique works well and saves time and head-banging   */
/* upon bootup.  You should, however, note carefully the following      */
/* caution:                                                             */
/*  o  Whenever you change the declarations of any tables you should:   */
/*   -->  Destroy the current Etc.tab file.                             */
/*   -->  Recompile and reload Ctdl.Exe.                                */
/*   -->  Ctdl.Exe will automatically build new Etc.tab and tables.     */
/*                                                                      */
/* If you ignore this warning, little pixies will prick you in your     */
/* sleep for the rest of your life.                                     */
/* -------------------------------------------------------------------- */

// This first structure is the is the contents of ETC.TAB.
// It contains configuration information.
struct config
    {
    long    version;

    ulong   callno;             // how many calls the system has gotten
    m_index NextBuildAddress;   // for buildaddress()

// -------------- System name, region, and other net stuff ------------
    label   nodeTitle;          // name of the system
    label   nodeRegion;         // name of the system's region
    label   nodeCountry;        // name of the system's country
    label   Cit86Country;       // For Citadel-86 style netting
    label   Cit86Domain;        // For Citadel-86 style netting
    label   softverb;           // this is really soft adjetive.
    char    nodeSignature[91];  // Signature line for the node
    char    PAD;
    long    sig_first_pos;      // Position of first signature (Multi)
    long    sig_current_pos;    // Current position of signature (Multi)
    char    alias[4];           // alias, for routing
    char    locID[5];           // location identifier, for routing
    char    Address[9];         // alias.region together
    label   nodephone;          // phone #, in "(206) 555-1212" format

    label   twitRegion;         // how many stupid fields did we get
    label   twitCountry;        // from jj and zen master?

    label   sysop;              // name of the system operator

    Bool    forward;            // forward sysop&aide messages to sysop?
    Bool    showSysop;          // show .AL stuff

// ------------------------- Modem Stuff ------------------------------
    int     mdata;              // DOS: 1, 2, 3, or 4: COM port to use
                                // Win32: Bitmapped; COM ports to use
                                // (LSB: Com1 - MSB: Com32)
    int     dumbmodem;          // See CONFIG.CIT
    PortSpeedE initbaud;        // Baud to start modem at
    ModemSpeedE minbaud;        // minimum allowed baud
    char    modsetup[64];       // Normal init string
    char    modunsetup[64];     // Normal de-init string
    label   downshift;          // downshift string
    char    dialsetup[64];      // Dial out string
    label   dialpref;           // ATDT
    label   dialring;           // What to dial when RING received
    int     dialringwait;       // Time 'tween last ring and dialring
    char    updays[7];          // Which Days board is answering?
    char    uphours[24];        // Which hours board is answering?
    int     hangupdelay;        // For Ogre's brain-dead modem.
    label   hangup;             // how to hang up with #dumbmodem 6
    label   offhookstr;         // how to offhook()
    int     baudPause;          // For Emily's brain-dead modem.
#ifdef WINCIT
    int     MaxTelnet;          // Number we allow
    int     TelnetPort;         // What port we listen on
    int     Net6969Port;        // What port we listen on
#endif

// ---------------------------- Paths ---------------------------------
    fpath   homepath;           // which path system files will be in
    fpath   msgpath;            // which path message file will be in
    fpath   helppath;           // path which help files live in
    fpath   helppath2;          // 2nd path which help files live in
    fpath   temppath;           // temporary files (networking) go here
    fpath   roompath;           // where to look for room descriptions
    fpath   trapfile;           // where trap file goes
    fpath   aplpath;            // which path applications will be in
    fpath   dirpath;            // default path for directory rooms
    fpath   transpath;          // path transient files will be in
    fpath   printer;            // where to put output at alt-p
    fpath   dlpath;             // where to put network file transfers
    fpath   logextdir;          // where to put log extension files
    fpath   lexpath;            // where to find dictionaries
    fpath   rlmpath;            // where to find RLMs
    fpath   ScriptPath;         // Where to find CSFs

// ----------------------- Default Terminal ---------------------------
    int     autoansi;           // how to autoansi detect...

// ------------------------- New User ---------------------------------
    label   enter_name;         // enter "your name" or "nym" or "..."
    label   newuserapp;         // application to run for new users
    int     oldcount;           // how many message new to new users
    Bool    l_closedsys;        // System is closed to new users
    Bool    l_verified;         // Leave users verified
    Bool    l_sysop_msg;        // Message to Sysop
    Bool    l_create;           // create accounts

    NewUserQuestions NUQuest[NUQ_NUMQUEST+1];   // The questions to ask
    label   newuserLogFileName; // And where to save the answers

// ------------------------- Accounting -------------------------------
    int     unlogtimeout;       // how many idle mins unlogged user gets
    int     timeout;            // how many idle mins logged user gets
    int     consoletimeout;     // how many idle mins console user gets
    int     OutputTimeout;      // how many idle mins user gets when Citadel
                                // is displaying information
    long    unlogbal;           // # credits unlogged users get
    int     pwDays;             // Days users are allowed to keep in;pw
    int     callLimit;          // Number of calls allowed per day

// ------------------------- Appearance -------------------------------
    char    datestamp[64];      // default time date format string
    char    vdatestamp[64];     // default time date format string
    char    prompt[64];         // default prompt format string
    label   Umsg_nym;           // Upper-case name of message
    label   Umsgs_nym;          // Upper-case name of messages
    label   Uroom_nym;          // Upper-case name of room
    label   Urooms_nym;         // Upper-case name of rooms
    label   Uhall_nym;          // Upper-case name of hall
    label   Uhalls_nym;         // Upper-case name of halls
    label   Uuser_nym;          // Upper-case name of user
    label   Uusers_nym;         // Upper-case name of users
    label   Ugroup_nym;         // Upper-case name of group
    label   Ugroups_nym;        // Upper-case name of groups
    label   Ucredit_nym;        // Upper-case name of credit
    label   Ucredits_nym;       // Upper-case name of credits
    label   Lmsg_nym;           // Lower-case name of message
    label   Lmsgs_nym;          // Lower-case name of messages
    label   Lroom_nym;          // Lower-case name of room
    label   Lrooms_nym;         // Lower-case name of rooms
    label   Lhall_nym;          // Lower-case name of hall
    label   Lhalls_nym;         // Lower-case name of halls
    label   Luser_nym;          // Lower-case name of user
    label   Lusers_nym;         // Lower-case name of users
    label   Lgroup_nym;         // Lower-case name of group
    label   Lgroups_nym;        // Lower-case name of groups
    label   Lcredit_nym;        // Lower-case name of credit
    label   Lcredits_nym;       // Lower-case name of credits
    label   msg_done;           // what to do to message
    label   netPrefix;          // default net prefix
    int     nopwecho;           // what to echo initials & pw at login
    label   anonauthor;         // author of anonymous messages
    label   twirly;             // twirly string
    int     twirlypause;        // twirly speed
    Bool    twitrev;            // Twitty implementation of Reverse?
    char    AltF3Msg[81];       // What to say, dude!
    char    AltF3Timeout[81];   // What to say, dude!

// ------------------------- System Size ------------------------------
    long    MsgDatSizeInK;      // how many K message table is
    int     maxfiles;           // Max number of files per directory
    l_slot  MAXLOGTAB;          // number of log entries supported
    r_slot  maxrooms;           // number of rooms
    h_slot  maxhalls;           // number of halls
    g_slot  maxgroups;          // number of groups
    int     maxborders;         // number of borders
    m_slot  nmessages;          // # of messages to be stored in table

// -------------------------- Features --------------------------------
    uint    borderfreq;         // frequency of borders
    Bool    msgNym;             // aides can change Message nyms
    Bool    borders;            // Borderlines enabled
    Bool    forcelogin;         // Automatically log someone in?
    int     offhook;            // TRUE to go off-hook when 'L' is hit
    Bool    accounting;         // is accounting enabled on the system?
    Bool    netmail;            // put networked mail in proper rooms?
    int     subhubs;            // provision for special g)oto looping
    Bool    colors;             // allow colored room names, etc.
    char    moreprompt[80];     // configurable <more> prompt
    char    sleepprompt[80];    // configurable sleeping... prompt
    Bool    showmoved;          // show original room if not verbose
    Bool    censor;             // Let users be able to view censored?
    Bool    countbeep;          // beep when counting down
    int     sleepcount;         // countdown timer for timout
    char    dialmacro[48][80];  // dialout macros - 48 should be
                                //  DM_NUMKEYS, but it cannot...
    long    diskfree;           // nothing, really
    Bool    aideChatHours[24];  // when .AC works
    int     chatmail;           // Leave msg to sysop when chatting?
    Bool    chatwhy;            // Reason for chatting.
    int     chatflash;          // Flash border while ringing sysop.
    int     ringlimit;          // #chatloop
    Bool    ecTwirly;           // if .EC can set twirly cursor
    Bool    ecUserlog;          // if .EC can set list in userlog
    Bool    ecSignature;        // if .EC can set user signature
    char    dictionary[13];     // the default dictionary in use
    Bool    ovrEms;             // use EMS for overlays
    Bool    ovrExt;             // use extended for overlays
    int     checkSysMail;       // 0; no; 1 'sysop'; 2 #sysop; 3 both
    unsigned long memfree;      // supershell if less
    Bool    music;              // esc[MZ/F sounds?
    int     allowCrypt;         // 0: None; 1: Mail; 2: Everywhere
    int     badPwPause;         // how long
    int     altF3Time;          // How much time to give (Seconds).
    Bool    ignore_uptime;      // if Alt-U has been hit
    Bool    SaveJB;             // Save #JB in LE*.DAT?
    Bool    SwapNote;           // Note supershell to user.
    Bool    FastScripts;        // Keep stuff loaded.
    Bool    NewNodeAlert;       // Alert in when added to ROUTE.CIT?
    Bool    Psycho;             // Psycho filter enabled
    Bool    Backout;            // Backout filter enabled
    Bool    Mmmm;               // Mmmm filter enabled
    Bool    VerboseConsole;     // Tell console user what he did?
    Bool    AltXPrompt;         // Ask Yes/No?
    char    laughter[90];       // configurable 'new message somewhere' text

// ------------------------- Privileges -------------------------------
    Bool    entersur;           // Let users enter their own surname?
    Bool    aidehall;           // Aides mess with halls.
    Bool    readluser;          // Read Limited Access Userlog?
    Bool    unlogEnterOk;       // TRUE if OK to enter messages anon
    Bool    unlogReadOk;        // TRUE if unlogged folks can read mess
    Bool    nonAideRoomOk;      // TRUE general folks can make rooms
    Bool    moderate;           // can aides see moderated messages?
    Bool    sysopOk;            // can make sysops from remote?
    Bool    SuperSysop;         // Enable this feature?
    uint    numRooms;           // how many rooms can be made per call?
    int     readLog;            // Who can .RU?
    Bool    PersonalHallOK;     // Can users use it?

    // TITLES / SURNAMES
    Bool    surnames;           // are the surenames on?
    Bool    netsurname;         // Display networked surnames?
    Bool    titles;             // Titles are on
    Bool    nettitles;          // Titles enabled

    // CONSOLE
    Bool    bios;               // to use BIOS calls for screen I/O?
    uchar   attr;               // color of text displayed on screen
    uchar   wattr;              // color of text displayed on window
    uchar   cattr;              // color of text displayed on window
    uchar   battr;              // color of text screen border lines
    uchar   uttr;               // color of text displayed in underline
    int     screensave;         // # minutes before blanking screen
    Bool    fucking_stupid;     // whether or not to unblank the screen upon carrier detect
    Bool really_fucking_stupid; // whether keep the screen unblanked for the duration of the call
    Bool really_really_fucking_stupid;  // whether or not to keep the cursor on while the screen is blanked
    Bool    LockOnBlank;        // Lock console when saver turned on?
    int     turboClock;         // how often to move the turboClock
    label   SaverMsg;           // Replace turboClock?
    Bool    NoConsoleTrap;      // Don't trap stuff onConsole
#ifdef WINCIT
    int     ScreenUpdatePause;  // Milliseconds to wait
    int     BlinkPause;         //
    SHORT   ConsoleRows;        // Size of console
    SHORT   ConsoleCols;        //
#endif

    // OTHER STUFF
    int     connectwait;        // how long to wait after we connect
    int     idle;               // how long we wait before we net

    int     ad_chance;          // chance 0-100% of an ad appearing
    uint    ad_time;            // time before random ad appears

    int     ansiBye;            // Goodbye blbs.
    int     normBye;            // Goodbye blbs.
    int     ansiHello;
    int     normHello;

    int     ansiAd;
    int     normAd;

    int     ansiCometoChat;
    int     normCometoChat;

    int     ansiJoke;
    int     normJoke;

    label   f6pass;             // Console lock password
    Bool    FullConLock;        // If the console is fully locked
    Bool    trapit[T_NUMTRAP];  // which events are logged
    int     MessageRoom;        // max messages per room per call
    Bool    noChat;             // TRUE to suppress chat attempts
    int     noBells;            // 2 to supress all bells, 1 all but chat
    Bool    mci;                // fuck this shit
    label   mci_name;           // if off, what to show...
    label   mci_firstname;
    label   mci_time;
    label   mci_date;
    label   mci_poop;

#define MAXCHT 30
    int     LocalChat[MAXCHT];  // codes for local chat sound
    int     LocalChatLen;       // How many are in use.

#ifdef AUXMEM
    char    vmemfile[64];       // where to make the virtual memory
#endif
    Bool    checkCTS;           // pay attention to CTS from the modem?

    int     poop;
    Bool    poopdebug;
    Bool    poopwow;
    long    maxpoop;
    label   poopuser;

    Bool    printerprompt;      // prompt for file when doing alt-p?

    long    scrollSize;
    Bool    scrollColors;
    uint    scrollTimeOut;
    Bool    msgCompress;
    Bool    restore_mode;
    int     expire;
    int     maxjumpback;
    uchar   fuelbarempty;
    uchar   fuelbarfull;
    int     statnum;
    int     wysiwyg;            // richard's silly little screen stuff
    int     concolors;          // console color stuff
    Bool    forcetermcap;       // console termcap always on?

    Bool    speechOn;           // getting geeky with the sound blaster
    int     maxerror;           // maximum # of errors before hanging up

    int     ReadStatusSecurity; // Who can see all debugging information
	int     chattype;           // Preset chat string select.  0 == Jason, 1 == Brent's original
    };

EXTERN config       cfg;        // A buncha variables

// so we can have a pointer to them in taskInfo. see various .H files.
class LogEntry;
class LogEntry2;
class Message;

class AccountInfo;              // Stuff read from GRPDATA.CIT
class UserAccountInfo;          // Stuff for logged in user
class NodesCitC;              // Stuff read from NODES.CIT

class TalleyBufferC;            // Counts messages per room by logged in user (plus some other stuff...)

enum MonthsE
    {
    M_JAN,  M_FEB,  M_MAR,  M_APR,  M_MAY,  M_JUN,
    M_JUL,  M_AUG,  M_SEP,  M_OCT,  M_NOV,  M_DEC,

    NUM_MONTHS
    };

enum DaysE
    {
    D_SUN,  D_MON,  D_TUE,  D_WED,  D_THU,  D_FRI,  D_SAT,

    NUM_DAYS
    };


// --------------------------------------------------------------------------
// Citadel only has Xmodem (and 1K Xmodem and Xmodem CRC) built into
// it. For any serious file transfering (including those used for
// networking), you must tell it to use an external file transfer
// protocol. These are defined in EXTERNAL.CIT, and are read into the
// following structure, which forms a linked list.

struct protocols
    {
    protocols *next;
    label name;                 // The name of the protocol.
    label MenuName;
    char CommandKey;
    label autoDown;             // An auto-download initiation string.
    char respDown[128];         // Command send with response file.
    char rcv[128];              // Command line to receive files.
    char snd[128];              // Command line to send files.
    int block;                  // Size of block. (0 for variable.)
    Bool batch;                 // If it is a batch protocol.
    Bool NetOnly;               // Only for networking.
    int CheckType;              // Woo.
    char CheckMethod[128];      // Woo woo.
    };



// --------------------------------------------------------------------------
// Citadel can be told to run any specific application when any
// specific user logs in. These "user applications" are defined in
// EXTERNAL.CIT and read into this structure, which forms a linked
// list.

struct userapps
    {
    userapps *next;
    label name;                 // The name of the user.
    char outstr[64];            // Anything to display to the user.
    label cmd;                  // The command to run.
    Bool hangup;                // Hang up when done?
    };


// --------------------------------------------------------------------------
// Citadel can be set up to use doors, which can also override the
// internal commands. Doors are defined in EXTERNAL.CIT and read into
// the following structure, which forms a linked list.

struct doors
    {
    doors *next;
    label name;                 // The name of the door.
    char cmd[41];               // The command to run.
    label group;                // Limited to a specific group...
    Bool DIR    : 1;            // Only in directory rooms.
    Bool SYSOP  : 1;            // Must be a Sysop to use it.
    Bool AIDE   : 1;            // Must be an Aide to use it.
    Bool CON    : 1;            // Must be on console to use it.
    int WHERE   : 2;            // 0=.ED_; 1=_,.ED_; 2=._,.ED_
    };


// --------------------------------------------------------------------------
// Citadel can replace its standard date string with a holiday if the
// holiday has been specified. Specify holidays in EXTERNAL.CIT, and
// they get read into this structure, which forms a linked list.

struct holidays
    {
    holidays *next;
    label name;                 // The name of the holiday.
    MonthsE Month;              // The month of the holiday.
    int date;                   // The date of the holiday.
    int week;                   // The week of the month of the holiday.
    int year;                   // The year of the holiday. (-1 for all)
    DaysE day;                  // The day of the week of the holiday.
    };


// --------------------------------------------------------------------------
// You can decide to censor messages based their content, author, or
// (in the case of networked messages), their originating node. This
// is defined in EXTERNAL.CIT, and the data is read into the following
// structure, which forms a linked list.

enum CensorType
    {
    CENSOR_TEXT,    CENSOR_AUTHOR,  CENSOR_NODE,
    };

struct censor
    {
    censor *next;
    CensorType what;            // What type of thing we are censoring.
    label str;                  // And what exactly is it.
    };


// --------------------------------------------------------------------------
// Citadel allows Aides to create lists of messages, to operate on all
// at once. (Such as to kill them all at once, or insert all of them
// into another room.) This list is implemented with the following
// linked list. (Should it be done with an array? Perhaps. But it is
// not.)

struct messageList
    {
    messageList *next;
    long msg;                   // ID of the message in the list.
    };


// --------------------------------------------------------------------------
// Citadel keeps a stack of rooms that a user has been in. This is to
// allow them to "jump back" to rooms that they have left. This stack
// is implemented as an array of the following data.

struct jumpback
    {
    m_index newpointer;         // Which messages were new in the room.
    m_slot  newMsgs;            // How many messages were new.
    h_slot  hall;               // What hall was the user in.
    r_slot  room;               // Which room, too.
    Bool    bypass;             // Had the room been bypassed.
    };





// --------------------------------------------------------------------------
//  Sometimes, it's useful to store a couple of strings in a pair.

struct pairedStrings
    {
    pairedStrings *next;
    label string1;
    label string2;
    };


// --------------------------------------------------------------------------
// Citadel often keeps a list of strings in memory. (For example, the
// personal dictionary feature of the spell checker is implemented
// as one.) The following structure is used to define the list. Note
// that there is only space in the structure for one character of the
// string. This is to make efficient use of memory: we always know how
// long the string is going to be before we add it to the list. We use
// this knowledge to allocate just enough memory to contain the string
// and no more. The space to allocate for the string is given by the
// formula
//
//                  sizeof(strList) + strlen(str)
//
// Where str is the string that we want to add to the list. strlen()
// returns the number of bytes in the string, not including the space
// required for the terminal Nul character. This is already in the
// structure, however, so we get just how much memory we want. It is
// then merely a matter of strcpy()ing the string to the new space.

struct strList
    {
    strList *next;
    char string[1];             // The string being stored.
    };


struct SystemEventS
    {
    SystemEventS    *next;
    Bool            HideFromConsole;
    char            string[1];
    };


// --------------------------------------------------------------------------
// For the F4 screen, and some internal processing, Citadel uses a
// global variable telling it what is going on. This variable can be
// assigned one of the following activities.
enum dowhattype
    {
    DUNO,                   MAINMENU,               SYSOPMENU,
    PROMPT,                 MESSAGETEXT,            DIALOUT,
    NETWORKING,             ENTERBORDER,            READMESSAGE,
    HELPFILES,              READUSERLOG,            READSTATUS,
    READCONFIG,             READINTRO,              DODOWNLOAD,
    DOUPLOAD,               DOCHAT,                 DOENTER,
    DOVOLKS,                DOEXCLUDE,              DOGOTO,
    DOKNOWN,                DOLOGOUT,               DOREAD,
    DOSMALLCHAT,            DOAIDE,                 ENTERCONFIG,
    ENTERDOOR,              ENTERHALL,              ENTERPW,
    ENTERROOM,              ENTERTEXT,              ENTERWC,
    ENTERAPP,               ENTERNAME,              ENTERSUR,
    ENTERMENU,              DOBYPASS,               KNOWNROOMS,
    KNOWNHALLS,             KNOWNRMINFO,            KNOWNMENU,
    TERMMENU,               TERMQUIT,               TERMSTAY,
    READDIR,                READINFO,               READHALLS,
    READTEXT,               READWC,                 READZIP,
    READMENU,               AIDEATTR,               AIDECHAT,
    EDITROOM,               AIDEFILESET,            AIDEGROUP,
    AIDEHALL,               AIDEINSERT,             AIDEKILL,
    AIDELIST,               AIDEMOVE,               AIDENYM,
    AIDEQUEUE,              AIDEQUEUEM,             AIDEQUEUEC,
    AIDEQUEUEI,             AIDEQUEUEK,             AIDEQUEUEL,
    AIDEQUEUER,             AIDEQUEUES,             AIDERENAME,
    AIDESETINFO,            AIDEUNLINK,             AIDEVIEW,
    AIDEWINDOW,             MOVEROOM,               AIDEMSGIN,
    AIDEMENU,               SYSMAINT,               SYSCRON,
    SYSOFFHK,               SYSDATE,                SYSENTER,
    SYSGRP,                 SYSHALL,                SYSINFO,
    SYSJOURNAL,             SYSKUSER,               SYSMASS,
    SYSNUSER,               SYSSCR,                 SYSSHOW,
    SYSEDIT,                SYSVIEW,                SYSEXIT,
    SYSKEMPTY,              SYSSHELL,               SYSNET69,
    SYSAUXDEB,              SYSREADMSG,
    SYSCFG,                 SYSMENU,                S69FETCH,
    S69INC,                 S69ROOMREQ,             S69MENU,
    SYSFPDOWN,              SECURITYVIOLATION,      LOGIN,
    SCROLLBACK,             CONSYSOP,               FILEQUEUEL,
    FILEQUEUER,             FILEQUEUEC,             FILEQUEUEA,
    DOGOTOMAIL,             DOPERSONAL,             PERSONALMENU,
    PERSONALADD,            PERSONALRMV,            PERSONALLST,
    PERSONALUSE,            PERSONALADDALL,         PERSONALADDLOCAL,
    PERSONALADDROOM,        PERSONALADDMENU,        PERSONALCLEAR,
    S86FETCH,               S86INC,                 FINGER,
    FINGERVIEW,             FINGEREDIT,             FINGERMENU,
    SYSTAB,                 FINGERLIST,             DOWHO,
    WHOMENU,                CHATMENU,
    };


// TERMCAP definitions
enum LogAttr
    {
    ATTR_NORMAL,            ATTR_BLINK,             ATTR_REVERSE,
    ATTR_BOLD,              ATTR_UNDERLINE,

    ATTR_NUM,
    };

enum SystemEventE
    {
    SE_LOGONOFF,            SE_NEWMESSAGE,          SE_EXCLUSIVEMESSAGE,
    SE_CHATALL,             SE_CHATUSER,            SE_CHATGROUP,
    SE_CHATROOM,            SE_ROOMINOUT,           SE_TEST,
      SE_NEWMESSAGE_SOMEWHERE,
    SE_NUM,
    };


// list stuff
#define     LIST_START  NULL
#define     LIST_END    ((char *) 1)


// modem stuff
enum ModemConsoleE
    {
    MODEM,      CONSOLE,    NOMODCON,
    };

enum EchoType
    {
    NEITHER,                // don't echo input at all
    CALLER,                 // echo to caller only -- passwords, etc.
    BOTH,                   // echo to caller and console both
    };


// These flags are for special action during message retrvial
enum SpecialMessageOperations
    {
    NO_SPECIAL,             // do not do anything special
    PULL_IT,                // kill the message
    MARK_IT,                // mark the message to be moved
    REVERSE_READ,           // change read direction
    COPY_IT,                // copy message to file (UNUSED)
    CENSOR_IT,              // censor toggle
    REPLY,                  // Reply to it.
    };


// For reading by various criteria...
struct ReadFilter
    {
    Bool    Mail;
    Bool    Group;
    Bool    Public;
    Bool    Local;
    Bool    User;
    Bool    Text;

    label   SearchUser;
    label   SearchText;
    BoolExpr    SearchGroup;
    };


// The read options
struct MsgReadOptions
    {
    Bool    Number;         // .R#...
    Bool    Headerscan;     // .R!...
    Bool    Date;           // .R&...
    Bool    DatePrevious;   // .R&...
    Bool    All;            // .RA...
    Bool    Verbose;        // .RV...
    Bool    Reverse;        // .RR...
    Bool    PauseBetween;   // .RJ...

    time_t  CheckDate;

    long    MessageNumber;

    SpecialMessageOperations DotoMessage;

    void Clear(Bool SetVerbose);
    };


// values for showMess and readdirectory/infofile/filldirectory routines
enum OldNewPick
    {
    NewOnly,        OldAndNew,      OldOnly,        WhoKnows,
    };

// values for filldirectory
enum SortE
    {
    SORT_NONE,      SORT_ALPHA,     SORT_DATE,
    };

#ifdef MULTI
    class TermWindowC;

    #define tw()        TW->
    #define WC_TW       TermWindowC *TW
    #define WC_TWp      this
    #define WC_TWpn     NULL
    #define CfgDlg      if (TW) TW->mPrintfCR

    #define TWcPrintf   if (TW) TW->cPrintf
    #define TWdoccr     if (TW) TW->doccr
    #define TWwDoCR     if (TW) TW->wDoCR
    #define TWwPrintf   if (TW) TW->wPrintf

    #define CitThis     this
#else
    #define tw()
    #define WC_TW   void
    #define WC_TWp
    #define WC_TWpn
    #define TW      (1)

    #define TWcPrintf   cPrintf
    #define TWdoccr     doccr
    #define TWwDoCR     wDoCR
    #define TWwPrintf   wPrintf

    #define CitThis     NULL
#endif


enum UserControlType
    {
    OUTOK,                  // normal output
    OUTNEXT,                // quit this message, get the next
    OUTSKIP,                // stop current process
    OUTPARAGRAPH,           // skip to next paragraph
    IMPERVIOUS,             // make current output unstoppable
    NOSTOP,                 // can pause, but not stop
    };

class UserOutputControl
    {
public:
    Bool CanK, CanM, CanStar;   // K, M, * allowed
    Bool CanR, CanBang, CanAt;  // R, !, @ allowed

    Bool CanControlD;           // Control+D allowed

    Bool Continuous, ControlD;  // User hit C or Control+D

    UserControlType OutFlag;    // OUTOK, IMPERVIOUS, etc.

    void Reset(void);

    void ResetCanFlags(void)
        {
        CanK = CanM = CanStar = CanBang = CanAt = CanR = FALSE;
        }

    Bool CanOutput(void) const
        {
        return (OutFlag == OUTOK || OutFlag == IMPERVIOUS || OutFlag == NOSTOP);
        }

    void SetUserOutFlag(UserControlType New)
        {
        OutFlag = New;
        }

    UserControlType GetOutFlag(void) const
        {
        return (OutFlag);
        }

    void ResetUserOutParagraph(void)
        {
        if (GetOutFlag() == OUTPARAGRAPH)
            {
            SetUserOutFlag(OUTOK);
            }
        }

    void SetContinuous(Bool New)
        {
        Continuous = !!New;
        }

    void SetCanK(Bool New)
        {
        CanK = !!New;
        }

    void SetCanM(Bool New)
        {
        CanM = !!New;
        }

    void SetCanStar(Bool New)
        {
        CanStar = !!New;
        }

    void SetCanR(Bool New)
        {
        CanR = !!New;
        }

    void SetCanBang(Bool New)
        {
        CanBang = !!New;
        }

    void SetCanControlD(Bool New)
        {
        CanControlD = !!New;

        if (!CanControlD)
            {
            ControlD = FALSE;
            }
        }

    void SetCanAt(Bool New)
        {
        CanAt = !!New;
        }
    };


struct OutputControl
    {
    Bool        NoReplace;      // don't replace this stuff...
    Bool        Formatting;     // TRUE or FALSE
    EchoType    Echo;           // NEITHER, CALLER, or BOTH
    int         EchoChar;       // What to echo on NEITHER
    int         HangingIndent;  // Hanging indent. Duh.
    Bool        Modem;
    Bool        Console;
    Bool        inANSI;         // Processing an ANSI code...
    Bool        UseMCI;         // Is MCI Active?
    Bool        MCI_goto;       // if we are in middle of one
    label       MCI_label;      // where we are gotoing
    Bool        Psycho;         // Psycho chicken?
    Bool        Printing;       // printing now?
    Bool        WasPrinting;    // printing before logged in?
    FILE        *PrintFile;     // printer.out
    fpath       PrintfileName;  // name of printer file
    int         CrtColumn;      // current position on screen
    uchar       ansiattr;
    Bool        justdidpause;
    char        prtf_buff[512]; // for printing output

    ModemConsoleE whichIO;      // Who is using the system?

    UserOutputControl User;     // What the user can do

#ifdef WINCIT
    TermWindowC *term;
#endif

    void setio(void);

    void SetOutFlag(UserControlType New)
        {
        User.SetUserOutFlag(New);
        setio();
        }

    void ResetOutParagraph(void)
        {
        if (User.GetOutFlag() == OUTPARAGRAPH)
            {
            SetOutFlag(OUTOK);
            }
        }

#ifdef MULTI
    Bool CheckInput(Bool Pause, TermWindowC *TW);
#else
    Bool CheckInput(Bool Pause);
#endif
    };

enum AutoMsgE
    {
    AM_NONE,    AM_MARK,    AM_KILL,    AM_CENSOR,

    AM_ERROR
    };

struct MsgStatus
    {
    Message     *AbortedMessage;    // saved message buffer
    int         Read;               // #messages read
    int         Entered;            // #messages entered
    int         Expired;            // #messages expired (network)
    int         Duplicate;          // #messages duplicate (network)
    m_index     MarkedID;           // ID of marked message
    messageList *MsgList;           // message queue pointer
    AutoMsgE    AutoMKC;            // automatically mark/kill/censor message?
    };


// Rooms
#define LOBBY           0       // Lobby>   is >always< room 0.
#define MAILROOM        1       // Mail>    is >always< room 1.
#define AIDEROOM        2       // Aide)    is >always< room 2.
#define DUMP            3       // Dump>    is >always< room 3.


// Groups
#define EVERYBODY       0       // The Everybody group's index
#define SPECIALSECURITY 1       // The Special Security group's index


// Halls
#define MAINHALL        0       // The Main hall's index
#define MAINTENANCE     1       // The Maintenance hall's index


// known stuff
#define K_APPLIC    0x00000001l
#define K_ANON      0x00000002l
#define K_PERM      0x00000004l
#define K_DIR       0x00000008l
#define K_GROUP     0x00000010l
#define K_LOCAL     0x00000020l
#define K_NEW       0x00000040l
#define K_OLD       0x00000080l
#define K_MAIL      0x00000100l
#define K_SHARED    0x00000200l
#define K_THALL     0x00000400l
#define K_WINDOWS   0x00000800l
#define K_EXCLUDE   0x00001000l
#define K_NUM       0x00002000l
#define K_NOMSGS    0x00004000l
#define K_BIO       0x00008000l
#define K_HIDDEN    0x00010000l
#define K_KEYWORD   0x00020000l


// for F4 status screen
struct statRecord
    {
    char theStatus[61];
    char PAD;
    time_t theTime;
    };


// file download stuff
struct fileQueue
    {
    fileQueue *next;
    long size;
    label room;
    char fname[1];
    };


struct archivers
    {
    archivers *next;
    label name;
    char view[64];
    char extract[64];
    char ext[4];
    };


struct discardable
    {
    discardable *next;
    discardableType type;
    long length;
    void *data;
    void *aux;
    };


struct datestruct
    {
    int Year;
    MonthsE Month;
    int Date;
    };


struct timestruct
    {
    int Hour;
    int Minute;
    int Second;
    };

struct events;
struct intEvents;
class LogEntry1;
class LogEntry1Data;
class LogEntry2;
class LogEntry3;
class LogEntry4;
class LogEntry5;
class LogEntry6;
class LogExtensions;
class RoomC;

struct directoryinfo
    {
    long    DateTime;
    ulong   Length;
#ifdef WINCIT
    DWORD   Attr;
    char    FullName[MAX_PATH];
#else
    char    Attr;
#endif
    char    Name[13];
    };



class TwirlyCursorC
    {
    int Length;                     // Length of twirly string
    int Position;                   // Position in the string
    Bool Active;                    // Being displayed?

public:
    void Start(WC_TW);              // uimisc.cpp
    void Update(WC_TW);             // uimisc.cpp
    void Stop(WC_TW);               // uimisc.cpp
    };


class ConsoleLockC
    {
    // CON_LOCKED: Nothing works
    // CON_F6LOCKED: All but F6 (modified and unmodified) works
    // CON_UNLOCKED: Everything works

    enum ConsoleLockE
        {
        CON_LOCKED,     CON_F6LOCKED,   CON_UNLOCKED,
        } LockState;

public:
    ConsoleLockC(void)
        {
        LockState = CON_UNLOCKED;
        }

    void Lock(void)
        {
        LockState = CON_LOCKED;
        }

    void LockF6(void)
        {
        LockState = CON_F6LOCKED;
        }

    void Unlock(void)
        {
        LockState = CON_UNLOCKED;
        }

    Bool MayUnlock(void);
    Bool IsLocked(Bool CheckingF6 = FALSE);
    };


enum MCICommandsE
    {
    MCI_REALNAME,   MCI_LASTNAME,       MCI_PHONENUM,   MCI_ADDR,
    MCI_LASTCALLD,  MCI_ACCTBAL,        MCI_ROOMNAME,   MCI_HALLNAME,
    MCI_NODENAME,   MCI_NODEPHONE,      MCI_DISKFREE,   MCI_NUMMSGS,
    MCI_CLS,        MCI_CALLNUM,        MCI_SYSOPNAME,  MCI_CONNRATE,
    MCI_PORTRATE,   MCI_MSGROOM,        MCI_BEEPNUM,    MCI_GETCHAR,
    MCI_GETSTR,     MCI_PUTCHAR,        MCI_PUTSTR,     MCI_BSNUM,
    MCI_OUTSPEC,    MCI_SLOW,           MCI_PASSWORD,   MCI_INITIALS,
    MCI_GOTO,       MCI_LABEL,          MCI_COMPARE,    MCI_GT,
    MCI_LT,         MCI_EQ,             MCI_RANDOM,     MCI_ASSIGN,
    MCI_ADD,        MCI_SUBTRACT,       MCI_TIMES,      MCI_DIVIDE,
    MCI_ASGNNEXT,   MCI_HANGINGINDENT,  MCI_HELPFILE,

    // These are not in the MCICodes array...
    MCI_USERNAME,   MCI_FIRSTNAME,  MCI_TIME,       MCI_DATE,
    MCI_POOP,

    MCI_NUMCODES,
    };


class MCIRecursionCheckerC
    {
    Bool MCIDisabled[MCI_NUMCODES];
    Bool MCIBusy[MCI_NUMCODES];

public:
    void EnableAll(void)
        {
        memset(MCIDisabled, 0, sizeof(MCIDisabled));
        }

    MCIRecursionCheckerC(void)
        {
        EnableAll();
        memset(MCIBusy, 0, sizeof(MCIBusy));
        }

    Bool Start(MCICommandsE WhichOne);  // mci.cpp

    void End(MCICommandsE WhichOne)
        {
        assert(WhichOne >= (MCICommandsE) 0);
        assert(WhichOne < MCI_NUMCODES);

        assert(MCIBusy[WhichOne]);
        MCIBusy[WhichOne] = FALSE;
        }

    void SetEnabled(MCICommandsE WhichOne, Bool Enable)
        {
        assert(WhichOne >= (MCICommandsE) 0);
        assert(WhichOne < MCI_NUMCODES);

        MCIDisabled[WhichOne] = !Enable;
        }
    };


class ScreenSaverC
    {
    long Timer;                     // time of last key hit
    Bool On;                        // if it is turned on
    Bool MayTurnOn;                 // If it is allowed.

public:
    ScreenSaverC(void)
        {
        Timer = time(NULL);
        On = FALSE;
        MayTurnOn = FALSE;
        }

    Bool IsOn(void) const
        {
        return (On);
        }

    void Check(void);
    Bool TurnOn(void);
    void TurnOff(void);

    void Update(void)
        {
        Timer = time(NULL);

        if (On)
            {
            TurnOff();
            }
        }

    void SetMayTurnOn(Bool New)
        {
        MayTurnOn = !!New;
        }
    };

class StatusLineC;

class KeyboardBufferC
    {
    char Buffer[KEYBUFSIZE];
    int Start, End;
    Bool Dirty;

    strList *InsertedStrings;

    char filterChar(const char In)
        {
        if (In == '\n')
            {
            return ('\r');
            }

        return (In);
        }

public:

    void Flush(void)
        {
void disposeLL(void **list);    // from proto.h
        disposeLL((void **) &InsertedStrings);

        Start = End = 0;
        Dirty = TRUE;
        }

    KeyboardBufferC(void)
        {
        InsertedStrings = NULL;
        Flush();
        }

    ~KeyboardBufferC(void)
        {
        Flush();
        }

    Bool HasInsertedStrings(void) const
        {
        return (!!InsertedStrings);
        }

    Bool IsFull(void) const
        {
        return ((End + 1) % KEYBUFSIZE == Start);
        }

    Bool Add(char NewKey)
        {
        NewKey = filterChar(NewKey);

        if (!IsFull())
            {
            Buffer[End] = NewKey;
            End = (End + 1) % KEYBUFSIZE;
            Dirty = TRUE;

            return (TRUE);
            }
        else
            {
            return (FALSE);
            }
        }

    Bool InsertString(const char *NewString);

    Bool IsEmpty(void) const
        {
        return (!InsertedStrings && (Start == End));
        }

    Bool IsDirty(void) const
        {
        return (Dirty);
        }

    void SetDirty(Bool NewDirt = TRUE)
        {
        Dirty = NewDirt;
        }

    char Peek(void) const
        {
        if (IsEmpty())
            {
            return (0);
            }
        else
            {
            if (InsertedStrings)
                {
                return (*InsertedStrings->string);
                }
            else
                {
                return (Buffer[Start]);
                }
            }
        }

    char Retrieve(void);
    };


enum CronCallerE
    {
    CRON_TIMEOUT,       CRON_LOGOUT,        CRON_PROMPT,
    };

class CronEventC
    {
    CronTypesE Type;            // Event type (network, shell, etc.)
    label String;               // nodename, shell command, etc

    int RedoTime;               // minutes before it will redo
    int RetryTime;              // minutes before it will retry

    long LastSuccess;           // last time it has success
    long LastTried;             // last time it tried

    Bool Zapped;                // was this event zapped?

    char Months[NUM_MONTHS];    // valid months for event
    char Dates[31];             // valid dates for event
    char Hours[24];             // valid hours for event
    char Days[NUM_DAYS];        // valid days for event

public:
    CronEventC(void)
        {
        memset(this, 0, sizeof(*this));
        }

    CronTypesE GetType(void) const
        {
        assert(this);
        return (Type);
        }

    void Reset(void)
        {
        LastSuccess = LastTried = 0;
        Zapped = FALSE;
        }

    void ToggleZap(void)
        {
        assert(this);
        Zapped = !Zapped;
        }

    Bool IsZapped(void) const
        {
        return (Zapped);
        }

    void SetString(const char *NewStr)
        {
        CopyStringToBuffer(String, NewStr);
        }

    const char *GetString(void) const
        {
        return (String);
        }

    void SetType(CronTypesE NewType)
        {
        assert(this);
        Type = NewType;
        }

    Bool CanDo(Bool IgnoreTime = FALSE) const;

    void SetRedo(int NewRedo)
        {
        assert(this);
        RedoTime = NewRedo;
        }

    void SetRetry(int NewRetry)
        {
        assert(this);
        RetryTime = NewRetry;
        }

    int GetRedo(void) const
        {
        assert(this);
        return (RedoTime);
        }

    int GetRetry(void) const
        {
        assert(this);
        return (RetryTime);
        }

    void SetLastSuccess(time_t Time = 0)
        {
        assert(this);
        if (!Time)
            {
            Time = time(NULL);
            }

        LastSuccess = Time;
        }

    void SetLastTried(time_t Time = 0)
        {
        assert(this);
        if (!Time)
            {
            Time = time(NULL);
            }

        LastTried = Time;
        }

    void SetDone(void)
        {
        SetLastSuccess();
        SetLastTried(LastSuccess);
        }

    time_t GetLastSuccess(void) const
        {
        assert(this);
        return (LastSuccess);
        }

    time_t GetLastTried(void) const
        {
        assert(this);
        return (LastTried);
        }

    Bool LastAttemptFailed(void) const
        {
        return (LastTried != LastSuccess);
        }

    void SetMonth(MonthsE NewMonth, Bool Okay)
        {
        assert(this);
        assert(NewMonth >= (MonthsE) 0 && NewMonth < NUM_MONTHS);
        Months[NewMonth] = (char) Okay;
        }

    void SetDate(int NewDate, Bool Okay)
        {
        assert(this);
        assert(NewDate > 0 && NewDate < 32);
        Dates[NewDate - 1] = (char) Okay;
        }

    void SetHour(int NewHour, Bool Okay)
        {
        assert(this);
        assert(NewHour >= 0 && NewHour <= 23);
        Hours[NewHour] = (char) Okay;
        }

    void SetDay(DaysE NewDay, Bool Okay)
        {
        assert(this);
        assert(NewDay >= (DaysE) 0 && NewDay < NUM_DAYS);
        Days[NewDay] = (char) Okay;
        }

    Bool Do(void);
    Bool Store(FILE *file);
    Bool Load(FILE *file);
    };

struct CronEventListS
    {
    CronEventListS *next;
    CronEventC Event;
    };


class CronC
    {
    time_t StartTime;       // When to count from

    Bool Pause;             // .SCP state

    CronEventListS *Events;
    uint NumEvents;
    CronEventListS *OnEvent;

    CronEventListS *ExecutingEvent;

public:
    CronC(void)
        {
        StartTime = time(NULL);
        Pause = FALSE;
        Events = OnEvent = NULL;
        NumEvents = 0;
        }

    void ResetTimer(void)
        {
        StartTime = time(NULL);
        }

    void SetPause(Bool NewP = TRUE)
        {
        Pause = NewP;
        }

    CronEventListS *AddEvent(void);
    void DumpEvents(void);

    void TogglePause(void)
        {
        Pause = !Pause;
        }

    Bool HasEvents(void) const
        {
        return (Events != NULL);
        }

    Bool IsOnEvent(const CronEventListS *Test) const
        {
        return (OnEvent == Test);
        }

    void SetOnEvent(CronEventC *Set);

    void SetOnEvent(CronEventListS *Set)
        {
        OnEvent = Set ? Set : Events;
        }

    Bool IsPaused(void) const
        {
        return (Pause);
        }

    uint GetNumEvents(void) const
        {
        return (NumEvents);
        }

    time_t GetStartTime(void) const
        {
        return (StartTime);
        }

    const CronEventListS *GetExecutingEvent(void) const
        {
        return (ExecutingEvent);
        }

    Bool IsReady(void) const
        {
        return ((((int) (time(NULL) - StartTime)) / 60) >= cfg.idle);
        }

    CronEventListS *GetEventNum(int Number);
    Bool Add(const CronEventC &NewEvent);
    const CronEventC *GetEvent(int Num) const;
    Bool Do(CronCallerE WhyCall);

    Bool WriteTable(void);
    Bool ReadTable(WC_TW);
    Bool ReadCronCit(WC_TW);
    };

#ifdef MULTI

struct xLoginListS
    {
    xLoginListS *next;
    l_index Index;
    };

class LoginListC
    {
    HANDLE OurMutex;

    xLoginListS *List;

    void Lock(void)
        {
        WaitForSingleObject(OurMutex, INFINITE);
        }

    Bool TimedLock(void)
        {
        return (WaitForSingleObject(OurMutex, 2000) != WAIT_TIMEOUT);
        }

    void Unlock(void)
        {
        ReleaseMutex(OurMutex);
        }

public:
    LoginListC(void)
        {
        List = NULL;
        OurMutex = CreateMutex(NULL, FALSE, NULL);
        }

    ~LoginListC();

    Bool Add(l_index Index);
    Bool AddUnique(l_index Index);
    Bool Remove(l_index Index);
    Bool IsLoggedIn(l_index Index);
    };

class SerialPortC;

struct SerialPortListS
    {
    SerialPortListS *next;
    SerialPortC *Port;
    };
#endif


struct CITWINDOW;

enum OSTypeE
    {
    OS_DOS,     OS_OS2,     OS_DV,      OS_WINS,        OS_WIN3,
    OS_NUM
    };


class TermCapC;         // term.h for full definition
struct ScriptInfoS;     // script.h for full definition
struct editors;         // extedit.h for full definition
struct hufTree;         // huf.h for full definition
class LogExtensions;    // log.h for full definition

#ifdef MAIN
#include "statline.h"
#endif



#ifdef WINCIT
class SequencerC
    {
private:
    long NextSequence;
    HANDLE OurMutex;

public:
    SequencerC(void)
        {
    	NextSequence = 0;
    	OurMutex = CreateMutex(NULL, FALSE, NULL);
        //SequencerC(0);
        }

    SequencerC(int Initial)
        {
        NextSequence = Initial;
        OurMutex = CreateMutex(NULL, FALSE, NULL);
        }

    ~SequencerC(void)
        {
        if (OurMutex != INVALID_HANDLE_VALUE)
            {
            CloseHandle(OurMutex);
            }
        }

    long Next(void)
        {
        WaitForSingleObject(OurMutex, INFINITE);
        const long ToRet = NextSequence++;
        ReleaseMutex(OurMutex);

        return (ToRet);
        }
    };
#else

    EXTERN void *comDriver;
    EXTERN label cdDesc;
    EXTERN void cdecl (*initrs)(int Port, int Baud, int Stops, int Parity, int Length, int cCts);
    EXTERN void cdecl (*deinitrs)(void);
    EXTERN void cdecl (*flushrs)(void);
    EXTERN int cdecl (*statrs)(void);
    EXTERN int cdecl (*getrs)(void);
    EXTERN void cdecl (*putrs)(char ch);
    EXTERN void cdecl (*dtrrs)(int dtr);
    EXTERN int cdecl (*carrstatrs)(void);
    EXTERN int cdecl (*ringstatrs)(void);
    EXTERN void cdecl (*flushoutrs)(void);
#endif


enum ComTypeE
    {
    CT_UNKNOWN,     CT_SERIAL,      CT_LOCAL,       CT_TELNET,

    CT_NUM
    };

enum InputReadyE
    {
    IR_NONE,    IR_READY,   IR_IGNORE,

    IR_NUM
    };

#ifdef WINCIT
#include "global.h"
class CommunicationPortC
    {
    Bool Initialized;

    ulong Transmitted;              // # characters transmitted
    ulong Received;                 // # characters received

    uchar BeenSent[KEYBUFSIZE];
    int SentStart, SentEnd, NumBeenSent;

    uchar BeenReceived[KEYBUFSIZE];
    int RecStart, RecEnd, NumBeenReceived;

    Bool Dirty;

    virtual uchar getChar(void) = 0;
    virtual void putChar(uchar Out) = 0;

    ComTypeE ComType;

protected:
    PortSpeedE Speed;
    ModemSpeedE ModemSpeed;
    int PortNumber;

public:
    label PortName;                 // Human-readable name of port

    Bool UsingCompression;
    Bool UsingCorrection;

    HANDLE InputWaiting;

    CommunicationPortC(ComTypeE CT)
        {
        Initialized = FALSE;
        Speed = PS_300;
        ModemSpeed = MS_300;
        UsingCompression = UsingCorrection = FALSE;
        Transmitted = Received = 0;
        SentStart = SentEnd = RecStart = RecEnd = NumBeenSent = NumBeenReceived = 0;
        Dirty = FALSE;
        ComType = CT;

        label eName;
        sprintf(eName, "WincitEvent:%p", this);
        InputWaiting = CreateEvent(NULL, TRUE, TRUE, eName);
        }

    virtual ~CommunicationPortC(void)
        {
        if (InputWaiting)
            {
            CloseHandle(InputWaiting);
            }
        }

    ComTypeE GetType(void) const
        {
        return (ComType);
        }

    Bool IsDirty(void) const
        {
        return (Dirty);
        }

    void SetDirty(Bool NewDirt = TRUE)
        {
        Dirty = NewDirt;
        }

    Bool IsInitialized(void) const
        {
        return (Initialized);
        }

    void SetInitialized(Bool NewInit = TRUE)
        {
        Initialized = NewInit;
        }

    virtual InputReadyE CheckInputReady(void) = 0;
    virtual void FlushOutput(void) = 0;
    virtual Bool HaveConnection(void) const = 0;

    virtual void Ping(void) {}

    Bool IsInputReady(void);

    uchar Input(void)
        {
        uchar ToRet = getChar();
        Received++;

        SetDirty();

        BeenReceived[RecEnd] = ToRet;
        RecEnd = (RecEnd + 1) % KEYBUFSIZE;
        if (RecEnd == RecStart)
            {
            RecStart = (RecStart + 1) % KEYBUFSIZE;
            }
        else
            {
            NumBeenReceived++;
            }

        return (ToRet);
        }

    int GetNumInSentBuffer(void) const
        {
        return (NumBeenSent);
        }

    int GetNumInReceivedBuffer(void) const
        {
        return (NumBeenReceived);
        }

    virtual void Init(void) = 0;
    virtual void Deinit(void) = 0;
    virtual void BreakConnection(void) = 0;

    void ResetReceived(void)
        {
        Received = 0;
        }

    void ResetTransmitted(void)
        {
        Transmitted = 0;
        }

    void IncReceived(void)
        {
        Received++;
        }

    void IncTransmitted(void)
        {
        Transmitted++;
        }

    ulong GetReceived(void) const
        {
        return (Received);
        }

    ulong GetTransmitted(void) const
        {
        return (Transmitted);
        }

    void Output(uchar Out);

    void FlushInput(void);

    uchar GetFromSentBuffer(int Num) const
        {
        return (BeenSent[(SentStart + Num - 1) % KEYBUFSIZE]);
        }

    uchar GetFromReceivedBuffer(int Num) const
        {
        return (BeenReceived[(RecStart + Num - 1) % KEYBUFSIZE]);
        }

    virtual void SetSpeed(PortSpeedE NewSpeed) = 0;

    PortSpeedE GetSpeed(void) const
        {
        return (Speed);
        }

    ModemSpeedE GetModemSpeed(void) const
        {
        return (ModemSpeed);
        }

    void SetModemSpeed(ModemSpeedE MSpeed)
        {
        ModemSpeed = MSpeed;
        }

    void OutString(const char *String, int Pace = 0);

    virtual long GetTransmitSpeed(void)
        {
        return (connectbauds[ModemSpeed]);
        }

    virtual long GetReceiveSpeed(void)
        {
        return (connectbauds[ModemSpeed]);
        }

    int GetPortNumber(void)
        {
        return (PortNumber);
        }

    virtual void Pause(void) {}
    virtual void Resume(void) {}
    };


class NoComPortC: public CommunicationPortC
    {
    static int ActiveCounter;
    static int TotalCounter;

    virtual uchar getChar(void)                     { return (0); }
    virtual void putChar(uchar Out)                 {}

public:
    NoComPortC(void) : CommunicationPortC(CT_LOCAL)
        {
        // We'll never have input waiting... just turn off the event
        if (InputWaiting)
            {
            ResetEvent(InputWaiting);
            }

        sprintf(PortName, "Local%d", ++TotalCounter);
        ActiveCounter++;
        }

    virtual ~NoComPortC(void)                       { TotalCounter--; }
    int GetCounter(void)                            { return (TotalCounter); }

    virtual InputReadyE CheckInputReady(void)       { return (IR_NONE); }
    virtual void FlushOutput(void)                  {}
    virtual Bool HaveConnection(void) const         { return (FALSE); }
    virtual void Init(void)                         {}
    virtual void Deinit(void)                       {}
    virtual void BreakConnection(void)              {}
    virtual void SetSpeed(PortSpeedE NewSpeed)      { Speed = NewSpeed; }
    };
#endif

// --------------------------------------------------------------------------
// Citadel can be told to do certain things after waiting a certain
// amount of time during certain hours of certain days of certain
// months and certainly this is getting hard to follow. These events
// called Cron Events, are read from CRON.CIT and stored in the this
// structure, which forms a linked list in memory. Because this is
// time dependent, Citadel also stores this information in CRON.TAB
// when it exits.


// This enumeration is supposed to be used to tell the Cron code why
// it is being called. However, the only one that is ever used is
// CRON_TIMEOUT. The other two might be implemented some time, but I
// really doubt it.
#ifdef MULTI
    #define tiEXT
    #define TERMWINDOWMEMBER TermWindowC::

    class TermWindowC
        {
    private:
        HANDLE SystemEventMutex;
        SystemEventS *PendingSystemEvents;

    public:
        DWORD OurThreadId;
        long OurConSeq;

        label WindowCaption;
        Bool SetTitle(void);

        TermWindowC(CommunicationPortC *CP, long OurConSeq);
        ~TermWindowC(void);

        Bool IsGood(void);

        HANDLE hThread;

        Bool AddSystemEvent(SystemEventE Type, int Data, void *MoreData, Bool HideFromConsole, const char *Event);
#else
    #define tiEXT   EXTERN
    #define TERMWINDOWMEMBER
    #define WindowCaption  ns
#endif

tiEXT   ReadFilter      rf;             // read filter
tiEXT   ReadFilter      rf2;            // Saved read filter

tiEXT   char            prevChar;       // for EOLN/EOParagraph stuff
tiEXT   char            PAD;
tiEXT   long            logtimestamp;   // when someone last logged in
tiEXT   long            conntimestamp;  // when connection was made
tiEXT   UserAccountInfo *CurrentUserAccount;// Groupdata for current user
tiEXT   NodesCitC   *node;          // node buffer
tiEXT   Bool            chatReq;        // Did they want to chat?
tiEXT   h_slot          thisHall;       // hall we're in
tiEXT   LogEntry        *CurrentUser;   // Task dependent log stuff.
tiEXT   l_index         ThisLog;        // where in file
tiEXT   l_slot          ThisSlot;       // where in table
tiEXT   Bool            loggedIn;       // Global have-caller flag
tiEXT   Bool            modStat;        // Had carrier LAST time checked
tiEXT   Bool            sleepkey;       // Alt-Z pressed
tiEXT   Bool            chatkey;        // F8 or Alt-C pressed
tiEXT   Bool            showdir;        // for .K... if !expert
tiEXT   Bool            showhidden;     // for .K... if !expert
tiEXT   Bool            showbio;        // for .K... if !expert
tiEXT   Bool            showgroup;      // for .K... if !expert
tiEXT   uint            roomsMade;      // # rooms made this call
tiEXT   char            gprompt[256];
tiEXT   int             numLines;
tiEXT   long            wowcount;       // wowcount

tiEXT   TermCapC        *TermCap;

tiEXT   int             logiRow, logiCol;// Cursor position


tiEXT   MsgReadOptions  MRO;            // .R!, .R&, etc.
tiEXT   MsgStatus       MS;             // ...

tiEXT   Bool            callout;        // Always send to modem if TRUE.

tiEXT   Bool            netError;       // save net error message?

tiEXT   Bool            netFailed;      // Did netting fail?
tiEXT   fileQueue       *fileList;      // .Q...
tiEXT   long            altF3Timeout;   // When will timeout
tiEXT   label           MCI_str[10];    // String variables
tiEXT   char            MCI_char[10];   // Character variables
tiEXT   int             MCI_result;     // Last result.
tiEXT   time_t          LastActiveTime; // The last time the user did anything
tiEXT   r_slot          thisRoom;       // Which room user is in.
tiEXT   RoomC           *CurrentRoom;   // The room we are in.
tiEXT   TalleyBufferC   *Talley;        // Our Talley BUFFER!!!!!!
tiEXT   Message         *AideMsg;       // For aide message being created
tiEXT   Bool            ExitToMsdos;    // bring system down
tiEXT   dowhattype      DoWhat;         // What this task is doing
tiEXT   dowhattype      PDoWhat;        // What was doing before PROMPT
tiEXT   Bool            showPrompt;     // should show the prompt?
tiEXT   Bool            seen;
tiEXT   Bool            displayedmessage;

tiEXT   OutputControl   OC;             // Control of Citadel's output

#ifdef WINCIT
tiEXT   PCHAR_INFO      ScreenBuffer;   // Where we put our output.
tiEXT   Bool            CursorVisible;  // Is it?
#else
tiEXT   char            *logiScreen;    // logical address of screen
tiEXT   char            *saveBuffer;    // buffer for screen saves
tiEXT   void            *talleyInfo;    // whee
#endif

tiEXT   TwirlyCursorC   TwirlyCursor; // The twirly cursor.

tiEXT   MCIRecursionCheckerC MCIRecursionChecker;   // Wow.

tiEXT   char            ContextSensitiveScriptText[256];
tiEXT   KeyboardBufferC KeyboardBuffer; // The keyboard buffer.

tiEXT   CITWINDOW       *KeyboardBufferMonitor; // It's monitor window.
tiEXT   CITWINDOW       *SerialPortMonitor; // It's monitor window.
tiEXT   CITWINDOW       *CronMonitor;   // It's monitor window.

#ifdef WINCIT
tiEXT   CommunicationPortC *CommPort;   // Our link to the outside world
#endif


tiEXT   Bool AllowESC;                  // For EDIT.CPP

tiEXT   Message         *MsgScriptMsg;

#ifdef WINCIT
tiEXT   fpath           LocalTempPath;  // Our thread's very own path!
#else
#define LocalTempPath cfg.temppath
#endif

tiEXT   Bool            IdlePrompt;     // Don't show the full room prompt
tiEXT   Bool            CursorIsAtPrompt;      
tiEXT   Bool            OkayToShowEvents;      

    // Chat.CPP
tiEXT   int             LastChat;       // What the user did last time
tiEXT   label           LastChatUser;
tiEXT   label           LastChatGroup;


    // DOMISC.CPP
tiEXT   int             Ycounter;       // for doY()

    // output.cpp
tiEXT   Bool            UpDoWn;         // You do not want to know

    // misc.cpp
tiEXT   uint            borderCounter;  // Counts between borders
tiEXT   uint            lastBorder;     // Last one displayed

tiEXT   Bool            MCI_asgnnext;   // Assign next MCI to a variable?
tiEXT   int             MCI_asgnvar;    // Which one?


    // net1.cpp
tiEXT   FILE            *nodefile;


    // netcmd.cpp
tiEXT   Message         *NetCommandMsg;
tiEXT   Message         *NetCmdReplyMsg;

    // outovl.cpp
tiEXT   Bool            PLfirst;        // For prtList()
tiEXT   Bool            PLlisted;       // For prtList()


    // window.cpp
tiEXT   char            ANSIargs[ANSILEN];// current ANSI arguments
tiEXT   Bool            ANSIfirst;      // Beginning of ANSI
tiEXT   Bool            ANSIhilight;    // hilight is on
tiEXT   Bool            ANSIblink;      // blinking is on
tiEXT   Bool            ANSIisSound;    // Sound output
tiEXT   int             ANSIc_x;        // stored X position
tiEXT   int             ANSIc_y;        // stored Y position


    // ctdl.cpp
tiEXT   Bool            hitSix;         // for doRegular()


    // files.cpp
tiEXT   char            dlTimeBuff[10]; // for dltime()


    // help.cpp
tiEXT   int             ansiChat;       // for nochat()
tiEXT   int             normChat;       // for nochat()


tiEXT   FILE            *journalfl;     // journal file descriptor


#ifdef WINCIT
tiEXT   l_slot          *LogOrder;      // sorted differently for each thread
#endif

tiEXT   Bool            sysopkey;       // F6 pressed
tiEXT   Bool            sysopNew;
tiEXT   Bool            dialout_fkey;   // f-keys in dialout mode

tiEXT   Bool            seenAllRooms;   // "far-off scribbling"...


    // Normal functions if not MULTI; member functions if MULTI

    // ANDY.CPP
    void cdecl Andy(const char *Format, const char *Codes, ...);
    int Brent(const char **Format, int *colCount, char *collect);


    // APLFILE.CPP
    void writeAplFile(void);
    void readAplFile(void);
    void wrtLvl(int x, FILE *fd);
    void wrtAddr(FILE *fd);
    void writeCallinfo(void);
    void writeChain(void);
    void writeDoorfileSr(void);
    void writeDoorSys(void);
    void writeDorinfo(void);
    void writePcboard(void);
    void writeTribbs(void);
    void writeUserinfo(void);
    long UserSpeed(Bool both);          // both = 0 or 1 for port rate
    long LogTime(void);                 // when logged in
    int Credits(int divisor);           // 60 for minutes, 1 for secs
    void wrtUserSpd(Bool x, FILE *fd);
    void wrtCredits(int x, FILE *fd);


    // APLIC.CPP
    int ExeAplic(void);
    void shellescape(Bool super);
    void wxsnd(const char *path, const char *file, const protocols *theProt, int type);
    void wxrcv(const char *path, const char *file, const protocols *theProt);
    Bool execDoor(char c, int where);
    int apsystem(const char *stuff, Bool trapit);
    int RunApplication(const char *CommandLine, const char *FileName, Bool TrapIt, Bool ChangeDir);
    void EnterApplication(void);
    void EnterDoor(void);


    // BOOLEXPR.CPP
#ifdef WINCIT
    void ShowBooleanExpression(const BoolExpr Expression, void (*Shower)(short, TermWindowC *TW));
#else
    void ShowBooleanExpression(const BoolExpr Expression, void (*Shower)(short));
#endif


    // CARRIER.CPP
    void CarrierJustFound(void);
    void CarrierJustLost(void);


    // CONOVL.CPP
    void StartPrinting(const char *FileName);
    void fkey(uint inkey);
    void outConRawBs(void);
    void cdecl cOutput(const char *fmt);
    void cdecl cPrintf(const char *fmt, ...);


    // CONSOLE.CPP
    void doccr(void);
    Bool KBReady(void);
    int ciChar(void);
    void outCon(char c);
    Bool kb_hit(void);


    // CHAT.CPP
    void chat(void);
    Bool ringSysop(void);
    void doChat(Bool expand);
    void ChatAll(void);
    void ChatUser(void);
    void ChatGroup(void);
    void ChatRoom(void);
    void ChatConsole(void);


    // COMMANDS.CPP
    void DoCommand(UserCommandsE Cmd, long P1 = 0, long P2 = 0, long P3 = 0);


    // CRON.CPP
    void ListAllEvents(void);
    void cron_commands(void);
    CronEventC *GetEventFromUser(void);


    // CTDL.CPP
    void MainCitadelLoop(void);
    Bool doRegular(Bool x, char c);
    Bool takeAutoUp(char c);
    void CheckTimeOnSystem(void);
    Bool getCommand(char *c, Bool prompt);
    void doAd(Bool force);
    void ShowPrompt(void);


    // DOAIDE.CPP
    void doAide(Bool moreYet, char first);
    void AideNameMessages(void);


    // DOENTER.CPP
    void doEnter(Bool moreYet, char first);
    void EnterName(void);
    void EnterPassword(void);
    void EnterTitleSurname(void);


    // DOINV.CPP
    void doInvite(void);
    void InviteUserList(void);
    void InviteRoomList(void);
    void InviteUser(void);


    // DOMENU.CPP
    int DoMenu(MenuE WhichMenu, int letter);


    // DOMISC.CPP
    void doVolkswagen(Bool moreYet, const char *CmdKeys);
    void doHelp(Bool expand, const char *HelpFile);
    void doIntro(void);
    void doLogout(Bool expand, char first);
    void doXpert(void);
    void doverbose(void);
    Bool CheckDatForMore(const char *String);
    void doSmallChat(void);
    void doY(Bool backwards);
    Bool doPoop(char c);
    void doWow(Bool dot);
    void doBorder(void);
    void FingerDelete(void);
    void FingerEdit(void);
    void FingerList(Bool docr);
    void FingerUser(void);
    void doFinger(void);
    void doReadBull(Bool FromRoomPrompt);
    void TerminateQuitAlso(Bool Ask);
    void TerminateStay(void);


    // DOREAD.CPP
    void doRead(Bool moreYet, char first);
    Bool ProcessReadFilter(ReadFilter *rf, Bool AllowMetaGroups);
    void resetMsgfilter(int resetOld);


    // DORMHALL.CPP
    void exclude(void);
    void doGoto(Bool expand, char skip);
    void doGotoMail(void);
    void doKnown(Bool moreYet, char first);
    void doNext(void);
    void doPrevious(void);
    void doNextHall(void);
    void doPreviousHall(void);
    void doPersonal(void);
    void ListRoomsNotInPersonalHall(void);
    void ListRoomsInPersonalHall(void);


    // DOSYSOP.CPP
    void doSysop(void);
    void doSysFPDown(void);
    void ShowTables(void);
    void do_SysopEnter(void);
    void do_SysopHall(void);
    void do_SysopGroup(void);


    // DOUPDOWN.CPP
    void doFileQueue(void);
    void doDownload(Bool ex);
    void doUpload(Bool ex);


    // EDIT.CPP
    int editText(Message *EditBuffer, int lim, Message *HoldBuffer);
    char *matchString(char *buf, const char *pattern, char *bufEnd, int ver, Bool exact);
    void putheader(Bool first, Message *Msg);
    Bool getText(Message *Msg, char *Buffer);
    void replaceString(char *buf, uint lim, Bool ver, char *oldStr);
    void wordcount(const char *buf);


    // EXTEDIT.CPP
    Bool RunExternalEditor(char Key, size_t len, char *text);
    void RunAutomaticExternalEditors(size_t len, char *text);
    void doExtEdit(const editors *theEditor, Bool autoEdit, size_t len, char *text);


    // FILECMD.CPP
    void XMnu(char cmd);
    Bool CanUploadFile(const char *filename);
    void doTheAdd(const char *fn);
    Bool downloadFileQueue(void);
    void addFileQueue(void);
    void listFileQueue(void);
    void removeFileQueue(void);
    void clearFileQueue(void);
    void AideAttributes(void);
    void blocks(const char *filename, long length, int bsize);
    Bool CheckDirectoryRoom(void);
    void EnterTextfile(Bool HideIt);
    void EnterWithProtocol(Bool HideIt);
    void ReadDirectory(OldNewPick which);
    void ReadTextfile(OldNewPick which, Bool Verbose);
    void ReadWithProtocol(void);
    void AideRenameFile(void);
    void AideUnlink(void);
    void doTheDownload(const char *filename, const char *thePath, Bool fullPath, const protocols *theProt, Bool queue);
    void download(char c);
    void upload(char c, Bool HideIt);
    void upDownMnu(char cmd, Bool respOnly);
    void textdown(const char *filename, OldNewPick which);
    void textup(const char *filename, Bool HideIt);
    void wcdown(const char *filename, InternalProtocols prot);
    void wcup(const char *filename, InternalProtocols prot);
    void ActuallyDoTheUpload(const protocols *theProt, Bool HideIt, Bool Ask);
    InternalProtocols XmodemMenu(char cmd);
    Bool getNewOldFileDate(void);
    void AideMoveFile(void);
    Bool IsFileDownloadOkay(void);
    Bool IsFileUploadOkay(void);
    void AideFileInfoSet(void);


    // FILES.CPP
    directoryinfo *filldirectory(const char *filename, SortE Sort, OldNewPick which, Bool rev);
    const char *dltime(long size);

    // GLOBVER.CPP
    void globalverify(void);


    // GROUP.CPP
    g_slot FindGroupByPartialName(const char *TestName, Bool IgnoreInGroup);


    // GRPEDIT.CPP
    int killgroup(const char *gname);
    void AideListGroup(void);
    Bool newgroup(const char *name, const char *desc, Bool lock, Bool hide, Bool autoadd);
    Bool renamegroup(void);

    // GRPMEMBR.CPP
    void OperatorSpecificMembership(void);
    void groupfunc(const char *grpName, const char *userName, Bool addToGroup);
    void globalgroup(const char *theGroup, Bool addAll);
    void globaluser(void);
    void AideGroup(void);


    // GRPOVL.CPP
    void ListGroups(Bool CheckInGroup);


    // HALL.CPP
    h_slot partialhall(const char *hallname);


    // HALLEDIT.CPP
    void globalhall(void);
    void hallfunc(void);
    Bool killhall(const char *hn);
    Bool newhall(const char *name, const char *group, r_slot room);
    Bool renamehall(void);
    void windowfunc(void);
    int xhallfunc(r_slot roomslot, h_slot hallslot, int xyn, Bool AddToAideMsg);
    void AideHall(void);
    void AideWindow(void);


    // HALLOVL.CPP
    void force(void);
    void ListHalls(Bool WindowedOffRoom);
    Bool accesshall(h_slot slot);
    void EnterHall(void);
    void knownhalls(void);
    void ReadHalls(void);
    void stephall(int direction);


    // HELP.CPP
    int dumpf(const char *filename, Bool format, int special);
    int dumpfc(const char *filename, Bool format);
    void tutorial(const char *filename, const Bool *ShownInternal = NULL);
    int BLBRotate(const char *base, const char *ext, int *reg, int *ansi, Bool special, BlurbFiles BlbNum);
    void hello(void);
    void cometochat(void);
    void goodbye(void);
    void nochat(Bool reset);
    void showMenu(MenuNames reqMnu, Bool isHelp);
    void showMenu(MenuNames reqMnu);
    void ShowHelpMenu(void);
    void youAreHere(void);
    void dispBlb(BlurbFiles theBlb);
    void dispHlp(HelpFiles theHlp);
    long getMnuOff(int reqMnu, char *menuFile);


    // HUF.CPP
#if VERSION != RELEASE
    void CheckTreePointer(const void *Check, hufTree *cur);
    void CheckHufPointer(const void *Check);
#endif


    // INFOFILE.CPP
    short FillFileInfo(const char *Directory, const char *Uploader, Bool askuser, char *Report, Bool HideThem);


    // INPOVL.CPP
    long GetNumberWithBlurb(const char *prompt, long min, long max, long dfault, BlurbFiles Blb);
    Bool GetStringWithBlurb(const char *prompt, char *buf, int len, const char *dfault, BlurbFiles Blb);
    Bool getNormStr(const char *prompt, char *s, int size);
    Bool getNormStr(const char *prompt, char *s, int size, Bool doEcho);
    long getNumber(const char *prompt, long bottom, long top, long dfaultnum, Bool QuestIsSpecial, Bool *QuestionEntered);
    Bool getString(const char *prompt, char *buf, int lim, const char *dfault);
    Bool getString(const char *prompt, char *buf, int lim, Bool QuestIsSpecial, const char *dfault);
    Bool getString(const char *prompt, char *buf, int lim, Bool QuestIsSpecial, Bool doEcho, const char *dfault);
    int cdecl GetOneKey(const char *prompt, const char *keys, const char dfault, BlurbFiles HelpBlurb, ...);
    int getYesNo(const char *prompt, int dfault);
    void getFmtString(const char *prompt, char *buf, Bool premature, Bool inclPrompt);


    // INPUT.CPP
    Bool BBSCharReady(void);
    int iCharNE(void);
    int iChar(void);
    Bool IsKeyFromUserReady(void);


    // LOG.CPP
    void storeLog(void);
    void displaypw(const char *name, const char *in, const char *pw);
    Bool LoadPersonByName(const char *name);

#ifdef WINCIT
    l_slot FindPersonByName(const char *name);
    l_slot nodexists(const char *name);
    l_slot addressexists(const char *name);
    l_slot FindPersonByPartialName(const char *name);
#endif


    // LOG2.CPP
    void purgeuserlog(void);
    Bool killuser(const char *name);


    // LOG3.CPP
    void newPW(Bool check);
    void forwardaddr(LogEntry *EditLog);
    void defaulthall(LogEntry *EditLog, Bool UserlogEdit);
    void doColumn(Bool *Col);
    void listExcl(LogExtensions *LE, KillFileE Type, const char *Pmsg, const char *Pmsgs);
    Bool FinishUpMenu(void);
    void editIt(LogExtensions *LE, KillFileE Type, const char *Pmsg, const char *Pmsgs);
    void listAuthorTags(const LogExtensions *LE, const char *Author);
    void listTaggedAuthors(const LogExtensions *LE);
    void editTags(LogExtensions *LE);
    void DisplayOnOffWithColumn(int Msg, Bool Value, Bool *Col);
    void DisplayYesNoWithColumn(int Msg, Bool Value, Bool *Col);
    void DisplayOnOffWithCR(int Msg, Bool Value);
    void editTerminal(LogEntry1 *EditLog1, Bool UserlogEdit);
    void editPersonalInfo(LogEntry1 *EditLog1);
    void editSystemEvents(LogEntry1 *EditLog1);
    void editFormatStrings(LogEntry1 *EditLog1);
    void listDictionary(const LogExtensions *LE);
    void spellCheckerOptions(LogEntry *EditLog);
    void listReplacedText(const LogExtensions *LE);
    void textReplacementOptions(LogExtensions *LE);
    void messageEditOptions(LogEntry *EditLog);
    void editMsgOption(LogEntry *EditLog, Bool UserlogEdit, Bool *pedited_pointers);
    void userPointers(LogEntry *ToEdit, Bool fromEC);
    Bool PrivilegeEdit(LogEntry *EditLog, Bool UserlogEdit, l_index Index);
    Bool configure(LogEntry *EditLog, Bool newUser, Bool UserlogEdit);
    void EnterConfiguration(Bool newUser);
    Bool DoUserlogEdit(LogEntry *EditLog, l_index Index);


    // LOGEDIT.CPP
    void userEdit(void);


    // LOGIN.CPP
    void setlbvisit(void);
    void doLogin(Bool moreYet);
    void minibin(int CallersSinceLastCall, long CreditsAtLastCall);

#ifdef WINCIT
    l_slot pwslot(const char *in, const char *pw);
    l_slot FindPwInHashInTable(const char *in, const char *pw);
#endif

    // LOGINNEW.CPP
    void newUser(char *initials, char *password);
    void loginNew(char *initials, char *password);
    void newUserFile(void);
    Bool newlog(void);


    // LOGOUT.CPP
    void terminate(Bool discon);
    void setdefaultconfig(Bool keepAnsi);


    // LOGOVL.CPP
    void ListUsers(Bool ListUnlisted, Bool ListOnlyLoggedIn);


    // MCI.CPP
    void MCI_DispOrAsgn(const char *str, label vars[]);
    void MCI(const char *str);


    // MISC.CPP
#ifdef WINCIT
    const char *i64toac(__int64 Number);
#endif
    const char *ltoac(long num);
    Bool HaveConnectionToUser(void);
    void ShowHelpMessage(int msgNumber);
    void SetDoWhat(dowhattype NewDoWhat);


    // MISC2.CPP
    void greeting(void);
    void EnterBorder(void);
    void offhook(void);
    void displayOnOff(const char *string, int value);
    void displayYesNo(const char *string, int value);
    Bool changeOnOff(const char *str, Bool value);
    Bool changeYesNo(const char *str, Bool value);
    Bool theAlgorithm(r_slot rm, h_slot hl, Bool exclude);
    void theOtherAlgorithm(Bool *rooms, h_slot hl, Bool exclude);


    // MISC3.CPP
    void ringSystemREQ(void);
    void dial_out(void);
    void cdecl cCPrintf(const char *fmt, ...);
    void securityViolation(const char *pathname);
    void UserHasTimedOut(void);


    // MMAKEHI.CPP
    Bool makeMessage(Message *Msg, Message *ReplyTo, r_slot PutInRoom = CERROR);


    // MODEM.CPP
    void domcr(void);
    void Hangup(void);


    // MOVEHALL.CPP
    Bool moveHall(int offset, h_slot hall, Bool disp);


    // MSGMAKE.CPP
    void msgtosysop(Message *Msg);


    // MSGMOD.CPP
    void copymessage(m_index id, r_slot roomno);
    void addtomsglist(m_index msgnumber);
    void AideQueueRoomMark(void);
    void AideQueueSort(void);
    void AideQueueMove(void);
    void AideQueueList(void);
    void AideQueueInsert(void);
    Bool AideQueueKill(void);
    void sortmsglist(Bool Reverse);
    void AideQueueClear(void);
    void AideQueueAutomark(void);
    void deleteMessage(m_index id);
    void insert(m_index id);
    Bool markIt(Message *Msg);
    Bool censorIt(Message *Msg);
    Bool pullIt(Message *Msg);
    void markmsg(Message *Msg);
    void massdelete(void);
    void AideCopyMessage(void);
    void AideInsert(void);


    // MSGREAD.CPP
    void ShowTagsForUser(const char *User);
    void RestoreJournalFile(FILE *saved);
    void SuspendJournalFile(FILE **save);
    void PrintMessage(Message *Msg);
    void AdjustDownForRoom(m_index *Number);
    void AdjustUpForRoom(m_index *Number);
    void DoTheStep(int Direction, m_index *ID);
    Bool stepMessage(m_index *at, int Direction);
    Bool SetMessageReadingOptions(void);
    void ActuallyDisplayMessage(Message *Msg, Bool ShowHeader, Bool ShowBody, Bool ShowSignature);
    void showMessages(OldNewPick whichMess, Bool revOrder, long howMany);
    void roomJournal(void);
    Bool ShouldAskToRelease(const Message *Msg);
    Bool IsRecipientOfMail(const Message *Msg);
    Bool ShouldAskToReply(const Message *Msg, OldNewPick whichMess);
    void printheader(Message *Msg);
    Message *LoadMessageByID(m_index ID, Bool CheckMaySeeIt, Bool Display);
    Bool ShouldShowMessage(m_index ID);
    void PrintMessageByID(m_index id);


    // MSGREAD2.CPP
    void readbymsgno(void);
    void ShowWhyMayNotSeeMsg(MaySeeMsgType Why, m_index MsgID);


    // MSGSCRPT.CPP
    void PrepareMsgScriptInit(Message *Msg);
    Bool CheckMsgScript(Message *Msg);
    void DeinitMsgScript(ScriptInfoS *si);
    void InitMsgScript(ScriptInfoS *si);


    // MUSIC.CPP
    void playSound(const char *snd);
    void sound_effect(const char *Sound_Buffer);
    void play(const char *Sound_Buffer);
    void submit_note(int note, long note_len);
    void submit_sound(int freq,int dly);


    // NCMDDBG.CPP
    void DoDebugCmd(const char **ncmsg);


    // NET.CPP
    Bool net_slave(void);
    Bool net_master(void);
    Bool n_login(void);
    Bool parseLine(char *line);
    int n_dial(void);
    int dial(const char *dialstring, int timeout);  // -3 if manually aborted
    Bool wait_for(const char *str, int timeout);
    Bool net_callout(const char *node);


    // NET1.CPP
    Bool Net1Slave(void);
    Bool Net1Master(void);
    void Net1Cleanup(void);
    Bool get_first_room(char *here, char *there);
    Bool get_next_room(char *here, char *there);


    // NET69.CPP
    Bool net69(Bool master);
    void net69_incorporate(ModemConsoleE W);
    Bool net69_fetch(Bool needFull, ModemConsoleE W);
    void net69_makeroomrequest(ModemConsoleE W);
    void cdecl net69_error(Bool Fetch, Bool errorflag, const char *node, const char *fmt, ...);


    // NET69MNU.CPP
    void net69_menu(void);


    // NET86.CPP
    void net86_incorporate(ModemConsoleE W);
    Bool net86_fetch(ModemConsoleE W);
    Bool Citadel86Network(Bool master);


    // NETCMD.CPP
    void readNetCmdTmp(ModemConsoleE W);
    void AppendLineToNetCommandReply(const char *str);


    // NETDC15.CPP
    Bool dc15network(Bool master);
    void sendRequest(const protocols *theProt);
    void reciveRequest(const protocols *theProt);
    void makeSendFile(void);
    void sendFiles(const protocols *theProt);
    void receiveFiles(const protocols *theProt);


    // NETMAIL.CPP
    void buildaddress(ModemConsoleE W);
    Bool asknode(char *rnode, char *rregion, char *raddress);
    void parseNetAddress(const char *str, char *u, char *n, char *r, char *c);


    // NETMSGI.CPP
    Bool ReadMsgFl(r_slot room, const char *filename, FILE *file, int *newMsgs, int *expired, int *duplicate, int *error,
            const LogEntry *FromNode);


    // NETMSGO.CPP
    int NewRoom(r_slot room, const char *filename, FILE *file);


    // NETNODE.CPP
    void shownode(const NodesCitC *nd, ModemConsoleE W);
    Bool readnode(Bool Option);


    // OUTFILE.CPP
    void dFormat(const char *string, FILE *file);
    void ddoCR(FILE *fd);
    void doChar(register char c, FILE *file);
    void dputWord(const uchar *st, FILE *file);


    // OUTOVL.CPP
    Bool outSpeech(Bool out, const char *st);
    void dospCR(void);
    void cdecl rmPrintf(const char *fmt, ...);
    void prtList(const char *item);
    void cdecl CRCRmPrintfCR(const char *fmt, ...);
    void cdecl wPrintf(ModemConsoleE W, const char *fmt, ...);
    void wDoCR(ModemConsoleE W);


    // OUTPUT.CPP
    void mFormat(const char *string);
    void doBS(void);
    void doBS(int Count);
    void doCR(void);
    void doCR(int Count);
    void doTAB(void);
    void echocharacter(const char c);
    void oChar(register char c);
    void updcrtpos(const char c);
    void cdecl mPrintf(const char *fmt, ...);
    void cdecl CRmPrintf(const char *fmt, ...);
    void cdecl mPrintfCR(const char *fmt, ...);
    void cdecl CRmPrintfCR(const char *fmt, ...);
    int getWord(uchar *dest, register const uchar *source, int offset, int lim);
    void putWord(const uchar *st);
    void cdecl DebugOut(const char *fmt, ...);
    void cdecl DebugOut(int DbMsgIndex, ...);
    void ResetOutputControl(void);


#ifdef WINCIT
    // OUTWIN.CPP
    Bool WinStatCon(void);
    int WinGetCon(void);
    Bool WinPutChar(char Ch, int Attr, Bool phys);
    void WinPutString(int Row, const char *Str, int Attr, Bool phys);
    void curson(void);
    void cursoff(void);
    void clearline(int row, int attr);
#endif


    // PORT.CPP
    void Initport(void);


    // READINFO.CPP
    void showinfo(directoryinfo *files, CITHANDLE FileInfo);
    void ReadInfoFile(Bool ask, OldNewPick which);


    // READLOG.CPP
    void ReadUserlog(Bool revOrder, OldNewPick which, long howMany);

    // ROOM2.CPP
    void CreateRoomSummary(char *buffer, r_slot RoomIndex);
    char *MakeRoomName(r_slot RoomIndex, char *Buffer);
    r_slot PartialRoomExists(const char *SearchString, r_slot StartRoom, Bool IgnoreHalls);
    void checkdir(void);


    // ROOM3.CPP
    void ListAllRooms(Bool ThisHallOnly, Bool CanGetToRoom, Bool ShowNetID);
    void listRooms(ulong what);
    void ShowRoomStatus(void);
    void printroomVer(r_slot room, Bool numMess);
    void stepRoom(Bool direction);
    void unGotoRoom(void);
    void gotoRoom(const char *roomname, Bool skiproom, Bool partial, Bool mailScan);
    void AideJumpToAideAndMaintenance(void);
    void ChangeRooms(r_slot NewRoom);


    // ROOMEDIT.CPP
    Bool inExternal(const char *key, const char *str);
    Bool AideEditRoom(void);
    void DestroyAllReferencesToRoom(r_slot TheRoom);
    void killempties(void);
    Bool killroom(const char *name);
    Bool AideMoveRoom(int offset, Bool inHall);
    void AideKillRoom(void);


    // ROOMMAKE.CPP
    void EnterRoom(Bool GroupOnly);


    // ROOMSHOW.CPP
    void dumpRoom(Bool infoLineAndAutoApp);
    void showRoomDescription(void);


    // SETINFO.CPP
    void AideSetFileInfo(void);


    // SPELLCHK.CPP
    void spellCheck(char *buf, int lim);


    // STATUS.CPP
    void ReadStatus(void);
#if defined(__BORLANDC__) && (VERSION != RELEASE)
    void CheckArrayPtr(const void *p, const void *Check, const char *Name, int i);
    void CheckLLPtr(const void *LL, const void *Check, const char *Name);
    void CheckDiscardablePtr(const discardable *D, const void *Check, const char *Name);
#endif

    // TERM.CPP
    void setdefaultTerm(TermTypeE NewType);
    void autoansi(void);
    void localTermCap(const char *c);
    void askAttributes(LogEntry1 *EditLog);
    void termCap(const char *c);


    // TIMEDATE.CPP
    const char *diffstamp(time_t oldtime);
    void netpause(register int ptime);
    void twirlypause(register int ptime);
    Bool CheckIdleUserTimeout(Bool InOutput);


    // TIMEOVL.CPP
    void changeDate(void);


    // TRAP.CPP
    void SaveAideMess(const char *group = NULL, r_slot RoomNumber = AIDEROOM);
    void cdecl amPrintf(const char *fmt, ...);
    void amZap(void);


    // UIMISC.CPP
    void BadMenuSelection(int C);
    int DoMenuPrompt(const char *Text1, const char *Text2);
    Bool AskHallName(char *Buffer, h_slot Dfault);
    Bool AskHallName(char *Buffer, const char *Dfault);
    Bool AskHallName(char *Buffer, h_slot Dfault, const char *Prompt);
    Bool AskHallName(char *Buffer, const char *Dfault, const char *Prompt);
    Bool AskHallName(char *Buffer, const char *Dfault, Bool ListOnlyAccessible);
    Bool AskHallName(char *Buffer, const char *Dfault, const char *Prompt, Bool ListOnlyAccessible);
    Bool AskRoomName(char *Buffer, r_slot Dfault);
    Bool AskRoomName(char *Buffer, const char *Dfault);
    Bool AskRoomName(char *Buffer, r_slot Dfault, const char *Prompt);
    Bool AskRoomName(char *Buffer, const char *Dfault, const char *Prompt);

    Bool AskGroupName(char *Buffer);
    Bool AskGroupName(char *Buffer, Bool ListOnlyInGroup);
    Bool AskGroupName(char *Buffer, const char *Dfault);
    Bool AskGroupName(char *Buffer, const char *Dfault, Bool ListOnlyInGroup);
    Bool AskGroupName(char *Buffer, g_slot Dfault, const char *Prompt);
    Bool AskGroupName(char *Buffer, g_slot Dfault, const char *Prompt, int Len);
    Bool AskGroupName(char *Buffer, const char *Dfault, const char *Prompt, int Len = LABELSIZE,
            Bool ListOnlyInGroup = FALSE);

    Bool AskUserName(char *Buffer, const LogEntry *Dfault);
    Bool AskUserName(char *Buffer, const char *Dfault);
    Bool AskUserName(char *Buffer);
    Bool AskUserName(char *Buffer, const char *Dfault, const char *Prompt, int Len = LABELSIZE, Bool *GetStringReturnVal = NULL, Bool ListOnlyLoggedIn = FALSE, Bool fingerYes = FALSE);


    // USEROUT.CPP
    void GiveRoomPrompt(Bool JustDisplayedEvent);
    void SetOutOK(void);


    // VIEWLOG.CPP
    void ShowToggles(LogEntry1 *Log1);
    void SysopShowUser(void);
    void showconfig(l_slot LogSlot, Bool showuser);
    void ReadConfiguration(void);


    // WINDOVL.CPP
    void cls(int whatToDo);
    Bool save_screen(void);
    void restore_screen(void);
    void getScreenSize(uint *Cols, uint *Rows);
    void ClearToEndOfLine(void);
    void scroll(int row, int howmany, uchar attr);
    void readcursorsize(uchar *cursorstart, uchar *cursorend);
    void outPhys(Bool phys);


    // WINDOW.CPP
    void ansi(char c);
    void position(int row, int column);


    // ZIPFILE.CPP
    void ReadArchive(OldNewPick which);


    // ZMODEM.CPP
    Bool got_ESC(void);
    void show_loc(ulong l, uint w);
    void cdecl z_status(const char *fmt, ...);
    void cdecl z_message(const char *fmt, ...);
    void throughput(int opt, ulong bytes);
    void show_debug_name(int index);
    int Zmodem_Send_Batch(const char *filepath, uint baud);
    int got_error(const char *string1, const char *string2);
    int Z_GetByte(int tenths);
    int Z_GetHeader(byte *hdr);
    int _Z_GetBinaryHeader(register uchar *hdr);
    int _Z_32GetBinaryHeader(register uchar *hdr);
    int _Z_GetHexHeader(register uchar *hdr);
    int Send_Zmodem(char *fname, int fsent);
    void Z_SendHexHeader(uint type, register uchar *hdr);
    int _Z_GetHex(void);
    int Z_GetZDL(void);
    void send_can(void);
    void Z_PutString(register uchar *s);
    int _Z_TimedRead(void);
    void ZS_SendBinaryHeader(uint type, register byte * hdr);
    void ZS_32SendBinaryHeader(uint type, register byte * hdr);
    void ZS_SendData(register byte * buf, int length, uint frameend);
    void ZS_32SendData(register byte * buf, int length, uint frameend);
    void ZS_SendByte(register byte c);
    int ZS_SendFile(int blen);
    int ZS_SendFileData(void);
    void ZS_EndSend(void);
    int ZS_GetReceiverInfo(void);
    int ZS_SyncWithReceiver(int num_errs);
    int RZ_ReceiveData(register byte * buf, register int length);
    int RZ_32ReceiveData(register byte * buf, register int length);
    int RZ_InitReceiver(void);
    int RZ_ReceiveFile(FILE * xferinfo);
    int RZ_SaveToDisk(long *rxbytes);
    int get_Zmodem(const char *rcvpath, FILE * xferinfo);
    int RZ_ReceiveBatch(FILE * xferinfo);
    int RZ_GetHeader(void);
    int Zmodem_Receive_Batch(const char *filepath, uint baud);
    void RZ_AckBibi(void);


#ifdef MULTI
    };
#endif

#ifdef WINCIT
#define onConsole   ((CommPort->GetType() == CT_LOCAL) || (OC.whichIO == CONSOLE))
#else
#define onConsole   (OC.whichIO == CONSOLE)
#endif

#define CTDL
#include "rlm.h"
#ifndef WINCIT
#include "global.h"
#endif


#include "keycodes.h"
#include "proto.h"

#ifndef WINCIT
#include "spawno.h"

class CommunicationPortC
    {
    Bool Initialized;

    ulong Transmitted;              // # characters transmitted
    ulong Received;                 // # characters received

    uchar BeenSent[KEYBUFSIZE];
    int SentStart, SentEnd, NumBeenSent;

    uchar BeenReceived[KEYBUFSIZE];
    int RecStart, RecEnd, NumBeenReceived;

    Bool Dirty;

    virtual uchar getChar(void) = 0;
    virtual void putChar(uchar Out) = 0;

    ComTypeE ComType;

protected:
    PortSpeedE Speed;
    ModemSpeedE ModemSpeed;
    int PortNumber;

public:
    label PortName;                 // Human-readable name of port

    Bool UsingCompression;
    Bool UsingCorrection;

    CommunicationPortC(ComTypeE CT)
        {
        Initialized = FALSE;
        Speed = PS_300;
        ModemSpeed = MS_300;
        UsingCompression = UsingCorrection = FALSE;
        Transmitted = Received = 0;
        SentStart = SentEnd = RecStart = RecEnd = NumBeenSent = NumBeenReceived = 0;
        Dirty = FALSE;
        ComType = CT;

        }

    ComTypeE GetType(void) const
        {
        return (ComType);
        }

    Bool IsDirty(void) const
        {
        return (Dirty);
        }

    void SetDirty(Bool NewDirt = TRUE)
        {
        Dirty = NewDirt;
        }

    Bool IsInitialized(void) const
        {
        return (Initialized);
        }

    void SetInitialized(Bool NewInit = TRUE)
        {
        Initialized = NewInit;
        }

    virtual InputReadyE CheckInputReady(void) = 0;
    virtual void FlushOutput(void) = 0;
    virtual Bool HaveConnection(void) const = 0;

    virtual void Ping(void) {}

    Bool IsInputReady(void);

    uchar Input(void)
        {
        uchar ToRet = getChar();
        Received++;

        SetDirty();

        BeenReceived[RecEnd] = ToRet;
        RecEnd = (RecEnd + 1) % KEYBUFSIZE;
        if (RecEnd == RecStart)
            {
            RecStart = (RecStart + 1) % KEYBUFSIZE;
            }
        else
            {
            NumBeenReceived++;
            }

        return (ToRet);
        }

    int GetNumInSentBuffer(void) const
        {
        return (NumBeenSent);
        }

    int GetNumInReceivedBuffer(void) const
        {
        return (NumBeenReceived);
        }

    virtual void Init(void) = 0;
    virtual void Deinit(void) = 0;
    virtual void BreakConnection(void) = 0;

    void ResetReceived(void)
        {
        Received = 0;
        }

    void ResetTransmitted(void)
        {
        Transmitted = 0;
        }

    void IncReceived(void)
        {
        Received++;
        }

    void IncTransmitted(void)
        {
        Transmitted++;
        }

    ulong GetReceived(void) const
        {
        return (Received);
        }

    ulong GetTransmitted(void) const
        {
        return (Transmitted);
        }

    void Output(uchar Out);

    void FlushInput(void);

    uchar GetFromSentBuffer(int Num) const
        {
        return (BeenSent[(SentStart + Num - 1) % KEYBUFSIZE]);
        }

    uchar GetFromReceivedBuffer(int Num) const
        {
        return (BeenReceived[(RecStart + Num - 1) % KEYBUFSIZE]);
        }

    virtual void SetSpeed(PortSpeedE NewSpeed) = 0;

    PortSpeedE GetSpeed(void) const
        {
        return (Speed);
        }

    ModemSpeedE GetModemSpeed(void) const
        {
        return (ModemSpeed);
        }

    void SetModemSpeed(ModemSpeedE MSpeed)
        {
        ModemSpeed = MSpeed;
        }

    void OutString(const char *String, int Pace = 0);

    virtual long GetTransmitSpeed(void)
        {
        return (connectbauds[ModemSpeed]);
        }

    virtual long GetReceiveSpeed(void)
        {
        return (connectbauds[ModemSpeed]);
        }

    int GetPortNumber(void)
        {
        return (PortNumber);
        }

    virtual void Pause(void) {}
    virtual void Resume(void) {}
    };


class NoComPortC: public CommunicationPortC
    {
    static int ActiveCounter;
    static int TotalCounter;

    virtual uchar getChar(void)                     { return (0); }
    virtual void putChar(uchar Out)                 {}

public:
    NoComPortC(void) : CommunicationPortC(CT_LOCAL)
        {

        sprintf(PortName, "Local%d", ++TotalCounter);
        ActiveCounter++;
        }

    virtual ~NoComPortC(void)                       { TotalCounter--; }
    int GetCounter(void)                            { return (TotalCounter); }

    virtual InputReadyE CheckInputReady(void)       { return (IR_NONE); }
    virtual void FlushOutput(void)                  {}
    virtual Bool HaveConnection(void) const         { return (FALSE); }
    virtual void Init(void)                         {}
    virtual void Deinit(void)                       {}
    virtual void BreakConnection(void)              {}
    virtual void SetSpeed(PortSpeedE NewSpeed)      { Speed = NewSpeed; }
    };

class SerialPortC : public CommunicationPortC
    {
    Bool Disabled;                  // Or is it differently abled?

    virtual uchar getChar(void)
        {
        assert(getrs);
        return ((uchar) (*getrs)());
        }


    virtual void putChar(uchar Out)
        {
        assert(putrs);
        (*putrs)(Out);
        }

public:

    SerialPortC(void) : CommunicationPortC(CT_SERIAL)
        {
        Disabled = TRUE;
        }

    void Disable(void)
        {
        Disabled = TRUE;
        }

    void Enable(void)
        {
        Disabled = FALSE;
        }

    Bool IsDisabled(void)
        {
        return (Disabled);
        }

    Bool IsRingIndicator(void) const
        {
        assert(ringstatrs);
        return ((*ringstatrs)());
        }

    virtual InputReadyE CheckInputReady(void)
        {
        assert(statrs);
        return ((*statrs)() ? IR_READY : IR_NONE);
        }

    virtual void FlushOutput(void)
        {
        assert(flushoutrs);
        (*flushoutrs)();
        }

    void RaiseDtr(void) const
        {
        assert(dtrrs);
        (*dtrrs)(1);
        }

    Bool HaveConnection(void) const
        {
        assert(carrstatrs);
#ifdef SOFTPC
        return (FALSE);
#else
        return ((*carrstatrs)());
#endif
        }


    virtual void Deinit(void)
        {
        SetInitialized(FALSE);
        assert(deinitrs);
        (*deinitrs)();
        }

    virtual void BreakConnection(void);

    void DropDtr(void);

    void CycleDTR(void);

    virtual void SetSpeed(PortSpeedE NewSpeed);

    virtual void Init(void)
        {
        //??SetInitialized();
        SetSpeed(GetSpeed());
        }
    };
#endif

#ifndef WINCIT
tiEXT   SerialPortC     *CommPort;      // The serial port, of course.
#endif



#ifndef __BORLANDC__
    // copyright violation as you watch.
inline int random(int __num) { return(int)(((long)rand()*__num)/((uint)RAND_MAX+1)); }
#endif
