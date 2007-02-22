// --------------------------------------------------------------------------
// Citadel: Net.CPP
//
// General-purpse networking code; not specific to any one protocol.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "filecmd.h"
#include "cwindows.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"
#include "filerdwr.h"
#include "carrier.h"


// --------------------------------------------------------------------------
// Contents
//
// net_slave()      network entry point from LOGIN
// net_master()     entry point to call a node
// n_dial()         call the bbs in the node buffer
// n_login()        Login to the bbs with the macro in the node file
// wait_for()       wait for a string to come from the modem
// net_callout()    Entry point from CRON.CPP

// In Net1.CPP
Bool Net1Slave(void);
Bool Net1Master(void);
void Net1Cleanup(void);

Bool NetMessagePacketC::SaveByte(char Byte)
    {
    assert(NetFile);
    return (fputc(Byte, NetFile) != EOF);
    }

Bool NetMessagePacketC::SaveString(const char *String)
    {
    assert(NetFile);
    assert(String);

    return (PutStr(NetFile, String));
    }

Bool NetMessagePacketC::GetByte(char *Byte)
    {
    assert(NetFile);

    if (feof(NetFile))
        {
        *Byte = 0;
        return (FALSE);
        }
    else
        {
        *Byte = (char) fgetc(NetFile);
        return (TRUE);
        }
    }

// --------------------------------------------------------------------------
// net_slave(): network entry point from LOGIN
//
// Return value:
//  TRUE if worked, FALSE if not.

Bool TERMWINDOWMEMBER net_slave(void)
    {
    callout = TRUE;

    if (!read_net_messages())
        {
        cPrintf(getmsg(59));
        callout = FALSE;
        return (FALSE);
        }

    if (!readnode(TRUE))
        {
        cOutOfMemory(112);
        callout = FALSE;
        dump_net_messages();
        return (FALSE);
        }

    label Buffer, Buffer1;
    if (!*CurrentUser->GetAlias(Buffer, sizeof(Buffer)) ||
            !*CurrentUser->GetLocID(Buffer1, sizeof(Buffer1)))
        {
        dump_net_messages();
        callout = FALSE;
        return (FALSE);
        }

    if (!onConsole)
        {
        netError = FALSE;

        // cleanup
        changedir(LocalTempPath);
        ambigUnlink(getnetmsg(162));
        ambigUnlink(getnetmsg(163));
        unlink(roomreqIn);
        unlink(roomreqOut);
        unlink(roomdataIn);
        unlink(roomdataOut);
        unlink(getnetmsg(164));
        unlink(getnetmsg(165));

        switch (node->GetNetworkType())
            {
            default:
            case NET_DCIT10:
                {
                Net1Slave();
                if (Net1Master())
                    {
                    Net1Cleanup();
                    buildaddress(CONSOLE);
                    did_net(node->GetName());
                    }
                else
                    {
                    dump_net_messages();
                    callout = FALSE;
                    return (FALSE);
                    }

                break;
                }

            case NET_DCIT15:
            case NET_DCIT16:
                {
                if (dc15network(FALSE))
                    {
                    Net1Cleanup();
                    buildaddress(CONSOLE);
                    did_net(node->GetName());
                    }
                else
                    {
                    dump_net_messages();
                    callout = FALSE;
                    return (FALSE);
                    }

                break;
                }

            case NET_6_9:
                {
                if (net69(FALSE))
                    {
                    did_net(node->GetName());
                    }
                else
                    {
                    dump_net_messages();
                    callout = FALSE;
                    return (FALSE);
                    }

                break;
                }

            case NET_C86:
                {
                if (!Citadel86Network(FALSE))
                    {
                    dump_net_messages();
                    callout = FALSE;
                    return (FALSE);
                    }

                did_net(node->GetName());
                break;
                }

#ifdef HENGE
            case NET_HENGE:
                {
                if (!hengeNet(FALSE))
                    {
                    dump_net_messages();
                    callout = FALSE;
                    return (FALSE);
                    }

                break;
                }
#endif
            }
        }
    else
        {
        dump_net_messages();
        callout = FALSE;
        return (FALSE);
        }

    dump_net_messages();
    callout = FALSE;
    return (TRUE);
    }

