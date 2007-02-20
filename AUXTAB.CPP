// --------------------------------------------------------------------------
// Citadel: AuxTab.CPP
//
// This is the code to deal with the tables in auxiliary memory.

#ifdef AUXMEM
#include "ctdl.h"
#pragma hdrstop

#include "auxtab.h"
#include "tallybuf.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// this swapping code exists so really huge tables can exist in DOS's 640K.
// it will swap unused portions of the tables (in 4K chunks) to XMS, EMS, or
// the hard disk (in that order).
//
// the concept is quite simple. the execution is just a little more
// difficult. :)
//
// we keep a linked list for each table that tells us where each 4K chunk of
// the table is located. When we need to find an entry, we find where it is
// in this list. if it is in the heap, great; we can get right to it and all
// is fine. if, however, it is in xms, ems, or on disk, we have to swap a
// chunk off the heap into auxiliary memory, then copy the desired chunk into
// the heap so we can use it. this is the fun part.
//
// the lists are kept so that whenever a chunk in heap is used, it is moved
// to the start of the list. thus, the oldest chunk is kept at the end. when
// it is time to swap a chunk out of heap, we look for the last chunk in heap
// in the list to toss out.
//
// the rest of citadel can only access the tables through these routines.
// this will be a bit slower overall, but the message table only gets a good
// thrashing when incorporating netmessages, when there is nobody on line,
// anyway. The room table only gets such a thrashing when a new room is
// created. So, the overall performance hit can be pretty much ignored
// without worry.
//
// this code, of course, is not at all portable to any other operating
// system. i wrote it with borland c++ 3.0, but it should compile with msc.
//
// BiB/92Apr21 r.92Oct25/94Feb23

// --------------------------------------------------------------------------
// Contents
//
// bringToFront()			moves a mtList entry to the front
// lastMtList() 			finds last node of our linked list
// swapOutOldest()			puts the oldest chunk in Heap to aux
// getMsgTab()				gets slot in heap and gives addr
// getFlags()				returns flags for msg#
// getLocation()			returns location for msg#
// getRoomNum() 			returns room# for msg#
// getToHash()				returns hash of to of msg#
// getAuthHash()			returns hash of auth of msg#
// getOriginID()			returns origin id for msg#

auxTabList *freeBlock;					// the swap block
auxTabList *freeList;					// for fragmented auxmem
static int ListCounter;

static void bringToFront(auxTabList *theone, auxTabList **base);
static void swapOutOldest(auxTabList *base);


static void *addToHeap(int *cih, int maxPages)
	{
	if (++(*cih) <= maxPages)
		{
		return ((void *) new char[AUXPAGESIZE]);
		}
	else
		{
		(*cih)--;
		return (NULL);
		}
	}


static void addFreeList(auxTabList *toAdd)
	{
	auxTabList *tmpList;

	assert(toAdd != NULL);

	if (freeList)
		{
		for (tmpList = freeList; tmpList->next; tmpList = tmpList->next);
		tmpList->next = toAdd;
		}
	else
		{
		freeList = toAdd;
		}

	toAdd->next = NULL;
	}


static void clearFreeList(void)
	{
	disposeLL((void **) &freeList);
	}


static uint addToXMS(void)
	{
	auxTabList *thisNode, *lastNode;

	for (lastNode = NULL, thisNode = freeList; thisNode; lastNode = thisNode, thisNode = thisNode->next)
		{
		if (thisNode->whatMem == atXMS)
			{
			if (lastNode)
				{
				lastNode->next = thisNode->next;
				}
			else
				{
				freeList = thisNode->next;
				}

			const uint where = thisNode->Where;
			delete thisNode;
			return (where);
			}
		}

	return (_addToXMS());
	}

static uint addToEMS(void)
	{
	auxTabList *thisNode, *lastNode;

	for (lastNode = NULL, thisNode = freeList; thisNode; lastNode = thisNode, thisNode = thisNode->next)
		{
		if (thisNode->whatMem == atEMS)
			{
			if (lastNode)
				{
				lastNode->next = thisNode->next;
				}
			else
				{
				freeList = thisNode->next;
				}

			const uint where = thisNode->Where;
			delete thisNode;
			return (where);
			}
		}

	return (_addToEMS());
	}

