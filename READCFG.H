// --------------------------------------------------------------------------
// Citadel: ReadCfg.H
//
// Keywords and data for reading CONFIG.CIT.

#ifdef READCFG_CPP

#if defined (__BORLANDC__)
	// Maximum array size. I know this is correct for the 16-bit compiler.
	// It might as well be for the 32-bit one, as well.
	#define MAS (UINT_MAX - 3)
#endif

#ifdef VISUALC
	#define MAS (UINT_MAX - 3)
#endif

// If any of these three enums or the struct changes, you must also make
// changes to CTDLDATA.DEF(0:7)

enum ConfigKeywordDataTypeE
	{
	CK_UCHAR,	CK_UCHARHEX,	CK_LONG,	CK_INT,
	CK_UINT,	CK_SHORT,		CK_STR, 	CK_BOOLEAN,
	CK_SPECIAL,	CK_PATH,
	};

enum ConfigKeywordIgnoreE
	{
	CK_ALWAYSREAD,	CK_DASHEORC,	CK_DASHC,
	};

enum ConfigKeywordCategoryE
	{
	CK_REQUIRED,	CK_MODEMSETTINGS,	CK_PATHANDFILENAME,
	CK_SYSTEMSIZE,	CK_CONSOLEOPTIONS,	CK_SECURITY,
	CK_SYSSETTINGS, CK_USERCONFIG,		CK_SYSPERSONALITY,
	CK_ACCOUNTING,

	CK_NONE
	};

struct config_keywords
	{
	long dfault;			// the default (if string, index in cfg.msg)
	long min;				// the minimum if numeric, ignored if string
	long max;				// the maximum if numeric, max len if string
	BoolS optional;			// if it is optional or not
	BoolS allowmany; 		// if multiple definiations are allowed or not
	ushort type;			// ConfigKeywordDataTypeE from above
	ushort ignore;			// ConfigKeywordIgnoreE from above
	ushort category;		// ConfigKeywordCategoryE from above
	};

