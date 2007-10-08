// --------------------------------------------------------------------------
// Citadel: ReadCfg.CPP
//
// The CONFIG.CIT reading code.

#include "ctdl.h"
#pragma hdrstop

#define READCFG_CPP
#include "config.h"
#include "auxtab.h"
#include "libovl.h"
#include "log.h"
#include "readcfg.h"
#include "net.h"
#include "meminit.h"
#include "hall.h"
#include "scrlback.h"
#include "cfgfiles.h"
#include "miscovl.h"
#include "extmsg.h"

#ifdef WINCIT
static const char *ltoac(int wow)
    {
    static label Buffer;
    sprintf(Buffer, "%d", wow);
    return (Buffer);
    }

// taken from msdn q124103
HWND GetConsoleHwnd(void) { 

    #define MY_BUFSIZE 1024 // buffer size for console window titles
    HWND hwndFound;         // this is what is returned to the caller
    char pszNewWindowTitle[MY_BUFSIZE]; // contains fabricated WindowTitle
    char pszOldWindowTitle[MY_BUFSIZE]; // contains original WindowTitle

    // fetch current window title

    GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

    // format a "unique" NewWindowTitle

    wsprintf(pszNewWindowTitle,"%d/%d",
                GetTickCount(),
                GetCurrentProcessId());

    // change current window title

    SetConsoleTitle(pszNewWindowTitle);

    // ensure window title has been updated

    Sleep(40);

    // look for NewWindowTitle

    hwndFound=FindWindow(NULL, pszNewWindowTitle);

    // restore original window title

    SetConsoleTitle(pszOldWindowTitle);

    return(hwndFound);

} 
#endif

// --------------------------------------------------------------------------
// Contents
//
// readconfig()         reads CONFIG.CIT values

static discardable *readcfgd;
static config_keywords *keywords;

#define keyword(nmbr) ((const char **) readcfgd->aux)[(uint)(nmbr)]

#ifdef WINCIT
class Win32_Con
    {
    HANDLE ih, oh;
    BOOL Alloced;

    void open(void);

public:
	Win32_Con(void)
        {
        ih = NULL;
        oh = NULL;
        Alloced = 0;
        }

    ~Win32_Con(void)
        {
        close();
        }

    void flush(void);
    void printf(const char *fmt, ... );
    BOOL kbhit(void);
    void doccr(void);
    char getcon(void);
    void putcon(char ch);
    void close(void);
	};

void Win32_Con::open(void)
	{
	Alloced = AllocConsole();
	if (Alloced)
		{
        // if AllocConsole() does not setup the std handles we do it here
        ih = CreateFile
         (
         "CONIN$",
         GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ|FILE_SHARE_WRITE,
         NULL,
         OPEN_EXISTING,
         0,
         NULL
         );
        SetStdHandle(STD_INPUT_HANDLE, ih );
        oh = CreateFile
         (
         "CONOUT$",
         GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ|FILE_SHARE_WRITE,
         NULL,
         OPEN_EXISTING,
         0,
         NULL
         );
        SetStdHandle(STD_OUTPUT_HANDLE, oh );
        SetStdHandle(STD_ERROR_HANDLE, oh );
		SetConsoleMode(ih, 0);
		SetConsoleTitle("Citadel+ Config");
		HWND hwndCon = GetConsoleHwnd();
		SetFocus(hwndCon);	
		}
	}

void Win32_Con::flush(void)
	{
	if (!Alloced)
		{
		open();
		}
	if (Alloced)
		{
		FlushConsoleInputBuffer(ih);
		}
	}

void Win32_Con::printf(const char *fmt, ... )
	{
	char Buffer[128];
	DWORD scratch;

	va_list ap;

	va_start(ap, fmt);
	vsprintf(Buffer, fmt, ap);
	va_end(ap);

	if (!Alloced)
		{
		open();
		}
	if (Alloced)
		{
		WriteConsole(oh, Buffer, strlen(Buffer), &scratch, NULL);
		}
	}

BOOL Win32_Con::kbhit(void)
	{
	if (!Alloced)
		{
		open();
		}

	if (Alloced)
		{
		DWORD numevents = 0;
		INPUT_RECORD buffer[256];

		PeekConsoleInput(ih, buffer, 255, &numevents);

		for (DWORD i = 0; i < numevents; i++)
			{
			if ((buffer[i].EventType == KEY_EVENT) && buffer[i].Event.KeyEvent.bKeyDown
				&& buffer[i].Event.KeyEvent.uChar.AsciiChar)
				{
				return (TRUE);
				}
			}
		}

	return (FALSE);
	}

void Win32_Con::doccr(void)
	{
	DWORD scratch;

	if (!Alloced)
		{
		open();
		}
	if (Alloced)
		{
		WriteConsole(oh, "\n", 1, &scratch, NULL);
		}
	}

char Win32_Con::getcon(void)
	{
	DWORD scratch;
	char Wow = 0;

	if (!Alloced)
		{
		open();
		}
	if (Alloced)
		{
		ReadConsole(ih, &Wow, 1, &scratch, NULL);
		}
	return Wow;
	}

void Win32_Con::putcon(char ch)
	{
	DWORD scratch;

	if (!Alloced)
		{
		open();
		}
	if (Alloced)
		{
		WriteConsole(oh, &ch, 1, &scratch, NULL);
		}
	}

void Win32_Con::close(void)
	{
	if (Alloced)
		{
		FreeConsole();
		Alloced = 0;
		}
	}

class Win32_Con win32_con;

#define doccr     win32_con.doccr
#define cPrintf   win32_con.printf
#define outCon    win32_con.putcon
#endif

static void checkPaths(void)
    {
    // dlpath, transpath, temppath, homepath
    // transpath, temppath, homepath
    // temppath, homepath

    if (SameString(cfg.dlpath, cfg.transpath))
        {
        illegal(getcfgmsg(1), keyword(K_DLPATH), keyword(K_TRANSPATH));
        }
    if (SameString(cfg.dlpath, cfg.temppath))
        {
        illegal(getcfgmsg(1), keyword(K_DLPATH), keyword(K_TEMPPATH));
        }
    if (SameString(cfg.dlpath, cfg.homepath))
        {
        illegal(getcfgmsg(1), keyword(K_DLPATH), keyword(K_HOMEPATH));
        }

    if (SameString(cfg.transpath, cfg.temppath))
        {
        illegal(getcfgmsg(1), keyword(K_TRANSPATH), keyword(K_TEMPPATH));
        }
    if (SameString(cfg.transpath, cfg.homepath))
        {
        illegal(getcfgmsg(1), keyword(K_TRANSPATH), keyword(K_HOMEPATH));
        }

    if (SameString(cfg.temppath, cfg.homepath))
        {
        illegal(getcfgmsg(1), keyword(K_TEMPPATH), keyword(K_HOMEPATH));
        }
    }

