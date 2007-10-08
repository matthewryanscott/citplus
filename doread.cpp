// --------------------------------------------------------------------------
// Citadel: DoRead.CPP
//
// Handles the .Read... commands

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "filecmd.h"
#include "log.h"
#include "msg.h"
#include "hall.h"
#include "boolexpr.h"
#include "group.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
//
// doRead()         handles R(ead) command


// --------------------------------------------------------------------------
// doRead(): Handles R(ead) command.

void TERMWINDOWMEMBER doRead(Bool moreYet, char first)
// moreYet: TRUE to accept following parameters
// first:   first parameter if TRUE
    {
    SetDoWhat(DOREAD);

    if (!LockMenu(MENU_READ))
        {
        OutOfMemory(45);
        return;
        }

    mPrintf(getmenumsg(MENU_READ, 26));

    if (!loggedIn && !cfg.unlogReadOk)
        {
        CRmPrintfCR(getmsg(239));
        UnlockMenu(MENU_READ);
        return;
        }

    Bool abort = FALSE;
    Bool revOrder = FALSE;
    OldNewPick whichMess = WhoKnows;
    Bool textverbose = FALSE;
    Bool read_forward_userlog = FALSE;
    long howMany = 0;
    Bool lastNum = FALSE;

    resetMsgfilter(FALSE);

    int OptionCounter = 0;

    MRO.Clear(CurrentUser->IsAutoVerbose());

    if (moreYet)
        {
        first = 0;
        }

    do
        {
        const int ich = first ? first : iCharNE();

        if (++OptionCounter == 69)
            {
            mPrintf(pctss, getmsg(69));
            OptionCounter = 0;
            }

        if (isdigit(ich))
            {
            if (!lastNum)
                {
                howMany = 0;
                }

            lastNum = TRUE;

            mPrintf(pctc, ich);

            howMany *= 10;
            howMany += ich - '0';
            }
        else
            {
            if (lastNum)
                {
                mPrintf(spc);
                }
            lastNum = FALSE;

            switch (DoMenu(MENU_READ, ich))
                {
                case 1: // All
                    {
                    MRO.All = TRUE;
                    break;
                    }

                case 2: // By-user
                    {
                    rf.User = TRUE;
                    break;
                    }

                case 3: // Configuration
                    {
                    DoCommand(UC_READCONFIGURATION);
                    abort = TRUE;
                    break;
                    }

                case 4: // Directory
                    {
                    if (whichMess == WhoKnows)
                        {
                        whichMess = OldAndNew;
                        }

                    DoCommand(UC_READDIRECTORY, whichMess);

                    abort = TRUE;
                    MRO.Reverse = FALSE;
                    break;
                    }

                case 5: // Exclusive
                    {
                    rf.Mail = TRUE;
                    break;
                    }

                case 6: // Forward
                    {
                    revOrder = FALSE;
                    whichMess = OldAndNew;
                    read_forward_userlog = TRUE;
                    break;
                    }

                case 7: // Group-only
                    {
                    rf.Group = TRUE;
                    break;
                    }

                case 8: // Halls
                    {
                    DoCommand(UC_READHALLS);
                    abort = TRUE;
                    break;
                    }

                case 9: // Info file
                    {
                    if (whichMess == WhoKnows)
                        {
                        whichMess = OldAndNew;
                        }

                    DoCommand(UC_READINFOFILE, whichMess);

                    abort = TRUE;
                    MRO.Reverse = FALSE;
                    break;
                    }

                case 10: // Keyword search
                    {
                    rf.Text = TRUE;
                    break;
                    }

                case 11: // Local
                    {
                    rf.Local = TRUE;
                    break;
                    }

                case 12: // Messages
                    {
                    moreYet = FALSE;
                    break;
                    }

                case 13: // New
                    {
                    whichMess = NewOnly;
                    break;
                    }

                case 14: // Old
                    {
                    revOrder = TRUE;
                    read_forward_userlog = FALSE;
                    whichMess = OldOnly;
                    break;
                    }

                case 15: // Public
                    {
                    rf.Public = TRUE;
                    break;
                    }

                case 16: // Reverse
                    {
                    revOrder = TRUE;
                    read_forward_userlog = TRUE;
                    MRO.Reverse = TRUE;
                    whichMess = OldAndNew;
                    break;
                    }

                case 17: // Status
                    {
                    DoCommand(UC_READSTATUS);
                    abort = TRUE;
                    break;
                    }

                case 18: // Text file
                    {
                    DoCommand(UC_READTEXTFILE, whichMess, textverbose);
                    abort = TRUE;
                    break;
                    }

                case 19: // Userlog
                    {
                    doCR();
                    DoCommand(UC_READUSERLOG, read_forward_userlog,
                            whichMess, howMany);
                    abort = TRUE;
                    break;
                    }

                case 20: // Verbose
                    {
                    MRO.Verbose = TRUE;
                    textverbose = TRUE;
                    break;
                    }

                case 21: // With protocol download
                    {
                    DoCommand(UC_READWITHPROTOCOL);
                    abort = TRUE;
                    break;
                    }

                case 22: // Archive
                    {
                    DoCommand(UC_READARCHIVE, whichMess);
                    abort = TRUE;
                    break;
                    }

                case 23: // By msg number
                    {
                    MRO.Number = TRUE;
                    break;
                    }

                case 24: // By date
                    {
                    MRO.Date = TRUE;
                    break;
                    }

                case 25: // Header scan
                    {
                    MRO.Headerscan = TRUE;
                    break;
                    }

                case 26: // Just one at a time
                    {
                    MRO.PauseBetween = TRUE;
                    break;
                    }

                case -1: // ESC
                    {
                    abort = TRUE;
                    break;
                    }

                default: // Unknown
                    {
                    oChar((char) ich);
                    mPrintf(sqst);
                    abort = TRUE;

                    if (CurrentUser->IsExpert())
                        {
                        break;
                        }
                    }

                case -2: // Menu
                    {
                    if (ich == '?')
                        {
                        oChar('?');
                        }

                    SetDoWhat(READMENU);

                    showMenu(M_READOPT);
                    abort = TRUE;
                    break;
                    }
                }
            }

        first = 0;
        } while (moreYet && !abort);

    if (!abort)
        {
        // no by-user read in anon rooms
//      if (RoomTab[thisRoom].IsAnonymous())
//          {
//          rf.User = FALSE;
//          }

        if (whichMess == WhoKnows)
            {
            whichMess = NewOnly;
            }

        doCR();
        showMessages(whichMess, revOrder, howMany);
        }

    UnlockMenu(MENU_READ);
    }

