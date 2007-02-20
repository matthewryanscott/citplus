// --------------------------------------------------------------------------
// Citadel: UIMisc.CPP
//
// User interface garbage dump; if it ain't elsewhere, it's here.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "room.h"
#include "hall.h"
#include "group.h"
#include "miscovl.h"
#include "term.h"

// --------------------------------------------------------------------------
// Contents
//


#ifdef MULTI
void TwirlyCursorC::Start(TermWindowC *TW)
#else
void TwirlyCursorC::Start(void)
#endif
	{
	if (!Active && tw()CurrentUser->IsTwirly() && !tw()IsKeyFromUserReady())
		{
		Length = strlen(cfg.twirly);
		Active = TRUE;

		tw()oChar(' ');

		tw()CommPort->FlushOutput();
		}
	}


#ifdef MULTI
void TwirlyCursorC::Update(TermWindowC *TW)
#else
void TwirlyCursorC::Update(void)
#endif
	{
	if (Active && !tw()IsKeyFromUserReady())
		{
		// oldprinting = printing
		// printing = FALSE;

		tw()twirlypause(cfg.twirlypause);

		if (++Position >= Length)
			{
			Position = 0;
			}

		tw()oChar('\b');
		tw()oChar(cfg.twirly[Position]);

		tw()CommPort->FlushOutput();
		// printing = oldprinting;
		}
	}

#ifdef MULTI
void TwirlyCursorC::Stop(TermWindowC *TW)
#else
void TwirlyCursorC::Stop(void)
#endif
	{
	if (Active)
		{
		tw()doBS();
		Active = FALSE;

		tw()CommPort->FlushOutput();
		}
	}

void TERMWINDOWMEMBER BadMenuSelection(int C)
	{
	if (C)
		{
		oChar((char) C);
		mPrintfCR(sqst);
		}
	else
		{
		mPrintfCR(qst);
		}

	if (!CurrentUser->IsExpert())
		{
		CRmPrintfCR(getmsg(200));
		}
	}

int TERMWINDOWMEMBER DoMenuPrompt(const char *Text1, const char *Text2)
	{
    int wow;
	OC.SetOutFlag(IMPERVIOUS);

	CRmPrintf(Text1, Text2);

	do
	    {
        wow = iCharNE();
		} while (!wow && HaveConnectionToUser());

	return (wow);
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, h_slot Dfault)
	{
	label HN;
	return (AskHallName(Buffer, Dfault != CERROR ? HallData[Dfault].GetName(HN, sizeof(HN)) : ns, cfg.Lhall_nym, FALSE));
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, const char *Dfault)
	{
	return (AskHallName(Buffer, Dfault, cfg.Lhall_nym, FALSE));
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, h_slot Dfault, const char *Prompt)
	{
	label HN;
	return (AskHallName(Buffer, Dfault != CERROR ? HallData[Dfault].GetName(HN, sizeof(HN)) : ns, Prompt, FALSE));
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, const char *Dfault, const char *Prompt)
	{
	return (AskHallName(Buffer, Dfault, Prompt, FALSE));
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, const char *Dfault, Bool ListOnlyAccessible)
	{
	return (AskHallName(Buffer, Dfault, cfg.Lhall_nym, ListOnlyAccessible));
	}

Bool TERMWINDOWMEMBER AskHallName(char *Buffer, const char *Dfault, const char *Prompt, Bool ListOnlyAccessible)
	{
	do
		{
		getString(Prompt, Buffer, LABELSIZE, Dfault);

		if (*Buffer == '?')
			{
			ListHalls(ListOnlyAccessible);
			}
		} while (*Buffer == '?');

	normalizeString(Buffer);
	return (!!*Buffer);
	}


Bool TERMWINDOWMEMBER AskRoomName(char *Buffer, r_slot Dfault)
	{
	label RN;
	return (AskRoomName(Buffer, Dfault != CERROR ? RoomTab[Dfault].GetName(RN, sizeof(RN)) : ns, cfg.Lroom_nym));
	}

Bool TERMWINDOWMEMBER AskRoomName(char *Buffer, const char *Dfault)
	{
	return (AskRoomName(Buffer, Dfault, cfg.Lroom_nym));
	}

