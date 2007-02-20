// --------------------------------------------------------------------------
// Citadel: MsgTab.CPP
//
// Message table related code.

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "filemake.h"


// --------------------------------------------------------------------------
// MsgTableC::Index(): Builds one message index from passed message.

Bool MessageTableC::Index(Message *Msg)
	{
	if (roomBuild)
		{
		buildroom(Msg);
		}

	m_index OurID = atol(Msg->GetLocalID());
	m_slot OurSlot = OurID - OldestMsgTab;

	// Should we handle this gracefully in the debugging version?
	assert(OurSlot >= 0 && OurSlot < Size);

	// This is about as gracefully as we can handle it.
	if (!OurSlot < 0 || OurSlot >= Size)
		{
		return (FALSE);
		}

#ifdef REGULAR
	*((int *) &(Flags[OurSlot])) = 0;
	AuthHash[OurSlot] = 0;
	ToHash[OurSlot] = 0;
	OriginID[OurSlot] = 0;
	RoomNum[OurSlot] = DUMP;
	LocLO[OurSlot] = long_LO(Msg->GetHeadLoc());
	LocHI[OurSlot] = long_HI(Msg->GetHeadLoc());

	#define smt(x)	x[OurSlot]

#else
	MsgTabEntryS *MT =
	#ifdef AUXMEM
			(MsgTabEntryS *) (LoadAuxmemBlock(OurSlot, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)));
	#else
			&Table[OurSlot];
	#endif

	*((int *) (&MT->Flags)) = 0;
	MT->AuthHash = 0;
	MT->ToHash = 0;
	MT->OriginID = 0;
	MT->RoomNum = DUMP;
	MT->Loc = Msg->GetHeadLoc();

	#define smt(x)	MT->x

#endif

	if (*Msg->GetSourceID())
		{
		m_index oid = atol(Msg->GetSourceID());

		smt(OriginID) = (ushort) oid;
		smt(Flags).SetNetworked(TRUE);
		}

	smt(AuthHash) = hash(Msg->GetAuthor());

	if (*Msg->GetToUser() && *Msg->GetGroup() && (SameString(Msg->GetToUser(), Msg->GetGroup())))
		{
		smt(Flags).SetMassemail(TRUE);
		smt(ToHash) = hash(Msg->GetGroup());
		}
	else
		{
		if (*Msg->GetToUser())
			{
			smt(ToHash) = hash(Msg->GetToUser());
			smt(Flags).SetMail(TRUE);

			if (*Msg->GetForward())
				{
				smt(Flags).SetForwarded(TRUE);

				if (!*Msg->GetSourceID())
					{
					// use the mtomesg field
					smt(OriginID) = hash(Msg->GetForward());
					}
				else
					{
					// use the mttohash and lose the recipient
					smt(ToHash) = hash(Msg->GetForward());
					}
				}
			}

		if (*Msg->GetGroup())
			{
			smt(ToHash) = hash(Msg->GetGroup());
			smt(Flags).SetLimited(TRUE);
			}
		}

	if (Msg->IsNetworkedMail() && !Msg->IsNetworkedMailForHere())
		{
		smt(Flags).SetNet(TRUE);
		}

	if (*Msg->GetX() == 'Y')
		{
		smt(Flags).SetProblem(TRUE);
		}

	if (*Msg->GetX() == 'M')
		{
		smt(Flags).SetModerated(TRUE);
		}

	smt(Flags).SetReceived(Msg->IsReceived());
	smt(Flags).SetReply(Msg->IsRepliedTo());
	smt(Flags).SetCensored(Msg->IsCensored());
	smt(Flags).SetMadevis(Msg->IsMadeVisible());
	smt(Flags).SetLocal(Msg->IsLocal());
	smt(Flags).SetInuse(TRUE);

	smt(RoomNum) = Msg->GetRoomNumber();

	// This is special.
	if	(*Msg->GetCopyOfMessage())
		{
		smt(Flags).SetCopy(TRUE);

		// get the ID#
		m_index copy = atol(Msg->GetCopyOfMessage());

		smt(ToHash) = long_LO(OurID - copy);
		smt(AuthHash) = long_HI(OurID - copy);
		}

