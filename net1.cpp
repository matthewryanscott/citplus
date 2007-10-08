// --------------------------------------------------------------------------
// Citadel: Net1.CPP
//
// Net 1.0/1.1 protocol implementation.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "net.h"
#include "filecmd.h"
#include "cfgfiles.h"
#include "cwindows.h"
#include "filerdwr.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// Net1Slave()		Actual networking slave
// Net1Master() 	During network master code
// get_first_room() get the first room in the room list
// get_next_room()	gets the next room in the list
// Net1Cleanup()	Done with other system, save mail and messages


// --------------------------------------------------------------------------
// Net1Slave(): Actual networking slave
//
// Return value:
//	TRUE: Worked
//	FALSE: Didn't

Bool TERMWINDOWMEMBER Net1Slave(void)
	{
	char line[100];
	label troo;
	label fn;
	FILE *file;
	int i = 0;

	const protocols *theProt = GetProtocolByKey(node->GetProtocol(), TRUE);

	if (!theProt)
		{
		doccr();
		cPrintf(getnetmsg(177));
		doCR();
		return (FALSE);
		}

	cPrintf(getnetmsg(178));
	doccr();

	sprintf(line, sbs, cfg.transpath, node->GetMailFileName());

	// create empty mail file if there is none
	if ((file = fopen(line, FO_AB)) != NULL)
		{
		fclose(file);
		}

	wxsnd(LocalTempPath, line, theProt, 0);

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	cPrintf(getnetmsg(179));
	doccr();

	sprintf(line, sbs, LocalTempPath, roomreqTmp);
	unlink(line);

	wxrcv(LocalTempPath, roomreqTmp, theProt);
	changedir(LocalTempPath);

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	sprintf(line, sbs, LocalTempPath, roomreqTmp);
	if ((file = fopen(line, FO_RB)) == NULL)
		{
		perror(getnetmsg(1));
		return (FALSE);
		}

	doccr();
	cPrintf(getnetmsg(2));
	doccr();

	GetStr(file, troo, LABELSIZE);

	while (*troo && !feof(file))
		{
		KBReady();

		r_slot rm;

		if ((rm = RoomExists(troo)) != CERROR)
			{
			if (CurrentUser->CanAccessRoom(rm))
				{
				sprintf(fn, getnetmsg(126), i);
				cPrintf(getnetmsg(166), deansi(troo));
				if (!((i+1) % 3))
					{
					doccr();
					}

				NewRoom(rm, fn, NULL);
				}
			else
				{
				doccr();
				cPrintf(getnetmsg(111), deansi(troo));
				doccr();
				amPrintf(getnetmsg(4), troo, bn);
				netError = TRUE;
				}
			}
		else
			{
			doccr();
			cPrintf(getnetmsg(5), deansi(troo));
			doccr();
			amPrintf(getnetmsg(167), troo, bn);
			netError = TRUE;
			}

		i++;
		GetStr(file, troo, LABELSIZE);
		}

	doccr();
	fclose(file);
	unlink(line);

	cPrintf(getnetmsg(7), cfg.Lmsg_nym);
	doccr();

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	wxsnd(LocalTempPath, getnetmsg(162), theProt, 0);

	ambigUnlink(getnetmsg(162));

	return (TRUE);
	}


// --------------------------------------------------------------------------
// Net1Master(): Network master code
//
// Return value:
//	TRUE: Worked
//	FALSE: Didn't

Bool TERMWINDOWMEMBER Net1Master(void)
	{
	char line[100], line2[100];
	label here, there;
	FILE *file;
	int i, rms;
	time_t t;

	const protocols *theProt = GetProtocolByKey(node->GetProtocol(), TRUE);

	if (!theProt)
		{
		doccr();
		cPrintf(getnetmsg(8));
		doccr();
		return (FALSE);
		}

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	cPrintf(getnetmsg(9));
	doccr();

	wxrcv(LocalTempPath, getnetmsg(164), theProt);

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	sprintf(line, getnetmsg(168), LocalTempPath);
	if ((file = fopen(line, FO_WB)) == NULL)
		{
		perror(getnetmsg(1));
		return (FALSE);
		}

	for (i = get_first_room(here, there), rms = 0; i; i = get_next_room(here, there), rms++)
		{
		PutStr(file, there);
		}

	PutStr(file, ns);
	fclose(file);

	cPrintf(getnetmsg(10));
	doccr();

	wxsnd(LocalTempPath, getnetmsg(169), theProt, 0);
	unlink(line);

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	CommPort->FlushInput();


	cPrintf(getnetmsg(11));
	// wait for them to get thier shit together
	t = time(NULL);
	while (CommPort->HaveConnection() && !CommPort->IsInputReady())
		{
		KBReady();
		if (time(NULL) > (t + (node->GetFetchTimeout() * 60)))
			{
			CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(73));

			Hangup();

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
			}

		// instantaneous reaction time isn't critical here
		CitIsIdle();
		}
	doccr();

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	cPrintf(getnetmsg(12), cfg.Lmsg_nym);
	doccr();

	wxrcv(LocalTempPath, ns, theProt);

	for (i = 0; i < rms; i++)
		{
		sprintf(line, getnetmsg(126), i);
		sprintf(line2, getnetmsg(127), i);
		rename(line, line2);
		}

	return (TRUE);
	}


