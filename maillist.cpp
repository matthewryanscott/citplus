// --------------------------------------------------------------------------
// Citadel: MailList.CPP
//
// Mailing list stuff, even.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "maillist.h"
#include "net.h"
#include "room.h"
#include "cfgfiles.h"
#include "miscovl.h"
#include "extmsg.h"


class MailingList
	{
	strList *Members;
	strList *Current;

	Bool MaySubscribe;
	Bool MayTell;
	Bool MayList;
	char InfoLine[80];
	char InfoFile[80];

public:
	MailingList(void)
		{
		assert(this);

		memset(this, 0, sizeof(*this));
		MayTell = TRUE;
		MayList = TRUE;
		}

	~MailingList()
		{
		assert(this);

		disposeLL((void **) &Members);
		}

	const char *GetFirstUser(void)
		{
		assert(this);

		Current = Members;

		if (Current)
			{
			return (Current->string);
			}
		else
			{
			return (ns);
			}
		}

	const char *GetNextUser(void)
		{
		if (Current)
			{
			Current = Current->next;
			}

		if (Current)
			{
			return (Current->string);
			}
		else
			{
			return (ns);
			}
		}

	Bool AddUser(const char *Name);
	Bool RemoveUser(const char *Name);

	Bool Load(const char *ListName);
	Bool Save(const char *ListName);

	Bool IsMayTell(void) const
		{
		assert(this);
		return (MayTell);
		}

	Bool IsMayList(void) const
		{
		assert(this);
		return (MayList);
		}

	Bool IsMaySubscribe(void) const
		{
		assert(this);
		return (MaySubscribe);
		}

	const char *GetInfoLine(void) const
		{
		assert(this);
		return (InfoLine);
		}

	const char *GetInfoFile(void) const
		{
		assert(this);
		return (InfoFile);
		}
	};

Bool MailingList::AddUser(const char *Name)
	{
	assert(this);
	assert(Name);

	if (*Name)
		{
		strList *theStrLL;

		for (theStrLL = Members; theStrLL; theStrLL = (strList *) getNextLL(theStrLL))
			{
			if (SameString(theStrLL->string, Name))
				{
				return (FALSE);
				}
			}

		theStrLL = (strList *) addLL((void **) &Members, sizeof(*theStrLL) + strlen(Name));

		if (theStrLL)
			{
			strcpy(theStrLL->string, Name);
			return (TRUE);
			}
		}

	return (FALSE);
	}

Bool MailingList::RemoveUser(const char *Name)
	{
	assert(this);
	assert(Name);

	if (*Name)
		{
		int i;
		strList *theStr;

		for (i = 1, theStr = Members; theStr; theStr = (strList *) getNextLL(theStr), i++)
			{
			if (SameString(theStr->string, Name))
				{
				deleteLLNode((void **) &Members, i);
				return (TRUE);
				}
			}
		}

	return (FALSE);
	}

Bool MailingList::Load(const char *ListName)
	{
	FILE *fBuf;
	char path[80];

	sprintf(path, sbs, cfg.homepath, citfiles[C_MAILLIST_CIT]);

	if ((fBuf = fopen(path, FO_R)) != NULL)
		{
		char line[90];

		while (fgets(line, 90, fBuf) != NULL)
			{
			if (line[0] != '#')
				{
				continue;
				}

			char *words[256];
			int count;
			count = parse_it(words, line);

			if (SameString(words[0], getmsg(463)))
				{
				if (SameString(ListName, words[1]))
					{
					while (fgets(line, 90, fBuf) != NULL)
						{
						if (line[0] != '#')
							{
							continue;
							}

						parse_it(words, line);

						if (SameString(words[0], getmsg(MSG_MAILLIST, 1)))
							{
							if (count > 1)
								{
								AddUser(words[1]);
								}
							}
						else if (SameString(words[0], getmsg(MSG_MAILLIST, 2)))
							{
							MaySubscribe = atoi(words[1]);
							}
						else if (SameString(words[0], getmsg(MSG_MAILLIST, 3)))
							{
							MayTell = atoi(words[1]);
							}
						else if (SameString(words[0], getmsg(MSG_MAILLIST, 4)))
							{
							MayList = atoi(words[1]);
							}
						else if (SameString(words[0], getmsg(MSG_MAILLIST, 5)))
							{
							CopyStringToBuffer(InfoLine, words[1]);
							}
						else if (SameString(words[0], getmsg(MSG_MAILLIST, 6)))
							{
							CopyStringToBuffer(InfoFile, words[1]);
							}
						else if (SameString(words[0], getmsg(463)))
							{
							break;
							}
						}

					fclose(fBuf);
					return (TRUE);
					}
				}
			}

		fclose(fBuf);
		}

	return (FALSE);
	}

