// --------------------------------------------------------------------------
// Citadel: NetMsgI.CPP
//
// How to read messages in from the network

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "log.h"
#include "group.h"
#include "net.h"
#include "maillist.h"
#include "events.h"
#include "extmsg.h"
#include "netcmd.h"


// --------------------------------------------------------------------------
// Contents
//
// Cit86_GetMessage()
// netGetMessage()		Gets a message from a file, returns sucess
// PutMessage() 		Puts a message to a file
// NewRoom()			Puts all new messages in a room to a file
// ReadMsgFl()			Reads a message file into thisRoom

const char *NodesCitC::ThereToHereGroupName(const char *GroupName) const
	{
	assert(this);

	for (nodeGroupList *theGroup = MappedGroups; theGroup; theGroup = (nodeGroupList *) getNextLL(theGroup))
		{
		if (SameString(theGroup->there, GroupName))
			{
			return (theGroup->here);
			}
		}

	return (NULL);
	}

static Bool BadStr(const char *str)
	{
	for (; *str; str++)
		{
		if (*str > 1 && *str < 32)
			{
			return (TRUE);
			}
		}
	return (FALSE);
	}

extern const char *netcmdTmp;

void Message::FixupNetwork(NodesCitC *N)
	{
	if (N)
		{
		if (!*GetLocalID())
			{
			// assign it something...
			SetLocalID(GetCreationTime());

			if (!*GetLocalID())
				{
				// anything at all at this point...
				SetLocalID(getnetmsg(120));
				}
			}

		if (!*GetOriginNodeName())
			{
			SetOriginNodeName(N->GetName());
			}

		if (!*GetOriginRegion())
			{
			SetOriginRegion(N->GetLocID());
			}

		if (!*GetSourceID())
			{
			SetSourceID(GetLocalID());
			}

		if (!isNumeric(GetSourceID()) ||
				BadStr(GetAuthor()) ||
				BadStr(GetSurname()) ||
				BadStr(GetTitle()) ||
				BadStr(GetOriginNodeName()) ||
				BadStr(GetOriginRegion()) ||
				BadStr(GetOriginCountry()) ||
				BadStr(GetTwitRegion()) ||
				BadStr(GetTwitCountry()) ||
				BadStr(GetOriginPhoneNumber()) ||
				BadStr(GetCit86Country()) ||
				BadStr(GetOriginSoftware()))
			{
			// this is a bit of a kludge.
			SetAuthor(ns); // easy way to call it a duplicate
			}

		// If the other node did not set up a from path, do it.
		if (!*GetFromPath())
			{
			if (SameString(GetOriginNodeName(), N->GetName()))
				{
				SetFromPath(GetOriginNodeName());
				}
			else
				{
				// last node did not originate, make due with what we got...
				char Buffer[128];

				sprintf(Buffer, getnetmsg(119), GetOriginNodeName(), N->GetName());

				SetFromPath(Buffer);
				}
			}

		if (*GetEZCreationTime())
			{
			label Buffer;
			SetCreationTime(ltoa(net69_time(GetEZCreationTime()), Buffer, 10));
			}

		if (!*GetCreationTime() || !isNumeric(GetCreationTime()))
			{
			// this is a bit of a kludge.
			SetAuthor(ns); // easy way to call it a duplicate
			}
		}
	}


// --------------------------------------------------------------------------
// ReadMsgFl(): Reads a message file into thisRoom.
//
// filename = "name" for old netting or NULL for 6.9
// file = msg.69 for 6.9 or NULL for old netting
// returns # of messages incorporated or CERROR

#ifdef WINCIT
IncorporateStatusE MessageDatC::IncorporateMessage(Message *Msg, r_slot Room, const LogEntry *Log,
		const char *LocalTempPath, l_index SkipLog, NodesCitC *Node)
#else
IncorporateStatusE MessageDatC::IncorporateMessage(Message *Msg, r_slot Room, const LogEntry *Log,
        l_index SkipLog, NodesCitC *Node)
