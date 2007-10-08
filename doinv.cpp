// --------------------------------------------------------------------------
// Citadel: DoInv.CPP
//
// .Invite... commands

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "miscovl.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// doInvite()				handles .I(nvite) command


// --------------------------------------------------------------------------
// InviteUserList()

void TERMWINDOWMEMBER InviteUserList(void)
	{
	doCR();

	label roomname;
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
		CRmPrintf(getmsg(157), cfg.Lroom_nym, roomname);
		return;
		}

	LogEntry1 Log1;
	LogEntry3 Log3(cfg.maxrooms);
	l_slot i;

	OC.SetOutFlag(OUTOK);

	if (LockMenu(MENU_INV))
		{
		label Buffer;
		CRmPrintfCR(getmenumsg(MENU_INV, 4), cfg.Uroom_nym,
				RoomTab[roomslot].GetName(Buffer, sizeof(Buffer)), cfg.Lusers_nym);

		MRO.Verbose = FALSE;
		prtList(LIST_START);

		for (i = 0; i < cfg.MAXLOGTAB && OC.User.CanOutput(); i++)
			{
			if (LTab(i).IsInuse())
				{
				if (Log3.Load(LTab(i).GetLogIndex()) &&
						!Log3.IsInRoom(roomslot))
					{
					if (Log1.Load(LTab(i).GetLogIndex()))
						{
						label Buffer;
						prtList(Log1.GetName(Buffer, sizeof(Buffer)));
						}
					}
				}

			if (OC.User.GetOutFlag() == OUTSKIP)
				{
				UnlockMenu(MENU_INV);
				return;
				}
			}

		prtList(LIST_END);

		if (OC.User.GetOutFlag() != OUTSKIP)
			{
			OC.SetOutFlag(OUTOK);

			CRmPrintfCR(getmenumsg(MENU_INV, 5), cfg.Uroom_nym,
					RoomTab[roomslot].GetName(Buffer, sizeof(Buffer)),
					cfg.Lusers_nym);

			prtList(LIST_START);

			for (i = 0;
					i < cfg.MAXLOGTAB && (OC.User.GetOutFlag() != OUTSKIP);
					i++)
				{
				if (LTab(i).IsInuse())
					{
					if (Log3.Load(LTab(i).GetLogIndex()) &&
							Log3.IsInRoom(roomslot))
						{
						if (Log1.Load(LTab(i).GetLogIndex()))
							{
							label Buffer;
							prtList(Log1.GetName(Buffer, sizeof(Buffer)));
							}
						}
					}
				}

			prtList(LIST_END);
			}

		UnlockMenu(MENU_INV);
		}
	}


// --------------------------------------------------------------------------
// InviteRoomList()

void TERMWINDOWMEMBER InviteRoomList(void)
	{
	l_slot logNo;

	doCR();

	label who;
	if (!AskUserName(who, loggedIn ? CurrentUser : NULL))
		{
		return;
		}

	logNo = FindPersonByPartialName(who);
	if (logNo == CERROR)
		{
		mPrintfCR(getmsg(595), who);
		return;
		}

	if (LockMenu(MENU_INV))
		{
		LogEntry1 Log1;
		LogEntry3 Log3(cfg.maxrooms);
		r_slot i;

		if (Log1.Load(LTab(logNo).GetLogIndex()) &&
				Log3.Load(LTab(logNo).GetLogIndex()))
			{
			Log1.GetName(who, sizeof(who));

			doCR();
			OC.SetOutFlag(OUTOK);

			label Buffer;
			mPrintfCR(getmenumsg(MENU_INV, 6), cfg.Urooms_nym,
					Log1.GetName(Buffer, sizeof(Buffer)));

			MRO.Verbose = FALSE;
			if (!CurrentUser->IsWideRoom())
				{
				prtList(LIST_START);
				}

			for (i = 0;
					i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP);
					i++)
				{
				if (RoomTab[i].IsInuse() && !Log3.IsInRoom(i))
					{
					printroomVer(i, FALSE);
					}
				}

			if (OC.User.GetOutFlag() != OUTSKIP)
				{
				if (!CurrentUser->IsWideRoom())
					{
					prtList(LIST_END);
					}
				else
					{
					doCR();
					}

				OC.SetOutFlag(OUTOK);

				CRmPrintfCR(getmenumsg(MENU_INV, 7), cfg.Urooms_nym,
						Log1.GetName(Buffer, sizeof(Buffer)));

				MRO.Verbose = FALSE;
				prtList(LIST_START);

				for (i = 0;
						i < cfg.maxrooms && (OC.User.GetOutFlag() != OUTSKIP);
						i++)
					{
					if (RoomTab[i].IsInuse() && Log3.IsInRoom(i))
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
				}
			}

		UnlockMenu(MENU_INV);
		}
	}


