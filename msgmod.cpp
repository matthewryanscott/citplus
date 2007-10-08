// --------------------------------------------------------------------------
// Citadel: MsgMod.CPP
//
// Message modification code

#include "ctdl.h"
#pragma hdrstop

#include "tallybuf.h"
#include "room.h"
#include "auxtab.h"
#include "log.h"
#include "msg.h"
#include "domenu.h"
#include "extmsg.h"



// --------------------------------------------------------------------------
// Contents
//
// copymessage()    copies specified message # into specified room
// deleteMessage()  deletes message for pullIt()
// insert()         aide fn: to insert a message
// markIt()         is a sysop special to mark current message
// markmsg()        marks a message for insertion and/or visibility
// pullIt()         is a sysop special to remove a message from a room


Bool censor_message(Message *Msg)
    {
    censor *theCensor;

    if (*Msg->GetToUser())
        {
        return (FALSE);
        }

    for (theCensor = censorList; theCensor; theCensor = (censor *) getNextLL(theCensor))
        {
        switch (theCensor->what)
            {
            case CENSOR_TEXT:
                {
                if (    IsSubstr(Msg->GetAuthor(),          theCensor->str) ||
                        IsSubstr(Msg->GetTitle(),           theCensor->str) ||
                        IsSubstr(Msg->GetOriginCountry(),   theCensor->str) ||
                        IsSubstr(Msg->GetToCountry(),       theCensor->str) ||
                        IsSubstr(Msg->GetFromPath(),        theCensor->str) ||
                        IsSubstr(Msg->GetToPath(),          theCensor->str) ||
                        IsSubstr(Msg->GetForward(),         theCensor->str) ||
                        IsSubstr(Msg->GetGroup(),           theCensor->str) ||
                        IsSubstr(Msg->GetOriginNodeName(),  theCensor->str) ||
                        IsSubstr(Msg->GetOriginRegion(),    theCensor->str) ||
                        IsSubstr(Msg->GetCreationRoom(),    theCensor->str) ||
                        IsSubstr(Msg->GetToUser(),          theCensor->str) ||
                        IsSubstr(Msg->GetSurname(),         theCensor->str) ||
                        IsSubstr(Msg->GetToNodeName(),      theCensor->str) ||
                        IsSubstr(Msg->GetToRegion(),        theCensor->str) ||
                        IsSubstr(Msg->GetUserSignature(),   theCensor->str) ||
                        IsSubstr(Msg->GetSubject(),         theCensor->str) ||
                        IsSubstr(Msg->GetSignature(),       theCensor->str) ||
                        IsSubstr(Msg->GetOriginSoftware(),  theCensor->str) ||
                        IsSubstr(Msg->GetTwitRegion(),      theCensor->str) ||
                        IsSubstr(Msg->GetTwitCountry(),     theCensor->str) ||
                        IsSubstr(Msg->GetOriginPhoneNumber(),theCensor->str)||
                        IsSubstr(Msg->GetToPhoneNumber(),   theCensor->str) ||
                        IsSubstr(Msg->GetText(),            theCensor->str))
                    {
                    censormsg(Msg);
                    return (TRUE);
                    }

                break;
                }

            case CENSOR_AUTHOR:
                {
                if (SameString(Msg->GetAuthor(), theCensor->str))
                    {
                    censormsg(Msg);
                    return (TRUE);
                    }

                break;
                }

            case CENSOR_NODE:
                {
                if (SameString(Msg->GetOriginNodeName(), theCensor->str))
                    {
                    censormsg(Msg);
                    return (TRUE);
                    }

                break;
                }

            default:
                {
                assert(FALSE);
                break;
                }
            }
        }

    return (FALSE);
    }


// --------------------------------------------------------------------------
// addtomsglist(): Adds an element to message queue.

void TERMWINDOWMEMBER addtomsglist(m_index msgnumber)
    {
    messageList *thisMsg;

    compactMemory();

    thisMsg = (messageList *) addLL((void **) &MS.MsgList, sizeof(*thisMsg));

    if (thisMsg)
        {
        thisMsg->msg = msgnumber;
        }
    else
        {
        OutOfMemory(64);
        }
    }