#if defined(AUXMEM) || defined(WINCIT)
	r_slot RoomNum = MT->RoomNum;

	assert(RoomNum < cfg.maxrooms && RoomNum >= 0);

	// insert it into room list in message table
	if (FirstMessageInRoom[RoomNum] == M_INDEX_ERROR)
		{
		// first message in this room...
		assert(LastMessageInRoom[RoomNum] == M_INDEX_ERROR);

		MT->PrevRoomMsg = M_INDEX_ERROR;
		MT->NextRoomMsg = M_INDEX_ERROR;

		FirstMessageInRoom[RoomNum] = OurID;
		LastMessageInRoom[RoomNum] = OurID;
		}
	else
		{
		assert(LastMessageInRoom[RoomNum] != M_INDEX_ERROR);
		assert(FirstMessageInRoom[RoomNum] != M_INDEX_ERROR);

		if (OurID < FirstMessageInRoom[RoomNum])
			{
			// this message is older then the old first message in room...
			MT->NextRoomMsg = FirstMessageInRoom[RoomNum];
			MT->PrevRoomMsg = M_INDEX_ERROR;

			// remember: if we put more than one other slot reference
			// here, we may lose MT in the Auxmem version, as it may have
			// to be swapped out.
			assert(IsValid(FirstMessageInRoom[RoomNum]));
			SetPrevRoomMsg(FirstMessageInRoom[RoomNum], OurID);

			FirstMessageInRoom[RoomNum] = OurID;
			}
		else if (OurID > LastMessageInRoom[RoomNum])
			{
			// this message is newer than the old last message in room...
			MT->PrevRoomMsg = LastMessageInRoom[RoomNum];
			MT->NextRoomMsg = M_INDEX_ERROR;

			// remember: if we put more than one other slot reference
			// here, we may lose MT in the Auxmem version, as it may have
			// to be swapped out.
			assert(IsValid(LastMessageInRoom[RoomNum]));
			SetNextRoomMsg(LastMessageInRoom[RoomNum], OurID);

			LastMessageInRoom[RoomNum] = OurID;
			}
		else
			{
			// insert it somewhere...
			m_index CurrentMessage, LastMessage;

			for (CurrentMessage = FirstMessageInRoom[RoomNum];
					CurrentMessage < OurID;
					LastMessage = CurrentMessage, assert(IsValid(CurrentMessage)),
					CurrentMessage = GetNextRoomMsg(CurrentMessage))
				{
				assert(CurrentMessage != M_INDEX_ERROR);
				}

			assert(LastMessage != OurID);

			// insert slot between LastMessage and CurrentMessage
			// remember: MT is no longer good in Auxmem, because
			// getNextRoomMsg() may have swapped it out of memory.

			assert(IsValid(CurrentMessage));
			SetPrevRoomMsg(CurrentMessage, OurID);

			assert(IsValid(LastMessage));
			SetNextRoomMsg(LastMessage, OurID);

			SetPrevRoomMsg(OurID, LastMessage);
			SetNextRoomMsg(OurID, CurrentMessage);
			}
		}
#endif

	if (OurID > NewestMsg)
		{
		NewestMsg = OurID;
		}

#if VERSION == ALPHA
	Verify();
#endif

	return (TRUE);
	}


#ifndef NDEBUG
#define CHECKINTERVAL 180*60		// how often to check table, maximum

#include "cwindows.h"

void MessageTableC::Verify(void)
	{
#ifdef GOODBYE
	static time_t LastChecked;

	if (time(NULL) >= LastChecked + CHECKINTERVAL)
		{
		CITWINDOW *w = CitWindowsMsg(NULL, "Verifying message table...");

		#if defined(AUXMEM) || defined(WINCIT)
			m_slot TableSize = sizetable();
			m_slot CurrentMsg, LastMsg;
			r_slot CurrentRoom;

			for (CurrentMsg = 0; CurrentMsg < TableSize; CurrentMsg++)
				{
				getMsgTab(CurrentMsg)->Checked = !getFlags(CurrentMsg)->IsInuse();
				}

			for (CurrentRoom = 0; CurrentRoom < cfg.maxrooms; CurrentRoom++)
				{
				if (FirstMessageInRoom[CurrentRoom] != M_SLOT_ERROR)
					{
					for (LastMsg = M_SLOT_ERROR, CurrentMsg = FirstMessageInRoom[CurrentRoom];

							CurrentMsg != M_SLOT_ERROR;

							LastMsg = CurrentMsg, CurrentMsg = getNextRoomMsg(indexslot(CurrentMsg)))
						{
						m_slot CMSlot = indexslot(CurrentMsg);

						if (CMSlot == M_SLOT_ERROR)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}

#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld", CurrentMsg);
							doccr();
#endif
							assert(FALSE);
							}

						if (getMsgTab(CMSlot)->Checked)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}

#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld", CurrentMsg);
							doccr();
#endif
							assert(FALSE);
							}

						if (getRoomNum(CMSlot) != CurrentRoom)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}

