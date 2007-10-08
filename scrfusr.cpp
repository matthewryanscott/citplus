// --------------------------------------------------------------------------
// Citadel: ScrFUsr.CPP
//
// User... internal script functions.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "log.h"

// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ USER STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
void sfUserUseCurrentUser(strList *)
	{
	RequiresScriptTW();

	if (!pCurScript->UseCU)
		{
		delete pCurScript->UsrUsr;
		}

	pCurScript->UseCU = TRUE;
	pCurScript->UsrUsr = ScriptTW()CurrentUser;
	}

void sfUserStart(strList *)
	{
	if (!pCurScript->UseCU)
		{
		delete pCurScript->UsrUsr;
		}

	pCurScript->UsrUsr = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
	pCurScript->UseCU = FALSE;

	SETBOOL(pCurScript->UsrUsr != NULL);
	}

void sfUserClear(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->Clear();
		}
	}

void sfUserDump(strList *)
	{
	if (!pCurScript->UseCU)
		{
		delete pCurScript->UsrUsr;
		}

	pCurScript->UsrUsr = NULL;
	pCurScript->UseCU = FALSE;
	}

void sfUserSave(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->Save(evaluateInt(params->string), pCurScript->UsrUsr->GetMessageRoom()));
		}
	}

void sfUserLoad(strList *params)
	{
	RequiresScriptTW();

	sfUserDump(NULL);

	const l_index Index = evaluateInt(params->string);

	if (ScriptTW()loggedIn && (Index == ScriptTW()ThisLog))
		{
		sfUserUseCurrentUser(NULL);
		SETBOOL(TRUE);
		}
	else
		{
		sfUserStart(NULL);

		if (pCurScript->UsrUsr)
			{
			SETBOOL(pCurScript->UsrUsr->Load(Index));
			}
		// else... sfUserStart() did the SETBOOL() for us.
		}
	}

void sfUserIsInuse(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsInuse());
		}
	}

void sfUserSetInuse(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetInuse(evaluateBool(params->string));
		}
	}

void sfUserIsUpperOnly(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUpperOnly());
		}
	}

void sfUserSetUpperOnly(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUpperOnly(evaluateBool(params->string));
		}
	}

void sfUserIsLinefeeds(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsLinefeeds());
		}
	}

void sfUserSetLinefeeds(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLinefeeds(evaluateBool(params->string));
		}
	}

void sfUserIsExpert(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsExpert());
		}
	}

void sfUserSetExpert(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetExpert(evaluateBool(params->string));
		}
	}

void sfUserIsAide(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsAide());
		}
	}

void sfUserSetAide(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetAide(evaluateBool(params->string));
		}
	}

void sfUserIsTabs(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsTabs());
		}
	}

void sfUserSetTabs(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetTabs(evaluateBool(params->string));
		}
	}

void sfUserIsOldToo(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsOldToo());
		}
	}

void sfUserSetOldToo(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetOldToo(evaluateBool(params->string));
		}
	}

void sfUserIsProblem(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsProblem());
		}
	}

void sfUserSetProblem(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetProblem(evaluateBool(params->string));
		}
	}

void sfUserIsUnlisted(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUnlisted());
		}
	}

void sfUserSetUnlisted(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUnlisted(evaluateBool(params->string));
		}
	}

void sfUserIsPermanent(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsPermanent());
		}
	}

void sfUserSetPermanent(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPermanent(evaluateBool(params->string));
		}
	}

void sfUserIsSysop(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsSysop());
		}
	}

void sfUserSetSysop(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSysop(evaluateBool(params->string));
		}
	}

void sfUserIsNode(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsNode());
		}
	}

void sfUserSetNode(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetNode(evaluateBool(params->string));
		}
	}

void sfUserIsNetUser(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsNetUser());
		}
	}

void sfUserSetNetUser(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetNetUser(evaluateBool(params->string));
		}
	}

void sfUserIsAccounting(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsAccounting());
		}
	}

void sfUserSetAccounting(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetAccounting(evaluateBool(params->string));
		}
	}

void sfUserIsMail(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsMail());
		}
	}

void sfUserSetMail(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMail(evaluateBool(params->string));
		}
	}

void sfUserIsViewRoomDesc(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewRoomDesc());
		}
	}