#endif

	{
	Msg->SetRoomNumber(Room);

	if (SameString(cfg.nodeTitle, Msg->GetOriginNodeName()))
		{
		// uh...
		if (SameString(cfg.nodeRegion, Msg->GetOriginRegion()))
			{
			return (IS_DUPLICATE);
			}
		else
			{
			return (IS_DUPLICATE);
			}
		}

	// Throw away authorless messages
	if (!*Msg->GetAuthor())
		{
		return (IS_NOAUTHOR);
		}

	// Expired?
	if (cfg.expire && (labs(atol(Msg->GetCreationTime()) - time(NULL)) > (cfg.expire * SECSINDAY)))
		{
		return (IS_EXPIRED);
		}

	// Don't accept messages into a room a node doesn't have access to
	if (Log && !Log->CanAccessRoom(Room))
		{
		return (IS_NOACCESS);
		}

	// for program structure, it would be best to do this in the for() loop down there; however for speed
	// we do this here.
	Message *DupeTest = new Message;

	if (!DupeTest)
		{
		return (IS_OUTOFMEMORY);
		}

	// We want to lock ourselves from dupe checking until done, in case two identical messages come in simultaneously
	Lock();

	const ushort oid = (ushort) atol(Msg->GetSourceID());

	for (m_index ID = NewestMessage(); ID >= OldestMessageInTable(); ID--)
		{
		if (oid == GetOriginID(ID))
			{
			ReadMessageStatus rms = LoadDupeCheck(ID, DupeTest);

			// Could read the message
			if (rms == RMS_OK

					// and author or origin node the same
					&& (SameString(Msg->GetAuthor(), DupeTest->GetAuthor())
					|| SameString(Msg->GetOriginNodeName(), DupeTest->GetOriginNodeName()))

					// and same time
					&& SameString(Msg->GetCreationTime(), DupeTest->GetCreationTime())

					// and same source id.
					&& SameString(Msg->GetSourceID(), DupeTest->GetSourceID()))
				{
				delete DupeTest;
				Unlock();
				return (IS_DUPLICATE);
				}
			}
		}

	delete DupeTest;

#ifndef WINCIT
	PrepareMsgScriptInit(Msg);
	doEvent(EVT_LOADNETMESSAGE, InitMsgScript);

	if (!CheckMsgScript(Msg))
		{
		Unlock();
		return (IS_SCRIPT);
		}
#endif

	if (Msg->IsNetworkedMail())
		{
		// is mail
		if (!Msg->IsNetworkedMailForHere())
			{
			if (!save_mail(Msg, TRUE, getmsg(390), SkipLog))
				{
				Unlock();
				return (IS_NOSAVENETMAIL);
				}
			else
				{
				Unlock();
				return (IS_SAVENETMAIL);
				}
			}
		else
			{
#ifndef WINCIT
			PrepareMsgScriptInit(Msg);
			doEvent(EVT_READNETMAIL, InitMsgScript);

			if (CheckMsgScript(Msg))
				{
#endif
				// for this system?
				if (SameString(Msg->GetToUser(), cfg.nodeTitle) || SameString(Msg->GetToUser(), cfg.Address))
					{
					// Net command
					char FileName[128];
					sprintf(FileName, sbs, LocalTempPath, netcmdTmp);

					FILE *fl = fopen(FileName, FO_AB);

					if (fl != NULL)
						{
						NetCmdFileC NetCmdFile(fl);
						NetCmdFile.Store(Msg);
						fclose(fl);
						}

                    StoreAsStub(Msg, getmsg(112), cfg.poopdebug);

					Unlock();
					return (IS_NETCOMMAND);
					}
				else
					{
					// Mailing list?
					if (IsMailingList(Msg->GetToUser()))
						{
						SendMailToList(Msg);
						Unlock();
						return (IS_MAILINGLIST);
						}

					// handled forwarded E-mail. Check existence of forwardee if forwarded
					if (*Msg->GetToUser()
							&& FindPersonByName(*Msg->GetForward() ? Msg->GetForward() : Msg->GetToUser(), NULL) == CERROR
							&& !SameString(*Msg->GetForward() ? Msg->GetForward() : Msg->GetToUser(), getmsg(386))
							&& !SameString(*Msg->GetForward() ? Msg->GetForward() : Msg->GetToUser(), getmsg(385)))
						{
						// Generate a "User not found" error

						Message *ErrorMsg = new Message;

						if (ErrorMsg)
							{
							label Buffer;

							ErrorMsg->SetAuthor(programName);
							ErrorMsg->SetToUser(Msg->GetAuthor());
							ErrorMsg->SetToNodeName(Msg->GetOriginNodeName());

							ErrorMsg->SetToRegion(Msg->GetOriginRegion());

							ErrorMsg->SetRoomNumber(Msg->GetRoomNumber());
							ErrorMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));

							ErrorMsg->SetTextWithFormat(getmsg(509), bn, Msg->GetToUser(), cfg.nodeTitle);

							save_mail(ErrorMsg, FALSE, NULL, SkipLog);

							// and unsubscribe from mailing list if appropriate
							long Idx, NumCmt = Msg->GetCommentCount();
							const char *Cmt;

							for (Idx = 1; Idx <= NumCmt; Idx++)
								{
								Cmt = Msg->GetComment(Idx);

                                if (strncmpi(Cmt, getmsg(165), strlen(getmsg(165))) == SAMESTRING)
									{
									break;
									}
								}

							if (Idx <= NumCmt)
								{
								char listname[LABELSIZE+LABELSIZE];
                                strcpy(listname, Cmt + strlen(getmsg(165)));
								char *ptr = strrchr(listname, '@');

								if (ptr)
									{
									ptr[-1] = 0;	// strip " @ nodename"
									}

								ErrorMsg->SetAuthor(Msg->GetAuthor());	//forged
								ErrorMsg->SetToUser(listname);
                                ErrorMsg->SetSubject(getmsg(134));
								// Leave error message
								save_mail(ErrorMsg, FALSE, NULL, SkipLog);
								}

							delete ErrorMsg;
							}
						else
							{
							cOutOfMemory(98);
							}

						Unlock();
						return (IS_USERNOTFOUND);
						}
					}
#ifndef WINCIT
				}
			else
				{
				Unlock();
				return (IS_SCRIPT);
				}
#endif
			}
		}

	// fix group only messages
	if (*Msg->GetGroup())
		{
		const char *ThereGroup = Node->ThereToHereGroupName(Msg->GetGroup());

		if (ThereGroup)
			{
			Msg->SetGroup(ThereGroup);
			}
		else 
			{
			// put it in #MAPUNKGROUP if no mapping set
			char ReportString[128];

            sprintf(ReportString, getmsg(113), cfg.nodeTitle, Msg->GetGroup());
			Msg->AddComment(ReportString);
			Msg->SetGroup(Node->GetMapUnknownGroup());
			}
		}

	// it's good; save it

	// put roomname here (here was a variable passed in)
	// new development: always put roomname. We decided not to worry about preserving the room names.
	// too ugly and too much of a hassle
	Msg->SetCreationRoom(Room);

	r_slot temproom = Room;
	if (*Msg->GetToUser())
		{
		temproom = FindRoomForNetMail(Msg->GetCreationRoom());
		}

	Msg->SetRoomNumber(temproom);

	// this is some shit added 7-9-92 to forward messages to 'sysop' to #sysop if #forward
	if (SameString(Msg->GetToUser(), getmsg(386)) && cfg.forward && (FindPersonByName(cfg.sysop, NULL) != CERROR))
		{
		Msg->SetToUser(cfg.sysop);
		}

	putAndNoteMessage(Msg, TRUE);

	Unlock();
	return (IS_OK);
	}

