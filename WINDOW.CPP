// --------------------------------------------------------------------------
// Citadel: Window.CPP
//
// Machine-dependent console routines.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "music.h"
#include "account.h"
#include "hall.h"
#include "scrlback.h"
#include "cwindows.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// scroll()					scrolls window up
// StatusLineC::Update()	updates the 25th line
// bioschar()				BIOS print char with attr
// biosstring()				BIOS print string w/attr at row
// position()				Set logical cursor position

extern ConsoleLockC ConsoleLock;


#ifndef WINCIT
void scroll_bios(int row, uchar howmany, uchar attr)
	{
	union REGS regs;
	
	const int rw = logiRow, col = logiCol;

	regs.h.al = howmany;		// scroll how many lines

	regs.h.cl = 0;				// row # of upper left hand corner
	regs.h.ch = 0;				// col # of upper left hand corner
	regs.h.dh = (uchar) row;	// row # of lower left hand corner
	regs.h.dl = (uchar)
				(conCols - 1);	// col # of lower left hand corner

	regs.h.ah = 0x06;			// scroll window up interrupt
	regs.h.bh = attr;			// char attribute for blank lines

	int86(0x10, &regs, &regs);

	// put cursor back
	position(rw, col);
	}

static void scroll_fast(int row, uchar howmany, uchar attr)
	{
	fastcpy(logiScreen, logiScreen + howmany * conCols * 2, (row - howmany + 1) * conCols * 2);
	fastsetw(logiScreen + ((row - howmany + 1) * conCols << 1), attr << 8, howmany * conCols);

	if (CitWindowsVid)
		{
		CitWindowsTTYUpdate(0, scrollpos);
		}
	}
#endif


// --------------------------------------------------------------------------
// scroll(): Scrolls window up from specified line.
//
// Input:
//	int row: Where to stop scrolling at (always starts at top of screen)
//	uchar howmany: How many to scroll
//	uchar attr: Attribute to scroll in

void TERMWINDOWMEMBER scroll(int row, int howmany, uchar attr)
	{
#ifndef WINCIT
	if (cfg.scrollSize)
		{
		SaveToScrollBackBuffer(howmany);
		}

	if (!cfg.bios || ScreenSaver.IsOn() || StatusLine.IsFullScreen() || allWindows)
		{
		scroll_fast(row, howmany, attr);
		}
	else
		{
		scroll_bios(row, howmany, attr);
		}
#else
	WaitForSingleObject(ScreenUpdateMutex, INFINITE);

	memmove(ScreenBuffer, ScreenBuffer + howmany * conCols, (row - howmany + 1) * conCols * sizeof(CHAR_INFO));

	int Offset = (row - howmany + 1) * conCols;
	for (int i = 0; i < howmany * conCols; i++)
		{
		ScreenBuffer[Offset + i].Char.AsciiChar = 0;
		ScreenBuffer[Offset + i].Attributes = attr;
		}

	ScreenUpdateRect.Left = 0;
	ScreenUpdateRect.Top = 0;
	ScreenUpdateRect.Bottom = cfg.ConsoleRows - 1;
	ScreenUpdateRect.Right = cfg.ConsoleCols - 1;
	ReleaseMutex(ScreenUpdateMutex);
#endif
	}


#ifndef WINCIT
// --------------------------------------------------------------------------
// biosstring(): Print a string with attribute.
//
// Input:
//	uint row: Where to print it
//	const char *str: What to print
//	ucahr attr: What attribute to use
//	Bool phys: TRUE to force output to the physical screen; FALSE to send
//		output to the logical screen.

