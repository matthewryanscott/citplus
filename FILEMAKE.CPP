// --------------------------------------------------------------------------
// Citadel: FileMake.CPP
//
// File making routines.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "libovl.h"
#include "msg.h"
#include "log.h"
#include "hall.h"
#include "group.h"
#include "datafile.h"
#include "extmsg.h"


#ifdef WINCIT
static const char *ltoac(int wow)
	{
	static label Buffer;
	sprintf(Buffer, "%d", wow);
	return (Buffer);
	}

#define doccr()
#endif


// --------------------------------------------------------------------------
// Contents
//
// zapGrpFile() 	erases & reinitializes GRP.DAT
// zapHallFile()	erases & reinitializes HALL?.DAT
// zapRoomPosFile() erases & re-initializes ROOMPOS.DAT
// zapLogFile() 	erases & re-initializes LOG?.DAT
// zapRoomFile()	erases and re-initailizes ROOM.DAT

static void WriteDataFileSize(const char *FileName, long Size)
	{
	char Buffer[128];
	FILE *fd;

	sprintf(Buffer, sbs, cfg.homepath, FileName);

	if ((fd = fopen(Buffer, FO_WB)) == NULL)
		{
		crashout(getcfgmsg(171), FileName);
		}
	else
		{
		fwrite(&Size, sizeof(Size), 1, fd);
		fclose(fd);
		}
	}


// --------------------------------------------------------------------------
// buildhalls(): Builds hall table (all rooms in Maintenance).

void buildhalls(void)
	{
	msgDisp(getcfgmsg(162), cfg.Lhall_nym, ltoac(cfg.maxrooms - 1));
	doccr();
	doccr();

	for (r_slot i = 4; i < cfg.maxrooms; ++i)
		{
		if (RoomTab[i].IsInuse())
			{
			// Put all rooms into Maintenance, but not windowed
			HallData[MAINTENANCE].SetRoomInHall(i, TRUE);
			HallData[MAINTENANCE].SetWindowedIntoHall(i, FALSE);
			}
		}

	if (!HallData.Save())
		{
		crashout(getmsg(646));
		}

#ifdef WINCIT
	msgDisp(NULL);
//	msgCaption(ns);
#endif
	}


// --------------------------------------------------------------------------
// buildroom(): builds a new room according to a message.

void buildroom(Message *mb)
	{
	r_slot roomslot;

	if (*mb->GetCopyOfMessage())
		{
		return;
		}

	roomslot = mb->GetRoomNumber();

	if (mb->GetRoomNumber() < cfg.maxrooms)
		{
		RoomC RoomBuffer;
		RoomBuffer.Load(roomslot);

		// we use strcmp() here instead of IsSameName because we want to
		// record case changes.
		label Buffer;
		if (!RoomBuffer.IsInuse() || (strcmp(RoomBuffer.GetName(Buffer,
				sizeof(Buffer)), mb->GetCreationRoom()) != SAMESTRING))
			{
			if (mb->GetRoomNumber() > 3)
				{
				RoomBuffer.Clear();

				RoomBuffer.SetInuse(TRUE);
				RoomBuffer.SetPublic(TRUE);

				// recreate network rooms
				if (*mb->GetSourceID())
					{
					RoomBuffer.SetShared(TRUE);
					// strcpy(rBuf.netID, deansi(mb->GetCreationRoom()));
					}
				}

			// Don't bother trying to find new name of Dump>
			if (roomslot != DUMP)
				{
				RoomBuffer.SetName(mb->GetCreationRoom());
				}

			RoomBuffer.Save(roomslot);
			}
		}
	}


// --------------------------------------------------------------------------
// zapGrpFile(): Erases & reinitializes GRP.DAT.

void zapGrpFile(const char *FileName, const Bool Display)
	{
#ifdef WINCIT
	HWND hPB = NULL;
#endif

	if (Display)
		{
#ifdef WINCIT
		hPB = GetDlgItem(hCreateDlg, 104);

		if (hPB)
			{
			PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.maxgroups));
			}
#else
		cPrintf(getcfgmsg(163), cfg.Lgroup_nym, bn);
		doccr();
#endif
		}

	GroupData.Clear();

	GroupData[EVERYBODY].SetName(getcfgmsg(165));
	GroupData[EVERYBODY].SetDescription(getcfgmsg(98));
	GroupData[EVERYBODY].SetInuse(TRUE);

	GroupData[SPECIALSECURITY].SetName(getcfgmsg(166));
	GroupData[SPECIALSECURITY].SetDescription(getcfgmsg(99));
	GroupData[SPECIALSECURITY].SetInuse(TRUE);

	if (FileName)
		{
		if (!GroupData.Save(FileName))
			{
			crashout(getmsg(646));
			}
		}
	else
		{
		if (!GroupData.Save())
			{
			crashout(getmsg(646));
			}
		}

