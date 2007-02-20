#include "ctdl.h"

Bool citOpen(const char *filename, CitOpenCodes mode, FILE **cf)
	{
	const char *cMode;

	switch (mode)
		{
		case CO_A:		cMode = FO_A;	break;
		case CO_AB: 	cMode = FO_AB;	break;
		case CO_AP: 	cMode = FO_AP;	break;
		case CO_R:		cMode = FO_R;	break;
		case CO_W:		cMode = FO_W;	break;
		case CO_WB: 	cMode = FO_WB;	break;
		case CO_RB: 	cMode = FO_RB;	break;
		case CO_RPB:	cMode = FO_RPB; break;
		default:
		case CO_WPB:	cMode = FO_WPB; break;
		}

	*cf = fopen(filename, cMode);

	return (*cf != NULL);
	}
