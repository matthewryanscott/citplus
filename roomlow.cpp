// --------------------------------------------------------------------------
// Citadel: RoomLow.CPP
//
// Low-level parts of the room database subsystem.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#ifdef WINCIT
#include "net6969.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
//
// GetRoomPosInTable()	Gets a room's slot in the roompos table.
// CanUserSeeRoom() 	Checks to see if the current user has access to a
//							specified room.
// RoomExists() 		Checks to see if a room exists, given a name.
// getRoom()			load given room into RAM
// putRoom()			store room to given disk slot


RoomTable RoomTab;

#ifdef WINCIT
// these have to go somewhere...
HANDLE RTable::Mutex;
int RTable::UsageCounter;
#endif

// --------------------------------------------------------------------------
// GetRoomPosInTable(): Gets a room's slot in the roompos table.
//
//	Input:		RoomIndex	The room to find in the table.
//
//	Output: 	return		The position in the table of the room.

r_slot GetRoomPosInTable(r_slot RoomIndex)
	{
	assert(RoomIndex < cfg.maxrooms);
	assert(RoomIndex >= 0);
	assert(roomPos != NULL);

	for (r_slot i = 0; i < cfg.maxrooms; i++)
		{
		if (roomPos[i] == RoomIndex)
			{
			return (i);
			}
		}

	// crashing is a bit bogus. perhaps recreate the table?
	crashout(getmsg(503));

	return (0); 	// bogus return value to keep the compiler happy.
	}


// -------------------------------------------------------------------------
// groupseesroom(): Returns true if group can see room.

Bool LogEntry::HasGroupAccessToRoom(r_slot roomslot) const
	{
	assert(this);
	assert(roomslot >= 0);
	assert(roomslot < cfg.maxrooms);

	if (!RoomTab[roomslot].IsGroupOnly())
		{
		return (TRUE);
		}

	if (IsMainSysop())
		{
		return (TRUE);
		}

	if (!RoomTab[roomslot].IsBooleanGroup())
		{
		return (IsInGroup(RoomTab[roomslot].GetGroupNumber()));
		}
	else
		{
		L2FGT = this;

		BoolExpr Buffer;
		return (EvaluateBooleanExpression(RoomTab[roomslot].GetGroupExpression(Buffer), GroupTester));
		}
	}

// --------------------------------------------------------------------------
// CanUserSeeRoom(): Checks to see if the current user has access to a
//	specified room.
//
//	Input:		RoomIndex	The room that we are checking.
//				ThisHall	TRUE if the room must be in current hall.

Bool LogEntry::CanAccessRoom(r_slot RoomIndex, h_slot Hall, Bool IgnoreHidden) const
	{
	assert(this);
	assert(RoomIndex < cfg.maxrooms);
	assert(RoomIndex >= 0);

	return
		(
		// The room is in use
		RoomTab[RoomIndex].IsInuse() &&

			// Don't care about halls, or it is in the current one
			(Hall == CERROR || (IsUsePersonalHall() ? IsRoomInPersonalHall(RoomIndex) : roominhall(RoomIndex, Hall)))

		&&

			(
			// either the user is the #SYSOP
			IsMainSysop() ||

			// or...
				(
				// The user has group access to room
				HasGroupAccessToRoom(RoomIndex) &&

				// and either not the aide room, or the user is an Aide
				(RoomIndex != AIDEROOM || IsAide()) &&

				// and not bio, hidden, or kicked out, or it is hidden, but we are ignoring that
   				(IsInRoom(RoomIndex) || (IgnoreHidden && !RoomTab[RoomIndex].IsPublic()))
				)
			)

		&&

			// Special restrictions for nodes: it is not Aide), it is not Dump>, and it is shared
			(IsNode() ? (RoomIndex != AIDEROOM && RoomIndex != DUMP && RoomTab[RoomIndex].IsShared()) : TRUE));
	}


// --------------------------------------------------------------------------
// RoomExists(): Checks to see if a room exists, given a name.
//
//	Input:		RoomName	The room name to look for.
//
//	Output: 	return		Index into room database of the room if a match is found, else CERROR.

