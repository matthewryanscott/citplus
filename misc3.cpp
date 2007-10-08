// --------------------------------------------------------------------------
// Citadel: Misc3.CPP
//
// Overlayed miscellaneous stuff that is not used often; compare with
// Misc2.CPP.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "config.h"
#include "msg.h"
#include "fscfg.h"
#include "scrlback.h"
#include "timer.h"
#include "readcfg.h"
#include "helpfile.h"
#include "cfgfiles.h"
#include "cwindows.h"
#include "timechk.h"
#include "miscovl.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// offhook()        sysop fn: to take modem off hook
// ringSystemREQ()  signals a system request for 2 minutes.
// dial_out()       dial out to other boards
// greeting()       System-entry blurb etc
// cCPrintf()       send formatted output to console, centered
// logo()           prints out system logo at startup
// parseArgs()      sets global flags baised on command line


extern ConsoleLockC ConsoleLock;
extern int FilesRequired;

void GetSysPWHandler(const char *PW);

// --------------------------------------------------------------------------
// ringSystemREQ(): signals a system request for 2 minutes.

void TERMWINDOWMEMBER ringSystemREQ(void)
    {
    altF3Timeout = 0;

    doccr();
    doccr();

#ifdef WINCIT
    WinPutString(logiRow, getmsg(506), cfg.wattr | 128, FALSE);
#else
    (*stringattr)(logiRow, getmsg(506), (uchar)(cfg.wattr | 128), FALSE);
#endif
    StatusLine.Update(WC_TWp);
    doccr();

    ScreenSaver.Update();

    Hangup();

    Bool answered = FALSE;
    for (int i = 0; (i < 120) && !answered; i++)
        {
        outCon(BELL);
        pause(100);

        if (KBReady())
            {
            answered = TRUE;
            }
        }

    if (!answered)
        {
        CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
                CitWindowsMsg(NULL, getmsg(19));

        Initport();

        if (w)
            {
            destroyCitWindow(w, FALSE);
            }
        }

    StatusLine.Update(WC_TWp);
    }


// --------------------------------------------------------------------------
// dial_out(): dial out to other boards

