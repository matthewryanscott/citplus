// --------------------------------------------------------------------------
// Citadel: SetInfo.CPP
//
// .aide Set Info command

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "infofile.h"
#include "log.h"
#include "msg.h"
#include "extmsg.h"

// --------------------------------------------------------------------------
// Contents
//
// AideSetFileInfo()	menu level .as routine sets entry to aide's name if none present or leaves original uploader


// --------------------------------------------------------------------------
// AideSetFileInfo(): Menu level .AS routine sets entry to aide's name if none present or leaves original uploader

void TERMWINDOWMEMBER AideSetFileInfo(void)
	{
	if (!CheckDirectoryRoom())
		{
		return;
		}

	label filename;
	label uploader;
	char comments[64];
	char path[80];
	fInfo old;

	SetDoWhat(AIDESETINFO);

	memset(&old, 0, sizeof(old));

	doCR();
	getNormStr(getmsg(289), filename, LABELSIZE);

	if (!*filename)
		{
		return;
		}

	// no bad file names
	if (!IsFilenameLegal(filename, FALSE))
		{
		CRmPrintfCR(getmsg(425), filename);
		return;
		}

	char Buffer[64];
	sprintf(path, sbs, CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), filename);

	if (!filexists(path))
		{
		CRmPrintfCR(getmsg(425), filename);
		return;
		}

	time_t UploadTime;
	short Downloads;

	if (!GetSingleFileInfo(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)), filename, &old))
		{
		CurrentUser->GetName(uploader, sizeof(uploader));
		UploadTime = 0;
		Downloads = 0;
		}
	else
		{
		CopyStringToBuffer(uploader, old.uploader);
		UploadTime = old.uploadtime;
		Downloads = old.downloads;
		}

	getNormStr(getmsg(383), comments, 63);

	if (*comments)
		{
#ifdef WINCIT
		// Need short name for FILEINFO.DAT.
		if (CurrentRoom->ChangeDir())
			{
			directoryinfo *files = filldirectory(filename, SORT_DATE, OldAndNew, TRUE);

			if (files && *files[0].Name)
				{
				UpdateInfoFile(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)),
					files[0].Name, uploader, comments, Downloads, UploadTime);
				}

			delete [] files;

			changedir(cfg.homepath);
			}
#else
		UpdateInfoFile(CurrentRoom->GetDirectory(Buffer, sizeof(Buffer)),
				filename, uploader, comments, Downloads, UploadTime);
#endif

		if (LockMessages(MSG_INFOFILE))
			{
			Message *Msg = new Message;

			if (Msg)
				{
				Msg->SetTextWithFormat(getmsg(MSG_INFOFILE, 20), filename, CurrentUser->GetName(Buffer, sizeof(Buffer)));

				trap(T_AIDE, pcts, Msg->GetText());

				Msg->AppendText(getmsg(MSG_INFOFILE, 21));
				Msg->AppendText(bn);
				Msg->AppendText(spc);
				Msg->AppendText(comments);

				Msg->SetRoomNumber(thisRoom);

				systemMessage(Msg);

				delete Msg;
				}
			else
				{
				OutOfMemory(74);
				}

			UnlockMessages(MSG_INFOFILE);
			}
		else
			{
			OutOfMemory(74);
			}
		}
	}
