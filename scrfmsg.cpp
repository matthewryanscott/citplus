// --------------------------------------------------------------------------
// Citadel: ScrFMsg.CPP
//
// Msg... internal script functions.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "room.h"
#include "log.h"
#include "net.h"
#include "maillist.h"


// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ MESSAGE STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
void sfGetNumberOfMessages(strList *)
	{
	SETLONG(MessageDat.NewestMessage() - MessageDat.OldestMessageInTable() + 1);
	}

void sfMsgTabIsMail(strList *params)
	{
	SETBOOL(MessageDat.IsMail(evaluateLong(params->string)));
	}

void sfMsgTabIsReceived(strList *params)
	{
	SETBOOL(MessageDat.IsReceived(evaluateLong(params->string)));
	}

void sfMsgTabIsRepliedTo(strList *params)
	{
	SETBOOL(MessageDat.IsReply(evaluateLong(params->string)));
	}

void sfMsgTabIsProblem(strList *params)
	{
	SETBOOL(MessageDat.IsProblem(evaluateLong(params->string)));
	}

void sfMsgTabIsMadevis(strList *params)
	{
	SETBOOL(MessageDat.IsMadevis(evaluateLong(params->string)));
	}

void sfMsgTabIsLimited(strList *params)
	{
	SETBOOL(MessageDat.IsLimited(evaluateLong(params->string)));
	}

void sfMsgTabIsModerated(strList *params)
	{
	SETBOOL(MessageDat.IsModerated(evaluateLong(params->string)));
	}

void sfMsgTabIsCopy(strList *params)
	{
	SETBOOL(MessageDat.IsCopy(evaluateLong(params->string)));
	}

void sfMsgTabIsNetMail(strList *params)
	{
	SETBOOL(MessageDat.IsNet(evaluateLong(params->string)));
	}

void sfMsgTabIsForwarded(strList *params)
	{
	SETBOOL(MessageDat.IsForwarded(evaluateLong(params->string)));
	}

void sfMsgTabIsNetworked(strList *params)
	{
	SETBOOL(MessageDat.IsNetworked(evaluateLong(params->string)));
	}

void sfMsgTabIsMassemail(strList *params)
	{
	SETBOOL(MessageDat.IsMassemail(evaluateLong(params->string)));
	}

void sfMsgTabIsCensored(strList *params)
	{
	SETBOOL(MessageDat.IsCensored(evaluateLong(params->string)));
	}

void sfMsgTabIsLocal(strList *params)
	{
	SETBOOL(MessageDat.IsLocal(evaluateLong(params->string)));
	}

void sfMsgTabIsInuse(strList *params)
	{
	SETBOOL(MessageDat.IsInuse(evaluateLong(params->string)));
	}

void sfMsgTabGetLocation(strList *params)
	{
	SETLONG(MessageDat.GetLocation(evaluateLong(params->string)));
	}

void sfMsgTabGetRoomNum(strList *params)
	{
	SETINT(MessageDat.GetRoomNum(evaluateLong(params->string)));
	}

void sfMsgTabGetToHash(strList *params)
	{
	SETLONG(MessageDat.GetToHash(evaluateLong(params->string)));
	}

void sfMsgTabGetAuthHash(strList *params)
	{
	SETLONG(MessageDat.GetAuthHash(evaluateLong(params->string)));
	}

void sfMsgTabGetOriginID(strList *params)
	{
	SETULONG(MessageDat.GetOriginID(evaluateLong(params->string)));
	}

void sfMsgTabGetPrevRoomMsg(strList *params)
	{
	#if defined(AUXMEM) || defined(WINCIT)
		SETLONG(MessageDat.GetPrevRoomMsg(evaluateLong(params->string)));
	#else
		SETLONG(M_INDEX_ERROR);
	#endif
	}

void sfMsgTabGetNextRoomMsg(strList *params)
	{
	#if defined(AUXMEM) || defined(WINCIT)
		SETLONG(MessageDat.GetNextRoomMsg(evaluateLong(params->string)));
	#else
		SETLONG(M_INDEX_ERROR);
	#endif
	}

