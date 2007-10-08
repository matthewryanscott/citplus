// --------------------------------------------------------------------------
// Citadel: ReadLog.CPP
//
// .Read Userlog stuff

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "group.h"
#include "boolexpr.h"


// --------------------------------------------------------------------------
// Contents
//
// ReadUserlog()	handles read userlog


// --------------------------------------------------------------------------
// ReadUserlog(): Handles .Read Userlog (.RU) and show-users-on-login.

void TERMWINDOWMEMBER ReadUserlog(Bool revOrder, OldNewPick whichMess, long howMany)
	{
	l_slot CurrentLogSlot;
	Bool wild = FALSE;
	Bool buser = FALSE;
	int step;
	Bool wow = FALSE;

	SetDoWhat(READUSERLOG);

	if ((cfg.readLog < 0) || (cfg.readLog == 0 && !CurrentUser->IsSysop()) || (cfg.readLog == 1 && !CurrentUser->IsAide()))
		{
		CRmPrintfCR(getmsg(58), cfg.Luser_nym);
		return;
		}

	if (!(cfg.readluser || CurrentUser->IsSysop() || CurrentUser->IsAide()))
		{
		rf.Group = FALSE;
		}

	rf.Text = FALSE;

	if (!ProcessReadFilter(&rf, CurrentUser->IsAide()))
		{
		return;
		}

	if (rf.User)
		{
		if (FindPersonByPartialName(rf.SearchUser) == CERROR)
			{
			if (strpos('?',rf.SearchUser) || strpos('*',rf.SearchUser) || strpos('[',rf.SearchUser))
				{
				wild = TRUE;
				}
			else
				{
				CRmPrintfCR(getmsg(595), rf.SearchUser);
				return;
				}
			}
		else
			{
			LTab(FindPersonByPartialName(rf.SearchUser)).GetName(rf.SearchUser, sizeof(rf.SearchUser));
			buser = TRUE;
			}
		}

	OC.SetOutFlag(OUTOK);

	if (!CurrentUser->IsExpert())
		{
		CRmPrintfCR(getmsg(577));
		}

	if (!revOrder)
		{
		step = 1;
		CurrentLogSlot = 0;
		}
	else
		{
		step = -1;
		CurrentLogSlot = (l_slot) (cfg.MAXLOGTAB - 1);
		}

	for (; ((CurrentLogSlot < cfg.MAXLOGTAB) && (OC.User.GetOutFlag() != OUTSKIP) && (CurrentLogSlot >= 0));
			CurrentLogSlot = (l_slot) (CurrentLogSlot + step))
		{
#ifdef MULTI
		if (OC.CheckInput(FALSE, this))
#else
		if (OC.CheckInput(FALSE))
#endif
			{
			return;
			}

		if (LTab(CurrentLogSlot).IsInuse())
			{
			// .RNU .ROU
			if (whichMess == NewOnly && LTab(CurrentLogSlot).GetCallNumber() <=
					(loggedIn ? CurrentUser->GetCallNumber() : last_console_login_callno))
				{
				continue;
				}

			if (whichMess == OldOnly && LTab(CurrentLogSlot).GetCallNumber() >=
					(loggedIn ? CurrentUser->GetCallNumber() : last_console_login_callno))
				{
				continue;
				}

			label LTName;
			LTab(CurrentLogSlot).GetName(LTName, sizeof(LTName));

			if (buser && !SameString(rf.SearchUser, LTName))
				{
				continue;
				}

			if (wild && !u_match(deansi(LTName), rf.SearchUser))
				{
				continue;
				}

			if (rf.Group)
				{
				// We have to actually look at the user's info
				LogEntry2 Log2(cfg.maxgroups);

				if (Log2.Load(LTab(CurrentLogSlot).GetLogIndex()))
					{
					L2FGT = &Log2;
#ifdef WINCIT
					SlotForGroupTester = LogOrder[CurrentLogSlot];
#else
					SlotForGroupTester = CurrentLogSlot;
#endif

					if (!EvaluateBooleanExpression(rf.SearchGroup, GroupWithMetaTester))
						{
						// not "really" in the group - check to see if this
						// is the #SYSOP...

						if (LTab(CurrentLogSlot).IsMainSysop())
							{
							// so this is the #SYSOP, show onlyif set to show #SYSOP's groupiness or user is #SYSOP
							if (cfg.showSysop || (loggedIn && CurrentUser->IsMainSysop()))
								{
								for (g_slot g = 0; g < cfg.maxgroups; g++)
									{
									Log2.SetInGroup(g, TRUE);
									}

								if (!EvaluateBooleanExpression(rf.SearchGroup, GroupWithMetaTester))
									{
									continue;
									}
								}
							else
								{
								continue;
								}
							}
						else
							{
							continue;
							}
						}
					}
				else
					{
					mPrintf(getmsg(83), log2Dat);
					continue;
					}
				}

			// If this is user's account, user is an Aide, or accout is not unlised
			if ((!CurrentLogSlot && loggedIn) || CurrentUser->IsAide() || !LTab(CurrentLogSlot).IsUnlisted())
				{
				LogEntry1 Log1;

				if (Log1.Load(LTab(CurrentLogSlot).GetLogIndex()))
					{
					label L1Name;
					Log1.GetName(L1Name, sizeof(L1Name));

					if (MRO.Verbose)
						{
						char dtstr[80];

						char DS[64];
						strftime(dtstr, 79, (loggedIn) ? CurrentUser->GetVerboseDateStamp(DS, sizeof(DS)) : cfg.vdatestamp,
								Log1.GetCallTime());

						if ((cfg.surnames || cfg.titles) && CurrentUser->IsViewTitleSurname())
							{
							if (wow)
								{
								doCR();
								}

							wow = TRUE;
							doCR();

							label Buffer;
							if (*Log1.GetTitle(Buffer, sizeof(Buffer)))
								{
								mPrintf(getmsg(93), Buffer);
								}

							mPrintf(getmsg(75), L1Name);

							if (*Log1.GetSurname(Buffer, sizeof(Buffer)))
								{
								mPrintf(getmsg(93), Buffer);
								}

							CRmPrintf(getmsg(70), ltoac(Log1.GetCallNumber()), dtstr);
							}
						else
							{
							CRmPrintf(getmsg(68), LABELSIZE + (strlen(L1Name) - strlen(deansi(L1Name))),
									L1Name, ltoac(Log1.GetCallNumber()), dtstr);
							}
						}
					else
						{
						doCR();

						if (CurrentUser->IsAide() || Log1.IsNode())
							{
							mPrintf(getmsg(67), LABELSIZE + (strlen(L1Name) - strlen(deansi(L1Name))), L1Name);
							}
						else
							{
							mPrintf(getmsg(56), L1Name);
							}
						}

					// A>ide T>wit P>erm N>etuser U>nlist S>ysop
					if (CurrentUser->IsAide())
						{
						label flags;

						if (cfg.accounting && MRO.Verbose)
							{
							if (Log1.IsAccounting())
								{
								mPrintf(getmsg(53), Log1.GetCredits() / 60);
								}
							else
								{
								mPrintf(getmsg(55), getmsg(54));
								}
							}

						strcpy(flags, getmsg(226));

						if (Log1.IsAide())			flags[0] = 'A';
						if (Log1.IsProblem())		flags[1] = 'T';
						if (Log1.IsPermanent()) 	flags[2] = 'P';
						if (Log1.IsNetUser())		flags[3] = 'N';
						if (Log1.IsUnlisted())		flags[4] = 'U';
						if (Log1.IsSysop()) 		flags[5] = 'S';
						if (!Log1.IsMail()) 		flags[6] = 'M';
						if (!Log1.IsVerified()) 	flags[7] = 'V';
						if (Log1.IsDungeoned()) 	flags[8] = 'D';

						mPrintf(spcts, flags);
						}

					if (Log1.IsNode())
						{
						mPrintf(getmsg(508));
						}

					if (howMany)
						{
						if (!--howMany)
							{
							break;
							}
						}
					}
				}
			}

		if (MRO.DotoMessage == REVERSE_READ)
			{
			MRO.DotoMessage = NO_SPECIAL;
			OC.SetOutFlag(OUTOK);

			step = -step;
			whichMess = OldAndNew;
			mPrintf(getmsg(357), (step == 1) ? '+' : '-');
			howMany = 0;
			}
		}

	doCR();
	}
