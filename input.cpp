// --------------------------------------------------------------------------
// Citadel: Input.CPP
//
// User input routines

#include "ctdl.h"
#pragma hdrstop

#define INPUTMAIN
#include "infilt.h"
#include "log.h"
#include "account.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "termwndw.h"
#include "carrier.h"


// --------------------------------------------------------------------------
// Contents
//
// BBSCharReady()   Returns if char is available from modem or console
// setio()          Set output flags according to OutFlag, Echo, and whichIO
// iCharNE()        As below, but don't echo
// iChar()          Get a character from user. This also checks timeout, carrier detect, and a host of other things


// --------------------------------------------------------------------------
// BBSCharReady(): Returns if char is available from modem or console.

Bool TERMWINDOWMEMBER BBSCharReady(void)
    {
    return ((CommPort->HaveConnection() && !onConsole && CommPort->IsInputReady()) || KBReady());
    }

Bool TERMWINDOWMEMBER IsKeyFromUserReady(void)
    {
    // Either a key has been hit on console or there is modem input and someone is on-line
    return (KBReady() || (!onConsole && CommPort->IsInputReady() && CommPort->HaveConnection() && modStat));
    }


// --------------------------------------------------------------------------
// OutputControl::setio(): Set output flags according to OutFlag, Echo, and whichIO

void OutputControl::setio(void)
    {
    if (!(User.GetOutFlag() == OUTOK || User.GetOutFlag() == IMPERVIOUS || User.GetOutFlag() == NOSTOP))
        {
        Modem = FALSE;
        Console = FALSE;
        }
    else if (Echo == BOTH)
        {
        Modem = TRUE;
        Console = TRUE;
        }
    else if (Echo == CALLER)
        {
#ifdef WINCIT
        if ((term->CommPort->GetType() == CT_LOCAL) || whichIO == CONSOLE)
            {
            Modem = FALSE;
            Console = TRUE;
            }
        else if (whichIO == MODEM)
            {
            Modem = TRUE;
            Console = FALSE;
            }
#else
        if (whichIO == MODEM)
            {
            Modem = TRUE;
            Console = FALSE;
            }
        else if (whichIO == CONSOLE)
            {
            Modem = FALSE;
            Console = TRUE;
            }
#endif
        }
    else if (Echo == NEITHER)
        {
        Modem = FALSE;
        Console = FALSE;
        }

#ifdef WINCIT
    if (!term->CommPort->HaveConnection() || !term->modStat)
#else
    if (!CommPort->HaveConnection() || !modStat)
#endif
        {
        Modem = FALSE;
        }
    }


// --------------------------------------------------------------------------
// iCharNE(): iChar() without echo.

int TERMWINDOWMEMBER iCharNE(void)
    {
    const EchoType oldEcho = OC.Echo;

    OC.Echo = NEITHER;
    OC.setio();

    const int c = iChar();

    OC.Echo = oldEcho;
    OC.setio();

    return (c);
    }


// --------------------------------------------------------------------------
// iChar(): Get a character from user. This also checks timeout, carrier detect, and a host of other things.

int TERMWINDOWMEMBER iChar(void)
    {
	uchar c = 0;

	const EchoType oldEcho = OC.Echo;
	OC.Echo = BOTH;
	OC.setio();

	sysopkey = FALSE;   // F6 to go into sysop mode
	eventkey = FALSE;   // Alt+E, +R, or +F for an event
	forceevent = FALSE; // Alt+F or +R

	time_t ad_timer = time(&LastActiveTime);

	CommPort->FlushOutput();


#ifdef WINCIT
	TwirlyCursor.Start(this);
#else
	TwirlyCursor.Start();
#endif


	for (;;)
		{
#ifdef WINCIT
		TwirlyCursor.Update(this);
#else
		TwirlyCursor.Update();
#endif

#ifndef WINCIT
		if (statups && (*statups)())
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			dispBlb(B_PWRFAIL);
			ExitToMsdos = TRUE;
			break;
			}
#endif

		// Exit to MS-DOS key hit or carrier lost state or carrier gained at
		// the room prompt. (Carrier can only be gained in DOS versions)
		if (ExitToMsdos || !HaveConnectionToUser()
#ifndef WINCIT
			|| (DoWhat == MAINMENU && !loggedIn && !modStat && CheckForNewCarrier(CommPort))
#endif
			)
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			DebugOut(89);
			break;
			}

		// Keyboard press
		if (KBReady())
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			c = (char) ciChar();

			time(&LastActiveTime);
			break;
			}

		// Get character from modem
		if (CommPort->IsInputReady() && CommPort->HaveConnection() && modStat)
			{
			c = (char) CommPort->Input();

			// we do not want line feeds from remote, just CRs
			//          if (c == 10)
			//              {
			//              c = 0;
			//              }

			if (c && !onConsole)
				{
#ifdef WINCIT
				TwirlyCursor.Stop(this);
#else
				TwirlyCursor.Stop();
#endif
				break;
				}
			}

		// Request for sysop menu at main
		if ((sysopkey || chatkey) && DoWhat == MAINMENU)
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			DebugOut(90);
			break;
			}

		// Event key
		if (eventkey && (DoWhat == MAINMENU))
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			DebugOut(91);
			break;
			}

		// Sysop initiated chat
		if (chatkey && DoWhat == PROMPT)
			{
			EchoType oldEcho;

#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif

			oldEcho = OC.Echo;
			OC.Echo = BOTH;
			OC.setio();

			doCR();
			chat();
			CRmPrintf(pcts, gprompt);

#ifdef WINCIT
			TwirlyCursor.Start(this);
#else
			TwirlyCursor.Start();
#endif

			OC.Echo = oldEcho;
			OC.setio();

			time(&LastActiveTime);

			chatkey = FALSE;
			}

		// Keypress timeout
		if (CheckIdleUserTimeout(FALSE) && (loggedIn || modStat || DoWhat != MAINMENU))
			{
			sleepkey = FALSE;
			time(&LastActiveTime);

#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif

			UserHasTimedOut();
			}
		else
			{
			sleepkey = FALSE;
			}

