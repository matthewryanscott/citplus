// --------------------------------------------------------------------------
// Citadel: NetMail.CPP
//
// Network-mail related code.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "log.h"
#ifndef WINCIT
#include "net.h"
#endif
#include "cfgfiles.h"
#include "events.h"
#include "miscovl.h"
#include "extmsg.h"
#ifdef WINCIT
#include "net6969.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
// asknode()			performs fill in the blanks by reading ROUTE.CIT
// BeenThere()			Checks to see if nodename or alias.locID is in
// buildaddress()		builds ROUTE.CIT #ADDRESS lines from the msg
// isaddress()			returns true if it looks like an address
// makeaddress()		return full address after joining two strings
// FindRoomForNetMail() find the room or net id for mail
// parse_address()		splits a string into alias & locID
// parseNetAddress()	Parse a net address from user input.
// parse_recipient()	break string up into pieces we can use
// save_mail()			save a message bound for another system
// search_route_cit()	returns the route or alias specified

static Bool search_route_cit(char *str, const char *srch);

// --------------------------------------------------------------------------
// parse_recipient(): Break string up into pieces we can use.

void parse_recipient(const char *str, char *user, char *node, char *region)
	{
	int posAt; 

	*user = 0;
	*node = 0;
	*region = 0;

	posAt = strpos('@', str);

	if (posAt)
		{
		posAt--;

		strncpy(user, str, min(posAt, LABELSIZE));
		user[min(posAt, LABELSIZE)] = 0;

		posAt++;

		str += posAt;

		if (str)
			{
			int posComma;

			posComma = strpos(',', str);

			if (posComma)
				{
				posComma--;

				strncpy(node, str, min(posComma, LABELSIZE));
				node[min(posComma, LABELSIZE)] = 0;

				posComma++;

				str += posComma;

				strcpy(region, str);
				}
			else
				{
				CopyString2Buffer(node, str, LABELSIZE);
				}
			}
		}
	else
		{
		CopyString2Buffer(user, str, LABELSIZE);
		}

	normalizeString(user);
	normalizeString(node);
	normalizeString(region);
	}


// --------------------------------------------------------------------------
// asknode(): This routine performs fill in the blanks by reading ROUTE.CIT
//	and asking the user questions.
//
// Return value:
//	TRUE if match found
//	FALSE if no match found