void TERMWINDOWMEMBER dial_out(void)
    {
#ifndef WINCIT
    TimeChangeCheckerC UpdateTime;
    Bool LocalEcho = FALSE;
    Bool quit = FALSE;

    SetDoWhat(DIALOUT);

    OC.SetOutFlag(IMPERVIOUS);

    if (!rlmEvent(LT_STARTDIALOUT, (long) &LocalEcho, (long) &quit))
        {
        doCR();
        dispBlb(B_DIALOUT);
        }

    CITWINDOW *w = CitWindowsMsg(NULL, getmsg(19));

    Hangup();

    CommPort->Enable();

    CommPort->SetSpeed(cfg.initbaud);
    StatusLine.Update(WC_TWp);

    CommPort->OutString(cfg.dialsetup);
    CommPort->OutString(br);

    pause(100);

    if (w)
        {
        destroyCitWindow(w, FALSE);
        //w = NULL;
        }

    callout = TRUE;

    do
        {
        // If some script kicked in and changed outflag...
        if (!OC.User.CanOutput())
            {
            OC.SetOutFlag(IMPERVIOUS);
            }

        dialout_fkey = 0;

        if (KBReady())
            {
            char con = (char) ciChar();

            if (con)
                {
                CommPort->Output(con);

                if (LocalEcho)
                    {
                    oChar(con);
                    }
                }
            }

        if (CommPort->IsInputReady())
            {
            char mod = (char) CommPort->Input();

            if (mod == '\n')
                {
                doccr();
                }
            else if (mod != '\r')
                {
                oChar(mod);
                }
            }

        // handle alt-keys
        if (dialout_fkey)
            {
            if (!rlmEvent(LT_DIALOUTKEY, dialout_fkey, 0))
                {
                if (LockMessages(MSG_CONSOLE))
                    {
                    switch (dialout_fkey)
                        {
                        case ALT_D:
                            {
                            if (!ConsoleLock.IsLocked())
                                {
                                if (debug)
                                    {
                                    TurnDebugOff();

                                    if (cfg.VerboseConsole)
                                        {
                                        CitWindowsNote(NULL,
                                                getmsg(MSG_CONSOLE, 21),
                                                getmsg(316));
                                        }
                                    }
                                else
                                    {
                                    if (TurnDebugOn())
                                        {
                                        if (cfg.VerboseConsole)
                                            {
                                            CitWindowsNote(NULL,
                                                    getmsg(MSG_CONSOLE, 21),
                                                    getmsg(317));
                                            }
                                        }
                                    else
                                        {
                                        if (cfg.VerboseConsole)
                                            {
                                            CitWindowsNote(NULL,
                                                    getmsg(MSG_CONSOLE, 22));
                                            }
                                        }
                                    }
                                }
                            else
                                {
                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 2));
                                    }
                                }

                            break;
                            }

                        case ALT_E:
                            {
                            if (!ConsoleLock.IsLocked())
                                {
                                shellescape(FALSE);
                                }
                            else
                                {
                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 2));
                                    }
                                }

                            break;
                            }

                        case ALT_H:
                            {
                            CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
                                    CitWindowsMsg(NULL, getmsg(82));

                            Hangup();
                            CommPort->Enable(); // ?? RAF ??

                            if (w)
                                {
                                destroyCitWindow(w, FALSE);
                                }

                            break;
                            }

                        case ALT_K:
                            {
                            DisplayScrollBackBuffer();
                            break;
                            }

                        case ALT_L:
                            {
                            if (ConsoleLock.IsLocked())
                                {
                                if (ConsoleLock.MayUnlock())
                                    {
                                    GetStringFromConsole(getmsg(MSG_CONSOLE, 0),
                                            LABELSIZE, ns, GetSysPWHandler,
                                            TRUE);
                                    }
                                else
                                    {
                                    if (cfg.VerboseConsole)
                                        {
                                        CitWindowsNote(NULL,
                                                getmsg(MSG_CONSOLE, 1));
                                        }
                                    }
                                }
                            else if (*cfg.f6pass)
                                {
                                ConsoleLock.Lock();

                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL,
                                            getmsg(MSG_CONSOLE, 32));
                                    }
                                }
                            else
                                {
                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL,
                                            getmsg(MSG_CONSOLE, 33),
                                            citfiles[C_CONFIG_CIT]);
                                    }
                                }

                            break;
                            }

                        case ALT_P:
                            {
                            LocalEcho = !LocalEcho;

                            if (cfg.VerboseConsole)
                                {
                                CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 34),
                                        LocalEcho ? getmsg(317) :
                                        getmsg(316));
                                }

                            break;
                            }

                        case ALT_Q:
                            {
                            doccr();
                            cPrintf(getmsg(MSG_CONSOLE, 35));
                            quit = TRUE;
                            break;
                            }

                        case ALT_S:
                            {
                            if (!ConsoleLock.IsLocked())
                                {
                                shellescape(TRUE);
                                }
                            else
                                {
                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL,
                                            getmsg(MSG_CONSOLE, 2));
                                    }
                                }

                            break;
                            }

                        case ALT_V:
                            {
                            if (!ConsoleLock.IsLocked())
                                {
                                if (StatusLine.ToggleFullScreen())
                                    {
                                    StatusLine.Update(WC_TWp);
                                    }
                                else
                                    {
                                    if (cfg.VerboseConsole)
                                        {
                                        CitWindowsNote(NULL,
                                                getmsg(MSG_CONSOLE, 8));
                                        }
                                    }
                                }
                            else
                                {
                                if (cfg.VerboseConsole)
                                    {
                                    CitWindowsNote(NULL,
                                            getmsg(MSG_CONSOLE, 2));
                                    }
                                }

                            break;
                            }

                        case PGUP:
                            {
                            PortSpeedE NewSpeed = (PortSpeedE)
                                    ((CommPort->GetSpeed() + 1) % PS_NUM);

                            CITWINDOW *w = (ScreenSaver.IsOn() || !cfg.baudPause) ?
                                    NULL : CitWindowsMsg(NULL, getmsg(86),
                                    bauds[NewSpeed]);

                            CommPort->SetSpeed(NewSpeed);

                            if (w)
                                {
                                destroyCitWindow(w, FALSE);
                                }

                            StatusLine.Update(WC_TWp);
                            break;
                            }

                        case PGDN:
                            {
                            PortSpeedE NewSpeed = (PortSpeedE)
                                    (CommPort->GetSpeed() - 1);
                            if (NewSpeed < PS_300)
                                {
                                NewSpeed = (PortSpeedE) (PS_NUM - 1);
                                }


                            CITWINDOW *w = (ScreenSaver.IsOn() || !cfg.baudPause) ?
                                    NULL : CitWindowsMsg(NULL, getmsg(86),
                                    bauds[NewSpeed]);

                            CommPort->SetSpeed(NewSpeed);

                            if (w)
                                {
                                destroyCitWindow(w, FALSE);
                                }

                            StatusLine.Update(WC_TWp);
                            break;
                            }

                        default:
                            {
                            int dm_ofs = -1;

                            // Tell me: is this ugly?
                            if (dialout_fkey >= F1 && dialout_fkey <= F10)
                                {
                                dm_ofs = dialout_fkey - F1 + DM_F1;
                                }
                            else if (dialout_fkey == F11 || dialout_fkey == F12)
                                {
                                dm_ofs = dialout_fkey - F11 + DM_F11;
                                }
                            else if (dialout_fkey >= ALT_F1 && dialout_fkey <= ALT_F10)
                                {
                                dm_ofs = dialout_fkey - ALT_F1 + DM_ALT_F1;
                                }
                            else if (dialout_fkey >= ALT_F11 || dialout_fkey == ALT_F12)
                                {
                                dm_ofs = dialout_fkey - ALT_F11 + DM_ALT_F11;
                                }
                            else if (dialout_fkey >= SFT_F1 && dialout_fkey <= SFT_F10)
                                {
                                dm_ofs = dialout_fkey - SFT_F1 + DM_SFT_F1;
                                }
                            else if (dialout_fkey >= SFT_F11 || dialout_fkey == SFT_F12)
                                {
                                dm_ofs = dialout_fkey - SFT_F11 + DM_SFT_F11;
                                }
                            else if (dialout_fkey >= CTL_F1 && dialout_fkey <= CTL_F10)
                                {
                                dm_ofs = dialout_fkey - CTL_F1 + DM_CTL_F1;
                                }
                            else if (dialout_fkey >= CTL_F11 || dialout_fkey == CTL_F12)
                                {
                                dm_ofs = dialout_fkey - CTL_F11 + DM_CTL_F11;
                                }

                            if (dm_ofs > -1)
                                {
                                CommPort->OutString(cfg.dialmacro[dm_ofs]);

                                if (LocalEcho)
                                    {
                                    cPrintf(cfg.dialmacro[dm_ofs]);
                                    }
                                }
                            }
                        }

                    UnlockMessages(MSG_CONSOLE);
                    }
                else
                    {
                    cOutOfMemory(7);
                    }
                }
            }

        // Update time on status line
        if (UpdateTime.Check())
            {
            if (CronMonitor)
                {
                (CronMonitor->func)(EVT_DRAWINT, 0, 0, CronMonitor);
                }

            StatusLine.Update(WC_TWp);
            UpdateTime.Reset();
            }
        } while (!quit);

    callout = FALSE;

    w = CitWindowsMsg(NULL, getmsg(19));

    Initport();

    if (cfg.offhook && loggedIn)
        {
        offhook();
        }

    if (w)
        {
        destroyCitWindow(w, FALSE);
        //w = NULL;
        }

    rlmEvent(LT_ENDDIALOUT, 0, 0);

    SetDoWhat(DUNO);

    Cron.ResetTimer();
    time(&LastActiveTime);

    doCR();
