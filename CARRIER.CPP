// --------------------------------------------------------------------------
// Citadel: Carrier.CPP
//
// Code to detect and report on Carrier.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "mdmreslt.h"
#include "cwindows.h"
#include "extmsg.h"
#include "winser.h"
#ifndef WINCIT
#include "statline.h"
#endif

time_t THREAD lastringtime; // Time of last RING from modem

// --------------------------------------------------------------------------
// Contents
//
// checkCR				Checks for CRs from the data port for half a second.
// findbaud 			Finds the baud from sysop and user supplied data.
// CarrierJustFound		sets global flags for carrier detect
// CarrierJustLost		sets global flags for carrier loss
// ringdetectbaud		sets baud rate according to ring detect
// smartbaud			sets baud rate according to result codes
// modInpStr			get a string from the modem, waiting for upto 3 secs
// checkring			Checks for 'RING' or '2' from data port.
// CheckForNewCarrier	checks carrier


// --------------------------------------------------------------------------
// checkCR(): Checks for CRs from the data port for half a second
//
// Return value:
//	TRUE: CR detected
//	FALSE: CR not detected

static Bool checkCR(CommunicationPortC *CP)
	{
	pause(50);

	if (CP->IsInputReady() && (CP->Input() == '\r'))
		{
		return (TRUE);
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// findbaud(): Finds the baud by checking for CRs from user.
//
// Return value:
//	TRUE: Baud found and set
//	FALSE: Couldn't detect baud

static Bool findbaud(CommunicationPortC *CP)
	{
	Bool good = FALSE;
	int time = 0;

	PortSpeedE baudRunner = PS_300;

	while (CP->IsInputReady())
		{
		CP->Input();
		}

	while (CP->HaveConnection() && !good && time++ < 120)
		{
		CP->SetSpeed(baudRunner);
		good = checkCR(CP);

		if (!good)
			{
			baudRunner = (PortSpeedE) ((baudRunner + 1) % PS_NUM);
			}
		}

	for (ModemSpeedE i = MS_300; i < MS_NUM; i = (ModemSpeedE) (i + 1))
		{
		if (connectbauds[i] == bauds[baudRunner])
			{
			CP->SetModemSpeed(i);
			break;
			}
		}

	return (good);
	}


// --------------------------------------------------------------------------
// CarrierJustFound(): sets global flags for carrier detect

void TERMWINDOWMEMBER CarrierJustFound(void)
	{

    if(!read_tr_messages())
        {
        cPrintf(getmsg(59));
        return;
        }

	modStat = TRUE;

	time(&conntimestamp);

	// screensaver junk only if fucking_stupid
	if (cfg.fucking_stupid)
		{
		ScreenSaver.Update();
		}

#ifdef WINCIT
    switch(CommPort->GetType())
        {
        case CT_SERIAL:
            {
            trap(T_CARRIER, ns, gettrmsg(1), OurConSeq, gettrmsg(6), gettrmsg(3), gettrmsg(9), WindowCaption);
            trap(T_CARRIER, WindowCaption, gettrmsg(12), connectbauds[CommPort->GetModemSpeed()], WindowCaption);
            break;
            }

        case CT_TELNET:
            {
//            trap(T_CARRIER, WindowCaption, gettrmsg(12), connectbauds[CommPort->GetModemSpeed()], WindowCaption);
            break;
            }

        default:
            {
            trap(T_CARRIER, ns, gettrmsg(1), OurConSeq, gettrmsg(13), gettrmsg(14), gettrmsg(9), WindowCaption);
            break;
            }
        }
#else
    trap(T_CARRIER, gettrmsg(12), connectbauds[CommPort->GetModemSpeed()], tst);
#endif

	CurrentUser->SetCredits(cfg.unlogbal);
	time(&LastActiveTime);
    dump_tr_messages();
	}


// --------------------------------------------------------------------------
// CarrierJustLost(): sets global flags for carrier loss

void TERMWINDOWMEMBER CarrierJustLost(void)
	{

    if (!read_tr_messages())
        {
        cPrintf(getmsg(59));
        return;
        }

	CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(91));

	OC.SetOutFlag(OUTSKIP);

	modStat = FALSE;
	pause(100);
#ifndef WINCIT
	CommPort->Disable();
    trap(T_CARRIER, getmsg(166));
#else
    switch(CommPort->GetType())
        {
        case CT_SERIAL:
            {
            trap(T_CARRIER, ns, gettrmsg(10), WindowCaption);
            break;
            }

        case CT_TELNET:
            {
            trap(T_CARRIER, WindowCaption, gettrmsg(11), gettrmsg(4), gettrmsg(2));
            break;
            }

        default:
            {
            trap(T_CARRIER, ns, gettrmsg(11), gettrmsg(13), gettrmsg(14));
            break;
            }
        }

#endif

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}

        dump_tr_messages();
	}