Bool TERMWINDOWMEMBER asknode(char *rnode, char *rregion, char *raddress)
	{
	FILE *fBuf;
	char line[95];
	char *words[256];
	char path[80];
	label previous_raddress;
	label previous_rnode;
	label previous_rregion;
	int count;
	char raddress_alias[4];
	char raddress_locID[5];
	char routecit_alias[4]; // [xxx].xxxx
	char routecit_locID[5]; // xxx.[xxxx]
	char query[100];

	*raddress_alias = 0;
	*routecit_alias = 0;
	*previous_rnode = 0;
	*raddress_locID = 0;
	*previous_raddress = 0;
	*routecit_locID = 0;
	*previous_rregion = 0;

	normalizeString(raddress);
	if (*raddress)
		{
		parse_address(raddress, raddress_alias, raddress_locID, TRUE);
		}

	sprintf(path, sbs, cfg.homepath, citfiles[C_ROUTE_CIT]);

	if ((fBuf = fopen(path, FO_R)) == NULL) // ASCII mode
		{
		mPrintfCR(getmsg(15), citfiles[C_ROUTE_CIT]);
		return (FALSE);
		}

	while (fgets(line, 90, fBuf) != NULL)
		{
		if (strnicmp(line, getmsg(624), 8) != SAMESTRING)
			{
			continue;
			}

		count = parse_it(words, line);

		if (count < 4)	// too few parameters
			{
			mPrintfCR(getmsg(516));
			}

		if (!isaddress(words[1])) // naughty alias
			{
			mPrintf(getmsg(515));
			mPrintfCR(getmsg(310), words[1]);
			}

		// words[1] == xxx.xxxx
		// words[2] == nodename
		// words[3] == region

		// We must match both.

		// This code is less than optimal but it works and I can read it.
		if ( (*rnode && *rregion) || (*raddress_alias && *raddress_locID))
			{
			if (*rnode) // Matching nodes and regions
				{
				if (SameString(rnode, words[2]) && SameString(rregion, words[3]))
					{
					// We found a match. Copy stuff and get out.
					CopyString2Buffer(rnode, words[2], LABELSIZE);
					CopyString2Buffer(rregion, words[3], LABELSIZE);
					CopyString2Buffer(raddress, words[1], LABELSIZE);

					fclose(fBuf);
					return (TRUE);
					}
				}
			else if (*raddress) // Matching full alias
				{
				// if we want to we could do a parse_address
				// on "words[1]" and compare the small strings
				// this would allow us to use addresses such as
				// "tgi" in ROUTE.CIT

				if (SameString(raddress, words[1]))
					{
					// We found a match. Copy stuff and get out.
					CopyString2Buffer(rnode, words[2], LABELSIZE);
					CopyString2Buffer(rregion, words[3], LABELSIZE);
					CopyString2Buffer(raddress, words[1], LABELSIZE);

					fclose(fBuf);
					return (TRUE);
					}
				}
			}
		// We are trying for a partial match.
		else if (*rnode)
			{
			if (SameString(rnode, words[2]))
				{
				if (!*previous_rnode)
					{
					// Fill in temp var and keep Searching
					CopyStringToBuffer(previous_rnode, words[2]);
					CopyStringToBuffer(previous_rregion, words[3]);
					CopyStringToBuffer(previous_raddress, words[1]);
					}
				else
					{
					sprintf(query, getmsg(514), previous_rnode, previous_rregion, previous_raddress);

					if (getYesNo(query, 0))
						{
						// Fill in info and get out.
						CopyString2Buffer(rnode, previous_rnode, LABELSIZE);
						CopyString2Buffer(rregion, previous_rregion, LABELSIZE);
						CopyString2Buffer(raddress, previous_raddress, LABELSIZE);

						fclose(fBuf);
						return (TRUE);
						}
					else
						{
						// Fill in temp var and keep Searching
						CopyStringToBuffer(previous_rnode, words[2]);
						CopyStringToBuffer(previous_rregion, words[3]);
						CopyStringToBuffer(previous_raddress, words[1]);
						}
					}
				}
			}
		else if (*raddress_alias)
			{
			// words[1] == xxx.xxxx
			// words[2] == nodename
			// words[3] == region

			parse_address(words[1], routecit_alias, routecit_locID, TRUE);

			if (SameString(raddress_alias, routecit_alias))
				{
				if (!*previous_rnode)
					{
					// Fill in temp var and keep Searching
					CopyStringToBuffer(previous_rnode, words[2]);
					CopyStringToBuffer(previous_rregion, words[3]);
					CopyStringToBuffer(previous_raddress, words[1]);
					}
				else
					{
					sprintf(query, getmsg(514), previous_rnode, previous_rregion, previous_raddress);

					if (getYesNo(query, 0))
						{
						// Fill in info and get out.
						CopyString2Buffer(rnode, previous_rnode, LABELSIZE);
						CopyString2Buffer(rregion, previous_rregion, LABELSIZE);
						CopyString2Buffer(raddress, previous_raddress, LABELSIZE);

						fclose(fBuf);
						return (TRUE);
						}
					else
						{
						// Fill in temp var and keep Searching
						CopyStringToBuffer(previous_rnode, words[2]);
						CopyStringToBuffer(previous_rregion, words[3]);
						CopyStringToBuffer(previous_raddress, words[1]);
						}
					}
				}
			}
		}

	if (*previous_rnode)	// Default to previous partial match
		{
		// Fill in info and get out.
		// We could ask them if this is really what they wanted.
		CopyString2Buffer(rnode, previous_rnode, LABELSIZE);
		CopyString2Buffer(rregion, previous_rregion, LABELSIZE);
		CopyString2Buffer(raddress, previous_raddress, LABELSIZE);
		fclose(fBuf);
		return (TRUE);
		}

	fclose(fBuf);
	return (FALSE);
	}


