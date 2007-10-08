// --------------------------------------------------------------------------
// Citadel: ConOvl.CPP
//
// This handles the console on IBM PC and compatible computers. This code
// will probably need heavy modification for use with other computers.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "cwindows.h"
#include "scrlback.h"
#include "cfgfiles.h"
#include "config.h"
#include "miscovl.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"
#include "termwndw.h"


// --------------------------------------------------------------------------
// Contents
//
// fkey()           Deals with function keys from console
// outConRawBs()    BS to console, no overwrite
// cPrintf          send formatted output to console

#ifdef WINCIT
Bool ConsoleLockC::IsLocked(Bool)
    {
    return (FALSE);
    }
#else
Bool ConsoleLockC::IsLocked(Bool CheckingF6)
    {
    if (onConsole && CurrentUser && CurrentUser->IsSuperSysop())
        {
        return (FALSE);
        }
    else if (LockState == CON_UNLOCKED)
        {
        return (FALSE);
        }
    else if (LockState == CON_LOCKED)
        {
        return (TRUE);
        }
    else
        {
        assert(LockState == CON_F6LOCKED);
        return (CheckingF6);
        }
    }
#endif


Bool ConsoleLockC::MayUnlock(void)
    {
    return (LockState == CON_LOCKED &&
			!(SameString(cfg.f6pass, getmsg(670)) || SameString(cfg.f6pass, getmsg(MSG_CONSOLE, 36))));
    }

ConsoleLockC ConsoleLock;


// --------------------------------------------------------------------------
// GetSysPWHandler()
//
// Notes:
//  MSG_CONSOLE is almost guaranteed to be already loaded into memory, so
//  the LockMessages() call is almost guaranteed to work, and work quickly.

void GetSysPWHandler(const char *PW)
    {
    if (LockMessages(MSG_CONSOLE))
        {
        if (SameString(PW, cfg.f6pass))
            {
            ConsoleLock.Unlock();
            ScreenSaver.Update();

            if (cfg.VerboseConsole)
                {
                CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 40));
                }
            }
        else
            {
            if (cfg.VerboseConsole)
                {
                CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 39));
                }
            }
        }
    else
        {
        cOutOfMemory(104);
        }
    }


// --------------------------------------------------------------------------
// Assumes MSG_CONSOLE is locked.

void TERMWINDOWMEMBER StartPrinting(const char *FileName)
    {
    if (OC.Printing)
        {
        fclose(OC.PrintFile);
        }

#ifdef WINCIT
	sprintf(OC.PrintfileName, "%s.%ld", FileName, GetCurrentThreadId());
#else
	CopyStringToBuffer(OC.PrintfileName, FileName);
#endif

    normalizeString(OC.PrintfileName);

    if (citOpen(OC.PrintfileName, CO_A, &OC.PrintFile))
        {
        if (cfg.VerboseConsole)
            {
            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 37));
            }

        OC.Printing = TRUE;
        }
    else
        {
        if (cfg.VerboseConsole)
            {
            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 38));
            }

        OC.Printing = FALSE;
        }
    }

#ifndef WINCIT
static void ExitYNHandler(int Result)
    {
    ExitToMsdos = Result;
    }
#endif

// --------------------------------------------------------------------------
// fkey(): Deals with function keys from console.
//
// Input:
//  inkey: the high byte is the scan code returned by the IBM PC BIOS.  The low byte is ignored.
//
// Return value:
//  None.

