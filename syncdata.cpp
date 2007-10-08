// --------------------------------------------------------------------------
// Citadel: SyncData.CPP
//
// Used by the Windows version for synchronization of data across threads

#ifdef WINCIT

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "room.h"

SynchronizerC Sync;

Bool SynchronizerC::Add(LogEntry1Data *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry1List *L1L = (LogEntry1List *) addLL((void **) &L1, sizeof(LogEntry1List));

    Bool ToRet;

    if (L1L)
        {
        ToRet = TRUE;

        L1L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogEntry2 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry2List *L2L = (LogEntry2List *) addLL((void **) &L2, sizeof(LogEntry2List));

    Bool ToRet;

    if (L2L)
        {
        ToRet = TRUE;

        L2L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogEntry3 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry3List *L3L = (LogEntry3List *) addLL((void **) &L3, sizeof(LogEntry3List));

    Bool ToRet;

    if (L3L)
        {
        ToRet = TRUE;

        L3L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogEntry4 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry4List *L4L = (LogEntry4List *) addLL((void **) &L4, sizeof(LogEntry4List));

    Bool ToRet;

    if (L4L)
        {
        ToRet = TRUE;

        L4L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogEntry5 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry5List *L5L = (LogEntry5List *) addLL((void **) &L5, sizeof(LogEntry5List));

    Bool ToRet;

    if (L5L)
        {
        ToRet = TRUE;

        L5L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogEntry6 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry6List *L6L = (LogEntry6List *) addLL((void **) &L6, sizeof(LogEntry6List));

    Bool ToRet;

    if (L6L)
        {
        ToRet = TRUE;

        L6L->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(LogExtensions *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogExtensionsList *LEL = (LogExtensionsList *) addLL((void **) &LE, sizeof(LogExtensionsList));

    Bool ToRet;

    if (LEL)
        {
        ToRet = TRUE;

        LEL->L = L;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Add(RoomC *IR)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    RoomCList *RL = (RoomCList *) addLL((void **) &R, sizeof(RoomCList));

    Bool ToRet;

    if (RL)
        {
        ToRet = TRUE;

        RL->R = IR;
        }
    else
        {
        ToRet = FALSE;
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (ToRet);
    }

Bool SynchronizerC::Remove(LogEntry1Data *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry1List *L1L;
    int C;
    for (L1L = L1, C = 1; L1L; L1L = (LogEntry1List *) getNextLL(L1L), C++)
        {
        if (L1L->L == L)
            {
            deleteLLNode((void **) &L1, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogEntry2 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry2List *L2L;
    int C;
    for (L2L = L2, C = 1; L2L; L2L = (LogEntry2List *) getNextLL(L2L), C++)
        {
        if (L2L->L == L)
            {
            deleteLLNode((void **) &L2, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogEntry3 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry3List *L3L;
    int C;
    for (L3L = L3, C = 1; L3L; L3L = (LogEntry3List *) getNextLL(L3L), C++)
        {
        if (L3L->L == L)
            {
            deleteLLNode((void **) &L3, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogEntry4 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry4List *L4L;
    int C;
    for (L4L = L4, C = 1; L4L; L4L = (LogEntry4List *) getNextLL(L4L), C++)
        {
        if (L4L->L == L)
            {
            deleteLLNode((void **) &L4, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogEntry5 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry5List *L5L;
    int C;
    for (L5L = L5, C = 1; L5L; L5L = (LogEntry5List *) getNextLL(L5L), C++)
        {
        if (L5L->L == L)
            {
            deleteLLNode((void **) &L5, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogEntry6 *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogEntry6List *L6L;
    int C;
    for (L6L = L6, C = 1; L6L; L6L = (LogEntry6List *) getNextLL(L6L), C++)
        {
        if (L6L->L == L)
            {
            deleteLLNode((void **) &L6, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(LogExtensions *L)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    LogExtensionsList *LEL;
    int C;
    for (LEL = LE, C = 1; LEL;
            LEL = (LogExtensionsList *) getNextLL(LEL), C++)
        {
        if (LEL->L == L)
            {
            deleteLLNode((void **) &LE, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

Bool SynchronizerC::Remove(RoomC *IR)
    {
    WaitForSingleObject(SyncMutex, INFINITE);
    VerifyHeap();

    RoomCList *RL;
    int C;
    for (RL = R, C = 1; RL; RL = (RoomCList *) getNextLL(RL), C++)
        {
        if (RL->R == IR)
            {
            deleteLLNode((void **) &R, C);
            ReleaseMutex(SyncMutex);
            VerifyHeap();
            return (TRUE);
            }
        }

    ReleaseMutex(SyncMutex);
    VerifyHeap();

    return (FALSE);
    }

void *SynchronizerC::FindMatching(LogEntry1Data *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry1List *L1L = L1; L1L; L1L = (LogEntry1List *) getNextLL(L1L))
        {
        if ((L1L->L != L) && (L1L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L1L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogEntry2 *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry2List *L2L = L2; L2L; L2L = (LogEntry2List *) getNextLL(L2L))
        {
        if ((L2L->L != L) && (L2L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L2L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogEntry3 *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry3List *L3L = L3; L3L; L3L = (LogEntry3List *) getNextLL(L3L))
        {
        if ((L3L->L != L) && (L3L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L3L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogEntry4 *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry4List *L4L = L4; L4L; L4L = (LogEntry4List *) getNextLL(L4L))
        {
        if ((L4L->L != L) && (L4L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L4L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogEntry5 *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry5List *L5L = L5; L5L; L5L = (LogEntry5List *) getNextLL(L5L))
        {
        if ((L5L->L != L) && (L5L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L5L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogEntry6 *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogEntry6List *L6L = L6; L6L; L6L = (LogEntry6List *) getNextLL(L6L))
        {
        if ((L6L->L != L) && (L6L->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (L6L->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(LogExtensions *L)
    {
    assert(L);

    const int SyncNum = L->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (LogExtensionsList *LEL = LE; LEL; LEL = (LogExtensionsList *) getNextLL(LEL))
        {
        if ((LEL->L != L) && (LEL->L->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (LEL->L);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

void *SynchronizerC::FindMatching(RoomC *iR)
    {
    assert(R);

    const int SyncNum = iR->GetSyncNumber();

    if (SyncNum == CERROR)
        {
        return (NULL);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    for (RoomCList *RL = R; RL; RL = (RoomCList *) getNextLL(RL))
        {
        if ((RL->R != iR) && (RL->R->GetSyncNumber() == SyncNum))
            {
            ReleaseMutex(SyncMutex);
            return (RL->R);
            }
        }

    ReleaseMutex(SyncMutex);
    return (NULL);
    }

Bool SynchronizerC::Synchronize(int SyncNumber, SyncActionE Action, SYNCTYPE D1, SYNCTYPE D2, Bool *RetVal)
    {
    if (SyncNumber == CERROR)
        {
        return (FALSE);
        }

    WaitForSingleObject(SyncMutex, INFINITE);

    if (Busy)
        {
        ReleaseMutex(SyncMutex);
        return (FALSE);
        }

    Bool ToRet = FALSE;
    Busy = TRUE;

    if (Action >= L1_FIRST && Action <= L1_LAST)
        {
        // With L1
        for (LogEntry1List *L1L = L1; L1L; L1L = (LogEntry1List *) getNextLL(L1L))
            {
            assert(L1L->L);

            if (L1L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                switch (Action)
                    {
                    case L1_SETNAME:			L1L->L->SetName((const char *) D1);				break;
                    case L1_SETINITIALS:		L1L->L->SetInitials((const char *) D1);			break;
                    case L1_SETPASSWORD:    	L1L->L->SetPassword((const char *) D1);			break;
                    case L1_SETSURNAME:			L1L->L->SetSurname((const char *) D1);			break;
                    case L1_SETTITLE:			L1L->L->SetTitle((const char *) D1);			break;
                    case L1_SETREALNAME:		L1L->L->SetRealName((const char *) D1);			break;
                    case L1_SETPHONENUM:		L1L->L->SetPhoneNumber((const char *) D1);		break;
                    case L1_SETFORWARDADDR:		L1L->L->SetForwardAddr((const char *) D1);		break;
                    case L1_SETFORWARDADDRNODE:	L1L->L->SetForwardAddrNode((const char *) D1);	break;
                    case L1_SETFORWARDADDRREG:	L1L->L->SetForwardAddrRegion((const char *) D1);break;
                    case L1_SETPROMPTFORMAT:	L1L->L->SetPromptFormat((const char *) D1);		break;
                    case L1_SETDATESTAMP:		L1L->L->SetDateStamp((const char *) D1);		break;
                    case L1_SETVERBOSEDATESTAMP:L1L->L->SetVerboseDateStamp((const char *) D1);	break;
                    case L1_SETSIGNATURE:		L1L->L->SetSignature((const char *) D1);		break;
                    case L1_SETNETPREFIX:		L1L->L->SetNetPrefix((const char *) D1);		break;
                    case L1_SETMAILADDR1:		L1L->L->SetMailAddr1((const char *) D1);		break;
                    case L1_SETMAILADDR2:		L1L->L->SetMailAddr2((const char *) D1);		break;
                    case L1_SETMAILADDR3:		L1L->L->SetMailAddr3((const char *) D1);		break;
                    case L1_SETALIAS:			L1L->L->SetAlias((const char *) D1);			break;
                    case L1_SETLOCID:			L1L->L->SetLocID((const char *) D1);			break;
                    case L1_SETMOREPROMPT:		L1L->L->SetMorePrompt((const char *) D1);		break;
                    case L1_SETOCCUPATION:		L1L->L->SetOccupation((const char *) D1);		break;
                    case L1_SETWHEREHEAR:		L1L->L->SetWhereHear((const char *) D1);		break;
                    case L1_SETLASTROOM:		L1L->L->SetLastRoom((const char *) D1);			break;
                    case L1_SETLASTHALL:		L1L->L->SetLastHall((const char *) D1);			break;
                    case L1_SETDEFAULTROOM:		L1L->L->SetDefaultRoom((const char *) D1);		break;
                    case L1_SETDEFAULTHALL:		L1L->L->SetDefaultHall((const char *) D1);		break;
                    case L1_SETTERMTYPE:		L1L->L->SetTermType((const char *) D1);			break;
                    case L1_SETBIRTHDATE:		L1L->L->SetBirthDate((time_t) D1);				break;
                    case L1_SETFIRSTON:			L1L->L->SetFirstOn((time_t) D1);				break;
                    case L1_SETSEX:				L1L->L->SetSex((SexE)(int) D1);					break;
                    case L1_SETNULLS:			L1L->L->SetNulls((int) D1);						break;
                    case L1_SETWIDTH:			L1L->L->SetWidth((int) D1);						break;
                    case L1_SETLINESPERSCREEN:	L1L->L->SetLinesPerScreen((int) D1);			break;
                    case L1_SETATTRIBUTE:		L1L->L->SetAttribute((LogAttr)(int) D1, (int) D2);break;
                    case L1_SETNUMUSERSHOW:		L1L->L->SetNumUserShow((int) D1);				break;
                    case L1_SETDEFAULTPROTOCOL:	L1L->L->SetDefaultProtocol((char) D1);			break;
                    case L1_SETCALLTIME:		L1L->L->SetCallTime((time_t) D1);				break;
                    case L1_SETCALLNUMBER:		L1L->L->SetCallNumber((ulong) D1);				break;
                    case L1_SETTOTALTIME:		L1L->L->SetTotalTime((time_t) D1);				break;
                    case L1_SETCREDITS:			L1L->L->SetCredits((long) D1);					break;
                    case L1_SETLOGINS:			L1L->L->SetLogins((long) D1);					break;
                    case L1_SETPOSTED:			L1L->L->SetPosted((long) D1);					break;
                    case L1_SETREAD:			L1L->L->SetRead((long) D1);						break;
                    case L1_SETPASSWORDCHANGETIME:L1L->L->SetPasswordChangeTime((time_t) D1);	break;
                    case L1_SETCALLSTODAY:		L1L->L->SetCallsToday((int) D1);				break;
                    case L1_SETCALLLIMIT:		L1L->L->SetCallLimit((int) D1);					break;
                    case L1_SETLASTMESSAGE:		L1L->L->SetLastMessage((m_index) D1);			break;
                    case L1_SETDLBYTES:			L1L->L->SetDL_Bytes((ulong) D1);				break;
                    case L1_SETULBYTES:			L1L->L->SetUL_Bytes((ulong) D1);				break;
                    case L1_SETDLNUM:			L1L->L->SetDL_Num((uint) D1);					break;
                    case L1_SETULNUM:			L1L->L->SetUL_Num((uint) D1);					break;
                    case L1_SETPOOPCOUNT:		L1L->L->SetPoopcount((long) D1);				break;
                    case L1_MOREPOOP:			L1L->L->MorePoop((long) D1);					break;
                    case L1_SETDUNGEONED:		L1L->L->SetDungeoned((Bool) D1);				break;
                    case L1_SETFORWARDTONODE:	L1L->L->SetForwardToNode((Bool) D1);			break;
                    case L1_SETAUTONEXTHALL:	L1L->L->SetAutoNextHall((Bool) D1);				break;
                    case L1_SETENTERBORDERS:	L1L->L->SetEnterBorders((Bool) D1);				break;
                    case L1_SETVERIFIED:		L1L->L->SetVerified((Bool) D1);					break;
                    case L1_SETSURNAMELOCKED:	L1L->L->SetSurnameLocked((Bool) D1);			break;
                    case L1_SETDEFAULTHALLLOCKED:L1L->L->SetDefaultHallLocked((Bool) D1);		break;
                    case L1_SETPSYCHO:			L1L->L->SetPsycho((Bool) D1);					break;
                    case L1_SETVIEWTITLESURNAME:L1L->L->SetViewTitleSurname((Bool) D1);			break;
                    case L1_SETVIEWSUBJECTS:	L1L->L->SetViewSubjects((Bool) D1);				break;
                    case L1_SETVIEWSIGNATURES:	L1L->L->SetViewSignatures((Bool) D1);			break;
                    case L1_SETOLDIBMGRAPH:		L1L->L->SetOldIBMGraph((Bool) D1);				break;
                    case L1_SETOLDIBMANSI:		L1L->L->SetOldIBMANSI((Bool) D1);				break;
                    case L1_SETOLDIBMCOLOR:		L1L->L->SetOldIBMColor((Bool) D1);				break;
                    case L1_SETTWIRLY:			L1L->L->SetTwirly((Bool) D1);					break;
                    case L1_SETAUTOVERBOSE:		L1L->L->SetAutoVerbose((Bool) D1);				break;
                    case L1_SETPAUSEBETWEENMSGS:L1L->L->SetPauseBetweenMessages((Bool) D1);		break;
                    case L1_SETMINIBIN:			L1L->L->SetMinibin((Bool) D1);					break;
                    case L1_SETCLEARSCREENMSGS:	L1L->L->SetClearScreenBetweenMessages((Bool) D1);break;
                    case L1_SETVIEWROOMINFO:	L1L->L->SetViewRoomInfoLines((Bool) D1);		break;
                    case L1_SETVIEWHALLDESC:	L1L->L->SetViewHallDescription((Bool) D1);		break;
                    case L1_SETVERBOSECONTINUE:	L1L->L->SetVerboseContinue((Bool) D1);			break;
                    case L1_SETVIEWCENSORED:	L1L->L->SetViewCensoredMessages((Bool) D1);		break;
                    case L1_SETVIEWBORDERS:		L1L->L->SetViewBorders((Bool) D1);				break;
                    case L1_SETOUT300:			L1L->L->SetOut300((Bool) D1);					break;
                    case L1_SETUSERSIGLOCKED:	L1L->L->SetUserSignatureLocked((Bool) D1);		break;
                    case L1_SETHIDEMSGEXCL:		L1L->L->SetHideMessageExclusions((Bool) D1);	break;
                    case L1_SETDOWNLOAD:		L1L->L->SetDownload((Bool) D1);					break;
                    case L1_SETUPLOAD:			L1L->L->SetUpload((Bool) D1);					break;
                    case L1_SETCHAT:			L1L->L->SetChat((Bool) D1);						break;
                    case L1_SETPRINTFILE:		L1L->L->SetPrintFile((Bool) D1);				break;
                    case L1_SETSPELLCHECKMODE:	L1L->L->SetSpellCheckMode((int) D1);			break;
                    case L1_SETMAKEROOM:		L1L->L->SetMakeRoom((Bool) D1);					break;
                    case L1_SETVERBOSELOGOUT:	L1L->L->SetVerboseLogOut((Bool) D1);			break;
                    case L1_SETCONFIRMSAVE:		L1L->L->SetConfirmSave((Bool) D1);				break;
                    case L1_SETCONFIRMABORT:	L1L->L->SetConfirmAbort((Bool) D1);				break;
                    case L1_SETCONFIRMNOEO:		L1L->L->SetConfirmNoEO((Bool) D1);				break;
                    case L1_SETUSEPERSONALHALL:	L1L->L->SetUsePersonalHall((Bool) D1);			break;
                    case L1_SETYOUAREHERE:		L1L->L->SetYouAreHere((Bool) D1);				break;
                    case L1_SETIBMROOM:			L1L->L->SetIBMRoom((Bool) D1);					break;
                    case L1_SETWIDEROOM:		L1L->L->SetWideRoom((Bool) D1);					break;
                    case L1_SETMUSIC:			L1L->L->SetMusic((Bool) D1);					break;
                    case L1_SETCHECKAPOSTROPHES:L1L->L->SetCheckApostropheS((Bool) D1);			break;
                    case L1_SETCHECKALLCAPS:	L1L->L->SetCheckAllCaps((Bool) D1);				break;
                    case L1_SETCHECKDIGITS:		L1L->L->SetCheckDigits((Bool) D1);				break;
                    case L1_SETEXCLUDEENCRYPTED:L1L->L->SetExcludeEncryptedMessages((Bool) D1);	break;
                    case L1_SETVIEWCOMMAS:		L1L->L->SetViewCommas((Bool) D1);				break;
                    case L1_SETPUNPAUSES:		L1L->L->SetPUnPauses((Bool) D1);				break;
                    case L1_SETROMAN:			L1L->L->SetRoman((Bool) D1);					break;
                    case L1_SETSUPERSYSOP:		L1L->L->SetSuperSysop((Bool) D1);				break;
                    case L1_SETINUSE:			L1L->L->SetInuse((Bool) D1);					break;
                    case L1_SETUPPERONLY:		L1L->L->SetUpperOnly((Bool) D1);				break;
                    case L1_SETLINEFEEDS:		L1L->L->SetLinefeeds((Bool) D1);				break;
                    case L1_SETEXPERT:			L1L->L->SetExpert((Bool) D1);					break;
                    case L1_SETAIDE:			L1L->L->SetAide((Bool) D1);						break;
                    case L1_SETTABS:			L1L->L->SetTabs((Bool) D1);						break;
                    case L1_SETOLDTOO:			L1L->L->SetOldToo((Bool) D1);					break;
                    case L1_SETPROBLEM:			L1L->L->SetProblem((Bool) D1);					break;
                    case L1_SETUNLISTED:		L1L->L->SetUnlisted((Bool) D1);					break;
                    case L1_SETPERMANENT:		L1L->L->SetPermanent((Bool) D1);				break;
                    case L1_SETSYSOP:			L1L->L->SetSysop((Bool) D1);					break;
                    case L1_SETNODE:			L1L->L->SetNode((Bool) D1);						break;
                    case L1_SETNETUSER:			L1L->L->SetNetUser((Bool) D1);					break;
                    case L1_SETACCOUNTING:		L1L->L->SetAccounting((Bool) D1);				break;
                    case L1_SETMAIL:			L1L->L->SetMail((Bool) D1);						break;
                    case L1_SETVIEWROOMDESC:	L1L->L->SetViewRoomDesc((Bool) D1);				break;
                    case L1_SETBUNNY:			L1L->L->SetBunny((Bool) D1);					break;
					case L1_SSE_LOGONOFF:		L1L->L->SetSELogOnOff((Bool) D1);				break;
					case L1_SSE_NEWMSG:			L1L->L->SetSENewMessage((Bool) D1);				break;
					case L1_SSE_EXCLMSG:		L1L->L->SetSEExclusiveMessage((Bool) D1);		break;
					case L1_SSE_CHATALL:		L1L->L->SetSEChatAll((Bool) D1);				break;
					case L1_SSE_CHATROOM:		L1L->L->SetSEChatRoom((Bool) D1);				break;
					case L1_SSE_CHATGROUP:		L1L->L->SetSEChatGroup((Bool) D1);				break;
					case L1_SSE_CHATUSER:		L1L->L->SetSEChatUser((Bool) D1);				break;
					case L1_SSE_RMINOUT:		L1L->L->SetSERoomInOut((Bool) D1);				break;
					case L1_SETBANNED:			L1L->L->SetBanned((Bool) D1);					break;
					case L1_SETKILLOWN:			L1L->L->SetKillOwn((Bool) D1);					break;
                 	case L1_SETSEEOWNCHATS:     L1L->L->SetSeeOwnChats((Bool) D1); 				break;
                	case L1_SETERASEPROMPT:     L1L->L->SetErasePrompt((Bool) D1);	    		break;	
                 	case L1_SETAUTOIDLESECONDS: L1L->L->SetAutoIdleSeconds((int) D1);			break;
                    case L1_SETHEARLAUGHTER:    L1L->L->SetHearLaughter((Bool) D1);             break;
                    
					default: assert(FALSE);
                    }
                }
            }
        }
    else if (Action == L2_SETINGROUP)
        {
        // With L2

        for (LogEntry2List *L2L = L2; L2L; L2L = (LogEntry2List *) getNextLL(L2L))
            {
            assert(L2L->L);

            if (L2L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                L2L->L->SetInGroup((g_slot) D1, (Bool) D2);
                }
            }
        }
    else if (Action == L3_SETINROOM)
        {
        // With L3

        for (LogEntry3List *L3L = L3; L3L; L3L = (LogEntry3List *) getNextLL(L3L))
            {
            assert(L3L->L);

            if (L3L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                L3L->L->SetInRoom((r_slot) D1, (Bool) D2);
                }
            }
        }
    else if (Action == L4_SETROOMEXCLUDED)
        {
        // With L4

        for (LogEntry4List *L4L = L4; L4L; L4L = (LogEntry4List *) getNextLL(L4L))
            {
            assert(L4L->L);

            if (L4L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                L4L->L->SetRoomExcluded((r_slot) D1, (Bool) D2);
                }
            }
        }
    else if (Action == L5_SETROOMNEWPOINTER)
        {
        // With L5

        for (LogEntry5List *L5L = L5; L5L; L5L = (LogEntry5List *) getNextLL(L5L))
            {
            assert(L5L->L);

            if (L5L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                L5L->L->SetRoomNewPointer((r_slot) D1, (m_index) D2);
                }
            }
        }
    else if (Action == L6_SETROOMINPERSONALHALL)
        {
        // With L6

        for (LogEntry6List *L6L = L6; L6L; L6L = (LogEntry6List *) getNextLL(L6L))
            {
            assert(L6L->L);

            if (L6L->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                L6L->L->SetRoomInPersonalHall((r_slot) D1, (Bool) D2);
                }
            }
        }
    else if (Action >= LE_FIRST && Action <= LE_LAST)
        {
        // With LE

        for (LogExtensionsList *LEL = LE; LEL; LEL = (LogExtensionsList *) getNextLL(LEL))
            {
            assert(LEL->L);

            if (LEL->L->GetSyncNumber() == SyncNumber)
                {
                // Do it
                switch (Action)
                    {
                    case LE_SETMESSAGEROOM:	LEL->L->SetMessageRoom((r_slot) D1);	 		break;
                    case LE_SETMESSAGE:		LEL->L->SetMessage((Message *) D1);		 		break;
                    case LE_CLEARMESSAGE:	LEL->L->ClearMessage();					 		break;
					case LE_JUMPBACKPUSH:	LEL->L->JumpbackPush(*((jumpback *) D1));		break;

                    case LE_SETFINGER:			ToRet = LEL->L->SetFinger((const char *) D1);						break;
                    case LE_ADDKILL:			ToRet = LEL->L->AddKill((KillFileE)(int) D1, (const char *) D2);	break;
                    case LE_REMOVEKILL:			ToRet = LEL->L->RemoveKill((KillFileE)(int) D1, (const char *) D2);	break;
                    case LE_ADDTAGUSER:			ToRet = LEL->L->AddTagUser((const char *) D1, (const char *) D2);	break;
                    case LE_ADDREPLACE:			ToRet = LEL->L->AddReplace((const char *) D1, (const char *) D2);	break;
                    case LE_ADDWORDTODICT:		ToRet = LEL->L->AddWordToDictionary((const char *) D1);				break;
                    case LE_REMOVEUSERTAG:		ToRet = LEL->L->RemoveUserTag((const char *) D1);					break;
                    case LE_REMOVEREPLACE:		ToRet = LEL->L->RemoveReplace((const char *) D1);					break;
                    case LE_REMOVEWORDFROMDICT:	ToRet = LEL->L->RemoveWordFromDictionary((const char *) D1);		break;
                    case LE_SETUSERDEFINED:		ToRet = LEL->L->SetUserDefined((const char *) D1, (const char *) D2);break;
                    case LE_REMOVEUSERDEFINED:	ToRet = LEL->L->RemoveUserDefined((const char *) D1);				break;
                    case LE_JUMPBACKPOP:		ToRet = LEL->L->JumpbackPop((jumpback *) D1);						break;

                    default: assert(FALSE);
                    }
                }
            }
        }
    else
        {
        // With R
        assert(Action >= RM_FIRST && Action <= RM_LAST);

        for (RoomCList *RL = R; RL; RL = (RoomCList *) getNextLL(RL))
            {
            assert(RL->R);

            if (RL->R->GetSyncNumber() == SyncNumber)
                {
                // Do it
                switch (Action)
                    {                                                                   	
                    case RM_SETINUSE:			RL->R->SetInuse((Bool) D1);					break;
                    case RM_SETPUBLIC:			RL->R->SetPublic((Bool) D1);            	break;              	
                    case RM_SETMSDOSDIR:    	RL->R->SetMsDOSdir((Bool) D1);          	break;              	
                    case RM_SETPERMANENT:   	RL->R->SetPermanent((Bool) D1);         	break;              	
                    case RM_SETGROUPONLY:   	RL->R->SetGroupOnly((Bool) D1);         	break;              	
                    case RM_SETREADONLY:    	RL->R->SetReadOnly((Bool) D1);          	break;              	
                    case RM_SETDOWNLOADONLY:	RL->R->SetDownloadOnly((Bool) D1);      	break;              	
                    case RM_SETSHARED:      	RL->R->SetShared((Bool) D1);            	break;              	
                    case RM_SETMODERATED:   	RL->R->SetModerated((Bool) D1);         	break;              	
                    case RM_SETAPPLICATIONB:	RL->R->SetApplication((Bool) D1);       	break;              	
                    case RM_SETBIO:         	RL->R->SetBIO((Bool) D1);               	break;              	
                    case RM_SETUPLOADONLY:  	RL->R->SetUploadOnly((Bool) D1);        	break;
                    case RM_SETPRIVILEGEDGROUP:	RL->R->SetPrivilegedGroup((Bool) D1);   	break;
                    case RM_SETANONYMOUS:		RL->R->SetAnonymous((Bool) D1);         	break;              	
                    case RM_SETSUBJECT:     	RL->R->SetSubject((Bool) D1);           	break;
                    case RM_SETGROUPMODERATES:	RL->R->SetGroupModerates((Bool) D1);    	break;
                    case RM_SETARCHIVE:			RL->R->SetArchive((Bool) D1);           	break;              	
                    case RM_SETBOOLEANGROUP:	RL->R->SetBooleanGroup((Bool) D1);      	break;              	
                    case RM_SETNAME:        	RL->R->SetName((const char *) D1);      	break;              	
                    case RM_SETDIRECTORY:   	RL->R->SetDirectory((const char *) D1); 	break;              	
                    case RM_SETDESCFILE:    	RL->R->SetDescFile((const char *) D1);  	break;              	
                    case RM_SETAPPLICATIONS:	RL->R->SetApplication((const char *) D1);	break;              	
                    case RM_SETGROUP:       	RL->R->SetGroup((g_slot) D1);           	break;              	
                    case RM_SETPGROUP:      	RL->R->SetPGroup((g_slot) D1);          	break;              	
                    case RM_SETAUTOAPP:     	RL->R->SetAutoApp((Bool) D1);           	break;              	
                    case RM_SETUNEXCLUDABLE:	RL->R->SetUnexcludable((Bool) D1);      	break;              	
                    case RM_SETINFOLINE:    	RL->R->SetInfoLine((const char *) D1);  	break;              	
                    case RM_SETNETID:       	RL->R->SetNetID((const char *) D1);     	break;              	
                    case RM_SETARCHIVEFILE: 	RL->R->SetArchiveFile((const char *) D1);	break;              	
                    case RM_SETDICTIONARY:  	RL->R->SetDictionary((const char *) D1);	break;
                    case RM_SETGROUPEXPRESSION:	RL->R->SetGroupExpression((ushort *) D1);	break;
                    case RM_SETCREATOR:         RL->R->SetCreator((const char *) D1);   	break;

                    default: assert(FALSE);
                    }
                }
            }
        }

    Busy = FALSE;

    if (RetVal)
        {
        *RetVal = ToRet;
        }

    ReleaseMutex(SyncMutex);
    return (TRUE);
    }
#endif
