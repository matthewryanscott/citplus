// --------------------------------------------------------------------------
// Citadel: Help.CPP
//
// BLB, HLP, and MNU files.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "msg.h"
#include "hall.h"
#include "helpfile.h"
#include "term.h"


// --------------------------------------------------------------------------
// Contents
//
// dumpf()          does formatted dump of specified file
// goodbye()        prints random goodbye blurbs
// hello()          prints random hello blurbs
// tutorial()       dumps fomatted help files, handles wildcarding
// dispBlb()        Displays BLBs to users
// nochat()         Prints a nochat BLB, handle rotation
// BLBRotate()      prints rotating blurbs
// ShowHelpMenu()   List available helpfiles.

static long ansi_menus[MAXMENUS];
static long norm_menus[MAXMENUS];


// --------------------------------------------------------------------------
// dumpf(): Does formatted dump of specified file.

int TERMWINDOWMEMBER dumpf(const char *filename, Bool format, int special)
    {
    FILE *fbuf;
    int returnval = TRUE, alloced = TRUE;
    char line[MAXWORDLEN];
    int bufSize;
    char *buffer = NULL;
    int foundSpecial = FALSE;

    // last itteration might have been N>exted
    OC.SetOutFlag(OUTOK);

    doCR();

    if ((fbuf = fopen(filename, FO_R)) == NULL)
        {
        if (!special)
            {
            mPrintfCR(getmsg(425), filename);
            }

        return (CERROR);
        }

    // gotos can only be within a single mFormat call; try to dump as much
    // of the file at a time if OC.UseMCI. Else, don't worry about such
    // things and do it line by line, which looks smoother to the caller on
    // a slow machine. (Not a big bunch of file access at the start and at
    // any buffer breaks.

    if (OC.UseMCI)
        {
        bufSize = (int) min(filelength(fileno(fbuf)), 32000l);

        compactMemory();

        while (((buffer = new char[bufSize]) == NULL) && (bufSize > 1000))
            {
            bufSize -= 1000;
            }
        }

    if (!buffer)
        {
        buffer = line;
        bufSize = MAXWORDLEN;
        alloced = FALSE;
        }

    OC.Formatting = format;

    // looks like a kludge, but we need speed!!
    while (fgets(buffer, bufSize, fbuf) && (OC.User.GetOutFlag() != OUTNEXT) &&
            (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
            !OC.CheckInput(FALSE, this))
#else
            !OC.CheckInput(FALSE))
#endif
        {
        if (special != 2 && !foundSpecial)
            {
            foundSpecial = buffer[0] == '#';
            }

        if (!special || (special == 1 && !foundSpecial) ||
                (special == 2 && foundSpecial))
            {
            mFormat(buffer);
            }

        if (!foundSpecial)
            {
            foundSpecial = buffer[0] == '#';
            }
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        returnval = CERROR;
        }

    fclose(fbuf);

    if (alloced)
        {
        delete [] buffer;
        }

    ResetOutputControl();
    return (returnval);
    }


// --------------------------------------------------------------------------
// dumpfc(): Formatted dump of specified file to console.

int TERMWINDOWMEMBER dumpfc(const char *filename, Bool format)
    {
    FILE *fbuf;
    int returnval = TRUE, oldModem, alloced = TRUE;
    char line[MAXWORDLEN];
    int bufSize;
    char *buffer = NULL;

    // last itteration might have been N>exted
    OC.SetOutFlag(OUTOK);

    oldModem = OC.Modem;
    OC.Modem = FALSE;
    doCR();

    if ((fbuf = fopen(filename, FO_R)) == NULL)
        {
        mPrintf(getmsg(425), filename);
        OC.Modem = oldModem;
        return (CERROR);
        }

    // gotos can only be within a single mFormat call; try to dump as much
    // of the file at a time if OC.UseMCI. Else, don't worry about such
    // things and do it line by line, which looks smoother to the caller on
    // a slow machine. (Not a big bunch of file access at the start and at
    // any buffer breaks.

    if (OC.UseMCI)
        {
        bufSize = (int) min(filelength(fileno(fbuf)), 32000l);

        compactMemory();

        while (((buffer = new char[bufSize]) == NULL) && (bufSize > 1000))
            {
            bufSize -= 1000;
            }
        }

    if (!buffer)
        {
        buffer = line;
        bufSize = MAXWORDLEN;
        alloced = FALSE;
        }

    // looks like a kludge, but we need speed!!
    OC.Formatting = format;

    while (fgets(buffer, bufSize, fbuf) && (OC.User.GetOutFlag() != OUTNEXT) &&
            (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
            !OC.CheckInput(FALSE, this))
#else
            !OC.CheckInput(FALSE))
#endif
        {
        mFormat(buffer);
        }

    if (OC.User.GetOutFlag() == OUTSKIP)
        {
        returnval = CERROR;
        }

    fclose(fbuf);

    if (alloced)
        {
        delete [] buffer;
        }

    ResetOutputControl();
    OC.Modem = oldModem;
    return (returnval);
    }


