// --------------------------------------------------------------------------
// Citadel: Misc2.CPP
//
// Overlayed miscellaneous stuff that is used often; compare with Misc3.CPP.

#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "room.h"
#include "tallybuf.h"
#include "libovl.h"
#include "log.h"
#include "hall.h"
#include "cfgfiles.h"
#include "term.h"
#include "domenu.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// offhook()		sysop fn to take modem off hook
// greeting()		System-entry blurb etc
// EnterBorder() 	edit a border line.
// digitbaud()		Calculates digit baud rate (0-4) from number (300+)


// --------------------------------------------------------------------------
// greeting(): gives system-entry blurb, etc.

void TERMWINDOWMEMBER greeting(void)
	{
	MRO.Verbose = FALSE;

	if (loggedIn)
		{
		terminate(FALSE);
		}

	OC.Echo = BOTH;
	OC.setio();

	setdefaultconfig(FALSE);

	pause(10);

	cls(SCROLL_SAVE);

	doccr();

	StatusLine.Update(WC_TWp);

	if (modStat)
		{
		if (cfg.connectwait)
			{
			CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(84));

			pause(cfg.connectwait * 100);

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
			}

		CommPort->FlushInput();
		}

	// make sure we want to talk to this baud rate
	if (modStat && (CommPort->GetModemSpeed() < cfg.minbaud))
		{
		dispBlb(B_TOOLOW);

		CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(82));

		Hangup();
		pause(200);

		if (w)
			{
			destroyCitWindow(w, FALSE);
			}
		}
	else
		{
		OC.User.SetCanControlD(TRUE);

		// set terminal
		autoansi();

		OC.SetOutFlag(OUTOK);

		if (modStat || debug)
			{
			hello();
			doCR();
			}

		OC.SetOutFlag(OUTOK);

		mPrintfCR(getmsg(683), cfg.nodeTitle, cfg.nodeRegion, cfg.nodeCountry);
		mPrintfCR(getmsg(682), cfg.softverb, *cfg.softverb ? spc : ns, programName, version);
		mPrintf(pcts, Author);

#if VERSION != RELEASE
		doCR();
#ifndef HARRY
		CRCRmPrintfCR(" 2=== 1NOTE02 ===0");
#else
        CRCRmPrintfCR(" 2=== NOTE ===0");
#endif
#ifndef NDEBUG
		CRmPrintf("This BBS is running pre-release software.  Because this is a test version ");
		mPrintf("of the software, it has extra code to assure that things are running as ");
		mPrintf("they should.  This may cause a noticeable slow-down in the operation of ");
		mPrintf("the board.  Also, because this is pre-release software, it may contain ");
		mPrintf("bugs that could cause a loss of data.  This is not likely, but it is best ");
		mPrintfCR("to be aware of potential problems before they surprise you.");
#else
		mPrintf("This BBS is running pre-release software.  Because this is pre-release ");
		mPrintf("software, it may contain ");
		mPrintf("bugs that could cause a loss of data.  This is not likely, but it is best ");
		mPrintfCR("to be aware of potential problems before they surprise you.");
#endif
#ifndef HARRY
		CRmPrintfCR(" 2=== 1NOTE02 ===0");
#else
        CRmPrintfCR(" 2=== NOTE ===0");
#endif
#endif


		char dtstr[80];
		strftime(dtstr, 79, cfg.vdatestamp, 0l);

		doCR();
		CRmPrintf(pcts, dtstr);

		if (!cfg.forcelogin)
			{
			CRmPrintf(getmsg(677));
			CRmPrintf(getmsg(678));
			CRmPrintf(getmsg(679));
			}

		CurrentRoom->Load(LOBBY);
		checkdir();
		thisRoom = LOBBY;

		const ulong messages = Talley->MessagesInRoom(thisRoom);

		CRmPrintfCR(getmsg(144), ltoac(messages), (messages == 1) ? cfg.Lmsg_nym : cfg.Lmsgs_nym);

		CommPort->FlushInput();
		}
	}


// --------------------------------------------------------------------------
// EnterBorder(): Edit a border line.