#endif
    }


// --------------------------------------------------------------------------
// cCPrintf(): send formatted output to console, centered
//
// Input:
//  const char *fmt: The format string, as in printf()
//  ...: What to format, as in printf()

void cdecl TERMWINDOWMEMBER cCPrintf(const char *fmt, ...)
    {
    va_list ap;

    va_start(ap, fmt);
    vsprintf(OC.prtf_buff, fmt, ap);
    va_end(ap);

    int i = (conCols - strlen(OC.prtf_buff)) / 2;

    strrev(OC.prtf_buff);

    while (i--)
        {
        strcat(OC.prtf_buff, spc);
        }

    strrev(OC.prtf_buff);

#ifdef WINCIT
    WinPutString(logiRow, OC.prtf_buff, cfg.attr, TRUE);
#else
    (*stringattr)(logiRow, OC.prtf_buff, cfg.attr, TRUE);
#endif
    }


// --------------------------------------------------------------------------
// logo(): Prints out system logo at startup.

#ifdef WINCIT
long FAR PASCAL logoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    switch (message)
        {
        case WM_PAINT:
            {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBITMAP hBMP = LoadBitmap(hInstance, "CITADELLOGO");

            if (hBMP)
                {
                DrawBitmap(hdc, hBMP, 0, 0);
                DeleteObject(hBMP);
                }

            EndPaint(hwnd, &ps);
            return (0);
            }
        }

    return (DefWindowProc(hwnd, message, wParam, lParam));
    }

void logo(int turnOn)
    {
    static HWND logoWindow;

    if (turnOn)
        {
        if (!logoWindow)
            {
            HBITMAP hBMP = LoadBitmap(hInstance, "CITADELLOGO");

            if (hBMP)
                {
                BITMAP bm;

                if (!GetObject(hBMP, sizeof(BITMAP), (LPVOID) &bm))
                    {
                    logoWindow = CreateWindow("Citadel Logo", programName, WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) - bm.bmWidth) / 2,
                            (GetSystemMetrics(SM_CYSCREEN) - bm.bmHeight) / 2, bm.bmWidth, bm.bmHeight, NULL, NULL, hInstance, NULL);

                    if (logoWindow)
                        {
                        ShowWindow(logoWindow, SW_SHOW);
                        UpdateWindow(logoWindow);
                        }
                    }

                DeleteObject(hBMP);
                }
            }
        }
    else
        {
        if (logoWindow)
            {
            if (DestroyWindow(logoWindow))
                {
                logoWindow = NULL;
                }
            }
        }

    letWindowsMultitask();
    }
#else
void logo(int turnOn)
    {
    outPhys(TRUE);

    if (turnOn)
        {
        int i;

        cls(SCROLL_NOSAVE);

        getScreenSize((&conCols), (&conRows));

        for (i = 0; i < ((conRows/2) -1); i++)
            {
            doccr();
            }

        cCPrintf(getmsg(682), cfg.softverb, cfg.softverb[0] ? spc : ns,
                programName, version);
        doccr();

        cCPrintf(pcts, Author);
        doccr();
        doccr();
        }
    else
        {
        cls(SCROLL_SAVE);
        }

    outPhys(FALSE);
    }
#endif

// --------------------------------------------------------------------------
// generateHelpFiles()
//
// Notes:
//  Assumes MSG_CMDLINE is locked.

