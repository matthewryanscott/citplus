// --------------------------------------------------------------------------
// Citadel: ScrFunc.CPP
//
// General-purpose Script Functions

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "events.h"
#include "net.h"
#include "tallybuf.h"
#include "miscovl.h"
#include "domenu.h"
#include "carrier.h"

// --------------------------------------------------------------------------
// Contents
//
// sfPrintm()			prints to modem/console
// sfPrintc()			prints to console

extern ConsoleLockC ConsoleLock;

void sfPrintm(strList *params)
	{
	RequiresScriptTW();

	for (; params; params = (strList *) getNextLL(params))
		{
		ScriptTW()mPrintf(pcts, evaluateString(params->string));
		}
	}

void sfOutstrpaced(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		ScriptTW()CommPort->OutString(evaluateString(params->string), params->next ? evaluateInt(params->next->string) : 0);
		}
	}

void sfPrintc(strList *params)
	{
	RequiresScriptTW();

	for (; params; params = (strList *) getNextLL(params))
		{
		ScriptTW()cPrintf(pcts, evaluateString(params->string));
		}
	}

void sfGetstr(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		const char *prompt = evaluateString(params->string);
		params = (strList *) getNextLL(params);

		if (params)
			{
			char dfault[256];

			strcpy(dfault, evaluateString(params->string));
			params = (strList *) getNextLL(params);

			if (params)
				{
				const int i = min(evaluateInt(params->string), 255);

				ScriptTW()getString(prompt, pCurScript->wowza, i, FALSE, dfault);
				pCurScript->lastResult.type = TYPE_STRING_MAX;
				pCurScript->lastResult.v.string = pCurScript->wowza;
				}
			}
		}
	}

