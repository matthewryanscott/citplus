// --------------------------------------------------------------------------
// Citadel: Login.CPP
//
// Code to log in a user.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "log.h"
#include "msg.h"
#include "account.h"
#include "hall.h"
#include "events.h"
#include "cwindows.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"
#include "termwndw.h"
#include "cfgfiles.h"


// --------------------------------------------------------------------------
// Contents
//
// setroomgen()         sets room gen# with log gen
// setlbvisit()         sets lbvisit at log-in
// slideLTab()          crunches up slot, then frees slot at beginning, it then copies information to first slot
// minibin()            minibin log-in stats
// pwslot()             returns logtab slot password is in, else CERROR
// pwexists()           returns TRUE if password exists in logtable
// doLogin()            handles L(ogin) command


// --------------------------------------------------------------------------
// gotodefaulthall(): Goes to user's default hall.

h_slot LogEntry::GetDefaultHallIndex(void) const
    {
    label DefHall;
    GetDefaultHall(DefHall, sizeof(DefHall));

    if (*DefHall)
        {
        for (h_slot i = 0; i < cfg.maxhalls; ++i)
            {
            if (HallData[i].IsInuse() && HallData[i].IsSameName(DefHall) && CanAccessHall(i))
                {
                return (i);
                }
            }
        }

    // If the user doesn't have one, use the system default
    return (HallData.GetDefault());
    }


// --------------------------------------------------------------------------
// doLogin(): handles L(ogin) command

