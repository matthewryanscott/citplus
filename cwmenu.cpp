/* -------------------------------------------------------------------- */
/*	CWMENU.CPP						Citadel 							*/
/* -------------------------------------------------------------------- */
/*					The CitWindows menu routines.						*/
/* -------------------------------------------------------------------- */
#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"

/* -------------------------------------------------------------------- */
/*								Contents								*/
/* -------------------------------------------------------------------- */
/*	MakeMenuWindow			Make a menu window							*/
/* -------------------------------------------------------------------- */

typedef struct
	{
	const char **Menu;							// The menu
	int curSelection;							// Currently selected entry
	int numEntries; 							// How many entries
	Bool (*KeyHandler)(CITWINDOW *, int);		// What to do
	void (*ControlHandler)(CITWINDOW *, long);	// What to do
	void (*OnDestroy)(void);					// What to do
	} menuWindowData;

static void MenuWindowDrawer(CITWINDOW *wnd)
	{
	if (wnd->flags.visible)
		{
		if (buildClipArray(wnd))
			{
			int i;
			menuWindowData *menuData = (menuWindowData *) wnd->LocalData;

			for (i = 0; *menuData->Menu[i]; i++)
				{
				int j, x;

				if (i >= wnd->extents.bottom - wnd->extents.top - 1)
					{
					break;
					}

				for (j = 0, x = 1; menuData->Menu[i][j]; j++, x++)
					{
					if (menuData->Menu[i][j] == '&')
						{
						if (menuData->Menu[i][j+1] == '&')
							{
							CitWindowOutChr(wnd, x, i + 1,
									menuData->Menu[i][j],
									menuData->curSelection == i ?
									cfg.cattr : cfg.attr);
							j++;
							}
						else
							{
							CitWindowOutChr(wnd, x, i + 1,
									menuData->Menu[i][++j], cfg.uttr);
							}
						}
					else
						{
						CitWindowOutChr(wnd, x, i + 1, menuData->Menu[i][j],
								menuData->curSelection == i ?
								cfg.cattr : cfg.attr);
						}
					}

				for (; x < wnd->extents.right - wnd->extents.left; x++)
					{
					CitWindowOutChr(wnd, x, i + 1, ' ',
							menuData->curSelection == i ?
							cfg.cattr : cfg.attr);
					}
				}

			freeClipArray();
			}
		}
	}

static int ProcessMenuWindowMouse(CITWINDOW *wnd, long param, int more)
	{
	menuWindowData *ld = (menuWindowData *) wnd->LocalData;

	if ((more & 1) && !wnd->oldB)			// new click
		{
		int i = long_LO(param);

		if (i != wnd->extents.left && i != wnd->extents.right)
			{
			i = long_HI(param);

			if (i != wnd->extents.top && i != wnd->extents.bottom)
				{
				wnd->oldB = more;
				ld->curSelection = i - wnd->extents.top - 1;
				return (MK_REDRAW);
				}
			}
		}
	else if ((wnd->oldB & 1) && !more)		// just released
		{
		int i = long_HI(param);

		wnd->oldB = more;

		if ((i - wnd->extents.top - 1) == ld->curSelection)
			{
			return (ld->curSelection);
			}
		else
			{
			return (MK_NULL);
			}
		}

	wnd->oldB = more;

	// handle stuff like move and resize and whatever CitWindows has
	defaultHandler(EVT_MSTAT, param, more, wnd);
	return (MK_NULL);
	}