// --------------------------------------------------------------------------
// ringdetectbaud(): sets baud rate according to ring detect

#ifdef WINCIT
static void ringdetectbaud(CommunicationPortC *)
	{
	}
#else
static void ringdetectbaud(SerialPortC *CP)
	{
	CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86),
			bauds[CP->IsRingIndicator() ? PS_1200 : PS_300]);

	CP->SetSpeed(CP->IsRingIndicator() ? PS_1200 : PS_300);

	CP->SetModemSpeed(CP->GetSpeed() == PS_300 ? MS_300 : MS_1200);

	if (w)
		{
		destroyCitWindow(w, FALSE);
		}
	}
#endif


// --------------------------------------------------------------------------
// getModStr(): get a string from the modem, waiting for up to 3 secs for it.
//
// Input:
//	char *str: String to be filled.
//	uint len: Maximum length of string to be filled, not including nul.
//
// Output:
//	char *str: Filled with string.
//
// Return value:
//	TRUE: String found
//	FALSE: String not found
//
// Notes:
//	Keyboard input aborts. Length is passed, not assumed to be 40.

Bool getModStr(char *str, uint len, CommunicationPortC *CP)
	{
#ifndef WINCIT
	if (debug)
		{
		cPrintf(getdbmsg(1));
		}
#endif

	const time_t tm = time(NULL);
	uint l = 0;

	while ((time(NULL) - tm) < 4
#ifndef WINCIT
			&& !KBReady()
#endif
			&& l <= len)
		{
		if (CP->IsInputReady())
			{
			const int c = CP->Input();

			if (c == '\r' || c == '\n') // CR || LF
				{
				str[l] = 0;
#ifndef WINCIT
				if (debug)
					{
					cPrintf(getdbmsg(0));
					}
#else
            // this was commented out.  i'm not sure why...
            // 66066.nh
            errorDisp("getModStr: \"%s\"", str);
#endif

				return (TRUE);
				}
			else
				{
#ifndef WINCIT
				if (debug)
					{
					cPrintf(pctc, c);
					}
#endif

				str[l] = (char) c;
				l++;
				}
			}
		}

#ifndef WINCIT
	if (debug)
		{
		cPrintf(getdbmsg(2));
		}
#endif

	str[0] = 0;

	return (FALSE);
	}


