#include "comp.h"

#include <limits.h>

#ifdef PORTABLE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#else
#ifdef VISUALC
#include <memory.h>
#else
#include <mem.h>
#endif
#endif

#ifdef VISUALC
#define EXTRA_OPEN_BRACE {
#define EXTRA_CLOSE_BRACE }
#else
#define EXTRA_OPEN_BRACE 
#define EXTRA_CLOSE_BRACE 
#endif

static unsigned int PowersOf2[] =
    {
    1u,     2u,     4u,     8u,     16u,    32u,    64u,    128u,
    256u,   512u,   1024u,  2048u,  4096u,  8192u,  16384u, 32768u,
    };

#define CT_ALL      0x01
#define CT_KEYWORDS 0x02
#define CT_ALPHA    0x04
#define CT_TEXT     0x08

static unsigned char CharTypes[] =
    {
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 000 ^@
    CT_ALL | CT_TEXT,                                           // 001 ^A
    CT_ALL,                                                     // 002 ^B
    CT_ALL,                                                     // 003 ^C
    CT_ALL,                                                     // 004 ^D
    CT_ALL,                                                     // 005 ^E
    CT_ALL,                                                     // 006 ^F
    CT_ALL,                                                     // 007 ^G
    CT_ALL,                                                     // 008 ^H
    CT_ALL,                                                     // 009 ^I
    CT_ALL,                                                     // 010 ^J
    CT_ALL,                                                     // 011 ^K
    CT_ALL,                                                     // 012 ^L
    CT_ALL,                                                     // 013 ^M
    CT_ALL,                                                     // 014 ^N
    CT_ALL,                                                     // 015 ^O
    CT_ALL,                                                     // 016 ^P
    CT_ALL,                                                     // 017 ^Q
    CT_ALL,                                                     // 018 ^R
    CT_ALL,                                                     // 019 ^S
    CT_ALL,                                                     // 020 ^T
    CT_ALL,                                                     // 021 ^U
    CT_ALL,                                                     // 022 ^V
    CT_ALL,                                                     // 023 ^W
    CT_ALL,                                                     // 024 ^X
    CT_ALL,                                                     // 025 ^Y
    CT_ALL,                                                     // 026 ^Z
    CT_ALL,                                                     // 027 ^[
    CT_ALL,                                                     // 028 ^backslash
    CT_ALL,                                                     // 029 ^]
    CT_ALL,                                                     // 030 ^^
    CT_ALL,                                                     // 031 ^_
    CT_ALL | CT_TEXT,                                           // 032 ' '
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 033 '!'
    CT_ALL | CT_TEXT,                                           // 034 '"'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 035 '#'
    CT_ALL | CT_TEXT,                                           // 036 '$'
    CT_ALL | CT_TEXT,                                           // 037 '%'
    CT_ALL | CT_TEXT,                                           // 038 '&'
    CT_ALL | CT_TEXT,                                           // 039 '''
    CT_ALL | CT_TEXT,                                           // 040 '('
    CT_ALL | CT_TEXT,                                           // 041 ')'
    CT_ALL | CT_TEXT,                                           // 042 '*'
    CT_ALL | CT_TEXT,                                           // 043 '+'
    CT_ALL | CT_TEXT,                                           // 044 ','
    CT_ALL | CT_TEXT,                                           // 045 '-'
    CT_ALL | CT_TEXT,                                           // 046 '.'
    CT_ALL | CT_TEXT,                                           // 047 '/'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 048 '0'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 049 '1'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 050 '2'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 051 '3'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 052 '4'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 053 '5'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 054 '6'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 055 '7'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 056 '8'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 057 '9'
    CT_ALL | CT_TEXT,                                           // 058 ':'
    CT_ALL | CT_TEXT,                                           // 059 ';'
    CT_ALL | CT_TEXT,                                           // 060 '<'
    CT_ALL | CT_TEXT,                                           // 061 '='
    CT_ALL | CT_TEXT,                                           // 062 '>'
    CT_ALL | CT_TEXT,                                           // 063 '?'
    CT_ALL | CT_TEXT,                                           // 064 '@'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 065 'A'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 066 'B'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 067 'C'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 068 'D'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 069 'E'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 070 'F'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 071 'G'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 072 'H'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 073 'I'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 074 'J'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 075 'K'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 076 'L'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 077 'M'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 078 'N'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 079 'O'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 080 'P'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 081 'Q'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 082 'R'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 083 'S'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 084 'T'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 085 'U'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 086 'V'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 087 'W'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 088 'X'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 089 'Y'
    CT_ALL | CT_ALPHA | CT_TEXT | CT_KEYWORDS,                  // 090 'Z'
    CT_ALL | CT_TEXT,                                           // 091 '['
    CT_ALL | CT_TEXT,                                           // 092 '\'
    CT_ALL | CT_TEXT,                                           // 093 ']'
    CT_ALL | CT_TEXT,                                           // 094 '^'
    CT_ALL | CT_TEXT | CT_KEYWORDS,                             // 095 '_'
    CT_ALL | CT_TEXT,                                           // 096 '`'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 097 'a'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 098 'b'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 099 'c'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 100 'd'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 101 'e'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 102 'f'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 103 'g'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 104 'h'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 105 'i'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 106 'j'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 107 'k'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 108 'l'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 109 'm'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 110 'n'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 111 'o'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 112 'p'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 113 'q'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 114 'r'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 115 's'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 116 't'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 117 'u'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 118 'v'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 119 'w'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 120 'x'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 121 'y'
    CT_ALL | CT_ALPHA | CT_TEXT,                                // 122 'z'
    CT_ALL | CT_TEXT,                                           // 123 '{'
    CT_ALL | CT_TEXT,                                           // 124 '|'
    CT_ALL | CT_TEXT,                                           // 125 '}'
    CT_ALL | CT_TEXT,                                           // 126 '~'
    CT_ALL,                                                     // 127 ''
    CT_ALL,                                                     // 128 '€'
    CT_ALL,                                                     // 129 ''
    CT_ALL,                                                     // 130 '‚'
    CT_ALL,                                                     // 131 'ƒ'
    CT_ALL,                                                     // 132 '„'
    CT_ALL,                                                     // 133 '…'
    CT_ALL,                                                     // 134 '†'
    CT_ALL,                                                     // 135 '‡'
    CT_ALL,                                                     // 136 'ˆ'
    CT_ALL,                                                     // 137 '‰'
    CT_ALL,                                                     // 138 'Š'
    CT_ALL,                                                     // 139 '‹'
    CT_ALL,                                                     // 140 'Œ'
    CT_ALL,                                                     // 141 ''
    CT_ALL,                                                     // 142 'Ž'
    CT_ALL,                                                     // 143 ''
    CT_ALL,                                                     // 144 ''
    CT_ALL,                                                     // 145 '‘'
    CT_ALL,                                                     // 146 '’'
    CT_ALL,                                                     // 147 '“'
    CT_ALL,                                                     // 148 '”'
    CT_ALL,                                                     // 149 '•'
    CT_ALL,                                                     // 150 '–'
    CT_ALL,                                                     // 151 '—'
    CT_ALL,                                                     // 152 '˜'
    CT_ALL,                                                     // 153 '™'
    CT_ALL,                                                     // 154 'š'
    CT_ALL,                                                     // 155 '›'
    CT_ALL,                                                     // 156 'œ'
    CT_ALL,                                                     // 157 ''
    CT_ALL,                                                     // 158 'ž'
    CT_ALL,                                                     // 159 'Ÿ'
    CT_ALL,                                                     // 160 ' '
    CT_ALL,                                                     // 161 '¡'
    CT_ALL,                                                     // 162 '¢'
    CT_ALL,                                                     // 163 '£'
    CT_ALL,                                                     // 164 '¤'
    CT_ALL,                                                     // 165 '¥'
    CT_ALL,                                                     // 166 '¦'
    CT_ALL,                                                     // 167 '§'
    CT_ALL,                                                     // 168 '¨'
    CT_ALL,                                                     // 169 '©'
    CT_ALL,                                                     // 170 'ª'
    CT_ALL,                                                     // 171 '«'
    CT_ALL,                                                     // 172 '¬'
    CT_ALL,                                                     // 173 '­'
    CT_ALL,                                                     // 174 '®'
    CT_ALL,                                                     // 175 '¯'
    CT_ALL,                                                     // 176 '°'
    CT_ALL,                                                     // 177 '±'
    CT_ALL,                                                     // 178 '²'
    CT_ALL,                                                     // 179 '³'
    CT_ALL,                                                     // 180 '´'
    CT_ALL,                                                     // 181 'µ'
    CT_ALL,                                                     // 182 '¶'
    CT_ALL,                                                     // 183 '·'
    CT_ALL,                                                     // 184 '¸'
    CT_ALL,                                                     // 185 '¹'
    CT_ALL,                                                     // 186 'º'
    CT_ALL,                                                     // 187 '»'
    CT_ALL,                                                     // 188 '¼'
    CT_ALL,                                                     // 189 '½'
    CT_ALL,                                                     // 190 '¾'
    CT_ALL,                                                     // 191 '¿'
    CT_ALL,                                                     // 192 'À'
    CT_ALL,                                                     // 193 'Á'
    CT_ALL,                                                     // 194 'Â'
    CT_ALL,                                                     // 195 'Ã'
    CT_ALL,                                                     // 196 'Ä'
    CT_ALL,                                                     // 197 'Å'
    CT_ALL,                                                     // 198 'Æ'
    CT_ALL,                                                     // 199 'Ç'
    CT_ALL,                                                     // 200 'È'
    CT_ALL,                                                     // 201 'É'
    CT_ALL,                                                     // 202 'Ê'
    CT_ALL,                                                     // 203 'Ë'
    CT_ALL,                                                     // 204 'Ì'
    CT_ALL,                                                     // 205 'Í'
    CT_ALL,                                                     // 206 'Î'
    CT_ALL,                                                     // 207 'Ï'
    CT_ALL,                                                     // 208 'Ð'
    CT_ALL,                                                     // 209 'Ñ'
    CT_ALL,                                                     // 210 'Ò'
    CT_ALL,                                                     // 211 'Ó'
    CT_ALL,                                                     // 212 'Ô'
    CT_ALL,                                                     // 213 'Õ'
    CT_ALL,                                                     // 214 'Ö'
    CT_ALL,                                                     // 215 '×'
    CT_ALL,                                                     // 216 'Ø'
    CT_ALL,                                                     // 217 'Ù'
    CT_ALL,                                                     // 218 'Ú'
    CT_ALL,                                                     // 219 'Û'
    CT_ALL,                                                     // 220 'Ü'
    CT_ALL,                                                     // 221 'Ý'
    CT_ALL,                                                     // 222 'Þ'
    CT_ALL,                                                     // 223 'ß'
    CT_ALL,                                                     // 224 'à'
    CT_ALL,                                                     // 225 'á'
    CT_ALL,                                                     // 226 'â'
    CT_ALL,                                                     // 227 'ã'
    CT_ALL,                                                     // 228 'ä'
    CT_ALL,                                                     // 229 'å'
    CT_ALL,                                                     // 230 'æ'
    CT_ALL,                                                     // 231 'ç'
    CT_ALL,                                                     // 232 'è'
    CT_ALL,                                                     // 233 'é'
    CT_ALL,                                                     // 234 'ê'
    CT_ALL,                                                     // 235 'ë'
    CT_ALL,                                                     // 236 'ì'
    CT_ALL,                                                     // 237 'í'
    CT_ALL,                                                     // 238 'î'
    CT_ALL,                                                     // 239 'ï'
    CT_ALL,                                                     // 240 'ð'
    CT_ALL,                                                     // 241 'ñ'
    CT_ALL,                                                     // 242 'ò'
    CT_ALL,                                                     // 243 'ó'
    CT_ALL,                                                     // 244 'ô'
    CT_ALL,                                                     // 245 'õ'
    CT_ALL,                                                     // 246 'ö'
    CT_ALL,                                                     // 247 '÷'
    CT_ALL,                                                     // 248 'ø'
    CT_ALL,                                                     // 249 'ù'
    CT_ALL,                                                     // 250 'ú'
    CT_ALL,                                                     // 251 'û'
    CT_ALL,                                                     // 252 'ü'
    CT_ALL,                                                     // 253 'ý'
    CT_ALL,                                                     // 254 'þ'
    CT_ALL,                                                     // 255 'ÿ'
    };