void sfWaitfor(strList *params)
	{
	RequiresScriptTW();

	if (params && params->next)
		{
		SETBOOL(ScriptTW()wait_for(evaluateString(params->string), evaluateInt(params->next->string)));
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfRandom(strList *params)
	{
	if (params)
		{
		SETLONG(random(evaluateInt(params->string)));
		}
	}

void sfSetRandom(strList *)
	{
#ifdef VISUALC
    srand( (unsigned)time( NULL ) );
#else
	randomize();
#endif
	}

void sfDoCR(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		ScriptTW()doCR(evaluateInt(params->string));
		}
	else
		{
		ScriptTW()doCR();
		}
	}

void sfDoccr(strList *)
	{
	RequiresScriptTW();
	ScriptTW()doccr();
	}

void sfDiskfree(strList *params)
	{
#ifdef WINCIT
	char Root[4];
	Root[0] = *evaluateString(params->string);
	Root[1] = ':';
	Root[2] = '\\';
	Root[3] = 0;

	SETLONG(CitGetDiskFreeSpace(*Root == '@' ? NULL : Root));
#else
	union REGS REG;

	REG.h.ah = 0x36;
	REG.h.dl = toupper(*evaluateString(params->string)) - '@';
	intdos(&REG, &REG);

	SETLONG((long) REG.x.cx * (long) REG.x.ax * (long) REG.x.bx);
#endif
	}

void sfThisroom(strList *)
	{
	RequiresScriptTW();
	SETINT(ScriptTW()thisRoom);
	}

void sfAdduser(strList *params)
	{
	RequiresScriptTW();

	if (params && params->next)
		{
		ScriptTW()groupfunc(evaluateString(params->next->string), evaluateString(params->string), TRUE);
		}
	}

void sfRemoveuser(strList *params)
	{
	RequiresScriptTW();

	if (params && params->next)
		{
		ScriptTW()groupfunc(evaluateString(params->next->string), evaluateString(params->string), FALSE);
		}
	}

void sfAskyn(strList *params)
	{
	RequiresScriptTW();

	if (params && params->next)
		{
		SETLONG(ScriptTW()getYesNo(evaluateString(params->string), evaluateInt(params->next->string)));
		}
	}

// prompt, keys, dflt, ...
void sfGetOneKey(strList *params)
	{
	RequiresScriptTW();

	char Prompt[256];
	char Keys[37];
	char dfault;
	label RepList[36];

	CopyStringToBuffer(Prompt, evaluateString(params->string));
	CopyStringToBuffer(Keys, evaluateString(params->next->string));
	dfault = evaluateString(params->next->next->string)[0];

	strList *wow = params->next->next->next;

	for (int rlC = 0; rlC < 36; rlC++)
		{
		if (wow)
			{
			CopyStringToBuffer(RepList[rlC], evaluateString(wow->string));
			wow = (strList *) getNextLL(wow);
			}
		else
			{
			*RepList[rlC] = 0;
			}
		}

	int Result = ScriptTW()GetOneKey(Prompt, Keys, dfault, MAXBLBS, RepList[0], RepList[1], RepList[2], RepList[3],
			RepList[4], RepList[5], RepList[6], RepList[7], RepList[8], RepList[9], RepList[10], RepList[11], RepList[12],
			RepList[13], RepList[14], RepList[15], RepList[16], RepList[17], RepList[18], RepList[19], RepList[20],
			RepList[21], RepList[22], RepList[23], RepList[24], RepList[25], RepList[26], RepList[27], RepList[28],
			RepList[29], RepList[30], RepList[31], RepList[32], RepList[33], RepList[34], RepList[35]);

	SETLONG(Result);
	}

void sfRunapplic(strList *params)
	{
	RequiresScriptTW();

	const Bool ChDir = params->next ? evaluateBool(params->next->string) : TRUE;
	SETLONG(ScriptTW()RunApplication(evaluateString(params->string), NULL, TRUE, ChDir));
	}

void sfChatreq(strList *)
	{
	RequiresScriptTW();
	if (LockMenu(MENU_CHAT))  
		{
		SETBOOL(ScriptTW()ringSysop());
		UnlockMenu(MENU_CHAT);
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

#ifdef WINCIT

DWORD playWAVEFile(LPCSTR lpszWAVEFileName);
DWORD playCDTrack(BYTE bTrack);
DWORD playMIDIFile(LPCSTR lpszMIDIFileName);

#endif

void sfPlaysound(strList *params)
	{
#ifdef WINCIT
	switch (evaluateInt(params->string))
		{
		case 1:		// Wave
			{
			SETLONG((long) playWAVEFile(evaluateString(params->next->string)));
			break;
			}

		case 2:		// MIDI
			{
			SETLONG((long) playMIDIFile(evaluateString(params->next->string)));
			break;
			}

		case 3:		// CD
			{
			SETLONG((long) playCDTrack(evaluateInt(params->next->string)));
			break;
			}

		default:
			{
			// Fake it
			SETLONG(MCIERR_INVALID_DEVICE_NAME);
			break;
			}
		}
#else
	(*start_snd_src)(evaluateInt(params->string), evaluateString(params->next->string));
	SETINT((*play_sound)(evaluateInt(params->string)));
#endif
	}

void sfSayascii(strList *params)
	{
#ifndef WINCIT
	SETINT((*say_ascii)(evaluateString(params->string), strlen(evaluateString(params->string))));
#endif
	}

void sfGetconnectrate(strList *)
	{
	RequiresScriptTW();
	SETLONG(connectbauds[ScriptTW()CommPort->GetModemSpeed()]);
	}

void sfGetportrate(strList *)
	{
	RequiresScriptTW();
	SETLONG(bauds[ScriptTW()CommPort->GetSpeed()]);
	}


void sfSetportrate(strList *params)
	{
	RequiresScriptTW();

	Bool Ret;

	if (params)
		{
		PortSpeedE PS = digitbaud(evaluateLong(params->string));

		if (PS != PS_ERROR)
			{
			ScriptTW()CommPort->SetSpeed(PS);
			Ret = TRUE;
			}
		else
			{
			Ret = FALSE;
			}
		}
	else
		{
		ScriptTW()CommPort->SetSpeed(cfg.initbaud);
		Ret = TRUE;
		}

	SETBOOL(Ret);
	}

void sfChat(strList *)
	{
	RequiresScriptTW();
	ScriptTW()chat();
	}

void sfDochat(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()doChat(evaluateBool(params->string));
	}

void sfGetparam(strList *params)
	{
	char line[256];
	char *words[256];
	int count, i = evaluateInt(params->next->string) - 1;

	strcpy(line, evaluateString(params->string));
	count = parse_it(words, line);

	if (i < count)
		{
		SETSTR(words[i]);
		}
	else
		{
		SETSTR(ns);
		}
	}

void sfGetparamcount(strList *params)
	{
	char line[256];
	char *words[256];

	strcpy(line, evaluateString(params->string));
	SETLONG(parse_it(words, line));
	}

void sfIchar(strList *)
	{
	RequiresScriptTW();

	char wow[2];
	wow[1] = 0;

	wow[0] = (char) ScriptTW()iChar();

	SETSTR(wow);
	}

void sfIcharne(strList *)
	{
	RequiresScriptTW();

	char wow[2];
	wow[1] = 0;

	wow[0] = (char) ScriptTW()iCharNE();

	SETSTR(wow);
	}

void sfGetModemStr(strList *params)
	{
	RequiresScriptTW();

	char wow[256];

	uint len = (params) ? evaluateUint(params->string) : sizeof(wow) - 1;

	if (len > sizeof(wow) - 1)
		{
		len = sizeof(wow) - 1;
		}

	getModStr(wow, len, ScriptTW()CommPort);

	SETSTR(wow);
	}

void sfDumpf(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()dumpf(evaluateString(params->string), evaluateBool(params->next->string), FALSE);
	}

void sfDumpfc(strList *params)
	{
	RequiresScriptTW()
	ScriptTW()dumpfc(evaluateString(params->string), evaluateBool(params->next->string));
	}

void sfPause(strList *params)
	{
	pause(evaluateInt(params->string));
	}

void sfGetFormattedString(strList *params)
	{
	RequiresScriptTW();

	char Prompt[256];
	char Result[256];

	strcpy(Prompt, evaluateString(params->string));

	ScriptTW()getFmtString(Prompt, Result, evaluateBool(params->next->string), evaluateBool(params->next->next->string));

	SETSTR(Result);
	}

void sfAsciitochar(strList *params)
	{
	char wow[2];
	wow[1] = 0;

	wow[0] = evaluateInt(params->string);

	SETSTR(wow);
	}

void sfChartoascii(strList *params)
	{
	SETINT(evaluateString(params->string)[0]);
	}

void sfSformat(strList *params)
	{
	char RepList[10][255];

	strList *wow = params->next->next;

	for (int rlC = 0; rlC < 10; rlC++)
		{
		if (wow)
			{
			strcpy(RepList[rlC], evaluateString(wow->string));
			wow = (strList *) getNextLL(wow);
			}
		else
			{
			*RepList[rlC] = 0;
			}
		}

	char Result[256];
	sformat(Result, evaluateString(params->string), evaluateString(params->next->string), RepList[0], RepList[1],
			RepList[2], RepList[3], RepList[4], RepList[5], RepList[6], RepList[7], RepList[8], RepList[9], 0);

	SETSTR(Result);
	}

void sfStrftime(strList *params)
	{
	char Result[256];

	strftime(Result, evaluateInt(params->next->next->string), evaluateString(params->string),
			evaluateLong(params->next->string));

	SETSTR(Result);
	}

void sfGetcurrenttime(strList *)
	{
	SETLONG(time(NULL));
	}

void sfHangup(strList *)
	{
	RequiresScriptTW();
	ScriptTW()Hangup();
	}

void sfModemflush(strList *)
	{
	RequiresScriptTW();
	ScriptTW()CommPort->FlushInput();
	}

void sfModeminitport(strList *)
	{
	RequiresScriptTW();
	ScriptTW()Initport();
	}

void sfGetContextSensitiveText(strList *)
	{
	RequiresScriptTW();
	SETSTR(ScriptTW()ContextSensitiveScriptText);
	}

void sfMemoryFree(strList *)
	{
#ifdef WINCIT
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatus(&ms);
	SETLONG(ms.dwAvailVirtual);
#else
	SETLONG(farcoreleft());
#endif
	}

void sfStartDupCheck(strList *)
	{
	disposeLL((void **) &pCurScript->dupCheck);
	}

void sfAddDupCheck(strList *params)
	{
	strList *cur = (strList *) addLL((void **) &pCurScript->dupCheck,
			sizeof(strList) + strlen(evaluateString(params->string)));

	if (cur)
		{
		strcpy(cur->string, evaluateString(params->string));
		}

	SETBOOL(cur != NULL);
	}

void sfCheckForDup(strList *params)
	{
	strList *cur;
	for (cur = pCurScript->dupCheck; cur; cur = (strList *) getNextLL(cur))
		{
		if (SameString(cur->string, evaluateString(params->string)))
			{
			break;
			}
		}

	SETBOOL(cur != NULL);
	}

void sfSetOutImpervious(strList *)
	{
	RequiresScriptTW();
	ScriptTW()OC.SetOutFlag(IMPERVIOUS);
	}

void sfSetOutNormal(strList *)
	{
	RequiresScriptTW();
	ScriptTW()OC.SetOutFlag(OUTOK);
	}

void sfSetOutNostop(strList *)
	{
	RequiresScriptTW();
	ScriptTW()OC.SetOutFlag(NOSTOP);
	}

void sfIsCanOutput(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()OC.User.CanOutput());
	}

void sfCheckUserOutControl(strList *params)
	{
	RequiresScriptTW();
#ifdef MULTI
	SETBOOL(ScriptTW()OC.CheckInput(evaluateBool(params->string), pCurScript->TW));
#else
	SETBOOL(ScriptTW()OC.CheckInput(evaluateBool(params->string)));
#endif
	}

void sfIsOutputStop(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()OC.User.GetOutFlag() == OUTSKIP);
	}

void sfIsOutputNext(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()OC.User.GetOutFlag() == OUTNEXT);
	}

