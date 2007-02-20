// --------------------------------------------------------------------------
// Citadel: Log2.H
//
// This class defines the contents of LOG2.DAT for each user. It specifies
// which groups a user is in. See LOG.H for an explanation of the LogBitBag
// class, which is used to store this information. The three reasons we
// derive this class are to give it a default file name for storage on disk,
// so we can use nicely-named functions for access, and to allow us to create
// a "smart" Clear() function.


class LogEntry2 : public LogBitBag
	{
public:
#ifdef WINCIT
	void ResumeSyncSend(void);
	Bool Load(l_index Index);
#endif

	LogEntry2(r_slot NumRooms) : LogBitBag(NumRooms)
		{
		GAINEXCLUSIVEACCESS();
		ResetFileName();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry2(void)
		{
		REMOVESYNC();
		}

	void ResetFileName(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SetFileName(log2Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsInGroup(g_slot GroupSlot) const
		{
		return (BitTest(GroupSlot));
		}

	void SetInGroup(g_slot GroupSlot, Bool InGroup)
		{
		SYNCDATA(L2_SETINGROUP, GroupSlot, InGroup);
		
		BitSet(GroupSlot, InGroup);
		}

	void Clear(void);
	};
