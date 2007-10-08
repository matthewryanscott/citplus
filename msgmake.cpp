// --------------------------------------------------------------------------
// Citadel: MsgMake.CPP
//
// Message making code.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "log.h"
#include "events.h"
#include "filerdwr.h"
#include "extmsg.h"
#ifdef WINCIT
#include "net6969.h"
#else
#include "net.h"
#endif



// --------------------------------------------------------------------------
// Contents
//
// systemMessage()			save messages from node
// putMessage() 			stores a message to disk
// crunchmsgTab()			obliterates slots at the beginning of table
// noteMessage()			puts message in mesgBuf into message index
// Message::StoreAsStub()	Saves message as a stub


Bool MessageDatC::RawSaveByte(char Byte)
	{
	const long pos = ftell(MsgDat);

	if (pos == SizeInBytes)
		{
		fseek(MsgDat, 0l, SEEK_SET);
		}
	else 
		{
		fseek(MsgDat, 0l, SEEK_CUR);
        }

	return(fwrite(&Byte, sizeof(Byte), 1, MsgDat) == 1);
	}

Bool MessageDatC::SaveByte(char Byte)
	{
	PrepareToOverwrite(1);
	return (RawSaveByte(Byte));
	}

Bool MessageDatC::SaveString(const char *String)
	{
	const char *s;

	PrepareToOverwrite(strlen(String) + 1); // the '+1' is for the null

	for (s = String; *s; s++)
		{
		RawSaveByte(*s);
		}

	return(RawSaveByte(0));
	}

// --------------------------------------------------------------------------
// Message::StoreAsStub(): Saves message as a stub.

Bool MessageDatC::StoreAsStub(Message *Msg, const char *TypeStr, Bool FullMsg)
	{
	Message *Stub = new Message;

	if (Stub)
		{
		if (FullMsg)
			{
			*Stub = *Msg;

			char ToString[128];
			sprintf(ToString, getmsg(101), Msg->GetToUser(), Msg->GetToNodeName());

			Stub->AddComment(ToString);

			Stub->SetToUser(ns);
			Stub->SetToNodeName(ns);
			}
		else
			{
			// this is SODA
			Stub->SetAuthor(deansi(Msg->GetAuthor()));
			Stub->SetCreationTime(Msg->GetCreationTime());
			Stub->SetOriginNodeName(deansi(Msg->GetOriginNodeName()));
			Stub->SetSourceID(Msg->GetSourceID());

			Stub->SetTextWithFormat(getmsg(462), TypeStr);
			}

		// set room# and attribute byte for message
		Stub->SetRoomNumber(DUMP);
		Stub->SetAttribute(0);

		// Save the stub
		Store(Stub);

		delete Stub;

		return (TRUE);
		}
	else
		{
		OutOfMemory(57);

		return (FALSE);
		}
	}


Bool MessageDatC::Store(Message *Msg)
	{
	Lock();

	Msg->SetLocalID(NewestMessage() + 1);

	if (!Msg->GetOriginalID())
		{
		Msg->SetOriginalID(NewestMessage() + 1);		// For censoring
		}

	Bool RetVal = StoreCommon(Msg);

	if (RetVal)
		{
		TalleyBufferCollection.NewMessage(Msg);
		}

	// Fix up table before indexing... start removing any messages overwritten by this save
	m_slot ToCrunch = 0;
	if (OldestMessageInTable() < OldestMessageInFile())
		{
		ToCrunch = (m_slot) (OldestMessageInFile() - OldestMessageInTable());
		}

	// then, make sure that there is room for this entry
	if ((atol(Msg->GetLocalID()) - OldestMessageInTable() - ToCrunch) >= Table.GetSize())
		{
		ToCrunch++;
		}

	if (ToCrunch)
		{
		Table.Crunch(ToCrunch);
		}

	Table.Index(Msg);

	Unlock();

#ifdef WINCIT
	Net6969.Propogate(Msg);
#endif

	return (RetVal);
	}