void sfIsOutputJump(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()OC.User.GetOutFlag() == OUTPARAGRAPH);
	}

void sfKeyboardFlush(strList *)
	{
	RequiresScriptTW();
	ScriptTW()KeyboardBuffer.Flush();
	}

void sfKeyboardAdd(strList *params)
	{
	RequiresScriptTW();

	Bool Good = TRUE;

	for (; Good && params; params = (strList *) getNextLL(params))
		{
		const char *str = evaluateString(params->string);
		const int slen = strlen(str);

		for (int i = 0; i < slen; i++)
			{
			if (!ScriptTW()KeyboardBuffer.Add(str[i] == '\n' ? '\r' : str[i]))
				{
				Good = FALSE;
				break;
				}
			}
		}
	}

void sfExitCitadel(strList *params)
	{
	RequiresScriptTW();
	return_code = evaluateInt(params->string);
	ScriptTW()ExitToMsdos = TRUE;
	}

void sfIsConsoleLocked(strList *)
	{
	SETBOOL(ConsoleLock.IsLocked());
	}

void sfSetConsoleLock(strList *params)
	{
	if (evaluateBool(params->string))
		{
		ConsoleLock.Lock();
		}
	else
		{
		ConsoleLock.Unlock();
		}
	}

void sfIsLoggedIn(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()loggedIn);
	}

