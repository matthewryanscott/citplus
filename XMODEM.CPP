// --------------------------------------------------------------------------
// Citadel: Xmodem.CPP
//
// xmodem.c copyright 1986 Maple Lawn Farm, Inc.
// modified for MS-DOS and MSC 5.0 by Matthew Pfleger
// supports Xmodem Checksum, Xmodem CRC and Xmodem CRC 1K
// (c) 1988 Anticlimactic Teleservices Ltd.
//
// Updated June 1990 for use with DragCit
//
// And then updated again February 1996 for use with Citadel+/Win32 using
// C++ classes and stuff. It's good to see 10-year-old code still being
// used and updated.

#include "ctdl.h"
#pragma hdrstop


#include "extmsg.h"
#include "xmodem.h"




// --------------------------------------------------------------------------
//  note:   Mode = 0: Xmodem Checksum
//          Mode = 1: Xmodem CRC
//          Mode = 2: Xmodem CRC 1K

Bool XmodemC::Send(const char *Filename, InternalProtocols Mode)
    {
    if (!LockMessages(MSG_XMODEM))
        {
        OutOfMemory(54);
        return (FALSE);
        }

    crc = Mode != IP_XMODEM;
    oneK = Mode == IP_ONEKXMODEM;
    bsize = (oneK) ? 1024 : 128;

    if (fp)
        {
        fclose(fp);
        }

    if ((fp = fopen(Filename, FO_RB)) == NULL)
        {
        toUser(getmsg(78), Filename);
        UnlockMessages(MSG_XMODEM);
        return (FALSE);
        }

    delete [] buf;

    if ((buf = new char[bsize]) == NULL)
        {
        OutOfMemory(54);
        fclose(fp);
        fp = NULL;
        UnlockMessages(MSG_XMODEM);
        return (FALSE);
        }

    const Bool retval = xput();

    fclose(fp);
    fp = NULL;

    delete [] buf;
    buf = NULL;

    UnlockMessages(MSG_XMODEM);
    return (retval);
    }


Bool XmodemC::Receive(const char *Filename, InternalProtocols Mode)
    {
    if (!LockMessages(MSG_XMODEM))
        {
        OutOfMemory(55);
        return (FALSE);
        }

    crc = Mode != IP_XMODEM;
    oneK = Mode == IP_ONEKXMODEM;
    bsize = (oneK) ? 1024 : 128;

    if (fp)
        {
        fclose(fp);
        }

    if ((fp = fopen(Filename, FO_WB)) == NULL)
        {
        toUser(getmsg(661), Filename);
        UnlockMessages(MSG_XMODEM);
        return (FALSE);
        }

    delete [] buf;

    if ((buf = new char[bsize]) == NULL)
        {
        OutOfMemory(55);
        fclose(fp);
        fp = NULL;
        UnlockMessages(MSG_XMODEM);
        return (FALSE);
        }

    const Bool retval = xget();

    fclose(fp);
    fp = NULL;

    delete [] buf;
    buf = NULL;

    if (!retval)
        {
        if (unlink(Filename) == -1)
            {
            toUser(getmsg(662), Filename);
            }
        }

    UnlockMessages(MSG_XMODEM);
    return (retval);
    }

