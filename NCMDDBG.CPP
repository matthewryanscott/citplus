// --------------------------------------------------------------------------
// Citadel: NCmdDbg.CPP
//
// Network command processor for the #DEBUG command


#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "net.h"
#include "hall.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// DoDebugCmd()			Respond to #DEBUG


void TERMWINDOWMEMBER DoDebugCmd(const char **ncmsg)
	{
	char str[128];

	NetCmdReplyMsg->SetTextWithFormat(ncmsg[5], programName, version);
	NetCmdReplyMsg->AppendText(bn);

#ifndef VISUALC
	sprintf(str, ncmsg[6], _osmajor, _osminor);
	AppendLineToNetCommandReply(str);
#endif

	if (LockMenu(MENU_READ))
		{
#ifndef WINCIT
		if (*vdDesc)
			{
			sprintf(str, getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 85),
					vdDesc);
			AppendLineToNetCommandReply(str);
			}

		if (*cdDesc)
			{
			sprintf(str, getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 86),
					cdDesc);
			AppendLineToNetCommandReply(str);
			}

		if (*kdDesc)
			{
			sprintf(str, getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 87),
					kdDesc);
			AppendLineToNetCommandReply(str);
			}

		if (*sdDesc)
			{
			sprintf(str, getmenumsg(MENU_READ, 84), getmenumsg(MENU_READ, 88),
					sdDesc);
			AppendLineToNetCommandReply(str);
			}
#endif

#ifdef MSC
		r.h.ah = 0x48;	 // allocate memory
		r.h.al = 0;
		r.x.bx = 0xffff; // ask for an impossible amount

		intdos(&r, &r);

		sprintf(str, getmenumsg(MENU_READ, 90), _bios_memsize());
		NetCmdReplyMsg->AppendText(str);
		sprintf(str, getmenumsg(MENU_READ, 91), ltoac((long)r.x.bx*16L));
		AppendLineToNetCommandReply(str);
#else
	#ifdef WINCIT
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);

		label Avail;

		CopyStringToBuffer(Avail, ltoac(ms.dwAvailPhys / 1024));
		sprintf(str, getmenumsg(MENU_READ, 92), ltoac(ms.dwTotalPhys / 1024),
				Avail);
		AppendLineToNetCommandReply(str);

		CopyStringToBuffer(Avail, ltoac(ms.dwAvailPageFile / 1024));
		sprintf(str, getmenumsg(MENU_READ, 93),
				ltoac(ms.dwTotalPageFile / 1024), Avail);
		AppendLineToNetCommandReply(str);

		CopyStringToBuffer(Avail, ltoac(ms.dwAvailVirtual / 1024));
		sprintf(str, getmenumsg(MENU_READ, 107),
				ltoac(ms.dwTotalVirtual / 1024), Avail);
		AppendLineToNetCommandReply(str);

		sprintf(str, getmenumsg(MENU_READ, 108), ms.dwMemoryLoad);
		AppendLineToNetCommandReply(str);
	#else
		sprintf(str, getmenumsg(MENU_READ, 90), biosmemory());
		NetCmdReplyMsg->AppendText(str);
		sprintf(str, getmenumsg(MENU_READ, 91), ltoac(farcoreleft()));
		AppendLineToNetCommandReply(str);
	#endif
#endif

		if (*cmdLine)
			{
			sprintf(str, getmenumsg(MENU_READ, 94), cmdLine);
			AppendLineToNetCommandReply(str);
			}

		UnlockMenu(MENU_READ);
		}
	else
		{
		sprintf(str, getmsg(188), ncmsg[76]);
		AppendLineToNetCommandReply(str);
		}

	sprintf(str, ncmsg[7], cfg.sysop);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[8], cfg.mdata);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[9], cfg.dumbmodem);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[10], bauds[cfg.initbaud]);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[11], connectbauds[cfg.minbaud]);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[12], cfg.modsetup);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[13], cfg.modunsetup);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[14], cfg.downshift);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[15], cfg.dialsetup);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[16], cfg.dialpref);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[17], cfg.dialring);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[18], cfg.dialringwait);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[19], cfg.downshift);
	NetCmdReplyMsg->AppendText(str);
	for (int i = 0; i < 7; i++)
		{
		if (cfg.updays[i])
			{
			sprintf(str, ncmsg[38], i + 1);
			NetCmdReplyMsg->AppendText(str);
			}
		}
	NetCmdReplyMsg->AppendText(bn);

	sprintf(str, ncmsg[20], cfg.downshift);
	NetCmdReplyMsg->AppendText(str);
EXTRA_OPEN_BRACE	
	for (int i = 0; i < 24; i++)
		{
		if (cfg.uphours[i])
			{
			sprintf(str, ncmsg[38], i + 1);
			NetCmdReplyMsg->AppendText(str);
			}
		}
EXTRA_CLOSE_BRACE
	NetCmdReplyMsg->AppendText(bn);

	sprintf(str, ncmsg[21], cfg.hangupdelay);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[22], cfg.hangup);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[23], cfg.offhookstr);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[24], cfg.baudPause);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[26], cfg.autoansi);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[27], cfg.MsgDatSizeInK);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[28], cfg.maxfiles);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[29], cfg.MAXLOGTAB);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[30], cfg.maxrooms);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[31], cfg.maxhalls);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[32], cfg.maxgroups);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[33], cfg.maxborders);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[34], (ulong) cfg.nmessages);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[35], cfg.ovrEms);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[36], cfg.ovrExt);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[37], cfg.bios);
	AppendLineToNetCommandReply(str);

#ifdef AUXMEM
	sprintf(str, ncmsg[40], cfg.vmemfile);
	AppendLineToNetCommandReply(str);
#endif

	sprintf(str, ncmsg[41], cfg.checkCTS);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[42], cfg.scrollSize);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[43], cfg.scrollColors);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[44], cfg.maxjumpback);
	AppendLineToNetCommandReply(str);

	sprintf(str, ncmsg[45], cfg.statnum);
	AppendLineToNetCommandReply(str);
	}
