// --------------------------------------------------------------------------
// Citadel: Resize.CPP
//
// File resizing code (for number of entries, not size of entries).

#include "ctdl.h"

#include "config.h"
#include "auxtab.h"
#include "filemake.h"
#include "log.h"
#include "msg.h"
#include "group.h"
#include "hall.h"
#include "extmsg.h"
#include "filefind.h"


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
// resizelogfile(): Resizes log file.

void resizelogfile(void)
	{
#ifdef WINCIT
	LogEntry    *CurrentUser = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
	l_slot      *LogOrder = new l_slot[cfg.MAXLOGTAB];

	if (!LogOrder || !CurrentUser)
		{
		illegal("no memory");
		}

	for (l_slot slot = 0; slot < cfg.MAXLOGTAB; slot++)
		{
		LogOrder[slot] = slot;
		}

	LogTab.Sort(&LogOrder);
#endif

	chdir(cfg.homepath);

	// get rid of any log?.tmp files
	unlink(getcfgmsg(156));
	unlink(getcfgmsg(157));
	unlink(getcfgmsg(158));
	unlink(getcfgmsg(159));
	unlink(getcfgmsg(160));
	unlink(getcfgmsg(161));

	// and the LE*.TMP files
	changedir(cfg.logextdir);
	ambigUnlink(getcfgmsg(215));
	changedir(cfg.homepath);

	// create new log?.tmp files with new size
	l_slot i = cfg.MAXLOGTAB;
	cfg.MAXLOGTAB = newmaxlogtab;
	zapLogFile(TRUE, TRUE);
	cfg.MAXLOGTAB = i;

	for (i = 0; (i < cfg.MAXLOGTAB) && (i < newmaxlogtab); i++)
		{
		msgDisp(getcfgmsg(183), ltoac(LTab(i).GetLogIndex()));
#ifndef WINCIT
		cPrintf(getcfgmsg(282), br);
#endif

		// Load from old files (the .DAT ones)
		CurrentUser->LogEntry1::ResetFileName();
		CurrentUser->LogEntry2::ResetFileName();
		CurrentUser->LogEntry3::ResetFileName();
		CurrentUser->LogEntry4::ResetFileName();
		CurrentUser->LogEntry5::ResetFileName();
		CurrentUser->LogEntry6::ResetFileName();
		CurrentUser->LogExtensions::ResetFileExtension();

		if (!CurrentUser->Load(LTab(i).GetLogIndex()))
			{
			crashout(getcfgmsg(281), getcfgmsg(155));
			}

		// And save to the new files (the .TMP ones)
		CurrentUser->LogEntry1::SetFileName(getcfgmsg(156));
		CurrentUser->LogEntry2::SetFileName(getcfgmsg(157));
		CurrentUser->LogEntry3::SetFileName(getcfgmsg(158));
		CurrentUser->LogEntry4::SetFileName(getcfgmsg(159));
		CurrentUser->LogEntry5::SetFileName(getcfgmsg(160));
		CurrentUser->LogEntry6::SetFileName(getcfgmsg(161));
		CurrentUser->LogExtensions::SetFileExtension(getcfgmsg(230));

		if (!CurrentUser->Save(i, CurrentUser->GetMessageRoom()))
			{
			crashout(getcfgmsg(283), getcfgmsg(155));
			}
		}

	doccr();
	doccr();

	// Back to using the .DAT files
	CurrentUser->LogEntry1::ResetFileName();
	CurrentUser->LogEntry2::ResetFileName();
	CurrentUser->LogEntry3::ResetFileName();
	CurrentUser->LogEntry4::ResetFileName();
	CurrentUser->LogEntry5::ResetFileName();
	CurrentUser->LogEntry6::ResetFileName();
	CurrentUser->LogExtensions::ResetFileExtension();

	// clear RAM buffer out
	CurrentUser->Clear();

	// Then clean up - .TMP to .DAT
	unlink(logDat);
	unlink(log2Dat);
	unlink(log3Dat);
	unlink(log4Dat);
	unlink(log5Dat);
	unlink(log6Dat);

	rename(getcfgmsg(156), logDat);
	rename(getcfgmsg(157), log2Dat);
	rename(getcfgmsg(158), log3Dat);
	rename(getcfgmsg(159), log4Dat);
	rename(getcfgmsg(160), log5Dat);
	rename(getcfgmsg(161), log6Dat);

	changedir(cfg.logextdir);
	ambigUnlink(getcfgmsg(211));

	FindFileC FF;
	if (FF.FindFirst(getcfgmsg(215)))
		{
		do
			{
			if (FF.IsNormalFile())
				{
				label newname;

				CopyStringToBuffer(newname, FF.GetShortName());
				strcpy(strchr(newname, '.'), getcfgmsg(285));
				rename(FF.GetShortName(), newname);
				}
			} while (FF.FindNext());
		}

	changedir(cfg.homepath);

#ifdef WINCIT
	msgDisp(NULL);
	//	msgCaption(ns);
	delete CurrentUser;
	delete [] LogOrder;
#endif
}


