// --------------------------------------------------------------------------
// Citadel: SyncData.H
//
// Used by the Windows version for synchronization of data across threads

#ifndef _SyncData_H
#define _SyncData_H

#ifdef WINCIT

struct LogEntry1List
	{
	LogEntry1List *next;
	LogEntry1Data *L;
	};

struct LogEntry2List
	{
	LogEntry2List *next;
	LogEntry2 *L;
	};

struct LogEntry3List
	{
	LogEntry3List *next;
	LogEntry3 *L;
	};

struct LogEntry4List
	{
	LogEntry4List *next;
	LogEntry4 *L;
	};

struct LogEntry5List
	{
	LogEntry5List *next;
	LogEntry5 *L;
	};

struct LogEntry6List
	{
	LogEntry6List *next;
	LogEntry6 *L;
	};

struct LogExtensionsList
	{
	LogExtensionsList *next;
	LogExtensions *L;
	};

struct RoomCList
	{
	RoomCList *next;
	RoomC *R;
	};

typedef void * SYNCTYPE;

enum SyncActionE
	{
	L1_FIRST,
	L1_SETNAME = L1_FIRST,	L1_SETINITIALS,			L1_SETPASSWORD,
	L1_SETSURNAME,			L1_SETTITLE,			L1_SETREALNAME,
	L1_SETPHONENUM,			L1_SETFORWARDADDR,		L1_SETFORWARDADDRNODE,
	L1_SETFORWARDADDRREG,	L1_SETPROMPTFORMAT,		L1_SETDATESTAMP,
	L1_SETVERBOSEDATESTAMP,	L1_SETSIGNATURE,		L1_SETNETPREFIX,
	L1_SETMAILADDR1,		L1_SETMAILADDR2,		L1_SETMAILADDR3,
	L1_SETALIAS,			L1_SETLOCID,			L1_SETMOREPROMPT,
	L1_SETOCCUPATION,		L1_SETWHEREHEAR,		L1_SETLASTROOM,
	L1_SETLASTHALL,			L1_SETDEFAULTROOM,		L1_SETDEFAULTHALL,
	L1_SETTERMTYPE,			L1_SETBIRTHDATE,		L1_SETFIRSTON,
	L1_SETSEX,				L1_SETNULLS,			L1_SETWIDTH,
	L1_SETLINESPERSCREEN,	L1_SETATTRIBUTE,		L1_SETNUMUSERSHOW,
	L1_SETDEFAULTPROTOCOL,	L1_SETCALLTIME,			L1_SETCALLNUMBER,
	L1_SETTOTALTIME,		L1_SETCREDITS,			L1_SETLOGINS,
	L1_SETPOSTED,			L1_SETREAD,				L1_SETPASSWORDCHANGETIME,
	L1_SETCALLSTODAY,		L1_SETCALLLIMIT,		L1_SETLASTMESSAGE,
	L1_SETDLBYTES,			L1_SETULBYTES,			L1_SETDLNUM,
	L1_SETULNUM,			L1_SETPOOPCOUNT,		L1_SETDUNGEONED,
	L1_SETFORWARDTONODE,	L1_SETAUTONEXTHALL,		L1_SETENTERBORDERS,
	L1_SETVERIFIED,			L1_SETSURNAMELOCKED,	L1_SETDEFAULTHALLLOCKED,
	L1_SETPSYCHO,			L1_SETVIEWTITLESURNAME,	L1_SETVIEWSUBJECTS,
	L1_SETVIEWSIGNATURES,	L1_SETOLDIBMGRAPH,		L1_SETOLDIBMANSI,
	L1_SETOLDIBMCOLOR,		L1_SETTWIRLY,			L1_SETAUTOVERBOSE,
	L1_SETPAUSEBETWEENMSGS,	L1_SETMINIBIN,			L1_SETCLEARSCREENMSGS,
	L1_SETVIEWROOMINFO,		L1_SETVIEWHALLDESC,		L1_SETVERBOSECONTINUE,
	L1_SETVIEWCENSORED,		L1_SETVIEWBORDERS,		L1_SETOUT300,
	L1_SETUSERSIGLOCKED,	L1_SETHIDEMSGEXCL,		L1_SETDOWNLOAD,
	L1_SETUPLOAD,			L1_SETCHAT,				L1_SETPRINTFILE,
	L1_SETSPELLCHECKMODE,	L1_SETMAKEROOM,			L1_SETVERBOSELOGOUT,
	L1_SETCONFIRMSAVE,		L1_SETCONFIRMABORT,		L1_SETCONFIRMNOEO,
	L1_SETUSEPERSONALHALL,	L1_SETYOUAREHERE,		L1_SETIBMROOM,
	L1_SETWIDEROOM,			L1_SETMUSIC,			L1_SETCHECKAPOSTROPHES,
	L1_SETCHECKALLCAPS,		L1_SETCHECKDIGITS,		L1_SETEXCLUDEENCRYPTED,
	L1_SETVIEWCOMMAS,		L1_SETPUNPAUSES,		L1_SETROMAN,
	L1_MOREPOOP,			L1_SETINUSE,			L1_SETUPPERONLY,
	L1_SETLINEFEEDS,		L1_SETEXPERT,			L1_SETAIDE,
	L1_SETTABS,				L1_SETOLDTOO,			L1_SETPROBLEM,
	L1_SETUNLISTED,			L1_SETPERMANENT,		L1_SETSYSOP,
	L1_SETNODE,				L1_SETNETUSER,			L1_SETACCOUNTING,
	L1_SETMAIL,				L1_SETVIEWROOMDESC,		L1_SETSUPERSYSOP,
	L1_SETBUNNY,            L1_SSE_LOGONOFF,        L1_SSE_NEWMSG,
	L1_SSE_EXCLMSG,         L1_SSE_CHATALL,         L1_SSE_CHATROOM,
	L1_SSE_CHATGROUP,		L1_SSE_CHATUSER,        L1_SSE_RMINOUT,
	L1_SETBANNED,			L1_SETKILLOWN,			L1_SETSEEOWNCHATS,     
    L1_SETERASEPROMPT,      L1_SETAUTOIDLESECONDS,  L1_SETHEARLAUGHTER, 
	      	
    L1_LAST = L1_SETHEARLAUGHTER,

	L2_SETINGROUP,			L3_SETINROOM,			L4_SETROOMEXCLUDED,
	L5_SETROOMNEWPOINTER,	L6_SETROOMINPERSONALHALL,

	LE_FIRST,				LE_SETMESSAGEROOM = LE_FIRST,
	LE_SETMESSAGE,			LE_CLEARMESSAGE,		LE_SETFINGER,
	LE_ADDKILL,				LE_REMOVEKILL,			LE_ADDTAGUSER,
	LE_ADDREPLACE,			LE_ADDWORDTODICT,		LE_REMOVEUSERTAG,
	LE_REMOVEREPLACE,		LE_REMOVEWORDFROMDICT,	LE_SETUSERDEFINED,
	LE_REMOVEUSERDEFINED,	LE_JUMPBACKPUSH,		LE_JUMPBACKPOP,
	LE_LAST = LE_JUMPBACKPOP,

	RM_FIRST,
	RM_SETINUSE = RM_FIRST,	RM_SETPUBLIC,			RM_SETMSDOSDIR,
	RM_SETPERMANENT,		RM_SETGROUPONLY,		RM_SETREADONLY,
	RM_SETDOWNLOADONLY,		RM_SETSHARED,			RM_SETMODERATED,
	RM_SETAPPLICATIONB,		RM_SETBIO,				RM_SETUPLOADONLY,
	RM_SETPRIVILEGEDGROUP,	RM_SETANONYMOUS,		RM_SETSUBJECT,
	RM_SETGROUPMODERATES,	RM_SETARCHIVE,			RM_SETBOOLEANGROUP,
	RM_SETNAME,				RM_SETDIRECTORY,		RM_SETDESCFILE,
	RM_SETAPPLICATIONS,		RM_SETGROUP,			RM_SETPGROUP,
	RM_SETAUTOAPP,			RM_SETUNEXCLUDABLE,		RM_SETINFOLINE,
	RM_SETNETID,			RM_SETARCHIVEFILE,		RM_SETDICTIONARY,
	RM_SETGROUPEXPRESSION,	RM_SETCREATOR,
	RM_LAST = RM_SETCREATOR,
	};

