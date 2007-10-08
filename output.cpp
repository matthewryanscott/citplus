// --------------------------------------------------------------------------
// Citadel: Output.CPP
//
// Output routines.

#include "ctdl.h"
#pragma hdrstop

#define OUTMAIN
#include "outfilt.h"
#include "msg.h"
#include "log.h"
#include "account.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// getWord()	Gets the next word from the buffer and returns it
// mFormat()	Outputs a string to modem and console with wordwrap
// putWord()	Writes one word to modem and console, with wordwrap
// doBS()		does a backspace to modem & console
// doCR()		does a return to both modem & console
// doTAB()		prints a tab to modem & console according to flag
// oChar()		is the top-level user-output function (one byte)
// updcrtpos()	updates OC.CrtColumn according to character
// mPrintf()	sends formatted output to modem & console


// --------------------------------------------------------------------------
// mFormat(): Outputs a string to modem and console with wordwrap.
//

void TERMWINDOWMEMBER mFormat(const char *string)
	{
	uchar wordBuf[MAXWORDLEN + 8];
	int i;

	if (CheckIdleUserTimeout(TRUE))
		{
		sleepkey = FALSE;
		time(&LastActiveTime);

		UserHasTimedOut();
		return;
		}
	else
		{
		sleepkey = FALSE;
		}

	if (!outSpeech(TRUE, string))
		{
		Bool saveGoto = FALSE;

		for (i = 0; string[i] && (OC.User.CanOutput() || OC.User.GetOutFlag() == OUTPARAGRAPH);)
			{
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX			letWindowsMultitask();

#ifdef MULTI
			if (OC.CheckInput(FALSE, this))
#else
			if (OC.CheckInput(FALSE))
#endif
				{
				ResetOutputControl();
				break;
				}

			i = getWord(wordBuf, (uchar *) string, i, MAXWORDLEN);

			if (*wordBuf == CTRL_A)
				{
				termCap((char *) wordBuf + 1);
				}
			else if (*wordBuf == CTRL_B)
				{
				localTermCap((char *) wordBuf + 1);
				}
			else
				{
				putWord(wordBuf);
				}

			if (OC.MCI_goto)
				{
				if (saveGoto != OC.MCI_goto)
					{
					i = 0;
					}
				}

			saveGoto = OC.MCI_goto;
			}
		}

	ansi(14);
	OC.MCI_goto = FALSE;
	}


// --------------------------------------------------------------------------
// doBS(): Does a backspace to modem & console.

void TERMWINDOWMEMBER doBS(void)
	{
	oChar('\b');
	oChar(' ');
	oChar('\b');
	}


// --------------------------------------------------------------------------
// doCR(): Does a return to both modem & console.

void TERMWINDOWMEMBER doCR(void)
	{
	if (!OC.Formatting)
		{
		return;
		}

	if (!(OC.User.CanOutput()))
		{
		return;
		}

	OC.CrtColumn = 1;

	if (journalfl)
		{
		fprintf(journalfl, bn);
		prevChar = ' ';
#ifdef MULTI
		OC.CheckInput(FALSE, this);
#else
		OC.CheckInput(FALSE);
#endif
		return;
		}

	domcr();
	doccr();

	if (OC.Console && OC.Printing)
		{
		fprintf(OC.PrintFile, bn);
		}

	prevChar = ' ';

	// pause on full screen
	if (CurrentUser->GetLinesPerScreen() && (OC.User.GetOutFlag() == OUTOK || OC.User.GetOutFlag() == NOSTOP))
		{
		if (++numLines >= CurrentUser->GetLinesPerScreen() &&
				(!(CurrentUser->IsPauseBetweenMessages() || MRO.PauseBetween) || !OC.justdidpause))
			{
			numLines = 0;
#ifdef MULTI
			OC.CheckInput(TRUE, this);
#else
			OC.CheckInput(TRUE);
#endif

			OC.justdidpause = TRUE;
			numLines = 0;

			// set it again cause it gets screwed up by the <more>
			prevChar = ' ';
			OC.CrtColumn = 1;
			}
		else
			{
			OC.justdidpause = FALSE;
			}
		}
	else
		{
		numLines = 0;
		}
	}

