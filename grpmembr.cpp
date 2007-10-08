// --------------------------------------------------------------------------
// Citadel: GrpMembr.CPP
//
// Code edit group membership.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "msg.h"
#include "group.h"
#include "boolexpr.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// groupfunc()		aide fn to add/remove group members
// globalgroup()	sysop fn to add/remove group members (global)
// globaluser() 	sysop fn to add/remove user from groups (global)



// --------------------------------------------------------------------------
// OperatorSpecificMembership()
//
// Notes:
//	Assumes MSG_SYSOP is locked.

void TERMWINDOWMEMBER OperatorSpecificMembership(void)
	{
	char String[256], Prompt[128];

	sprintf(Prompt, getsysmsg(90), cfg.Lgroup_nym);

	doCR();

	if (!AskGroupName(String, ns, Prompt, 255))
		{
		return;
		}

	BoolExpr Expression;
#ifdef WINCIT
	if (!CreateBooleanExpression(String, TestGroupOrMetaExists, Expression, this))
#else
	if (!CreateBooleanExpression(String, TestGroupOrMetaExists, Expression))
#endif
		{
		CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
		return;
		}

	label groupname;
	int add;

	switch (toupper(DoMenuPrompt(pctss, getsysmsg(257))))
		{
		case 'A':
			{
			mPrintfCR(getmsg(358));
			add = 1;
			break;
			}

//		case 'B':
//		case 10:
//		case 13:
		default:
			{
			mPrintfCR(getsysmsg(258));
			add = 0;
			break;
			}

		case 'R':
			{
			mPrintfCR(getmsg(359));
			add = 2;
			break;
			}

		case '?':
			{
			showMenu(M_GRPGLOB);
			return;
			}
		}

	if (!AskGroupName(groupname, ns))
		{
		return;
		}

	g_slot groupslot = FindGroupByPartialName(groupname, FALSE);

	if (groupslot != CERROR && GroupData[groupslot].IsHidden() && !CurrentUser->IsInGroup(groupslot))
		{
		groupslot = CERROR;
		}

	if (groupslot == CERROR)
		{
		CRmPrintfCR(getmsg(157), cfg.Lgroup_nym, groupname);
		return;
		}

	const GroupEntry *GE = &GroupData[groupslot];
	label GEName;
	GE->GetName(GEName, sizeof(GEName));

// Only sysops can get to this function!
//	if (GE->IsLocked() && !CurrentUser->IsSysop())
//		{
//		CRmPrintf(getmsg(581), cfg.Ugroup_nym);
//		return;
//		}

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		OC.SetOutFlag(OUTOK);

#ifdef MULTI
		if (OC.CheckInput(FALSE, this))
#else
		if (OC.CheckInput(FALSE))
#endif
			{
			break;
			}

		OC.SetOutFlag(IMPERVIOUS);

		if (LTab(i).IsInuse())
			{
			LogEntry2 Log2(cfg.maxgroups);

			const l_index logNo = LTab(i).GetLogIndex();

			if (Log2.Load(logNo))
				{
				L2FGT = &Log2;
#ifdef WINCIT
				SlotForGroupTester = LogOrder[i];
#else
				SlotForGroupTester = i;
#endif

				if (EvaluateBooleanExpression(Expression, GroupWithMetaTester))
					{
					const Bool StartedInGroup = Log2.IsInGroup(groupslot);

					if (add == 0 || (StartedInGroup && add == 2) || (!StartedInGroup && add == 1))
						{
						label UN;
						LTab(i).GetName(UN, sizeof(UN));

						char query[128];
						int yn;

						sprintf(query, StartedInGroup ? getmsg(580) : getmsg(579), UN, cfg.Lgroup_nym, GEName);

						yn = getYesNo(query, 3);
						if (yn == 2)
							{
							SaveAideMess(GEName);
							return;
							}

						if (yn)
							{
							label Name;
							CurrentUser->GetName(Name, sizeof(Name));

							Log2.SetInGroup(groupslot, !StartedInGroup);

#ifdef WINCIT
                            trap(T_SYSOP, WindowCaption, StartedInGroup ? getmsg(583) : getmsg(582), UN, GEName, Name);
#else
							trap(T_SYSOP, StartedInGroup ? getmsg(583) : getmsg(582), UN, GEName, Name);
#endif
							char MsgStr[256];
							sprintf(MsgStr, StartedInGroup ? getmsg(456) : getmsg(455), UN, cfg.Lgroup_nym, GEName, Name);

							amPrintf(getmsg(429), MsgStr, bn);

							Log2.Save(logNo);

							// see if it is us
							if (loggedIn && CurrentUser->IsSameName(UN))
								{
								CurrentUser->SetInGroup(groupslot, !StartedInGroup);
								}
							}
						}
					}
				}
			}
		}

	SaveAideMess(GEName);
	}


