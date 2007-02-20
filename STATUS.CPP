// --------------------------------------------------------------------------
// Citadel: Status.CPP
//
// The .Read Status command and F4 function

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "account.h"
#include "hall.h"
#include "net.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// ReadStatus()     The .Read Status command


// --------------------------------------------------------------------------
// ReadStatus(): The .Read Status command.

void MessageDatC::CountMessageTypes(m_index *Public, m_index *Networked, m_index *Private, m_index *Moderated,
        m_index *Group, m_index *Copied, m_index *MassEMail, m_index *Censored, m_index *Local, m_index *Problem)
    {
    *Public     =
    *Networked  =
    *Private    =
    *Moderated  =
    *Group      =
    *Copied     =
    *MassEMail  =
    *Censored   =
    *Local      =
    *Problem    = 0;

    Lock();

    for (m_index ID = OldestMessageInTable(); ID <= NewestMessage(); ++ID)
        {
        if (IsNetworked(ID))    (*Networked)++;
        if (IsCopy(ID))         (*Copied)++;
        if (IsProblem(ID))      (*Problem)++;
        if (IsModerated(ID))    (*Moderated)++;
        if (IsCensored(ID))     (*Censored)++;
        if (IsLocal(ID))        (*Local)++;

        if (IsLimited(ID))
            {
            (*Group)++;
            }
        else if (IsMail(ID))
            {
            (*Private)++;
            }
        else if (IsMassemail(ID))
            {
            (*MassEMail)++;
            }
        else
            {
            (*Public)++;
            }
        }

    Unlock();
    }