Bool MessageDatC::ResizeStore(Message *Msg)
	{
	Lock();

	Bool RetVal = StoreCommon(Msg);

	Unlock();

	return (RetVal);
	}

// Must Lock() before calling (and Unlock() afterwords, of course)
Bool MessageDatC::StoreCommon(Message *Msg)
	{
	Msg->SetHeadLoc(catLoc);

	fseek(MsgDat, catLoc, SEEK_SET);

	const Bool RetVal = MessageStoreC::Store(Msg);

	fflush(MsgDat);

	// record where to begin writing next message
	catLoc = ftell(MsgDat);

	return (RetVal);
	}

Bool MessageStoreC::Store(Message *Msg)
	{
#ifndef WINCIT
	if (NetPacket)
		{
		KBReady();
		}
#endif

	SaveByte(0xFF);

	if (*Msg->GetEncryptionKey())
		{
		if (encrypt(Msg->GetTextPointer(), Msg->GetEncryptionKey()))
			{
			Msg->SetEncrypted(TRUE);
			}
		else
			{
			Msg->SetEncryptionKey(ns);
			}
		}

	// try compressing the message if it isn't already
	if (Compressable)
		{
		Msg->Compress();
		}

	// networked messages can only be received or replied to.
	if (NetPacket)
		{
		Msg->Attribute = (char) (Msg->Attribute & (ATTR_RECEIVED | ATTR_REPLY));
		}

	SaveByte(Msg->Attribute);

	if (Msg->Attribute & ATTR_MORE)
		{
		SaveByte(Msg->Attribute2);
		}

	if (!NetPacket)
		{
		// write room #
		if (Msg->IsBigRoom())
			{
			SaveByte(short_LO(Msg->GetRoomNumber() * 2));
			SaveByte(short_HI(Msg->GetRoomNumber() * 2));
			}
		else
			{
			SaveByte((char) Msg->GetRoomNumber());
			}
		}

	// write message ID; if has no ID, give it 1. (was: LESAVEd messages have no ID; fake one.)
	if (*Msg->GetLocalID())
		{
		SaveString(Msg->GetLocalID());
		}
	else
		{
		SaveString("1");
		}

	if (!NetPacket)
		{
		if (!*Msg->GetCopyOfMessage())
			{
			// setup time/datestamp
			if (!*Msg->GetCreationTime())
				{
				label Buffer;
				Msg->SetCreationTime(ltoa(time(NULL), Buffer, 10));
				}

			// write room name out
			Msg->SetCreationRoom();
			}
		else
			{
			Msg->SetCreationTime(ns);
			Msg->SetCreationRoom(ns);
			}
		}

	if (NetPacket)
		{
		if (!*Msg->GetSourceID())
			{
            label dude;
			Msg->SetOriginNodeName(cfg.nodeTitle);
			Msg->SetOriginRegion(cfg.nodeRegion);
			Msg->SetOriginCountry(cfg.nodeCountry);
			Msg->SetSourceID(Msg->GetLocalID());

            RoomTab[Msg->GetRoomNumber()].GetName(dude, sizeof(dude));
            Msg->SetSourceRoomName(dude);

			char Buffer[128];
			strcpy(Buffer, programName);
			strcat(Buffer, version);
			Msg->SetOriginSoftware(Buffer);

			Msg->SetTwitRegion(cfg.twitRegion);
			Msg->SetTwitCountry(cfg.twitCountry);

			Msg->SetCit86Country(cfg.Cit86Country);

			Msg->SetOriginPhoneNumber(cfg.nodephone);
			}

		if (*Msg->GetFromPath())
			{
			char TempPath[PATHSIZE + 1];
			int i, j;
			label lastlocID;

			strcpy(TempPath, Msg->GetFromPath());
			strcat(TempPath, getmsg(312));

			// Get last region
			for (i = strlen(TempPath); i && TempPath[i] != '.'; i--);

			for (j = 0, i++; TempPath[i] && TempPath[i] != '!' && j < 4; i++, j++)
				{
				lastlocID[j] = TempPath[i];
				}

			lastlocID[j] = 0;

			// if last region was same as current region just use alias
			if (SameString(lastlocID, cfg.locID))
				{
				strcat(TempPath, cfg.alias);
				}
			else
				{
				strcat(TempPath, cfg.Address);
				}

			Msg->SetFromPath(TempPath);
			}
		else
			{
			Msg->SetFromPath(cfg.Address);
			}

		if (*Msg->GetEZCreationTime())
			{
			label Buffer;
			Msg->SetCreationTime(ltoa(net69_time(Msg->GetEZCreationTime()), Buffer, 10));
			}

		if (!*Msg->GetCreationTime())
			{
			label Buffer;
			Msg->SetCreationTime(ltoa(time(NULL), Buffer, 10));
			}

		// kludge.
#ifndef WINCIT
        if (node && node->IsGateway() && !*Msg->GetEZCreationTime())
			{
			label Buffer;
            Msg->SetEZCreationTime(net69_gateway_time(atol(Msg->GetCreationTime()), Buffer));
			}
#endif
		}

#define SaveField(f,l)	 	\
	{					 	\
	if (*f())			 	\
		{				 	\
		SaveByte(l);	 	\
		SaveString(f()); 	\
		}				 	\
	}


	SaveField(Msg->GetAuthor,				'A');
	SaveField(Msg->GetCreationTime,			'D');
	SaveField(Msg->GetOriginNodeName,		'O');
	SaveField(Msg->GetSourceID,				'S');
	// above four fields (SODA) for net double checking speed

	// 'a' - MavenAnon poop             	'a'

	SaveField(Msg->GetSubject,				'B');

	if (NetPacket)
		{
		SaveField(Msg->GetEZCreationTime,	'd');
		}
	else 
		{
		SaveField(Msg->GetCopyOfMessage, 	'C');
		}

	SaveField(Msg->GetForward,				'F');
	SaveField(Msg->GetGroup, 				'G');
	SaveField(Msg->GetReplyToMessage,		'I');
	SaveField(Msg->GetTwitRegion,			'J');
	SaveField(Msg->GetTwitCountry,			'j');

	if (!NetPacket)
		{
		SaveField(Msg->GetFileLink,			'L');
		SaveField(Msg->GetLinkedApplication, 'l');
		}

	SaveField(Msg->GetTitle, 				'N');
	SaveField(Msg->GetSurname,				'n');
	SaveField(Msg->GetOriginRegion,			'o');
	SaveField(Msg->GetFromPath,				'P');
	SaveField(Msg->GetToPath,				'p');
	SaveField(Msg->GetOriginCountry, 		'Q');
	SaveField(Msg->GetToCountry, 			'q');
	SaveField(Msg->GetCreationRoom,			'R');
	SaveField(Msg->GetSourceRoomName,		'r');
	SaveField(Msg->GetOriginSoftware,		's');
	SaveField(Msg->GetToUser,				'T');
	SaveField(Msg->GetPublicToUser,			't');
	SaveField(Msg->GetCit86Country,			'U');

	if (!NetPacket)
		{
		SaveField(Msg->GetX, 				'X');
		}

	SaveField(Msg->GetToNodeName,			'Z');
	SaveField(Msg->GetToRegion,				'z');
	SaveField(Msg->GetSignature, 			'.');
	SaveField(Msg->GetUserSignature, 		'_');
	SaveField(Msg->GetRealName,				'!');
	SaveField(Msg->GetOriginPhoneNumber, 	'H');
	SaveField(Msg->GetToPhoneNumber, 		'h');
	SaveField(Msg->GetDestinationAddress,	'>');

	SaveField(Msg->GetMoreFlags, 			'#');

	// put any comments...
	for (strList *sl = Msg->Comments; sl; sl = (strList *) getNextLL(sl))
		{
		SaveByte('%');
		SaveString(sl->string);
		}

	// put any unknowns...
	for (unkLst *lul = Msg->firstUnk; lul; lul = (unkLst *) getNextLL(lul))
		{
		SaveByte(lul->whatField);
		SaveString(lul->theValue);
		}

	// The actual message text is last
	SaveByte('M');
	SaveString(Msg->GetText());

	return (TRUE);
	}


