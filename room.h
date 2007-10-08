// --------------------------------------------------------------------------
// Citadel: Room.H
//
// One of Citadel's main concepts is that of rooms. Rooms have many
// attributes (name, group, application, etc.), and contain messages. Rooms
// are grouped together into Halls, another of the main concepts which
// Citadel is built on top of. Because rooms are so key to the operation of
// the program, the interface to this subsystem is quite extensive: there is
// a lot of data involved with rooms. Many of the routines are in various
// .CPP files, because there are too many to fit into one file and have the
// overlay system work well. It may seem that the grouping of the routines
// into different source files is arbitrary. This is because the first
// critereon for grouping these routines is in terms of overlay friendlyness,
// which is not always what makes the most since in terms of function
// groupings.

#ifndef _ROOM_H
#define _ROOM_H

#include "auxtab.h"
#include "boolexpr.h"
#include "syncdata.h"


// --------------------------------------------------------------------------
// The next structure contains a room's flags. This is used in both the full
// structure of a room and in the index.

class RoomFlags
	{
	Bool INUSE		: 1;		// Room in use?
	Bool PUBLIC 	: 1;		// Room public?
	Bool MSDOSDIR	: 1;		// Room directory?
	Bool PERMROOM	: 1;		// Room permanent?
	Bool GROUPONLY	: 1;		// Room Owned by a group?
	Bool READONLY	: 1;		// Room Read only?
	Bool DOWNONLY	: 1;		// Room download only?
	Bool SHARED 	: 1;		// Room Shared?
	Bool MODERATED	: 1;		// Room Moderated?
	Bool APLIC		: 1;		// Room have application?
	Bool BIO		: 1;		// By invintation only..
	Bool UPONLY 	: 1;		// Upload only
	Bool PRIVGRP	: 1;		// Has a privlage group
	Bool ANON		: 1;		// Anonymous messages in this room...
	Bool SUBJECT	: 1;		// ask user for subject in this room
	Bool GRP_MOD	: 1;		// Access group is moderators..
	Bool ARCHIVE	: 1;		// archive the room?
	Bool BOOLGROUP	: 1;		// boolean expresion for group access?
	uint UNUSED 	: 14;		// ...

public:
	RoomFlags(void)
		{
		INUSE = PUBLIC = MSDOSDIR = PERMROOM = GROUPONLY = READONLY =
				DOWNONLY = SHARED = MODERATED = APLIC = BIO = UPONLY =
				PRIVGRP = ANON = SUBJECT = GRP_MOD = ARCHIVE = BOOLGROUP =
				FALSE;

		UNUSED = 0;
		}

	void SetInuse(Bool New) 			{	INUSE = !!New;		}
	Bool IsInuse(void) const			{	return (INUSE); 	}

	void SetPublic(Bool New)			{	PUBLIC = !!New; 	}
	Bool IsPublic(void) const			{	return (PUBLIC);	}

	void SetHidden(Bool New)			{	PUBLIC = !New;	}
	Bool IsHidden(void) const			{	return (!PUBLIC);	}

	void SetMsDOSdir(Bool New)			{	MSDOSDIR = !!New;	}
	Bool IsMsDOSdir(void) const 		{	return (MSDOSDIR);	}

	void SetPermanent(Bool New) 		{	PERMROOM = !!New;	}
	Bool IsPermanent(void) const		{	return (PERMROOM);	}

	void SetGroupOnly(Bool New) 		{	GROUPONLY = !!New;	}
	Bool IsGroupOnly(void) const		{	return (GROUPONLY); }

	void SetReadOnly(Bool New)			{	READONLY = !!New;	}
	Bool IsReadOnly(void) const 		{	return (READONLY);	}

	void SetDownloadOnly(Bool New)		{	DOWNONLY = !!New;	}
	Bool IsDownloadOnly(void) const 	{	return (DOWNONLY);	}

	void SetShared(Bool New)			{	SHARED = !!New; }
	Bool IsShared(void) const			{	return (SHARED);	}

	void SetModerated(Bool New) 		{	MODERATED = !!New;	}
	Bool IsModerated(void) const		{	return (MODERATED); }

	void SetApplication(Bool New)		{	APLIC = !!New;		}
	Bool IsApplication(void) const		{	return (APLIC); 	}

	void SetBIO(Bool New)				{	BIO = !!New;		}
	Bool IsBIO(void) const				{	return (BIO);		}

	void SetUploadOnly(Bool New)		{	UPONLY = !!New; 	}
	Bool IsUploadOnly(void) const		{	return (UPONLY);	}

	void SetPrivilegedGroup(Bool New)	{	PRIVGRP = !!New;	}
	Bool IsPrivilegedGroup(void) const	{	return (PRIVGRP);	}

	void SetAnonymous(Bool New) 		{	ANON = !!New;		}
	Bool IsAnonymous(void) const		{	return (ANON);		}

	void SetSubject(Bool New)			{	SUBJECT = !!New;	}
	Bool IsSubject(void) const			{	return (SUBJECT);	}

	void SetGroupModerates(Bool New)	{	GRP_MOD = !!New;	}
	Bool IsGroupModerates(void) const	{	return (GRP_MOD);	}

	void SetArchive(Bool New)			{	ARCHIVE = !!New;	}
	Bool IsArchive(void) const			{	return (ARCHIVE);	}

	void SetBooleanGroup(Bool New)
		{
		if (New)
			{
			GROUPONLY = TRUE;
			}

		BOOLGROUP = !!New;
		}

	Bool IsBooleanGroup(void) const
		{
		return (GROUPONLY && BOOLGROUP);
		}
	};


