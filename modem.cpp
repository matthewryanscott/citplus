// --------------------------------------------------------------------------
// Citadel: Modem.CPP
//
// High level modem code, should not need to be changed for porting(?)

#include "ctdl.h"
#pragma hdrstop

#include "log.h"

// --------------------------------------------------------------------------
// Contents
//
// domcr()				print cr on modem, nulls and lf's if needed
// OutString()			push a string directly to the modem
// SerialPortC::Flush()	Flush garbage characters from the modem.


// --------------------------------------------------------------------------
// domcr(): Print cr on modem, nulls and LFs if needed.

void TERMWINDOWMEMBER domcr(void)
	{
	CommPort->Output('\r');

	for (int i = CurrentUser->GetNulls(); i; i--)
		{
		CommPort->Output(0);
		}

	if (CurrentUser->IsLinefeeds())
		{
		CommPort->Output('\n');
		}
	}


// --------------------------------------------------------------------------
// OutString(): Push a string directly to the modem.

void CommunicationPortC::OutString(const char *String, int Pace)
	{
#ifndef WINCIT
	Bool mtmp;

	mtmp = OC.Modem;
	OC.Modem = TRUE;
#endif

#ifdef WINCIT
errorDisp("OutString: \"%s\"", String);
#endif

	while (*String)
		{
		Output(*String++);	// output string

		if (Pace)
			{
			pause(Pace);
			}
		}

#ifndef WINCIT
	OC.Modem = mtmp;
#endif
	}


// --------------------------------------------------------------------------
// SerialPortC::Flush(): Flush garbage characters from the modem.

void CommunicationPortC::FlushInput(void)
	{
	while (IsInputReady())
		{
		// Don't use Input so Received and BeenReceived are not affected
		getChar();
		}
	}


// --------------------------------------------------------------------------
// Hangup(): Breaks the modem connection.

void TERMWINDOWMEMBER Hangup(void)
	{
	if (!slv_door)
		{
		CommPort->BreakConnection();
		}
	else
		{
		modStat = FALSE;		// hangup simulation...
		}
	}
