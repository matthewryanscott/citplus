// --------------------------------------------------------------------------
// Citadel: Discard.CPP
//
// This file contains all the code to deal with discardable data

#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// checkDataVer 	checks version of CTDL.DAT
// readData 		reads from CTDL.DAT
// discardData		discards memory


#include "comp.h"
#include "compx.h"

class DataDecompressor : CompressionEngine
	{
	uchar *Ptr;

public:
	DataDecompressor(CEDataType DataType) : CompressionEngine(DataType)
		{
		Ptr = NULL;
		}

	CEError GetError(void)
		{
		return (CompressionEngine::GetError());
		}

	virtual void WriteByte(uchar ToWrite) { ToWrite = ToWrite; }

	virtual uchar GetByte(void)
		{
		return (*(Ptr++));
		}

	long Decompress(void **OriginalData);
	};

long DataDecompressor::Decompress(void **OriginalData)
	{
	Ptr = (uchar *) *OriginalData;

	long FullSize = *((long *) Ptr);

	if (FullSize > (ulong) UINT_MAX)
		{
		return (0);
		}

	uchar *FullData = new uchar[(size_t) FullSize];

	if (!FullData)
		{
		return (0);
		}

	SetBytesLeft(FullSize);

	Ptr += sizeof(long) / sizeof(char);

	const uchar *Buffer;
	int BytesRead, FDPos = 0;

	while ((Buffer = CompressionEngine::Decompress(&BytesRead)) != NULL)
		{
//printf("Read: %d, Left: %ld, (%s)\n", BytesRead, GetBytesLeft(), Buffer);
		int i = 0;

//printf("Read: %d, Left: %ld, (%s)\n", BytesRead, GetBytesLeft(), Buffer);
		while (BytesRead--)
			{
			FullData[FDPos + i] = Buffer[i];
			i++;
			}

		FDPos += i;
		}

	delete [] *OriginalData;
	*OriginalData = FullData;
//if (GetError()){
//printf("Read: %d, Left: %ld, (%s)\n", BytesRead, GetBytesLeft(), Buffer);
//getch();
//}

	return (GetError() == CEE_NOERROR ? FullSize : 0);
	}

class DataDecompressorX : CompressionEngineX
	{
	char *Ptr;
	char *FullData, *Ptr2;

	virtual int readByte(char *Read)
		{
		*Read = *(Ptr++);
		return (TRUE);
		}

	virtual void writeByte(char ToWrite)
		{
		*(Ptr2++) = ToWrite;
		}

	virtual int isEndOfInput(void) { return (TRUE); }

public:

	long Decompress(void **OriginalData);
	};

long DataDecompressorX::Decompress(void **OriginalData)
	{
	Ptr = (char *) *OriginalData;

	long FullSize = *((long *) Ptr);

	if (FullSize > (ulong) UINT_MAX)
		{
		return (0);
		}

	Ptr2 = FullData = new char[(size_t) FullSize];

	if (!FullData)
		{
		return (0);
		}

	Ptr += sizeof(long) / sizeof(char);

	DecompressIt(FullSize);

	delete [] *OriginalData;
	*OriginalData = FullData;

	return (FullSize);
	}

static ulong dataOffset;


// --------------------------------------------------------------------------
// checkDataVer(): checks version of CTDL.DAT.

long checkDataVer(void)
	{
	FILE *f;

	char pathToCtdlDat[128];
	strcpy(pathToCtdlDat, fullExePath);

	char *ptr = strrchr(pathToCtdlDat, '\\');

	if (ptr)
		{
		strcpy(ptr + 1, ctdlDat);
		}
	else
		{
		strcpy(pathToCtdlDat, ctdlDat);
		}

	if (citOpen(pathToCtdlDat, CO_RB, &f))
		{
		long l;

		fread(&l, sizeof(l), 1, f);
		fclose(f);

		dataOffset = sizeof(long);

		return (l);
		}
	else
		{
		// no CTDL.DAT - use what is in the .EXE...
		if (citOpen(fullExePath, CO_RB, &f))
			{
			long l, sig;

			fseek(f, 0, SEEK_END);
			l = ftell(f) - sizeof(l);

			do
				{
				fseek(f, l--, SEEK_SET);
				fread(&sig, sizeof(sig), 1, f);
				} while (sig != 0x69696969l && l > 620000l); // 620000l: Kludge

			if (sig == 0x69696969l)
				{
				l++;
				l -= sizeof(l);
				fseek(f, l, SEEK_SET);
				fread(&l, sizeof(l), 1, f);
				dataOffset = l + sizeof(long);
				fseek(f, l, SEEK_SET);
				fread(&l, sizeof(l), 1, f);
				}
			else
				{
				l = CERROR;
				}

			fclose(f);

			return (l);
			}
		else
			{
			return (CERROR);
			}
		}
	}


// --------------------------------------------------------------------------
// readData(): Reads from CTDL.DAT.

