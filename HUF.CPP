// --------------------------------------------------------------------------
// Citadel: Huf.CPP
//
// This is the huffman encoder/decoder.

#include "ctdl.h"
#pragma hdrstop

#include "miscovl.h"

// --------------------------------------------------------------------------
// for building a huffman tree from the data in CTDLDATA.DEF
//
// NOTE: the offsets for these are in hufbit.asm - if you change this struct,
// you must change hufbit.asm if you don't want Citadel to crash when you
// read a compressed message.
struct hufTree
    {
    uchar   theChar;
    char    PAD;
    hufTree *right;
    hufTree *left;
    };


// --------------------------------------------------------------------------
// this is in hufbit.asm as well. Also, if you need more than 8192, you will
// have to change "bitsenc" to handle more than 65535.
#define ENCBUFSIZE 8192


//int THREAD hufOrig, hufNew;
uint cdecl THREAD bitsenc;

long cdecl THREAD htRoot;
long THREAD htData;

static int THREAD dataOffsets[255]; // 0 to 254

#if VERSION != RELEASE
void TERMWINDOWMEMBER CheckTreePointer(const void *Check, hufTree *cur)
    {
    if (cur)
        {
        if (cur == Check)
            {
            mPrintf(" (Huffman tree)");
            return;
            }

        if (cur->left)
            {
            CheckTreePointer(Check, cur->left);
            }

        if (cur->right)
            {
            CheckTreePointer(Check, cur->right);
            }
        }
    }

void TERMWINDOWMEMBER CheckHufPointer(const void *Check)
    {
#ifndef VISUALC
    CheckDiscardablePtr((discardable *) htData, Check, "htData");
#endif
    CheckTreePointer(Check, (hufTree *) htRoot);
    }
#endif


// --------------------------------------------------------------------------
// With hufInit, used to create a tree from the data in huf.h.

static hufTree *followOrMakeTree(hufTree *start, int whichWay)
    {
    if (whichWay)
        {
        // go right
        if (!start->right)
            {
            start->right = new hufTree;

            if (start->right)
                {
                start->right->left = NULL;
                start->right->right = NULL;
                }
            }

        return (start->right);
        }
    else
        {
        // go left
        if (!start->left)
            {
            start->left = new hufTree;

            if (start->left)
                {
                start->left->left = NULL;
                start->left->right = NULL;
                }
            }

        return (start->left);
        }
    }


// --------------------------------------------------------------------------
// Builds tree.

