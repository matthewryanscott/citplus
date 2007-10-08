// --------------------------------------------------------------------------
// Citadel: CWindows.CPP
//
// The main CitWindows routines.


#include "ctdl.h"
#pragma hdrstop
#define CWINMAIN
#include "cwindows.h"
#ifndef WINCIT
#include "statline.h"
#endif

#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// initCitWindows() 		Initializes everything
// deinitCitWindows()		Deinitializes everything


static Bool onDesktop(int x, int y);

struct PendingMessagesS
	{
	PendingMessagesS *next;

	EVENT evt;
	long param;
	int more;
	CITWINDOW far *wnd;
	};

WINDOWLIST *allWindows; 		// Z buffer: top is first

#ifdef WINCIT

CITWINDOW *makeCitWindow(Bool (*)(EVENT, long, int, CITWINDOW *),
		CITWINDOW *, const char *, WINDOWFLAGS, SRECT, void *, Bool)
	{
	return (NULL);
	}

CITWINDOW *getFocus(void)
	{
	return (NULL);
	}

void setFocus(CITWINDOW *)
	{
	}

void CitWindowsGetString(const char *, int, const char *,
		CITWINDOW *, int, void *, Bool)
	{
	}

void CitWindowsGetYN(const char *, int, CITWINDOW *, int, void *)
	{
	}

Bool defaultHandler(EVENT, long, int, CITWINDOW *)
	{
	return (FALSE);
	}

Bool buildClipArray(CITWINDOW *)
	{
	return (FALSE);
	}

void CitWindowClearLine(CITWINDOW *, int, int)
	{
	}

void CitWindowOutChr(CITWINDOW *, int, int, char, int)
	{
	}

void CitWindowOutStr(CITWINDOW *, int, int, const char *, int)
	{
	}

void freeClipArray(void)
	{
	}
#else

static CITWINDOW *hasFocus; 	// Window with focus (NULL for TTY)
static Bool cwInitialized;		// wow me.
static PendingMessagesS
		*PendingMessages;		// Pending messages...


// --------------------------------------------------------------------------
// initCitWindows(): Initializes everything.