void TERMWINDOWMEMBER ReadStatus(void)
    {
    if (!LockMenu(MENU_READ))
        {
        return;
        }

    l_slot i;

    Bool nat = FALSE;

    char dtstr[80];
    int publicRoom, active, directory, shared, hidden, anon, group, problem, perm, aides, sysops, nodes, moderated, bio;


    SetDoWhat(READSTATUS);

    OC.SetOutFlag(OUTOK);

    // On...
    doCR(2);
    rmPrintf(getmenumsg(MENU_READ, 42), cfg.nodeTitle, cfg.nodeRegion, cfg.nodeCountry, cfg.alias, cfg.locID);
    doCR();

    rmPrintf(getmenumsg(MENU_READ, 43), cfg.softverb, *cfg.softverb ? spc : ns, programName, version);
    doCR();

    rmPrintf(getmenumsg(MENU_READ, 44), Author);
    doCR();

    if (MRO.Verbose)
        {
        rmPrintf(getmenumsg(MENU_READ, 45), cmpDate, cmpTime);
        doCR();
        }

    if (MRO.PauseBetween)
        {
        nat = TRUE;
        }

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;
        if(OC.User.GetOutFlag() == OUTSKIP)
            {
            UnlockMenu(MENU_READ);
            return;
            }
        else
            {
            OC.SetOutFlag(OUTOK);
            }
        }


    // times..
    OC.ResetOutParagraph();

    char Buffer[64];
    strftime(dtstr, 79, CurrentUser->GetVerboseDateStamp(Buffer, sizeof(Buffer)), 0l);
    doCR();
    rmPrintf(getmenumsg(MENU_READ, 46), (loggedIn) ? getmsg(342) : MRO.Verbose ? spc : ns, dtstr);
    doCR();

    if (MRO.Verbose)
        {
        rmPrintf(getmenumsg(MENU_READ, 47), loggedIn ? getmsg(311) : ns, diffstamp(uptimestamp));
        doCR();
        }

    if (CommPort->HaveConnection())
        {
        rmPrintf(getmenumsg(MENU_READ, 48), loggedIn ? getmsg(311) : ns, diffstamp(conntimestamp));

        if (loggedIn)
            {
            rmPrintf(getmenumsg(MENU_READ, 49), diffstamp(logtimestamp));
            }

        doCR();
        }
    else
        {
        if (loggedIn)
            {
            rmPrintf(getmenumsg(MENU_READ, 50), diffstamp(logtimestamp));
            doCR();
            }
        }

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;
        if(OC.User.GetOutFlag() == OUTSKIP)
            {
            UnlockMenu(MENU_READ);
            return;
            }
        else
            {
            OC.SetOutFlag(OUTOK);
            }
        }



    // Userlog info.
    OC.ResetOutParagraph();

    doCR();
    rmPrintf(getmenumsg(MENU_READ, 51), cfg.Uuser_nym);

    active = aides = sysops = problem = perm = nodes = 0;

    for (i = 0; i < cfg.MAXLOGTAB; i++)
        {
        if (LTab(i).IsInuse())
            {
            active++;
            if (LTab(i).IsAide())       aides++;
            if (LTab(i).IsSysop())      sysops++;
            if (LTab(i).IsProblem())    problem++;
            if (LTab(i).IsPermanent())  perm++;
            if (LTab(i).IsNode())       nodes++;
            }
        }

    doCR();
    label MaxLogTab, ActiveLogTab;
    CopyStringToBuffer(MaxLogTab, ltoac(cfg.MAXLOGTAB));
    CopyStringToBuffer(ActiveLogTab, ltoac(active));

    rmPrintf(getmenumsg(MENU_READ, 52), MaxLogTab, ActiveLogTab, ltoac(cfg.callno));
    doCR();

    if (MRO.Verbose)
        {
        rmPrintf(getmenumsg(MENU_READ, 53), ltoac(nodes), (nodes == 1) ? ns : justs);

        if (CurrentUser->IsAide())
            {
            label Aides, Sysops, Twits;
            CopyStringToBuffer(Aides, ltoac(aides));
            CopyStringToBuffer(Sysops, ltoac(sysops));
            CopyStringToBuffer(Twits, ltoac(problem));

            rmPrintf(getmenumsg(MENU_READ, 54), Aides, (aides == 1) ? ns : justs, Sysops, (sysops == 1) ? ns : justs,
                    Twits, (problem == 1) ? cfg.Luser_nym : cfg.Lusers_nym, ltoac(perm),
                    (perm == 1) ? cfg.Luser_nym : cfg.Lusers_nym);
            }

        doCR();
        rmPrintf(getmenumsg(MENU_READ, 55), cfg.Luser_nym, cfg.poopuser, ltoac(cfg.maxpoop));
        doCR();
        }

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;

        if(OC.User.GetOutFlag() == OUTSKIP)
            {
            UnlockMenu(MENU_READ);
            return;
            }
        else
            {
            OC.SetOutFlag(OUTOK);
            }
        }

    // Group info
    OC.ResetOutParagraph();

    doCR();
    rmPrintf(getmenumsg(MENU_READ, 56), cfg.Ugroup_nym);

    for (active = 0, anon = 0, hidden = 0, nodes = 0, i = 0; i < cfg.maxgroups; ++i)
        {
        if (GroupData[i].IsInuse())
            {
            active++;

            if (GroupData[i].IsAutoAdd())
                {
                anon++;
                }

            if (GroupData[i].IsHidden())
                {
                hidden++;
                }

            if (GroupData[i].IsLocked())
                {
                nodes++;
                }
            }
        }
    doCR();
    label MaxGroups;
    CopyStringToBuffer(MaxGroups, ltoac(cfg.maxgroups));

    rmPrintf(getmenumsg(MENU_READ, 57), MaxGroups, cfg.Lgroups_nym, ltoac(active));

    if (MRO.Verbose)
        {
        label AutoAdd, Hidden;
        CopyStringToBuffer(AutoAdd, ltoac(anon));
        CopyStringToBuffer(Hidden, ltoac(hidden));

        rmPrintf(getmenumsg(MENU_READ, 58), AutoAdd, Hidden, ltoac(nodes));
        }
    doCR();

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;
        if(OC.User.GetOutFlag() == OUTSKIP)
            {
            UnlockMenu(MENU_READ);
            return;
            }
        else
            {
            OC.SetOutFlag(OUTOK);
            }
        }

    // Hall info
    OC.ResetOutParagraph();

    doCR();
    rmPrintf(getmenumsg(MENU_READ, 56), cfg.Uhall_nym);

    for (anon = 0, group = 0, active = 0, i = 0; i < cfg.maxhalls; ++i)
        {
        if (HallData[i].IsInuse())
            {
            active++;

            if (HallData[i].IsOwned())
                {
                group++;
                }

            if (HallData[i].IsEnterRoom())
                {
                anon++;
                }
            }
        }

    doCR();
    label MaxHalls;
    CopyStringToBuffer(MaxHalls, ltoac(cfg.maxhalls));

    rmPrintf(getmenumsg(MENU_READ, 57), MaxHalls, cfg.Lhalls_nym, ltoac(active));

    if (MRO.Verbose)
        {
        label Group;
        CopyStringToBuffer(Group, ltoac(group));

        rmPrintf(getmenumsg(MENU_READ, 60), Group, cfg.Lgroup_nym, ltoac(anon), cfg.Lroom_nym);
        }
    doCR();

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;
        if(OC.User.GetOutFlag() == OUTSKIP)
            {
            UnlockMenu(MENU_READ);
            return;
            }
        else
            {
            OC.SetOutFlag(OUTOK);
            }
        }

    // Room info
    OC.ResetOutParagraph();

    doCR();
    rmPrintf(getmenumsg(MENU_READ, 56), cfg.Uroom_nym);

    anon = active = publicRoom = hidden = directory = group = shared = moderated = bio = 0;

    for (i = 0; i < cfg.maxrooms; i++)
        {
        if (RoomTab[i].IsInuse())
            {
            active++;
            if (!RoomTab[i].IsHidden())
                {
                if (!RoomTab[i].IsGroupOnly() && !RoomTab[i].IsBIO())
                    {
                    publicRoom++;
                    }
                }
            else
                {
                hidden++;
                }

            if (RoomTab[i].IsBIO())
                {
                bio++;
                }

            if (RoomTab[i].IsMsDOSdir())
                {
                directory++;
                }

            if (RoomTab[i].IsGroupOnly())
                {
                group++;
                }

            if (RoomTab[i].IsShared())
                {
                shared++;
                }

            if (RoomTab[i].IsModerated())
                {
                moderated++;
                }

            if (RoomTab[i].IsAnonymous())
                {
                anon++;
                }
            }
        }

    doCR();
    label MaxRooms;
    CopyStringToBuffer(MaxRooms, ltoac(cfg.maxrooms));
    rmPrintf(getmenumsg(MENU_READ, 57), MaxRooms, cfg.Lrooms_nym, ltoac(active));
    doCR();

    if (MRO.Verbose)
        {
        label Public, Hidden, Bio, Shared, Directory, Moderated, Group;
        CopyStringToBuffer(Public, ltoac(publicRoom));
        CopyStringToBuffer(Hidden, ltoac(hidden));
        CopyStringToBuffer(Bio, ltoac(bio));
        CopyStringToBuffer(Shared, ltoac(shared));
        CopyStringToBuffer(Directory, ltoac(directory));
        CopyStringToBuffer(Moderated, ltoac(moderated));
        CopyStringToBuffer(Group, ltoac(group));

        rmPrintf(getmenumsg(MENU_READ, 62), Public, Hidden, Bio, Shared, Directory, Moderated, Group, cfg.Lgroup_nym,
                ltoac(anon));
        doCR();
        }

    if(nat)
        {
        if(OC.User.GetOutFlag() == OUTOK)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            }
        numLines = 0;
        }

    // Message status
    OC.ResetOutParagraph();

    doCR();
    rmPrintf(getmenumsg(MENU_READ, 56), cfg.Umsg_nym);
    doCR();

    // stop before the message count if already aborted...
    if (((OC.User.GetOutFlag() != OUTOK) && !(nat)) || ((nat) &&
          (OC.User.GetOutFlag() == OUTSKIP)))
        {
        UnlockMenu(MENU_READ);
        return;
        }
    else
        {
        OC.SetOutFlag(OUTOK);
        }

    label Newest, Oldest;
    CopyStringToBuffer(Newest, ltoac(MessageDat.NewestMessage()));
    CopyStringToBuffer(Oldest, ltoac(MessageDat.OldestMessageInFile()));

    rmPrintf(getmenumsg(MENU_READ, 63), ltoac(MessageDat.NewestMessage() - MessageDat.OldestMessageInFile() + 1),
             Oldest, Newest);
    doCR();

    if (MessageDat.OldestMessageInTable() > MessageDat.OldestMessageInFile() && CurrentUser->IsAide())
        {
        rmPrintf(getmenumsg(MENU_READ, 64), ltoac(MessageDat.OldestMessageInTable() - MessageDat.OldestMessageInFile()));

        if (MRO.Verbose)
            {
            rmPrintf(getmenumsg(MENU_READ, 65));
            }
        else
            {
            doCR();
            }
        }

    if (MRO.Verbose)
        {
        m_slot public_mess, networked_mess, private_mess, moderated_mess, group_mess, copies_mess, problem_mess,
                massemail_mess, local_mess, censored_mess;

        MessageDat.CountMessageTypes(&public_mess, &networked_mess, &private_mess, &moderated_mess, &group_mess,
                &copies_mess, &problem_mess, &massemail_mess, &local_mess, &censored_mess);

        label Public, Private, Networked, Local, Moderated, Censored;

        CopyStringToBuffer(Public, ltoac(public_mess));
        CopyStringToBuffer(Private, ltoac(private_mess));
        CopyStringToBuffer(Networked, ltoac(networked_mess));
        CopyStringToBuffer(Local, ltoac(local_mess));
        CopyStringToBuffer(Moderated, ltoac(moderated_mess));
        CopyStringToBuffer(Censored, ltoac(censored_mess));

        rmPrintf(getmenumsg(MENU_READ, 66), Public, Private, Networked, Local, Moderated, Censored, ltoac(massemail_mess));

        if (!CurrentUser->IsAide())
            {
            rmPrintf(getmenumsg(MENU_READ, 67));
            }

        rmPrintf(getmenumsg(MENU_READ, 68), ltoac(group_mess), cfg.Lgroup_nym);

        if (CurrentUser->IsAide())
            {
            label Copies;
            CopyStringToBuffer(Copies, ltoac(copies_mess));

            rmPrintf(getmenumsg(MENU_READ, 69), Copies, ltoac(problem_mess), cfg.Luser_nym);
            }

        doCR();

        if (cfg.MsgDatSizeInK > 10*1024)
            {
            long Megabytes = cfg.MsgDatSizeInK / 1024;

            long Thousandths = ((cfg.MsgDatSizeInK % 1024) * 1000) / 1024;

            // Here we are rouding to the nearest hundredth.
            if (Thousandths % 10 > 4)
                {
                Thousandths += 10;
                }

            if (Thousandths > 1000)
                {
                Megabytes++;
                }

            // Now this is really hundredths.
            label MessageM;

            sprintf(MessageM, getmenumsg(MENU_READ, 70), ltoac(Megabytes), Thousandths / 10);

            rmPrintf(getmenumsg(MENU_READ, 71), MessageM, 'M', cfg.Lmsg_nym, ltoac(cfg.nmessages), cfg.Lmsg_nym);
            }
        else
            {
            label MessageK;

            CopyStringToBuffer(MessageK, ltoac(cfg.MsgDatSizeInK));

            rmPrintf(getmenumsg(MENU_READ, 71), MessageK, 'K', cfg.Lmsg_nym, ltoac(cfg.nmessages), cfg.Lmsg_nym);
            }

        if (MessageDat.GetPercentFull() < 100)
            {
            rmPrintf(getmenumsg(MENU_READ, 72), cfg.Lmsg_nym, MessageDat.GetPercentFull());
            }

        doCR();
        }

    rmPrintf(getmenumsg(MENU_READ, 73), ltoac(MessageDat.GetAverageMessageLength()), cfg.Lmsg_nym);
    doCR();

    if (MRO.Verbose)
        {
        label Entered;
        CopyStringToBuffer(Entered, ltoac(MS.Entered));

        rmPrintf(getmenumsg(MENU_READ, 74), Entered, ltoac(MS.Read), (loggedIn) ? ns : getmenumsg(MENU_READ, 75));
        doCR();
        if(nat)
            {
#ifdef MULTI
            OC.CheckInput(TRUE, this);
#else
            OC.CheckInput(TRUE);
#endif
            numLines = 0;
            }
        }

    // System and debugging
    OC.ResetOutParagraph();

    if (MRO.Verbose)
        {
        doCR();
        rmPrintf(getmenumsg(MENU_READ, 76));
        doCR();

#ifndef WINCIT
        if (OSType == OS_OS2)
            {
            rmPrintf(getmenumsg(MENU_READ, 77), OSMajor, OSMinor);
            doCR();
            }
        else
            {
            rmPrintf(getmenumsg(MENU_READ, 78), _osmajor, _osminor);
            doCR();

            switch (OSType)
                {
                case OS_DV:
                    {
                    rmPrintf(getmenumsg(MENU_READ, 79), OSMajor, OSMinor);
                    doCR();
                    break;
                    }

                case OS_WINS:
                case OS_WIN3:
                    {
                    rmPrintf(getmenumsg(MENU_READ, 80), OSMajor, OSMinor,
                            OSType == OS_WINS ? getmenumsg(MENU_READ, 81) : getmenumsg(MENU_READ, 82));
                    doCR();
                    break;
                    }
                }
            }
#else
        OSVERSIONINFO ovi;
        ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&ovi))
            {
            rmPrintf(getmenumsg(MENU_READ, 103),
                    ovi.dwPlatformId == VER_PLATFORM_WIN32_NT ? getmenumsg(MENU_READ, 104) :
                    ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ? getmenumsg(MENU_READ, 105) :
                    getmenumsg(MENU_READ, 106), ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber);

            if (*ovi.szCSDVersion)
                {
                rmPrintf(spcts, ovi.szCSDVersion);
                }
            }
        else
            {
            rmPrintf(getmenumsg(MENU_READ, 83), GetVersion() & 255, (GetVersion() >> 8) & 255);
            }
        doCR();
