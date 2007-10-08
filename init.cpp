// --------------------------------------------------------------------------
// Citadel: Init.CPP
//
// Citadel initialization code.

#include "ctdl.h"
#pragma hdrstop

#include "config.h"
#include "net.h"
#include "music.h"
#include "libovl.h"
#include "meminit.h"
#include "account.h"
#include "infofile.h"
#include "group.h"
#include "hall.h"
#include "log.h"
#include "scrlback.h"
#include "timer.h"
#include "tallybuf.h"
#include "ser.h"
#include "cwindows.h"
#include "events.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "room.h"
#include "datafile.h"
#include "cfgfiles.h"
#include "resize.h"
#include "filemake.h"
#ifndef WINCIT
#include "statline.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
// checkfiles()     makes sure there are enough FILES in CONFIG.SYS.
// initCitadel()    Load up data files and open everything.


Bool    resizeMsg = FALSE,      // Resize messages?
        resizeLog = FALSE,      // Resize userlog?
        resizeGrp = FALSE,      // Resize groups?
        resizeRoom = FALSE,     // Resize rooms?
        resizeHall = FALSE;     // Resize halls?

Bool CitadelIsConfiguring;  // Don't do some things.


extern ConsoleLockC ConsoleLock;
#ifndef WINCIT
extern Bool TimeoutChecking;
#endif


// --------------------------------------------------------------------------
// checkfiles(): Makes sure there are enough FILES in config.sys.

int FilesRequired = 13;

static void checkfiles(void)
    {
    const int i = fileHandlesLeft();

    if (i < FilesRequired)
        {
#ifdef WINCIT
        char Buffer[128];
        sprintf(Buffer, getcfgmsg(118), FilesRequired - i);
        MessageBox(NULL, Buffer, NULL, MB_ICONSTOP | MB_OK);
#else
        printf(bn);
        printf(getcfgmsg(117));
        printf(bn);
        printf(getcfgmsg(118), FilesRequired - i);
        printf(bn);
        DeinitializeTimer();
        critical(FALSE);
#endif
        exit(200);
        }
    }

int fileHandlesLeft(void)
    {
    FILE *files[FOPEN_MAX];
    int i, count = FOPEN_MAX;

    for (i = 0; i < FOPEN_MAX; i++)
        {
        if ((files[i] = fopen(fullExePath, FO_RB)) == NULL)
            {
            count = i;
            break;
            }
        }

    for (i = 0; i < count && i < FOPEN_MAX; i++)
        {
        fclose(files[i]);
        }

    return (count);
    }

// by David Gibbs
// FidoNet: 1:115/439.0
// Internet: David.Gibbs@f439.n115.z1.fidonet.org
// Modified, of course.
#define is_DOS  0x01
#define is_OS2  0x02
#define is_DV   0x04
#define is_WINS 0x08
#define is_WIN3 0x10

static void get_os(void)
    {
#ifndef WINCIT
    union REGS t_regs;

    OSMajor = 0;
    OSMinor = 0;
    int osType = 0;

    // test for DOS or OS/2
    if (_osmajor < 10)
        {
        OSMajor = _osmajor;
        OSMinor = _osminor;

        osType |= is_DOS;
        }
    else
        {
        OSMajor = _osmajor / 10;
        OSMinor = _osminor;

        osType |= is_OS2;
        }

    // test for Windows
    t_regs.x.ax = 0x4680;
    int86(0x2F, &t_regs, &t_regs);

    if (t_regs.x.ax == 0x0000)
        {
        OSMajor = 3;
        OSMinor = 0;

        osType |= is_WINS;
        }
    else
        {
        t_regs.x.ax = 0x1600;
        int86(0x2F, &t_regs, &t_regs);

        switch (t_regs.h.al)
            {
            case 0x00:
            case 0x80:
            case 0x01:
            case 0xFF:
                {
                break;
                }

            default:
                {
                OSMajor = t_regs.h.al;
                OSMinor = t_regs.h.ah;
                osType |= is_WIN3;
                break;
                }
            }
        }

    // Test for DESQview
    t_regs.x.cx = 0x4445;   // load incorrect date
    t_regs.x.dx = 0x5351;
    t_regs.x.ax = 0x2B01;   // DV set up call

    intdos(&t_regs, &t_regs);
    if (t_regs.h.al != 0xFF)
        {
        OSMajor = t_regs.h.bh;
        OSMinor = t_regs.h.bl;

        osType |= is_DV;
        }

    if (osType & is_DOS)
        {
        OSType = OS_DOS;
        }

    if (osType & is_WINS)
        {
        OSType = OS_WINS;
        }

    if (osType & is_WIN3)
        {
        OSType = OS_WIN3;
        }

    if (osType & is_DV)
        {
        OSType = OS_DV;
        }

    if (osType & is_OS2)
        {
        OSType = OS_OS2;
        }
#endif
    }

// --------------------------------------------------------------------------
// checkresize(): Checks to see if files need to be resized.

