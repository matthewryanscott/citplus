// --------------------------------------------------------------------------
// Citadel: Edit.CPP
//
// Message editor and related code.

#include "ctdl.h"
#pragma hdrstop

#include "extedit.h"
#include "room.h"
#include "tallybuf.h"
#include "net.h"
#include "group.h"
#include "log.h"
#include "hall.h"
#include "events.h"
#include "helpfile.h"
#include "filerdwr.h"
#include "miscovl.h"
#include "term.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// editText()       handles the end-of-message-entry menu.
// putheader()      prints header for message entry
// getText()        reads a message from the user
// matchString()    searches for match to given string.
// replaceString()  corrects typos in message entry
// wordcount()      talleys # lines, word & characters message contains
// xPutStr()        Put a string to a file w/o trailing blank
// GetFileMessage() gets a null-terminated string from a file


// --------------------------------------------------------------------------
// editText(): Handles the end-of-message-entry menu.
//
// Input:
//  Message *EditBuffer: The message to edit
//  int lim: The maximum length allowed
//  Message *HoldBuffer: Where to copy of the message if Abort; set to NULL
//      if calling for non-message-editing reasons. (.FE)
//
// Return value:
//  TRUE: User picked Save
//  FALSE: User picked Abort
//  CERROR: User picked Continue

