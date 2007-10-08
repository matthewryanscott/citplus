/// --------------------------------------------------------------------------
// Citadel: Proto.H
//
// Many of Citadel's function prototypes.

//#if defined(__BORLANDC__)
inline void VerifyHeap(int Level)
    {
    if (DEBUGLEVEL >= Level)
        {
#ifdef VISUALC
// _heapchk() is Compatible with Windows NT only
//      assert(_heapchk() == _HEAPOK);
#else
        assert(heapcheck() > 0);
#endif
        }
    }

inline void VerifyHeap(void)
    {
#if DEBUGLEVEL
    VerifyHeap(1);
#endif
    }
//#else
//#define VerifyHeap()
//#endif


// APLIC.CPP
int QuickRunApp(const char *Cmd);


// AUTOROOM.CPP
Bool AutoRoom(const char *NewName, const char *Creator, g_slot NewGroup, h_slot NewHall);


// CONSOLE.CPP
Bool cdecl special_pressed(void);


// CONOVL.CPP
Bool IsConsoleLocked(Bool CheckingF6 = FALSE);


// CONSYSOP.CPP
void doConsoleSysop(void);


// CRON.CPP
void did_net(const char *callnode);


// CRYPT.CPP
Bool decrypt(char *text, const char *key);
Bool encrypt(char *text, const char *key);


// DISCARD.CPP
long checkDataVer(void);
discardable *readData(int Entry, int StartSubEntry = 0, int EndSubEntry = INT_MAX);
void discardData(discardable *data);


// DOWN.CPP
void cdecl crashout(const char *message, ...);
void exitcitadel(void);


// CWINDOWS.CPP
void CitWindowsTTYUpdate(int sline, int eline);
void CitWindowsProcessKbd(void);
void CitWindowsProcessMouse(void);


// EDIT.CPP
void GetFileMessage(FILE *fl, char *str, int mlen);


// small part of EXTMSG.CPP
Bool read_messages(void);

inline const char *getmsg(uint nmbr)
    {
    // Uncomment this for supreme error checking.
    //assert(ExtMessages[msg].UseCounter);
    return (msgsI[nmbr]);
    }



// FILECIT.CPP
Bool citOpen(const char *filename, CitOpenCodes mode, FILE **cf);


// FILES.CPP
#ifdef MINGW
int fwdstrcmp(void *a, void *b);
int revstrcmp(void *a, void *b);
#else
int _cdecl fwdstrcmp(void *a, void *b);
int _cdecl revstrcmp(void *a, void *b);
#endif
Bool haveRespProt(void);
const char *nameonly(const char *full);
int ambig(const char *filename);
int ambigUnlink(const char *filename, const char *directory = NULL);
#ifdef WINCIT
__int64 bytesfree(void);
#else
long bytesfree(void);
#endif
Bool IsFilenameLegal(const char *filename, Bool AllowSpace);
void entrycopy(int element);
void entrymake(struct find_t *file_buf);
uchar getattr(const char *filename);
void hide(const char *filename);
void setattr(const char *filename, uchar attr);
Bool copyfile(const char *source, const char *dest);
#ifdef WINCIT
__int64 CitGetDiskFreeSpace(char *path);
#endif

// FORMAT.CPP
void normalizeString(char *s);
int strpos(char ch, const char *str);
int substr(const char *str1, const char *str2, Bool StripAnsi);
inline Bool IsSubstr(const char *s1, const char *s2)
    {
    return (substr(s1, s2, TRUE) != CERROR);
    }

const char *deansi(const char *str);
const char *special_deansi(const char *str, const char *strip);
int deansi_str_cmpi(const char *string1, const char *string2);
int deansi_str_cmp(const char *string1, const char *string2);
void stripansi(char *string);
int hasansi(const char *string);
void strip(char *str, char ch);
const char *strip_punct(const char *str);
void strip_special(char *str);


