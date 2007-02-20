// --------------------------------------------------------------------------
// Citadel: Zmodem.CPP
//
// Source file for Citadel Zmodem.

#include "zmodem.h"


static Bool THREAD needcr = FALSE;

void cdecl TERMWINDOWMEMBER z_message(const char *fmt, ...)
    {
    va_list arg_ptr;

    va_start(arg_ptr, fmt);

    if (!fmt)
        {
        return;
        }

    vsprintf((char *) e_input, fmt, arg_ptr);

    doccr();
    cPrintf(pcts, (char *) e_input);

    va_end(arg_ptr);

    needcr = TRUE;
    }

void cdecl TERMWINDOWMEMBER z_status(const char *fmt, ...)
    {
    va_list arg_ptr;

    va_start(arg_ptr, fmt);

    if (fmt == NULL)
        {
        return;
        }

    vsprintf((char *) e_input, fmt, arg_ptr);

    if (needcr)
        {
        doccr();
        needcr = FALSE;
        }

    cPrintf(br);
    cPrintf((char *) e_input);

    va_end(arg_ptr);
    }

void TERMWINDOWMEMBER show_loc(ulong l, uint w)
    {
    cPrintf(getzmmsg(1), l, w);
    doccr();
    }

// This file contains routines to implement a simple multiple alarm system.
// The routines allow setting any number of alarms, and then checking if any
// one of them has expired. It also allows adding time to an alarm.

// long timerset (t) uint t;
//
// This routine returns a timer variable based on the MS-DOS time. The
// variable returned is a long which corresponds to the MS-DOS time at which
// the timer will expire. The variable need never be used once set, but to
// find if the timer has in fact expired, it will be necessary to call the
// timeup function. The expire time 't' is in hundredths of a second.
//
// Note: This routine as coded had a granularity of one week. I have put the
// code that implements that inside the flag "HIGH_OVERHEAD". If you want it,
// go ahead and use it. For BT's purposes, (minute) granularity should
// suffice.

#ifndef WINCIT
static void dostime(int *hour, int *min, int *sec, int *hdths)
    {
    union REGS r;

    r.h.ah = 0x2c;

    intdos(&r, &r);

    *hour = r.h.ch;
    *min = r.h.cl;
    *sec = r.h.dh;
    *hdths = r.h.dl;
    }

static ulong timerset(uint t)
    {
    long l;

#ifdef HIGH_OVERHEAD
    int l2;
#endif
    int hours, mins, secs, ths;

#ifdef HIGH_OVERHEAD
    extern int week_day();
#endif

    dostime(&hours, &mins, &secs, &ths);

#ifdef HIGH_OVERHEAD
    l2 = week_day();
#endif

    // Figure out the hundredths of a second so far this week
    l =

#ifdef HIGH_OVERHEAD
        l2 * PER_DAY + (hours % 24) * PER_HOUR +
#endif
        (mins % 60) * PER_MINUTE + (secs % 60) * PER_SECOND + ths;

    // Add in the timer value
    l += t;

    // Return the alarm off time
    return (l);
    }

// int timeup (t) long t;
//
// This routine returns a 1 if the passed timer variable corresponds to a
// timer which has expired, or 0 otherwise.

static int timeup(ulong t)
    {
    ulong l = timerset(0);

    // If current is less then set by more than max int, then adjust
    if (l < (t - 65536L))
        {
#ifdef HIGH_OVERHEAD
        l += PER_WEEK;
#else
        l += PER_HOUR;
#endif
        }

    return (l >= t);
    }

#else


// These fail when the Windows tick count wraps, once approximately every
// 49.7 days. I should fix this, but later...

static ulong timerset(uint t)
    {
    return (GetTickCount() + t * 10);
    }

// int timeup (t) long t;
//
// This routine returns a 1 if the passed timer variable corresponds to a
// timer which has expired, or 0 otherwise.

static int timeup(ulong t)
    {
    return (GetTickCount() > t);
    }

#endif

// ---------------------------------------------------------------------------
// THROUGHPUT
//  Print throughput message at end of transfer

void TERMWINDOWMEMBER throughput(int opt, ulong bytes)
    {
    static long started = 0L;
    static long elapsed;
    static long cps;

    if (!opt)
        {
        started = time(NULL);
        }
    else if (started)
        {
        elapsed = time(NULL);

        // The next line tests for day wrap without the date rolling over
        if (elapsed < started)
            {
            elapsed += 86400L;
            }

        elapsed -= started;

        if (elapsed == 0L)
            {
            elapsed = 1L;
            }

        cps = (long) (bytes / (ulong) elapsed);
        started = (cps * 1000L) / ((long) baud_rate);

        z_message(getzmmsg(2), cps, bytes, started);
        }
    }


void TERMWINDOWMEMBER send_can(void)
    {
    int i;

    CommPort->FlushInput();

    for (i = 0; i < 10; i++)
        {
        CommPort->Output(CAN);
        }

    for (i = 0; i < 10; i++)
        {
        CommPort->Output(BS);
        }

    CommPort->FlushOutput();
    }

void TERMWINDOWMEMBER show_debug_name(int index)
    {
    if (debug)
        {
        z_message(getdbmsg(index));
        }
    }


// --------------------------------------------------------------------------
// UNIQUE_NAME
//  Increments the suffix of a filename as necessary to make the name unique

static void unique_name(char *fname)
    {
    register char *p;
    register int n;

    if (filexists(fname))
        {
        p = fname;

        while (*p && *p != '.')
            {
            p++;    // find the extension, if any
            }

        for (n = 0; n < 4; n++) // fill it out if neccessary
            {
            if (!*p)
                {
                *p = getzmmsg(48)[n];
                *(++p) = '\0';
                }
            else
                {
                p++;
                }
            }

        while (filexists(fname))
            {       // If 'file.ext' exists suffix++
            p = fname + strlen(fname) - 1;

            for (n = 3; n--;)
                {
                if (!isdigit(*p))
                    {
                    *p = '0';
                    }

                if (++(*p) <= '9')
                    {
                    break;
                    }
                else
                    {
                    *p-- = '0';
                    }
                }
            }
        }
    }

Bool TERMWINDOWMEMBER got_ESC(void)
    {
    while (KBReady())
        {
        if (KeyboardBuffer.Retrieve() == ESC)
            {
            KeyboardBuffer.Flush();
            return (TRUE);
            }
        }

    return (FALSE);
    }

// Handle uploads from terminal mode.

int TERMWINDOWMEMBER Zmodem_Send_Batch(const char *filepath, uint baud)
    {
    if (!read_zm_messages())
        {
        OutOfMemory(12);
        return (0);
        }

    cr3tab = (long) ExtMessages[MSG_ZMODEM].Data->next->data;
    crctab = (long) ExtMessages[MSG_ZMODEM].Data->next->next->data;

    int count;
    char *words[256];

    char temp[81];
    CopyStringToBuffer(temp, filepath);
    count = parse_it(words, temp);

    if (count == 0)
        {
        z_message(getzmmsg(3));
        dump_zm_messages();
        VerifyHeap();
        return (0);
        }

    baud_rate = baud;       // kinda icky

    Txbuf = (long) calloc(1, BUFSIZE + 16);

    if (!Txbuf)
        {
        OutOfMemory(12);
        dump_zm_messages();
        VerifyHeap();
        return (0);
        }

    e_input = (long) calloc(1, 255);

    if (!e_input)
        {
        OutOfMemory(12);
        free((void *) Txbuf);
        dump_zm_messages();
        VerifyHeap();
        return (0);
        }


    // Disambiguate
    int err = 1;    // Xmodem, Ymodem, Telink flag
    for (int i = 0; (i < count) && err; i++)
        {
        FindFileC FndF;

        if (!FndF.FindFirst(words[i]))
            {
            continue;
            }

#ifdef WINCIT
        char currentfilename[MAX_PATH];
#else
        char currentfilename[_MAX_PATH];
#endif

        CopyStringToBuffer(currentfilename, words[i]);

        // Find the delimiter on the pathspec for use by the various
        // batch protocols.

        char *p = strrchr(currentfilename, '\\');

        if (p == NULL)
            {
            p = strrchr(currentfilename, '/');
            }

        if (p == NULL)
            {
            p = strchr(currentfilename, ':');
            }

        if (p == NULL)
            {
            p = currentfilename;
            }
        else
            {
            p++;
            }


        // At this point *p points to the location in the input string
        // where the prepended path information ends. All we need to do,
        // then, is to keep plugging in the stuff we get from
        // _dos_find(first|next) and transfer files. We already have the
        // first matching filename from the _dos_findfirst we did above,
        // so we use a "do" loop.

        do
            {
            if (FndF.IsNormalFile())
                {
                // Append the current filename
                strcpy(p, FndF.GetFullName());

                err = Send_Zmodem(currentfilename, 0);
                }
            else
                {
                err = TRUE;
                }
            } while (err && FndF.FindNext());
        }

    if (err)
        {
        Send_Zmodem(NULL, END_BATCH);
        }

    free((void *) Txbuf);
    free((void *) e_input);
    dump_zm_messages();
    VerifyHeap();
    return (err);
    }