// --------------------------------------------------------------------------
// net_master(): Entry point to call a node
//
// Return value:
//  TRUE: Network worked.
//  FALSE: It didn't.

Bool TERMWINDOWMEMBER net_master(void)
    {
    int tries = 1;

    if (!readnode(FALSE))
        {
        doccr();
        label Buffer;
        cPrintf(getmsg(206), CurrentUser->GetName(Buffer, sizeof(Buffer)));
        doccr();
        return (FALSE);
        }

    // cleanup
    changedir(LocalTempPath);
    ambigUnlink(getnetmsg(162));
    ambigUnlink(getnetmsg(163));
    unlink(roomreqIn);
    unlink(roomreqOut);
    unlink(roomdataIn);
    unlink(roomdataOut);
    unlink(getnetmsg(164));
    unlink(getnetmsg(165));

    int connected;

#ifdef WINCIT
    if (*node->GetIpAddress())
        {
            connected = TRUE;

        }
    else
#endif
    do
        {
        if (tries > 1)
            {
            doccr();
            cPrintf(getnetmsg(130), ltoac(tries));
            }

        if ((connected = n_dial()) == CERROR)
            {
            return (FALSE);
            }
        } while (tries++ < node->GetRedial() && !connected);

    if (!connected)
        {
        return (FALSE);
        }

    if (node->GetNetworkType() == NET_C86)
        {
        if (!net86_fetch(CONSOLE))
            {
            return (FALSE);
            }
        }

    if (!n_login())
        {
        return (FALSE);
        }

    // We have logged in: Call it a new call.
#ifdef WINCIT
    slideLTab(&ThisSlot, CurrentUser, LogOrder);
#else
    slideLTab(&ThisSlot, CurrentUser);
#endif
    cfg.callno++;

    netError = FALSE;

    switch (node->GetNetworkType())
        {
        default:
        case NET_DCIT10:
            {
            if (Net1Master() == FALSE)
                {
                return (FALSE);
                }

            if (Net1Slave() == FALSE)
                {
                return (FALSE);
                }

            Net1Cleanup();
            buildaddress(CONSOLE);
            break;
            }

        case NET_DCIT15:
        case NET_DCIT16:
            {
            if (!dc15network(TRUE))
                {
                return (FALSE);
                }

            Net1Cleanup();
            buildaddress(CONSOLE);
            break;
            }

        case NET_6_9:
            {
            if (!net69(TRUE))
                {
                return (FALSE);
                }

            break;
            }

        case NET_C86:
            {
            if (!Citadel86Network(TRUE))
                {
                return (FALSE);
                }

            break;
            }

#ifdef HENGE
        case NET_HENGE:
            {
            if (!hengeNet(TRUE))
                {
                return (FALSE);
                }

            break;
            }
#endif
        }

    CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
            CitWindowsMsg(NULL, getmsg(19));

    Initport();

    if (w)
        {
        destroyCitWindow(w, FALSE);
        }

    return (TRUE);
    }


// --------------------------------------------------------------------------
// wait_for(): Wait for a string to come from the modem
//
// Input:
//  const char *str: The string to wait for
//  int timeout: The number of seconds to wait

Bool TERMWINDOWMEMBER wait_for(const char *str, int timeout)
    {
    char line[80];
    long st;
    int i, stl;

    stl = strlen(str);

    if (stl)
        {
        for (i = 0; i < stl; i++)
            {
            line[i] = '\0';
            }

        time(&st);

        for (;;)
            {
            if (CommPort->IsInputReady())
                {
                memcpy(line, line+1, stl);
                line[stl-1] = (char) CommPort->Input();
                line[stl] = '\0';

                if (debug)
                    {
                    outCon(line[stl-1]);
                    }

                if (SameString(line, str))
                    {
                    return (TRUE);
                    }
                }
            else
                {
                if ((time(NULL) - st) > (long)timeout ||
                        !CommPort->HaveConnection())
                    {
                    return (FALSE);
                    }

                if (KBReady())  // Aborted by user
                    {
                    ciChar();
                    return (FALSE);
                    }
                }
            }
        }
    else
        {
        return (FALSE);
        }
    }

// --------------------------------------------------------------------------
// net_callout(): Entry point from RUNCRON.CPP
//
// Input:
//  const char *nodenm: Name of node to net with.
//
// Return value:
//  TRUE: It worked.
//  FALSE: It didn't work.

