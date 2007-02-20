// --------------------------------------------------------------------------
// Citadel: Commands.CPP
//
// The central user command dispatcher thingie.

#include "ctdl.h"
#pragma hdrstop


void TERMWINDOWMEMBER DoCommand(UserCommandsE Cmd, long P1, long P2, long P3)
	{
	if (*CommandOverrides[Cmd])
		{
		sprintf(ContextSensitiveScriptText, getmsg(336), P1, P2, P3);

#ifdef WINCIT
		runScript(CommandOverrides[Cmd], this, NULL);
#else
		runScript(CommandOverrides[Cmd], NULL);
#endif
		}
	else
		{
		switch (Cmd)
			{
			case UC_AIDEATTRIBUTES:					AideAttributes();						break;
			case UC_AIDEEDITROOM:					AideEditRoom();							break;
			case UC_AIDEFILEINFOSET:				AideFileInfoSet();						break;
			case UC_AIDEGROUP:						AideGroup();							break;
			case UC_AIDEHALL:						AideHall();								break;
			case UC_AIDEINSERT:						AideInsert();							break;
			case UC_AIDEJUMPTOAIDEANDMAINTENANCE:	AideJumpToAideAndMaintenance();			break;
			case UC_AIDEKILLROOM:					AideKillRoom();							break;
			case UC_AIDELISTGROUP:					AideListGroup();						break;
			case UC_AIDEMOVEFILE:					AideMoveFile();							break;
			case UC_AIDENAMEMESSAGES:				AideNameMessages();						break;
			case UC_AIDEQUEUEAUTOMARK:				AideQueueAutomark();					break;
			case UC_AIDEQUEUECLEAR:					AideQueueClear();						break;
			case UC_AIDEQUEUEINSERT:				AideQueueInsert();						break;
			case UC_AIDEQUEUEKILL:					AideQueueKill();						break;
			case UC_AIDEQUEUELIST:					AideQueueList();						break;
			case UC_AIDEQUEUEMOVE:					AideQueueMove();						break;
			case UC_AIDEQUEUEROOMMARK:				AideQueueRoomMark();					break;
			case UC_AIDEQUEUESORT:					AideQueueSort();						break;
			case UC_AIDERENAMEFILE:					AideRenameFile();						break;
			case UC_AIDESETFILEINFO:    			AideSetFileInfo();  		    		break;
			case UC_AIDEUNLINK:						AideUnlink();							break;
			case UC_AIDEWINDOW:						AideWindow();							break;
			case UC_AIDEMOVEROOM:					AideMoveRoom(P1, TRUE);					break;
			case UC_AIDECOPYMESSAGE:				AideCopyMessage();						break;
#ifdef WINCIT
			case UC_CHATALL:						ChatAll();								break;
			case UC_CHATUSER:						ChatUser();								break;
			case UC_CHATGROUP:						ChatGroup();							break;
			case UC_CHATROOM:						ChatRoom();								break;
#endif
			case UC_CHATCONSOLE:					ChatConsole();							break;
			case UC_ENTERAPPLICATION:				EnterApplication();						break;
			case UC_ENTERBORDER:					EnterBorder();							break;
			case UC_ENTERCONFIGURATION:				EnterConfiguration(FALSE);				break;
			case UC_ENTERDOOR:						EnterDoor();							break;
			case UC_ENTERHALL:						EnterHall();							break;
			case UC_ENTERNAME:						EnterName();							break;
			case UC_ENTERPASSWORD:					EnterPassword();						break;
			case UC_ENTERROOM:						EnterRoom(P1);							break;
			case UC_ENTERTITLESURNAME:				EnterTitleSurname();					break;
			case UC_ENTERTEXTFILE:					EnterTextfile(P1);						break;
			case UC_ENTERWITHPROTOCOL:				EnterWithProtocol(P1);					break;
			case UC_EXCLUDEROOM:					exclude();								break;
			case UC_INVITEUSERLIST:					InviteUserList();						break;
			case UC_INVITEROOMLIST:					InviteRoomList();						break;
			case UC_INVITEUSER:						InviteUser();							break;
			case UC_INTRO:							doIntro();								break;
			case UC_TERMINATEQUITALSO:				TerminateQuitAlso(P1);					break;
			case UC_TERMINATESTAY:					TerminateStay();						break;
			case UC_EXPERTTOGGLE:					doXpert();								break;
			case UC_VERBOSETOGGLE:					doverbose();							break;
			case UC_Y:								doY(P1);								break;
			case UC_FINGERDELETE:					FingerDelete();							break;
			case UC_FINGEREDIT:						FingerEdit();							break;
            case UC_FINGERLIST:                     FingerList(TRUE);                       break;
			case UC_FINGERUSER:						FingerUser();							break;
			case UC_READCONFIGURATION:				ReadConfiguration();					break;
			case UC_READDIRECTORY:					ReadDirectory((OldNewPick) P1);			break;
			case UC_READHALLS:						ReadHalls();							break;
			case UC_READINFOFILE:					ReadInfoFile(TRUE, (OldNewPick) P1);	break;
			case UC_READSTATUS:						ReadStatus();							break;
			case UC_READTEXTFILE:					ReadTextfile((OldNewPick) P1, P2);		break;
			case UC_READUSERLOG:					ReadUserlog(P1, (OldNewPick) P2, P3);	break;
			case UC_READWITHPROTOCOL:				ReadWithProtocol();						break;
			case UC_READARCHIVE:					ReadArchive((OldNewPick) P1);			break;
			default:								assert(FALSE);							break;
			}
		}
	}