// --------------------------------------------------------------------------
// groupfunc(): Aide fn to add/remove group members.

void TERMWINDOWMEMBER groupfunc(const char *grpName, const char *userName, Bool addToGroup)
	{
	label who;
	label groupname;
	g_slot groupslot;
	char line[128];

	if (!grpName)
		{
		AskGroupName(groupname, ns);
		}
	else
		{
		CopyStringToBuffer(groupname, grpName);
		}

	if (!*groupname)
		{
		return;
		}

	// If grpName specified, ignore user being in the group
	groupslot = FindGroupByPartialName(groupname, !!grpName);

	if (groupslot != CERROR && !grpName)
		{
		if ((GroupData[groupslot].IsLocked() && !CurrentUser->IsSysop())
			|| (GroupData[groupslot].IsHidden() && !CurrentUser->IsInGroup(groupslot) &&
			!(onConsole || CurrentUser->IsSuperSysop())))
			{
			groupslot = CERROR;
			}
		}

	if (groupslot == CERROR)
		{
		if (grpName)
			{
			CRmPrintf(getmsg(157), cfg.Lgroup_nym, grpName);
			}
		else
			{
			CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
			}
		}
	else
		{
		if (!userName)
			{
			AskUserName(who, loggedIn ? CurrentUser : NULL);
			}
		else
			{
			CopyStringToBuffer(who, userName);
			}

		l_slot logNo = FindPersonByPartialName(who);

		if (logNo == CERROR || !*who)
			{
			if (*who)
				{
				CRmPrintfCR(getmsg(595), who);
				}

			return;
			}

		LogEntry2 Log2(cfg.maxgroups);
		if (!Log2.Load(LTab(logNo).GetLogIndex()))
			{
			return;
			}

		LTab(logNo).GetName(who, sizeof(who));
		GroupData[groupslot].GetName(groupname, sizeof(groupname));

		const Bool StartedInGroup = Log2.IsInGroup(groupslot);

		sprintf(line, StartedInGroup ? getmsg(580) : getmsg(579), who, cfg.Lgroup_nym, groupname);

		if ((userName && (addToGroup != StartedInGroup)) || (!userName && getYesNo(line, 0)))
			{
			Log2.SetInGroup(groupslot, !StartedInGroup);

			if (!userName)
				{
				label UserName;
				CurrentUser->GetName(UserName, sizeof(UserName));

#ifdef WINCIT
                trap(T_AIDE, WindowCaption, StartedInGroup ? getmsg(583) : getmsg(582), who, groupname, UserName);
#else
				trap(T_AIDE, StartedInGroup ? getmsg(583) : getmsg(582), who, groupname, UserName);
#endif

				Message *Msg = new Message;

				if (Msg)
					{
					Msg->SetGroup(groupname);
					Msg->SetRoomNumber(AIDEROOM);

					char MsgStr[256];
					sprintf(MsgStr, StartedInGroup ? getmsg(456) : getmsg(455), who, cfg.Lgroup_nym, groupname, UserName);
					Msg->SetText(MsgStr);
					systemMessage(Msg);

					delete Msg;
					}
				else
					{
					OutOfMemory(52);
					}
				}

			Log2.Save(LTab(logNo).GetLogIndex());

			// see if it is us
			if (loggedIn && CurrentUser->IsSameName(who))
				{
				CurrentUser->SetInGroup(groupslot, !StartedInGroup);
				}
			}
		}
	}


