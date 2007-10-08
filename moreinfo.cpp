/* -------------------------------------------------------------------- */
/*	MOREINFO.CPP				Citadel 								*/
/* -------------------------------------------------------------------- */
/*							MoreInfo stuff. 							*/
/* -------------------------------------------------------------------- */
#include "ctdl.h"
#pragma hdrstop

#include "moreinfo.h"

Bool MoreInfo::Open(const char *FileName)
	{
	assert(this);

	if (!File)
		{
		File = fopen(FileName, FO_RPB);
		}

	return (File != NULL);
	}

void MoreInfo::Close(void)
	{
	assert(this);

	if (File)
		{
		fclose(File);
		File = NULL;
		}
	}
