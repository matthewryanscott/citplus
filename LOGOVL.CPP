// access to log stuff

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "room.h"
#include "group.h"

LogEntry::LogEntry(r_slot NumRooms, g_slot NumGroups, int MaxJumpback) :
	LogEntry2(NumGroups),
	LogEntry3(NumRooms),
	LogEntry4(NumRooms),
	LogEntry5(NumRooms),
	LogEntry6(NumRooms),
	LogExtensions(MaxJumpback)
	{
	}

#ifdef WINCIT
void LogEntry::PauseSyncSend(void)
	{
	LogEntry1::PauseSyncSend();
	LogEntry2::PauseSyncSend();
	LogEntry3::PauseSyncSend();
	LogEntry4::PauseSyncSend();
	LogEntry5::PauseSyncSend();
	LogEntry6::PauseSyncSend();
	LogExtensions::PauseSyncSend();
	}

void LogEntry::ResumeSyncSend(void)
	{
	LogEntry1::ResumeSyncSend();
	LogEntry2::ResumeSyncSend();
	LogEntry3::ResumeSyncSend();
	LogEntry4::ResumeSyncSend();
	LogEntry5::ResumeSyncSend();
	LogEntry6::ResumeSyncSend();
	LogExtensions::ResumeSyncSend();
	}

