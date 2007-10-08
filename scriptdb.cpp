// --------------------------------------------------------------------------
// Citadel: ScriptDB.CPP
//
// The script debugger and profiler

#include "ctdl.h"
#pragma hdrstop

#include "script.h"
#include "msg.h"
#include "log.h"

// --------------------------------------------------------------------------
// Contents


// Debugging stuff.
Bool dwHandler(EVENT evt, long param, int more, CITWINDOW *wnd);
static Bool wwHandler(EVENT evt, long param, int more, CITWINDOW *wnd);
static Bool swHandler(EVENT evt, long param, int more, CITWINDOW *wnd);

struct swLocal
	{
	int ThisLine, LastLine;
	strList *FileData, *ThisLineData;
	};



void DebuggingMsg(const char *Msg)
	{
#ifndef WINCIT
	assert(pCurScript->DebuggingWindow);
	assert(pCurScript->DebuggingWindow->LocalData);

	memmove(pCurScript->DebuggingWindow->LocalData,
			((char *) pCurScript->DebuggingWindow->LocalData) + (conCols - 2),
			(conRows / 2 - 2) * (conCols - 2));

	memset(((char *) pCurScript->DebuggingWindow->LocalData) + (conRows / 2 - 2) *
			(conCols - 2), 0, conCols - 2);

	if (*Msg)
		{
		memcpy(((char *) pCurScript->DebuggingWindow->LocalData) + (conRows / 2 - 2) * (conCols - 2), Msg, strlen(Msg));
		}

	(pCurScript->DebuggingWindow->func)(EVT_DRAWINT, 0, 0, pCurScript->DebuggingWindow);
#endif
	}

enum
	{
	CTRL_EVALEXP,	CTRL_BPLINE,	CTRL_WATCHEXP,	CTRL_ABORT,
	CTRL_QUIT,
	};

static Bool HandleDebuggingKeyboardCommand(int Key)
	{
	switch (toupper(Key))
		{
		case ' ':
			{
			if (pCurScript->Pause)
				{
				pCurScript->SingleStep = TRUE;
				DebuggingMsg(getscrmsg(41));
				}

			return (TRUE);
			}

		case 'B':
			{
			CitWindowsGetString(getscrmsg(42), 10, ns, pCurScript->DebuggingWindow,
					CTRL_BPLINE, NULL, FALSE);
			return (TRUE);
			}

		case 'E':
			{
			CitWindowsGetString(getscrmsg(43), 60, ns, pCurScript->DebuggingWindow, 
					CTRL_EVALEXP, NULL, FALSE);
			return (TRUE);
			}

		case 'P':
			{
			DebuggingMsg(getscrmsg(44));
			pCurScript->Pause = TRUE;
			return (TRUE);
			}

		case 'Q':
			{
			CitWindowsGetYN(getscrmsg(48), 0, pCurScript->DebuggingWindow, CTRL_QUIT, NULL);
			return (TRUE);
			}

		case 'S':
			{
			if (!pCurScript->SourceWindow)
				{
#ifndef WINCIT
				swLocal *sw = new swLocal;

				if (sw)
					{
					memset(sw, 0, sizeof(*sw));

					WINDOWFLAGS flags;
					SRECT rect;

					memset(&flags, 0, sizeof(flags));

					flags.visible = TRUE;
					flags.showTitle = TRUE;
					flags.moveable = TRUE;
					flags.resize = TRUE;
					flags.minimize = TRUE;
					flags.maximize = TRUE;
					flags.close = TRUE;

					rect.top = 0;
					rect.left = 0;
					rect.bottom = conRows / 4;
					rect.right = conCols - 1;

					pCurScript->SourceWindow = makeCitWindow(swHandler, pCurScript->DebuggingWindow,
							getscrmsg(49), flags, rect, sw, FALSE);
					}
#endif
				}

			return (TRUE);
			}

		case 'U':
			{
			DebuggingMsg(getscrmsg(45));
			pCurScript->Pause = FALSE;
			return (TRUE);
			}

		case 'W':
			{
			CitWindowsGetString(getscrmsg(46), 60, ns, pCurScript->DebuggingWindow, CTRL_WATCHEXP, NULL, FALSE);
			return (TRUE);
			}

		case 'X':
			{
			CitWindowsGetYN(getscrmsg(47), 0, pCurScript->DebuggingWindow, CTRL_ABORT, NULL);
			return (TRUE);
			}
		}

	return (FALSE);
	}