static uint addToVirt(void)
	{
	auxTabList *thisNode, *lastNode;

	for (lastNode = NULL, thisNode = freeList; thisNode; lastNode = thisNode, thisNode = thisNode->next)
		{
		if (thisNode->whatMem == atVIRT)
			{
			if (lastNode)
				{
				lastNode->next = thisNode->next;
				}
			else
				{
				freeList = thisNode->next;
				}

			const uint where = thisNode->Where;
			delete thisNode;
			return (where);
			}
		}

	return (_addToVirt());
	}

void *LoadAuxmemBlock(ulong BlockSlot, auxTabList **BlockList, int SlotsInBlock, size_t SizeOfEntry)
	{
	auxTabList *lal, tmp;

	for (lal = *BlockList; lal; lal = lal->next)
		{
		if (lal->first <= BlockSlot && BlockSlot < lal->first + (ulong) SlotsInBlock)
			{
			// our slot is in this chunk
			if (lal->whatMem != atHEAP)
				{
				swapOutOldest(*BlockList);
				switch (lal->whatMem)
					{
					case atXMS:
						{
						if (!XMStoHeap(MkPtr(freeBlock->Where), (uint) lal->Where))
							{
							crashout(getmsg(MSG_AUXMEM, 0), xmserr);
							}

						break;
						}

					case atEMS:
						{
						if (!EMStoHeap(MkPtr(freeBlock->Where), (uint) lal->Where))
							{
							crashout(getmsg(MSG_AUXMEM, 1), emserr);
							}

						break;
						}

					case atVIRT:
						{
						if (!VirttoHeap(MkPtr(freeBlock->Where), (uint) lal->Where))
							{
							crashout(getmsg(MSG_AUXMEM, 2));
							}

						break;
						}
					}

				tmp.whatMem = freeBlock->whatMem;
				tmp.Where = freeBlock->Where;

				freeBlock->whatMem = lal->whatMem;
				freeBlock->Where = lal->Where;

				lal->whatMem = tmp.whatMem;
				lal->Where = tmp.Where;
				}

			bringToFront(lal, BlockList);
			return ((void *) ((char *) MkPtr((*BlockList)->Where) +
					((uint) (BlockSlot - (*BlockList)->first)) * SizeOfEntry));
			}
		}

#ifdef GOODBYE
	label t;
	extern char *scrollBuf;
	extern auxTabList *mtList;
	extern auxTabList *rtList;

	typedef struct
		{
		int talleyBlocksInHeap;
		auxTabList *tbList;
		} talleyTaskInfo;

	if (*BlockList == (auxTabList *) scrollBuf)
		{
		strcpy(t, getmsg(MSG_AUXMEM, 3));
		}
	else if (*BlockList == mtList)
		{
		strcpy(t, getmsg(MSG_AUXMEM, 4));
		}
	else if (*BlockList == rtList)
		{
		strcpy(t, getmsg(MSG_AUXMEM, 5));
		}
	else if (*BlockList == (((talleyTaskInfo *) (talleyInfo))->tbList))
		{
		strcpy(t, getmsg(MSG_AUXMEM, 6));
		}
	else
		{
		strcpy(t, getmsg(MSG_AUXMEM, 7));
		}

	crashout(getmsg(MSG_AUXMEM, 8), ltoac(BlockSlot), SizeOfEntry, t);
#else
	crashout(getmsg(MSG_AUXMEM, 8), ltoac(BlockSlot), SizeOfEntry, "Wow");
#endif

	// Just to make compiler happy.
	return (NULL);
	}

static void bringToFront(auxTabList *theone, auxTabList **base)
	{
	auxTabList *lal;

	if (theone == *base)
		{
		return;
		}

	lal = *base;
	while (lal != NULL && lal->next != theone)
		{
		lal = lal->next;
		}

	if (!lal)
		{
		crashout(getmsg(MSG_AUXMEM, 9));
		}

	lal->next = theone->next;
	theone->next = *base;
	*base = theone;
	}

