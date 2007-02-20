// --------------------------------------------------------------------------
// Citadel: Log3.CPP
//
// .Enter Configuration and .Sysop Userlog edit

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "hall.h"
#include "room.h"
#include "maillist.h"
#include "filecmd.h"
#include "tallybuf.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// configure()  sets user configuration via menu
// newPW()      is menu-level routine to change password & initials


// --------------------------------------------------------------------------
// displaypw(): Displays caller's name, initials, and password.

void TERMWINDOWMEMBER displaypw(const char *name, const char *in, const char *pw)
    {
    CRmPrintf(getmsg(507), name);
    CRmPrintf(pctss, getmsg(500));
    OC.Echo = CALLER;
    OC.setio();
    mPrintf(pcts, in);
    OC.Echo = BOTH;
    OC.setio();
    CRmPrintf(pctss, getmsg(496));
    OC.Echo = CALLER;
    OC.setio();
    mPrintf(pcts, pw);
    OC.Echo = BOTH;
    OC.setio();

    doCR();
    }


// --------------------------------------------------------------------------
// newPW(): Menu-level routine to change password & initials.
//
// Input:
//  Bool check: TRUE to check if user knows old in;pw; FALSE to not
//
// Return value:
//  None.

void TERMWINDOWMEMBER newPW(Bool check)
    {
    char InitPw[LABELSIZE*2+2];
    char passWord[LABELSIZE*2+2];
    char Initials[LABELSIZE*2+2];
    char oldPw[LABELSIZE*2+2];
    char oldIn[LABELSIZE*2+2];
    char *semicolon;
    Bool goodpw;

    SetDoWhat(ENTERPW);

    label Buffer;
    if (check)
        {
        if (!getYesNo(getmsg(62), 0))
            {
            return;
            }

        getNormStr(getmsg(608), InitPw, 40, FALSE);

        semicolon = strchr(InitPw, ';');

        if (semicolon)
            {
            normalizepw(InitPw, Initials, passWord);
            }
        else
            {
            CopyStringToBuffer(Initials, InitPw);

            getNormStr(getmsg(602), passWord, LABELSIZE, FALSE);
            }

        label Buffer1;
        if (!SameString(Initials, CurrentUser->GetInitials(Buffer, sizeof(Buffer))) ||
				!SameString(passWord, CurrentUser-> GetPassword(Buffer1, sizeof(Buffer1))))
            {
            CRmPrintfCR(getmsg(601));
            return;
            }
        }

    dospCR();
    dospCR();

    CurrentUser->GetInitials(oldIn, sizeof(oldIn));
    CurrentUser->GetPassword(oldPw, sizeof(oldPw));

    do
        {
        getNormStr(getmsg(603), InitPw, sizeof(InitPw) - 1, FALSE);
        dospCR();

        semicolon = strchr(InitPw, ';');

        if (semicolon)
            {
            normalizepw(InitPw, Initials, passWord);
            }
        else
            {
            CopyStringToBuffer(Initials, InitPw);
            }

        // do not allow anything over LABELSIZE characters
        Initials[LABELSIZE] = 0;

        if (!semicolon)
            {
            getNormStr(getmsg(604), passWord, LABELSIZE, FALSE);
            dospCR();
            }

        goodpw = (((FindPwInHashInTable(Initials, passWord) == CERROR) && strlen(passWord) >= 2) ||
                (SameString(Initials, oldIn) && SameString(passWord, oldPw)));

        if (!goodpw)
            {
            CRmPrintfCR(getmsg(605));
            }
        } while (!goodpw && HaveConnectionToUser());

    // insure against loss of carrier
    if (HaveConnectionToUser())
        {
        CurrentUser->SetInitials(Initials);
        CurrentUser->SetPassword(passWord);
        CurrentUser->SetPasswordChangeTime(time(NULL));

        storeLog();

        label Buffer1, Buffer2, Buffer3;
        displaypw(CurrentUser->GetName(Buffer1, sizeof(Buffer1)), CurrentUser->GetInitials(Buffer2, sizeof(Buffer2)),
                CurrentUser->GetPassword(Buffer3, sizeof(Buffer3)));

        if(!read_tr_messages())
            {
            errorDisp(getmsg(172));
            return;
            }

#ifdef WINCIT
        trap(T_PASSWORD, WindowCaption, gettrmsg(15));
#else
        trap(T_PASSWORD, gettrmsg(15));
#endif
        dump_tr_messages();
        }
    }


// --------------------------------------------------------------------------
// Everything from here down is for the .EC and .SU commands


// --------------------------------------------------------------------------
// forwardaddr(): Sets up forwarding address for mail.
//
// Input:
//  LogEntry *EditLog is a pointer to the user to edit.

void TERMWINDOWMEMBER forwardaddr(LogEntry *EditLog)
    {
    char str[LABELSIZE+LABELSIZE+2];

    doCR();
    getNormStr(getecmsg(5), str, LABELSIZE + LABELSIZE + 1);
    doCR();

    if (!*str)
        {
        EditLog->SetForwardToNode(FALSE);
        EditLog->SetForwardAddr(ns);
        EditLog->SetForwardAddrNode(ns);
        EditLog->SetForwardAddrRegion(ns);

        mPrintfCR(getecmsg(6), cfg.Lmsgs_nym);
        }
    else
        {
        label ruser;
        label rnode;
        label rregion;
        label raddress;

        parseNetAddress(str, ruser, rnode, rregion, raddress);

        if (!*rnode)        // if not forwarding to a node
            {
            const l_slot logslot = FindPersonByPartialName(str);

            if (logslot == CERROR)
                {
                mPrintfCR(getmsg(595), str);
                }
            else
                {
                label Buffer;

                EditLog->SetForwardToNode(FALSE);
                EditLog->SetForwardAddr(LTab(logslot).GetName(Buffer, sizeof(Buffer)));
                EditLog->SetForwardAddrNode(ns);
                EditLog->SetForwardAddrRegion(ns);

                mPrintfCR(getecmsg(7), cfg.Lmsgs_nym, EditLog->GetForwardAddr(Buffer, sizeof(Buffer)));
                }
            }
        else            // if forwarding to node
            {
            EditLog->SetForwardToNode(TRUE);
            EditLog->SetForwardAddr(ruser);
            EditLog->SetForwardAddrNode(rnode);
            EditLog->SetForwardAddrRegion(rregion);

            label Buffer, Buffer1;
            mPrintfCR(getecmsg(8), cfg.Lmsgs_nym, EditLog->GetForwardAddr(Buffer, sizeof(Buffer)),
					EditLog->GetForwardAddrNode(Buffer1, sizeof(Buffer1)));
            }
        }
    }


// --------------------------------------------------------------------------
// defaulthall(): Edits default hall for a log entry.
//
// Input:
//  LogEntry *EditLog: The log entry to edit.
//  Bool UserlogEdit: TRUE if from .SU, FALSE if from .EC.

void TERMWINDOWMEMBER defaulthall(LogEntry *EditLog, Bool UserlogEdit)
    {
    doCR();

    if (!UserlogEdit && EditLog->IsDefaultHallLocked())
        {
        CRmPrintfCR(getecmsg(9), cfg.Lhall_nym);
        }
    else
        {
        label hallname, Buffer;

        do
            {
            if (!getString(cfg.Lhall_nym, hallname, LABELSIZE, EditLog->GetDefaultHall(Buffer, sizeof(Buffer))))
                {
                return;
                }

            if (*hallname == '?')
                {
                ListHalls(TRUE);
                }
            } while (*hallname == '?');

        if (EditLog->IsDefaultHall(hallname))
            {
            // no change.
            }
        else if (SameString(hallname, spc))
            {
            // Single space: back to the system's default
            EditLog->SetDefaultHall(ns);
            }
        else if (SameString(hallname, spcspc))
            {
            // Double space: use this hall.
            EditLog->SetDefaultHall(HallData[thisHall].GetName(Buffer, sizeof(Buffer)));
            }
        else
            {
            normalizeString(hallname);

            if (*hallname)
                {
                h_slot slot = partialhall(hallname);

                if ((slot == CERROR) || !accesshall(slot))
                    {
                    CRmPrintfCR(getmsg(611), cfg.Lhall_nym);
                    }
                else
                    {
                    EditLog->SetDefaultHall(HallData[slot].GetName(Buffer, sizeof(Buffer)));
                    }
                }
            }

        EditLog->GetDefaultHall(Buffer, sizeof(Buffer));
        CRmPrintfCR(getecmsg(11), cfg.Lhall_nym, *Buffer ? Buffer : getecmsg(61));
        }
    }


// --------------------------------------------------------------------------
// doColumn(): Used to columnize menus
//
// Input:
//  Bool *Col: TRUE if in the second column; FALSE if in first
//
// Output:
//  Bool *Col: Modified to new status

void TERMWINDOWMEMBER doColumn(Bool *Col)
    {
    if (!*Col)
        {
        mPrintf(getecmsg(1));
        }
    else
        {
        doCR();
        }

    *Col = !*Col;
    }


// --------------------------------------------------------------------------

void TERMWINDOWMEMBER listExcl(LogExtensions *LE, KillFileE Type,
        const char *Pmsg, const char *Pmsgs)
    {
    ulong Count = LE->GetKillCount(Type);

    if (Count)
        {
        CRmPrintfCR(getecmsg(18), Pmsg);

        prtList(LIST_START);
        for (ulong Index = 1; Index <= Count; Index++)
            {
            label Buffer;
            if (LE->GetKillNum(Type, Index, Buffer, sizeof(Buffer)))
                {
                prtList(Buffer);
                }
            }

        prtList(LIST_END);
        }
    else
        {
        CRmPrintfCR(getecmsg(19), Pmsgs);
        }
    }


Bool TERMWINDOWMEMBER FinishUpMenu(void)
    {
    CRmPrintfCR(getecmsg(16));
    return (!CurrentUser->IsExpert());
    }

// --------------------------------------------------------------------------
// editIt(): Edits one of the message exclusion types.
//
// Input:
//  strList **theList: Pointer to base of list being edited
//  const char *Pmsg: What the list is called (singular)
//  const char *Pmsgs: What the list is called (plural)
//
// Output:
//  strList **theList: New, edited, list.