Bool XmodemC::xget(void)
    {
    uchar inch, inch2, crchi, errcount = 0;
    int BlockNumFull = 1, BlockNum8Bit = 1;
    register int i;

    toRemote((char) (crc ? CRCINIT : NAK));
    flushInput();

    for (;;)
        {
        if (!errcount)
            {
            status(getmsg(MSG_XMODEM, 1), BlockNumFull, errcount);
            }

        if (!haveRemote())
            {
            status(getmsg(MSG_XMODEM, 2), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        if (errcount >= XMODEMMAXERR)
            {
            status(getmsg(MSG_XMODEM, 3), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        errcount++;

        if (rchar(10, &inch) == -1)
            {
            if (oneK)
                {
                status(getmsg(MSG_XMODEM, 4), BlockNumFull, errcount);
                }
            else
                {
                status(getmsg(MSG_XMODEM, 5), BlockNumFull, errcount);
                }

            cksend((char) (crc ? CRCINIT : NAK));
            continue;
            }

        if (inch == EOT)
            {
            break;
            }

        if (inch == CAN)
            {
            status(getmsg(MSG_XMODEM, 6), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        if (oneK)
            {
            if (inch != STX)
                {
                status(getmsg(MSG_XMODEM, 7), BlockNumFull, errcount);
                cksend(NAK);
                continue;
                }
            }
        else
            {
            if (inch != SOH)
                {
                status(getmsg(MSG_XMODEM, 8), BlockNumFull, errcount);
                cksend(NAK);
                continue;
                }
            }

        if (rchar(2, &inch) == -1)
            {
            status(getmsg(MSG_XMODEM, 9), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        if (rchar(2, &inch2) == -1)
            {
            status(getmsg(MSG_XMODEM, 10), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        if ((~inch &0xff) != (inch2 &0xff))
            {
            status(getmsg(MSG_XMODEM, 11), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        if (inch == (uchar) ((BlockNum8Bit - 1) & 0xff))
            {
            status(getmsg(MSG_XMODEM, 12), BlockNumFull, errcount);
            cksend(ACK);
            continue;
            }

        if ((inch & 0xff) != BlockNum8Bit)
            {
            status(getmsg(MSG_XMODEM, 13), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        if ((inch2 & 0xff) != (~BlockNum8Bit & 0xff))
            {
            status(getmsg(MSG_XMODEM, 14), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        // Read in 128 byte block without taking time for checksums or crc.
        for (i = 0; i < bsize; i++)
            {
            if (rchar(2, (uchar *) &buf[i]) == -1)
                {
                break;
                }
            }

        if (i < bsize)
            {
            status(getmsg(MSG_XMODEM, 15), BlockNumFull, errcount);
            cksend(NAK);
            continue;
            }

        if (crc)
            {
            if (rchar(2, &crchi) == -1)
                {
                status(getmsg(MSG_XMODEM, 16), BlockNumFull, errcount);
                cksend(NAK);
                continue;
                }
            crchi &= 0xff;
            }

        if (rchar(2, &inch) == -1)
            {
            if (crc)
                {
                status(getmsg(MSG_XMODEM, 17), BlockNumFull, errcount);
                }
            else
                {
                status(getmsg(MSG_XMODEM, 18), BlockNumFull, errcount);
                }

            cksend(NAK);
            continue;
            }

        // Now, when we have the whole packet, do the checksum or crc.
        for (cksum = 0, crcsum = 0, i = 0; i < bsize; i++)
            {
            upsum(buf[i]);
            }

        if (crc)
            {
            upsum(0);                           // needed for crcsum
            upsum(0);
            if ((unsigned short) ((inch & 0xff) + (crchi << 8)) != crcsum)
                {
                status(getmsg(MSG_XMODEM, 19), BlockNumFull, errcount);
                cksend(NAK);
                continue;
                }
            }
        else
            {
            cksum %= 256;
            if (cksum != (inch & 0xff))
                {
                status(getmsg(MSG_XMODEM, 20), BlockNumFull, errcount);
                cksend(NAK);
                continue;
                }
            }

        toRemote(ACK);
        flushInput();

        fwrite(buf, bsize, 1, fp);
        BlockNum8Bit++;
        BlockNum8Bit %= 256;
        BlockNumFull++;
        errcount = 0;
        }

    toRemote(ACK);
    flushInput();

    complete(1, BlockNumFull, errcount - 1);
    return (TRUE);
    }

Bool XmodemC::xput(void)
    {
    uchar inch, errcount = 0;
    int BlockNumFull = 1, BlockNum8Bit = 1;
    register int i;
    int cread;

    flushInput();
    status(getmsg(MSG_XMODEM, 21), BlockNumFull, errcount);
    rchar(60, &inch);

    if (inch == CRCINIT)
        {
        crc = TRUE;
        }
    else if (inch == NAK && !oneK)
        {
        crc = FALSE;
        }
    else
        {
        if (oneK)
            {
            status(getmsg(MSG_XMODEM, 22), BlockNumFull, errcount);
            }
        else
            {
            status(getmsg(MSG_XMODEM, 23), BlockNumFull, errcount);
            }

        pause(200);
        complete(0, BlockNumFull, errcount);
        return (FALSE);
        }

    cread = fread(buf, 1, bsize, fp);

    while (cread)
        {
        if (!errcount)
            {
            status(getmsg(MSG_XMODEM, 24), BlockNumFull, errcount);
            }

        if (!haveRemote())
            {
            status(getmsg(MSG_XMODEM, 2), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        if (errcount >= XMODEMMAXERR)
            {
            status(getmsg(MSG_XMODEM, 3), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        errcount++;

        for (i = cread; i < bsize; i++)
            {
            buf[i] = 0x1a;
            }

        if (oneK)
            {
            toRemote(STX);
            }
        else
            {
            toRemote(SOH);
            }

        toRemote(BlockNum8Bit);
        toRemote((uchar) (~BlockNum8Bit & 0xff));

        for (cksum = 0, crcsum = 0, i = 0; i < bsize; i++)
            {
            toRemote(buf[i]);
            upsum(buf[i]);
            }

        if (crc)
            {
            upsum(0);        // needed for crcsum
            upsum(0);
            toRemote((uchar) (crcsum >> 8));
            toRemote((uchar) crcsum);
            }
        else
            {
            cksum %= 256;
            toRemote(cksum);
            }

//      flushInput();

        if (rchar(15, &inch) == -1)
            {
            status(getmsg(MSG_XMODEM, 25), BlockNumFull, errcount);
            continue;
            }

        if (inch == CAN)
            {
            status(getmsg(MSG_XMODEM, 26), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }

        if (inch != ACK)
            {
            status(getmsg(MSG_XMODEM, 27), BlockNumFull, errcount);
            continue;
            }

        cread = fread(buf, 1, bsize, fp);
        BlockNum8Bit++;
        BlockNum8Bit %= 256;
        BlockNumFull++;
        errcount = 0;
        }

    for (;;)
        {
        toRemote(EOT);
//      flushRemote();

        if (rchar(15, &inch) == -1)
            {
            status(getmsg(MSG_XMODEM, 28), BlockNumFull, errcount);
            continue;
            }
        if (inch == CAN)
            {
            status(getmsg(MSG_XMODEM, 29), BlockNumFull, errcount);
            pause(200);
            complete(0, BlockNumFull, errcount);
            return (FALSE);
            }
        if (inch != ACK)
            {
            status(getmsg(MSG_XMODEM, 30), BlockNumFull, errcount);
            continue;
            }

        break;
        }

    complete(1, BlockNumFull, errcount);
    return (TRUE);
    }


void XmodemC::upsum(char c)
    {
    register unsigned short shift;
    register unsigned short flag;

    if (crc)
        {
        for (shift = 0x80; shift; shift >>= 1)
            {
            flag = (crcsum & 0x8000);
            crcsum <<= 1;
            crcsum |= ((shift & c) ? 1 : 0);
            if (flag)
                {
                crcsum ^= 0x1021;
                }
            }
        }
    else
        {
        cksum += c;
        }
    }


int XmodemC::rchar(uint timeout, uchar *ch)
    {
    time_t t = time(NULL);

    while ((uint) (time(NULL) - t) < timeout)
        {
        if (!haveRemote())
            {
            return (-1);
            }

        if (remoteReady())
            {
            *ch = fromRemote();
            return (1);
            }
        }

    return (-1);
    }


void XmodemC::status(const char *string, int blocknum, int errcount)
    {
    label Block;
    CopyStringToBuffer(Block, tw()ltoac(blocknum));

    toScreen(br);
    toScreen(getmsg(MSG_XMODEM, 31), string, Block, tw()ltoac(errcount));
    toScreen(spcspc);
    }


void XmodemC::complete(Bool success, int blocknum, int errcount)
    {
    char message[80];

    sprintf(message, getmsg(MSG_XMODEM, 32), success ?
            getmsg(MSG_XMODEM, 33) : getmsg(MSG_XMODEM, 34));

    status(message, blocknum, errcount);
    toScreen(bn);
    }


void XmodemC::cksend(char ch)
    {
    int j;
    uchar cp;

    do
        {
        j = rchar(2, &cp);
        } while (j != -1);

    toRemote(ch);
    flushInput();
    }
