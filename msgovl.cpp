// --------------------------------------------------------------------------
// Citadel: MsgOvl.CPP
//
// This is the overlayed high level message code.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "log.h"
#include "net.h"
#include "filemake.h"
#include "group.h"

// --------------------------------------------------------------------------
// Contents
//
// MaySeeMsg() 			returns TRUE if person can see message. 100%

//	 Truth Tables
//
//	 (!a || !b) 	   ==	  !(a && b)
//	 ----------------		  -----------------
//	   1	 1	=  0			1	 1	=  0
//	   0	 1	=  1			0	 1	=  1
//	   1	 0	=  1			1	 0	=  1
//	   0	 0	=  1			0	 0	=  1
//
//
//	 (!a || b)
//	 ----------------
//	   1	1	=  1
//	   0	1	=  1
//	   1	0	=  0
//	   0	0	=  1


// --------------------------------------------------------------------------
// MaySeeMsg(): Returns TRUE if person can see message. 100%

MaySeeMsgType LogEntry::MaySeeMessage(Message *Msg)
	{
	char msgflags_NET;
	char msgflags_NETWORKED;

	// Message is censored, user does not view censored messages, and user is not node
	if ((Msg->GetOriginalAttribute() & ATTR_CENSORED) && !IsViewCensoredMessages() && !IsNode())
		{
		return (MSM_CENSORED);
		}

	// check for PUBLIC non problem user messages first
	if (!*Msg->GetToUser() && !*Msg->GetX() && !*Msg->GetGroup())
		{
		return (MSM_GOOD);
		}

	if (*Msg->GetX())
		{
		if (IsSameName(Msg->GetAuthor()))
			{
			// problem users cannot see copies of their own messages
			if (Msg->IsViewDuplicate())
				{
				return (MSM_COPYOWNTWIT);
				}
			}
		else
			{
			// but everyone else cannot see the orignal if it has been
			// released
			if (!Msg->IsViewDuplicate() && (Msg->GetOriginalAttribute() & ATTR_MADEVIS))
				{
				return (MSM_RELEASEDORIG);
				}

			// problem user message... only Aides see them
			if (*Msg->GetX() == 'Y' && !IsAide() && !Msg->IsMadeVisible())
				{
				return (MSM_TWIT);
				}

			// moderated message... only room moderators see them
			if (*Msg->GetX() == 'M' && !CanModerateRoom(Msg->GetRoomNumber()) && !Msg->IsMadeVisible())
				{
				return (MSM_MODERATED);
				}
			}
		}

	// mass e-mail or group only
	if (*Msg->GetGroup())
		{
		g_slot i = FindGroupByName(Msg->GetGroup());

		if (i != CERROR)
			{
			return (IsInGroup(i) ? MSM_GOOD : MSM_NOTINGROUP);
			}
		else
			{
#ifdef GOODBYE
			if (IsSysop())
				{
				mPrintfCR("        Unknown %s %s!", cfg.Lgroup_nym, Msg->GetGroup());
				}
#endif

			return (IsSysop() ? MSM_GOOD : MSM_GROUPBAD);
			}
		}

	// The following stuff was copied from MsgTab.Index()
	// when updating that function make sure to update these too

	msgflags_NET = (char) (Msg->IsNetworkedMail() && !Msg->IsNetworkedMailForHere());
	msgflags_NETWORKED = (char) (*Msg->GetSourceID() != 0);

	if (Msg->IsMail())
		{
		if (!msgflags_NET)
			{
			// sysops see messages to 'Sysop'
			// (except messages to sysop @ blah from being seen on local)
			if (IsSysop() && SameString(Msg->GetToUser(), getmsg(386)))
				{
				return (MSM_GOOD);
				}
	
			// aides see messages to 'Aide'
			// (except messages to aide @ blah from being seen on local)
			if (IsAide() && SameString(Msg->GetToUser(), getmsg(385)))
				{
				return (MSM_GOOD);
				}
			}

		if (!msgflags_NETWORKED)
			{
			// sysops see messages from 'Sysop'
			// (except messages fr sysop @ blah from being seen on local)
			if (IsSysop() && SameString(Msg->GetAuthor(), getmsg(386)))
				{
				return (MSM_GOOD);
				}
	
			// aides see messages from 'Aide'
			// (except messages fr aide @ blah from being seen on local
			if (IsAide() && SameString(Msg->GetAuthor(), getmsg(385)))
				{
				return (MSM_GOOD);
				}
			}

		// Unlogged users see no mail if not sysop and not aide.
#ifdef GOODBYE
		// is this needed? only if [Not Logged In] can leave mail, I guess...
		if (!loggedIn)
			{
			return (MSM_NOTLOGGEDIN);
			}
#endif

		// author can see his own private messages
		// but ONLY on local system.
		if (!msgflags_NETWORKED && IsSameName(Msg->GetAuthor()))
			{
			return (MSM_GOOD);
			}

#ifdef GOODBYE
		// This one is easier to understand
		// recipient can see private messages
		if (SameString(Msg->GetToUser(), GetName())
		{
			if (!*Msg->GetForward())
			{
				if (!msgflags_NET)
				{
					return (MSM_GOOD);
				}
			}
			else
			{
				if (!msgflags_NETWORKED)
				{
					return (MSM_GOOD);
				}
			}
		}
#endif

		// But this one is more compact...
		if (IsSameName(Msg->GetToUser())
			&&
			// Keeps out unforwarded outbound messages
			(*Msg->GetForward() || !msgflags_NET)
			&&
			// Keeps out forwarded networked messages
			!(*Msg->GetForward() && msgflags_NETWORKED)
			)
			{
			return (MSM_GOOD);
			}

		// forwardee can see private messages
		if (!msgflags_NET && *Msg->GetForward() && IsSameName(Msg->GetForward()))
			{
			return (MSM_GOOD);
			}

		// none of those so cannot see message
		return (MSM_MAIL);
		}

	return (MSM_GOOD);
	}

Message& Message::operator =(const Message &Original)
	{
	VerifyHeap();
	assert(this != NULL);

	disposeUnknownList();
	disposeComments();

	unkLst *cur;
	for (cur = Original.firstUnk; cur;
			cur = (unkLst *) getNextLL(cur))
		{
		unkLst *New = addUnknownList();

		if (New)
			{
			memcpy(New + sizeof(New->next), cur + sizeof(cur->next),
					sizeof(*New) - sizeof(New->next));
			}
		}

	strList *sl;
	for (sl = Original.Comments; sl; sl = (strList *) getNextLL(sl))
		{
		AddComment(sl->string);
		}

	cur = firstUnk;
	sl = Comments;

	memcpy(this, &Original, sizeof(Message));

	firstUnk = cur;
	Comments = sl;

	VerifyHeap();
	return (*this);
	}

Message::Message(const Message &Original)
	{
	VerifyHeap();
	assert(this != NULL);

	firstUnk = NULL;		// Because ClearAll() tries to dispose it.
	Comments = NULL;		// same thing.
	ClearAll();

	unkLst *cur;
	for (cur = Original.firstUnk; cur;
			cur = (unkLst *) getNextLL(cur))
		{
		unkLst *New = addUnknownList();

		if (New)
			{
			memcpy(New + sizeof(New->next), cur + sizeof(cur->next),
					sizeof(*New) - sizeof(New->next));
			}
		}

	strList *sl;
	for (sl = Original.Comments; sl; sl = (strList *) getNextLL(sl))
		{
		AddComment(sl->string);
		}

	cur = firstUnk;
	sl = Comments;

	memcpy(this, &Original, sizeof(Message));

	firstUnk = cur;
	Comments = sl;

	VerifyHeap();
	}


Bool Message::SetMoreFlag(const char Flag, Bool New)
	{
	assert(this);
	assert(Flag);

	if (IsMoreFlag(Flag) && !New)
		{
		// Take it out
		for (char *Wow = MoreFlags; *Wow; Wow++)
			{
			if (*Wow == Flag)
				{
				for (char *Wow2 = Wow; *Wow2; Wow2++)
					{
					*Wow2 = *(Wow2 + 1);
					}

				Wow--;	// Retest this position, in case flag in multiple times
				}
			}
		}
	else if (!IsMoreFlag(Flag) && New)
		{
		// Add it
		if (strlen(MoreFlags) < sizeof(MoreFlags) - 1)
			{
			char *Wow = strchr(MoreFlags, 0);

			*Wow = Flag;
			*(Wow + 1) = 0;
			}
		else
			{
			return (FALSE);
			}
		}

	return (!!Flag);	// Flag == 0 is bad
	}
