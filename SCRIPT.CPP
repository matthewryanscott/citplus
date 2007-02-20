// --------------------------------------------------------------------------
// Citadel: Script.CPP
//
// The script interpretter, but not the functions (ScrF*.CPP).

#include "ctdl.h"
#pragma hdrstop

#define SCRIPTMAIN
#include "script.h"
#include "scrfarry.h"
#include "msg.h"
#include "log.h"

// --------------------------------------------------------------------------
// Contents


// Debugging stuff - SCRIPTDB.CPP
Bool dwHandler(EVENT evt, long param, int more, CITWINDOW *wnd);



static void (*findInternalFunction(const strList *callWith))(strList *);
static scriptFunctionList *findFunc(const char *name);
static scriptVariables *getVar(const char *name, Bool *Keep);
static Bool specialEvaluateBool(const scriptVariables *var);
static const char *specialEvaluateString(const scriptVariables *var);
static Bool CopyVars(scriptVariables **Dest, scriptVariables *Src);


long THREAD CurScript;

discardable *ScriptDData;				// all script discardable data

const char *resultStr = "_RESULT";

scriptVariables *createVariable(scriptVariables **Base, const char *Name, VariableTypeE Type, Bool MakeTempl,
		Bool NeedTempl, scriptVariables *structTempl)
	{
	if (NeedTempl && Type == TYPE_STRUCT && !structTempl)
		{
		return (NULL);
		}

	scriptVariables *theVar = (scriptVariables *) addLL((void **) Base, sizeof(scriptVariables));

	if (theVar)
		{
		theVar->type = Type;
		CopyStringToBuffer(theVar->name, Name);

		if (Type == TYPE_STRUCT)
			{
			theVar->v.s = NULL;

			if (NeedTempl)
				{
				if (MakeTempl)
					{
					theVar->type = TYPE_STRUCTTEMPL;

					int NameLength = strlen(structTempl->name) + 1;
					theVar->v.string = new char[NameLength];

					if (theVar->v.string)
						{
						CopyString2Buffer(theVar->v.string, structTempl->name, NameLength);
						}
					else
						{
						deleteLLNode((void **) Base, getLLCount((void *) *Base));
						theVar = NULL;
						}
					}
				else
					{
					if (!CopyVars(&theVar->v.s, structTempl->v.s))
						{
						disposeLL((void **) &theVar->v.s);
						deleteLLNode((void **) Base, getLLCount((void *) *Base));
						theVar = NULL;
						}
					}
				}
			}
		else if (Type >= TYPE_STRING_MIN && Type <= TYPE_STRING_MAX)
			{
			if (MakeTempl)
				{
				theVar->v.string = NULL;
				}
			else
				{
				theVar->v.string = new char[Type - TYPE_STRING_MIN + 2];

				if (theVar->v.string)
					{
					*theVar->v.string = 0;
					}
				else
					{
					deleteLLNode((void **) Base, getLLCount((void *) *Base));
					theVar = NULL;
					}
				}
			}
		}

	return (theVar);
	}


void dumpVars(scriptVariables *base)
	{
	scriptVariables *theVar, *theVar2;

	for (theVar = base; theVar; theVar = theVar2)
		{
		if (theVar->type == TYPE_STRUCTTEMPL || (theVar->type >= TYPE_STRING_MIN && theVar->type <= TYPE_STRING_MAX)
			)
			{
			delete [] theVar->v.string;
			}

		if (theVar->type == TYPE_STRUCT)
			{
			dumpVars(theVar->v.s);
			}

		theVar2 = theVar->next;

		delete theVar;
		}
	}

static Bool AssignVariable(scriptVariables *Var, const char *Expr)
	{
	if (!Var)
		{
		return (FALSE);
		}

	switch (Var->type)
		{
		case TYPE_INT:
			{
			Var->v.i = evaluateInt(Expr);
			break;
			}

		case TYPE_UINT:
			{
			Var->v.ui = evaluateUint(Expr);
			break;
			}

		case TYPE_LONG:
			{
			Var->v.l = evaluateLong(Expr);
			break;
			}

		case TYPE_ULONG:
			{
			Var->v.ul = evaluateUlong(Expr);
			break;
			}

		case TYPE_BOOL:
			{
			Var->v.b = evaluateBool(Expr);
			break;
			}

		default:
			{
			if (Var->type >= TYPE_STRING_MIN && Var->type <= TYPE_STRING_MAX)
				{
				CopyString2Buffer(Var->v.string, evaluateString(Expr), Var->type - TYPE_STRING_MIN + 2);
				}
			else
				{
				return (FALSE);
				}

			break;
			}
		}

	return (TRUE);
	}

