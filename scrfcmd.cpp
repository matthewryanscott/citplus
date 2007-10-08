// --------------------------------------------------------------------------
// Citadel: ScrFCmd.CPP
//
// Script Functions for user Commands

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"

void sfCmdAideAttributes(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideAttributes();
	}

void sfCmdAideEditRoom(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideEditRoom();
	}

void sfCmdAideFileInfoSet(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideFileInfoSet();
	}

void sfCmdAideGroup(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideGroup();
	}

void sfCmdAideHall(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideHall();
	}

void sfCmdAideInsert(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideInsert();
	}

void sfCmdAideJumpToAideAndMaintenance(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideJumpToAideAndMaintenance();
	}

void sfCmdAideKillRoom(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideKillRoom();
	}

void sfCmdAideListGroup(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideListGroup();
	}

void sfCmdAideMoveFile(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideMoveFile();
	}

void sfCmdAideNameMessages(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideNameMessages();
	}

void sfCmdAideQueueAutomark(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueAutomark();
	}

void sfCmdAideQueueClear(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueClear();
	}

void sfCmdAideQueueInsert(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueInsert();
	}

void sfCmdAideQueueKill(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueKill();
	}

void sfCmdAideQueueList(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueList();
	}

void sfCmdAideQueueMove(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueMove();
	}

void sfCmdAideQueueRoomMark(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueRoomMark();
	}

void sfCmdAideQueueSort(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideQueueSort();
	}

void sfCmdAideRenameFile(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideRenameFile();
	}

void sfCmdAideSetFileInfo(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideSetFileInfo();
	}

void sfCmdAideUnlink(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideUnlink();
	}

void sfCmdAideWindow(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideWindow();
	}

void sfCmdAideMoveRoom(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()AideMoveRoom(evaluateInt(params->string),
			evaluateBool(params->next->string));
	}

void sfCmdAideCopyMessage(strList *)
	{
	RequiresScriptTW();
	ScriptTW()AideCopyMessage();
	}

void sfCmdEnterApplication(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterApplication();
	}

void sfCmdEnterBorder(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterBorder();
	}

void sfCmdEnterConfiguration(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()EnterConfiguration(evaluateBool(params->string));
	}

void sfCmdEnterDoor(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterDoor();
	}

void sfCmdEnterHall(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterHall();
	}

void sfCmdEnterName(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterName();
	}

void sfCmdEnterPassword(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterPassword();
	}

void sfCmdEnterRoom(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()EnterRoom(evaluateBool(params->string));
	}

void sfCmdEnterTitleSurname(strList *)
	{
	RequiresScriptTW();
	ScriptTW()EnterTitleSurname();
	}

void sfCmdEnterTextfile(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()EnterTextfile(evaluateBool(params->string));
	}

void sfCmdEnterWithProtocol(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()EnterWithProtocol(evaluateBool(params->string));
	}

void sfCmdExcludeRoom(strList *)
	{
	RequiresScriptTW();
	ScriptTW()exclude();
	}

void sfCmdInviteUserList(strList *)
	{
	RequiresScriptTW();
	ScriptTW()InviteUserList();
	}

void sfCmdInviteRoomList(strList *)
	{
	RequiresScriptTW();
	ScriptTW()InviteRoomList();
	}

void sfCmdInviteUser(strList *)
	{
	RequiresScriptTW();
	ScriptTW()InviteUser();
	}

void sfCmdIntro(strList *)
	{
	RequiresScriptTW();
	ScriptTW()doIntro();
	}

void sfCmdTerminateQuitAlso(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()TerminateQuitAlso(evaluateBool(params->string));
	}

void sfCmdTerminateStay(strList *)
	{
	RequiresScriptTW();
	ScriptTW()TerminateStay();
	}

void sfCmdExpertToggle(strList *)
	{
	RequiresScriptTW();
	ScriptTW()doXpert();
	}

void sfCmdVerboseToggle(strList *)
	{
	RequiresScriptTW();
	ScriptTW()doverbose();
	}

void sfCmdY(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()doY(evaluateBool(params->string));
	}

void sfCmdFingerDelete(strList *)
	{
	RequiresScriptTW();
	ScriptTW()FingerDelete();
	}

void sfCmdFingerEdit(strList *)
	{
	RequiresScriptTW();
	ScriptTW()FingerEdit();
	}

void sfCmdFingerList(strList *)
	{
	RequiresScriptTW();
    ScriptTW()FingerList(TRUE);
	}

void sfCmdFingerUser(strList *)
	{
	RequiresScriptTW();
	ScriptTW()FingerUser();
	}