void TERMWINDOWMEMBER doBS(int Count)
	{
	while (Count--)
		{
		doBS();
		}
	}

void TERMWINDOWMEMBER doCR(int Count)
	{
	while (Count--)
		{
		doCR();
		}
	}


// --------------------------------------------------------------------------
// doTAB(): Prints a tab to modem & console according to flag.

void TERMWINDOWMEMBER doTAB(void)
	{
	int column, column2;

	column = OC.CrtColumn;
	column2 = OC.CrtColumn;

	if (journalfl)
		{
		fputc('\t', journalfl);
		}
	else
		{
		if (OC.Console && OC.Printing)
			{
			fputc('\t', OC.PrintFile);
			}

		do
			{
			outCon(' ');
			} while ((++column % 8) != 1);

		if (OC.Modem)
			{
			if (CurrentUser->IsTabs())
				{
				CommPort->Output('\t');
				}
			else
				{
				do
					{
					CommPort->Output(' ');
					} while ((++column2 % 8) != 1);
				}
			}
		}

	updcrtpos('\t');
	}


// --------------------------------------------------------------------------
// echocharacter(): Echos bbs input according to global flags.

void TERMWINDOWMEMBER echocharacter(const char c)
	{
	if (OC.Echo == NEITHER)
		{
		return;
		}

	if (c == '\b')
		{
		doBS();
		}
	else if (c == '\n')
		{
		doCR();
		}
	else
		{
		oChar(c);
		}
	}


// --------------------------------------------------------------------------
// oChar(): the top-level user-output function (one byte)
// sends to modem port and console both
// does conversion to upper-case etc as necessary

void TERMWINDOWMEMBER oChar(register char c)
	{
	int i;
	Bool wasANSI = OC.inANSI;

	prevChar = c;					// for end-of-paragraph code

	if (c == 1)
		{
		c = 0;
		}

	if (c == '\t')
		{
		doTAB();
		return;
		}

	// You don't want to know
	if (CurrentUser->IsPsycho() && cfg.Psycho)
		{
		if (UpDoWn)
			{
			c = (char) toupper(c);
			}
		else
			{
			c = (char) tolower(c);
			}

		UpDoWn = !UpDoWn;
		}

#ifdef WINCIT
	// Sure I break all the damn rules.

    // 66062(nh) ...and the DOS version, it would seem

	if (CurrentUser->IsOut300())
		Sleep(50) ;
#endif

	if (CurrentUser->IsUpperOnly())
		{
		c = (char) toupper(c);
		}

	if (c == '\n')
		{
		if (OC.Formatting)
			{
			c = ' ';    // doCR() handles real newlines
			}
		else
			{
			OC.Formatting = TRUE;
			doCR();
			OC.Formatting = FALSE;
			OC.CrtColumn = 1;
			return;
			}
		}

	if (!TermCap->IsIBMExtended())
		{
		c = filt_out[(uchar)c];
		}

	if (!c)
		{
		return;
		}

	if (journalfl)
		{
		fputc(c, journalfl);
		}
	else
		{
		// show on console
		if (OC.Console)
			{
			outCon(c);
	
			// show on printer
			if (OC.Printing)
				{
				fputc(c, OC.PrintFile);
				}
			}
		}

	if (!journalfl)
		{
		// send out the modem
		if (OC.Modem && CommPort->HaveConnection() && !(wasANSI || OC.inANSI))
			{
			if (CurrentUser->IsOut300())
				{
				for (i = 1; i < (int) (connectbauds[CommPort->GetModemSpeed()] / 300); i++)
					{
#ifndef WINCIT
					assert(putrs);
					(*putrs)(0);
#endif
					//CommPort->Output(0);
					//transmitted--;		// don't count fake chars
					}
				}

			CommPort->Output(c);
			}
		}

	updcrtpos(c);
	}


// --------------------------------------------------------------------------
// updcrtpos(): Updates OC.CrtColumn according to character.

