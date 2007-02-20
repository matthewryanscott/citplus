// --------------------------------------------------------------------------
// Citadel: DoMisc.CPP
//
// Miscellaneous commands

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#ifndef WINCIT
#include "msg.h"
#endif
#include "events.h"
#include "helpfile.h"
#include "account.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"

#ifdef WINCIT
#include "termwndw.h"
#include "net6969.h"

// andy debug
#include "telnetd.h"
#endif

// --------------------------------------------------------------------------
// Contents
//
// doHelp()             does it
// doIntro()            does it
// doLogout()           does it
// doVolkswagen()       does it
// doSmallChat()        does it
// doY()                does it
// doPoop()             does it
// doWow()              does it
// doReadBull()         does it


// --------------------------------------------------------------------------
// doVolkswagen(): Handles V(olkswagen) command.

void TERMWINDOWMEMBER doVolkswagen(Bool moreYet, const char *CmdKeys)
    {
    SetDoWhat(DOVOLKS);

    discardable *d;

    if (!moreYet)
        {
        mPrintf(getmsg(367));
        }
    else if ((d = readData(22)) != NULL)
        {
        Bool HitChars[38];  // A-Z 0-9 ESC CR
        Bool done;
        int theChar, NumBSAble = 0, CmdKeysIndex = 0;

        memset(HitChars, 0, sizeof(HitChars));

        mPrintf(getmsg(406));

        do
            {
            done = FALSE;

            if (CmdKeys && CmdKeys[CmdKeysIndex])
                {
                theChar = toupper(CmdKeys[CmdKeysIndex++]);
                }
            else
                {
                theChar = toupper(iCharNE());
                }

            if (isalnum(theChar) || theChar == ESC || theChar == '\n')
                {
                int hcIndex;

                if (isalpha(theChar))
                    {
                    hcIndex = theChar - 'A';
                    }
                else if (isdigit(theChar))
                    {
                    hcIndex = theChar - '0' + 26;
                    }
                else
                    {
                    hcIndex = theChar == ESC ? 36 : 37;
                    }

                if (!HitChars[hcIndex])
                    {
                    HitChars[hcIndex] = TRUE;

                    for (discardable *Cur = d; Cur; Cur = (discardable *) getNextLL(Cur))
                        {
                        const char **Text = (const char **) Cur->aux;

                        if (Text[0][0] == theChar)
                            {
                            done = !strchr(Text[0], 'c');

                            if (strchr(Text[0], 'b'))
                                {
                                while (NumBSAble--)
                                    {
                                    doBS();
                                    }

                                doBS();

                                NumBSAble = 0;
                                }

                            for (int Index = 1; Text[Index][0] != '#'; Index++)
                                {
                                mPrintf(Text[Index]);

                                if (!done)
                                    {
                                    NumBSAble += strlen(Text[Index]);
                                    }
                                }

                            break;
                            }
                        }
                    }
                }
            else if (theChar == '?')
                {
                oChar('?');
                showMenu(M_VOLKSWAGEN);
                done = TRUE;
                }
            } while (!done);

        if (theChar != '?')
            {
            doCR();
            }

        discardData(d);
        }
    else
        {
        OutOfMemory(38);
        }
    }


// --------------------------------------------------------------------------
// doHelp(): Handles H(elp) command.

void TERMWINDOWMEMBER doHelp(Bool expand, const char *HelpFile)
    {
    label fileName;

    SetDoWhat(HELPFILES);

    mPrintf(getmsg(263));

    if (!expand)
        {
        doCR();
        dispHlp(H_DOHELP);
        return;
        }

    if (HelpFile)
        {
        CopyStringToBuffer(fileName, HelpFile);
        }
    else
        {
        if (!getString(ns, fileName, LABELSIZE - 4, ns))
            {
            return;
            }
        }

    normalizeString(fileName);

    if (!*fileName)
        {
        dispHlp(H_DOHELP);
        return;
        }

    if (fileName[0] == '?')
        {
        ShowHelpMenu();
        }
    else if (SameString(fileName, getmsg(108)))
        {
        CRmPrintfCR(getmsg(109));
        }
    else
        {
//      strupr(fileName);

        Bool ShownInternal = FALSE;
        Bool WhichOnes[MAXHLP];

        for (HelpFiles theFile = (HelpFiles) 0; theFile < MAXHLP; theFile = (HelpFiles) (theFile + 1))
            {
            if (u_match(hlpnames[theFile], fileName))
                {
                dispHlp(theFile);
                WhichOnes[theFile] = ShownInternal = TRUE;

                if (OC.User.GetOutFlag() == OUTSKIP)
                    {
                    return;
                    }
                }
            else
                {
                WhichOnes[theFile] = FALSE;
                }
            }

        // adding the extention makes things look simpler for the user...
        // and restricts the files which can be read
        strcat(fileName, getmsg(411));

        changedir(cfg.helppath);
        directoryinfo *files = filldirectory(fileName, SORT_ALPHA, OldAndNew, FALSE);

        if ((files && *files[0].Name) || !ShownInternal)
            {
            delete [] files;
            files = NULL;

            tutorial(fileName, WhichOnes);
            }

        delete [] files;
        }
    }


// --------------------------------------------------------------------------
// doIntro(): handles Intro to .... command.

void TERMWINDOWMEMBER doIntro(void)
    {
    SetDoWhat(READINTRO);

    mPrintfCR(getmsg(264), cfg.nodeTitle);

    dispHlp(H_INTRO);
    }


// --------------------------------------------------------------------------
// doLogout(): Handles T(erminate) command.