void TERMWINDOWMEMBER fkey(uint inkey)
    {
    int key = inkey >> 8;

    // this handles the keys for dialout mode
    if (DoWhat == DIALOUT)
        {
        dialout_fkey = key;
        return;
        }

    // let RLM take over keyboard if it wants
    if (!CitadelIsConfiguring && rlmEvent(LT_KEYSTROKE, key, inkey))
        {
        return;
        }

    if (cfg.FullConLock && ConsoleLock.IsLocked() && ScreenSaver.IsOn() &&
            key != ALT_L)
        {
        return;
        }

    if (!LockMessages(MSG_CONSOLE))
        {
        cOutOfMemory(49);
        }

    if (key == CTL_F2)      // CTRL_F2 is never locked.
        {
#ifndef WINCIT
        if (allWindows)
            {
            setFocus(allWindows->wnd);
            }
#endif
        }
    else if (key != ALT_F1 && ConsoleLock.IsLocked(key == F6 || key == ALT_F6 || key == SFT_F6 || key == CTL_F6))
        {
#ifndef WINCIT
        if (key == ALT_L && ConsoleLock.MayUnlock())
            {
            GetStringFromConsole(getmsg(MSG_CONSOLE, 0), LABELSIZE, ns, GetSysPWHandler, TRUE);
            }
        else
            {
            if (cfg.VerboseConsole)
                {
                CitWindowsNote(NULL, (key == ALT_L) ? getmsg(MSG_CONSOLE, 1) : getmsg(MSG_CONSOLE, 2));
                }
            }
#endif
        }
    else
        {
        // Console is not locked

        if (CitadelIsConfiguring)
            {
            // Only let certain keystrokes through if configuring
            if (key != CTL_F6 && key != F7 && key != F8 && key != ALT_D && key != ALT_L && key != ALT_M &&
                    key != ALT_S && key != ALT_U)
                {
                WaitUntilDoneConfiguring();
                key = 0;
                }
            }

        ScreenSaver.Update();
        switch (key)
            {
            case F1:
                {
                CITWINDOW *w = CitWindowsMsg(NULL, getmsg(MSG_CONSOLE, 3));

#ifndef WINCIT
                CommPort->DropDtr();
#endif

                if (w)
                    {
                    destroyCitWindow(w, FALSE);
                    }

                break;
                }

#ifndef WINCIT
            case CTL_F1:
                {
                if (say_ascii != (int (cdecl *)(const char *, int)) nullFunc)
                    {
                    cfg.speechOn = !cfg.speechOn;

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 4), cfg.speechOn ? getmsg(317) : getmsg(316));
                        }
                    }
                else
                    {
                    cfg.speechOn = FALSE;

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 5));
                        }
                    }

                break;
                }
#endif

            case ALT_F1:
                {
                cls(SCROLL_SAVE);
                break;
                }

            case F2:
                {
                CITWINDOW *w = CitWindowsMsg(NULL, getmsg(19));

                Initport();

                if (w)
                    {
                    destroyCitWindow(w, FALSE);
                    }

                break;
                }

            case F3:
                {
                sysReq = !sysReq;

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 6), sysReq ? getmsg(317) : getmsg(316));
                    }

                break;
                }

            case ALT_F3:
                {
                if (sysReq)
                    {
                    altF3Timeout = 0;
                    sysReq = FALSE;

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 6), getmsg(316));
                        }
                    }
                else
                    {
                    altF3Timeout = time(NULL) + cfg.altF3Time;
                    sysReq = TRUE;

                    if (cfg.VerboseConsole)
                        {
                        long tme = time(NULL);

                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 7), (altF3Timeout - tme) / 60, (altF3Timeout - tme) % 60);
                        }
                    }

                break;
                }

            case F4:
                {
                if (!StatusLine.ToggleFullScreen(WC_TWp))
                    {
                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 8));
                        }
                    }

                break;
                }

            case F5:
                {
                if (onConsole)
                    {
                    OC.whichIO = MODEM;
                    OC.setio();

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 9), getmsg(MSG_CONSOLE, 10));
                        }
                    }
                else
                    {
                    OC.whichIO = CONSOLE;
                    OC.setio();

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 9), getmsg(MSG_CONSOLE, 11));
                        }
                    }

                break;
                }

            case F6:
                {
                sysopkey = TRUE;
                break;
                }

#ifndef WINCIT
            case CTL_F6:
                {
                doConsoleSysop();
                break;
                }
#endif

            case ALT_F6:
                {
                CurrentUser->SetAide(!CurrentUser->IsAide());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 12),
							CurrentUser->IsAide() ? getmsg(MSG_CONSOLE, 13) : getmsg(MSG_CONSOLE, 14), getmsg(385));
                    }

                break;
                }

            case SFT_F6:
                {
                CurrentUser->SetSysop(!CurrentUser->IsSysop());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 12),
                            CurrentUser->IsSysop() ? getmsg(MSG_CONSOLE, 13) : getmsg(MSG_CONSOLE, 14), getmsg(386));
                    }

                break;
                }

            case F7:
                {
                cfg.noBells++;

                if (cfg.noBells == 3)
                    {
                    cfg.noBells = 0;
                    }

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 15), getmsg(MSG_CONSOLE, 16 + cfg.noBells));
                    }

                break;
                }

            case ALT_C:
            case F8:
                {
                chatkey = !chatkey;
                break;
                }

            case F9:
                {
                cfg.noChat = !cfg.noChat;
                chatReq = FALSE;

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 19), cfg.noChat ? getmsg(316) : getmsg(317));
                    }

                break;
                }

            case F10:
            case ALT_H:
                {
                StatusLine.ToggleHelp(WC_TWp);
                break;
                }

            case ALT_F10:
                {
                StatusLine.ToggleSecond(WC_TWp);
                break;
                }

            case SFT_F10:
                {
                StatusLine.Toggle(WC_TWp);
                break;
                }