static ResultCodeEnum FindResultCode(const char *Test)
	{
	const ModemResultCodes *mrc;

	for (mrc = ResultCodes; mrc; mrc = (ModemResultCodes *) getNextLL(mrc))
		{
		if (mrc->Which == MR_CONNECTX || mrc->Which == MR_CONNECTXE || mrc->Which == MR_CONNECTXC)
			{
			const int SpeedPos = strpos('|', mrc->Code);

			if (SpeedPos)
				{
				if (strncmpi(Test, mrc->Code, SpeedPos - 1) == SAMESTRING)
					{
					label SpeedString;
					const char *ch;
					int i;

					for (ch = Test + SpeedPos - 1, i = 0; isdigit(*ch) && i < LABELSIZE; ch++, i++)
						{
						SpeedString[i] = *ch;
						}

					SpeedString[i] = 0;

					if (u_match(ch, mrc->Code + SpeedPos))
						{
						switch(atol(SpeedString))
							{
							default:
							case 300: return ((ResultCodeEnum) (mrc->Which + 3));
							case 600: return ((ResultCodeEnum) (mrc->Which + 6));
							case 1200: return ((ResultCodeEnum) (mrc->Which + 9));
							case 2400: return ((ResultCodeEnum) (mrc->Which + 12));
							case 4800: return ((ResultCodeEnum) (mrc->Which + 15));
							case 7200: return ((ResultCodeEnum) (mrc->Which + 18));
							case 9600: return ((ResultCodeEnum) (mrc->Which + 21));
							case 12000: return ((ResultCodeEnum) (mrc->Which + 24));
							case 14400: return ((ResultCodeEnum) (mrc->Which + 27));
							case 16800: return ((ResultCodeEnum) (mrc->Which + 30));
							case 19200: return ((ResultCodeEnum) (mrc->Which + 33));
							case 21600: return ((ResultCodeEnum) (mrc->Which + 36));
							case 24000: return ((ResultCodeEnum) (mrc->Which + 39));
							case 26400: return ((ResultCodeEnum) (mrc->Which + 42));
							case 28800: return ((ResultCodeEnum) (mrc->Which + 45));
							case 38400l: return ((ResultCodeEnum) (mrc->Which + 48));
							case 57600l: return ((ResultCodeEnum) (mrc->Which + 51));
							case 115200l: return ((ResultCodeEnum) (mrc->Which + 54));
							case 230400l: return ((ResultCodeEnum) (mrc->Which + 57));
							}
						}
					}
				}
			else
				{
				return (mrc->Which);
				}
			}
		else if (u_match(Test, mrc->Code))
			{
			return (mrc->Which);
			}
		}

	return (MR_NUM);
	}

static const char *NextResultCode(const char *Test) 	// MR_RUNAPPLIC
	{
	ModemResultCodes *mrc;

	for (mrc = ResultCodes; mrc; mrc = (ModemResultCodes *) getNextLL(mrc))
		{
		if (SameString(Test, mrc->Code))
			{
			return (mrc->next->Code);
			}
		}

	return (ns);
	}


// --------------------------------------------------------------------------
// smartbaud(): sets baud rate according to result codes
//
// Return value:
//	TRUE: Found baud
//	FALSE: Didn't find baud
//	CERROR: NO CARRIER, ERROR, NO DIALTONE, etc.
//
// Notes:
//	Must return TRUE or FALSE or CERROR, else dumbmodem 7 breaks.

#ifdef WINCIT
int RunApplication(const char *, const char *, Bool, Bool) { return (0); }
#endif