void sfAddCommasToNumber(strList *params)
	{
	RequiresScriptTW();
	SETSTR(ScriptTW()ltoac(evaluateLong(params->string)));
	}

void sfStrPos(strList *params)
	{
	char c = *evaluateString(params->string);
	SETLONG(strpos(c, evaluateString(params->next->string)));
	}

void sfHexToDec(strList *params)
	{
	SETLONG(strtol(evaluateString(params->string), NULL, 16));
	}

void sfDecToHex(strList *params)
	{
	const int Width = params->next ? evaluateInt(params->next->string) : 0;
	char buf[256];

	sprintf(buf, "%0*lX", Width, evaluateLong(params->string));

	SETSTR(buf);
	}

void sfHowLongAgoWasIt(strList *params)
	{
	RequiresScriptTW();
	SETSTR(ScriptTW()diffstamp(evaluateLong(params->string)));
	}

void sfParseDateString(strList *params)
	{
	char DateString[256];

	CopyStringToBuffer(DateString, evaluateString(params->string));

	datestruct FillDate;

	gdate(DateString, &FillDate);

	struct date dt;
	struct time tm;

	tm.ti_min = 0;
	tm.ti_hour = 0;
	tm.ti_sec = 0;

	if (FillDate.Date)
		{
		dt.da_year = FillDate.Year + 1900;			// i hate dostounix
		dt.da_day = (char) FillDate.Date;
		dt.da_mon = (char) (FillDate.Month + 1); 	// i hate dostounix
		}
	else
		{
		dt.da_year = 1970;	// i hate dostounix
		dt.da_day = 1;
		dt.da_mon = 1;		// i hate dostounix
		}

	SETLONG(dostounix(&dt, &tm));
	}

