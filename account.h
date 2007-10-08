/* -------------------------------------------------------------------- */
/*	This struture has a dual purpose. It is allocated as both an array	*/
/*	to store information for each group on the system and as a single	*/
/*	instance to store information for the current caller.				*/
/*	Group information is read from GRPDATA.CIT when Citadel loads. It	*/
/*	is then used whenever a user logs onto the system: Citadel looks at */
/*	which groups the user is a member of, then assigns the user values	*/
/*	for each of these fields (well, not each: priority is not used for	*/
/*	users) based on which groups he is in. Citadel then computes all	*/
/*	accounting information based on the user's structure.               */
/*																		*/
/*	If accounting is not turned on for the system, none of this takes	*/
/*	place.																*/
/* -------------------------------------------------------------------- */
class AccountInfo
	{
	Bool	Days[7];			/* Which days this group can log-in 	*/
	Bool	Hours[24];			/* hours of the day group can log-in	*/
	Bool	SpecialHours[24];	/* Which hours are special				*/

	long	DailyIncrement; 	/* Maximum time add per day 			*/
	int		Priority;			/* What is the priority for this group	*/

	long	MaximumBalance; 	/* This groups maxtime on the system	*/

	int		DownloadMultiplier; /* Multiplier for Downloads 			*/
	int		UploadMultiplier;	/* Multiplier for Uploads				*/

	Bool	AccountingOn;		/* Does group/user have accounting? 	*/

public:
	void Clear(void)
		{
		assert(this);

		memset(this, 0, sizeof(AccountInfo));

		SetDownloadMultiplier(INT_MIN); 		// Charge max.
		SetUploadMultiplier(INT_MIN);			// Charge max.
		}

	AccountInfo(void)
		{
		assert(this);
		Clear();
		}

	void SetDay(int Day, Bool Value)
		{
		assert(this);
		assert(Day >= 0);
		assert(Day < 7);

		Days[Day] = !!Value;
		}

	void SetHour(int Hour, Bool Value)
		{
		assert(this);
		assert(Hour >= 0);
		assert(Hour < 24);

		Hours[Hour] = !!Value;
		}

	void SetSpecialHour(int Hour, Bool Value)
		{
		assert(this);
		assert(Hour >= 0);
		assert(Hour < 24);

		SpecialHours[Hour] = !!Value;
		}

	void SetDailyIncrement(long Value)
		{
		assert(this);
		DailyIncrement = Value;
		}

	void SetPriority(int Value)
		{
		assert(this);
		Priority = Value;
		}

	void SetMaximumBalance(long Value)
		{
		assert(this);
		MaximumBalance = Value;
		}

	void SetDownloadMultiplier(int Value)
		{
		assert(this);
		DownloadMultiplier = Value;
		}

	void SetUploadMultiplier(int Value)
		{
		assert(this);
		UploadMultiplier = Value;
		}

	void SetAccountingOn(Bool Value)
		{
		assert(this);
		AccountingOn = !!Value;
		}

	Bool IsDay(int Day) const
		{
		assert(this);
		assert(Day >= 0);
		assert(Day < 7);

		return (Days[Day]);
		}

	Bool IsHour(int Hour) const
		{
		assert(this);
		assert(Hour >= 0);
		assert(Hour < 24);

		return (Hours[Hour]);
		}

	Bool IsSpecialHour(int Hour) const
		{
		assert(this);
		assert(Hour >= 0);
		assert(Hour < 24);

		return (SpecialHours[Hour]);
		}

	long GetDailyIncrement() const
		{
		assert(this);
		return (DailyIncrement);
		}

	int GetPriority() const
		{
		assert(this);
		return (Priority);
		}

	long GetMaximumBalance() const
		{
		assert(this);
		return (MaximumBalance);
		}

	int GetDownloadMultiplier() const
		{
		assert(this);
		return (DownloadMultiplier);
		}

	int GetUploadMultiplier() const
		{
		assert(this);
		return (UploadMultiplier);
		}

	Bool IsAccountingOn() const
		{
		assert(this);
		return (AccountingOn);
		}
	};

class UserAccountInfo : public AccountInfo
	{
	Bool	SpecialTime;		// Is it a "special" hour

	time_t	LastTimeAccounted;	// The last time we calculated balance.

	long	BalanceAtLogin; 	// Guess.
	LogEntry *User; 			// Associated userlog entry.

public:
	void Clear(void)
		{
		assert(this);
		AccountInfo::Clear();

		SpecialTime = FALSE;
		LastTimeAccounted = time(NULL); 	// not 0
		BalanceAtLogin = 0;
		User = NULL;
		}

	UserAccountInfo(void)
		{
		assert(this);
		Clear();
		}

	void GiveDailyIncrement(void);
	void CreateAccount(void);
	Bool MayLogin(void);
	void Negotiate(LogEntry *NewUser);
	void UpdateBalance(WC_TW);
#ifdef MULTI
	void AfterTransfer(time_t StartTime, time_t EndTime, Bool Upload,
			TermWindowC *TW);
#else
	void AfterTransfer(time_t StartTime, time_t EndTime, Bool Upload);
#endif

	void SetSpecialTime(Bool Value)
		{
		assert(this);
		SpecialTime = !!Value;
		}

	Bool IsSpecialTime() const
		{
		assert(this);
		return (SpecialTime);
		}

	void SetBalanceAtLogin(long Value)
		{
		assert(this);
		BalanceAtLogin = Value;
		}

	long GetBalanceAtLogin() const
		{
		assert(this);
		return (BalanceAtLogin);
		}

	void SetLastTimeAccounted(time_t Value)
		{
		assert(this);
		LastTimeAccounted = Value;
		}

	time_t GetLastTimeAccounted() const
		{
		assert(this);
		return (LastTimeAccounted);
		}
	};

// this wonderful thing is in ACCTCFG.CPP
void ReadGrpdataCit(WC_TW);
