// --------------------------------------------------------------------------
// Citadel: CSysGrp.CPP
//
// Console Sysop Group

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "libovl.h"
#include "group.h"
#include "cwindows.h"
#include "consysop.h"
#include "extmsg.h"


// all of the control IDs used in this module
enum
	{
	CTRL_GROUPKILLSTR,	CTRL_GROUPKILLYN,	CTRL_GROUPNEW,
	CTRL_GROUPNEWYN,	CTRL_GROUPEDITSTR,	CTRL_GROUPEDIT,
	};

typedef struct
	{
	label gn;
	char gd[80];
	Bool lock;
	Bool hide;
	Bool autoadd;
	} groupNewData;

typedef struct
	{
	int gs;
	label gn;
	char gd[80];
	Bool lock;
	Bool hide;
	Bool autoadd;
	} groupEditData;

static Bool gnHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			TEINFO *te;
			int i, j;

			i = strlen(getsysmsg(228));
			j = strlen(getsysmsg(229));

			i = max(i, j);

			j = strlen(getsysmsg(230));
			i = max(i, j);

			j = strlen(getsysmsg(231));
			i = max(i, j);

			j = strlen(getsysmsg(232));
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
						if ((te->ci.ptr = getMemG(80, 1)) != NULL)
							{
							TGINFO *tg;

							rect.top = 2;
							rect.bottom = 2;

							te->len = 79;
							te->ci.id = 2;

							makeCitWindow(teHandler, wnd, getsysmsg(233),
									flags, rect, te, TRUE);

							if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1)) != NULL)
								{
								if ((tg->ci.ptr = getMemG(sizeof(Bool), 1))
										!= NULL)
									{
									int i, j;

									tg->on = getmsg(521);
									tg->off = getmsg(522);

									i = strlen(getmsg(521));
									j = strlen(getmsg(522));

									i = max(i, j);

									rect.top = 3;
									rect.bottom = 3;
									rect.right = rect.left + i;

									tg->ci.id = 3;

									makeCitWindow(tgHandler, wnd, getsysmsg(234),
											flags, rect, tg, TRUE);

									if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1)) !=
											NULL)
										{
										if ((tg->ci.ptr =
												getMemG(sizeof(Bool), 1))
												!= NULL)
											{
											tg->on = getmsg(521);
											tg->off = getmsg(522);

											rect.top = 4;
											rect.bottom = 4;

											tg->ci.id = 4;

											makeCitWindow(tgHandler, wnd,
													getsysmsg(235), flags,
													rect, tg, TRUE);

											if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1))
													!= NULL)
												{
												if ((tg->ci.ptr =
														getMemG(sizeof(Bool),
														1)) != NULL)
													{
													tg->on = getmsg(521);
													tg->off = getmsg(522);

													rect.top = 5;
													rect.bottom = 5;

													tg->ci.id = 5;

													makeCitWindow(tgHandler,
															wnd, getsysmsg(236),
															flags, rect, tg,
															TRUE);
													}
												else
													{
													freeMemG(tg);
													}
												}
											}
										else
											{
											freeMemG(tg);
											}
										}
									}
								else
									{
									freeMemG(tg);
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
			groupNewData *gd = (groupNewData *) (((CONTROLINFO *) wnd->LocalData)->ptr);

			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				if ((l->wnd->func)(EVT_ISCTRLID, 0, 1, l->wnd))
					{
					strcpy(gd->gn, (char *) (((TEINFO *)(l->wnd->LocalData))->ci.ptr));
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 2, l->wnd))
					{
					strcpy(gd->gd, (char *) (((TEINFO *)(l->wnd->LocalData))->ci.ptr));
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 3, l->wnd))
					{
					gd->lock = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 4, l->wnd))
					{
					gd->hide = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 5, l->wnd))
					{
					gd->autoadd = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				}

			if (*gd->gn && (FindGroupByName(gd->gn) != CERROR))
				{
				CitWindowsError(wnd, getmsg(105), gd->gn, cfg.Lgroup_nym);
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
				int i, j, k, l, m, n, o, p;

				i = strlen(getsysmsg(228));
				j = strlen(getsysmsg(229));
				m = strlen(getsysmsg(230));
				n = strlen(getsysmsg(231));
				o = strlen(getsysmsg(232));

				k = max(i, j);
				p = max(m, n);
				k = max(k, p);
				k = max(o, k) + 2;

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

				for (l = 1; l < k - n; l++)
					{
					CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
					}

				for (l = 1; l < k - o; l++)
					{
					CitWindowOutChr(wnd, l, 5, ' ', cfg.attr);
					}

				CitWindowOutStr(wnd, k - i, 1, getsysmsg(228), cfg.attr);
				CitWindowOutStr(wnd, k - j, 2, getsysmsg(229), cfg.attr);
				CitWindowOutStr(wnd, k - m, 3, getsysmsg(230), cfg.attr);
				CitWindowOutStr(wnd, k - n, 4, getsysmsg(231), cfg.attr);
				CitWindowOutStr(wnd, k - o, 5, getsysmsg(232), cfg.attr);

				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 2, ' ', cfg.attr);

				i = strlen(getmsg(521));
				j = strlen(getmsg(522));

				i = max(i, j);

				for (l = k + i;
						l < wnd->extents.right - wnd->extents.left; l++)
					{
					CitWindowOutChr(wnd, l, 3, ' ', cfg.attr);
					CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
					CitWindowOutChr(wnd, l, 5, ' ', cfg.attr);
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

static Bool geHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_NEWWINDOW:
			{
			groupEditData *ge = (groupEditData *) ((CONTROLINFO *)(wnd->LocalData))->ptr;
			TEINFO *te;
			int i, j;

			i = strlen(getsysmsg(228));
			j = strlen(getsysmsg(229));

			i = max(i, j);

			j = strlen(getsysmsg(230));
			i = max(i, j);

			j = strlen(getsysmsg(231));
			i = max(i, j);

			j = strlen(getsysmsg(232));
			i = max(i, j);

			if ((te = (TEINFO *) getMemG(sizeof(*te), 1)) != NULL)
				{
				if ((te->ci.ptr = getMemG(LABELSIZE + 1, 1)) != NULL)
					{
					WINDOWFLAGS flags;
					SRECT rect;

					strcpy((char *) te->ci.ptr, ge->gn);

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
						if ((te->ci.ptr = getMemG(80, 1)) != NULL)
							{
							TGINFO *tg;

							strcpy((char *) te->ci.ptr, ge->gd);

							rect.top = 2;
							rect.bottom = 2;

							te->len = 79;
							te->ci.id = 2;

							makeCitWindow(teHandler, wnd, getsysmsg(233),
									flags, rect, te, TRUE);

							if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1)) != NULL)
								{
								if ((tg->ci.ptr = getMemG(sizeof(Bool), 1))
										!= NULL)
									{
									int i, j;

									tg->on = getmsg(521);
									tg->off = getmsg(522);

									i = strlen(getmsg(521));
									j = strlen(getmsg(522));

									i = max(i, j);

									*((Bool *)(tg->ci.ptr)) = ge->lock;

									rect.top = 3;
									rect.bottom = 3;
									rect.right = rect.left + i;

									tg->ci.id = 3;

									makeCitWindow(tgHandler, wnd,
											getsysmsg(234), flags, rect, tg,
											TRUE);

									if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1)) !=
											NULL)
										{
										if ((tg->ci.ptr =
												getMemG(sizeof(Bool), 1))
												!= NULL)
											{
											tg->on = getmsg(521);
											tg->off = getmsg(522);

											*(Bool *)(tg->ci.ptr) =
													ge->hide;

											rect.top = 4;
											rect.bottom = 4;

											tg->ci.id = 4;

											makeCitWindow(tgHandler, wnd,
													getsysmsg(235), flags,
													rect, tg, TRUE);

											if ((tg = (TGINFO *) getMemG(sizeof(*tg), 1))
													!= NULL)
												{
												if ((tg->ci.ptr =
														getMemG(sizeof(Bool),
														1)) != NULL)
													{
													tg->on = getmsg(521);
													tg->off = getmsg(522);

													*(Bool *)(tg->ci.ptr) =
															ge->autoadd;

													rect.top = 5;
													rect.bottom = 5;

													tg->ci.id = 5;

													makeCitWindow(tgHandler,
															wnd, getsysmsg(236),
															flags, rect, tg,
															TRUE);
													}
												else
													{
													freeMemG(tg);
													}
												}
											}
										else
											{
											freeMemG(tg);
											}
										}
									}
								else
									{
									freeMemG(tg);
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
			groupEditData *ge = (groupEditData *) ((CONTROLINFO *) wnd->LocalData)->ptr;

			for (l = wnd->controls; l; l = (WINDOWLIST *) getNextLL(l))
				{
				if ((l->wnd->func)(EVT_ISCTRLID, 0, 1, l->wnd))
					{
					strcpy(ge->gn, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 2, l->wnd))
					{
					strcpy(ge->gd, (char *) ((TEINFO *)(l->wnd->LocalData))->ci.ptr);
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 3, l->wnd))
					{
					ge->lock = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 4, l->wnd))
					{
					ge->hide = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				else if ((l->wnd->func)(EVT_ISCTRLID, 0, 5, l->wnd))
					{
					ge->autoadd = *(Bool *)
							((TGINFO *)(l->wnd->LocalData))->ci.ptr;
					}
				}

			if (!(*ge->gn))
				{
				CitWindowsError(wnd, getsysmsg(237));
				return (TRUE);
				}

			if ((FindGroupByName(ge->gn) != CERROR) &&
					!GroupData[ge->gs].IsSameName(ge->gn))
				{
				CitWindowsError(wnd, getsysmsg(185), ge->gn);
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
				int i, j, k, l, m, n, o, p;

				i = strlen(getsysmsg(228));
				j = strlen(getsysmsg(229));
				m = strlen(getsysmsg(230));
				n = strlen(getsysmsg(231));
				o = strlen(getsysmsg(232));

				k = max(i, j);
				p = max(m, n);
				k = max(k, p);
				k = max(o, k) + 2;

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

				for (l = 1; l < k - n; l++)
					{
					CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
					}

				for (l = 1; l < k - o; l++)
					{
					CitWindowOutChr(wnd, l, 5, ' ', cfg.attr);
					}

				CitWindowOutStr(wnd, k - i, 1, getsysmsg(228), cfg.attr);
				CitWindowOutStr(wnd, k - j, 2, getsysmsg(229), cfg.attr);
				CitWindowOutStr(wnd, k - m, 3, getsysmsg(230), cfg.attr);
				CitWindowOutStr(wnd, k - n, 4, getsysmsg(231), cfg.attr);
				CitWindowOutStr(wnd, k - o, 5, getsysmsg(232), cfg.attr);

				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 1, ' ', cfg.attr);
				CitWindowOutChr(wnd, wnd->extents.right -
						wnd->extents.left - 1, 2, ' ', cfg.attr);

				i = strlen(getmsg(521));
				j = strlen(getmsg(522));

				i = max(i, j);

				for (l = k + i;
						l < wnd->extents.right - wnd->extents.left; l++)
					{
					CitWindowOutChr(wnd, l, 3, ' ', cfg.attr);
					CitWindowOutChr(wnd, l, 4, ' ', cfg.attr);
					CitWindowOutChr(wnd, l, 5, ' ', cfg.attr);
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

void GroupControlHandler(CITWINDOW *wnd, long param)
	{
	CONTROLINFO *ci = (CONTROLINFO *) param;

	switch (ci->id)
		{
		case CTRL_GROUPKILLSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				int gs;

				gs = FindGroupByPartialName((char *)(ci->ptr), TRUE);

				if (gs == 0 || gs == 1)
					{
					label gn;
					CitWindowsError(wnd, getsysmsg(238),
							GroupData[gs].GetName(gn, sizeof(gn)));
					}
				else if (gs != CERROR)
					{
					g_slot *gsp;

					if ((gsp = (g_slot *) getMemG(sizeof(*gsp), 1)) != NULL)
						{
						char pmt[80];
						label gn;

						GroupData[gs].GetName(gn, sizeof(gn));

						sprintf(pmt, getsysmsg(239), gn);

						*gsp = gs;

						CitWindowsGetYN(pmt, 0, wnd, CTRL_GROUPKILLYN, gsp);
						}
					}
				else
					{
					CitWindowsError(wnd, getsysmsg(240), ci->ptr);
					}
				}

			break;
			}

		case CTRL_GROUPKILLYN:
			{
			if (*((int *)(ci->ptr)))
				{
				label gn;

				GroupData[*((int *)(ci->more))].GetName(gn, sizeof(gn));

				if (!killgroup(gn))
					{
					CitWindowsError(wnd, getsysmsg(241), gn);
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

		case CTRL_GROUPNEWYN:
			{
			if (*((int *)(ci->ptr)))
				{
				groupNewData *gd = (groupNewData *) ci->more;

				if (!newgroup(gd->gn, gd->gd, gd->lock, gd->hide,
						gd->autoadd))
					{
					CitWindowsError(wnd, getsysmsg(242), gd->gn);
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

		case CTRL_GROUPNEW:
			{
			groupNewData *gd = (groupNewData *) ci->ptr;

			if (*gd->gn)
				{
				groupNewData *gd1;

				if ((gd1 = (groupNewData *) getMemG(sizeof(*gd1), 1)) != NULL)
					{
					char pmt[80];

					*gd1 = *gd;

					sprintf(pmt, getsysmsg(243), gd->gn);

					CitWindowsGetYN(pmt, 0, wnd, CTRL_GROUPNEWYN, gd1);
					}
				}

			break;
			}

		case CTRL_GROUPEDITSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				int gs;

				gs = FindGroupByPartialName((char *)(ci->ptr), TRUE);

				if (gs != CERROR)
					{
					CONTROLINFO *ci;

					if ((ci = (CONTROLINFO *) getMemG(sizeof(*ci), 1)) !=
							NULL)
						{
						groupEditData *ge;

						ge = (groupEditData *) getMemG(sizeof(groupEditData),
								1);

						if (ge)
							{
							CITWINDOW *w;
							WINDOWFLAGS flags;
							SRECT rect;
							int i, j;

							ci->id = CTRL_GROUPEDIT;
							ci->ptr = ge;

							memset(&flags, 0, sizeof(flags));

							flags.visible = TRUE;
							flags.showTitle = TRUE;
							flags.moveable = TRUE;
							flags.close = TRUE;
							flags.minimize = TRUE;

							i = strlen(getsysmsg(228)) + LABELSIZE + 3;
							j = strlen(getsysmsg(229)) + LABELSIZE + 3;

							i = max(i, j);

							j = strlen(getsysmsg(230)) + LABELSIZE + 3;
							i = max(i, j);

							j = strlen(getsysmsg(231)) + LABELSIZE + 3;
							i = max(i, j);

							j = strlen(getsysmsg(232)) + LABELSIZE + 3;
							i = max(i, j);

							i = min(i, (conCols - 3));

							rect.top = (scrollpos - 3) / 2;
							rect.bottom = rect.top + 6;
							rect.left = (conCols - i) / 2;
							rect.right = rect.left + i;

							ge->gs = gs;
							GroupData[gs].GetName(ge->gn, sizeof(ge->gn));
							GroupData[gs].GetDescription(ge->gd,
									sizeof(ge->gd));
							ge->lock = GroupData[gs].IsLocked();
							ge->hide = GroupData[gs].IsHidden();
							ge->autoadd = GroupData[gs].IsAutoAdd();

							w = makeCitWindow(geHandler, wnd,
									getsysmsg(244), flags, rect, ci,
									FALSE);

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
					CitWindowsError(wnd, getsysmsg(245), ci->ptr);
					}
				}

			break;
			}

		case CTRL_GROUPEDIT:
			{
			groupEditData *ge = (groupEditData *) ci->ptr;

			GroupData[ge->gs].SetName(ge->gn);
			GroupData[ge->gs].SetDescription(ge->gd);
			GroupData[ge->gs].SetLocked(ge->lock);
			GroupData[ge->gs].SetHidden(ge->hide);
			GroupData[ge->gs].SetAutoAdd(ge->autoadd);

			GroupData.Save();

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

Bool GroupKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSGM_EXIT:
		case MK_ESC:
			{
			return (FALSE);
			}

		case CSGM_NEW:
			{
			int i;

			for (i = 0; i < cfg.maxgroups; i++)
				{
				if (!GroupData[i].IsInuse())
					{
					break;
					}
				}

			if (i == cfg.maxgroups)
				{
				CitWindowsError(wnd, getsysmsg(264));
				}
			else
				{
				CONTROLINFO *ci;

				if ((ci = (CONTROLINFO *) getMemG(sizeof(*ci), 1)) != NULL)
					{
					ci->id = CTRL_GROUPNEW;
					ci->ptr = getMemG(sizeof(groupNewData), 1);

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
						j = strlen(getsysmsg(229)) + LABELSIZE + 3;

						i = max(i, j);

						j = strlen(getsysmsg(230)) + LABELSIZE + 3;
						i = max(i, j);

						j = strlen(getsysmsg(231)) + LABELSIZE + 3;
						i = max(i, j);

						j = strlen(getsysmsg(232)) + LABELSIZE + 3;
						i = max(i, j);

						i = min(i, (conCols - 3));

						rect.top = (scrollpos - 3) / 2;
						rect.bottom = rect.top + 6;
						rect.left = (conCols - i) / 2;
						rect.right = rect.left + i;

						w = makeCitWindow(gnHandler, wnd, getsysmsg(246),
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

		case CSGM_EDIT:
			{
			label df;

			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				label Buffer;
				strcpy(df, (((listerData *)
						(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, Buffer,
						sizeof(Buffer)));
				}
			else
				{
				df[0] = 0;
				}

			CitWindowsGetString(getsysmsg(247), LABELSIZE, df, wnd,
					CTRL_GROUPEDITSTR, NULL, FALSE);

			break;
			}

		case CSGM_KILL:
			{
			label df;

			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				label Buffer;
				strcpy(df, (((listerData *)
						(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, Buffer,
						sizeof(Buffer)));
				}
			else
				{
				df[0] = 0;
				}

			CitWindowsGetString(getsysmsg(248), LABELSIZE, df, wnd,
					CTRL_GROUPKILLSTR, NULL, FALSE);

			break;
			}
		}

	return (TRUE);
	}

const char *groupLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length)
	{
	listerData *ld = (listerData *) wnd->LocalData;

	switch (cmd)
		{
		case LST_DRAW:
			{
			int i, j;

			if (!GroupData[ld->on].IsInuse())
				{
				ld->on = 0; 	// root
				}

			if (ld->on < ld->top)
				{
				ld->top = ld->on;
				}

			if (buildClipArray(wnd))
				{
				for (j = 1, i = ld->top; i < cfg.maxgroups; i++)
					{
					if (j >= wnd->extents.bottom - wnd->extents.top)
						{
						break;
						}

					if (GroupData[i].IsInuse())
						{
						int x;
						int r = min((LABELSIZE + 4),
								(wnd->extents.right - wnd->extents.left));
						label GroupName;
						GroupData[i].GetName(GroupName, sizeof(GroupName));

						ld->bottom = i;

						CitWindowOutChr(wnd, 1, j, ' ',
								i == ld->on ? cfg.cattr : cfg.attr);

						CitWindowOutStr(wnd, 2, j, GroupName,
								i == ld->on ? cfg.cattr : cfg.attr);

						for (x = strlen(GroupName) + 2; x < r; x++)
							{
							CitWindowOutChr(wnd, x, j, ' ',
									i == ld->on ? cfg.cattr : cfg.attr);
							}

						if (x == LABELSIZE + 4)
							{
							char Buffer[80];
							GroupData[i].GetDescription(Buffer,
									sizeof(Buffer));

							if (!*Buffer)
								{
								CitWindowOutStr(wnd, x, j, getsysmsg(249),
										i == ld->on ? cfg.cattr : cfg.attr);

								x += strlen(getsysmsg(249));
								}
							else
								{
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

			while (++ld->on < cfg.maxgroups && !GroupData[ld->on].IsInuse());
			if (ld->on >= cfg.maxgroups)
				{
				ld->on = tmp;
				}

			if (ld->on > ld->bottom)
				{
				while (!GroupData[++ld->top].IsInuse());
				}

			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_UP:
			{
			while (ld->on && !GroupData[--ld->on].IsInuse());
			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGDN:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				int tmp = ld->on;

				while (++ld->on < cfg.maxgroups &&
						!GroupData[ld->on].IsInuse());

				if (ld->on >= cfg.maxgroups)
					{
					ld->on = tmp;
					}

				if (ld->on > ld->bottom)
					{
					while (!GroupData[++ld->top].IsInuse());
					}
				}

			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGUP:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				while (ld->on && !GroupData[--ld->on].IsInuse());
				}

			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_GET:
			{
			return (GroupData[ld->on].GetName(Buffer, Length));
			}

		case LST_HOME:
			{
			ld->on = 0;
			ld->top = 0;
			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_END:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;
			int tmp;

			for (tmp = ld->on; ; tmp = ld->on)
				{
				while (++ld->on < cfg.maxgroups &&
						!GroupData[ld->on].IsInuse());

				if (ld->on >= cfg.maxgroups)
					{
					ld->on = tmp;
					break;
					}
				}

			ld->top = ld->on;
			for (i = 0; i < j; i++)
				{
				while (ld->top && !GroupData[--ld->top].IsInuse());
				}

			groupLister(wnd, LST_DRAW, NULL, 0);
			break;
			}
		}

	return (NULL);
	}

#endif
