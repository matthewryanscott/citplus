// --------------------------------------------------------------------------
// Citadel: Aplic.CPP
//
// Application code for Citadel

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "config.h"
#include "room.h"
#include "log.h"
#include "music.h"
#include "group.h"
#include "account.h"
#include "hall.h"
#include "cwindows.h"
#include "filecmd.h"
#include "timer.h"
#include "cfgfiles.h"
#include "events.h"
#include "miscovl.h"
#include "xmodem.h"
#include "domenu.h"
#include "statline.h"

// --------------------------------------------------------------------------
// Contents
//
// ExeAplic 	gets name of aplication and executes
// shellescape	handles the sysop '!' shell command
// wxsnd		do external download
// wxrcv		do external upload
// execDoor 	Execute a door
// apsystem 	turns off interupts and makes a system call

extern	int Zmodem_Send_Batch (const char *filepath, unsigned int baud);
extern	int Zmodem_Receive_Batch (const char *filepath, unsigned int baud);

extern ConsoleLockC ConsoleLock;

int TERMWINDOWMEMBER RunApplication(const char *CommandLine,
		const char *FileName, Bool TrapIt, Bool ChangeDir)
	{
	if (TrapIt)
		{
		doEvent(EVT_APPLIC);
		}

	label MData, MDataL, PSpeed, MSpeed, UName;
#ifdef WINCIT
    switch (CommPort->GetType())
		{
		case CT_SERIAL:
			{
			sprintf(MData, pctd, CommPort->GetPortNumber());
			break;
			}

		case CT_LOCAL:
			{
			CopyStringToBuffer(MData, getmsg(376));
			break;
			}

		case CT_TELNET:
			{
			CopyStringToBuffer(MData, getmsg(465));
			break;
			}

		default:
			{
			CopyStringToBuffer(MData, getmsg(339));
			break;
			}
		}
#else
	sprintf(MData, pctd, cfg.mdata);
#endif

	CopyStringToBuffer(MDataL, onConsole ? getmsg(376) : MData);
	sprintf(PSpeed, pctld, bauds[CommPort->GetSpeed()]);
	sprintf(MSpeed, pctld, connectbauds[CommPort->GetModemSpeed()]);
	CurrentUser->GetName(UName, sizeof(UName));
	stripansi(UName);

	char ExpandedCommandLine[256];
	sformat(ExpandedCommandLine, CommandLine,
			FileName ? getmsg(325) : getmsg(324), MData, PSpeed, MSpeed,
            cfg.aplpath, MDataL, UName, &LocalTempPath, FileName, 0);

	if (ChangeDir && !changedir(cfg.aplpath))
		{
		CRmPrintfCR(getmsg(154));
		return (CERROR);
		}
	else
		{
		const int SpawnResult = apsystem(ExpandedCommandLine, TrapIt);

		changedir(cfg.homepath);

		DebugOut(pcts, ExpandedCommandLine);

		return (SpawnResult);
		}
	}


// --------------------------------------------------------------------------
// ExeAplic(): Executes the current room's application.
//
// Return value:
//	negative: Error running application
//	non-negative: Application's result code

int TERMWINDOWMEMBER ExeAplic(void)
	{
	SetDoWhat(ENTERAPP);

	if (!RoomTab[thisRoom].IsApplication())
		{
		CRmPrintfCR(getmsg(151), cfg.Uroom_nym);
		changedir(cfg.homepath);
		return (-1);
		}

	if (!changedir(cfg.aplpath))
		{
		CRmPrintfCR(getmsg(154));
		changedir(cfg.homepath);
		return (-1);
		}

	doEvent(EVT_APPLIC);

	char stuff[100];
	char comm[5];

	sprintf(comm, getmsg(323), cfg.mdata, ns, ns);

	label Buffer, Buffer2;
	sprintf(stuff, getmsg(314),
			CurrentRoom->GetApplication(Buffer2, sizeof(Buffer2)),
			onConsole ? getmsg(376) : comm,
			onConsole ? 2400l : bauds[CommPort->GetSpeed()],
			CurrentUser->IsSysop(),
			deansi(CurrentUser->GetName(Buffer, sizeof(Buffer))));

	const int SpawnResult = apsystem(stuff, TRUE);
	changedir(cfg.homepath);

	return (SpawnResult);
	}


