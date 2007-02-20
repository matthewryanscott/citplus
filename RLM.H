// --------------------------------------------------------------------------
// Citadel: RLM.H
// Everything to be included to create an RLM.


#ifndef CTDL

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

typedef unsigned long	ulong;	// Makes it easier to format
typedef unsigned short	ushort; // Makes it easier to format
typedef unsigned int	uint;	// Makes it easier to format
typedef unsigned char	uchar;	// Makes it easier to format
typedef uint			Bool;	// Makes it easier to format

#define TRUE			1
#define FALSE			0
#define CERROR			-1
#define ULONG_ERROR 	0xFFFFFFFFL
#define UINT_ERROR		0xFFFF

#define LABELSIZE		30		// length of room names
typedef char			label[LABELSIZE+1];

#include "keycodes.h"


// --------------------------------------------------------------------------
// What we are doing... dowhat

typedef enum
	{
	DUNO,		MAINMENU,	SYSOPMENU,	PROMPT, 	MESSAGETEXT,
	DIALOUT,	NETWORKING, ENTERBORDER,READMESSAGE,HELPFILES,
	READUSERLOG,READSTATUS, READCONFIG, READINTRO,	DODOWNLOAD,
	DOUPLOAD,	DOCHAT, 	DOENTER,	DOVOLKS,	DOEXCLUDE,
	DOGOTO, 	DOKNOWN,	DOLOGOUT,	DOREAD, 	DOSMALLCHAT,
	DOAIDE, 	ENTERCONFIG,ENTERDOOR,	ENTERHALL,	ENTERPW,
	ENTERROOM,	ENTERTEXT,	ENTERWC,	ENTERAPP,	ENTERNAME,
	ENTERSUR,	ENTERMENU,	DOBYPASS,	KNOWNROOMS, KNOWNHALLS,
	KNOWNRMINFO,KNOWNMENU,	TERMMENU,	TERMQUIT,	TERMSTAY,
	READDIR,	READINFO,	READHALLS,	READTEXT,	READWC,
	READZIP,	READMENU,	AIDEATTR,	AIDECHAT,	EDITROOM,
	AIDEFILESET,AIDEGROUP,	AIDEHALL,	AIDEINSERT, AIDEKILL,
	AIDELIST,	AIDEMOVE,	AIDENYM,	AIDEQUEUE,	AIDEQUEUEM,
	AIDEQUEUEC, AIDEQUEUEI, AIDEQUEUEK, AIDEQUEUEL, AIDEQUEUER,
	AIDEQUEUES, AIDERENAME, AIDESETINFO,AIDEUNLINK, AIDEVIEW,
	AIDEWINDOW, MOVEROOM,	AIDEMSGIN,	AIDEMENU,	SYSMAINT,
	SYSCRON,	SYSOFFHK,	SYSDATE,	SYSENTER,	SYSGRP,
	SYSHALL,	SYSINFO,	SYSJOURNAL, SYSKUSER,	SYSMASS,
	SYSNUSER,	SYSSCR, 	SYSSHOW,	SYSEDIT,	SYSVIEW,
	SYSEXIT,	SYSKEMPTY,	SYSSHELL,	SYSNET69,	SYSAUXDEB,
	SYSREADMSG, SYSUNLINK,	SYSCFG, 	SYSMENU,	S69FETCH,
	S69INC, 	S69ROOMREQ, S69MENU,	SYSFPDOWN,	SECURITYVIOLATION,
	LOGIN,		SCROLLBACK, CONSYSOP,	FILEQUEUEL, FILEQUEUER,
	FILEQUEUEC, FILEQUEUEA, DOGOTOMAIL, DOPERSONAL, PERSONALMENU,
	PERSONALADD,PERSONALRMV,PERSONALLST,PERSONALUSE,PERSONALADDALL,
	PERSONALADDLOCAL,PERSONALADDROOM,PERSONALADDMENU,PERSONALCLEAR,
	S86FETCH,	S86INC,
	} dowhattypes;

typedef struct sl
	{
	struct sl far *next;
	char string[1];
	} strList;

extern int far *citFunc;
extern void far *cep;
extern int lockCnt;

#include "cwindows.h"

#else
// --------------------------------------------------------------------------
// this structure is emulated in RLMBASE.ASM, and used by Citadel to know how
// to load the RLM.
//
// Of course, I've since lost RLMBASE.ASM, which was required to be linked
// into any RLM created, so if you really want to use any of this code,
// you'll have to figure out what RLMBASE.ASM did. I've forgotten, myself.

typedef struct
	{
	uint sig;				// signature
	uint nameOff;			// offset in segment of name
	uint linkOff;			// offset in segment of linkData
	uint needOff;			// offset in segment of neededData
	uint tempReq;			// bytes requested while loaded
	uint permReq;			// bytes requested always
	uint flags; 			// usage flags
	uint lockCount; 		// how locked is it?
	void *citFunc;			// which Citadel function is being called
	void *cep;				// Where to call it
	void *tmpData;			// Requested tmpData
	void *permData; 		// Requested permData
	} RLMstart;

