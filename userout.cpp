// --------------------------------------------------------------------------
// Citadel: UserOut.CPP
//
// This is a collection of routines that give output to the user.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "hall.h"
#include "events.h"
#include "term.h"


// --------------------------------------------------------------------------
// Contents
//
// GiveRoomPrompt() 	Gives the usual "THISROOM>" prompt.
// SetOutOK()			Sets for usual output.

void UserOutputControl::Reset(void)
	{
	ResetCanFlags();
	CanR = cfg.twitrev;

	OutFlag = OUTOK;
	Continuous = FALSE;
	}


// --------------------------------------------------------------------------
// GiveRoomPrompt(): Gives the usual "THISROOM>" prompt.

void TERMWINDOWMEMBER GiveRoomPrompt(Bool JustDisplayedEvent)
	{
	static int roomCounter;
	label rn, gn, np, ex, hn;
	char rd[81];
	char tm[128], dt[128];
	int i;

	CommPort->FlushInput();

	OC.User.Reset();

	OC.Echo = BOTH;
	OC.setio();

	OC.ansiattr = cfg.attr;

	doEvent(EVT_ROOMPROMPT);

	if (CurrentUser->IsYouAreHere())
		{
		youAreHere();
		}

#ifdef WINCIT
	// if (!PendingSystemEvents)
    	// {
    	OC.SetOutFlag(PendingSystemEvents ? NOSTOP : IMPERVIOUS);
	// }

//	if (IdlePrompt)
//		{
//		return;
//		}

	if (!(CurrentUser->IsErasePrompt() && JustDisplayedEvent))
    	{
	    doCR();
		}

	if (JustDisplayedEvent)
		{
		numLines = 0;
		}
#endif

	OC.Formatting = FALSE;

	roomCounter++;

	RoomTab[thisRoom].GetName(rn, sizeof(rn));

	*gn = 0;
	if (RoomTab[thisRoom].IsGroupOnly())
		{
		if (RoomTab[thisRoom].IsBooleanGroup())
			{
			CopyStringToBuffer(gn, getmsg(471));
			}
		else
			{
			GroupData[RoomTab[thisRoom].GetGroupNumber()].GetName(gn, sizeof(gn));
			}
		}

	*np = 0;
	if (RoomTab[thisRoom].IsShared())
		{
		label Buffer;
		CopyStringToBuffer(np, loggedIn ? CurrentUser->GetNetPrefix(Buffer, sizeof(Buffer)) : cfg.netPrefix);
		}


	*ex = 0;
	i = 0;
	if (RoomTab[thisRoom].IsMsDOSdir())
		{
		if (CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended())
			{
			ex[i++] = roomCounter == 6969 ? '\xcc' : '\xb9';
			}
		else
			{
			ex[i++] = roomCounter == 6969 ? '[' : ']';
			}
		}

	if (RoomTab[thisRoom].IsGroupOnly())
		{
		if (CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended())
			{
			ex[i++] = roomCounter == 6969 ? '\xba' : '\xb3';
			}
		else
			{
			ex[i++] = roomCounter == 6969 ? ';' : ':';
			}
		}

	if (RoomTab[thisRoom].IsHidden())
		{
		ex[i++] = roomCounter == 6969 ? '(' : ')';
		}
	if (RoomTab[thisRoom].IsBIO())
		{
		ex[i++] = roomCounter == 6969 ? '{' : '}';
		}
	else if (!RoomTab[thisRoom].IsGroupOnly() && !RoomTab[thisRoom].IsMsDOSdir() && !RoomTab[thisRoom].IsHidden())
		{
		if (!(iswindow(thisRoom) && CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended()))
			{
			ex[i++] = roomCounter == 6969 ? '<' : '>';
			}
		}

	if (iswindow(thisRoom))
		{
		if (CurrentUser->IsIBMRoom() && TermCap->IsIBMExtended())
			{
			ex[i++] = roomCounter == 6969 ? '\xae' : '\xaf';
			}
		else
			{
			ex[i++] = roomCounter == 6969 ? '<' : '>';
			}
		}
	ex[i] = 0;

	HallData[thisHall].GetName(hn, sizeof(hn));
	CurrentRoom->GetInfoLine(rd, sizeof(rd));

	char Buffer[64];
	strftime(tm, 127, (loggedIn) ? special_deansi(CurrentUser->GetDateStamp(Buffer, sizeof(Buffer)),
			TERM_TIME) : special_deansi(cfg.datestamp, TERM_TIME), 0l);

	strftime(dt, 127, (loggedIn) ? special_deansi(CurrentUser->GetVerboseDateStamp(Buffer,
			sizeof(Buffer)), TERM_DATE) : special_deansi(cfg.vdatestamp, TERM_DATE), 0l);

	Andy(loggedIn ? CurrentUser->GetPromptFormat(Buffer, sizeof(Buffer)) :
			cfg.prompt, getmsg(638), rn, gn, np, ex, hn, rd, tm, dt);

	OC.Formatting = TRUE;
	ansi(14);
	OC.MCI_goto = FALSE;

	termCap(TERM_NORMAL);
	mPrintf(spc);
    CursorIsAtPrompt = TRUE;

	OC.SetOutFlag(OUTOK);

	if (roomCounter == 6969)
		{
		roomCounter = 0;
		}
	}


