// --------------------------------------------------------------------------
// Citadel: RoomMake.CPP
//
// Room creation code

#include "ctdl.h"
#pragma hdrstop

#include "libovl.h"
#include "room.h"
#include "log.h"
#include "group.h"
#include "net.h"
#include "hall.h"
#include "miscovl.h"
#include "domenu.h"
#include "termwndw.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// EnterRoom()		constructs a new room via dialogue with user.
// AutoRoom()       auto-creates a net room


// --------------------------------------------------------------------------
// EnterRoom()		constructs a new room via dialogue with user.
//
// Notes:
//	This is a bit icky in that it uses CurrentRoom to create the new room.
//		This should be changed to using its own buffer.

void TERMWINDOWMEMBER EnterRoom(Bool GroupOnly)
	{
	doCR();

	if (!loggedIn)
		{
		CRmPrintfCR(getmsg(239));
		return;
		}

	if (!LockMenu(MENU_ENTER))
		{
		return;
		}

	// The user must be able to make a room, and at least one of the remaining bits
	if (CurrentUser->IsMakeRoom()  && (cfg.nonAideRoomOk || HallData[thisHall].IsEnterRoom() || CurrentUser->IsAide()))
		{
		// There must be no limit to the number of rooms to make, be an aide, or be under the limit
		if (!cfg.numRooms || CurrentUser->IsAide() || roomsMade < cfg.numRooms)
			{
			label roomname;
			label groupname;
			g_slot groupslot;
			r_slot newSlot;
			Bool test;

			SetDoWhat(ENTERROOM);

			newSlot = FindFreeRoomSlot();

			if (newSlot == CERROR)
				{
				mPrintf(getmsg(490), cfg.Uroom_nym);
				UnlockMenu(MENU_ENTER);
				return;
				}

			if (!CurrentUser->IsExpert())
				{
				dispBlb(B_NEWROOM);
				doCR();
				}

			do
				{
				char Prompt[128];
				sprintf(Prompt, getmsg(106), cfg.Lroom_nym);

				do
					{
					getString(Prompt, roomname, LABELSIZE, ns);

					if (*roomname == '?')
						{
						MRO.Verbose = FALSE;
						listRooms(0);
						}
					} while (*roomname == '?');

				normalizeString(roomname);

				if (!*roomname)
					{
					UnlockMenu(MENU_ENTER);
					return;
					}

				test = TRUE;
				if (RoomExists(roomname) != CERROR)
					{
					mPrintfCR(getmsg(105), roomname, cfg.Lroom_nym);
					test = FALSE;
					}
				} while (!test);

			CurrentRoom->Clear();

			if (GroupOnly)
				{
				char String[128];
				sprintf(String, getmsg(588), cfg.Lgroup_nym, cfg.Lroom_nym);

				do
					{
					if (!AskGroupName(groupname, ns, String, LABELSIZE, TRUE))
						{
						CurrentRoom->Load(thisRoom); // From official release
						UnlockMenu(MENU_ENTER);
						return;
						}

					groupslot = FindGroupByPartialName(groupname, FALSE);

					test = TRUE;
					if (groupslot == CERROR || !CurrentUser->IsInGroup(groupslot))
						{
						CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
						test = FALSE;
						}
					} while (!test);

				CurrentRoom->SetGroup(groupslot);
				}

			CurrentRoom->SetInuse(TRUE);
			CurrentRoom->SetGroupOnly(GroupOnly);

			char Prompt[128], InfoLine[80];

			sprintf(Prompt, getmsg(107), cfg.Lroom_nym);
			getNormStr(Prompt, InfoLine, 79);
			CurrentRoom->SetInfoLine(InfoLine);

			sprintf(Prompt, getmenumsg(MENU_ENTER, 43), cfg.Lroom_nym);
			CurrentRoom->SetPublic(getYesNo(Prompt, 1));

			sprintf(Prompt, getmenumsg(MENU_ENTER, 44), roomname, (CurrentRoom->IsPublic()) ?
					getmenumsg(MENU_ENTER, 45) : getmenumsg(MENU_ENTER, 46), cfg.Lroom_nym);

			if (!getYesNo(Prompt, 0))
				{
				CurrentRoom->Load(thisRoom);
				UnlockMenu(MENU_ENTER);
				return;
				}

			CurrentRoom->SetName(roomname);

			label L;
			CurrentUser->GetName(L, sizeof(L));
			CurrentRoom->SetCreator(L);

			// Make sure that there are no stray messages still in this room
			MessageDat.EnsureRoomEmpty(newSlot);

			// Borgah!						VVV as if it could be BIO...
			if (!CurrentRoom->IsPublic() || CurrentRoom->IsBIO())
				{
				cycleroom(newSlot, FALSE, TRUE);
				}
			else
				{
				cycleroom(newSlot, TRUE, TRUE);
				}

			CurrentRoom->Save(newSlot);

			// remove and unwindow room from all halls
			for (h_slot j = 0; j < cfg.maxhalls; j++)
				{
				HallData[j].SetRoomInHall(newSlot, FALSE);
				HallData[j].SetWindowedIntoHall(newSlot, FALSE);
				}

			// put room in current hall
			HallData[thisHall].SetRoomInHall(newSlot, TRUE);

			// put room in maintenance hall
			HallData[MAINTENANCE].SetRoomInHall(newSlot, TRUE);

			HallData.Save();

			Message *Msg = new Message;

			if (Msg)
				{
				label UserName;
				CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
                trap(T_NEWROOM, WindowCaption, getmsg(110), roomname, CurrentRoom->IsPublic() ? '>' : ')', UserName);
#else
                trap(T_NEWROOM, getmsg(110), roomname, CurrentRoom->IsPublic() ? '>' : ')', UserName);
#endif

				Msg->SetTextWithFormat(getmsg(457), cfg.Uroom_nym, roomname, CurrentRoom->IsPublic() ? '>' : ')',
						UserName);

				Msg->SetRoomNumber(newSlot);

				systemMessage(Msg);

				// Do it again because of funky compression stuff.
				Msg->SetTextWithFormat(getmsg(457), cfg.Uroom_nym, roomname, CurrentRoom->IsPublic() ? '>' : ')',
						UserName);

				if (CurrentRoom->IsGroupOnly())
					{
					Msg->SetGroup(CurrentRoom->GetGroup());
					}

				Msg->SetRoomNumber(AIDEROOM);

				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(50);
				}

			CurrentUser->SetInRoom(newSlot, TRUE);

			roomsMade++;

			gotoRoom(roomname, FALSE, FALSE, FALSE);
			}
		else
			{
			CRmPrintfCR(getmenumsg(MENU_ENTER, 31), ltoac(cfg.numRooms), cfg.Lroom_nym);
			}
		}
	else
		{
		CRmPrintfCR(getmenumsg(MENU_ENTER, 32), cfg.Lrooms_nym);
		}

	UnlockMenu(MENU_ENTER);
	}

