#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "tallybuf.h"
#include "miscovl.h"
#include "extmsg.h"
#include "statline.h"


void TERMWINDOWMEMBER net69_menu(void)
    {
//  LogEntry SaveLog(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
    Bool loadedlog = FALSE;
    Bool Incorporated = FALSE;

    SetDoWhat(SYSNET69);

    if (!read_net_messages())
        {
        cPrintf(getmsg(59));
        return;
        }

    //  SaveLog = *CurrentUser;
    storeLog();

    const l_slot oldthisLog = ThisLog;
    const Bool OldLoggedIn = loggedIn;

    int ich;
    switch (toupper(ich = iCharNE()))
        {
        case 'F':
            {
            SetDoWhat(S69FETCH);

            mPrintfCR(getnetmsg(103));

            label rnode;
            getNormStr(getnetmsg(104), rnode, LABELSIZE);

#ifdef WINCIT
            if (!*rnode || !ReadNodesCit(this, &node, rnode, MODEM, TRUE))
#else
            if (!*rnode || !ReadNodesCit(&node, rnode, MODEM, TRUE))
#endif
                {
                break;
                }

            if (LoadPersonByName(node->GetName()))
                {
                LogEntry5 Log5(cfg.maxrooms);

                loggedIn = TRUE;

                mPrintfCR(getmsg(207), node->GetName());

                // fetch uses the loaded node entry
                loadedlog = TRUE;

                amZap();

                Log5.CopyRoomNewPointers(*CurrentUser);

                if (net69_fetch(TRUE, MODEM))
                    {
                    CurrentUser->SetCallNumber(cfg.callno);
                    time(&logtimestamp);
                    CurrentUser->SetCallTime(logtimestamp);
#ifdef WINCIT
                    slideLTab(&ThisSlot, CurrentUser, LogOrder);
#else
                    slideLTab(&ThisSlot, CurrentUser);
#endif
                    }
                else
                    {
                    CurrentUser->CopyRoomNewPointers(Log5);
                    }

                SaveAideMess(NULL);
                }
            else
                {
                CRmPrintfCR(getmsg(255), node->GetName());
                }

            break;
            }

        case '>':
            {
            SetDoWhat(S86FETCH);

            mPrintfCR(getnetmsg(142));

            label rnode;
            getNormStr(getnetmsg(104), rnode, LABELSIZE);

#ifdef WINCIT
            if (*rnode && ReadNodesCit(this, &node, rnode, MODEM, FALSE))
#else
            if (*rnode && ReadNodesCit(&node, rnode, MODEM, FALSE))
#endif
                {
                if (LoadPersonByName(node->GetName()))
                    {
                    LogEntry5 Log5(cfg.maxrooms);

                    loggedIn = TRUE;

                    mPrintfCR(getmsg(207), node->GetName());

                    // fetch uses the loaded node entry
                    loadedlog = TRUE;

                    amZap();

                    Log5.CopyRoomNewPointers(*CurrentUser);

                    if (net86_fetch(MODEM))
                        {
                        CurrentUser->SetCallNumber(cfg.callno);
                        time(&logtimestamp);
                        CurrentUser->SetCallTime(logtimestamp);
#ifdef WINCIT
                        slideLTab(&ThisSlot, CurrentUser, LogOrder);
#else
                        slideLTab(&ThisSlot, CurrentUser);
#endif
                        }
                    else
                        {
                        CurrentUser->CopyRoomNewPointers(Log5);
                        }

                    SaveAideMess(NULL);
                    }
                else
                    {
                    CRmPrintfCR(getmsg(255), node->GetName());
                    }
                }

            break;
            }

        case 'I':
            {
            SetDoWhat(S69INC);

            mPrintfCR(getnetmsg(105));

            amZap();
            net69_incorporate(MODEM);
            SaveAideMess(NULL);

            Incorporated = TRUE;

            break;
            }

        case '<':
            {
            SetDoWhat(S86INC);

            mPrintfCR(getnetmsg(143));

            label rnode;
            getNormStr(getnetmsg(104), rnode, LABELSIZE);

#ifdef WINCIT
            if (*rnode && ReadNodesCit(this, &node, rnode, MODEM, FALSE))
#else
            if (*rnode && ReadNodesCit(&node, rnode, MODEM, FALSE))
#endif
                {
                amZap();
                net86_incorporate(MODEM);
                SaveAideMess(NULL);

                Incorporated = TRUE;
                }

            break;
            }

        case 'R':
            {
            SetDoWhat(S69ROOMREQ);

            mPrintfCR(getnetmsg(106));

            label rnode;
            getNormStr(getnetmsg(104), rnode, LABELSIZE);

            if (*rnode)
                {
                NetLogEntryC Log;

                if (Log.IsValid())
                    {
#ifdef WINCIT
                    if (Log.LoadByName(rnode, LogOrder))
#else
                    if (Log.LoadByName(rnode))
#endif
                        {
                        doCR();

#ifdef WINCIT
                        Log.MakeNet69RoomRequestFile(MODEM, this);
#else
                        Log.MakeNet69RoomRequestFile(MODEM);
#endif
                        }
                    else
                        {
                        CRmPrintfCR(getmsg(255), rnode);
                        }
                    }
                else
                    {
                    OutOfMemory(85);
                    }
                }

            break;
            }

        case '@':
            {
            mPrintfCR(getnetmsg(112));
            buildaddress(MODEM);
            break;
            }

        case '?':
            {
            oChar('?');
            SetDoWhat(S69MENU);

            showMenu(M_SYSNET);
            break;
            }

        default:
            {
            BadMenuSelection(ich);
            break;
            }
        }

    if (loadedlog)
        {
        storeLog();

        ThisLog = oldthisLog;
        CurrentUser->Load(ThisLog);
//      *CurrentUser = SaveLog;
        loggedIn = OldLoggedIn;
        }

    if (loadedlog || Incorporated)
        {
        Talley->Fill();
        }

    dump_net_messages();

    freeNode(&node);

    StatusLine.Update(WC_TWp);
    }
