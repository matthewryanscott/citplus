// --------------------------------------------------------------------------
// Citadel: ScrFRoom.CPP
//
// Internal script functions dealing with rooms.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "room.h"
#include "tallybuf.h"
#include "hall.h"


// --------------------------------------------------------------------------
// ROOM TABLE STUFF

void sfIsroominuse(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsInuse());
	}

void sfIsroompublic(strList *params)
	{
	SETBOOL(!RoomTab[evaluateInt(params->string)].IsHidden());
	}

void sfIsroomdir(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsMsDOSdir());
	}

void sfIsroomperm(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsPermanent());
	}

void sfIsroomgrponly(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsGroupOnly());
	}

void sfIsroomreadonly(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsReadOnly());
	}

void sfIsroomdownonly(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsDownloadOnly());
	}

void sfIsroomshared(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsShared());
	}

void sfIsroommoderated(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsModerated());
	}

void sfIsroomapplic(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsApplication());
	}

void sfIsroombio(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsBIO());
	}

void sfIsroomuponly(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsUploadOnly());
	}

void sfIsroomprivgrp(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsPrivilegedGroup());
	}

void sfIsroomanon(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsAnonymous());
	}

void sfIsroomsubject(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsSubject());
	}

void sfIsroomgrpmod(strList *params)
	{
	SETBOOL(RoomTab[evaluateInt(params->string)].IsGroupModerates());
	}

void sfGetroomname(strList *params)
	{
	RoomTab[evaluateInt(params->string)].GetName(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetnetid(strList *params)
	{
	RoomTab[evaluateInt(params->string)].GetNetID(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetroomgroup(strList *params)
	{
	SETINT(RoomTab[evaluateInt(params->string)].GetGroupNumber());
	}

void sfGetroomtotal(strList *params)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()Talley->TotalMessagesInRoom(evaluateInt(params->string)));
	}

void sfGetroommsgs(strList *params)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()Talley->MessagesInRoom(evaluateInt(params->string)));
	}

void sfGetroomnew(strList *params)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()Talley->NewInRoom(evaluateInt(params->string)));
	}

void sfGetroomentered(strList *params)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()Talley->EnteredInRoom(evaluateInt(params->string)));
	}

void sfGetroomhasmail(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()Talley->HasNewMail(evaluateInt(params->string)));
	}

void sfGetroomhasoldmail(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()Talley->HasOldMail(evaluateInt(params->string)));
	}

void sfGetroomvisited(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()Talley->Visited(evaluateInt(params->string)));
	}

void sfGetroombypass(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()Talley->Bypassed(evaluateInt(params->string)));
	}

void sfFindRoomByName(strList *params)
	{
	RequiresScriptTW();

	r_slot slot;

	if (evaluateBool(params->next->string))
		{
		slot = ScriptTW()PartialRoomExists(evaluateString(params->string),
				ScriptTW()thisRoom, TRUE);
		}
	else
		{
		slot = RoomExists(evaluateString(params->string));
		}

	SETINT(slot);
	}

void sfGetroomdir(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentRoom->GetDirectory(pCurScript->wowza, sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfKillRoom(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()killroom(params ? evaluateString(params->string) : NULL));
	}

void sfFindRoomByNetID(strList *params)
	{
	SETINT(IdExists(evaluateString(params->string),
			evaluateInt(params->next->string)));
	}

void sfIsroomwindow(strList *params)
	{
	SETBOOL(iswindow(evaluateInt(params->string)));
	}

//////////////////////////// ROOM FILE STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void sfRoomUseCurrentRoom(strList *)
	{
	RequiresScriptTW();

	if (!pCurScript->UseCR)
		{
		delete pCurScript->UsrRoom;
		}

	pCurScript->UseCR = TRUE;
	pCurScript->UsrRoom = ScriptTW()CurrentRoom;
	}

void sfRoomStart(strList *)
	{
	if (!pCurScript->UseCR)
		{
		delete pCurScript->UsrRoom;
		}

	pCurScript->UsrRoom = new RoomC;
	pCurScript->UseCR = FALSE;

	SETBOOL(pCurScript->UsrRoom != NULL);
	}

void sfRoomClear(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->Clear();
		}
	}

void sfRoomDump(strList *)
	{
	if (!pCurScript->UseCR)
		{
		delete pCurScript->UsrRoom;
		}

	pCurScript->UsrRoom = NULL;
	pCurScript->UseCR = FALSE;
	}

void sfRoomSave(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->Save(evaluateInt(params->string)));
		}
	}

void sfRoomLoad(strList *params)
	{
	RequiresScriptTW();

	sfRoomDump(NULL);

	const r_slot Index = evaluateInt(params->string);

	if (Index == ScriptTW()thisRoom)
		{
		sfRoomUseCurrentRoom(NULL);
		SETBOOL(TRUE);
		}
	else
		{
		sfRoomStart(NULL);

		if (pCurScript->UsrRoom)
			{
			SETBOOL(pCurScript->UsrRoom->Load(Index));
			}
		// else... sfRoomStart() did the SETBOOL() for us.
		}
	}


void sfRoomIsInuse(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsInuse());
		}
	}


