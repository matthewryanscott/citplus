/* -------------------------------------------------------------------- */
/*	APPLIC.H					Citadel 								*/
/* -------------------------------------------------------------------- */
/*	This file contains the table to generate OUTPUT.APL and INPUT.APL	*/
/* -------------------------------------------------------------------- */
/*
 *	Citadel Application Header file
 *	for use with the DragCit BBS system
 *	version 3.11.00d and up.
 *
 *	Writen by Peter Torkelson & Louis Erikson
 *	11/25/89, for the public domain.
 */

/*
 *	Citadel will write an OUTPUT.APL file, and read in an INPUT.APL file.
 *	These files contain infomation that applications might find useful,
 *	the format of the file is as follows: 
 *
 *	Mutiple lines, the first character defines what that line will
 *	contain. The rest of the line is the value in ascii format. The
 *	defines in this file will tell you what that first char means. 
 *	Valid values are 32-249.
 *
 *	Values 250-255 are used for control flags. See below:
 *
 *	APL_EXTEND (254) marks an extended field, this is for
 *	latter expansion, and is yet to be fully defined.
 *
 *	APL_END (255) marks the end of the file. No further data 
 *	should be read past this point.
 *
 *	Future versions of the program may write out more fields, so if you
 *	see something you do not understand, just ignore it. Citadel will do
 *	the same.
 *
 *	Fields that have a comment marked with a '@' or a '!' will be read in 
 *	by Citadel, others will simply be ignored.
 *	'!' fields are not writen out by Citadel...
 *
 *	This method of data transer is being adopted so there can be
 *	additions without screwing up all older applications, as when ANSION
 *	was added to the old structures. Also, this makes it so structures in
 *	programs written with the applic.lib can be bitwise-packed.
 *
 *
 *	If there is something you want to know from Citadel that it knows,
 *	suggest it for future versions. 
 */

/*
 * control stuff
 */
#define APL_EXTEND		254 	/*@ Extended field, for expansion	*/
#define APL_END 		255 	/*@ Last field in file				*/

/*
 * who they are 
 */
#define APL_NAME		32		/*	name of current user				*/
#define APL_SURNAME 	33		/*@ surname of currnet user 			*/
#define APL_TITLE		34		/*@ title of current user				*/
#define APL_SYSOP		35		/*@ is user a sysop? 1 or 0 			*/
#define APL_AIDE		36		/*@ is user an aide? 1 or 0 			*/
#define APL_TWIT		37		/*@ is user twitted? 1 or 0 			*/
#define APL_EXPERT		38		/*@ is user in expert mode? 1 or 0		*/
#define APL_CREDITS 	39		/*@ number of minutes left to the user	*/
#define APL_PERMANENT	40		/*@ user have a permanent acct? 1 or 0	*/
#define APL_NETUSER 	41		/*@ user have net access? 1 or 0		*/
#define APL_NOMAIL		42		/*@ can user send mail? 1 or 0			*/
#define APL_VERIFIED	43		/*@ Verified user? 1 || 0				*/

#define APL_DLMULT		44
#define APL_ULMULT		45
#define APL_INITIALS	46
#define APL_PASSWORD	47
#define APL_REALNAME	48		/*@ user's real name                    */

/*
 * terminal stuff! 
 */
// 49 used below: APL_CONNRATE
#define APL_NULLS		50		/*@ how many nulls do they need?		*/
#define APL_LINEFEED	51		/*@ linefeeds? 1 or 0					*/
#define APL_UPPERCASE	52		/*@ uppercase only terminal? 1 or 0 	*/
#define APL_ANSION		53		/*	UNUSED BY CITADEL					*/
#define APL_COLUMNS 	54		/*@ number of columns on screen 		*/
#define APL_LINES		55		/*@ number lines on screen				*/
#define APL_ULISTED 	56		/*@ unlusted userlog entry? 1 or 0		*/
#define APL_MDATA		57		/*	on what com port?					*/
#define APL_BAUD		58		/*	OLD port rate. Index. Bad.
#define APL_IBM_GRAPH	59		/*	UNUSED BY CITADEL					*/
#define APL_ANSI_FORE	60		// not used by citadel
#define APL_ANSI_BACK	61		// not used by citadel
#define APL_TRM_NORM	62		/* strings to send to modem...			*/
#define APL_TRM_BOLD	63
#define APL_TRM_INVR	64
#define APL_TRM_BLINK	65
#define APL_TRM_UNDER	66
#define APL_EFF_BAUD	67		// OLD Modem connect rate. Index. Bad.