#endif

        Bool CanShowAllDebugInfo = TRUE;

        switch (abs(cfg.ReadStatusSecurity))
            {
            case 1: CanShowAllDebugInfo = CurrentUser->IsAide();        break;
            case 2: CanShowAllDebugInfo = CurrentUser->IsSysop();       break;
            case 3: CanShowAllDebugInfo = CurrentUser->IsSuperSysop();  break;
            case 4: CanShowAllDebugInfo = onConsole;                    break;
            }

        if (cfg.ReadStatusSecurity < 0)
            {
            CanShowAllDebugInfo = !CanShowAllDebugInfo;
            }

        if (CanShowAllDebugInfo)
            {
#ifndef WINCIT
            if (*vdDesc)
                {
                rmPrintf(getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 85), vdDesc);
                doCR();
                }

            if (*cdDesc)
                {
                rmPrintf(getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 86), cdDesc);
                doCR();
                }

            if (*kdDesc)
                {
                rmPrintf(getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 87), kdDesc);
                doCR();
                }

            if (*sdDesc)
                {
                rmPrintf(getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 88), sdDesc);
                doCR();
                }

            if (*upsDesc)
                {
                rmPrintf(getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 109), upsDesc);
                doCR();
                }
#endif

//////////////////////////////////////////////////////////////
#include "config.h"     // Just for fileHandlesLeft
            rmPrintf(getmenumsg(MENU_READ, 89), fileHandlesLeft());
            doCR();
