// -------------------------------------------------------------------------
// Citadel: ViewLog.CPP
//
// View log entries.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "group.h"
#include "hall.h"
#include "net.h"
#include "filecmd.h"
#include "term.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// showconfig()     displays user configuration
// SysopShowUser()  sysop fn to display any user's config.



// --------------------------------------------------------------------------
// ShowToggles(): showconfig() helper function.
//
// Notes:
//  Assumes MSG_VIEWLOG is locked.

#define NUMOPTS 30
void TERMWINDOWMEMBER ShowToggles(LogEntry1 *Log1)
    {
    termCap(TERM_BOLD);
    rmPrintf(getmsg(MSG_VIEWLOG, 1));
    termCap(TERM_NORMAL);

    doCR();

    const char *Options[NUMOPTS], *On[NUMOPTS], *Off[NUMOPTS];
    int numOn = 0, numOff = 0;

#ifdef WINCIT
    for (int CurrentOption = 0; CurrentOption < 28; CurrentOption++)
#else
    for (int CurrentOption = 0; CurrentOption < 26; CurrentOption++)
#endif
        {
        Options[CurrentOption] = getmsg(MSG_VIEWLOG, CurrentOption + 2);
        }

    if (!Log1->IsExpert())
        {
        On[numOn++] = Options[0];
        }
    else
        {
        Off[numOff++] = Options[0];
        }

    if (Log1->IsOldToo())
        {
        On[numOn++] = Options[1];
        }
    else
        {
        Off[numOff++] = Options[1];
        }

    if (Log1->IsAutoNextHall())
        {
        On[numOn++] = Options[2];
        }
    else
        {
        Off[numOff++] = Options[2];
        }

    if (Log1->IsViewRoomDesc())
        {
        On[numOn++] = Options[3];
        }
    else
        {
        Off[numOff++] = Options[3];
        }

    if (Log1->IsViewHallDescription())
        {
        On[numOn++] = Options[4];
        }
    else
        {
        Off[numOff++] = Options[4];
        }

    if (Log1->IsViewRoomInfoLines())
        {
        On[numOn++] = Options[5];
        }
    else
        {
        Off[numOff++] = Options[5];
        }

    if (cfg.surnames || cfg.netsurname || cfg.titles || cfg.nettitles)
        {
        if (Log1->IsViewTitleSurname())
            {
            On[numOn++] = Options[6];
            }
        else
            {
            Off[numOff++] = Options[6];
            }
        }

    if (Log1->IsViewSubjects())
        {
        On[numOn++] = Options[7];
        }
    else
        {
        Off[numOff++] = Options[7];
        }

    if (Log1->IsViewSignatures())
        {
        On[numOn++] = Options[8];
        }
    else
        {
        Off[numOff++] = Options[8];
        }

    if (Log1->IsAutoVerbose())
        {
        On[numOn++] = Options[9];
        }
    else
        {
        Off[numOff++] = Options[9];
        }

    if (Log1->IsTwirly())
        {
        On[numOn++] = Options[10];
        }
    else
        {
        Off[numOff++] = Options[10];
        }

    if (Log1->IsPauseBetweenMessages())
        {
        On[numOn++] = Options[11];
        }
    else
        {
        Off[numOff++] = Options[11];
        }

    if (Log1->IsClearScreenBetweenMessages())
        {
        On[numOn++] = Options[12];
        }
    else
        {
        Off[numOff++] = Options[12];
        }

    if (Log1->IsMinibin())
        {
        On[numOn++] = Options[13];
        }
    else
        {
        Off[numOff++] = Options[13];
        }

    if (Log1->IsVerboseContinue())
        {
        On[numOn++] = Options[14];
        }
    else
        {
        Off[numOff++] = Options[14];
        }

    if (cfg.censor)
        {
        if (Log1->IsViewCensoredMessages())
            {
            On[numOn++] = Options[15];
            }
        else
            {
            Off[numOff++] = Options[15];
            }
        }

    if (cfg.borders)
        {
        if (Log1->IsViewBorders())
            {
            On[numOn++] = Options[16];
            }
        else
            {
            Off[numOff++] = Options[16];
            }
        }

    if (!Log1->IsAutoVerbose())
        {
        if (Log1->IsVerboseLogOut())
            {
            On[numOn++] = Options[17];
            }
        else
            {
            Off[numOff++] = Options[17];
            }
        }

    if (Log1->IsYouAreHere())
        {
        On[numOn++] = Options[18];
        }
    else
        {
        Off[numOff++] = Options[18];
        }

    if (Log1->IsIBMRoom())
        {
        On[numOn++] = Options[19];
        }
    else
        {
        Off[numOff++] = Options[19];
        }

    if (Log1->IsWideRoom())
        {
        On[numOn++] = Options[20];
        }
    else
        {
        Off[numOff++] = Options[20];
        }


    if (Log1->IsViewCommas())
        {
        On[numOn++] = Options[21];
        }
    else
        {
        Off[numOff++] = Options[21];
        }

    if (Log1->IsPUnPauses())
        {
        On[numOn++] = Options[22];
        }
    else
        {
        Off[numOff++] = Options[22];
        }

    if((Log1->GetAutoIdleSeconds()) == -1)
        {
        Off[numOff++] = Options[23];
        }
    else
        {
        On[numOn++] = Options[23];
        }

    if(Log1->IsBunny())
        {
        On[numOn++] = Options[24];
        }
    else
        {
        Off[numOff++] = Options[24];
        }

    if(Log1->IsRoman())
        {
        On[numOn++] = Options[25];
        }


#ifdef WINCIT
    if(Log1->IsSeeOwnChats())
        {
        On[numOn++] = Options[26];
        }
    else
        {
        Off[numOff++] = Options[26];
        }

    if(Log1->IsErasePrompt())
        {
        On[numOn++] = Options[27];
        }
    else
        {
        Off[numOff++] = Options[27];
        }
#endif



    rmPrintf(getmsg(MSG_VIEWLOG, 35));

    prtList(LIST_START);

EXTRA_OPEN_BRACE
    for (int CurrentOption = 0; CurrentOption < numOn; ++CurrentOption)
        {
        char stupid_stuff[81];

        if (On[CurrentOption] == Options[1])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Lmsg_nym);
            }
        else if (On[CurrentOption] == Options[11] || On[CurrentOption] == Options[12] || On[CurrentOption] == Options[15])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Lmsgs_nym);
            }
        else if (On[CurrentOption] == Options[2])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Lhall_nym);
            }
        else if (On[CurrentOption] == Options[3] || On[CurrentOption] == Options[5])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Uroom_nym);
            }
        else if (On[CurrentOption] == Options[4])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Uhall_nym);
            }
        else if (On[CurrentOption] == Options[19])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Lroom_nym);
            }
        else if (On[CurrentOption] == Options[20])
            {
            sprintf(stupid_stuff, On[CurrentOption], cfg.Lrooms_nym);
            }
        else
            {
            strcpy(stupid_stuff, On[CurrentOption]);
            }

        prtList(stupid_stuff);
        }
