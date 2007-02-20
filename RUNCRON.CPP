// --------------------------------------------------------------------------
// Citadel: RunCron.CPP
//
// Code to run cron events.

#ifdef WINCIT
#include <winsock.h>
#endif

#include "ctdl.h"
#pragma hdrstop

#ifdef WINCIT
#include "net6969.h"
#else
#include "net.h"
#endif
#include "log.h"
#include "events.h"
#include "cwindows.h"
#include "term.h"
#include "extmsg.h"
#ifdef WINCIT
#include "winser.h"
#include "telnetd.h"
#else
#include "statline.h"
#endif
#include "termwndw.h"


// --------------------------------------------------------------------------
// Contents
//
// do_event         Actualy do this event


// --------------------------------------------------------------------------
// do_event(): Actually do this event.

Bool CronEventC::Do(void)
    {
    if (cfg.fucking_stupid)
        {
        ScreenSaver.Update();
        }

    SetLastTried();

#ifndef WINCIT
    statRecord *theStatus;

    if ((theStatus = addStatus()) != NULL)
        {
        updateStatus = TRUE;

        switch (Type)
            {
            case CR_SHELL_1:
            case CR_SHELL_2:
                {
                sprintf(theStatus->theStatus, getmsg(202), String);
                break;
                }

            case CR_CHAT_OFF:
                {
                sprintf(theStatus->theStatus, getmsg(184));
                break;
                }

            case CR_CHAT_ON:
                {
                sprintf(theStatus->theStatus, getmsg(185));
                break;
                }

            case CR_SHUTDOWN:
                {
                sprintf(theStatus->theStatus, getmsg(186), String);
                break;
                }

            case CR_NET:
                {
                if(read_tr_messages())
                    {
                    sprintf(theStatus->theStatus, gettrmsg(49), String);
                    dump_tr_messages();
                    }
                break;
                }

            case CR_NET69_IN:
                {
                sprintf(theStatus->theStatus, getmsg(203));
                break;
                }

            case CR_NET69_OUT:
                {
                sprintf(theStatus->theStatus, getmsg(207), String);
                break;
                }

            case CR_NET86_IN:
                {
                sprintf(theStatus->theStatus, getmsg(513));
                break;
                }

            case CR_NET86_OUT:
                {
                sprintf(theStatus->theStatus, getmsg(512));
                break;
                }

            case CR_COMMAND:
                {
                sprintf(theStatus->theStatus, getmsg(209), String);
                break;
                }

            default:
                {
                sprintf(theStatus->theStatus, getmsg(210), Type);
                break;
                }
            }

        strcat(theStatus->theStatus, getmsg(371));
        time(&theStatus->theTime);
        StatusLine.Update(WC_TWp);
        }
#endif

    switch (Type)
        {
        case CR_SHELL_1:
        case CR_SHELL_2:
            {
            char trapstr[128];
            sprintf(trapstr, getmsg(202), String);
#ifndef WINCIT
            cPrintf(pcts, trapstr);
            trap(T_CRON, pcts, trapstr);
#else
            trap(T_CRON, "", trapstr);
#endif

            doEvent(EVT_APPLIC);

#ifndef WINCIT
            RunApplication(String, NULL, TRUE, TRUE);

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

            Initport();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }

            SetDone();
#else
            if (QuickRunApp(String) >= 0)
                {
                SetDone();
                }
#endif

            break;
            }

        case CR_CHAT_OFF:
            {
            char trapstr[128];

            CopyStringToBuffer(trapstr, getmsg(184));
#ifndef WINCIT
            cPrintf(trapstr);
#endif
            trap(T_CRON, pcts, trapstr);

            cfg.noChat = TRUE;

            SetDone();
            break;
            }

        case CR_CHAT_ON:
            {
            char trapstr[128];

            CopyStringToBuffer(trapstr, getmsg(185));
#ifndef WINCIT
            cPrintf(trapstr);
#endif
            trap(T_CRON, pcts, trapstr);

            cfg.noChat = FALSE;

            SetDone();
            break;
            }

        case CR_SHUTDOWN:
            {
            char trapstr[128];

            sprintf(trapstr, getmsg(186), String);
            trap(T_CRON, pcts, trapstr);

            return_code = atoi(String);

#ifdef WINCIT
            PostMessage(MainDlgH, WM_COMMAND, MAKEWPARAM(105, BN_CLICKED), 0);
#else
            cPrintf(pcts, trapstr);
            ExitToMsdos = TRUE;
#endif

            SetDone();
            break;
            }

#ifdef WINCIT
        case CR_NET:
            {
            char trapstr[128];

            if(!read_tr_messages())
                {
                errorDisp("%s- %s", getmsg(172), getmsg(59));
                break;
                }

            sprintf(trapstr, gettrmsg(49), String);

            dump_tr_messages();

            trap(T_CRON, "", pcts, trapstr);

            NodesCitC *NodeInfo = NULL;

            if (ReadNodesCit(NULL, &NodeInfo, String, NOMODCON, FALSE))
                {
                const l_slot NodeLogSlot = nodexists(String, NULL);

                if (NodeLogSlot != CERROR)
                    {
                    if (NodeInfo->GetNetworkType() == NET_6969)
                        {
                        Net6969.Network(NodeInfo, NodeLogSlot);
                        }
                    else
                        {
                        if (LoginList.AddUnique(LogTab[NodeLogSlot].GetLogIndex()))
                            {
                            if (*NodeInfo->GetIpAddress())
                                {
                                PHOSTENT phe;
                                SOCKADDR_IN dest_sin;  /* DESTination Socket INternet */
                                SOCKET sock;

                                char ipaddress[128];
                                CopyStringToBuffer(ipaddress, NodeInfo->GetIpAddress());
                                int ipport = NodeInfo->GetIpPort();

                                /////////////////////////////////////////////////////////////////////////////

                                sock = socket(AF_INET, SOCK_STREAM, 0);
                                if (sock == INVALID_SOCKET)
                                    {
                                    errorDisp("CR_NET: socket() failed");
                                    goto here;
                                    }

                                dest_sin.sin_family = AF_INET;

                                phe = gethostbyname(ipaddress);
                                if (phe == NULL)
                                    {
                                    errorDisp("CR_NET: gethostbyname() failed.");
                                    closesocket(sock);
                                    goto here;
                                    }
                                memcpy((char FAR *)&(dest_sin.sin_addr), phe->h_addr, phe->h_length);

                                dest_sin.sin_port = htons(ipport);   /* Convert to network ordering */

                                if (connect(sock, (PSOCKADDR) &dest_sin, sizeof( dest_sin)) < 0)
                                    {
                                    closesocket(sock);
                                    errorDisp("CR_NET: connect() failed");
                                    goto here;
                                    }

                                errorDisp("CR_NET: connect() worked!");

                                //////////////////////////////////////////////////////////////////////////////////////

                                Bool KeepWindow = ViewingTerm;

                                TurnOnTermWindow();

                                if (ViewingTerm)
                                    {
                                    TelnetPortC *TP = new TelnetPortC(sock);

                                    if (TP)
                                        {
                                        sprintf(TP->PortName, "%d.%d.%d.%d", dest_sin.sin_addr.S_un.S_un_b.s_b1,
                                                dest_sin.sin_addr.S_un.S_un_b.s_b2, dest_sin.sin_addr.S_un.S_un_b.s_b3,
                                                dest_sin.sin_addr.S_un.S_un_b.s_b4);

                                        TermWindowC *TW = new TermWindowC(TP, ConnSeq.Next());

                                        if (TW && TW->IsGood())
                                            {
                                            if (!KeepWindow)    // Not quite a good var name...
                                                {
                                                TermWindowCollection.SetFocus(TW);
                                                }

                                            TW->CarrierJustFound();

                                            TW->node = NodeInfo;

                                            TW->OC.Echo = BOTH;
                                            TW->OC.whichIO = CONSOLE;
                                            TW->OC.SetOutFlag(IMPERVIOUS);
                                            TW->modStat = FALSE;
                                            TW->OC.setio();

                                            if (TW->net_callout(String))
                                                {
                                                did_net(String);
                                                SetDone();
                                                }

                                            delete TW;
                                            NodeInfo = NULL;

                                            KeepWindow = TRUE;
                                            }
                                        else
                                            {
                                            delete TW;
                                            }

                                        delete TP;
                                        }

                                    if (!KeepWindow)
                                        {
                                        TurnOffTermWindow();
                                        }
                                    }
                                }
                            else
                                {
                                // Serial net
                                for (SerialPortListS *Cur = SerialPorts; Cur; Cur = (SerialPortListS *) getNextLL(Cur))
                                    {
                                    if (Cur->Port->Own())
                                        {
                                        Bool KeepWindow = ViewingTerm;

                                        TurnOnTermWindow();

                                        if (ViewingTerm)
                                            {
                                            TermWindowC *TW = new TermWindowC(Cur->Port, ConnSeq.Next());

                                            if (TW && TW->IsGood())
                                                {
                                                // Not quite a good var name...
                                                if (!KeepWindow)
                                                    {
                                                    TermWindowCollection.SetFocus(TW);
                                                    }

                                                TW->node = NodeInfo;

                                                TW->OC.Echo = BOTH;
                                                TW->OC.whichIO = CONSOLE;
                                                TW->OC.SetOutFlag(IMPERVIOUS);
                                                TW->modStat = FALSE;
                                                TW->OC.setio();

                                                if (TW->net_callout(String))
                                                    {
                                                    did_net(String);
                                                    SetDone();
                                                    }

                                                delete TW;
                                                NodeInfo = NULL;

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

                                        Cur->Port->Disown();
                                        break;
                                        }
                                    else
                                        {
                                        errorDisp("Could not get serial: %s", String);
                                        }
                                    }
                                }

                            here:

                            LoginList.Remove(LogTab[NodeLogSlot].GetLogIndex());
                            }
                        else
                            {
                            errorDisp("Login not allowed: %s", String);
                            }
                        }
                    }
                else
                    {
                    errorDisp("Node not found: %s", String);
                    }

                delete NodeInfo;
                }
            else
                {
                errorDisp("NODES.CIT not found for %s", String);
                }

            break;
            }
#else
        case CR_NET:
            {
            if (!loggedIn)
                {
                char trapstr[128];

                if(!read_tr_messages())
                    {
                    errorDisp("%s- %s", getmsg(172), getmsg(59));
                    break;
                    }

                sprintf(trapstr, gettrmsg(49), String);
                cPrintf(pcts, trapstr);

                dump_tr_messages();

                trap(T_CRON, pcts, trapstr);

                if (net_callout(String))
                    {
                    did_net(String);
                    SetDone();
                    }

                setdefaultTerm(TT_ANSI);
                }
            else
                {
                mPrintfCR(getmsg(187), cfg.Luser_nym);
                }

            break;
            }

        case CR_NET69_IN:
            {
            if (read_net_messages())
                {
                if (!loggedIn)
                    {
                    char trapstr[128];

                    CopyStringToBuffer(trapstr, getmsg(203));
                    cPrintf(trapstr);
                    doccr();
                    trap(T_CRON, pcts, trapstr);

                    amZap();
                    net69_incorporate(MODEM);
                    SaveAideMess(NULL);

                    SetDone();
                    }
                else
                    {
                    CRmPrintfCR(getmsg(204));
                    }

                dump_net_messages();
                }
            else
                {
                cPrintf(getmsg(59));
                }

            break;
            }

        case CR_NET69_OUT:
            {
            if (read_net_messages())
                {
                if (!loggedIn)
                    {
                    cPrintf(getmsg(205));
                    doccr();

                    label rnode;
                    CopyStringToBuffer(rnode, String);

#ifdef WINCIT
                    if (ReadNodesCit(this, &node, rnode, MODEM, TRUE))
#else
                    if (ReadNodesCit(&node, rnode, MODEM, TRUE))
#endif
                        {
                        if (LoadPersonByName(node->GetName()))
                            {
                            char trapstr[128];
                            sprintf(trapstr, getmsg(207), node->GetName());

                            cPrintf(pcts, trapstr);
                            doccr();
                            trap(T_CRON, pcts, trapstr);

                            LogEntry5 Log5Save(cfg.maxrooms);
                            Log5Save.CopyRoomNewPointers(*CurrentUser);

                            amZap();

                            if (!net69_fetch(TRUE,MODEM))
                                {
                                CurrentUser->CopyRoomNewPointers(Log5Save);
                                }

                            SaveAideMess(NULL);

                            CurrentUser->Save(ThisLog, thisRoom);

                            SetDone();
                            }
                        else
                            {
                            mPrintfCR(getmsg(255), node->GetName());
                            break;
                            }
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmsg(208));
                    }

                dump_net_messages();
                }
            else
                {
                cPrintf(getmsg(59));
                }

            break;
            }

        case CR_NET86_IN:
            {
            if (read_net_messages())
                {
                if (!loggedIn)
                    {
                    label rnode;
                    CopyStringToBuffer(rnode, String);

                    char trapstr[128];
                    CopyStringToBuffer(trapstr, getmsg(513));
                    cPrintf(trapstr);
                    doccr();

                    // Fix this later

#ifdef WINCIT
                    if (ReadNodesCit(this, &node, rnode, MODEM, FALSE))
#else
                    if (ReadNodesCit(&node, rnode, MODEM, FALSE))
#endif
                        {
                        trap(T_CRON, pcts, trapstr);

                        amZap();
                        net86_incorporate(MODEM);
                        SaveAideMess(NULL);

                        SetDone();
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmsg(204));
                    }

                dump_net_messages();
                }
            else
                {
                cPrintf(getmsg(59));
                }

            break;
            }

        case CR_NET86_OUT:
            {
            if (read_net_messages())
                {
                if (!loggedIn)
                    {
                    cPrintf(getmsg(512));
                    doccr();

                    label rnode;
                    CopyStringToBuffer(rnode, String);

#ifdef WINCIT
                    if (ReadNodesCit(this, &node, rnode, MODEM, FALSE))
#else
                    if (ReadNodesCit(&node, rnode, MODEM, FALSE))
#endif
                        {
                        if (LoadPersonByName(node->GetName()))
                            {
                            char trapstr[128];
                            sprintf(trapstr, getmsg(207), node->GetName());

                            cPrintf(pcts, trapstr);
                            doccr();
                            trap(T_CRON, pcts, trapstr);

                            LogEntry5 Log5Save(cfg.maxrooms);
                            Log5Save.CopyRoomNewPointers(*CurrentUser);

                            amZap();

                            if (!net86_fetch(MODEM))
                                {
                                CurrentUser->CopyRoomNewPointers(Log5Save);
                                }

                            SaveAideMess(NULL);

                            CurrentUser->Save(ThisLog, thisRoom);

                            SetDone();
                            }
                        else
                            {
                            mPrintfCR(getmsg(255), node->GetName());
                            break;
                            }
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmsg(208));
                    }

                dump_net_messages();
                }
            else
                {
                cPrintf(getmsg(59));
                }

            break;
            }
#endif

        case CR_COMMAND:
            {
            char trapstr[128];
            sprintf(trapstr, getmsg(209), String);
            trap(T_CRON, pcts, trapstr);

#ifndef WINCIT
            cPrintf(pcts, trapstr);
            doccr();
#endif

            // Note that runScript is different between WINCIT and DOS
            if (runScript(String, NULL))
                {
                SetDone();
                }
            else
                {
                SetLastTried();
                }

            break;
            }


#ifdef WINCIT
        default:
            {
            errorDisp("could not do event type: %d", Type);
            break;
            }
#else
        default:
            {
            cPrintf(getmsg(210), Type);
            doCR();
            break;
            }
#endif
        }

#ifndef WINCIT
    if (theStatus)
        {
        updateStatus = TRUE;

        switch (Type)
            {
            case CR_SHELL_1:
            case CR_SHELL_2:
                {
                sprintf(theStatus->theStatus, getmsg(202), String);
                break;
                }

            case CR_CHAT_OFF:
                {
                sprintf(theStatus->theStatus, getmsg(184));
                break;
                }

            case CR_CHAT_ON:
                {
                sprintf(theStatus->theStatus, getmsg(185));
                break;
                }

            case CR_SHUTDOWN:
                {
                sprintf(theStatus->theStatus, getmsg(186), String);
                break;
                }

            case CR_NET:
                {
                if(read_tr_messages())
                    {
                    sprintf(theStatus->theStatus, gettrmsg(49), String);
                    dump_tr_messages();
                    }
                break;
                }

            case CR_NET69_IN:
                {
                sprintf(theStatus->theStatus, getmsg(203));
                break;
                }

            case CR_NET69_OUT:
                {
                sprintf(theStatus->theStatus, getmsg(207), String);
                break;
                }

            case CR_NET86_IN:
                {
                sprintf(theStatus->theStatus, getmsg(513));
                break;
                }

            case CR_NET86_OUT:
                {
                sprintf(theStatus->theStatus, getmsg(512));
                break;
                }

            case CR_COMMAND:
                {
                sprintf(theStatus->theStatus, getmsg(209), String);
                break;
                }

            default:
                {
                sprintf(theStatus->theStatus, getmsg(210), Type);
                break;
                }
            }

        strcat(theStatus->theStatus, LastAttemptFailed() ? getmsg(370) : getmsg(369));

        theStatus->theTime = LastTried;
        StatusLine.Update(WC_TWp);
        }
#endif

    return (!LastAttemptFailed());
    }