// ConfigCitVariableLocations
static void *CCVL[] =
	{
	cfg.nodeTitle,			// NODENAME
	cfg.nodeRegion,			// NODEREGION
	cfg.nodeCountry,		// NODECOUNTRY
	cfg.nodephone,			// NODEPHONE
	cfg.homepath,			// HOMEPATH
#ifdef WINCIT
	NULL,					// MDATA
#else
	&cfg.mdata,				// MDATA
#endif
	cfg.sysop,				// SYSOP
	NULL,					// ADDRESS
	cfg.modsetup,			// MODSETUP
	cfg.modunsetup,			// MODUNSETUP
	&cfg.initbaud,			// INIT_BAUD
	&cfg.dumbmodem,			// DUMB_MODEM
	cfg.downshift,			// DOWNSHIFT
	cfg.hangup,				// HANGUP
	&cfg.minbaud,			// MIN_BAUD
	cfg.dialsetup,			// DIAL_INIT
	cfg.dialpref,			// DIAL_PREF
	NULL,					// DIAL_RING
	cfg.offhookstr,			// OFFHOOKSTR
	&cfg.hangupdelay,		// HANGUPDELAY
	&cfg.connectwait,		// CONNECTWAIT
	&cfg.baudPause,			// BAUDPAUSE
	&cfg.offhook,			// OFFHOOK
	&cfg.checkCTS,			// CHECKCTS
#ifdef WINCIT
	NULL,					// TELNET
	&cfg.Net6969Port,		// NET6969PORT
#endif
	cfg.aplpath,			// APPLICPATH
	cfg.helppath,			// HELPPATH
	cfg.helppath2,			// HELPPATH2
	cfg.roompath,			// ROOMPATH
	cfg.ScriptPath,			// SCRIPTPATH
	cfg.temppath,			// TEMPPATH
	cfg.transpath,			// TRANSPATH
	cfg.dirpath,			// DIRPATH
	cfg.dlpath,				// DLPATH
	cfg.msgpath,			// MSGPATH
	cfg.logextdir,			// LOGEXTDIR
	cfg.lexpath,			// LEXPATH
	cfg.rlmpath,			// RLMPATH
	NULL,					// PRINTER
	cfg.trapfile,			// TRAP_FILE
	cfg.dictionary,			// DICTIONARY
#ifdef AUXMEM
	cfg.vmemfile,			// VIRTMEM
#else
	NULL,					// VIRTMEM
#endif
	&cfg.MsgDatSizeInK,		// MESSAGEK
	&cfg.nmessages,			// NMESSAGES
	&cfg.MAXLOGTAB,			// MAXLOGTAB
	&cfg.maxrooms,			// MAXROOMS
	&cfg.maxhalls,			// MAXHALLS
	&cfg.maxgroups,			// MAXGROUPS
	&cfg.maxfiles,			// MAXFILES
	&cfg.maxborders,		// MAXBORDERS
	&cfg.maxjumpback,		// MAXJUMPBACK
	&cfg.statnum,			// MAXSTAT
	&cfg.attr,				// ATTR
	&cfg.battr,				// BATTR
	&cfg.cattr,				// CATTR
	&cfg.uttr,				// UTTR
	&cfg.wattr,				// WATTR
	&cfg.bios,				// BIOS
	NULL,					// WYSIWYG
#ifdef WINCIT
	&cfg.BlinkPause,		// BLINKPAUSE
	&cfg.ScreenUpdatePause,	// SCREENUPDATEPAUSE
	NULL,					// CONSOLESIZE
//#else
//    NULL,
//    NULL,
#endif
	NULL,					// LOGIN
	cfg.newuserapp,			// NEWUSERAPP
	NULL,					// NEWUSERQUESTIONS
	cfg.f6pass,				// F6PASSWORD
	&cfg.FullConLock,		// FULLCONLOCK
	&cfg.forcelogin,		// FORCELOGIN
	&cfg.unlogEnterOk,		// ENTEROK
	&cfg.unlogReadOk,		// READOK
	&cfg.MessageRoom,		// MESSAGE_ROOM
	&cfg.readluser,			// READLLOG
	&cfg.nonAideRoomOk,		// ROOMOK
	&cfg.newuserLogFileName,// NEWUSERLOGFILENAME
	&cfg.aidehall,			// AIDEHALL
	&cfg.moderate,			// MODERATE
	&cfg.nopwecho,			// NOPWECHO
	NULL,					// TRAP
	&cfg.sysopOk,			// SYSOPOK
	&cfg.SuperSysop,		// SUPERSYSOP
	&cfg.showSysop,			// SHOWSYSOP
	&cfg.numRooms,			// NUMROOMS
	&cfg.readLog,			// READLOG
	&cfg.allowCrypt,		// ALLOWCRYPT
	&cfg.badPwPause,		// BADPWPAUSE
	&cfg.pwDays,			// PWDAYS
	&cfg.callLimit,			// CALLLIMIT
	&cfg.NoConsoleTrap,		// NOCONSOLETRAP
	&cfg.maxerror,			// MAXERROR
	&cfg.ReadStatusSecurity,// READSTATUSSECURITY
	&cfg.idle,				// IDLE_WAIT
	&cfg.oldcount,			// OLDCOUNT
	NULL,					// UP_DAYS
	NULL,					// UP_HOURS
	&cfg.netmail,			// NETMAIL
	&cfg.subhubs,			// SUBHUBS
#ifndef WINCIT
	NULL,					// SCREENSAVE
#endif
	cfg.SaverMsg,			// SAVERMSG
#ifndef WINCIT
	NULL,					// SCROLL_BACK
#endif
	&cfg.msgCompress,		// MSGCOMPRESS
	&cfg.forward,			// FORWARD
	&cfg.timeout,			// TIMEOUT
	&cfg.consoletimeout,	// CONSOLETIMEOUT
	&cfg.unlogtimeout,		// UNLOGTIMEOUT
	&cfg.OutputTimeout,		// OUTPUTTIMEOUT
	&cfg.sleepcount,		// SLEEPCOUNT
	&cfg.countbeep,			// COUNTBEEP
	&cfg.restore_mode,		// RESTORE_MODE
	NULL,					// DIALMACRO
	&cfg.expire,			// EXPIRE
	cfg.Cit86Country,		// CIT86COUNTRY
	cfg.Cit86Domain,		// CIT86DOMAIN
	&cfg.diskfree,			// DISKFREE
	&cfg.noBells,			// NOBELLS
	&cfg.noChat,			// NOCHAT
	NULL,					// AIDECHATHRS
	&cfg.chatmail,			// CHATMAIL
	&cfg.chatwhy,			// CHATWHY
	&cfg.chatflash,			// CHATFLASH
    &cfg.ringlimit,         // CHATLOOP
	&cfg.speechOn,			// SPEECHON
	&cfg.ovrEms,			// OVREMS
	&cfg.ovrExt,			// OVREXT
	&cfg.checkSysMail,		// CHECKSYSMAIL
	&cfg.memfree,			// MEMFREE
	&cfg.showmoved,			// SHOWMOVED
	&cfg.altF3Time,			// ALTF3TIME
	cfg.AltF3Msg,			// ALTF3MSG
	cfg.AltF3Timeout,		// ALTF3TIMEOUT
	&cfg.SaveJB,			// SAVEJB
	&cfg.SwapNote,			// SWAPNOTE
	&cfg.FastScripts,		// FASTSCRIPTS
	&cfg.NewNodeAlert,		// NEWNODEALERT
	&cfg.VerboseConsole,	// VERBOSECONSOLE
	&cfg.AltXPrompt,		// ALTXPROMPT
    &cfg.laughter,          // NEWMSGTEXT
	cfg.netPrefix,			// NET_PREFIX
	cfg.datestamp,			// DATESTAMP
	cfg.vdatestamp,			// VDATESTAMP
	cfg.prompt,				// PROMPT
	cfg.moreprompt,			// MOREPROMPT
	&cfg.autoansi,			// AUTOANSI
	cfg.enter_name,			// ENTER_NAME
	&cfg.censor,			// CENSOR
	NULL,					// TWIT_FEATURES
	NULL,					// SIGNATURE
	cfg.anonauthor,			// ANONAUTHOR
	cfg.twitRegion,			// TWITREGION
	cfg.twitCountry,		// TWITCOUNTRY
	NULL,					// MESSAGE_NYM
	NULL,					// CREDIT_NYM
	NULL,					// BORDER
	NULL,					// POOP!
	cfg.mci_firstname,		// MCI_FIRSTNAME
	cfg.mci_name,			// MCI_NAME
	cfg.mci_time,			// MCI_TIME
	cfg.mci_date,			// MCI_DATE
	cfg.mci_poop,			// MCI_POOP
	cfg.sleepprompt,		// SLEEPPROMPT
	NULL,					// TWIRLY
	NULL,					// FUELBAR
	cfg.softverb,			// SOFTVERB
	&cfg.ecTwirly,			// ECTWIRLY
	&cfg.ecUserlog,			// ECUSERLOG
	&cfg.ecSignature,		// ECSIGNATURE
	&cfg.borderfreq,		// BORDERFREQ
	&cfg.music,				// MUSIC
	&cfg.twitrev,			// TWITREV
	&cfg.PersonalHallOK,	// PERSONALHALLOK
	NULL,					// CHATBELL
	NULL,					// ROOM_NYM
	NULL,					// HALL_NYM
	NULL,					// USER_NYM
	NULL,					// GROUP_NYM
	NULL,					// FILTER
	&cfg.ad_chance,			// ADCHANCE
	&cfg.ad_time,			// ADTIME
#ifdef WINCIT
	&cfg.chattype,          // CHATTYPE
#endif
	&cfg.accounting,		// ACCOUNTING
	&cfg.unlogbal,			// UNLOGGEDBALANCE
	NULL,					// end...
	};
