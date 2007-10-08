// --------------------------------------------------------------------------
// Citadel: CSysUser.CPP
//
// Console Sysop User

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "cwindows.h"
#include "consysop.h"
#include "extmsg.h"


// all of the control IDs used in this module
enum
	{
	CTRL_USERKILLSTR,	CTRL_USERKILLYN,
	};

const char *userLister(CITWINDOW *wnd, LISTCMDS cmd, char *Buffer, int Length)
	{
	listerData *ld = (listerData *) wnd->LocalData;

	switch (cmd)
		{
		case LST_DRAW:
			{
			int i, j;

			if (!LTab(ld->on).IsInuse())
				{
				ld->on = 0; 	// root
				}

			if (ld->on < ld->top)
				{
				ld->top = ld->on;
				}

			if (buildClipArray(wnd))
				{
				for (j = 1, i = ld->top; i < cfg.MAXLOGTAB; i++)
					{
					if (j >= wnd->extents.bottom - wnd->extents.top)
						{
						break;
						}

					if (LTab(i).IsInuse())
						{
						int x;

						ld->bottom = i;

						CitWindowOutChr(wnd, 1, j, ' ',
								i == ld->on ? cfg.cattr : cfg.attr);

						label UserName;
						LTab(i).GetName(UserName, sizeof(UserName));

						CitWindowOutStr(wnd, 2, j, UserName,
								i == ld->on ? cfg.cattr : cfg.attr);

						for (x = strlen(UserName) + 2;
								x < wnd->extents.right - wnd->extents.left;
								x++)
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

			while (++ld->on < cfg.MAXLOGTAB && !LTab(ld->on).IsInuse());
			if (ld->on >= cfg.MAXLOGTAB)
				{
				ld->on = tmp;
				}

			if (ld->on > ld->bottom)
				{
				while (!LTab(++ld->top).IsInuse());
				}

			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_UP:
			{
			while (ld->on && !LTab(--ld->on).IsInuse());
			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGDN:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				int tmp = ld->on;

				while (++ld->on < cfg.MAXLOGTAB && !LTab(ld->on).IsInuse());
				if (ld->on >= cfg.MAXLOGTAB)
					{
					ld->on = tmp;
					}

				if (ld->on > ld->bottom)
					{
					while (!LTab(++ld->top).IsInuse());
					}
				}

			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_PGUP:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;

			for (i = 0; i < j; i++)
				{
				while (ld->on && !LTab(--ld->on).IsInuse());
				}

			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_GET:
			{
			return (LTab(ld->on).GetName(Buffer, Length));
			}

		case LST_HOME:
			{
			ld->on = 0;
			ld->top = 0;
			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}

		case LST_END:
			{
			int i, j = wnd->extents.bottom - wnd->extents.top - 2;
			int tmp;

			for (tmp = ld->on; ; tmp = ld->on)
				{
				while (++ld->on < cfg.MAXLOGTAB && !LTab(ld->on).IsInuse());

				if (ld->on >= cfg.MAXLOGTAB)
					{
					ld->on = tmp;
					break;
					}
				}

			ld->top = ld->on;
			for (i = 0; i < j; i++)
				{
				while (ld->top && !LTab(--ld->top).IsInuse());
				}

			userLister(wnd, LST_DRAW, NULL, 0);
			break;
			}
		}

	return (NULL);
	}

void UserControlHandler(CITWINDOW *wnd, long param)
	{
	CONTROLINFO *ci = (CONTROLINFO *) param;

	switch (ci->id)
		{
		case CTRL_USERKILLSTR:
			{
			if (*((char *)(ci->ptr)))
				{
				int ls;

				ls = FindPersonByPartialName((char *) (ci->ptr));

				if (ls != CERROR)
					{
					l_slot *lsp;

					if (loggedIn && ThisSlot == ls)
						{
						label Buffer;
						CitWindowsError(wnd, getsysmsg(345),
								CurrentUser->GetName(Buffer, sizeof(Buffer)));
						}
					else
						{
						if ((lsp = new l_slot) != NULL)
							{
							char pmt[80];
							label ln;

							LTab(ls).GetName(ln, sizeof(ln));

							sprintf(pmt, getsysmsg(346), ln);

							*lsp = ls;

							CitWindowsGetYN(pmt, 0, wnd, CTRL_USERKILLYN,
									lsp);
							}
						}
					}
				else
					{
					CitWindowsError(wnd, getsysmsg(347), ci->ptr);
					}
				}

			break;
			}

		case CTRL_USERKILLYN:
			{
			if (*((int *)(ci->ptr)))
				{
				label ln;

				LTab(*((int *)(ci->more))).GetName(ln, sizeof(ln));

				if (!killuser(ln))
					{
					CitWindowsError(wnd, getsysmsg(348), ln);
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
		}
	}

Bool UserKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSUM_EXIT:
		case MK_ESC:
			{
			// exit user menu stuff
			return (FALSE);
			}

		case CSUM_NEW:
			{
			break;
			}

		case CSUM_EDIT:
			{
			break;
			}

		case CSUM_KILL:
			{
			label df;

			if (wnd->children && wnd->children->wnd->func == listHandler)
				{
				(((listerData *)(wnd->children->wnd->LocalData))->lister)
						(wnd->children->wnd, LST_GET, df, sizeof(df));
				}
			else
				{
				*df = 0;
				}

			CitWindowsGetString(getsysmsg(349), LABELSIZE, df, wnd,
					CTRL_USERKILLSTR, NULL, FALSE);

			break;
			}
		}

	return (TRUE);
	}

#endif
