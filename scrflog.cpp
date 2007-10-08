// --------------------------------------------------------------------------
// Citadel: ScrFLog.CPP
//
// Internal functions for user scripts relating to the userlog.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "log.h"


// --------------------------------------------------------------------------
// CURRENT USER STUFF

void sfGetusername(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetName(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserin(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetInitials(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserpw(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetPassword(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserfwd(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetForwardAddr(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserfwdnode(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetForwardAddrNode(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetusersurname(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetSurname(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetusertitle(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetTitle(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserdefhall(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetDefaultHall(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetusernulls(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetNulls());
	}

void sfGetuserwidth(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetWidth());
	}

void sfGetusercalltime(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetCallTime());
	}

void sfGetusercallno(strList *)
	{
	RequiresScriptTW();
	SETULONG(ScriptTW()CurrentUser->GetCallNumber());
	}

void sfGetusercredits(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetCredits());
	}

void sfSetusercredits(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->SetCredits(evaluateLong(params->string));
	}

void sfGetuserlastpointer(strList *)
	{
	RequiresScriptTW();
	SETULONG(ScriptTW()CurrentUser->GetLastMessage());
	}

void sfGetuserlasthall(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetLastHall(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserlastroom(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetLastRoom(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserlines(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetLinesPerScreen());
	}

//attributes

void sfGetuserproto(strList *)
	{
	RequiresScriptTW();
	SETINT(ScriptTW()CurrentUser->GetDefaultProtocol());
	}

void sfGetuserprompt(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetPromptFormat(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserdstamp(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetDateStamp(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuservdstamp(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetVerboseDateStamp(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetusersig(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetSignature(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetusernetpref(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetNetPrefix(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuseraddr1(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetMailAddr1(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuseraddr2(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetMailAddr2(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuseraddr3(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetMailAddr3(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserpoop(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()CurrentUser->GetPoopcount());
	}

void sfIsuserinuse(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsInuse());
	}

void sfIsuseruponly(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsUpperOnly());
	}

void sfIsuserlf(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsLinefeeds());
	}

void sfIsuserexpert(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsExpert());
	}

void sfIsuseraide(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsAide());
	}

void sfIsusertabs(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsTabs());
	}

void sfIsuserlastold(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsOldToo());
	}

void sfIsuserproblem(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsProblem());
	}

void sfIsuserunlisted(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsUnlisted());
	}

void sfIsuserperm(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsPermanent());
	}

void sfIsusersysop(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsSysop());
	}

void sfIsusernode(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsNode());
	}

void sfIsusernet(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsNetUser());
	}

void sfIsusernoacct(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsAccounting());
	}

void sfIsusernomail(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsMail());
	}

void sfIsuserroomtell(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewRoomDesc());
	}

void sfIsuserfwdnode(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsForwardToNode());
	}

void sfIsusernexthall(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsAutoNextHall());
	}

void sfIsuserentborders(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsEnterBorders());
	}

void sfIsuserverified(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsVerified());
	}

void sfIsusersurlock(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsSurnameLocked());
	}

void sfIsuserhalllock(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsDefaultHallLocked());
	}

void sfIsuserpsycho(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsPsycho());
	}

void sfIsuserdisplayts(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewTitleSurname());
	}

void sfIsusersubjects(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewSubjects());
	}

void sfIsusersignatures(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewSignatures());
	}

void sfIsusertwirly(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsTwirly());
	}

void sfIsuserverbose(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsAutoVerbose());
	}

void sfIsusermsgpause(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsPauseBetweenMessages());
	}

void sfIsuserminibin(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsMinibin());
	}

void sfIsusermsgcls(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsClearScreenBetweenMessages());
	}

void sfIsuserroominfo(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewRoomInfoLines());
	}

void sfIsuserhalltell(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewHallDescription());
	}

void sfIsuservcont(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsVerboseContinue());
	}

void sfIsuserviewcensor(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewCensoredMessages());
	}

void sfIsuserseeborders(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsViewBorders());
	}

void sfIsuserout300(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsOut300());
	}

void sfIsuserlocksig(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsUserSignatureLocked());
	}

void sfIsuserhideexcl(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsHideMessageExclusions());
	}

void sfIsusernodownload(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsDownload());
	}

void sfIsusernoupload(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsUpload());
	}

void sfIsusernochat(strList *)
	{
	RequiresScriptTW();
	SETBOOL(!ScriptTW()CurrentUser->IsChat());
	}

void sfIsuserprint(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsPrintFile());
	}

void sfGetuserrealname(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetRealName(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetuserphone(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CurrentUser->GetPhoneNumber(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}


// --------------------------------------------------------------------------
// LOG TABLE STUFF

void sfGetltpwhash(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETLONG(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].GetPasswordHash());
		}
	else
		{
		SETLONG(LogTab[evaluateInt(params->string)].GetPasswordHash());
		}
#else
	SETINT(LTab(evaluateInt(params->string)).GetPasswordHash());
#endif
	}

void sfGetltinhash(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETLONG(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].GetInitialsHash());
		}
	else
		{
		SETLONG(LogTab[evaluateInt(params->string)].GetInitialsHash());
		}