#endif

#ifdef WINCIT
enum ConfigCitKeywordsE
	{
// CK_REQUIRED
	K_NODENAME, 			K_NODEREGION,			K_NODECOUNTRY,
	K_NODEPHONE,			K_HOMEPATH,				K_MDATA,
	K_SYSOP,				K_ADDRESS,

// CK_MODEMSETTINGS (and other communication-related settings...)
	K_MODSETUP, 			K_MODUNSETUP,			K_INIT_BAUD,
	K_DUMB_MODEM,			K_DOWNSHIFT,			K_HANGUP,
	K_MIN_BAUD, 			K_DIAL_INIT,			K_DIAL_PREF,
	K_DIAL_RING,			K_OFFHOOKSTR,			K_HANGUPDELAY,
	K_CONNECTWAIT,			K_BAUDPAUSE,			K_OFFHOOK,
	K_CHECKCTS,				K_TELNET,				K_NET6969PORT,

// CK_PATHANDFILENAME
	K_APPLICPATH,			K_HELPPATH, 			K_HELPPATH2,
	K_ROOMPATH, 			K_SCRIPTPATH,			K_TEMPPATH,
	K_TRANSPATH,			K_DIRPATH,				K_DLPATH,
	K_MSGPATH,				K_LOGEXTDIR,			K_LEXPATH,
	K_RLMPATH,				K_PRINTER,				K_TRAP_FILE,
	K_DICTIONARY,			K_VIRTMEM,

// CK_SYSTEMSIZE
	K_MESSAGEK, 			K_NMESSAGES,			K_MAXLOGTAB,
	K_MAXROOMS, 			K_MAXHALLS, 			K_MAXGROUPS,
	K_MAXFILES, 			K_MAXBORDERS,			K_MAXJUMPBACK,
	K_MAXSTAT,

// CK_CONSOLEOPTIONS
	K_ATTR, 				K_BATTR,				K_CATTR,
	K_UTTR, 				K_WATTR,				K_BIOS,
	K_WYSIWYG,				K_BLINKPAUSE,			K_SCREENUPDATEPAUSE,
    K_CONSOLESIZE,

// CK_SECURITY
	K_LOGIN,				K_NEWUSERAPP,			K_NEWUSERQUESTIONS,
	K_F6PASSWORD,			K_FULLCONLOCK,			K_FORCELOGIN,
	K_ENTEROK,				K_READOK,				K_MESSAGE_ROOM,
	K_READLLOG, 			K_ROOMOK,				K_NEWUSERLOGFILENAME,
	K_AIDEHALL, 			K_MODERATE, 			K_NOPWECHO,
	K_TRAP, 				K_SYSOPOK,				K_SUPERSYSOP,
	K_SHOWSYSOP,			K_NUMROOMS, 			K_READLOG,
	K_ALLOWCRYPT,			K_BADPWPAUSE,			K_PWDAYS,
	K_CALLLIMIT,			K_NOCONSOLETRAP,		K_MAXERROR,
	K_READSTATUSSECURITY,

// CK_SYSSETTINGS
	K_IDLE_WAIT,			K_OLDCOUNT, 			K_UP_DAYS,
	K_UP_HOURS, 			K_NETMAIL,				K_SUBHUBS,
    K_SAVERMSG,     
	K_MSGCOMPRESS,			K_FORWARD,				K_TIMEOUT,
	K_CONSOLETIMEOUT,		K_UNLOGTIMEOUT, 		K_OUTPUTTIMEOUT,
	K_SLEEPCOUNT,			K_COUNTBEEP,			K_RESTORE_MODE,
	K_DIALMACRO,			K_EXPIRE,				K_CIT86COUNTRY,
	K_CIT86DOMAIN,			K_DISKFREE, 			K_NOBELLS,
	K_NOCHAT,				K_AIDECHATHRS,			K_CHATMAIL,
    K_CHATWHY,              K_CHATFLASH,            K_CHATLOOP, K_SPEECHON,
	K_OVREMS,				K_OVREXT,				K_CHECKSYSMAIL,
	K_MEMFREE,				K_SHOWMOVED,			K_ALTF3TIME,
	K_ALTF3MSG, 			K_ALTF3TIMEOUT,
	K_SAVEJB,				K_SWAPNOTE,				K_FASTSCRIPTS,
	K_NEWNODEALERT, 		K_VERBOSECONSOLE,		K_ALTXPROMPT,
    K_NEWMSGTEXT,

// CK_USERCONFIG
	K_NET_PREFIX,			K_DATESTAMP,			K_VDATESTAMP,
	K_PROMPT,				K_MOREPROMPT,

// CK_SYSPERSONALITY
	K_AUTOANSI, 			K_ENTER_NAME,			K_CENSOR,
	K_TWIT_FEATURES,		K_SIGNATURE,			K_ANONAUTHOR,
	K_TWITREGION,			K_TWITCOUNTRY,			K_MESSAGE_NYM,
	K_CREDIT_NYM,			K_BORDER,				K_POOP,
	K_MCI_FIRSTNAME,		K_MCI_NAME, 			K_MCI_TIME,
	K_MCI_DATE, 			K_MCI_POOP, 			K_SLEEPPROMPT,
	K_TWIRLY,				K_FUELBAR,				K_SOFTVERB,
	K_ECTWIRLY, 			K_ECUSERLOG,			K_ECSIGNATURE,
	K_BORDERFREQ,			K_MUSIC,
	K_TWITREV,				K_PERSONALHALLOK,		K_CHATBELL,
	K_ROOM_NYM, 			K_HALL_NYM, 			K_USER_NYM,
	K_GROUP_NYM,			K_FILTER,				K_ADCHANCE,
	K_ADTIME,				K_CHATTYPE,

// CK_ACCOUNTING
	K_ACCOUNTING,			K_UNLOGGEDBALANCE,

	K_NWORDS,
	};