void TERMWINDOWMEMBER doLogout(Bool expand, char first)
    {
    if (!LockMenu(MENU_TERM))
        {
        return;
        }

    Bool done = FALSE;

    SetDoWhat(DOLOGOUT);

    MRO.Verbose = FALSE;

    if (expand)
        {
        first = 0;
        }

    mPrintf(getmenumsg(MENU_TERM, 3));

    if (first == 'q')
        {
        MRO.Verbose = TRUE;
        }

    int OptionCounter = 0;

    while (!done && HaveConnectionToUser())
        {
        done = TRUE;

        if (!first)
            {
            if (++OptionCounter == 69)
                {
                mPrintf(getmsg(472), getmsg(69), spc);
                OptionCounter = 0;
                }
            }

        const int ich = first ? first : iCharNE();

        switch (DoMenu(MENU_TERM, ich))
            {
            case 1: // Quit also
                {
                DoCommand(UC_TERMINATEQUITALSO, !expand);
                break;
                }

            case 2:
                {
                DoCommand(UC_TERMINATESTAY);
                break;
                }

            case 3:
                {
                MRO.Verbose = 2;
                done = FALSE;
                break;
                }

            case -1:
                {
                break;
                }

            default:
                {
                oChar((char) ich);
                mPrintf(sqst);

                if (CurrentUser->IsExpert())
                    {
                    break;
                    }
                }

            case -2:
                {
                if (ich == '?')
                    {
                    oChar('?');
                    }

                SetDoWhat(TERMMENU);

                showMenu(M_TERMINATE);
                break;
                }
            }

        first = 0;
        }

    UnlockMenu(MENU_TERM);
    }


// --------------------------------------------------------------------------
// doXpert()

void TERMWINDOWMEMBER doXpert(void)
    {
    mPrintfCR(getmsg(315), CurrentUser->IsExpert() ? getmsg(316) : getmsg(317));
    CurrentUser->SetExpert(!CurrentUser->IsExpert());
    }


// --------------------------------------------------------------------------
// doVerbose()

void TERMWINDOWMEMBER doverbose(void)
    {
    mPrintfCR(getmsg(318), CurrentUser->IsAutoVerbose() ? getmsg(316) : getmsg(317));
    CurrentUser->SetAutoVerbose(!CurrentUser->IsAutoVerbose());
    }


// --------------------------------------------------------------------------
// doSmallChat(): Does it.

Bool TERMWINDOWMEMBER CheckDatForMore(const char *String)
    {
    discardable *d;
    Bool toReturn = FALSE;

    if ((d = readData(17, 0, 0)) != NULL)
        {
        const char **Data = (const char **) d->aux;

        for (int Index = 0; Data[Index][0] != '#'; Index++)
            {
            if (SameString(Data[Index], String))
                {
                discardData(d);

                if ((d = readData(17, Index + 1, Index + 1)) != NULL)
                    {
                    Data = (const char **) d->aux;

                    if (Data[0][0] == '*')
                        {
                        int RandomIndex = (int) random(atoi(Data[0] + 1));

                        int EntryIndex;
                        for (EntryIndex = 1; RandomIndex; RandomIndex--, EntryIndex++)
                            {
                            while (Data[EntryIndex + 1][0] != '!')
                                {
                                EntryIndex++;
                                }
                            }

                        doCR();
                        do
                            {
                            mFormat(Data[EntryIndex] + (Data[EntryIndex][0] == '!' ? 1 : 0));
                            } while (Data[++EntryIndex][0] != '!');

                        doCR();
                        }
                    else
                        {
                        Bool Repeat = FALSE;

                        do
                            {
                            doCR();

                            for (Index = 0; !SameString(Data[Index], getmsg(307)); Index++)
                                {
                                if (Data[Index][0] == '#')
                                    {
                                    Repeat = TRUE;
                                    }
                                else
                                    {
                                    mFormat(Data[Index]);
                                    mFormat(bn);
                                    }
                                }
                            } while (Repeat && OC.User.GetOutFlag() == OUTOK);

                        doCR();
                        }
                    discardData(d);
                    toReturn = TRUE;
                    }
                return (toReturn);
                }
            }

        discardData(d);
        }
    return (toReturn);
    }