// autoroom, auto-creates a net room
Bool AutoRoom(const char *NewName, const char *Creator, g_slot NewGroup, h_slot NewHall)
	{
	if (!*NewName)
		{
		return (FALSE);
		}

	r_slot testslot = RoomExists(NewName);
	if (testslot != CERROR)
		{
		return (FALSE);
		}

	testslot = IdExists(NewName, FALSE);
	if (testslot != CERROR)
		{
		return (FALSE);
		}

	// add: locking the room database now, to ensure that this slot continues to be free
	const r_slot newSlot = FindFreeRoomSlot();
	if (newSlot == CERROR)
		{
		return (FALSE);
		}

	RoomC RoomBuffer;

	RoomBuffer.SetInuse(TRUE);
	RoomBuffer.SetPublic(TRUE);
	RoomBuffer.SetShared(TRUE);

	RoomBuffer.SetNetID(NewName);
	RoomBuffer.SetName(NewName);

	RoomBuffer.SetCreator(Creator);

	// Make sure no messages are in this room currently
	MessageDat.EnsureRoomEmpty(newSlot);

	cycleroom(newSlot, TRUE, TRUE);
#ifndef WINCIT
	// cycleroom() does this in wincit with its data syncronization. non-wincit needs CurrentUser to be updated
	CurrentUser->SetInRoom(newSlot, TRUE); 
#endif

	if (NewGroup >= 0)
		{
		RoomBuffer.SetGroupOnly(TRUE);
		RoomBuffer.SetBooleanGroup(FALSE);
		RoomBuffer.SetGroup(NewGroup);
		}

	// Create the room!
	RoomBuffer.Save(newSlot);

	// remove and unwindow room from all halls
	for (h_slot j = 0; j < cfg.maxhalls; j++)
		{
		HallData[j].SetRoomInHall(newSlot, FALSE);
		HallData[j].SetWindowedIntoHall(newSlot, FALSE);
		}

	// put room in maintenance hall
	HallData[MAINTENANCE].SetRoomInHall(newSlot, TRUE);

	// put room in hall if hall set in nodes.cit
	if (NewHall >= 0)
		{
		HallData[NewHall].SetRoomInHall(newSlot, TRUE);
		}

	HallData.Save();

	Message *Msg = new Message;

	if (Msg)
		{
#ifdef WINCIT
        trap(T_NEWROOM, "wow", getmsg(110), NewName, '>', Creator);
#else
        trap(T_NEWROOM, getmsg(110), NewName, '>', Creator);
#endif

		Msg->SetTextWithFormat(getmsg(457), cfg.Uroom_nym, NewName, '>', Creator);
		Msg->SetRoomNumber(newSlot);

		systemMessage(Msg);
		delete Msg;
		}
	else
		{
		OutOfMemory(46);
		}

	AddNetIDToNetIDCit(NewName, Creator);

	return (TRUE);
	}