#ifdef WINCIT
            case F11:
                {
				TermWindowCollection.FocusPrev();
                break;
                }

            case F12:
                {
				TermWindowCollection.FocusNext();
                break;
                }
#endif

            case ALT_A:
                {
                CurrentUser->SetAccounting(!CurrentUser->IsAccounting());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 20), CurrentUser->IsAccounting() ? getmsg(317) : getmsg(316));
                    }

                break;
                }

            case ALT_D:
                {
                if (debug)
                    {
                    TurnDebugOff();

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 21), getmsg(316));
                        }
                    }
                else
                    {
                    if (TurnDebugOn())
                        {
                        if (cfg.VerboseConsole)
                            {
                            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 21), getmsg(317));
                            }
                        }
                    else
                        {
                        if (cfg.VerboseConsole)
                            {
                            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 22));
                            }
                        }
                    }

                break;
                }

#ifndef WINCIT
            case ALT_K:
                {
                if (!allWindows)
                    {
                    if (cfg.scrollSize)
                        {
                        DisplayScrollBackBuffer();
                        time(&LastActiveTime);
                        }
                    else
                        {
                        if (cfg.VerboseConsole)
                            {
                            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 23), citfiles[C_CONFIG_CIT]);
                            }
                        }
                    }
                else
                    {
                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 24));
                        }
                    }

                break;
                }
#endif

            case ALT_L:
                {
                if (*cfg.f6pass)
                    {
                    if (!SameString(cfg.f6pass, getmsg(670)))
                        {
                        ConsoleLock.Lock();

                        if (cfg.VerboseConsole)
                            {
                            CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 32));
                            }
                        }
                    }
                else
                    {
                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 33), citfiles[C_CONFIG_CIT]);
                        }
                    }

                break;
                }

            case ALT_M:
                {
                memoryDump();
                break;
                }

            case ALT_P:
                {
                if (OC.Printing)
                    {
                    OC.Printing = FALSE;
                    fclose(OC.PrintFile);

                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 25));
                        }
                    }
                else
                    {
#ifndef WINCIT
                    if (cfg.printerprompt)
                        {
                        GetStringFromConsole(getmsg(289), 63, cfg.printer, StartPrinting, FALSE);
                        }
                    else
#endif
                        {
                        char Buffer[sizeof(cfg.homepath) + sizeof(cfg.printer)];
                        sprintf(Buffer, sbs, cfg.homepath, cfg.printer);

                        StartPrinting(Buffer);
                        }
                    }

                break;
                }

            case ALT_R:
                repeatevent = TRUE;
                // fall through

            case ALT_F:
                forceevent = TRUE;
                // fall through

            case ALT_E:
                eventkey = TRUE;
                break;


            case ALT_S:
                {
                if (ScreenSaver.TurnOn())
                    {
#ifndef WINCIT
                    while ((*sp_press)());
#endif
                    }
                else
                    {
                    if (cfg.VerboseConsole)
                        {
                        CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 26));
                        }
                    }

                break;
                }

            case ALT_T:
                {
                CurrentUser->SetProblem(!CurrentUser->IsProblem());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 12),
                            CurrentUser->IsProblem() ? getmsg(MSG_CONSOLE, 13) : getmsg(MSG_CONSOLE, 14),
							getmsg(MSG_CONSOLE, 27));
                    }

                break;
                }

            case ALT_U:
                {
                cfg.ignore_uptime = !cfg.ignore_uptime;

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 28),
							citfiles[C_CONFIG_CIT], cfg.ignore_uptime ? getmsg(317) : getmsg(316));
                    }

                break;
                }

            case ALT_V:
                {
                CurrentUser->SetVerified(!CurrentUser->IsVerified());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 12),
							CurrentUser->IsVerified() ? getmsg(MSG_CONSOLE, 13) : getmsg(MSG_CONSOLE, 14),
							getmsg(MSG_CONSOLE, 29));
                    }

                break;
                }

            case ALT_X:
            case ALT_F4:
                {
#ifndef WINCIT
                if (cfg.AltXPrompt)
                    {
                    GetYNFromConsole(getmsg(MSG_CONSOLE, 30), 0, ExitYNHandler);
                    }
                else
#endif
                    {
                    ExitToMsdos = TRUE;
                    }

                break;
                }

            case ALT_Z:
                {
                if (CommPort->HaveConnection() || debug)
                    {
                    sleepkey = TRUE;
                    }

                break;
                }

            case ALT_3:
                {
                CurrentUser->SetOut300(!CurrentUser->IsOut300());

                if (cfg.VerboseConsole)
                    {
                    CitWindowsNote(NULL, getmsg(MSG_CONSOLE, 12),
                            CurrentUser->IsOut300() ? getmsg(MSG_CONSOLE, 13) : getmsg(MSG_CONSOLE, 14),
                            getmsg(MSG_CONSOLE, 31));
                    }

                break;
                }

            case PGUP:
                {
                CurrentUser->SetCredits(CurrentUser->GetCredits() + 5 * 60);
                break;
                }

            case PGDN:
                {
                CurrentUser->SetCredits(CurrentUser->GetCredits() - 5 * 60);
                break;
                }
            }
        }

    UnlockMessages(MSG_CONSOLE);

    StatusLine.Update(WC_TWp);
    }