static Bool getKeyword(ConfigCitKeywordsE kw)
    {
    printf("3..\n");
    int maxval = LABELSIZE, minval = 1, c;
    char input[80], dflt[80];

    doccr();
    cPrintf(getcfgmsg(3), keyword(kw));
    doccr();
    doccr();

    dflt[0] = 0;

    switch (kw)
        {
        case K_NODECOUNTRY:
            {
            cPrintf(getcfgmsg(4));  doccr();
            cPrintf(getcfgmsg(5));  doccr();
            cPrintf(getcfgmsg(6));  doccr();
            cPrintf(getcfgmsg(7));  doccr();
            break;
            }

        case K_NODENAME:
            {
            cPrintf(getcfgmsg(9));  doccr();
            cPrintf(getcfgmsg(10)); doccr();
            cPrintf(getcfgmsg(11)); doccr();
            cPrintf(getcfgmsg(12)); doccr();
            break;
            }

        case K_NODEREGION:
            {
            cPrintf(getcfgmsg(14)); doccr();
            cPrintf(getcfgmsg(15)); doccr();
            cPrintf(getcfgmsg(16)); doccr();
            cPrintf(getcfgmsg(17)); doccr();
            cPrintf(getcfgmsg(18)); doccr();
            break;
            }

        case K_SYSOP:
            {
            cPrintf(getcfgmsg(20)); doccr();
            cPrintf(getcfgmsg(21)); doccr();
            cPrintf(getcfgmsg(22)); doccr();
            break;
            }

        case K_NODEPHONE:
            {
            cPrintf(getcfgmsg(24)); doccr();
            cPrintf(getcfgmsg(25)); doccr();
            cPrintf(getcfgmsg(26)); doccr();
            cPrintf(getcfgmsg(27)); doccr();
            cPrintf(getcfgmsg(28)); doccr();
            cPrintf(getcfgmsg(29)); doccr();
            break;
            }

        case K_HOMEPATH:
            {
            cPrintf(getcfgmsg(31)); doccr();
            cPrintf(getcfgmsg(32)); doccr();
            cPrintf(getcfgmsg(33)); doccr();

            maxval = 64;
#ifdef VISUALC
            GetCurrentDirectory(79, dflt);
#else
            dflt[0] = (char) (getdisk() + 'A');
            dflt[1] = ':';
            dflt[2] = '\\';
            getcurdir(0, dflt+3);
#endif
            break;
            }

        case K_ADDRESS:
            {
            cPrintf(getcfgmsg(35)); doccr();
            cPrintf(getcfgmsg(36)); doccr();
            cPrintf(getcfgmsg(37)); doccr();
            cPrintf(getcfgmsg(38)); doccr();
            maxval = 8;
            break;
            }

        case K_MDATA:
            {
            cPrintf(getcfgmsg(40)); doccr();
            cPrintf(getcfgmsg(41)); doccr();
            cPrintf(getcfgmsg(42)); doccr();
            cPrintf(getcfgmsg(43)); doccr();
            maxval = 4;
            break;
            }

        default:
            {
            crashout(getcfgmsg(47), keyword(kw));
            break;
            }
        }

	Bool try_again = TRUE;
    while (try_again)
        {
        printf("4...\n");
        memset(input, 0, sizeof(input));
        doccr();
        if (*dflt)
            {
            cPrintf(getcfgmsg(46), keyword(kw), dflt);
            }
        else
            {
            cPrintf(getcfgmsg(45), keyword(kw));
            }
        outCon(' ');

        for (c = 0; c != '\r' && c != '\n';)
            {
#ifdef WINCIT
            if (win32_con.kbhit())
                {
                c = win32_con.getcon();
#else
            if ((*statcon)())
                {
                c = (*getcon)();
#endif
                if (c < 256)
                    {
                    if (c == '\r' || c == '\n')
                        {
                        doccr();
                        }
                    else if (c == '\b')
                        {
                        if (*input)
                            {
                            *(strchr(input, 0) - 1) = 0;
                            outCon('\b');
                            outCon(' ');
                            outCon('\b');
                            }
                        }
                    else if (keywords[kw].type == CK_STR || keywords[kw].type == CK_PATH || kw == K_ADDRESS)
                        {
                        if ((int) strlen(input) < maxval)
                            {
                            *(strchr(input, 0) + 1) = 0;
                            *strchr(input, 0) = (char) c;
                            outCon((char) c);
                            }
                        }
                    else
                        {
                        // assume int
                        if (!*input)
                            {
                            input[1] = 0;
                            input[0] = (char) c;
                            outCon((char) c);
                            }
                        }
                    }
                }
            }

        if (!*input)
            {
            if (*dflt)
                {
                strcpy(input, dflt);
                }
            else
                {
                return (FALSE);
                }
            }
        normalizeString(input);

		try_again = FALSE;

        if (keywords[kw].type == CK_STR || keywords[kw].type == CK_PATH)
            {
            strcpy((char *) CCVL[kw], input);
            }
        else if (kw == K_ADDRESS)
            {
            if (!isaddress(input))
                {
                try_again = TRUE;
                }

            parse_address(input, cfg.alias, cfg.locID, FALSE);
            makeaddress(cfg.alias, cfg.locID, cfg.Address);
            }
        else
            {
            // assume int
            if ((* (int *) CCVL[kw] = atoi(input)) < minval || (*(int *) CCVL[kw]) > maxval)
                {
                try_again = TRUE;
                }
            }
        }

    return (TRUE);
    }

static void changeOrMakeDir(const char *check, char *store, const char *what)
    {
    if (strlen(check) > 63)
        {
        illegal(getmsg(102), what, 64);
        }

    if (!changedir(check))
        {
        cPrintf(getcfgmsg(48), what, check);
        doccr();
        cPrintf(pctss, getcfgmsg(49));

        int i;
        for (i = 0; i != toupper(getcfgmsg(205)[0]) && i != toupper(getcfgmsg(206)[0]);)
            {
#ifdef WINCIT
            if (win32_con.kbhit())
                {
                i = toupper(win32_con.getcon());
#else
            if ((*statcon)())
                {
                i = toupper((*getcon)());
#endif
                }
            }

        if (i == toupper(getcfgmsg(205)[0]))
            {
            cPrintf(getcfgmsg(205));
            doccr();

            if (mkdir(check) == 0)
                {
                if (!changedir(check))
                    {
                    illegal(getcfgmsg(52), check);
                    }
                else
                    {
                    doccr();
                    cPrintf(getcfgmsg(51));
                    doccr();
                    doccr();
                    }
                }
            else
                {
                illegal(getcfgmsg(50), check);
                }
            }
        else
            {
#ifdef WINCIT
            win32_con.close();
#endif
            illegal(ns);
            }
        }

    if (store)
        {
#ifdef VISUALC
        char buf[80];
#endif
        store[0] = check[0];
        store[1] = check[1];
        store[2] = check[2];

#ifdef VISUALC
        GetCurrentDirectory(79, buf);
        strcpy(store + 3, buf + 3);
#else
        getcurdir(0, store + 3);
#endif

        strupr(store);
        }
    }

static void SetDefault(ConfigCitKeywordsE Keyword)
    {
    switch (keywords[Keyword].type)
        {
        case CK_UCHAR:
        case CK_UCHARHEX:
            {
            if (CCVL[Keyword])
                {
                *(uchar *) CCVL[Keyword] = (uchar) keywords[Keyword].dfault;
                }

            break;
            }

        case CK_PATH:
        case CK_STR:
            {
            if (CCVL[Keyword])
                {
                if (keywords[Keyword].dfault)
                    {
                    strcpy((char *) CCVL[Keyword], getcfgmsg((int) keywords[Keyword].dfault));
                    }
                else
                    {
                    *(char *) CCVL[Keyword] = 0;
                    }
                }

            break;
            }

        case CK_SHORT:
            {
            if (CCVL[Keyword])
                {
                *(short *) CCVL[Keyword] = (short) keywords[Keyword].dfault;
                }

            break;
            }

        case CK_INT:
        case CK_UINT:
        case CK_BOOLEAN:
            {
            if (CCVL[Keyword])
                {
                *(uint *) CCVL[Keyword] = (uint) keywords[Keyword].dfault;
                }

            break;
            }

        case CK_LONG:
            {
            if (CCVL[Keyword])
                {
                *(long *) CCVL[Keyword] = (long) keywords[Keyword].dfault;
                }

            break;
            }

        case CK_SPECIAL:
            {
            switch (Keyword)
                {
#ifdef WINCIT
                case K_MDATA:
                    {
                    cfg.mdata = 0;
                    break;
                    }

                case K_TELNET:
                    {
                    cfg.MaxTelnet = 0;
                    cfg.TelnetPort = 23;
                    break;
                    }

                case K_CONSOLESIZE:
                    {
                    cfg.ConsoleRows = 25;
                    cfg.ConsoleCols = 80;
                    break;
                    }
#endif
                case K_LOGIN:
                    {
                    cfg.l_closedsys = FALSE;
                    cfg.l_verified = TRUE;
                    cfg.l_sysop_msg = FALSE;
                    cfg.l_create = TRUE;
                    break;
                    }

                case K_TWIT_FEATURES:
                    {
                    cfg.msgNym = FALSE;
                    cfg.borders = FALSE;
                    cfg.titles = TRUE;
                    cfg.nettitles = TRUE;
                    cfg.surnames = TRUE;
                    cfg.netsurname = TRUE;
                    cfg.entersur = TRUE;
                    cfg.colors = TRUE;
                    cfg.mci = TRUE;
                    break;
                    }

                case K_UP_HOURS:
                    {
                    for (int j = 0; j < 24; j++)
                        {
                        cfg.uphours[j] = TRUE;
                        }

                    break;
                    }

                case K_UP_DAYS:
                    {
                    for (int j = 0; j < 7; j++)
                        {
                        cfg.updays[j] = TRUE;
                        }

                    break;
                    }

#ifndef WINCIT
                case K_SCREENSAVE:
                    {
                    cfg.fucking_stupid = FALSE;
                    cfg.really_fucking_stupid = FALSE;
                    cfg.really_really_fucking_stupid = FALSE;
                    cfg.LockOnBlank = FALSE;
                    cfg.turboClock = 0;
                    cfg.screensave = 0;
                    break;
                    }
#endif

                case K_MESSAGE_NYM:
                    {
                    label L;

                    strcpy(cfg.Lmsg_nym, MakeItLower(getcfgmsg(53), L));
                    strcpy(cfg.Lmsgs_nym, MakeItLower(getcfgmsg(54), L));
                    strcpy(cfg.Umsg_nym, MakeItUpper(getcfgmsg(53), L));
                    strcpy(cfg.Umsgs_nym, MakeItUpper(getcfgmsg(54), L));
                    strcpy(cfg.msg_done, getcfgmsg(55));
                    break;
                    }

                case K_CREDIT_NYM:
                    {
                    label L;

                    strcpy(cfg.Lcredit_nym, MakeItLower(getcfgmsg(56), L));
                    strcpy(cfg.Lcredits_nym, MakeItLower(getcfgmsg(57), L));
                    strcpy(cfg.Ucredit_nym, MakeItUpper(getcfgmsg(56), L));
                    strcpy(cfg.Ucredits_nym, MakeItUpper(getcfgmsg(57), L));
                    break;
                    }

#ifndef WINCIT
                case K_SCROLL_BACK:
                    {
                    cfg.scrollSize = 0;
                    cfg.scrollTimeOut = 0;
                    cfg.scrollColors = 0;
                    break;
                    }
#endif

                case K_TWIRLY:
                    {
                    strcpy(cfg.twirly, getcfgmsg(266));
#ifdef WINCIT
                    cfg.twirlypause = 25;
#else
                    cfg.twirlypause = 10;
#endif
                    break;
                    }

                case K_TRAP:
                    {
                    for (int j = 0; j < 19; j++)
                        {
                        cfg.trapit[j] = TRUE;
                        }

                    break;
                    }

                case K_FUELBAR:
                    {
                    cfg.fuelbarempty = 249;
                    cfg.fuelbarfull = 35;
                    break;
                    }

                case K_PRINTER:
                    {
                    cfg.printer[0] = 0;
                    cfg.printerprompt = FALSE;
                    break;
                    }

                case K_DIALMACRO:
                    {
                    for (int j = 0; j < DM_NUMKEYS; j++)
                        {
                        cfg.dialmacro[j][0] = 0;
                        }

                    break;
                    }

                case K_AIDECHATHRS:
                    {
                    for (int j = 0; j < 24; j++)
                        {
                        cfg.aideChatHours[j] = TRUE;
                        }

                    break;
                    }

                case K_WYSIWYG:
                    {
                    cfg.concolors = 0;
                    cfg.forcetermcap = FALSE;
                    cfg.wysiwyg = 0;
                    break;
                    }

                case K_DIAL_RING:
                    {
                    cfg.dialringwait = 0;
                    strcpy(cfg.dialring, getcfgmsg(267));
                    break;
                    }

                case K_POOP:
                    {
                    cfg.poopdebug = FALSE;
                    cfg.poopwow = FALSE;
                    cfg.poop = 0;
                    break;
                    }

                case K_ADDRESS:
                    {
                    cfg.alias[0] = 0;
                    cfg.locID[0] = 0;
                    break;
                    }

                case K_CHATBELL:
                    {
                    cfg.LocalChatLen = 0;
                    break;
                    }

                case K_ROOM_NYM:
                    {
                    label L;

                    strcpy(cfg.Lroom_nym, MakeItLower(getcfgmsg(234), L));
                    strcpy(cfg.Lrooms_nym, MakeItLower(getcfgmsg(235), L));
                    strcpy(cfg.Uroom_nym, MakeItUpper(getcfgmsg(234), L));
                    strcpy(cfg.Urooms_nym, MakeItUpper(getcfgmsg(235), L));
                    break;
                    }

                case K_HALL_NYM:
                    {
                    label L;

                    strcpy(cfg.Lhall_nym, MakeItLower(getcfgmsg(236), L));
                    strcpy(cfg.Lhalls_nym, MakeItLower(getcfgmsg(237), L));
                    strcpy(cfg.Uhall_nym, MakeItUpper(getcfgmsg(236), L));
                    strcpy(cfg.Uhalls_nym, MakeItUpper(getcfgmsg(237), L));
                    break;
                    }

                case K_USER_NYM:
                    {
                    label L;

                    strcpy(cfg.Luser_nym, MakeItLower(getcfgmsg(238), L));
                    strcpy(cfg.Lusers_nym, MakeItLower(getcfgmsg(239), L));
                    strcpy(cfg.Uuser_nym, MakeItUpper(getcfgmsg(238), L));
                    strcpy(cfg.Uusers_nym, MakeItUpper(getcfgmsg(239), L));
                    break;
                    }

                case K_GROUP_NYM:
                    {
                    label L;

                    strcpy(cfg.Lgroup_nym, MakeItLower(getcfgmsg(240), L));
                    strcpy(cfg.Lgroups_nym, MakeItLower(getcfgmsg(241), L));
                    strcpy(cfg.Ugroup_nym, MakeItUpper(getcfgmsg(240), L));
                    strcpy(cfg.Ugroups_nym, MakeItUpper(getcfgmsg(241), L));
                    break;
                    }

                case K_NEWUSERQUESTIONS:
                    {
                    // Ask no questions by default.
                    cfg.NUQuest[0] = NUQ_NUMQUEST;
                    break;
                    }

                case K_FILTER:
                    {
                    cfg.Psycho = TRUE;
                    cfg.Backout = FALSE;
                    cfg.Mmmm = FALSE;
                    break;
                    }

                case K_CHATLOOP:
                    {
                    cfg.ringlimit = 30;
                    break;
                    }
                }

            break;
            }
        }
    }

// --------------------------------------------------------------------------
// readconfig(): Reads CONFIG.CIT values.
//
// Input:
//  int ignore: 0 CTDL -C (don't ignore anything)
//              1 CTDL -E (ignore some things)
//              2 .Sysop Enter CONFIG.CIT (.SEC) (ignore more things)
//
// Return value:
//  TRUE: All went well.
//  FALSE: All didn't go well.

#ifdef WINCIT
int readconfig(TermWindowC *TW, int ignore)
#else
int readconfig(void *, int ignore)
#endif
    {
    // Verify values used in CTDLDATA.DEF are still accurate
    assert(PS_2400 == 3);
    assert(PS_NUM == 10);
    assert(MS_NUM == 19);
    assert(LABELSIZE == 30);
    assert(BORDERSIZE == 80);
    assert(MS_NUM == 19);

#ifdef WINCIT
    assert(sizeof(LTable) == 56);
    assert(MAS == 4294967292ul);
    assert(sizeof(directoryinfo) == 288);
    assert(sizeof(statRecord) == 68);
    assert(INT_MAX == 2147483647l);
    assert(sizeof(jumpback) == 16);
#else
    assert(sizeof(LTable) == 52);
    assert(MAS == 65532u);
    assert(sizeof(directoryinfo) == 22);
    assert(sizeof(statRecord) == 66);
    assert(INT_MAX == 32767);
    #ifdef AUXMEM
        assert(sizeof(jumpback) == 14);
    #else
        assert(sizeof(jumpback) == 12);
    #endif
#endif


    readcfgd = readData(0, CONFIGDDSTART, CONFIGDDEND);

    if (!readcfgd)
        {
        cOutOfMemory(99);
        return (FALSE);
        }

    keywords = (config_keywords *) readcfgd->next->next->next->next->next->next->data;


    FILE *fBuf;
    char line[256];
    char *words[256];
    int i, count;
    Bool found[K_NWORDS];
    int lineNo = 0;
    long file_pos;          // for multi sigs
    strList *borderList = NULL;

    Bool LoginQuestionsSet = FALSE;

#ifndef WINCIT
    if ((long) physScreen == 0xB0000000l)
        {
        // monochrome: change default colors...
        keywords[K_ATTR].dfault = 7;
        keywords[K_WATTR].dfault = 0x70;
        keywords[K_CATTR].dfault = 15;
        keywords[K_UTTR].dfault = 1;
        }
#endif

    // set defaults for all keywords
    for (i = 0; i < K_NWORDS; i++)
        {
        if (!ignore || (ignore == 1 && keywords[i].ignore < 2) || (!keywords[i].ignore))
            {
            SetDefault((ConfigCitKeywordsE) i);
            }
        }

    cfg.sig_first_pos = LONG_MAX;
    cfg.sig_current_pos = LONG_MAX;
    cfg.nodeSignature[0] = 0;

    cfg.version = CFGVER;

    for (i = 0; i < K_NWORDS; i++)
        {
        found[i] = FALSE;
        }

    if ((fBuf = fopen(citfiles[C_CONFIG_CIT], FO_R)) != NULL) // ASCII mode
        {
        for (;;)
            {
            file_pos = ftell(fBuf);

            if (fgets(line, 254, fBuf) == NULL)
                {
                break;
                }

            lineNo++;

            if (line[0] != '#')
                {
                continue;
                }

            count = parse_it(words, line);

            if (debug)
                {
                mPrintfOrUpdateCfgDlg(TW, getcfgmsg(268), words[0], ltoac(lineNo), br);
                }

            for (i = 0; i < K_NWORDS; i++)
                {
                if (SameString(words[0] + 1, keyword(i)))
                    {
                    break;
                    }
                }

            if (i == K_NWORDS)
                {
                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_CONFIG_CIT], ltoac(lineNo), words[0]);
                continue;
                }
            else
                {
                if (found[i] == TRUE && !keywords[i].allowmany)
                    {
                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(58), ltoac(lineNo), words[0]);
                    }
                else
                    {
                    found[i] = TRUE;
                    }
                }

            if (!ignore || (ignore == 1 && keywords[i].ignore < 2) || (!keywords[i].ignore))
                {
                if (i != K_DIALMACRO)
                    {
                    SetDefault((ConfigCitKeywordsE) i);
                    }

                switch (i)
                    {
#ifndef WINCIT
                    case K_SCREENSAVE:
                        {
                        cfg.screensave = atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.fucking_stupid = atoi(words[2]);

                            if (count > 3)
                                {
                                cfg.really_fucking_stupid = atoi(words[3]);

                                if (count > 4)
                                    {
                                    cfg.turboClock = (uchar) atoi(words[4]);

                                    if (count > 5)
                                        {
                                        cfg.really_really_fucking_stupid = atoi(words[5]);

                                        if (count > 6)
                                            {
                                            cfg.LockOnBlank = atoi(words[6]);
                                            }
                                        }
                                    }
                                }
                            }

                        break;
                        }

                    case K_SCROLL_BACK:
                        {
                        cfg.scrollColors = !!atoi(words[1]);
#ifdef AUXMEM
                        cfg.scrollSize = atol(words[2]);
#else
                        cfg.scrollSize = min(cfg.scrollColors ? 65530u / (conCols*2) : 65530u / conCols, atol(words[2]));
#endif
                        cfg.scrollTimeOut = atoi(words[3]);

                        if (!CreateScrollBackBuffer())
                            {
                            cPrintf(getcfgmsg(60));
#ifndef WINCIT
                            doccr();
#endif
                            }

                        break;
                        }
#endif
                    case K_FILTER:
                        {
                        cfg.Psycho = atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.Backout = atoi(words[2]);

                            if (count > 3)
                                {
                                cfg.Mmmm = atoi(words[3]);
                                }
                            }

                        break;
                        }

#ifdef WINCIT
                    case K_MDATA:
                        {
                        cfg.mdata = 0;

                        for (int Idx = 1; Idx < count; Idx++)
                            {
                            const int Port = atoi(words[Idx]);

                            if (Port)
                                {
                                cfg.mdata |= 1 << (Port - 1);
                                }
                            }

                        break;
                        }

                    case K_TELNET:
                        {
                        cfg.MaxTelnet = atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.TelnetPort = atoi(words[2]);
                            }

                        break;
                        }

                    case K_CONSOLESIZE:
                        {
                        cfg.ConsoleRows = (SHORT) atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.ConsoleCols = (SHORT) atoi(words[2]);
                            }

                        if (cfg.ConsoleCols < 80)
                            {
                            cfg.ConsoleCols = 80;
                            }

                        if (cfg.ConsoleRows < 5)    // yes, 5 is arbitrary!!!
                            {
                            cfg.ConsoleRows = 5;
                            }

                        break;
                        }