Bool MailingList::Save(const char *ListName)
	{
	Bool Good = TRUE;
	FILE *Original;
	char Opath[80];
	FILE *New;
	char Npath[80];

	sprintf(Opath, sbs, cfg.homepath, citfiles[C_MAILLIST_CIT]);
	strcpy(Npath, Opath);
	strcpy(strchr(Npath, '.'), getmsg(MSG_MAILLIST, 7));

	if ((New = fopen(Npath, FO_W)) != NULL)
		{
		if ((Original = fopen(Opath, FO_R)) != NULL)
			{
			char line[90];
			char saveline[90];
			Bool Saving = TRUE;

			while (fgets(line, 90, Original) != NULL)
				{
				strcpy(saveline, line);

				if (line[0] == '#')
					{
					char *words[256];

					parse_it(words, line);

					if (SameString(words[0], getmsg(463)))
						{
						if (SameString(words[1], ListName))
							{
							Saving = FALSE;
							}
						else
							{
							Saving = TRUE;
							}
						}
					}

				if (Saving)
					{
					fwrite(saveline, strlen(saveline), sizeof(char), New);
					}
				}

			fclose(Original);
			}

		fprintf(New, bn);

		char TmpStr[256];
		fprintf(New, getmsg(MSG_MAILLIST, 8), getmsg(463), mkDblBck(ListName, TmpStr), bn);

		strList *sl;
		for (sl = Members; sl ; sl = (strList *) getNextLL(sl))
			{
			fprintf(New, getmsg(MSG_MAILLIST, 8), getmsg(MSG_MAILLIST, 1), mkDblBck(sl->string, TmpStr), bn);
			}

		fprintf(New, getmsg(MSG_MAILLIST, 9), getmsg(MSG_MAILLIST, 2), MaySubscribe, bn);
		fprintf(New, getmsg(MSG_MAILLIST, 9), getmsg(MSG_MAILLIST, 3), MayTell, bn);
		fprintf(New, getmsg(MSG_MAILLIST, 9), getmsg(MSG_MAILLIST, 4), MayList, bn);
		fprintf(New, getmsg(MSG_MAILLIST, 8), getmsg(MSG_MAILLIST, 5), mkDblBck(InfoLine, TmpStr), bn);
		fprintf(New, getmsg(MSG_MAILLIST, 8), getmsg(MSG_MAILLIST, 6), mkDblBck(InfoFile, TmpStr), bn);

		fclose(New);

		unlink(Opath);
		rename(Npath, Opath);
		}
	else
		{
#ifndef WINCIT
		mPrintf(getmsg(78), Npath);
#endif
		Good = FALSE;
		}

	return (Good);
	}

Bool IsMailingList(const char *Name)
	{
	assert(Name);

	if (*Name)
		{
		FILE *fBuf;
		char path[80];

		sprintf(path, sbs, cfg.homepath, citfiles[C_MAILLIST_CIT]);

		if ((fBuf = fopen(path, FO_R)) != NULL)
			{
			char line[90];
			const int len = strlen(getmsg(463));

			while (fgets(line, 90, fBuf) != NULL)
				{
				if (line[0] != '#')
					{
					continue;
					}

				if (strnicmp(line, getmsg(463), len) != SAMESTRING)
					{
					continue;
					}

				char *words[256];
				parse_it(words, line);

				if (SameString(words[0], getmsg(463)))
					{
					if (SameString(Name, words[1]))
						{
						fclose(fBuf);
						return (TRUE);
						}
					}
				}

			fclose(fBuf);
			}
		}

	return (FALSE);
	}

