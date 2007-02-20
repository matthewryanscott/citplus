// --------------------------------------------------------------------------
// Citadel: WindOvl.CPP
//
// Machine dependent windowing routines.


#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "scrlback.h"
#include "extmsg.h"
#include "statline.h"

// --------------------------------------------------------------------------
// Contents
//
// cls()					clears the screen
// help()					toggles help menu
// setscreen()				Set SCREEN to correct address for VIDEO
// save_screen()			allocates a buffer and saves the screen
// setborder()				wow.

extern ConsoleLockC ConsoleLock;


// --------------------------------------------------------------------------
// setborder(): wow.

void setborder(int battr)
	{
#ifndef WINCIT
	union REGS REG;

	if (battr != 0xff)
		{
		REG.h.ah = 11;
		REG.x.bx = ScreenSaver.IsOn() ? 0 : battr;
		int86(0x10, &REG, &REG);					// border clr
		}
#endif
	}


// --------------------------------------------------------------------------
// cls(): Clears the screen.

void TERMWINDOWMEMBER cls(int whatToDo)
	{
#ifdef WINCIT
	scroll(conRows, conRows + 1, (uchar) cfg.attr);
	position(0, 0);
#else
	if (whatToDo == SCROLL_SAVE && cfg.scrollSize)
		{
		SaveToScrollBackBuffer(logiRow);
		}

	if (cfg.bios && !ScreenSaver.IsOn() && !StatusLine.IsFullScreen() &&
			!allWindows)
		{
		scroll_bios(conRows, 0, cfg.attr);
		}
	else
		{
		cls_fast(ScreenSaver.IsOn() ? 0 : cfg.attr);
		}

	setborder(cfg.battr);
	position(0, 0);

	if (allWindows && !oPhys)
		{
		RedrawAll();
		}
#endif
	}


// --------------------------------------------------------------------------
// help(): This toggles our help menu.

void StatusLineC::ToggleHelp(WC_TW)
	{
	if (!Visible)
		{
#ifdef WINCIT
		Toggle(TW);
#else
		Toggle();
#endif
		}
	else
		{
		int row = tw()logiRow, col = tw()logiCol;

		// small window?
		if (!HelpTimeout)
			{
			if (LockMessages(MSG_HELP))
				{
				// Make big window...

				time(&HelpTimeout);

				scrollpos = conRows - Height();

				if (row > scrollpos)
					{
					tw()scroll(row, row - scrollpos, cfg.attr);
					row = scrollpos;
					}
				}
			}
		else
			{
			// Make small window...

			HelpTimeout = 0;

			if (State == SL_TWOLINES)
				{
				tw()clearline(conRows - 5, cfg.attr);
				}

			tw()clearline(conRows - 4, cfg.attr);
			tw()clearline(conRows - 3, cfg.attr);
			tw()clearline(conRows - 2, cfg.attr);
			tw()clearline(conRows - 1, cfg.attr);

			scrollpos = conRows - Height();

			UnlockMessages(MSG_HELP);
			}

		tw()position(row, col);
		updateStatus = TRUE;

#ifdef WINCIT
		Update(TW);
#else
		Update();
#endif
		}
	}


// --------------------------------------------------------------------------
// sftf10(void): Blank stat lines.

void StatusLineC::Toggle(WC_TW)
	{
	int row = tw()logiRow, col = tw()logiCol;

	if (Visible)
		{
		Visible = FALSE;

		for (int i = scrollpos + 1; i <= conRows; i++)
			{
			tw()clearline(i, cfg.attr);
			}

		scrollpos = conRows;
		}
	else
		{
		Visible = TRUE;

		scrollpos = conRows - Height();

		if (row > scrollpos)
			{
			tw()scroll(row, row - scrollpos, cfg.attr);
			row = scrollpos;
			}
		}

	tw()position(row, col);
	updateStatus = TRUE;

#ifdef WINCIT
	Update(TW);
#else
	Update();
#endif
	}


// --------------------------------------------------------------------------
// altF10(): Extra stat line.

void StatusLineC::ToggleSecond(WC_TW)
	{
	// No second line when in full-screen mode
	if (State == SL_FULLSCREEN)
		{
		return;
		}

	// If nothing is visible... make it so.
	if (!Visible)
		{
#ifdef WINCIT
		Toggle(TW);
#else
		Toggle();
#endif
		}
	else
		{
		int row = tw()logiRow, col = tw()logiCol;

		if (State == SL_ONELINE)
			{
			// Make it two...

			State = SL_TWOLINES;

			scrollpos = conRows - Height();

			if (row > scrollpos)
				{
				tw()scroll(row, row - scrollpos, cfg.attr);
				row = scrollpos;
				}
			}
		else
			{
			// Make it one...

			State = SL_ONELINE;

			scrollpos = conRows - Height();

			tw()clearline(scrollpos, cfg.attr);
			}

		tw()position(row, col);
		updateStatus = TRUE;

#ifdef WINCIT
		Update(TW);
#else
		Update();
#endif
		}
	}


