// --------------------------------------------------------------------------
// Citadel: TimeChk.H
//
// Our time-has-changed checker.

// This is written with the assumption that it will be called fairly often.
// That is, there is no provision for the case of it being called at exactly
// the same time once a day - there is no date checking. this was done for
// speed's sake.

class TimeChangeCheckerC
	{
	struct time t;

public:
	void Reset(void)
		{
		gettime(&t);

		// Ignore changes of hundredths
		t.ti_hund = 0;
		}

	TimeChangeCheckerC(void)
		{
		Reset();
		}

	Bool Check(void)
		{
		struct time t1;

		gettime(&t1);

		// Ignore changes of hundredths
		t1.ti_hund = 0;

		return (memcmp(&t, &t1, sizeof(t)));
		}
	};