void sfUserSetViewRoomDesc(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewRoomDesc(evaluateBool(params->string));
		}
	}

void getnumkill(KillFileE KF)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetKillCount(KF));
		}
	}

void getkill(KillFileE KF, strList *params)
	{
	SETSTR(ns);

	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetKillNum(KF, evaluateInt(params->string), pCurScript->wowza, sizeof(pCurScript->wowza));
		}
	}

void addkill(KillFileE KF, strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->AddKill(KF, evaluateString(params->string)));
		}
	}

void removekill(KillFileE KF, strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->RemoveKill(KF, evaluateString(params->string)));
		}
	}

void iskill(KillFileE KF, strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsKill(KF, evaluateString(params->string)));
		}
	}

void sfUserGetNumKillUser(strList *)
	{
	getnumkill(KF_USER);
	}

void sfUserGetKillUser(strList *params)
	{
	getkill(KF_USER, params);
	}

void sfUserAddKillUser(strList *params)
	{
	addkill(KF_USER, params);
	}

void sfUserRemoveKillUser(strList *params)
	{
	removekill(KF_USER, params);
	}

void sfUserIsKillUser(strList *params)
	{
	iskill(KF_USER, params);
	}

void sfUserGetNumKillNode(strList *)
	{
	getnumkill(KF_NODE);
	}

void sfUserGetKillNode(strList *params)
	{
	getkill(KF_NODE, params);
	}

void sfUserAddKillNode(strList *params)
	{
	addkill(KF_NODE, params);
	}

void sfUserRemoveKillNode(strList *params)
	{
	removekill(KF_NODE, params);
	}

void sfUserIsKillNode(strList *params)
	{
	iskill(KF_NODE, params);
	}

void sfUserGetNumKillText(strList *)
	{
	getnumkill(KF_TEXT);
	}

void sfUserGetKillText(strList *params)
	{
	getkill(KF_TEXT, params);
	}

void sfUserAddKillText(strList *params)
	{
	addkill(KF_TEXT, params);
	}

void sfUserRemoveKillText(strList *params)
	{
	removekill(KF_TEXT, params);
	}

void sfUserIsKillText(strList *params)
	{
	iskill(KF_TEXT, params);
	}

void sfUserGetNumKillRegion(strList *)
	{
	getnumkill(KF_REGION);
	}

void sfUserGetKillRegion(strList *params)
	{
	getkill(KF_REGION, params);
	}

void sfUserAddKillRegion(strList *params)
	{
	addkill(KF_REGION, params);
	}

void sfUserRemoveKillRegion(strList *params)
	{
	removekill(KF_REGION, params);
	}

void sfUserIsKillRegion(strList *params)
	{
	iskill(KF_REGION, params);
	}

void sfUserIsWordInDictionary(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsWordInDictionary(evaluateString(params->string)));
		}
	}

void sfUserAddWordToDictionary(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->AddWordToDictionary(evaluateString(params->string)));
		}
	}

void sfUserRemoveWordFromDictionary(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->RemoveWordFromDictionary(evaluateString(params->string)));
		}
	}

