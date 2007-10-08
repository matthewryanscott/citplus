// --------------------------------------------------------------------------
// Citadel: MsgRead.CPP
//
// Stuff for reading messages

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "auxtab.h"
#include "log.h"
#include "group.h"
#include "net.h"
#include "boolexpr.h"
#include "events.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// printMessage()       prints message on modem and console
// stepMessage()        find the next message in DIR
// showMessages()       is routine to print roomful of msgs
// printheader()        prints current message header
// GetMessageHeader()   reads a message off disk into RAM.

static void argh(void)
{

}

void ShowWhyMayNotSeeMsg(MaySeeMsgType Why, ulong MsgID);

#define SEARCHUPDATETIME    5

void TERMWINDOWMEMBER PrintMessageByID(m_index ID)
    {
    Message *Msg = LoadMessageByID(ID, TRUE, TRUE);
    delete Msg;
    }


// --------------------------------------------------------------------------
// LoadMessageByID(): Loads a message into RAM from MSG.DAT based on its
//  numeric ID.
//
// Input:
//  ulong ID:           The message ID to load.
//  Bool CheckMaySeeIt: TRUE to only load the message if the user is allowed
//                      to see it; FALSE to always load the message.
//  Bool Display:       TRUE to display the message (or error messages if the
//                      message could not be loaded) to the user; FALSE to
//                      not.
//
// Return value:
//  Message *:          A pointer to the message if it could be loaded, NULL
//                      if it couldn't.

Message *TERMWINDOWMEMBER LoadMessageByID(m_index ID, Bool CheckMaySeeIt, Bool Display)
    {
    if (!MessageDat.IsValid(ID))
        {
        if (Display)
            {
            DebugOut(34, ltoac(ID));
            }

        return (NULL);
        }

    if (CheckMaySeeIt)
        {
        const MaySeeMsgType MaySeeIt = CurrentUser->MaySeeMessage(ID);

        if (MaySeeIt != MSM_GOOD)
            {
            if (debug && Display)
                {
                ShowWhyMayNotSeeMsg(MaySeeIt, ID);
                }

            return (NULL);
            }
        }

    Bool referenceFlag;
    ulong referenceID;
    r_slot referenceRoom;
    uchar referenceAttr;

    Bool iter = FALSE;

    do
        {
        if (!MessageDat.IsValid(ID))
            {
            if (Display)
                {
                DebugOut(34, ltoac(ID));
                }

            return (NULL);
            }

        if (!MessageDat.IsInuse(ID))
            {
            if (Display)
                {
                mPrintfCR(getmsg(142), cfg.Umsg_nym);
                }

            return (NULL);
            }

        if (!iter)
            {
            referenceFlag = MessageDat.IsCopy(ID);
            referenceRoom = MessageDat.GetRoomNum(ID);
            referenceID = ID;
            referenceAttr = 0;
            referenceAttr |= (char) (MessageDat.IsReceived(ID) ? ATTR_RECEIVED : 0);
            referenceAttr |= (char) (MessageDat.IsReply(ID) ? ATTR_REPLY : 0);
            referenceAttr |= (char) (MessageDat.IsMadevis(ID) ? ATTR_MADEVIS : 0);
            referenceAttr |= (char) (MessageDat.IsCensored(ID) ? ATTR_CENSORED : 0);
            }

        if (MessageDat.IsCopy(ID))
            {
            m_index CopyOffset = MessageDat.GetCopyOffset(ID);
            assert(CopyOffset);

            if (CopyOffset < ID - MessageDat.OldestMessageInTable())
                {
                if (CopyOffset)
                    {
                    ID -= CopyOffset;
                    }
                else
                    {
                    // Copy of self? Something's wrong... give up.
                    return (NULL);
                    }
                }
            else
                {
                // Copied message has scrolled
                return (NULL);
                }
            }

        iter = TRUE;
        } while (MessageDat.IsCopy(ID));

    ulong LocationInFile = MessageDat.GetLocation(ID);

    if (LocationInFile != ULONG_ERROR)
        {
        Message *Msg = new Message;

        if (Msg)
            {
            ReadMessageStatus Status;

            Status = MessageDat.LoadAll(ID, Msg);

            if (Status != RMS_OK)
                {
                if (Display)
                    {
                    mPrintfCR(getmsg(143));
                    }

                delete Msg;
                return (NULL);
                }

            if (atol(Msg->GetLocalID()) != ID)
                {
                if (Display)
                    {
                    label Byte;
                    CopyStringToBuffer(Byte, ltoac(LocationInFile));
                    mPrintfCR(getmsg(393), cfg.Lmsg_nym, ltoac(ID), Byte);
                    }

                delete Msg;
                return (NULL);
                }

            Msg->SetViewDuplicate(referenceFlag);
            Msg->SetOriginalID(referenceID);
            Msg->SetOriginalAttribute(referenceAttr);
            Msg->SetOriginalRoom(referenceRoom);

            PrepareMsgScriptInit(Msg);

#ifndef WINCIT
            doEvent(EVT_LOADMESSAGE, InitMsgScript);
#endif

            if (!CheckMsgScript(Msg))
                {
                delete Msg;
                return (NULL);
                }

            if (Display)
                {
                PrintMessage(Msg);
                }

            return (Msg);
            }
        else
            {
            if (Display)
                {
                OutOfMemory(25);
                }

            return (NULL);
            }
        }
    else
        {
        if (Display)
            {
            label Byte;
            CopyStringToBuffer(Byte, ltoac(LocationInFile));

            DebugOut(35, Byte, ltoac(ID));
            }

        return (NULL);
        }
    }


Bool LogExtensions::IsKill(KillFileE Type, const char *String) const
    {
    assert(this);
    GAINEXCLUSIVEACCESS();
    const strList *theStr;

    for (theStr = Kill[Type]; theStr; theStr = (strList *) getNextLL(theStr))
        {
        if ((Type == KF_TEXT) ? IsSubstr(String, theStr->string) : SameString(theStr->string, String))
            {
            RELEASEEXCLUSIVEACCESS();
            return (TRUE);
            }
        }

    RELEASEEXCLUSIVEACCESS();
    return (FALSE);
    }


// for lack of a better place and function name, not prototyped above
static Bool FindTextInComments(const Message *Msg, const char *SearchText)
    {
    const long NumCmt = Msg->GetCommentCount();

    for (long Idx = 1; Idx <= NumCmt; Idx++)
        {
        if (IsSubstr(Msg->GetComment(Idx), SearchText))
            {
            return (TRUE);
            }
        }
    return (FALSE);
    }


// --------------------------------------------------------------------------
// ActuallyDisplayMessage()
//
// Notes:
//  Assumes MSG_READMSG is locked.

void TERMWINDOWMEMBER ActuallyDisplayMessage(Message *Msg, Bool ShowHeader, Bool ShowBody, Bool ShowSignature)
    {
    if (ShowHeader)
        {
        printheader(Msg);
        }

    if (ShowBody)
        {
        if (Msg->GetFileLink()[0])
            {
            dumpf(Msg->GetFileLink(), 1, FALSE);
            }
        else
            {
            mFormat(Msg->GetText());
            ResetOutputControl();

            doCR();
            }

        if ((Msg->GetSignature()[0] || Msg->GetUserSignature()[0]) &&
                ShowSignature)
            {
            termCap(TERM_NORMAL);
            termCap(TERM_BOLD);

            CRmPrintf(getmsg(279));

            termCap(TERM_NORMAL);

            if (*Msg->GetUserSignature())
                {
                CRmPrintf(pcts, Msg->GetUserSignature());
                }

            if (*Msg->GetSignature())
                {
                CRmPrintf(pcts, Msg->GetSignature());
                }

            doCR();
            }
        }
    }

