/* -------------------------------------------------------------------- */
/*	MOREINFO.H					Citadel									*/
/*			Data and structures for CTDLINFO.DAT and MOREINFO.69.		*/
/* -------------------------------------------------------------------- */

/* Codes... */
#define MI_ERASED	-1
#define	MI_NETCLASS	1
#define	MI_ROOMINFO	2
#define MI_AUTOROOM	3

class MoreInfo
	{
	FILE *File;

	long Start;
	Bool Valid;

	long Code;
	time_t Time;
	long Length;

	void *Data;

public:
	MoreInfo(void)
		{
		assert(this);
		memset(this, 0, sizeof(*this));
		}

	~MoreInfo(void)
		{
		assert(this);
		if (File)
			{
			fclose(File);
			}

		delete [] Data;
		}

	Bool Open(const char *FileName);
	void Close(void);
	Bool LoadNext(void);
	Bool Save(void);
	Bool EraseThis(void);
	Bool SetData(const void *Data, size_t Length);
	Bool GetData(void **Data, size_t *Length);
	};