#ifdef WINCIT
void TermWindowCollectionC::FocusPrev(void)
	{
	Lock();

	TermWindowListS *cur, *Last = (TermWindowListS *) getLLNum(List, getLLCount(List));

	for (cur = List; cur; Last = cur, cur = cur->next)
		{
		if (cur->T == Focus)
			{
			break;
			}
		}

	if (Last)
		{
		if (Last->T)
			{
			SetFocus(Last->T);
			}
		}
	else if (List && List->T)
		{
		SetFocus(cur->T);
		}

	Unlock();
	}

void TermWindowCollectionC::FocusNext(void)
	{
	Lock();

	TermWindowListS *cur;
	for (cur = List; cur; cur = cur->next)
		{
		if (cur->T == Focus)
			{
			cur = cur->next;
			break;
			}
		}

	if (cur)
		{
		if (cur->T)
			{
			SetFocus(cur->T);
			}
		}
	else if (List && List->T)
		{
		SetFocus(List->T);
		}

	Unlock();
	}




// --------------------------------------------------------------------------
// outConRawBs(): Send BS to console, no overwrite.
//
// Input:
//  None.
//
// Output:
//  None.

void TERMWINDOWMEMBER outConRawBs(void)
    {
    if (logiCol == 0 && prevChar != 10)
        {
        logiRow--;
        logiCol = conCols;
        }

    position(logiRow, logiCol - 1);
    OC.CrtColumn--;
    }


// --------------------------------------------------------------------------
// cPrintf(): outputs formatted text to console.
//
// Input:
//  const char *fmt: Formatting string, as in the C RTL's ...printf()
//      functions.
//
//  ...: Anything needed by fmt.
//
// Return value:
//  None.

void cdecl TERMWINDOWMEMBER cOutput(const char *fmt)
    {
    // for screenblanker
    if (!oPhys)
        {
        kb_hit();
        }

    const Bool oldcolor = TermCap->IsColor();
    TermCap->SetColor(TRUE);

    outSpeech(FALSE, fmt);

    register const char *buf = fmt;
    while (*buf)
        {
        if (*buf == CTRL_A)
            {
            uchar wordBuf[MAXWORDLEN + 8];
            int i;

            i = getWord(wordBuf, (uchar *) buf, 0, MAXWORDLEN);

            const Bool oldmodem = OC.Modem;
            OC.Modem = FALSE;
            termCap(buf + 1);
            OC.Modem = oldmodem;
            buf += i;
            }
        else
            {
            outCon(*buf++);
            }
        }

    TermCap->SetColor(oldcolor);
    ansi(14);
    OC.MCI_goto = FALSE;
    }

void cdecl TERMWINDOWMEMBER cPrintf(const char *fmt, ...)
    {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(OC.prtf_buff, fmt, ap);
    va_end(ap);

    cOutput(OC.prtf_buff);
    }

#endif