Bool TERMWINDOWMEMBER AskRoomName(char *Buffer, r_slot Dfault, const char *Prompt)
	{
	label RN;
	return (AskRoomName(Buffer, Dfault != CERROR ? RoomTab[Dfault].GetName(RN, sizeof(RN)) : ns, Prompt));
	}

Bool TERMWINDOWMEMBER AskRoomName(char *Buffer, const char *Dfault, const char *Prompt)
	{
	do
		{
		getString(Prompt, Buffer, LABELSIZE, Dfault);

		if (*Buffer == '?')
			{
			ListAllRooms(FALSE, !CurrentUser->IsSysop(), FALSE);
			}
		} while (*Buffer == '?');

	normalizeString(Buffer);
	return (!!*Buffer);
	}


Bool TERMWINDOWMEMBER AskGroupName(char *Buffer)
	{
	return (AskGroupName(Buffer, NULL, cfg.Lgroup_nym, LABELSIZE, FALSE));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, Bool ListOnlyInGroup)
	{
	return (AskGroupName(Buffer, NULL, cfg.Lgroup_nym, LABELSIZE, ListOnlyInGroup));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, const char *Dfault)
	{
	return (AskGroupName(Buffer, Dfault, cfg.Lgroup_nym, LABELSIZE, FALSE));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, const char *Dfault, Bool ListOnlyInGroup)
	{
	return (AskGroupName(Buffer, Dfault, cfg.Lgroup_nym, LABELSIZE, ListOnlyInGroup));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, g_slot Dfault, const char *Prompt, int Len)
	{
	label GN;
	return (AskGroupName(Buffer, Dfault != CERROR ? GroupData[Dfault].GetName(GN, sizeof(GN)) : ns, Prompt, Len, FALSE));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, g_slot Dfault,
		const char *Prompt)
	{
	label GN;
	return (AskGroupName(Buffer, Dfault != CERROR ? GroupData[Dfault].GetName(GN, sizeof(GN)) : ns, Prompt, LABELSIZE,
			FALSE));
	}

Bool TERMWINDOWMEMBER AskGroupName(char *Buffer, const char *Dfault, const char *Prompt, int Len, Bool ListOnlyInGroup)
	{
	label wowPrompt;
	strcpy(wowPrompt, Prompt);
	do
		{
		getString(wowPrompt, Buffer, Len, Dfault);

		if (*Buffer == '?')
			{
			ListGroups(ListOnlyInGroup);
    		if (!*Prompt)
    		    {
    		    strcpy(wowPrompt, cfg.Lgroup_nym);
				}
			}
		} while (*Buffer == '?');

	normalizeString(Buffer);
	return (!!*Buffer);
	}


Bool TERMWINDOWMEMBER AskUserName(char *Buffer, const LogEntry *Dfault)
	{
	label UN;
    return (AskUserName(Buffer, Dfault ? Dfault->GetName(UN, sizeof(UN)) : ns, cfg.Luser_nym, LABELSIZE, NULL, FALSE, FALSE));
	}

Bool TERMWINDOWMEMBER AskUserName(char *Buffer, const char *Dfault)
	{
    return (AskUserName(Buffer, Dfault, cfg.Luser_nym, LABELSIZE, NULL, FALSE, FALSE));
	}

Bool TERMWINDOWMEMBER AskUserName(char *Buffer)
	{
    return (AskUserName(Buffer, NULL, cfg.Luser_nym, LABELSIZE, NULL, FALSE, FALSE));
	}

Bool TERMWINDOWMEMBER AskUserName(char *Buffer, const char *Dfault, const char *Prompt, int Len, Bool *GetStringReturnVal, Bool ListOnlyLoggedIn, Bool fingerYes)
	{
	label wowPrompt;
	strcpy(wowPrompt, Prompt);
	do
		{
		Bool wow = getString(wowPrompt, Buffer, Len, Dfault);
		if (GetStringReturnVal)
			{
			*GetStringReturnVal = wow;
			}
        if ((*Buffer == '?') && !fingerYes)
			{
			ListUsers(CurrentUser->IsAide(), ListOnlyLoggedIn);
    		if (!*Prompt)
    		    {
    		    strcpy(wowPrompt, cfg.Luser_nym);
				}
            }
        else if ((*Buffer == '?') && fingerYes)
            {
            FingerList(FALSE);
            }
        } while (*Buffer == '?');

	normalizeString(Buffer);
	return (!!*Buffer);
	}