CompNode *CompNode::AddSibling(unsigned char NewChar, int NewCode)
    {
    CompNode *CN;

    for (CN = this; CN->Sibling; CN = (CompNode *) MkPtr(CN->Sibling))
        {
        if (CN->TheChar == NewChar)
            {
            return (CN);
            }
        }

    if (CN->TheChar == NewChar)
        {
        return (CN);
        }

    CN->Sibling = MkSmPtr(new CompNode(NewChar, NewCode,
            ((CompNode *) MkPtr(CN->Parent))));

    return ((CompNode *) MkPtr(CN->Sibling));
    }

CompNode *CompNode::AddChild(unsigned char NewChar, int NewCode)
    {
    if (Child)
        {
        return (((CompNode *) MkPtr(Child))->AddSibling(NewChar, NewCode));
        }
    else
        {
        Child = MkSmPtr(new CompNode(NewChar, NewCode, this));

        return ((CompNode *) MkPtr(Child));
        }
    }

CompNode *CompNode::FindSibling(unsigned char ToFind)
    {
    CompNode *CN;

    for (CN = this; CN; CN = (CompNode *) MkPtr(CN->Sibling))
        {
        if (CN->TheChar == ToFind)
            {
            break;
            }
        }

    return (CN);
    }

CompNode *CompNode::FindChild(unsigned char ToFind)
    {
    CompNode *CN;

    for (CN = (CompNode *) MkPtr(this->Child); CN;
            CN = (CompNode *) MkPtr(CN->Sibling))
        {
        if (CN->TheChar == ToFind)
            {
            break;
            }
        }

    return (CN);
    }