void sfMsgTabGetCopyOffset(strList *params)
	{
	SETLONG(MessageDat.GetCopyOffset(evaluateLong(params->string)));
	}

void sfMsgStart(strList *)
	{
	delete pCurScript->UsrMsg;
	pCurScript->UsrMsg = new Message;

	SETBOOL(pCurScript->UsrMsg != NULL);
	}

void sfMsgClear(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->ClearAll();
		}
	}

void sfMsgHeaderClear(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->ClearHeader();
		}
	}

void sfMsgDump(strList *)
	{
	delete pCurScript->UsrMsg;
	pCurScript->UsrMsg = NULL;
	}

void sfMsgSave(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		// save it for netting...
		if (*pCurScript->UsrMsg->GetToNodeName())
			{
			// room names get included in networked mail
			pCurScript->UsrMsg->SetCreationRoom();

			if (!save_mail(pCurScript->UsrMsg, FALSE, NULL, CERROR))
				{
				SETBOOL(FALSE);
				return;
				}
			}

		putAndNoteMessage(pCurScript->UsrMsg, FALSE);

		if (*pCurScript->UsrMsg->GetToUser() && !*pCurScript->UsrMsg->GetToNodeName() &&
				IsMailingList(pCurScript->UsrMsg->GetToUser()))
			{
			SendMailToList(pCurScript->UsrMsg);
			}

		SETBOOL(TRUE);
		}
	}

void sfMsgLoad(strList *params)
	{
	RequiresScriptTW();

	delete pCurScript->UsrMsg;

	pCurScript->UsrMsg = ScriptTW()LoadMessageByID(evaluateLong(params->string),
			params->next ? evaluateBool(params->next->string) : FALSE, FALSE);

	if (pCurScript->UsrMsg)
		{
#ifdef WINCIT
		pCurScript->UsrMsg->Decompress(/* pCurScript->TW */);
#else
		pCurScript->UsrMsg->Decompress();
#endif
		}

	SETBOOL(pCurScript->UsrMsg != NULL);
	}

void sfMsgShow(strList *)
	{
	RequiresScriptTW();

	if (pCurScript->UsrMsg)
		{
		ScriptTW()PrintMessage(pCurScript->UsrMsg);
		}
	}

void sfMsgIsReceived(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsReceived());
		}
	}

void sfMsgIsRepliedTo(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsRepliedTo());
		}
	}

void sfMsgIsMadeVisible(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsMadeVisible());
		}
	}

void sfMsgIsCompressed(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->WasEverCompressed());
		}
	}

void sfMsgIsCensored(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsCensored());
		}
	}

void sfMsgIsLocal(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsLocal());
		}
	}

void sfMsgSetCensored(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetCensored(evaluateBool(params->string));
		}
	}

void sfMsgSetLocal(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetLocal(evaluateBool(params->string));
		}
	}

void sfMsgAppendText(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		for (; params; params = (strList *) getNextLL(params))
			{
			pCurScript->UsrMsg->AppendText(evaluateString(params->string));
			}
		}
	}

void sfMsgSetText(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetText(ns);

		sfMsgAppendText(params);
		}
	}

void sfMsgGetHeadLoc(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETLONG(pCurScript->UsrMsg->GetHeadLoc());
		}
	}

void sfMsgGetRoomNumber(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETINT(pCurScript->UsrMsg->GetRoomNumber());
		}
	}

void sfMsgIsReceiptConfirmationRequested(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsReceiptConfirmationRequested());
		}
	}

void sfMsgIsEncrypted(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsEncrypted());
		}
	}

void sfMsgGetEncryptionKey(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetEncryptionKey());
		}
	}

void sfMsgGetAuthor(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetAuthor());
		}
	}

void sfMsgGetRealName(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetRealName());
		}
	}

void sfMsgGetSurname(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetSurname());
		}
	}

void sfMsgGetTitle(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetTitle());
		}
	}

void sfMsgGetForward(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetForward());
		}
	}

void sfMsgGetOriginNodeName(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetOriginNodeName());
		}
	}