// --------------------------------------------------------------------------
// globalgroup(): Sysop fn to add/remove group members (global).
//
// Notes:
//	Assumes MSG_SYSOP is locked.

void TERMWINDOWMEMBER globalgroup(const char *theGroup, Bool addAll)
	{
	label groupname;
	int add;

	if (theGroup && *theGroup)
		{
		if (addAll)
			{
			add = 1;
			}
		else
			{
			add = 0;	// both
			}

		CopyStringToBuffer(groupname, theGroup);
		}
	else
		{
		mPrintf(pctss, getsysmsg(257));

		switch (toupper(iCharNE()))
			{
			case 'A':
				{
				mPrintfCR(getmsg(358));
				add = 1;
				break;
				}

//			case 'B':
//			case 10:
//			case 13:
			default:
				{
				mPrintfCR(getsysmsg(258));
				add = 0;
				break;
				}

			case 'R':
				{
				mPrintfCR(getmsg(359));
				add = 2;
				break;
				}

			case '?':
				{
				showMenu(M_GRPGLOB);
				return;
				}
			}

		if (!AskGroupName(groupname, ns))
			{
			return;
			}
		}

	g_slot groupslot = FindGroupByPartialName(groupname, FALSE);

	if (groupslot != CERROR && GroupData[groupslot].IsHidden() && !CurrentUser->IsInGroup(groupslot))
		{
		groupslot = CERROR;
		}

	if (groupslot == CERROR)
		{
		CRmPrintfCR(getmsg(157), cfg.Lgroup_nym, groupname);
		return;
		}

	const GroupEntry *GE = &GroupData[groupslot];
	label GEName;
	GE->GetName(GEName, sizeof(GEName));

	if (GE->IsLocked() && !CurrentUser->IsSysop())
		{
		CRmPrintf(getmsg(581), cfg.Ugroup_nym);
		return;
		}

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		OC.SetOutFlag(OUTOK);

#ifdef MULTI
		if (OC.CheckInput(FALSE, this))
#else
		if (OC.CheckInput(FALSE))
#endif
			{
			break;
			}

		OC.SetOutFlag(IMPERVIOUS);

		if (LTab(i).IsInuse())
			{
			LogEntry2 Log2(cfg.maxgroups);

			const l_index logNo = LTab(i).GetLogIndex();

			if (Log2.Load(logNo))
				{
				const Bool StartedInGroup = Log2.IsInGroup(groupslot);

				if (add == 0 || (StartedInGroup && add == 2) || (!StartedInGroup && add == 1))
					{
					label UN;
					LTab(i).GetName(UN, sizeof(UN));

					int yn;

					if (!addAll)
						{
						char query[128];

						sprintf(query, StartedInGroup ? getmsg(580) : getmsg(579), UN, cfg.Lgroup_nym, GEName);

						yn = getYesNo(query, 3);
						if (yn == 2)
							{
							SaveAideMess(GEName);
							return;
							}
						}

					if (addAll || yn)
						{
						label Name;
						CurrentUser->GetName(Name, sizeof(Name));

						Log2.SetInGroup(groupslot, !StartedInGroup);

#ifdef WINCIT
                        trap(T_SYSOP, WindowCaption, StartedInGroup ? getmsg(583) : getmsg(582), UN, GEName, Name);
#else
						trap(T_SYSOP, StartedInGroup ? getmsg(583) : getmsg(582), UN, GEName, Name);
#endif



						char MsgStr[256];
						sprintf(MsgStr, StartedInGroup ? getmsg(456) : getmsg(455), UN, cfg.Lgroup_nym, GEName, Name);
						amPrintf(getmsg(429), MsgStr, bn);

						Log2.Save(logNo);

						// see if it is us
						if (loggedIn && CurrentUser->IsSameName(UN))
							{
							CurrentUser->SetInGroup(groupslot, !StartedInGroup);
							}
						}
					}
				}
			}
		}

	SaveAideMess(GEName);
	}


