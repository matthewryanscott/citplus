// --------------------------------------------------------------------------
// Citadel: CTDL.CPP
//
// The root of all evil.

#define MAIN
#define EXTERN
#pragma hdrstop

#include "ctdl.h"

#include "config.h"
#include "log.h"
#include "account.h"
#include "helpfile.h"
#include "mdmreslt.h"
#include "cfgfiles.h"
#include "cwindows.h"
#include "room.h"
#include "infofile.h"
#include "miscovl.h"
#include "filecmd.h"
#include "term.h"
#include "extmsg.h"
#include "net6969.h"
#include "termwndw.h"
#include "tallybuf.h"

//#define DEMO

#ifdef DEMO
    #define DEMOWARN (25*60)
    #define DEMOTIMEOUT (30*60)
#endif

// --------------------------------------------------------------------------
// Contents
//
// doRegular()      fanout for above commands
// getCommand()     prints prompt and gets command char
// main()           has the central menu code
// doAd()           shows an ad/blurb cfg.ad_chance % of the time


// --------------------------------------------------------------------------
// Static data and defines
#ifdef WINCIT

    #define ID_TIMER    1                   // For blink technology
    static Bool blinkstate;                 // More blink technology

    int NoComPortC::TotalCounter;
    int NoComPortC::ActiveCounter;

#else

    uint cdecl _stklen = 1024*12;           // set up 12K of stack

    #ifndef FLOPPY
        #if VERSION == RELEASE
            uint cdecl _ovrbuffer = 96*64;  // set up 96K for overlays
        #else
            uint cdecl _ovrbuffer = 96*64;  // set up 96K for overlays
        #endif
    #endif

    #define PURGE1VALUE 100l*1024l          // Do a minor purge at 100K
    #define PURGE2VALUE 80l*1024l           // Try harder at 80K
    #define PURGE3VALUE 60l*1024l           // Try harder still at 60K
    #define WARNVALUE   40l*1024l           // Start warning at 40K
    #define DANGERVALUE 20l*1024l           // And get worried at 20K


    #ifdef MSC
        int _far _nullcheck(void);          // Undocumented MSC 6 function
    #endif

#endif

// --------------------------------------------------------------------------
// main(): Call init, general system loop, then shutdown.
//
// Input:
//  argc, argv: Standard C RTL stuff.
//
// Return value:
//  This function never returns.  It ends by calling exitcitadel(), which
//  uses exit() to exit Citadel.

#ifndef WINCIT

#ifdef __BORLANDC__ // Gotta keep that Golden Spud happy!
void cdecl main(int argc, const char *argv[])
#else
int cdecl main(int argc, const char *argv[])
#endif
    {
    startUp(argc, argv);

    MainCitadelLoop();

    exitcitadel();
    }

#else

#include "telnetd.h"
#include "winser.h"
#include "libovl.h"
#include "carrier.h"

void _USERENTRY ListenForSerialConnection(void *P)
    {
    SerialPortC *Port = (SerialPortC *) P;
    time_t LastInit = 0;

    // do stuff
    for (;;)
        {
        Sleep(1000);

        if (Port->Own())
            {
            // Initialize the port every 5 minutes.
            if (time(NULL) > LastInit + 60 * 5)
                {
                Port->SetSpeed(cfg.initbaud);

                Port->OutString(cfg.modsetup);
                Port->OutString(br);

                Sleep(1000);

                LastInit = time(NULL);
                }

            if (CheckForNewCarrier(Port))
                {
                Bool KeepWindow = ViewingTerm;

                TurnOnTermWindow();

                if (ViewingTerm)
                    {
                    TermWindowC *TW = new TermWindowC(Port, ConnSeq.Next());

                    if (TW && TW->IsGood())
                        {
                        if (!KeepWindow)    // Not quite a good var name...
                            {
                            TermWindowCollection.SetFocus(TW);
                            }

                        TW->CarrierJustFound();

                        HANDLE t = (HANDLE) _beginthread(NewTermWindowThreadKeepComm, 1024 * 20, TW);

                        if (t != (HANDLE) -1)
                            {
                            TW->hThread = t;

                            // We don't get to start looking for carrier again until this term window is done with the
                            // port, of course.
                            WaitForSingleObject(t, INFINITE);

                            // But when we're ready, force a modem reset
                            LastInit = 0;
                            }

                        KeepWindow = TRUE;
                        }
                    else
                        {
                        delete TW;
                        }

                    if (!KeepWindow)
                        {
                        TurnOffTermWindow();
                        }
                    }
                }

            Port->Disown();
            }
        }
    }


