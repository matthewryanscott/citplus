// --------------------------------------------------------------------------
// Citadel: ConSysop.H
//
// Stuff for doConsoleSysop().


enum ConSysopMainMenuE
	{
	CSMM_HALLS,		CSMM_ROOMS,		CSMM_USERS,		CSMM_GROUPS,
	CSMM_MESSAGES,	CSMM_HELPFILES,	CSMM_SYSMON,	CSMM_CONFIG,
	CSMM_DOSUTIL,	CSMM_EXIT,

	CSMM_NUM
	};

enum ConSysopHallMenuE
	{
	CSHM_NEW,		CSHM_EDIT,		CSHM_KILL,		CSHM_ROOMS,
	CSHM_MOVEBACK,	CSHM_MOVEUP,	CSHM_EXIT,

	CSHM_NUM
	};

enum ConSysopHallRoomMenuE
	{
	CSHRM_ADDRMV,	CSHRM_WINUNWIN,	CSHRM_MOVEBACK,	CSHRM_MOVEUP,
	CSHRM_PREVHALL,	CSHRM_NEXTHALL,	CSHRM_EXIT,

	CSHRM_NUM
	};

enum ConSysopRoomMenuE
	{
	CSRM_NEW,		CSRM_EDIT,		CSRM_KILL,		CSRM_MOVEBACK,
	CSRM_MOVEUP,	CSRM_PREVHALL,	CSRM_NEXTHALL,	CSRM_EXIT,

	CSRM_NUM
	};

enum ConSysopUserMenuE
	{
	CSUM_NEW,		CSUM_EDIT,		CSUM_KILL,		CSUM_EXIT,

	CSUM_NUM
	};

enum ConSysopGroupMenuE
	{
	CSGM_NEW,		CSGM_EDIT,		CSGM_KILL,		CSGM_EXIT,

	GSGM_NUM
	};

enum ConSysopHelpMenuE
	{
	CSHPM_EDIT,		CSHPM_INFO,		CSHPM_EXIT,

	CSHPM_NUM
	};

enum ConSysopDosMenuE
	{
	CSDM_COPY,		CSDM_MOVE,		CSDM_DELETE,	CSDM_RENAME,
	CSDM_VIEW,		CSDM_MAKEDIR,	CSDM_RMVDIR,	CSDM_MEMDUMP,
	CSDM_EXIT,

	CSDM_NUM
	};

enum ConSysopSysMonMenuE
	{
	CSSMM_KEYBUF,	CSSMM_SERPORT,	CSSMM_CRON,		CSSMM_EXIT,

	CSSMM_NUM
	};


enum CONSYSOPMENUS
	{
	CS_MAIN,	CS_HALL,	CS_HALLROOM,	CS_ROOM,	CS_USER,
	CS_GROUP,	CS_HELP,	CS_HELPFILES,	CS_DOSUTIL,
	};

enum LISTCMDS
	{
	LST_DRAW,	LST_UP, 	LST_DOWN,	LST_GET,	LST_PGUP,	LST_PGDN,
	LST_HOME,	LST_END,	LST_NEW,	LST_DESTROY,LST_REFRESH,
	};

struct menuWindowData
	{
	CONSYSOPMENUS menuNumber;			// which menu
	int curSelection;					// currently selected
	int numEntries; 					// how many entries
	};

struct listerData
	{
	const char *(*lister)(CITWINDOW *, LISTCMDS, char *, int);
										// the lister to use
	int on; 							// which entry is on
	int top;							// which entry is at top
	int bottom; 						// which entry is at bottom
	void *ptr;							// in case you need more data
	};


// consysop.cpp
Bool listHandler(EVENT evt, long param, int more, CITWINDOW *wnd);

// csysdosu.cpp
const char *fileLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length);
Bool DOSUtilKeyHandler(CITWINDOW *wnd, int key);

// csysgrp.cpp
const char *groupLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length);
void GroupControlHandler(CITWINDOW *wnd, long param);
Bool GroupKeyHandler(CITWINDOW *wnd, int key);

// csyshall.cpp
const char *hallLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length);
void HallControlHandler(CITWINDOW *wnd, long param);
Bool HallKeyHandler(CITWINDOW *wnd, int key);

// csyshelp.cpp
Bool HelpKeyHandler(CITWINDOW *wnd, int key);

// csysmon.cpp
Bool SysMonKeyHandler(CITWINDOW *wnd, int key);

// csysroom.cpp
const char *roomLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length);
void RoomControlHandler(CITWINDOW *wnd, long param);
Bool RoomKeyHandler(CITWINDOW *wnd, int key);

// csysuser.cpp
const char *userLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length);
void UserControlHandler(CITWINDOW *wnd, long param);
Bool UserKeyHandler(CITWINDOW *wnd, int key);


