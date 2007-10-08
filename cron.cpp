// --------------------------------------------------------------------------
// Citadel: Cron.CPP
//
// Code to deal with cron events.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"

#include "cwindows.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "net.h"


// --------------------------------------------------------------------------
// Contents
//
// CronC::SetOnEvent		change current event
// CronC::GetEventNum		look up an event
// CronC::Do				called when system is ready to do an event
// CronEventC::CanDo		Can we do this event?
// ListAllEvents			List all events
// GetEventFromUser 		Ask user for an event
// cron_commands			Sysop Fn: Cron commands
// reset_event				Reset an even so that it has not been done
// did_net					Set all events for a node to done


// --------------------------------------------------------------------------
// CronC::SetOnEvent(): Change current event.

void CronC::SetOnEvent(CronEventC *Set)
	{
	WAITFORm(CronMutex);

	for (CronEventListS *cur = Events; cur; cur = (CronEventListS *) getNextLL(cur))
		{
		if (&(cur->Event) == Set)
			{
			OnEvent = cur;
			RELEASEm(CronMutex);
			return;
			}
		}

	RELEASEm(CronMutex);
	OnEvent = Events;
	}


// --------------------------------------------------------------------------
// CronC::GetEventNum(): Look up an event.

CronEventListS *CronC::GetEventNum(int Number)
	{
	WAITFORm(CronMutex);
	CronEventListS *RetVal = (CronEventListS *) getLLNum(Events, Number);
	RELEASEm(CronMutex);
	return (RetVal);
	}


// --------------------------------------------------------------------------
// CronC::Do(): Called when the system is ready to do an event.

Bool CronC::Do(CronCallerE)
	{
	DEBUGCODE(MessageDat.Verify());

	Bool done = FALSE;

	if (Events && !Pause)
		{
		CronEventListS *StartEvent = OnEvent;

		WAITFORm(CronMutex);
		do
			{
			if (OnEvent->Event.CanDo(forceevent))
				{
				ExecutingEvent = OnEvent;

				repeatevent = !OnEvent->Event.Do() && repeatevent;

				ExecutingEvent = NULL;

				done = TRUE;
				}

			if (!repeatevent && (OnEvent = (CronEventListS *) getNextLL(OnEvent)) == NULL)
				{
				OnEvent = Events;
				}

			} while (!done && OnEvent != StartEvent);
		RELEASEm(CronMutex);
		}

#ifndef WINCIT
	if (!done)
		{
		DebugOut(52);

		CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

		Initport();

		if (w)
			{
			destroyCitWindow(w, FALSE);
			}

		return (FALSE);
		}

	doCR();
#endif

	return (TRUE);
	}


// --------------------------------------------------------------------------
// CronEventC::CanDo(): Can we do this event?

Bool CronEventC::CanDo(Bool IgnoreTime) const
	{
	if (Zapped)
		{
		return (FALSE);
		}

	const time_t tn = time(NULL);
	const struct tm *tmnow = localtime(&tn);

	return (IgnoreTime ||

			(
			// right time, day, date, or month
			Hours[tmnow->tm_hour] && Days[tmnow->tm_wday] &&
			Dates[tmnow->tm_mday - 1] && Months[tmnow->tm_mon] &&

			// and beyond #REDO_TIME
			((tn - LastSuccess) / 60 >= RedoTime) &&

			// and beyond #RETRY_TIME
			((tn - LastTried) / 60 >= RetryTime)
			)
		);
	}

// --------------------------------------------------------------------------
// ListAllEvents(): List all events.
//
// Notes:
//	This function was borrowed from Acit.

