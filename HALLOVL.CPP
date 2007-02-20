// --------------------------------------------------------------------------
// Citadel: HallOvl.CPP
//
// Overlayed hall code for Citadel bulletin board system

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "hall.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// force()              sysop special to force access into a hall
// listhalls()          sysop special to list all halls
// accesshall()         returns TRUE if person can access hall
// EnterHall()          handles .EH
// knownhalls()         handles .kh, .kvh
// ReadHalls()          handles .rh, .rvh
// stephall()           handles previous, next hall


// --------------------------------------------------------------------------
// force(): Sysop special to force access into a hall.

void TERMWINDOWMEMBER force(void)
    {
    label HallName;

    doCR();

    if (!AskHallName(HallName, 1))  // Maintenance
        {
        return;
        }

    const h_slot slot = partialhall(HallName);

    if (slot == CERROR)
        {
        CRmPrintfCR(getmsg(157), cfg.Lhall_nym, HallName);
        return;
        }

    thisHall = slot;
	HallData[thisHall].GetName(HallName, sizeof(HallName));

    CRmPrintfCR(getsysmsg(356), cfg.Lhall_nym, HallName);

#ifdef WINCIT
    trap(T_SYSOP, WindowCaption, getsysmsg(321), HallName);
#else
    trap(T_SYSOP, getsysmsg(321), HallName);
#endif

    }


// --------------------------------------------------------------------------
// listhalls(): List all halls, or just those windowed off of room.

void TERMWINDOWMEMBER ListHalls(Bool WindowedOffRoom)
    {
    CRmPrintfCR(getmsg(578), cfg.Uhalls_nym);

    prtList(LIST_START);

    for (h_slot HallSlot = 0; HallSlot < cfg.maxhalls; HallSlot++)
        {
        if (HallData[HallSlot].IsInuse() && CurrentUser->CanAccessHall(HallSlot) &&
                (!WindowedOffRoom || (HallData[HallSlot].IsWindowedIntoHall(thisRoom) || cfg.subhubs == 4)))
            {
            label Buffer;
            prtList(HallData[HallSlot].GetName(Buffer, sizeof(Buffer)));
            }
        }

    prtList(LIST_END);
    }


// --------------------------------------------------------------------------
// accesshall(): Returns true if hall can be accessed from current room.

Bool TERMWINDOWMEMBER accesshall(h_slot slot)
    {
    return (    // always list this hall
            (slot == thisHall) ||

            // if not this hall, check to see if it is in use
            (HallData[slot].IsInuse() &&

            // and this room is windowed into the hall
            (HallData[slot].IsWindowedIntoHall(thisRoom) ||
            cfg.subhubs == 4) &&

            // and we have group-access to the hall
            CurrentUser->CanAccessHall(slot)));
    }


// --------------------------------------------------------------------------
// EnterHall(): Handles .EH.

void TERMWINDOWMEMBER EnterHall(void)
    {
    label hallname;
    h_slot slot;
    int accessible;

    SetDoWhat(ENTERHALL);

    doCR();
    if (!AskHallName(hallname, ns, TRUE))
        {
        return;
        }

    slot = partialhall(hallname);

    if (slot != CERROR)
        {
        accessible = accesshall(slot);
        }

    if ((slot == CERROR) || !(*hallname) || !accessible)
        {
        CRmPrintfCR(getmsg(611), cfg.Lhall_nym);
        return;
        }
    else
        {
        thisHall = slot;
        }
    }


// --------------------------------------------------------------------------
// knownhalls(): Handles .kh .kvh.

void TERMWINDOWMEMBER knownhalls(void)
    {
    SetDoWhat(KNOWNHALLS);

    CRmPrintfCR(getmsg(476), cfg.Uhalls_nym);

    prtList(LIST_START);

    for (h_slot i = 0; i < cfg.maxhalls; i++)
        {
        if (accesshall(i))
            {
            label Buffer;
            prtList(HallData[i].GetName(Buffer, sizeof(Buffer)));
            }
        }

    prtList(LIST_END);
    }


// --------------------------------------------------------------------------
// ReadHalls(): Handles .r[v]h

void TERMWINDOWMEMBER ReadHalls(void)
    {
    char str[LABELSIZE + LABELSIZE];

    SetDoWhat(READHALLS);

    CRCRmPrintfCR(getmsg(448), cfg.Uroom_nym, MakeRoomName(thisRoom, str));

    prtList(LIST_START);

    for (h_slot i = 0; i < cfg.maxhalls; i++)
        {
        if (HallData[i].IsInuse() && HallData[i].IsRoomInHall(thisRoom) && CurrentUser->CanAccessHall(i))
            {
            label Buffer;
            prtList(HallData[i].GetName(Buffer, sizeof(Buffer)));
            }
        }

    prtList(LIST_END);
    }