// --------------------------------------------------------------------------
// PrintMessage(): Displays message on modem and console
//
// Input:
//  Message *Msg: Message to display

void TERMWINDOWMEMBER PrintMessage(Message *Msg)
    {
    if (!LockMessages(MSG_READMSG))
        {
        OutOfMemory(105);
        return;
        }

    Bool HeaderPrinted = FALSE;

    if (CurrentUser->CanKillMessage(Msg, loggedIn))
        {
        OC.User.SetCanK(TRUE);
        }

    if (CurrentUser->IsAide())
        {
        OC.User.SetCanM(TRUE);
        OC.User.SetCanStar(TRUE);
        }

    if (loggedIn)
        {
        OC.User.SetCanAt(TRUE);
        }

    OC.User.SetCanBang(TRUE);
    OC.User.SetCanR(TRUE);

    MRO.DotoMessage = NO_SPECIAL;

    const MaySeeMsgType MaySeeIt = CurrentUser->MaySeeMessage(Msg);

    if (MaySeeIt != MSM_GOOD)
        {
        if (debug)
            {
            ShowWhyMayNotSeeMsg(MaySeeIt, atol(Msg->GetLocalID()));
            }

        OC.User.ResetCanFlags();
        UnlockMessages(MSG_READMSG);
        return;
        }

    // SearchUser From Acit
    if (rf.User)
        {
        if (    !u_match(deansi(Msg->GetToUser()),              rf.SearchUser) &&
                !u_match(deansi(Msg->GetSubject()),             rf.SearchUser) &&
                !u_match(deansi(Msg->GetOriginSoftware()),      rf.SearchUser) &&
                !u_match(deansi(Msg->GetAuthor()),              rf.SearchUser) &&
                !u_match(deansi(Msg->GetForward()),             rf.SearchUser) &&
                !u_match(deansi(Msg->GetOriginNodeName()),      rf.SearchUser) &&
                !u_match(deansi(Msg->GetOriginRegion()),        rf.SearchUser) &&
                !u_match(deansi(Msg->GetOriginCountry()),       rf.SearchUser) &&
                !u_match(deansi(Msg->GetToNodeName()),          rf.SearchUser) &&
                !u_match(deansi(Msg->GetToRegion()),            rf.SearchUser) &&
                !u_match(deansi(Msg->GetOriginPhoneNumber()),   rf.SearchUser) &&
                !u_match(deansi(Msg->GetToPhoneNumber()),       rf.SearchUser) &&
                !u_match(deansi(Msg->GetToCountry()),           rf.SearchUser))
            {
            OC.User.ResetCanFlags();
            UnlockMessages(MSG_READMSG);
            return;
            }
        }

    if (rf.Group)
        {
        GNFNT = Msg->GetGroup();

        if (!EvaluateBooleanExpression(rf.SearchGroup, GroupNameTester))
            {
            OC.User.ResetCanFlags();
            UnlockMessages(MSG_READMSG);
            return;
            }
        }

    if (MRO.Date)
        {
        if (MRO.DatePrevious)
            {
            if (atol(Msg->GetCreationTime()) >= MRO.CheckDate)
                {
                OC.User.ResetCanFlags();
                UnlockMessages(MSG_READMSG);
                return;
                }
            }
        else
            {
            if (atol(Msg->GetCreationTime()) < MRO.CheckDate)
                {
                OC.User.ResetCanFlags();
                UnlockMessages(MSG_READMSG);
                return;
                }
            }
        }

    if (CurrentUser->IsKill(KF_NODE, Msg->GetOriginNodeName()) || CurrentUser->IsKill(KF_REGION, Msg->GetOriginRegion()) ||
            CurrentUser->IsKill(KF_USER, Msg->GetAuthor()) ||
            (Msg->IsEncrypted() && CurrentUser->IsExcludeEncryptedMessages()))
        {
        if (!CurrentUser->IsHideMessageExclusions())
            {
            CRmPrintfCR(getmsg(MSG_READMSG, 16), *Msg->GetToUser() ? getmsg(MSG_READMSG, 17) : ns,
                    *Msg->GetToUser() ? cfg.Lmsg_nym : cfg.Umsg_nym);

            if (!*Msg->GetToUser())
                {
                displayedmessage = TRUE;    // a substituted msg, anyway
                }
            }

        OC.User.ResetCanFlags();
        UnlockMessages(MSG_READMSG);
        return;
        }

    Msg->Decompress();

    if (Msg->IsEncrypted() && OC.User.GetOutFlag() == OUTOK)
        {
        // need to get key
        printheader(Msg);
        HeaderPrinted = TRUE;

        mPrintfCR(getmsg(MSG_READMSG, 18));

        label Key;

        GetStringWithBlurb(getmsg(MSG_READMSG, 19), Key, LABELSIZE, Msg->GetEncryptionKey(), B_ENCRYPT);
        normalizeString(Key);

        OC.SetOutFlag(OUTOK);

        if (!*Key)
            {
            OC.User.ResetCanFlags();
            UnlockMessages(MSG_READMSG);
            return;
            }

        Msg->SetEncryptionKey(Key);

        doCR();
        if (decrypt(Msg->GetTextPointer(), Key))
            {
            Msg->SetEncrypted(FALSE);
            }
        }

    if (rf.Text)
        {
        const char *T = rf.SearchText;

        if (!
                (
                    FindTextInComments(Msg,             T) ||
                    IsSubstr(Msg->GetAuthor(),          T) ||
                    IsSubstr(Msg->GetTitle(),           T) ||
                    IsSubstr(Msg->GetOriginCountry(),   T) ||
                    IsSubstr(Msg->GetToCountry(),       T) ||
                    IsSubstr(Msg->GetForward(),         T) ||
                    IsSubstr(Msg->GetGroup(),           T) ||
                    IsSubstr(Msg->GetOriginNodeName(),  T) ||
                    IsSubstr(Msg->GetOriginRegion(),    T) ||
                    IsSubstr(Msg->GetCreationRoom(),    T) ||
                    IsSubstr(Msg->GetToUser(),          T) ||
                    IsSubstr(Msg->GetSurname(),         T) ||
                    IsSubstr(Msg->GetToNodeName(),      T) ||
                    IsSubstr(Msg->GetToRegion(),        T) ||
                    IsSubstr(Msg->GetUserSignature(),   T) ||
                    IsSubstr(Msg->GetSubject(),         T) ||
                    IsSubstr(Msg->GetSignature(),       T) ||
                    IsSubstr(Msg->GetText(),            T) ||

                    (MRO.Verbose &&
                        (
                        IsSubstr(Msg->GetFromPath(),        T) ||
                        IsSubstr(Msg->GetToPath(),          T) ||
                        IsSubstr(Msg->GetOriginSoftware(),  T) ||
                        IsSubstr(Msg->GetSourceID(),        T) ||
                        IsSubstr(Msg->GetTwitRegion(),      T) ||
                        IsSubstr(Msg->GetTwitCountry(),     T) ||
                        IsSubstr(Msg->GetOriginPhoneNumber(),T) ||
                        IsSubstr(Msg->GetToPhoneNumber(),   T)
                        )
                    )
                )
            )
            {
            OC.User.ResetCanFlags();
            UnlockMessages(MSG_READMSG);
            return;
            }
        }

    if (CurrentUser->IsKill(KF_TEXT, Msg->GetText()))
        {
        if (!CurrentUser->IsHideMessageExclusions())
            {
            CRmPrintfCR(getmsg(MSG_READMSG, 16), *Msg->GetToUser() ? getmsg(MSG_READMSG, 17) : ns,
                    *Msg->GetToUser() ? cfg.Lmsg_nym : cfg.Umsg_nym);

            if (!*Msg->GetToUser())
                {
                displayedmessage = TRUE;    // a substituted msg, anyway
                }
            }

        OC.User.ResetCanFlags();
        UnlockMessages(MSG_READMSG);
        return;
        }

    if (!OC.UseMCI && substr(Msg->GetText(), getmsg(MSG_READMSG, 10), FALSE) != CERROR)
        {
        CRmPrintfCR(getmsg(MSG_READMSG, 21), cfg.Umsg_nym);
        OC.User.ResetCanFlags();
        UnlockMessages(MSG_READMSG);
        return;
        }

    MS.Read++;  // Increment # messages read
    doEvent(EVT_READMSG);

    if (CurrentUser->IsClearScreenBetweenMessages())
        {
        numLines = 0;
        cls(SCROLL_SAVE);
        TermCap->SendClearScreen();
        }

    displayedmessage = TRUE;

    ActuallyDisplayMessage(Msg, !HeaderPrinted, !MRO.Headerscan, CurrentUser->IsViewSignatures());

    if (!MRO.Headerscan)
        {
        seen = TRUE;
        }

    OC.User.ResetCanFlags();
    termCap(TERM_NORMAL);
    OC.Echo = BOTH;
    OC.setio();

    if (StatusLine.IsFullScreen())
        {
        StatusLine.Update(WC_TWp);
        }

    if (*Msg->GetLinkedApplication())
        {
        RunApplication(Msg->GetLinkedApplication(), NULL, TRUE, TRUE);
        }

    UnlockMessages(MSG_READMSG);
    }


