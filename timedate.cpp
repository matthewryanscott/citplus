// --------------------------------------------------------------------------
// Citadel: TimeDate.CPP
//
// Routines relating to the time and date

#include "ctdl.h"
#pragma hdrstop

#include "termwndw.h"
#include "log.h"

// --------------------------------------------------------------------------
// Contents
//
// dayofweek()				returns current day of week 0 - 6; sun = 0
// diffstamp()				display the diffrence from timestamp to present
// getdstamp()				get datestamp (text format)
// gettstamp()				get timestamp (text format)
// hour()					returns hour of day 0 - 23
// pause()					busy-waits N/100 seconds
// CheckIdleUserTimeout()	returns TRUE if time has been exceeded


// this is the number of seconds MSC 7.00 is off by
#define TIME_FUDGE 2209075200L

#ifndef WINCIT
Bool TimeoutChecking;
#endif

#ifdef VISUALC
long dostounix(struct date *d, struct time *t)
{
    struct tm when;

    when.tm_year      =    d->da_year - 1900;  
    when.tm_mday 	  =    d->da_day;   
    when.tm_mon 	  =    d->da_mon  - 1;   
    when.tm_min 	  =    t->ti_min;   
    when.tm_hour	  =    t->ti_hour;  
    when.tm_sec       =    t->ti_sec;   
 	when.tm_isdst     =    -1;

    return(mktime( &when ));
}

void unixtodos(long ltime, struct date *d, struct time *t)
{
    struct tm *newtime;

    time( &ltime );

    newtime = gmtime( &ltime );

    d->da_year = newtime->tm_year + 1900;     
    d->da_day  = newtime->tm_mday;     
    d->da_mon  = newtime->tm_mon + 1;  
    t->ti_min  = newtime->tm_min;
    t->ti_hour = newtime->tm_hour; 
    t->ti_sec  = newtime->tm_sec;    
}

void gettime(struct time *timep)
{
    struct tm *newtime;
    long ltime;

    time( &ltime );

    newtime = gmtime( &ltime );

    timep->ti_min  = newtime->tm_min;
    timep->ti_hour = newtime->tm_hour; 
    timep->ti_sec  = newtime->tm_sec;    
}
#endif

// --------------------------------------------------------------------------
// dayofweek(): Returns current day of week 0 - 6; sun = 0.

int dayofweek(void)
	{
	long stamp;
	struct tm *timestruct;

	time(&stamp);

	timestruct = localtime(&stamp);

	return (timestruct->tm_wday);
	}


// --------------------------------------------------------------------------
// diffstamp(): Display the diffrence from timestamp to present.

const char *TERMWINDOWMEMBER diffstamp(time_t oldtime)
	{
	static label THREAD diffstampstr;

	time_t diff;

	diff = time(NULL) - oldtime;

	// break it down
	long days;						// long in case it has been 90 years. :)
	int hours, minutes, seconds;

	seconds = (int) (diff % 60L);
	diff /= 60L;
	minutes = (int) (diff % 60L);
	diff /= 60L;
	hours = (int) (diff % 24L);
	days = diff / 24L;


	if (days)
		{
		sprintf(diffstampstr, getmsg(394), ltoac(days),
			(days == 1L) ? ns : justs, hours, minutes, seconds);
		}
	else
		{
		sprintf(diffstampstr, getmsg(420), hours, minutes, seconds);
		}

	return (diffstampstr);
	}


// --------------------------------------------------------------------------
// getdstamp(): Get datestamp (text format).

void getdstamp(char *buffer, time_t stamp)
	{
	if (stamp == 0)
		{
		strcpy(buffer, getmsg(330));
		return;
		}

	struct tm *TM = localtime(&stamp);

	char month[4];
	CopyStringToBuffer(month, monthTab[TM->tm_mon]);

	sprintf(buffer, getmsg(329), TM->tm_year % 100, month, TM->tm_mday);
	}


// --------------------------------------------------------------------------
// gettstamp(): get timestamp (text format)

void gettstamp(char *buffer, time_t stamp)
	{
	struct tm *TM = localtime(&stamp);
	 
	sprintf(buffer, getmsg(328), TM->tm_hour, TM->tm_min, TM->tm_sec);
	}