// FMTOVL.CPP
void cdecl sformat(char *str, const char *fmt, const char *val, ...);
int parse_it(char *words[], char input[]);
Bool u_match(register const char *s, register const char *p);
void stripdoubleasterisk(char *str);
char *MakeItUpper(const char *Source, char *Dest);
char *MakeItLower(const char *Source, char *Dest);


// GRPMEMBR.CPP
Bool GroupNameTester(g_slot GroupSlot);
Bool GroupWithMetaTester(g_slot GroupSlot);
Bool GroupTester(g_slot GroupSlot);
#ifdef WINCIT
short TestGroupExists(const char *TestString, TermWindowC *TW);
short TestGroupOrMetaExists(const char *TestString, TermWindowC *TW);
void ShowGroupWithMeta(g_slot Group, TermWindowC *TW);
void ShowGroup(g_slot Group, TermWindowC *TW);
#else
short TestGroupExists(const char *TestString);
short TestGroupOrMetaExists(const char *TestString);
void ShowGroupWithMeta(g_slot Group);
void ShowGroup(g_slot Group);
#endif

// HELP.CPP
void initMenus(void);

// HUF.CPP
void freeHufTree(void);
void encode(uchar in);
Bool compress(char *str);
void decompress(uchar *str);


#ifndef WINCIT
// HUFBIT.ASM
extern "C" uchar cdecl decode(const uchar *str);
#endif


// LLSUP.CPP
void disposeLL(void **list);
void *addLL(void **list, uint nodeSize);
void *getNextLL(const void *list);
void *getLLNum(void *list, ulong n);
void deleteLLNode(void **list, ulong n);
void *insertLLNode(void **list, ulong n, int nodeSize);
ulong getLLCount(const void *list);


// LOGIN.CPP
#ifdef WINCIT
void slideLTab(l_slot *slot, const LogEntry1 *Entry, l_slot *Order);
l_slot FindPwInHashInTable(const char *in, const char *pw, const l_slot *Order);
l_slot pwslot(const char *in, const char *pw, const l_slot *Order);
#else
void slideLTab(l_slot *slot, const LogEntry1 *Entry);
l_slot FindPwInHashInTable(const char *in, const char *pw);
l_slot pwslot(const char *in, const char *pw);
#endif



// MAILLIST.CPP
void SendMailToList(Message *Msg);


// MAKETAB.CPP
void RoomTabBld(void);


// MISC.CPP
void compactMemory(int Level = 0);
Bool filexists(const char *filename);
ushort hash(const char *str);
Bool changedir(const char *path);

#define short_LO(i)         ((char) ((i) & 0xff))
#define short_HI(i)         ((char) (((short) (i)) >> 8))
#define short_JOIN(cl, ch)  ((short) (((short) (ch) << 8) | ((short) (cl))))

#define long_LO(l)          ((short) ((l) & 0xffff))
#define long_HI(l)          ((short) ((l) >> 16))
#define long_JOIN(il, ih)   ((long) (((long) ((ushort) (ih))) << 16) | \
                                    ((long) ((ushort) (il))))

void getfpath(char *path);
int rlmEvent(int type, long param1, long param2);

#ifndef VISUALC
int max(int a, int b);
long max(long a, long b);
int min(int a, int b);
long min(long a, long b);
#endif
Bool isNumeric(const char *str);
FILE *CitFopen(const char *FileName, const char *Mode);
int CitUnlink(const char *Path);
int CitRename(const char *Orig, const char *Dest);
char *CitStrdup(const char *Orig);
DaysE EvaluateDay(const char *Str);
MonthsE EvaluateMonth(const char *Str);
void CitIsIdle(void);
void cdecl noPrintf(const char *, ...);
void noDoCR(void);


// MISC3.CPP (Most of MISC3.CPP is in MISCOVL.H)
void OutOfMemory(int Code);
void cOutOfMemory(int Code);

#ifdef WINCIT
	void initMouseHandler(void);
	void deinitMouseHandler(void);
	void showMouse(void);
	void hideMouse(void);
	void setMousePos(int x, int y);

	extern int hideCounter;