int smartbaud(CommunicationPortC *CP)
	{
	CP->UsingCompression = FALSE;
	CP->UsingCorrection = FALSE;

	char str[128];

	while (getModStr(str, sizeof(str) - 1, CP))
		{
		ResultCodeEnum code;

		code = FindResultCode(str);

		switch (code)
			{
			case MR_RUNAPPLIC:
				{
				CopyStringToBuffer(modem_result, getmsg(449));
				RunApplication(NextResultCode(str), NULL, TRUE, TRUE);
				break;
				}

			case MR_NOCARRIER:
				{
				CopyStringToBuffer(modem_result, getmsg(171));
				return (CERROR);
				}

			case MR_ERROR:
				{
				CopyStringToBuffer(modem_result, getmsg(172));
				return (CERROR);
				}

			case MR_NODIALTONE:
				{
				CopyStringToBuffer(modem_result, getmsg(173));
				return (CERROR);
				}

			case MR_BUSY:
				{
				CopyStringToBuffer(modem_result, getmsg(174));
				return (CERROR);
				}

			case MR_NOANSWER:
				{
				CopyStringToBuffer(modem_result, getmsg(175));
				return (CERROR);
				}

			case MR_COMPRESSION:
				{
				CP->UsingCompression = TRUE;
				break;
				}

			case MR_CORRECTION:
				{
				CP->UsingCorrection = TRUE;
				break;
				}

			case MR_CONNECT300C:	CP->UsingCompression = TRUE;
			case MR_CONNECT300E:	CP->UsingCorrection = TRUE;
			case MR_CONNECT300:
				{
				CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86), bauds[PS_300]);

				CP->SetSpeed(PS_300);
				CopyStringToBuffer(modem_result, getmsg(167));
				CP->SetModemSpeed(MS_300);

				if (w)
					{
					destroyCitWindow(w, FALSE);
					}

				return (TRUE);
				}

			case MR_CONNECT600C:	CP->UsingCompression = TRUE;
			case MR_CONNECT600E:	CP->UsingCorrection = TRUE;
			case MR_CONNECT600:
				{
				CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86), bauds[PS_600]);

				CP->SetSpeed(PS_600);
				CopyStringToBuffer(modem_result, getmsg(167));
				CP->SetModemSpeed(MS_600);

				if (w)
					{
					destroyCitWindow(w, FALSE);
					}

				return (TRUE);
				}

			case MR_CONNECT1200C:	CP->UsingCompression = TRUE;
			case MR_CONNECT1200E:	CP->UsingCorrection = TRUE;
			case MR_CONNECT1200:
				{
				CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86), bauds[PS_1200]);

				CP->SetSpeed(PS_1200);
				CopyStringToBuffer(modem_result, getmsg(168));
				CP->SetModemSpeed(MS_1200);

				if (w)
					{
					destroyCitWindow(w, FALSE);
					}

				return (TRUE);
				}

			case MR_CONNECT2400C:   CP->UsingCompression = TRUE;
			case MR_CONNECT2400E:   CP->UsingCorrection = TRUE;
			case MR_CONNECT2400:
				{
				CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86), bauds[PS_2400]);

				CP->SetSpeed(PS_2400);
				CopyStringToBuffer(modem_result, getmsg(169));
				CP->SetModemSpeed(MS_2400);

				if (w)
					{
					destroyCitWindow(w, FALSE);
					}

				return (TRUE);
				}

			case MR_CONNECT4800C:   CP->UsingCompression = TRUE;
			case MR_CONNECT4800E:   CP->UsingCorrection = TRUE;
			case MR_CONNECT4800:
				{
				CopyStringToBuffer(modem_result, getmsg(181));
				CP->SetModemSpeed(MS_4800);
				return (TRUE);
				}

			case MR_CONNECT7200C:   CP->UsingCompression = TRUE;
			case MR_CONNECT7200E:   CP->UsingCorrection = TRUE;
			case MR_CONNECT7200:
				{
				CopyStringToBuffer(modem_result, getmsg(182));
				CP->SetModemSpeed(MS_7200);
				return (TRUE);
				}

			case MR_CONNECT9600C:   CP->UsingCompression = TRUE;
			case MR_CONNECT9600E:   CP->UsingCorrection = TRUE;
			case MR_CONNECT9600:
				{
				CopyStringToBuffer(modem_result, getmsg(170));
				CP->SetModemSpeed(MS_9600);
				return (TRUE);
				}

			case MR_CONNECT12000C:  CP->UsingCompression = TRUE;
			case MR_CONNECT12000E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT12000:
				{
				CopyStringToBuffer(modem_result, getmsg(176));
				CP->SetModemSpeed(MS_12000);
				return (TRUE);
				}

			case MR_CONNECT14400C:  CP->UsingCompression = TRUE;
			case MR_CONNECT14400E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT14400:
				{
				CopyStringToBuffer(modem_result, getmsg(177));
				CP->SetModemSpeed(MS_14400);
				return (TRUE);
				}

			case MR_CONNECT16800C:  CP->UsingCompression = TRUE;
			case MR_CONNECT16800E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT16800:
				{
				CopyStringToBuffer(modem_result, getmsg(183));
				CP->SetModemSpeed(MS_16800);
				return (TRUE);
				}

			case MR_CONNECT19200C:  CP->UsingCompression = TRUE;
			case MR_CONNECT19200E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT19200:
				{
				CopyStringToBuffer(modem_result, getmsg(178));
				CP->SetModemSpeed(MS_19200);
				return (TRUE);
				}

			case MR_CONNECT21600C:  CP->UsingCompression = TRUE;
			case MR_CONNECT21600E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT21600:
				{
				CopyStringToBuffer(modem_result, getmsg(178));
				CP->SetModemSpeed(MS_21600);
				return (TRUE);
				}

			case MR_CONNECT24000C:  CP->UsingCompression = TRUE;
			case MR_CONNECT24000E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT24000:
				{
				CopyStringToBuffer(modem_result, getmsg(178));
				CP->SetModemSpeed(MS_24000);
				return (TRUE);
				}

			case MR_CONNECT26400C:  CP->UsingCompression = TRUE;
			case MR_CONNECT26400E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT26400:
				{
				CopyStringToBuffer(modem_result, getmsg(178));
				CP->SetModemSpeed(MS_26400);
				return (TRUE);
				}

			case MR_CONNECT28800C:  CP->UsingCompression = TRUE;
			case MR_CONNECT28800E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT28800:
				{
				CopyStringToBuffer(modem_result, getmsg(669));
				CP->SetModemSpeed(MS_28800);
				return (TRUE);
				}

			case MR_CONNECT38400C:  CP->UsingCompression = TRUE;
			case MR_CONNECT38400E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT38400:
				{
				CopyStringToBuffer(modem_result, getmsg(179));
				CP->SetModemSpeed(MS_38400);
				return (TRUE);
				}

			case MR_CONNECT57600C:  CP->UsingCompression = TRUE;
			case MR_CONNECT57600E:  CP->UsingCorrection = TRUE;
			case MR_CONNECT57600:
				{
				CopyStringToBuffer(modem_result, getmsg(180));
				CP->SetModemSpeed(MS_57600);
				return (TRUE);
				}

			case MR_CONNECT115200C: CP->UsingCompression = TRUE;
			case MR_CONNECT115200E: CP->UsingCorrection = TRUE;
			case MR_CONNECT115200:
				{
				CopyStringToBuffer(modem_result, getmsg(159));
				CP->SetModemSpeed(MS_115200);
				return (TRUE);
				}

			case MR_CONNECT230400C: CP->UsingCompression = TRUE;
			case MR_CONNECT230400E: CP->UsingCorrection = TRUE;
			case MR_CONNECT230400:
				{
				CopyStringToBuffer(modem_result, getmsg(668));
				CP->SetModemSpeed(MS_230400);
				return (TRUE);
				}
			}
		}

	return (FALSE);
	}


