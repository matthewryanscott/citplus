// --------------------------------------------------------------------------
// Citadel: NetDC15.CPP
//
// DragCit 1.5 Networking

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "net.h"
#include "filecmd.h"
#include "cwindows.h"
#include "filerdwr.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// dc15network()	During network master code



// --------------------------------------------------------------------------
// dc15network(): During network master code.

Bool TERMWINDOWMEMBER dc15network(Bool master)
	{
	char line[100], line2[100];
	label here, there;
	FILE *file;
	int i, rms;
	Bool done = FALSE;

	netFailed = FALSE;

	const protocols *theProt = GetProtocolByKey(node->GetProtocol(), TRUE);

	if (!theProt)
		{
		doccr();
		cPrintf(getnetmsg(177));
		doCR();
		return (FALSE);
		}

	if (!CommPort->HaveConnection())
		{
		return (FALSE);
		}

	sprintf(line, sbs, LocalTempPath, mesgTmp);
	unlink(line);

	sprintf(line, getnetmsg(123), LocalTempPath);
	unlink(line);

	sprintf(line, getnetmsg(124), LocalTempPath);
	unlink(line);

	sprintf(line, getnetmsg(125), LocalTempPath);
	unlink(line);

	if ((file = fopen(line, FO_AB)) == NULL)
		{
		perror(getnetmsg(25));
		return (FALSE);
		}

	for (i = get_first_room(here, there), rms = 0; i; i = get_next_room(here, there), rms++)
		{
		PutStr(file, there);
		}

	PutStr(file, ns);
	fclose(file);

	if (master)
		{
		sendRequest(theProt);
		if (!CommPort->HaveConnection())
			{
			return (FALSE);
			}

		reciveRequest(theProt);
		}
	else
		{
		reciveRequest(theProt);
		if (!CommPort->HaveConnection())
			{
			return (FALSE);
			}

		sendRequest(theProt);
		}

	if (!CommPort->HaveConnection() || netFailed)
		{
		return (FALSE);
		}

	if (master)
		{
		// clear the buffer
		while (CommPort->HaveConnection() && CommPort->IsInputReady())
			{
			CommPort->Input();
			}
		}

	makeSendFile();

	if (!CommPort->HaveConnection() || netFailed)
		{
		return (FALSE);
		}

	// wait for them to get their shit together
	cPrintf(getnetmsg(11));

	CommPort->Output('X');

	time_t t2 = 0;
	const time_t t = time(NULL);

	while (CommPort->HaveConnection() && !done)
		{
		if (time(NULL) > (t + (node->GetFetchTimeout() * 60)))
			{
			CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(73));

			Hangup();
			netFailed = TRUE;

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
			}

		KBReady();

		if (CommPort->IsInputReady())
			{
			i = CommPort->Input();
			if (i == 'X' || node->GetNetworkType() != NET_DCIT16)
				{
				done = TRUE;
				}
			else
				{
				DebugOut(pctc, i);
				}

			}

		// wake them up! (every second)
		if (time(NULL) != t2)
			{
			CommPort->Output('X');
			t2 = time(NULL);
			}

		// instantaneous reaction time isn't critical here
		CitIsIdle();
		}

	// wake them up!
	for (i = 0; i < 10; i++)
		{
		CommPort->Output('X');
		}

	doccr();

	if (!CommPort->HaveConnection() || netFailed)
		{
		return (FALSE);
		}

	if (master)
		{
		receiveFiles(theProt);
		if (!CommPort->HaveConnection() || netFailed)
			{
			return (FALSE);
			}

		sendFiles(theProt);
		}
	else
		{
		sendFiles(theProt);
		if (!CommPort->HaveConnection() || netFailed)
			{
			return (FALSE);
			}

		receiveFiles(theProt);
		}

	if (netFailed)
		{
		return (FALSE);
		}

	cPrintf(getnetmsg(84));
	doccr();

	CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

	Hangup();

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}

	OC.setio(); // hmmm

	cPrintf(getnetmsg(85), cfg.Lmsg_nym);
	doccr();

	sformat(line, node->GetExtractPacket(), getmsg(48), roomdataIn, 0);
	apsystem(line, TRUE);

	changedir(LocalTempPath);
	unlink(roomdataIn);

	for (i = 0; i < rms; i++)
		{
		sprintf(line, getnetmsg(126), i);
		sprintf(line2, getnetmsg(127), i);
		rename(line, line2);
		}

	sprintf(line, sbs, LocalTempPath, mesgTmp);
	sprintf(line2, getnetmsg(123), LocalTempPath);
	rename(line, line2);

	return (TRUE);
	}


