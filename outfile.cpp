// --------------------------------------------------------------------------
// Citadel: OutFile.CPP
//
// Formatted file output functions.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"

// --------------------------------------------------------------------------
// Contents
//
// dFormat()	 Outputs a string to file w/ wordwrap
// ddoCR() 		 does a return to file
// dputWord()	 Writes one word to file, w/ wordwrap
// doChar()		 is the top-level user-output function (one byte)
// ddoTAB()		 prints a tab to file

static int THREAD medium_kludge;

// --------------------------------------------------------------------------
// ISBLANK is a special macro used by getword and putword

#define TAB '\t'
#define ISBLANK(cc) ((cc) == ' ' || (cc) == TAB)


// --------------------------------------------------------------------------
// ddoCR(): Does a return to file.

void TERMWINDOWMEMBER ddoCR(FILE *fd)
	{
	if (!OC.Formatting)
		{
		return;
		}

	OC.CrtColumn = 1;

	fputc('\r', fd);
	fputc('\n', fd);

	prevChar = ' ';
	}


// --------------------------------------------------------------------------
// ddoTAB(): Prints a tab to file.

static void ddoTAB(FILE *file)
	{
	fputc ('\t', file);
#ifndef WINCIT
	updcrtpos('\t');
#endif
	}



// --------------------------------------------------------------------------
// doChar(): Top-level user-output function (one byte)
//	sends to file
//	does conversion to upper-case etc as necessary
//	Globals modified:		prevChar

void TERMWINDOWMEMBER doChar(register char c, FILE *file)
	{
	prevChar = c;	// for end-of-paragraph code

	if (c == '\t')
		{
		ddoTAB(file);
		return;
		}

	if (CurrentUser->IsUpperOnly())
		{
		c = (char) toupper(c);
		}

	if (c == 10)	// newline
		{
		if (OC.Formatting)
			{
			c = ' ';    // doCR() handles real newlines
			}
		else
			{
			OC.Formatting = TRUE;
			ddoCR(file);
			OC.Formatting = FALSE;
			OC.CrtColumn = 1;
			return;
			}
		}

	fputc(c, file);

	updcrtpos(c);
	}


// --------------------------------------------------------------------------
// dputWord(): Writes one word to file, w/ wordwrap.

void TERMWINDOWMEMBER dputWord(const uchar *st, FILE *file)
	{
	register const uchar *s;
	register int newColumn;

	if (!OC.Formatting)
		{
		for (; *st; st++)
			{
			doChar(*st, file);
			}

		return;
		}

	if (prevChar == '\n' && ISBLANK(*st))
		{
		ddoCR(file);
		}

	for (newColumn = OC.CrtColumn, s = st; *s; s++)
		{
		if (*s == '\b')
			{
			newColumn--;
			}
		else if (*s == BELL)
			{
			// beeps do nothing to column
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

	if (!(newColumn > CurrentUser->GetWidth()))
		{
		if (isspace(*st) && (((newColumn + medium_kludge) >
				CurrentUser->GetWidth()) || !medium_kludge))
			{
			OC.CrtColumn += medium_kludge;
			prevChar = *st;
			return;
			}
		}

	if (newColumn > CurrentUser->GetWidth())
		{
		ddoCR(file);

		if (isspace(*st))
			{
			return;
			}
		}

	for (; *st; st++)
		{
		if (OC.CrtColumn >= CurrentUser->GetWidth())
			{
			ddoCR(file);
			}
		doChar(*st, file);
		}
	}


// --------------------------------------------------------------------------
// dFormat(): Outputs a string to file w/ wordwrap.

void TERMWINDOWMEMBER dFormat(const char *string, FILE *file)
	{
	uchar wordBuf[MAXWORDLEN + 8];
	uchar nextwordBuf[MAXWORDLEN + 8];
	int i;
	int j;
	int oldcrtColumn = OC.CrtColumn;

	for (i = 0; string[i] && (OC.User.CanOutput() || OC.User.GetOutFlag() == OUTPARAGRAPH);)
		{
		KBReady();
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX		letWindowsMultitask();

		i = getWord(wordBuf, (uchar *) string, i, MAXWORDLEN);

		j = i;
		
		do
			{
			j = getWord(nextwordBuf, (uchar *) string, j, MAXWORDLEN);
			} while (string[j] && *nextwordBuf == CTRL_A);

		if ((*wordBuf == '\n' || *wordBuf == '\r') && ISBLANK(*nextwordBuf))
			{
			medium_kludge = 0;
			}
		else
			{
			medium_kludge = strlen((char *)nextwordBuf);
			}

		if (*wordBuf == CTRL_A && *(wordBuf+1) == TERM_FMT_OFF[0])
			{
			OC.Formatting = FALSE;
			}
		else if (*wordBuf == CTRL_A && *(wordBuf+1) == TERM_FMT_ON[0])
			{
			OC.Formatting = TRUE;
			}

		if (*wordBuf == CTRL_A && OC.CrtColumn > (int) strlen(string) - 1)
			{
			OC.CrtColumn -= strlen(string);
			}

		dputWord(wordBuf, file);
		}

	ResetOutputControl();
	ddoCR(file);
	OC.CrtColumn = oldcrtColumn;
	}
