// --------------------------------------------------------------------------
// Citadel: Account.CPP
//
// Accounting related routines


#include "ctdl.h"
#pragma hdrstop

#include "account.h"
#include "log.h" 
#include "group.h"
#include "cwindows.h"

// --------------------------------------------------------------------------
// Contents
//
//UserAccountInfo::...
// GiveDailyIncrement()	Increments balance according to data.
// MayLogin()			Check for user's ability to log-in now.
// Negotiate()			Determines lowest cost, highest time.
// CreateAccount() 		Sets up accounting balance for new user
// UpdateBalance()		Updates user's accounting balance
// AfterTransfer()		Calculate and adjust time after a transfer

// --------------------------------------------------------------------------
// GiveDailyIncrement(): Increments balance according to data

void UserAccountInfo::GiveDailyIncrement(void)
	{
	assert(this);
	assert(User);

	long diff;
	long numdays;

	diff = time(NULL) - User->GetCallTime();

	numdays = diff / SECSINDAY;

	if (numdays < 0)
		{
		// date was wrong - give em something for sysop's mistake
		numdays = 1;
		diff = SECSINDAY;

		// and make that the last time.
		User->SetCallTime(time(NULL) - diff);
		}

	diff -= numdays * SECSINDAY;

	// our total credits is...
	long TotalCredits =

			// startaing balance	
			User->GetCredits() +

			// and whole days
			numdays * GetDailyIncrement() +

			// and fractional days.
			(diff * GetDailyIncrement()) / SECSINDAY;

	// but not more than we can have.
	TotalCredits = min(TotalCredits, GetMaximumBalance());

	User->SetCredits(TotalCredits);

	SetBalanceAtLogin(TotalCredits);
	SetLastTimeAccounted(time(NULL));
	}


// --------------------------------------------------------------------------
// MayLogin(): Check for user's ability to log-in now.

Bool UserAccountInfo::MayLogin(void)
	{
	assert(this);
	assert(User);

	// Local and bad (already hung up) calls get no accounting.
	// i don't like this here. experiment removing?
// sure.... i didn't add it anywhere else.
//	if (!CommPort->HaveConnection() || onConsole)
//		{
//		return (TRUE);
//		}

	SetSpecialTime(IsSpecialHour(hour()));

	return (
			// free time
			IsSpecialTime() ||

			// or all of...
			(
				// Good day
				IsDay(dayofweek()) &&

				// and good hour
				IsHour(hour()) &&

				// and has time left in account
				User->GetCredits() > 0
			)
		);
	}


// --------------------------------------------------------------------------
// Negotiate(): Determines lowest cost, highest time for user.

void UserAccountInfo::Negotiate(LogEntry *NewUser)
	{
	assert(this);
	assert(NewUser);

	Clear();
	User = NewUser;

	int CurrentPriority = 0;
	g_slot groupslot;

	for (groupslot = 0; groupslot < cfg.maxgroups; groupslot++)
		{
		if (User->IsInGroup(groupslot) && AccountingData[groupslot].IsAccountingOn())
			{
			// is in a group with accounting
			SetAccountingOn(TRUE);

			if (AccountingData[groupslot].GetPriority() >= CurrentPriority)
				{
				if (AccountingData[groupslot].GetPriority() > CurrentPriority)
					{
					Clear();
					User = NewUser;

					SetAccountingOn(TRUE);
					CurrentPriority = AccountingData[groupslot].GetPriority();
					}

				int i;
				for (i = 0; i < 7; ++i)
					{
					if (AccountingData[groupslot].IsDay(i))
						{
						SetDay(i, TRUE);
						}
					}

				for (i = 0; i < 24; ++i)
					{
					if (AccountingData[groupslot].IsHour(i))
						{
						SetHour(i, TRUE);
						}

					if (AccountingData[groupslot].IsSpecialHour(i))
						{
						SetSpecialHour(i, TRUE);
						}
					}

				if (AccountingData[groupslot].GetDailyIncrement() >
						GetDailyIncrement())
					{
					SetDailyIncrement(AccountingData[groupslot].GetDailyIncrement());
					}

				if (AccountingData[groupslot].GetMaximumBalance() > GetMaximumBalance())
					{
					SetMaximumBalance(AccountingData[groupslot].GetMaximumBalance());
					}

				if (AccountingData[groupslot].GetDownloadMultiplier() >
						GetDownloadMultiplier())
					{
					SetDownloadMultiplier(AccountingData[groupslot].GetDownloadMultiplier());
					}

				if (AccountingData[groupslot].GetUploadMultiplier() >
						GetUploadMultiplier())
					{
					SetUploadMultiplier(AccountingData[groupslot].GetUploadMultiplier());
					}
				}
			}
		}
	}


