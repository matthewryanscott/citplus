// --------------------------------------------------------------------------
// Citadel: InfoFile.CPP
//
// The info file subsystem.
//
// Citadel keeps track of file information for all files in directory rooms.
// This subsystem handles access to this file information.

#include "ctdl.h"
#pragma hdrstop

#include "infofile.h"
#include "extmsg.h"
#include "log.h"


// --------------------------------------------------------------------------
// Contents
//
// ------------ Public
// Functions:
//     DisposeFileInfo()
//         Pass it a CITHANDLE to loaded file information for it to
//         remove from memory.
//
//     LoadFileInfoForDirectory()
//         Loads file information for a directory.
//
//     GetInfoByName()
//         Returns the file information for a file specified by name.
//
//     GetInfoByNumber()
//         Returns the file information for a file specified by number.
//
//     RemoveFileInfo()
//         Removes the information for a file.
//
//     ReIndexFileInfo()
//         Removes all non-existant entries from the info file,
//         reformats it for quicker lookup of file information.
//
//     FillFileInfo()
//         Adds all files in a directory that have no info file
//         entries to the info file.
//
//     UpdateInfoFile()
//         Updates the file info for a file, adding it to the database
//         if it doesn't exist.
//
//     GetSingleFileInfo()
//         Get file information for a single file.
//
//  Data types:
//     fInfo            File info structure.
//
// ------------ Private
//  Functions:
//     addFileInfoList()
//         Adds file info slots to an info file list.
//
//     seekFileInfo()
//         Seeks a specified file's information in the file info
//         database.
//
//     setFileInfoForAppend()
//         Sets FILEINFO.DAT ready to append file information to it.
//
//  Data types:
//     fInfoList        Used for loading fInfo structures.
//     InfoFileLoad     Used for loading everything.
//
//  Data:
//     fileinfoDat  "FILEINFO.DAT"
//     newfinfoTmp  "NEWFINFO.TMP"


// --------------------------------------------------------------------------
// Citadel keeps information about files in directory rooms in a file named
// FILEINFO.DAT. This file is formatted as follows
//
// Full Path
//  This is the full file path for the files that follow. This is 64 bytes
//  long, including the trailing Nul character.
//
// Number of files
//  This is the number of files that are described for this path. This is
//  an integer, so it is 2 bytes long for the MS-DOS implementation.
//
// File info
//  This is the structure that follows. The structure is repeated, once
//  for each file described.
//
// This format is repeated for however long it needs to be repeated. A "full
// path" can have many entries in the file. This makes it so that adding a
// file is always a matter of just appending data: there is never a need to
// make space in the middle to add a file to a file path's info. Just write
// the file's information to the end of the file. Citadel reorganizes this
// file every time it is loaded, so that all files for a path are kept
// together in the file. This makes loading and using the file faster. The
// sysop can also do this with the .Sysop Infofile reset (.SI) command; this
// is useful for people who do not take their board down often and have a lot
// of file system use. (Uploading, moving, and deleting files all can
// fragment the database.)


// --------------------------------------------------------------------------
// When Citadel loads information from FILEINFO.DAT, it uses this structure.
// It is told to get all the file information for a desired path. It does
// this by allocating one of these structures (which form a linked list in
// memory) for each section of FILEINFO.DAT that contains information for
// that path.
//
// Note that this structure defines an array of 1 for the actual file
// information. When we allocate memory for this, we use the following
// formula to determine how much to allocate:
//
//  sizeof(fInfoList) + (num - 1) * sizeof(fInfo)
//
// Where "num" is the number of files being described. This allocates exactly
// the amount of memory we want. We can then index each entry of the group
// using standard C array indexing: fil->fi[3] would give us the fourth
// entry, for example (assuming that fil is a pointer to this strucutre, of
// course).

typedef struct fil
    {
    struct fil *next;
    short num;                  // The number of files in this section.
    fInfo fi[1];                // The file information.
    } fInfoList;


// --------------------------------------------------------------------------
// When another module asks to to load all of the information for a
// directory, we return it a CITHANDLE that points to this structure.