#else
                    case K_MDATA:
                        {
                        cfg.mdata = atoi(words[1]);

                        if ((cfg.mdata < 1) || (cfg.mdata > 4))
                            {
                            illegal(getcfgmsg(61));
                            }

                        break;
                        }
#endif

                    case K_POOP:
                        {
                        cfg.poop = atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.poopdebug = (atoi(words[2]) == 69);
                            if (count > 3)
                                {
                                cfg.poopwow = (atoi(words[3]) == 6969);
                                }
                            }

                        break;
                        }

                    case K_WYSIWYG:
                        {
                        cfg.wysiwyg = atoi(words[1]);

                        if (count > 2)
                            {
                            cfg.concolors = atoi(words[2]);

                            if (count > 3)
                                {
                                cfg.forcetermcap = atoi(words[3]);
                                }
                            }

                        break;
                        }

                    case K_MAXLOGTAB:
                        {
                        cfg.MAXLOGTAB = (l_slot) atoi(words[1]);

                        if (cfg.MAXLOGTAB < 1)
                            {
                            cfg.MAXLOGTAB = 1;
                            }

#ifndef WINCIT
                        if (cfg.MAXLOGTAB > (int) (MAS / sizeof(LTable)))
                            {
                            cfg.MAXLOGTAB = (int) (MAS / sizeof(LTable));
                            }
#endif

                        if (cfg.MAXLOGTAB != atoi(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.MAXLOGTAB));
                            }

                        break;
                        }

                    case K_MAXROOMS:
                        {
                        cfg.maxrooms = (r_slot) atoi(words[1]);

                        if (cfg.maxrooms % 8)
                            {
                            cfg.maxrooms = (r_slot)
                                    ((cfg.maxrooms / 8 + 1) * 8);
                            }

                        if (cfg.maxrooms < 8)
                            {
                            cfg.maxrooms = 8;
                            }

#if defined(AUXMEM) || defined(WINCIT)
                        if (cfg.maxrooms > 16376)
                            {
                            cfg.maxrooms = 16376;
                            }
#else
                        if (cfg.maxrooms > (int) (MAS / sizeof(rTable)))
                            {
                            cfg.maxrooms = (int) (MAS / sizeof(rTable) / 8 * 8);
                            }
#endif
                        if (cfg.maxrooms != atoi(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.maxrooms));
                            }
                        break;
                        }

                    case K_MAXHALLS:
                        {
                        cfg.maxhalls = (h_slot) atoi(words[1]);

                        if (cfg.maxhalls % 8)
                            {
                            cfg.maxhalls = (h_slot) ((cfg.maxhalls / 8 + 1) * 8);
                            }

                        if (cfg.maxhalls < 8)
                            {
                            cfg.maxhalls = 8;
                            }

#ifndef WINCIT
                        if (cfg.maxhalls > (int) (MAS / sizeof(HallEntry)))
                            {
                            cfg.maxhalls = (int) (MAS / sizeof(HallEntry) / 8 * 8);
                            }
#endif

                        if (cfg.maxhalls != atoi(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.maxhalls));
                            }

                        break;
                        }

                    case K_MAXGROUPS:
                        {
                        cfg.maxgroups = (g_slot) atoi(words[1]);

                        if (cfg.maxgroups % 8)
                            {
                            cfg.maxgroups = (g_slot) ((cfg.maxgroups / 8 + 1) * 8);
                            }

                        if (cfg.maxgroups < 8)
                            {
                            cfg.maxgroups = 8;
                            }

#ifdef HAHA
//citadel can handle this stuff internally, but some group slots are saved out
//as uchars, not ints. :(

//i'll fix this later
                        if (cfg.maxgroups > (int) (65532u / sizeof(groupBuffer)))
                            {
                            cfg.maxgroups = (int) (65532u / sizeof(groupBuffer) / 8 * 8);
                            }
#else
                        if (cfg.maxgroups > 256)
                            {
                            cfg.maxgroups = 256;
                            }
#endif
                        if (cfg.maxgroups != atoi(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.maxgroups));
                            }

                        break;
                        }

                    case K_MAXJUMPBACK:
                        {
                        cfg.maxjumpback = atoi(words[1]) + 1;

                        if (cfg.maxjumpback > (int) (65532u / sizeof(jumpback)))
                            {
                            cfg.maxjumpback = (int) (65532u / sizeof(jumpback));
                            }

                        if (cfg.maxjumpback != atoi(words[1]) + 1)
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.maxjumpback - 1));
                            }

                        break;
                        }

                    case K_INIT_BAUD:
                        {
                        PortSpeedE PS = digitbaud(atol(words[1]));

                        if (PS == PS_ERROR)
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(63), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                            }
                        else
                            {
                            cfg.initbaud = PS;
                            }

                        break;
                        }

                    case K_MIN_BAUD:
                        {
                        ModemSpeedE MS = digitConnectbaud(atol(words[1]));

                        if (MS == MS_ERROR)
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(63), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                            }
                        else
                            {
                            cfg.minbaud = MS;
                            }

                        break;
                        }

                    case K_MAXBORDERS:
                        {
                        cfg.maxborders = atoi(words[1]);

                        if (cfg.maxborders > (int) (65532u / 81))
                            {
                            cfg.maxborders = (int) (65532u / 81);
                            }

                        if (cfg.maxborders != atoi(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.maxborders));
                            }

                        break;
                        }

                    case K_MAXSTAT:
                        {
                        cfg.statnum = atoi(words[1]) + 1;

                        if (cfg.statnum > (int) (65532u / sizeof(statRecord)))
                            {
                            cfg.statnum = (int) (65532u / sizeof(statRecord));
                            }

                        if (cfg.statnum != atoi(words[1]) + 1)
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(cfg.statnum - 1));
                            }

                        break;
                        }

                    case K_TWIRLY:
                        {
                        if (strlen(words[1]) > LABELSIZE)
                            {
                            illegal(getmsg(102), words[0], LABELSIZE+1);
                            }

                        strcpy(cfg.twirly, words[1]);

                        if (count > 2)
                            {
#ifdef WINCIT
                            cfg.twirlypause = atoi(words[2]) * 10;
#else
                            cfg.twirlypause = atoi(words[2]);
#endif
                            }

                        break;
                        }

                    case K_NMESSAGES:
                        {
                        cfg.nmessages = (m_slot) atol(words[1]);
#ifdef AUXMEM
                        if (cfg.nmessages % MSGTABPERPAGE)
                            {
                            cfg.nmessages = (cfg.nmessages / MSGTABPERPAGE + 1) * MSGTABPERPAGE;
                            }

                        if (cfg.nmessages != (m_slot) atol(words[1]))
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(112), keyword(i), ltoac(cfg.nmessages));
                            }
