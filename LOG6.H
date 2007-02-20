// --------------------------------------------------------------------------
// Citadel: Log6.H
//
// This class defines the contents of LOG6.DAT for each user. It specifies
// which rooms a user has added to his personal hall. See LOG.H for an
// explanation of the LogBitBag class, which is used to store this
// information. The only two reasons we derive this class are to give it a
// default file name for storage on disk, and so we can use nicely-named
// functions for access.

class LogEntry6 : public LogBitBag
	{
public:
#ifdef WINCIT
	void ResumeSyncSend(void);
	Bool Load(l_index Index);
#endif

	LogEntry6(r_slot NumRooms) : LogBitBag(NumRooms)
		{
		GAINEXCLUSIVEACCESS();
		ResetFileName();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry6(void)
		{
		REMOVESYNC();
		}

	void ResetFileName(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SetFileName(log6Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsRoomInPersonalHall(r_slot RoomSlot) const
		{
		return (BitTest(RoomSlot));
		}

	void SetRoomInPersonalHall(r_slot RoomSlot, Bool InHall)
		{
		SYNCDATA(L6_SETROOMINPERSONALHALL, RoomSlot, InHall);
		
		BitSet(RoomSlot, InHall);
		}
	};
