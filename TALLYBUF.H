// --------------------------------------------------------------------------
// Citadel: TallyBuf.H
//
// How to use the talley buffer subsystem.

#include "auxtab.h"

struct TalleyRoomInfoS
	{
	m_slot	Total;					// total number of messages in the room
	m_slot	Messages;				// # messages user can see in room
	m_slot	New;					// # messages new to the user in room

#if defined(AUXMEM) || defined(WINCIT)
	int		MsgRoom;				// # messages user left this call
#else
	uint	MsgRoom 	: 8;		// # messages user left this call
#endif
	Bool	HasMail 	: 1;		// if user has new mail in the room
	Bool	HasOldMail	: 1;		// if user has new mail in the room
	Bool	Visited 	: 1;		// if user has seen the room this call
	Bool	Bypassed	: 1;		// if user has bypassed room this call
#ifdef AUXMEM
    AUXMEMWINCODE(Bool Valid : 1;)
#endif
	};


class TalleyBufferC
	{
#ifdef AUXMEM
	#define TALLEYBUFPERPAGE	(AUXPAGESIZE / sizeof(TalleyRoomInfoS))

	typedef struct
		{
		int 	talleyBlocksInHeap;
		auxTabList *tbList;
		} talleyTaskInfo;

#else
	TalleyRoomInfoS *Buffer;
#endif

	r_slot Size;

	TalleyRoomInfoS *GetEntry(r_slot Room)
		{
		assert(Room >= 0 && Room < Size);

#ifdef AUXMEM
        return ((TalleyRoomInfoS *) (LoadAuxmemBlock(Room, &(((talleyTaskInfo *) (talleyInfo))->tbList), TALLEYBUFPERPAGE, sizeof(TalleyRoomInfoS))));
#else
		return (Buffer + Room);
#endif
		}

public:
	LogEntry *User;

#ifdef WINCIT
	TalleyBufferC(r_slot NewSize, LogEntry *NewUser);
#else
    TalleyBufferC();
#endif

	~TalleyBufferC();

	Bool IsGood(void)
		{
		#ifdef AUXMEM
            return ((((talleyTaskInfo *) (talleyInfo))->tbList) != NULL);
		#else
			return (Buffer != NULL);
		#endif
		}

	void Fill(void);
	void ResetBypassedAndEntered(void);

	void SetBypass(r_slot Room, Bool ToSet)				{	GetEntry(Room)->Bypassed = ToSet;	}
	void SetVisited(r_slot Room, Bool ToSet)			{	GetEntry(Room)->Visited = ToSet;	}
		
	void SetHasNewMail(r_slot Room, Bool ToSet)			{	GetEntry(Room)->HasMail = ToSet;	}
	void SetHasOldMail(r_slot Room, Bool ToSet)			{	GetEntry(Room)->HasOldMail = ToSet;	}
		
	Bool Bypassed(r_slot Room)							{	return (GetEntry(Room)->Bypassed);	}
	Bool Visited(r_slot Room)							{	return (GetEntry(Room)->Visited);	}                                                                                       

	Bool HasNewMail(r_slot Room)						{	return (GetEntry(Room)->HasMail);	}
	Bool HasOldMail(r_slot Room)						{	return (GetEntry(Room)->HasOldMail);}

	void SetEnteredInRoom(r_slot Room, m_slot Entered)	{	GetEntry(Room)->MsgRoom = Entered;	}
	void IncrementEnteredInRoom(r_slot Room)			{	GetEntry(Room)->MsgRoom++;			}
	int EnteredInRoom(r_slot Room)  					{	return (GetEntry(Room)->MsgRoom);	}

	void AddRoomNewMessages(r_slot Room, m_slot ToAdd)	{	GetEntry(Room)->New += ToAdd;		}
	void SetRoomNewMessages(r_slot Room, m_slot ToSet)	{	GetEntry(Room)->New = ToSet;		}
	m_slot NewInRoom(r_slot Room)						{	return (GetEntry(Room)->New);		}

	void SetRoomMessages(r_slot Room, m_slot ToSet)		{	GetEntry(Room)->Messages = ToSet;	}
	void AddRoomMessages(r_slot Room, m_slot ToAdd)		{	GetEntry(Room)->Messages += ToAdd;	}
	m_slot MessagesInRoom(r_slot Room)					{	return (GetEntry(Room)->Messages);	}

	void SetRoomTotal(r_slot Room, m_slot NewTotal)		{	GetEntry(Room)->Total = NewTotal;	}
	void AddRoomTotal(r_slot Room, m_slot ToAdd)		{	GetEntry(Room)->Total += ToAdd;		}
	m_slot TotalMessagesInRoom(r_slot Room)				{	return (GetEntry(Room)->Total);		}
	};

struct TalleyBufferListS
	{
	TalleyBufferListS *next;
	TalleyBufferC *B;
	};

class TalleyBufferCollectionC
	{
#ifdef WINCIT
	HANDLE OurMutex;
#endif

	TalleyBufferListS *List;

	void Lock(void)
		{
#ifdef WINCIT
		WaitForSingleObject(OurMutex, INFINITE);
#endif
		}

	void Unlock(void)
		{
#ifdef WINCIT
		ReleaseMutex(OurMutex);
#endif
		}

public:
	TalleyBufferCollectionC(void)
		{
#ifdef WINCIT
		OurMutex = CreateMutex(NULL, FALSE, NULL);
#endif
		List = NULL;
		}

	~TalleyBufferCollectionC()
		{
		Lock();

		disposeLL((void **) &List);

#ifdef WINCIT
		if (OurMutex != INVALID_HANDLE_VALUE)
			{
			CloseHandle(OurMutex);
			}
#endif
		}

	void Add(TalleyBufferC *New);
	void Remove(TalleyBufferC *Old);
	void NewMessageNoNotify(Message *Msg);
	void NewMessage(Message *Msg);
	void RemoveMessage(m_slot ID);
	void MoveMessage(m_slot ID, r_slot OldRoom, r_slot NewRoom);
	};


extern TalleyBufferCollectionC TalleyBufferCollection;
