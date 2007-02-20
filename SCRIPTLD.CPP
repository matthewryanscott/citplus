// --------------------------------------------------------------------------
// Citadel: ScriptLd.CPP
//
// The script loader.


#include "ctdl.h"
#pragma hdrstop

#include "script.h"
#include "scrfarry.h"
#include "msg.h"
#include "log.h"

// --------------------------------------------------------------------------
// Contents
//
// runScript()	runs a script file


static VariableTypeE parseType(const char *type);
static void dumpScript(void);
static int runCounter;

// this next stuff is for starting up a script
extern const char *resultStr;

static struct poo
	{
	char *foo;
	char doo[6];
	} xxxcall = {NULL, "_MAIN"};        // Moo!

static scriptCommandRecord xxxcmd2 = {NULL,		0, CMD_EXIT, NULL, 0, 0, (strList *) resultStr};
static scriptCommandRecord xxxcmd1 = {&xxxcmd2,	0, CMD_CALL, NULL, 0, 0, (strList *) &xxxcall};

DEBUGCODE(static Bool tableChecked;)


// Script.cpp
void ExecuteScript(void);
scriptVariables *createVariable(scriptVariables **Base, const char *Name, VariableTypeE Type, Bool MakeTempl,
		Bool NeedTempl, scriptVariables *structTempl);
void dumpVars(scriptVariables *base);

