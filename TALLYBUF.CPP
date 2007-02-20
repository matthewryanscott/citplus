// --------------------------------------------------------------------------
// Citadel: TallyBuf.CPP
//
// For talley buffer stuff.

#include "ctdl.h"
#pragma hdrstop

#include "auxtab.h"
#include "log.h"
#include "msg.h"
#include "tallybuf.h"
#include "cwindows.h"
#include "termwndw.h"

// --------------------------------------------------------------------------
// Contents
//
// Public
//
//	Functions:

TalleyBufferCollectionC TalleyBufferCollection;

void TalleyBufferC::ResetBypassedAndEntered(void)
	{
    for (r_slot i = 0; i < Size; i++)
        {
        SetBypass(i, FALSE);
        SetEnteredInRoom(i, 0);
        }
	}
// --------------------------------------------------------------------------
// CreateTalleyBuffer(): Creates it for the current task.
#ifndef WINCIT
TalleyBufferC::TalleyBufferC(void)
    {
#else
TalleyBufferC::TalleyBufferC(r_slot NewSize, LogEntry *NewUser)
	{
	assert(NewSize);

	Size = NewSize;
	User = NewUser;
#endif

#ifdef AUXMEM

    assert(talleyInfo == NULL);
#ifndef AUXMEM
    assert(cfg.maxrooms != NULL);
#endif

    Size = cfg.maxrooms;
    User = CurrentUser;

    talleyInfo = new talleyTaskInfo;

    if(talleyInfo)
        {
        ((talleyTaskInfo *) (talleyInfo))->talleyBlocksInHeap = 0;
        ((talleyTaskInfo *) (talleyInfo))->tbList = NULL;

        for (r_slot i = 0; i < cfg.maxrooms; i += TALLEYBUFPERPAGE)
            {
            printf("%u", i);
            if (!AddAuxmemBlock(&(((talleyTaskInfo *) (talleyInfo))->tbList),
					i,
                    &(((talleyTaskInfo *) (talleyInfo))->talleyBlocksInHeap),
					1))
                {
                    printf("uh-oh....");
                    DeleteAuxmemList(&(((talleyTaskInfo *) (talleyInfo))->tbList),
                        &(((talleyTaskInfo *) (talleyInfo))->talleyBlocksInHeap));


                delete talleyInfo;

                break;
                }
            }
#else
	Buffer = new TalleyRoomInfoS[NewSize];
#endif

        printf("is good?");
        if (IsGood())
            {
            printf("guess so...");
#ifndef WINCIT
            for (r_slot i = 0; i < cfg.maxrooms; i++)
#else
            for (r_slot i = 0; i < NewSize; i++)
#endif
                {
                SetBypass(i, FALSE);
                SetEnteredInRoom(i, 0);
        
                SetRoomTotal(i, 0);
                SetRoomMessages(i, 0);
                SetRoomNewMessages(i, 0);
                SetHasNewMail(i, FALSE);
                SetHasOldMail(i, FALSE);
                SetVisited(i, FALSE);
                }

#ifdef WINCIT
            TalleyBufferCollection.Add(this);
#endif
            printf("looks good from here!");
            }
#ifndef WINCIT
        }
#endif
    }


// --------------------------------------------------------------------------
// DestroyTalleyBuffer(): Destroys it for the current task.

TalleyBufferC::~TalleyBufferC()
	{
#ifdef WINCIT
	TalleyBufferCollection.Remove(this);
#endif

	#ifdef AUXMEM
        assert(talleyInfo != NULL);
        DeleteAuxmemList(&(((talleyTaskInfo *) (talleyInfo))->tbList),
                &(((talleyTaskInfo *) (talleyInfo))->talleyBlocksInHeap));
        delete talleyInfo;
        talleyInfo = NULL;
    #else
		delete [] Buffer;
	#endif
	}

// --------------------------------------------------------------------------
// roomtalley(): Talleys up total, messages & new for every room.

void TalleyBufferC::Fill(void)
	{
	assert(User);

	sysMail = FALSE;

	r_slot room;

	for (room = 0; room < Size; room++)
		{
		GetEntry(room)->Total = 0;
		GetEntry(room)->Messages = 0;
		GetEntry(room)->New = 0;
		GetEntry(room)->HasMail = FALSE;
		GetEntry(room)->HasOldMail = FALSE;
		GetEntry(room)->Visited = FALSE;
		}

	CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(31));

	label Buffer;
	const int hashSysop = hash(getmsg(386));
	const int hashCfgSysop = hash(cfg.sysop);
	const int hashUser = hash(User->GetName(Buffer, sizeof(Buffer)));

	for (m_index ID = MessageDat.OldestMessageInTable(); ID <= MessageDat.NewestMessage(); ++ID)
		{
		MsgTabEntryS Tab;
		if (MessageDat.GetTabEntry(ID, &Tab))
			{
			room = Tab.RoomNum;
	
			if (room >= Size)
				{
				// if the message is beyond what we know about, pretend it's in Dump> and keep going.
				room = DUMP;
				}

			if (room >= Size)
				{
				// Uh-oh! We don't even know about DUMP!
				room = 0;
				}
	
			if (!Tab.Flags.IsCopy())
				{
				if (cfg.checkSysMail && !sysMail && Tab.Flags.IsMail() && !Tab.Flags.IsNet() && !Tab.Flags.IsReceived())
					{
					// "sysop"
					if ((cfg.checkSysMail & 1) && Tab.ToHash == hashSysop)
						{
						sysMail = TRUE;
						}
	
					// "cfg.sysop"
					if ((cfg.checkSysMail & 2) && Tab.ToHash == hashCfgSysop)
						{
						sysMail = TRUE;
						}
					}
	
				GetEntry(room)->Total++;
				}
			else
				{
				if (Tab.GetCopyOffset() <= ID - MessageDat.OldestMessageInTable())
					{
					GetEntry(room)->Total++;
					}
				}
	
			if (User->MaySeeMessage(ID) == MSM_GOOD)
				{
				GetEntry(room)->Messages++;
	
				if (ID > User->GetRoomNewPointer(room))
					{
					GetEntry(room)->New++;
	
					// check to see if it is private mail and set flag if so
					if (Tab.Flags.IsMail() || Tab.Flags.IsMassemail())
						{
						GetEntry(room)->HasMail = TRUE;
						}
					}
				else
					{
					// check to see if it is private mail and set flag if so
					if (Tab.Flags.IsMail() && !Tab.Flags.IsNet() && !Tab.Flags.IsReceived() &&
							(Tab.ToHash == hashUser || (User->IsSysop() && Tab.ToHash == hashSysop)))
						{
						GetEntry(room)->HasOldMail = TRUE;
						}
					}
				}
			}
		}

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}
	}