//////////////////////////////////////////////////////////////

#ifdef WINCIT
            MEMORYSTATUS ms;
            ms.dwLength = sizeof(ms);
            GlobalMemoryStatus(&ms);

            label Avail;
            CopyStringToBuffer(Avail, ltoac(ms.dwAvailPhys / 1024));

            rmPrintf(getmenumsg(MENU_READ, 92), ltoac(ms.dwTotalPhys / 1024), Avail);
            doCR();

            CopyStringToBuffer(Avail, ltoac(ms.dwAvailPageFile / 1024));

            rmPrintf(getmenumsg(MENU_READ, 93), ltoac(ms.dwTotalPageFile / 1024), Avail);
            doCR();

            CopyStringToBuffer(Avail, ltoac(ms.dwAvailVirtual / 1024));

            rmPrintf(getmenumsg(MENU_READ, 107), ltoac(ms.dwTotalVirtual / 1024), Avail);
            doCR();

            rmPrintf(getmenumsg(MENU_READ, 108), ms.dwMemoryLoad);
            doCR();
#else
#ifdef MSC
            union REGS r;
            r.h.ah = 0x48;   // allocate memory
            r.h.al = 0;
            r.x.bx = 0xffff; // ask for an impossible amount

            intdos(&r, &r);

            rmPrintf(getmenumsg(MENU_READ, 90), _bios_memsize());
            rmPrintf(getmenumsg(MENU_READ, 91), ltoac((long)r.x.bx*16L));
            doCR();
