// --------------------------------------------------------------------------
// Citadel: GrpOvl.CPP
//
// Overlayed group code.

#include "ctdl.h"
#pragma hdrstop

#include "group.h"
#include "log.h"


// --------------------------------------------------------------------------
// Contents


void TERMWINDOWMEMBER ListGroups(Bool CheckInGroup)
	{
	CRmPrintfCR(getmsg(578), cfg.Ugroups_nym);

	prtList(LIST_START);

	for (g_slot GroupSlot = 0; GroupSlot < cfg.maxgroups; GroupSlot++)
		{
		if (// group is in use
			GroupData[GroupSlot].IsInuse() &&

				(
				// and in the group
				CurrentUser->IsInGroup(GroupSlot) ||

					(
					// or not checking if in the group
					!CheckInGroup &&

					// and the group is not hidden, or super sysop, or on console
					(!GroupData[GroupSlot].IsHidden() ||
					CurrentUser->IsSuperSysop() || onConsole) &&

					// and the group is not locked, or is sysop
					(!GroupData[GroupSlot].IsLocked() ||
							CurrentUser->IsSysop())
					)
				)
			)
			{
			label Buffer;
			prtList(GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)));
			}
		}

	prtList(LIST_END);
	}

void GroupBuffer::Clear(void)
	{
	VerifyHeap();

	for (g_slot Index = 0; Index < NumGroups; Index++)
		{
		Groups[Index].Clear();
		}
	}
