// --------------------------------------------------------------------------
// Citadel: Net69.CPP
//
// Net 6.9 stuff.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "auxtab.h"
#include "log.h"
#include "net.h"
#include "moreinfo.h"
#include "filecmd.h"
#include "cwindows.h"
#include "filerdwr.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// net69_error()        saves net69 error messages
// net69()


static const char *msg69        = "MSG.69";
static const char *moreinfo69   = "MOREINFO.69";
static const char *sbroomreqPs  = "%s\\ROOMREQ.%s";


// --------------------------------------------------------------------------
// net69_error(): Saves net69 error messages.

void cdecl TERMWINDOWMEMBER net69_error(Bool Fetch, Bool errorflag, const char *node, const char *fmt, ...)
    {
    char buff[256];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    if (!errorflag)
        {
        amPrintf(getnetmsg(144), Fetch ? getnetmsg(55) : getnetmsg(56),
                node, bn);
        }

    amPrintf(getnetmsg(145), buff, bn);
    }


// --------------------------------------------------------------------------
// net69()

Bool TERMWINDOWMEMBER net69(Bool master)
    {
    int i;
    Bool done = FALSE;
    char packetname[20];
    Bool fetchit = FALSE;

    amZap();

    netFailed = FALSE;

    const protocols *theProt = GetProtocolByKey(node->GetProtocol(), TRUE);

    if (!theProt)
        {
        doccr();
        cPrintf(getnetmsg(177));
        doccr();
        return (FALSE);
        }

    if (!CommPort->HaveConnection())
        {
        return (FALSE);
        }

    setdefaultTerm(TT_ANSI);

    sprintf(packetname, getnetmsg(146), node->GetAlias(), cfg.alias);

    changedir(cfg.transpath);

    if (node->GetFetch() > -1)
        {
        directoryinfo *files = filldirectory(packetname, SORT_NONE, OldAndNew, FALSE);

        if (files && *files[0].Name == 0)
            {
            fetchit = TRUE;
            }

        delete [] files;
        }

    if (fetchit || node->GetFetch() > 0)
        {
        LogEntry5 Log5(cfg.maxrooms);

        Log5.CopyRoomNewPointers(*CurrentUser);

        if (!net69_fetch(fetchit, CONSOLE))
            {
            CurrentUser->CopyRoomNewPointers(Log5);
            }
        }

    changedir(cfg.transpath);

    if (!CommPort->HaveConnection())
        {
        return (FALSE);
        }

    if (master)
        {
        // clear the buffer
        while (CommPort->HaveConnection() && CommPort->IsInputReady())
            {
            CommPort->Input();
            }
        }

    if (!CommPort->HaveConnection() || netFailed)
        {
        return (FALSE);
        }

    // wait for them to get their shit together
    doccr();
    cPrintf(getnetmsg(57));

    CommPort->Output('X');

    const time_t t = time(NULL);
    time_t t2 = 0;

    while (CommPort->HaveConnection() && !done)
        {
        if (time(NULL) > (t + (node->GetFetchTimeout() * 60)))
            {
            Hangup();
            netFailed = TRUE;
            }

        KBReady();

        if (CommPort->IsInputReady())
            {
            i = CommPort->Input();

            if (i == 'X')
                {
                done = TRUE;
                }
            else
                {
                DebugOut(pctc, i);
                }
            }

        // wake them up! (every second)
        if (time(NULL) != t2)
            {
            CommPort->Output('X');
            t2 = time(NULL);
            }

        // instantaneous reaction time isn't critical here
        CitIsIdle();
        }

    if (!CommPort->HaveConnection() || netFailed)
        {
        return (FALSE);
        }
    else
        {
        // wake them up!
        for (i = 0; i < 10; i++)
            {
            CommPort->Output('X');
            }
        }


    doccr();
    doccr();
    cPrintf(getnetmsg(58));
    doccr();

    sprintf(packetname, getnetmsg(72), node->GetAlias());

    if (node->GetNetworkRevisionNumber() > 0)
        {
        // unidirectional transfers.

        if (master)
            {
            wxsnd(cfg.transpath, packetname, theProt, (theProt->CheckType == 1) ? 2 : 0);
            wxrcv(cfg.transpath, ns, theProt);
            }
        else
            {
            wxrcv(cfg.transpath, ns, theProt);
            wxsnd(cfg.transpath, packetname, theProt, (theProt->CheckType == 1) ? 2 : 0);
            }
        }
    else
        {
        // bidirectional transfer
        wxsnd(cfg.transpath, packetname, theProt, (theProt->CheckType == 1) ? 2 : 0);
        }

    if (netFailed)
        {
        return (FALSE);
        }

    doccr();
    cPrintf(getnetmsg(59));
    doccr();

    CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

    Hangup();

    if (w)
        {
        destroyCitWindow(w, FALSE);
        }

    OC.setio();

    if (theProt->CheckType == 0)
        {
        ambigUnlink(packetname);
        }
    else if (theProt->CheckType == 2)
        {
        RunApplication(theProt->CheckMethod, packetname, TRUE, FALSE);
        }

    net69_incorporate(CONSOLE);

    SaveAideMess(NULL);

    return (TRUE);
    }