void _USERENTRY RunStandAloneScript(void *)
    {
    char FileName[512];
    *FileName = 0;

    OPENFILENAME ofn;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = MainDlgH;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = "Citadel Script Files\0*.CSF\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = FileName;
    ofn.nMaxFile = sizeof(FileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = cfg.ScriptPath;
    ofn.lpstrTitle = "Run Script";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = "csf";
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (GetOpenFileName(&ofn))
        {
        runScript(FileName, NULL, NULL);
        }
    }


void _USERENTRY NewTermWindowThreadKeepComm(void *TW)
    {
    ((TermWindowC *) TW)->MainCitadelLoop();
    delete (TermWindowC *) TW;
    }


void _USERENTRY NewTermWindowThread(void *TW)
    {
    ((TermWindowC *) TW)->MainCitadelLoop();
    delete ((TermWindowC *) TW)->CommPort;
    delete (TermWindowC *) TW;
    }

void _USERENTRY CronThread(void *)
    {
    for (;;)
        {
        Sleep(30000);

        TermWindowCollection.PingAll();

        WaitForSingleObject(CronMutex, INFINITE);

        Cron.Do(CRON_TIMEOUT);

        ReleaseMutex(CronMutex);
        }
    }

#define MASK_FOREGROUND    7
#define MASK_INTENSITY     8
#define MASK_BACKGROUND    112
#define MASK_BLINK         128

// taken from msvc console sample
void resizeConBufAndWindow(HANDLE hConsole, SHORT xSize, SHORT ySize)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
//  BOOL bSuccess;
  SMALL_RECT srWindowRect; /* hold the new console size */
  COORD coordScreen;

  /*bSuccess = */GetConsoleScreenBufferInfo(hConsole, &csbi);
  /* get the largest size we can size the console window to */
  coordScreen = GetLargestConsoleWindowSize(hConsole);
  /* define the new console window size and scroll position */
  srWindowRect.Right = (SHORT) (min(xSize, coordScreen.X) - 1);
  srWindowRect.Bottom = (SHORT) (min(ySize, coordScreen.Y) - 1);
  srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
  /* define the new console buffer size */
  coordScreen.X = xSize;
  coordScreen.Y = ySize;
  /* if the current buffer is larger than what we want, resize the */
  /* console window first, then the buffer */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
    {
    /*bSuccess = */SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
    /*bSuccess = */SetConsoleScreenBufferSize(hConsole, coordScreen);
    }
  /* if the current buffer is smaller than what we want, resize the */
  /* buffer first, then the console window */
  if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)
    {
    /*bSuccess = */SetConsoleScreenBufferSize(hConsole, coordScreen);
    /*bSuccess = */SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
    }
  /* if the current buffer *is* the size we want, don't do anything! */
  return;
}

void _USERENTRY ScreenUpdateThread(void *)
    {
    ScreenUpdateMutex = CreateMutex(NULL, FALSE, "WincitMutex:SUpdate");

    if (ScreenUpdateMutex == NULL)
        {
        return;
        }

//  TermWindowC *LastWindow = NULL;
    static COORD CursorPos;
    CursorPos.X = -1;
    CursorPos.Y = -1;

    Bool lastblinkstate = FALSE;
    Bool Alloced = FALSE;

    CHAR_INFO *screenbufcopy = new CHAR_INFO[cfg.ConsoleCols * cfg.ConsoleRows];

    int x, y, i;

    for (;;)
        {
        Sleep(cfg.ScreenUpdatePause);

        if (Alloced != ViewingTerm)
            {
            if (Alloced)
                {
                FreeConsole();
                Alloced = FALSE;
                hConsoleInput = NULL;
                hConsoleOutput = NULL;
                TermWindowCollection.SetFocus(NULL);
                }
            else
                {
                if (AllocConsole())
                    {
                    Alloced = TRUE;

                    /* if allocconsole() does not setup the std handles we do it here */
                    hConsoleInput = CreateFile
                     (
                     "CONIN$",
                     GENERIC_READ|GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL
                     );
                    SetStdHandle(STD_INPUT_HANDLE, hConsoleInput );
                    hConsoleOutput = CreateFile
                     (
                     "CONOUT$",
                     GENERIC_READ|GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL
                     );
                    SetStdHandle(STD_OUTPUT_HANDLE, hConsoleOutput );
                    SetStdHandle(STD_ERROR_HANDLE, hConsoleOutput );

                    if (hConsoleInput == INVALID_HANDLE_VALUE || hConsoleOutput == INVALID_HANDLE_VALUE)
                        {
                        // Should this call CloseHandle? My docs don't specify...
                        hConsoleInput = INVALID_HANDLE_VALUE;
                        hConsoleOutput = INVALID_HANDLE_VALUE;
                        FreeConsole();
                        Alloced = FALSE;
                        }
                    else
                        {
                        resizeConBufAndWindow(hConsoleOutput, cfg.ConsoleCols, cfg.ConsoleRows);
                        }
                    }
                else
                    {
                    MessageBox(NULL, "Could not create console", NULL, MB_ICONSTOP | MB_OK);
                    }
                }
            }

        PCHAR_INFO Screen = TermWindowCollection.GetFocusedScreen();

        if (Alloced && Screen)
            {
            if (blinkstate != lastblinkstate || ScreenUpdateRect.Left < cfg.ConsoleCols)
                {
                WaitForSingleObject(ScreenUpdateMutex, INFINITE);

                CONSOLE_SCREEN_BUFFER_INFO sbi;

                if (GetConsoleScreenBufferInfo(hConsoleOutput, &sbi))
                    {
                    COORD BufSize;
                    BufSize.X = cfg.ConsoleCols;
                    BufSize.Y = cfg.ConsoleRows;

                    if (cfg.BlinkPause && screenbufcopy)
                        {
                        memcpy(screenbufcopy, Screen, cfg.ConsoleCols * cfg.ConsoleRows * 4);

                        for (y= 0, i = 0; y < cfg.ConsoleRows; y++)
                            {
                            for (x = 0; x < cfg.ConsoleCols; x++, i++)
                                {
                                if (screenbufcopy[i].Attributes & MASK_BLINK)
                                    {
                                    // Blinking characters disappear during blink state
                                    if (blinkstate)
                                        {
                                        // clear foreground
                                        screenbufcopy[i].Attributes &= ~MASK_FOREGROUND;

                                        // set foreground equal to background
                                        screenbufcopy[i].Attributes |= (WORD)((screenbufcopy[i].Attributes & MASK_BACKGROUND) >> 4);

                                        // Turn off intensity
                                        screenbufcopy[i].Attributes &= ~MASK_INTENSITY;
                                        }

                                    // Turn off blink
                                    screenbufcopy[i].Attributes &= ~MASK_BLINK;

                                    if (ScreenUpdateRect.Left > x)
                                        {
                                        ScreenUpdateRect.Left = (SHORT) x;
                                        }
                                    if (ScreenUpdateRect.Right < x)
                                        {
                                        ScreenUpdateRect.Right = (SHORT) x;
                                        }
                                    if (ScreenUpdateRect.Top > y)
                                        {
                                        ScreenUpdateRect.Top = (SHORT) y;
                                        }
                                    if (ScreenUpdateRect.Bottom < y)
                                        {
                                        ScreenUpdateRect.Bottom = (SHORT) y;
                                        }
                                    }
                                }
                            }

                        lastblinkstate = blinkstate;
                        }

                    COORD WriteFrom;
                    WriteFrom.X = ScreenUpdateRect.Left;
                    WriteFrom.Y = ScreenUpdateRect.Top;

                    if (ScreenUpdateRect.Left < cfg.ConsoleCols)
                        {
                        WriteConsoleOutput(hConsoleOutput, ((cfg.BlinkPause && screenbufcopy) ? screenbufcopy : Screen), BufSize, WriteFrom, &ScreenUpdateRect);
                        }

                    ScreenUpdateRect.Left = cfg.ConsoleCols;
                    ScreenUpdateRect.Top = cfg.ConsoleRows;
                    ScreenUpdateRect.Bottom = -1;
                    ScreenUpdateRect.Right = -1;

                    }

                ReleaseMutex(ScreenUpdateMutex);
                }

            if (CursorPos.X != (SHORT) TermWindowCollection.FocusedCursorCol() || CursorPos.Y != (SHORT) TermWindowCollection.FocusedCursorRow())
                {
                CursorPos.X = (SHORT) TermWindowCollection.FocusedCursorCol();
                CursorPos.Y = (SHORT) TermWindowCollection.FocusedCursorRow();
                SetConsoleCursorPosition(hConsoleOutput, CursorPos);
                }
            }
        }

    // code never gets here.
    //delete [] screenbufcopy;
    }