// --------------------------------------------------------------------------
// checkring(): Checks for #RING (MDMRESLT.CIT) and sends answer string to modem if found.
//
// Notes:
//	This is also where #RUNAPPLIC in MDMRESLT.CIT is processed.

static void checkring(CommunicationPortC *CP)
	{
	if (CP->IsInputReady())
		{
		char str[128];

		// don't be interrupted by cron events
		Cron.ResetTimer();

		// make this pause quite small. it is here to give time for carrier
		// status to be updated after the modem sends the CONNECT string
		pause(25);

		if (CP->HaveConnection())
			{
			return;
			}

		getModStr(str, sizeof(str) - 1, CP);

		if (FindResultCode(str) == MR_RUNAPPLIC)
			{
			CopyStringToBuffer(modem_result, getmsg(449));
			RunApplication(NextResultCode(str), NULL, TRUE, TRUE);
			return;
			}

		if (*cfg.dialring)
			{
#ifdef WINCIT
errorDisp("cfg.dialring");
#endif
			if (	(FindResultCode(str) == MR_RING) &&

					// abs in case you set your clock back
					(abs((int) (time(NULL) - lastringtime)) > (long) cfg.dialringwait))
				{
				CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(90));

				pause(50);
				CP->OutString(cfg.dialring);
				CP->OutString(br);
				time(&lastringtime);

				if (w)
					{
					destroyCitWindow(w, FALSE);
					}
				}
			}
		}
	}