void cdecl biosstring(uint row, const char *str, uchar attr, Bool phys)
	{
	union REGS regs;
	union REGS temp_regs;

	if (!ScreenSaver.IsOn() && !StatusLine.IsFullScreen() && !allWindows)
		{
		regs.h.ah = 9;			// service 9, write character & attribute
		regs.h.bl = attr;		// character attribute
		regs.x.cx = 1;			// number of character to write
		regs.h.bh = 0;			// display page

		for (register int i = 0; str[i]; i++)
			{
			position(row, i);	// Move cursor to the correct position
			regs.h.al = str[i]; // set character to write with 0x09
			int86(0x10, &regs, &temp_regs);
			}
		}
	else
		{
		directstring(row, str, attr, phys);
		}
	}


// --------------------------------------------------------------------------
// bioschar(): Print a char with attribute.
//
// Input:
//	char ch: The character to output
//	uchar attr: The attribute to use
//	Bool phys: TRUE to force output to the physical screen; FALSE to send to
//		the logical streen
//
// Return value:
//	TRUE: Ran out of space on the current line
//	FALSE: Still have room on the current line

Bool cdecl bioschar(char ch, uchar attr, Bool phys)
	{
	union REGS regs;

	if (!ScreenSaver.IsOn() && !StatusLine.IsFullScreen() && !allWindows)
		{
		regs.h.ah = 9;			// service 9, write char & attribute
		regs.h.bl = attr;		// character attribute
		regs.h.al = ch;			// char to write
		regs.x.cx = 1;			// write 1 character
		regs.h.bh = 0;			// display page
		int86(0x10, &regs, &regs);

		return (logiCol + 1 >= conCols);
		}
	else
		{
		return (directchar(ch, attr, phys));
		}
	}
#endif


// --------------------------------------------------------------------------
// ansi(): Handle ansi and music escape sequences.
//
// Input:
//	char c: The character we are outputing

