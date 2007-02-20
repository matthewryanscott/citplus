// --------------------------------------------------------------------------
// Citadel: CWinTE.CPP
//
// Citadel Windows Text Editor

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop
#include "cwindows.h"

// --------------------------------------------------------------------------
// Contents

Bool teHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_ISCTRLID:
			{
			return (((TEINFO *)(wnd->LocalData))->ci.id == more);
			}

		case EVT_NEWWINDOW:
			{
			TEINFO *te = (TEINFO *) wnd->LocalData;

			te->end = strlen((char *)(te->ci.ptr));
			te->cur = te->end;

			while (te->cur - te->start > wnd->extents.right -
					wnd->extents.left - 1)
				{
				te->start++;
				}

			break;
			}

		case EVT_LOSTFOCUS: 			// when losing focus, return cursor
			{
			TEINFO *te = (TEINFO *) wnd->LocalData;

			CitWindowsCsr = FALSE;

			cursoff();

			position(logiRow, logiCol);

			te->focused = FALSE;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_GOTFOCUS:				// when getting focus, capture cursor
			{
			TEINFO *te = (TEINFO *) wnd->LocalData;

			CitWindowsCsr = TRUE;

			te->focused = TRUE;

			physPosition((uchar) wnd->parent->extents.top + wnd->extents.top,
					(uchar) wnd->parent->extents.left + wnd->extents.left +
					te->cur - te->start);

			if (!wnd->parent->flags.minimized)
				{
				if (te->ins)
					{
					curshalf();
					}
				else
					{
					curson();
					}
				}

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_DESTROY:
			{
			TEINFO *te = (TEINFO *) wnd->LocalData;

			if (te->ci.ptr)
				{
				freeMemG(te->ci.ptr);
				te->ci.ptr = NULL;
				}

			if (te->ci.more)
				{
				freeMemG(te->ci.more);
				te->ci.more = NULL;
				}

			break;
			}

		case EVT_DRAWINT:
			{
			TEINFO *te = (TEINFO *) wnd->LocalData;
			SRECT pr = wnd->parent->extents;
			SRECT r = wnd->extents;
			int attr;

			if (te->focused)
				{
				physPosition((uchar) wnd->parent->extents.top + wnd->extents.top,
						(uchar) wnd->parent->extents.left + wnd->extents.left +
						te->cur - te->start);

				attr = cfg.cattr;
				}
			else
				{
				attr = cfg.attr;
				}

			wnd->parent->extents.right = wnd->parent->extents.left + r.right;
			wnd->parent->extents.bottom = wnd->parent->extents.top + r.bottom;
			wnd->parent->extents.top += r.top;
			wnd->parent->extents.left += r.left;

			if (buildClipArray(wnd->parent))
				{
				int i;

				if (te->usenopwecho)
					{
					char Output;

					if (cfg.nopwecho == 1)	// Nothing (actually, spaces for us)
						{
						Output = 0;
						}
					else if (cfg.nopwecho)	// Echo character (count if digit)
						{
						Output = cfg.nopwecho;
						}

					for (i = 0; i < te->end - te->start; i++)
						{
						char ToOutput;

						if (!cfg.nopwecho)	// Echo it
							{
							ToOutput = ((char *)(te->ci.ptr))[te->start + i];
							}
						else				// Echo mask
							{
							// if digit - count (this is really stupid: it
							// should happen after the assignment. However,
							// we don't set the precedent here. That's in
							// getString()...
							if (isdigit(Output))
								{
								Output++;

								if (!isdigit(Output))
									{
									Output = '0';
									}
								}

							ToOutput = Output;
							}

						CitWindowOutChr(wnd->parent, i, 0, ToOutput, attr);
						}
					}
				else
					{
					CitWindowOutStr(wnd->parent, 0, 0,
							(char *)(te->ci.ptr) + te->start, attr);
					}

				for (i = te->end - te->start; i < r.right - r.left; i++)
					{
					CitWindowOutChr(wnd->parent, i, 0, ' ', cfg.attr);
					}

				freeClipArray();
				}

			wnd->parent->extents = pr;

			break;
			}

		case EVT_INKEY:
			{
			if (param == 13)			// return string
				{
				TEINFO *te = (TEINFO *) wnd->LocalData;

				normalizeString((char *) te->ci.ptr);

				(wnd->parent->func)(EVT_CTRLRET, (long) &(te->ci),
						0, wnd->parent);

				return (TRUE);
				}
			else if (param == ESC)		// abort edit
				{
				destroyCitWindow(wnd->parent, FALSE);

				return (TRUE);
				}
			else if (param == 8)		// backspace
				{
				TEINFO *te = (TEINFO *) wnd->LocalData;

				if (te->cur)
					{
					int i;

					te->cur--;
					te->end--;

					if (te->cur < te->start)
						{
						te->start = te->cur;
						}

					for (i = te->cur; ((char *)(te->ci.ptr))[i]; i++)
						{
						((char *)(te->ci.ptr))[i] =
								((char *)(te->ci.ptr))[i + 1];
						}

					(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
					}

				return (TRUE);
				}
			else if (param < 256 && param > 31) 	// add to string
				{
				TEINFO *te = (TEINFO *) wnd->LocalData;

				if ((te->cur < te->len) && (!te->ins || (te->end < te->len)))
					{
					if (te->ins)
						{
						int i;

						((char *)(te->ci.ptr))[te->end + 1] = 0;

						for (i = te->end; i != te->cur; i--)
							{
							((char *)(te->ci.ptr))[i] =
									((char *)(te->ci.ptr))[i - 1];
							}
						}

					((char *)(te->ci.ptr))[te->cur] = (char) param;

					te->cur++;

					if (te->cur > te->end || te->ins)
						{
						te->end++;
						}

					while (te->cur - te->start > wnd->extents.right -
							wnd->extents.left - 1)
						{
						te->start++;
						}

					(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
					}

				return (TRUE);
				}
			else if (param > 255)
				{
				switch (param >> 8)
					{
					case CURS_DEL:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						if (te->cur != te->end)
							{
							int i;

							for (i = te->cur; ((char *)(te->ci.ptr))[i]; i++)
								{
								((char *)(te->ci.ptr))[i] =
										((char *)(te->ci.ptr))[i + 1];
								}

							te->end--;

							(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
							}

						return (TRUE);
						}

					case CURS_INS:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						te->ins = !te->ins;

						if (!wnd->parent->flags.minimized)
							{
							if (te->ins)
								{
								curshalf();
								}
							else
								{
								curson();
								}
							}

						return (TRUE);
						}

					case CTL_CURS_LEFT:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						if (te->cur)
							{
							// eat up space
							while (te->cur &&
									isspace(((char *)(te->ci.ptr))[te->cur - 1]))
								{
								te->cur--;
								}

							// eat up word
							while (te->cur &&
									!isspace(((char *)(te->ci.ptr))[te->cur - 1]))
								{
								te->cur--;
								}

							if (te->cur < te->start)
								{
								te->start = te->cur;
								}

							(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
							}

						return (TRUE);
						}

					case CTL_CURS_RIGHT:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						if (te->cur < te->end)
							{
							// eat up word
							while ((te->cur < te->end) &&
									!isspace(((char *)(te->ci.ptr))[te->cur + 1]))
								{
								te->cur++;
								}

							// eat up space
							while ((te->cur < te->end) &&
									isspace(((char *)(te->ci.ptr))[te->cur + 1]))
								{
								te->cur++;
								}

							if (te->cur < te->end)
								{
								te->cur++;
								}

							while (te->cur - te->start >
									wnd->extents.right -
									wnd->extents.left - 1)
								{
								te->start++;
								}

							(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
							}

						return (TRUE);
						}

					case CURS_LEFT:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						if (te->cur)
							{
							te->cur--;

							if (te->cur < te->start)
								{
								te->start = te->cur;
								}

							(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
							}

						return (TRUE);
						}

					case CURS_RIGHT:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						if (te->cur < te->end)
							{
							te->cur++;

							while (te->cur - te->start >
									wnd->extents.right -
									wnd->extents.left - 1)
								{
								te->start++;
								}

							(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
							}
						return (TRUE);
						}

					case CURS_HOME:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						te->start = te->cur = 0;
						(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
						return (TRUE);
						}

					case CURS_END:
						{
						TEINFO *te = (TEINFO *) wnd->LocalData;

						te->end = strlen((char *)(te->ci.ptr));
						te->cur = te->end;

						while (te->cur - te->start >
								wnd->extents.right - wnd->extents.left - 1)
							{
							te->start++;
							}

						(wnd->func)(EVT_DRAWINT, 0, 0, wnd);
						return (TRUE);
						}
					}
				}

			return (FALSE);
			}

		default:
			{
			return (FALSE);
			}
		}

	return (TRUE);
	}
#endif
