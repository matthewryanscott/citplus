// --------------------------------------------------------------------------
// Citadel: DoRmHall.CPP
//
// Room and Hall commands

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "hall.h"
#include "miscovl.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// exclude()        handles X>clude room, toggles the bit
// doGoto()         handles G(oto) command
// doGotoMail()     handles .M(ail scan) command
// doKnown()        handles K(nown rooms) command
// doNext()         handles '+' next room
// doPrevious()     handles '-' previous room
// doNextHall()     handles '>' next room
// doPreviousHall() handles '<' previous room
// doPersonal()     handles .P(ersonal hall) command


// --------------------------------------------------------------------------
// exclude(): Handles X>clude room, toggles the bit.
//
// Notes:
//  Assumes MENU_ENTER is locked.

void TERMWINDOWMEMBER exclude(void)
    {
    SetDoWhat(DOEXCLUDE);

    if ((thisRoom == LOBBY || CurrentRoom->IsUnexcludable()) && !CurrentUser->IsRoomExcluded(thisRoom))
        {
        CRCRmPrintfCR(getmenumsg(MENU_ENTER, 49), cfg.Lroom_nym);
        }
    else if (!CurrentUser->IsRoomExcluded(thisRoom))
        {
        if (!CurrentUser->IsExpert())
            {
            char Prompt[128];

            doCR();
            sprintf(Prompt, getmenumsg(MENU_ENTER, 50), cfg.Lroom_nym);
            if (!getYesNo(Prompt, 1))
                {
                return;
                }
            }
        else
            {
            doCR();
            }

        CRmPrintfCR(getmenumsg(MENU_ENTER, 51), cfg.Uroom_nym);
        CurrentUser->SetRoomExcluded(thisRoom, TRUE);
        }
    else
        {
        CRCRmPrintfCR(getmenumsg(MENU_ENTER, 52), cfg.Uroom_nym);
        CurrentUser->SetRoomExcluded(thisRoom, FALSE);
        }
    }


// --------------------------------------------------------------------------
// doGoto(): Handles G(oto) command.

void TERMWINDOWMEMBER doGoto(Bool expand, char skip)
    {
    if (loggedIn)
        {
        CurrentUser->Save(ThisLog, thisRoom);
        }

    if (!skip)
        {
        mPrintf(getmsg(260));
        if (expand)
            {
            mPrintf(cfg.Lroom_nym);
            mPrintf(": ");
            }
        SetDoWhat(DOGOTO);
        }
    else
        {
        mPrintf(getmsg(261));
        if (expand)
            {
            mPrintf(cfg.Lroom_nym);
            mPrintf(": ");
            }
        SetDoWhat(DOBYPASS);
        }

    if (!expand)
        {
        gotoRoom(ns, skip, TRUE, FALSE);
        }
    else
        {
        label roomName;
        roomName[0] = 0;

        do
            {
            char Prompt[128];
            Prompt[0] = 0;

//          sprintf(Prompt, *roomName == '?' ? getmsg(262) : ns, cfg.Lroom_nym);

            if (*roomName == '?')
                {
                sprintf(Prompt, getmsg(262), cfg.Lroom_nym);
                }
            if (!getString(Prompt, roomName, LABELSIZE, ns))
                {
                return;
                }

            normalizeString(roomName);

            if (*roomName == '?')
                {
                MRO.Verbose = FALSE;
                listRooms(0);
                }
            } while (*roomName == '?');

        //jrs - remove Bypass on .Goto -- gotoRoom(roomName, skip, TRUE, FALSE);
        gotoRoom(roomName, FALSE, TRUE, FALSE);
        }
    }


// --------------------------------------------------------------------------
// doGotoMail(): Handles .M(ail Scan) command.

void TERMWINDOWMEMBER doGotoMail(void)
    {
    mPrintf(getmsg(621), cfg.Lroom_nym);
    SetDoWhat(DOGOTOMAIL);

    gotoRoom(ns, FALSE, TRUE, TRUE);
    }


// --------------------------------------------------------------------------
// doKnown(): Handles K(nown rooms) command.

