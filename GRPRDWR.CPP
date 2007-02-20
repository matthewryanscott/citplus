// --------------------------------------------------------------------------
// Citadel: GrpRdWr.CPP
//
// Group Read/Write stuff.

#include "ctdl.h"
#pragma hdrstop

#include "group.h"

// --------------------------------------------------------------------------
// Contents


Bool GroupBuffer::Save(const char *Name) const
	{
	assert(this);
	Bool Good = TRUE;
	VerifyHeap();

	if (IsValid())
		{
		FILE *File;
		char FileName[256];

		sprintf(FileName, sbs, cfg.homepath, Name);

		if ((File = fopen(FileName, FO_WB)) != NULL)
			{
			long l = Groups[0].SizeOfDiskRecord();
			fwrite(&l, sizeof(long), 1, File);

			for (g_slot Index = 0; Index < NumGroups; Index++)
				{
				if (!Groups[Index].Save(File))
					{
#ifndef WINCIT
					mPrintf(getmsg(661), FileName);
#endif
					Good = FALSE;
					break;
					}
				}

			fclose(File);
			}
		else
			{
#ifndef WINCIT
			mPrintf(getmsg(78), FileName);
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool GroupBuffer::Load(const char *Name)
	{
	assert(this);
	Bool Good = TRUE;
	VerifyHeap();

	if (IsValid())
		{
		FILE *File;
		char FileName[256];

		sprintf(FileName, sbs, cfg.homepath, Name);

		if ((File = fopen(FileName, FO_RB)) != NULL)
			{
			fseek(File, sizeof(long), SEEK_SET);

			for (g_slot Index = 0; Index < NumGroups; Index++)
				{
				if (!Groups[Index].Load(File))
					{
#ifndef WINCIT
					mPrintf(getmsg(83), FileName);
#endif
					Good = FALSE;
					break;
					}

				Groups[Index].Verify();
				}

			fclose(File);
			}
		else
			{
#ifndef WINCIT
			mPrintf(getmsg(78), FileName);
#endif
			Good = FALSE;
			}
		}
	else
		{
		Good = FALSE;
		}

	VerifyHeap();
	return (Good);
	}

Bool GroupEntry::Save(FILE *File)
	{
	assert(this);
	assert(File != NULL);

	GAINEXCLUSIVEACCESS();

	const Bool Good = fwrite(((char *) this) + EXCLUSIVEACCESSRECORDSIZE(),
			SizeOfDiskRecord(), 1, File) == 1;

	RELEASEEXCLUSIVEACCESS();

	return (Good);
	}

Bool GroupEntry::Load(FILE *File)
	{
	assert(this);
	assert(File != NULL);

	GAINEXCLUSIVEACCESS();

	const Bool Good = fread(((char *) this) + EXCLUSIVEACCESSRECORDSIZE(),
			SizeOfDiskRecord(), 1, File) == 1;

	RELEASEEXCLUSIVEACCESS();

	return (Good);
	}
