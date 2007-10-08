// --------------------------------------------------------------------------
// Citadel: RoomShow.CPP
//
// Stuff for showing information from the room database

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "boolexpr.h"
#include "log.h"
#include "group.h"
#include "msg.h"
#include "tallybuf.h"


// --------------------------------------------------------------------------
// dumpRoom(): tells us # new messages etc.
//
// Input:
//	Bool infoLineAndAutoApp: TRUE to show info line and run room's auto-app;
//		FALSE to not.
//
// Return value:
//	None.

void TERMWINDOWMEMBER dumpRoom(Bool infoLineAndAutoApp)
	{
	doCR();

	if (infoLineAndAutoApp)
		{
		char Buffer[80];
		if (*CurrentRoom->GetDescFile(Buffer, sizeof(Buffer)) &&  !Talley->Visited(thisRoom))
			{
			showRoomDescription();
			}
		else if (*CurrentRoom->GetInfoLine(Buffer, sizeof(Buffer)) && CurrentUser->IsViewRoomInfoLines())
			{
			mPrintfCR(getmsg(634), Buffer);
			}
		}

	// hmmm... where to put this?
	if (infoLineAndAutoApp && CurrentRoom->IsApplication() && CurrentRoom->IsAutoApp() && !Talley->Visited(thisRoom))
		{
		ExeAplic();
		}

	if (CurrentUser->IsAide())
		{
		mPrintf(getmsg(635), ltoac(Talley->TotalMessagesInRoom(thisRoom)));
		}

	const m_slot messages = Talley->MessagesInRoom(thisRoom);
	mPrintf(getmsg(636), ltoac(messages), (messages == 1) ? cfg.Lmsg_nym : cfg.Lmsgs_nym);

	const m_slot newMsgs = Talley->NewInRoom(thisRoom);
	if (newMsgs && loggedIn)
		{
		mPrintf(getmsg(637), ltoac(newMsgs));
		}

	mPrintfCR(getmsg(433));

	if (Talley->HasNewMail(thisRoom) && loggedIn)
		{
		mPrintfCR(getmsg(591));
		}

	if (Talley->HasOldMail(thisRoom) && loggedIn)
		{
		mPrintfCR(getmsg(599));
		}

	label NetID;
	RoomTab[thisRoom].GetNetID(NetID, sizeof(NetID));

	// maybe IsRoomShared check? definitely not!
	if (*NetID && CurrentUser->IsSysop() && IsNetIDCorrupted(NetID))
		{
		mPrintfCR(getmsg(464));
		}
	}


// --------------------------------------------------------------------------
// showRoomDescription(): Displays room description
//
// Input:
//	None.
//
// Return value:
//	None.

void TERMWINDOWMEMBER showRoomDescription(void)
	{
	OC.SetOutFlag(OUTOK);

	label DescFile;
	CurrentRoom->GetDescFile(DescFile, sizeof(DescFile));

	if (CurrentUser->IsViewRoomDesc() && *DescFile && !Talley->Visited(thisRoom) && IsFilenameLegal(DescFile, NULL))
		{
		char pathToRoomDesc[128];
		sprintf(pathToRoomDesc, sbs, cfg.roompath, DescFile);

		if (filexists(pathToRoomDesc))
			{
			if (!CurrentUser->IsExpert())
				{
				CRmPrintfCR(getmsg(577));
				}

			dumpf(pathToRoomDesc, TRUE, 0);
			doCR();

			OC.SetOutFlag(OUTOK);
			}
		else
			{
			mPrintfCR(getmsg(427), cfg.Lroom_nym, DescFile);
			}

		doCR();
		}
	}
