// --------------------------------------------------------------------------
// Citadel: Zmodem.H
//
// Include file for Citadel's ZModem


#include "ctdl.h"
#pragma hdrstop

#include "pragmas.h"
#include "extmsg.h"
#include "filefind.h"

#ifdef VISUALC
#include <sys/utime.h>
#endif

#define Z_UpdateCRC(cp,crc) ((ushort)(((ushort *) crctab)[((crc >> 8) & 255) ^ cp] ^ (crc << 8)))
#define Z_32UpdateCRC(c,crc) (((ulong *) cr3tab)[((short) crc ^ c) & 0xff] ^ ((crc >> 8) & 0x00FFFFFFl))


// Send a byte as two hex digits
#define Z_PUTHEX(i,c)									\
	{													\
	i = (c);											\
	CommPort->Output(getzmmsg(47)[((i) & 0xF0) >> 4]);	\
	CommPort->Output(getzmmsg(47)[(i) & 0xF]);			\
	}



// --------------------------------------------------------------------------
// Routines from N_Zmodem.H...

#define ZPAD		'*'
#define ZDLE		030
#define ZDLEE		(ZDLE^0100)
#define ZBIN		'A'
#define ZHEX		'B'
#define ZBIN32		'C'


// --------------------------------------------------------------------------
// Frame types (see array "frametypes" in zm.c)

#define ZRQINIT		0
#define ZRINIT		1
#define ZSINIT		2
#define ZACK		3
#define ZFILE		4
#define ZSKIP		5
#define ZNAK		6
#define ZABORT		7
#define ZFIN		8
#define ZRPOS		9
#define ZDATA		10
#define ZEOF		11
#define ZFERR		12
#define ZCRC		13
#define ZCHALLENGE	14
#define ZCOMPL		15
#define ZCAN		16
#define ZFREECNT	17
#define ZCOMMAND	18
#define ZSTDERR		19


// --------------------------------------------------------------------------
// ZDLE sequences

#define ZCRCE		'h'
#define ZCRCG		'i'
#define ZCRCQ		'j'
#define ZCRCW		'k'
#define ZRUB0		'l'
#define ZRUB1		'm'


// --------------------------------------------------------------------------
// Z_GetZDL return values (internal)
//	-1 is general error, -2 is timeout

#define GOTOR		0400
#define GOTCRCE		(ZCRCE|GOTOR)
#define GOTCRCG		(ZCRCG|GOTOR)
#define GOTCRCQ		(ZCRCQ|GOTOR)
#define GOTCRCW		(ZCRCW|GOTOR)
#define GOTCAN		(GOTOR|030)


// --------------------------------------------------------------------------
// Byte positions within header array

#define ZF0			3
#define ZF1			2
#define ZF2			1
#define ZF3			0
#define ZP0			0
#define ZP1			1
#define ZP2			2
#define ZP3			3


// --------------------------------------------------------------------------
// Bit Masks for ZRINIT flags byte ZF0

#define CANFDX		01
#define CANOVIO		02
#define CANBRK		04
#define CANCRY		010
#define CANLZW		020
#define CANFC32		040


// --------------------------------------------------------------------------
// PARAMETERS FOR ZFILE FRAME...


// --------------------------------------------------------------------------
// Conversion options one of these in ZF0

#define ZCBIN		1
#define ZCNL		2
#define ZCRESUM		3


// --------------------------------------------------------------------------
// Management options, one of these in ZF1

#define ZMNEW		1
#define ZMCRC		2
#define ZMAPND		3
#define ZMCLOB		4
#define ZMSPARS		5
#define ZMDIFF		6
#define ZMPROT		7


// --------------------------------------------------------------------------
// Transport options, one of these in ZF2

#define ZTLZW		1
#define ZTCRYPT		2
#define ZTRLE		3


// --------------------------------------------------------------------------
// Parameters for ZCOMMAND frame ZF0 (otherwise 0)

#define ZCACK1		1


// --------------------------------------------------------------------------
// Miscellaneous definitions

#define OK			0
#define ZERROR		(-1)
#define TIMEOUT		(-2)
#define RCDO		(-3)
#define FUBAR		(-4)

#define XON			('Q'&037)
#define XOFF		('S'&037)
#define CPMEOF		('Z'&037)

#define RXBINARY	FALSE
#define RXASCII		FALSE
#define LZCONV		0
#define LZMANAG		0
#define LZTRANS		0
#define PATHLEN		128
#define KSIZE		1024
#define BUFSIZE		8192
#define SECSPERDAY	(24L*60L*60L)


// --------------------------------------------------------------------------
// Parameters for calling ZMODEM routines

#define SPEC_COND		2
#define END_BATCH		(-1)
#define NOTHING_TO_DO	(-2)
#define DELETE_AFTER	'-'
#define TRUNC_AFTER		'#'
#define NOTHING_AFTER	'@'
#define END_BATCH		(-1)
#define SHOW_DELETE_AFTER	'^'


// --------------------------------------------------------------------------
// ASCII MNEMONICS