void TERMWINDOWMEMBER doLogin(Bool moreYet)
    {
    char InitPw[LABELSIZE+LABELSIZE+2];
    char password[LABELSIZE+LABELSIZE+2];
    char initials[LABELSIZE+LABELSIZE+2];
    long CreditsAtLastCall = 0L;

    OC.User.SetCanControlD(FALSE);

    SetDoWhat(LOGIN);

    compactMemory(1);

    CommPort->FlushInput();

    if (LockMessages(MSG_LOGIN))
        {
        if (login_user || login_pw)
            {
            // handle command line log-ins

            if (!modStat && cfg.offhook)
                {
                offhook();
                }

            if (login_pw)
                {
                // login using initials and pw
                normalizepw(cmd_login, initials, password);
                login_pw = FALSE;
                }
            else
                {
                // login using name
                l_slot Slot;

                normalizeString(cmd_login);

                if ((Slot = FindPersonByName(cmd_login)) != CERROR)
                    {
                    LogEntry1 Log1;

                    if (Log1.Load(LTab(Slot).GetLogIndex()))
                        {
                        Log1.GetInitials(initials, sizeof(initials));
                        Log1.GetPassword(password, sizeof(password));
                        login_user = FALSE;
                        }
                    }
                }
            }
        else
            {
            // ask user for initials and password

            if (moreYet == 2)
                {
                moreYet = FALSE;
                }
            else
                {
                // do not print Login when hitting 'L' from console mode
                if (!(!moreYet && !loggedIn && !modStat))
                    {
                    mPrintf(getmsg(MSG_LOGIN, 1));

                    if (!moreYet && !loggedIn)
                        {
                        doCR();
                        }
                    }
                }

            if (loggedIn)
                {
                CRCRmPrintfCR(getmsg(MSG_LOGIN, 2));
                UnlockMessages(MSG_LOGIN);
                return;
                }

            if (!modStat && cfg.offhook)
                {
                offhook();
                }

            getNormStr(moreYet ? ns : getmsg(MSG_LOGIN, 3), InitPw, LABELSIZE + LABELSIZE + 1, FALSE);

            if (HaveConnectionToUser())
                {
                dospCR();
                }

            const char *semicolon = strchr(InitPw, ';');

            if (!semicolon)
                {
                strcpy(initials, InitPw);
                getNormStr(getmsg(MSG_LOGIN, 4), password, LABELSIZE, FALSE);
                dospCR();
                }
            else
                {
                normalizepw(InitPw, initials, password);
                }

            // do not allow anything over LABELSIZE characters
            initials[LABELSIZE] = 0;

            if (!HaveConnectionToUser() && (*initials || *password))
                {
                UnlockMessages(MSG_LOGIN);

                if(!read_tr_messages())
                    {
                    errorDisp(getmsg(172));
                    }
#ifdef WINCIT
                trap(T_HACK, WindowCaption, gettrmsg(35), initials, password, WindowCaption);
#else
                trap(T_HACK, gettrmsg(35), initials, password);
#endif
                dump_tr_messages();
                return;
                }
            }

        CommPort->ResetTransmitted();
        CommPort->ResetReceived();

        MS.Read = 0;
        MS.Entered = 0;

        MS.AutoMKC = AM_NONE;
        MS.MarkedID = 0L;

        const l_slot FoundSlot = pwslot(initials, password);

#ifdef WINCIT
        Bool AllowLogin = TRUE;

        if (FoundSlot != CERROR)
            {
            if (LoginList.IsLoggedIn(LTab(FoundSlot).GetLogIndex()))
                {
                if (LTab(FoundSlot).IsNode() || !getYesNo(getmsg(MSG_LOGIN, 79), 0))
                    {
                    AllowLogin = FALSE;
                    }
                }

            if (AllowLogin)
                {
                if (!LoginList.Add(LTab(FoundSlot).GetLogIndex()))
                    {
                    mPrintfCR(getmsg(MSG_LOGIN, 78));
                    AllowLogin = FALSE;
                    }
                }

            if (!AllowLogin)
                {
                UnlockMessages(MSG_LOGIN);
                return;
                }
            }
#endif

        if (FoundSlot != CERROR && *password)
            {
            ThisSlot = FoundSlot;
            ThisLog = LTab(FoundSlot).GetLogIndex();

            if (CurrentUser->Load(ThisLog))
                {
                label Buffer;
                if (!TermCap->Load(CurrentUser->GetTermType(Buffer, sizeof(Buffer))))
                    {
                    mPrintfCR(getmsg(MSG_LOGIN, 40), CurrentUser->GetTermType(Buffer, sizeof(Buffer)));
                    }
                else
                    {
                    // We keep the old stuff up-to-date in case someone
                    // downgrades to /065 or earlier.

                    CurrentUser->SetOldIBMGraph(TermCap->IsIBMExtended());
                    CurrentUser->SetOldIBMANSI(TermCap->IsANSI());
                    CurrentUser->SetOldIBMColor(TermCap->IsColor());
                    }

                // record login time, date
                time(&logtimestamp);

                loggedIn = TRUE;
                StatusLine.Update(WC_TWp);

                // trap it
                if (!CurrentUser->IsNode())
                    {
                    if (onConsole)
                        {
                        UnlockMessages(MSG_LOGIN);
                        if(!read_tr_messages())
                            {
                            errorDisp(getmsg(172));
                            }
#ifdef WINCIT
                        trap(T_LOGIN, WindowCaption, gettrmsg(32));
#else
                        trap(T_LOGIN, gettrmsg(32));
#endif
                        dump_tr_messages();
                        LockMessages(MSG_LOGIN);
                        }

                    doEvent(EVT_LOGIN);
                    UnlockMessages(MSG_LOGIN);
                    if(!read_tr_messages())
                        {
                        errorDisp(getmsg(172));
                        }
#ifdef WINCIT
                    trap(T_LOGIN, WindowCaption, gettrmsg(33), CurrentUser->GetName(Buffer, sizeof(Buffer)), WindowCaption);
#else
                    trap(T_LOGIN, gettrmsg(33), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif
                    dump_tr_messages();
                    LockMessages(MSG_LOGIN);
                    }
                else
                    {
                    UnlockMessages(MSG_LOGIN);
                    if(!read_tr_messages())
                        {
                        errorDisp(getmsg(172));
                        }
#ifdef WINCIT
                    trap(T_NETWORK, WindowCaption, gettrmsg(34), CurrentUser->GetName(Buffer, sizeof(Buffer)), WindowCaption);
#else
                    trap(T_NETWORK, gettrmsg(34), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif
                    dump_tr_messages();
                    LockMessages(MSG_LOGIN);
                    }
                }
            }
        else
            {
            loginNew(initials, password);
            login_user = FALSE;
            }

        if (!loggedIn)
            {
            UnlockMessages(MSG_LOGIN);
            return;
            }

        delete MS.AbortedMessage;
        MS.AbortedMessage = NULL;

        setlbvisit();

        if (CurrentUser->IsPrintFile())
            {
            OC.WasPrinting = OC.Printing;

            if (!OC.Printing)
                {
#ifdef WINCIT
                sprintf(OC.PrintfileName, "%s\\%s.%ld", cfg.homepath, cfg.printer, GetCurrentThreadId());
#else
                sprintf(OC.PrintfileName, sbs, cfg.homepath, cfg.printer);
#endif
                OC.PrintFile = fopen(OC.PrintfileName, FO_A);
                }

            if (OC.PrintFile)
                {
                OC.Printing = TRUE;
                label Buffer;
                fprintf(OC.PrintFile, getmsg(MSG_LOGIN, 9), bn, bn, cfg.Luser_nym,
                        CurrentUser->GetName(Buffer, sizeof(Buffer)), bn, bn);
                }
            }

        const int CallersSinceLastCall = ThisSlot;

#ifdef WINCIT
        slideLTab(&ThisSlot, CurrentUser, LogOrder);
#else
        slideLTab(&ThisSlot, CurrentUser);
#endif
        cfg.callno++;

        // User can't log in now.
        if (!CurrentUser->IsVerified() && !onConsole)
            {
            dispBlb(B_VERIFIED);

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }

            UnlockMessages(MSG_LOGIN);
            return;
            }

#ifdef WINCIT
        if (CurrentUser->IsBanned() && !onConsole)
            {
            dispBlb(B_BANNED);

            char ec[200];
            sprintf(ec, sbs, cfg.homepath, citfiles[C_EXTERNAL_CIT]);

            label Name;
            CurrentUser->GetName(Name, sizeof(Name));

            FILE *fBuf = fopen(ec, FO_A);
            if (fBuf)
                {
                fprintf(fBuf, getmsg(MSG_LOGIN, 81), bn, Name, bn);
                fprintf(fBuf, getmsg(MSG_LOGIN, 82), WindowCaption, bn);
                fclose(fBuf);

                trap(T_HACK, WindowCaption, getmsg(MSG_LOGIN, 83), WindowCaption, Name);
                }
            else
                {
                trap(T_HACK, WindowCaption, getmsg(MSG_LOGIN, 84), WindowCaption, citfiles[C_EXTERNAL_CIT], Name);
                }

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }

            UnlockMessages(MSG_LOGIN);
            return;
            }
#endif

        // User can't log in now.
        if (cfg.accounting && CurrentUser->IsAccounting())
            {
            CurrentUserAccount->Negotiate(CurrentUser);
            CreditsAtLastCall = (cfg.accounting && CurrentUser->IsAccounting()) ? CurrentUser->GetCredits() : -1L;
            CurrentUserAccount->GiveDailyIncrement();

            if (!CurrentUserAccount->MayLogin())
                {
                dispBlb(B_NOLOGIN);
                loggedIn = FALSE;   // setting loggedIn to FALSE here keeps terminate() from doing lots of trapping...
                MRO.Verbose = FALSE;
                terminate(TRUE);

#ifdef MULTI
                // ...however, it also keeps terminate() from removing the login from the LoginList
                LoginList.Remove(ThisLog);
#endif

                CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

                Hangup();

                if (w)
                    {
                    destroyCitWindow(w, FALSE);
                    }

                UnlockMessages(MSG_LOGIN);
                return;
                }
            }

        for (userapps *theUserapp = userAppList; theUserapp; theUserapp = (userapps *) getNextLL(theUserapp))
            {
            if (CurrentUser->IsSameName(theUserapp->name))
                {
                mPrintfCR(pcts, theUserapp->outstr);

                RunApplication(theUserapp->cmd, NULL, TRUE, TRUE);

                if (theUserapp->hangup)
                    {
                    CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

                    Hangup();

                    if (w)
                        {
                        destroyCitWindow(w, FALSE);
                        }

                    UnlockMessages(MSG_LOGIN);
                    return;
                    }
                }
            }

#ifdef MULTI
        char LoginEvent[256];
        label NameBuffer;

        sprintf(LoginEvent, getmsg(MSG_LOGIN, 80), CurrentUser->GetName(NameBuffer, sizeof(NameBuffer)));
        TermWindowCollection.SystemEvent(SE_LOGONOFF, TRUE, NULL, FALSE, LoginEvent);
#endif

        if (CurrentUser->IsNode())
            {
            UnlockMessages(MSG_LOGIN);
            return;
            }

        struct time tm;
        struct date dtnow, dtthen;
        unixtodos(time(NULL), &dtnow, &tm);
        unixtodos(CurrentUser->GetCallTime(), &dtthen, &tm);

        if (!memcmp(&dtnow, &dtthen, sizeof(dtnow)))
            {
            CurrentUser->SetCallsToday(CurrentUser->GetCallsToday() + 1);
            }
        else
            {
            CurrentUser->SetCallsToday(1);
            }

        if ((CurrentUser->GetCallLimit() && CurrentUser->GetCallsToday() > CurrentUser->GetCallLimit()) ||
                (cfg.callLimit && CurrentUser->GetCallsToday() > cfg.callLimit))
            {
            dispBlb(B_TOOMANY);

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }

            UnlockMessages(MSG_LOGIN);
            return;
            }

        thisHall = CurrentUser->GetDefaultHallIndex();

        const Bool isJoke = BLBRotate(getmsg(MSG_LOGIN, 10), getmsg(355), &cfg.normJoke, &cfg.ansiJoke, 1, MAXBLBS) !=
                CERROR;

        if (!isJoke)
            {
            mPrintfCR(getmsg(MSG_LOGIN, 11));
            }

        Talley->Fill();

        if (isJoke)
            {
            BLBRotate(getmsg(MSG_LOGIN, 10), getmsg(355), &cfg.normJoke, &cfg.ansiJoke, 2, MAXBLBS);
            }

        MRO.Verbose = CurrentUser->IsAutoVerbose();

        // reverse engineering Minibin?!?!
        if (CurrentUser->IsMinibin())
            {
            minibin(CallersSinceLastCall, CreditsAtLastCall);
            }

        if (cfg.pwDays && ((time(NULL) - CurrentUser->GetPasswordChangeTime()) / (24*60*60)) > cfg.pwDays)
            {
            CRmPrintfCR(getmsg(MSG_LOGIN, 12));
            newPW(FALSE);
            }

        if (CurrentUser->GetNumUserShow())
            {
            resetMsgfilter(FALSE);
            DoCommand(UC_READUSERLOG, FALSE, OldAndNew, CurrentUser->GetNumUserShow());
            }

        // reset chats
        nochat(TRUE);

        roomsMade = 0;

        doReadBull(FALSE);

        if (CurrentUser->GetMessage())
            {
            char string[128];

            dispBlb(B_RESUME);

            sprintf(string, getmsg(MSG_LOGIN, 13), cfg.Lmsg_nym);

            if (getYesNo(string, 0))
                {
                Message *Msg = new Message;

                if (Msg)
                    {
                    *Msg = *CurrentUser->GetMessage();
                    CurrentUser->SetMessage(NULL);

                    // relocate for anon
                    const r_slot currentRoom = thisRoom;
                    thisRoom = CurrentUser->GetMessageRoom();

                    if (*Msg->GetToUser())
                        {
                        OC.Echo = CALLER;
                        OC.setio();
                        }

                    makeMessage(Msg, NULL, CurrentUser->GetMessageRoom());

                    OC.Echo = BOTH;
                    OC.setio();

                    delete Msg;

                    // and come back
                    thisRoom = currentRoom;
                    }
                else
                    {
                    OutOfMemory(0);
                    }
                }
            else
                {
                doCR();

                assert(!MS.AbortedMessage);
                MS.AbortedMessage = new Message;

                if (MS.AbortedMessage)
                    {
                    *MS.AbortedMessage = *CurrentUser->GetMessage();
                    mPrintfCR(getmsg(MSG_LOGIN, 14), cfg.Umsg_nym);
                    }
                else
                    {
                    OutOfMemory(0);
                    }
                }

            CurrentUser->SetMessage(NULL);
            }

        // hmmm... where to put this
        if (RoomTab[thisRoom].IsApplication() && CurrentRoom->IsAutoApp())
            {
            ExeAplic();
            }

        // why???
        if (CurrentUser->IsClearScreenBetweenMessages())
            {
            doCR();
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }

        MRO.Clear(CurrentUser->IsAutoVerbose());
        resetMsgfilter(TRUE);
        showMessages(NewOnly, FALSE, 0);

        MRO.Verbose = FALSE;
        if (CurrentUser->IsExpert())
            {
            listRooms(K_NEW);
            }
        else
            {
            listRooms(0);
            }

        OC.SetOutFlag(OUTOK);

        storeLog();

        UnlockMessages(MSG_LOGIN);
        }
    else
        {
        OutOfMemory(0);
        }
    }