#else
            rmPrintf(getmenumsg(MENU_READ, 90), biosmemory());
            rmPrintf(getmenumsg(MENU_READ, 91), ltoac(farcoreleft()));
            doCR();
#endif
#endif

            if (*cmdLine)
                {
                rmPrintf(getmenumsg(MENU_READ, 94), cmdLine);
                doCR();
                }
            }

        CopyStringToBuffer(dtstr, ltoac(CommPort->GetReceived()));
        rmPrintf(getmenumsg(MENU_READ, 97), ltoac(CommPort->GetTransmitted()), dtstr);
        doCR();

#ifdef WINCIT
        switch (CommPort->GetType())
            {
            case CT_SERIAL:
                {
                rmPrintf(getmenumsg(MENU_READ, 111), getmenumsg(MENU_READ, 110));
                doCR();

                rmPrintf(getmenumsg(MENU_READ, 98), connectbauds[CommPort->GetModemSpeed()], bauds[CommPort->GetSpeed()],
                        CommPort->UsingCompression ? ns : getmenumsg(MENU_READ, 99),
                        CommPort->UsingCorrection ? ns : getmenumsg(MENU_READ, 99));
                break;
                }

            case CT_LOCAL:
                {
                rmPrintf(getmenumsg(MENU_READ, 112));
                break;
                }

            case CT_TELNET:
                {
                rmPrintf(getmenumsg(MENU_READ, 111), getmenumsg(MENU_READ, 113));
                doCR();

                label TSpeed, RSpeed;
                if (CommPort->GetTransmitSpeed())
                    {
                    CopyStringToBuffer(TSpeed, ltoac(CommPort->GetTransmitSpeed()));
                    }
                else
                    {
                    CopyStringToBuffer(TSpeed, getmenumsg(MENU_READ, 114));
                    }

                if (CommPort->GetReceiveSpeed())
                    {
                    CopyStringToBuffer(RSpeed, ltoac(CommPort->GetReceiveSpeed()));
                    }
                else
                    {
                    CopyStringToBuffer(RSpeed, getmenumsg(MENU_READ, 114));
                    }

                rmPrintf(getmenumsg(MENU_READ, 115), TSpeed, RSpeed);
                break;
                }

            default:
                {
                rmPrintf(getmenumsg(MENU_READ, 111), getmenumsg(MENU_READ, 116));
                break;
                }
            }
