// --------------------------------------------------------------------------
// Citadel: FileCmd.CPP
//
// File handling user commands for Citadel

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "infofile.h"
#include "filecmd.h"
#include "msg.h"
#include "log.h"
#include "group.h"
#include "account.h"
#include "cfgfiles.h"
#include "events.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"
#include "xmodem.h"
#include "filefind.h"


// --------------------------------------------------------------------------
// Contents
//
// AideAttributes()     aide fn to set file attributes
// blocks()             displays how many blocks file is
// CanUploadFile()      returns TRUE if filename can be uploaded
// CheckDirectoryRoom() returns TRUE if directory, else -- not dir room
// EnterTextfile()      menu level .ET
// EnterWithProtocol()  menu level .EW
// entrycopy()          readable struct -> directory array
// entrymake()          dos transfer struct -> readable struct
// ReadDirectory()      menu level .r[v]d routine
// readtextfile()       menu level .rt routine
// ReadWithProtocol()   menu level .rw file
// AideRenameFile()     aide fn to rename a file
// textdown()           does wildcarded unformatted file dumps
// textup()             handles actual text upload
// AideUnlink()         handles the .au command
// wcdown()             calls xmodem downloading routines
// wcup()               calls xmodem uploading routines
// upload()             menu level routine
// AideMoveFile()       copy info-buffer & move file


extern int Zmodem_Send_Batch (const char *filepath, unsigned int baud);
extern int Zmodem_Receive_Batch (const char *filepath, unsigned int baud);


// --------------------------------------------------------------------------
// downloadFileQueue(): .QD.
//
// Notes:
//  Assumes MENU_FILEQUEUE is locked.

Bool TERMWINDOWMEMBER downloadFileQueue(void)
    {
    if (fileList)
        {
        Bool toRet;

        int ich;
        int ch = tolower(ich = iCharNE());

        if (ch == '\n' || ch == '\r')
            {
            ch = CurrentUser->GetDefaultProtocol();
            }

        const protocols *theProt = GetProtocolByKey((char) ch, TRUE);

        if (theProt)
            {
            mPrintf(pcts, theProt->name);

            if (!theProt->respDown[0])
                {
                mPrintf(getmsg(MENU_FILEQUEUE, 18));
                return (FALSE);
                }

            doCR();

            FILE *tmpFile;

            changedir(cfg.aplpath);
            tmpFile = fopen(citfiles[C_FILEQTMP_CIT], FO_W);

            if (tmpFile)
                {
                for (fileQueue *theFile = fileList; theFile; theFile = (fileQueue *) getNextLL(theFile))
                    {
                    fprintf(tmpFile, pcts, theFile->fname);
                    fprintf(tmpFile, bn);
                    }

                fclose(tmpFile);

                doTheDownload(citfiles[C_FILEQTMP_CIT], cfg.aplpath, FALSE, theProt, TRUE);

                unlink(citfiles[C_FILEQTMP_CIT]);
                toRet = TRUE;
                }
            else
                {
                CRmPrintf(getmsg(8), citfiles[C_FILEQTMP_CIT]);
                toRet = FALSE;
                }
            }
        else
            {
            if (ch == '?')
                {
                oChar('?');
                upDownMnu('D', TRUE);
                }
            else
                {
                oChar((char) ich);
                mPrintf(sqst);
                if (!CurrentUser->IsExpert())
                    {
                    upDownMnu('D', TRUE);
                    }
                }

            toRet = FALSE;
            }


        CommPort->FlushInput();
        // go back to home
        changedir(cfg.homepath);
        return (toRet);
        }
    else
        {
        CRCRmPrintfCR(getmsg(MENU_FILEQUEUE, 15));
        changedir(cfg.homepath);
        return (FALSE);
        }
    }


// --------------------------------------------------------------------------
// doTheAdd: Adds specified file to .Q.
//
// Notes:
//  Assumes MENU_FILEQUEUE is locked.

void TERMWINDOWMEMBER doTheAdd(const char *fn)
    {
    if (!IsFilenameLegal(fn, NULL) || !filexists(fn))
        {
        CRmPrintf(getmsg(425), fn);
        }
    else
        {
        fileQueue *theFile;

        for (theFile = fileList; theFile; theFile = (fileQueue *) getNextLL(theFile))
            {
            if (RoomTab[thisRoom].IsSameName(theFile->room) && SameString(nameonly(theFile->fname), fn))
                {
                CRmPrintf(getmsg(MENU_FILEQUEUE, 6), fn);
                break;
                }
            }

        if (!theFile)
            {
            char RoomDir[64];
            CurrentRoom->GetDirectory(RoomDir, sizeof(RoomDir));

            theFile = (fileQueue *) addLL((void **) &fileList, sizeof(*theFile) + strlen(fn) + strlen(RoomDir) + 1);

            if (theFile)
                {
                RoomTab[thisRoom].GetName(theFile->room, sizeof(theFile->room));
                sprintf(theFile->fname, sbs, RoomDir, fn);
                strlwr(theFile->fname);

                const int handle = open(fn, O_RDONLY);

                if (handle >= 0)
                    {
                    theFile->size = filelength(handle);
                    close(handle);
                    }

                CRmPrintf(getmsg(MENU_FILEQUEUE, 7), nameonly(theFile->fname), ltoac(theFile->size),
                        theFile->size == 1 ? ns : justs, dltime(theFile->size));
                }
            else
                {
                CRmPrintf(getmsg(188), fn);
                }
            }
        }
    }


// --------------------------------------------------------------------------
// addFileQueue: Adds files to .Q.
//
// Notes:
//  Assumes MENU_FILEQUEUE is locked.

