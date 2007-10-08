// --------------------------------------------------------------------------
// Citadel: MemInit.CPP
//
// This is the code to initialize memory


#include "ctdl.h"
#pragma hdrstop

#include "auxtab.h"
#include "msg.h"
#include "log.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// addMtBlock() 	adds another chunk to the mt


Bool MessageTableC::Resize(m_slot NewSize)
	{
	Size = NewSize;

#ifdef AUXMEM
	DeleteAuxmemList(&mtList, &msgBlocksInHeap);
#endif
#ifdef WINCIT
	delete [] Table;
	Table = NULL;
#endif
#ifdef REGULAR
	delete [] Flags;
	delete [] LocLO;
	delete [] LocHI;
	delete [] RoomNum;
	delete [] ToHash;
	delete [] AuthHash;
	delete [] OriginID;

	Flags = NULL;
	LocLO = NULL;
	LocHI = NULL;
	RoomNum = NULL;
	ToHash = NULL;
	AuthHash = NULL;
	OriginID = NULL;
#endif

#ifndef REGULAR
	delete [] FirstMessageInRoom;
	delete [] LastMessageInRoom;
	FirstMessageInRoom = NULL;
	LastMessageInRoom = NULL;
#endif

#ifdef AUXMEM
	assert(mtList == NULL);

	for (m_slot i = 0; i < NewSize; i += MSGTABPERPAGE)
		{
		if (!AddAuxmemBlock(&mtList, i, &msgBlocksInHeap, 2))
			{
			crashout(getcfgmsg(114), ltoac(i));
			}
		}
#endif
#ifdef WINCIT
	if ((Table = new MsgTabEntryS[NewSize + 1]) == NULL)
		{
		crashout(getcfgmsg(115));
		}
#endif
#ifdef REGULAR
	Flags = new msgflags[NewSize + 1];
	LocLO = new int[NewSize + 1];
	LocHI = new int[NewSize + 1];
	RoomNum = new r_slot[NewSize + 1];
	ToHash = new int[NewSize + 1];
	AuthHash = new int[NewSize + 1];
	OriginID = new uint[NewSize + 1];

	if (Flags == NULL || LocLO == NULL || LocHI == NULL || RoomNum == NULL ||
			ToHash == NULL || AuthHash == NULL || OriginID == NULL)
		{
		crashout(getcfgmsg(115));
		}
#endif

#ifndef REGULAR
	// should probably pass a parameter...
	FirstMessageInRoom = new m_index[cfg.maxrooms];
	LastMessageInRoom = new m_index[cfg.maxrooms];

	if (!FirstMessageInRoom || !LastMessageInRoom)
		{
		crashout(getcfgmsg(107));
		}
	else
		{
		memset(FirstMessageInRoom, 0xff, sizeof(m_index) * cfg.maxrooms);
		memset(LastMessageInRoom, 0xff, sizeof(m_index) * cfg.maxrooms);
		}
#endif

	return (TRUE);
	}


void destroyBorders(void)
	{
	delete [] borders;
	borders = NULL;
	}


void makeBorders(void)
	{
	destroyBorders();

	borders = new char[cfg.maxborders * (BORDERSIZE + 1)];

	if (borders)
		{
		memset(borders, 0, cfg.maxborders * (BORDERSIZE + 1));
		}
	else
		{
		OutOfMemory(88);
		}
	}


void LogTable::Clear(void)
	{
	VerifyHeap();

	if (IsValid())
		{
		for (l_slot TableIndex = 0; TableIndex < TableSize; TableIndex++)
			{
			Table[TableIndex].Clear(TableIndex);
			}
		}

	VerifyHeap();
	}


void RoomTable::Clear(void)
	{
	VerifyHeap();

	if (IsValid())
		{
		for (r_slot TableIndex = 0; TableIndex < TableSize; TableIndex++)
			{
			(*this)[TableIndex].Clear();
			}
		}

	VerifyHeap();
	}