// Handle downloads from terminal mode.
//
// If filespec is NULL and we need a filename, ask for it. Otherwise, assume
// that the spec we got is OK and use it.

int TERMWINDOWMEMBER Zmodem_Receive_Batch(const char *filepath, uint baud)
    {
    if (!read_zm_messages())
        {
        OutOfMemory(13);
        return (0);
        }

    cr3tab = (long) ExtMessages[MSG_ZMODEM].Data->next->data;
    crctab = (long) ExtMessages[MSG_ZMODEM].Data->next->next->data;

    baud_rate = baud;       // kinda icky

    Txbuf = (long) calloc(1, BUFSIZE + 16);

    if (!Txbuf)
        {
        OutOfMemory(13);
        dump_zm_messages();
        VerifyHeap();
        return (0);
        }

    e_input = (long) calloc(1, 255);

    if (!e_input)
        {
        OutOfMemory(13);
        free((void *) Txbuf);
        dump_zm_messages();
        VerifyHeap();
        return (0);
        }

    int err = get_Zmodem(filepath, NULL);

    free((void *) Txbuf);
    free((void *) e_input);
    dump_zm_messages();
    VerifyHeap();
    return (err);
    }

// utime function for Turbo / Borland C.
//
// We should make this more generic in case some other DOS compiler comes up
// lacking, but for now the only one we have that needs it happens to be
// Borland.

#ifndef VISUALC
static int cdecl b_utime(const char *name, struct utimbuf * times)
    {
    int handle;
    struct date d;
    struct time t;
    struct ftime ft;
    unixtodos(times->modtime, &d, &t);

    ft.ft_tsec = t.ti_sec / 2;
    ft.ft_min = t.ti_min;
    ft.ft_hour = t.ti_hour;
    ft.ft_day = d.da_day;
    ft.ft_month = d.da_mon;
    ft.ft_year = d.da_year - 1980;

    if ((handle = open(name, O_RDONLY)) == -1)
        {
        return (-1);
        }

    setftime(handle, &ft);
    close(handle);
    return (0);
    }
#endif

int TERMWINDOWMEMBER got_error(const char *string1, const char *string2)
    {
    if (errno == 0x18)
        {
        errno = 0;
        }

    if (errno != 0)
        {
        z_message(getzmmsg(4), errno, string1, string2);
        errno = 0;
        return (1);
        }

    return (0);
    }

#ifdef WINCIT
static long zfree(char *drive)
    {
    char Root[4];
    Root[0] = *drive;
    Root[1] = ':';
    Root[2] = '\\';
    Root[3] = 0;
    __int64 freespace = CitGetDiskFreeSpace(*Root ? Root : NULL);

    if (freespace > LONG_MAX)
        {
        return(LONG_MAX);
        }
    else
        {
        return(freespace);
        }
    }
#else
static long zfree(char *drive)
    {
    union REGS r;

    uchar driveno;

    if (drive[0] != '\0' && drive[1] == ':')
        {
        driveno = (uchar) (islower(*drive) ? toupper(*drive) : *drive);
        driveno = (uchar) (driveno - 'A' + 1);
        }
    else
        {
        driveno = 0;
        }

    r.x.ax = 0x3600;
    r.h.dl = driveno;
    int86(0x21, &r, &r);

    if (r.x.ax == 0xffff)
        {
        return (0);
        }

    return ((long) r.x.bx * (long) r.x.ax * (long) r.x.cx);
    }
#endif

// --------------------------------------------------------------------------
// Z GET BYTE
// Get a byte from the modem; return TIMEOUT if no read within timeout
//  tenths; return RCDO if carrier lost

int TERMWINDOWMEMBER Z_GetByte(int tenths)
    {
    if (CommPort->CheckInputReady() != IR_NONE)
        {
        return (CommPort->Input());
        }

    ulong timeout = timerset(tenths * 10);

    do
        {
        if (CommPort->IsInputReady() != IR_NONE)
            {
            return (CommPort->Input());
            }

        if (!CommPort->HaveConnection())
            {
            return (RCDO);
            }

        if (got_ESC())
            {
            return -1;
            }
        } while (!timeup(timeout));

    return (TIMEOUT);
    }

// --------------------------------------------------------------------------
// Z PUT STRING
// Send a string to the modem, processing for \336 (sleep 1 sec)
//  and \335 (break signal, ignored)

void TERMWINDOWMEMBER Z_PutString(register uchar *s)
    {
    register uint c;

    while (*s)
        {
        switch (c = *s++)
            {
            case (uint) '\336':
                {
                ulong timeout = timerset(200);

                while (!timeup(timeout))
                    {
                    if (CommPort->CheckInputReady() != IR_NONE)
                        {
                        break;
                        }
                    }

                break;
                }

            case (uint) '\335':
                {
                // Should send a break on this
                break;
                }

            default:
                {
                CommPort->Output((uchar) c);
                break;
                }
            }
        }

    CommPort->FlushOutput();
    }

// --------------------------------------------------------------------------
// Z SEND HEX HEADER
// Send ZMODEM HEX header hdr of type type

void TERMWINDOWMEMBER Z_SendHexHeader(uint type, register uchar *hdr)
    {
    register int n;
    register int i;
    register ushort crc;

    show_debug_name(61);

    CommPort->Output(ZPAD);
    CommPort->Output(ZPAD);
    CommPort->Output(ZDLE);
    CommPort->Output(ZHEX);

    Z_PUTHEX(i, type);

    Crc32t = 0;
    crc = Z_UpdateCRC(type, 0);

    for (n = 4; --n >= 0;)
        {
        Z_PUTHEX(i, (*hdr));
        crc = Z_UpdateCRC(((uint) (*hdr++)), crc);
        }

    Z_PUTHEX(i, (crc >> 8));
    Z_PUTHEX(i, crc);

    // Make it printable on remote machine
    CommPort->Output('\r');
    CommPort->Output('\n');

    // Uncork the remote in case a fake XOFF has stopped data flow
    if (type != ZFIN && type != ZACK)
        {
        CommPort->Output(021);
        }

    CommPort->FlushOutput();
    }


// --------------------------------------------------------------------------
// Z LONG TO HEADER
// Store long integer pos in Txhdr

static void Z_PutLongIntoHeader(long pos)
    {
#ifndef GENERIC
    *((long *) Txhdr) = pos;
#else
    Txhdr[ZP0] = pos;
    Txhdr[ZP1] = pos >> 8;
    Txhdr[ZP2] = pos >> 16;
    Txhdr[ZP3] = pos >> 24;
#endif
    }

// --------------------------------------------------------------------------
// Z PULL LONG FROM HEADER
// Recover a long integer from a header

static long _Z_PullLongFromHeader(uchar *hdr)
    {
#ifndef GENERIC
    return (*((long *) hdr));   // PLF Fri  05-05-1989  06:42:41
#else

    long l;

    l = hdr[ZP3];
    l = (l << 8) | hdr[ZP2];
    l = (l << 8) | hdr[ZP1];
    l = (l << 8) | hdr[ZP0];
    return (l);
#endif
    }

// --------------------------------------------------------------------------
// Z GET HEADER
// Read a ZMODEM header to hdr, either binary or hex.
//  On success, set Zmodem to 1 and return type of header.
//  Otherwise return negative on error

