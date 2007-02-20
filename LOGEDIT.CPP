// --------------------------------------------------------------------------
// Citadel: LogEdit.CPP
//
// Userlog edit code.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "room.h"
#include "net.h"
#include "hall.h"
#include "tallybuf.h"
#include "term.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// userEdit()			Edit a user via menu


// --------------------------------------------------------------------------
// userEdit(): Edit a user via menu.

void TERMWINDOWMEMBER userEdit(void)
	{
	SetDoWhat(SYSEDIT);

	label who, Buffer;
	if (!AskUserName(who, loggedIn ? CurrentUser : NULL))
		{
		return;
		}

	const l_slot logslot = FindPersonByPartialName(who);

	if (logslot == CERROR)
		{
		CRmPrintfCR(getmsg(595), who);
		}
	else
		{
		Bool editSelf = FALSE;

		LogEntry EditLog(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
		l_index logNo = LTab(logslot).GetLogIndex();

		if (LTab(logslot).IsSameName(CurrentUser->GetName(Buffer, sizeof(Buffer))))
			{
			editSelf = TRUE;
			storeLog();
			}

		if (EditLog.Load(logNo))
			{
			CRmPrintf(getsysmsg(350), EditLog.GetName(Buffer, sizeof(Buffer)));

			if (DoUserlogEdit(&EditLog, logNo))
				{
				if (editSelf)
					{
					if (!CurrentUser->Load(logNo))
						{
						mPrintfCR(getmsg(686));
						}

                    OC.Psycho = CurrentUser->IsPsycho();

    	            label Buffer;
                    if (!TermCap->Load(CurrentUser->GetTermType(Buffer, sizeof(Buffer))))
                        {
                        mPrintfCR(getmsg(MSG_LOGIN, 40), CurrentUser->GetTermType(Buffer, sizeof(Buffer)));
                        }

					Talley->Fill();
					}

				// trap it
				char trapstr[256];
				sprintf(trapstr, getsysmsg(154), EditLog.GetName(Buffer, sizeof(Buffer)));

				if (EditLog.IsSysop())
					{
					strcat(trapstr, getsysmsg(155));
					}

				if (EditLog.IsAide())
					{
					strcat(trapstr, getsysmsg(156));
					}

				if (EditLog.IsNode())
					{
					strcat(trapstr, getsysmsg(157));
					}

				if (cfg.accounting)
					{
					if (!EditLog.IsAccounting())
						{
						strcat(trapstr, getsysmsg(158));
						}
					else
						{
						label temp;

						long C = EditLog.GetCredits() / 60;

						sprintf(temp, getmsg(373), ltoac(C), (C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);

						strcat(trapstr, temp);
						}
					}

				if (EditLog.IsPermanent())	strcat(trapstr, getsysmsg(159));
				if (EditLog.IsNetUser())	strcat(trapstr, getsysmsg(160));
				if (EditLog.IsProblem())	strcat(trapstr, getsysmsg(161));
				if (!EditLog.IsMail())		strcat(trapstr, getsysmsg(162));
				if (!EditLog.IsVerified())	strcat(trapstr, getsysmsg(163));

                trap(T_SYSOP, WindowCaption, trapstr);
				}
			}
		}
	}