void TERMWINDOWMEMBER addFileQueue(void)
    {
    char filename[128];
    char *words[256];

    SetDoWhat(FILEQUEUEA);

    OC.SetOutFlag(OUTOK);

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    getNormStr(getmsg(289), filename, 126);

    if (!*filename)
        {
        changedir(cfg.homepath);
        return;
        }

    const int count = parse_it(words, filename);

    if (count == 0)
        {
        changedir(cfg.homepath);
        return;
        }

    for (int i = 0; i < count; i++)
        {
            FindFileC FF;
            if (FF.FindFirst(words[i]))
                {
                do
                    {
                    if (FF.IsNormalFile())
					    {
                        doTheAdd(FF.GetShortName());
						}
                    } while (FF.FindNext());
                }
            else
                {
                CRmPrintf(getmsg(425), words[i]);
                }
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// listFileQueue(): Lists all files in .Q.
//
// Notes:
//  Assumes MENU_FILEQUEUE is locked.

void TERMWINDOWMEMBER listFileQueue(void)
    {
    SetDoWhat(FILEQUEUEL);

    OC.SetOutFlag(OUTOK);
    doCR();

    if (fileList)
        {
        long totalSize = 0;
        int fileCounter = 0;

        mPrintfCR(getmsg(MENU_FILEQUEUE, 8), cfg.Uroom_nym, getmsg(MENU_FILEQUEUE, 9), getmsg(MENU_FILEQUEUE, 10),
                getmsg(MENU_FILEQUEUE, 11));

        for (fileQueue *thisFile = fileList; thisFile; thisFile = (fileQueue *) getNextLL(thisFile))
            {
            mPrintfCR(getmsg(MENU_FILEQUEUE, 12), ++fileCounter, thisFile->room, nameonly(thisFile->fname),
                    ltoac(thisFile->size), dltime(thisFile->size));

            totalSize += thisFile->size;
            }

        if (fileCounter > 1)
            {
            for (int i = 0; i < 71; i++)
                {
                oChar('-');
                }

            CRmPrintfCR(getmsg(MENU_FILEQUEUE, 13), getmsg(MENU_FILEQUEUE, 14), ltoac(totalSize), dltime(totalSize));
            }
        }
    else
        {
        CRmPrintfCR(getmsg(MENU_FILEQUEUE, 15));
        }
    }


// --------------------------------------------------------------------------
// removeFileQueue(): Removes files from .Q.
//
// Notes:
//  Assumes MENU_FILEQUEUE is locked.

void TERMWINDOWMEMBER removeFileQueue(void)
    {
    label rmv;

    SetDoWhat(FILEQUEUER);

    OC.SetOutFlag(OUTOK);
    doCR();

    do
        {
        getString(getmsg(MENU_FILEQUEUE, 16), rmv, LABELSIZE, ns);

        if (*rmv == '?')
            {
            listFileQueue();
            }
        } while (*rmv == '?');

    normalizeString(rmv);

    if (*rmv)
        {
        const int r = atoi(rmv);
        fileQueue *theFile = fileList;
        for (int i = 1; theFile; theFile = (fileQueue *) getNextLL(theFile), i++)
            {
            if (i == r || SameString(rmv, nameonly(theFile->fname)))
                {
                deleteLLNode((void **) &fileList, i);
                break;
                }
            }

        if (!theFile)
            {
            mPrintf(getmsg(MENU_FILEQUEUE, 17));
            }
        }
    }


// --------------------------------------------------------------------------
// clearFileQueue(): Clears file queue.

void TERMWINDOWMEMBER clearFileQueue(void)
    {
    SetDoWhat(FILEQUEUEC);
    disposeLL((void **) &fileList);
    }


// --------------------------------------------------------------------------
// AideAttributes(): Aide fn to set file attributes.

void TERMWINDOWMEMBER AideAttributes(void)
    {
    if (CheckDirectoryRoom())
        {
        label filename;

        SetDoWhat(AIDEATTR);

        if (!CurrentRoom->ChangeDir())
            {
            label Buffer;
            CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
            changedir(cfg.homepath);
            return;
            }

        doCR();
        getNormStr(getmsg(289), filename, LABELSIZE);

        if (!*filename)
            {
            changedir(cfg.homepath);
            return;
            }

        if (!IsFilenameLegal(filename, NULL) || ambig(filename))
            {
            CRmPrintfCR(getmsg(382), filename);
            changedir(cfg.homepath);
            return;
            }

        if (!filexists(filename))
            {
            CRmPrintfCR(getmsg(422));
            changedir(cfg.homepath);
            return;
            }

        if (!LockMenu(MENU_AIDE))
            {
            changedir(cfg.homepath);
            return;
            }

        Message *Msg = new Message;

        if (Msg)
            {
            Bool ReadOnly, Hidden;
#ifdef WINCIT
            int attr = GetFileAttributes(filename);
#else
            int attr = _chmod(filename, 0);
#endif
            ReadOnly = getYesNo(getmsg(MENU_AIDE, 25), attr & FA_RDONLY);
            Hidden = getYesNo(getmsg(MENU_AIDE, 26), (attr & FA_HIDDEN) == FA_HIDDEN);

            // set read-only and hidden bits
            if (ReadOnly)
                {
                attr |= FA_RDONLY;
                }
            else
                {
                attr &= ~FA_RDONLY;
                }

            if (Hidden)
                {
                attr |= FA_HIDDEN;
                }
            else
                {
                attr &= ~FA_HIDDEN;
                }
#ifdef WINCIT
            SetFileAttributes(filename, attr);
#else
            _chmod(filename, 1, attr);
#endif

            label Buffer, Buffer1;
            Msg->SetTextWithFormat(getmsg(MENU_AIDE, 27), filename, RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)),
                    CurrentUser->GetName(Buffer, sizeof(Buffer)));

#ifdef WINCIT
            trap(T_AIDE, WindowCaption, pcts, Msg->GetText());
#else
            trap(T_AIDE, pcts, Msg->GetText());
#endif

            if (RoomTab[thisRoom].IsGroupOnly())
                {
                if (RoomTab[thisRoom].IsBooleanGroup())
                    {
                    Msg->SetGroup(SPECIALSECURITY);
                    }
                else
                    {
                    Msg->SetGroup(RoomTab[thisRoom].GetGroupNumber());
                    }
                }
            Msg->SetRoomNumber(AIDEROOM);
            systemMessage(Msg);

            delete Msg;
            }
        else
            {
            OutOfMemory(14);
            }

        UnlockMenu(MENU_AIDE);
        changedir(cfg.homepath);
        }
    }


// --------------------------------------------------------------------------
// blocks(): Displays how many blocks file is upon download.
//
// Notes:
//  Assumes MSG_TRANS is loaded.

void TERMWINDOWMEMBER blocks(const char *filename, long length, int bsize)
    {
    OC.SetOutFlag(OUTOK);

    CRmPrintf(getmsg(632), filename);

    if (length == -1l)
        {
        return;
        }

    doCR();

    if (!bsize)
        {
        mPrintf(getmsg(MSG_TRANS, 20), ltoac(length), (length == 1l) ? ns : justs);
        }
    else
        {
        const int blks = (int) (length / (long) bsize) + 1;
        label Blocks;
        CopyStringToBuffer(Blocks, ltoac(blks));

        mPrintf(getmsg(MSG_TRANS, 21), Blocks, (blks == 1) ? ns : justs, ltoac(length), (length == 1l) ? ns : justs);
        }

    CRmPrintfCR(getmsg(MSG_TRANS, 22), dltime(length));
    }


// --------------------------------------------------------------------------
// CanUploadFile(): Returns TRUE if filename can be uploaded.

Bool TERMWINDOWMEMBER CanUploadFile(const char *filename)
    {
    if (!*filename || ambig(filename))
        {
        return (FALSE);
        }

    // no bad files
    if (!IsFilenameLegal(filename, FALSE))
        {
        CRmPrintfCR(getmsg(382), filename);
        return (FALSE);
        }

    if (!CurrentRoom->ChangeDir())
        {
        return (FALSE);
        }

    if (filexists(filename))
        {
        CRmPrintfCR(getmsg(346), filename);
        changedir(cfg.homepath);
        return (FALSE);
        }

    return (TRUE);
    }


// --------------------------------------------------------------------------
// CheckDirectoryRoom(): returns TRUE if directory, else -- not dir room.

Bool TERMWINDOWMEMBER CheckDirectoryRoom(void)
    {
    if (!RoomTab[thisRoom].IsMsDOSdir())
        {
        if (CurrentUser->IsExpert())
            {
            mPrintf(qst);
            }
        else
            {
            CRCRmPrintfCR(getmsg(215), cfg.Lroom_nym);
            }

        return (FALSE);
        }
    else
        {
        return (TRUE);
        }
    }


// --------------------------------------------------------------------------
// EnterTextfile(): Menu level .ET.

void TERMWINDOWMEMBER EnterTextfile(Bool HideIt)
    {
    if (!LockMenu(MENU_ENTER))
        {
        return;
        }

    if (LockMessages(MSG_TRANS))
        {
        if (IsFileUploadOkay())
            {
            label filename;
            char comments[64];

            SetDoWhat(ENTERTEXT);

            if (!CurrentUser->IsExpert())
                {
                dispBlb(B_TEXTUP);
                }
            else
                {
                doCR();
                }

            getNormStr(getmsg(289), filename, LABELSIZE);

            if (CanUploadFile(filename))
                {
                getNormStr(getmsg(383), comments, 63);

                textup(filename, HideIt);

                if (CurrentRoom->ChangeDir() && filexists(filename))
                    {
                    label Buffer;
                    char Buffer1[64];

#ifdef WINCIT
                    // Need short name for FILEINFO.DAT.
                    directoryinfo *files = filldirectory(filename, SORT_DATE, OldAndNew, TRUE);

                    if (files && *files[0].Name)
                        {
                        UpdateInfoFile(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)), files[0].Name,
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), comments, 0, 0);
                        }

                    delete [] files;
#else
                    UpdateInfoFile(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)), filename,
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), comments, 0, 0);
#endif

                    Message *Msg = new Message;

                    if (Msg)
                        {
                        if (*comments)
                            {
                            Msg->SetTextWithFormat(getmsg(MSG_TRANS, 23), filename,
                                    CurrentUser->GetName(Buffer, sizeof(Buffer)), bn, comments);
                            }
                        else
                            {
                            Msg->SetTextWithFormat(getmsg(MSG_TRANS, 24), filename,
                                    CurrentUser->GetName(Buffer, sizeof(Buffer)));
                            }

                        Msg->SetRoomNumber(thisRoom);
                        systemMessage(Msg);

                        delete Msg;
                        }
                    else
                        {
                        OutOfMemory(15);
                        }
                    }
                }

            changedir(cfg.homepath);
            }

        UnlockMessages(MSG_TRANS);
        }
    else
        {
        OutOfMemory(118);
        }

    UnlockMenu(MENU_ENTER);
    }


// --------------------------------------------------------------------------
// GetInternalProtocolName()
//
// Notes:
//  Assumes MSG_TRANS is locked.

static const char *GetInternalProtocolName(InternalProtocols prot)
    {
    switch (prot)
        {
        case IP_XMODEM:     return (getmsg(MSG_TRANS, 25));
        case IP_CRCXMODEM:  return (getmsg(MSG_TRANS, 26));
        case IP_ONEKXMODEM: return (getmsg(MSG_TRANS, 5));
        case IP_ZMODEM:     return (getmsg(MSG_TRANS, 4));
        default:            return (getmsg(MSG_TRANS, 3));
        }
    }


// --------------------------------------------------------------------------
// ReadDirectory(): Menu level .r[v]d HIGH level routine.