// --------------------------------------------------------------------------
// InviteUser()

void TERMWINDOWMEMBER InviteUser(void)
	{
	doCR();

	label who;
	if (!AskUserName(who, loggedIn ? CurrentUser : NULL))
		{
		return;
		}

	l_slot logNo = FindPersonByPartialName(who);
	if (logNo == CERROR)
		{
		mPrintfCR(getmsg(595), who);
		return;
		}

	label roomname;
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
		CRmPrintf(getmsg(157), cfg.Lroom_nym, roomname);
		return;
		}

	if (LockMenu(MENU_INV))
		{
		LogEntry1 Log1;
		LogEntry3 Log3(cfg.maxrooms);

		if (Log1.Load(LTab(logNo).GetLogIndex()) &&
				Log3.Load(LTab(logNo).GetLogIndex()))
			{
			Log1.GetName(who, sizeof(who));

			if (!Log3.IsInRoom(roomslot))
				{
				char query[128];
				label Buffer;
				sprintf(query, getmenumsg(MENU_INV, 8), who, cfg.Lroom_nym,
						RoomTab[roomslot].GetName(Buffer, sizeof(Buffer)));

				if (getYesNo(query, 0))
					{
					Log3.SetInRoom(roomslot, TRUE);

					if (Log3.Save(LTab(logNo).GetLogIndex()))
						{
						label Buffer, Buffer1;
#ifndef WINCIT
						trap(T_AIDE, getmenumsg(MENU_INV, 9), who, RoomTab[roomslot].GetName(Buffer1, sizeof(Buffer1)),
								CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
                        trap(T_AIDE, WindowCaption, getmenumsg(MENU_INV, 9), who, RoomTab[roomslot].GetName(Buffer1, sizeof(Buffer1)),
								CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

						}
					}
				}
			else
				{
				char query[128];
				label Buffer;

				sprintf(query, getmenumsg(MENU_INV, 10), who, cfg.Lroom_nym,
						RoomTab[roomslot].GetName(Buffer, sizeof(Buffer)));

				if (getYesNo(query, 0))
					{
					Log3.SetInRoom(roomslot, FALSE);
					if (Log3.Save(LTab(logNo).GetLogIndex()))
						{
						label Buffer1;
#ifndef WINCIT
                        trap(T_AIDE, getmenumsg(MENU_INV, 11), who, RoomTab[roomslot].GetName(Buffer1, sizeof(Buffer1)),
								CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
                        trap(T_AIDE,  WindowCaption, getmenumsg(MENU_INV, 11), who, RoomTab[roomslot].GetName(Buffer1, sizeof(Buffer1)),
								CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

						}
					}
				}

			// see if it is us
			if (loggedIn && CurrentUser->IsSameName(who))
				{
				CurrentUser->SetInRoom(roomslot, Log3.IsInRoom(roomslot));
				}
			}

		UnlockMenu(MENU_INV);
		}
	}

void TERMWINDOWMEMBER doInvite(void)
	{
	if (!LockMenu(MENU_INV))
		{
		return;
		}

	MRO.Reverse = FALSE;

	mPrintf(getmenumsg(MENU_INV, 3));

	const int ich = iCharNE();
	switch (DoMenu(MENU_INV, ich))
		{
		case 1:
			{
			DoCommand(UC_INVITEUSERLIST);
			break;
			}

		case 2:
			{
			DoCommand(UC_INVITEROOMLIST);
			break;
			}

		case 3:
			{
			DoCommand(UC_INVITEUSER);
			break;
			}

		case -2:
			{
			oChar('?');
			showMenu(M_INVITE);
			break;
			}

		case -1:
			{
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}

	UnlockMenu(MENU_INV);
	}