EXTRA_CLOSE_BRACE
    prtList(LIST_END);

    rmPrintf(getmsg(MSG_VIEWLOG, 36));

    prtList(LIST_START);

EXTRA_OPEN_BRACE
    for (int CurrentOption = 0; CurrentOption < numOff; ++CurrentOption)
        {
        char stupid_stuff[81];

        if (Off[CurrentOption] == Options[1])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Lmsg_nym);
            }
        else if (Off[CurrentOption] == Options[11] || Off[CurrentOption] == Options[12] || Off[CurrentOption] == Options[15])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Lmsgs_nym);
            }
        else if (Off[CurrentOption] == Options[2])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Lhall_nym);
            }
        else if (Off[CurrentOption] == Options[3] || Off[CurrentOption] == Options[5])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Uroom_nym);
            }
        else if (Off[CurrentOption] == Options[4])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Uhall_nym);
            }
        else if (Off[CurrentOption] == Options[19])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Lroom_nym);
            }
        else if (Off[CurrentOption] == Options[20])
            {
            sprintf(stupid_stuff, Off[CurrentOption], cfg.Lrooms_nym);
            }
        else
            {
            strcpy(stupid_stuff, Off[CurrentOption]);
            }

        prtList(stupid_stuff);
        }
EXTRA_CLOSE_BRACE
    prtList(LIST_END);
    }

