#include "ctdl.h"
#pragma hdrstop

#include "auxtab.h"
#include "pathdat.h"
#include "msg.h"
#include "extmsg.h"

void CreatePathDat(void)
	{
#ifndef WINCIT
#ifndef AUXMEM
#ifndef REGULAR
	Message *Msg = new Message;

	if (Msg)
		{
		FILE *fl;

		mPrintfCR(getsysmsg(266));

		char fullPathToPathDat[128];
		sprintf(fullPathToPathDat, sbs, cfg.homepath, getsysmsg(8));

		if ((fl = fopen(fullPathToPathDat, FO_W)) != NULL)
			{
			for (m_index mcnt = cfg.OldestMsgTab; mcnt <= cfg.NewestMsg; mcnt++)
				{
				long loc = MsgTab.GetLocation(mcnt);
				int i, j;
				label lastlocID;

				Msg->ReadHeader(RMC_NORMAL, NULL, loc, NULL);

				if (*Msg->GetFromPath())
					{
					fprintf(fl, pcts, Msg->GetFromPath());

					// Get last locID, this stuff probably not necessary
					for (i = strlen(Msg->GetFromPath()); i && Msg->GetFromPath()[i] != '.'; i--);

					for (j = 0, i++; Msg->GetFromPath()[i] && Msg->GetFromPath()[i] != '!' && j < 4; i++, j++)
						{
						lastlocID[j] = Msg->GetFromPath()[i];
						}
					lastlocID[j] = 0;

					/* if last locID was same as current locID just use alias */
					if (SameString(lastlocID, cfg.locID))
						{
						fprintf(fl, getmsg(2), cfg.alias);
						}
					else
						{
						fprintf(fl, getmsg(2), cfg.Address);
						}

					fprintf(fl, bn);
					}
				}

			fclose(fl);
			}
		else
			{
			mPrintf(getmsg(78), fullPathToPathDat);
			}

		delete Msg;
		}
	else
		{
		OutOfMemory(90);
		}
#endif
#endif
#endif
	}