void TERMWINDOWMEMBER ListAllEvents(void)
	{
	doCR();

	discardable *crdd;
	if ((crdd = readData(9)) == NULL)
		{
		mPrintf(getmsg(59));
		return;
		}

	OC.SetOutFlag(OUTOK);

	if (Cron.HasEvents())
		{
		termCap(TERM_BOLD);
		mPrintf(getsysmsg(169));
		termCap(TERM_NORMAL);
		doCR();

		const char **crontypes = (const char **) crdd->aux;
		CronEventListS *theEvent;

		for (int i = 1; (theEvent = Cron.GetEventNum(i)) != NULL; i++)
			{
			CronEventC Evt = theEvent->Event;

			char dtstr[20], etstr[20];

			mPrintf(getsysmsg(352), i,

					Cron.IsOnEvent(theEvent) ? getsysmsg(353) : spc,

					Evt.CanDo() ? getsysmsg(354) : (Evt.LastAttemptFailed() ? getsysmsg(355) : spc),

					crontypes[Evt.GetType()], Evt.GetString(),

					Evt.IsZapped() ? getsysmsg(170) : itoa(Evt.GetRedo(), dtstr, 10),

					Evt.IsZapped() ? getsysmsg(170) : itoa(Evt.GetRetry(), etstr, 10));

			if (Evt.GetLastSuccess())
				{
				strftime(dtstr, 19, getsysmsg(351), Evt.GetLastSuccess());
				mPrintf(pcts, dtstr);
				}
			else
				{
				mPrintf(getsysmsg(171));
				}

			if (Evt.GetLastTried())
				{
				strftime(dtstr, 19, getsysmsg(351), Evt.GetLastTried());
				mPrintfCR(pcts, dtstr);
				}
			else
				{
				mPrintfCR(getsysmsg(172));
				}
			}

		if (Cron.IsPaused())
			{
			CRmPrintfCR(getsysmsg(173));
			}

		if (!CurrentUser->IsExpert())
			{
			CRmPrintf(getsysmsg(174));
			CRmPrintf(getsysmsg(175));
			CRmPrintfCR(getsysmsg(176));
			}
		}
	else
		{
		mPrintfCR(getsysmsg(79));
		}

	discardData(crdd);
	}


CronEventC *TERMWINDOWMEMBER GetEventFromUser(void)
	{
	int i;

	Bool List;
	do
		{
		List = FALSE;

		i = (int) getNumber(getsysmsg(78), 0L, Cron.GetNumEvents(), 0, TRUE, &List);

		if (List)
			{
			ListAllEvents();
			}
		} while (List);

	if (i)
		{
		return (&(Cron.GetEventNum(i)->Event));
		}
	else
		{
		return (NULL);
		}
	}


// --------------------------------------------------------------------------
// did_net(): Set all events for a node to done.

void did_net(const char *callnode)
	{
	CronEventListS *theEvent;
	label alias, locID, address;

	const l_slot logslot = nodexists(callnode, NULL);

	if (logslot == CERROR)		// extremely unlikely
		{
		return;
		}

	LogTab[logslot].GetAlias(alias, sizeof(alias));
	LogTab[logslot].GetLocID(locID, sizeof(locID));
	makeaddress(alias, locID, address);

	WAITFORm(CronMutex);
	for (int i = 1; (theEvent = Cron.GetEventNum(i)) != NULL; i++)
		{
		if (theEvent->Event.GetType() == CR_NET && (
				SameString(theEvent->Event.GetString(), callnode) ||
				SameString(theEvent->Event.GetString(), address) ||
				SameString(theEvent->Event.GetString(), alias)))
			{
			theEvent->Event.SetDone();
			}
		}
	RELEASEm(CronMutex);
	}


// --------------------------------------------------------------------------
// cron_commands() Sysop Fn: Cron commands