// This should be replaced with NetLogEntry::MakeNet69RoomRequestFile.
// Until then, these two functions have to be maintained together.
void TERMWINDOWMEMBER net69_makeroomrequest(ModemConsoleE W)
    {
    FILE *roomreqfd;
    char roomreqfilename[80];

    sprintf(roomreqfilename, sbroomreqPs, LocalTempPath, cfg.alias);

    wPrintf(W, getnetmsg(60), roomreqfilename);
    wDoCR(W);

    if ((roomreqfd = fopen(roomreqfilename, FO_WB)) == NULL)
        {
        wDoCR(W);
        wPrintf(W, getmsg(78), roomreqfilename);
        wDoCR(W);
        }
    else
        {
        for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
            {
            label NetID;

            if (RoomTab[RoomSlot].IsInuse())
                {
                RoomTab[RoomSlot].GetNetID(NetID, sizeof(NetID));

                if (CurrentUser->CanAccessRoom(RoomSlot) && *NetID && !IsNetIDCorrupted(NetID))
                    {
                    net69_PutStr(roomreqfd, NetID);
                    }
                }
            }

        fclose(roomreqfd);
        }
    }


#ifdef WINCIT
Bool NetLogEntryC::MakeNet69RoomRequestFile(ModemConsoleE W, TermWindowC *TW)
#else
Bool NetLogEntryC::MakeNet69RoomRequestFile(ModemConsoleE W)
#endif
    {
    FILE *roomreqfd;
    char roomreqfilename[80];

    sprintf(roomreqfilename, sbroomreqPs, cfg.temppath, cfg.alias);

    tw()wPrintf(W, getnetmsg(60), roomreqfilename);
    tw()wDoCR(W);

    if ((roomreqfd = fopen(roomreqfilename, FO_WB)) == NULL)
        {
        tw()wDoCR(W);
        tw()wPrintf(W, getmsg(78), roomreqfilename);
        tw()wDoCR(W);

        return (FALSE);
        }
    else
        {
        for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
            {
            label NetID;

            if (RoomTab[RoomSlot].IsInuse())
                {
                RoomTab[RoomSlot].GetNetID(NetID, sizeof(NetID));

                if (CanAccessRoom(RoomSlot) && *NetID && !IsNetIDCorrupted(NetID))
                    {
                    net69_PutStr(roomreqfd, NetID);
                    }
                }
            }

        fclose(roomreqfd);
        }

    return (TRUE);
    }


