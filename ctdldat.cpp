#ifdef __GNUC__
#define cdecl
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef PORTABLE
#include <direct.h>
#endif
#include <time.h>
#ifndef PORTABLE
#include <dos.h>
#include <conio.h>
#endif
#include <string.h>
#include <stdarg.h>
#ifdef PORTABLE
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#else
#ifndef WINCIT
#include <bios.h>
#endif
#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <sys/stat.h>
#endif

#ifdef PORTABLE
#define strcmpi(a,b) strcasecmp(a,b)
#define strncmpi(a,b,c) strncasecmp(a,b,c)
#endif

#ifndef VISUALC
unsigned cdecl int _stklen = 1024*12;       // set up 12K of stack
#endif

#ifdef VISUALC
#define strncmpi strnicmp
#endif

#define FALSE 0
#define TRUE 1

#include "comp.h"
#include "compx.h"

typedef unsigned int uint;
typedef unsigned long ulong;

int parse_it(char *words[], char input[]);
int qtext(char *buf, char *line, char end);
void disposeLL(void **list);
void *addLL(void **list, uint nodeSize);
void *getNextLL(void *list);
void *getLLNum(void *list, ulong n);
void deleteLLNode(void **list, ulong n);
ulong getLLCount(void *list);

class StringCompressor : CompressionEngine
    {
    FILE *OutFile;
    long OriginalPlace;
    long UncompressedLength;

public:
    StringCompressor(FILE *File, CEDataType DataType) :
            CompressionEngine(DataType)
        {
        OutFile = File;

        OriginalPlace = ftell(OutFile);
        fseek(OutFile, sizeof(long), SEEK_CUR);

        UncompressedLength = 0;
        }

    void SaveString(const char *TheString);

    void AllDone(void)
        {
        FlushOutput();

        long EndPlace = ftell(OutFile);
        fseek(OutFile, OriginalPlace, SEEK_SET);
        fwrite(&UncompressedLength, 1, sizeof(long), OutFile);
        fseek(OutFile, EndPlace, SEEK_SET);
        }

    virtual unsigned char GetByte(void) { return (0); }

    virtual void WriteByte(unsigned char ToWrite)
        {
        if (OutFile)
            {
            fwrite(&ToWrite, 1, sizeof(unsigned char), OutFile);
            }
        }

    CEError GetError(void)
        {
        return CompressionEngine::GetError();
        }
    };

struct strList
    {
    strList *next;
    char str[1];
    };

class FileCompressorX : CompressionEngineX
    {
    FILE *OutFile;
    long OriginalPlace;

public:
    FileCompressorX(FILE *File) :
            CompressionEngineX()
        {
        OutFile = File;
        OriginalPlace = ftell(OutFile);
        fseek(OutFile, sizeof(long), SEEK_CUR);
        }

    void CompressToFile(void);

    virtual void writeByte(char ToWrite)
        {
        if (OutFile)
            {
            fwrite(&ToWrite, 1, sizeof(char), OutFile);
            }
        }
    };

void FileCompressorX::CompressToFile(void)
    {
    CompressIt();

    const long EndPlace = ftell(OutFile);
    fseek(OutFile, OriginalPlace, SEEK_SET);
    const long UncompressedLength = GetUncompressedLength();
    fwrite(&UncompressedLength, 1, sizeof(long), OutFile);
    fseek(OutFile, EndPlace, SEEK_SET);
    }

class StringCompressorX : public FileCompressorX
    {
    const strList *theStr;
    const char *theChar;

public:
    StringCompressorX(FILE *File, const strList *theList) :
            FileCompressorX(File)
        {
        theStr = theList;
        theChar = theStr->str;
        }

    virtual int readByte(char *TheByte)
        {
        if (theChar)
            {
            *TheByte = *theChar;

            if (!*(theChar++))
                {
                theStr = (const strList *) getNextLL((void *) theStr);

                if (theStr)
                    {
                    theChar = theStr->str;
                    }
                else
                    {
                    theChar = NULL;
                    }
                }

            return (1);
            }
        else
            {
            return (0);
            }
        }

    virtual int isEndOfInput(void)
        {
        return (theChar == NULL);
        }
    };

class BlockCompressorX : public FileCompressorX
    {
    char *Block;
    int Index;
    int BlockLength;

public:
    BlockCompressorX(FILE *File) : FileCompressorX(File)
        {
        Block = NULL;
        Index = BlockLength = 0;
        }

    int AddChar(char ToAdd);
    int AddInt(int ToAdd);
    int AddLong(long ToAdd);

    virtual int readByte(char *TheByte)
        {
        if (Block && Index < BlockLength)
            {
            *TheByte = Block[Index++];
            return (1);
            }
        else
            {
            return (0);
            }
        }

    virtual int isEndOfInput(void)
        {
        return (Index >= BlockLength);
        }
    };

