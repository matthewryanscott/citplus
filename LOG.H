// --------------------------------------------------------------------------
// Citadel: Log.H
//
// All about users.


#ifndef __LOG_H
#define __LOG_H

#include "syncdata.h"

// --------------------------------------------------------------------------
// This class contains a user's flags. This is used in both the full
// structure of a user and in the in-RAM index.

class LogFlags
	{
protected:
	Bool	INUSE		: 1;	// Is this slot in use?
	Bool	UCMASK		: 1;	// Uppercase?
	Bool	LFMASK		: 1;	// Linefeeds?
	Bool	EXPERT		: 1;	// Expert?
	Bool	AIDE		: 1;	// Aide?
	Bool	TABS		: 1;	// Tabs?
	Bool	OLDTOO		: 1;	// Print out last oldmessage on N>ew?
	Bool	PROBLEM 	: 1;	// Twit bit
	Bool	UNLISTED	: 1;	// List in Userlog?
	Bool	PERMANENT	: 1;	// Permanent log entry?
	Bool	SYSOP		: 1;	// Sysop special access.
	Bool	NODE		: 1;	// Are they a node?
	Bool	NETUSER 	: 1;	// Can they send networked messages?
	Bool	NOACCOUNT	: 1;	// Accounting disabled for this person?
	Bool	NOMAIL		: 1;	// Can user send exclusive mail?
	Bool	ROOMTELL	: 1;	// Room-descriptions?

public:
	void Clear(void)
		{
		memset(this, 0, sizeof(*this));
		}

	LogFlags(void)
		{
		Clear();
		}

	Bool IsInuse(void) const		{ assert(this); return (INUSE); }
	void SetInuse(Bool New) 		{ assert(this); INUSE = !!New; }
	Bool IsUpperOnly(void) const	{ assert(this); return (UCMASK); }
	void SetUpperOnly(Bool New) 	{ assert(this); UCMASK = !!New; }
	Bool IsLinefeeds(void) const	{ assert(this); return (LFMASK); }
	void SetLinefeeds(Bool New) 	{ assert(this); LFMASK = !!New; }
	Bool IsExpert(void) const		{ assert(this); return (EXPERT); }
	void SetExpert(Bool New)		{ assert(this); EXPERT = !!New; }
	Bool IsAide(void) const 		{ assert(this); return (AIDE || SYSOP); }
	void SetAide(Bool New)			{ assert(this); AIDE = !!New; }
	Bool IsTabs(void) const 		{ assert(this); return (TABS); }
	void SetTabs(Bool New)			{ assert(this); TABS = !!New; }
	Bool IsOldToo(void) const		{ assert(this); return (OLDTOO); }
	void SetOldToo(Bool New)		{ assert(this); OLDTOO = !!New; }
	Bool IsProblem(void) const		{ assert(this); return (PROBLEM && !IsAide()); }
	void SetProblem(Bool New)		{ assert(this); PROBLEM = !!New; }
	Bool IsUnlisted(void) const 	{ assert(this); return (UNLISTED); }
	void SetUnlisted(Bool New)		{ assert(this); UNLISTED = !!New; }
	Bool IsPermanent(void) const	{ assert(this); return (PERMANENT); }
	void SetPermanent(Bool New) 	{ assert(this); PERMANENT = !!New; }
	Bool IsSysop(void) const		{ assert(this); return (SYSOP); }
	void SetSysop(Bool New) 		{ assert(this); SYSOP = !!New; }
	Bool IsNode(void) const 		{ assert(this); return (NODE); }
	void SetNode(Bool New)			{ assert(this); NODE = !!New; }
	Bool IsNetUser(void) const		{ assert(this); return (NETUSER); }
	void SetNetUser(Bool New)		{ assert(this); NETUSER = !!New; }
	Bool IsAccounting(void) const	{ assert(this); return (!NOACCOUNT && !NODE); }
	void SetAccounting(Bool New)	{ assert(this); NOACCOUNT = !New; }
	Bool IsMail(void) const 		{ assert(this); return (!NOMAIL); }
	void SetMail(Bool New)			{ assert(this); NOMAIL = !New; }
	Bool IsViewRoomDesc(void) const { assert(this); return (ROOMTELL); }
	void SetViewRoomDesc(Bool New)	{ assert(this); ROOMTELL = !!New; }

	void CopyFlags(const LogFlags &ToCopy);
	};