#else
        if (CommPort->HaveConnection())
            {
            rmPrintf(getmenumsg(MENU_READ, 98), connectbauds[CommPort->GetModemSpeed()], bauds[CommPort->GetSpeed()],
                    CommPort->UsingCompression ? ns : getmenumsg(MENU_READ, 99),
                    CommPort->UsingCorrection ? ns : getmenumsg(MENU_READ, 99));
            }
        else
            {
            rmPrintf(getmenumsg(MENU_READ, 100), bauds[CommPort->GetSpeed()]);
            }
#endif
        doCR();

        const int ourtimeout = (loggedIn) ? ((onConsole) ? cfg.consoletimeout : cfg.timeout) : cfg.unlogtimeout;

        rmPrintf(getmenumsg(MENU_READ, 101), ltoac(ourtimeout), ourtimeout == 1 ? ns : justs);
        doCR();
        }

    UnlockMenu(MENU_READ);
    }

#ifndef WINCIT
// i am not sure if this is the best place for this function,
// overlay-wise. i want to keep it in an overlay because it will not
// be used often. But, it will be called often when it is in use, so i
// want to put it in a small or well used overlay. whatever.
// gee, how about its own overlay?!?
// No, that would be too smart.

#define DOWHATOFFSET 40

void showStatScreen(void)
    {
    char buf[100], buf2[20];
    int i, j;

    const char **dowhatmsgs = ((const char **) ExtMessages[MSG_F4SCREEN].Data->aux) + DOWHATOFFSET;

    sprintf(buf, getmsg(472), programName, version);
    statDisp(0, 1, cfg.wattr, buf);
    statDisp(1, 1, cfg.wattr, getmsg(MSG_F4SCREEN, 0));

    strftime(buf, 30, getmsg(MSG_F4SCREEN, 1), 0l);
    statDisp(0, 32, cfg.attr, buf);

    sprintf(buf, getmsg(MSG_F4SCREEN, 2), diffstamp(uptimestamp));
    statDisp(0, 54, cfg.attr, buf);

    label Buffer;
    sprintf(buf, getmsg(MSG_F4SCREEN, 3), deansi(cfg.Uuser_nym),
            loggedIn ? CurrentUser->GetName(Buffer, sizeof(Buffer)) : getmsg(100));
    statDisp(3, 1, cfg.attr, buf);

    sprintf(buf, getmsg(MSG_F4SCREEN, 4), dowhatmsgs[PDoWhat]);
    statDisp(3, 40, cfg.attr, buf);

    if (loggedIn)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 5), RoomTab[thisRoom].IsInuse() ?
                deansi(RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer))) : getmsg(MSG_F4SCREEN, 6));
        statDisp(4, 1, cfg.attr, buf);
        sprintf(buf, getmsg(MSG_F4SCREEN, 7), diffstamp(logtimestamp));
        statDisp(4, 40, cfg.attr, buf);
        }
    else
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 8), RoomTab[thisRoom].IsInuse() ?
                deansi(RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer))) : getmsg(MSG_F4SCREEN, 6));
        statDisp(4, 1, cfg.attr, buf);
        }

    if (loggedIn && cfg.accounting && CurrentUser->IsAccounting())
        {
        if (CurrentUserAccount->IsSpecialTime())
            {
            statDisp(5, 40, cfg.attr, getmsg(MSG_F4SCREEN, 9));
            }
        else
            {
            const long C = CurrentUser->GetCredits() / 60;

            sprintf(buf, getmsg(MSG_F4SCREEN, 10), deansi((C == 1) ? cfg.Ucredit_nym : cfg.Ucredits_nym), C);
            statDisp(5, 40, cfg.attr, buf);
            }

        sprintf(buf, getmsg(MSG_F4SCREEN, 3), cfg.Uhall_nym, deansi(HallData[thisHall].GetName(Buffer, sizeof(Buffer))));
        }
    else
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 11), cfg.Uhall_nym, deansi((CurrentUser->IsUsePersonalHall()) ?
                getmsg(MSG_F4SCREEN, 12) : HallData[thisHall].GetName(Buffer, sizeof(Buffer))));
        }
    statDisp(5, 1, cfg.attr, buf);

    if (loggedIn)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 13), getmsg(MSG_F4SCREEN, 14), CommPort->GetReceived(), getmsg(MSG_F4SCREEN, 15),
                MS.Entered, getmsg(MSG_F4SCREEN, 16), CurrentUser->GetPoopcount());
        }
    else
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 17), getmsg(MSG_F4SCREEN, 14), CommPort->GetReceived(), getmsg(MSG_F4SCREEN, 14),
                MS.Entered);
        }
    statDisp(7, 1, cfg.attr, buf);

    sprintf(buf, getmsg(MSG_F4SCREEN, 18), getmsg(MSG_F4SCREEN, 19),
            CommPort->GetTransmitted(), getmsg(MSG_F4SCREEN, 20), MS.Read);

    statDisp(8, 1, cfg.attr, buf);

    sprintf(buf, getmsg(MSG_F4SCREEN, 21), ltoac(farcoreleft()));
    statDisp(10, 1, cfg.attr, buf);