void TERMWINDOWMEMBER doKnown(Bool moreYet, char first)
    {
    if (!LockMenu(MENU_KNOWN))
        {
        return;
        }

    Bool done = FALSE;
    ulong kWhat = K_NEW | K_OLD;

    SetDoWhat(DOKNOWN);

    MRO.Verbose = FALSE;
    MRO.Reverse = FALSE;

    if (moreYet)
        {
        first = 0;
        }

    mPrintf(getmenumsg(MENU_KNOWN, 23));

    int OptionCounter = 0;

    do
        {
        const int ich = first ? first : iCharNE();

        if (++OptionCounter == 69)
            {
            mPrintf(getmsg(472), getmsg(69), spc);
            OptionCounter = 0;
            }

        switch (DoMenu(MENU_KNOWN, ich))
            {
            case 1: // Empty
                {
                kWhat |= K_NOMSGS;
                break;
                }

            case 2: // Application
                {
                kWhat |= K_APPLIC;
                break;
                }

            case 3: // BIO
                {
                kWhat |= K_BIO;
                break;
                }

            case 4: // Directory
                {
                kWhat |= K_DIR;
                break;
                }

            case 5: // Group-only
                {
                kWhat |= K_GROUP;
                break;
                }

            case 6: // Halls
                {
                doCR();
                knownhalls();
                done = TRUE;
                break;
                }

            case 7: // Room info
                {
                doCR();
                ShowRoomStatus();
                done = TRUE;
                break;
                }

            case 8: // Keyword search
                {
                kWhat |= K_KEYWORD;
                break;
                }

            case 9: // Local
                {
                kWhat |= K_LOCAL;
                kWhat &= ~K_SHARED;
                break;
                }

            case 10: // Mail
                {
                kWhat |= K_MAIL;
                break;
                }

            case 11: // New
                {
                kWhat |= K_NEW;
                kWhat &= ~K_OLD;
                break;
                }

            case 12: // Old
                {
                kWhat |= K_OLD;
                kWhat &= ~K_NEW;
                break;
                }

            case 13: // Permanent
                {
                kWhat |= K_PERM;
                break;
                }

            case 14: // Rooms
                {
                doCR();
                listRooms(kWhat);
                done = TRUE;
                break;
                }

            case 15: // Shared
                {
                kWhat |= K_SHARED;
                kWhat &= ~K_LOCAL;
                break;
                }

            case 16: // This-hall-only
                {
                kWhat |= K_THALL;
                break;
                }

            case 17: // Verbose
                {
                MRO.Verbose = TRUE;
                break;
                }

            case 18: // Window
                {
                if (cfg.subhubs != 4)
                    {
                    kWhat |= K_WINDOWS;
                    }

                break;
                }

            case 19: // Excluded
                {
                kWhat |= K_EXCLUDE;
                kWhat &= ~K_NEW & ~K_OLD;
                break;
                }

            case 20: // Anonymous
                {
                kWhat |= K_ANON;
                break;
                }

            case 21: // Hidden
                {
                kWhat |= K_HIDDEN;
                break;
                }

            case 22: // Number of messages
                {
                kWhat |= K_NUM;
                break;
                }

            case -1:
                {
                done = TRUE;
                break;
                }

            default:
                {
                oChar((char) ich);
                mPrintf(sqst);
                done = TRUE;

                if (CurrentUser->IsExpert())
                    {
                    break;
                    }
                }

            case -2:
                {
                if (ich == '?')
                    {
                    oChar('?');
                    }

                SetDoWhat(KNOWNMENU);

                showMenu(M_KNOWN);
                done = TRUE;
                break;
                }
            }
        } while (!done && moreYet);

    UnlockMenu(MENU_KNOWN);
    }


// --------------------------------------------------------------------------
// doNext(): Handles the '+' for next room.

void TERMWINDOWMEMBER doNext()
    {
    mPrintf(getmsg(321), cfg.Lroom_nym);
    stepRoom(1);
    }


// --------------------------------------------------------------------------
// doPrevious(): Handles the '-' for previous room.

void TERMWINDOWMEMBER doPrevious()
    {
    mPrintf(getmsg(322), cfg.Lroom_nym);
    stepRoom(0);
    }


// --------------------------------------------------------------------------
// doNextHall(): Handles the '>' for next hall.