void sfRoomSetInuse(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetInuse(evaluateBool(params->string));
		}
	}


void sfRoomIsPublic(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsPublic());
		}
	}


void sfRoomSetPublic(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetPublic(evaluateBool(params->string));
		}
	}


void sfRoomIsMsDOSdir(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsMsDOSdir());
		}
	}


void sfRoomSetMsDOSdir(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetMsDOSdir(evaluateBool(params->string));
		}
	}


void sfRoomIsPermanent(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsPermanent());
		}
	}


void sfRoomSetPermanent(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetPermanent(evaluateBool(params->string));
		}
	}


void sfRoomIsGroupOnly(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsGroupOnly());
		}
	}


void sfRoomSetGroupOnly(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetGroupOnly(evaluateBool(params->string));
		}
	}


void sfRoomIsReadOnly(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsReadOnly());
		}
	}


void sfRoomSetReadOnly(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetReadOnly(evaluateBool(params->string));
		}
	}


void sfRoomIsDownloadOnly(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsDownloadOnly());
		}
	}


void sfRoomSetDownloadOnly(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetDownloadOnly(evaluateBool(params->string));
		}
	}


void sfRoomIsShared(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsShared());
		}
	}


void sfRoomSetShared(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetShared(evaluateBool(params->string));
		}
	}


void sfRoomIsModerated(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsModerated());
		}
	}


void sfRoomSetModerated(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetModerated(evaluateBool(params->string));
		}
	}


void sfRoomIsApplication(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsApplication());
		}
	}


void sfRoomSetApplication(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetApplication(evaluateBool(params->string));
		}
	}


void sfRoomIsBIO(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsBIO());
		}
	}


void sfRoomSetBIO(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetBIO(evaluateBool(params->string));
		}
	}


void sfRoomIsUploadOnly (strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsUploadOnly ());
		}
	}


void sfRoomSetUploadOnly (strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetUploadOnly (evaluateBool(params->string));
		}
	}


void sfRoomIsPrivilegedGroup(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsPrivilegedGroup());
		}
	}


void sfRoomSetPrivilegedGroup(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetPrivilegedGroup(evaluateBool(params->string));
		}
	}


void sfRoomIsAnonymous(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsAnonymous());
		}
	}


void sfRoomSetAnonymous(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetAnonymous(evaluateBool(params->string));
		}
	}


void sfRoomIsSubject(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsSubject());
		}
	}


void sfRoomSetSubject(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetSubject(evaluateBool(params->string));
		}
	}


void sfRoomIsGroupModerates(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsGroupModerates());
		}
	}


void sfRoomSetGroupModerates(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetGroupModerates(evaluateBool(params->string));
		}
	}


void sfRoomIsArchive(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsArchive());
		}
	}


void sfRoomSetArchive(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetArchive(evaluateBool(params->string));
		}
	}


void sfRoomIsBooleanGroup(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsBooleanGroup());
		}
	}


void sfRoomSetBooleanGroup(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetBooleanGroup(evaluateBool(params->string));
		}
	}


void sfRoomSetName(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetName(evaluateString(params->string));
		}
	}


void sfRoomGetName(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetName(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetDirectory(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetDirectory(evaluateString(params->string));
		}
	}


void sfRoomGetDirectory(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetDirectory(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomChangeDir(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->ChangeDir());
		}
	}

void sfRoomSetDescFile(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetDescFile(evaluateString(params->string));
		}
	}


void sfRoomGetDescFile(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetDescFile(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetApplicationName(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetApplication(evaluateString(params->string));
		}
	}


void sfRoomGetApplicationName(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetApplication(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetGroup(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetGroup(evaluateInt(params->string));
		}
	}


void sfRoomGetGroup(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETINT(pCurScript->UsrRoom->GetGroup());
		}
	}


void sfRoomSetPGroup(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetPGroup(evaluateInt(params->string));
		}
	}


void sfRoomGetPGroup(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETINT(pCurScript->UsrRoom->GetPGroup());
		}
	}


void sfRoomSetAutoApp(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetAutoApp(evaluateBool(params->string));
		}
	}


void sfRoomIsAutoApp(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsAutoApp());
		}
	}


void sfRoomSetUnexcludable(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetUnexcludable(evaluateBool(params->string));
		}
	}


void sfRoomIsUnexcludable(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		SETBOOL(pCurScript->UsrRoom->IsUnexcludable());
		}
	}


void sfRoomSetInfoLine(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetInfoLine(evaluateString(params->string));
		}
	}


void sfRoomGetInfoLine(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetInfoLine(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetNetID(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetNetID(evaluateString(params->string));
		}
	}


void sfRoomGetNetID(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetNetID(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetArchiveFile(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetArchiveFile(evaluateString(params->string));
		}
	}


void sfRoomGetArchiveFile(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetArchiveFile(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetDictionary(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetDictionary(evaluateString(params->string));
		}
	}


void sfRoomGetDictionary(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetDictionary(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomGetCreator(strList *)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->GetCreator(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}


void sfRoomSetCreator(strList *params)
	{
	if (pCurScript->UsrRoom)
		{
		pCurScript->UsrRoom->SetCreator(evaluateString(params->string));
		}
	}