// a list of the modules
typedef struct rlm
	{
	struct rlm *next;
	fpath fname;				// Where to load it
	label name; 				// which one described?
	char PAD1;
	void *where;				// where memory for it starts
	RLMstart *loadArea; 		// where it is actually loaded
	void *permData; 			// Any permanent data requested
	int flags;					// Flags for dealing with it
	} RLMlist;

// a list of the callable functions
typedef struct rlm2
	{
	struct rlm2 *next;
	RLMlist *theRLM;			// Which one holds this function
	int type;					// When to call this function
	long moreData;				// From linkDataType below
	uint offset;				// Where it is in RLM
	} RLMfuncs;
#endif

typedef enum
	{
	LT_UNUSED,		LT_KEYSTROKE,	LT_LOADMODULE,	LT_DUMPMODULE,
	LT_ENCRYPT, 	LT_DECRYPT, 	LT_DIALOUTKEY,	LT_STARTDIALOUT,
	LT_ENDDIALOUT,

	} LINKTYPE;

typedef enum
	{
	ND_UNUSED,		ND_DOWHAT,		ND_DIALOUTFKEY, ND_SCROLLPOS,
	ND_CONCOLS, 	ND_APPLICPATH,	ND_DIALPREF,	ND_MSTHANDLER,
	ND_MDATA,		ND_BTHANDLER,	ND_SPEED,		ND_CHECKCTS,

	} NEEDTYPE;

typedef enum
	{
	CF_MPRINTF, 	CF_GETMEMG, 	CF_FREEMEMG,	CF_MAKEWINDOW,
	CF_DESTROYWINDOW,CF_MEMSET, 	CF_GETFOCUS,	CF_SETFOCUS,
	CF_SPRINTF, 	CF_FOPEN,		CF_FCLOSE,		CF_FGETS,
	CF_FSEEK,		CF_STRCPY,		CF_STRLEN,		CF_BUILDCLIPARRAY,
	CF_CWOS,		CF_CWOC,		CF_CWCL,		CF_FREECLIPARRAY,
	CF_TOUPPER, 	CF_SENDMESSAGE, CF_STRCAT,		CF_OUTSTRING,
	CF_DISPOSELL,	CF_ADDLL,		CF_GETNEXTLL,	CF_GETLLNUM,
	CF_DELETELLNODE,CF_INSERTLLNODE,CF_GETLLCOUNT,	CF_BRINGTOTOP,
	CF_INITSERIAL,	CF_CLS, 		CF_CPRINTF, 	CF_CWGETYN,
	CF_HANGUP,

	} CITFUNCS;

typedef struct
	{
	int near (*func)(long, long);	// the function we are pointing to
	LINKTYPE type;					// when to call it
	long moreData;					// anything else to tell Citadel
	} linkDataType;

#ifndef CTDL
#define mPrintf(x,y)		*citFunc = CF_MPRINTF; x\
									(((int cdecl (far *)(char far *, ...)) cep)y);\
									asm push cs; asm pop ds;
#define getMemG(x,y)		*citFunc = CF_GETMEMG; x\
									(((void far * (huge *)(ulong, ulong)) cep)y);\
									asm push cs; asm pop ds;
#define freeMemG(x,y)		*citFunc = CF_FREEMEMG; x\
									(((void (far *)(void far *)) cep)y);\
									asm push cs; asm pop ds;
#define makeWindow(x,y) 	*citFunc = CF_MAKEWINDOW; x\
									(((CITWINDOW far *(far *) \
									(Bool (far *)(EVENT, long, int, CITWINDOW far *), \
									CITWINDOW far *, char far *, WINDOWFLAGS, \
									SRECT, void far *, Bool)) cep)y);\
									asm push cs; asm pop ds;
#define destroyWindow(x,y)	*citFunc = CF_DESTROYWINDOW; x\
									(((Bool (far *)(CITWINDOW far *, Bool)) cep)y);\
									asm push cs; asm pop ds;
#define memset(x,y) 		*citFunc = CF_MEMSET; x\
									(((void (far *)(void far *, int, uint)) cep)y);\
									asm push cs; asm pop ds;
#define getFocus(x,y)		*citFunc = CF_GETFOCUS; x\
									(((CITWINDOW far *(far *)(void)) cep)y);\
									asm push cs; asm pop ds;
#define setFocus(x,y)		*citFunc = CF_SETFOCUS; x\
									(((void (far *)(CITWINDOW far *)) cep)y);\
									asm push cs; asm pop ds;
#define sprintf(x,y)		*citFunc = CF_SPRINTF; x\
									(((int cdecl (far *)(char far *, char far *, ...)) cep)y);\
									asm push cs; asm pop ds;
#define fopen(x,y)			*citFunc = CF_FOPEN; x\
									(((FILE far *(far *)(char far *, char far *)) cep)y);\
									asm push cs; asm pop ds;
#define fclose(x,y) 		*citFunc = CF_FCLOSE; x\
									(((void (far *)(FILE far *)) cep)y);\
									asm push cs; asm pop ds;