void SendMailToList(Message *Msg)
	{
	if (!LockMessages(MSG_MAILLIST))
		{
		OutOfMemory(53);
		return;
		}

	MailingList List;

	if (!List.Load(Msg->GetToUser()))
		{
		UnlockMessages(MSG_MAILLIST);
		return;
		}

	if (SameString(Msg->GetSubject(), getmsg(MSG_MAILLIST, 10)))
		{
#ifndef WINCIT
		doccr();
		cPrintf(getmsg(MSG_MAILLIST, 11), getmsg(MSG_MAILLIST, 10));
#endif

		if (*Msg->GetOriginNodeName())
			{
			MessageDat.StoreAsStub(Msg, getmsg(MSG_MAILLIST, 12), cfg.poopdebug);
			}

		Message *NewMsg = new Message;

		if (NewMsg)
			{
			NewMsg->SetCreationRoom(Msg->GetCreationRoom());

			NewMsg->SetRoomNumber(Msg->GetRoomNumber());

			NewMsg->SetToUser(Msg->GetAuthor());
			NewMsg->SetAuthor(Msg->GetToUser());

			NewMsg->SetReplyToMessage(*Msg->GetSourceID() ? Msg->GetSourceID() : Msg->GetLocalID());

			if (*Msg->GetSourceID())
				{
				label Buffer;

				NewMsg->SetToNodeName(Msg->GetOriginNodeName());
				NewMsg->SetToRegion(Msg->GetOriginRegion());
				NewMsg->SetToCountry(Msg->GetOriginCountry());
				NewMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));
				}

			label Subject;
			sprintf(Subject, getmsg(MSG_MAILLIST, 13), getmsg(MSG_MAILLIST, 10));
			NewMsg->SetSubject(Subject);

			if (List.IsMayTell())
				{
				NewMsg->SetText(ns);

				if (*List.GetInfoLine())
					{
					NewMsg->AppendText(List.GetInfoLine());
					NewMsg->AppendText(bn);
					}

				if (*List.GetInfoFile())
					{
					char *TextFromFile = new char[MAXTEXT];

					if (TextFromFile)
						{
						FILE *fd;

						if ((fd = fopen(List.GetInfoFile(), FO_RB)) != NULL)
							{
							GetFileMessage(fd, TextFromFile, MAXTEXT - 1);
							fclose(fd);

							NewMsg->AppendText(TextFromFile);
							NewMsg->AppendText(bn);
							}
#ifndef WINCIT
						else
							{
							cPrintf(getmsg(78), List.GetInfoFile());
							doccr();
							}
#endif

						delete [] TextFromFile;
						}
					else
						{
						OutOfMemory(53);
						}
					}

				if (List.IsMayList())
					{
					if (*List.GetFirstUser())
						{
						NewMsg->AppendText(getmsg(MSG_MAILLIST, 14));
						NewMsg->AppendText(bn);

						char destination[4 * LABELSIZE + 7];

						for (	CopyStringToBuffer(destination, List.GetFirstUser());

								*destination;

								CopyStringToBuffer(destination, List.GetNextUser()))
							{
							NewMsg->AppendText(spc);
							NewMsg->AppendText(destination);
							NewMsg->AppendText(bn);
							}
						}
					else
						{
						NewMsg->AppendText(getmsg(MSG_MAILLIST, 15));
						NewMsg->AppendText(bn);
						}
					}
				else
					{
					NewMsg->AppendText(getmsg(MSG_MAILLIST, 16));
					NewMsg->AppendText(bn);
					}
				}
			else
				{
				NewMsg->SetText(getmsg(MSG_MAILLIST, 17));
				}

			if (*NewMsg->GetToNodeName())
				{
				if (!save_mail(NewMsg, TRUE, getmsg(MSG_MAILLIST, 12), CERROR))
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 18));
#endif
					}
				}
			else
				{
				putAndNoteMessage(NewMsg, TRUE);
				}

			delete NewMsg;
			}
		else
			{
			OutOfMemory(53);
			}