#ifdef WINCIT
	if (Display)
		{
		if (hPB)
			{
			PostMessage(hPB, PBM_SETPOS, cfg.maxgroups, 0);
			}
		}
#endif

#ifdef WINCIT
	letWindowsMultitask();
#endif
	}


// --------------------------------------------------------------------------
// zapHallFile(): Erases & reinitializes HALL?.DAT.

void zapHallFile(void)
	{
#ifdef WINCIT
	HWND hPB = GetDlgItem(hCreateDlg, 105);

	if (hPB)
		{
		PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.maxgroups));
		}
#else
	cPrintf(getcfgmsg(163), cfg.Lhall_nym, bn);
	doccr();
#endif

	// Main.
	HallData[MAINHALL].SetName(getcfgmsg(168));
	HallData[MAINHALL].SetOwned(FALSE);
	HallData[MAINHALL].SetInuse(TRUE);

	HallData[MAINHALL].SetRoomInHall(LOBBY, TRUE);
	HallData[MAINHALL].SetRoomInHall(MAILROOM, TRUE);
	HallData[MAINHALL].SetRoomInHall(AIDEROOM, TRUE);

	// Maintenance.
	HallData[MAINTENANCE].SetName(getcfgmsg(169));
	HallData[MAINTENANCE].SetOwned(FALSE);
	HallData[MAINTENANCE].SetInuse(TRUE);

	HallData[MAINTENANCE].SetRoomInHall(LOBBY, TRUE);
	HallData[MAINTENANCE].SetRoomInHall(MAILROOM, TRUE);
	HallData[MAINTENANCE].SetRoomInHall(AIDEROOM, TRUE);

	// and a window between them
	HallData[MAINHALL].SetWindowedIntoHall(AIDEROOM, TRUE);
	HallData[MAINTENANCE].SetWindowedIntoHall(AIDEROOM, TRUE);

	HallData.SetDefault(MAINHALL);

	if (!HallData.Save())
		{
		crashout(getmsg(646));
		}

#ifdef WINCIT
	if (hPB)
		{
		PostMessage(hPB, PBM_SETPOS, cfg.maxgroups, 0);
		}
#endif

#ifdef WINCIT
	letWindowsMultitask();
#endif
	}


// --------------------------------------------------------------------------
// zapRoomPosFile(): Erases & re-initializes ROOMPOS.DAT.

void zapRoomPosFile(const Bool Display)
	{
#ifdef WINCIT
	HWND hPB = NULL;
#endif

	if (Display)
		{
#ifdef WINCIT
		hPB = GetDlgItem(hCreateDlg, 106);

		if (hPB)
			{
			PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.maxrooms));
			}
#else
		char String[128];
		sprintf(String, getcfgmsg(170), cfg.Lroom_nym);
		cPrintf(getcfgmsg(163), String, bn);
		doccr();
#endif
		}

	for (r_slot i = 0; i < cfg.maxrooms; i++)
		{
#ifdef WINCIT
        letWindowsMultitask();
		if (hPB)
			{
			PostMessage(hPB, PBM_SETPOS, i, 0);
			}
#endif
		roomPos[i] = i;
		}

	putRoomPos();

#ifdef WINCIT
	if (hPB)
		{
		PostMessage(hPB, PBM_SETPOS, cfg.maxrooms, 0);
		}
#endif

#ifdef WINCIT
	letWindowsMultitask();
#endif
	}


// --------------------------------------------------------------------------
// zapLog2(): Erases & re-initializes LOG2.DAT.

void zapLog2(void)
	{
	LogEntry2 Log2(cfg.maxgroups);

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		Log2.Save(i);
		}
	}


// --------------------------------------------------------------------------
// zapLog3(): Erases & re-initializes LOG3.DAT.

void zapLog3(void)
	{
	LogEntry3 Log3(cfg.maxrooms);

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		Log3.Save(i);
		}
	}


// --------------------------------------------------------------------------
// zapLog4(): Erases & re-initializes LOG4.DAT.

void zapLog4(void)
	{
	LogEntry4 Log4(cfg.maxrooms);

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		Log4.Save(i);
		}
	}


// --------------------------------------------------------------------------
// zapLog5(): Erases & re-initializes LOG5.DAT.

void zapLog5(void)
	{
	LogEntry5 Log5(cfg.maxrooms);

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		Log5.Save(i);
		}
	}


// --------------------------------------------------------------------------
// zapLog6(): Erases & re-initializes LOG6.DAT.