void TERMWINDOWMEMBER updcrtpos(const char c)
	{
	if (c == '\b')
		{
		OC.CrtColumn--;
		}
	else if (c == '\t')
		{
		while ((++OC.CrtColumn % 8) != 1);
		}
	else if ((c == '\n') || (c == '\r'))
		{
		OC.CrtColumn = 1;
		}
	else
		{
		if (c != BELL)	// beep!!
			{
			OC.CrtColumn++;
			}
		}
	}


// --------------------------------------------------------------------------
// mPrintf(): Sends formatted output to modem & console.

void cdecl TERMWINDOWMEMBER mPrintf(const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	mFormat(OC.prtf_buff);
	ResetOutputControl();
	}

void cdecl TERMWINDOWMEMBER mPrintfCR(const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	mFormat(OC.prtf_buff);
	doCR();
	ResetOutputControl();
	}

void cdecl TERMWINDOWMEMBER CRmPrintf(const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	doCR();
	mFormat(OC.prtf_buff);
	ResetOutputControl();
	}

void cdecl TERMWINDOWMEMBER CRmPrintfCR(const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	doCR();
	mFormat(OC.prtf_buff);
	doCR();
	ResetOutputControl();
	}

// --------------------------------------------------------------------------
// getWord(): Gets the next word from the buffer and returns it.

int TERMWINDOWMEMBER getWord(uchar *dest, register const uchar *source, int offset, int lim)
	{
	register int i = offset;

	if (source[i] == '\r' || source[i] == '\n')
		{
		i++;
		}
	else if (isspace(source[i]))
		{
		// step over spaces
		for (; isspace(source[i]) &&
				!(source[i] == '\r' || source[i] == '\n') &&
				((i - offset) < lim); i++);
		}
	else if (source[i] == CTRL_A || source[i] == CTRL_B)
		{
		if (source[i + 1])
			{
			i += 2;

			if (toupper(source[i - 1]) == 'X')
				{
				while ((i - offset < lim) && source[i] > CTRL_B &&
						!isspace(source[i]) && toupper(source[i]) != 'X' &&
						source[i] != '\r' && source[i] != '\n')
					{
					i++;
					}

				if (toupper(source[i]) == 'X')
					{
					i++;
					}
				}
			}
		else
			{
			i++;
			}
		}
	else
		{
		// step over word
		for (; !(isspace(source[i])) && ((i - offset) < lim) && source[i] &&
				source[i] != CTRL_A && source[i] != CTRL_B; i++);
		}

	strncpy((char *) dest, (char *) (source + offset), i - offset);

	dest[i - offset] = '\0';

	return (i);
	}


// --------------------------------------------------------------------------
// putWord(): Writes one word to modem and console, with wordwrap.

