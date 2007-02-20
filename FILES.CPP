// --------------------------------------------------------------------------
// Citadel: Files.CPP
//
// File handling routines.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "msg.h"
#include "log.h"
#include "extmsg.h"
#include "filefind.h"


// --------------------------------------------------------------------------
// Contents
//
// ambig()              returns true if filename is ambiguous
// ambigUnlink()        unlinks ambiguous filenames
// bytesfree()          returns #bytes free on current drive
// CitGetDiskFreeSpace()returns #bytes free on drive.
// IsFilenameLegal()    Is it?
// dltime()             computes dl time from size & global rate
// filldirectory()      fills our directory structure
// hide()               hides a file. for limited-access u-load
// copyfile()           Copy file function. (and test code)

Bool haveRespProt(void)
    {
    for (const protocols *theProt = extProtList; theProt;
            theProt = (protocols *) getNextLL(theProt))
        {
        if (theProt->respDown[0])
            {
            return (TRUE);
            }
        }

    return (FALSE);
    }

const char *nameonly(const char *full)
    {
    if (strrchr(full, '\\'))
        {
        return (strrchr(full, '\\') + 1);
        }
    else
        {
        return (full);
        }
    }


// --------------------------------------------------------------------------
// ambig(): Returns TRUE if string is an ambiguous filename.

int ambig(const char *filename)
    {
    return (strchr(filename, '*') || strchr(filename, '?'));
    }


// --------------------------------------------------------------------------
// ambigUnlink(): This routine will fill the directory structure according to
//  wildcard.

int ambigUnlink(const char *filename, const char *directory)
    {
    int count = 0;

    if (directory && !changedir(directory))
        {
        return (0);
        }

    // keep going till it errors, which is end of directory
    FindFileC FF;
    if (FF.FindFirst(starstar))
        {
        do
            {
            if (FF.IsNormalFile())
                {
                // + 1 for strcat() call below
#ifdef WINCIT
                char Name[MAX_PATH + 1];
#else
                char Name[_MAX_PATH + 1];
#endif

                CopyStringToBuffer(Name, FF.GetFullName());

                if (!strpos('.', Name))
                    {
                    strcat(Name, getmsg(433));
                    }

                // filename match wildcard?
                if (u_match(Name, filename))
                    {
                    if (!unlink(Name))
                        {
                        count++;
                        }
                    }
                }
            } while (FF.FindNext());
        }

    return (count);
    }

// --------------------------------------------------------------------------
// CitGetDiskFreeSpace(): Returns # bytes free on drive.

#ifdef WINCIT
typedef BOOL (CALLBACK* LPGETDISKFREESPACEEX)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER,
                                                    PULARGE_INTEGER);

__int64 CitGetDiskFreeSpace(char *path)
    {
    LPGETDISKFREESPACEEX lpGetDiskFreeSpaceEx;

    lpGetDiskFreeSpaceEx = (LPGETDISKFREESPACEEX)GetProcAddress(GetModuleHandle("kernel32.dll"),
                          "GetDiskFreeSpaceExA");

    if (lpGetDiskFreeSpaceEx)
        {
        // variable to receive free bytes on disk available to the caller
        ULARGE_INTEGER FreeBytesAvailableToCaller;

        // variable to receive number of bytes on disk
        ULARGE_INTEGER TotalNumberOfBytes;

        // variable to receive free bytes on disk
        ULARGE_INTEGER TotalNumberOfFreeBytes;

        if (lpGetDiskFreeSpaceEx(path, &FreeBytesAvailableToCaller,
            &TotalNumberOfBytes, &TotalNumberOfFreeBytes))
            {
            return(TotalNumberOfFreeBytes.QuadPart);
            }
        else
            {
            return (0);
            }
        }
    else
        {
        DWORD SectorsPerCluster, BytesPerSector, FreeClusters, Clusters;

        if (GetDiskFreeSpace(path, &SectorsPerCluster, &BytesPerSector,
            &FreeClusters, &Clusters))
            {
            return (FreeClusters * SectorsPerCluster * BytesPerSector);
            }
        else
            {
            return (0);
            }
        }
}
#endif

// --------------------------------------------------------------------------
// bytesfree(): Returns # bytes free on drive.

#ifdef WINCIT
__int64 bytesfree(void)
    {
    return (CitGetDiskFreeSpace(NULL));
    }
#else
long bytesfree(void)
    {
    char path[64];
    union REGS REG;

    getcwd(path, 64);

    REG.h.ah = 0x36;    // select drive

    REG.h.dl = (uchar)(path[0] - '@');

    intdos(&REG, &REG);

    return ((long) ((long) REG.x.cx * (long) REG.x.ax * (long) REG.x.bx));
    }
