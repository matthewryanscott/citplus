// --------------------------------------------------------------------------
// Citadel: HallEdit.CPP
//
// Code for the aide/sysop to edit halls.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "msg.h"
#include "libovl.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// globalhall() 	adds/removes rooms from current hall
// hallfunc()		adds/removes room from current hall
// xhallfunc()		called by hallfunc() for each hall
// killhall()		sysop special to kill a hall
// newhall()		sysop special to add a new hall
// renamehall() 	sysop special to rename a hall
// windowfunc() 	windows/unwindows room from current hall


// --------------------------------------------------------------------------
// globalhall(): Adds/removes rooms from hall.

void TERMWINDOWMEMBER globalhall(void)
	{
	Bool changed_room = FALSE;

	doCR();

	label hallname;
	if (!AskHallName(hallname, thisHall))
		{
		return;
		}

	h_slot hallslot = partialhall(hallname);

	if (hallslot == CERROR)
		{
		CRmPrintfCR(getmsg(157), cfg.Lhall_nym, hallname);
		return;
		}

	for (r_slot roomslot = 0; roomslot < cfg.maxrooms; roomslot++)
		{
		if (RoomTab[roomslot].IsInuse())
			{
			const int ret = xhallfunc(roomslot, hallslot, 3, TRUE);

			if (ret == CERROR)
				{
				break;
				}

			if (ret)
				{
				changed_room = TRUE;
				}
			}
		}

	if (changed_room)
		{
		label Buffer, Buffer1;
		amPrintf(getsysmsg(322), bn, CurrentUser->GetName(Buffer, sizeof(Buffer)), cfg.Lhall_nym,
				HallData[hallslot].GetName(Buffer1, sizeof(Buffer1)), bn);

		SaveAideMess(HallData[hallslot].IsOwned() ? HallData[hallslot].IsBoolGroup() ?
				GroupData[SPECIALSECURITY].GetName(Buffer, sizeof(Buffer)) :
				GroupData[HallData[hallslot].GetGroupNumber()].GetName(Buffer1, sizeof(Buffer1)) : NULL);
		}

	HallData.Save();
	}


// --------------------------------------------------------------------------
// hallfunc(): Adds/removes room from hall.

void TERMWINDOWMEMBER hallfunc(void)
	{
	label roomname, hallname;

	SetDoWhat(AIDEHALL);

	if (CurrentUser->IsUsePersonalHall())
		{
		CRCRmPrintfCR(getmsg(477), cfg.Lhall_nym);
		return;
		}

	if (!AskRoomName(roomname, thisRoom))
		{
		return;
		}

	r_slot roomslot = RoomExists(roomname);

	if (roomslot == CERROR)
		{
		roomslot = PartialRoomExists(roomname, thisRoom, TRUE);
		}

	if (roomslot == CERROR)
		{
		CRmPrintfCR(getmsg(368), cfg.Lroom_nym, roomname);
		return;
		}

	if (!AskHallName(hallname, thisHall))
		{
		return;
		}

	const h_slot hallslot = partialhall(hallname);

	if (hallslot == CERROR)
		{
		CRmPrintfCR(getmsg(157), cfg.Lhall_nym, hallname);
		return;
		}

	if (xhallfunc(roomslot, hallslot, 0, FALSE))
		{
		HallData.Save();
		}
	}


// --------------------------------------------------------------------------
// xhallfunc(): Called by hallfunc() for each hall.
//
// Return value:
//	TRUE	Room added/removed to/from hall.
//	FALSE	Room not added/removed to/from hall.
//	CERROR	User chose to abort operation.