void CompressionEngine::encode(CompNode *Node, int Flush)
    {
    for (int BitsWritten = 0; BitsWritten < BitCounter;)
        {
        int BitsToWrite = 8 - BitsInByteBuffer;

        if (BitsToWrite > BitCounter - BitsWritten)
            {
            BitsToWrite = BitCounter - BitsWritten;
            }

        ByteBuffer |= (unsigned char) (((Node->GetCode() >> BitsWritten) &
                (PowersOf2[BitsToWrite] - 1)) << BitsInByteBuffer);

        BitsInByteBuffer += BitsToWrite;
        BitsWritten += BitsToWrite;

        if (BitsInByteBuffer == 8)
            {
            WriteByte(ByteBuffer);
            BitsInByteBuffer = 0;
            ByteBuffer = 0;
            }
        }

    if (Flush && BitsInByteBuffer)
        {
        WriteByte(ByteBuffer);
        BitsInByteBuffer = 0;
        ByteBuffer = 0;
        }
    }

void CompressionEngine::FreeAllMemory(void)
    {
    if (Trie)
        {
        if (NodeArray)
            {
            CompNode *CN1, *CN2;

            CN1 = (CompNode *) MkPtr(NodeArray[0]);

            while (CN1)
                {
                CN2 = CN1;
                CN1 = CN1->GetNext();

                delete CN2;
                }
            }
        else
            {
            delete Trie;
            }
        }

    delete [] NodeArray;
    delete [] CompressionBuffer;
    }

