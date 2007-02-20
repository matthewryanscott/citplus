// --------------------------------------------------------------------------
// Citadel: FileFind.H
//
// File finding stuff


class FindFileC
	{
#ifdef WINCIT
	HANDLE SearchHandle;
	WIN32_FIND_DATA Data;
#else
    struct ffblk Data;
#endif

public:
	FindFileC(void)
		{
#ifdef WINCIT
		SearchHandle = INVALID_HANDLE_VALUE;
#endif
		}

	~FindFileC(void)
		{
#ifdef WINCIT
		if (SearchHandle != INVALID_HANDLE_VALUE)
			{
			FindClose(SearchHandle);
			}
#endif
		}

	Bool FindFirst(const char *Search)
		{
#ifdef WINCIT
		if (SearchHandle != INVALID_HANDLE_VALUE)
			{
			FindClose(SearchHandle);
			}

		SearchHandle = FindFirstFile(Search, &Data);
		return (SearchHandle != INVALID_HANDLE_VALUE);
#else
		return (!findfirst(Search, &Data, FA_RDONLY | FA_HIDDEN | FA_SYSTEM |
				FA_ARCH));
#endif
		}

	Bool FindNext(void)
		{
#ifdef WINCIT
		return (FindNextFile(SearchHandle, &Data));
#else
		return (!findnext(&Data));
#endif
		}

	const char *GetFullName(void) const
		{
#ifdef WINCIT
		return (Data.cFileName);
#else
		return (Data.ff_name);
#endif
		}

	const char *GetShortName(void) const
		{
#ifdef WINCIT
		if (*Data.cAlternateFileName)
			{
			return (Data.cAlternateFileName);
			}
		else
			{
			return (Data.cFileName);
			}
#else
		return (Data.ff_name);
#endif
		}

	Bool IsNormalFile(Bool HiddenOK = FALSE) const
		{
#ifdef WINCIT
		return (((Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
				(HiddenOK ||
					((Data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)) &&
				((Data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) &&
				((Data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0));
#else
		return (((Data.ff_attrib & FA_DIREC) == 0) &&
				(HiddenOK || ((Data.ff_attrib & FA_HIDDEN) == 0)) &&
				((Data.ff_attrib & FA_RDONLY) == 0) &&
				((Data.ff_attrib & FA_SYSTEM) == 0) &&
				((Data.ff_attrib & FA_LABEL) == 0));
#endif
		}

	time_t GetDateTime(void) const
		{
#ifdef WINCIT
		WORD Date, Time;

		FileTimeToDosDateTime(&Data.ftLastWriteTime, &Date, &Time);

		struct date fdate;
		fdate.da_year = (Date >> 9) + 1980;
		fdate.da_day = (char) (Date & 31);
		fdate.da_mon = (char) ((Date >> 5) & 15);

		struct time ftime;
		ftime.ti_hour = (uchar) ((Time >> 11) & 31);
		ftime.ti_min = (uchar) ((Time >> 5) & 63);
		ftime.ti_sec = (uchar) ((Time & 31) << 1);
		ftime.ti_hund = 0;
#else
		struct date fdate;
		fdate.da_year = (Data.ff_fdate >> 9) + 1980;
		fdate.da_day = (char) (Data.ff_fdate & 31);
		fdate.da_mon = (char) ((Data.ff_fdate >> 5) & 15);

		struct time ftime;
		ftime.ti_hour = (Data.ff_ftime >> 11) & 31;
		ftime.ti_min = (Data.ff_ftime >> 5) & 63;
		ftime.ti_sec = (Data.ff_ftime & 31) << 1;
		ftime.ti_hund = 0;
#endif

		return (dostounix(&fdate, &ftime));
		}

#ifdef WINCIT
	DWORD GetAttr(void) const
		{
		return (Data.dwFileAttributes);
		}
#else
	char GetAttr(void) const
		{
		return (Data.ff_attrib);
		}
#endif

	ulong GetSize(void) const
		{
#ifdef WINCIT
		return (Data.nFileSizeHigh ? ULONG_MAX : Data.nFileSizeLow);
#else
		return (Data.ff_fsize);
#endif
		}
	};
