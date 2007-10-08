// --------------------------------------------------------------------------
// Citadel: InpOvl.CPP
//
// Overlayed input functions

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "cwindows.h"
#include "helpfile.h"
#include "miscovl.h"
#include "term.h"


// --------------------------------------------------------------------------
// Contents
//
// getNormStr()     gets a string and normalizes it. No default.
// getNumber()      Get a number in range (top, bottom)
// GetOneKey()      Asks a question with a single-key answer
// getYesNo()       Gets a yes/no/abort or the default
// getString()      gets a string from user w/ prompt & default, ext.


long TERMWINDOWMEMBER GetNumberWithBlurb(const char *prompt, long min, long max, long dfault, BlurbFiles Blb)
    {
    long value;

    Bool ShowBlb;
    do
        {
        ShowBlb = FALSE;

        value = getNumber(prompt, min, max, dfault, TRUE, &ShowBlb);

        if (ShowBlb)
            {
            dispBlb(Blb);
            }
        } while (ShowBlb);

    return (value);
    }

Bool TERMWINDOWMEMBER GetStringWithBlurb(const char *prompt, char *buf, int len, const char *dfault, BlurbFiles Blb)
    {
    do
        {
        if (!getString(prompt, buf, len, TRUE, dfault))
            {
            return (FALSE);
            }

        if (*buf == '?')
            {
            dispBlb(Blb);
            }
        } while (*buf == '?');

    return (TRUE);
    }


// --------------------------------------------------------------------------
// getNormStr(): Gets a string and normalizes it. No default.

Bool TERMWINDOWMEMBER getNormStr(const char *prompt, char *s, int size)
    {
    const Bool toRet = getString(prompt, s, size, FALSE, ns);
    normalizeString(s);
    return (toRet);
    }

Bool TERMWINDOWMEMBER getNormStr(const char *prompt, char *s, int size,
        Bool doEcho)
    {
    const Bool toRet = getString(prompt, s, size, FALSE, doEcho, ns);
    normalizeString(s);
    return (toRet);
    }


// --------------------------------------------------------------------------
// getNumber(): Get a number in range (top, bottom)

long TERMWINDOWMEMBER getNumber(const char *prompt, long bottom, long top, long dfaultnum, Bool QuestIsSpecial,
		Bool *QuestionEntered)
    {
    if (QuestionEntered)
        {
        *QuestionEntered = FALSE;
        }

    long UserInput;
    char dfault[80], numstring[80];

    if (dfaultnum == -1l)
        {
        *dfault = 0;
        }
    else
        {
        CopyStringToBuffer(dfault, ltoac(dfaultnum));
        }

    do
        {
        getString(prompt, numstring, sizeof(numstring) - 1, QuestIsSpecial, dfault);

        normalizeString(numstring);

        if (QuestIsSpecial && (*numstring == '?'))
            {
            if (QuestionEntered)
                {
                *QuestionEntered = TRUE;
                }

            return (CERROR);
            }

        if (*numstring)
            {
            if (isdigit(*numstring) || *numstring == '-')
                {
                UserInput = strtol(numstring, NULL, (strncmpi(numstring, getmsg(51),
						strlen(getmsg(51))) == SAMESTRING) ? 16 : 10);
                }
            else
                {
                UserInput = RomanInput(numstring);
                }

            if (UserInput < bottom)
                {
                CRmPrintfCR(getmsg(499), ltoac(bottom));
                }

            if (UserInput > top)
                {
                CRmPrintfCR(getmsg(494), ltoac(top));
                }
            }
        else
            {
            UserInput = dfaultnum;
            }

        if (UserInput == 69)
            {
            CRmPrintfCR(getmsg(69));
            }
        } while ((UserInput < bottom || UserInput > top) && HaveConnectionToUser() && *numstring);

    return (UserInput);
    }


// --------------------------------------------------------------------------
// getString(): Gets a string from user w/ prompt & default, etc.
// char *prompt;            Enter "prompt"
// char *buf;               Where to put it
// int  lim;                max # chars to read
// Bool doEcho;             To echo, or not to echo, that is the question
// Bool QuestIsSpecial;     Return immediately on '?' input
// char *dfault;            Default for the lazy.