#else
enum ConfigCitKeywordsE
	{
// CK_REQUIRED
	K_NODENAME, 			K_NODEREGION,			K_NODECOUNTRY,
	K_NODEPHONE,			K_HOMEPATH,				K_MDATA,
	K_SYSOP,				K_ADDRESS,

// CK_MODEMSETTINGS (and other communication-related settings...)
	K_MODSETUP, 			K_MODUNSETUP,			K_INIT_BAUD,
	K_DUMB_MODEM,			K_DOWNSHIFT,			K_HANGUP,
	K_MIN_BAUD, 			K_DIAL_INIT,			K_DIAL_PREF,
	K_DIAL_RING,			K_OFFHOOKSTR,			K_HANGUPDELAY,
	K_CONNECTWAIT,			K_BAUDPAUSE,			K_OFFHOOK,
    K_CHECKCTS,          

// CK_PATHANDFILENAME
	K_APPLICPATH,			K_HELPPATH, 			K_HELPPATH2,
	K_ROOMPATH, 			K_SCRIPTPATH,			K_TEMPPATH,
	K_TRANSPATH,			K_DIRPATH,				K_DLPATH,
	K_MSGPATH,				K_LOGEXTDIR,			K_LEXPATH,
	K_RLMPATH,				K_PRINTER,				K_TRAP_FILE,
	K_DICTIONARY,			K_VIRTMEM,

// CK_SYSTEMSIZE
	K_MESSAGEK, 			K_NMESSAGES,			K_MAXLOGTAB,
	K_MAXROOMS, 			K_MAXHALLS, 			K_MAXGROUPS,
	K_MAXFILES, 			K_MAXBORDERS,			K_MAXJUMPBACK,
	K_MAXSTAT,

// CK_CONSOLEOPTIONS
	K_ATTR, 				K_BATTR,				K_CATTR,
	K_UTTR, 				K_WATTR,				K_BIOS,
    K_WYSIWYG,          

// CK_SECURITY
	K_LOGIN,				K_NEWUSERAPP,			K_NEWUSERQUESTIONS,
	K_F6PASSWORD,			K_FULLCONLOCK,			K_FORCELOGIN,
	K_ENTEROK,				K_READOK,				K_MESSAGE_ROOM,
	K_READLLOG, 			K_ROOMOK,				K_NEWUSERLOGFILENAME,
	K_AIDEHALL, 			K_MODERATE, 			K_NOPWECHO,
	K_TRAP, 				K_SYSOPOK,				K_SUPERSYSOP,
	K_SHOWSYSOP,			K_NUMROOMS, 			K_READLOG,
	K_ALLOWCRYPT,			K_BADPWPAUSE,			K_PWDAYS,
	K_CALLLIMIT,			K_NOCONSOLETRAP,		K_MAXERROR,
	K_READSTATUSSECURITY,

// CK_SYSSETTINGS
	K_IDLE_WAIT,			K_OLDCOUNT, 			K_UP_DAYS,
	K_UP_HOURS, 			K_NETMAIL,				K_SUBHUBS,
	K_SCREENSAVE,			K_SAVERMSG,				K_SCROLL_BACK,
	K_MSGCOMPRESS,			K_FORWARD,				K_TIMEOUT,
	K_CONSOLETIMEOUT,		K_UNLOGTIMEOUT, 		K_OUTPUTTIMEOUT,
	K_SLEEPCOUNT,			K_COUNTBEEP,			K_RESTORE_MODE,
	K_DIALMACRO,			K_EXPIRE,				K_CIT86COUNTRY,
	K_CIT86DOMAIN,			K_DISKFREE, 			K_NOBELLS,
	K_NOCHAT,				K_AIDECHATHRS,			K_CHATMAIL,
    K_CHATWHY,              K_CHATFLASH,            K_CHATLOOP, K_SPEECHON,
	K_OVREMS,				K_OVREXT,				K_CHECKSYSMAIL,
	K_MEMFREE,				K_SHOWMOVED,			K_ALTF3TIME,
	K_ALTF3MSG, 			K_ALTF3TIMEOUT,
	K_SAVEJB,				K_SWAPNOTE,				K_FASTSCRIPTS,
	K_NEWNODEALERT, 		K_VERBOSECONSOLE,		K_ALTXPROMPT,

// CK_USERCONFIG
	K_NET_PREFIX,			K_DATESTAMP,			K_VDATESTAMP,
	K_PROMPT,				K_MOREPROMPT,

// CK_SYSPERSONALITY
	K_AUTOANSI, 			K_ENTER_NAME,			K_CENSOR,
	K_TWIT_FEATURES,		K_SIGNATURE,			K_ANONAUTHOR,
	K_TWITREGION,			K_TWITCOUNTRY,			K_MESSAGE_NYM,
	K_CREDIT_NYM,			K_BORDER,				K_POOP,
	K_MCI_FIRSTNAME,		K_MCI_NAME, 			K_MCI_TIME,
	K_MCI_DATE, 			K_MCI_POOP, 			K_SLEEPPROMPT,
	K_TWIRLY,				K_FUELBAR,				K_SOFTVERB,
	K_ECTWIRLY, 			K_ECUSERLOG,			K_ECSIGNATURE,
	K_BORDERFREQ,			K_MUSIC,
	K_TWITREV,				K_PERSONALHALLOK,		K_CHATBELL,
	K_ROOM_NYM, 			K_HALL_NYM, 			K_USER_NYM,
	K_GROUP_NYM,			K_FILTER,				K_ADCHANCE,
    K_ADTIME,             

// CK_ACCOUNTING
	K_ACCOUNTING,			K_UNLOGGEDBALANCE,

	K_NWORDS,
    };