// --------------------------------------------------------------------------
// BeenThere(): Checks to see if nodename or alias.region is in path. Will
//	optionally copy next thing in path to the 'next' variable.

Bool BeenThere(const char *path, const char *nodename, const char *alias, const char *locID, char *next)
	{
	int i;
	char current_locID[5];
	Bool foundit = FALSE;

	*current_locID = 0;

	if (next)
		{
		*next = 0;
		}

	// Look for the node's name in the path. Note that this will deliver a
	// false positive if the node we are looking for is a substring of a
	// node that is in the path.
	if (*nodename)
		{
		i = substr(path, nodename, FALSE);

		if (i != CERROR)
			{
			foundit = TRUE;
			i += strlen(nodename) + 1;
			}
		}

	if (!foundit)
		{
		// Look for the node's address in the path.

		for (i = 0; path[i] && !foundit;)
			{
			int k;

			for (k = i; path[i] && (path[i] != '!'); i++);

			if (i - k < 9)
				{
				char address[9];

				CopyStringToBuffer(address, path + k);

				if (isaddress(address))
					{
					char path_alias[4], path_locID[5];
					parse_address(address, path_alias, path_locID, TRUE);

					if (*path_locID)
						{
						CopyStringToBuffer(current_locID, path_locID);
						}

					if (SameString(alias, path_alias) && SameString(locID, current_locID))
						{
						foundit = TRUE;
						}
					}
				}

			if (path[i] == '!')
				{
				i++;
				}
			}
		}

	if (foundit && (next != NULL))
		{
		int j;

		for (j = 0; path[i] && (path[i] != '!') && (j < LABELSIZE); i++, j++)
			{
			next[j] = path[i];
			}

		next[j] = 0;
		}

#ifdef TOGGLE_FOR_VPP

// Note:	This is ONLY a guess as to how it should be done. Fix this before
//			using.

	// this part should be toggleable for people like vpp who want to net
	// anything irregardless of sending things back to their source locID

	// IRREGARDLESS is considered nonstandard because it is
	// ------------ redundant: once the negative idea is
	//				expressed by the -less ending, it is
	//				excessive to add the negative ir-
	//				prefix to express the same.

	if (*locID && SameString(locID, current_locID) && (strlen(locID) < 5))
		{
		sprintf(lookfor, getmsg(308), locID);

		if (IsSubstr(path, lookfor))
			{
			return (TRUE);
			}
		}
#else
	return (foundit);
#endif
	}


// --------------------------------------------------------------------------
// buildaddress(): builds ROUTE.CIT #ADDRESS lines from the msg base in the
//	format #ADDRESS "xxx.xxxx" "Nodename" "RegionName"

#define UPDATE_TIME 3