void TurnOffTermWindow(void)
    {
    ViewingTerm = FALSE;
    }

void TurnOnTermWindow(void)
    {
    ViewingTerm = TRUE;
    }

void TermWindowCollectionC::SetFocus(TermWindowC *NewFocus)
    {
    Lock();

    Focus = NewFocus;

    WaitForSingleObject(ScreenUpdateMutex, INFINITE);
    ScreenUpdateRect.Left = 0;
    ScreenUpdateRect.Top = 0;
    ScreenUpdateRect.Bottom = cfg.ConsoleRows - 1;
    ScreenUpdateRect.Right = cfg.ConsoleCols - 1;
    ReleaseMutex(ScreenUpdateMutex);

    if (NewFocus != NULL)
        {
        NewFocus->SetTitle();
        }

    Unlock();
    }

HWND hAboutDlg;
BOOL CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    switch (uMsg)
        {
        case WM_INITDIALOG:
            {
            return (1);
            }

        case WM_COMMAND:
            {
            switch (LOWORD(wParam)) // ID
                {
                case 1: // Ok
                    {
                    EndDialog(hwndDlg, 0);
                    hAboutDlg = NULL;
                    return (1);
                    }
                }
            }
        }

    return (0);
    }


void TermWindowCollectionC::SetFocusInt(int NewFocus)
    {
    Lock();

    TermWindowListS *cur = (TermWindowListS *) getLLNum(List, NewFocus + 1);

    if (cur && cur->T)
        {
        SetFocus(cur->T);
        }

    Unlock();
    }


void TermWindowCollectionC::KillAll(void)
    {
    Lock();

    for (TermWindowListS *cur = List; cur; cur = (TermWindowListS *) getNextLL(cur))
        {
        if (cur->T)
            {
            cur->T->ExitToMsdos = TRUE;
            }
        }

    Unlock();
    }

void TermWindowCollectionC::SetForKill(int ToKill)
    {
    Lock();

    TermWindowListS *cur = (TermWindowListS *) getLLNum(List, ToKill + 1);

    if (cur && cur->T)
        {
        cur->T->ExitToMsdos = TRUE;
        }

    Unlock();
    }