#endif



enum ConfigCitDialMacroKeywordsE
	{
	DM_F1,				DM_F2,				DM_F3,
	DM_F4,				DM_F5,				DM_F6,
	DM_F7,				DM_F8,				DM_F9,
	DM_F10, 			DM_F11, 			DM_F12,
	DM_ALT_F1,			DM_ALT_F2,			DM_ALT_F3,
	DM_ALT_F4,			DM_ALT_F5,			DM_ALT_F6,
	DM_ALT_F7,			DM_ALT_F8,			DM_ALT_F9,
	DM_ALT_F10, 		DM_ALT_F11, 		DM_ALT_F12,
	DM_SFT_F1,			DM_SFT_F2,			DM_SFT_F3,
	DM_SFT_F4,			DM_SFT_F5,			DM_SFT_F6,
	DM_SFT_F7,			DM_SFT_F8,			DM_SFT_F9,
	DM_SFT_F10, 		DM_SFT_F11, 		DM_SFT_F12,
	DM_CTL_F1,			DM_CTL_F2,			DM_CTL_F3,
	DM_CTL_F4,			DM_CTL_F5,			DM_CTL_F6,
	DM_CTL_F7,			DM_CTL_F8,			DM_CTL_F9,
	DM_CTL_F10, 		DM_CTL_F11, 		DM_CTL_F12,

	DM_NUMKEYS
	};

enum ConfigCitLoginKeywordsE
	{
	L_CLOSED,			L_VERIFIED, 		L_SYSOP_MESSAGE,
	L_NEW_ACCOUNTS,

	L_NUMLOGIN
	};

enum ConfigCitTwitKeywordsE
	{
	TWIT_MSG_NYMS,		TWIT_BORDER_LINES,	TWIT_TITLES,
	TWIT_NET_TITLES,	TWIT_SURNAMES,		TWIT_NET_SURNAMES,
	TWIT_ENTER_TITLES,	TWIT_COLORS,		TWIT_MCI,

	TWIT_NUM
	};