#ifdef AUXMEM
    i = 22;
    if (xmssize)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 22), getmsg(MSG_F4SCREEN, 23), ltoac(xmssize));
        statDisp(10, i, cfg.attr, buf);
        i += 19;
        }

    if (emssize)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 22), getmsg(MSG_F4SCREEN, 24), ltoac(emssize * (AUXPAGESIZE / 1024)));
        statDisp(10, i, cfg.attr, buf);
        i += 19;
        }

    if (vrtpags)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 22), getmsg(MSG_F4SCREEN, 25), ltoac(vrtpags * (AUXPAGESIZE / 1024)));
        statDisp(10, i, cfg.attr, buf);
        }
#endif

    if (CommPort->HaveConnection())
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 26), connectbauds[CommPort->GetModemSpeed()], bauds[CommPort->GetSpeed()]);
        if (CommPort->UsingCorrection)
            {
            strcat(buf, getmsg(MSG_F4SCREEN, 27));
            }
        if (CommPort->UsingCompression)
            {
            strcat(buf, getmsg(MSG_F4SCREEN, 28));
            }
        }
    else
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 29), bauds[CommPort->GetSpeed()]);
        }

    statDisp(11, 1, cfg.attr, buf);

    if (updateStatus)
        {
        sprintf(buf, getmsg(MSG_F4SCREEN, 30), getmsg(MSG_F4SCREEN, 31), getmsg(MSG_F4SCREEN, 32));
        statDisp(13, 1, cfg.attr, buf);

        for (i = 0, j = sl_end; j != sl_start && i < scrollpos - 13; i++)
            {
            j--;
            if (j < 0)
                {
                j = cfg.statnum - 1;
                }
            }

        for (i = 14; j != sl_end; j = ++j % cfg.statnum)
            {
            strftime(buf2, 19, getmsg(MSG_F4SCREEN, 33), statList[j].theTime);
            sprintf(buf, getmsg(MSG_F4SCREEN, 30), deansi(statList[j].theStatus), buf2);
            statDisp(i++, 1, cfg.attr, buf);
            }

        updateStatus = FALSE;
        }
    }
#endif