// --------------------------------------------------------------------------
// AideQueueRoomMark(): Adds all messages in a room to queue.
// bibnewmsg: need to work on locks here
void TERMWINDOWMEMBER AideQueueRoomMark(void)
    {
    SetDoWhat(AIDEQUEUER);

#if defined(WINCIT) || defined(AUXMEM)
    m_index ID2;

    for (m_index ID = MessageDat.GetFirstMessageInRoom(thisRoom); ID != M_INDEX_ERROR; ID = ID2)
        {
        ID2 = MessageDat.GetNextRoomMsg(ID);
#else
    for (m_index ID = MessageDat.OldestMessageInTable(); ID <= MessageDat.NewestMessage(); ID++)
        {
        if (MessageDat.GetRoomNum(ID) == thisRoom)
#endif
            if (CurrentUser->MaySeeMessage(ID) == MSM_GOOD)
                {
                addtomsglist(ID);
                }
        }
    }


// --------------------------------------------------------------------------
// AideQueueList(): Prints out messages currently in list.

void TERMWINDOWMEMBER AideQueueList(void)
    {
    if (LockMenu(MENU_AIDEQUEUE))
        {
        messageList *thisMsg;

        SetDoWhat(AIDEQUEUEL);

        MRO.Headerscan = TRUE;
        OC.SetOutFlag(OUTOK);
        doCR();

        if (MS.MsgList)
            {
            for (thisMsg = MS.MsgList; thisMsg;
                    thisMsg = (messageList *) getNextLL(thisMsg))
                {
                if (MRO.Verbose)
                    {
                    PrintMessageByID(thisMsg->msg);
                    }
                else
                    {
                    mPrintf(getmenumsg(MENU_AIDEQUEUE, 26), thisMsg->msg);
                    }
                }
            }
        else
            {
            CRmPrintfCR(getmenumsg(MENU_AIDEQUEUE, 22), cfg.Lmsgs_nym);
            }

        MRO.Headerscan = FALSE;

        UnlockMenu(MENU_AIDEQUEUE);
        }
    }


// --------------------------------------------------------------------------
// AideQueueInsert()

void TERMWINDOWMEMBER AideQueueInsert(void)
    {
    if (LockMenu(MENU_AIDEQUEUE))
        {
        if (LockMenu(MENU_AIDE))
            {
            messageList *thisMsg;

            SetDoWhat(AIDEQUEUEI);

            MRO.Headerscan = TRUE;
            OC.SetOutFlag(OUTOK);
            doCR();

            if (MS.MsgList)
                {
                for (thisMsg = MS.MsgList;
                        thisMsg && (OC.User.GetOutFlag() == OUTOK);
                        thisMsg = (messageList *) getNextLL(thisMsg))
                    {
                    if (MRO.Verbose)
                        {
                        CRmPrintf(getmenumsg(MENU_AIDEQUEUE, 20), cfg.Lmsg_nym);
                        PrintMessageByID(thisMsg->msg);
                        }
                    else
                        {
                        mPrintfCR(getmenumsg(MENU_AIDEQUEUE, 21), cfg.Lmsg_nym, ltoac(thisMsg->msg));
                        }

                    insert(thisMsg->msg);
                    }
                }
            else
                {
                mPrintfCR(getmenumsg(MENU_AIDEQUEUE, 22), cfg.Lmsgs_nym);
                }

            MRO.Headerscan = FALSE;

            UnlockMenu(MENU_AIDE);
            }

        UnlockMenu(MENU_AIDEQUEUE);
        }
    }


// --------------------------------------------------------------------------
// AideQueueKill()

Bool TERMWINDOWMEMBER AideQueueKill(void)
    {
    Bool RetVal = TRUE;

    if (LockMenu(MENU_AIDEQUEUE))
        {
        messageList *thisMsg;

        SetDoWhat(AIDEQUEUEK);

        MRO.Headerscan = TRUE;
        OC.SetOutFlag(OUTOK);
        doCR();

        if (MS.MsgList)
            {
            for (thisMsg = MS.MsgList;
                    thisMsg && (OC.User.GetOutFlag() == OUTOK);
                    thisMsg = (messageList *) getNextLL(thisMsg))
                {
                if (MRO.Verbose)
                    {
                    CRmPrintf(getmenumsg(MENU_AIDEQUEUE, 23), cfg.Lmsg_nym);
                    PrintMessageByID(thisMsg->msg);
                    }
                else
                    {
                    CRmPrintf(getmenumsg(MENU_AIDEQUEUE, 24), cfg.Lmsg_nym, ltoac(thisMsg->msg));
                    }

                deleteMessage(thisMsg->msg);
                }
            }
        else
            {
            mPrintfCR(getmenumsg(MENU_AIDEQUEUE, 22), cfg.Lmsgs_nym);
            RetVal = FALSE;
            }

        MRO.Headerscan = FALSE;

        UnlockMenu(MENU_AIDEQUEUE);
        }

    return (RetVal);
    }


// --------------------------------------------------------------------------
// msglistsort()

static int _cdecl msglistsort(void *s1, void *s2)
    {
    if (*(ulong *)s1 > *(ulong *)s2)
        {
        return (1);
        }

    if (*(ulong *)s1 < *(ulong *)s2)
        {
        return (-1);
        }

    return (0);
    }


// --------------------------------------------------------------------------
// revmsglistsort()

static int _cdecl revmsglistsort(void *s1, void *s2)
    {
    if (*(ulong *)s1 > *(ulong *)s2)
        {
        return (-1);
        }

    if (*(ulong *)s1 < *(ulong *)s2)
        {
        return (1);
        }

    return (0);
    }


// --------------------------------------------------------------------------
// sortmsglist()
//
// Notes:
//  Assumes MENU_AIDEQUEUE is locked.

void TERMWINDOWMEMBER sortmsglist(Bool Reverse)
    {
    ulong *listArray, p, i = getLLCount(MS.MsgList);
    messageList *thisMsg;

    if (MS.MsgList)
        {
        // this 8150 limit doesn't really exist in WINCIT
        if (i > 8150 || (listArray = new ulong[(uint) i]) == NULL)
            {
            OutOfMemory(65);
            }
        else
            {
            for (thisMsg = MS.MsgList, p = 0; thisMsg;
                    thisMsg = (messageList *) getNextLL(thisMsg), p++)
                {
                listArray[(uint) p] = thisMsg->msg;
                }
            disposeLL((void **) &MS.MsgList);

            if (Reverse)
                {
                qsort(listArray, (uint) p, (unsigned) sizeof(*listArray), (QSORT_CMP_FNP) revmsglistsort);
                }
            else
                {
                qsort(listArray, (uint) p, (unsigned) sizeof(*listArray), (QSORT_CMP_FNP) msglistsort);
                }

            for (i = 0; i < p; i++)
                {
                thisMsg = (messageList *) addLL((void **) &MS.MsgList, sizeof(*MS.MsgList));
                thisMsg->msg = listArray[(uint)i];
                }

            delete [] listArray;
            }
        }
    else
        {
        mPrintfCR(getmenumsg(MENU_AIDEQUEUE, 22), cfg.Lmsgs_nym);
        }
    }


// --------------------------------------------------------------------------
// AideQueueClear(): Clears msg queue.

void TERMWINDOWMEMBER AideQueueClear(void)
    {
    SetDoWhat(AIDEQUEUEC);
    disposeLL((void **) &MS.MsgList);
    }


// --------------------------------------------------------------------------
// AideQueueAutomark(): Automatically mark or display messages.

void TERMWINDOWMEMBER AideQueueAutomark(void)
    {
    doCR();

    if (LockMenu(MENU_AIDEQUEUE))
        {
        SetDoWhat(AIDEQUEUEM);

        if (MS.AutoMKC != AM_NONE)
            {
            // Yeah, MS.AutoMKC + 12 is sloppy. So shoot me.
            CRmPrintfCR(getmenumsg(MENU_AIDEQUEUE, 12), cfg.Lmsg_nym, getmenumsg(MENU_AIDEQUEUE, MS.AutoMKC + 12));

            MS.AutoMKC = AM_NONE;
            }
        else
            {
            switch (GetOneKey(getmenumsg(MENU_AIDEQUEUE, 17), getmenumsg(MENU_AIDEQUEUE, 16),
                    *getmenumsg(MENU_AIDEQUEUE, 16), B_AIDEQUEUEAUTO, getmenumsg(MENU_AIDEQUEUE, 13),
                    getmenumsg(MENU_AIDEQUEUE, 14), getmenumsg(MENU_AIDEQUEUE, 15)))
                {
                case 0:     // Mark
                    {
                    MS.AutoMKC = AM_MARK;
                    break;
                    }

                case 1:     // Kill
                    {
                    MS.AutoMKC = AM_KILL;
                    break;
                    }

                case 2:     // Censor
                    {
                    MS.AutoMKC = AM_CENSOR;
                    break;
                    }

                default:    // ESC, probably
                    {
                    MS.AutoMKC = AM_NONE;
                    break;
                    }

                }
            }

        UnlockMenu(MENU_AIDEQUEUE);
        }
    }


// --------------------------------------------------------------------------
// copymessage(): Copies specified message # into specified room.

void TERMWINDOWMEMBER copymessage(m_index ID, r_slot roomno)
    {
    if (!MessageDat.IsValid(ID))
        {
        return;
        }

    Message *Msg = new Message;

    if (Msg)
        {
        // load in message to be inserted
        MessageDat.LoadHeader(ID, Msg);

        // retain vital information
        uchar attr = Msg->GetAttribute();

        label copy;
        CopyStringToBuffer(copy, Msg->GetLocalID());

        Msg->ClearAll();

        Msg->SetCopyOfMessage(copy);
        Msg->SetAttribute(attr);
        Msg->SetRoomNumber(roomno);

        putAndNoteMessage(Msg, FALSE);

        delete Msg;
        }
    else
        {
        OutOfMemory(66);
        }
    }


// --------------------------------------------------------------------------
// deleteMessage(): Deletes a specified message.

void TERMWINDOWMEMBER deleteMessage(m_index ID)
    {
    if (!MessageDat.IsValid(ID))
        {
        return;
        }

    ulong LocationInFile = MessageDat.GetLocation(ID);

    if (LocationInFile == ULONG_ERROR)
        {
        return;
        }

    Message *Msg = new Message;
    if (Msg)
        {
        MessageDat.LoadAll(ID, Msg);

        Msg->Decompress();

        const r_slot room = MessageDat.GetRoomNum(ID);

        MessageDat.ChangeRoom(ID, DUMP);

        // if author move into old buffer
        if (CurrentUser->IsSameName(Msg->GetAuthor()))
            {
            delete MS.AbortedMessage;
            MS.AbortedMessage = new Message;

            if (MS.AbortedMessage)
                {
                *MS.AbortedMessage = *Msg;

                MS.AbortedMessage->SetToPath(ns);
                MS.AbortedMessage->SetFromPath(ns);
                MS.AbortedMessage->SetEncrypted(FALSE);
                MS.AbortedMessage->SetEncryptionKey(ns);
                }
            else
                {
                OutOfMemory(67);
                }
            }

        // at this point, we are done with Msg, so we can now use the buffer
        // to save the Aide) room message...

        if (thisRoom != AIDEROOM && thisRoom != DUMP && !CurrentUser->IsSameName(Msg->GetAuthor()))
            {
            Msg->ClearAll();

            // this bit gets the group from the original message, if there
            // was one. If we don't have the memory to load another message,
            // then don't sweat it; this is not really important.
            Message *OldMsg = new Message;
            if (OldMsg)
                {
                m_index myid = ID;
                Bool Good = TRUE;

                do
                    {
                    if (!MessageDat.IsValid(myid))
                        {
                        Good = FALSE;
                        }

                    if (Good && MessageDat.IsCopy(myid))
                        {
                        if (MessageDat.GetCopyOffset(myid) <= (myid - MessageDat.OldestMessageInTable()))
                            {
                            myid -= MessageDat.GetCopyOffset(myid);
                            }
                        else
                            {
                            // Copied message has scrolled
                            Good = FALSE;
                            }
                        }
                    } while (Good && MessageDat.IsCopy(myid));

                if (Good)
                    {
                    assert(MessageDat.IsValid(myid));

                    // load in message to be inserted
                    MessageDat.LoadHeader(myid, OldMsg);
                    Msg->SetGroup(OldMsg->GetGroup());
                    }

                delete OldMsg;
                }

            // trap it.
            label RoomName, UserName;
            RoomTab[room].GetName(RoomName, sizeof(RoomName));
            CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
            trap(T_AIDE, WindowCaption, getmsg(592), ltoac(ID), RoomName, UserName);
#else
            trap(T_AIDE, getmsg(592), ltoac(ID), RoomName, UserName);
#endif

            // note in Aide).
            Msg->SetTextWithFormat(getmsg(451), cfg.Lmsg_nym, ltoac(ID), RoomName, UserName);

            Msg->SetRoomNumber(AIDEROOM);
            systemMessage(Msg);

            // give our memory to copymessage() - it may need it.
            delete Msg;
            copymessage(ID, AIDEROOM);
            }
        else
            {
            delete Msg;
            }
        }
    else
        {
        OutOfMemory(67);
        }
    }


// --------------------------------------------------------------------------
// insert(): Aide fn: to insert a specified message.
//
// Notes:
//  Assumes MENU_AIDE is locked.

void TERMWINDOWMEMBER insert(m_index ID)
    {
    SetDoWhat(AIDEINSERT);

    if (thisRoom == AIDEROOM)
        {
        mPrintf(getmenumsg(MENU_AIDE, 157));
        return;
        }

    if (!ID || !MessageDat.IsValid(ID))
        {
        return;
        }

    ulong LocationInFile = MessageDat.GetLocation(ID);

    if (LocationInFile == ULONG_ERROR)
        {
        return;
        }

    Message *Msg = new Message;
    if (Msg)
        {
        copymessage(ID, thisRoom);

        // this bit gets the group from the original message, if there
        // was one. If we don't have the memory to load another message,
        // then don't sweat it; this is not really important.
        Message *OldMsg = new Message;
        if (OldMsg)
            {
            m_index myid = ID;
            Bool Good = TRUE;

            do
                {
                if (!MessageDat.IsValid(myid))
                    {
                    Good = FALSE;
                    }

                if (Good && MessageDat.IsCopy(myid))
                    {
                    if (MessageDat.GetCopyOffset(myid) <= (myid - MessageDat.OldestMessageInTable()))
                        {
                        myid -= MessageDat.GetCopyOffset(myid);
                        }
                    else
                        {
                        // Copied message has scrolled
                        Good = FALSE;
                        }
                    }
                } while (Good && MessageDat.IsCopy(myid));

            if (Good)
                {
                assert(MessageDat.IsValid(myid));

                // load in message to be inserted
                MessageDat.LoadHeader(myid, OldMsg);

                Msg->SetGroup(OldMsg->GetGroup());
                }

            delete OldMsg;
            }

        // trap it.
        label RoomName, UserName;
        RoomTab[thisRoom].GetName(RoomName, sizeof(RoomName));
        CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
        trap(T_AIDE, WindowCaption, getmenumsg(MENU_AIDE, 158), ltoac(ID), RoomName, UserName);
#else
        trap(T_AIDE, getmenumsg(MENU_AIDE, 158), ltoac(ID), RoomName, UserName);
#endif

        // Note in Aide)
        Msg->SetTextWithFormat(getmenumsg(MENU_AIDE, 159), cfg.Lmsg_nym, ltoac(ID), RoomName, UserName);

        Msg->SetRoomNumber(AIDEROOM);
        systemMessage(Msg);

        // give our memory to copymessage() - it may need it.
        delete Msg;
        copymessage(ID, AIDEROOM);
        }
    else
        {
        OutOfMemory(68);
        }
    }

// --------------------------------------------------------------------------
// markIt(): Is a query to mark current message.

Bool TERMWINDOWMEMBER markIt(Message *Msg)
    {
    int yna;

    // confirm that we're marking the right one
    if (MS.AutoMKC != AM_MARK)
        {
        Bool oldverbose;

        OC.SetOutFlag(OUTOK);
        oldverbose = MRO.Verbose;
        MRO.Verbose = FALSE;
        PrintMessage(Msg);
        MRO.Verbose = oldverbose;
        }

    OC.SetOutFlag(OUTOK);

    yna = getYesNo(getmsg(120), MS.AutoMKC == AM_MARK ? 4 : 1);

    if (yna == 1)
        {
        markmsg(Msg);
        return (TRUE);
        }
    else if (yna == 2)
        {
        OC.SetOutFlag(OUTSKIP);
        }

    return (FALSE);
    }


// --------------------------------------------------------------------------
// censorIt(): Is a query to censor current message.

Bool TERMWINDOWMEMBER censorIt(Message *Msg)
    {
    int yna;

    // confirm that we're censoring the right one
    OC.SetOutFlag(OUTOK);
    const Bool oldverbose = MRO.Verbose;
    MRO.Verbose = FALSE;
    if (MS.AutoMKC != AM_CENSOR)
        {
        PrintMessage(Msg);
        }

    MRO.Verbose = oldverbose;

    OC.SetOutFlag(OUTOK);

    if (Msg->GetOriginalAttribute() & ATTR_CENSORED)
        {
        yna = getYesNo(getmsg(276), MS.AutoMKC == AM_CENSOR ? 4 : 1);
        }
    else
        {
        yna = getYesNo(getmsg(275), MS.AutoMKC == AM_CENSOR ? 4 : 1);
        }

    if (yna == 1)
        {
        MS.MarkedID = Msg->GetOriginalID();

        // put in message queue. wonder if we should comment it out
        addtomsglist(MS.MarkedID);

        censormsg(Msg);
        return (TRUE);
        }

    if (yna == 2)
        {
        OC.SetOutFlag(OUTSKIP);
        }

    return (FALSE);
    }


// --------------------------------------------------------------------------
// censormsg(): Toggles censor bit for current message.
void censormsg(Message *Msg)
    {
    TalleyBufferCollection.RemoveMessage(atol(Msg->GetLocalID()));

    Msg->SetOriginalAttribute((char)(Msg->GetOriginalAttribute() ^ ATTR_CENSORED));
    MessageDat.ChangeAttribute(Msg->GetOriginalID(), Msg->GetOriginalAttribute());

    TalleyBufferCollection.NewMessageNoNotify(Msg);
    }

// --------------------------------------------------------------------------
// markmsg(): Marks current msg for insertion and/or visibility.

void TERMWINDOWMEMBER markmsg(Message *Msg)
    {
    MS.MarkedID = Msg->GetOriginalID();

    // put in message queue
    addtomsglist(MS.MarkedID);

    if (*Msg->GetX())
        {
        Msg->SetOriginalAttribute((char) (Msg->GetOriginalAttribute() ^ ATTR_MADEVIS));

        MessageDat.ChangeAttribute(Msg->GetOriginalID(), Msg->GetOriginalAttribute());

        if (Msg->GetOriginalAttribute() & ATTR_MADEVIS)
            {
            copymessage(Msg->GetOriginalID(), Msg->GetOriginalRoom());
            }
        }
    }


// --------------------------------------------------------------------------
// pullIt(): Is a query to remove current msg from room.

Bool TERMWINDOWMEMBER pullIt(Message *Msg)
    {
    // confirm that we're removing the right one
    OC.SetOutFlag(OUTOK);

    const Bool oldverbose = MRO.Verbose;
    MRO.Verbose = FALSE;

    if (MS.AutoMKC != AM_KILL)
        {
        PrintMessage(Msg);
        }

    MRO.Verbose = oldverbose;

    OC.SetOutFlag(OUTOK);

    const int yna = getYesNo(getmsg(277), MS.AutoMKC == AM_KILL ? 3 : 0);

    if (yna == 1)
        {
        // moved up from deletemessage()
        if (!*Msg->GetX())
            {
            markmsg(Msg);   // Mark it for possible insertion elsewhere
            }

        deleteMessage(Msg->GetOriginalID());
        return (TRUE);
        }

    if (yna == 2)
        {
        OC.SetOutFlag(OUTSKIP);
        }

    return (FALSE);
    }


void MessageDatC::ChangeRoom(m_index ID, r_slot NewRoom)
    {
    Lock();

    if (!Table.IsValid(ID))
        {
        Unlock();
        return;
        }

    long loc = Table.GetLocation(ID);
    if (loc == CERROR)
        {
        Unlock();
        return;
        }

    // determine room # of message to be changed
    const r_slot OldRoom = Table.GetRoomNum(ID);

    TalleyBufferCollection.MoveMessage(ID, OldRoom, NewRoom);

#if defined(AUXMEM) || defined(WINCIT)
    // and fix our room list in the message table...

    // first, take it out of old room.
    if (Table.GetFirstMessageInRoom(OldRoom) == ID)
        {
        // this was the first message in the room. set new first to next.

        const m_index FM = Table.GetNextRoomMsg(ID);

        Table.SetFirstMessageInRoom(OldRoom, FM);

        if (FM != M_INDEX_ERROR)
            {
            assert(Table.IsValid(FM));

            Table.SetPrevRoomMsg(FM, M_INDEX_ERROR);
            }

        // it might also have been the last...
        if (Table.GetLastMessageInRoom(OldRoom) == ID)
            {
            // this was the last message in the room. set new last to prev.
            const m_index LM = Table.GetPrevRoomMsg(ID);

            Table.SetLastMessageInRoom(OldRoom, LM);

            if (LM != M_INDEX_ERROR)
                {
                assert(Table.IsValid(LM));

                Table.SetNextRoomMsg(LM, M_INDEX_ERROR);
                }
            }
        }
    else if (Table.GetLastMessageInRoom(OldRoom) == ID)
        {
        // this was the last message in the room. set new last to prev.
        const m_index LM = Table.GetPrevRoomMsg(ID);

        Table.SetLastMessageInRoom(OldRoom, LM);

        if (LM != M_INDEX_ERROR)
            {
            assert(Table.IsValid(LM));

            Table.SetNextRoomMsg(LM, M_INDEX_ERROR);
            }
        }
    else
        {
        // just a message somewhere in between the first and last...
        const m_index ID2 = Table.GetPrevRoomMsg(ID);
        const m_index ID3 = Table.GetNextRoomMsg(ID);

        assert(Table.IsValid(ID2));
        assert(Table.IsValid(ID3));

        Table.SetNextRoomMsg(ID2, ID3);
        Table.SetPrevRoomMsg(ID3, ID2);
        }

    // now, put it in new room.
    if (Table.GetFirstMessageInRoom(NewRoom) == M_INDEX_ERROR)
        {
        // first message in this room...
        assert(Table.GetLastMessageInRoom(NewRoom) == M_INDEX_ERROR);

        Table.SetPrevRoomMsg(ID, M_INDEX_ERROR);
        Table.SetNextRoomMsg(ID, M_INDEX_ERROR);

        Table.SetFirstMessageInRoom(NewRoom, ID);
        Table.SetLastMessageInRoom(NewRoom, ID);
        }
    else
        {
        assert(Table.GetLastMessageInRoom(NewRoom) != M_INDEX_ERROR);

        if (ID < Table.GetFirstMessageInRoom(NewRoom))
            {
            // this message is older then the old first message in room...
            const m_index OF = Table.GetFirstMessageInRoom(NewRoom);
            assert(Table.IsValid(OF));

            Table.SetNextRoomMsg(ID, OF);
            Table.SetPrevRoomMsg(ID, M_INDEX_ERROR);

            Table.SetPrevRoomMsg(OF, ID);

            Table.SetFirstMessageInRoom(NewRoom, ID);
            }
        else if (ID > Table.GetLastMessageInRoom(NewRoom))
            {
            // this message is newer than the old last message in room...
            const m_index OL = Table.GetLastMessageInRoom(NewRoom);
            assert(Table.IsValid(OL));

            Table.SetPrevRoomMsg(ID, OL);
            Table.SetNextRoomMsg(ID, M_INDEX_ERROR);

            Table.SetNextRoomMsg(OL, ID);

            Table.SetLastMessageInRoom(NewRoom, ID);
            }
        else
            {
            // insert it somewhere...
            m_index CurrentMessage, LastMessage;

            for (CurrentMessage = Table.GetFirstMessageInRoom(NewRoom);

                    CurrentMessage < ID;

                    LastMessage = CurrentMessage,
                    assert(CurrentMessage >= Table.GetOldestMessage() && CurrentMessage <= Table.GetNewestMessage()),
                    CurrentMessage = Table.GetNextRoomMsg(CurrentMessage))
                {
                assert(CurrentMessage != M_INDEX_ERROR);
                }

            assert(LastMessage != ID);
            assert(CurrentMessage >= Table.GetOldestMessage() && CurrentMessage <= Table.GetNewestMessage());
            assert(LastMessage >= Table.GetOldestMessage() && LastMessage <= Table.GetNewestMessage());

            Table.SetPrevRoomMsg(CurrentMessage, ID);
            Table.SetNextRoomMsg(LastMessage, ID);

            Table.SetPrevRoomMsg(ID, LastMessage);
            Table.SetNextRoomMsg(ID, CurrentMessage);
            }
        }
#endif

    // find start of message
    fseek(MsgDat, loc, SEEK_SET);
    char c;

    do
        {
        GetByte(&c);
        } while (c != -1/*0xFF*/);

    // get the attribute -- it tells us if we have another attribute byte to get
    char attr1;
    GetByte(&attr1);

    // and get the second attribute byte if it's there
    if (attr1 & ATTR_MORE)
        {
        GetByte(&c);
        }

    // even if we're under 255 now, we still might need to save as two bytes, if we had been over it before
    // note that we only move to DUMP or MAIL, never anywhere else. So we never need to grow from one byte
    // to two. which is good. as that would take much time.
    if (attr1 & ATTR_BIGROOM)
        {
        SaveByte(short_LO(NewRoom * 2));
        SaveByte(short_HI(NewRoom * 2));
        }
    else
        {
        SaveByte((char) NewRoom);
        }

    Table.SetRoomNum(ID, NewRoom);

#if VERSION == ALPHA
    Table.Verify();
#endif

    Unlock();
    }

void MessageDatC::ChangeAttribute(m_index ID, uchar NewAttr)
    {
    Lock();

    if (!Table.IsValid(ID))
        {
        Unlock();
        return;
        }

    long loc = Table.GetLocation(ID);
    if (loc == CERROR)
        {
        Unlock();
        return;
        }

    // find start of message
    fseek(MsgDat, loc, SEEK_SET);
    char c;
    do
        {
        GetByte(&c);
        } while (c != -1/*0xFF*/);

    loc = ftell(MsgDat);

    uchar old_attr;

    GetByte((char *) &old_attr);

    // Create new_attr by combining the old_attr with attr

    uchar new_attr = (char) (NewAttr & (ATTR_RECEIVED | ATTR_REPLY | ATTR_MADEVIS | ATTR_CENSORED));
    new_attr |= (char) (old_attr & (ATTR_COMPRESSED | ATTR_BIGROOM | ATTR_MORE));

    fseek(MsgDat, loc, SEEK_SET);

    // write new attribute
    SaveByte(new_attr);

    Table.GetFlags(ID)->SetReceived(new_attr & ATTR_RECEIVED);
    Table.GetFlags(ID)->SetReply(new_attr & ATTR_REPLY);
    Table.GetFlags(ID)->SetMadevis(new_attr & ATTR_MADEVIS);
    Table.GetFlags(ID)->SetCensored(new_attr & ATTR_CENSORED);

#if VERSION == ALPHA
    Table.Verify();
#endif

    Unlock();
    }


// --------------------------------------------------------------------------
// massdelete(): Sysop fn to kill all msgs from person.

#define MASSDELETEUPDATETIME 5
void TERMWINDOWMEMBER massdelete(void)
    {
    label who;
    char string[256];

    SetDoWhat(SYSMASS);

    getNormStr(cfg.Luser_nym, who, LABELSIZE);

    if (!*who)
        {
        return;
        }

    sprintf(string, getsysmsg(87), cfg.Lmsgs_nym, who);

    if (getYesNo(string, 0))
        {
        time_t LastProgressTime = time(NULL);

        const int namehash = hash(who);
        int killed = 0;

        for (m_index ID = MessageDat.NewestMessage(); ID >= MessageDat.OldestMessageInTable(); ID--)
            {
            if (MessageDat.GetAuthHash(ID) == namehash)
                {
                // maybe check actual msg too?
                if (MessageDat.GetRoomNum(ID) != DUMP)
                    {
                    ++killed;
                    MessageDat.ChangeRoom(ID, DUMP);
                    }
                }

            if (!(ID & 15)) // 15? 15? 15? Why didn't you pick 69??? :)
                {
                if (BBSCharReady())
                    {
#ifdef MULTI
                    OC.CheckInput(FALSE, this);
#else
                    OC.CheckInput(FALSE);
#endif

                    if (OC.User.GetOutFlag() == OUTSKIP)
                        {
                        OC.SetOutFlag(OUTOK);
                        break;
                        }
                    }

                if (LastProgressTime + MASSDELETEUPDATETIME <= time(NULL))
                    {
                    mPrintfCR(getsysmsg(271), cfg.Lmsg_nym, ltoac(ID));
                    LastProgressTime = time(&LastActiveTime);
                    }
                }
            }

        CRCRmPrintfCR(getsysmsg(88), ltoac(killed), killed == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym);

        if (killed)
            {
#ifdef WINCIT
            trap(T_SYSOP, WindowCaption, getsysmsg(89), who);
#else
            trap(T_SYSOP, getsysmsg(89), who);
#endif
            Talley->Fill();
            }
        }
    }


// --------------------------------------------------------------------------
// AideCopyMessage(): Aide fn to copy message to buffer.

void TERMWINDOWMEMBER AideCopyMessage(void)
    {
    if (!LockMenu(MENU_AIDE))
        {
        return;
        }

    char prompt[80];

    SetDoWhat(AIDEMSGIN);

    label Oldest;
    CopyStringToBuffer(Oldest, ltoac(MessageDat.OldestMessageInTable()));

    CRmPrintfCR(getmsg(392), Oldest, ltoac(MessageDat.NewestMessage()));

    sprintf(prompt, getmenumsg(MENU_AIDE, 154), cfg.Lmsg_nym);
    const long msgno = getNumber(prompt, 0l, LONG_MAX, -1l, FALSE, NULL);

    if (msgno <= 0)
        {
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!MessageDat.IsValid(msgno))
        {
        mPrintfCR(getmsg(391));
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (CurrentUser->MaySeeMessage(msgno) != MSM_GOOD)
        {
        mPrintf(getmenumsg(MENU_AIDE, 155), cfg.Umsg_nym);
        }
    else
        {
        delete MS.AbortedMessage;
        MS.AbortedMessage = new Message;

        if (MS.AbortedMessage)
            {
            dowhattype oldDowhat;

            MRO.Verbose = TRUE;

            MRO.DotoMessage = NO_SPECIAL;

            oldDowhat = DoWhat;
            SetDoWhat(READMESSAGE);
            MRO.DotoMessage = NO_SPECIAL;

            OC.SetOutFlag(OUTOK);
            PrintMessageByID(msgno);

            SetDoWhat(oldDowhat);

            MessageDat.LoadAll(msgno, MS.AbortedMessage);

#ifdef WINCIT
            MS.AbortedMessage->Decompress(/*this*/);
#else
            MS.AbortedMessage->Decompress();
#endif

            MS.AbortedMessage->ClearHeader();
            }
        else
            {
            OutOfMemory(69);
            }
        }

    OC.SetOutFlag(OUTOK);
    doCR();
    UnlockMenu(MENU_AIDE);
    }


void TERMWINDOWMEMBER AideInsert(void)
    {
    doCR();
    insert(MS.MarkedID);
    }

void TERMWINDOWMEMBER AideQueueMove(void)
    {
    if (AideQueueKill())
        {
        AideQueueInsert();
        }
    }

void TERMWINDOWMEMBER AideQueueSort(void)
    {
    if (LockMenu(MENU_AIDEQUEUE))
        {
        SetDoWhat(AIDEQUEUES);

        doCR();

        // sort forward
        if (getYesNo(getmenumsg(MENU_AIDEQUEUE, 10), 0))
            {
            sortmsglist(0);
            }
        // sort reverse
        else if (getYesNo(getmenumsg(MENU_AIDEQUEUE, 11), 0))
            {
            sortmsglist(1);
            }

        UnlockMenu(MENU_AIDEQUEUE);
        }
    }