void TERMWINDOWMEMBER doSmallChat(void)
    {
    Bool first = TRUE;
    SetDoWhat(DOSMALLCHAT);

    oChar(';');

    getNormStr(ns, ContextSensitiveScriptText, 255);

    if (*ContextSensitiveScriptText == ';')
        {
        const char *ChatText1 = ContextSensitiveScriptText + 1;     // skip ';'

        if (SameString(ChatText1, getmsg(305)))                     // ;;wow
            {
            mPrintf(getmsg(303), CurrentUser->IsProblem() ? '.' : '!');
            }
        else if (SameString(ChatText1, getmsg(302)))                // ;;Lobby
            {
            label LobbyName;
            RoomTab[LOBBY].GetName(LobbyName, sizeof(LobbyName));

            gotoRoom(LobbyName, FALSE, FALSE, FALSE);
            }
        else if (SameString(ChatText1, getmsg(10)))                 // ;;roman
            {
            CurrentUser->SetRoman(!CurrentUser->IsRoman());

            CRmPrintfCR(CurrentUser->IsRoman() ? getmsg(317) : getmsg(316));
            }
        else if (SameString(ChatText1, getmsg(301)))                // ;;mavencit
            {
            uint i = 0;

            CRmPrintf(getmsg(299));
            while (!BBSCharReady() && HaveConnectionToUser())
                {
#ifdef WINCIT
                if (!first) twirlypause(25);
#else
                if (!first) twirlypause(15);
#endif
                first = FALSE;

                CommPort->Output('\b');
                CommPort->FlushOutput();
                outConRawBs();

                CommPort->Output('\b');
                CommPort->FlushOutput();
                outConRawBs();

                CommPort->Output(getmsg(300)[i]);
                CommPort->FlushOutput();
                outCon(getmsg(300)[i]);

                CommPort->Output(']');
                CommPort->FlushOutput();
                outCon(']');

                if (++i == strlen(getmsg(300))) i = 0;

                if (CheckIdleUserTimeout(TRUE))
                    {
                    sleepkey = FALSE;
                    time(&LastActiveTime);

                    UserHasTimedOut();
                    }
                else
                    {
                    sleepkey = FALSE;
                    }
                }

            doCR();

            if (BBSCharReady())
                {
                iCharNE();
                }
            }
        else if (SameString(ChatText1, getmsg(5)))                // ;;idle
            {
            IdlePrompt = TRUE;
            CursorIsAtPrompt = FALSE;
            }
#if (VERSION != RELEASE)
        else if (SameString(ChatText1, "fmt"))
            {
            label l, m;
            getString("format string", l, LABELSIZE, FALSE, ns);
            const Bool premature = getYesNo("Premature", 0);
            const Bool inclPrompt = getYesNo("Include prompt", 0);
            doCR();
            getFmtString(l, m, premature, inclPrompt);
            mPrintfCR(pcts, m);
            }
        else if (SameString(ChatText1, "debug"))
            {
            if (debug)
                {
                TurnDebugOff();
                CRmPrintfCR("Debug mode is now: %s", getmsg(316));
                }
            else
                {
                if (TurnDebugOn())
                    {
                    CRmPrintfCR("Debug mode is now: %s", getmsg(317));
                    }
                else
                    {
                    CRmPrintfCR("Could not turn debug mode on.");
                    }
                }
            }
#ifdef WINCIT
        else if (SameString(ChatText1, "6969d"))
            {
            Net6969.Debug(this);
            }
#endif
        else if (SameString(ChatText1, "compact memory"))
            {
            const int Value = (int) getNumber("level", 0, 2, 0, FALSE, NULL);
            compactMemory(Value);
            doCR();
            }
#ifdef WINCIT
        else if (SameString(ChatText1, "system event"))
            {
            char Event[128];
            getNormStr("event text", Event, 127);

            TermWindowCollection.SystemEvent(SE_TEST, 0, NULL, FALSE, Event);
            }
#endif
        else if (SameString(ChatText1, "read messages"))
            {
            label l;
            getNormStr("messages to read", l, LABELSIZE);

            Bool res = FALSE;
            if (SameString(l, "net"))
                {
                res = read_net_messages();
                }
            else if (SameString(l, "sys"))
                {
                res = read_sys_messages();
                }
            else if (SameString(l, "cfg"))
                {
                res = read_cfg_messages();
                }
            else if (SameString(l, "ec"))
                {
                res = read_ec_messages();
                }
            else if (SameString(l, "fs"))
                {
                res = read_fs_messages();
                }
            else if (SameString(l, "cw"))
                {
                res = read_cw_messages();
                }
            else if (SameString(l, "zm"))
                {
                res = read_zm_messages();
                }

            if (res)
                {
                mPrintfCR("%s messages read.", l);
                }
            else
                {
                mPrintfCR("Could not read %s messages", l);
                }
            }
        else if (SameString(ChatText1, "dump messages"))
            {
            label l;
            getNormStr("messages to dump", l, LABELSIZE);

            Bool res = FALSE;
            if (SameString(l, "net"))
                {
                res = TRUE;
                dump_net_messages();
                }
            else if (SameString(l, "sys"))
                {
                res = TRUE;
                dump_sys_messages();
                }
            else if (SameString(l, "cfg"))
                {
                res = TRUE;
                dump_cfg_messages();
                }
            else if (SameString(l, "ec"))
                {
                res = TRUE;
                dump_ec_messages();
                }
            else if (SameString(l, "fs"))
                {
                res = TRUE;
                dump_fs_messages();
                }
            else if (SameString(l, "cw"))
                {
                res = TRUE;
                dump_cw_messages();
                }
            else if (SameString(l, "zm"))
                {
                res = TRUE;
                dump_zm_messages();
                }

            if (res)
                {
                mPrintfCR("%s messages dumped.", l);
                }
            else
                {
                mPrintfCR("What?");
                }
            }
    #if !defined(WINCIT)
        else if (SameString(ChatText1, "saver on"))
            {
            if (ScreenSaver.TurnOn())
                {
                pause(50);
                while (kb_hit());

                CRmPrintfCR("Screen saver is now on.");
                }
            else
                {
                CRmPrintfCR("Could not turn screen saver on.");
                }
            }
        else if (SameString(ChatText1, "saver off"))
            {
            ScreenSaver.TurnOff();

            CRmPrintfCR("Screen saver is now off.");
            }
        else if (SameString(ChatText1, "memory dump"))
            {
            label l;

            getNormStr("segment", l, LABELSIZE);
            const uint seg = (uint) strtoul(l, NULL, 16);

            getString("offset", l, LABELSIZE, FALSE, "0004");
            uint off = (uint) strtoul(l, NULL, 16);

            long len = getNumber("length", 0, UINT_MAX, 20 * 16, FALSE, NULL);

            for (; len > 0; len -= 16)
                {
                uchar val[16];

                for (int x = 0; x < 16; x++)
                    {
                    val[x] = *((uchar *) MK_FP(seg, off++));
                    }

// For stripping out control characters...
#define nc(a) (a < 32 ? 32 : a)

                CRmPrintf("%p  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x  "
                        "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", MK_FP(seg, off - 16), val[0], val[1], val[2], val[3], val[4],
                        val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15],
                        nc(val[0]), nc(val[1]), nc(val[2]), nc(val[3]), nc(val[4]), nc(val[5]), nc(val[6]), nc(val[7]),
                        nc(val[8]), nc(val[9]), nc(val[10]), nc(val[11]), nc(val[12]), nc(val[13]), nc(val[14]),
                        nc(val[15]));
                }

            doCR();
            }
    #endif
        else if (SameString(ChatText1, "memory"))
            {
            static void *ptr;

#ifdef VISUALC
            free(ptr);
#else
            farfree(ptr);
#endif
            ptr = NULL;

            const long memorysize = getNumber("amount (bytes)", 0, LONG_MAX, 0, FALSE, NULL);

            if (memorysize)
                {
        #ifndef HACKER
                #ifdef WINCIT
                    ptr = malloc(memorysize);
                #else
                    ptr = _fmalloc(memorysize);
                #endif
        #endif
                }

        #ifndef HACKER
            CRmPrintfCR("Memory %sallocated", ptr ? ns : "not ");
        #else
            CRmPrintfCR("Memory allocated");
            trap(T_HACK "Allocated %ld memory", memorysize);
            amPrintf("User %s allocated %ld memory\n", CurrentUser->GetName(), memorysize);
            SaveAideMess(NULL);
        #endif
            }
#endif
        else if (!CheckDatForMore(ChatText1))
            {
            doEvent(EVT_SMALLCHAT); // User configurable is least important
            }
        }
    }

