// --------------------------------------------------------------------------
// Citadel: GrpEdit.CPP
//
// Code to edit groups.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// killgroup()      sysop special to kill a group
// AideListGroup()  sysop/aide fn to list groups
// newgroup()       sysop special to add a new group
// renamegroup()    sysop special to rename a group


// --------------------------------------------------------------------------
// killgroup(): Sysop special to kill a group.
//
// Notes:
//  Assumes MSG_SYSOP is locked.

int TERMWINDOWMEMBER killgroup(const char *gname)
    {
    label groupname;

    if (!gname)
        {
        doCR();
        AskGroupName(groupname, ns);
        }
    else
        {
        CopyStringToBuffer(groupname, gname);
        }

    const g_slot groupslot = FindGroupByPartialName(groupname, FALSE);

    if (groupslot == 0 || groupslot == 1)
        {
        if (!gname)
            {
            label Buffer1, Buffer2;
            CRmPrintfCR(getsysmsg(210), GroupData[EVERYBODY].GetName(Buffer1, sizeof(Buffer1)),
					GroupData[SPECIALSECURITY].GetName(Buffer2, sizeof(Buffer2)), cfg.Lgroups_nym);
            }

        return (FALSE);
        }

    if (groupslot == CERROR || !(*groupname))
        {
        if (!gname)
            {
            CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
            }

        return (FALSE);
        }

    for (g_slot i = 0; i < cfg.maxrooms; i++)
        {
        if (RoomTab[i].IsInuse() && (RoomTab[i].IsGroupOnly() || RoomTab[i].IsPrivilegedGroup()))
            {
            Bool IsInuse = FALSE;

            if (RoomTab[i].IsGroupOnly())
                {
                if (RoomTab[i].IsBooleanGroup())
                    {
                    BoolExpr Buffer;
                    IsInuse = IsInBooleanExpression(RoomTab[i].GetGroupExpression(Buffer), groupslot);
                    }
                else
                    {
                    IsInuse = (RoomTab[i].GetGroupNumber() == groupslot);
                    }
                }

            if (!IsInuse && RoomTab[i].IsPrivilegedGroup())
                {
                IsInuse = (RoomTab[i].GetPGroupNumber() == groupslot);
                }

            if (IsInuse)
                {
                if (!gname)
                    {
                    CRmPrintfCR(getsysmsg(208), cfg.Ugroup_nym, cfg.Lroom_nym);
                    }

                return (FALSE);
                }
            }
        }
EXTRA_OPEN_BRACE    

    for (h_slot i = 0; i < cfg.maxhalls; i++)
        {
        if (HallData[i].IsInuse() && HallData[i].IsOwned())
            {
            Bool IsInuse;

            if (HallData[i].IsBoolGroup())
                {
                BoolExpr Buffer;
                IsInuse = IsInBooleanExpression(HallData[i].GetGroupExpression(Buffer), groupslot);
                }
            else
                {
                IsInuse = (HallData[i].GetGroupNumber() == groupslot);
                }

            if (IsInuse)
                {
                if (!gname)
                    {
                    CRmPrintfCR(getsysmsg(208), cfg.Ugroup_nym, cfg.Lhalls_nym);
                    }

                return (FALSE);
                }
            }
        }
EXTRA_CLOSE_BRACE    

    label Buffer;
    char Question[256];
    sprintf(Question, getsysmsg(84), cfg.Lgroup_nym, GroupData[groupslot].GetName(Buffer, sizeof(Buffer)));

    if (!gname && !getYesNo(Question, 0))
        {
        return (FALSE);
        }

    GroupData[groupslot].SetInuse(FALSE);
    GroupData.Save();

#ifdef WINCIT
    trap(T_SYSOP, WindowCaption, getsysmsg(206), groupname);
#else
    trap(T_SYSOP, getsysmsg(206), groupname);
#endif 

    return (TRUE);
    }


// --------------------------------------------------------------------------
// AideListGroup(): Sysop/aide fn to list groups.