void TERMWINDOWMEMBER editIt(LogExtensions *LE, KillFileE Type, const char *Pmsg, const char *Pmsgs)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    mPrintfCR(Pmsg);

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            CRmPrintfCR(getecmsg(12), Pmsg);
            CRmPrintf(getecmsg(13));
            CRmPrintf(getecmsg(14));
            CRmPrintfCR(getecmsg(15));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(17), Pmsg);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'L':
                {
                mPrintfCR(getecmsg(327));
                listExcl(LE, Type, Pmsg, Pmsgs);

                break;
                }

            case 'A':
                {
                char ooga[70];

                mPrintfCR(getmsg(358));

                do
                    {
                    getString(getecmsg(20), ooga, sizeof(ooga) - 1, ns);

                    if (*ooga == '?')
                        {
                        listExcl(LE, Type, Pmsg, Pmsgs);
                        }
                    } while (*ooga == '?');

                if (*ooga)
                    {
                    if (LE->IsKill(Type, ooga))
                        {
                        mPrintf(getecmsg(21));
                        }
                    else
                        {
                        if (!LE->AddKill(Type, ooga))
                            {
                            OutOfMemory(115);
                            }
                        }
                    }

                break;
                }

            case 'R':
                {
                char ooga[70];

                mPrintfCR(getmsg(359));

                do
                    {
                    getString(getecmsg(22), ooga, sizeof(ooga) - 1, ns);

                    if (*ooga == '?')
                        {
                        listExcl(LE, Type, Pmsg, Pmsgs);
                        }
                    } while (*ooga == '?');

                if (*ooga)
                    {
                    if (!LE->IsKill(Type, ooga))
                        {
                        mPrintf(getecmsg(23));
                        }
                    else
                        {
                        LE->RemoveKill(Type, ooga);
                        }
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------

void TERMWINDOWMEMBER listAuthorTags(const LogExtensions *LE,
        const char *Author)
    {
    Bool Started = FALSE;
    doCR();

    ulong Limit = LE->GetTagUserCount(Author);

    for (ulong Index = 1; Index < Limit; Index++)
        {
        label Buffer;

        if (LE->GetUserTag(Index, Author, Buffer, sizeof(Buffer)))
            {
            if (!Started)
                {
                mPrintfCR(getecmsg(296), cfg.Luser_nym, Author);

                prtList(LIST_START);
                Started = TRUE;
                }

            prtList(Buffer);
            }
        }

    if (Started)
        {
        prtList(LIST_END);
        }
    else
        {
        mPrintfCR(getecmsg(297), cfg.Luser_nym, Author);
        }
    }


// --------------------------------------------------------------------------

void TERMWINDOWMEMBER listTaggedAuthors(const LogExtensions *LE)
    {
    ulong Limit = LE->GetTagUserCount();

    if (Limit)
        {
        CRmPrintfCR(getecmsg(294), cfg.Lusers_nym);

        strList *beenShown = NULL;

        prtList(LIST_START);
        for (ulong Index = 1; Index <= Limit; Index++)
            {
            label User, Tag;
            if (LE->GetTagUserNum(Index, User, sizeof(User), Tag, sizeof(Tag)))
                {
                strList *theList;
                for (theList = beenShown; theList; theList = (strList *) getNextLL(theList))
                    {
                    if (SameString(theList->string, User))
                        {
                        break;
                        }
                    }

                if (!theList)
                    {
                    prtList(User);

                    theList = (strList *) addLL((void **) beenShown, sizeof(strList) + strlen(User));

                    if (theList)
                        {
                        strcpy(theList->string, User);
                        }
                    else
                        {
                        OutOfMemory(91);
                        }
                    }
                }
            }

        disposeLL((void **) &beenShown);

        prtList(LIST_END);
        }
    else
        {
        CRmPrintfCR(getecmsg(295), cfg.Luser_nym);
        }
    }


// --------------------------------------------------------------------------
// editTags(): Edits message author tags.
//
// Input:
//  LogEntry *EditLog: Log entry to edit.

void TERMWINDOWMEMBER editTags(LogExtensions *LE)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    mPrintfCR(getecmsg(25), cfg.Umsg_nym);

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            CRmPrintfCR(getecmsg(25), cfg.Umsg_nym);
            CRmPrintf(getecmsg(13));
            CRmPrintf(getecmsg(14));
            CRmPrintfCR(getecmsg(15));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(26), cfg.Lmsg_nym);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'L':
                {
                mPrintfCR(getecmsg(327));

                ulong Limit = LE->GetTagUserCount();

                if (Limit)
                    {
                    CRmPrintfCR(getecmsg(27), cfg.Lmsg_nym);

                    OC.SetOutFlag(OUTOK);

                    for (ulong Index = 1; Index <= Limit; Index++)
                        {
                        label User, Tag;

                        if (LE->GetTagUserNum(Index, User, sizeof(User), Tag, sizeof(Tag)))
                            {
                            mPrintfCR(getecmsg(2), User, Tag);
                            }
                        }
                    }
                else
                    {
                    CRmPrintfCR(getecmsg(28), cfg.Lmsg_nym);
                    }

                break;
                }

            case 'A':
                {
                label ooga;

                mPrintfCR(getmsg(358));

                do
                    {
                    getString(getecmsg(29), ooga, LABELSIZE, ns);

                    if (*ooga == '?')
                        {
                        listTaggedAuthors(LE);
                        }
                    } while (*ooga == '?');

                if (*ooga)
                    {
                    label ooga2;

                    do
                        {
                        getString(getecmsg(30), ooga2, LABELSIZE, ns);

                        if (*ooga2 == '?')
                            {
                            listAuthorTags(LE, ooga);
                            }
                        } while (*ooga2 == '?');

                    if (*ooga2)
                        {
                        LE->AddTagUser(ooga, ooga2);
                        }
                    }

                break;
                }

            case 'R':
                {
                label ooga;

                mPrintfCR(getmsg(359));

                do
                    {
                    getString(getecmsg(31), ooga, LABELSIZE, ns);

                    if (*ooga == '?')
                        {
                        listTaggedAuthors(LE);
                        }
                    } while (*ooga == '?');

                if (*ooga && !LE->RemoveUserTag(ooga))
                    {
                    mPrintf(getecmsg(32));
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }

void TERMWINDOWMEMBER DisplayOnOffWithColumn(int Msg, Bool Value, Bool *Col)
    {
    displayOnOff(getecmsg(Msg), Value);
    doColumn(Col);
    }

void TERMWINDOWMEMBER DisplayYesNoWithColumn(int Msg, Bool Value, Bool *Col)
    {
    displayYesNo(getecmsg(Msg), Value);
    doColumn(Col);
    }

void TERMWINDOWMEMBER DisplayOnOffWithCR(int Msg, Bool Value)
    {
    displayOnOff(getecmsg(Msg), Value);
    doCR();
    }

// --------------------------------------------------------------------------
// editTerminal(): Edits terminal stuff.
//
// Input:
//  LogEntry1 *EditLog1: Log entry to edit
//
// Output:
//  LogEntry1 *EditLog1: Modified entry.

void TERMWINDOWMEMBER editTerminal(LogEntry1 *EditLog1, Bool UserlogEdit)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            Bool Col = FALSE;

            CRmPrintf(getecmsg(36), EditLog1->GetWidth());
            doColumn(&Col);

            DisplayOnOffWithColumn(37, EditLog1->IsUpperOnly(), &Col);
            DisplayOnOffWithColumn(38, EditLog1->IsLinefeeds(), &Col);
            DisplayOnOffWithColumn(39, EditLog1->IsTabs(), &Col);

            mPrintf(getecmsg(40), EditLog1->GetNulls());
            doColumn(&Col);

            DisplayOnOffWithColumn(41, EditLog1->IsIBMRoom(), &Col);

            if (cfg.music)
                {
                DisplayOnOffWithColumn(42, EditLog1->IsMusic(), &Col);
                }

            if (cfg.ecTwirly)
                {
                DisplayOnOffWithColumn(141, EditLog1->IsTwirly(), &Col);
                }

            // from here down is not columnized...
            if (Col)
                {
                doCR();
                }

            label Buffer;
            mPrintfCR(getecmsg(43), EditLog1->GetLinesPerScreen() ?
					itoa(EditLog1->GetLinesPerScreen(), Buffer, 10) : getecmsg(44));

            char MP[80];
            mPrintfCR(getecmsg(291), EditLog1->GetMorePrompt(MP, sizeof(MP)));
            mPrintfCR(getecmsg(33), EditLog1->GetTermType(Buffer, sizeof(Buffer)));
            mPrintfCR(getecmsg(45));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(46), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case '>':
                {
                char stuff[80];

                mPrintfCR(getecmsg(292));

                char Buffer[80];
                if (GetStringWithBlurb(getecmsg(293), stuff, 79, EditLog1->GetMorePrompt(Buffer, sizeof(Buffer)),
						B_MOREPRMP) && *stuff)
                    {
                    if (SameString(stuff, spc))
                        {
                        EditLog1->SetMorePrompt(cfg.moreprompt);
                        }
                    else
                        {
                        stripansi(stuff);
                        normalizeString(stuff);
                        EditLog1->SetMorePrompt(stuff);
                        }
                    }

                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            case 'A':
                {
                EditLog1->SetTabs(changeOnOff(getecmsg(58), EditLog1->IsTabs()));
                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case 'F':
                {
                EditLog1->SetLinefeeds(changeOnOff(getecmsg(57), EditLog1->IsLinefeeds()));
                break;
                }

            // I is in optional section

            case 'L':
                {
                mPrintfCR(getecmsg(50));

                long value;

                Bool ShowBlb;
                do
                    {
                    ShowBlb = FALSE;

                    value = getNumber(getecmsg(51), 0L, INT_MAX, EditLog1->GetLinesPerScreen(), TRUE, &ShowBlb);

                    if (ShowBlb || value == 1 || value == 2)
                        {
                        dispBlb(B_LENGTH);
                        value = CERROR;
                        }
                    } while (ShowBlb || value == 1 || value == 2);

                EditLog1->SetLinesPerScreen((int) value);

                break;
                }

            // M is in optional section

            case 'N':
                {
                mPrintfCR(getecmsg(53));

                EditLog1->SetNulls((int) GetNumberWithBlurb(getecmsg(54), 0L, 255L, EditLog1->GetNulls(), B_NULLS));

                break;
                }

            case 'O':
                {
                EditLog1->SetIBMRoom(changeOnOff(getecmsg(62), EditLog1->IsIBMRoom()));
                break;
                }

            case 'R':
                {
                mPrintf(getecmsg(59));
                askAttributes(EditLog1);
                break;
                }

            case 'T':
                {
                mPrintfCR(getecmsg(60));

                int Desired = 0;
                label TermType, Buffer;

                do
                    {
                    int Count = 0, Current = 0;

                    while (GetTermType(Count, TermType, sizeof(TermType)))
                        {
                        CRmPrintf(getecmsg(34), ++Count, TermType);

                        if (SameString(EditLog1->GetTermType(Buffer, sizeof(Buffer)), TermType))
                            {
                            Current = Count;
                            }
                        }

                    if (Count)
                        {
                        doCR();

                        Desired = (int) getNumber(getecmsg(35), 1, Count, Current, TRUE, NULL);
                        }
                    else
                        {
                        CRmPrintfCR(getecmsg(47));
                        break;
                        }
                    } while (Desired < 0);

                if (Desired)
                    {
                    EditLog1->SetTermType(GetTermType(Desired - 1, TermType, sizeof(TermType)));

                    if (!UserlogEdit)
                        {
                        TermCap->Load(EditLog1->GetTermType(Buffer, sizeof(Buffer)));

                        EditLog1->SetDefaultColors(TermCap->IsColor());

                        // We keep the old stuff up-to-date in case someone
                        // downgrades to /065 or earlier.

                        CurrentUser->SetOldIBMGraph(TermCap->IsIBMExtended());
                        CurrentUser->SetOldIBMANSI(TermCap->IsANSI());
                        CurrentUser->SetOldIBMColor(TermCap->IsColor());
                        }
                    }

                break;
                }

            case 'U':
                {
                EditLog1->SetUpperOnly(changeOnOff(getecmsg(56), EditLog1->IsUpperOnly()));
                break;
                }

            case 'W':
                {
                mPrintfCR(getecmsg(48));

                const long value = GetNumberWithBlurb(getecmsg(49), 10l, 255l, (long) EditLog1->GetWidth(), B_WIDTH);

                if (HaveConnectionToUser())
                    {
                    EditLog1->SetWidth((int) value);
                    }

                break;
                }

            // from here on are not always on and we do icky break stuff
            case 'I':
                if (cfg.ecTwirly)
                    {
                    EditLog1->SetTwirly(changeOnOff(getecmsg(206), EditLog1->IsTwirly()));
                    break;
                    }

            case 'M':
                if (toupper(c) == 'M' && cfg.music)
                    {
                    EditLog1->SetMusic(changeOnOff(getecmsg(63), EditLog1->IsMusic()));
                    break;
                    }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------
// editPersonalInfo(): Edits personal stuff
//
// Input/Output:
//  LogEntry1 *EditLog1: Log entry to edit

void TERMWINDOWMEMBER editPersonalInfo(LogEntry1 *EditLog1)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            label Buffer;
            CRmPrintf(getecmsg(64), EditLog1->GetRealName(Buffer, sizeof(Buffer)));
            CRmPrintf(getecmsg(65), EditLog1->GetPhoneNumber(Buffer, sizeof(Buffer)));
            CRmPrintf(getecmsg(66), EditLog1->GetMailAddr1(Buffer, sizeof(Buffer)));
            CRmPrintf(getecmsg(67), EditLog1->GetMailAddr2(Buffer, sizeof(Buffer)));
            CRmPrintfCR(getecmsg(68), EditLog1->GetMailAddr3(Buffer, sizeof(Buffer)));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(300), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'R':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(69));

                if (GetStringWithBlurb(getecmsg(70), stuff, LABELSIZE, EditLog1->GetRealName(Buffer, sizeof(Buffer)),
						B_REALNAME) && *stuff)
                    {
                    normalizeString(stuff);
                    EditLog1->SetRealName(stuff);
                    }

                break;
                }

            case 'P':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(71));

                if (GetStringWithBlurb(getecmsg(72), stuff, LABELSIZE, EditLog1->GetPhoneNumber(Buffer, sizeof(Buffer)),
                        B_PHONENUM) && *stuff)
                    {
                    normalizeString(stuff);
                    EditLog1->SetPhoneNumber(stuff);
                    }

                break;
                }

            case '1':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(73));

                if (GetStringWithBlurb(getecmsg(74), stuff, LABELSIZE, EditLog1->GetMailAddr1(Buffer, sizeof(Buffer)),
                        B_ADDRESS) && *stuff)
                    {
                    normalizeString(stuff);
                    EditLog1->SetMailAddr1(stuff);
                    }

                break;
                }

            case '2':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(75));

                if (GetStringWithBlurb(getecmsg(76), stuff, LABELSIZE, EditLog1->GetMailAddr2(Buffer, sizeof(Buffer)),
                        B_ADDRESS) && *stuff)
                    {
                    normalizeString(stuff);
                    EditLog1->SetMailAddr2(stuff);
                    }

                break;
                }

            case '3':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(77));

                if (GetStringWithBlurb(getecmsg(78), stuff, LABELSIZE, EditLog1->GetMailAddr3(Buffer, sizeof(Buffer)),
                        B_ADDRESS) && *stuff)
                    {
                    normalizeString(stuff);
                    EditLog1->SetMailAddr3(stuff);
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------
// editSystemEvents(): Edits sysem events!
//
// Input/Output:
//  LogEntry1 *EditLog1: Log entry to edit

void TERMWINDOWMEMBER editSystemEvents(LogEntry1 *EditLog1)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
			doCR();
            DisplayOnOffWithCR(338, EditLog1->ShowSELogOnOff());
            DisplayOnOffWithCR(339, EditLog1->ShowSENewMessage());
            DisplayOnOffWithCR(365, EditLog1->IsHearLaughter());
            DisplayOnOffWithCR(340, EditLog1->ShowSEExclusiveMessage());
            DisplayOnOffWithCR(341, EditLog1->ShowSERoomInOut());
            DisplayOnOffWithCR(342, EditLog1->ShowSEChatAll());
            DisplayOnOffWithCR(343, EditLog1->ShowSEChatRoom());
            DisplayOnOffWithCR(344, EditLog1->ShowSEChatGroup());
            DisplayOnOffWithCR(345, EditLog1->ShowSEChatUser());
            DisplayOnOffWithCR(359, EditLog1->IsSeeOwnChats());
            DisplayOnOffWithCR(360, EditLog1->IsErasePrompt());

			int wow = EditLog1->GetAutoIdleSeconds();

            mPrintfCR(getecmsg(361), (wow == -1) ? "Disabled" : ltoac(wow));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(346), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'L':
                {
                EditLog1->SetSELogOnOff(changeOnOff(getecmsg(347), EditLog1->ShowSELogOnOff()));
                break;
                }

            case 'M':
                {
                EditLog1->SetSENewMessage(changeOnOff(getecmsg(348), EditLog1->ShowSENewMessage()));
                break;
                }

            case 'E':
                {
                EditLog1->SetSEExclusiveMessage(changeOnOff(getecmsg(349), EditLog1->ShowSEExclusiveMessage()));
                break;
                }

            case 'O':
                {
                EditLog1->SetSERoomInOut(changeOnOff(getecmsg(350), EditLog1->ShowSERoomInOut()));
                break;
                }

            case 'A':
                {
                EditLog1->SetSEChatAll(changeOnOff(getecmsg(351), EditLog1->ShowSEChatAll()));
                break;
                }

            case 'R':
                {
                EditLog1->SetSEChatRoom(changeOnOff(getecmsg(352), EditLog1->ShowSEChatRoom()));
                break;
                }

            case 'G':
                {
                EditLog1->SetSEChatGroup(changeOnOff(getecmsg(353), EditLog1->ShowSEChatGroup()));
                break;
                }

            case 'U':
                {
                EditLog1->SetSEChatUser(changeOnOff(getecmsg(354), EditLog1->ShowSEChatUser()));
                break;
                }

            case '1':
                {
                EditLog1->SetSeeOwnChats(changeOnOff(getecmsg(362), EditLog1->IsSeeOwnChats()));
                break;
                }

            case '2':
                {
                EditLog1->SetErasePrompt(changeOnOff(getecmsg(363), EditLog1->IsErasePrompt()));
				if (EditLog1->IsErasePrompt())
    				{
                    CRmPrintfCR("2 * 1WARNING!!!02 * 0 Not recommended for multi-line room prompts!");
	    			if (!getYesNo("Are you sure you want to use this", FALSE))
		    		     {
			    		 EditLog1->SetErasePrompt(FALSE);
    					 }
					}
                break;
                }

            case '3':
                {
                mPrintfCR(getecmsg(364));

     			int wow = EditLog1->GetAutoIdleSeconds();
                long value;

    			if (getYesNo("Enable AutoIdle", (wow == -1) ? FALSE : TRUE))
				    {
                    value = getNumber(getecmsg(364), 0l, (long)INT_MAX, (long) EditLog1->GetAutoIdleSeconds(), FALSE, NULL);
					}
				else
				    {
					value = -1;
					}

                if (HaveConnectionToUser())
                    {
                    EditLog1->SetAutoIdleSeconds((int) value);
                    }

                break;
                }

            case 'N':
                {
                EditLog1->SetHearLaughter(changeOnOff(getecmsg(366), EditLog1->IsHearLaughter()));
                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }

// --------------------------------------------------------------------------
// editFormatStrings(): Edits format strings
//
// Input/Output:
//  LogEntry1 *EditLog1: Log entry to edit

void TERMWINDOWMEMBER editFormatStrings(LogEntry1 *EditLog1)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            char Buffer[64];

            CRmPrintf(getecmsg(79), EditLog1->GetPromptFormat(Buffer, sizeof(Buffer)));
            CRmPrintf(getecmsg(80), EditLog1->GetDateStamp(Buffer, sizeof(Buffer)));
            CRmPrintf(getecmsg(81), EditLog1->GetVerboseDateStamp(Buffer, sizeof(Buffer)));
            CRmPrintfCR(getecmsg(82), EditLog1->GetNetPrefix(Buffer, sizeof(Buffer)));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(301), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'P':
                {
                char stuff[64], Buffer[64];

                mPrintfCR(getecmsg(83));

                if (GetStringWithBlurb(getecmsg(84), stuff, 63, EditLog1->GetPromptFormat(Buffer, sizeof(Buffer)),
                        B_PROMPT) && *stuff)
                    {
                    if (SameString(stuff, spc))
                        {
                        EditLog1->SetPromptFormat(cfg.prompt);
                        }
                    else
                        {
                        normalizeString(stuff);
                        EditLog1->SetPromptFormat(stuff);
                        }
                    }

                break;
                }

            case 'N':
                {
                label stuff, Buffer;

                mPrintfCR(getecmsg(85));

                if (GetStringWithBlurb(getecmsg(86), stuff, LABELSIZE, EditLog1->GetNetPrefix(Buffer, sizeof(Buffer)),
                        B_NETPREFX) && *stuff)
                    {
                    if (SameString(stuff, spc))
                        {
                        EditLog1->SetNetPrefix(cfg.netPrefix);
                        }
                    else
                        {
                        normalizeString(stuff);
                        EditLog1->SetNetPrefix(stuff);
                        }
                    }

                break;
                }

            case 'T':
                {
                char stuff[64];

                mPrintfCR(getecmsg(87));

                char DS[64];
                if (GetStringWithBlurb(getecmsg(88), stuff, 63, EditLog1->GetDateStamp(DS, sizeof(DS)), B_TIME) &&
                        *stuff)
                    {
                    if (SameString(stuff, spc))
                        {
                        EditLog1->SetDateStamp(cfg.datestamp);
                        }
                    else
                        {
                        normalizeString(stuff);
                        EditLog1->SetDateStamp(stuff);
                        }
                    }

                if (GetStringWithBlurb(getecmsg(89), stuff, 63, EditLog1->GetVerboseDateStamp(DS, sizeof(DS)),
                        B_TIME) && *stuff)
                    {
                    if (SameString(stuff, spc))
                        {
                        EditLog1->SetVerboseDateStamp(cfg.vdatestamp);
                        }
                    else
                        {
                        normalizeString(stuff);
                        EditLog1->SetVerboseDateStamp(stuff);
                        }
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------
// messageEditOptions(): Sets message editor options.

void TERMWINDOWMEMBER listDictionary(const LogExtensions *LE)
    {
    OC.SetOutFlag(OUTOK);

    ulong Limit = LE->GetDictionaryCount();

    if (Limit)
        {
        CRmPrintfCR(getecmsg(110));

        prtList(LIST_START);
        for (ulong Index = 1; Index <= Limit; Index++)
            {
            label Buffer;
            if (LE->GetDictNum(Index, Buffer, sizeof(Buffer)))
                {
                prtList(Buffer);
                }
            }

        prtList(LIST_END);
        }
    else
        {
        CRmPrintfCR(getecmsg(111));
        }
    }

void TERMWINDOWMEMBER spellCheckerOptions(LogEntry *EditLog)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            CRmPrintfCR(getecmsg(94), getecmsg(EditLog->GetSpellCheckMode() + 95));

            DisplayOnOffWithCR(99, EditLog->IsCheckApostropheS());
            DisplayOnOffWithCR(100, EditLog->IsCheckAllCaps());
            DisplayOnOffWithCR(101, EditLog->IsCheckDigits());

            mPrintfCR(getecmsg(102));
            mPrintfCR(getecmsg(103));
            mPrintfCR(getecmsg(104));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(304), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case 'S':
                {
                EditLog->SetCheckApostropheS(changeOnOff(getecmsg(106), EditLog->IsCheckApostropheS()));
                break;
                }

            case 'D':
                {
                EditLog->SetCheckDigits(changeOnOff(getecmsg(107), EditLog->IsCheckDigits()));
                break;
                }

            case 'C':
                {
                EditLog->SetCheckAllCaps(changeOnOff(getecmsg(109), EditLog->IsCheckAllCaps()));
                break;
                }

            case 'L':
                {
                mPrintfCR(getecmsg(327));
                listDictionary(EditLog);

                break;
                }

            case 'P':
                {
                int m;

                mPrintf(getecmsg(112));

                const int ii = toupper(iCharNE());

                if (ii == 'V')
                    {
                    m = 95;
                    EditLog->SetSpellCheckMode(0);
                    }
                else if (ii == 'R')
                    {
                    m = 96;
                    EditLog->SetSpellCheckMode(1);
                    }
                else if (ii == 'T')
                    {
                    m = 97;
                    EditLog->SetSpellCheckMode(2);
                    }
                else if (ii == 'A')
                    {
                    m = 98;
                    EditLog->SetSpellCheckMode(3);
                    }
                else
                    {
                    m = 113;
                    oChar((char) ii);
                    if (ii != '?')
                        {
                        mPrintf(sqst);
                        }

                    doCR(2);
                    }

                mPrintfCR(getecmsg(m));
                break;
                }

            case 'A':
                {
                char word[70];

                mPrintfCR(getmsg(358));

                do
                    {
                    getString(getecmsg(116), word, 69 /* Dude! */, ns);

                    if (*word == '?')
                        {
                        listDictionary(EditLog);
                        }
                    } while (*word == '?');

                if (*word)
                    {
                    if (EditLog->IsWordInDictionary(word))
                        {
                        CRmPrintfCR(getecmsg(117));
                        }
                    else
                        {
                        if (!EditLog->AddWordToDictionary(word))
                            {
                            OutOfMemory(92);
                            }
                        }
                    }

                break;
                }

            case 'R':
                {
                char word[70];

                mPrintfCR(getmsg(359));

                do
                    {
                    getString(getecmsg(118), word, 69 /* Dude! */, ns);

                    if (*word == '?')
                        {
                        listDictionary(EditLog);
                        }
                    } while (*word == '?');

                if (*word && !EditLog->RemoveWordFromDictionary(word))
                    {
                    CRmPrintfCR(getecmsg(119));
                    }

                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }

void TERMWINDOWMEMBER listReplacedText(const LogExtensions *LE)
    {
    ulong Limit = LE->GetReplaceCount();

    if (Limit)
        {
        CRmPrintfCR(getecmsg(315));

        prtList(LIST_START);
        for (ulong Index = 1; Index <= Limit; Index++)
            {
            label Orig, Repl;

            if (LE->GetReplaceNum(Index, Orig, sizeof(Orig), Repl, sizeof(Repl)))
                {
                prtList(Orig);
                }
            }

        prtList(LIST_END);
        }
    else
        {
        CRmPrintfCR(getecmsg(316));
        }
    }

void TERMWINDOWMEMBER textReplacementOptions(LogExtensions *LE)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            CRmPrintfCR(getecmsg(13));
            mPrintfCR(getecmsg(14));
            mPrintfCR(getecmsg(15));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(307), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'L':
                {
                mPrintfCR(getecmsg(327));

                ulong Limit = LE->GetReplaceCount();

                if (Limit)
                    {
                    CRmPrintfCR(getecmsg(308));

                    OC.SetOutFlag(OUTOK);

                    for (ulong Index = 1; Index <= Limit; Index++)
                        {
                        label Orig, Repl;
                        if (LE->GetReplaceNum(Index, Orig, sizeof(Orig), Repl, sizeof(Repl)))
                            {
                            mPrintfCR(getecmsg(309), Orig, Repl);
                            }
                        }
                    }
                else
                    {
                    CRmPrintfCR(getecmsg(310));
                    }

                break;
                }

            case 'A':
                {
                label ooga;

                mPrintfCR(getmsg(358));

                do
                    {
                    getString(getecmsg(311), ooga, LABELSIZE, ns);

                    if (*ooga == '?')
                        {
                        listReplacedText(LE);
                        }
                    } while (*ooga == '?');

                if (*ooga)
                    {
                    label Buffer;
                    if (LE->FindReplace(ooga, Buffer, sizeof(Buffer)))
                        {
                        CRmPrintfCR(getecmsg(317));
                        }
                    else
                        {
                        label ooga2;

                        getString(getecmsg(312), ooga2, LABELSIZE, FALSE, ns);

                        if (*ooga2)
                            {
                            LE->AddReplace(ooga, ooga2);
                            }
                        }
                    }

                break;
                }

            case 'R':
                {
                label ooga;

                mPrintfCR(getmsg(359));

                do
                    {
                    getString(getecmsg(313), ooga, LABELSIZE, ns);

                    if (*ooga == '?')
                        {
                        listReplacedText(LE);
                        }
                    } while (*ooga == '?');

                if (*ooga && !LE->RemoveReplace(ooga))
                    {
                    CRmPrintfCR(getecmsg(314));
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }

void TERMWINDOWMEMBER messageEditOptions(LogEntry *EditLog)
    {
    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            doCR();

            DisplayOnOffWithCR(90, EditLog->IsVerboseContinue());
            DisplayOnOffWithCR(91, EditLog->IsConfirmSave());
            DisplayOnOffWithCR(92, EditLog->IsConfirmAbort());
            DisplayOnOffWithCR(93, EditLog->IsConfirmNoEO());
            DisplayOnOffWithCR(333, EditLog->IsBunny());

            mPrintfCR(getecmsg(284));
            mPrintfCR(getecmsg(303));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(298), cfg.Lmsg_nym);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'A':
                {
                EditLog->SetConfirmAbort(changeOnOff(getecmsg(105), EditLog->IsConfirmAbort()));
                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case 'E':
                {
                EditLog->SetConfirmNoEO(changeOnOff(getecmsg(108), EditLog->IsConfirmNoEO()));
                break;
                }

            case 'P':
                {
                mPrintfCR(getecmsg(305));

                spellCheckerOptions(EditLog);
                break;
                }

            case 'R':
                {
                mPrintfCR(getecmsg(306));

                textReplacementOptions(EditLog);
                break;
                }

            case 'S':
                {
                EditLog->SetConfirmSave(changeOnOff(getecmsg(114), EditLog->IsConfirmSave()));
                break;
                }

            case 'U':
                {
                EditLog->SetBunny(changeOnOff(getecmsg(334), EditLog->IsBunny()));
                break;
                }

            case 'V':
                {
                EditLog->SetVerboseContinue(changeOnOff(getecmsg(115), EditLog->IsVerboseContinue()));
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------
// editMsgOption(): Edits message read options.

void TERMWINDOWMEMBER editMsgOption(LogEntry *EditLog, Bool UserlogEdit,
        Bool *pedited_pointers)
    {
    Bool prtMsg = !CurrentUser->IsExpert();
    Bool quit = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            doCR();

            if (cfg.censor || UserlogEdit || CurrentUser->IsSysop())
                {
                DisplayOnOffWithCR(155, EditLog->IsViewCensoredMessages());
                }

            DisplayOnOffWithCR((153),
                    EditLog->IsClearScreenBetweenMessages());

            DisplayOnOffWithCR(151, EditLog->IsPauseBetweenMessages());
            DisplayOnOffWithCR(150, EditLog->IsViewSubjects());
            DisplayOnOffWithCR(152, EditLog->IsViewSignatures());
            DisplayOnOffWithCR(147, EditLog->IsOldToo());

            mPrintfCR(getecmsg(120));
            mPrintfCR(getecmsg(121));
            mPrintfCR(getecmsg(122));
            mPrintfCR(getecmsg(123));
            DisplayOnOffWithCR(124, EditLog->IsExcludeEncryptedMessages());
            DisplayOnOffWithCR(125, !EditLog->IsHideMessageExclusions());

            mPrintfCR(getecmsg(126));

            prtMsg = FinishUpMenu();
            }

        const int c = DoMenuPrompt(getecmsg(299), cfg.Lmsg_nym);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case '*':
                {
                if (cfg.censor || UserlogEdit || CurrentUser->IsSysop())
                    {
                    char temp[128];
                    sprintf(temp, getecmsg(178), cfg.Lmsgs_nym);

                    EditLog->SetViewCensoredMessages(changeOnOff(temp, EditLog->IsViewCensoredMessages()));

                    *pedited_pointers = TRUE;

                    if (EditLog->IsViewCensoredMessages() && !UserlogEdit)
                        {
                        dispBlb(B_DISCLAIM);
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '@':
                {
                char temp[128];

                sprintf(temp, getecmsg(187), cfg.Lmsg_nym);
                EditLog->SetPauseBetweenMessages(changeOnOff(temp, EditLog->IsPauseBetweenMessages()));
                break;
                }

            case 'A':
                {
                editIt(EditLog, KF_USER, getecmsg(127), getecmsg(128));
                break;
                }

            case ESC:
            case 'B':
                {
                mPrintf(getecmsg(24));
                quit = TRUE;
                break;
                }

            case 'C':
                {
                char temp[128];

                sprintf(temp, getecmsg(189), cfg.Lmsg_nym);
                EditLog->SetClearScreenBetweenMessages(changeOnOff(temp, EditLog->IsClearScreenBetweenMessages()));

                break;
                }

            case 'E':
                {
                EditLog->SetExcludeEncryptedMessages(!EditLog->IsExcludeEncryptedMessages());

                DisplayOnOffWithCR(129, EditLog->IsExcludeEncryptedMessages());
                break;
                }

            case 'G':
                {
                EditLog->SetViewSignatures(!EditLog->IsViewSignatures());
                mPrintfCR(getecmsg(182), EditLog->IsViewSignatures() ? ns : getecmsg(180));
                break;
                }

            case 'J':
                {
                EditLog->SetViewSubjects(!EditLog->IsViewSubjects());
                mPrintfCR(getecmsg(181), EditLog->IsViewSubjects() ? ns : getecmsg(180));
                break;
                }

            case 'L':
                {
                EditLog->SetOldToo(changeOnOff(getecmsg(193), EditLog->IsOldToo()));
                break;
                }

            case 'M':
                {
                editTags(EditLog);
                break;
                }

            case 'N':
                {
                editIt(EditLog, KF_NODE, getecmsg(130), getecmsg(131));
                break;
                }

            case 'O':
                {
                EditLog->SetHideMessageExclusions(!EditLog->IsHideMessageExclusions());
                DisplayOnOffWithCR(132, !EditLog->IsHideMessageExclusions());
                break;
                }

            case 'R':
                {
                editIt(EditLog, KF_REGION, getecmsg(133), getecmsg(134));
                break;
                }

            case 'T':
                {
                editIt(EditLog, KF_TEXT, getecmsg(135), getecmsg(135));
                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    doCR();
    }


// --------------------------------------------------------------------------
// userPointers(): Manually reset userlog pointers.

void TERMWINDOWMEMBER userPointers(LogEntry *ToEdit, Bool fromEC)
    {
    Bool menu = !CurrentUser->IsExpert();

    mPrintfCR(getecmsg(55));

    for (;;)
        {
        OC.SetOutFlag(OUTOK);

        if (menu)
            {
            CRmPrintfCR(getecmsg(268));
            mPrintfCR(getecmsg(269));
            mPrintfCR(getecmsg(270));
            mPrintfCR(getecmsg(271));
            mPrintfCR(getecmsg(272));

            CRmPrintfCR(getecmsg(273));

            menu = !CurrentUser->IsExpert();
            }

        label Oldest, Newest;
		m_index OldI = MessageDat.OldestMessageInFile(), NewI = MessageDat.NewestMessage();

        CopyStringToBuffer(Oldest, ltoac(OldI));
        CopyStringToBuffer(Newest, ltoac(NewI));
        CRmPrintfCR(getecmsg(274), ltoac(NewI - OldI + 1), Oldest, Newest);

        const int c = DoMenuPrompt(getecmsg(275), NULL);

        if (!HaveConnectionToUser())
            {
            return;
            }

        switch (toupper(c))
            {
            case 'R':
                {
                mPrintfCR(getecmsg(276));

                label roomname;
                AskRoomName(roomname, thisRoom);

                r_slot roomslot = RoomExists(roomname);

                if (roomslot == CERROR)
                    {
                    roomslot = PartialRoomExists(roomname, thisRoom, TRUE);

                    // no partial room name to get to hidden room
                    if (roomslot != CERROR)
                        {
                        if (!ToEdit->IsInRoom(roomslot))
                            {
                            roomslot = CERROR;
                            }
                        }
                    }

                // cannot get to BIO rooms or public rooms you have been
                // removed from.
                if (roomslot != CERROR)
                    {
                    if (RoomTab[roomslot].IsBIO() || !RoomTab[roomslot].IsHidden())
                        {
                        if (!ToEdit->IsInRoom(roomslot))
                            {
                            roomslot = CERROR;
                            }
                        }
                    }

                if (roomslot == CERROR || !(*roomname) ||
						(fromEC && !(ToEdit->CanAccessRoom(roomslot) && theAlgorithm(roomslot, thisHall, FALSE))))
                    {
                    if (*roomname)
                        {
                        CRmPrintf(getmsg(157), cfg.Lroom_nym, roomname);
                        }
                    }
                else
                    {
                    char prompt[128];

                    ToEdit->SetInRoom(roomslot, TRUE);

                    label Buffer;
                    sprintf(prompt, getecmsg(277), RoomTab[roomslot].GetName(Buffer, sizeof(Buffer)));

                    const m_index msgnumber = getNumber(prompt, 0L, MessageDat.NewestMessage(),
                            ToEdit->GetRoomNewPointer(roomslot), FALSE, NULL);

                    ToEdit->SetRoomNewPointer(roomslot, msgnumber);
                    }

                break;
                }

            case 'D':
                {
                r_slot i;

                mPrintfCR(getecmsg(278));

                OC.SetOutFlag(OUTOK);
                doCR();

                for (i = 0;
#ifdef MULTI
                        i < cfg.maxrooms && (!OC.CheckInput(FALSE, this));
#else
                        i < cfg.maxrooms && (!OC.CheckInput(FALSE));
#endif
                        i++)
                    {
                    if (RoomTab[i].IsInuse() && (!fromEC || (ToEdit->CanAccessRoom(i) && theAlgorithm(i, thisHall, FALSE))))
                        {
                        label Buffer;
                        mPrintfCR(getecmsg(279), deansi(RoomTab[i].GetName(Buffer, sizeof(Buffer))),
                                ltoac(ToEdit->GetRoomNewPointer(i)));
                        }
                    }

                break;
                }

            case 'N':
                {
                mPrintfCR(getecmsg(280), cfg.Lmsgs_nym);

                for (r_slot i = 0; i < cfg.maxrooms; i++)
                    {
                    ToEdit->SetRoomNewPointer(i, MessageDat.OldestMessageInFile() - 1);
                    }

                break;
                }

            case 'O':
                {
                mPrintfCR(getecmsg(281), cfg.Lmsgs_nym);

                for (r_slot i = 0; i < cfg.maxrooms; i++)
                    {
                    ToEdit->SetRoomNewPointer(i, MessageDat.NewestMessage());
                    }

                break;
                }

            case 'S':
                {
                char prompt[128];

                mPrintfCR(getecmsg(282));

                sprintf(prompt, getecmsg(283), cfg.Lmsg_nym);

                const long msgnumber = getNumber(prompt, 0L, LONG_MAX, -1l, FALSE, NULL);

                if (msgnumber)
                    {
                    if (msgnumber > MessageDat.NewestMessage())
                        {
                        mPrintfCR(getmsg(391));
                        }
                    else
                        {
                        for (r_slot i = 0; i < cfg.maxrooms; i++)
                            {
                            ToEdit->SetRoomNewPointer(i, msgnumber);
                            }

                        doCR();
                        }
                    }

                break;
                }

            case ESC:
            case 'B':
                {
                mPrintfCR(getecmsg(24));
                return;
                }

            default:
                {
                mPrintfCR(getmsg(351));
                menu = TRUE;
                break;
                }
            }
        }
    }

Bool TERMWINDOWMEMBER PrivilegeEdit(LogEntry *EditLog, Bool UserlogEdit, l_index Index)
    {
    doCR();

    Bool prtMsg = !CurrentUser->IsExpert(), quit = FALSE, RetVal = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMsg)
            {
            Bool Col = FALSE;

            doCR();

            DisplayYesNoWithColumn(213, EditLog->IsSurnameLocked(), &Col);
            DisplayYesNoWithColumn(214, EditLog->IsUserSignatureLocked(), &Col);

            if (CurrentUser->IsSuperSysop() || onConsole || cfg.sysopOk || !UserlogEdit)
                {
                DisplayYesNoWithColumn(215, EditLog->IsSysop(), &Col);
                }

            if (cfg.SuperSysop && (CurrentUser->IsSuperSysop() || onConsole))
                {
                DisplayYesNoWithColumn(329, EditLog->IsSuperSysop(), &Col);
                }

            DisplayYesNoWithColumn(216, EditLog->IsAide(), &Col);
            DisplayYesNoWithColumn(217, EditLog->IsNode(), &Col);
            DisplayYesNoWithColumn(218, EditLog->IsPermanent(), &Col);
            DisplayYesNoWithColumn(219, EditLog->IsNetUser(), &Col);
            DisplayYesNoWithColumn(220, EditLog->IsProblem(), &Col);
            DisplayYesNoWithColumn(221, EditLog->IsMail(), &Col);
            DisplayYesNoWithColumn(222, EditLog->IsVerified(), &Col);
			DisplayYesNoWithColumn(355, EditLog->IsBanned(), &Col);

            if (cfg.borders)
                {
                DisplayYesNoWithColumn(223, EditLog->IsEnterBorders(), &Col);
                }

            DisplayYesNoWithColumn(224, EditLog->IsDefaultHallLocked(), &Col);
            DisplayOnOffWithColumn(225, EditLog->IsChat(), &Col);
            DisplayOnOffWithColumn(226, EditLog->IsMakeRoom(), &Col);
            DisplayOnOffWithColumn(227, EditLog->IsOut300(), &Col);
            DisplayYesNoWithColumn(228, EditLog->IsPsycho(), &Col);
            DisplayOnOffWithColumn(229, EditLog->IsUpload(), &Col);
            DisplayOnOffWithColumn(230, EditLog->IsDownload(), &Col);
            DisplayOnOffWithColumn(231, EditLog->IsPrintFile(), &Col);
            DisplayOnOffWithColumn(357, EditLog->IsKillOwn(), &Col);

            // from here down is not columnized...
            if (Col)
                {
                doCR();
                }

            label Buffer;
            mPrintfCR(getecmsg(212), EditLog->GetName(Buffer, sizeof(Buffer)));

            label tmp;
            mPrintfCR(getecmsg(232), EditLog->GetCallLimit() ? itoa(EditLog->GetCallLimit(), tmp, 10) : getecmsg(233));

            if (cfg.accounting)
                {
                mPrintf(getecmsg(234));

                if (EditLog->IsAccounting())
                    {
                    mPrintfCR(pcts, ltoac(EditLog->GetCredits() / 60));
                    }
                else
                    {
                    mPrintfCR(getecmsg(235));
                    }
                }

            if (UserlogEdit)
                {
                mPrintfCR(getecmsg(318));
                }

            if (onConsole || CurrentUser->IsSuperSysop())
                {
                label Buffer1;
                cPrintf(getecmsg(236), EditLog->GetInitials(Buffer, sizeof(Buffer)),
						EditLog->GetPassword(Buffer1, sizeof(Buffer1)));
                doccr();
                }

            CRmPrintfCR(UserlogEdit ? getmsg(633) : getecmsg(16));
            prtMsg = !CurrentUser->IsExpert();
            }

        int c = DoMenuPrompt(getecmsg(302), NULL);

        if (!HaveConnectionToUser())
            {
            return (FALSE);
            }

        if (c == ESC)
            {
            c = (UserlogEdit ? 'A' : 'B');
            }

        switch (toupper(c))
            {
            case '.':
                {
                if (UserlogEdit)
                    {
                    mPrintf(getecmsg(319));
                    configure(EditLog, FALSE, TRUE);
                    doCR();
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '*':
                {
                if (cfg.SuperSysop && (CurrentUser->IsSuperSysop() || onConsole))
                    {
                    EditLog->SetSuperSysop(changeYesNo(getecmsg(330), EditLog->IsSuperSysop()));
                    }
                else
                    {
                    BadMenuSelection(c);
                    }
                break;
                }

            case '1':
                {
                EditLog->SetPsycho(changeOnOff(getecmsg(263), EditLog->IsPsycho()));
                break;
                }

            case '2':
                {
                mPrintfCR(getecmsg(264));
                EditLog->SetCallLimit((int) GetNumberWithBlurb(getecmsg(265), 0, INT_MAX, EditLog->GetCallLimit(),
						B_CALLIMIT));
                break;
                }

            case '3':
                {
                EditLog->SetOut300(changeOnOff(getecmsg(266), EditLog->IsOut300()));
                break;
                }

            case '4':
                {
                if (cfg.borders)
                    {
                    EditLog->SetEnterBorders(changeYesNo(getecmsg(267), EditLog->IsEnterBorders()));
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '5':
                {
                EditLog->SetDownload(changeOnOff(getecmsg(239), EditLog->IsDownload()));
                break;
                }

            case '6':
                {
                EditLog->SetBanned(changeOnOff(getecmsg(356), EditLog->IsBanned()));
                break;
                }

            case 'A':
                {
                if (UserlogEdit)
                    {
                    mPrintfCR(getmsg(653));

                    if (getYesNo(getmsg(654), 1))
                        {
                        RetVal = FALSE;
                        quit = TRUE;
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'B':
                {
                if (!UserlogEdit)
                    {
                    mPrintf(getecmsg(24));
                    RetVal = TRUE;
                    quit = TRUE;
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'C':
                {
                EditLog->SetChat(changeOnOff(getecmsg(238), EditLog->IsChat()));
                break;
                }

            case 'D':
                {
                if (EditLog->IsSysop())
                    {
                    mPrintfCR(getecmsg(287));
                    CRmPrintfCR(getecmsg(288));
                    }
                else
                    {
                    EditLog->SetAide(changeYesNo(getecmsg(243), EditLog->IsAide()));
                    }

                break;
                }

            case 'E':
                {
                EditLog->SetNetUser(changeYesNo(getecmsg(240), EditLog->IsNetUser()));
                break;
                }

            case 'F':
                {
                EditLog->SetPrintFile(changeOnOff(getecmsg(241), EditLog->IsPrintFile()));
                break;
                }

            case 'H':
                {
                char Prompt[128];
                sprintf(Prompt, getecmsg(242), cfg.Lhall_nym);

                EditLog->SetDefaultHallLocked(changeYesNo(Prompt, EditLog->IsDefaultHallLocked()));
                break;
                }

            case 'I':
                {
                mPrintfCR(getecmsg(244));

                if (cfg.accounting)
                    {
                    EditLog->SetAccounting(!getYesNo(getecmsg(245), !EditLog->IsAccounting()));

                    if (EditLog->IsAccounting())
                        {
                        char string[200];

                        sprintf(string, getecmsg(246), cfg.Ucredits_nym);

                        EditLog->SetCredits(
								getNumber(string, 0, LONG_MAX / 60, EditLog->GetCredits() / 60, FALSE, NULL) * 60);
                        }
                    }
                else
                    {
                    CRmPrintfCR(getecmsg(247));
                    }

                break;
                }

            case 'K':
                {
                EditLog->SetUserSignatureLocked(changeOnOff(getecmsg(248), EditLog->IsUserSignatureLocked()));
                break;
                }

            case 'L':
                {
                EditLog->SetSurnameLocked(changeOnOff(getecmsg(249), EditLog->IsSurnameLocked()));
                break;
                }

            case 'M':
                {
                EditLog->SetMail(changeOnOff(getecmsg(250), EditLog->IsMail()));
                break;
                }

            case 'N':
                {
                mPrintfCR(getecmsg(251));

                label temp;
                AskUserName(temp, EditLog);

                l_slot NameSlot = FindPersonByName(temp);

                if (    // Didn't enter a name or...
                        !*temp ||

                        // name exists, and it is not us, or...
                        (
                            (NameSlot != CERROR) &&
                            (LTab(NameSlot).GetLogIndex() != Index)
                        ) ||

                        // it is the name of a mailing list
                        IsMailingList(temp))
                    {
                    if (*temp)
                        {
                        CRmPrintfCR(getmsg(475), temp);
                        }
                    }
                else
                    {
                    Bool Change = TRUE;

                    if (SameString(temp, getmsg(386)) || SameString(temp, getmsg(385)) || SameString(temp, cfg.nodeTitle))
                        {
                        CRmPrintf(getecmsg(253), temp);

                        if (!getYesNo(getecmsg(254), 0))
                            {
                            Change = FALSE;
                            }
                        }

                    if (Change && inExternal(getmsg(353), temp))
                        {
                        CRmPrintf(getecmsg(255), temp);

                        if (!getYesNo(getecmsg(254), 0))
                            {
                            Change = FALSE;
                            }
                        }

                    if (Change)
                        {
                        EditLog->SetName(temp);
                        }
                    }

                break;
                }

            case 'O':
                {
                EditLog->SetNode(changeYesNo(getecmsg(256), EditLog->IsNode()));

                label Buffer, Buffer1;
                if (EditLog->IsNode() && !*EditLog->GetAlias(Buffer, sizeof(Buffer)) &&
						!*EditLog->GetLocID(Buffer1, sizeof(Buffer1)))
                    {
                    Bool Valid = FALSE;

                    do
                        {
                        label string, temp;
                        char tempalias[4], templocID[5];

                        makeaddress(EditLog->GetAlias(Buffer, sizeof(Buffer)), 
                                EditLog->GetLocID(Buffer1, sizeof(Buffer1)), temp);

                        if (getString(getecmsg(322), string, 8, FALSE, temp))
                            {
                            normalizeString(string);

                            parse_address(string, tempalias, templocID, FALSE);

                            if (    !isaddress(string) ||

                                    (
                                        (addressexists(string) != CERROR) &&

                                        !(
                                        SameString(tempalias,
                                        EditLog->GetAlias(Buffer1,
                                        sizeof(Buffer1))) &&
                                        SameString(templocID,
                                        EditLog->GetLocID(Buffer,
                                        sizeof(Buffer)))
                                        )
                                    )

                                    ||

                                    (
                                        SameString(tempalias, cfg.alias) &&
                                        SameString(templocID, cfg.locID)
                                    )

                                    ||

                                    strlen(tempalias) < 2

                                    ||

                                    strlen(templocID) < 3
                                )
                                {
                                CRmPrintfCR(*string ? getecmsg(52) : getecmsg(321));
                                }
                            else
                                {
                                EditLog->SetAlias(tempalias);
                                EditLog->SetLocID(templocID);

                                Valid = TRUE;
                                }
                            }
                        else
                            {
                            EditLog->SetNode(FALSE);
                            CRmPrintfCR(getecmsg(320));
                            Valid = TRUE;
                            }
                        } while (!Valid);
                    }

                break;
                }

            case 'P':
                {
                EditLog->SetPermanent(changeYesNo(getecmsg(257), EditLog->IsPermanent()));
                break;
                }

            case 'R':
                {
                EditLog->SetMakeRoom(changeOnOff(getecmsg(258), EditLog->IsMakeRoom()));
                break;
                }

            case 'S':
                {
                if (UserlogEdit)
                    {
                    mPrintfCR(getmsg(652));

                    if (getYesNo(getmsg(652), 1))
                        {
                        quit = TRUE;
                        RetVal = TRUE;
                        EditLog->Save(Index, EditLog->GetMessageRoom());
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'T':
                {
                if (EditLog->IsSysop())
                    {
                    mPrintfCR(getecmsg(289));
                    CRmPrintf(getecmsg(290));
                    }
                else
                    {
                    EditLog->SetProblem(changeYesNo(getecmsg(260), EditLog->IsProblem()));
                    }

                break;
                }

            case 'U':
                {
                EditLog->SetUpload(changeOnOff(getecmsg(262), EditLog->IsUpload()));
                break;
                }

            case 'V':
                {
                EditLog->SetVerified(changeYesNo(getecmsg(261), EditLog->IsVerified()));
                break;
                }

            case 'W':
                {
                EditLog->SetKillOwn(changeOnOff(getecmsg(358), EditLog->IsKillOwn()));
                break;
                }

            case 'Y':
                {
                if (onConsole || cfg.sysopOk || CurrentUser->IsSuperSysop() || !UserlogEdit)
                    {
                    if (EditLog->IsSuperSysop())
                        {
                        mPrintfCR(getecmsg(331));
                        CRmPrintfCR(getecmsg(332));
                        }
                    else
                        {
                        EditLog->SetSysop(changeYesNo(getecmsg(259), EditLog->IsSysop()));
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '?':
            case '\r':
            case '\n':
                {
                mPrintfCR(getmsg(351));
                prtMsg = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                break;
                }
            }
        } while (!quit);

    return (RetVal);
    }


// --------------------------------------------------------------------------
// configure(): Sets user configuration via menu.

Bool TERMWINDOWMEMBER configure(LogEntry *EditLog, Bool newUser,
        Bool UserlogEdit)
    {
    Bool prtMess = !CurrentUser->IsExpert();
    Bool quit = FALSE;
    const Bool oldcensor = EditLog->IsViewCensoredMessages();
    Bool edited_pointers = FALSE;

    doCR();

    Bool RetVal = FALSE;

    do
        {
        OC.SetOutFlag(OUTOK);

        if (prtMess || newUser)
            {
            label dProtocol;

            Bool Col = FALSE;

            // Get the Upper/Lower correct on the default hall
            label Buffer;
            if (*EditLog->GetDefaultHall(Buffer, sizeof(Buffer)))
                {
                label dHall;
                *dHall = 0;

                for (h_slot i = 0; i < cfg.maxhalls; ++i)
                    {
                    label Buffer1;
                    if (HallData[i].IsInuse() && SameString(HallData[i].GetName(Buffer1, sizeof(Buffer1)),
                            EditLog->GetDefaultHall(Buffer, sizeof(Buffer))))
                        {
                        if (EditLog->CanAccessHall(i))
                            {
                            HallData[i].GetName(dHall, sizeof(dHall));
                            }
                        }
                    }

                EditLog->SetDefaultHall(dHall);
                }

            if (EditLog->GetDefaultProtocol())
                {
                const protocols *theProt = GetProtocolByKey(EditLog->GetDefaultProtocol());

                if (theProt)
                    {
                    CopyStringToBuffer(dProtocol, theProt->name);
                    }
                else
                    {
                    *dProtocol = 0;
                    }
                }
            else
                {
                *dProtocol = 0;
                }

            doCR();

            DisplayOnOffWithColumn(136, EditLog->IsViewRoomDesc(), &Col);
            DisplayOnOffWithColumn(137, EditLog->IsViewHallDescription(), &Col);
            DisplayOnOffWithColumn(138, EditLog->IsViewRoomInfoLines(), &Col);
            DisplayOnOffWithColumn(139, EditLog->IsWideRoom(), &Col);
            DisplayOnOffWithColumn(140, EditLog->IsAutoNextHall(), &Col);

            DisplayOnOffWithColumn(142, EditLog->IsAutoVerbose(), &Col);

            if (!EditLog->IsAutoVerbose())
                {
                DisplayOnOffWithColumn(143, EditLog->IsVerboseLogOut(), &Col);
                }

            if (cfg.ecUserlog || UserlogEdit || CurrentUser->IsSysop())
                {
                DisplayYesNoWithColumn(144, !EditLog->IsUnlisted(), &Col);
                }

            DisplayOnOffWithColumn(145, !EditLog->IsExpert(), &Col);
            DisplayOnOffWithColumn(146, EditLog->IsYouAreHere(), &Col);
            DisplayOnOffWithColumn(148, EditLog->IsMinibin(), &Col);

            mPrintf(getecmsg(149), EditLog->GetNumUserShow());
            doColumn(&Col);

            if (cfg.surnames || cfg.netsurname || cfg.titles || cfg.nettitles)
                {
                DisplayOnOffWithColumn(154, EditLog->IsViewTitleSurname(), &Col);
                }

            if (cfg.borders)
                {
                DisplayOnOffWithColumn(156, EditLog->IsViewBorders(), &Col);
                }

            DisplayYesNoWithColumn(157, EditLog->IsViewCommas(), &Col);
            DisplayYesNoWithColumn(286, EditLog->IsPUnPauses(), &Col);

            // from here down is not columnized...
            if (Col)
                {
                doCR();
                }

            EditLog->GetDefaultHall(Buffer, sizeof(Buffer));
            mPrintfCR(getecmsg(158), *Buffer ? Buffer : getecmsg(61));

            mPrintfCR(getecmsg(159), dProtocol);

            if (cfg.titles && (cfg.entersur || UserlogEdit || CurrentUser->IsSysop()))
                {
                mPrintfCR(getecmsg(323), EditLog->GetTitle(Buffer, sizeof(Buffer)));
                }

            if (cfg.surnames && (cfg.entersur || UserlogEdit || CurrentUser->IsSysop()))
                {
                mPrintfCR(getecmsg(324), EditLog->GetSurname(Buffer, sizeof(Buffer)));
                }

            if (cfg.ecSignature || UserlogEdit || CurrentUser->IsSysop())
                {
                char Buffer[91];
                mPrintfCR(getecmsg(160), EditLog->GetSignature(Buffer, sizeof(Buffer)));
                }

            label Buffer1;
            mPrintf(getecmsg(161));
            if (!EditLog->IsForwardToNode())
                {
                mPrintfCR(getecmsg(4), EditLog->GetForwardAddr(Buffer, sizeof(Buffer)));
                }
            else
                {
                mPrintfCR(getecmsg(3), EditLog->GetForwardAddr(Buffer1, sizeof(Buffer1)),
						EditLog->GetForwardAddrNode(Buffer, sizeof(Buffer)));
                }

			label Buffer2;
            mPrintfCR(getecmsg(162), makeaddress(EditLog->GetAlias(Buffer1, sizeof(Buffer1)),
					EditLog->GetLocID(Buffer, sizeof(Buffer)), Buffer2));

            mPrintfCR(getecmsg(163), CurrentUser->IsExpert() ? ns : getecmsg(164));
            mPrintfCR(getecmsg(165), CurrentUser->IsExpert() ? ns : getecmsg(166));
            mPrintfCR(getecmsg(167), CurrentUser->IsExpert() ? ns : getecmsg(168));

#ifdef MULTI
            mPrintfCR(getecmsg(335), CurrentUser->IsExpert() ? ns : getecmsg(336));
#endif

            mPrintfCR(getecmsg(169), cfg.Umsg_nym, CurrentUser->IsExpert() ? ns : getecmsg(170));

            mPrintfCR(getecmsg(171), CurrentUser->IsExpert() ? ns : getecmsg(172));

            mPrintfCR(getecmsg(173), cfg.Umsg_nym, CurrentUser->IsExpert() ? ns : getecmsg(174));

            if (!UserlogEdit && CurrentUser->IsSysop())
                {
                mPrintfCR(getecmsg(237));
                }

            if (!newUser)
                {
                CRmPrintfCR(UserlogEdit ? getecmsg(16) : getmsg(633));
                }

            prtMess = !CurrentUser->IsExpert();
            }

        if (newUser)
            {
            if (getYesNo(getecmsg(175), 1))
                {
                quit = TRUE;
                continue;
                }

            newUser = FALSE;
            }

        int c = DoMenuPrompt(getmsg(616), NULL);

        if (!HaveConnectionToUser())
            {
            return (FALSE);
            }

        if (c == ESC)
            {
            c = (UserlogEdit ? 'B' : 'A');
            }

        switch (toupper(c))
            {
            case ',':
                {
                EditLog->SetViewCommas(changeYesNo(getecmsg(176), EditLog->IsViewCommas()));
                break;
                }

            case '&':
                {
                label stuff, Input, Buffer, Buffer1;

                mPrintfCR(getecmsg(328));

                makeaddress(EditLog->GetAlias(Buffer, sizeof(Buffer)), EditLog->GetLocID(Buffer1, sizeof(Buffer1)), stuff);

                if (getString(getecmsg(322), Input, 8, FALSE, stuff))
                    {
                    normalizeString(Input);

                    if (*Input)
                        {
                        char tempalias[4], templocID[5];

                        parse_address(Input, tempalias, templocID, FALSE);

                        if (    !isaddress(Input) ||

                                (
                                    (addressexists(Input) != CERROR) &&

                                    !(
                                    SameString(tempalias, EditLog->GetAlias(Buffer, sizeof(Buffer))) &&
                                    SameString(templocID, EditLog->GetLocID(Buffer1, sizeof(Buffer1)))
                                    )
                                )

                                ||

                                (
                                    SameString(tempalias, cfg.alias) && SameString(templocID, cfg.locID)
                                )

                                ||

                                strlen(tempalias) < 2

                                ||

                                strlen(templocID) < 3
                            )
                            {
                            CRmPrintfCR(getecmsg(52));
                            }
                        else
                            {
                            EditLog->SetAlias(tempalias);
                            EditLog->SetLocID(templocID);
                            }
                        }
                    else
                        {
                        EditLog->SetAlias(ns);
                        EditLog->SetLocID(ns);
                        }
                    }

                break;
                }

            case '>':
                {
                mPrintf(getecmsg(199), cfg.Lhall_nym);

                defaulthall(EditLog, UserlogEdit);
                break;
                }

            case '#':
                {
                mPrintfCR(getecmsg(185));
                EditLog->SetNumUserShow((int) GetNumberWithBlurb(getecmsg(186),
                        0, INT_MAX, EditLog->GetNumUserShow(), B_NUMUSERS));
                break;
                }

            case '-':
                {
                char Prompt[128];
                sprintf(Prompt, getecmsg(195), cfg.Uhall_nym);

                EditLog->SetViewHallDescription(changeOnOff(Prompt, EditLog->IsViewHallDescription()));
                break;
                }

            case '+':
                {
                EditLog->SetViewRoomInfoLines(changeOnOff(getecmsg(196), EditLog->IsViewRoomInfoLines()));
                break;
                }

            case '_':
                {
                if ((cfg.ecSignature || UserlogEdit || CurrentUser->IsSysop()))
                    {
                    mPrintfCR(getmsg(17));

                    if (!UserlogEdit && !EditLog->IsSysop() && EditLog->IsUserSignatureLocked())
                        {
                        CRmPrintfCR(getecmsg(209));
                        doCR();
                        }
                    else
                        {
                        char stuff[91], Buffer[91];

                        if (GetStringWithBlurb(getecmsg(210), stuff, 90, EditLog->GetSignature(Buffer, sizeof(Buffer)),
                                B_SIGNATUR) && *stuff)
                            {
                            normalizeString(stuff);
                            EditLog->SetSignature(stuff);
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '.':
                {
                if (!UserlogEdit && CurrentUser->IsSysop())
                    {
                    mPrintf(getecmsg(211));
                    PrivilegeEdit(EditLog, FALSE, ThisLog);
                    doCR();
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '1':
                {
                EditLog->SetWideRoom(changeOnOff(getecmsg(198), EditLog->IsWideRoom()));
                break;
                }

            case '2':
                {
                if (cfg.borders)
                    {
                    EditLog->SetViewBorders(changeOnOff(getecmsg(183), EditLog->IsViewBorders()));
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '3':
                {
                if (cfg.titles && (cfg.entersur || UserlogEdit || CurrentUser->IsSysop()))
                    {
                    mPrintfCR(getecmsg(325));

                    if (!UserlogEdit && !EditLog->IsSysop() && EditLog->IsSurnameLocked())
                        {
                        CRmPrintfCR(getmsg(252));
                        doCR();
                        }
                    else
                        {
                        label NewTitle, Buffer;

                        // Note: getmsg(253) also in doenter.cpp
                        if (GetStringWithBlurb(getmsg(253), NewTitle, LABELSIZE, EditLog->GetTitle(Buffer, sizeof(Buffer)),
								B_TITLESUR) && *NewTitle)
                            {
                            normalizeString(NewTitle);
                            EditLog->SetTitle(NewTitle);
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '4':
                {
                if (cfg.surnames && (cfg.entersur || UserlogEdit || CurrentUser->IsSysop()))
                    {
                    mPrintfCR(getecmsg(326));

                    if (!UserlogEdit && !EditLog->IsSysop() && EditLog->IsSurnameLocked())
                        {
                        CRmPrintfCR(getmsg(252));
                        doCR();
                        }
                    else
                        {
                        label NewSurname, Buffer;

                        // Note: getmsg(254) also in doenter.cpp
                        if (GetStringWithBlurb(getmsg(254), NewSurname, LABELSIZE,
								EditLog->GetSurname(Buffer, sizeof(Buffer)), B_TITLESUR) && *NewSurname)
                            {
                            normalizeString(NewSurname);
                            EditLog->SetSurname(NewSurname);
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case '5':
                {
                mPrintfCR(getecmsg(188));

                editFormatStrings(EditLog);
                break;
                }

            case '6':
                {
                mPrintfCR(getecmsg(203));
                editPersonalInfo(EditLog);
                break;
                }

#ifdef MULTI
			case '7':
				{
				mPrintfCR(getecmsg(337));
				editSystemEvents(EditLog);
				break;
				}
#endif

            case 'A':
                {
                if (!UserlogEdit)
                    {
                    mPrintfCR(getmsg(653));

                    if (getYesNo(getmsg(654), 1))
                        {
                        quit = TRUE;
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'B':
                {
                if (UserlogEdit)
                    {
                    mPrintf(getecmsg(24));
                    RetVal = TRUE;
                    quit = TRUE;
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'D':
                {
                EditLog->SetViewRoomDesc(changeOnOff(getecmsg(194), EditLog->IsViewRoomDesc()));
                break;
                }

            case 'E':
                {
                mPrintfCR(getecmsg(205), cfg.Umsg_nym);
                messageEditOptions(EditLog);
                break;
                }

            case 'F':
                {
                mPrintf(getecmsg(200));
                forwardaddr(EditLog);
                break;
                }

            case 'H':
                {
                EditLog->SetExpert(!changeOnOff(getecmsg(191), !EditLog->IsExpert()));
                prtMess = !CurrentUser->IsExpert();
                break;
                }

            case 'I':
                {
                if (!(cfg.surnames || cfg.netsurname || cfg.titles || cfg.nettitles))
                    {
                    break;
                    }

                EditLog->SetViewTitleSurname(!EditLog->IsViewTitleSurname());
                mPrintfCR(getecmsg(179), EditLog->IsViewTitleSurname() ? ns : getecmsg(180));
                break;
                }

            case 'L':
                {
                if (toupper(c) == 'L' && !EditLog->IsAutoVerbose())
                    {
                    EditLog->SetVerboseLogOut(changeOnOff(getecmsg(208), EditLog->IsVerboseLogOut()));
                    }
                else
                    {
                    mPrintfCR(getecmsg(10));
                    }

                break;
                }

            case 'M':
                {
                EditLog->SetMinibin(changeOnOff(getecmsg(190), EditLog->IsMinibin()));
                break;
                }

            case 'N':
                {
                EditLog->SetPUnPauses(changeYesNo(getecmsg(285), EditLog->IsPUnPauses()));
                break;
                }

            case 'P':
                {
                int ich = 0;
                mPrintfCR(getecmsg(201));

                do
                    {
                    if (!CurrentUser->IsExpert() || (ich == '?'))
                        {
                        if (ich == '?')
                            {
                            oChar((char) ich);
                            doCR();
                            }

                        doCR();

                        for (protocols *theProt = extProtList; theProt; theProt = (protocols *) getNextLL(theProt))
                            {
                            if (!theProt->NetOnly)
                                {
                                mPrintfCR(getmsg(361), theProt->CommandKey, theProt->MenuName);
                                }
                            }
                        }

                    ich = DoMenuPrompt(getecmsg(202), NULL);
                    } while (ich == '?');

                if (ich == ' ')
                    {
                    EditLog->SetDefaultProtocol(0);
                    }
                else
                    {
                    const protocols *theProt = GetProtocolByKey((char) ich);

                    if (theProt)
                        {
                        mPrintfCR(pcts, theProt->name);
                        EditLog->SetDefaultProtocol((char) ich);
                        }
                    else
                        {
                        BadMenuSelection(ich);
                        }
                    }

                break;
                }

            case 'R':
                {
                mPrintfCR(getecmsg(204), cfg.Lmsg_nym);
                editMsgOption(EditLog, UserlogEdit, &edited_pointers);
                break;
                }

            case 'S':
                {
                if (!UserlogEdit)
                    {
                    mPrintfCR(getmsg(652));

                    if (getYesNo(getmsg(652), 1))
                        {
                        quit = TRUE;
                        RetVal = TRUE;
                        storeLog();

                        if (oldcensor != EditLog->IsViewCensoredMessages() || edited_pointers)
                            {
                            Talley->Fill();
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(c);
                    }

                break;
                }

            case 'T':
                {
                mPrintfCR(getecmsg(177));

                editTerminal(EditLog, UserlogEdit);
                break;
                }

            case 'U':
                {
                if (toupper(c) == 'U' && (cfg.ecUserlog || UserlogEdit || CurrentUser->IsSysop()))
                    {
                    EditLog->SetUnlisted(!changeYesNo(getecmsg(207), !EditLog->IsUnlisted()));
                    break;
                    }
                }

            case 'V':
                {
                EditLog->SetAutoVerbose(changeOnOff(getecmsg(184), EditLog->IsAutoVerbose()));
                break;
                }

            case 'X':
                {
                char Prompt[128];
                sprintf(Prompt, getecmsg(197), cfg.Lhall_nym);

                EditLog->SetAutoNextHall(changeOnOff(Prompt, EditLog->IsAutoNextHall()));
                break;
                }

            case 'Y':
                {
                EditLog->SetYouAreHere(changeOnOff(getecmsg(192), EditLog->IsYouAreHere()));
                break;
                }

            case 'Z':
                {
                userPointers(EditLog, !UserlogEdit);
                edited_pointers = TRUE;
                break;
                }

            case '\r':
            case '\n':
            case '?':
                {
                mPrintfCR(getmsg(351));
                prtMess = TRUE;
                break;
                }

            default:
                {
                BadMenuSelection(c);
                }
            }
        } while (!quit);

    return (RetVal);
    }

Bool TERMWINDOWMEMBER DoUserlogEdit(LogEntry *EditLog, l_index Index)
    {
    if (!read_ec_messages())
        {
        OutOfMemory(93);
        return (FALSE);
        }

#ifdef WINCIT
    EditLog->PauseSyncSend();
#endif

    Bool RetVal = PrivilegeEdit(EditLog, TRUE, Index);

    if (RetVal)
        {
#ifdef WINCIT
        EditLog->ResumeSyncSend();
#endif

    	LogTab.UpdateTable(EditLog, Index);
	}

    dump_ec_messages();

    return (RetVal);
    }

void TERMWINDOWMEMBER EnterConfiguration(Bool newUser)
    {
    SetDoWhat(ENTERCONFIG);

    if (!read_ec_messages())
        {
        OutOfMemory(94);
        return;
        }

    storeLog();

#ifdef WINCIT
    CurrentUser->PauseSyncSend();
#endif

    if (configure(CurrentUser, newUser, FALSE))
        {
#ifdef WINCIT
        CurrentUser->ResumeSyncSend();
#endif

        if (loggedIn)
		    {
            LogTab.UpdateTable(CurrentUser, ThisLog);
            }
        }

    dump_ec_messages();

    if (loggedIn)
        {
        // load it back to compact memory - log extensions.
        CurrentUser->Load(ThisLog);
        }

    // If already resumed above, this is harmless.
#ifdef WINCIT
    CurrentUser->ResumeSyncSend();
#endif

    OC.Psycho = CurrentUser->IsPsycho();

    label Buffer;
    if (!TermCap->Load(CurrentUser->GetTermType(Buffer, sizeof(Buffer))))
        {
        mPrintfCR(getmsg(MSG_LOGIN, 40), CurrentUser->GetTermType(Buffer, sizeof(Buffer)));
        }

    if (loggedIn)
		{
        LogTab.UpdateTable(CurrentUser, ThisLog);
        }
    }
