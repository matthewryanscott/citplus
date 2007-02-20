/* -------------------------------------------------------------------- */
/*	ROOM2.CPP					Citadel 								*/
/* -------------------------------------------------------------------- */
/*				Overlayed parts of the room database subsystem. 		*/
/* -------------------------------------------------------------------- */
/*	For full information about the room database subsystem, see ROOM.C. */
/* -------------------------------------------------------------------- */
#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#include "msg.h"
#include "extmsg.h"
#include "term.h"


/* -------------------------------------------------------------------- */
/*								Contents								*/
/* -------------------------------------------------------------------- */
/*								Public									*/
/*																		*/
/*	Functions:															*/
/*		FindFreeRoomSlot()												*/
/*			Find a free (unused) room slot. 							*/
/*																		*/
/*		CreateRoomSummary() 											*/
/*			Creates a summary of a specified room.						*/
/*																		*/
/*		MakeRoomName()													*/
/*			Creates a "Roomname>" type string.                          */
/*																		*/
/*		LoadCurrentRoom()												*/
/*			Loads a specified room as "current".                        */
/*																		*/
/*		DebugAuxmemRoomTab()											*/
/*			Dumps info to the user. 									*/
/*																		*/
/*		PartialRoomExists() 											*/
/*			Checks to see if a room exists, given a partial room name.	*/
/*																		*/
/* -------------------------------------------------------------------- */
/*								Private 								*/
/*	Functions:															*/
/* addRtBlock() 			adds another chunk to the rt				*/
/*																		*/
/*	Data types: 														*/
/*																		*/
/*	Data:																*/
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*	FindFreeRoomSlot()		Find a free (unused) room slot. 			*/
/*																		*/
/*	Output: 	return		A free room slot. This returns CERROR if it */
/*							cannot find a free slot.					*/
/* -------------------------------------------------------------------- */
r_slot FindFreeRoomSlot(void)
	{
	for (r_slot roomRover = 0; roomRover < cfg.maxrooms; roomRover++)
		{
		if (!RoomTab[roomRover].IsInuse())
			{
			return (roomRover);
			}
		}

	return (CERROR);
	}


/* -------------------------------------------------------------------- */
/*	CreateRoomSummary() 	Creates a summary of a specified room.		*/
/*																		*/
/*	Input:		RoomIndex	Index in room database of room to sumarize. */
/*							This room should already have been checked	*/
/*							with IsRoomInuse to verify that it used.	*/
/*																		*/
/*	Output: 	buffer		Where to put the summary.					*/
/* -------------------------------------------------------------------- */
void TERMWINDOWMEMBER CreateRoomSummary(char *buffer, r_slot RoomIndex)
	{
	assert(RoomIndex < cfg.maxrooms);
	assert(RoomIndex >= 0);
	assert(RoomTab[RoomIndex].IsInuse());


	RoomC *RoomBuffer;

	if ((RoomBuffer = new RoomC) == NULL)
		{
		sprintf(buffer, getmsg(188), getmsg(461));
		}
	else
		{
		RoomBuffer->Load(RoomIndex);

		char Buffer[80];
		sprintf(buffer, getmsg(609), cfg.Uroom_nym, RoomBuffer->GetName(Buffer, sizeof(Buffer)));

		if (RoomBuffer->IsShared())
			{
			label id;
			RoomBuffer->GetNetID(id, sizeof(id));

			if (*id)
				{
				char String[128];
				sprintf(String, getmsg(504), id);
				strcat(buffer, String);
				}
			}

		if (RoomBuffer->IsGroupOnly())
			{
			char String[128];

			if (RoomBuffer->IsBooleanGroup())
				{
				sprintf(String, getmsg(278), cfg.Lgroup_nym);
				}
			else
				{
				sprintf(String, getmsg(505), cfg.Lgroup_nym,
						GroupData[RoomBuffer->GetGroup()].GetName(Buffer, sizeof(Buffer)));
				}

			strcat(buffer, String);
			}

		if (RoomBuffer->IsPrivilegedGroup())
			{
			char String[128];

			sprintf(String, getmsg(520), cfg.Lgroup_nym, GroupData[RoomBuffer->GetPGroup()].GetName(Buffer, sizeof(Buffer)));

			strcat(buffer, String);
			}

		if (!RoomBuffer->IsPublic())
			{
			strcat(buffer, getmsg(523));
			}

		if (RoomBuffer->IsAnonymous())
			{
			strcat(buffer, getmsg(530));
			}

		if (RoomBuffer->IsBIO())
			{
			strcat(buffer, getmsg(539));
			}

		if (RoomBuffer->IsModerated())
			{
			strcat(buffer, getmsg(540));
			}

		if (RoomBuffer->IsReadOnly())
			{
			strcat(buffer, getmsg(541));
			}

		if (RoomBuffer->IsDownloadOnly())
			{
			strcat(buffer, getmsg(542));
			}

		if (RoomBuffer->IsShared())
			{
			strcat(buffer, getmsg(545));
			}

		if (RoomBuffer->IsApplication())
			{
			strcat(buffer, getmsg(568));
			}

		if (RoomBuffer->IsAutoApp())
			{
			strcat(buffer, getmsg(569));
			}

		if (RoomBuffer->IsPermanent())
			{
			char String[128];

			sprintf(String, getmsg(570), cfg.Lroom_nym);
			strcat(buffer, String);
			}

		if (CurrentUser->IsAide() && RoomBuffer->IsMsDOSdir())
			{
			char String[128];

			sprintf(String, getmsg(571), bn, RoomBuffer->GetDirectory(Buffer, sizeof(Buffer)));
			strcat(buffer, String);
			}

		if (CurrentUser->IsSysop() && RoomBuffer->IsApplication())
			{
			char String[128];

			sprintf(String, getmsg(572), bn, RoomBuffer->GetApplication(Buffer, sizeof(Buffer)));
			strcat(buffer, String);
			}

		if (CurrentUser->IsSysop() && *RoomBuffer->GetDescFile(Buffer, sizeof(Buffer)))
			{
			char String[128];

			sprintf(String, getmsg(573), bn, cfg.Uroom_nym, Buffer);
			strcat(buffer, String);
			}

		if (*RoomBuffer->GetInfoLine(Buffer, sizeof(Buffer)))
			{
			char String[256];

			sprintf(String, getmsg(574), bn, cfg.Uroom_nym, Buffer);
			strcat(buffer, String);
			}

		delete RoomBuffer;
		}
	}


