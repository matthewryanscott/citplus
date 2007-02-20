// --------------------------------------------------------------------------
// Citadel: Term.CPP
//
// Terminal emulation stuff.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "cwindows.h"
#include "term.h"


// --------------------------------------------------------------------------
// Contents
//
// setTerm()		Setup the terminal
// termCap()		Does a terminal command
// putCode()		Sends the escape code to the modem


// --------------------------------------------------------------------------
// putCode(): Sends the escape code to the modem.

void TermCapC::putCode(const char *str)
	{
	if (tw()journalfl)
		{
		if (debug)
			{
			fprintf(tw()journalfl, pcts, str);
			}
		}
	else
		{
		if (tw()OC.Printing && debug)
			{
			fprintf(tw()OC.PrintFile, pcts, str);
			}

		if (!tw()OC.Modem)
			{
			return;
			}

		while (*str)
			{
			tw()CommPort->Output(*str);
			str++;
			}
		}
	}


// --------------------------------------------------------------------------
// localTermCap(): Does a local (non-networked) terminal command.

void TERMWINDOWMEMBER localTermCap(const char *c)
	{
	if (!OC.MCI_goto)
		{
		const char d = c[0];

		if (d == TERM_BS[0])
			{
			doBS();
			pause(5);
			}
		else if (d == TERM_IMPERV[0])
			{
			OC.SetOutFlag(NOSTOP);
			}
		else if (d == TERM_PAUSE[0])
			{
			CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
					CitWindowsMsg(NULL, getmsg(30));

			pause(100);

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
			}
		else if (d == TERM_HANGUP[0])
			{
			CITWINDOW *w = ScreenSaver.IsOn() ? NULL :
					CitWindowsMsg(NULL, getmsg(685), getmsg(81));

			Hangup();

			if (w)
				{
				destroyCitWindow(w, FALSE);
				}
			}
		else if (d == TERM_CONT[0])
			{
			OC.User.SetContinuous(TRUE);
			}
		else if (d == TERM_MCI_ON[0])
			{
			OC.UseMCI = TRUE;
			}
		else if (d == TERM_MCI_DEF[0])
			{
			OC.UseMCI = cfg.mci;
			}
		else if (d == TERM_REPLACE_OFF[0])
			{
			OC.NoReplace = TRUE;
			}
		else if (d == TERM_REPLACE_ON[0])
			{
			OC.NoReplace = FALSE;
			}
		}
	}


// --------------------------------------------------------------------------
// termCap(): Does a terminal command.

