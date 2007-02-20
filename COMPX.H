/*
compx - a sliding window compression algorithm. compresses really slowly,
decompresses really quickly.
*/

#define WINDOWBITS		12
#define WINDOWLENGTH	4096		// 2^WINDOWBITS

#define RUNBITS			4
#define RUNLENGTH		16			// 2^RUNLENGTH
#define MIN_RUN_LENGTH	2			// Shortest to bother saving as a run.

#define AS_RUN			1			// How we save
#define	AS_BYTE			0


class CompressionEngineX
	{
	char Window[WINDOWLENGTH];
	int InWindow, StartWindow;

	char Run[RUNLENGTH];
	int InRun;

	char ByteBuffer;
	int BitsInByteBuffer;

	long BytesLeft;
	long UncompressedLength;

	virtual int readByte(char *Read) = 0;
	virtual void writeByte(char ToWrite) = 0;
	virtual int isEndOfInput(void) = 0;
	
	const char *Decompress(int *Length);
	void writeBits(int NumBits, int Value);
	int readBits(int NumBits);
	void saveData(void);

public:
	long GetUncompressedLength(void)
		{
		return (UncompressedLength);
		}

	void ResetEngine(void)
		{
		ByteBuffer = 0;
		BitsInByteBuffer = InWindow = StartWindow = InRun = 0;
		UncompressedLength = 0;
		}

	CompressionEngineX(void)
		{
		ResetEngine();
		}

	void CompressIt(void);
	void DecompressIt(long Bytes);
	};
