// --------------------------------------------------------------------------
// Citadel: CSysHall.CPP
//
// Console Sysop Hall

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "libovl.h"
#include "hall.h"
#include "group.h"
#include "cwindows.h"
#include "consysop.h"
#include "extmsg.h"


typedef struct
	{
	label hn;
	label gn;
	} hallNewData;

typedef struct
	{
	h_slot hs;
	label hn;
	label gn;
	char fn[13];
	Bool roomOK;
	} hallEditData;

// all of the control IDs used in this module
enum
	{
	CTRL_HALLKILLSTR,	CTRL_HALLKILLYN,	CTRL_HALLNEW,
	CTRL_HALLNEWYN, 	CTRL_HALLEDITSTR,	CTRL_HALLEDIT,
	};

static Bool hnHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			TEINFO *te;
			int i, j;

			i = strlen(getsysmsg(228));
			j = strlen(getsysmsg(250));

			i = max(i, j);

			if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
				{
				if ((te->ci.ptr = getMemG(LABELSIZE + 1, 1)) != NULL)
					{
					WINDOWFLAGS flags;
					SRECT rect;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;

					rect.top = 1;
					rect.bottom = 1;
					rect.left = i + 2;
					rect.right = rect.left + LABELSIZE;

					te->len = LABELSIZE;
					te->ci.id = 1;

					makeCitWindow(teHandler, wnd, getmsg(424), flags, rect,
							te, TRUE);

					if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
						{
						if ((te->ci.ptr = getMemG(LABELSIZE + 1, 1)) != NULL)
							{
							rect.top = 2;
							rect.bottom = 2;

							te->len = LABELSIZE;
							te->ci.id = 2;

							makeCitWindow(teHandler, wnd, getsysmsg(251),
									flags, rect, te, TRUE);
							}
						else
							{
							freeMemG(te);
							}
						}
					}
				else
					{
					freeMemG(te);
					}
				}

			break;
			}

		case EVT_CTRLRET:
			{
			WINDOWLIST *l;
			hallNewData *hd = (hallNewData *) (((CONTROLINFO *) wnd->LocalData)->ptr);

			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				if ((l->wnd->func)(EVT_ISCTRLID, 0, 1, l->wnd))
					{
					strcpy(hd->hn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 2, l->wnd))
					{
					strcpy(hd->gn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				}

			if (*hd->hn)
				{
				if (hallexists(hd->hn) != CERROR)
					{
					CitWindowsError(wnd, getmsg(105), hd->hn, cfg.Lhall_nym);
					return (TRUE);
					}

				if (*(hd->gn) && (FindGroupByPartialName(hd->gn, TRUE) == CERROR))
					{
					CitWindowsError(wnd, getsysmsg(252), hd->gn);
					return (TRUE);
					}
				}

			(wnd->parent->func)(EVT_CTRLRET, (long) wnd->LocalData, 0,
					wnd->parent);

			destroyCitWindow(wnd, FALSE);

			break;
			}

		case EVT_DRAWINT:
			{
			WINDOWLIST *l;

			if (buildClipArray(wnd))
				{
				int i, j, k, l;

				i = strlen(getsysmsg(228));
				j = strlen(getsysmsg(250));

				k = max(i, j) + 2;

				// do the prompts
				for (l = 1; l < k - i; l++)
					{
					CitWindowOutChr(wnd, l, 1, ' ', cfg.attr);
					}

				for (l = 1; l < k - j; l++)
					{
					CitWindowOutChr(wnd, l, 2, ' ', cfg.attr);
					}

				CitWindowOutStr(wnd, k - i, 1, getsysmsg(228), cfg.attr);
				CitWindowOutStr(wnd, k - j, 2, getsysmsg(250), cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 2, ' ', cfg.attr);

				freeClipArray();
				}

			// do the things being edited
			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				(l->wnd->func)(evt, param, more, l->wnd);
				}

			break;
			}

		case EVT_DESTROY:
			{
			CONTROLINFO *ci = (CONTROLINFO *) wnd->LocalData;

			if (ci->ptr)
				{
				freeMemG(ci->ptr);
				ci->ptr = NULL;
				}

			if (ci->more)
				{
				freeMemG(ci->more);
				ci->more = NULL;
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

static Bool heHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			hallEditData *he = (hallEditData *) ((CONTROLINFO *)(wnd->LocalData))->ptr;
			TEINFO *te;
			int i, j, k;

			i = strlen(getsysmsg(228));
			j = strlen(getsysmsg(250));
			k = strlen(getsysmsg(253));

			i = max(i, j);
			i = max(i, k);

			if (!cfg.nonAideRoomOk)
				{
				k = strlen(getsysmsg(254));
				i = max(i, k);
				}

			if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
				{
				if ((te->ci.ptr = getMemG(LABELSIZE + 1, 1)) != NULL)
					{
					WINDOWFLAGS flags;
					SRECT rect;

					strcpy((char *) te->ci.ptr, he->hn);

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;

					rect.top = 1;
					rect.bottom = 1;
					rect.left = i + 2;
					rect.right = rect.left + LABELSIZE;

					te->len = LABELSIZE;
					te->ci.id = 1;

					makeCitWindow(teHandler, wnd, getmsg(424), flags, rect,
							te, TRUE);

					if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
						{
						if ((te->ci.ptr = getMemG(LABELSIZE + 1, 1)) != NULL)
							{
							strcpy((char *) te->ci.ptr, he->gn);

							rect.top = 2;
							rect.bottom = 2;

							te->len = LABELSIZE;
							te->ci.id = 2;

							makeCitWindow(teHandler, wnd, getsysmsg(251),
									flags, rect, te, TRUE);

							if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
								{
								if ((te->ci.ptr = getMemG(13, 1)) != NULL)
									{
									strcpy((char *) te->ci.ptr, he->fn);

									rect.top = 3;
									rect.bottom = 3;
									rect.right = rect.left + 12;

									te->len = 12;
									te->ci.id = 3;

									makeCitWindow(teHandler, wnd,
											getsysmsg(255), flags, rect, te,
											TRUE);

									if (!cfg.nonAideRoomOk)
										{
										TGINFO *tg;

										if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1)) !=
												NULL)
											{
											if ((tg->ci.ptr =
													getMemG(sizeof(Bool), 1))
													!= NULL)
												{
												int i, j;

												*(Bool *)(tg->ci.ptr) =
														he->roomOK;

												tg->on = getmsg(521);
												tg->off = getmsg(522);

												i = strlen(getmsg(521));
												j = strlen(getmsg(522));

												i = max(i, j);

												rect.top = 4;
												rect.bottom = 4;
												rect.right = rect.left + i;

												te->ci.id = 4;

												makeCitWindow(tgHandler, wnd,
														getsysmsg(256), flags,
														rect, tg, TRUE);
												}
											else
												{
												freeMemG(tg);
												}
											}
										}
									}
								else
									{
									freeMemG(te);
									}
								}
							}
						else
							{
							freeMemG(te);
							}
						}
					}
				else
					{
					freeMemG(te);
					}
				}

			break;
			}

		case EVT_CTRLRET:
			{
			WINDOWLIST *l;
			hallEditData *he = (hallEditData *) ((CONTROLINFO *) wnd->LocalData)->ptr;

			he->roomOK = TRUE;

			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				if ((l->wnd->func)(EVT_ISCTRLID, 0, 1, l->wnd))
					{
					strcpy(he->hn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 2, l->wnd))
					{
					strcpy(he->gn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 3, l->wnd))
					{
					strcpy(he->fn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 4, l->wnd))
					{
					he->roomOK = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				}

			if (!(*he->hn))
				{
				CitWindowsError(wnd, getsysmsg(265));
				return (TRUE);
				}

			if ((hallexists(he->hn) != CERROR) &&
					!HallData[he->hs].IsSameName(he->hn))
				{
				CitWindowsError(wnd, getsysmsg(198), he->hn,
						cfg.Lhall_nym);
				return (TRUE);
				}

			if (*he->gn && (FindGroupByPartialName(he->gn, TRUE) == CERROR))
				{
				CitWindowsError(wnd, getsysmsg(252), he->gn);
				return (TRUE);
				}

			(wnd->parent->func)(EVT_CTRLRET, (long) wnd->LocalData, 0,
					wnd->parent);

			destroyCitWindow(wnd, FALSE);

			break;
			}

		case EVT_DRAWINT:
			{
			WINDOWLIST *l;

			if (buildClipArray(wnd))
				{
				int i, j, k, l, m, n;

				i = strlen(getsysmsg(228));
				j = strlen(getsysmsg(250));
				m = strlen(getsysmsg(253));

				if (!cfg.nonAideRoomOk)
					{
					n = strlen(getsysmsg(254));
					}
				else
					{
					n = 0;
					}

				k = max(i, j);
				k = max(k, n);
				k = max(k, m) + 2;

				// do the prompts
				for (l = 1; l < k - i; l++)
					{
					CitWindowOutChr(wnd, l, 1, ' ', cfg.attr);
					}

				for (l = 1; l < k - j; l++)
					{
					CitWindowOutChr(wnd, l, 2, ' ', cfg.attr);
					}

				for (l = 1; l < k - m; l++)
					{
					CitWindowOutChr(wnd, l, 3, ' ', cfg.attr);
					}

				CitWindowOutStr(wnd, k - i, 1, getsysmsg(228), cfg.attr);
				CitWindowOutStr(wnd, k - j, 2, getsysmsg(250), cfg.attr);
				CitWindowOutStr(wnd, k - m, 3, getsysmsg(253),
						cfg.attr);

				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 2, ' ', cfg.attr);

				for (l = k + 12; l < wnd->extents.right - wnd->extents.left;
						l++)
					{
					CitWindowOutChr(wnd, l, 3, ' ', cfg.attr);
					}

				if (!cfg.nonAideRoomOk)
					{
					int i = strlen(getmsg(521));
					int j = strlen(getmsg(522));

					i = max(i, j);

					for (l = 1; l < k - n; l++)
						{
						CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
						}

					CitWindowOutStr(wnd, k - n, 4, getsysmsg(254),
							cfg.attr);

					for (l = k + i;
							l < wnd->extents.right - wnd->extents.left; l++)
						{
						CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
						}
					}

				freeClipArray();
				}

			// do the things being edited
			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				(l->wnd->func)(evt, param, more, l->wnd);
				}

			break;
			}

		case EVT_DESTROY:
			{
			CONTROLINFO *ci = (CONTROLINFO *) wnd->LocalData;

			if (ci->ptr)
				{
				freeMemG(ci->ptr);
				ci->ptr = NULL;
				}

			if (ci->more)
				{
				freeMemG(ci->more);
				ci->more = NULL;
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

void HallControlHandler(CITWINDOW *wnd, long param)
	{
	CONTROLINFO *ci = (CONTROLINFO *) param;

	switch (ci->id)
		{
		case CTRL_HALLKILLSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				h_slot hs;

				hs = partialhall((char *)(ci->ptr));

				if (hs == 0 || hs == 1)
					{
					label hn;
					CitWindowsError(wnd, getsysmsg(325),
							HallData[hs].GetName(hn, sizeof(hn)));
					}
				else if (hs != CERROR)
					{
					h_slot *hsp;

					if ((hsp = (h_slot *) getMemG(sizeof(*hsp), 1)) != NULL)
						{
						char pmt[80];
						label hn;

						HallData[hs].GetName(hn, sizeof(hn));

						sprintf(pmt, getsysmsg(326), hn);

						*hsp = hs;

						CitWindowsGetYN(pmt, 0, wnd, CTRL_HALLKILLYN, hsp);
						}
					}
				else
					{
					CitWindowsError(wnd, getsysmsg(327), ci->ptr);
					}
				}

			break;
			}

		case CTRL_HALLKILLYN:
			{
			if (*((int *)(ci->ptr)))
				{
				label hn;

				HallData[*((h_slot *)(ci->more))].GetName(hn, sizeof(hn));

				if (!killhall(hn))
					{
					CitWindowsError(wnd, getsysmsg(328), hn);
					}
				else
					{
					if (wnd->children)
						{
						if (wnd->children->wnd->func == listHandler)
							{
							(wnd->children->wnd->func)(EVT_DRAWINT, 0, 0,
									wnd->children->wnd);
							}
						}
					}
				}

			break;
			}

		case CTRL_HALLNEWYN:
			{
			if (*((int *)(ci->ptr)))
				{
				hallNewData *hd = (hallNewData *) ci->more;

				if (!newhall(hd->hn, hd->gn, thisRoom)) // thisRoom? icky
					{
					CitWindowsError(wnd, getsysmsg(329), hd->hn);
					}
				else
					{
					if (wnd->children)
						{
						if (wnd->children->wnd->func == listHandler)
							{
							(wnd->children->wnd->func)(EVT_DRAWINT,
									0, 0, wnd->children->wnd);
							}
						}
					}
				}

			break;
			}

		case CTRL_HALLNEW:
			{
			hallNewData *hd = (hallNewData *) ci->ptr;

			if (*hd->hn)
				{
				hallNewData *hd1;

				if ((hd1 = (hallNewData *) getMemG(sizeof(*hd1), 1)) != NULL)
					{
					char pmt[80];

					*hd1 = *hd;

					sprintf(pmt, getsysmsg(330), hd->hn);

					CitWindowsGetYN(pmt, 0, wnd, CTRL_HALLNEWYN, hd1);
					}
				}

			break;
			}

		case CTRL_HALLEDITSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				h_slot hs;

				hs = partialhall((char *)(ci->ptr));

				if (hs != CERROR)
					{
					CONTROLINFO *ci;

					if ((ci = (CONTROLINFO *) getMemG(sizeof(*ci), 1)) != NULL)
						{
						hallEditData *he;

						he = (hallEditData *) getMemG(sizeof(hallEditData), 1);

						if (he)
							{
							CITWINDOW *w;
							WINDOWFLAGS flags;
							SRECT rect;
							int i, j, k;

							ci->id = CTRL_HALLEDIT;
							ci->ptr = he;

							memset(&flags, 0, sizeof(flags));

							flags.visible = TRUE;
							flags.showTitle = TRUE;
							flags.moveable = TRUE;
							flags.close = TRUE;
							flags.minimize = TRUE;

							i = strlen(getsysmsg(228)) + LABELSIZE + 3;
							j = strlen(getsysmsg(250)) + LABELSIZE + 3;
							k = strlen(getsysmsg(253)) + LABELSIZE + 3;

							i = max(i, j);
							i = max(i, k);

							if (!cfg.nonAideRoomOk)
								{
								k = strlen(getsysmsg(254)) +
										LABELSIZE + 3;
								i = max(i, k);
								}

							i = min(i, (conCols - 3));

							rect.top = (scrollpos - 2) / 2;
							rect.bottom = rect.top + 4;
							rect.left = (conCols - i) / 2;
							rect.right = rect.left + i;

							if (!cfg.nonAideRoomOk)
								{
								rect.bottom++;
								}

							he->hs = hs;
							HallData[hs].GetName(he->hn, sizeof(he->hn));
							if (HallData[hs].IsOwned() &&
									!HallData[hs].IsBoolGroup())
								{
								GroupData[HallData[hs].GetGroupNumber()].
										GetName(he->gn, sizeof(he->gn));
								}

							if (HallData[hs].IsDescribed())
								{
								HallData[hs].GetDescriptionFile(he->fn,
										sizeof(he->fn));
								}

							he->roomOK = HallData[hs].IsEnterRoom();

							w = makeCitWindow(heHandler, wnd, getsysmsg(331),
									flags, rect, ci, FALSE);

							if (w)
								{
								setFocus(w);
								}
							}
						else
							{
							freeMemG(ci);
							}
						}
					}
				else
					{
					CitWindowsError(wnd, getsysmsg(332), ci->ptr);
					}
				}

			break;
			}

		case CTRL_HALLEDIT:
			{
			hallEditData *he = (hallEditData *) ci->ptr;

			HallData[he->hs].SetName(he->hn);

			if (*he->gn)
				{
				HallData[he->hs].SetOwned(TRUE);
				HallData[he->hs].SetGroupNumber(FindGroupByPartialName(he->gn, TRUE));
				}
			else
				{
				HallData[he->hs].SetOwned(FALSE);
				}

			if (*he->fn)
				{
				HallData[he->hs].SetDescribed(TRUE);
				HallData[he->hs].SetDescriptionFile(he->fn);
				}
			else
				{
				HallData[he->hs].SetDescribed(FALSE);
				}

			HallData[he->hs].SetEnterRoom(he->roomOK);

			HallData.Save();

			if (wnd->children)
				{
				if (wnd->children->wnd->func == listHandler)
					{
					(wnd->children->wnd->func)(EVT_DRAWINT, 0, 0,
							wnd->children->wnd);
					}
				}

			break;
			}
		}
	}