int TERMWINDOWMEMBER editText(Message *EditBuffer, int lim, Message *HoldBuffer)
    {
    VerifyHeap();

    if (!LockMenu(MENU_EDIT))
        {
        OutOfMemory(30);
        return (FALSE);
        }

    strcpy(gprompt, getmsg(MENU_EDIT, 8));
    SetDoWhat(PROMPT);

    OC.User.Reset();
    ansi(14);
    OC.MCI_goto = FALSE;

    do
        {
        OC.SetOutFlag(IMPERVIOUS);

        while (CommPort->IsInputReady())
            {
            CommPort->Input();
            }

        Bool MayAltF3Timeout = FALSE;

        // We prepare for CheckTimeOnSystem calling a Terminate()...
        if (altF3Timeout)
            {
            CurrentUser->SetMessage(EditBuffer);
            MayAltF3Timeout = TRUE;
            }

        // Then we check the time...
        CheckTimeOnSystem();

        // Then we undo our preparation.
        CurrentUser->SetMessage(NULL);

        CRmPrintf(getmsg(MENU_EDIT, 8));
        OC.User.Reset();

        // Time out?
        if (MayAltF3Timeout && !altF3Timeout)
            {
            SetDoWhat(DUNO);
            UnlockMenu(MENU_EDIT);
            return (FALSE);
            }

        const int ch = iCharNE();

        if (RunExternalEditor((char) ch, MAXTEXT,
                EditBuffer->GetTextPointer()))
            {
            continue;
            }

        switch (DoMenu(MENU_EDIT, ch))
            {
            case -1:
            case 1: // Abort
                {
                mPrintfCR(getmsg(653));

                if (    // nothing to lose
                        !*EditBuffer->GetText() ||

                        // user doesn't want to be asked
                        !CurrentUser->IsConfirmAbort() ||

                        // user confirmed
                        getYesNo(getmsg(57), 0))
                    {
                    if (HoldBuffer)
                        {
                        *HoldBuffer = *EditBuffer;
                        }

                    SetDoWhat(DUNO);
                    UnlockMenu(MENU_EDIT);
                    return (FALSE);
                    }

                break;
                }

            case 2: // Continue
                {
                doCR(2);

                if (HoldBuffer)
                    {
                    OC.SetOutFlag(OUTOK);

                    putheader(FALSE, EditBuffer);
                    doCR();
                    }

                int textlen = strlen(EditBuffer->GetText());

                do
                    {
                    EditBuffer->GetTextPointer()[textlen] = 0;
                    } while (textlen > 0 && isspace(EditBuffer->GetTextPointer()[--textlen]));

                if (textlen < MAXTEXT - 1)
                    {
                    if (textlen < 130 || CurrentUser->IsVerboseContinue())
                        {
                        mFormat(EditBuffer->GetText());
                        }
                    else
                        {
                        char *textptr, *textend;

                        for (textptr = EditBuffer->GetTextPointer(),
                                textend = textptr + textlen;
                                (textend - textptr) >= 129; ++textptr)
                            {
                            if (*textptr == CTRL_A)
                                {
                                char wordBuf[MAXWORDLEN + 8];
                                int i;

                                i = getWord((uchar *) wordBuf,
                                        (uchar *) textptr, 0, MAXWORDLEN);

                                if (IsColorOrFormattingOnly(wordBuf + 1))
                                    {
                                    termCap(wordBuf + 1);
                                    }

                                textptr += i;
                                ansi(14);
                                OC.MCI_goto = FALSE;
                                }
                            }

                        mFormat(textptr);
                        }

                    ResetOutputControl();
                    OC.SetOutFlag(IMPERVIOUS);

                    UnlockMenu(MENU_EDIT);
                    return (CERROR);
                    }

                mPrintfCR(getmsg(MENU_EDIT, 9));

                break;
                }

            case 3: // Delete
                {
                if (HoldBuffer)
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 10), cfg.Lmsg_nym);
                    }
                else
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 11));
                    }

                if (getYesNo(getmsg(57), 0))
                    {
                    EditBuffer->SetText(ns);

                    OC.SetOutFlag(OUTOK);
                    doCR();

                    if (HoldBuffer)
                        {
                        putheader(FALSE, EditBuffer);
                        doCR();
                        }

                    UnlockMenu(MENU_EDIT);
                    return (CERROR);
                    }

                break;
                }

            case 4: // Encrypt
                {
                if (HoldBuffer)
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 12), cfg.Lmsg_nym);

                    if (cfg.allowCrypt == 2 ||
                            (cfg.allowCrypt && *EditBuffer->GetToUser()))
                        {
                        label InputString;

                        GetStringWithBlurb(getmsg(MENU_EDIT, 13), InputString,
                                LABELSIZE, EditBuffer->GetEncryptionKey(),
                                B_ENCRYPT);

                        if (*InputString)
                            {
                            normalizeString(InputString);
                            EditBuffer->SetEncryptionKey(InputString);
                            }
                        }
                    else
                        {
                        if (cfg.allowCrypt)
                            {
                            CRmPrintfCR(getmsg(MENU_EDIT, 14));
                            }
                        else
                            {
                            CRmPrintfCR(getmsg(MENU_EDIT, 15));
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 5: // Find & Replace
                {
                doCR();
                replaceString(EditBuffer->GetTextPointer(), lim, TRUE, NULL);
                break;
                }

            case 6: // Group
                {
                if (HoldBuffer)
                    {
                    label GroupName;

                    mPrintfCR(getmsg(MENU_EDIT, 16), cfg.Ugroup_nym);

					if (!CurrentUser->IsSysop() && *EditBuffer->GetToUser())
						{
						CRmPrintfCR(getmsg(MENU_EDIT, 53), cfg.Lgroup_nym, cfg.Lmsg_nym);
						}
					else
						{
						AskGroupName(GroupName, EditBuffer->GetGroup(), cfg.Lgroup_nym, LABELSIZE, TRUE);
	
						if (*GroupName)
							{
							normalizeString(GroupName);
	
							if (*GroupName)
								{
								g_slot GroupSlot;
	
								GroupSlot = FindGroupByPartialName(GroupName, FALSE);
	
								if (GroupSlot == CERROR || !CurrentUser->IsInGroup(GroupSlot))
									{
									CRmPrintf(getmsg(584), cfg.Lgroup_nym);
									}
								else
									{
									EditBuffer->SetGroup(GroupSlot);
									}
								}
							else
								{
								EditBuffer->SetGroup(ns);
								}
							}
						}
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 7: // Signature
                {
                if (HoldBuffer && (cfg.ecSignature || CurrentUser->IsSysop()))
                
                    {
                    mPrintfCR(getmsg(17));

                    if (!CurrentUser->IsUserSignatureLocked())
                        {
                        char InputString[100];

                        GetStringWithBlurb(getmsg(MENU_EDIT, 17), InputString, 90,
								EditBuffer->GetUserSignature(), B_SIGNATUR);

                        if (*InputString)
                            {
                            normalizeString(InputString);
                            EditBuffer->SetUserSignature(InputString);
                            }
                        }
                    else
                        {
                        CRmPrintfCR(getmsg(MENU_EDIT, 52));
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 8: // Location
                {
                if (HoldBuffer)
                    {
                    label Buffer, Buffer1;
                    mPrintfCR(getmsg(MENU_EDIT, 18));
                    CRmPrintfCR(getmsg(MENU_EDIT, 19), cfg.Lroom_nym, RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)),
                            cfg.Lhall_nym, HallData[thisHall].GetName(Buffer, sizeof(Buffer)));
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 9: // Print formatted
                {
                doCR(2);
                OC.SetOutFlag(OUTOK);

                if (HoldBuffer)
                    {
                    putheader(FALSE, EditBuffer);
                    doCR();
                    }

                mFormat(EditBuffer->GetText());
                ResetOutputControl();

                if (HoldBuffer)
                    {
                    if ((*EditBuffer->GetSignature() || *EditBuffer->GetUserSignature()) && CurrentUser->IsViewSignatures())
                        {
                        termCap(TERM_NORMAL);
                        doCR();
                        termCap(TERM_BOLD);
                        CRmPrintf(getmsg(279));
                        termCap(TERM_NORMAL);

                        if (*EditBuffer->GetUserSignature())
                            {
                            CRmPrintf(pcts, EditBuffer->GetUserSignature());
                            }

                        if (*EditBuffer->GetSignature())
                            {
                            CRmPrintf(pcts, EditBuffer->GetSignature());
                            }
                        }
                    }

                termCap(TERM_NORMAL);
                OC.SetOutFlag(IMPERVIOUS);

                doCR();
                break;
                }

            case 10: // Replace
                {
                doCR();
                replaceString(EditBuffer->GetTextPointer(), lim, FALSE, NULL);
                break;
                }

            case 11: // Save
                {
                if (HoldBuffer)
                    {
                    mPrintf(getmsg(636), cfg.msg_done, cfg.Lmsg_nym);
                    }
                else
                    {
                    mPrintf(getmsg(MENU_EDIT, 20));
                    }

                if (CurrentUser->GetSpellCheckMode() == 3)
                    {
                    CRCRmPrintfCR(getmsg(MENU_EDIT, 21));
                    spellCheck(EditBuffer->GetTextPointer(), lim);
                    }

                if (CurrentUser->IsConfirmSave())
                    {
                    if (CurrentUser->GetSpellCheckMode() != 3)
                        {
                        doCR();
                        }

                    if (!getYesNo((CurrentUser->GetSpellCheckMode() == 3) ? getmsg(MENU_EDIT, 22) : getmsg(57), 1))
                        {
                        break;
                        }
                    }

                if (strlen(EditBuffer->GetText()) < 2 && !*EditBuffer->GetSubject() && !*EditBuffer->GetLinkedApplication())
                    {
                    if (HoldBuffer)
                        {
                        mPrintfCR(getmsg(412), cfg.Lmsg_nym);
                        MS.Entered--;
                        }
                    }
                else
                    {
                    if (!CurrentUser->IsConfirmSave() && CurrentUser->GetSpellCheckMode() != 3)
                        {
                        doCR();
                        }

                    RunAutomaticExternalEditors(MAXTEXT, EditBuffer->GetTextPointer());
                    }

                if (HoldBuffer)
                    {
                    MS.Entered++;   // increment # messages entered
                    cyclesignature();   // load a new signature from CONFIG.CIT
                    doEvent(EVT_SAVEMSG);
                    }

                SetDoWhat(DUNO);
                UnlockMenu(MENU_EDIT);
                return (TRUE);
                }

            case 12: // Subject
                {
                if (HoldBuffer)
                    {
                    char InputString[100];

                    mPrintfCR(getmsg(MENU_EDIT, 23));

                    GetStringWithBlurb(getmsg(415), InputString, 80, EditBuffer->GetSubject(), B_SUBJECT);

                    if (*InputString)
                        {
                        normalizeString(InputString);
                        EditBuffer->SetSubject(InputString);
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 13: // Verify spelling
                {
                doCR();
                spellCheck(EditBuffer->GetTextPointer(), lim);
                break;
                }

            case 14: // Word count
                {
                wordcount(EditBuffer->GetText());
                break;
                }

            case 15: // Allow ESC
                {
                AllowESC = changeYesNo(getmsg(MENU_EDIT, 24), AllowESC);
                break;
                }

            case -2: // Menu
                {
                oChar('?');
                showMenu(HoldBuffer ? M_EDIT : M_EDITTEXT);
                break;
                }

            case 16: // Link application
                {
                if (HoldBuffer && (CurrentUser->IsSuperSysop() || (onConsole && CurrentUser->IsSysop())))
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 25));

                    char InputString[128];
                    GetStringWithBlurb(getmsg(MENU_EDIT, 26), InputString, 127, EditBuffer->GetLinkedApplication(),
                            B_LINKAPP);

                    if (*InputString)
                        {
                        normalizeString(InputString);
                        EditBuffer->SetLinkedApplication(InputString);
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 17: // Address message
                {
                if (HoldBuffer)
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 27));

                    if (loggedIn)
                        {
						if (!CurrentUser->IsSysop() && *EditBuffer->GetGroup())
							{
							CRmPrintfCR(getmsg(MENU_EDIT, 53), cfg.Lgroup_nym, cfg.Lmsg_nym);
							}
						else 
							{
							label SaveToUser;
							label SaveToNodeName;
							label SaveToRegion;
							label SaveDestinationAddress;
	
							strcpy(SaveToUser, EditBuffer->GetToUser());
							strcpy(SaveToNodeName, EditBuffer->GetToNodeName());
							strcpy(SaveToRegion, EditBuffer->GetToRegion());
							strcpy(SaveDestinationAddress, EditBuffer->GetDestinationAddress());
	
							char destination[LABELSIZE + LABELSIZE + LABELSIZE + LABELSIZE + 7];

							char wow[200];
							sprintf(wow, getmsg(565), cfg.Lmsg_nym, SaveToUser);
	
							AskUserName(destination, ns, wow, LABELSIZE + LABELSIZE + LABELSIZE + 6);
	
							stripansi(destination);
	
							if (*destination)
								{
								if (SameString(destination, spc))
									{
									EditBuffer->SetToUser(ns);
									EditBuffer->SetToNodeName(ns);
									EditBuffer->SetToRegion(ns);
									EditBuffer->SetDestinationAddress(ns);
									}
								else
									{
									label ToUser, ToNode, ToRegion, ToAddress;
	
									parseNetAddress(destination, ToUser, ToNode, ToRegion, ToAddress);
	
									if (!*ToUser)
										{
										strcpy(ToUser, getmsg(386));
										}
	
									EditBuffer->SetToUser(ToUser);
									EditBuffer->SetToNodeName(ToNode);
									EditBuffer->SetToRegion(ToRegion);
									EditBuffer->SetDestinationAddress(ToAddress);
									}
								}
	
							if (*EditBuffer->GetToUser())
								{
#ifdef WINCIT
                                if (!ResolveMessageAddressingIssues(EditBuffer, this, MODEM))
#else
                                if (!ResolveMessageAddressingIssues(EditBuffer, MODEM))
#endif
									{
									EditBuffer->SetToUser(SaveToUser);
									EditBuffer->SetToNodeName(SaveToNodeName);
									EditBuffer->SetToRegion(SaveToRegion);
									EditBuffer->SetDestinationAddress(SaveDestinationAddress);
									}
								}
	
							if (*EditBuffer->GetToUser())
								{
								if ((SameString(EditBuffer->GetToUser(), getmsg(386))) ||
									(SameString(EditBuffer->GetToUser(), getmsg(385))) ||
									(SameString(EditBuffer->GetAuthor(), getmsg(386))) ||
									(SameString(EditBuffer->GetAuthor(), getmsg(385))))
									{
									OC.Echo = BOTH;
									OC.setio();
									}
								else
									{
									OC.Echo = CALLER;
									OC.setio();
									}
								}
                            }
                        }
                    else
                        {
                        CRmPrintfCR(getmsg(239));
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 18: // Special delivery options
                {
                if (HoldBuffer)
                    {
                    // special delivery options
                    mPrintfCR(getmsg(MENU_EDIT, 28));

                    if (*EditBuffer->GetToUser())
                        {
                        EditBuffer->SetReceiptConfirmationRequested(getYesNo(getmsg(MENU_EDIT, 29),
                                EditBuffer->IsReceiptConfirmationRequested()));
                        }
                    else
                        {
                        CRmPrintfCR(getmsg(MENU_EDIT, 30));
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 19: // Change name
                {
                if (HoldBuffer)
                    {
                    mPrintfCR(getmsg(MENU_EDIT, 31));

                    if (cfg.titles && CurrentUser->IsViewTitleSurname() &&
							(!CurrentUser->IsSurnameLocked() || CurrentUser->IsSysop()))
                        {
                        label InputString;

                        GetStringWithBlurb(getmsg(253), InputString, LABELSIZE, EditBuffer->GetTitle(), B_MNAMECHG);

                        if (*InputString)
                            {
                            normalizeString(InputString);
                            EditBuffer->SetTitle(InputString);
                            }
                        }

                    if (CurrentUser->IsSysop())
                        {
                        label InputString;

                        GetStringWithBlurb(getmsg(424), InputString, LABELSIZE, EditBuffer->GetAuthor(), B_MNAMECHG);

                        if (*InputString)
                            {
                            normalizeString(InputString);
                            EditBuffer->SetAuthor(InputString);
                            }
                        }

                    if (cfg.surnames && CurrentUser->IsViewTitleSurname() &&
							(!CurrentUser->IsSurnameLocked() || CurrentUser->IsSysop()))
                        {
                        label InputString;

                        GetStringWithBlurb(getmsg(254), InputString, LABELSIZE, EditBuffer->GetSurname(), B_MNAMECHG);

                        if (*InputString)
                            {
                            normalizeString(InputString);
                            EditBuffer->SetSurname(InputString);
                            }
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            case 20: // Insert file
                {
                if (CurrentUser->IsSuperSysop() || (CurrentUser->IsSysop() && onConsole))
                    {
                    char *TextFromFile = new char[MAXTEXT];

                    if (TextFromFile)
                        {
                        char FileName[80];

                        mPrintfCR(getmsg(MENU_EDIT, 32));

                        GetStringWithBlurb(getmsg(289), FileName, 79, ns, B_FILENSRT);

                        normalizeString(FileName);

                        if (*FileName)
                            {
                            FILE *fd;

                            if ((fd = fopen(FileName, FO_RB)) != NULL)
                                {
                                GetFileMessage(fd, TextFromFile, MAXTEXT - 1);
                                fclose(fd);

                                EditBuffer->AppendText(TextFromFile);
                                }
                            else
                                {
                                CRmPrintfCR(getmsg(422));
                                }
                            }

                        delete [] TextFromFile;
                        }
                    else
                        {
                        OutOfMemory(30);
                        }
                    }
                else
                    {
                    BadMenuSelection(ch);
                    }

                break;
                }

            default:
                {
                BadMenuSelection(ch);
                break;
                }
            }
        } while ((HaveConnectionToUser()));

    SetDoWhat(DUNO);
    UnlockMenu(MENU_EDIT);
    return (FALSE);
    }


// --------------------------------------------------------------------------
// putheader(): Prints header for message entry.
//
// Input:
//  Bool first: TRUE if the first time called for the message (Right after
//      the Enter command is picked, not during Continue or Print formatted)
//  Message *Msg: The message
//
// Notes:
//  This really needs to be combined with printheader() in MSGREAD.CPP.

void TERMWINDOWMEMBER putheader(Bool first, Message *Msg)
    {
    if (LockMessages(MSG_READMSG))
        {
        char dtstr[80];
        const EchoType oldEcho = OC.Echo;

        if (first)
            {
            OC.Echo = BOTH;
            OC.setio();
            }

        termCap(TERM_BOLD);

        if (!RoomTab[thisRoom].IsAnonymous())
            {
            char Buffer[64];
            strftime(dtstr, 79, (loggedIn) ?
                    CurrentUser->GetDateStamp(Buffer, sizeof(Buffer)) :
                    cfg.datestamp, 0l);

            mPrintf(getmsg(MSG_READMSG, 66), dtstr);
            }
        else
            {
            mPrintf(getmsg(311));
            }

        mPrintf(getmsg(MSG_READMSG, 32));

        if (!RoomTab[thisRoom].IsAnonymous())
            {
            if (*Msg->GetTitle() &&
                    CurrentUser->IsViewTitleSurname() &&
                    (Msg->GetOriginNodeName() ?
                        cfg.nettitles : cfg.titles)
                )
                {
                mPrintf(getmsg(MSG_READMSG, 67), Msg->GetTitle());
                }

            mPrintf(getmsg(MSG_READMSG, 68), Msg->GetAuthor());

            if (*Msg->GetSurname() &&
                    (
                        (
                        CurrentUser->IsViewTitleSurname() &&
                        (*Msg->GetOriginNodeName() ?
                                cfg.netsurname : cfg.surnames)
                        )
                    ||
                        (
                        SameString(Msg->GetAuthor(), getmsg(385)) ||
                        SameString(Msg->GetAuthor(), getmsg(386))
                        )
                    )
                )
                {
                mPrintf(getmsg(MSG_READMSG, 69), Msg->GetSurname());
                }
            }
        else
            {
            mPrintf(getmsg(MSG_READMSG, 68), cfg.anonauthor);
            }

        if (Msg->GetToUser()[0])
                mPrintf(getmsg(MSG_READMSG, 34), Msg->GetToUser());

        if (Msg->GetForward()[0])
                mPrintf(getmsg(MSG_READMSG, 35), Msg->GetForward());

        if (Msg->GetToNodeName()[0])
                mPrintf(getmsg(211), Msg->GetToNodeName());

        if (Msg->GetToRegion()[0])
                mPrintf(getmsg(229), Msg->GetToRegion());

        if (Msg->GetToCountry()[0])
                mPrintf(getmsg(229), Msg->GetToCountry());

        if (Msg->GetGroup()[0])
                mPrintf(getmsg(MSG_READMSG, 41), Msg->GetGroup());

        if (Msg->IsLocal())
            {
            mPrintf(getmsg(MSG_READMSG, 70), cfg.Lmsg_nym);
            }

        if (Msg->IsReceiptConfirmationRequested())
            {
            mPrintf(getmsg(MSG_READMSG, 71));
            }

        if (*Msg->GetPublicToUser())
            {
            CRmPrintf(getmsg(MSG_READMSG, 72), Msg->GetPublicToUser());
            }

        if (*Msg->GetSubject() && CurrentUser->IsViewSubjects())
            {
            CRmPrintf(getmsg(MSG_READMSG, 73), Msg->GetSubject());
            }

        termCap(TERM_NORMAL);

        OC.Echo = oldEcho;
        OC.setio();

        UnlockMessages(MSG_READMSG);
        }
    else
        {
        OutOfMemory(108);
        }
    }


Bool LogExtensions::FindReplace(const char *ToReplace, char *Buffer, int Length)
    {
    GAINEXCLUSIVEACCESS();

    for (const pairedStrings *cur = le_replace; cur;
            cur = (pairedStrings *) getNextLL(cur))
        {
        if (SameString(cur->string1, ToReplace))
            {
            CopyString2Buffer(Buffer, cur->string2, Length);
            RELEASEEXCLUSIVEACCESS();
            return (TRUE);
            }
        }

    RELEASEEXCLUSIVEACCESS();
    *Buffer = 0;
    return (FALSE);
    }


// --------------------------------------------------------------------------
// getText(): Reads a message from the user
//
// Input:
//  Message *Msg: A place to put the message, or NULL if just editing block
//      of text
//  char *Buffer: A place to put a block of test, or NULL if editing a
//      message
//
// Return value:
//  TRUE: User wants it saved
//  FALSE: User doesn't want it saved

#define GTWORDSIZE  50      // For wordwrap, replace in log extensions

Bool TERMWINDOWMEMBER getText(Message *Msg, char *Buffer)
    {
    Bool done, beeped = FALSE;
    char *buf;
    int toReturn;

    VerifyHeap();

    AllowESC = FALSE;

    if (Msg)
        {
        if (!CurrentUser->IsExpert())
            {
            dispBlb(B_ENTRY);
            OC.SetOutFlag(OUTOK);

            CRmPrintfCR(getmsg(430), ltoac(MAXTEXT));
            mPrintfCR(getmsg(431), cfg.Lmsg_nym);
            }

        OC.SetOutFlag(IMPERVIOUS);

        doCR();
        putheader(TRUE, Msg);
        doCR();

        buf = Msg->GetTextPointer();
        }
    else
        {
        buf = Buffer;
        }

    OC.SetOutFlag(OUTOK);

    for (   // look at the end
            int i = strlen(buf);

            // and strip out any whitespace (is isspace(10) == TRUE???)
            // isspace() matches: spc, FF, NL, CR, HT, VT
            i > 0 && isspace(buf[i - 1]);

            // and keep going
            i--)
        {
        buf[i - 1] = 0;
        }

    VerifyHeap();

    mFormat(buf);
    ResetOutputControl();

    OC.SetOutFlag(IMPERVIOUS);

    done = FALSE;

    VerifyHeap();

    do
        {
        VerifyHeap();

        int termPos = 0;
        label tempTerm;

        tempTerm[0] = 0;

        SetDoWhat(MESSAGETEXT);
        int i = strlen(buf);

        char word[GTWORDSIZE + 1];
        int wsize = 0;

        int ReplaceCharCounter = 0;

        int c = 0;

        while (!done && i < (MAXTEXT - 1) && (HaveConnectionToUser()))
            {
            c = iCharNE();

            if (ReplaceCharCounter)
                {
                ReplaceCharCounter--;
                }

            if (termPos)
                {
                if (c == '\b')
                    {
                    if (termPos > 2)
                        {
                        termPos -= 1;
                        }
                    else
                        {
                        termPos = 0;
                        }
                    }
                else
                    {
                    tempTerm[termPos++] = (char) c;
                    tempTerm[termPos] = 0;

                    if (c == 'X')
                        {
                        if (i + termPos >= (MAXTEXT - 1))
                            {
                            oChar(BELL);
                            }
                        else
                            {
                            strcat(buf, tempTerm);
                            i += termPos;
                            termCap(tempTerm + 1);
                            ansi(14);
                            OC.MCI_goto = FALSE;
                            }

                        termPos = 0;
                        }
                    else if (termPos >= LABELSIZE)
                        {
                        oChar(BELL);
                        termPos = 0;
                        }
                    }
                }
            else
                {
                switch (c)                      // Analyze what they typed
                    {
                    case CTRL_A:                // CTRL+A>nsi
                        {
                        const int d = iCharNE();

                        if (ReplaceCharCounter)
                            {
                            ReplaceCharCounter--;
                            }

                        if (d == '?')
                            {
                            dispHlp(H_ANSI);
                            OC.SetOutFlag(OUTOK);

                            for (i = strlen(buf);
                                    i > 0 && buf[i - 1] == '\n'; i--)
                                {
                                buf[i-1] = 0;
                                }

                            doCR(2);

                            if (Msg)
                                {
                                OC.SetOutFlag(OUTOK);

                                putheader(FALSE, Msg);
                                doCR();
                                }

                            mFormat(buf);
                            ResetOutputControl();

                            OC.SetOutFlag(IMPERVIOUS);
                            }
                        else if (d == 'X')
                            {
                            tempTerm[0] = CTRL_A;
                            tempTerm[1] = 'X';
                            tempTerm[2] = 0;
                            termPos = 2;
                            }
                        else if (
                                (d >= '0' && d <= '8')      ||
                                (d >= 'A' && d <= 'H')      ||
                                (d == TERM_DATE[0])         ||
                                (d == TERM_TIME[0])         ||
                                (d == TERM_USERNAME[0])     ||
                                (d == TERM_FIRSTNAME[0])    ||
                                (d == TERM_POOP[0])         ||
                                (d == TERM_RND_BACK[0])     ||
                                (d == TERM_RND_FORE[0])     ||
                                (d >= 'a' && d <= 'h')
                                )
                            {
                            if ((d != '5') && (d != '6'))
                                {
                                tempTerm[0] = (char) d;
                                tempTerm[1] = 0;

                                termCap(tempTerm);
                                }

                            buf[i++] = 0x01;
                            buf[i++] = (char) d;
                            }
                        else
                            {
                            oChar(BELL);
                            }

                        break;
                        }

                    case CTRL_D:
                        {
                        // 128-168; 171-175
                        int New = 0;

                        if (i > 1)
                            {
                            discardable *d = readData(6, 7, 7);

                            if (d)
                                {
                                struct CombineData
                                    {
                                    short First, Second, Result;
                                    } *Data = (CombineData *) d->data;

                                for (int j = 0; Data[j].First; j++)
                                    {
                                    if (buf[i - 2] == Data[j].First &&
                                            buf[i - 1] == Data[j].Second)
                                        {
                                        New = Data[j].Result;
                                        break;
                                        }
                                    }

                                discardData(d);
                                }
                            else
                                {
                                OutOfMemory(31);
                                }
                            }

                        if (New)
                            {
                            i--;
                            buf[i - 1] = (char) New;

                            doBS();
                            doBS();
                            echocharacter((char) New);
                            }
                        else
                            {
                            oChar(BELL);
                            }

                        break;
                        }

                    case 26:                        // Control+Z
                        {
                        done = TRUE;

                        if (Msg)
                            {
                            MS.Entered++;
                            }

                        break;
                        }

                    case '\b':                      // Backspace
                        {
                        if (i > 0 && buf[i - 1] == '\t')    // Tab
                            {
                            doBS();
                            i--;

                            while ((OC.CrtColumn % 8) != 1)
                                {
                                doBS();
                                }
                            }
                        else if (i > 0 && buf[i - 1] != '\n')
                            {
                            doBS();
                            i--;

                            if (wsize > 0)
                                {
                                wsize--;
                                }
                            }
                        else
                            {
                            oChar(BELL);
                            }

                        break;
                        }

                    case '\n':                          // Newline
                        {
                        if (!i || (buf[i - 1] == '\n'))
                            {
                            done = TRUE;
                            break;
                            }
                        }

                    case ESC:
                        {
                        if (c == ESC && !AllowESC)
                            {
                            echocharacter(BELL);
                            break;
                            }
                        }

                    default:
                        {
                        if ((ispunct(c) || isspace(c)) && wsize &&
                                !ReplaceCharCounter)
                            {
                            // #replace in logext
                            word[wsize] = 0;

                            label Replacement;
                            if (CurrentUser->FindReplace(word, Replacement,
                                    sizeof(Replacement)))
                                {
                                // Here are some assumptions: if word
                                // is initial cap, make the replacement
                                // be initial cap. if it is all cap, make
                                // the replacement be all cap. else,
                                // replace in replacement's case.
                                Bool InitialCap = isupper(word[0]);

                                Bool AllCap = TRUE;
                                for (int j = 0; j <= wsize; j++)
                                    {
                                    if (islower(word[j]))
                                        {
                                        AllCap = FALSE;
                                        break;
                                        }
                                    }

                                // Okay... we know the case. Now do it

                                // First, insert our original character
                                char Wow[2];
                                Wow[0] = (char) c;
                                Wow[1] = 0;
                                KeyboardBuffer.InsertString(Wow);
                                ReplaceCharCounter = 2; // This character plus 1

                                // Now insert our new text...
    Bool DoneFirst = FALSE, InANSI = FALSE, InMCI = FALSE;
EXTRA_OPEN_BRACE	
    for (int j = 0; Replacement[j]; j++)
        {
        char Cur = Replacement[j];

        // If this is ^A/^B, start ANSI
        if (Cur == CTRL_A || Cur == CTRL_B)
            {
            InANSI = TRUE;
            }

        // If this is X, start extended MCI
        if (InANSI && toupper(Cur) == 'X')
            {
            InMCI = TRUE;
            }

        // Capitalize if needed, but only if not in ANSI or MCI
        if (AllCap || (InitialCap && !DoneFirst) && !InANSI && !InMCI)
            {
            Replacement[j] = (char) toupper(Cur);
            }

        // We have done our first "normal" character... no more initial cap
        if (!InANSI && !InMCI)
            {
            DoneFirst = TRUE;
            }

        // At the first X, InANSI is still TRUE, so InMCI stays on. At
        // the second X, InANSI is now FALSE, so InMCI turns off.
        if (InMCI && toupper(Cur) == 'X')
            {
            InMCI = InANSI;
            }

        // If we have done a non-^A/^B character, ANSI turns off.
        if (Cur != CTRL_A && Cur != CTRL_B)
            {
            InANSI = FALSE;
            }
        }
EXTRA_CLOSE_BRACE	
                                if (KeyboardBuffer.InsertString(Replacement))
                                    {
                                    ReplaceCharCounter += strlen(Replacement);

                                    // And insert BSes before that...
                                    int j;
                                    for (j = 0; j < wsize; j++)
                                        {
                                        word[j] = '\b';
                                        }
                                    word[j] = 0;

                                    if (KeyboardBuffer.InsertString(word))
                                        {
                                        ReplaceCharCounter += wsize;
                                        }
                                    }

                                // And skip our character for now...
                                c = 0;

                                break;
                                }
                            }

                        if (c && (c == '\n' || c == ' ' || c == '\t' ||
                                wsize >= GTWORDSIZE))
                            {
                            wsize = 0;
                            }
                        else if (c)
                            {
                            if (OC.CrtColumn >=
                                    CurrentUser->GetWidth() - 1)
                                {
                                if (wsize)
                                    {
                                    doBS(wsize);

                                    doCR();

                                    for (int j = 0; j < wsize; j++)
                                        {
                                        echocharacter(word[j]);
                                        }
                                    }
                                else
                                    {
                                    doCR();
                                    }
                                }

                            word[wsize] = (char) c;
                            wsize++;
                            }

                        if (c)
                            {
                            echocharacter((char) c);
                            buf[i++] = (char) c;

                            if (i > MAXTEXT - 80 && !beeped)
                                {
                                beeped = TRUE;
                                oChar(BELL);
                                }
                            }

                        break;
                        }
                    }
                }

            buf[i] = 0;                         // null to terminate message

            if (i >= (MAXTEXT - 1))
                {
                mPrintfCR(getmsg(432));
                KeyboardBuffer.Flush();
                }
            }

        done = FALSE;                           // In case they Continue

        termCap(TERM_NORMAL);

        if (c == 26 && i != (MAXTEXT - 1))      // if was Control+Z
            {
            buf[i++] = '\n';                    // end with newline
            buf[i] = 0;
            toReturn = TRUE;                    // yes, save it

            CRmPrintf(getmsg(636), cfg.msg_done, cfg.Lmsg_nym);

            if (Msg)
                {
                if (strlen(buf) > 1 || (Msg &&
                        (*Msg->GetSubject() || *Msg->GetLinkedApplication())))
                    {
                    // load a new signature from config.cit
                    cyclesignature();
                    }
                else
                    {
                    mPrintf(getmsg(412), cfg.Lmsg_nym);
                    MS.Entered--;
                    }
                }

            doCR();
            }
        else                        // done or lost carrier
            {
            if (Msg)
                {
                delete MS.AbortedMessage;
                MS.AbortedMessage = NULL;

                compactMemory();

                MS.AbortedMessage = new Message;

                if (MS.AbortedMessage)
                    {
                    toReturn = editText(Msg, (MAXTEXT - 1),
                            MS.AbortedMessage);

                    VerifyHeap();

                    // editText() might indirectly delete MS.AbortedMessage
                    // if Alt+F3 timeout happens
                    if (MS.AbortedMessage &&
                            (toReturn || !*MS.AbortedMessage->GetText()))
                        {
                        delete MS.AbortedMessage;
                        MS.AbortedMessage = NULL;
                        }
                    }
                else
                    {
                    OutOfMemory(31);
                    toReturn = FALSE;
                    }
                }
            else
                {
                Message *Msg1 = new Message;

                if (Msg1)
                    {
                    Msg1->SetText(buf);

                    toReturn = editText(Msg1, (MAXTEXT - 1), NULL);

                    CopyString2Buffer(buf, Msg1->GetText(), MAXTEXT);

                    delete Msg1;
                    }
                else
                    {
                    OutOfMemory(31);
                    toReturn = FALSE;
                    }
                }
            }
        } while ((toReturn == CERROR) && (HaveConnectionToUser()));
                // CERROR returned from editor means continue

    if (Msg && !HaveConnectionToUser() && (strlen(buf) > 1) && loggedIn)
        {
        jumpback jb;

        jb.hall = thisHall;
        jb.room = thisRoom;
        jb.newpointer = CurrentUser->GetRoomNewPointer(thisRoom);
        jb.bypass = Talley->Bypassed(thisRoom);
        jb.newMsgs = Talley->NewInRoom(thisRoom);

        CurrentUser->JumpbackPush(jb);
        CurrentUser->SetMessage(Msg);
        }

    if (toReturn == TRUE || toReturn == CERROR) // Filter null messages
        {
        toReturn = FALSE;

        for (int i = 0; buf[i] != 0 && !toReturn; i++)
            {
            toReturn = (buf[i] > ' ');
            }

        if (!toReturn)
            {
            if (Msg)
                {
                toReturn = *Msg->GetSubject() || *Msg->GetLinkedApplication()
                        ? TRUE : FALSE;
                }
            }
        }

    return (toReturn);
    }


// --------------------------------------------------------------------------
// matchString(): Searches for match to given string.
//
// Notes:
//  Runs backward through buffer so we get most recent match first.
//  Assumes MENU_EDIT is locked.

char *TERMWINDOWMEMBER matchString(char *buf, const char *pattern,
        char *bufEnd, int ver, Bool exact)
    {
    char *loc, *pc2;
    const char *pc1;
    Bool foundIt;

    for (loc = bufEnd, foundIt = FALSE; !foundIt && --loc >= buf;)
        {
        for (pc1 = pattern, pc2 = loc, foundIt = TRUE; *pc1 && foundIt;)
            {
            if (tolower(*pc1++) != tolower(*pc2++))
                {
                foundIt = FALSE;
                }
            }

        if (foundIt && exact)
            {
            if (
                (loc != buf && isalnum(loc[-1]))
                ||
                isalnum(loc[strlen(pattern)])
            )
                {
                foundIt = FALSE;
                }
            }

        if (ver && foundIt)
            {
            char subbuf[11];

            doCR();
            strncpy(subbuf, buf + 10 > loc ? buf : loc - 10,
                (uint) (loc - buf) > 10 ? 10 : (uint) (loc - buf));

            subbuf[(uint) (loc - buf) > 10 ? 10 : (uint) (loc - buf)] = 0;

            mPrintf(pcts, subbuf);

            if (TermCap->IsBoldAvailable())
                {
                termCap(TERM_BOLD);
                }
            else
                {
                mPrintf(getmsg(MENU_EDIT, 1));
                }

            mPrintf(pcts, pattern);

            if (TermCap->IsBoldAvailable())
                {
                termCap(TERM_NORMAL);
                }
            else
                {
                mPrintf(getmsg(MENU_EDIT, 46));
                }

            strncpy(subbuf, loc + strlen(pattern), 10);

            subbuf[10] = 0;
            mPrintfCR(pcts, subbuf);

            if (!getYesNo(getmsg(MENU_EDIT, 38), 0))
                {
                foundIt = FALSE;
                }
            }
        }

    return (foundIt ? loc : NULL);
    }


// --------------------------------------------------------------------------
// replaceString(): Lets user correct typos in message entry.
//
// Notes:
//  Assumes MENU_EDIT is locked.

void TERMWINDOWMEMBER replaceString(char *buf, uint lim, Bool ver, char *oldStr)
    {
    char oldString[260];
    char newString[260];
    char *loc, *textEnd;
    char *pc;
    int incr, length;

    // find terminal null
    for (textEnd = buf, length = 0; *textEnd; length++, textEnd++);

    if (oldStr)
        {
        strcpy(oldString, oldStr);
        }
    else
        {
        getString(getmsg(MENU_EDIT, 33), oldString, 256, FALSE, ns);
        }

    if (!*oldString)
        {
        return;
        }

    if (!oldStr)
        {
        if ((loc = matchString(buf, oldString, textEnd, ver, FALSE)) == NULL)
            {
            mPrintfCR(getmsg(MENU_EDIT, 34));
            return;
            }
        }

    if (!getString(oldStr ? getmsg(MENU_EDIT, 35) : getmsg(MENU_EDIT, 36),
            newString, 256, FALSE, ns))
        {
        return;
        }

    if (oldStr)
        {
        if (!*newString)
            {
            return;
            }

        char *Orig = strdup(oldString);
        char *New = strdup(newString);

        if (Orig && New)
            {
            stripansi(Orig);
            stripansi(New);
            strlwr(Orig);
            strlwr(New);

            char Prompt[256];
            sprintf(Prompt, getmsg(MENU_EDIT, 37), Orig, New);

            if (getYesNo(Prompt, 0))
                {
                if (!CurrentUser->AddReplace(Orig, New))
                    {
                    OutOfMemory(32);
                    }
                }
            }
        else
            {
            OutOfMemory(32);
            }

        delete [] Orig;
        delete [] New;
        }

    do
        {
        if (oldStr)
            {
            if ((loc = matchString(buf, oldString, textEnd, ver, TRUE)) == NULL)
                {
                return;
                }
            }

        for (textEnd = buf, length = 0; *textEnd; length++, textEnd++)
            {
            ;
            }

        if ((length + (strlen(newString) - strlen(oldString))) >= lim)
            {
            mPrintfCR(getmsg(432));
            return;
            }

        // delete old string
        for (pc = loc, incr = strlen(oldString); (*pc = *(pc + incr)) != 0;
                pc++);
        textEnd -= incr;

        // make room for new string
        for (pc = textEnd, incr = strlen(newString); pc >= loc; pc--)
            {
            *(pc + incr) = *pc;
            }

        // insert new string
        for (pc = newString; *pc; *loc++ = *pc++);

        textEnd = loc;
        } while (oldStr);
    }

// --------------------------------------------------------------------------
// wordcount(): Counts lines, words, and characters in a block of text, and
//  displays the numbers to the user.
//
// Input:
//  const char *buf: The text to count
//
// Notes:
//  Assumes MENU_EDIT is locked.

void TERMWINDOWMEMBER wordcount(const char *buf)
    {
    int lines = 0, words = 0;

    const char *counter = buf;
    const int chars = strlen(buf);

    while (*counter++)
        {
        if (*counter == ' ')
            {
            if ((*(counter - 1) != ' ') && (*(counter - 1) != '\n'))
                {
                words++;
                }
            }

         if (*counter == '\n')
            {
            if ((*(counter - 1) != ' ') && (*(counter - 1) != '\n'))
                {
                words++;
                }

            lines++;
            }
        }

    label Words, Chars;
    CopyStringToBuffer(Words, ltoac(words));
    CopyStringToBuffer(Chars, ltoac(chars));
    CRCRmPrintfCR(getmsg(MENU_EDIT, 39), ltoac(lines), lines == 1 ? ns : justs,
            Words, words == 1 ? ns : justs, Chars, chars == 1 ? ns : justs);
    }


// --------------------------------------------------------------------------
// GetFileMessage(): Gets a string from a file.
//
// Input:
//  FILE *fl: The file to read
//  char *str: Where to put the text
//  int mlen: Maximum length to read
//
// Output:
//  char *str: The read text
//
// Notes:
//  The string is considered to come to an end at a CR, 0xFF, or 0x00.

void GetFileMessage(FILE *fl, char *str, int mlen)
    {
    register int l = 0;

    while (!feof(fl) && l < mlen)
        {
        const char ch = (uchar) fgetc(fl);

        if (ch != '\r' && ch != '\xFF' && ch)
            {
            *str++ = ch;
            l++;
            }
        }

    *str = 0;
    }