void TERMWINDOWMEMBER doY(Bool backwards)
    {
    discardable *d = readData(6, 2, 2);

    if (d)
        {
        int HighestIndex = atoi(((const char **) d->aux)[0]) - 1;

        if (backwards)
            {
            Ycounter--;
            }
        else
            {
            Ycounter++;
            }

        if (Ycounter < 0)
            {
            Ycounter = HighestIndex;
            }
        else if (Ycounter > HighestIndex)
            {
            Ycounter = 0;
            }

        mPrintf(((const char **) d->aux)[Ycounter + 1]);

        discardData(d);
        }
    else
        {
        OutOfMemory(39);
        }
    }

Bool TERMWINDOWMEMBER doPoop(char c)
    {
    if (toupper(c) == 'P' || cfg.poop < 0)
        {
        if (c == 'P')
            {
            CurrentUser->AdjustPoopcount(cfg.poop < 0 ? 1 : 69);
            }
        else if (c == 'p')
            {
            CurrentUser->AdjustPoopcount(1);
            }
        else if (c == 'S')
            {
            CurrentUser->AdjustPoopcount(69000l);
            }
        else // s
            {
            CurrentUser->AdjustPoopcount(100);
            }

        if ((CurrentUser->GetPoopcount() > cfg.maxpoop) && loggedIn)
            {
            cfg.maxpoop = CurrentUser->GetPoopcount();
            CurrentUser->GetName(cfg.poopuser, sizeof(cfg.poopuser));
            }

        if ((CurrentUser->IsExpert() && (cfg.poop == 1)) || cfg.poop < 1)
            {
            if (CurrentUser->GetPoopcount() < 0)
                {
                mPrintf(toupper(c) == 'P' ? getmsg(363) : getmsg(364));
                }
            else
                {
                mPrintf(toupper(c) == 'P' ? getmsg(365) : getmsg(366));
                }
            }

        return (FALSE);
        }
    else
        {
        return (TRUE);
        }
    }

#define WHO_ALL   1
#define WHO_ROOM  2
#define WHO_GROUP 4
#define WHO_USER  8