#ifndef WINCIT
		doccr();
#endif
		}
	else if (SameString(Msg->GetSubject(), getmsg(MSG_MAILLIST, 19)))
		{
#ifndef WINCIT
		doccr();
		cPrintf(getmsg(MSG_MAILLIST, 11), getmsg(MSG_MAILLIST, 19));
#endif

		if (*Msg->GetOriginNodeName())
			{
			MessageDat.StoreAsStub(Msg, getmsg(MSG_MAILLIST, 12), cfg.poopdebug);
			}

		Message *NewMsg = new Message;

		if (NewMsg)
			{
			NewMsg->SetCreationRoom(Msg->GetCreationRoom());

			NewMsg->SetRoomNumber(Msg->GetRoomNumber());

			NewMsg->SetToUser(Msg->GetAuthor());
			NewMsg->SetAuthor(Msg->GetToUser());

			NewMsg->SetReplyToMessage(*Msg->GetSourceID() ? Msg->GetSourceID() : Msg->GetLocalID());

			if (*Msg->GetSourceID())
				{
				label Buffer;

				NewMsg->SetToNodeName(Msg->GetOriginNodeName());
				NewMsg->SetToRegion(Msg->GetOriginRegion());
				NewMsg->SetToCountry(Msg->GetOriginCountry());
				NewMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));
				}

			label Subject;
			sprintf(Subject, getmsg(MSG_MAILLIST, 13), getmsg(MSG_MAILLIST, 19));
			NewMsg->SetSubject(Subject);

			if (List.IsMaySubscribe())
				{
				NewMsg->SetTextWithFormat(getmsg(MSG_MAILLIST, 20), Msg->GetToUser(), cfg.nodeTitle);

				if (*Msg->GetOriginNodeName())
					{
					char string[256];
					sprintf(string, getmsg(101), Msg->GetAuthor(), Msg->GetOriginNodeName());

					List.AddUser(string);
					}
				else
					{
					List.AddUser(Msg->GetAuthor());
					}

				List.Save(Msg->GetToUser());
				}
			else
				{
				NewMsg->SetTextWithFormat(getmsg(MSG_MAILLIST, 21), Msg->GetToUser(), cfg.nodeTitle);
				}

			if (*NewMsg->GetToNodeName())
				{
				if (!save_mail(NewMsg, TRUE, getmsg(MSG_MAILLIST, 12), CERROR))
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 18));
#endif
					}
				}
			else
				{
				putAndNoteMessage(NewMsg, TRUE);
				}

			delete NewMsg;
			}
		else
			{
			OutOfMemory(53);
			}

#ifndef WINCIT
		doccr();
