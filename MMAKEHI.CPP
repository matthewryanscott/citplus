// --------------------------------------------------------------------------
// Citadel: MMakeHi.CPP
//
// High-level message making code.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "log.h"
#include "net.h"
#include "maillist.h"


// --------------------------------------------------------------------------
// Contents
//
// makeMessage()    is menu-level routine to enter a message
// fakeFullCase()   converts a message in uppercase-only to a


// --------------------------------------------------------------------------
// fakeFullCase(): Converts a message in uppercase-only to a reasonable mix.
//  It can't possibly make matters worse...
//
// Notes:
//  Algorithm: First alphabetic after a period is uppercase, all others are
//  lowercase, excepting pronoun "I" is a special case.  We assume an
//  imaginary period preceding the text.

static void fakeFullCase(char *text)
    {
    char *c;
    char lastWasPeriod;
    char state;

    for (lastWasPeriod = TRUE, c=text; *c; c++)
        {
        if ((*c != '.') && (*c != '?') && (*c != '!') && (*c != 1))
            {
            if (isalpha(*c))
                {
                if (!lastWasPeriod)
                    {
                    *c = (char)tolower(*c);
                    }

                lastWasPeriod = FALSE;
                }
            }
        else
            {
            lastWasPeriod = TRUE ;
            }
        }

    // little state machine to search for ' i '
    #define NUTHIN      0
    #define FIRSTBLANK  1
    #define BLANKI      2

    for (state = NUTHIN, c = text; *c; c++)
        {
        switch (state)
            {
            case NUTHIN:
                {
                if (isspace(*c) || ispunct(*c))
                    {
                    state = FIRSTBLANK;
                    }
                else
                    {
                    state = NUTHIN;
                    }
                break;
                }

            case FIRSTBLANK:
                {
                if (*c == 'i')
                    {
                    state = BLANKI;
                    }
                else
                    {
                    state = NUTHIN;
                    }
                break;
                }

            case BLANKI:
                {
                if (isspace(*c) || ispunct(*c))
                    {
                    state = FIRSTBLANK;
                    }
                else
                    {
                    state = NUTHIN;
                    }

                if (!isalpha(*c))
                    {
                    *(c-1) = 'I';
                    }

                break;
                }
            }
        }
    }


// --------------------------------------------------------------------------
// makeMessage(): Menu-level routine to enter a message.