BOOL CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    switch (uMsg)
        {
        case WM_INITDIALOG:
            {
            MainDlgH = hwndDlg;

            if (cfg.BlinkPause)
                {
                if (!SetTimer (MainDlgH, ID_TIMER, cfg.BlinkPause, NULL))
                    {
                    MessageBox (MainDlgH, "Too many clocks or timers!", NULL, MB_ICONEXCLAMATION | MB_RETRYCANCEL);
                    }
                }

            return (1);
            }

        case WM_TIMER:
            {
            if (wParam == ID_TIMER) // ID
                {
                blinkstate = !blinkstate;
                return(1);
                }

            break;
            }

        case WM_DESTROY:
            {
            KillTimer (MainDlgH, ID_TIMER);
            break ;
            }

        case WM_COMMAND:
            {
            switch (LOWORD(wParam)) // ID
                {
                case 101:   // List box
                    {
                    if (HIWORD(wParam) == LBN_SELCHANGE)
                        {
                        LONG Sel = SendDlgItemMessage(MainDlgH, 101, LB_GETCURSEL, 0, 0);

                        if (Sel != LB_ERR)
                            {
                            TermWindowCollection.SetFocusInt(Sel);
                            return (1);
                            }
                        }

                    break;
                    }

                case 102:   // New Local
                    {
                    if (HIWORD(wParam) == BN_CLICKED)
                        {
                        Bool KeepWindow = ViewingTerm;

                        TurnOnTermWindow();

                        if (ViewingTerm)
                            {
                            NoComPortC *NCP = new NoComPortC;

                            if (NCP)
                                {
                                TermWindowC *TW = new TermWindowC(NCP, ConnSeq.Next());

                                if (TW && TW->IsGood())
                                    {
                                    TermWindowCollection.SetFocus(TW);

                                    HANDLE t = (HANDLE) _beginthread(NewTermWindowThread, 1024 * 20, TW);

                                    if (t != (HANDLE) -1)
                                        {
                                        TW->hThread = t;
                                        }

                                    KeepWindow = TRUE;
                                    }
                                else
                                    {
                                    delete TW;
                                    delete NCP;
                                    }
                                }

                            if (!KeepWindow)
                                {
                                TurnOffTermWindow();
                                }
                            }

                        return (1);
                        }

                    break;
                    }

                case 103:   // Kill Connection
                    {
                    if (HIWORD(wParam) == BN_CLICKED)
                        {
                        LONG Sel = SendDlgItemMessage(MainDlgH, 101, LB_GETCURSEL, 0, 0);

                        if (Sel != LB_ERR)
                            {
                            TermWindowCollection.SetForKill(Sel);
                            return (1);
                            }
                        }

                    break;
                    }

                case 105:   // Exit Citadel
                    {
                    switch (HIWORD(wParam)) // Notification
                        {
                        case BN_CLICKED:
                            {
                            TermWindowCollection.KillAll();

                            while (TermWindowCollection.AnyExist())
                                {
                                Sleep(500);
                                }

                            EndDialog(hwndDlg, 0);
                            return (1);
                            }
                        }

                    break;
                    }

                case 110:   // Run script
                    {
                    if (HIWORD(wParam) == BN_CLICKED)
                        {
                        _beginthread(RunStandAloneScript, 1024 * 20, NULL);
                        return (1);
                        }

                    break;
                    }

//              case 901:   // There's no help for you

                case 903:   // About
                    {
                    if (HIWORD(wParam) == BN_CLICKED)
                        {
                        if (hAboutDlg)
                            {
                            SetActiveWindow(hAboutDlg);
                            }
                        else
                            {
                            hAboutDlg = CreateDialog(hInstance, "ABOUTBOX", hwndDlg, (DLGPROC) AboutDlgProc);

                            if (hAboutDlg)
                                {
                                // Set about text
                                HWND hText = GetDlgItem(hAboutDlg, 101);

                                if (hText)
                                    {
                                    discardable *d;

                                    int ToRead = random(5) + 1;

                                    if ((d = readData(17, ToRead, ToRead)) != NULL)
                                        {
                                        char *Buffer = new char[32000];

                                        if (Buffer)
                                            {
                                            const char **Data =
                                                    (const char **) d->aux;
                                            *Buffer = 0;

                                            for (int Index = 0; !SameString(Data[Index], getmsg(307)); Index++)
                                                {
                                                if (Data[Index][0] != '#')
                                                    {
                                                    strcat(Buffer, deansi(Data[Index]));
                                                    strcat(Buffer, br);
                                                    strcat(Buffer, bn);
                                                    }
                                                }

                                            SendMessage(hText, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) Buffer);
                                            delete [] Buffer;
                                            }
                                        }

                                    discardData(d);
                                    }

                                ShowWindow(hAboutDlg, SW_SHOW);
                                }
                            }

                        return (1);
                        }
                    }
                }
            }

        // I guess I'll just close everything for now
        case MM_MCINOTIFY:
            {
            mciSendCommand(LOWORD(lParam), MCI_CLOSE, 0, NULL);
            return (1);
            }
        }

    return (0);
    }

BOOL WINAPI CitConsoleHandler(DWORD dwCtrlType)
    {
    switch (dwCtrlType)
        {
        case CTRL_BREAK_EVENT:
        case CTRL_C_EVENT:
            {
            return (TRUE);
            }

        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_CLOSE_EVENT:
            {
            PostMessage(MainDlgH, WM_COMMAND, MAKEWPARAM(105, BN_CLICKED), 0);

            for (;;)
                {
                Sleep(1000);
                }

            return (FALSE);
            }

        default:
            {
            return (FALSE);
            }
        }
    }


