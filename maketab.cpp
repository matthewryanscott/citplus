// --------------------------------------------------------------------------
// Citadel: MakeTab.CPP
//
// Table making code.


#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "extmsg.h"
#include "room.h"


// --------------------------------------------------------------------------
// Contents
//
// RoomTabBld()			builds room.tab, indexes room.dat


// --------------------------------------------------------------------------
// logInit(): Indexes LOG.DAT.

void logInit(void)
	{
	LogEntry1 Log1;
	l_index Index;
	int count = 0;

#ifdef WINCIT
	HWND hPB = GetDlgItem(hConfigDlg, 103);

	if (hPB)
		{
		PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.MAXLOGTAB));
		}
#else
	cPrintf(getcfgmsg(173), getcfgmsg(155), ltoac(cfg.MAXLOGTAB), bn);
#endif

	cfg.callno = 0l;
	cfg.maxpoop = 0;
	cfg.poopuser[0] = 0;

	LogTab.Clear();

	Log1.OpenFile();

	for (Index = 0; Index < cfg.MAXLOGTAB; Index++)
		{
		letWindowsMultitask();

#ifdef WINCIT
		if (hPB)
			{
			PostMessage(hPB, PBM_SETPOS, Index, 0);
			}
#else
		KBReady();
		cPrintf(getcfgmsg(174), ltoac(Index + 1), br);
#endif

		if (!Log1.Load(Index))
			{
			crashout(getcfgmsg(281), getcfgmsg(155));
			}

		Log1.Verify();

		// perhaps not in release. perhaps in release. i don't know.
		if (Log1.IsInuse())
			{
			cfg.callno = max((long) cfg.callno, (long) Log1.GetCallNumber());

			if (Log1.GetPoopcount() > cfg.maxpoop)
				{
				cfg.maxpoop = Log1.GetPoopcount();
				Log1.GetName(cfg.poopuser, sizeof(cfg.poopuser));
				}

			count++;
			}
		else
			{
			Log1.SetCallNumber(0);
			Log1.SetPoopcount(0);
			}

		Log1.Save(Index);
		}

	Log1.CloseFile();


#ifdef WINCIT
	if (hPB)
		{
		PostMessage(hPB, PBM_SETPOS, cfg.MAXLOGTAB, 0);
		}
#else
	cPrintf(getcfgmsg(175), bn, ltoac(cfg.callno), br);
	cPrintf(getcfgmsg(176), ltoac(count), br, br);

	LogTab.Sort();
#endif
	}


// --------------------------------------------------------------------------
// verifyRoom(): Verifies room entry will not crash the system.

void RoomC::Verify(void)
	{
	// make sure no strings overflow
	Name[LABELSIZE]				= 0;
	NetID[LABELSIZE] 			= 0;
	ModHost[LABELSIZE]			= 0;
	ExclNetPartner[LABELSIZE]	= 0;
	Archive[12]					= 0;
	Dictionary[12]				= 0;
	Directory[63]				= 0;
	DescFile[12]				= 0;
	Applic[12]					= 0;
	Description[79] 			= 0;

	// fill any unused areas with emptiness
	UNUSED1 = 0;
	UNUSED2 = 0;
	UNUSED3 = 0;
	UNUSED4 = 0;
	UNUSED5 = 0;

	// don't overindex the group buffer
	if (GrpNo >= cfg.maxgroups)
		{
		GrpNo = 1;
		}

	if (PGrpNo >= cfg.maxgroups)
		{
		PGrpNo = 1;
		}

	// Just for looks.
	strupr(Directory);
	}


// --------------------------------------------------------------------------
// RoomTabBld(): Builds RAM index to ROOM.DAT and displays stats.

void RoomTabBld(void)
	{
	r_slot slot = 0, roomCount = 0;

#ifdef WINCIT
	HWND hPB = GetDlgItem(hConfigDlg, 102);

	if (hPB)
		{
		PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.maxrooms));
		}
#else
	cPrintf(getcfgmsg(173), cfg.Lroom_nym, ltoac(cfg.maxrooms), bn);
	cPrintf(getcfgmsg(177), cfg.Uroom_nym, ltoac(slot), br);
#endif

	RoomC RoomBuffer;

	for (; slot < cfg.maxrooms; slot++)
		{
		letWindowsMultitask();

		RoomBuffer.Load(slot);
		RoomBuffer.Verify();

		if (!((slot + 1) % 10))
			{
#ifdef WINCIT
			if (hPB)
				{
				PostMessage(hPB, PBM_SETPOS, slot, 0);
				}
#else
			KBReady();
			cPrintf(getcfgmsg(177), cfg.Uroom_nym, ltoac(slot + 1), br);
#endif
			}

		if (RoomBuffer.IsInuse())
			{
			++roomCount;
			}

		RoomBuffer.Save(slot);	// this indexes
		}

#ifdef WINCIT
	if (hPB)
		{
		PostMessage(hPB, PBM_SETPOS, cfg.maxrooms, 0);
		}
#else
	cPrintf(getcfgmsg(177), cfg.Uroom_nym, ltoac(slot), bn);

	label roomCountStr;
	CopyStringToBuffer(roomCountStr, ltoac(roomCount));

	cPrintf(getcfgmsg(178), roomCountStr, ltoac(cfg.maxrooms),
			cfg.Lrooms_nym, bn, bn);
#endif
	}
