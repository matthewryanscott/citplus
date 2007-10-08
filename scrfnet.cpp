// --------------------------------------------------------------------------
// Citadel: ScrFNet.CPP
//
// The networking-related internal script functions.

#include "ctdl.h"
#pragma hdrstop


#include "scrfunc.h"
#include "log.h"
#include "net.h"
#include "tallybuf.h"
#include "extmsg.h"


// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ NETWORK STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
void sfNet69fetch(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		const char *rnode = evaluateString(params->string);
//		LogEntry SaveLog(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
		l_slot oldthisLog;
		Bool OldLoggedIn;

		ModemConsoleE W;
		if (params->next)
			{
			W = (ModemConsoleE) evaluateInt(params->next->string);
			}
		else
			{
			W = MODEM;
			}

		if (!read_net_messages())
			{
			ScriptTW()wPrintf(W, getmsg(59));
			return;
			}

		oldthisLog = ScriptTW()ThisLog;
//		SaveLog = *ScriptTW()CurrentUser;
    	ScriptTW()storeLog();
		OldLoggedIn = ScriptTW()loggedIn;

#ifdef WINCIT
		if (ReadNodesCit(((ScriptInfoS *) CurScript)->TW,
				&ScriptTW()node, rnode, NOMODCON, TRUE))
#else
		if (ReadNodesCit(&node, rnode, NOMODCON, TRUE))
#endif
			{
			if (ScriptTW()LoadPersonByName(ScriptTW()node->GetName()))
				{
				LogEntry5 Log5(cfg.maxrooms);

				ScriptTW()loggedIn = TRUE;

				ScriptTW()amZap();

				Log5.CopyRoomNewPointers(*ScriptTW()CurrentUser);

				if (!ScriptTW()net69_fetch(TRUE, W))
					{
					ScriptTW()CurrentUser->CopyRoomNewPointers(Log5);
					}

				ScriptTW()SaveAideMess(NULL);

				ScriptTW()CurrentUser->SetCallNumber(cfg.callno);
				time(&ScriptTW()logtimestamp);
				ScriptTW()CurrentUser->SetCallTime(ScriptTW()logtimestamp);
#ifdef WINCIT
				slideLTab(&ScriptTW()ThisSlot, ScriptTW()CurrentUser,
						ScriptTW()LogOrder);
#else
				slideLTab(&ThisSlot, CurrentUser);
#endif
				ScriptTW()storeLog();

				ScriptTW()ThisLog = oldthisLog;
         		ScriptTW()CurrentUser->Load(ScriptTW()ThisLog);
//				*ScriptTW()CurrentUser = SaveLog;
				ScriptTW()loggedIn = OldLoggedIn;

				ScriptTW()Talley->Fill();
				}
			}

		dump_net_messages();

		freeNode(&ScriptTW()node);
		}
	}

void sfNet69incorporate(strList *params)
	{
	RequiresScriptTW();

	ModemConsoleE W;
	if (params)
		{
		W = (ModemConsoleE) evaluateInt(params->string);
		}
	else
		{
		W = MODEM;
		}

	if (!read_net_messages())
		{
		ScriptTW()wPrintf(W, getmsg(59));
		return;
		}

	ScriptTW()amZap();
	ScriptTW()net69_incorporate(W);
	ScriptTW()SaveAideMess(NULL);

	ScriptTW()Talley->Fill();

	dump_net_messages();

	freeNode(&ScriptTW()node);
	}

void sfNet86fetch(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		const char *rnode = evaluateString(params->string);
		LogEntry SaveLog(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
		l_slot oldthisLog;
		Bool OldLoggedIn;

		ModemConsoleE W;
		if (params->next)
			{
			W = (ModemConsoleE) evaluateInt(params->next->string);
			}
		else
			{
			W = MODEM;
			}

		if (!read_net_messages())
			{
			ScriptTW()wPrintf(W, getmsg(59));
			return;
			}

		oldthisLog = ScriptTW()ThisLog;
		SaveLog = *ScriptTW()CurrentUser;
		OldLoggedIn = ScriptTW()loggedIn;

#ifdef WINCIT
		if (ReadNodesCit(((ScriptInfoS *) CurScript)->TW,
				&ScriptTW()node, rnode, NOMODCON, FALSE))
#else
		if (ReadNodesCit(&node, rnode, NOMODCON, FALSE))
#endif
			{
			if (ScriptTW()LoadPersonByName(ScriptTW()node->GetName()))
				{
				LogEntry5 Log5(cfg.maxrooms);

				ScriptTW()loggedIn = TRUE;

				ScriptTW()amZap();

				Log5.CopyRoomNewPointers(*ScriptTW()CurrentUser);

				if (!ScriptTW()net86_fetch(W))
					{
					ScriptTW()CurrentUser->CopyRoomNewPointers(Log5);
					}

				ScriptTW()SaveAideMess(NULL);

				ScriptTW()CurrentUser->SetCallNumber(cfg.callno);
				time(&ScriptTW()logtimestamp);
				ScriptTW()CurrentUser->SetCallTime(ScriptTW()logtimestamp);
#ifdef WINCIT
				slideLTab(&ScriptTW()ThisSlot, ScriptTW()CurrentUser,
						ScriptTW()LogOrder);
#else
				slideLTab(&ThisSlot, CurrentUser);
#endif
				ScriptTW()storeLog();

				ScriptTW()ThisLog = oldthisLog;
				*ScriptTW()CurrentUser = SaveLog;
				ScriptTW()loggedIn = OldLoggedIn;

				ScriptTW()Talley->Fill();
				}
			}

		dump_net_messages();

		freeNode(&ScriptTW()node);
		}
	}

