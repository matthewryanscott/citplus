// --------------------------------------------------------------------------
// Citadel: CWindows.H
//
// The CitWindows structures.


#define CWINDOWS_H

struct SRECT
	{
	int top, left, bottom, right;
	};

enum EVENT
	{
	EVT_CREATE, 	EVT_DRAW,		EVT_DESTROY,	EVT_QDESTROY,
	EVT_LOSTFOCUS,	EVT_DRAWBORDER, EVT_GOTFOCUS,	EVT_DRAWINT,
	EVT_DRAWALL,	EVT_CLEARINT,	EVT_NEWWINDOW,	EVT_INKEY,
	EVT_MSTAT,		EVT_USERDEF,	EVT_CTRLRET,	EVT_ISCTRLID,
	EVT_MINIMIZE,	EVT_MAXIMIZE,	EVT_RESTORE,	EVT_CHILDDESTROY,
	EVT_MAYFOCUS,	EVT_CHECKNEWEXT,
	};

struct CONTROLINFO
	{
	int id;
	void far *ptr;
	void far *more;
	};

struct GSINFO
	{
	CONTROLINFO ci;
	const char far *pmt;
	int len;
	Bool usenopwecho;
	};

struct YNINFO
	{
	CONTROLINFO ci;
	char far *pmt;
	};

struct TGINFO
	{
	CONTROLINFO ci;
	const char far *on;
	const char far *off;
	Bool focused;
	Bool clicked;
	};

struct MSTINFO
	{
	CONTROLINFO ci;
	int num;
	strList far *sl;
	Bool focused;
	Bool clicked;
	};

struct TEINFO
	{
	CONTROLINFO ci;
	int start;
	int end;
	int cur;
	int len;
	Bool ins;
	Bool focused;
	Bool usenopwecho;
	};

struct BTINFO
	{
	CONTROLINFO ci;
	Bool focused;
	Bool clicked;
	};

struct WINDOWFLAGS
	{
	Bool visible : 1;			// show it
	Bool showTitle : 1; 		// show its title
	Bool resize : 1;			// can resize it
	Bool close : 1; 			// can close it
	Bool moveable : 1;			// can move it
	Bool minimize : 1;			// can minimize it
	Bool maximize : 1;			// can maximize it
	Bool nodiscard : 1; 		// don't discard LocalData when destroying it
	Bool modal : 1; 			// modal window?
	Bool minimized : 1; 		// has it been?
	Bool maximized : 1; 		// has it been?
	};

struct WINDOWLIST
	{
	WINDOWLIST far *next;
	CITWINDOW far *wnd;
	};

struct CITWINDOW
	{
	SRECT extents, oldext;
	SRECT invalid;

	CITWINDOW far *parent;

	WINDOWLIST far *children;

	WINDOWLIST far *controls;

	int borderColor;
	int titleColor;
	int textColor;

	int oldB;

	WINDOWFLAGS flags;

	void far *LocalData;

	Bool (far *func)(EVENT, long, int, CITWINDOW far *);

	label Title;
	};

enum CORNERS
	{
	TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT, TOP, SIDE
	};

extern WINDOWLIST far *allWindows;			// Z buffer: top is first

// CWINCTL.CPP
void initializeResize(CITWINDOW *wnd, Bool show, CORNERS c);
void initializeMove(CITWINDOW *wnd, Bool show);


// CWINDOWS.CPP
Bool initCitWindows(void);
CITWINDOW far *makeCitWindow(Bool (far *func)(EVENT, long, int, CITWINDOW far *),
		CITWINDOW far *parent, const char far *title, WINDOWFLAGS flags, SRECT ext,
		void far *ld, Bool isCtrl);
Bool destroyCitWindow(CITWINDOW far *wnd, Bool ask);
Bool defaultHandler(EVENT evt, long param, int more, CITWINDOW far *wnd);
CITWINDOW far *getFocus(void);
void setFocus(CITWINDOW far *wnd);
Bool buildClipArray(CITWINDOW far *wnd);
void freeClipArray(void);
void CitWindowOutChr(CITWINDOW far *wnd, int x, int y, char chr, int color);
void CitWindowOutStr(CITWINDOW far *wnd, int x, int y, const char far *str, int color);
void CitWindowClearLine(CITWINDOW far *wnd, int line, int color);
Bool overlapRect(SRECT r1, SRECT r2);
void redrawRect(SRECT rect);
void RedrawAll(void);
void bringToTop(CITWINDOW far *wnd, Bool insert = FALSE, WINDOWLIST far * far *lst = NULL);
CITWINDOW far *findWindow(int x, int y, Bool visible);
Bool inRect(int x, int y, SRECT rect);
void CitWindowsGetString(const char far *prompt, int lim, const char far *dfault,
		CITWINDOW far *parent, int ctrlID, void far *more, Bool UseNoPWEcho);
void GetStringFromConsole(const char far *prompt, int lim,
		const char far *dfault, void (far *func)(const char far *),
		Bool UseNoPWEcho);
void CitWindowsGetYN(const char far *pmt, int dfault, CITWINDOW far *parent, int ctrlID, void far *more);
void GetYNFromConsole(const char far *prompt, int dfault,
		void (far *func)(int));
void cdecl CitWindowsError(CITWINDOW far *parent, const char far *fmt, ...);
void cdecl CitWindowsNote(CITWINDOW far *parent, const char far *fmt, ...);
Bool teHandler(EVENT evt, long param, int more, CITWINDOW far *wnd);
Bool tgHandler(EVENT evt, long param, int more, CITWINDOW far *wnd);
Bool mstHandler(EVENT evt, long param, int more, CITWINDOW far *wnd);
Bool btHandler(EVENT evt, long param, int more, CITWINDOW far *wnd);
CITWINDOW far * cdecl CitWindowsMsg(CITWINDOW far *parent, const char far *fmt, ...);
void DispatchPendingMessages(void);
Bool QueueMessage(EVENT evt, long param, int more, CITWINDOW far *wnd);
Bool IsWindowValid(CITWINDOW *wnd);

// CWMENU.CPP
#define MK_INVALID	-4			// never a valid result
#define MK_REDRAW	-3			// redraw
#define MK_ESC		-2			// ESC hit - exit menu
#define MK_NULL 	-1			// do nothing

CITWINDOW *MakeMenuWindow(CITWINDOW *Parent, const char **Menu,
		int Top, int Left, Bool (*KeyHandler)(CITWINDOW *, int),
		void (*ControlHandler)(CITWINDOW *, long), void (*OnDestroy)(void),
		const char *MenuTitle);