int TERMWINDOWMEMBER Z_GetHeader(byte *hdr)
    {
    register int c;
    register int n;
    int cancount;

    show_debug_name(62);

    n = baud_rate;      // Max characters before start of frame
    cancount = 5;

Again:
    if (got_ESC())
        {
        send_can();
        z_message(getzmmsg(5));
        return (ZCAN);
        }

    Rxframeind = Rxtype = 0;

    switch (c = _Z_TimedRead())
        {
        case ZPAD:
        case ZPAD | 0200:
            {
            // This is what we want.
            break;
            }

        case RCDO:
        case TIMEOUT:
            {
            goto Done;
            }

        case CAN:
GotCan:
            if (--cancount <= 0)
                {
                c = ZCAN;
                goto Done;
                }

            switch (c = Z_GetByte(1))
                {
                case TIMEOUT:
                    {
                    goto Again;
                    }

                case ZCRCW:
                    c = ZERROR;
                    // fallthrough...

                case RCDO:
                    {
                    goto Done;
                    }

                case CAN:
                    {
                    if (--cancount <= 0)
                        {
                        c = ZCAN;
                        goto Done;
                        }

                    goto Again;
                    }
                }

            // fallthrough...

        default:
            {
Agn2:
            if (--n <= 0)
                {
                z_message(getzmmsg(6));
                return (ZERROR);
                }

            if (c != CAN)
                {
                cancount = 5;
                }

            goto Again;
            }
        }

    cancount = 5;

Splat:
    switch (c = _Z_TimedRead())
        {
        case ZDLE:
            {
            // This is what we want.
            break;
            }

        case ZPAD:
            {
            goto Splat;
            }

        case RCDO:
        case TIMEOUT:
            {
            goto Done;
            }

        default:
            {
            goto Agn2;
            }
        }

    switch (c = _Z_TimedRead())
        {
        case ZBIN:
            {
            Rxframeind = ZBIN;
            Crc32 = 0;
            c = _Z_GetBinaryHeader(hdr);
            break;
            }

        case ZBIN32:
            {
            Crc32 = Rxframeind = ZBIN32;
            c = _Z_32GetBinaryHeader(hdr);
            break;
            }

        case ZHEX:
            {
            Rxframeind = ZHEX;
            Crc32 = 0;
            c = _Z_GetHexHeader(hdr);
            break;
            }

        case CAN:
            {
            goto GotCan;
            }

        case RCDO:
        case TIMEOUT:
            {
            goto Done;
            }

        default:
            {
            goto Agn2;
            }
        }

    Rxpos = _Z_PullLongFromHeader(hdr);

Done:
    return (c);
    }

// --------------------------------------------------------------------------
// Z GET BINARY HEADER
// Receive a binary style header (type and position)

int TERMWINDOWMEMBER _Z_GetBinaryHeader(register uchar *hdr)
    {
    register int c;
    register uint crc;
    register int n;

    show_debug_name(63);

    if ((c = Z_GetZDL()) & ~0xFF)
        {
        return (c);
        }

    Rxtype = c;
    crc = Z_UpdateCRC(c, 0);

    for (n = 4; --n >= 0;)
        {
        if ((c = Z_GetZDL()) & ~0xFF)
            {
            return (c);
            }

        crc = Z_UpdateCRC(c, crc);
        *hdr++ = (uchar) (c & 0xff);
        }

    if ((c = Z_GetZDL()) & ~0xFF)
        {
        return (c);
        }

    crc = Z_UpdateCRC(c, crc);

    if ((c = Z_GetZDL()) & ~0xFF)
        {
        return (c);
        }

    crc = Z_UpdateCRC(c, crc);

    if (crc & 0xFFFF)
        {
        z_message(getzmmsg(7));
        return (ZERROR);
        }

    return (Rxtype);
    }


// --------------------------------------------------------------------------
// Z GET BINARY HEADER with 32 bit CRC
// Receive a binary style header (type and position)

int TERMWINDOWMEMBER _Z_32GetBinaryHeader(register uchar *hdr)
    {
    register int c;
    register ulong crc;
    register int n;

    show_debug_name(64);

    if ((c = Z_GetZDL()) & ~0xFF)
        {
        return (c);
        }

    Rxtype = c;
    crc = 0xFFFFFFFFl;
    crc = Z_32UpdateCRC(c, crc);

    for (n = 4; --n >= 0;)
        {
        if ((c = Z_GetZDL()) & ~0xFF)
            {
            return (c);
            }

        crc = Z_32UpdateCRC(c, crc);
        *hdr++ = (uchar) (c & 0xff);
        }

    for (n = 4; --n >= 0;)
        {
        if ((c = Z_GetZDL()) & ~0xFF)
            {
            return (c);
            }

        crc = Z_32UpdateCRC(c, crc);
        }

    if (crc != 0xDEBB20E3l)
        {
        z_message(getzmmsg(7));
        return (ZERROR);
        }

    return (Rxtype);
    }

// --------------------------------------------------------------------------
// Z GET HEX
// Decode two lower case hex digits into an 8 bit byte value

int TERMWINDOWMEMBER _Z_GetHex(void)
    {
    register int c, n;

    show_debug_name(65);

    if ((n = _Z_TimedRead()) < 0)
        {
        return (n);
        }

    n -= '0';
    if (n > 9)
        {
        n -= ('a' - ':');
        }

    if (n & ~0xF)
        {
        return (ZERROR);
        }

    if ((c = _Z_TimedRead()) < 0)
        {
        return (c);
        }

    c -= '0';
    if (c > 9)
        {
        c -= ('a' - ':');
        }

    if (c & ~0xF)
        {
        return (ZERROR);
        }

    return ((n << 4) | c);
    }

// --------------------------------------------------------------------------
// Z GET HEX HEADER
// Receive a hex style header (type and position)

int TERMWINDOWMEMBER _Z_GetHexHeader(register uchar *hdr)
    {
    register int c;
    register uint crc;
    register int n;

    show_debug_name(66);

    if ((c = _Z_GetHex()) < 0)
        {
        return (c);
        }

    Rxtype = c;
    crc = Z_UpdateCRC(c, 0);

    for (n = 4; --n >= 0;)
        {
        if ((c = _Z_GetHex()) < 0)
            {
            return (c);
            }

        crc = Z_UpdateCRC(c, crc);
        *hdr++ = (uchar) c;
        }

    if ((c = _Z_GetHex()) < 0)
        {
        return (c);
        }

    crc = Z_UpdateCRC(c, crc);
    if ((c = _Z_GetHex()) < 0)
        {
        return (c);
        }

    crc = Z_UpdateCRC(c, crc);
    if (crc & 0xFFFF)
        {
        z_message(getzmmsg(7));
        return (ZERROR);
        }

    if (Z_GetByte(1) == '\r')
        {
        Z_GetByte(1);       // Throw away possible cr/lf
        }

    return (Rxtype);
    }


// --------------------------------------------------------------------------
// Z GET ZDL
// Read a byte, checking for ZMODEM escape encoding including CAN*5 which
// represents a quick abort

int TERMWINDOWMEMBER Z_GetZDL(void)
    {
    register int c;

    if ((c = Z_GetByte(Rxtimeout)) != ZDLE)
        {
        return (c);
        }

    switch (c = Z_GetByte(Rxtimeout))
        {
        case CAN:
            {
            return ((c = Z_GetByte(Rxtimeout)) < 0) ? c :
                    ((c == CAN) && ((c = Z_GetByte(Rxtimeout)) < 0)) ? c :
                    ((c == CAN) && ((c = Z_GetByte(Rxtimeout)) < 0)) ? c : (GOTCAN);
            }

        case ZCRCE:
        case ZCRCG:
        case ZCRCQ:
        case ZCRCW:
            {
            return (c | GOTOR);
            }

        case ZRUB0:
            {
            return (0x7F);
            }

        case ZRUB1:
            {
            return (0xFF);
            }

        default:
            {
            return (c < 0) ? c :
                    ((c & 0x60) == 0x40) ? (c ^ 0x40) : ZERROR;
            }
        }
    }

// --------------------------------------------------------------------------
// Z TIMED READ
// Read a character from the modem line with timeout. Eat parity, XON and
// XOFF characters.