// --------------------------------------------------------------------------
// This class is the base class for all of the log-related classes that get
// saved to disk. It is used to store the file name to use and to store
// information about the file if it is currently open.

class LogStarter
	{
	char	FileName[13];
	char	PAD;
	FILE	*File;
	int		OpenCounter;

public:
	EXCLUSIVEACCESSRECORD()

	// All derived classes have a default file name (LOG.DAT, LOG2.DAT, etc.)
	// that their data are saved to. Each one defines its own version of
	// this function to set its default.
	virtual void ResetFileName(void) = 0;

	LogStarter(void)
		{
		assert(this);
		VerifyHeap();

		FileName[0] = 0;
		File = NULL;
		OpenCounter = 0;
		INITIALIZEEXCLUSIVEACCESS(FALSE);
		}

	~LogStarter(void)
		{
		GAINEXCLUSIVEACCESS();
		assert(this);
		VerifyHeap();

		if (File)
			{
			fclose(File);
			}

		RELEASEEXCLUSIVEACCESS();
		DESTROYEXCLUSIVEACCESS();
		}

	LogStarter& operator =(const LogStarter &Original)
		{
#ifdef GOODBYE
		SAVEEXCLUSIVEACCESSRECORD();
		memcpy(this, &Original, sizeof(LogStarter));
		File = NULL;
		OpenCounter = 0;
		RESTOREEXCLUSIVEACCESSRECORD();
#endif

		return (*this);
		}

	const char *GetFileName(char *Buffer, int Length) const
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, FileName, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetFileName(const char *New)
		{
		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(FileName, New);
		RELEASEEXCLUSIVEACCESS();
		}

	void OpenFile(void);
	void CloseFile(void);

	FILE *GetFile(void) const
		{
		assert(this);

		return (File);
		}

	void SetFile(FILE *New)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		File = New;
		RELEASEEXCLUSIVEACCESS();
		}
	};

#include "bitbag.h"


// --------------------------------------------------------------------------
// Several of the log files (LOG2.DAT, LOG3.DAT, LOG4.DAT, and LOG6.DAT) keep
// track of arrays of bits. As all of their data are stored in the same way
// in memory and disk, they all are based on the same class. Their only
// differences are the default filenames to use for disk storage, and the
// names used to access their data.

class LogBitBag : public LogStarter, public BitBag
	{
public:
#ifdef WINCIT
	l_index SyncNumber;
	Bool SyncSendPaused;

	int GetSyncNumber(void)
		{
		return (SyncNumber);
		}

	void PauseSyncSend(void)
		{
		SyncSendPaused = TRUE;
		}
#endif

	void Clear(void)
		{
		BitBag::Clear();
#ifdef WINCIT
		SyncNumber = CERROR;
		SyncSendPaused = FALSE;
#endif
		}

	LogBitBag(g_slot NumGroups) : BitBag(NumGroups)
		{
		Clear();
		VerifyHeap();
		}

	Bool Load(l_index Index);
	Bool Save(l_index Index);
	};


// --------------------------------------------------------------------------
// Here are all of the classes used to store information in LOG*.DAT.

#include "log1.h"
#include "log2.h"
#include "log3.h"
#include "log4.h"
#include "log5.h"
#include "log6.h"


// --------------------------------------------------------------------------
// In addition to LOG.DAT, LOG2.DAT, LOG3.DAT, LOG4.DAT, LOG5.DAT, and
// LOG6.DAT, which all store structured information, Citadel uses log
// extension files to store non-structured data. This includes anything that
// might change in number and length, such as words in the user's personal
// dictionary. Each user gets one file. The name is derived from the user's
// location in the LOG.DAT file: LE###.DAT, where ### is replaced by an ASCII
// representation of the user's location (in decimal).

struct userDefLE
	{
	userDefLE *next;
	label Code;
	char Data[1];
	};