int PASCAL WinMain(HINSTANCE hInst, HINSTANCE /* hPrevInstance */, LPSTR lpszCmdParam, int /* nCmdShow */)
    {  
    hInstance = hInst;
    CopyStringToBuffer(cmdLine, lpszCmdParam);

    char line[256];
    CopyStringToBuffer(line, lpszCmdParam);

    char *words[256];
    const int argc = parse_it(words + 1, line);

    char line2[256];
    GetModuleFileName(hInstance, line2, 255);
    words[0] = line2;


    RoomFileMutex = CreateMutex(NULL, FALSE, "WincitMutex:RoomF");
    CronMutex = CreateMutex(NULL, FALSE, "WincitMutex:Cron");
    MenuMutex = CreateMutex(NULL, FALSE, "WincitMutex:Menu");
    TrapMutex = CreateMutex(NULL, FALSE, "WincitMutex:Trap");

#ifdef NAHERROR
    CreateErrorMutexes();
#endif

    InitCommonControls();

    startUp(argc + 1, (const char **) words);


    conRows = cfg.ConsoleRows - 1;
    conCols = cfg.ConsoleCols;

    HANDLE hUpdateThread = (HANDLE) _beginthread(ScreenUpdateThread, 1024 * 5, NULL);

    if (hUpdateThread != (HANDLE) -1)
        {
        SetThreadPriority(hUpdateThread, THREAD_PRIORITY_ABOVE_NORMAL);

        _beginthread(CronThread, 1024 * 5, NULL);

        if (cfg.MaxTelnet)
            {
            StartTelnetD();
            }

        for (int ComCounter = 0; ComCounter < 32; ComCounter++)
            {
            if (cfg.mdata & (1 << ComCounter))
                {
                label ComName;
                sprintf(ComName, "COM%d", ComCounter + 1);

                HANDLE h = CreateFile(ComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

                if (h != INVALID_HANDLE_VALUE)
                    {
                    DCB dcb;

                    if (GetCommState(h, &dcb))
                        {
                        dcb.BaudRate = bauds[cfg.initbaud];
                        dcb.fBinary = TRUE;
                        dcb.fParity = FALSE;
                        dcb.fOutxCtsFlow = cfg.checkCTS;
                        dcb.fOutxDsrFlow = FALSE;
                        dcb.fDtrControl = DTR_CONTROL_ENABLE;
                        dcb.fDsrSensitivity = FALSE;
//                      dcb.fTXContinueOnXoff:1; // XOFF continues Tx

                        dcb.fOutX = FALSE;
                        dcb.fInX = FALSE;
//                      dcb.fErrorChar: 1;   // enable error replacement
                        dcb.fNull = FALSE;
                        dcb.fRtsControl = RTS_CONTROL_ENABLE;
                        dcb.fAbortOnError = FALSE;
//                      dcb.fDummy2:17;      // reserved
//                      dcb.wReserved;        // not currently used
//                      dcb.XonLim;           // transmit XON threshold

//                      dcb.XoffLim;          // transmit XOFF threshold
                        dcb.ByteSize = 8;
                        dcb.Parity = NOPARITY;
                        dcb.StopBits = ONESTOPBIT;
//                      dcb.XonChar;          // Tx and Rx XON character
//                      dcb.XoffChar;         // Tx and Rx XOFF character
//                      dcb.ErrorChar;        // error replacement character
//                      dcb.EofChar;          // end of input character
//                      dcb.EvtChar;          // received event character

                        if (SetCommState(h, &dcb))
                            {
                            SerialPortListS *Cur = (SerialPortListS *) addLL((void **) &SerialPorts, sizeof(SerialPortListS));

                            if (Cur)
                                {
                                Cur->Port = new SerialPortC(h, ComCounter + 1);

                                if (Cur->Port)
                                    {
                                    _beginthread(ListenForSerialConnection, 4096, Cur->Port);
                                    }
                                else
                                    {
                                    deleteLLNode((void **) &SerialPorts, getLLCount(SerialPorts));
                                    }
                                }
                            }
                        else
                            {
                            char String[256];
                            sprintf(String, "SetComState(%s) failed.", ComName);
                            MessageBox(NULL, String, NULL, MB_ICONSTOP | MB_OK);
                            }
                        }
                    else
                        {
                        char String[256];
                        sprintf(String, "GetComState(%s) failed.", ComName);
                        MessageBox(NULL, String, NULL, MB_ICONSTOP | MB_OK);
                        }
                    }
                else
                    {
                    char String[256];

                    const DWORD Error = GetLastError();
                    switch (Error)
                        {
                        case 2:
                            {
                            sprintf(String, "Device \"%s\" does not exist.", ComName);
                            break;
                            }

                        case 5:
                            {
                            sprintf(String, "Device \"%s\" is already in use.", ComName);
                            break;
                            }

                        default:
                            {
                            sprintf(String, "Could not open device \"%s\" (%d).", ComName, Error);
                            break;
                            }
                        }

                    MessageBox(NULL, String, NULL, MB_ICONSTOP | MB_OK);
                    }
                }
            }

        SetConsoleCtrlHandler(CitConsoleHandler, TRUE);

        Net6969.StartListener();

        DialogBox(hInstance, "MAINDIALOG", NULL, (DLGPROC) MainDlgProc);

        if (cfg.MaxTelnet)
            {
            StopTelnetD();
            }

        // It might be nice to create a window here saying that we're waiting
        // for everybody else to go away...
        while (TermWindowCollection.AnyExist())
            {
            Sleep(500);
            }
        }
    else
        {
        MessageBox(NULL, "Could not create screen update thread", NULL, MB_ICONSTOP | MB_OK);
        }

#ifdef NAHERROR
    CloseErrorMutexes();
#endif

    CloseHandle(RoomFileMutex);
    CloseHandle(MenuMutex);

    exitcitadel();

    CloseHandle(CronMutex);
    CloseHandle(TrapMutex);

    return (return_code);
    }

void letWindowsMultitask(void)
    {
    Sleep(0);

    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    }
#endif

void TERMWINDOWMEMBER MainCitadelLoop(void)
    {
    char c;
    Bool modStatMain = FALSE;   // newCarrier && JustLostCarrier
    Bool didterminate = FALSE;  // nodes log in only on the first log-in
    Bool auto_login = FALSE;    // this is so it will go to login when run in door mode

#ifdef WINCIT
    seenAllRooms = FALSE;
    OurThreadId = GetCurrentThreadId();

    sprintf(LocalTempPath, "%s\\%ld", cfg.temppath, GetCurrentThreadId());
    mkdir(LocalTempPath);   // Check for error? Nah. Be bold!
#else
#endif

    if (login_pw || login_user || (slv_door && cfg.forcelogin))
        {
        auto_login = TRUE;
        }

#ifdef WINCIT
    srand((unsigned)time(NULL));
    Talley->Fill(); // hopefully the delay is unnoticeable
    ReadMCICit(FALSE, WC_TWp);
    OkayToShowEvents = FALSE;
#endif

#ifdef WINCIT
    if (inExternal(getmsg(1), WindowCaption))
        {
        setdefaultTerm(TT_DUMB);

        modStatMain = TRUE;
        if (modStat)
            {
            OC.whichIO = MODEM;
            OC.setio();
            }

        CommPort->FlushInput();

        dispBlb(B_BANPORT);

        CommPort->FlushOutput();

        if(!read_tr_messages())
            {
            errorDisp(getmsg(172));
            return;
            }

        trap(T_HACK, WindowCaption, gettrmsg(18));

        dump_tr_messages();

        return;
        }
#endif

    // this is the main citadel loop
    while (!ExitToMsdos)
        {
#ifdef MSC
        if (_nullcheck()) // undocumented MSC function
            {
            doccr();
            }
#endif
        VerifyHeap();

        OC.SetOutFlag(IMPERVIOUS);

        StatusLine.Update(WC_TWp);

#if defined(__BORLANDC__) && !defined(WINCIT)
        if (heapcheck() < 0)
            {
            crashout(getmsg(646));
            }
#endif

#ifndef WINCIT
        if (farcoreleft() < PURGE1VALUE)
            {
            compactMemory();

            if (farcoreleft() < PURGE2VALUE)
                {
                compactMemory(1);

                if (farcoreleft() < PURGE3VALUE)
                    {
                    compactMemory(2);

                    if (farcoreleft() < WARNVALUE)
                        {
                        CRmPrintfCR(farcoreleft() < DANGERVALUE ? getmsg(610) : getmsg(612));
                        }
                    }
                }
            }
#endif

        // Carrier Detect
        if (!loggedIn)
            {
            if ((modStat && !modStatMain) || auto_login)
                {
                showPrompt = TRUE;

                setdefaultTerm(TT_DUMB);

                modStatMain = TRUE;
                if (modStat)
                    {
                    OC.whichIO = MODEM;
                    OC.setio();
                    }

                CommPort->FlushInput();
                greeting();

                if (cfg.forcelogin || auto_login)
                    {
                    int trys = 0;

                    do
                        {
                        // login modified to handle command line login
                        doLogin(2);

                        if (!loggedIn && cfg.badPwPause)
                            {
                            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(92));

                            pause(cfg.badPwPause * 100);

                            if (w)
                                {
                                destroyCitWindow(w, FALSE);
                                }
                            }

                        } while (!loggedIn && HaveConnectionToUser() && trys++ < 3);

                    if (!loggedIn)
                        {
                        if (!onConsole)
                            {
                            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

                            Hangup();

                            if (w)
                                {
                                destroyCitWindow(w, FALSE);
                                }
                            }
                        else
                            {
                            OC.whichIO = MODEM; // simulated hangup
                            OC.setio();
                            }
                        }
                    }

                auto_login = FALSE;
                }
            }

        if (CurrentUser->IsNode() && loggedIn && modStat && !didterminate)
            {
            SetDoWhat(NETWORKING);

            netError = !net_slave();
            SetDoWhat(DUNO);

            CarrierJustLost();

            MRO.Verbose = FALSE;
            terminate(FALSE);   // why not true?

            setdefaultTerm(TT_ANSI);
            showPrompt = TRUE;
            }

        // Carrier Loss
        if (!HaveConnectionToUser())
            {
            didterminate = FALSE; // node can log in now

            if (slv_door)
                {
                ExitToMsdos = TRUE;
                }

            CarrierJustLost();

            if (loggedIn)
                {
                MRO.Verbose = FALSE;
                terminate(FALSE);
                }

            modStatMain = FALSE;

            OC.Echo = BOTH;
            OC.whichIO = CONSOLE;
            OC.SetOutFlag(IMPERVIOUS);
            modStat = FALSE;

            OC.setio();

            if (!CommPort->HaveConnection())
                {
                CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

#ifndef WINCIT
                Initport();
#endif

                if (w)
                    {
                    destroyCitWindow(w, FALSE);
                    }
                }

            setdefaultTerm(TT_ANSI);
            showPrompt = TRUE;
#ifdef WINCIT
            ExitToMsdos = TRUE;
#endif
            }

        // I'm not sure if this really belongs here
        if (loggedIn && !CurrentUser->IsVerified())
            {
            MRO.Verbose = FALSE;
            terminate(TRUE);
            setdefaultTerm(TT_ANSI);
            showPrompt = TRUE;
            }

#ifndef WINCIT
        // Better do this *BEFORE* a room prompt!
        if (sysReq && !loggedIn && !modStat && !ExitToMsdos)
            {
            sysReq = FALSE;

            if (cfg.offhook)
                {
                offhook();
                }
            else
                {
                CommPort->DropDtr();
                }

            ringSystemREQ();
            }
#endif

        Cron.ResetTimer();
        Bool x = getCommand(&c, showPrompt);
        showPrompt = TRUE;
        Bool NeedToShowPrompt = FALSE;

        if (c)
            {
            Cron.ResetTimer();

            if (toupper(c) == 'T' && !onConsole)
                {
                didterminate = TRUE;    // node cannot log in
                }

            // do greeting from console login
            if ((toupper(c) == 'L') && !x && !loggedIn && !modStat)
                {
                greeting();
                }
            }

        if (chatkey)
            {
            chat();
            NeedToShowPrompt = TRUE;
            }

#ifndef WINCIT
        if ((eventkey || Cron.IsReady()) && !modStat && !loggedIn)
            {
            if (Cron.Do(CRON_TIMEOUT))
                {
                NeedToShowPrompt = TRUE;
                }

            eventkey = FALSE;
            forceevent = FALSE;
            repeatevent = FALSE;

            setdefaultTerm(TT_ANSI);
            }
#endif

        if (sysopkey)
            {
            const ModemConsoleE oldIO = OC.whichIO;
            OC.whichIO = CONSOLE;
            OC.setio();

            mPrintfCR(getmsg(613));

            doSysop();
            if (modStat)
                {
                OC.whichIO = oldIO;
                OC.setio();
                }

            if (!modStat)
                {
                setdefaultTerm(TT_ANSI);
                }

            NeedToShowPrompt = TRUE;
            }

        if (c && (x || toupper(c) != 'P' || (CurrentUser->IsExpert() && (cfg.poop == 1)) || cfg.poop < 1))
            {
            doRegular(x, c);
            }
        else
            {
            showPrompt = NeedToShowPrompt;
            }

        // unsuccessful login
        if ((toupper(c) == 'L') && !x && !loggedIn && !modStat)
            {
            OC.whichIO = MODEM; // simulated hangup
            OC.setio();
            }
        }

    if (loggedIn)
        {
        MRO.Verbose = FALSE;
        terminate(TRUE);
        }
    }