void sfParseTimeString(strList *params)
	{
	char TimeString[256];

	CopyStringToBuffer(TimeString, evaluateString(params->string));

	timestruct FillTime;

	gtime(TimeString, &FillTime);

	if (FillTime.Hour >= 0)
		{
		struct date dt;
		struct time tm;

		tm.ti_min = (char) FillTime.Minute;
		tm.ti_hour = (char) FillTime.Hour;
		tm.ti_sec = (char) FillTime.Second;

		dt.da_year = 1970;	// i hate dostounix
		dt.da_day = 1;
		dt.da_mon = 1;		// i hate dostounix

		SETLONG(dostounix(&dt, &tm));
		}
	else
		{
		SETLONG(0);
		}
	}

void sfIsMatchWildcard(strList *params)
	{
	SETBOOL(u_match(evaluateString(params->string), evaluateString(params->next->string)));
	}

void sfSetRoomBypassed(strList *params)
	{
	RequiresScriptTW();
	ScriptTW()Talley->SetBypass(evaluateInt(params->string), evaluateBool(params->next->string));
	}

void sfIsRoomBypassed(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()Talley->Bypassed(evaluateInt(params->string)));
	}

void sfHasTermWindow(strList *)
	{
#ifdef WINCIT
	SETBOOL(pCurScript->TW != NULL);
#else
	SETBOOL(TRUE);
#endif
	}

void sfNormalizeString(strList *params)
	{
	CopyStringToBuffer(pCurScript->wowza, evaluateString(params->string));
	normalizeString(pCurScript->wowza);
	SETRTSTR();
	}

void sfGetMsgReadOption(strList *params)
	{
	RequiresScriptTW();

	switch (evaluateInt(params->string))
		{
		case 0:
			{
			SETBOOL(ScriptTW()MRO.Number);
			break;
			}

		case 1:
			{
			SETBOOL(ScriptTW()MRO.Headerscan);
			break;
			}

		case 2:
			{
			SETBOOL(ScriptTW()MRO.Date);
			break;
			}

		case 3:
			{
			SETBOOL(ScriptTW()MRO.DatePrevious);
			break;
			}

		case 4:
			{
			SETBOOL(ScriptTW()MRO.All);
			break;
			}

		case 5:
			{
			SETBOOL(ScriptTW()MRO.Verbose);
			break;
			}

		case 6:
			{
			SETBOOL(ScriptTW()MRO.Reverse);
			break;
			}

		case 7:
			{
			SETLONG(ScriptTW()MRO.CheckDate);
			break;
			}

		case 8:
			{
			SETLONG(ScriptTW()MRO.MessageNumber);
			break;
			}

		case 9:
			{
			SETBOOL(ScriptTW()rf.Mail);
			break;
			}

		case 10:
			{
			SETBOOL(ScriptTW()rf.Group);
			break;
			}

		case 11:
			{
			SETBOOL(ScriptTW()rf.Public);
			break;
			}

		case 12:
			{
			SETBOOL(ScriptTW()rf.Local);
			break;
			}

		case 13:
			{
			SETBOOL(ScriptTW()rf.User);
			break;
			}

		case 14:
			{
			SETBOOL(ScriptTW()rf.Text);
			break;
			}

		case 15:
			{
			SETSTR(ScriptTW()rf.SearchUser);
			break;
			}

		case 16:
			{
			SETSTR(ScriptTW()rf.SearchText);
			break;
			}
		}
	}