void TERMWINDOWMEMBER doWow(Bool dot)
    {
#ifdef WINCIT
    if (!dot && cfg.poopwow)
        {
        wowcount++;

        if (wowcount < -2)
            {
            mPrintf(getmsg(285), ltoac(-1 * wowcount));

            if (wowcount == -69)
                {
                mPrintf(getmsg(472), spc, getmsg(69));
                }
            }
        else if (wowcount < 0)
            {
            if (wowcount == -2)
                {
                mPrintf(getmsg(284));
                }

            mPrintf(getmsg(283));
            }
        else if (wowcount == 0)
            {
            mPrintf(getmsg(282));
            }
        else if (wowcount > 2)
            {
            mPrintf(getmsg(281), ltoac(wowcount));

            if (wowcount == 69)
                {
                mPrintf(getmsg(472), spc, getmsg(69));
                }
            else if (wowcount == 120)
                {
                discardable *d = readData(6, 16, 16);

                if (d)
                    {
                    doCR(2);

                    for (int i = 0;; i++)
                        {
                        const char *String = ((const char **) d->aux)[i];

                        if (*String == '#')
                            {
                            break;
                            }

                        mPrintf(getmsg(472), String, bn);
                        }

                    doCR();

                    discardData(d);
                    }
                }
            else if (wowcount == 420)
                {
                discardable *b = readData(6, 17, 17);

                if (b)
                    {
                    doCR(2);

                    for (int i = 0;; i++)
                        {
                        const char *String = ((const char **) b->aux)[i];

                        if (*String == '#')
                            {
                            break;
                            }

                        mPrintf(getmsg(472), String, bn);
                        }

                    doCR();

                    discardData(b);
                    }
                }
            }
        else
            {
            if (wowcount == 2)
                {
                mPrintf(getmsg(284));
                }

            mPrintf(getmsg(280));
            }
        }
    else
        {

        // It's Who here...

        if (!LockMenu(MENU_WHO))
            {
            return;
            }

        SetDoWhat(DOWHO);
        OC.SetOutFlag(OUTOK);

        MRO.Verbose = FALSE;

        mPrintf(getmenumsg(MENU_WHO, 0));

        int OptionCounter = 0;
        int who_what = WHO_ALL;
        Bool wow = FALSE;

        for (Bool done = false; !done && HaveConnectionToUser();)
            {
            done = TRUE;

            int ich = 0;

            if (dot)
                {
                if (++OptionCounter == 69)
                    {
                    mPrintf(getmsg(472), getmsg(69), spc);
                    OptionCounter = 0;
                    }

                ich = iCharNE();
                }

            switch (dot ? DoMenu(MENU_WHO, ich) : 2)
                {
                // Verbose levels:
                //  0=not pressed, no auto verbose
                //  1=not pressed, auto verbose
                //  2=pressed once
                //  3=pressed twice
                //  4=etc.
                case 1: // Verbose
                    {
                    if (!MRO.Verbose)
                        {
                        MRO.Verbose = 1;
                        }

                    MRO.Verbose++;
                    done = FALSE;
                    break;
                    }
                case 3: // All
                    {
                    who_what = WHO_ALL;
                    done = FALSE;
                    break;
                    }
                case 4: // Group
                    {
                    who_what |= WHO_GROUP;
                    done = FALSE;
                    break;
                    }
                case 5: // Room
                    {
                    who_what |= WHO_ROOM;
                    done = FALSE;
                    break;
                    }

                case 6: // User
                    {
                    who_what |= WHO_USER;
                    done = FALSE;
                    break;
                    }

                case 2: // CR or LF
                    {
                    ulong NumberOfTermWindows = TermWindowCollection.Count();

                    Bool *RoomAccess = NULL;
                    Bool ShowDowhat = FALSE;
                    const char **dowhatmsgs;
                    g_slot GroupSlot;
                  	Bool wild = FALSE;

                    label who;
                    if (who_what & WHO_USER)
                        {
                        doCR();
					    wow = TRUE;
                        AskUserName(who, ns, cfg.Luser_nym, LABELSIZE, NULL, TRUE);

                        if (*who)
                            {
                            l_slot logNo = FindPersonByPartialName(who);

                            if (logNo == CERROR)
                                {
                     			if (strpos('?', who) || strpos('*', who) || strpos('[', who))
                     				{
                    				wild = TRUE;
                     				}
                        		else
                                    {
                                    mPrintfCR(getmsg(595), who);
                                    UnlockMenu(MENU_WHO);
                                    return;
									}
                                }
                            else 
                                {
                                LTab(logNo).GetName(who, sizeof(who));
								}
                            }
                        else
                            {
                            who_what &= ~WHO_USER;
                            }
                        }

                    if (who_what & WHO_GROUP)
                        {
                        label GroupName;

                        if (!wow) doCR();
					    wow = TRUE;
                        AskGroupName(GroupName, TRUE);

                        if (*GroupName)
                            {
                            GroupSlot = FindGroupByPartialName(GroupName, FALSE);

                            if (GroupSlot != CERROR)
                                {
                                label Buffer;
                                CRmPrintfCR("%s belonging to %s ""%s"".", cfg.Uusers_nym, cfg.Lgroup_nym,
                                    GroupData[GroupSlot].GetName(Buffer, sizeof(Buffer)));
                                }
                            else
                                {
                                CRmPrintfCR(getmsg(584), cfg.Lgroup_nym);
                                UnlockMenu(MENU_WHO);
                                return;
                                }
                            }
                        else
                            {
                            who_what &= ~WHO_GROUP;
                            CRmPrintfCR("%s belonging to any %s.", cfg.Uusers_nym, cfg.Lgroup_nym);
                            }
                        }

                    if (MRO.Verbose)
                        {
                        RoomAccess = new Bool[cfg.maxrooms];

                        if (RoomAccess)
                            {
                            theOtherAlgorithm(RoomAccess, thisHall, FALSE);
                            }

                        if (LockMessages(MSG_F4SCREEN))
                            {
                            ShowDowhat = TRUE;
                            // these two lines from STATUS.CPP. Should clean this up sometime somehow...
#define DOWHATOFFSET 40
                            dowhatmsgs = ((const char **) ExtMessages[MSG_F4SCREEN].Data->aux) + DOWHATOFFSET;
                            }
                        }

                    if (!wow)
                        {
                        doCR();
					    }

                    for (Bool first = TRUE; NumberOfTermWindows; NumberOfTermWindows--)
                        {
                        label TermID, User;
                        *TermID = 0; *User = 0;
                        Bool Us = FALSE;
                        const TermWindowC *Peek = TermWindowCollection.Peek(NumberOfTermWindows);

                        if (Peek)
                            {
                            Us = Peek == this;

                            CopyStringToBuffer(TermID, Peek->WindowCaption);
                            Peek->CurrentUser->GetName(User, sizeof(User));
                            }

                        // add: lock Peek so it doesn't go away while we're doing this stuff!

                        if (*TermID && (who_what == WHO_ALL) || (
                        (!(who_what & WHO_GROUP) || Peek->CurrentUser->IsInGroup(GroupSlot)) &&
                        (!(who_what & WHO_ROOM)  || (Peek->thisRoom == thisRoom)) &&
                        (!(who_what & WHO_USER)  ||  ((!wild && SameString(User, who)) || (wild && u_match(deansi(User), who))))
                         										)
                         )
                            {
                            if (MRO.Verbose)
                                {
                                if (!first)
                                    {
                                    doCR();
                                    }
                                CRmPrintf(getmenumsg(MENU_WHO, 2), strlen(cfg.Uuser_nym) - strlen(deansi(cfg.Uuser_nym)), ns, cfg.Uuser_nym,
                                    User, TermID, Us ? getmenumsg(MENU_WHO, 3) : ns);

                                char Buffer1[80], Buffer2[80];

                                if (Peek->CommPort->HaveConnection())
                                    {
                                    strftime(Buffer1, sizeof(Buffer1) - 1, CurrentUser->GetVerboseDateStamp(Buffer2,
                                        sizeof(Buffer2)), Peek->conntimestamp);

                                    CRmPrintf(getmenumsg(MENU_WHO, 4), getmenumsg(MENU_WHO, 5), Buffer1,
                                        diffstamp(Peek->conntimestamp));
                                    }

                                if (Peek->loggedIn)
                                    {
                                    strftime(Buffer1, sizeof(Buffer1) - 1, CurrentUser->GetVerboseDateStamp(Buffer2,
                                        sizeof(Buffer2)), Peek->logtimestamp);

                                    CRmPrintf(getmenumsg(MENU_WHO, 4), getmenumsg(MENU_WHO, 6), Buffer1,
                                        diffstamp(Peek->logtimestamp));
                                    }

                                strftime(Buffer1, sizeof(Buffer1) - 1,
                                    CurrentUser->GetVerboseDateStamp(Buffer2, sizeof(Buffer2)), Peek->LastActiveTime);

                                CRmPrintf(getmenumsg(MENU_WHO, 4), getmenumsg(MENU_WHO, 7), Buffer1,
                                    diffstamp(Peek->LastActiveTime));

                                r_slot ThatRoom = Peek->thisRoom;
                                label RoomName;

                                CRmPrintf(getmenumsg(MENU_WHO, 9), strlen(cfg.Uroom_nym) - strlen(deansi(cfg.Uroom_nym)), ns, cfg.Uroom_nym, (RoomAccess && RoomAccess[ThatRoom]) ?
                                    RoomTab[ThatRoom].GetName(RoomName, sizeof(RoomName)) : getmenumsg(MENU_WHO, 10));

                                if (ShowDowhat)
                                    {
                                    CRmPrintf(getmenumsg(MENU_WHO, 9), 0, ns, getmenumsg(MENU_WHO, 11), dowhatmsgs[Peek->PDoWhat]);
                                    }

                                if (MRO.Verbose > 2)
                                    {
                                    if (Peek->CommPort->GetType() == CT_TELNET)
                                        {
                                        TelnetPortC *TPeek = (TelnetPortC *) Peek->CommPort;

                                        CRmPrintf("Telnet info: InCommand=%d; Enabled: ", TPeek->InCommand);

                                        prtList(LIST_START);

                                        for (int i = 0; i < 256; i++)
                                            {
                                            if (TPeek->Enabled[i])
                                                {
                                                label wow;
                                                sprintf(wow, "%d", i);
                                                prtList(wow);
                                                }
                                            }

                                        prtList(LIST_END);

                                        CRmPrintf("LastInWasCR=%d; LastOutWasCR=%d; let255through=%d; getCharHasMutex=%d; TThreadRunning=%d; TBufSize=%d; InTBuf=%d",
                                            TPeek->LastInWasCR, TPeek->LastOutWasCR, TPeek->let255through, TPeek->getCharHasMutex, TPeek->TThreadRunning, TPeek->TBufSize, TPeek->InTBuf);

                                        CRmPrintf("Attempting to get TMutex (wait up to 2 seconds)...");
                                        DWORD WaitResult = WaitForSingleObject(TPeek->TMutex, 2000);

                                        switch (WaitResult)
                                            {
                                        case WAIT_FAILED: mPrintf("WAIT_FAILED"); break;
                                        case WAIT_ABANDONED: mPrintf("WAIT_ABANDONED"); break;
                                        case WAIT_OBJECT_0: mPrintf("WAIT_OBJECT_0"); break;
                                        case WAIT_TIMEOUT: mPrintf("WAIT_TIMEOUT"); break;
                                        default: mPrintf("WAIT_??? (%d)", WaitResult);
                                            }

                                        if (WaitResult == WAIT_OBJECT_0 || WaitResult == WAIT_ABANDONED)
                                            {
                                            ReleaseMutex(TPeek->TMutex);
                                            }
                                        }
                                    }
                                first = FALSE;
                                }
                            else
                                {
                                if (first)
                                    {
                                    CRmPrintf(getmenumsg(MENU_WHO, 12), ns, cfg.Uuser_nym, 31 - strlen(deansi(cfg.Uuser_nym)), ns, ns);
                                    }

                                label Loggedin;
                                CopyStringToBuffer(Loggedin, Peek->loggedIn ? diffstamp(Peek->logtimestamp) : getmsg(529));

                                CRmPrintf(getmenumsg(MENU_WHO, 13), TermID, User, 31 - strlen(deansi(User)), ns, Loggedin,
                                    diffstamp(Peek->LastActiveTime));
                                }
                            first = FALSE;
                            }
                        }

                    doCR();

                    if (RoomAccess)
                        {
                        delete [] RoomAccess;
                        }

                    if (ShowDowhat)
                        {
                        UnlockMessages(MSG_F4SCREEN);
                        }

                    break;
                    }

                case -1:
                    {
                    break;
                    }

                default:
                    {
                    oChar((char) ich);
                    mPrintf(sqst);

                    if (CurrentUser->IsExpert())
                        {
                        break;
                        }
                    }

                case -2:
                    {
                    if (ich == '?')
                        {
                        oChar('?');
                        }

                    SetDoWhat(WHOMENU);

                    showMenu(M_WHO);
                    break;
                    }
                }
            }

        UnlockMenu(MENU_WHO);
        }
#else
    if (dot)
        {
        mPrintfCR(getmsg(287));
        wowcount = GetNumberWithBlurb(getmsg(288), LONG_MIN, LONG_MAX, wowcount, B_WOWCOUNT);
        }
    else
        {
        wowcount++;

        if (wowcount < -2)
            {
            mPrintf(getmsg(285), ltoac(-1 * wowcount));

            if (wowcount == -69)
                {
                mPrintf(getmsg(472), spc, getmsg(69));
                }
            }
        else if (wowcount < 0)
            {
            if (wowcount == -2)
                {
                mPrintf(getmsg(284));
                }

            mPrintf(getmsg(283));
            }
        else if (wowcount == 0)
            {
            mPrintf(getmsg(282));
            }
        else if (wowcount > 2)
            {
            mPrintf(getmsg(281), ltoac(wowcount));

            if (wowcount == 69)
                {
                mPrintf(getmsg(472), spc, getmsg(69));
                }
            else if (wowcount == 120)
                {
                discardable *d = readData(6, 16, 16);

                if (d)
                    {
                    doCR(2);

                    for (int i = 0;; i++)
                        {
                        const char *String = ((const char **) d->aux)[i];

                        if (*String == '#')
                            {
                            break;
                            }

                        mPrintf(getmsg(472), String, bn);
                        }

                    doCR();

                    discardData(d);
                    }
                }
            else if (wowcount == 420)
                {
                discardable *b = readData(6, 17, 17);

                if (b)
                    {
                    doCR(2);

                    for (int i = 0;; i++)
                        {
                        const char *String = ((const char **) b->aux)[i];

                        if (*String == '#')
                            {
                            break;
                            }

                        mPrintf(getmsg(472), String, bn);
                        }

                    doCR();

                    discardData(b);
                    }
                }

            }
        else
            {
            if (wowcount == 2)
                {
                mPrintf(getmsg(284));
                }

            mPrintf(getmsg(280));
            }
        }
#endif
    }


