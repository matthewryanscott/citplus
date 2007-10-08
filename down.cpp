// --------------------------------------------------------------------------
// Citadel: Down.CPP
//
// Code to bring Citadel down.

#include "ctdl.h"
#pragma hdrstop

#include "libovl.h"
#include "music.h"
#include "config.h"
#include "group.h"
#include "hall.h"
#include "scrlback.h"
#include "timer.h"
#include "cfgfiles.h"
#include "cwindows.h"
#include "events.h"
#include "term.h"
#include "msg.h"
#include "termwndw.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// crashout()			Fatal system error
// exitcitadel()		Done with cit, time to leave

// --------------------------------------------------------------------------
// crashout(): Fatal system error.

void cdecl crashout(const char *message, ...)
	{
	char buf[256];
	va_list ap;

	va_start(ap, message);
	vsprintf(buf, message, ap);
	va_end(ap);

#ifndef WINCIT
	Hangup();
#endif

	MessageDat.CloseMessageFile();

#ifndef WINCIT
	if (OC.PrintFile)
		{
		fclose(OC.PrintFile);
		OC.PrintFile = NULL;
		}

	if (journalfl)
		{
		fclose(journalfl);
		journalfl = NULL;
		}
#endif

	if (TrapFile)
		{
		fclose(TrapFile);
		TrapFile = NULL;
		}

	if (RoomFile)
		{
		fclose(RoomFile);
		RoomFile = NULL;
		}

	FILE *fd = fopen(citfiles[C_CRASH_CIT], FO_W);
	fprintf(fd, pcts, buf);
	fclose(fd);

#ifndef WINCIT
	cfg.attr = 7;	// exit with white letters
	cfg.battr = 0;	// and black border

	CommPort->OutString(cfg.modunsetup);
	CommPort->OutString(br);

	CommPort->DropDtr();
	CommPort->Deinit();
#endif

	freeTables();

#ifndef WINCIT
	DestroyScrollBackBuffer();
	position(0, 0);

	for (int i = 0; i < strlen(getmsg(666)); i++)
		{
		cPrintf(pctc, getmsg(666)[i]);
		doccr();
		}

	cPrintf(pcts, buf);
	doccr();

	(*deinitups)();
	(*deinitkbd)();
	(*close_sound)();
	deinitMouseHandler();
	deinit_internal_sound();

	delete [] vidDriver;
	delete [] comDriver;
	delete [] kbdDriver;

	curson();
	DeinitializeTimer();
	critical(FALSE);
#else
	MessageBox(NULL, buf, NULL, MB_ICONSTOP | MB_OK);
#endif
	exit(199);
	}


// --------------------------------------------------------------------------
// exitcitadel(): Done with cit, time to leave.

void exitcitadel(void)
	{
#ifndef WINCIT
	static Bool ShutdownStarted;	// I just know someone will try to
									// #CALL ExitCitadel in a SHUTDOWN #EVENT

	if (!ShutdownStarted)
		{
		ExitToMsdos = FALSE;	// Script processor aborts if TRUE
		ShutdownStarted = TRUE;
		doEvent(EVT_SHUTDOWN);
		ExitToMsdos = TRUE; 	// Probably not needed
		}

	CITWINDOW *w = CitWindowsMsg(NULL, getmsg(34));
#endif

	GroupData.Save();
	HallData.Save();

	writeTables();

    if(!(read_tr_messages()))
        {
        errorDisp(getmsg(172));
        }
    else
        {
        trap(T_SYSOP, "", gettrmsg(36));
        dump_tr_messages();
        }

	// close all files
	MessageDat.CloseMessageFile();

#ifndef WINCIT
	if (OC.PrintFile)
		{
		fclose(OC.PrintFile);
		OC.PrintFile = NULL;
		}

	if (journalfl)
		{
		fclose(journalfl);
		journalfl = NULL;
		}
#endif

	if (TrapFile)
		{
		fclose(TrapFile);
		TrapFile = NULL;
		}

	if (RoomFile)
		{
		fclose(RoomFile);
		RoomFile = NULL;
		}

#ifndef WINCIT
	if (!slv_door)
		{
		Hangup();
		CommPort->OutString(cfg.modunsetup);
		CommPort->OutString(br);

		CommPort->DropDtr();
		}

	CommPort->Deinit();

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}

	deinitMouseHandler();

	cfg.attr = 7;	// exit with white letters
	cfg.battr = 0;	// and black border

	outPhys(TRUE);
	cls(SCROLL_NOSAVE);

	setdefaultTerm(TT_ANSI); // Not sure about this.
	char dtstr[80];
	strftime(dtstr, 79, cfg.vdatestamp, 0l);
	cPrintf(getmsg(566), dtstr);
	doccr();

	cPrintf(getmsg(567), diffstamp(uptimestamp));
	doccr();
#endif

	freeTables();

#ifndef WINCIT
	DestroyScrollBackBuffer();

	(*deinitups)();
	(*deinitkbd)();
	(*close_sound)();
	deinit_internal_sound();

	delete [] vidDriver;
	delete [] comDriver;
	delete [] kbdDriver;

	curson();

	DeinitializeTimer();
	critical(FALSE);
#endif

	exit(return_code);
	}
