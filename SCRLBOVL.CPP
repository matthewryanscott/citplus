// --------------------------------------------------------------------------
// Citadel: ScrlBOvl.CPP
//
// Overlayed parts of the scroll-back buffer

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "scrlback.h"
#include "auxtab.h"
#include "events.h"
#include "cwindows.h"
#ifndef WINCIT
#include "statline.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
// DisplayScrollBackBuffer()		Displays contents on-screen.


static void GetFromScrollBackBuffer(int DisplayLine, long LinesBack);
static void GetLineFromScrollBackBuffer(char *Line, long LinesBack);

extern char *scrollBuf; 				// where the buffer is
extern long lastScrollBufLine;			// the first line in buffer
extern long firstScrollBufLine; 		// the last line in buffer
#ifdef AUXMEM
extern int LinesPerPage;				// 0 if all in heap
extern int sbBlocksInHeap;
#endif

Bool CreateScrollBackBuffer() // return FALSE if failed to create
	{
	DestroyScrollBackBuffer();

#ifdef AUXMEM
	if (cfg.scrollSize * conCols * (cfg.scrollColors + 1) <= AUXPAGESIZE)
		{
		LinesPerPage = 0;
		scrollBuf = new char[((int) cfg.scrollSize) * conCols *
				(cfg.scrollColors + 1)];
		}
	else
		{
		LinesPerPage = AUXPAGESIZE / (conCols * (cfg.scrollColors + 1));

		for (int i = 0; i < cfg.scrollSize; i += LinesPerPage)
			{
			if (!AddAuxmemBlock(&((auxTabList *) scrollBuf), i,
					&sbBlocksInHeap, 1))
				{
				cfg.scrollSize = i;
				break;
				}
			}
		}
#else
	if (cfg.scrollSize)
		{
		scrollBuf = new char[(int) cfg.scrollSize * conCols *
				(cfg.scrollColors + 1)];
		}
#endif
	lastScrollBufLine = firstScrollBufLine = 0;

	return (scrollBuf || !cfg.scrollSize);
	}

void DestroyScrollBackBuffer(void)
	{
	if (scrollBuf)
		{
#ifdef AUXMEM
		if (!LinesPerPage)
			{
			delete [] scrollBuf;
			}
		else
			{
			DeleteAuxmemList(&((auxTabList *) scrollBuf), &sbBlocksInHeap);
			}

		LinesPerPage = 0;
#else
		delete [] scrollBuf;
#endif
		}

	lastScrollBufLine = firstScrollBufLine = 0;
	scrollBuf = NULL;
	}


// --------------------------------------------------------------------------
// DisplayScrollBackBuffer(): Displays contents on-screen.

#define SWIDTH 80

static Bool sbGetString(const char *Prompt, char *Buffer, int BufferLength)
	{
	char Line25[SWIDTH + 1];
	int ipPos = 0;

	int offset = strlen(Prompt);
	memset(Line25, 32, SWIDTH);
	Line25[SWIDTH] = 0;
	strncpy(Line25, Prompt, offset);
	(*stringattr)(conRows, Line25, cfg.wattr, TRUE);

	position(conRows, offset + 1);
	if (*Buffer)
		{
		for (const char *p = Buffer; *p; p++)
			{
			(*charattr)(*p, cfg.uttr, TRUE);
			ipPos++;
			position(conRows, offset + ipPos + 1);
			}
		}
	curson();

	Bool Done = FALSE, Abort = FALSE;
	while (!(Done || Abort))
		{
		if ((*statcon)())
			{
			int inKey = (*getcon)();

			if (inKey <= 255)
				{
				if (inKey == 13)
					{
					Done = TRUE;
					}
				else if (inKey == ESC)
					{
					Abort = TRUE;
					}
				else if (inKey == 8)
					{
					if (ipPos)
						{
						Buffer[--ipPos] = 0;
						position(conRows, offset + ipPos + 1);
						(*charattr)(' ', cfg.wattr, TRUE);
						}
					else
						{
						doEvent(EVT_BEEP);
						#ifdef WINCIT
							MessageBeep(0);
						#else
							putchar(BELL);
						#endif
						}
					}
				else
					{
					if (ipPos < BufferLength)
						{
						Buffer[ipPos++] = inKey;
						Buffer[ipPos] = 0;
						(*charattr)(inKey, cfg.uttr, TRUE);
						position(conRows, offset + ipPos + 1);
						}
					else
						{
						doEvent(EVT_BEEP);
						#ifdef WINCIT
							MessageBeep(0);
						#else
							putchar(BELL);
						#endif
						}
					}
				}
			}
		}

	if (Abort)
		{
		Buffer[0] = 0;
		}

	cursoff();

	return (Buffer[0]);
	}

