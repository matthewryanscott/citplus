#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "msg.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// globalverify(): Does global sweep to verify any un-verified.

void TERMWINDOWMEMBER globalverify(void)
	{
	l_slot i;

	SetDoWhat(SYSNUSER);

	OC.SetOutFlag(OUTOK);

	label CUName;
	CurrentUser->GetName(CUName, sizeof(CUName));

	for (i = 0; (i < cfg.MAXLOGTAB) && (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
			!OC.CheckInput(FALSE, this); i++)
#else
			!OC.CheckInput(FALSE); i++)
#endif
		{
		if (LTab(i).IsInuse())
			{
			l_index logNo = LTab(i).GetLogIndex();
			LogEntry1 Log1;

			if (Log1.Load(logNo))
				{
				label Name;
				Log1.GetName(Name, sizeof(Name));

				Bool Changed = FALSE;

				if (!Log1.IsVerified())
					{
					int yn;

					char Prompt[128];
					sprintf(Prompt, getsysmsg(59), Name);

					yn = getYesNo(Prompt, 3);

					if (yn == 2)
						{
						SaveAideMess(NULL);
						return;
						}

					if (yn)
						{
						char trapstr[128];

						Changed = TRUE;

						Log1.SetVerified(TRUE);

						if (loggedIn && SameString(CUName, Name))
							{
							CurrentUser->SetVerified(TRUE);
							}

						sprintf(trapstr, getsysmsg(60), Name, CUName);

						trap(T_SYSOP, pcts, trapstr);
						amPrintf(getmsg(429), trapstr, bn);
						}
					else if (!SameString(CUName, Name))
						{
						if (getYesNo(getsysmsg(61), 0))
							{
							char trapstr[128];

							Changed = TRUE;

							CRmPrintfCR(getsysmsg(85), Name);

							// trap it
							sprintf(trapstr, getsysmsg(62), Name);
							trap(T_SYSOP, pcts, trapstr);

							amPrintf(getmsg(429), trapstr, bn);

							Log1.Clear();
							}
						}
					}
				else if (Log1.IsProblem())
					{
					int yn;

					char Prompt[128];
					sprintf(Prompt, getsysmsg(41), Name);

					yn = getYesNo(Prompt, 3);

					if (yn == 2)
						{
						SaveAideMess(NULL);
						return;
						}

					if (yn)
						{
						char trapstr[128];

						Changed = TRUE;

						Log1.SetProblem(FALSE);

						if (loggedIn && SameString(CUName, Name))
							{
							CurrentUser->SetProblem(FALSE);
							}

						sprintf(trapstr, getsysmsg(42), Name, CUName);

						trap(T_SYSOP, pcts, trapstr);
						amPrintf(getmsg(429), trapstr, bn);
						}
					else if (!SameString(CUName, Name))
						{
						if (getYesNo(getsysmsg(61), 0))
							{
							char trapstr[128];

							Changed = TRUE;

							CRmPrintfCR(getsysmsg(85), Name);

							// trap it
							sprintf(trapstr, getsysmsg(51), Name);
							trap(T_SYSOP, pcts, trapstr);

							amPrintf(getmsg(429), trapstr, bn);

							Log1.Clear();
							}
						}
					}

				if (Changed)
					{
					Log1.Save(logNo);
					}
				}
			}
		}

	SaveAideMess(NULL);
	}
