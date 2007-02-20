// --------------------------------------------------------------------------
// Citadel: Hall.H
//
// Data and classes for the hall subsystem.

#include "bitbag.h"
#include "boolexpr.h"


class HallBuffer;

// --------------------------------------------------------------------------
// This is almost all of the information we know about halls. This is stored
// in HALL.DAT. Like groups, we keep this whole buffer in memory at a time.
// There is nothing that can really be taken out to make a table out of it.
// But, why is it only "almost" all of the information? This is because we
// also keep track of which rooms are in a hall, and which rooms are windowed
// into a hall. We cannot put this data into the class, because the number of
// rooms varies.
//
// To keep track of that data, we have another hall file: HALL2.DAT. This
// file is very simple: for each hall, we first write out which rooms are in
// the hall, then which rooms are windowed into the hall. Each room gets a
// single bit in each list, set if the room is in or windowed into the hall,
// cleared if it is not. This is handled by the HallEntry2 and HallEntry3
// classes, below.

class HallEntry1
	{
	EXCLUSIVEACCESSRECORD()

	label	Name;				// Name of our hall

	uchar	GroupNumber;		// owning group's #
								// Would prefer a g_slot... someday

	Bool	Inuse		: 1;	// Is hall in use?
	Bool	Owned		: 1;	// Is this hall owned?
	Bool	Described	: 1;	// described?
	Bool	Default 	: 1;	// default for new users (only one allowed!)
	Bool	EnterRoom	: 1;	// Can users enter rooms here?
	Bool	BoolGroup	: 1;	// If GroupExpression...

	uint	UNUSED2 	: 10;

	uchar	UNUSED3;			// hall default mainencance hall (not)
	char	DescriptionFile[13];// Hall description

	BoolExpr GroupExpression;	// If BoolGroup


public:
	HallEntry1(void)
		{
		assert(this);

		INITIALIZEEXCLUSIVEACCESS(TRUE);

		Clear();

		RELEASEEXCLUSIVEACCESS();
		}

	~HallEntry1(void)
		{
		DESTROYEXCLUSIVEACCESS();
		}

	size_t SizeOfDiskRecord(void) const
		{
		return (sizeof(Name) + sizeof(GroupNumber) + 2 + sizeof(UNUSED3) +
				sizeof(DescriptionFile) + sizeof(GroupExpression));
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

	HallEntry1& operator =(const HallEntry1 &Original)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();

		VerifyHeap();

		SAVEEXCLUSIVEACCESSRECORD();

		memcpy(this, &Original, sizeof(HallEntry1));

		RESTOREEXCLUSIVEACCESSRECORD();

		VerifyHeap();

		RELEASEEXCLUSIVEACCESS();

		return (*this);
		}

	void Verify(void)
		{
		assert(this);

		// Make sure no strings are too long
		Name[LABELSIZE] = 0;
		DescriptionFile[12] = 0;

		// Fill unused areas with nothingness
		UNUSED2 = 0;
		UNUSED3 = 0;

		// Don't overindex the group buffer
		if (GroupNumber >= cfg.maxgroups)
			{
			GroupNumber = 0;
			}
		}

	const char *GetName(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Name, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetName(const char *New)
		{
		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Name, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetDescriptionFile(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, DescriptionFile, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetDescriptionFile(const char *New)
		{
		assert(this);
		assert(New);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(DescriptionFile, New);
		RELEASEEXCLUSIVEACCESS();
		}

	g_slot GetGroupNumber(void) const
		{
		assert(this);
		assert(Owned);
		assert(!BoolGroup);

		return (GroupNumber);
		}

	void SetGroupNumber(g_slot New)
		{
		assert(this);
		assert(Owned);
		assert(!BoolGroup);

		GroupNumber = (uchar) New;
		}

	Bool IsOwned(void) const
		{
		assert(this);
		return (Owned);
		}

	void SetOwned(Bool New)
		{
		assert(this);
		Owned = !!New;
		}

	Bool IsDescribed(void) const
		{
		assert(this);
		return (Described);
		}

	void SetDescribed(Bool New)
		{
		assert(this);
		Described = !!New;
		}

	// Don't set to TRUE without unsetting everything else. It won't crash if
	// you do, but it might confuse the user.
	void SetDefault(Bool New)
		{
		assert(this);
		Default = !!New;
		}

	Bool IsDefault(void) const
		{
		assert(this);
		return (Default);
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

	Bool IsEnterRoom(void) const
		{
		assert(this);
		return (EnterRoom);
		}

	void SetEnterRoom(Bool New)
		{
		assert(this);
		EnterRoom = !!New;
		}

	Bool IsBoolGroup(void) const
		{
		assert(this);
		return (BoolGroup);
		}

	void SetBoolGroup(Bool New)
		{
		assert(this);
		BoolGroup = !!New;
		}

	const ushort *GetGroupExpression(BoolExpr Buffer) const
		{
		assert(this);
		assert(Owned);
		assert(BoolGroup);

		GAINEXCLUSIVEACCESS();
		memcpy(Buffer, GroupExpression, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetGroupExpression(BoolExpr New)
		{
		assert(this);
		assert(Owned);
		assert(BoolGroup);
		assert(IsBooleanExpressionValid(New));

		GAINEXCLUSIVEACCESS();
		memcpy(GroupExpression, New, sizeof(BoolExpr));
		RELEASEEXCLUSIVEACCESS();
		}

	Bool Load(h_slot Slot, FILE *File);
	Bool Save(h_slot Slot, FILE *File) const;

	Bool IsSameName(const char *TestName)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		Bool ToRet = SameString(TestName, Name);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}
	};

class HallEntry2 : public BitBag
	{
	EXCLUSIVEACCESSRECORD()

	char FileName[13];

public:
	void SetFileName(const char *NewName)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(FileName, NewName);
		RELEASEEXCLUSIVEACCESS();
		}

	void ResetFileName(void)
		{
		GAINEXCLUSIVEACCESS();
		SetFileName(hall2Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	HallEntry2(r_slot NumRooms = 0) :
		BitBag(NumRooms)
		{
		INITIALIZEEXCLUSIVEACCESS(TRUE);
		ResetFileName();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	Bool Load(h_slot Index, FILE *HallFile);
	Bool Save(h_slot Index, FILE *HallFile) const;

	Bool IsRoomInHall(r_slot RoomSlot) const
		{
		return (BitTest(RoomSlot));
		}

	void SetRoomInHall(r_slot RoomSlot, Bool RoomInHall)
		{
		BitSet(RoomSlot, RoomInHall);
		}
	};

class HallEntry3 : public BitBag
	{
	EXCLUSIVEACCESSRECORD()

	char FileName[13];

public:
	void SetFileName(const char *NewName)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(FileName, NewName);
		RELEASEEXCLUSIVEACCESS();
		}

	void ResetFileName(void)
		{
		GAINEXCLUSIVEACCESS();
		SetFileName(hall2Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	HallEntry3(r_slot NumRooms = 0) :
		BitBag(NumRooms)
		{
		INITIALIZEEXCLUSIVEACCESS(TRUE);
		ResetFileName();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	Bool Load(h_slot Index, FILE *HallFile);
	Bool Save(h_slot Index, FILE *HallFile) const;

	Bool IsWindowedIntoHall(r_slot RoomSlot) const
		{
		return (BitTest(RoomSlot));
		}

	void SetWindowedIntoHall(r_slot RoomSlot, Bool Windowed)
		{
		BitSet(RoomSlot, Windowed);
		}
	};

class HallEntry : public HallEntry1, public HallEntry2, public HallEntry3
	{
public:
	HallEntry(void);
	HallEntry(r_slot Rooms);

	HallEntry& HallEntry::operator = (const HallEntry &Original)
		{
		assert(this);
		VerifyHeap();
		HallEntry1::operator =(Original);
		HallEntry2::operator =(Original);
		HallEntry3::operator =(Original);
		VerifyHeap();
		return (*this);
		}

	void Resize(r_slot Rooms)
		{
		assert(this);
		HallEntry2::Resize(Rooms);
		HallEntry3::Resize(Rooms);
		}

	void Clear(void);

	Bool IsValid(void) const;

	Bool Load(h_slot HallSlot, FILE *HallFile1, FILE *HallFile2);
	Bool Save(h_slot HallSlot, FILE *HallFile1, FILE *HallFile2) const;
	};

class HallBuffer
	{
	h_slot NumHalls;
	HallEntry *Halls;

public:
	HallBuffer(void)
		{
		NumHalls = 0;
		Halls = NULL;
		}

	void Clear(void);

	void Resize(h_slot NewNumberOfHalls, r_slot NumberOfRooms);

	~HallBuffer(void)
		{
//		assert(this);
		VerifyHeap();
		delete [] Halls;
		VerifyHeap();
		}

	Bool IsValid(void) const
		{
//		assert(this);
		return (Halls != NULL);
		}

	HallEntry &operator [](h_slot HallSlot) const
		{
//		assert(this);
		assert(HallSlot >= 0);
		assert(HallSlot < NumHalls);
		assert (IsValid());

		return (Halls[HallSlot]);
		}

	void HallBuffer::SetDefault(h_slot New);
	h_slot HallBuffer::GetDefault(void);

	Bool Load(const char *Name = hallDat);
	Bool Save(const char *Name = hallDat) const;

	const HallEntry *GetPointer(void) const
		{
//		assert(this);
		return (Halls);
		}
	};

extern HallBuffer	HallData;			// Hall data goes here

// HALL.CPP
h_slot hallexists(const char *hallname);
Bool iswindow(r_slot roomslot);
Bool roominhall(r_slot roomslot, h_slot hallslot);
Bool roomonlyinthishall(r_slot roomslot, h_slot hallslot);