Bool TERMWINDOWMEMBER net_callout(const char *nodenm)
    {
    Bool NetResult = FALSE;
    char String[80];

    if (!read_net_messages())
        {
        cPrintf(getmsg(59));
        return (FALSE);
        }

    SetDoWhat(NETWORKING);

    strftime(String, sizeof(String) - 1, cfg.vdatestamp, 0l);
    cPrintf(getnetmsg(170), String);
    doccr();

#ifdef WINCIT
    if (!ReadNodesCit(this, &node, nodenm, MODEM, FALSE))
#else
    if (!ReadNodesCit(&node, nodenm, MODEM, FALSE))
#endif
        {
        cPrintf(getmsg(206), nodenm);
        doccr();
        dump_net_messages();
        return (FALSE);
        }

    // login user
    MS.Read = 0;
    MS.Entered = 0;

    const l_slot slot = FindPersonByName(node->GetName());

    if (slot == CERROR)
        {
        cPrintf(getnetmsg(13));
        dump_net_messages();
        return (FALSE);
        }

    ThisSlot = slot;
    ThisLog = LTab(slot).GetLogIndex();
    loggedIn = TRUE;

    label Buffer;
    if (CurrentUser->Load(ThisLog))
        {
        setlbvisit();
        StatusLine.Update(WC_TWp);

        callout = TRUE;

#ifdef WINCIT
        trap(T_NETWORK, WindowCaption, getnetmsg(14), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
        trap(T_NETWORK, getnetmsg(14), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

        // node logged in
        NetResult = net_master();

        // be sure to hang up
        if (CommPort->HaveConnection())
            {
            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }
            }

        // terminate user
        if (NetResult == TRUE)
            {
            CurrentUser->SetCallNumber(cfg.callno);
            time(&logtimestamp);
            CurrentUser->SetCallTime(logtimestamp);

            CurrentUser->SetLastMessage(MessageDat.NewestMessage());

            storeLog();

            loggedIn = FALSE;

            // trap it
#ifdef WINCIT
            trap(T_NETWORK, WindowCaption, getnetmsg(15), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
            trap(T_NETWORK, getnetmsg(15), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif


            OC.SetOutFlag(IMPERVIOUS);

            cPrintf(getnetmsg(16), CurrentUser->GetName(Buffer, sizeof(Buffer)));

            delete MS.AbortedMessage;
            MS.AbortedMessage = NULL;

#ifndef WINCIT
            trap(T_NETWORK, getnetmsg(17), MS.Entered);
            trap(T_NETWORK, getnetmsg(18), MS.Read);
            trap(T_NETWORK, getnetmsg(19), MS.Expired);
            trap(T_NETWORK, getnetmsg(20), MS.Duplicate);
#else
            trap(T_NETWORK, WindowCaption, getnetmsg(17), MS.Entered);
            trap(T_NETWORK, WindowCaption, getnetmsg(18), MS.Read);
            trap(T_NETWORK, WindowCaption, getnetmsg(19), MS.Expired);
            trap(T_NETWORK, WindowCaption, getnetmsg(20), MS.Duplicate);
#endif

            }
        else
            {
            loggedIn = FALSE;

            Bool SaveTrapit = cfg.trapit[T_NETWORK];

            if (node->GetNetFail() > 0)
                {
                cfg.trapit[T_NETWORK] = TRUE;
                }

            if (node->GetNetFail() < 0)
                {
                cfg.trapit[T_NETWORK] = FALSE;
                }

#ifdef WINCIT
            trap(T_NETWORK, WindowCaption, getnetmsg(21), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
            trap(T_NETWORK, getnetmsg(21), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif



            cfg.trapit[T_NETWORK] = SaveTrapit;
            }
        }
    else
        {
        loggedIn = FALSE;
        }

    setdefaultconfig(FALSE);

    // user terminated
    callout = FALSE;

    CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

    Initport();

    if (w)
        {
        destroyCitWindow(w, FALSE);
        }

    freeNode(&node);

    dump_net_messages();
    return (NetResult);
    }


// --------------------------------------------------------------------------
// n_dial(): Call the bbs in the node buffer
//
// Return value:
//  TRUE: Connected
//  FALSE: Didn't connect; retry if #REDIAL set in NODES.CIT
//  CERROR: User abort; don't retry

int TERMWINDOWMEMBER n_dial(void)
    {
    int checkbaud;

    doccr();

    CommPort->SetSpeed(node->GetBaud());
    StatusLine.Update(WC_TWp);

    CommPort->OutString(cfg.dialsetup);
    CommPort->OutString(br);

    if (*node->GetPreDial())
        {
        if (debug)
            {
            cPrintf(getdbmsg(3), node->GetPreDial());
            doccr();
            }

        CommPort->OutString(node->GetPreDial());
        CommPort->OutString(br);
        }

    if (debug)
        {
        cPrintf(getdbmsg(4), cfg.dialpref, node->GetDialOut());
        }
    else
        {
        cPrintf(getnetmsg(40));
        }

    pause(100);

    CommPort->FlushInput();

    checkbaud = dial(node->GetDialOut(), node->GetDialTimeout());

    if (checkbaud && (cfg.dumbmodem == 4 || cfg.dumbmodem == 5 ||
            cfg.dumbmodem == 6 || cfg.dumbmodem == 7))
        {
        CommPort->SetSpeed(node->GetBaud());
        }

    if (checkbaud == TRUE)
        {
        pause(100);
        }

    StatusLine.Update(WC_TWp);

    if (checkbaud != FALSE) // TRUE || CERROR
        {
#ifdef WINCIT
        trap(T_NETWORK, WindowCaption, getnetmsg(44), modem_result);
#else
        trap(T_NETWORK, getnetmsg(44), modem_result);
#endif
        }

    switch (checkbaud)
        {
        case TRUE:
            {
            OC.ansiattr = cfg.wattr;
            cPrintf(getnetmsg(45));
            OC.ansiattr = cfg.attr;
            cPrintf(getnetmsg(170), modem_result);
            return (TRUE);
            }

        case CERROR:
            {
            OC.ansiattr = cfg.wattr;
            cPrintf(getnetmsg(46));
            OC.ansiattr = cfg.attr;
            cPrintf(getnetmsg(170), modem_result);
            return (FALSE);
            }
        case FALSE:
            {
            return (FALSE);
            }
        default:    // -3
            {
            return (CERROR);
            }
        }

    }


// --------------------------------------------------------------------------
// dial(): Actually make the call.
//
// Input:
//  const char *dialstring: String to send to modem to call
//  int timeout: How long to wait for carrier, in seconds
//
// Return value:
//  Whatever smartbaud() returns, or -3 if manually aborted.

int TERMWINDOWMEMBER dial(const char *dialstring, int timeout)
    {
    time_t ts, tx, ty;
    int checkbaud;

    pause(100);
    CommPort->FlushInput();

    CommPort->OutString(cfg.dialpref);
    CommPort->OutString(dialstring);
    CommPort->OutString(br);

    tx = time(&ts);

    for (;;)
        {
        if ((int) (time(&ty) - ts) > timeout)
            {
            OC.ansiattr = cfg.wattr;
            cPrintf(getnetmsg(41));
            OC.ansiattr = cfg.attr;

            return (FALSE);
            }

        if (tx != ty)
            {
            if (debug)
                {
                cPrintf(br);
                cPrintf(getdbmsg(5), cfg.dialpref, dialstring,
                        ltoac(ty - ts));
                }

            tx = ty;
            }

        if (KBReady())  // User abort
            {
            ciChar();
            OC.ansiattr = cfg.wattr;
            cPrintf(getnetmsg(43));
            OC.ansiattr = cfg.attr;

            return (-3);
            }

        if (CommPort->IsInputReady())
            {
            checkbaud = smartbaud(CommPort);

            if (checkbaud != FALSE) // TRUE || CERROR
                {
                return (checkbaud);
                }
            }
        }
    }


// --------------------------------------------------------------------------
// n_login(): Parse login macro
//
// Input:
//  char *line: Macro for loggin in. Will be modified.
//
// Return value:
//  TRUE if was able to parse and execute
//  FALSE if failed

Bool TERMWINDOWMEMBER parseLine(char *line)
    {
    int i, j, k;
    char *words[128];
    char newline[82];
    char done;

    const int count = parse_it(words, line);

    i = -1;

    while (++i < count)
        {
        switch (tolower(*words[i++]))
            {
            case 'd':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(6), words[i], words[i + 1], words[i + 2]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                const protocols *theProt = GetProtocolByKey(words[i][0], TRUE);

                if (!theProt)
                    {
                    return (FALSE);
                    }

                wxrcv(words[i + 1], words[i + 2], theProt);

                i += 2;
                break;
                }

            case 'p':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(7), words[i]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                netpause(atoi(words[i]) * 100);
                break;
                }

            case 'r':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(8), words[i], words[i+1], words[i+2]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                k = atoi(words[i+2]);

                for (done = FALSE, j = 0; j < k && !done; j++)
                    {
                    if (debug)
                        {
                        OC.ansiattr = cfg.wattr;
                        cPrintf(getdbmsg(9), words[i]);
                        OC.ansiattr = cfg.attr;
                        cPrintf(spc);
                        }
                    else
                        {
                        cPrintf(getmsg(433));
                        }

                    if (!wait_for(words[i], node->GetWaitTimeout()))
                        {
                        if (debug)
                            {
                            OC.ansiattr = cfg.wattr;
                            cPrintf(getdbmsg(10), words[i+1]);
                            OC.ansiattr = cfg.attr;
                            cPrintf(spc);
                            }
                        else
                            {
                            cPrintf(getmsg(433));
                            }

                        CommPort->OutString(words[i + 1], node->GetOutputPace());
                        }
                    else
                        {
                        done = TRUE;
                        }
                    }

                i += 2;

                if (!done)
                    {
                    return (FALSE);
                    }

                break;
                }

            case 's':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(10), words[i]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                CommPort->OutString(words[i], node->GetOutputPace());
                break;
                }

            case 'u':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(11), words[i], words[i + 1],
                            words[i + 2]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                const protocols *theProt = GetProtocolByKey(words[i][0],
                        TRUE);

                if (!theProt)
                    {
                    return (FALSE);
                    }

                wxsnd(words[i + 1], words[i + 2], theProt, 0);

                i += 2;
                break;
                }

            case 'w':
                {
                if (debug)
                    {
                    OC.ansiattr = cfg.wattr;
                    cPrintf(getdbmsg(9), words[i]);
                    OC.ansiattr = cfg.attr;
                    cPrintf(spc);
                    }
                else
                    {
                    cPrintf(getmsg(433));
                    }

                if (!wait_for(words[i], node->GetWaitTimeout()))
                    {
                    return (FALSE);
                    }

                break;
                }

            case '!':
                {
                RunApplication(words[i], NULL, TRUE, TRUE);
                break;
                }

            case '@':
                {
                FILE *file;

                if ((file = fopen(words[i], FO_R)) != NULL)
                    {
                    while (fgets(newline, 80, file))
                        {
                        if (!parseLine(newline))
                            {
                            fclose(file);
                            return (FALSE);
                            }
                        }

                    fclose(file);
                    }
                else
                    {
                    return (FALSE);
                    }

                break;
                }

            default:
                {
                OC.ansiattr = (uchar)(cfg.cattr | 128);
                cPrintf(getnetmsg(52), words[i-1], words[i]);
                OC.ansiattr = cfg.attr;
                cPrintf(spc);
                break;
                }
            }
        }

    return (TRUE);
    }


// --------------------------------------------------------------------------
// n_login(): Login to the bbs with the #LOGIN macro in node buffer
//
// Return value:
//  TRUE if worked
//  FALSE if not

Bool TERMWINDOWMEMBER n_login(void)
    {
    changedir(cfg.homepath);    // for @ command
    doccr();
    cPrintf(getnetmsg(53));

    char Macro[256];

    strcpy(Macro, node->GetLoginMacro());

    if (!parseLine(Macro))
        {
        OC.ansiattr = cfg.wattr;
        cPrintf(getnetmsg(54));
        OC.ansiattr = cfg.attr;

        return (FALSE);
        }

    OC.ansiattr = cfg.wattr;
    cPrintf(getnetmsg(45));
    OC.ansiattr = cfg.attr;
    doccr();
    doccr();

    return (TRUE);
    }