#else
extern "C"
	{
	/* MOUSE.ASM */
	extern cdecl void initMouseHandler(void);
	extern cdecl void deinitMouseHandler(void);
	extern cdecl void showMouse(void);
	extern cdecl void hideMouse(void);
	extern cdecl void setMousePos(int x, int y);

	extern cdecl int hideCounter;
	};
#endif
	
// NETNODE.CPP
#ifdef WINCIT
Bool ReadNodesCit(TermWindowC *TW, NodesCitC **node, const char *nodename, ModemConsoleE W, Bool option);
#else
Bool ReadNodesCit(NodesCitC **node, const char *nodename, ModemConsoleE W, Bool option);
#endif

#ifndef WINCIT
// RLM.CPP
int callRLM(RLMlist *theRLM, uint funcOff, long param1, long param2);
Bool initRLM(const char *rlmFileName);
void purgeAllRLMs(void);


// RLMASM.ASM
extern "C" void cdecl citEntryPoint(void);
#endif


// SCRIPT.CPP
#ifdef WINCIT
Bool runScript(const char *what, TermWindowC *TermWindow,
        void (*InitScript)(ScriptInfoS *si) = NULL);
#else
Bool runScript(const char *what, void (*InitScript)(ScriptInfoS *si) = NULL);
#endif


// STRFTIME.CPP
void strftime(char *outstr, int maxsize, const char *formatstr, long tnow);


// TIMEDATE.CPP
#ifdef VISUALC
long dostounix(struct date *d, struct time *t);
void unixtodos(long time, struct date *d, struct time *t);
void gettime(struct time *timep);
#endif
time_t CitadelTime(time_t *tloc);
int dayofweek(void);
void getdstamp(char *buffer, time_t stamp);
void gettstamp(char *buffer, time_t stamp);
int hour(void);
void pause(register int ptime);


// TIMEOVL.CPP
time_t net69_time(const char *datestring);
char *net69_gateway_time(time_t time, char *datestring);
void set_date(void);
void gdate(char *line, datestruct *filldate);   // both args mauled
void gtime(char *line, timestruct *filltime);   // both args mauled


// TRAP.CPP
#ifdef WINCIT
void trap(TrapKeywords what, char *port, const char *fmt, ...);
#else
void trap(TrapKeywords what, const char *fmt, ...);
#endif
statRecord *addStatus(void);
void addStatusText(const char *string, time_t when);


// WINDOVL.CPP
void setborder(int battr);
void setscreen(void);


// WINDOW.CPP
void scroll_bios(int row, uchar howmany, uchar attr);
void cdecl biosstring(uint row, const char *str, uchar attr, Bool phys);
Bool cdecl bioschar(char ch, uchar attr, Bool phys);


#ifdef WINCIT
#else
// WIND.ASM
extern "C"
    {
    void cdecl cls_fast(char attr);
    void cdecl physPosition(uchar row, uchar column);
    void cdecl clearline(uchar row, uchar attr);
    void cdecl physReadpos(const uchar *row, const uchar *column);
    void cdecl fastcpy(void *dest, void *src, uint len);
    void cdecl directstring(uint row, const char *str, uchar attr, Bool phys);
    Bool cdecl directchar(char ch, uchar attr, Bool phys);
    void cdecl fastsetw(void *dest, uint what, uint len);
    void cdecl statDisp(int row, int col, int attr, const char *str);
    int cdecl gmode(void);
    void cdecl cursoff(void);
    void cdecl curson(void);
    void cdecl curshalf(void);
    void interrupt critical_error(...);
    };
#endif


#ifdef WINCIT
// CTDL.CPP
void TurnOffTermWindow(void);
void TurnOnTermWindow(void);
void _USERENTRY NewTermWindowThreadKeepComm(void *TW);
void _USERENTRY NewTermWindowThread(void *TW);
void letWindowsMultitask(void);


// OUTPUT.CPP
void cdecl DebugOut(const char *fmt, ...);
void cdecl DebugOut(int DbMsgIndex, ...);

// OUTWIN.CPP
void output(int Column, int Row, LPSTR str);
void updateText(RECT r, HDC hdc);
void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart);
void resetTextWindow(int attr, Bool resize);