#define NUL	0x00
#define SOH	0x01
#define STX	0x02
#define ETX	0x03
#define EOT	0x04
#define ENQ	0x05
#define ACK	0x06
#define BEL	0x07
#define BS	0x08
#define HT	0x09
//#define LF	0x0a
#define VT	0x0b
#define FF	0x0c
//#define CR	0x0d
#define SO	0x0e
#define SI	0x0f
#define DLE	0x10
#define DC1	0x11
#define DC2	0x12
#define DC3	0x13
#define DC4	0x14
#define NAK	0x15
#define SYN	0x16
#define ETB	0x17
#define CAN	0x18
#define EM	0x19
#define SUB	0x1a
//#define ESC	0x1b
#define FS	0x1c
#define GS	0x1d
#define RS	0x1e
#define US	0x1f


// --------------------------------------------------------------------------
// Parameters for calling ZMODEM routines
#ifndef VISUALC
struct utimbuf
	{
	time_t	actime;		// access time
	time_t	modtime;	// modification time
	};

typedef struct utimbuf UTIMBUF;
#define UT_ACTIME actime
#endif
//typedef long off_t;

typedef uint bit;
typedef ushort word;
typedef uchar byte;

// This union holds a long integer as a long, 2 ints or 4 chars
typedef union
	{
	long l;

	struct
		{
		uchar c[4];
		} c;

	struct
		{
		ushort i[2];
		} i;
	} TIMETYPE;

#define PER_WEEK	60480000L
#define PER_DAY		8640000L
#define PER_HOUR	360000L
#define PER_MINUTE	6000L
#define PER_SECOND	100L


// --------------------------------------------------------------------------
// Global variables

int THREAD fstblklen = 0;	// Start Zmodem packet size
int THREAD overwrite = 0;
uint THREAD baud_rate;
long THREAD Txbuf;			// Pointer to xmit buffer
long THREAD e_input;
long THREAD file_length = 0L;
char THREAD Rxhdr[4];		// Received header
char THREAD Txhdr[4];		// Transmitted header
long THREAD Rxpos;			// Received file position
int THREAD Txfcs32;			// TRUE means send binary frames with 32 bit FCS
int THREAD Crc32t;			// Display flag indicating 32 bit CRC being sent
int THREAD Crc32;			// Display flag indicating 32 bit CRC being	received
int THREAD Rxtimeout;		// Tenths of seconds to wait for something
int THREAD Rxframeind;		// ZBIN ZBIN32,ZHEX type of frame received
long THREAD Filename;		// Name of the file being up/downloaded
int THREAD z_size = 0;

// --------------------------------------------------------------------------
// First, the polynomial itself and its table of feedback terms.  The
// polynomial is:
//
// X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
//
// Note that we take it "backwards" and put the highest-order term in the
// lowest-order bit.  The X^32 term is "implied"; the LSB is the X^31 term,
// etc.  The X^0 term (usually shown as "+1") results in the MSB being 1.
//
// Note that the usual hardware shift register implementation, which is what
// we're using (we're merely optimizing it by doing eight-bit chunks at a
// time) shifts bits into the lowest-order term.  In our implementation, that
// means shifting towards the right.  Why do we do it this way?  Because the
// calculated CRC must be transmitted in order from highest-order term to
// lowest-order term.  UARTs transmit characters in order from LSB to MSB.
// By storing the CRC this way, we hand it to the UART in the order low-byte
// to high-byte; the UART sends each low-bit to hight-bit; and the result is
// transmission bit by bit from highest- to lowest-order term without
// requiring any bit shuffling on our part.  Reception works similarly.
//
// The feedback terms table consists of 256, 32-bit entries.  Notes:
//
// The table can be generated at runtime if desired; code to do so is shown
// later.  It might not be obvious, but the feedback terms simply represent
// the results of eight shift/xor operations for all combinations of data and
// CRC register values.
//
// The values must be right-shifted by eight bits by the "updcrc" logic; the
// shift must be unsigned (bring in zeroes).  On some hardware you could
// probably optimize the shift in assembler by using byte-swap instructions.

long THREAD cr3tab;				// zmmsgs->next->data

// -rev 04-16-87  (abbreviated) The CRC-16 routines used by XMODEM, YMODEM,
// and ZMODEM are also in this file, a fast table-driven macro version

// crctab calculated by Mark G. Mendel, Network Systems Corporation
long THREAD crctab;// zmmsgs->next->next->data

long THREAD Infile;			// Handle of file being sent
long THREAD Strtpos;			// Starting byte position of download
long THREAD LastZRpos;			// Last error location
long THREAD ZRPosCount;			// ZRPOS repeat count
long THREAD Txpos;				// Transmitted file position
int THREAD Rxbuflen;			// Receiver's max buffer length
int THREAD Rxflags;				// Receiver's flags
long THREAD DiskAvail;
long THREAD filetime;
char THREAD realname[64];

// Parameters for ZSINIT frame
#define ZATTNLEN 32
char THREAD Attn[ZATTNLEN + 1];	// String rx sends to tx on err
long THREAD Outfile;			// Handle of file being received
int THREAD Tryzhdrtype;			// Hdr type to send for Last rx close
char THREAD isBinary;			// Current file is binary mode
char THREAD EOFseen;			// indicates cpm eof (^Z) was received
char THREAD Zconv;				// ZMODEM file conversion request
int THREAD RxCount;				// Count of data bytes received
char THREAD Upload_path[PATHLEN];//Dest. path of file being received
long THREAD Filestart;			// File offset we started this xfer from
int THREAD Rxtype;				// Type of header received