void sfMsgGetOriginRegion(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetOriginRegion());
		}
	}

void sfMsgGetOriginCountry(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetOriginCountry());
		}
	}

void sfMsgGetTwitRegion(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetTwitRegion());
		}
	}

void sfMsgGetTwitCountry(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetTwitCountry());
		}
	}

void sfMsgGetOriginPhoneNumber(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetOriginPhoneNumber());
		}
	}

void sfMsgGetCit86Country(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetCit86Country());
		}
	}

void sfMsgGetDestinationAddress(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetDestinationAddress());
		}
	}

void sfMsgGetCreationRoom(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetCreationRoom());
		}
	}

void sfMsgGetSignature(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetSignature());
		}
	}

void sfMsgGetUserSignature(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetUserSignature());
		}
	}

void sfMsgGetSubject(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetSubject());
		}
	}

void sfMsgGetOriginSoftware(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetOriginSoftware());
		}
	}

void sfMsgGetSourceID(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetSourceID());
		}
	}

void sfMsgGetFromPath(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetFromPath());
		}
	}

void sfMsgSetFromPath(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetFromPath(evaluateString(params->string));
		}
	}

void sfMsgGetToUser(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToUser());
		}
	}

void sfMsgGetPublicToUser(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetPublicToUser());
		}
	}

void sfMsgGetToNodeName(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToNodeName());
		}
	}

void sfMsgGetToRegion(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToRegion());
		}
	}

void sfMsgGetToCountry(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToCountry());
		}
	}

void sfMsgGetToPhoneNumber(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToPhoneNumber());
		}
	}

void sfMsgGetToPath(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetToPath());
		}
	}

void sfMsgGetLocalID(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetLocalID());
		}
	}

void sfMsgGetCopyOfMessage(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetCopyOfMessage());
		}
	}

void sfMsgGetGroup(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetGroup());
		}
	}

void sfMsgGetCreationTime(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetCreationTime());
		}
	}

void sfMsgGetEZCreationTime(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetEZCreationTime());
		}
	}

void sfMsgGetReplyToMessage(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetReplyToMessage());
		}
	}

void sfMsgGetX(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetX());
		}
	}

void sfMsgGetFileLink(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetFileLink());
		}
	}

void sfMsgGetSourceRoomName(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETSTR(pCurScript->UsrMsg->GetSourceRoomName());
		}
	}

void sfMsgSetRoomNumber(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetRoomNumber(evaluateInt(params->string));
		}
	}

void sfMsgSetReceiptConfirmationRequested(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->SetReceiptConfirmationRequested(
				evaluateBool(params->string)));
		}
	}

void sfMsgSetEncrypted(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->SetEncrypted(evaluateBool(params->string)));
		}
	}

void sfMsgSetEncryptionKey(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetEncryptionKey(evaluateString(params->string));
		}
	}

void sfMsgSetAuthor(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetAuthor(evaluateString(params->string));
		}
	}

void sfMsgSetRealName(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetRealName(evaluateString(params->string));
		}
	}

void sfMsgSetSurname(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetSurname(evaluateString(params->string));
		}
	}

void sfMsgSetTitle(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetTitle(evaluateString(params->string));
		}
	}

void sfMsgSetForward(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetForward(evaluateString(params->string));
		}
	}

void sfMsgSetDestinationAddress(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetDestinationAddress(evaluateString(params->string));
		}
	}

void sfMsgSetCreationRoom(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetCreationRoom(evaluateString(params->string));
		}
	}

void sfMsgSetSignature(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetSignature(evaluateString(params->string));
		}
	}

void sfMsgSetUserSignature(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetUserSignature(evaluateString(params->string));
		}
	}

void sfMsgSetSubject(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetSubject(evaluateString(params->string));
		}
	}

