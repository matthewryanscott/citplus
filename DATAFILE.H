// the structure of our data files

/* -------------------------------------------------------------------- */
/*	The next structure contains a user's flags. This is used in both    */
/*	the full structure of a user and in the index.						*/
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/*	This is most of the information we know about each user. This is	*/
/*	stored in LOG.DAT. Following this, there are structures used for	*/
/*	LOG2.DAT, LOG3.DAT, LOG4.DAT, LOG5.DAT, and LOG6.DAT. Citadel also	*/
/*	keeps an index to LOG.DAT, as it does to ROOM.DAT, for the quick	*/
/*	of data it needs often about users. The structure of this index 	*/
/*	follows.															*/
/* -------------------------------------------------------------------- */
struct LogDatStructure
	{
	label	lbname; 			// caller's name
	label	lbin;				// caller's initials
	label	lbpw;				// caller's password
	label	forward;			// forwarding address (user)
	label	forward_node;		// forwarding address (node) or xxx.xxxx
	label	surname;			// users surname
	label	title;				// users title
	char	PAD1;
	short	UNUSED1;			// (RETHINK) default hall hash
	uchar	lbnulls;			// #Nulls, lCase, lFeeds
	uchar	lbwidth;			// terminal width
	time_t	calltime;			// Time/date stamp of last call
	ulong	callno; 			// Which caller # they were
	long	credits;			// Credits for accounting
	ulong	lastpointer;		// newest message on system last call
	uchar	UNUSED2;			// (RETHINK) hall they were in last call
	char	PAD2;
	short	UNUSED3;			// (RETHINK) room they were in last call
	uchar	linesScreen;		// the number of lines per screen
	uchar	attr[5];			// bold, inverse, blink, underline
	char	protocol;			// Default protocol
	char	prompt[64]; 		// prompt format string
	char	dstamp[64]; 		// time/date format string
	char	vdstamp[64];		// time/date format string
	char	signature[91];		// user's signature line
	label	netPrefix;			// Prefix for networked rooms
	label	addr1;				// Mailing address...
	label	addr2;				// Mailing address...
	label	addr3;				// Mailing address...
	char	alias[4];			// ALIAS.LOCID.PLANET
	char	locID[5];
	char	PAD3;
	ulong	dl_bytes;
	ulong	ul_bytes;
	ushort	dl_num;
	ushort	ul_num;
	label	forward_region; 	// forwarding address (region)
	char	moreprompt[80]; 	// User configurable more prompt

	label	occupation; 		// What you do.

	short	numUserShow;		// # users to show on login

	long	birthdate;			// midnight of birthdate
	long	firston;			// first time on the system
	SexE	Sex;				// Oh so very sexy.
	long	totaltime;			// Total seconds on the system.
	long	logins; 			// How many times.
	long	posted; 			// Number of messages posted.
	long	read;				// Number of messages read.
	long	pwChange;			// The last time the PW was changed.
	short	callsToday; 		// Number of calls today (mid-mid).
	short	callLimit;			// Number of calls allowed per day
	char	wherehear[80];		// Where did you hear about us?
    long    AutoIdleSeconds;    // # seconds before entering idle mode (shoul dhave been short. oh well.)
	Bool    SEEOWNCHATS : 1;    // Can you see your own chats?
	Bool    ERASEPROMPT : 1;    // Erase room prompt before event display?
    Bool    HEARLAUGHTER : 1;
    Bool    UNUSED6      : 13;
	char	UNUSED4[10];		// some blank space

	label	lastRoom;			// room they were in last call
	label	lastHall;			// hall they were in last call
	label	defaultRoom;		// default room
	label	defaultHall;		// default hall

	long	poopcount;			// How many times have they pooped?

	struct
		{
		Bool L_INUSE	: 1;	// Is this slot in use?
		Bool UCMASK 	: 1;	// Uppercase?
		Bool LFMASK 	: 1;	// Linefeeds?
		Bool EXPERT 	: 1;	// Expert?
		Bool AIDE		: 1;	// Aide?
		Bool TABS		: 1;	// Tabs?
		Bool OLDTOO 	: 1;	// Print out last oldmessage on N>ew?
		Bool PROBLEM	: 1;	// Twit bit
		Bool UNLISTED	: 1;	// List in Userlog?
		Bool PERMANENT	: 1;	// Permanent log entry?
		Bool SYSOP		: 1;	// Sysop special access.
		Bool NODE		: 1;	// Are they a node?
		Bool NETUSER	: 1;	// Can they send network E-mail
		Bool NOACCOUNT	: 1;	// Accounting disabled for this person?
		Bool NOMAIL 	: 1;	// Can user send exclusive mail?
		Bool ROOMTELL	: 1;	// Room-descriptions?
		} lbflags;

	Bool	DUNGEONED	: 1;	// dungeoned user?
	Bool	MSGAIDE 	: 1;	// message only aide?
	Bool	FORtOnODE	: 1;	// forward to node?
	Bool	NEXTHALL	: 1;	// Auto Next Hall?
	Bool	FILEAIDE	: 1;	// Aide responsible for file sections
	Bool	BORDERS 	: 1;	// Can they enter borders
	Bool	VERIFIED	: 1;	// has the user been verified?
	Bool	SURNAMLOK	: 1;	// surname locked?
	Bool	LOCKHALL	: 1;	// lock default hall
	Bool	PSYCHO		: 1;	// make life a pain in the ass for him
	Bool	DISPLAYTS	: 1;	// display titles/surnames?
	Bool	SUBJECTS	: 1;	// display subjects?
	Bool	SIGNATURES	: 1;	// display signatures?
	Bool	oldIBMGRAPH	: 1;	// Terminal : IBM graphics characters?
	Bool	oldIBMANSI 	: 1;	// Terminal : IBM ansi enabled?
	Bool	oldIBMCOLOR	: 1;	// Terminal : ISO-Color?
	Bool	TWIRLY		: 1;	// twirling cursor at prompts?
	Bool	VERBOSE 	: 1;	// Auto-verbose flag?
	Bool	MSGPAUSE	: 1;	// Pause after each message.
	Bool	MINIBIN 	: 1;	// MiniBin login stats?
	Bool	MSGCLS		: 1;	// Clear screen after message?
	Bool	ROOMINFO	: 1;	// Display room info lines?
	Bool	HALLTELL	: 1;	// Display hall descriptions?
	Bool	VERBOSECONT : 1;	// continue in the icky verbose way?
	Bool	VIEWCENSOR	: 1;	// View censored messages?
	Bool	SEEBORDERS	: 1;	// Can they see borders?
	Bool	OUT300		: 1;	// simulate 300 baud output
	Bool	LOCKUSIG	: 1;	// user signature locked?
	Bool	HIDEEXCL	: 1;	// hide msg exclusion?
	Bool	NODOWNLOAD	: 1;	// user cannot download
	Bool	NOUPLOAD	: 1;	// user cannot upload
	Bool	NOCHAT		: 1;	// user cannot chat?
	Bool	PRINTFILE	: 1;	// turn on printfile when user logs in?
	uint	spellCheck	: 2;	// how the user likes to spell check
	Bool	NOMAKEROOM	: 1;	// if user cannot .ER
	Bool	VERBOSELO	: 1;	// do it verbosely...
	Bool	CONFIRMSAVE : 1;	// confirm save from msg editor?
	Bool	NOCONFABORT : 1;	// don't confirm abort from msg editor
	Bool	CONFIRMNOEO : 1;	// confirm not .EO after abort
	Bool	USEPERSONAL : 1;	// Use personal hall?
	Bool	YOUAREHERE	: 1;	// Dargos is such a God.
	Bool	IBMROOM 	: 1;	// Fred is a genius. Centauri, too.
	Bool	WIDEROOM	: 1;	// ...
	Bool	MUSIC		: 1;	// user can hear it?
	Bool	CHECKAPS	: 1;	// check 'S in spell checker
	Bool	CHECKALLCAP : 1;	// check all caps in spell checker
	Bool	CHECKDIGITS : 1;	// check digits in spell checker

	char	UNUSED5;
	label	realname;
	label	phonenum;
	char	PAD4;

	Bool	EXCLUDECRYPT: 1;	// Exclude encrypted messages.
	Bool	HIDECOMMAS	: 1;	// how's this for user-configurability?
	Bool	PUNPAUSES	: 1;	// Some people acutally want it to...
	Bool	ROMAN		: 1;	// Ho ho ho
	Bool	SUPERSYSOP	: 1;	// Super Sysop to the rescue!
	Bool	NOBUNNY		: 1;	// No bunny filter for me, mom!
	Bool	HSE_LOGONOFF: 1;	// Hide SE_LOGONOFF
	Bool	HSE_NEWMSG	: 1;	// Hide SE_NEWMESSAGE
	Bool	HSE_EXCLMSG	: 1;	// Hide SE_EXCLUSIVEMESSAGE
	Bool	HSE_CHATALL	: 1;	// Hide SE_CHATALL
	Bool	HSE_CHATROOM: 1;	// Hide SE_CHATROOM
	Bool	HSE_CHATGRP	: 1;	// Hide SE_CHATGROUP
	Bool	HSE_CHATUSER: 1;	// Hide SE_CHATUSER
	Bool	SSE_RMINOUT	: 1;	// Show SE_ROOMINOUT
	Bool	BANNED		: 1;	// BANNED!!!!!!
	Bool	NOKILLOWN	: 1;	// Cannot kill own messages

	long	LastCalledVersion;	// Version of Citadel running last call

	label	TermType;			// Terminal type, of all things!
	char	PAD5;
	};