static void checkresize(void)
	{
	int fh;
	long result;
    char MsgFilePath[128];
	// save old values for later
	newmessagek = cfg.MsgDatSizeInK;
	newmaxrooms = cfg.maxrooms;
	newmaxhalls = cfg.maxhalls;
	newmaxlogtab = cfg.MAXLOGTAB;
	newmaxgroups = cfg.maxgroups;

	// check message file
	sprintf(MsgFilePath, sbs, cfg.msgpath, msgDat);
	fh = open(MsgFilePath, O_RDONLY);
	if (fh != -1)
		{
		if ((result = filelength(fh)) == 0)
			{
			close(fh);
			unlink(MsgFilePath);
			}
		else if (result != ((long) cfg.MsgDatSizeInK * 1024l))
			{
			resizeMsg = TRUE;
#ifdef WINCIT
			errorDisp(getcfgmsg(139), msgDat);
#endif
#ifndef WINCIT
			doccr();
#endif

			// set messagek to actual value
			cfg.MsgDatSizeInK = result / 1024l;
			close(fh);
			}
		else
			{
			close(fh);
			}
		}

	// check log file
	fh = open(logDat, O_RDONLY);
	if (fh != -1)
		{
		if ((result = filelength(fh)) == 0)
			{
			close(fh);
			unlink(logDat);
			unlink(log2Dat);
			unlink(log3Dat);
			unlink(log4Dat);
			unlink(log5Dat);
			unlink(log6Dat);
			}
		else if (result != ((long) cfg.MAXLOGTAB * (long) LogDatRecordSize(*(LogDatStructure *) NULL)) + (long) sizeof(long))
			{
			resizeLog = TRUE;
#ifdef WINCIT
			errorDisp(getcfgmsg(139), logDat);
#endif
#ifndef WINCIT
			doccr();
#endif

			// set MAXLOGTAB to actual value
			cfg.MAXLOGTAB = (l_slot) ((result - sizeof(long)) / (long) LogDatRecordSize(*(LogDatStructure *) NULL));
			close(fh);
			}
		else
			{
			close(fh);
			}
		}

	// check group file
	fh = open(grpDat, O_RDONLY);
	if (fh != -1)
		{
		if ((result = filelength(fh)) == 0)
			{
			close(fh);
			unlink(grpDat);
			}
		else if (result != ((long) cfg.maxgroups * (long) ((GroupEntry *) NULL)->SizeOfDiskRecord()) + (long) sizeof(long))
			{
			resizeGrp = TRUE;
#ifdef WINCIT
			errorDisp(getcfgmsg(139), grpDat);
#endif
#ifndef WINCIT
			doccr();
#endif

			// set maxgroups to actual value
			cfg.maxgroups = (g_slot) ((result - sizeof(long)) / (long) sizeof(GroupEntry));
			close(fh);
			}
		else
			{
			close(fh);
			}
		}

	// check room file
	fh = open(roomDat, O_RDONLY);
	if (fh != -1)
		{
		if ((result = filelength(fh)) == 0)
			{
			close(fh);
			unlink(roomDat);
			}
		else if (result != ((long) cfg.maxrooms * (long) RoomC::GetDiskRecordSize()) + (long) sizeof(long))
			{
			resizeRoom = TRUE;
#ifdef WINCIT
			errorDisp(getcfgmsg(139), roomDat);
#endif
#ifndef WINCIT
			doccr();
#endif

			// set maxrooms to actual value
			cfg.maxrooms = (r_slot) ((result - sizeof(long)) /
					(long) RoomC::GetDiskRecordSize());
			close(fh);
			}
		else
			{
			close(fh);
			}
		}

	// check hall file
	fh = open(hallDat, O_RDONLY);
	if (fh != -1)
		{
		if ((result = filelength(fh)) == 0)
			{
			close(fh);
			unlink(hallDat);
			unlink(hall2Dat);
			}
		else if (result != (long) cfg.maxhalls *  (long) ((HallEntry1 *) NULL)->SizeOfDiskRecord() + (long) sizeof(long))
			{
			resizeHall = TRUE;
#ifdef WINCIT
			errorDisp(getcfgmsg(139), hallDat);
#endif
#ifndef WINCIT
			doccr();
#endif

			// set maxhalls to actual value
			cfg.maxhalls = (h_slot) ((result - sizeof(long)) /
					(long) ((HallEntry1 *) NULL)->SizeOfDiskRecord());
			close(fh);
			}
		else
			{
			close(fh);
			}
		}

	if (resizeMsg || resizeLog || resizeRoom || resizeGrp || resizeHall)
		{
#ifndef WINCIT
		doccr();
#endif
		pause(200);
		}
	}

// --------------------------------------------------------------------------
// autoConvert(): Up/downsizes .DAT files.