void TERMWINDOWMEMBER AdjustDownForRoom(m_index *Number)
    {
    m_index Oldest = MessageDat.OldestMessageInTable();
    m_index Newest = MessageDat.NewestMessage();

    if (*Number < Oldest)
        {
        *Number = Oldest;
        }
    if (*Number > Newest)
        {
        *Number = Newest;
        }

    if (!MRO.All)
        {
        while (*Number >= Oldest && MessageDat.GetRoomNum(*Number) != thisRoom)
            {
            (*Number)--;
            }
        }
    }

void TERMWINDOWMEMBER AdjustUpForRoom(m_index *Number)
    {
    m_index Oldest = MessageDat.OldestMessageInTable();
    m_index Newest = MessageDat.NewestMessage();

    if (*Number < Oldest)
        {
        *Number = Oldest;
        }
    if (*Number > Newest)
        {
        *Number = Newest;
        }

    if (!MRO.All)
        {
        while (*Number <= Newest && MessageDat.GetRoomNum(*Number) != thisRoom)
            {
            (*Number)++;
            }
        }
    }


#if !defined(AUXMEM) && !defined(WINCIT)
inline
#endif
void TERMWINDOWMEMBER DoTheStep(int Direction, m_index *ID)
    {
    #if defined(AUXMEM) || defined(WINCIT)
        if (1 == Direction)
            {
            if (MRO.All)
                {
                ++(*ID);
                }
            else
                {
                if (*ID == M_INDEX_ERROR)
                    {
                    *ID = MessageDat.GetFirstMessageInRoom(thisRoom);
                    }
                else
                    {
                    *ID = MessageDat.GetNextRoomMsg(*ID);
                    }
                }
            }
        else
            {
            if (MRO.All)
                {
                --(*ID);
                }
            else
                {
                if (*ID == M_INDEX_ERROR)
                    {
                    *ID = MessageDat.GetLastMessageInRoom(thisRoom);
                    }
                else
                    {
                    *ID = MessageDat.GetPrevRoomMsg(*ID);
                    }
                }
            }
    #else
        if (1 == Direction)
            {
            ++(*ID);
            }
        else
            {
            --(*ID);
            }
    #endif
    }

Bool TERMWINDOWMEMBER ShouldShowMessage(m_index ID)
    {
    return (    // It's a good message...
                ID != M_INDEX_ERROR

                // and not reading by mail, or it is mail...
                && (!rf.Mail || MessageDat.IsMail(ID) || MessageDat.IsMassemail(ID))

                // and not reading by group, or it is group...
                && (!rf.Group || MessageDat.IsLimited(ID) || MessageDat.IsMassemail(ID))

                // and not reading public, or it is public...
                && (!rf.Public || (!MessageDat.IsLimited(ID) &&!MessageDat.IsMail(ID) && !MessageDat.IsMassemail(ID)))

                // and not reading local, or it is local.
                && (!rf.Local || !MessageDat.IsNetworked(ID)));
    }

// --------------------------------------------------------------------------
// stepMessage(): Find the next message in Direction.
//
// Input:
//  ulong *at: Message to start at
//  int Direction: 1 for forward; -1 for backwards
//
// Output:
//  ulong *at: Where we ended up
//
// Return value:
//  TRUE: We were able to find another message
//  FALSE: We were not able to find another message

Bool TERMWINDOWMEMBER stepMessage(m_index *at, int Direction)
    {
#if defined(AUXMEM) || defined(WINCIT)
    if (!MRO.All && *at == M_INDEX_ERROR)
        {
        if (1 == Direction)
            {
            *at = MessageDat.GetFirstMessageInRoom(thisRoom);
            }
        else
            {
            *at = MessageDat.GetLastMessageInRoom(thisRoom);
            }

        return (*at != M_INDEX_ERROR);
        }
#endif

    if (*at < MessageDat.OldestMessageInTable())
        {
        if (1 == Direction)
            {
            *at = MessageDat.OldestMessageInTable();
            }
        else
            {
            return (FALSE);
            }
        }

    for (DoTheStep(Direction, at); MessageDat.IsValid(*at); DoTheStep(Direction, at))
        {
        #ifdef REGULAR
            // skip messages not in this room: already done in DoTheStep()
            // for Windows and Auxmem versions.

            if (!MRO.All && MessageDat.GetRoomNum(at) != thisRoom)
                {
                continue;
                }
        #endif

        if (!ShouldShowMessage(*at))
            {
            continue;
            }

        const MaySeeMsgType MaySeeIt = CurrentUser->MaySeeMessage(*at);

        if (MaySeeIt == MSM_GOOD)
            {
            return (TRUE);
            }
        else
            {
            if (debug)
                {
                ShowWhyMayNotSeeMsg(MaySeeIt, *at);
                }
            }
        }

    return (FALSE);
    }

void TERMWINDOWMEMBER SuspendJournalFile(FILE **save)
    {
    if (journalfl)
        {
        *save = journalfl;
        journalfl = NULL;
        }
    else
        {
        *save = NULL;
        }
    }

void TERMWINDOWMEMBER RestoreJournalFile(FILE *saved)
    {
    journalfl = saved;
    }


// --------------------------------------------------------------------------
// SetMessageReadingOptions()
//
// Notes:
//  Assumes MSG_READMSG is locked.

