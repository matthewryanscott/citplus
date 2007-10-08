// --------------------------------------------------------------------------
// Citadel: RoomEdit.CPP
//
// Room editing stuff.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "boolexpr.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#include "libovl.h"
#include "net.h"
#include "cfgfiles.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// AideEditRoom()       aide special to edit rooms
// killempties()        deletes empty rooms
// killroom()           aide fn to kill current room


// --------------------------------------------------------------------------
// inExternal(): Returns wether an entry is in EXTERNAL.CIT.


Bool TERMWINDOWMEMBER inExternal(const char *key, const char *str)
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    char path[80];
    int len = strlen(key);

    sprintf(path, sbs, cfg.homepath, citfiles[C_EXTERNAL_CIT]);

if (
(
fBuf
                            =
                                fopen(
            path, FO_R)



                ) ==



                                        NULL



                                                                                    )   /*
    {
                                ASCII
     }               mode */
  { { {/*{*/{{{ {{{ {                                                                                                                                               {
  {{
  {{
   { {  {
  {{          mPrintf(getmsg(                                                                                    15)            ,                  citfiles[C_EXTERNAL_CIT]    /*externalCit*/                                                                                         /*"Cannot find external.cit!"*/);   return FALSE;} }
   }} }  }        }} }
   }} }}} }}}  }}
   }

    while (fgets(line, 90, fBuf) != NULL)
    {
        if (line[0] != '#')
            {
            continue;
            }

        if (strnicmp(line, key, len) != SAMESTRING)
            {
            continue;
            }

        parse_it(words, line);

        if (SameString(words[0], key))
        {
            if (u_match(str, words[1]))
            {
                fclose(fBuf);
                return (TRUE);
            }
        }
    }
    fclose(fBuf);
    return (FALSE);
}


// --------------------------------------------------------------------------
// AideEditRoom(): Is aide special fn.
//
// Notes:
//  This is really gross in that it uses CurrentRoom for the editing. This
//      ought to be changed at some time.