static void autoConvert(const char *fname, long size)
	{
	FILE *fl;
	long l;

	if ((fl = fopen(fname, FO_RPB)) != NULL)
		{
		fread(&l, sizeof(l), 1, fl);

		if (l != size)
			{
			FILE *fl2;
			const char *tmp = getcfgmsg(137);

			if ((fl2 = fopen(tmp, FO_WPB)) != NULL)
				{
				char *buf;
				size_t bufsize = (size_t) max(l, size);

				buf = new char[bufsize];

				if (buf)
					{
					int cnt = 0;

					fwrite(&size, sizeof(size), 1, fl2);

					while (!feof(fl))
						{
						memset(buf, 0, bufsize);

						if (fread(buf, 1, (size_t) l, fl) == (size_t) l)
							{
#ifndef WINCIT
							cPrintf(br);
							cPrintf(getcfgmsg(138), fname, ltoac(++cnt));
#endif
							fwrite(buf, 1, (size_t) size, fl2);
							}
						}

#ifndef WINCIT
					doccr();
					doccr();
#endif
					delete [] buf;
					}
				else
					{
					crashout(getmsg(188), getcfgmsg(203));
					}

				fclose(fl2);
				fclose(fl);

				unlink(fname);
				rename(tmp, fname);
				unlink(tmp);

				return; // avoid the fclose down there
				}
			else
				{
				crashout(getmsg(8), tmp);
				}
			}

		fclose(fl);
		}
	}

// --------------------------------------------------------------------------
// initfiles(): Initializes files, opens them.

