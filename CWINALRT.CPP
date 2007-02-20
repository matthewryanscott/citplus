// --------------------------------------------------------------------------
// Citadel: CWinAlrt.CPP
//
// Citadel Windows Alert

#include "ctdl.h"
#ifndef WINCIT
#pragma hdrstop
#include "cwindows.h"

#include "extmsg.h"


struct CWMsgLD
	{
	char Message[80];
	CITWINDOW *OldFocus;
	};

static Bool cweHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			BTINFO *bi;

			if ((bi = (BTINFO *) getMemG(sizeof(*bi), 1)) != NULL)
				{
				if ((bi->ci.ptr = strdup(getcwmsg(14))) != NULL)
					{
					WINDOWFLAGS flags;
					SRECT rect;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;

					rect.top = 2;
					rect.bottom = 2;
					rect.right = wnd->extents.right - wnd->extents.left - 1;
					rect.left = rect.right - strlen(getcwmsg(14));

					makeCitWindow(btHandler, wnd, getcwmsg(15), flags, rect,
							bi, TRUE);
					}
				else
					{
					freeMemG(bi);
					}
				}

			break;
			}

		case EVT_CTRLRET:
			{
			destroyCitWindow(wnd, FALSE);
			break;
			}

		case EVT_DRAWINT:
			{
			if (wnd->LocalData && buildClipArray(wnd))
				{
				CitWindowOutChr(wnd, 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right - wnd->extents.left -
						1, 1, ' ', cfg.attr);
				CitWindowOutStr(wnd, 2, 1, (char *) wnd->LocalData, cfg.attr);

				CitWindowClearLine(wnd, 2, cfg.attr);
				freeClipArray();
				}

			// do the button
			if (wnd->controls)
				{
				(wnd->controls->wnd->func)
						(evt, param, more, wnd->controls->wnd);
				}

			break;
			}

		case EVT_INKEY:
			{
			if ((int) param == ESC)
				{
				destroyCitWindow(wnd, FALSE);
				}
			else
				{
				return (defaultHandler(evt, param, more, wnd));
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

static Bool cwmHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DRAWINT:
			{
			if (wnd->LocalData && buildClipArray(wnd))
				{
				CitWindowOutChr(wnd, 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right - wnd->extents.left -
						1, 1, ' ', cfg.attr);
				CitWindowOutStr(wnd, 2, 1, (char *) wnd->LocalData, cfg.attr);

				freeClipArray();
				}

			break;
			}

		case EVT_DESTROY:
			{
			if (getFocus() == wnd)
				{
				CITWINDOW *w = ((CWMsgLD *) (wnd->LocalData))->OldFocus;

				if (IsWindowValid(w))
					{
					setFocus(w);
					}
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
#endif

#ifdef WINCIT
void cdecl CitWindowsError(CITWINDOW *, const char *, ...)  {}
#else
void cdecl CitWindowsError(CITWINDOW *parent, const char *fmt, ...)
	{
	if (initCitWindows)
		{
		char *ptr, str[80];

		va_list ap;

		va_start(ap, fmt);
		vsprintf(str, fmt, ap);
		va_end(ap);

		if ((ptr = strdup(str)) != NULL)
			{
			CITWINDOW *w;
			WINDOWFLAGS flags;
			SRECT rect;
			int i = strlen(str);

			memset(&flags, 0, sizeof(flags));

			flags.visible = TRUE;
			flags.showTitle = TRUE;
			flags.moveable = TRUE;
			flags.close = TRUE;

			rect.top = (scrollpos - 1) / 2;
			rect.bottom = rect.top + 3;
			rect.left = (conCols - i) / 2;
			rect.right = rect.left + i + 3;

			if ((w = makeCitWindow(cweHandler, parent, getcwmsg(18), flags,
					rect, ptr, FALSE)) != NULL)
				{
				setFocus(w);
				}
			}
		}
	}
#endif

#ifdef WINCIT
void cdecl CitWindowsNote(CITWINDOW *, const char *, ...) {}
#else
void cdecl CitWindowsNote(CITWINDOW *parent, const char *fmt, ...)
	{
	if (initCitWindows())
		{
		char *ptr, str[80];

		va_list ap;

		va_start(ap, fmt);
		vsprintf(str, fmt, ap);
		va_end(ap);

		if ((ptr = strdup(str)) != NULL)
			{
			CITWINDOW *w;
			WINDOWFLAGS flags;
			SRECT rect;
			int i = strlen(str);

			memset(&flags, 0, sizeof(flags));

			flags.visible = TRUE;
			flags.showTitle = TRUE;
			flags.moveable = TRUE;
			flags.close = TRUE;

			rect.top = (scrollpos - 1) / 2;
			rect.bottom = rect.top + 3;
			rect.left = (conCols - i) / 2;
			rect.right = rect.left + i + 3;

			if ((w = makeCitWindow(cweHandler, parent, getcwmsg(19), flags, rect,
					ptr, FALSE)) != NULL)
				{
				setFocus(w);
				}
			}
		}
	}
#endif

#ifdef WINCIT
CITWINDOW * cdecl CitWindowsMsg(CITWINDOW *, const char *, ...) {return (NULL);}
#else
CITWINDOW * cdecl CitWindowsMsg(CITWINDOW *parent, const char *fmt, ...)
	{
	CITWINDOW *w = NULL;

	if (initCitWindows())
		{
		CWMsgLD *LD = new CWMsgLD;

		if (LD)
			{
			va_list ap;

			va_start(ap, fmt);
			vsprintf(LD->Message, fmt, ap);
			va_end(ap);

			LD->OldFocus = getFocus();

			WINDOWFLAGS flags;
			SRECT rect;
			int i = strlen(LD->Message);

			memset(&flags, 0, sizeof(flags));

			flags.visible = TRUE;
			flags.moveable = TRUE;

			rect.top = (scrollpos - 1) / 2;
			rect.bottom = rect.top + 2;
			rect.left = (conCols - i) / 2;
			rect.right = rect.left + i + 3;

			if ((w = makeCitWindow(cwmHandler, parent, getcwmsg(20), flags,
					rect, LD, FALSE)) != NULL)
				{
				setFocus(w);
				}
			}
		}

	return (w);
	}
#endif

#ifndef WINCIT
enum
	{
	CTRL_YES,	CTRL_NO,	CTRL_ABORT,
	};

static Bool cwynHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
// 0 = yN; 1 = Yn; 2 = ynA; 3 = yNa; 4 = Yna
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			YNINFO *yn = (YNINFO *) wnd->LocalData;
			BTINFO *bi;
			Bool bad = FALSE;

			if ((bi = (BTINFO *) getMemG(sizeof(*bi), 1)) != NULL)
				{
				if ((bi->ci.ptr = strdup(getcwmsg(8))) != NULL)
					{
					WINDOWFLAGS flags;
					SRECT rect;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;

					rect.top = 2;
					rect.bottom = 2;
					rect.left = 1;
					rect.right = rect.left + strlen(getcwmsg(8));

					bi->ci.id = CTRL_YES;

					makeCitWindow(btHandler, wnd, getcwmsg(9), flags, rect,
							bi, TRUE);

					if ((bi = (BTINFO *) getMemG(sizeof(*bi), 1)) != NULL)
						{
						if ((bi->ci.ptr = strdup(getcwmsg(10))) != NULL)
							{
							if (*((int *)(yn->ci.ptr)) > 1)
								{
								rect.left = (wnd->extents.right -
									wnd->extents.left - strlen(getcwmsg(10))) / 2;
								rect.right = rect.left + strlen(getcwmsg(10));
								}
							else
								{
								rect.right = wnd->extents.right -
										wnd->extents.left;

								rect.left = rect.right - strlen(getcwmsg(10));
								}

							bi->ci.id = CTRL_NO;

							makeCitWindow(btHandler, wnd, getcwmsg(11), flags, rect,
									bi, TRUE);

							if (*((int *)(yn->ci.ptr)) > 1)
								{
								if ((bi = (BTINFO *) getMemG(sizeof(*bi), 1)) != NULL)
									{
									if ((bi->ci.ptr = strdup(getcwmsg(12)))
											!= NULL)
										{
										rect.right = wnd->extents.right -
												wnd->extents.left;
										rect.left = rect.right -
												strlen(getcwmsg(12));

										bi->ci.id = CTRL_ABORT;

										makeCitWindow(btHandler, wnd,
												getcwmsg(13), flags, rect,
												bi, TRUE);
										}
									else
										{
										bad = TRUE;
										freeMemG(bi);
										}
									}
								else
									{
									bad = TRUE;
									}
								}
							}
						else
							{
							bad = TRUE;
							freeMemG(bi);
							}
						}
					else
						{
						bad = TRUE;
						}
					}
				else
					{
					bad = TRUE;
					freeMemG(bi);
					}
				}
			else
				{
				bad = TRUE;
				}

			if (!bad)
				{
				switch (*((int *)(yn->ci.ptr)))
					{
					case 2: 	// skip to last
						{
						bringToTop(wnd->controls->next->next->wnd, FALSE,
								&(wnd->controls));
						break;
						}

					case 0: 	// skip to next
					case 3:
						{
						bringToTop(wnd->controls->next->wnd, FALSE,
								&(wnd->controls));
						break;
						}
					}
				}

			break;
			}

		case EVT_CTRLRET:
			{
			CONTROLINFO *ci = (CONTROLINFO *) param;
			YNINFO *yn = (YNINFO *) wnd->LocalData;

			if (ci->id == CTRL_YES)
				{
				*(int *)(yn->ci.ptr) = TRUE;
				}
			else if (ci->id == CTRL_NO)
				{
				*(int *)(yn->ci.ptr) = FALSE;
				}
			else	// abort
				{
				*(int *)(yn->ci.ptr) = 2;
				}

			(wnd->parent->func)(EVT_CTRLRET, (long) &(yn->ci), 0,
					wnd->parent);

			destroyCitWindow(wnd, FALSE);
			break;
			}

		case EVT_DRAWINT:
			{
			WINDOWLIST *l;

			if (wnd->LocalData && buildClipArray(wnd))
				{
				CitWindowOutChr(wnd, 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right - wnd->extents.left -
						1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right - wnd->extents.left -
						2, 1, '?', cfg.attr);
				CitWindowOutStr(wnd, 2, 1, ((YNINFO *)(wnd->LocalData))->pmt,
						cfg.attr);

				CitWindowClearLine(wnd, 2, cfg.attr);
				freeClipArray();
				}

			// do the buttons
			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				(l->wnd->func)(evt, param, more, l->wnd);
				}

			break;
			}

		case EVT_DESTROY:
			{
			YNINFO *yn = (YNINFO *) wnd->LocalData;

			if (yn->pmt)
				{
				freeMemG(yn->pmt);
				yn->pmt = NULL;
				}

			if (yn->ci.ptr)
				{
				freeMemG(yn->ci.ptr);
				yn->ci.ptr = NULL;
				}

			if (yn->ci.more)
				{
				freeMemG(yn->ci.more);
				yn->ci.more = NULL;
				}

			break;
			}

		case EVT_INKEY:
			{
			YNINFO *yn = (YNINFO *) wnd->LocalData;

			Bool Parsed = FALSE;

			if ((int) param == ESC)
				{
				if (*((int *)(yn->ci.ptr)) > 1)
					{
					*(int *)(yn->ci.ptr) = 2;
					}
				else
					{
					*(int *)(yn->ci.ptr) = FALSE;
					}

				Parsed = TRUE;
				}
			else if (toupper((int) param) == 'Y')
				{
				*(int *)(yn->ci.ptr) = TRUE;
				Parsed = TRUE;
				}
			else if (toupper((int) param) == 'N')
				{
				*(int *)(yn->ci.ptr) = FALSE;
				Parsed = TRUE;
				}
			else if (toupper((int) param) == 'A' && (*((int *)(yn->ci.ptr)) > 1))
				{
				*(int *)(yn->ci.ptr) = 2;
				Parsed = TRUE;
				}

			if (Parsed)
				{
				(wnd->parent->func)(EVT_CTRLRET, (long) &(yn->ci), 0,
						wnd->parent);
				destroyCitWindow(wnd, FALSE);

				break;
				}
			else
				{
				return (defaultHandler(evt, param, more, wnd));
				}
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

static Bool cwgsHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			TEINFO *te;

			if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
				{
				GSINFO *gs = (GSINFO *) wnd->LocalData;
				WINDOWFLAGS flags;
				SRECT rect;

				memset(&flags, 0, sizeof(flags));

				flags.visible = TRUE;

				rect.top = 1;
				rect.bottom = 1;
				rect.left = 4 + strlen(getcwmsg(5)) + strlen(gs->pmt);
				rect.right = wnd->extents.right - wnd->extents.left - 1;

				te->ci = gs->ci;
				te->len = gs->len;
				te->usenopwecho = gs->usenopwecho;

				if (makeCitWindow(teHandler, wnd, getcwmsg(6), flags, rect,
						te, TRUE))
					{
					// relenquish responsibility to dispose; TE will do it
					gs->ci.ptr = NULL;
					gs->ci.more = NULL;
					}
				}

			break;
			}

		case EVT_CTRLRET:
			{
			(wnd->parent->func)(EVT_CTRLRET, param, 0, wnd->parent);

			destroyCitWindow(wnd, FALSE);
			break;
			}

		case EVT_DRAWINT:
			{
			if (buildClipArray(wnd))
				{
				GSINFO *gs = (GSINFO *) wnd->LocalData;
				int i = 2;

				// do the prompt
				CitWindowOutChr(wnd, 1, 1, ' ', cfg.attr);
				CitWindowOutStr(wnd, i, 1, getcwmsg(5), cfg.attr);
				i += strlen(getcwmsg(5));
				CitWindowOutStr(wnd, i, 1, gs->pmt, cfg.attr);
				i += strlen(gs->pmt);
				CitWindowOutStr(wnd, i, 1, getcwmsg(7), cfg.attr);
				i += 2;

				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 1, ' ', cfg.attr);

				freeClipArray();
				}

			// do the thing being edited
			if (wnd->controls)
				{
				(wnd->controls->wnd->func)
						(evt, param, more, wnd->controls->wnd);
				}

			break;
			}

		case EVT_DESTROY:
			{
			GSINFO *gs = (GSINFO *) wnd->LocalData;

			if (gs->pmt)
				{
				freeMemG((void *) gs->pmt);
				gs->pmt = NULL;
				}

			if (gs->ci.ptr)
				{
				freeMemG(gs->ci.ptr);
				gs->ci.ptr = NULL;
				}

			if (gs->ci.more)
				{
				freeMemG(gs->ci.more);
				gs->ci.more = NULL;
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

void CitWindowsGetString(const char *prompt, int lim, const char *dfault,
		CITWINDOW *parent, int ctrlID, void *more, Bool UseNoPWEcho)
	{
	if (initCitWindows())
		{
		GSINFO *gs;

		if ((gs = (GSINFO *) getMemG(sizeof(*gs), 1)) != NULL)
			{
			gs->ci.id = ctrlID;
			gs->ci.ptr = getMemG(lim + 1, 1);
			gs->len = lim;
			gs->usenopwecho = UseNoPWEcho;

			if (gs->ci.ptr)
				{
				gs->pmt = strdup(prompt);

				if (gs->pmt)
					{
					CITWINDOW *w;
					WINDOWFLAGS flags;
					SRECT rect;
					int i;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;
					flags.showTitle = TRUE;
					flags.moveable = TRUE;

					i = strlen(prompt) + strlen(getcwmsg(5)) + 4 + lim;

					i = min(i, (conCols - 5));

					rect.top = (scrollpos - 1) / 2;
					rect.bottom = rect.top + 2;
					rect.left = (conCols - i) / 2;
					rect.right = rect.left + i;

					strcpy((char *)(gs->ci.ptr), dfault);
					gs->ci.more = more;

					w = makeCitWindow(cwgsHandler, parent, getcwmsg(16),
							flags, rect, gs, FALSE);

					if (w)
						{
						setFocus(w);
						}
					}
				else
					{
					if (more)
						{
						freeMemG(more);
						}

					freeMemG(gs->ci.ptr);
					freeMemG(gs);
					}
				}
			else
				{
				if (more)
					{
					freeMemG(more);
					}

				freeMemG(gs);
				}
			}
		else
			{
			if (more)
				{
				freeMemG(more);
				}
			}
		}
	}

static Bool gsfcHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_CTRLRET:
			{
			if (wnd->LocalData)
				{
				CONTROLINFO *ci = (CONTROLINFO *) param;

				(*(void (*)(const char *))(wnd->LocalData))
						((const char *)(ci->ptr));
				}

			break;
			}

		case EVT_CHILDDESTROY:
			{
			QueueMessage(EVT_DESTROY, (long) wnd, FALSE, NULL);
			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

void GetStringFromConsole(const char *prompt, int lim, const char *dfault,
		void (*func)(const char *), Bool UseNoPWEcho)
	{
	SRECT ext;
	memset(&ext, 0, sizeof(ext));

	WINDOWFLAGS flags;
	memset(&flags, 0, sizeof(flags));
	flags.nodiscard = TRUE;

#ifdef NDEBUG
	CITWINDOW *w = makeCitWindow(gsfcHandler, NULL, ns, flags, ext, func,
			FALSE);
#else
	CITWINDOW *w = makeCitWindow(gsfcHandler, NULL, "gsfc", flags, ext, func,
			FALSE);
#endif

	if (w)
		{
		CitWindowsGetString(prompt, lim, dfault, w, 0, NULL, UseNoPWEcho);
		}
	}

void CitWindowsGetYN(const char *pmt, int dfault, CITWINDOW *parent,
		int ctrlID, void *more)
// 0 = yN; 1 = Yn; 2 = ynA; 3 = yNa; 4 = Yna
	{
	if (initCitWindows())
		{
		YNINFO *yn;

		if ((yn = (YNINFO *) getMemG(sizeof(*yn), 1)) != NULL)
			{
			yn->ci.id = ctrlID;
			yn->ci.ptr = getMemG(sizeof(int), 1);

			if (yn->ci.ptr)
				{
				yn->pmt = strdup(pmt);

				if (yn->pmt)
					{
					CITWINDOW *w;
					WINDOWFLAGS flags;
					SRECT rect;
					int i, j;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;
					flags.showTitle = TRUE;
					flags.moveable = TRUE;

					i = strlen(pmt) + 4;

					j = strlen(getcwmsg(8)) + strlen(getcwmsg(10)) + 2;

					if (dfault > 1)
						{
						j += strlen(getcwmsg(12));
						}

					i = max(i, j);

					rect.top = (scrollpos - 2) / 2;
					rect.bottom = rect.top + 3;
					rect.left = (conCols - i) / 2;
					rect.right = rect.left + i;

					*((int *)(yn->ci.ptr)) = dfault;
					yn->ci.more = more;

					if ((w = makeCitWindow(cwynHandler, parent, getcwmsg(17),
							flags, rect, yn, FALSE)) != NULL)
						{
						setFocus(w);
						}
					}
				else
					{
					if (more)
						{
						freeMemG(more);
						}

					freeMemG(yn->ci.ptr);
					freeMemG(yn);
					}
				}
			else
				{
				if (more)
					{
					freeMemG(more);
					}

				freeMemG(yn);
				}
			}
		else
			{
			if (more)
				{
				freeMemG(more);
				}
			}
		}
	}

static Bool gynfcHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_CTRLRET:
			{
			if (wnd->LocalData)
				{
				CONTROLINFO *ci = (CONTROLINFO *) param;

				(*(void (*)(int))(wnd->LocalData))
						(*(int *)(ci->ptr));
				}

			break;
			}

		case EVT_CHILDDESTROY:
			{
			QueueMessage(EVT_DESTROY, (long) wnd, FALSE, NULL);
			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

void GetYNFromConsole(const char *prompt, int dfault, void (*func)(int))
// 0 = yN; 1 = Yn; 2 = ynA; 3 = yNa; 4 = Yna
	{
	SRECT ext;
	memset(&ext, 0, sizeof(ext));

	WINDOWFLAGS flags;
	memset(&flags, 0, sizeof(flags));
	flags.nodiscard = TRUE;

#ifdef NDEBUG
	CITWINDOW *w = makeCitWindow(gynfcHandler, NULL, ns, flags, ext, func,
			FALSE);
#else
	CITWINDOW *w = makeCitWindow(gynfcHandler, NULL, "gynfc", flags, ext,
			func, FALSE);
#endif

	if (w)
		{
		CitWindowsGetYN(prompt, dfault, w, 0, NULL);
		}
	}
#endif