// --------------------------------------------------------------------------
// systemMessage(): Save messages from node.

void systemMessage(Message *Msg)
	{
	Msg->SetAuthor(cfg.nodeTitle);
	Msg->SetLocal(TRUE);
	putAndNoteMessage(Msg, TRUE);
	}


// --------------------------------------------------------------------------
// crunchmsgTab(): Obliterates slots at the beginning of table.

void MessageTableC::Crunch(m_slot howmany)
	{
#ifdef WINCIT
	for (m_slot i = 0; i < howmany; ++i)
		{
		if (Table[i].Flags.IsInuse())
			{
			TalleyBufferCollection.RemoveMessage(OldestMsgTab + i);

			// remove from our list of rooms
			assert(Table[i].PrevRoomMsg == M_INDEX_ERROR);

			const r_slot room = Table[i].RoomNum;
			const m_index FM = Table[i].NextRoomMsg;

			FirstMessageInRoom[room] = FM;

			if (FM == M_INDEX_ERROR)
				{
				// no more messages in this room
				LastMessageInRoom[room] = M_INDEX_ERROR;
				}
			else
				{
				assert(IsValid(FM));

				Table[FM - OldestMsgTab].PrevRoomMsg = M_INDEX_ERROR;
				}
			}
		}

	memmove(Table, &(Table[howmany]), ((cfg.nmessages - howmany) * sizeof(*Table)));
#endif

#ifdef REGULAR
	for (m_slot i = 0; i < howmany; ++i)
		{
		if (Flags[i].IsInuse())
			{
			TalleyBufferCollection.RemoveMessage(OldestMsgTab + i);
			}
		}

	const m_slot total = cfg.nmessages - howmany;
	memmove(Flags, &(Flags[howmany]), (total * sizeof(*Flags)));
	memmove(LocLO, &(LocLO[howmany]), (total * sizeof(*LocLO)));
	memmove(LocHI, &(LocHI[howmany]), (total * sizeof(*LocHI)));
	memmove(RoomNum, &(RoomNum[howmany]), (total * sizeof(*RoomNum)));
	memmove(ToHash, &(ToHash[howmany]), (total * sizeof(*ToHash)));
	memmove(AuthHash, &(AuthHash[howmany]), (total * sizeof(*AuthHash)));
	memmove(OriginID, &(OriginID[howmany]), (total * sizeof(*OriginID)));
#endif

#ifdef AUXMEM
	while (howmany > MSGTABPERPAGE)
		{
        Crunch(MSGTABPERPAGE);
        howmany -= MSGTABPERPAGE;
		}

	for (m_slot i = 0; i < howmany; ++i)
		{
		MsgTabEntryS *lmt = (MsgTabEntryS *) (LoadAuxmemBlock(i, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)));

		if (lmt->Flags.IsInuse())
			{
			TalleyBufferCollection.RemoveMessage(OldestMsgTab + i);

			// remove from our list of rooms
			assert(lmt->PrevRoomMsg == M_INDEX_ERROR);

			const r_slot room = lmt->RoomNum;
			const m_index FM = lmt->NextRoomMsg;

			FirstMessageInRoom[room] = FM;

			if (FM == M_INDEX_ERROR)
				{
				// no more messages in this room
				LastMessageInRoom[room] = M_INDEX_ERROR;
				}
			else
				{
				assert(IsValid(FM));
				GetTabEntry(FirstMessageInRoom[room])->PrevRoomMsg = M_INDEX_ERROR;
				}
			}
		}

	m_slot total = cfg.nmessages - howmany;
    m_slot total2 = (GetNewestMessage() - OldestMsgTab + 1) - howmany;

	for (m_slot i = 0; i < total2; i += MSGTABPERPAGE)
		{
		const m_index Idx1 = OldestMsgTab + i;
		const m_index Idx2 = Idx1 + howmany;

		memmove(GetTabEntry(Idx1), GetTabEntry(Idx2), sizeof(MsgTabEntryS) * (MSGTABPERPAGE - (uint) howmany));

		if (i + MSGTABPERPAGE < total)
			{
			memcpy(GetTabEntry(Idx1 + MSGTABPERPAGE - howmany), GetTabEntry(Idx1 + MSGTABPERPAGE),
				   sizeof(MsgTabEntryS) * (uint) howmany);
			}
		}