#endif

// --------------------------------------------------------------------------
// IsFilenameLegal(): Is it?

#ifdef WINCIT
Bool IsFilenameLegal(const char *filename, Bool)
#else
Bool IsFilenameLegal(const char *filename, Bool AllowSpace)
#endif
    {
    char *s;
    FILE *fl;

#ifdef WINCIT
    const char *invalid = getmsg(338);
#else
    const char *invalid = getmsg(641);

    if (AllowSpace)
        {
        invalid++;
        }
#endif

    if (strpbrk(invalid, filename) != NULL)
        {
        return (FALSE);
        }

    char device[14];
    CopyStringToBuffer(device, filename);

    if ((s = strchr(device, '.')) != NULL)
        {
        *s = '\0';
        }

    if ((fl = fopen(filename, FO_RB)) == NULL)
        {
        return (TRUE);
        }

    if (isatty(fileno(fl)))
        {
        fclose(fl);
        return (FALSE);
        }

    fclose(fl);
    return (TRUE);
    }

#ifdef MINGW
int fwdstrcmp(void *a, void *b)
#else
int _cdecl fwdstrcmp(void *a, void *b)
#endif
    {
#ifdef WINCIT
    return (strcmp(((directoryinfo *) a)->FullName,
            ((directoryinfo *) b)->FullName));
#else
    return (strcmp(((directoryinfo *) a)->Name,
            ((directoryinfo *) b)->Name));
#endif
    }

#ifdef MINGW
int revstrcmp(void *a, void *b)
#else
int _cdecl revstrcmp(void *a, void *b)
#endif
    {
    return (-fwdstrcmp(a, b));
    }

static int _cdecl old_to_new_cmp(void *a, void *b)
    {
    if (((directoryinfo *) a)->DateTime > ((directoryinfo *) b)->DateTime)
        {
        return (1);
        }
    else if (((directoryinfo *) a)->DateTime < ((directoryinfo *) b)->DateTime)
        {
        return (-1);
        }
    else
        {
        return (0);
        }
    }

static int _cdecl new_to_old_cmp(void *a, void *b)
    {
    return (-old_to_new_cmp(a, b));
    }


// --------------------------------------------------------------------------
// filldirectory(): This routine will fill the directory structure according
//  to wildcard

directoryinfo *TERMWINDOWMEMBER filldirectory(const char *temp, SortE Sort,
        OldNewPick which, Bool rev)
    {
    // to avoid possible memory corruption later
#ifdef WINCIT
    char filename[MAX_PATH];
#else
    char filename[_MAX_PATH];
#endif
    CopyStringToBuffer(filename, temp);
    if (!strpos('.', filename) && (strlen(filename) < sizeof(filename) - 1))
        {
        strcat(filename, getmsg(433));
        }

    stripdoubleasterisk(filename);

    directoryinfo *files = new directoryinfo[cfg.maxfiles];

    if (files == NULL)
        {
        mPrintfCR(getmsg(447));
        return (NULL);
        }

    const Bool HiddenOK = CurrentUser->IsAide();

    // keep going till it errors, which is end of directory
    int i = 0;
    FindFileC FF;
    if (FF.FindFirst(starstar))
        {
        do
            {
            if (FF.IsNormalFile(HiddenOK))
                {
                if (which == NewOnly || which == OldOnly)
                    {
                    time_t funix = FF.GetDateTime();

                    if (which == NewOnly)
                        {
                        if (funix < MRO.CheckDate)
                            {
                            //i--;
                            continue;
                            }
                        }
                    else
                        {
                        if (funix > MRO.CheckDate)
                            {
                            //i--;
                            continue;
                            }
                        }
                    }

                label Name;
                CopyStringToBuffer(Name, FF.GetShortName());

#ifdef WINCIT
                char FullName[MAX_PATH];
                CopyStringToBuffer(FullName, FF.GetFullName());
#endif

                Bool strip;

                if (!strpos('.', Name))
                    {
                    strcat(Name, getmsg(433));
#ifdef WINCIT
                    strcat(FullName, getmsg(433));
#endif
                    strip = TRUE;
                    }
                else
                    {
                    strip = FALSE;
                    }

                if (u_match(Name, filename)
#ifdef WINCIT
                        || u_match(FullName, filename)
#endif
                        )
                    {
                    if (strip)
                        {
                        Name[strlen(Name) - 1] = 0;
#ifdef WINCIT
                        FullName[strlen(FullName) - 1] = 0;
#endif
                        }

                    files[i].Attr = FF.GetAttr();
                    files[i].DateTime = FF.GetDateTime();
                    files[i].Length = FF.GetSize();
                    CopyStringToBuffer(files[i].Name, Name);
                    strlwr(files[i].Name);

#ifdef WINCIT
                    CopyStringToBuffer(files[i].FullName, FullName);
#endif

                    i++;
                    }
                }
            } while ((i + 1 < cfg.maxfiles) && FF.FindNext());
        }

    *files[i].Name = 0;
#ifdef WINCIT
    *files[i].FullName = 0;
#endif

    // alphabetical order
    if (Sort == SORT_ALPHA)
        {
        qsort(files, i, sizeof(directoryinfo),
                (rev) ? (QSORT_CMP_FNP) revstrcmp :
                (QSORT_CMP_FNP) fwdstrcmp);
        }

    // by date
    else if (Sort == SORT_DATE)
        {
        qsort(files, i, sizeof(directoryinfo),
                (rev) ? (QSORT_CMP_FNP) new_to_old_cmp :
                (QSORT_CMP_FNP) old_to_new_cmp);
        }

    return (files);
    }