int TERMWINDOWMEMBER _Z_TimedRead(void)
    {
    register int c;

    show_debug_name(67);

    for (;;)
        {
        if ((c = Z_GetByte(Rxtimeout)) < 0)
            {
            return (c);
            }

        switch (c &= 0x7F)
            {
            case XON:
            case XOFF:
                {
                continue;
                }

            default:
                {
                if (!(c & 0x60))
                    {
                    continue;
                    }
                }

            case '\r':
            case '\n':
            case ZDLE:
                {
                return (c);
                }
            }
        }
    }


// --------------------------------------------------------------------------
// SEND ZMODEM (send a file)
//  returns TRUE (1) for good xfer, FALSE (0) for bad
//  sends one file per call; 'fsent' flags start and end of batch

int TERMWINDOWMEMBER Send_Zmodem(char *fname, int fsent)
    {
    register byte *p;
    register byte *q;
    struct stat f;
    int i;
    int rc = TRUE;
    char j[200];

    show_debug_name(68);

    z_size = 0;
    Infile = NULL;

    switch (fsent)
        {
        case 0:
            Z_PutString((byte *) getzmmsg(0));
            Z_PutLongIntoHeader(0L);
            Z_SendHexHeader(ZRQINIT, (byte *) Txhdr);
            // Fall through

        case NOTHING_TO_DO:
            {
            Rxtimeout = 200;
            if (ZS_GetReceiverInfo() == ZERROR)
                {
                return (FALSE);
                }

            break;
            }
        }

    Rxtimeout = (int) (614400L / (long) baud_rate);

    if (Rxtimeout < 100)
        {
        Rxtimeout = 100;
        }

    if (fname == NULL)
        {
        goto Done;
        }

    // Prepare the file for transmission. Just ignore file open errors
    // because there may be other files that can be sent.
    Filename = (long) fname;
    errno = 0;

#ifdef MINGW
    if ((Infile = (long) fopen(Filename, FO_RB /*, SH_DENYWR /* DENY_WRITE */ )) == NULL)
#else
    if ((Infile = (long) _fsopen((char *) Filename, FO_RB, SH_DENYWR /* DENY_WRITE */ )) == NULL)
#endif
        {
        got_error(getzmmsg(8), (char *) Filename);
        rc = OK;
        goto Done;
        }

    if (isatty(fileno((FILE *) Infile)))
        {
        errno = 1;
        got_error(getzmmsg(9), (char *) Filename);
        rc = OK;
        goto Done;
        }

    // Send the file
    // Display outbound filename, size, and ETA for sysop
    stat((char *) Filename, &f);

    i = (int) (f.st_size * 10 / baud_rate + 53) / 54;
    
    // Limit filenames to 64 chars.   
    if (strlen(fname) > 64)
        {
        fname[64] = 0;
        }

    sprintf(j, getzmmsg(10), (char *) Filename, f.st_size, i);
    file_length = f.st_size;

    z_message(pcts, j);

    // Get outgoing file name; no directory path, lower case

    for (p = (byte *) Filename, q = (byte *) Txbuf; *p;)
        {
        if ((*p == '/') || (*p == '\\') || (*p == ':'))
            {
            q = (byte *) Txbuf;
            }
        else
            {
            *q++ = (char) tolower(*p);
            }

        p++;
        }

    *q++ = '\0';
    p = q;

    // Zero out remainder of file header packet
    while (q < ((byte *) Txbuf + KSIZE))
        {
        *q++ = '\0';
        }

    // Store filesize, time last modified, and file mode in header packet
    sprintf((char *) p, getzmmsg(11), f.st_size, f.st_mtime, f.st_mode);

    // Transmit the filename block and { the download
    throughput(0, 0L);

    // Check the results
    switch (ZS_SendFile(1 + strlen((char *) p) + (int) (p - (byte *) Txbuf)))
        {
        case ZERROR:
            // Something tragic happened
            goto Err_Out;

        case OK:
            // File was sent
            errno = 0;
            fclose((FILE *) Infile);
            Infile = NULL;

            z_message(getzmmsg(12), getzmmsg(13), Crc32t ? getzmmsg(14) : ns, (char *) Filename);
            doccr();

            goto Done;

        case ZSKIP:
            z_message(getzmmsg(15), (char *) Filename);
            rc = SPEC_COND; // Success but don't truncate!
            goto Done;

        default:
            // Ignore the problem, get next file, trust other
            // error handling mechanisms to deal with problems
            goto Done;
        }

Err_Out:
    rc = FALSE;

Done:
    if (Infile)
        {
        fclose((FILE *) Infile);
        }

    if (fsent < 0)
        {
        ZS_EndSend();
        }

    return (rc);
    }


// --------------------------------------------------------------------------
// ZS SEND BINARY HEADER
// Send ZMODEM binary header hdr of type type

void TERMWINDOWMEMBER ZS_SendBinaryHeader(uint type, register byte * hdr)
    {
    register uint crc;
    int n;

    show_debug_name(69);

    CommPort->Output(ZPAD);
    CommPort->Output(ZDLE);

    if ((Crc32t = Txfcs32) != 0)
        {
        ZS_32SendBinaryHeader(type, hdr);
        }
    else
        {
        CommPort->Output(ZBIN);
        ZS_SendByte((byte) type);

        crc = Z_UpdateCRC(type, 0);

        for (n = 4; --n >= 0;)
            {
            ZS_SendByte(*hdr);
            crc = Z_UpdateCRC(((uint) (*hdr++)), crc);
            }

        ZS_SendByte((byte) (crc >> 8));
        ZS_SendByte((byte) crc);
        }

    CommPort->FlushOutput();
    }


// --------------------------------------------------------------------------
// ZS SEND BINARY HEADER
// Send ZMODEM binary header hdr of type type

void TERMWINDOWMEMBER ZS_32SendBinaryHeader(uint type, register byte * hdr)
    {
    ulong crc;
    int n;

    show_debug_name(70);

    CommPort->Output(ZBIN32);
    ZS_SendByte((byte) type);

    crc = 0xFFFFFFFFl;
    crc = Z_32UpdateCRC(type, crc);

    for (n = 4; --n >= 0;)
        {
        ZS_SendByte(*hdr);
        crc = Z_32UpdateCRC(((uint) (*hdr++)), crc);
        }

    crc = ~crc;
    for (n = 4; --n >= 0;)
        {
        ZS_SendByte((byte) crc);
        crc >>= 8;
        }

    CommPort->FlushOutput();
    }

// --------------------------------------------------------------------------
// ZS SEND DATA
// Send binary array buf with ending ZDLE sequence frameend

void TERMWINDOWMEMBER ZS_SendData(register byte * buf, int length, uint frameend)
    {
    register uint crc;

    show_debug_name(71);

    if (Crc32t)
        {
        ZS_32SendData(buf, length, frameend);
        }
    else
        {
        crc = 0;
        for (; --length >= 0;)
            {
            ZS_SendByte(*buf);
            crc = Z_UpdateCRC(((uint) (*buf++)), crc);
            }

        CommPort->Output(ZDLE);
        CommPort->Output((uchar) frameend);

        crc = Z_UpdateCRC(frameend, crc);
        ZS_SendByte((byte) (crc >> 8));
        ZS_SendByte((byte) crc);
        }

    if (frameend == ZCRCW)
        {
        CommPort->Output(XON);
        }

    CommPort->FlushOutput();
    }

// --------------------------------------------------------------------------
// ZS SEND DATA with 32 bit CRC
// Send binary array buf with ending ZDLE sequence frameend

void TERMWINDOWMEMBER ZS_32SendData(register byte * buf, int length, uint frameend)
    {
    ulong crc;

    show_debug_name(72);

    crc = 0xFFFFFFFFl;
    for (; --length >= 0; ++buf)
        {
        ZS_SendByte(*buf);
        crc = Z_32UpdateCRC(((uint) (*buf)), crc);
        }

    CommPort->Output(ZDLE);
    CommPort->Output((uchar) frameend);
    crc = Z_32UpdateCRC(frameend, crc);

    crc = ~crc;

    for (length = 4; --length >= 0;)
        {
        ZS_SendByte((byte) crc);
        crc >>= 8;
        }

    CommPort->FlushOutput();
    }

// --------------------------------------------------------------------------
// ZS SEND BYTE
// Send character c with ZMODEM escape sequence encoding.
// Escape XON, XOFF. Escape CR following @ (Telenet net escape)

