// --------------------------------------------------------------------------
// Citadel: OpenFile.CPP
//
// Opening of the data files, checking for conversion and stuff

#include "ctdl.h"
#pragma hdrstop

//#include "config.h"
//#include "resize.h"
//#include "room.h"
//#include "group.h"
//#include "extmsg.h"
//#include "msg.h"


// --------------------------------------------------------------------------
// Contents
//
// openFile()			Special to open a .DAT file


// --------------------------------------------------------------------------
// openFile(): Special to open a .DAT file.

void openFile(const char *filename, FILE **fd)
	{
	if (!citOpen(filename, CO_RPB, fd))
		{
		crashout(getmsg(597), filename, errno);
		}
	}