// --------------------------------------------------------------------------
// dispBlb(): Displays BLBs to users.

void TERMWINDOWMEMBER dispBlb(BlurbFiles theBlb)
    {
    Bool shown = FALSE;
    label Blb;
    fpath old;

    getfpath(old);

    strcpy(Blb, blbnames[theBlb]);
    strcat(Blb, getmsg(428));

    if (changedir(cfg.helppath))
        {
        label temp;

        strcpy(temp, Blb);
        temp[strlen(temp) - 1] = '@';

        if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
            {
            dumpf(temp, FALSE, FALSE);
            shown = TRUE;
            }
        else
            {
            if (filexists(Blb))
                {
                dumpf(Blb, TRUE, FALSE);
                shown = TRUE;
                }
            else
                {
                if (*cfg.helppath2)
                    {
                    changedir(cfg.helppath2);
                    if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
                        {
                        dumpf(temp, FALSE, FALSE);
                        shown = TRUE;
                        }
                    else
                        {
                        if (filexists(Blb))
                            {
                            dumpf(Blb, TRUE, FALSE);
                            shown = TRUE;
                            }
                        }
                    }
                }
            }
        }

    if (!shown)
        {
        discardable *d;

        if ((d = readData(13, theBlb, theBlb)) != NULL)
            {
            Bool first = TRUE;

            for (int i = 0; ((char **) d->aux)[i][0] != '#'; i++)
                {
                if (first)
                    {
                    doCR();
                    first = FALSE;
                    }

                mFormat(((char **) d->aux)[i]);
                mFormat(bn);
                }

            doCR();

            discardData(d);
            }
        else
            {
            OutOfMemory(82);
            }
        }
    else
        {
        doCR();
        }

    changedir(old);
//  changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// dispHlp(): Displays default HLPs to users.

void TERMWINDOWMEMBER dispHlp(HelpFiles theHlp)
    {
    OC.SetOutFlag(OUTOK);

    Bool shown = FALSE;
    label Hlp;

    if (!CurrentUser->IsExpert())
        {
        CRmPrintfCR(getmsg(577));
        }

    strcpy(Hlp, hlpnames[theHlp]);
    strcat(Hlp, getmsg(423));

    if (changedir(cfg.helppath))
        {
        label temp;

        strcpy(temp, Hlp);
        temp[strlen(temp) - 1] = '@';

        if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
            {
            dumpf(temp, FALSE, FALSE);
            shown = TRUE;
            }
        else
            {
            if (filexists(Hlp))
                {
                dumpf(Hlp, TRUE, FALSE);
                shown = TRUE;
                }
            else
                {
                if (*cfg.helppath2)
                    {
                    changedir(cfg.helppath2);
                    if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
                        {
                        dumpf(temp, TRUE, FALSE);
                        shown = TRUE;
                        }
                    else
                        {
                        if (filexists(Hlp))
                            {
                            dumpf(Hlp, TRUE, FALSE);
                            shown = TRUE;
                            }
                        }
                    }
                }
            }
        }

    if (!shown)
        {
        discardable *d;

        doCR();

        if ((d = readData(14, theHlp, theHlp)) != NULL)
            {
            for (int i = 0; ((char **) d->aux)[i][0] != '#'; i++)
                {
                mFormat(((char **) d->aux)[i]);
                mFormat(bn);
                }

            doCR();

            discardData(d);
            }
        else
            {
            OutOfMemory(83);
            }
        }
    else
        {
        doCR();
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// tutorial(): Dumps fomatted help files, handles wildcarding.

void TERMWINDOWMEMBER tutorial(const char *filename, const Bool *ShownInternal)
    {
    int i;
    label temp;

    OC.SetOutFlag(OUTOK);

    if (!changedir(cfg.helppath))
        {
        return;
        }

    // no bad files
    if (!IsFilenameLegal(filename, FALSE))
        {
        CRmPrintfCR(getmsg(425), filename);
        changedir(cfg.homepath);
        return;
        }

    if (ambig(filename))
        {
        directoryinfo *files = filldirectory(filename, SORT_ALPHA, OldAndNew,
                FALSE);

        if (files)
            {
            for (i = 0; *files[i].Name ; i++)
                {
                if (ShownInternal)
                    {
                    label WithoutExtension;
                    int j;

                    strcpy(WithoutExtension, files[i].Name);

                    char *Period = strchr(WithoutExtension, '.');

                    if (Period)
                        {
                        *Period = 0;
                        }

                    for (j = 0; j < MAXHLP; j++)
                        {
                        if (ShownInternal[j] && SameString(WithoutExtension,
                                hlpnames[j]))
                            {
                            break;
                            }
                        }

                    if (j != MAXHLP)
                        {
                        continue;
                        }
                    }

                if (!CurrentUser->IsExpert())
                    {
                    CRmPrintfCR(getmsg(577));
                    }

                if (dumpf(files[i].Name, TRUE, FALSE) == CERROR)
                    {
                    break;
                    }
                else
                    {
                    doCR();
                    }
                }

            if (!i)
                {
                CRmPrintfCR(getmsg(425), filename);
                }

            delete [] files;
            }
        }
    else
        {
        if (ShownInternal)
            {
            label WithoutExtension;
            int j;

            strcpy(WithoutExtension, filename);

            char *Period = strchr(WithoutExtension, '.');

            if (Period)
                {
                *Period = 0;
                }

            for (j = 0; j < MAXHLP; j++)
                {
                if (ShownInternal[j] && SameString(WithoutExtension,
                        hlpnames[j]))
                    {
                    return;
                    }
                }
            }

        strcpy(temp, filename);
        temp[strlen(temp)-1] = '@';

        if (!CurrentUser->IsExpert())
            {
            CRmPrintfCR(getmsg(577));
            }

        if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
            {
            dumpf(temp, FALSE, FALSE);
            }
        else
            {
            if (filexists(filename))
                {
                dumpf(filename, TRUE, FALSE);
                }
            else
                {
                if (*cfg.helppath2)
                    {
                    changedir(cfg.helppath2);
                    if (TermCap->GetTermType() == TT_ANSI && filexists(temp))
                        {
                        dumpf(temp, TRUE, FALSE);
                        }
                    else
                        {
                        dumpf(filename, TRUE, FALSE);
                        }
                    }
                else
                    {
                    dumpf(filename, TRUE, FALSE);
                    }
                }
            }
        }

    changedir(cfg.homepath);
    }

// --------------------------------------------------------------------------
// initMenus(): Inits the menus.

void initMenus(void)
    {
    FILE *fl;
    int i;
    char line[90];
    char *words[256];

    for (i = 0; i < MAXMENUS; i++)
        {
        norm_menus[i] = -1;
        ansi_menus[i] = -1;
        }

    if (!changedir(cfg.helppath))
        {
        return;
        }

    if ((fl = fopen(ctdlMnu, FO_R)) != NULL)
        {
        while (fgets(line, 90, fl) != NULL)
            {
            if (line[0] != '#')
                {
                continue;
                }

            parse_it(words, line);

            for (i = 0; i < MAXMENUS; i++)
                {
                if (SameString(words[0] + 1, menunames[i]))
                    {
                    break;
                    }
                }

            if (i < MAXMENUS)
                {
                norm_menus[i] = ftell(fl);
                }
            }
        fclose(fl);
        }

    if ((fl = fopen(ctdlMna, FO_R)) != NULL)
        {
        while (fgets(line, 90, fl) != NULL)
            {
            if (line[0] != '#')
                {
                continue;
                }

            parse_it(words, line);

            for (i = 0; i < MAXMENUS; i++)
                {
                if (SameString(words[0] + 1, menunames[i]))
                    {
                    break;
                    }
                }

            if (i < MAXMENUS)
                {
                ansi_menus[i] = ftell(fl);
                }
            }
        fclose(fl);
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// getMnuOff(): Gets offset to menu in CTDL.MNU.

long TERMWINDOWMEMBER getMnuOff(int reqMnu, char *menuFile)
    {
    if (TermCap->GetTermType() == TT_ANSI && ansi_menus[reqMnu] >= 0)
        {
        if (menuFile)
            {
            strcpy(menuFile, ctdlMna);
            }

        return (ansi_menus[reqMnu]);
        }
    else
        {
        if (menuFile)
            {
            strcpy(menuFile, ctdlMnu);
            }

        return (norm_menus[reqMnu]);
        }
    }


// --------------------------------------------------------------------------
// showMenu(): Shows the requested menu.

void TERMWINDOWMEMBER showMenu(MenuNames reqMnu)
    {
    showMenu(reqMnu, FALSE);
    }

void TERMWINDOWMEMBER showMenu(MenuNames reqMnu, Bool isHelp)
    {
    FILE *fl = NULL;
    long offset;
    char line[MAXWORDLEN];

    OC.SetOutFlag(OUTOK);

    if (!isHelp)
        {
        doCR();
        doCR();
        }

    if (!CurrentUser->IsExpert() && !isHelp)
        {
        mPrintfCR(getmsg(577));
        doCR();
        }

    if (!changedir(cfg.helppath))
        {
        return;
        }

    if (TermCap->GetTermType() == TT_ANSI && ansi_menus[reqMnu] >= 0)
        {
        fl = fopen(ctdlMna, FO_R);
        offset = ansi_menus[reqMnu];
        }
    else
        {
        if ((offset = norm_menus[reqMnu]) >= 0)
            {
            fl = fopen(ctdlMnu, FO_R);
            }
        }

    if (fl)
        {
        fseek(fl, offset, SEEK_SET);

        while (fgets(line, MAXWORDLEN, fl) &&
                (OC.User.GetOutFlag() != OUTNEXT) &&
                (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
                !OC.CheckInput(FALSE, this) && line[0] != '#')
#else
                !OC.CheckInput(FALSE) && line[0] != '#')
#endif
            {
            mFormat(line);
            }

        ResetOutputControl();
        fclose(fl);
        doCR();
        }
    else
        {
        discardable *d;

        if ((d = readData(11, reqMnu, reqMnu)) != NULL)
            {
            int i;

            for (i = 0; ((char **) d->aux)[i][0] != '#'; i++)
                {
                mFormat(((char **) d->aux)[i]);
                mFormat(bn);
                }

            discardData(d);
            }
        else
            {
            OutOfMemory(84);
            }

        doCR();
        }
    }


// --------------------------------------------------------------------------
// hello(): Prints a login BLB, handle rotation.

void TERMWINDOWMEMBER hello(void)
    {
    BLBRotate(getmsg(348), getmsg(355), &cfg.normHello, &cfg.ansiHello,
            FALSE, B_HELLO);
    }


// --------------------------------------------------------------------------
// cometochat(): Prints a login BLB, handle rotation.

void TERMWINDOWMEMBER cometochat(void)
    {
    BLBRotate(getmsg(349), getmsg(355), &cfg.normCometoChat,
        &cfg.ansiCometoChat, FALSE, B_CHAT);
    }


// --------------------------------------------------------------------------
// goodbye(): Prints a logout BLB, handle rotation.

void TERMWINDOWMEMBER goodbye(void)
    {
    BLBRotate(getmsg(350), getmsg(355), &cfg.normBye, &cfg.ansiBye, FALSE,
        B_LOGOUT);
    }


// --------------------------------------------------------------------------
// nochat(): Prints a nochat BLB, handle rotation.

void TERMWINDOWMEMBER nochat(Bool reset)
    {
    if (reset)
        {
        ansiChat = 0;
        normChat = 0;
        }
    else
        {
        BLBRotate(getmsg(354), getmsg(355), &normChat, &ansiChat,
                FALSE, B_NOCHAT);
        }
    }


// --------------------------------------------------------------------------
// BLBRotate(): Prints rotating blurbs.

int TERMWINDOWMEMBER BLBRotate(const char *base, const char *ext, int *reg, int *ansi,
        Bool special, BlurbFiles BlbNum)
    {
    char fn[15];
    char ext2[4];
    int *num;
    int ret;
    Bool UseANSI;

    strcpy(ext2, ext);

    if (!changedir(cfg.helppath))
        {
        return (CERROR);
        }

    // Are we doing ansi or normal
    sprintf(fn, getmsg(421), base, ext);

    if (TermCap->GetTermType() == TT_ANSI && filexists(fn))
        {
        ext2[2] = '@';
        num = ansi;
        UseANSI = TRUE;
        }
    else
        {
        num = reg;
        UseANSI = FALSE;
        }

    if (*num == 0)
        {
        sprintf(fn, getmsg(416), base, ext2);

        if (filexists(fn) || BlbNum == MAXBLBS)
            {
            ret = dumpf(fn, !UseANSI, special);
            }
        else
            {
            dispBlb(BlbNum);
            ret = TRUE;
            }
        }
    else
        {
        sprintf(fn, getmsg(414), base, *num, ext2);

        if (!filexists(fn))
            {
            sprintf(fn, getmsg(416), base, ext2);

            if (filexists(fn) || BlbNum == MAXBLBS)
                {
                ret = dumpf(fn, !UseANSI, special);
                }
            else
                {
                dispBlb(BlbNum);
                ret = TRUE;
                }

            *num = 0;
            }
        else
            {
            ret = dumpf(fn, !UseANSI, special);
            }
        }

    if (special != 1)
        {
        (*num)++;
            }

    changedir(cfg.homepath);
    return (ret);
    }

// --------------------------------------------------------------------------
// ShowHelpMenu(): List available helpfiles.

void TERMWINDOWMEMBER ShowHelpMenu(void)
    {
    OC.SetOutFlag(OUTOK);

    if (!CurrentUser->IsExpert())
        {
        CRmPrintfCR(getmsg(577));
        doCR();
        }

    if (getMnuOff(M_HELP, NULL) > 0)
        {
        showMenu(M_HELP, TRUE);
        }
    else
        {
        directoryinfo *files;
        Bool extFiles[MAXHLP];

        for (int i = 0; i < MAXHLP; i++)
            {
            extFiles[i] = FALSE;
            }

        // create list of actual .HLP files in #HELPPATH

        if (!changedir(cfg.helppath))
            {
            return;
            }

        // load our directory structure with *.HLP files
        files = filldirectory(getmsg(413), SORT_ALPHA, OldAndNew, FALSE);

        if (files)
            {
            int entries;
            for (entries = 0; *files[entries].Name; ++entries);

            for (int i = 0;
                    (i < entries && (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
                    !OC.CheckInput(FALSE, this)); ++i)
#else
                    !OC.CheckInput(FALSE)); ++i)
#endif
                {
                char line[MAXWORDLEN];

#ifdef WINCIT
                strcpy(line, files[i].FullName);
#else
                strcpy(line, files[i].Name);
                strupr(line);
#endif

                const int TestPoint = strlen(line) - 4;
                if (TestPoint >= 0)
                    {
                    if (SameString(line + TestPoint, getmsg(411)))
                        {
                        *(line + TestPoint) = 0;
                        }
                    }

                mPrintfCR(getmsg(410), line);

                for (int j = 0; j < MAXHLP; j++)
                    {
                    if (SameString(line, hlpnames[j]))
                        {
                        extFiles[j] = TRUE;
                        break;
                        }
                    }
                }

            delete [] files;

            // fake internal files
EXTRA_OPEN_BRACE
//            const char hlpName[MAXHLP];

            for (int i = 0; i < MAXHLP; i++)
                {
                if (!extFiles[i])
                    {
                    mPrintfCR(getmsg(410), hlpnames[i]);
//                    sprintf(hlpName[i], getmsg(410), hlpnames[i]);
                    }
                }
EXTRA_CLOSE_BRACE
            }
        }

    changedir(cfg.homepath);
    }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
    this is the (soon to be) world famous YouAreHere!!!
                     version 6.9

    this is really stupid. if the user is lost, he should just do a known.
    perhaps even a .kw or .rh or something.

    but at least the code doesn't suck any more. it really sucked how it
    was implemented in cencit. the display was way butt-kicking ugly in
    cencit, as well.

 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void TERMWINDOWMEMBER youAreHere(void)
    {
    CRmPrintf(getmsg(374), cfg.Uhalls_nym);
    prtList(LIST_START);

    for (h_slot i = 0; i < cfg.maxhalls; i++)
        {
        if (accesshall(i))
            {
            char str[LABELSIZE + 12];

            label Buffer;
            sprintf(str, getmsg(636), (i < thisHall) ? getmsg(409) :
                    (i == thisHall) ? getmsg(408) : getmsg(407),
                    HallData[i].GetName(Buffer, sizeof(Buffer)));

            prtList(str);
            }
        }

    prtList(LIST_END);

    CRmPrintf(getmsg(374), cfg.Urooms_nym);
    prtList(LIST_START);

    const r_slot rps = GetRoomPosInTable(thisRoom);

EXTRA_OPEN_BRACE
    for (r_slot i = 0; i < cfg.maxrooms; i++)
        {
        if (CurrentUser->CanAccessRoom(RoomPosTableToRoom(i), thisHall))
            {
            label Buffer;
            char str[LABELSIZE + 12];

            sprintf(str, getmsg(636), (i < rps) ? getmsg(405) :
                    (i == rps) ? getmsg(408) : getmsg(403),
                    RoomTab[RoomPosTableToRoom(i)].GetName(Buffer,
                    sizeof(Buffer)));

            prtList(str);
            }
        }
EXTRA_CLOSE_BRACE
    prtList(LIST_END);
    }