#endif
		}
    else if (SameString(Msg->GetSubject(), getmsg(134)))
		{
#ifndef WINCIT
		doccr();
        cPrintf(getmsg(MSG_MAILLIST, 11), getmsg(134));
#endif

		if (*Msg->GetOriginNodeName())
			{
			MessageDat.StoreAsStub(Msg, getmsg(MSG_MAILLIST, 12), cfg.poopdebug);
			}

		Message *NewMsg = new Message;

		if (NewMsg)
			{
			NewMsg->SetCreationRoom(Msg->GetCreationRoom());

			NewMsg->SetRoomNumber(Msg->GetRoomNumber());

			NewMsg->SetToUser(Msg->GetAuthor());
			NewMsg->SetAuthor(Msg->GetToUser());

			NewMsg->SetReplyToMessage(*Msg->GetSourceID() ? Msg->GetSourceID() : Msg->GetLocalID());

			if (*Msg->GetSourceID())
				{
				label Buffer;

				NewMsg->SetToNodeName(Msg->GetOriginNodeName());
				NewMsg->SetToRegion(Msg->GetOriginRegion());
				NewMsg->SetToCountry(Msg->GetOriginCountry());
				NewMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));
				}

			label Subject;
            sprintf(Subject, getmsg(MSG_MAILLIST, 13), getmsg(134));
			NewMsg->SetSubject(Subject);

			char string[256];

			if (*Msg->GetOriginNodeName())
				{
				sprintf(string, getmsg(101), Msg->GetAuthor(), Msg->GetOriginNodeName());
				}
			else
				{
				strcpy(string, Msg->GetAuthor());
				}

			if (List.RemoveUser(string))
				{
				NewMsg->SetTextWithFormat(getmsg(MSG_MAILLIST, 23), Msg->GetToUser(), cfg.nodeTitle);

				List.Save(Msg->GetToUser());
				}
			else
				{
				NewMsg->SetTextWithFormat(getmsg(MSG_MAILLIST, 24), Msg->GetToUser(), cfg.nodeTitle);
				}

			if (*NewMsg->GetToNodeName())
				{
				if (!save_mail(NewMsg, TRUE, getmsg(MSG_MAILLIST, 12), CERROR))
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 18));
#endif
					}
				}
			else
				{
				putAndNoteMessage(NewMsg, TRUE);
				}

			delete NewMsg;
			}
		else
			{
			OutOfMemory(53);
			}

#ifndef WINCIT
		doccr();
#endif
		}
	else if (*Msg->GetSubject() == '#')
		{
#ifndef WINCIT
		doccr();
		cPrintf(getmsg(MSG_MAILLIST, 25), Msg->GetSubject());
#endif

		if (*Msg->GetOriginNodeName())
			{
			MessageDat.StoreAsStub(Msg, getmsg(MSG_MAILLIST, 12), cfg.poopdebug);
			}

		Message *NewMsg = new Message;

		if (NewMsg)
			{
			NewMsg->SetCreationRoom(Msg->GetCreationRoom());

			NewMsg->SetRoomNumber(Msg->GetRoomNumber());

			NewMsg->SetToUser(Msg->GetAuthor());
			NewMsg->SetAuthor(Msg->GetToUser());

			NewMsg->SetReplyToMessage(*Msg->GetSourceID() ? Msg->GetSourceID() : Msg->GetLocalID());

			if (*Msg->GetSourceID())
				{
				label Buffer;

				NewMsg->SetToNodeName(Msg->GetOriginNodeName());
				NewMsg->SetToRegion(Msg->GetOriginRegion());
				NewMsg->SetToCountry(Msg->GetOriginCountry());
				NewMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));
				}

			NewMsg->SetSubject(getmsg(MSG_MAILLIST, 26));

			NewMsg->SetTextWithFormat(getmsg(MSG_MAILLIST, 27), Msg->GetSubject());

			if (*NewMsg->GetToNodeName())
				{
				if (!save_mail(NewMsg, TRUE, getmsg(MSG_MAILLIST, 12), CERROR))
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 18));
#endif
					}
				}
			else
				{
				putAndNoteMessage(NewMsg, TRUE);
				}

			delete NewMsg;
			}
		else
			{
			OutOfMemory(53);
			}

#ifndef WINCIT
		doccr();
