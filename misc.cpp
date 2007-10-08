// --------------------------------------------------------------------------
// Citadel: Misc.CPP
//
// Garbage dump: if it ain't elsewhere, it's here.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "events.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"
#include "statline.h"
#include "tallybuf.h"
#include "termwndw.h"

// --------------------------------------------------------------------------
// Contents
//
// HaveConnectionToUser()   do we have a connection, either console or modem?
// filexists()              Checks for existence of a file
// hash()                   Make an int out of a string
// changedir()              changes curent drive and directory
// changedisk()             change to another drive
// ltoac()                  change a long into a number with ','s in it
// i64toac()                change an int64 into a number with commas in it.

#ifdef WINCIT
TermWindowCollectionC TermWindowCollection;
#endif

// MISC3.CPP
extern discardable *RomanOutputData;
extern discardable *RomanInputData;

// --------------------------------------------------------------------------
// filexists(): Does the file exist?

Bool filexists(const char *filename)
    {
#ifndef WINCIT
    if (debug)
        {
        doccr();
        cPrintf(getdbmsg(53), filename);
        }
#endif

    Bool RetVal = access(filename, 4) == 0;

#ifndef WINCIT
    if (debug)
        {
        if (RetVal)
            {
            cPrintf(getdbmsg(54));
            }
        else
            {
            cPrintf(getdbmsg(55));
            }

        doccr();
        }
#endif

    return (RetVal);
    }


// --------------------------------------------------------------------------
// hash(): Make an int out of a string.

ushort hash(const char *string)
    {
    const char *str;
    ushort h, shift;

    assert(string != NULL);

    str = deansi(string);

    for (h = shift = 0; *str; shift = (ushort) ((shift + 1) & 7), str++)
        {
        h ^= (ushort) ((toupper(*str)) << shift);
        }

    return (h);
    }


// --------------------------------------------------------------------------
// changedisk(): Change to another drive.

static void changedisk(char disk)
    {
#ifndef WINCIT
    disk = (char) toupper(disk);

    union REGS REG;

    assert(disk >= 'A' && disk <= 'Z');

    REG.h.ah = 0x0E;        // select drive

    REG.h.dl = (uchar) (disk - 'A');

    intdos(&REG, &REG);
#endif
    }


// --------------------------------------------------------------------------
// changedir(): Changes curent drive and directory.

Bool changedir(const char *path)
    {
    assert(path);

    if (debug)
        {
#ifndef WINCIT
        doccr();
        cPrintf(getdbmsg(47), path);
#endif
        }

    Bool RetVal = TRUE;

    if (*path)
        {
        if (path[1] == ':')
            {
            changedisk(path[0]);
            }

        if (chdir(path) == -1)
            {
            RetVal = FALSE;
            }
        }
    else
        {
        RetVal = FALSE;
        }

    if (debug)
        {
#ifndef WINCIT
        cPrintf(RetVal ? getdbmsg(13) : getdbmsg(14));
        doccr();
#endif
        }

    return (RetVal);
    }

// --------------------------------------------------------------------------
// i64toac(): Change an int64 into a number with commas in it.

#ifdef WINCIT
const char *TERMWINDOWMEMBER i64toac(__int64 Number)
    {
    if (CurrentUser && CurrentUser->IsRoman() && (Number < 1888888888L))
        {
        return(RomanOutput(Number));
        }
    else
        {
        static char Result[25];
        const Bool Negative = (Number < 0);
        int Pos = 0;

        if (Negative)
            {
            Number = -Number;
            }

        int Counter = -1;

        while (Number > 0)
            {
            if (++Counter == 3)
                {
                if (!CurrentUser || CurrentUser->IsViewCommas())
                    {
                    Result[Pos++] = getmsg(446)[0]; // ','
                    }

                Counter = 0;
                }

            Result[Pos++] = char (((char) (Number % 10)) + '0');
            Number /= 10;
            }

        if (!Pos)
            {
            Result[Pos++] = '0';
            }

        if (Negative)
            {
            Result[Pos++] = '-';
            }

        Result[Pos] = 0;

        strrev(Result);

        return (Result);
        }
    }
#endif




// --------------------------------------------------------------------------
// ltoac(): Change a long into a number with commas in it.

