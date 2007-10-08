// --------------------------------------------------------------------------
// Citadel: Log.CPP
//
// Local log code

#include "ctdl.h"
#pragma hdrstop

#include "net.h"
#include "log.h"
#include "room.h"

// --------------------------------------------------------------------------
// Contents
//
// FindPersonByName()	returns slot# of named person else CERROR
// storeLog()			stores the current log record.
// normalizepw()		This breaks down inits;pw into separate strings


LogTable			LogTab; 		// RAM index of pippuls

#ifdef WINCIT
// these have to go somewhere...
HANDLE LTable::Mutex;
int LTable::UsageCounter;
LTable *LogTable::Table;
#endif


// --------------------------------------------------------------------------
// FindPersonByName(): Returns slot# of named person else CERROR.

#ifdef WINCIT
l_slot TermWindowC::FindPersonByName(const char *name)
	{
	return (::FindPersonByName(name, LogOrder));
	}
#endif

l_slot FindPersonByName(const char *name, const l_slot *Order)
	{
	label alias;
	CopyStringToBuffer(alias, name);
	char *ptr = strchr(alias, '.');
	if (ptr)
		{
		*ptr = 0;	// tie off
		}

	// check to see if name is in log table
	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		l_slot S = Order ? Order[i] : i;

		if (LogTab[S].IsInuse() &&
				(
				LogTab[S].IsSameName(name) || LogTab[S].IsSameAlias(alias)
				)
			)
			{
			return (i);
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// nodexists(): Returns slot# of named person else CERROR.

#ifdef WINCIT
l_slot TermWindowC::nodexists(const char *name)
	{
	return (::nodexists(name, LogOrder));
	}
#endif

l_slot nodexists(const char *name, const l_slot *Order)
	{
	char alias[4];
	char locID[5];
	Bool addrsearch = FALSE;

	if (isaddress(name))
		{
		parse_address(name, alias, locID, 0);
		addrsearch = TRUE;
		}

	// check to see if name is in log table
	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		l_slot S = Order ? Order[i] : i;

		if (LogTab[S].IsInuse() && LogTab[S].IsNode())
			{
			if (addrsearch)
				{
				if (	// same alias and...
						LogTab[S].IsSameAlias(alias) &&

						// either...
						(
							// not searching locIDs or...
							!*locID || 

							// it matches, too
							LogTab[S].IsSameLocID(locID)
						)
					)
					{
					return (i);
					}
				}
			else
				{
				if (LogTab[S].IsSameName(name))
					{
					return (i);
					}
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// addressexists(): Returns slot# of named person else CERROR.

#ifdef WINCIT
l_slot TermWindowC::addressexists(const char *name)
	{
	return (::addressexists(name, LogOrder));
	}
#endif

l_slot addressexists(const char *name, const l_slot *Order)
	{
	char alias[4];
	char locID[5];

	parse_address(name, alias, locID, 0);

	// check to see if name is in log table
	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		l_slot S = Order ? Order[i] : i;

		if (LogTab[S].IsInuse())
			{
			if (LogTab[S].IsSameAlias(alias) && LogTab[S].IsSameLocID(locID))
				{
				return (i);
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// FindPersonByPartialName(): Returns slot # of partial user name, else CERROR

#ifdef WINCIT
l_slot TermWindowC::FindPersonByPartialName(const char *name)
	{
	return (::FindPersonByPartialName(name, LogOrder));
	}
#endif

l_slot FindPersonByPartialName(const char *name, const l_slot *Order)
	{
	label da_pn;

	if (strlen(name) > LABELSIZE)
		{
		return (CERROR);
		}

	// Exact match
	l_slot i = FindPersonByName(name, Order);
	if (i != CERROR)
		{
		return (i);
		}

	strcpy(da_pn, deansi(name));
	const int length = strlen(da_pn);

	if (!length)
		{
		return (CERROR);
		}

	// Start of string match
	for (i = 0; i < cfg.MAXLOGTAB; i++)
		{
		l_slot S = Order ? Order[i] : i;

		if (LogTab[S].IsInuse())
			{
			label Buffer;
			if (strnicmp(deansi(LogTab[S].GetName(Buffer, sizeof(Buffer))), da_pn, length) == SAMESTRING)
				{
				return (i);
				}
			}
		}

	// Partial match
	for (i = 0; i < cfg.MAXLOGTAB; i++)
		{
		l_slot S = Order ? Order[i] : i;

		if (LogTab[S].IsInuse())
			{
			label Buffer;
			if (IsSubstr(LogTab[S].GetName(Buffer, sizeof(Buffer)), da_pn))
				{
				return (i);
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// normalizepw(): This breaks down inits;pw into separate strings.

void normalizepw(const char *InitPw, char *Initials, char *passWord)
	{
	const char *inpwptr = InitPw;
	char *pwptr = passWord;
	char *inptr = Initials;

	while (*inpwptr && *inpwptr != ';')
		{
		*inptr++ = *inpwptr;
		inpwptr++;
		}
	*inptr = 0; 	// tie off with a null
	normalizeString(Initials);

	if (*inpwptr == ';')
		{
		inpwptr++;			// step over semicolon
	
		while (*inpwptr != '\0')
			{
			*pwptr++ = *inpwptr;
			inpwptr++;
			}
		*pwptr = 0; 	// tie off with a null
		normalizeString(passWord);
		}
	else
		{
		*passWord = 0;
		}

	// don't allow anything over LABELSIZE characters
	Initials[LABELSIZE] = 0;
	passWord[LABELSIZE] = 0;
	}


// --------------------------------------------------------------------------
// LogTab::Update(): Logbuf to logtable converter.

void LogTable::Update(l_slot TabSlot, const LogEntry1 *Log, l_index FileIndex)
	{
	assert(this);
	assert(TabSlot >= 0);
	assert(TabSlot < TableSize);
	assert(IsValid());

	if (IsValid())
		{
		Table[TabSlot].CopyFlags(*Log);

		if (Log->IsInuse())
			{
			label Buffer;
			Table[TabSlot].SetInitialsHash(Log->GetInitials(Buffer,
					sizeof(Buffer)));
			Table[TabSlot].SetPasswordHash(Log->GetPassword(Buffer,
					sizeof(Buffer)));
			Table[TabSlot].SetName(Log->GetName(Buffer, sizeof(Buffer)));
			Table[TabSlot].SetAlias(Log->GetAlias(Buffer, sizeof(Buffer)));
			Table[TabSlot].SetLocID(Log->GetLocID(Buffer, sizeof(Buffer)));
			Table[TabSlot].SetCallNumber(Log->GetCallNumber());
			}
		else
			{
			Table[TabSlot].SetInitialsHash((short) 0);
			Table[TabSlot].SetPasswordHash((short) 0);
			Table[TabSlot].SetName(ns);
			Table[TabSlot].SetAlias(ns);
			Table[TabSlot].SetLocID(ns);
			Table[TabSlot].SetCallNumber(0);
			}

		Table[TabSlot].SetLogIndex(FileIndex);
		}
	}

Bool TERMWINDOWMEMBER LoadPersonByName(const char *name)
	{
	const l_slot NewPerson = FindPersonByName(name);

	if (NewPerson == CERROR)
		{
		return (FALSE);
		}
	else
		{
		ThisSlot = NewPerson;
		ThisLog = LTab(NewPerson).GetLogIndex();

		return (CurrentUser->Load(ThisLog));
		}
	}

#ifdef WINCIT
Bool LogEntry::LoadByName(const char *name, l_slot *LogOrder)
#else
Bool LogEntry::LoadByName(const char *name)
#endif
	{
	l_slot NewPerson;

#ifdef WINCIT
	NewPerson = FindPersonByName(name, LogOrder);
#else
	NewPerson = FindPersonByName(name);
#endif

	if (NewPerson == CERROR)
		{
		return (FALSE);
		}
	else
		{
		return (Load(LTab(NewPerson).GetLogIndex()));
		}
	}


// --------------------------------------------------------------------------
// storeLog(): Stores the current log record.

void TERMWINDOWMEMBER storeLog(void)
	{
	if (loggedIn)
		{
		CurrentUser->Save(ThisLog, thisRoom);
		}
	}


// --------------------------------------------------------------------------
// LogEntry::CanModerateRoom(): Returns whether the user can moderate a room or not.
//
// Input:		RoomIndex	Room's index in the database that we want to know about.
//
// Output:		return		TRUE if the user can moderate it.
//							FALSE if he cannot.

Bool LogEntry::CanModerateRoom(r_slot RoomIndex) const
	{
	assert(RoomIndex < cfg.maxrooms);
	assert(RoomIndex >= 0);
	assert(RoomTab[RoomIndex].IsInuse());

	return
		(
		IsSysop() ||

		(IsAide() && cfg.moderate) ||

		(RoomTab[RoomIndex].IsGroupModerates() && HasRoomPrivileges(RoomIndex))
		);
	}

Bool LogEntry::CanKillMessage(Message *Msg, Bool IsLoggedIn) const
	{
    return (CanModerateRoom(Msg->GetOriginalRoom()) || (IsLoggedIn && IsSameName(Msg->GetAuthor()) && IsKillOwn()));
	}

Bool LogEntry::ShowSystemEvent(SystemEventE Event)
	{
	switch (Event)
		{
		case SE_LOGONOFF:			return (ShowSELogOnOff());
		case SE_NEWMESSAGE:			return (ShowSENewMessage());
		case SE_EXCLUSIVEMESSAGE:	return (ShowSEExclusiveMessage());
		case SE_CHATALL:			return (ShowSEChatAll());
		case SE_CHATUSER:		return (ShowSEChatUser());
		case SE_CHATGROUP:			return (ShowSEChatGroup());
		case SE_CHATROOM:			return (ShowSEChatRoom());
		case SE_ROOMINOUT:			return (ShowSERoomInOut());
		}

	return (TRUE);
	}