// --------------------------------------------------------------------------
// setlbvisit(): Sets lbvisit at log-in.

void TERMWINDOWMEMBER setlbvisit(void)
    {
    // see if the message base was cleared since last call
    m_index Newest = MessageDat.NewestMessage();

    for (r_slot i = 0; i < cfg.maxrooms; i++)
        {
        if (CurrentUser->GetRoomNewPointer(i) > Newest)
            {
            for (i = 0; i < cfg.maxrooms; i++)
                {
                CurrentUser->SetRoomNewPointer(i, 1);
                }

            CRmPrintfCR(getmsg(575), cfg.Umsg_nym);
            mPrintfCR(getmsg(576), cfg.Lmsg_nym);
            break;
            }
        }

    Talley->ResetBypassedAndEntered();
    }


// --------------------------------------------------------------------------
// slideLTab(): Crunches up slot, then frees slot at beginning. It then
//  copies information to first slot.
// slot is current tabslot being moved; reset to 0

#ifdef WINCIT
void slideLTab(l_slot *slot, const LogEntry1 *Entry, l_slot *LogOrder)
#else
void slideLTab(l_slot *slot, const LogEntry1 *Entry)
#endif
    {
    // if at the top - stay there
    if (!*slot)
        {
        return;
        }

    const l_slot ourIndex = LTab(*slot).GetLogIndex();

    // move table down, then insert our log info at the beginning of table
#ifdef WINCIT
    l_slot S = LogOrder[*slot];
    LogTab.SlideTableDown(*slot, LogOrder);
    LogOrder[0] = S;
    LogTab.Update(S, Entry, ourIndex);
#else
    LogTab.SlideTableDown(*slot);
    LogTab.Update(0, Entry, ourIndex);
#endif

    *slot = 0;
    }


