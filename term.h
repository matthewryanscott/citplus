class TermCapC
	{
	label	Name;

	Bool	Color;
	Bool	IBMExtended;

	label	ClsCode, SetForeCode, SetBackCode, SetBothCode, UnderlineCode;
	label	FlashCode, SetHighForeCode, SetHighBothCode, NormalCode;
	label	ReverseCode, BoldCode, DelEolCode;

	label	CursorUpCode, CursorLeftCode, CursorAbsoluteCode;
	label	CursorDownCode, SaveCursCode, RestCursCode;
	label	CursorRightCode;

	Bool	BlankForCursorOne, BlankForTLCursor;

	int 	CursTopNum, CursLeftNum;

	Bool	Hilight;
	Bool	Blink;
	Bool	Dirty;

	TermTypeE TermType;

	void buildString(char *Buffer, uchar Attr);

#ifdef WINCIT
	TermWindowC *TW;
#endif

public:
#ifdef WINCIT
	TermCapC(TermWindowC *NewTW)
		{
		TW = NewTW;
		Clear();
		}

	void Clear(void)
		{
		TermWindowC *SaveTW = TW;
		memset(this, 0, sizeof(*this));
		TW = SaveTW;
		}
#else
	TermCapC(void)
		{
		Clear();
		}

	void Clear(void)
		{
		memset(this, 0, sizeof(*this));
		}
#endif

	Bool IsColor(void)				{ return (!!Color); }
	Bool IsHilight(void)			{ return (!!Hilight); }
	Bool IsBlink(void)				{ return (!!Blink); }
	Bool IsDirty(void)				{ return (!!Dirty); }
	Bool IsIBMExtended(void)		{ return (!!IBMExtended); }

	void SetHilight(Bool New)		{ Hilight = !!New; }
	void SetBlink(Bool New) 		{ Blink = !!New; }
	void SetDirty(Bool New) 		{ Dirty = !!New; }
	void SetColor(Bool New) 		{ Color = !!New; }
	void SetIBMExtended(Bool New)	{ IBMExtended = !!New; }

	void SendBackground(int Color);
	void SendForeground(int Color);
	void SendBoth(int Fore, int Back);
	void SendBlink(void);
	void SendBold(void);
	void SendNormal(void);
	void SendClearScreen(void);
	void SendDelEOL(void);
	void SendReverse(void);
	void SendUnderline(void);
	void SaveCursor(void);
	void RestoreCursor(void);
	void CursorUp(int Delta);
	void CursorDown(int Delta);
	void CursorLeft(int Delta);
	void CursorRight(int Delta);
	void CursorAbsolute(int Row, int Col);

	Bool IsBoldAvailable(void)
		{
		return (*BoldCode || Color);
		}

	TermTypeE GetTermType(void) 	{ return (TermType); }
	Bool IsANSI(void)				{ return (TermType == TT_ANSI); }

	const char *GetNormal(char *Buffer);
	const char *GetBold(char *Buffer);
	const char *GetInverse(char *Buffer);
	const char *GetBlink(char *Buffer);
	const char *GetUnderline(char *Buffer);

	const char *GetName(void);
	void putCode(const char *str);

	Bool Load(const char *Type);
	};

// TERM.CPP
Bool IsColorOrFormattingOnly(const char *Code);

// TERMOVL.CPP
const char *GetTermType(int Num, char *Buffer, int Length);