// --------------------------------------------------------------------------
// Net1Cleanup(): Done with other system; save mail and messages

void TERMWINDOWMEMBER Net1Cleanup(void)
	{
	int t, i;
	label fn, here, there;
	char line[100];

	CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

	Hangup();

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}

	if (cfg.offhook == 1)
		{
		offhook();
		}

	sprintf(line, sbs, cfg.transpath, node->GetMailFileName());
	unlink(line);

	OC.SetOutFlag(IMPERVIOUS);

	doccr();
	cPrintf(getnetmsg(22));
	doccr();
	cPrintf(getnetmsg(23));
	doccr();
	cPrintf(getnetmsg(24));
	doccr();

	int Rooms = 0, TotalNew = 0, TotalExpired = 0, TotalDuplicate = 0;
	int ThisNew, ThisExpired, ThisDuplicate, ThisError;

	for (t = get_first_room(here, there), i = 0; t; t = get_next_room(here, there), i++)
		{
		sprintf(fn, getnetmsg(127), i);

		KBReady();

		const r_slot rm = RoomExists(here);
		if (rm != CERROR && CurrentUser->CanAccessRoom(rm))
			{
			cPrintf(getnetmsg(166), deansi(here));

			if (ReadMsgFl(rm, fn, NULL, &ThisNew, &ThisExpired, &ThisDuplicate, &ThisError, CurrentUser))
				{
				if (ThisNew)
					{
					OC.ansiattr = cfg.cattr;
					}
				cPrintf(getnetmsg(133), ThisNew);
				OC.ansiattr = cfg.attr;
				cPrintf(getnetmsg(134));

				if (ThisExpired)
					{
					OC.ansiattr = cfg.cattr;
					}

				cPrintf(getnetmsg(133), ThisExpired);
				OC.ansiattr = cfg.attr;
				cPrintf(getnetmsg(134));

				if (ThisDuplicate)
					{
					OC.ansiattr = cfg.cattr;
					}
				cPrintf(getnetmsg(133), ThisDuplicate);
				OC.ansiattr = cfg.attr;

				TotalNew += ThisNew;
				TotalExpired += ThisExpired;
				TotalDuplicate += ThisDuplicate;
				Rooms++;
				}
			else
				{
				amPrintf(getnetmsg(28), there, bn);
				netError = TRUE;
				cPrintf(getnetmsg(29));
				}

			doccr();
			}
		else
			{
			cPrintf(getnetmsg(30), deansi(here));
			amPrintf(getnetmsg(31), here, bn);
			netError = TRUE;
			doccr();
			}

		unlink(fn);
		}

	cPrintf(getnetmsg(24));
	doccr();
	cPrintf(getnetmsg(32));

	if (Rooms)
		{
		OC.ansiattr = cfg.cattr;
		}
	cPrintf(getnetmsg(133), Rooms);
	OC.ansiattr = cfg.attr;

	cPrintf(getnetmsg(134));
	if (TotalNew)
		{
		OC.ansiattr = cfg.cattr;
		}
	cPrintf(getnetmsg(133), TotalNew);
	OC.ansiattr = cfg.attr;

	cPrintf(getnetmsg(134));
	if (TotalExpired)
		{
		OC.ansiattr = cfg.cattr;
		}
	cPrintf(getnetmsg(133), TotalExpired);
	OC.ansiattr = cfg.attr;

	cPrintf(getnetmsg(134));
	if (TotalDuplicate)
		{
		OC.ansiattr = cfg.cattr;
		}
	cPrintf(getnetmsg(133), dup);
	OC.ansiattr = cfg.attr;

	doccr();
	cPrintf(getnetmsg(33));
	doccr();
	doccr();
	cPrintf(getnetmsg(34));
	doccr();

	ReadMsgFl(MAILROOM, getnetmsg(164), NULL, &ThisNew, &ThisExpired, &ThisDuplicate, &ThisError, NULL);

	label New, Routed, Rejected;
	CopyStringToBuffer(New, ltoac(i == CERROR ? 0 : i));
	CopyStringToBuffer(Routed, ltoac(ThisExpired));
	CopyStringToBuffer(Rejected, ltoac(ThisDuplicate));

	cPrintf(getnetmsg(35), New, ThisNew == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym, Routed, Rejected,
			ltoac(ThisNew + ThisExpired + ThisDuplicate));

	doccr();

	changedir(LocalTempPath);
	ambigUnlink(getnetmsg(162));
	ambigUnlink(getnetmsg(163));

	unlink(getnetmsg(164));

	changedir(cfg.homepath);

	MS.Expired = TotalExpired;
	MS.Duplicate = TotalDuplicate;
	MS.Entered = TotalNew + ThisNew;

	if (netError)
		{
		label Buffer;
		amPrintf(getnetmsg(36), bn, CurrentUser->GetName(Buffer, sizeof(Buffer)), bn);
		SaveAideMess(NULL);
		}

	readNetCmdTmp(CONSOLE);

	// we are no longer calling out...
	callout = FALSE;
	}


