// --------------------------------------------------------------------------
// Citadel: Console.CPP
//
// This code handles the console on the IBM PC and compatible computers. This
// code will need heavy modification for use with other computers.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "events.h"
#include "cwindows.h"
#include "timechk.h"
#include "statline.h"

// --------------------------------------------------------------------------
// Contents
//
// doccr			Do CR on console, used to not scroll the window
// KBReady 			returns TRUE if a console char is ready
// ciChar			Get character from console			   
// outCon			put a character out to the console	   
// kb_hit			returns TRUE if character waiting	   
// special_pressed true if ALT, Shift, or CTRL pressed 	   

extern ConsoleLockC ConsoleLock;		// conovl.cpp


// --------------------------------------------------------------------------
// doccr(): Do CR on console, used to not scroll the window.

void TERMWINDOWMEMBER doccr(void)
	{
	if (!OC.Console)
		{
		return;
		}

	if (logiRow >= scrollpos)
		{
		scroll(scrollpos, 1, cfg.wysiwyg ? OC.ansiattr : cfg.attr);
		position(scrollpos, 0);
		}
	else
		{
		position(logiRow + 1, 0);
		}
	}


// --------------------------------------------------------------------------
// KBReady(): Returns TRUE if a console char is ready.

Bool TERMWINDOWMEMBER KBReady(void)
	{
	if (KeyboardBufferMonitor && KeyboardBuffer.IsDirty())
		{
		(KeyboardBufferMonitor->func)(EVT_DRAWINT, 0, 0,
				KeyboardBufferMonitor);

		KeyboardBuffer.SetDirty(FALSE);
		}

	if (SerialPortMonitor && CommPort->IsDirty())
		{
		(SerialPortMonitor->func)(EVT_DRAWINT, 0, 0,
				SerialPortMonitor);

		CommPort->SetDirty(FALSE);
		}

	if (kb_hit())
		{
		if (KeyboardBuffer.Peek() == 12)
			{
			KeyboardBuffer.Retrieve();

			if (!ConsoleLock.IsLocked(TRUE))
				{
				sysopkey = TRUE;
				}
			}
		else
			{
			return (TRUE);
			}
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// ciChar(): Get character from console.

int TERMWINDOWMEMBER ciChar(void)
	{
	while (!kb_hit());

	CommPort->IncReceived();
	return (KeyboardBuffer.Retrieve());
	}


// --------------------------------------------------------------------------
// outCon(): Put a character out to the console.

void TERMWINDOWMEMBER outCon(char c)
	{
	if (!OC.Console || (c == BELL && cfg.noBells) || (c == 26))
		{
		return;
		}

	if (c == ESC || OC.inANSI)	// ESC || ANSI sequence
		{
		ansi(c);
		return;
		}

	// if we don't have carrier then count what goes to console
	if (!modStat)
		{
		CommPort->IncTransmitted();
		}

	if (c == '\n')                  // Newline
		{
		doccr();
		}
	else if (c == '\r')             // Return
		{
		position(logiRow, 0);
		}
	else if (c == BELL) 			// Bell
		{
		doEvent(EVT_BEEP);
#ifdef WINCIT
		MessageBeep(0);
#else
		putchar(c);
#endif
		}
	else if (c == '\b')             // Backspace
		{
		if (logiCol == 0 && prevChar != 10)
			{
			logiRow--;
			logiCol = conCols;
			}

		int newR = logiRow, newC = logiCol - 1;

		position(newR, newC);
#ifdef WINCIT
		WinPutChar(' ', OC.ansiattr, FALSE);
#else
		(*charattr)(' ', OC.ansiattr, FALSE);
#endif
		position(newR, newC);

		if (CitWindowsVid)
			{
			CitWindowsTTYUpdate(newR, newR);
			}
		}
	else							// Other Character
		{
#ifdef WINCIT
		if (WinPutChar(c, OC.ansiattr, FALSE))
#else
		if ((*charattr)(c, OC.ansiattr, FALSE))
#endif
			{
			if (CitWindowsVid)
				{
// I don't think we need the + 1 here; the CitWindowsTTYUpdate call below
// should do it for us
//				CitWindowsTTYUpdate(logiRow, logiRow + 1);
				CitWindowsTTYUpdate(logiRow, logiRow);
				}

			doccr();
			}
		else
			{
			// now do cursor positioning here
			position(logiRow, logiCol + 1);
			}

		if (CitWindowsVid)
			{
			CitWindowsTTYUpdate(logiRow, logiRow);
			}
		}
	}


// --------------------------------------------------------------------------
// kb_hit(): Returns TRUE if character waiting.

// Moving this outside of the kb_hit() function gives us a slight speed
// increase: it is initialized when Citadel loads instead of the first time
// that kb_hit is called. This avoids a little check inserted by the compiler
// to check if it is the first calling.
static TimeChangeCheckerC UpdateTime;

Bool TERMWINDOWMEMBER kb_hit(void)
	{
	// Update clock and cron monitor when needed
	if (
			(	// Nobody on
				!loggedIn ||

				// Or screen-saver is on
				ScreenSaver.IsOn() ||

				// Or status screen is up
				StatusLine.IsFullScreen() ||

				// Or CronMonitor is on
				CronMonitor
			)
		&&
			UpdateTime.Check()
		)
		{
		if (CronMonitor)
			{
			(CronMonitor->func)(EVT_DRAWINT, 0, 0, CronMonitor);
			}

		StatusLine.Update(WC_TWp);
		UpdateTime.Reset();
		}

	if (allWindows)
		{
		CitWindowsProcessMouse();
		}

	if (CitWindowsKbd)
		{
		CitWindowsProcessKbd();
		}

	if (!(cfg.FullConLock && ConsoleLock.IsLocked())
#ifndef WINCIT
			&& (*sp_press)()
#endif
			)
		{
		ScreenSaver.Update();
		}

	if (!CitWindowsKbd)
		{
#ifdef WINCIT
		while (WinStatCon() && !KeyboardBuffer.IsFull())
#else
		while ((*statcon)() && !KeyboardBuffer.IsFull())
#endif
			{
#ifdef WINCIT
			int c = WinGetCon();
#else
			int c = (*getcon)();
#endif

			if (c)
				{
				if (c < 256)
					{
					if (cfg.FullConLock && ConsoleLock.IsLocked())
						{
						if (!ScreenSaver.IsOn() && cfg.VerboseConsole)
							{
							CitWindowsNote(NULL, getmsg(108));
							}
						}
					else
						{
						if (!KeyboardBuffer.Add((char) c))
							{
							putchar(BELL);
							}
						}
					}
				else
					{
					fkey(c);
					}
				}
			}
		}

	if (KeyboardBuffer.IsEmpty())
		{
		// Screensaver timeout?
		ScreenSaver.Check();

		return (FALSE);
		}

	if (!(cfg.FullConLock && ConsoleLock.IsLocked()))
		{
		ScreenSaver.Update();
		}

	return (TRUE);
	}

// --------------------------------------------------------------------------
// special_pressed(): TRUE if ALT, Shift, or CTRL pressed.

#define SPECIAL_KEYS	0xF00F
Bool cdecl special_pressed(void)
	{
#ifndef WINCIT
	return (*((uint *) MK_FP(0x0000,0x0417)) & SPECIAL_KEYS);
#else
	return (FALSE);
#endif
	}

char KeyboardBufferC::Retrieve(void)
	{
	while (IsEmpty());

	char c;

	if (InsertedStrings)
		{
		c = *InsertedStrings->string;

		for (char *d = InsertedStrings->string; *d; d++)
			{
			*d = *(d + 1);
			}

		if (!*InsertedStrings->string)
			{
			deleteLLNode((void **) &InsertedStrings, 1);
			}
		}
	else
		{
		c = Buffer[Start];

		Start = (Start + 1) % KEYBUFSIZE;
		}

	Dirty = TRUE;
	return (c);
	}

Bool KeyboardBufferC::InsertString(const char *NewString)
	{
	strList *Inserted = (strList *) insertLLNode((void **) &InsertedStrings,
			1, sizeof(*InsertedStrings) + strlen(NewString));

	if (Inserted)
		{
		strcpy(Inserted->string, NewString);

		for (char *Chr = Inserted->string; *Chr; Chr++)
			{
			*Chr = filterChar(*Chr);
			}

		Dirty = TRUE;
		}

	return (!!Inserted);
	}
