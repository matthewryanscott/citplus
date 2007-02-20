// --------------------------------------------------------------------------
// Citadel: MsgRead2.CPP
//
// Stuff for reading messages (secondary importance)

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// readbymsgno()		sysop fn to read by message #


// --------------------------------------------------------------------------
// readbymsgno(): Sysop fn to read by message #.

void TERMWINDOWMEMBER readbymsgno(void)
	{
	label Newest;
	CopyStringToBuffer(Newest, ltoac(MessageDat.NewestMessage()));

	CRmPrintfCR(getsysmsg(164), ltoac(MessageDat.OldestMessageInTable()), Newest);

	for (;;)
		{
		char prompt[80];
		sprintf(prompt, getsysmsg(165), cfg.Lmsg_nym);
		const m_index msgno = getNumber(prompt, 0l, LONG_MAX, -1l, FALSE, NULL);

		if (!msgno || msgno < 0)
			{
			return;
			}

		if (!MessageDat.IsValid(msgno))
			{
			CRmPrintfCR(getsysmsg(166));
			}
		else
			{
			const MaySeeMsgType MaySeeIt = CurrentUser->MaySeeMessage(msgno);
			// that const is so Richard

			if (MaySeeIt == MSM_GOOD)
				{
				dowhattype oldDowhat;

				MRO.Verbose = TRUE;

				oldDowhat = DoWhat;
				SetDoWhat(READMESSAGE);
				MRO.DotoMessage = NO_SPECIAL;

				Message *Msg = LoadMessageByID(msgno, TRUE, TRUE);
				SetDoWhat(oldDowhat);

				if (Msg)
					{
					if (MRO.DotoMessage == MARK_IT)
						{
						markIt(Msg);
						}

					if (MRO.DotoMessage == PULL_IT)
						{
						pullIt(Msg);
						}

					if (MRO.DotoMessage == CENSOR_IT)
						{
						censorIt(Msg);
						}

					delete Msg;
					}

				OC.SetOutFlag(OUTOK);
				doCR();
				}
			else
				{
				ShowWhyMayNotSeeMsg(MaySeeIt, msgno);
				}
			}
		}
	}

void TERMWINDOWMEMBER ShowWhyMayNotSeeMsg(MaySeeMsgType Why, m_index MsgID)
	{
	if (LockMessages(MSG_READMSG))
		{
		switch (Why)
			{
			case MSM_SCROLLED:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 74), ltoac(MsgID), cfg.Lmsg_nym);
				break;
				}

			case MSM_COPYSCROLLED:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 56), ltoac(MsgID), cfg.Lmsg_nym);
				break;
				}

			case MSM_CENSORED:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 57), ltoac(MsgID), cfg.Lmsg_nym);
				break;
				}

			case MSM_NOTLOGGEDIN:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 58), ltoac(MsgID));
				break;
				}

			case MSM_COPYOWNTWIT:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 59), ltoac(MsgID), cfg.Luser_nym, cfg.Lmsg_nym);
				break;
				}

			case MSM_RELEASEDORIG:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 60), ltoac(MsgID), cfg.Lmsg_nym);
				break;
				}

			case MSM_TWIT:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 61), ltoac(MsgID), cfg.Luser_nym, cfg.Lmsg_nym);
				break;
				}

			case MSM_MODERATED:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 62), ltoac(MsgID), cfg.Lmsg_nym);
				break;
				}

			case MSM_NOTINGROUP:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 63), ltoac(MsgID), cfg.Lgroup_nym);
				break;
				}

			case MSM_GROUPBAD:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 64), ltoac(MsgID), cfg.Ugroup_nym);
				break;
				}

			case MSM_MAIL:
				{
				CRmPrintfCR(getmsg(MSG_READMSG, 65), ltoac(MsgID));
				break;
				}

			default:		// This can "never" happen.
				{
				CRmPrintfCR(getmsg(419));
				break;
				}
			}

		UnlockMessages(MSG_READMSG);
		}
	else
		{
		OutOfMemory(107);
		}
	}