#endif
                        break;
                        }

                    case K_MOREPROMPT:
                        {
                        if (strlen(words[1]) > 79)
                            {
                            illegal(getmsg(102), words[0], 80);
                            }

                        strcpy(cfg.moreprompt, words[1]);
                        stripansi(cfg.moreprompt);
                        break;
                        }

                    case K_SIGNATURE:
                        {
                        if (strlen(words[1]) > 90)
                            {
                            illegal(getmsg(102), words[0], 91);
                            }

                        if (cfg.sig_first_pos == LONG_MAX)
                            {
                            cfg.sig_first_pos = file_pos;
                            strcpy(cfg.nodeSignature, words[1]);
                            }
                        else
                            {
                            cfg.sig_current_pos = ftell(fBuf);
                            }

                        break;
                        }

                    case K_PRINTER:
                        {
                        cfg.printerprompt = FALSE;

                        if (strlen(words[1]) > 63)
                            {
                            illegal(getmsg(102), words[0], 64);
                            }

                        strcpy(cfg.printer, words[1]);

                        if (count > 2)
                            {
                            cfg.printerprompt = (Bool) atoi(words[2]);
                            }

                        break;
                        }

                    case K_TRAP:
                        {
                        for (int j = 0; j < 17; j++)
                            {
                            cfg.trapit[j] = FALSE;
                            }
EXTRA_OPEN_BRACE
                        for (int j = 1; j < count; j++)
                            {
                            char **trapkeys = ((char **) readcfgd->next->aux);
                            Bool Valid = FALSE;

                            for (int k = 0; k < T_NUMTRAP; k++)
                                {
                                char notkeyword[20];
                                sprintf(notkeyword, getcfgmsg(269), trapkeys[k]);

                                if (SameString(words[j], trapkeys[k]))
                                    {
                                    Valid = TRUE;

                                    if (k == 0)     // ALL
                                        {
                                        for (int l = 0; l < T_NUMTRAP; l++)
                                            {
                                            cfg.trapit[l] = TRUE;
                                            }
                                        }
                                    else
                                        {
                                        cfg.trapit[k] = TRUE;
                                        }
                                    }
                                else if (SameString(words[j], notkeyword))
                                    {
                                    Valid = TRUE;

                                    if (k == 0)     // ALL
                                        {
                                        for (int l = 0; l < T_NUMTRAP; l++)
                                            {
                                            cfg.trapit[l] = FALSE;
                                            }
                                        }
                                    else
                                        {
                                        cfg.trapit[k] = FALSE;
                                        }
                                    }
                                }

                            if (!Valid)
                                {
                                CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[j]);
                                }
                            }