Bool TERMWINDOWMEMBER ProcessReadFilter(ReadFilter *rf, Bool AllowMetaGroups)
    {
    if (rf->User)
        {
        if (!getString(cfg.Luser_nym, rf->SearchUser, LABELSIZE, FALSE,
                rf2.SearchUser))
            {
            return (FALSE);
            }

        normalizeString(rf->SearchUser);
        stripansi(rf->SearchUser);

        if (!*(rf->SearchUser))
            {
            rf->User = FALSE;
            }
        }

    if (rf->Group)
        {
        char groupname[256];

        Bool GroupDone = FALSE;
        do
            {
            if (rf2.SearchGroup[0] != BOOLEAN_END && IsBooleanExpressionValid(rf2.SearchGroup))
                {
                CRmPrintf(getmsg(553));
                ShowBooleanExpression(rf2.SearchGroup, ShowGroupWithMeta);
                doCR();
                }

            if (!getString(cfg.Lgroup_nym, groupname, 254, ns))
                {
                return (FALSE);
                }

            stripansi(groupname);

            if (*groupname == '?')
                {
                ListGroups(TRUE);
                }
            else if (*groupname)
                {
                if (SameString(groupname, spc))
                    {
                    GroupDone = TRUE;
                    rf->Group = FALSE;
                    }
                else
                    {
                    BoolExpr Expression;

                    if (!CreateBooleanExpression(groupname, AllowMetaGroups ? TestGroupOrMetaExists : TestGroupExists,
#ifdef WINCIT
                            Expression, this))
#else
                            Expression))
#endif
                        {
                        CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
                        }
                    else
                        {
                        memcpy(rf->SearchGroup, Expression, sizeof(BoolExpr));
                        GroupDone = TRUE;
                        }
                    }
                }
            else
                {
                if (rf2.SearchGroup[0] != BOOLEAN_END &&
                        IsBooleanExpressionValid(rf2.SearchGroup))
                    {
                    memcpy(rf->SearchGroup, rf2.SearchGroup, sizeof(BoolExpr));
                    }
                else
                    {
                    rf->Group = FALSE;
                    }

                GroupDone = TRUE;
                }
            } while (!GroupDone);

        if (rf->Group)
            {
            CRmPrintf(getmsg(198));
            ShowBooleanExpression(rf->SearchGroup, ShowGroupWithMeta);
            mPrintfCR(getmsg(552));
            }
        }

    if (rf->Text)
        {
        if (!getString(getmsg(388), rf->SearchText, LABELSIZE, FALSE, rf2.SearchText))
            {
            return (FALSE);
            }

//      normalizeString(rf->SearchText);

        if (!*rf->SearchText)
            {
            rf->Text = FALSE;
            }
        }

    return (TRUE);
    }

void TERMWINDOWMEMBER resetMsgfilter(int resetOld)
    {
    if (resetOld)
        {
        CurrentUser->GetName(rf2.SearchUser, sizeof(rf2.SearchUser));
        stripansi(rf2.SearchUser);
        rf2.SearchGroup[0] = 0;
        rf2.SearchGroup[1] = BOOLEAN_END;
        rf2.SearchText[0] = 0;
        }
    else
        {
        if (*rf.SearchUser)
            {
            CopyStringToBuffer(rf2.SearchUser, rf.SearchUser);
            }

        if (rf.SearchGroup[0] != BOOLEAN_END)
            {
            memcpy(rf2.SearchGroup, rf.SearchGroup, sizeof(BoolExpr));
            }

        if (*rf.SearchText)
            {
            CopyStringToBuffer(rf2.SearchText, rf.SearchText);
            }
        }

    memset(&rf, 0, sizeof(ReadFilter));
    rf.SearchGroup[0] = BOOLEAN_END;
    }