typedef struct
    {
    fInfoList   *List;          // The list of loaded info buffers
    ushort      FilesLoaded;    // The total number loaded.
    } InfoFileLoad;


static const char *fileinfoDat = "FILEINFO.DAT";
static const char *newfinfoTmp = "NEWFINFO.TMP";

static fInfoList *addFileInfoList(InfoFileLoad *ifl, ushort toAdd);
static Bool seekFileInfo(FILE *fl, const char *Directory, const char *FileName, short EntryNum);
static void setFileInfoForAppend(FILE *fl, const char *Directory, ushort toAdd);

#ifdef WINCIT
    static void ReadFIRecord(fInfo *a, FILE *fd)
        {
        fread(&a->downloads, sizeof(a->downloads), 1, fd);

        fread(&a->uploadtime, sizeof(a->uploadtime), 1, fd);

        fread(&a->fn, sizeof(a->fn) + sizeof(a->uploader) + sizeof(a->comment) + sizeof(a->extcomments), 1, fd);

        short s;
        fread(&s, sizeof(s), 1, fd);

        a->deleted = s & 1;
        a->UNUSED = (s >> 1) & 127;
        }

    static void WriteFIRecord(fInfo *a, FILE *fd)
        {
        fwrite(&a->downloads, sizeof(a->downloads), 1, fd);

        fwrite(&a->uploadtime, sizeof(a->uploadtime), 1, fd);

        fwrite(&a->fn, sizeof(a->fn) + sizeof(a->uploader) + sizeof(a->comment) + sizeof(a->extcomments), 1, fd);

        short s = (short) (a->deleted | (a->UNUSED << 1));
        fwrite(&s, sizeof(s), 1, fd);
        }
#else
    #define ReadFIRecord(a,f)   fread(a, sizeof(fInfo), 1, f)
    #define WriteFIRecord(a,f)  fwrite(a, sizeof(fInfo), 1, f)
#endif


// --------------------------------------------------------------------------
// DisposeFileInfo(): Passed a CITHANDLE to file information, it disposes of
//  associated file information.
//
// Input:
//  LoadedInfo  A CITHANDLE to loaded information to dispose.

void DisposeFileInfo(CITHANDLE LoadedInfo)
    {
    assert(LoadedInfo != NULL);

    disposeLL((void **) &(((InfoFileLoad *) (LoadedInfo))->List));
    delete LoadedInfo;
    }


// --------------------------------------------------------------------------
// LoadFileInfoForDirectory(): Loads file information for a directory.
//
// Input:
//  Directory   The directory to load info for.
//
// Output:
//  LoadInfo    A CITHANDLE for accessing the data later.
//
// Return value:
//  TRUE if it worked, else FALSE.

