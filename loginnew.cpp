// --------------------------------------------------------------------------
// Citadel: LoginNew.CPP
//
// Overlayed new user login log code.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "msg.h"
#include "account.h"
#include "maillist.h"
#include "events.h"
#include "filecmd.h"
#include "cwindows.h"
#include "term.h"
#include "miscovl.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// loginNew()       login a new user, handle diffrent configs.
// newUser()        prompts for name and password
// newUserFile()    Writes new user info out to a file
// newlog()         sets up a new log entry for new users returns CERROR
//                      if cannot find a usable slot
// newslot()        attempts to find a slot for a new user to reside in
//                      puts slot in global var thisSlot


#ifdef WINCIT
static l_slot newslot(const l_slot *LogOrder);
#else
static l_slot newslot(void);
#endif


// --------------------------------------------------------------------------
// loginNew(): Login a new user, handle diffrent configs.
//
// Notes:
//  Assumes MSG_LOGIN is locked.

void TERMWINDOWMEMBER loginNew(char *initials, char *password)
    {
    char string[128];

    sprintf(string, getmsg(MSG_LOGIN, 36), cfg.Luser_nym);

    if (getYesNo(cfg.l_verified ? string : getmsg(MSG_LOGIN, 37), 1))
        {
        if (!HaveConnectionToUser())
            {
            if (!login_user && (*initials || *password))
                {
                UnlockMessages(MSG_LOGIN);
                if(!read_tr_messages())
                    {
                    errorDisp(getmsg(172));
                    return;
                    }
#ifdef WINCIT
                switch(CommPort->GetType())
                    {
                    case CT_TELNET:
                        {
                        if(cfg.trapit[T_PORT])
                            {
                            trap(T_HACK, WindowCaption, gettrmsg(29), initials, password);
                            }
                        else if(!(cfg.trapit[T_PORT]))
                            {
                            trap(T_HACK, ns, gettrmsg(28), initials, password, gettrmsg(8), WindowCaption);
                            }
                        break;
                        }

                    case CT_SERIAL:
                        {
                        if(cfg.trapit[T_PORT])
                            {
                            trap(T_HACK, WindowCaption, gettrmsg(29), initials, password);
                            }
                        else if(!(cfg.trapit[T_PORT]))
                            {
                            trap(T_HACK, ns, gettrmsg(28), initials, password, gettrmsg(9), WindowCaption);
                            }
                        break;
                        }

                    default:
                        {
                        trap(T_HACK, WindowCaption, gettrmsg(28), initials, password, gettrmsg(8), WindowCaption);
                        break;
                        }

                    }
#else
                trap(T_HACK, gettrmsg(29), initials, password);
#endif
                dump_tr_messages();
                LockMessages(MSG_LOGIN);
                }
            return;
            }

        if (cfg.l_closedsys && !sysopNew && !(onConsole && !debug))
            {
            dispBlb(B_CLOSESYS);

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }

            return;
            }

        dispBlb(B_USERINFO);

        if (cfg.l_create || sysopNew || (onConsole && !debug))
            {
            newUser(initials, password);

            CurrentUser->SetVerified((onConsole && !debug) ?
                    TRUE : cfg.l_verified);

            if (!loggedIn)
                {
                return;
                }

            if (cfg.accounting && CurrentUser->IsAccounting())
                {
                CurrentUserAccount->CreateAccount();
                }

            StatusLine.Update(WC_TWp);
            }

        if ((cfg.NUQuest[0] != NUQ_NUMQUEST) && (!(onConsole && !debug)))
            {
            newUserFile();
            }

        if (*cfg.newuserapp && (!(onConsole && !debug)))
            {
            RunApplication(cfg.newuserapp, NULL, TRUE, TRUE);
            }

        if (cfg.l_sysop_msg && (!(onConsole && !debug)))
            {
            dispBlb(B_NEWMSG);

            Message *Msg = new Message;

            if (Msg)
                {
                // to sysop
                Msg->SetToUser(getmsg(386));

                makeMessage(Msg, NULL, MAILROOM);

                delete Msg;
                }
            else
                {
                OutOfMemory(2);
                }
            }

        if (!CurrentUser->IsVerified() && !sysopNew && loggedIn)
            {
            MRO.Verbose = FALSE;
            terminate(TRUE);
            CRmPrintfCR(getmsg(MSG_LOGIN, 41));

            CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

            Hangup();

            if (w)
                {
                destroyCitWindow(w, FALSE);
                }
            }

        if (cfg.oldcount)
            {
            ulong newpointer;

            if (MessageDat.OldestMessageInFile() + cfg.oldcount <= MessageDat.NewestMessage())
                {
                newpointer = (MessageDat.NewestMessage() - cfg.oldcount);

                for (r_slot i = 0; i < cfg.maxrooms; i++)
                    {
                    CurrentUser->SetRoomNewPointer(i, newpointer);
                    }
                }
            }
        }
    else
        {
        if (!login_user && (*initials || *password))
            {
            UnlockMessages(MSG_LOGIN);
            if(!(read_tr_messages()))
                {
                errorDisp(getmsg(172));
                return;
                }
#ifdef WINCIT
            switch(CommPort->GetType())
                {
                case CT_TELNET:
                    {
                    if(cfg.trapit[T_PORT])
                        {   
                        trap(T_HACK, WindowCaption, gettrmsg(29), initials, password);
                        }
                    else if(!(cfg.trapit[T_PORT]))
                        {
                        trap(T_HACK, ns, gettrmsg(28), initials, password, gettrmsg(8), WindowCaption);
                        }
                    break;
                    }

                case CT_SERIAL:
                    {
                    if(cfg.trapit[T_PORT])
                        {   
                        trap(T_HACK, WindowCaption, gettrmsg(29), initials, password);
                        }
                    else if(!(cfg.trapit[T_PORT]))
                        {
                        trap(T_HACK, ns, gettrmsg(28), initials, password, gettrmsg(9), WindowCaption);
                        }
                    break;
                    }

                default:
                    {
                    trap(T_HACK, WindowCaption, gettrmsg(28), initials, password, gettrmsg(8), WindowCaption);
                    break;
                    }
                }
#else
            trap(T_HACK, gettrmsg(29), initials, password);
#endif
            dump_tr_messages();
            LockMessages(MSG_LOGIN);
            }
        }

    sysopNew = FALSE;
    }