void TERMWINDOWMEMBER net69_incorporate(ModemConsoleE W)
    {
    int big_total_new = 0, big_total_expired = 0, big_total_duplicate = 0;

    SetDoWhat(NETWORKING);

#ifdef WINCIT
    if (!ReadNodesCit(this, &node, getnetmsg(61), NOMODCON, TRUE))
#else
    if (!ReadNodesCit(&node, getnetmsg(61), NOMODCON, TRUE))
#endif
        {
        wDoCR(W);
        wPrintf(W, getnetmsg(62));
        wDoCR(W);
        return;
        }

    char packetmask[20];
    sprintf(packetmask, getnetmsg(72), cfg.alias);

    changedir(cfg.transpath);

    MRO.Verbose = TRUE;
    MRO.Reverse = FALSE;

    // Sort by date to attempt some order in message incorporation
    directoryinfo *files = filldirectory(packetmask, SORT_DATE, OldAndNew,
            FALSE);

    if (files)
        {
        char fullPathToMsg69[80];
        sprintf(fullPathToMsg69, sbs, LocalTempPath, msg69);

        char fullPathToMoreinfo69[80];
        sprintf(fullPathToMoreinfo69, sbs, LocalTempPath, moreinfo69);

        char fullPathToMesgTmp[80];
        sprintf(fullPathToMesgTmp, sbs, LocalTempPath, mesgTmp);

        char fullPathToFile69[80];
        sprintf(fullPathToFile69, getnetmsg(147), LocalTempPath, cfg.alias);

        Bool errorflag = FALSE;

        for (int i = 0; *files[i].Name; i++)
            {
            label packetname;
            CopyStringToBuffer(packetname, files[i].Name);

            wDoCR(W);
            wPrintf(W, getnetmsg(63), packetname);
            wDoCR(W);

            char fullPathToPacket[128];
            sprintf(fullPathToPacket, sbs, cfg.transpath, packetname);

            char ExtractCommandLine[256];
            sformat(ExtractCommandLine, node->GetExtractPacket(),
                    getnetmsg(48), fullPathToPacket, 0);

            changedir(LocalTempPath);
            apsystem(ExtractCommandLine, TRUE);
            changedir(LocalTempPath);

            if ((cfg.offhook == 1) && !CommPort->HaveConnection())
                {
                offhook();
                }

            // If we didn't get a MSG.69, try again, supering it.
            if (!filexists(fullPathToMsg69))
                {
                char *wo = ExtractCommandLine;
                Bool superit = FALSE;

                while (*wo == '!' || *wo == '@' || *wo == '$' ||
                        *wo == '?' || *wo == '*')
                    {
                    if (*wo == '!')
                        {
                        superit = TRUE;
                        }

                    wo++;
                    }

                if (!superit)
                    {
                    char ExtractCommandLine2[256];
                    ExtractCommandLine2[0] = '!';

                    strcpy(ExtractCommandLine2 + 1, ExtractCommandLine);
                    apsystem(ExtractCommandLine2, TRUE);
                    changedir(LocalTempPath);
                    }
                }

            unlink(fullPathToPacket);

            FILE *msgfd;
            if ((msgfd = fopen(fullPathToMsg69, FO_RB)) == NULL)
                {
                wDoCR(W);
                wPrintf(W, getmsg(78), fullPathToMsg69);
                wDoCR(W);
                }
            else
                {
                NetLogEntryC IncomingLog;
                label rnode;

                if (!IncomingLog.IsValid())
                    {
                    wDoCR(W);
                    wPrintf(W, getnetmsg(50));
                    wDoCR(W);

                    net69_error(FALSE, errorflag, rnode, getnetmsg(50));
                    errorflag = TRUE;
                    }

                if (net69_GetStr(msgfd, rnode, LABELSIZE))
                    {
#ifdef WINCIT
                    if (!ReadNodesCit(this, &node, rnode, NOMODCON, FALSE))
#else
                    if (!ReadNodesCit(&node, rnode, NOMODCON, FALSE))
#endif
                        {
                        DebugOut(206, rnode);

#ifdef WINCIT
                        ReadNodesCit(this, &node, getnetmsg(61), NOMODCON, FALSE);
#else
                        ReadNodesCit(&node, getnetmsg(61), NOMODCON, FALSE);
#endif
                        node->SetName(rnode); // Not "default" in path
                        }

                    // If we were able to allocate space for a log buffer,
                    // attempt to load node this packet came from so we
                    // can control which rooms we accept messages to.
                    const Bool ControlAccess = IncomingLog.IsValid() ?
#ifdef WINCIT
                            IncomingLog.LoadByName(rnode, LogOrder) : FALSE;
#else
                            IncomingLog.LoadByName(rnode) : FALSE;
#endif

                    wDoCR(W);
                    wPrintf(W, getnetmsg(65), cfg.Lmsgs_nym, rnode);
                    wDoCR(W);
                    wDoCR(W);
                    wPrintf(W, getnetmsg(66));
                    wDoCR(W);

                    int total_new = 0, total_expired = 0, total_duplicate = 0;
                    int total_rooms = 0;

                    label netid;
                    while (net69_GetStr(msgfd, netid, LABELSIZE))
                        {
                        KBReady();

                        r_slot roomNo = IdExists(netid, TRUE);

                        if (roomNo == CERROR)
                            {
                            if (node->IsAutoRoom() && !IsNetIDCorrupted(netid) && !IsNetIDInNetIDCit(netid))
                                {
                                label Name;
                                CurrentUser->GetName(Name, sizeof(Name));

                                if (AutoRoom(netid, Name, node->GetAutoGroup(), node->GetAutoHall()))
                                    {
                                    if (node->IsVerbose(NCV_ROOMCREATED))
                                        {
                                        net69_error(FALSE, errorflag, rnode, getnetmsg(107), netid);
                                        errorflag = TRUE;
                                        }
                                    }
                                else
                                    {
                                    if (node->IsVerbose(NCV_ROOMNOTCREATED))
                                        {
                                        net69_error(FALSE, errorflag, rnode, getnetmsg(108), netid);
                                        errorflag = TRUE;
                                        }
                                    }
                                }
                            else
                                {
                                if (node->IsVerbose(NCV_NETIDNOTFOUND))
                                    {
                                    net69_error(FALSE, errorflag, rnode, getnetmsg(109), netid);
                                    errorflag = TRUE;
                                    }
                                }

                            roomNo = IdExists(netid, TRUE);
                            }

                        if (roomNo != CERROR)
                            {
                            int our_new, our_expired, our_duplicate, our_error;

                            label Buffer;
                            wPrintf(W, getnetmsg(131), deansi(RoomTab[roomNo].GetName(Buffer, sizeof(Buffer))));

                            if (ControlAccess && node->IsVerbose(NCV_NOACCESS) && !IncomingLog.CanAccessRoom(roomNo))
                                {
                                net69_error(FALSE, errorflag, rnode, getnetmsg(117), netid);
                                errorflag = TRUE;
                                }

                            ReadMsgFl(roomNo, NULL, msgfd, &our_new, &our_expired, &our_duplicate, &our_error,
                                    ControlAccess ? &IncomingLog : NULL);

                            if (our_new)
                                {
                                wPrintf(W, getnetmsg(132), our_new);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_new);
                                }
                            wPrintf(W, getnetmsg(134));

                            if (our_expired)
                                {
                                wPrintf(W, getnetmsg(132), our_expired);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_expired);
                                }
                            wPrintf(W, getnetmsg(134));

                            if (our_duplicate)
                                {
                                wPrintf(W, getnetmsg(132), our_duplicate);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_duplicate);
                                }

                            wDoCR(W);

                            total_new += our_new;
                            total_expired += our_expired;
                            total_duplicate += our_duplicate;
                            total_rooms++;
                            }
                        else
                            {
                            int ch, lastch;

                            ch = 1;
                            lastch = ch;

                            while (!feof(msgfd) && !((ch == 0) && (lastch == 0)))
                                {
                                lastch = ch;
                                ch = fgetc(msgfd);
                                }

                            wPrintf(W, getnetmsg(75), deansi(netid));
                            wDoCR(W);
                            }

                        StatusLine.Update(WC_TWp);
                        }

                    big_total_new += total_new;
                    big_total_expired += total_expired;
                    big_total_duplicate += total_duplicate;

                    wPrintf(W, getnetmsg(67));
                    wDoCR(W);
                    wPrintf(W, getnetmsg(68));

                    if (total_rooms)
                        {
                        wPrintf(W, getnetmsg(132), total_rooms);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_rooms);
                        }
                    wPrintf(W, getnetmsg(175));

                    if (total_new)
                        {
                        wPrintf(W, getnetmsg(132), total_new);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_new);
                        }
                    wPrintf(W, getnetmsg(134));

                    if (total_expired)
                        {
                        wPrintf(W, getnetmsg(132), total_expired);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_expired);
                        }
                    wPrintf(W, getnetmsg(134));

                    if (total_duplicate)
                        {
                        wPrintf(W, getnetmsg(132), total_duplicate);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_duplicate);
                        }
                    }

                fclose(msgfd);
                unlink(fullPathToMsg69);
                }

            MoreInfo NetInfo;

            if (NetInfo.Open(fullPathToMoreinfo69))
                {
                // read it in...

                NetInfo.Close();
                unlink(fullPathToMoreinfo69);
                }

            if ((msgfd = fopen(fullPathToMesgTmp, FO_RB)) != NULL)
                {
                wDoCR(W);
                wPrintf(W, getnetmsg(69));
                wDoCR(W);
                wDoCR(W);
                wPrintf(W, getnetmsg(34));
                wDoCR(W);

                int our_new, our_expired, our_duplicate, our_error;

                ReadMsgFl(MAILROOM, NULL, msgfd, &our_new, &our_expired, &our_duplicate, &our_error, NULL);

                label New, Routed, Rejected;
                CopyStringToBuffer(New, ltoac(our_new));
                CopyStringToBuffer(Routed, ltoac(our_expired));
                CopyStringToBuffer(Rejected, ltoac(our_duplicate));

                wPrintf(W, getnetmsg(70), New, our_new == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym, Routed, Rejected,
                        ltoac(our_new + our_expired + our_duplicate));

                fclose(msgfd);
                unlink(fullPathToMesgTmp);
                }

            wDoCR(W);

            // Get the alias from the packetname
            char alias[4];
            int k, j = substr(packetname, getnetmsg(148), FALSE) + 2;
            for (k = 0; (k < 3) && packetname[j] && (packetname[j] != '.'); j++, k++)
                {
                alias[k] = packetname[j];
                }
            alias[k] = 0;

            char roomreqfilename[80];
            sprintf(roomreqfilename, sbroomreqPs, LocalTempPath, alias);
            if (filexists(roomreqfilename))
                {
                char RoomReqInTrans[80];
                sprintf(RoomReqInTrans, sbroomreqPs, cfg.transpath, alias);

                wDoCR(W);
                wPrintf(W, getnetmsg(71), roomreqfilename);
                wDoCR(W);

                copyfile(roomreqfilename, RoomReqInTrans);
                unlink(roomreqfilename);
                }

            if (filexists(fullPathToFile69))
                {
                wDoCR(W);
                wPrintf(W, getnetmsg(63), fullPathToFile69);
                wDoCR(W);

                if (node->IsVerbose(NCV_FILE69IN))
                    {
                    net69_error(FALSE, errorflag, alias, getnetmsg(110), fullPathToFile69);
                    errorflag = TRUE;
                    }

                if (node->IsVerbose(NCV_FILE69INFULL))
                    {
                    char PartialCommandLine[256];

                    sprintf(PartialCommandLine, getnetmsg(149), fullPathToFile69, LocalTempPath, getnetmsg(150));

                    ExtractCommandLine[0] = '$';

                    sformat(ExtractCommandLine + 1, node->GetExtractPacket(), getnetmsg(48), PartialCommandLine, 0);
                    }
                else
                    {
                    sformat(ExtractCommandLine, node->GetExtractPacket(), getnetmsg(48), fullPathToFile69, 0);
                    }

                changedir(cfg.dlpath);
                apsystem(ExtractCommandLine, TRUE);

                unlink(fullPathToFile69);

                if (node->IsVerbose(NCV_FILE69INFULL))
                    {
                    char TempFileName[128];

                    sprintf(TempFileName, sbs, LocalTempPath, getnetmsg(150));

                    if (AideMsg)
                        {
                        FILE *fl;

                        if ((fl = fopen(TempFileName, FO_RB)) != NULL)
                            {
                            char *TextFromFile = new char[MAXTEXT];

                            if (TextFromFile)
                                {
                                GetFileMessage(fl, TextFromFile, MAXTEXT - 1);

                                AideMsg->AppendText(TextFromFile);

                                delete [] TextFromFile;
                                }
                            else
                                {
                                OutOfMemory(70);
                                }

                            fclose(fl);
                            }
                        }

                    unlink(TempFileName);
                    }
                }
            }

        delete [] files;
        }


    // Copy all FILE69.* out of #DLPATH to #TRANSPATH
    changedir(cfg.dlpath);
    files = filldirectory(getnetmsg(152), SORT_NONE, OldAndNew, FALSE);

    if (files)
        {
        for (int i = 0; *files[i].Name; i++)
            {
            char fullPathToFile69InTrans[128];

            sprintf(fullPathToFile69InTrans, sbs, cfg.transpath, files[i].Name);

            wDoCR(W);
            wPrintf(W, getnetmsg(71), files[i].Name);
            wDoCR(W);

            copyfile(files[i].Name, fullPathToFile69InTrans);

            unlink(files[i].Name);
            }

        delete [] files;
        }

    MS.Entered = big_total_new;
    MS.Expired = big_total_expired;
    MS.Duplicate = big_total_duplicate;

    callout = FALSE;

    buildaddress(W);

    readNetCmdTmp(W);
    }