void TalleyBufferCollectionC::MoveMessage(m_index ID, r_slot OldRoom, r_slot NewRoom)
	{
	Lock();

	for (TalleyBufferListS *C = List; C; C = (TalleyBufferListS *) getNextLL(C))
		{
		assert (C->B->IsGood());

		C->B->AddRoomTotal(OldRoom, -1);
		C->B->AddRoomTotal(NewRoom, 1);

		if (C->B->User && C->B->User->MaySeeMessage(ID) == MSM_GOOD)
			{
			C->B->AddRoomMessages(OldRoom, -1);
			C->B->AddRoomMessages(NewRoom, 1);

			if (ID > C->B->User->GetRoomNewPointer(OldRoom))
				{
				C->B->AddRoomNewMessages(OldRoom, -1);
				}

			if (ID > C->B->User->GetRoomNewPointer(NewRoom))
				{
				C->B->AddRoomNewMessages(NewRoom, 1);
				}
			}
		}

	Unlock();
	}

void TalleyBufferCollectionC::NewMessageNoNotify(Message *Msg)
	{
	Lock();

	const r_slot Room = Msg->GetRoomNumber();

	for (TalleyBufferListS *cur = List; cur; cur = (TalleyBufferListS *) getNextLL(cur))
		{
		assert (cur->B->IsGood());

		cur->B->AddRoomTotal(Room, 1);

		if (cur->B->User && cur->B->User->MaySeeMessage(Msg) == MSM_GOOD)
			{
			cur->B->AddRoomMessages(Room, 1);

			if (atol(Msg->GetLocalID()) > cur->B->User->GetRoomNewPointer(Room))
				{
				cur->B->AddRoomNewMessages(Room, 1);
				}
			}
		}

	Unlock();
	}

void TalleyBufferCollectionC::NewMessage(Message *Msg)
	{
    char Event[256];
#ifdef WINCIT
	const r_slot Room = Msg->GetRoomNumber();
#endif
	const Bool Exclusive = Msg->IsMail() || Msg->IsMassEMail();

    if (!Exclusive)
        {
#ifdef WINCIT
        TermWindowCollection.SystemEvent(SE_NEWMESSAGE_SOMEWHERE, Room, Msg, FALSE, cfg.laughter);
#endif
        }


    NewMessageNoNotify(Msg);

	if (Exclusive)
		{
		label RoomName;

		sprintf(Event, getmsg(544), cfg.Lmsg_nym, Msg->GetAuthor(),
				RoomTab[Msg->GetRoomNumber()].GetName(RoomName, sizeof(RoomName)));
		}
	else 
		{
        sprintf(Event, getmsg(166), cfg.Lmsg_nym, Msg->GetAuthor());
		}

#ifdef WINCIT
	TermWindowCollection.SystemEvent(Exclusive ? SE_EXCLUSIVEMESSAGE : SE_NEWMESSAGE, Room, Msg, FALSE, Event);
#endif
	}

void TalleyBufferCollectionC::RemoveMessage(m_index ID)
	{
	Lock();

	const r_slot Room = MessageDat.GetRoomNum(ID);

	for (TalleyBufferListS *cur = List; cur; cur = (TalleyBufferListS *) getNextLL(cur))
		{
		assert (cur->B->IsGood());

		cur->B->AddRoomTotal(Room, -1);

		if (cur->B->User && cur->B->User->MaySeeMessage(ID) == MSM_GOOD)
			{
			cur->B->AddRoomMessages(Room, -1);

			if (ID > cur->B->User->GetRoomNewPointer(Room))
				{
				cur->B->AddRoomNewMessages(Room, -1);
				}
			}
		}

	Unlock();
	}

void TalleyBufferCollectionC::Add(TalleyBufferC *New)
	{
	Lock();

	TalleyBufferListS *cur;
	for (cur = List; cur; cur = (TalleyBufferListS *) getNextLL(cur))
		{
		if (cur->B == New)
			{
			break;
			}
		}

	if (!cur)
		{
		cur = (TalleyBufferListS *) addLL((void **) &List, sizeof(TalleyBufferListS));

		if (cur)
			{
			cur->B = New;
			}
		}

	Unlock();
	}

void TalleyBufferCollectionC::Remove(TalleyBufferC *Old)
	{
	Lock();

	int i;
	TalleyBufferListS *cur;

	for (i = 1, cur = List; cur; cur = (TalleyBufferListS *) getNextLL(cur), i++)
		{
		if (cur->B == Old)
			{
			deleteLLNode((void **) &List, i);
			break;
			}
		}

	Unlock();
	}