Bool LoadFileInfoForDirectory(const char *Directory, CITHANDLE *LoadInfo)
    {
    assert(Directory != NULL);
    assert(*Directory);

    *LoadInfo = new InfoFileLoad;

    if (*LoadInfo)
        {
        ((InfoFileLoad *) *LoadInfo)->List = NULL;
        ((InfoFileLoad *) *LoadInfo)->FilesLoaded = 0;

        FILE *fd;

        char pathToFileinfoDat[128];
        sprintf(pathToFileinfoDat, sbs, cfg.homepath, fileinfoDat);

        if ((fd = fopen(pathToFileinfoDat, FO_RB)) != NULL)
            {
            while (!feof(fd))
                {
                char path[64];

                fread(path, 64, 1, fd);

                if (!feof(fd))
                    {
                    short i;
                    fInfoList *fi;

                    fread(&i, sizeof(i), 1, fd);

                    if (SameString(path, Directory))
                        {
                        short maxAdd;

                        maxAdd = (65532u - sizeof(fInfoList) + sizeof(fInfo)) / sizeof(fInfo);

                        while (i > 0)
                            {
                            short toAdd = min(i, maxAdd);

                            if ((fi = addFileInfoList((InfoFileLoad *) *LoadInfo, toAdd)) != NULL)
                                {
                                for (int ReadRecord = 0; ReadRecord < toAdd; ReadRecord++)
                                    {
                                    ReadFIRecord(&(fi->fi[ReadRecord]), fd);
                                    }
                                }
                            else
                                {
                                OutOfMemory(27);

                                fseek(fd, (long) fInfoDiskSize * (long) toAdd, SEEK_CUR);
                                }

                            i -= toAdd;
                            }
                        }
                    else
                        {
                        fseek(fd, (long) fInfoDiskSize * (long) i, SEEK_CUR);
                        }
                    }
                }

            fclose(fd);
            }

        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }


// --------------------------------------------------------------------------
// GetInfoByName(): Returns the file information for a file specified by
//  name.
//
// Input:
//  LoadedInfo  CITHANDLE to loaded file information.
//  FileName    Name of file to look for.
//
// Output:
//  InfoBuffer  Description of file.
//
// Return value:
//  TRUE if info found, else FALSE.

Bool GetInfoByName(CITHANDLE LoadedInfo, const char *FileName, fInfo **InfoBuffer)
    {
    fInfoList *curl;
    short i;

    assert(LoadedInfo != NULL);
    assert(FileName != NULL);
    assert(*FileName);

    for (curl = ((InfoFileLoad *) LoadedInfo)->List; curl; curl = (fInfoList *) getNextLL(curl))
        {
        for (i = curl->num; i; i--)
            {
            if (!(curl->fi[i - 1].deleted) && SameString(curl->fi[i - 1].fn, FileName))
                {
                *InfoBuffer = &(curl->fi[i - 1]);
                return (TRUE);
                }
            }
        }

    *InfoBuffer = NULL;
    return (FALSE);
    }


// --------------------------------------------------------------------------
// GetInfoByNumber(): Returns the file information for a file specified by
//  number.
//
// Input:
//  LoadedInfo  CITHANDLE to loaded file information.
//  Number      Number of file to look for, starting at 1.
//
// Output:
//  InfoBuffer  Description of file.
//
// Return value:
//  TRUE if info found, else FALSE.

static Bool GetInfoByNumber(CITHANDLE LoadedInfo, short Number, fInfo **InfoBuffer)
    {
    fInfoList *curl;
    short i;

    assert(LoadedInfo != NULL);
    assert(Number > 0);

    for (curl = ((InfoFileLoad *) LoadedInfo)->List; curl; curl = (fInfoList *) getNextLL(curl))
        {
        for (i = 0; i < curl->num; i++)
            {
            if (--Number == 0)
                {
                *InfoBuffer = &(curl->fi[i]);
                return (TRUE);
                }
            }
        }

    *InfoBuffer = NULL;
    return (FALSE);
    }


// --------------------------------------------------------------------------
// RemoveFileInfo(): Removes the information for a file.
//
// Input:
//  FilePath    Directory for file to remove.
//  FileName    Name of file to remove.

void RemoveFileInfo(const char *FilePath, const char *FileName)
    {
    FILE *fd;

    assert(FilePath != NULL);
    assert(*FilePath);
    assert(FileName != NULL);
    assert(*FileName);

    char pathToFileinfoDat[128];
    sprintf(pathToFileinfoDat, sbs, cfg.homepath, fileinfoDat);

    if ((fd = fopen(pathToFileinfoDat, FO_RPB)) != NULL)
        {
        while (!feof(fd))
            {
            char path[64];

            fread(path, 64, 1, fd);

            if (!feof(fd))
                {
                short i;

                fread(&i, sizeof(i), 1, fd);

                if (SameString(path, FilePath))
                    {
                    for (; i; i--)
                        {
                        fInfo info;

                        ReadFIRecord(&info, fd);

                        if (!info.deleted && SameString(FileName, info.fn))
                            {
                            info.deleted = TRUE;
                            fseek(fd, -1 * fInfoDiskSize, SEEK_CUR);
                            WriteFIRecord(&info, fd);
                            fclose(fd);
                            return;
                            }
                        }
                    }
                else
                    {
                    fseek(fd, fInfoDiskSize * (long) i, SEEK_CUR);
                    }
                }
            }
        fclose(fd);
        }
    }


// --------------------------------------------------------------------------
// ReIndexFileInfo(): Removes all non-existant entries from the info file,
//  reformats it for quicker lookup of file information.

void ReIndexFileInfo(void)
    {
    FILE *fd;
    strList *dlBase = NULL, *cur;

    VerifyHeap();

    char pathToFileinfoDat[128];
    sprintf(pathToFileinfoDat, sbs, cfg.homepath, fileinfoDat);

    if ((fd = fopen(pathToFileinfoDat, FO_RPB)) != NULL)
        {
        short i;
        char Directory[64];
        fInfo info, *slot;
        FILE *fd2;

        // first, mark any deleted as deleted and clean up entries
        while (!feof(fd))
            {
            fread(Directory, 64, 1, fd);

            if (!feof(fd))
                {
                for (fread(&i, sizeof(i), 1, fd); i; i--)
                    {
                    ReadFIRecord(&info, fd);

                    info.fn[12] = 0;
                    info.uploader[LABELSIZE] = 0;
                    info.comment[64] = 0;
                    info.extcomments[0][64] = 0;
                    info.extcomments[1][64] = 0;
                    info.extcomments[2][64] = 0;
                    info.UNUSED = 0;

                    if (!info.deleted)
                        {
                        char fpath[128];

                        sprintf(fpath, sbs, Directory, info.fn);
                        if (!filexists(fpath))
                            {
                            info.deleted = TRUE;
                            }
                        }

                    fseek(fd, -1 * fInfoDiskSize, SEEK_CUR);
                    WriteFIRecord(&info, fd);

                    // Reprepare the file for reading.
                    fseek(fd, 0, SEEK_CUR);
                    }
                }
            }

        fclose(fd);

        // then copy to new file, purging deleted and combining records
        if ((fd = fopen(pathToFileinfoDat, FO_RB)) != NULL)
            {
            char pathToNewfinfoTmp[128];
            sprintf(pathToNewfinfoTmp, sbs, cfg.homepath, newfinfoTmp);

            if ((fd2 = fopen(pathToNewfinfoTmp, FO_WB)) != NULL)
                {
                // copy fd to fd2
                while (!feof(fd))
                    {
                    fread(Directory, 64, 1, fd);

                    if (!feof(fd))
                        {
                        fread(&i, sizeof(i), 1, fd);
                        fseek(fd, fInfoDiskSize * (long) i, SEEK_CUR);

                        if (*Directory) // valid dir name
                            {
                            for (cur = dlBase; cur; cur = (strList *) getNextLL(cur))
                                {
                                if (SameString(Directory, cur->string))
                                    {
                                    break;
                                    }
                                }

                            if (!cur)
                                {
                                Bool writeIt;
                                short k;

                                cur = (strList *) addLL((void **) &dlBase, sizeof(*dlBase) + strlen(Directory));

                                if (cur)
                                    {
                                    CITHANDLE FileInfo;

                                    strcpy(cur->string, Directory);

                                    if (LoadFileInfoForDirectory(Directory, &FileInfo))
                                        {
                                        short j = ((InfoFileLoad *) FileInfo)->FilesLoaded;

                                        for (writeIt = FALSE, k = 0; k < j && !writeIt; k++)
                                            {
                                            if (GetInfoByNumber(FileInfo, (short) (k + 1), &slot) && !slot->deleted)
                                                {
                                                writeIt = TRUE;
                                                }
                                            }

                                        // save out to fd2
                                        if (writeIt)
                                            {
                                            long pos;

                                            fwrite(Directory, sizeof(Directory), 1, fd2);

                                            pos = ftell(fd2);
                                            i = j;
                                            fwrite(&i, sizeof(i), 1, fd2);

                                            for (k = 0; k < j; k++)
                                                {
                                                if (GetInfoByNumber(FileInfo, (short) (k + 1), &slot) && !slot->deleted)
                                                    {
                                                    WriteFIRecord(slot, fd2);
                                                    }
                                                else
                                                    {
                                                    i--;
                                                    }
                                                }

                                            fseek(fd2, pos, SEEK_SET);
                                            fwrite(&i, sizeof(i), 1, fd2);
                                            fseek(fd2, 0, SEEK_END);
                                            }

                                        DisposeFileInfo(FileInfo);
                                        }
                                    }
                                }
                            }
                        }
                    }

                fclose(fd2);
                fclose(fd);

                // delete / rename
                unlink(pathToFileinfoDat);
                rename(pathToNewfinfoTmp, pathToFileinfoDat);
                }
            else
                {
                fclose(fd);
                }
            }
        }

    disposeLL((void **) &dlBase);

    VerifyHeap();
    }


// --------------------------------------------------------------------------
// FillFileInfo(): Adds all files in a directory that have no info file
//  entries to the info file.
//
// Input:
//  Directory   The directory to search.
//  Uploader    The name to attach to the file as the uploader of the file.
//  AskUser     TRUE to ask the user for file comments. If FALSE, an empty
//              comment is given.
//  HideThem    TRUE to hide added files, else FALSE.
//
// Output:
//  Report      A text buffer to write a report of what files are added. NULL
//              for no report.
//
// Return value:
//  The number of files added to the info file.

short TERMWINDOWMEMBER FillFileInfo(const char *Directory, const char *Uploader, Bool askuser, char *Report, Bool HideThem)
    {
    assert(Directory != NULL);
    assert(*Directory);
    assert(Uploader != NULL);

    if (!changedir(Directory))
        {
        return (0);
        }

    short total = 0;

    if (LockMessages(MSG_INFOFILE))
        {
        if (Report)
            {
            label Buffer;
            sprintf(Report, getmsg(MSG_INFOFILE, 16), CurrentUser->GetName(Buffer, sizeof(Buffer)), bn);
            }

        directoryinfo *files = filldirectory(starstar, SORT_ALPHA, OldAndNew, FALSE);

        if (files)
            {
            CITHANDLE FileInfo;

            if (LoadFileInfoForDirectory(Directory, &FileInfo))
                {
                short i;

                for (i = 0; *files[i].Name; i++)
                    {
                    fInfo *slot;
                    const char *FileName = files[i].Name;
#ifdef WINCIT
                    const char *FullFileName = files[i].FullName;
#endif
                    if (!GetInfoByName(FileInfo, FileName, &slot))
                        {
                        if (HideThem)
                            {
                            char FullName[128];

                            sprintf(FullName, sbs, Directory, FileName);
                            hide(FullName);
                            }

                        if (askuser)
                            {
                            char comment[64];
                            label FileDate;

                            getdstamp(FileDate, files[i].DateTime);
#ifdef WINCIT
                            CRmPrintfCR(getmsg(MSG_INFOFILE, 17), FullFileName, files[i].Length, FileDate);
#else
                            CRmPrintfCR(getmsg(MSG_INFOFILE, 17), FileName, files[i].Length, FileDate);
#endif
                            getNormStr(getmsg(383), comment, 63);

                            UpdateInfoFile(Directory, FileName, Uploader, comment, 0, 0);

                            total++;

                            if (Report)
                                {
                                char tmp[200+MAX_PATH];

                                if (!*comment)
                                    {
#ifdef WINCIT
                                    sprintf(tmp, getmsg(429), FullFileName, bn);
#else
                                    sprintf(tmp, getmsg(429), FileName, bn);
#endif
                                    }
                                else
                                    {
#ifdef WINCIT
                                    sprintf(tmp, getmsg(MSG_INFOFILE, 18), FullFileName, comment, bn);
#else
                                    sprintf(tmp, getmsg(MSG_INFOFILE, 18), FileName, comment, bn);
#endif
                                    }

                                strcat(Report, tmp);
                                }

#ifdef WINCIT
                            trap(T_UPLOAD, WindowCaption, getmsg(MSG_INFOFILE, 19), FileName, Directory);
#else
                            trap(T_UPLOAD, getmsg(MSG_INFOFILE, 19), FileName, Directory);
#endif
                            }
                        else
                            {
                            UpdateInfoFile(Directory, FileName, Uploader, ns, 0, 0);
                            }
                        }
                    }

                DisposeFileInfo(FileInfo);
                }

            delete [] files;
            }

        UnlockMessages(MSG_INFOFILE);
        }
    else
        {
        OutOfMemory(110);
        }

    changedir(cfg.homepath);
    return (total);
    }

// --------------------------------------------------------------------------
// UpdateInfoFile(): Updates the file info for a file, adding it to the
//  database if it doesn't exist.
//
// Input:
//  Directory   The directory of the file to update.
//  FileName    The name of the file to update. (Give short name!)
//  Uploader    The name of the uploader of the file.
//  Comment     The comment to assign to the file.
//  Downloads   The number of downloads for the file.
//  UploadTime  The time of the file's upload. Use 0 for the current time.
//
// Note:
//  If the file is already in the database, Downloads and UploadTime are left
//  the same.

void UpdateInfoFile(const char *Directory, const char *FileName, const char *Uploader, const char *Comment,
        short Downloads, time_t UploadTime)
    {
    fInfo ours;
    FILE *fl;
    short i;

    assert(Directory != NULL);
    assert(*Directory);
    assert(FileName != NULL);
    assert(*FileName);
    assert(Uploader != NULL);
    assert(Comment != NULL);

    memset(&ours, 0, sizeof(ours));

    char pathToFileinfoDat[128];
    sprintf(pathToFileinfoDat, sbs, cfg.homepath, fileinfoDat);

    if ((fl = fopen(pathToFileinfoDat, FO_RPB)) == NULL)
        {
        if ((fl = fopen(pathToFileinfoDat, FO_WPB)) != NULL)
            {
            CopyStringToBuffer(ours.fn, FileName);
            CopyStringToBuffer(ours.uploader, Uploader);
            CopyStringToBuffer(ours.comment, Comment);
            ours.deleted = FALSE;
            ours.downloads = Downloads;

            if (UploadTime)
                {
                ours.uploadtime = UploadTime;
                }
            else
                {
                time(&ours.uploadtime);
                }

            fwrite(Directory, 64, 1, fl);
            i = 1;
            fwrite(&i, sizeof(i), 1, fl);
            WriteFIRecord(&ours, fl);
            fclose(fl);
            }
        }
    else
        {
        Bool found = FALSE;

        for (i = 0; seekFileInfo(fl, Directory, FileName, (short) (i + 1)); i++)
            {
            ReadFIRecord(&ours, fl);

            if (!found)
                {
                CopyStringToBuffer(ours.uploader, Uploader);
                CopyStringToBuffer(ours.comment, Comment);
                ours.deleted = FALSE;
                found = TRUE;
                }
            else
                {
                // this is a duplicate entry. kill it.
                ours.deleted = TRUE;
                }

            fseek(fl, -1 * fInfoDiskSize, SEEK_CUR);
            WriteFIRecord(&ours, fl);
            }

        if (!found)
            {
            CopyStringToBuffer(ours.fn, FileName);
            CopyStringToBuffer(ours.uploader, Uploader);
            CopyStringToBuffer(ours.comment, Comment);
            ours.deleted = FALSE;
            ours.downloads = Downloads;

            if (UploadTime)
                {
                ours.uploadtime = UploadTime;
                }
            else
                {
                time(&ours.uploadtime);
                }

            setFileInfoForAppend(fl, Directory, 1);
            WriteFIRecord(&ours, fl);
            }

        fclose(fl);
        }
    }


// --------------------------------------------------------------------------
// GetSingleFileInfo(): Get file information for a single file.
//
// Input:
//  Directory   Directory that contains the file.
//  FileName    Name of the file to find.
//
// Output:
//  Buffer      Where to write the information to.
//
// Return value:
//  TRUE if info found, else FALSE.

Bool GetSingleFileInfo(const char *Directory, const char *FileName, fInfo *Buffer)
    {
    FILE *fl;
    Bool found = FALSE;

    assert(Directory != NULL);
    assert(*Directory);
    assert(FileName != NULL);
    assert(*FileName);
    assert(Buffer != NULL);

    char pathToFileinfoDat[128];
    sprintf(pathToFileinfoDat, sbs, cfg.homepath, fileinfoDat);

    if ((fl = fopen(pathToFileinfoDat, FO_RB)) != NULL)
        {
        if (seekFileInfo(fl, Directory, FileName, 1))
            {
            ReadFIRecord(Buffer, fl);
            found = TRUE;
            }

        fclose(fl);
        }

    return (found);
    }



// --------------------------------------------------------------------------
// addFileInfoList(): Adds file info slots to an info file list.
//
// Input:
//  ifl         The list to add the slots to.
//  toAdd       The number of slots to add.
//
// Return value:
//  The newly added slots, or NULL if cannot do it.
//
// Note:
//  You can only add up to 64K worth of slots at a time, since we don't use
//  huge pointers. Verify your toAdd fits before calling this function.

static fInfoList *addFileInfoList(InfoFileLoad *ifl, ushort toAdd)
    {
    fInfoList *cur;

    assert(toAdd > 0);
    assert(ifl);
    assert(sizeof(*ifl->List) + (toAdd - 1) * sizeof(fInfo) < 65532u);

    cur = (fInfoList *) addLL((void **) &ifl->List, sizeof(*ifl->List) + (toAdd - 1) * sizeof(fInfo));

    if (cur)
        {
        cur->num = toAdd;
        ifl->FilesLoaded += toAdd;
        }

    return (cur);
    }


// --------------------------------------------------------------------------
// seekFileInfo(): Seeks a specified file's information in the file info
//  database.
//
// Input:
//  fl          The FILEINFO.DAT file, already opened.
//  Directory   The directory of the file to find.
//  FileName    The name of the file to find.
//  EntryNumber The number of the entry to find, starting with 1. This lets
//              you find multiple entries for the same file. (And remove
//              them.)
//
// Return value:
//  TRUE if the file was found. FALSE if not.
//
// Notes:
//  fl is seeked to beginning of the file info.

static Bool seekFileInfo(FILE *fl, const char *Directory, const char *FileName, short EntryNum)
    {
    char path[64];
    fInfo info;

    assert(fl != NULL);
    assert(Directory != NULL);
    assert(*Directory);
    assert(FileName != NULL);
    assert(*FileName);
    assert(EntryNum > 0);

    memset(&info, 0, sizeof(info));

    fseek(fl, 0, SEEK_SET);

    while (!feof(fl))
        {
        fread(path, sizeof(path), 1, fl);

        if (!feof(fl))
            {
            short i;

            fread(&i, sizeof(i), 1, fl);

            if (SameString(Directory, path))
                {
                for (; i; i--)
                    {
                    ReadFIRecord(&info, fl);

                    if (SameString(info.fn, FileName))
                        {
                        if (--EntryNum == 0)
                            {
                            fseek(fl, -1 * fInfoDiskSize, SEEK_CUR);
                            return (TRUE);
                            }
                        }
                    }
                }
            else
                {
                fseek(fl, fInfoDiskSize * (long) i, SEEK_CUR);
                }
            }
        }

    return (FALSE);
    }


// --------------------------------------------------------------------------
// setFileInfoForAppend(): Sets FILEINFO.DAT ready to append file
//  information to it.
//
// Input:
//  fl          The FILEINFO.DAT file, already opened.
//  Directory   The directory of the file or files to append.
//  NumAppend   The number of entries that you are going to append.
//
// Note:
//  This function does not actually do any of the appending; the caller is
//  responsible. The number of entries specified by NumAppend must be
//  appended, or the info file database will be corrupted: NumAppend is used
//  to set an internal index.

static void setFileInfoForAppend(FILE *fl, const char *Directory, ushort NumAppend)
    {
    char path[64];
    long pos;
    short i;

    fseek(fl, 0, SEEK_SET);

    while (!feof(fl))
        {
        fread(path, sizeof(path), 1, fl);

        if (!feof(fl))
            {
            pos = ftell(fl);
            fread(&i, sizeof(i), 1, fl);
            fseek(fl, fInfoDiskSize * (long) i, SEEK_CUR);
            }
        }

    if (SameString(path, Directory))
        {
        i = (short) (i + NumAppend);
        fseek(fl, pos, SEEK_SET);
        fwrite(&i, sizeof(i), 1, fl);
        fseek(fl, 0, SEEK_END);
        }
    else
        {
        fseek(fl, 0, SEEK_END);
        fwrite(Directory, 64, 1, fl);
        fwrite(&NumAppend, sizeof(NumAppend), 1, fl);
        }
    }