#ifndef WINCIT
static void generateHelpFiles(void)
    {
    VerifyHeap();

    printf(getmsg(MSG_CMDLINE, 3));

    for (int i = 0; i < MAXHLP; i++)
        {
        VerifyHeap();

        discardable *d;

        if ((d = readData(14, i, i)) != NULL)
            {
            VerifyHeap();

            if (i)
                {
                printf(getmsg(236));
                }

            label fn;
            sprintf(fn, getmsg(MSG_CMDLINE, 4), hlpnames[i]);
            printf(fn);

            FILE *fd;
            if ((fd = fopen(fn, FO_W)) != NULL)
                {
                for (int j = 0; ((const char **) d->aux)[j][0] != '#'; j++)
                    {
                    fprintf(fd, getmsg(472), ((const char **) d->aux)[j], bn);
                    }

                fclose(fd);
                }
            else
                {
                printf(getmsg(MSG_CMDLINE, 5));
                }

            discardData(d);
            }
        else
            {
            printf(getmsg(MSG_CMDLINE, 6));
            }
        }

    for (int i = 0; i < MAXBLBS; i++)
        {
        VerifyHeap();

        discardable *d;

        if ((d = readData(13, i, i)) != NULL)
            {
            VerifyHeap();

            printf(getmsg(236));

            label fn;
            sprintf(fn, getmsg(MSG_CMDLINE, 7), blbnames[i]);
            printf(fn);

            FILE *fd;
            if ((fd = fopen(fn, FO_W)) != NULL)
                {
                for (int j = 0; ((const char **) d->aux)[j][0] != '#'; j++)
                    {
                    fprintf(fd, getmsg(472), ((const char **) d->aux)[j], bn);
                    }

                fclose(fd);
                }
            else
                {
                printf(getmsg(MSG_CMDLINE, 5));
                }

            discardData(d);
            }
        else
            {
            printf(getmsg(MSG_CMDLINE, 6));
            }
        }

    VerifyHeap();

    printf(getmsg(472), getmsg(236), ctdlMnu);
    discardable *d;

    if ((d = readData(11)) != NULL)
        {
        VerifyHeap();

        FILE *fd;
        if ((fd = fopen(ctdlMnu, FO_W)) != NULL)
            {
            VerifyHeap();

            discardable *c;
            int i;

            for (c = d, i = 0; i < MAXMENUS; i++,
                    c = (discardable *) getNextLL(c))
                {
                VerifyHeap();

                // Don't write out optional menus.
                if (c && ((const char **) c->aux)[0][0] != '#')
                    {
                    fprintf(fd, getmsg(217), menunames[i], bn);

                    for (int j = 0; ((const char **) c->aux)[j][0] != '#';
                            j++)
                        {
                        fprintf(fd, getmsg(472), ((const char **) c->aux)[j],
                                bn);
                        }
                    }
                }

            fclose(fd);
            }
        else
            {
            printf(getmsg(MSG_CMDLINE, 5));
            }

        discardData(d);
        }
    else
        {
        printf(getmsg(MSG_CMDLINE, 6));
        }

    printf(getmsg(MSG_CMDLINE, 8));

    VerifyHeap();
    }
#endif

// --------------------------------------------------------------------------
// parseArgs(): Sets global flags baised on command line.
//
// Input:
//  int argc: As in C++ RTL's main()
//  const char *argv[]: As in C++ RTL's main()