static void initfiles(Bool *grpZap, Bool *hallZap, Bool *msgZap, Bool *logZap, Bool *log2Zap, Bool *log3Zap,
		Bool *log4Zap, Bool *log5Zap, Bool *log6Zap, Bool *roomZap, Bool *roomposZap)
	{
	FILE *aFile;

	changedir(cfg.homepath);

	if (cfg.msgpath[(strlen(cfg.msgpath) - 1)] == '\\')
		{
		cfg.msgpath[(strlen(cfg.msgpath) - 1)] = 0;
		}

	autoConvert(grpDat, ((GroupEntry *) NULL)->SizeOfDiskRecord());
	autoConvert(logDat, LogDatRecordSize(*(LogDatStructure *) NULL));
	autoConvert(hallDat, ((HallEntry1 *) NULL)->SizeOfDiskRecord());
	autoConvert(roomDat, RoomC::GetDiskRecordSize());

	checkresize();

	// check group file
	if (!citOpen(grpDat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), grpDat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
		doccr();
#endif
		*grpZap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// check hall file
	if (!citOpen(hallDat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), hallDat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
		doccr();
#endif
		*hallZap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// check room pos file
	if (!citOpen(roomposDat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), roomposDat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
		doccr();
#endif
		*roomposZap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log file
	if (!citOpen(logDat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), logDat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*logZap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log2 file
	if (!citOpen(log2Dat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), log2Dat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*log2Zap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log3 file
	if (!citOpen(log3Dat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), log3Dat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*log3Zap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log4 file
	if (!citOpen(log4Dat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), log4Dat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*log4Zap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log5 file
	if (!citOpen(log5Dat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), log5Dat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*log5Zap = TRUE;
		}
	else
		{
		fclose(aFile);
		}

	// open log6 file
	if (!citOpen(log6Dat, CO_RPB, &aFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), log6Dat);
#endif
#ifndef WINCIT
		doccr();
#endif
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#endif
#ifndef WINCIT
		doccr();
#endif
		*log6Zap = TRUE;
		}
	else
		{
		fclose(aFile);
		}


#ifndef WINCIT
	if (logZap || log2Zap || log3Zap || log4Zap || log5Zap || log6Zap)
		{
		doccr();
		}
#endif

	// open message file
	if (!MessageDat.OpenMessageFile(cfg.msgpath))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), MessageDat.GetFilename());
#endif
#ifndef WINCIT
		doccr();
#endif

		if (!MessageDat.CreateMessageFile(cfg.msgpath))
			{
			illegal(getmsg(8), MessageDat.GetFilename());
			}
#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#else
        printf("problem with msg.dat\n");
#endif
#ifndef WINCIT
		doccr();
		doccr();
#endif
		*msgZap = TRUE;
		}

	// open room file
	if (!citOpen(roomDat, CO_RPB, &RoomFile))
		{
#ifdef WINCIT
		errorDisp(getcfgmsg(140), roomDat);
#endif
#ifndef WINCIT
		doccr();
#endif

		if (!citOpen(roomDat, CO_WPB, &RoomFile))
			{
			illegal(getmsg(8), roomDat);
			}

#ifdef WINCIT
		errorDisp(getcfgmsg(141));
#else
        printf("problem with room.dat\n");
#endif
#ifndef WINCIT
		doccr();
		doccr();
#endif
		*roomZap = TRUE;
		}
	}

// --------------------------------------------------------------------------
// configcit(): The main configuration for Citadel

#ifdef WINCIT
BOOL CALLBACK EmptyDlgProc(HWND, UINT uMsg, WPARAM, LPARAM)
    {
    return (uMsg == WM_INITDIALOG);
    }
#endif


void CreateConfigurationFile(int FileNameIndex, int FileDataIndex, Bool PrintStuff)
    {
    discardable *d;

    if (!filexists(citfiles[FileNameIndex]))
        {
        if ((d = readData(16, FileDataIndex, FileDataIndex)) != NULL)
            {
            FILE *File;

            if ((File = fopen(citfiles[FileNameIndex], FO_W)) != NULL)
                {
                if (PrintStuff)
                    {
#ifndef WINCIT
                    cPrintf(getcfgmsg(133), citfiles[FileNameIndex]);
                    doccr();
#endif
                    }

                const char **Data = (const char **) d->aux;

                for (int i = 0; !SameString(Data[i], getcfgmsg(244)); i++)
                    {
                    fprintf(File, getmsg(472), Data[i], bn);
                    }

                fclose(File);
                }
            else
                {
                if (PrintStuff)
                    {
#ifndef WINCIT
                    cPrintf(getcfgmsg(134), citfiles[FileNameIndex]);
                    doccr();
#endif
                    }
                }

            discardData(d);
            }
        else
            {
            if (PrintStuff)
                {
                cOutOfMemory(51);
                }
            }
        }
    }

static Bool configcit(void)
    {
    Bool oldresizelog = FALSE, oldresizemsg = FALSE;

    CitadelIsConfiguring = TRUE;

    VerifyHeap();

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


    // read CONFIG.CIT
    if (!readconfig(NULL, 0))
        {
        return (FALSE);
        }


	Bool grpZap = FALSE, hallZap = FALSE, msgZap = FALSE, logZap = FALSE, log2Zap = FALSE, log3Zap = FALSE,
			log4Zap = FALSE, log5Zap = FALSE, log6Zap = FALSE, roomZap = FALSE, roomposZap = FALSE;

    initfiles(&grpZap, &hallZap, &msgZap, &logZap, &log2Zap, &log3Zap, &log4Zap, &log5Zap, &log6Zap, &roomZap, &roomposZap);

    VerifyHeap();

    // if we are about to make a new system, make default .CIT files.
    if (msgZap && roomZap && logZap && grpZap && hallZap && roomposZap)
        {
        CreateConfigurationFile(C_EXTERNAL_CIT, 0, TRUE);
        CreateConfigurationFile(C_NODES_CIT, 1, TRUE);
        CreateConfigurationFile(C_GRPDATA_CIT, 2, TRUE);
        CreateConfigurationFile(C_CRON_CIT, 3, TRUE);
        CreateConfigurationFile(C_MDMRESLT_CIT, 4, TRUE);
        CreateConfigurationFile(C_PROTOCOL_CIT, 5, TRUE);
        CreateConfigurationFile(C_TERMCAP_CIT, 6, TRUE);
        CreateConfigurationFile(C_COMMANDS_CIT, 7, TRUE);
        }

    VerifyHeap(1);

    // allocate tables here so readconfig() can be called from sysop menu
    allocateTables();

#ifndef WINCIT
    setdefaultTerm(TT_ANSI);
    termCap(TERM_NORMAL);
#endif

    VerifyHeap(1);

#ifdef WINCIT
    if (msgZap || roomZap || logZap || grpZap || hallZap || roomposZap)
        {
        hCreateDlg = CreateDialog(hInstance, "CREATEDATAFILES", NULL, (DLGPROC) EmptyDlgProc);
        }
#endif

    if (msgZap)
        {
        MessageDat.InitializeMessageFile(cfg.MsgDatSizeInK * 1024l);
        }

    if (roomZap)    zapRoomFile();
    if (logZap)     zapLogFile(FALSE, TRUE);
    if (grpZap)     zapGrpFile(NULL, TRUE);
    if (hallZap)    zapHallFile();
    if (roomposZap) zapRoomPosFile(TRUE);

    if (roomZap && !msgZap) roomBuild = TRUE;
    if (hallZap && !msgZap) hallBuild = TRUE;

    if (!logZap)
        {
        if (log2Zap)    zapLog2();
        if (log3Zap)    zapLog3();
        if (log4Zap)    zapLog4();
        if (log5Zap)    zapLog5();
        if (log6Zap)    zapLog6();
        }

#ifdef WINCIT
    if (hCreateDlg)
        {
        DestroyWindow(hCreateDlg);
        hCreateDlg = NULL;
        }

// no go for debug version; td32 go crash crash
    hConfigDlg = CreateDialog(hInstance, "CONFIGURE", NULL, (DLGPROC) EmptyDlgProc);
#endif

    VerifyHeap(1);

    logInit();

    VerifyHeap(1);

	MessageDat.BuildTable();

    VerifyHeap(1);

    RoomTabBld();

    VerifyHeap(1);

#ifdef WINCIT
    if (hConfigDlg)
        {
        DestroyWindow(hConfigDlg);
        hConfigDlg = NULL;
        }
#endif

    if (hallBuild)
        {
        buildhalls();
        VerifyHeap(1);
        }

    if (resizeRoom)
        {
        resizeroomfile();
        VerifyHeap(1);
        }

    if (resizeGrp)
        {
        resizegrpfile();
        VerifyHeap(1);
        }

    if (resizeHall)
        {
        resizehallfile();
        VerifyHeap(1);
        }

    if (resizeLog)
        {
        resizelogfile();
        VerifyHeap(1);
        }

    if (resizeMsg)
        {
        MessageDat.Resize(newmessagek * 1024l);	// bibnewmsg
        VerifyHeap(1);
        }

    if (resizeLog || resizeMsg || resizeRoom || resizeGrp)
        {
        oldresizelog = resizeLog;
        oldresizemsg = resizeMsg;
        resizeLog = FALSE;
        resizeMsg = FALSE;

        MessageDat.CloseMessageFile();

#ifndef MINGW
        fcloseall();
#endif

        cfg.MsgDatSizeInK = newmessagek;
        cfg.MAXLOGTAB = newmaxlogtab;

        initfiles(&grpZap, &hallZap, &msgZap, &logZap, &log2Zap, &log3Zap, &log4Zap, &log5Zap, &log6Zap, &roomZap, &roomposZap);
        }

    VerifyHeap(1);

#ifdef WINCIT
    if (oldresizelog || oldresizemsg)
        {
        hConfigDlg = CreateDialog(hInstance, "CONFIGURE", NULL, (DLGPROC) EmptyDlgProc);
        }
#endif

    if (oldresizelog)
        {
        if (!LogTab.Resize(cfg.MAXLOGTAB))
            {
            crashout(getcfgmsg(116));
            }

        VerifyHeap(1);

        logInit();

        VerifyHeap(1);
        }

	if (oldresizemsg)	// bibnewmsg - required?
        {
        MessageDat.SetTableSize(cfg.nmessages);

        VerifyHeap(1);

        MessageDat.BuildTable();

        VerifyHeap(1);
        }

#ifdef WINCIT
    if (hConfigDlg)
        {
        DestroyWindow(hConfigDlg);
        hConfigDlg = NULL;
        }
#else
    doccr();
    cPrintf(getcfgmsg(135));
    doccr();
    pause(200);
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

    VerifyHeap(1);

    CitadelIsConfiguring = FALSE;

    return (TRUE);
    }


// --------------------------------------------------------------------------
// initCitadel(): Load up data files and open everything.

static Bool initCitadel(void)
    {
    if (!read_cfg_messages())
        {
#ifdef WINCIT
        char Buffer[128];
        sprintf(Buffer, getmsg(188), getmsg(671));
        MessageBox(NULL, Buffer, NULL, MB_ICONSTOP | MB_OK);
#else
        printf(getmsg(188), getmsg(671));
#endif
        return (FALSE);
        }

    checkfiles();

    initExtDrivers();

    get_os();

	cfg.battr = 0xff;
    setscreen();

    logo(TRUE); // no go for debug version; td32 go crash crash

    init_internal_sound();

	// some mouse initialization technology!!!!
    initMouseHandler();
    hideCounter = 1;

    if (time(NULL) < 700000000L)
        {
#ifdef WINCIT
        MessageBox(NULL, getcfgmsg(119), NULL, MB_ICONSTOP | MB_OK);
#else
        doccr();
        doccr();
        cPrintf(getcfgmsg(119));
        doccr();
#endif
        dump_cfg_messages();
        return (FALSE);
        }


    static char prompt[92];
    static char citadel[92];
    char *envprompt;
    char *citprompt;

    envprompt = getenv(getcfgmsg(120));
    citprompt = getenv(getcfgmsg(121));
    if (citprompt)
        {
        sprintf(prompt, getcfgmsg(122), citprompt);
        }
    else if (envprompt)
        {
        sprintf(prompt, getcfgmsg(123), envprompt);
        }
    else
        {
        strcpy(prompt, getcfgmsg(124));
        }
    putenv(prompt);

    sprintf(citadel, getcfgmsg(125), programName, version);
    putenv(citadel);


#ifndef WINCIT
    OC.whichIO = CONSOLE;
    OC.SetOutFlag(OUTOK);
    OC.Echo = BOTH;
    OC.setio();
#endif

    VerifyHeap(1);

    // If we aren't reconfiguring, load the tables...
    if (!reconfig)
        {
        // Start by reading ETC.TAB
        getcwd(etcpath, 64);

        FILE *fd;
        if ((fd = fopen(etcTab, FO_RB)) != NULL)
            {
            if (filelength(fileno(fd)) != (long) sizeof(config) ||
					fread(&cfg, 1, sizeof(config), fd) != (long) sizeof(config))
                {
                memset(&cfg, 0, sizeof(cfg));
                reconfig = TRUE;
                }

            fclose(fd);
            unlink(etcTab);


            // If ETC.TAB could be loaded, load the rest
            if (!reconfig)
                {
                changedir(cfg.homepath);

                allocateTables();

                if (!LogTab.Load() || !MessageDat.LoadTable() || !RoomTab.Load())
                    {
                    reconfig = TRUE;
                    }

                Cron.ReadTable(WC_TWpn);
                }
            }
        else
            {
            if (!batchmode)
                {
#ifdef WINCIT
                MessageBox(NULL, "No ETC.TAB.", NULL, MB_ICONSTOP | MB_OK);
#else
                doccr();

                discardable *d;

                if ((d = readData(6)) != NULL)
                    {
                    int i;

                    for (i = 0; ((char **) d->next->aux)[i][0] != '#'; i++)
                        {
                        cPrintf(pcts, ((char **) d->next->aux)[i]);
                        doccr();
                        }

                    doccr();

                    discardData(d);
                    }
                else
                    {
                    cOutOfMemory(28);
                    }

                DeinitializeTimer();
                critical(FALSE);
#endif
                exit(1);
                }

            reconfig = TRUE;
            }
        }



    if (reconfig)
        {
        cfg.attr = 7;

#ifndef WINCIT
        pause(200);
        cls(SCROLL_SAVE);

        cCPrintf(getcfgmsg(126));
        doccr();
#endif


        if (!configcit())
            {
#ifdef WINCIT
            MessageBox(NULL, getcfgmsg(127), NULL, MB_ICONSTOP | MB_OK);
#else
            doccr();
            doccr();
            cPrintf(getcfgmsg(127));
            doccr();
#endif
            dump_cfg_messages();
            return (FALSE);
            }


#ifndef WINCIT
        setdefaultTerm(TT_ANSI);
        CurrentUser->SetWidth(80);
#endif

        Cron.ReadCronCit(WC_TWpn);
        }
    else
        {
#ifndef WINCIT
        if (!CreateScrollBackBuffer())
            {
            cPrintf(getcfgmsg(60));
            doccr();
            }
#endif


        if (readconfigcit)  // forced to read in config.cit
            {
            if (!readconfig(NULL, 1))
                {
#ifdef WINCIT
                MessageBox(NULL, getcfgmsg(129), NULL, MB_ICONSTOP | MB_OK);
#else
                doccr();
                doccr();
                cPrintf(getcfgmsg(129));
                doccr();
#endif
                dump_cfg_messages();
                return (FALSE);
                }
            }
        }

    VerifyHeap(1);

    makeBorders();
    readBordersDat();

    if (cmd_nobells)
        {
        cfg.noBells = 2;
        }

    if (cmd_nochat)
        {
        cfg.noChat = TRUE;
        }

    if (cmd_mdata != CERROR)
        {
        cfg.mdata = cmd_mdata;
        }

    if (*cfg.f6pass)
        {
        if (SameString(cfg.f6pass, getmsg(670)))
            {
            ConsoleLock.LockF6();
            }
        else
            {
            ConsoleLock.Lock();
            }
        }


#ifndef WINCIT
    if (cfg.ovrEms)
        {
        if (_OvrInitEms(0, 0, 0))
            {
            cPrintf(getcfgmsg(130));
            doccr();
            pause(200);
            }
        }

    if (cfg.ovrExt)
        {
        if (_OvrInitExt(0, 0))
            {
            cPrintf(getcfgmsg(131));
            doccr();
            pause(200);
            }
        }

    CommPort->Init();
    setscreen();
#endif

    logo(TRUE); // no go for debug version; td32 go crash crash

#ifndef WINCIT
    StatusLine.Update(WC_TWp);
#endif

    if (cfg.msgpath[(strlen(cfg.msgpath) - 1)] == '\\')
        {
        cfg.msgpath[(strlen(cfg.msgpath) - 1)] = '\0';
        }

    // move to home path
    changedir(cfg.homepath);
    char FileName[128];

    ReIndexFileInfo();  // keep fileinfo.dat nice and pretty

    // open message file
    if (!MessageDat.OpenMessageFile(cfg.msgpath))
		{
		illegal(getmsg(78), MessageDat.GetFilename());
		}

    // Then room file
    sprintf(FileName, sbs, cfg.homepath, roomDat);
    openFile(FileName, &RoomFile);

    citOpen(cfg.trapfile, CO_A, &TrapFile);
    initMenus();
    dump_cfg_messages();

    if(!read_tr_messages())
        {
        errorDisp(getmsg(172));
        }
    else
        {
#ifdef WINCIT
    trap(T_SYSOP, "", gettrmsg(37));
#else
    trap(T_SYSOP, gettrmsg(37));
#endif
        dump_tr_messages();
        }

        read_cfg_messages();            // uh-oh!

    if (!GroupData.Load())
        {
        return (FALSE);
        }

    if (!HallData.Load())
        {
        return (FALSE);
        }

    getRoomPos();

    if (cfg.accounting)
        {
        ReadGrpdataCit(WC_TWpn);
        }

    ReadExternalCit(WC_TWpn);
    ReadProtocolCit(WC_TWpn);
    ReadMdmresltCit(WC_TWpn);
    ReadCommandsCit(WC_TWpn);


#ifndef WINCIT
    ReadMCICit(FALSE);

    CurrentRoom->Load(LOBBY);
    thisRoom = LOBBY;
    checkdir();

    if (!slv_door)
        {
        CITWINDOW *w = CitWindowsMsg(NULL, getmsg(19));

        Initport();
        Initport();

        if (w)
            {
            destroyCitWindow(w, FALSE);
            }
        }
    else
        {
        CommPort->Enable();
        }

    OC.whichIO = MODEM;
    OC.setio();
#endif


    // record when we put system up
    time(&uptimestamp);

#ifndef WINCIT
    setdefaultconfig(FALSE);
    Talley->Fill();
#endif
    logo(FALSE);

    VerifyHeap(1);

    dump_cfg_messages();
    compactMemory(1);

    return (TRUE);
    }

// --------------------------------------------------------------------------
// critical(): Critical error handling.

#ifdef WINCIT
void critical(Bool) {}
#else
void critical(Bool turnOn)
    {
    static void interrupt (*dosHandler)(...);

    if (turnOn && !dosHandler)
        {
        // turn it on
        dosHandler = getvect(0x24);
        setvect(0x24, critical_error);
        }
    else if (!turnOn && dosHandler)
        {
        // turn if off
        setvect(0x24, dosHandler);
        dosHandler = NULL;
        }
    }
#endif


// --------------------------------------------------------------------------
// startUp(): Very low-level initialization.

// Borland C++ 4.00 (and later) uses exceptions for handling out of memory,
// not returning NULL like we want. This tells it to return NULL.
#if __BORLANDC__ >= 1106
    #include <new.h>
#endif

void startUp(int argc, const char *argv[])
    {
#if __BORLANDC__ >= 1106
    set_new_handler(0);
#endif

    strcpy(fullExePath, argv[0]);

#ifndef WINCIT
    fseek(stdin, 0, SEEK_CUR);  // Borland says to fseek() before setbuf()
    setbuf(stdin, NULL);
#endif

#ifndef WINCIT
#ifdef MULTI
    if (!initTaskInfo(&ti))
#else
    cfg.maxrooms = 0 ;
    if (!initFakeTaskInfo())
#endif
        {
        printf("Could not create initial task information structure.\n");
        exit(200);
        }
#endif

    tzset();

    critical(TRUE);

#ifndef WINCIT
    if (checkDataVer() != NumericVer)
        {
        printf("CTDL.DAT is version %d; version %d needed.\n", checkDataVer(), NumericVer);
        critical(FALSE);
        exit(200);
        }

    if (!read_messages())
        {
        printf("Could not read messages from CTDL.DAT\n");
        critical(FALSE);
        exit(200);
        }

    InitializeTimer();
#else
    if (checkDataVer() != NumericVer)
        {
		char bbb[100];
		sprintf(bbb, "CTDL.DAT is version %d; version %d needed.", checkDataVer(), NumericVer);

        MessageBox(NULL, bbb, NULL, MB_ICONSTOP | MB_OK);
        exit(200);
        }

    if (!read_messages())
        {
        MessageBox(NULL, "Could not read messages from CTDL.DAT.", NULL, MB_ICONSTOP | MB_OK);
        exit(200);
        }

    WNDCLASS wndclass;

    // define and register the main window class
    wndclass.style = 0; // CS_HREDRAW | CS_VREDRAW
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, "CITADEL");
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    // define and register the logo window class
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = logoWndProc;
    wndclass.lpszClassName = "Citadel Logo";
    if (!RegisterClass(&wndclass))
        {
        MessageBox(NULL, "Could not register logo window", NULL, MB_ICONSTOP | MB_OK);
        exit(200);
        }

#ifdef NAHERROR
    // define and register the error window class
    wndclass.style = CS_VREDRAW;
    wndclass.lpfnWndProc = errorWndProc;
    wndclass.lpszClassName = "Citadel Error";
    if (!RegisterClass(&wndclass))
        {
        MessageBox(NULL, "Could not register error window", NULL, MB_ICONSTOP | MB_OK);
        exit(200);
        }

    // define and register the msg window class
    wndclass.lpfnWndProc = msgWndProc;
    wndclass.lpszClassName = "Citadel Message";
    if (!RegisterClass(&wndclass))
        {
        MessageBox(NULL, "Could not register msg window", NULL, MB_ICONSTOP | MB_OK);
        exit(200);
        }
#endif
#endif

    // initialize all drivers to internal functions

#ifdef WINCIT
    // com
//  initrs          = (void (cdecl *)(int,int,int,int,int,int)) nullFunc;
//  deinitrs        = nullFunc;
//  ringstatrs      = (int (cdecl *)(void)) nullFunc;
//  carrstatrs      = (int (cdecl *)(void)) nullFunc;
//  statrs          = (int (cdecl *)(void)) nullFunc;
//  flushrs         = nullFunc;
//  getrs           = (int (cdecl *)(void)) nullFunc;
//  putrs           = (void (cdecl *)(char)) nullFunc;
//  dtrrs           = (void (cdecl *)(int)) nullFunc;
//  flushoutrs      = nullFunc;
#else
    // Start using BIOS
    charattr = bioschar;
    stringattr = biosstring;

    // com
    initrs          = InitRS;
    deinitrs        = DeInitRS;
    ringstatrs      = RingStatRS;
    carrstatrs      = CarrStatRS;
    statrs          = StatRS;
    flushrs         = FlushRS;
    getrs           = GetRS;
    putrs           = PutRS;
    dtrrs           = DtrRS;
    flushoutrs      = nullFunc;

    // ups
    initups         = nullFunc;
    deinitups       = nullFunc;
    statups         = NULL;

    // kbd
    initkbd         = nullFunc;
    deinitkbd       = nullFunc;
    statcon         = StatCON;
    getcon          = GetCON;
    sp_press        = special_pressed;

    // snd
    init_sound      = (int (cdecl *)(void)) nullFunc;
    close_sound     = (int (cdecl *)(void)) nullFunc;
    get_version     = (int (cdecl *)(void)) nullFunc;
    query_drivers   = (int (cdecl *)(void)) nullFunc;
    query_status    = (int (cdecl *)(void)) nullFunc;
    start_snd_src   = (int (cdecl *)(int, const void *)) nullFunc;
    play_sound      = (int (cdecl *)(int)) nullFunc;
    stop_sound      = (int (cdecl *)(int)) nullFunc;
    pause_sound     = (int (cdecl *)(int)) nullFunc;
    resume_sound    = (int (cdecl *)(int)) nullFunc;
    read_snd_stat   = (int (cdecl *)(int)) nullFunc;
    set_midi_map    = (int (cdecl *)(int)) nullFunc;
    get_src_vol     = (int (cdecl *)(int)) nullFunc;
    set_src_vol     = (int (cdecl *)(int, int)) nullFunc;
    set_fade_pan    = (int (cdecl *)(void *)) nullFunc;
    strt_fade_pan   = (int (cdecl *)(void)) nullFunc;
    stop_fade_pan   = (int (cdecl *)(int)) nullFunc;
    pse_fade_pan    = (int (cdecl *)(void)) nullFunc;
    res_fade_pan    = (int (cdecl *)(void)) nullFunc;
    read_fade_pan   = (int (cdecl *)(int)) nullFunc;
    get_pan_pos     = (int (cdecl *)(int)) nullFunc;
    set_pan_pos     = (int (cdecl *)(int, int)) nullFunc;
    say_ascii       = (int (cdecl *)(const char *, int)) nullFunc;
#endif

    parseArgs(argc, argv);

#ifndef WINCIT
    if (!cfg.bios)
        {
        charattr = directchar;
        stringattr = directstring;
        }
#endif

#ifndef WINCIT
    if (!cmdLine[1])
        {
        uchar *ptr = (uchar *) MK_FP(_psp, 128);

        if (ptr[0])
            {
            memcpy(cmdLine, ptr + 1, max(128, ptr[0]));

            cmdLine[ptr[0]] = 0;
            }
        }

    OC.whichIO = CONSOLE;
#endif


    if (!initCitadel())
        {
#ifndef WINCIT
        DeinitializeTimer();
#endif
        critical(FALSE);
        exit(200);
        }


#ifndef WINCIT
    ScreenSaver.Update();
#endif

    if (BoardNameHash && hash(cfg.nodeTitle) != BoardNameHash)
        {
        crashout(getmsg(681));
        }

#ifndef WINCIT
    // Set system to a known state
    OC.Echo = BOTH;
    OC.SetOutFlag(IMPERVIOUS);
    modStat = FALSE;
    OC.whichIO = CONSOLE;

    OC.setio();

    if (!(login_pw || login_user || (slv_door && cfg.forcelogin)))
        {
        CommPort->FlushInput();
        greeting();
        }

    Cron.ResetTimer();

    if (slv_net)
        {
        doccr();

        if(read_tr_messages())
            {
            cPrintf(gettrmsg(49), slv_node);
            dump_tr_messages();
            }

        if (net_callout(slv_node))
            {
            did_net(slv_node);
            }

        ExitToMsdos = TRUE;
        }

    if (slv_door) // set according to carrier
        {
        // set baud rate even if carrier not present
        if (slv_baud != PS_ERROR)
            {
            CommPort->SetSpeed(slv_baud);
            }
        else
            {
            CommPort->SetSpeed(cfg.initbaud);
            }

        for (ModemSpeedE i = MS_300; i < MS_NUM; i = (ModemSpeedE) (i +1))
            {
            if (connectbauds[i] == bauds[CommPort->GetSpeed()])
                {
                CommPort->SetModemSpeed(i);
                break;
                }
            }

        if (CommPort->HaveConnection())
            {
            CarrierJustFound();

            OC.whichIO = MODEM;
            OC.setio();
            }
        else
            {
            OC.whichIO = CONSOLE;
            OC.setio();
            }
        }

    setdefaultTerm(TT_ANSI);

    StatusLine.Toggle();        // Turns it on (starts life off).

    ScreenSaver.SetMayTurnOn(TRUE);

    time(&LastActiveTime);
    TimeoutChecking = TRUE;

    if (*cmd_script)
        {
#ifdef WINCIT
        runScript(cmd_script, NULL);
#else
        runScript(cmd_script);
#endif
        }

    doEvent(EVT_STARTUP);
#endif
    }