void sfMsgSetToUser(strList *params)
	{
	RequiresScriptTW();

	Bool Set = TRUE;

	if (pCurScript->UsrMsg)
		{
		label SaveToUser;
		label SaveToNodeName;
		label SaveToRegion;
		label SaveDestinationAddress;

		strcpy(SaveToUser, pCurScript->UsrMsg->GetToUser());
		strcpy(SaveToNodeName, pCurScript->UsrMsg->GetToNodeName());
		strcpy(SaveToRegion, pCurScript->UsrMsg->GetToRegion());
		strcpy(SaveDestinationAddress, pCurScript->UsrMsg->GetDestinationAddress());

		char destination[LABELSIZE + LABELSIZE + LABELSIZE + LABELSIZE + 7];

		strcpy(destination, evaluateString(params->string));

		if (*destination)
			{
			if (SameString(destination, spc))
				{
				pCurScript->UsrMsg->SetToUser(ns);
				pCurScript->UsrMsg->SetToNodeName(ns);
				pCurScript->UsrMsg->SetToRegion(ns);
				pCurScript->UsrMsg->SetDestinationAddress(ns);
				}
			else
				{
				label ToUser, ToNode, ToRegion, ToAddress;

				ScriptTW()parseNetAddress(destination, ToUser, ToNode, ToRegion, ToAddress);

				if (!*ToUser)
					{
					strcpy(ToUser, getmsg(386));
					}

				pCurScript->UsrMsg->SetToUser(ToUser);
				pCurScript->UsrMsg->SetToNodeName(ToNode);
				pCurScript->UsrMsg->SetToRegion(ToRegion);
				pCurScript->UsrMsg->SetDestinationAddress(ToAddress);
				}
			}

		if (*pCurScript->UsrMsg->GetToUser())
			{
#ifdef WINCIT
			Set = ResolveMessageAddressingIssues(pCurScript->UsrMsg, pCurScript->TW, NOMODCON);
#else
            Set = ResolveMessageAddressingIssues(pCurScript->UsrMsg, NOMODCON);
#endif

			if (!Set)
				{
				pCurScript->UsrMsg->SetToUser(SaveToUser);
				pCurScript->UsrMsg->SetToNodeName(SaveToNodeName);
				pCurScript->UsrMsg->SetToRegion(SaveToRegion);
				pCurScript->UsrMsg->SetDestinationAddress(SaveDestinationAddress);
				}
			}
		}
	else
		{
		Set = FALSE;
		}

	SETBOOL(Set);
	}

void sfMsgSetPublicToUser(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetPublicToUser(evaluateString(params->string));
		}
	}

void sfMsgSetToPhoneNumber(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetToPhoneNumber(evaluateString(params->string));
		}
	}

void sfMsgSetToPath(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetToPath(evaluateString(params->string));
		}
	}

void sfMsgSetGroup(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetGroup(evaluateString(params->string));
		}
	}

void sfMsgSetCreationTime(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetCreationTime(evaluateString(params->string));
		}
	}

void sfMsgSetEZCreationTime(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetEZCreationTime(evaluateString(params->string));
		}
	}

void sfMsgSetReplyToMessage(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetReplyToMessage(evaluateString(params->string));
		}
	}

void sfMsgSetX(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetX(evaluateString(params->string));
		}
	}

void sfMsgSetFileLink(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetFileLink(evaluateString(params->string));
		}
	}

void sfMsgSetSourceRoomName(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetSourceRoomName(evaluateString(params->string));
		}
	}

void sfMsgIsViewDuplicate(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->IsViewDuplicate());
		}
	}

void sfMsgGetOriginalID(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETULONG(pCurScript->UsrMsg->GetOriginalID());
		}
	}

void sfMsgGetOriginalAttribute(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETINT(pCurScript->UsrMsg->GetOriginalAttribute());
		}
	}

void sfMsgGetOriginalRoom(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETINT(pCurScript->UsrMsg->GetOriginalRoom());
		}
	}

void sfMsgSetOriginalRoom(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->SetOriginalRoom(evaluateInt(params->string));
		}
	}

void sfMsgGetOriginAddress(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg->GetOriginAddress(pCurScript->wowza);
		pCurScript->lastResult.v.string = pCurScript->wowza;
		pCurScript->lastResult.type = TYPE_STRING_MAX;
		}
	}