// --------------------------------------------------------------------------
// shellescape(): Handles the sysop '!' shell command.
//
// Input:
//	Bool super: TRUE if super-shell, FALSE if not

void TERMWINDOWMEMBER shellescape(Bool super)
	{
	char command[80];

	doEvent(EVT_APPLIC);

	changedir(RoomTab[thisRoom].IsMsDOSdir() ?
			CurrentRoom->GetDirectory(command, sizeof(command)) :
			cfg.homepath);

	sprintf(command, getmsg(313), super ? getmsg(312) : ns,
			getenv(getmsg(502)));

	apsystem(command, TRUE);

	StatusLine.Update(WC_TWp);

	changedir(cfg.homepath);
	}


// --------------------------------------------------------------------------
// wxsnd(): Do external download.
//
// Input:
//	const char *path: The path to send files from
//	const char *file: The file or files to send
//	const protocols *theProt: Which protocol to use
//	int type: 1 if this is a queued transfer; 2 if CheckTransfer; 0 if normal

void TERMWINDOWMEMBER wxsnd(const char *path, const char *file,
		const protocols *theProt, int type)
	{
	if (!changedir(path))
		{
		return;
		}

	const char *SendCmd =
			(type == 2) ? theProt->CheckMethod :
			(type == 1) ? theProt->respDown : theProt->snd;

	if (*SendCmd == '[')
		{
		InternalProtocols prot;

		if (SameString(SendCmd, getmsg(531)))
			{
			prot = IP_XMODEM;
			}
		else if (SameString(SendCmd, getmsg(532)))
			{
			prot = IP_CRCXMODEM;
			}
		else if (SameString(SendCmd, getmsg(533)))
			{
			prot = IP_ONEKXMODEM;
			}
		else if (SameString(SendCmd, getmsg(534)))
			{
			prot = IP_ZMODEM;
			}
		else
			{
			CRmPrintf(getmsg(535), SendCmd);
			return;
			}

		if (prot == IP_ZMODEM)
			{
			Zmodem_Send_Batch(file,
					(uint) connectbauds[CommPort->GetModemSpeed()]);
			}
		else
			{
#ifdef WINCIT
			XmodemC Xmodem(this);
#else
			XmodemC Xmodem;
#endif

			Xmodem.Send(file, prot);
			}
		}
	else
		{
		RunApplication(SendCmd, file, TRUE, FALSE);
		}

	changedir(path);
	}


// --------------------------------------------------------------------------
// wxrcv(): do external upload.
//
// Input:
//	const char *path: Where to upload to
//	const char *file: File to receive (not used if batch transfer)
//	const protocols *theProt: File transfer protocol to use

void TERMWINDOWMEMBER wxrcv(const char *path, const char *file,
		const protocols *theProt)
	{
	if (!changedir(path))
		{
		return;
		}

	if (theProt->rcv[0] == '[')
		{
		InternalProtocols prot;

		if (SameString(theProt->rcv, getmsg(531)))
			{
			prot = IP_XMODEM;
			}
		else if (SameString(theProt->rcv, getmsg(532)))
			{
			prot = IP_CRCXMODEM;
			}
		else if (SameString(theProt->rcv, getmsg(533)))
			{
			prot = IP_ONEKXMODEM;
			}
		else if (SameString(theProt->rcv, getmsg(534)))
			{
			prot = IP_ZMODEM;
			}
		else
			{
			CRmPrintf(getmsg(535), theProt->rcv);
			return;
			}

		if (prot == IP_ZMODEM)
			{
			Zmodem_Receive_Batch(ns,
					(uint) connectbauds[CommPort->GetModemSpeed()]);
			}
		else
			{
#ifdef WINCIT
			XmodemC Xmodem(this);
#else
			XmodemC Xmodem;
#endif

			Xmodem.Receive(file, prot);
			}
		}
	else
		{
		RunApplication(theProt->rcv, file, TRUE, FALSE);
		}

	changedir(path);
	}


// --------------------------------------------------------------------------
// execDoor(): Execute a door
//
// Input:
//	char c: The menu key used
//	int where: Where menu key was pressed:
//		0: .ED command
//		1: x at room prompt
//		2: .x at room prompt
//
// Return value:
//	TRUE: Door found and executed
//	FALSE: Door not found