void TERMWINDOWMEMBER ReadDirectory(OldNewPick which)
    {
    if (!CheckDirectoryRoom())
        {
        return;
        }

    label filename;

#ifdef WINCIT
    __int64 total_bytes = 0;
#else
    long total_bytes = 0l;
#endif

    SetDoWhat(READDIR);

    doEvent(EVT_READDIR);

    doCR();
    if (!getNormStr(getmsg(289), filename, LABELSIZE - 2))
        {
        return;
        }

    if (!LockMenu(MENU_READ))
        {
        return;
        }

    if (!*filename)
        {
        strcpy(filename, starstar);
        }
    else if (!strchr(filename, '.'))
        {
        strcat(filename, getmsg(MENU_READ, 27));
        }
    else if (*filename == '.')
        {
        label filename2;
        CopyStringToBuffer(filename2, filename);
        *filename = '*';
        CopyString2Buffer(filename + 1, filename2, sizeof(filename) - 1);
        }

    if (CurrentRoom->ChangeDir())
        {
        if (which != OldAndNew)
            {
            if (!getNewOldFileDate())
                {
                UnlockMenu(MENU_READ);
                return;
                }
            }

        SetOutOK();

        // load our directory structure according to filename
        directoryinfo *files = filldirectory(filename, MRO.Date ? SORT_DATE : SORT_ALPHA, which, MRO.Reverse);

        if (files && *files[0].Name)
            {
            int i;
            char dstamp[10];

            doCR();
            if (MRO.Verbose)
                {
#ifdef WINCIT
                mPrintf(getmsg(MENU_READ, 117), (CurrentUser->GetWidth() > 41) ? spcspc : ns,
                        (CurrentUser->GetWidth() > 45) ? getmsg(MENU_READ, 29) : getmsg(MENU_READ, 30),
                        (CurrentUser->GetWidth() > 57) ? getmsg(MENU_READ, 118) : ns);
#else
                mPrintf(getmsg(MENU_READ, 28), (CurrentUser->GetWidth() > 41) ? spcspc : ns,
                        (CurrentUser->GetWidth() > 45) ? getmsg(MENU_READ, 29) : getmsg(MENU_READ, 30));
#endif
                }

            int entries;
            for (entries = 0; *files[entries].Name; ++entries);

            for (i = 0;

                    (i < entries &&
                        (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
                        !OC.CheckInput(FALSE, this));
#else
                        !OC.CheckInput(FALSE));
#endif

                    ++i)
                {
                if (MRO.Verbose)
                    {
                    getdstamp(dstamp, files[i].DateTime);

                    if (CurrentUser->GetWidth() > 41)
                        {
                        CRmPrintf(getmsg(MENU_READ, 31), files[i].Name, dstamp, ltoac(files[i].Length),
                                CurrentUser->GetWidth() > 45 ? spcspc : ns, dltime(files[i].Length));
#ifdef WINCIT
                        if (CurrentUser->GetWidth() > 57)
                            {
                            mPrintf(getmsg(MENU_READ, 119), files[i].FullName);
                            }
#endif
                        }
                    else
                        {
                        CRmPrintf(getmsg(MENU_READ, 32), files[i].Name, dstamp, files[i].Length, dltime(files[i].Length));
                        }

                    total_bytes += files[i].Length;
                    }
                else
                    {
                    mPrintf(getmsg(MENU_READ, 33), files[i].Name);
                    }

                if (MRO.DotoMessage == REVERSE_READ)
                    {
                    MRO.DotoMessage = NO_SPECIAL;
                    OC.SetOutFlag(OUTOK);

                    MRO.Reverse = !MRO.Reverse;
                    qsort(files, entries, sizeof(*files), (MRO.Reverse) ?
                            (QSORT_CMP_FNP) revstrcmp : (QSORT_CMP_FNP) fwdstrcmp);

                    mPrintf(getmsg(357), (!MRO.Reverse) ? '+' : '-');
                    if (!MRO.Verbose)
                        {
                        doCR();
                        }

                    i = entries - (i + 1);
                    }
                }

            if (MRO.Verbose && OC.User.GetOutFlag() != OUTSKIP)
                {
                label Files, Bytes;

#ifdef WINCIT
                CopyStringToBuffer(Bytes, i64toac(total_bytes));
#else
                CopyStringToBuffer(Bytes, ltoac(total_bytes));
#endif

                CopyStringToBuffer(Files, ltoac(i));

                doCR();
                CRmPrintf(getmsg(MENU_READ, 34), Bytes,
                        (total_bytes == 1) ? ns : justs, Files,
#ifdef WINCIT
                        (i == 1) ? ns : justs, i64toac(bytesfree()));
#else
                        (i == 1) ? ns : justs, ltoac(bytesfree()));
#endif
                }

            doCR();
            }
        else
            {
            CRmPrintfCR(getmsg(425), filename);
            }

        delete [] files;
        }
    else
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        }

    changedir(cfg.homepath);
    UnlockMenu(MENU_READ);
    }


// --------------------------------------------------------------------------
// readtextfile(): Menu level .rt HIGH level routine.

void TERMWINDOWMEMBER ReadTextfile(OldNewPick which, Bool Verbose)
    {
    if (LockMenu(MENU_READ))
        {
        if (LockMessages(MSG_TRANS))
            {
            if (IsFileDownloadOkay())
                {
                if (which == WhoKnows)
                    {
                    which = OldAndNew;
                    }

                MRO.Verbose = Verbose;

                SetDoWhat(READTEXT);

                label filename;

                doCR();
                getNormStr(getmsg(289), filename, 13);

                if (*filename)
                    {
                    if (!strchr(filename, '.'))
                        {
                        strcat(filename, getmsg(MENU_READ, 27));
                        }

                    textdown(filename, which);
                    }
                }

            UnlockMessages(MSG_TRANS);
            }
        else
            {
            OutOfMemory(117);
            }

        UnlockMenu(MENU_READ);
        }
    else
        {
        OutOfMemory(117);
        }
    }


// --------------------------------------------------------------------------
// ReadWithProtocol(): Menu level .rw HIGH level routine.
//
// Notes:
//  Assumes MENU_READ is locked.
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER ReadWithProtocol(void)
    {
    if (LockMenu(MENU_READ))
        {
        if (LockMessages(MSG_TRANS))
            {
            if (IsFileDownloadOkay())
                {
                char filename[80];

                const InternalProtocols prot = XmodemMenu('R');
                if (prot == IP_ERROR)
                    {
                    return;
                    }

                SetDoWhat(READWC);

                if (!CurrentUser->IsExpert())
                    {
                    dispBlb(B_WCDOWN);
                    }

                doCR();
                getNormStr(getmsg(289), filename, 79);

                if (*filename)
                    {
                    wcdown(filename, prot);
                    }
                }

            UnlockMessages(MSG_TRANS);
            }
        else
            {
            OutOfMemory(120);
            }

        UnlockMenu(MENU_READ);
        }
    else
        {
        OutOfMemory(120);
        }
    }


// --------------------------------------------------------------------------
// AideRenameFile(): Aide fn to rename a file.