void TERMWINDOWMEMBER EnterBorder(void)
	{
	if (!LockMenu(MENU_ENTER))
		{
		return;
		}

	SetDoWhat(ENTERBORDER);

	if (!CurrentUser->IsEnterBorders() && !CurrentUser->IsSysop())
		{
		CRCRmPrintfCR(getmenumsg(MENU_ENTER, 18));
		}
	else
		{
		doCR(2);

		if (!cfg.borders)
			{
			mPrintfCR(getmenumsg(MENU_ENTER, 38));
			UnlockMenu(MENU_ENTER);
			return;
			}

		OC.SetOutFlag(OUTOK);

		for (int i = 0; i < cfg.maxborders; i++)
			{
			mPrintf(getmenumsg(MENU_ENTER, 39), i + 1);

			if (borders[i * (BORDERSIZE + 1)])
				{
				mPrintf(pcts, &(borders[i * (BORDERSIZE + 1)]));
				termCap(TERM_NORMAL);
				}
			else
				{
				mPrintf(getmenumsg(MENU_ENTER, 40));
				}

			doCR(2);
			}

		const int ToEdit = (int) getNumber(getmenumsg(MENU_ENTER, 41), 0L, (long) cfg.maxborders, 0L, FALSE, NULL);

		if (ToEdit)
			{
			CRmPrintf(getmenumsg(MENU_ENTER, 39), ToEdit);

			if (borders[(ToEdit - 1) * (BORDERSIZE + 1)])
				{
				mPrintf(pcts, &(borders[(ToEdit - 1) * (BORDERSIZE + 1)]));
				termCap(TERM_NORMAL);
				}
			else
				{
				mPrintf(getmenumsg(MENU_ENTER, 40));
				}

			char stuff[BORDERSIZE + 1];

			doCR();
			getString(getmenumsg(MENU_ENTER, 42), stuff, BORDERSIZE, FALSE, ns);

			if (*stuff)
				{
				CopyString2Buffer(&(borders[(ToEdit - 1) * (BORDERSIZE + 1)]), stuff, BORDERSIZE + 1);
				normalizeString(&(borders[(ToEdit - 1) * (BORDERSIZE + 1)]));
				writeBordersDat();
				}
			}
		}

	UnlockMenu(MENU_ENTER);
	}


// --------------------------------------------------------------------------
// digitbaud(): Calculates PortSpeedE baud rate from number
//
// Input:
//	bigbaud: The baud to find the PortSpeedE of. This can be in PortSpeedE
//		format already, or in 300/600/1200... format.
//
// Return value:
//	PortSpeedE format of bigbaud's rate.
//
// Notes:
//	If bigbaud could not be understood, this returns PS_ERROR.

PortSpeedE digitbaud(long bigbaud)
	{
	if (bigbaud < PS_NUM - 1)
		{
		// This is a little bit of a kludge to keep people from having to
		// change data files when 600 baud was introduced. Pretty ugly, huh?

		if (bigbaud > 0)
			{
			bigbaud++;
			}

		return ((PortSpeedE) bigbaud);
		}
	else
		{
		for (PortSpeedE i = (PortSpeedE) 0; i < PS_NUM; i = (PortSpeedE) (i + 1))
			{
			if (bigbaud == bauds[i])
				{
				return (i);
				}
			}

		return (PS_ERROR);
		}
	}


// --------------------------------------------------------------------------
// digitConnectbaud(): Calculates ModemSpeedE baud rate from number
//
// Input:
//	bigbaud: The baud to find the ModemSpeedE of. This can be in ModemSpeedE
//		format already, or in 300/600/1200... format.
//
// Return value:
//	ModemSpeedE format of bigbaud's rate.
//
// Notes:
//	If bigbaud could not be understood, this returns MS_ERROR.

ModemSpeedE digitConnectbaud(long bigbaud)
	{
	if (bigbaud < MS_NUM - 1)
		{
		// This is a little bit of a kludge to keep people from having to
		// change data files when 600 baud was introduced. Pretty ugly, huh?

		if (bigbaud > 0)
			{
			bigbaud++;
			}

		return ((ModemSpeedE) bigbaud);
		}
	else
		{
		for (ModemSpeedE i = (ModemSpeedE) 0; i < MS_NUM; i = (ModemSpeedE) (i + 1))
			{
			if (bigbaud == connectbauds[i])
				{
				return (i);
				}
			}

		return (MS_ERROR);
		}
	}