void TERMWINDOWMEMBER putWord(const uchar *st)
	{
	if (!OC.MCI_goto)
		{
		const register char *s;
		register int newColumn;
		char buf[MAXWORDLEN];

		if (!OC.NoReplace && replaceList && *st > 32)
			{
			pairedStrings *thisReplace;
			for (thisReplace = replaceList; thisReplace;
					thisReplace = (pairedStrings *) getNextLL(thisReplace))
				{
				if (SameString(thisReplace->string1,
						strip_punct((char *) st)))
					{
					break;
					}
				}

			if (thisReplace)
				{
				OC.NoReplace = TRUE;

				int i;

				for (i = 0, s = (const char *) st; ispunct(*s); s++)
					{
					buf[i++] = *s;
					}

				buf[i] = 0;

				putWord((uchar *) buf);

				for (i = 0, s = (strchr((const char *) st, 0) - 1);
						s > (const char *) st && ispunct(*s); s--)
					{
					buf[i++] = *s;
					}

				buf[i] = 0;
				strrev(buf);

				mFormat((char *) (thisReplace->string2));

				putWord((uchar *) buf);

				OC.NoReplace = FALSE;
				return;
				}
			}

		CopyStringToBuffer(buf, (const char *) st);

		if (CurrentUser->IsPsycho())
			{
			if (cfg.Backout)
				{
				strrev(buf);
				}

			if (cfg.Mmmm)
				{
				for (int i = 0; buf[i]; i++)
					{
					if (isupper(buf[i]))
						{
						buf[i] = 'M';
						}
					else if (islower(buf[i]))
						{
						buf[i] = 'm';
						}
					}
				}
			}

		const char *p = buf;

		if (!OC.Formatting)
			{
			for (; *p; p++)
				{
				oChar(*p);
				}

			return;
			}


// --------------------------------------------------------------------------
// ISBLANK is a macro used by getword and putword.

#define TAB '\t'
#define ISBLANK(cc) ((cc) == ' ' || (cc) == TAB)

// Experimental nifty new word wrap

		if (prevChar == '\n' && ISBLANK(*p))
			{
			// end of paragraph
			if (OC.User.GetOutFlag() == OUTPARAGRAPH)
				{
				OC.SetOutFlag(OUTOK);
				}

			doCR();
			}

		Bool StartANSI = FALSE, InMusic = FALSE, InANSI = FALSE;
		for (newColumn = OC.CrtColumn, s = p; *s; s++)
			{
			if (StartANSI)
				{
				StartANSI = FALSE;

				if (*s == '[')
					{
					InANSI = TRUE;
					InMusic = FALSE;
					}
				}
			else if (InANSI)
				{
				if (*s == 'M')  // Music
					{
					InMusic = TRUE;
					}
				else
					{
					if ((!InMusic && isalpha(*s)) || (InMusic && *s == 14))
						{
						InANSI = FALSE;
						InMusic = FALSE;
						}
					}
				}
			else
				{
				if (*s == ESC)
					{
					StartANSI = TRUE;
					}
				else if (*s == '\b')
					{
					newColumn--;
					}
				else if (*s == BELL)
					{
					// beeps don't do any column stuff at all
					}
				else if (*s != TAB)
					{
					++newColumn;
					}
				else
					{
					while (newColumn++ % 8);
					}
				}
			}

		if (newColumn > CurrentUser->GetWidth())
			{
			// force a carriage return
			doCR();

			if (OC.HangingIndent < CurrentUser->GetWidth())
				{
				for (int i = OC.HangingIndent; i; i--)
					{
					oChar(' ');
					}
				}

			if (isspace(*p))
				{
				return;
				}
			}

		for (; *p; p++)
			{
			// worry about words longer than a line
			if (OC.CrtColumn >= CurrentUser->GetWidth())
				{
				doCR();

				if (OC.HangingIndent < CurrentUser->GetWidth())
					{
					for (int i = OC.HangingIndent; i; i--)
						{
						oChar(' ');
						}
					}
				}

			oChar(*p);
			}
		}
	}


// --------------------------------------------------------------------------
// mPrintf(): Sends formatted output to modem & console.

void cdecl TERMWINDOWMEMBER DebugOut(const char *fmt, ...)
	{
	if (debug)
		{
		char Wow[256];
		va_list ap;

		va_start(ap, fmt);
		vsprintf(Wow, fmt, ap);
		va_end(ap);

		cPrintf(getdbmsg(32), Wow);
		}
	}

void cdecl TERMWINDOWMEMBER DebugOut(int DbMsgIndex, ...)
	{
	if (debug)
		{
		char Wow[256];
		va_list ap;

		va_start(ap, DbMsgIndex);
		vsprintf(Wow, getdbmsg(DbMsgIndex), ap);
		va_end(ap);

		cPrintf(getdbmsg(32), Wow);
		}
	}

#ifdef WINCIT
void cdecl DebugOut(const char *fmt, ...)
	{
	if (debug)
		{
		char Wow[256];
		va_list ap;

		va_start(ap, fmt);
		vsprintf(Wow, fmt, ap);
		va_end(ap);

		errorDisp(Wow);
		}
	}

void cdecl DebugOut(int DbMsgIndex, ...)
	{
	if (debug)
		{
		char Wow[256];
		va_list ap;

		va_start(ap, DbMsgIndex);
		vsprintf(Wow, getdbmsg(DbMsgIndex), ap);
		va_end(ap);

		errorDisp(Wow);
		}
	}
#endif

// Remove ^A5, ^BM, etc.
void TERMWINDOWMEMBER ResetOutputControl(void)
	{
	OC.Formatting = TRUE;
	OC.UseMCI = cfg.mci;
	OC.HangingIndent = 0;
	OC.NoReplace = FALSE;
	}