void zapLog6(void)
	{
	LogEntry6 Log6(cfg.maxrooms);

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		Log6.Save(i);
		}
	}


// --------------------------------------------------------------------------
// zapLogFile(): Erases & re-initializes LOG?.DAT.

void zapLogFile(const Bool UseAlternateNames, const Bool Display)
	{
#ifdef WINCIT
	HWND hPB = NULL;
#endif
	LogEntry Log(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);

	if (Display)
		{
#ifdef WINCIT
		hPB = GetDlgItem(hCreateDlg, 103);

		if (hPB)
			{
			PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.MAXLOGTAB));
			}
#else
		cPrintf(getcfgmsg(154), ltoac(cfg.MAXLOGTAB), bn);
#endif
		}

	if (UseAlternateNames)
		{
		Log.LogEntry1::SetFileName(getcfgmsg(156));
		Log.LogEntry2::SetFileName(getcfgmsg(157));
		Log.LogEntry3::SetFileName(getcfgmsg(158));
		Log.LogEntry4::SetFileName(getcfgmsg(159));
		Log.LogEntry5::SetFileName(getcfgmsg(160));
		Log.LogEntry6::SetFileName(getcfgmsg(161));
		}

	label Buffer;

	// Yes, the parameter to the LogDatRecordSize() macro is somewhat cheezy
	WriteDataFileSize(Log.LogEntry1::GetFileName(Buffer, sizeof(Buffer)),
			LogDatRecordSize((*(LogDatStructure *) Buffer)));

	// write empty buffer all over file
	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
#ifdef WINCIT
		letWindowsMultitask();
#endif

		if (Display)
			{
#ifdef WINCIT
			if (hPB)
				{
				PostMessage(hPB, PBM_SETPOS, i, 0);
				}
#else
			cPrintf(getcfgmsg(152), getcfgmsg(155), ltoac(i + 1), br);
#endif
			}

		Log.Save(i, 0);
		}

	if (Display)
		{
#ifdef WINCIT
		if (hPB)
			{
			PostMessage(hPB, PBM_SETPOS, cfg.MAXLOGTAB, 0);
			}
#else
		doccr();
		doccr();
#endif
		}

#ifdef WINCIT
	letWindowsMultitask();
#endif
	}


// --------------------------------------------------------------------------
// zapRoomFile(): Erases and re-initailizes ROOM.DAT.

void zapRoomFile(void)
	{
	RoomC RoomBuffer;

	fseek(RoomFile, 0, SEEK_SET);

	const long l = RoomBuffer.GetDiskRecordSize();
	fwrite(&l, sizeof(l), 1, RoomFile);

#ifdef WINCIT
	HWND hPB = GetDlgItem(hCreateDlg, 102);

	if (hPB)
		{
		PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, cfg.maxrooms));
		}
#else
	cPrintf(getcfgmsg(151), ltoac(cfg.maxrooms), bn);
#endif

	for (r_slot i = DUMP + 1; i < cfg.maxrooms; i++)
		{

#ifdef WINCIT
        letWindowsMultitask();
		if (hPB)
			{
			PostMessage(hPB, PBM_SETPOS, i, 0);
			}
#else
		cPrintf(getcfgmsg(152), cfg.Lroom_nym, ltoac(i + 1), br);
#endif
		RoomBuffer.Save(i);
		}

	// Lobby> always exists -- guarantees us a place to stand
	RoomBuffer.SetName(getcfgmsg(144));
	RoomBuffer.SetInfoLine(getcfgmsg(100));
	RoomBuffer.SetPermanent(TRUE);
	RoomBuffer.SetPublic(TRUE);
	RoomBuffer.SetInuse(TRUE);
	RoomBuffer.Save(LOBBY);
	RoomBuffer.SetCreator(programName);

	// Mail> is also permanent...
	RoomBuffer.SetName(getcfgmsg(145));
	RoomBuffer.SetInfoLine(getcfgmsg(101));
	RoomBuffer.Save(MAILROOM);

	// Aide) also...
	RoomBuffer.SetName(getcfgmsg(146));
	RoomBuffer.SetInfoLine(getcfgmsg(148));
	RoomBuffer.SetPublic(FALSE);
	RoomBuffer.Save(AIDEROOM);

	// Dump) also...
	RoomBuffer.SetName(getcfgmsg(147));
	RoomBuffer.SetInfoLine(getcfgmsg(186));
	RoomBuffer.Save(DUMP);

#ifdef WINCIT
	if (hPB)
		{
		PostMessage(hPB, PBM_SETPOS, cfg.maxrooms, 0);
		}
#else
	doccr();
	doccr();
#endif

#ifdef WINCIT
	letWindowsMultitask();
#endif
	}
