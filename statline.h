// --------------------------------------------------------------------------
// Citadel: StatLine.h
//
// All about mighty Mr. Status Line

enum StatusLineStateE
	{
	SL_ONELINE, 		SL_TWOLINES,		SL_FULLSCREEN,
	};

class StatusLineC
	{
	StatusLineStateE State; 		// 1, 2, or full-screen
	Bool Visible;					// Blanked?

	time_t HelpTimeout; 			// Time of take-down (0 = Not up)

public:
	StatusLineC(void)
		{
		State = SL_ONELINE;
		Visible = FALSE;
		HelpTimeout = 0;
		};

	Bool IsVisible(void) const
		{
		return (Visible);
		}

	int Height(void) const
		{
		int h = 0;

		if (Visible)
			{
			h = 1;

			if (HelpTimeout)
				{
				h += 4;
				}

			if (State == SL_TWOLINES)
				{
				h++;
				}
			}

		return (h);
		}

	StatusLineStateE GetState(void) const
		{
		return (State);
		}

	Bool IsFullScreen(void) const
		{
		return (State == SL_FULLSCREEN);
		}

	void Update(WC_TW);
	void ToggleHelp(WC_TW);
	void Toggle(WC_TW);
	void ToggleSecond(WC_TW);
	Bool ToggleFullScreen(WC_TW);
	};