// --------------------------------------------------------------------------
// doRegular(): High-level command menu.
//
// Input:
//  Bool x: TRUE if this is a dot command; FALSE if single-key.
//
//  char c: The character key selected by the user.
//
// Return value:
//  TRUE if command not handled.  Unused.

Bool TERMWINDOWMEMBER doRegular(Bool x, char c)
    {
    Bool toReturn = FALSE;
    Bool didit = FALSE;

    MRO.Reverse = FALSE;

    OC.SetOutFlag(IMPERVIOUS);

    if (execDoor(c, x ? 2 : 1))
        {
        return (FALSE);
        }

    if (hitSix)
        {
        if (c == '9')
            {
            mPrintf(getmsg(69));
            didit = TRUE;
            }

        hitSix = FALSE;
        }

#ifndef WINCIT

    if (toupper(c) != 'W')
        {
        wowcount = 0;
        }

#else

    if ((toupper(c) == 'W' && x) || toupper(c) != 'W')
        {
        wowcount = 0;
        }

#endif


    if (c == '6')
        {
        hitSix = TRUE;
        }

    switch (toupper(c))
        {
        case 'A':
            {
            if (CurrentUser->IsAide())
                {
                doAide(x, 'E');
                }
            else
                {
                toReturn = TRUE;
                }

            break;
            }

        case 'B': doGoto(x, TRUE);      break;
        case 'C': doChat(x);            break;
        case 'D': doDownload(x);        break;
        case 'E': doEnter(x, 'm');      break;

        case 'F':
            {
            if (x)
                {
                doFinger();
                }
            else
                {
                doRead(x, 'f');
                }

            break;
            }

        case 'G': doGoto(x, FALSE); break;
        case 'H': doHelp(x, NULL);  break;

        case 'I':
            {
            if (x && (CurrentUser->IsAide() || CurrentUser->IsSysop()))
                {
                doInvite();
                }
            else
                {
                DoCommand(UC_INTRO);
                }

            break;
            }

        case 'J': unGotoRoom();     break;
        case 'K': doKnown(x, 'r');  break;
        case 'L': doLogin(x);       break;

        case 'M':
            {
            if (x)
                {
                doGotoMail();
                }
            else
                {
                doEnter(FALSE, 'e');
                }
            break;
            }

        case 'N': doRead(x, 'n');   break;
        case 'O': doRead(x, 'o');   break;

        case 'P':
            {
            if (x)
                {
                doPersonal();
                }
            else
                {
                toReturn = doPoop(c);
                }

            break;
            }

        case 'Q':
            {
            if (x && haveRespProt())
                {
                doFileQueue();
                }
            else
                {
                if ((CurrentUser->IsExpert() && (cfg.poop == 1)) || cfg.poop < 1)
                    {
                    mPrintf(getmsg(231));
                    }
                else
                    {
                    toReturn = TRUE;
                    }
                }

            break;
            }

        case 'R': doRead(x, 'r');   break;

        case 'S':
            {
            if (CurrentUser->IsSysop() && x)
                {
                mPrintfCR(getmsg(38));
                doSysop();
                }
            else
                {
                toReturn = doPoop(c);
                }

            break;
            }

        case 'T': doLogout(x, 'q'); break;
        case 'U': doUpload(x);      break;

        case 'V':
            {
            if (x)
                {
                doVolkswagen(x, NULL);
                }
            else
                {
                DoCommand(UC_VERBOSETOGGLE);
                }
            break;
            }

        case 'W': doWow(x);         break;

        case 'X':
            {
            if (!x)
                {
                doEnter(x, 'x');
                }
            else
                {
                DoCommand(UC_EXPERTTOGGLE);
                }
            break;
            }

        case 'Y':
            {
            DoCommand(UC_Y, x);
            break;
            }

//      case 'Z':
//          {
//          QuickRunApp("debug");
//          break;
//          }

        case '=':
        case '+': doNext();         break;

        case '\b':
            {
            if (x)
                {
                oChar('\b');
                showPrompt = FALSE;
                break;
                }
            }
        case '-': doPrevious();     break;

        case ']':
        case '>': doNextHall();     break;

        case '[':
        case '<': doPreviousHall(); break;

        case ';': doSmallChat();    break;

        case '*': doReadBull(TRUE); break;

        case '?':
            {
            oChar('?');
            if (x)
                {
                showMenu(M_MAINDOT);
                }
            else
                {
                showMenu(M_MAINOPT);
                }
            break;
            }

        case 0:     // irrelevant value
            {
            if (debug)
                {
                DebugOut(88, getdbmsg(93));
                }

            break;
            }

        default:
            {
            toReturn = TRUE;
            break;
            }
        }

    if (toReturn && !didit)
        {
        oChar(c);

        if (!CurrentUser->IsExpert())
            {
            CRCRmPrintfCR(getmsg(44));
            }
        else
            {
            mPrintfCR(sqst);
            }
        }

    return (toReturn);
    }