Bool TERMWINDOWMEMBER execDoor(char c, int where)
	{
	doors *theDoor;

	if (c == '?')
		{
		return (FALSE);
		}

	for (theDoor = extDoorList; theDoor; theDoor = (doors *) getNextLL(theDoor))
		{
		if (tolower(c) == tolower(theDoor->name[0]))
			{
			if ((!theDoor->SYSOP	|| CurrentUser->IsSysop()) &&
					(!theDoor->AIDE || CurrentUser->IsAide()) &&
					(!theDoor->CON	|| (onConsole && !ConsoleLock.IsLocked())) &&
					(!theDoor->DIR	|| RoomTab[thisRoom].IsMsDOSdir()) &&
					(!where 		|| (theDoor->WHERE & where)))
				{
				if (theDoor->group[0])
					{
					g_slot GroupSlot = FindGroupByName(theDoor->group);

					if (GroupSlot != CERROR)
						{
						if (CurrentUser->IsInGroup(GroupSlot))
							{
							break;
							}
						}
					}
				else
					{
					break;
					}
				}
			}
		}

	if (theDoor)
		{
		mPrintfCR(pcts, theDoor->name);
		doCR();

		RunApplication(theDoor->cmd, NULL, TRUE, TRUE);

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}


struct RunFlagsS
	{
	Bool Clear, Super, Batch, DoorFiles, Script, Wait;

	RunFlagsS(void)
		{
		Clear = TRUE;
		Super = FALSE;
		Batch = FALSE;
		DoorFiles = TRUE;
		Script = FALSE;
		Wait = TRUE;
		}

	const char *Check(const char *CL);
	};


const char *RunFlagsS::Check(const char *CL)
	{
	while (*CL == '!' || *CL == '@' || *CL == '$' || *CL == '?' ||
			*CL == '*' || *CL == '&')
		{
		if (*CL == '!')
			{
			Super = TRUE;
			}
		else if (*CL == '@')
			{
			Clear = FALSE;
			}
		else if (*CL == '$')
			{
			Batch = TRUE;
			}
		else if (*CL == '?')
			{
			DoorFiles = FALSE;
			}
		else if (*CL == '*')
			{
			Script = TRUE;
			}
		else if (*CL == '&')
			{
			Wait = FALSE;
			}

		CL++;
		}

	return (CL);
	}

#ifdef WINCIT
void createProperCommandLine(char *Result, const char *Input)
	{
	// We don't want to run MS-DOS applications directly from Citadel,
	// but pass them through the helper application CITRUN.EXE. Running a
	// Windows application through CITRUN.EXE is harmless, just a little
	// ugly. Running an MS-DOS application without CITRUN.EXE ties up
	// Citadel without input from a user at the system console. We
	// therefore always run through CITRUN.EXE unless we can verify that
	// we are running a Windows application.

	Bool RunAsMSDOS = TRUE;

	char scratch[256];
    strcpy(scratch, Input);
	char *words[256];
	if (parse_it(words, scratch))
		{
errorDisp("parse_it: %s", words[0]);
		char Scratch2[256];
		char *Foo;

		if (SearchPath(NULL, words[0], getmsg(691), sizeof(Scratch2),
				Scratch2, &Foo))
			{
errorDisp("SearchPath: %s", Scratch2);
			DWORD Type;

			if (GetBinaryType(Scratch2, &Type))
				{
errorDisp("GetBinaryType: %d", Type);
				if (!(Type & (SCS_DOS_BINARY | SCS_PIF_BINARY)))
					{
					// We have verfied that it is not an MS-DOS
					// application; we can run it directly
					RunAsMSDOS = FALSE;
					}
				}
			}
		}

	// hard-coding command.com is evil naughty bad. isn't it?
	sprintf(Result, getmsg(636), RunAsMSDOS ? getmsg(657) : "command.com /c" , Input);
	}
#endif

int QuickRunApp(const char *Cmd)
	{
	RunFlagsS RF;

	Cmd = RF.Check(Cmd);

	char scratch[256];
    if(!RF.Script)
        {
	if (RF.Batch)
		{
		sprintf(scratch, getmsg(536), getenv(getmsg(502)), Cmd);
		}
	else
		{
#ifdef WINCIT
		createProperCommandLine(scratch, Cmd);
#else
		CopyStringToBuffer(scratch, Cmd);
#endif
		}
        }
    else
        {
		label ScriptName;

        CopyStringToBuffer(ScriptName, Cmd);

        if (strchr(ScriptName, ' '))
			{
            *strchr(ScriptName, ' ') = 0;
			}

#ifdef WINCIT
        return (runScript(ScriptName, NULL));
#else
		return (runScript(ScriptName));
#endif
        }

	int RetVal = -2;

#ifdef WINCIT
	STARTUPINFO Startup;

	Startup.cb = sizeof(Startup);
	Startup.lpReserved = NULL;
	Startup.lpDesktop = NULL;
	Startup.lpTitle = NULL;
	// dw[XY], dw[XY]Size, dw[XY]CountChars, dwFillAttribute ignored
	Startup.dwFlags = 0;
	// wShowWindows ignored
	Startup.cbReserved2 = 0;
	Startup.lpReserved2 = NULL;
	// hStdInput, hStdOutput, hStdError ignored

	PROCESS_INFORMATION ProcessInfo;

	if (CreateProcess(NULL, scratch, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
			NULL, NULL, &Startup, &ProcessInfo))
		{
		CloseHandle(ProcessInfo.hThread);

		if (RF.Wait)
			{
			WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

			if (!GetExitCodeProcess(ProcessInfo.hProcess,
					(ulong*) (&RetVal)) || RetVal < 0)
				{
				RetVal = 0;
				}
			}
		else
			{
			RetVal = 0;
			}

		CloseHandle(ProcessInfo.hProcess);
		}
#else
	char *words[256];
	const int count = parse_it(words, scratch);
	words[count] = NULL;

	if (count)
		{
		changedir(cfg.aplpath);
		RetVal = spawnvp(P_WAIT, words[0], words);
		changedir(cfg.homepath);
		}
#endif

	return (RetVal);
	}

// --------------------------------------------------------------------------
// apsystem(): Actually executes other applications.
//
// Input:
//	const char *stuff: Command line to execute, with flags (! for super-shell,
//		etc.)
//	Bool trapit: TRUE to trap running, FALSE to not.
//
// Return value:
//	negative: Error running application.
//	non-negative: Application's return code

int TERMWINDOWMEMBER apsystem(const char *stuff, Bool trapit)
	{
	VerifyHeap();

	RunFlagsS RF;

	DebugOut(pcts, stuff);

	stuff = RF.Check(stuff);

	if (RF.Script)
		{
		label ScriptName;

		CopyStringToBuffer(ScriptName, stuff);

		if (strchr(ScriptName, ' '))
			{
			*strchr(ScriptName, ' ') = 0;
			}

#ifdef WINCIT
		return (runScript(ScriptName, this));
#else
		return (runScript(ScriptName));
#endif
		}

	if (RF.DoorFiles)
		{
		writeAplFile();
		}

	if (!RF.Super)
		{
		char OldPath[128];

		getfpath(OldPath);

		compactMemory(2);

		changedir(OldPath);
		}

#ifndef WINCIT
	if (farcoreleft() < cfg.memfree)
		{
		RF.Super = TRUE;
		}
#endif

	if (DoWhat != NETWORKING && RF.Super && cfg.SwapNote)
		{
		CRmPrintfCR(getmsg(162));
		}

#ifndef WINCIT
	if (CommPort->IsDisabled())
		{
		CommPort->DropDtr();
		}

	CommPort->Deinit();

	(*deinitups)();
	(*deinitkbd)();
	(*close_sound)();

	deinitMouseHandler();

	deinit_internal_sound();

    MessageDat.CloseMessageFile();

	if (OC.PrintFile)
		{
		fclose(OC.PrintFile);
		OC.PrintFile = NULL;
		}

	if (RoomFile)
		{
		fclose(RoomFile);
		RoomFile = NULL;
		}

	if (journalfl)
		{
		fclose(journalfl);
		journalfl = NULL;
		}

	int row = logiRow, col = logiCol;
	if (RF.Clear)
		{
		save_screen();
		outPhys(TRUE);
		cls(SCROLL_NOSAVE);
		outPhys(FALSE);
		}
#endif

	outPhys(TRUE);

	if (loggedIn && RF.Clear)
		{
		label L;
		CopyStringToBuffer(L, cfg.Uuser_nym);
		stripansi(L);

		label Buffer;
		cPrintf(getmsg(163), L, deansi(CurrentUser->GetName(Buffer,
				sizeof(Buffer))));
		doccr();
		}

	if (SameString(stuff, getenv(getmsg(502))))
		{
		cPrintf(getmsg(164), programName);
		}

	outPhys(FALSE);

	char scratch[256];
	if (RF.Batch)
		{
		sprintf(scratch, getmsg(536), getenv(getmsg(502)), stuff);
		}
	else
		{
#ifdef WINCIT
		createProperCommandLine(scratch, stuff);
#else
		CopyStringToBuffer(scratch, stuff);
#endif
		}

	curson();

	if (trapit)
		{
#ifdef WINCIT
        trap(T_APPLIC, WindowCaption, getmsg(401), stuff);
#else
        trap(T_APPLIC, getmsg(401), stuff);
#endif
		}

	int SpawnResult;

#ifdef WINCIT
	STARTUPINFO Startup;

	Startup.cb = sizeof(Startup);
	Startup.lpReserved = NULL;
	Startup.lpDesktop = NULL;
	Startup.lpTitle = NULL;
	// dw[XY], dw[XY]Size, dw[XY]CountChars, dwFillAttribute ignored
	Startup.dwFlags = 0;
	// wShowWindows ignored
	Startup.cbReserved2 = 0;
	Startup.lpReserved2 = NULL;
	// hStdInput, hStdOutput, hStdError ignored

	PROCESS_INFORMATION ProcessInfo;

#ifndef HARRY
	CommPort->Pause();
#endif

	if (CreateProcess(NULL, scratch, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
   			NULL, NULL, &Startup, &ProcessInfo))
		{
		CloseHandle(ProcessInfo.hThread);

		if (RF.Wait)
			{
			WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

			if (!GetExitCodeProcess(ProcessInfo.hProcess,
					(ulong*) (&SpawnResult)) || SpawnResult < 0)
				{
				SpawnResult = 0;
				}
			}
		else
			{
			SpawnResult = 0;
			}

		CloseHandle(ProcessInfo.hProcess);
		}
	else
		{
		SpawnResult = -1;
		}
#ifndef HARRY
	CommPort->Resume();
#endif
#else
	critical(FALSE);
	DeinitializeTimer();

	fclose(TrapFile);
	char *words[256];

	const int count = parse_it(words, scratch);
	words[count] = NULL;

	if (RF.Super)
		{
		SpawnResult = spawnvpeo(LocalTempPath, words[0], (const char **) words,
				(const char **) environ);
		}
	else
		{
		SpawnResult = spawnvp(P_WAIT, words[0], words);

		if (SpawnResult == -1)
			{
			VerifyHeap();

			if (errno == ENOMEM)
				{
				if (DoWhat != NETWORKING)
					{
					CRmPrintfCR(getmsg(18));
					}

				SpawnResult = spawnvpeo(LocalTempPath, words[0],
						(const char **) words, (const char **) environ);
				}
			}
		}
#endif

#ifndef WINCIT
	citOpen(cfg.trapfile, CO_AP, &TrapFile);

	InitializeTimer();
	critical(TRUE);
#endif

	if (!ScreenSaver.IsOn())
		{
		ScreenSaver.Update();
		}

	time(&LastActiveTime);

	if (trapit)
		{
#ifdef WINCIT
        trap(T_APPLIC, WindowCaption, getmsg(402));
#else
        trap(T_APPLIC, getmsg(402));
#endif
		}

#ifndef WINCIT
	// restore the screen if applicaton changes modes.
	uint c, r;
	getScreenSize(&c, &r);
	if (c != conCols || r != conRows)
		{
		setscreen();
		}
#endif

	// back to blank screen saver mode
	if (ScreenSaver.IsOn() || StatusLine.IsFullScreen() || allWindows)
		{
		outPhys(TRUE);
		cls(SCROLL_NOSAVE);
		outPhys(FALSE);

		if (ScreenSaver.IsOn() && !cfg.really_really_fucking_stupid)
			{
			cursoff();
			}

		updateStatus = TRUE;

#ifndef WINCIT
		if (allWindows)
			{
			RedrawAll();
			}
#endif
		}

#ifndef WINCIT
	if (RF.Clear)
		{
		if (!StatusLine.IsFullScreen())
			{
			restore_screen();
			}

		position(row, col);
		}
#endif

	setborder(cfg.battr); // not quite sure about this

	// move things around in memory to maximize free space here.
	compactMemory(2);

#ifndef WINCIT
	CommPort->Init();

	(*initups)();
	(*initkbd)();
	(*init_sound)();
	(*dtrrs)(1);

	initMouseHandler();

	if (!hideCounter)
		{
		showMouse();
		hideCounter = 0;
		}	

	init_internal_sound();

	StatusLine.Update(WC_TWp);

	sprintf(scratch, sbs, cfg.msgpath, msgDat);
    if (!MessageDat.OpenMessageFile(cfg.msgpath))
        {
        printf("error opening msg.dat");
        }

	sprintf(scratch, sbs, cfg.homepath, roomDat);
	openFile(scratch, &RoomFile);

	if (CommPort->IsDisabled())
		{
		CommPort->DropDtr();
		}
#endif

	initMenus();	// may have changed when shelled

	if (((cfg.offhook == 1) || (cfg.offhook && DoWhat != NETWORKING)) &&
			!CommPort->HaveConnection() && loggedIn)
		{
		offhook();
		}

	readAplFile();

	if (OC.Printing)
		{
		if (!citOpen(OC.PrintfileName, CO_A, &OC.PrintFile))
			{
			OC.Printing = FALSE;

			if (!ScreenSaver.IsOn() && cfg.VerboseConsole)
				{
				CitWindowsNote(NULL, getmsg(32));
				}
			}
		}

	if (!ScreenSaver.IsOn() && onConsole)
		{
		ScreenSaver.Update();
		}

	VerifyHeap();

	return (SpawnResult);
	}


void TERMWINDOWMEMBER EnterApplication(void)
	{
	doCR();

	if (!loggedIn)
		{
		if (LockMenu(MENU_ENTER))
			{
			CRmPrintfCR(getmenumsg(MENU_ENTER, 17));
			UnlockMenu(MENU_ENTER);
			}
		}
	else
		{
		ExeAplic();
		}
	}


void TERMWINDOWMEMBER EnterDoor(void)
	{
	if (!LockMenu(MENU_ENTER))
		{
		return;
		}

	SetDoWhat(ENTERDOOR);

	const int ich = iCharNE();

	if (!execDoor((char) ich, 0))
		{
		Bool NeedMenu = !CurrentUser->IsExpert();

		if (ich == '?')
			{
			oChar('?');
			NeedMenu = TRUE;
			}
		else
			{
			oChar((char) ich);
			mPrintf(sqst);
			}

		if (NeedMenu)
			{
			if (getMnuOff(M_DOORS, NULL) > 0)
				{
				showMenu(M_DOORS);
				}
			else
				{
				doCR(2);

				doors *theDoor;

				for (theDoor = extDoorList; theDoor; 
						theDoor = (doors *) getNextLL(theDoor))
					{
					if ((!theDoor->SYSOP || CurrentUser->IsSysop()) &&
							(!theDoor->AIDE || CurrentUser->IsAide()) &&
							(!theDoor->CON || (onConsole && !ConsoleLock.IsLocked())) &&
							(!theDoor->DIR || RoomTab[thisRoom].IsMsDOSdir()))
						{
						Bool ShowOnMenu = TRUE;

						if (theDoor->group[0])
							{
							g_slot GroupSlot = 
									FindGroupByName(theDoor->group);

							if (GroupSlot != CERROR)
								{
								if (!CurrentUser->IsInGroup(GroupSlot))
									{
									ShowOnMenu = FALSE;
									}
								}
							}

						if (ShowOnMenu)
							{
							mPrintfCR(getmenumsg(MENU_ENTER, 19),
									theDoor->name[0],
									theDoor->name);
							}
						}
					}

				mPrintfCR(getmenumsg(MENU_ENTER, 20));
				}
			}
		else
			{
			doCR();
			}
		}

	UnlockMenu(MENU_ENTER);
	}