// --------------------------------------------------------------------------
// sendRequest(): Send the room request file.

void TERMWINDOWMEMBER sendRequest(const protocols *theProt)
	{
	cPrintf(getnetmsg(86));
	doccr();

	wxsnd(LocalTempPath, roomreqOut, theProt, 0);

	unlink(roomreqOut);
	}


// --------------------------------------------------------------------------
// reciveRequest(): Recive the room request file.

void TERMWINDOWMEMBER reciveRequest(const protocols *theProt)
	{
	cPrintf(getnetmsg(87));
	doccr();

	wxrcv(LocalTempPath, roomreqIn, theProt);

	if (!filexists(roomreqIn))
		{
		CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(76));

		Hangup();
		netFailed = TRUE;

		if (w)
			{
			destroyCitWindow(w, FALSE);
			}
		}
	}


// --------------------------------------------------------------------------
// makeSendFile(): Make the file to send to remote.

void TERMWINDOWMEMBER makeSendFile(void)
	{
	char line[100], line2[100];
	label troo;
	label fn;
	FILE *file;
	int i = 0;
	int crcounter = 0;

	if ((file = fopen(roomreqIn, FO_RB)) == NULL)
		{
		perror(getnetmsg(90));
		return; // ok
		}

	doccr();
	cPrintf(getnetmsg(2));
	doccr();

	GetStr(file, troo, LABELSIZE);

	while (*troo && !feof(file))
		{
		r_slot rm = RoomExists(troo);

		if (rm == CERROR)
			{
			rm = IdExists(troo, TRUE);
			}

		if (rm != CERROR)
			{
			if (CurrentUser->CanAccessRoom(rm))
				{
				sprintf(fn, getnetmsg(126), i);
				cPrintf(getnetmsg(128), deansi(troo));
				if (!((crcounter+1) % 3))
					{
					doccr();
					}
				crcounter++;
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
			cPrintf(getnetmsg(97), deansi(troo));
			doccr();
			amPrintf(getnetmsg(6), troo, bn);
			netError = TRUE;
			}

		i++;
		GetStr(file, troo, LABELSIZE);
		}

	doccr();
	fclose(file);
	unlink(roomreqIn);

	cPrintf(getnetmsg(98));
	doccr();
	sprintf(line, sbs, cfg.transpath, node->GetMailFileName());
	sprintf(line2, sbs, LocalTempPath, mesgTmp);
	if ((file = fopen(line2, FO_WB)) != NULL)
		{
		fclose(file);
		}
	copyfile(line, line2);

	cPrintf(getnetmsg(99), cfg.Lmsg_nym);
	doccr();

	// Zip them up
	sprintf(line2, getnetmsg(113), mesgTmp, roomStar);
	sformat(line, node->GetCreatePacket(), getmsg(49), roomdataOut, line2, 0);
	apsystem(line, TRUE);

	// Remove them.
	changedir(LocalTempPath);
	ambigUnlink(roomStar);
	unlink(mesgTmp);
	}


// --------------------------------------------------------------------------
// sendFiles(): Send the data files.

void TERMWINDOWMEMBER sendFiles(const protocols *theProt)
	{
	cPrintf(getnetmsg(100));
	doccr();

	wxsnd(LocalTempPath, roomdataOut, theProt, 0);

	unlink(roomdataOut);
	}


// --------------------------------------------------------------------------
// receiveFiles(): Recive the data files.

void TERMWINDOWMEMBER receiveFiles(const protocols *theProt)
	{
	cPrintf(getnetmsg(101));
	doccr();

	wxrcv(LocalTempPath, roomdataIn, theProt);

	if (!filexists(roomdataIn))
		{
		CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(74));

		Hangup();
		netFailed = TRUE;

		if (w)
			{
			destroyCitWindow(w, FALSE);
			}
		}
	}
