// --------------------------------------------------------------------------
// Citadel: DoMenu.CPP
//
// Handles the menu processing

#include "ctdl.h"
#pragma hdrstop

#include "domenu.h"

MenuTrackerS Menus[MENU_NUM];

struct MenuFormatS
	{
	short Key;
	short MIndex;
	short CIndex;
	};

// --------------------------------------------------------------------------
// DoMenu(): Finds a menu key
//
// Return value:
//	0	Not found
//	-1	ESC
//	-2	?
//	pos	Index of selected option
//
// For menu text...
//	%0	cfg.Ugroup_nym
//	%1	cfg.Uroom_nym
//	%2	cfg.Uuser_nym
//	%3	cfg.Umsg_nym
//	%4	cfg.Umsgs_nym
//	%5	cfg.Lgroup_nym
//	%6	cfg.Lroom_nym
//	%7	cfg.Luser_nym
//	%8	cfg.Lmsg_nym
//	%9	cfg.Lmsgs_nym
//	%A	cfg.Uhall_nym
//	%B	cfg.Lhall_nym
//	%C	cfg.Uhalls_nym
//	%D	cfg.Urooms_nym


int TERMWINDOWMEMBER DoMenu(MenuE WhichMenu, int letter)
	{
	if (letter == '?')
		{
		return (-2);
		}

	if (letter == ESC)
		{
		return (-1);
		}

	if (WhichMenu < 0 || WhichMenu >= MENU_NUM)
		{
		assert(FALSE);
		return (0);
		}

#ifdef WINCIT
	WaitForSingleObject(MenuMutex, INFINITE);
#endif

	assert(Menus[WhichMenu].UseCounter);

	const MenuFormatS *Menu = (const MenuFormatS *) Menus[WhichMenu].Menu->next->data;
	const char **Msgs = (const char **) Menus[WhichMenu].Menu->aux;

	int retVal = 0;

	for (int i = 0; !retVal && Menu[i].Key; i++)
		{
		if (toupper(letter) == toupper(Menu[i].Key))
			{
			char Str[128];

			sformat(Str, Msgs[Menu[i].MIndex], "0123456789ABCD",
					cfg.Ugroup_nym, cfg.Uroom_nym, cfg.Uuser_nym, cfg.Umsg_nym, cfg.Umsgs_nym,
					cfg.Lgroup_nym, cfg.Lroom_nym, cfg.Luser_nym, cfg.Lmsg_nym, cfg.Lmsgs_nym,
					cfg.Uhall_nym, cfg.Lhall_nym, cfg.Uhalls_nym, cfg.Urooms_nym);

			mPrintf(pcts, Str);
			retVal = Menu[i].CIndex;
			}
		}

#ifdef WINCIT
	ReleaseMutex(MenuMutex);
#endif

	return (retVal);
	}


void DumpUnlockedMenus(void)
	{
#ifdef WINCIT
	WaitForSingleObject(MenuMutex, INFINITE);
#endif

	for (int i = 0; i < MENU_NUM; i++)
		{
		if (!Menus[i].UseCounter && Menus[i].Menu)
			{
			discardData(Menus[i].Menu);
			Menus[i].Menu = NULL;
			Menus[i].Messages = NULL;
			}
		}

#ifdef WINCIT
	ReleaseMutex(MenuMutex);
#endif
	}

Bool LockMenu(MenuE WhichMenu)
	{
#ifdef WINCIT
	WaitForSingleObject(MenuMutex, INFINITE);
#endif

	Bool retVal;

	if (!Menus[WhichMenu].Menu)
		{
		assert(!Menus[WhichMenu].UseCounter);

		Menus[WhichMenu].Menu = readData(5, WhichMenu * 2 + 1, WhichMenu * 2 + 2);
		}

	if (Menus[WhichMenu].Menu)
		{
		Menus[WhichMenu].UseCounter++;
		Menus[WhichMenu].Messages = (const char **)
				Menus[WhichMenu].Menu->aux;
		retVal = TRUE;
		}
	else
		{
		OutOfMemory(5);
		retVal = FALSE;
		}

#ifdef WINCIT
	ReleaseMutex(MenuMutex);
#endif

	return (retVal);
	}

void UnlockMenu(MenuE WhichMenu)
	{
#ifdef WINCIT
	WaitForSingleObject(MenuMutex, INFINITE);
#endif

	assert(Menus[WhichMenu].UseCounter);
	Menus[WhichMenu].UseCounter--;

#ifdef WINCIT
	ReleaseMutex(MenuMutex);
#endif
	}