r_slot RoomExists(const char *RoomName)
	{
	static r_slot THREAD lastFoundRoom;

	if (*RoomName)
		{
		r_slot i, p;

		for (i = lastFoundRoom, p = 0; p < cfg.maxrooms; i = (r_slot) ((i + 1) % cfg.maxrooms), p++)
			{
			if (RoomTab[i].IsInuse() && RoomTab[i].IsSameName(RoomName))
				{
				lastFoundRoom = i;
				return (i);
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// IdExists(): Checks to see if a room exists, given a Network ID.
//
//	Input:		NetID		The Network ID to look for.
//				Network 	TRUE if the room has to be a shared room.
//
//	Output: 	return		Index into room database of the room if a match is found, else CERROR.

r_slot IdExists(const char *NetID, Bool Network)
	{
	if (*NetID)
		{
		for (r_slot i = 0; i < cfg.maxrooms; i++)
			{
			if (RoomTab[i].IsInuse() && (!Network || RoomTab[i].IsShared()) && RoomTab[i].IsSameNetID(NetID))
				{
				return (i);
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// getRoom()

Bool RoomC::Load(r_slot rm)
	{
	WAITFORm(RoomFileMutex);
	GAINEXCLUSIVEACCESS();

	const long s = (long) rm * (long) GetDiskRecordSize();

	fseek(RoomFile, s + sizeof(long), SEEK_SET);

#ifdef WINCIT
	SyncNumber = rm;

	RoomC *Other = (RoomC *) Sync.FindMatching(this);

	if (Other)
		{
		SetInuse(Other->IsInuse());
		SetPublic(Other->IsPublic());
		SetMsDOSdir(Other->IsMsDOSdir());
		SetPermanent(Other->IsPermanent());
		SetGroupOnly(Other->IsGroupOnly());
		SetReadOnly(Other->IsReadOnly());
		SetDownloadOnly(Other->IsDownloadOnly());
		SetShared(Other->IsShared());
		SetModerated(Other->IsModerated());
		SetApplication(Other->IsApplication());
		SetBIO(Other->IsBIO());
		SetUploadOnly (Other->IsUploadOnly());
		SetPrivilegedGroup(Other->IsPrivilegedGroup());
		SetAnonymous(Other->IsAnonymous());
		SetSubject(Other->IsSubject());
		SetGroupModerates(Other->IsGroupModerates());
		SetArchive(Other->IsArchive());
		SetBooleanGroup(Other->IsBooleanGroup());

		char Buffer[256];

		SetName(Other->GetName(Buffer, sizeof(Buffer)));
		SetDirectory(Other->GetDirectory(Buffer, sizeof(Buffer)));
		SetDescFile(Other->GetDescFile(Buffer, sizeof(Buffer)));
		SetApplication(Other->GetApplication(Buffer, sizeof(Buffer)));
		SetInfoLine(Other->GetInfoLine(Buffer, sizeof(Buffer)));
		SetNetID(Other->GetNetID(Buffer, sizeof(Buffer)));
		SetArchiveFile(Other->GetArchiveFile(Buffer, sizeof(Buffer)));
		SetDictionary(Other->GetDictionary(Buffer, sizeof(Buffer)));

		BoolExpr G;
		SetGroupExpression(Other->GetGroupExpression(G));

		SetGroup(Other->GrpNo);
		SetPGroup(Other->PGrpNo);
		SetAutoApp(Other->AUTOAPP);
		SetUnexcludable(Other->NOEXCLUDE);

		RELEASEEXCLUSIVEACCESS();
		RELEASEm(RoomFileMutex);
		return (TRUE);
		}

	// Read each element separately. This is needed because our memory is
	// 4-byte aligned, while the datafile (being based on the DOS verion's
	// memory) is 2-byte aligned.

	ushort Temp;
	int Bad = fread(&Flags, sizeof(Flags), 1, RoomFile) != 1;
	Bad += fread(Name, sizeof(Name), 1, RoomFile) != 1;
	Bad += fread(Directory, sizeof(Directory), 1, RoomFile) != 1;
	Bad += fread(DescFile, sizeof(DescFile), 1, RoomFile) != 1;
	Bad += fread(Applic, sizeof(Applic), 1, RoomFile) != 1;
	Bad += fread(&UNUSED1, sizeof(UNUSED1), 1, RoomFile) != 1;
	Bad += fread(&GrpNo, sizeof(GrpNo), 1, RoomFile) != 1;
	Bad += fread(&UNUSED2, sizeof(UNUSED2), 1, RoomFile) != 1;
	Bad += fread(&UNUSED3, sizeof(UNUSED3), 1, RoomFile) != 1;
	Bad += fread(&PGrpNo, sizeof(PGrpNo), 1, RoomFile) != 1;
	Bad += fread(&Temp, sizeof(Temp), 1, RoomFile) != 1;
	Bad += fread(Description, sizeof(Description), 1, RoomFile) != 1;
	Bad += fread(NetID, sizeof(NetID), 1, RoomFile) != 1;
	Bad += fread(Archive, sizeof(Archive), 1, RoomFile) != 1;
	Bad += fread(Dictionary, sizeof(Dictionary), 1, RoomFile) != 1;
	Bad += fread(ModHost, sizeof(ModHost), 1, RoomFile) != 1;
	Bad += fread(ExclNetPartner, sizeof(ExclNetPartner), 1, RoomFile) != 1;
	Bad += fread(&UNUSED5, sizeof(UNUSED5), 1, RoomFile) != 1;
	Bad += fread(GroupExpression, sizeof(GroupExpression), 1, RoomFile) != 1;
	Bad += fread(Creator, sizeof(Creator), 1, RoomFile) != 1;
	Bad += fread(&UNUSED6, sizeof(UNUSED6), 1, RoomFile) != 1;

	if (!Bad)
		{
		AUTOAPP = Temp & 1;
		NOEXCLUDE = (Temp >> 1) & 1;
		UNUSED4 = (Temp >> 2) & 16383;
		}
	else 
#else
	if (fread(this, sizeof(*this), 1, RoomFile) != 1)
#endif
		{
		crashout(getmsg(83), roomDat);
		}

	RELEASEEXCLUSIVEACCESS();
	RELEASEm(RoomFileMutex);
	return (TRUE);
	}


// --------------------------------------------------------------------------
// putRoom(): Stores room in buf into slot rm in ROOM.DAT.

Bool RoomC::Save(r_slot rm)
	{
	WAITFORm(RoomFileMutex);
	GAINEXCLUSIVEACCESS();

	assert(strlen(Name) <= LABELSIZE);
	assert(strlen(NetID) <= LABELSIZE);

	const long s = (long) rm * (long) GetDiskRecordSize();

	fseek(RoomFile, s + sizeof(long), SEEK_SET);

#ifdef WINCIT
	SyncNumber = rm;

	// Write each element out separately. This is needed because our
	// memory is 4-byte aligned, while the datafile (being based on the
	// DOS verion's memory) is 2-byte aligned.

	ushort Temp = (ushort) (AUTOAPP | (NOEXCLUDE << 1) | (UNUSED4 << 2));
	int Bad = fwrite(&Flags, sizeof(Flags), 1, RoomFile) != 1;
	Bad += fwrite(Name, sizeof(Name), 1, RoomFile) != 1;
	Bad += fwrite(Directory, sizeof(Directory), 1, RoomFile) != 1;
	Bad += fwrite(DescFile, sizeof(DescFile), 1, RoomFile) != 1;
	Bad += fwrite(Applic, sizeof(Applic), 1, RoomFile) != 1;
	Bad += fwrite(&UNUSED1, sizeof(UNUSED1), 1, RoomFile) != 1;
	Bad += fwrite(&GrpNo, sizeof(GrpNo), 1, RoomFile) != 1;
	Bad += fwrite(&UNUSED2, sizeof(UNUSED2), 1, RoomFile) != 1;
	Bad += fwrite(&UNUSED3, sizeof(UNUSED3), 1, RoomFile) != 1;
	Bad += fwrite(&PGrpNo, sizeof(PGrpNo), 1, RoomFile) != 1;
	Bad += fwrite(&Temp, sizeof(Temp), 1, RoomFile) != 1;
	Bad += fwrite(Description, sizeof(Description), 1, RoomFile) != 1;
	Bad += fwrite(NetID, sizeof(NetID), 1, RoomFile) != 1;
	Bad += fwrite(Archive, sizeof(Archive), 1, RoomFile) != 1;
	Bad += fwrite(Dictionary, sizeof(Dictionary), 1, RoomFile) != 1;
	Bad += fwrite(ModHost, sizeof(ModHost), 1, RoomFile) != 1;
	Bad += fwrite(ExclNetPartner, sizeof(ExclNetPartner), 1, RoomFile) != 1;
	Bad += fwrite(&UNUSED5, sizeof(UNUSED5), 1, RoomFile) != 1;
	Bad += fwrite(GroupExpression, sizeof(GroupExpression), 1, RoomFile) != 1;
	Bad += fwrite(Creator, sizeof(Creator), 1, RoomFile) != 1;
	Bad += fwrite(&UNUSED6, sizeof(UNUSED6), 1, RoomFile) != 1;

	if (Bad)
#else
	if (fwrite(this, sizeof(*this), 1, RoomFile) != 1)
#endif
		{
		crashout(getmsg(661), roomDat);
		}

	fflush(RoomFile);

	// enter room into RAM index array.
	if (Flags.IsInuse())
		{
		RoomTab[rm].SetInuse(TRUE);
		RoomTab[rm].SetPublic(Flags.IsPublic());
		RoomTab[rm].SetMsDOSdir(Flags.IsMsDOSdir());
		RoomTab[rm].SetPermanent(Flags.IsPermanent());
		RoomTab[rm].SetGroupOnly(Flags.IsGroupOnly());
		RoomTab[rm].SetReadOnly(Flags.IsReadOnly());
		RoomTab[rm].SetDownloadOnly(Flags.IsDownloadOnly());
		RoomTab[rm].SetShared(Flags.IsShared());
		RoomTab[rm].SetModerated(Flags.IsModerated());
		RoomTab[rm].SetApplication(Flags.IsApplication());
		RoomTab[rm].SetBIO(Flags.IsBIO());
		RoomTab[rm].SetUploadOnly(Flags.IsUploadOnly());
		RoomTab[rm].SetPrivilegedGroup(Flags.IsPrivilegedGroup());
		RoomTab[rm].SetAnonymous(Flags.IsAnonymous());
		RoomTab[rm].SetSubject(Flags.IsSubject());
		RoomTab[rm].SetGroupModerates(Flags.IsGroupModerates());
		RoomTab[rm].SetArchive(Flags.IsArchive());
		RoomTab[rm].SetBooleanGroup(Flags.IsBooleanGroup());

		RoomTab[rm].SetName(Name);
		RoomTab[rm].SetNetID(NetID);
		RoomTab[rm].SetGroupExpression(GroupExpression);

		RoomTab[rm].SetGroupNumber(GrpNo);
		RoomTab[rm].SetPGroupNumber(PGrpNo);

		}
	else
		{
		RoomTab[rm].Clear();
		}

#ifdef WINCIT
	// Net6969C::Propogate(RoomC *) checks for shared & NetID
	Net6969.Propogate(rm, this);
#endif

	RELEASEEXCLUSIVEACCESS();
	RELEASEm(RoomFileMutex);
	return (TRUE);
	}


#ifdef WINCIT
void RoomC::ResumeSyncSend(void)
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
	SetInuse(Flags.IsInuse());
	SetPublic(Flags.IsPublic());
	SetMsDOSdir(Flags.IsMsDOSdir());
	SetPermanent(Flags.IsPermanent());
	SetGroupOnly(Flags.IsGroupOnly());
	SetReadOnly(Flags.IsReadOnly());
	SetDownloadOnly(Flags.IsDownloadOnly());
	SetShared(Flags.IsShared());
	SetModerated(Flags.IsModerated());
	SetApplication(Flags.IsApplication());
	SetBIO(Flags.IsBIO());
	SetUploadOnly (Flags.IsUploadOnly());
	SetPrivilegedGroup(Flags.IsPrivilegedGroup());
	SetAnonymous(Flags.IsAnonymous());
	SetSubject(Flags.IsSubject());
	SetGroupModerates(Flags.IsGroupModerates());
	SetArchive(Flags.IsArchive());
	SetBooleanGroup(Flags.IsBooleanGroup());

	GAINEXCLUSIVEACCESS();

	char Buffer[256];

	CopyStringToBuffer(Buffer, Name);
	SetName(Buffer);

	CopyStringToBuffer(Buffer, Directory);
	SetDirectory(Buffer);

	CopyStringToBuffer(Buffer, DescFile);
	SetDescFile(Buffer);

	CopyStringToBuffer(Buffer, Applic);
	SetApplication(Buffer);

	CopyStringToBuffer(Buffer, Description);
	SetInfoLine(Buffer);

	CopyStringToBuffer(Buffer, NetID);
	SetNetID(Buffer);

	CopyStringToBuffer(Buffer, Archive);
	SetArchiveFile(Buffer);

	CopyStringToBuffer(Buffer, Dictionary);
	SetDictionary(Buffer);

	RELEASEEXCLUSIVEACCESS();

	BoolExpr G;
	SetGroupExpression(GetGroupExpression(G));

	SetGroup(GrpNo);
	SetPGroup(PGrpNo);
	SetAutoApp(AUTOAPP);
	SetUnexcludable(NOEXCLUDE);
	}
#endif