EXTRA_CLOSE_BRACE
                        break;
                        }

                    case K_CHATBELL:
                        {
                        cfg.LocalChatLen = 0;

                        for (int j = 1; j < count; j++)
                            {
                            int code;
                            int val = abs(atoi(words[j] + 1));

                            switch (toupper(words[j][0]))
                                {
                                case 'F':
                                    {
                                    code = val;
                                    break;
                                    }

                                case 'D':
                                    {
                                    val = min(val, 5000);
                                    code = -val;
                                    break;
                                    }

                                case 'P':
                                    {
                                    val = min(val, 5000);
                                    code = -5000 - val;
                                    break;
                                    }

                                case 'M':
                                    {
                                    code = INT_MIN;
                                    break;
                                    }

                                case 'L':
                                    {
                                    code = INT_MIN + 1;
                                    break;
                                    }

                                case 'B':
                                    {
                                    code = INT_MIN + 2;
                                    break;
                                    }

                                default:
                                    {
                                    code = INT_MAX;
                                    break;
                                    }
                                }

                            if ((code != INT_MAX) && (cfg.LocalChatLen < MAXCHT))
                                {
                                cfg.LocalChat[cfg.LocalChatLen++] = code;
                                }
                            }

                        break;
                        }

                    case K_CHATLOOP:
                        {
                        cfg.ringlimit = atol(words[1]);
                        break;
                        }

                    case K_UNLOGGEDBALANCE:
                        {
                        cfg.unlogbal = 60 * atol(words[1]);
                        break;
                        }

                    case K_TWIT_FEATURES:
                        {
                        const char **twitkeys = ((const char **) readcfgd->next->next->next->next->aux);

                        for (int j = 1; j < count; j++)
                            {
                            Bool Valid, Value;
                            int k;

                            for (k = 0, Valid = FALSE; !Valid && k < TWIT_NUM; k++)
                                {
                                if (SameString(words[j], twitkeys[k]))
                                    {
                                    Valid = TRUE;
                                    Value = TRUE;
                                    }
                                else if (words[j][0] == '!' && SameString(words[j] + 1, twitkeys[k]))
                                    {
                                    Valid = TRUE;
                                    Value = FALSE;
                                    }

                                if (Valid)
                                    {
                                    switch (k)
                                        {
                                        case TWIT_MSG_NYMS:
                                            {
                                            cfg.msgNym = Value;
                                            break;
                                            }

                                        case TWIT_BORDER_LINES:
                                            {
                                            cfg.borders = Value;
                                            break;
                                            }

                                        case TWIT_TITLES:
                                            {
                                            cfg.titles = Value;
                                            break;
                                            }

                                        case TWIT_NET_TITLES:
                                            {
                                            cfg.nettitles = Value;
                                            break;
                                            }

                                        case TWIT_SURNAMES:
                                            {
                                            cfg.surnames = Value;
                                            break;
                                            }

                                        case TWIT_NET_SURNAMES:
                                            {
                                            cfg.netsurname = Value;
                                            break;
                                            }

                                        case TWIT_ENTER_TITLES:
                                            {
                                            cfg.entersur = Value;
                                            break;
                                            }

                                        case TWIT_COLORS:
                                            {
                                            cfg.colors = Value;
                                            break;
                                            }

                                        case TWIT_MCI:
                                            {
                                            cfg.mci = Value;
                                            break;
                                            }
                                        }
                                    }
                                }

                            if (!Valid)
                                {
                                CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[j]);
                                }
                            }

                        break;
                        }

                    case K_LOGIN:
                        {
                        const char **loginkeys = ((const char **) readcfgd->next->next->next->aux);

                        for (int j = 1; j < count; j++)
                            {
                            Bool Valid, Value;
                            int k;

                            for (k = 0, Valid = FALSE; !Valid && k < L_NUMLOGIN; k++)
                                {
                                if (SameString(words[j], loginkeys[k]))
                                    {
                                    Valid = TRUE;
                                    Value = TRUE;
                                    }
                                else if (words[j][0] == '!' && SameString((words[j]) + 1, loginkeys[k]))
                                    {
                                    Valid = TRUE;
                                    Value = FALSE;
                                    }

                                if (Valid)
                                    {
                                    switch (k)
                                        {
                                        case L_CLOSED:
                                            {
                                            cfg.l_closedsys = Value;
                                            break;
                                            }

                                        case L_VERIFIED:
                                            {
                                            cfg.l_verified = Value;
                                            break;
                                            }

                                        case L_SYSOP_MESSAGE:
                                            {
                                            cfg.l_sysop_msg = Value;
                                            break;
                                            }

                                        case L_NEW_ACCOUNTS:
                                            {
                                            cfg.l_create = Value;
                                            break;
                                            }

                                        default:
                                            {
                                            CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT],
													keyword(i), words[j]);
                                            break;
                                            }
                                        }
                                    }
                                }

                            if (!Valid)
                                {
                                if (SameString(words[j], getcfgmsg(264)))   // QUESTIONS - courtesy, here
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(265), citfiles[C_CONFIG_CIT], ltoac(lineNo),
											getcfgmsg(264), keyword(i));

                                    LoginQuestionsSet = TRUE;
                                    }
                                else
                                    {
                                    CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT],
											keyword(i), words[j]);
                                    }
                                }
                            }

                        break;
                        }

                    case K_DIAL_RING:
                        {
                        if (strlen(words[1]) > LABELSIZE)
                            {
                            illegal(getmsg(102), words[0], LABELSIZE+1);
                            }

                        strcpy(cfg.dialring, words[1]);

                        if (count > 2)
                            {
                            cfg.dialringwait = atoi(words[2]);
                            }

                        break;
                        }

                    case K_UP_DAYS:
                        {
                        // init days
                        for (int j = 0; j < 7; j++)
                            {
                            cfg.updays[j] = 0;
                            }
EXTRA_OPEN_BRACE
                        for (int j = 1; j < count; j++)
                            {
                            int k;
                            for (k = 0; k < 7; k++)
                                {
                                if (SameString(words[j], days[k]) || SameString(words[j], fulldays[k]))
                                    {
                                    break;
                                    }
                                }

                            if (k < 7)
                                {
                                cfg.updays[k] = TRUE;
                                }
                            else if (SameString(words[j], getcfgmsg(76))) // any
                                {
                                for (int l = 0; l < 7; ++l)
                                    {
                                    cfg.updays[l] = TRUE;
                                    }
                                }
                            else
                                {
                                CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(63), citfiles[C_CONFIG_CIT], keyword(i), words[j]);
                                }
                            }