void DisplayScrollBackBuffer(void)
	{
	int inKey, scrollCnt = 0, reDisp = FALSE, j;
	long numBack = 0, sbLines;
	long lastTime;
	dowhattype oldDowhat;

	int scurrow = logiRow, scurcol = logiCol;

	if (StatusLine.IsFullScreen() || ScreenSaver.IsOn() || allWindows ||
			!cfg.scrollSize)
		{
		return;
		}

	oldDowhat = DoWhat;
	SetDoWhat(SCROLLBACK);

	if (save_screen())
		{
		if (firstScrollBufLine < lastScrollBufLine)
			{
			sbLines = lastScrollBufLine - firstScrollBufLine;
			}
		else
			{
			sbLines = lastScrollBufLine + cfg.scrollSize - firstScrollBufLine;
			}

		cursoff();
		time(&lastTime);

		if (StatusLine.IsVisible())
			{
			char localStatus[SWIDTH + 1];
			localStatus[SWIDTH] = 0;
			memset(localStatus, ' ', SWIDTH);

			(*stringattr)(conRows, localStatus, cfg.wattr, TRUE);

			sprintf(localStatus, getmsg(136), ltoac(numBack),
					numBack == 1 ? ns : justs);
			(*stringattr)(conRows, localStatus, cfg.wattr, TRUE);
			}

		do
			{
			if ((*statcon)())
				{
				time(&lastTime);

				switch (inKey = (*getcon)())
					{
					case 'g':
					case 'G':
						{
						label NewLine;

						*NewLine = 0;

						if (sbGetString(getmsg(161), NewLine, LABELSIZE))
							{
							numBack = atol(NewLine);
							reDisp = TRUE;
							}

						break;
						}

					case 's':
					case 'S':
					case 'b':
					case 'B':
						{
						static label SearchString;

						if (sbGetString(getmsg(240), SearchString, LABELSIZE))
							{
							char *Buffer = new char[conCols + 1];

							if (Buffer)
								{
								long cur, limit;
								int direction;

								if ('S' == toupper(inKey))
									{
									cur = numBack - 1L;
									limit = 0;
									direction = -1;

									if (cur < limit)
										{
										cur = limit;
										}
									}
								else
									{
									cur = numBack + 1L;
									limit = sbLines;
									direction = 1;

									if (cur > limit)
										{
										cur = limit;
										}
									}

								for (; cur != limit; cur += direction)
									{
									GetLineFromScrollBackBuffer(Buffer, cur);

									if (IsSubstr(Buffer, SearchString))
										{
										numBack = cur;
										reDisp = TRUE;
										break;
										}
									}

								if (!reDisp)
									{
									doEvent(EVT_BEEP);
									#ifdef WINCIT
										MessageBeep(0);
									#else
										putchar(BELL);
									#endif
									}

								delete [] Buffer;
								}
							}

						break;
						}

					case 'w':
					case 'W':
						{
						label FileName;
						*FileName = 0;

						if (sbGetString(getmsg(270), FileName, LABELSIZE))
							{
							char Lines[11];
							*Lines = 0;

							if (sbGetString(getmsg(271), Lines, 10))
								{
								long toWrite = atol(Lines);

								if (toWrite)
									{
									char *Buffer = new char[conCols + 1];

									if (Buffer)
										{
										char FullFileName[128];
										FILE *fl;

										if (!strchr(FileName, ':') &&
												!strchr(FileName, '\\'))
											{
											sprintf(FullFileName, sbs,
													cfg.homepath, FileName);
											}
										else
											{
											CopyStringToBuffer(FullFileName,
													FileName);
											}

										if ((fl = fopen(FullFileName, FO_A)) !=
												NULL)
											{
											long cur;

											for (cur = numBack;
													(cur > numBack - toWrite)
													&&
													(cur >= (0 - scrollpos));
													cur--)
												{
												if (cur > 0)
													{
													GetLineFromScrollBackBuffer(Buffer, cur);
													}
												else
													{
													int j;
													for (j = 0; j < conCols; j++)
														{
														Buffer[j] = *(saveBuffer +
																(0 - (int) cur) *
																conCols * 2 + j * 2);

														if (Buffer[j] == 0)
															{
															Buffer[j] = ' ';
															}
														}

													Buffer[j] = 0;
													}

												int Last = strlen(Buffer) - 1;

												while (Last && Buffer[Last] == 32)
													{
													Buffer[Last--] = 0;
													}

												fprintf(fl, getmsg(472), Buffer, bn);
												}

											fclose(fl);
											}

										delete [] Buffer;
										}
									}
								}
							}

						break;
						}

					case CURS_HOME << 8:
						{
						numBack = sbLines;
						reDisp = TRUE;
						break;
						}

					case CURS_UP << 8:
						{
						numBack++;
						scrollCnt--;
						break;
						}

					case PGUP << 8:
						{
						numBack += scrollpos;
						reDisp = TRUE;
						break;
						}

					case CURS_END << 8:
						{
						if (numBack)
							{
							numBack = 0;
							reDisp = TRUE;
							}

						break;
						}

					case CURS_DOWN << 8:
						{
						numBack--;
						scrollCnt++;
						break;
						}

					case PGDN << 8:
						{
						numBack -= scrollpos;
						reDisp = TRUE;
						break;
						}
					}

				if (numBack < 0)
					{
					numBack = 0;
					scrollCnt = 0;
					}

				if (numBack > sbLines)
					{
					numBack = sbLines;
					scrollCnt = 0;
					}

				if (reDisp)
					{
					for (j = 0; j <= scrollpos; j++)
						{
						if (j < numBack)
							{
							GetFromScrollBackBuffer(j, numBack - j);
							}
						else
							{
							memcpy(logiScreen + j * conCols * 2,
									saveBuffer + (j - ((int) numBack)) *
									conCols * 2, conCols * 2);
							}
						}
					}

				if (scrollCnt < 0)
					{
					_fmemmove(logiScreen + conCols * 2, logiScreen,
							scrollpos * conCols * 2);

					GetFromScrollBackBuffer(0, numBack);
					}
				else if (scrollCnt)
					{
					_fmemmove(logiScreen, logiScreen + conCols * 2,
							scrollpos * conCols * 2);

					if (scrollpos - numBack < 0)
						{
						GetFromScrollBackBuffer(scrollpos, numBack - scrollpos);
						}
					else
						{
						memcpy(logiScreen + scrollpos * conCols * 2,
								saveBuffer + (scrollpos - ((int) numBack)) *
								conCols * 2, conCols * 2);
						}
					}

				scrollCnt = 0;
				reDisp = FALSE;

				if (StatusLine.IsVisible())
					{
					char localStatus[SWIDTH + 1], localStatus2[SWIDTH + 1];
					localStatus[SWIDTH] = 0;
					memset(localStatus, ' ', SWIDTH);

					sprintf(localStatus2, getmsg(136), ltoac(numBack),
							numBack == 1 ? ns : justs);
					strncpy(localStatus, localStatus2, strlen(localStatus2));
					(*stringattr)(conRows, localStatus, cfg.wattr, TRUE);
					}
				}
			else
				{
				if (cfg.scrollTimeOut && time(NULL) >
						lastTime + cfg.scrollTimeOut)
					{
					inKey = ESC;
					}
				}
			} while (inKey != ESC);

		restore_screen();
		SetDoWhat(oldDowhat);
		position(scurrow, scurcol);
		curson();
		}
	}