Bool TERMWINDOWMEMBER SetMessageReadingOptions(void)
    {
    // Get message number from user.
    if (MRO.Number)
        {
        Bool good;
        FILE *tmp;
        char prompt[128];

        SuspendJournalFile(&tmp);

        do
            {
            good = TRUE;

            label Oldest;
            CopyStringToBuffer(Oldest, ltoac(MessageDat.OldestMessageInFile()));

            CRmPrintfCR(getmsg(392), Oldest, ltoac(MessageDat.NewestMessage()));

            sprintf(prompt, getmsg(MSG_READMSG, 9), cfg.Lmsg_nym);
            MRO.MessageNumber = getNumber(prompt, 0, LONG_MAX, -1, FALSE, NULL);

            if (MRO.MessageNumber < 0)
                {
                MRO.MessageNumber = 0;
                }

            if (!MRO.MessageNumber)
                {
                MRO.Number = FALSE;
                }
            else
                {
                if (!MessageDat.IsValid(MRO.MessageNumber))
                    {
                    mPrintfCR(getmsg(391));
                    good = FALSE;
                    }
                }
            } while (!good);

        if (tmp)
            {
            mPrintf(getmsg(MSG_READMSG, 10), cfg.Lmsgs_nym);
            RestoreJournalFile(tmp);
            }
        }

    // Get message date from user.
    if (MRO.Date)
        {
        char prompt[128];
        label usrIn;
        datestruct ds;
        struct date m_date;
        struct time m_time;
        label tmp;
        Bool first = TRUE;

        do
            {
            if (!HaveConnectionToUser())
                {
                return (FALSE);
                }

            if (!first)
                {
                mPrintfCR(getmsg(156));
                }

            first = FALSE;

            if (!GetStringWithBlurb(getmsg(MSG_READMSG, 11), usrIn, LABELSIZE, ns, B_DATESET))
                {
                return (FALSE);
                }

            gdate(usrIn, &ds);
            } while (!ds.Date);

        m_date.da_year = ds.Year + 1900;        // i hate dostounix
        m_date.da_mon = (char) (ds.Month + 1);  // i hate dostounix
        m_date.da_day = (char) ds.Date;

        m_time.ti_min = 0;
        m_time.ti_hour = 0;
        m_time.ti_sec = 0;
        m_time.ti_hund = 0;

        MRO.CheckDate = dostounix(&m_date, &m_time);
        strftime(tmp, LABELSIZE, getmsg(435), MRO.CheckDate);

        sprintf(prompt, getmsg(MSG_READMSG, 12), cfg.Lmsgs_nym, tmp);

        int Res = GetOneKey(prompt, getmsg(MSG_READMSG, 13), getmsg(MSG_READMSG, 13)[0], B_BA, getmsg(MSG_READMSG, 14),
                getmsg(MSG_READMSG, 15));

        if (Res == CERROR)
            {
            return (FALSE);
            }

        MRO.DatePrevious = !Res;
        }

    return (ProcessReadFilter(&rf, FALSE));
    }

Bool TERMWINDOWMEMBER ShouldAskToRelease(const Message *Msg)
    {
    return (// Problem/Moderated message
            *Msg->GetX()

            // And we saw it
            && seen

            // And it hasn't already been released
            && !Msg->IsMadeVisible()

            // And this message is not in Aide) or Dump>
            && Msg->GetOriginalRoom() != AIDEROOM && Msg->GetOriginalRoom() != DUMP

            // And we haven't stopped or nexted the output
            && OC.User.CanOutput()

            // And we can moderate in this message's room
            && CurrentUser->CanModerateRoom(Msg->GetOriginalRoom())
            );
    }


// --------------------------------------------------------------------------
// IsRecipientOfMail()

Bool TERMWINDOWMEMBER IsRecipientOfMail(const Message *Msg)
    {
    label CUName;
    CurrentUser->GetName(CUName, sizeof(CUName));

    return (// Logged in
            loggedIn &&

            // And it is mail
            *Msg->GetToUser() &&

            // And it is not mail bound for elsewhere
            !(Msg->IsNetworkedMail() && !Msg->IsNetworkedMailForHere()) &&

            // And it is to us, because of one of the following...
                (
                // mail to us
                SameString(Msg->GetToUser(), CUName) ||

                // mail to Aide and we are aide
                (SameString(Msg->GetToUser(), getmsg(385)) && CurrentUser->IsAide()) ||

                // mail to Sysop and we are sysop
                (SameString(Msg->GetToUser(), getmsg(386)) && CurrentUser->IsSysop()) ||

                // forwarded to us
                SameString(Msg->GetForward(), CUName) ||

                // mass e-mail
                Msg->IsMassEMail()
                )
            );
    }


// --------------------------------------------------------------------------
// ShouldAskToReply()

Bool TERMWINDOWMEMBER ShouldAskToReply(const Message *Msg, OldNewPick whichMess)
    {
    return (!MRO.Headerscan &&

                (   // We asked to reply to it
                    (MRO.DotoMessage == REPLY) ||

                    (
                    // Only when reading new
                    (whichMess == NewOnly) &&

                    // and we are the recipient
                    IsRecipientOfMail(Msg)
                    )
                )

            && !Msg->IsAnonymous());
    }


// --------------------------------------------------------------------------
// showMessages(): Print roomful of msgs
//
// Input:
//  OldNewPick whichMess: Which ones to read
//  Bool revOrder: TRUE to read them backwards; FALSE for forward
//  long howMany: How many to read. (0 for all?)

#if defined(AUXMEM) || defined(WINCIT)
    #define GetNewest()     (MRO.All ? MessageDat.NewestMessage() : MessageDat.GetLastMessageInRoom(thisRoom))
    #define GetOldest()     (MRO.All ? MessageDat.OldestMessageInTable() : MessageDat.GetFirstMessageInRoom(thisRoom))
#else
    #define GetNewest()     (MessageDat.NewestMessage())
    #define GetOldest()     (MessageDat.OldestMessageInTable())
#endif