#endif
		}
	else
		{
		// Check for mail list loops
		const int NumComments = (int) Msg->GetCommentCount();
		for (int Idx = 1; Idx <= NumComments; Idx++)
			{
			const char *Cmt = Msg->GetComment(Idx);

            if (strncmpi(Cmt, getmsg(165), strlen(getmsg(165))) == SAMESTRING)
				{
				label ToUser, ToNode, ToRegion;
				parse_recipient(strchr(Cmt, ':') + 2, ToUser, ToNode, ToRegion);

				if (SameString(ToUser, Msg->GetToUser()) && SameString(ToNode, cfg.nodeTitle))
					{
					UnlockMessages(MSG_MAILLIST);
					return;
					}
				}
			}

		char String[128];
		sprintf(String, getmsg(MSG_MAILLIST, 29), Msg->GetToUser(), cfg.nodeTitle);

		Msg->AddComment(String);

		if (*List.GetFirstUser())
			{
			char destination[LABELSIZE + LABELSIZE + LABELSIZE + LABELSIZE + 7];
			Bool FirstSaved = FALSE;

			strList *removeList = NULL;
			label listName;
			strcpy(listName, Msg->GetToUser()); // For unsubscribing

			for (CopyStringToBuffer(destination, List.GetFirstUser()); *destination;
					CopyStringToBuffer(destination, List.GetNextUser()))
				{
				label ToUser, ToNode, ToRegion;
				parse_recipient(destination, ToUser, ToNode, ToRegion);

				if (!*ToUser)
					{
					strcpy(ToUser, getmsg(386));
					}

#ifndef WINCIT
				if (*ToNode)
					{
					cPrintf(getmsg(MSG_MAILLIST, 30), ToUser, ToNode);
					}
				else
					{
					cPrintf(getmsg(MSG_MAILLIST, 31), ToUser);
					}
				doccr();
#endif

				if (SameString(ToUser, Msg->GetAuthor()) && SameString(ToNode, Msg->GetOriginNodeName()))
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 32));
					doccr();
#endif
					}
				else
					{
					Msg->SetToUser(ToUser);
					Msg->SetToNodeName(ToNode);
					Msg->SetToRegion(ToRegion);

#ifdef WINCIT
					Bool Saveit = ResolveMessageAddressingIssues(Msg, NULL, CONSOLE);
#else
                    Bool Saveit = ResolveMessageAddressingIssues(Msg, CONSOLE);
#endif
					if (!Saveit)
						{
						strList *theStr = (strList *) addLL((void **) &removeList,
								sizeof(*theStr) + strlen(Msg->GetToUser()));

						if (theStr)
							{
							strcpy(theStr->string, Msg->GetToUser());
							}
						}

					if (Saveit)
						{
						// If this came in over the network, we have to give
						// each copy some sort of unique Source ID. Here we
						// go...
						if (*Msg->GetSourceID())
							{
							label Buffer;
							Msg->SetSourceID(ltoa(time(NULL) + MessageDat.NewestMessage(), Buffer, 10));
							}

						if (*Msg->GetToNodeName())	// save it for netting
							{
							// room names get included in networked mail
							Msg->SetCreationRoom();

							if (save_mail(Msg, FirstSaved, getmsg(MSG_MAILLIST, 12), CERROR))
								{
								FirstSaved = FALSE;
								}
							else
								{
#ifndef WINCIT
								cPrintf(getmsg(MSG_MAILLIST, 18));
#endif
								}
							}
						else
							{
							putAndNoteMessage(Msg, TRUE);
							}

						if (!*Msg->GetToNodeName() && IsMailingList(Msg->GetToUser()))
							{
							// send a copy because of the Comments stuff.
							Message *Msg2 = new Message;

							if (Msg2)
								{
								*Msg2 = *Msg;
								SendMailToList(Msg2);
								delete Msg2;
								}
							else
								{
								OutOfMemory(53);
								}
							}
						}
					}
				}

			if (removeList)
				{
				for (const strList *theStr = removeList; theStr; theStr = (const strList *) getNextLL(theStr))
					{
					List.RemoveUser(theStr->string);
#ifndef WINCIT
					cPrintf(getmsg(MSG_MAILLIST, 33), theStr->string, listName);
					doCR();
#endif
					}

				disposeLL((void **) &removeList);
				List.Save(listName);
				}
			}
		}

	UnlockMessages(MSG_MAILLIST);
	}