/* -------------------------------------------------------------------- */
/*	MakeRoomName()			Creates a "Roomname>" type string.          */
/*																		*/
/*	Input:		RoomIndex	Index in room database of room to name. 	*/
/*							This room should already have been checked	*/
/*							with IsRoomInuse to verify that it used.	*/
/*																		*/
/*	Output: 	Buffer		A place to write the name to.				*/
/*				return		A pointer to Buffer.						*/
/* -------------------------------------------------------------------- */
char *TERMWINDOWMEMBER MakeRoomName(r_slot RoomIndex, char *Buffer)
	{
	Bool Flagged = FALSE;

	char NewFlag[2];
	NewFlag[1] = 0;

	const int flagsIndex = (CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended()) ? 49 : 50;

	assert(RoomTab[RoomIndex].IsInuse());

	if (RoomTab[RoomIndex].IsShared())
		{
		label NP;
		strcpy(Buffer, loggedIn ? CurrentUser->GetNetPrefix(NP, sizeof(NP)) : cfg.netPrefix);
		}
	else
		{
		Buffer[0] = '\0';
		}

	label RN;
	strcat(Buffer, RoomTab[RoomIndex].GetName(RN, sizeof(RN)));

	if (RoomTab[RoomIndex].IsMsDOSdir())
		{
		*NewFlag = getmsg(flagsIndex)[0];
		strcat(Buffer, NewFlag);
		Flagged = TRUE;
		}

	if (RoomTab[RoomIndex].IsGroupOnly())
		{
		*NewFlag = getmsg(flagsIndex)[1];
		strcat(Buffer, NewFlag);
		Flagged = TRUE;
		}

	if (RoomTab[RoomIndex].IsHidden())
		{
		*NewFlag = getmsg(flagsIndex)[2];
		strcat(Buffer, NewFlag);
		Flagged = TRUE;
		}

	if (RoomTab[RoomIndex].IsBIO())
		{
		*NewFlag = getmsg(flagsIndex)[3];
		strcat(Buffer, NewFlag);
		Flagged = TRUE;
		}

	if (iswindow(RoomIndex))
		{
		*NewFlag = getmsg(flagsIndex)[4];
		strcat(Buffer, NewFlag);

		if (CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended())
			{
			Flagged = TRUE;
			}
		}

	if (!Flagged)
		{
		*NewFlag = getmsg(flagsIndex)[5];
		strcat(Buffer, NewFlag);
		}

	return (Buffer);
	}