EXTRA_CLOSE_BRACE
                        break;
                        }

                    case K_UP_HOURS:
                        {
                        // init hours
                        for (int j = 0; j < 24; j++)
                            {
                            cfg.uphours[j] = FALSE;
                            }
EXTRA_OPEN_BRACE
                        for (int j = 1; j < count; j++)
                            {
                            if (SameString(words[j], getcfgmsg(76)))
                                {
                                for (int l = 0; l < 24; l++)
                                    {
                                    cfg.uphours[l] = TRUE;
                                    }
                                }
                            else
                                {
                                int l = atoi(words[j]);

                                if (l > 23 || l < 0)
                                    {
                                    CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(63), citfiles[C_CONFIG_CIT],
											keyword(i), words[j]);
                                    }
                                else
                                    {
                                    cfg.uphours[l] = TRUE;
                                    }
                                }
                            }
EXTRA_CLOSE_BRACE
                        break;
                        }

                    case K_AIDECHATHRS:
                        {
                        // init hours
                        for (int j = 0; j < 24; j++)
                            {
                            cfg.aideChatHours[j] = FALSE;
                            }
EXTRA_OPEN_BRACE
                        for (int j = 1; j < count; j++)
                            {
                            if (SameString(words[j], getcfgmsg(76)))
                                {
                                for (int l = 0; l < 24; l++)
                                    {
                                    cfg.aideChatHours[l] = TRUE;
                                    }
                                }
                            else
                                {
                                int l = atoi(words[j]);

                                if (l > 23 || l < 0)
                                    {
                                    CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(63), citfiles[C_CONFIG_CIT],
											keyword(i), words[j]);
                                    }
                                else
                                    {
                                    cfg.aideChatHours[l] = TRUE;
                                    }
                                }
                            }
