// --------------------------------------------------------------------------
// Citadel: Port.CPP
//
// All the code specific to the communications hardware.

#include "ctdl.h"
#pragma hdrstop
#include "winser.h"
#include "statline.h"

// --------------------------------------------------------------------------
// Contents
//
// pHangup()		breaks modem connection
// Initport()		sets up modem port and modem


// --------------------------------------------------------------------------
// Initport(): Sets up the modem port and modem.

void TERMWINDOWMEMBER Initport(void)
	{
	Hangup();

	pause(50);

#ifndef WINCIT
	CommPort->Enable();
#endif

	CommPort->SetSpeed(cfg.initbaud);

	CommPort->OutString(cfg.modsetup);
	CommPort->OutString(br);

	pause(100);

	StatusLine.Update(WC_TWp);
	}


// --------------------------------------------------------------------------
// pHangup(): Breaks the modem connection.

void SerialPortC::BreakConnection(void)
	{
	FlushOutput();

	DropDtr();
	RaiseDtr();

	pause(50);

	if (cfg.dumbmodem == 6 || cfg.dumbmodem == 7)
		{
		OutString(cfg.hangup);
		OutString(br);

		if (cfg.hangupdelay)
			{
			pause(cfg.hangupdelay * 100);
			}
		else
			{
			pause(50);
			}
		}
	}


// --------------------------------------------------------------------------
// dumbmodemHangup(): Breaks the modem connection.

void SerialPortC::CycleDTR(void)
	{
	DropDtr();
	RaiseDtr();
	pause(50);
#ifndef WINCIT
	Enable();
#endif
	}


// --------------------------------------------------------------------------
// SerialPort::DropDtr(): Turns DTR off.
void SerialPortC::DropDtr(void)
	{
#ifdef WINCIT
	WaitForSingleObject(RMutex, INFINITE);

	if (!Closed)
		{
		EscapeCommFunction(hDev, CLRDTR);
		}

	ReleaseMutex(RMutex);
#else
	assert(dtrrs);

	Disabled = TRUE;

	(*dtrrs)(0);
#endif

	if (cfg.hangupdelay && !(cfg.dumbmodem == 6 || cfg.dumbmodem == 7))
		{
		pause (cfg.hangupdelay * 100);
		}
	else
		{
		pause (50);
		}
	}

#ifdef WINCIT
void SerialPortC::RaiseDtr(void) const
	{
	WaitForSingleObject(RMutex, INFINITE);

	if (!Closed)
		{
		EscapeCommFunction(hDev, SETDTR);
		}

	ReleaseMutex(RMutex);
	}
#endif


// --------------------------------------------------------------------------
// baud(): Sets up serial baud rate 0=300 1=1200 2=2400 3=4800 4=9600
//									5=19.2K 6=38.4K 7=57.6K 8=115.2K

#ifndef WINCIT
void SerialPortC::SetSpeed(PortSpeedE NewSpeed)
	{
	Speed = NewSpeed;

	int rs_Baud;

	switch (NewSpeed)
		{
		default: assert(FALSE);

		case PS_300:	{ rs_Baud = 2; break; }
		case PS_600:	{ rs_Baud = 3; break; }
		case PS_1200:	{ rs_Baud = 4; break; }
		case PS_2400:	{ rs_Baud = 5; break; }
		case PS_4800:	{ rs_Baud = 6; break; }
		case PS_9600:	{ rs_Baud = 7; break; }
		case PS_19200:	{ rs_Baud = 8; break; }
		case PS_38400:	{ rs_Baud = 9; break; }
		case PS_57600:	{ rs_Baud = 10; break; }
		case PS_115200: { rs_Baud = 11; break; }
		}

	// disable interrupts for baud change
	if (IsInitialized())
		{
		Deinit();
		}

	assert(initrs);
	(*initrs)((cfg.mdata - 1), rs_Baud, 0, 0, 3, cfg.checkCTS);

	SetInitialized();

	if (cfg.baudPause)
		{
		pause(cfg.baudPause);
		}
	}
#endif


// --------------------------------------------------------------------------
// SerialPort::Output(): Stuffs a char out the modem port.

Bool CommunicationPortC::IsInputReady(void)
		{
		while (CheckInputReady() == IR_IGNORE)
			{
			getChar();
			}

		return (CheckInputReady() != IR_NONE);
		}


void CommunicationPortC::Output(uchar Out)
	{
#ifndef WINCIT
	if ((!OC.Modem && !callout) || OC.inANSI)
		{
		return;
		}
#endif

	putChar(Out);

	Transmitted++;

	SetDirty();

	BeenSent[SentEnd] = Out;
	SentEnd = (SentEnd + 1) % KEYBUFSIZE;
	if (SentEnd == SentStart)
		{
		SentStart = (SentStart + 1) % KEYBUFSIZE;
		}
	else
		{
		NumBeenSent++;
		}
	}


#ifdef WINCIT

void _USERENTRY SerialStatusT(void *A)
	{
	SerialPortC *SP = (SerialPortC *) A;

	while (!SP->Closed)
		{
		Sleep(1000);
		WaitForSingleObject(SP->RMutex, INFINITE);

		if (!SP->Closed)
			{
			GetCommModemStatus(SP->hDev, &SP->ModemStatus);
			}

		ReleaseMutex(SP->RMutex);
		}
	}

void _USERENTRY SerialR(void *A)
	{
	SerialPortC *SP = (SerialPortC *) A;

	while (!SP->Closed)
		{
		Sleep(0);

		if (!SP->InRBuf)
			{
			WaitForSingleObject(SP->RMutex, INFINITE);

			DWORD RRet;

			if (!SP->Closed)
				{
				if (ReadFile(SP->hDev, SP->RBuf, sizeof(SP->RBuf), &RRet,
						NULL))
					{
					if (RRet > 0)
						{
                        if (SP->InputWaiting)
                            {
                            SetEvent(SP->InputWaiting);
                            }
						SP->InRBuf = RRet;
						SP->RBufIdx = 0;
						}
					}
				}

			ReleaseMutex(SP->RMutex);
			}
		}
	}

uchar SerialPortC::getChar(void)
	{
	if (InRBuf)
		{
        WaitForSingleObject(RMutex, INFINITE); // not sure

		uchar toRet = RBuf[RBufIdx++];
		InRBuf--;

        ReleaseMutex(RMutex); // not sure

        if (!InRBuf)
            {
            if (InputWaiting)
                {
                ResetEvent(InputWaiting);
                }
            }
		return (toRet);
		}

	return (0);
	}

void SerialPortC::SetSpeed(PortSpeedE NewSpeed)
	{
	Speed = NewSpeed;

	DCB dcb;

	WaitForSingleObject(RMutex, INFINITE);

	if (!Closed)
		{
		if (GetCommState(hDev, &dcb))
			{
			dcb.BaudRate = bauds[Speed];
			SetCommState(hDev, &dcb);
			}
		}

	ReleaseMutex(RMutex);

	if (cfg.baudPause)
		{
		pause(cfg.baudPause);
		}
	}
#endif