void sfSetMsgReadOption(strList *params)
	{
	RequiresScriptTW();

	switch (evaluateInt(params->string))
		{
		case 0:
			{
			ScriptTW()MRO.Number = evaluateBool(params->next->string);
			break;
			}

		case 1:
			{
			ScriptTW()MRO.Headerscan = evaluateBool(params->next->string);
			break;
			}

		case 2:
			{
			ScriptTW()MRO.Date = evaluateBool(params->next->string);
			break;
			}

		case 3:
			{
			ScriptTW()MRO.DatePrevious = evaluateBool(params->next->string);
			break;
			}

		case 4:
			{
			ScriptTW()MRO.All = evaluateBool(params->next->string);
			break;
			}

		case 5:
			{
			ScriptTW()MRO.Verbose = evaluateBool(params->next->string);
			break;
			}

		case 6:
			{
			ScriptTW()MRO.Reverse = evaluateBool(params->next->string);
			break;
			}

		case 7:
			{
			ScriptTW()MRO.CheckDate = evaluateLong(params->next->string);
			break;
			}

		case 8:
			{
			ScriptTW()MRO.MessageNumber = evaluateLong(params->next->string);
			break;
			}

		case 9:
			{
			ScriptTW()rf.Mail = evaluateBool(params->next->string);
			break;
			}

		case 10:
			{
			ScriptTW()rf.Group = evaluateBool(params->next->string);
			break;
			}

		case 11:
			{
			ScriptTW()rf.Public = evaluateBool(params->next->string);
			break;
			}

		case 12:
			{
			ScriptTW()rf.Local = evaluateBool(params->next->string);
			break;
			}

		case 13:
			{
			ScriptTW()rf.User = evaluateBool(params->next->string);
			break;
			}

		case 14:
			{
			ScriptTW()rf.Text = evaluateBool(params->next->string);
			break;
			}

		case 15:
			{
			CopyStringToBuffer(ScriptTW()rf.SearchUser, evaluateString(params->next->string));
			break;
			}

		case 16:
			{
			CopyStringToBuffer(ScriptTW()rf.SearchText, evaluateString(params->next->string));
			break;
			}
		}
	}

void sfGetProgramName(strList *)
	{
	SETSTR(programName);
	}

void sfGetProgramAuthor(strList *)
	{
	SETSTR(Author);
	}

void sfGetCompileDate(strList *)
	{
	SETSTR(cmpDate);
	}

void sfGetCompileTime(strList *)
	{
	SETSTR(cmpTime);
	}

void sfResetOutParagraph(strList *)
	{
	RequiresScriptTW();
	ScriptTW()OC.ResetOutParagraph();
	}

void sfGetUpTime(strList *)
	{
	SETLONG(uptimestamp);
	}

void sfHaveConnection(strList *)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()CommPort->HaveConnection());
	}

void sfGetConnectionTime(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()conntimestamp);
	}

void sfGetLoginTime(strList *)
	{
	RequiresScriptTW();
	SETLONG(ScriptTW()logtimestamp);
	}