void sfUserGetName(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetName(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetName(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetName(evaluateString(params->string));
		}
	}

void sfUserGetInitials(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetInitials(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetInitials(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetInitials(evaluateString(params->string));
		}
	}

void sfUserGetPassword(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetPassword(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetPassword(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPassword(evaluateString(params->string));
		}
	}

void sfUserGetSurname(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetSurname(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetSurname(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSurname(evaluateString(params->string));
		}
	}

void sfUserGetTitle(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetTitle(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetTitle(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetTitle(evaluateString(params->string));
		}
	}

void sfUserGetRealName(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetRealName(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetRealName(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRealName(evaluateString(params->string));
		}
	}

void sfUserGetPhoneNumber(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetPhoneNumber(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetPhoneNumber(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPhoneNumber(evaluateString(params->string));
		}
	}

void sfUserGetForwardAddr(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetForwardAddr(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetForwardAddr(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetForwardAddr(evaluateString(params->string));
		}
	}

void sfUserGetForwardAddrNode(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetForwardAddrNode(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetForwardAddrNode(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetForwardAddrNode(evaluateString(params->string));
		}
	}

void sfUserGetForwardAddrRegion(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetForwardAddrRegion(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetForwardAddrRegion(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetForwardAddrRegion(evaluateString(params->string));
		}
	}

void sfUserGetPromptFormat(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetPromptFormat(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetPromptFormat(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPromptFormat(evaluateString(params->string));
		}
	}

void sfUserGetDateStamp(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetDateStamp(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetDateStamp(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDateStamp(evaluateString(params->string));
		}
	}

void sfUserGetVerboseDateStamp(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetVerboseDateStamp(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetVerboseDateStamp(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetVerboseDateStamp(evaluateString(params->string));
		}
	}

void sfUserGetSignature(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetSignature(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetSignature(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSignature(evaluateString(params->string));
		}
	}

void sfUserGetNetPrefix(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetNetPrefix(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetNetPrefix(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetNetPrefix(evaluateString(params->string));
		}
	}

void sfUserGetMailAddr1(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetMailAddr1(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetMailAddr1(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMailAddr1(evaluateString(params->string));
		}
	}

void sfUserGetMailAddr2(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetMailAddr2(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetMailAddr2(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMailAddr2(evaluateString(params->string));
		}
	}

void sfUserGetMailAddr3(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetMailAddr3(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetMailAddr3(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMailAddr3(evaluateString(params->string));
		}
	}

void sfUserGetAlias(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetAlias(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetAlias(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetAlias(evaluateString(params->string));
		}
	}

void sfUserGetLocID(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetLocID(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetLocID(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLocID(evaluateString(params->string));
		}
	}

void sfUserGetMorePrompt(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetMorePrompt(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetMorePrompt(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMorePrompt(evaluateString(params->string));
		}
	}

void sfUserGetOccupation(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetOccupation(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetOccupation(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetOccupation(evaluateString(params->string));
		}
	}

void sfUserGetWhereHear(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetWhereHear(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetWhereHear(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetWhereHear(evaluateString(params->string));
		}
	}

void sfUserGetLastRoom(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetLastRoom(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetLastRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLastRoom(evaluateString(params->string));
		}
	}

void sfUserGetLastHall(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetLastHall(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetLastHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLastHall(evaluateString(params->string));
		}
	}

void sfUserGetDefaultRoom(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetDefaultRoom(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetDefaultRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDefaultRoom(evaluateString(params->string));
		}
	}

void sfUserGetDefaultHall(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetDefaultHall(pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetDefaultHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDefaultHall(evaluateString(params->string));
		}
	}

void sfUserGetBirthDate(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetBirthDate());
		}
	}

void sfUserSetBirthDate(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetBirthDate(evaluateLong(params->string));
		}
	}

void sfUserGetFirstOn(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetFirstOn());
		}
	}

void sfUserSetFirstOn(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetFirstOn(evaluateLong(params->string));
		}
	}

void sfUserGetSex(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetSex());
		}
	}

void sfUserSetSex(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		uint Sex = evaluateInt(params->string);

		// Make unknown sexes unknown.
		if (Sex > 2)
			{
			Sex = 0;
			}

		pCurScript->UsrUsr->SetSex((SexE) Sex);
		}
	}

void sfUserGetNulls(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetNulls());
		}
	}

void sfUserSetNulls(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetNulls(evaluateInt(params->string));
		}
	}

void sfUserGetWidth(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetWidth());
		}
	}

void sfUserSetWidth(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetWidth(evaluateInt(params->string));
		}
	}

void sfUserGetLinesPerScreen(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetLinesPerScreen());
		}
	}

void sfUserSetLinesPerScreen(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLinesPerScreen(evaluateInt(params->string));
		}
	}

void sfUserGetNumUserShow(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetNumUserShow());
		}
	}

void sfUserSetNumUserShow(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetNumUserShow(evaluateInt(params->string));
		}
	}

void sfUserGetDefaultProtocol(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		char Prot[2];

		Prot[0] = pCurScript->UsrUsr->GetDefaultProtocol();
		Prot[1] = 0;

		SETSTR(Prot);
		}
	}

void sfUserSetDefaultProtocol(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDefaultProtocol(evaluateString(params->string)[0]);
		}
	}

void sfUserGetCallTime(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetCallTime());
		}
	}