// --------------------------------------------------------------------------
// doBorder(): print a border line.

void TERMWINDOWMEMBER doBorder(void)
    {
    assert(borders != NULL);

    if (!CurrentUser->IsViewBorders())
        {
        return;
        }

    if (++borderCounter == cfg.borderfreq)
        {
        int tries;

        borderCounter = 0;

        for (lastBorder = (lastBorder + 1) % cfg.maxborders, tries = 0; tries <= cfg.maxborders;
                lastBorder = (lastBorder + 1) % cfg.maxborders, tries++)
            {
            if (borders[lastBorder * (BORDERSIZE + 1)])
                {
                CRmPrintf(pcts, &(borders[lastBorder * (BORDERSIZE + 1)]));
                break;
                }
            }
        }
    }


// --------------------------------------------------------------------------
// FingerEdit()

void TERMWINDOWMEMBER FingerEdit(void)
    {
    if (LockMenu(MENU_FINGER))
        {
        doCR(2);
        SetDoWhat(FINGEREDIT);

        char *EditBuffer = new char[MAXTEXT];

        if (EditBuffer)
            {
            CurrentUser->GetFinger(EditBuffer, MAXTEXT);

            mPrintfCR(getmenumsg(MENU_FINGER, 6));
            doCR();

            if (getText(NULL, EditBuffer))
                {
                CurrentUser->SetFinger(EditBuffer);

                if (loggedIn)
                    {
                    label Buffer;
                    CurrentUser->LogExtensions::Save(ThisLog, CurrentUser->GetName(Buffer, sizeof(Buffer)), thisRoom);
                    }
                }

            delete [] EditBuffer;
            }
        else
            {
            OutOfMemory(40);
            }

        UnlockMenu(MENU_FINGER);
        }
    }


