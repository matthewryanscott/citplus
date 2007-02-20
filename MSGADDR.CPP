// --------------------------------------------------------------------------
// Citadel: MsgAddr.CPP
//
// Stuff to deal with addressing messages as mail

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "maillist.h"

#ifdef WINCIT
Bool ResolveMessageAddressingIssues(Message *Msg, TermWindowC *TW, ModemConsoleE W)
#else
Bool ResolveMessageAddressingIssues(Message *Msg, ModemConsoleE W)
#endif
	{
	l_slot logslot;

	if (*Msg->GetToNodeName())
		{
		logslot = FindPersonByPartialName(Msg->GetToNodeName(), NULL);
		}
	else
		{
		logslot = FindPersonByPartialName(Msg->GetToUser(), NULL);
		}

	if (logslot == CERROR && *Msg->GetDestinationAddress())
		{
		logslot = nodexists(Msg->GetDestinationAddress(), NULL);

		if (logslot != CERROR)
			{
			label Buffer;
			Msg->SetToNodeName(LogTab[logslot].GetName(Buffer, sizeof(Buffer)));

			label Buffer1, ToAddress;
			makeaddress(LogTab[logslot].GetAlias(Buffer, sizeof(Buffer)),
					LogTab[logslot].GetLocID(Buffer1, sizeof(Buffer1)), ToAddress);
			Msg->SetDestinationAddress(ToAddress);
			}
		}

	if (logslot == CERROR)
		{
		if (!*Msg->GetToNodeName())
			{
			// forward 'sysop' messages to #sysop if #forward
			if (SameString(Msg->GetToUser(), getmsg(386)) && cfg.forward && (FindPersonByName(cfg.sysop, NULL) != CERROR))
				{
				Msg->SetToUser(cfg.sysop);

				if (TW)
					{
					tw()wPrintf(W, getmsg(563), cfg.Lmsg_nym, cfg.sysop);
					tw()wDoCR(W);
					}
				}
			else if (!SameString(Msg->GetToUser(), getmsg(386)) && !SameString(Msg->GetToUser(), getmsg(385)) &&
					!IsMailingList(Msg->GetToUser()))
				{
				if (TW)
					{
					tw()wDoCR(W);
					tw()wPrintf(W, getmsg(595), Msg->GetToUser());
					tw()wDoCR(W);
					}

				return (FALSE);
				}
			}
		}
	else
		{
		if (*Msg->GetToNodeName())
			{
			label Buffer;
			Msg->SetToNodeName(LogTab[logslot].GetName(Buffer, sizeof(Buffer)));
			}
		else
			{
			label Buffer;
			Msg->SetToUser(LogTab[logslot].GetName(Buffer, sizeof(Buffer)));
			}

		if (LogTab[logslot].IsNode() && !(*Msg->GetToNodeName()))
			{
			Msg->SetToNodeName(Msg->GetToUser());
			Msg->SetToUser(getmsg(386));

			if (TW)
				{
				tw()wPrintf(W, getmsg(562), cfg.Umsg_nym, Msg->GetToNodeName());
				tw()wDoCR(W);
				}
			}
		}

	if (logslot != CERROR && !*Msg->GetToNodeName())
		{
		LogEntry1 Log1;
		label ForwardAddr;

		if (Log1.Load(LogTab[logslot].GetLogIndex()) && *(Log1.GetForwardAddr(ForwardAddr, sizeof(ForwardAddr))))
			{
			if (!Log1.IsForwardToNode())
				{
				// Regular forwarding
				l_slot logslot2;

				logslot2 = FindPersonByName(ForwardAddr, NULL);

				if (logslot2 != CERROR)
					{
					label Buffer;
					LogTab[logslot2].GetName(Buffer, sizeof(Buffer));

					Msg->SetForward(Buffer);

					if (TW)
						{
						tw()wPrintf(W, getmsg(561), cfg.Umsg_nym);
						tw()wPrintf(W, pcts, Msg->GetForward());
						}
					}

				if (TW)
					{
					tw()wDoCR(W);
					}
				}
			else
				{
				// Forwarding to node
				label Buffer;
				Log1.GetForwardAddrNode(Buffer, sizeof(Buffer));

				Msg->SetForward(ForwardAddr);
				Msg->SetToNodeName(Buffer);

				if (TW)
					{
					tw()wPrintf(W, getmsg(561), cfg.Umsg_nym);
					tw()wPrintf(W, getmsg(101), ForwardAddr, Buffer);
					}
				}
			}
		}

	return (TRUE);
	}