// --------------------------------------------------------------------------
// CreateAccount(): Sets up accounting balance for new user

void UserAccountInfo::CreateAccount(void)
	{
	assert(this);
	assert(User);

	SetBalanceAtLogin(User->GetCredits());
	SetLastTimeAccounted(time(NULL));
	User->SetCallTime(time(NULL));
	}


// --------------------------------------------------------------------------
// UpdateBalance(): Updates user's accounting balance
//
// Note:
//	This routine will warn the user of excessive use, and terminate him when time has run out.

#ifdef MULTI
void UserAccountInfo::UpdateBalance(TermWindowC *TW)
#else
void UserAccountInfo::UpdateBalance(void)
#endif
	{
	assert(this);
	assert(User);

	if (IsSpecialHour(hour()) && !IsSpecialTime())
		{
		SetSpecialTime(TRUE);

		if (tw()loggedIn)
			{
			tw()CRmPrintfCR(getmsg(139));
			}
		}

	if (!IsSpecialHour(hour()) && IsSpecialTime())
		{
		SetSpecialTime(FALSE);

		if (tw()loggedIn)
			{
			const long C = User->GetCredits() / 60;
			tw()CRmPrintfCR(getmsg(140), tw()ltoac(C),
					(C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
			}

		SetLastTimeAccounted(time(NULL));
		}

	if (!IsSpecialTime())
		{
		long diff = time(NULL) - GetLastTimeAccounted();

		// If the time was set wrong
		if (diff < 0)
			{
			diff = 0;
			}

		User->SetCredits(User->GetCredits() - diff);
		SetLastTimeAccounted(time(NULL));

		if (!tw()CommPort->HaveConnection() || (tw()OC.whichIO == CONSOLE))
			{
			return;
			}

		if (User->GetCredits() < 5*60)
			{
			const long C = User->GetCredits() / 60;
			tw()CRmPrintfCR(getmsg(141), tw()ltoac(C),
					(C == 1) ? cfg.Lcredit_nym : cfg.Lcredits_nym);
			}

		if (!IsDay(dayofweek()) || !IsHour(hour()) || User->GetCredits() <= 0)
			{
			tw()dispBlb(B_GOODBYE);
			tw()MRO.Verbose = TRUE;
			tw()terminate(TRUE);
			}
		}
	}


// --------------------------------------------------------------------------
// AfterTransfer(): Calculate and adjust time after a transfer

#ifdef MULTI
void UserAccountInfo::AfterTransfer(time_t StartTime, time_t EndTime,
		Bool Upload, TermWindowC *TW)
#else
void UserAccountInfo::AfterTransfer(time_t StartTime, time_t EndTime,
		Bool Upload)
#endif
	{
	assert(this);
	assert(User);

	time_t TimeUsed = EndTime - StartTime;
	time_t change = TimeUsed *
			(Upload ? GetUploadMultiplier() : GetDownloadMultiplier()) / 100;

	User->SetCredits(User->GetCredits() + change);

	Bool neg = FALSE;			// is change negative?
	if (change < 0)
		{
		neg = TRUE;
		}
	else
		{
		change += TimeUsed; 	// Make less confusion: add time used
		}						// when telling them about credit

	change /= 60;
	TimeUsed /= 60;

	label TimeUsedStr;
	CopyStringToBuffer(TimeUsedStr, tw()ltoac(TimeUsed));

	tw()CRmPrintf(getmsg(145), TimeUsedStr,
			(TimeUsed == 1)	? getmsg(146)		: getmsg(147),
			(neg)			? getmsg(149)		: getmsg(150),
			tw()ltoac((neg)	? (-change)			: change),
			(change == 1)	? cfg.Lcredit_nym	: cfg.Lcredits_nym);

	SetLastTimeAccounted(time(NULL));
	}