// --------------------------------------------------------------------------
// minibin(): Minibin log-in stats.
//
// Notes:
//  Assumes MSG_LOGIN is locked.

void TERMWINDOWMEMBER minibin(int CallersSinceLastCall, long CreditsAtLastCall)
    {
    char dtstr[80];

    m_index Newest = MessageDat.NewestMessage();

    const m_index messages = (Newest >= CurrentUser->GetLastMessage()) ? (Newest - CurrentUser->GetLastMessage()) : 0;

    const long calls = (cfg.callno > CurrentUser->GetCallNumber()) ? (cfg.callno - 1 - CurrentUser->GetCallNumber()) : -1;

    if (!CurrentUser->IsExpert())
        {
        CRmPrintfCR(getmsg(577));
        }

    CRmPrintf((calls > -1l) ? getmsg(MSG_LOGIN, 15) : getmsg(MSG_LOGIN, 16));

    label Buffer;
    if (cfg.titles && *CurrentUser->GetTitle(Buffer, sizeof(Buffer)) && CurrentUser->IsViewTitleSurname())
        {
        mPrintf(getmsg(269), Buffer);
        }

    mPrintf(getmsg(272), CurrentUser->GetName(Buffer, sizeof(Buffer)));

    if (cfg.surnames && *CurrentUser->GetSurname(Buffer, sizeof(Buffer)) && CurrentUser->IsViewTitleSurname())
        {
        mPrintf(getmsg(273), Buffer);
        }

    mPrintfCR(getmsg(MSG_LOGIN, 17));

    char Buffer1[64];
    strftime(dtstr, 79, (loggedIn) ?
            CurrentUser->GetVerboseDateStamp(Buffer1, sizeof(Buffer1)) : cfg.vdatestamp, time(NULL));

    mPrintfCR(getmsg(MSG_LOGIN, 18), dtstr);

    if (calls != -1)
        {
        strftime(dtstr, 79, (loggedIn) ?
                CurrentUser->GetVerboseDateStamp(Buffer1, sizeof(Buffer1)) : cfg.vdatestamp, CurrentUser->GetCallTime());

        if (CurrentUser->GetCallTime() < time(NULL))
            {
            mPrintfCR(getmsg(MSG_LOGIN, 19), dtstr, diffstamp(CurrentUser->GetCallTime()));
            }
        else
            {
            mPrintfCR(getmsg(MSG_LOGIN, 20), dtstr);
            }

        if (calls == 0)
            {
            mPrintfCR(getmsg(MSG_LOGIN, 21));
            }
        else
            {
            mPrintfCR(getmsg(MSG_LOGIN, 22), ltoac(cfg.callno));

            mPrintfCR(getmsg(MSG_LOGIN, 23), ltoac(CallersSinceLastCall),
                    CallersSinceLastCall == 1 ? getmsg(MSG_LOGIN, 24) : getmsg(MSG_LOGIN, 25));

            mPrintfCR(getmsg(MSG_LOGIN, 26), ltoac(calls), calls == 1 ? getmsg(MSG_LOGIN, 27) : getmsg(MSG_LOGIN, 28));

            mPrintfCR(getmsg(MSG_LOGIN, 29), ltoac(messages), (messages == 1) ? cfg.Lmsg_nym : cfg.Lmsgs_nym);
            }
        }

    Bool *roomCheck = new Bool[cfg.maxrooms];
    if (roomCheck)
        {
        ulong newMsgs = 0;

        theOtherAlgorithm(roomCheck, thisHall, TRUE);

        for (r_slot rmCnt = 0; rmCnt < cfg.maxrooms; rmCnt++)
            {
            if (roomCheck[rmCnt] && !CurrentUser->IsRoomExcluded(rmCnt))
                {
                newMsgs += Talley->NewInRoom(rmCnt);
                }
            }

        mPrintfCR(getmsg(MSG_LOGIN, 30), ltoac(newMsgs), newMsgs == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym);
        if (newMsgs == 0)
            {
            seenAllRooms = TRUE;
            }

        delete [] roomCheck;
        }

    if (cfg.accounting && CurrentUser->IsAccounting())
        {
        if (!CurrentUserAccount->IsSpecialTime())
            {
            const long C = CurrentUser->GetCredits() / 60;

            mPrintf(getmsg(MSG_LOGIN, 31), ltoac(C), (C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);

            const long CreditDifferenceDiv60 = C - (CreditsAtLastCall / 60);

            if (CreditDifferenceDiv60 > 0)
                {
                mPrintfCR(getmsg(MSG_LOGIN, 32), ltoac(CreditDifferenceDiv60),
                        (CreditDifferenceDiv60 == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
                }
            else if (CreditDifferenceDiv60 < 0)
                {
                mPrintfCR(getmsg(MSG_LOGIN, 33), ltoac(-CreditDifferenceDiv60),
                        (CreditDifferenceDiv60 == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
                }
            else
                {
                mPrintfCR(getmsg(MSG_LOGIN, 34));
                }
            }
        else
            {
            mPrintfCR(getmsg(MSG_LOGIN, 35), cfg.Lcredits_nym);
            }
        }

    OC.SetOutFlag(OUTOK);
    }


// --------------------------------------------------------------------------
// pwslot(): Returns logtab slot password is in, else CERROR.
#ifdef WINCIT
l_slot TermWindowC::pwslot(const char *in, const char *pw)
    {
    return (::pwslot(in, pw, LogOrder));
    }

l_slot pwslot(const char *in, const char *pw, const l_slot *LogOrder)
#else
l_slot pwslot(const char *in, const char *pw)
#endif
    {
    if (strlen(pw) < 2)
        {
        return (CERROR);    // Don't search for int pwds
        }

#ifdef WINCIT
    const l_slot slot = FindPwInHashInTable(in, pw, LogOrder);
#else
    const l_slot slot = FindPwInHashInTable(in, pw);
#endif

    if (slot != CERROR)
        {
        LogEntry1 Log1;

        if (Log1.Load(LTab(slot).GetLogIndex()))
            {
            label Buffer, Buffer1;
            if (SameString(pw, Log1.GetPassword(Buffer1, sizeof(Buffer1))) &&
                    SameString(in, Log1.GetInitials(Buffer, sizeof(Buffer))))
                {
                return (slot);
                }
            }
        }

    return (CERROR);
    }


// --------------------------------------------------------------------------

#ifdef WINCIT
l_slot TermWindowC::FindPwInHashInTable(const char *in, const char *pw)
    {
    return (::FindPwInHashInTable(in, pw, LogOrder));
    }

l_slot FindPwInHashInTable(const char *in, const char *pw, const l_slot *LogOrder)
#else
l_slot FindPwInHashInTable(const char *in, const char *pw)
#endif
    {
    const int inhash = hash(in);
    const int pwhash = hash(pw);

    for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
        {
        if (pwhash == LTab(i).GetPasswordHash() && inhash == LTab(i).GetInitialsHash())
            {
            return (i);
            }
        }

    return (CERROR);
    }