// --------------------------------------------------------------------------
// This is everything we know about a room. This data is stored in the
// ROOM.DAT file, one entry per room.

class RoomC
	{
	EXCLUSIVEACCESSRECORD()

#ifdef WINCIT
	r_slot	SyncNumber;
	Bool	SyncSendPaused;
#endif

	RoomFlags Flags;			// see above
	label	Name;				// name of room
	char	Directory[64];		// user directory for room's files
	char	DescFile[13];		// filename for room description
	char	Applic[13]; 		// filename for room application
	char	UNUSED1;			// Padding
	g_slot	GrpNo;				// group # which owns it

	short	UNUSED2;			// (RETHINK) moderator's hash
	short	UNUSED3;			// (RETHINK) host for BIO room
	g_slot	PGrpNo; 			// group number for privliges grp

	Bool	AUTOAPP : 1;		// Automatically run application.
	Bool	NOEXCLUDE : 1;		// users cannot exclude the room

	uint	UNUSED4 : 14;

	char	Description[80];	// for .kv[...]

	label	NetID;				// Network ID for room
	char	Archive[13];		// archive file for room
	char	Dictionary[13]; 	// dictionary file for room.
	label	ModHost;			// the moderator and host of the room
	label	ExclNetPartner; 	// net with only here.
	char	UNUSED5;			// Pad
	BoolExpr GroupExpression;	// If BOOLGROUP
//	BoolExpr NetClass;			// Taking over the network.

	label	Creator;			// Zen-o-phobia
	char	UNUSED6;			// Pad

public:
#ifdef WINCIT
	int GetSyncNumber(void) const
		{
		return (SyncNumber);
		}

	void PauseSyncSend(void)
		{
		SyncSendPaused = TRUE;
		}

	void ResumeSyncSend(void);
#endif

	RoomC(void)
		{
		assert(this);
		INITIALIZEEXCLUSIVEACCESS(TRUE);
		Clear();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~RoomC(void)
		{
		REMOVESYNC();
		DESTROYEXCLUSIVEACCESS();
		}

	void Clear(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SAVEEXCLUSIVEACCESSRECORD();
		memset(this, 0, sizeof(RoomC));

#ifdef WINCIT
		SyncNumber = CERROR;
#endif
		RESTOREEXCLUSIVEACCESSRECORD();
		RELEASEEXCLUSIVEACCESS();
		}

	static size_t GetDiskRecordSize(void)
		{
		return (sizeof(RoomFlags) + sizeof(label) + 64 + 13 + 13 +
				sizeof(char) + sizeof(g_slot) + sizeof(short) +
				sizeof(short) + sizeof(g_slot) + 2 + 80 + sizeof(label) +
				13 + 13 + sizeof(label) + sizeof(label) + sizeof(char) +
				sizeof(BoolExpr) + sizeof(label) + sizeof(char));
		}

	void Verify(void);

	Bool Load(r_slot Slot);
	Bool Save(r_slot Slot);

	Bool IsInuse(void) const			{ return (Flags.IsInuse()); 		}
	void SetInuse(Bool New)
		{
		SYNCDATA(RM_SETINUSE, New, 0);

		Flags.SetInuse(New);
		}

	Bool IsPublic(void) const			{ return (Flags.IsPublic());		}
	void SetPublic(Bool New)
		{
		SYNCDATA(RM_SETPUBLIC, New, 0);

		Flags.SetPublic(New);
		}

	Bool IsMsDOSdir(void) const 		{ return (Flags.IsMsDOSdir());		}
	void SetMsDOSdir(Bool New)
		{
		SYNCDATA(RM_SETMSDOSDIR, New, 0);

		Flags.SetMsDOSdir(New);
		}

	Bool IsPermanent(void) const		{ return (Flags.IsPermanent()); 	}
	void SetPermanent(Bool New)
		{
		SYNCDATA(RM_SETPERMANENT, New, 0);

		Flags.SetPermanent(New);
		}

	Bool IsGroupOnly(void) const		{ return (Flags.IsGroupOnly()); 	}
	void SetGroupOnly(Bool New)
		{
		SYNCDATA(RM_SETGROUPONLY, New, 0);

		Flags.SetGroupOnly(New);
		}

	Bool IsReadOnly(void) const 		{ return (Flags.IsReadOnly());		}
	void SetReadOnly(Bool New)
		{
		SYNCDATA(RM_SETREADONLY, New, 0);

		Flags.SetReadOnly(New);
		}

	Bool IsDownloadOnly(void) const 	{ return (Flags.IsDownloadOnly());	}
	void SetDownloadOnly(Bool New)
		{
		SYNCDATA(RM_SETDOWNLOADONLY, New, 0);

		Flags.SetDownloadOnly(New);
		}

	Bool IsShared(void) const			{ return (Flags.IsShared());		}
	void SetShared(Bool New)
		{
		SYNCDATA(RM_SETSHARED, New, 0);

		Flags.SetShared(New);
		}

	Bool IsModerated(void) const		{ return (Flags.IsModerated()); 	}
	void SetModerated(Bool New)
		{
		SYNCDATA(RM_SETMODERATED, New, 0);

		Flags.SetModerated(New);
		}

	Bool IsApplication(void) const		{ return (Flags.IsApplication());	}
	void SetApplication(Bool New)
		{
		SYNCDATA(RM_SETAPPLICATIONB, New, 0);

		Flags.SetApplication(New);
		}

	Bool IsBIO(void) const				{ return (Flags.IsBIO());			}
	void SetBIO(Bool New)
		{
		SYNCDATA(RM_SETBIO, New, 0);

		Flags.SetBIO(New);
		}

	Bool IsUploadOnly (void) const		{ return (Flags.IsUploadOnly());	}
	void SetUploadOnly (Bool New)
		{
		SYNCDATA(RM_SETUPLOADONLY , New, 0);

		Flags.SetUploadOnly(New);
		}

	Bool IsPrivilegedGroup(void) const	{ return (Flags.IsPrivilegedGroup());}
	void SetPrivilegedGroup(Bool New)
		{
		SYNCDATA(RM_SETPRIVILEGEDGROUP, New, 0);

		Flags.SetPrivilegedGroup(New);
		}

	Bool IsAnonymous(void) const		{ return (Flags.IsAnonymous()); 	}
	void SetAnonymous(Bool New)
		{
		SYNCDATA(RM_SETANONYMOUS, New, 0);

		Flags.SetAnonymous(New);
		}

	Bool IsSubject(void) const			{ return (Flags.IsSubject());		}
	void SetSubject(Bool New)
		{
		SYNCDATA(RM_SETSUBJECT, New, 0);

		Flags.SetSubject(New);
		}

	Bool IsGroupModerates(void) const	{ return (Flags.IsGroupModerates());}
	void SetGroupModerates(Bool New)
		{
		SYNCDATA(RM_SETGROUPMODERATES, New, 0);

		Flags.SetGroupModerates(New);
		}

	Bool IsArchive(void) const			{ return (Flags.IsArchive());		}
	void SetArchive(Bool New)
		{
		SYNCDATA(RM_SETARCHIVE, New, 0);

		Flags.SetArchive(New);
		}

	Bool IsBooleanGroup(void) const 	{ return (Flags.IsBooleanGroup());	}
	void SetBooleanGroup(Bool New)
		{
		SYNCDATA(RM_SETBOOLEANGROUP, New, 0);

		Flags.SetBooleanGroup(New);
		}

	void SetName(const char *New)
		{
		SYNCDATA(RM_SETNAME, New, 0);

		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		if (*New)
			{
			CopyStringToBuffer(Name, New);
			}
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetName(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Name, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetDirectory(const char *New)
		{
		SYNCDATA(RM_SETDIRECTORY, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Directory, New);
		strupr(Directory);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDirectory(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Directory, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	Bool ChangeDir(void) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		Bool ToRet = changedir(Directory);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	void SetDescFile(const char *New)
		{
		SYNCDATA(RM_SETDESCFILE, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(DescFile, New);
		strupr(DescFile);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDescFile(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, DescFile, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetApplication(const char *New)
		{
		SYNCDATA(RM_SETAPPLICATIONS, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Applic, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetApplication(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Applic, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetGroup(g_slot New)
		{
		SYNCDATA(RM_SETGROUP, New, 0);

		GrpNo = New;
		}

	g_slot GetGroup(void) const
		{
		return (GrpNo);
		}

	void SetPGroup(g_slot New)
		{
		SYNCDATA(RM_SETPGROUP, New, 0);

		PGrpNo = New;
		}

	g_slot GetPGroup(void) const
		{
		return (PGrpNo);
		}

	void SetAutoApp(Bool New)
		{
		SYNCDATA(RM_SETAUTOAPP, New, 0);

		AUTOAPP = New;
		}

	Bool IsAutoApp(void) const
		{
		return (AUTOAPP);
		}

	void SetUnexcludable(Bool New)
		{
		SYNCDATA(RM_SETUNEXCLUDABLE, New, 0);

		NOEXCLUDE = New;
		}

	Bool IsUnexcludable(void) const
		{
		return (NOEXCLUDE);
		}

	void SetInfoLine(const char *New)
		{
		SYNCDATA(RM_SETINFOLINE, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Description, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetInfoLine(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Description, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetNetID(const char *New)
		{
		SYNCDATA(RM_SETNETID, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(NetID, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetNetID(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, NetID, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetArchiveFile(const char *New)
		{
		SYNCDATA(RM_SETARCHIVEFILE, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Archive, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetArchiveFile(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Archive, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetDictionary(const char *New)
		{
		SYNCDATA(RM_SETDICTIONARY, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Dictionary, New);
		strupr(Dictionary);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDictionary(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Dictionary, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetGroupExpression(const BoolExpr New)
		{
		SYNCDATA(RM_SETGROUPEXPRESSION, New, 0);

		assert(this);
		GAINEXCLUSIVEACCESS();
//		assert(IsBooleanExpressionValid(New));
		memcpy(GroupExpression, New, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		}

	const ushort *GetGroupExpression(BoolExpr Buffer) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		memcpy(Buffer, GroupExpression, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	const char *GetCreator(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Creator, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetCreator(const char *New)
		{
		SYNCDATA(RM_SETCREATOR, New, 0);

		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		if (*New)
			{
			CopyStringToBuffer(Creator, New);
			}
		RELEASEEXCLUSIVEACCESS();
		}
	};


// --------------------------------------------------------------------------
// The following is the room table structure. The room table is used as a
// quick in-memory lookup into ROOM.DAT, and contains all of the fields that
// we need often.

class RTable : public RoomFlags
	{
	STATICEXCLUSIVEACCESSRECORD()

	label	Name;				// name of room
	label	NetID;				// Network ID for room
	g_slot	GroupNumber;		// group # which owns it
	g_slot	PGroupNumber;		// group # which is privileged

	BoolExpr GroupExpression;	// If BOOLGROUP
//	BoolExpr NetClass;			// ...

public:
	RTable(void)
		{
#ifdef WINCIT
		INITIALIZEANDCLAIMSTATICEXCLUSIVEACCESS();
#endif
		Clear();
		RELEASEEXCLUSIVEACCESS();
		}

	~RTable(void)
		{
		DESTROYSTATICEXCLUSIVEACCESS();
		}

	void Clear(void)
		{
		GAINEXCLUSIVEACCESS();
		memset(this, 0, sizeof(*this));
		RELEASEEXCLUSIVEACCESS();
		}

	size_t SizeOfDiskRecord(void)
		{
		return (sizeof(*this));
		}

	Bool Save(FILE *f)
		{
		GAINEXCLUSIVEACCESS();
		const Bool Good = fwrite(this, SizeOfDiskRecord(), 1, f) == 1;
		RELEASEEXCLUSIVEACCESS();
		return (Good);
		}

	Bool Load(FILE *f)
		{
		GAINEXCLUSIVEACCESS();
		const Bool Good = fread(this, SizeOfDiskRecord(), 1, f) == 1;
		RELEASEEXCLUSIVEACCESS();
		return (Good);
		}

	void SetName(const char *New)
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Name, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetName(char *Buffer, int Length) const
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Name, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	Bool IsSameName(const char *TestName) const
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(Name, TestName);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	void SetNetID(const char *New)
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(NetID, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetNetID(char *Buffer, int Length) const
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, NetID, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	Bool IsSameNetID(const char *TestNetID) const
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(NetID, TestNetID);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	void SetGroupNumber(g_slot New)
		{
		assert(IsInuse());
		GroupNumber = New;
		}

	g_slot GetGroupNumber(void) const
		{
		assert(IsInuse());
		assert(IsGroupOnly());
		assert(!IsBooleanGroup());

		return (GroupNumber);
		}

	void SetPGroupNumber(g_slot New)
		{
		assert(IsInuse());
		PGroupNumber = New;
		}

	g_slot GetPGroupNumber(void) const
		{
		assert(IsInuse());
		assert(IsPrivilegedGroup());

		return (PGroupNumber);
		}

	void SetGroupExpression(const BoolExpr New)
		{
		assert(IsInuse());
		GAINEXCLUSIVEACCESS();
		memcpy(GroupExpression, New, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		}

	const ushort *GetGroupExpression(BoolExpr Buffer)
		{
		assert(IsInuse());
		assert(IsGroupOnly());
		assert(IsBooleanGroup());

		GAINEXCLUSIVEACCESS();
		memcpy(Buffer, GroupExpression, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}
	};


class RoomTable
	{
	r_slot TableSize;

#ifdef AUXMEM
	#define ROOMTABPERPAGE	(AUXPAGESIZE / sizeof(RTable))

	auxTabList	*rtList;			// base of our linked list
	int 		roomBlocksInHeap;	// how many we have
#else
	RTable *Table;
#endif

public:
	RoomTable(void)
		{
		TableSize = 0;
#ifdef AUXMEM
		rtList = NULL;
		roomBlocksInHeap = 0;
#else
		Table = NULL;
#endif
		}

	void Clear(void);


#ifdef AUXMEM
	Bool Resize(r_slot SizeOfTable);
#else
	Bool Resize(r_slot SizeOfTable)
		{
		delete [] Table;

		VerifyHeap();

		TableSize = SizeOfTable;

		if (SizeOfTable)
			{
			Table = new RTable[SizeOfTable];
			Clear();
			}
		else
			{
			Table = NULL;
			}

		VerifyHeap();
		return (IsValid());
		}
#endif

	~RoomTable(void)
		{
		VerifyHeap();
		Resize(0);
		VerifyHeap();
		}

	Bool IsValid(void) const
		{
#ifdef AUXMEM
		return (rtList != NULL);
#else
		return (Table != NULL);
#endif
		}

	RTable &operator [](r_slot RoomSlot)
		{
		assert(RoomSlot >= 0);
		assert(RoomSlot < TableSize);
		assert(IsValid());

#ifdef AUXMEM
		return (*((RTable *) (LoadAuxmemBlock((RoomSlot), &rtList, ROOMTABPERPAGE, sizeof(RTable)))));
#else
		return (Table[RoomSlot]);
#endif
		}

	Bool Load(void);
	void Save(void);
	};


// ROOM2.CPP
r_slot FindFreeRoomSlot(void);
void DebugAuxmemRoomTab(void);


// ROOMLOW.CPP
r_slot GetRoomPosInTable(r_slot RoomIndex);
r_slot RoomExists(const char *RoomName);

// --------------------------------------------------------------------------
// RoomPosTableToRoom(): Gets a room from position in the roompos table.
//
// Input:
//	r_slot RoomIndex: The index to look up
//
// Return value:
//	r_slot: The room referenced by RoomIndex

inline r_slot RoomPosTableToRoom(r_slot RoomIndex)
	{
	assert(RoomIndex < cfg.maxrooms);
	assert(RoomIndex >= 0);
	assert(roomPos != NULL);

	return (roomPos[RoomIndex]);
	}

// ROOM3 - ICKY STUFF TO BE MOVED LATER
Bool IsNetIDCorrupted(const char *NetID);

// RoomLow.CPP
r_slot IdExists(const char *id, Bool network);


extern RoomTable RoomTab;

#endif