// When taken as a whole, the Windows version comes up with a different size
// because it is aligned on 4-byte boundaries, not 2-byte.

#define LogDatRecordSize(LD) ((long) (	\
	sizeof((LD).lbname)					+\
	sizeof((LD).lbin)					+\
	sizeof((LD).lbpw)					+\
	sizeof((LD).forward)				+\
	sizeof((LD).forward_node)			+\
	sizeof((LD).surname)				+\
	sizeof((LD).title)					+\
	sizeof((LD).PAD1)					+\
	sizeof((LD).UNUSED1)				+\
	sizeof((LD).lbnulls)				+\
	sizeof((LD).lbwidth)				+\
	sizeof((LD).calltime)				+\
	sizeof((LD).callno)					+\
	sizeof((LD).credits)				+\
	sizeof((LD).lastpointer)			+\
	sizeof((LD).UNUSED2)				+\
	sizeof((LD).PAD2)					+\
	sizeof((LD).UNUSED3)				+\
	sizeof((LD).linesScreen)			+\
	sizeof((LD).attr)					+\
	sizeof((LD).protocol)				+\
	sizeof((LD).prompt)					+\
	sizeof((LD).dstamp)					+\
	sizeof((LD).vdstamp)				+\
	sizeof((LD).signature)				+\
	sizeof((LD).netPrefix)				+\
	sizeof((LD).addr1)					+\
	sizeof((LD).addr2)					+\
	sizeof((LD).addr3)					+\
	sizeof((LD).alias)					+\
	sizeof((LD).locID)					+\
	sizeof((LD).PAD3)					+\
	sizeof((LD).dl_bytes)				+\
	sizeof((LD).ul_bytes)				+\
	sizeof((LD).dl_num)					+\
	sizeof((LD).ul_num)					+\
	sizeof((LD).forward_region)			+\
	sizeof((LD).moreprompt)				+\
	sizeof((LD).occupation)				+\
	sizeof((LD).numUserShow)			+\
	sizeof((LD).birthdate)				+\
	sizeof((LD).firston)				+\
	2 /* SexE */						+\
	sizeof((LD).totaltime)				+\
	sizeof((LD).logins)					+\
	sizeof((LD).posted)					+\
	sizeof((LD).read)					+\
	sizeof((LD).pwChange)				+\
	sizeof((LD).callsToday)				+\
	sizeof((LD).callLimit)				+\
	sizeof((LD).wherehear)				+\
    sizeof((LD).AutoIdleSeconds)        +\
	2									+\
    sizeof((LD).UNUSED4)				+\
	sizeof((LD).lastRoom)				+\
	sizeof((LD).lastHall)				+\
	sizeof((LD).defaultRoom)			+\
	sizeof((LD).defaultHall)			+\
	sizeof((LD).poopcount)				+\
	2									+\
	2									+\
	2									+\
	2									+\
	sizeof((LD).UNUSED5)				+\
	sizeof((LD).realname)				+\
	sizeof((LD).phonenum)				+\
	sizeof((LD).PAD4)					+\
	2									+\
	sizeof((LD).LastCalledVersion)		+\
	sizeof((LD).TermType)				+\
	sizeof((LD).PAD5)					\
	))
