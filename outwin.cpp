// --------------------------------------------------------------------------
// Citadel: OutWin.CPP
//
// Screen output (and, actually, keyboard input...) stuff for Windows


#ifdef WINCIT
#include "ctdl.h"
#include "wincit.h"

#include "termwndw.h"


Bool TERMWINDOWMEMBER WinStatCon(void)
	{
	if (TermWindowCollection.InFocus(this))
		{
		DWORD Number;

		if (GetNumberOfConsoleInputEvents(hConsoleInput, &Number))
			{
			return (!!Number);
			}
		}

	return (FALSE);
	}

int TERMWINDOWMEMBER WinGetCon(void)
	{
	if (TermWindowCollection.InFocus(this))
		{
		INPUT_RECORD ir;
		DWORD Number;

		if (ReadConsoleInput(hConsoleInput, &ir, 1, &Number))
			{
			if (Number && ir.EventType == KEY_EVENT &&
					ir.Event.KeyEvent.bKeyDown)
				{
				// Just to make things simpler to type...
				const DWORD cks = ir.Event.KeyEvent.dwControlKeyState;

				int OurFlags;

				if (((cks & LEFT_ALT_PRESSED) == LEFT_ALT_PRESSED) ||
						((cks & RIGHT_ALT_PRESSED) == RIGHT_ALT_PRESSED))
					{
					OurFlags = WC_ALT;
					}
				else if (((cks & LEFT_CTRL_PRESSED) == LEFT_CTRL_PRESSED) ||
						((cks & RIGHT_CTRL_PRESSED) == RIGHT_CTRL_PRESSED))
					{
					if (ir.Event.KeyEvent.uChar.AsciiChar)
						{
						return (ir.Event.KeyEvent.uChar.AsciiChar);
						}

					OurFlags = WC_CTRL;
					}
				else if (!ir.Event.KeyEvent.uChar.AsciiChar &&
						((cks & SHIFT_PRESSED) == SHIFT_PRESSED))
					{
					OurFlags = WC_SHIFT;
					}
				else if (ir.Event.KeyEvent.uChar.AsciiChar)
					{
					return (ir.Event.KeyEvent.uChar.AsciiChar);
					}
				else
					{
					OurFlags = 0;
					}

				return ((OurFlags | ir.Event.KeyEvent.wVirtualKeyCode) << 8);
				}
			}
		}

	return (0);
	}


Bool TERMWINDOWMEMBER WinPutChar(char Ch, int Attr, Bool)
	{
	const int Offset = logiRow * conCols + logiCol;

	ScreenBuffer[Offset].Char.AsciiChar = Ch;
	ScreenBuffer[Offset].Attributes = (WORD) Attr;

	if (TermWindowCollection.InFocus(this))
		{
		WaitForSingleObject(ScreenUpdateMutex, INFINITE);
		if (ScreenUpdateRect.Left > logiCol)
			{
			ScreenUpdateRect.Left = (SHORT) logiCol;
			}
		if (ScreenUpdateRect.Right < logiCol)
			{
			ScreenUpdateRect.Right = (SHORT) logiCol;
			}
		if (ScreenUpdateRect.Top > logiRow)
			{
			ScreenUpdateRect.Top = (SHORT) logiRow;
			}
		if (ScreenUpdateRect.Bottom < logiRow)
			{
			ScreenUpdateRect.Bottom = (SHORT) logiRow;
			}
		ReleaseMutex(ScreenUpdateMutex);
		}

	return (logiCol + 1 >= conCols);
	}


void TERMWINDOWMEMBER WinPutString(int Row, const char *Str, int Attr, Bool)
	{
	for (int i = 0; Str[i]; i++)
		{
		position(Row, i);

		WinPutChar(Str[i], Attr, FALSE);
		}
	}

void TERMWINDOWMEMBER clearline(int Row, int Attr)
	{
	uint Cols, Rows;

	getScreenSize(&Cols, &Rows);

	position(Row, 0);

	for (int P = 0; P < Cols; P++)
		{
		position(Row, P);
		WinPutChar(' ', Attr, FALSE);
		}
	}


void TERMWINDOWMEMBER curson(void)
	{
	CursorVisible = TRUE;
	}

void TERMWINDOWMEMBER cursoff(void)
	{
	CursorVisible = FALSE;
	}

// petzold, pp 621-622
void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart)
	{
	BITMAP bm;
	HDC hdcMem;
	POINT ptSize, ptOrg;

	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	SetMapMode(hdcMem, GetMapMode(hdc));

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR) &bm);

	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdc, &ptSize, 1);

	ptOrg.x = 0;
	ptOrg.y = 0;
	DPtoLP(hdcMem, &ptOrg, 1);

	BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y,
			SRCCOPY);

	DeleteDC(hdcMem);
	}


// --------------------------------------------------------------------------
// Plays a given WAVE file by using MCI_OPEN, MCI_PLAY.
// Returns when playback begins. Returns 0L on success;
// otherwise returns an MCI error code.
//
// originally from Win32 API help file; modified for Citadel

