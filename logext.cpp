#include "ctdl.h"
#pragma hdrstop

#include "net.h"
#include "log.h"
#include "room.h"

void LogExtensions::ClearMessage(void)
	{
	SYNCDATA(LE_CLEARMESSAGE, 0, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	delete Msg;
	Msg = NULL;
	RELEASEEXCLUSIVEACCESS();
	}

void LogExtensions::Clear(void)
	{
	assert(this);
	GAINEXCLUSIVEACCESS();
	VerifyHeap();

	for (int i = 0; i < KF_NUM; i++)
		{
		disposeLL((void **) &Kill[i]);
		}

	disposeLL((void **) &le_tuser);
	disposeLL((void **) &le_replace);
	disposeLL((void **) &le_dict);
	disposeLL((void **) &le_userdef);
	delete [] le_Finger;
	delete Msg;
	le_Finger = NULL;
	Msg = NULL;
	jb_start = jb_end = 0;

#ifdef WINCIT
	SyncNumber = CERROR;
#endif

	VerifyHeap();
	RELEASEEXCLUSIVEACCESS();
	}

void LogExtensions::SetMessage(Message *NewMsg)
	{
	SYNCDATA(LE_SETMESSAGE, NewMsg, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();

	delete Msg;

	if (NewMsg)
		{
		Msg = new Message;

		if (Msg)
			{
			*Msg = *NewMsg;
			}
		}
	else
		{
		Msg = NULL;
		}
	RELEASEEXCLUSIVEACCESS();
	}

Bool LogExtensions::IsWordInDictionary(const char *Word) const
	{
	assert(this);

	GAINEXCLUSIVEACCESS();
	assert(Word);

	for (strList *userDict = le_dict; userDict; userDict = (strList *) getNextLL(userDict))
		{
		if (SameString(Word, userDict->string))
			{
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::RemoveUserTag(const char *User)
	{
	SYNCDATAR(LE_REMOVEUSERTAG, User, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(User);

	int i;
	pairedStrings *theTag;

	for (i = 1, theTag = le_tuser; theTag; theTag = (pairedStrings *) getNextLL(theTag), i++)
		{
		if (SameString(theTag->string1, User))
			{
			deleteLLNode((void **) &le_tuser, i);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::RemoveReplace(const char *Replace)
	{
	SYNCDATAR(LE_REMOVEREPLACE, Replace, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	int i;
	pairedStrings *theRepl;

	for (i = 1, theRepl = le_replace; theRepl; theRepl = (pairedStrings *) getNextLL(theRepl), i++)
		{
		if (SameString(theRepl->string1, Replace))
			{
			deleteLLNode((void **) &le_replace, i);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::RemoveWordFromDictionary(const char *Word)
	{
	SYNCDATAR(LE_REMOVEWORDFROMDICT, Word, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Word);
	int i;
	strList *theStr;

	for (i = 1, theStr = le_dict; theStr; theStr = (strList *) getNextLL(theStr), i++)
		{
		if (SameString(theStr->string, Word))
			{
			deleteLLNode((void **) &le_dict, i);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

LogExtensions& LogExtensions::operator =(const LogExtensions &Original)
	{
	assert(this);
	GAINEXCLUSIVEACCESS();

	VerifyHeap();

	Clear();

	// Don't sync while copying!
	WINCODE(SyncNumber = CERROR);

	strcpy(FileExtension, Original.FileExtension);
	jb_start = Original.jb_start;
	jb_end = Original.jb_end;
	jb_length = Original.jb_length;

	VerifyHeap();

	delete [] jb;
	jb = NULL;

	if (Original.Msg)
		{
		Msg = new Message;

		if (Msg)
			{
			*Msg = *Original.Msg;
			}
		}

	VerifyHeap();

	if (Original.jb)
		{
		jb = new jumpback[jb_length];

		if (jb)
			{
			memcpy(jb, Original.jb, sizeof(jumpback) * jb_length);
			}
		}

	if (Original.le_Finger)
		{
		le_Finger = strdup(Original.le_Finger);
		}

	VerifyHeap();

	strList *theStr;

	for (int i = 0; i < KF_NUM; i++)
		{
		for (theStr = Original.Kill[i]; theStr; theStr = (strList *) getNextLL(theStr))
			{
			AddKill((KillFileE) i, theStr->string);
			}
		}

	for (theStr = Original.le_dict; theStr; theStr = (strList *) getNextLL(theStr))
		{
		AddWordToDictionary(theStr->string);
		}

	pairedStrings *thePair;

	for (thePair = Original.le_tuser; thePair; thePair = (pairedStrings *) getNextLL(thePair))
		{
		AddTagUser(thePair->string1, thePair->string2);
		}

	for (thePair = Original.le_replace; thePair; thePair = (pairedStrings *) getNextLL(thePair))
		{
		AddReplace(thePair->string1, thePair->string2);
		}

	for (userDefLE *theUserDef = Original.le_userdef; theUserDef; theUserDef = (userDefLE *) getNextLL(theUserDef))
		{
		SetUserDefined(theUserDef->Code, theUserDef->Data);
		}

	WINCODE(SyncNumber = Original.SyncNumber);

	RELEASEEXCLUSIVEACCESS();
	return (*this);
	}

LogExtensions::LogExtensions(const LogExtensions &Original)
	{
	INITIALIZEEXCLUSIVEACCESS(TRUE);
	SAVEEXCLUSIVEACCESSRECORD();

	memset(this, 0, sizeof(*this));

	RESTOREEXCLUSIVEACCESSRECORD();

	*this = Original;

	RELEASEEXCLUSIVEACCESS();
	}

Bool LogExtensions::AddKill(KillFileE Type, const char *String)
	{
	SYNCDATAR(LE_ADDKILL, Type, String);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(String);

	if (*String)
		{
		if (IsKill(Type, String))
			{
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		else
			{
			strList *theStrLL = (strList *) addLL((void **) &Kill[Type], sizeof(*theStrLL) + strlen(String));

			if (theStrLL)
				{
				strcpy(theStrLL->string, String);
				RELEASEEXCLUSIVEACCESS();
				return (TRUE);
				}
			}
		}

	RELEASEEXCLUSIVEACCESS();

	return (FALSE);
	}

Bool LogExtensions::RemoveKill(KillFileE Type, const char *String)
	{
	SYNCDATAR(LE_REMOVEKILL, Type, String);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(String);

	long i;
	strList *theStr;

	for (i = 1, theStr = Kill[Type]; theStr; theStr = (strList *) getNextLL(theStr), i++)
		{
		if (SameString(theStr->string, String))
			{
			deleteLLNode((void **) &Kill[Type], i);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}


Bool LogExtensions::AddTagUser(const char *User, const char *Tag)
	{
	SYNCDATAR(LE_ADDTAGUSER, User, Tag);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(User);
	assert(Tag);

	if (*User && *Tag)
		{
		pairedStrings *theTag = (pairedStrings *) addLL((void **) &le_tuser, sizeof(*theTag));

		if (theTag)
			{
			CopyStringToBuffer(theTag->string1, User);
			CopyStringToBuffer(theTag->string2, Tag);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::AddReplace(const char *Orig, const char *Repl)
	{
	SYNCDATAR(LE_ADDREPLACE, Orig, Repl);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Orig);
	assert(Repl);

	if (*Orig && *Repl)
		{
		pairedStrings *theRepl = (pairedStrings *) addLL((void **) &le_replace, sizeof(*theRepl));

		if (theRepl)
			{
			CopyStringToBuffer(theRepl->string1, Orig);
			CopyStringToBuffer(theRepl->string2, Repl);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::AddWordToDictionary(const char *Word)
	{
	SYNCDATAR(LE_ADDWORDTODICT, Word, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Word);

	if (*Word)
		{
		if (IsWordInDictionary(Word))
			{
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		else
			{
			strList *theStrLL = (strList *) addLL((void **) &le_dict, sizeof(*theStrLL) + strlen(Word));

			if (theStrLL)
				{
				strcpy(theStrLL->string, Word);
				strupr(theStrLL->string);
				RELEASEEXCLUSIVEACCESS();
				return (TRUE);
				}
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

Bool LogExtensions::SetUserDefined(const char *Code, const char *Data)
	{
	SYNCDATAR(LE_SETUSERDEFINED, Code, Data);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Code);
	assert(Data);

	RemoveUserDefined(Code);

	userDefLE *NewDefined = (userDefLE *) addLL((void **) &le_userdef, sizeof(userDefLE) + strlen(Data));

	if (NewDefined)
		{
		CopyStringToBuffer(NewDefined->Code, Code);
		strcpy(NewDefined->Data, Data);
		RELEASEEXCLUSIVEACCESS();
		return (TRUE);
		}
	else
		{
		RELEASEEXCLUSIVEACCESS();
		return (FALSE);
		}
	}

Bool LogExtensions::RemoveUserDefined(const char *Code)
	{
	SYNCDATAR(LE_REMOVEUSERDEFINED, Code, 0);

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Code);

	long i;
	userDefLE *theDef;

	for (i = 1, theDef = le_userdef; theDef; theDef = (userDefLE *) getNextLL(theDef), i++)
		{
		if (SameString(theDef->Code, Code))
			{
			deleteLLNode((void **) &le_userdef, i);
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

const char *LogExtensions::GetUserDefined(const char *Code, char *Buffer, int Length) const
	{
	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Code);
	assert(Buffer);
	assert(Length);

	for (userDefLE *theDef = le_userdef; theDef; theDef = (userDefLE *) getNextLL(theDef))
		{
		if (SameString(theDef->Code, Code))
			{
			CopyString2Buffer(Buffer, theDef->Data, Length);
			RELEASEEXCLUSIVEACCESS();
			return (Buffer);
			}
		}

	*Buffer = 0;
	RELEASEEXCLUSIVEACCESS();
	return (NULL);
	}

Bool LogExtensions::IsUserDefined(const char *Code) const
	{
	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(Code);

	for (userDefLE *theDef = le_userdef; theDef; theDef = (userDefLE *) getNextLL(theDef))
		{
		if (SameString(theDef->Code, Code))
			{
			RELEASEEXCLUSIVEACCESS();
			return (TRUE);
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}

ulong LogExtensions::GetTagUserCount(const char *User) const
	{
	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(User);

	ulong Counter = 0;

	for (pairedStrings *Cur = le_tuser; Cur; Cur = (pairedStrings *) getNextLL(Cur))
		{
		if (SameString(Cur->string1, User))
			{
			Counter++;
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (Counter);
	}

Bool LogExtensions::GetUserTag(ulong Num, const char *User, char *Buffer, int Length) const
	{
	if (!Num)
		{
		return (FALSE);
		}

	assert(this);
	GAINEXCLUSIVEACCESS();
	assert(User);
	assert(Buffer);
	assert(Length);

	for (pairedStrings *Cur = le_tuser; Cur; Cur = (pairedStrings *) getNextLL(Cur))
		{
		if (SameString(Cur->string1, User))
			{
			if (!--Num)
				{
				CopyString2Buffer(Buffer, Cur->string2, Length);
				RELEASEEXCLUSIVEACCESS();
				return (TRUE);
				}
			}
		}

	RELEASEEXCLUSIVEACCESS();
	return (FALSE);
	}
