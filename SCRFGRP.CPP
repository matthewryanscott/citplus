// --------------------------------------------------------------------------
// Citadel: ScrFGrp.CPP
//
// Internal script functions dealing with groups.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "group.h"

// --------------------------------------------------------------------------
// GROUP FILE STUFF

void sfIsgrpinuse(strList *params)
	{
	SETBOOL(GroupData[evaluateInt(params->string)].IsInuse());
	}

void sfIsgrplocked(strList *params)
	{
	SETBOOL(GroupData[evaluateInt(params->string)].IsLocked());
	}

void sfIsgrphidden(strList *params)
	{
	SETBOOL(GroupData[evaluateInt(params->string)].IsHidden());
	}

void sfIsgrpautoadd(strList *params)
	{
	SETBOOL(GroupData[evaluateInt(params->string)].IsAutoAdd());
	}

void sfGetgrpname(strList *params)
	{
	GroupData[evaluateInt(params->string)].GetName(pCurScript->wowza,
			sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetgrpdesc(strList *params)
	{
	GroupData[evaluateInt(params->string)].GetDescription(pCurScript->wowza, sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGetgroupslot(strList *params)
	{
	SETINT(FindGroupByName(evaluateString(params->string)));
	}