Bool TERMWINDOWMEMBER getString(const char *prompt, char *buf, int lim, const char *dfault)
    {
    return (getString(prompt, buf, lim, TRUE, TRUE, dfault));
    }

Bool TERMWINDOWMEMBER getString(const char *prompt, char *buf, int lim, Bool QuestIsSpecial, const char *dfault)
    {
    return (getString(prompt, buf, lim, QuestIsSpecial, TRUE, dfault));
    }

Bool TERMWINDOWMEMBER getString(const char *prompt, char *buf, int lim, Bool QuestIsSpecial, Bool doEcho, const char *dfault)
    {
    char c;
    int errors = 0;
    int i;
    const EchoType oldEcho = OC.Echo;
    const dowhattype oldDowhat = DoWhat;
    int termPos = 0;
    label tempTerm;
    Bool useDefault = (dfault && *dfault);

    ansi(14);

    if (!HaveConnectionToUser())
        {
        buf[0] = 0;

        OC.MCI_goto = FALSE;

        return (FALSE);
        }

    Bool ToReturn = TRUE;
    OC.SetOutFlag(IMPERVIOUS);

    if (*prompt)
        {
        if (useDefault)
            {
            sprintf(gprompt, getmsg(493), prompt, hasansi(prompt) ? getmsg(214) : ns, hasansi(dfault) ? getmsg(213) : ns,
					dfault, hasansi(dfault) ? getmsg(214) : ns);
            }
        else
            {
            sprintf(gprompt, getmsg(489), prompt);
            }

        CRmPrintf(pcts, gprompt);

        SetDoWhat(PROMPT);
        }

    tempTerm[0] = 0;

    OC.Echo = NEITHER;
    OC.setio();

    if (!doEcho)
        {
        if (!cfg.nopwecho)
            {
            OC.EchoChar = 1;
            }
        else if (cfg.nopwecho == 1)
            {
            OC.EchoChar = 0;
            }
        else
            {
            OC.EchoChar = cfg.nopwecho;
            }
        }

    i = 0;

    for (c = (char) iChar(); c != '\n' && HaveConnectionToUser(); c = (char) iChar())
        {
        OC.SetOutFlag(OUTOK);

        if (c == 0 || c == CTRL_D)
            {
            if (!HaveConnectionToUser())
                {
                break;
                }
            }
        else if (c == ESC)
            {
            i = 0;
            useDefault = FALSE;
            ToReturn = FALSE;
            break;
            }
        else if (termPos)
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
                tempTerm[termPos++] = c;
                tempTerm[termPos] = 0;

                if (c == 'X')
                    {
                    if (i + termPos >= lim)
                        {
                        oChar(BELL);
                        }
                    else
                        {
                        buf[i] = 0;
                        strcat(buf, tempTerm);
                        i += termPos;
                        termCap(tempTerm + 1);
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
        else if (c == '\b')
            {
            if (i != 0)
                {
                OC.Echo = oldEcho;
                OC.setio();

                doBS();

                OC.Echo = NEITHER;
                OC.setio();

                i--;

                if ((OC.EchoChar >= '0') && (OC.EchoChar <= '9'))
                    {
                    OC.EchoChar--;
                    if (OC.EchoChar < '0') OC.EchoChar = '9';
                    }
                }
            else
                {
                OC.Echo = oldEcho;
                OC.setio();

                oChar(BELL);

                OC.Echo = NEITHER;
                OC.setio();
                }
            }
        else
            {
            if (c == CTRL_A && (i < lim - 1) && cfg.colors)
                {
                // Control+A(nsi)

                const int d = iCharNE();

                if (d == '?')
                    {
                    OC.Echo = oldEcho;
                    OC.setio();
                    dispHlp(H_ANSI);

                    OC.SetOutFlag(IMPERVIOUS);

                    CRmPrintf(pcts, gprompt);

                    buf[i] = 0;
                    mPrintf(pcts, buf);

                    SetDoWhat(PROMPT);

                    OC.Echo = NEITHER;
                    OC.setio();
                    }
                else if (d == 'X')
                    {
                    tempTerm[0] = CTRL_A;
                    tempTerm[1] = 'X';
                    tempTerm[2] = 0;
                    termPos = 2;
                    }
                else if ((d >= '0' && d <= '8') ||
                        (d >= 'a' && d <= 'h')  ||
                        (d == TERM_DATE[0])     ||
                        (d == TERM_TIME[0])     ||
                        (d == TERM_USERNAME[0]) ||
                        (d == TERM_FIRSTNAME[0])||
                        (d == TERM_POOP[0])     ||
                        (d == TERM_RND_BACK[0]) ||
                        (d == TERM_RND_FORE[0]) ||
                        (d >= 'A' && d <= 'H'))
                    {
                    OC.Echo = oldEcho;
                    OC.setio();

                    tempTerm[0] = (char) d;
                    tempTerm[1] = 0;

                    termCap(tempTerm);

                    OC.Echo = NEITHER;
                    OC.setio();

                    buf[i++] = CTRL_A;
                    buf[i++] = (char) d;
                    }
                else
                    {
                    OC.Echo = oldEcho;
                    OC.setio();

                    oChar(BELL);

                    OC.Echo = NEITHER;
                    OC.setio();
                    }
                }
            else if (i < lim && c != '\t' && c != CTRL_A)
                {
                if ((OC.EchoChar >= '0') && (OC.EchoChar <= '9'))
                    {
                    OC.EchoChar++;
                    if (OC.EchoChar > '9') OC.EchoChar = '0';
                    }

                buf[i] = c;

                if (doEcho)
                    {
                    OC.Echo = oldEcho;
                    OC.setio();

                    oChar(c);

                    OC.Echo = NEITHER;
                    OC.setio();
                    }
                else
                    {
                    if (cfg.nopwecho == 0)
                        {
                        OC.Echo = CALLER;
                        OC.setio();
                        oChar(c);
                        }
                    else
                        {
                        OC.Echo = oldEcho;
                        OC.setio();
                        oChar((char) OC.EchoChar);
                        }

                    OC.Echo = NEITHER;
                    OC.setio();
                    }

                i++;
                }
            else
                {
                OC.Echo = oldEcho;
                OC.setio();

                oChar(BELL);

                OC.Echo = NEITHER;
                OC.setio();

                errors++;

                if (errors > cfg.maxerror && !onConsole)
                    {
                    CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(79));

                    Hangup();

                    if (w)
                        {
                        destroyCitWindow(w, FALSE);
                        }
                    }
                }
            }

        // kludge to return immediately on single '?'
        if (QuestIsSpecial && *buf == '?')
            {
            break;
            }
        }

    if (!HaveConnectionToUser())
        {
        i = 0;
        useDefault = FALSE;
        ToReturn = FALSE;
        }

    OC.Echo = oldEcho;
    OC.setio();

    while (i && buf[i - 1] == CTRL_A)
        {
        buf[--i] = 0;
        }

    buf[i] = 0;
    OC.EchoChar = 0;

    if (useDefault && !*buf)
        {
        strcpy(buf, dfault);
        }

    SetDoWhat(oldDowhat);

    doCR();

    OC.MCI_goto = FALSE;

    return (ToReturn);
    }


// --------------------------------------------------------------------------
// GetOneKey(): Asks a question with a single-key answer.
//
// Input:
//  prompt      What to ask
//  keys        What to acccept
//  dfault      What ENTER means
//  HelpBlurb   What ? shows
//  ...         char * of full names of keys
//
// Output:
//  ReturnVal   Index to keys of selected option (0-based), or CERROR if ESC

int cdecl TERMWINDOWMEMBER GetOneKey(const char *prompt, const char *keys,
        const char dfault, BlurbFiles HelpBlurb, ...)
    {
    assert(strchr(keys, dfault));

    const char *FullNames[36];  // A-Z0-9 better be enough...

    va_list ap;

    int KeyCount = strlen(keys);
    if (KeyCount > 36)
        {
        KeyCount = 36;
        }

    va_start(ap, HelpBlurb);

    for (int n = 0; n < KeyCount; ++n)
        {
        FullNames[n] = va_arg(ap, const char *);
        }

    va_end(ap);

    Bool ShowFull = !CurrentUser->IsExpert();

    int RetVal;
    dowhattype oldDowhat = DoWhat;
    SetDoWhat(PROMPT);

    Bool DoAgain;

    do
        {
        doCR();

        OC.SetOutFlag(IMPERVIOUS);

        sprintf(gprompt, getmsg(52), prompt);

        if (ShowFull)
            {
            for (int i = 0; i < KeyCount; i++)
                {
                strcat(gprompt, FullNames[i]);
                strcat(gprompt, getmsg(138));
                }

            gprompt[strlen(gprompt) - 1] = 0;

            strcat(gprompt, getmsg(148));
            strcat(gprompt, FullNames[strpos(dfault, keys) - 1]);
            }
        else
            {
            // This whole bit is quite gross. cope.

            label Temp;
            Temp[1] = '/';
            Temp[2] = 0;

            for (int i = 0; i < KeyCount; i++)
                {
                Temp[0] = keys[i];
                strcat(gprompt, Temp);
                }

            gprompt[strlen(gprompt) - 1] = 0;

            Temp[0] = ')';
            Temp[1] = '[';
            Temp[2] = dfault;
            Temp[3] = 0;

            strcat(gprompt, Temp);
            }

        strcat(gprompt, getmsg(189));

        mPrintf(pcts, gprompt);

        int errors = 0;

        do
            {
            DoAgain = FALSE;
            int c = toupper(iCharNE());

            if ((c == '\n') || (c == '\r'))
                {
                c = dfault;
                }
            else if (c == ESC || (!c && !HaveConnectionToUser()))
                {
                doCR();
                RetVal = CERROR;
                break;
                }

            if (!c && HaveConnectionToUser())
                {
                continue;
                }

            if (strchr(keys, c))
                {
                RetVal = strpos((char) c, keys) - 1;
                mPrintfCR(pcts, FullNames[RetVal]);
                break;
                }
            else
                {
                if (c == '?')
                    {
                    DoAgain = TRUE;

                    if (ShowFull)
                        {
                        if (HelpBlurb == MAXBLBS)
                            {
                            CRmPrintfCR(getmsg(103));
                            }
                        else
                            {
                            doCR();
                            dispBlb(HelpBlurb);
                            }
                        }
                    else
                        {
                        ShowFull = TRUE;
                        doCR();
                        }

                    break;
                    }
                else
                    {
                    errors++;

                    if (errors > cfg.maxerror && !onConsole)
                        {
                        CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(79));

                        Hangup();

                        if (w)
                            {
                            destroyCitWindow(w, FALSE);
                            }
                        }
                    }
                }
            } while (HaveConnectionToUser());
        } while (HaveConnectionToUser() && DoAgain);
                // This HaveConnectionToUser() is probably redundant, but safe

    OC.SetOutFlag(OUTOK);

    SetDoWhat(oldDowhat);

    if (!HaveConnectionToUser())
        {
        return (strpos(dfault, keys) - 1);
        }

    return (RetVal);
    }