static Bool loadScript(void)
	{
	VerifyHeap();

	FILE *scr;

	if ((scr = fopen(pCurScript->Name, FO_R)) != NULL)	// ASCII mode
		{
		Bool noMemory = FALSE;
		scriptFunctionList *theFunc = NULL;

		int linenum = 0;
		char ScriptFileLine[128], *ScriptFileLineStart;

		scriptVariables *NewStruct = NULL;

		while (fgets(ScriptFileLine, 128, scr) && !noMemory && !pCurScript->Error)
			{
			linenum++;
			ScriptFileLineStart = ScriptFileLine;

			while (*ScriptFileLineStart && (*ScriptFileLineStart < 33 && *ScriptFileLineStart > 0))
				{
				ScriptFileLineStart++;
				}

			if (*ScriptFileLineStart != '#')
				{
				continue;
				}

			char *words[128];
			const int count = parse_it(words, ScriptFileLineStart);

			int i;
			for (i = 0; i < S_NUM; i++)
				{
				if (SameString(words[0] + 1, ((char **) ScriptDData->aux)[i]))
					{
					break;
					}
				}

			switch (i)
				{
				case S_VERSION:
					{
					if (count > 1)
						{
						const long Needed = atol(words[1]);

						if (Needed > NumericVer)
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(17), Needed / 1000, Needed % 1000);
								}
#else
							mPrintf(getscrmsg(17), Needed / 1000, Needed % 1000);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_GVAR:
					{
					if (count > 2 && (NewStruct || words[2][0] == '_'))
						{
						if (!createVariable(NewStruct ? &NewStruct->v.s : &pCurScript->gvBase, words[2], parseType(words[1]),
								!!NewStruct, TRUE, NewStruct ? NewStruct : FindStructTemplate(words[1])))
							{
							noMemory = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_AVAR:
				case S_LVAR:
					{
					if (NewStruct || theFunc)
						{
						if (count > 2 && (NewStruct || words[2][0] == '_'))
							{
							scriptVariables **Base = NewStruct ? &(NewStruct->v.s) : i == S_LVAR ? &(theFunc->vars) :
									&(theFunc->AutoVars);

							if (!createVariable(Base, words[2], parseType(words[1]), NewStruct || (i == S_AVAR), TRUE,
									NewStruct ? NewStruct : FindStructTemplate(words[1])))
								{
								noMemory = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_FUNC:
					{
					if (!NewStruct)
						{
						if (count > 2 && words[1][0] == '_')
							{
							theFunc = (scriptFunctionList *) addLL((void **) &pCurScript->sfBase, sizeof(*theFunc));

							if (theFunc)
								{
								theFunc->rett = parseType(words[2]);

								CopyStringToBuffer(theFunc->name, words[1]);

								for (int j = 0; j < MAXARGS; j++)
									{
									if (count > j * 2 + 4)
										{
										theFunc->argt[j] = parseType(words[j * 2 + 3]);

										CopyString2Buffer(theFunc->argn[j], words[j * 2 + 4], sizeof(label));
										}
									}
								}
							else
								{
								noMemory = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_ENDFUNC:
					{
					if (!NewStruct)
						{
						theFunc = NULL;
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}
					break;
					}

				case S_ADD:
				case S_SUB:
				case S_MUL:
				case S_DIV:
				case S_MOD:
				case S_CALL:
				case S_AND:
				case S_OR:
				case S_XOR:
					{
					if (!NewStruct)
						{
						if (theFunc)
							{
							if (count > 2 || (i == S_CALL && count == 2))
								{
								scriptCommandRecord *theCmd = (scriptCommandRecord *) addLL((void **) &(theFunc->cmds),
										sizeof(*(theFunc->cmds)));

								if (theCmd)
									{
									switch (i)
										{
										case S_ADD:	theCmd->type = CMD_ADD;	break;
										case S_SUB:	theCmd->type = CMD_SUB;	break;
										case S_MUL:	theCmd->type = CMD_MUL;	break;
										case S_DIV:	theCmd->type = CMD_DIV;	break;
										case S_MOD:	theCmd->type = CMD_MOD;	break;
										case S_CALL:theCmd->type = CMD_CALL;break;
										case S_AND:	theCmd->type = CMD_AND;	break;
										case S_OR:	theCmd->type = CMD_OR;	break;
										case S_XOR:	theCmd->type = CMD_XOR;	break;
										}

									theCmd->line = linenum;

									for (int j = 1; j < count && !noMemory; j++)
										{
										strList *theSL = (strList *) addLL((void **) &(theCmd->c.sl),
												sizeof(*(theCmd->c.sl)) + strlen(words[j]));

										if (theSL)
											{
											strcpy(theSL->string, words[j]);
											}
										else
											{
											noMemory = TRUE;
											}
										}
									}
								else
									{
									noMemory = TRUE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
									}
#else
								mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
								pCurScript->Error = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_ASGN:
				case S_EQ:
				case S_GT:
				case S_LT:
				case S_NE:
				case S_GE:
				case S_LE:
					{
					if (!NewStruct)
						{
						if (theFunc)
							{
							if (count == 3 && (i != S_ASGN || words[1][0] == '_'))
								{
								scriptCommandRecord *theCmd = (scriptCommandRecord *) addLL((void **) &(theFunc->cmds),
										sizeof(*(theFunc->cmds)));

								if (theCmd)
									{
									if (i == S_ASGN)
										{
										theCmd->type = CMD_ASGN;
										}
									else if (i == S_EQ)
										{
										theCmd->type = CMD_EQ;
										}
									else if (i == S_GT)
										{
										theCmd->type = CMD_GT;
										}
									else if (i == S_LT)
										{
										theCmd->type = CMD_LT;
										}
									else if (i == S_NE)
										{
										theCmd->type = CMD_NE;
										}
									else if (i == S_GE)
										{
										theCmd->type = CMD_GE;
										}
									else
										{
										theCmd->type = CMD_LE;
										}

									theCmd->line = linenum;

									for (int j = 1; j < count && !noMemory; j++)
										{
										strList *theSL = (strList *) addLL((void **) &(theCmd->c.sl),
												sizeof(*(theCmd->c.sl)) + strlen(words[j]));

										if (theSL)
											{
											strcpy(theSL->string, words[j]);
											}
										else
											{
											noMemory = TRUE;
											}
										}
									}
								else
									{
									noMemory = TRUE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
									}
#else
								mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
								pCurScript->Error = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_STR:
					{
					if (!NewStruct)
						{
						if (theFunc)
							{
							if (count > 2)
								{
								scriptCommandRecord *theCmd = (scriptCommandRecord *) addLL((void **) &(theFunc->cmds),
										sizeof(*(theFunc->cmds)));

								if (theCmd)
									{
									switch (toupper(words[1][0]))
										{
										case 'B':
											{
											theCmd->type = CMD_STRB;
											break;
											}

										case 'U':
											{
											theCmd->type = CMD_STRU;
											break;
											}

										case 'L':
											{
											theCmd->type = CMD_STRL;
											break;
											}

										case 'C':
											{
											theCmd->type = CMD_STRC;
											break;
											}

										case 'S':
											{
											theCmd->type = CMD_STRS;
											break;
											}

										case 'E':
											{
											theCmd->type = CMD_STRE;
											break;
											}

										case 'P':
											{
											if (toupper(words[1][1]) == 'R')
												{
												theCmd->type = CMD_STRPR;
												break;
												}
											else if (toupper(words[1][1]) == 'L')
												{
												theCmd->type = CMD_STRPL;
												break;
												}
											}

										default:
											{
#ifdef WINCIT
											if (pCurScript->TW)
												{
												pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
												}
#else
											mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
											pCurScript->Error = TRUE;
											break;
											}
										}

									if (!pCurScript->Error)
										{
										theCmd->line = linenum;

										for (int j = 2; j < count && !noMemory; j++)
											{
											strList *theSL = (strList *) addLL((void **) &(theCmd->c.sl),
													sizeof(*(theCmd->c.sl)) + strlen(words[j]));

											if (theSL)
												{
												strcpy(theSL->string, words[j]);
												}
											else
												{
												noMemory = TRUE;
												}
											}
										}
									}
								else
									{
									noMemory = TRUE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
									}
#else
								mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
								pCurScript->Error = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_WHILE:
				case S_WHILENOT:
				case S_LOOP:
				case S_LOOPNOT:
				case S_IF:
				case S_IFNOT:
				case S_GOTO:
				case S_LABEL:
				case S_RET:
				case S_EXIT:
				case S_RUN:
				case S_CHAIN:
				case S_SWITCH:
				case S_CASE:
					{
					if (!NewStruct)
						{
						if (theFunc)
							{
							if (count == 2)
								{
								scriptCommandRecord *theCmd = (scriptCommandRecord *) addLL((void **) &(theFunc->cmds),
										sizeof(*(theFunc->cmds)));

								if (theCmd)
									{
									if (i == S_WHILE)
										{
										theCmd->type = CMD_WHILE;
										}
									else if (i == S_WHILENOT)
										{
										theCmd->type = CMD_WHILENOT;
										}
									else if (i == S_LOOP)
										{
										theCmd->type = CMD_LOOP;
										}
									else if (i == S_LOOPNOT)
										{
										theCmd->type = CMD_LOOPNOT;
										}
									else if (i == S_IF)
										{
										theCmd->type = CMD_IF;
										}
									else if (i == S_IFNOT)
										{
										theCmd->type = CMD_IFNOT;
										}
									else if (i == S_GOTO)
										{
										theCmd->type = CMD_GOTO;
										}
									else if (i == S_RET)
										{
										theCmd->type = CMD_RET;
										}
									else if (i == S_EXIT)
										{
										theCmd->type = CMD_EXIT;
										}
									else if (i == S_RUN)
										{
										theCmd->type = CMD_RUN;
										}
									else if (i == S_CHAIN)
										{
										theCmd->type = CMD_CHAIN;
										}
									else if (i == S_SWITCH)
										{
										theCmd->type = CMD_SWITCH;
										}
									else if (i == S_CASE)
										{
										theCmd->type = CMD_CASE;
										}
									else
										{
										theCmd->type = CMD_LABEL;
										}

									theCmd->line = linenum;

									theCmd->c.str = strdup(words[1]);

									if (!theCmd->c.str)
										{
										noMemory = TRUE;
										}
									}
								else
									{
									noMemory = TRUE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
									}
#else
								mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
								pCurScript->Error = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_WEND:
				case S_DO:
				case S_ELSE:
				case S_ENDIF:
				case S_DEFAULT:
				case S_ENDSWITCH:
					{
					if (!NewStruct)
						{
						if (theFunc)
							{
							if (count == 1)
								{
								scriptCommandRecord *theCmd = (scriptCommandRecord *) addLL((void **) &(theFunc->cmds),
										sizeof(*(theFunc->cmds)));

								if (theCmd)
									{
									if (i == S_WEND)
										{
										theCmd->type = CMD_WEND;
										}
									else if (i == S_DO)
										{
										theCmd->type = CMD_DO;
										}
									else if (i == S_ELSE)
										{
										theCmd->type = CMD_ELSE;
										}
									else if (i == S_DEFAULT)
										{
										theCmd->type = CMD_DEFAULT;
										}
									else if (i == S_ENDSWITCH)
										{
										theCmd->type = CMD_ENDSWITCH;
										}
									else
										{
										theCmd->type = CMD_ENDIF;
										}

									theCmd->line = linenum;
									}
								else
									{
									noMemory = TRUE;
									}
								}
							else
								{
#ifdef WINCIT
								if (pCurScript->TW)
									{
									pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
									}
#else
								mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
								pCurScript->Error = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(2), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_STRUCT:
					{
					if (!NewStruct)
						{
						if (count == 2)
							{
							NewStruct = createVariable(&(pCurScript->structTemplates), words[1], TYPE_STRUCT, TRUE, FALSE,
									NULL);

							if (!NewStruct)
								{
								noMemory = TRUE;
								}
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
							}
#else
						mPrintfCR(getscrmsg(56), pCurScript->Name, linenum, words[0]);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				case S_ENDSTRUCT:
					{
					if (NewStruct)
						{
						if (count == 1)
							{
							NewStruct = NULL;
							}
						else
							{
#ifdef WINCIT
							if (pCurScript->TW)
								{
								pCurScript->TW->mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
								}
#else
							mPrintf(getscrmsg(1), pCurScript->Name, linenum, words[0]);
#endif
							pCurScript->Error = TRUE;
							}
						}
					else
						{
#ifdef WINCIT
						if (pCurScript->TW)
							{
							pCurScript->TW->mPrintfCR(getscrmsg(58), pCurScript->Name, linenum);
							}
#else
						mPrintfCR(getscrmsg(58), pCurScript->Name, linenum);
#endif
						pCurScript->Error = TRUE;
						}

					break;
					}

				default:
					{
#ifdef WINCIT
					if (pCurScript->TW)
						{
						pCurScript->TW->mPrintfCR(getscrmsg(3), pCurScript->Name, linenum, words[0]);
						}
#else
					mPrintfCR(getscrmsg(3), pCurScript->Name, linenum, words[0]);
#endif
					break;
					}
				}
			}

		fclose(scr);

		if (noMemory || pCurScript->Error)
			{
			return (FALSE);
			}

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

#ifdef WINCIT
Bool runScript(const char *what, TermWindowC *TermWindow, void (*InitScript)(ScriptInfoS *si))
#else
Bool runScript(const char *what, void (*InitScript)(ScriptInfoS *si))
#endif
	{
	VerifyHeap();

	ScriptInfoS *SaveScript = pCurScript;
	CurScript = (long) new ScriptInfoS;

	if (!pCurScript)
		{
		OutOfMemory(80);
		CurScript = (long) SaveScript;
		return (FALSE);
		}

	memset(pCurScript, 0, sizeof(*pCurScript));

#ifdef WINCIT
	pCurScript->TW = TermWindow;
#endif

//	pCurScript->Error = FALSE;

	runCounter++;

	if (!ScriptDData)
		{
		if (cfg.FastScripts)
			{
			compactMemory(2);
			}

		ScriptDData = readData(2);

		if (!ScriptDData)
			{
			dumpScript();
			OutOfMemory(80);

			delete pCurScript;
			CurScript = (long) SaveScript;

			return (FALSE);
			}
		}

#ifndef NDEBUG
	if (!tableChecked)
		{
		CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, "Checking function names... ");

		const char **arry = (const char **) ScriptDData->next->aux;

		char LastOne[128];
		*LastOne = 0;

		for (int i = 0; internalFunctions[i].func; i++)
			{
			if (*LastOne)
				{
				assert(strcmpi(LastOne, arry[i]) < 0);
				}

			CopyStringToBuffer(LastOne, arry[i]);

			for (const char *ptr = LastOne; *ptr; ptr++)
				{
				assert(isdigit(*ptr) || isupper(*ptr) || *ptr == '_');
				}
			}

		tableChecked = TRUE;

		if (w)
			{
			destroyCitWindow(w, FALSE);
			}
		}
#endif

	if (strchr(what, '\\') || strchr(what, ':'))
		{
		strcpy(pCurScript->Name, what);
		}
	else
		{
		sprintf(pCurScript->Name, sbs, cfg.ScriptPath, what);
		}

	if (pCurScript->Name[strlen(pCurScript->Name) - 1] == '*')
		{
		pCurScript->Debug = pCurScript->Pause = TRUE;
		pCurScript->Name[strlen(pCurScript->Name) - 1] = 0;
		}
	else
		{
		pCurScript->Debug = pCurScript->Pause = FALSE;
		}

	pCurScript->SingleStep = FALSE;
	pCurScript->oldSystemDebug = debug;

	if (!strchr(pCurScript->Name, '.'))
		{
		strcat(pCurScript->Name, getscrmsg(30));
		}

	strupr(pCurScript->Name);

	compactMemory();

	// load script into memory...
	if (loadScript())
		{
		// then let our custom initialization happen...
		if (InitScript)
			{
			(*InitScript)(pCurScript);
			}

		// okay -- now run it
		xxxcmd1.ptr = NULL;
		xxxcmd2.ptr = NULL;

		pCurScript->curCmd = &xxxcmd1;

		ExecuteScript();

		dumpScript();

		if (pCurScript->ReadAplOnExit)
			{
#ifdef WINCIT
			if (pCurScript->TW)
				{
				pCurScript->TW->readAplFile();
				}
#else
			readAplFile();
#endif
			}

		Bool toRet = pCurScript->ToRet;

		delete pCurScript;
		CurScript = (long) SaveScript;

		return (toRet);
		}
	else
		{
#ifdef WINCIT
		if (pCurScript->TW)
			{
			pCurScript->TW->CRmPrintfCR(getscrmsg(54), pCurScript->Name);
			}
#else
		CRmPrintfCR(getscrmsg(54), pCurScript->Name);
#endif
		dumpScript();

		delete pCurScript;
		CurScript = (long) SaveScript;

		return (FALSE);
		}
	}

static void dumpScript(void)
	{
	VerifyHeap();

	// Run any de-init we have set for this script.
	if (pCurScript->DeinitScript)
		{
		(*pCurScript->DeinitScript)(pCurScript);
		}

	runCounter--;

	int i;
	scriptFunctionList *theFunc, *theFunc2;

	if (!runCounter && !cfg.FastScripts)
		{
		discardData(ScriptDData);
		ScriptDData = NULL;
		}

	if (pCurScript->DebuggingWindow)
		{
		destroyCitWindow(pCurScript->DebuggingWindow, FALSE);
		pCurScript->DebuggingWindow = NULL;
		}

	disposeLL((void **) &pCurScript->BPoints);
	disposeLL((void **) &pCurScript->callList);

	dumpVars(pCurScript->gvBase);
	pCurScript->gvBase = NULL;

	// then dump the functions
	for (theFunc = pCurScript->sfBase; theFunc; theFunc = theFunc2)
		{
		dumpVars(theFunc->vars);
		dumpVars(theFunc->AutoVars);

		FunctionContext *Context, *Context2;
		for (Context = theFunc->Context; Context; Context = Context2)
			{
			dumpVars(Context->AutoVars);
			disposeLL((void **) &(Context->loopInfo));

			for (i = 0; i < MAXARGS; i++)
				{
				dumpVars(Context->argv[i]);
				}

			Context2 = Context->next;
			delete Context;
			}


		scriptCommandRecord *theCmd, *theCmd2;
		for (theCmd = theFunc->cmds; theCmd; theCmd = theCmd2)
			{
			switch (theCmd->type)
				{
				case CMD_WHILE:
				case CMD_WHILENOT:
				case CMD_LOOP:
				case CMD_LOOPNOT:
				case CMD_IF:
				case CMD_IFNOT:
				case CMD_GOTO:
				case CMD_LABEL:
				case CMD_RET:
				case CMD_EXIT:
				case CMD_RUN:
				case CMD_CHAIN:
					{
					delete [] theCmd->c.str;
					break;
					}

				case CMD_ADD:
				case CMD_SUB:
				case CMD_MUL:
				case CMD_DIV:
				case CMD_MOD:
				case CMD_CALL:
				case CMD_ASGN:
				case CMD_EQ:
				case CMD_GT:
				case CMD_LT:
				case CMD_NE:
				case CMD_GE:
				case CMD_LE:
				case CMD_STRB:
				case CMD_STRU:
				case CMD_STRL:
				case CMD_STRC:
				case CMD_STRS:
				case CMD_STRE:
				case CMD_STRPR:
				case CMD_STRPL:
					{
					disposeLL((void **) &theCmd->c.sl);
					break;
					}
				}

			theCmd2 = theCmd->next;

			delete theCmd;
			}

		theFunc2 = theFunc->next;
		delete theFunc;
		}

	pCurScript->sfBase = NULL;

	delete pCurScript->UsrMsg;
	pCurScript->UsrMsg = NULL;

	if (!pCurScript->UseCU)
		{
		delete pCurScript->UsrUsr;
		}

	pCurScript->UseCU = FALSE;
	pCurScript->UsrUsr = NULL;

	disposeLL((void **) &pCurScript->dupCheck);

	delete [] pCurScript->Files;
	pCurScript->Files = NULL;

	for (i = 0; i < 5; i++)
		{
		if (pCurScript->UsrFile[i])
			{
			fclose(pCurScript->UsrFile[i]);
			pCurScript->UsrFile[i] = NULL;
			}
		}

	*pCurScript->Name = 0;

	VerifyHeap();
	}

static VariableTypeE parseType(const char *a_type)
	{
	VariableTypeE type = TYPE_NONE;

	if (SameString(a_type, getscrmsg(18)))
		{
		type = TYPE_NONE;
		}
	if (strncmpi(a_type, getscrmsg(19), 3) == SAMESTRING)
		{
		type = TYPE_INT;
		}
	else if (strncmpi(a_type, getscrmsg(20), 4) == SAMESTRING)
		{
		type = TYPE_UINT;
		}
	else if (strncmpi(a_type, getscrmsg(21), 4) == SAMESTRING)
		{
		type = TYPE_LONG;
		}
	else if (strncmpi(a_type, getscrmsg(22), 5) == SAMESTRING)
		{
		type = TYPE_ULONG;
		}
	else if (strncmpi(a_type, getscrmsg(23), 4) == SAMESTRING)
		{
		type = TYPE_BOOL;
		}
	else if (strncmpi(a_type, getscrmsg(24), 6) == SAMESTRING)
		{
		if (strchr(a_type, '_'))
			{
			type = TYPE_STRING_MAX;
			}
		else
			{
			int i = atoi(a_type + 6);
			if (i < 1 || i > 255)
				{
				i = 255;
				}

			type = (VariableTypeE) (TYPE_STRING_MIN + i - 1);
			}
		}
	else if (strncmpi(a_type, getscrmsg(10), 7) == SAMESTRING)
		{
		type = TYPE_STRUCT;
		}

	if (strchr(a_type, '_'))
		{
		type = (VariableTypeE) (type | BY_REF);
		}

	return (type);
	}