void sfUserSetCallTime(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCallTime(evaluateLong(params->string));
		}
	}

void sfUserGetCallNumber(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetCallNumber());
		}
	}

void sfUserSetCallNumber(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCallNumber(evaluateUlong(params->string));
		}
	}

void sfUserGetTotalTime(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetTotalTime());
		}
	}

void sfUserSetTotalTime(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetTotalTime(evaluateLong(params->string));
		}
	}

void sfUserGetCredits(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetCredits());
		}
	}

void sfUserSetCredits(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCredits(evaluateLong(params->string));
		}
	}

void sfUserGetLogins(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetLogins());
		}
	}

void sfUserSetLogins(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLogins(evaluateLong(params->string));
		}
	}

void sfUserGetPosted(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetPosted());
		}
	}

void sfUserSetPosted(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPosted(evaluateLong(params->string));
		}
	}

void sfUserGetRead(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetRead());
		}
	}

void sfUserSetRead(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRead(evaluateLong(params->string));
		}
	}

void sfUserGetPasswordChangeTime(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetPasswordChangeTime());
		}
	}

void sfUserSetPasswordChangeTime(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPasswordChangeTime(evaluateLong(params->string));
		}
	}

void sfUserGetCallsToday(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetCallsToday());
		}
	}

void sfUserSetCallsToday(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCallsToday(evaluateInt(params->string));
		}
	}

void sfUserGetCallLimit(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetCallLimit());
		}
	}

void sfUserSetCallLimit(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCallLimit(evaluateInt(params->string));
		}
	}

void sfUserGetLastMessage(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetLastMessage());
		}
	}

void sfUserSetLastMessage(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetLastMessage(evaluateUlong(params->string));
		}
	}

void sfUserGetDL_Bytes(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetDL_Bytes());
		}
	}

void sfUserSetDL_Bytes(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDL_Bytes(evaluateUlong(params->string));
		}
	}

void sfUserGetDL_Num(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetDL_Num());
		}
	}

void sfUserSetDL_Num(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDL_Num(evaluateUint(params->string));
		}
	}

void sfUserGetUL_Bytes(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetUL_Bytes());
		}
	}

void sfUserSetUL_Bytes(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUL_Bytes(evaluateUlong(params->string));
		}
	}

void sfUserGetUL_Num(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetUL_Num());
		}
	}

void sfUserSetUL_Num(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUL_Num(evaluateUint(params->string));
		}
	}

void sfUserGetPoopcount(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetPoopcount());
		}
	}

void sfUserSetPoopcount(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPoopcount(evaluateLong(params->string));
		}
	}

void sfUserIsDungeoned(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsDungeoned());
		}
	}

void sfUserSetDungeoned(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDungeoned(evaluateBool(params->string));
		}
	}

void sfUserIsForwardToNode(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsForwardToNode());
		}
	}

void sfUserSetForwardToNode(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetForwardToNode(evaluateBool(params->string));
		}
	}

void sfUserIsAutoNextHall(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsAutoNextHall());
		}
	}

void sfUserSetAutoNextHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetAutoNextHall(evaluateBool(params->string));
		}
	}

void sfUserIsEnterBorders(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsEnterBorders());
		}
	}

void sfUserSetEnterBorders(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetEnterBorders(evaluateBool(params->string));
		}
	}

void sfUserIsVerified(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsVerified());
		}
	}

void sfUserSetVerified(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetVerified(evaluateBool(params->string));
		}
	}

void sfUserIsSurnameLocked(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsSurnameLocked());
		}
	}

void sfUserSetSurnameLocked(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSurnameLocked(evaluateBool(params->string));
		}
	}

void sfUserIsDefaultHallLocked(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsDefaultHallLocked());
		}
	}

void sfUserSetDefaultHallLocked(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDefaultHallLocked(evaluateBool(params->string));
		}
	}

void sfUserIsPsycho(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsPsycho());
		}
	}

void sfUserSetPsycho(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPsycho(evaluateBool(params->string));
		}
	}

void sfUserIsViewTitleSurname(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewTitleSurname());
		}
	}

void sfUserSetViewTitleSurname(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewTitleSurname(evaluateBool(params->string));
		}
	}

void sfUserIsViewSubjects(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewSubjects());
		}
	}