void TERMWINDOWMEMBER ansi(char c)
	{
	if (c == ESC)
		{
		ANSIargs[0] = 0;
		ANSIfirst = TRUE;
		OC.inANSI = TRUE;
		return;
		}

	if (ANSIfirst)
		{
		ANSIfirst = FALSE;

		if (c == '[')
			{
			OC.inANSI = TRUE;
			return;
			}
		else
			{
			OC.inANSI = FALSE;

			char str[3];
			str[0] = ESC;
			str[1] = c;
			str[2] = 0;
			TermCap->putCode(str);

			return;
			}
		}

	if (ANSIisSound && c == 14)		// 14 is end of sound: ''
		{
		ANSIisSound = FALSE;
		OC.inANSI = FALSE;
		playSound(ANSIargs);
		}
	else if (isalpha(c) && !ANSIisSound) // if not sound, alpha ends ANSI
		{
		OC.inANSI = FALSE;

		int argc, a[5];
		for (argc = 0; argc < 5; ++argc)
			{
			a[argc] = 0;
			}

		argc = 0;

		const char *p = ANSIargs;
		while (*p && (argc < 5))
			{
			if (isdigit(*p))
				{
				a[argc++] = atoi(p);

				while (isdigit(*p))
					{
					p++;
					}
				}
			else
				{
				p++;
				}
			}

		//								row	col
		// ANSI:				origin= 1	1
		// position/readpos:	origin= 0	0
		// numLines/CrtColumn:	origin= 0	1

		switch (c)
			{
			case 'M':			// Sound stuff
				{
				ANSIisSound = TRUE;
				OC.inANSI = TRUE;
				return;
				}

			case 'J':			// cls
				{
				numLines = 0;
				OC.CrtColumn = 1;

				cls(SCROLL_SAVE);
				StatusLine.Update(WC_TWp);

				TermCap->SendClearScreen();
				break;
				}

			case 'K':			// del to end of line
				{
				ClearToEndOfLine();

				TermCap->SendDelEOL();
				break;
				}

			case 'n':			// auto ansi detect
				{
				if (a[0] == 6)
					{
					CommPort->OutString("[Z");
					}

				break;
				}

			case 'm':			// color
				{
				int NewFore = -1, NewBack = -1;

				for (int i = 0; i < argc; i++)
					{
					switch (a[i])
						{
						case 5: // Blink
							{
							OC.ansiattr = (uchar) (OC.ansiattr | 128);
							ANSIblink = TRUE;

							TermCap->SendBlink();
							break;
							}

						case 4: // Underline
							{
							OC.ansiattr = (uchar)(OC.ansiattr | 1);
							TermCap->SendUnderline();
							break;
							}

						case 7: // Reverse Video
							{
							OC.ansiattr = 0x70;
							if (ANSIhilight)
								{
								OC.ansiattr = (uchar) (OC.ansiattr | 8); // Bold
								}

							if (ANSIblink)
								{
								OC.ansiattr = (uchar) (OC.ansiattr | 128);// Blink
								}

							TermCap->SendReverse();

							break;
							}

						case 0: // Default
							{
							OC.ansiattr = 7;
							ANSIblink = FALSE;
							ANSIhilight = FALSE;

							TermCap->SendNormal();

							break;
							}

						case 1: // Bold
							{
							OC.ansiattr = (uchar) (OC.ansiattr | 8);
							ANSIhilight = TRUE;
							TermCap->SendBold();
							break;
							}

						default:
							{
							// Set the background
							if (a[i] >= 40 && a[i] <= 47)
								{
								OC.ansiattr = (uchar) ((OC.ansiattr & 0x0F) | ((iso_clr[a[i] - 40]) << 4) |
										(ANSIblink ? 0x80 : 0));

								NewBack = a[i] - 40;
								}

							// Set the foreground
							if (a[i] >= 30 && a[i] <= 37)
								{
								OC.ansiattr = (uchar) ((OC.ansiattr & 0xF0) | ((iso_clr[a[i] - 30]) |
										(ANSIhilight ? 0x08 : 0)));

								NewFore = a[i] - 30;
								}

							break;
							}
						}
					}

				if (NewFore != -1)
					{
					if (NewBack != -1)
						{
						TermCap->SendBoth(NewFore, NewBack);
						}
					else
						{
						TermCap->SendForeground(NewFore);
						}
					}
				else
					{
					if (NewBack != -1)
						{
						TermCap->SendBackground(NewBack);
						}
					}

				break;
				}

			case 's':			// save cursor
				{
				ANSIc_y = logiRow;
				ANSIc_x = logiCol;

				TermCap->SaveCursor();
				break;
				}

			case 'u':			// restore cursor
				{
				numLines = ANSIc_y;
				OC.CrtColumn = ANSIc_x + 1;

				position(ANSIc_y, ANSIc_x);

				TermCap->RestoreCursor();
				break;
				}

			case 'A':
				{
				const int Delta = (argc ? a[0] : 1);
				int NewRow = logiRow - Delta;

				if (NewRow < 0)
					{
					NewRow = 0;
					}

				numLines = NewRow;	// don't subtract
				position(NewRow, logiCol);

				TermCap->CursorUp(Delta);
				break;
				}

			case 'B':
				{
				const int Delta = (argc ? a[0] : 1);
				int NewRow = logiRow + Delta;

				if (NewRow > conRows - 1)
					{
					NewRow = conRows - 1;
					}

				numLines = NewRow;
				position(NewRow, logiCol);

				TermCap->CursorDown(Delta);
				break;
				}

			case 'D':
				{
				const int Delta = (argc ? a[0] : 1);
				int NewCol = logiCol - Delta;

				if (NewCol < 0)
					{
					NewCol = 0;
					}

				OC.CrtColumn = NewCol + 1;
				position(logiRow, NewCol);

				TermCap->CursorLeft(Delta);
				break;
				}

			case 'C':
				{
				const int Delta = (argc ? a[0] : 1);
				int NewCol = logiCol + Delta;

				if (NewCol > conCols - 1)
					{
					NewCol = conCols - 1;
					}

				OC.CrtColumn = NewCol + 1;
				position(logiRow, NewCol);

				TermCap->CursorRight(Delta);
				break;
				}

			case 'f':
			case 'H':
				{
				int NewRow = 0, NewCol = 0;

				if (argc == 1)
					{
					if (ANSIargs[0] == ';')
						{
						NewCol = 0;
						NewRow = a[0] - 1;
						}
					else
						{
						NewCol = a[0] - 1;
						NewRow = 0;
						}
					}
				else if (argc == 2)
					{
					NewCol = a[0] - 1;
					NewRow = a[1] - 1;
					}

				if (NewCol < 0)
					{
					NewCol = 0;
					}

				if (NewRow < 0)
					{
					NewRow = 0;
					}

				TermCap->CursorAbsolute(NewRow, NewCol);

				if (NewRow > scrollpos - 1)
					{
					NewRow = scrollpos - 1;
					}

				if (NewCol > conCols - 1)
					{
					NewCol= conCols - 1;
					}

				numLines = NewRow;
				OC.CrtColumn = NewCol + 1;

				position(NewRow, NewCol);
				break;
				}

			default:
				{
#ifdef GOODBYE
				if (debug)
					{
					sprintf(temp, "%s%c", ANSIargs, c);

					sprintf(str, "Ansi Code: [%-15s Args: (%d) [%02d %02d %02d %02d %02d]",
							temp, argc, a[0], a[1], a[2], a[3], a[4]);

					(*stringattr)(0, str, cfg.wattr);
					}
#endif
				break;
				}
			}

#ifdef GOODBYE
		if (debug)
			{
			sprintf(temp, "%s%c", ANSIargs, c);

			sprintf(str, "Ansi Code: [%-15s Args: (%d) [%02d %02d %02d %02d %02d]",
					temp, argc, a[0], a[1], a[2], a[3], a[4]);

			(*stringattr)(0, str, cfg.wattr);
			}
#endif
		}
	else if (OC.inANSI)
		{
		int i = strlen(ANSIargs);

		if (i < ANSILEN - 1)
			{
			ANSIargs[i] = c;
			ANSIargs[i + 1] = 0;
			}
		else
			{
			// cPrintf("Ansi code too long\n")
			ANSIargs[i]= 0;
			}
		}
	}