CompressionEngine::~CompressionEngine(void)
    {
    FreeAllMemory();
    }

void CompressionEngine::ResetEngine(CEDataType DataType)
    {
    FreeAllMemory();

    Error = CEE_NOERROR;

    ByteBuffer = 0;
    BitsInByteBuffer = 0;

    BytesLeft = 0;

    Trie = new CompNode(0, 0, NULL);

    CurrentWriteNode = NULL;
    CurrentCmpNode = NULL;

    CB_Position = 0;
    CB_Length = 256;

    LastNode = NULL;
    NodesInArray = TillNextInArray = 0;

    NodeArray = new smPtr[NODE_ARRAY_SIZE];

    if (Trie && NodeArray)
        {
        unsigned char TypeMask;

        if (DataType == CEDT_KEYWORDS)
            {
            TypeMask = CT_KEYWORDS;
            }
        else if (DataType == CEDT_TEXT)
            {
            TypeMask = CT_TEXT;
            }
        else if (DataType == CEDT_ALPHA)
            {
            TypeMask = CT_ALPHA;
            }
        else
            {
            TypeMask = CT_ALL;
            }

        indexInArray(Trie);

        NextCode = 1;
        BitCounter = 0;

        for (int i = 1; i < 256 && Error == CEE_NOERROR; i++)
            {
            if (CharTypes[i] & TypeMask)
                {
                CompNode *CN = Trie->AddSibling((unsigned char) i, NextCode);

                if (CN)
                    {
                    indexInArray(CN);

                    if (NextCode++ >= PowersOf2[BitCounter])
                        {
                        BitCounter++;
                        }
                    }
                else
                    {
                    Error = CEE_OUTOFMEMORY;
                    }
                }

            }
        }
    else
        {
        Error = CEE_OUTOFMEMORY;
        }

    if (Error == CEE_NOERROR)
        {
        CompressionBuffer = new unsigned char[CB_Length];

        if (!CompressionBuffer)
            {
            Error = CEE_OUTOFMEMORY;
            }
        }
    }

