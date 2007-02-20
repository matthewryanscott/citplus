// --------------------------------------------------------------------------
// Citadel: ScrlBack.CPP
//
// The interface to the scroll-back buffer

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "scrlback.h"
#include "auxtab.h"

char *scrollBuf;						// where the buffer is
long lastScrollBufLine; 				// the first line in buffer
long firstScrollBufLine;				// the last line in buffer
#ifdef AUXMEM
int LinesPerPage;						// 0 if all in heap
int sbBlocksInHeap;
#endif

void SaveToScrollBackBuffer(int Lines)
	{
	if (cfg.scrollSize)
		{
		for (int i = 0; i < Lines; i++)
			{
			if (++lastScrollBufLine >= cfg.scrollSize)
				{
				lastScrollBufLine = 0;
				}

			if (lastScrollBufLine == firstScrollBufLine)
				{
				if (++firstScrollBufLine >= cfg.scrollSize)
					{
					firstScrollBufLine = 0;
					}
				}

			if (cfg.scrollColors)
				{
#ifdef AUXMEM
				if (LinesPerPage)
					{
					fastcpy(LoadAuxmemBlock(lastScrollBufLine,
							&(auxTabList *) scrollBuf, LinesPerPage,
							conCols * 2), logiScreen + i * conCols * 2,
							conCols * 2);
					}
				else
					{
					fastcpy(scrollBuf + ((int) lastScrollBufLine) * conCols * 2,
							logiScreen + i * conCols * 2, conCols * 2);
					}
#else
				fastcpy(scrollBuf + ((int) lastScrollBufLine) * conCols * 2,
						logiScreen + i * conCols * 2, conCols * 2);
#endif
				}
			else
				{
#ifdef AUXMEM
				if (LinesPerPage)
					{
					for (int j = 0; j < conCols; j++)
						{
						((char *) LoadAuxmemBlock(lastScrollBufLine,
								&(auxTabList *) scrollBuf, LinesPerPage,
								conCols))[j] = *(logiScreen + i *
								conCols * 2 + j * 2);
						}
					}
				else
					{
					for (int j = 0; j < conCols; j++)
						{
						scrollBuf[((int) lastScrollBufLine) * conCols + j] =
								*(logiScreen + i * conCols * 2 + j * 2);
						}
					}
#else
				for (int j = 0; j < conCols; j++)
					{
					scrollBuf[((int) lastScrollBufLine) * conCols + j] =
							*(logiScreen + i * conCols * 2 + j * 2);
					}
#endif
				}
			}
		}
	}
#endif