void executeCurrentCommand(void)
	{
	if (pCurScript->LastCmd)
		{
		pCurScript->LastCmd->runCounter++;

#ifndef WINCIT
		ulong TheTime = pCurScript->CmdTimer.Read();
		pCurScript->LastCmd->time += TheTime;

		if (pCurScript->DebuggingWindow)
			{
			char wow[80];

			sprintf(wow, getscrmsg(51), pCurScript->LastCmd->line, TheTime / (TIMER_HERTZ / 1000),
					pCurScript->LastCmd->time / (TIMER_HERTZ / 1000), pCurScript->LastCmd->runCounter,
					pCurScript->LastCmd->time / (TIMER_HERTZ / 1000) / pCurScript->LastCmd->runCounter);

			DebuggingMsg(wow);
			}
#endif
		}

	pCurScript->LastCmd = pCurScript->curCmd;

#ifdef WINCIT
	if (pCurScript->TW)
		{
		pCurScript->TW->KBReady();
		}
#else
	KBReady();
#endif

	if (debug && !pCurScript->oldSystemDebug)
		{
		pCurScript->Debug = TRUE;
		}

	pCurScript->oldSystemDebug = debug;

	if (!pCurScript->Debug)
		{
		Breakpoints *bp;
		for (bp = pCurScript->BPoints; bp; bp = (Breakpoints *) getNextLL(bp))
			{
			if (bp->LineNum == pCurScript->curCmd->line)
				{
				pCurScript->Debug = TRUE;
				break;
				}
			}
		}

	if (pCurScript->Debug)
		{
		if (!pCurScript->DebuggingWindow && pCurScript->curCmd->line)
			{
#ifndef WINCIT
			char *ld = new char[(conRows / 2 - 1) * (conCols - 2)];

			if (ld)
				{
				memset(ld, 0, (conRows / 2 - 1) * (conCols - 2));
				WINDOWFLAGS flags;
				SRECT rect;

				memset(&flags, 0, sizeof(flags));

				flags.visible = TRUE;
				flags.showTitle = TRUE;
				flags.moveable = TRUE;
				flags.minimize = TRUE;
				flags.resize = TRUE;

				rect.top = 0;
				rect.left = 0;
				rect.bottom = conRows / 2;
				rect.right = conCols / 3;

				pCurScript->DebuggingWindow = makeCitWindow(dwHandler, NULL, getscrmsg(28), flags, rect, ld, FALSE);

				setFocus(pCurScript->DebuggingWindow);
				}
#endif
			}
		}
	else
		{
		if (pCurScript->DebuggingWindow)
			{
			destroyCitWindow(pCurScript->DebuggingWindow, FALSE);
			pCurScript->DebuggingWindow = NULL;
			}

		pCurScript->Pause = FALSE;
		}

	if (pCurScript->DebuggingWindow)
		{
		char wow[80];

		sprintf(wow, getscrmsg(31), pCurScript->curCmd->line);
		DebuggingMsg(wow);

		Breakpoints *bp;
		for (bp = pCurScript->BPoints; bp; bp = (Breakpoints *) getNextLL(bp))
			{
			if (bp->LineNum == pCurScript->curCmd->line)
				{
				DebuggingMsg(getscrmsg(32));
				pCurScript->Pause = TRUE;
				break;
				}
			}

		if (pCurScript->WWindow)
			{
			(pCurScript->WWindow->func)(EVT_DRAWINT, 0, 0, pCurScript->WWindow);
			}

		if (pCurScript->SourceWindow)
			{
			(pCurScript->SourceWindow->func)(EVT_DRAWINT, 0, 1, pCurScript->SourceWindow);
			}

		if (pCurScript->Pause)
			{
			if (!getFocus())
				{
				setFocus(pCurScript->DebuggingWindow);
				}
			}
		}

	while (pCurScript->Pause && pCurScript->curCmd->line && !pCurScript->SingleStep && !pCurScript->Error &&
		   pCurScript->Debug)
		{
#ifdef WINCIT
		if (pCurScript->TW)
			{
			pCurScript->TW->KBReady();
			}
#else
		KBReady();
#endif
		}

	pCurScript->SingleStep = FALSE;

#ifndef WINCIT
	pCurScript->CmdTimer.Reset();
#endif

	switch (pCurScript->curCmd->type)
		{
		case CMD_ADD:
			{
			long theResult = 0;

			for (strList *theSL = pCurScript->curCmd->c.sl; theSL; theSL = (strList *) getNextLL(theSL))
				{
				theResult += evaluateLong(theSL->string);
				}

			pCurScript->lastResult.type = TYPE_LONG;
			pCurScript->lastResult.v.l = theResult;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_SUB:
			{
			strList *theSL = pCurScript->curCmd->c.sl;

			if (theSL)
				{
				long theResult = evaluateLong(theSL->string);

				for (theSL = (strList *) getNextLL(theSL); theSL; theSL = (strList *) getNextLL(theSL))
					{
					theResult -= evaluateLong(theSL->string);
					}

				pCurScript->lastResult.type = TYPE_LONG;
				pCurScript->lastResult.v.l = theResult;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_MUL:
			{
			strList *theSL = pCurScript->curCmd->c.sl;

			if (theSL)
				{
				long theResult = evaluateLong(theSL->string);

				for (theSL = (strList *) getNextLL(theSL); theSL; theSL = (strList *) getNextLL(theSL))
					{
					theResult *= evaluateLong(theSL->string);
					}

				pCurScript->lastResult.type = TYPE_LONG;
				pCurScript->lastResult.v.l = theResult;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_DIV:
			{
			strList *theSL = pCurScript->curCmd->c.sl;

			if (theSL)
				{
				long theResult = evaluateLong(theSL->string);

				for (theSL = (strList *) getNextLL(theSL); theSL; theSL = (strList *) getNextLL(theSL))
					{
					theResult /= evaluateLong(theSL->string);
					}

				pCurScript->lastResult.type = TYPE_LONG;
				pCurScript->lastResult.v.l = theResult;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_MOD:
			{
			strList *theSL = pCurScript->curCmd->c.sl;

			if (theSL)
				{
				long theResult = evaluateLong(theSL->string);

				for (theSL = (strList *) getNextLL(theSL); theSL; theSL = (strList *) getNextLL(theSL))
					{
					theResult %= evaluateLong(theSL->string);
					}

				pCurScript->lastResult.type = TYPE_LONG;
				pCurScript->lastResult.v.l = theResult;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_AND:
			{
			Bool theResult = TRUE;

			for (strList *theSL = pCurScript->curCmd->c.sl; theSL; theSL = (strList *) getNextLL(theSL))
				{
				if (!evaluateBool(theSL->string))
					{
					theResult = FALSE;
					break;
					}
				}

			pCurScript->lastResult.type = TYPE_BOOL;
			pCurScript->lastResult.v.b = theResult;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_OR:
			{
			Bool theResult = FALSE;

			for (strList *theSL = pCurScript->curCmd->c.sl; theSL; theSL = (strList *) getNextLL(theSL))
				{
				if (evaluateBool(theSL->string))
					{
					theResult = TRUE;
					break;
					}
				}

			pCurScript->lastResult.type = TYPE_BOOL;
			pCurScript->lastResult.v.b = theResult;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_XOR:
			{
			int TrueCount = 0;

			for (strList *theSL = pCurScript->curCmd->c.sl; theSL; theSL = (strList *) getNextLL(theSL))
				{
				if (evaluateBool(theSL->string))
					{
					TrueCount++;
					}
				}

			pCurScript->lastResult.type = TYPE_BOOL;
			pCurScript->lastResult.v.b = TrueCount == 1;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_ASGN:
			{
			scriptVariables *theVar;

			if (!pCurScript->curCmd->ptr)
				{
				Bool Keep;
				theVar = getVar(pCurScript->curCmd->c.sl->string, &Keep);
				if (Keep) pCurScript->curCmd->ptr = theVar;
				}
			else
				{
				theVar = (scriptVariables *) pCurScript->curCmd->ptr;
				}

			strList *theSL = (strList *) getNextLL(pCurScript->curCmd->c.sl);

			if (theVar)
				{
				if (!AssignVariable(theVar, theSL->string))
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->CRmPrintfCR(getscrmsg(4), pCurScript->Name, pCurScript->curCmd->line,
							theVar->type);
						}
#else
					CRmPrintfCR(getscrmsg(4), pCurScript->Name, pCurScript->curCmd->line, theVar->type);
#endif

					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					}
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
						pCurScript->curCmd->c.sl->string);
					}
#else
				CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
						pCurScript->curCmd->c.sl->string);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRB:
			{
			const int Offset = evaluateInt(pCurScript->curCmd->c.sl->next->string);
			const int StrLen = evaluateInt(pCurScript->curCmd->c.sl->next->next->string);
			const char *Str = evaluateString(pCurScript->curCmd->c.sl->string);

			pCurScript->lastResult.type = TYPE_STRING_MAX;
			pCurScript->lastResult.v.string = pCurScript->wowza;

			if ((int) strlen(Str) > Offset)
				{
				CopyString2Buffer(pCurScript->lastResult.v.string, Str + Offset, min(StrLen, 256));
				}
			else
				{
				pCurScript->lastResult.v.string[0] = 0;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRU:
			{
			pCurScript->lastResult.type = TYPE_STRING_MAX;
			pCurScript->lastResult.v.string = pCurScript->wowza;
			strcpy(pCurScript->lastResult.v.string, evaluateString(pCurScript->curCmd->c.sl->string));
			strupr(pCurScript->lastResult.v.string);
			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRL:
			{
			pCurScript->lastResult.type = TYPE_STRING_MAX;
			pCurScript->lastResult.v.string = pCurScript->wowza;
			strcpy(pCurScript->lastResult.v.string, evaluateString(pCurScript->curCmd->c.sl->string));
			strlwr(pCurScript->lastResult.v.string);
			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRC:
			{
			char Temp[512]; 	// Twice max size
			*Temp = 0;

			for (const strList *theSL = pCurScript->curCmd->c.sl; theSL; theSL = (strList *) getNextLL(theSL))
				{
				strcat(Temp, evaluateString(theSL->string));
				Temp[255] = 0;	// Truncate in case of overflow
				}

			pCurScript->lastResult.type = TYPE_STRING_MAX;
			pCurScript->lastResult.v.string = pCurScript->wowza;
			CopyStringToBuffer(pCurScript->wowza, Temp);

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRS:
			{
			pCurScript->lastResult.type = TYPE_LONG;

			Bool Keep;
			scriptVariables *theVar = getVar(pCurScript->curCmd->c.sl->string, &Keep);

			if (theVar)
				{
				pCurScript->lastResult.v.l = theVar->type - TYPE_STRING_MIN + 1;
				}
			else
				{
				pCurScript->lastResult.v.l = 0;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRE:
			{
			const long Length = strlen(evaluateString(pCurScript->curCmd->c.sl->string));

			pCurScript->lastResult.type = TYPE_LONG;
			pCurScript->lastResult.v.l = Length;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_STRPR:
		case CMD_STRPL:
			{
			if (pCurScript->curCmd->c.sl->next)
				{
				int j = strlen(evaluateString(pCurScript->curCmd->c.sl->string));

				if (j < evaluateInt(pCurScript->curCmd->c.sl->next->string))
					{
					j = evaluateInt(pCurScript->curCmd->c.sl->next->string) - j;

					if (pCurScript->curCmd->type == CMD_STRPR)
						{
						strcpy(pCurScript->wowza, evaluateString(pCurScript->curCmd->c.sl->string));

						for (; j; j--)
							{
							strcat(pCurScript->wowza, spc);
							}
						}
					else
						{
						pCurScript->wowza[0] = 0;
						for (; j; j--)
							{
							strcat(pCurScript->wowza, spc);
							}

						strcat(pCurScript->wowza, evaluateString(pCurScript->curCmd->c.sl->string));
						}
					}
				else
					{
					strcpy(pCurScript->wowza, evaluateString(pCurScript->curCmd->c.sl->string));
					}

				pCurScript->lastResult.v.string = pCurScript->wowza;
				pCurScript->lastResult.type = TYPE_STRING_MAX;
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(6), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				CRmPrintfCR(getscrmsg(6), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_GOTO:
			{
			scriptCommandRecord *theCmd;
			for (theCmd = pCurScript->curFunc->cmds; theCmd; theCmd = (scriptCommandRecord *) getNextLL(theCmd))
				{
				if (theCmd->type == CMD_LABEL && SameString(theCmd->c.str, evaluateString(pCurScript->curCmd->c.str)))
					{
					pCurScript->curCmd = (scriptCommandRecord *) getNextLL(theCmd);
					break;
					}
				}

			if (!theCmd)
				{
				// no matching label - bad stuff
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(7), pCurScript->Name, pCurScript->curCmd->line,
							evaluateString(pCurScript->curCmd->c.str));
					}
#else
				CRmPrintfCR(getscrmsg(7), pCurScript->Name, pCurScript->curCmd->line,
						evaluateString(pCurScript->curCmd->c.str));
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			break;
			}

		case CMD_RUN:
			{
			char toRun[128];
			CopyStringToBuffer(toRun, evaluateString(pCurScript->curCmd->c.str));

			// Use temp var Ran just to be safe - runScript messes
			// with CurScript. It shouldn't do anything bad, but
			// this incures such little overhead that i'm doing it.

#ifdef WINCIT
			Bool Ran = runScript(toRun, pCurScript->TW);
#else
			Bool Ran = runScript(toRun);
#endif

			pCurScript->lastResult.v.b = Ran;
			pCurScript->lastResult.type = TYPE_BOOL;

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_CHAIN:
			{
			char toRun[128];
			CopyStringToBuffer(toRun, evaluateString(pCurScript->curCmd->c.str));

			pCurScript->lastResult.type = TYPE_BOOL;
#ifdef WINCIT
			pCurScript->ToRet = runScript(toRun, pCurScript->TW);
#else
			pCurScript->ToRet = runScript(toRun);
#endif

			// Don't continue with this script when chained is done
			pCurScript->curCmd = NULL;
			break;
			}

		case CMD_IF:
		case CMD_IFNOT:
			{
			scriptVariables *theVar = (scriptVariables *) pCurScript->curCmd->ptr;

			if (!theVar && pCurScript->curCmd->c.str[0] == '_')
				{
				Bool Keep;
				theVar = getVar(pCurScript->curCmd->c.str, &Keep);
				if (Keep) pCurScript->curCmd->ptr = theVar;
				}

			Bool EvalsTo = theVar ? specialEvaluateBool(theVar) : evaluateBool(pCurScript->curCmd->c.str);

			if (pCurScript->curCmd->type == CMD_IFNOT)
				{
				EvalsTo = !EvalsTo;
				}

			if (!EvalsTo)
				{
				// skip to #ELSE or #ENDIF
				int IfCounter;
				scriptCommandRecord *theCmd;

				for (IfCounter = 0, theCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd); theCmd;
						theCmd = (scriptCommandRecord *) getNextLL(theCmd))
					{
					if (theCmd->type == CMD_IF || theCmd->type == CMD_IFNOT)
						{
						IfCounter++;
						}

					if ((theCmd->type == CMD_ELSE || theCmd->type == CMD_ENDIF) && !IfCounter)
						{
						pCurScript->curCmd = theCmd;
						break;
						}

					if (theCmd->type == CMD_ENDIF)
						{
						IfCounter--;
						}
					}

				if (!theCmd)
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->CRmPrintfCR(getscrmsg(8), pCurScript->Name, pCurScript->curCmd->line);
						}
#else
					CRmPrintfCR(getscrmsg(8), pCurScript->Name, pCurScript->curCmd->line);
#endif
					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					break;
					}
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_ELSE:
			{
			// skip to endif
			int IfCounter;
			scriptCommandRecord *theCmd;

			for (IfCounter = 0, theCmd = pCurScript->curCmd; theCmd; theCmd = (scriptCommandRecord *) getNextLL(theCmd))
				{
				if (theCmd->type == CMD_IF || theCmd->type == CMD_IFNOT)
					{
					IfCounter++;
					}

				if (theCmd->type == CMD_ENDIF)
					{
					if (!IfCounter)
						{
						pCurScript->curCmd = (scriptCommandRecord *) getNextLL(theCmd);
						break;
						}
					else
						{
						IfCounter--;
						}
					}
				}

			if (!theCmd)
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(8), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				CRmPrintfCR(getscrmsg(8), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			break;
			}

		case CMD_SWITCH:
			{
			char Value[256];

			scriptVariables *theVar = (scriptVariables *) pCurScript->curCmd->ptr;

			if (!theVar && pCurScript->curCmd->c.str[0] == '_')
				{
				Bool Keep;
				theVar = getVar(pCurScript->curCmd->c.str, &Keep);
				if (Keep) pCurScript->curCmd->ptr = theVar;
				}

			CopyStringToBuffer(Value, theVar ? specialEvaluateString(theVar) : evaluateString(pCurScript->curCmd->c.str));

			// skip to proper #CASE or #DEFAULT or #ENDSWITCH
			int SwitchCounter;
			scriptCommandRecord *theCmd;

			for (SwitchCounter = 0, theCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd); theCmd;
					theCmd = (scriptCommandRecord *) getNextLL(theCmd))
				{
				if (theCmd->type == CMD_SWITCH)
					{
					SwitchCounter++;
					}

				if (theCmd->type == CMD_CASE && !SwitchCounter)
					{
					scriptVariables *testVar = (scriptVariables *) theCmd->ptr;

					if (!testVar && theCmd->c.str[0] == '_')
						{
						Bool Keep;
						testVar = getVar(theCmd->c.str, &Keep);
						if (Keep) theCmd->ptr = testVar;
						}

					if (SameString(Value, testVar ? specialEvaluateString(testVar) : evaluateString(theCmd->c.str)))
						{
						pCurScript->curCmd = theCmd;
						break;
						}
					}

				if (theCmd->type == CMD_DEFAULT && !SwitchCounter)
					{
					pCurScript->curCmd = theCmd;
					break;
					}

				if (theCmd->type == CMD_ENDSWITCH)
					{
					if (SwitchCounter)
						{
						SwitchCounter--;
						}
					else
						{
						pCurScript->curCmd = theCmd;
						break;
						}
					}
				}

			if (!theCmd)
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(59), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				CRmPrintfCR(getscrmsg(59), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				break;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_CASE:
		case CMD_DEFAULT:
			{
			// skip to endswitch
			int SwitchCounter;
			scriptCommandRecord *theCmd;

			for (SwitchCounter = 0, theCmd = pCurScript->curCmd; theCmd; theCmd = (scriptCommandRecord *) getNextLL(theCmd))
				{
				if (theCmd->type == CMD_SWITCH)
					{
					SwitchCounter++;
					}

				if (theCmd->type == CMD_ENDSWITCH)
					{
					if (SwitchCounter)
						{
						SwitchCounter--;
						}
					else
						{
						pCurScript->curCmd = (scriptCommandRecord *) getNextLL(theCmd);
						break;
						}
					}
				}

			if (!theCmd)
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->CRmPrintfCR(getscrmsg(59), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				CRmPrintfCR(getscrmsg(59), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			break;
			}

		case CMD_EQ:
		case CMD_GE:
		case CMD_LE:
		case CMD_NE:
		case CMD_GT:
		case CMD_LT:
			{
			scriptVariables *theVar;

			if (pCurScript->curCmd->ptr)
				{
				theVar = (scriptVariables *) pCurScript->curCmd->ptr;
				}
			else
				{
				if (pCurScript->curCmd->c.sl->string[0] == '_')
					{
					// first is a variable
					Bool Keep;
					if ((theVar = getVar(pCurScript->curCmd->c.sl->string, &Keep)) == NULL)
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
									pCurScript->curCmd->c.sl->string);
							}
#else
						CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
									pCurScript->curCmd->c.sl->string);
#endif

						pCurScript->curCmd = NULL;
						pCurScript->ToRet = FALSE;
						break;
						}
					}
				else if (pCurScript->curCmd->c.sl->next->string[0] == '_')
					{
					// second is a variable
					Bool Keep;
					if ((theVar = getVar(pCurScript->curCmd->c.sl->next->string, &Keep)) == NULL)
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
									pCurScript->curCmd->c.sl->next->string);
							}
#else
						CRmPrintfCR(getscrmsg(5), pCurScript->Name, pCurScript->curCmd->line,
								pCurScript->curCmd->c.sl->next->string);
#endif
						pCurScript->curCmd = NULL;
						pCurScript->ToRet = FALSE;
						break;
						}
					}
				else
					{
					// const with const - icky bad
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->CRmPrintfCR(getscrmsg(9), pCurScript->Name, pCurScript->curCmd->line);
						}
#else
					CRmPrintfCR(getscrmsg(9), pCurScript->Name, pCurScript->curCmd->line);
#endif
					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					break;
					}

				pCurScript->curCmd->ptr = theVar;
				}

			long l;
			switch (theVar->type)
				{
				case TYPE_INT:
					{
					l = evaluateInt(pCurScript->curCmd->c.sl->string) -
							evaluateInt(pCurScript->curCmd->c.sl->next->string);
					break;
					}

				case TYPE_UINT:
					{
					l = evaluateUint(pCurScript->curCmd->c.sl->string) -
							evaluateUint(pCurScript->curCmd->c.sl->next->string);
					break;
					}

				case TYPE_LONG:
					{
					l = evaluateLong(pCurScript->curCmd->c.sl->string) -
							evaluateLong(pCurScript->curCmd->c.sl->next->string);
					break;
					}

				case TYPE_ULONG:
					{
					l = evaluateUlong(pCurScript->curCmd->c.sl->string) -
							evaluateUlong(pCurScript->curCmd->c.sl->next->string);
					break;
					}

				case TYPE_BOOL:
					{
					l = evaluateBool(pCurScript->curCmd->c.sl->string) -
							evaluateBool(pCurScript->curCmd->c.sl->next->string);
					break;
					}

				default:
					{
					if (theVar->type >= TYPE_STRING_MIN && theVar->type <= TYPE_STRING_MAX)
						{
						l = strcmpi(evaluateString(pCurScript->curCmd->c.sl->string),
								evaluateString(pCurScript->curCmd->c.sl->next->string));
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->CRmPrintfCR(getscrmsg(4), pCurScript->Name,
									pCurScript->curCmd->line, theVar->type);
							}
#else
						CRmPrintfCR(getscrmsg(4), pCurScript->Name, pCurScript->curCmd->line, theVar->type);
#endif
						pCurScript->curCmd = NULL;
						pCurScript->ToRet = FALSE;
						}

					break;
					}
				}

			pCurScript->lastResult.type = TYPE_BOOL;
			switch (pCurScript->curCmd->type)
				{
				case CMD_EQ:
					{
					pCurScript->lastResult.v.b = l == 0;
					break;
					}

				case CMD_GE:
					{
					pCurScript->lastResult.v.b = l >= 0;
					break;
					}

				case CMD_LE:
					{
					pCurScript->lastResult.v.b = l <= 0;
					break;
					}

				case CMD_NE:
					{
					pCurScript->lastResult.v.b = l != 0;
					break;
					}

				case CMD_GT:
					{
					pCurScript->lastResult.v.b = l > 0;
					break;
					}

				case CMD_LT:
					{
					pCurScript->lastResult.v.b = l < 0;
					break;
					}
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		case CMD_CALL:
			{
			if (pCurScript->curCmd->c.sl->string[0] != '_')
				{
				// internal function
				if (pCurScript->curCmd->ptr)
					{
					(*(void (*)(strList *)) (pCurScript->curCmd->ptr)) (pCurScript->curCmd->c.sl->next);
					}
				else
					{
					void (*func)(strList *params) = findInternalFunction(pCurScript->curCmd->c.sl);

					if (func)
						{
						pCurScript->curCmd->ptr = func;
						(*func)(pCurScript->curCmd->c.sl->next);
						}
					}

				pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
				// add code here to deal with null-functions???
				}
			else
				{
				scriptFunctionList *theFunc;

				// script function
				if (!pCurScript->curCmd->ptr)
					{
					pCurScript->curCmd->ptr = theFunc = findFunc(pCurScript->curCmd->c.sl->string);
					}
				else
					{
					theFunc = (scriptFunctionList *) pCurScript->curCmd->ptr;
					}

				if (theFunc)
					{
					FunctionContext *NewContext = (FunctionContext *) addLL((void **) &theFunc->Context,
							sizeof(FunctionContext));

					scriptCallList *theCallList = (scriptCallList *) addLL((void **) &pCurScript->callList,
							sizeof(*theCallList));

					if (NewContext && theCallList && CopyVars(&NewContext->AutoVars, theFunc->AutoVars))
						{
						NewContext->loopInfo = NULL;

						theCallList->oldFunc = pCurScript->curFunc;
						theCallList->oldCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
						theCallList->oldContext = pCurScript->curContext;

						strList *theSL;
						int j;

						for (j = 0, theSL = pCurScript->curCmd->c.sl->next;
								j < MAXARGS && theSL && pCurScript->curCmd;
								j++, theSL = (strList *) getNextLL(theSL))
							{
							if (theFunc->argt[j] != TYPE_NONE)
								{
								createVariable(&NewContext->argv[j], theFunc->argn[j],
										(VariableTypeE) (theFunc->argt[j] & ~BY_REF), FALSE, TRUE, theFunc->args[j]);

								if (NewContext->argv[j])
									{
									if (theFunc->argt[j] & BY_REF)
										{
										Bool Keep;
										NewContext->byref[j] = getVar(theSL->string, &Keep);
										}

									if (!AssignVariable(NewContext->argv[j], theSL->string))
										{
#ifdef WINCIT
										if (pCurScript->TW)
											{
											pCurScript->TW->CRmPrintfCR(getscrmsg(4), pCurScript->Name,
													pCurScript->curCmd->line, NewContext->argv[j]->type);
											}
#else
										CRmPrintfCR(getscrmsg(4), pCurScript->Name, pCurScript->curCmd->line,
												NewContext->argv[j]->type);
#endif

										pCurScript->curCmd = NULL;
										pCurScript->ToRet = FALSE;
										}
									}
								else
									{
#ifdef WINCIT
									if (pCurScript->TW)
										{
										pCurScript->TW->mPrintfCR(getscrmsg(57), pCurScript->Name, pCurScript->curCmd->line,
												theFunc->argn[j]);
										}
#else
									mPrintfCR(getscrmsg(57), pCurScript->Name, pCurScript->curCmd->line,
											theFunc->argn[j]);
#endif
									pCurScript->curCmd = NULL;
									pCurScript->ToRet = FALSE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->CRmPrintfCR(getscrmsg(12), pCurScript->Name,
											pCurScript->curCmd->line, pCurScript->curCmd->c.sl->string);
									}
#else
								CRmPrintfCR(getscrmsg(12), pCurScript->Name, pCurScript->curCmd->line,
										pCurScript->curCmd->c.sl->string);
#endif
								pCurScript->curCmd = NULL;
								pCurScript->ToRet = FALSE;
								}
							}

						if (pCurScript->curCmd)
							{
							pCurScript->curCmd = theFunc->cmds;
							// add code here to deal with null-functions???
							pCurScript->curFunc = theFunc;
							pCurScript->curContext = NewContext;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
							}
#else
						mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
#endif
						pCurScript->curCmd = NULL;
						pCurScript->ToRet = FALSE;
						}
					}
				else
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->mPrintfCR(getscrmsg(14), pCurScript->Name, pCurScript->curCmd->line,
								pCurScript->curCmd->c.sl->string);
						}
#else
					mPrintfCR(getscrmsg(14), pCurScript->Name, pCurScript->curCmd->line, pCurScript->curCmd->c.sl->string);
#endif
					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					}
				}

			break;
			}

		case CMD_RET:
			{
			scriptFunctionList *StartFunc = pCurScript->curFunc;
			FunctionContext *StartContext = pCurScript->curContext;

			int j2 = (int) getLLCount((void *) pCurScript->callList);
			scriptCallList *theCallList = (scriptCallList *) getLLNum((void *) pCurScript->callList, j2);

			if (theCallList)
				{
				switch (StartFunc->rett)
					{
					case TYPE_BOOL:
						{
						pCurScript->lastResult.v.b = evaluateBool(pCurScript->curCmd->c.str);
						pCurScript->lastResult.type = TYPE_BOOL;
						break;
						}

					case TYPE_INT:
						{
						pCurScript->lastResult.v.i = evaluateInt(pCurScript->curCmd->c.str);
						pCurScript->lastResult.type = TYPE_INT;
						break;
						}

					case TYPE_UINT:
						{
						pCurScript->lastResult.v.ui = evaluateUint(pCurScript->curCmd->c.str);
						pCurScript->lastResult.type = TYPE_UINT;
						break;
						}

					case TYPE_ULONG:
						{
						pCurScript->lastResult.v.ul = evaluateUlong(pCurScript->curCmd->c.str);
						pCurScript->lastResult.type = TYPE_ULONG;
						break;
						}

					case TYPE_LONG:
						{
						pCurScript->lastResult.v.l = evaluateLong(pCurScript->curCmd->c.str);
						pCurScript->lastResult.type = TYPE_LONG;
						break;
						}

					default:
						{
						if (StartFunc->rett >= TYPE_STRING_MIN && StartFunc->rett <= TYPE_STRING_MAX)
							{
							pCurScript->lastResult.v.string = pCurScript->wowza;
							strcpy(pCurScript->wowza, evaluateString(pCurScript->curCmd->c.str));
							pCurScript->lastResult.type = TYPE_STRING_MAX;
							}
						}
					}

				for (int i = 0; i < MAXARGS; i++)
					{
					if (StartFunc->argt[i] & BY_REF)
						{
						AssignVariable(StartContext->byref[i], StartFunc->argn[i]);
						}
					}

				pCurScript->curFunc = theCallList->oldFunc;
				pCurScript->curCmd = theCallList->oldCmd;
				pCurScript->curContext = theCallList->oldContext;

				deleteLLNode((void **) &pCurScript->callList, j2);
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->mPrintfCR(getscrmsg(27), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				mPrintfCR(getscrmsg(27), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			disposeLL((void **) &StartContext->loopInfo);
			dumpVars(StartContext->AutoVars);

			for (int wow = 0; wow < MAXARGS; wow++)
				{
				dumpVars(StartContext->argv[wow]);
				}

			deleteLLNode((void **) &StartFunc->Context, getLLCount((void *) StartFunc->Context));

			break;
			}

		case CMD_DO:
			{
			scriptLoops *theLoop = (scriptLoops *) addLL((void **)
					&(pCurScript->curContext->loopInfo), sizeof(*theLoop));

			if (theLoop)
				{
				theLoop->LoopType = LOOP_DO;
				theLoop->where = pCurScript->curCmd;

				pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			break;
			}

		case CMD_LOOP:
		case CMD_LOOPNOT:
			{
			scriptLoops *theLoop;
			for (theLoop = pCurScript->curContext->loopInfo; theLoop && theLoop->next;
					theLoop = (scriptLoops *) getNextLL(theLoop));

			if (theLoop && theLoop->LoopType == LOOP_DO)
				{
				scriptVariables *theVar = (scriptVariables *) pCurScript->curCmd->ptr;

				if (!theVar && pCurScript->curCmd->c.str[0] == '_')
					{
					Bool Keep;
					theVar = getVar(pCurScript->curCmd->c.str, &Keep);
					if (Keep) pCurScript->curCmd->ptr = theVar;
					}

				Bool EvalsTo = theVar ? specialEvaluateBool(theVar) : evaluateBool(pCurScript->curCmd->c.str);

				if (pCurScript->curCmd->type == CMD_LOOPNOT)
					{
					EvalsTo = !EvalsTo;
					}

				if (EvalsTo)
					{
					pCurScript->curCmd = theLoop->where;
					}
				else
					{
					deleteLLNode((void **) &(pCurScript->curContext->loopInfo),
							getLLCount(pCurScript->curContext->loopInfo));
					}
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->mPrintfCR(getscrmsg(15), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				mPrintfCR(getscrmsg(15), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);

			break;
			}

		case CMD_WHILE:
		case CMD_WHILENOT:
			{
			scriptVariables *theVar = (scriptVariables *) pCurScript->curCmd->ptr;

			if (!theVar && pCurScript->curCmd->c.str[0] == '_')
				{
				Bool Keep;
				theVar = getVar(pCurScript->curCmd->c.str, &Keep);
				if (Keep) pCurScript->curCmd->ptr = theVar;
				}

			Bool EvalsTo = theVar ? specialEvaluateBool(theVar) : evaluateBool(pCurScript->curCmd->c.str);

			if (pCurScript->curCmd->type == CMD_WHILENOT)
					{
					EvalsTo = !EvalsTo;
					}

			if (EvalsTo)
				{
				scriptLoops *theLoop = (scriptLoops *) addLL((void **)
						&(pCurScript->curContext->loopInfo), sizeof(*theLoop));

				if (theLoop)
					{
					theLoop->LoopType = (pCurScript->curCmd->type == CMD_WHILENOT) ? LOOP_WHILENOT : LOOP_WHILE;

					theLoop->where = pCurScript->curCmd;
					pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
					}
				else
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
						}
#else
					mPrintfCR(getscrmsg(13), pCurScript->Name, pCurScript->curCmd->line);
#endif
					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					}
				}
			else
				{
				int WhileCounter;
				scriptCommandRecord *theCmd;

				for (WhileCounter = 0, theCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
						theCmd; theCmd = (scriptCommandRecord *) getNextLL(theCmd))
					{
					if (theCmd->type == CMD_WHILE || theCmd->type == CMD_WHILENOT)
						{
						WhileCounter++;
						}
					else if (theCmd->type == CMD_WEND)
						{
						if (!WhileCounter)
							{
							pCurScript->curCmd = (scriptCommandRecord *) getNextLL(theCmd);
							break;
							}
						else
							{
							WhileCounter--;
							}
						}
					}

				if (!theCmd)
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->CRmPrintfCR(getscrmsg(53), pCurScript->Name, pCurScript->curCmd->line);
						}
#else
					CRmPrintfCR(getscrmsg(53), pCurScript->Name, pCurScript->curCmd->line);
#endif
					pCurScript->curCmd = NULL;
					pCurScript->ToRet = FALSE;
					}
				}

			break;
			}

		case CMD_WEND:
			{
			scriptLoops *theLoop;
			for (theLoop = pCurScript->curContext->loopInfo; theLoop && theLoop->next;
					theLoop = (scriptLoops *) getNextLL(theLoop));

			if (theLoop && (theLoop->LoopType == LOOP_WHILE || theLoop->LoopType == LOOP_WHILENOT))
				{
				pCurScript->curCmd = theLoop->where;

				deleteLLNode((void **) &(pCurScript->curContext->loopInfo),
						(int) getLLCount(pCurScript->curContext->loopInfo));
				}
			else
				{
#ifdef WINCIT
				if (pCurScript->TW)
					{
					pCurScript->TW->mPrintfCR(getscrmsg(52), pCurScript->Name, pCurScript->curCmd->line);
					}
#else
				mPrintfCR(getscrmsg(52), pCurScript->Name, pCurScript->curCmd->line);
#endif
				pCurScript->curCmd = NULL;
				pCurScript->ToRet = FALSE;
				}

			break;
			}

		case CMD_EXIT:
			{
			pCurScript->ToRet = evaluateBool(pCurScript->curCmd->c.str);
			pCurScript->curCmd = NULL;
			break;
			}

		case CMD_LABEL:
		case CMD_ENDIF:
		case CMD_ENDSWITCH:
			{
			pCurScript->curCmd = (scriptCommandRecord *) getNextLL(pCurScript->curCmd);
			break;
			}

		default:
			{
#ifdef WINCIT
			if (pCurScript->TW)
				{
				pCurScript->TW->mPrintfCR(getscrmsg(16), pCurScript->Name,
						pCurScript->curCmd->line, pCurScript->curCmd->type);
				}
#else
			mPrintfCR(getscrmsg(16), pCurScript->Name, pCurScript->curCmd->line, pCurScript->curCmd->type);
#endif
			pCurScript->curCmd = NULL;
			pCurScript->ToRet = FALSE;
			break;
			}
		}
	}

static void (*findInternalFunction(const strList *callWith))(strList *)
	{
	int i, high, low, found = FALSE;
	const char *str = callWith->string; // speed
	const char **arry = (const char **) ScriptDData->next->aux; // more speed

	low = 0;
	high = sizeof(internalFunctions) / sizeof(internalFunctionRecord) - 1;

	for (i = high / 2; !found; i = ((high - low) / 2) + low)
		{
		int test = strcmpi(arry[i], str);

		if (test == 0)
			{
			found = TRUE;
			}
		else if (test > 0)
			{
			high = i - 1;
			}
		else
			{
			low = i + 1;
			}

		if (high < low)
			{
			break;
			}
		}

	if (found)
		{
		return (internalFunctions[i].func);
		}
	else
		{
#ifdef WINCIT
		if (pCurScript->TW)
			{
			pCurScript->TW->mPrintfCR(getscrmsg(14), pCurScript->Name, pCurScript->curCmd->line, callWith->string);
			}
#else
		mPrintfCR(getscrmsg(14), pCurScript->Name, pCurScript->curCmd->line, callWith->string);
#endif
		return (NULL);
		}
	}

static scriptFunctionList *findFunc(const char *name)
	{
	scriptFunctionList *cur;

	for (cur = pCurScript->sfBase; cur; cur = (scriptFunctionList *) getNextLL(cur))
		{
		if (SameString(name, cur->name))
			{
			break;
			}
		}

	return (cur);
	}

static scriptVariables *FindVarInList(const char *name, scriptVariables *Var)
	{
	for (; Var; Var = (scriptVariables *) getNextLL(Var))
		{
		if (SameString(Var->name, name))
			{
			break;
			}
		}

	return (Var);
	}

static scriptVariables *FindStructElement(scriptVariables *Base,
		const char *Name)
	{
	if (Base->type != TYPE_STRUCT)
		{
		return (NULL);
		}

	label SubName;
	CopyStringToBuffer(SubName, Name);
	const int PerPos = strpos('.', SubName);
	if (PerPos)
		{
		SubName[PerPos - 1] = 0;
		}

	scriptVariables *cur = FindVarInList(SubName, Base->v.s);

	if (cur && PerPos)
		{
		cur = FindStructElement(cur, Name + PerPos);
		}

	return (cur);
	}

// returns a script variable based on the name
static scriptVariables *getVar(const char *name, Bool *Keep)
	{
	*Keep = TRUE;

	// is it _RESULT?
	if (SameString(name, resultStr))
		{
		return (&pCurScript->lastResult);
		}

	label SubName;
	CopyStringToBuffer(SubName, name);

	scriptVariables *cur;

	const int PerPos = strpos('.', SubName);
	if (PerPos)
		{
		SubName[PerPos - 1] = 0;
		}

	if (pCurScript->curFunc)
		{
		// first search local vars
		cur = FindVarInList(SubName, pCurScript->curFunc->vars);
		if (cur)
			{
			if (PerPos)
				{
				cur = FindStructElement(cur, name + PerPos);
				}

			return (cur);
			}

		// Now, data local to this running of the function...
		*Keep = FALSE;

		// Look first at automatic variables
		cur = FindVarInList(SubName, pCurScript->curContext->AutoVars);
		if (cur)
			{
			if (PerPos)
				{
				cur = FindStructElement(cur, name + PerPos);
				}

			return (cur);
			}

		// then, try function's parameters
		for (int i = 0; i < MAXARGS; i++)
			{
			if (SameString(pCurScript->curFunc->argn[i], SubName))
				{
				if (PerPos)
					{
					return (FindStructElement(pCurScript->curContext->argv[i], name + PerPos));
					}
				else
					{
					return (pCurScript->curContext->argv[i]);
					}
				}
			}
		}

	*Keep = TRUE;

	// perhaps global? If not, then not found.
	cur = FindVarInList(SubName, pCurScript->gvBase);

	if (cur && PerPos)
		{
		cur = FindStructElement(cur, name + PerPos);
		}

	return (cur);
	}

short evaluateInt(const char *what)
	{
	scriptVariables *cur;

	if (what[0] == '_')
		{
		Bool Keep;
		cur = getVar(what, &Keep);

		if (cur->type >= TYPE_STRING_MIN && cur->type <= TYPE_STRING_MAX)
			{
			return ((short) atoi(cur->v.string));
			}

		return (cur->v.i);
		}
	else
		{
		return ((short) atoi(what));
		}
	}

ushort evaluateUint(const char *what)
	{
	scriptVariables *cur;

	if (what[0] == '_')
		{
		Bool Keep;
		cur = getVar(what, &Keep);

		if (cur->type >= TYPE_STRING_MIN && cur->type <= TYPE_STRING_MAX)
			{
			return ((ushort) atoi(cur->v.string));
			}

		return (cur->v.ui);
		}
	else
		{
		return ((ushort) atoi(what));
		}
	}

long evaluateLong(const char *what)
	{
	scriptVariables *cur;

	if (what[0] == '_')
		{
		Bool Keep;
		cur = getVar(what, &Keep);

		if (cur->type >= TYPE_STRING_MIN && cur->type <= TYPE_STRING_MAX)
			{
			return (atol(cur->v.string));
			}

		if (cur->type == TYPE_LONG || cur->type == TYPE_ULONG)
			{
			return (cur->v.l);
			}
		else
			{
			return ((long) cur->v.i);
			}
		}
	else
		{
		return (atol(what));
		}
	}

ulong evaluateUlong(const char *what)
	{
	scriptVariables *cur;

	if (what[0] == '_')
		{
		Bool Keep;
		cur = getVar(what, &Keep);

		if (cur->type >= TYPE_STRING_MIN && cur->type <= TYPE_STRING_MAX)
			{
			return ((ulong) atol(cur->v.string));
			}

		if (cur->type == TYPE_LONG || cur->type == TYPE_ULONG)
			{
			return (cur->v.ul);
			}
		else
			{
			return ((ulong) cur->v.ui);
			}
		}
	else
		{
		return ((ulong) atol(what));
		}
	}

static Bool specialEvaluateBool(const scriptVariables *var)
	{
	if (var->type >= TYPE_STRING_MIN && var->type <= TYPE_STRING_MAX)
		{
		return (evaluateBool(var->v.string));
		}

	return (var->v.b != 0);
	}

Bool evaluateBool(const char *what)
	{
	if (what[0] == '_')
		{
		Bool Keep;
		const scriptVariables *cur = getVar(what, &Keep);

		if (cur)
			{
			return (specialEvaluateBool(cur));
			}
		else
			{
			return (FALSE);
			}
		}
	else
		{
		if (SameString(what, getscrmsg(25)))
			{
			return (FALSE);
			}

		if (SameString(what, getscrmsg(26)))
			{
			return (TRUE);
			}

		return ((Bool) (atoi(what) != 0));
		}
	}


static const char *specialEvaluateString(const scriptVariables *var)
	{
	if (var->type >= TYPE_STRING_MIN && var->type <= TYPE_STRING_MAX)
		{
		return (var->v.string);
		}

	switch (var->type)
		{
		case TYPE_INT:
			{
			return (itoa(var->v.i, pCurScript->tmpstr, 10));
			}

		case TYPE_UINT:
			{
			return (itoa(var->v.ui, pCurScript->tmpstr, 10));
			}

		case TYPE_LONG:
			{
			return (ltoa(var->v.l, pCurScript->tmpstr, 10));
			}

		case TYPE_ULONG:
			{
#ifdef MINGW
			return (_ultoa(var->v.ul, pCurScript->tmpstr, 10));
#else
			return (ultoa(var->v.ul, pCurScript->tmpstr, 10));
#endif
			}

		case TYPE_BOOL:
			{
			return (var->v.b ? getscrmsg(26) : getscrmsg(25));
			}
		}

	return (ns);
	}


const char *evaluateString(const char *what)
	{
	if (what[0] == '_')
		{
		Bool Keep;
		const scriptVariables *cur = getVar(what, &Keep);

		if (cur)
			{
			return (specialEvaluateString(cur));
			}
		else
			{
			return (ns);
			}
		}
	else
		{
		return (what);
		}
	}


scriptVariables *FindStructTemplate(const char *Name)
	{
	const char *SName;

	if (strncmpi(Name, getscrmsg(10), 7) == SAMESTRING)
		{
		SName = Name + 7;
		}
	else
		{
		SName = Name;
		}

	return (FindVarInList(SName, pCurScript->structTemplates));
	}


static Bool CopyVars(scriptVariables **Dest, scriptVariables *Src)
	{
	for (; Src; Src = (scriptVariables *) getNextLL((void *) Src))
		{
		Bool Stop;

		if (Src->type == TYPE_STRUCTTEMPL)
			{
			Stop = !createVariable(Dest, Src->name, TYPE_STRUCT, FALSE, TRUE, FindStructTemplate(Src->v.string));
			}
		else
			{
			Stop = !createVariable(Dest, Src->name, Src->type, FALSE, FALSE, NULL);
			}

		if (Stop)
			{
			return (FALSE);
			}
		}

	return (TRUE);
	}

#ifdef WINCIT
#define HCTU()	(pCurScript->TW && pCurScript->TW->HaveConnectionToUser())
#else
#define HCTU()	HaveConnectionToUser()
#endif

void ExecuteScript(void)
	{
	const Bool HadConnectionToUser = HCTU();

	while (pCurScript->curCmd)
		{
		executeCurrentCommand();

		if (pCurScript->Error ||
				(HadConnectionToUser && !HCTU()) ||
#ifdef WINCIT
				(pCurScript->TW && pCurScript->TW->ExitToMsdos))
#else
				ExitToMsdos)
#endif
			{
			pCurScript->curCmd = NULL;
			pCurScript->ToRet = FALSE;
			}
		}
	}