#define APL_PORTRATE	68		// 300, 600, 1200... 115200.
#define APL_CONNRATE	49		// 300, 600, 1200... 230400.

/*
 * where they are 
 */
#define APL_COUNTRY 	69		/*	What country are we in? 			*/
#define APL_NODE		70		/*	What node are we on?				*/
#define APL_REGION		71		/*	Region of the BBS					*/
#define APL_VERSION 	72		/*	text version string from Citadel	*/
#define APL_HALL		73		/*@ name of hall we are in				*/
#define APL_ROOM		74		/*@ name of room we are in				*/
#define APL_TEMPPATH	75		/*	full 'tempptah' path                */
#define APL_APPLPATH	76		/*	full applications directory path	*/
#define APL_HELPPATH	77		/*	full helpfile path					*/
#define APL_HOMEPATH	78		/*	full citadel homepath				*/
#define APL_ROOMPATH	79		/*	full path of room descriptions		*/
#define APL_MSGPATH 	80		/*	full path for messgae base			*/
#define APL_PRINTPATH	81		/*	name of printer capture file		*/
#define APL_TRAPFILE	82		/*	full filename of the TRAP file		*/
#define APL_ACCOUNTING	83		/*	is system accunting on? 			*/
#define APL_TRANSPATH	84		/*	path for transient files (eg mail)	*/
#define APL_SOFTWARE	85		/*	what software this is				*/

/* 
 * console stuff 
 */
#define APL_ATTR		90		/*	main text attribute 				*/
#define APL_WATTR		91		/*	Statline attribute					*/
#define APL_CATTR		92		/*	hi-lited attribute					*/
#define APL_UTTR		93		/*	underlining attribute				*/
#define APL_BATTR		94		/*	Borderline attribute				*/
#define APL_TRANSMIT	95		/*@ number charachters sent by Citadel	*/
#define APL_RECEIVE 	96		/*@ number charachters recived			*/
#define APL_SYSREQ		97		/*@ Has SysReq been pushed? 1 or 0		*/
#define APL_CONLOCK 	98		/*	are Fkeys locked out? 1 or 0		*/
#define APL_ECHO		99		/*@ display paused w/F4 1 or 0			*/
#define APL_CHAT		100 	/*@ is chat mode on or off? 1 or 0		*/
#define APL_BELLS		101 	/*@ are bells on? 1 or 0				*/
#define APL_PRINTING	102 	/*@ is alt-p printer catch on? 1 or 0	*/
#define APL_SYSOPNAME	103 	/*	#SYSOP in CONFIG.CIT				*/

/*
 * Group stuff
 */
#define GRP_IN			110 	/*	The user is in this group.			*/
#define GRP_ADD 		111 	/*! Add the user to this group. 		*/
#define GRP_DEL 		112 	/*! Remove the user from this group.	*/

/*
 * Message Info 
 *
 * For use with the MESSAGE.APL file, tells Citadel what to do with it...
 *
 */
#define MSG_NAME		200 	/*! Name of the person the message is	*/
								/*	from. default: Nodename 			*/
#define MSG_TO			201 	/*! Name of the person the message is	*/
								/*	to. default: none (Public Message)	*/
#define MSG_GROUP		202 	/*! Group the message is for.			*/
								/*	default: none (Public Message)		*/
#define MSG_ROOM		203 	/*! Room the message is in. 			*/
								/*	default: Curent Room				*/