Bool TERMWINDOWMEMBER makeMessage(Message *Msg, Message *ReplyTo, r_slot PutInRoom)
    {
    // the next three are changed later if responding to aide/sysop messages.
    if (!*Msg->GetAuthor())
        {
        Msg->SetAuthor(CurrentUser);
        }

    if (!*Msg->GetSurname())
        {
        Msg->SetSurname(CurrentUser);
        }

    if (!*Msg->GetTitle())
        {
        Msg->SetTitle(CurrentUser);
        }

    if (*Msg->GetDestinationAddress() && !*Msg->GetToNodeName())
        {
        Msg->SetToNodeName(Msg->GetDestinationAddress());
        }

    // if replying, author becomes recipient. Also record message which
    // message is being replied to
    if (ReplyTo)
        {
        // baskin stuff
        if (SameString(ReplyTo->GetToUser(), getmsg(385)))
            {
            Msg->SetAuthor(getmsg(385));
            label Buffer;
            Msg->SetSurname(CurrentUser->GetName(Buffer, sizeof(Buffer)));
            Msg->SetTitle(ns);
            }

        if (SameString(ReplyTo->GetToUser(), getmsg(386)))
            {
            Msg->SetAuthor(getmsg(386));
            label Buffer;
            Msg->SetSurname(CurrentUser->GetName(Buffer, sizeof(Buffer)));
            Msg->SetTitle(ns);
            }

        Msg->SetToUser(ReplyTo->GetAuthor());
        Msg->SetReplyToMessage(*ReplyTo->GetSourceID() ? ReplyTo->GetSourceID() : ReplyTo->GetLocalID());

        // If networked
        if (*ReplyTo->GetSourceID())
            {
            label Buffer;

            Msg->SetToNodeName(ReplyTo->GetOriginNodeName());
            Msg->SetToRegion(ReplyTo->GetOriginRegion());
            Msg->SetToCountry(ReplyTo->GetOriginCountry());
            Msg->SetDestinationAddress(ReplyTo->GetOriginAddress(Buffer));
            }

        Msg->SetSubject(ReplyTo->GetSubject());
        }
    else
        {
        if (RoomTab[PutInRoom == CERROR ? thisRoom : PutInRoom].IsSubject() && !*Msg->GetSubject())
            {
            char Subject[81];

            getNormStr(getmsg(415), Subject, 80);

            Msg->SetSubject(Subject);
            }
        }

    if (!*Msg->GetGroup() && *Msg->GetToUser())     // not mass email
        {
#ifdef WINCIT
        if (!ResolveMessageAddressingIssues(Msg, this, MODEM))
#else
        if (!ResolveMessageAddressingIssues(Msg, MODEM))
#endif
            {
            return (FALSE);
            }
        }

    // moderated messages
    if (
            (
            // The room is moderated...
            RoomTab[PutInRoom == CERROR ? thisRoom : PutInRoom].IsModerated()

            // ...or networked and not a net user
            || (RoomTab[PutInRoom == CERROR ? thisRoom : PutInRoom].IsShared() && !CurrentUser->IsNetUser())
            )

            // and not mail
            && !*Msg->GetToUser())
        {
        Msg->SetX("M");
        }

    // problem user and not mail, set to problem user message
    if (CurrentUser->IsProblem() && !*Msg->GetToUser())
        {
        Msg->SetX("Y");
        }

    if (RoomTab[PutInRoom == CERROR ? thisRoom : PutInRoom].IsShared())
        {
        Msg->SetSignature(cfg.nodeSignature);
        }

    // echo to both for sysop messages
    if (SameString(Msg->GetToUser(), getmsg(386)) || SameString(Msg->GetToUser(), getmsg(385)) ||
        SameString(Msg->GetAuthor(), getmsg(386)) || SameString(Msg->GetAuthor(), getmsg(385)))
        {
        OC.Echo = BOTH;
        OC.setio();
        }

    // copy author name into message buffer
    if (RoomTab[PutInRoom == CERROR ? thisRoom : PutInRoom].IsAnonymous())
        {
        Msg->SetAuthor(getmsg(360));
        Msg->SetSurname(ns);
        Msg->SetTitle(ns);
        }
    else
        {
        // non-anonymous messages get real name
        Msg->SetRealName(CurrentUser);
        }

    // clear our flags
    delete MS.AbortedMessage;
    MS.AbortedMessage = NULL;

    // set room# and attribute byte for message
    Msg->SetRoomNumber(PutInRoom == CERROR ? thisRoom : PutInRoom);

    if (!*Msg->GetFileLink())
        {
        if (loggedIn && !RoomTab[Msg->GetRoomNumber()].IsAnonymous())
            {
            Msg->SetUserSignature(CurrentUser);
            }

        if (getText(Msg, NULL))
            {
            Bool allUpper;
            const char *pc;

            if (CurrentUser->IsBunny())
                {
                for (pc = Msg->GetText(), allUpper = TRUE;
                        *pc && allUpper; pc++)
                    {
                    if (*pc == 1)
                        {
                        if (*(pc + 1))
                            {
                            pc++;
                            }
                        }
                    else if (toupper(*pc) != *pc)
                        {
                        allUpper = FALSE;
                        }
                    }

                if (allUpper)
                    {
                    fakeFullCase(Msg->GetTextPointer());
                    }
                }
            }
        else
            {
            return (FALSE);
            }
        }
    else
        {
        doCR();
        putheader(TRUE, Msg);
        }

    if (CurrentUser->IsSysop() && Msg->GetX()[0] == 'M')
        {
        char prompt[80];

        sprintf(prompt, getmsg(560), cfg.Lmsg_nym);

        if (getYesNo(prompt, 1))
            {
            Msg->SetX(ns);
            }
        }

    if (*Msg->GetToNodeName())  // save it for netting...
        {
        // room names get included in networked mail
        Msg->SetCreationRoom();

        if (!save_mail(Msg, FALSE, NULL, CERROR))
            {
            delete MS.AbortedMessage;
            MS.AbortedMessage = NULL;

            CRmPrintfCR(getmsg(559), Msg->GetToNodeName(), cfg.Lmsg_nym);

            // 1 because this is somewhat of an important allocation
            compactMemory(1);

            MS.AbortedMessage = new Message;

            if (MS.AbortedMessage)
                {
                *MS.AbortedMessage = *Msg;
                return (FALSE);
                }
            else
                {
                OutOfMemory(42);
                return (FALSE);
                }
            }
        }

    if (putAndNoteMessage(Msg, TRUE))
        {
        dispBlb(B_CENSOR);
        }

    if (*Msg->GetToUser() && !*Msg->GetToNodeName() && IsMailingList(Msg->GetToUser()))
        {
        SendMailToList(Msg);
        }

    if (!(ReplyTo && *ReplyTo->GetToUser()))
        {
        Talley->IncrementEnteredInRoom(Msg->GetRoomNumber());
        }

    if (RoomTab[Msg->GetRoomNumber()].IsAnonymous())
        {
#ifdef WINCIT
        trap(T_ANONYMOUS, WindowCaption, getmsg(558), Msg->GetLocalID());
#else
        trap(T_ANONYMOUS, getmsg(558), Msg->GetLocalID());
#endif
        }

    return (TRUE);
    }