// --------------------------------------------------------------------------
// resizehallfile(): Resizes hall file.

void resizehallfile(void)
	{
	long l;

	const char *File2;
	char ch;

	FILE *oldfile;
	FILE *newfile;
	HallEntry1 tmpHall;

	msgDisp(getcfgmsg(184), cfg.Lhalls_nym);
	doccr();

	chdir(cfg.homepath);

	// do hall.dat
	File2 = getcfgmsg(216);

	msgDisp(getcfgmsg(185), hallDat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), File2);
		}
	if ((oldfile = fopen(hallDat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), hallDat);
		}

	fread(&l, sizeof(l), 1, oldfile);
	fwrite(&l, sizeof(l), 1, newfile);

	h_slot i;
	for (i = 0; i < cfg.maxhalls && i < newmaxhalls; i++)
		{
		if (!tmpHall.Load(i, oldfile))
			{
			crashout(getcfgmsg(281), getcfgmsg(236));
			}

		if (!tmpHall.Save(i, newfile))
			{
			crashout(getcfgmsg(283), getcfgmsg(236));
			}
		}

	tmpHall.Clear();

	for (; i < newmaxhalls; i++)
		{
		if (!tmpHall.Save(i, newfile))
			{
			crashout(getcfgmsg(283), getcfgmsg(236));
			}
		}

	fclose(newfile);
	fclose(oldfile);

	int result = unlink(hallDat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(187), hallDat);
		doccr();
		}

	result = rename(File2, hallDat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(188), File2);
		doccr();
		}

	// hall2.dat
	File2 = getcfgmsg(210);

	msgDisp(getcfgmsg(185), hall2Dat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), File2);
		}
	if ((oldfile = fopen(hall2Dat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), hall2Dat);
		}

	// inhall
	h_slot q;
	for (q = 0; q < cfg.maxhalls && q < newmaxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}
		}

	ch = 0;

	for (; q < newmaxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}
		}

	for (; q < cfg.maxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	// window
	for (q = 0; q < cfg.maxhalls && q < newmaxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}
		}

	ch = 0;

	for (; q < newmaxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}
		}

	for (; q < cfg.maxhalls; q++)
		{
		for (i = 0; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	fclose(newfile);
	fclose(oldfile);

	result = unlink(hall2Dat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(187), hall2Dat);
		doccr();
		}

	result = rename(File2, hall2Dat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(188), File2);
		doccr();
		}

	cfg.maxhalls = newmaxhalls;

	HallData.Resize(cfg.maxhalls, cfg.maxrooms);

	if (!HallData.IsValid())
		{
		crashout(getcfgmsg(107));
		}

	doccr();
	msgDisp(getcfgmsg(190));
	doccr();
	doccr();

#ifdef WINCIT
	msgDisp(NULL);
//	msgCaption(ns);
#endif
	}


// --------------------------------------------------------------------------
// resizegrpfile(): Resizes group file.

void resizegrpfile(void)
	{
	int i;
	const char *File2;
	FILE *oldfile;
	FILE *newfile;
#ifdef WINCIT
	LogEntry    *CurrentUser = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);

	if (!CurrentUser)
		{
		illegal("no memory.");
		}