// --------------------------------------------------------------------------
// getYesNo(): Gets a yes/no/abort or the default.

int TERMWINDOWMEMBER getYesNo(const char *prompt, int dfault)
// 0 = yN; 1 = Yn; 2 = ynA; 3 = yNa; 4 = Yna
    {
    int FromGOK;

    switch (dfault)
        {
        case 0: // yN
        case 1: // Yn
            {
            char dflt = getmsg(617)[!dfault];
            FromGOK = GetOneKey(prompt, getmsg(617), dflt, B_YN, getmsg(620),
                    getmsg(619));
            break;
            }

        case 2: // ynA
        case 3: // yNa
        case 4: // Yna
            {
            char dflt = dfault == 2 ? getmsg(615)[2] : dfault == 3 ?
                    getmsg(615)[1] : getmsg(615)[0];
            FromGOK = GetOneKey(prompt, getmsg(615), dflt, B_YNA, getmsg(620),
                    getmsg(619), getmsg(618));
            break;
            }

        default:
            {
            assert(FALSE);
            FromGOK = 1;
            break;
            }
        }

    switch (FromGOK)
        {
        case -1:    // ESC
            {
            return (dfault > 1 ? 2 : 0);
            }

        case 0: return (1);
        case 1: return (0);
        default: return (2);
        }
    }