static void swapOutOldest(auxTabList *base)
	{
	auxTabList *lal, tmp;

	if (!base)
		{
		crashout(getmsg(MSG_AUXMEM, 10));
		}

	if (freeBlock->whatMem == atHEAP)
		{
		crashout(getmsg(MSG_AUXMEM, 11));
		}

	lal = base;
	while (lal->next->whatMem == atHEAP)
		{
		lal = lal->next;
		}

	// lal is last heap block - get rid of it.
	switch (freeBlock->whatMem)
		{
		case atXMS: 	// XMS returns 0 for error
			{
			if (!HeapToXMS((uint) freeBlock->Where, MkPtr(lal->Where)))
				{
				crashout(getmsg(MSG_AUXMEM, 12), xmserr);
				}

			break;
			}

		case atEMS: 	// EMS returns 0 for no error
			{
			if (HeapToEMS((uint) freeBlock->Where, MkPtr(lal->Where)))
				{
				crashout(getmsg(MSG_AUXMEM, 13), emserr);
				}

			break;
			}

		case atVIRT:	// Heap returns 0 for error
			{
			if (!HeapToVirt((uint) freeBlock->Where, MkPtr(lal->Where)))
				{
				crashout(getmsg(MSG_AUXMEM, 14));
				}

			break;
			}
		}

	tmp.whatMem = freeBlock->whatMem;
	tmp.Where = freeBlock->Where;

	freeBlock->whatMem = lal->whatMem;	// free chunk now where lal was
	freeBlock->Where = lal->Where;

	lal->whatMem = tmp.whatMem; 		// lal now what was free
	lal->Where = tmp.Where;
	}

Bool AddAuxmemBlock(auxTabList **List, long NewSlot, int *InHeap, int MaxHeap)
	{
	auxTabList *Local;

	if (!ListCounter)
		{
		setUpAuxMem(cfg.vmemfile);
		}

	if (!*List && List != &freeBlock)
		{
		ListCounter++;
		}

	if ((Local = (auxTabList *) addLL((void **) List, sizeof(**List))) == NULL)
		{
		ListCounter--;

		if (!ListCounter)
			{
			disposeAuxMem(cfg.vmemfile);
			}

		return (FALSE);
		}

	Local->first = NewSlot;

	smPtr where;

    printf("locating spot for block...\n");

	if ((where = MkSmPtr(addToHeap(InHeap, MaxHeap))) != 0)
		{
		Local->whatMem = atHEAP;
		Local->Where = where;
        printf("1");
		}
	else if ((where = (smPtr) addToXMS()) != 0)
		{
		Local->whatMem = atXMS;
		Local->Where = where;
        printf("2");
		}
	else if ((where = (smPtr) addToEMS()) != 0)
		{
		Local->whatMem = atEMS;
		Local->Where = where;
        printf("3");
		}
	else if ((where = (smPtr) addToVirt()) != 0)
		{
		Local->whatMem = atVIRT;
		Local->Where = where;
        printf("4");
		}
	else
		{
		// if we could not put it anywhere, try again in heap, with no limit.
		if ((where = MkSmPtr(addToHeap(InHeap, INT_MAX))) != 0)
			{
			Local->whatMem = atHEAP;
			Local->Where = where;
            printf("problem?");
			}
		else
			{
            printf("problem2?");
			ListCounter--;
			return (FALSE);
			}
		}

	// add one more chunk for swapping if needed
	if (!freeBlock && (emssize || xmssize || vrtpags))
		{
        printf("wooooow");
		int tmp = 0;

		if (!AddAuxmemBlock(&freeBlock, 0, &tmp, 0))
			{
			crashout(getmsg(MSG_AUXMEM, 15));
			}
		}

    printf("should be all good");

    return(TRUE);
	}

void DeleteAuxmemList(auxTabList **List, int *BlockCounter)
	{
	auxTabList *lal, *lal2;
	lal = *List;

	if (lal)
		{
		ListCounter--;
		}

	while (lal)
		{
		lal2 = lal;
		lal = lal->next;

		if (lal2->whatMem == atHEAP)
			{
			delete MkPtr(lal2->Where);
			delete lal2;
			}
		else
			{
			addFreeList(lal2);
			}
		}

	*BlockCounter = 0;
	*List = NULL;

	if (!ListCounter)
		{
		if (freeBlock)
			{
			if (freeBlock->whatMem == atHEAP)
				{
				delete MkPtr(freeBlock->Where);
				}

			delete freeBlock;
			freeBlock = NULL;
			}

		clearFreeList();
		disposeAuxMem(cfg.vmemfile);
		}
	}
#endif