void TERMWINDOWMEMBER cron_commands(void)
	{
	int ich;

	SetDoWhat(SYSCRON);

	switch (toupper(ich = iCharNE()))
		{
		case 'A':
			{
			mPrintfCR(getsysmsg(68));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventListS *theEvent;
				for (int i = 1; (theEvent = Cron.GetEventNum(i)) != NULL; i++)
					{
					theEvent->Event.SetDone();
					}
				RELEASEm(CronMutex);

				CRmPrintfCR(getsysmsg(69));
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'D':
			{
			mPrintfCR(getsysmsg(70));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventC *theEvent = GetEventFromUser();

				if (theEvent)
					{
					theEvent->SetDone();
					}
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'E':
			{
			mPrintfCR(getsysmsg(71));

			if (WAITFORmTIMED(CronMutex))
				{
				Cron.ReadCronCit(WC_TWp);
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'F':
			{
			mPrintfCR(getsysmsg(72));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventC *theEvent = GetEventFromUser();

				if (theEvent)
					{
					theEvent->Do();
					}
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'L':
			{
			mPrintfCR(getsysmsg(73));

			if (WAITFORmTIMED(CronMutex))
				{
				ListAllEvents();
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'N':
			{
			mPrintfCR(getsysmsg(74));

			if (WAITFORmTIMED(CronMutex))
				{
				Cron.SetOnEvent(GetEventFromUser());
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'P':
			{
			Cron.TogglePause();
			displayOnOff(getsysmsg(75), Cron.IsPaused());
			doCR();
			break;
			}

		case 'R':
			{
			mPrintfCR(getsysmsg(76));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventC *theEvent = GetEventFromUser();

				if (theEvent)
					{
					theEvent->Reset();
					}
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'S':
			{
			mPrintfCR(getsysmsg(268));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventC *theEvent = GetEventFromUser();

				if (theEvent)
					{
					Bool First = TRUE;

					struct date dt;
					struct time tm;
					datestruct ds;
					timestruct ts;

					do
						{
						if (!HaveConnectionToUser())
							{
							RELEASEm(CronMutex);
							return;
							}

						if (!First)
							{
							mPrintfCR(getmsg(156));
							}

						First = FALSE;

						label usrIn;
						if (!GetStringWithBlurb(getmsg(266), usrIn, LABELSIZE, ns, B_DATESET))
							{
							RELEASEm(CronMutex);
							return;
							}

						gdate(usrIn, &ds);
						} while (!ds.Date);

					First = TRUE;

					do
						{
						if (!HaveConnectionToUser())
							{
							RELEASEm(CronMutex);
							return;
							}

						if (!First)
							{
							mPrintfCR(getmsg(121));
							}

						First = FALSE;

						label usrIn;
						if (!GetStringWithBlurb(getmsg(230), usrIn, LABELSIZE, ns, B_TIMESET))
							{
							RELEASEm(CronMutex);
							return;
							}

						gtime(usrIn, &ts);
						} while (ts.Hour < 0);

					tm.ti_min = (uchar) ts.Minute;
					tm.ti_hour = (uchar) ts.Hour;
					tm.ti_sec = (uchar) ts.Second;
					dt.da_year = ds.Year + 1900;		// i hate dostounix
					dt.da_day = (char) ds.Date;
					dt.da_mon = (char) (ds.Month + 1);	// i hate dostounix

					time_t tried = dostounix(&dt, &tm);

					theEvent->SetLastSuccess(tried);
					theEvent->SetLastTried(tried);

					label dtstr;

					strftime(dtstr, LABELSIZE, getmsg(435), tried);
					CRmPrintf(getmsg(598), dtstr);

					strftime(dtstr, LABELSIZE, getmsg(434), tried);
					CRmPrintfCR(getmsg(607), dtstr);
					}

				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case 'Z':
			{
			mPrintfCR(getsysmsg(77));

			if (WAITFORmTIMED(CronMutex))
				{
				CronEventC *theEvent = GetEventFromUser();

				if (theEvent)
					{
					theEvent->ToggleZap();
					}
				RELEASEm(CronMutex);
				}
			else
				{
				CRmPrintfCR(getsysmsg(264));
				}

			break;
			}

		case '?':
			{
			oChar('?');
			showMenu(M_CRON);
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}
	}
