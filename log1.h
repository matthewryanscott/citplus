// --------------------------------------------------------------------------
// Citadel: Log1.H
//
// This is most of the information we know about each user. This is stored in
// LOG.DAT. See also LOG2.H, LOG3.H, LOG4.H, LOG5.H, and LOG6.H for the
// classes used to create LOG2.DAT, LOG3.DAT, LOG4.DAT, LOG5.DAT, and
// LOG6.DAT. Citadel also keeps an index to LOG.DAT for quick access to data
// it needs often about users. The structure of this index is in LOGTAB.H.

// This is separate from LogEntry1 so we can clear it quickly by just doing
// a memset() on it. Because LogStarter has a virtual function, doing a
// memset() on LogEntry1 overwrites the function pointer to it.
class LogEntry1Data : public LogFlags
	{
	EXCLUSIVEACCESSRECORD()

#ifdef WINCIT
	l_index SyncNumber;
	Bool	SyncSendPaused;
#endif

	label	Name;					// caller's name
	label	Initials;				// caller's initials
	label	Password;				// caller's password
	label	Surname;				// users surname
	label	Title;					// users title
	label	RealName;				// Caller's real name.
	label	PhoneNumber;			// Caller's phone number.

	label	ForwardAddr;			// forwarding address (user)
	label	ForwardAddrNode;		// forwarding address (node) or xxx.xxxx
	label	ForwardAddrRegion;		// forwarding address (region)

	char	PromptFormat[64];		// prompt format string
	char	DateStamp[64];			// time/date format string
	char	VerboseDateStamp[64];	// time/date format string
	char	Signature[91];			// user's signature line
	label	NetPrefix;				// Prefix for networked rooms
	label	MailAddr1;				// Mailing address...
	label	MailAddr2;				// Mailing address...
	label	MailAddr3;				// Mailing address...
	char	Alias[4];				// ALIAS.LOCID.PLANET
	char	LocID[5];
	char	MorePrompt[80]; 		// User configurable more prompt
	label	Occupation; 			// What you do.
	char	WhereHear[80];			// Where did you hear about us?

	label	LastRoom;				// room they were in last call
	label	LastHall;				// hall they were in last call
	label	DefaultRoom;			// default room
	label	DefaultHall;			// default hall

	label	TermType;				// Terminal type

	char	DefaultProtocol;		// Default protocol

	long	BirthDate;				// midnight of birthdate
	long	FirstOn;				// first time on the system
	SexE	Sex;					// Oh so very sexy

	int 	Nulls;					// #Nulls
	int 	Width;					// terminal width
	int 	LinesPerScreen; 		// the number of lines per screen
	int 	attributes[5];			// bold, inverse, blink, underline

	int 	NumUserShow;			// # users to show on login

	time_t	CallTime;				// Time/date stamp of last call
	ulong	CallNumber; 			// Which caller # they were
	time_t	TotalTime;				// Total seconds on the system.
	long	Credits;				// Credits for accounting
	long	Logins; 				// How many times.
	long	Posted; 				// Number of messages posted.
	long	Read;					// Number of messages read.
	time_t	PasswordChangeTime; 	// The last time the PW was changed.
	int 	CallsToday; 			// Number of calls today (mid-mid).
	int 	CallLimit;				// Number of calls allowed per day

	m_index	LastMessage;			// newest message on system last call

	ulong	DL_Bytes;
	ulong	UL_Bytes;
	uint	DL_Num;
	uint	UL_Num;

	long	Poopcount;				// How many times have they pooped?

	Bool	DUNGEONED	: 1;		// dungeoned user?
	Bool	FORtOnODE	: 1;		// forward to node?
	Bool	NEXTHALL	: 1;		// Auto Next Hall?
	Bool	BORDERS 	: 1;		// Can they enter borders
	Bool	UNVERIFIED	: 1;		// has the user been verified?
	Bool	SURNAMLOK	: 1;		// surname locked?
	Bool	LOCKHALL	: 1;		// lock default hall
	Bool	PSYCHO		: 1;		// make life a pain in the ass for him
	Bool	DISPLAYTS	: 1;		// display titles/surnames?
	Bool	SUBJECTS	: 1;		// display subjects?
	Bool	SIGNATURES	: 1;		// display signatures?
	Bool	oldIBMGRAPH : 1;		// Old terminal : IBM graphics characters?
	Bool	oldIBMANSI	: 1;		// Old terminal : IBM ansi enabled?
	Bool	oldIBMCOLOR : 1;		// Old terminal : ISO-Color?
	Bool	TWIRLY		: 1;		// twirling cursor at prompts?
	Bool	VERBOSE 	: 1;		// Auto-verbose flag?
	Bool	MSGPAUSE	: 1;		// Pause after each message.
	Bool	MINIBIN 	: 1;		// MiniBin login stats?
	Bool	MSGCLS		: 1;		// Clear screen after message?
	Bool	ROOMINFO	: 1;		// Display room info lines?
	Bool	HALLTELL	: 1;		// Display hall descriptions?
	Bool	VERBOSECONT : 1;		// continue in the icky verbose way?
	Bool	VIEWCENSOR	: 1;		// View censored messages?
	Bool	SEEBORDERS	: 1;		// Can they see borders?
	Bool	OUT300		: 1;		// simulate 300 baud output
	Bool	LOCKUSIG	: 1;		// user signature locked?
	Bool	HIDEEXCL	: 1;		// hide msg exclusion?
	Bool	NODOWNLOAD	: 1;		// user cannot download
	Bool	NOUPLOAD	: 1;		// user cannot upload
	Bool	NOCHAT		: 1;		// user cannot chat?
	Bool	PRINTFILE	: 1;		// turn on printfile when user logs in?
	uint	spellCheck	: 2;		// how the user likes to spell check
	Bool	NOMAKEROOM	: 1;		// if user cannot .ER
	Bool	VERBOSELO	: 1;		// do it verbosely...
	Bool	CONFIRMSAVE : 1;		// confirm save from msg editor?
	Bool	NOCONFABORT : 1;		// don't confirm abort from msg editor
	Bool	CONFIRMNOEO : 1;		// confirm not .EO after abort
	Bool	USEPERSONAL : 1;		// Use personal hall?
	Bool	YOUAREHERE	: 1;		// Dargos is such a God.
	Bool	IBMROOM 	: 1;		// Fred is a genius. Centauri, too.
	Bool	WIDEROOM	: 1;		// ...
	Bool	MUSIC		: 1;		// user can hear it?
	Bool	CHECKAPS	: 1;		// check 'S in spell checker
	Bool	CHECKALLCAP : 1;		// check all caps in spell checker
	Bool	CHECKDIGITS : 1;		// check digits in spell checker
	Bool	EXCLUDECRYPT: 1;		// Exclude encrypted messages.
	Bool	HIDECOMMAS	: 1;		// how's this for user-configurability?
	Bool	PUNPAUSES	: 1;		// Some people acutally want it to...
	Bool	ROMAN		: 1;		// Ho ho ho.
	Bool	SUPERSYSOP	: 1;		// Super Sysop to the rescue!
	Bool	NOBUNNY		: 1;		// No bunny filter for me, mom!
	Bool	HSE_LOGONOFF: 1;		// Hide SE_LOGONOFF
	Bool	HSE_NEWMSG	: 1;		// Hide SE_NEWMESSAGE
	Bool	HSE_EXCLMSG	: 1;		// Hide SE_EXCLUSIVEMESSAGE
	Bool	HSE_CHATALL	: 1;		// Hide SE_CHATALL
	Bool	HSE_CHATROOM: 1;		// Hide SE_CHATROOM
	Bool	HSE_CHATGRP	: 1;		// Hide SE_CHATGROUP
	Bool	HSE_CHATUSER: 1;		// Hide SE_CHATUSER
	Bool	SSE_RMINOUT	: 1;		// Show SE_ROOMINOUT
	Bool	BANNED		: 1;		// Ban user, send port to EXTERNAL.CIT on attempted login
	Bool	NOKILLOWN	: 1;		// Cannot kill own messages

	Bool    SEEOWNCHATS : 1;        // Can you see your own chats?
	Bool    ERASEPROMPT : 1;        // Erase room prompt before event display?
    Bool    HEARLAUGHTER: 1;        // dude
    int     AutoIdleSeconds;     	// # seconds before entering idle mode

public:
#ifdef WINCIT
	void SetSyncNumber(l_index New)
		{
		SyncNumber = New;
		}

	int GetSyncNumber(void)
		{
		return (SyncNumber);
		}

	void PauseSyncSend(void)
		{
		SyncSendPaused = TRUE;
		}

	void ResumeSyncSend(void);

	void SetInuse(Bool New)
		{
		SYNCDATA(L1_SETINUSE, New, 0);
		INUSE = !!New;
		}

	void SetUpperOnly(Bool New)
		{
		SYNCDATA(L1_SETUPPERONLY, New, 0);
		UCMASK = !!New;
		}

	void SetLinefeeds(Bool New)
		{
		SYNCDATA(L1_SETLINEFEEDS, New, 0);
		LFMASK = !!New;
		}

	void SetExpert(Bool New)
		{
		SYNCDATA(L1_SETEXPERT, New, 0);
		EXPERT = !!New;
		}

	void SetAide(Bool New)
		{
		SYNCDATA(L1_SETAIDE, New, 0);
		AIDE = !!New;
		}

	void SetTabs(Bool New)
		{
		SYNCDATA(L1_SETTABS, New, 0);
		TABS = !!New;
		}

	void SetOldToo(Bool New)
		{
		SYNCDATA(L1_SETOLDTOO, New, 0);
		OLDTOO = !!New;
		}

	void SetProblem(Bool New)
		{
		SYNCDATA(L1_SETPROBLEM, New, 0);
		PROBLEM = !!New;
		}

	void SetUnlisted(Bool New)
		{
		SYNCDATA(L1_SETUNLISTED, New, 0);
		UNLISTED = !!New;
		}

	void SetPermanent(Bool New)
		{
		SYNCDATA(L1_SETPERMANENT, New, 0);
		PERMANENT = !!New;
		}

	void SetSysop(Bool New)
		{
		SYNCDATA(L1_SETSYSOP, New, 0);
		SYSOP = !!New;
		}

	void SetNode(Bool New)
		{
		SYNCDATA(L1_SETNODE, New, 0);
		NODE = !!New;
		}

	void SetNetUser(Bool New)
		{
		SYNCDATA(L1_SETNETUSER, New, 0);
		NETUSER = !!New;
		}

	void SetAccounting(Bool New)
		{
		SYNCDATA(L1_SETACCOUNTING, New, 0);
		NOACCOUNT = !New;
		}

	void SetMail(Bool New)
		{
		SYNCDATA(L1_SETMAIL, New, 0);
		NOMAIL = !New;
		}

	void SetViewRoomDesc(Bool New)
		{
		SYNCDATA(L1_SETVIEWROOMDESC, New, 0);
		ROOMTELL = !!New;
		}
#endif

	void CopyLog1Data(const LogEntry1Data *Src);

	void Clear(void)
		{
		GAINEXCLUSIVEACCESS();
		VerifyHeap();

		SAVEEXCLUSIVEACCESSRECORD();

		memset(this, 0, sizeof(*this));
		WINCODE(SyncNumber = CERROR);

		RESTOREEXCLUSIVEACCESSRECORD();

		Width = 80;
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	LogEntry1Data(void)
		{
		INITIALIZEEXCLUSIVEACCESS(TRUE);
		Clear();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry1Data(void)
		{
		REMOVESYNC();
		DESTROYEXCLUSIVEACCESS();
		}

	void Verify(void);

	const char *GetName(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, *Name ? Name : getmsg(529), Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetName(const char *New)
		{
		SYNCDATA(L1_SETNAME, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Name, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetInitials(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Initials, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetInitials(const char *New)
		{
		SYNCDATA(L1_SETINITIALS, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Initials, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetPassword(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Password, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetPassword(const char *New)
		{
		SYNCDATA(L1_SETPASSWORD, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Password, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetSurname(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Surname, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetSurname(const char *New)
		{
		SYNCDATA(L1_SETSURNAME, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Surname, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetTitle(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Title, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetTitle(const char *New)
		{
		SYNCDATA(L1_SETTITLE, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Title, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetRealName(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, RealName, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetRealName(const char *New)
		{
		SYNCDATA(L1_SETREALNAME, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(RealName, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetPhoneNumber(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, PhoneNumber, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetPhoneNumber(const char *New)
		{
		SYNCDATA(L1_SETPHONENUM, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(PhoneNumber, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetForwardAddr(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, ForwardAddr, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetForwardAddr(const char *New)
		{
		SYNCDATA(L1_SETFORWARDADDR, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(ForwardAddr, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetForwardAddrNode(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, ForwardAddrNode, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetForwardAddrNode(const char *New)
		{
		SYNCDATA(L1_SETFORWARDADDRNODE, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(ForwardAddrNode, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetForwardAddrRegion(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, ForwardAddrRegion, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetForwardAddrRegion(const char *New)
		{
		SYNCDATA(L1_SETFORWARDADDRREG, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(ForwardAddrRegion, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetPromptFormat(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, PromptFormat, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetPromptFormat(const char *New)
		{
		SYNCDATA(L1_SETPROMPTFORMAT, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(PromptFormat, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDateStamp(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, DateStamp, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetDateStamp(const char *New)
		{
		SYNCDATA(L1_SETDATESTAMP, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(DateStamp, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetVerboseDateStamp(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, VerboseDateStamp, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetVerboseDateStamp(const char *New)
		{
		SYNCDATA(L1_SETVERBOSEDATESTAMP, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(VerboseDateStamp, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetSignature(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Signature, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetSignature(const char *New)
		{
		SYNCDATA(L1_SETSIGNATURE, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Signature, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetNetPrefix(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, NetPrefix, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetNetPrefix(const char *New)
		{
		SYNCDATA(L1_SETNETPREFIX, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(NetPrefix, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetMailAddr1(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, MailAddr1, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetMailAddr1(const char *New)
		{
		SYNCDATA(L1_SETMAILADDR1, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(MailAddr1, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetMailAddr2(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, MailAddr2, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetMailAddr2(const char *New)
		{
		SYNCDATA(L1_SETMAILADDR2, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(MailAddr2, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetMailAddr3(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, MailAddr3, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetMailAddr3(const char *New)
		{
		SYNCDATA(L1_SETMAILADDR3, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(MailAddr3, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetAlias(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Alias, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetAlias(const char *New)
		{
		SYNCDATA(L1_SETALIAS, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Alias, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetLocID(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, LocID, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetLocID(const char *New)
		{
		SYNCDATA(L1_SETLOCID, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(LocID, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetMorePrompt(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, MorePrompt, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetMorePrompt(const char *New)
		{
		SYNCDATA(L1_SETMOREPROMPT, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(MorePrompt, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetOccupation(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Occupation, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetOccupation(const char *New)
		{
		SYNCDATA(L1_SETOCCUPATION, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Occupation, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetWhereHear(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, WhereHear, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetWhereHear(const char *New)
		{
		SYNCDATA(L1_SETWHEREHEAR, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(WhereHear, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetLastRoom(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, LastRoom, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetLastRoom(const char *New)
		{
		SYNCDATA(L1_SETLASTROOM, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(LastRoom, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetLastHall(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, LastHall, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetLastHall(const char *New)
		{
		SYNCDATA(L1_SETLASTHALL, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(LastHall, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDefaultRoom(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, DefaultRoom, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	Bool IsDefaultRoom(const char *TestRoom) const
		{
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(DefaultRoom, TestRoom);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	void SetDefaultRoom(const char *New)
		{
		SYNCDATA(L1_SETDEFAULTROOM, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(DefaultRoom, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDefaultHall(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, DefaultHall, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	Bool IsDefaultHall(const char *TestHall) const
		{
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(DefaultHall, TestHall);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	void SetDefaultHall(const char *New)
		{
		SYNCDATA(L1_SETDEFAULTHALL, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(DefaultHall, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetTermType(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, TermType, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetTermType(const char *New)
		{
		SYNCDATA(L1_SETTERMTYPE, New, 0);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(TermType, New);
		RELEASEEXCLUSIVEACCESS();
		}

	long GetBirthDate(void) const
		{
		return (BirthDate);
		}

	void SetBirthDate(long New)
		{
		SYNCDATA(L1_SETBIRTHDATE, New, 0);

		BirthDate = New;
		}

	long GetFirstOn(void) const
		{
		return (FirstOn);
		}

	void SetFirstOn(long New)
		{
		SYNCDATA(L1_SETFIRSTON, New, 0);

		FirstOn = New;
		}

	SexE GetSex(void) const
		{
		return (Sex);
		}

	void SetSex(SexE New)
		{
		SYNCDATA(L1_SETSEX, New, 0);

		Sex = New;
		}

	int GetNulls(void) const
		{
		return (Nulls);
		}

	void SetNulls(int New)
		{
		SYNCDATA(L1_SETNULLS, New, 0);

		Nulls = New;
		}

	int GetWidth(void) const
		{
		return (Width);
		}

	void SetWidth(int New)
		{
		SYNCDATA(L1_SETWIDTH, New, 0);

		Width = New;
		}

	int GetLinesPerScreen(void) const
		{
		return (LinesPerScreen);
		}

	void SetLinesPerScreen(int New)
		{
		SYNCDATA(L1_SETLINESPERSCREEN, New, 0);

		LinesPerScreen = New;
		}

	int GetAttribute(LogAttr Which) const
		{
		assert(Which >= 0);
		assert(Which < ATTR_NUM);

		return (attributes[Which]);
		}

	void SetAttribute(LogAttr Which, int New)
		{
		SYNCDATA(L1_SETATTRIBUTE, Which, New);

		assert(Which >= 0);
		assert(Which < ATTR_NUM);
		assert(New >= 0);
		assert(New < 256);

		attributes[Which] = New;
		}

	void SetDefaultColors(Bool Color)
		{
		if (Color)
			{
			SetAttribute(ATTR_BOLD, 14);
			SetAttribute(ATTR_BLINK, 128);
			SetAttribute(ATTR_NORMAL, 15);
			SetAttribute(ATTR_REVERSE, 78);
			SetAttribute(ATTR_UNDERLINE, 13);
			}
		else
			{
			SetAttribute(ATTR_BOLD, 8);
			SetAttribute(ATTR_BLINK, 128);
			SetAttribute(ATTR_NORMAL, 7);
			SetAttribute(ATTR_REVERSE, 112);
			SetAttribute(ATTR_UNDERLINE, 1);
			}
		}

	int GetNumUserShow(void) const
		{
		return (NumUserShow);
		}

	void SetNumUserShow(int New)
		{
		SYNCDATA(L1_SETNUMUSERSHOW, New, 0);

		NumUserShow = New;
		}

	char GetDefaultProtocol(void) const
		{
		return (DefaultProtocol);
		}

	void SetDefaultProtocol(char New)
		{
		SYNCDATA(L1_SETDEFAULTPROTOCOL, New, 0);

		DefaultProtocol = New;
		}

	time_t GetCallTime(void) const
		{
		return (CallTime);
		}

	void SetCallTime(time_t New)
		{
		SYNCDATA(L1_SETCALLTIME, New, 0);

		CallTime = New;
		}

	ulong GetCallNumber(void) const
		{
		return (CallNumber);
		}

	void SetCallNumber(ulong New)
		{
		SYNCDATA(L1_SETCALLNUMBER, New, 0);

		CallNumber = New;
		}

	time_t GetTotalTime(void) const
		{
		return (TotalTime);
		}

	void SetTotalTime(time_t New)
		{
		SYNCDATA(L1_SETTOTALTIME, New, 0);

		TotalTime = New;
		}

	long GetCredits(void) const
		{
		return (Credits);
		}

	void SetCredits(long New)
		{
		SYNCDATA(L1_SETCREDITS, New, 0);

		Credits = New;
		}

	long GetLogins(void) const
		{
		return (Logins);
		}

	void SetLogins(long New)
		{
		SYNCDATA(L1_SETLOGINS, New, 0);

		Logins = New;
		}

	long GetPosted(void) const
		{
		return (Posted);
		}

	void SetPosted(long New)
		{
		SYNCDATA(L1_SETPOSTED, New, 0);

		Posted = New;
		}

	long GetRead(void) const
		{
		return (Read);
		}

	void SetRead(long New)
		{
		SYNCDATA(L1_SETREAD, New, 0);

		Read = New;
		}

	time_t GetPasswordChangeTime(void) const
		{
		return (PasswordChangeTime);
		}

	void SetPasswordChangeTime(time_t New)
		{
		SYNCDATA(L1_SETPASSWORDCHANGETIME, New, 0);

		PasswordChangeTime = New;
		}

	int GetCallsToday(void) const
		{
		return (CallsToday);
		}

	void SetCallsToday(int New)
		{
		SYNCDATA(L1_SETCALLSTODAY, New, 0);

		CallsToday = New;
		}

	int GetCallLimit(void) const
		{
		return (CallLimit);
		}

	void SetCallLimit(int New)
		{
		SYNCDATA(L1_SETCALLLIMIT, New, 0);

		CallLimit = New;
		}

	m_index GetLastMessage(void) const
		{
		return (LastMessage);
		}

	void SetLastMessage(m_index New)
		{
		SYNCDATA(L1_SETLASTMESSAGE, New, 0);

		LastMessage = New;
		}

	ulong GetDL_Bytes(void) const
		{
		return (DL_Bytes);
		}

	void SetDL_Bytes(ulong New)
		{
		SYNCDATA(L1_SETDLBYTES, New, 0);

		DL_Bytes = New;
		}

	ulong GetUL_Bytes(void) const
		{
		return (UL_Bytes);
		}

	void SetUL_Bytes(ulong New)
		{
		SYNCDATA(L1_SETULBYTES, New, 0);

		UL_Bytes = New;
		}

	uint GetDL_Num(void) const
		{
		return (DL_Num);
		}

	void SetDL_Num(uint New)
		{
		SYNCDATA(L1_SETDLNUM, New, 0);

		DL_Num = New;
		}

	uint GetUL_Num(void) const
		{
		return (UL_Num);
		}

	void SetUL_Num(uint New)
		{
		SYNCDATA(L1_SETULNUM, New, 0);

		UL_Num = New;
		}

	long GetPoopcount(void) const
		{
		return (Poopcount);
		}

	void SetPoopcount(long New)
		{
		SYNCDATA(L1_SETPOOPCOUNT, New, 0);

		Poopcount = New;
		}

	void AdjustPoopcount(long Adjust)
		{
		SetPoopcount(Poopcount + Adjust);
		}

	void MorePoop(long ToAdd)
		{
		SYNCDATA(L1_MOREPOOP, ToAdd, 0);

		Poopcount += ToAdd;
		}

	Bool IsDungeoned(void) const
		{
		return (DUNGEONED);
		}

	void SetDungeoned(Bool New)
		{
		SYNCDATA(L1_SETDUNGEONED, New, 0);

		DUNGEONED = !!New;
		}

	Bool IsForwardToNode(void) const
		{
		return (FORtOnODE);
		}

	void SetForwardToNode(Bool New)
		{
		SYNCDATA(L1_SETFORWARDTONODE, New, 0);

		FORtOnODE = !!New;
		}

	Bool IsAutoNextHall(void) const
		{
		return (NEXTHALL);
		}

	void SetAutoNextHall(Bool New)
		{
		SYNCDATA(L1_SETAUTONEXTHALL, New, 0);

		NEXTHALL = !!New;
		}

	Bool IsEnterBorders(void) const
		{
		return (BORDERS);
		}

	void SetEnterBorders(Bool New)
		{
		SYNCDATA(L1_SETENTERBORDERS, New, 0);

		BORDERS = !!New;
		}

	Bool IsVerified(void) const
		{
		return (!UNVERIFIED);
		}

	void SetVerified(Bool New)
		{
		SYNCDATA(L1_SETVERIFIED, New, 0);

		UNVERIFIED = !New;
		}

	Bool IsSurnameLocked(void) const
		{
		return (SURNAMLOK);
		}

	void SetSurnameLocked(Bool New)
		{
		SYNCDATA(L1_SETSURNAMELOCKED, New, 0);

		SURNAMLOK = !!New;
		}

	Bool IsDefaultHallLocked(void) const
		{
		return (LOCKHALL);
		}

	void SetDefaultHallLocked(Bool New)
		{
		SYNCDATA(L1_SETDEFAULTHALLLOCKED, New, 0);

		LOCKHALL = !!New;
		}

	Bool IsPsycho(void) const
		{
		return (PSYCHO);
		}

	void SetPsycho(Bool New)
		{
		SYNCDATA(L1_SETPSYCHO, New, 0);

		PSYCHO = !!New;
		}

	Bool IsViewTitleSurname(void) const
		{
		return (DISPLAYTS);
		}

	void SetViewTitleSurname(Bool New)
		{
		SYNCDATA(L1_SETVIEWTITLESURNAME, New, 0);

		DISPLAYTS = !!New;
		}

	Bool IsViewSubjects(void) const
		{
		return (SUBJECTS);
		}

	void SetViewSubjects(Bool New)
		{
		SYNCDATA(L1_SETVIEWSUBJECTS, New, 0);

		SUBJECTS = !!New;
		}

	Bool IsViewSignatures(void) const
		{
		return (SIGNATURES);
		}

	void SetViewSignatures(Bool New)
		{
		SYNCDATA(L1_SETVIEWSIGNATURES, New, 0);

		SIGNATURES = !!New;
		}

	Bool IsOldIBMGraph(void) const
		{
		return (oldIBMGRAPH);
		}

	void SetOldIBMGraph(Bool New)
		{
		SYNCDATA(L1_SETOLDIBMGRAPH, New, 0);

		oldIBMGRAPH = !!New;
		}

	Bool IsOldIBMANSI(void) const
		{
		return (oldIBMANSI);
		}

	void SetOldIBMANSI(Bool New)
		{
		SYNCDATA(L1_SETOLDIBMANSI, New, 0);

		oldIBMANSI = !!New;
		}

	Bool IsOldIBMColor(void) const
		{
		return (oldIBMCOLOR);
		}

	void SetOldIBMColor(Bool New)
		{
		SYNCDATA(L1_SETOLDIBMCOLOR, New, 0);

		oldIBMCOLOR = !!New;
		}

	Bool IsTwirly(void) const
		{
		return (TWIRLY);
		}

	void SetTwirly(Bool New)
		{
		SYNCDATA(L1_SETTWIRLY, New, 0);

		TWIRLY = !!New;
		}

	Bool IsAutoVerbose(void) const
		{
		return (VERBOSE);
		}

	void SetAutoVerbose(Bool New)
		{
		SYNCDATA(L1_SETAUTOVERBOSE, New, 0);

		VERBOSE = !!New;
		}

	Bool IsPauseBetweenMessages(void) const
		{
		return (MSGPAUSE);
		}

	void SetPauseBetweenMessages(Bool New)
		{
		SYNCDATA(L1_SETPAUSEBETWEENMSGS, New, 0);

		MSGPAUSE = !!New;
		}

	Bool IsMinibin(void) const
		{
		return (MINIBIN);
		}

	void SetMinibin(Bool New)
		{
		SYNCDATA(L1_SETMINIBIN, New, 0);

		MINIBIN = !!New;
		}

	Bool IsClearScreenBetweenMessages(void) const
		{
		return (MSGCLS);
		}

	void SetClearScreenBetweenMessages(Bool New)
		{
		SYNCDATA(L1_SETCLEARSCREENMSGS, New, 0);

		MSGCLS = !!New;
		}

	Bool IsViewRoomInfoLines(void) const
		{
		return (ROOMINFO);
		}

	void SetViewRoomInfoLines(Bool New)
		{
		SYNCDATA(L1_SETVIEWROOMINFO, New, 0);

		ROOMINFO = !!New;
		}

	Bool IsViewHallDescription(void) const
		{
		return (HALLTELL);
		}

	void SetViewHallDescription(Bool New)
		{
		SYNCDATA(L1_SETVIEWHALLDESC, New, 0);

		HALLTELL = !!New;
		}

	Bool IsVerboseContinue(void) const
		{
		return (VERBOSECONT);
		}

	void SetVerboseContinue(Bool New)
		{
		SYNCDATA(L1_SETVERBOSECONTINUE, New, 0);

		VERBOSECONT = !!New;
		}

	Bool IsViewCensoredMessages(void) const
		{
		return (VIEWCENSOR);
		}

	void SetViewCensoredMessages(Bool New)
		{
		SYNCDATA(L1_SETVIEWCENSORED, New, 0);

		VIEWCENSOR = !!New;
		}

	Bool IsViewBorders(void) const
		{
		return (SEEBORDERS);
		}

	void SetViewBorders(Bool New)
		{
		SYNCDATA(L1_SETVIEWBORDERS, New, 0);

		SEEBORDERS = !!New;
		}

	Bool IsOut300(void) const
		{
		return (OUT300);
		}

	void SetOut300(Bool New)
		{
		SYNCDATA(L1_SETOUT300, New, 0);

		OUT300 = !!New;
		}

	Bool IsUserSignatureLocked(void) const
		{
		return (LOCKUSIG);
		}

	void SetUserSignatureLocked(Bool New)
		{
		SYNCDATA(L1_SETUSERSIGLOCKED, New, 0);

		LOCKUSIG = !!New;
		}

	Bool IsHideMessageExclusions(void) const
		{
		return (HIDEEXCL);
		}

	void SetHideMessageExclusions(Bool New)
		{
		SYNCDATA(L1_SETHIDEMSGEXCL, New, 0);

		HIDEEXCL = !!New;
		}

	Bool IsDownload(void) const
		{
		return (!NODOWNLOAD);
		}

	void SetDownload(Bool New)
		{
		SYNCDATA(L1_SETDOWNLOAD, New, 0);

		NODOWNLOAD = !New;
		}

	Bool IsUpload(void) const
		{
		return (!NOUPLOAD);
		}

	void SetUpload(Bool New)
		{
		SYNCDATA(L1_SETUPLOAD, New, 0);

		NOUPLOAD = !New;
		}

	Bool IsChat(void) const
		{
		return (!NOCHAT);
		}

	void SetChat(Bool New)
		{
		SYNCDATA(L1_SETCHAT, New, 0);

		NOCHAT = !New;
		}

	Bool IsPrintFile(void) const
		{
		return (PRINTFILE);
		}

	void SetPrintFile(Bool New)
		{
		SYNCDATA(L1_SETPRINTFILE, New, 0);

		PRINTFILE = !!New;
		}

	uint GetSpellCheckMode(void) const
		{
		return (spellCheck);
		}

	void SetSpellCheckMode(uint New)
		{
		SYNCDATA(L1_SETSPELLCHECKMODE, New, 0);

		assert(New < 4);

		spellCheck = New;
		}

	Bool IsMakeRoom(void) const
		{
		return (!NOMAKEROOM);
		}

	void SetMakeRoom(Bool New)
		{
		SYNCDATA(L1_SETMAKEROOM, New, 0);

		NOMAKEROOM = !New;
		}

	Bool IsVerboseLogOut(void) const
		{
		return (VERBOSELO);
		}

	void SetVerboseLogOut(Bool New)
		{
		SYNCDATA(L1_SETVERBOSELOGOUT, New, 0);

		VERBOSELO = !!New;
		}

	Bool IsConfirmSave(void) const
		{
		return (CONFIRMSAVE);
		}

	void SetConfirmSave(Bool New)
		{
		SYNCDATA(L1_SETCONFIRMSAVE, New, 0);

		CONFIRMSAVE = !!New;
		}

	Bool IsConfirmAbort(void) const
		{
		return (!NOCONFABORT);
		}

	void SetConfirmAbort(Bool New)
		{
		SYNCDATA(L1_SETCONFIRMABORT, New, 0);

		NOCONFABORT = !New;
		}

	Bool IsConfirmNoEO(void) const
		{
		return (CONFIRMNOEO);
		}

	void SetConfirmNoEO(Bool New)
		{
		SYNCDATA(L1_SETCONFIRMNOEO, New, 0);

		CONFIRMNOEO = !!New;
		}

	Bool IsUsePersonalHall(void) const
		{
		return (cfg.PersonalHallOK && USEPERSONAL);
		}

	void SetUsePersonalHall(Bool New)
		{
		SYNCDATA(L1_SETUSEPERSONALHALL, New, 0);

		USEPERSONAL = New;
		}

	Bool IsYouAreHere(void) const
		{
		return (YOUAREHERE);
		}

	void SetYouAreHere(Bool New)
		{
		SYNCDATA(L1_SETYOUAREHERE, New, 0);

		YOUAREHERE = !!New;
		}

	Bool IsIBMRoom(void) const
		{
		return (IBMROOM);
		}

	void SetIBMRoom(Bool New)
		{
		SYNCDATA(L1_SETIBMROOM, New, 0);

		IBMROOM = !!New;
		}

	Bool IsWideRoom(void) const
		{
		return (WIDEROOM);
		}

	void SetWideRoom(Bool New)
		{
		SYNCDATA(L1_SETWIDEROOM, New, 0);

		WIDEROOM = !!New;
		}

	Bool IsMusic(void) const
		{
		return (MUSIC);
		}

	void SetMusic(Bool New)
		{
		SYNCDATA(L1_SETMUSIC, New, 0);

		MUSIC = !!New;
		}

	Bool IsCheckApostropheS(void) const
		{
		return (CHECKAPS);
		}

	void SetCheckApostropheS(Bool New)
		{
		SYNCDATA(L1_SETCHECKAPOSTROPHES, New, 0);

		CHECKAPS = !!New;
		}

	Bool IsCheckAllCaps(void) const
		{
		return (CHECKALLCAP);
		}

	void SetCheckAllCaps(Bool New)
		{
		SYNCDATA(L1_SETCHECKALLCAPS, New, 0);

		CHECKALLCAP = !!New;
		}

	Bool IsCheckDigits(void) const
		{
		return (CHECKDIGITS);
		}

	void SetCheckDigits(Bool New)
		{
		SYNCDATA(L1_SETCHECKDIGITS, New, 0);

		CHECKDIGITS = !!New;
		}

	Bool IsExcludeEncryptedMessages(void) const
		{
		return (EXCLUDECRYPT);
		}

	void SetExcludeEncryptedMessages(Bool New)
		{
		SYNCDATA(L1_SETEXCLUDEENCRYPTED, New, 0);

		EXCLUDECRYPT = !!New;
		}

	Bool IsViewCommas(void) const
		{
		return (!HIDECOMMAS);
		}

	void SetViewCommas(Bool New)
		{
		SYNCDATA(L1_SETVIEWCOMMAS, New, 0);

		HIDECOMMAS = !New;
		}

	Bool IsPUnPauses(void) const
		{
		return (PUNPAUSES);
		}

	void SetPUnPauses(Bool New)
		{
		SYNCDATA(L1_SETPUNPAUSES, New, 0);

		PUNPAUSES = !!New;
		}

	Bool IsRoman(void) const
		{
		return (ROMAN);
		}

	void SetRoman(Bool New)
		{
		SYNCDATA(L1_SETROMAN, New, 0);

		ROMAN = !!New;
		}

	Bool IsSuperSysop(void) const
		{
		return (cfg.SuperSysop && SUPERSYSOP);
		}

	void SetSuperSysop(Bool New)
		{
		Bool WasSysop = SUPERSYSOP;

		SYNCDATA(L1_SETSUPERSYSOP, New, 0);
		SUPERSYSOP = !!New;

		// This updates the log table in memory
		if (SUPERSYSOP || WasSysop)
			{
			SetSysop(TRUE);
			}
		}

	Bool IsMainSysop(void) const
		{
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(Name, cfg.sysop);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool IsBunny(void) const
		{
		return (!NOBUNNY);
		}

	void SetBunny(Bool New)
		{
		SYNCDATA(L1_SETBUNNY, New, 0);

		NOBUNNY = !New;
		}

	Bool ShowSELogOnOff(void) const
		{
		return (!HSE_LOGONOFF);
		}

	void SetSELogOnOff(Bool New)
		{
		SYNCDATA(L1_SSE_LOGONOFF, New, 0);

		HSE_LOGONOFF = !New;
		}

	Bool ShowSENewMessage(void) const
		{
		return (!HSE_NEWMSG);
		}

	void SetSENewMessage(Bool New)
		{
		SYNCDATA(L1_SSE_NEWMSG, New, 0);

		HSE_NEWMSG = !New;
		}

	Bool ShowSEExclusiveMessage(void) const
		{
		return (!HSE_EXCLMSG);
		}

	void SetSEExclusiveMessage(Bool New)
		{
		SYNCDATA(L1_SSE_EXCLMSG, New, 0);

		HSE_EXCLMSG = !New;
		}

	Bool ShowSEChatAll(void) const
		{
		return (!HSE_CHATALL);
		}

	void SetSEChatAll(Bool New)
		{
		SYNCDATA(L1_SSE_CHATALL, New, 0);

		HSE_CHATALL = !New;
		}

	Bool ShowSEChatRoom(void) const
		{
		return (!HSE_CHATROOM);
		}

	void SetSEChatRoom(Bool New)
		{
		SYNCDATA(L1_SSE_CHATROOM, New, 0);

		HSE_CHATROOM = !New;
		}

	Bool ShowSEChatGroup(void) const
		{
		return (!HSE_CHATGRP);
		}

	void SetSEChatGroup(Bool New)
		{
		SYNCDATA(L1_SSE_CHATGROUP, New, 0);

		HSE_CHATGRP = !New;
		}

	Bool ShowSEChatUser(void) const
		{
		return (!HSE_CHATUSER);
		}

	void SetSEChatUser(Bool New)
		{
		SYNCDATA(L1_SSE_CHATUSER, New, 0);

		HSE_CHATUSER = !New;
		}

	Bool ShowSERoomInOut(void) const
		{
		return (SSE_RMINOUT);
		}

	void SetSERoomInOut(Bool New)
		{
		SYNCDATA(L1_SSE_RMINOUT, New, 0);

		SSE_RMINOUT = !!New;
		}

	Bool IsBanned(void) const
		{
		return (BANNED);
		}

	void SetBanned(Bool New)
		{
		SYNCDATA(L1_SETBANNED, New, 0);

		BANNED = !!New;
		}

	Bool IsKillOwn(void) const
		{
		return (!NOKILLOWN);
		}

	void SetKillOwn(Bool New)
		{
		SYNCDATA(L1_SETKILLOWN, New, 0);

		NOKILLOWN = !New;
		}

	Bool IsSeeOwnChats(void) const
		{
		return (SEEOWNCHATS);
		}

	void SetSeeOwnChats(Bool New)
		{
		SYNCDATA(L1_SETSEEOWNCHATS, New, 0);

		SEEOWNCHATS = !!New;
		}

    Bool IsHearLaughter(void) const
		{
        return (HEARLAUGHTER);
		}

    void SetHearLaughter(Bool New)
		{
        SYNCDATA(L1_SETHEARLAUGHTER, New, 0);

        HEARLAUGHTER = !!New;
		}


	Bool IsErasePrompt(void) const
		{
		return (ERASEPROMPT);
		}

	void SetErasePrompt(Bool New)
		{
		SYNCDATA(L1_SETERASEPROMPT, New, 0);

		ERASEPROMPT = !!New;
		}

	int GetAutoIdleSeconds(void) const
		{
		return (AutoIdleSeconds);
		}

	void SetAutoIdleSeconds(int New)
		{
		SYNCDATA(L1_SETAUTOIDLESECONDS, New, 0);

		AutoIdleSeconds = New;
		}

	Bool IsSameName(const char *TestName) const
		{
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(Name, TestName);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}
	};

class LogEntry1 : public LogStarter, public LogEntry1Data
	{
public:
	void Clear(void)
		{
		LogFlags::Clear();
		LogEntry1Data::Clear();
		}

	LogEntry1(void)
		{
		Clear();
		ResetFileName();
		}

	LogEntry1& operator =(const LogEntry1 &Original);
	LogEntry1& operator =(const LogEntry1Data &Original);

	Bool Load(l_index Index);
	Bool Save(l_index Index);

	void ResetFileName(void)
		{
		SetFileName(logDat);
		}
	};