Bool HallKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSHM_EXIT:
		case MK_ESC:
			{
			// exit hall menu stuff
			return (FALSE);
			}

		case CSHM_NEW:
			{
			h_slot i;

			for (i = 0; i < cfg.maxhalls; i++)
				{
				if (!HallData[i].IsInuse())
					{
					break;
					}
				}

			if (i == cfg.maxhalls)
				{
				CitWindowsError(wnd, getmsg(490), cfg.Uhall_nym);
				}
			else
				{
				CONTROLINFO *ci;

				if ((ci = (CONTROLINFO *) getMemG(sizeof(*ci), 1)) != NULL)
					{
					ci->id = CTRL_HALLNEW;
					ci->ptr = getMemG(sizeof(hallNewData), 1);

					if (ci->ptr)
						{
						CITWINDOW *w;
						WINDOWFLAGS flags;
						SRECT rect;
						int i, j;

						memset(&flags, 0, sizeof(flags));

						flags.visible = TRUE;
						flags.showTitle = TRUE;
						flags.moveable = TRUE;
						flags.close = TRUE;
						flags.minimize = TRUE;

						i = strlen(getsysmsg(228)) + LABELSIZE + 3;
						j = strlen(getsysmsg(250)) + LABELSIZE + 3;

						i = max(i, j);
						i = min(i, (conCols - 3));

						rect.top = (scrollpos - 2) / 2;
						rect.bottom = rect.top + 3;
						rect.left = (conCols - i) / 2;
						rect.right = rect.left + i;

						w = makeCitWindow(hnHandler, wnd, getsysmsg(333),
								flags, rect, ci, FALSE);

						if (w)
							{
							setFocus(w);
							}
						}
					else
						{
						freeMemG(ci);
						}
					}
				}

			break;
			}

		case CSHM_EDIT:
			{
			label df;

			if (wnd->children &&
					wnd->children->wnd->func == listHandler)
				{
				(((listerData *)(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, df, sizeof(df));
				}
			else
				{
				df[0] = 0;
				}

			CitWindowsGetString(getsysmsg(334), LABELSIZE, df, wnd,
					CTRL_HALLEDITSTR, NULL, FALSE);

			break;
			}

		case CSHM_KILL:
			{
			label df;

			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				(((listerData *)(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, df, sizeof(df));
				}
			else
				{
				df[0] = 0;
				}

			CitWindowsGetString(getsysmsg(335), LABELSIZE, df, wnd,
					CTRL_HALLKILLSTR, NULL, FALSE);

			break;
			}

		case CSHM_ROOMS:
			{
			break;
			}

		case CSHM_MOVEBACK:
			{
			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				label Buffer;
				h_slot hs = hallexists((((listerData *)
						(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, Buffer,
						sizeof(Buffer)));

				if (hs == CERROR)
					{
					CitWindowsError(wnd, getsysmsg(336));
					}
				else if (!moveHall(-1, hs, FALSE))
					{
					CitWindowsError(wnd, getmsg(498));
					}
				else
					{
					(wnd->children->wnd->func)(EVT_INKEY, CURS_UP << 8, 0,
							wnd->children->wnd);
					}
				}
			else
				{
				CitWindowsError(wnd, getsysmsg(337));
				}

			break;
			}

		case CSHM_MOVEUP:
			{
			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				label Buffer;
				h_slot hs = hallexists((((listerData *)
						(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, Buffer,
						sizeof(Buffer)));

				if (hs == CERROR)
					{
					CitWindowsError(wnd, getsysmsg(336));
					}
				else if (!moveHall(1, hs, FALSE))
					{
					CitWindowsError(wnd, getmsg(498));
					}
				else
					{
					(wnd->children->wnd->func)(EVT_INKEY, CURS_DOWN << 8, 0,
							wnd->children->wnd);
					}
				}
			else
				{
				CitWindowsError(wnd, getsysmsg(337));
				}

			break;
			}
		}

	return (TRUE);
	}

const char *hallLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length)
	{
	listerData *ld = (listerData *) wnd->LocalData;

	switch (cmd)
		{
		case LST_DRAW:
			{
			int i, j;

			if (!HallData[ld->on].IsInuse())
				{
				ld->on = 0; 	// root
				}

			if (ld->on < ld->top)
				{
				ld->top = ld->on;
				}

			if (buildClipArray(wnd))
				{
				for (j = 1, i = ld->top; i < cfg.maxhalls; i++)
					{
					if (j >= wnd->extents.bottom - wnd->extents.top)
						{
						break;
						}

					if (HallData[i].IsInuse())
						{
						int x;
						int r = min((LABELSIZE + 4),
								(wnd->extents.right - wnd->extents.left));

						ld->bottom = i;

						CitWindowOutChr(wnd, 1, j, ' ',
								i == ld->on ? cfg.cattr : cfg.attr);

						label HallName;
						HallData[i].GetName(HallName, sizeof(HallName));

						CitWindowOutStr(wnd, 2, j, HallName,
								i == ld->on ? cfg.cattr : cfg.attr);

						for (x = strlen(HallName) + 2; x < r; x++)
							{
							CitWindowOutChr(wnd, x, j, ' ',
									i == ld->on ? cfg.cattr : cfg.attr);
							}

						if (x == LABELSIZE + 4)
							{
							if (!HallData[i].IsOwned())
								{
								CitWindowOutStr(wnd, x, j, getsysmsg(338),
										i == ld->on ? cfg.cattr : cfg.attr);

								x += strlen(getsysmsg(338));
								}
							else
								{
								label Buffer;
								GroupData[HallData[i].GetGroupNumber()].
										GetName(Buffer, sizeof(Buffer));

								CitWindowOutStr(wnd, x, j, Buffer,
										i == ld->on ? cfg.cattr : cfg.attr);

								x += strlen(Buffer);
								}
							}

						for (r = wnd->extents.right - wnd->extents.left;
								x < r; x++)
							{
							CitWindowOutChr(wnd, x, j, ' ',
									i == ld->on ? cfg.cattr : cfg.attr);
							}

						j++;
						}
					}

				for (; j < wnd->extents.bottom - wnd->extents.top; j++)
					{
					CitWindowClearLine(wnd, j, cfg.attr);
					}

				freeClipArray();
				}

			break;
			}

		case LST_DOWN:
			{
			int tmp = ld->on;

			while (++ld->on < cfg.maxhalls && !HallData[ld->on].IsInuse());
			if (ld->on >= cfg.maxhalls)
				{
				ld->on = tmp;
				}

			if (ld->on > ld->bottom)
				{
				while (!HallData[++ld->top].IsInuse());
				}

			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_UP:
			{
			while (ld->on && !HallData[--ld->on].IsInuse());
			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGDN:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				int tmp = ld->on;

				while (++ld->on < cfg.maxhalls && !HallData[ld->on].IsInuse());
				if (ld->on >= cfg.maxhalls)
					{
					ld->on = tmp;
					}

				if (ld->on > ld->bottom)
					{
					while (!HallData[++ld->top].IsInuse());
					}
				}

			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGUP:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				while (ld->on && !HallData[--ld->on].IsInuse());
				}

			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_GET:
			{
			return (HallData[ld->on].GetName(Buffer, Length));
			}

		case LST_HOME:
			{
			ld->on = 0;
			ld->top = 0;
			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_END:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;
			int tmp;

			for (tmp = ld->on; ; tmp = ld->on)
				{
				while (++ld->on < cfg.maxhalls && !HallData[ld->on].IsInuse());

				if (ld->on >= cfg.maxhalls)
					{
					ld->on = tmp;
					break;
					}
				}

			ld->top = ld->on;
			for (i = 0; i < j; i++)
				{
				while (ld->top && !HallData[--ld->top].IsInuse());
				}

			hallLister(wnd, LST_DRAW, NULL, 0);
			break;
			}
		}

	return (NULL);
	}

#endif