// --------------------------------------------------------------------------
// globaluser(): Sysop function to add/remove user from groups (global).

void TERMWINDOWMEMBER globaluser(void)
	{
	label who;
	l_slot logNo;

	label Buffer;
	if (!AskUserName(who, loggedIn ? CurrentUser : NULL))
		{
		return;
		}

	logNo = FindPersonByPartialName(who);

	if (logNo == CERROR)
		{
		CRmPrintfCR(getmsg(595), who);
		return;
		}

	Bool done;
	Bool Changed = FALSE;
	g_slot GroupSlot;
	LogEntry2 Log2(cfg.maxgroups);

	if (Log2.Load(LTab(logNo).GetLogIndex()))
		{
		LTab(logNo).GetName(who, sizeof(who));

		for (done = FALSE, GroupSlot = 0; !done && GroupSlot < cfg.maxgroups; GroupSlot++)
			{
			const GroupEntry *GE = &GroupData[GroupSlot];
			label GEName;
			GE->GetName(GEName, sizeof(GEName));

			if (GE->IsInuse() && (!GE->IsLocked() || CurrentUser->IsSysop()) &&
					(!GE->IsHidden() || CurrentUser->IsInGroup(GroupSlot)))
				{
				const Bool StartedInGroup = Log2.IsInGroup(GroupSlot);

				char query[128];
				int yn;

				sprintf(query, StartedInGroup ? getmsg(580) : getmsg(579), who, cfg.Lgroup_nym, GEName);

				if ((yn = getYesNo(query, 3)) == 2)
					{
					done = TRUE;
					}
				else if (yn == 1)
					{
					Changed = TRUE;

					Log2.SetInGroup(GroupSlot, !StartedInGroup);

					label Name;
					CurrentUser->GetName(Name, sizeof(Name));

#ifdef WINCIT
                    trap(T_SYSOP, WindowCaption, StartedInGroup ? getmsg(583) : getmsg(582), who, GEName, Name);
#else
					trap(T_SYSOP, StartedInGroup ? getmsg(583) : getmsg(582), who, GEName, Name);
#endif

					char MsgStr[256];
					sprintf(MsgStr, StartedInGroup ? getmsg(456) : getmsg(455), who, cfg.Lgroup_nym, GEName, Name);

					amPrintf(getmsg(429), MsgStr, bn);
					}
				}
			}

		if (Changed)
			{
			if (Log2.Save(LTab(logNo).GetLogIndex()))
				{
				// see if it is us
				if (loggedIn && (logNo == ThisSlot))
					{
					CurrentUser->LogEntry2::Load(ThisLog);
					}
				}
			}
		}

	SaveAideMess(GroupData[SPECIALSECURITY].GetName(Buffer, sizeof(Buffer)));
	}

#ifdef WINCIT
short TestGroupExists(const char *TestString, TermWindowC *TW)
#else
short TestGroupExists(const char *TestString)
#endif
	{
	return (tw()FindGroupByPartialName(TestString, FALSE));
	}

// yes, this is a bit of a kludge. cope.

#define META_SYSOP		32767
#define META_AIDE		32766
#define META_NODE		32765
#define META_UNLISTED	32764
#define META_TWIT		32763
#define META_PERMANENT	32762
#define META_NETWORK	32761
#define META_NOMAIL 	32760