EXTRA_CLOSE_BRACE
                        break;
                        }

                    case K_ADDRESS:
                        {
                        if (!isaddress(words[1]))
                            {
                            illegal(getcfgmsg(245));
                            }

                        parse_address(words[1], cfg.alias, cfg.locID, FALSE);
                        makeaddress(cfg.alias, cfg.locID, cfg.Address);

                        break;
                        }

                    case K_MESSAGE_NYM:
                        {
                        if (words[1])
                            {
                            label L;

                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            strcpy(cfg.Lmsg_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Umsg_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lmsgs_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Umsgs_nym, MakeItUpper(words[2], L));

                                if (words[3])
                                    {
                                    if (strlen(words[3]) > LABELSIZE)
                                        {
                                        illegal(getmsg(102), words[0], LABELSIZE + 1);
                                        }

                                    strcpy(cfg.msg_done, words[3]);
                                    }
                                }
                            }

                        break;
                        }

                    case K_CREDIT_NYM:
                        {
                        if (words[1])
                            {
                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            label L;
                            strcpy(cfg.Lcredit_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Ucredit_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lcredits_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Ucredits_nym, MakeItUpper(words[2], L));
                                }
                            }

                        break;
                        }

                    case K_GROUP_NYM:
                        {
                        if (words[1])
                            {
                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            label L;
                            strcpy(cfg.Lgroup_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Ugroup_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lgroups_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Ugroups_nym, MakeItUpper(words[2], L));
                                }
                            }

                        break;
                        }

                    case K_USER_NYM:
                        {
                        if (words[1])
                            {
                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            label L;
                            strcpy(cfg.Luser_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Uuser_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lusers_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Uusers_nym, MakeItUpper(words[2], L));
                                }
                            }

                        break;
                        }

                    case K_ROOM_NYM:
                        {
                        if (words[1])
                            {
                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            label L;
                            strcpy(cfg.Lroom_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Uroom_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lrooms_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Urooms_nym, MakeItUpper(words[2], L));
                                }
                            }

                        break;
                        }

                    case K_HALL_NYM:
                        {
                        if (words[1])
                            {
                            if (strlen(words[1]) > LABELSIZE)
                                {
                                illegal(getmsg(102), words[0], LABELSIZE + 1);
                                }

                            label L;
                            strcpy(cfg.Lhall_nym, MakeItLower(words[1], L));
                            strcpy(cfg.Uhall_nym, MakeItUpper(words[1], L));

                            if (words[2])
                                {
                                if (strlen(words[2]) > LABELSIZE)
                                    {
                                    illegal(getmsg(102), words[0], LABELSIZE + 1);
                                    }

                                strcpy(cfg.Lhalls_nym, MakeItLower(words[2], L));
                                strcpy(cfg.Uhalls_nym, MakeItUpper(words[2], L));
                                }
                            }

                        break;
                        }

                    case K_BORDER:
                        {
                        if (strlen(words[1]) > 80)
                            {
                            illegal(getmsg(102), words[0], 81);
                            }

                        strList *sl = (strList *) addLL((void **) &borderList, sizeof(*sl) + strlen(words[1]));

                        if (sl)
                            {
                            strcpy(sl->string, words[1]);
                            }

                        break;
                        }

                    case K_FUELBAR:
                        {
                        cfg.fuelbarempty = (uchar) words[1][0];
                        cfg.fuelbarfull = (uchar) words[2][0];
                        break;
                        }

                    case K_DIALMACRO:
                        {
                        if (strlen(words[2]) > 79)
                            {
                            illegal(getmsg(102), words[0], 80);
                            }

                        int j;
                        for (j = 0; j < DM_NUMKEYS; j++)
                            {
                            if (SameString(words[1], ((const char **) readcfgd->next->next->next->next->next->aux)[j]))
                                {
                                break;
                                }
                            }

                        if (j < DM_NUMKEYS)
                            {
                            strcpy(cfg.dialmacro[j], words[2]);
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(77), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                            }

                        break;
                        }

                    case K_NEWUSERQUESTIONS:
                        {
                        Bool Valid = FALSE;
                        int CurrentQuestion = 0;

                        for (int j = 1; j < count; j++)
                            {
                            const char **questkeys = ((const char **) readcfgd->next->next->aux);

                            for (int k = 0; k < NUQ_NUMQUEST; k++)
                                {
                                if (SameString(words[j], questkeys[k]))
                                    {
                                    Valid = TRUE;

                                    if (CurrentQuestion >= NUQ_NUMQUEST)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(263), citfiles[C_CONFIG_CIT],
												ltoac(lineNo), keyword(i));
                                        }
                                    else
                                        {
                                        cfg.NUQuest[CurrentQuestion++] = (NewUserQuestions) k;
                                        }
                                    }
                                }

                            if (SameString(words[j], getcfgmsg(262)))   // ALL
                                {
                                Valid = TRUE;

                                for (CurrentQuestion = 0; CurrentQuestion < NUQ_NUMQUEST; CurrentQuestion++)
                                    {
                                    cfg.NUQuest[CurrentQuestion] = (NewUserQuestions) CurrentQuestion;
                                    }
                                }

                            if (!Valid)
                                {
                                CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[j]);
                                }
                            }

                        // Tie off the end so we know there are no more questions
                        cfg.NUQuest[CurrentQuestion] = NUQ_NUMQUEST;

                        break;
                        }

                    default:
                        {
                        Bool NoSet = TRUE;

                        if (CCVL[i])
                            {
                            NoSet = FALSE;

                            if (keywords[i].type == CK_STR)
                                {
                                CopyString2Buffer((char *) CCVL[i], words[1], (size_t) keywords[i].max + 1);

                                if (strlen(words[1]) > (size_t) keywords[i].max)
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getmsg(102), keyword(i), keywords[i].max + 1);
                                    }
                                }
                            else if (keywords[i].type == CK_PATH)
                                {
                                changeOrMakeDir(words[1], (char *) CCVL[i], keyword(i));
                                }
                            else if (keywords[i].type == CK_LONG)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    *(long *) CCVL[i] = atol(words[1]);

                                    if (*(long *) CCVL[i] > keywords[i].max)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                        *(long *) CCVL[i] = keywords[i].max;
                                        }
                                    else if (*(long *) CCVL[i] < keywords[i].min)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min));
                                        *(long *) CCVL[i] = keywords[i].min;
                                        }
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(long *) CCVL[i] = keywords[i].dfault;
                                    }
                                }
                            else if (keywords[i].type == CK_UINT)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    *(uint *) CCVL[i] = (uint) atol(words[1]);

                                    if (*(uint *) CCVL[i] > (uint) keywords[i].max)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                        *(uint *) CCVL[i] = (uint) keywords[i].max;
                                        }
                                    else if (*(uint *) CCVL[i] < (uint) keywords[i].min)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min));
                                        *(uint *) CCVL[i] = (uint) keywords[i].min;
                                        }
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(uint *) CCVL[i] = (uint) keywords[i].dfault;
                                    }
                                }
                            else if (keywords[i].type == CK_SHORT)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    *(short *) CCVL[i] = (short) atol(words[1]);

                                    if (*(short *) CCVL[i] > keywords[i].max)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                        *(short *) CCVL[i] = (short) keywords[i].max;
                                        }
                                    else if (*(short *) CCVL[i] < keywords[i].min)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min));
                                        *(short *) CCVL[i] = (short ) keywords[i].min;
                                        }
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(short *) CCVL[i] = (short) keywords[i].dfault;
                                    }
                                }
                            else if (keywords[i].type == CK_INT)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    *(int *) CCVL[i] = atoi(words[1]);

                                    if (*(int *) CCVL[i] > keywords[i].max)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                        *(int *) CCVL[i] = (int) keywords[i].max;
                                        }
                                    else if (*(int *) CCVL[i] < keywords[i].min)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min)); 
                                        *(int *) CCVL[i] = (int) keywords[i].min;
                                        }
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(int *) CCVL[i] = (int) keywords[i].dfault;
                                    }
                                }
                            else if (keywords[i].type == CK_UCHAR)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    *(uchar *) CCVL[i] = (uchar) atoi(words[1]);

                                    if (*(uchar *) CCVL[i] > keywords[i].max)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                        *(uchar *) CCVL[i] = (uchar) keywords[i].max;
                                        }
                                    else if (*(uchar *) CCVL[i] < keywords[i].min)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min));
                                        *(uchar *) CCVL[i] = (uchar) keywords[i].min;
                                        }
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(uchar *) CCVL[i] = (uchar) keywords[i].dfault;
                                    }
                                }
                            else if (keywords[i].type == CK_UCHARHEX)
                                {
                                *(uchar *) CCVL[i] = (uchar) strtol(words[1], NULL, 16);

                                if (*(uchar *) CCVL[i] > keywords[i].max)
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].max));
                                    *(uchar *) CCVL[i] = (uchar) keywords[i].max;
                                    }
                                else if (*(uchar *) CCVL[i] < keywords[i].min)
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(59), keyword(i), ltoac(keywords[i].min));
                                    *(uchar *) CCVL[i] = (uchar) keywords[i].min;
                                    }
                                }
                            else if (keywords[i].type == CK_BOOLEAN)
                                {
                                Bool Error;
                                Bool Value = GetBooleanFromCfgFile(words[1], &Error);

                                if (Error)
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(62), citfiles[C_CONFIG_CIT], keyword(i), words[1]);
                                    *(int *) CCVL[i] = (int) keywords[i].dfault;
                                    }
                                else
                                    {
                                    *(int *) CCVL[i] = Value;
                                    }
                                }
                            else
                                {
                                NoSet = TRUE;
                                }
                            }

                        if (NoSet)
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_CONFIG_CIT], ltoac(lineNo), words[0]);
                            }

                        break;
                        }
                    }
                }
            }

        if (debug)
            {
            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(270), getcfgmsg(2));
            }

        fclose(fBuf);
        }

    Bool Valid;
    for (i = 0, Valid = TRUE; i < K_NWORDS; i++)
        {
        printf("%li\n", i);   //q
        if (!found[i] && !keywords[i].optional)
            {
            Valid = FALSE;
            if (!getKeyword((ConfigCitKeywordsE) i))
                {
#ifdef WINCIT
                win32_con.close();
#endif
                illegal(ns);
                }
            }
        }

     printf("4.5...\n");

    changeOrMakeDir(cfg.homepath, cfg.homepath, keyword(K_HOMEPATH));

    if (!Valid)
        {
        if ((fBuf = fopen(citfiles[C_CONFIG_CIT], FO_A)) != NULL)
            {
            fprintf(fBuf, bn);

            for (i = 0; i < K_NWORDS; i++)
                {
                if (!found[i] && !keywords[i].optional)
                    {
                    if (keywords[i].type == CK_STR || keywords[i].type == CK_PATH)
                        {
                        char stuff[80];

                        fprintf(fBuf, getcfgmsg(271), keyword(i), mkDblBck((char *) CCVL[i], stuff), bn);
                        }
                    else if (i == K_ADDRESS)
                        {
                        fprintf(fBuf, getcfgmsg(271), keyword(i), cfg.Address, bn);
                        }
                    else
                        {
                        // assume int
                        fprintf(fBuf, getcfgmsg(272), keyword(i), *(int *) CCVL[i], bn);
                        }
                    }
                }

            fclose(fBuf);
            }
        else
            {
            crashout(getmsg(661), citfiles[C_CONFIG_CIT]);
            }
        }

        printf("5...");

    if ((h_slot) (((65532u / (cfg.maxrooms / 8)) / 8) * 8) < cfg.maxhalls)
        {
        illegal(getcfgmsg(65));
        }

    if (!cfg.msgpath[0])
        {
        strcpy(cfg.msgpath, cfg.homepath);
        changeOrMakeDir(cfg.msgpath, NULL, keyword(K_MSGPATH));
        }

    if (!cfg.lexpath[0])
        {
        strcpy(cfg.lexpath, cfg.homepath);
        changeOrMakeDir(cfg.lexpath, NULL, keyword(K_LEXPATH));
        }

    if (!cfg.logextdir[0])
        {
        strcpy(cfg.logextdir, cfg.homepath);
        changeOrMakeDir(cfg.logextdir, NULL, keyword(K_LOGEXTDIR));
        }

    if (!cfg.aplpath[0])
        {
        strcpy(cfg.aplpath, cfg.homepath);
        strcat(cfg.aplpath, getcfgmsg(273));
        changeOrMakeDir(cfg.aplpath, NULL, keyword(K_APPLICPATH));
        }

    if (!cfg.rlmpath[0])
        {
        strcpy(cfg.rlmpath, cfg.aplpath);
        changeOrMakeDir(cfg.rlmpath, NULL, keyword(K_RLMPATH));
        }

    if (!cfg.helppath[0])
        {
        strcpy(cfg.helppath, cfg.homepath);
        strcat(cfg.helppath, getcfgmsg(274));
        changeOrMakeDir(cfg.helppath, NULL, keyword(K_HELPPATH));
        }

    if (!cfg.temppath[0])
        {
        strcpy(cfg.temppath, cfg.homepath);
        strcat(cfg.temppath, getcfgmsg(275));
        changeOrMakeDir(cfg.temppath, NULL, keyword(K_TEMPPATH));
        }

    if (!cfg.transpath[0])
        {
        strcpy(cfg.transpath, cfg.homepath);
        strcat(cfg.transpath, getcfgmsg(276));
        changeOrMakeDir(cfg.transpath, NULL, keyword(K_TRANSPATH));
        }

    if (!cfg.dirpath[0])
        {
        strcpy(cfg.dirpath, cfg.helppath);
        changeOrMakeDir(cfg.dirpath, NULL, keyword(K_DIRPATH));
        }

    if (!cfg.dlpath[0])
        {
        strcpy(cfg.dlpath, cfg.homepath);
        strcat(cfg.dlpath, getcfgmsg(277));
        changeOrMakeDir(cfg.dlpath, NULL, keyword(K_DLPATH));
        }

    if (!cfg.trapfile[0])
        {
        strcpy(cfg.trapfile, cfg.homepath);
        strcat(cfg.trapfile, getcfgmsg(278));
        }

        printf("6...");