const char *TERMWINDOWMEMBER ltoac(long Number)
    {
    if (CurrentUser && CurrentUser->IsRoman())
        {
        return(RomanOutput(Number));
        }
    else
        {
        static char Result[15];
        const Bool Negative = (Number < 0);
        int Pos = 0;

        if (Negative)
            {
            Number = -Number;
            }

        int Counter = -1;

        while (Number > 0)
            {
            if (++Counter == 3)
                {
                if (!CurrentUser || CurrentUser->IsViewCommas())
                    {
                    Result[Pos++] = getmsg(446)[0]; // ','
                    }

                Counter = 0;
                }

            Result[Pos++] = char (((char) (Number % 10)) + '0');
            Number /= 10;
            }

        if (!Pos)
            {
            Result[Pos++] = '0';
            }

        if (Negative)
            {
            Result[Pos++] = '-';
            }

        Result[Pos] = 0;

        strrev(Result);

        return (Result);
        }
    }


// --------------------------------------------------------------------------
// HaveConnectionToUser(): Do we have a connection, either console or modem

Bool TERMWINDOWMEMBER HaveConnectionToUser(void)
    {
    return ((onConsole || (CommPort->HaveConnection() && modStat)) && !ExitToMsdos);
    }

// perhaps should be moved. i'll worry about that later...
void getfpath(char *path)
    {
#ifdef VISUALC
    GetCurrentDirectory(DOSPATHSIZE, path);
#else
    path[0] = (char) (getdisk() + 'A');
    path[1] = ':';
    path[2] = '\\';
    getcurdir(0, path + 3);
#endif
    }

void doEvent(CITEVENTS eventNum, void (*InitScript)(ScriptInfoS *si))
    {
    assert(eventNum < EVT_NUMEVTS && eventNum >= 0);

    fpath old;
    Bool ran = FALSE;

    for (events *theEvent = eventList; theEvent;
            theEvent = (events *) getNextLL(theEvent))
        {
        if (theEvent->type == eventNum)
            {
            if (!ran)
                {
                getfpath(old);
                ran = TRUE;
                }

            // This is a bit cheezy...
            if (*theEvent->line == '*')
                {
#ifdef WINCIT
                runScript(theEvent->line + 1, NULL, InitScript);
#else
                runScript(theEvent->line + 1, InitScript);
#endif
                }
            else
                {
#ifndef WINCIT
                RunApplication(theEvent->line, NULL, FALSE, TRUE);
#endif
                }
            }
        }

    if (ran)
        {
        changedir(old);
        }

    for (intEvents *theIntEvent = intEventList; theIntEvent;
            theIntEvent = (intEvents *) getNextLL(theIntEvent))
        {
        if (theIntEvent->type == eventNum)
            {
            (*theIntEvent->func)(eventNum);
            }
        }
    }

int rlmEvent(int type, long param1, long param2)
    {
#ifndef WINCIT
    RLMfuncs *fCur;
    int toRet = 0;

    for (fCur = RLMdo; !toRet && fCur; fCur = (RLMfuncs *) getNextLL(fCur))
        {
        if (fCur->type == type)
            {
            Bool run = TRUE;

            switch (type)
                {
                case LT_KEYSTROKE:
                case LT_DIALOUTKEY:
                    {
                    if (fCur->moreData && fCur->moreData != param1)
                        {
                        run = FALSE;
                        }

                    break;
                    }
                }

            if (run)
                {
                toRet = callRLM(fCur->theRLM, fCur->offset, param1, param2);
                }
            }
        }

    return (toRet);
#else
    return (0);
#endif
    }

void TERMWINDOWMEMBER ShowHelpMessage(int msgNumber)
    {
    if (!CurrentUser->IsExpert())
        {
        CRmPrintfCR(getmsg(msgNumber));
        }
    }

void MsgReadOptions::Clear(Bool SetVerbose)
    {
    Number          = FALSE;
    Headerscan      = FALSE;
    Date            = FALSE;
    DatePrevious    = FALSE;
    All             = FALSE;
    Verbose         = !!SetVerbose;
    Reverse         = FALSE;
    CheckDate       = 0;
    MessageNumber   = 0;
    DotoMessage     = NO_SPECIAL;
    PauseBetween    = FALSE;
    }

#ifndef VISUALC
int max(int a, int b)
    {
    return ((a > b) ? a : b);
    }

int min(int a, int b)
    {
    return ((a < b) ? a : b);
    }

long max(long a, long b)
    {
    return ((a > b) ? a : b);
    }

long min(long a, long b)
    {
    return ((a < b) ? a : b);
    }
#endif

