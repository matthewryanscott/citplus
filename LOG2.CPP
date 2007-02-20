// --------------------------------------------------------------------------
// Citadel: Log2.CPP
//
// User create and kill code

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// purgeuserlog()	From oldest to newest, prompt to kill each entry
// killuser()		sysop special to kill a log entry


// --------------------------------------------------------------------------
// purgeuserlog(): From oldest to newest, prompt to kill each entry.
//
// Notes:
//	Assumes MSG_SYSOP is locked.

void TERMWINDOWMEMBER purgeuserlog(void)
	{
	Bool done = FALSE;

	for (l_slot i = (l_slot) (cfg.MAXLOGTAB - 1); i >= 0 && !done; i--)
		{
		if (LTab(i).IsInuse())
			{
			label UserName;
			LTab(i).GetName(UserName, sizeof(UserName));

			if (!loggedIn || !CurrentUser->IsSameName(UserName))
				{
				char prompt[256];

				sprintf(prompt, getsysmsg(57), cfg.Luser_nym, UserName);

				const int response = getYesNo(prompt, 3);

				if (response == 1)
					{
					LogEntry1 Log1;

					Log1.Clear();

					if (Log1.Save(LTab(i).GetLogIndex()))
						{
                        if(!read_tr_messages())
                            {
                            errorDisp(getmsg(172));
                            done = TRUE;
                            return;
                            }
#ifdef WINCIT
                        trap(T_SYSOP, WindowCaption, gettrmsg(16), UserName);
#else
                        trap(T_SYSOP, gettrmsg(16), UserName);
#endif
                        dump_tr_messages();
						}
					}
				else if (response == 2)
					{
					done = TRUE;
					}
				}
			}
		}
	}


// --------------------------------------------------------------------------
// killuser(): Sysop special to kill a log entry.

Bool TERMWINDOWMEMBER killuser(const char *name)
	{
	label who;
	l_slot logno;
	char line[80];

	if (!name)
		{
		SetDoWhat(SYSKUSER);
		}

	if (name)
		{
		CopyStringToBuffer(who, name);
		}
	else
		{
		getNormStr(cfg.Luser_nym, who, LABELSIZE);
		}

	if (!*who)
		{
		return (FALSE);
		}

	logno = FindPersonByPartialName(who);

	if (logno == CERROR)
		{
		if (!name)
			{
			mPrintfCR(getmsg(595), who);
			}

		return (FALSE);
		}

	LTab(logno).GetName(who, sizeof(who));

	if (loggedIn && CurrentUser->IsSameName(who))
		{
		if (!name)
			{
			mPrintfCR(getsysmsg(83));
			}

		return (FALSE);
		}

	sprintf(line, getsysmsg(84), cfg.Luser_nym, who);

	if (!name && !getYesNo(line, 0))
		{
		return (FALSE);
		}
	else
		{
		LogEntry1 Log1;

		if (!name)
			{
			CRmPrintfCR(getsysmsg(85), who);
			}

		Log1.Clear();
		if (Log1.Save(LTab(logno).GetLogIndex()))
			{
            if(!(read_tr_messages()))
                {
                errorDisp(getmsg(172));
                return(FALSE);
                }

#ifdef WINCIT
            trap(T_SYSOP, WindowCaption, gettrmsg(16), who);
#else
            trap(T_SYSOP, gettrmsg(16), who);
#endif
            dump_tr_messages();
			}

		return (TRUE);
		}
	}
