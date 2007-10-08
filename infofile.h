// --------------------------------------------------------------------
// Citadel: INFOFILE.H
//
// How to use the info file subsystem.

typedef struct
	{
	short	downloads;			/* The number of times downloaded... NU	*/
	time_t	uploadtime;			/* The time of the upload.				*/

	char	fn[13];				/* The file name.						*/
	label	uploader;			/* Who uploaded it.						*/
	char	comment[65];		/* Citadel file comments.				*/
	char	extcomments[3][65];	/* More comments. Not Used.	Yet.		*/

	Bool	deleted : 1;		/* Has the file been deleted?			*/
	ushort	UNUSED	: 15;		/* ...									*/
	} fInfo;

#define fInfoDiskSize (2 + 4 + 13 + 31 + 65*4 + 2)

void DisposeFileInfo(CITHANDLE LoadedInfo);
Bool LoadFileInfoForDirectory(const char *Directory, CITHANDLE *LoadInfo);
Bool GetInfoByName(CITHANDLE LoadedInfo, const char *FileName, fInfo **InfoBuffer);
void RemoveFileInfo(const char *FilePath, const char *FileName);
void ReIndexFileInfo(void);
void UpdateInfoFile(const char *Directory, const char *FileName, const char *Uploader,
		const char *Comment, short Downloads, time_t UploadTime);
Bool GetSingleFileInfo(const char *Directory, const char *FileName, fInfo *Buffer);
