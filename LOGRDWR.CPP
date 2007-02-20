// --------------------------------------------------------------------------
// Citadel: LogRdWr.CPP
//
// Log entry read/write stuff

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "msg.h"
#include "filerdwr.h"
#include "miscovl.h"


void LogStarter::OpenFile(void)
	{
	assert(this);

	if (!File)
		{
		char FullName[128];
		sprintf(FullName, sbs, cfg.homepath, FileName);

   		OpenCounter = 0;
		File = fopen(FullName, FO_RPB);

		if (!File)
			{
			File = fopen(FullName, FO_WPB);
			}
		}

	if (File)
		{
		OpenCounter++;
		}
	}

void LogStarter::CloseFile(void)
	{
	assert(this);

	if (OpenCounter)
		{
		OpenCounter--;
		}

	if (File && !OpenCounter)
		{
		fclose(File);
		File = NULL;
		}
	}

#ifdef VISUALC
Bool LogEntry::Save(l_index Index, r_slot Room) 
#else
Bool LogEntry::Save(l_index Index, r_slot Room) const
#endif
	{
	assert(this);
	Bool Good = FALSE;

	VerifyHeap();

	label Buffer;
	if (LogEntry1::Save(Index) && LogEntry2::Save(Index) &&
			LogEntry3::Save(Index) && LogEntry4::Save(Index) &&
			LogEntry5::Save(Index) && LogEntry6::Save(Index) &&
			LogExtensions::Save(Index, GetName(Buffer, sizeof(Buffer)), Room)
			)
		{
		Good = TRUE;
		}

	VerifyHeap();
	return (Good);
	}

Bool LogEntry::Load(l_index Index)
	{
	assert(this);
	Bool Good = FALSE;

	VerifyHeap();

	if (LogEntry1::Load(Index) && LogEntry2::Load(Index) &&
			LogEntry3::Load(Index) && LogEntry4::Load(Index) &&
			LogEntry5::Load(Index) && LogEntry6::Load(Index) &&
			LogExtensions::Load(Index))
		{
		Good = TRUE;
		}

	VerifyHeap();
	return (Good);
	}

#include "datafile.h"

