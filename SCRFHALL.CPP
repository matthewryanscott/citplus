// --------------------------------------------------------------------------
// Citadel: ScrFHall.CPP
//
// Internal script functions dealing with halls.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "hall.h"


// --------------------------------------------------------------------------
// HALL FILE STUFF


void sfGethallname(strList *params)
	{
	HallData[evaluateInt(params->string)].GetName(pCurScript->wowza, sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfGethallgroup(strList *params)
	{
	SETINT(HallData[evaluateInt(params->string)].GetGroupNumber());
	}

void sfIshallinuse(strList *params)
	{
	SETBOOL(HallData[evaluateInt(params->string)].IsInuse());
	}

void sfIshallgrp(strList *params)
	{
	SETBOOL(HallData[evaluateInt(params->string)].IsOwned());
	}

void sfIshalldesc(strList *params)
	{
	SETBOOL(HallData[evaluateInt(params->string)].IsDescribed());
	}

void sfIshallenterroom(strList *params)
	{
	SETBOOL(HallData[evaluateInt(params->string)].IsEnterRoom());
	}

void sfGethalldesc(strList *params)
	{
	HallData[evaluateInt(params->string)].GetDescriptionFile(pCurScript->wowza, sizeof(pCurScript->wowza));
	SETRTSTR();
	}

void sfIsRoomInHall(strList *params)
	{
	SETBOOL(HallData[evaluateInt(params->string)].IsRoomInHall(evaluateInt(params->next->string)));
	}

void sfSetRoomInHall(strList *params)
	{
	HallData[evaluateInt(params->string)].SetRoomInHall(evaluateInt(params->next->string),
			evaluateBool(params->next->next->string));
	}

void sfFindHallByName(strList *params)
	{
	if (params->next && evaluateBool(params->next->string))
		{
#ifndef WINCIT
		SETINT(partialhall(evaluateString(params->string)));
#endif
		}
	else
		{
		SETINT(hallexists(evaluateString(params->string)));
		}
	}