void TERMWINDOWMEMBER showMessages(OldNewPick whichMess, Bool revOrder, long howMany)
    {
argh();
    if (!LockMessages(MSG_READMSG))
        {
        OutOfMemory(26);
        return;
        }

    m_index lowLimit;           // The oldest message to display
    m_index highLimit;          // The newest message to display
    m_index msgNo;              // The message being displayed

    int     increment;          // +1 or -1: how to get to next msg
    Bool    done;               // Done with all messages.

    Bool    *roomCheck = NULL;  // For .Read All...

    if (!SetMessageReadingOptions())
        {
        UnlockMessages(MSG_READMSG);
        return;
        }

    OC.SetOutFlag(OUTOK);

    ShowHelpMessage(577);

    #if defined(AUXMEM) || defined(WINCIT)
        if (!MRO.All && MessageDat.GetFirstMessageInRoom(thisRoom) == M_INDEX_ERROR)
            {
            UnlockMessages(MSG_READMSG);
            return;
            }
    #endif

    // Set the highLimit and lowLimit.
    switch (whichMess)
        {
        case NewOnly:
            {
            highLimit = GetNewest();

            if (loggedIn)
                {
                lowLimit = CurrentUser->GetRoomNewPointer(thisRoom) + 1;
                if (lowLimit < GetOldest())
                    {
                    lowLimit = GetOldest();
                    }
                }
            else
                {
                lowLimit = GetOldest();
                }

            AdjustDownForRoom(&highLimit);
            AdjustUpForRoom(&lowLimit);

            // Go back one more if Last old on New is turned on.
            if (!revOrder && CurrentUser->IsOldToo() && (highLimit >= lowLimit))
                {
                m_index oldlowLimit = lowLimit;
                if (!stepMessage(&lowLimit, -1))
                    {
                        lowLimit = oldlowLimit;
                    }
                }

            // Adjust lowLimit if specifying how many to read.
            if (howMany)
                {
                m_index hmll = GetNewest();

                AdjustDownForRoom(&hmll);

                for (long l = 1; l < howMany; l++)
                    {
                    if (!stepMessage(&hmll, -1))
                        {
                        break;
                        }
                    }

                // Don't read any old, or more than howMany says to read.
                lowLimit = max((long) lowLimit, (long) hmll);
                }

            break;
            }

        case OldAndNew:
            {
            highLimit = GetNewest();

            AdjustDownForRoom(&highLimit);

            if (howMany)
                {
                lowLimit = GetNewest();

                AdjustUpForRoom(&lowLimit);

                // Go back howMany messages.
                for (long l = 1; l < howMany; l++)
                    {
                    if (!stepMessage(&lowLimit, -1))
                        {
                        break;
                        }
                    }
                }
            else
                {
                lowLimit = GetOldest();

                AdjustUpForRoom(&lowLimit);
                }

            break;
            }

        case OldOnly:
            {
            lowLimit = GetOldest();

            AdjustUpForRoom(&lowLimit);

            if (loggedIn)
                {
                highLimit = CurrentUser->GetRoomNewPointer(thisRoom);

                if (highLimit > GetNewest())
                    {
                    highLimit = GetNewest();
                    }
                }
            else
                {
                highLimit = GetNewest();
                }

            AdjustDownForRoom(&highLimit);

            if (howMany)
                {
                m_index hmll = highLimit;

                AdjustDownForRoom(&hmll);

                for (long l = 1; l < howMany; l++)
                    {
                    if (!stepMessage(&hmll, -1))
                        {
                        break;
                        }
                    }

                lowLimit = max((long) lowLimit, (long) hmll);
                }

            break;
            }
        }

    // Allow for reverse retrieval
    if (revOrder)
        {
        msgNo = MRO.Number ? MRO.MessageNumber : highLimit;
        increment = -1;

        if (MRO.Number)
            {
            AdjustDownForRoom(&msgNo);
            }
        }
    else
        {
        msgNo = MRO.Number ? MRO.MessageNumber : lowLimit;
        increment = 1;

        if (MRO.Number)
            {
            AdjustUpForRoom(&msgNo);
            }
        }

    if (MRO.All)
        {
        roomCheck = new Bool[cfg.maxrooms];

        if (roomCheck)
            {
            theOtherAlgorithm(roomCheck, thisHall, FALSE);
            }
        else
            {
            OutOfMemory(26);
            UnlockMessages(MSG_READMSG);
            return;
            }
        }

    doEvent(EVT_READMSGS);

    time_t LastMessageReadTime = time(NULL);

    Bool pausemsg;          // Should we pause after message?

    for (   done = ShouldShowMessage(msgNo) ? FALSE : !stepMessage(&msgNo, increment);

            !done && msgNo >= lowLimit && msgNo <= highLimit && HaveConnectionToUser();

            done = !stepMessage(&msgNo, increment))
        {
        // all this trouble just to find out if this is going to be the last
        // message we're displaying
        if (CurrentUser->IsPauseBetweenMessages() || MRO.PauseBetween)
            {
            m_index dummymsgno = msgNo;

            const Bool dummydone = !stepMessage(&dummymsgno, increment);

            if (!dummydone && dummymsgno >= lowLimit && dummymsgno <= highLimit && HaveConnectionToUser())
                {
                pausemsg = TRUE;
                }
            }

        // Allow interruptions
        if (BBSCharReady())
            {
#ifdef MULTI
            OC.CheckInput(FALSE, this);
#else
            OC.CheckInput(FALSE);
#endif
            }

        CheckTimeOnSystem();

        if (OC.User.GetOutFlag() != OUTOK)
            {
            if (OC.User.GetOutFlag() == OUTNEXT ||
                    OC.User.GetOutFlag() == OUTPARAGRAPH)
                {
                OC.SetOutFlag(OUTOK);
                }
            else if (OC.User.GetOutFlag() == OUTSKIP)
                {
                OC.Echo = BOTH;
                OC.setio();

                resetMsgfilter(FALSE);

                delete [] roomCheck;
                UnlockMessages(MSG_READMSG);
                return;
                }
            }

        if (roomCheck && !roomCheck[MessageDat.GetRoomNum(msgNo)])
            {
            continue;
            }

        seen = FALSE;

        SetDoWhat(READMESSAGE);
        displayedmessage = FALSE;

        Message *Msg = LoadMessageByID(msgNo, TRUE, TRUE);

        SetDoWhat(DUNO);

        if (displayedmessage)
            {
            LastMessageReadTime = time(NULL);
            }
        else if (Msg && (LastMessageReadTime + SEARCHUPDATETIME <= time(NULL)))
            {
            char DateStr[128];

            char Buffer[64];
            strftime(DateStr, sizeof(DateStr) - 1, loggedIn ? CurrentUser->GetVerboseDateStamp(Buffer, sizeof(Buffer)) :
                    cfg.vdatestamp, atol(Msg->GetCreationTime()));

            mPrintfCR(getmsg(MSG_READMSG, 1), cfg.Lmsg_nym, ltoac(atol(Msg->GetLocalID())), DateStr);

            LastMessageReadTime = time(NULL);
            }

        if ((OC.User.GetOutFlag() != OUTSKIP) && displayedmessage && Msg)
            {
            switch (MS.AutoMKC)
                {
                case AM_MARK:   MRO.DotoMessage = MARK_IT;      break;
                case AM_KILL:   MRO.DotoMessage = PULL_IT;      break;
                case AM_CENSOR: MRO.DotoMessage = CENSOR_IT;    break;
                }

            if (OC.User.GetOutFlag() == OUTOK && (CurrentUser->IsPauseBetweenMessages() || MRO.PauseBetween) &&
                    pausemsg && (!OC.justdidpause || !CurrentUser->GetLinesPerScreen()) &&

                    // Won't be asked to pull, mark, or censor
                    (MRO.DotoMessage != PULL_IT) && (MRO.DotoMessage != MARK_IT) && (MRO.DotoMessage != CENSOR_IT) &&

                    // Won't be asked to release
                    !ShouldAskToRelease(Msg) &&

                    // Won't be asked to respond
                    !ShouldAskToReply(Msg, whichMess))
                {

// taken from printmessage()
                if (CurrentUser->CanKillMessage(Msg, loggedIn))
                    {
                    OC.User.SetCanK(TRUE);
                    }

                if (CurrentUser->IsAide())
                    {
                    OC.User.SetCanM(TRUE);
                    OC.User.SetCanStar(TRUE);
                    }

                if (loggedIn)
                    {
                    OC.User.SetCanAt(TRUE);
                    }

                OC.User.SetCanBang(TRUE);
                OC.User.SetCanR(TRUE);

/////////////////////////////////////////////////////////////////////////////////////////
#ifdef MULTI
                OC.CheckInput(TRUE, this);
#else
                OC.CheckInput(TRUE);
#endif
                OC.User.ResetCanFlags();
                OC.justdidpause = TRUE;
                numLines = 0;
                pausemsg = FALSE;
                }

            if (journalfl)
                {
                FILE *tmp;

                SuspendJournalFile(&tmp);
                mPrintf(pctss, ltoac(msgNo));
                RestoreJournalFile(tmp);
                }
            else
                {
                switch (MRO.DotoMessage)
                    {
                    case PULL_IT:
                        {
                        #if defined(AUXMEM) || defined(WINCIT)
                            // To keep from reading Dump> after the kill
                            m_index TmpMsgNo;

                            if (!MRO.All)
                                {
                                if (1 == increment)
                                    {
                                    TmpMsgNo = MessageDat.GetPrevRoomMsg(msgNo);
                                    }
                                else
                                    {
                                    TmpMsgNo = MessageDat.GetNextRoomMsg(msgNo);
                                    }
                                }
                        #endif

                        if (pullIt(Msg))
                            {
                            #if defined(AUXMEM) || defined(WINCIT)
                                if (!MRO.All)
                                    {
                                    msgNo = TmpMsgNo;
                                    }
                            #endif
                            }

                        break;
                        }

                    case MARK_IT:
                        {
                        markIt(Msg);
                        break;
                        }

                    case CENSOR_IT:
                        {
                        censorIt(Msg);
                        break;
                        }

                    case REVERSE_READ:
                        {
                        OC.SetOutFlag(OUTOK);

                        increment = -increment;
                        CRmPrintfCR(getmsg(357), (increment == 1) ? '+' : '-');
                        lowLimit = MessageDat.OldestMessageInTable();
                        highLimit= MessageDat.NewestMessage();  // reevaluate for Livia
                        break;
                        }

                    default:
                        {
                        if (OC.User.GetOutFlag() == OUTNEXT)
                            {
                            break;
                            }

                        if (ShouldAskToRelease(Msg))
                            {
                            // Release?
                            int Res = GetOneKey(getmsg(MSG_READMSG, 2), getmsg(MSG_READMSG, 3), getmsg(MSG_READMSG, 3)[1],
                                    B_YNK, getmsg(521), getmsg(522), getmsg(MSG_READMSG, 4));

                            if (Res == 0)   // Yes
                                {
                                markmsg(Msg);
                                OC.SetOutFlag(OUTOK);
                                }
                            else if (Res == 2)  // Kill
                                {
                                #if defined(AUXMEM) || defined(WINCIT)
                                    // To keep from reading Dump> after the kill
                                    m_index TmpMsgNo;

                                    if (!MRO.All)
                                        {
                                        if (1 == increment)
                                            {
                                            TmpMsgNo = MessageDat.GetPrevRoomMsg(msgNo);
                                            }
                                        else
                                            {
                                            TmpMsgNo = MessageDat.GetNextRoomMsg(msgNo);
                                            }
                                        }
                                #endif

                                if (pullIt(Msg))
                                    {
                                    #if defined(AUXMEM) || defined(WINCIT)
                                        if (!MRO.All)
                                            {
                                            msgNo = TmpMsgNo;
                                            }
                                    #endif
                                    }
                                }
                            }

                        // reply to mail
                        if (ShouldAskToReply(Msg, whichMess))
                            {
                            OC.SetOutFlag(OUTOK);

                            doCR();

                            if (getYesNo(getmsg(MSG_READMSG, 5), 1))
                                {
                                Message *NewMsg = new Message;

                                if (NewMsg)
                                    {
                                    // First, check for mailing list
                                    long Idx, Num = Msg->GetCommentCount();
                                    const char *Cmt;

                                    for (Idx = 1; Idx <= Num; Idx++)
                                        {
                                        Cmt = Msg->GetComment(Idx);

                                        if (strncmpi(Cmt, getmsg(MSG_READMSG, 6),
                                                strlen(getmsg(MSG_READMSG, 6))) == SAMESTRING)
                                            {
                                            break;
                                            }
                                        }

                                    if (Idx <= Num)
                                        {
                                        CRmPrintf(getmsg(MSG_READMSG, 7));
                                        const int i = toupper(iCharNE());

                                        if (i == 'M')
                                            {
                                            char listname[LABELSIZE+LABELSIZE];
                                            strcpy(listname, Cmt + strlen(getmsg(MSG_READMSG, 6)));
                                            char *ptr = strrchr(listname, '@');
                                            if (ptr)
                                                {
                                                ptr[-1] = '\0';     // strip " @ nodename"
                                                }

                                            Msg->SetAuthor(listname);   // hack
                                            mPrintfCR(getmsg(MSG_READMSG, 8));
                                            }
                                        else if (i == 'U' || i == 'M' || i == CR || i == LF || i == 'Y')
                                            {
                                            mPrintfCR(pcts, cfg.Uuser_nym);
                                            }
                                        else
                                            {
                                            mPrintfCR(getmsg(653));
                                            continue;
                                            }
                                        }

                                    if (!(SameString(Msg->GetToUser(), getmsg(385)) ||
                                            SameString(Msg->GetToUser(), getmsg(386))))
                                        {
                                        if (OC.whichIO != CONSOLE)
                                            {
                                            OC.Echo = CALLER;
                                            OC.setio();
                                            }
                                        }

                                    if (makeMessage(NewMsg, Msg, Msg->GetRoomNumber()))
                                        {
                                        if (*Msg->GetToUser())
                                            {
                                            Msg->SetOriginalAttribute((char) (Msg->GetOriginalAttribute() | ATTR_REPLY));
                                            MessageDat.ChangeAttribute(Msg->GetOriginalID(), Msg->GetOriginalAttribute());
                                            }
                                        }

                                    // Restore privacy zapped by make...
                                    if (OC.whichIO != CONSOLE)
                                        {
                                        OC.Echo = BOTH;
                                        }

                                    OC.SetOutFlag(OUTOK);

                                    // If the mail scrolled messages off
                                    if (MessageDat.OldestMessageInTable() > lowLimit)
                                        {
                                        lowLimit = MessageDat.OldestMessageInTable();

                                        if (msgNo < lowLimit)
                                            {
                                            msgNo = lowLimit;
                                            }
                                        }

                                    delete NewMsg;
                                    }
                                else
                                    {
                                    OutOfMemory(26);
                                    }
                                }
                            }

                        break;
                        }
                    }
                }
            }

        delete Msg;
        }

    OC.Echo = BOTH;
    OC.setio();

    resetMsgfilter(FALSE);
    delete [] roomCheck;
    UnlockMessages(MSG_READMSG);
    }