static void GetFromScrollBackBuffer(int DisplayLine, long LinesBack)
	{
	long LineToDisplay = lastScrollBufLine - LinesBack + 1;

	if (LineToDisplay < 0)
		{
		LineToDisplay += cfg.scrollSize;
		}

	if (cfg.scrollColors)
		{
#ifdef AUXMEM
		if (LinesPerPage)
			{
			memcpy(logiScreen + DisplayLine * conCols * 2,
					LoadAuxmemBlock(LineToDisplay, &(auxTabList *) scrollBuf,
					LinesPerPage, conCols * 2), conCols * 2);
			}
		else
			{
			memcpy(logiScreen + DisplayLine * conCols * 2,
					scrollBuf + ((int) LineToDisplay) * conCols * 2,
					conCols * 2);
			}
#else
		memcpy(logiScreen + DisplayLine * conCols * 2,
				scrollBuf + ((int) LineToDisplay) * conCols * 2,
				conCols * 2);
#endif
		}
	else
		{
		for (int k = 0; k < conCols; k++)
			{
#ifdef AUXMEM
			if (LinesPerPage)
				{
				*((uint *) logiScreen + DisplayLine * conCols + k) =
						(*(((uchar *) (LoadAuxmemBlock(LineToDisplay,
						&(auxTabList *) scrollBuf, LinesPerPage, conCols))) +
						k)) + (cfg.attr << 8);
				}
			else
				{
				*((uint *) logiScreen + DisplayLine * conCols + k) =
						((uchar) *(scrollBuf + ((int) LineToDisplay) * conCols + k)) +
						(cfg.attr << 8);
				}
#else
			*((uint *) logiScreen + DisplayLine * conCols + k) =
					((uchar) *(scrollBuf + ((int) LineToDisplay) * conCols + k)) +
					(cfg.attr << 8);
#endif
			}
		}
	}

