// --------------------------------------------------------------------------
// Citadel: LogOut.CPP
//
// Overlayed logout log code.

#include "ctdl.h"
#pragma hdrstop

#include "group.h"
#include "room.h"
#include "filecmd.h"
#include "tallybuf.h"
#include "log.h"
#include "net.h"
#include "account.h"
#include "events.h"
#include "cfgfiles.h"
#include "cwindows.h"
#include "term.h"
#include "hall.h"
#include "statline.h"
#include "termwndw.h"
#include "extmsg.h"

// --------------------------------------------------------------------------
// Contents
//
// terminate()          menu-level routine to exit system
// setdefaultconfig()   sets default new user configuration


// --------------------------------------------------------------------------
// terminate(): Menu-level routine to exit system.

void TERMWINDOWMEMBER terminate(Bool discon)
    {
    char dtstr[80];
    Bool initport = FALSE;

    if (!altF3Timeout && (cfg.chatmail == 2 && chatReq) || (cfg.chatmail == 3) || (cfg.chatmail == 4))
        {
        if (cfg.chatmail == 2)
            {
            dispBlb(B_CHATTED);
            }

        Message *Msg = new Message;

        if (Msg)
            {
            msgtosysop(Msg);
            delete Msg;
            }
        else
            {
            OutOfMemory(41);
            }
        }

    if (loggedIn && onConsole)
        {
        last_console_login_callno = cfg.callno;
        }

    chatReq = FALSE;
    const Bool doStore = HaveConnectionToUser();

    if (discon || !doStore)
        {
        sysopNew = FALSE;
        }

    const long balance = CurrentUser->GetCredits();

    OC.SetOutFlag(OUTOK);

    if ((doStore && (MRO.Verbose == 2)) || CurrentUser->IsAutoVerbose() || CurrentUser->IsVerboseLogOut() ||
            CurrentUser->IsNode())
        {
        if (CurrentUser->IsNode())
            {
            OC.SetOutFlag(IMPERVIOUS);  // no carrier
            }

        CRmPrintfCR(getmsg(119), ltoac(cfg.callno));
        if (loggedIn)
           {
           mPrintfCR(getmsg(118), diffstamp(logtimestamp));
           }
        label Entered, L;
        CopyStringToBuffer(Entered, ltoac(MS.Entered));
        mPrintfCR(getmsg(117), Entered, MS.Entered == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym, ltoac(MS.Read));

        if (cfg.accounting && CurrentUser->IsAccounting())
            {
            long C = (CurrentUserAccount->GetBalanceAtLogin() - balance) / 60;

            mPrintfCR(getmsg(116), ltoac(C), (C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym, L);

            C = balance / 60;

            mPrintfCR(getmsg(115), ltoac(C), (C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
            }

        char Buffer[64];
        strftime(dtstr, 79, (loggedIn) ? CurrentUser->GetVerboseDateStamp(Buffer, sizeof(Buffer)) : cfg.vdatestamp, 0l);
        }

    if (doStore && MRO.Verbose)
        {
        goodbye();
        }

    OC.SetOutFlag(IMPERVIOUS);

    label Buffer;
    if (loggedIn)
        {
        if ((MRO.Verbose == 2) || CurrentUser->IsAutoVerbose() || CurrentUser->IsVerboseLogOut() || CurrentUser->IsNode())
            {
            CRmPrintfCR(getmsg(614), CurrentUser->GetName(Buffer, sizeof(Buffer)), dtstr);
            }
        else
            {
            CRmPrintfCR(getmsg(114), CurrentUser->GetName(Buffer, sizeof(Buffer)));
            }
        }

    // Go back to the default hall
    thisHall = HallData.GetDefault();

    if (discon)
        {
#ifdef WINCIT
        switch(CommPort->GetType())
            {
            case CT_SERIAL:
                {
                initport = TRUE;
                break;
                }

            case CT_TELNET:
                {
                initport = FALSE;
                break;
                }

            default:
                {
                initport = FALSE;
                break;
                }
            }
#endif

        if (CommPort->HaveConnection())
            {
            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }
            }

        //OC.whichIO = MODEM; // I really don't know
        //OC.setio();
        }

    if (!slv_door && !CommPort->HaveConnection())
        {
        CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

#ifdef WINCIT
        if(initport)
#endif
            {
            Initport();
            }

        if (w)
            {
            destroyCitWindow(w, FALSE);
            }
        }

    CurrentUser->SetInRoom(thisRoom, TRUE);

    if (!doStore) // if carrier dropped
        {
        if(!read_tr_messages())
            {
            errorDisp(getmsg(172));
            }

#ifdef WINCIT
        trap(T_CARRIER, WindowCaption, gettrmsg(19), WindowCaption);
#else
        trap(T_CARRIER, gettrmsg(19));
#endif
        dump_tr_messages();
        }

    // update new pointer only if carrier not dropped
    if (loggedIn && doStore)
        {
        CurrentUser->SetRoomNewPointer(thisRoom, MessageDat.NewestMessage());
        }

    if (loggedIn)
        {
        CurrentUser->SetCallNumber(cfg.callno);
        CurrentUser->SetCallTime(logtimestamp);

        // for the Minibin() function to calculate #new messages
        CurrentUser->SetLastMessage(MessageDat.NewestMessage());

        CurrentUser->SetTotalTime(CurrentUser->GetTotalTime() + (time(NULL) - logtimestamp));

        CurrentUser->SetLogins(CurrentUser->GetLogins() + 1);
        CurrentUser->SetPosted(CurrentUser->GetPosted() + MS.Entered);
        CurrentUser->SetRead(CurrentUser->GetRead() + MS.Read);

        CurrentUser->Save(ThisLog, thisRoom);

        // this stuff puts the current room at the end of jumpback,
        // so J will take you back here.
        jumpback jb;

        jb.hall = thisHall;
        jb.room = thisRoom;
        jb.newpointer = CurrentUser->GetRoomNewPointer(thisRoom);
        jb.bypass = Talley->Bypassed(thisRoom);
        jb.newMsgs = Talley->NewInRoom(thisRoom);

        CurrentUser->JumpbackPush(jb);

#ifdef MULTI
        char LogoffEvent[256];
        label NameBuffer;

        sprintf(LogoffEvent, doStore ? getmsg(665) : getmsg(29), CurrentUser->GetName(NameBuffer, sizeof(NameBuffer)));
        TermWindowCollection.SystemEvent(SE_LOGONOFF, FALSE, NULL, FALSE, LogoffEvent);
#endif

        loggedIn = FALSE;

        if (CurrentUser->IsPrintFile() && OC.Printing)
            {
            OC.Printing = OC.WasPrinting;

            if (!OC.Printing)
                {
                fclose(OC.PrintFile);
                }
            }

        // trap it
        if (CurrentUser->IsNode())
            {
            if (netError)
                {
                Bool OldTrapit = cfg.trapit[T_NETWORK];

                if (node->GetNetFail() > 0)
                    {
                    cfg.trapit[T_NETWORK] = TRUE;
                    }
                else if (node->GetNetFail() < 0)
                    {
                    cfg.trapit[T_NETWORK] = FALSE;
                    }

#ifdef WINCIT
                trap(T_NETWORK, WindowCaption, getmsg(606), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_NETWORK, getmsg(606), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

                cfg.trapit[T_NETWORK] = OldTrapit;
                }
            else
                {
#ifdef WINCIT
                trap(T_NETWORK, WindowCaption, getmsg(22), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_NETWORK, getmsg(22), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif
                }
            }
        else
            {
            doEvent(EVT_LOGOUT);
            if(!read_tr_messages())
                {
                errorDisp(getmsg(172));
                }
#ifdef WINCIT
            trap(T_LOGIN, WindowCaption, gettrmsg(24), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
            trap(T_LOGIN, gettrmsg(24), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif
            dump_tr_messages();
            }

        const TrapKeywords TrapType = CurrentUser->IsNode() ? T_NETWORK : T_ACCOUNT;
        if(!read_tr_messages())
            {
            errorDisp(getmsg(172));
            }
#ifndef WINCIT
        trap(TrapType, gettrmsg(25), MS.Entered);
        trap(TrapType, gettrmsg(26), MS.Read);
#else
        trap(TrapType, WindowCaption, gettrmsg(25), MS.Entered);
        trap(TrapType, WindowCaption, gettrmsg(26), MS.Read);
#endif
        dump_tr_messages();

        if (CurrentUser->IsNode())
            {
            if(!read_tr_messages())
                {
                errorDisp(getmsg(172));
                }
#ifndef WINCIT
            trap(T_NETWORK, gettrmsg(20), MS.Expired);
            trap(T_NETWORK, gettrmsg(21), MS.Duplicate);
#else
            trap(T_NETWORK, WindowCaption, gettrmsg(20), MS.Expired);
            trap(T_NETWORK, WindowCaption, gettrmsg(21), MS.Duplicate);
#endif
            dump_tr_messages();
            }
        else if (cfg.accounting)    // There's just no accounting for nodes
            {
            if(!read_tr_messages())
                {
                errorDisp(getmsg(172));
                }
#ifdef WINCIT
            trap(T_ACCOUNT, WindowCaption, gettrmsg(27), CurrentUserAccount->GetBalanceAtLogin() - balance);
#else
            trap(T_ACCOUNT, gettrmsg(27), CurrentUserAccount->GetBalanceAtLogin() - balance);
#endif
            dump_tr_messages();
            }

        delete MS.AbortedMessage;
        MS.AbortedMessage = NULL;

#ifdef MULTI
        LoginList.Remove(ThisLog);
#endif
        }

    setdefaultconfig(FALSE);

    if (discon)
        {
        setdefaultTerm(TT_ANSI);
        }
    else
        {
        setdefaultTerm(TT_DUMB);
        }

    CurrentUser->SetCredits(discon ? 0L : cfg.unlogtimeout * 60L);
    StatusLine.Update(WC_TWp);

    Talley->Fill();

    CurrentRoom->Load(LOBBY);
    checkdir();
    thisRoom = LOBBY;

    AideQueueClear();
    clearFileQueue();
    MS.AutoMKC = AM_NONE;
    MS.MarkedID = 0L;

    freeNode(&node);
    netError = FALSE;   // just in case

    Cron.ResetTimer();
    }


// --------------------------------------------------------------------------
// setdefaultconfig(): Sets default new user configuration.

void TERMWINDOWMEMBER setdefaultconfig(Bool KeepTerm)
    {
    int oldATT[ATTR_NUM];

    oldATT[ATTR_NORMAL]     = CurrentUser->GetAttribute(ATTR_NORMAL);
    oldATT[ATTR_BLINK]      = CurrentUser->GetAttribute(ATTR_BLINK);
    oldATT[ATTR_REVERSE]    = CurrentUser->GetAttribute(ATTR_REVERSE);
    oldATT[ATTR_BOLD]       = CurrentUser->GetAttribute(ATTR_BOLD);
    oldATT[ATTR_UNDERLINE]  = CurrentUser->GetAttribute(ATTR_UNDERLINE);

    prevChar = ' '; // why here?

    CurrentUser->Clear();

    // set all default defaults...
    CurrentUser->SetWidth(80);
    CurrentUser->SetCredits(3600);

    CurrentUser->SetLinefeeds(TRUE);
    CurrentUser->SetOldToo(TRUE);
    CurrentUser->SetNetUser(TRUE);
    CurrentUser->SetViewRoomDesc(TRUE);
    CurrentUser->SetEnterBorders(TRUE);
    CurrentUser->SetViewTitleSurname(TRUE);
    CurrentUser->SetViewSubjects(TRUE);
    CurrentUser->SetViewSignatures(TRUE);
    CurrentUser->SetMinibin(TRUE);
    CurrentUser->SetViewRoomInfoLines(TRUE);
    CurrentUser->SetViewHallDescription(TRUE);
    CurrentUser->SetViewBorders(TRUE);
    CurrentUser->SetSeeOwnChats(TRUE);
    CurrentUser->SetHearLaughter(TRUE);

    if (!KeepTerm)
        {
        CurrentUser->SetDefaultColors(TermCap->IsColor());
        }
    else
        {
        CurrentUser->SetAttribute(ATTR_NORMAL,      oldATT[ATTR_NORMAL]);
        CurrentUser->SetAttribute(ATTR_BLINK,       oldATT[ATTR_BLINK]);
        CurrentUser->SetAttribute(ATTR_REVERSE,     oldATT[ATTR_REVERSE]);
        CurrentUser->SetAttribute(ATTR_BOLD,        oldATT[ATTR_BOLD]);
        CurrentUser->SetAttribute(ATTR_UNDERLINE,   oldATT[ATTR_UNDERLINE]);
        }

    CurrentUser->SetPromptFormat(cfg.prompt);           // "%n^A2%r%e"
    CurrentUser->SetDateStamp(cfg.datestamp);           // "%x %X %p"
    CurrentUser->SetVerboseDateStamp(cfg.vdatestamp);   // "%A %x %X %p"
    CurrentUser->SetNetPrefix(cfg.netPrefix);           // "3*0"
    CurrentUser->SetMorePrompt(cfg.moreprompt);         // "<more>"

    if (!KeepTerm)
        {
        setdefaultTerm(TT_DUMB);
        }

    if (cfg.accounting)
        {
        CurrentUserAccount->Negotiate(CurrentUser);
        }

    changedir(cfg.homepath);

    discardable *du = readData(8, 0, 0);

    if (du)
        {
        FILE *defusr;
        if ((defusr = fopen(citfiles[C_DEFUSER_CIT], FO_R)) != NULL)
            {
            const char **duk = (const char **) du->aux;
            char line[90];
            char *words[256];

            while (fgets(line, 90, defusr) != NULL)
                {
                if (line[0] != '#')
                    {
                    continue;
                    }

                const int count = parse_it(words, line);

                if (count > 1)
                    {
                    int i;
                    for (i = 0; i < DUK_NUM; i++)
                        {
                        if (SameString(words[0] + 1, duk[i]))
                            {
                            break;
                            }
                        }

                    switch (i)
                        {
                        case DUK_FORWARD:
                            {
                            CurrentUser->SetForwardAddr(words[1]);
                            break;
                            }

                        case DUK_SURNAME:
                            {
                            CurrentUser->SetSurname(words[1]);
                            break;
                            }

                        case DUK_TITLE:
                            {
                            CurrentUser->SetTitle(words[1]);
                            break;
                            }

                        case DUK_BOLD:
                            {
                            CurrentUser->SetAttribute(ATTR_BOLD, atoi(words[1]));
                            break;
                            }

                        case DUK_NULLS:
                            {
                            CurrentUser->SetNulls(atoi(words[1]));
                            break;
                            }

                        case DUK_WIDTH:
                            {
                            CurrentUser->SetWidth(atoi(words[1]));
                            break;
                            }

                        case DUK_CREDITS:
                            {
                            CurrentUser->SetCredits(60 * atol(words[1]));
                            break;
                            }

                        case DUK_INVERSE:
                            {
                            CurrentUser->SetAttribute(ATTR_REVERSE, atoi(words[1]));
                            break;
                            }

                        case DUK_BLINK:
                            {
                            CurrentUser->SetAttribute(ATTR_BLINK, atoi(words[1]));
                            break;
                            }

                        case DUK_UNDERLINE:
                            {
                            CurrentUser->SetAttribute(ATTR_UNDERLINE, atoi(words[1]));
                            break;
                            }

                        case DUK_NORMAL:
                            {
                            CurrentUser->SetAttribute(ATTR_NORMAL, atoi(words[1]));
                            break;
                            }

                        case DUK_PROTOCOL:
                            {
                            CurrentUser->SetDefaultProtocol(words[1][0]);
                            break;
                            }

                        case DUK_PROMPT:
                            {
                            CurrentUser->SetPromptFormat(words[1]);
                            break;
                            }

                        case DUK_DSTAMP:
                            {
                            CurrentUser->SetDateStamp(words[1]);
                            break;
                            }

                        case DUK_VDSTAMP:
                            {
                            CurrentUser->SetVerboseDateStamp(words[1]);
                            break;
                            }

                        case DUK_SIGNATURE:
                            {
                            CurrentUser->SetSignature(words[1]);
                            break;
                            }

                        case DUK_NETPREFIX:
                            {
                            CurrentUser->SetNetPrefix(words[1]);
                            break;
                            }

                        case DUK_ADDR1:
                            {
                            CurrentUser->SetMailAddr1(words[1]);
                            break;
                            }

                        case DUK_ADDR2:
                            {
                            CurrentUser->SetMailAddr2(words[1]);
                            break;
                            }

                        case DUK_ADDR3:
                            {
                            CurrentUser->SetMailAddr3(words[1]);
                            break;
                            }

                        case DUK_POOP:
                            {
                            CurrentUser->SetPoopcount(atol(words[1]));
                            break;
                            }

                        case DUK_UCMASK:
                            {
                            CurrentUser->SetUpperOnly(atoi(words[1]));
                            break;
                            }

                        case DUK_EXPERT:
                            {
                            CurrentUser->SetExpert(atoi(words[1]));
                            break;
                            }

                        case DUK_AIDE:
                            {
                            CurrentUser->SetAide(atoi(words[1]));
                            break;
                            }

                        case DUK_TABS:
                            {
                            CurrentUser->SetTabs(atoi(words[1]));
                            break;
                            }

                        case DUK_OLDTOO:
                            {
                            CurrentUser->SetOldToo(atoi(words[1]));
                            break;
                            }

                        case DUK_UNLISTED:
                            {
                            CurrentUser->SetUnlisted(atoi(words[1]));
                            break;
                            }

                        case DUK_PERMANENT:
                            {
                            CurrentUser->SetPermanent(atoi(words[1]));
                            break;
                            }

                        case DUK_SYSOP:
                            {
                            CurrentUser->SetSysop(atoi(words[1]));
                            break;
                            }

                        case DUK_NODE:
                            {
                            CurrentUser->SetNode(atoi(words[1]));
                            break;
                            }

                        case DUK_NOACCOUNT:
                            {
                            CurrentUser->SetAccounting(!atoi(words[1]));
                            break;
                            }

                        case DUK_NOMAIL:
                            {
                            CurrentUser->SetMail(!atoi(words[1]));
                            break;
                            }

                        case DUK_ROOMTELL:
                            {
                            CurrentUser->SetViewRoomDesc(atoi(words[1]));
                            break;
                            }

                        case DUK_BORDERS:
                            {
                            CurrentUser->SetEnterBorders(atoi(words[1]));
                            break;
                            }

                        case DUK_VERIFIED:
                            {
                            CurrentUser->SetVerified(!!atoi(words[1]));
                            break;
                            }

                        case DUK_SURNAMLOK:
                            {
                            CurrentUser->SetSurnameLocked(atoi(words[1]));
                            break;
                            }

                        case DUK_LOCKHALL:
                            {
                            CurrentUser->SetDefaultHallLocked(atoi(words[1]));
                            break;
                            }

                        case DUK_DISPLAYTS:
                            {
                            CurrentUser->SetViewTitleSurname(atoi(words[1]));
                            break;
                            }

                        case DUK_SUBJECTS:
                            {
                            CurrentUser->SetViewSubjects(atoi(words[1]));
                            break;
                            }

                        case DUK_SIGNATURES:
                            {
                            CurrentUser->SetViewSignatures(atoi(words[1]));
                            break;
                            }

                        case DUK_DEFAULTHALL:
                            {
                            CurrentUser->SetDefaultHall(words[1]);
                            break;
                            }

                        case DUK_LINESSCREEN:
                            {
                            CurrentUser->SetLinesPerScreen(atoi(words[1]));
                            break;
                            }

                        case DUK_FORWARDNODE:
                            {
                            CurrentUser->SetForwardAddrNode(words[1]);

                            if (*words[1])
                                {
                                CurrentUser->SetForwardToNode(TRUE);
                                }

                            break;
                            }

                        case DUK_LFMASK:
                            {
                            CurrentUser->SetLinefeeds(atoi(words[1]));
                            break;
                            }

                        case DUK_PROBLEM:
                            {
                            CurrentUser->SetProblem(atoi(words[1]));
                            break;
                            }

                        case DUK_NETUSER:
                            {
                            CurrentUser->SetNetUser(atoi(words[1]));
                            break;
                            }

                        case DUK_NEXTHALL:
                            {
                            CurrentUser->SetAutoNextHall(atoi(words[1]));
                            break;
                            }

                        case DUK_PSYCHO:
                            {
                            CurrentUser->SetPsycho(atoi(words[1]));
                            break;
                            }

                        case DUK_TWIRLY:
                            {
                            CurrentUser->SetTwirly(atoi(words[1]));
                            break;
                            }

                        case DUK_VERBOSE:
                            {
                            CurrentUser->SetAutoVerbose(atoi(words[1]));
                            break;
                            }

                        case DUK_MSGPAUSE:
                            {
                            CurrentUser->SetPauseBetweenMessages(atoi(words[1]));
                            break;
                            }

                        case DUK_MINIBIN:
                            {
                            CurrentUser->SetMinibin(atoi(words[1]));
                            break;
                            }

                        case DUK_MSGCLS:
                            {
                            CurrentUser->SetClearScreenBetweenMessages(atoi(words[1]));
                            break;
                            }

                        case DUK_ROOMINFO:
                            {
                            CurrentUser->SetViewRoomInfoLines(atoi(words[1]));
                            break;
                            }

                        case DUK_HALLTELL:
                            {
                            CurrentUser->SetViewHallDescription(atoi(words[1]));
                            break;
                            }

                        case DUK_VERBOSECONT:
                            {
                            CurrentUser->SetVerboseContinue(atoi(words[1]));
                            break;
                            }

                        case DUK_VIEWCENSOR:
                            {
                            CurrentUser->SetViewCensoredMessages(atoi(words[1]));
                            break;
                            }

                        case DUK_SEEBORDERS:
                            {
                            CurrentUser->SetViewBorders(atoi(words[1]));
                            break;
                            }

                        case DUK_OUT300:
                            {
                            CurrentUser->SetOut300(atoi(words[1]));
                            break;
                            }

                        case DUK_LOCKUSIG:
                            {
                            CurrentUser->SetUserSignatureLocked(atoi(words[1]));
                            break;
                            }

                        case DUK_HIDEEXCL:
                            {
                            CurrentUser->SetHideMessageExclusions(atoi(words[1]));
                            break;
                            }

                        case DUK_NODOWNLOAD:
                            {
                            CurrentUser->SetDownload(!atoi(words[1]));
                            break;
                            }

                        case DUK_NOUPLOAD:
                            {
                            CurrentUser->SetUpload(!atoi(words[1]));
                            break;
                            }

                        case DUK_NOCHAT:
                            {
                            CurrentUser->SetChat(!atoi(words[1]));
                            break;
                            }

                        case DUK_PRINTFILE:
                            {
                            CurrentUser->SetPrintFile(atoi(words[1]));
                            break;
                            }

                        case DUK_REALNAME:
                            {
                            CurrentUser->SetRealName(words[1]);
                            break;
                            }

                        case DUK_PHONENUM:
                            {
                            CurrentUser->SetPhoneNumber(words[1]);
                            break;
                            }

                        case DUK_SPELLCHECK:
                            {
                            CurrentUser->SetSpellCheckMode(atoi(words[1]));
                            break;
                            }

                        case DUK_NOMAKEROOM:
                            {
                            CurrentUser->SetMakeRoom(!atoi(words[1]));
                            break;
                            }

                        case DUK_VERBOSELO:
                            {
                            CurrentUser->SetVerboseLogOut(atoi(words[1]));
                            break;
                            }

                        case DUK_CONFSAVE:
                            {
                            CurrentUser->SetConfirmSave(atoi(words[1]));
                            break;
                            }

                        case DUK_CONFABORT:
                            {
                            CurrentUser->SetConfirmAbort(atoi(words[1]));
                            break;
                            }

                        case DUK_CONFEOABORT:
                            {
                            CurrentUser->SetConfirmNoEO(atoi(words[1]));
                            break;
                            }

                        case DUK_USEPERSONAL:
                            {
                            CurrentUser->SetUsePersonalHall(atoi(words[1]));
                            break;
                            }

                        case DUK_YOUAREHERE:
                            {
                            CurrentUser->SetYouAreHere(atoi(words[1]));
                            break;
                            }

                        case DUK_IBMROOM:
                            {
                            CurrentUser->SetIBMRoom(atoi(words[1]));
                            break;
                            }

                        case DUK_WIDEROOM:
                            {
                            CurrentUser->SetWideRoom(atoi(words[1]));
                            break;
                            }

                        case DUK_MUSIC:
                            {
                            CurrentUser->SetMusic(atoi(words[1]));
                            break;
                            }

                        case DUK_MOREPROMPT:
                            {
                            CurrentUser->SetMorePrompt(words[1]);
                            break;
                            }

                        case DUK_NUMUSERSHOW:
                            {
                            CurrentUser->SetNumUserShow(atoi(words[1]));
                            break;
                            }

                        case DUK_CALLLIMIT:
                            {
                            CurrentUser->SetCallLimit(atoi(words[1]));
                            break;
                            }

                        case DUK_CHECKAPS:
                            {
                            CurrentUser->SetCheckApostropheS(atoi(words[1]));
                            break;
                            }

                        case DUK_CHECKALLCAPS:
                            {
                            CurrentUser->SetCheckAllCaps(atoi(words[1]));
                            break;
                            }

                        case DUK_CHECKDIGITS:
                            {
                            CurrentUser->SetCheckDigits(atoi(words[1]));
                            break;
                            }

                        case DUK_EXCLUDEENCRYPTED:
                            {
                            CurrentUser->SetExcludeEncryptedMessages(atoi(words[1]));
                            break;
                            }

                        case DUK_SHOWCOMMAS:
                            {
                            CurrentUser->SetViewCommas(atoi(words[1]));
                            break;
                            }

                        case DUK_PUNPAUSES:
                            {
                            CurrentUser->SetPUnPauses(atoi(words[1]));
                            break;
                            }

                        case DUK_KUSER:
                            {
                            CurrentUser->AddKill(KF_USER, words[1]);
                            break;
                            }

                        case DUK_KTEXT:
                            {
                            CurrentUser->AddKill(KF_TEXT, words[1]);
                            break;
                            }

                        case DUK_KNODE:
                            {
                            CurrentUser->AddKill(KF_NODE, words[1]);
                            break;
                            }

                        case DUK_KREG:
                            {
                            CurrentUser->AddKill(KF_REGION, words[1]);
                            break;
                            }

                        case DUK_TUSER:
                            {
                            if (count > 2)
                                {
                                CurrentUser->AddTagUser(words[1], words[2]);
                                }

                            break;
                            }

                        case DUK_DICTWORD:
                            {
                            CurrentUser->AddWordToDictionary(words[1]);
                            break;
                            }

                        case DUK_FINGER:
                            {
                            CurrentUser->SetFinger(words[1]);
                            break;
                            }

                        case DUK_USERDEF:
                            {
                            if (count > 2)
                                {
                                CurrentUser->SetUserDefined(words[1], words[2]);
                                }

                            break;
                            }

                        case DUK_REPLACE:
                            {
                            if (count > 2)
                                {
                                CurrentUser->AddReplace(words[1], words[2]);
                                }

                            break;
                            }

                        case DUK_ROMAN:
                            {
                            CurrentUser->SetRoman(atoi(words[1]));
                            break;
                            }

                        case DUK_SUPERSYSOP:
                            {
                            CurrentUser->SetSuperSysop(atoi(words[1]));
                            break;
                            }

                        case DUK_BUNNY:
                            {
                            CurrentUser->SetBunny(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_LOGONOFF:
                            {
                            CurrentUser->SetSELogOnOff(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_NEWMSG:
                            {
                            CurrentUser->SetSENewMessage(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_EXCLMSG:
                            {
                            CurrentUser->SetSEExclusiveMessage(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_CHATALL:
                            {
                            CurrentUser->SetSEChatAll(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_CHATROOM:
                            {
                            CurrentUser->SetSEChatRoom(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_CHATGROUP:
                            {
                            CurrentUser->SetSEChatGroup(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_CHATUSER:
                            {
                            CurrentUser->SetSEChatUser(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_RMINOUT:
                            {
                            CurrentUser->SetSERoomInOut(atoi(words[1]));
                            break;
                            }

                        case DUK_TERMTYPE:
                            {
                            CurrentUser->SetTermType(words[1]);
                            TermCap->Load(words[1]);
                            break;
                            }

                        case DUK_BANNED:
                            {
                            CurrentUser->SetBanned(atoi(words[1]));
                            break;
                            }

                        case DUK_KILLOWN:
                            {
                            CurrentUser->SetKillOwn(atoi(words[1]));
                            break;
                            }

                        case DUK_SEEOWNCHATS:
                            {
                            CurrentUser->SetSeeOwnChats(atoi(words[1]));
                            break;
                            }

                        case DUK_ERASEPROMPT:
                            {
                            CurrentUser->SetErasePrompt(atoi(words[1]));
                            break;
                            }

                        case DUK_AUTOIDLESECONDS:
                            {
                            CurrentUser->SetAutoIdleSeconds(atoi(words[1]));
                            break;
                            }

                        case DUK_SSE_MSGSOMEWHERE:
                            {
                            CurrentUser->SetHearLaughter(atoi(words[1]));
                            break;
                            }

                        default:
                            {
                            doccr();
                            cPrintf(getmsg(21), citfiles[C_DEFUSER_CIT], words[0]);
                            doccr();
                            break;
                            }
                        }
                    }
                }

            fclose(defusr);
            }

        discardData(du);
        }
    }
