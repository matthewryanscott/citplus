// --------------------------------------------------------------------------
// Citadel: OutOvl.CPP
//
// Overlayed output functions.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "msg.h"
#include "outfilt.h"
#include "term.h"


// --------------------------------------------------------------------------
// Contents
//
// rmPrintf()			sends formatted output to modem & console
// prtList()			Print a list of rooms, etc.
// dospCR()				does CR for entry of initials & pw
// outSpeech() 			wow.


// --------------------------------------------------------------------------
// rmPrintf(): Sends formatted output to modem & console.

void cdecl TERMWINDOWMEMBER rmPrintf(const char *fmt, ... )
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	if (MRO.DotoMessage == REVERSE_READ)
		{
		MRO.DotoMessage = NO_SPECIAL;
		OC.SetOutFlag(OUTOK);

		if (cfg.twitrev)
			{
			char buf[80];

			MRO.Reverse = !MRO.Reverse;
			sprintf(buf, getmsg(357), MRO.Reverse ? '-' : '+');
			mFormat(buf);
			doCR();
			}
		}

	if (cfg.twitrev && MRO.Reverse)
		{
		stripansi(OC.prtf_buff);
		strrev(OC.prtf_buff);
		}

	mFormat(OC.prtf_buff);
	ResetOutputControl();
	}


// --------------------------------------------------------------------------
// prtList(): Print a list of rooms, etc.

void TERMWINDOWMEMBER prtList(const char *item)
	{
	if (item == LIST_START || item == LIST_END)
		{
		if (item == LIST_END)
			{
			if (!PLlisted)
				{
				rmPrintf(getmsg(631));
				}

			rmPrintf(getmsg(645));
			doCR();

			OC.ResetOutParagraph();
			}

		PLlisted = FALSE;
		PLfirst = TRUE;
		}
	else
		{
		PLlisted = TRUE;

		if (PLfirst)
			{
			PLfirst = FALSE;
			}
		else
			{
			rmPrintf(getmsg(644));
			}

		if ((int) strlen(item) + 2 + OC.CrtColumn > CurrentUser->GetWidth())
			{
			doCR();
			}

		rmPrintf(pcts, item);
		}
	}


// --------------------------------------------------------------------------
// dospCR(): Does CR for entry of initials & pw.

void TERMWINDOWMEMBER dospCR(void)
	{
#ifdef WINCIT
	return;
#endif
	EchoType oldecho;

	oldecho = OC.Echo;

	OC.Echo = BOTH;
	OC.setio();

	if (cfg.nopwecho == 1)
		{
		doCR();
		}
	else
		{
		if (onConsole)
			{
			if (CommPort->HaveConnection())
				{
				domcr();
				}
			}
		else
			{
			doccr();
			}
		}

	OC.Echo = oldecho;
	OC.setio();
	}


// --------------------------------------------------------------------------
// outSpeech(): Wow.

Bool TERMWINDOWMEMBER outSpeech(Bool out, const char *st)
	{
#ifndef WINCIT
	int i, j, k;
	char buf[255];
	uchar wordBuf[MAXWORDLEN + 8];

	if (cfg.speechOn && say_ascii)
		{
		buf[0] = 0;

		for (i = 0; st[i] && (OC.User.CanOutput() ||
				OC.User.GetOutFlag() == OUTPARAGRAPH);)
			{
#ifdef MULTI
			if (OC.CheckInput(FALSE))
#else
			if (OC.CheckInput(FALSE))
#endif
				{
				ResetOutputControl();
				buf[0] = 0;
				break;
				}

			i = getWord(wordBuf, (uchar *) st, i, MAXWORDLEN);

			if (*wordBuf == CTRL_A)
				{
				if (out)
					{
					termCap((char *) wordBuf + 1);
					}
				}
			else if (*wordBuf == CTRL_B)
				{
				if (out)
					{
					localTermCap((char *) wordBuf + 1);
					}
				}
			else
				{
				if (strlen((char *) wordBuf) + strlen(buf) < 255)
					{
					strcat(buf, (char *) wordBuf);
					}
				else
					{
					k = strlen(buf);

					for (j = 0; j < k; j++)
						{
						buf[j] = filt_out[buf[j]];
						}

					(*say_ascii)(buf, k);
					strcpy(buf, (char *) wordBuf);
					}

				if (out)
					{
					putWord(wordBuf);
					}
				}
			}

		k = strlen(buf);
		for (j = 0; j < k; j++)
			{
			buf[j] = filt_out[buf[j]];
			}

		ansi(14);
		OC.MCI_goto = FALSE;
		(*say_ascii)(buf, k);
		return (TRUE);
		}
	else
		{
#endif
		return (FALSE);
#ifndef WINCIT
		}
#endif
	}


void cdecl TERMWINDOWMEMBER CRCRmPrintfCR(const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	doCR(2);
	mFormat(OC.prtf_buff);
	doCR();
	ResetOutputControl();
	}

void cdecl TERMWINDOWMEMBER wPrintf(ModemConsoleE W, const char *fmt, ...)
	{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(OC.prtf_buff, fmt, ap);
	va_end(ap);

	if (W == MODEM)
		{
		mFormat(OC.prtf_buff);
		ResetOutputControl();
		}
	else if (W == CONSOLE)
		{
		cOutput(OC.prtf_buff);
		}
	}

void TERMWINDOWMEMBER wDoCR(ModemConsoleE W)
	{
	if (W == MODEM)
		{
		doCR();
		}
	else if (W == CONSOLE)
		{
		doccr();
		}
	}