void TERMWINDOWMEMBER doNextHall()
    {
    mPrintf(getmsg(321), cfg.Lhall_nym);
    stephall(1);
    }


// --------------------------------------------------------------------------
// doPreviousHall(): Handles the '<' for previous hall.

void TERMWINDOWMEMBER doPreviousHall()
    {
    mPrintf(getmsg(322), cfg.Lhall_nym);
    stephall(0);
    }


// --------------------------------------------------------------------------
//
// Notes:
//  Assumes MENU_PERSONAL is locked.

void TERMWINDOWMEMBER ListRoomsNotInPersonalHall(void)
    {
    Bool *AccessArray = new Bool[cfg.maxrooms];

    if (AccessArray)
        {
        theOtherAlgorithm(AccessArray, thisHall, FALSE);
        }
    else
        {
        OutOfMemory(33);
        return;
        }

    CRmPrintfCR(getmenumsg(MENU_PERSONAL, 15), cfg.Urooms_nym, cfg.Lhall_nym);

    prtList(LIST_START);

    for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
        {
        if (AccessArray[RoomPosTableToRoom(RoomSlot)] && CurrentUser->CanAccessRoom(RoomPosTableToRoom(RoomSlot)) &&
                !CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom(RoomSlot)))
            {
            label Buffer;
            prtList(RoomTab[RoomPosTableToRoom(RoomSlot)].GetName(Buffer, sizeof(Buffer)));
            }
        }

    prtList(LIST_END);

    delete [] AccessArray;
    }


// --------------------------------------------------------------------------
//
// Notes:
//  Assumes MENU_PERSONAL is locked.

void TERMWINDOWMEMBER ListRoomsInPersonalHall(void)
    {
    CRmPrintfCR(getmenumsg(MENU_PERSONAL, 16), cfg.Urooms_nym, cfg.Lhall_nym);

    prtList(LIST_START);

    for (r_slot RoomSlot = 0; RoomSlot < cfg.maxrooms; RoomSlot++)
        {
        if (CurrentUser->CanAccessRoom(RoomPosTableToRoom(RoomSlot)) &&
                CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom(RoomSlot)))
            {
            label Buffer;
            prtList(RoomTab[RoomPosTableToRoom(RoomSlot)].GetName(Buffer, sizeof(Buffer)));
            }
        }

    prtList(LIST_END);
    }


// --------------------------------------------------------------------------
// doPersonal(): Handles .P(ersonal hall) command.