#define fgets(x,y)			*citFunc = CF_FGETS; x\
									(((char far *(far *)(char far *, int, FILE far *)) cep)y);\
									asm push cs; asm pop ds;
#define fseek(x,y)			*citFunc = CF_FSEEK; x\
									(((long (far *)(FILE far *, long, int)) cep)y);\
									asm push cs; asm pop ds;
#define strcpy(x,y) 		*citFunc = CF_STRCPY; x\
									(((char far *(far *)(char far *, char far *)) cep)y);\
									asm push cs; asm pop ds;
#define strlen(x,y) 		*citFunc = CF_STRLEN; x\
									(((uint (far *)(char far *)) cep)y);\
									asm push cs; asm pop ds;
#define buildClipArray(x,y) *citFunc = CF_BUILDCLIPARRAY; x\
									(((Bool (far *)(CITWINDOW far *)) cep)y);\
									asm push cs; asm pop ds;
#define CitWindowOutStr(x,y)	*citFunc = CF_CWOS; x\
									(((void (far *)(CITWINDOW far *, int, int, char far *, int)) cep)y);\
									asm push cs; asm pop ds;
#define CitWindowOutChr(x,y)	*citFunc = CF_CWOC; x\
									(((void (far *)(CITWINDOW far *, int, int, char, int)) cep)y);\
									asm push cs; asm pop ds;
#define CitWindowClearLine(x,y) *citFunc = CF_CWCL; x\
									(((void (far *)(CITWINDOW far *, int, int)) cep)y);\
									asm push cs; asm pop ds;
#define freeClipArray(x,y)	*citFunc = CF_FREECLIPARRAY; x\
									(((void (far *)(void)) cep)y);\
									asm push cs; asm pop ds;
#define toupper(x,y)		*citFunc = CF_TOUPPER; x\
									(((int (far *)(int)) cep)y);\
									asm push cs; asm pop ds;
#define sendMessage(x,y)	*citFunc = CF_SENDMESSAGE; x\
									(((Bool (far *)(EVENT, long, int, CITWINDOW far *)) cep)y);\
									asm push cs; asm pop ds;
#define strcat(x,y) 		*citFunc = CF_STRCAT; x\
									(((char far *(far *)(char far *, char far *)) cep)y);\
									asm push cs; asm pop ds;
#define outstring(x,y)		*citFunc = CF_OUTSTRING; x\
									(((void (far *)(char far *)) cep)y);\
									asm push cs; asm pop ds;
#define disposeLL(x,y)		*citFunc = CF_DISPOSELL; x\
									(((void (far *)(void far * far *)) cep)y);\
									asm push cs; asm pop ds;
#define addLL(x,y)			*citFunc = CF_ADDLL; x\
									(((void far * (far *)(void far * far *, uint)) cep)y);\
									asm push cs; asm pop ds;
#define getNextLL(x,y)		*citFunc = CF_GETNEXTLL; x\
									(((void far * (far *)(void far *)) cep)y);\
									asm push cs; asm pop ds;
#define getLLNum(x,y)		*citFunc = CF_GETLLNUM; x\
									(((void far * (far *)(void far *, ulong)) cep)y);\
									asm push cs; asm pop ds;
#define deleteLLNode(x,y)	*citFunc = CF_DELETELLNODE; x\
									(((void (far *)(void far * far *, ulong)) cep)y);\
									asm push cs; asm pop ds;
#define insertLLNode(x,y)	*citFunc = CF_INSERTLLNODE; x\
									(((void far *(far *)(void far * far *, ulong, uint)) cep)y);\
									asm push cs; asm pop ds;
#define getLLCount(x,y) 	*citFunc = CF_GETLLCOUNT; x\
									(((ulong (far *)(void far *)) cep)y);\
									asm push cs; asm pop ds;
#define bringToTop(x,y) 	*citFunc = CF_BRINGTOTOP; x\
									(((void (far *)(CITWINDOW far *, Bool, WINDOWLIST far * far *)) cep)y);\
									asm push cs; asm pop ds;
#define initSerial(x,y) 	*citFunc = CF_INITSERIAL; x\
									(((void (far *)(int, int, int, int, int, Bool)) cep)y);\
									asm push cs; asm pop ds;
#define cls(x,y)			*citFunc = CF_CLS; x\
									(((void (far *)(Bool)) cep)y);\
									asm push cs; asm pop ds;
#define cPrintf(x,y)		*citFunc = CF_CPRINTF; x\
									(((int cdecl (far *)(char far *, ...)) cep)y);\
									asm push cs; asm pop ds;
#define WindowedGetYN(x,y)	*citFunc = CF_CWGETYN; x\
									(((void (far *)(char far *, int, CITWINDOW far *, int, void far *)) cep)y);\
									asm push cs; asm pop ds;
#define Hangup(x,y) 		*citFunc = CF_HANGUP; x\
									(((void (far *)(void)) cep)y);\
									asm push cs; asm pop ds;
#endif
