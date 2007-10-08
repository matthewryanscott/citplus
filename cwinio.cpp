// --------------------------------------------------------------------------
// Citadel: CWinIO.CPP
//
// The I/O CitWindows routines.


#include "ctdl.h"
#pragma hdrstop

#ifdef WINCIT
void CitWindowsTTYUpdate(int, int) {}
void CitWindowsProcessMouse(void) {}
void CitWindowsProcessKbd(void) {}
#else
#include "cwindows.h"


// --------------------------------------------------------------------------
// Contents

static SRECT TTYExtents;
Bool *clipArray;				// Used for clipping output

Bool buildClipArray(CITWINDOW *wnd)
	{
	int size;
	int left, right, top, bottom;
	SRECT extents;

	if (clipArray)
		{
		freeMemG(clipArray);
		}

	if (wnd)
		{
		extents = wnd->extents;
		}
	else
		{
		extents = TTYExtents;
		}

	// speed
	left = extents.left;
	right = extents.right;
	top = extents.top;
	bottom = extents.bottom;

	size = (right - left + 1) * (bottom - top + 1);

	clipArray = (Bool *) getMemG(sizeof(*clipArray), size);

	if (clipArray)
		{
		WINDOWLIST *lst;

		// assume you can write everywhere to start with
		fastsetw(clipArray, TRUE, size);

		for (lst = allWindows; lst; lst = (WINDOWLIST *) getNextLL(lst))
			{
			int x, y, nleft, nright, ntop, nbottom;
			int cleft, cright, ctop, cbottom;

			if (lst->wnd == wnd)
				{
				break;
				}

			if (lst->wnd->flags.visible &&
					overlapRect(lst->wnd->extents, extents))
				{
				nleft = lst->wnd->extents.left;
				nright = lst->wnd->extents.right;
				ntop = lst->wnd->extents.top;
				nbottom = lst->wnd->extents.bottom;

				cleft = max(nleft, left);
				cleft = min(cleft, right);

				cright = max(nright, left);
				cright = min(cright, right);

				ctop = max(ntop, top);
				ctop = min(ctop, bottom);

				cbottom = max(nbottom, top);
				cbottom = min(cbottom, bottom);

				for (y = ctop; y <= cbottom; y++)
					{
					int j = (y - top) * (right - left + 1);

					for (x = cleft; x <= cright; x++)
						{
						clipArray[j + x - left] = FALSE;
						}
					}
				}
			}

		hideMouse();
		}

	return (clipArray != NULL);
	}

void freeClipArray(void)
	{
	if (clipArray)
		{
		freeMemG(clipArray);
		}

	showMouse();

	clipArray = NULL;
	}

void CitWindowOutChr(CITWINDOW *wnd, int x, int y, char chr, int color)
	{
	if (!wnd->flags.minimized)
		{
		int left = wnd->extents.left;
		int top = wnd->extents.top;
		int height = wnd->extents.bottom - top + 1;
		int width = wnd->extents.right - left + 1;

		if (x >= 0 && x <= width && y >= 0 && y <= height)
			{
			if (clipArray[y * width + x])
				{
				int soffs = ((top + y) * conCols + left + x) << 1;

				physScreen[soffs++] = chr;
				physScreen[soffs] = (char) color;
				}
			}
		}
	}

void CitWindowOutStr(CITWINDOW *wnd, int x, int y, const char *str, int color)
	{
	if (!wnd->flags.minimized)
		{
		int left = wnd->extents.left;
		int top = wnd->extents.top;
		int height = wnd->extents.bottom - top + 1;
		int width = wnd->extents.right - left + 1;

		if (x >= 0 && x <= width && y >= 0 && y <= height)
			{
			const char *thisChar;
			int offs = y * width + x;
			int soffs = ((top + y) * conCols + left + x) << 1;

			for (thisChar = str; *thisChar && x < width - 1; thisChar++, x++, offs++)
				{
				if (*thisChar == '\t')
					{
					do
						{
						if (x < width - 1)
							{
							if (clipArray[offs++])
								{
								physScreen[soffs++] = ' ';
								physScreen[soffs++] = (char) color;
								}
							else
								{
								soffs += 2;
								}
							}
						} while (x++ % 8);

					x--;
					offs--;
					}
				else
					{
					if (clipArray[offs])
						{
						physScreen[soffs++] = *thisChar;
						physScreen[soffs++] = (char) color;
						}
					else
						{
						soffs += 2;
						}
					}
				}
			}
		}
	}

