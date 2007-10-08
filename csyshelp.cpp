// --------------------------------------------------------------------------
// Citadel: CSysHelp.CPP
//
// Console Sysop Help

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "consysop.h"

Bool HelpKeyHandler(CITWINDOW *, int key)
	{
	switch (key)
		{
		case CSHPM_EXIT:
		case MK_ESC:
			{
			// exit user menu stuff
			return (FALSE);
			}

		case CSHPM_EDIT:
			{
			break;
			}

		case CSHPM_INFO:
			{
			break;
			}
		}

	return (TRUE);
	}
#endif