// MISC2.CPP
long FAR PASCAL logoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// ERROR.CPP
long FAR PASCAL errorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
long FAR PASCAL trapWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
long FAR PASCAL msgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void cdecl errorDisp(const char *fmt, ...);
void cdecl msgDisp(const char *fmt, ...);
void cdecl msgCaption(const char *fmt, ...);
void CreateErrorMutexes(void);
void CloseErrorMutexes(void);

#define cPrintfDOS(wow)


#else
#define msgDisp cPrintf
#define msgCaption cPrintf
#define cPrintfDOS(wow) cPrintf(wow)
#define letWindowsMultitask()
#endif


// wowness. some windows/dos memory and other stuff.
#ifdef WINCIT
#define getMemG(size, num) calloc(size, num)
#define freeMemG(wow) free(wow)

#define STATICEXCLUSIVEACCESSRECORD()           \
    static HANDLE Mutex;                        \
    static int UsageCounter;

#define EXCLUSIVEACCESSRECORD()                 \
    HANDLE Mutex;

#define INITIALIZEANDCLAIMSTATICEXCLUSIVEACCESS()\
    if (!UsageCounter++)                        \
        {                                       \
        INITIALIZEEXCLUSIVEACCESS(TRUE);        \
        }                                       \
    else                                        \
        {                                       \
        GAINEXCLUSIVEACCESS();                  \
        }

#define INITIALIZESTATICEXCLUSIVEACCESS()      \
    if (!UsageCounter++)                        \
        {                                       \
        INITIALIZEEXCLUSIVEACCESS(FALSE);       \
        }

#define INITIALIZEEXCLUSIVEACCESS(Claim)        \
    label mName;                                \
    sprintf(mName, "WincitMutex:%p", this);     \
    Mutex = CreateMutex(NULL, Claim, mName)

#define GAINEXCLUSIVEACCESS()                   \
    WaitForSingleObject(Mutex, INFINITE)

#define SAVEEXCLUSIVEACCESSRECORD()             \
    HANDLE SaveMutex = Mutex

#define RESTOREEXCLUSIVEACCESSRECORD()          \
    Mutex = SaveMutex

#define RELEASEEXCLUSIVEACCESS()                \
    ReleaseMutex(Mutex)

#define DESTROYSTATICEXCLUSIVEACCESS()          \
    assert(UsageCounter);                       \
    if (!--UsageCounter)                        \
        {                                       \
        CloseHandle(Mutex);                     \
        }

#define DESTROYEXCLUSIVEACCESS()                \
    CloseHandle(Mutex)

#define EXCLUSIVEACCESSRECORDSIZE() (sizeof(HANDLE))

#define WAITFORmTIMED(x)    (WaitForSingleObject(x, 2000) != WAIT_TIMEOUT)
#define WAITFORm(x) WaitForSingleObject(x, INFINITE)
#define RELEASEm(x) ReleaseMutex(x)

#define LTab(x)     LogTab[LogOrder[x]]

#else
#define getMemG(size, num) _fcalloc(size, num)
#define freeMemG(wow) _ffree(wow)

#define STATICEXCLUSIVEACCESSRECORD()
#define EXCLUSIVEACCESSRECORD()
#define INITIALIZESTATICEXCLUSIVEACCESS(Claim)
#define INITIALIZEEXCLUSIVEACCESS(Claim)
#define GAINEXCLUSIVEACCESS()
#define SAVEEXCLUSIVEACCESSRECORD()
#define RESTOREEXCLUSIVEACCESSRECORD()
#define RELEASEEXCLUSIVEACCESS()
#define DESTROYSTATICEXCLUSIVEACCESS()
#define DESTROYEXCLUSIVEACCESS()
#define EXCLUSIVEACCESSRECORDSIZE() (0)

#define WAITFORmTIMED(x)    (TRUE)
#define WAITFORm(x)
#define RELEASEm(x)

#define LTab(x)     LogTab[x]
#endif