#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld; room: %d:%d", CurrentMsg, CurrentRoom, getRoomNum(CMSlot));
							doccr();
#endif
							assert(FALSE);
							}

						getMsgTab(CMSlot)->Checked = TRUE;
						}

					if (LastMessageInRoom[CurrentRoom] != LastMsg)
						{
						if (w)
							{
							destroyCitWindow(w, FALSE);
							}

#ifndef WINCIT
						doccr();
						cPrintf("LMIR: %ld, LM: %ld, room: %d", LastMessageInRoom[CurrentRoom], LastMsg, CurrentRoom);
						doccr();
#endif
						assert(FALSE);
						}
					}
				else
					{
					if (LastMessageInRoom[CurrentRoom] != M_SLOT_ERROR)
						{
						if (w)
							{
							destroyCitWindow(w, FALSE);
							}

#ifndef WINCIT
						doccr();
						cPrintf("LMIR: %ld, Room: %d", LastMessageInRoom[CurrentRoom], CurrentRoom);
						doccr();
#endif
						assert(FALSE);
						}
					}
				}

			for (CurrentMsg = 0; CurrentMsg < TableSize; CurrentMsg++)
				{
				if (!getMsgTab(CurrentMsg)->Checked)
					{
					if (w)
						{
						destroyCitWindow(w, FALSE);
						}

#ifndef WINCIT
					doccr();
					cPrintf("Bad: %ld", CurrentMsg);
					doccr();
#endif
					assert(FALSE);
					}

				getMsgTab(CurrentMsg)->Checked = !getFlags(CurrentMsg)->IsInuse();
				}

			for (CurrentRoom = 0; CurrentRoom < cfg.maxrooms; CurrentRoom++)
				{
				if (LastMessageInRoom[CurrentRoom] != M_SLOT_ERROR)
					{
					for (LastMsg = M_SLOT_ERROR, CurrentMsg = LastMessageInRoom[CurrentRoom]; CurrentMsg != M_SLOT_ERROR;
							LastMsg = CurrentMsg, CurrentMsg = getPrevRoomMsg(indexslot(CurrentMsg)))
						{
						m_slot CMSlot = indexslot(CurrentMsg);

						if (CMSlot == M_SLOT_ERROR)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}

#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld", CurrentMsg);
							doccr();
#endif
							assert(FALSE);
							}

						if (getMsgTab(CMSlot)->Checked)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}

#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld", CurrentMsg);
							doccr();
#endif
							assert(FALSE);
							}

						if (getRoomNum(CMSlot) != CurrentRoom)
							{
							if (w)
								{
								destroyCitWindow(w, FALSE);
								}
#ifndef WINCIT
							doccr();
							cPrintf("Bad: %ld; room: %d:%d", CurrentMsg, CurrentRoom, getRoomNum(CMSlot));
							doccr();
#endif
							assert(FALSE);
							}

						getMsgTab(CMSlot)->Checked = TRUE;
						}

					if (FirstMessageInRoom[CurrentRoom] != LastMsg)
						{
						if (w)
							{
							destroyCitWindow(w, FALSE);
							}

#ifndef WINCIT
						doccr();
						cPrintf("LMIR: %ld, LM: %ld, room: %d", LastMessageInRoom[CurrentRoom], LastMsg, CurrentRoom);
						doccr();
#endif
						assert(FALSE);
						}
					}
				}

			for (CurrentMsg = 0; CurrentMsg < TableSize; CurrentMsg++)
				{
				if (!getMsgTab(CurrentMsg)->Checked)
					{
					if (w)
						{
						destroyCitWindow(w, FALSE);
						}

#ifndef WINCIT
					doccr();
					cPrintf("Bad: %ld", CurrentMsg);
					doccr();
#endif
					assert(FALSE);
					}
				}
		#else
			m_slot TableSize = sizetable();
			m_slot CurrentMsg;

			for (CurrentMsg = 0; CurrentMsg < TableSize; CurrentMsg++)
				{
				if (w && (getRoomNum(CurrentMsg) >= cfg.maxrooms))
					{
					destroyCitWindow(w, FALSE);
					}

				assert(FALSE);
				}
		#endif

		LastChecked = time(NULL);
		if (w)
			{
			destroyCitWindow(w, FALSE);
			}
		}
#endif
	}
#endif