// --------------------------------------------------------------------------
// offhook(): Take the modem off-hook.

void TERMWINDOWMEMBER offhook(void)
	{
	CITWINDOW *w = CitWindowsMsg(NULL, getmsg(19));

	Initport();

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}

	CommPort->OutString(cfg.offhookstr);
	CommPort->OutString(br);
#ifndef WINCIT
	CommPort->Disable();
#endif
	}

void TERMWINDOWMEMBER displayOnOff(const char *string, int value)
	{
	mPrintf(string, value ? getmsg(549) : getmsg(550));
	}

void TERMWINDOWMEMBER displayYesNo(const char *string, int value)
	{
	mPrintf(string, value ? getmsg(521) : getmsg(522));
	}

Bool TERMWINDOWMEMBER changeOnOff(const char *str, Bool value)
	{
	mPrintfCR(str, value ? getmsg(550) : getmsg(549));
	return (!value);
	}

Bool TERMWINDOWMEMBER changeYesNo(const char *str, Bool value)
	{
	mPrintfCR(str, value ? getmsg(522) : getmsg(521));
	return (!value);
	}

void cdecl nullFunc(void)
	{
	}

void cyclesignature(void)
	{
	FILE *fBuf;
	char line[256];
	char *words[256];
	Bool found = FALSE;
	char path[80];

	// there is only one signature line, so get out and use it
	if (cfg.sig_current_pos == LONG_MAX)
		{
		return;
		}

	sprintf(path, sbs, cfg.homepath, citfiles[C_CONFIG_CIT]);

	if ((fBuf = fopen(path, FO_R)) == NULL) // ASCII mode
		{
		return;
		}

	fseek(fBuf, cfg.sig_current_pos, SEEK_SET);

	Bool HaveStartedOver = FALSE;

	for (; !found;)
		{
		// if end of file cycle back up
		if (fgets(line, 254, fBuf) == NULL)
			{
			if (!HaveStartedOver)
				{
				fseek(fBuf, cfg.sig_first_pos, SEEK_SET);
				HaveStartedOver = TRUE;
				}
			else 
				{
				found = TRUE;	// not found, but this breaks us out of the loop
				}

			continue;
			}

		cfg.sig_current_pos = ftell(fBuf);

		if (line[0] != '#')
			{
			continue;
			}

		if (strnicmp(line, getmsg(137), strlen(getmsg(137))) != SAMESTRING)
			{
			// okay we've gone past the last signature, cycle back up
			if (!HaveStartedOver)
				{
				fseek(fBuf, cfg.sig_first_pos, SEEK_SET);
				HaveStartedOver = TRUE;
				}
			else 
				{
				found = TRUE;	// not found, but this breaks us out of the loop
				}

			continue;
			}
		else
			{
			found = TRUE;
			}

		parse_it(words, line);

		CopyStringToBuffer(cfg.nodeSignature, words[1]);

		cfg.sig_current_pos = ftell(fBuf);
		}

	fclose(fBuf);
	}


// --------------------------------------------------------------------------
// theAlgorithm(): Determine's users ability to get to a room.
//
// Input:
//	r_slot rm: The room we wonder about
//	h_slot hl: pass starting hall (usually thisHall)
//	Bool exclude: TRUE means that excluded rooms should be considered
//		inaccessible
//
// Return value:
//	TRUE: The room can be accessed
//	FALSE: The room cannot be accessed
//
// Notes:
//	This is a recursive function.
//	If this function runs out of memory, it returns FALSE for almost all rooms

