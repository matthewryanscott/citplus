// --------------------------------------------------------------------------
// Citadel: Trap.CPP
//
// Trap file and aide message code

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "events.h"
#include "termwndw.h"
#ifndef WINCIT
#include "statline.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
// trap()				Record a line to the trap file
// SaveAideMess()		Save aide message being created
// amPrintf()			aide message printf
// amZap() 				Zap aide message being created


// --------------------------------------------------------------------------
// trap(): Record a line to the trap file.

void trap(TrapKeywords what, char *port, const char *fmt, ...)
	{
	char ToTrap[512];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(ToTrap, fmt, ap);
	va_end(ap);

	stripansi(ToTrap);

	if (what == T_LOGIN || what == T_NEWROOM || what == T_ERROR)
		{
		addStatusText(ToTrap, time(NULL));
		}

	switch (what)
		{
		case T_CARRIER: 	doEvent(EVT_CARRIER);	break;
		case T_NEWROOM: 	doEvent(EVT_NEWROOM);	break;
		case T_ANONYMOUS:	doEvent(EVT_ANONYMOUS); break;
		case T_CHAT:		doEvent(EVT_CHAT);		break;
		case T_PASSWORD:	doEvent(EVT_PASSWORD);	break;
		case T_AIDE:		doEvent(EVT_AIDE);		break;
		case T_SYSOP:		doEvent(EVT_SYSOP); 	break;
		case T_UPLOAD:		doEvent(EVT_UPLOAD);	break;
		case T_DOWNLOAD:	doEvent(EVT_DOWNLOAD);	break;
		case T_ERROR:		doEvent(EVT_ERROR); 	break;
		case T_NETWORK: 	doEvent(EVT_NETWORK);	break;
		case T_ACCOUNT: 	doEvent(EVT_ACCOUNT);	break;
		case T_CRON:		doEvent(EVT_CRON);		break;
		case T_HACK:		doEvent(EVT_HACK);		break;
		}

	// check to see if we are supposed to log this event
	if (!cfg.trapit[what])
		{
		return;
		}

	label dtstr;
	strftime(dtstr, LABELSIZE, getmsg(130), 0l);

#ifndef WINCIT
	if (!onConsole || !cfg.NoConsoleTrap)
#endif

		{

#ifdef WINCIT
		if (TrapMutex != INVALID_HANDLE_VALUE)
			{
			if (WAITFORmTIMED(TrapMutex))
				{
				DWORD TID = GetCurrentThreadId();
				long Seq = TermWindowCollection.FindConnSeqByThreadId(TID);

                if (!(cfg.trapit[T_PORT]) && (Seq >= 0))
					{
                    fprintf(TrapFile, getmsg(13), dtstr, TID, Seq, ToTrap, bn);
					}
                else if (!(cfg.trapit[T_PORT]) && !(Seq >= 0))
					{
                    fprintf(TrapFile, getmsg(12), dtstr, TID, ToTrap, bn);
					}
                else if (cfg.trapit[T_PORT] && (Seq >= 0))
                    {
                    fprintf(TrapFile, getmsg(11), dtstr, TID, Seq, port, ToTrap, bn);
                    }
                else if (cfg.trapit[T_PORT] && !(Seq >= 0))
                    {
                    fprintf(TrapFile, getmsg(12), dtstr, TID, ToTrap, bn);
                    }
                else
                    {
                    fprintf(TrapFile, getmsg(172));
                    }

				fflush(TrapFile);
				RELEASEm(TrapMutex);
				}
			}

#endif
		}
	}


// --------------------------------------------------------------------------
// SaveAideMess(): Save aide message being created.

void TERMWINDOWMEMBER SaveAideMess(const char *group, r_slot RoomNumber)
	{
	if (AideMsg)
		{
		if (strlen(AideMsg->GetText()) > 10)
			{
			AideMsg->SetRoomNumber(RoomNumber);

			if (group)
				{
				AideMsg->SetGroup(group);
				}

			systemMessage(AideMsg);
			}

		delete AideMsg;
		AideMsg = NULL;
		}
	}


// --------------------------------------------------------------------------
// amPrintf(): aide message printf.

void cdecl TERMWINDOWMEMBER amPrintf(const char *fmt, ... )
	{
	// no message in progress? make one!
	if (AideMsg == NULL)
		{
		AideMsg = new Message;
		}

	if (AideMsg)
		{
		va_list ap;
		char Buffer[128];

		va_start(ap, fmt);
		vsprintf(Buffer, fmt, ap);
		va_end(ap);

		AideMsg->AppendText(Buffer);
		}
	}


// --------------------------------------------------------------------------
// amZap(): Zap aide message being created.

void TERMWINDOWMEMBER amZap(void)
	{
	delete AideMsg;
	AideMsg = NULL;
	}


// --------------------------------------------------------------------------
// f4 trap stuff

void addStatusText(const char *string, time_t when)
	{
	statRecord *theStatus;

	if ((theStatus = addStatus()) != NULL)
		{
		strncpy(theStatus->theStatus, string, 60);
		theStatus->theStatus[60] = 0;
		theStatus->theTime = when;
		updateStatus = TRUE;
#ifndef WINCIT
		StatusLine.Update();
#endif
		}
	}

statRecord *addStatus(void)
	{
	int old_end = sl_end;

	sl_end = ++sl_end % cfg.statnum;
	if (sl_end == sl_start)
		{
		sl_start++;
		}
	return (&statList[old_end]);
	}