class LogExtensions
	{
	EXCLUSIVEACCESSRECORD()

#ifdef WINCIT
	l_index		SyncNumber;
	Bool		SyncSendPaused;
#endif

	char		FileExtension[4];	// Extension to use for file.
	strList 	*Kill[KF_NUM];		// Kill file
	pairedStrings *le_tuser;		// Log extension: tag user
	pairedStrings *le_replace;		// Log extension: replace
	strList 	*le_dict;			// Log extension: dictionary
	userDefLE	*le_userdef;		// User defined data (scripts)
	char		*le_Finger; 		// User's finger stuff.
	jumpback	*jb;				// pointer to jumpback stack
	int			jb_start;			// where valid data starts...
	int			jb_end; 			// ...and ends
	int			jb_length;			// how much we can store plus 1.
	Message 	*Msg;				// Lost message.
	r_slot		MsgRoom;			// Room for lost message.
	short		PAD;				// This is a test...

public:
#ifdef WINCIT
	int GetSyncNumber(void)
		{
		return (SyncNumber);
		}

	void PauseSyncSend(void)
		{
		SyncSendPaused = TRUE;
		}

	void ResumeSyncSend(void);
#endif

	Bool Load(l_index Index);
	Bool Save(l_index Index, const char *Name, r_slot MsgRoom);

	Message *GetMessage(void) const
		{
		assert(this);
		return (Msg);
		}

	r_slot GetMessageRoom(void) const
		{
		assert(this);
		return (MsgRoom);
		}

	void SetMessageRoom(r_slot NewMsgRoom)
		{
		SYNCDATA(LE_SETMESSAGEROOM, NewMsgRoom, 0);

		assert(this);
		MsgRoom = NewMsgRoom;
		}

	void SetMessage(Message *NewMsg);
	void ClearMessage(void);

	const char *GetFinger(char *Buffer, int Length) const
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		if (le_Finger)
			{
			CopyString2Buffer(Buffer, le_Finger, Length);
			RELEASEEXCLUSIVEACCESS();
			return (Buffer);
			}
		else
			{
			*Buffer = 0;
			RELEASEEXCLUSIVEACCESS();
			return (NULL);
			}
		}

	Bool SetFinger(const char *NewFinger)
		{
		SYNCDATAR(LE_SETFINGER, NewFinger, 0);

		assert(this);
		assert(NewFinger);

		GAINEXCLUSIVEACCESS();
		delete [] le_Finger;

		if (*NewFinger)
			{
			le_Finger = strdup(NewFinger);
			RELEASEEXCLUSIVEACCESS();
			return (!!le_Finger);
			}
		else
			{
			le_Finger = NULL;
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	void Clear(void);

	LogExtensions(int MaxJumpback)
		{
		INITIALIZEEXCLUSIVEACCESS(TRUE);
		SAVEEXCLUSIVEACCESSRECORD();

		VerifyHeap();

		memset(this, 0, sizeof(*this));
#ifdef WINCIT
		SyncNumber = -1;
#endif

		VerifyHeap();
		
		RESTOREEXCLUSIVEACCESSRECORD();

		VerifyHeap();
		
		jb_length = MaxJumpback;
		jb = new jumpback[MaxJumpback];
		ResetFileExtension();
		
		VerifyHeap();

		ADDSYNC();

		RELEASEEXCLUSIVEACCESS();

		VerifyHeap();
		}

	~LogExtensions()
		{
		GAINEXCLUSIVEACCESS();
		Clear();
		delete [] jb;
		REMOVESYNC();
		RELEASEEXCLUSIVEACCESS();
		DESTROYEXCLUSIVEACCESS();
		}

	LogExtensions& operator =(const LogExtensions &Original);
	LogExtensions(const LogExtensions &Original);

	void SetFileExtension(const char *NewExtension)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(FileExtension, NewExtension);
		RELEASEEXCLUSIVEACCESS();
		}

	void ResetFileExtension(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(FileExtension, "dat");
		RELEASEEXCLUSIVEACCESS();
		}

	const void *GetPointer1(void) const
		{
		assert(this);
		return (Kill[KF_USER]);
		}

	const void *GetPointer2(void) const
		{
		assert(this);
		return (Kill[KF_NODE]);
		}

	const void *GetPointer3(void) const
		{
		assert(this);
		return (Kill[KF_TEXT]);
		}

	const void *GetPointer4(void) const
		{
		assert(this);
		return (Kill[KF_REGION]);
		}

	const void *GetPointer5(void) const
		{
		assert(this);
		return (le_tuser);
		}

	const void *GetPointer6(void) const
		{
		assert(this);
		return (le_dict);
		}

	const void *GetPointer7(void) const
		{
		assert(this);
		return (le_Finger);
		}

	const void *GetPointer8(void) const
		{
		assert(this);
		return (Msg);
		}

	const void *GetPointer9(void) const
		{
		assert(this);
		return (jb);
		}

	const void *GetPointer10(void) const
		{
		assert(this);
		return (le_userdef);
		}

	const void *GetPointer11(void) const
		{
		assert(this);
		return (le_replace);
		}

	ulong GetKillCount(KillFileE Type)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		ulong ToRet = getLLCount(Kill[Type]);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool GetKillNum(KillFileE Type, ulong Num, char *Buffer, int Length)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const strList *sl = (strList *) getLLNum(Kill[Type], Num);
		if (sl)
			{
			CopyString2Buffer(Buffer, sl->string, Length);
			}
		else
			{
			*Buffer = 0;
			}
		RELEASEEXCLUSIVEACCESS();
		return (!!sl);
		}

	ulong GetTagUserCount(void) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		ulong ToRet = getLLCount(le_tuser);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool GetTagUserNum(ulong Num, char *UBuffer, int ULength,
			char *TBuffer, int TLength) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const pairedStrings *ps = (pairedStrings *) getLLNum(le_tuser, Num);
		if (ps)
			{
			CopyString2Buffer(UBuffer, ps->string1, ULength);
			CopyString2Buffer(TBuffer, ps->string2, TLength);
			}
		else
			{
			*UBuffer = *TBuffer = 0;
			}
		RELEASEEXCLUSIVEACCESS();
		return (!!ps);
		}

	ulong GetTagUserCount(const char *User) const;
	Bool GetUserTag(ulong Num, const char *User, char *Buffer, int Length) const;

	ulong GetReplaceCount(void) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		ulong ToRet = getLLCount(le_replace);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool GetReplaceNum(ulong Num, char *OBuffer, int OLength,
			char *RBuffer, int RLength) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const pairedStrings *ps = (pairedStrings *) getLLNum(le_replace, Num);
		if (ps)
			{
			CopyString2Buffer(OBuffer, ps->string1, OLength);
			CopyString2Buffer(RBuffer, ps->string2, RLength);
			}
		else
			{
			*OBuffer = *RBuffer = 0;
			}
		RELEASEEXCLUSIVEACCESS();
		return (!!ps);
		}

	Bool FindReplace(const char *ToReplace, char *Buffer, int Length);

	ulong GetDictionaryCount(void) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		ulong ToRet = getLLCount(le_dict);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool GetDictNum(ulong Num, char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const strList *sl = (strList *) getLLNum(le_dict, Num);
		if (sl)
			{
			CopyString2Buffer(Buffer, sl->string, Length);
			}
		else
			{
			*Buffer = 0;
			}
		RELEASEEXCLUSIVEACCESS();
		return (!!sl);
		}

	Bool AddKill(KillFileE Type, const char *String);
	Bool RemoveKill(KillFileE Type, const char *String);
	Bool IsKill(KillFileE Type, const char *String) const;

	Bool AddTagUser(const char *User, const char *Tag);
	Bool AddReplace(const char *Orig, const char *Repl);
	Bool AddWordToDictionary(const char *Word);
	Bool RemoveUserTag(const char *User);
	Bool RemoveReplace(const char *Orig);
	Bool RemoveWordFromDictionary(const char *Word);
	Bool IsWordInDictionary(const char *Word) const;
	Bool SetUserDefined(const char *Code, const char *Data);
	Bool RemoveUserDefined(const char *Code);
	const char *GetUserDefined(const char *Code, char *Buffer, int Length) const;
	Bool IsUserDefined(const char *Code) const;

	void JumpbackPush(jumpback NewJB)
		{
		SYNCDATA(LE_JUMPBACKPUSH, &NewJB, 0);

		GAINEXCLUSIVEACCESS();

		if (jb)
			{
			jb_end = ++jb_end % jb_length;

			if (jb_end == jb_start)
				{
				jb_start++;
				}

			jb[jb_end] = NewJB;
			}

		RELEASEEXCLUSIVEACCESS();
		}


	Bool JumpbackPop(jumpback *Where)
		{
		SYNCDATAR(LE_JUMPBACKPOP, Where, 0);

		GAINEXCLUSIVEACCESS();

		// if we were not able to allocate jb, these will always be the same
		if (jb_start != jb_end)
			{
			*Where = jb[jb_end];

			if (--jb_end < 0)
				{
				jb_end = jb_length - 1;
				}

			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		else
			{
			RELEASEEXCLUSIVEACCESS();
			return (FALSE);
			}
		}
	};