// --------------------------------------------------------------------------
// LogExtensions::ShowTagsForUser()
//
// Notes:
//  Assumes MSG_READMSG is locked.

void TERMWINDOWMEMBER ShowTagsForUser(const char *User)
    {
    for (ulong Index = 1; Index <= CurrentUser->GetTagUserCount(User); Index++)
        {
        char Buffer[64];
        if (CurrentUser->GetUserTag(Index, User, Buffer, sizeof(Buffer)))
            {
            mPrintf(getmsg(MSG_READMSG, 22), Buffer);
            }
        }
    }

// --------------------------------------------------------------------------
// printheader(): Prints current message header.
//
// Notes:
//  Assumes MSG_READMSG is locked.

void TERMWINDOWMEMBER printheader(Message *Msg)
    {
    char dtstr[80];
    int madeRec = FALSE;

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        return;
        }

    if (OC.User.GetOutFlag() == OUTNEXT)
        {
        OC.SetOutFlag(OUTOK);
        }

    if (MRO.Verbose)
        {
        // print any unknowns...
        for (unkLst *lul = Msg->GetFirstUnknown(); lul; lul = (unkLst *) getNextLL(lul))
            {
            DebugOut(36, lul->whatField, lul->theValue);
            }
        }

    if (*Msg->GetCreationTime())
        {
        const long timestamp = atol(Msg->GetCreationTime());

        if (MRO.Verbose)
            {
            char Buffer[80];
            strftime(dtstr, 79, (loggedIn) ?
                    CurrentUser->GetVerboseDateStamp(Buffer, sizeof(Buffer)) : cfg.vdatestamp, timestamp);
            }
        else
            {
            char Buffer[80];
            strftime(dtstr, 79, (loggedIn) ? CurrentUser->GetDateStamp(Buffer, sizeof(Buffer)) : cfg.datestamp, timestamp);
            }
        }

    // verbose and is a sysop or not anonymous
    if (MRO.Verbose && (CurrentUser->IsSysop() || !(RoomTab[Msg->GetRoomNumber()].IsAnonymous() || Msg->IsAnonymous())))
        {
        termCap(TERM_BOLD);
        label LocalID;
        CopyStringToBuffer(LocalID, ltoac(atol(Msg->GetLocalID())));

        CRmPrintf(getmsg(MSG_READMSG, 23), LocalID, ltoac(MessageDat.NewestMessage()));

        DebugOut(37, ltoac(Msg->GetOriginalRoom()));

        if (Msg->WasEverCompressed())
            {
            label Orig;
            CopyStringToBuffer(Orig, ltoac(hufOrig));

            DebugOut(38, Orig, ltoac(hufNew));
            }

        if (Msg->IsViewDuplicate() && CurrentUser->IsAide())
            {
            mPrintf(getmsg(MSG_READMSG, 24), ltoac(Msg->GetOriginalID()));
            }

        if (*Msg->GetSourceID())
            {
            CRmPrintf(getmsg(MSG_READMSG, 25), ltoac(atol(Msg->GetSourceID())));
            }

        if (*Msg->GetSourceRoomName())
            {
            CRmPrintf(getmsg(MSG_READMSG, 26), Msg->GetSourceRoomName());
            }

        if (*Msg->GetFileLink() && CurrentUser->IsSysop())
            {
            CRmPrintf(getmsg(MSG_READMSG, 27), Msg->GetFileLink());
            }

        if (*Msg->GetLinkedApplication() && CurrentUser->IsSysop())
            {
            CRmPrintf(getmsg(MSG_READMSG, 28), Msg->GetLinkedApplication());
            }

        if (*Msg->GetFromPath())
            {
            char lastlocID[5];
            CRmPrintf(getmsg(MSG_READMSG, 29), Msg->GetFromPath());

            // Get last region, this stuff probably not necessary
            int i, j;

            for (i = strlen(Msg->GetFromPath()); i && Msg->GetFromPath()[i] != '.'; i--);

            for (j = 0, i++; Msg->GetFromPath()[i] && Msg->GetFromPath()[i] != '!' && j < 4; i++, j++)
                {
                lastlocID[j] = Msg->GetFromPath()[i];
                }
            lastlocID[j] = 0;

            // if last region was same as current region just use alias
            if (SameString(lastlocID, cfg.locID))
                {
                mPrintf(getmsg(2), cfg.alias);
                }
            else
                {
                mPrintf(getmsg(2), cfg.Address);
                }
            }

        if (*Msg->GetOriginSoftware())
            {
            CRmPrintf(getmsg(MSG_READMSG, 30), Msg->GetOriginSoftware());
            }

        if (*Msg->GetToPath())
            {
            CRmPrintf(getmsg(MSG_READMSG, 31), Msg->GetToPath());
            }
        }

    termCap(TERM_BOLD);

    if (RoomTab[thisRoom].IsAnonymous() || Msg->IsAnonymous())
        {
        CRmPrintf(getmsg(311));
        }
    else
        {
        CRmPrintf(getmsg(MSG_READMSG, 66), dtstr);
        }

    if (*Msg->GetAuthor())
        {
        mPrintf(getmsg(MSG_READMSG, 32));

        if (!RoomTab[Msg->GetRoomNumber()].IsAnonymous() && !Msg->IsAnonymous())
            {
            if (*Msg->GetTitle() && CurrentUser->IsViewTitleSurname() &&
                    (Msg->GetOriginNodeName() ? cfg.nettitles : cfg.titles))
                {
                mPrintf(getmsg(MSG_READMSG, 67), Msg->GetTitle());
                }

            mPrintf(getmsg(MSG_READMSG, 68), Msg->GetAuthor());

            ShowTagsForUser(Msg->GetAuthor());

            termCap(TERM_BOLD);

            if (*Msg->GetSurname() &&
                    (
                        (CurrentUser->IsViewTitleSurname() && (*Msg->GetOriginNodeName() ? cfg.netsurname : cfg.surnames))
                    ||
                        (SameString(Msg->GetAuthor(), getmsg(385)) || SameString(Msg->GetAuthor(), getmsg(386)))
                    ))
                {
                mPrintf(getmsg(MSG_READMSG, 69), Msg->GetSurname());
                }
            }
        else
            {
            mPrintf(getmsg(MSG_READMSG, 68), cfg.anonauthor);
            }
        }

    termCap(TERM_BOLD);

    if (*Msg->GetOriginNodeName() &&
            (!SameString(Msg->GetOriginNodeName(), cfg.nodeTitle) || !SameString(Msg->GetOriginRegion(), cfg.nodeRegion)) &&
            !SameString(Msg->GetAuthor(), Msg->GetOriginNodeName()))
        {
        mPrintf(getmsg(211), Msg->GetOriginNodeName());
        }

    if (*Msg->GetOriginRegion() && !SameString(Msg->GetOriginRegion(), cfg.nodeRegion))
        {
        mPrintf(getmsg(229), Msg->GetOriginRegion());

        if (MRO.Verbose && *Msg->GetTwitRegion())
            {
            mPrintf(getmsg(MSG_READMSG, 33), Msg->GetTwitRegion());
            }
        }

    if (*Msg->GetOriginCountry() && MRO.Verbose && !SameString(Msg->GetOriginCountry(), cfg.nodeCountry))
        {
        mPrintf(getmsg(229), Msg->GetOriginCountry());

        if (MRO.Verbose && *Msg->GetTwitCountry())
            {
            mPrintf(getmsg(MSG_READMSG, 33), Msg->GetTwitCountry());
            }
        }

    if (*Msg->GetCit86Country() && MRO.Verbose)
        {
        mPrintf(getmsg(229), Msg->GetCit86Country());
        }

    if (Msg->GetOriginPhoneNumber()[0] && MRO.Verbose)
        {
        mPrintf(getmsg(229), Msg->GetOriginPhoneNumber());
        }

    if (Msg->GetToUser()[0])
        {
        // !mass e-mail
        if (!Msg->GetGroup()[0])
            {
            mPrintf(getmsg(MSG_READMSG, 34), Msg->GetToUser());
            }
        else
            {
            label Buffer;
            mPrintf(getmsg(MSG_READMSG, 34), CurrentUser->GetName(Buffer, sizeof(Buffer)));
            }

        if (*Msg->GetForward())
            {
            mPrintf(getmsg(MSG_READMSG, 35), Msg->GetForward());
            }

        if (Msg->IsNetworkedMail() && !Msg->IsNetworkedMailForHere())
            {
            mPrintf(getmsg(211), Msg->GetToNodeName());
            }

        if (*Msg->GetToRegion() && !SameString(Msg->GetToRegion(), cfg.nodeRegion))
            {
            mPrintf(getmsg(229), Msg->GetToRegion());
            }

        if (*Msg->GetReplyToMessage())
            {
            if (MRO.Verbose)
                {
                mPrintf(getmsg(MSG_READMSG, 36), Msg->GetReplyToMessage());
                }
            else
                {
                mPrintf(getmsg(MSG_READMSG, 37));
                }
            }

        if (Msg->IsReceived())
            {
            mPrintf(getmsg(MSG_READMSG, 38));
            }

        if (Msg->IsRepliedTo())
            {
            mPrintf(getmsg(MSG_READMSG, 39));
            }

        if (*Msg->GetToUser() && IsRecipientOfMail(Msg))
            {
            if (!(Msg->GetOriginalAttribute() & ATTR_RECEIVED) && !MRO.Headerscan)
                {
                Msg->SetOriginalAttribute((char) (Msg->GetOriginalAttribute() | ATTR_RECEIVED));

                MessageDat.ChangeAttribute(Msg->GetOriginalID(), Msg->GetOriginalAttribute());

                madeRec = TRUE;
                }
            }
        }

    if (*Msg->GetCreationRoom() && (cfg.showmoved || MRO.Verbose) && !RoomTab[thisRoom].IsSameName(Msg->GetCreationRoom()))
        {
        mPrintf(getmsg(MSG_READMSG, 40), Msg->GetCreationRoom());
        }

    if (*Msg->GetGroup())
        {
        mPrintf(getmsg(MSG_READMSG, 41), Msg->GetGroup());
        }

    if (Msg->IsLocal())
        {
        mPrintf(getmsg(MSG_READMSG, 70), cfg.Lmsg_nym);
        }

    if (Msg->IsReceiptConfirmationRequested())
        {
        mPrintf(getmsg(MSG_READMSG, 71));
        }

    if (*Msg->GetX() && CurrentUser->CanModerateRoom(Msg->GetOriginalRoom()))
        {
        if (!Msg->IsMadeVisible())
            {
            if (*Msg->GetX() == 'Y')
                {
                mPrintf(getmsg(MSG_READMSG, 42), cfg.Luser_nym);
                }
            else
                {
                mPrintf(getmsg(MSG_READMSG, 43));
                }
            }
        else
            {
            mPrintf(getmsg(MSG_READMSG, 44), *Msg->GetX() == 'Y' ? getmsg(MSG_READMSG, 45) : getmsg(MSG_READMSG, 46),
                    *Msg->GetX() == 'Y' ? cfg.Luser_nym : ns);
            }
        }

    if (*Msg->GetFileLink() && CurrentUser->IsAide())
        {
        mPrintf(getmsg(MSG_READMSG, 47));
        }

    if (Msg->IsCensored())
        {
        mPrintf(getmsg(MSG_READMSG, 48));
        }

    if (*Msg->GetPublicToUser())
        {
        CRmPrintf(getmsg(MSG_READMSG, 72), Msg->GetPublicToUser());
        }

    if (*Msg->GetSubject() && CurrentUser->IsViewSubjects())
        {
        CRmPrintf(getmsg(MSG_READMSG, 73), Msg->GetSubject());
        }

    long Idx, NumCmt = Msg->GetCommentCount();
    for (Idx = 1; Idx <= NumCmt; Idx++)
        {
        const char *Cmt = Msg->GetComment(Idx);

        CRmPrintf(getmsg(MSG_READMSG, 49), Cmt);
        }

    termCap(TERM_NORMAL);
    doCR();

    if (madeRec && Msg->IsReceiptConfirmationRequested())
        {
        Message *NewMsg;
        Bool good = TRUE;

        CRmPrintf(getmsg(MSG_READMSG, 50));

        NewMsg = new Message;

        if (NewMsg)
            {
            NewMsg->SetCreationRoom(Msg->GetCreationRoom());

            NewMsg->SetRoomNumber(Msg->GetRoomNumber());

            NewMsg->SetToUser(Msg->GetAuthor());
            NewMsg->SetAuthor(cfg.nodeTitle);

            NewMsg->SetReplyToMessage(*Msg->GetSourceID() ? Msg->GetSourceID() : Msg->GetLocalID());

            if (*Msg->GetSourceID())
                {
                label Buffer;

                NewMsg->SetToNodeName(Msg->GetOriginNodeName());
                NewMsg->SetToRegion(Msg->GetOriginRegion());
                NewMsg->SetToCountry(Msg->GetOriginCountry());
                NewMsg->SetDestinationAddress(Msg->GetOriginAddress(Buffer));
                }

            NewMsg->SetSubject(getmsg(MSG_READMSG, 51));

            if (*Msg->GetSourceID())
                {
                NewMsg->SetTextWithFormat(getmsg(MSG_READMSG, 52), ltoac(atol(Msg->GetSourceID())),
                        Msg->GetAuthor(), Msg->GetOriginNodeName(), Msg->GetToUser(), cfg.nodeTitle);

                NewMsg->AppendText(bn);
                NewMsg->AppendText(getmsg(MSG_READMSG, 53));
                NewMsg->AppendText(Msg->GetFromPath());
                }
            else
                {
                NewMsg->SetTextWithFormat(getmsg(MSG_READMSG, 54), ltoac(atol(Msg->GetLocalID())), Msg->GetAuthor(),
                        Msg->GetToUser());
                }

            if (*NewMsg->GetToNodeName())
                {
                if (!save_mail(NewMsg, FALSE, NULL, CERROR))
                    {
                    good = FALSE;
                    CRmPrintfCR(getmsg(MSG_READMSG, 55));
                    }
                }

            if (good)
                {
                putAndNoteMessage(NewMsg, FALSE);
                doCR();
                }

            delete NewMsg;
            }
        else
            {
            OutOfMemory(106);
            }

        doCR();
        }

    if (*Msg->GetToUser() && OC.whichIO != CONSOLE)
        {
        if (!(SameString(Msg->GetToUser(), getmsg(385)) || SameString(Msg->GetToUser(), getmsg(386)) ||
                SameString(Msg->GetAuthor(), getmsg(385)) || SameString(Msg->GetAuthor(), getmsg(386))))
            {
            OC.Echo = CALLER;
            OC.setio();
            }
        }

    if (OC.User.GetOutFlag() == OUTPARAGRAPH)
        {
        OC.SetOutFlag(OUTOK);
        }

    }

void TERMWINDOWMEMBER roomJournal(void)
    {
    char fileName[64];

    doCR();

    SetDoWhat(SYSJOURNAL);

    getNormStr(getmsg(289), fileName, 63);

    if (*fileName)
        {
        changedir(cfg.homepath);

        if ((journalfl = fopen(fileName, FO_A)) == NULL)
            {
            CRmPrintfCR(getmsg(78), fileName);
            }
        else
            {
            FILE *tmp;
            SuspendJournalFile(&tmp);
            long howmany = getNumber(getsysmsg(223), 0, LONG_MAX, 0, FALSE, NULL);
            RestoreJournalFile(tmp);

            doCR();

            MRO.Verbose = TRUE;
            MRO.Number = TRUE;

            showMessages(OldAndNew, FALSE, howmany);
            fclose(journalfl);

            journalfl = NULL;
            doCR();
            }
        }
    }
