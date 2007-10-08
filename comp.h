/*
	This implements a general-purpose data compression engine class. It has
	adequate performance for most uses. I am pretty sure that the algorithm
	used is Limpel-Ziv, or at least highly similar. (I wrote this over a year
	after last reading about data compression algorithms, and did a better
	job at remembering the algorithm than who developed it.)

	The CompressionEngine class contains the basics, but has no interface
	to the outside world, such as a method to read and write the data. To
	use this, derive a class, and define the OpenStream(), CloseStream(),
	GetByte() and WriteByte() member functions. 
*/

#include <stdlib.h>

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif


/*
	This cuts the memory requirement of compressor by half under Borland C++.
	Because I know that the offset portion of all far pointers returned by
	the new operator is 0004, I can get away with only storing the segment
	when I need to store a far pointer returned by the new operator. This is
	a bit messy, but the memory gains are extreme.
*/
#if defined(__BORLANDC__) && !defined(WINCIT)
	#include <dos.h>

	typedef unsigned int	smPtr;

	#define	BC_OFFSET	4

	#define MkPtr(in)	(in ? MK_FP(in, BC_OFFSET) : NULL)
	#define MkSmPtr(in)	(FP_SEG(in))
#else
	typedef void *smPtr;

	#define MkPtr(in)	(in)
	#define MkSmPtr(in)	(in)
#endif


/*
	This class is used for each node of the trie used by the engine.
*/
class CompNode
	{
	smPtr Child;				// Down the trie
	smPtr Sibling;				// Across the trie
	smPtr Parent;				// Up the trie

	unsigned char TheChar;		// The character that this node represents.
	unsigned int Code;			// The code for this node.

	smPtr Next;					// Separate from the trie: a linked list of
								// the nodes on the trie. this is used by the
								// CompressionEngine class for quick look-up
								// and destroying the trie.

public:
	CompNode(unsigned char NewChar, unsigned int NewCode, CompNode *NewParent)
		{
		Child = MkSmPtr(NULL);
		Sibling = MkSmPtr(NULL);
		Parent = MkSmPtr(NewParent);
		Next = MkSmPtr(NULL);
		TheChar = NewChar;
		Code = NewCode;
		}

	const unsigned int GetCode(void) const
		{
		return (Code);
		}

	unsigned char GetChar(void) const
		{
		return (TheChar);
		}

	const CompNode *GetParent(void) const
		{
		return ((const CompNode *) MkPtr(Parent));
		}

	CompNode *GetNext(void)
		{
		return ((CompNode *) MkPtr(Next));
		}

	void SetNext(CompNode *CN)
		{
		Next = MkSmPtr(CN);
		}

	CompNode *AddSibling(unsigned char NewChar, int NewCode);
	CompNode *AddChild(unsigned char NewChar, int NewCode);
	CompNode *FindSibling(unsigned char ToFind);
	CompNode *FindChild(unsigned char ToFind);
	};



/*
	Here is the CompressionEngine itself.
*/
#define MAX_NODES			USHRT_MAX
#define NODE_ARRAY_SPACING	8
#define NODE_ARRAY_SIZE		(MAX_NODES / NODE_ARRAY_SPACING)

enum CEDataType
	{
	CEDT_NORMAL,	CEDT_KEYWORDS,		CEDT_TEXT,		CEDT_ALPHA,
	};

enum CEError
	{
	CEE_NOERROR,	CEE_OUTOFMEMORY,	CEE_BADDATA,	CEE_CANNOTWRITE,
	CEE_CANNOTREAD,
	};

class CompressionEngine
	{
	CompNode *Trie;				// Dictionary data
	CompNode *CurrentWriteNode;	// One to write
	CompNode *CurrentCmpNode;	// One to add to trie

	unsigned int NextCode;		// Code for next node in dictionary
	unsigned int BitCounter;	// Current dictionary width

	CEError Error;				// Any errors

	long BytesLeft;				// Bytes left to decompress.

	unsigned char *CompressionBuffer;// What needs to be compressed or has been
	size_t CB_Position;			// decompressed. This buffer can grow in
	size_t CB_Length;			// usage, though this is very rare.

	smPtr *NodeArray;			// For looking up nodes when decompressing.
	unsigned int NodesInArray;	// ...
	unsigned int TillNextInArray;// ...
	CompNode *LastNode;			// ...

	unsigned char ByteBuffer;	// Read, or to be written.
	int BitsInByteBuffer;		// ...

	void addNode(unsigned char NewChar);
	void indexInArray(CompNode *NewNode);
	void encode(CompNode *Node, int Flush = FALSE);
	const CompNode *findCode(unsigned int SearchCode) const;
	void expandCompressionBuffer(void);
	void FreeAllMemory(void);

public:
	CEError GetError(void)
		{
		return (Error);
		}

	void SetBytesLeft(long New)
		{
		BytesLeft = New;
		}

	long GetBytesLeft(void)
		{
		return (BytesLeft);
		}

	void FlushOutput(void)
		{
		encode(CurrentWriteNode, TRUE);
		}

	CompressionEngine(CEDataType DataType);
	~CompressionEngine(void);
	void ResetEngine(CEDataType DataType);
	void CompressByte(unsigned char ToCompress);
	const unsigned char *Decompress(int *BytesRetrieved);

	virtual unsigned char GetByte(void) = 0;
	virtual void WriteByte(unsigned char ToWrite) = 0;
	};