// --------------------------------------------------------------------------
// All of this comes together in the LogEntry class, which stores everything
// known about the user.

class LogEntry : public LogEntry1, public LogEntry2, public LogEntry3,
		public LogEntry4, public LogEntry5, public LogEntry6,
		public LogExtensions
	{
public:
	Bool IsInGroup(g_slot GroupSlot) const; 	// overrides LOG2.H's
	h_slot GetDefaultHallIndex(void) const;

	LogEntry(r_slot NumRooms, g_slot NumGroups, int MaxJumpback);
	LogEntry(const LogEntry &Original);
	LogEntry& operator = (const LogEntry &Original);

	Bool Load(l_index Index);
#ifdef WINCIT
	void PauseSyncSend(void);
	void ResumeSyncSend(void);

	Bool LoadByName(const char *name, l_slot *LogOrder);
#else
	Bool LoadByName(const char *name);
#endif

#ifdef VISUALC
	Bool Save(l_index Index, r_slot Room);
#else
	Bool Save(l_index Index, r_slot Room) const;
#endif
	void Clear(void);

	Bool IsValid(void) const;

	Bool CanAccessRoom(r_slot RoomIndex, h_slot Hall = CERROR, Bool IgnoreHidden = FALSE) const;
	Bool CanModerateRoom(r_slot RoomIndex) const;
	Bool CanKillMessage(Message *Msg, Bool IsLoggedIn) const;
	Bool HasGroupAccessToRoom(r_slot roomslot) const;
	Bool CanAccessHall(h_slot hallslot) const;
	Bool HasRoomPrivileges(r_slot roomslot) const;
	Bool ShowSystemEvent(SystemEventE Event);
	MaySeeMsgType MaySeeMessage(Message *Msg);
	MaySeeMsgType MaySeeMessage(m_index ID);
	};

