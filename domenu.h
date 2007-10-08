// Support for DoMenu.CPP

struct MenuTrackerS
	{
	discardable *Menu;
	const char **Messages;
	int UseCounter;
	};

extern MenuTrackerS Menus[];

inline const char *getmsg(MenuE menu, uint nmbr)
	{
	// Uncomment this for supreme error checking
	//assert(Menus[menu].UseCounter);
	return (Menus[menu].Messages[nmbr]);
	}

#define getmenumsg(x,y) getmsg(x,y)

void DumpUnlockedMenus(void);
Bool LockMenu(MenuE WhichMenu);
void UnlockMenu(MenuE WhichMenu);
