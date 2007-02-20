// --------------------------------------------------------------------------
// Citadel: Room3.CPP
//
// This should be a temporary file.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "libovl.h"
#include "log.h"
#include "group.h"
#include "msg.h"
#include "hall.h"
#include "miscovl.h"
#include "term.h"
#include "domenu.h"
#include "termwndw.h"


// --------------------------------------------------------------------------
// Contents
//
// listRooms()      lists known rooms
// ShowRoomStatus() Shows the status of a room...
// stepRoom()       goes to next or previous room
// gotoRoom()       handles "g(oto)" command for menu
// unGotoRoom()


void TERMWINDOWMEMBER ListAllRooms(Bool ThisHallOnly, Bool CanGetToRoom, Bool ShowNetID)
    {
    Bool *AccessArray = NULL;

    if (CanGetToRoom)
        {
        AccessArray = new Bool[cfg.maxrooms];

        if (AccessArray)
            {
            theOtherAlgorithm(AccessArray, thisHall, FALSE);
            }
        else
            {
            OutOfMemory(73);
            return;
            }
        }

    CRmPrintfCR(getmsg(578), cfg.Urooms_nym);

    prtList(LIST_START);

    for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
        {
        const r_slot R = RoomPosTableToRoom(RoomSlot);
        if (CurrentUser->CanAccessRoom(R, (h_slot) (ThisHallOnly ? thisHall : CERROR), FALSE))
            {
            if (!CanGetToRoom || AccessArray[R])
                {
                if (ShowNetID)
                    {
                    label Buffer;

                    if (*RoomTab[R].GetNetID(Buffer, sizeof(Buffer)))
                        {
                        prtList(Buffer);
                        }
                    }
                else
                    {
                    label Buffer;
                    prtList(RoomTab[R].GetName(Buffer, sizeof(Buffer)));
                    }
                }
            }
        }

    prtList(LIST_END);

    delete [] AccessArray;
    }


// --------------------------------------------------------------------------
// ShouldShowRoom()
#ifdef WINCIT
static Bool ShouldShowRoom(ulong what, r_slot R, g_slot G, const char *Keyword, TermWindowC *TW)
#else

static Bool ShouldShowRoom(ulong what, r_slot R, g_slot G, const char *Keyword)
#endif
    {
    label RoomName;
    char RoomInfo[80];

    if (*Keyword)
        {
        const r_slot room = RoomPosTableToRoom(R);
        RoomC RoomBuffer;
        RoomBuffer.Load(room);
        *RoomBuffer.GetInfoLine(RoomInfo, sizeof(RoomInfo));
        stripansi(RoomInfo);

        RoomTab[R].GetName(RoomName, sizeof(RoomName));
        stripansi(RoomName);
        }

    BoolExpr Buffer;

    return (
            (!(what & K_APPLIC) || RoomTab[R].IsApplication())
        &&
            (!(what & K_ANON) || RoomTab[R].IsAnonymous())
        &&
            (!(what & K_PERM) || RoomTab[R].IsPermanent())
        &&
            (!(what & K_DIR) || RoomTab[R].IsMsDOSdir())
        &&
            (
            !(what & K_GROUP) ||
                (
                G == CERROR ?
                    (
                    RoomTab[R].IsGroupOnly()
                    )
                :
                    (
                    RoomTab[R].IsGroupOnly() ?
                        (
                        RoomTab[R].IsBooleanGroup() ?
                            (
                            IsInBooleanExpression(RoomTab[R].
                            GetGroupExpression(Buffer), G)
                            )
                        :
                            (
                            RoomTab[R].GetGroupNumber() == G
                            )
                        )
                    :
                        (
                        FALSE
                        )
                    )
                )
            )
        &&
            (!(what & K_LOCAL) || !RoomTab[R].IsShared())
        &&
            (!(what & K_SHARED) || RoomTab[R].IsShared())
        &&
            (!(what & K_THALL) || roomonlyinthishall(R, tw()thisHall))
        &&
            (!(what & K_NOMSGS) || !tw()Talley->MessagesInRoom(R))
        &&
            (!(what & K_BIO) || RoomTab[R].IsBIO())
        &&
            (!(what & K_HIDDEN) || !RoomTab[R].IsPublic())
        &&
            (!(what & K_WINDOWS) || iswindow(R))
        &&
            (!(what & K_KEYWORD) || u_match(RoomName, Keyword) || u_match(RoomInfo, Keyword))
        );
    }


