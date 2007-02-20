// --------------------------------------------------------------------------
// Citadel: CSysDosU.CPP
//
// Console Sysop Dos Utilities

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "consysop.h"
#include "miscovl.h"

typedef struct
	{
	fpath path;
	directoryinfo *di;
	int entries;
	} fileListerData;

const char *fileLister(CITWINDOW *wnd, LISTCMDS cmd, char *, int)
	{
	listerData *ld = (listerData *) wnd->LocalData;
	fileListerData *fld = NULL;

	if (ld)
		{
		fld = (fileListerData *) ld->ptr;
		}

	switch (cmd)
		{
		case LST_DRAW:
			{
			if (fld)
				{
				if (ld->on > fld->entries - 1)
					{
					ld->on = 0;
					}

				if (ld->on < ld->top)
					{
					ld->top = ld->on;
					}

				if (buildClipArray(wnd))
					{
					int i, j, x;
					int r = wnd->extents.right - wnd->extents.left;

					if (wnd->extents.bottom - wnd->extents.top > 1)
						{
						CitWindowOutStr(wnd, 1, 1, fld->path, cfg.attr);

						for (x = strlen(fld->path) + 1; x < r; x++)
							{
							CitWindowOutChr(wnd, x, 1, ' ', cfg.attr);
							}
						}

					for (j = 2, i = ld->top; i < fld->entries - 1; i++, j++)
						{
						if (j >= wnd->extents.bottom - wnd->extents.top)
							{
							break;
							}

						ld->bottom = i;

						CitWindowOutChr(wnd, 1, j, ' ',
								i == ld->on ? cfg.cattr : cfg.attr);

						CitWindowOutStr(wnd, 2, j, fld->di[i].name,
								i == ld->on ? cfg.cattr : cfg.attr);

						for (x = strlen(fld->di[i].name) + 2; x < r; x++)
							{
							CitWindowOutChr(wnd, x, j, ' ',
									i == ld->on ? cfg.cattr : cfg.attr);
							}
						}

					for (; j < wnd->extents.bottom - wnd->extents.top; j++)
						{
						CitWindowClearLine(wnd, j, cfg.attr);
						}

					freeClipArray();
					}
				}

			break;
			}

		case LST_NEW:
			{
			ld->ptr = getMemG(sizeof(fileListerData), 1);
			break;
			}

		case LST_DESTROY:
			{
			if (fld)
				{
				delete [] fld->di;

				freeMemG(fld);
				}

			break;
			}

		case LST_REFRESH:
			{
			if (fld)
				{
				getfpath(fld->path);
				fld->di = filldirectory(starstar, SORT_ALPHA, OldAndNew,
						FALSE);

				if (fld->di)
					{
					for (fld->entries = 0; fld->di[fld->entries].name[0];
							++(fld->entries));
					}
				}

			break;
			}
		}

	return (NULL);
	}

Bool DOSUtilKeyHandler(CITWINDOW *, int key)
	{
	switch (key)
		{
		case CSDM_EXIT:
		case MK_ESC:
			{
			// exit user menu stuff
			return (FALSE);
			}

		case CSDM_COPY:
			{
			break;
			}

		case CSDM_MOVE:
			{
			break;
			}

		case CSDM_DELETE:
			{
			break;
			}

		case CSDM_RENAME:
			{
			break;
			}

		case CSDM_VIEW:
			{
			break;
			}

		case CSDM_MAKEDIR:
			{
			break;
			}

		case CSDM_RMVDIR:
			{
			break;
			}

		case CSDM_MEMDUMP:
			{
			memoryDump();
			break;
			}
		}

	return (TRUE);
	}

#endif