// --------------------------------------------------------------------------
// dltime(): Give this routine the size of your file and it will return the
//  amount of time it will take to download according to speed.

const char * TERMWINDOWMEMBER dltime(long size)
    {
    long seconds;

    // could be more accurate
    if (CommPort->HaveConnection())
        {
        seconds = (size / (connectbauds[CommPort->GetModemSpeed()] / 10L));
        }
    else
        {
        seconds = (size / (bauds[CommPort->GetSpeed()] / 10L));
        }

    const long hours = (seconds / 3600);
    seconds %= 3600;
    const long minutes = (seconds / 60);
    seconds %= 60;

    sprintf(dlTimeBuff, getmsg(642), hours, minutes, seconds);

    return (dlTimeBuff);
    }


// --------------------------------------------------------------------------
// hide(): Hides a file. for limited-access u-load.

void hide(const char *filename)
    {
#ifdef WINCIT
    SetFileAttributes(filename, GetFileAttributes(filename) | FA_HIDDEN);
#else
    _chmod(filename, 1, _chmod(filename, 0) | FA_HIDDEN);
#endif
    }

#define MAXBUF  32*1024 // 32K


// --------------------------------------------------------------------------
// copyfile(): Copy file function. (and test code).

Bool copyfile(const char *source, const char *dest)
    {
    FILE *sfl, *dfl;

    if ((sfl = fopen(source, FO_RB)) == NULL)
        {
#ifndef WINCIT
        if (debug)
            {
            doccr();
            cPrintf(getdbmsg(87), source, dest);
            cPrintf(getdbmsg(14));
            doccr();
            }
#endif

        return (FALSE);
        }

    if ((dfl = fopen(dest, FO_WB)) == NULL)
        {
#ifndef WINCIT
        if (debug)
            {
            doccr();
            cPrintf(getdbmsg(87), source, dest);
            cPrintf(getdbmsg(14));
            doccr();
            }
#endif

        fclose(sfl);
        return (FALSE);
        }

#ifndef WINCIT
    if (debug)
        {
        doccr();
        cPrintf(getdbmsg(87), source, dest);
        }
#endif

    char *Buffer;
    size_t BufferLength;
    char StackBuffer[256];
    Bool Allocated = FALSE;

    for (BufferLength = MAXBUF; !Allocated && BufferLength > 256;
            BufferLength -= 1024)
        {
        Buffer = new char[BufferLength];

        if (Buffer)
            {
            Allocated = TRUE;
            }
        }

    if (!Allocated)
        {
        Buffer = StackBuffer;
        BufferLength = 256;
        }

    size_t bytes;
    do
        {
        if ((bytes = fread(Buffer, 1, BufferLength, sfl)) != 0)
            {
            if (fwrite(Buffer, 1, bytes, dfl) != bytes)
                {
                fclose(sfl);
                fclose(dfl);

#ifndef WINCIT
                if (debug)
                    {
                    cPrintf(getdbmsg(14));
                    doccr();
                    }
#endif

                unlink(dest);

                if (Allocated)
                    {
                    delete [] Buffer;
                    }

                return (FALSE);
                }
            }
        } while (bytes == BufferLength);

    fclose(sfl);
    fclose(dfl);

    if (Allocated)
        {
        delete [] Buffer;
        }

#ifndef WINCIT
    if (debug)
        {
        cPrintf(getdbmsg(13));
        doccr();
        }
#endif

    return (TRUE);
    }
