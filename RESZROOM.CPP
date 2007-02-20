// temp file

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "hall.h"
#include "config.h"
#include "msg.h"
#include "tallybuf.h"
#include "extmsg.h"


#ifdef WINCIT
BOOL CALLBACK EmptyDlgProc(HWND, UINT uMsg, WPARAM, LPARAM);

static const char *ltoac(int wow)
	{
	static label Buffer;
	sprintf(Buffer, "%d", wow);
	return (Buffer);
	}

#define doccr()
#endif

Bool MessageTableC::SetRooms(r_slot New)
	{
	m_index *OldFirstMessageInRoom = FirstMessageInRoom;
	m_index *OldLastMessageInRoom = LastMessageInRoom;

	FirstMessageInRoom = new m_index[New];
	LastMessageInRoom = new m_index[New];

	if (!FirstMessageInRoom || !LastMessageInRoom)
		{
		delete [] OldFirstMessageInRoom;
		delete [] OldLastMessageInRoom;

		crashout(getcfgmsg(107));
		}
	else
		{
		memset(FirstMessageInRoom, 0xff, sizeof(m_index) * New);
		memset(LastMessageInRoom, 0xff, sizeof(m_index) * New);

		if (OldFirstMessageInRoom)
			{
			delete [] OldFirstMessageInRoom;
			memcpy(FirstMessageInRoom, OldFirstMessageInRoom, sizeof(m_index) * min(cfg.maxrooms, New));
			}

		if (OldLastMessageInRoom)
			{
			delete [] OldLastMessageInRoom;
			memcpy(LastMessageInRoom, OldLastMessageInRoom, sizeof(m_index) * min(cfg.maxrooms, New));
			}
		}

	return (TRUE);	// because we just crash if we fail
	}

// --------------------------------------------------------------------------
// resizeroomfile(): Resizes room file.

void resizeroomfile(void)
	{
	int q, result, in;

	const char *File2;
	long l;
	char ch;

	FILE *oldfile;
	FILE *newfile;
	RoomC TempRoom;

	r_slot checkedmaxrooms = 0;
	for (r_slot i = 0; i < cfg.maxrooms; i++)
		{
		if (RoomTab[i].IsInuse())
			{
			checkedmaxrooms = i;
			}
		}

	if (checkedmaxrooms % 8)
		{
		checkedmaxrooms = (r_slot) ((checkedmaxrooms / 8 + 1) * 8);
		}

	if (checkedmaxrooms > newmaxrooms)
		{
		msgDisp(getcfgmsg(246), newmaxrooms, checkedmaxrooms);
		doccr();
		newmaxrooms = checkedmaxrooms;
		}

	if (newmaxrooms == cfg.maxrooms)
		{
		return;
		}

	msgDisp(getcfgmsg(191), cfg.Lrooms_nym);
	doccr();

	chdir(cfg.homepath);

	// do room.dat
	File2 = getcfgmsg(208);
	msgDisp(getcfgmsg(185), roomDat);
	cPrintfDOS(br);
	fclose(RoomFile);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), getcfgmsg(195));
		}

	if ((oldfile = fopen(roomDat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), roomDat);
		}

	fread(&l, sizeof(l), 1, oldfile);
	fwrite(&l, sizeof(l), 1, newfile);

#ifndef VISUALC
	r_slot i;
#endif
#ifdef MINGW
	r_slot i;
#endif

#ifdef WINCIT
	if (!RoomTab.Resize(newmaxrooms))
		{
		crashout(getcfgmsg(113));
		}
