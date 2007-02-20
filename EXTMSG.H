// Support for ExtMsg.CPP

enum ExtMsgE
	{
	MSG_NET,		MSG_SYSOP,		MSG_CONFIG,		MSG_ENTERCONF,
	MSG_FSCONFIG,	MSG_CITWIN,		MSG_ZMODEM,		MSG_DEBUG,
	MSG_F4SCREEN,	MSG_CMDLINE,	MSG_XMODEM,		MSG_VIEWLOG,
	MSG_HELP,		MSG_MAILLIST,	MSG_CONSOLE,	MSG_TERMCAP,
	MSG_TRANS,		MSG_OOM,		MSG_AUXMEM,		MSG_READMSG,
    MSG_INFOFILE,   MSG_LOGIN,      MSG_TRAP,

	MSG_NUM
	};


struct MessageTrackerS
	{
	discardable *Data;
	const char **Messages;
	int UseCounter;
	};

extern MessageTrackerS ExtMessages[];

inline const char *getmsg(ExtMsgE msg, uint nmbr)
	{
	// Uncomment this for supreme error checking.
	//assert(ExtMessages[msg].UseCounter);
	return (ExtMessages[msg].Messages[nmbr]);
	}

void DumpUnlockedMessages(void);
Bool LockMessages(ExtMsgE WhichMessage);
void UnlockMessages(ExtMsgE WhichMessage);

#define read_net_messages()	LockMessages(MSG_NET)
#define getnetmsg(nmbr)		getmsg(MSG_NET, nmbr)
#define dump_net_messages()	UnlockMessages(MSG_NET)

#define read_sys_messages()	LockMessages(MSG_SYSOP)
#define getsysmsg(nmbr)		getmsg(MSG_SYSOP, nmbr)
#define dump_sys_messages()	UnlockMessages(MSG_SYSOP)

#define read_cfg_messages()	LockMessages(MSG_CONFIG)
#define getcfgmsg(nmbr)		getmsg(MSG_CONFIG, nmbr)
#define dump_cfg_messages()	UnlockMessages(MSG_CONFIG)

#define read_ec_messages()	LockMessages(MSG_ENTERCONF)
#define getecmsg(nmbr)		getmsg(MSG_ENTERCONF, nmbr)
#define dump_ec_messages()	UnlockMessages(MSG_ENTERCONF)

#define read_fs_messages()	LockMessages(MSG_FSCONFIG)
#define getfsmsg(nmbr)		getmsg(MSG_FSCONFIG, nmbr)
#define dump_fs_messages()	UnlockMessages(MSG_FSCONFIG)

#define read_cw_messages()	LockMessages(MSG_CITWIN)
#define getcwmsg(nmbr)		getmsg(MSG_CITWIN, nmbr)
#define dump_cw_messages()	UnlockMessages(MSG_CITWIN)

#define read_zm_messages()	LockMessages(MSG_ZMODEM)
#define getzmmsg(nmbr)		getmsg(MSG_ZMODEM, nmbr)
#define dump_zm_messages()	UnlockMessages(MSG_ZMODEM)

#define read_db_messages()	LockMessages(MSG_DEBUG)
#define getdbmsg(nmbr)		getmsg(MSG_DEBUG, nmbr)
#define dump_db_messages()	UnlockMessages(MSG_DEBUG)

#define read_tr_messages() LockMessages(MSG_TRAP)
#define gettrmsg(nmbr)     getmsg(MSG_TRAP, nmbr)
#define dump_tr_messages() UnlockMessages(MSG_TRAP)


void PurgeAllSubMessages(void);
void ReloadAllSubMessages(void);