DWORD playWAVEFile(LPCSTR lpszWAVEFileName)
	{
	DWORD dwReturn;
	MCI_OPEN_PARMS mciOpenParms;
	MCI_PLAY_PARMS mciPlayParms;
	
	// Open the device by specifying the device name and device element.
	// MCI will choose a device capable of playing the given file.
 
	mciOpenParms.lpstrDeviceType = "waveaudio";
	mciOpenParms.lpstrElementName = lpszWAVEFileName;
	
	if ((dwReturn = mciSendCommand(0, MCI_OPEN,
			MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
			(DWORD)(LPVOID) &mciOpenParms)) != 0)
		{
		// Failed to open device; don't close it, just return error.

		return (dwReturn);
		}
		
	// Device opened successfully. Get the device ID.
	UINT wDeviceID = mciOpenParms.wDeviceID;

	// Begin playback. The window-procedure function for the parent window is 
	// notified with an MM_MCINOTIFY message when playback is complete. The 
	// window procedure then closes the device.

	mciPlayParms.dwCallback = (DWORD) MainDlgH;

	if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY,
			(DWORD)(LPVOID) &mciPlayParms)) != 0)
		{
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
		
		return (dwReturn);
		}

	return (0);
	}


// --------------------------------------------------------------------------
// Plays a given compact disc track using MCI_OPEN, MCI_PLAY. Returns as soon
// as playback begins. The window procedure function for the given window
// will be notified when playback is complete. Returns 0L on success;
// otherwise, it returns an MCI error code.
//
// originally from Win32 API help file; modified for Citadel

DWORD playCDTrack(BYTE bTrack)
	{
	UINT wDeviceID;
	DWORD dwReturn;
	MCI_OPEN_PARMS mciOpenParms;
	MCI_SET_PARMS mciSetParms;
	MCI_PLAY_PARMS mciPlayParms;
	
	// Open the compact disc device by specifying the device name.
	mciOpenParms.lpstrDeviceType = "cdaudio";
	
	if ((dwReturn = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE,
			(DWORD)(LPVOID) &mciOpenParms)) != 0)
		{
		// Failed to open device; don't close it, just return error.

		return (dwReturn);
		}
		
	
	// Device opened successfully, get the device ID.
	wDeviceID = mciOpenParms.wDeviceID;
																
 	// Set the time format to track/minute/second/frame.
	mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
	if ((dwReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
			(DWORD)(LPVOID) &mciSetParms)) != 0)
		{
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
		return (dwReturn);
		}

	// Begin playback from the given track and play until the beginning of
	// the next track. The window procedure function for the parent window
	// will be notified with an MM_MCINOTIFY message when playback is
	// complete. Unless the play command fails, the window procedure closes
	// the device.

	mciPlayParms.dwFrom = MCI_MAKE_TMSF(bTrack, 0, 0, 0);
	mciPlayParms.dwTo = MCI_MAKE_TMSF(bTrack + 1, 0, 0, 0);
	mciPlayParms.dwCallback = (DWORD) MainDlgH;

	if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY,
			MCI_FROM | MCI_TO | MCI_NOTIFY,
			(DWORD)(LPVOID) &mciPlayParms)) != 0)
		{
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
		return (dwReturn);
		}

	return (0);
	}


// --------------------------------------------------------------------------
// Plays a given MIDI file using MCI_OPEN, MCI_PLAY. Returns as soon as
// playback begins. The window procedure function for the given window is 
// notified when playback is complete. Returns 0L on success; otherwise
// returns an MCI error code.
//
// originally from Win32 API help file; modified for Citadel

DWORD playMIDIFile(LPCSTR lpszMIDIFileName)
	{
	UINT wDeviceID;
	DWORD dwReturn;
	MCI_OPEN_PARMS mciOpenParms;
	MCI_PLAY_PARMS mciPlayParms;
	MCI_STATUS_PARMS mciStatusParms;

	// Open the device by specifying the device name and device element.
	// MCI will attempt to choose the MIDI Mapper as the output port.
	mciOpenParms.lpstrDeviceType = "sequencer";
	mciOpenParms.lpstrElementName = lpszMIDIFileName;

	if ((dwReturn = mciSendCommand(NULL, MCI_OPEN,
			MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
			(DWORD)(LPVOID) &mciOpenParms)) != 0)
		{
		// Failed to open device; don't close it, just return error.
		return (dwReturn);
		}

	// Device opened successfully. Get the device ID.
	wDeviceID = mciOpenParms.wDeviceID;

	// See if the output port is the MIDI Mapper.
	mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;

	if ((dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
			(DWORD)(LPVOID) &mciStatusParms)) != 0)
		{
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
		return (dwReturn);
		}

	// The output port is not the MIDI Mapper. Ask if user wants to continue.
	if (LOWORD(mciStatusParms.dwReturn) != (WORD) MIDI_MAPPER)
		{
//		if (MessageBox(hMainWnd,
//				"The MIDI Mapper is not available. Continue?",
//				"", MB_YESNO) == IDNO)
			{
			// User does not want to continue. Not an error; just close the 
			// device and return.

			mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
			return (0);
			}
		}

	// Begin playback. The window procedure function for the parent window is
	// notified with an MM_MCINOTIFY message when playback is complete. The
	// window procedure then closes the device.

	mciPlayParms.dwCallback = (DWORD) MainDlgH;

	if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY,
			(DWORD)(LPVOID) &mciPlayParms)) != 0)
		{
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
		return (dwReturn);
		}

	return (0);
	}

#endif