void TERMWINDOWMEMBER ZS_SendByte(register byte c)
    {
    static byte lastsent;

    switch (c)
        {
        case 015:
        case 0215:
            {
            if ((lastsent & 0x7F) != '@')
                {
                goto SendIt;
                }
            }

        case 020:
        case 021:
        case 023:
        case 0220:
        case 0221:
        case 0223:
        case ZDLE:
            // Quoted characters
            CommPort->Output(ZDLE);
            c ^= 0x40;

        default:
            {
            // Normal character output
SendIt:
            CommPort->Output(lastsent = c);
            }
        }
    }

// -------------------------------------------------------------------------
// ZS GET RECEIVER INFO
// Get the receiver's init parameters

int TERMWINDOWMEMBER ZS_GetReceiverInfo(void)
    {
    int n;

    show_debug_name(73);

    for (n = 10; --n >= 0;)
        {
        switch (Z_GetHeader((byte *) Rxhdr))
            {
            case ZCHALLENGE:
                {
                // Echo receiver's challenge number
                Z_PutLongIntoHeader(Rxpos);
                Z_SendHexHeader(ZACK, (byte *) Txhdr);
                continue;
                }

            case ZCOMMAND:
                {
                // They didn't see our ZRQINIT
                Z_PutLongIntoHeader(0L);
                Z_SendHexHeader(ZRQINIT, (byte *) Txhdr);
                continue;
                }

            case ZRINIT:
                {
                Rxflags = 0377 & Rxhdr[ZF0];
                Rxbuflen = ((word) Rxhdr[ZP1] << 8) | Rxhdr[ZP0];
                Txfcs32 = Rxflags & CANFC32;
                return (OK);
                }

            case ZCAN:
            case RCDO:
            case TIMEOUT:
                {
                return (ZERROR);
                }

            case ZRQINIT:
                {
                if (Rxhdr[ZF0] == ZCOMMAND)
                    {
                    continue;
                    }
                }

            default:
                {
                Z_SendHexHeader(ZNAK, (byte *) Txhdr);
                continue;
                }
            }
        }

    return (ZERROR);
    }

// --------------------------------------------------------------------------
// ZS SEND FILE
// Send ZFILE frame and begin sending ZDATA frame

int TERMWINDOWMEMBER ZS_SendFile(int blen)
    {
    register int c;

    show_debug_name(74);

    for (;;)
        {
        if (got_ESC())
            {
            send_can();
            z_message(getzmmsg(5));
            return (ZERROR);
            }
        else if (!CommPort->HaveConnection())
            {
            return (ZERROR);
            }

        Txhdr[ZF0] = LZCONV;    // Default file conversion mode
        Txhdr[ZF1] = LZMANAG;   // Default file management mode
        Txhdr[ZF2] = LZTRANS;   // Default file transport mode
        Txhdr[ZF3] = 0;
        ZS_SendBinaryHeader(ZFILE, (byte *) Txhdr);
        ZS_SendData((byte *) Txbuf, blen, ZCRCW);

Again:
        switch (c = Z_GetHeader((byte *) Rxhdr))
            {
            case ZRINIT:
                while ((c = Z_GetByte(50)) > 0)
                    {
                    if (c == ZPAD)
                        {
                        goto Again;
                        }
                    }
                // Fall thru

            default:
                continue;

            case ZCAN:
            case RCDO:
            case TIMEOUT:
            case ZFIN:
            case ZABORT:
                {
                return (ZERROR);
                }

            case ZSKIP:
                {
                // Other system wants to skip this file
                return (c);
                }

            case ZRPOS:
                {
                // Resend from this position...
                fseek((FILE *) Infile, Rxpos, SEEK_SET);

                if (Rxpos != 0L)
                    {
                    z_message(getzmmsg(16), Rxpos);
                    CommPort->Output(XON);
                    }

                LastZRpos = Strtpos = Txpos = Rxpos;
                ZRPosCount = 10;
                CommPort->FlushInput();
                return (ZS_SendFileData());
                }
            }
        }
    }

// --------------------------------------------------------------------------
// ZS SEND FILE DATA
// Send the data in the file

int TERMWINDOWMEMBER ZS_SendFileData(void)
    {
    register int c, e;
    int i;
    uint rate;
    int newcnt;
    int blklen;
    int maxblklen;
    int goodblks = 0;

    show_debug_name(75);

    rate = baud_rate;

    maxblklen = (rate < 300) ? 128 : rate / 300 * 256;

    if (maxblklen > BUFSIZE)
        {
        maxblklen = BUFSIZE;
        }

    if (maxblklen > KSIZE)
        {
        maxblklen = KSIZE;
        }

    if (Rxbuflen && maxblklen > (unsigned) Rxbuflen)
        {
        maxblklen = Rxbuflen;
        }

    blklen = (fstblklen != 0) ? fstblklen : maxblklen;
    int goodneeded = (fstblklen != 0) ? 8 : 1;

SomeMore:
    if (CommPort->CheckInputReady() != IR_NONE)
        {
WaitAck:
        switch (c = ZS_SyncWithReceiver(1))
            {
            case ZSKIP:
                {
                // Skip this file
                return (c);
                }

            case ZACK:
                {
                break;
                }

            case ZRPOS:
                {
                // Resume at this position
                blklen = ((blklen >> 2) > 64) ? blklen >> 2 : 64;
                goodblks = 0;
                goodneeded = ((goodneeded << 1) > 16) ? 16 : goodneeded << 1;
                break;
                }

            case ZRINIT:
                {
                // Receive init
                throughput(1, Txpos - Strtpos);
                return (OK);
                }

            case TIMEOUT:
                {
                // Timed out on message from other side
                break;
                }

            default:
                {
                z_message(getzmmsg(17));
                fclose((FILE *) Infile);
                return (ZERROR);
                }
            }

        // Noise probably got us here. Odds of surviving are not good.
        // But we have to get unstuck in any event.

        CommPort->Output(XON);

        while (CommPort->CheckInputReady() != IR_NONE)
            {
            switch (CommPort->Input())
                {
                case CAN:
                case RCDO:
                case ZPAD:
                    {
                    goto WaitAck;
                    }
                }
            }
        }

    newcnt = Rxbuflen;
    Z_PutLongIntoHeader(Txpos);
    ZS_SendBinaryHeader(ZDATA, (byte *) Txhdr);

    do
        {
        if (got_ESC())
            {
            send_can();
            z_message(getzmmsg(5));
            goto oops;
            }

        if (!CommPort->HaveConnection())
            {
            goto oops;
            }

        if ((unsigned) (c = fread((byte *) Txbuf, 1, blklen, (FILE *) Infile)) != z_size)
            {
            z_size = c;
            }

        if ((unsigned) c < blklen)
            {
            e = ZCRCE;
            }
        else if (Rxbuflen && (newcnt -= c) <= 0)
            {
            e = ZCRCW;
            }
        else
            {
            e = ZCRCG;
            }

        ZS_SendData((byte *) Txbuf, c, e);

        i = (int) ((file_length - Txpos) * 10 / rate + 53) / 54;

        z_status(getzmmsg(18), Txpos, i);

        Txpos += c;

        if (blklen < maxblklen && ++goodblks > goodneeded)
            {
            blklen = ((blklen << 1) < maxblklen) ? blklen << 1 : maxblklen;
            goodblks = 0;
            }

        if (e == ZCRCW)
            {
            goto WaitAck;
            }

        while (CommPort->CheckInputReady() != IR_NONE)
            {
            switch (CommPort->Input())
                {
                case CAN:
                case RCDO:
                case ZPAD:
                    {
                    // Interruption detected
                    // stop sending and process complaint
                    z_message(getzmmsg(19));
                    ZS_SendData((byte *) Txbuf, 0, ZCRCE);
                    goto WaitAck;
                    }
                }
            }
        } while (e == ZCRCG);

    for (;;)
        {
        Z_PutLongIntoHeader(Txpos);
        ZS_SendBinaryHeader(ZEOF, (byte *) Txhdr);

        switch (ZS_SyncWithReceiver(7))
            {
            case ZACK:
                {
                continue;
                }

            case ZRPOS:
                {
                // Resume at this position...
                goto SomeMore;
                }

            case ZRINIT:
                {
                // Receive init
                throughput(1, Txpos - Strtpos);
                return (OK);
                }

            case ZSKIP:
                {
                // Request to skip the current file
                z_message(getzmmsg(20));
                errno = 0;
                fclose((FILE *) Infile);
                return (c);
                }

            default:
                {
oops:
                z_message(getzmmsg(17));
                fclose((FILE *) Infile);
                return (ZERROR);
                }
            }
        }
    }

