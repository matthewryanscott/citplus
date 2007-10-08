/* -------------------------------------------------------------------- */
/*	APSTRUCT.H					Citadel 								*/
/* -------------------------------------------------------------------- */
/*	This file contains the table to generate OUTPUT.APL and INPUT.APL	*/
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*	#defines															*/
/* -------------------------------------------------------------------- */
#define TYP_CHAR	0
#define TYP_INT 	1
#define TYP_STR 	2
#define TYP_BOOL	3
#define TYP_LONG	5
#define TYP_OTHER	6
#define TYP_GRP 	7
#define TYP_END 	8

/* -------------------------------------------------------------------- */
/*	Local variables 													*/
/* -------------------------------------------------------------------- */
struct ApplicTable
	{
	int item;
	void *variable;
	int type;
	int length;
	Bool keep;
	};

static struct ApplicTable AplTab[] =
	{
	// Item 			*Variable		Type		Length		Keep

	{ APL_NAME, 		NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_SURNAME,		NULL,			TYP_OTHER,	LABELSIZE,	TRUE	},
	{ APL_TITLE,		NULL,			TYP_OTHER,	LABELSIZE,	TRUE	},
	{ APL_SYSOP,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_AIDE, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_TWIT, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_EXPERT,		NULL,			TYP_OTHER,	0,			TRUE	},

	{ APL_CREDITS,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_PERMANENT,	NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_NETUSER,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_NOMAIL,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_VERIFIED, 	NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_NULLS,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_LINEFEED, 	NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_UPPERCASE,	NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_ANSION,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_COLUMNS,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_LINES,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_ULISTED,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_MDATA,		NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_BAUD, 		NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_EFF_BAUD, 	NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_PORTRATE, 	NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_CONNRATE, 	NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_NODE, 		NULL,			TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_REGION,		NULL,			TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_COUNTRY,		NULL,			TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_VERSION,		version,		TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_HALL, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_ROOM, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_TEMPPATH, 	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_APPLPATH, 	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_HELPPATH, 	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_HOMEPATH, 	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_ROOMPATH, 	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_MSGPATH,		NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_PRINTPATH,	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_TRANSPATH,	NULL,			TYP_STR,	64, 		FALSE	},
	{ APL_ACCOUNTING,	NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_ATTR, 		NULL,			TYP_CHAR,	0,			FALSE	},
	{ APL_WATTR,		NULL,			TYP_CHAR,	0,			FALSE	},
	{ APL_CATTR,		NULL,			TYP_CHAR,	0,			FALSE	},
	{ APL_UTTR, 		NULL,			TYP_CHAR,	0,			FALSE	},
	{ APL_BATTR,		NULL,			TYP_CHAR,	0,			FALSE	},
	{ APL_TRANSMIT, 	NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_RECEIVE,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_SYSREQ,		&sysReq,		TYP_BOOL,	0,			TRUE	},
	{ APL_CONLOCK,		NULL,			TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_ECHO, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_CHAT, 		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_BELLS,		NULL,			TYP_OTHER,	0,			TRUE	},
	{ APL_PRINTING, 	NULL,			TYP_BOOL,	0,			TRUE	},
	{ GRP_ADD,			NULL,			TYP_GRP,	0,			TRUE	},
	{ GRP_DEL,			NULL,			TYP_GRP,	0,			TRUE	},
	{ APL_TRM_NORM, 	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_TRM_BOLD, 	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_TRM_INVR, 	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_TRM_BLINK,	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_TRM_UNDER,	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},

	{ APL_INITIALS, 	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_PASSWORD, 	NULL,			TYP_OTHER,	LABELSIZE,	FALSE	},
	{ APL_ULMULT,		NULL,			TYP_OTHER,	0,			FALSE	},
	{ APL_DLMULT,		NULL,			TYP_OTHER,	0,			FALSE	},

	{ APL_SYSOPNAME,	NULL,			TYP_STR,	LABELSIZE,	FALSE	},
	{ APL_SOFTWARE, 	programName,	TYP_STR,	LABELSIZE,	FALSE	},

	{ APL_END,			NULL,			TYP_END,	0,			TRUE	}

	};
