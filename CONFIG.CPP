// --------------------------------------------------------------------------
// Citadel: Config.CPP
//
// Configuration routines.

#include "ctdl.h"
#pragma hdrstop

#include "config.h"
#include "libovl.h"
#include "log.h"
#include "timer.h"
#include "cwindows.h"
#include "term.h"
#include "extmsg.h"
#include "msg.h"


// --------------------------------------------------------------------------
// Contents
//
// configcit        the main configuration for citadel
// illegal          abort configuration program
// initfiles() 		initializes files, opens them
// autoConvert()	up/downsizes .DAT files
// checkresize()	checks to see if files need to be resized.


// --------------------------------------------------------------------------
// illegal(): Prints out configuration error message and aborts.

void cdecl illegal(const char *fmt, ...)
    {
    char buff[256];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    changedir(cfg.homepath);

#ifdef WINCIT
    errorDisp(pcts, buff);
    MessageBox(NULL, getcfgmsg(136), NULL, MB_ICONSTOP | MB_OK);
#else
    doccr();
//    errorDisp(pcts, buff);
    doccr();
//    errorDisp(getcfgmsg(136));
    doccr();
    curson();
    DeinitializeTimer();
    critical(FALSE);
#endif
    exit(7);
    }


// --------------------------------------------------------------------------
// Tell the user to wait until configuration is done before he can do
// something he tried to do.

void WaitUntilDoneConfiguring(void)
    {
    CitWindowsNote(NULL, getcfgmsg(286));
    }


// --------------------------------------------------------------------------

#ifdef WINCIT
#define TWip    TermWindowC *TW
#else
#define TWip    int
#endif

void mPrintfOrUpdateCfgDlg(TWip, const char *fmt, ...)
    {
    va_list ap;
    char Buffer[256];

    va_start(ap, fmt);
    vsprintf(Buffer, fmt, ap);
    va_end(ap);

#ifdef WINCIT
    errorDisp(pcts, Buffer);
    if (TW)
#endif
        {
        tw()mPrintf(pcts, Buffer);
        }
    }


void CRmPrintfOrUpdateCfgDlg(TWip, const char *fmt, ...)
    {
    va_list ap;
    char Buffer[256];

    va_start(ap, fmt);
    vsprintf(Buffer, fmt, ap);
    va_end(ap);

#ifdef WINCIT
    errorDisp(pcts, Buffer);
    if (TW)
#endif
        {
        tw()CRmPrintf(pcts, Buffer);
        }
    }


void CRmPrintfCROrUpdateCfgDlg(TWip, const char *fmt, ...)
    {
    va_list ap;
    char Buffer[256];

    va_start(ap, fmt);
    vsprintf(Buffer, fmt, ap);
    va_end(ap);

#ifdef WINCIT
    errorDisp(pcts, Buffer);
    if (TW)
#endif
        {
        tw()CRmPrintfCR(pcts, Buffer);
        }
    }


void mPrintfCROrUpdateCfgDlg(TWip, const char *fmt, ...)
    {
    va_list ap;
    char Buffer[256];

    va_start(ap, fmt);
    vsprintf(Buffer, fmt, ap);
    va_end(ap);

#ifdef WINCIT
    errorDisp(pcts, Buffer);
    if (TW)
#endif
        {
        tw()mPrintfCR(pcts, Buffer);
        }
    }