Bool initCitWindows(void)
	{
	// somewhat icky.
	ScreenSaver.TurnOff();

	if (cwInitialized)
		{
		return (TRUE);
		}

	if (read_cw_messages())
		{
		if (save_screen())
			{
			CitWindowsVid = TRUE;
			logiScreen = saveBuffer;
			dgLogiScreen = logiScreen;
			showMouse();
			cwInitialized = TRUE;
			return (TRUE);
			}
		else
			{
			dump_cw_messages();
			}
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// deinitCitWindows(): Deinitializes everything.

static void deinitCitWindows(void)
	{
	if (cwInitialized)
		{
		assert(!allWindows);

		assert(getFocus() == NULL);

		hideMouse();

		CitWindowsVid = FALSE;
		CitWindowsKbd = FALSE;

		restore_screen();

		dump_cw_messages();

		// this is a bit of a kludge, by the way
		if (CurrentUser)
			{
			StatusLine.Update(WC_TWp);
			}

		cwInitialized = FALSE;
		}
	}


// --------------------------------------------------------------------------
// makeCitWindow(): Creates a window.

CITWINDOW *makeCitWindow(Bool (*func)(EVENT, long, int, CITWINDOW *),
		CITWINDOW *parent, const char *title, WINDOWFLAGS flags, SRECT ext,
		void *ld, Bool isCtrl)
	{
	CITWINDOW *newWindow;

	if (!allWindows)
		{
		if (!initCitWindows())
			{
			if (ld && !flags.nodiscard)
				{
				freeMemG(ld);
				}

			return (NULL);
			}
		}

	if ((newWindow = (CITWINDOW *) getMemG(sizeof (CITWINDOW), 1)) != NULL)
		{
		WINDOWLIST *newl;

		if (isCtrl || (newl = (WINDOWLIST *) insertLLNode((void **) &allWindows, 1,
				sizeof(*newl))) != NULL)
			{
			if (!isCtrl)
				{
				newl->wnd = newWindow;
				}

			CopyStringToBuffer(newWindow->Title, title);

			newWindow->parent = parent;
			newWindow->func = func;
			newWindow->flags = flags;

			newWindow->extents = ext;
			newWindow->invalid = ext;
			newWindow->oldext = ext;

			newWindow->borderColor = cfg.wattr;
			newWindow->titleColor = cfg.wattr;
			newWindow->textColor = cfg.attr;
			newWindow->LocalData = ld;

			if (parent)
				{
				WINDOWLIST *newc;

				if ((newc = (WINDOWLIST *) addLL((void **) (isCtrl ? &(parent->controls) :
						&(parent->children)), sizeof(*newc))) != NULL)
					{
					newc->wnd = newWindow;
					}
				}

			(func)(EVT_NEWWINDOW, 0, 0, newWindow);
			(func)(EVT_DRAWALL, 0, 0, newWindow);

			return (newWindow);
			}
		else
			{
			freeMemG(newWindow);

			if (ld && !flags.nodiscard)
				{
				freeMemG(ld);
				}

			if (!allWindows)
				{
				deinitCitWindows();
				}

			return (NULL);
			}
		}
	else
		{
		if (!allWindows)
			{
			deinitCitWindows();
			}

		if (ld && !flags.nodiscard)
			{
			freeMemG(ld);
			}

		return (NULL);
		}
	}

Bool IsWindowValid(CITWINDOW *wnd)
	{
	// verify that it is a real window
	for (WINDOWLIST *cur = allWindows; cur;
			cur = (WINDOWLIST *) getNextLL(cur))
		{
		assert(cur->wnd);

		if (cur->wnd == wnd)
			{
			return (TRUE);
			}

		for (WINDOWLIST *ctl = cur->wnd->controls; ctl;
				ctl = (WINDOWLIST *) getNextLL(ctl))
			{
			if (ctl->wnd == wnd)
				{
				return (TRUE);
				}
			}
		}

	return (FALSE);
	}
#endif


// --------------------------------------------------------------------------
// destroyCitWindow(): Destroys a window and its children.

#ifdef WINCIT
Bool destroyCitWindow(CITWINDOW *, Bool) { return (TRUE); }
#else
Bool destroyCitWindow(CITWINDOW *wnd, Bool ask)
	{
	WINDOWLIST *cur;

	assert(wnd);

	if (!IsWindowValid(wnd))
		{
		assert(FALSE);
		return (FALSE);
		}

	assert(wnd->func);

	// verify delete with window
	if (!ask || (wnd->func)(EVT_QDESTROY, 0, 0, wnd))
		{
		if (getFocus() == wnd)
			{
			setFocus(wnd->parent);
			}

		int i;

		// remove its children
		for (cur = wnd->children; cur; cur = wnd->children)
			{
			destroyCitWindow(cur->wnd, FALSE);
			}

		// and its controls
		for (cur = wnd->controls; cur; cur = wnd->controls)
			{
			destroyCitWindow(cur->wnd, FALSE);
			}

		// take out of parent's children list
		if (wnd->parent)
			{
			for (i = 1, cur = wnd->parent->children; cur;
					i++, cur = (WINDOWLIST *) getNextLL(cur))
				{
				if (cur->wnd == wnd)
					{
					deleteLLNode((void **) &(wnd->parent->children), i);
					break;
					}
				}

			// not a child - take out of parent's control list
			if (!cur)
				{
				for (i = 1, cur = wnd->parent->controls; cur;
						i++, cur = (WINDOWLIST *) getNextLL(cur))
					{
					if (cur->wnd == wnd)
						{
						deleteLLNode((void **) &(wnd->parent->controls), i);
						break;
						}
					}
				}

			// tell the parent...
			assert(wnd->parent->func);
			(wnd->parent->func)(EVT_CHILDDESTROY, (long) wnd, 0, wnd->parent);
			}

		// save its extents for later
		SRECT rect = wnd->extents;

		// Yes, we have already done this. However, if one of its children
		// had input focus, it would have gained focus since we lost tried
		// to get rid of it.
		if (getFocus() == wnd)
			{
			setFocus(wnd->parent);
			}

		// tell it that it is about to die
		(wnd->func)(EVT_DESTROY, 0, 0, wnd);

		// remove it
		if (wnd->LocalData && !wnd->flags.nodiscard)
			{
			freeMemG(wnd->LocalData);
			}

		freeMemG(wnd);

		// take it out of allWindows
		for (i = 1, cur = allWindows; cur;
				i++, cur = (WINDOWLIST *) getNextLL(cur))
			{
			if (cur->wnd == wnd)
				{
				deleteLLNode((void **) &allWindows, i);
				break;
				}
			}

		// if this is last window to be killed, take it all down else cleanup
		if (!allWindows)
			{
			deinitCitWindows();
			}
		else
			{
			redrawRect(rect);
			}

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

Bool defaultHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	// first, MSTAT might change active control...
	if (evt == EVT_MSTAT)
		{
		if (!wnd->oldB && (more & 1))		// new click
			{
			WINDOWLIST *l;

			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				SRECT r = l->wnd->extents;

				r.right += wnd->extents.left;
				r.bottom += wnd->extents.top;
				r.top += wnd->extents.top;
				r.left += wnd->extents.left;

				if (inRect(long_LO(param), long_HI(param), r))
					{
					if (l != wnd->controls)
						{
						(wnd->controls->wnd->func)(EVT_LOSTFOCUS,
								0, 0, wnd->controls->wnd);
						bringToTop(l->wnd, FALSE, &(wnd->controls));
						(l->wnd->func)(EVT_GOTFOCUS, 0, 0, l->wnd);
						}

					break;
					}
				}
			}

		wnd->oldB = more;
		}

	// let active control handle it first
	if (wnd->controls)
		{
		if ((wnd->controls->wnd->func)(evt, param, more, wnd->controls->wnd))
			{
			return (TRUE);
			}
		}

	// if it doesn't know how, then we do it
	switch (evt)
		{
		case EVT_ISCTRLID:
			{
			return (FALSE);
			}

		case EVT_NEWWINDOW:
			{
			(wnd->func)(EVT_CLEARINT, 0, 0, wnd);
			break;
			}

		case EVT_DRAWALL:
			{
			WINDOWLIST *cur;

			(wnd->func)(EVT_DRAWBORDER, param, 0, wnd);
			(wnd->func)(EVT_DRAWINT, param, 0, wnd);

			for (cur = wnd->controls; cur; cur = (WINDOWLIST *) getNextLL(cur))
				{
				(wnd->controls->wnd->func)
						(evt, param, more, wnd->controls->wnd);
				}

			break;
			}

		case EVT_CLEARINT:
		case EVT_DRAWINT:
			{
			WINDOWLIST *cur;

			if (wnd->flags.visible)
				{
				if (buildClipArray(wnd))
					{
					int y;

					for (y = 1; y <= wnd->extents.bottom - wnd->extents.top - 1;
							y++)
						{
						CitWindowClearLine(wnd, y, cfg.attr);
						}

					freeClipArray();
					}
				}

			for (cur = wnd->controls; cur; cur = (WINDOWLIST *) getNextLL(cur))
				{
				(wnd->controls->wnd->func)
						(evt, param, more, wnd->controls->wnd);
				}

			break;
			}

		case EVT_DRAWBORDER:
			{
			WINDOWLIST *cur;

			if (wnd->flags.visible)
				{
				// this is, btw, as much of a kludge as it looks like
				Bool oldMinimized = wnd->flags.minimized;
				wnd->flags.minimized = FALSE;

				if (buildClipArray(wnd))
					{
					static const char focus[] = {'É', '»', 'È', '¼', 'Í', 'º'};
					static const char notfocus[] = {'Ú', '¿', 'À', 'Ù', 'Ä', '³'};
					int i, right, bottom;
					const char *borderChars;

					borderChars = (getFocus() == wnd) ? focus : notfocus;
					right = wnd->extents.right - wnd->extents.left;
					bottom = wnd->extents.bottom - wnd->extents.top;

					// first, do corners
					CitWindowOutChr(wnd, 0, 0, borderChars[TOPLEFT], cfg.wattr);
					CitWindowOutChr(wnd, right, 0, borderChars[TOPRIGHT], cfg.wattr);
					CitWindowOutChr(wnd, right, bottom, borderChars[BOTTOMRIGHT], cfg.wattr);
					CitWindowOutChr(wnd, 0, bottom, borderChars[BOTTOMLEFT], cfg.wattr);

					// then, close button
					CitWindowOutChr(wnd, 1, 0, wnd->flags.close ? '' :
							borderChars[TOP], cfg.wattr);

					// minimize...
					CitWindowOutChr(wnd, right - 2, 0, wnd->flags.minimize ?
							oldMinimized ? '' : '' :
							borderChars[TOP], cfg.wattr);

					// maximize...
					CitWindowOutChr(wnd, right - 1, 0, wnd->flags.maximize ?
							wnd->flags.maximized ? '' : '' :
							borderChars[TOP], cfg.wattr);

					// resize...
					if (bottom - 1 > 0)
						{
						CitWindowOutChr(wnd, right, bottom - 1,
								wnd->flags.resize ? '' : borderChars[SIDE],
								cfg.wattr);
						}
					CitWindowOutChr(wnd, right - 1, bottom, wnd->flags.resize ?
							'' : borderChars[TOP], cfg.wattr);

					// top
					if (wnd->flags.showTitle && wnd->Title[0])
						{
						label title;
						int j;

						CitWindowOutChr(wnd, 2, 0, borderChars[TOP], cfg.wattr);

						CopyStringToBuffer(title, wnd->Title);

						i = strlen(title);
						j = right - 6;

						i = min(i, j);

						title[i] = 0;

						CitWindowOutStr(wnd, 3, 0, title, cfg.wattr);

						for (j = i + 3; j < right - 2; j++)
							{
							CitWindowOutChr(wnd, j, 0, borderChars[TOP], cfg.wattr);
							}
						}
					else
						{
						for (i = 2; i < right - 2; i++)
							{
							CitWindowOutChr(wnd, i, 0, borderChars[TOP], cfg.wattr);
							}
						}

					// bottom
					for (i = 1; i < right - 1; i++)
						{
						CitWindowOutChr(wnd, i, bottom, borderChars[TOP], cfg.wattr);
						}

					// left and right
					for (i = 1; i < bottom - 1; i++)
						{
						CitWindowOutChr(wnd, 0, i, borderChars[SIDE], cfg.wattr);
						CitWindowOutChr(wnd, right, i, borderChars[SIDE], cfg.wattr);
						}

					if (bottom - 1 > 0)
						{
						CitWindowOutChr(wnd, 0, bottom - 1,
								borderChars[SIDE], cfg.wattr);
						}

					freeClipArray();
					}

				wnd->flags.minimized = oldMinimized;
				}

			for (cur = wnd->controls; cur; cur = (WINDOWLIST *) getNextLL(cur))
				{
				(wnd->controls->wnd->func)
						(evt, param, more, wnd->controls->wnd);
				}

			break;
			}

		case EVT_MINIMIZE:
			{
			SRECT r = wnd->extents;
			Bool oldVis = wnd->flags.visible;
			Bool found = FALSE;
			Bool hadFocus = getFocus() == wnd;
			int i, j, y, len = strlen(wnd->Title) + 7;

			wnd->flags.visible = FALSE;

			if (hadFocus)
				{
				(wnd->func)(EVT_LOSTFOCUS, 0, 0, wnd);
				}

			wnd->oldext = r;

			for (y = scrollpos - 3; y > 0 && !found; y--)
				{
				for (i = 0; i < conCols - len; i = j)
					{
					for (j = i; j < i + len; j++)
						{
						if (!onDesktop(j, y) ||
								!onDesktop(j, y + 1))
							{
							break;
							}
						}

					if (j == i + len)
						{
						found = TRUE;
						break;
						}
					else
						{
						j++;
						}
					}
				}

			if (!found)
				{
				y = scrollpos - 3;
				i = 0;
				}
			else
				{
				y++;
				}

			wnd->extents.top = y;
			wnd->extents.bottom = y + 1;
			wnd->extents.left = i;
			wnd->extents.right = i + len;
			wnd->flags.minimized = TRUE;
			wnd->flags.maximized = FALSE;

			if (hadFocus)
				{
				(wnd->func)(EVT_GOTFOCUS, 0, 0, wnd);
				}

			wnd->flags.visible = oldVis;

			(wnd->func)(EVT_DRAWALL, 0, 0, wnd);
			redrawRect(r);
			break;
			}

		case EVT_MAXIMIZE:
			{
			wnd->oldext = wnd->extents;
			wnd->extents.top = 0;
			wnd->extents.bottom = scrollpos;
			wnd->extents.left = 0;
			wnd->extents.right = conCols - 1;
			wnd->flags.minimized = FALSE;
			wnd->flags.maximized = TRUE;
			(wnd->func)(EVT_DRAWALL, 0, 0, wnd);

			break;
			}

		case EVT_RESTORE:
			{
			SRECT r = wnd->extents;
			Bool hadFocus = getFocus() == wnd;

			if (hadFocus)
				{
				(wnd->func)(EVT_LOSTFOCUS, 0, 0, wnd);
				}

			wnd->extents = wnd->oldext;
			wnd->flags.minimized = FALSE;
			wnd->flags.maximized = FALSE;

			if (hadFocus)
				{
				(wnd->func)(EVT_GOTFOCUS, 0, 0, wnd);
				}

			(wnd->func)(EVT_DRAWALL, 0, 0, wnd);
			redrawRect(r);
			break;
			}

		case EVT_MSTAT:
			{
			if (more & 1)
				{
				int mx = long_LO(param);
				int my = long_HI(param);
				SRECT r = wnd->extents;

				if (wnd->flags.close && my == r.top && mx == r.left + 1)
					{
					destroyCitWindow(wnd, TRUE);
					}
				else if (wnd->flags.minimize && my == r.top &&
						mx == r.right - 2)
					{
					if (wnd->flags.minimized)
						{
						(wnd->func)(EVT_RESTORE, 0, 0, wnd);
						}
					else
						{
						(wnd->func)(EVT_MINIMIZE, 0, 0, wnd);
						}
					}
				else if (wnd->flags.maximize && my == r.top &&
						mx == r.right - 1)
					{
					if (wnd->flags.maximized)
						{
						(wnd->func)(EVT_RESTORE, 0, 0, wnd);
						}
					else
						{
						(wnd->func)(EVT_MAXIMIZE, 0, 0, wnd);
						}
					}
				else if (wnd->flags.resize && mx == r.left && my == r.top)
					{
					// resize here...
					initializeResize(wnd, FALSE, TOPLEFT);
					}
				else if (wnd->flags.resize && mx == r.right && my == r.top)
					{
					// resize here...
					initializeResize(wnd, FALSE, TOPRIGHT);
					}
				else if (wnd->flags.resize && mx == r.left && my == r.bottom)
					{
					// resize here...
					initializeResize(wnd, FALSE, BOTTOMLEFT);
					}
				else if (wnd->flags.resize && mx == r.right && my == r.bottom)
					{
					// resize here...
					initializeResize(wnd, FALSE, BOTTOMRIGHT);
					}
				else if (my == wnd->extents.top && mx != r.left &&
						mx != r.right)
					{
					// initializeMove handles wnd->flags.moveable
					initializeMove(wnd, FALSE);
					}
				}

			break;
			}

		case EVT_MAYFOCUS:
			{
			return (wnd->flags.visible);
			}

		case EVT_CHECKNEWEXT:
			{
			// Make sure new extents fit on screen.

			SRECT *rect = (SRECT *) param;

			if (rect->bottom > scrollpos)
				{
				rect->bottom = scrollpos;
				}

			if (rect->top < 0)
				{
				rect->top = 0;
				}

			if (rect->left < 0)
				{
				rect->left = 0;
				}

			if (rect->right >= conCols)
				{
				rect->right = conCols - 1;
				}

			break;
			}

		case EVT_INKEY:
			{
			if (param == 9) // tab - forward a control
				{
				if (wnd->controls && wnd->controls->next)
					{
					(wnd->controls->wnd->func)(EVT_LOSTFOCUS, 0, 0,
							wnd->controls->wnd);

					bringToTop(wnd->controls->next->wnd, FALSE,
							&(wnd->controls));

					(wnd->controls->wnd->func)(EVT_GOTFOCUS, 0, 0,
							wnd->controls->wnd);
					}
				}
			else if (param > 256)
				{
				switch (param >> 8)
					{
					case CTL_F2:
						{
						setFocus(NULL);
						break;
						}

					case ALT_F4:
						{
						if (wnd->flags.close)
							{
							destroyCitWindow(wnd, TRUE);
							}

						break;
						}

					case F6:
						{
						if (!wnd->flags.modal)
							{
							// Find us in allWindows
							WINDOWLIST *us;
							for (us = allWindows; us; us =
									(WINDOWLIST *) getNextLL(us))
								{
								if (us->wnd == wnd)
									{
									break;
									}
								}

							assert(us);

							if (us)
								{
								WINDOWLIST *cur = us;

								do
									{
									cur = (WINDOWLIST *) getNextLL(cur);

									if (!cur)
										{
										cur = allWindows;
										}

									WINDOWLIST *next = cur->next;
									if (!next)
										{
										next = allWindows;
										}

									if ((next->wnd->func)(EVT_MAYFOCUS, 0, 0,
											next->wnd))
										{
										cur = next;
										break;
										}
									} while (cur != us);

								bringToTop(cur->wnd, FALSE, NULL);
								setFocus(allWindows->wnd);

								redrawRect(allWindows->wnd->extents);
								}
							}

						break;
						}

					case F5:			// move
						{
						initializeMove(wnd, TRUE);
						break;
						}

					case CTL_F5:		// resize
						{
						initializeResize(wnd, TRUE, BOTTOMRIGHT);
						break;
						}

					case SFT_TAB:		// back a control
						{
						if (wnd->controls && wnd->controls->next)
							{
							WINDOWLIST *l;

							(wnd->controls->wnd->func)(EVT_LOSTFOCUS, 0, 0,
									wnd->controls->wnd);

							for (l = wnd->controls; l->next; l = (WINDOWLIST *) getNextLL(l));

							bringToTop(l->wnd, FALSE, &(wnd->controls));

							(wnd->controls->wnd->func)(EVT_GOTFOCUS, 0, 0,
									wnd->controls->wnd);
							}

						break;
						}

					default:
						{
						return (FALSE);
						}
					}
				}
			else
				{
				return (FALSE);
				}
			}
		}

	return (TRUE);
	}

CITWINDOW *getFocus(void)
	{
	return (hasFocus);
	}

void setFocus(CITWINDOW *wnd)
	{
	CITWINDOW *hadFocus = hasFocus;

	hasFocus = wnd;

	if (hadFocus)
		{
		assert(hadFocus->func);

		(hadFocus->func)(EVT_LOSTFOCUS, 0, 0, hadFocus);
		(hadFocus->func)(EVT_DRAWBORDER, 0, 0, hadFocus);
		}

	if (wnd)
		{
		assert(IsWindowValid(wnd));
		cursoff();

		assert(wnd->func);

		(wnd->func)(EVT_GOTFOCUS, 0, 0, wnd);
		(wnd->func)(EVT_DRAWBORDER, 0, 0, wnd);
		}
	else
		{
		curson();
		}

	CitWindowsKbd = wnd ? TRUE : FALSE;
	}

Bool overlapRect(SRECT r1, SRECT r2)
	{
	if (r1.top > r2.bottom || r2.top > r1.bottom || r1.left > r2.right ||
			r2.left > r1.right)
		{
		return (FALSE);
		}
	else
		{
		return (TRUE);
		}
	}

void bringToTop(CITWINDOW *wnd, Bool insert, WINDOWLIST **lst)
	{
	WINDOWLIST *cur, *cur2;

	if (!lst)
		{
		lst = &allWindows;
		}

	for (cur2 = NULL, cur = *lst; cur; cur2 = cur, cur = (WINDOWLIST *) getNextLL(cur))
		{
		if (cur->wnd == wnd)
			{
			break;
			}
		}

	if (cur && cur != *lst)
		{
		if (insert)
			{
			cur2->next = cur->next;
			cur->next = *lst;
			*lst = cur;
			}
		else
			{
			WINDOWLIST *w, *cur2;

			w = *lst;

			*lst = cur;

			for (cur2 = cur; cur2->next; cur2 = (WINDOWLIST *) getNextLL(cur2));

			cur2->next = w;

			for (; cur2->next != cur; cur2 = (WINDOWLIST *) getNextLL(cur2));

			cur2->next = NULL;
			}

		for (cur = *lst; cur; cur = (WINDOWLIST *) getNextLL(cur))
			{
			(cur->wnd->func)(EVT_DRAWALL, 0, 0, cur->wnd);
			}
		}
	}

Bool inRect(int x, int y, SRECT rect)
	{
	if (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom)
		{
		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

CITWINDOW *findWindow(int x, int y, Bool visible)
	{
	WINDOWLIST *cur;

	for (cur = allWindows; cur; cur = (WINDOWLIST *) getNextLL(cur))
		{
		if ((!visible || cur->wnd->flags.visible) &&
				inRect(x, y, cur->wnd->extents))
			{
			return (cur->wnd);
			}
		}

	return (NULL);
	}

static Bool onDesktop(int x, int y)
	{
	WINDOWLIST *lst;

	for (lst = allWindows; lst; lst = (WINDOWLIST *) getNextLL(lst))
		{
		if (lst->wnd->flags.visible && inRect(x, y, lst->wnd->extents))
			{
			return (FALSE);
			}
		}

	return (TRUE);
	}

void DispatchPendingMessages(void)
	{
	for (PendingMessagesS *cur = PendingMessages; cur;)
		{
		PendingMessagesS ToDispatch = *cur;

		cur = (PendingMessagesS *) getNextLL(cur);
		deleteLLNode((void **) &PendingMessages, 1);

		if (ToDispatch.wnd)
			{
			(*ToDispatch.wnd->func)(ToDispatch.evt, ToDispatch.param,
					ToDispatch.more, ToDispatch.wnd);
			}
		else
			{
			// System function...
			switch (ToDispatch.evt)
				{
				case EVT_DESTROY:
					{
					destroyCitWindow((CITWINDOW *) ToDispatch.param,
							ToDispatch.more);
					break;
					}

				default:
					{
					cPrintf(getcwmsg(21), ToDispatch.evt, ToDispatch.param,
							ToDispatch.more, ToDispatch.wnd);

					assert(FALSE);
					}
				}
			}
		}
	}

Bool QueueMessage(EVENT evt, long param, int more, CITWINDOW far *wnd)
	{
	PendingMessagesS *cur = (PendingMessagesS *) addLL((void **)
			&PendingMessages, sizeof(PendingMessagesS));

	if (cur)
		{
		cur->evt = evt;
		cur->param = param;
		cur->more = more;
		cur->wnd = wnd;

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}
#endif
