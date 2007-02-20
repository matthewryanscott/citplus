#ifndef BITBAG_H
#define BITBAG_H
/* These nice bitbag Routines were STOLEN from Stonenehenge source code */
/* copyrighted	by Megalithic Microproducts Ltd.						*/
/* If you would like to sue me for copyright infringment go ahead:		*/
/* my info: 															*/
/*			Matthew S. Pfleger											*/
/*			11201 Algonquin Rd											*/
/*			Edmonds, WA 98020											*/
/*			Ph: 206-628-3810											*/

// Though they have been converted to C++ since then.

class BitBag
	{
	char *Data;
	uint Number;

public:
	size_t GetSize(void) const
		{
		return (sizeof(char) * (Number + (CHAR_BIT - 1)) / (CHAR_BIT));
		}

	uint GetNumberOfBits(void) const
		{
		return (Number);
		}

	Bool IsValid(void) const
		{
		return (Data != NULL);
		}

	void Clear(void)
		{
		assert(this);
		VerifyHeap();

		if (IsValid() && Number)
			{
			memset(Data, 0, GetSize());
			VerifyHeap();
			}
		}

	BitBag(uint NumBits)
		{
		assert(this);
		VerifyHeap();
		Number = NumBits;

		Data = new char[GetSize()];
		Clear();
		}

	~BitBag(void)
		{
		assert(this);
		VerifyHeap();
		delete [] Data;
		VerifyHeap();
		}

	BitBag(const BitBag &Original)
		{
		assert(this);
		VerifyHeap();

		Number = Original.Number;
		Data = new char[GetSize()];

		if (IsValid() && Original.IsValid() && Number)
			{
			memcpy(Data, Original.Data, GetSize());
			}

		VerifyHeap();
		}

	BitBag& operator =(const BitBag &Original)
		{
		assert(this);
		VerifyHeap();

		delete [] Data;

		Number = Original.Number;
		Data = new char[GetSize()];

		if (IsValid() && Original.IsValid() && Number)
			{
			memcpy(Data, Original.Data, GetSize());
			}

		VerifyHeap();
		return (*this);
		}

	void Resize(uint NewSize)
		{
		assert(this);
		VerifyHeap();

		delete [] Data;

		Number = NewSize;

		Data = new char[GetSize()];

		Clear();
		}

	void BitSet(uint BitNumber, Bool SetOrClear)
		{
		assert(this);
		assert(BitNumber < Number);

		if (IsValid())
			{
			if (SetOrClear)
				{
				Data[BitNumber / CHAR_BIT] |=
						(char) (1 << (BitNumber & (CHAR_BIT - 1)));
				}
			else
				{
				Data[BitNumber / CHAR_BIT] &=
						(char) ~(1 << (BitNumber & (CHAR_BIT - 1)));
				}
			}
		}

	Bool BitTest(uint BitNumber) const
		{
		assert(this);
		assert(BitNumber < Number);

		if (IsValid())
			{
			return (Data[BitNumber / CHAR_BIT] & (1 << (BitNumber & (CHAR_BIT - 1))));
			}
		else
			{
			return (FALSE);
			}
		}

	const void *GetPointer(void) const
		{
		assert(this);
		return (Data);
		}

	// NonConst...
	void *GetNCPointer(void)
		{
		assert(this);
		return (Data);
		}
	};
#endif