void TERMWINDOWMEMBER AideListGroup(void)
    {
    label groupname;

    SetDoWhat(AIDELIST);

    doCR();

    do
        {
        if (!getString(cfg.Lgroup_nym, groupname, LABELSIZE, ns))
            {
            return;
            }

        if (*groupname == '?')
            {
            ListGroups(FALSE);
            }
        } while (*groupname == '?');

    OC.SetOutFlag(OUTOK);

    if (!*groupname)
        {
        g_slot GroupSlot;

        doCR();

        for (GroupSlot = 0; GroupSlot < cfg.maxgroups; GroupSlot++)
            {
            // can they see the group
            if (GroupData[GroupSlot].IsInuse() &&
                    (!GroupData[GroupSlot].IsLocked() || CurrentUser->IsSysop()) &&
                    (!GroupData[GroupSlot].IsHidden() || onConsole || CurrentUser->IsSuperSysop() || CurrentUser->IsInGroup(GroupSlot)))
                {
                label Buffer;
                mPrintf(getmsg(232),
                        (CurrentUser->IsInGroup(GroupSlot)) ? '*' : ' ',
                        GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)),
                        GroupData[GroupSlot].IsLocked() ? 'L' : ' ',
                        GroupData[GroupSlot].IsHidden() ? 'H' : ' ',
                        (GroupData[GroupSlot].IsAutoAdd() || !GroupSlot) ? 'A' : ' ');

                char Desc[80];
                mPrintfCR(getmsg(331), GroupData[GroupSlot].GetDescription(Desc, sizeof(Desc)));
                }
            }
        }
    else
        {
        g_slot GroupSlot;
        l_slot i;   // this is a l_slot and r_slot and h_slot...

        GroupSlot = FindGroupByPartialName(groupname, FALSE);

        if (GroupSlot == CERROR || (GroupData[GroupSlot].IsHidden() && !CurrentUser->IsInGroup(GroupSlot)))
            {
            CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
            return;
            }

        if (GroupData[GroupSlot].IsLocked() && (!(CurrentUser->IsSysop() || onConsole)))
            {
            CRmPrintf(getmsg(581), cfg.Ugroup_nym);
            return;
            }

        doCR();

        OC.SetOutFlag(OUTOK);

        mPrintfCR(getmsg(578), cfg.Uusers_nym);

        prtList(LIST_START);

        for (i = 0; ((i < cfg.MAXLOGTAB) && (OC.User.GetOutFlag() != OUTSKIP) && (OC.User.GetOutFlag() != OUTNEXT)); i++)
            {
            if (LTab(i).IsInuse())
                {
                LogEntry2 Log2(cfg.maxgroups);

                if (Log2.Load(LTab(i).GetLogIndex()))
                    {
                    if (Log2.IsInGroup(GroupSlot) ||
                            ((cfg.showSysop || (loggedIn && CurrentUser->IsMainSysop()))
                            && LTab(i).IsMainSysop()))
                        {
                        label Buffer;
                        prtList(LTab(i).GetName(Buffer, sizeof(Buffer)));
                        }
                    else
                        {
#ifdef MULTI
                        OC.CheckInput(FALSE, this);
#else
                        OC.CheckInput(FALSE);
#endif
                        }
                    }
                }

            if (OC.User.GetOutFlag() == OUTSKIP)
                {
                return;
                }
            }

        prtList(LIST_END);

        if (OC.User.GetOutFlag() == OUTSKIP)
            {
            return;
            }

        OC.SetOutFlag(OUTOK);

        CRmPrintfCR(getmsg(578), cfg.Urooms_nym);

        prtList(LIST_START);

        for (i = 0; (i < cfg.maxrooms) && (OC.User.GetOutFlag() != OUTSKIP) && (OC.User.GetOutFlag() != OUTNEXT); i++)
            {
            if (RoomTab[i].IsInuse() && (RoomTab[i].IsGroupOnly() || RoomTab[i].IsPrivilegedGroup()))
                {
                Bool ShowIt = FALSE;

                if (RoomTab[i].IsGroupOnly())
                    {
                    if (RoomTab[i].IsBooleanGroup())
                        {
                        BoolExpr Buffer;
                        ShowIt = IsInBooleanExpression(RoomTab[i].GetGroupExpression(Buffer), GroupSlot);
                        }
                    else
                        {
                        ShowIt = (RoomTab[i].GetGroupNumber() == GroupSlot);
                        }
                    }

                if (!ShowIt && RoomTab[i].IsPrivilegedGroup())
                    {
                    ShowIt = (RoomTab[i].GetPGroupNumber() == GroupSlot);
                    }

                if (ShowIt)
                    {
                    label Buffer;
                    prtList(RoomTab[i].GetName(Buffer, sizeof(Buffer)));
                    }
                }
            }

        prtList(LIST_END);

        if (OC.User.GetOutFlag() == OUTSKIP)
            {
            return;
            }

        OC.SetOutFlag(OUTOK);

        CRmPrintfCR(getmsg(578), cfg.Uhalls_nym);

        prtList(LIST_START);

        for (i = 0; (i < cfg.maxhalls) && (OC.User.GetOutFlag() != OUTSKIP) && (OC.User.GetOutFlag() != OUTNEXT); i++)
            {
            if (HallData[i].IsInuse() && HallData[i].IsOwned())
                {
                Bool ShowIt;

                if (HallData[i].IsBoolGroup())
                    {
                    BoolExpr Buffer;
                    ShowIt = IsInBooleanExpression(HallData[i].GetGroupExpression(Buffer), GroupSlot);
                    }
                else
                    {
                    ShowIt = (HallData[i].GetGroupNumber() == GroupSlot);
                    }

                if (ShowIt)
                    {
                    label Buffer;
                    prtList(HallData[i].GetName(Buffer, sizeof(Buffer)));
                    }
                }
            }

        prtList(LIST_END);
        }
    }