#ifdef WINCIT
		// Auto idle 
		if (!IdlePrompt && OkayToShowEvents && ((time(NULL) - LastActiveTime) >= CurrentUser->GetAutoIdleSeconds()))
			{
			if (CurrentUser->GetAutoIdleSeconds() > 0)
				{
    			TwirlyCursor.Stop(this);
				mPrintfCR("[AutoIdle]");
    			TwirlyCursor.Start(this);
				CursorIsAtPrompt = FALSE;
				}
			if (CurrentUser->GetAutoIdleSeconds() > -1)
				{
    			IdlePrompt = TRUE;
			    }
			}
#endif

		// Cron timeout
		if (Cron.IsReady() && DoWhat == MAINMENU && !modStat && !loggedIn)
			{
#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif
			DebugOut(92);
			break;
			}

		// else check for ads
		if (cfg.ad_time && loggedIn && DoWhat == MAINMENU && CurrentUser->IsViewBorders() &&
			((time(NULL) - ad_timer) >= (time_t) cfg.ad_time))
			{
			time(&ad_timer);

#ifdef WINCIT
			TwirlyCursor.Stop(this);
#else
			TwirlyCursor.Stop();
#endif

			doAd(TRUE);
			GiveRoomPrompt(FALSE);

#ifdef WINCIT
			TwirlyCursor.Start(this);
#else
			TwirlyCursor.Start();
#endif
			}

#ifdef MULTI
		// Check for system events...
		if (OkayToShowEvents && PendingSystemEvents && SystemEventMutex != NULL)
			{
     		Bool StartedSE = FALSE; 
			DWORD w;
   			OkayToShowEvents = FALSE;
			do
				{
				w = WaitForSingleObject(SystemEventMutex, 100);

				if (w != WAIT_TIMEOUT && w != WAIT_FAILED)
					{
					SystemEventS *s = PendingSystemEvents;

					if (s)
						{
						PendingSystemEvents = s->next;
						ReleaseMutex(SystemEventMutex);

						if (!StartedSE)
							{
							TwirlyCursor.Stop(this);                 
							if (!IdlePrompt)
								{
								if (CurrentUser->IsErasePrompt())
									{
									int wow = 1000;	// prevent infinite loop, just in case
									while ((OC.CrtColumn >= 1) && wow--) doBS();
									oChar(' ');
									}
								else
									{
									doCR();
									}
								}
							else
								{
								if (CursorIsAtPrompt)
									{
									doCR();
									}    
								}
							StartedSE = TRUE;
							}
						OC.Echo = (s->HideFromConsole) ? CALLER : BOTH;
						OC.setio();
						mPrintfCR(getmsg(333), s->string);
						CursorIsAtPrompt = FALSE;
						}
					}
				} while (w != WAIT_TIMEOUT && w != WAIT_FAILED && PendingSystemEvents);
   			OkayToShowEvents = TRUE;

			if (StartedSE)
				{
            	OC.User.Reset();
				OC.Echo = BOTH;
				OC.setio();
              	OC.SetOutFlag(IMPERVIOUS);
				if (!IdlePrompt)
				    {
					OkayToShowEvents = FALSE;
				    GiveRoomPrompt(TRUE);
					OkayToShowEvents = TRUE;
					}
				TwirlyCursor.Start(this);
				}


			}
#endif

#ifdef WINCIT
		// If we have no pending input to process, wait half a second (1/10th, now)
		if (!CurrentUser->IsTwirly() && KeyboardBuffer.IsEmpty() && !CommPort->IsInputReady())
			{
			HANDLE ToWaitFor[2];
			int WaitingFor = 0;

			// keyboard is worth looking at if we are focused
			if (TermWindowCollection.InFocus(this))
				{
				ToWaitFor[WaitingFor++] = hConsoleInput;
				}

			// comm port is worth looking at if we could create the event
			if (CommPort->InputWaiting)
				{
				ToWaitFor[WaitingFor++] = CommPort->InputWaiting;
				}

			if (WaitingFor)
				{
				WaitForMultipleObjects(WaitingFor, ToWaitFor, FALSE, 100);
				}
			}
#else
		CitIsIdle();    //None of the above occurred
#endif
		}

	if (c < 128)
		{
		c = filt_in[c];
		}
	else if (c == 255)
		{
		c = 0;
		}

	OC.Echo = oldEcho;
	OC.setio();

	if (c != CTRL_A) // don't print ^As
		{
		echocharacter(c);
		}

	return (c);
}

