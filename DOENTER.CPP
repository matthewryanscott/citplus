// --------------------------------------------------------------------------
// Citadel: DoEnter.CPP
//
// .E... command code.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "filecmd.h"
#include "tallybuf.h"
#include "log.h"
#include "net.h"
#include "group.h"
#include "hall.h"
#include "helpfile.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// doEnter()	Handles E(nter) command.

extern ConsoleLockC ConsoleLock;


// --------------------------------------------------------------------------
// doEnter(): Handles E(nter) command.

void TERMWINDOWMEMBER doEnter(Bool moreYet, char first)
	{
	if (!LockMenu(MENU_ENTER))
		{
		OutOfMemory(1);
		return;
		}

	SetDoWhat(DOENTER);

	if (moreYet)
		{
		first = 0;
		}

	mPrintf(getmenumsg(MENU_ENTER, 36));

	int OptionCounter = 0;
	Bool EnterLocal = FALSE, EnterOld = FALSE, EnterFileLinked = FALSE;
	Bool EnterMail = FALSE, EnterGroup = FALSE, done;

	do
		{
		done = TRUE;

		int ich = first ? first : iCharNE();
		const int Uch = toupper(ich);

		if (++OptionCounter == 69)
			{
			mPrintf(getmsg(472), getmsg(69), spc);
			OptionCounter = 0;
			}

		// allow exclusive mail entry only. !!!! 'E' is not translatable
		if (!loggedIn && !cfg.unlogEnterOk && (Uch != 'E'))
			{
			CRCRmPrintfCR(getmenumsg(MENU_ENTER, 37));
			break;
			}

		// !!!! 'S' is not translatable
		if (Uch == 'S' && !(cfg.surnames || cfg.titles))
			{
			ich = 0; 	// force non-recognition
			}

		switch (DoMenu(MENU_ENTER, ich))
			{
			case 1: // Application
				{
				DoCommand(UC_ENTERAPPLICATION);
				break;
				}

			case 2: // Border
				{
				DoCommand(UC_ENTERBORDER);
				break;
				}

			case 3: // Configuration
				{
				DoCommand(UC_ENTERCONFIGURATION);
				break;
				}

			case 4: // Door
				{
				DoCommand(UC_ENTERDOOR);
				break;
				}

			case 5: // Group-only
				{
				done = FALSE;
				EnterGroup = TRUE;
				break;
				}

			case 6: // Hall
				{
				DoCommand(UC_ENTERHALL);
				break;
				}

			case 7: // Local
				{
				done = FALSE;
				EnterLocal = TRUE;
				break;
				}

			case 8: // Exclusive
				{
				EnterMail = TRUE;

				// handle nomail flag!
				if (!CurrentUser->IsMail())
					{
					CRCRmPrintfCR(getmenumsg(MENU_ENTER, 21), cfg.Lmsgs_nym);
					UnlockMenu(MENU_ENTER);
					return;
					}

				// fall through
				}

			case 9: // Message
				{
				mPrintf(pcts, cfg.Umsg_nym);

				// handle readonly flag!
				if (RoomTab[thisRoom].IsReadOnly() && !CurrentUser->HasRoomPrivileges(thisRoom))
					{
					CRCRmPrintfCR(getmenumsg(MENU_ENTER, 22), cfg.Uroom_nym);
					UnlockMenu(MENU_ENTER);
					return;
					}

				// handle steeve
				// We are checking (-1 means no check) and we are not an Aide and we have reached our limit
				if (cfg.MessageRoom >= 0 && !CurrentUser->IsAide() && Talley->EnteredInRoom(thisRoom) >= cfg.MessageRoom)
					{
					CRCRmPrintfCR(getmenumsg(MENU_ENTER, 23), cfg.MessageRoom,
							cfg.MessageRoom == 1 ? cfg.Lmsg_nym : cfg.Lmsgs_nym, cfg.Lroom_nym);
					UnlockMenu(MENU_ENTER);
					return;
					}

				// user configurability is your friend
				if (!EnterOld && MS.AbortedMessage && CurrentUser->IsConfirmNoEO())
					{
					char prompt[128];
					sprintf(prompt, getmenumsg(MENU_ENTER, 24), cfg.Lmsg_nym);

					doCR();
					if (!getYesNo(prompt, 0))
						{
						UnlockMenu(MENU_ENTER);
						return;
						}
					}

				if (EnterOld && !MS.AbortedMessage)
					{
					CRCRmPrintfCR(getmenumsg(MENU_ENTER, 25), cfg.Lmsg_nym);
					UnlockMenu(MENU_ENTER);
					return;
					}

				Message *Msg = new Message;

				if (Msg)
					{
					Bool Massemail = FALSE;

					// let's handle .Enter Old
					if (EnterOld)
						{
						doCR();
						CRmPrintf(getmenumsg(MENU_ENTER, 26), cfg.Lmsg_nym);

						*Msg = *MS.AbortedMessage;
						Msg->SetCensored(FALSE);	// check later
						}

					if (EnterFileLinked)
						{
						char LinkedFile[64];

						doCR();

						getNormStr(getmenumsg(MENU_ENTER, 27), LinkedFile, 63);

						if (!*LinkedFile)
							{
							delete Msg;
							UnlockMenu(MENU_ENTER);
							return;
							}

						Msg->SetFileLink(LinkedFile);
						Msg->SetText(ns);				// .EO*
						}

					if (RoomTab[thisRoom].IsShared())
						{
						Msg->SetLocal(EnterLocal);
						}

					doCR();

					if (EnterMail || (EnterOld && Msg->IsMail()))
						{
						OC.Echo = CALLER;
						OC.setio();

						if (CurrentUser->IsSysop() && EnterGroup)
							{
							Massemail = TRUE;
							}
						else
							{
							EnterGroup = FALSE;

							if (loggedIn)
								{
								char destination[LABELSIZE + LABELSIZE + LABELSIZE + LABELSIZE + 7];
								char defaultdest[LABELSIZE + LABELSIZE + LABELSIZE + LABELSIZE + 7];

								if (*Msg->GetToUser())
									{
									strcpy(defaultdest, Msg->GetToUser());
									if (*Msg->GetDestinationAddress())
										{
										strcat(defaultdest, getmenumsg(MENU_ENTER, 28));
										strcat(defaultdest, Msg->GetDestinationAddress());
										}
									else if (*Msg->GetToNodeName())
										{
										strcat(defaultdest, getmenumsg(MENU_ENTER, 28));
										strcat(defaultdest, Msg->GetToNodeName());

										if (*Msg->GetToRegion())
											{
											strcat(defaultdest, getmsg(236));
											strcat(defaultdest, Msg->GetToRegion());
											}
										}
									}
								else
									{
									*defaultdest = 0;
									}

								AskUserName(destination, defaultdest, cfg.Luser_nym, LABELSIZE + LABELSIZE + LABELSIZE + 6);

								stripansi(destination);
								if (*destination)
									{
									label ToUser, ToNode, ToRegion, ToAddress;
									parseNetAddress(destination, ToUser, ToNode, ToRegion, ToAddress);

									if (!*ToUser)
										{
										strcpy(ToUser, getmsg(386));
										}

									Msg->SetToUser(ToUser);
									Msg->SetToNodeName(ToNode);
									Msg->SetToRegion(ToRegion);
									Msg->SetDestinationAddress(ToAddress);
									}
								else
									{
									delete Msg;
									UnlockMenu(MENU_ENTER);
									return;
									}
								}
							else
								{
								// not logged in? mail only to sysop.
								Msg->SetToUser(getmsg(386));
								}
							}
						}

					if (EnterGroup)
						{
						label GroupName;

						AskGroupName(GroupName, TRUE);

						if (*GroupName)
							{
							g_slot GroupSlot;

							GroupSlot = FindGroupByPartialName(GroupName, FALSE);

							if (GroupSlot == CERROR || !CurrentUser->IsInGroup(GroupSlot))
								{
								CRmPrintf(getmsg(584), cfg.Lgroup_nym);
								delete Msg;
								UnlockMenu(MENU_ENTER);
								return;
								}

							Msg->SetGroup(GroupSlot);

							if (Massemail)
								{
								Msg->SetToUser(Msg->GetGroup());
								}
							}
						}

					makeMessage(Msg, NULL);

					OC.Echo = BOTH;
					OC.setio();

					delete Msg;
					}
				else
					{
					OutOfMemory(1);
					}

				break;
				}

			case 10: // Name
				{
				DoCommand(UC_ENTERNAME);
				break;
				}

			case 11: // Local
				{
				done = FALSE;
				EnterOld = TRUE;
				break;
				}

			case 12: // Password
				{
				DoCommand(UC_ENTERPASSWORD);
				break;
				}

			case 13: // Room
				{
				DoCommand(UC_ENTERROOM, EnterGroup);
				break;
				}

			case 14: // Title/surname
				{
				DoCommand(UC_ENTERTITLESURNAME);
				break;
				}

			case 15: // Text file
				{
				DoCommand(UC_ENTERTEXTFILE, EnterGroup);
				break;
				}

			case 16: // With protocol upload
				{
				DoCommand(UC_ENTERWITHPROTOCOL, EnterGroup);
				break;
				}

			case 17: // Exclude room
				{
				DoCommand(UC_EXCLUDEROOM);
				break;
				}

			case 18: // File-linked
				{
				if (!CurrentUser->IsSysop())
					{
					CRCRmPrintfCR(getmenumsg(MENU_ENTER, 25), cfg.Lmsgs_nym);
					UnlockMenu(MENU_ENTER);
					return;
					}

				done = FALSE;
				EnterFileLinked = TRUE;
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

				if (CurrentUser->IsExpert())
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

				SetDoWhat(ENTERMENU);

				showMenu(M_ENTOPT);
				break;
				}
			}
		} while (!done && moreYet);

	UnlockMenu(MENU_ENTER);
	}


