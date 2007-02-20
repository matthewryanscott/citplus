// --------------------------------------------------------------------------
// Citadel: TermWndw.H
//
// For stuff that had to deal explicityly with term windows

#ifdef MULTI
    struct TermWindowListS
        {
        TermWindowListS *next;
        TermWindowC *T;
        };

	class TermWindowCollectionC
		{
		HANDLE OurMutex;
		TermWindowListS *List;

		TermWindowC *Focus;

		void Lock(void)
			{
			WaitForSingleObject(OurMutex, INFINITE);
			}

		void Unlock(void)
			{
			ReleaseMutex(OurMutex);
			}

	public:
		TermWindowCollectionC(void)
			{
			OurMutex = CreateMutex(NULL, FALSE, NULL);
			List = NULL;
			Focus = NULL;
			}
	
		~TermWindowCollectionC()
			{
			Lock();
	
			disposeLL((void **) &List);
	
			if (OurMutex != INVALID_HANDLE_VALUE)
				{
				CloseHandle(OurMutex);
				}
			}
	
		void Add(TermWindowC *New);
		void Remove(TermWindowC *Old);

		int EventToUser(const char *User, Bool HideFromConsole, const char *Event);

		void FocusPrev(void);
		void FocusNext(void);

		Bool InFocus(const TermWindowC *Check) const
			{
			return (Check == Focus);
			}

		void SetFocus(TermWindowC *NewFocus);
		void SetFocusInt(int NewFocus);

		PCHAR_INFO GetFocusedScreen(void)
			{
			return (Focus ? Focus->ScreenBuffer : NULL);
			}

		void SetForKill(int ToKill);
		void KillAll(void);

		int FocusedCursorRow(void)
			{
			return (Focus ? Focus->logiRow : 0);
			}

		int FocusedCursorCol(void)
			{
			return (Focus ? Focus->logiCol : 0);
			}

		Bool AnyExist(void)
			{
			return (List != NULL);
			}

		ulong Count(void)
			{
			Lock();
			ulong cnt = getLLCount(List);
			Unlock();

			return (cnt);
			}

		const TermWindowC *Peek(ulong Id)
			{
			Lock();
			TermWindowListS *cur = (TermWindowListS *) getLLNum(List, Id);
			Unlock();
			return (cur ? cur->T : NULL);
			}

		void SystemEvent(SystemEventE Type, int Data, void *MoreData, Bool HideFromConsole, const char *Event);
		Bool IsUserLoggedIn(const char *Name);
		long FindConnSeqByThreadId(DWORD ThreadId);
		void PingAll(void);
		};

extern TermWindowCollectionC TermWindowCollection;
#endif