CompressionEngine::CompressionEngine(CEDataType DataType)
    {
    Trie = NULL;
    NodeArray = NULL;
    CompressionBuffer = NULL;
    ResetEngine(DataType);
    }

void CompressionEngine::indexInArray(CompNode *NewNode)
    {
    if (LastNode)
        {
        LastNode->SetNext(NewNode);
        }

    if (!TillNextInArray--)
        {
        NodeArray[NodesInArray++] = MkSmPtr(NewNode);
        TillNextInArray = NODE_ARRAY_SPACING - 1;
        }

    LastNode = NewNode;
    }

const CompNode *CompressionEngine::findCode(unsigned int SearchCode) const
    {
    if (SearchCode >= NextCode)
        {
        return (NULL);
        }

    int PreviousIndex = SearchCode / NODE_ARRAY_SPACING;

    if (PreviousIndex >= NodesInArray)
        {
        return (NULL);
        }

    CompNode *cur;
    for (cur = (CompNode *) MkPtr(NodeArray[PreviousIndex]);
            cur && (cur->GetCode() < SearchCode);
            cur = cur->GetNext());

    return (cur);
    }

void CompressionEngine::expandCompressionBuffer(void)
    {
    const size_t NewSize = CB_Length + 256;
    unsigned char *NewBuffer = new unsigned char[NewSize];

    if (NewBuffer)
        {
        memcpy(NewBuffer, CompressionBuffer, CB_Length);

        CB_Length = NewSize;
        delete [] CompressionBuffer;
        CompressionBuffer = NewBuffer;
        }
    else
        {
        Error = CEE_OUTOFMEMORY;
        }
    }