void TERMWINDOWMEMBER EnterName(void)
	{
	if (LockMenu(MENU_ENTER))
		{
		doCR();

		SetDoWhat(ENTERNAME);

		label NewName, Buffer;
		GetStringWithBlurb(cfg.enter_name, NewName, LABELSIZE, CurrentUser->GetName(Buffer, sizeof(Buffer)), B_ENTERNYM);
		normalizeString(NewName);

		if (*NewName)
			{
			if (CurrentUser->IsSysop() || CurrentUser->IsSameName(NewName))
				{
				CurrentUser->SetName(NewName);

				if (loggedIn)
					{
                    LogTab.UpdateTable(CurrentUser, ThisLog);
					}
				}
			else
				{
				CRmPrintfCR(getmenumsg(MENU_ENTER, 30));
				}
			}

		UnlockMenu(MENU_ENTER);
		}
	}

void TERMWINDOWMEMBER EnterPassword(void)
	{
	doCR();

	if (!loggedIn)
		{
		CRmPrintfCR(getmsg(239));
		}
	else
		{
		newPW(TRUE);
		}
	}


void TERMWINDOWMEMBER EnterTitleSurname(void)
	{
	if (!LockMenu(MENU_ENTER))
		{
		return;
		}

	SetDoWhat(ENTERSUR);

	if (cfg.surnames || cfg.titles) 	// currently redundant
		{
		mPrintf(getmenumsg(MENU_ENTER, 33));

		if (!CurrentUser->IsSysop() && !cfg.entersur)
			{
			CRCRmPrintfCR(getmenumsg(MENU_ENTER, 34), cfg.Uuser_nym);
			}
		else if (!CurrentUser->IsSysop() && CurrentUser->IsSurnameLocked())
			{
			CRCRmPrintfCR(getmsg(252));
			}
		else
			{
			doCR();

			if (cfg.titles)
				{
				label NewTitle;

				// Note: getmsg(253) also in log3.cpp
				label Buffer;
				GetStringWithBlurb(getmsg(253), NewTitle, LABELSIZE, CurrentUser->GetTitle(Buffer, sizeof(Buffer)),
						B_TITLESUR);

				if (*NewTitle)
					{
					normalizeString(NewTitle);
					CurrentUser->SetTitle(NewTitle);
					}
				}

			if (cfg.surnames)
				{
				label NewSurname;

				// Note: getmsg(254) also in log3.cpp
				label Buffer;
				GetStringWithBlurb(getmsg(254), NewSurname, LABELSIZE, CurrentUser->GetSurname(Buffer, sizeof(Buffer)),
						B_TITLESUR);

				if (*NewSurname)
					{
					normalizeString(NewSurname);
					CurrentUser->SetSurname(NewSurname);
					}
				}
			}
		}

	UnlockMenu(MENU_ENTER);
	}