Bool TERMWINDOWMEMBER net69_fetch(Bool needFull, ModemConsoleE W)
    {
    Bool usingroomrequest = FALSE;
    FILE *roomreqfd;
    label netid;
    FILE *msgfd;
    int msgsread;
    int crcounter = 0;
    char errorflag = FALSE;

    int oldmread = MS.Read;
    MS.Read = 0;

    SetDoWhat(NETWORKING);

    changedir(LocalTempPath);

    Talley->Fill();

    if (needFull || node->GetFetch() == 2)
        {
        char fullPathToRoomreq[80];
        sprintf(fullPathToRoomreq, getnetmsg(153), cfg.transpath, node->GetAlias());

        if ((roomreqfd = fopen(fullPathToRoomreq, FO_RB)) == NULL)
            {
            DebugOut(48, node->GetAlias());
            }
        else
            {
            usingroomrequest = TRUE;
            }

        char fullPathToMsg69[80];
        sprintf(fullPathToMsg69, sbs, LocalTempPath, msg69);

        if ((msgfd = fopen(fullPathToMsg69, FO_WB)) == NULL)
            {
            wDoCR(W);
            wPrintf(W, getmsg(78), fullPathToMsg69);
            wDoCR(W);

            if (W == MODEM)
                {
                MS.Read = oldmread;
                }

            if (roomreqfd)
                {
                fclose(roomreqfd);
                }

            return (FALSE);
            }

        PutStr(msgfd, cfg.Address);

        wDoCR(W);

        wPrintf(W, getnetmsg(73), cfg.Lmsgs_nym);

        if (usingroomrequest)
            {
            DebugOut(49, node->GetAlias());
            }

        wDoCR(W);
        wDoCR(W);

        wPrintf(W, getnetmsg(76));

        if (usingroomrequest)
            {
            wDoCR(W);

            while (net69_GetStr(roomreqfd, netid, LABELSIZE))
                {
                // It would probably be good to let the modem user
                // abort the fetch as well.
                time(&LastActiveTime);  // avoid timeouts with KBReady
                while (KBReady())
                    {
                    if (ciChar() == ESC)
                        {
                        if (roomreqfd)
                            {
                            fclose(roomreqfd);
                            }

                        fclose(msgfd);
                        return (FALSE);
                        }
                    }

                r_slot roomNo;

                if ((roomNo = IdExists(netid, TRUE)) != CERROR)
                    {
                    if (CurrentUser->CanAccessRoom(roomNo))
                        {
                        if (Talley->NewInRoom(roomNo))
                            {
                            label Buffer;
                            wPrintf(W, getnetmsg(139), deansi(RoomTab[roomNo].GetName(Buffer, sizeof(Buffer))));

                            msgsread = NewRoom(roomNo, NULL, msgfd);

                            wPrintf(W, getnetmsg(140), msgsread);

                            if (!((crcounter+1) % 2))
                                {
                                wDoCR(W);
                                }
                            crcounter++;
                            }
                        }
                    else
                        {
                        if (node->IsVerbose(NCV_NOACCESS))
                            {
                            label Buffer;
                            net69_error(TRUE, errorflag, node->GetName(), getnetmsg(111),
                                    RoomTab[roomNo].GetName(Buffer, sizeof(Buffer)));
                            errorflag = TRUE;
                            }
                        }
                    }
                else
                    {
                    wPrintf(W, getnetmsg(139), deansi(netid));

                    if (node->IsAutoRoom() && !IsNetIDCorrupted(netid) && !IsNetIDInNetIDCit(netid))
                        {
                        label Name;
                        CurrentUser->GetName(Name, sizeof(Name));

                        if (AutoRoom(netid, Name, node->GetAutoGroup(), node->GetAutoHall()))
                            {
                            wPrintf(W, getnetmsg(77));
                            if (node->IsVerbose(NCV_ROOMCREATED))
                                {
                                net69_error(TRUE, errorflag, node->GetName(), getnetmsg(107), netid);
                                errorflag = TRUE;
                                }
                            }
                        else
                            {
                            wPrintf(W, getnetmsg(141));
                            if (node->IsVerbose(NCV_ROOMNOTCREATED))
                                {
                                net69_error(TRUE, errorflag, node->GetName(), getnetmsg(108), netid);
                                errorflag = TRUE;
                                }
                            }
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(141));
                        if (node->IsVerbose(NCV_NETIDNOTFOUND))
                            {
                            net69_error(TRUE, errorflag, node->GetName(), getnetmsg(109), netid);
                            errorflag = TRUE;
                            }
                        }

                    if (!((crcounter+1) % 2))
                        {
                        wDoCR(W);
                        }

                    crcounter++;
                    }
                }

            fclose(roomreqfd);
            }
        else
            {
            wDoCR(W);

            for (r_slot roomNo = 0; roomNo < cfg.maxrooms; roomNo++)
                {
                KBReady();

                label Buffer;
                if (CurrentUser->CanAccessRoom(roomNo) && *RoomTab[roomNo].GetNetID(Buffer, sizeof(Buffer)) &&
                        !CurrentUser->IsRoomExcluded(roomNo) && Talley->NewInRoom(roomNo))
                    {
                    wPrintf(W, getnetmsg(139), deansi(RoomTab[roomNo].GetName(Buffer, sizeof(Buffer))));

                    msgsread = NewRoom(roomNo, NULL, msgfd);

                    if (msgsread)
                        {
                        wPrintf(W, getnetmsg(140), msgsread);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(156), msgsread);
                        }

                    if (!((crcounter+1) % 2))
                        {
                        wDoCR(W);
                        }
                    crcounter++;
                    }
                }
            }

        fclose(msgfd);

        wDoCR(W);
        wDoCR(W);
        wPrintf(W, getnetmsg(74), ltoac(MS.Read), (MS.Read == 1) ? cfg.Lmsg_nym : cfg.Lmsgs_nym);
        wDoCR(W);
        wDoCR(W);
        }

    char fullPathToMailFile[80];
    sprintf(fullPathToMailFile, sbs, cfg.transpath, node->GetMailFileName());

    if (filexists(fullPathToMailFile))
        {
        char fullPathToMesgTmp[80];
        sprintf(fullPathToMesgTmp, sbs, LocalTempPath, mesgTmp);

        wPrintf(W, getnetmsg(78));
        wDoCR(W);
        wDoCR(W);

        if (copyfile(fullPathToMailFile, fullPathToMesgTmp))
            {
            unlink(fullPathToMailFile);
            }
        else
            {
            wPrintf(W, getnetmsg(42));
            wDoCR(W);
            wDoCR(W);
            }
        }

    CurrentUser->MorePoop(1);

    char roomreqfilename[20];
    *roomreqfilename = 0;

    if (node->GetRequest() && !(CurrentUser->GetPoopcount() % node->GetRequest()))
        {
        changedir(LocalTempPath);

        net69_makeroomrequest(W);
        wDoCR(W);

        // how's sbroomreqPs + 3 for icky?
        //// it's extremely icky if you give it two parameters!
        sprintf(roomreqfilename, sbroomreqPs + 3, cfg.alias);
        }

    char fullPathToFile69[80];
    sprintf(fullPathToFile69, getnetmsg(157), cfg.transpath, node->GetAlias());

    if (filexists(fullPathToFile69))
        {
        wPrintf(W, getnetmsg(79), fullPathToFile69);
        wDoCR(W);
        wDoCR(W);
        }
    else
        {
        *fullPathToFile69 = 0;
        }

    wPrintf(W, getnetmsg(80));
    wDoCR(W);
    wDoCR(W);

    changedir(LocalTempPath);

    char packetname[20];
    sprintf(packetname, getnetmsg(158), node->GetAlias(), cfg.alias, CurrentUser->GetPoopcount() % 1000);

    // Zip them up
    char filesToSend[128];

    if (needFull || node->GetFetch() == 2)
        {
        sprintf(filesToSend, getnetmsg(159), msg69, mesgTmp, roomreqfilename, fullPathToFile69);
        }
    else
        {
        sprintf(filesToSend, getnetmsg(160), mesgTmp, roomreqfilename, fullPathToFile69);
        }

    char createCommandLine[256];
    sformat(createCommandLine, node->GetCreatePacket(), getnetmsg(49), packetname, filesToSend, 0);

    apsystem(createCommandLine, TRUE);
    changedir(LocalTempPath);

    // check to make sure it was really zipped - if you are out of memory,
    // and zip fails, netting becomes hosed.
    if (!filexists(packetname))
        {
        char *wo = createCommandLine;
        int superit = FALSE;

        while (*wo == '!' || *wo == '@' || *wo == '$' || *wo == '?' || *wo == '*')
            {
            if (*wo == '!')
                {
                superit = TRUE;
                }
            wo++;
            }

        if (!superit)
            {
            char createCommandLine2[256];

            *createCommandLine2 = '!';
            strcpy(createCommandLine2 + 1, createCommandLine);

            apsystem(createCommandLine2, TRUE);
            changedir(LocalTempPath);
            }

        if (!filexists(packetname))
            {
            // either we already supershelled, or supershelling failed, too.
            wPrintf(W, getnetmsg(81));
            wDoCR(W);

            char fullPathToMesgTmp[80];
            sprintf(fullPathToMesgTmp, sbs, LocalTempPath, mesgTmp);

            if (filexists(fullPathToMesgTmp))
                {
                wPrintf(W, getnetmsg(82));
                wDoCR(W);
                wDoCR(W);

                copyfile(fullPathToMesgTmp, fullPathToMailFile);
                unlink(fullPathToMesgTmp);
                }

            unlink(msg69);
            unlink(roomreqfilename);

            if (W == MODEM)
                {
                MS.Read = oldmread;
                }

            return (FALSE);
            }
        }

    wPrintf(W, getnetmsg(83), packetname);
    wDoCR(W);

    char fullPathToPacketInTrans[80], fullPathToPacketInTemp[80];
    sprintf(fullPathToPacketInTrans, sbs, cfg.transpath, packetname);
    sprintf(fullPathToPacketInTemp, sbs, LocalTempPath, packetname);

    if (copyfile(fullPathToPacketInTemp, fullPathToPacketInTrans))
        {
        changedir(LocalTempPath);
        unlink(mesgTmp);
        unlink(msg69);
        unlink(fullPathToPacketInTemp);
        unlink(roomreqfilename);
        unlink(fullPathToFile69);

        if (W == MODEM)
            {
            MS.Read = oldmread;
            }

        return (TRUE);
        }
    else
        {
        wDoCR(W);
        wPrintf(W, getnetmsg(47));
        wDoCR(W);

        char fullPathToMesgTmp[80];
        sprintf(fullPathToMesgTmp, sbs, LocalTempPath, mesgTmp);

        if (filexists(fullPathToMesgTmp))
            {
            wDoCR(W);
            wPrintf(W, getnetmsg(82));
            wDoCR(W);

            copyfile(fullPathToMesgTmp, fullPathToMailFile);
            unlink(fullPathToMesgTmp);
            }

        changedir(LocalTempPath);
        unlink(msg69);
        unlink(roomreqfilename);

        if (W == MODEM)
            {
            MS.Read = oldmread;
            }

        return (FALSE);
        }
    }