Bool TERMWINDOWMEMBER theAlgorithm(r_slot rm, h_slot hl, Bool exclude)
	{
	assert(rm >= -1);
	assert(rm < cfg.maxrooms);
	assert(hl >= -1);
	assert(hl < cfg.maxhalls);

	static BitBag *hallCheck;
	Bool lowest = FALSE;

	if (!hallCheck)
		{
		if (rm == thisRoom || (rm == LOBBY && cfg.subhubs == 4))
			{
			return (TRUE);
			}

		if ((exclude && CurrentUser->IsRoomExcluded(rm)) || !CurrentUser->CanAccessRoom(rm))
			{
			return (FALSE);
			}

		hallCheck = new BitBag(cfg.maxhalls);

		lowest = TRUE;
		}

	if (!hallCheck)
		{
		OutOfMemory(78);
		}
	else
		{
		if (hallCheck->BitTest(hl))
			{
			return (FALSE);
			}
		else
			{
			hallCheck->BitSet(hl, TRUE);

			if (roominhall(rm, hl))
				{
				delete hallCheck;
				hallCheck = NULL;

				return (TRUE);
				}

			for (r_slot rmCnt = 0; rmCnt < cfg.maxrooms; rmCnt++)
				{
				if (CurrentUser->CanAccessRoom(rmCnt) && (!exclude || !CurrentUser->IsRoomExcluded(rmCnt)) &&
						roominhall(rmCnt, hl) && iswindow(rmCnt))
					{
					for (h_slot hlCnt = 0; hlCnt < cfg.maxhalls; hlCnt++)
						{
						if (HallData[hlCnt].IsInuse() && CurrentUser->CanAccessHall(hlCnt) &&
								HallData[hlCnt].IsWindowedIntoHall(rmCnt))
							{
							if (theAlgorithm(rm, hlCnt, exclude))
								{
								return (TRUE);
								}
							}
						}
					}
				}
			}
		}

	if (lowest && hallCheck)
		{
		delete hallCheck;
		hallCheck = NULL;
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// theOtherAlgorithm(): Determine user's ability to get to each room on the system
//
// Input:
//	Bool *rooms: A porinter to a cfg.maxrooms array of Bools
//	h_slot hl: The hall to start in; usually thisHall
//	Bool exclude: TRUE means that excluded rooms should be considered
//		inaccessible
//
// Output:
//	Bool *rooms: Each member of the array is set to FALSE (user cannot
//		access the corresponding room) or TRUE (user can access it)

void TERMWINDOWMEMBER theOtherAlgorithm(Bool *rooms, h_slot hl, Bool exclude)
	{
	assert(hl >= -1);
	assert(hl < cfg.maxhalls);

	static BitBag *hallCheck;
	Bool lowest = FALSE;

	if (!hallCheck)
		{
		hallCheck = new BitBag(cfg.maxhalls);
		memset(rooms, 0, sizeof(Bool) * cfg.maxrooms);

		lowest = TRUE;
		}

	if (!hallCheck)
		{
		OutOfMemory(79);
		}
	else
		{
		if (hallCheck->BitTest(hl))
			{
			return;
			}
		else
			{
			hallCheck->BitSet(hl, TRUE);

            if (cfg.subhubs != 4)
                {

			for (r_slot rmCnt = 0; rmCnt < cfg.maxrooms; rmCnt++)
				{
				if (!rooms[rmCnt] && CurrentUser->CanAccessRoom(rmCnt) && roominhall(rmCnt, hl) &&
						(!exclude || !CurrentUser->IsRoomExcluded(rmCnt)))
					{
					rooms[rmCnt] = TRUE;

                    if (iswindow(rmCnt))
						{
						for (h_slot hlCnt = 0; hlCnt < cfg.maxhalls; hlCnt++)
							{
							if (HallData[hlCnt].IsInuse() && CurrentUser->CanAccessHall(hlCnt) &&
									HallData[hlCnt].IsWindowedIntoHall(rmCnt))
								{
								theOtherAlgorithm(rooms, hlCnt, exclude);
								}
							}
						}
					}
				}
                }
            else if (cfg.subhubs == 4)
                {
                for (r_slot rmCnt = 0; rmCnt < cfg.maxrooms; rmCnt++)
                    {
                    if (!rooms[rmCnt] && CurrentUser->CanAccessRoom(rmCnt) && roominhall(rmCnt, hl) &&
                            (!exclude || !CurrentUser->IsRoomExcluded(rmCnt)))
                        {
                        rooms[rmCnt] = TRUE;

                        if (rmCnt == 0)
                            {
                            for (h_slot hlCnt = 0; hlCnt < cfg.maxhalls; hlCnt++)
                                {
                                if (HallData[hlCnt].IsInuse() && CurrentUser->CanAccessHall(hlCnt))
                                    {
                                    theOtherAlgorithm(rooms, hlCnt, exclude);
                                    }
                                }
                            }
                        }
                    }
                }
			}
		}

	if (lowest && hallCheck)
		{
		delete hallCheck;
		hallCheck = NULL;
		}
	}