void parseArgs(int argc, const char *argv[])
    {
    if (!LockMessages(MSG_CMDLINE))
        {
#ifdef WINCIT
        char Buffer[128];
        sprintf(Buffer, getmsg(188), getmsg(460));
        MessageBox(NULL, Buffer, NULL, MB_ICONSTOP | MB_OK);
#else
        printf(getmsg(188), getmsg(460));
        DeinitializeTimer();
#endif
        critical(FALSE);
        exit(0);
        }

    cfg.bios = TRUE;

    // logo gets white letters
    cfg.attr = 7;
    cfg.cattr = 15;
    cfg.uttr = 1;
    cfg.wattr = 0x70;

    for (int i = 1; i < argc; i++)
        {
        if (argv[i][0] == '/' || argv[i][0] == '-')
            {
            switch (toupper((int) argv[i][1]))
                {
                case '0':
                    {
                    cmdLine[1] = 1;
                    break;
                    }

                case 'A':
                case 'L':
                    {
                    slv_door = TRUE;
                    break;
                    }

#ifndef WINCIT
                case 'B':
                    {
                    const char *SpeedPtr;

                    if (argv[i][2])
                        {
                        SpeedPtr = argv[i] + 2;
                        }
                    else
                        {
                        SpeedPtr = argv[++i];
                        }

                    slv_baud = digitbaud(atol(SpeedPtr));

                    if (slv_baud == PS_ERROR)
                        {
                        printf(getmsg(MSG_CMDLINE, 1), SpeedPtr);
                        DeinitializeTimer();
                        critical(FALSE);
                        exit(0);
                        }

                    for (ModemSpeedE j = (ModemSpeedE) 0; j < MS_NUM;
                            j = (ModemSpeedE) (j + 1))
                        {
                        if (connectbauds[j] == bauds[slv_baud])
                            {
                            CommPort->SetModemSpeed(j);
                            break;
                            }
                        }

                    slv_door = TRUE;
                    break;
                    }
#endif

                case 'C':
                    {
                    unlink(etcTab);

                    unlink(lgTab);
                    unlink("msg.tab");  // mgTab
                    unlink(rmTab);
                    unlink(cronTab);

                    reconfig = TRUE;
                    break;
                    }

                case 'D':
                    {
                    cfg.bios = 0;
                    break;
                    }

                case 'E':
                    {
                    readconfigcit = TRUE;
                    break;
                    }

                case 'F':
                    {
                    if (argv[i][2])
                        {
                        FilesRequired = atoi(argv[i] + 2);
                        }
                    else
                        {
                        i++;
                        FilesRequired = atoi(argv[i]);
                        }

                    break;
                    }

#ifndef WINCIT
                case 'G':
                    {
                    if (argv[i][2])
                        {
                        changedir(argv[i] + 2);
                        }

                    generateHelpFiles();
                    DeinitializeTimer();
                    critical(FALSE);
                    exit(0);
                    break;  // useless, but it looks nifty.
                    }

                case 'M':
                    {
                    conMode = atoi(argv[i]+2);
                    break;
                    }
#endif

                case 'N':
                    {
                    if (toupper(argv[i][2]) == 'B')
                        {
                        cmd_nobells = TRUE;
                        }

                    if (toupper(argv[i][2]) == 'C')
                        {
                        cmd_nochat = TRUE;
                        }

                    break;
                    }

#ifndef WINCIT
                case 'O':
                    {
                    if (!read_cfg_messages())
                        {
                        printf(getmsg(188), getmsg(460));
                        DeinitializeTimer();
                        critical(FALSE);
                        exit(0);
                        }

                    initExtDrivers();

                    dump_cfg_messages();
                    compactMemory(1);

                    if (!FullScreenConfig())
                        {
                        DeinitializeTimer();
                        critical(FALSE);
                        exit(0);
                        }

                    break;
                    }
#endif

                // log in user with password
                case 'P':
                    {
                    login_pw = TRUE;
                    login_user = FALSE;

                    if (argv[i][2])
                        {
                        CopyStringToBuffer(cmd_login, argv[i] + 2);
                        }
                    else
                        {
                        i++;
                        CopyStringToBuffer(cmd_login, argv[i]);
                        }

                    break;
                    }

                // Run script
                case 'S':
                    {
                    if (argv[i][2])
                        {
                        CopyStringToBuffer(cmd_script, argv[i] + 2);
                        }
                    else
                        {
                        i++;
                        CopyStringToBuffer(cmd_script, argv[i]);
                        }

                    break;
                    }

                // log in user with password. Actually, isn't this by name?
                case 'U':
                    {
                    if (!login_pw)
                        {
                        login_user = TRUE;

                        if (argv[i][2])
                            {
                            CopyStringToBuffer(cmd_login, argv[i] + 2);
                            }
                        else
                            {
                            i++;
                            CopyStringToBuffer(cmd_login, argv[i]);
                            }
                        }

                    break;
                    }

                // debug (verbose) mode
                case 'V':
                    {
                    TurnDebugOn();
                    break;
                    }

                case 'X':
                    {
                    batchmode = TRUE;
                    break;
                    }

                case '#':
                    {
                    cmd_mdata = atoi(argv[i]+2);
                    break;
                    }

                case '!':
                    {
                    slv_net = TRUE;

                    if (argv[i][2])
                        {
                        strcpy(slv_node, argv[i]+2);
                        }
                    else
                        {
                        i++;
                        strcpy(slv_node, argv[i]);
                        }

                    break;
                    }

                default:
#ifdef WINCIT
                    char Buffer[128];
                    sprintf(Buffer, getmsg(MSG_CMDLINE, 2), argv[i]);
                    MessageBox(NULL, Buffer, NULL, MB_ICONSTOP | MB_OK);
#else
                    printf(bn);
                    printf(getmsg(MSG_CMDLINE, 2), argv[i]);
                    printf(bn);
#endif
                    // Fall through...

                case '?':
                case 'H':
                    {
#ifdef WINCIT
                    BOOL Alloced = AllocConsole();

                    if (Alloced)
                        {
                        HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);

                        if (oh != INVALID_HANDLE_VALUE)
                            {
                            COORD size;
                            size.X = 80;
                            size.Y = 26;
                            SetConsoleScreenBufferSize(oh, size);

                            SMALL_RECT rect;
                            rect.Left = rect.Top = 0;
                            rect.Bottom = 25;
                            rect.Right = 79;
                            SetConsoleWindowInfo(oh, TRUE, &rect);

                            DWORD scratch;

                            for (int i = 9; i < 33; i++)
                                {
                                char Buffer[128];
                                CopyString2Buffer(Buffer,
                                        getmsg(MSG_CMDLINE, i),
                                        sizeof(Buffer) - 1);
                                strcat(Buffer, bn);

                                WriteConsole(oh, Buffer, strlen(Buffer),
                                        &scratch, NULL);
                                }

                            WriteConsole(oh, bn, 1, &scratch, NULL);
                            WriteConsole(oh, getmsg(MSG_CMDLINE, 45),
                                    strlen(getmsg(MSG_CMDLINE, 45)),
                                    &scratch, NULL);

                            HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);

                            if (ih != INVALID_HANDLE_VALUE)
                                {
                                SetConsoleMode(ih, 0);

                                char Wow;
                                ReadConsole(ih, &Wow, 1, &scratch, NULL);
                                }
                            }

                        FreeConsole();
                        }
                    else
                        {
                        MessageBox(NULL, getmsg(MSG_CMDLINE, 44), NULL,
                                MB_ICONSTOP | MB_OK);
                        }
#else
                    puts(ns);

                    for (int i = 9; i < 44; i++)
                        {
                        puts(getmsg(MSG_CMDLINE, i));
                        }

                    DeinitializeTimer();
                    critical(FALSE);
#endif
                    exit(200);
                    }
                }
            }
        }

    UnlockMessages(MSG_CMDLINE);
    }


void TERMWINDOWMEMBER securityViolation(const char *pathname)
    {
    SetDoWhat(SECURITYVIOLATION);

    CurrentUser->SetVerified(FALSE);

    label UserName;
    CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
    trap(T_HACK, WindowCaption, getmsg(667), pathname, UserName);
#else
    trap(T_HACK, getmsg(667), pathname, UserName);
#endif

    Message *Msg = new Message;

    if (Msg)
        {
        char MsgStr[256];
        sprintf(MsgStr, getmsg(452), pathname, UserName, cfg.Uuser_nym);
        Msg->SetText(MsgStr);
        Msg->SetRoomNumber(AIDEROOM);
        systemMessage(Msg);
        delete Msg;
        }
    else
        {
        OutOfMemory(8);
        }

    CRmPrintfCR(getmsg(680));

    CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

    Hangup();

    if (w)
        {
        destroyCitWindow(w, FALSE);
        }
    }

