// --------------------------------------------------------------------------
// Citadel: Log4.H
//
// This class defines the contents of LOG4.DAT for each user. It specifies
// which rooms a user has excluded. See LOG.H for an explanation of the
// LogBitBag class, which is used to store this information. The only two
// reasons we derive this class are to give it a default file name for
// storage on disk, and so we can use nicely-named functions for access.	


class LogEntry4 : public LogBitBag
	{
public:
#ifdef WINCIT
	void ResumeSyncSend(void);
	Bool Load(l_index Index);
#endif

	LogEntry4(r_slot NumRooms) : LogBitBag(NumRooms)
		{
		GAINEXCLUSIVEACCESS();
		ResetFileName();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry4(void)
		{
		REMOVESYNC();
		}

	void ResetFileName(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SetFileName(log4Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsRoomExcluded(r_slot RoomSlot) const
		{
		return (BitTest(RoomSlot));
		}

	void SetRoomExcluded(r_slot RoomSlot, Bool ExcludeRoom)
		{
		SYNCDATA(L4_SETROOMEXCLUDED, RoomSlot, ExcludeRoom);
		
		BitSet(RoomSlot, ExcludeRoom);
		}
	};
