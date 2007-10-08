// --------------------------------------------------------------------------
// Citadel: Xmodem.H

#include "filecmd.h"

#define XMODEMMAXERR 10

#define SOH     0x01
#define STX     0x02
#define EOT     0x04
#define ACK     0x06
#define NAK     0x15
#define CAN     0x18
#define CRCINIT 'C'

class XmodemC
    {
    uchar cksum;

    Bool crc;       // must be set before calling xget()
    Bool oneK;      // must be set before calling xget() or xput()
    int bsize;

    unsigned short crcsum;

    char *buf;

    FILE *fp;


#ifdef WINCIT
    TermWindowC *TW;
#endif

    void toUser(const char *Fmt, ...)
        {
        char Buffer[256];
        va_list ap;

        va_start(ap, Fmt);
        vsprintf(Buffer, Fmt, ap);
        va_end(ap);

        tw()CRmPrintfCR(pcts, Buffer);
        }

    void toScreen(const char *Fmt, ...)
        {
        char Buffer[256];
        va_list ap;

        va_start(ap, Fmt);
        vsprintf(Buffer, Fmt, ap);
        va_end(ap);

        tw()cPrintf(pcts, Buffer);
        }

    void toRemote(const int Out)
        {
        tw()CommPort->Output((char) Out);
        }

    uchar fromRemote(void)
        {
        return (tw()CommPort->Input());
        }

    void flushInput(void)
        {
        tw()CommPort->FlushInput();
        }

    Bool haveRemote(void)
        {
        return (tw()CommPort->HaveConnection());
        }

    Bool remoteReady(void)
        {
        return (tw()CommPort->CheckInputReady() != IR_NONE);
        }


    Bool xget(void);
    Bool xput(void);


    void upsum(char c);
    int rchar(uint timeout, uchar *ch);
    void status(const char *string, int blocknum, int errcount);
    void complete(Bool success, int blocknum, int errcount);
    void cksend(char ch);

public:
#ifdef WINCIT
    XmodemC(TermWindowC *NewTW)
        {
        TW = NewTW;
#else
    XmodemC(void)
        {
#endif
        cksum = 0;
        crc = FALSE;
        oneK = FALSE;
        bsize = 0;
        crcsum = 0;
        buf = NULL;
        fp = NULL;
        }

    ~XmodemC(void)
        {
        delete [] buf;

        if (fp)
            {
            fclose(fp);
            }
        }

    Bool Send(const char *Filename, InternalProtocols Mode);
    Bool Receive(const char *Filename, InternalProtocols Mode);
    };
