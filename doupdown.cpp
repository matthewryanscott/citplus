// --------------------------------------------------------------------------
// Citadel: DoUpDown.CPP
//
// .U/.D Commands

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "infofile.h"
#include "filecmd.h"
#include "log.h"
#include "msg.h"
#include "account.h"
#include "miscovl.h"
#include "domenu.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// doFileQueue()		handles Q(ueue) command
// doDownload() 		handles D(ownload) command
// doUpload()			handles U(pload) command

void TERMWINDOWMEMBER doFileQueue(void)
	{
	if (!LockMenu(MENU_FILEQUEUE))
		{
		OutOfMemory(34);
		return;
		}

	mPrintf(getmenumsg(MENU_FILEQUEUE, 5));

	const int ich = iCharNE();

	switch (DoMenu(MENU_FILEQUEUE, ich))
		{
		case 1: // Add
			{
			doCR();

			if (LockMessages(MSG_TRANS))
				{
				if (IsFileDownloadOkay())
					{
					addFileQueue();
					}

				UnlockMessages(MSG_TRANS);
				}
			else
				{
				OutOfMemory(34);
				}

			break;
			}

		case 2: // Clear
			{
			clearFileQueue();
			break;
			}

		case 3: // Download
			{
			if (downloadFileQueue())
				{
				clearFileQueue();
				}

			break;
			}

		case 4: // List
			{
			listFileQueue();
			break;
			}

		case 5: // Remove
			{
			removeFileQueue();
			break;
			}

		case -2:
			{
			oChar('?');
			showMenu(M_FILEQUEUE);
			break;
			}

		case -1:
			{
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}

	UnlockMenu(MENU_FILEQUEUE);
	}


// --------------------------------------------------------------------------
// doDownload()

void TERMWINDOWMEMBER doDownload(Bool ex)
	{
	if (!LockMessages(MSG_TRANS))
		{
		OutOfMemory(35);
		return;
		}

	SetDoWhat(DODOWNLOAD);

	mPrintf(getmsg(MSG_TRANS, 13));

	if (fileList)
		{
		// queue to download

		if (LockMenu(MENU_FILEQUEUE))
			{
			if (downloadFileQueue())
				{
				clearFileQueue();
				}

			UnlockMenu(MENU_FILEQUEUE);
			}
		else
			{
			OutOfMemory(35);
			}
		}
	else
		{
		if (IsFileDownloadOkay())
			{
			download((char) ((ex || !CurrentUser->GetDefaultProtocol()) ?
					0 : CurrentUser->GetDefaultProtocol()));
			}
		}

	UnlockMessages(MSG_TRANS);
	}


// --------------------------------------------------------------------------
// doUpload()

void TERMWINDOWMEMBER doUpload(Bool ex)
	{
	if (!LockMessages(MSG_TRANS))
		{
		OutOfMemory(36);
		return;
		}

	SetDoWhat(DOUPLOAD);

	mPrintf(getmsg(216));

	if (IsFileUploadOkay())
		{
		upload((char) ((ex || !CurrentUser->GetDefaultProtocol()) ?
				0 : CurrentUser->GetDefaultProtocol()), FALSE);
		}

	UnlockMessages(MSG_TRANS);
	}
