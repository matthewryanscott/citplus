// --------------------------------------------------------------------------
// Citadel: LogAcc.CPP
//
// Access to log stuff

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "room.h"

// overrides LOG2.H's function; adds #SYSOP checking to it.
Bool LogEntry::IsInGroup(g_slot GroupSlot) const
	{
	return (
			// really in group
			LogEntry2::IsInGroup(GroupSlot) ||

			// or #SYSOP
			IsMainSysop()
		);
	}

Bool LogEntry::IsValid(void) const
	{
	return (LogEntry2::IsValid() && LogEntry3::IsValid() &&
			LogEntry4::IsValid() && LogEntry5::IsValid() &&
			LogEntry6::IsValid());
	}

void LogEntry::Clear(void)
	{
	VerifyHeap();
	LogEntry1::Clear();
	LogEntry2::Clear();
	LogEntry3::Clear();
	LogEntry4::Clear();
	LogEntry5::Clear();
	LogEntry6::Clear();
	LogExtensions::Clear();
	VerifyHeap();
	}

void LogTable::UpdateTable(const LogEntry1 *Entry, l_index Index)
	{
	for (l_slot Slot = 0; Slot < TableSize; Slot++)
		{
		if (LogTab[Slot].GetLogIndex() == Index)
			{
			LogTab.Update(Slot, Entry, Index);
			break;
			}
		}
	}

#ifdef WINCIT
void LogTable::SlideTableDown(l_slot HowFar, l_slot *Order)
	{
	assert(HowFar >= 0);
	assert(HowFar < TableSize);
	assert(Order);

	for (l_slot i = HowFar; i > 0; i--)
		{
		Order[i] = Order[i - 1];
		}
	}
#else
void LogTable::SlideTableDown(l_slot HowFar)
	{
	assert(HowFar >= 0);
	assert(HowFar < TableSize);

	if (IsValid())
		{
		for (l_slot i = HowFar; i > 0; i--)
			{
			Table[i] = Table[i - 1];
			}
		}
	}
#endif