static Bool initHuf(void)
    {
    int i, j;
    hufTree *cur;

    if ((htData = (long) readData(15)) != NULL)
        {
        int p = 0;

        if ((htRoot = (long) (new hufTree)) == NULL)
            {
            discardData((discardable *) htData);
            htData = NULL;
            return (FALSE);
            }

        ((hufTree *) htRoot)->left = NULL;
        ((hufTree *) htRoot)->right = NULL;

        for (i = 1; i < 255; i++)
            {
            cur = (hufTree *) htRoot;
            dataOffsets[i] = p++;

            for (j = 0; j < ((char *)(((discardable *)htData)->data))[dataOffsets[i]]; j++)
                {
                cur = followOrMakeTree(cur, ((char *)(((discardable *)htData)->data))[p++]);

                if (!cur)
                    {
                    discardData((discardable *) htData);
                    htData = NULL;
                    freeHufTree();
                    return (FALSE);
                    }
                }

            cur->theChar = (char) i;
            }

        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }


// --------------------------------------------------------------------------
// Unbuilds tree.

static void freeTree(hufTree *cur)
    {
    if (cur->left)
        {
        freeTree(cur->left);
        }

    if (cur->right)
        {
        freeTree(cur->right);
        }

    delete cur;
    }

void freeHufTree(void)
    {
    if (htRoot)
        {
        freeTree((hufTree *) htRoot);
        htRoot = NULL;
        }

    if (htData)
        {
        discardData((discardable *) htData);
        htData = NULL;
        }
    }

#ifdef WINCIT
// For DOS versions, this stuff is in HUFBIT.ASM.

// --------------------------------------------------------------------------
// Returns next bit from str.

static int nextBit (uchar *str)
    {
	int wow;

    if (!(bitsenc & 7))
        {
        // first bit of new byte
        if (str[bitsenc >> 3] == ESC)
            {
            bitsenc += 8;
            switch (str[bitsenc >> 3])
                {
                case 1:
                    {
                    str[bitsenc >> 3] = 0;
                    break;
                    }

                case 2:
                    {
                    str[bitsenc >> 3] = ESC;
                    break;
                    }

                case 3:
                    {
                    str[bitsenc >> 3] = 255;
                    break;
                    }

                default:
                    {
//                  mPrintfCR(getmsg(25), str[bitsenc >> 3]);
                    break;
                    }
                }
            }
        }
	
    wow = str[bitsenc >> 3] & (1 << (7 - (bitsenc & 7)));
	bitsenc++;
	return wow;
    }


// --------------------------------------------------------------------------
// Decodes the next sequence in the bitstream.

uchar decode(uchar *str)
    {
    hufTree *cur = (hufTree *) htRoot;

    while (cur->left)
        {
        if (nextBit(str))
            {
            cur = cur->right;
            }
        else
            {
            cur = cur->left;
            }
        }

    return (cur->theChar);
    }
#endif


// --------------------------------------------------------------------------
// Decompresses string at str - writes back to str.

void /* TERMWINDOWMEMBER */ decompress(uchar *str)
    {
    int decoded = 0;
    bitsenc = 16;
    hufOrig = 0;

    if (!htRoot)
        {
        if (!initHuf())
            {
            return;
            }
        }

    if (!encbuf)
        {
        encbuf = new uchar[ENCBUFSIZE];

        if (!encbuf)
            {
            return;
            }
        }

    memset(encbuf, 0, ENCBUFSIZE);

    if (str[0] == 1)
        {
        hufNew = strlen((char *) str);
        if (str[1] == ESC)
            {
            bitsenc += 8;
            switch (str[2])
                {
                case 1:
                    {
                    break;
                    }

                case 2:
                    {
                    hufOrig = ESC;
                    break;
                    }

                case 3:
                    {
                    hufOrig = 255;
                    break;
                    }

                default:
                    {
#ifdef WINCIT
                    errorDisp(getmsg(25), str[2]);
#else
                    mPrintfCR(getmsg(25), str[2]);
#endif
                    break;
                    }
                }
            }
        else
            {
            hufOrig = str[1];
            }

        if (str[bitsenc >> 3] == ESC)
            {
            bitsenc += 8;
            switch (str[bitsenc >> 3])
                {
                case 1:
                    {
                    // This is a way to work around pre-65.155 corrupted
                    // messages. In reality, we should always break here.
                    if (hufNew < 300)
                        {
                        break;
                        }
#ifdef WINCIT
                    errorDisp(getmsg(24));
#else
                    mPrintfCR(getmsg(24));
#endif
                    }

                case 2:
                    {
                    hufOrig += ESC << 8;
                    break;
                    }

                case 3:
                    {
                    hufOrig += 255 << 8;
                    break;
                    }

                default:
                    {
#ifdef WINCIT
                    errorDisp(getmsg(25), str[bitsenc >> 3]);
#else
                    mPrintfCR(getmsg(25), str[bitsenc >> 3]);
#endif
                    break;
                    }
                }
            }
        else
            {
            hufOrig += str[bitsenc >> 3] << 8;
            }

        bitsenc += 8;

        // only decode as many bytes as we have room
        if (hufOrig > ENCBUFSIZE - 1) hufOrig = ENCBUFSIZE - 1;

        while (decoded < hufOrig)
            {
            encbuf[decoded++] = decode(str);
            }

        strcpy((char *) str, (char *) encbuf);
        }
    }


// --------------------------------------------------------------------------
// Encodes the character in to the bitstream.

void /* TERMWINDOWMEMBER */ encode(uchar in)
    {
    int i;

    if ( (long)bitsenc + (long) ((char *)(((discardable *)htData)->data))[dataOffsets[in]] >
            (long)(ENCBUFSIZE-1) * 8
        || in == 0 || in == 255 || hufNew == -1)
        {
        hufNew = -1;
        return;
        }

    for (i = 0; i < ((char *)(((discardable *)htData)->data))[dataOffsets[in]]; i++)
        {
        encbuf[bitsenc >> 3] |= (char)
                (((char *)(((discardable *)htData)->data))[dataOffsets[in] + i + 1] << (7 - (bitsenc & 7)));
        bitsenc++;
        if (!(bitsenc & 7))     // done with byte; check for icky character
            {
            switch (encbuf[(bitsenc - 1) >> 3])
                {
                case 0:
                    {
                    encbuf[(bitsenc - 1) >> 3] = ESC;
                    encbuf[bitsenc >> 3] = 1;
                    bitsenc += 8;
                    break;
                    }

                case ESC:
                    {
                    encbuf[bitsenc >> 3] = 2;
                    bitsenc += 8;
                    break;
                    }

                case 255:
                    {
                    encbuf[(bitsenc - 1) >> 3] = ESC;
                    encbuf[bitsenc >> 3] = 3;
                    bitsenc += 8;
                    break;
                    }

                // Default: Leave alone.
                }
            }
        }
    }


// --------------------------------------------------------------------------
// Compresses string at str; new string at str - checks for length.

Bool /* TERMWINDOWMEMBER */ compress(char *str)
    {
    int i;

    if (!htRoot)
        {
        if (!initHuf())
            {
            return (FALSE);
            }
        }

    if (!encbuf)
        {
        encbuf = new uchar[ENCBUFSIZE];

        if (!encbuf)
            {
            return (FALSE);
            }
        }

    hufOrig = strlen(str);
    memset(encbuf, 0, ENCBUFSIZE);
    bitsenc = 16;

    encbuf[0] = 1;  // compression type
    encbuf[1] = (uchar) (hufOrig & 255);
    switch (encbuf[1])
        {
        case 0:
            {
            encbuf[1] = ESC;
            encbuf[2] = 1;
            bitsenc += 8;
            break;
            }

        case ESC:
            {
            encbuf[2] = 2;
            bitsenc += 8;
            break;
            }

        case 255:
            {
            encbuf[1] = ESC;
            encbuf[2] = 3;
            bitsenc += 8;
            break;
            }
        }

    encbuf[bitsenc >> 3] = (uchar) (hufOrig >> 8);
    switch (encbuf[bitsenc >> 3])
        {
        case 0:
            {
            encbuf[bitsenc >> 3] = ESC;
            bitsenc += 8;
            encbuf[bitsenc >> 3] = 1;
            break;
            }

        case ESC:
            {
            bitsenc += 8;
            encbuf[bitsenc >> 3] = 2;
            break;
            }

        case 255:
            {
            encbuf[bitsenc >> 3] = ESC;
            bitsenc += 8;
            encbuf[bitsenc >> 3] = 3;
            break;
            }
        }
    bitsenc += 8;

    for (i = 0; i < hufOrig; i++)
        {
        encode(str[i]);
        }

    hufNew = strlen((char *) encbuf);

    if (hufNew >= hufOrig || hufNew == -1)
        {
        return (FALSE);
        }
    strcpy(str, (char *) encbuf);
    return (TRUE);
    }