void TERMWINDOWMEMBER buildaddress(ModemConsoleE W)
	{
	Message *Msg = new Message;

	if (Msg)
		{
		// Used to keep track of addresses that have been found so far.
		struct addr
			{
			struct addr *next;
			char address[9];
			};

		FILE *fBuf;
		char line[95];
		char *words[256];
		char path[80];
		int count;
		struct addr *addressList = NULL;
		char found;
		char raddress[9];

		compactMemory(1);

		sprintf(path, sbs, cfg.homepath, citfiles[C_ROUTE_CIT]);

		if ((fBuf = fopen(path, FO_AP)) == NULL)	// ASCII mode
			{
			wPrintf(W, getmsg(15), citfiles[C_ROUTE_CIT]);
			wDoCR(W);
			delete Msg;
			return;
			}

		fseek(fBuf, 0l, SEEK_SET);

		while (fgets(line, 90, fBuf) != NULL)
			{
			if (strnicmp(line, getmsg(624), 8) != SAMESTRING)
				{
				continue;
				}

			count = parse_it(words, line);

			if (count < 4)	// too few parameters
				{
				wPrintf(W, getmsg(516));
				wDoCR(W);
				}

			if (!isaddress(words[1])) // naughty alias
				{
				wPrintf(W, getmsg(515));
				wPrintf(W, getmsg(310), words[1]);
				wDoCR(W);
				}

			// Copy address info into memory
			addr *thisAddress = thisAddress = (addr *) addLL((void **) &addressList, sizeof(*addressList));

			if (thisAddress == NULL)
				{
				cOutOfMemory(86);
				}
			else
				{
				// words[1] == xxx.xxxx
				// words[2] == nodename
				// words[3] == region
				CopyString2Buffer(thisAddress->address, words[1], sizeof(thisAddress->address));
				}
			}

		wDoCR(W);
		wPrintf(W, getmsg(387));

		if (!MessageDat.IsValid(cfg.NextBuildAddress))
			{
			cfg.NextBuildAddress = MessageDat.OldestMessageInTable();
			}

		m_index FirstToBuild = cfg.NextBuildAddress;
		m_index LastToBuild = MessageDat.NewestMessage();
		m_index TotalToBuild = LastToBuild - FirstToBuild + 1;

		if (!TotalToBuild)
			{
			TotalToBuild = 1;
			}

		time_t tm = time(NULL);
		int percent = 0, pctLength = 2;

		for (m_index ID = FirstToBuild; ID <= LastToBuild; ID++)
			{
			if (time(NULL) - tm >= UPDATE_TIME)
				{
				time(&tm);

				const int nowpercent = 99 - (int) ((99 * (LastToBuild - ID)) / TotalToBuild);

				if (nowpercent != percent)
					{
					percent = nowpercent;

					while (pctLength--)
						{
						outConRawBs();
						CommPort->Output('\b');
						}

					label pctStr;
					sprintf(pctStr, getmsg(47), percent);
					pctLength = strlen(pctStr);

					wPrintf(W, pctStr);
					}
				}

			if (MessageDat.IsNetworked(ID))
				{
				if (MessageDat.LoadHeader(ID, Msg) != RMS_OK)
					{
					continue;
					}

				label Buffer;
				CopyStringToBuffer(raddress, Msg->GetOriginAddress(Buffer));

				if (!isaddress(raddress))
					{
					continue;
					}

				found = FALSE;
				addr *thisAddress;
				for (thisAddress = addressList; thisAddress && !found; thisAddress = (addr *) getNextLL(thisAddress))
					{
					if (SameString(thisAddress->address, raddress))
						{
						found = TRUE;
						}
					}

				if (!found)
					{
					// Copy address info into memory
					thisAddress = (struct addr *) addLL((void **) &addressList, sizeof(*addressList));

					if (thisAddress == NULL)
						{
						cOutOfMemory(86);
						}
					else
						{
						CopyString2Buffer(thisAddress->address, raddress, sizeof(thisAddress->address));
						}

					char yuk[128], yuk2[128], yuk3[128];

					mkDblBck(raddress, line);
					sprintf(yuk, getmsg(306), line);

					mkDblBck(Msg->GetOriginNodeName(), line);
					sprintf(yuk2, getmsg(306), line);

					mkDblBck(Msg->GetOriginRegion(), line);
					sprintf(yuk3, getmsg(306), line);

					sprintf(line, getmsg(286), bn, getmsg(624), yuk, yuk2, yuk3);
					fwrite(line, sizeof(char), strlen(line), fBuf);

					if (cfg.NewNodeAlert)
						{
						label Name, Region, Phone, MsgNo;

						CopyString2Buffer(Name, Msg->GetOriginNodeName(), sizeof(Name));
						CopyString2Buffer(Region, Msg->GetOriginRegion(), sizeof(Region));
						CopyString2Buffer(Phone, Msg->GetOriginPhoneNumber(), sizeof(Phone));
						CopyString2Buffer(MsgNo, Msg->GetLocalID(), sizeof(MsgNo));

						Msg->ClearAll();
						Msg->SetRoomNumber(AIDEROOM);

						Msg->SetTextWithFormat(getmsg(443), MsgNo, Name, Region, raddress, Phone);

						systemMessage(Msg);
						}
					}
				}
			}

		while (pctLength--)
			{
			outConRawBs();
			CommPort->Output('\b');
			}

		wPrintf(W, getmsg(47), 100);
		wPrintf(W, getmsg(389));
		wDoCR(W);

		cfg.NextBuildAddress = LastToBuild + 1;

		disposeLL((void **) &addressList);
		fclose(fBuf);

		delete Msg;
		}
	else
		{
		cOutOfMemory(86);
		}
	}