Bool TERMWINDOWMEMBER AideEditRoom(void)
    {
    label roomname, oldname, Buffer;
    Bool waspublic;
    Bool prtMess = !CurrentUser->IsExpert();
    Bool quit = FALSE;

    SetDoWhat(EDITROOM);

    const r_slot oldthisRoom = thisRoom;

    doCR();

    if (!AskRoomName(roomname, thisRoom))
        {
        return (FALSE);
        }

    r_slot roomslot = RoomExists(roomname);
    if (roomslot == CERROR)
        {
        roomslot = PartialRoomExists(roomname, oldthisRoom, TRUE);
        }

    if (roomslot == CERROR)
        {
        CRmPrintfCR(getmsg(368), cfg.Lroom_nym, roomname);
        return (FALSE);
        }

    if (!LockMenu(MENU_AIDE))
        {
        return (FALSE);
        }

    thisRoom = roomslot;
    CurrentRoom->Load(thisRoom);

    CurrentRoom->GetName(oldname, sizeof(oldname));

    if (!CurrentRoom->IsMsDOSdir())
        {
        CurrentRoom->SetDirectory(ns);
        }

    if (CurrentRoom->IsPublic() && !CurrentRoom->IsBIO())
        {
        waspublic = TRUE;
        }
    else
        {
        waspublic = FALSE;
        }

    do
        {
        if (prtMess)
            {
            doCR();
            OC.SetOutFlag(OUTOK);

            char Buffer[80];

            mPrintfCR(getmenumsg(MENU_AIDE, 29), CurrentRoom->GetName(Buffer, sizeof(Buffer)));
            mPrintfCR(getmenumsg(MENU_AIDE, 30), CurrentRoom->GetInfoLine(Buffer, sizeof(Buffer)));
            mPrintfCR(getmenumsg(MENU_AIDE, 31), CurrentRoom->IsMsDOSdir() ?
					CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)) : getmenumsg(MENU_AIDE, 32));
            mPrintfCR(getmenumsg(MENU_AIDE, 33), CurrentRoom->IsApplication() ?
                    CurrentRoom->GetApplication(Buffer, sizeof(Buffer)) : getmenumsg(MENU_AIDE, 32));

            if (CurrentRoom->IsApplication())
                {
                displayYesNo(getmenumsg(MENU_AIDE, 34), CurrentRoom->IsAutoApp());
                doCR();
                }

            CurrentRoom->GetDescFile(Buffer, sizeof(Buffer));
            mPrintfCR(getmenumsg(MENU_AIDE, 35), *Buffer ? Buffer : getmenumsg(MENU_AIDE, 32));

            if (CurrentRoom->IsGroupOnly() && CurrentRoom->IsBooleanGroup())
                {
                mPrintf(getmenumsg(MENU_AIDE, 36), ns);
                BoolExpr Buffer;

                ShowBooleanExpression(CurrentRoom->GetGroupExpression(Buffer), ShowGroup);
                }
            else
                {
                mPrintf(getmenumsg(MENU_AIDE, 36), CurrentRoom->IsGroupOnly() ?
                        GroupData[CurrentRoom->GetGroup()].GetName(Buffer, sizeof(Buffer)) : getmenumsg(MENU_AIDE, 32));
                }

            CRmPrintfCR(getmenumsg(MENU_AIDE, 37), CurrentRoom->IsPrivilegedGroup() ?
                    GroupData[CurrentRoom->GetPGroup()].GetName(Buffer, sizeof(Buffer)) : getmenumsg(MENU_AIDE, 32));

            if (CurrentRoom->IsPrivilegedGroup())
                {
                displayYesNo(getmenumsg(MENU_AIDE, 38), CurrentRoom->IsDownloadOnly());
                doCR();

                displayYesNo(getmenumsg(MENU_AIDE, 39), CurrentRoom->IsUploadOnly());
                doCR();

                displayYesNo(getmenumsg(MENU_AIDE, 40), CurrentRoom->IsReadOnly());
                doCR();

                displayYesNo(getmenumsg(MENU_AIDE, 41), CurrentRoom->IsGroupModerates());
                doCR();
                }

            displayYesNo(getmenumsg(MENU_AIDE, 42), !CurrentRoom->IsPublic());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 43), CurrentRoom->IsBIO());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 44), CurrentRoom->IsAnonymous());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 45), CurrentRoom->IsModerated());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 46), CurrentRoom->IsShared());
            doCR();

            if (CurrentRoom->IsShared())
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 47), CurrentRoom->GetNetID(Buffer, sizeof(Buffer)));
                }

            displayYesNo(getmenumsg(MENU_AIDE, 48), CurrentRoom->IsPermanent());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 49), CurrentRoom->IsSubject());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 50), CurrentRoom->IsArchive());
            doCR();

            displayYesNo(getmenumsg(MENU_AIDE, 51), CurrentRoom->IsUnexcludable());
            doCR();

            mPrintfCR(getmenumsg(MENU_AIDE, 52), CurrentRoom->GetDictionary(Buffer, sizeof(Buffer)));

            CRmPrintfCR(getmsg(633));

            prtMess = !CurrentUser->IsExpert();
            }

        const int c = DoMenuPrompt(getmsg(616), NULL);

        if (!(HaveConnectionToUser()))
            {
            thisRoom = oldthisRoom;
            CurrentRoom->Load(thisRoom);
            UnlockMenu(MENU_AIDE);
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
                    thisRoom = oldthisRoom;
                    CurrentRoom->Load(thisRoom);
                    UnlockMenu(MENU_AIDE);
                    return (FALSE);
                    }

                break;
                }

            case 'B':
                {
                if ((thisRoom > 2) || (thisRoom >= 0 && CurrentUser->IsSysop()))
                    {
                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 53), cfg.Lroom_nym);

                    CurrentRoom->SetBIO(changeYesNo(Prompt, CurrentRoom->IsBIO()));
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 54), cfg.Lroom_nym);

                    label Buffer1, Buffer2;
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 55), RoomTab[LOBBY].GetName(Buffer, sizeof(Buffer)),
                            RoomTab[MAILROOM].GetName(Buffer1, sizeof(Buffer1)),
							RoomTab[AIDEROOM].GetName(Buffer2, sizeof(Buffer2)), getmenumsg(MENU_AIDE, 56));
                    }

                break;
                }

            case 'C':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 57));

                if (CurrentUser->IsSuperSysop() || (CurrentUser->IsSysop() && onConsole))
                    {
                    char Dictionary[13];

                    if (getString(getmenumsg(MENU_AIDE, 58), Dictionary, 12, FALSE,
							CurrentRoom->GetDictionary(Buffer, sizeof(Buffer))))
                        {
                        normalizeString(Dictionary);
                        CurrentRoom->SetDictionary(Dictionary);
                        }
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 59), getmenumsg(MENU_AIDE, 57));
                    }

                break;
                }

            case 'D':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 60));

                if (CurrentUser->IsSysop())
                    {
                    Bool WasDirectory = CurrentRoom->IsMsDOSdir();

                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 61), cfg.Lroom_nym);

                    if (getYesNo(Prompt, WasDirectory))
                        {
                        char pathname[64], Buffer[64];

                        doCR();

                        CurrentRoom->GetDirectory(Buffer, sizeof(Buffer));

                        if (!*Buffer)
                            {
                            mPrintfCR(getmenumsg(MENU_AIDE, 62));
                            }
                        else
                            {
                            mPrintfCR(getmenumsg(MENU_AIDE, 63), Buffer);
                            }

                        getString(getmenumsg(MENU_AIDE, 64), pathname, 63, FALSE, *Buffer ? Buffer : cfg.dirpath);

                        if (*pathname)
                            {
                            if (strncmp(pathname, getmenumsg(MENU_AIDE, 65), 2) == SAMESTRING)
                                {
                                char Temp[64];

                                strcpy(Temp, pathname + 1);

                                CurrentRoom->GetDirectory(Buffer, sizeof(Buffer));
                                strcpy(pathname, *Buffer ? Buffer : cfg.dirpath);

                                strncat(pathname, Temp, 63 - strlen(pathname));
                                }

                            if (pathname[strlen(pathname) - 1] == '\\')
                                {
                                pathname[strlen(pathname) - 1] = 0;
                                }

                            strupr(pathname);

                            doCR();

                            if (!CurrentUser->IsSuperSysop() && !onConsole && inExternal(getmsg(341), pathname))
                                {
                                securityViolation(pathname);
                                thisRoom = oldthisRoom;
                                CurrentRoom->Load(thisRoom);
                                UnlockMenu(MENU_AIDE);
                                return (FALSE);
                                }

                            if (changedir(pathname))
                                {
                                CurrentRoom->SetMsDOSdir(TRUE);
                                CurrentRoom->SetDirectory(pathname);

                                mPrintfCR(getmenumsg(MENU_AIDE, 63), CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)));
                                }
                            else
                                {
                                mPrintfCR(getmenumsg(MENU_AIDE, 66), pathname);

                                if (getYesNo(getmenumsg(MENU_AIDE, 67), 0))
                                    {
                                    if (mkdir(pathname) == -1)
                                        {
                                        CRmPrintfCR(getmenumsg(MENU_AIDE, 68), pathname);
                                        }
                                    else
                                        {
                                        CurrentRoom->SetDirectory(pathname);
                                        CurrentRoom->SetMsDOSdir(TRUE);

                                        CRmPrintfCR(getmenumsg(MENU_AIDE, 63),
												CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)));
                                        }
                                    }
                                }

                            if (!WasDirectory && CurrentRoom->IsPrivilegedGroup() && CurrentRoom->IsMsDOSdir())
                                {
                                CurrentRoom->SetDownloadOnly(getYesNo(getmenumsg(MENU_AIDE, 69),
										CurrentRoom->IsDownloadOnly()));

                                if (!CurrentRoom->IsDownloadOnly())
                                    {
                                    CurrentRoom->SetUploadOnly(getYesNo(getmenumsg(MENU_AIDE, 70),
                                            CurrentRoom->IsUploadOnly()));
                                    }
                                }
                            }
                        }
                    else
                        {
                        CurrentRoom->SetMsDOSdir(FALSE);
                        CurrentRoom->SetDownloadOnly(FALSE);
                        CurrentRoom->SetUploadOnly(FALSE);
                        }

                    changedir(cfg.homepath);
                    }
                else
                    {
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 71));
                    }

                break;
                }

            case 'E':
                {
                if (CurrentUser->IsSysop())
                    {
                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 72), cfg.Lroom_nym);

                    CurrentRoom->SetShared(changeYesNo(Prompt, CurrentRoom->IsShared()));

                    if (CurrentRoom->IsShared() && !*CurrentRoom->GetNetID(Buffer, sizeof(Buffer)))
                        {
                        label NetID1, NetID2;

                        CurrentRoom->GetName(NetID1, sizeof(NetID1));
                        normalizeString(NetID1);
                        stripansi(NetID1);
                        strip(NetID1, '"');

                        r_slot roomslot = IdExists(NetID1, FALSE);

                        if (roomslot != CERROR)
                            {
                            NetID1[0] = 0;
                            }

                        do
                            {
                            getString(getmenumsg(MENU_AIDE, 73), NetID2, LABELSIZE, NetID1);

                            if (*NetID2 == '?')
                                {
                                ListAllRooms(FALSE, TRUE, TRUE);
                                }
                            } while (*NetID2 == '?');

                        normalizeString(NetID2);
                        stripansi(NetID2);
                        strip(NetID2, '"');

                        roomslot = IdExists(NetID2, FALSE);

                        if (roomslot != CERROR && (roomslot != thisRoom) && *NetID2)
                            {
                            mPrintfCR(getmenumsg(MENU_AIDE, 74), NetID2);
                            }
                        else
                            {
                            CurrentRoom->SetNetID(NetID2);
                            }
                        }
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 75), cfg.Lroom_nym);
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 76), cfg.Lrooms_nym);
                    }

                break;
                }

            case 'F':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 77));

                if (CurrentUser->IsSysop())
                    {
                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 78), cfg.Lroom_nym);

                    if (getYesNo(Prompt, *CurrentRoom->GetDescFile(Buffer, sizeof(Buffer)) != 0))
                        {
                        char description[13];

                        getString(getmenumsg(MENU_AIDE, 79), description, 12, FALSE,
								CurrentRoom->GetDescFile(Buffer, sizeof(Buffer)));

                        CurrentRoom->SetDescFile(description);
                        }
                    else
                        {
                        CurrentRoom->SetDescFile(ns);
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 80), cfg.Lroom_nym);
                    }

                break;
                }

            case 'G':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 81), cfg.Lgroup_nym);

                if ((thisRoom > 2) || (thisRoom > 0 && CurrentUser->IsSysop()))
                    {
                    if (CurrentRoom->IsGroupOnly() && CurrentRoom->IsBooleanGroup())
                        {
                        CRmPrintf(getmenumsg(MENU_AIDE, 82));
                        BoolExpr Buffer;

                        ShowBooleanExpression(CurrentRoom->GetGroupExpression(Buffer), ShowGroup);
                        doCR();
                        }

                    char groupname[256], prompt[128];

                    sprintf(prompt, getmsg(548), cfg.Lgroup_nym, cfg.Lroom_nym);

                    do
                        {
                        getString(prompt, groupname, 255, CurrentRoom->IsGroupOnly() && !CurrentRoom->IsBooleanGroup() ?
                                GroupData[CurrentRoom->GetGroup()].GetName(Buffer, sizeof(Buffer)) : ns);

                        if (*groupname == '?')
                            {
                            ListGroups(TRUE);
                            }
                        } while (*groupname == '?');

                    if (SameString(groupname, spc))
                        {
                        CurrentRoom->SetGroupOnly(FALSE);
                        CurrentRoom->SetBooleanGroup(FALSE);
                        }
                    else
                        {
                        const g_slot groupslot = FindGroupByPartialName(groupname, FALSE);

                        if (*groupname && (groupslot != CERROR))
                            {
                            CurrentRoom->SetGroupOnly(TRUE);
                            CurrentRoom->SetBooleanGroup(FALSE);
                            CurrentRoom->SetGroup(groupslot);
                            }
                        else
                            {
                            if (*groupname)
                                {
                                BoolExpr Expression;

                                if (!CreateBooleanExpression(groupname,
#ifdef WINCIT
                                        TestGroupExists, Expression, this))
#else
                                        TestGroupExists, Expression))
#endif
                                    {
                                    CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
                                    }
                                else
                                    {
                                    CurrentRoom->SetGroupExpression(Expression);

                                    CurrentRoom->SetGroupOnly(TRUE);
                                    CurrentRoom->SetBooleanGroup(TRUE);
                                    }
                                }
                            }
                        }
                    }
                else
                    {
                    if (thisRoom > 0)
                        {
                        label Buffer1;

                        CRmPrintfCR(getmenumsg(MENU_AIDE, 83), cfg.Lgroup_nym,
                                RoomTab[MAILROOM].GetName(Buffer, sizeof(Buffer)),
                                RoomTab[AIDEROOM].GetName(Buffer1, sizeof(Buffer1)));
                        }
                    else
                        {
                        CRmPrintfCR(getmenumsg(MENU_AIDE, 84), RoomTab[LOBBY].GetName(Buffer, sizeof(Buffer)),
								cfg.Lgroup_nym);
                        }
                    }

                break;
                }

            case 'H':
                {
                if ((thisRoom > 2) || (thisRoom >= 0 && CurrentUser->IsSysop()))
                    {
                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 85), cfg.Lroom_nym);

                    CurrentRoom->SetPublic(!changeYesNo(Prompt, !CurrentRoom->IsPublic()));
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 86), cfg.Lroom_nym);

                    label Buffer1, Buffer2;

                    CRmPrintfCR(getmenumsg(MENU_AIDE, 55), RoomTab[LOBBY].GetName(Buffer, sizeof(Buffer)),
                            RoomTab[MAILROOM].GetName(Buffer1, sizeof(Buffer1)),
                            RoomTab[AIDEROOM].GetName(Buffer2, sizeof(Buffer2)), getmenumsg(MENU_AIDE, 87));
                    }

                break;
                }

            case 'I':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 88));

                char Prompt[128];
                sprintf(Prompt, getmenumsg(MENU_AIDE, 89), cfg.Lroom_nym);

                char InfoLine[80];
                if (getNormStr(Prompt, InfoLine, 79))
                    {
                    CurrentRoom->SetInfoLine(InfoLine);
                    }

                break;
                }

            case 'L':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 90));

                if (CurrentUser->IsSuperSysop() || (CurrentUser->IsSysop() && onConsole))
                    {
                    if (getYesNo(getmenumsg(MENU_AIDE, 90), CurrentRoom->IsApplication()))
                        {
                        char application[13];

                        getString(getmenumsg(MENU_AIDE, 91), application, 12, FALSE,
								CurrentRoom->GetApplication(Buffer, sizeof(Buffer)));

                        CurrentRoom->SetApplication(application);
                        CurrentRoom->SetApplication(TRUE);
                        }
                    else
                        {
                        CurrentRoom->SetApplication(ns);
                        CurrentRoom->SetApplication((Bool) FALSE);
                        }
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 59), getmenumsg(MENU_AIDE, 90));
                    }

                break;
                }

            case 'M':
                {
                if (CurrentUser->IsSysop())
                    {
                    CurrentRoom->SetModerated(changeYesNo(getmenumsg(MENU_AIDE, 92), CurrentRoom->IsModerated()));
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 93));
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 94), cfg.Lrooms_nym);
                    }

                break;
                }

            case 'N':
                {
                mPrintfCR(getmsg(248));

                char Prompt[128];
                sprintf(Prompt, getmsg(551), cfg.Lroom_nym);

                label roomname;
                AskRoomName(roomname, thisRoom, Prompt);

                const r_slot roomslot = RoomExists(roomname);

                if (roomslot >= 0 && roomslot != thisRoom)
                    {
                    mPrintf(getmsg(105), roomname, cfg.Lroom_nym);
                    }
                else
                    {
                    CurrentRoom->SetName(roomname);
                    }

                break;
                }

            case 'O':
                {
                if (CurrentRoom->IsApplication())
                    {
                    char Prompt[128];
                    sprintf(Prompt, getmenumsg(MENU_AIDE, 95), cfg.Lroom_nym);

                    CurrentRoom->SetAutoApp(changeYesNo(Prompt, CurrentRoom->IsAutoApp()));
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 96), cfg.Lroom_nym);
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 97), cfg.Uroom_nym);
                    }

                break;
                }

            case 'P':
                {
                if (thisRoom > DUMP)
                    {
                    if (!CurrentRoom->IsMsDOSdir())
                        {
                        CurrentRoom->SetPermanent(changeYesNo(getmenumsg(MENU_AIDE, 98), CurrentRoom->IsPermanent()));
                        }
                    else
                        {
                        CurrentRoom->SetPermanent(TRUE);
                        mPrintfCR(getmenumsg(MENU_AIDE, 99));
                        CRmPrintfCR(getmenumsg(MENU_AIDE, 100), cfg.Lrooms_nym);
                        }
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 99));

                    CurrentRoom->SetPermanent(TRUE);

                    label Buffer1, Buffer2, Buffer3;

                    CRmPrintfCR(getmenumsg(MENU_AIDE, 101), RoomTab[LOBBY].GetName(Buffer, sizeof(Buffer)),
                            RoomTab[MAILROOM].GetName(Buffer1, sizeof(Buffer1)),
                            RoomTab[AIDEROOM].GetName(Buffer2, sizeof(Buffer2)),
                            RoomTab[DUMP].GetName(Buffer3, sizeof(Buffer3)));
                    }

                break;
                }

            case 'R':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 102));

                if (CurrentUser->IsSuperSysop() || (CurrentUser->IsSysop() && onConsole))
                    {
                    if (getYesNo(getmenumsg(MENU_AIDE, 102), CurrentRoom->IsArchive()))
                        {
                        char archive[13];

                        getString(getmenumsg(MENU_AIDE, 103), archive, 12, FALSE,
								CurrentRoom->GetArchiveFile(Buffer, sizeof(Buffer)));

                        normalizeString(archive);

                        CurrentRoom->SetArchiveFile(archive);
                        CurrentRoom->SetArchive(*archive);
                        }
                    else
                        {
                        CurrentRoom->SetArchiveFile(ns);
                        CurrentRoom->SetArchive(FALSE);
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 59), getmenumsg(MENU_AIDE, 102));
                    }

                break;
                }

            case 'S':
                {
                mPrintfCR(getmsg(652));

                if (getYesNo(getmsg(652), FALSE))
                    {
                    quit = TRUE;
                    }

                break;
                }

            case 'U':
                {
                char Prompt[128];
                sprintf(Prompt, getmenumsg(MENU_AIDE, 104), cfg.Lroom_nym);

                CurrentRoom->SetSubject(changeYesNo(Prompt, CurrentRoom->IsSubject()));
                break;
                }

            case 'V':
                {
                char prompt[128];
                label groupname;

                mPrintfCR(getmenumsg(MENU_AIDE, 105), cfg.Lgroup_nym);

                sprintf(prompt, getmsg(548), cfg.Lgroup_nym, cfg.Lroom_nym);

                do
                    {
                    getString(prompt, groupname, LABELSIZE, CurrentRoom->IsPrivilegedGroup() ?
                            GroupData[CurrentRoom->GetPGroup()].GetName(Buffer, sizeof(Buffer)) : ns);

                    if (*groupname == '?')
                        {
                        ListGroups(TRUE);
                        }
                    } while (*groupname == '?');

                if (SameString(groupname, spc))
                    {
                    CurrentRoom->SetPrivilegedGroup(FALSE);
                    CurrentRoom->SetReadOnly(FALSE);
                    CurrentRoom->SetDownloadOnly(FALSE);
                    CurrentRoom->SetUploadOnly(FALSE);
                    CurrentRoom->SetGroupModerates(FALSE);
                    }
                else
                    {
                    const g_slot groupslot = FindGroupByPartialName(groupname, FALSE);

                    if (*groupname && (groupslot != CERROR))
                        {
                        CurrentRoom->SetPrivilegedGroup(TRUE);
                        CurrentRoom->SetPGroup(groupslot);

                        CurrentRoom->SetReadOnly(getYesNo(getmenumsg(MENU_AIDE, 25), CurrentRoom->IsReadOnly()));

                        char prompt[128];

                        sprintf(prompt, getmenumsg(MENU_AIDE, 106), cfg.Ugroup_nym);

                        CurrentRoom->SetGroupModerates(getYesNo(prompt, CurrentRoom->IsGroupModerates()));

                        if (CurrentRoom->IsMsDOSdir())
                            {
                            CurrentRoom->SetDownloadOnly(getYesNo(getmenumsg(MENU_AIDE, 69), CurrentRoom->IsDownloadOnly()));

                            if (!CurrentRoom->IsDownloadOnly())
                                {
                                CurrentRoom->SetUploadOnly(getYesNo(getmenumsg(MENU_AIDE, 70), CurrentRoom->IsUploadOnly()));
                                }
                            else
                                {
                                CurrentRoom->SetUploadOnly(FALSE);
                                }
                            }
                        }
                    else
                        {
                        if (*groupname)
                            {
                            CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
                            }
                        }
                    }

                break;
                }

            case 'W':
                {
                mPrintfCR(getmenumsg(MENU_AIDE, 107));

                if (CurrentUser->IsSysop())
                    {
                    label NetID;

                    do
                        {
                        getString(getmenumsg(MENU_AIDE, 73), NetID, LABELSIZE,
								CurrentRoom->GetNetID(Buffer, sizeof(Buffer)));

                        if (*NetID == '?')
                            {
                            ListAllRooms(FALSE, TRUE, TRUE);
                            }
                        } while (*NetID == '?');

                    normalizeString(NetID);
                    stripansi(NetID);
                    strip(NetID, '"');

                    r_slot roomslot = IdExists(NetID, FALSE);

                    if (roomslot != CERROR && (roomslot != thisRoom) && *NetID)
                        {
                        mPrintfCR(getmenumsg(MENU_AIDE, 74), NetID);
                        }
                    else
                        {
                        CurrentRoom->SetNetID(NetID);
                        }
                    }
                else
                    {
                    CRmPrintfCR(getmenumsg(MENU_AIDE, 108));
                    }

                break;
                }

            case 'X':
                {
                CurrentRoom->SetUnexcludable(changeYesNo(getmenumsg(MENU_AIDE, 109), CurrentRoom->IsUnexcludable()));
                break;
                }

            case 'Y':
                {
                if ((thisRoom > 2) || (thisRoom >= 0 && CurrentUser->IsSysop()))
                    {
                    CurrentRoom->SetAnonymous(changeYesNo(getmenumsg(MENU_AIDE, 110), CurrentRoom->IsAnonymous()));
                    }
                else
                    {
                    mPrintfCR(getmenumsg(MENU_AIDE, 111));

                    label Buffer1, Buffer2;

                    CRmPrintfCR(getmenumsg(MENU_AIDE, 55), RoomTab[LOBBY].GetName(Buffer, sizeof(Buffer)),
                            RoomTab[MAILROOM].GetName(Buffer1, sizeof(Buffer1)),
                            RoomTab[AIDEROOM].GetName(Buffer2, sizeof(Buffer2)), getmenumsg(MENU_AIDE, 112));
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

    if (waspublic && (!CurrentRoom->IsPublic() || CurrentRoom->IsBIO()))
        {
        cycleroom(thisRoom, FALSE, FALSE);
        CurrentUser->SetInRoom(thisRoom, TRUE);
        }

    if (!waspublic && (CurrentRoom->IsPublic() && !CurrentRoom->IsBIO()))
        {
        cycleroom(thisRoom, TRUE, FALSE);
        }

    CurrentRoom->Save(thisRoom);

    // trap it
	char TrapStr[200];
    label Buffer1;
    sprintf(TrapStr, getmenumsg(MENU_AIDE, 113), oldname, CurrentRoom->GetName(Buffer, sizeof(Buffer)),
            CurrentUser->GetName(Buffer1, sizeof(Buffer1)));
#ifdef WINCIT
    trap(T_AIDE, WindowCaption, TrapStr);
#else
	trap(T_AIDE, TrapStr);
#endif

    // Aide room
    Message *Msg = new Message;

    if (Msg)
        {
        char summary[500];

        CreateRoomSummary(summary, thisRoom);
        Msg->SetTextWithFormat(getmenumsg(MENU_AIDE, 114), TrapStr, bn, summary);

        if (CurrentRoom->IsGroupOnly())
            {
            if (CurrentRoom->IsBooleanGroup())
                {
                Msg->SetGroup(SPECIALSECURITY);
                }
            else
                {
                Msg->SetGroup(CurrentRoom->GetGroup());
                }
            }

        Msg->SetRoomNumber(AIDEROOM);

        systemMessage(Msg);

        delete Msg;
        }
    else
        {
        OutOfMemory(21);
        }

    thisRoom = oldthisRoom;
    CurrentRoom->Load(thisRoom);

    UnlockMenu(MENU_AIDE);
    return (TRUE);
    }


void MessageDatC::EnsureRoomEmpty(r_slot Room)
	{
	Lock();

#if defined(WINCIT) || defined(AUXMEM)
    m_index ID2;

    for (m_index ID = GetFirstMessageInRoom(Room); ID != M_INDEX_ERROR; ID = ID2)
        {
        ID2 = GetNextRoomMsg(ID);
#else
    for (m_index ID = OldestMessageInTable(); ID <= NewestMessage(); ID++)
        {
		if (GetRoomNum(ID) == Room)
#endif
        ChangeRoom(ID, (r_slot) (IsMail(ID) ? MAILROOM : DUMP));
        }

	Unlock();
	}

// --------------------------------------------------------------------------
// DestroyAllReferencesToRoom()

void TERMWINDOWMEMBER DestroyAllReferencesToRoom(r_slot TheRoom)
    {
    // Remove all messages from room.
	MessageDat.EnsureRoomEmpty(TheRoom);

    // kill room from every hall and write halls to disk
    for (h_slot j = 0; j < cfg.maxhalls; j++)
        {
        HallData[j].SetRoomInHall(TheRoom, FALSE);
        HallData[j].SetWindowedIntoHall(TheRoom, FALSE);
        }

    HallData.Save();

    // remove from all personal halls.
    CurrentUser->SetRoomInPersonalHall(TheRoom, FALSE);

    LogEntry6 Log6(cfg.maxrooms);
    Log6.OpenFile();
    for (l_slot l = 0; l < cfg.MAXLOGTAB; l++)
        {
        Log6.Load(l);
        Log6.SetRoomInPersonalHall(TheRoom, FALSE);
        Log6.Save(l);
        }
    Log6.CloseFile();
    }

// --------------------------------------------------------------------------
// killempties(): Sysop fn to kill empty rooms.

Bool MessageDatC::RoomIsEmpty(r_slot Room)
	{
	#if defined(WINCIT) || defined(AUXMEM)
		return (GetFirstMessageInRoom(Room) == M_INDEX_ERROR);
	#else
		Lock();

		for (m_index ID = OldestMessageInTable(); ID <= NewestMessage(); ID++)
			{
			if (GetRoomNum(ID) == Room)
				{
				Unlock();
				return (FALSE);
				}
			}

		Unlock();
		return (TRUE);
	#endif
	}

void TERMWINDOWMEMBER killempties(void)
    {
    Message *Msg = new Message;

    if (Msg)
        {
        label oldName;
        Bool didkill = FALSE;
        int i;

        SetDoWhat(SYSKEMPTY);

        CurrentRoom->GetName(oldName, sizeof(oldName));

		label UserName;
        Msg->SetTextWithFormat(getsysmsg(168), cfg.Lrooms_nym, CurrentUser->GetName(UserName, sizeof(UserName)));

        r_slot slot;
        for (slot = DUMP + 1; slot < cfg.maxrooms; slot++)
            {
            if (RoomTab[slot].IsInuse())
                {
                Bool goodRoom;

                if (RoomTab[slot].IsPermanent() || RoomTab[slot].IsMsDOSdir() || RoomTab[slot].IsApplication())
                    {
                    goodRoom = TRUE;
                    }
                else
                    {
                    goodRoom = !MessageDat.RoomIsEmpty(slot);
                    }

                if (!goodRoom)
                    {
                    RoomC RoomBuffer;

                    char text[LABELSIZE+10];

                    RoomBuffer.Load(slot);

					label RoomName;
                    sprintf(text, getmsg(377), RoomBuffer.GetName(RoomName, sizeof(RoomName)));

                    if ((i = getYesNo(text, 3)) == TRUE)
                        {
                        if (slot == thisRoom)
                            {
                            ChangeRooms(LOBBY);
                            }

                        // Note its death
                        Msg->AppendText(RoomName);
                        Msg->AppendText(getmsg(340));

                        if(!read_tr_messages())
                            {
                            errorDisp(getmsg(172));
                            }
                        else
                            {

#ifdef WINCIT
                        trap(T_AIDE, WindowCaption, gettrmsg(17), RoomName, UserName);
#else
                        trap(T_AIDE, gettrmsg(17), RoomName, UserName);
#endif
                            dump_tr_messages();
                            }

                        // Nuke the room
                        DestroyAllReferencesToRoom(slot);
                        RoomBuffer.Clear();
                        RoomBuffer.Save(slot);
                        didkill = TRUE;
                        }

                    if (i == 2)
                        {
                        break;
                        }
                    }
                }
            }

        if (i != 2)
            {
            doCR();
            mPrintfCR(getsysmsg(360), cfg.Lrooms_nym);
            }

        if (didkill)
            {
            Msg->SetRoomNumber(AIDEROOM);
            systemMessage(Msg);
            }

        delete Msg;
        }
    else
        {
        OutOfMemory(22);
        }
    }


// --------------------------------------------------------------------------
// killroom(): Aide fn to kill specified room.

Bool TERMWINDOWMEMBER killroom(const char *name)
    {
    label roomname;
    r_slot roomslot;

    if (name)
        {
        roomslot = RoomExists(name);

        if (roomslot == CERROR)
            {
            return (FALSE);
            }
        }
    else
        {
        Bool test;

        SetDoWhat(AIDEKILL);

        do
            {
            label Buffer;
            do
                {
                getString(cfg.Lroom_nym, roomname, LABELSIZE, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));

                if (*roomname == '?')
                    {
                    ListAllRooms(FALSE, TRUE, FALSE);
                    }
                } while (*roomname == '?');

            if (RoomTab[thisRoom].IsSameName(roomname))
                {
                roomslot = thisRoom;
                test = TRUE;
                }
            else
                {
                normalizeString(roomname);

                if (!*roomname)
                    {
                    return (FALSE);
                    }

                roomslot = RoomExists(roomname);
                if (roomslot == CERROR)
                    {
                    roomslot = PartialRoomExists(roomname, thisRoom, TRUE);
                    }

                test = TRUE;
                if (roomslot == CERROR)
                    {
                    CRmPrintfCR(getmsg(157), cfg.Lroom_nym, roomname);
                    test = FALSE;
                    }
                }
            } while (!test);
        }

    label RoomName, UserName;
	RoomTab[roomslot].GetName(RoomName, sizeof(RoomName));
	CurrentUser->GetName(UserName, sizeof(UserName));

    if (roomslot <= DUMP)
        {
        if (!name)
            {
            label Buffer2, Buffer3;

            CRmPrintfCR(getmsg(379), RoomTab[LOBBY].GetName(RoomName, sizeof(RoomName)),
                    RoomTab[MAILROOM].GetName(UserName, sizeof(UserName)),
                    RoomTab[AIDEROOM].GetName(Buffer2, sizeof(Buffer2)),
                    RoomTab[DUMP].GetName(Buffer3, sizeof(Buffer3)));
            }

        return (FALSE);
        }

    char Prompt[256];
    sprintf(Prompt, getmsg(377), RoomName);
    if (!name && !getYesNo(Prompt, 0))
        {
        return (FALSE);
        }

    DestroyAllReferencesToRoom(roomslot);

#ifdef WINCIT
    trap(T_AIDE, WindowCaption, getmsg(380), RoomName, UserName);
#else
    trap(T_AIDE, getmsg(380), RoomName, UserName);
#endif


    RoomC RoomBuffer;
    RoomBuffer.Load(roomslot);

    Message *Msg = new Message;

    if (Msg)
        {
        Msg->SetTextWithFormat(getmsg(450), cfg.Uroom_nym, RoomName, UserName);

        if (RoomBuffer.IsGroupOnly())
            {
            if (RoomBuffer.IsBooleanGroup())
                {
                Msg->SetGroup(SPECIALSECURITY);
                }
            else
                {
                Msg->SetGroup(RoomBuffer.GetGroup());
                }
            }

        Msg->SetRoomNumber(AIDEROOM);
        systemMessage(Msg);

        delete Msg;
        }
    else
        {
        OutOfMemory(23);
        }

	label NetID;
    RoomBuffer.GetNetID(NetID, sizeof(NetID));
    if (RoomBuffer.IsShared() && *NetID && !IsNetIDInNetIDCit(NetID))
        {
        AddNetIDToNetIDCit(NetID, getmsg(459));
        }

    RoomBuffer.SetInuse(FALSE);

    RoomBuffer.Save(roomslot);

    if (roomslot == thisRoom)
        {
        CRmPrintf(getmsg(458));
        stepRoom(0);
        }

    return (TRUE);
    }


// --------------------------------------------------------------------------
// AideMoveRoom(): Move a room in the room position table.
//
// Input:
//  offset: 1 for forward; -1 for backward
//  inHall: TRUE to move until effect is noticed in this hall; FALSE to move
//          only one position at a time.

Bool TERMWINDOWMEMBER AideMoveRoom(int offset, Bool inHall)
    {
    if (!LockMenu(MENU_AIDE))
        {
        return (FALSE);
        }

    r_slot rp1, rp2;

    SetDoWhat(MOVEROOM);

    mPrintfCR(getmenumsg(MENU_AIDE, 153), cfg.Lroom_nym);
    doCR();

    rp1 = GetRoomPosInTable(thisRoom);

    if  ((rp1 > (LOBBY+1) || offset == 1) && (rp1 < (cfg.maxrooms - 1) || offset == -1) && (thisRoom != LOBBY))
        {
        do
            {
            rp2 = (r_slot) (rp1 + offset);

            roomPos[rp1] = RoomPosTableToRoom(rp2);
            roomPos[rp2] = thisRoom;

            rp1 = rp2;
            } while (
                    (rp1 > (LOBBY+1) || offset == 1) &&
                    (rp1 < cfg.maxrooms - 1 || offset == -1) &&
                    !(RoomTab[RoomPosTableToRoom((r_slot) (rp1 - offset))].IsInuse() &&
                    (!inHall || CurrentUser->IsUsePersonalHall() ?
                    CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom((r_slot) (rp1 - offset))) :
                    roominhall(RoomPosTableToRoom((r_slot) (rp1 - offset)), thisHall))));

        putRoomPos();

        if (!(RoomTab[RoomPosTableToRoom((r_slot) (rp1 - offset))].IsInuse() &&
                (!inHall || CurrentUser->IsUsePersonalHall() ?
                CurrentUser->IsRoomInPersonalHall(RoomPosTableToRoom((r_slot) (rp1 - offset))) :
                roominhall(RoomPosTableToRoom((r_slot) (rp1 - offset)), thisHall))))
            {
            UnlockMenu(MENU_AIDE);
            return (FALSE);
            }
        else
            {
            do
                {
                rp2--;
                } while
                    (
                    rp2 &&
                        !(
                        RoomTab[RoomPosTableToRoom(rp2)].IsInuse() &&
                            (
                            !inHall ||

                            CurrentUser->IsUsePersonalHall() ?

                                CurrentUser->IsRoomInPersonalHall
                                    (
                                    RoomPosTableToRoom
                                        (
                                        (r_slot) (rp1 - offset)
                                        )
                                    )
                            :
                                roominhall
                                    (
                                    RoomPosTableToRoom(rp2), thisHall
                                    )
                            )
                        )
                    );

            label Buffer;
            mPrintfCR(getmsg(384), cfg.Uroom_nym, RoomTab[RoomPosTableToRoom(rp2)].GetName(Buffer, sizeof(Buffer)));

            UnlockMenu(MENU_AIDE);
            return (TRUE);
            }
        }
    else
        {
        UnlockMenu(MENU_AIDE);
        return (FALSE);
        }
    }

void TERMWINDOWMEMBER AideKillRoom(void)
    {
    doCR();
    killroom(NULL);
    }