static int ProcessMenuWindowKey(CITWINDOW *wnd, long inkey, int more)
	{
	if (inkey == ESC)			// esc
		{
		return (MK_ESC);
		}
	if (inkey == '\r')          // cr
		{
		return (((menuWindowData *)(wnd->LocalData))->curSelection);
		}
	else if (inkey == '\t')     // tab
		{
		if (wnd->children)
			{
			bringToTop(wnd->children->wnd, TRUE, NULL);
			setFocus(wnd->children->wnd);
			}

		return (MK_NULL);
		}
	else if (inkey < 256)
		{
		menuWindowData *menuData = (menuWindowData *) wnd->LocalData;

		inkey = toupper((int) inkey);

		for (int i = 0; *menuData->Menu[i]; i++)
			{
			for (int j = 0; menuData->Menu[i][j]; j++)
				{
				if (menuData->Menu[i][j] == '&')
					{
//cPrintf("(%c)",(menuData->Menu)[i][j+1]);
					if (menuData->Menu[i][j+1] == '&')
						{
						j++;
						}
					else if (inkey == toupper((menuData->Menu)[i][j+1]))
						{
						menuData->curSelection = i;
						(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
						return (i);
						}

					break;
					}
				}
			}
		}
	else
		{
		switch (inkey >> 8)
			{
			case CURS_HOME:
				{
				((menuWindowData *)(wnd->LocalData))->curSelection = 0;
				return (MK_REDRAW);
				}

			case CURS_UP:
				{
				((menuWindowData *)(wnd->LocalData))->curSelection--;

				if (((menuWindowData *)(wnd->LocalData))->curSelection < 0)
					{
					((menuWindowData *)(wnd->LocalData))->curSelection =
							((menuWindowData *)(wnd->LocalData))->numEntries - 1;
					}

				return (MK_REDRAW);
				}

			case CURS_END:
				{
				((menuWindowData *)(wnd->LocalData))->curSelection =
						((menuWindowData *)(wnd->LocalData))->numEntries - 1;

				return (MK_REDRAW);
				}

			case CURS_DOWN:
				{
				((menuWindowData *)(wnd->LocalData))->curSelection++;

				if (((menuWindowData *)(wnd->LocalData))->curSelection >=
						((menuWindowData *)(wnd->LocalData))->numEntries)
					{
					((menuWindowData *)(wnd->LocalData))->curSelection = 0;
					}

				return (MK_REDRAW);
				}

			case CURS_RIGHT:
			case CURS_LEFT:
			case SFT_TAB:
				{
				if (wnd->children)
					{
					bringToTop(wnd->children->wnd, TRUE, NULL);
					setFocus(wnd->children->wnd);
					}

				return (MK_NULL);
				}
			}
		}

	// handle stuff like F6 and Alt-F4 and whatever CitWindows has
	defaultHandler(EVT_INKEY, inkey, more, wnd);
	return (MK_NULL);
	}

static Bool MenuWindowHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	int i = MK_INVALID;

	switch (evt)
		{
		case EVT_DRAWINT:
			{
			MenuWindowDrawer(wnd);
			break;
			}

		case EVT_DESTROY:
			{
			if (((menuWindowData *) (wnd->LocalData))->OnDestroy)
				{
				(*((menuWindowData *)(wnd->LocalData))->OnDestroy)();
				}

			break;
			}

		case EVT_CTRLRET:
			{
			if (((menuWindowData *) (wnd->LocalData))->ControlHandler)
				{
				(*((menuWindowData *) (wnd->LocalData))->ControlHandler)(wnd, param);
				}

			break;
			}

		case EVT_MSTAT:
			i = ProcessMenuWindowMouse(wnd, param, more);

		case EVT_INKEY:
			{
			if (i == MK_INVALID)
				{
				i = ProcessMenuWindowKey(wnd, param, more);
				}

			if (i == MK_REDRAW)
				{
				(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
				}
			else if (!(*((menuWindowData *) (wnd->LocalData))->KeyHandler)(wnd, i))
				{
				destroyCitWindow(wnd, FALSE);
				}

			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

/* -------------------------------------------------------------------- */
/*	MakeMenuWindow			Make a menu window							*/
/* -------------------------------------------------------------------- */
CITWINDOW *MakeMenuWindow(CITWINDOW *Parent, const char **Menu,
		int Top, int Left, Bool (*KeyHandler)(CITWINDOW *, int),
		void (*ControlHandler)(CITWINDOW *, long), void (*OnDestroy)(void),
		const char *MenuTitle)
	{
	menuWindowData *ld = new menuWindowData;

	if (ld != NULL)
		{
		WINDOWFLAGS flags;
		SRECT rect;
		int menuLines, menuWidth;

		memset(&flags, 0, sizeof(flags));

		flags.visible = TRUE;
		flags.showTitle = TRUE;
		flags.moveable = TRUE;
		flags.minimize = TRUE;

		for (menuWidth = 0, menuLines = 0; *Menu[menuLines]; menuLines++)
			{
			menuWidth = max(menuWidth, strlen(Menu[menuLines]));
			}

		rect.top = Top;
		rect.left = Left;
		rect.bottom = Top + menuLines + 1;
		rect.right = Left + menuWidth + 1;

		ld->numEntries = menuLines;
		ld->Menu = Menu;
		ld->KeyHandler = KeyHandler;
		ld->ControlHandler = ControlHandler;
		ld->OnDestroy = OnDestroy;
		ld->curSelection = 0;

		CITWINDOW *ourWindow;

		if ((ourWindow = makeCitWindow(MenuWindowHandler, Parent, MenuTitle,
				flags, rect, ld, FALSE)) != NULL)
			{
			setFocus(ourWindow);
			return (ourWindow);
			}
		else
			{
			return (NULL);
			}
		}
	else
		{
		return (NULL);
		}
	}

#endif
