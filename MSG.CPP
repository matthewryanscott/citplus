// --------------------------------------------------------------------------
// Citadel: Msg.CPP
//
// Often-used (non-overlayed) message related routines.
// Compare to MsgOvl.cpp, which is overlayed

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "msg.h"
#include "log.h"
#include "group.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// clearmsgbuf()	this clears the message buffer out
// indexslot()		give it a message # and it returns a slot#
// MaySeeMsg() 		Can see message by slot #. 99%
// sizetable()		returns # messages in table


MessageDatC MessageDat;

// --------------------------------------------------------------------------
// MaySeeMsg(): Can see message by slot #? 99+%

MaySeeMsgType LogEntry::MaySeeMessage(m_index ID)
	{
	const m_index OrigID = ID;

	MsgTabEntryS Tab;
	if (!MessageDat.GetTabEntry(ID, &Tab))
		{
		return (MSM_SCROLLED);
		}

	// seek out original message table entry
	Bool copy = FALSE;

	while (Tab.Flags.IsCopy())
		{
		copy = TRUE;
		const m_index CopyOffset = Tab.GetCopyOffset();

		assert(CopyOffset);

		if (!CopyOffset)
			{
			return (MSM_ERROR);
			}

		ID -= CopyOffset;
		if (!MessageDat.GetTabEntry(ID, &Tab))
			{
			return (MSM_COPYSCROLLED);
			}
		}

	if (Tab.Flags.IsCensored() && (!IsViewCensoredMessages() && !IsNode()))
		{
		return (MSM_CENSORED);
		}

	// check for PUBLIC non problem user messages first
	if (!Tab.ToHash && !Tab.Flags.IsProblem() && !Tab.Flags.IsModerated() && !Tab.Flags.IsMassemail() &&
			!Tab.Flags.IsLimited())
		{
		return (MSM_GOOD);
		}

	label Buffer;
	const int CUHash = hash(GetName(Buffer, sizeof(Buffer)));

	if (Tab.Flags.IsProblem() || Tab.Flags.IsModerated())
		{
		if (Tab.AuthHash == CUHash)
			{
			// problem or moderated users cannot see copies of their own messages
			if (copy)
				{
				return (MSM_COPYOWNTWIT);
				}
			}
		else
			{
			// but everyone else cannot see the original if it has been released
			if (!copy && MessageDat.IsMadevis(OrigID))
				{
				return (MSM_RELEASEDORIG);
				}

			// problem user message...
			if (Tab.Flags.IsProblem() && !IsAide() && !Tab.Flags.IsMadevis())
				{
				return (MSM_TWIT);
				}

			// moderated message...
			if (Tab.Flags.IsModerated() && !CanModerateRoom(Tab.RoomNum) && !Tab.Flags.IsMadevis())
				{
				return (MSM_MODERATED);
				}
			}
		}

	// mass e-mail or group only
	if (Tab.Flags.IsLimited() || Tab.Flags.IsMassemail())
		{
		for (g_slot i = 0; i < cfg.maxgroups; ++i)
			{
			// check to see which group message is to
			if (GroupData[i].IsInuse() && (hash(GroupData[i].GetName(Buffer, sizeof(Buffer))) == Tab.ToHash))
				{
				return (IsInGroup(i) ? MSM_GOOD : MSM_NOTINGROUP);
				}
			}

		// The group does not exist: can only read if a sysop.
		return (IsSysop() ? MSM_GOOD : MSM_GROUPBAD);
		}

	if (Tab.Flags.IsMail())
		{
		// check for messages to sysop or aide if not net (to sysop/aide @ othernode)
		if (!Tab.Flags.IsNet())
			{
			// sysops see messages to 'Sysop', but not to sysop @ blah on local
			if (IsSysop() && (Tab.ToHash == (int) hash(getmsg(386))))
				{
				return (MSM_GOOD);
				}
	
			// aides see messages to 'Aide', but not to aide @ blah on local
			if (IsAide() && (Tab.ToHash == (int) hash(getmsg(385))))
				{
				return (MSM_GOOD);
				}
			}

		// check for messages from sysop or aide if not from net (from sysop/aide @ othernode)
		if (!Tab.Flags.IsNetworked())
			{
			// sysops see messages from 'Sysop', but not fr sysop @ blah on local
			if (IsSysop() && (Tab.AuthHash == (int) hash(getmsg(386))))
				{
				return (MSM_GOOD);
				}
	
			// aides see messages from 'Aide', but not fr aide @ blah on local
			if (IsAide() && (Tab.AuthHash == (int) hash(getmsg(385))))
				{
				return (MSM_GOOD);
				}
			}

#ifdef GOODBYE
		// Unlogged users see no mail if not sysop and not aide.
		if (!loggedIn)
			{
			return (MSM_NOTLOGGEDIN);
			}
#endif

		// author can see his or her own private messages...
		//	but ONLY on local system
		if (Tab.AuthHash == CUHash && !Tab.Flags.IsNetworked())
			{
			return (MSM_GOOD);
			}

#ifdef GOODBYE
		// This one is easier to understand
		// recipient can see private messages
		if (Tab.ToHash == CUHash)
		{
			if (!Tab.Flags.IsForwarded())
			{
				if (!Tab.Flags.IsNet())
				{
					return (MSM_GOOD);
				}
			}
			else
			{
				if (!Tab.Flags.IsNetworked())
				{
					return (MSM_GOOD);
				}
			}
		}
#endif
		// recipient can see private messages
		if (Tab.ToHash == CUHash
			&&
			// Keeps out unforwarded outbound messages
			(Tab.Flags.IsForwarded() || !Tab.Flags.IsNet())
			&&
			// Keeps out forwarded networked messages
			!(Tab.Flags.IsForwarded() && Tab.Flags.IsNetworked())
			)
			{
			return (MSM_GOOD);
			}

		// forwardee can see private messages
		if (Tab.Flags.IsForwarded())
			{
			// if networked message, the FORWARDEE hash is stored in the
			//	RECIPIENT hash and RECIPIENT does not see message.
			if (((!Tab.Flags.IsNetworked()) ? (int) Tab.OriginID : (int) Tab.ToHash) == CUHash && !Tab.Flags.IsNet())
				{
				return (MSM_GOOD);
				}
			}

		// none of those so cannot see message
		return (MSM_MAIL);
		}

	return (MSM_GOOD);
	}