const char *mkDblBck(const char *str, char *newstr)
    {
    char *wow = newstr;

    for (; *str; str++)
        {
        if (*str == '\\')
            {
            *(newstr++) = '\\';
            *newstr = '\\';
            }
        else if (*str == '\n')
            {
            *(newstr++) = '\\';
            *newstr = 'n';
            }
        else if (*str == '"')
            {
            *(newstr++) = '\\';
            *newstr = '"';
            }
        else if (*str == '\t')
            {
            *(newstr++) = '\\';
            *newstr = 't';
            }
        else if (*str == '\r')
            {
            *(newstr++) = '\\';
            *newstr = 'r';
            }
        else if (*str == '\f')
            {
            *(newstr++) = '\\';
            *newstr = 'f';
            }
        else if (*str == '\b')
            {
            *(newstr++) = '\\';
            *newstr = 'b';
            }
        else if (*str == '^')
            {
            *(newstr++) = '\\';
            *newstr = '^';
            }
        else if (*str == 127)
            {
            *(newstr++) = '^';
            *newstr = '?';
            }
        else
            {
            if (*str < 32 && *str > 0)
                {
                *(newstr++) = '^';
                *newstr = (char) (*str + '@');
                }
            else
                {
                *newstr = *str;
                }
            }

        newstr++;
        }

    *newstr = 0;
    return (wow);
    }

Bool TurnDebugOn(void)
    {
    if (!debug)
        {
        debug = read_db_messages();
        }

    return (debug);
    }

Bool TurnDebugOff(void)
    {
    if (debug)
        {
        debug = FALSE;
        dump_db_messages();
        }

    return (!debug);
    }

void TERMWINDOWMEMBER UserHasTimedOut(void)
    {
    time(&LastActiveTime);

    OC.Echo = BOTH;
    OC.setio();

    int i;

    if (cfg.sleepcount)
        {
        putWord((uchar *) getmsg(648));

        int i2;
        long t;

        time(&t);
        i2 = -1;

        do
            {
            i = cfg.sleepcount - (int) (time(NULL) - t);

            if (i != i2)
                {
                if (cfg.countbeep)
                    {
                    mPrintf("\a");
                    }

                mPrintf(getmsg(381), i);
                i2 = i;
                }

            if (BBSCharReady())
                {
                iCharNE();
                break;
                }
            } while (i > 0);
        }
    else
        {
        i = 0;
        }

    if (i <= 0)
        {
        char tmp[80];
        sprintf(tmp, pcts, cfg.sleepprompt);
        mFormat(tmp);
        doCR();

        if (onConsole)
            {
            OC.whichIO = MODEM;
            OC.setio();
            }

        CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
                CitWindowsMsg(NULL, getmsg(82));

        Hangup();

        if (w)
            {
            destroyCitWindow(w, FALSE);
            }

#ifdef WINCIT
        // onConsole will always be TRUE when connected locally in WINCIT;
        // OC.whichIO is ignored. Setting ExitToMsdos forces the logout,
        // because it makes HaveConnectionToUser() return TRUE.
        if (onConsole)
            {
            ExitToMsdos = TRUE;
            }
#endif
        }
    }

struct RomanInputTableS
    {
    char Digit;
    short Value;
    };

discardable *RomanInputData;

static Bool GetNextRomanDigit(const char *Roman, int *Pos, long *Value)
    {
    if (Roman[*Pos])
        {
        const RomanInputTableS *RomanInputTable = (const RomanInputTableS *)
                RomanInputData->data;

        long Multiplier = 1;

        if (Roman[*Pos] == '=')
            {
            Multiplier = 1000000l;
            (*Pos)++;
            }
        else if (Roman[*Pos] == '-')
            {
            Multiplier = 1000l;
            (*Pos)++;
            }

        for (int i = 0; i < 7; i++)
            {
            if (RomanInputTable[i].Digit == toupper(Roman[*Pos]))
                {
                (*Pos)++;
                *Value = RomanInputTable[i].Value * Multiplier;
                return (TRUE);
                }
            }
        }

    *Value = 0;
    return (FALSE);
    }

long RomanInput(const char *Roman)
    {
    if (!RomanInputData)
        {
        RomanInputData = readData(6, 3, 3);

        if (!RomanInputData)
            {
            OutOfMemory(9);
            return (0);
            }
        }

    int Pos = 0;
    long Value, Total = 0;

    while (GetNextRomanDigit(Roman, &Pos, &Value))
        {
        int CopyPos = Pos;

        long Value2;
        GetNextRomanDigit(Roman, &CopyPos, &Value2);

        if (Value < Value2)
            {
            Total -= Value;
            }
        else
            {
            Total += Value;
            }
        }

    return (Total);
    }

struct RomanOutputTableS
    {
    long Breakoff;
    char Char1;
    char Char2;
    long Offset;
    };

discardable *RomanOutputData;

