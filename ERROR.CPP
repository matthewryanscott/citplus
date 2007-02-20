#ifdef WINCIT
#include "ctdl.h"

#define NUMERRORS 30
#define NUMMSGS 30
#define MSGLENGTH 80

#ifdef NAHERROR
static char errors[NUMERRORS][MSGLENGTH];
static int nextError;
static HWND hwndError = NULL;
#endif

static char messages[NUMMSGS][MSGLENGTH];
static int nextMsg;
static HWND hwndMsg = NULL;

static HANDLE ErrorMutex = INVALID_HANDLE_VALUE;
static HANDLE MsgMutex = INVALID_HANDLE_VALUE;

void CreateErrorMutexes(void)
    {
    ErrorMutex = CreateMutex(NULL, FALSE, "WincitMutex:Error");
    MsgMutex = CreateMutex(NULL, FALSE, "WincitMutex:Msg");
    }

static void CloseMutex(HANDLE *M)
    {
    if (*M != INVALID_HANDLE_VALUE)
        {
        CloseHandle(*M);
        *M = INVALID_HANDLE_VALUE;
        }
    }

void CloseErrorMutexes(void)
    {
    CloseMutex(&ErrorMutex);
    CloseMutex(&MsgMutex);
    }

long FAR PASCAL errorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
#ifdef NAHERROR
    static int /*ecx,*/ ecy, ecxChar, ecyChar;

    switch (message)
        {
        case WM_CREATE:
            {
            TEXTMETRIC tm;
            HDC hdc = GetDC(hwnd);

            GetTextMetrics(hdc, &tm);
            ecxChar = tm.tmAveCharWidth;
            ecyChar = tm.tmHeight + tm.tmExternalLeading;

            ReleaseDC(hwnd, hdc);

            return (0);
            }

        case WM_SIZE:
            {
            ecy = HIWORD(lParam);
//          ecx = LOWORD(lParam);
            return (0);
            }

        case WM_PAINT:
            {
            int lines, wow, te;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wow = max(ecy - ecyChar / 2, 0);
            lines = wow / ecyChar;

            te = max(0, nextError - lines);

            for (lines = 0; te < nextError; te++, lines++)
                {
                TextOut(hdc, ecxChar, ecyChar * (1 + lines) - ecyChar / 2, errors[te], strlen(errors[te]));
                }

            EndPaint(hwnd, &ps);
            return (0);
            }

        case WM_DESTROY:
            {
            hwndError = NULL;
            return (0);
            }
        }
    return (DefWindowProc(hwnd, message, wParam, lParam));
#endif
    }

long FAR PASCAL msgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    static int /*ecx,*/ ecy, ecxChar, ecyChar;

    switch (message)
        {
        case WM_CREATE:
            {
            TEXTMETRIC tm;
            HDC hdc = GetDC(hwnd);

            GetTextMetrics(hdc, &tm);
            ecxChar = tm.tmAveCharWidth;
            ecyChar = tm.tmHeight + tm.tmExternalLeading;

            ReleaseDC(hwnd, hdc);

            return (0);
            }

        case WM_SIZE:
            {
            ecy = HIWORD(lParam);
//          ecx = LOWORD(lParam);
            return (0);
            }

        case WM_PAINT:
            {
            int lines, wow, te;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wow = max(ecy - ecyChar / 2, 0);
            lines = wow / ecyChar;

            te = max(0, nextMsg - lines);

            for (lines = 0; te < nextMsg; te++, lines++)
                {
                TextOut(hdc, ecxChar, ecyChar * (1 + lines) - ecyChar / 2, messages[te], strlen(messages[te]));
                }

            EndPaint(hwnd, &ps);
            return (0);
            }

        case WM_DESTROY:
            {
            hwndMsg = NULL;
            return (0);
            }
        }
    return (DefWindowProc(hwnd, message, wParam, lParam));
    }


void cdecl errorDisp(const char *fmt, ...)
    {
#ifdef NAHERROR
    int i;
    va_list ap;

    if (ErrorMutex != INVALID_HANDLE_VALUE)
        {
        if (WAITFORmTIMED(ErrorMutex))
            {
            if (fmt)
                {
                if (nextError == NUMERRORS)
                    {
                    for (i = 1; i < NUMERRORS; i++)
                        {
                        strcpy(errors[i-1], errors[i]);
                        }

                    nextError--;
                    }

				char tmp[256];
                va_start(ap, fmt);
                vsprintf(tmp, fmt, ap);
                va_end(ap);

                stripansi(tmp);
				tmp[MSGLENGTH-1] = 0;
	            strcpy(errors[nextError], tmp);

                for (i = 0; errors[nextError][i]; i++)
                    {
                    if (errors[nextError][i] < 32)
                        {
                        errors[nextError][i] = 32;
                        }
                    }

                nextError++;
                }

            if (!hwndError)
                {
                hwndError = CreateWindow("Citadel Error", "Citadel Error Log", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

				if (hwndError)
					{
					ShowWindow(hwndError, SW_SHOWMINNOACTIVE);
					}
                }
            else
                {
                InvalidateRect(hwndError, NULL, TRUE);
                }

            if (!fmt && hwndError)
                {
                ShowWindow(hwndError, IsIconic(hwndError) ? SW_RESTORE : SW_SHOW);
                SetFocus(hwndError);
                }

            UpdateWindow(hwndError);

            RELEASEm(ErrorMutex);
            }
        }
#endif
    }

void cdecl msgDisp(const char *fmt, ...)
    {
    int i;
    va_list ap;

    if (MsgMutex != INVALID_HANDLE_VALUE)
        {
        if (WAITFORmTIMED(MsgMutex))
            {
            if (fmt)
                {
                if (nextMsg == NUMMSGS)
                    {
                    for (i = 1; i < NUMMSGS; i++)
                        {
                        strcpy(messages[i-1], messages[i]);
                        }

                    nextMsg--;
                    }

				char tmp[256];
                va_start(ap, fmt);
                vsprintf(tmp, fmt, ap);
                va_end(ap);

                stripansi(tmp);
				tmp[MSGLENGTH-1] = 0;
	            strcpy(messages[nextMsg], tmp);

                for (i = 0; messages[nextMsg][i]; i++)
                    {
                    if (messages[nextMsg][i] < 32)
                        {
                        messages[nextMsg][i] = 32;
                        }
                    }

                nextMsg++;
                }

            if (!hwndMsg)
                {
                hwndMsg = CreateWindow("Citadel Message", "Citadel Message Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

				if (hwndMsg)
					{
					ShowWindow(hwndMsg, SW_SHOWMINNOACTIVE);
					}
                }
            else
                {
                InvalidateRect(hwndMsg, NULL, TRUE);
                }

            if (!fmt && hwndMsg)
                {
                ShowWindow(hwndMsg, IsIconic(hwndMsg) ? SW_RESTORE : SW_SHOW);
                SetFocus(hwndMsg);
                }

            UpdateWindow(hwndMsg);

            RELEASEm(MsgMutex);
            }
        }
    }

#endif