void sfUserSetViewSubjects(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewSubjects(evaluateBool(params->string));
		}
	}

void sfUserIsViewSignatures(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewSignatures());
		}
	}

void sfUserSetViewSignatures(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewSignatures(evaluateBool(params->string));
		}
	}

void sfUserIsTwirly(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsTwirly());
		}
	}

void sfUserSetTwirly(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetTwirly(evaluateBool(params->string));
		}
	}

void sfUserIsAutoVerbose(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsAutoVerbose());
		}
	}

void sfUserSetAutoVerbose(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetAutoVerbose(evaluateBool(params->string));
		}
	}

void sfUserIsPauseBetweenMessages(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsPauseBetweenMessages());
		}
	}

void sfUserSetPauseBetweenMessages(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPauseBetweenMessages(evaluateBool(params->string));
		}
	}

void sfUserIsMinibin(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsMinibin());
		}
	}

void sfUserSetMinibin(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMinibin(evaluateBool(params->string));
		}
	}

void sfUserIsClearScreenBetweenMessages(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsClearScreenBetweenMessages());
		}
	}

void sfUserSetClearScreenBetweenMessages(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetClearScreenBetweenMessages(evaluateBool(params->string));
		}
	}

void sfUserIsViewRoomInfoLines(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewRoomInfoLines());
		}
	}

void sfUserSetViewRoomInfoLines(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewRoomInfoLines(evaluateBool(params->string));
		}
	}

void sfUserIsViewHallDescription(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewHallDescription());
		}
	}

void sfUserSetViewHallDescription(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewHallDescription(evaluateBool(params->string));
		}
	}

void sfUserIsViewCensoredMessages(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewCensoredMessages());
		}
	}

void sfUserSetViewCensoredMessages(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewCensoredMessages(evaluateBool(params->string));
		}
	}

void sfUserIsViewBorders(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewBorders());
		}
	}

void sfUserSetViewBorders(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewBorders(evaluateBool(params->string));
		}
	}

void sfUserIsOut300(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsOut300());
		}
	}

void sfUserSetOut300(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetOut300(evaluateBool(params->string));
		}
	}

void sfUserIsUserSignatureLocked(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUserSignatureLocked());
		}
	}

void sfUserSetUserSignatureLocked(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUserSignatureLocked(evaluateBool(params->string));
		}
	}

void sfUserIsHideMessageExclusions(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsHideMessageExclusions());
		}
	}

void sfUserSetHideMessageExclusions(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetHideMessageExclusions(evaluateBool(params->string));
		}
	}

void sfUserIsDownload(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsDownload());
		}
	}

void sfUserSetDownload(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetDownload(evaluateBool(params->string));
		}
	}

void sfUserIsUpload(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUpload());
		}
	}

void sfUserSetUpload(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUpload(evaluateBool(params->string));
		}
	}

void sfUserIsChat(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsChat());
		}
	}

void sfUserSetChat(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetChat(evaluateBool(params->string));
		}
	}

void sfUserIsPrintFile(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsPrintFile());
		}
	}

void sfUserSetPrintFile(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPrintFile(evaluateBool(params->string));
		}
	}

void sfUserGetSpellCheckMode(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETLONG(pCurScript->UsrUsr->GetSpellCheckMode());
		}
	}

void sfUserSetSpellCheckMode(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSpellCheckMode(evaluateInt(params->string));
		}
	}

void sfUserIsMakeRoom(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsMakeRoom());
		}
	}

void sfUserSetMakeRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMakeRoom(evaluateBool(params->string));
		}
	}

void sfUserIsVerboseLogOut(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsVerboseLogOut());
		}
	}

void sfUserSetVerboseLogOut(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetVerboseLogOut(evaluateBool(params->string));
		}
	}

void sfUserIsConfirmNoEO(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsConfirmNoEO());
		}
	}

void sfUserSetConfirmNoEO(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetConfirmNoEO(evaluateBool(params->string));
		}
	}

void sfUserIsUsePersonalHall(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUsePersonalHall());
		}
	}

void sfUserSetUsePersonalHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetUsePersonalHall(evaluateBool(params->string));
		}
	}

void sfUserIsYouAreHere(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsYouAreHere());
		}
	}