void CompressionEngine::CompressByte(unsigned char ToCompress)
    {
    if (CurrentWriteNode)
        {
        CompNode *CN = CurrentWriteNode->FindChild(ToCompress);

        if (CN)
            {
            CurrentWriteNode = CN;
            }
        else
            {
            encode(CurrentWriteNode);

            for (size_t Poop = 0; Poop < CB_Position; Poop++)
                {
                addNode(CompressionBuffer[Poop]);
                }
            CB_Position = 0;

            CurrentWriteNode = Trie->FindSibling(ToCompress);
            }
        }
    else
        {
        CurrentWriteNode = Trie->FindSibling(ToCompress);
        }

    if (!CurrentWriteNode)
        {
printf("couldn't find '%d'\n",ToCompress);
        Error = CEE_BADDATA;
        }

    if (CB_Position >= CB_Length)
        {
        expandCompressionBuffer();
        }

    if (CB_Position < CB_Length)
        {
        CompressionBuffer[CB_Position++] = ToCompress;
        }
    }

void CompressionEngine::addNode(unsigned char NewChar)
    {
    if (CurrentCmpNode)
        {
        CompNode *CN = CurrentCmpNode->FindChild(NewChar);

        if (CN)
            {
            CurrentCmpNode = CN;
            }
        else
            {
            if (NextCode < MAX_NODES)
                {
                CN = CurrentCmpNode->AddChild(NewChar, NextCode);

                if (CN)
                    {
                    indexInArray(CN);

                    if (NextCode++ >= PowersOf2[BitCounter])
                        {
                        BitCounter++;
                        }
                    }
                else
                    {
                    Error = CEE_OUTOFMEMORY;
                    }
                }

            CurrentCmpNode = Trie->FindSibling(NewChar);
            }
        }
    else
        {
        CurrentCmpNode = Trie->FindSibling(NewChar);
        }
    }

const unsigned char *CompressionEngine::Decompress(int *BytesRetrieved)
    {
    if (BytesLeft)
        {
        int ReadValue = 0;

        for (int BitsRead = 0; BitsRead < BitCounter;)
            {
            if (!BitsInByteBuffer)
                {
                ByteBuffer = GetByte();
                BitsInByteBuffer = 8;
                }

            int BitsToRead = BitsInByteBuffer;

            if (BitsToRead > BitCounter - BitsRead)
                {
                BitsToRead = BitCounter - BitsRead;
                }

            ReadValue |= (ByteBuffer & (PowersOf2[BitsToRead] - 1)) <<
                    BitsRead;

            BitsInByteBuffer -= BitsToRead;
            ByteBuffer = (unsigned char) (ByteBuffer >> BitsToRead);

            BitsRead += BitsToRead;
            }

        const CompNode *CN = findCode(ReadValue);

        if (!CN)
            {
            Error = CEE_BADDATA;
            BytesLeft = 0;
            *BytesRetrieved = 0;

            return (NULL);
            }
        else
            {
            CB_Position = 0;

            for (const CompNode *CN2 = CN; CN2; CN2 = CN2->GetParent())
                {
                if (CB_Position >= CB_Length)
                    {
                    expandCompressionBuffer();
                    }

                if (CB_Position < CB_Length)
                    {
                    CompressionBuffer[CB_Position++] = CN2->GetChar();
                    }
                }
            CompressionBuffer[CB_Position] = 0;

            for (size_t Poop = 0; Poop < CB_Position / 2; Poop++)
                {
                const unsigned char SChar = CompressionBuffer[Poop];
                CompressionBuffer[Poop] =
                        CompressionBuffer[CB_Position - 1 - Poop];
                CompressionBuffer[CB_Position - 1 - Poop] = SChar;
                }
EXTRA_OPEN_BRACE
            for (size_t Poop = 0; Poop < CB_Position; Poop++)
                {
                addNode(CompressionBuffer[Poop]);
                }
EXTRA_CLOSE_BRACE
            BytesLeft -= CB_Position;
            *BytesRetrieved = CB_Position;

            return (CompressionBuffer);
            }
        }
    else
        {
        *BytesRetrieved = 0;
        return (NULL);
        }
    }
