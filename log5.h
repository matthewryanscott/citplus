// --------------------------------------------------------------------------
// Citadel: Log5.H
//
// This class specifies the contents of LOG5.DAT. This is used to keep track
// of each user's message pointers for each room. For each room, each user
// has an unsigned long that is the number of the newest message on the
// system the last time the user was in the room. Any messages newer than
// this were posted after he was last in the room, and are therefore "new."

class LogEntry5 : public LogStarter
	{
	m_index *NewPointer;
	r_slot	MaxRooms;

#ifdef WINCIT
	l_index SyncNumber;
	Bool	SyncSendPaused;
#endif

public:
#ifdef WINCIT
	int GetSyncNumber(void)
		{
		return (SyncNumber);
		}

	void PauseSyncSend(void)
		{
		SyncSendPaused = TRUE;
		}

	void ResumeSyncSend(void);
#endif

	r_slot GetMaxRooms(void) const
		{
		return (MaxRooms);
		}

	void Clear(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		VerifyHeap();

		if (IsValid())
			{
			memset(NewPointer, 0, MaxRooms * sizeof(*NewPointer));
			WINCODE(SyncNumber = CERROR);
			WINCODE(SyncSendPaused = FALSE);
			VerifyHeap();
			}

		RELEASEEXCLUSIVEACCESS();
		}

	LogEntry5(r_slot NumRooms)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		VerifyHeap();

		MaxRooms = NumRooms;
		NewPointer = new m_index[NumRooms];
		Clear();
		ResetFileName();
		ADDSYNC();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry5()
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		VerifyHeap();
		delete [] NewPointer;
		REMOVESYNC();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	LogEntry5(const LogEntry5 &Original)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		VerifyHeap();
		MaxRooms = Original.MaxRooms;
		NewPointer = new m_index[MaxRooms];
#ifdef WINCIT
		SyncNumber = Original.SyncNumber;
#endif

		if (IsValid() && Original.IsValid())
			{
			memcpy(NewPointer, Original.NewPointer, sizeof(*NewPointer) * MaxRooms);
			}

		label Buffer;
		SetFileName(Original.GetFileName(Buffer, sizeof(Buffer)));
		ADDSYNC();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	LogEntry5& operator =(const LogEntry5 &Original)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		VerifyHeap();
		delete [] NewPointer;

		MaxRooms = Original.MaxRooms;
		NewPointer = new m_index[MaxRooms];
#ifdef WINCIT
		SyncNumber = Original.SyncNumber;
#endif

		if (IsValid() && Original.IsValid())
			{
			memcpy(NewPointer, Original.NewPointer, sizeof(*NewPointer) * MaxRooms);
			}

		label Buffer;
		SetFileName(Original.GetFileName(Buffer, sizeof(Buffer)));

		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		return (*this);
		}

	Bool Load(l_index Index);
	Bool Save(l_index Index);

	void ResetFileName(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SetFileName(log5Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	void Resize(r_slot NewNumRooms)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
	#if defined(AUXMEM) || defined(WINCIT)
		assert(NewNumRooms <= 16376);
	#else
		assert(NewNumRooms <= 65532u / sizeof(rTable));
	#endif

		VerifyHeap();
		delete [] NewPointer;

		MaxRooms = NewNumRooms;
		NewPointer = new m_index[MaxRooms];
		Clear();
		VerifyHeap();
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsValid(void) const
		{
		assert(this);
		return (NewPointer != NULL);
		}

	m_index GetRoomNewPointer(r_slot RoomSlot) const
		{
		assert(this);
		assert(RoomSlot >= 0);
		assert(RoomSlot < MaxRooms);

		return (NewPointer[RoomSlot]);
		}

	void SetRoomNewPointer(r_slot RoomSlot, m_index NewNewPointer)
		{
		SYNCDATA(L5_SETROOMNEWPOINTER, RoomSlot, NewNewPointer);

		assert(this);
		GAINEXCLUSIVEACCESS();
		assert(RoomSlot >= 0);
		assert(RoomSlot < MaxRooms);

		if (IsValid())
			{
			NewPointer[RoomSlot] = NewNewPointer;
			}
		RELEASEEXCLUSIVEACCESS();
		}

	void CopyRoomNewPointers(const LogEntry5 &ToCopy)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		if (IsValid() && ToCopy.IsValid())
			{
			r_slot NumCopy = min(MaxRooms, ToCopy.MaxRooms);

			memcpy(NewPointer, ToCopy.NewPointer, NumCopy * sizeof(*NewPointer));
			}

		RELEASEEXCLUSIVEACCESS();
		}

	const void *GetPointer(void) const
		{
		assert(this);
		return (NewPointer);
		}
	};