// --------------------------------------------------------------------------
// roomonlyinthishall(): Returns TRUE if room# is in hall and no others.

Bool roomonlyinthishall(r_slot roomslot, h_slot hallslot)
    {
    h_slot hall;

    if (!roominhall(roomslot, hallslot))
        {
        return (FALSE);
        }

    for (hall = 0; hall < cfg.maxhalls; hall++)
        {
        if ((hall != hallslot) && HallData[hall].IsRoomInHall(roomslot))
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }


// --------------------------------------------------------------------------
// stephall(): Handles previous, next hall.

void TERMWINDOWMEMBER stephall(int direction)
    {
    label Buffer;

    if (CurrentUser->IsUsePersonalHall())
        {
        CRCRmPrintfCR(getmsg(477), cfg.Lhall_nym);
        return;
        }

    h_slot i;
    int done = FALSE;

    i = thisHall;

    do
        {
        // step
        if (direction == 1)
            {
            if (++i == cfg.maxhalls)
                {
                i = 0;
                }
            }
        else
            {
            if (--i == -1)
                {
                i = (short) (cfg.maxhalls - 1);
                }
            }

        // keep from looping endlessly
        if (i == thisHall)
            {
            CRCRmPrintfCR(getmsg(445), cfg.Lroom_nym);
            return;
            }

        if (HallData[i].IsInuse())
            {
            // is this room a window in hall we're checking
            if (HallData[i].IsWindowedIntoHall(thisRoom) || cfg.subhubs == 4)
                {
                // can group see this hall
                if (CurrentUser->CanAccessHall(i))
                    {
                    mPrintf(pctss, HallData[i].GetName(Buffer, sizeof(Buffer)));
                    thisHall = i;
                    done = TRUE;
                    }
                }
            }
        } while (!done);

    if (HallData[thisHall].IsDescribed() && CurrentUser->IsViewHallDescription())
        {
        if (!changedir(cfg.roompath))
            {
            return;
            }

        label HallDesc;
        HallData[thisHall].GetDescriptionFile(HallDesc, sizeof(HallDesc));

        if (!IsFilenameLegal(HallDesc, FALSE))
            {
            changedir(cfg.homepath);
            return;
            }

        doCR();

        if (!filexists(HallDesc))
            {
            CRmPrintfCR(getmsg(427), cfg.Lhall_nym, HallDesc);
            changedir(cfg.homepath);
            return;
            }

        if (!CurrentUser->IsExpert())
            {
            CRmPrintfCR(getmsg(426));
            }

        dumpf(HallDesc, 1, FALSE);

        changedir(cfg.homepath);

        OC.SetOutFlag(OUTOK);
        }
    else
        {
        doCR();
        }
    }

void HallEntry::Clear(void)
    {
    HallEntry1::Clear();
    HallEntry2::Clear();
    HallEntry3::Clear();
    }

void HallBuffer::Clear(void)
    {
    VerifyHeap();

    if (IsValid())
        {
        for (h_slot HS = 0; HS < NumHalls; HS++)
            {
            Halls[HS].Clear();
            }

        VerifyHeap();
        }
    }

void HallBuffer::Resize(h_slot NewNumberOfHalls, r_slot NumberOfRooms)
    {
    VerifyHeap();

    assert (NewNumberOfHalls <= (65532u / sizeof(HallEntry)));

    if (IsValid())
        {
        delete [] Halls;
        }

    NumHalls = NewNumberOfHalls;

    if (NewNumberOfHalls)
        {
        Halls = new HallEntry[NumHalls];

        if (Halls)
            {
            for (h_slot HS = 0; HS < NewNumberOfHalls; HS++)
                {
                Halls[HS].Resize(NumberOfRooms);
                }
            }

        Clear();
        }
    else
        {
        Halls = NULL;
        }

    VerifyHeap();
    }


HallEntry::HallEntry(void) :
        HallEntry1(),
        HallEntry2(0),
        HallEntry3(0)
        {
        }


HallEntry::HallEntry(r_slot Rooms) :
        HallEntry1(),
        HallEntry2(Rooms),
        HallEntry3(Rooms)
        {
        }

h_slot HallBuffer::GetDefault(void)
    {
    for (h_slot i = 0; i < NumHalls; i++)
        {
        if (Halls[i].IsDefault() && Halls[i].IsInuse())
            {
            return (i);
            }
        }

    return (MAINHALL);
    }

void HallBuffer::SetDefault(h_slot New)
    {
    for (h_slot i = 0; i < NumHalls; i++)
        {
        Halls[i].SetDefault(i == New);
        }
    }