int BlockCompressorX::AddChar(char ToAdd)
    {
    BlockLength += sizeof(char);
    char *NewBlock = (char *) realloc(Block, BlockLength);

    if (NewBlock)
        {
		Block = NewBlock;

        Block[BlockLength - 1] = ToAdd;

        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }

int BlockCompressorX::AddInt(int ToAdd)
    {
    BlockLength += sizeof(short);

    char *NewBlock = (char *) realloc(Block, BlockLength);

    if (NewBlock)
        {
		Block = NewBlock;

        Block[BlockLength - 2] = (char) (ToAdd & 0xFF);
        Block[BlockLength - 1] = (char) (ToAdd >> 8);

        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }

int BlockCompressorX::AddLong(long ToAdd)
    {
    BlockLength += sizeof(long);
    char *NewBlock = (char *) realloc(Block, BlockLength);

    if (NewBlock)
        {
		Block = NewBlock;

        Block[BlockLength - 4] = (char) (ToAdd & 0xFF);
        Block[BlockLength - 3] = (char) ((ToAdd >> 8) & 0xFF);
        Block[BlockLength - 2] = (char) ((ToAdd >> 16) & 0xFF);
        Block[BlockLength - 1] = (char) (ToAdd >> 24);

        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }

void StringCompressor::SaveString(const char *TheString)
    {
    while (*TheString)
        {
        UncompressedLength++;
        CompressByte(*TheString);
        TheString++;
        }

    UncompressedLength++;
    CompressByte(*TheString);
    }

enum subTypes
    {
    SUB_STRING,     SUB_RAWDATA,    SUB_STRCMP,     SUB_STRCMPK,
    SUB_STRCMPT,    SUB_STRCMPA,    SUB_STRCMPX,    SUB_FORMATTED,
    SUB_FORMATTEDX, SUB_RAWDATAX,

    SUB_BAD
    };

struct subEntry
    {
    subEntry *next;
    subTypes type;
    };

struct entryInfo
    {
    entryInfo *next;
    subEntry *sub;
    };

struct info
    {
    long ver;
    entryInfo *entry;
    };

struct defineInfo
    {
    defineInfo *next;

    long Value;
    char Name[1];
    };

enum FormatTypeE
    {
    FT_LONG,    FT_INT,     FT_CHAR,    FT_IGNORE,

    FT_BAD
    };

struct formatInfo
    {
    formatInfo *next;

    FormatTypeE Type;
    };

strList *GlobalDefines;


int DoIfProcessing(char **words, int *count)
    {
    if (!strcmpi(words[0], "IF") || !strcmpi(words[0], "IFNOT"))
        {
        strList *curDef;

        for (curDef = GlobalDefines; curDef;
                curDef = (strList *) getNextLL(curDef))
            {
            if (!strcmpi(curDef->str, words[1]))
                {
                break;
                }
            }

// Borland C++ 4.5 refuses to compile this line, but I think it is valid.
// Which of us is correct?
//      if ((!strcmpi(words[0], "IF")) ? !curDef : curDef)
//          {
//          return (FALSE);
//          }


        if (!strcmpi(words[0], "IF"))
            {
            if (!curDef)
                {
                return (FALSE);
                }
            }
        else
            {
            if (curDef)
                {
                return (FALSE);
                }
            }

        *count -= 2;

        for (int i = 0; i < *count; i++)
            {
            words[i] = words[i + 2];
            }
        }

    return (TRUE);
    }

int cdecl main(int argc, char *argv[])
    {
    FILE *in, *out;
    char inFile[80], outFile[80];
    info data;
    int Compression = TRUE;

    strcpy(inFile, "ctdldata.def");
    strcpy(outFile, "ctdl.dat");

    printf("Citadel+/066 Datafile Compiler\n"
            "By Anticlimactic Teleservices");

    while (--argc)
        {
        if (argv[argc][0] == '/' || argv[argc][0] == '-')
            {
            switch (tolower(argv[argc][1]))
                {
                case 'd':
                    {
                    printf("\nDefine: %s", argv[argc] + 2);
                    strList *curDef = (strList *) addLL((void **)
                            &GlobalDefines, sizeof(*GlobalDefines) +
                            strlen(argv[argc] + 2));

                    if (curDef)
                        {
                        strcpy(curDef->str, argv[argc] + 2);
                        }
                    else
                        {
                        printf("Out of memory.");
                        return (3);
                        }

                    break;
                    }

                case 'n':
                    {
                    printf("\nNo compression.");
                    Compression = FALSE;
                    break;
                    }

                case 'i':
                    {
                    strcpy(inFile, argv[argc] + 2);
                    break;
                    }

                case 'o':
                    {
                    strcpy(outFile, argv[argc] + 2);
                    break;
                    }

                case '?':
                    {
                    printf("\n\nUsage: %s [-dsymbol][-n][-ifilename][-ofilename]\n\n"
                        "       -d define symbol\n"
                        "       -n No compression\n"
                        "       -i Specify input file name\n"
                        "       -o Specify output file name\n", argv[0]);
                    return (1);
                    }

                default:
                    {
                    printf("\nUnknown command-line parameter: %s",
                            argv[argc]);
                    return (2);
                    }
                }
            }
        else
            {
            printf("\nUnknown command-line parameter: %s", argv[argc]);
            return (2);
            }
        }

    printf("\nUsing input data file: %s\n"
        "Using output data file: %s", inFile, outFile);

    if ((in = fopen(inFile, "rt")) != NULL)
        {
        if ((out = fopen(outFile, "wb")) != NULL)
            {
            char line[90], line2[90];
            char *words[256];
            entryInfo *curEnt = NULL;
            subEntry *curSub = NULL;
            int lnum, bad, count;
            long l;

            data.ver = 0;
            data.entry = NULL;

            printf("\n\nPass one...\n");

            bad = FALSE;
            lnum = 0;

            int FormatCounter = 0;

            while (fgets(line, 90, in) != NULL)
                {
                lnum++;

                if (strchr(line, '\r')) *strchr(line, '\r') = 0;
                else if (strchr(line, '\n')) *strchr(line, '\n') = 0;
		else
                    {
                    printf("%d: Line too long\n", lnum);
                    return (4);
                    }

                strcpy(line2, line);

                count = parse_it(words, line);

                if (count)
                    {
                    if (words[0][0] == ';')
                        {
                        continue;
                        }

                    if (!DoIfProcessing(words, &count))
                        {
                        continue;
                        }

                    if (!strcmpi(words[0], "VERSION"))
                        {
                        if (count == 2)
                            {
                            data.ver = atol(words[1]);
                            }
                        else
                            {
                            printf("%d:Invalid VERSION (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "ENTRY"))
                        {
                        if (count == 1)
                            {
                            curEnt = (entryInfo *) addLL((void **) &data.entry,
                                    sizeof(*data.entry));

                            if (!curEnt)
                                {
                                printf("Out of memory.");
                                return (3);
                                }
                            }
                        else
                            {
                            printf("%d:Invalid ENTRY (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "ENDENT"))
                        {
                        if (count == 1)
                            {
                            if (curSub)
                                {
                                printf("%d:ENDENT with open SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            else if (curEnt)
                                {
                                curEnt = NULL;
                                }
                            else
                                {
                                printf("%d:ENDENT with no ENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid ENDENT (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "SUBENTRY"))
                        {
                        if (curEnt)
                            {
                            if (count > 1)
                                {
                                curSub = (subEntry *) addLL((void **) &curEnt->sub,
                                        sizeof(*curEnt->sub));

                                if (!curSub)
                                    {
                                    printf("Out of memory.");
                                    return (3);
                                    }

                                if (!strcmpi(words[1], "STRING"))
                                    {
                                    curSub->type = SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "RAWDATA"))
                                    {
                                    curSub->type = SUB_RAWDATA;
                                    }
                                else if (!strcmpi(words[1], "RAWDATACOMPX"))
                                    {
                                    curSub->type = Compression ? SUB_RAWDATAX : SUB_RAWDATA;
                                    }
                                else if (!strcmpi(words[1], "STRCMP"))
                                    {
                                    curSub->type = Compression ? SUB_STRCMP : SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "STRCMPK"))
                                    {
                                    curSub->type = Compression ? SUB_STRCMPK : SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "STRCMPT"))
                                    {
                                    curSub->type = Compression ? SUB_STRCMPT : SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "STRCMPA"))
                                    {
                                    curSub->type = Compression ? SUB_STRCMPA : SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "STRCMPX"))
                                    {
                                    curSub->type = Compression ? SUB_STRCMPX : SUB_STRING;
                                    }
                                else if (!strcmpi(words[1], "FORMATTED"))
                                    {
                                    curSub->type = SUB_FORMATTED;
                                    }
                                else if (!strcmpi(words[1], "FORMATTEDCOMPX"))
                                    {
                                    curSub->type = Compression ? SUB_FORMATTEDX : SUB_FORMATTED;
                                    }
                                else
                                    {
                                    printf("%d:Invalid SUBENTRY type: %s\n", lnum, words[1]);
                                    curSub->type = SUB_BAD;
                                    bad = TRUE;
                                    }
                                }
                            else
                                {
                                printf("%d:Invalid SUBENTRY (%s)\n", lnum, line2);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:SUBENTRY with no ENTRY\n", lnum);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "ENDSUB"))
                        {
                        if (count == 1)
                            {
                            if (curSub)
                                {
                                curSub = NULL;
                                FormatCounter = 0;
                                }
                            else
                                {
                                printf("%d:ENDSUB with no ENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid ENDSUB (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "STRING"))
                        {
                        if (count == 2)
                            {
                            if (curSub)
                                {
                                if (!(curSub->type == SUB_STRING ||
                                        curSub->type == SUB_STRCMP ||
                                        curSub->type == SUB_STRCMPA ||
                                        curSub->type == SUB_STRCMPT ||
                                        curSub->type == SUB_STRCMPX ||
                                        curSub->type == SUB_STRCMPK))
                                    {
                                    printf("%d:STRING not in proper SUBENTRY\n", lnum);
                                    bad = TRUE;
                                    }
                                }
                            else
                                {
                                printf("%d:STRING without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid STRING (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DB"))
                        {
                        if (count > 1)
                            {
                            if (curSub)
                                {
                                if (curSub->type != SUB_RAWDATA &&
                                        curSub->type != SUB_RAWDATAX)
                                    {
                                    bad = TRUE;
                                    printf("%d:DB not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:DB without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid DB (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DW"))
                        {
                        if (count > 1)
                            {
                            if (curSub)
                                {
                                if (curSub->type != SUB_RAWDATA &&
                                        curSub->type != SUB_RAWDATAX)
                                    {
                                    bad = TRUE;
                                    printf("%d:DW not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:DW without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid DW (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DD"))
                        {
                        if (count > 1)
                            {
                            if (curSub)
                                {
                                if (curSub->type != SUB_RAWDATA &&
                                        curSub->type != SUB_RAWDATAX)
                                    {
                                    bad = TRUE;
                                    printf("%d:DD not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:DD without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid DD (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DDU"))
                        {
                        if (count > 1)
                            {
                            if (curSub)
                                {
                                if (curSub->type != SUB_RAWDATA &&
                                        curSub->type != SUB_RAWDATAX)
                                    {
                                    bad = TRUE;
                                    printf("%d:DDU not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:DDU without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid DDU (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "STRFILE"))
                        {
                        if (count == 2)
                            {
                            if (curSub)
                                {
                                if (!(curSub->type == SUB_STRING ||
                                        curSub->type == SUB_STRCMP ||
                                        curSub->type == SUB_STRCMPA ||
                                        curSub->type == SUB_STRCMPT ||
                                        curSub->type == SUB_STRCMPX ||
                                        curSub->type == SUB_STRCMPK))
                                    {
                                    bad = TRUE;
                                    printf("%d:STRFILE not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:STRFILE without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid STRFILE (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "FORMAT"))
                        {
                        if (curSub)
                            {
                            if (!(curSub->type == SUB_FORMATTED || curSub->type == SUB_FORMATTEDX))
                                {
                                bad = TRUE;
                                printf("%d:FORMAT not in proper SUBENTRY\n", lnum);
                                }
                            else if (++FormatCounter > 1)
                                {
                                bad = TRUE;
                                printf("%d:Only one FORMAT per SUBENTRY\n", lnum);
                                }
                            }
                        else
                            {
                            printf("%d:FORMAT without SUBENTRY\n", lnum);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DEFINE"))
                        {
                        if (count == 3)
                            {
                            if (curSub)
                                {
                                if (!(curSub->type == SUB_FORMATTED || curSub->type == SUB_FORMATTEDX))
                                    {
                                    bad = TRUE;
                                    printf("%d:DEFINE not in proper SUBENTRY\n", lnum);
                                    }
                                }
                            else
                                {
                                printf("%d:DEFINE without SUBENTRY\n", lnum);
                                bad = TRUE;
                                }
                            }
                        else
                            {
                            printf("%d:Invalid DEFINE (%s)\n", lnum, line2);
                            bad = TRUE;
                            }
                        }
                    else if (!strcmpi(words[0], "DATA"))
                        {
                        if (curSub)
                            {
                            if (!(curSub->type == SUB_FORMATTED || curSub->type == SUB_FORMATTEDX))
                                {
                                bad = TRUE;
                                printf("%d:DATA not in proper SUBENTRY\n", lnum);
                                }
                            else if (!FormatCounter)
                                {
                                bad = TRUE;
                                printf("%d:DATA before FORMAT\n", lnum);
                                }
                            }
                        else
                            {
                            printf("%d:DATA without SUBENTRY\n", lnum);
                            bad = TRUE;
                            }
                        }
                    else
                        {
                        printf("%d:Unknown command: %s\n", lnum, words[0]);
                        bad  = TRUE;
                        }
                    }
                }

            if (bad)
                {
                return (4);
                }

            printf("Pass two...\n");

            fseek(in, 0, SEEK_SET);
            bad = FALSE;

            fwrite(&data.ver, sizeof(data.ver), 1, out);
            l = getLLCount(data.entry);
            fwrite(&l, sizeof(l), 1, out);
            fseek(out, sizeof(l) * l, SEEK_CUR);

            for (l = 1, curEnt = (entryInfo *) data.entry; curEnt;
                    l++, curEnt = (entryInfo *) getNextLL(curEnt))
                {
                long l2, l3;

                l2 = ftell(out);
                fseek(out, sizeof(l) * (l + 1), SEEK_SET);
                fwrite(&l2, sizeof(l2), 1, out);
                fseek(out, l2, SEEK_SET);

                l3 = getLLCount(curEnt->sub);
                fwrite(&l3, sizeof(l3), 1, out);

                bad = TRUE;
                while (fgets(line, 90, in) != NULL)
                    {

                    if (strchr(line, '\r')) *strchr(line, '\r') = 0;
                    else if (strchr(line, '\n')) *strchr(line, '\n') = 0;

                    count = parse_it(words, line);

                    if (!DoIfProcessing(words, &count))
                        {
                        continue;
                        }

                    if (count && !strcmpi(words[0], "ENTRY"))
                        {
                        bad = FALSE;
                        break;
                        }
                    }

                if (bad)
                    {
                    printf("Could not find entry %ld\n", l);
                    return (5);
                    }

                int CurSubCounter;
                for (CurSubCounter = 1, curSub = curEnt->sub; curSub;
                        curSub = (subEntry *) getNextLL(curSub), CurSubCounter++)
                    {
                    long off1, off2;

                    subTypes ToWrite = curSub->type;

                    switch (ToWrite)
                        {
                        case SUB_FORMATTED: ToWrite = SUB_RAWDATA; break;
                        case SUB_FORMATTEDX: ToWrite = SUB_RAWDATAX; break;
                        }

#if defined(WINCIT) || defined(PORTABLE)
                    fwrite(&ToWrite, sizeof(short), 1, out);
#else
                    fwrite(&ToWrite, sizeof(ToWrite), 1, out);
#endif
                    off1 = ftell(out);

                    fseek(out, sizeof(off1), SEEK_CUR);

                    bad = TRUE;
                    while (fgets(line, 90, in) != NULL)
                        {
                        if (strchr(line, '\r')) *strchr(line, '\r') = 0;
                        else if (strchr(line, '\n')) *strchr(line, '\n') = 0;

                        count = parse_it(words, line);

                        if (!DoIfProcessing(words, &count))
                            {
                            continue;
                            }

                        if (count && !strcmpi(words[0], "SUBENTRY"))
                            {
                            bad = FALSE;
                            break;
                            }
                        }

                    if (bad)
                        {
                        printf("Could not find subentry in entry %ld\n", l);
                        return (5);
                        }

                    printf("%ld:%d \r", l, CurSubCounter);

                    switch (curSub->type)
                        {
                        case SUB_STRING:
                            {
                            bad = TRUE;
                            while (fgets(line, 90, in) != NULL)
                                {
				if (strchr(line, '\r')) *strchr(line, '\r')=0;
				else if (strchr(line, '\n')) *strchr(line, '\n')=0;

                                count = parse_it(words, line);

                                if (!DoIfProcessing(words, &count))
                                    {
                                    continue;
                                    }

                                if (!count || words[0][0] == ';')
                                    {
                                    continue;
                                    }

                                if (!strcmpi(words[0], "STRING"))
                                    {
                                    if (!strcmp(words[1], "{}"))
                                        {
                                        words[1][0] = 0;
                                        }

                                    fwrite(words[1], strlen(words[1]) + 1, 1, out);
                                    }
                                else if (!strcmpi(words[0], "STRFILE"))
                                    {
                                    FILE *sf;

                                    if ((sf = fopen(words[1], "rt")) != NULL)
                                        {
                                        char ll[256], *p;

                                        while (fgets(ll, 256, sf) != NULL)
                                            {
					    p = ll+strlen(ll);
					    p--;
					    while (p>=ll && strchr("\r\n\4",*p))
						    *p-- = 0;

                                            if (!strcmp(ll, "{}"))
                                                {
                                                ll[0] = 0;
                                                }

                                            fwrite(ll, strlen(ll) + 1, 1, out);
                                            }

                                        fclose(sf);
                                        }
                                    else
                                        {
                                        printf("Could not open file: %s\n", words[1]);
                                        return (6);
                                        }
                                    }
                                else if (!strcmpi(words[0], "ENDSUB"))
                                    {
                                    bad = FALSE;
                                    break;
                                    }
                                else
                                    {
                                    printf("Internal error 1 in entry %ld:%d\n", l, CurSubCounter);
                                    return (5);
                                    }
                                }

                            if (bad)
                                {
                                printf("Could not find end of subentry in entry %ld\n", l);
                                return (5);
                                }

                            break;
                            }

                        case SUB_STRCMPA:
                        case SUB_STRCMPT:
                        case SUB_STRCMPK:
                        case SUB_STRCMP:
                            {
                            StringCompressor StrCmp(out,
                                    curSub->type == SUB_STRCMP ? CEDT_NORMAL :
                                    curSub->type == SUB_STRCMPK ? CEDT_KEYWORDS :
                                    curSub->type == SUB_STRCMPA ? CEDT_ALPHA :
                                    CEDT_TEXT);

                            bad = TRUE;
                            while (fgets(line, 90, in) != NULL)
                                {
				if (strchr(line, '\r')) *strchr(line, '\r')=0;
				else if (strchr(line, '\n')) *strchr(line, '\n')=0;

                                count = parse_it(words, line);

                                if (!DoIfProcessing(words, &count))
                                    {
                                    continue;
                                    }

                                if (!count || words[0][0] == ';')
                                    {
                                    continue;
                                    }

                                if (!strcmpi(words[0], "STRING"))
                                    {
                                    if (!strcmp(words[1], "{}"))
                                        {
                                        words[1][0] = 0;
                                        }

                                    StrCmp.SaveString(words[1]);
                                    }
                                else if (!strcmpi(words[0], "STRFILE"))
                                    {
                                    FILE *sf;

                                    if (!strcmpi(words[1], "COMMANDS.SMP"))
                                        {
                                        int i = 1;
                                        }

                                    if ((sf = fopen(words[1], "rt")) != NULL)
                                        {
                                        char ll[256], *p;

                                        while (fgets(ll, 256, sf) != NULL)
                                            {
					    p = ll+strlen(ll);
					    p--;
					    while (p>=ll && strchr("\r\n\4",*p))
						    *p-- = 0;
                                            if (!strcmp(ll, "{}"))
                                                {
                                                ll[0] = 0;
                                                }

                                            StrCmp.SaveString(ll);
                                            }

                                        fclose(sf);
                                        }
                                    else
                                        {
                                        printf("Could not open file: %s\n", words[1]);
                                        return (6);
                                        }
                                    }
                                else if (!strcmpi(words[0], "ENDSUB"))
                                    {
                                    bad = FALSE;
                                    break;
                                    }
                                else
                                    {
                                    printf("Internal error 2 in entry %ld:%d\n", l, CurSubCounter);
                                    return (5);
                                    }
                                }

                            if (bad)
                                {
                                printf("Could not find end of subentry in entry %ld\n", l);
                                return (5);
                                }

                            if (StrCmp.GetError() != CEE_NOERROR)
                                {
                                printf("Error in compression.");
                                return (3);
                                }

                            StrCmp.AllDone();

                            break;
                            }


                        case SUB_STRCMPX:
                            {
                            strList *List = NULL;

                            bad = TRUE;
                            while (fgets(line, 90, in) != NULL)
                                {
				if (strchr(line, '\r')) *strchr(line, '\r')=0;
				else if (strchr(line, '\n')) *strchr(line, '\n')=0;

                                count = parse_it(words, line);

                                if (!DoIfProcessing(words, &count))
                                    {
                                    continue;
                                    }

                                if (!count || words[0][0] == ';')
                                    {
                                    continue;
                                    }

                                if (!strcmpi(words[0], "STRING"))
                                    {
                                    if (!strcmp(words[1], "{}"))
                                        {
                                        words[1][0] = 0;
                                        }

                                    strList *curList = (strList *) addLL((void **)
                                            &List, sizeof(strList) + strlen(words[1]));

                                    if (!curList)
                                        {
                                        printf("Out of memory.");
                                        return (3);
                                        }

                                    strcpy(curList->str, words[1]);
                                    }
                                else if (!strcmpi(words[0], "STRFILE"))
                                    {
                                    FILE *sf;

                                    if ((sf = fopen(words[1], "rt")) != NULL)
                                        {
                                        char ll[256], *p;

                                        while (fgets(ll, 256, sf) != NULL)
                                            {
					    p = ll+strlen(ll);
					    p--;
					    while (p>=ll && strchr("\r\n\4",*p))
						    *p-- = 0;

                                            if (!strcmp(ll, "{}"))
                                                {
                                                ll[0] = 0;
                                                }

                                            strList *curList = (strList *) addLL((void **)
                                                    &List, sizeof(strList) + strlen(ll));

                                            if (!curList)
                                                {
                                                printf("Out of memory.");
                                                return (3);
                                                }

                                            strcpy(curList->str, ll);
                                            }

                                        fclose(sf);
                                        }
                                    else
                                        {
                                        printf("Could not open file: %s\n", words[1]);
                                        return (6);
                                        }
                                    }
                                else if (!strcmpi(words[0], "ENDSUB"))
                                    {
                                    bad = FALSE;
                                    break;
                                    }
                                else
                                    {
                                    printf("Internal error 3 in entry %ld:%d\n", l, CurSubCounter);
                                    return (5);
                                    }
                                }

                            if (bad)
                                {
                                printf("Could not find end of subentry in entry %ld\n", l);
                                return (5);
                                }

                            StringCompressorX StrCmpX(out, List);
                            StrCmpX.CompressToFile();

                            disposeLL((void **) &List);

                            break;
                            }

                        case SUB_RAWDATA:
                        case SUB_RAWDATAX:
                            {
                            BlockCompressorX *Cmp = NULL;

                            if (curSub->type == SUB_RAWDATAX)
                                {
                                Cmp = new BlockCompressorX(out);
                                }

                            bad = TRUE;
                            while (fgets(line, 90, in) != NULL)
                                {
				if (strchr(line, '\r')) *strchr(line, '\r')=0;
				else if (strchr(line, '\n')) *strchr(line, '\n')=0;

                                count = parse_it(words, line);

                                if (!DoIfProcessing(words, &count))
                                    {
                                    continue;
                                    }

                                if (!count || words[0][0] == ';')
                                    {
                                    continue;
                                    }

                                if (!strcmpi(words[0], "DB"))
                                    {
                                    int i;

                                    for (i = 1; i < count; i++)
                                        {
                                        char c = (char) strtol(words[i], NULL, (strncmpi(words[i], "0x", 2) == 0) ? 16 : 10);

                                        if (Cmp)
                                            {
                                            if (!Cmp->AddChar(c))
                                                {
                                                printf("Out of memory.");
                                                return (3);
                                                }
                                            }
                                        else
                                            {
                                            fwrite(&c, sizeof(c), 1, out);
                                            }
                                        }
                                    }
                                else if (!strcmpi(words[0], "DW"))
                                    {
                                    int i;

                                    for (i = 1; i < count; i++)
                                        {
                                        short c = (short) strtol(words[i], NULL, (strncmpi(words[i], "0x", 2) == 0) ? 16 : 10);

                                        if (Cmp)
                                            {
                                            if (!Cmp->AddInt(c))
                                                {
                                                printf("Out of memory.");
                                                return (3);
                                                }
                                            }
                                        else
                                            {
                                            fwrite(&c, sizeof(c), 1, out);
                                            }
                                        }
                                    }
                                else if (!strcmpi(words[0], "DD"))
                                    {
                                    int i;

                                    for (i = 1; i < count; i++)
                                        {
                                        long c = (long) strtol(words[i], NULL, (strncmpi(words[i], "0x", 2) == 0) ? 16 : 10);

                                        if (Cmp)
                                            {
                                            if (!Cmp->AddLong(c))
                                                {
                                                printf("Out of memory.");
                                                return (3);
                                                }
                                            }
                                        else
                                            {
                                            fwrite(&c, sizeof(c), 1, out);
                                            }
                                        }
                                    }
                                else if (!strcmpi(words[0], "DDU"))
                                    {
                                    int i;

                                    for (i = 1; i < count; i++)
                                        {
                                        long c = (long) strtoul(words[i], NULL, (strncmpi(words[i], "0x", 2) == 0) ? 16 : 10);

                                        if (Cmp)
                                            {
                                            if (!Cmp->AddLong(c))
                                                {
                                                printf("Out of memory.");
                                                return (3);
                                                }
                                            }
                                        else
                                            {
                                            fwrite(&c, sizeof(c), 1, out);
                                            }
                                        }
                                    }
                                else if (!strcmpi(words[0], "ENDSUB"))
                                    {
                                    bad = FALSE;
                                    break;
                                    }
                                else
                                    {
                                    printf("Internal error 4 in entry %ld:%d\n", l, CurSubCounter);
                                    return (5);
                                    }
                                }

                            if (bad)
                                {
                                printf("Could not find end of subentry in entry %ld\n", l);
                                return (5);
                                }

                            if (Cmp)
                                {
                                Cmp->CompressToFile();
                                delete Cmp;
                                }

                            break;
                            }

                        case SUB_FORMATTED:
                        case SUB_FORMATTEDX:
                            {
                            BlockCompressorX *Cmp = NULL;

                            defineInfo *LocalDefines = NULL;
                            formatInfo *Format = NULL;
                            int InFormat;

                            if (curSub->type == SUB_FORMATTEDX)
                                {
                                Cmp = new BlockCompressorX(out);
                                }

                            bad = TRUE;
                            while (fgets(line, 90, in) != NULL)
                                {
				if (strchr(line, '\r')) *strchr(line, '\r')=0;
				else if (strchr(line, '\n')) *strchr(line, '\n')=0;

                                count = parse_it(words, line);

                                if (!DoIfProcessing(words, &count))
                                    {
                                    continue;
                                    }

                                if (!count || words[0][0] == ';')
                                    {
                                    continue;
                                    }

                                if (!strcmpi(words[0], "FORMAT"))
                                    {
                                    InFormat = count - 1;
                                    disposeLL((void **) &Format);

                                    for (int i = 1; i < count; i++)
                                        {
                                        formatInfo *curFmt = (formatInfo *) addLL((void **) &Format, sizeof(*Format));

                                        if (!curFmt)
                                            {
                                            printf("Out of memory.");
                                            return (3);
                                            }

                                        if (!strcmpi(words[i], "CHAR"))
                                            {
                                            curFmt->Type = FT_CHAR;
                                            }
                                        else if (!strcmpi(words[i], "INT"))
                                            {
                                            curFmt->Type = FT_INT;
                                            }
                                        else if (!strcmpi(words[i], "LONG"))
                                            {
                                            curFmt->Type = FT_LONG;
                                            }
                                        else if (!strcmpi(words[i], "IGNORE"))
                                            {
                                            curFmt->Type = FT_IGNORE;
                                            }
                                        else
                                            {
                                            printf("Bad format type: %s",
                                                    words[i]);
                                            return (7);
                                            }
                                        }
                                    }
                                else if (!strcmpi(words[0], "DEFINE"))
                                    {
                                    defineInfo *curDef = (defineInfo *)
                                            addLL((void **) &LocalDefines,
                                            sizeof(*LocalDefines) +
                                            strlen(words[1]));

                                    if (curDef)
                                        {
                                        strcpy(curDef->Name, words[1]);
                                        curDef->Value = atol(words[2]);
                                        }
                                    else
                                        {
                                        printf("Out of memory.");
                                        return (3);
                                        }
                                    }
                                else if (!strcmpi(words[0], "DATA"))
                                    {
                                    if (count - 1 != InFormat)
                                        {
                                        printf("Wrong number of DATA entries.");
                                        return (8);
                                        }

                                    formatInfo *curFormat = Format;

                                    for (int i = 1; i < count; i++)
                                        {
                                        if (curFormat->Type != FT_IGNORE)
                                            {
                                            long c;

                                            if (isdigit(*words[i]) ||
                                                    *words[i] == '-' ||
                                                    *words[i] == '+')
                                                {
                                                c = strtol(words[i], NULL,
                                                        (strncmpi(words[i],
                                                        "0x", 2) == 0) ?
                                                        16 : 10);
                                                }
                                            else
                                                {
                                                defineInfo *curDef;

                                                for (curDef = LocalDefines; curDef;
                                                        curDef = (defineInfo *) getNextLL(curDef))
                                                    {
                                                    if (!strcmpi(curDef->Name,
                                                            words[i]))
                                                        {
                                                        c = curDef->Value;
                                                        break;
                                                        }
                                                    }

                                                if (!curDef)
                                                    {
                                                    printf("Unknown DEFINE: %s", words[i]);
                                                    return(9);
                                                    }
                                                }

                                            if (curFormat->Type == FT_CHAR)
                                                {
                                                const char i = (char) c;

                                                if (Cmp)
                                                    {
                                                    if (!Cmp->AddChar(i))
                                                        {
                                                        printf("Out of memory.");
                                                        return (3);
                                                        }
                                                    }
                                                else
                                                    {
                                                    fwrite(&i, sizeof(i), 1,
                                                            out);
                                                    }
                                                }
                                            else if (curFormat->Type == FT_INT)
                                                {
#if defined(WINCIT) || defined(PORTABLE)
                                                const short i = (short) c;
#else
                                                const int i = (int) c;
#endif
                                                if (Cmp)
                                                    {
                                                    if (!Cmp->AddInt(i))
                                                        {
                                                        printf("Out of memory.");
                                                        return (3);
                                                        }
                                                    }
                                                else
                                                    {
                                                    fwrite(&i, sizeof(i), 1,
                                                            out);
                                                    }
                                                }
                                            else
                                                {
                                                if (Cmp)
                                                    {
                                                    if (!Cmp->AddLong(c))
                                                        {
                                                        printf("Out of memory.");
                                                        return (3);
                                                        }
                                                    }
                                                else
                                                    {
                                                    fwrite(&c, sizeof(c), 1,
                                                            out);
                                                    }
                                                }
                                            }

                                        curFormat = (formatInfo *)
                                                getNextLL(curFormat);
                                        }
                                    }
                                else if (!strcmpi(words[0], "ENDSUB"))
                                    {
                                    bad = FALSE;
                                    break;
                                    }
                                else
                                    {
                                    printf("Internal error 5 in entry %ld:%d\n", l, CurSubCounter);
                                    return (5);
                                    }
                                }

                            if (bad)
                                {
                                printf("Could not find end of subentry in entry %ld\n", l);
                                return (5);
                                }

                            disposeLL((void **) &LocalDefines);
                            disposeLL((void **) &Format);

                            if (Cmp)
                                {
                                Cmp->CompressToFile();
                                delete Cmp;
                                }

                            break;
                            }

                        default:
                            {
                            printf("Internal error 6 in entry %ld:%d\n", l, CurSubCounter);
                            return(5);
                            }
                        }

                    off2 = ftell(out);
                    fseek(out, off1, SEEK_SET);
                    off1 = off2 - off1 - sizeof(off1);
                    fwrite(&off1, sizeof(off1), 1, out);
                    fseek(out, off2, SEEK_SET);
                    }
                }


            printf("\ndone.\n");

            fclose(out);
            }
        else
            {
            printf("\ncould not open %s for write\n", outFile);
            }

        fclose(in);
        }
    else
        {
        printf("\ncould not open %s for read\n", inFile);
        }

    return (0);
    }

// states of machine...
#define INWORD      0
#define OUTWORD     1
#define INQUOTES    2

// characters
#undef  TAB
#define TAB     9
#define BLANK   ' '
#define QUOTE   '\"'
#define QUOTE2  '\''
#define MXWORD  128

int parse_it(char *words[], char input[])
    {
    int i, state, thisword;

    input[strlen(input) + 1] = 0;

    for (state = OUTWORD, thisword = i = 0; input[i]; i++)
        {
        switch (state)
            {
            case INWORD:
                {
                if (isspace(input[i]))
                    {
                    input[i] = 0;
                    state = OUTWORD;
                    }
                break;
                }

            case OUTWORD:
                {
                if (input[i] == QUOTE || input[i] == QUOTE2)
                    {
                    state = INQUOTES;
                    }
                else if (!isspace(input[i]))
                    {
                    state = INWORD;
                    }

                // if we are now in a string, setup, otherwise, break
                if (state != OUTWORD)
                    {
                    if (thisword >= MXWORD)
                        {
                        return (thisword);
                        }

                    if (state == INWORD)
                        {
                        words[thisword++] = (input + i);
                        }
                    else
                        {
                        words[thisword++] = (input + i + 1);
                        }
                    }
                break;
                }

            case INQUOTES:
                {
                i += qtext(input + i, input + i, input[i - 1]);
                state = OUTWORD;
                break;
                }
            }
        }
    return (thisword);
    }

int qtext(char *buf, char *line, char end)
    {
    int index = 0;
    int slash = 0;
    char chr;

    while (line[index] != '\0' && (line[index] != end || slash != 0))
        {
        if (slash == 0)
            {
            if (line[index] == '\\')
                {
                slash = 1;
                }
            else if (line[index] == '^')
                {
                slash = 2;
                }
            else
                {
                *(buf++) = line[index];
                }
            }
        else if (slash == 1)
            {
            switch (line[index])
                {
                default:
                    {
                    *(buf++) = line[index];
                    break;
                    }

                case 'n':                       // newline
                    {
                    *(buf++) = '\n';
                    break;
                    }

                case 't':                       // tab
                    {
                    *(buf++) = '\t';
                    break;
                    }

                case 'r':                       // carriage return
                    {
                    *(buf++) = '\r';
                    break;
                    }

                case 'f':                       // formfeed
                    {
                    *(buf++) = '\f';
                    break;
                    }

                case 'b':                       // backspace
                    {
                    *(buf++) = '\b';
                    break;
                    }
                }
            slash = 0;
            }
        else // if (slash == 2)
            {
            if (line[index] == '?')
                {
                chr = 127;
                }
            else if (line[index] >= 'A' && line[index] <= 'Z')
                {
                chr = (char) (line[index] - 'A' + 1);
                }
            else if (line[index] >= 'a' && line[index] <= 'z')
                {
                chr = (char)(line[index] - 'a' + 1);
                }
            else
                {
                chr = line[index];
                }

            *(buf++) = chr;
            slash = 0;
            }

        index++;
        }

    *buf = 0;
    return (line[index] == end ? index + 1 : index);
    }

struct emptyLL
    {
    struct emptyLL  *next;
    };

void disposeLL(void **list)
    {
    struct emptyLL *nxt, *This;

    This = (emptyLL *) *list;

    while (This)
        {
        nxt = This->next;
        free(This);
        This = nxt;
        }

    *list = NULL;
    }

void *addLL(void **list, uint nodeSize)
    {
    struct emptyLL *New, *cur;

    New = (emptyLL *) calloc(1, nodeSize);

    if (*list)
        {
        cur = (emptyLL *) *list;
        while (cur->next)
            {
            cur = cur->next;
            }
        cur->next = New;
        }
    else
        {
        *list = New;
        }

    return (New);
    }

void *getNextLL(void *list)
    {
    if ((struct emptyLL *)list)
        {
        return (((struct emptyLL *)list)->next);
        }
    return (NULL);
    }

void *getLLNum(void *list, ulong n)
    {
    if (n)
        {
        while (--n && list)
            {
            list = ((struct emptyLL *) list)->next;
            }
        return (list);
        }
    return (NULL);
    }

void deleteLLNode(void **list, ulong n)
    {
    struct emptyLL *cur, *prev;

    if (*list && n)
        {
        if (n == 1)
            {
            cur = (emptyLL *) *list;
            *list = cur->next;
            }
        else
            {
            prev = (emptyLL *) getLLNum(*list, n - 1);
            if (prev)
                {
                cur = prev->next;

                if (cur)
                    {
                    prev->next = cur->next;
                    }
                }
            }

        free(cur);
        }
    }

ulong getLLCount(void *list)
    {
    ulong i = 0;

    while (list && ++i)
        {
        list = ((struct emptyLL *)list)->next;
        }
    return (i);
    }