// --------------------------------------------------------------------------
// CheckForNewCarrier(): Checks for new carrier. Do not call if we already have carrier.
//
// Return value:
//	TRUE: Carrier found
//	FALSE: Carrier not found

Bool CheckForNewCarrier(SerialPortC *CP)
	{
	int c;

	// ignore carrier according to uphours & updays
	if (
#ifndef WINCIT
			CP->IsDisabled() ||
#endif
			(!(cfg.uphours[hour()] && cfg.updays[dayofweek()]) && !cfg.ignore_uptime))
		{
		CP->FlushInput();
		return (FALSE);
		}

	// detect #RING in MDMRESLT.CIT?
	checkring(CP);

	if (CP->HaveConnection())
		{
		switch (cfg.dumbmodem)
			{
			case 0: // smartbaud!
			case 3: // smartbaud! dial_ring (obs)
				{
				c = smartbaud(CP);
				break;
				}

			case 1: // returns
				{
				c = findbaud(CP);
				break;
				}

			case 2: // HS on RI
				{
				ringdetectbaud(CP);
				c = TRUE;
				break;
				}

			default:
			case 4: // forced
				{
				smartbaud(CP); // hmm. I dunno about this
				CP->SetSpeed(cfg.initbaud);
				c = TRUE;
				break;
				}

			case 5: // forced
				{
				c = smartbaud(CP);
				if (c)								// perhaps c == TRUE
					{
					if (CP->GetModemSpeed() > MS_2400 || CP->UsingCompression)
						{
						CP->SetSpeed(cfg.initbaud);
						}
					}

				c = TRUE;
				break;
				}

			case 6: 	// downshift
				{
				c = smartbaud(CP);
				if (c)								// perhaps c == TRUE
					{
					if (CP->GetModemSpeed() > MS_2400 || CP->UsingCompression)
						{
						CP->SetSpeed(cfg.initbaud);
						}
					else
						{
						// 300, 1200, 2400
						CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86),
								connectbauds[CP->GetModemSpeed()]);

						CP->CycleDTR();

						CP->OutString(cfg.downshift);
						CP->OutString(br);
						pause(50);

						if (w)
							{
							destroyCitWindow(w, FALSE);
							}
						}
					}

				c = TRUE;
				break;
				}

			case 7: // Practical Peripherals:
					// "CARRIER", DCD on, "PROTOCOL", "CONNECT"
					// comparing with TRUE is evil but more efficient
				{
				c = FALSE;
				while (CP->HaveConnection() && (c != TRUE)) 	// (c==FALSE || c==CERROR)
					{
					c = smartbaud(CP);
					if (c == TRUE)	// (c!=FALSE && c!=CERROR)
						{
						if (CP->GetModemSpeed() > MS_2400 || CP->UsingCompression)
							{
							CP->SetSpeed(cfg.initbaud);
							}
						else
							{
							// 300, 1200, 2400
							CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(86),
									connectbauds[CP->GetModemSpeed()]);

							CP->CycleDTR();

							CP->OutString(cfg.downshift);
							CP->OutString(br);
							pause(50);

							if (w)
								{
								destroyCitWindow(w, FALSE);
								}
							}
						}
					}

				break;
				}
			}

		if (c == TRUE)
			{
#ifndef WINCIT
			CarrierJustFound();
			StatusLine.Update();
#endif
			return (TRUE);
			}
		else
			{
#ifndef WINCIT
			StatusLine.Update(WC_TWp);

			CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(19));

			Initport();

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
#endif

			return (FALSE);
			}
		}
	else
		{
		return (FALSE);
		}
	}
