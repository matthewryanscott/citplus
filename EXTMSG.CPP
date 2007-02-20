// --------------------------------------------------------------------------
// Citadel: ExtMsg.CPP
//
// Stuff to deal with discardable messages.

#include "ctdl.h"
#pragma hdrstop

#include "extmsg.h"

MessageTrackerS ExtMessages[MSG_NUM];

// --------------------------------------------------------------------------
// Contents



void DumpUnlockedMessages(void)
	{
	for (int i = 0; i < MSG_NUM; i++)
		{
		if (!ExtMessages[i].UseCounter && ExtMessages[i].Data)
			{
			discardData(ExtMessages[i].Data);
			ExtMessages[i].Data = NULL;
			ExtMessages[i].Messages = NULL;
			}
		}
	}


static discardable *ReadMsgData(ExtMsgE WhichMessage)
	{
	switch (WhichMessage)
		{
		case MSG_NET:		return (readData(4));
		case MSG_SYSOP:		return (readData(5, 0, 0));
		case MSG_CONFIG:	return (readData(0, 0, 0));
		case MSG_ENTERCONF:	return (readData(18));
		case MSG_FSCONFIG:	return (readData(20));
		case MSG_CITWIN:	return (readData(21));
		case MSG_ZMODEM:	return (readData(19, 0, 2));
		case MSG_DEBUG:		return (readData(1, 3, 3));
		case MSG_F4SCREEN:	return (readData(1, 1, 1));
		case MSG_CMDLINE:	return (readData(6, 0, 0));
		case MSG_XMODEM:	return (readData(19, 3, 3));
		case MSG_VIEWLOG:	return (readData(6, 5, 5));
		case MSG_HELP:		return (readData(6, 6, 6));
		case MSG_MAILLIST:	return (readData(6, 8, 8));
		case MSG_CONSOLE:	return (readData(6, 9, 9));
		case MSG_TRANS:		return (readData(6, 10, 10));
		case MSG_OOM:		return (readData(6, 11, 11));
		case MSG_AUXMEM:	return (readData(6, 12, 12));
		case MSG_READMSG:	return (readData(6, 13, 13));
		case MSG_INFOFILE:	return (readData(6, 14, 14));
		case MSG_LOGIN:		return (readData(6, 15, 15));
		case MSG_TERMCAP:	return (readData(8, 1, 3));
        case MSG_TRAP:      return (readData(1, 2, 2));
		}

	assert(FALSE);
	return (NULL);
	}

Bool LockMessages(ExtMsgE WhichMessage)
	{
	if (!ExtMessages[WhichMessage].Data)
		{
		assert(!ExtMessages[WhichMessage].UseCounter);
		ExtMessages[WhichMessage].Data = ReadMsgData(WhichMessage);
		}

	if (ExtMessages[WhichMessage].Data)
		{
		ExtMessages[WhichMessage].UseCounter++;
		ExtMessages[WhichMessage].Messages = (const char **) ExtMessages[WhichMessage].Data->aux;
		return (TRUE);
		}
	else
		{
#if VERSION != RELEASE
		// If to catch the case where we run out of memory trying to lock
		// MSG_OOM. In released versions, we don't lock MSG_OOM.

		if (ExtMessages[MSG_OOM].UseCounter)
#endif
			{
			OutOfMemory(103);
			}

		return (FALSE);
		}
	}

void UnlockMessages(ExtMsgE WhichMessage)
	{
	assert(ExtMessages[WhichMessage].UseCounter);
	ExtMessages[WhichMessage].UseCounter--;
	}


// Note that these next two functions are not very "safe," as there are
// plenty of chances for crashout() to be called in the second one.
void PurgeAllSubMessages(void)
	{
	for (int i = 0; i < MSG_NUM; i++)
		{
		if (ExtMessages[i].Data)
			{
			assert(ExtMessages[i].UseCounter);
			discardData(ExtMessages[i].Data);
			ExtMessages[i].Data = NULL;
			ExtMessages[i].Messages = 0;
			}
		else
			{
			assert(!ExtMessages[i].UseCounter);
			}
		}
	}

void ReloadAllSubMessages(void)
	{
	// We turn off debug until the debug messages are loaded...
	Bool SaveDebug = debug;
	debug = FALSE;

	for (int i = 0; i < MSG_NUM; i++)
		{
		assert(!ExtMessages[i].Data);

		if (ExtMessages[i].UseCounter)
			{
			ExtMessages[i].Data = ReadMsgData((ExtMsgE) i);

			if (!ExtMessages[i].Data)
				{
				crashout(getmsg(630));
				}

			ExtMessages[i].Messages = (const char **) ExtMessages[i].Data->aux;
			}
		}

	debug = SaveDebug;
	}


// --------------------------------------------------------------------------
// read in the message file
Bool read_messages(void)
	{
	if ((msgs = readData(1, 0, 0)) == NULL)
		{
		return (FALSE);
		}

	msgsI = (const char **) msgs->aux;

#ifdef WINCIT
	if (!SameString(getmsg(0), "W"))
		{
		MessageBox(NULL, "Data not created for Win32 version.", NULL, MB_ICONSTOP | MB_OK);
		return (FALSE);
		}
#else
#ifdef AUXMEM
	if (!SameString(getmsg(0), "A"))
		{
		printf("Data not created for Auxmem version.");
		return (FALSE);
		}
#else
	if (!SameString(getmsg(0), "R"))
		{
		printf("Data not created for Regular version.");
		return (FALSE);
		}
#endif
#endif

#if VERSION != RELEASE
	if (!LockMessages(MSG_OOM))
		{
		return (FALSE);
		}
#endif

#ifdef AUXMEM
	if (!LockMessages(MSG_AUXMEM))
		{
		return (FALSE);
		}
#endif

	if ((dateInfo = readData(3)) == NULL)
		{
		discardData(msgs);
		msgs = NULL;
		return (FALSE);
		}

	monthTab = (const char **) dateInfo->aux;
	fullmnts = (const char **) dateInfo->next->aux;
	days = (const char **) dateInfo->next->next->aux;
	fulldays = (const char **) dateInfo->next->next->next->aux;
	amPtr = ((const char **) (dateInfo->next->next->next->next->aux))[0];
	pmPtr = ((const char **) (dateInfo->next->next->next->next->aux))[1];

	return (TRUE);
	}