discardable *readData(int Entry, int StartSubEntry, int EndSubEntry)
	{
	assert(StartSubEntry >= 0);
	assert(EndSubEntry >= StartSubEntry);

	FILE *f;
	long l;

	compactMemory();

	if (!dataOffset)
		{
		return (NULL);
		}
	else if (dataOffset == sizeof(long))
		{
		char pathToCtdlDat[128];
		strcpy(pathToCtdlDat, fullExePath);

		char *ptr = strrchr(pathToCtdlDat, '\\');

		if (ptr)
			{
			strcpy(ptr + 1, ctdlDat);
			}
		else
			{
			strcpy(pathToCtdlDat, ctdlDat);
			}

		if (!citOpen(pathToCtdlDat, CO_RB, &f))
			{
			return (NULL);
			}
		}
	else 
		{
		if (!citOpen(fullExePath, CO_RB, &f))
			{
			return (NULL);
			}
		}

	fseek(f, dataOffset, SEEK_SET);
	fread(&l, sizeof(l), 1, f);

	if (Entry < l)
		{
		discardable *base = NULL, *cur;

		fseek(f, sizeof(l) * Entry, SEEK_CUR);
		fread(&l, sizeof(l), 1, f);
		fseek(f, l + dataOffset - sizeof(long), SEEK_SET);

		fread(&l, sizeof(l), 1, f);
		int SubCounter = 0;

		for (; l; l--)
			{
			if (++SubCounter > StartSubEntry)
				{
				if (SubCounter - 1 > EndSubEntry)
					{
					break;
					}

				cur = (discardable *) addLL((void **) &base, sizeof(* base));

				if (cur)
					{
#ifdef WINCIT
					// the data file is created by a 16-bit program. The
					// 16-bit compiler uses 16-bit integers to hold enums.
					// The 32-bit compiler uses 32-bit integers to hold enums.
					fread(&cur->type, sizeof(short), 1, f);
#else
					fread(&cur->type, sizeof(cur->type), 1, f);
#endif
					fread(&cur->length, sizeof(cur->length), 1, f);

					cur->data = new char[(uint) cur->length];
					if (cur->data)
						{
						fread(cur->data, (uint) cur->length, 1, f);

						if (cur->type == DD_STRCMP ||
								cur->type == DD_STRCMPK ||
								cur->type == DD_STRCMPT ||
								cur->type == DD_STRCMPA)
							{
							DataDecompressor DDcmp(
								cur->type == DD_STRCMP ? CEDT_NORMAL :
								cur->type == DD_STRCMPK ? CEDT_KEYWORDS :
								cur->type == DD_STRCMPA ? CEDT_ALPHA :
								CEDT_TEXT);

							if (DDcmp.GetError() != CEE_NOERROR)
								{
								discardData(base);
								fclose(f);
								return (NULL);
								}

							long FullSize = DDcmp.Decompress(&(cur->data));

							if (!FullSize)
								{
//cPrintf("%u !1\n", (uint)cur->length);
								discardData(base);
								fclose(f);
								return (NULL);
								}

							cur->type = DD_STRING;
							cur->length = FullSize;
							}
						else if (cur->type == DD_STRCMPX)
							{
							long FullSize;
							DataDecompressorX *DDcmp = new DataDecompressorX;

							if (DDcmp)
								{
								FullSize = DDcmp->Decompress(&(cur->data));

								cur->type = DD_STRING;
								cur->length = FullSize;

								delete DDcmp;
								}
							else
								{
								FullSize = 0;
								}

							if (!FullSize)
								{
								discardData(base);
								fclose(f);
								return (NULL);
								}
							}
						else if (cur->type == DD_RAWDATAX)
							{
							long FullSize;
							DataDecompressorX *DDcmp = new DataDecompressorX;

							if (DDcmp)
								{
								FullSize = DDcmp->Decompress(&(cur->data));

								cur->type = DD_RAWDATA;
								cur->length = FullSize;

								delete DDcmp;
								}
							else
								{
								FullSize = 0;
								}

							if (!FullSize)
								{
								discardData(base);
								fclose(f);
								return (NULL);
								}
							}

						switch (cur->type)
							{
							case DD_STRING:
								{
								uint i, j;
								char *c;

								for (c = (char *) cur->data, i = 0, j = 0;
										(long) i < cur->length; i++, c++)
									{
									if (!(*c))
										{
										j++;
										}
									}

								cur->aux = new char*[j];

								if (cur->aux)
									{
									Bool k;

									for (c = (char *) cur->data, i = j = 0, k = TRUE;
											(long) i < cur->length; i++, c++)
										{
										if (k)
											{
											((char **) cur->aux)[j++] = c;
											k = FALSE;
											}

										if (!*c)
											{
											k = TRUE;
											}
										}
									}
								else
									{
									discardData(base);
									fclose(f);
									return (NULL);
									}

								break;
								}
							}
						}
					else
						{
						discardData(base);
						fclose(f);
						return (NULL);
						}
					}
				else
					{
					discardData(base);
					fclose(f);
					return (NULL);
					}
				}
			else
				{
				discardable d;

#ifdef WINCIT
				// the data file is created by a 16-bit program. The
				// 16-bit compiler uses 16-bit integers to hold enums.
				// The 32-bit compiler uses 32-bit integers to hold enums.
				fread(&d.type, sizeof(short), 1, f);
#else
				fread(&d.type, sizeof(d.type), 1, f);
#endif
				fread(&d.length, sizeof(d.length), 1, f);
				fseek(f, d.length, SEEK_CUR);
				}
			}

		fclose(f);
		return (base);
		}
	else
		{
		fclose(f);
		return (NULL);
		}
	}


// --------------------------------------------------------------------------
// discardData(): Discards memory.

void discardData(discardable *data)
	{
	discardable *cur;

	for (cur = data; cur; cur = (discardable *) getNextLL(cur))
		{
		delete [] cur->data;
		delete [] cur->aux;
		}

	disposeLL((void **) &data);
	}
