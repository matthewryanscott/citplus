// --------------------------------------------------------------------------
// Citadel: ZipFile.CPP
//
// Archive file handling routines.

#include "ctdl.h"
#pragma hdrstop

#include "filecmd.h"
#include "room.h"
#include "msg.h"
#include "log.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// readzip()            menu level .rz HIGH level routine


// --------------------------------------------------------------------------
// ReadArchive(): Menu level .rz HIGH level routine.
//
// Notes:
//  Assumes MENU_READ is locked.

void TERMWINDOWMEMBER ReadArchive(OldNewPick which)
    {
    if (LockMenu(MENU_READ))
        {
        if (CheckDirectoryRoom())
            {
            if (which == WhoKnows)
                {
                which = OldAndNew;
                }

            SetDoWhat(READZIP);

            if (!CurrentRoom->ChangeDir())
                {
                return;
                }

            const int ich = iCharNE();

            archivers *theArc;
            for (theArc = arcList; theArc; theArc = (archivers *) getNextLL(theArc))
                {
                if (tolower(ich) == tolower(theArc->name[0]))
                    {
                    mPrintf(pcts, theArc->name);
                    break;
                    }
                }

            if (theArc)
                {
                doCR();

                label filename;
                if (!getNormStr(getmsg(289), filename, LABELSIZE))
                    {
                    return;
                    }

                if (!*filename)
                    {
                    sprintf(filename, getmenumsg(MENU_READ, 38), theArc->ext);
                    }

                if (!strchr(filename, '.'))
                    {
                    strcat(filename, getmsg(433));
                    strcat(filename, theArc->ext);
                    }

                if (which != OldAndNew)
                    {
                    if (!getNewOldFileDate())
                        {
                        return;
                        }
                    }

                directoryinfo *files = filldirectory(filename, MRO.Date ? SORT_DATE :
                        SORT_ALPHA, which, MRO.Reverse);

                if (files)
                    {
                    int entries;
                    for (entries = 0; *files[entries].Name; ++entries);

                    int i;
#ifdef MULTI
                    for (i = 0; i < entries && !OC.CheckInput(FALSE, this) &&
#else
                    for (i = 0; i < entries && !OC.CheckInput(FALSE) &&
#endif
                            (OC.User.GetOutFlag() != OUTSKIP); i++)
                        {
                        RunApplication(theArc->view, files[i].Name, TRUE, FALSE);

                        CurrentRoom->ChangeDir();

                        if (MRO.DotoMessage == REVERSE_READ)
                            {
                            MRO.DotoMessage = NO_SPECIAL;
                            OC.SetOutFlag(OUTOK);

                            MRO.Reverse = !MRO.Reverse;
                            qsort(files, i, sizeof(*files), (MRO.Reverse) ?
                                    (QSORT_CMP_FNP) revstrcmp : (QSORT_CMP_FNP) fwdstrcmp);
                            CRmPrintfCR(getmsg(357), (!MRO.Reverse) ? '+' : '-');
                            i = entries - i - 1;
                            }
                        }

                    if (!i)
                        {
                        CRmPrintfCR(getmsg(425), filename);
                        }

                    delete [] files;
                    }

                label Buffer;
#ifdef WINCIT
                trap(T_DOWNLOAD, WindowCaption, getmenumsg(MENU_READ, 39), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#else
                trap(T_DOWNLOAD, getmenumsg(MENU_READ, 39), filename, RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer)));
#endif

                }
            else
                {
                oChar((char) ich);

                if (ich != '?')
                    {
                    mPrintf(sqst);
                    }

                if (!CurrentUser->IsExpert() || ich == '?')
                    {
                    doCR(2);

                    for (theArc = arcList; theArc; theArc = (archivers *) getNextLL(theArc))
                        {
                        mPrintfCR(getmenumsg(MENU_READ, 40), *(theArc->name), (theArc->name + 1));
                        }

                    mPrintfCR(getmenumsg(MENU_READ, 41));
                    }
                }

            changedir(cfg.homepath);
            }
        }
    else
        {
        OutOfMemory(121);
        }
    }
