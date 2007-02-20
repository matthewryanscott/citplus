// --------------------------------------------------------------------------
// Citadel: Hall.CPP
//
// Hall stuff

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "hall.h"

// --------------------------------------------------------------------------
// Contents
//
// hallexists() 			returns # of named hall, else CERROR
// iswindow()				for .kw .kvw is # room a window
// partialhall()			returns slot of partially named hall
// roominhall() 			indicates if room# is in hall#


HallBuffer	HallData;					// Hall data goes here


// --------------------------------------------------------------------------
// hallexists(): Return # of named hall, else CERROR.

h_slot hallexists(const char *hallname)
	{
	for (h_slot i = 0; i < cfg.maxhalls; i++)
		{
		if (HallData[i].IsInuse() && HallData[i].IsSameName(hallname))
			{
			return (i);
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// iswindow(): Is room a window into accessible halls?

Bool iswindow(r_slot roomslot)
	{
	if (!RoomTab[roomslot].IsInuse())
		{
		return (FALSE);
		}

	// there are no window rooms when in floor mode.
	if (cfg.subhubs == 4)
		{
		return (FALSE);
		}

	for (h_slot i = 0; i < cfg.maxhalls; i++)
		{
		if (HallData[i].IsInuse() &&
				HallData[i].IsWindowedIntoHall(roomslot))
			{
			return (TRUE);
			}
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// partialhall(): Returns slot # of partial hall name, else error.

h_slot TERMWINDOWMEMBER partialhall(const char *hallname)
	{
	label da_hn;

	if (strlen(deansi(hallname)) > LABELSIZE)
		{
		return (CERROR);
		}

	h_slot i = hallexists(hallname);
	if (i != CERROR)
		{
		return (i);
		}

	strcpy(da_hn, deansi(hallname));
	const int length = strlen(da_hn);

	for (i = 0; i < cfg.maxhalls; i++)
		{
		if (HallData[i].IsInuse())
			{
			label Buffer;
			if ((strnicmp(deansi(HallData[i].GetName(Buffer,
					sizeof(Buffer))), da_hn, length) == SAMESTRING)
					&& CurrentUser->CanAccessHall(i))
				{
				return (i);
				}
			}
		}

	for (i = 0; i < cfg.maxhalls; i++)
		{
		if (HallData[i].IsInuse())
			{
			label Buffer;
			if (IsSubstr(HallData[i].GetName(Buffer, sizeof(Buffer)), da_hn)
					&& CurrentUser->CanAccessHall(i))
				{
				return (i);
				}
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// roominhall(): Returns TRUE if room# is in hall#.

Bool roominhall(r_slot roomslot, h_slot hallslot)
	{
	if (HallData[hallslot].IsRoomInHall(roomslot) ||
			(roomslot == LOBBY && cfg.subhubs == 4))
		{
		return (TRUE);
		}

	return (FALSE);
	}
