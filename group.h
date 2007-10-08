// --------------------------------------------------------------------------
// Citadel: Group.H
//
// Data and classes for the group subsystem.


// --------------------------------------------------------------------------
// This is all of the information we know about groups. Like rooms, this
// information is stored in GRP.DAT, one entry after another. Unlike rooms,
// however, we do not keep a table to index this file. The file is small
// enough that we keep the whole thing in memory always. This is a potential
// area to look into to save memory, but not much. (Which is why it hasn't
// been done yet.) The only member of this structure that doesn't get much
// use is the group's description. A typical number of groups configured for
// an installation of Citadel is the default: 16. With the description of
// each of these being 80 bytes, the amount of memory that can be saved is
// only 1,280 bytes.

#ifndef _GROUP_H
#define _GROUP_H
class GroupEntry
	{
	EXCLUSIVEACCESSRECORD()

	label	GroupName;			// Name of the group
	uchar	UNUSED1;			// Group generation #.
	char	Description[80];	// Description of group

	Bool	Inuse	: 1;		// Is group inuse?
	Bool	Locked	: 1;		// Sysop access needed for changes
	Bool	Hidden	: 1;		// hidden to all, unless specified by
								// name or in group
	Bool	AutoAdd	: 1;		// Add to new users.
	uint	UNUSED2	: 12;
	uint	UNUSED3 : 16;		// Pad out to 4-byte boundary; same data
								// between Windows and DOS

public:
	GroupEntry(void)
		{
		assert(this);

		INITIALIZEEXCLUSIVEACCESS(TRUE);

		Clear();

		RELEASEEXCLUSIVEACCESS();
		}

	~GroupEntry(void)
		{
		DESTROYEXCLUSIVEACCESS();
		}

	size_t SizeOfDiskRecord(void)
		{
		return (sizeof(*this) - EXCLUSIVEACCESSRECORDSIZE());
		}

	void Clear(void)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();

		VerifyHeap();

		SAVEEXCLUSIVEACCESSRECORD();

		memset(this, 0, sizeof(*this));

		RESTOREEXCLUSIVEACCESSRECORD();

		VerifyHeap();

		RELEASEEXCLUSIVEACCESS();
		}

	void Verify(void)
		{
		// Make sure no strings are too long
		assert(this);
		GroupName[sizeof(GroupName) - 1] = 0;
		Description[sizeof(Description) - 1] = 0;

		// Fill unused areas with nothingness
		UNUSED1	= 0;
		UNUSED2	= 0;
		UNUSED3 = 0;
		}

	const char *GetName(char *Buffer, int Length) const
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, GroupName, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetName(const char *New)
		{
		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(GroupName, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDescription(char *Buffer, int Length) const
		{
		assert(this);
		
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Description, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetDescription(const char *New)
		{
		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Description, New);
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsInuse(void) const
		{
		assert(this);
		return (Inuse);
		}

	void SetInuse(Bool New)
		{
		assert(this);
		Inuse = !!New;
		}

	Bool IsLocked(void) const
		{
		assert(this);
		return (Locked);
		}

	void SetLocked(Bool New)
		{
		assert(this);
		Locked = !!New;
		}

	Bool IsHidden(void) const
		{
		assert(this);
		return (Hidden);
		}

	void SetHidden(Bool New)
		{
		assert(this);
		Hidden = !!New;
		}

	Bool IsAutoAdd(void) const
		{
		assert(this);
		return (AutoAdd);
		}

	void SetAutoAdd(Bool New)
		{
		assert(this);
		AutoAdd = !!New;
		}

	Bool Load(FILE *File);
	Bool Save(FILE *File);

	Bool IsSameName(const char *TestName) const
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		const Bool ToRet = SameString(GroupName, TestName);
		RELEASEEXCLUSIVEACCESS();

		return (ToRet);
		}
	};

class GroupBuffer
	{
	g_slot NumGroups;
	GroupEntry *Groups;

public:
	void Clear(void);

	GroupBuffer(void)
		{
//		assert(this);

		NumGroups = 0;
		Groups = NULL;

		VerifyHeap();
		}

	GroupBuffer(g_slot NumberOfGroups)
		{
//		assert(this);
		VerifyHeap();
		assert(NumberOfGroups <= (65532u / sizeof(GroupEntry)));

		NumGroups = NumberOfGroups;

		Groups = new GroupEntry[NumGroups];
		VerifyHeap();
		}

	~GroupBuffer(void)
		{
//		assert(this);
		VerifyHeap();
		delete [] Groups;
		VerifyHeap();
		}

	void Resize(g_slot NumberOfGroups)
		{
//		assert(this);
		assert(NumberOfGroups <= (65532u / sizeof(GroupEntry)));

		VerifyHeap();

		delete [] Groups;

		NumGroups = NumberOfGroups;

		if (NumberOfGroups)
			{
			Groups = new GroupEntry[NumGroups];
			}
		else
			{
			Groups = NULL;
			}

		VerifyHeap();
		}

	Bool IsValid(void) const
		{
//		assert(this);
		return (Groups != NULL);
		}

	GroupEntry &operator [](g_slot GroupSlot) const
		{
//		assert(this);
		assert(GroupSlot >= 0);
		assert(GroupSlot < NumGroups);
		assert(IsValid());

		return (Groups[GroupSlot]);
		}

	Bool Load(const char *Name = grpDat);
	Bool Save(const char *Name = grpDat) const;

	const GroupEntry *GetPointer(void) const
		{
//		assert(this);
		return (Groups);
		}
	};

extern GroupBuffer GroupData;			// Group data goes here

// GROUP.CPP
g_slot FindGroupByName(const char *TestName);
#endif