#ifdef AUXMEM
    if (*cfg.vmemfile && !(strchr(cfg.vmemfile, '\\') || strchr(cfg.vmemfile, ':')))
        {
        char Build[80];

        sprintf(Build, sbs, cfg.homepath, cfg.vmemfile);
        strcpy(cfg.vmemfile, Build);
        }
#endif

    if (!cfg.printer[0])
        {
        CopyStringToBuffer(cfg.printer, getcfgmsg(280));
        }

    if (!cfg.roompath[0])
        {
        strcpy(cfg.roompath, cfg.helppath);
        changeOrMakeDir(cfg.roompath, NULL, keyword(K_ROOMPATH));
        }

    if (!cfg.ScriptPath[0])
        {
        strcpy(cfg.ScriptPath, cfg.homepath);
        changeOrMakeDir(cfg.ScriptPath, NULL, keyword(K_SCRIPTPATH));
        }

        printf("7...");

    checkPaths();

    printf("8...");

    // set consoletimeout to match timeout if not set separately
    if (-1 == cfg.consoletimeout)
        {
        cfg.consoletimeout = cfg.timeout;
        }

    // set outputtimeout if not set separately
    if (-1 == cfg.OutputTimeout)
        {
        cfg.OutputTimeout = max(30, cfg.timeout);
        }

    printf("getting warmer\n");

    // deal with borders here...
    if (!ignore)
        {
        int i, n = (int) getLLCount(borderList);
        strList *sl = borderList;

        if (n > cfg.maxborders)
            {
            cfg.maxborders = min((long) n, keywords[K_MAXBORDERS].max);

            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(59), keyword(K_MAXBORDERS), ltoac(cfg.maxborders));
            }

            printf("9\n");

        makeBorders();

        printf("10\n");

        if (borderList)
            {
            int xyzzy;
            printf("11\n");
            for (xyzzy = 0; xyzzy < cfg.maxborders; xyzzy++)
                {
                if (sl)
                    {
                    strcpy(&(borders[xyzzy * 81]), sl->string);
                    sl = (strList *) getNextLL(sl);
                    }
                else
                    {
                    break;
                    }
                    printf("12\n");
                }

            printf("11.5");
            disposeLL((void **) &borderList);
            printf("12\n");
            }
        else
            {
            readBordersDat();
            }

        printf("13\n");
        writeBordersDat();

        printf("wow\n");
        }

    // Kludgish - if #LOGIN QUESTIONS and no #NEWUSERQUESTIONS, set to old
    // #LOGIN QUESTIONS...
    if (LoginQuestionsSet && cfg.NUQuest[0] == NUQ_NUMQUEST)
        {
        cfg.NUQuest[0] = NUQ_TITLE;
        cfg.NUQuest[1] = NUQ_SURNAME;
        cfg.NUQuest[2] = NUQ_REALNAME;
        cfg.NUQuest[3] = NUQ_PHONENUMBER;
        cfg.NUQuest[4] = NUQ_ADDRESS;
        cfg.NUQuest[5] = NUQ_MESSAGE;
        cfg.NUQuest[6] = NUQ_NUMQUEST;
        printf("something's fucked up...");
        }

    cyclesignature();       // back to first signature for multi sigs

    // Twirly cursor is important! We need one!
    if (!*cfg.twirly)
        {
        strcpy(cfg.twirly, getcfgmsg(266));
        }

    discardData(readcfgd);
    readcfgd = NULL;
    keywords = NULL;

    if (ignore != 2)
        {
#ifndef WINCIT
        doccr();
#endif
        }
#ifdef WINCIT
    win32_con.close();
#endif
    return (TRUE);
    }
