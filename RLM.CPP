// --------------------------------------------------------------------------
// Citadel: RLM.CPP
//
// Everything to be included to load an RLM.


#include "ctdl.h"
#pragma hdrstop

#ifndef WINCIT
#include "cwindows.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// callRLM()			Sets up for call to RLM, calls, recovers.
// initRLM()			Initializes RLM for load into memory.
// dumpRLM()			Dumps RLM from memory.
// purgeAllRLMs()		Purges unused RLMs from memory.
// loadRLM()			Loads RLM into memory.


//extern Bool interrupts_enabled;			// from port.c

static Bool destroyRlmCitWindow(CITWINDOW *wnd, Bool ask);
static Bool loadRLM(RLMlist *theRLM);

static CITWINDOW *makeRlmCitWindow(Bool (*func)(EVENT, long, int, CITWINDOW *),
		CITWINDOW *parent, char *title, WINDOWFLAGS flags, SRECT ext,
		void *ld, Bool isCtrl);

static Bool rlmSendMessage(EVENT evt, long param, int more, CITWINDOW *wnd);
static void initSerial(int port, int baud, int stops, int par, int len, Bool cts);

int cdecl citFunc;

void * cdecl RLMptrs[] =
	{
	mPrintf,			farcalloc,			farfree,		makeRlmCitWindow,
	destroyRlmCitWindow,memset, 			getFocus,		setFocus,
	sprintf,			fopen,				fclose, 		fgets,
	fseek,				strcpy, 			strlen, 		buildClipArray,
	CitWindowOutStr,	CitWindowOutChr,	CitWindowClearLine,freeClipArray,
	toupper,			rlmSendMessage, 	strcat, 		NULL/*outstring*/,
	disposeLL,			addLL,				getNextLL,		getLLNum,
	deleteLLNode,		insertLLNode,		getLLCount, 	bringToTop,
	initSerial, 		cls,				cPrintf,		CitWindowsGetYN,
	Hangup,
	};


// --------------------------------------------------------------------------
// callRLM(): Sets up for call to RLM, calls, recovers.

int callRLM(RLMlist *theRLM, uint funcOff, long param1, long param2)
	{
	int retVal;

	if (loadRLM(theRLM))
		{
		uint savedDS = _DS;
		int (*func)(long, long) = (int (*)(long, long))
				MK_FP(FP_SEG(theRLM->loadArea), funcOff);

		_DS = FP_SEG(func);

		theRLM->loadArea->lockCount++;
		retVal = (func)(param1, param2);
		theRLM->loadArea->lockCount--;

		_DS = savedDS;
		}
	else
		{
		retVal = 0;
		}

	return (retVal);
	}


// --------------------------------------------------------------------------
// initRLM(): Initializes RLM for load into memory.

Bool initRLM(const char *rlmFileName)
	{
	Bool toRet = FALSE;
	int h;
	fpath path;

	sprintf(path, getcfgmsg(284), cfg.rlmpath, rlmFileName);

	if ((h = open(path, O_RDONLY | O_BINARY)) > 0)
		{
		RLMlist *mCur;
		long fl = filelength(h);

		if (fl < 65000l &&
				(mCur = (RLMlist *) addLL((void **) &RLMmod,
				sizeof(*mCur))) != NULL)
			{
			uint bufsize = (uint) fl + 256; // PSP emulation

			if ((mCur->where = getMemG(bufsize, 1)) != NULL)
				{
				mCur->loadArea = (RLMstart *) MK_FP(FP_SEG(mCur->where), 256);

				read(h, mCur->loadArea, (uint) fl);

				if (mCur->loadArea->sig == 0x6969)
					{
					Bool dup;
					RLMlist *cCur;
					char *name = (char *) MK_FP(FP_SEG(mCur->loadArea),
							mCur->loadArea->nameOff);

					for (dup = FALSE, cCur = RLMmod; !dup && cCur;
							cCur = (RLMlist *) getNextLL(cCur))
						{
						if (strcmp(cCur->name, name) == SAMESTRING)
							{
							dup = TRUE;
							}
						}

					if (!dup && (!mCur->loadArea->permReq || (mCur->permData =
							getMemG(mCur->loadArea->permReq, 1)) != NULL))
						{
						linkDataType *ldt;

						strcpy(mCur->name, name);

						strcpy(mCur->fname, path);

						mCur->flags = mCur->loadArea->flags;

						for (ldt = (linkDataType *) MK_FP(FP_SEG(mCur->loadArea),
								mCur->loadArea->linkOff); ldt->func; ldt++)
							{
							RLMfuncs *fCur;

							if ((fCur = (RLMfuncs *) addLL((void **) &RLMdo,
									sizeof(*fCur))) != NULL)
								{
								fCur->type = ldt->type;
								fCur->moreData = ldt->moreData;
								fCur->theRLM = mCur;
								fCur->offset = (uint) ldt->func;
								}
							}

						freeMemG(mCur->where);
						mCur->where = NULL;
						mCur->loadArea = NULL;
						toRet = TRUE;
						}
					else
						{
						freeMemG(mCur->where);
						deleteLLNode((void **) &RLMmod, getLLCount(RLMmod));
						}
					}
				else
					{
					freeMemG(mCur->where);
					deleteLLNode((void **) &RLMmod, getLLCount(RLMmod));
					}
				}
			else
				{
				deleteLLNode((void **) &RLMmod, getLLCount(RLMmod));
				}
			}

		close(h);
		}

	return (toRet);
	}


