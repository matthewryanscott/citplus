/* -------------------------------------------------------------------- */
/*	AUXTAB.H						Citadel 							*/
/*	How to use the memory manager that handles the tables in auxiliary	*/
/*	memory.																*/
/* -------------------------------------------------------------------- */

#ifdef AUXMEM

#ifndef AUXTAB_H
#define AUXTAB_H

/* -------------------------------------------------------------------- */
/*	In the Auxmem version, the message and room tables are stored in	*/
/*	4K pages.															*/
/*	The following data types help in dealing with Auxmem. For a full	*/
/*	discussion of what goes on, check out AUXTAB.C.						*/
/* -------------------------------------------------------------------- */

#define		AUXPAGESIZE	4096

/* -------------------------------------------------------------------- */
/*	Used to keep track of where this page happens to be located.		*/
/* -------------------------------------------------------------------- */
#ifdef __BORLANDC__
	#include <dos.h>

	typedef uint smPtr;

	#define	BC_OFFSET	4

	inline void *MkPtr(smPtr in)
		{
		return (in ? MK_FP(in, BC_OFFSET) : NULL);
		}

	inline smPtr MkSmPtr(const void *in)
		{
		return (FP_SEG(in));
		}
#else
	typedef void *smPtr;

	inline void *MkPtr(smPtr in)
		{
		return (in);
		}

	inline smPtr MkSmPtr(const void *in)
		{
		return (in);
		}
#endif

enum wmType
	{
	atHEAP, atXMS, atEMS, atVIRT
	};

struct auxTabList
	{
	auxTabList *next;				// next in list
	wmType whatMem;					// what sort of memory this is in
	smPtr Where;					// where in memory this chunk is
	ulong first;					// slot of first entry in chunk
	};

extern "C"
	{
	/* all in auxmem.asm */
	extern cdecl char usexms;		// if we are using XMS
	extern cdecl uint xmshand;		// XMS handle
	extern cdecl uint xmssize;		// XMS size
	extern cdecl uint emsframe;		// EMS page frame
	extern cdecl uint emshand;		// EMS handle
	extern cdecl uint emssize;		// EMS size
	extern cdecl uint vrthand;		// Virtual file handle
	extern cdecl uint vrtpags;		// Virtual file size
	extern cdecl uint xmserr;		// Error from last function
	extern cdecl uint emserr;		// Error from last function


	/***** auxmem.asm *****/
	uint cdecl _addToEMS(void);
	uint cdecl _addToVirt(void);
	uint cdecl _addToXMS(void);
	void cdecl disposeAuxMem(char *vmfile);
	void cdecl setUpAuxMem(char *vmfile);
	int cdecl EMStoHeap(void *whereto, int whatblock);
	int cdecl VirttoHeap(void *whereto, int whatblock);
	int cdecl XMStoHeap(void *whereto, int whatblock);
	int cdecl HeapToXMS(int whatblock, void *whereto);
	int cdecl HeapToEMS(int whatblock, void *whereto);
	int cdecl HeapToVirt(int whatblock, void *whereto);
	};

/***** auxtab.cpp *****/
void DeleteAuxmemList(auxTabList **List, int *BlockCounter);
Bool AddAuxmemBlock(auxTabList **List, long NewSlot, int *InHeap, int MaxHeap);
void *LoadAuxmemBlock(ulong BlockSlot, auxTabList **BlockList, int SlotsInBlock, size_t SizeOfEntry);

#endif
#endif
