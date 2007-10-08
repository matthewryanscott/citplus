// --------------------------------------------------------------------------
// Citadel: ConSysop.CPP
//
// The main console sysop (Control+F6) implementation stuff.

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "fscfg.h"
#include "consysop.h"
#include "config.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// doConsoleSysop	Ctl+F6
// initConSysop 	Initialize the windows

static CITWINDOW *conSysMain;		// Our main window
static discardable *conSysMenus;	// Our menus

static Bool initConSysop(void);
static void undoConsoleSysop(void);

void doConsoleSysop(void)
	{
	if (saveBuffer != NULL && (conSysMenus != NULL || (conSysMenus = readData(7)) != NULL))
		{
		if (!conSysMain)
			{
			read_sys_messages();

			if (initConSysop())
				{
				return;
				}
			else
				{
				cOutOfMemory(24);
				}
			}

		undoConsoleSysop();
		}
	}

static void undoConsoleSysop(void)
	{
	if (destroyCitWindow(conSysMain, TRUE))
		{
		dump_sys_messages();
		discardData(conSysMenus);
		conSysMenus = NULL;
		conSysMain = NULL;
		}
	}

static Bool MainKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSMM_EXIT:
		case MK_ESC:
			{
			// exit console sysop stuff
			undoConsoleSysop();
			break;
			}

		case CSMM_HALLS:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			label HallNoAnsi;
			CopyStringToBuffer(HallNoAnsi, cfg.Uhalls_nym);
			stripansi(HallNoAnsi);

			CITWINDOW *newWindow = MakeMenuWindow(wnd, (const char **) conSysMenus->next->aux, wnd->extents.top + 2,
					wnd->extents.left + 2, HallKeyHandler, HallControlHandler, NULL, HallNoAnsi);

			if (newWindow)
				{
				listerData *ld;

				if ((ld = (listerData *) getMemG(sizeof(listerData), 1)) != NULL)
					{
					ld->lister = hallLister;

					SRECT rect;
					rect.top = newWindow->extents.top;
					rect.bottom = newWindow->extents.bottom;
					rect.left = newWindow->extents.right + 1;
					rect.right = rect.left + LABELSIZE + 2;

					WINDOWFLAGS flags = newWindow->flags;
					flags.resize = TRUE;

					char Title[128];
					sprintf(Title, getsysmsg(197), HallNoAnsi);

					makeCitWindow(listHandler, newWindow, Title, flags, rect, ld, FALSE);
					}

				setFocus(newWindow);
				}

			break;
			}

		case CSMM_ROOMS:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			label RoomNoAnsi;
			CopyStringToBuffer(RoomNoAnsi, cfg.Urooms_nym);
			stripansi(RoomNoAnsi);

			CITWINDOW *newWindow = MakeMenuWindow(wnd, (const char **) conSysMenus->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, RoomKeyHandler, RoomControlHandler, NULL, RoomNoAnsi);

			if (newWindow)
				{
				listerData *ld;

				if ((ld = (listerData *) getMemG(sizeof(listerData), 1)) != NULL)
					{
					ld->lister = roomLister;

					SRECT rect;
					rect.top = newWindow->extents.top;
					rect.bottom = newWindow->extents.bottom;
					rect.left = newWindow->extents.right + 1;
					rect.right = rect.left + LABELSIZE + 2;

					WINDOWFLAGS flags = newWindow->flags;
					flags.resize = TRUE;

					char Title[128];
					sprintf(Title, getsysmsg(197), RoomNoAnsi);

					makeCitWindow(listHandler, newWindow, Title, flags, rect, ld, FALSE);
					}

				setFocus(newWindow);
				}

			break;
			}

		case CSMM_USERS:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			label UserNoAnsi;
			CopyStringToBuffer(UserNoAnsi, cfg.Uusers_nym);
			stripansi(UserNoAnsi);

			CITWINDOW *newWindow = MakeMenuWindow(wnd, (const char **) conSysMenus->next->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, UserKeyHandler, UserControlHandler, NULL, UserNoAnsi);

			if (newWindow)
				{
				listerData *ld;

				if ((ld = (listerData *) getMemG(sizeof(listerData), 1)) != NULL)
					{
					ld->lister = userLister;

					SRECT rect;
					rect.top = newWindow->extents.top;
					rect.bottom = newWindow->extents.bottom;
					rect.left = newWindow->extents.right + 1;
					rect.right = rect.left + LABELSIZE + 2;

					WINDOWFLAGS flags = newWindow->flags;
					flags.resize = TRUE;

					char Title[128];
					sprintf(Title, getsysmsg(197), UserNoAnsi);

					makeCitWindow(listHandler, newWindow, Title, flags, rect, ld, FALSE);
					}

				setFocus(newWindow);
				}

			break;
			}

		case CSMM_GROUPS:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			label GrpNoAnsi;
			CopyStringToBuffer(GrpNoAnsi, cfg.Ugroups_nym);
			stripansi(GrpNoAnsi);

			CITWINDOW *newWindow = MakeMenuWindow(wnd, (const char **) conSysMenus->next->next->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, GroupKeyHandler, GroupControlHandler, NULL, GrpNoAnsi);

			if (newWindow)
				{
				listerData *ld;

				if ((ld = (listerData *) getMemG(sizeof(listerData), 1)) != NULL)
					{
					ld->lister = groupLister;

					SRECT rect;
					rect.top = newWindow->extents.top;
					rect.bottom = newWindow->extents.bottom;
					rect.left = newWindow->extents.right + 1;
					rect.right = rect.left + LABELSIZE + 2;

					WINDOWFLAGS flags = newWindow->flags;
					flags.resize = TRUE;

					char Title[128];
					sprintf(Title, getsysmsg(197), GrpNoAnsi);

					makeCitWindow(listHandler, newWindow, Title, flags, rect, ld, FALSE);
					}

				setFocus(newWindow);
				}

			break;
			}

		case CSMM_MESSAGES:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			break;
			}

		case CSMM_HELPFILES:
			{
			CITWINDOW *newWindow = MakeMenuWindow(wnd, (const char **) conSysMenus->next->next->next->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, HelpKeyHandler, NULL, NULL, getsysmsg(224));

			if (newWindow)
				{
				setFocus(newWindow);
				}

			break;
			}

		case CSMM_SYSMON:
			{
			CITWINDOW *newWindow = MakeMenuWindow(wnd,
					(const char **) conSysMenus->next->next->next->next->next->next->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, SysMonKeyHandler, NULL, NULL, getsysmsg(52));

			if (newWindow)
				{
				setFocus(newWindow);
				}

			break;
			}

		case CSMM_CONFIG:
			{
			initFullScreenConfig(wnd);
			break;
			}

		case CSMM_DOSUTIL:
			{
			CITWINDOW *newWindow = MakeMenuWindow(wnd,
					(const char **) conSysMenus->next->next->next->next->next->next->next->next->aux,
					wnd->extents.top + 2, wnd->extents.left + 2, DOSUtilKeyHandler, NULL, NULL, getsysmsg(225));

			if (newWindow)
				{
				listerData *ld;

				if ((ld = (listerData *) getMemG(sizeof(listerData), 1)) != NULL)
					{
					ld->lister = fileLister;

					SRECT rect;
					rect.top = newWindow->extents.top;
					rect.bottom = newWindow->extents.bottom;
					rect.left = newWindow->extents.right + 1;
					rect.right = rect.left + LABELSIZE + 2;

					WINDOWFLAGS flags = newWindow->flags;
					flags.resize = TRUE;

					makeCitWindow(listHandler, newWindow, getsysmsg(226), flags, rect, ld, FALSE);
					}

				setFocus(newWindow);
				}

			break;
			}
		}

	return (TRUE);
	}