// --------------------------------------------------------------------------
// newgroup(): Sysop special to add a new group.

Bool TERMWINDOWMEMBER newgroup(const char *name, const char *desc, Bool lock,
        Bool hide, Bool autoadd)
    {
    label groupname;
    g_slot slot = 0;

    // search for a free group slot
    for (g_slot i = 0; i < cfg.maxgroups && !slot; i++)
        {
        if (!GroupData[i].IsInuse())
            {
            slot = i;
            }
        }

    if (!slot)
        {
        if (!name)
            {
            CRmPrintf(getmsg(490), cfg.Ugroup_nym);
            }

        return (FALSE);
        }

    if (name)
        {
        if ((!*name) || (FindGroupByName(name) != CERROR))
            {
            return (FALSE);
            }

        GroupData[slot].SetName(name);
        GroupData[slot].SetDescription(desc);
        GroupData[slot].SetLocked(lock);
        GroupData[slot].SetHidden(hide);
        GroupData[slot].SetAutoAdd(autoadd);
        }
    else
        {
        char Prompt[128];
        sprintf(Prompt, getmsg(106), cfg.Lgroup_nym);

        Bool test = FALSE;
        do
            {
            doCR();

            if (!AskGroupName(groupname, ns, Prompt))
                {
                return (FALSE);
                }

            if (FindGroupByName(groupname) != CERROR)
                {
                CRmPrintf(getmsg(105), groupname, cfg.Lgroup_nym);
                }
            else
                {
                test = TRUE;
                }
            } while (!test);

        char Desc[80];
        sprintf(Prompt, getmsg(107), cfg.Lgroup_nym);
        getNormStr(Prompt, Desc, 79);

        GroupData[slot].SetDescription(Desc);
        GroupData[slot].SetLocked(getYesNo(getsysmsg(259), 0));
        GroupData[slot].SetHidden(getYesNo(getsysmsg(260), 0));
        GroupData[slot].SetAutoAdd(getYesNo(getsysmsg(261), 0));
        GroupData[slot].SetName(groupname);
        }

    GroupData[slot].SetInuse(TRUE);

    if (name || getYesNo(getmsg(57), 0))
        {
        cyclegroup(slot);

        GroupData.Save();

        label Buffer;
#ifdef WINCIT
        trap(T_SYSOP, WindowCaption, getsysmsg(262), GroupData[slot].GetName(Buffer, sizeof(Buffer)));
#else
        trap(T_SYSOP, getsysmsg(262), GroupData[slot].GetName(Buffer, sizeof(Buffer)));
#endif

        CurrentUser->SetInGroup(slot, TRUE);

        storeLog();

        if (!name)
            {
            globalgroup(GroupData[slot].GetName(Buffer, sizeof(Buffer)), getYesNo(getsysmsg(263), 0));
            }

        return (TRUE);
        }
    else
        {
        if (!GroupData.Load())
            {
            mPrintfCR(getmsg(656));
            }

        return (FALSE);
        }
    }


// --------------------------------------------------------------------------
// renamegroup(): Sysop special to rename a group.

