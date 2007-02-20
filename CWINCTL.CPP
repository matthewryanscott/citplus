// --------------------------------------------------------------------------
// Citadel: CWinCtl.CPP
//
// Citadel Windows Control

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop
#include "cwindows.h"

// This works around a crash bug in BCC 4.0 (perhaps 4.5?)
#if __BORLANDC__ == 1106 || __BORLANDC__ == 1120
	#pragma option -x
#endif


// --------------------------------------------------------------------------
// Contents


Bool btHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_ISCTRLID:
			{
			return (((BTINFO *)(wnd->LocalData))->ci.id == more);
			}

		case EVT_LOSTFOCUS:
			{
			BTINFO *bi = (BTINFO *) wnd->LocalData;

			bi->focused = FALSE;
			wnd->oldB = 0;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_GOTFOCUS:
			{
			BTINFO *bi = (BTINFO *) wnd->LocalData;

			bi->focused = TRUE;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_DESTROY:
			{
			BTINFO *bi = (BTINFO *) wnd->LocalData;

			if (bi->ci.ptr)
				{
				freeMemG(bi->ci.ptr);
				bi->ci.ptr = NULL;
				}

			if (bi->ci.more)
				{
				freeMemG(bi->ci.more);
				bi->ci.more = NULL;
				}

			break;
			}

		case EVT_DRAWINT:
			{
			SRECT pr = wnd->parent->extents;
			SRECT r = wnd->extents;
			BTINFO *bi = (BTINFO *) wnd->LocalData;
			int attr = (bi->focused) ? cfg.cattr : cfg.attr;

			wnd->parent->extents.right = wnd->parent->extents.left + r.right;
			wnd->parent->extents.bottom = wnd->parent->extents.top + r.bottom;
			wnd->parent->extents.top += r.top;
			wnd->parent->extents.left += r.left;

			if (buildClipArray(wnd->parent))
				{
				CitWindowOutStr(wnd->parent, 0, 0, (char *)(bi->ci.ptr),
						attr);

				freeClipArray();
				}

			wnd->parent->extents = pr;

			break;
			}

		case EVT_MSTAT:
			{
			BTINFO *bi = (BTINFO *) wnd->LocalData;

			if (!wnd->oldB && (more & 1))		// new click
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r))
					{
					bi->clicked = TRUE;
					}
				}
			else if ((wnd->oldB & 1) && !more)	// just unclicked
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r) && bi->clicked)
					{
					wnd->oldB = more;

					(wnd->parent->func)(EVT_CTRLRET, (long) &(bi->ci),
							0, wnd->parent);

					return (TRUE);
					}
				}

			wnd->oldB = more;

			return (FALSE);
			}

		case EVT_INKEY:
			{
			BTINFO *bi = (BTINFO *) wnd->LocalData;

			if (bi->focused && (param == 32 || param == 13 || param == 10))
				{
				(wnd->parent->func)(EVT_CTRLRET, (long) &(bi->ci),
						0, wnd->parent);

				return (TRUE);
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

Bool tgHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_ISCTRLID:
			{
			return (((TGINFO *)(wnd->LocalData))->ci.id == more);
			}

		case EVT_LOSTFOCUS:
			{
			TGINFO *tg = (TGINFO *) wnd->LocalData;

			tg->focused = FALSE;
			wnd->oldB = 0;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_GOTFOCUS:
			{
			TGINFO *tg = (TGINFO *) wnd->LocalData;

			tg->focused = TRUE;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_DESTROY:
			{
			TGINFO *tg = (TGINFO *) wnd->LocalData;

			if (tg->ci.ptr)
				{
				freeMemG(tg->ci.ptr);
				tg->ci.ptr = NULL;
				}

			if (tg->ci.more)
				{
				freeMemG(tg->ci.more);
				tg->ci.more = NULL;
				}

			break;
			}

		case EVT_DRAWINT:
			{
			SRECT pr = wnd->parent->extents;
			SRECT r = wnd->extents;
			TGINFO *tg = (TGINFO *) wnd->LocalData;
			int attr = (tg->focused) ? cfg.cattr : cfg.attr;

			wnd->parent->extents.right = wnd->parent->extents.left + r.right;
			wnd->parent->extents.bottom = wnd->parent->extents.top + r.bottom;
			wnd->parent->extents.top += r.top;
			wnd->parent->extents.left += r.left;

			if (buildClipArray(wnd->parent))
				{
				char *pmt = (char *) ((*(Bool *)(tg->ci.ptr)) ?
						tg->on : tg->off);
				int i;

				CitWindowOutStr(wnd->parent, 0, 0, pmt, attr);

				for (i = strlen(pmt); i < r.right - r.left + 1; i++)
					{
					CitWindowOutChr(wnd->parent, i, 0, ' ', attr);
					}

				freeClipArray();
				}

			wnd->parent->extents = pr;

			break;
			}

		case EVT_MSTAT:
			{
			TGINFO *tg = (TGINFO *) wnd->LocalData;

			if (!wnd->oldB && (more & 1))		// new click
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r))
					{
					tg->clicked = TRUE;
					}
				}
			else if ((wnd->oldB & 1) && !more)	// just unclicked
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r) && tg->clicked)
					{
					wnd->oldB = more;

					*(Bool *)(tg->ci.ptr) = !*(Bool *)(tg->ci.ptr);

					(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

					return (TRUE);
					}
				}

			wnd->oldB = more;

			return (FALSE);
			}

		case EVT_INKEY:
			{
			TGINFO *tg = (TGINFO *) wnd->LocalData;

			if (tg->focused && (param == 32 || param == 13 || param == 10))
				{
				*(Bool *)(tg->ci.ptr) = !*(Bool *)(tg->ci.ptr);

				(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

				return (TRUE);
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

Bool mstHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_ISCTRLID:
			{
			return (((MSTINFO *)(wnd->LocalData))->ci.id == more);
			}

		case EVT_LOSTFOCUS:
			{
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;

			mst->focused = FALSE;
			wnd->oldB = 0;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_GOTFOCUS:
			{
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;

			mst->focused = TRUE;

			(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

			break;
			}

		case EVT_DESTROY:
			{
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;

			if (mst->ci.ptr)
				{
				freeMemG(mst->ci.ptr);
				mst->ci.ptr = NULL;
				}

			if (mst->ci.more)
				{
				freeMemG(mst->ci.more);
				mst->ci.more = NULL;
				}

			disposeLL((void **) &mst->sl);

			break;
			}

		case EVT_DRAWINT:
			{
			SRECT pr = wnd->parent->extents;
			SRECT r = wnd->extents;
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;
			int attr = (mst->focused) ? cfg.cattr : cfg.attr;

			wnd->parent->extents.right = wnd->parent->extents.left + r.right;
			wnd->parent->extents.bottom = wnd->parent->extents.top + r.bottom;
			wnd->parent->extents.top += r.top;
			wnd->parent->extents.left += r.left;

			if (buildClipArray(wnd->parent))
				{
				strList *sl;
				int i = *(int *)(mst->ci.ptr);

				for (sl = mst->sl; i; sl = (strList *) getNextLL(sl), i--);

				if (sl)
					{
					CitWindowOutStr(wnd->parent, 0, 0, sl->string, attr);
					i = strlen(sl->string);
					}
				else
					{
					i = 0;
					}

				for (; i < r.right - r.left + 1; i++)
					{
					CitWindowOutChr(wnd->parent, i, 0, ' ', attr);
					}

				freeClipArray();
				}

			wnd->parent->extents = pr;

			break;
			}

		case EVT_MSTAT:
			{
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;

			if (!wnd->oldB && (more & 1))		// new click
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r))
					{
					mst->clicked = TRUE;
					}
				}
			else if ((wnd->oldB & 1) && !more)	// just unclicked
				{
				SRECT r = wnd->extents;

				r.right += wnd->parent->extents.left;
				r.bottom += wnd->parent->extents.top;
				r.top += wnd->parent->extents.top;
				r.left += wnd->parent->extents.left;

				if (inRect(long_LO(param), long_HI(param), r) && mst->clicked)
					{
					wnd->oldB = more;

					(*(int *)(mst->ci.ptr))++;

					if (*(int *)(mst->ci.ptr) > mst->num)
						{
						*(int *)(mst->ci.ptr) = 0;
						}

					(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

					return (TRUE);
					}
				}

			wnd->oldB = more;

			return (FALSE);
			}

		case EVT_INKEY:
			{
			MSTINFO *mst = (MSTINFO *) wnd->LocalData;

			if (mst->focused && (param == 32 || param == 13 || param == 10))
				{
				(*(int *)(mst->ci.ptr))++;

				if (*(int *)(mst->ci.ptr) > mst->num)
					{
					*(int *)(mst->ci.ptr) = 0;
					}

				(wnd->func)(EVT_DRAWINT, 0, 0, wnd);

				return (TRUE);
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
