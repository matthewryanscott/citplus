// --------------------------------------------------------------------------
// Citadel: MoveHall.CPP
//
// Stupid BC4

#include "ctdl.h"
#pragma hdrstop

#include "hall.h"


// --------------------------------------------------------------------------
// Contents
//
//	moveHall()		Moves a hall up or down in the hall list


// --------------------------------------------------------------------------
// moveHall(): Moves a hall up or down in the hall list.

Bool TERMWINDOWMEMBER moveHall(int offset, h_slot hall, Bool disp)
	{
	assert (offset == 1 || offset == -1);

	if (disp)
		{
		mPrintfCR(getmsg(497), cfg.Lhall_nym);
		doCR();
		}

	if (((hall > 1) && (hall < (cfg.maxhalls - 1)) && offset == 1)
		 || ((hall > 2) && (hall < cfg.maxhalls) && offset == -1))
		{
		HallEntry tmp;

		do
			{
			tmp = HallData[hall];
			HallData[hall] = HallData[(h_slot) (hall + offset)];
			HallData[(h_slot) (hall + offset)] = tmp;

			if (hall == thisHall)
				{
				thisHall += (h_slot) offset;
				}
			hall += (h_slot) offset;
			} while ((((hall > 1) && (hall < (cfg.maxhalls - 1)) &&
						offset == 1)
						||
						((hall > 2) && (hall < cfg.maxhalls) && offset == -1))
					&& !HallData[(h_slot) (hall - offset)].IsInuse());

		if (!HallData[(h_slot) (hall - offset)].IsInuse())
			{
			if (disp)
				{
				mPrintf(getmsg(498), cfg.Lhall_nym);
				}

			return (FALSE);
			}
		else
			{
			if (disp)
				{
				label Buffer;
				mPrintfCR(getmsg(384), cfg.Uhall_nym,
						HallData[(h_slot) (hall - 1)].GetName(Buffer,
						sizeof(Buffer)));
				}
			}

		HallData.Save();
		return (TRUE);
		}
	else
		{
		if (disp)
			{
			mPrintf(getmsg(498), cfg.Lhall_nym);
			}

		return (FALSE);
		}
	}