// --------------------------------------------------------------------------
// SetOutOK(): Sets for usual output.

void TERMWINDOWMEMBER SetOutOK(void)
	{
	OC.SetOutFlag(OUTOK);
	}


// --------------------------------------------------------------------------
// UserOutputControl::CheckInput(): Returns TRUE if the user has aborted
//	typeout

#ifdef MULTI
Bool OutputControl::CheckInput(Bool Pause, TermWindowC *TW)
#else
Bool OutputControl::CheckInput(Bool Pause)
#endif
	{
	if (User.Continuous)
		{
		Pause = FALSE;
		}

	if (User.ControlD)
		{
		tw()KBReady();			// for screenblanker
		SetOutFlag(OUTSKIP);	// Auto-Stop everything
		return (TRUE);
		}

	// Cannot abort IMPERVIOUS
	if (User.GetOutFlag() == IMPERVIOUS)
		{
		tw()KBReady();			// for screenblanker
		return (FALSE);
		}

	// Carrier loss and not on Console
	if (!tw()HaveConnectionToUser())
		{
		SetOutFlag(OUTSKIP);
		return (TRUE);
		}

	Bool toReturn = FALSE;

	// Check for keypress
	if (tw()BBSCharReady() || Pause)
		{
		int c;

		if (!Pause)
			{
			c = toupper(tw()iCharNE());
			}

		if (Pause || c == 'P' || c == 19)   // P or ^S (XOFF) - Pause
			{
			if (Pause)
				{
				char Buffer[80];
				tw()CurrentUser->GetMorePrompt(Buffer, sizeof(Buffer));

				if (!tw()outSpeech(TRUE, tw()loggedIn ? Buffer : cfg.moreprompt))
					{
					tw()putWord((uchar *) (tw()loggedIn ? Buffer : cfg.moreprompt));
					}
				}

			do
				{
				// wait to resume
				c = toupper(tw()iCharNE());
				} while (!tw()CurrentUser->IsPUnPauses() && (c == 'P' || c == 19));

			if (Pause)
				{
				char Buffer[80];
				int i, backspace = strlen(tw()loggedIn ?
						tw()CurrentUser->GetMorePrompt(Buffer, sizeof(Buffer)) : cfg.moreprompt);

				for (i = 0; i < backspace; i++)
					{
					tw()doBS();
					}
				}
			}

		if (User.GetOutFlag() == NOSTOP)
			{
			return (FALSE);
			}

		const Bool WasFormatting = Formatting;
		Formatting = TRUE;

		switch (c)
			{
			case CTRL_D:
				{
				User.ControlD = TRUE;
				SetOutFlag(OUTSKIP);

				toReturn = TRUE;
				break;
				}

			case '9':
				{
				if (tw()hitSix)
					{
					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (!tw()outSpeech(TRUE, getmsg(69)))
						{
						tw()putWord((uchar *) getmsg(69));
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					tw()doCR();
					}

				break;
				}

			case 'J':                               // jump paragraph
				{
				tw()doCR();

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(327));

				tw()termCap(TERM_BOLD);
				if (!tw()outSpeech(TRUE, getmsg(547)))
					{
					tw()putWord((uchar *) getmsg(547));
					}

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(326));

				tw()doCR();

				SetOutFlag(OUTPARAGRAPH);

				break;
				}

			case 'K':                               // kill
				{
				if (User.CanK)
					{
					tw()MRO.DotoMessage = PULL_IT;

					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (!tw()outSpeech(TRUE, getmsg(546)))
						{
						tw()putWord((uchar *) getmsg(546));
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					SetOutFlag(OUTNEXT);

					toReturn = TRUE;
					}

				break;
				}

			case 'M':                               // mark
				{
				if (User.CanM)
					{
					tw()MRO.DotoMessage = MARK_IT;

					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (!tw()outSpeech(TRUE, getmsg(543)))
						{
						tw()putWord((uchar *) getmsg(543));
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					SetOutFlag(OUTNEXT);

					toReturn = TRUE;
					}

				break;
				}

			case '*':                               // censor
				{
				if (User.CanStar)
					{
					tw()MRO.DotoMessage = CENSOR_IT;

					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (!tw()outSpeech(TRUE, getmsg(538)))
						{
						tw()putWord((uchar *) getmsg(538));
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					SetOutFlag(OUTNEXT);

					toReturn = TRUE;
					}

				break;
				}

			case 'N':                               // next
				{
				tw()doCR();

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(327));

				tw()termCap(TERM_BOLD);
				if (!tw()outSpeech(TRUE, getmsg(537)))
					{
					tw()putWord((uchar *) getmsg(537));
					}

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(326));

				tw()doCR();
				SetOutFlag(OUTNEXT);

				toReturn = TRUE;
				break;
				}

			case 'S':                               // stop
				{
				tw()numLines = 0;	// Don't pause for stop!

				tw()doCR();

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(327));

				tw()termCap(TERM_BOLD);
				if (!tw()outSpeech(TRUE, getmsg(528)))
					{
					tw()putWord((uchar *) getmsg(528));
					}

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(326));

				tw()doCR();
				SetOutFlag(OUTSKIP);

				toReturn = TRUE;
				break;
				}

			case 'C':                               // continuous
				{
				tw()doCR();

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(327));

				tw()termCap(TERM_BOLD);
				if (!User.Continuous)
					{
					if (!tw()outSpeech(TRUE, getmsg(527)))
						{
						tw()putWord((uchar *) getmsg(527));
						}
					}
				else
					{
					if (!tw()outSpeech(TRUE, getmsg(526)))
						{
						tw()putWord((uchar *) getmsg(526));
						}
					}

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(326));

				User.Continuous = !User.Continuous;

				tw()doCR();

				break;
				}

			case 'R':
				{
				if (User.CanR)
					{
					tw()MRO.DotoMessage = REVERSE_READ;
					SetOutFlag(OUTNEXT);
					}

				break;
				}

			case 'V':
				{
				tw()MRO.Verbose = !tw()MRO.Verbose;

				tw()doCR();

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(327));

				tw()termCap(TERM_BOLD);
				if (tw()MRO.Verbose)
					{
					if (!tw()outSpeech(TRUE, getmsg(524)))
						{
						tw()putWord((uchar *) getmsg(524));
						}
					}
				else
					{
					if (!tw()outSpeech(TRUE, getmsg(519)))
						{
						tw()putWord((uchar *) getmsg(519));
						}
					}

				tw()termCap(TERM_NORMAL);
				tw()putWord((const uchar *) getmsg(326));

				tw()doCR();

				break;
				}

			case '!':
				{
				if (User.CanBang)
					{
					tw()MRO.Headerscan = !tw()MRO.Headerscan;

					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (tw()MRO.Headerscan)
						{
						if (!tw()outSpeech(TRUE, getmsg(517)))
							{
							tw()putWord((uchar *) getmsg(517));
							}
						}
					else
						{
						if (!tw()outSpeech(TRUE, getmsg(511)))
							{
							tw()putWord((uchar *) getmsg(511));
							}
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					tw()doCR();
					}

				break;
				}

			case '@':
				{
				if (User.CanAt)
					{
					tw()MRO.DotoMessage = REPLY;

					tw()doCR();

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(327));

					tw()termCap(TERM_BOLD);
					if (!tw()outSpeech(TRUE, getmsg(470)))
						{
						tw()putWord((uchar *) getmsg(470));
						}

					tw()termCap(TERM_NORMAL);
					tw()putWord((const uchar *) getmsg(326));

					tw()doCR();
					}

				break;
				}
			}

		Formatting = WasFormatting;

		if (c == '6')
			{
			tw()hitSix = TRUE;
			}
		else
			{
			tw()hitSix = FALSE;
			}
		}

	return (toReturn);
	}