Bool TERMWINDOWMEMBER renamegroup(void)
    {
    label Buffer;
    label groupname, newname;
    char desc[80];
    Bool prtMess = !CurrentUser->IsExpert();
    Bool quit = FALSE;

    doCR();

    if (!AskGroupName(groupname, ns))
        {
        return (FALSE);
        }

    g_slot GroupSlot = FindGroupByPartialName(groupname, FALSE);

    if (GroupSlot != CERROR && (GroupData[GroupSlot].IsHidden() &&
            !CurrentUser->IsInGroup(GroupSlot) && !onConsole && !CurrentUser->IsSuperSysop()))
        {
        GroupSlot = CERROR;
        }

    if (GroupSlot == CERROR)
        {
        CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
        return (FALSE);
        }

    do
        {
        if (prtMess)
            {
            doCR();
            OC.SetOutFlag(OUTOK);

            mPrintfCR(getsysmsg(177), GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)));

            char Desc[80];
            GroupData[GroupSlot].GetDescription(Desc, sizeof(Desc));
            mPrintfCR(getsysmsg(178), *Desc ? Desc : getsysmsg(179));

            displayYesNo(getsysmsg(180), GroupData[GroupSlot].IsLocked());
            doCR();

            displayYesNo(getsysmsg(181), GroupData[GroupSlot].IsHidden());
            doCR();

            if (!GroupSlot)
                {
                GroupData[GroupSlot].SetAutoAdd(TRUE);
                }

            displayYesNo(getsysmsg(182), GroupData[GroupSlot].IsAutoAdd());

            CRCRmPrintfCR(getmsg(633));

            prtMess = !CurrentUser->IsExpert();
            }

        const int c = DoMenuPrompt(getmsg(616), NULL);

        if (!(HaveConnectionToUser()))
            {
            GroupData.Load();
            return (FALSE);
            }

        switch (toupper(c))
            {
            case ESC:
            case 'A':
                {
                mPrintfCR(getmsg(653));

                if (getYesNo(getmsg(654), TRUE))
                    {
                    if (!GroupData.Load())
                        {
                        mPrintfCR(getmsg(656));
                        }

                    return (FALSE);
                    }

                break;
                }

            case 'D':
                {
                char Prompt[128];

                mPrintfCR(getsysmsg(186));

                sprintf(Prompt, getsysmsg(187), cfg.Lgroup_nym);

                char Desc[80];
                getString(Prompt, desc, 79, FALSE, GroupData[GroupSlot].GetDescription(Desc, sizeof(Desc)));

                if (*desc)
                    {
                    normalizeString(desc);
                    GroupData[GroupSlot].SetDescription(desc);
                    }

                break;
                }

            case 'H':
                {
                char Prompt[128];

                mPrintfCR(getsysmsg(190));

                sprintf(Prompt, getsysmsg(191), cfg.Lgroup_nym);

                GroupData[GroupSlot].SetHidden(getYesNo(Prompt, GroupData[GroupSlot].IsHidden()));
                break;
                }

            case 'L':
                {
                char Prompt[128];

                mPrintfCR(getsysmsg(188));

                sprintf(Prompt, getsysmsg(189), cfg.Lgroup_nym);

                GroupData[GroupSlot].SetLocked(getYesNo(Prompt, GroupData[GroupSlot].IsLocked()));
                break;
                }

            case 'N':
                {
                char Prompt[128];

                mPrintfCR(getmsg(248));

                sprintf(Prompt, getmsg(551), cfg.Lgroup_nym);

                AskGroupName(newname, GroupSlot, Prompt);

                if (*newname && (FindGroupByName(newname) != CERROR) &&
                        !SameString(newname, GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer))))
                    {
                    CRmPrintfCR(getsysmsg(185), newname, cfg.Lgroup_nym);
                    }
                else
                    {
                    if (*newname)
                        {
                        GroupData[GroupSlot].SetName(newname);
                        }
                    }

                break;
                }

            case 'S':
                {
                mPrintfCR(getmsg(60));

                if (getYesNo(getmsg(652), FALSE))
                    {
                    quit = TRUE;
                    }

                break;
                }

            case 'U':
                {
                char Prompt[128];

                mPrintfCR(getsysmsg(192));

                sprintf(Prompt, getsysmsg(193), cfg.Lgroup_nym);

                if (GroupSlot)
                    {
                    GroupData[GroupSlot].SetAutoAdd(getYesNo(Prompt, GroupData[GroupSlot].IsAutoAdd()));
                    }
                else
                    {
                    GroupData[GroupSlot].SetAutoAdd(TRUE);
                    CRmPrintfCR(getsysmsg(194), cfg.Lgroup_nym, GroupData[EVERYBODY].GetName(Buffer, sizeof(Buffer)));
                    }

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
                break;
                }
            }
        } while (!quit);

#ifdef WINCIT
    trap(T_SYSOP, WindowCaption, getsysmsg(195), groupname, GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)));
#else
    trap(T_SYSOP, getsysmsg(195), groupname, GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)));
#endif

    GroupData.Save();

    return (TRUE);
    }
