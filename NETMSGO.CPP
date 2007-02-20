// --------------------------------------------------------------------------
// Citadel: NetMsgO.CPP
//
// How to write messages out to the network

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "log.h"
#include "net.h"
#include "filerdwr.h"
#include "events.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// getdupeMessage() 	reads a message off disk into RAM, lim fields
// netPutMessage()		Puts a message to a file
// NewRoom()			Puts all new messages in a room to a file
// saveMessage()		saves a message to file if it is netable
// ReadMsgFile()		Reads a message file into thisRoom


// --------------------------------------------------------------------------
// NewRoom(): Puts all new messages in a room to a file.
//
// filename = "name" for old netting or NULL for 6.9
// file = msg.69 for 6.9 or NULL for old netting

Message *ShouldNetworkMessage(m_index ID, r_slot Room, LogEntry *Log)
	{
	if (!MessageDat.IsValid(ID))
		{
		return (NULL);
		}

	// skip messages not in this room. This loop could be reworked
	// for auxmem/windows with the room list stuff...
	if (MessageDat.GetRoomNum(ID) != Room)
		{
		return (NULL);
		}

	const Bool ReferenceFlag = MessageDat.IsCopy(ID);
	const r_slot ReferenceRoom = MessageDat.GetRoomNum(ID);
	uchar ReferenceAttr =
		(char) (MessageDat.IsReceived(ID)	? ATTR_RECEIVED : 0) |
		(char) (MessageDat.IsReply(ID)		? ATTR_REPLY	: 0) |
		(char) (MessageDat.IsMadevis(ID)	? ATTR_MADEVIS	: 0) |
		(char) (MessageDat.IsCensored(ID)	? ATTR_CENSORED : 0);

	// seek out original message table entry
	m_index OurID = ID;

	while (MessageDat.IsCopy(OurID))
		{
		// copy has scrolled?
		const m_index CopyOffset = MessageDat.GetCopyOffset(OurID);
		assert(CopyOffset);

		if (CopyOffset > OurID - MessageDat.OldestMessageInTable())
			{
			// scrolled
			return (NULL);
			}
		else	// look at original message index
			{
			assert(CopyOffset);

			if (CopyOffset)
				{
				OurID -= CopyOffset;
				}
			else
				{
				// bad table stuff
				return (NULL);
				}
			}
		}

	if (
			// skip mail
			MessageDat.IsMail(OurID) ||

			// skip mass e-mail
			MessageDat.IsMassemail(OurID) ||

			// No problem user messages to network
			((MessageDat.IsProblem(OurID) || MessageDat.IsModerated(OurID)) && !MessageDat.IsMadevis(OurID)) ||

			// skip Local messages
			MessageDat.IsLocal(OurID) ||

			// no open messages from the system
			// In 66 (perhaps 67?) this can go away. (By that time, all non-local
			// from #NODENAME messages will have scrolled.)
			(MessageDat.GetAuthHash(OurID) == hash(cfg.nodeTitle)) ||

			// Simply may not see it!
			(Log->MaySeeMessage(OurID) != MSM_GOOD && !MessageDat.IsNet(OurID))
			)
		{
		return (NULL);
		}

	Message *Msg = new Message;

	if (Msg)
		{
		if (MessageDat.LoadAll(OurID, Msg) != RMS_OK)
			{
			delete Msg;
			return (NULL);
			}

		// Something went bad in the load -- the message we got wasn't what we wanted!
		if (atol(Msg->GetLocalID()) != OurID)
			{
#ifdef GOODBYE
			label Byte;
			CopyStringToBuffer(Byte, ltoac(MessageDat.GetLocation(ID)));
			cPrintf(getmsg(393), cfg.Lmsg_nym, ltoac(ID), Byte);
			doccr();
#endif
			delete Msg;
			return (NULL);
			}

		label Name, Alias, LocID;
		if (BeenThere(Msg->GetFromPath(), Log->GetName(Name, sizeof(Name)), Log->GetAlias(Alias, sizeof(Alias)),
				Log->GetLocID(LocID, sizeof(LocID)), NULL))
			{
			delete Msg;
			return (NULL);
			}

		Msg->Decompress();

		Msg->SetViewDuplicate(ReferenceFlag);
		Msg->SetOriginalID(ID);
		Msg->SetOriginalAttribute(ReferenceAttr);
		Msg->SetOriginalRoom(ReferenceRoom);

		if (*Msg->GetFileLink())
			{
			FILE *LinkedFile;

			if ((LinkedFile = fopen(Msg->GetFileLink(), FO_R)) == NULL)
				{
				delete Msg;
				return (NULL);
				}

			GetFileMessage(LinkedFile, Msg->GetTextPointer(), MAXTEXT);
			fclose(LinkedFile);
			}

		return (Msg);
		}
	else
		{
		cOutOfMemory(77);
		return (NULL);
		}
	}


int TERMWINDOWMEMBER NewRoom(r_slot room, const char *filename, FILE *file)
	{
	int read = MS.Read;
	Bool first = TRUE;

	m_index lowLim = CurrentUser->GetRoomNewPointer(room) + 1;
	const m_index highLim = MessageDat.NewestMessage();

	// stuff may have scrolled off system unseen, so...
	if (MessageDat.OldestMessageInTable() > lowLim)
		{
		lowLim = MessageDat.OldestMessageInTable();
		}

	if (file == NULL)
		{
		char FileName[100];

		sprintf(FileName, sbs, LocalTempPath, filename);

		if ((file = fopen(FileName, FO_AB)) == NULL)
			{
			return (0);
			}
		}

	CurrentUser->SetRoomNewPointer(room, highLim);

	for (m_index ID = lowLim; ID <= highLim; ID++)
		{
		Message *Msg = ShouldNetworkMessage(ID, room, CurrentUser);

		if (Msg)
			{
			PrepareMsgScriptInit(Msg);
#ifndef WINCIT
			doEvent(EVT_STORENETMESSAGE, InitMsgScript);
#endif

			if (CheckMsgScript(Msg))
				{
				if (!filename) // Net 69 stuff
					{
					// Blank room name for net69 because we don't need it
					if (!*Msg->GetToUser())
						{
						if (node->GetNetworkType() != NET_C86)
							{
							Msg->SetCreationRoom(ns);
							}
						}

					if (first && (node->GetNetworkType() != NET_C86))
						{
						label Buffer;
						PutStr(file, RoomTab[room].GetNetID(Buffer, sizeof(Buffer)));
						first = FALSE;
						}
					}

				MS.Read++;

				if (node->GetNetworkType() == NET_C86)
					{
					label Buffer;
					Net86PacketC Net86(file, CurrentUser->GetName(Buffer, sizeof(Buffer)));

					Net86.Store(Msg);
					}
				else
					{
					NetMessagePacketC NetPacket(file, node);

					NetPacket.Store(Msg);
					}
				}

			delete Msg;
			}
		}

	if (filename)
		{
		fclose(file);
		}
	else
		{
		if (!first)
			{
			fputc(0, file);
			}
		}

	return (MS.Read - read);
	}