void TERMWINDOWMEMBER termCap(const char *c)
	{
	if (!*c)
		{
		return;
		}

	if (toupper(*c) == 'X')
		{
		MCI(c);
		}
	else
		{
		if (!OC.MCI_goto)
			{
			char ANSICode = c[0];

			if (ANSICode == TERM_RND_BACK[0])
				{
				ANSICode = (char) ('A' + random(8));
				}

			if (ANSICode == TERM_RND_FORE[0])
				{
				ANSICode = (char) ('a' + random(8));
				}

			// ISO COLOR support, should handle damn near any case.

			// Since the ^A0 may actualy be hilighted (or the ^A3 may not!)
			// we need to reset the hilight state if the system is flagged
			// 'dirty'.
			//
			// Also if we reset to non-hilighted we need to reoutput the
			// blinking if it was set.

			if (TermCap->IsDirty() &&
					(
						(ANSICode >= 'A' && ANSICode <= 'H') ||
						(ANSICode >= 'a' && ANSICode <= 'h')
					)
				)
				{
				if (TermCap->IsHilight())
					{
					TermCap->SendBold();
					}
				else
					{
					TermCap->SendNormal();
					}

				if (TermCap->IsBlink() && !TermCap->IsHilight())
					{
					TermCap->SendBlink();
					}

				TermCap->SetDirty(FALSE);
				}

			// Send the background
			if (ANSICode >= 'A' && ANSICode <= 'H')
				{
				if (TermCap->IsColor())
					{
					TermCap->SendBackground(ANSICode - 'A');

					OC.ansiattr = (uchar) ((OC.ansiattr & 0x0F) |
							(iso_clr[(ANSICode - 'A')] << 4) |
							(TermCap->IsBlink() ? 0x80 : 0));
					}
				else
					{
					if (cfg.forcetermcap)
						{
						OC.ansiattr = (uchar) ((OC.ansiattr & 0x0F) |
								(iso_clr[(ANSICode - 'A')] << 4) |
								(TermCap->IsBlink() ? 0x80 : 0));
						}
					}
				}
			// Send the forground
			else if (ANSICode >= 'a' && ANSICode <= 'h')
				{
				if (TermCap->IsColor())
					{
					TermCap->SendForeground(ANSICode - 'a');

					OC.ansiattr = (uchar) ((OC.ansiattr & 0xF0) |
							(iso_clr[(ANSICode - 'a')]) |
							(TermCap->IsHilight() ? 0x08 : 0));
					}
				else
					{
					if (cfg.forcetermcap)
						{
						OC.ansiattr = (uchar) ((OC.ansiattr & 0xF0) |
								(iso_clr[(ANSICode - 'a')]) |
								(TermCap->IsHilight() ? 0x08 : 0));
						}
					}
				}
			// Normal Citadel colors
			else if (ANSICode == TERM_BLINK[0])
				{
				char TermCodeBuffer[128];

				if (*TermCap->GetBlink(TermCodeBuffer))
					{
					TermCap->putCode(TermCodeBuffer);

					if (!loggedIn || !cfg.concolors ||
							(cfg.concolors == 1 &&
							(!onConsole || CommPort->HaveConnection())))
						{
						OC.ansiattr = (uchar) (OC.ansiattr | 128);
						}
					else
						{
                        if (TermCap->IsColor())
                            {
						OC.ansiattr = CurrentUser->GetAttribute(ATTR_BLINK) == 128 ?
								(uchar) (OC.ansiattr | 128) :
								(uchar) CurrentUser->GetAttribute(ATTR_BLINK);
                            }
                        else
                            {
						OC.ansiattr = CurrentUser->GetAttribute(ATTR_BLINK) == 128 ?
								(uchar) (OC.ansiattr | 128) :
								(uchar) CurrentUser->GetAttribute(ATTR_BLINK);
                            }
						}

					TermCap->SetBlink(TRUE);
					}
				else
					{
					if (cfg.forcetermcap)
						{
                            OC.ansiattr = (uchar) (OC.ansiattr | 128);
                            TermCap->SetBlink(TRUE);
						}
					}
				}
			else if (ANSICode == TERM_REVERSE[0])
				{
				char TermCodeBuffer[128];

				if (*TermCap->GetInverse(TermCodeBuffer))
					{
					TermCap->putCode(TermCodeBuffer);

					if (!loggedIn || (!cfg.concolors) || (cfg.concolors == 1 && (!onConsole || CommPort->HaveConnection()) ))
						{
						OC.ansiattr = cfg.wattr;
						}
					else
						{
                        if (TermCap->IsColor())
                            {
						OC.ansiattr = (uchar)
								CurrentUser->GetAttribute(ATTR_REVERSE);
                            }
                        else
                            {
                        OC.ansiattr = (uchar) 112;
                            }
						}

					TermCap->SetDirty(TRUE);
					}
				else
					{
					if (cfg.forcetermcap)
						{
                            OC.ansiattr = cfg.wattr;
                            TermCap->SetDirty(TRUE);
						}
					}
				}
			else if (ANSICode == TERM_BOLD[0])
				{
				char TermCodeBuffer[128];

				if (*TermCap->GetBold(TermCodeBuffer))
					{
					TermCap->putCode(TermCodeBuffer);

					if (!loggedIn || (!cfg.concolors) ||
							(cfg.concolors == 1 &&
							(!onConsole || CommPort->HaveConnection())))
						{
						OC.ansiattr = cfg.cattr;
						}
					else
						{
                        if (TermCap->IsColor())
                            {
						OC.ansiattr = (uchar)
								CurrentUser->GetAttribute(ATTR_BOLD);
                            }
                        else
                            {
                        OC.ansiattr = (uchar) 15;
                            }
						}

					TermCap->SetHilight(TRUE);
					TermCap->SetDirty(FALSE);
					}
				else
					{
					if (cfg.forcetermcap)
						{
                            OC.ansiattr = cfg.cattr;
                            TermCap->SetHilight(TRUE);
                            TermCap->SetDirty(FALSE);
						}
					}
				}
			else if (ANSICode == TERM_UNDERLINE[0])
				{
				char TermCodeBuffer[128];

				if (*TermCap->GetUnderline(TermCodeBuffer))
					{
					TermCap->putCode(TermCodeBuffer);

					if (!loggedIn || (!cfg.concolors) ||
							(cfg.concolors == 1 && (!onConsole || CommPort->HaveConnection())))
						{
						OC.ansiattr = cfg.uttr;
						}
					else
						{
                        if (TermCap->IsColor())
                            {
						OC.ansiattr = (uchar)
								CurrentUser->GetAttribute(ATTR_UNDERLINE);
                            }
                        else
                            {
                        OC.ansiattr = (uchar) 15u;
                            }
						}

					TermCap->SetDirty(TRUE);
					}
				else
					{
					if (cfg.forcetermcap)
						{
                            OC.ansiattr = cfg.uttr;
                            TermCap->SetDirty(TRUE);
						}
					}
				}
			else if (ANSICode == TERM_FMT_OFF[0])
				{
				OC.Formatting = FALSE;
				}
			else if (ANSICode == TERM_FMT_ON[0])
				{
				OC.Formatting = TRUE;
				}
			else if (ANSICode == TERM_USERNAME[0])
				{
				if (MCIRecursionChecker.Start(MCI_USERNAME))
					{
					if (OC.UseMCI)
						{
						if (loggedIn)
							{
							char string[100];

							CurrentUser->GetName(string, sizeof(string));
							CopyStringToBuffer(string,
									special_deansi(string, TERM_USERNAME));

							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}
						}
					else
						{
						mFormat(cfg.mci_name);
						}

					MCIRecursionChecker.End(MCI_USERNAME);
					}
				}
			else if (ANSICode == TERM_FIRSTNAME[0])
				{
				if (MCIRecursionChecker.Start(MCI_FIRSTNAME))
					{
					if (OC.UseMCI)
						{
						if (loggedIn)
							{
							char string[100];

							CurrentUser->GetName(string, sizeof(string));

							if (strchr(string, ' '))
								{
								*strchr(string, ' ') = 0;

								if (	SameString(deansi(string), getmsg(131)) ||
										SameString(deansi(string), getmsg(132)) ||
										SameString(deansi(string), getmsg(133))
									)
									{
									CurrentUser->GetName(string,
											sizeof(string));
									}
								else
									{
									char *tmp1, *tmp2;

									for (tmp1 = strchr(string, 0), tmp2 = tmp1 + 1;
											*tmp2; tmp2++)
										{
										if (*tmp2 == CTRL_A)
											{
											*tmp1++ = *tmp2++;
											*tmp1++ = *tmp2;
											}
										}

									*tmp1 = 0;
									}
								}

							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						}
					else
						{
						mFormat(cfg.mci_firstname);
						}

					MCIRecursionChecker.End(MCI_FIRSTNAME);
					}
				}
			else if (ANSICode == TERM_DATE[0])
				{
				if (MCIRecursionChecker.Start(MCI_DATE))
					{
					if (OC.UseMCI)
						{
						char string[100];

						char Buffer[64];
						strftime(string, 79,
								CurrentUser->GetVerboseDateStamp(Buffer,
								sizeof(Buffer)), 0l);

						MCI_DispOrAsgn(string, MCI_str);
						}
					else
						{
						mFormat(cfg.mci_date);
						}

					MCIRecursionChecker.End(MCI_DATE);
					}
				}
			else if (ANSICode == TERM_TIME[0])
				{
				if (MCIRecursionChecker.Start(MCI_TIME))
					{
					if (OC.UseMCI)
						{
						char string[100];

						char Buffer[64];
						strftime(string, 79, CurrentUser->GetDateStamp(
								Buffer, sizeof(Buffer)), 0l);

						MCI_DispOrAsgn(string, MCI_str);
						}
					else
						{
						mFormat(cfg.mci_time);
						}

					MCIRecursionChecker.End(MCI_TIME);
					}
				}
			else if (toupper(ANSICode) == TERM_POOP[0])
				{
				if (MCIRecursionChecker.Start(MCI_POOP))
					{
					if (OC.UseMCI)
						{
						char string[100];

						sprintf(string, pctld, CurrentUser->GetPoopcount());
						MCI_DispOrAsgn(string, MCI_str);
						}
					else
						{
						mFormat(cfg.mci_poop);
						}

					MCIRecursionChecker.End(MCI_POOP);
					}
				}
			else if (ANSICode != TERM_SPLCHK_OFF[0] &&
					ANSICode != TERM_SPLCHK_ON[0])
				{
				// Normal

				char TermCodeBuffer[128];

				if (*TermCap->GetNormal(TermCodeBuffer))
					{
					TermCap->putCode(TermCodeBuffer);

					if (!loggedIn || !cfg.concolors ||
							(cfg.concolors == 1 &&
							(!onConsole || CommPort->HaveConnection())))
						{
						OC.ansiattr = cfg.attr;
						}
					else
						{
						OC.ansiattr = (uchar)
								CurrentUser->GetAttribute(ATTR_NORMAL);
						}

					TermCap->SetHilight(FALSE);
					TermCap->SetBlink(FALSE);
					TermCap->SetDirty(TRUE);
					}
				else
					{
					if (cfg.forcetermcap)
						{
						OC.ansiattr = cfg.attr;
						TermCap->SetHilight(FALSE);
						TermCap->SetBlink(FALSE);
						TermCap->SetDirty(TRUE);
						}
					}
				}
			}
		}
	}


// --------------------------------------------------------------------------
// attrtoansi(): Converts attribute byte into ANSI sequence.
//
// currently never called with mono--take out that code, perhaps?
// 66062(nh)- now called with mono, so blinking can be disabled...

void TermCapC::buildString(char *Buffer, uchar Attr)
	{
// 7 6 5 4 3 2 1 0
// |  \ /  |  \ /
// B   B   I   F
// L   A   N   O
// I   C   T   R
// N   K   E   E
// K       N

	union
		{
		struct
			{
			uint foreground : 3;
			uint intensity	: 1;
			uint background : 3;
			uint blink		: 1;
			} breakdown;

		uchar attr;
		} character;

	character.attr = Attr;

	char ForeText[2], BackText[2];
	ForeText[0] = (char) (iso_clr[character.breakdown.foreground] + '0');
	BackText[0] = (char) (iso_clr[character.breakdown.background] + '0');
	ForeText[1] = BackText[1] = 0;

	if (Color)
		{
		if (character.breakdown.intensity)
			{
			strcpy(Buffer, BoldCode);
			}
		else
			{
			strcpy(Buffer, NormalCode);
			}

		if (character.breakdown.blink)
			{
			strcat(Buffer, FlashCode);
			}

		char Buffer2[128];
		sformat(Buffer2, SetBothCode, "bf", BackText, ForeText, 0);

		strcat(Buffer, Buffer2);
		}
	else // monochrome
		{
		const char *Code;

		if (character.breakdown.blink)	// Blink
			{
			Code = FlashCode;
			}
		else if (character.breakdown.intensity) // Boldface
			{
			Code = BoldCode;
			}
		else if (character.attr == 7)	// Normal
			{
			Code = NormalCode;
			}
		else if (character.attr == 1)	// Underline
			{
			Code = UnderlineCode;
			}
		else if (character.attr == 112) // Reverse Video
			{
			Code = ReverseCode;
			}

		strcpy(Buffer, Code);
		}
	}

const char *TermCapC::GetNormal(char *Buffer)
	{
	strcpy(Buffer, NormalCode);

	if (Color)
		{
		buildString(strchr(Buffer, 0),
				(uchar) tw()CurrentUser->GetAttribute(ATTR_NORMAL));
		}

	return (Buffer);
	}


const char *TermCapC::GetBold(char *Buffer)
	{
	if (Color)
		{
		buildString(Buffer,
				(uchar) tw()CurrentUser->GetAttribute(ATTR_BOLD));
		}
	else
		{
		strcpy(Buffer, BoldCode);
		}

	return (Buffer);
	}


const char *TermCapC::GetInverse(char *Buffer)
	{
	if (Color)
		{
		buildString(Buffer,
				(uchar) tw()CurrentUser->GetAttribute(ATTR_REVERSE));
		}
	else
		{
		strcpy(Buffer, ReverseCode);
		}

	return (Buffer);
	}


const char *TermCapC::GetBlink(char *Buffer)
	{
    if ((Color || IsANSI()) &&
        tw()CurrentUser->GetAttribute(ATTR_BLINK) < 128)
		{
		buildString(Buffer,
				(uchar) tw()CurrentUser->GetAttribute(ATTR_BLINK));
		}
	else
		{
		strcpy(Buffer, FlashCode);
		}

	return (Buffer);
	}


const char *TermCapC::GetUnderline(char *Buffer)
	{
	if (Color)
		{
		buildString(Buffer,
				(uchar) tw()CurrentUser->GetAttribute(ATTR_UNDERLINE));
		}
	else
		{
		strcpy(Buffer, UnderlineCode);
		}

	return (Buffer);
	}

void TermCapC::SendBackground(int Color)
	{
	char Buffer[128], ColorStr[2];

	ColorStr[0] = (char) (Color + '0');
	ColorStr[1] = 0;

	sformat(Buffer, SetBackCode, "b", ColorStr, 0);
	putCode(Buffer);
	}

void TermCapC::SendForeground(int Color)
	{
	char Buffer[128], ColorStr[2];

	ColorStr[0] = (char) (Color + '0');
	ColorStr[1] = 0;

	sformat(Buffer, SetForeCode, "f", ColorStr, 0);
	putCode(Buffer);
	}

void TermCapC::SendBoth(int Fore, int Back)
	{
	char Buffer[128], ForeStr[2], BackStr[2];

	ForeStr[0] = (char) (Fore + '0');
	ForeStr[1] = 0;
	BackStr[0] = (char) (Back + '0');
	BackStr[1] = 0;

	sformat(Buffer, SetBothCode, "bf", BackStr, ForeStr, 0);
	putCode(Buffer);
	}

void TermCapC::SendBlink(void)
	{
	putCode(FlashCode);
	}

void TermCapC::SendBold(void)
	{
	putCode(BoldCode);
	}

void TermCapC::SendNormal(void)
	{
	putCode(NormalCode);
	}

void TermCapC::SendReverse(void)
	{
	putCode(ReverseCode);
	}

void TermCapC::SendUnderline(void)
	{
	putCode(UnderlineCode);
	}

void TermCapC::SendDelEOL(void)
	{
	putCode(DelEolCode);
	}

void TermCapC::SendClearScreen(void)
	{
	putCode(ClsCode);
	}

void TermCapC::SaveCursor(void)
	{
	putCode(SaveCursCode);
	}

void TermCapC::RestoreCursor(void)
	{
	putCode(RestCursCode);
	}

void TermCapC::CursorLeft(int Delta)
	{
	label DeltaStr;

	if (Delta == 1 && BlankForCursorOne)
		{
		*DeltaStr = 0;
		}
	else
		{
		ltoa(Delta, DeltaStr, 10);
		}

	char TermBuffer[128];

	sformat(TermBuffer, CursorLeftCode, "c", DeltaStr, 0);
	putCode(TermBuffer);
	}

void TermCapC::CursorRight(int Delta)
	{
	label DeltaStr;

	if (Delta == 1 && BlankForCursorOne)
		{
		*DeltaStr = 0;
		}
	else
		{
		ltoa(Delta, DeltaStr, 10);
		}

	char TermBuffer[128];

	sformat(TermBuffer, CursorRightCode, "c", DeltaStr, 0);
	putCode(TermBuffer);
	}

void TermCapC::CursorDown(int Delta)
	{
	label DeltaStr;

	if (Delta == 1 && BlankForCursorOne)
		{
		*DeltaStr = 0;
		}
	else
		{
		ltoa(Delta, DeltaStr, 10);
		}

	char TermBuffer[128];

	sformat(TermBuffer, CursorDownCode, "c", DeltaStr, 0);
	putCode(TermBuffer);
	}

void TermCapC::CursorUp(int Delta)
	{
	label DeltaStr;

	if (Delta == 1 && BlankForCursorOne)
		{
		*DeltaStr = 0;
		}
	else
		{
		ltoa(Delta, DeltaStr, 10);
		}

	char TermBuffer[128];

	sformat(TermBuffer, CursorUpCode, "c", DeltaStr, 0);
	putCode(TermBuffer);
	}

void TermCapC::CursorAbsolute(int Row, int Column)
	{
	label RowStr, ColStr;

	if (BlankForTLCursor)
		{
		if (!Row)
			{
			*RowStr = 0;
			}
		else
			{
			ltoa(Row + CursTopNum, RowStr, 10);
			}

		if (!Column)
			{
			*ColStr = 0;
			}
		else
			{
			ltoa(Column + CursLeftNum, ColStr, 10);
			}
		}
	else
		{
		ltoa(Row + CursTopNum, RowStr, 10);
		ltoa(Column + CursLeftNum, ColStr, 10);
		}

	char TermBuffer[128];

	sformat(TermBuffer, CursorAbsoluteCode, "cr", ColStr, RowStr, 0);
	putCode(TermBuffer);
	}