void sfNet86incorporate(strList *params)
	{
	RequiresScriptTW();

	if (params)
		{
		ModemConsoleE W;
		if (params->next)
			{
			W = (ModemConsoleE) evaluateInt(params->next->string);
			}
		else
			{
			W = MODEM;
			}

		if (!read_net_messages())
			{
			ScriptTW()wPrintf(W, getmsg(59));
			return;
			}

		const char *rnode = evaluateString(params->string);

		if (*rnode &&
#ifdef WINCIT
			ReadNodesCit(((ScriptInfoS *) CurScript)->TW,
					&ScriptTW()node, rnode, NOMODCON, FALSE))
#else
			ReadNodesCit(&node, rnode, NOMODCON, FALSE))
#endif
			{
			ScriptTW()amZap();
			ScriptTW()net86_incorporate(W);
			ScriptTW()SaveAideMess(NULL);
			ScriptTW()Talley->Fill();
			}

		dump_net_messages();

		freeNode(&ScriptTW()node);
		}
	}

void sfDial(strList *params)
	{
	RequiresScriptTW();

	if (params && params->next)
		{
		if (!read_net_messages())
			{
			ScriptTW()cPrintf(getmsg(59));
			return;
			}

		SETLONG(ScriptTW()dial(evaluateString(params->string),
				evaluateInt(params->next->string)));

		dump_net_messages();
		}
	}

void sfIsNetIDInNetIDCit(strList *params)
	{
	SETBOOL(IsNetIDInNetIDCit(evaluateString(params->string)));
	}

void sfAddNetIDToNetIDCit(strList *params)
	{
	label NetID;

	CopyStringToBuffer(NetID, evaluateString(params->string));

	SETBOOL(AddNetIDToNetIDCit(NetID, evaluateString(params->next->string)));
	}

void sfRemoveNetIDFromNetIDCit(strList *params)
	{
	SETBOOL(RemoveNetIDFromNetIDCit(evaluateString(params->string)));
	}

void sfGetCommentOfNetIDInNetIDCit(strList *params)
	{
	char Comment[256];

	GetCommentOfNetIDInNetIDCit(evaluateString(params->string), Comment,
			sizeof(Comment));

	SETSTR(Comment);
	}

void sfStringIsAddress(strList *params)
	{
	SETBOOL(isaddress(evaluateString(params->string)));
	}

void sfFindNodeByNameOrAddress(strList *params)
	{
#ifdef WINCIT
	RequiresScriptTW();
	SETINT(nodexists(evaluateString(params->string), ScriptTW()LogOrder));
#else
	SETINT(nodexists(evaluateString(params->string)));
#endif
	}

void sfDoNetwork(strList *params)
	{
	RequiresScriptTW();
	SETBOOL(ScriptTW()net_callout(evaluateString(params->string)));
	}

void sfNet69RoomReq(strList *params)
	{
	RequiresScriptTW();

	ModemConsoleE W;
	if (params->next)
		{
		W = (ModemConsoleE) evaluateInt(params->next->string);
		}
	else
		{
		W = MODEM;
		}

	if (!read_net_messages())
		{
		ScriptTW()wPrintf(W, getmsg(59));
		SETBOOL(FALSE);
		return;
		}

	const char *rnode = evaluateString(params->string);

	if (*rnode)
		{
		NetLogEntryC Log;

		if (Log.IsValid())
			{
#ifdef WINCIT
			if (Log.LoadByName(rnode, ScriptTW()LogOrder))
#else
			if (Log.LoadByName(rnode))
#endif
				{
#ifdef WINCIT
				SETBOOL(Log.MakeNet69RoomRequestFile(NOMODCON,
						pCurScript->TW));
#else
				SETBOOL(Log.MakeNet69RoomRequestFile(NOMODCON));
#endif
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

	dump_net_messages();
	}