int TERMWINDOWMEMBER xhallfunc(r_slot roomslot, h_slot hallslot, int xyn, Bool AddToAideMsg)
	{
	const Bool StartInHall = HallData[hallslot].IsRoomInHall(roomslot);
	label RoomName, HallName;
	RoomTab[roomslot].GetName(RoomName, sizeof(RoomName));
	HallData[hallslot].GetName(HallName, sizeof(HallName));

	char string[100];
	sprintf(string, StartInHall ? getmsg(482) : getmsg(485), cfg.Lroom_nym, RoomName, cfg.Lhall_nym, HallName);

	const int yn = getYesNo(string, xyn);

	if (yn == 2)
		{
		return (CERROR);
		}

	if (yn)
		{
		HallData[hallslot].SetRoomInHall(roomslot, !StartInHall);

		label UserName;
		CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
        trap(T_AIDE, WindowCaption, StartInHall ? getmsg(483) : getmsg(486), RoomName, HallName, UserName);
#else
        trap(T_AIDE, StartInHall ? getmsg(483) : getmsg(486), RoomName, HallName, UserName);
#endif


		if (!AddToAideMsg)
			{
			Message *Msg = new Message;

			if (Msg)
				{
				char MsgStr[256];
				sprintf(MsgStr, StartInHall ? getmsg(454) : getmsg(453), cfg.Uroom_nym, RoomName, cfg.Lhall_nym,
						HallName, UserName);

				Msg->SetText(MsgStr);

				if (HallData[hallslot].IsOwned())
					{
					if (HallData[hallslot].IsBoolGroup())
						{
						Msg->SetGroup(SPECIALSECURITY);
						}
					else
						{
						Msg->SetGroup(HallData[hallslot].GetGroupNumber());
						}
					}

				Msg->SetRoomNumber(AIDEROOM);
				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(47);
				}
			}
		else
			{
			amPrintf(StartInHall ? getmsg(484) : getmsg(487), RoomName, bn);
			}
		}

	return (yn);
	}


// --------------------------------------------------------------------------
// killhall(): sysop special to kill a hall.
//
// Notes:
//	Assumes MSG_SYSOP is locked.

Bool TERMWINDOWMEMBER killhall(const char *hn)
	{
	int empty = TRUE;
	label hallname, Buffer;
	h_slot hallslot;
	char string[100];

	if (!hn)
		{
		doCR();

		if (!AskHallName(hallname, thisHall))
			{
			return (FALSE);
			}

		hallslot = partialhall(hallname);
		}
	else
		{
		hallslot = hallexists(hn);
		}

	if (hallslot == CERROR)
		{
		if (!hn)
			{
			CRmPrintfCR(getmsg(157), cfg.Lhall_nym, hallname);
			}

		return (FALSE);
		}

	if (hallslot == 0 || hallslot == 1)
		{
		if (!hn)
			{
			label Buffer1;
			CRmPrintfCR(getsysmsg(198), HallData[MAINHALL].GetName(Buffer, sizeof(Buffer)),
					HallData[MAINTENANCE].GetName(Buffer1, sizeof(Buffer1)), cfg.Lhalls_nym);
			}

		return (FALSE);
		}

	// Check hall for any rooms
	for (r_slot i = 0; i < cfg.maxrooms && empty; i++)
		{
		if (RoomTab[i].IsInuse() && HallData[hallslot].IsRoomInHall(i))
			{
			empty = FALSE;
			}
		}

	if (!empty && !hn)
		{
		CRmPrintfCR(getsysmsg(199), cfg.Uhall_nym, HallData[hallslot].GetName(Buffer, sizeof(Buffer)), cfg.Lrooms_nym);

		OC.SetOutFlag(OUTOK);

		MRO.Verbose = FALSE;

		if (!CurrentUser->IsWideRoom())
			{
			prtList(LIST_START);
			}

		for (r_slot i = 0; i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP); i++)
			{
			if (RoomTab[i].IsInuse() && HallData[hallslot].IsRoomInHall(i))
				{
				printroomVer(i, FALSE);
				}
			}

		if (!CurrentUser->IsWideRoom())
			{
			prtList(LIST_END);
			}
		else
			{
			doCR();
			}

		sprintf(string, getsysmsg(84), cfg.Lhall_nym, HallData[hallslot].GetName(Buffer, sizeof(Buffer)));
		}

	OC.SetOutFlag(IMPERVIOUS);

	if (hn || getYesNo(string, 0))
		{
		HallData[hallslot].SetInuse(FALSE);
		HallData[hallslot].SetOwned(FALSE);

		for (r_slot i = 0; i < cfg.maxrooms; i++)
			{
			HallData[hallslot].SetRoomInHall(i, FALSE);
			HallData[hallslot].SetWindowedIntoHall(i, FALSE);
			}

		HallData.Save();

#ifdef WINCIT
        trap(T_SYSOP, WindowCaption, getsysmsg(207), HallData[hallslot].GetName(Buffer, sizeof(Buffer)));
#else
		trap(T_SYSOP, getsysmsg(207), HallData[hallslot].GetName(Buffer, sizeof(Buffer)));
#endif


		if (thisHall == hallslot)
			{
			thisHall = HallData.GetDefault();
			}

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}