// --------------------------------------------------------------------------
// ZS SYNC WITH RECEIVER
// Respond to receiver's complaint, get back in sync with receiver

int TERMWINDOWMEMBER ZS_SyncWithReceiver(int num_errs)
    {
    register int c;
    char j[50];

    show_debug_name(76);

    for (;;)
        {
        c = Z_GetHeader((byte *) Rxhdr);
        CommPort->FlushInput();

        switch (c)
            {
            case TIMEOUT:
                z_message(getzmmsg(21));
                if ((num_errs--) >= 0)
                    {
                    break;
                    }

            case ZCAN:
            case ZABORT:
            case ZFIN:
            case RCDO:
                {
                z_message(getzmmsg(22));
                return (ZERROR);
                }

            case ZRPOS:
                {
                if (Rxpos == LastZRpos)
                    {   // Same as last time?
                    if (!(--ZRPosCount))    // Yup, 10 times yet?
                        {
                        return (ZERROR);        // Too many, get out
                        }
                    }
                else
                    {
                    ZRPosCount = 10;        // Reset repeat count
                    }

                LastZRpos = Rxpos;          // Keep track of this

                rewind((FILE *) Infile); // In case file EOF seen
                fseek((FILE *) Infile, Rxpos, SEEK_SET);
                Txpos = Rxpos;
                sprintf(j, getzmmsg(23),
#ifdef MINGW
                        _ultoa(((ulong) (Txpos)), (char *) e_input, 10));
#else
                        ultoa(((ulong) (Txpos)), (char *) e_input, 10));
#endif
                z_message(j);
                return (c);
                }

            case ZSKIP:
                z_message(getzmmsg(20));

            case ZRINIT:
                {
                errno = 0;
                fclose((FILE *) Infile);
                return (c);
                }

            case ZACK:
                {
                z_message(NULL);
                return (c);
                }

            default:
                {
                z_message(getzmmsg(46));
                ZS_SendBinaryHeader(ZNAK, (byte *) Txhdr);
                continue;
                }
            }
        }
    }


// --------------------------------------------------------------------------
// ZS END SEND
// Say BIBI to the receiver, try to do it cleanly

void TERMWINDOWMEMBER ZS_EndSend(void)
    {
    show_debug_name(77);

    for (;;)
        {
        Z_PutLongIntoHeader(0L);
        ZS_SendBinaryHeader(ZFIN, (byte *) Txhdr);

        switch (Z_GetHeader((byte *) Rxhdr))
            {
            case ZFIN:
                CommPort->Output('O');
                CommPort->Output('O');
                // fallthrough...

            case ZCAN:
            case RCDO:
            case TIMEOUT:
                {
                return;
                }
            }
        }
    }

// --------------------------------------------------------------------------
// GET ZMODEM
// Receive a batch of files.
// returns TRUE (1) for good xfer, FALSE (0) for bad
// can be called from f_upload or to get mail from a WaZOO Opus

int TERMWINDOWMEMBER get_Zmodem(const char *rcvpath, FILE * xferinfo)
    {
    char namebuf[PATHLEN];
    int i;
    char *p;

    show_debug_name(78);

    filetime = 0;

    // Secbuf = NULL;
    Outfile = NULL;
    z_size = 0;

    Rxtimeout = 100;
    Tryzhdrtype = ZRINIT;

    strcpy(namebuf, rcvpath);
    Filename = (long) namebuf;

    strcpy(Upload_path, rcvpath);
    p = Upload_path + strlen(Upload_path) - 1;

    while (p >= Upload_path && *p != '\\')
        {
        --p;
        }

    *(++p) = '\0';

    DiskAvail = zfree(Upload_path);

    if (((i = RZ_InitReceiver()) == ZCOMPL) ||
            ((i == ZFILE) && ((RZ_ReceiveBatch(xferinfo)) == OK)))
        {
        return (1);
        }

    send_can();

    if (Outfile)
        {
        fclose((FILE *) Outfile);
        }

    return (0);
    }


// --------------------------------------------------------------------------
// RZ RECEIVE DATA
// Receive array buf of max length with ending ZDLE sequence
// and CRC. Returns the ending character or error code.

int TERMWINDOWMEMBER RZ_ReceiveData(register byte * buf, register int length)
    {
    register int c;
    register ushort crc;
    char *endpos;
    int d;

    show_debug_name(79);

    if (Rxframeind == ZBIN32)
        {
        return (RZ_32ReceiveData(buf, length));
        }

    RxCount = crc = 0;
    buf[0] = buf[1] = 0;
    endpos = (char *) buf + length;

    while ((char *) buf <= endpos)
        {
        if ((c = Z_GetZDL()) & ~0xFF)
            {
CRCfoo:
            switch (c)
                {
                case GOTCRCE:
                case GOTCRCG:
                case GOTCRCQ:
                case GOTCRCW:
                    {
                    // C R C s
                    crc = Z_UpdateCRC(((d = c) & 0xFF), crc);

                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_UpdateCRC(c, crc);
                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_UpdateCRC(c, crc);
                    if (crc & 0xFFFF)
                        {
                        z_message(getzmmsg(7));
                        return (ZERROR);
                        }

                    RxCount = length - (int) (endpos - (char *) buf);
                    return (d);
                    }

                case GOTCAN:
                    {
                    // Cancel
                    z_message(getzmmsg(17));
                    return (ZCAN);
                    }

                case TIMEOUT:
                    {
                    // Timeout
                    z_message(getzmmsg(21));
                    return (c);
                    }

                case RCDO:
                    {
                    // No carrier
                    z_message(getzmmsg(24));
                    CommPort->FlushInput();
                    return (c);
                    }

                default:
                    {
                    // Something bizarre
                    z_message(getzmmsg(25));
                    CommPort->FlushInput();
                    return (c);
                    }
                }
            }

        *buf++ = (uchar) c;
        crc = Z_UpdateCRC(c, crc);
        }

    z_message(getzmmsg(26));
    return (ZERROR);
    }


// --------------------------------------------------------------------------
// RZ RECEIVE DATA with 32 bit CRC
// Receive array buf of max length with ending ZDLE sequence
// and CRC. Returns the ending character or error code.

int TERMWINDOWMEMBER RZ_32ReceiveData(register byte * buf, register int length)
    {
    register int c;
    ulong crc;
    char *endpos;
    int d;

    show_debug_name(80);

    crc = 0xFFFFFFFFl;
    RxCount = 0;
    buf[0] = buf[1] = 0;
    endpos = (char *) buf + length;

    while ((char *) buf <= endpos)
        {
        if ((c = Z_GetZDL()) & ~0xFF)
            {
CRCfoo:
            switch (c)
                {
                case GOTCRCE:
                case GOTCRCG:
                case GOTCRCQ:
                case GOTCRCW:
                    {
                    // C R C s
                    d = c;
                    c &= 0377;

                    crc = Z_32UpdateCRC(c, crc);
                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_32UpdateCRC(c, crc);
                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_32UpdateCRC(c, crc);
                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_32UpdateCRC(c, crc);
                    if ((c = Z_GetZDL()) & ~0xFF)
                        {
                        goto CRCfoo;
                        }

                    crc = Z_32UpdateCRC(c, crc);
                    if (crc != 0xDEBB20E3l)
                        {
                        z_message(getzmmsg(7));
                        return (ZERROR);
                        }

                    RxCount = length - (int) (endpos - (char *) buf);
                    return (d);
                    }

                case GOTCAN:
                    {
                    // Cancel
                    z_message(getzmmsg(17));
                    return (ZCAN);
                    }

                case TIMEOUT:
                    {
                    // Timeout
                    z_message(getzmmsg(21));
                    return (c);
                    }

                case RCDO:
                    {
                    // No carrier
                    z_message(getzmmsg(24));
                    CommPort->FlushInput();
                    return (c);
                    }

                default:
                    {
                    // Something bizarre
                    z_message(getzmmsg(25));
                    CommPort->FlushInput();
                    return (c);
                    }
                }
            }
        *buf++ = (uchar) c;
        crc = Z_32UpdateCRC(c, crc);
        }

    z_message(getzmmsg(26));
    return (ZERROR);
    }


