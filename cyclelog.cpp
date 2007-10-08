#include "ctdl.h"
#pragma hdrstop

#include "log.h"

void cyclegroup(g_slot slot)
	{
	LogEntry2 Log2(cfg.maxgroups);

#ifndef WINCIT
	if (loggedIn)
		{
		CurrentUser->LogEntry2::Save(ThisLog);
		}
#endif

	Log2.OpenFile();

	// take all users out of this group
	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		if (LogTab[i].IsInuse())
			{
			if (Log2.Load(LogTab[i].GetLogIndex()))
				{
				Log2.SetInGroup(slot, FALSE);
				Log2.Save(LogTab[i].GetLogIndex());
				}
			}
		}

	Log2.CloseFile();

#ifndef WINCIT
	if (loggedIn)
		{
		CurrentUser->LogEntry2::Load(ThisLog);
		}
#endif
	}

void cycleroom(r_slot slot, Bool GiveAccess, Bool ClearIgnore)
	{
	LogEntry3 Log3(cfg.maxrooms);
	LogEntry4 Log4(cfg.maxrooms);

#ifndef WINCIT
	if (loggedIn)
		{
		CurrentUser->LogEntry3::Save(ThisLog);
		CurrentUser->LogEntry4::Save(ThisLog);
		}
#endif

	Log3.OpenFile();
	Log4.OpenFile();

	for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
		{
		if (LogTab[i].IsInuse())
			{
			if (Log3.Load(LogTab[i].GetLogIndex()))
				{
				Log3.SetInRoom(slot, GiveAccess);
				Log3.Save(LogTab[i].GetLogIndex());
				}

			if (ClearIgnore)
				{
				if (Log4.Load(LogTab[i].GetLogIndex()) && Log4.IsRoomExcluded(slot))
					{
					Log4.SetRoomExcluded(slot, FALSE);
					Log4.Save(LogTab[i].GetLogIndex());
					}
				}
			}
		}

	Log3.CloseFile();
	Log4.CloseFile();

#ifndef WINCIT
	if (loggedIn)
		{
		CurrentUser->LogEntry3::Load(ThisLog);
		CurrentUser->LogEntry4::Load(ThisLog);
		}
#endif
	}