// --------------------------------------------------------------------------
// newhall(): Sysop special to add a new hall.
//
// Notes:
//	Assumes MSG_SYSOP is locked.

Bool TERMWINDOWMEMBER newhall(const char *name, const char *group, r_slot room)
	{
	label hallname, groupname;
	h_slot i, slot = 0;
	g_slot groupslot;
	Bool test;

	// search for a free hall slot
	for (i = 0; i < cfg.maxhalls && !slot ; i++)
		{
		if (!HallData[i].IsInuse())
			{
			slot = i;
			}
		}

	if (!slot)
		{
		if (!name)
			{
			CRmPrintf(getmsg(490), cfg.Uhall_nym);
			}

		return (FALSE);
		}

	if (name)
		{
		if (hallexists(name) != CERROR)
			{
			return (FALSE);
			}
		else
			{
			CopyStringToBuffer(hallname, name);
			}
		}
	else
		{
		test = FALSE;

		char Prompt[128];
		sprintf(Prompt, getmsg(106), cfg.Lhall_nym);

		do
			{
			doCR();

			if (!AskHallName(hallname, ns, Prompt))
				{
				return (FALSE);
				}

			if (hallexists(hallname) != CERROR)
				{
				CRmPrintf(getmsg(105), hallname, cfg.Lhall_nym);
				}
			else
				{
				test = TRUE;
				}

			} while (!test);
		}

	if (group)
		{
		if (*group)
			{
			groupslot = FindGroupByPartialName(group, FALSE);

			if (groupslot == CERROR)
				{
				return (FALSE);
				}
			}

		CopyStringToBuffer(groupname, group);
		}
	else
		{
		test = FALSE;

		char Prompt[128];
		sprintf(Prompt, getmsg(588), cfg.Lgroup_nym, cfg.Lhall_nym);

		do
			{
			AskGroupName(groupname, ns, Prompt);

			if (!*groupname)
				{
				test = TRUE;
				}
			else
				{
				groupslot = FindGroupByPartialName(groupname, FALSE);

				if (groupslot == CERROR)
					{
					CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
					}
				else
					{
					test = TRUE;
					}
				}

			} while (!test);
		}

	if (!*groupname)
		{
		HallData[slot].SetOwned(FALSE);
		}
	else
		{
		HallData[slot].SetOwned(TRUE);
		HallData[slot].SetGroupNumber(groupslot);
		}

	HallData[slot].SetName(hallname);
	HallData[slot].SetInuse(TRUE);

	// make current room a window into current hall so we can get back
	HallData[thisHall].SetWindowedIntoHall(room, TRUE);

	// clear all rooms from new hall
	for (i = 0; i < cfg.maxrooms; i++)
		{
		HallData[slot].SetRoomInHall(i, FALSE);
		HallData[slot].SetWindowedIntoHall(i, FALSE);
		}

	// put current room in hall
	HallData[slot].SetRoomInHall(room, TRUE);

	// make current room a window into new hall
	HallData[slot].SetWindowedIntoHall(room, TRUE);

	if (name || getYesNo(getmsg(57), 0))
		{
		HallData.Save();

		label Buffer;
#ifdef WINCIT
        trap(T_SYSOP, WindowCaption, getsysmsg(200), HallData[slot].GetName(Buffer, sizeof(Buffer)));
#else
		trap(T_SYSOP, getsysmsg(200), HallData[slot].GetName(Buffer, sizeof(Buffer)));
#endif


		if (!name)
			{
			thisHall = slot;
			}

		return (TRUE);
		}
	else
		{
		HallData.Load();
		return (FALSE);
		}
	}


// --------------------------------------------------------------------------
// renamehall(): Sysop special to rename a hall.
//
// Notes:
//	Assumes MSG_SYSOP is locked.