// --------------------------------------------------------------------------
// FingerList()

void TERMWINDOWMEMBER FingerList(Bool docr)
    {
    if (LockMenu(MENU_FINGER))
        {
        doCR();
        SetDoWhat(FINGERLIST);

        OC.SetOutFlag(OUTOK);

        if (docr)
            {
            CRmPrintfCR(getmenumsg(MENU_FINGER, 8), cfg.Uusers_nym);
            }
        else
            {
            mPrintfCR(getmenumsg(MENU_FINGER, 8), cfg.Uusers_nym);
            }

        prtList(LIST_START);

        for (l_slot logslot = 0; logslot < cfg.MAXLOGTAB &&
#ifdef MULTI
                !OC.CheckInput(FALSE, this); logslot++)
#else
                !OC.CheckInput(FALSE); logslot++)
#endif
            {
            if (LTab(logslot).IsInuse())
                {
                LogExtensions LE(0);

                LE.Load(LTab(logslot).GetLogIndex());

                label Buffer;   // Don't need the whole finger.
                if (LE.GetFinger(Buffer, sizeof(Buffer)))
                    {
                    prtList(LTab(logslot).GetName(Buffer, sizeof(Buffer)));
                    }
                }
            }

        prtList(LIST_END);

        UnlockMenu(MENU_FINGER);
        }
    }


// --------------------------------------------------------------------------
// FingerUser()