// And this is our in-RAM index of the users.
#include "logtab.h"

// log.cpp
#ifdef WINCIT
l_slot nodexists(const char *name, const l_slot *Order);
l_slot addressexists(const char *name, const l_slot *Order);
l_slot FindPersonByPartialName(const char *name, const l_slot *Order);
l_slot FindPersonByName(const char *name, const l_slot *Order);
#else
l_slot nodexists(const char *name, const l_slot *Order = NULL);
l_slot addressexists(const char *name, const l_slot *Order = NULL);
l_slot FindPersonByPartialName(const char *name, const l_slot *Order = NULL);
l_slot FindPersonByName(const char *name, const l_slot *Order = NULL);
#endif
void normalizepw(const char *InitPw, char *Initials, char *passWord);

// cyclelog.cpp
void cyclegroup(g_slot slot);
void cycleroom(r_slot slot, Bool GiveAccess, Bool ClearIgnore);


extern LogTable 		LogTab;

#ifdef MAIN
char *logextensions[] =
	{
	"KILLUSER",         "KILLTEXT",         "KILLNODE",
	"KILLREG",          "TAGUSER",          "MESSAGE",
	"DICTWORD",         "JB",               "FINGER",
	"USERDEF",          "REPLACE",

	NULL
	};
#else
extern char *logextensions[];
#endif

enum
	{
	LE_KUSER,			LE_KTEXT,			LE_KNODE,
	LE_KREG,			LE_TUSER,			LE_MESSAGE,
	LE_DICTWORD,		LE_JUMPBACK,		LE_FINGER,
	LE_USERDEF, 		LE_REPLACE,

	MAXLOGEXT,
	};
#endif