const char *RomanOutput(long Number)
    {
    if (!RomanOutputData)
        {
        RomanOutputData = readData(6, 4, 4);

        if (!RomanOutputData)
            {
            OutOfMemory(10);
            return (ns);
            }
        }

    // The longest number created with roman numerals is 1,888,888,888
    // (=M=D=C=C=C=L=X=X=X=V-M-M-M-D-C-C-C-L-X-X-X-VMMMDCCCLXXXVIII),
    // which is fifty-nine characters long. Add "Negative " and you get
    // sixty-eight. Add a few for translations of "Negative " and you get
    // the 79 that we are allowing ourselves.

    static char THREAD Result[80];
    int Pos;
    const RomanOutputTableS *RomanOutputTable = (const RomanOutputTableS *)
            RomanOutputData->data;

    if (Number < 0)
        {
        Number = -Number;

        strcpy(Result, getmsg(7));
        Pos = strlen(Result);
        }
    else
        {
        Pos = 0;
        }

    while (Number > 0)
        {
        int i;
        for (i = 0; Number < RomanOutputTable[i].Breakoff; i++);

        Result[Pos++] = RomanOutputTable[i].Char1;
        if (RomanOutputTable[i].Char2)
            {
            Result[Pos++] = RomanOutputTable[i].Char2;
            }

        Number += RomanOutputTable[i].Offset;
        }

    if (!Pos)
        {
        CopyStringToBuffer(Result, getmsg(6));
        }
    else
        {
        Result[Pos] = 0;
        }

    return (Result);
    }

void OutOfMemory(int Code)
    {
#ifndef WINCIT
    #if VERSION == RELEASE
        Label buffer;
        itoa(Code, buffer, 10);

        CRmPrintfCR(getmsg(188), buffer);
    #else
        CRmPrintfCR(getmsg(188), getmsg(MSG_OOM, Code));
    #endif
#endif
    }

void cOutOfMemory(int Code)
    {
#ifndef WINCIT
    doccr();

#if VERSION == RELEASE
    Label buffer;
    itoa(Code, buffer, 10);

    cPrintf(getmsg(188), buffer);
#else
    cPrintf(getmsg(188), getmsg(MSG_OOM, Code));
#endif
    doccr();
#endif
    }

#ifndef WINCIT
typedef struct
    {
    void *ptr;
    char srch[80];
    } memoryDumpData;

#define nb(a) (a && (a != '\t') ? a : ' ')      // don't want tab expansion

enum
    {
    CTRL_GETADDR,   CTRL_SEARCH,
    };

