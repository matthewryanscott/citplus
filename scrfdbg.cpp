// --------------------------------------------------------------------------
// Citadel: ScrFDbg.CPP
//
// Internal script functions dealing with debugging.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "miscovl.h"


// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ DEBUG STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
void sfSetDebugScript(strList *params)
	{
	pCurScript->Debug = evaluateBool(params->string);
	}

void sfPauseScript(strList *)
	{
	pCurScript->Debug = TRUE;
	pCurScript->Pause = TRUE;
	}

void sfDebuggingMsg(strList *params)
	{
	if (pCurScript->Debug)
		{
		DebuggingMsg(evaluateString(params->string));
		}
	}

void sfSetBreakpoint(strList *params)
	{
	if (pCurScript->Debug)
		{
		Breakpoints *bp;

		for (bp = pCurScript->BPoints; bp; bp = (Breakpoints *) getNextLL(bp))
			{
			if (bp->LineNum == evaluateInt(params->string))
				{
				break;
				}
			}

		if (!bp)
			{
			bp = (Breakpoints *) addLL((void **) &pCurScript->BPoints, sizeof(*bp));

			if (bp)
				{
				bp->LineNum = evaluateInt(params->string);
				SETBOOL(TRUE);
				}
			else
				{
				SETBOOL(FALSE);
				}
			}
		else
			{
			SETBOOL(TRUE);
			}
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfClearBreakpoint(strList *params)
	{
	if (pCurScript->Debug)
		{
		Breakpoints *bp;
		int i;

		for (i = 1, bp = pCurScript->BPoints; bp; i++,
				bp = (Breakpoints *) getNextLL(bp))
			{
			if (bp->LineNum == evaluateInt(params->string))
				{
				break;
				}
			}

		if (bp)
			{
			deleteLLNode((void **) &pCurScript->BPoints, i);
			SETBOOL(TRUE);
			}
		else
			{
			SETBOOL(FALSE);
			}
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfAddWatch(strList *params)
	{
	if (pCurScript->Debug)
		{
		SETBOOL(AddWatch(evaluateString(params->string)));
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfRemoveWatch(strList *params)
	{
	if (pCurScript->Debug)
		{
		SETBOOL(RemoveWatch(evaluateString(params->string)));
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfSetDebugSystem(strList *params)
	{
	pCurScript->oldSystemDebug = evaluateBool(params->string);

	SETBOOL(pCurScript->oldSystemDebug ? TurnDebugOn() : TurnDebugOff());
	}

void sfGetDebugSystem(strList *)
	{
	SETBOOL(debug);
	}
