// --------------------------------------------------------------------------
// Citadel: CSysRoom.CPP
//
// Console Sysop Room

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "group.h"
#include "cwindows.h"
#include "consysop.h"
#include "extmsg.h"


// all of the control IDs used in this module
enum
	{
	CTRL_ROOMKILLSTR,	CTRL_ROOMKILLYN,
	};

const char *roomLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length)
	{
	listerData *ld = (listerData *) wnd->LocalData;

	switch (cmd)
		{
		case LST_DRAW:
			{
			int i, j;

			if (!RoomTab[RoomPosTableToRoom(ld->on)].IsInuse())
				{
				ld->on = LOBBY;
				}

			if (ld->on < ld->top)
				{
				ld->top = ld->on;
				}

			if (buildClipArray(wnd))
				{
				for (j = 1, i = ld->top; i < cfg.maxrooms; i++)
					{
					if (j >= wnd->extents.bottom - wnd->extents.top)
						{
						break;
						}

					if (RoomTab[RoomPosTableToRoom(i)].IsInuse())
						{
						int x;
						int r = min((LABELSIZE + 4),
								(wnd->extents.right - wnd->extents.left));

						ld->bottom = i;

						CitWindowOutChr(wnd, 1, j, ' ',
								i == ld->on ? cfg.cattr : cfg.attr);

						label RoomName;
						RoomTab[RoomPosTableToRoom(i)].GetName(RoomName,
								sizeof(RoomName));

						CitWindowOutStr(wnd, 2, j, RoomName,
								i == ld->on ? cfg.cattr : cfg.attr);

						for (x = strlen(RoomName) + 2; x < r; x++)
							{
							CitWindowOutChr(wnd, x, j, ' ',
									i == ld->on ? cfg.cattr : cfg.attr);
							}

						if (x == LABELSIZE + 4)
							{
							if (!RoomTab[RoomPosTableToRoom(i)].IsGroupOnly())
								{
								CitWindowOutStr(wnd, x, j, getsysmsg(338),
										i == ld->on ? cfg.cattr : cfg.attr);

								x += strlen(getsysmsg(338));
								}
							else
								{
								if (RoomTab[RoomPosTableToRoom(i)].IsBooleanGroup())
									{
									CitWindowOutStr(wnd, x, j, getsysmsg(339),
											i == ld->on ? cfg.cattr : cfg.attr);

									x += strlen(getsysmsg(339));
									}
								else
									{
									label gn;
									GroupData[RoomTab[RoomPosTableToRoom(i)].
											GetGroupNumber()].GetName(gn,
											sizeof(gn));

									CitWindowOutStr(wnd, x, j, gn,
											i == ld->on ? cfg.cattr : cfg.attr);

									x += strlen(gn);
									}
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

			while (++ld->on < cfg.maxrooms &&
					!RoomTab[RoomPosTableToRoom(ld->on)].IsInuse());
			if (ld->on >= cfg.maxrooms)
				{
				ld->on = tmp;
				}

			if (ld->on > ld->bottom)
				{
				while (!RoomTab[RoomPosTableToRoom(++ld->top)].IsInuse());
				}

			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_UP:
			{
			while (ld->on && !RoomTab[RoomPosTableToRoom(--ld->on)].IsInuse());
			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGDN:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				int tmp = ld->on;

				while (++ld->on < cfg.maxrooms &&
						!RoomTab[RoomPosTableToRoom(ld->on)].IsInuse());
				if (ld->on >= cfg.maxrooms)
					{
					ld->on = tmp;
					}

				if (ld->on > ld->bottom)
					{
					while (!RoomTab[RoomPosTableToRoom(++ld->top)].IsInuse());
					}
				}

			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGUP:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				while (ld->on && !RoomTab[RoomPosTableToRoom(--ld->on)].IsInuse());
				}

			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_GET:
			{
			return (RoomTab[RoomPosTableToRoom(ld->on)].GetName(Buffer, Length));
			}

		case LST_HOME:
			{
			ld->on = 0;
			ld->top = 0;
			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_END:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;
			int tmp;

			for (tmp = ld->on; ; tmp = ld->on)
				{
				while (++ld->on < cfg.maxrooms && !RoomTab[ld->on].IsInuse());

				if (ld->on >= cfg.maxrooms)
					{
					ld->on = tmp;
					break;
					}
				}

			ld->top = ld->on;
			for (i = 0; i < j; i++)
				{
				while (ld->top && !RoomTab[--ld->top].IsInuse());
				}

			roomLister(wnd, LST_DRAW, NULL, 0);
			break;
			}
		}

	return (NULL);
	}

void RoomControlHandler(CITWINDOW *wnd, long param)
	{
	CONTROLINFO *ci = (CONTROLINFO *) param;

	switch (ci->id)
		{
		case CTRL_ROOMKILLSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				r_slot rs;

				rs = PartialRoomExists((char *) (ci->ptr), 0, TRUE);

				if (rs >= 0 && rs <= 3)
					{
					label Buffer;
					CitWindowsError(wnd, getsysmsg(340), RoomTab[rs].GetName(
							Buffer, sizeof(Buffer)));
					}
				else if (rs != CERROR)
					{
					r_slot *rsp;

					if ((rsp = (r_slot *) getMemG(sizeof(*rsp), 1)) != NULL)
						{
						char pmt[80];
						label rn;

						RoomTab[rs].GetName(rn, sizeof(rn));

						sprintf(pmt, getsysmsg(341), rn);

						*rsp = rs;

						CitWindowsGetYN(pmt, 0, wnd, CTRL_ROOMKILLYN,
								rsp);
						}
					}
				else
					{
					CitWindowsError(wnd, getsysmsg(342), ci->ptr);
					}
				}

			break;
			}

		case CTRL_ROOMKILLYN:
			{
			if (*((int *)(ci->ptr)))
				{
				label rn;

				RoomTab[*((r_slot *)(ci->more))].GetName(rn, sizeof(rn));

				if (!killroom(rn))
					{
					CitWindowsError(wnd, getsysmsg(343), rn);
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
		}
	}

Bool RoomKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSRM_EXIT:
		case MK_ESC:
			{
			// exit hall menu stuff
			return (FALSE);
			}

		case CSRM_NEW:
			{
			break;
			}

		case CSRM_EDIT:
			{
			break;
			}

		case CSRM_KILL:
			{
			label df;

			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				(((listerData *) (wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, df, sizeof(df));
				}
			else
				{
				*df = 0;
				}

			CitWindowsGetString(getsysmsg(344), LABELSIZE, df, wnd,
					CTRL_ROOMKILLSTR, NULL, FALSE);

			break;
			}

		case CSRM_MOVEBACK:
			{
			break;
			}

		case CSRM_MOVEUP:
			{
			break;
			}

		case CSRM_PREVHALL:
			{
			break;
			}

		case CSRM_NEXTHALL:
			{
			break;
			}
		}

	return (TRUE);
	}
#endif