// --------------------------------------------------------------------------
// RZ INIT RECEIVER
// Initialize for Zmodem receive attempt, try to activate Zmodem sender
// Handles ZSINIT, ZFREECNT, and ZCOMMAND frames
//
// Return codes
//  ZFILE .... Zmodem filename received
//  ZCOMPL ... transaction finished
//  ZERROR .... any other condition

int TERMWINDOWMEMBER RZ_InitReceiver(void)
    {
    register int n;
    int errors = 0;
    const char *sptr;


    show_debug_name(81);

    for (n = 12; --n >= 0;)
        {
        // Set buffer length (0=unlimited, don't wait).
        // Also set capability flags

        Z_PutLongIntoHeader(0L);
        Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
        Z_SendHexHeader(Tryzhdrtype, (byte *) Txhdr);

        if (Tryzhdrtype == ZSKIP)
            {
            Tryzhdrtype = ZRINIT;
            }

AGAIN:

        switch (Z_GetHeader((byte *) Rxhdr))
            {
            case ZFILE:
                {
                Zconv = Rxhdr[ZF0];
                Tryzhdrtype = ZRINIT;
                if (RZ_ReceiveData((byte *) Txbuf, BUFSIZE) == GOTCRCW)
                    {
                    return (ZFILE);
                    }

                Z_SendHexHeader(ZNAK, (byte *) Txhdr);
                if (--n < 0)
                    {
                    sptr = getzmmsg(27);
                    goto Err;
                    }

                goto AGAIN;
                }

            case ZSINIT:
                {
                if (RZ_ReceiveData((byte *) Attn, ZATTNLEN) == GOTCRCW)
                    {
                    Z_PutLongIntoHeader(1L);
                    Z_SendHexHeader(ZACK, (byte *) Txhdr);
                    }
                else
                    {
                    Z_SendHexHeader(ZNAK, (byte *) Txhdr);
                    }

                if (--n < 0)
                    {
                    sptr = getzmmsg(28);
                    goto Err;
                    }

                goto AGAIN;
                }

            case ZFREECNT:
                {
                Z_PutLongIntoHeader(DiskAvail);
                Z_SendHexHeader(ZACK, (byte *) Txhdr);
                goto AGAIN;
                }

            case ZCOMMAND:
                {
                // Paranoia is good for you...
                // Ignore command from remote, but lie and
                // say we did the command ok.

                if (RZ_ReceiveData((byte *) Txbuf, BUFSIZE) == GOTCRCW)
                    {
                    z_message(getzmmsg(29), (byte *) Txbuf);
                    Z_PutLongIntoHeader(0L);

                    do
                        {
                        Z_SendHexHeader(ZCOMPL, (byte *) Txhdr);
                        } while (++errors < 10 && Z_GetHeader((byte *) Rxhdr) != ZFIN);

                    RZ_AckBibi();
                    return (ZCOMPL);
                    }
                else
                    {
                    Z_SendHexHeader(ZNAK, (byte *) Txhdr);
                    }

                if (--n < 0)
                    {
                    sptr = getzmmsg(30);
                    goto Err;
                    }

                goto AGAIN;
                }

            case ZCOMPL:
                {
                if (--n < 0)
                    {
                    sptr = getzmmsg(31);
                    goto Err;
                    }

                goto AGAIN;
                }

            case ZFIN:
                {
                RZ_AckBibi();
                return (ZCOMPL);
                }

            case ZCAN:
                {
                sptr = getzmmsg(17);
                goto Err;
                }

            case RCDO:
                {
                sptr = getzmmsg(24);
                CommPort->FlushInput();
                goto Err;
                }
            }
        }

    sptr = getzmmsg(21);

Err:
    sprintf((char *) e_input, getzmmsg(32), sptr);
    z_message((char *) e_input);

    return (ZERROR);
    }

// --------------------------------------------------------------------------
// RZFILES
// Receive a batch of files using ZMODEM protocol

int TERMWINDOWMEMBER RZ_ReceiveBatch(FILE * xferinfo)
    {
    register int c;

    show_debug_name(82);

    for (;;)
        {
        switch (c = RZ_ReceiveFile(xferinfo))
            {
            case ZEOF:
            case ZSKIP:
                {
                switch (RZ_InitReceiver())
                    {
                    case ZCOMPL:
                        {
                        return (OK);
                        }

                    default:
                        {
                        return (ZERROR);
                        }

                    case ZFILE:
                        {
                        break;
                        }
                    }

                break;
                }

            default:
                {
                fclose((FILE *) Outfile);
                Outfile = NULL;
                unlink((char *) Filename);
                return (c);
                }
            }
        }
    }

// --------------------------------------------------------------------------
// RZ RECEIVE FILE
//  Receive one file; assumes file name frame is preloaded in Txbuf

int TERMWINDOWMEMBER RZ_ReceiveFile(FILE * xferinfo)
    {
    register int c;
    int n;
    long rxbytes;
    const char *sptr;
    struct utimbuf utimes;
    char j[50];


    show_debug_name(83);

    EOFseen = FALSE;
    c = RZ_GetHeader();

    if (c == ZERROR || c == ZSKIP)
        {
        return (Tryzhdrtype = ZSKIP);
        }

    n = 10;
    rxbytes = Filestart;

    for (;;)
        {
        Z_PutLongIntoHeader(rxbytes);
        Z_SendHexHeader(ZRPOS, (byte *) Txhdr);

NxtHdr:
        switch (c = Z_GetHeader((byte *) Rxhdr))
            {
            case ZDATA:
                {
                // Data Packet

                if (Rxpos != rxbytes)
                    {
                    if (--n < 0)
                        {
                        sptr = getzmmsg(6);
                        goto Err;
                        }

                    sprintf(j, getzmmsg(33), getzmmsg(34), rxbytes, Rxpos);
                    z_message(j);
                    Z_PutString((byte *) Attn);
                    continue;
                    }

MoreData:
                switch (c = RZ_ReceiveData((byte *) Txbuf, BUFSIZE))
                    {
                    case ZCAN:
                        {
                        sptr = getzmmsg(17);
                        goto Err;
                        }

                    case RCDO:
                        {
                        sptr = getzmmsg(24);
                        CommPort->FlushInput();
                        goto Err;
                        }

                    case ZERROR:
                        {
                        // CRC error
                        if (--n < 0)
                            {
                            sptr = getzmmsg(6);
                            goto Err;
                            }

                        show_loc(rxbytes, n);
                        Z_PutString((byte *) Attn);
                        continue;
                        }

                    case TIMEOUT:
                        {
                        if (--n < 0)
                            {
                            sptr = getzmmsg(21);
                            goto Err;
                            }

                        show_loc(rxbytes, n);
                        continue;
                        }

                    case GOTCRCW:
                        {
                        // End of frame

                        n = 10;
                        if (RZ_SaveToDisk(&rxbytes) == ZERROR)
                            {
                            return (ZERROR);
                            }

                        Z_PutLongIntoHeader(rxbytes);
                        Z_SendHexHeader(ZACK, (byte *) Txhdr);
                        goto NxtHdr;
                        }

                    case GOTCRCQ:
                        {
                        // Zack expected

                        n = 10;
                        if (RZ_SaveToDisk(&rxbytes) == ZERROR)
                            {
                            return (ZERROR);
                            }

                        Z_PutLongIntoHeader(rxbytes);
                        Z_SendHexHeader(ZACK, (byte *) Txhdr);
                        goto MoreData;
                        }

                    case GOTCRCG:
                        {
                        // Non-stop

                        n = 10;
                        if (RZ_SaveToDisk(&rxbytes) == ZERROR)
                            {
                            return (ZERROR);
                            }

                        goto MoreData;
                        }

                    case GOTCRCE:
                        {
                        // Header to follow

                        n = 10;
                        if (RZ_SaveToDisk(&rxbytes) == ZERROR)
                            {
                            return (ZERROR);
                            }

                        goto NxtHdr;
                        }
                    }
                }

            case ZNAK:
            case TIMEOUT:
                {
                // Packet was probably garbled
                if (--n < 0)
                    {
                    sptr = getzmmsg(35);
                    goto Err;
                    }

                show_loc(rxbytes, n);
                continue;
                }

            case ZFILE:
                {
                // Sender didn't see our ZRPOS yet
                RZ_ReceiveData((byte *) Txbuf, BUFSIZE);
                continue;
                }

            case ZEOF:
                {
                // End of the file
                // Ignore EOF if it's at wrong place; force
                // a timeout because the eof might have
                // gone out before we sent our ZRPOS
                if (Rxpos != rxbytes)
                    {
                    goto NxtHdr;
                    }

                throughput(2, rxbytes - Filestart);

                fclose((FILE *) Outfile);

                z_message(getzmmsg(12), getzmmsg(36), Crc32 ? getzmmsg(14) : ns, realname);
                doccr();

                if (filetime)
                    {
                    utimes.modtime = filetime;
#ifdef VISUALC
                    utimes.actime = filetime;
                    utime( (char *) Filename, &utimes);
#else
                    utimes.UT_ACTIME = filetime;
                    b_utime((char *) Filename, (UTIMBUF *) & utimes);
#endif

                    }

                Outfile = NULL;
                if (xferinfo != NULL)
                    {
                    fprintf(xferinfo, getmsg(472), (char *) Filename, bn);
                    }
                return (c);
                }

            case ZERROR:
                {
                // Too much garbage in header search error
                if (--n < 0)
                    {
                    sptr = getzmmsg(35);
                    goto Err;
                    }

                show_loc(rxbytes, n);
                Z_PutString((byte *) Attn);
                continue;
                }

            case ZSKIP:
                {
                return (c);
                }

            default:
                {
                sptr = getzmmsg(46);
                CommPort->FlushInput();
                goto Err;
                }
            }
        }

Err:
    sprintf((char *) e_input, getzmmsg(37), sptr);
    z_message((char *) e_input);
    return (ZERROR);
    }