#endif

	for (i = 0; i < cfg.maxrooms && i < newmaxrooms; i++)
		{
		if (!(i % 10))
			{
			msgDisp(getcfgmsg(196), roomDat, ltoac(i));
			cPrintfDOS(br);
			}

		// this bit with the roomfl is somewhat icky, but i'm not going to
		// fix it now. cope.
		RoomFile = oldfile;
		TempRoom.Load(i);
		RoomFile = newfile;
		TempRoom.Save(i);
		}

	TempRoom.Clear();

	// This should already be the case, but I feel better doing it again
	RoomFile = newfile;
	for (; i < newmaxrooms; i++)
		{
		if (!(i % 10))
			{
			msgDisp(getcfgmsg(196), roomDat, ltoac(i));
			cPrintfDOS(br);
			}

		TempRoom.Save(i);
		}

	msgDisp(getcfgmsg(196), roomDat, ltoac(i));
	cPrintfDOS(br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

	result = unlink(roomDat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(187), roomDat);
		doccr();
		}

	result = rename(File2, roomDat);
	if (result == -1)
		{
		msgDisp(getcfgmsg(188), File2);
		doccr();
		}

	// log3.dat
	File2 = getcfgmsg(158);

	msgDisp(getcfgmsg(185), log3Dat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), getcfgmsg(197));
		}

	if ((oldfile = fopen(log3Dat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), log3Dat);
		}

	for (q = 0; q < cfg.MAXLOGTAB; q++)
		{
		msgDisp(getcfgmsg(196), log3Dat, ltoac(q));
		cPrintfDOS(br);

		for (i = 0; i < cfg.maxrooms / 8 && i < newmaxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		ch = 0;

		for (; i < newmaxrooms / 8; i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		for (; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), log3Dat, ltoac(q));
	cPrintfDOS(br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

	unlink(log3Dat);
	rename(File2, log3Dat);

	// log4.dat
	File2 = getcfgmsg(159);

	msgDisp(getcfgmsg(185), log4Dat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), getcfgmsg(198));
		}

	if ((oldfile = fopen(log4Dat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), log4Dat);
		}

	for (q = 0; q < cfg.MAXLOGTAB; q++)
		{
		msgDisp(getcfgmsg(196), log4Dat, ltoac(q));
		cPrintfDOS(br);

		for (i = 0; (i < (cfg.maxrooms / 8)) && (i < (newmaxrooms / 8)); i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		ch = 0;

		for (; i < (newmaxrooms / 8); i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		for (; i < (cfg.maxrooms / 8); i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), log4Dat, ltoac(q));
	cPrintfDOS(br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

	unlink(log4Dat);
	rename(File2, log4Dat);


	// log5.dat
	File2 = getcfgmsg(160);

	msgDisp(getcfgmsg(185), log5Dat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), getcfgmsg(199));
		}

	if ((oldfile = fopen(log5Dat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), log5Dat);
		}

	for (q = 0; q < cfg.MAXLOGTAB; q++)
		{
		msgDisp(getcfgmsg(196), log5Dat, ltoac(q));
		cPrintfDOS(br);

		for (i = 0; (i < cfg.maxrooms) && (i < newmaxrooms); i++)
			{
			fread(&l, sizeof(l), 1, oldfile);
			fwrite(&l, sizeof(l), 1, newfile);
			}

		l = 0;

		for (; i < newmaxrooms; i++)
			{
			fwrite(&l, sizeof(l), 1, newfile);
			}

		for (; i < cfg.maxrooms; i++)
			{
			fread(&l, sizeof(l), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), log5Dat, ltoac(q));
	cPrintfDOS(br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

	unlink(log5Dat);
	rename(File2, log5Dat);


	// log6.dat
	File2 = getcfgmsg(161);

	msgDisp(getcfgmsg(185), log6Dat);
	cPrintfDOS(br);

	if ((newfile = fopen(File2, FO_WB)) == NULL)
		{
		illegal(getmsg(8), getcfgmsg(200));
		}

	if ((oldfile = fopen(log6Dat, FO_RB)) == NULL)
		{
		illegal(getmsg(83), log6Dat);
		}

	for (q = 0; q < cfg.MAXLOGTAB; q++)
		{
		msgDisp(getcfgmsg(196), log6Dat, ltoac(q));
		cPrintfDOS(br);

		for (i = 0; (i < (cfg.maxrooms / 8)) && (i < (newmaxrooms / 8)); i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		ch = 0;

		for (; i < (newmaxrooms / 8); i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		for (; i < (cfg.maxrooms / 8); i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), log6Dat, ltoac(q));
	cPrintfDOS(br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

	unlink(log6Dat);
	rename(File2, log6Dat);


	// roompos.dat
	if (newmaxrooms < cfg.maxrooms)
		{
		msgDisp(getcfgmsg(201), roomposDat);
		msgDisp(bn);
		// trash it
		if ((newfile = fopen(roomposDat, FO_WB)) == NULL)
			{
			illegal(getmsg(661), roomposDat);
			}

		for (r_slot i = 0; i < newmaxrooms; i++)
			{
			fwrite(&i, sizeof(i), 1, newfile);
			}
		fclose(newfile);
		}
	else
		{
		File2 = getcfgmsg(209);
		msgDisp(getcfgmsg(185), roomposDat);
		cPrintfDOS(br);
		if ((newfile = fopen(File2, FO_WB)) == NULL)
			{
			illegal(getmsg(8), File2);
			}

		if ((oldfile = fopen(roomposDat, FO_RB)) == NULL)
			{
			illegal(getmsg(83), roomposDat);
			}

		r_slot i;
		for (i = 0; i < cfg.maxrooms && i < newmaxrooms; i++)
			{
			fread(&in, sizeof(in), 1, oldfile);
			fwrite(&in, sizeof(in), 1, newfile);
			}

		for (; i < newmaxrooms; i++)
			{
			fwrite(&i, sizeof(i), 1, newfile);
			}

		fclose(newfile);
		fclose(oldfile);

		result = unlink(roomposDat);
		if (result == -1)
			{
			msgDisp(getcfgmsg(187), roomposDat);
			doccr();
			}

		result = rename(File2, roomposDat);
		if (result == -1)
			{
			msgDisp(getcfgmsg(188), File2);
			doccr();
			}
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

	for (q = 0; q < cfg.maxhalls; q++)
		{
		msgDisp(getcfgmsg(196), hall2Dat, ltoac(q));
		cPrintfDOS(br);

		for (i = 0; i < cfg.maxrooms / 8 && i < newmaxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		ch = 0;

		for (; i < newmaxrooms / 8; i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		for (; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), hall2Dat, ltoac(q));
	cPrintfDOS(br);

	for (q = 0; q < cfg.maxhalls; q++)
		{
		msgDisp(getcfgmsg(196), hall2Dat, ltoac(q));
		msgDisp(getcfgmsg(282), br);

		for (i = 0; i < cfg.maxrooms / 8 && i < newmaxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		ch = 0;

		for (; i < newmaxrooms / 8; i++)
			{
			fwrite(&ch, sizeof(ch), 1, newfile);
			}

		for (; i < cfg.maxrooms / 8; i++)
			{
			fread(&ch, sizeof(ch), 1, oldfile);
			}
		}

	msgDisp(getcfgmsg(196), hall2Dat, ltoac(q));
	msgDisp(getcfgmsg(282), br);

	fclose(newfile);
	fclose(oldfile);
	doccr();

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

	#if defined(AUXMEM) || defined(WINCIT)
		MessageDat.SetRooms(newmaxrooms);
	#endif			

	cfg.maxrooms = newmaxrooms;
	if (!RoomTab.Resize(cfg.maxrooms))
		{
		crashout(getcfgmsg(113));
		}

#ifndef WINCIT
	CurrentUser->LogEntry3::Resize(cfg.maxrooms);
	CurrentUser->LogEntry4::Resize(cfg.maxrooms);
	CurrentUser->LogEntry5::Resize(cfg.maxrooms);
	CurrentUser->LogEntry6::Resize(cfg.maxrooms);
#endif

	HallData.Resize(cfg.maxhalls, cfg.maxrooms);


	// I don't know if this is the best place to put this.
#ifndef MULTI
    Talley->~TalleyBufferC();
#endif


	delete [] roomPos;

	if (	((roomPos = new r_slot[cfg.maxrooms]) == NULL) ||
			!HallData.IsValid()
#ifndef MULTI
			|| !CurrentUser->IsValid() ||
            !(Talley = new TalleyBufferC())
#endif
       )
		{
		crashout(getcfgmsg(107));
		}

	openFile(roomDat, &RoomFile);

#ifdef WINCIT
	hConfigDlg = CreateDialog(hInstance, "CONFIGURE", NULL,
			(DLGPROC) EmptyDlgProc);
#endif

	RoomTabBld();

#ifdef WINCIT
	if (hConfigDlg)
		{
		DestroyWindow(hConfigDlg);
		hConfigDlg = NULL;
		}
#else
	msgDisp(getcfgmsg(190));
	doccr();
	doccr();
#endif
	}