void sfUserSetYouAreHere(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetYouAreHere(evaluateBool(params->string));
		}
	}

void sfUserIsIBMRoom(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsIBMRoom());
		}
	}

void sfUserSetIBMRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetIBMRoom(evaluateBool(params->string));
		}
	}

void sfUserIsWideRoom(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsWideRoom());
		}
	}

void sfUserSetWideRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetWideRoom(evaluateBool(params->string));
		}
	}

void sfUserIsMusic(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsMusic());
		}
	}

void sfUserSetMusic(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetMusic(evaluateBool(params->string));
		}
	}

void sfUserIsCheckApostropheS(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsCheckApostropheS());
		}
	}

void sfUserSetCheckApostropheS(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCheckApostropheS(evaluateBool(params->string));
		}
	}

void sfUserIsCheckAllCaps(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsCheckAllCaps());
		}
	}

void sfUserSetCheckAllCaps(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCheckAllCaps(evaluateBool(params->string));
		}
	}

void sfUserIsCheckDigits(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsCheckDigits());
		}
	}

void sfUserSetCheckDigits(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetCheckDigits(evaluateBool(params->string));
		}
	}

void sfUserIsExcludeEncryptedMessages(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsExcludeEncryptedMessages());
		}
	}

void sfUserSetExcludeEncryptedMessages(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetExcludeEncryptedMessages(evaluateBool(params->string));
		}
	}

void sfUserIsViewCommas(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsViewCommas());
		}
	}

void sfUserSetViewCommas(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetViewCommas(evaluateBool(params->string));
		}
	}

void sfUserIsPUnPauses(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsPUnPauses());
		}
	}

void sfUserIsRoman(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsRoman());
		}
	}

void sfUserIsSuperSysop(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsSuperSysop());
		}
	}

void sfUserSetPUnPauses(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetPUnPauses(evaluateBool(params->string));
		}
	}

void sfUserSetRoman(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRoman(evaluateBool(params->string));
		}
	}

void sfUserSetSuperSysop(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetSuperSysop(evaluateBool(params->string));
		}
	}

void sfUserIsMainSysop(strList *)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsMainSysop());
		}
	}

void sfUserIsInGroup(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsInGroup(evaluateInt(params->string)));
		}
	}

void sfUserSetInGroup(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetInGroup(evaluateInt(params->string), evaluateBool(params->next->string));
		}
	}

void sfUserIsInRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsInRoom(evaluateInt(params->string)));
		}
	}

void sfUserSetInRoom(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetInRoom(evaluateInt(params->string), evaluateBool(params->next->string));
		}
	}

void sfUserIsRoomExcluded(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsRoomExcluded(evaluateInt(params->string)));
		}
	}

void sfUserSetRoomExcluded(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRoomExcluded(evaluateInt(params->string), evaluateBool(params->next->string));
		}
	}

void sfUserIsRoomInPersonalHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsRoomInPersonalHall(evaluateInt(params->string)));
		}
	}

void sfUserSetRoomInPersonalHall(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRoomInPersonalHall(evaluateInt(params->string), evaluateBool(params->next->string));
		}
	}

void sfUserGetRoomNewPointer(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETULONG(pCurScript->UsrUsr->GetRoomNewPointer(evaluateInt(params->string)));
		}
	}

void sfUserSetRoomNewPointer(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->SetRoomNewPointer(evaluateInt(params->string), evaluateUlong(params->next->string));
		}
	}

void sfUserGetUserDefined(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		pCurScript->UsrUsr->GetUserDefined(evaluateString(params->string), pCurScript->wowza, sizeof(pCurScript->wowza));
		SETRTSTR();
		}
	}

void sfUserSetUserDefined(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->SetUserDefined(evaluateString(params->string), evaluateString(params->next->string)));
		}
	}

void sfUserIsUserDefined(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->IsUserDefined(evaluateString(params->string)));
		}
	}

void sfUserRemoveUserDefined(strList *params)
	{
	if (pCurScript->UsrUsr)
		{
		SETBOOL(pCurScript->UsrUsr->RemoveUserDefined(evaluateString(params->string)));
		}
	}

void sfKillUser(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()killuser(params ? evaluateString(params->string) : NULL));
	}
