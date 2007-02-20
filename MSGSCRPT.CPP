// --------------------------------------------------------------------------
// Citadel: MsgScrpt.CPP
//
// Message script

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "script.h"

// Make copy of message into our own place
void TERMWINDOWMEMBER PrepareMsgScriptInit(Message *Msg)
	{
	delete MsgScriptMsg;
	MsgScriptMsg = new Message(*Msg);
	}

// If we have a message, copy it to passed message, get rid of ours
Bool TERMWINDOWMEMBER CheckMsgScript(Message *Msg)
	{
	if (MsgScriptMsg)
		{
		*Msg = *MsgScriptMsg;

		delete MsgScriptMsg;
		MsgScriptMsg = NULL;

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

// Make copy of script's message if it has one.
void TERMWINDOWMEMBER DeinitMsgScript(ScriptInfoS *si)
	{
	delete MsgScriptMsg;

	if (si->UsrMsg)
		{
		MsgScriptMsg = new Message(*si->UsrMsg);
		}
	else
		{
		MsgScriptMsg = NULL;
		}
	}

// Give script our message, and then forget about it.
void TERMWINDOWMEMBER InitMsgScript(ScriptInfoS *si)
	{
	si->UsrMsg = MsgScriptMsg;
	MsgScriptMsg = NULL;

#ifndef WINCIT
	si->DeinitScript = DeinitMsgScript;
#endif
	}