void LogEntry1Data::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	GAINEXCLUSIVEACCESS();
	char Buffer[256];
	
	CopyStringToBuffer(Buffer, Name);
	SetName(Buffer);

	CopyStringToBuffer(Buffer, Initials);
	SetInitials(Buffer);

	CopyStringToBuffer(Buffer, Password);
	SetPassword(Buffer);

	CopyStringToBuffer(Buffer, Surname);
	SetSurname(Buffer);

	CopyStringToBuffer(Buffer, Title);
	SetTitle(Buffer);

	CopyStringToBuffer(Buffer, RealName);
	SetRealName(Buffer);

	CopyStringToBuffer(Buffer, PhoneNumber);
	SetPhoneNumber(Buffer);

	CopyStringToBuffer(Buffer, ForwardAddr);
	SetForwardAddr(Buffer);

	CopyStringToBuffer(Buffer, ForwardAddrNode);
	SetForwardAddrNode(Buffer);

	CopyStringToBuffer(Buffer, ForwardAddrRegion);
	SetForwardAddrRegion(Buffer);

	CopyStringToBuffer(Buffer, PromptFormat);
	SetPromptFormat(Buffer);

	CopyStringToBuffer(Buffer, DateStamp);
	SetDateStamp(Buffer);

	CopyStringToBuffer(Buffer, VerboseDateStamp);
	SetVerboseDateStamp(Buffer);

	CopyStringToBuffer(Buffer, Signature);
	SetSignature(Buffer);

	CopyStringToBuffer(Buffer, NetPrefix);
	SetNetPrefix(Buffer);

	CopyStringToBuffer(Buffer, MailAddr1);
	SetMailAddr1(Buffer);

	CopyStringToBuffer(Buffer, MailAddr2);
	SetMailAddr2(Buffer);

	CopyStringToBuffer(Buffer, MailAddr3);
	SetMailAddr3(Buffer);

	CopyStringToBuffer(Buffer, Alias);
	SetAlias(Buffer);

	CopyStringToBuffer(Buffer, LocID);
	SetLocID(Buffer);

	CopyStringToBuffer(Buffer, MorePrompt);
	SetMorePrompt(Buffer);

	CopyStringToBuffer(Buffer, Occupation);
	SetOccupation(Buffer);

	CopyStringToBuffer(Buffer, WhereHear);
	SetWhereHear(Buffer);

	CopyStringToBuffer(Buffer, LastRoom);
	SetLastRoom(Buffer);

	CopyStringToBuffer(Buffer, LastHall);
	SetLastHall(Buffer);

	CopyStringToBuffer(Buffer, DefaultRoom);
	SetDefaultRoom(Buffer);

	CopyStringToBuffer(Buffer, DefaultHall);
	SetDefaultHall(Buffer);

	CopyStringToBuffer(Buffer, TermType);
	SetTermType(Buffer);
	RELEASEEXCLUSIVEACCESS();

	SetBirthDate(BirthDate);
	SetFirstOn(FirstOn);
	SetSex(Sex);
	SetNulls(Nulls);
	SetWidth(Width);
	SetLinesPerScreen(LinesPerScreen);
	SetAttribute(ATTR_NORMAL, attributes[ATTR_NORMAL]);
	SetAttribute(ATTR_BLINK, attributes[ATTR_BLINK]);
	SetAttribute(ATTR_REVERSE, attributes[ATTR_REVERSE]);
	SetAttribute(ATTR_BOLD, attributes[ATTR_BOLD]);
	SetAttribute(ATTR_UNDERLINE, attributes[ATTR_UNDERLINE]);
	SetNumUserShow(NumUserShow);
	SetDefaultProtocol(DefaultProtocol);
	SetCallTime(CallTime);
	SetCallNumber(CallNumber);
	SetTotalTime(TotalTime);
	SetCredits(Credits);
	SetLogins(Logins);
	SetPosted(Posted);
	SetRead(Read);
	SetPasswordChangeTime(PasswordChangeTime);
	SetCallsToday(CallsToday);
	SetCallLimit(CallLimit);
	SetLastMessage(LastMessage);
	SetDL_Bytes(DL_Bytes);
	SetUL_Bytes(UL_Bytes);
	SetDL_Num(DL_Num);
	SetUL_Num(UL_Num);
	SetPoopcount(Poopcount);

	SetDungeoned(IsDungeoned());
	SetForwardToNode(IsForwardToNode());
	SetAutoNextHall(IsAutoNextHall());
	SetEnterBorders(IsEnterBorders());
	SetVerified(IsVerified());
	SetSurnameLocked(IsSurnameLocked());
	SetDefaultHallLocked(IsDefaultHallLocked());
	SetPsycho(IsPsycho());
	SetViewTitleSurname(IsViewTitleSurname());
	SetViewSubjects(IsViewSubjects());
	SetViewSignatures(IsViewSignatures());
	SetOldIBMGraph(IsOldIBMGraph());
	SetOldIBMANSI(IsOldIBMANSI());
	SetOldIBMColor(IsOldIBMColor());
	SetTwirly(IsTwirly());
	SetAutoVerbose(IsAutoVerbose());
	SetPauseBetweenMessages(IsPauseBetweenMessages());
	SetMinibin(IsMinibin());
	SetClearScreenBetweenMessages(IsClearScreenBetweenMessages());
	SetViewRoomInfoLines(IsViewRoomInfoLines());
	SetViewHallDescription(IsViewHallDescription());
	SetVerboseContinue(IsVerboseContinue());
	SetViewCensoredMessages(IsViewCensoredMessages());
	SetViewBorders(IsViewBorders());
	SetOut300(IsOut300());
	SetUserSignatureLocked(IsUserSignatureLocked());
	SetHideMessageExclusions(IsHideMessageExclusions());
	SetDownload(IsDownload());
	SetUpload(IsUpload());
	SetChat(IsChat());
	SetPrintFile(IsPrintFile());
	SetSpellCheckMode(GetSpellCheckMode());
	SetMakeRoom(IsMakeRoom());
	SetVerboseLogOut(IsVerboseLogOut());
	SetConfirmSave(IsConfirmSave());
	SetConfirmAbort(IsConfirmAbort());
	SetConfirmNoEO(IsConfirmNoEO());
	SetUsePersonalHall(IsUsePersonalHall());
	SetYouAreHere(IsYouAreHere());
	SetIBMRoom(IsIBMRoom());
	SetWideRoom(IsWideRoom());
	SetMusic(IsMusic());
	SetCheckApostropheS(IsCheckApostropheS());
	SetCheckAllCaps(IsCheckAllCaps());
	SetCheckDigits(IsCheckDigits());
	SetExcludeEncryptedMessages(IsExcludeEncryptedMessages());
	SetViewCommas(IsViewCommas());
	SetPUnPauses(IsPUnPauses());
	SetRoman(IsRoman());
	SetSuperSysop(IsSuperSysop());

	SetInuse(IsInuse());
	SetUpperOnly(IsUpperOnly());
	SetLinefeeds(IsLinefeeds());
	SetExpert(IsExpert());
	SetAide(IsAide());
	SetTabs(IsTabs());
	SetOldToo(IsOldToo());
	SetProblem(IsProblem());
	SetUnlisted(IsUnlisted());
	SetPermanent(IsPermanent());
	SetSysop(IsSysop());
	SetNode(IsNode());
	SetNetUser(IsNetUser());
	SetAccounting(IsAccounting());
	SetMail(IsMail());
	SetViewRoomDesc(IsViewRoomDesc());
	}