void TERMWINDOWMEMBER AideRenameFile(void)
    {
    if (!CheckDirectoryRoom())
        {
        return;
        }

    if (!LockMenu(MENU_AIDE))
        {
        return;
        }

    label source, destination;

    SetDoWhat(AIDERENAME);

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    doCR();
    getNormStr(getmsg(MENU_AIDE, 131), source, LABELSIZE);
    if (!*source)
        {
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!IsFilenameLegal(source, FALSE) || ambig(source))
        {
        CRmPrintfCR(getmsg(382), source);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!filexists(source))
        {
        CRmPrintfCR(getmsg(425), source);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    getNormStr(getmsg(MENU_AIDE, 132), destination, LABELSIZE);
    if (!*destination)
        {
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!IsFilenameLegal(destination, FALSE) || ambig(destination))
        {
        CRmPrintfCR(getmsg(382), destination);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (filexists(destination))
        {
        CRmPrintfCR(getmsg(346), destination);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    // if successful
    if (rename(source, destination) == 0)
        {
        fInfo FileInfo;
        char RoomDir[64];
        CurrentRoom->GetDirectory(RoomDir, sizeof(RoomDir));

        if (!GetSingleFileInfo(RoomDir, source, &FileInfo))
            {
            memset(&FileInfo, 0, sizeof(FileInfo));
            }

#ifdef WINCIT
        // Need short name for FILEINFO.DAT.
        directoryinfo *files = filldirectory(destination, SORT_DATE, OldAndNew, TRUE);

        if (files && *files[0].Name)
            {
            UpdateInfoFile(RoomDir, files[0].Name, FileInfo.uploader,
                FileInfo.comment, FileInfo.downloads, FileInfo.uploadtime);
            }

        delete [] files;
#else
        UpdateInfoFile(RoomDir, destination, FileInfo.uploader,
                FileInfo.comment, FileInfo.downloads, FileInfo.uploadtime);
#endif

        RemoveFileInfo(RoomDir, source);

        char TrapString[128];

        label Buffer, Buffer1;
        if(!read_tr_messages())
            {
            errorDisp("%s- %s", getmsg(172), getmsg(59));
            UnlockMenu(MENU_AIDE);
            return;
            }

        sprintf(TrapString, gettrmsg(38), source, destination,
                RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)), CurrentUser->GetName(Buffer, sizeof(Buffer)));

#ifndef WINCIT
        trap(T_AIDE, pcts, TrapString);
#else
        trap(T_AIDE, WindowCaption, pcts, TrapString);
#endif

        dump_tr_messages();

        Message *Msg = new Message;

        if (Msg)
            {
            Msg->SetText(TrapString);

            if (RoomTab[thisRoom].IsGroupOnly())
                {
                if (RoomTab[thisRoom].IsBooleanGroup())
                    {
                    Msg->SetGroup(SPECIALSECURITY);
                    }
                else
                    {
                    Msg->SetGroup(RoomTab[thisRoom].GetGroupNumber());
                    }
                }

            Msg->SetRoomNumber(AIDEROOM);
            systemMessage(Msg);

            Msg->ClearAll();
            Msg->SetText(TrapString);

            Msg->SetRoomNumber(thisRoom);
            systemMessage(Msg);

            delete Msg;
            }
        else
            {
            OutOfMemory(16);
            }
        }
    else
        {
        CRmPrintfCR(getmsg(80), source);
        }

    UnlockMenu(MENU_AIDE);
    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// textdown(): Dumps a host file with no formatting.
//
// Notes:
//  At this point, only called from ReadTextfile
//  This routine handles wildcarding of text downloads.
//  Assumes MENU_READ is locked.

void TERMWINDOWMEMBER textdown(const char *filename, OldNewPick which)
    {
    int retval = 0;

    OC.SetOutFlag(OUTOK);

    // no bad files
    if (!IsFilenameLegal(filename, FALSE))
        {
        CRmPrintf(getmsg(425), filename);
        return;
        }

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintf(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    if (ambig(filename))
        {
        if (which != OldAndNew)
            {
            if (!getNewOldFileDate())
                {
                return;
                }

            doCR();
            }

        directoryinfo *files = filldirectory(filename, MRO.Date ? SORT_DATE : SORT_ALPHA, which, MRO.Reverse);

        if (files)
            {
            // print out all the files
            int i;
            for (i = 0; *files[i].Name && (retval != CERROR); i++)
                {
                retval = dumpf(files[i].Name, MRO.Verbose, FALSE);
                }

            if (!i)
                {
                CRmPrintf(getmsg(425), filename);
                }

            delete [] files;
            }

        doCR();
        }
    else
        {
        if (dumpf(filename, MRO.Verbose, FALSE) != CERROR)
            {
            doCR();
            }
        }

    label Buffer;

    if(read_tr_messages())
        {

#ifdef WINCIT
        trap(T_DOWNLOAD, WindowCaption, gettrmsg(31), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
        trap(T_DOWNLOAD, gettrmsg(31), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
        dump_tr_messages();
        }
    else
        {
        errorDisp(getmsg(172));
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// textup(): Handles textfile uploads.
//
// Notes:
//  Currently, only called from EnterTextfile
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER textup(const char *filename, Bool HideIt)
    {
    FILE *upfd;

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    doCR();

    if ((upfd = fopen(filename, FO_W)) == NULL)
        {
        CRmPrintfCR(getmsg(8), filename);
        }
    else
        {
        int i;

        // CTRL+Z
        while (((i = iChar()) != 26) && OC.User.GetOutFlag() != OUTSKIP && (HaveConnectionToUser()))
            {
            fputc(i, upfd);
            }

        fclose(upfd);

        doCR();
        if (!getYesNo(getmsg(MSG_TRANS, 2), 1))
            {
            unlink(filename);
            }
        else
            {
            if (HideIt && filexists(filename))
                {
                hide(filename);
                }

            label Buffer;

            if(read_tr_messages())
                {
#ifdef WINCIT
                trap(T_UPLOAD, WindowCaption, gettrmsg(39), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_UPLOAD, gettrmsg(39), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                dump_tr_messages();
                }
            else
                {
                errorDisp("%s- %s", getmsg(172), getmsg(59));
                }
            }
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// AideUnlink(): Handles .AU Aide Unlink.

void TERMWINDOWMEMBER AideUnlink(void)
    {
    if (!CheckDirectoryRoom())
        {
        return;
        }

    if (!LockMenu(MENU_AIDE))
        {
        return;
        }

    char filename[128];

    SetDoWhat(AIDEUNLINK);

    doCR();

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    GetStringWithBlurb(CurrentUser->IsSysop() ? getmsg(MENU_AIDE, 135) : getmsg(289), filename,
            CurrentUser->IsSysop() ? 127 : 12, ns, B_UNLINK);

    normalizeString(filename);

    if (!*filename)
        {
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if ((ambig(filename) || strchr(filename, ' ')) && !CurrentUser->IsSysop())
        {
        CRmPrintfCR(getmsg(MENU_AIDE, 136));
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    strupr(filename);

    label Buffer, Buffer1;
    amPrintf(getmsg(MENU_AIDE, 137), cfg.Lroom_nym, RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)),
            CurrentUser->GetName(Buffer, sizeof(Buffer)));

    int FilesUnlinked = 0;
    Bool Continuous = FALSE, Abort = FALSE;

    char *words[128];
    const int count = parse_it(words, filename);

    for (int DelIndex = 0; !Abort && DelIndex < count; DelIndex++)
        {
        directoryinfo *files = filldirectory(words[DelIndex], SORT_ALPHA, OldAndNew, FALSE);

        if (files)
            {
            if (*files[0].Name)
                {
                for (int DelIndex2 = 0; *files[DelIndex2].Name; DelIndex2++)
                    {
                    strupr(files[DelIndex2].Name);

                    int Response;

                    if (Continuous)
                        {
                        // Force "Yes"
                        Response = 0;
                        }
                    else
                        {
                        char string[128];
                        sprintf(string, getmsg(MENU_AIDE, 138), files[DelIndex2].Name);

                        Response = GetOneKey(string, getmsg(MENU_AIDE, 139), getmsg(MENU_AIDE, 139)[0], B_YNAC,
                                getmsg(521), getmsg(522), getmsg(653), getmsg(MENU_AIDE, 140));

                        OC.SetOutFlag(IMPERVIOUS);
                        }

                    if (Response == -1 || Response == 2)    // ESC or Abort
                        {
                        Abort = TRUE;
                        break;
                        }

                    if (Response == 0 || Response == 3) // Yes or Continuous
                        {
                        Bool Success = (unlink(files[DelIndex2].Name) == 0);

                        if (!Continuous)
                            {
                            doCR();
                            }

                        mPrintfCR(getmsg(MENU_AIDE, 141), ns, ns, files[DelIndex2].Name,
                                Success ? ns : getmsg(MENU_AIDE, 142));

                        amPrintf(getmsg(MENU_AIDE, 141), bn, spc, files[DelIndex2].Name,
                                Success ? ns : getmsg(MENU_AIDE, 142));

                        FilesUnlinked += Success;
                        }

                    if (Response == 3)  // Continuous
                        {
                        Continuous = TRUE;
                        }

                    }
                }
            else
                {
                CRmPrintfCR(getmsg(425), words[DelIndex]);
                }

            delete [] files;
            }
        else
            {
            OutOfMemory(17);
            }
        }

    CRmPrintfCR(getmsg(MENU_AIDE, 144), ltoac(FilesUnlinked), FilesUnlinked == 1 ? ns : justs);

    if (FilesUnlinked)
        {

        if(!(read_tr_messages()))
            {
            errorDisp(getmsg(172));
            return;
            }

#ifdef WINCIT
        trap(T_AIDE, WindowCaption, gettrmsg(30), FilesUnlinked == 1 ? ns : justs, filename,
                RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#else
        trap(T_AIDE, gettrmsg(30), FilesUnlinked == 1 ? ns : justs, filename,
                RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)), CurrentUser->GetName(Buffer, sizeof(Buffer)));
#endif

        dump_tr_messages();


        SaveAideMess
                (
                RoomTab[thisRoom].IsGroupOnly() ?
                    (
                    RoomTab[thisRoom].IsBooleanGroup() ?
                        (
                        GroupData[SPECIALSECURITY].GetName(Buffer, sizeof(Buffer))
                        )
                    :
                        (
                        GroupData[RoomTab[thisRoom].GetGroupNumber()].GetName(Buffer, sizeof(Buffer))
                        )
                    )
                :
                    (
                    NULL
                    )
                );
        }
    else
        {
        amZap();
        }

    UnlockMenu(MENU_AIDE);
    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// wcdown() calls xmodem downloading routines.
//
// Notes:
//  Currently, only called from ReadWithProtocol
//  Assumes MENU_READ is locked.
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER wcdown(const char *filename, InternalProtocols prot)
    {
    long transTime1, transTime2;

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    if (prot == IP_ZMODEM)
        {
        int count;
        char *words[256];
        char temp[256];

        CopyStringToBuffer(temp, filename);

        count = parse_it(words, temp);

        if (count == 0)
            {
            changedir(cfg.homepath);
            return;
            }

        for (int i = 0; i < count; i++)
            {
            if (!IsFilenameLegal(words[i], FALSE) || !filexists(words[i]) && !ambig(words[i]))
                {
                CRmPrintfCR(getmsg(425), words[i]);
                changedir(cfg.homepath);
                return;
                }
            }

        int total_files = 0;
        long total_bytes = 0;

        // Disambiguate
EXTRA_OPEN_BRACE
        for (int i = 0; i < count; i++)
            {
            directoryinfo *files = filldirectory(words[i], SORT_ALPHA, OldAndNew, FALSE);

            if (files)
                {
                for (int j = 0; *files[j].Name && OC.User.GetOutFlag() == OUTOK; j++)
                    {
                    total_files++;
                    total_bytes += files[j].Length;

                    blocks(files[j].Name, files[j].Length, 0);
                    }

                delete [] files;
                }
            }
EXTRA_CLOSE_BRACE
        OC.SetOutFlag(OUTOK);

        if (total_files > 1)
            {
            CRmPrintfCR(getmsg(MSG_TRANS, 14), ltoac(total_bytes));
            mPrintfCR(getmsg(MSG_TRANS, 15), dltime(total_bytes));
            }
        }
    else
        {
        if (!IsFilenameLegal(filename, FALSE) || !filexists(filename))
            {
            CRmPrintfCR(getmsg(425), filename);
            changedir(cfg.homepath);
            return;
            }

        if (ambig(filename))
            {
            CRmPrintfCR(getmsg(MSG_TRANS, 16));
            changedir(cfg.homepath);
            return;
            }

        // display # blocks & download time
        FILE *stream = fopen(filename, FO_R);
        const long length = filelength(fileno(stream));
        fclose(stream);

        blocks(filename, length, (prot == 2) ? 1024 : 128);
        }

    if (getYesNo(getmsg(MSG_TRANS, 19), 0))
        {
        time(&transTime1);
        doCR();

        if (prot == IP_ZMODEM)
            {
            Zmodem_Send_Batch(filename, (uint) connectbauds[CommPort->GetModemSpeed()]);
            }
        else
            {
#ifdef WINCIT
            XmodemC Xmodem(this);
#else
            XmodemC Xmodem;
#endif

            Xmodem.Send(filename, prot);
            }

        time(&transTime2);

        if (cfg.accounting && CurrentUser->IsAccounting() && !CurrentUserAccount->IsSpecialTime())
            {
            CurrentUserAccount->AfterTransfer(transTime1, transTime2, FALSE
#ifdef MULTI
                    , this
#endif
                    );
            }

        if(!read_tr_messages())
            {
            errorDisp("%s- %s", getmsg(172), getmsg(59));
            return;
            }

        if (prot == IP_ZMODEM)
            {
            label Buffer;
#ifdef WINCIT
            trap(T_DOWNLOAD, WindowCaption, gettrmsg(40), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
            trap(T_DOWNLOAD, gettrmsg(40), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        else
            {
            label Buffer;
#ifdef WINCIT
            trap(T_DOWNLOAD, WindowCaption, gettrmsg(41), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
            trap(T_DOWNLOAD, gettrmsg(41), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        }

    dump_tr_messages();
    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// wcup(): Calls xmodem uploading routines.
//
// Notes:
//  Currently, only called from EnterWithProtocol
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER wcup(const char *filename, InternalProtocols prot)
    {
    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    if (getYesNo(getmsg(MSG_TRANS, 19), 0))
        {
        long transTime1, transTime2;

        // when did they start the Upload
        time(&transTime1);
        doCR();

        if (prot == IP_ZMODEM)
            {
            Zmodem_Receive_Batch(ns, (uint) connectbauds[CommPort->GetModemSpeed()]);
            }
        else
            {
#ifdef WINCIT
            XmodemC Xmodem(this);
#else
            XmodemC Xmodem;
#endif

            Xmodem.Receive(filename, prot);
            }

        time(&transTime2);

        if (cfg.accounting && CurrentUser->IsAccounting() && !CurrentUserAccount->IsSpecialTime())
            {
            CurrentUserAccount->AfterTransfer(transTime1, transTime2, TRUE
#ifdef MULTI
                    , this
#endif
                    );
            }

        CommPort->FlushInput();

        if(!read_tr_messages())
            {
            errorDisp("%s- %s", getmsg(172), getmsg(59));
            return;
            }

        if (prot == IP_ZMODEM)
            {
            label Buffer;
#ifdef WINCIT
                trap(T_UPLOAD, WindowCaption, gettrmsg(42), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_UPLOAD, gettrmsg(42), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        else
            {
            label Buffer;
#ifdef WINCIT
                trap(T_UPLOAD, WindowCaption, gettrmsg(43), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_UPLOAD, gettrmsg(43), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        }

    dump_tr_messages();
    changedir(cfg.homepath);
    }

void TERMWINDOWMEMBER doTheDownload(const char *filename, const char *thePath, Bool fullPath,
        const protocols *theProt, Bool queue)
    {
    if (!LockMessages(MSG_TRANS))
        {
        OutOfMemory(18);
        return;
        }

    long transTime1, transTime2;

    const int c = toupper(DoMenuPrompt(getmsg(MSG_TRANS, 0), NULL));

                                        // old prompt compatible
    if (c == 'D' || c == 'H' || c == CR || c == LF || c == 'Y')
        {
        if (c == 'H')
            {
            mPrintf(getmsg(MSG_TRANS, 1));
            }

        mPrintfCR(getmsg(MSG_TRANS, 13));

        if (!fullPath && !queue)
            {
            char Buffer[64];
            FillFileInfo(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), ns, FALSE, NULL, FALSE);
            }

        time(&transTime1);

        wxsnd(thePath, filename, theProt, queue ? 1 : 0);

        time(&transTime2);

        if(!read_tr_messages())
            {
            errorDisp("%s- %s", getmsg(172), getmsg(59));
            return;
            }

        if (fullPath)
            {
            label Buffer;
#ifdef WINCIT
                trap(T_DOWNLOAD, WindowCaption, gettrmsg(44), theProt->name, filename,
                        RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_DOWNLOAD, gettrmsg(44), theProt->name, filename,
                        RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        else if (queue)
            {
            label Buffer;
#ifdef WINCIT
                trap(T_DOWNLOAD, WindowCaption, gettrmsg(45), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_DOWNLOAD, gettrmsg(45), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
            }
        else
            {
            Message *Msg = new Message;

            char Buffer1[64];
            label Buffer;
            if (Msg)
                {
                if (FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                        CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, Msg->GetTextPointer(), FALSE))
                    {
                    Msg->SetRoomNumber(thisRoom);
                    systemMessage(Msg);
#ifdef WINCIT
                    trap(T_UPLOAD, WindowCaption, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                    trap(T_UPLOAD, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                    }

                delete Msg;
                }
            else
                {
                if (FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                        CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, NULL, FALSE))
                    {
#ifdef WINCIT
                    trap(T_UPLOAD, WindowCaption, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                    trap(T_UPLOAD, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                    }
                }

#ifdef WINCIT
            trap(T_DOWNLOAD, WindowCaption, gettrmsg(47), theProt->name, filename);
#else
            trap(T_DOWNLOAD, gettrmsg(47), theProt->name, filename);
#endif
            }

        dump_tr_messages();

        if (cfg.accounting && CurrentUser->IsAccounting() && !CurrentUserAccount->IsSpecialTime())
            {
            CurrentUserAccount->AfterTransfer(transTime1, transTime2, FALSE
#ifdef MULTI
                    , this
#endif
                    );
            }

        changedir(thePath);

        if (c == 'H')
            {
            time_t t;
            time(&t);
            int i;
            int ch = -1;
            Bool do_hangup = TRUE;

            OC.SetOutFlag(IMPERVIOUS);

            CRCRmPrintfCR(getmsg(MSG_TRANS, 6));

            do
                {
                i = (char) (20L - (time(NULL) - t));

                if (i != ch)
                    {
                    if (cfg.countbeep)
                        {
                        mPrintf("\a");
                        }

                    mPrintf(i ? getmsg(MSG_TRANS, 7) : getmsg(MSG_TRANS, 8), i);
                    ch = i;
                    }

                if (BBSCharReady())
                    {
                    ch = toupper(iCharNE());
                    do_hangup = (ch == 'H');
                    break;
                    }
                } while (i);

            doCR();

            if (do_hangup)
                {
                MRO.Verbose = FALSE;
                terminate(TRUE);
                }
            }
        }
    else
        {
        mPrintfCR(getmsg(653));
        }

    UnlockMessages(MSG_TRANS);
    }


// --------------------------------------------------------------------------
// download(): Menu level download routine.
//
// Notes:
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER download(char c)
    {
    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    int ich;
    if (!c)
        {
        ich = iCharNE();
        }
    else
        {
        ich = c;
        }

    if (ich == '\n' || ich == '\r')
        {
        ich = CurrentUser->GetDefaultProtocol();
        }

    const protocols *theProt = GetProtocolByKey((char) ich);

    if (theProt)
        {
        char filename[81];
        int i;

        mPrintfCR(pcts, theProt->name);

        if (!CurrentUser->IsExpert())
            {
            dispBlb(B_WCDOWN);
            CurrentRoom->ChangeDir();   // it was okay above
            }

        getNormStr(getmsg(289), filename, (theProt->batch) ? 80 : 12);

        if (!*filename)
            {
            changedir(cfg.homepath);
            return;
            }

        if (theProt->batch)
            {
            char temp[81];
            char *words[256];

            CopyStringToBuffer(temp, filename);

            const int count = parse_it(words, temp);

            if (count == 0)
                {
                return;
                }

            for (i = 0; i < count; i++)
                {
                if (!IsFilenameLegal(words[i], FALSE) || !ambig(words[i]) && !filexists(words[i]))
                    {
                    CRmPrintfCR(getmsg(425), words[i]);
                    changedir(cfg.homepath);
                    return;
                    }
                }

            // Disambiguate
            int total_files = 0;
            long total_bytes = 0l;
            for (i = 0; i < count; i++)
                {
                directoryinfo *files = filldirectory(words[i], SORT_ALPHA, OldAndNew, FALSE);

                if (files)
                    {
                    int j;
                    for (j = 0; *files[j].Name && OC.User.GetOutFlag() == OUTOK; j++)
                        {
                        total_files++;
                        total_bytes += files[j].Length;

#ifdef WINCIT
                        blocks(files[j].FullName, files[j].Length, theProt->block);
#else
                        blocks(files[j].Name, files[j].Length, theProt->block);
#endif
                        }

                    if (!j)
                        {
                        CRmPrintfCR(getmsg(425), words[i]);
                        }

                    delete [] files;
                    }
                }
            OC.SetOutFlag(OUTOK);

            if (total_files > 1)
                {
                CRmPrintfCR(getmsg(MSG_TRANS, 14), ltoac(total_bytes));
                mPrintfCR(getmsg(MSG_TRANS, 15), dltime(total_bytes));
                }
            else if (!total_files)
                {
                changedir(cfg.homepath);
                return;
                }
            }
        else
            {
            if (!IsFilenameLegal(filename, FALSE) || !filexists(filename))
                {
                CRmPrintfCR(getmsg(425), filename);
                changedir(cfg.homepath);
                return;
                }

            if (ambig(filename))
                {
                CRmPrintfCR(getmsg(MSG_TRANS, 16));
                changedir(cfg.homepath);
                return;
                }

            // display # blocks & download time
            FILE *stream = fopen(filename, FO_R);
            const long length = filelength(fileno(stream));
            fclose(stream);

            blocks(filename, length, theProt->block);
            }

        char Buffer[64];
        doTheDownload(filename, CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), FALSE, theProt, FALSE);
        }
    else
        {
        if (ich == '?')
            {
            oChar('?');
            upDownMnu('D', FALSE);
            }
        else
            {
            oChar((char) ich);
            mPrintf(sqst);
            if (!CurrentUser->IsExpert())
                {
                upDownMnu('D', FALSE);
                }
            }
        }

    CommPort->FlushInput();
    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// upload(): Menu level routine.
//
// Notes:
//  Assumes MSG_TRANS is locked.

void TERMWINDOWMEMBER upload(char c, Bool HideIt)
    {
    int ch, ich;

    if (!c)
        {
        ch = tolower(ich = iCharNE());
        }
    else
        {
        ch = ich = c;
        }

    if (ch == '\n' || ch == '\r')
        {
        ch = (!CurrentUser->GetDefaultProtocol()) ? 0 : CurrentUser->GetDefaultProtocol();
        }

    const protocols *theProt = GetProtocolByKey((char) ch);

    if (theProt)
        {
        mPrintfCR(pcts, theProt->name);

        if (!CurrentUser->IsExpert())
            {
            dispBlb(B_WCUP);
            }

        CurrentRoom->ChangeDir();

#ifdef WINCIT
        const __int64 freespace = bytesfree();
#else
        const long freespace = bytesfree();
#endif

        if (freespace <= cfg.diskfree)
            {
#ifdef WINCIT
            CRmPrintfCR(getmsg(MSG_TRANS, 17), i64toac(freespace / 1024l));
#else
            CRmPrintfCR(getmsg(MSG_TRANS, 17), ltoac(freespace / 1024l));
#endif
            return;
            }

#ifdef WINCIT
        CRmPrintfCR(getmsg(MSG_TRANS, 18), i64toac(bytesfree()));
#else
        CRmPrintfCR(getmsg(MSG_TRANS, 18), ltoac(bytesfree()));
#endif

        ActuallyDoTheUpload(theProt, HideIt, TRUE);
        }
    else
        {
        if (ch == '?')
            {
            oChar('?');
            upDownMnu('U', FALSE);
            }
        else
            {
            oChar((char) ich);
            mPrintf(sqst);
            if (!CurrentUser->IsExpert())
                {
                upDownMnu('U', FALSE);
                }
            }
        }

    changedir(cfg.homepath);
    }


// --------------------------------------------------------------------------
// ActuallyDoTheUpload(): Actually does the upload. (Really.)

void TERMWINDOWMEMBER ActuallyDoTheUpload(const protocols *theProt, Bool HideIt, Bool Ask)
    {
    if (!LockMessages(MSG_TRANS))
        {
        OutOfMemory(19);
        return;
        }

    char comments[64];
    label filename;

    if (!theProt->batch)
        {
        getNormStr(getmsg(289), filename, LABELSIZE);

        if (!CanUploadFile(filename))
            {
            CurrentRoom->ChangeDir();
            UnlockMessages(MSG_TRANS);
            return;
            }

        getNormStr(getmsg(383), comments, 63);
        }
    else
        {
        char Buffer[64];
        FillFileInfo(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), ns, FALSE, NULL, FALSE);
        }

    if (!Ask || getYesNo(getmsg(MSG_TRANS, 19), 0))
        {
        long transTime1, transTime2;

        time(&transTime1);      // when did they start the Upload

        char Buffer[64];
        wxrcv(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), theProt->batch ? ns : filename, theProt);

        time(&transTime2);      // when did they get done

        if (cfg.accounting && CurrentUser->IsAccounting() && !CurrentUserAccount->IsSpecialTime())
            {
            CurrentUserAccount->AfterTransfer(transTime1, transTime2, TRUE
#ifdef MULTI
                    , this
#endif
                    );
            }

        CommPort->FlushInput();

        if (!theProt->batch)
            {
            if (HideIt && filexists(filename))
                {
                hide(filename);
                }

            if (filexists(filename))
                {
                label Buffer;
                char Buffer1[64];
                if (read_tr_messages())
                    {
#ifdef WINCIT
                    // Need short name for FILEINFO.DAT.
                    directoryinfo *files = filldirectory(filename, SORT_DATE, OldAndNew, TRUE);

                    if (files && *files[0].Name)
                        {
                        UpdateInfoFile(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)), files[0].Name,
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), comments, 0, 0);
                        }

                    delete [] files;

                    trap(T_UPLOAD, WindowCaption, gettrmsg(48), theProt->name, filename,
                            RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));

#else
                    UpdateInfoFile(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)), filename,
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), comments, 0, 0);

                    trap(T_UPLOAD, gettrmsg(48), theProt->name, filename,
                            RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                    dump_tr_messages();
                    }
                else
                    {
                    errorDisp("%s- %s", getmsg(172), getmsg(59));
                    }

                Message *Msg = new Message;

                if (Msg)
                    {
                    label Buffer;
                    if (*comments)
                        {
                        Msg->SetTextWithFormat(getmsg(MSG_TRANS, 23), filename, CurrentUser->GetName(Buffer,
                                sizeof(Buffer)), bn, comments);
                        }
                    else
                        {
                        Msg->SetTextWithFormat(getmsg(MSG_TRANS, 24), filename, CurrentUser->GetName(Buffer,
                                sizeof(Buffer)));
                        }

                    Msg->SetRoomNumber(thisRoom);
                    systemMessage(Msg);

                    delete Msg;
                    }
                else
                    {
                    OutOfMemory(19);
                    }
                }
            }
        else
            {
            if (read_tr_messages())
                {
#ifdef WINCIT
                trap(T_UPLOAD, WindowCaption, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_UPLOAD, gettrmsg(46), theProt->name, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                dump_tr_messages();
                }
            else
                {
                errorDisp("%s- %s", getmsg(172), getmsg(59));
                }


            Message *Msg = new Message;

            label Buffer;
            char Buffer1[64];
            if (Msg)
                {
                if (FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                        CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, Msg->GetTextPointer(), HideIt))
                    {
                    Msg->SetRoomNumber(thisRoom);
                    systemMessage(Msg);
                    }

                delete Msg;
                }
            else
                {
                FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                        CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, NULL, HideIt);

                OutOfMemory(19);
                }
            }
        }

    UnlockMessages(MSG_TRANS);
    }


// --------------------------------------------------------------------------
// XmodemMenu()
//
// Notes:
//  Assumes MSG_TRANS is locked.

InternalProtocols TERMWINDOWMEMBER XmodemMenu(char cmd)
    {
    int ch, ich;

    ch = toupper(ich = iCharNE());

    if (ch == '\n' || ch == '\r')
        {
        ch = 'X';
        }

    switch (ch)
        {
        case 'C':
            {
            mPrintf(getmsg(MSG_TRANS, 26));
            return (IP_CRCXMODEM);
            }

        case '1':
            {
            mPrintf(getmsg(MSG_TRANS, 5));
            return (IP_ONEKXMODEM);
            }

        case 'X':
            {
            mPrintf(getmsg(MSG_TRANS, 25));
            return (IP_XMODEM);
            }

        case 'Z':
            {
            mPrintf(getmsg(MSG_TRANS, 4));
            return (IP_ZMODEM);
            }

        case '?':
            {
            oChar('?');
            XMnu(cmd);
            return (IP_ERROR);
            }

        default:
            {
            oChar((char) ich);
            mPrintf(sqst);
            if (!CurrentUser->IsExpert())
                {
                XMnu(cmd);
                }

            return (IP_ERROR);
            }
        }
    }

void TERMWINDOWMEMBER XMnu(char cmd)
    {
    if (cmd == 'R')
        {
        showMenu(M_READWC);
        }
    else if (cmd == 'E')
        {
        showMenu(M_ENTERWC);
        }
    }

// Up/Down menu
void TERMWINDOWMEMBER upDownMnu(char cmd, Bool respOnly)
    {
    const MenuNames TheMenu = cmd == 'U' ? M_UPLOAD : respOnly ? M_RESPONSEDOWNLOAD : M_DOWNLOAD;

    if (getMnuOff(TheMenu, NULL) > 0)
        {
        showMenu(TheMenu);
        }
    else
        {
        doCR(2);

        for (protocols *theProt = extProtList; theProt; theProt = (protocols *) getNextLL(theProt))
            {
            if ((!respOnly || *theProt->respDown) && !theProt->NetOnly)
                {
                mPrintfCR(getmsg(640), cmd, theProt->CommandKey, theProt->MenuName);
                }
            }

        mPrintfCR(getmsg(474), cmd);
        }
    }

// --------------------------------------------------------------------------
// getNewOldFileDate()
//
// Notes:
//  Assumes MENU_READ is locked.

Bool TERMWINDOWMEMBER getNewOldFileDate(void)
    {
    label usrIn;
    datestruct ds;
    struct date f_date;
    struct time f_time;
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

        GetStringWithBlurb(getmsg(MENU_READ, 102), usrIn, LABELSIZE - 1, ns, B_DATESET);

        gdate(usrIn, &ds);
        } while (!ds.Date);


    f_date.da_year = ds.Year + 1900;        // i hate dostounix
    f_date.da_mon = (char) (ds.Month + 1);  // i hate dostounix
    f_date.da_day = (char) ds.Date;

    f_time.ti_min = 0;
    f_time.ti_hour = 0;
    f_time.ti_sec = 0;
    f_time.ti_hund = 0;

    MRO.CheckDate = dostounix(&f_date, &f_time);

    return (TRUE);
    }


// --------------------------------------------------------------------------
// AideMoveFile(): Move a file, copying its file info.

void TERMWINDOWMEMBER AideMoveFile(void)
    {
    if (!CheckDirectoryRoom())
        {
        return;
        }

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        return;
        }

    SetDoWhat(AIDEMOVE);

    doCR();

    if (!LockMenu(MENU_AIDE))
        {
        changedir(cfg.homepath);
        return;
        }

    label SourceFileName;
    getNormStr(getmsg(MENU_AIDE, 131), SourceFileName, LABELSIZE);

    // Source file checking...
    if (!*SourceFileName)
        {
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    strupr(SourceFileName);

    if (!IsFilenameLegal(SourceFileName, FALSE) || ambig(SourceFileName))
        {
        CRmPrintfCR(getmsg(382), SourceFileName);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!filexists(SourceFileName))
        {
        CRmPrintfCR(getmsg(425), SourceFileName);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    char Prompt[128];
    sprintf(Prompt, getmsg(MENU_AIDE, 116), cfg.Lroom_nym);

    label DestRoomName;
    getNormStr(Prompt, DestRoomName, LABELSIZE);

    if (!*DestRoomName)
        {
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    r_slot roomNo;

    if ((roomNo = RoomExists(DestRoomName)) == CERROR)
        {
        roomNo = PartialRoomExists(DestRoomName, thisRoom, FALSE);
        }

    if (roomNo == CERROR)
        {
        CRmPrintfCR(getmsg(368), cfg.Lroom_nym, DestRoomName);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!RoomTab[roomNo].IsMsDOSdir())
        {
        CRmPrintfCR(getmsg(MENU_AIDE, 117), DestRoomName, cfg.Lroom_nym);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    RoomC RoomBuffer;
    RoomBuffer.Load(roomNo);

    char DestinationPath[64];
    RoomBuffer.GetDirectory(DestinationPath, sizeof(DestinationPath));
    RoomBuffer.GetName(DestRoomName, sizeof(DestRoomName));

#ifdef WINCIT
    // Need original short name for FILEINFO.DAT.
    directoryinfo *files = filldirectory(SourceFileName, SORT_DATE, OldAndNew, TRUE);

    label Source_Short_Filename;
    label Source_Long_Filename;
    if (files && *files[0].Name)
        {
        CopyStringToBuffer(Source_Short_Filename, files[0].Name);
        CopyStringToBuffer(Source_Long_Filename, files[0].FullName);
        }

    delete [] files;
#endif

    if (!changedir(DestinationPath))
        {
        CRmPrintfCR(getmsg(362), DestRoomName);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    // Source file at destination?
    char DestFullFilePath[128];

#ifdef WINCIT
    sprintf(DestFullFilePath, sbs, DestinationPath, Source_Long_Filename);
#else
    sprintf(DestFullFilePath, sbs, DestinationPath, SourceFileName);
#endif

    if (filexists(DestFullFilePath))
        {
        CRmPrintfCR(getmsg(346), DestFullFilePath);
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }

    if (!CurrentRoom->ChangeDir())
        {
        label Buffer;
        CRmPrintfCR(getmsg(362), RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
        changedir(cfg.homepath);
        UnlockMenu(MENU_AIDE);
        return;
        }


#ifdef WINCIT
    sprintf(Prompt, getmsg(MENU_AIDE, 118), Source_Long_Filename,
#else
    sprintf(Prompt, getmsg(MENU_AIDE, 118), SourceFileName,
#endif
         cfg.Lroom_nym, DestRoomName);

    if (getYesNo(Prompt, 1))
        {
        fInfo FileInfo;
        label Uploader;
        char FileComment[65];
        short Downloads;
        long UploadTime;


#ifdef WINCIT
        sprintf(DestFullFilePath, sbs, DestinationPath, Source_Long_Filename);
        if (rename(Source_Long_Filename, DestFullFilePath) != 0)
#else
        if (rename(SourceFileName, DestFullFilePath) != 0)
#endif
            {
            CRmPrintf(getmsg(MENU_AIDE, 119));

#ifdef WINCIT
            if (copyfile(Source_Long_Filename, DestFullFilePath))
#else
            if (copyfile(SourceFileName, DestFullFilePath))
#endif
                {
                doCR();
                unlink(SourceFileName);
                }
            else
                {
                CRCRmPrintfCR(getmsg(MENU_AIDE, 120), SourceFileName);
                changedir(cfg.homepath);
                UnlockMenu(MENU_AIDE);
                return;
                }
            }

        char RoomDir[64];
        CurrentRoom->GetDirectory(RoomDir, sizeof(RoomDir));
#ifdef WINCIT
        if (GetSingleFileInfo(RoomDir, Source_Short_Filename, &FileInfo))
#else
        if (GetSingleFileInfo(RoomDir, SourceFileName, &FileInfo))
#endif
            {
            CopyStringToBuffer(Uploader, FileInfo.uploader);
            CopyStringToBuffer(FileComment, FileInfo.comment);
            Downloads = FileInfo.downloads;
            UploadTime = FileInfo.uploadtime;
            }
        else
            {
            *FileComment = 0;
            *Uploader = 0;
            Downloads = 0;
            UploadTime = 0;
            }

#ifdef WINCIT
        if (!changedir(DestinationPath))
            {
            CRmPrintfCR(getmsg(362), DestRoomName);
            changedir(cfg.homepath);
            UnlockMenu(MENU_AIDE);
            return;
            }

        // Need short name for FILEINFO.DAT.
#ifdef WINCIT
        files = filldirectory(Source_Long_Filename, SORT_DATE, OldAndNew, TRUE);
#else
        files = filldirectory(SourceFileName, SORT_DATE, OldAndNew, TRUE);
#endif

        if (files && *files[0].Name)
            {
            UpdateInfoFile(DestinationPath, files[0].Name, Uploader, FileComment, Downloads, UploadTime);
            }

        // This was a bug! It might have had a different short file name in the original room!
        RemoveFileInfo(RoomDir, Source_Short_Filename);

        delete [] files;
#else
        UpdateInfoFile(DestinationPath, SourceFileName, Uploader, FileComment, Downloads, UploadTime);
        RemoveFileInfo(RoomDir, SourceFileName);
#endif

        char TrapString[256];

        label Buffer, Buffer1;
        sprintf(TrapString, getmsg(MENU_AIDE, 121), SourceFileName, DestRoomName,
                RoomTab[thisRoom].GetName(Buffer1, sizeof(Buffer1)), CurrentUser->GetName(Buffer, sizeof(Buffer)));

#ifdef WINCIT
        trap(T_AIDE, WindowCaption, pcts, TrapString);
#else
        trap(T_AIDE, pcts, TrapString);
#endif

        Message *Msg = new Message;

        if (Msg)
            {
            Msg->SetText(TrapString);
            Msg->SetRoomNumber(AIDEROOM);
            systemMessage(Msg);

            delete Msg;
            }
        else
            {
            OutOfMemory(20);
            }

        sprintf(Prompt, getmsg(MENU_AIDE, 123), cfg.Lmsg_nym, cfg.Lroom_nym);

        if (getYesNo(Prompt, 1))
            {
            Msg = new Message;

            if (Msg)
                {
                long length;
                const int handle = open(DestFullFilePath, O_RDONLY);

                if (handle >= 0)
                    {
                    length = filelength(handle);
                    close(handle);
                    }
                else
                    {
                    length = 0;
                    }

                Msg->SetTextWithFormat(getmsg(MENU_AIDE, 124),
#ifdef WINCIT
                        cfg.Lroom_nym, bn, Source_Long_Filename, ltoac(length),
#else
                        cfg.Lroom_nym, bn, SourceFileName, ltoac(length),
#endif
                        length == 1 ? ns : justs, bn, FileComment);

                Msg->SetRoomNumber(roomNo);
                systemMessage(Msg);

                delete Msg;
                }
            else
                {
                OutOfMemory(227);
                }
            }
        }

    UnlockMenu(MENU_AIDE);
    changedir(cfg.homepath);
    }

protocols *GetProtocolByKey(char Key, Bool Network)
    {
    protocols *theProt;

    for (theProt = extProtList; theProt; theProt = (protocols *) getNextLL(theProt))
        {
        if (tolower(Key) == tolower(theProt->CommandKey) && (Network || !theProt->NetOnly))
            {
            break;
            }
        }

    return (theProt);
    }


// Assumes MSG_TRANS is locked.

Bool TERMWINDOWMEMBER IsFileDownloadOkay(void)
    {
    if (!CheckDirectoryRoom())
        {
        return (FALSE);
        }

    if (!loggedIn && !cfg.unlogReadOk)
        {
        CRCRmPrintfCR(getmsg(239));
        return (FALSE);
        }

    if (!CurrentUser->IsDownload())
        {
        CRCRmPrintfCR(getmsg(MSG_TRANS, 9), getmsg(MSG_TRANS, 11));
        return (FALSE);
        }

    if (RoomTab[thisRoom].IsUploadOnly() && !CurrentUser->HasRoomPrivileges(thisRoom))
        {
        CRCRmPrintfCR(getmsg(MSG_TRANS, 12), cfg.Uroom_nym, getmsg(MSG_TRANS, 10));
        return (FALSE);
        }

    return (TRUE);
    }



// Assumes MSG_TRANS is locked.

Bool TERMWINDOWMEMBER IsFileUploadOkay(void)
    {
    if (!CheckDirectoryRoom())
        {
        return (FALSE);
        }

    if (!loggedIn && !cfg.unlogEnterOk)
        {
        CRCRmPrintfCR(getmsg(239));
        return (FALSE);
        }

    if (!CurrentUser->IsUpload())
        {
        CRCRmPrintfCR(getmsg(MSG_TRANS, 9), getmsg(MSG_TRANS, 10));
        return (FALSE);
        }

    if (RoomTab[thisRoom].IsDownloadOnly() && !CurrentUser->HasRoomPrivileges(thisRoom))
        {
        CRCRmPrintfCR(getmsg(MSG_TRANS, 12), cfg.Uroom_nym, getmsg(MSG_TRANS, 11));
        return (FALSE);
        }

    return (TRUE);
    }

void TERMWINDOWMEMBER AideFileInfoSet(void)
    {
    SetDoWhat(AIDEFILESET);

    if (CheckDirectoryRoom())
        {
        Message *Msg = new Message;

        if (Msg)
            {
            doCR();

            fpath Buffer1;
            label Buffer2;

            if (FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                    CurrentUser->GetName(Buffer2, sizeof(Buffer2)), TRUE, Msg->GetTextPointer(), FALSE))
                {
                Msg->SetRoomNumber(thisRoom);
                systemMessage(Msg);
                }

            delete Msg;
            }
        else
            {
            OutOfMemory(29);
            }
        }
    }


// --------------------------------------------------------------------------
// EnterWithProtocol(): Menu level .EW.

void TERMWINDOWMEMBER EnterWithProtocol(Bool HideIt)
    {
    if (!LockMenu(MENU_ENTER))
        {
        return;
        }

    if (LockMessages(MSG_TRANS))
        {
        if (IsFileUploadOkay())
            {
            label filename;

            const InternalProtocols prot = XmodemMenu('E');

            if (prot == IP_ERROR)
                {
                UnlockMessages(MSG_TRANS);
                UnlockMenu(MENU_ENTER);
                return;
                }

            SetDoWhat(ENTERWC);

            if (!CurrentUser->IsExpert())
                {
                dispBlb(B_WCUP);
                }

            doCR();
            if (prot != IP_ZMODEM)
                {
                getNormStr(getmsg(289), filename, LABELSIZE);

                if (!CanUploadFile(filename))
                    {
                    UnlockMessages(MSG_TRANS);
                    UnlockMenu(MENU_ENTER);
                    return;
                    }
                }

            if (prot == IP_ZMODEM || *filename)
                {
                char Buffer1[64];
                FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)), ns, FALSE, NULL, FALSE);

                wcup(filename, prot);

                label Buffer;
                if(read_tr_messages())
                    {
#ifdef WINCIT
                    trap(T_UPLOAD, WindowCaption, gettrmsg(46), GetInternalProtocolName(prot),
                            RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                    trap(T_UPLOAD, gettrmsg(46), GetInternalProtocolName(prot),
                            RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif
                    dump_tr_messages();
                    }
                else
                    {
                    errorDisp("%s- %s", getmsg(172), getmsg(59));
                    }

                Message *Msg = new Message;

                if (Msg)
                    {
                    if (FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, Msg->GetTextPointer(), HideIt))
                        {
                        Msg->SetRoomNumber(thisRoom);
                        systemMessage(Msg);
                        }

                    delete Msg;
                    }
                else
                    {
                    FillFileInfo(CurrentRoom->GetDirectory(Buffer1, sizeof(Buffer1)),
                            CurrentUser->GetName(Buffer, sizeof(Buffer)), TRUE, NULL, HideIt);
                    }
                }

            changedir(cfg.homepath);
            }

        UnlockMessages(MSG_TRANS);
        }
    else
        {
        OutOfMemory(119);
        }

    UnlockMenu(MENU_ENTER);
    }