Bool isNumeric(const char *str)
    {
    for (; *str; str++)
        {
        if (!isdigit(*str))
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }

void TERMWINDOWMEMBER SetDoWhat(dowhattype NewDoWhat)
    {
    if (NewDoWhat == READUSERLOG || NewDoWhat == READMESSAGE ||
            NewDoWhat == READDIR || NewDoWhat == READZIP || NewDoWhat == READINFO)
        {
        OC.User.SetCanR(TRUE);
        }

    DoWhat = NewDoWhat;

    if (DoWhat != PROMPT)
        {
        PDoWhat = DoWhat;
        }

    StatusLine.Update(WC_TWp);
    }


// --------------------------------------------------------------------------
// frees extra stuff from memory
//
// Level = 0: Do it quickly
// Level = 1: Take more time, but do it better
// Level = 2: Take even more time, but do as good a job as possible. (This is
//              not as safe.)

#ifdef WINCIT
void compactMemory(int) {}
#else
void compactMemory(int Level)
    {
    VerifyHeap();

    if (Level > 0)
        {
        DumpUnlockedMenus();
        DumpUnlockedMessages();

        if (Level > 1)
            {
            PurgeAllSubMessages();
            VerifyHeap();
            }
        }

    VerifyHeap();

    delete [] encbuf;
    encbuf = NULL;

    VerifyHeap();

    freeHufTree();

    VerifyHeap();

    purgeAllRLMs();

    VerifyHeap();

    if (RomanOutputData)
        {
        discardData(RomanOutputData);
        RomanOutputData = NULL;
        }

    if (RomanInputData)
        {
        discardData(RomanInputData);
        RomanInputData = NULL;
        }

    VerifyHeap();

    if (Level > 1)
        {
        ReloadAllSubMessages();
        VerifyHeap();
        }
    }
#endif

#undef fopen
#undef unlink
#undef rename
#undef strdup

FILE *CitFopen(const char *FileName, const char *Mode)
    {
    if (debug)
        {
#ifndef WINCIT
        doccr();
        cPrintf(getdbmsg(12), FileName, Mode);
#endif
        }

#ifdef MINGW
    FILE *f = fopen(FileName, Mode); //, SH_DENYNO);
#else
#ifdef VISUALC
    FILE *f = _fsopen(FileName, Mode, _SH_DENYNO);
#else
    FILE *f = _fsopen(FileName, Mode, SH_DENYNONE);
#endif
#endif

    if (debug)
        {
#ifndef WINCIT
        if (f)
            {
            cPrintf(getdbmsg(13));
            }
        else
            {
            cPrintf(getdbmsg(14));
            }

        doccr();
#endif
        }

    return (f);
    }

int CitUnlink(const char *Path)
    {
    if (debug)
        {
#ifndef WINCIT
        doccr();
        cPrintf(getdbmsg(56), Path);
#endif
        }

    int RetVal = unlink(Path);

    if (debug)
        {
#ifndef WINCIT
        if (!RetVal)
            {
            cPrintf(getdbmsg(13));
            }
        else
            {
            cPrintf(getdbmsg(14));
            }

        doccr();
#endif
        }

    return (RetVal);
    }

int CitRename(const char *Orig, const char *Dest)
    {
    if (debug)
        {
#ifndef WINCIT
        doccr();
        cPrintf(getdbmsg(57), Orig, Dest);
#endif
        }

    int RetVal = rename(Orig, Dest);

    if (debug)
        {
#ifndef WINCIT
        if (!RetVal)
            {
            cPrintf(getdbmsg(13));
            }
        else
            {
            cPrintf(getdbmsg(14));
            }

        doccr();
#endif
        }

    return (RetVal);
    }

char *CitStrdup(const char *Orig)
    {
    assert(Orig);

    char *Copy = new char[strlen(Orig) + 1];

    if (Copy)
        {
        strcpy(Copy, Orig);
        }

    return (Copy);
    }

DaysE EvaluateDay(const char *Str)
    {
    int i = atoi(Str);

    if (i > 0 && i <= NUM_DAYS)
        {
        return ((DaysE) (i - 1));
        }
    else
        {
        for (i = 0; i < NUM_DAYS; i++)
            {
            if (SameString(Str, days[i]) ||
                    SameString(Str, fulldays[i]))
                {
                return ((DaysE) i);
                }
            }
        }

    return (NUM_DAYS);
    }

MonthsE EvaluateMonth(const char *Str)
    {
    int i = atoi(Str);

    if (i > 0 && i <= NUM_MONTHS)
        {
        return ((MonthsE) (i - 1));
        }
    else
        {
        for (i = 0; i < NUM_MONTHS; i++)
            {
            if (SameString(Str, monthTab[i]) ||
                    SameString(Str, fullmnts[i]))
                {
                return ((MonthsE) i);
                }
            }
        }

    return (NUM_MONTHS);
    }


void CitIsIdle(void)
    {
#ifndef WINCIT
    if (!KBReady())
        {
        union REGS t_regs;

        switch (OSType)
            {
            case OS_OS2:
            case OS_WIN3:
            case OS_WINS:
                {
                t_regs.x.ax = 0x1680;
                int86(0x2f,&t_regs,&t_regs);
                break;
                }

            case OS_DV:
                {
                t_regs.x.ax = 0x1000;
                int86(0x15,&t_regs,&t_regs);
                break;
                }
            }
        }
#endif
    }

void cdecl noPrintf(const char *, ...)
    {
    }

void noDoCR(void)
    {
    }

// Publish SystemEvents to all users as appropriate
//  Data: for SE_LOGONOFF, TRUE if On; FALSE if off
//          for SE_NEWMESSAGE, room number of message
//          for SE_CHATGROUP, group slot
//          for SE_ROOMINOUT, room

#ifdef MULTI
void TermWindowCollectionC::SystemEvent(SystemEventE Type, int Data, void *MoreData, Bool HideFromConsole, const char *Event)
    {
    Lock();

    TermWindowListS *C = List;

    while (C)
        {
        C->T->AddSystemEvent(Type, Data, MoreData, HideFromConsole, Event);
        C = C->next;
        }

    Unlock();
    }

// returns true if added; false if not added for any reason at all (turned off, does not apply, out of memory...)
Bool TermWindowC::AddSystemEvent(SystemEventE Type, int Data, void *MoreData, Bool HideFromConsole, const char *Event)
    {
    Bool ToAdd = FALSE;
    Bool HideMyOwnEvents = OurThreadId == GetCurrentThreadId();
    if (CurrentUser->IsSeeOwnChats())
        {
        if (Type == SE_CHATALL
        || Type == SE_CHATUSER
        || Type == SE_CHATROOM
        || Type == SE_CHATGROUP)
        {
            HideMyOwnEvents = FALSE;
        }
        }

        if(Type == SE_NEWMESSAGE_SOMEWHERE)
        {
        Bool *fuckyou = new Bool[cfg.maxrooms];
        if (fuckyou)
        {
        ulong newMsgs = 0;

        theOtherAlgorithm(fuckyou, 0, TRUE);

        for (r_slot rmCnt = 0; rmCnt < cfg.maxrooms; rmCnt++)
            {
            if(CurrentUser->IsUsePersonalHall())
                {
                    if (fuckyou[rmCnt] && !(CurrentUser->IsRoomExcluded(rmCnt)) && CurrentUser->IsRoomInPersonalHall(rmCnt))
                        {
                        newMsgs += Talley->NewInRoom(rmCnt);
                        }
                }
            else
                {
                    if (fuckyou[rmCnt] && !CurrentUser->IsRoomExcluded(rmCnt))
                        {
                        newMsgs += Talley->NewInRoom(rmCnt);
                        }
                }
            }
        if (newMsgs == 0)
            {
            seenAllRooms = TRUE;
            }
        }
        delete [] fuckyou;
        }

    if (SystemEventMutex                                // The mutex must exist...
        && !HideMyOwnEvents                             // And the event must not have been generated by us...
        && !(Type == SE_NEWMESSAGE && Data != thisRoom) // And if a new message, must be in the room...
        && !((Type == SE_NEWMESSAGE_SOMEWHERE) && ((Data == thisRoom) || !(seenAllRooms))) // we must have seen all rooms...
        && !(Type == SE_CHATROOM && Data != thisRoom)   // And if chat to room, must be in the room...
        && !(Type == SE_CHATGROUP && !CurrentUser->IsInGroup((g_slot) Data))    // And if chat to group, must be in the group...
        && !(Type == SE_ROOMINOUT && Data != thisRoom)  // And if room in/out, must be in the room...
        && CurrentUser->ShowSystemEvent(Type))          // And user is showing this event.
        {
        ToAdd = TRUE;

        if (Type == SE_EXCLUSIVEMESSAGE || Type == SE_NEWMESSAGE || Type == SE_NEWMESSAGE_SOMEWHERE)
            {
            // For these types, only show the event if this user can see the message!
            if (CurrentUser->MaySeeMessage((Message *) MoreData) != MSM_GOOD)
                {
                ToAdd = FALSE;
                }

            if (Type == SE_NEWMESSAGE_SOMEWHERE)
                {
                if (!theAlgorithm(Data, 0, TRUE))
                    {
                    ToAdd = FALSE;
                    }
                if(CurrentUser->IsUsePersonalHall())
                    {
                    if(!(CurrentUser->IsRoomInPersonalHall(Data)))
                        {
                        ToAdd = FALSE;
                        }
                    }
                if (!CurrentUser->IsHearLaughter())
                    {
                    ToAdd = FALSE;
                    }

                 // sorry john...your implementation just didn't cut it! 66066

                 // jrd-
                 // Only deliver the Scribble Sound message once by hacking the meaning of seenAllRooms, to
                 // fool the situation into appearing that I haven't seen everything, and therefore don't
                 // wanna hear about scribbling.
                 if (ToAdd)
                    {
                    seenAllRooms = FALSE ;
                    }
                }
            }

        // If we can't add it after five seconds, screw it.
        if (WaitForSingleObject(SystemEventMutex, 5000) != WAIT_TIMEOUT)
            {

            if (ToAdd)
                {
                SystemEventS *Added = (SystemEventS *) addLL((void **) &PendingSystemEvents, sizeof(SystemEventS) + strlen(Event));

                if (Added)
                    {
                    strcpy(Added->string, Event);
                    Added->HideFromConsole = HideFromConsole;
                    }
                else
                    {
                    ToAdd = FALSE;
                    }
                }

            ReleaseMutex(SystemEventMutex);
            }
        }

    return (ToAdd);
    }

Bool TermWindowCollectionC::IsUserLoggedIn(const char *Name)
    {
    Bool retVal = FALSE;

    Lock();

    for (TermWindowListS *S = List; S && !retVal; S = S->next)
        {
        label TestName;
        if (SameString(Name, S->T->CurrentUser->GetName(TestName, sizeof(TestName))))
            {
            retVal = TRUE;
            }
        }

    Unlock();

    return (retVal);
    }

long TermWindowCollectionC::FindConnSeqByThreadId(DWORD ThreadId)
    {
    DWORD retVal = -1;

    Lock();

    for (TermWindowListS *S = List; S && retVal == -1; S = S->next)
        {
        if (S->T->OurThreadId == ThreadId)
            {
            retVal = S->T->OurConSeq;
            }
        }

    Unlock();

    return (retVal);
    }
#endif

#ifdef MULTI
LoginListC::~LoginListC()
    {
    if (OurMutex != INVALID_HANDLE_VALUE)
        {
        CloseHandle(OurMutex);
        }

    disposeLL((void **) &List);
    }

Bool LoginListC::Add(l_index Index)
    {
    xLoginListS *New = NULL;

    if (TimedLock())
        {
        New = (xLoginListS *) addLL((void **) &List, sizeof(xLoginListS));

        if (New)
            {
            New->Index = Index;
            }

        Unlock();
        }

    return (New != NULL);
    }

Bool LoginListC::AddUnique(l_index Index)
    {
    Bool RetVal = FALSE;

    Lock();

    if (!IsLoggedIn(Index))
        {
        RetVal = Add(Index);
        }

    Unlock();

    return (RetVal);
    }


Bool LoginListC::Remove(l_index Index)
    {
    Lock();

    xLoginListS *cur;
    int Counter;

    for (cur = List, Counter = 1; cur; cur = (xLoginListS *) getNextLL(cur), Counter++)
        {
        if (cur->Index == Index)
            {
            deleteLLNode((void **) &List, Counter);
            Unlock();
            return (TRUE);
            }
        }

    Unlock();

    return (FALSE);
    }

Bool LoginListC::IsLoggedIn(l_index Index)
    {
    Lock();

    xLoginListS *cur;

    for (cur = List; cur; cur = (xLoginListS *) getNextLL(cur))
        {
        if (cur->Index == Index)
            {
            break;
            }
        }

    Unlock();

    return (cur != NULL);
    }

// return:
//  TRUE: Good
//  FALSE: User not found
//  CERROR: Could not send
int TermWindowCollectionC::EventToUser(const char *User, Bool HideFromConsole, const char *Event)
    {
    Lock();

    Bool FoundUser = FALSE, Sent = FALSE;

    for (TermWindowListS *cur = List; cur; cur = (TermWindowListS *) getNextLL(cur))
        {
        label TestName;

        if (SameString(User, cur->T->CurrentUser->GetName(TestName, sizeof(TestName))))
            {
            FoundUser = TRUE;

            if (cur->T->AddSystemEvent(SE_CHATUSER, 0, NULL, HideFromConsole, Event))
                {
                Sent = TRUE;
                }
            }
        }

    Unlock();

    return (Sent ? TRUE : FoundUser ? CERROR : FALSE);
    }

void TermWindowCollectionC::PingAll(void)
    {
    Lock();

    for (TermWindowListS *cur = List; cur; cur = (TermWindowListS *) getNextLL(cur))
        {
        cur->T->CommPort->Ping();
        }

    Unlock();
    }
#endif