// --------------------------------------------------------------------------
// the folowing two routines are used for scaning through the rooms listed
// in NODES.CIT for the other node.

// --------------------------------------------------------------------------
// get_first_room(): Get the first room in the room list, prepare for reading
//	the rest.
//
// Output:
//	char *here: Filled with the room name here; must be >= LABELSIZE+1
//	char *there: Filled with the room name there; must be >= LABELSIZE+1
//
// Return value:
//	TRUE: Rooms found
//	FALSE: nothing found
//
// Notes:
//	If this returns TRUE, you must keep calling get_next_room until it
//	returns FALSE. If this or get_next_room returns FALSE, get_next_room
//	must not be called again until this is called and it returns TRUE.

Bool TERMWINDOWMEMBER get_first_room(char *here, char *there)
	{
	assert(!nodefile);

	char path[80];
	if (!node->GetRoomOffset())
		{
		return (FALSE);
		}

	sprintf(path, sbs, cfg.homepath, citfiles[C_NODES_CIT]);

	if ((nodefile = fopen(path, FO_R)) == NULL) // ASCII mode
		{
		cPrintf(getmsg(15), citfiles[C_NODES_CIT]);
		doccr();
		return (FALSE);
		}

	fseek(nodefile, node->GetRoomOffset(), SEEK_SET);

	return (get_next_room(here, there));
	}


// --------------------------------------------------------------------------
// get_next_room(): Gets the next room in the list
//
// Output:
//	char *here: Filled with the room name here; must be >= LABELSIZE+1
//	char *there: Filled with the room name there; must be >= LABELSIZE+1
//
// Return value:
//	TRUE: next room found
//	FALSE: no more rooms found
//
// Notes:
//	Before calling this, get_first_room() must be called, and it must return
//	TRUE. If get_first_room() returns FALSE, this must not be called.  If
//	get_first_room() returns TRUE, this must keep getting called until it
//	returns FALSE.	get_first_room() must not be called until this returns
//	FALSE.	After this returns FALSE, it must not be called again until
//	get_first_room() is called and returns TRUE.

Bool TERMWINDOWMEMBER get_next_room(char *here, char *there)
	{
	assert(nodefile);

	char line[95];

	while (fgets(line, 90, nodefile) != NULL)
		{
		if (line[0] != '#')
			{
			continue;
			}

		char *words[256];
		const int count = parse_it(words, line);

		if (SameString(words[0], getnetmsg(37)))
			{
			fclose(nodefile);
			nodefile = NULL;
			return (FALSE);
			}

		if (SameString(words[0], getnetmsg(38)))
			{
			CopyString2Buffer(here, words[1], LABELSIZE);

			if (count > 2)
				{
				CopyString2Buffer(there, words[2], LABELSIZE);
				}
			else
				{
				strcpy(there, here);
				}

			return (TRUE);
			}
		}

	fclose(nodefile);
	nodefile = NULL;
	return (FALSE);
	}