void sfMsgSaveTextToFile(strList *params)
	{
	RequiresScriptTW();

	Bool Worked = FALSE;

	if (pCurScript->UsrMsg)
		{
		FILE *fd = fopen(evaluateString(params->string), FO_WB);
		if (fd)
			{
			ScriptTW()dFormat(pCurScript->UsrMsg->GetText(), fd);
			fclose(fd);

			Worked = TRUE;
			}
		}

	SETBOOL(Worked);
	}

void sfMsgLoadTextFromFile(strList *params)
	{
	Bool Worked = FALSE;

	if (pCurScript->UsrMsg)
		{
		FILE *fd = fopen(evaluateString(params->string), FO_RB);
		if (fd)
			{
			GetFileMessage(fd, pCurScript->UsrMsg->GetTextPointer(), MAXTEXT);
			fclose(fd);

			Worked = TRUE;
			}
		}

	SETBOOL(Worked);
	}

void sfMsgIsMoreFlag(strList *params)
	{
	if (pCurScript->UsrMsg && params)
		{
		const char Flag = evaluateString(params->string)[0];

		SETBOOL(Flag ? pCurScript->UsrMsg->IsMoreFlag(Flag) : FALSE);
		}
	}

void sfMsgSetMoreFlag(strList *params)
	{
	if (pCurScript->UsrMsg && params && params->next)
		{
		const char Flag = evaluateString(params->string)[0];

		SETBOOL(Flag ? pCurScript->UsrMsg->SetMoreFlag(Flag, evaluateBool(params->next->string)) : FALSE);
		}
	}

void sfMsgGetCommentCount(strList *)
	{
	if (pCurScript->UsrMsg)
		{
		SETLONG(pCurScript->UsrMsg->GetCommentCount());
		}
	}

void sfMsgGetComment(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		const char *Cmt = pCurScript->UsrMsg->GetComment(evaluateLong(params->string));

		if (Cmt)
			{
			SETSTR(Cmt);
			}
		else
			{
			SETSTR(ns);
			}
		}
	}

void sfMsgAddComment(strList *params)
	{
	if (pCurScript->UsrMsg)
		{
		SETBOOL(pCurScript->UsrMsg->AddComment(evaluateString(params->string)));
		}
	}

// These three are not really scrfmsg functions, but i didn't want to include
// msg.h in scrfunc.cpp

void sfAppendToNetCommandReply(strList *params)
	{
	RequiresScriptTW();

	if (ScriptTW()NetCmdReplyMsg)
		{
		for (; params; params = (strList *) getNextLL(params))
			{
			ScriptTW()NetCmdReplyMsg->AppendText(evaluateString(params->string));
			}
		}
	}

void sfGetNetCommandText(strList *)
	{
	RequiresScriptTW();

	if (ScriptTW()NetCommandMsg)
		{
		SETSTR(ScriptTW()NetCommandMsg->GetText());
		}
	else
		{
		SETSTR(ns);
		}
	}

void sfGetNetCommandTextToFile(strList *params)
	{
	RequiresScriptTW();

	Bool Worked = FALSE;

	if (ScriptTW()NetCommandMsg)
		{
		FILE *fd;

		if ((fd = fopen(evaluateString(params->string), FO_WB)) != NULL)
			{
			ScriptTW()dFormat(ScriptTW()NetCommandMsg->GetText(), fd);
			fclose(fd);

			Worked = TRUE;
			}
		}

	SETBOOL(Worked);
	}

void sfStartMessageEditor(strList *params)
	{
	RequiresScriptTW();

	Bool WeMadeIt = FALSE;

	if (!pCurScript->UsrMsg)
		{
		pCurScript->UsrMsg = new Message;
		WeMadeIt = TRUE;
		}

	if (pCurScript->UsrMsg)
		{
		const Bool Saved = ScriptTW()makeMessage(pCurScript->UsrMsg, NULL, evaluateInt(params->string));

		if (WeMadeIt)
			{
			delete pCurScript->UsrMsg;
			pCurScript->UsrMsg = NULL;
			}

		SETBOOL(Saved);
		}
	else
		{
		SETBOOL(FALSE);
		}
	}