// --------------------------------------------------------------------------
// save_screen(): Allocates a buffer and saves the screen.

Bool TERMWINDOWMEMBER save_screen(void)
	{
#ifndef WINCIT
	if (saveBuffer == NULL)
		{
		compactMemory();

		saveBuffer = new char[(conRows + 1) * conCols * 2];

		if (saveBuffer == NULL)
			{
			mPrintfCR(getmsg(135));
			return (FALSE);
			}
		else
			{
			memcpy(saveBuffer, logiScreen,
					((scrollpos + 1) * conCols * 2));

			if (scrollpos != conRows)
				{
				memset(saveBuffer + ((scrollpos + 1) * conCols * 2), 0,
						(conRows - scrollpos) * conCols * 2);
				}

			s_conRows = conRows;
			return (TRUE);
			}
		}
	else
		{
		return (FALSE);
		}
#else
	return (FALSE);
#endif
	}


// --------------------------------------------------------------------------
// restore_screen(): Restores screen and frees buffer.

void TERMWINDOWMEMBER restore_screen(void)
	{
#ifndef WINCIT
	if (ScreenSaver.IsOn() || allWindows)
		{
		return;
		}

	// just to be sure, here
	if (logiScreen == saveBuffer)
		{
		logiScreen = physScreen;
		dgLogiScreen = logiScreen;
		}

	if (saveBuffer != NULL)
		{
		if (s_conRows <= conRows)
			{
			const int row = logiRow, col = logiCol;

			cls(SCROLL_NOSAVE);

			position(row, col);
			memcpy(physScreen, saveBuffer, (s_conRows+1) * conCols * 2);
			}
		else
			{
			if (cfg.scrollSize)
				{
				char *SaveIt = logiScreen;
				logiScreen = saveBuffer;

				SaveToScrollBackBuffer(s_conRows - conRows);

				logiScreen = SaveIt;
				}

			memcpy(physScreen,
					saveBuffer + (s_conRows - conRows) * conCols * 2,
					(conRows+1) * conCols * 2);
			}

		delete [] saveBuffer;
		saveBuffer = NULL;
		}
#endif
	}


// --------------------------------------------------------------------------
// setscreen(): Set video mode flag 0 mono 1 cga.

void setscreen(void)
	{
#ifndef WINCIT
	int mode;
	union REGS REG;
	static uchar heightmode = 0;
	static uchar scanlines = 0;
	char mono = 0;
	char far *CrtHite = (char far *) 0x00400085L;

	if (!cfg.restore_mode)
		{
		heightmode = 0;
		scanlines = 0;
		}

	if (gmode() == 7)
		{
		mono = TRUE;
		}

	if (!heightmode)
		{
		if (*CrtHite == 8)
			{
			heightmode = 0x012;
			}
		else if (*CrtHite == 14)
			{
			heightmode = 0x011;
			}
		else if (*CrtHite == 16)
			{
			heightmode = 0x014;
			}
		}

	if (scanlines)
		{
		REG.h.ah = 0x12;			// Video function:
		REG.h.bl = 0x30;			// Set scan lines
		REG.h.al = scanlines;		// Num scan lines
		int86(0x10, &REG, &REG);
		}

	if (heightmode && conMode != -1)	// make sure heightmode is set
		{
		// conMode 1000 --> EGA 43 line, or VGA 50 line

		REG.h.ah = 0x00;
		REG.h.al = (uchar) ((conMode >= 1000) ? 3 : conMode);
		int86(0x10, &REG, &REG);

		// Set to character set 18, (EGA 43 line, or VGA 50 line)
		if (conMode == 1000)
			{
			REG.h.ah = 0x11;
			REG.h.al = 18;
			REG.h.bl = 0x00;
			int86(0x10, &REG, &REG);
			}
		else
			{
			REG.h.ah = 0x11;
			REG.h.al = heightmode;
			REG.h.bl = 0x00;
			int86(0x10, &REG, &REG);
			}
		}

	if (!scanlines)
		{
		getScreenSize(&conCols, &conRows);

		if (!mono)
			{
			if (conRows == 24)
				{
				if (*CrtHite == 14) scanlines = 1;	// Old char set
				if (*CrtHite == 16) scanlines = 2;	// Vga char set
				}

			if (conRows == 42) scanlines = 1;
			if (conRows == 49) scanlines = 2;
			}
		else
			{
			if (conRows == 24) scanlines = 1;
			if (conRows == 27) scanlines = 2;		// herc only
			if (conRows == 42) scanlines = 1;
			}
		}

	mode = gmode();
	conMode = (mode == 3 && conMode >= 1000) ? conMode : mode;

	if (mode == 7)
		{
		physScreen = (char far *) 0xB0000000L;		// mono
		logiScreen = saveBuffer ? saveBuffer : physScreen;
		}
	else
		{
		physScreen = (char far *) 0xB8000000L;		// cga
		logiScreen = saveBuffer ? saveBuffer : physScreen;
		setborder(cfg.battr);
		}

	dgLogiScreen = logiScreen;

	getScreenSize(&conCols, &conRows);

	scrollpos = conRows - StatusLine.Height();

	if (cfg.bios)
		{
		charattr = bioschar;
		stringattr = biosstring;
		}
	else
		{
		charattr = directchar;
		stringattr = directstring;
		}

	OC.ansiattr = cfg.attr;
#endif
	}

