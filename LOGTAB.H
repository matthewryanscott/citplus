// --------------------------------------------------------------------------
// This is the table used to index LOG.DAT.

class LTable : public LogFlags
	{
	STATICEXCLUSIVEACCESSRECORD()

	short	PasswordHash;		// hash of password
	short	InitialsHash;		// hash of initials
	ulong	CallNumber; 		// Which caller # they were
	l_index LogIndex;			// location in LOG.DAT
	label	Name;				// name of user
	char	Alias[4];			// alias of user
	char	LocID[5];			// locID of user

public:
	LTable(void)
		{
#ifdef WINCIT
        INITIALIZEANDCLAIMSTATICEXCLUSIVEACCESS();
#endif
		Clear(0);
		RELEASEEXCLUSIVEACCESS();
		}

	~LTable(void)
		{
		DESTROYSTATICEXCLUSIVEACCESS();
		}

	void Clear(l_index Index)
		{
		GAINEXCLUSIVEACCESS();
		memset(this, 0, sizeof(*this));
		LogIndex = Index;
		RELEASEEXCLUSIVEACCESS();
		}

#ifdef WINCIT
	void GainExclusiveAccess(void)
		{
		GAINEXCLUSIVEACCESS();
		}

	void ReleaseExclusiveAccess(void)
		{
		RELEASEEXCLUSIVEACCESS();
		}
#endif

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

	int GetPasswordHash(void) const 	{ return (PasswordHash); }
	void SetPasswordHash(short New)		{ PasswordHash = New; }
	int GetInitialsHash(void) const 	{ return (InitialsHash); }
	void SetInitialsHash(short New)		{ InitialsHash = New; }

	void SetPasswordHash(const char *New)
		{
		PasswordHash = hash(New);
		}

	void SetInitialsHash(const char *New)
		{
		InitialsHash = hash(New);
		}

	const char *GetName(char *Buffer, int Length) const
		{
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Name, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetName(const char *New)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Name, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetAlias(char *Buffer, int Length) const
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, Alias, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetAlias(const char *New)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(Alias, New);
		RELEASEEXCLUSIVEACCESS();
		}

	const char *GetLocID(char *Buffer, int Length) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		CopyString2Buffer(Buffer, LocID, Length);
		RELEASEEXCLUSIVEACCESS();
		return (Buffer);
		}

	void SetLocID(const char *New)
		{
		assert(this);

		GAINEXCLUSIVEACCESS();
		CopyStringToBuffer(LocID, New);
		RELEASEEXCLUSIVEACCESS();
		}

	l_index GetLogIndex(void) const
		{
		assert(this);
		return (LogIndex);
		}

	void SetLogIndex(l_index New)
		{
		assert(this);
		assert(New >= 0);
		assert(New < cfg.MAXLOGTAB);

		LogIndex = New;
		}

	ulong GetCallNumber(void) const
		{
		assert(this);
		return (CallNumber);
		}

	void SetCallNumber(ulong New)
		{
		assert(this);
		CallNumber = New;
		}

	Bool IsMainSysop(void) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const Bool ToRet = SameString(Name, cfg.sysop);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool IsSameName(const char *TestName) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const Bool ToRet = SameString(TestName, Name);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool IsSameAlias(const char *TestAlias) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const Bool ToRet = SameString(TestAlias, Alias);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}

	Bool IsSameLocID(const char *TestLocID) const
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		const Bool ToRet = SameString(TestLocID, LocID);
		RELEASEEXCLUSIVEACCESS();
		return (ToRet);
		}
	};


class LogTable
	{
	l_slot TableSize;

#ifdef WINCIT
	static LTable *Table;
#else
	LTable *Table;
#endif

public:
	LogTable(void)
		{
		TableSize = 0;
		Table = NULL;
		}

	void Clear(void);

	Bool Resize(l_slot SizeOfTable)
		{
//		assert(this);
		assert (SizeOfTable <= (65532u / sizeof(LTable)));

		VerifyHeap();

		delete [] Table;

		VerifyHeap();

		TableSize = SizeOfTable;

		if (SizeOfTable)
			{
			Table = new LTable[SizeOfTable];
			Clear();
			}
		else
			{
			Table = NULL;
			}

		VerifyHeap();
		return (IsValid());
		}

	~LogTable(void)
		{
//		assert(this);

		VerifyHeap();
		delete [] Table;
		VerifyHeap();
		}

	Bool IsValid(void) const
		{
//		assert(this);
		return (Table != NULL);
		}

	LTable &operator [](l_slot LogSlot) const
		{
//		assert(this);
		assert(LogSlot >= 0);
		assert(LogSlot < TableSize);
		assert(IsValid());

		return (Table[LogSlot]);
		}

	Bool Load(void);
	void Save(void) const;

#ifdef WINCIT
	void SlideTableDown(l_slot HowFar, l_slot *Order);
#else
	void SlideTableDown(l_slot HowFar);
#endif

	void UpdateTable(const LogEntry1 *Entry, l_index Index);

#ifdef WINCIT
#ifdef MINGW
	static int LogSorter(l_slot *s1, l_slot *s2)
#else
	static int _cdecl LogSorter(l_slot *s1, l_slot *s2)
#endif
		{
		if (!Table[*s1].IsInuse() && !Table[*s2].IsInuse())
			{
			return 0;
			}

		if (!Table[*s1].IsInuse() && Table[*s2].IsInuse())
			{
			return 1;
			}

		if (Table[*s1].IsInuse() && !Table[*s2].IsInuse())
			{
			return -1;
			}

		if (Table[*s1].GetCallNumber() < Table[*s2].GetCallNumber())
			{
			return 1;
			}

		if (Table[*s1].GetCallNumber() > Table[*s2].GetCallNumber())
			{
			return -1;
			}

		return 0;
		}

	void Sort(l_slot **Order)
		{
		assert(Order);
		assert(*Order);

		if (IsValid() && Order && *Order)
			{
			qsort(*Order, TableSize, sizeof(l_slot), (QSORT_CMP_FNP) LogSorter);
			}
		}
#else
	static int _cdecl LogSorter(LTable *s1, LTable *s2)
		{
		if (!s1->IsInuse() && !s2->IsInuse())
			{
			return 0;
			}

		if (!s1->IsInuse() && s2->IsInuse())
			{
			return 1;
			}

		if (s1->IsInuse() && !s2->IsInuse())
			{
			return -1;
			}

		if (s1->GetCallNumber() < s2->GetCallNumber())
			{
			return 1;
			}

		if (s1->GetCallNumber() > s2->GetCallNumber())
			{
			return -1;
			}

		return 0;
		}

	void Sort(void)
		{
//		assert(this);

		if (IsValid())
			{
			qsort(Table, TableSize, sizeof(*Table), (QSORT_CMP_FNP) LogSorter);
			}
		}
#endif

	void *GetPointer(void)
		{
//		assert(this);
		return (Table);
		}

	void Update(l_slot TabSlot, const LogEntry1 *Log, l_index FileIndex);
	};