#endif

	OldestMsgTab += howmany;
	}


// --------------------------------------------------------------------------
// MessageDatC::PrepareToOverwrite(): Checks for any overwriting of old messages.
void MessageDatC::PrepareToOverwrite(int bytes)
	{
	const long pos = ftell(MsgDat);

	fseek(MsgDat, 0, SEEK_CUR);
	 
	for (int i = 0; i < bytes; ++i)
		{
		char Byte;
		GetByte(&Byte);

		if (Byte == -1) // obliterating a message
			{
			++OldestMessage;
			}
		}

	fseek(MsgDat, pos, SEEK_SET);
	}


// because putMessage and noteMessage are different. (silly)
static void checkAutoArchive(Message *Msg)
	{
	if (RoomTab[Msg->GetRoomNumber()].IsArchive() && !*Msg->GetToUser())
		{
#ifndef WINCIT
		RoomC RoomBuffer;

		RoomBuffer.Load(Msg->GetRoomNumber());

		label ArchiveFile;
		RoomBuffer.GetArchiveFile(ArchiveFile, sizeof(ArchiveFile));

		if (*ArchiveFile)
			{
			if (!journalfl)
				{
				char FileName[128];

				sprintf(FileName, sbs, cfg.homepath, ArchiveFile);

				if ((journalfl = fopen(FileName, FO_A)) != NULL)
					{
					MRO.Verbose = TRUE;
					const int savecrt = OC.CrtColumn;
					Bool CouldNotLock = FALSE;

					Message *LoadedMsg = LoadMessageByID(atol(Msg->GetLocalID()), FALSE, FALSE);

					if (LoadedMsg)
						{
						LoadedMsg->Decompress();

						if (LockMessages(MSG_READMSG))
							{
							ActuallyDisplayMessage(LoadedMsg, TRUE, TRUE, TRUE);

							UnlockMessages(MSG_READMSG);
							}
						else
							{
							CouldNotLock = TRUE;
							}

						delete LoadedMsg;
						}

					OC.CrtColumn = savecrt;
					fclose(journalfl);

					journalfl = NULL;

					// LoadedMsg is still NULL or old valid address. Check
					// out of memory here; need to have no journalfl first.
					if (!LoadedMsg || CouldNotLock)
						{
						OutOfMemory(58);
						}
					}
				else
					{
					mPrintf(getmsg(557));
					}
				}
			else
				{
				DebugOut(50);
				}
			}
		else
			{
			DebugOut(51);
			}
#endif
		}
	}

Bool putAndNoteMessage(Message *Msg, Bool censor)
	{
#ifndef WINCIT
	PrepareMsgScriptInit(Msg);

	doEvent(EVT_STOREMESSAGE, InitMsgScript);

	if (CheckMsgScript(Msg))
		{
#endif
		Bool ret = FALSE;

		MessageDat.Store(Msg);

		if (censor)
			{
			ret = censor_message(Msg);
			}

		checkAutoArchive(Msg);

		return (ret);
#ifndef WINCIT
		}
	else
		{
		return (FALSE);
		}
#endif
	}

void TERMWINDOWMEMBER msgtosysop(Message *Msg)
	{
	char string[64];

	if (!HaveConnectionToUser() || CurrentUser->IsMainSysop() || CurrentUser->IsNode())
		{
		return;
		}

	sprintf(string, getmsg(495), cfg.Lmsg_nym);

	if (getYesNo(string, 1))
		{
		Msg->SetToUser(getmsg(386));

		makeMessage(Msg, NULL);
		}
	}