Bool TERMWINDOWMEMBER renamehall(void)
	{
	label hallname, newname, Buffer;
	g_slot groupslot;
	h_slot hallslot;

	Bool prtMess = !CurrentUser->IsExpert();
	Bool quit = FALSE;

	doCR();

	if (!AskHallName(hallname, thisHall))
		{
		return (FALSE);
		}

	hallslot = partialhall(hallname);

	if (hallslot == CERROR)
		{
		CRmPrintfCR(getmsg(157), cfg.Lhall_nym, hallname);
		return (FALSE);
		}

	do
		{
		if (prtMess)
			{
			doCR();
			OC.SetOutFlag(OUTOK);

			mPrintfCR(getsysmsg(201), HallData[hallslot].GetName(Buffer, sizeof(Buffer)));

			if (HallData[hallslot].IsOwned() && HallData[hallslot].IsBoolGroup())
				{
				mPrintf(getsysmsg(202), ns);

				BoolExpr Buffer;
				ShowBooleanExpression(HallData[hallslot]. GetGroupExpression(Buffer), ShowGroup);
				}
			else
				{
				mPrintf(getsysmsg(202), HallData[hallslot].IsOwned() ?
						GroupData[HallData[hallslot].GetGroupNumber()].GetName(Buffer, sizeof(Buffer)) : getsysmsg(203));
				}

			doCR();

			if (!cfg.nonAideRoomOk)
				{
				displayYesNo(getsysmsg(204), HallData[hallslot].IsEnterRoom());
				doCR();
				}
			else
				{
				HallData[hallslot].SetEnterRoom(TRUE);
				}

			// use HallData.GetDefault instead of HallData[hallslot].IsDefault()
			// in case there is no default hall; MAINHALL is by default.
			displayYesNo(getsysmsg(272), HallData.GetDefault() == hallslot);
			doCR();
			
			mPrintfCR(getsysmsg(205), HallData[hallslot].IsDescribed() ?
					HallData[hallslot].GetDescriptionFile(Buffer, sizeof(Buffer)) : getsysmsg(179));

			CRmPrintfCR(getmsg(633));

			prtMess = !CurrentUser->IsExpert();
			}

		const int c = DoMenuPrompt(getmsg(616), NULL);

		if (!(HaveConnectionToUser()))
			{
			HallData.Load();
			return (FALSE);
			}

		switch (toupper(c))
			{
			case 'E':
				{
				if (HallData.GetDefault() == hallslot)
					{
					mPrintfCR(getsysmsg(358));
					}
				else
					{
					if (changeYesNo(getsysmsg(282), FALSE))
						{
						HallData.SetDefault(hallslot);
						}
					}

				break;
				}

			case 'N':
				{
				mPrintfCR(getmsg(248));

				char Prompt[128];
				sprintf(Prompt, getmsg(551), cfg.Lhall_nym);

				AskHallName(newname, hallslot, Prompt);

				if (*newname && (hallexists(newname) != CERROR)
						&& !SameString(newname, HallData[hallslot].GetName(Buffer, sizeof(Buffer))))
					{
					CRmPrintfCR(getsysmsg(198), newname, cfg.Lhall_nym);
					}
				else
					{
					if (*newname)
						{
						HallData[hallslot].SetName(newname);
						}
					}

				break;
				}

			case 'G':
				{
				mPrintfCR(getsysmsg(209), cfg.Lgroup_nym);

				if (HallData[hallslot].IsBoolGroup())
					{
					CRmPrintf(getsysmsg(45));

					BoolExpr Buffer;
					ShowBooleanExpression(HallData[hallslot].GetGroupExpression(Buffer), ShowGroup);
					doCR();
					}

				char Prompt[128];
				sprintf(Prompt, getmsg(548), cfg.Lgroup_nym, cfg.Lhall_nym);

				char GroupName[256];

				AskGroupName(GroupName, (HallData[hallslot].IsOwned() && !HallData[hallslot].IsBoolGroup()) ?
						HallData[hallslot].GetGroupNumber() : (g_slot) -1, Prompt, sizeof(GroupName) - 1);

				if (SameString(GroupName, spc))
					{
					HallData[hallslot].SetOwned(FALSE);
					HallData[hallslot].SetBoolGroup(FALSE);
					}
				else
					{
					groupslot = FindGroupByPartialName(GroupName, FALSE);

					if (*GroupName && (groupslot != CERROR))
						{
						HallData[hallslot].SetOwned(TRUE);
						HallData[hallslot].SetBoolGroup(FALSE);
						HallData[hallslot].SetGroupNumber(groupslot);
						}
					else
						{
						if (*GroupName)
							{
							BoolExpr Expression;

							if (!CreateBooleanExpression(GroupName,
#ifdef WINCIT
									TestGroupExists, Expression, this))
#else
									TestGroupExists, Expression))
#endif
								{
								CRmPrintfCR(getmsg(157), cfg.Lgroup_nym, GroupName);
								}
							else
								{
								HallData[hallslot].SetOwned(TRUE);
								HallData[hallslot].SetBoolGroup(TRUE);
								HallData[hallslot].SetGroupExpression(Expression);
								}
							}
						}
					}

				break;
				}


			case 'U':
				{
				if (!cfg.nonAideRoomOk)
					{
					HallData[hallslot].SetEnterRoom(changeYesNo(getsysmsg(211), HallData[hallslot].IsEnterRoom()));
					}
				else
					{
					BadMenuSelection(c);
					HallData[hallslot].SetEnterRoom(TRUE);
					}

				break;
				}

			case 'D':
				{
				mPrintfCR(getsysmsg(213));

				if (getYesNo(getsysmsg(214), HallData[hallslot].IsDescribed()))
					{
					label InputString;

					getString(getsysmsg(215), InputString, 12, FALSE,
							HallData[hallslot].GetDescriptionFile(Buffer, sizeof(Buffer)));

					normalizeString(InputString);

					HallData[hallslot].SetDescriptionFile(InputString);

					if (InputString[0])
						{
						HallData[hallslot].SetDescribed(TRUE);
						}
					else
						{
						HallData[hallslot].SetDescribed(FALSE);
						}
					}
				else
					{
					HallData[hallslot].SetDescribed(FALSE);
					}

				break;
				}

			case 'S':
				{
				mPrintfCR(getmsg(60));

				if (getYesNo(getmsg(652), FALSE))
					{
					quit = TRUE;
					}

				break;
				}

			case ESC:
			case 'A':
				{
				mPrintfCR(getmsg(653));

				if (getYesNo(getmsg(654), TRUE))
					{
					if (!HallData.Load())
						{
						mPrintf(getmsg(656));
						}

					return (FALSE);
					}

				break;
				}

			case '\r':
			case '\n':
			case '?':
				{
				mPrintfCR(getmsg(351));

				prtMess = TRUE;
				break;
				}

			default:
				{
				BadMenuSelection(c);
				break;
				}
			}
		} while (!quit);