Bool LogEntry1::Save(l_index Index)
	{
	Bool Good, WasOpen;
	FILE *fl;

	VerifyHeap();

	label Buffer;
	if (GetFile())
		{
		fl = GetFile();
		WasOpen = TRUE;
		}
	else
		{
		char file[128];
		sprintf(file, sbs, cfg.homepath, GetFileName(Buffer, sizeof(Buffer)));
		fl = fopen(file, FO_RPB);

		if (fl == NULL)
			{
			fl = fopen(file, FO_WPB);
			}

		WasOpen = FALSE;
		}

	if (fl != NULL)
		{
		VerifyHeap();

		LogDatStructure LD;

		// fill up LD
		memset(&LD, 0, sizeof(LD));

		VerifyHeap();

		GetName(LD.lbname,				sizeof(LD.lbname));
		GetInitials(LD.lbin,			sizeof(LD.lbin));
		GetPassword(LD.lbpw,			sizeof(LD.lbpw));
		GetSurname(LD.surname,			sizeof(LD.surname));
		GetTitle(LD.title,				sizeof(LD.title));
		GetRealName(LD.realname,		sizeof(LD.realname));
		GetPhoneNumber(LD.phonenum,		sizeof(LD.phonenum));

		GetForwardAddr(LD.forward,		sizeof(LD.forward));
		GetForwardAddrNode(LD.forward_node,sizeof(LD.forward_node));
		GetForwardAddrRegion(LD.forward_region,sizeof(LD.forward_region));

		GetPromptFormat(LD.prompt,		sizeof(LD.prompt));
		GetDateStamp(LD.dstamp,			sizeof(LD.dstamp));
		GetVerboseDateStamp(LD.vdstamp,	sizeof(LD.vdstamp));
		GetSignature(LD.signature,		sizeof(LD.signature));
		GetNetPrefix(LD.netPrefix,		sizeof(LD.netPrefix));
		GetMailAddr1(LD.addr1,			sizeof(LD.addr1));
		GetMailAddr2(LD.addr2,			sizeof(LD.addr2));
		GetMailAddr3(LD.addr3,			sizeof(LD.addr3));
		GetAlias(LD.alias,				sizeof(LD.alias));
		GetLocID(LD.locID,				sizeof(LD.locID));
		GetMorePrompt(LD.moreprompt,	sizeof(LD.moreprompt));
		GetOccupation(LD.occupation,	sizeof(LD.occupation));
		GetWhereHear(LD.wherehear,		sizeof(LD.wherehear));
		GetLastRoom(LD.lastRoom,		sizeof(LD.lastRoom));
		GetLastHall(LD.lastHall,		sizeof(LD.lastHall));
		GetDefaultRoom(LD.defaultRoom,	sizeof(LD.defaultRoom));
		GetDefaultHall(LD.defaultHall,	sizeof(LD.defaultHall));
		GetTermType(LD.TermType,		sizeof(LD.TermType));

		LD.birthdate =					GetBirthDate();
		LD.firston =					GetFirstOn();
		LD.Sex =						GetSex();

		LD.lbnulls = (char)				GetNulls();
		LD.lbwidth = (char)				GetWidth();
		LD.linesScreen = (char)			GetLinesPerScreen();

		LD.attr[ATTR_BOLD] = (char)		GetAttribute(ATTR_BOLD);
		LD.attr[ATTR_BLINK] = (char) 	GetAttribute(ATTR_BLINK);
		LD.attr[ATTR_NORMAL] = (char)	GetAttribute(ATTR_NORMAL);
		LD.attr[ATTR_REVERSE] = (char)	GetAttribute(ATTR_REVERSE);
		LD.attr[ATTR_UNDERLINE] = (char)GetAttribute(ATTR_UNDERLINE);

		LD.numUserShow = (short)		GetNumUserShow();
		LD.protocol =					GetDefaultProtocol();

		LD.calltime =					GetCallTime();
		LD.callno = 					GetCallNumber();
		LD.totaltime =					GetTotalTime();
		LD.credits =					GetCredits();
		LD.logins = 					GetLogins();
		LD.posted = 					GetPosted();
		LD.read =						GetRead();
		LD.pwChange =					GetPasswordChangeTime();
		LD.callsToday = (short)			GetCallsToday();
		LD.callLimit = (short)			GetCallLimit();

		LD.lastpointer =				GetLastMessage();

		LD.dl_bytes =					GetDL_Bytes();
		LD.ul_bytes =					GetUL_Bytes();
		LD.dl_num = (short)				GetDL_Num();
		LD.ul_num = (short)				GetUL_Num();

		LD.poopcount =					GetPoopcount();

		LD.DUNGEONED =					IsDungeoned();
		LD.FORtOnODE =					IsForwardToNode();
		LD.NEXTHALL =					IsAutoNextHall();
		LD.BORDERS =					IsEnterBorders();
		LD.VERIFIED =					!IsVerified();
		LD.SURNAMLOK =					IsSurnameLocked();
		LD.LOCKHALL =					IsDefaultHallLocked();
		LD.PSYCHO = 					IsPsycho();
		LD.DISPLAYTS =					IsViewTitleSurname();
		LD.SUBJECTS =					IsViewSubjects();
		LD.SIGNATURES = 				IsViewSignatures();
		LD.oldIBMGRAPH =				IsOldIBMGraph();
		LD.oldIBMANSI =					IsOldIBMANSI();
		LD.oldIBMCOLOR =				IsOldIBMColor();
		LD.TWIRLY = 					IsTwirly();
		LD.VERBOSE =					IsAutoVerbose();
		LD.MSGPAUSE =					IsPauseBetweenMessages();
		LD.MINIBIN =					IsMinibin();
		LD.MSGCLS = 					IsClearScreenBetweenMessages();
		LD.ROOMINFO =					IsViewRoomInfoLines();
		LD.HALLTELL =					IsViewHallDescription();
		LD.VERBOSECONT =				IsVerboseContinue();
		LD.VIEWCENSOR = 				IsViewCensoredMessages();
		LD.SEEBORDERS = 				IsViewBorders();
		LD.OUT300 = 					IsOut300();
		LD.LOCKUSIG =					IsUserSignatureLocked();
		LD.HIDEEXCL =					IsHideMessageExclusions();
		LD.NODOWNLOAD = 				!IsDownload();
		LD.NOUPLOAD =					!IsUpload();
		LD.NOCHAT = 					!IsChat();
		LD.PRINTFILE =					IsPrintFile();
		LD.spellCheck = 				GetSpellCheckMode();
		LD.NOMAKEROOM = 				!IsMakeRoom();
		LD.VERBOSELO =					IsVerboseLogOut();
		LD.CONFIRMSAVE =				IsConfirmSave();
		LD.NOCONFABORT =				!IsConfirmAbort();
		LD.CONFIRMNOEO =				IsConfirmNoEO();
		LD.USEPERSONAL =				IsUsePersonalHall();
		LD.YOUAREHERE = 				IsYouAreHere();
		LD.IBMROOM =					IsIBMRoom();
		LD.WIDEROOM =					IsWideRoom();
		LD.MUSIC =						IsMusic();
		LD.CHECKAPS =					IsCheckApostropheS();
		LD.CHECKALLCAP =				IsCheckAllCaps();
		LD.CHECKDIGITS =				IsCheckDigits();
		LD.EXCLUDECRYPT =				IsExcludeEncryptedMessages();
		LD.HIDECOMMAS = 				!IsViewCommas();
		LD.PUNPAUSES =					IsPUnPauses();
		LD.ROMAN =						IsRoman();
		LD.SUPERSYSOP =					IsSuperSysop();
		LD.NOBUNNY =					!IsBunny();
		LD.HSE_LOGONOFF =				!ShowSELogOnOff();
		LD.HSE_NEWMSG =					!ShowSENewMessage();
		LD.HSE_EXCLMSG =				!ShowSEExclusiveMessage();
		LD.HSE_CHATALL =				!ShowSEChatAll();
		LD.HSE_CHATROOM =				!ShowSEChatRoom();
		LD.HSE_CHATGRP =				!ShowSEChatGroup();
		LD.HSE_CHATUSER =				!ShowSEChatUser();
		LD.SSE_RMINOUT =				ShowSERoomInOut();
		LD.BANNED =						IsBanned();
		LD.NOKILLOWN =					!IsKillOwn();

		LD.lbflags.L_INUSE =			IsInuse();
		LD.lbflags.UCMASK = 			IsUpperOnly();
		LD.lbflags.LFMASK = 			IsLinefeeds();
		LD.lbflags.EXPERT = 			IsExpert();
		LD.lbflags.AIDE =				IsAide();
		LD.lbflags.TABS =				IsTabs();
		LD.lbflags.OLDTOO = 			IsOldToo();
		LD.lbflags.PROBLEM =			IsProblem();
		LD.lbflags.UNLISTED =			IsUnlisted();
		LD.lbflags.PERMANENT =			IsPermanent();
		LD.lbflags.SYSOP =				IsSysop();
		LD.lbflags.NODE =				IsNode();
		LD.lbflags.NETUSER =			IsNetUser();
		LD.lbflags.NOACCOUNT =			!IsAccounting();
		LD.lbflags.NOMAIL = 			!IsMail();
		LD.lbflags.ROOMTELL =			IsViewRoomDesc();

		LD.LastCalledVersion =			NumericVer;
				
		LD.SEEOWNCHATS	    =	IsSeeOwnChats();
		LD.ERASEPROMPT	    =	IsErasePrompt();
		LD.AutoIdleSeconds	=	GetAutoIdleSeconds();
        LD.HEARLAUGHTER     =   IsHearLaughter();

		VerifyHeap();

		const long s = (long) Index * LogDatRecordSize(LD) + sizeof(long);
		fseek(fl, s, SEEK_SET);

#ifdef WINCIT
		SetSyncNumber(Index);

		int Bad = fwrite(LD.lbname, sizeof(LD.lbname), 1, fl)			!= 1;
		Bad += fwrite(LD.lbin, sizeof(LD.lbin), 1, fl)					!= 1;
		Bad += fwrite(LD.lbpw, sizeof(LD.lbpw), 1, fl)					!= 1;
		Bad += fwrite(LD.forward, sizeof(LD.forward), 1, fl)			!= 1;
		Bad += fwrite(LD.forward_node, sizeof(LD.forward_node), 1, fl)	!= 1;
		Bad += fwrite(LD.surname, sizeof(LD.surname), 1, fl)			!= 1;
		Bad += fwrite(LD.title, sizeof(LD.title), 1, fl)				!= 1;
		Bad += fwrite(&LD.PAD1, sizeof(LD.PAD1), 1, fl)					!= 1;
		Bad += fwrite(&LD.UNUSED1, sizeof(LD.UNUSED1), 1, fl)			!= 1;
		Bad += fwrite(&LD.lbnulls, sizeof(LD.lbnulls), 1, fl)			!= 1;
		Bad += fwrite(&LD.lbwidth, sizeof(LD.lbwidth), 1, fl)			!= 1;
		Bad += fwrite(&LD.calltime, sizeof(LD.calltime), 1, fl)			!= 1;
		Bad += fwrite(&LD.callno, sizeof(LD.callno), 1, fl)				!= 1;
		Bad += fwrite(&LD.credits, sizeof(LD.credits), 1, fl)			!= 1;
		Bad += fwrite(&LD.lastpointer, sizeof(LD.lastpointer), 1, fl)	!= 1;
		Bad += fwrite(&LD.UNUSED2, sizeof(LD.UNUSED2), 1, fl)			!= 1;
		Bad += fwrite(&LD.PAD2, sizeof(LD.PAD2), 1, fl)					!= 1;
		Bad += fwrite(&LD.UNUSED3, sizeof(LD.UNUSED3), 1, fl)			!= 1;
		Bad += fwrite(&LD.linesScreen, sizeof(LD.linesScreen), 1, fl)	!= 1;
		Bad += fwrite(LD.attr, sizeof(LD.attr), 1, fl)					!= 1;
		Bad += fwrite(&LD.protocol, sizeof(LD.protocol), 1, fl)			!= 1;
		Bad += fwrite(LD.prompt, sizeof(LD.prompt), 1, fl)				!= 1;
		Bad += fwrite(LD.dstamp, sizeof(LD.dstamp), 1, fl)				!= 1;
		Bad += fwrite(LD.vdstamp, sizeof(LD.vdstamp), 1, fl)			!= 1;
		Bad += fwrite(LD.signature, sizeof(LD.signature), 1, fl)		!= 1;
		Bad += fwrite(LD.netPrefix, sizeof(LD.netPrefix), 1, fl)		!= 1;
		Bad += fwrite(LD.addr1, sizeof(LD.addr1), 1, fl)				!= 1;
		Bad += fwrite(LD.addr2, sizeof(LD.addr2), 1, fl)				!= 1;
		Bad += fwrite(LD.addr3, sizeof(LD.addr3), 1, fl)				!= 1;
		Bad += fwrite(LD.alias, sizeof(LD.alias), 1, fl)				!= 1;
		Bad += fwrite(LD.locID, sizeof(LD.locID), 1, fl)				!= 1;
		Bad += fwrite(&LD.PAD3, sizeof(LD.PAD3), 1, fl)					!= 1;
		Bad += fwrite(&LD.dl_bytes, sizeof(LD.dl_bytes), 1, fl)			!= 1;
		Bad += fwrite(&LD.ul_bytes, sizeof(LD.ul_bytes), 1, fl)			!= 1;
		Bad += fwrite(&LD.dl_num, sizeof(LD.dl_num), 1, fl)				!= 1;
		Bad += fwrite(&LD.ul_num, sizeof(LD.ul_num), 1, fl)				!= 1;
		Bad += fwrite(LD.forward_region, sizeof(LD.forward_region), 1, fl)!= 1;
		Bad += fwrite(LD.moreprompt, sizeof(LD.moreprompt), 1, fl)		!= 1;
		Bad += fwrite(LD.occupation, sizeof(LD.occupation), 1, fl)		!= 1;
		Bad += fwrite(&LD.numUserShow, sizeof(LD.numUserShow), 1, fl)	!= 1;
		Bad += fwrite(&LD.birthdate, sizeof(LD.birthdate), 1, fl)		!= 1;
		Bad += fwrite(&LD.firston, sizeof(LD.firston), 1, fl)			!= 1;

		ushort Temp;
		Temp = (ushort) LD.Sex;

		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;
		Bad += fwrite(&LD.totaltime, sizeof(LD.totaltime), 1, fl)		!= 1;
		Bad += fwrite(&LD.logins, sizeof(LD.logins), 1, fl)				!= 1;
		Bad += fwrite(&LD.posted, sizeof(LD.posted), 1, fl)				!= 1;
		Bad += fwrite(&LD.read, sizeof(LD.read), 1, fl)					!= 1;
		Bad += fwrite(&LD.pwChange, sizeof(LD.pwChange), 1, fl)			!= 1;
		Bad += fwrite(&LD.callsToday, sizeof(LD.callsToday), 1, fl)		!= 1;
		Bad += fwrite(&LD.callLimit, sizeof(LD.callLimit), 1, fl)		!= 1;
		Bad += fwrite(LD.wherehear, sizeof(LD.wherehear), 1, fl)		!= 1;

        Temp =  (short) (LD.SEEOWNCHATS | (LD.ERASEPROMPT << 1)             |
                (LD.HEARLAUGHTER << 2));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Bad += fwrite(&LD.AutoIdleSeconds, sizeof(LD.AutoIdleSeconds), 1, fl)	!= 1;		
        Bad += fwrite(&LD.UNUSED4, sizeof(LD.UNUSED4), 1, fl)			!= 1;
		Bad += fwrite(LD.lastRoom, sizeof(LD.lastRoom), 1, fl)			!= 1;
		Bad += fwrite(LD.lastHall, sizeof(LD.lastHall), 1, fl)			!= 1;
		Bad += fwrite(LD.defaultRoom, sizeof(LD.defaultRoom), 1, fl)	!= 1;
		Bad += fwrite(LD.defaultHall, sizeof(LD.defaultHall), 1, fl)	!= 1;
		Bad += fwrite(&LD.poopcount, sizeof(LD.poopcount), 1, fl)		!= 1;

		Temp =	(short) (LD.lbflags.L_INUSE | (LD.lbflags.UCMASK << 1)		|
				(LD.lbflags.LFMASK << 2)	| (LD.lbflags.EXPERT << 3)		|
				(LD.lbflags.AIDE << 4)		| (LD.lbflags.TABS << 5)		|
				(LD.lbflags.OLDTOO << 6)	| (LD.lbflags.PROBLEM << 7)		|
				(LD.lbflags.UNLISTED << 8)	| (LD.lbflags.PERMANENT << 9)	|
				(LD.lbflags.SYSOP << 10)	| (LD.lbflags.NODE << 11)		|
				(LD.lbflags.NETUSER << 12)	| (LD.lbflags.NOACCOUNT << 13)	|
				(LD.lbflags.NOMAIL << 14)	| (LD.lbflags.ROOMTELL << 15));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Temp =	(short) (LD.DUNGEONED		| (LD.MSGAIDE << 1)				|
				(LD.FORtOnODE << 2)			| (LD.NEXTHALL << 3)			|
				(LD.FILEAIDE << 4)			| (LD.BORDERS << 5)				|
				(LD.VERIFIED << 6)			| (LD.SURNAMLOK << 7)			|
				(LD.LOCKHALL << 8)			| (LD.PSYCHO << 9)				|
				(LD.DISPLAYTS << 10)		| (LD.SUBJECTS << 11)			|
				(LD.SIGNATURES << 12)		| (LD.oldIBMGRAPH << 13)		|
				(LD.oldIBMANSI << 14)		| (LD.oldIBMCOLOR << 15));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Temp =	(short) (LD.TWIRLY			| (LD.VERBOSE << 1)				|
				(LD.MSGPAUSE << 2)			| (LD.MINIBIN << 3)				|
				(LD.MSGCLS << 4)			| (LD.ROOMINFO << 5)			|
				(LD.HALLTELL << 6)			| (LD.VERBOSECONT << 7)			|
				(LD.VIEWCENSOR << 8)		| (LD.SEEBORDERS << 9)			|
				(LD.OUT300 << 10)			| (LD.LOCKUSIG << 11)			|
				(LD.HIDEEXCL << 12)			| (LD.NODOWNLOAD << 13)			|
				(LD.NOUPLOAD << 14)			| (LD.NOCHAT << 15));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Temp =	(short) (LD.PRINTFILE		| (LD.spellCheck << 1)			|
				(LD.NOMAKEROOM << 3)		| (LD.VERBOSELO << 4)			|
				(LD.CONFIRMSAVE << 5)		| (LD.NOCONFABORT << 6)			|
				(LD.CONFIRMNOEO << 7)		| (LD.USEPERSONAL << 8)			|
				(LD.YOUAREHERE << 9)		| (LD.IBMROOM  << 10)			|
				(LD.WIDEROOM << 11)			| (LD.MUSIC << 12)				|
				(LD.CHECKAPS << 13)			| (LD.CHECKALLCAP << 14)		|
				(LD.CHECKDIGITS << 15));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Bad += fwrite(&LD.UNUSED5, sizeof(LD.UNUSED5), 1, fl)			!= 1;
		Bad += fwrite(LD.realname, sizeof(LD.realname), 1, fl)			!= 1;
		Bad += fwrite(LD.phonenum, sizeof(LD.phonenum), 1, fl)			!= 1;
		Bad += fwrite(&LD.PAD4, sizeof(LD.PAD4), 1, fl)					!= 1;

		Temp =	(short) (LD.EXCLUDECRYPT	| (LD.HIDECOMMAS << 1)			|
				(LD.PUNPAUSES << 2)			| (LD.ROMAN << 3)				|
				(LD.SUPERSYSOP << 4)		| (LD.NOBUNNY << 5)				|
				(LD.HSE_LOGONOFF << 6)		| (LD.HSE_NEWMSG << 7)			|
				(LD.HSE_EXCLMSG << 8)		| (LD.HSE_CHATALL << 9)			|
				(LD.HSE_CHATROOM << 10)		| (LD.HSE_CHATGRP << 11)		|
				(LD.HSE_CHATUSER << 12)		| (LD.SSE_RMINOUT << 13)		|
				(LD.BANNED << 14)			| (LD.NOKILLOWN << 15));
		Bad += fwrite(&Temp, sizeof(Temp), 1, fl)						!= 1;

		Bad += fwrite(&LD.LastCalledVersion, sizeof(LD.LastCalledVersion), 1, fl)!= 1;
		Bad += fwrite(LD.TermType, sizeof(LD.TermType), 1, fl)			!= 1;
		Bad += fwrite(&LD.PAD5, sizeof(LD.PAD5), 1, fl)					!= 1;

		Good = !Bad;
#else
		if (fwrite(&LD, sizeof(LD), 1, fl) != 1)
			{
			label Buffer;
			mPrintf(getmsg(661), GetFileName(Buffer, sizeof(Buffer)));
			Good = FALSE;
			}
		else
			{
			Good = TRUE;
			}
#endif

		if (SameString(GetFileName(Buffer, sizeof(Buffer)), logDat))
			{
			LogTab.UpdateTable(this, Index);
			}

		if (!WasOpen)
			{
			fclose(fl);
			}
		}
	else
		{
#ifndef WINCIT
		label Buffer;
		mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool LogEntry1::Load(l_index Index)
	{
	assert(this);

#ifdef WINCIT
	SetSyncNumber(Index);
	LogEntry1Data *Other = (LogEntry1Data *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
#endif

	Bool Good, WasOpen;
	FILE *fl;

	if (GetFile())
		{
		fl = GetFile();
		WasOpen = TRUE;
		}
	else
		{
		char file[128];
		label Buffer;
		sprintf(file, sbs, cfg.homepath, GetFileName(Buffer, sizeof(Buffer)));
		fl = fopen(file, FO_RB);

		WasOpen = FALSE;
		}

	if (fl != NULL)
		{
		struct LogDatStructure LD;

		const long s = (long) Index * LogDatRecordSize(LD) + sizeof(long);
		fseek(fl, s, SEEK_SET);

		VerifyHeap();

#ifdef WINCIT
		int Bad = fread(LD.lbname, sizeof(LD.lbname), 1, fl)			!= 1;
		Bad += fread(LD.lbin, sizeof(LD.lbin), 1, fl)					!= 1;
		Bad += fread(LD.lbpw, sizeof(LD.lbpw), 1, fl)					!= 1;
		Bad += fread(LD.forward, sizeof(LD.forward), 1, fl)				!= 1;
		Bad += fread(LD.forward_node, sizeof(LD.forward_node), 1, fl)	!= 1;
		Bad += fread(LD.surname, sizeof(LD.surname), 1, fl)				!= 1;
		Bad += fread(LD.title, sizeof(LD.title), 1, fl)					!= 1;
		Bad += fread(&LD.PAD1, sizeof(LD.PAD1), 1, fl)					!= 1;
		Bad += fread(&LD.UNUSED1, sizeof(LD.UNUSED1), 1, fl)			!= 1;
		Bad += fread(&LD.lbnulls, sizeof(LD.lbnulls), 1, fl)			!= 1;
		Bad += fread(&LD.lbwidth, sizeof(LD.lbwidth), 1, fl)			!= 1;
		Bad += fread(&LD.calltime, sizeof(LD.calltime), 1, fl)			!= 1;
		Bad += fread(&LD.callno, sizeof(LD.callno), 1, fl)				!= 1;
		Bad += fread(&LD.credits, sizeof(LD.credits), 1, fl)			!= 1;
		Bad += fread(&LD.lastpointer, sizeof(LD.lastpointer), 1, fl)	!= 1;
		Bad += fread(&LD.UNUSED2, sizeof(LD.UNUSED2), 1, fl)			!= 1;
		Bad += fread(&LD.PAD2, sizeof(LD.PAD2), 1, fl)					!= 1;
		Bad += fread(&LD.UNUSED3, sizeof(LD.UNUSED3), 1, fl)			!= 1;
		Bad += fread(&LD.linesScreen, sizeof(LD.linesScreen), 1, fl)	!= 1;
		Bad += fread(LD.attr, sizeof(LD.attr), 1, fl)					!= 1;
		Bad += fread(&LD.protocol, sizeof(LD.protocol), 1, fl)			!= 1;
		Bad += fread(LD.prompt, sizeof(LD.prompt), 1, fl)				!= 1;
		Bad += fread(LD.dstamp, sizeof(LD.dstamp), 1, fl)				!= 1;
		Bad += fread(LD.vdstamp, sizeof(LD.vdstamp), 1, fl)				!= 1;
		Bad += fread(LD.signature, sizeof(LD.signature), 1, fl)			!= 1;
		Bad += fread(LD.netPrefix, sizeof(LD.netPrefix), 1, fl)			!= 1;
		Bad += fread(LD.addr1, sizeof(LD.addr1), 1, fl)					!= 1;
		Bad += fread(LD.addr2, sizeof(LD.addr2), 1, fl)					!= 1;
		Bad += fread(LD.addr3, sizeof(LD.addr3), 1, fl)					!= 1;
		Bad += fread(LD.alias, sizeof(LD.alias), 1, fl)					!= 1;
		Bad += fread(LD.locID, sizeof(LD.locID), 1, fl)					!= 1;
		Bad += fread(&LD.PAD3, sizeof(LD.PAD3), 1, fl)					!= 1;
		Bad += fread(&LD.dl_bytes, sizeof(LD.dl_bytes), 1, fl)			!= 1;
		Bad += fread(&LD.ul_bytes, sizeof(LD.ul_bytes), 1, fl)			!= 1;
		Bad += fread(&LD.dl_num, sizeof(LD.dl_num), 1, fl)				!= 1;
		Bad += fread(&LD.ul_num, sizeof(LD.ul_num), 1, fl)				!= 1;
		Bad += fread(LD.forward_region, sizeof(LD.forward_region), 1, fl)!= 1;
		Bad += fread(LD.moreprompt, sizeof(LD.moreprompt), 1, fl)		!= 1;
		Bad += fread(LD.occupation, sizeof(LD.occupation), 1, fl)		!= 1;
		Bad += fread(&LD.numUserShow, sizeof(LD.numUserShow), 1, fl)	!= 1;
		Bad += fread(&LD.birthdate, sizeof(LD.birthdate), 1, fl)		!= 1;
		Bad += fread(&LD.firston, sizeof(LD.firston), 1, fl)			!= 1;

		ushort Temp;

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.Sex = (SexE) Temp;

		Bad += fread(&LD.totaltime, sizeof(LD.totaltime), 1, fl)		!= 1;
		Bad += fread(&LD.logins, sizeof(LD.logins), 1, fl)				!= 1;
		Bad += fread(&LD.posted, sizeof(LD.posted), 1, fl)				!= 1;
		Bad += fread(&LD.read, sizeof(LD.read), 1, fl)					!= 1;
		Bad += fread(&LD.pwChange, sizeof(LD.pwChange), 1, fl)			!= 1;
		Bad += fread(&LD.callsToday, sizeof(LD.callsToday), 1, fl)		!= 1;
		Bad += fread(&LD.callLimit, sizeof(LD.callLimit), 1, fl)		!= 1;
		Bad += fread(LD.wherehear, sizeof(LD.wherehear), 1, fl)			!= 1;
     	
        Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
        LD.SEEOWNCHATS  = Temp & 1;         
        LD.ERASEPROMPT  = (Temp >> 1) & 1;
        LD.HEARLAUGHTER = (Temp >> 2) & 1;
		Bad += fread(&LD.AutoIdleSeconds, sizeof(LD.AutoIdleSeconds), 1, fl)	!= 1;

        Bad += fread(&LD.UNUSED4, sizeof(LD.UNUSED4), 1, fl)			!= 1;
		Bad += fread(LD.lastRoom, sizeof(LD.lastRoom), 1, fl)			!= 1;
		Bad += fread(LD.lastHall, sizeof(LD.lastHall), 1, fl)			!= 1;
		Bad += fread(LD.defaultRoom, sizeof(LD.defaultRoom), 1, fl)		!= 1;
		Bad += fread(LD.defaultHall, sizeof(LD.defaultHall), 1, fl)		!= 1;
		Bad += fread(&LD.poopcount, sizeof(LD.poopcount), 1, fl)		!= 1;

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.lbflags.L_INUSE = Temp & 1;
		LD.lbflags.UCMASK = (Temp >> 1) & 1;
		LD.lbflags.LFMASK = (Temp >> 2) & 1;
		LD.lbflags.EXPERT = (Temp >> 3) & 1;
		LD.lbflags.AIDE = (Temp >> 4) & 1;
		LD.lbflags.TABS = (Temp >> 5) & 1;
		LD.lbflags.OLDTOO = (Temp >> 6) & 1;
		LD.lbflags.PROBLEM = (Temp >> 7) & 1;
		LD.lbflags.UNLISTED = (Temp >> 8) & 1;
		LD.lbflags.PERMANENT = (Temp >> 9) & 1;
		LD.lbflags.SYSOP = (Temp >> 10) & 1;
		LD.lbflags.NODE = (Temp >> 11) & 1;
		LD.lbflags.NETUSER = (Temp >> 12) & 1;
		LD.lbflags.NOACCOUNT = (Temp >> 13) & 1;
		LD.lbflags.NOMAIL = (Temp >> 14) & 1;
		LD.lbflags.ROOMTELL = (Temp >> 15);

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.DUNGEONED = Temp & 1;
		LD.MSGAIDE = (Temp >> 1) & 1;
		LD.FORtOnODE = (Temp >> 2) & 1;
		LD.NEXTHALL = (Temp >> 3) & 1;
		LD.FILEAIDE = (Temp >> 4) & 1;
		LD.BORDERS = (Temp >> 5) & 1;
		LD.VERIFIED = (Temp >> 6) & 1;
		LD.SURNAMLOK = (Temp >> 7) & 1;
		LD.LOCKHALL = (Temp >> 8) & 1;
		LD.PSYCHO = (Temp >> 9) & 1;
		LD.DISPLAYTS = (Temp >> 10) & 1;
		LD.SUBJECTS = (Temp >> 11) & 1;
		LD.SIGNATURES = (Temp >> 12) & 1;
		LD.oldIBMGRAPH = (Temp >> 13) & 1;
		LD.oldIBMANSI = (Temp >> 14) & 1;
		LD.oldIBMCOLOR = (Temp >> 15);

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.TWIRLY = Temp & 1;
		LD.VERBOSE = (Temp >> 1) & 1;
		LD.MSGPAUSE = (Temp >> 2) & 1;
		LD.MINIBIN = (Temp >> 3) & 1;
		LD.MSGCLS = (Temp >> 4) & 1;
		LD.ROOMINFO = (Temp >> 5) & 1;
		LD.HALLTELL = (Temp >> 6) & 1;
		LD.VERBOSECONT = (Temp >> 7) & 1;
		LD.VIEWCENSOR = (Temp >> 8) & 1;
		LD.SEEBORDERS = (Temp >> 9) & 1;
		LD.OUT300 = (Temp >> 10) & 1;
		LD.LOCKUSIG = (Temp >> 11) & 1;
		LD.HIDEEXCL = (Temp >> 12) & 1;
		LD.NODOWNLOAD = (Temp >> 13) & 1;
		LD.NOUPLOAD = (Temp >> 14) & 1;
		LD.NOCHAT = (Temp >> 15);

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.PRINTFILE = Temp & 1;
		LD.spellCheck = (Temp >> 1) & 3;
		LD.NOMAKEROOM = (Temp >> 3) & 1;
		LD.VERBOSELO = (Temp >> 4) & 1;
		LD.CONFIRMSAVE = (Temp >> 5) & 1;
		LD.NOCONFABORT = (Temp >> 6) & 1;
		LD.CONFIRMNOEO = (Temp >> 7) & 1;
		LD.USEPERSONAL = (Temp >> 8) & 1;
		LD.YOUAREHERE = (Temp >> 9) & 1;
		LD.IBMROOM  = (Temp >> 10) & 1;
		LD.WIDEROOM = (Temp >> 11) & 1;
		LD.MUSIC = (Temp >> 12) & 1;
		LD.CHECKAPS = (Temp >> 13) & 1;
		LD.CHECKALLCAP = (Temp >> 14) & 1;
		LD.CHECKDIGITS = (Temp >> 15);

		Bad += fread(&LD.UNUSED5, sizeof(LD.UNUSED5), 1, fl)			!= 1;
		Bad += fread(LD.realname, sizeof(LD.realname), 1, fl)			!= 1;
		Bad += fread(LD.phonenum, sizeof(LD.phonenum), 1, fl)			!= 1;
		Bad += fread(&LD.PAD4, sizeof(LD.PAD4), 1, fl)					!= 1;

		Bad += fread(&Temp, sizeof(Temp), 1, fl)						!= 1;
		LD.EXCLUDECRYPT = Temp & 1;
		LD.HIDECOMMAS = (Temp >> 1) & 1;
		LD.PUNPAUSES = (Temp >> 2) & 1;
		LD.ROMAN = (Temp >> 3) & 1;
		LD.SUPERSYSOP = (Temp >> 4) & 1;
		LD.NOBUNNY = (Temp >> 5) & 1;
		LD.HSE_LOGONOFF = (Temp >> 6) & 1;
		LD.HSE_NEWMSG = (Temp >> 7) & 1;
		LD.HSE_EXCLMSG = (Temp >> 8) & 1;
		LD.HSE_CHATALL = (Temp >> 9) & 1;
		LD.HSE_CHATROOM = (Temp >> 10) & 1;
		LD.HSE_CHATGRP = (Temp >> 11) & 1;
		LD.HSE_CHATUSER = (Temp >> 12) & 1;
		LD.SSE_RMINOUT = (Temp >> 13) & 1;
		LD.BANNED = (Temp >> 14) & 1;
		LD.NOKILLOWN = (Temp >> 15);

		Bad += fread(&LD.LastCalledVersion, sizeof(LD.LastCalledVersion), 1, fl)!= 1;
		Bad += fread(LD.TermType, sizeof(LD.TermType), 1, fl)			!= 1;
		Bad += fread(&LD.PAD5, sizeof(LD.PAD5), 1, fl)					!= 1;

		Good = !Bad;
#else
		if (fread(&LD, sizeof(LD), 1, fl) != 1)
			{
			label Buffer;
			mPrintf(getmsg(83), GetFileName(Buffer, sizeof(Buffer)));
			Good = FALSE;
			}
		else
			{
			Good = TRUE;
			}
#endif

		VerifyHeap();

		if (!WasOpen)
			{
			fclose(fl);
			}

		if (Good)
			{
			// now, copy it... oh boy
			Clear();

			SetName(LD.lbname);
			SetInitials(LD.lbin);
			SetPassword(LD.lbpw);
			SetSurname(LD.surname);
			SetTitle(LD.title);
			SetRealName(LD.realname);
			SetPhoneNumber(LD.phonenum);

			SetForwardAddr(LD.forward);
			SetForwardAddrNode(LD.forward_node);
			SetForwardAddrRegion(LD.forward_region);

			SetPromptFormat(LD.prompt);
			SetDateStamp(LD.dstamp);
			SetVerboseDateStamp(LD.vdstamp);
			SetSignature(LD.signature);
			SetNetPrefix(LD.netPrefix);
			SetMailAddr1(LD.addr1);
			SetMailAddr2(LD.addr2);
			SetMailAddr3(LD.addr3);
			SetAlias(LD.alias);
			SetLocID(LD.locID);
			SetMorePrompt(LD.moreprompt);
			SetOccupation(LD.occupation);
			SetWhereHear(LD.wherehear);
			SetLastRoom(LD.lastRoom);
			SetLastHall(LD.lastHall);
			SetDefaultRoom(LD.defaultRoom);
			SetDefaultHall(LD.defaultHall);
			SetTermType(LD.TermType);

			SetBirthDate(LD.birthdate);
			SetFirstOn(LD.firston);
			SetSex(LD.Sex);

			SetNulls(LD.lbnulls);
			SetWidth(LD.lbwidth);
			SetLinesPerScreen(LD.linesScreen);

			SetAttribute(ATTR_BOLD, LD.attr[ATTR_BOLD]);
			SetAttribute(ATTR_BLINK, LD.attr[ATTR_BLINK]);
			SetAttribute(ATTR_NORMAL, LD.attr[ATTR_NORMAL]);
			SetAttribute(ATTR_REVERSE, LD.attr[ATTR_REVERSE]);
			SetAttribute(ATTR_UNDERLINE, LD.attr[ATTR_UNDERLINE]);

			SetNumUserShow(LD.numUserShow);
			SetDefaultProtocol(LD.protocol);

			SetCallTime(LD.calltime);
			SetCallNumber(LD.callno);
			SetTotalTime(LD.totaltime);
			SetCredits(LD.credits);
			SetLogins(LD.logins);
			SetPosted(LD.posted);
			SetRead(LD.read);
			SetPasswordChangeTime(LD.pwChange);
			SetCallsToday(LD.callsToday);
			SetCallLimit(LD.callLimit);

			SetLastMessage(LD.lastpointer);

			SetDL_Bytes(LD.dl_bytes);
			SetUL_Bytes(LD.ul_bytes);
			SetDL_Num(LD.dl_num);
			SetUL_Num(LD.ul_num);

			SetPoopcount(LD.poopcount);

			SetDungeoned(LD.DUNGEONED);
			SetForwardToNode(LD.FORtOnODE);
			SetAutoNextHall(LD.NEXTHALL);
			SetEnterBorders(LD.BORDERS);
			SetVerified(!LD.VERIFIED);
			SetSurnameLocked(LD.SURNAMLOK);
			SetDefaultHallLocked(LD.LOCKHALL);
			SetPsycho(LD.PSYCHO);
			SetViewTitleSurname(LD.DISPLAYTS);
			SetViewSubjects(LD.SUBJECTS);
			SetViewSignatures(LD.SIGNATURES);
			SetOldIBMGraph(LD.oldIBMGRAPH);
			SetOldIBMANSI(LD.oldIBMANSI);
			SetOldIBMColor(LD.oldIBMCOLOR);
			SetTwirly(LD.TWIRLY);
			SetAutoVerbose(LD.VERBOSE);
			SetPauseBetweenMessages(LD.MSGPAUSE);
			SetMinibin(LD.MINIBIN);
			SetClearScreenBetweenMessages(LD.MSGCLS);
			SetViewRoomInfoLines(LD.ROOMINFO);
			SetViewHallDescription(LD.HALLTELL);
			SetVerboseContinue(LD.VERBOSECONT);
			SetViewCensoredMessages(LD.VIEWCENSOR);
			SetViewBorders(LD.SEEBORDERS);
			SetOut300(LD.OUT300);
			SetUserSignatureLocked(LD.LOCKUSIG);
			SetHideMessageExclusions(LD.HIDEEXCL);
			SetDownload(!LD.NODOWNLOAD);
			SetUpload(!LD.NOUPLOAD);
			SetChat(!LD.NOCHAT);
			SetPrintFile(LD.PRINTFILE);
			SetSpellCheckMode(LD.spellCheck);
			SetMakeRoom(!LD.NOMAKEROOM);
			SetVerboseLogOut(LD.VERBOSELO);
			SetConfirmSave(LD.CONFIRMSAVE);
			SetConfirmAbort(!LD.NOCONFABORT);
			SetConfirmNoEO(LD.CONFIRMNOEO);
			SetUsePersonalHall(LD.USEPERSONAL);
			SetYouAreHere(LD.YOUAREHERE);
			SetIBMRoom(LD.IBMROOM);
			SetWideRoom(LD.WIDEROOM);
			SetMusic(LD.MUSIC);
			SetCheckApostropheS(LD.CHECKAPS);
			SetCheckAllCaps(LD.CHECKALLCAP);
			SetCheckDigits(LD.CHECKDIGITS);
			SetExcludeEncryptedMessages(LD.EXCLUDECRYPT);
			SetViewCommas(!LD.HIDECOMMAS);
			SetPUnPauses(LD.PUNPAUSES);
			SetRoman(LD.ROMAN);
			SetSuperSysop(LD.SUPERSYSOP);
			SetBunny(!LD.NOBUNNY);
			SetSELogOnOff(!LD.HSE_LOGONOFF);
			SetSENewMessage(!LD.HSE_NEWMSG);
			SetSEExclusiveMessage(!LD.HSE_EXCLMSG);
			SetSEChatAll(!LD.HSE_CHATALL);
			SetSEChatRoom(!LD.HSE_CHATROOM);
			SetSEChatGroup(!LD.HSE_CHATGRP);
			SetSEChatUser(!LD.HSE_CHATUSER);
			SetSERoomInOut(LD.SSE_RMINOUT);
			SetBanned(LD.BANNED);
			SetKillOwn(!LD.NOKILLOWN);

			SetInuse(LD.lbflags.L_INUSE);
			SetUpperOnly(LD.lbflags.UCMASK);
			SetLinefeeds(LD.lbflags.LFMASK);
			SetExpert(LD.lbflags.EXPERT);
			SetAide(LD.lbflags.AIDE);
			SetTabs(LD.lbflags.TABS);
			SetOldToo(LD.lbflags.OLDTOO);
			SetProblem(LD.lbflags.PROBLEM);
			SetUnlisted(LD.lbflags.UNLISTED);
			SetPermanent(LD.lbflags.PERMANENT);
			SetSysop(LD.lbflags.SYSOP);
			SetNode(LD.lbflags.NODE);
			SetNetUser(LD.lbflags.NETUSER);
			SetAccounting(!LD.lbflags.NOACCOUNT);
			SetMail(!LD.lbflags.NOMAIL);
			SetViewRoomDesc(LD.lbflags.ROOMTELL);

			SetSeeOwnChats(LD.SEEOWNCHATS);
			SetErasePrompt(LD.ERASEPROMPT);
			SetAutoIdleSeconds(LD.AutoIdleSeconds);
            SetHearLaughter(LD.HEARLAUGHTER);

            // ok, fine...
            if (LD.LastCalledVersion < 66065l)
                {
                SetHearLaughter(TRUE);
                }

            if (LD.LastCalledVersion < 66056l)
                {
                SetSeeOwnChats(TRUE);
                // Just in case
	            SetErasePrompt(FALSE);
			    SetAutoIdleSeconds(0);
                }

			// MorePrompt started being used in version 65074
			if (LD.LastCalledVersion < 65074l)
				{
				SetMorePrompt(cfg.moreprompt);
				}

			// Real terminal support added in 66003
			if (LD.LastCalledVersion < 66003l)
				{
				if (IsOldIBMColor())
					{
					if (IsOldIBMGraph())
						{
						SetTermType("ANSI Color");
						}
					else
						{
						SetTermType("Non-IBM ANSI Color");
						}
					}
				else
					{
					if (IsOldIBMANSI())
						{
						if (IsOldIBMGraph())
							{
							SetTermType("ANSI Mono");
							}
						else
							{
							SetTermType("Non-IBM ANSI Mono");
							}
						}
					else
						{
						if (IsOldIBMGraph())
							{
							SetTermType("Non-ANSI IBM");
							}
						else
							{
							SetTermType("Dumb");
							}
						}
					}
				}

#ifdef WINCIT
			SetSyncNumber(Index);
#endif
			}
		}
	else
		{
#ifndef WINCIT
		label Buffer;
		mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool LogBitBag::Save(l_index Index)
	{
	assert(this);
	VerifyHeap();

	Bool Good = TRUE;

	if (IsValid())
		{
#ifdef WINCIT
		SyncNumber = Index;
#endif
		FILE *fl = GetFile();
		Bool WasOpen;

		if (fl)
			{
			WasOpen = TRUE;
			}
		else
			{
			char file[128];
			label Buffer;
			sprintf(file, sbs, cfg.homepath, GetFileName(Buffer,
					sizeof(Buffer)));
			fl = fopen(file, FO_RPB);

			if (fl == NULL)
				{
				fl = fopen(file, FO_WPB);
				}

			WasOpen = FALSE;
			}

		if (fl != NULL)
			{
			fseek(fl, (long) Index * (long) GetSize(), SEEK_SET);

			if (fwrite(GetPointer(), 1, GetSize(), fl) != GetSize())
				{
#ifndef WINCIT
				label Buffer;
				mPrintf(getmsg(661), GetFileName(Buffer, sizeof(Buffer)));
#endif
				Good = FALSE;
				}

			if (!WasOpen)
				{
				fclose(fl);
				}
			}
		else
			{
#ifndef WINCIT
			label Buffer;
			mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

#ifdef WINCIT
Bool LogEntry2::Load(l_index Index)
	{
	SyncNumber = Index;

	LogEntry2 *Other = (LogEntry2 *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
	else
		{
		return (LogBitBag::Load(Index));
		}
	}

Bool LogEntry3::Load(l_index Index)
	{
	SyncNumber = Index;

	LogEntry3 *Other = (LogEntry3 *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
	else
		{
		return (LogBitBag::Load(Index));
		}
	}

Bool LogEntry4::Load(l_index Index)
	{
	SyncNumber = Index;

	LogEntry4 *Other = (LogEntry4 *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
	else
		{
		return (LogBitBag::Load(Index));
		}
	}

Bool LogEntry6::Load(l_index Index)
	{
	SyncNumber = Index;

	LogEntry6 *Other = (LogEntry6 *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
	else
		{
		return (LogBitBag::Load(Index));
		}
	}
#endif

Bool LogBitBag::Load(l_index Index)
	{
	assert(this);
	VerifyHeap();
	Bool Good = TRUE;

	if (IsValid())
		{
#ifdef WINCIT
		SyncNumber = Index;
#endif

		FILE *fl;
		Bool WasOpen;

		if (GetFile())
			{
			fl = GetFile();
			WasOpen = TRUE;
			}
		else
			{
			char file[128];
			label Buffer;
			sprintf(file, sbs, cfg.homepath, GetFileName(Buffer,
					sizeof(Buffer)));
			fl = fopen(file, FO_RB);

			WasOpen = FALSE;
			}

		if (fl != NULL)
			{
			fseek(fl, (long) Index * (long) GetSize(), SEEK_SET);

			if (fread(GetNCPointer(), 1, GetSize(), fl) != GetSize())
				{
#ifndef WINCIT
				label Buffer;
				mPrintf(getmsg(83), GetFileName(Buffer, sizeof(Buffer)));
#endif
				Good = FALSE;
				}

			if (!WasOpen)
				{
				fclose(fl);
				}
			}
		else
			{
#ifndef WINCIT
			label Buffer;
			mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool LogEntry5::Save(l_index Index)
	{
	assert(this);
	VerifyHeap();
	Bool Good = TRUE;

	if (IsValid())
		{
#ifdef WINCIT
		SyncNumber = Index;
#endif
		FILE *fl;
		Bool WasOpen;

		if (GetFile())
			{
			fl = GetFile();
			WasOpen = TRUE;
			}
		else
			{
			char file[128];
			label Buffer;
			sprintf(file, sbs, cfg.homepath, GetFileName(Buffer, sizeof(Buffer)));
			fl = fopen(file, FO_RPB);

			if (fl == NULL)
				{
				fl = fopen(file, FO_WPB);
				}

			WasOpen = FALSE;
			}

		if (fl != NULL)
			{
			const long s = (long) Index * (long) sizeof(*NewPointer) * MaxRooms;

			fseek(fl, s, SEEK_SET);

			if (fwrite(NewPointer, sizeof(*NewPointer), MaxRooms, fl) != (size_t) MaxRooms)
				{
#ifndef WINCIT
				label Buffer;
				mPrintf(getmsg(661), GetFileName(Buffer, sizeof(Buffer)));
#endif
				Good = FALSE;
				}

			if (!WasOpen)
				{
				fclose(fl);
				}
			}
		else
			{
#ifndef WINCIT
			label Buffer;
			mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool LogEntry5::Load(l_index Index)
	{
	assert(this);

#ifdef WINCIT
	SyncNumber = Index;
	LogEntry5 *Other = (LogEntry5 *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
#endif


	VerifyHeap();
	Bool Good = TRUE;

	if (IsValid())
		{
#ifdef WINCIT
		SyncNumber = Index;
#endif
		FILE *fl;
		Bool WasOpen;

		if (GetFile())
			{
			fl = GetFile();
			WasOpen = TRUE;
			}
		else
			{
			char file[128];
			label Buffer;
			sprintf(file, sbs, cfg.homepath, GetFileName(Buffer,
					sizeof(Buffer)));
			fl = fopen(file, FO_RB);

			WasOpen = FALSE;
			}

		if (fl != NULL)
			{
			long s;

			s = (long) Index * (long) sizeof(*NewPointer) * MaxRooms;

			fseek(fl, s, SEEK_SET);

			if (fread(NewPointer, sizeof(*NewPointer), MaxRooms, fl) != (size_t) MaxRooms)
				{
#ifndef WINCIT
				label Buffer;
				mPrintf(getmsg(83), GetFileName(Buffer, sizeof(Buffer)));
#endif
				Good = FALSE;
				}

			if (!WasOpen)
				{
				fclose(fl);
				}
			}
		else
			{
#ifndef WINCIT
			label Buffer;
			mPrintf(getmsg(78), GetFileName(Buffer, sizeof(Buffer)));
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

class LEMessageC: MessageStoreC
	{
	FILE *LEFile;

	virtual Bool SaveByte(char Byte);
	virtual Bool SaveString(const char *String);
	virtual Bool GetByte(char *Byte);

public:
	LEMessageC(FILE *File)
		{
		LEFile = File;
		}

	Bool Store(Message *Msg);
	ReadMessageStatus LoadAll(Message *Msg);
	};

Bool LEMessageC::SaveByte(char Byte)
	{
	assert(LEFile);
	return (fputc(Byte, LEFile) != EOF);
	}

Bool LEMessageC::SaveString(const char *String)
	{
	assert(LEFile);
	assert(String);

	return (PutStr(LEFile, String));
	}

Bool LEMessageC::GetByte(char *Byte)
	{
	assert(LEFile);

	if (feof(LEFile))
		{
		*Byte = 0;
		return (FALSE);
		}
	else 
		{
		*Byte = (char) fgetc(LEFile);
		return (TRUE);
		}
	}

Bool LEMessageC::Store(Message *Msg)
	{
	return (MessageStoreC::Store(Msg));
	}

ReadMessageStatus LEMessageC::LoadAll(Message *Msg)
	{
	return (MessageStoreC::LoadAll(Msg));
	}


Bool LogExtensions::Save(l_slot Slot, const char *Name, r_slot MsgRoom)
	{
	assert(this);
	VerifyHeap();
	Bool Good = TRUE;

	GAINEXCLUSIVEACCESS();

#ifdef WINCIT
	SyncNumber = Slot;
#endif

	char lbuf[128], wow[128], wow2[256];
	FILE *fd;
	strList *theStr;
	pairedStrings *thePair;

	sprintf(lbuf, getmsg(224), cfg.logextdir, Slot, FileExtension);

	if ((fd = fopen(lbuf, FO_W)) != NULL)
		{
		if (Name)
			{
			fprintf(fd, getmsg(682), Name, bn, bn, ns);
			}

		Bool Written = FALSE;
		int i;

		for (i = 0; i < KF_NUM; i++)
			{
			int Keyword = (i == KF_USER) ? LE_KUSER :
					(i == KF_TEXT) ? LE_KTEXT :
					(i == KF_NODE) ? LE_KNODE :
					(i == KF_REGION) ? LE_KREG : MAXLOGEXT;
			assert(Keyword != MAXLOGEXT);

			for (theStr = Kill[i]; theStr; theStr = (strList *) getNextLL(theStr))
				{
				if (fprintf(fd, getmsg(223), logextensions[Keyword],
						mkDblBck(theStr->string, wow), bn) == EOF)
					{
					Good = FALSE;
					}

				Written = TRUE;
				}
			}

		for (thePair = le_tuser; thePair; thePair = (pairedStrings *) getNextLL(thePair))
			{
			if (fprintf(fd, getmsg(222), logextensions[LE_TUSER], mkDblBck(thePair->string1, wow),
					mkDblBck(thePair->string2, wow2), bn) == EOF)
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		for (thePair = le_replace; thePair; thePair = (pairedStrings *) getNextLL(thePair))
			{
			if (fprintf(fd, getmsg(222), logextensions[LE_REPLACE], mkDblBck(thePair->string1, wow),
					mkDblBck(thePair->string2, wow2), bn) == EOF)
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		for (theStr = le_dict; theStr;
				theStr = (strList *) getNextLL(theStr))
			{
			if (fprintf(fd, getmsg(223), logextensions[LE_DICTWORD],
					mkDblBck(theStr->string, wow), bn) == EOF)
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		for (userDefLE *theDef = le_userdef; theDef;
				theDef = (userDefLE *) getNextLL(theDef))
			{
			if (fprintf(fd, getmsg(222), logextensions[LE_USERDEF],
					mkDblBck(theDef->Code, wow),
					mkDblBck(theDef->Data, wow2), bn) == EOF)
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		if (cfg.SaveJB)
			{
			for (i = (jb_start + 1) % cfg.maxjumpback;
					i != (jb_end + 1) % cfg.maxjumpback;
					i = (i + 1) % cfg.maxjumpback)
				{
				if (fprintf(fd, getmsg(221),
						logextensions[LE_JUMPBACK], jb[i].newpointer,
						(ulong) jb[i].newMsgs, jb[i].hall, jb[i].room,
						jb[i].bypass, bn) == EOF)
					{
					Good = FALSE;
					}

				Written = TRUE;
				}
			}

		if (Msg)
			{
			if (fprintf(fd, getmsg(218), logextensions[LE_MESSAGE], MsgRoom, bn) == EOF)
				{
				Good = FALSE;
				}

			LEMessageC LEMessage(fd);

			LEMessage.Store(Msg);

			if (fprintf(fd, bn) == EOF)
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		if (le_Finger)
			{
			if ((fprintf(fd, getmsg(217), logextensions[LE_FINGER], bn) == EOF) ||
					(fwrite(le_Finger, sizeof(char), strlen(le_Finger) + 1, fd) != strlen(le_Finger) +1) ||
					(fprintf(fd, bn) == EOF))
				{
				Good = FALSE;
				}

			Written = TRUE;
			}

		fclose(fd);

		if (!Written)
			{
			unlink(lbuf);
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	RELEASEEXCLUSIVEACCESS();
	return (Good);
	}

Bool LogExtensions::Load(l_slot Slot)
	{
	assert(this);

#ifdef WINCIT
	SyncNumber = Slot;
	LogExtensions *Other = (LogExtensions *) Sync.FindMatching(this);

	if (Other)
		{
		*this = *Other;
		return (TRUE);
		}
#endif

	Bool Good = TRUE;
	VerifyHeap();

	GAINEXCLUSIVEACCESS();

	char lbuf[128], line[384];
	char *words[256];
	int i;
	FILE *fd;

	Clear();

#ifdef WINCIT
	SyncNumber = Slot;
#endif

	compactMemory();

	sprintf(lbuf, getmsg(224), cfg.logextdir, Slot, FileExtension);

	if ((fd = fopen(lbuf, FO_R)) != NULL)
		{
		while (fgets(line, sizeof(line), fd) != NULL)
			{
			if (line[0] != '#')
				{
				continue;
				}

			parse_it(words, line);

			for (i = 0; i < MAXLOGEXT; i++)
				{
				if (SameString(words[0] + 1, logextensions[i]))
					{
					break;
					}
				}

			switch (i)
				{
				case LE_MESSAGE:
					{
					Msg = new Message;

					if (Msg)
						{
						LEMessageC LEMessage(fd);

						LEMessage.LoadAll(Msg);

						MsgRoom = (r_slot) atoi(words[1]);
						}
					else
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_KUSER:
					{
					if (!AddKill(KF_USER, words[1]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_KNODE:
					{
					if (!AddKill(KF_NODE, words[1]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_KTEXT:
					{
					if (!AddKill(KF_TEXT, words[1]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_KREG:
					{
					if (!AddKill(KF_REGION, words[1]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_TUSER:
					{
					if (!AddTagUser(words[1], words[2]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_REPLACE:
					{
					if (!AddReplace(words[1], words[2]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_DICTWORD:
					{
					if (!AddWordToDictionary(words[1]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_USERDEF:
					{
					if (!SetUserDefined(words[1], words[2]))
						{
						OutOfMemory(72);
						}

					break;
					}

				case LE_JUMPBACK:
					{
					if (jb_length)
						{
						jb_end = ++jb_end % jb_length;

						if (jb_end == jb_start)
							{
							jb_start++;
							}

						jb[jb_end].newpointer = atol(words[1]);
						jb[jb_end].newMsgs = (m_slot) atol(words[2]);
						jb[jb_end].hall = (h_slot) atoi(words[3]);
						jb[jb_end].room = (r_slot) atoi(words[4]);
						jb[jb_end].bypass = atoi(words[5]);
						}

					break;
					}

				case LE_FINGER:
					{
					char *Buffer = new char[MAXTEXT];

					if (Buffer)
						{
						if (GetStr(fd, Buffer, MAXTEXT - 1))
							{
							SetFinger(Buffer);
							}

						delete [] Buffer;
						}
					else
						{
						OutOfMemory(72);
						}

					break;
					}

				default:
					{
#ifndef WINCIT
					mPrintfCR(getmsg(345), words[0]);
#endif
					break;
					}
				}
			}

		fclose(fd);
		}

	VerifyHeap();
	RELEASEEXCLUSIVEACCESS();
	return (Good);
	}