// --------------------------------------------------------------------------
// Isfunction(): Checks validity of character received.

static Bool Isfunction(char c, char type)
    {
    switch (type)
        {
        case 'A': return (isalpha(c));
        case 'a': return (isalpha(c) || ' ' == c);
        case '#': return (isdigit(c));
        case '?': return (isprint(c));
        case '*': return (isascii(c));
        case 'X': return (isalnum(c));
        case 'x': return (isalnum(c) || ' ' == c);
        default: return (TRUE);
        }
    }


// --------------------------------------------------------------------------
// getFmtString(): Gets a string from user in desired format.
// char *prompt;                Enter PROMPT
// char *buf;                   Where to put it
// Bool premature               Can return before through
// Bool inclPrompt              Include prompt in returned buffer

#define FT '\''         // easier to type

void TERMWINDOWMEMBER getFmtString(const char *prompt, char *buf,
        Bool premature, Bool inclPrompt)
    {
    char c;
    int errors = 0;
    int entered, parsed = 0;

    const EchoType oldEcho = OC.Echo;
    const dowhattype oldDowhat = DoWhat;

    ansi(14);   // shut off music

    if (!HaveConnectionToUser())
        {
        *buf = 0;
        OC.MCI_goto = FALSE;
        return;
        }

    char *parsestr = new char[strlen(prompt) + 1];      // parsed characters
    char *notparsestr = new char[strlen(prompt) + 1];   // literal characters

    if (notparsestr && parsestr)
        {
        OC.SetOutFlag(IMPERVIOUS);

        SetDoWhat(PROMPT);

        for (int infmt = 0, i = 0; prompt[i]; i++)
            {
            if (prompt[i] == FT)
                {
                infmt = !infmt;
                }
            else if (infmt)
                {
                notparsestr[parsed] = prompt[i];
                parsestr[parsed] = FT;
                parsed++;
                }
            else if (strchr(getmsg(672), prompt[i]))
                {
                notparsestr[parsed] = FT;
                parsestr[parsed] = prompt[i];
                parsed++;
                }
            }

        notparsestr[parsed] = parsestr[parsed] = 0;  // tie off

        for (parsed = 0; parsestr[parsed] && FT == parsestr[parsed]; parsed++)
            {
            oChar(notparsestr[parsed]);
            if (inclPrompt)
                {
                buf[parsed] = notparsestr[parsed];
                }
            }

        OC.Echo = NEITHER;
        OC.setio();

        for (entered = 0, c = (char) iChar(); HaveConnectionToUser(); c = (char) iChar())
            {
            OC.SetOutFlag(OUTOK);
            OC.Echo = oldEcho;
            OC.setio();

            if (c == 0)
                {
                if (HaveConnectionToUser())
                    {
                    continue;
                    }
                else
                    {
                    entered = parsed = 0;
                    break;
                    }
                }
            else if (c == '\b' && entered)
                {
                for ( ; parsestr[parsed - 1] == FT; parsed--)
                    {
                    doBS();
                    }
                doBS();
                parsed--;
                entered--;
                }
            else if (c == '\n' && (premature || !parsestr[parsed]))
                {
                break;
                }
            else if (parsestr[parsed] && Isfunction(c, parsestr[parsed]))
                {
                oChar(c);
                buf[inclPrompt ? parsed : entered] = c;
                entered++;
                parsed++;
                for (; parsestr[parsed] && (FT == parsestr[parsed]); parsed++)
                    {
                    oChar(notparsestr[parsed]);
                    if (inclPrompt)
                        {
                        buf[parsed] = notparsestr[parsed];
                        }
                    }
                }
            else
                {
                oChar(BELL);

                if ('\b' != c && parsestr[parsed])
                    {
                    errors++;

                    if (errors > cfg.maxerror && !onConsole)
                        {
                        CITWINDOW *w = ScreenSaver.IsOn() ? NULL : CitWindowsMsg(NULL, getmsg(685), getmsg(79));

                        Hangup();

                        if (w)
                            {
                            destroyCitWindow(w, FALSE);
                            }
                        }
                    }
                }

            OC.Echo = NEITHER;
            OC.setio();
            }

        OC.Echo = oldEcho;
        OC.setio();

        buf[inclPrompt ? parsed : entered] = 0;

        OC.EchoChar = 0;

        SetDoWhat(oldDowhat);

        doCR();

        OC.MCI_goto = FALSE;
        }
    else
        {
        OutOfMemory(81);
        *buf = 0;
        }

    delete [] notparsestr;
    delete [] parsestr;
    }