// --------------------------------------------------------------------------
// dumpRLM(): Dumps RLM from memory.

static void dumpRLM(RLMlist *theRLM)
	{
	if (theRLM->loadArea && !theRLM->loadArea->lockCount)
		{
		if (theRLM->loadArea->tmpData)
			{
			freeMemG(theRLM->loadArea->tmpData);
			theRLM->loadArea->tmpData = NULL;
			}

		freeMemG(theRLM->where);
		theRLM->where = NULL;
		theRLM->loadArea = NULL;
		}
	}


// --------------------------------------------------------------------------
// purgeAllRLMs(): Purges unused RLMs from memory.

void purgeAllRLMs(void)
	{
	RLMlist *cur = RLMmod;

	while (cur)
		{
		if (cur->loadArea && !cur->loadArea->lockCount)
			{
			dumpRLM(cur);

			cur = RLMmod;
			}
		else
			{
			cur = (RLMlist *) getNextLL(cur);
			}
		}
	}


// --------------------------------------------------------------------------
// loadRLM(): Loads RLM into memory.

Bool loadRLM(RLMlist *theRLM)
	{
	int h;
	Bool toRet = FALSE;

	if (theRLM->loadArea)
		{
		return (TRUE);
		}

	if ((h = open(theRLM->fname, O_RDONLY | O_BINARY)) > 0)
		{
		long fl = filelength(h);
		uint bufsize = (uint) fl + 256; // PSP emulation

		compactMemory();

		if ((theRLM->where = getMemG(bufsize, 1)) != NULL)
			{
			theRLM->loadArea = (RLMstart *) MK_FP(FP_SEG(theRLM->where), 256);

			read(h, theRLM->loadArea, (uint) fl);

			if (!theRLM->loadArea->tempReq || (theRLM->loadArea->tmpData =
					getMemG(theRLM->loadArea->tempReq, 1)) != NULL)
				{
				void **nd;

				theRLM->loadArea->citFunc = &citFunc;
				theRLM->loadArea->cep = citEntryPoint;
				theRLM->loadArea->permData = theRLM->permData;

				for (nd = (void **) MK_FP(FP_SEG(theRLM->loadArea),
						theRLM->loadArea->needOff); *nd; nd++)
					{
					switch ((long) *nd)
						{
						case ND_DOWHAT:
							{
							*nd = &DoWhat;
							break;
							}

						case ND_DIALOUTFKEY:
							{
							*nd = &dialout_fkey;
							break;
							}

						case ND_SCROLLPOS:
							{
							*nd = &scrollpos;
							break;
							}

						case ND_CONCOLS:
							{
							*nd = &conCols;
							break;
							}

						case ND_APPLICPATH:
							{
							*nd = cfg.aplpath;
							break;
							}

						case ND_DIALPREF:
							{
							*nd = cfg.dialpref;
							break;
							}

						case ND_MSTHANDLER:
							{
							*nd = mstHandler;
							break;
							}

						case ND_MDATA:
							{
							*nd = &cfg.mdata;
							break;
							}

						case ND_BTHANDLER:
							{
							*nd = btHandler;
							break;
							}

						case ND_SPEED:
							{
//							*nd = &PortSpeed;
							break;
							}

						case ND_CHECKCTS:
							{
							*nd = &cfg.checkCTS;
							break;
							}
						}
					}

				toRet = TRUE;
				}
			else
				{
				freeMemG(theRLM->where);
				}
			}

		close(h);
		}

	return (toRet);
	}