/* -------------------------------------------------------------------- */
/*	PartialRoomExists() 	Checks to see if a room exists, given a 	*/
/*							partial room name.							*/
/*																		*/
/*	Input:		SearchString	The string to look for. 				*/
/*				StartRoom		The room to start looking in.			*/
/*				IgnoreHalls 	TRUE to look for the room in any hall.	*/
/*								FALSE to look only in the current hall. */
/*																		*/
/*	Output: 	return			Index into room database of the room	*/
/*								if a match is found, else CERROR.		*/
/* -------------------------------------------------------------------- */
r_slot TERMWINDOWMEMBER PartialRoomExists(const char *SearchString, r_slot StartRoom, Bool IgnoreHalls)
	{
	assert(StartRoom < cfg.maxrooms && StartRoom >= 0);

	label da_rn, Buffer;
	CopyStringToBuffer(da_rn, deansi(SearchString));

	// if longer than a room name, it cannot be a match...
	if (strlen(da_rn) > LABELSIZE)
		{
		return (CERROR);
		}

	const r_slot rpStartRoom = GetRoomPosInTable(StartRoom);

	const int length = strlen(da_rn);
	int i;
	r_slot rps;
	for (i = 0, rps = (r_slot) (rpStartRoom + 1); i < cfg.maxrooms; ++i, ++rps)
		{
		if (rps == cfg.maxrooms)
			{
			rps = 0;
			}
		const r_slot j = RoomPosTableToRoom(rps);

		if (RoomTab[j].IsInuse() && CurrentUser->IsInRoom(j))
			{
			if (
					/* room names match */
					(strnicmp(deansi(RoomTab[j].GetName(Buffer, sizeof(Buffer))), da_rn, length) == SAMESTRING) &&

					/* and it is in the correct hall */
					(IgnoreHalls || (CurrentUser->IsUsePersonalHall() ?
							CurrentUser->IsRoomInPersonalHall(j) : roominhall(j, thisHall))) &&

					/* And the user can see the room */
					CurrentUser->CanAccessRoom(j, (h_slot) (IgnoreHalls ? CERROR : thisHall)) &&

					/* And the user hasn't excluded it */
					!CurrentUser->IsRoomExcluded(j)
					)
				{
				return (j);
				}
			}
		}

	for (i = 0, rps = (r_slot) (rpStartRoom + 1); i < cfg.maxrooms;
			++i, ++rps)
		{
		if (rps == cfg.maxrooms)
			{
			rps = 0;
			}
		const r_slot j = RoomPosTableToRoom(rps);

		if (RoomTab[j].IsInuse() && CurrentUser->IsInRoom(j))
			{
			if (
					/* If the names match */
					IsSubstr(RoomTab[j].GetName(Buffer, sizeof(Buffer)), da_rn) &&

					/* and it is in the correct hall */
					(IgnoreHalls || (CurrentUser->IsUsePersonalHall() ?
							CurrentUser->IsRoomInPersonalHall(j) : roominhall(j, thisHall))) &&

					/* And the user can see the room */
					CurrentUser->CanAccessRoom(j, (h_slot) (IgnoreHalls ? CERROR : thisHall)) &&

					/* And the user hasn't excluded it */
					!CurrentUser->IsRoomExcluded(j)
					)
				{
				return (j);
				}
			}
		}

	for (i = 0, rps = (r_slot) (rpStartRoom + 1); i < cfg.maxrooms;
			++i, ++rps)
		{
		if (rps == cfg.maxrooms)
			{
			rps = 0;
			}
		const r_slot j = RoomPosTableToRoom(rps);

		if (RoomTab[j].IsInuse() && CurrentUser->IsInRoom(j))
			{
			if (
					/* room names match */
					(strnicmp(deansi(RoomTab[j].GetName(Buffer, sizeof(Buffer))), da_rn, length) == SAMESTRING) &&

					/* and it is in the correct hall */
					(IgnoreHalls || (CurrentUser->IsUsePersonalHall() ?
							CurrentUser->IsRoomInPersonalHall(j) : roominhall(j, thisHall))) &&

					/* And the user can see the room */
					CurrentUser->CanAccessRoom(j, (h_slot) (IgnoreHalls ? CERROR : thisHall)))
				{
				return (j);
				}
			}
		}

	for (i = 0, rps = (r_slot) (rpStartRoom + 1); i < cfg.maxrooms;
			++i, ++rps)
		{
		if (rps == cfg.maxrooms)
			{
			rps = 0;
			}
		const r_slot j = RoomPosTableToRoom(rps);

		if (RoomTab[j].IsInuse() && CurrentUser->IsInRoom(j))
			{
			if (
					/* If the names match */
					IsSubstr(RoomTab[j].GetName(Buffer, sizeof(Buffer)), da_rn) &&

					/* and it is in the correct hall */
					(IgnoreHalls || (CurrentUser->IsUsePersonalHall() ?
							CurrentUser->IsRoomInPersonalHall(j) : roominhall(j, thisHall))) &&

					/* And the user can see the room */
					CurrentUser->CanAccessRoom(j, (h_slot) (IgnoreHalls ? CERROR : thisHall)))
				{
				return (j);
				}
			}
		}

	return (CERROR);
	}


// closes invalid directory rooms
void TERMWINDOWMEMBER checkdir(void)
	{
	if (CurrentRoom->IsMsDOSdir())
		{
		char Dir[64];

		if (!changedir(CurrentRoom->GetDirectory(Dir, sizeof(Dir))))
			{
			CurrentRoom->SetMsDOSdir(FALSE);
			CurrentRoom->SetDownloadOnly(FALSE);
			CurrentRoom->SetUploadOnly(FALSE);

			CurrentRoom->Save(thisRoom);

			Message *Msg = new Message;

			if (Msg)
				{
				label Buffer;

				Msg->SetTextWithFormat(getmsg(66), CurrentRoom->GetName(Buffer, sizeof(Buffer)), Dir);

				if (CurrentRoom->IsGroupOnly())
					{
					if (CurrentRoom->IsBooleanGroup())
						{
						Msg->SetGroup(SPECIALSECURITY);
						}
					else
						{
						Msg->SetGroup(CurrentRoom->GetGroup());
						}
					}

				Msg->SetRoomNumber(AIDEROOM);

				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(100);
				}
			}
		}
	}