void LogEntry2::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	for (g_slot G = 0; G < (g_slot) GetNumberOfBits(); G++)
		{
		SetInGroup(G, IsInGroup(G));
		}
	}

void LogEntry3::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	for (r_slot R = 0; R < (r_slot) GetNumberOfBits(); R++)
		{
		SetInRoom(R, IsInRoom(R));
		}
	}

void LogEntry4::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	for (r_slot R = 0; R < (r_slot) GetNumberOfBits(); R++)
		{
		SetRoomExcluded(R, IsRoomExcluded(R));
		}
	}

void LogEntry5::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	for (r_slot R = 0; R < GetMaxRooms(); R++)
		{
		SetRoomNewPointer(R, GetRoomNewPointer(R));
		}
	}

void LogEntry6::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody else all about ourselves.
	for (r_slot R = 0; R < (r_slot) GetNumberOfBits(); R++)
		{
		SetRoomInPersonalHall(R, IsRoomInPersonalHall(R));
		}
	}


// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// I'll probably want to do this one for real some time... :)
void LogExtensions::ResumeSyncSend(void)
	{
	if (!SyncSendPaused)
		{
		return;
		}

	SyncSendPaused = FALSE;

	if (SyncNumber == CERROR)
		{
		return;
		}

	// We just unpaused and we're assigned a record number... tell everybody
	// else all about ourselves.
	}

#endif

void LogEntry1Data::Verify(void)
	{
	// make sure no strings overflow
	Name[LABELSIZE] 			= 0;
	Initials[LABELSIZE] 		= 0;
	Password[LABELSIZE] 		= 0;
	Surname[LABELSIZE]			= 0;
	Title[LABELSIZE]			= 0;
	RealName[LABELSIZE] 		= 0;
	PhoneNumber[LABELSIZE]		= 0;

	ForwardAddr[LABELSIZE]		= 0;
	ForwardAddrNode[LABELSIZE]	= 0;
	ForwardAddrRegion[LABELSIZE]= 0;

	PromptFormat[63]			= 0;
	DateStamp[63]				= 0;
	VerboseDateStamp[63]		= 0;
	Signature[90]				= 0;
	NetPrefix[LABELSIZE]		= 0;
	MailAddr1[LABELSIZE]		= 0;
	MailAddr2[LABELSIZE]		= 0;
	MailAddr3[LABELSIZE]		= 0;
	Alias[3]					= 0;
	LocID[4]					= 0;
	MorePrompt[79]				= 0;
	Occupation[LABELSIZE]		= 0;
	WhereHear[79]				= 0;

	LastRoom[LABELSIZE] 		= 0;
	LastHall[LABELSIZE] 		= 0;
	DefaultRoom[LABELSIZE]		= 0;
	DefaultHall[LABELSIZE]		= 0;

	if (Sex > SEX_FEMALE || Sex < SEX_UNKNOWN)
		{
		Sex = SEX_UNKNOWN;
		}
	}

LogEntry::LogEntry(const LogEntry &Original) :
	LogEntry1(Original),
	LogEntry2(Original),
	LogEntry3(Original),
	LogEntry4(Original),
	LogEntry5(Original),
	LogEntry6(Original),
	LogExtensions(Original)
	{

	}