Bool TERMWINDOWMEMBER ReadMsgFl(r_slot room, const char *filename, FILE *file, int *newMsgs, int *expired, int *duplicate,
		int *error, const LogEntry *FromNode)
	{
	Message *NewMsg = new Message;

	if (NewMsg)
		{
		*newMsgs = 0;
		*expired = 0;
		*duplicate = 0;

		KBReady();

		if (file == NULL)
			{
			char FileName[80];

			sprintf(FileName, sbs, LocalTempPath, filename);

			if ((file = fopen(FileName, FO_RB)) == NULL)
				{
				delete NewMsg;
				return (FALSE);
				}
			}

		NetMessagePacketC NetMsgPacket(file, node);

		while (NetMsgPacket.LoadAll(NewMsg) == RMS_OK)
			{
#ifndef WINCIT
            switch (MessageDat.IncorporateMessage(NewMsg, room, FromNode, ThisLog, node))
#else
            switch (MessageDat.IncorporateMessage(NewMsg, room, FromNode, LocalTempPath, ThisLog, node))
#endif
				{
				case IS_NOSAVENETMAIL:
				case IS_DUPLICATE:  	(*duplicate)++;	break;

				case IS_SAVENETMAIL:
				case IS_EXPIRED:    	(*expired)++;	break;

				case IS_OK:
				case IS_MAILINGLIST:	(*newMsgs)++;	break;


				default:				assert(FALSE); // no break; non-debugging versions just up error
				case IS_NOAUTHOR:
				case IS_NOACCESS:
				case IS_SCRIPT:
				case IS_OUTOFMEMORY:
				case IS_NETCOMMAND:
				case IS_USERNOTFOUND:	(*error)++;		break;
				}

			// I am not sure about this
			// Set newpointers here because if I don't then the new messages
			// Will be sent back next time there's a network
			// logBuf.newpointer[mb.mbroomno] = cfg.NewestMsg;
			if (filename)
				{
				CurrentUser->SetRoomNewPointer(NewMsg->GetRoomNumber(), MessageDat.NewestMessage());
				}

			// if !filename (that is, net69) then check for double-nul signaling end of netid's messages
			if (!filename)
				{
				long position = ftell(file);

				if (fgetc(file) == 0)
					{
					break;
					}
				else
					{
					fseek(file, position, SEEK_SET);
					}
				}
			}

		if (filename)
			{
			fclose(file);
			}

		delete NewMsg;

		return (TRUE);
		}
	else
		{
		cOutOfMemory(98);
		return (FALSE);
		}
	}