// --------------------------------------------------------------------------
// isaddress(): Examines string and returns TRUE if it looks like an address.

Bool isaddress(const char *str)
	{
	int dotpos;
	Bool FoundDot = FALSE;

	// Don't be fooled by "St. Dism"
	if (strchr(str, ' ') != NULL)
		{
		return (FALSE);
		}

	const int len = strlen(str);

	for (int i = 0; i < len; i++)
		{
		if (str[i] == '.')
			{
			// we can only have one dot.
			if (FoundDot)
				{
				return (FALSE);
				}

			dotpos = i + 1;
			FoundDot = TRUE;
			}
		}

	if	(	// 1, 2, or 3 chars, no dots
			((len > 0) && (len < 4) && !FoundDot)

			||

			// 5, 6, 7, or 8 chars, there is a dot, and it is at char
			// pos 2, 3, or 4, and not too far from the end.
			(	(len > 4) && (len < 9) && FoundDot && (dotpos > 1) &&
				(dotpos < 5) && ((len - dotpos) < 5)
			)
		)
		{
		return (TRUE);
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// makeaddress(): Return full address after joining two strings

const char *makeaddress(const char *alias, const char *locID, char *AddressBuffer)
	{
	*AddressBuffer = 0;

	if (*alias || *locID)
		{
		CopyString2Buffer(AddressBuffer, alias, 4);

		strcat(AddressBuffer, getmsg(433));

		char temp[5];
		CopyStringToBuffer(temp, locID);
		strcat(AddressBuffer, temp);

		strlwr(AddressBuffer);
		}

	return (AddressBuffer);
	}

// --------------------------------------------------------------------------
// FindRoomForNetMail(): Find the room or net id for mail.

r_slot FindRoomForNetMail(const char *str)
	{
	r_slot i;

	if (cfg.netmail)
		{
		i = RoomExists(str);

		if (i == CERROR)
			{
			i = IdExists(str, 0);
			}

		if (i == CERROR)
			{
			i = MAILROOM;
			}
		}
	else
		{
		i = MAILROOM;
		}

	return (i);
	}


// --------------------------------------------------------------------------
// Message::GetOriginAddress(): Attempts to pull the origin address from msgbuf.

const char *Message::GetOriginAddress(char *Buffer) const
	{
	int i;

	for (i = 0; FromPath[i] && (FromPath[i] != '!') && (i < 8); i++)
		{
		Buffer[i] = FromPath[i];
		}

	Buffer[i] = 0;

	// If we filled up the address, make sure there is not more. If there is more, then the address is not an address.
	if (i == 9 && !(FromPath[i] == '!' || FromPath[i] == 0))
		{
		*Buffer = 0;
		}

	if (!isaddress(Buffer))
		{
		*Buffer = 0;
		}

	return (Buffer);
	}


// --------------------------------------------------------------------------
// parse_address(): Splits a string into alias & locID.

void parse_address(const char *str, char *alias, char *locID, Bool option)
	{
	int periods[2];
	int i;
	int j;
	int len;
	int numperiods = 0;

	alias[0] = 0;
	locID[0] = 0;

	len = strlen(str);

	for (i = len - 1; i >= 0 && (numperiods < 2) ; i--)
		{
		if (str[i] == '.')
			{
			periods[numperiods] = i;
			numperiods++;
			}
		}

	// No periods, Copy alias from beginning, default to SEA or cfg.locID
	if (!numperiods)
		{
		for (i = 0; i < 3 && str[i] && !ispunct(str[i]) && !isspace(str[i]); i++)
			{
			alias[i] = str[i];
			}
		alias[i] = 0;

		if (!option && alias[0])
			{
			strcpy(locID, *cfg.locID ? cfg.locID : getmsg(256));
			}
		}
	else
		{
		// Copy locID from first dot from the end
		for (i = periods[0]+1, j = 0; ((i - periods[0]) < 5) && str[i] && !ispunct(str[i]) && !isspace(str[i]); i++, j++)
			{
			locID[j] = str[i];
			}
		locID[j] = 0;


		if (numperiods > 1)
			{
			// Copy alias from second dot from the end
			for (i = periods[1]+1, j = 0; ((i - periods[1]) < 4) && str[i] && !ispunct(str[i]) && !isspace(str[i]); i++, j++)
				{
				alias[j] = str[i];
				}
			alias[j] = 0;
			}
		else
			{
			// Copy alias from beginning of string
			for (i = 0; i < 3 && str[i] && !ispunct(str[i]) && !isspace(str[i]); i++)
				{
				alias[i] = str[i];
				}
			alias[i] = 0;
			}
		}

	strlwr(alias);
	strlwr(locID);
	}


// --------------------------------------------------------------------------
// parseNetAddress(): Parse a net address from user input.

void TERMWINDOWMEMBER parseNetAddress(const char *str, char *ruser, char *rnode, char *rregion, char *raddress)
	{
	char path[80];

	// Blank 'em out just in case
	*rnode = 0;
	*ruser = 0;
	*raddress = 0;
	*rregion = 0;

	parse_recipient(str, ruser, rnode, rregion);

	if (!*ruser)
		{
		CopyString2Buffer(ruser, getmsg(386), LABELSIZE);
		}

	if (!*rnode)
		{
		if (*raddress)
			{
			mPrintfCR(getmsg(684), raddress);
			}

		return;
		}

	if (isaddress(rnode))
		{
		strcpy(raddress, rnode);
		*rnode = 0;
		}

	sprintf(path, sbs, cfg.homepath, citfiles[C_ROUTE_CIT]);
	if (!filexists(path)) // If there is no ROUTE.CIT
		{
		dispBlb(B_NOROUTE);
		return;
		}

	if (!asknode(rnode, rregion, raddress))
		{
		if (*rnode && nodexists(rnode) == CERROR)
			{
			mPrintf(getmsg(625));

			if (*raddress)
				{
				mPrintf(pcts, raddress);
				}
			else if (*rnode && *rregion)
				{
				mPrintf(getmsg(251), rnode, rregion);
				}
			else
				{
				mPrintf(pcts, rnode);
				}

			mPrintfCR(getmsg(250));
			}
		}
	}

static Bool IsSameLocID(const char *Addr1, const char *Addr2)
	{
	const char *DP1 = strchr(Addr1, '.');
	const char *DP2 = strchr(Addr2, '.');

	if (DP1 && DP2)
		{
		return (SameString(DP1, DP2));
		}
	else
		{
		return (FALSE);
		}
	}


// --------------------------------------------------------------------------
// save_mail(): Save a message bound for another system.
//
//	Msg: Message to send
//	Stub: TRUE to keep a stub on our system
//	StubMsg: If Stub, what the stub is to be
//	SkipLog: Do not route through this node. -1 (invalid log entry) to send to all nodes.
//
// Return value:
//	TRUE if was able to route somewhere
//	FASLE if was not able to route anywhere

Bool save_mail(Message *Msg, Bool Stub, const char *StubMsg, l_index SkipLog)
	{
	if (!*Msg->GetLocalID())
		{
		Msg->SetLocalID(MessageDat.NewestMessage() + 1);
		}

	DebugOut(15, *Msg->GetForward() ? Msg->GetForward() : Msg->GetToUser(), Msg->GetToNodeName());

	if (*Msg->GetToPath())
		{
		DebugOut(16, Msg->GetToPath());
		}

	if (*Msg->GetFromPath())
		{
		DebugOut(17, Msg->GetFromPath());
		}

	Message *SaveMessage = new Message;

	if (!SaveMessage)
		{
		OutOfMemory(87);
		return (FALSE);
		}

	label tosystem, oldtosystem;

	if (Stub)
		{
		MessageDat.StoreAsStub(Msg, StubMsg, cfg.poopdebug);
		}

	// ------------------------- Determine route ------------------------

	// Use Forced Path
	if (*Msg->GetToPath())
		{
		DebugOut(18);

		// Find next node in path
		if (!(BeenThere(Msg->GetToPath(), cfg.nodeTitle, cfg.alias, cfg.locID, tosystem) && *tosystem))
			{
			DebugOut(19);
			CopyStringToBuffer(tosystem, Msg->GetToNodeName());
			}
		}
	else
		{
		// to where are we sending it
		CopyStringToBuffer(tosystem, Msg->GetToNodeName());
		}

	CopyStringToBuffer(oldtosystem, tosystem);

	Bool FoundRoute = search_route_cit(tosystem, getmsg(623));

	DebugOut(20, tosystem);

	const l_slot nodenum = nodexists(tosystem, NULL);
	if (nodenum != CERROR)
		{
		// It's a direct connect--do it directly!
		FILE *fl;
		char filename[100];

		label Buffer;
		sprintf(filename, getmsg(444), cfg.transpath, LogTab[nodenum].GetAlias(Buffer, sizeof(Buffer)));

		if ((fl = fopen(filename, FO_AB)) == NULL)
			{
			DebugOut(21, filename);
			delete SaveMessage;
			return (FALSE);
			}

		*SaveMessage = *Msg;

#ifndef WINCIT
		PrepareMsgScriptInit(Msg);
		LogTab[nodenum].GetName(ContextSensitiveScriptText, sizeof(ContextSensitiveScriptText));
		doEvent(EVT_WRITENETMAIL, InitMsgScript);

		if (CheckMsgScript(Msg))
#endif
			{
			NetMessagePacketC MailFile(fl, NULL);
			MailFile.Store(Msg);
			}

		*Msg = *SaveMessage;

		fclose(fl);
		}
	else
		{
		// not a direct connect -- must route
		DebugOut(22);

		if (FoundRoute)
			{
			DebugOut(23);

			// We have a #ROUTE to a place we don't net with.
			SaveMessage->SetTextWithFormat(getmsg(622), oldtosystem, tosystem);
			SaveMessage->SetRoomNumber(AIDEROOM);

			systemMessage(SaveMessage);

			SaveMessage->ClearAll();
			}

		//	spasm!!!! goes through userlog, send it to each node found, skipping any that are
		//	already in the path or specified by SkipLog.

		// make sure all outgoing mail has the same time... needed for duplicate checking.
		if (!*Msg->GetCreationTime())
			{
			label Buffer;
			Msg->SetCreationTime(ltoa(time(NULL), Buffer, 10));
			}

		l_slot LogSlot;
		Bool done;

		// New development (94Apr18) - look at #ADDRESSes and locIDs
		label ToLocID;
		*ToLocID = 0;

		if (*Msg->GetDestinationAddress() || isaddress(Msg->GetToNodeName()))
			{
			label ToAddress;

			if (*Msg->GetDestinationAddress())
				{
				CopyStringToBuffer(ToAddress, Msg->GetDestinationAddress());
				}
			else
				{
				CopyStringToBuffer(ToAddress, Msg->GetToNodeName());
				}

			if (IsSameLocID(cfg.Address, ToAddress))
				{
				// Where we're sending it to is in our local region
				CopyStringToBuffer(ToLocID, cfg.locID);
				DebugOut(24);
				}
			else
				{
				DebugOut(25);

				// It's not to our local region; check to see if we have a direct connect to the remote region
				for (LogSlot = 0; LogSlot < cfg.MAXLOGTAB; LogSlot++)
					{
					LogEntry1 Log1;

					if (LogTab[LogSlot].GetLogIndex() != SkipLog && LogTab[LogSlot].IsNode() &&
							Log1.Load(LogTab[LogSlot].GetLogIndex()))
						{
						if (!(cfg.expire && Log1.GetCallTime() < (time(NULL) - ((long) cfg.expire * SECSINDAY))))
							{
							label NodesAddress, Buffer, Buffer1;
							sprintf(NodesAddress, getmsg(416), Log1.GetAlias(Buffer, sizeof(Buffer)),
									Log1.GetLocID(Buffer1, sizeof(Buffer1)));

							if (IsSameLocID(NodesAddress, ToAddress))
								{
								Log1.GetLocID(ToLocID, sizeof(ToLocID));
								DebugOut(26, ToLocID);
								break;
								}
							}
						}
					}
				}
			}

		// ToLocID is either an empty string (we don't know where we're routing it) or the only LocID that we're
		// to route through.

		*SaveMessage = *Msg;

		for (LogSlot = 0, done = FALSE; LogSlot < cfg.MAXLOGTAB && !done; LogSlot++)
			{
			if (LogTab[LogSlot].GetLogIndex() != SkipLog && LogTab[LogSlot].IsInuse() && LogTab[LogSlot].IsNode())
				{
				LogEntry1 Log1;

				label Buffer, Buffer1, Buffer2;
				if (Log1.Load(LogTab[LogSlot].GetLogIndex()))
					{
					DebugOut(27, Log1.GetName(Buffer, sizeof(Buffer)));

					// Expired
					if (cfg.expire && Log1.GetCallTime() < (time(NULL) - ((long) cfg.expire * SECSINDAY)))
						{
						DebugOut(28);
						}
					else
						{
						// If we're routing only to one LocID, make sure this node is in that LocID
						if (*ToLocID && !SameString(ToLocID, Log1.GetLocID(Buffer, sizeof(Buffer))))
							{
							DebugOut(29);
							continue;
							}

						label RemoteNode;
						Log1.GetName(RemoteNode, sizeof(RemoteNode));

						// If the message has not been to the remote node, and we aren't set to #NOSPASM to it
						if (!BeenThere(Msg->GetFromPath(), RemoteNode, Log1.GetAlias(Buffer1, sizeof(Buffer1)),
								Log1.GetLocID(Buffer2, sizeof(Buffer2)), NULL) && !search_route_cit(RemoteNode, getmsg(9)))
							{
							FILE *fl;
							char filename[100];

#ifndef WINCIT
							PrepareMsgScriptInit(Msg);
							Log1.GetName(ContextSensitiveScriptText, sizeof(ContextSensitiveScriptText));
							doEvent(EVT_WRITENETMAIL, InitMsgScript);

							if (CheckMsgScript(Msg))
#endif
								{
								sprintf(filename, getmsg(444), cfg.transpath, Log1.GetAlias(Buffer, sizeof(Buffer)));

								if ((fl = fopen(filename, FO_AB)) != NULL)
									{
									DebugOut(30, filename);

									NetMessagePacketC MailFile(fl, NULL);
									MailFile.Store(Msg);
									fclose(fl);
									}
								else
									{
									DebugOut(21, filename);
									}
								}

							*Msg = *SaveMessage;
							}
						else if (debug)
							{
							if (BeenThere(Msg->GetFromPath(), RemoteNode, Log1.GetAlias(Buffer1, sizeof(Buffer1)),
									Log1.GetLocID(Buffer2, sizeof(Buffer2)), NULL))
								{
								DebugOut(31);
								}
							else
								{
								DebugOut(95);
								}
							}
						}
					}
				}
			}
		}

	delete SaveMessage;
	return (TRUE);
	}


// --------------------------------------------------------------------------
// search_route_cit(): Returns the route or alias specified.

static Bool search_route_cit(char *str, const char *srch)
	{
	FILE *fBuf;
	char line[95];
	char *words[256];
	char path[80];

	sprintf(path, sbs, cfg.homepath, citfiles[C_ROUTE_CIT]);

	if ((fBuf = fopen(path, FO_R)) == NULL) // ASCII mode
		{
#ifndef WINCIT
		mPrintfCR(getmsg(15), citfiles[C_ROUTE_CIT]);
#endif
		return (FALSE);
		}

	const int len = strlen(srch);

	while (fgets(line, 90, fBuf) != NULL)
		{
		if (line[0] != '#')
			{
			continue;
			}

		if (strnicmp(line, srch, len) != SAMESTRING)
			{
			continue;
			}

		const int count = parse_it(words, line);

		if (count > 1 && SameString(srch, words[0]))
			{
			if (SameString(str, words[1]))
				{
				fclose(fBuf);

				if (count > 2)
					{
					CopyString2Buffer(str, words[2], LABELSIZE);
					}

				return (TRUE);
				}
			}
		}

	fclose(fBuf);
	return (FALSE);
	}