// --------------------------------------------------------------------------
// newUser(): Prompts for name and password.
//
// Notes:
//  Assumes MSG_LOGIN is locked.

void TERMWINDOWMEMBER newUser(char *initials, char *password)
    {
    label fullnm;
    char InitPw[62];

    Bool askpw = FALSE;

    if (login_user)
        {
        askpw = TRUE;
        }

    if (!HaveConnectionToUser())
        {
        return;
        }

    // this only initializes CurrentUser, does not write it out
    if (!newlog())
        {
        return;
        }

    // Give new users a chance to configure themselves
    EnterConfiguration(TRUE);

    dispBlb(B_PASSWORD);
    doCR();

    int abort;
    do
        {
        Bool good;

        if (!login_user) // Use the name entered on Command Line
            {
            do
                {
                getNormStr(cfg.enter_name, fullnm, LABELSIZE);
                strip_special(fullnm);

                if (!(*fullnm) ||
                        (FindPersonByName(fullnm) != CERROR) || SameString(fullnm, getmsg(529)) ||
                        SameString(fullnm, getmsg(386)) || SameString(fullnm, getmsg(385)) ||
                        SameString(fullnm, cfg.nodeTitle) || SameString(fullnm, getmsg(360)) ||
                        inExternal(getmsg(353), fullnm) || IsMailingList(fullnm))
                    {
                    CRmPrintfCR(getmsg(475), fullnm);
                    good = FALSE;
                    }
                else
                    {
                    good = TRUE;
                    }
                } while (!good && (HaveConnectionToUser()));
            }
        else
            {
            strcpy(fullnm, cmd_login);
            }

        if (!HaveConnectionToUser())
            {
            return;
            }

        do
            {
            if (askpw)
                {
                getNormStr(getmsg(MSG_LOGIN, 3), InitPw, 61, FALSE);
                dospCR();

                char *semicolon = strchr(InitPw, ';');

                if (semicolon)
                    {
                    normalizepw(InitPw, initials, password);
                    }
                else
                    {
                    strcpy(initials, InitPw);
                    }

                // dont allow anything over LABELSIZE characters
                initials[LABELSIZE] = 0;

                if (!semicolon)
                    {
                    getNormStr(getmsg(MSG_LOGIN, 4), password, LABELSIZE,
                            FALSE);
                    dospCR();
                    }
                }
            askpw = TRUE;

            if (FindPwInHashInTable(initials, password) != CERROR ||
                    strlen(password) < 2)
                {
                good = FALSE;
                CRmPrintfCR(getmsg(605));
                }
            else
                {
                good = TRUE;
                }

            } while (!good && (HaveConnectionToUser()));

        CurrentUser->SetPasswordChangeTime(time(NULL));

        displaypw(fullnm, initials, password);

        abort = getYesNo(getmsg(MSG_LOGIN, 42), 2);

        if (abort == 2)
            {
            return; // check for Abort at (Y/N/A)[A]:
            }
        } while (!abort && (HaveConnectionToUser()));

    if (HaveConnectionToUser())
        {
        CurrentUser->SetName(fullnm);
        CurrentUser->SetInitials(initials);
        CurrentUser->SetPassword(password);

        CurrentUser->SetCallTime(time(NULL));
        CurrentUser->SetFirstOn(CurrentUser->GetCallTime());

        // trap it
        if (onConsole)
            {
            UnlockMessages(MSG_LOGIN);
            if(!read_tr_messages())
                {
                errorDisp(getmsg(172));
                }

#ifdef WINCIT
//          trap(T_LOGIN, WindowCaption, gettrmsg(32), T_LOGIN);
//          why does this have an extra T_LOGIN??
            trap(T_LOGIN, WindowCaption, gettrmsg(32));
#else
            trap(T_LOGIN, gettrmsg(32), T_LOGIN);
#endif
            dump_tr_messages();
            LockMessages(MSG_LOGIN);
            }

        label Buffer;
#ifdef WINCIT
        trap(T_LOGIN, WindowCaption, getmsg(MSG_LOGIN, 43), CurrentUser->GetName(Buffer, sizeof(Buffer)), WindowCaption);
#else
        trap(T_LOGIN, getmsg(MSG_LOGIN, 43), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

        doEvent(EVT_NEWUSER);   // follow trap, 'cuz timing is everything

        time(&logtimestamp);
        loggedIn = TRUE;
#ifdef WINCIT
        slideLTab(&ThisSlot, CurrentUser, LogOrder);
#else
        slideLTab(&ThisSlot, CurrentUser);
#endif

        storeLog();
        }
    }


// --------------------------------------------------------------------------
// newUserFile(): Writes new user info out to a file.

void TERMWINDOWMEMBER newUserFile(void)
    {
    label FileName, Slot;
    char FullFileName[128];

    sprintf(Slot, getmsg(MSG_LOGIN, 77), ThisLog);
    sformat(FileName, cfg.newuserLogFileName, getmsg(MSG_LOGIN, 58), Slot, 0);

    sprintf(FullFileName, sbs, cfg.homepath, FileName);

    FILE *fl;
    if ((fl = fopen(FullFileName, FO_A)) != NULL)
        {
        char dtstr[85];
        strftime(dtstr, 79, cfg.vdatestamp, 0l);

        fprintf(fl, bn);
        fprintf(fl, pcts, dtstr);
        fprintf(fl, bn);
        label Buffer;
        fprintf(fl, getmsg(MSG_LOGIN, 45), deansi(CurrentUser->GetName(Buffer, sizeof(Buffer))));
        fprintf(fl, bn);
        fprintf(fl, getmsg(MSG_LOGIN, 46), connectbauds[CommPort->GetModemSpeed()]);
        fprintf(fl, bn);
        }
    else
        {
        cPrintf(getmsg(78), FullFileName);
        doccr();
        }


    dispBlb(B_NEWQUEST);

    for (int QIndex = 0; cfg.NUQuest[QIndex] != NUQ_NUMQUEST && QIndex < NUQ_NUMQUEST; QIndex++)
        {
        switch (cfg.NUQuest[QIndex])
            {
            case NUQ_FINGER:
                {
                dispBlb(B_NEWQFNGR);
                FingerEdit();
                break;
                }

            case NUQ_REALNAME:
                {
                label name;

                do
                    {
                    GetStringWithBlurb(getmsg(MSG_LOGIN, 47), name, LABELSIZE, ns, B_REALNAME);

                    normalizeString(name);
                    } while (!*name && HaveConnectionToUser());

                CurrentUser->SetRealName(name);

                if (fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 48), deansi(name));
                    fprintf(fl, bn);
                    }

                break;
                }

            case NUQ_PHONENUMBER:
                {
                label phone;

                do
                    {
                    GetStringWithBlurb(getmsg(MSG_LOGIN, 49), phone, LABELSIZE, ns, B_PHONENUM);

                    normalizeString(phone);
                    } while (!*phone && HaveConnectionToUser());

                CurrentUser->SetPhoneNumber(phone);

                if (fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 50), deansi(phone));
                    fprintf(fl, bn);
                    }

                break;
                }

            case NUQ_ADDRESS:
                {
                label addr1, addr2, addr3;

                *addr2 = 0;
                *addr3 = 0;

                GetStringWithBlurb(getmsg(MSG_LOGIN, 51), addr1, LABELSIZE, ns, B_ADDRESS);

                normalizeString(addr1);
                if (*addr1)
                    {
                    GetStringWithBlurb(getmsg(MSG_LOGIN, 52), addr2, LABELSIZE, ns, B_ADDRESS);

                    normalizeString(addr2);
                    if (*addr2)
                        {
                        GetStringWithBlurb(getmsg(MSG_LOGIN, 53), addr3, LABELSIZE, ns, B_ADDRESS);
                        }
                    }

                CurrentUser->SetMailAddr1(addr1);
                CurrentUser->SetMailAddr2(addr2);
                CurrentUser->SetMailAddr3(addr3);


                if (*addr1 && fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 54), deansi(addr1));
                    fprintf(fl, bn);

                    if (*addr2)
                        {
                        fprintf(fl, getmsg(MSG_LOGIN, 54), deansi(addr2));
                        fprintf(fl, bn);

                        if (*addr3)
                            {
                            fprintf(fl, getmsg(MSG_LOGIN, 54), deansi(addr3));
                            fprintf(fl, bn);
                            }
                        }
                    }


                break;
                }

            case NUQ_TITLE:
                {
                if (cfg.titles)
                    {
                    label title;

                    GetStringWithBlurb(getmsg(MSG_LOGIN, 55), title, LABELSIZE, ns, B_TITLESUR);

                    normalizeString(title);
                    CurrentUser->SetTitle(title);

                    if (*title && fl)
                        {
                        fprintf(fl, getmsg(MSG_LOGIN, 56), deansi(title));
                        fprintf(fl, bn);
                        }
                    }
                else
                    {
                    cPrintf(getmsg(MSG_LOGIN, 57), getmsg(MSG_LOGIN, 58));
                    doccr();
                    }

                break;
                }

            case NUQ_SURNAME:
                {
                if (cfg.surnames)
                    {
                    label surname;

                    GetStringWithBlurb(getmsg(MSG_LOGIN, 59), surname, LABELSIZE, ns, B_TITLESUR);

                    normalizeString(surname);
                    CurrentUser->SetSurname(surname);

                    if (*surname && fl)
                        {
                        fprintf(fl, getmsg(MSG_LOGIN, 60), deansi(surname));
                        fprintf(fl, bn);
                        }
                    }
                else
                    {
                    cPrintf(getmsg(MSG_LOGIN, 61), getmsg(MSG_LOGIN, 58));
                    doccr();
                    }

                break;
                }

            case NUQ_MESSAGE:
                {
                Message *Msg = new Message;

                if (Msg)
                    {
                    dispBlb(B_NEWQMSG);

                    Msg->ClearAll();

                    Msg->SetAuthor(CurrentUser);
                    Msg->SetSurname(CurrentUser);
                    Msg->SetTitle(CurrentUser);

                    Msg->SetToUser(getmsg(386));

                    getText(Msg, NULL);
                    stripansi(Msg->GetTextPointer());
                    }
                else
                    {
                    OutOfMemory(3);
                    }

                if (fl)
                    {
                    if (Msg)
                        {
                        dFormat(Msg->GetText(), fl);
                        }
                    else
                        {
                        fprintf(fl, getmsg(188), getmsg(MSG_LOGIN, 39));
                        fprintf(fl, bn);
                        }
                    }

                delete Msg;
                break;
                }

            case NUQ_PROTOCOL:
                {
                int c = 0;

                do
                    {
                    if (!CurrentUser->IsExpert() || (c == '?'))
                        {
                        doCR();

                        for (protocols *theProt = extProtList; theProt; theProt = (protocols *) getNextLL(theProt))
                            {
                            if (!theProt->NetOnly)
                                {
                                mPrintfCR(getmsg(361), theProt->CommandKey, theProt->MenuName);
                                }
                            }
                        }

                    c = DoMenuPrompt(getmsg(MSG_LOGIN, 62), NULL);

                    if (c == ' ')
                        {
                        CurrentUser->SetDefaultProtocol(0);
                        }
                    else
                        {
                        const protocols *theProt = GetProtocolByKey((char) c);

                        if (theProt)
                            {
                            mPrintfCR(pcts, theProt->name);
                            CurrentUser->SetDefaultProtocol((char) c);
                            }
                        else
                            {
                            oChar((char) c);
                            if (c == '?')
                                {
                                doCR();
                                }
                            else
                                {
                                mPrintfCR(sqst);
                                c = '?';
                                }
                            }
                        }
                    } while (c == '?' && HaveConnectionToUser());

                break;
                }

            case NUQ_OCCUPATION:
                {
                label Occupation;

                getNormStr(getmsg(MSG_LOGIN, 63), Occupation, LABELSIZE);
                CurrentUser->SetOccupation(Occupation);

                if (*Occupation && fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 64), deansi(Occupation));
                    fprintf(fl, bn);
                    }

                break;
                }

            case NUQ_WHEREHEAR:
                {
                char Wherehear[80];

                getNormStr(getmsg(MSG_LOGIN, 65), Wherehear, 79);
                CurrentUser->SetWhereHear(Wherehear);

                if (*Wherehear && fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 66), deansi(Wherehear));
                    fprintf(fl, bn);
                    }

                break;
                }

            case NUQ_BIRTHDAY:
                {
                label BDay;
                datestruct date;

                do
                    {
                    date.Date = 1;

                    GetStringWithBlurb(getmsg(MSG_LOGIN, 67), BDay, LABELSIZE, ns, B_DATESET);

                    normalizeString(BDay);

                    if (*BDay)
                        {
                        gdate(BDay, &date);

                        if (date.Date == 0)
                            {
                            CRmPrintfCR(getmsg(MSG_LOGIN, 68));
                            }
                        }
                    } while (!date.Date && HaveConnectionToUser());

                if (*BDay)
                    {
                    struct date bdate;
                    struct time btime;

                    bdate.da_mon = (char) (date.Month + 1); // i hate dostounix
                    bdate.da_day = (char) date.Date;
                    bdate.da_year = date.Year + 1900;       // i hate dostounix

                    btime.ti_hour = 0;
                    btime.ti_min = 0;
                    btime.ti_sec = 0;
                    btime.ti_hund = 0;

                    CurrentUser->SetBirthDate(dostounix(&bdate, &btime));

                    if (fl)
                        {
                        fprintf(fl, getmsg(MSG_LOGIN, 69), monthTab[date.Month], date.Date, date.Year + 1900);
                        fprintf(fl, bn);
                        }
                    }

                break;
                }

            case NUQ_COLORS:
                {
                if (TermCap->IsColor())
                    {
                    askAttributes(CurrentUser);
                    }

                break;
                }

            case NUQ_SEX:
                {
                int dfault;

                switch (CurrentUser->GetSex())
                    {
                    case SEX_MALE:      dfault = 0; break;
                    case SEX_FEMALE:    dfault = 1; break;
                    default:            dfault = 2; break;
                    }

                int Sex = GetOneKey(getmsg(MSG_LOGIN, 70),
                        getmsg(MSG_LOGIN, 71),
                        getmsg(MSG_LOGIN, 71)[dfault], B_MFU,
                        getmsg(MSG_LOGIN, 72), getmsg(MSG_LOGIN, 73),
                        getmsg(MSG_LOGIN, 74));

                if (Sex == 0)       // Male
                    {
                    CurrentUser->SetSex(SEX_MALE);
                    }
                else if (Sex == 1)  // Female
                    {
                    CurrentUser->SetSex(SEX_FEMALE);
                    }
                else if (Sex == 3)  // Unspecified
                    {
                    CurrentUser->SetSex(SEX_UNKNOWN);
                    }

                if (fl)
                    {
                    fprintf(fl, getmsg(MSG_LOGIN, 75));

                    switch (CurrentUser->GetSex())
                        {
                        case SEX_MALE:
                            {
                            fprintf(fl, getmsg(MSG_LOGIN, 72));
                            break;
                            }

                        case SEX_FEMALE:
                            {
                            fprintf(fl, getmsg(MSG_LOGIN, 73));
                            break;
                            }

                        default:
                            assert(FALSE);

                        case SEX_UNKNOWN:
                            {
                            fprintf(fl, getmsg(MSG_LOGIN, 74));
                            break;
                            }
                        }

                    fprintf(fl, bn);
                    }

                break;
                }

            default:
                {
                cPrintf(getmsg(MSG_LOGIN, 76), getmsg(MSG_LOGIN, 58));
                doccr();
                break;
                }
            }
        }

    if (fl)
        {
        fclose(fl);
        }

    doCR();
    }