// --------------------------------------------------------------------------
// initConSysop(): Initialize the windows.
//
// Return value:
//	TRUE: Started
//	FALSE: Not

Bool initConSysop(void)
	{
	conSysMain = MakeMenuWindow(NULL, (const char **) conSysMenus->aux, 0, 0, MainKeyHandler, NULL, NULL, getsysmsg(227));

	if (conSysMain)
		{
		setFocus(conSysMain);
		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

Bool listHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			(((listerData *)(wnd->LocalData))->lister)(wnd, LST_NEW, NULL, 0);
			(((listerData *)(wnd->LocalData))->lister)(wnd, LST_REFRESH, NULL, 0);
			break;
			}

		case EVT_DESTROY:
			{
			(((listerData *)(wnd->LocalData))->lister)(wnd, LST_DESTROY, NULL, 0);
			break;
			}

		case EVT_DRAWINT:
			{
			(((listerData *)(wnd->LocalData))->lister)(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case EVT_INKEY:
			{
			if (param == 9) // tab
				{
				bringToTop(wnd->parent, TRUE, NULL);
				setFocus(wnd->parent);
				return (TRUE);
				}
			else if (param > 256)
				{
				switch (param >> 8)
					{
					case CURS_RIGHT:
					case CURS_LEFT:
					case SFT_TAB:
						{
						bringToTop(wnd->parent, TRUE, NULL);
						setFocus(wnd->parent);
						return (TRUE);
						}

					case CURS_DOWN:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_DOWN, NULL, 0);
						return (TRUE);
						}

					case CURS_UP:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_UP, NULL, 0);
						return (TRUE);
						}

					case PGUP:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_PGUP, NULL, 0);
						return (TRUE);
						}

					case PGDN:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_PGDN, NULL, 0);
						return (TRUE);
						}

					case CURS_HOME:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_HOME, NULL, 0);
						return (TRUE);
						}

					case CURS_END:
						{
						(((listerData *)(wnd->LocalData))->lister) (wnd, LST_END, NULL, 0);
						return (TRUE);
						}
					}
				}

			return (defaultHandler(evt, param, more, wnd));
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

#endif