static Bool mdHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
    {
    switch (evt)
        {
        case EVT_DRAWINT:
            {
            if (buildClipArray(wnd))
                {
                memoryDumpData *md = (memoryDumpData *) wnd->LocalData;
                SRECT r = wnd->extents;
                int i, j;
                uint seg, ofs;
                seg = FP_SEG(md->ptr);
                ofs = FP_OFF(md->ptr);

                for (i = 0, j = r.top + 1; j < r.bottom; i++, j++)
                    {
                    char str[128];
                    uchar val[16];
                    int x;

                    for (x = 0; x < 16; x++)
                        {
                        val[x] = *((uchar *) MK_FP(seg, ofs++));
                        }

                    sprintf(str, getmsg(129),
                            MK_FP(seg, ofs - 16),
                            val[0], val[1], val[2], val[3],
                            val[4], val[5], val[6], val[7],
                            val[8], val[9], val[10], val[11],
                            val[12], val[13], val[14], val[15],
                            nb(val[0]), nb(val[1]), nb(val[2]), nb(val[3]),
                            nb(val[4]), nb(val[5]), nb(val[6]), nb(val[7]),
                            nb(val[8]), nb(val[9]), nb(val[10]), nb(val[11]),
                            nb(val[12]), nb(val[13]), nb(val[14]), nb(val[15])
                            );

                    CitWindowOutStr(wnd, 1, i + 1, str, cfg.attr);

                    for (x = strlen(str) + 1; x < r.right - r.left; x++)
                        {
                        CitWindowOutChr(wnd, x, i + 1, ' ', cfg.attr);
                        }
                    }

                freeClipArray();
                }

            break;
            }

        case EVT_CTRLRET:
            {
            CONTROLINFO *ci = (CONTROLINFO *) param;

            switch (ci->id)
                {
                case CTRL_GETADDR:
                    {
                    memoryDumpData *md = (memoryDumpData *) wnd->LocalData;
                    label tmp;
                    uint seg, ofs;

                    strcpy(tmp, (char *) ci->ptr);

                    if (strchr(tmp, ':'))
                        {
                        *strchr(tmp, ':') = 0;
                        }

                    seg = (int) strtol(tmp, NULL, 16);

                    if (strchr((char *) ci->ptr, ':'))
                        {
                        strcpy(tmp, strchr((char *) ci->ptr, ':') + 1);
                        ofs = (int) strtol(tmp, NULL, 16);
                        }
                    else
                        {
                        ofs = 0;
                        }

                    md->ptr = MK_FP(seg, ofs);
                    break;
                    }

                case CTRL_SEARCH:
                    {
                    char *srch = (char *) ci->ptr;
                    uint rseg, rofs;

                    rseg = FP_SEG(srch) + (FP_OFF(srch) >> 4);
                    rofs = FP_OFF(srch) & 0x0f;
                    srch = (char *) MK_FP(rseg, rofs);

                    if (*srch)
                        {
                        memoryDumpData *md = (memoryDumpData *) wnd->LocalData;
                        uint seg, offstart = FP_OFF(md->ptr) + 1;
                        uint len = strlen(srch);
                        Bool found = FALSE;
                        CITWINDOW *w;

                        w = CitWindowsMsg(wnd, getmsg(128));

                        for (seg = FP_SEG(md->ptr); !found && seg <= 0xf000;
                                seg += 0x1000)
                            {
                            uint ofs;

                            for (ofs = offstart; !found && ofs < 0xffff; ofs++)
                                {
                                char *m, *s;
                                uint i;
                                rseg = seg + (ofs >> 4);
                                rofs = ofs & 0x0f;

                                m = (char *) MK_FP(rseg, rofs);
                                s = srch;

                                if (m != s)     // don't look at search string
                                    {
                                    for (i = 0; i < len; i++)
                                        {
                                        if (toupper(*(s++)) != toupper(*(m++)))
                                            {
                                            break;
                                            }
                                        }

                                    if (i == len)
                                        {
                                        // found it...
                                        md->ptr = MK_FP(rseg, rofs);
                                        found = TRUE;
                                        }
                                    }
                                }

                            offstart = 0;

                            if (seg == 0xf000)
                                {
                                break;
                                }
                            }

                        if (w)
                            {
                            destroyCitWindow(w, FALSE);
                            }

                        if (found)
                            {
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            }
                        else
                            {
                            CitWindowsError(wnd, getmsg(127));
                            }

                        strcpy(md->srch, srch);
                        }

                    break;
                    }
                }

            break;
            }

        case EVT_INKEY:
            {
            memoryDumpData *md = (memoryDumpData *) wnd->LocalData;

            if (md)
                {
                if (param < 256)
                    {
                    switch (toupper((int)param))
                        {
                        case 'G':       // goto
                            {
                            char df[12];

                            sprintf(df, getmsg(126), md->ptr);

                            CitWindowsGetString(getmsg(125), 11,
                                    df, wnd, CTRL_GETADDR, NULL, FALSE);

                            return (TRUE);
                            }

                        case 'R':       // refresh
                            {
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case 'S':       // search
                            {
                            memoryDumpData *md = (memoryDumpData *) wnd->LocalData;

                            CitWindowsGetString(getmsg(124), 80,
                                    md->srch, wnd, CTRL_SEARCH, NULL, FALSE);

                            return (TRUE);
                            }
                        }
                    }
                else
                    {
                    switch (param >> 8)
                        {
                        case CTL_CURS_HOME:
                            {
                            md->ptr = NULL;
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_UP:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr),
                                    FP_OFF(md->ptr) - 16);
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_DOWN:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr),
                                    FP_OFF(md->ptr) + 16);
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_LEFT:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr),
                                    FP_OFF(md->ptr) - 1);
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_RIGHT:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr),
                                    FP_OFF(md->ptr) + 1);
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case PGUP:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr) - 1,
                                    FP_OFF(md->ptr));
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case PGDN:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr) + 1,
                                    FP_OFF(md->ptr));
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_HOME:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr) - 16,
                                    FP_OFF(md->ptr));
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }

                        case CURS_END:
                            {
                            md->ptr = MK_FP(FP_SEG(md->ptr) + 16,
                                    FP_OFF(md->ptr));
                            (wnd->func)(EVT_DRAWINT, 0, 0, wnd);
                            return (TRUE);
                            }
                        }
                    }
                }

            return (defaultHandler(evt, param, more, wnd));
            }

        default:
            {
            return (defaultHandler(evt, param, more, wnd));
            }
        }

    return (TRUE);
    }

void memoryDump(void)
    {
    memoryDumpData *ld;

    if ((ld = new memoryDumpData) != NULL)
        {
        CITWINDOW *w;
        WINDOWFLAGS flags;
        SRECT rect;

        memset(&flags, 0, sizeof(flags));

        flags.visible = TRUE;
        flags.showTitle = TRUE;
        flags.moveable = TRUE;
        flags.minimize = TRUE;
        flags.maximize = TRUE;
        flags.resize = TRUE;
        flags.close = TRUE;

        rect.top = 0;
        rect.left = 0;
        rect.bottom = scrollpos / 2;
        rect.right = conCols - 1;

        ld->ptr = NULL;
        *ld->srch = 0;

        if ((w = makeCitWindow(mdHandler, NULL, getmsg(123), flags,
                rect, ld, FALSE)) != NULL)
            {
            setFocus(w);
            }
        }
    }
#else
void memoryDump(void)
    {
    }
#endif

Bool GetBooleanFromCfgFile(const char *Setting, Bool *Error)
    {
    *Error = FALSE;

    if (SameString(getmsg(627), Setting) || SameString(getmsg(522), Setting) || SameString(getmsg(316), Setting) ||
            SameString(getmsg(688), Setting) || SameString(getmsg(690), Setting))
        {
        return (FALSE);
        }
    else if (SameString(getmsg(626), Setting) || SameString(getmsg(521), Setting) || SameString(getmsg(317), Setting) ||
            SameString(getmsg(687), Setting) || SameString(getmsg(689), Setting))
        {
        return (TRUE);
        }
    else if (isNumeric(Setting))
        {
        return (!!atoi(Setting));
        }
    else
        {
        *Error = TRUE;
        return (FALSE);
        }
    }

#ifdef WINCIT
Bool TERMWINDOWMEMBER SetTitle(void)
    {
    char Buffer[128];
    sprintf(Buffer, "%s, %s - %s", cfg.nodeTitle, programName, WindowCaption);

    while (hConsoleOutput == NULL);
    return (SetConsoleTitle(Buffer));
    }
#endif
