// temporary?

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "group.h"
#include "log.h"
#include "hall.h"
#include "boolexpr.h"


// --------------------------------------------------------------------------
// groupseeshall(): Returns true if group can see hall.

Bool LogEntry::CanAccessHall(h_slot hallslot) const
	{
	assert(this);
	assert(hallslot >= 0);
	assert(hallslot < cfg.maxhalls);

	if (!HallData[hallslot].IsOwned())
		{
		return (TRUE);
		}

	if (IsMainSysop())
		{
		return (TRUE);
		}

	if (!HallData[hallslot].IsBoolGroup())
		{
		return (IsInGroup(HallData[hallslot].GetGroupNumber()));
		}
	else
		{
		L2FGT = this;

		BoolExpr Buffer;

		return (EvaluateBooleanExpression(HallData[hallslot].GetGroupExpression(Buffer), GroupTester));
		}
	}



// --------------------------------------------------------------------------
// pgroupseesroom(): Returns true if group can see room.

Bool LogEntry::HasRoomPrivileges(r_slot roomslot) const
	{
	assert(this);
	assert(roomslot >= 0);
	assert(roomslot < cfg.maxrooms);

	return (!RoomTab[roomslot].IsPrivilegedGroup() || IsInGroup(RoomTab[roomslot].GetPGroupNumber()));
	}
