#ifdef PORTABLE
#include <stdio.h>
#include <string.h>
#else
#ifdef VISUALC
#include <stdio.h>
#include <string.h>
#include <memory.h>
#else
#include <mem.h>
#endif
#endif
#include "compx.h"


static unsigned int PowersOf2Minus1[] =
	{
	0u, 	1u, 	3u, 	7u, 	15u,	31u,	63u,	127u,
	255u,	511u,	1023u,	2047u,	4095u,	8191u,	16383u, 32767u,
	65535u,
	};

int CompressionEngineX::readBits(int NumBits)
	{
	int ReadValue = 0;

	for (int BitsRead = 0; BitsRead < NumBits;)
		{
		if (!BitsInByteBuffer)
			{
			readByte(&ByteBuffer);
			BitsInByteBuffer = 8;
			}

		int BitsToRead = BitsInByteBuffer;

		if (BitsToRead > NumBits - BitsRead)
			{
			BitsToRead = NumBits - BitsRead;
			}

		ReadValue |= (ByteBuffer & (PowersOf2Minus1[BitsToRead])) << BitsRead;

		BitsInByteBuffer -= BitsToRead;
		ByteBuffer = (char) (ByteBuffer >> BitsToRead);

		BitsRead += BitsToRead;
		}

	return (ReadValue);
	}

void CompressionEngineX::writeBits(int NumBits, int Value)
	{
	for (int BitsWritten = 0; BitsWritten < NumBits;)
		{
		int BitsToWrite = 8 - BitsInByteBuffer;

		if (BitsToWrite > NumBits - BitsWritten)
			{
			BitsToWrite = NumBits - BitsWritten;
			}

		ByteBuffer |= (char) (((Value >> BitsWritten) &
				(PowersOf2Minus1[BitsToWrite])) << BitsInByteBuffer);

		BitsInByteBuffer += BitsToWrite;
		BitsWritten += BitsToWrite;

		if (BitsInByteBuffer == 8)
			{
			writeByte(ByteBuffer);
			BitsInByteBuffer = 0;
			ByteBuffer = 0;
			}
		}
	}

void CompressionEngineX::DecompressIt(long Bytes)
	{
	BytesLeft = Bytes;

	const char *Buffer;
	int BytesRead;

	while ((Buffer = Decompress(&BytesRead)) != NULL)
		{
		int bPos = 0;

		while (BytesRead--)
			{
			writeByte(Buffer[bPos++]);
			}
		}
	}

void CompressionEngineX::CompressIt(void)
	{
	while (!isEndOfInput())
		{
		char ToAdd;

		if (readByte(&ToAdd))
			{
			UncompressedLength++;

			Run[InRun++] = ToAdd;

			if (InRun >= RUNLENGTH)
				{
				saveData();
				}
			}
		}

	while (InRun)
		{
		saveData();
		}

	if (BitsInByteBuffer)
		{
		writeByte(ByteBuffer);
		BitsInByteBuffer = 0;
		ByteBuffer = 0;
		}
	}

void CompressionEngineX::saveData(void)
	{
	class DataRun
		{
		int Offset, Length;

	public:
		DataRun(void)
			{
			Offset = Length = 0;
			}

		int GetLength(void)
			{
			return (Length);
			}

		int GetOffset(void)
			{
			return (Offset);
			}

		void Set(int NewOff, int NewLen)
			{
			Offset = NewOff;
			Length = NewLen;
			}
		} LongestRun;

	for (int Offset = 0; Offset < InWindow; Offset++)
		{
		if (Window[Offset] == Run[0])
			{
			int RunPos = 1;

			while (
					(RunPos < InRun) &&
					(Window[(Offset + RunPos) % InWindow] == Run[RunPos])
					)
				{
				RunPos++;
				}

			if (RunPos > LongestRun.GetLength())
				{
				LongestRun.Set(Offset, RunPos);

				if (RunPos == InRun)
					{
					break;
					}
				}
			}
		}

	int BytesToMoveToWindow;

	if (LongestRun.GetLength() >= MIN_RUN_LENGTH)
		{
		// Save run
		BytesToMoveToWindow = LongestRun.GetLength();

		writeBits(1, AS_RUN);
		writeBits(WINDOWBITS, LongestRun.GetOffset());
		writeBits(RUNBITS, LongestRun.GetLength() - 1);
		}
	else
		{
		// Save byte
		BytesToMoveToWindow = 1;

		writeBits(1, AS_BYTE);
		writeBits(8, Run[0]);
		}

	for (int BytesMoved = 0; BytesMoved < BytesToMoveToWindow; BytesMoved++)
		{
		Window[StartWindow++] = Run[BytesMoved];

		if (StartWindow > InWindow)
			{
			InWindow = StartWindow;
			}

		StartWindow &= WINDOWLENGTH - 1;
		}

	memmove(Run, Run + BytesToMoveToWindow,
			RUNLENGTH - BytesToMoveToWindow);
	InRun -= BytesToMoveToWindow;
	}

const char *CompressionEngineX::Decompress(int *Length)
	{
#ifndef USEASM
	if (BytesLeft > 0)
		{
		if (readBits(1) == AS_BYTE)
			{
			Run[0] = (char) readBits(8);
			*Length = 1;
			}
		else
			{
			int Offset = readBits(WINDOWBITS);
			*Length = readBits(RUNBITS) + 1;

			for (int i = 0; i < *Length; i++)
				{
				Run[i] = Window[(Offset + i) % InWindow];
				}
			}

		for (int BytesMoved = 0; BytesMoved < *Length; BytesMoved++)
			{
			Window[StartWindow++] = Run[BytesMoved];

			if (StartWindow > InWindow)
				{
				InWindow = StartWindow;
				}

			StartWindow &= WINDOWLENGTH - 1;
			}

		BytesLeft -= *Length;
		return (Run);
		}
	else
		{
		*Length = 0;
		return (NULL);
		}
#else
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx not done yet
	asm cmp BytesLeft, 0
	asm jle NoMore

	readBits(1);
	asm cmp ax, AS_BYTE
	asm jne NotByte

	readBits(8);
	mov [Run], al;
	*Length = 1;

	asm jmp short UpdateWindow

NotByte:
	readBits(WINDOWBITS);
	asm mov bx, ax

	readBits(RUNBITS);
	asm inc ax
	asm mov cx, ax
	*Length = ax;

	asm push di
	asm push si
	asm push ds

	asm mov di, word ptr [Run]
	asm add di, bx
	asm push ds
	asm pop es

	asm mov si, word ptr [Window]
	asm mov ax, word ptr [Window + 2]
	asm mov ds, ax

	asm mov dx, cx

CopyRunLoop:
	asm movsb

	asm inc dx
	asm cmp dx, InWindow

			*Length = readBits(RUNBITS) + 1;

			for (int i = 0; i < *Length; i++)
				{
				Run[i] = Window[(Offset + i) % InWindow];
				}
			}

NoMore:

#endif
	}