// --------------------------------------------------------------------------
// showconfig(): Displays user configuration.

void TERMWINDOWMEMBER showconfig(l_slot LogSlot, Bool showuser)
    {
    char Buffer[91];
    label Buffer1 = "";

    if (!LockMessages(MSG_VIEWLOG))
        {
        OutOfMemory(11);
        return;
        }

    g_slot CurrentGroup;
    label dProtocol;
    Bool in_a_group = FALSE;

    LogEntry1 *Log1;
    LogEntry2 *Log2;

    if (showuser)
        {
        Log1 = new LogEntry1;
        Log2 = new LogEntry2(cfg.maxgroups);

        // from .SS: load off disk.
        if (Log1 == NULL || !Log2->IsValid() || !Log1->Load(LTab(LogSlot).GetLogIndex()) || !Log2->Load(LTab(LogSlot).GetLogIndex()))
            {
            OutOfMemory(11);

            delete Log1;
            delete Log2;
            UnlockMessages(MSG_VIEWLOG);
            return;
            }
        }
    else
        {
        // from .RC: use our current stuff.
        Log1 = (LogEntry1 *)CurrentUser;
        Log2 = (LogEntry2 *)CurrentUser;

        doCR();
        }

    SetDoWhat(READCONFIG);

    OC.SetOutFlag(OUTOK);

    if (loggedIn || showuser)
        {
        doCR();
        termCap(TERM_BOLD);
        rmPrintf(getmsg(632), cfg.Uuser_nym);
        termCap(TERM_NORMAL);
        doCR();

        if (cfg.titles && Log1->IsViewTitleSurname() && *Log1->GetTitle(Buffer, sizeof(Buffer)))
            {
            rmPrintf(getmsg(MSG_VIEWLOG, 37), Buffer);
            }

        rmPrintf(getmsg(MSG_VIEWLOG, 38), Log1->GetName(Buffer, sizeof(Buffer)));

        if (cfg.surnames && Log1->IsViewTitleSurname() && *Log1->GetSurname(Buffer, sizeof(Buffer)))
            {
            rmPrintf(getmsg(93), Buffer);
            }

        if (*Log1->GetPhoneNumber(Buffer, sizeof(Buffer)) || *Log1->GetRealName(Buffer1, sizeof(Buffer1)))
            {
            doCR();

            if (*Buffer1)
                {
                rmPrintf(getmsg(MSG_VIEWLOG, 38), Buffer1);
                }
            else
                {
                if (*Log1->GetRealName(Buffer1, sizeof(Buffer1)))
                    {
                    rmPrintf(getmsg(MSG_VIEWLOG, 38), Buffer1);
                    }
                }

            if (*Buffer)
                {
                rmPrintf(*Buffer1 ? getmsg(MSG_VIEWLOG, 39) : getmsg(MSG_VIEWLOG, 38), Buffer);
                }
            }

        if (*Log1->GetMailAddr1(Buffer, sizeof(Buffer)))
            {
            doCR();
            rmPrintf(getmsg(MSG_VIEWLOG, 38), Buffer);
            }

        if (*Log1->GetMailAddr2(Buffer, sizeof(Buffer)))
            {
            doCR();
            rmPrintf(getmsg(MSG_VIEWLOG, 38), Buffer);
            }

        if (*Log1->GetMailAddr3(Buffer, sizeof(Buffer)))
            {
            doCR();
            rmPrintf(getmsg(MSG_VIEWLOG, 38), Buffer);
            }

        doCR();
        }

    OC.ResetOutParagraph();

    doCR();
    termCap(TERM_BOLD);
    rmPrintf(getmsg(MSG_VIEWLOG, 40));
    termCap(TERM_NORMAL);
    doCR();

    if (    Log1->IsUnlisted()      || Log1->IsSysop() ||
            Log1->IsAide()          || Log1->IsNetUser() ||
            Log1->IsNode()          || Log1->IsDungeoned() ||
            Log1->IsPermanent()     || Log1->IsBanned()    ||
            Log1->IsSuperSysop())
        {
        if (Log1->IsNode())         rmPrintf(getmsg(MSG_VIEWLOG, 41));
        if (Log1->IsAide())         rmPrintf(getmsg(MSG_VIEWLOG, 42));
        if (Log1->IsSysop())        rmPrintf(getmsg(MSG_VIEWLOG, 43));
        if (Log1->IsSuperSysop())   rmPrintf(getmsg(MSG_VIEWLOG, 81));
        if (Log1->IsDungeoned())    rmPrintf(getmsg(MSG_VIEWLOG, 44));
        if (Log1->IsNetUser())      rmPrintf(getmsg(MSG_VIEWLOG, 45));
        if (Log1->IsUnlisted())     rmPrintf(getmsg(MSG_VIEWLOG, 46));
        if (Log1->IsBanned())       rmPrintf(getmsg(MSG_VIEWLOG, 82));
        if (Log1->IsPermanent())    rmPrintf(getmsg(MSG_VIEWLOG, 47));
        doCR();
        }

    rmPrintf(getmsg(MSG_VIEWLOG, 48), cfg.Ugroups_nym);

    prtList(LIST_START);
    for (CurrentGroup = 0; CurrentGroup < cfg.maxgroups; ++CurrentGroup)
        {
        if (GroupData[CurrentGroup].IsInuse() && (Log2->IsInGroup(CurrentGroup) || Log1->IsMainSysop()))
            {
            prtList(GroupData[CurrentGroup].GetName(Buffer, sizeof(Buffer)));
            in_a_group = TRUE;
            }
        }
    prtList(LIST_END);

    if (!in_a_group)
        {
        doCR();
        }

    doCR();

    ShowToggles(Log1);

    if (*Log1->GetForwardAddr(Buffer, sizeof(Buffer)))
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 49), cfg.Lmsgs_nym);

        if (!Log1->IsForwardToNode())
            {
            if (FindPersonByName(Buffer) != CERROR)
                {
                rmPrintf(pcts, Buffer);
                }
            }
        else
            {
            rmPrintf(getmsg(101), Buffer, Log1->GetForwardAddrNode(Buffer1, sizeof(Buffer1)));
            }
        doCR();
        }


    label Buffer2;
    if((*Log1->GetAlias(Buffer, sizeof(Buffer))) != NULL)
        {
    rmPrintf(getmsg(MSG_VIEWLOG, 50), makeaddress(Log1->GetAlias(Buffer, sizeof(Buffer)),
        Log1->GetLocID(Buffer1, sizeof(Buffer1)), Buffer2));
    doCR();
        }

    if (*Log1->GetDefaultHall(Buffer, sizeof(Buffer)))
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 51), cfg.Lhall_nym);

        h_slot CurrentHall;
        for (CurrentHall = 0; CurrentHall < cfg.maxhalls; ++CurrentHall)
            {
            if (HallData[CurrentHall].IsInuse() && HallData[CurrentHall].IsSameName(Buffer) &&
                    CurrentUser->CanAccessHall(CurrentHall))
                {
                rmPrintf(pcts, Buffer);
                }
            }

        doCR();
        }

    if(Log1->IsUsePersonalHall())
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 80), cfg.Lhall_nym);
        doCR();
        }


    if (Log1->GetDefaultProtocol())
        {
        const protocols *theProt = GetProtocolByKey(Log1->GetDefaultProtocol());

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

    if (*dProtocol)
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 52), dProtocol);
        doCR();
        }

    rmPrintf(getmsg(MSG_VIEWLOG, 53), (loggedIn || showuser) ? Log1->GetNetPrefix(Buffer, sizeof(Buffer)) : cfg.netPrefix);
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 54), (loggedIn || showuser) ? Log1->GetPromptFormat(Buffer, sizeof(Buffer)) : cfg.prompt);
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 55), (loggedIn || showuser) ? Log1->GetDateStamp(Buffer, sizeof(Buffer)) : cfg.datestamp);
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 56), (loggedIn || showuser) ? Log1->GetVerboseDateStamp(Buffer, sizeof(Buffer)) : cfg.vdatestamp);
    doCR();

    if ((loggedIn || showuser) && *Log1->GetSignature(Buffer, sizeof(Buffer)))
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 57), Buffer);
        doCR();
        }

    OC.ResetOutParagraph();

    doCR();
    termCap(TERM_BOLD);
    rmPrintf(getmsg(MSG_VIEWLOG, 58));
    termCap(TERM_NORMAL);
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 59), Log1->GetTermType(Buffer, sizeof(Buffer)));
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 60));
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 61), ltoac(Log1->GetWidth()));


    if (Log1->IsUpperOnly())
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 62));
        }

    rmPrintf(getmsg(MSG_VIEWLOG, 63), Log1->IsLinefeeds() ? getmsg(MSG_VIEWLOG, 73) : getmsg(MSG_VIEWLOG, 74));

    if (Log1->GetLinesPerScreen())
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 64), ltoac(Log1->GetLinesPerScreen()));
        }

    if (Log1->IsMusic())
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 79));
        }


    if (Log1->GetNulls())
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 65), ltoac(Log1->GetNulls()));
        }

    rmPrintf(getmsg(MSG_VIEWLOG, 66), Log1->IsTabs() ? getmsg(MSG_VIEWLOG, 75) : getmsg(MSG_VIEWLOG, 76));
    doCR();

    rmPrintf(getmsg(MSG_VIEWLOG, 67), (loggedIn || showuser) ?
            Log1->GetMorePrompt(Buffer, sizeof(Buffer)) : cfg.moreprompt);
    doCR();

    if (cfg.accounting && Log1->IsAccounting())
        {
        long C = Log1->GetCredits() / 60;

        rmPrintf(getmsg(MSG_VIEWLOG, 68), ltoac(C), (C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
        doCR();
        }

    if (showuser || (MRO.Verbose && Log1->GetPoopcount()))
        {
        rmPrintf(getmsg(MSG_VIEWLOG, 69), ltoac(Log1->GetPoopcount()));
        doCR();
        }

    if (Log1->IsNode())
        {
        NodesCitC *nd = NULL;

        OC.ResetOutParagraph();

        doCR();

#ifdef WINCIT
        if (ReadNodesCit(this, &nd, Log1->GetName(Buffer, sizeof(Buffer)), NOMODCON, TRUE))
#else
        if (ReadNodesCit(&nd, Log1->GetName(Buffer, sizeof(Buffer)), NOMODCON, TRUE))
#endif

            {
            shownode(nd, MODEM);
            }
        else
            {
            rmPrintf(getmsg(MSG_VIEWLOG, 70));
            doCR();
            }

        freeNode(&nd);
        }

    if (showuser)
        {
        delete Log1;
        delete Log2;
        }
    UnlockMessages(MSG_VIEWLOG);
    }


// --------------------------------------------------------------------------
// SysopShowUser(): Sysop fn to display any user's config.

void TERMWINDOWMEMBER SysopShowUser(void)
    {
    SetDoWhat(SYSSHOW);

    label who;
    if (!AskUserName(who, loggedIn ? CurrentUser : NULL))
        {
        return;
        }

    const l_slot logno = FindPersonByPartialName(who);

    if (logno == CERROR)
        {
        CRmPrintfCR(getmsg(595), who);
        }
    else
        {
        showconfig(logno, TRUE);
        }
    }

void TERMWINDOWMEMBER ReadConfiguration(void)
    {
    showconfig(0, FALSE);
    }