// --------------------------------------------------------------------------
// StatusLineC::Update: updates status line, lines, or screen

#ifdef WINCIT
#define twOnConsole ((TW->CommPort->GetType() == CT_LOCAL) || (TW->OC.whichIO == CONSOLE))
#else
#define twOnConsole	onConsole
#endif
void StatusLineC::Update(WC_TW)
	{
	static long tcLast;

	if (ScreenSaver.IsOn())
		{
#ifndef WINCIT
		static int tcRow = -1, tcCol = -1, tcClr;

		/*				   ’—∏	 ’Õ∏
		update the way cool ≥urbo≥	it clock that is now in
							œ	 ‘Õæ
				⁄ƒƒƒƒø					  ≥   ⁄ƒƒƒƒƒø
				≥			⁄ƒƒƒƒƒø  ≥	  ≥   ≥			≥	  ≥
				≥	   ⁄ƒƒƒø≥	  ≥  √ø  ⁄¥   ≥			   ƒƒƒ≈ƒƒ
				≥	ƒø ≥	√ƒƒƒƒƒŸ  ≥¿ø⁄Ÿ≥   ≥				  ≥
				≥	 ≥ ≥	≥		 ≥ ¿Ÿ ≥   ≥		≥	≥	  ≥
				¿ƒƒƒƒŸ ≥	¿ƒƒƒƒƒŸ  ≥	  ≥   ¿ƒƒƒƒƒŸ	≥	  ≥
		*/

		if (cfg.turboClock)
			{
			label ClockString;
			static int Len;
			long tcurrent = time(NULL);
			Bool j = tcurrent >= tcLast + cfg.turboClock;

			if (tcRow >= 0 && tcCol >= 0)
				{
				if (j && tcLast)
					{
					sprintf(ClockString, getmsg(152), ns);
					ClockString[Len] = 0;
					statDisp(tcRow, tcCol, cfg.attr & 0x0f, ClockString);
					}
				}
			else
				{
				j = TRUE;
				}

			if (tcurrent > tcLast)
				{
				if (*cfg.SaverMsg)
					{
					strcpy(ClockString, cfg.SaverMsg);
					}
				else
					{
					strftime(ClockString, 30, getmsg(467), 0l);
					}

				Len = strlen(ClockString);

				if (j)
					{
					tcRow = (int) ((long) conRows * (long) rand() / (long) RAND_MAX);
					tcCol = (int) ((long) (conCols - Len) * (long) rand() / (long) RAND_MAX);

					do
						{
						tcClr = (int) (128l * (long) rand() / (long) RAND_MAX);
						} while ((tcClr & 0x0ff) == (tcClr >> 8));

					time(&tcLast);
					}

				statDisp(tcRow, tcCol, tcClr, ClockString);
				}
			}
#endif
		}
	else
		{
		Bool TurnedCursorOff = FALSE;
		tcLast = 0; // so it displays immediately next time blanked

		if (!Visible)
			{
			return;
			}

		const int row = tw()logiRow, col = tw()logiCol;

		if (cfg.bios && !StatusLine.IsFullScreen() && !allWindows)
			{
			tw()cursoff();
			TurnedCursorOff = TRUE;
			}

		if (HelpTimeout)
			{
			if (HelpTimeout < (time(NULL) - (long) (60 * 2)))
				{
#ifdef WINCIT
				ToggleHelp(TW);
#else
				ToggleHelp();
#endif
				}
			else
				{
				char bigline[81];

				strcpy(bigline, getmsg(MSG_HELP, 0));

#ifdef WINCIT
				tw()WinPutString(conRows - 4, bigline, cfg.wattr, TRUE);
#else
				(*stringattr)(conRows - 4, bigline, cfg.wattr, TRUE);
#endif

				sprintf(bigline, getmsg(MSG_HELP, 1), getmsg(MSG_HELP, 2), getmsg(MSG_HELP, 3), getmsg(MSG_HELP, 4),
						StatusLine.IsFullScreen() ? getmsg(MSG_HELP, 5) : getmsg(MSG_HELP, 6),
						twOnConsole ? getmsg(MSG_HELP, 7) : getmsg(MSG_HELP, 8));

#ifdef WINCIT
				tw()WinPutString(conRows - 3, bigline, cfg.wattr, TRUE);
#else
				(*stringattr)(conRows - 3, bigline, cfg.wattr, TRUE);
#endif

				sprintf(bigline, getmsg(MSG_HELP, 1), getmsg(MSG_HELP, 9), cfg.noBells ?
						(cfg.noBells == 1 ? getmsg(MSG_HELP, 10) : getmsg(MSG_HELP, 11)) : getmsg(MSG_HELP, 12),
						getmsg(MSG_HELP, 13), cfg.noChat ? getmsg(MSG_HELP, 14) : getmsg(MSG_HELP, 15),
						getmsg(MSG_HELP, 16));

#ifdef WINCIT
				tw()WinPutString(conRows - 2, bigline, cfg.wattr, TRUE);
#else
				(*stringattr)(conRows - 2, bigline, cfg.wattr, TRUE);
#endif

				strcpy(bigline, getmsg(MSG_HELP, 17));

#ifdef WINCIT
				tw()WinPutString(conRows - 1, bigline, cfg.wattr, TRUE);
#else
				(*stringattr)(conRows - 1, bigline, cfg.wattr, TRUE);
#endif

				tw()position(row >= scrollpos ? scrollpos : row, col);
				}
			}

#ifndef WINCIT
		if (State == SL_FULLSCREEN)
			{
			showStatScreen();
			}
#endif

		if (State == SL_TWOLINES)
			{
			label hallname, roomname;

			HallData[tw()thisHall].GetName(hallname, sizeof(hallname));
			RoomTab[tw()thisRoom].GetName(roomname, sizeof(roomname));
			stripansi(hallname);
			stripansi(roomname);

			char Clock[15];
			strftime(Clock, 14, conCols > 90 ? getmsg(268) : getmsg(372), 0);

			char str[200];
			sprintf(str, getmsg(518), hallname, roomname, Clock);

			// oh my god this is ugly
			int Max = min((int) conCols, sizeof(str));
			for (int i = strlen(str); i < Max; i++)
				{
				strcat(str, spc);
				}

#ifdef WINCIT
			tw()WinPutString(scrollpos + 1, str, cfg.wattr, TRUE);
#else
			(*stringattr)(scrollpos + 1, str, cfg.wattr, TRUE);
#endif
			}

		// The first line...

		label name;
		if (tw()loggedIn)
			{
			tw()CurrentUser->GetName(name, sizeof(name));
			stripansi(name);

			const int Limit = (30 - strlen(name)) / 2;

			for (int i = 0; i < Limit; i++)
				{
				strcat(name, spc);
				}
			}
		else
			{
			strftime(name, 30, getmsg(466), 0l);
			}

		const char *carr = tw()CommPort->HaveConnection() ? getmsg(72) : getmsg(65);

		char flags[10];
		CopyStringToBuffer(flags, getmsg(64));

		if (tw()CurrentUser->IsAide())		flags[0] = getmsg(63)[0];
		if (tw()CurrentUser->IsProblem())	flags[1] = getmsg(63)[1];
		if (tw()CurrentUser->IsPermanent())	flags[2] = getmsg(63)[2];
		if (tw()CurrentUser->IsNetUser())	flags[3] = getmsg(63)[3];
		if (tw()CurrentUser->IsUnlisted())	flags[4] = getmsg(63)[4];
		if (tw()CurrentUser->IsSysop())		flags[5] = getmsg(63)[5];
		if (!tw()CurrentUser->IsMail())		flags[6] = getmsg(63)[6];

		char tmleft[5];
		if (!cfg.accounting || !tw()CurrentUser->IsAccounting() || !tw()loggedIn)
			{
			CopyStringToBuffer(tmleft, getmsg(61));
			}
		else
			{
			if (tw()CurrentUserAccount->IsSpecialTime())
				{
				CopyStringToBuffer(tmleft, getmsg(46));
				}
			else
				{
				long Minutes = tw()CurrentUser->GetCredits() / 60;

				if (Minutes > 9999)
					{
					CopyStringToBuffer(tmleft, getmsg(33));
					}
				else
					{
					sprintf(tmleft, getmsg(122), Minutes);
					}
				}
			}


		char displaybaud[10];
		switch (tw()CommPort->GetType())
			{
			case CT_LOCAL:
				{
				CopyStringToBuffer(displaybaud, getmsg(309));
				break;
				}

			case CT_TELNET:
				{
				CopyStringToBuffer(displaybaud, getmsg(334));
				break;
				}

			case CT_SERIAL:
				{
				long rspeed;
				if (tw()CommPort->HaveConnection() && tw()DoWhat != DIALOUT)
					{
					rspeed = connectbauds[tw()CommPort->GetModemSpeed()];
					}
				else
					{
					rspeed = bauds[tw()CommPort->GetSpeed()];
					}

				switch (rspeed)
					{
					case 12000:
						{
						CopyStringToBuffer(displaybaud, getmsg(290));
						break;
						}

					case 14400:
						{
						CopyStringToBuffer(displaybaud, getmsg(291));
						break;
						}

					case 16800:
						{
						CopyStringToBuffer(displaybaud, getmsg(292));
						break;
						}

					case 19200:
						{
						CopyStringToBuffer(displaybaud, "19.2");
						break;
						}

					case 21600:
						{
						CopyStringToBuffer(displaybaud, "21.6");
						break;
						}

					case 24000:
						{
						CopyStringToBuffer(displaybaud, "24.0");
						break;
						}

					case 26400:
						{
						CopyStringToBuffer(displaybaud, getmsg(293));
						break;
						}

					case 28800:
						{
						CopyStringToBuffer(displaybaud, getmsg(294));
						break;
						}

					case 38400l:
						{
						CopyStringToBuffer(displaybaud, getmsg(295));
						break;
						}

					case 57600l:
						{
						CopyStringToBuffer(displaybaud, getmsg(296));
						break;
						}

					case 115200l:
						{
						CopyStringToBuffer(displaybaud, getmsg(297));
						break;
						}

					case 230400l:
						{
						CopyStringToBuffer(displaybaud, getmsg(298));
						break;
						}

					default:
						{
						sprintf(displaybaud, getmsg(111), rspeed);
						break;
						}
					}

				break;
				}

			default:
				{
				CopyStringToBuffer(displaybaud, getmsg(335));
				break;
				}
			}

        if (strlen(displaybaud) > 4)
    		{
		    strcpy(displaybaud, "????");
    		}

		char string[200];
		sprintf(string, getmsg(36), name, twOnConsole ? getmsg(586) : getmsg(587), carr, displaybaud, tmleft,
#ifdef WINCIT
				getmsg(45)[1],
#else
				getmsg(45)[CommPort->IsDisabled() ? 0 : 1],
#endif

				(sysMail)					? '' : ' ',

				(cfg.noBells) ? (cfg.noBells == 1 ? '\r' : ' ') : '',

				(tw()OC.Psycho)				? 'ù' : ' ',
				(debug)						? 'Ë' : ' ',
				(ConsoleLock.IsLocked(TRUE))? '' : ' ',
				(cfg.ignore_uptime)			? '˜' : ' ',

				(cfg.noChat) ? (tw()chatReq ? getmsg(43) : getmsg(42)) : (tw()chatReq ? getmsg(41) : getmsg(40)),

				(tw()OC.Printing)			? getmsg(39) : getmsg(37),
				(sysReq)					? getmsg(35) : getmsg(37),

				flags);

		int Max = min((int) conCols, sizeof(string));
		for (int i = strlen(string); i < Max; i++)
			{
			strcat(string, spc);
			}

#ifdef WINCIT
		tw()WinPutString(conRows, string, cfg.wattr, TRUE);
#else
		(*stringattr)(conRows, string, cfg.wattr, TRUE);

		// flashy stuff
		if (tw()chatReq)
			{
			char *poo = logiScreen + ((conCols * conRows) << 1) + 121;

			for (int i = 0; i < 4; i++, poo += 2)
				{
				*poo = *poo | 0x80;
				}
			}
#endif

		tw()position(row, col);

		if (TurnedCursorOff)
			{
			tw()curson();
			}
		}
	}

void TERMWINDOWMEMBER position(int row, int column)
	{
	logiRow = row;
	logiCol = column;

#ifndef WINCIT
	dgLogiRow = row;
	dgLogiCol = column;

	if (!CitWindowsCsr)
		{
		physPosition((char) row, (char) column);
		}
#endif
	}


void ScreenSaverC::Check(void)
	{
#ifndef WINCIT
	// It's allowed to turn on and we are configured to turn on and we are not on now and...
	if (MayTurnOn && cfg.screensave && !On &&

			// It is not configured to stay off while user is on and have carrier or user is on console
			!(cfg.really_fucking_stupid && (modStat || (onConsole && loggedIn)))

			// And we are not in the scroll-back buffer and...
			&& (DoWhat != SCROLLBACK) &&

			// the time since last updated is greater than or equal to our timeout
			(((time(NULL) - Timer) / (time_t) 60) >= (time_t) cfg.screensave))
		{
		TurnOn();
		}
#endif
	}//1096//1044