// --------------------------------------------------------------------------
// RZ GET HEADER
// Process incoming file information header

int TERMWINDOWMEMBER RZ_GetHeader(void)
    {
    register byte *p;
    struct stat f;
    char *ourname;
    char *theirname;
    long filesize;
    char *fileinfo;
    char j[200];

    show_debug_name(84);

    // Setup the transfer mode
    isBinary = (char) ((!RXBINARY && Zconv == ZCNL) ? 0 : 1);

    // Extract and verify filesize, if given.
    // Reject file if not at least 10K free
    filesize = 0L;
    filetime = 0L;
    fileinfo = (char *) Txbuf + 1 + strlen((char *) Txbuf);

    if (*fileinfo)
        {
        sscanf(fileinfo, getzmmsg(38), &filesize, &filetime);
        }

    if (filesize + 10240 > DiskAvail)
        {
        z_message(getzmmsg(39));
        return (ZERROR);
        }

    // Get and/or fix filename for uploaded file
    p = (byte *) ((char *) Filename + strlen((char *) Filename) - 1); // Find end of upload path
    while ((char *) p >= (char *) Filename && *p != '\\')
        {
        p--;
        }

    ourname = (char *) ++p;

    p = (byte *) Txbuf + strlen((char *) Txbuf) - 1; // Find transmitted simple filename
    while (p >= (byte *) Txbuf && *p != '\\' && *p != '/' && *p != ':')
        {
        p--;
        }
    theirname = (char *) ++p;

    // Limit filenames to 64 chars.
    if (strlen(theirname) > 64) 
        {
        theirname[64] = 0;
        }

    strcpy(ourname, theirname); // Start w/ our path & their name
    strcpy(realname, (char *) Filename);

    // Open either the old or a new file, as appropriate

    strcpy(ourname, theirname);

    // If the file already exists
    // 1) And the new file has the same time and size, return ZSKIP
    // 2) And OVERWRITE is turned on, delete the old copy
    // 3) Else create a unique file name in which to store new data

    if (filexists((char *) Filename)) // If file already exists...
        {
        if ((Outfile = (long) fopen((char *) Filename, FO_RB)) == NULL)
            {
            got_error(getzmmsg(8), (char *) Filename);
            return (ZERROR);
            }

        fstat(fileno((FILE *) Outfile), &f);
        fclose((FILE *) Outfile);
        if (filesize == f.st_size && filetime == f.st_mtime)
            {
            z_message(getzmmsg(40), (char *) Filename);
            return (ZSKIP);
            }

        if (!overwrite)
            {
            unique_name((char *) Filename);
            }
        else
            {
            unlink((char *) Filename);
            }
        }

    if (strcmp(ourname, theirname))
        {
        z_message(getzmmsg(41), ourname);
        }

    p = (byte *) FO_WB;
    if ((Outfile = (long) fopen((char *) Filename, (char *) p)) == NULL)
        {
        got_error(getzmmsg(8), (char *) Filename);
        return (ZERROR);
        }

    if (isatty(fileno((FILE *) Outfile)))
        {
        errno = 1;
        got_error(getzmmsg(9), (char *) Filename);
        fclose((FILE *) Outfile);
        return (ZERROR);
        }

    Filestart = 0L;

    fseek((FILE *) Outfile, Filestart, SEEK_SET);

    p = NULL;

    sprintf(j, getzmmsg(42),
            (p != NULL) ? (char *) p : getzmmsg(43),
            realname, (isBinary) ? ns : getzmmsg(44), filesize,
            (int) ((filesize - Filestart) * 10 / baud_rate + 53) / 54);

    file_length = filesize;

    z_message(pcts, j);

    throughput(0, 0L);

    return (OK);
    }

// --------------------------------------------------------------------------
// RZ SAVE TO DISK
// Writes the received file data to the output file.
// If in ASCII mode, stops writing at first ^Z, and converts all
//  solo CR's or LF's to CR/LF pairs.

int TERMWINDOWMEMBER RZ_SaveToDisk(long *rxbytes)
    {
    static byte lastsent;

    register byte *p;
    register uint count;
    int i;

    show_debug_name(85);

    count = RxCount;

    if (got_ESC())
        {
        send_can();
        // Wait for line to clear
        while ((i = Z_GetByte(20)) != TIMEOUT && i != RCDO)
            {
            CommPort->FlushInput();
            }

        send_can();
        z_message(getzmmsg(5));
        return (ZERROR);
        }

    if (count != z_size)
        {
        z_size = count;
        }

    if (isBinary)
        {
        if (fwrite((byte *) Txbuf, 1, count, (FILE *) Outfile) != count)
            {
            goto oops;
            }
        }
    else
        {
        if (EOFseen)
            {
            return (OK);
            }

        for (p = (byte *) Txbuf; count > 0; count--)
            {
            if (*p == CPMEOF)
                {
                EOFseen = TRUE;
                return (OK);
                }

            if (*p == '\n')
                {
                if (lastsent != '\r' && putc('\r', (FILE *) Outfile) == EOF)
                    {
                    goto oops;
                    }
                }
            else
                {
                if (lastsent == '\r' && putc('\n', (FILE *) Outfile) == EOF)
                    {
                    goto oops;
                    }
                }

            if (putc((lastsent = *p++), (FILE *) Outfile) == EOF)
                {
                goto oops;
                }
            }
        }

    *rxbytes += RxCount;
    i = (int) ((file_length - *rxbytes) * 10 / baud_rate + 53) / 54;

    z_status(getzmmsg(18), *rxbytes, i);

    return (OK);

oops:
    got_error(getzmmsg(45), (char *) Filename);
    return (ZERROR);
    }

// --------------------------------------------------------------------------
// RZ ACK BIBI
// Ack a ZFIN packet, let byegones be byegones

void TERMWINDOWMEMBER RZ_AckBibi(void)
    {
    register int n;

    show_debug_name(86);

    Z_PutLongIntoHeader(0L);

    for (n = 4; --n;)
        {
        Z_SendHexHeader(ZFIN, (byte *) Txhdr);
        switch (Z_GetByte(100))
            {
            case 'O':
                Z_GetByte(1);   // Discard 2nd 'O'

            case TIMEOUT:
            case RCDO:
                {
                return;
                }
            }
        }
    }
