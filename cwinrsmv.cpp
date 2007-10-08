/* -------------------------------------------------------------------- */
/*	CWINRSMV.CPP					Citadel 							*/
/* -------------------------------------------------------------------- */
/*						The CitWindows control routines.				*/
/* -------------------------------------------------------------------- */
#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop
#include "cwindows.h"

#include "extmsg.h"


/* -------------------------------------------------------------------- */
/*								Contents								*/
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct
	{
	CORNERS c;
	} sizerData;

static Bool moverHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DRAWINT:
			{
			if (wnd->flags.visible)
				{
				if (buildClipArray(wnd))
					{
					CitWindowOutStr(wnd, 2, 1, getcwmsg(1), cfg.attr);
					freeClipArray();
					}
				}

			break;
			}

		case EVT_MSTAT:
			{
			if (more & 1)
				{
				SRECT rect = wnd->parent->extents;
				SRECT rect2 = rect;
				int i, h, w;

				h = rect.bottom - rect.top + 1;
				w = rect.right - rect.left + 1;

				i = long_HI(param);

				i = min(i, (scrollpos + 1 - h));
				rect.top = i;
				rect.bottom = rect.top + h - 1;

				i = long_LO(param);

				i = min(i, (conCols - w));
				rect.left = i;
				rect.right = rect.left + w - 1;

				if ((wnd->parent->func)(EVT_CHECKNEWEXT, (long) &rect, 0,
						wnd->parent))
					{
					wnd->parent->extents = rect;

					(wnd->parent->func)(EVT_DRAWALL, 0, 0, wnd->parent);
					redrawRect(rect2);
					}
				}
			else
				{
				destroyCitWindow(wnd, FALSE);
				}

			break;
			}

		case EVT_INKEY:
			{
			if (param == ESC)
				{
				destroyCitWindow(wnd, FALSE);
				}
			else if (param > 256)
				{
				switch (param >> 8)
					{
					case CURS_DOWN:
						{
						if (wnd->parent->extents.bottom < scrollpos)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.top++;
							rect.bottom++;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_UP:
						{
						if (wnd->parent->extents.top)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.top--;
							rect.bottom--;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_LEFT:
						{
						if (wnd->parent->extents.left)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.left--;
							rect.right--;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_RIGHT:
						{
						if (wnd->parent->extents.right < conCols - 1)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.left++;
							rect.right++;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}
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

static Bool sizerHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DRAWINT:
			{
			if (wnd->flags.visible)
				{
				if (buildClipArray(wnd))
					{
					CitWindowOutStr(wnd, 2, 1, getcwmsg(2), cfg.attr);
					freeClipArray();
					}
				}

			break;
			}

		case EVT_MSTAT:
			{
			if (more & 1)
				{
				SRECT rect = wnd->parent->extents;
				SRECT rect2 = rect;

				switch (((sizerData *) wnd->LocalData)->c)
					{
					case TOPLEFT:
						{
						int i;

						i = long_HI(param);

						i = min(i, (rect.bottom - 1));
						rect.top = i;

						i = long_LO(param);

						i = min(i, (rect.right - 10));
						rect.left = i;

						break;
						}

					case BOTTOMRIGHT:
						{
						int i;

						i = long_HI(param);

						i = max(i, (rect.top + 1));
						i = min(i, scrollpos);
						rect.bottom = i;

						i = long_LO(param);

						i = max(i, (rect.left + 10));
						rect.right = i;

						break;
						}

					case TOPRIGHT:
						{
						int i;

						i = long_HI(param);

						i = min(i, (rect.bottom - 1));
						rect.top = i;

						i = long_LO(param);

						i = max(i, (rect.left + 10));
						rect.right = i;

						break;
						}

					case BOTTOMLEFT:
						{
						int i;

						i = long_HI(param);

						i = max(i, (rect.top + 1));
						i = min(i, scrollpos);
						rect.bottom = i;

						i = long_LO(param);

						i = min(i, (rect.right - 10));
						rect.left = i;

						break;
						}
					}

				if ((wnd->parent->func)(EVT_CHECKNEWEXT, (long) &rect, 0,
						wnd->parent))
					{
					wnd->parent->extents = rect;

					(wnd->parent->func)(EVT_DRAWALL, 0, 0, wnd->parent);
					redrawRect(rect2);
					}
				}
			else
				{
				destroyCitWindow(wnd, FALSE);
				}

			break;
			}

		case EVT_INKEY:
			{
			if (param == ESC)
				{
				destroyCitWindow(wnd, FALSE);
				}
			else if (param > 256)
				{
				switch (param >> 8)
					{
					case CURS_DOWN:
						{
						if (wnd->parent->extents.bottom < scrollpos)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.bottom++;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_UP:
						{
						if (wnd->parent->extents.bottom >
								wnd->parent->extents.top + 1)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.bottom--;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_LEFT:
						{
						if (wnd->parent->extents.right >
								wnd->parent->extents.left + 5)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.right--;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}
							}

						break;
						}

					case CURS_RIGHT:
						{
						if (wnd->parent->extents.right < conCols - 1)
							{
							SRECT rect = wnd->parent->extents;
							SRECT rect2 = rect;

							rect.right++;

							if ((wnd->parent->func)(EVT_CHECKNEWEXT,
									(long) &rect, 0, wnd->parent))
								{
								wnd->parent->extents = rect;

								(wnd->parent->func)(EVT_DRAWALL, 0, 0,
										wnd->parent);

								redrawRect(rect2);
								}

							wnd->parent->extents.right++;
							(wnd->parent->func)(EVT_DRAWALL, param, 0, wnd->parent);
							}

						break;
						}
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

void initializeMove(CITWINDOW *wnd, Bool show)
	{
	if (wnd->flags.moveable)
		{
		CITWINDOW *mover;
		SRECT rect;
		WINDOWFLAGS flags;
		int len;

		len = strlen(getcwmsg(1)) + 2;

		memset(&flags, 0, sizeof(flags));

		flags.visible = show;
		flags.showTitle = show;

		rect.top = (scrollpos - 3) / 2;
		rect.left = (conCols - len) / 2;
		rect.bottom = rect.top + 2;
		rect.right = rect.left + len;

		mover = makeCitWindow(moverHandler, wnd, getcwmsg(3), flags, rect,
				NULL, FALSE);

		if (mover)
			{
			(mover->func)(EVT_DRAWALL, 0, 0, mover);
			setFocus(mover);

			setMousePos(wnd->extents.left * 8, wnd->extents.top * 8);
			}

		}
	}

void initializeResize(CITWINDOW *wnd, Bool show, CORNERS c)
	{
	if (wnd->flags.resize)
		{
		sizerData *sd;

		if ((sd = (sizerData *) getMemG(sizeof(*sd), 1)) != NULL)
			{
			CITWINDOW *sizer;
			SRECT rect;
			WINDOWFLAGS flags;
			int len;

			len = strlen(getcwmsg(2)) + 2;

			memset(&flags, 0, sizeof(flags));

			flags.visible = show;
			flags.showTitle = show;

			rect.top = (scrollpos - 3) / 2;
			rect.left = (conCols - len) / 2;
			rect.bottom = rect.top + 2;
			rect.right = rect.left + len;

			sd->c = c;

			wnd->flags.minimized = FALSE;
			wnd->flags.maximized = FALSE;

			sizer = makeCitWindow(sizerHandler, wnd, getcwmsg(4),
					flags, rect, sd, FALSE);

			if (sizer)
				{
				(sizer->func)(EVT_DRAWALL, 0, 0, sizer);
				setFocus(sizer);
				}
			}
		}
	}
#endif