// --------------------------------------------------------------------------
// newlog(): Sets up a new log entry for new users returns FALSE if cannot
//  find a usable slot

Bool TERMWINDOWMEMBER newlog(void)
    {
    // get a new slot for this user
#ifdef WINCIT
    ThisSlot = newslot(LogOrder);
#else
    ThisSlot = newslot();
#endif

    if (ThisSlot == CERROR)
        {
        CRmPrintfCR(getmsg(490), cfg.Uuser_nym);
        ThisSlot = 0;
        return (FALSE);
        }

    ThisLog = LTab(ThisSlot).GetLogIndex();

    CurrentUser->SetTermType(TermCap->GetName());
    setdefaultconfig(cfg.autoansi);

    CurrentUser->SetInuse(TRUE);

    for (r_slot i = 0; i < cfg.maxrooms; i++)
        {
        CurrentUser->SetRoomNewPointer(i, 1);
        }

    // accurate read-userlog for first time call
    CurrentUser->SetCallNumber(cfg.callno + 1);

    return (TRUE);
    }

// --------------------------------------------------------------------------
// newslot(): Attempts to find a slot for a new user to reside in CERROR if
//  none.

#ifdef WINCIT
static l_slot newslot(const l_slot *LogOrder)
#else
static l_slot newslot(void)
#endif
    {
    l_slot foundit = CERROR;

    // find one not in use
    for (l_slot i = (l_slot) (cfg.MAXLOGTAB - 1); i > -1; --i)
        {
        if (!LTab(i).IsInuse())
            {
            foundit = i;
            break;
            }
        }

    if (foundit == CERROR)
        {
        // find one not permanent
        for (l_slot i = (l_slot) (cfg.MAXLOGTAB - 1); i > -1; --i)
            {
            if (!LTab(i).IsPermanent())
                {
                foundit = i;
                break;
                }
            }
        }

    return (foundit);
    }