#else
	SETINT(LTab(evaluateInt(params->string)).GetInitialsHash());
#endif
	}

void sfGetltname(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		LogTab[pCurScript->TW->LogOrder[evaluateInt(params->string)]].
				GetName(pCurScript->wowza, sizeof(pCurScript->wowza));
		}
	else
		{
		LogTab[evaluateInt(params->string)].GetName(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		}
#else
	LTab(evaluateInt(params->string)).GetName(pCurScript->wowza,
			sizeof(pCurScript->wowza));
#endif
	SETRTSTR();
	}

void sfGetltalias(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		LogTab[pCurScript->TW->LogOrder[evaluateInt(params->string)]].
				GetAlias(pCurScript->wowza, sizeof(pCurScript->wowza));
		}
	else
		{
		LogTab[evaluateInt(params->string)].GetAlias(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		}
#else
	LTab(evaluateInt(params->string)).GetAlias(pCurScript->wowza,
			sizeof(pCurScript->wowza));
#endif
	SETRTSTR();
	}

void sfGetltlocid(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		LogTab[pCurScript->TW->LogOrder[evaluateInt(params->string)]].
				GetLocID(pCurScript->wowza, sizeof(pCurScript->wowza));
		}
	else
		{
		LogTab[evaluateInt(params->string)].GetLocID(pCurScript->wowza,
				sizeof(pCurScript->wowza));
		}
#else
	LTab(evaluateInt(params->string)).GetLocID(pCurScript->wowza,
			sizeof(pCurScript->wowza));
#endif
	SETRTSTR();
	}

void sfGetltlogslot(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETINT(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].GetLogIndex());
		}
	else
		{
		SETINT(LogTab[evaluateInt(params->string)].GetLogIndex());
		}
#else
	SETINT(LTab(evaluateInt(params->string)).GetLogIndex());
#endif
	}

void sfGetltcallno(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETULONG(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].GetCallNumber());
		}
	else
		{
		SETULONG(LogTab[evaluateInt(params->string)].GetCallNumber());
		}
#else
	SETULONG(LTab(evaluateInt(params->string)).GetCallNumber());
#endif
	}

void sfGetltinuse(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsInuse());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsInuse());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsInuse());
#endif
	}

void sfGetltucmask(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsUpperOnly());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsUpperOnly());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsUpperOnly());
#endif
	}

void sfGetltlfmask(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsLinefeeds());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsLinefeeds());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsLinefeeds());
#endif
	}

void sfGetltexpert(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsExpert());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsExpert());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsExpert());
#endif
	}

void sfGetltaide(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsAide());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsAide());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsAide());
#endif
	}

void sfGetlttabs(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsTabs());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsTabs());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsTabs());
#endif
	}

void sfGetltoldtoo(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsOldToo());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsOldToo());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsOldToo());
#endif
	}

void sfGetltproblem(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsProblem());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsProblem());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsProblem());
#endif
	}

void sfGetltunlisted(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsUnlisted());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsUnlisted());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsUnlisted());
#endif
	}

void sfGetltpermanent(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsPermanent());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsPermanent());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsPermanent());
#endif
	}

void sfGetltsysop(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsSysop());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsSysop());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsSysop());
#endif
	}

void sfGetltnode(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsNode());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsNode());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsNode());
#endif
	}

void sfGetltnetuser(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsNetUser());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsNetUser());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsNetUser());
#endif
	}

void sfGetltnoaccount(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsAccounting());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsAccounting());
		}
#else
	SETBOOL(!LTab(evaluateInt(params->string)).IsAccounting());
#endif
	}

void sfGetltnomail(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsMail());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsMail());
		}
#else
	SETBOOL(!LTab(evaluateInt(params->string)).IsMail());
#endif
	}

void sfGetltroomtell(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		SETBOOL(LogTab[pCurScript->TW->LogOrder[
				evaluateInt(params->string)]].IsViewRoomDesc());
		}
	else
		{
		SETBOOL(LogTab[evaluateInt(params->string)].IsViewRoomDesc());
		}
#else
	SETBOOL(LTab(evaluateInt(params->string)).IsViewRoomDesc());
#endif
	}

void sfIsingroup(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CurrentUser->IsInGroup(evaluateInt(params->string)));
	}

void sfFindUserByName(strList *params)
	{
	l_slot slot;

	if (evaluateBool(params->next->string))
		{
#ifdef WINCIT
		slot = FindPersonByPartialName(evaluateString(params->string),
				pCurScript->TW ? pCurScript->TW->LogOrder : NULL);
#else
		slot = FindPersonByPartialName(evaluateString(params->string));
#endif
		}
	else
		{
#ifdef WINCIT
		slot = FindPersonByName(evaluateString(params->string),
				pCurScript->TW ? pCurScript->TW->LogOrder : NULL);
#else
		slot = FindPersonByName(evaluateString(params->string));
#endif
		}

	SETINT(slot);
	}