static void GetLineFromScrollBackBuffer(char *Line, long LinesBack)
	{
	long LineToGet = lastScrollBufLine - LinesBack + 1;

	if (LineToGet < 0)
		{
		LineToGet += cfg.scrollSize;
		}

	if (cfg.scrollColors)
		{
		for (int k = 0; k < conCols; k++)
			{
#ifdef AUXMEM
			if (LinesPerPage)
				{
				Line[k] = (char) *(((int *) LoadAuxmemBlock(LineToGet,
						&(auxTabList *) scrollBuf, LinesPerPage,
						conCols * 2)) + k);
				}
			else
				{
				Line[k] = (char) *((int *) scrollBuf + ((int) LineToGet) *
						conCols + k);
				}
#else
			Line[k] = (char) *((int *) scrollBuf + ((int) LineToGet) *
					conCols + k);
#endif
			if (Line[k] == 0)
				{
				Line[k] = ' ';
				}
			}
		}
	else
		{
#ifdef AUXMEM
		if (LinesPerPage)
			{
			memcpy(Line, LoadAuxmemBlock(LineToGet, &(auxTabList *) scrollBuf,
					LinesPerPage, conCols), conCols);
			}
		else
			{
			memcpy(Line, scrollBuf + ((int) LineToGet) * conCols, conCols);
			}
#else
		memcpy(Line, scrollBuf + ((int) LineToGet) * conCols, conCols);
#endif
		}

	Line[conCols] = 0;
	}
#endif