LogEntry& LogEntry::operator = (const LogEntry &Original)
	{
	LogEntry1::operator =(Original);
	LogEntry2::operator =(Original);
	LogEntry3::operator =(Original);
	LogEntry4::operator =(Original);
	LogEntry5::operator =(Original);
	LogEntry6::operator =(Original);
	LogExtensions::operator =(Original);

	return (*this);
	}

void LogFlags::CopyFlags(const LogFlags &ToCopy)
	{
	VerifyHeap();
	memcpy(this, &ToCopy, sizeof(LogFlags));
	VerifyHeap();
	}

// I really don't like this one. Copying the virual pointer and all.
LogEntry1& LogEntry1::operator =(const LogEntry1 &Original)
	{
	VerifyHeap();
	memcpy(this, &Original, sizeof(LogEntry1));
	VerifyHeap();

	return (*this);
	}


void LogEntry1Data::CopyLog1Data(const LogEntry1Data *Src)
	{
	VerifyHeap();
	SAVEEXCLUSIVEACCESSRECORD();
	memcpy(this, Src, sizeof(LogEntry1Data));
	RESTOREEXCLUSIVEACCESSRECORD();
	VerifyHeap();
	}

LogEntry1& LogEntry1::operator =(const LogEntry1Data &Original)
	{
	CopyLog1Data(&Original);
	return (*this);
	}

void LogEntry2::Clear(void)
	{
	VerifyHeap();

	if (IsValid())
		{
		LogBitBag::Clear();

		VerifyHeap();

		// Always put in NULL.
		SetInGroup(0, TRUE);

		// Then put in any other auto-add groups if we have groups.
		if (GroupData.IsValid())
			{
			for (g_slot GroupSlot = 1; GroupSlot < cfg.maxgroups; GroupSlot++)
				{
				SetInGroup(GroupSlot, GroupData[GroupSlot].IsAutoAdd());
				}
			}
		}
	}

void LogEntry3::Clear(void)
	{
	VerifyHeap();

	if (IsValid())
		{
		LogBitBag::Clear();

		VerifyHeap();

		for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
			{
			if (RoomTab[RoomSlot].IsInuse() && !RoomTab[RoomSlot].IsHidden() &&
					!RoomTab[RoomSlot].IsBIO())
				{
				// make public rooms known
				SetInRoom(RoomSlot, TRUE);
				}
			else
				{
				// make private rooms unknown
				SetInRoom(RoomSlot, FALSE);
				}
			}
		}
	}

void TERMWINDOWMEMBER ListUsers(Bool ListUnlisted, Bool ListOnlyLoggedIn)
	{
	CRmPrintfCR(getmsg(578), cfg.Uusers_nym);

	prtList(LIST_START);

	for (l_slot LogSlot = 0; LogSlot < cfg.MAXLOGTAB; LogSlot++)
		{
#ifdef MULTI
		if (// slot is in use
			LTab(LogSlot).IsInuse() &&

				(!ListOnlyLoggedIn && 
				// and listing the unlisted
				(ListUnlisted ||

				// The account is not unlisted
				!LTab(LogSlot).IsUnlisted() ||

				// Logged in and it is us (we are always 0).
				(loggedIn && !LogSlot)) ||

				// For .CU?
                (ListOnlyLoggedIn && LoginList.IsLoggedIn(LTab(LogSlot).GetLogIndex()) && !LTab(LogSlot).IsNode())
				)
			)
#else
		if (// slot is in use
			LTab(LogSlot).IsInuse() &&

				(!ListOnlyLoggedIn && 
				// and listing the unlisted
				(ListUnlisted ||

				// The account is not unlisted
				!LTab(LogSlot).IsUnlisted() ||

				// Logged in and it is us (we are always 0).
                (loggedIn && !LogSlot))
				)
			)
#endif

			{
			label Buffer;
			prtList(LTab(LogSlot).GetName(Buffer, sizeof(Buffer)));
			}
		}

	prtList(LIST_END);
	}