static Bool swHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_INKEY:
			{
			if (param <= 255)
				{
				if (HandleDebuggingKeyboardCommand((int) param))
					{
					break;
					}
				}
			else
				{
				switch (param >> 8)
					{
					case CURS_UP:
						{
						(wnd->func)(EVT_DRAWINT, 0, 3, wnd);
						return (TRUE);
						}

					case CURS_DOWN:
						{
						(wnd->func)(EVT_DRAWINT, 0, 2, wnd);
						return (TRUE);
						}

					case CURS_HOME:
						{
						(wnd->func)(EVT_DRAWINT, 0, 1, wnd);
						return (TRUE);
						}
					}
				}

			return (defaultHandler(evt, param, more, wnd));
			}

		case EVT_NEWWINDOW:
			{
			FILE *fl;
			if ((fl = fopen(pCurScript->Name, FO_R)) != NULL)
				{
				char string[128];
				int ThisLine = 0;

				while (fgets(string, 128, fl))
					{
					if (string[strlen(string) - 1] == '\n')
						{
						string[strlen(string) - 1] = 0;
						}

					strList *sl = (strList *) addLL((void **) &((swLocal *) (wnd->LocalData))->FileData,
							sizeof(*sl) + strlen(string));

					if (sl)
						{
						ThisLine++;
						strcpy(sl->string, string);
						}
					else
						{
						break;
						}
					}

				((swLocal *) (wnd->LocalData))->LastLine = ThisLine;
				((swLocal *) (wnd->LocalData))->ThisLine = 1;
				((swLocal *) (wnd->LocalData))->ThisLineData = ((swLocal *) (wnd->LocalData))->FileData;

				fclose(fl);
				}
			else
				{
				strList *sl = (strList *) addLL((void **) &((swLocal *) (wnd->LocalData))->FileData,
						sizeof(*sl) + strlen(getscrmsg(50)));

				if (sl)
					{
					strcpy(sl->string, getscrmsg(50));
					}
				}

			break;
			}

		case EVT_DESTROY:
			{
			disposeLL((void **) &((swLocal *) (wnd->LocalData))->FileData);
			pCurScript->SourceWindow = NULL;
			break;
			}

		case EVT_DRAWINT:
			{
			int LinesVisible = wnd->extents.bottom - wnd->extents.top - 1;

			if (LinesVisible && wnd->flags.visible)
				{
				if (buildClipArray(wnd))
					{
					Bool Changed = FALSE;

					if (more == 1)
						{
						// Find current command line

						if (pCurScript->curCmd->line >= ((swLocal *) (wnd->LocalData))->ThisLine + LinesVisible)
							{
							((swLocal *) (wnd->LocalData))->ThisLine = pCurScript->curCmd->line - (LinesVisible / 2);
							Changed = TRUE;
							}

						if (pCurScript->curCmd->line < ((swLocal *) (wnd->LocalData))->ThisLine)
							{
							((swLocal *) (wnd->LocalData))->ThisLine = pCurScript->curCmd->line;
							Changed = TRUE;
							}
						}
					else if (more == 2)
						{
						// Down
						((swLocal *) (wnd->LocalData))->ThisLine++;
						Changed = TRUE;
						}
					else if (more == 3)
						{
						// Up
						((swLocal *) (wnd->LocalData))->ThisLine--;
						Changed = TRUE;
						}

					int CurLine = ((swLocal *) (wnd->LocalData))->ThisLine;

					strList *w;

					if (Changed)
						{
						w = (strList *) getLLNum(((swLocal *) (wnd->LocalData))->FileData, CurLine);
						((swLocal *) (wnd->LocalData))->ThisLineData = w;
						}
					else
						{
						w = ((swLocal *) (wnd->LocalData))->ThisLineData;
						}

					int i;
					for (i = 1; w && i < wnd->extents.bottom - wnd->extents.top;
							CurLine++, i++, w = (strList *) getNextLL(w))
						{
						CitWindowClearLine(wnd, i, cfg.attr);

						label LNum;
						sprintf(LNum, pctd, CurLine);

						Bool IsBP = FALSE;

						for (Breakpoints *bp = pCurScript->BPoints; bp; bp = (Breakpoints *) getNextLL(bp))
							{
							if (bp->LineNum == CurLine)
								{
								IsBP = TRUE;
								break;
								}
							}

						CitWindowOutStr(wnd, 1, i, LNum, IsBP ? cfg.cattr : cfg.attr);
						CitWindowOutStr(wnd, 5, i, w->string, CurLine == pCurScript->curCmd->line ? cfg.wattr : cfg.attr);
						}

					for (; i < wnd->extents.bottom - wnd->extents.top; i++)
						{
						CitWindowClearLine(wnd, i, cfg.attr);
						}

					freeClipArray();
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

static Bool wwHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_INKEY:
			{
			if (param <= 255)
				{
				if (HandleDebuggingKeyboardCommand((int) param))
					{
					break;
					}
				}

			return (defaultHandler(evt, param, more, wnd));
			}

		case EVT_DESTROY:
			{
			disposeLL((void **) &wnd->LocalData);
			pCurScript->WWindow = NULL;
			break;
			}

		case EVT_DRAWINT:
			{
			if (wnd->flags.visible)
				{
				if (buildClipArray(wnd))
					{
					strList *w;
					int i;

					for (i = 1, w = (strList *) wnd->LocalData; w && i < wnd->extents.bottom - wnd->extents.top;
							i++, w = (strList *) getNextLL(w))
						{
						label showWatch;
						char watchValue[256], display[90];

						CopyStringToBuffer(showWatch, w->string);
						strcpy(watchValue, evaluateString(w->string));
						watchValue[50] = 0;

						sprintf(display, getscrmsg(55), showWatch, watchValue);

						CitWindowClearLine(wnd, i, cfg.attr);
						CitWindowOutStr(wnd, 1, i, display, cfg.attr);
						}

					for (; i < wnd->extents.bottom - wnd->extents.top; i++)
						{
						CitWindowClearLine(wnd, i, cfg.attr);
						}

					freeClipArray();
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

static void CreateWatchWindow(void)
	{
	if (!pCurScript->WWindow)
		{
#ifndef WINCIT
		WINDOWFLAGS flags;
		SRECT rect;

		memset(&flags, 0, sizeof(flags));

		flags.visible = TRUE;
		flags.showTitle = TRUE;
		flags.moveable = TRUE;
		flags.minimize = TRUE;
		flags.maximize = TRUE;
		flags.resize = TRUE;

		rect.top = 0;
		rect.left = 0;
		rect.bottom = 2;
		rect.right = conCols / 2;

		pCurScript->WWindow = makeCitWindow(wwHandler, pCurScript->DebuggingWindow, getscrmsg(36), flags, rect, NULL, FALSE);
#endif
		}
	}

static strList *findWatch(const char *Expr)
	{
	strList *w = NULL;

	if (pCurScript->WWindow)
		{
		for (w = (strList *) pCurScript->WWindow->LocalData; w; w = (strList *) getNextLL(w))
			{
			if (SameString(w->string, Expr))
				{
				break;
				}
			}
		}

	return (w);
	}

Bool RemoveWatch(const char *Expr)
	{
	if (pCurScript->WWindow)
		{
		strList *w;
		int i;

		for (i = 1, w = (strList *) pCurScript->WWindow->LocalData; w; w = (strList *) getNextLL(w), i++)
			{
			if (SameString(w->string, Expr))
				{
				break;
				}
			}

		if (w)
			{
			deleteLLNode((void **) &pCurScript->WWindow->LocalData, i);

			if (!pCurScript->WWindow->LocalData)
				{
				destroyCitWindow(pCurScript->WWindow, FALSE);
				pCurScript->WWindow = NULL;
				}
			else
				{
				(pCurScript->WWindow->func)(EVT_DRAWINT, 0, 0, pCurScript->WWindow);
				}

			return (TRUE);
			}
		else
			{
			return (FALSE);
			}
		}
	else
		{
		return (FALSE);
		}
	}

Bool AddWatch(const char *Expr)
	{
	CreateWatchWindow();

	if (pCurScript->WWindow)
		{
		strList *w = (strList *) addLL((void **) &pCurScript->WWindow->LocalData, sizeof(*w) + strlen(Expr));

		if (w)
			{
			strcpy(w->string, Expr);

			(pCurScript->WWindow->func)(EVT_DRAWINT, 0, 0, pCurScript->WWindow);
			return (TRUE);
			}
		else
			{
			return (FALSE);
			}
		}
	else
		{
		return (FALSE);
		}
	}

static void ToggleWatch(const char *Expr)
	{
	CreateWatchWindow();

	if (pCurScript->WWindow)
		{
		strList *w = findWatch(Expr);

		if (w)
			{
			DebuggingMsg(getscrmsg(37));
			RemoveWatch(Expr);
			}
		else
			{
			if (AddWatch(Expr))
				{
				DebuggingMsg(getscrmsg(38));
				}
			else
				{
				DebuggingMsg(getscrmsg(39));
				}
			}
		}
	else
		{
		DebuggingMsg(getscrmsg(40));
		}
	}

Bool dwHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_CTRLRET:
			{
			CONTROLINFO *ci = (CONTROLINFO *) param;

			switch (ci->id)
				{
				case CTRL_ABORT:
					{
					pCurScript->Error = *((int *)(ci->ptr));
					break;
					}

				case CTRL_QUIT:
					{
					pCurScript->Debug = !*((int *)(ci->ptr));
					break;
					}

				case CTRL_EVALEXP:
					{
					char wow[256];

					sprintf(wow, getscrmsg(29), ci->ptr);
					DebuggingMsg(wow);

					if (evaluateString((char *) ci->ptr))
					strcpy(wow, evaluateString((char *) ci->ptr));
					wow[70] = 0;
					DebuggingMsg(wow);

					break;
					}

				case CTRL_BPLINE:
					{
					int bpLine = atoi((char *) ci->ptr);

					if (bpLine > 0)
						{
						Breakpoints *bp;
						int i;

						for (i = 1, bp = pCurScript->BPoints; bp; i++, bp = (Breakpoints *) getNextLL(bp))
							{
							if (bp->LineNum == bpLine)
								{
								break;
								}
							}

						if (bp)
							{
							DebuggingMsg(getscrmsg(33));
							deleteLLNode((void **) &pCurScript->BPoints, i);
							}
						else
							{
							bp = (Breakpoints *) addLL((void **) &pCurScript->BPoints, sizeof(*bp));

							if (bp)
								{
								DebuggingMsg(getscrmsg(34));
								bp->LineNum = bpLine;
								}
							else
								{
								DebuggingMsg(getscrmsg(35));
								}
							}

						if (pCurScript->SourceWindow)
							{
							(pCurScript->SourceWindow->func)(EVT_DRAWINT, 0, 0, pCurScript->SourceWindow);
							}
						}

					break;
					}

				case CTRL_WATCHEXP:
					{
					ToggleWatch((char *) ci->ptr);
					break;
					}
				}

			break;
			}

		case EVT_DRAWINT:
			{
			if (wnd->flags.visible)
				{
#ifndef WINCIT
				if (buildClipArray(wnd))
					{
					int h = wnd->extents.bottom - wnd->extents.top;
					int w = wnd->extents.right - wnd->extents.left;

					int i;
					for (i = 1; i < h; i++)
						{
						if (i >= wnd->extents.bottom - wnd->extents.top)
							{
							break;
							}

						int off = conRows / 2 - h + i - 1;

						if (off < 0)
							{
							off = 0;
							}

						for (int j = 1; j < w; j++)
							{
							CitWindowOutChr(wnd, j, i, *(((char *) wnd->LocalData) + off * (conCols - 2) + j - 1), cfg.attr);
							}
						}

					for (; i < h; i++)
						{
						CitWindowClearLine(wnd, i, cfg.attr);
						}

					freeClipArray();
					}
#endif
				}

			break;
			}

		case EVT_INKEY:
			{
			if (param <= 255)
				{
				if (HandleDebuggingKeyboardCommand((int) param))
					{
					break;
					}
				}

			return (defaultHandler(evt, param, more, wnd));
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}