// --------------------------------------------------------------------------
// hour(): returns hour of day (0 - 23)

int hour(void)
	{
	long stamp;
	struct tm *timestruct;

	time(&stamp);

	timestruct = localtime(&stamp);

	return (timestruct->tm_hour);
	}


// --------------------------------------------------------------------------
// pause(): Waits N/100 seconds.

void pause(register int ptime)
	{
#ifdef WINCIT
	Sleep(ptime * 10);
#else
	union REGS in, out;
	register int i, j = 0;

	in.h.ah = 0x2C;
	intdos(&in, &out);
	i = out.h.dl;

	while (j < ptime)
		{
		in.h.ah = 0x2C;
		intdos(&in, &out);

		if (out.h.dl < (uchar) i)
			{
			j += (100 + out.h.dl) - i;
			}
		else
			{
			j += out.h.dl - i;
			}

		i = out.h.dl;

		CitIsIdle();
		}
#endif
	}


// --------------------------------------------------------------------------
// netpause(): waits N/100 seconds.

void TERMWINDOWMEMBER netpause(register int ptime)
	{
#ifdef WINCIT
	Sleep(ptime * 10);

	// After waiting, display any input we got form the comm port
	while (CommPort->IsInputReady())
		{
		const int ch = CommPort->Input();

		if (debug)
			{
			outCon((char) ch);
			}
		}
#else
	union REGS in, out;
	register int i, j=0;

	in.h.ah = 0x2C;
	intdos(&in, &out);
	i = out.h.dl;

	while (j < ptime)
		{
		in.h.ah=0x2C;
		intdos(&in, &out);

		if (out.h.dl < (uchar)i)
			{
			j += (100 + out.h.dl) - i;
			}
		else
			{
			j += out.h.dl - i;
			}

		i = out.h.dl;

		if (CommPort->IsInputReady())
			{
			const int ch = CommPort->Input();

			if (debug)
				{
				outCon(ch);
				}
			}

		CitIsIdle();
		}
#endif
	}


// --------------------------------------------------------------------------
// twirlypause(): waits N/100 seconds.

void TERMWINDOWMEMBER twirlypause(register int ptime)
	{
#ifdef WINCIT
	if (KeyboardBuffer.IsEmpty() && !CommPort->IsInputReady())
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
			WaitForMultipleObjects(WaitingFor, ToWaitFor, FALSE, ptime * 10);
			}
		}
#else
	union REGS in, out;
	register int i, j=0;

	in.h.ah = 0x2C;
	intdos(&in, &out);
	i = out.h.dl;

	while (j < ptime && !KBReady() && !(CommPort->IsInputReady() &&
			CommPort->HaveConnection() && modStat && (OC.whichIO == MODEM)))
		{
		in.h.ah = 0x2C;
		intdos(&in, &out);

		if (out.h.dl < (uchar) i)
			{
			j += (100 + out.h.dl) - i;
			}
		else
			{
			j += out.h.dl - i;
			}
		i = out.h.dl;

		CitIsIdle();
		}
#endif
	}


// --------------------------------------------------------------------------
// CheckIdleUserTimeout(): returns TRUE if time has been exceeded.

Bool TERMWINDOWMEMBER CheckIdleUserTimeout(Bool InOutput)
	{
#ifndef WINCIT
	if (!TimeoutChecking)
		{
		return (FALSE);
		}
#endif

	int Timeout = loggedIn ?
		(
		(onConsole && cfg.consoletimeout != -1) ?
			(
			cfg.consoletimeout
			)
		:
			(
            CurrentUser->IsProblem() ?
                (
        	    cfg.unlogtimeout
	       		)
            :
                (
        	    cfg.timeout
	       		)
            )
		)
	:
		(
		cfg.unlogtimeout
		);

	if (InOutput)
		{
		Timeout = max(cfg.OutputTimeout, Timeout);
		}

	return (sleepkey || (((time(NULL) - LastActiveTime) / 60) >= Timeout));
	}


#ifdef MSC
#undef time
time_t CitadelTime(time_t *tloc)
	{
	time_t x;

	x = time(NULL);

	if (tloc)
		{
		*tloc = x - TIME_FUDGE;
		}

	return (x - TIME_FUDGE);
	}
#endif
