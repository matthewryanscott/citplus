// readinfo.cpp - temporary? I don't know.

#include "ctdl.h"
#pragma hdrstop

#include "infofile.h"
#include "room.h"
#include "filecmd.h"
#include "msg.h"
#include "log.h"
#include "events.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// showinfo(): Display info-buffer.

void TERMWINDOWMEMBER showinfo(directoryinfo *files, CITHANDLE FileInfo)
    {
    if (LockMessages(MSG_INFOFILE))
        {
        const Bool Skinny = CurrentUser->GetWidth() < 41;
        int i;
        int entries;

        assert(files != NULL);
        assert(FileInfo != NULL);

        SetOutOK();

        if (!MRO.Verbose)
            {
            if (!Skinny)
                {
                CRmPrintfCR(getmsg(MSG_INFOFILE, 1));
                }
            else
                {
                CRmPrintfCR(getmsg(MSG_INFOFILE, 2));
                mPrintfCR(getmsg(MSG_INFOFILE, 3));
                }
            }

        for (entries = 0; *files[entries].Name; ++entries);

        for (i = 0;
                (i < entries && (OC.User.GetOutFlag() != OUTSKIP) &&
#ifdef MULTI
                        !OC.CheckInput(FALSE, this));
#else
                        !OC.CheckInput(FALSE));
#endif
                ++i)
            {
            label uploader;
            char comment[65], filename[13], size[20], downtime[20], date[20];
            char ftime[20];
            fInfo *slot;

            CopyStringToBuffer(filename, files[i].Name);
            getdstamp(date, files[i].DateTime);
            gettstamp(ftime, files[i].DateTime);
            CopyStringToBuffer(downtime, dltime(files[i].Length));
            CopyStringToBuffer(size, ltoac(files[i].Length));

            if (GetInfoByName(FileInfo, filename, &slot))
                {
                CopyStringToBuffer(comment, slot->comment);
                CopyStringToBuffer(uploader, slot->uploader);
                }
            else
                {
                comment[0] = 0;
                uploader[0] = 0;
                }

            if (    // Not .RK or text matches
                    (
                    !rf.Text || IsSubstr(deansi(comment), rf.SearchText) ||
                            IsSubstr(deansi(uploader), rf.SearchText) ||
                            IsSubstr(filename, rf.SearchText)
                    )
                &&
                    // And not .RB or user matches
                    (
                    !rf.User || u_match(deansi(uploader), rf.SearchUser)
                    )
                )
                {
                if (MRO.Verbose)
                    {
#ifdef WINCIT
                    CRmPrintf(getmsg(MSG_INFOFILE, 4), Skinny ? 19 : 16,
                            getmsg(MSG_INFOFILE, 22), filename);
#else
                    CRmPrintf(getmsg(MSG_INFOFILE, 4), Skinny ? 19 : 16,
                            getmsg(MSG_INFOFILE, 5), filename);
#endif

                    if (Skinny)
                        {
                        doCR();
                        }

                    mPrintf(getmsg(MSG_INFOFILE, 6), Skinny ? 19 : 17,
                            getmsg(MSG_INFOFILE, 7), size, ns);

                    if (Skinny)
                        {
                        CRmPrintf(getmsg(MSG_INFOFILE, 4), 19,
                                getmsg(MSG_INFOFILE, 8), downtime);
                        }
                    else
                        {
                        mPrintf(getmsg(MSG_INFOFILE, 9), downtime);
                        }

                    CRmPrintf(getmsg(MSG_INFOFILE, 10), Skinny ? 19 : 16,
                            getmsg(MSG_INFOFILE, 11), date, ftime);

                    if (Skinny)
                        {
                        doCR();
                        }

                    mPrintfCR(getmsg(MSG_INFOFILE, 6), Skinny ? 19 : 17,
                            getmsg(MSG_INFOFILE, 12), uploader, ns);

#ifdef WINCIT
                    mPrintfCR(getmsg(MSG_INFOFILE, 6), Skinny ? 20 : 17,
                            getmsg(MSG_INFOFILE, 23), Skinny ?
                            getmsg(MSG_INFOFILE, 24) :
                            getmsg(MSG_INFOFILE, 25), files[i].FullName);
#endif

                    mPrintfCR(getmsg(MSG_INFOFILE, 6), Skinny ? 20 : 17,
                            getmsg(MSG_INFOFILE, 13), Skinny ?
                            getmsg(MSG_INFOFILE, 24) :
                            getmsg(MSG_INFOFILE, 25), comment);
                    }
                else
                    {
                    mPrintf(getmsg(MSG_INFOFILE, 14), filename, size);

                    if (Skinny)
                        {
                        doCR();
                        }
                    else
                        {
                        OC.HangingIndent = atoi(getmsg(MSG_INFOFILE, 15));
                        }

                    mPrintfCR(pcts, comment);

                    if (Skinny)
                        {
                        doCR();
                        }
                    }
                }

            if (MRO.DotoMessage == REVERSE_READ)
                {
                MRO.DotoMessage = NO_SPECIAL;
                SetOutOK();

                MRO.Reverse = !MRO.Reverse;

                qsort(files, entries, sizeof(*files),
                        (MRO.Reverse) ? (QSORT_CMP_FNP) revstrcmp :
                        (QSORT_CMP_FNP) fwdstrcmp);

                mPrintfCR(getmsg(357), (!MRO.Reverse) ? '+' : '-');
                i = entries - (i + 1);
                }
            }

        UnlockMessages(MSG_INFOFILE);
        }
    else
        {
        OutOfMemory(109);
        }
    }


// --------------------------------------------------------------------------
// ReadInfoFile(): Menu level .RI routine.

void TERMWINDOWMEMBER ReadInfoFile(Bool ask, OldNewPick which)
    {
    if (!CheckDirectoryRoom())
        {
        return;
        }

    label filename;
    directoryinfo *files;
    CITHANDLE FileInfo;

    if (!CurrentRoom->ChangeDir())
        {
        return;
        }

    SetDoWhat(READINFO);

    doEvent(EVT_READINFO);

    if (ask)
        {
        doCR();

        if (!getNormStr(getmsg(289), filename, LABELSIZE - 2))
            {
            return;
            }
        }
    else
        {
        *filename = 0;
        doCR();
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
        strcat(filename, getmenumsg(MENU_READ, 27));
        }
    else if (*filename == '.')
        {
        label filename2;
        CopyStringToBuffer(filename2, filename);
        *filename = '*';
        CopyString2Buffer(filename + 1, filename2, sizeof(filename) - 1);
        }

    if (which != OldAndNew)
        {
        if (!getNewOldFileDate())
            {
            UnlockMenu(MENU_READ);
            return;
            }
        }

    rf.Group = FALSE;   // You cannot .RGI

    if (ProcessReadFilter(&rf, FALSE))
        {
        files = filldirectory(filename, MRO.Date ? SORT_DATE : SORT_ALPHA,
                which, MRO.Reverse);

        // check for matches
        char Buffer[64];
        if (!files || !*files[0].Name)
            {
            CRmPrintfCR(getmsg(425), filename);
            }
        else if (LoadFileInfoForDirectory(CurrentRoom->GetDirectory(Buffer,
                sizeof(Buffer)), &FileInfo))
            // allocate & read in fileinfo buffer
            {
            showinfo(files, FileInfo);

            DisposeFileInfo(FileInfo);
            }

        delete [] files;
        }

    UnlockMenu(MENU_READ);
    changedir(cfg.homepath);
    }