void TERMWINDOWMEMBER getScreenSize(uint *Cols, uint *Rows)
	{
#ifndef WINCIT
	*Cols = (uint) *(uchar far *) (0x0000044A);
	*Rows = (uint) *(uchar far *) (0x00000484);

	// sanity checks, some cards don't set these locations.
	// (Heather's CGA for example. =] )
	if (*Cols < 40) *Cols = 80;
	if (*Rows < 15) *Rows = 24;

	// now an overide for stupid CGA's (like Krissy's) who
	// insist on writing to this location..
	if (conMode == 1001)
		{
		*Cols = 80;
		*Rows = 24;
		}
#else
	// This might be correct...
	*Cols = cfg.ConsoleCols;
	*Rows = cfg.ConsoleRows - 1;
#endif
	}


// --------------------------------------------------------------------------
// ClearToEndOfLine(): Delete to end of line.

void TERMWINDOWMEMBER ClearToEndOfLine(void)
	{
	const int row = logiRow, col = logiCol;

	for (int i = col; i < conCols; i++)
		{
#ifdef WINCIT
		WinPutChar(' ', OC.ansiattr, FALSE);
#else
		(*charattr)(' ', OC.ansiattr, FALSE);
#endif
		}

	position(row, col);
	}

Bool ScreenSaverC::TurnOn(void)
	{
#ifndef WINCIT
	if (!On && save_screen())
		{
		const int row = logiRow, col = logiCol;

		int wow = cfg.attr;
		int wow2 = cfg.battr;

		cfg.attr = 0;
		cfg.battr = 0;

		cls(SCROLL_NOSAVE);

		cfg.attr = wow;
		cfg.battr = wow2;

		position(row, col);

		On = TRUE;

		logiScreen = saveBuffer;
		dgLogiScreen = logiScreen;

		if (!cfg.really_really_fucking_stupid)
			{
			cursoff();
			}

		if (cfg.LockOnBlank && *cfg.f6pass)
			{
			ConsoleLock.Lock();
			}
		}

	return (On);
#else
	return (FALSE);
#endif
	}

void ScreenSaverC::TurnOff(void)
	{
#ifndef WINCIT
	if (On)
		{
		On = FALSE;

		logiScreen = physScreen;
		dgLogiScreen = logiScreen;

		restore_screen();
		curson();
		StatusLine.Update(WC_TWp);
		}
#endif
	}

void TERMWINDOWMEMBER outPhys(Bool phys)
	{
#ifndef WINCIT
	static char *save;

	if (phys)
		{
		save = logiScreen;
		logiScreen = physScreen;
		dgLogiScreen = logiScreen;
		oPhys = TRUE;
		}
	else
		{
		if (save)
			{
			logiScreen = save;
			dgLogiScreen = logiScreen;
			oPhys = FALSE;
			}
		}
#endif
	}

Bool StatusLineC::ToggleFullScreen(WC_TW)
	{
#ifndef WINCIT
	if (!ScreenSaver.IsOn())
		{
		if (State == SL_FULLSCREEN)
			{
			State = SL_ONELINE;

			logiScreen = physScreen;
			dgLogiScreen = logiScreen;

			restore_screen();

			curson();

			UnlockMessages(MSG_F4SCREEN);

			return (TRUE);
			}
		else
			{
			if (!Visible)
				{
				Toggle();
				}

			if (State == SL_TWOLINES)
				{
				ToggleSecond();
				}

			State = SL_FULLSCREEN;
			updateStatus = TRUE;

			if (save_screen())
				{
				logiScreen = saveBuffer;
				dgLogiScreen = logiScreen;

				const int oldrow = logiRow, oldcol = logiCol;

				if (!cfg.really_really_fucking_stupid)
					{
					cursoff();
					}

				outPhys(TRUE);
				cls(SCROLL_NOSAVE);
				outPhys(FALSE);

				position(oldrow, oldcol);

				LockMessages(MSG_F4SCREEN);

				return (TRUE);
				}
			else
				{
				State = SL_ONELINE;
				}
			}

		}
#endif

	return (FALSE);
	}