void CitWindowClearLine(CITWINDOW *wnd, int line, int color)
	{
	int left = wnd->extents.left;
	int top = wnd->extents.top;
	int height = wnd->extents.bottom - top + 1;
	int width = wnd->extents.right - left + 1;

	if (line >= 0 && line <= height)
		{
		int x = 1;
		int offs = line * width + x;
		int soffs = ((top + line) * conCols + left + x) << 1;

		for (; x < width - 1; x++, offs++)
			{
			if (clipArray[offs])
				{
				physScreen[soffs++] = ' ';
				physScreen[soffs++] = (char) color;
				}
			else
				{
				soffs += 2;
				}
			}
		}
	}

void CitWindowsTTYUpdate(int sline, int eline)
	{
	TTYExtents.left = 0;
	TTYExtents.right = conCols - 1;
	TTYExtents.top = sline;
	TTYExtents.bottom = eline;

	if (buildClipArray(NULL))
		{
		int x, y;
		uint offs, soffs;

		offs = 0;
		soffs = (sline * conCols) << 1;

		for (y = sline; y <= eline; y++)
			{
			for (x = 0; x < conCols; x++)
				{
				if (clipArray[offs++])
					{
					physScreen[soffs] = logiScreen[soffs++];
					physScreen[soffs] = logiScreen[soffs++];
					}
				else
					{
					soffs += 2;
					}
				}
			}

		freeClipArray();
		}
	}

void CitWindowsProcessMouse(void)
	{
	DispatchPendingMessages();

	if (mouseEvent)
		{
		static int oMB;
		CITWINDOW *wnd;

		if (mouseButtons && !oMB)
			{
			wnd = findWindow(mouseX / 8, mouseY / 8, TRUE);

			if (getFocus() == NULL)
				{
				if (!allWindows->wnd->flags.modal || wnd == allWindows->wnd)
					{
					setFocus(wnd);
					}
				}
			else if (!wnd || !allWindows->wnd->flags.modal)
				{
				bringToTop(wnd, TRUE, NULL);
				setFocus(wnd);
				}
			}

		if ((wnd = getFocus()) != NULL)
			{
			(wnd->func)(EVT_MSTAT, long_JOIN(mouseX / 8, mouseY / 8),
					mouseButtons, wnd);
			}

		oMB = mouseButtons;
		mouseEvent = FALSE;
		}
	}

void CitWindowsProcessKbd(void)
	{
	DispatchPendingMessages();

	if ((*statcon)())
		{
		const int inkey = (*getcon)();

		CITWINDOW *wnd = getFocus();

		if (wnd)
			{
			assert(wnd->func);

			(wnd->func)(EVT_INKEY, inkey, 0, wnd);
			}
		}
	}

// this whole thing can be sped up, but i don't want to think that hard now
void redrawRect(SRECT rect)
	{
	WINDOWLIST *rd = NULL, *cur;

	for (cur = allWindows; cur; cur = (WINDOWLIST *) getNextLL(cur))
		{
		CITWINDOW *wnd = cur->wnd;
		SRECT rect2 = wnd->extents;

		if (overlapRect(rect, rect2))
			{
			// overlap going on...
			WINDOWLIST *cur2;

			for (cur2 = rd; cur2; cur2 = (WINDOWLIST *) getNextLL(cur2))
				{
				if (cur2->wnd == wnd)
					{
					break;
					}
				}

			if (!cur2)
				{
				cur2 = (WINDOWLIST *) addLL((void **) &rd, sizeof(*cur2));

				if (cur2)
					{
					cur2->wnd = wnd;
					}
				}
			}
		}

	// for speed, should check to see if this is needed...
	CitWindowsTTYUpdate(0, scrollpos);

	// redraw any windows with overlap
	for (cur = rd; cur; cur = (WINDOWLIST *) getNextLL(cur))
		{
		(cur->wnd->func)(EVT_DRAWALL, 0, 0, cur->wnd);
		}

	disposeLL((void **) &rd);
	}

void RedrawAll(void)
	{
	SRECT r;

	r.top = INT_MIN;
	r.bottom = INT_MAX;
	r.left = INT_MIN;
	r.right = INT_MAX;

	redrawRect(r);
	}
#endif