// citwindows stuff.....................

typedef struct rlmCWL
	{
	struct rlmCWL *next;
	CITWINDOW *wnd;
	Bool (*h)(EVENT, long, int, CITWINDOW *);
	} rlmCitWindowList;

rlmCitWindowList *rcwl;

static Bool rlmHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	Bool b = FALSE;
	rlmCitWindowList *cur;

	for (cur = rcwl; cur; cur = (rlmCitWindowList *) getNextLL(cur))
		{
		if (cur->wnd == wnd)
			{
			RLMstart *rs = (RLMstart *) MK_FP(FP_SEG(cur->h), 0x100);

			uint savedDS = _DS;

			_DS = FP_SEG(cur->h);

			rs->lockCount++;
			b = (cur->h)(evt, param, more, wnd);
			rs->lockCount--;

			_DS = savedDS;
			break;
			}
		}

	if (!b)
		{
		return (defaultHandler(evt, param, more, wnd));
		}
	else
		{
		return (TRUE);
		}
	}

#ifdef GOODBYE
// handler for controls made in an RLM... (not used yet)
static Bool rlmCtrlHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	Bool b = FALSE;
	rlmCitWindowList *cur;

	for (cur = rcwl; cur; cur = (rlmCitWindowList *) getNextLL(cur))
		{
		if (cur->wnd == wnd)
			{
			RLMstart *rs = (RLMstart *) MK_FP(FP_SEG(cur->h), 0x100);

			uint savedDS = _DS;

			_DS = FP_SEG(cur->h);

			rs->lockCount++;
			b = (cur->h)(evt, param, more, wnd);
			rs->lockCount--;

			_DS = savedDS;
			break;
			}
		}

	return (b);
	}
#endif

CITWINDOW *makeRlmCitWindow(Bool (*func)(EVENT, long, int, CITWINDOW *),
		CITWINDOW *parent, char *title, WINDOWFLAGS flags, SRECT ext,
		void *ld, Bool isCtrl)
	{
	CITWINDOW *w = NULL;
	rlmCitWindowList *cur;

	if (isCtrl)
		{
		w = makeCitWindow(func, parent, title, flags, ext, ld, isCtrl);
		}
	else
		{
		if ((cur = (rlmCitWindowList *) addLL((void **) &rcwl,
				sizeof(*cur))) != NULL)
			{
			w = makeCitWindow(rlmHandler, parent, title, flags, ext, ld,
					isCtrl);

			if (w)
				{
				RLMstart *rs = (RLMstart *) MK_FP(FP_SEG(func), 0x100);
				rs->lockCount++;

				cur->wnd = w;
				cur->h = func;

				// redo what makeCitWindow did - rlmHandler now ready
				// (icky, but it works)
				(w->func)(EVT_NEWWINDOW, 0, 0, w);
				(w->func)(EVT_DRAWALL, 0, 0, w);
				}
			else
				{
				deleteLLNode((void **) &rcwl, getLLCount(rcwl));
				}
			}
		}

	return (w);
	}

Bool destroyRlmCitWindow(CITWINDOW *wnd, Bool ask)
	{
	if (destroyCitWindow(wnd, ask))
		{
		rlmCitWindowList *cur;
		int i;

		for (i = 1, cur = rcwl; cur;
				cur = (rlmCitWindowList *) getNextLL(cur), i++)
			{
			if (cur->wnd == wnd)
				{
				RLMstart *rs = (RLMstart *) MK_FP(FP_SEG(cur->h), 0x100);
				rs->lockCount--;
				deleteLLNode((void **) &rcwl, i);
				break;
				}
			}

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

static Bool rlmSendMessage(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	return ((wnd->func)(evt, param, more, wnd));
	}

//void initSerial(int port, int baud, int stops, int par, int len, Bool cts)
void initSerial(int, int, int, int, int, Bool)
	{
//	if (par == 2)
//		{
//		par++;
//		}

//	(*initrs)(port, baud, stops, par, len, cts);
//	interrupts_enabled = TRUE;
	}
#else
Bool initRLM(const char *)
	{
	return (FALSE);
	}
#endif