void TERMWINDOWMEMBER FingerUser(void)
    {
    doCR();
    SetDoWhat(FINGERVIEW);

    label Name;
    if (!AskUserName(Name, ns, cfg.Luser_nym, 31, NULL, FALSE, TRUE))
        {
        return;
        }

    const l_slot logslot = FindPersonByPartialName(Name);

    if (logslot == CERROR)
        {
        CRmPrintfCR(getmsg(595), Name);
        }
    else
        {
        LogExtensions LE(0);

        LE.Load(LTab(logslot).GetLogIndex());

        doCR();

        char *Buffer = new char[MAXTEXT];

        if (Buffer)
            {
            if (LE.GetFinger(Buffer, MAXTEXT))
                {
                mFormat(Buffer);
                }
            else
                {
                if (LockMenu(MENU_FINGER))
                    {
                    mPrintf(getmenumsg(MENU_FINGER, 9), LTab(logslot).GetName(Buffer, MAXTEXT));
                    UnlockMenu(MENU_FINGER);
                    }
                }

            delete [] Buffer;
            }
        else
            {
            OutOfMemory(114);
            }

        doCR();
        }
    }

void TERMWINDOWMEMBER doFinger(void)
    {
    if (!LockMenu(MENU_FINGER))
        {
        return;
        }

    SetDoWhat(FINGER);

    mPrintf(getmenumsg(MENU_FINGER, 4));

    const int ich = iCharNE();

    switch (DoMenu(MENU_FINGER, ich))
        {
        case 1: // Delete
            {
            DoCommand(UC_FINGERDELETE);
            break;
            }

        case 2:
            {
            DoCommand(UC_FINGEREDIT);
            break;
            }

        case 3:
            {
            DoCommand(UC_FINGERLIST);
            break;
            }

        case 4:
            {
            DoCommand(UC_FINGERUSER);
            break;
            }

        case -1:
            {
            break;
            }

        default:
            {
            oChar((char) ich);
            mPrintf(sqst);

            if (CurrentUser->IsExpert())
                {
                break;
                }
            }

        case -2:
            {
            if (ich == '?')
                {
                oChar('?');
                }

            SetDoWhat(FINGERMENU);

            showMenu(M_FINGER);
            break;
            }
        }

    UnlockMenu(MENU_FINGER);
    }

void TERMWINDOWMEMBER doReadBull(Bool FromRoomPrompt)
    {
    if (FromRoomPrompt)
        {
        mPrintfCR(getmsg(23));
        }

    long bmenuOff;
    label menuFile;

    if ((bmenuOff = getMnuOff(M_BULLETINS, menuFile)) > 0)
        {
        doCR();

        changedir(cfg.helppath);

        FILE *fl;

        if ((fl = fopen(menuFile, FO_R)) != NULL)
            {
            int ich = '?';
            Bool found;

            do
                {
                found = ich == '?';
                OC.SetOutFlag(OUTOK);

                fseek(fl, bmenuOff, SEEK_SET);

                if (ich == '?')
                    {
                    char line[MAXWORDLEN];

                    while (fgets(line, MAXWORDLEN, fl) && (OC.User.GetOutFlag() != OUTNEXT) &&
                            (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
                            !OC.CheckInput(FALSE, this) &&
#else
                            !OC.CheckInput(FALSE) &&
#endif
                            line[0] != '#')
                        {
                        if (line[0] != '!')
                            {
                            mFormat(line);
                            }
                        else
                            {
                            char *words[256];

                            parse_it(words, line);
                            if (atoi(words[2]))
                                {
                                CRmPrintf(getmsg(274), toupper(words[1][0]), words[3]);
                                }
                            }
                        }

                    ResetOutputControl();
                    }
                else
                    {
                    char line[MAXWORDLEN];

                    while (fgets(line, MAXWORDLEN, fl) && line[0] != '#')
                        {
                        if (line[0] == '!')
                            {
                            char *words[256];

                            parse_it(words, line);
                            if (toupper(words[1][0]) == ich)
                                {
                                if (!CurrentUser->IsExpert())
                                    {
                                    CRmPrintfCR(getmsg(426));
                                    }

                                dumpf(words[0] + 1, TRUE, FALSE);
                                ResetOutputControl();
                                found = TRUE;
                                }
                            }
                        }
                    }

                doCR();

                if (!found)
                    {
                    CRmPrintfCR(getmsg(675));
                    }

                numLines = 0;
                CRmPrintf(getmsg(676));

                ich = toupper(iCharNE());
                if (ich == '?')
                    {
                    doCR();
                    }
                } while (ich != '\r' && ich != '\n' && (HaveConnectionToUser()));

            fclose(fl);
            }
        }
    else
        {
        dispBlb(B_BULLETIN);
        doCR();
        }

    changedir(cfg.homepath);
    }


void TERMWINDOWMEMBER TerminateQuitAlso(Bool Ask)
    {
    SetDoWhat(TERMQUIT);

    doCR();

    if (Ask)
        {
        if (!getYesNo(getmsg(57), 0))
            {
            return;
            }
        }

    if (!HaveConnectionToUser())
        {
        return;
        }

    terminate(TRUE);

    if (slv_door)
        {
        ExitToMsdos = TRUE;
        }
    }


void TERMWINDOWMEMBER TerminateStay(void)
    {
    SetDoWhat(TERMSTAY);

    doCR();

    terminate(!modStat && !slv_door);
    }


void TERMWINDOWMEMBER FingerDelete(void)
    {
    if (LockMenu(MENU_FINGER))
        {
        doCR();

        label Buffer;   // We don't need the whole thing.
        if (CurrentUser->GetFinger(Buffer, sizeof(Buffer)))
            {
            if (getYesNo(getmenumsg(MENU_FINGER, 5), 0))
                {
                CurrentUser->SetFinger(ns);
                }
            }

        UnlockMenu(MENU_FINGER);
        }
    }