void TERMWINDOWMEMBER ShowPrompt(void)
    {
#ifdef DRAGON
    dragonAct();    // user abuse rutine :-)
#endif
    doAd(FALSE);    // A Genius rutine :-)

//  doRapp(FALSE);  // a dork rutine :-)

    if (cfg.borders)
        {
        doBorder();
        }

    GiveRoomPrompt(FALSE);
    }


// --------------------------------------------------------------------------
// takeAutoUp(): take an auto upload from a user into directory rooms
//
// Input:
//  char c: The character entered by the user.
//
// Return value:
//  TRUE if upload happened; FALSE if not.

Bool TERMWINDOWMEMBER takeAutoUp(char c)
    {
    protocols *theProt;

    if (!RoomTab[thisRoom].IsMsDOSdir() ||
            (
            RoomTab[thisRoom].IsDownloadOnly() && !CurrentUser->HasRoomPrivileges(thisRoom)
            )
        )
        {
        return (FALSE);
        }

    for (theProt = extProtList; theProt; theProt = (protocols *) getNextLL(theProt))
        {
        if ((c == theProt->autoDown[0]) && theProt->batch)
            {
            break;
            }
        }

    if (!theProt)
        {
        return (FALSE);
        }

    twirlypause(15);

    if (!CommPort->IsInputReady())
        {
        return (FALSE);
        }

    for (int t = 1; theProt->autoDown[t]; t++)
        {
        twirlypause(15);

        if (!CommPort->IsInputReady() || (CommPort->Input() != theProt->autoDown[t]))
            {
            return (FALSE);
            }
        }

    SetDoWhat(DOUPLOAD);
    ActuallyDoTheUpload(theProt, FALSE, FALSE);

    ShowPrompt();
    return (TRUE);
    }