void TERMWINDOWMEMBER doPersonal(void)
    {
    if (!LockMenu(MENU_PERSONAL))
        {
        return;
        }

    SetDoWhat(DOPERSONAL);

    MRO.Reverse = FALSE;

    mPrintf(getmenumsg(MENU_PERSONAL, 12), cfg.Lhall_nym);

    if (!cfg.PersonalHallOK)
        {
        CRCRmPrintfCR(getmenumsg(MENU_PERSONAL, 13), cfg.Lhalls_nym);
        UnlockMenu(MENU_PERSONAL);
        return;
        }

    const int ich = iCharNE();

    switch (DoMenu(MENU_PERSONAL, ich))
        {
        case 1: // Add
            {
            SetDoWhat(PERSONALADD);

            if (LockMenu(MENU_PERSONALADD))
                {
                const int ich = iCharNE();

                switch (DoMenu(MENU_PERSONALADD, ich))
                    {
                    case 1: // All
                        {
                        Bool add = FALSE;

                        doCR();

                        SetDoWhat(PERSONALADDALL);

                        char Prompt[128];
                        sprintf(Prompt, getmenumsg(MENU_PERSONALADD, 3),
                                cfg.Lrooms_nym, cfg.Lhall_nym);
                        if (getYesNo(Prompt, 0))
                            {
                            for (r_slot rm = 0; rm < cfg.maxrooms; rm++)
                                {
                                if (theAlgorithm(RoomPosTableToRoom(rm), thisHall, FALSE) &&
                                        !CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom(rm)))
                                    {
                                    char string[LABELSIZE+LABELSIZE];

                                    if (!add)
                                        {
                                        CRmPrintf(getmenumsg(MENU_PERSONALADD, 4), cfg.Lrooms_nym);
                                        prtList(LIST_START);
                                        }

                                    MakeRoomName(RoomPosTableToRoom(rm), string);
                                    prtList(string);
                                    add = TRUE;
                                    CurrentUser->SetRoomInPersonalHall(RoomPosTableToRoom(rm), TRUE);
                                    }
                                }

                            if (add)
                                {
                                prtList(LIST_END);
                                }
                            else
                                {
                                mPrintfCR(getmenumsg(MENU_PERSONALADD, 5), ns, cfg.Lrooms_nym, cfg.Lhall_nym);
                                }
                            }

                        break;
                        }

                    case 2: // Local
                        {
                        Bool add = FALSE;

                        doCR();

                        SetDoWhat(PERSONALADDLOCAL);

                        char Prompt[128];
                        sprintf(Prompt, getmenumsg(MENU_PERSONALADD, 7), cfg.Lrooms_nym, cfg.Lhall_nym);

                        if (getYesNo(Prompt, 0))
                            {
                            for (r_slot rm = 0; rm < cfg.maxrooms; rm++)
                                {
                                if (theAlgorithm(RoomPosTableToRoom(rm), thisHall, FALSE) &&
                                        !CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom(rm)) &&
                                        !RoomTab[RoomPosTableToRoom(rm)].IsShared())
                                    {
                                    char string[LABELSIZE + LABELSIZE];

                                    if (!add)
                                        {
                                        CRmPrintf(getmenumsg(MENU_PERSONALADD, 4), cfg.Lrooms_nym);
                                        prtList(LIST_START);
                                        }

                                    MakeRoomName(RoomPosTableToRoom(rm), string);
                                    prtList(string);
                                    add = TRUE;
                                    CurrentUser->SetRoomInPersonalHall(RoomPosTableToRoom(rm), TRUE);
                                    }
                                }

                            if (add)
                                {
                                prtList(LIST_END);
                                }
                            else
                                {
                                mPrintfCR(getmenumsg(MENU_PERSONALADD, 5), getmenumsg(MENU_PERSONALADD, 6),
                                        cfg.Lrooms_nym, cfg.Lhall_nym);
                                }
                            }

                        break;
                        }

                    case 3: // Room
                        {
                        label roomname;

                        doCR();

                        SetDoWhat(PERSONALADDROOM);

                        do
                            {
                            label Buffer;
                            getString(cfg.Lroom_nym, roomname, LABELSIZE, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));

                            if (*roomname == '?')
                                {
                                ListRoomsNotInPersonalHall();
                                }
                            } while (*roomname == '?');

                        r_slot rm = RoomExists(roomname);

                        if (rm == CERROR)
                            {
                            rm = PartialRoomExists(roomname, 0, TRUE);
                            }

                        if (rm == CERROR || !(*roomname) || !theAlgorithm(rm, thisHall, FALSE))
                            {
                            CRmPrintfCR(getmsg(368), cfg.Lroom_nym, roomname);
                            }
                        else if (!CurrentUser->IsRoomInPersonalHall(rm))
                            {
                            label Buffer;
                            CurrentUser->SetRoomInPersonalHall(rm, TRUE);
                            CRmPrintfCR(getmenumsg(MENU_PERSONALADD, 8), cfg.Uroom_nym,
                                    RoomTab[rm].GetName(Buffer, sizeof(Buffer)), cfg.Lhall_nym);
                            }
                        else
                            {
                            label Buffer;
                            CRmPrintfCR(getmenumsg(MENU_PERSONALADD, 9), cfg.Uroom_nym,
                                    RoomTab[rm].GetName(Buffer, sizeof(Buffer)), cfg.Lhall_nym);
                            }

                        break;
                        }

                    case -1:
                        {
                        break;
                        }

                    default:
                        {
                        oChar((char) ich);
                        mPrintf(sqst);
                        if (CurrentUser->IsExpert())
                            {
                            break;
                            }
                        }

                    case -2:
                        {
                        if (ich == '?')
                            {
                            oChar('?');
                            }

                        SetDoWhat(PERSONALADDMENU);

                        showMenu(M_PERSONALADD);
                        break;
                        }
                    }

                UnlockMenu(MENU_PERSONALADD);
                }

            break;
            }

        case 2: // Clear
            {
            char Prompt[128];

            doCR();

            SetDoWhat(PERSONALCLEAR);

            sprintf(Prompt, getmenumsg(MENU_PERSONAL, 5), cfg.Lrooms_nym, cfg.Lhall_nym);
            if (getYesNo(Prompt, 0))
                {
                for (r_slot rm = 0; rm < cfg.maxrooms; rm++)
                    {
                    CurrentUser->SetRoomInPersonalHall(rm, FALSE);
                    }
                }

            break;
            }

        case 3: // List rooms
            {
            Bool rooms = FALSE;

            SetDoWhat(PERSONALLST);

            doCR(2);

            OC.SetOutFlag(OUTOK);

            for (r_slot rm = 0; rm < cfg.maxrooms; rm++)
                {
                if (CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom(rm)))
                    {
                    if (CurrentUser->CanAccessRoom(RoomPosTableToRoom(rm)))
                        {
                        char string[LABELSIZE+LABELSIZE];

                        if (!rooms)
                            {
                            mPrintfCR(getmenumsg(MENU_PERSONAL, 6), cfg.Urooms_nym, cfg.Lhall_nym);
                            prtList(LIST_START);
                            }

                        MakeRoomName(RoomPosTableToRoom(rm), string);
                        prtList(string);
                        rooms = TRUE;
                        }
                    }
                }

            if (rooms)
                {
                prtList(LIST_END);
                }
            else
                {
                mPrintfCR(getmenumsg(MENU_PERSONAL, 7), cfg.Lrooms_nym, cfg.Lhall_nym);
                }

            break;
            }

        case 4: // Remove room
            {
            label roomname;

            SetDoWhat(PERSONALRMV);

            doCR();

            do
                {
                label Buffer;
                getString(cfg.Lroom_nym, roomname, LABELSIZE, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));

                if (*roomname == '?')
                    {
                    ListRoomsInPersonalHall();
                    }
                } while (*roomname == '?');

            r_slot rm = RoomExists(roomname);

            if (rm == CERROR)
                {
                rm = PartialRoomExists(roomname, 0, TRUE);
                }

            if (rm == CERROR || !(*roomname) || !theAlgorithm(rm, thisHall, FALSE))
                {
                if (*roomname)
                    {
                    CRmPrintfCR(getmsg(368), cfg.Lroom_nym, roomname);
                    }
                }
            else if (CurrentUser->IsRoomInPersonalHall(rm))
                {
                label Buffer;
                CurrentUser->SetRoomInPersonalHall(rm, FALSE);
                CRmPrintfCR(getmenumsg(MENU_PERSONAL, 8), cfg.Uroom_nym, RoomTab[rm].GetName(Buffer, sizeof(Buffer)),
                        cfg.Lhall_nym);
                }
            else
                {
                label Buffer;
                CRmPrintfCR(getmenumsg(MENU_PERSONAL, 9), cfg.Lroom_nym, RoomTab[rm].GetName(Buffer, sizeof(Buffer)),
                        cfg.Lhall_nym);
                }

            break;
            }

        case 5: // Use
            {
            SetDoWhat(PERSONALUSE);

            doCR();

            CurrentUser->SetUsePersonalHall(!CurrentUser->IsUsePersonalHall());

            if (CurrentUser->IsUsePersonalHall())
                {
                CRmPrintfCR(getmenumsg(MENU_PERSONAL, 10), cfg.Lhall_nym);
                }
            else
                {
                CRmPrintfCR(getmenumsg(MENU_PERSONAL, 11), cfg.Lhall_nym);
                }

            break;
            }

        case -1:
            {
            break;
            }

        default:
            {
            oChar((char) ich);
            mPrintf(sqst);
            if (CurrentUser->IsExpert())
                {
                break;
                }
            }

        case -2:
            {
            if (ich == '?')
                {
                oChar('?');
                }

            SetDoWhat(PERSONALMENU);

            showMenu(M_PERSONAL);
            break;
            }
        }

    UnlockMenu(MENU_PERSONAL);
    }
