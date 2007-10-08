// --------------------------------------------------------------------------
// Citadel: DoAide.CPP
//
// Code for doAide() and some function implemetations.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "infofile.h"
#include "filecmd.h"
#include "log.h"
#include "msg.h"
#include "hall.h"
#include "helpfile.h"
#include "miscovl.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// doAide() 	handles the aide-only menu


// --------------------------------------------------------------------------
// doAide(): Handles the aide-only menu.

void TERMWINDOWMEMBER doAide(Bool moreYet, char first)
	{
	if (!LockMenu(MENU_AIDE))
		{
		return;
		}

	SetDoWhat(DOAIDE);

	MRO.Reverse = FALSE;
	MRO.Verbose = CurrentUser->IsAutoVerbose();

	if (moreYet)
		{
		first = 0;
		}

	mPrintf(getmenumsg(MENU_AIDE, 24));

	const int ich = first ? first : iCharNE();

	switch (DoMenu(MENU_AIDE, ich))
		{
		case 1: // Attributes
			{
			DoCommand(UC_AIDEATTRIBUTES);
			break;
			}

		case 2: // Chat
			{
			SetDoWhat(AIDECHAT);

			const Bool oldchat = cfg.noChat;

			if (cfg.aideChatHours[hour()])
				{
				cfg.noChat = FALSE;
				}

			ChatConsole();
//			doChat(false);

			cfg.noChat = oldchat;

			break;
			}

		case 3: // Edit room
			{
			DoCommand(UC_AIDEEDITROOM);
			break;
			}

		case 4: // File info set
			{
			DoCommand(UC_AIDEFILEINFOSET);
			break;
			}

		case 5: // Group membership
			{
			DoCommand(UC_AIDEGROUP);
			break;
			}

		case 6: // Hall
			{
			DoCommand(UC_AIDEHALL);
			break;
			}

		case 7: // Insert message
			{
			DoCommand(UC_AIDEINSERT);
			break;
			}

		case 8: // Jump to Mainenance and Aide
			{
			DoCommand(UC_AIDEJUMPTOAIDEANDMAINTENANCE);
			break;
			}

		case 9: // Kill room
			{
			DoCommand(UC_AIDEKILLROOM);
			break;
			}

		case 10: // List group
			{
			DoCommand(UC_AIDELISTGROUP);
			break;
			}

		case 11: // Move file
			{
			DoCommand(UC_AIDEMOVEFILE);
			break;
			}

		case 12: // Name messages
			{
			DoCommand(UC_AIDENAMEMESSAGES);
			break;
			}

		case 13: // Queue
			{
			SetDoWhat(AIDEQUEUE);

			if (LockMenu(MENU_AIDEQUEUE))
				{
				Bool done;
				do
					{
					done = TRUE;
					const int ich = iCharNE();

					switch (DoMenu(MENU_AIDEQUEUE, ich))
						{
						case 1: // Auto mark/kill/censor
							{
							DoCommand(UC_AIDEQUEUEAUTOMARK);
							break;
							}

						case 2: // Clear
							{
							DoCommand(UC_AIDEQUEUECLEAR);
							break;
							}

						case 3: // Insert
							{
							DoCommand(UC_AIDEQUEUEINSERT);
							break;
							}

						case 4: // Kill
							{
							DoCommand(UC_AIDEQUEUEKILL);
							break;
							}

						case 5: // List
							{
							DoCommand(UC_AIDEQUEUELIST);
							break;
							}

						case 6: // Move
							{
							DoCommand(UC_AIDEQUEUEMOVE);
							break;
							}

						case 7: // Room mark
							{
							DoCommand(UC_AIDEQUEUEROOMMARK);
							break;
							}

						case 8: // Sort
							{
							DoCommand(UC_AIDEQUEUESORT);
							break;
							}

						case 9: // Verbose
							{
							MRO.Verbose = TRUE;
							done = FALSE;
							break;
							}

						case -1:
							{
							break;
							}

						default:
							{
							oChar((char) ich);
							mPrintf(sqst);

							if (!CurrentUser->IsExpert())
								{
								break;
								}
							}

						case -2:
							{
							if (ich == '?')
								{
								oChar('?');
								}

							showMenu(M_AIDEQUEUE);
							break;
							}
						}
					} while (!done);

				UnlockMenu(MENU_AIDEQUEUE);
				}

			break;
			}

		case 14: // Rename file
			{
			DoCommand(UC_AIDERENAMEFILE);
			break;
			}

		case 15: // Set file info
			{
			DoCommand(UC_AIDESETFILEINFO);
			break;
			}

		case 16: // Unlink file
			{
			DoCommand(UC_AIDEUNLINK);
			break;
			}

		case 17: // Window
			{
			DoCommand(UC_AIDEWINDOW);
			break;
			}

		case 18: // Move room forward
			{
			DoCommand(UC_AIDEMOVEROOM, 1);
			break;
			}

		case 19: // Move room backwards
			{
			DoCommand(UC_AIDEMOVEROOM, -1);
			break;
			}

		case 20: // Copy message into old buffer
			{
			DoCommand(UC_AIDECOPYMESSAGE);
			break;
			}

		case -2:
			{
			oChar('?');
			SetDoWhat(AIDEMENU);
			showMenu(M_AIDE);
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

	UnlockMenu(MENU_AIDE);
	}

void TERMWINDOWMEMBER AideNameMessages(void)
	{
	if (LockMenu(MENU_AIDE))
		{
		doCR();

		if (!cfg.msgNym)
			{
			CRmPrintfCR(getmenumsg(MENU_AIDE, 125));
			return;
			}

		Message *Msg = new Message;
		if (Msg)
			{
			label stuff;
			char string[64];

			SetDoWhat(AIDENYM);

			GetStringWithBlurb(getmenumsg(MENU_AIDE, 126), stuff,
					LABELSIZE, cfg.Lmsg_nym, B_MSGNYM);

			normalizeString(stuff);

			if (*stuff)
				{
				label L;
				CopyStringToBuffer(cfg.Lmsg_nym, MakeItLower(stuff, L));
				CopyStringToBuffer(cfg.Umsg_nym, MakeItUpper(stuff, L));
				}

			GetStringWithBlurb(getmenumsg(MENU_AIDE, 127), stuff,
					LABELSIZE, cfg.Lmsgs_nym, B_MSGNYM);

			normalizeString(stuff);

			if (*stuff)
				{
				label L;
				CopyStringToBuffer(cfg.Lmsgs_nym, MakeItLower(stuff, L));
				CopyStringToBuffer(cfg.Umsgs_nym, MakeItUpper(stuff, L));
				}

			sprintf(string, getmenumsg(MENU_AIDE, 128), cfg.Lmsgs_nym);

			GetStringWithBlurb(string, stuff, LABELSIZE, cfg.msg_done,
					B_MSGNYM);
			normalizeString(stuff);

			if (*stuff)
				{
				CopyStringToBuffer(cfg.msg_done, stuff);
				}

			label Buffer;
			Msg->SetTextWithFormat(getmenumsg(MENU_AIDE, 0),
					CurrentUser->GetName(Buffer, sizeof(Buffer)),
					cfg.Lmsg_nym, cfg.Lmsgs_nym, cfg.msg_done);
			Msg->SetRoomNumber(AIDEROOM);

			systemMessage(Msg);

			delete Msg;
			}
		else
			{
			mPrintf(getmsg(188), getmenumsg(MENU_AIDE, 130));
			}

		UnlockMenu(MENU_AIDE);
		}
	}