// --------------------------------------------------------------------------
// CheckTimeOnSystem(): Check if it is time to kick the user off, and do so
//  if it is.

void TERMWINDOWMEMBER CheckTimeOnSystem(void)
    {
    // update user's balance
    if (cfg.accounting && CurrentUser->IsAccounting())
        {
#ifdef WINCIT
        CurrentUserAccount->UpdateBalance(this);
#else
        CurrentUserAccount->UpdateBalance();
#endif
        }

    if (altF3Timeout)
        {
        long tme;

        if (time(&tme) < altF3Timeout)
            {
            label Minutes, ZeroMinutes, Seconds;
            char OutMsg[128];

            sprintf(Minutes, pctld, (altF3Timeout - tme) / 60);
            sprintf(ZeroMinutes, getmsg(20), (altF3Timeout - tme) / 60);
            sprintf(Seconds, getmsg(20), (altF3Timeout - tme) % 60);

            sformat(OutMsg, cfg.AltF3Msg, getmsg(227), Minutes, ZeroMinutes, Seconds, 0);

            CRmPrintfCR(pcts, OutMsg);
            }
        else
            {
            label Minutes, ZeroMinutes, Seconds;
            char OutMsg[128];

            sprintf(Minutes, pctd, cfg.altF3Time / 60);
            sprintf(ZeroMinutes, getmsg(47), cfg.altF3Time / 60);
            sprintf(Seconds, getmsg(47), cfg.altF3Time % 60);

            sformat(OutMsg, cfg.AltF3Timeout, getmsg(227), Minutes, ZeroMinutes, Seconds, 0);

            CRmPrintf(pcts, OutMsg);

            MRO.Verbose = TRUE;
            terminate(TRUE);
            altF3Timeout = 0;
            }
        }
    }

// --------------------------------------------------------------------------
// getCommand(): prints menu prompt and gets command char.
//
// Input:
//  char *c: Used for output; see below.
//
//  Bool prompt: I don't know.
//
// Output:
//  char *c: They input selected by the user.
//
// Return value:
//  TRUE if dot command; FALSE if single-key.

Bool TERMWINDOWMEMBER getCommand(char *c, Bool prompt)
    {
    Bool expand = FALSE;

    // Carrier loss!
    if (!HaveConnectionToUser())
        {
        *c = 0;
        return (FALSE);
        }

    OC.SetOutFlag(IMPERVIOUS);


#ifdef DEMO
    if (time(NULL) > (uptimestamp + DEMOTIMEOUT))
        {
        ExitToMsdos = TRUE;
        }
    else if (time(NULL) > (uptimestamp + DEMOWARN))
        {
        mPrintfCR("Demonstration version expires in %d seconds", uptimestamp + DEMOTIMEOUT - time(NULL));
        }
#endif

    CheckTimeOnSystem();

    if (prompt && !IdlePrompt)
        {
        ShowPrompt();
        }

    SetDoWhat(MAINMENU);

    OkayToShowEvents = TRUE;
    *c = (char) iCharNE();
    OkayToShowEvents = FALSE;

    if (IdlePrompt && !CursorIsAtPrompt)
	    {
        GiveRoomPrompt(FALSE);
		}
      
    IdlePrompt = FALSE;

    if (!*c && debug)
        {
        DebugOut(88, getdbmsg(94));
        }

    if (takeAutoUp(*c))
        {
        *c = 0;
        }
    else
        {
        expand = (char) ((*c == ' ') || (*c == '.') || (*c == ',') || (*c == '/'));

        if (expand)
            {
            oChar('.');
            *c = (char) iCharNE();
            }
        SetDoWhat(DUNO);
        }

    return (expand);
    }

// --------------------------------------------------------------------------
// doAd(): Shows an AD*.BLB cfg.ad_chance % of the time
//
// Input:
//  Bool force: TRUE to force the blurb to display; don't compute chance.
//
// Return value:
//  None.

void TERMWINDOWMEMBER doAd(Bool force)
    {
    if (loggedIn && !CurrentUser->IsViewBorders())
        {
        return;
        }

    if ((cfg.ad_chance && (random(100) + 1 <= cfg.ad_chance)) || force)
        {
        BLBRotate(getmsg(228), getmsg(355), &cfg.normAd, &cfg.ansiAd, FALSE, B_AD);
        }
    }