#ifdef WINCIT
short TestGroupOrMetaExists(const char *TestString, TermWindowC *TW)
#else
short TestGroupOrMetaExists(const char *TestString)
#endif
	{
	if (SameString(TestString, getmsg(658)))
		{
		return (META_SYSOP);
		}

	if (SameString(TestString, getmsg(659)))
		{
		return (META_AIDE);
		}

	if (SameString(TestString, getmsg(660)))
		{
		return (META_NODE);
		}

	if (SameString(TestString, getmsg(554)))
		{
		return (META_UNLISTED);
		}

	if (SameString(TestString, getmsg(593)))
		{
		return (META_TWIT);
		}

	if (SameString(TestString, getmsg(594)))
		{
		return (META_PERMANENT);
		}

	if (SameString(TestString, getmsg(673)))
		{
		return (META_NETWORK);
		}

	if (SameString(TestString, getmsg(674)))
		{
		return (META_NOMAIL);
		}

	return (tw()FindGroupByPartialName(TestString, FALSE));
	}

#ifdef WINCIT
void ShowGroup(g_slot Group, TermWindowC *TW)
#else
void ShowGroup(g_slot Group)
#endif
	{
	label Buffer;
	tw()mPrintf(getmsg(306), GroupData[Group].GetName(Buffer, sizeof(Buffer)));
	}

#ifdef WINCIT
void ShowGroupWithMeta(g_slot Group, TermWindowC *TW)
#else
void ShowGroupWithMeta(g_slot Group)
#endif
	{
	switch (Group)
		{
		case META_SYSOP:
			{
			tw()mPrintf(getmsg(306), getmsg(658));
			break;
			}

		case META_AIDE:
			{
			tw()mPrintf(getmsg(306), getmsg(659));
			break;
			}

		case META_NODE:
			{
			tw()mPrintf(getmsg(306), getmsg(660));
			break;
			}

		case META_UNLISTED:
			{
			tw()mPrintf(getmsg(306), getmsg(554));
			break;
			}

		case META_TWIT:
			{
			tw()mPrintf(getmsg(306), getmsg(593));
			break;
			}

		case META_PERMANENT:
			{
			tw()mPrintf(getmsg(306), getmsg(594));
			break;
			}

		case META_NETWORK:
			{
			tw()mPrintf(getmsg(306), getmsg(673));
			break;
			}

		case META_NOMAIL:
			{
			tw()mPrintf(getmsg(306), getmsg(674));
			break;
			}

		default:
			{
			label Buffer;
			tw()mPrintf(getmsg(306), GroupData[Group].GetName(Buffer,
					sizeof(Buffer)));
			break;
			}
		}
	}

Bool GroupNameTester(g_slot GroupSlot)
	{
	return (GroupData[GroupSlot].IsSameName(GroupNameForNameTester));
	}

Bool GroupTester(g_slot GroupSlot)
	{
	return (Log2ForGroupTester->IsInGroup(GroupSlot));
	}

Bool GroupWithMetaTester(g_slot GroupSlot)
	{
	switch (GroupSlot)
		{
		case META_SYSOP:
			{
			return (LogTab[SlotForGroupTester].IsSysop());
			}

		case META_AIDE:
			{
			return (LogTab[SlotForGroupTester].IsAide());
			}

		case META_NODE:
			{
			return (LogTab[SlotForGroupTester].IsNode());
			}

		case META_UNLISTED:
			{
			return (LogTab[SlotForGroupTester].IsUnlisted());
			}

		case META_TWIT:
			{
			return (LogTab[SlotForGroupTester].IsProblem());
			}

		case META_PERMANENT:
			{
			return (LogTab[SlotForGroupTester].IsPermanent());
			}

		case META_NETWORK:
			{
			return (LogTab[SlotForGroupTester].IsNetUser());
			}

		case META_NOMAIL:
			{
			return (!LogTab[SlotForGroupTester].IsMail());
			}

		default:
			{
			return (Log2ForGroupTester->IsInGroup(GroupSlot));
			}
		}
	}

void TERMWINDOWMEMBER AideGroup(void)
	{
	doCR();
	SetDoWhat(AIDEGROUP);

	groupfunc(NULL, NULL, 0);
	}