#ifdef WINCIT
    trap(T_SYSOP, WindowCaption, getsysmsg(216), hallname, HallData[hallslot].GetName(Buffer, sizeof(Buffer)));
#else
	trap(T_SYSOP, getsysmsg(216), hallname, HallData[hallslot].GetName(Buffer, sizeof(Buffer)));
#endif

	HallData.Save();

	return (TRUE);
	}


// --------------------------------------------------------------------------
// windowfunc(): Windows/unwindows room from current hall.
//
// Notes:
//	Assumes MENU_AIDE is locked.

void TERMWINDOWMEMBER windowfunc(void)
	{
	r_slot roomslot;
	h_slot hallslot;
	Bool test;

	SetDoWhat(AIDEWINDOW);

	if (CurrentUser->IsUsePersonalHall())
		{
		CRCRmPrintfCR(getmsg(477), cfg.Lhall_nym);
		return;
		}

	do
		{
		label roomname;

		if (!AskRoomName(roomname, thisRoom))
			{
			return;
			}

		roomslot = RoomExists(roomname);

		if (roomslot == CERROR)
			{
			roomslot = PartialRoomExists(roomname, thisRoom, TRUE);
			}

		test = TRUE;
		if (roomslot == CERROR)
			{
			CRmPrintf(getmsg(157), cfg.Lroom_nym, roomname);
			test = FALSE;
			}
		} while (!test);

	do
		{
		label hallname;

		if (!AskHallName(hallname, thisHall))
			{
			return;
			}

		hallslot = partialhall(hallname);

		test = TRUE;
		if (hallslot == CERROR)
			{
			CRmPrintfCR(getmsg(157), cfg.Lhall_nym, hallname);
			test = FALSE;
			}
		} while (!test);

	label HallName, RoomName, UserName;
	HallData[hallslot].GetName(HallName, sizeof(HallName));
	RoomTab[roomslot].GetName(RoomName, sizeof(RoomName));
	CurrentUser->GetName(UserName, sizeof(UserName));

	if (HallData[hallslot].IsWindowedIntoHall(roomslot))
		{
		char string[256];

		sprintf(string, getmenumsg(MENU_AIDE, 146), cfg.Lroom_nym, RoomName, cfg.Lhall_nym, HallName);

		if (getYesNo(string,0))
			{
			HallData[hallslot].SetWindowedIntoHall(roomslot, FALSE);

#ifdef WINCIT
            trap(T_AIDE, WindowCaption, getmenumsg(MENU_AIDE, 147), HallName, RoomName, UserName);
#else
			trap(T_AIDE, getmenumsg(MENU_AIDE, 147), HallName, RoomName, UserName);
#endif 


			Message *Msg = new Message;

			if (Msg)
				{
				char MsgStr[256];
				sprintf(MsgStr, getmenumsg(MENU_AIDE, 148), cfg.Uhall_nym, HallName, cfg.Lroom_nym, RoomName, UserName);

				Msg->SetText(MsgStr);

				if (HallData[hallslot].IsOwned())
					{
					if (HallData[hallslot].IsBoolGroup())
						{
						Msg->SetGroup(SPECIALSECURITY);
						}
					else
						{
						Msg->SetGroup(HallData[hallslot].GetGroupNumber());
						}
					}

				Msg->SetRoomNumber(AIDEROOM);
				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(48);
				}

			HallData.Save();
			}
		}
	else
		{
		char string[256];

		sprintf(string, getmenumsg(MENU_AIDE, 150), cfg.Lroom_nym, RoomName, cfg.Lhall_nym, HallName);

		if (getYesNo(string,0))
			{
			HallData[hallslot].SetWindowedIntoHall(roomslot, TRUE);

#ifdef WINCIT
            trap(T_AIDE, WindowCaption, getmenumsg(MENU_AIDE, 151), HallName, RoomName, UserName);
#else
			trap(T_AIDE, getmenumsg(MENU_AIDE, 151), HallName, RoomName, UserName);
#endif



			Message *Msg = new Message;

			if (Msg)
				{
				char MsgStr[256];
				sprintf(MsgStr, getmenumsg(MENU_AIDE, 152), cfg.Uhall_nym, HallName, cfg.Lroom_nym, RoomName, UserName);
				Msg->SetText(MsgStr);

				if (HallData[hallslot].IsOwned())
					{
					if (HallData[hallslot].IsBoolGroup())
						{
						Msg->SetGroup(SPECIALSECURITY);
						}
					else
						{
						Msg->SetGroup(HallData[hallslot].GetGroupNumber());
						}
					}

				Msg->SetRoomNumber(AIDEROOM);
				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(48);
				}

			HallData.Save();

			//Maybe add target room to target hall too if not already there
			if (!HallData[hallslot].IsRoomInHall(roomslot))
				{
				xhallfunc(roomslot, hallslot, 1, FALSE);
				}
			}
		}
	}

void TERMWINDOWMEMBER AideHall(void)
	{
	doCR();

	if (!cfg.aidehall && !CurrentUser->IsSysop())
		{
		if (LockMenu(MENU_AIDE))
			{
			mPrintfCR(getmenumsg(MENU_AIDE, 19));
			UnlockMenu(MENU_AIDE);
			}
		}
	else
		{
		hallfunc();
		}
	}

void TERMWINDOWMEMBER AideWindow(void)
	{
	doCR();

	if (LockMenu(MENU_AIDE))
		{
		if (cfg.subhubs == 4)
			{
			mPrintfCR(getmenumsg(MENU_AIDE, 23));
			}
		else if (!cfg.aidehall && !CurrentUser->IsSysop())
			{
			mPrintfCR(getmenumsg(MENU_AIDE, 19));
			}
		else
			{
			windowfunc();
			}

		UnlockMenu(MENU_AIDE);
		}
	}