#endif
	///////////////////////////////////////
	msgDisp(getcfgmsg(191), cfg.Ugroups_nym);
	doccr();

	chdir(cfg.homepath);

	File2 = getcfgmsg(217);
	msgDisp(getcfgmsg(185), grpDat);
	cPrintfDOS(br);

	unlink(File2);

	GroupData.Resize(newmaxgroups);

	zapGrpFile(File2, TRUE);

	Bool Success = FALSE;

	if ((oldfile = fopen(grpDat, FO_RB)) != NULL)
		{
		if ((newfile = fopen(File2, FO_RPB)) != NULL)
			{
			fseek(oldfile, sizeof(long), SEEK_SET);
			fseek(newfile, sizeof(long), SEEK_SET);

			for (g_slot GroupSlot = 0; (GroupSlot < cfg.maxgroups) && (GroupSlot < newmaxgroups); GroupSlot++)
				{
				GroupEntry TempEntry;

				if (!TempEntry.Load(oldfile))
					{
					crashout(getcfgmsg(281), getcfgmsg(240));
					}

				if (!TempEntry.Save(newfile))
					{
					crashout(getcfgmsg(283), getcfgmsg(240));
					}
				}

			Success = TRUE;

			fclose(newfile);
			}

		fclose(oldfile);
		}

	if (!Success)
		{
		crashout(getcfgmsg(218), grpDat);
		}

	// log2.dat
	Success = FALSE;

	File2 = getcfgmsg(157);
	msgDisp(getcfgmsg(185), log2Dat);
	cPrintfDOS(br);

	unlink(File2);

	if ((oldfile = fopen(log2Dat, FO_RB)) != NULL)
		{
		if ((newfile = fopen(File2, FO_WB)) != NULL)
			{
			for (l_index LogIndex = 0; LogIndex < cfg.MAXLOGTAB; LogIndex++)
				{
				char ch;

				for (i = 0; i < cfg.maxgroups / 8 && i < newmaxgroups / 8; i++)
					{
					fread(&ch, sizeof(ch), 1, oldfile);
					fwrite(&ch, sizeof(ch), 1, newfile);
					}

				ch = 0;

				for (; i < newmaxgroups / 8; i++)
					{
					fwrite(&ch, sizeof(ch), 1, newfile);
					}

				for (; i < cfg.maxgroups / 8; i++)
					{
					fread(&ch, sizeof(ch), 1, oldfile);
					}
				}

			Success = TRUE;

			fclose(newfile);
			}

		fclose(oldfile);
		}

	if (!Success)
		{
		crashout(getcfgmsg(218), log2Dat);
		}

	unlink(grpDat);
	rename(getcfgmsg(217), grpDat);
	unlink(log2Dat);
	rename(File2, log2Dat);

	cfg.maxgroups = newmaxgroups;

	CurrentUser->LogEntry2::Resize(cfg.maxgroups);

	if (!GroupData.IsValid() || !CurrentUser->IsValid())
		{
		crashout(getcfgmsg(107));
		}

	doccr();
	msgDisp(getcfgmsg(190));
	doccr();
	doccr();

#ifdef WINCIT
	msgDisp(NULL);
//	msgCaption(ns);
	delete CurrentUser;
#endif
	}


// --------------------------------------------------------------------------
// resizemsgfile(): Resizes message file.
void MessageDatC::Resize(long NewSizeInBytes)
	{
	Lock();

	Message *Msg = new Message;

	if (Msg)
		{
		char MsgFilePath2[128];

		FILE *oldmsg;
		FILE *newmsg;

		const char *msgTmp = getcfgmsg(219);

		sprintf(MsgFilePath2, sbs, cfg.msgpath, msgTmp);

		if (MsgDat)
			{
			fclose(MsgDat);
			MsgDat = NULL;
			}

		if (!citOpen(MsgFilePath2, CO_WPB, &MsgDat))
			{
			illegal(getmsg(8), getcfgmsg(192));
			}

		InitializeMessageFile(NewSizeInBytes);
		catLoc = 0l;

		fclose(MsgDat);
		MsgDat = NULL;

		// open first message file
		if (!citOpen(MsgFilePath, CO_RPB, &oldmsg))
			{
			msgDisp(getmsg(78), MsgFilePath);
			doccr();
			delete Msg;
			Unlock();
			return;
			}

		// open temp message file
		if (!citOpen(MsgFilePath2, CO_RPB, &newmsg))
			{
			msgDisp(getmsg(78), MsgFilePath2);
			doccr();
			fclose(oldmsg);
			delete Msg;
			Unlock();
			return;
			}

		for (m_index ID = OldestMessageInTable(); ID <= NewestMessage(); ID++)
			{
			MsgDat = oldmsg;

			if (LoadAll(ID, Msg) != RMS_OK)
				{
				continue;
				}

			const m_index here = atol(Msg->GetLocalID());

			if (here != ID)
				{
				continue;
				}

			msgDisp(getcfgmsg(193), cfg.Lmsg_nym, ltoac(here));
			cPrintfDOS(br);

			MsgDat = newmsg;

			ResizeStore(Msg);
			}

		doccr();
		doccr();

		fclose(oldmsg);
		fclose(newmsg);
		MsgDat = NULL;

		int result = unlink(MsgFilePath);
		if (result == -1)
			{
			msgDisp(getcfgmsg(187), msgDat);
			doccr();
			}

		result = rename(MsgFilePath2, MsgFilePath);
		if (result == -1)
			{
			msgDisp(getcfgmsg(188), msgTmp);
			doccr();
			}

		delete Msg;

		if (!OpenMessageFile(cfg.msgpath))
			{
			illegal(getmsg(78), MsgFilePath);
			}
		}
	else
		{
		crashout(getmsg(188), getcfgmsg(172));
		}

#ifdef WINCIT
	msgDisp(NULL);
//	msgCaption(ns);
#endif

	Unlock();
	}