// --------------------------------------------------------------------------
// listRooms(): Lists known rooms.
//
// Input:
//  ulong what: Bitwise ORed (|) flags, defined in CTDL.H:
//      K_APPLIC    0x00000001l             K_ANON      0x00000002l
//      K_PERM      0x00000004l             K_DIR       0x00000008l
//      K_GROUP     0x00000010l             K_LOCAL     0x00000020l
//      K_NEW       0x00000040l             K_OLD       0x00000080l
//      K_MAIL      0x00000100l             K_SHARED    0x00000200l
//      K_THALL     0x00000400l             K_WINDOWS   0x00000800l
//      K_EXCLUDE   0x00001000l             K_NUM       0x00002000l
//      K_NOMSGS    0x00004000l             K_BIO       0x00008000l
//      K_HIDDEN    0x00010000l             K_KEYWORD   0x00020000l

void TERMWINDOWMEMBER listRooms(ulong what)
    {
    if (!LockMenu(MENU_KNOWN))
        {
        OutOfMemory(111);
        return;
        }

    SetDoWhat(KNOWNROOMS);

    MRO.Reverse = FALSE;

    if ((what & K_THALL) && CurrentUser->IsUsePersonalHall())
        {
        CRmPrintfCR(getmsg(MENU_KNOWN, 24), cfg.Lhall_nym, cfg.Lhall_nym);
        UnlockMenu(MENU_KNOWN);
        return;
        }

    g_slot groupslot = CERROR;

    if (what & K_GROUP)
        {
        label groupname;

        if (!GetStringWithBlurb(cfg.Lgroup_nym, groupname, LABELSIZE, ns, B_KNOWNGRP))
            {
            UnlockMenu(MENU_KNOWN);
            return;
            }

        normalizeString(groupname);

        if (*groupname)
            {
            groupslot = FindGroupByPartialName(groupname, FALSE);

            if (groupslot != CERROR && !CurrentUser->IsInGroup(groupslot))
                {
                groupslot = CERROR;
                }

            if (groupslot != CERROR)
                {
                label Buffer;
                CRmPrintfCR(getmsg(MENU_KNOWN, 25), cfg.Urooms_nym, cfg.Lgroup_nym,
						GroupData[groupslot].GetName(Buffer, sizeof(Buffer)));
                }
            else
                {
                CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
                UnlockMenu(MENU_KNOWN);
                return;
                }
            }
        else
            {
            groupslot = CERROR;
            CRmPrintfCR(getmsg(MENU_KNOWN, 26), cfg.Urooms_nym, cfg.Lgroup_nym);
            }
        }


    char Keyword[128];
    if (what & K_KEYWORD)
        {
        if (!GetStringWithBlurb(getmsg(MENU_KNOWN, 27), Keyword, sizeof(Keyword) - 1, ns, B_KNWNKYWD))
            {
            UnlockMenu(MENU_KNOWN);
            return;
            }

        normalizeString(Keyword);

		if (!(strchr(Keyword, '*') || strchr(Keyword, '?') || strchr(Keyword, '[')))
			{
			if (strlen(Keyword) < sizeof(Keyword) - 3)
				{
				char StupidTempString[128];
				sprintf(StupidTempString, "*%s*", Keyword);
				CopyStringToBuffer(Keyword, StupidTempString);
				}
			}

        if (!*Keyword)
            {
            what &= ~K_KEYWORD;
            }
        }

    if (!(what & K_KEYWORD))
        {
        *Keyword = 0;
        }


    OC.SetOutFlag(OUTOK);

    showdir     = FALSE;
    showhidden  = FALSE;
    showbio     = FALSE;
    showgroup   = FALSE;

    const Bool ShowNum = !!(what & K_NUM);

    // NEW rooms
    if (    // Normal Known, or .KN
            (!what || what & K_NEW) &&

            // and not .KX (we don't care if they are new)
            !(what & K_EXCLUDE) &&

            // and not .K0 (if there are no messages, none will be new)
            !(what & K_NOMSGS) &&

            // and not .KM (we list .KM all at once)
            !(what & K_MAIL) &&

            // and not .KW (we list .KW all at once)
            !(what & K_WINDOWS)
        )
        {
        char wow[64];

        if (CurrentUser->IsUsePersonalHall())
            {
            sprintf(wow, getmsg(MENU_KNOWN, 28), cfg.Lhall_nym);
            }
        else
            {
            HallData[thisHall].GetName(wow, sizeof(wow));
            }

        CRmPrintfCR(getmsg(MENU_KNOWN, 29), cfg.Urooms_nym, cfg.Lmsgs_nym, wow);

        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            prtList(LIST_START);
            }

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);

            if (CurrentUser->CanAccessRoom(R, thisHall) && !CurrentUser->IsRoomExcluded(R) && Talley->NewInRoom(R))
                {
#ifdef WINCIT
                if (ShouldShowRoom(what, R, groupslot, Keyword, this))
#else
                if (ShouldShowRoom(what, R, groupslot, Keyword))
#endif
                    {
                    printroomVer(R, ShowNum);
                    }
                }
            }

        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            if (!ShowNum)
                {
                prtList(LIST_END);
                }
            }
        else
            {
            doCR();
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    // new MAIL rooms
    if (
            // Normal K or .KN
            (!what || what & K_NEW) ||

            // or .KM, but not .KOM
            (
                (what & K_MAIL) && !(what & K_OLD)
            )
        )
        {
        Bool FirstTime = TRUE;

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);
            if (CurrentUser->CanAccessRoom(R, thisHall) && Talley->HasNewMail(R))
                {
#ifdef WINCIT
                if (ShouldShowRoom(what, R, groupslot, Keyword, this))
#else
                if (ShouldShowRoom(what, R, groupslot, Keyword))
#endif
                    {
                    if (FirstTime)
                        {
                        CRmPrintfCR(getmsg(MENU_KNOWN, 30), cfg.Lmsgs_nym);

                        FirstTime = FALSE;

                        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                            {
                            prtList(LIST_START);
                            }
                        }

                    printroomVer(R, ShowNum);
                    }
                }
            }

        if (!FirstTime)
            {
            if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                {
                if (!ShowNum)
                    {
                    prtList(LIST_END);
                    }
                }
            else
                {
                doCR();
                }
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    // old MAIL rooms - only on .KOM
    if ((what & K_MAIL) && (what & K_OLD))
        {
        Bool FirstTime = TRUE;

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);
            if (CurrentUser->CanAccessRoom(R, thisHall) && Talley->HasOldMail(R))
                {
#ifdef WINCIT
                if (ShouldShowRoom(what, R, groupslot, Keyword, this))
#else
                if (ShouldShowRoom(what, R, groupslot, Keyword))
#endif
                    {
                    if (FirstTime)
                        {
                        CRmPrintfCR(getmsg(MENU_KNOWN, 31));

                        FirstTime = FALSE;
                        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                            {
                            prtList(LIST_START);
                            }
                        }

                    printroomVer(R, ShowNum);
                    }
                }
            }

        if (!FirstTime)
            {
            if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                {
                if (!ShowNum)
                    {
                    prtList(LIST_END);
                    }
                }
            else
                {
                doCR();
                }
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    // OLD rooms
    if (
            // Normal K or .KO
            (!what || what & K_OLD) &&

            // and not .KX (all listed at once)
            !(what & K_EXCLUDE) &&

            // and not .KM
            !(what & K_MAIL) &&

            // and not .KW
            !(what & K_WINDOWS))
        {

        // "no unseen messages" or "no messages" if .K0
        CRmPrintfCR((what & K_NOMSGS) ? getmsg(MENU_KNOWN, 32) : getmsg(MENU_KNOWN, 33), cfg.Lmsgs_nym);

        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            prtList(LIST_START);
            }

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);
            if (CurrentUser->CanAccessRoom(R, thisHall) && !CurrentUser->IsRoomExcluded(R) && !Talley->NewInRoom(R))
                {
#ifdef WINCIT
                if (ShouldShowRoom(what, R, groupslot, Keyword, this))
#else
                if (ShouldShowRoom(what, R, groupslot, Keyword))
#endif
                    {
                    printroomVer(R, ShowNum);
                    }
                }
            }

        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            if (!ShowNum)
                {
                prtList(LIST_END);
                }
            }
        else
            {
            doCR();
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    // EXCLUDED rooms - list always unless .KN
    if (what != K_NEW)
        {
        Bool FirstTime = TRUE;

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);
            if (CurrentUser->CanAccessRoom(R, thisHall) && CurrentUser->IsRoomExcluded(R))
                {
                if (    // Normal rules
#ifdef WINCIT
                        ShouldShowRoom(what, R, groupslot, Keyword, this) &&
#else
                        ShouldShowRoom(what, R, groupslot, Keyword) &&
#endif

                        // Plus: Don't list if .KM
                        (!(what & K_MAIL) || Talley->HasNewMail(R) || Talley->HasOldMail(R))
                    )
                    {
                    if (FirstTime)
                        {
                        CRmPrintfCR(getmsg(MENU_KNOWN, 34), cfg.Lrooms_nym);

                        FirstTime = FALSE;

                        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                            {
                            prtList(LIST_START);
                            }
                        }

                    printroomVer(R, ShowNum);
                    }
                }
            }

        if (!FirstTime)
            {
            if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                {
                if (!ShowNum)
                    {
                    prtList(LIST_END);
                    }
                }
            else
                {
                doCR();
                }
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    // WINDOWS
    if (what & K_WINDOWS && !(what & K_EXCLUDE))
        {
        Bool FirstTime = TRUE;

        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            prtList(LIST_START);
            }

        for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
            {
            const r_slot R = RoomPosTableToRoom(i);
            if (CurrentUser->CanAccessRoom(R, thisHall) && !CurrentUser->IsRoomExcluded(R))
                {
#ifdef WINCIT
                if (ShouldShowRoom(what, R, groupslot, Keyword, this))
#else
                if (ShouldShowRoom(what, R, groupslot, Keyword))
#endif
                    {
                    if (FirstTime)
                        {
                        CRmPrintfCR(getmsg(MENU_KNOWN, 35), cfg.Urooms_nym, cfg.Lhalls_nym);

                        FirstTime = FALSE;

                        if (!MRO.Verbose && !CurrentUser->IsWideRoom())
                            {
                            prtList(LIST_START);
                            }
                        }

                    if (MRO.Verbose)
                        {
                        char string[LABELSIZE+LABELSIZE];
                        mPrintf(getmsg(MENU_KNOWN, 36), MakeRoomName(R, string));

                        prtList(LIST_START);

                        for (h_slot j = 0 ; j < cfg.maxhalls; j++)
                            {
                            if (
                                    HallData[j].IsInuse()

                                    &&

                                    HallData[j].IsWindowedIntoHall(R)

                                    &&

                                    CurrentUser->CanAccessHall(j))
                                {
                                label Buffer;
                                prtList(HallData[j].GetName(Buffer, sizeof(Buffer)));
                                }
                            }

                        prtList(LIST_END);
                        }
                    else
                        {
                        MRO.Verbose = FALSE;
                        printroomVer(R, FALSE);
                        }
                    }
                }
            }

        if (!FirstTime && !MRO.Verbose && !CurrentUser->IsWideRoom())
            {
            prtList(LIST_END);
            }
        else
            {
            if (!FirstTime && !MRO.Verbose)
                {
                doCR();
                }
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        UnlockMenu(MENU_KNOWN);
        return;
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    if (!CurrentUser->IsExpert())
        {
        if (showhidden)
            {
            CRmPrintf(getmsg(MENU_KNOWN, 37), cfg.Lroom_nym);
            }

        if (showbio)
            {
            CRmPrintf(getmsg(MENU_KNOWN, 38), cfg.Lroom_nym);
            }

        if (showgroup)
            {
            CRmPrintf(getmsg(MENU_KNOWN, 39), CurrentUser->IsIBMRoom() ? '\xb3' : ':', cfg.Lgroup_nym, cfg.Lroom_nym);
            }

        if (showdir)
            {
            CRmPrintf(getmsg(MENU_KNOWN, 40), CurrentUser->IsIBMRoom() ? '\xb9' : ']', cfg.Lroom_nym);
            }

        if (showhidden || showbio || showgroup || showdir)
            {
            doCR();
            }
        }

    UnlockMenu(MENU_KNOWN);
    }


// --------------------------------------------------------------------------
// ShowRoomStatus(): Shows the status of a room.

void TERMWINDOWMEMBER ShowRoomStatus(void)
    {
    char buff[500];

    SetDoWhat(KNOWNRMINFO);

    OC.SetOutFlag(OUTOK);

    CRmPrintf(pctss, cfg.Uhall_nym);
    label Buffer;
    mPrintf(pcts, HallData[thisHall].GetName(Buffer, sizeof(Buffer)));

    if (HallData[thisHall].IsOwned())
        {
        if (HallData[thisHall].IsBoolGroup())
            {
            mPrintf(getmsg(469));
            }
        else
            {
            mPrintf(getmsg(396), cfg.Lgroup_nym,
					GroupData[HallData[thisHall].GetGroupNumber()].GetName(Buffer, sizeof(Buffer)));
            }
        }
    doCR();

    CreateRoomSummary(buff, thisRoom);
    CRmPrintfCR(pcts, buff);

    CurrentRoom->GetCreator(buff, sizeof(buff));
    if (*buff)
        {
        mPrintfCR(getmsg(337), buff);
        }

    if (iswindow(thisRoom))
        {
        CRmPrintfCR(getmsg(397), cfg.Lhalls_nym);

        prtList(LIST_START);
        for (h_slot j = 0; j < cfg.maxhalls; j++)
            {
            if (
                    HallData[j].IsInuse()

                    &&

                    HallData[j].IsWindowedIntoHall(thisRoom)

                    &&

                    CurrentUser->CanAccessHall(j))
                {
                prtList(HallData[j].GetName(Buffer, sizeof(Buffer)));
                }
            }
        prtList(LIST_END);
        }

    ReadHalls();

    dumpRoom(FALSE);
    }


// --------------------------------------------------------------------------
// printroomVer(): displays name of specified room.
//
// Input:
//  r_slot room: Room to display
//  Bool numMess: Should we talk about the number of messages?
//
// Notes:
//  Called by listRooms. Sets global flags.

void TERMWINDOWMEMBER printroomVer(r_slot room, Bool numMess)
    {
    char string[LABELSIZE+LABELSIZE];

    MakeRoomName(room, string);

    if (RoomTab[room].IsGroupOnly())
        {
        showgroup = TRUE;
        }

    if (RoomTab[room].IsMsDOSdir())
        {
        showdir = TRUE;
        }

    if (!RoomTab[room].IsPublic())
        {
        showhidden = TRUE;
        }

    if (RoomTab[room].IsBIO())
        {
        showbio = TRUE;
        }

    if (!MRO.Verbose && !numMess)
        {
        if (CurrentUser->IsWideRoom())
            {
            if (OC.CrtColumn > 3 && OC.CrtColumn < 40)
                {
                while (OC.CrtColumn < 40)
                    {
                    oChar(' ');
                    }
                }
            else
                {
                doCR();
                }

            mPrintf(pcts, string);
            termCap(TERM_NORMAL);

            PLlisted = TRUE;    // for prtList(LIST_END)
            }
        else
            {
            prtList(string);
            }

        }
    else
        {
        RoomC RoomBuffer;

        RoomBuffer.Load(room);

        mPrintf(getmsg(468), string, 33 - strlen(deansi(string)), ns);

        // this sux, fix it later
        if (!numMess)
            {
            label Buffer; if (*RoomBuffer.GetNetID(Buffer, sizeof(Buffer)) && RoomBuffer.IsShared())
                {
                sprintf(string, getmsg(347), Buffer);
                mPrintf(getmsg(343), string);
                }
            }
        /////////////////////////

        if (numMess)
            {
            if (CurrentUser->IsAide())
                {
                mPrintf(getmsg(398), (long) Talley->TotalMessagesInRoom(room));
                }

            mPrintf(getmsg(399), (long) Talley->MessagesInRoom(room),
					Talley->MessagesInRoom(room) == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym, (long) Talley->NewInRoom(room));

            if (Talley->NewInRoom(room) && Talley->HasNewMail(room))
                {
                mPrintf(getmsg(400));
                }

            doCR();

            char Buffer[80];
            if (*RoomBuffer.GetInfoLine(Buffer, sizeof(Buffer)) && MRO.Verbose)
                {
                mPrintf(getmsg(342));
                }
            }

        if (MRO.Verbose)
            {
            char Buffer[80];

            if (*RoomBuffer.GetInfoLine(Buffer, sizeof(Buffer)))
                {
                mPrintf(pcts, Buffer);
                }

            if (!numMess || *Buffer)
                {
                doCR();
                }
            }

        if (MRO.Verbose && numMess)
            {
            doCR();
            }
        }
    }


// --------------------------------------------------------------------------
// stepRoom(): Move to an adjacent room.
//
// Input:
//  Bool direction: TRUE for next; FALSE for previous

void TERMWINDOWMEMBER stepRoom(Bool direction)
    {
    r_slot i, rp;
    Bool done = FALSE;

    i = rp = GetRoomPosInTable(thisRoom);

    do
        {
        if (direction)
            {
            if (++i == cfg.maxrooms)
                {
                i = 0;
                }
            }
        else
            {
            if (--i == -1)
                {
                i = (r_slot) (cfg.maxrooms - 1);
                }
            }

        if (i == rp || (CurrentUser->CanAccessRoom(RoomPosTableToRoom(i), thisHall)))
            {
            label RoomName;

            // for the i == rp case if killed only room in hall.
            if (!RoomTab[RoomPosTableToRoom(i)].IsInuse())
                {
                mPrintfCR(pcts, RoomTab[RoomPosTableToRoom(LOBBY)].GetName(RoomName, sizeof(RoomName)));

                ChangeRooms(LOBBY);

                dumpRoom(TRUE);
                }
            else
                {
                mPrintfCR(pcts, RoomTab[RoomPosTableToRoom(i)].GetName(RoomName, sizeof(RoomName)));

                gotoRoom(RoomName, FALSE, FALSE, FALSE);
                }

            done = TRUE;
            }
        } while (!done);
    }


// --------------------------------------------------------------------------
// unGotoRoom()

void TERMWINDOWMEMBER unGotoRoom(void)
    {
    Bool tryAgain;
    int TryCounter = 0;

    mPrintf(getmsg(590));

    do
        {
        jumpback JB;
        tryAgain = FALSE;

        if (CurrentUser->JumpbackPop(&JB))
            {
            if (JB.room < cfg.maxrooms && JB.room >= 0 && CurrentUser->CanAccessRoom(JB.room)
// too slow && theAlgorithm(JB.room, thisHall, FALSE)
                    )
                {
                if (JB.hall < cfg.maxhalls && JB.hall >= 0 && HallData[JB.hall].IsInuse() &&
						CurrentUser->CanAccessHall(JB.hall))
                    {
                    thisHall = JB.hall;
                    }

                doBS(TryCounter);

                label Buffer;
                mPrintfCR(spcts, RoomTab[JB.room].GetName(Buffer, sizeof(Buffer)));

                ChangeRooms(JB.room);

                Talley->SetBypass(thisRoom, JB.bypass);
                CurrentUser->SetRoomNewPointer(thisRoom, JB.newpointer);
                Talley->SetRoomNewMessages(thisRoom, JB.newMsgs);

                dumpRoom(TRUE);
                }
            else
                {
                mPrintf(getmsg(433));
                TryCounter++;
                tryAgain = TRUE;
                }
            }
        else
            {
            doBS(TryCounter + strlen(getmsg(590)));

            mPrintf(getmsg(344));
            }
        } while (tryAgain);
    }


// --------------------------------------------------------------------------
// gotoRoom(): Travel to a new room
//
// Input:
//  const char *roomname: Room name to go to or empty string for next room with new messages.
//  Bool skiproom: TRUE to not update message pointers (Bypass)
//  Bool partial: TRUE to allow roomname to be partial; FALSE for exact
//  Bool mailScan: TRUE to skip to next room with mail. Overrides roomname

void TERMWINDOWMEMBER gotoRoom(const char *roomname, Bool skiproom, Bool partial, Bool mailScan)
    {
    r_slot roomNo = CERROR;
    h_slot oldHall = thisHall;

    r_slot rp = GetRoomPosInTable(thisRoom);

    // make sure we can get back
    CurrentUser->SetInRoom(thisRoom, TRUE);

    if (!(*roomname) || mailScan)
        {
        Bool *RoomAccess = NULL;

        if (mailScan)
            {
            RoomAccess = new Bool[cfg.maxrooms];

            if (RoomAccess)
                {
                theOtherAlgorithm(RoomAccess, thisHall, FALSE);
                }
            }

        r_slot i, j;
        for (j = (r_slot) (rp + 1), i = 0; j != rp && roomNo == CERROR && i < cfg.maxrooms; j++, i++)
            {
            if (j == cfg.maxrooms)
                {
                j = 0;
                }

			const r_slot Slot = RoomPosTableToRoom(j);

            if (mailScan)
                {
                Bool HasMail;

                if (RoomAccess)
                    {
                    HasMail = RoomAccess[Slot] && Talley->HasNewMail(Slot);
                    }
                else
                    {
                    // Wasn't enough memory for theOtherAlgorithm... use
                    // theAlgorithm.

                    HasMail =
                        // auxmem or wincit - faster to to theAlgorithm.
                        // else, faster to do IsRoomMail
                        #if defined(AUXMEM) || defined(WINCIT)
                            theAlgorithm(Slot, thisHall, FALSE) && Talley->HasNewMail(Slot);
                        #else
                            IsRoomMail(Slot) && theAlgorithm(Slot, thisHall, FALSE);
                        #endif
                    }

                if (HasMail)
                    {
                    roomNo = Slot;
                    }
                }
            else
                {
                if (CurrentUser->CanAccessRoom(Slot, thisHall) && (Talley->NewInRoom(Slot)

                // or is it a window?
                || (iswindow(Slot) && j > rp && thisHall != 1 && !(cfg.subhubs == 4)
                    && cfg.subhubs == 2 /* DragCit-style: empties visited */))

                // is it NOT excluded
                &&  (!CurrentUser->IsRoomExcluded(Slot) || Talley->HasNewMail(Slot))

                // is it NOT bypassed
                &&  (!Talley->Bypassed(Slot))

                // we do not come back to current room
                &&   j != rp)
                    {
                    roomNo = Slot;
                    }
                }
            }

        delete [] RoomAccess;

        if (roomNo == CERROR && !(cfg.subhubs == 4) && cfg.subhubs)
            {
            for (j = (r_slot) (rp + 1), i = 0; j != rp && roomNo == CERROR && i < cfg.maxrooms; j++, i++)
                {
                if (j == cfg.maxrooms)
                    {
                    j = 0;
                    }

				const r_slot Slot = RoomPosTableToRoom(j);

                if (iswindow(Slot)

                // can user access this room?
                && CurrentUser->CanAccessRoom(Slot, thisHall)

                // is it NOT excluded
                && !CurrentUser->IsRoomExcluded(Slot)

                // is it NOT bypassed
                && !Talley->Bypassed(Slot))
                    {
                    roomNo = Slot;
                    }
                }
            }

        if ((mailScan || (cfg.subhubs == 4)) && roomNo == CERROR)
            {
            roomNo = LOBBY;
            }

        if (roomNo != CERROR)
            {
            label Buffer;
            mPrintf(pcts, RoomTab[roomNo].GetName(Buffer, sizeof(Buffer)));
            }
        }
    else
        {
        if (CurrentUser->IsUsePersonalHall())
            {
            r_slot check = RoomExists(roomname);

            if (check != CERROR)
                {
                if (!CurrentUser->CanAccessRoom(check, CERROR, TRUE))
                    {
                    check = CERROR;
                    }
                else
                    {
                    CurrentUser->SetInRoom(check, TRUE);
                    }
                }

            if (check == CERROR && partial)
                {
                check = PartialRoomExists(roomname, thisRoom, TRUE);
                }

            if (check != CERROR && theAlgorithm(check, thisHall, FALSE) && CurrentUser->CanAccessRoom(check))
                {
                roomNo = check;
                }
            else
                {
                CRmPrintfCR(getmsg(157), cfg.Lroom_nym, roomname);
                }
            }
        else
            {
            r_slot check = RoomExists(roomname);

            if (check != CERROR)
                {
                if (!CurrentUser->CanAccessRoom(check, thisHall, TRUE))
                    {
                    check = CERROR;
                    }
                else
                    {
                    CurrentUser->SetInRoom(check, TRUE);
                    }
                }

            if (check == CERROR && partial)
                {
                check = PartialRoomExists(roomname, thisRoom, FALSE);
                }

            if (check != CERROR && CurrentUser->CanAccessRoom(check, thisHall))
                {
                roomNo = check;
                }
            else
                {
                CRmPrintfCR(getmsg(157), cfg.Lroom_nym, roomname);
                }
            }
        }

    if (roomNo != CERROR)
        {
        if (roomNo != thisRoom)
            {
            jumpback jb;

            jb.hall = oldHall;
            jb.room = thisRoom;
            jb.newpointer = CurrentUser->GetRoomNewPointer(thisRoom);
            jb.bypass = Talley->Bypassed(thisRoom);
            jb.newMsgs = Talley->NewInRoom(thisRoom);

            CurrentUser->JumpbackPush(jb);
            }

        // update new pointer for this room
        if (!skiproom)
            {
            Talley->SetVisited(thisRoom, TRUE);
            CurrentUser->SetRoomNewPointer(thisRoom, MessageDat.NewestMessage());

            Talley->SetHasNewMail(thisRoom, FALSE);
            Talley->SetHasOldMail(thisRoom, FALSE);
            Talley->SetRoomNewMessages(thisRoom, 0);

            // unset Bypass on .Goto
            Talley->SetBypass(thisRoom, FALSE);
            }

        if ((roomNo != thisRoom) && skiproom && thisRoom != LOBBY)
            {
            Talley->SetBypass(thisRoom, TRUE);
            }

        // Get room and display it.
        ChangeRooms(roomNo);

        if (!*roomname)
            {
            if (((!(cfg.subhubs == 4) && iswindow(roomNo)) || (cfg.subhubs == 4 && roomNo == LOBBY)) &&
                    !CurrentUser->IsUsePersonalHall() && CurrentUser->IsAutoNextHall())
                {
                mPrintf(getmsg(589), cfg.Lhall_nym);
                stephall(1);
                }

            doCR();
            }

        dumpRoom(TRUE);

        if (CurrentUser->IsUsePersonalHall())
            {
            CurrentUser->SetRoomInPersonalHall(thisRoom, TRUE);
            }
        }
    }


Bool IsNetIDCorrupted(const char *NetID)
    {
    if (NetID && *NetID && strlen(NetID) <= LABELSIZE && !hasansi(NetID) && !strchr(NetID, '"'))
        {
        for (int i = 0; NetID[i]; i++)
            {
            if (NetID[i] < 32)
                {
                return (TRUE);
                }
            }

        label Buffer;

        CopyStringToBuffer(Buffer, NetID);
        normalizeString(Buffer);

        return (!SameString(NetID, Buffer));
        }

    return (TRUE);
    }

void TERMWINDOWMEMBER AideJumpToAideAndMaintenance(void)
    {
    if (LockMenu(MENU_AIDE))
        {
        label Buffer, Buffer1;
        mPrintfCR(getmenumsg(MENU_AIDE, 20), RoomTab[AIDEROOM].GetName(Buffer1, sizeof(Buffer1)), cfg.Lroom_nym,
                HallData[MAINTENANCE].GetName(Buffer, sizeof(Buffer)), cfg.Lhall_nym);

        if (CurrentUser->CanAccessRoom(AIDEROOM, CERROR, TRUE) && CurrentUser->CanAccessHall(1))
            {
            if (CurrentUser->IsUsePersonalHall())
                {
                CRCRmPrintfCR(getmenumsg(MENU_AIDE, 21), cfg.Uhall_nym, cfg.Lhall_nym);
                }
            else
                {
                thisHall = MAINTENANCE;
                }

            label AideName;
            RoomTab[AIDEROOM].GetName(AideName, sizeof(AideName));

            gotoRoom(AideName, FALSE, FALSE, FALSE);
            }
        else
            {
            CRmPrintfCR(getmenumsg(MENU_AIDE, 22), cfg.Lgroup_nym);
            }

        UnlockMenu(MENU_AIDE);
        }
    }

void TERMWINDOWMEMBER ChangeRooms(r_slot NewRoom)
    {
    if (thisRoom != NewRoom)
        {
#ifdef WINCIT
        char Event[256];
        label Name;

        // system message to leave thisRoom
        sprintf(Event, getmsg(219), CurrentUser->GetName(Name, sizeof(Name)), cfg.Lroom_nym);
        TermWindowCollection.SystemEvent(SE_ROOMINOUT, thisRoom, NULL, FALSE, Event);
#endif

        thisRoom = NewRoom;
        CurrentRoom->Load(thisRoom);
        checkdir();

#ifdef WINCIT
        // system message to enter the new thisRoom
        sprintf(Event, getmsg(220), CurrentUser->GetName(Name, sizeof(Name)), cfg.Lroom_nym);
        TermWindowCollection.SystemEvent(SE_ROOMINOUT, thisRoom, NULL, FALSE, Event);
#endif
        }
    }