class SynchronizerC
	{
	HANDLE SyncMutex;
	Bool Busy;

	LogEntry1List *L1;
	LogEntry2List *L2;
	LogEntry3List *L3;
	LogEntry4List *L4;
	LogEntry5List *L5;
	LogEntry6List *L6;

	LogExtensionsList *LE;

	RoomCList *R;

public:
	SynchronizerC(void)
		{
		L1 = NULL;
		L2 = NULL;
		L3 = NULL;
		L4 = NULL;
		L5 = NULL;
		L6 = NULL;
		LE = NULL;
		R = NULL;

		Busy = FALSE;

		SyncMutex = CreateMutex(NULL, FALSE, "WincitMutex:Sync");
		}

	~SynchronizerC(void)
		{
		disposeLL((void **) &L1);
		disposeLL((void **) &L2);
		disposeLL((void **) &L3);
		disposeLL((void **) &L4);
		disposeLL((void **) &L5);
		disposeLL((void **) &L6);
		disposeLL((void **) &LE);
		disposeLL((void **) &R);

		CloseHandle(SyncMutex);
		}

	Bool Add(LogEntry1Data *L);
	Bool Add(LogEntry2 *L);
	Bool Add(LogEntry3 *L);
	Bool Add(LogEntry4 *L);
	Bool Add(LogEntry5 *L);
	Bool Add(LogEntry6 *L);
	Bool Add(LogExtensions *L);
	Bool Add(RoomC *R);

	Bool Remove(LogEntry1Data *L);
	Bool Remove(LogEntry2 *L);
	Bool Remove(LogEntry3 *L);
	Bool Remove(LogEntry4 *L);
	Bool Remove(LogEntry5 *L);
	Bool Remove(LogEntry6 *L);
	Bool Remove(LogExtensions *L);
	Bool Remove(RoomC *R);

	void *FindMatching(LogEntry1Data *L);
	void *FindMatching(LogEntry2 *L);
	void *FindMatching(LogEntry3 *L);
	void *FindMatching(LogEntry4 *L);
	void *FindMatching(LogEntry5 *L);
	void *FindMatching(LogEntry6 *L);
	void *FindMatching(LogExtensions *L);
	void *FindMatching(RoomC *R);

	Bool Synchronize(int SyncNumber, SyncActionE Action, SYNCTYPE D1,
			SYNCTYPE D2, Bool *RetVal = NULL);
	};

extern SynchronizerC Sync;

#define SYNCDATA(x,y,z)																		\
	if (!SyncSendPaused && Sync.Synchronize(SyncNumber, x, (SYNCTYPE) (y), (SYNCTYPE) (z)))	\
		{																					\
		return;																				\
		}

#define SYNCDATAR(x,y,z)																		\
	Bool R;																						\
	if (!SyncSendPaused && Sync.Synchronize(SyncNumber, x, (SYNCTYPE) (y), (SYNCTYPE) (z), &R))	\
		{																						\
		return (R);																				\
		}

#define ADDSYNC() Sync.Add(this)
#define REMOVESYNC() Sync.Remove(this)
#else
#define SYNCDATA(x,y,z)
#define SYNCDATAR(x,y,z)
#define ADDSYNC()
#define REMOVESYNC()
#endif
#endif
