// --------------------------------------------------------------------------
// Citadel: Andy.CPP
//
// This is Andy's function

#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// Andy()				This is it


// --------------------------------------------------------------------------
// Andy(): This is it.
//
// Notes:
//	This code is based highly on Andy's mdformat function, which I guess he
//	wrote for Elegia. I updated it to work with Citadel's output routines and
//	added '^' and '\' as special characters. I also put in the %[...]x stuff.

int TERMWINDOWMEMBER Brent(const char **Format, int *colCount, char *collect)
	{
	char thisFormat = **Format;
	char toOutput = thisFormat;

	if (thisFormat == '^')
		{
		toOutput = *(++(*Format));

		if (toupper(toOutput) >= 'A' && toupper(toOutput) <= 'Z')
			{
			toOutput = (char) (toupper(toOutput) - 64);
			}

		}
	else if (thisFormat == '\\')
		{
		toOutput = *(++(*Format));

		switch (toupper(toOutput))
			{
			case 'N':
				{
				if (*colCount)
					{
					collect[*colCount] = 0;
					mFormat(collect);
					*colCount = 0;
					}

				OC.Formatting = TRUE;
				doCR();
				OC.Formatting = FALSE;

				toOutput = 0;
				break;
				}

			case 'T':
				{
				toOutput = '\t';
				break;
				}

			case 'B':
				{
				toOutput = '\b';
				break;
				}
			}
		}

	if (toOutput)
		{
		collect[(*colCount)++] = toOutput;
		}

	return (**Format);
	}

void cdecl TERMWINDOWMEMBER Andy(const char *Format, const char *Codes, ...)
	{
	char *Vals[32], collect[512];
	const char *Code;
	int n, CodeCount, colCount;
	va_list ap;

	va_start(ap, Codes);

	for (n = 0, CodeCount = strlen(Codes); n < CodeCount && n < 32; ++n)
		{
		Vals[n] = va_arg(ap, char *);
		}

	va_end(ap);

	for (colCount = 0; *Format != '\0'; ++Format)
		{
		if (*Format != '%')
			{
			if (!Brent(&Format, &colCount, collect))
				{
				break;
				}
			continue;
			}

		if (*(++Format) == '\0')
			{
			break;
			}

		if (*Format == '%')
			{
			collect[colCount++] = *Format;
			continue;
			}

		// not stuff
		if (*Format == '[')
			{
			// Scan for close and include-location.
			for (Code = Format; *Code != 0 && *Code != ']'; ++Code);

			if (*Code == '\0')
				{
				break;
				}

			if ((n = strpos(*(Code + 1), Codes)) != 0)
				{
				if (Vals[n - 1][0] == '\0')
					{
					for (++Format; Format < Code; ++Format)
						{
						if (!Brent(&Format, &colCount, collect))
							{
							break;
							}
						}

					if (*Format)
						{
						Format++;
						}
					}
				else
					{
					Format = Code + 1;
					}
				}

			continue;
			}

		if (*Format != '{')
			{
			if ((n = strpos(*Format, Codes)) != 0)
				{
				if (colCount)
					{
					collect[colCount] = 0;
					mFormat(collect);
					colCount = 0;
					}

				mFormat(Vals[n - 1]);
				}

			continue;
			}

		// Scan for close and include-location.
		const char *Include = NULL;
		for (Code = Format; *Code != 0 && *Code != '}'; ++Code)
			{
			if (*Code == '&')
				{
				Include = Code;
				}
			}

		if (*Code == '\0')
			{
			break;
			}

		if (Include == NULL)
			{
			if ((n = strpos(*(Code + 1), Codes)) != 0)
				{
				if (Vals[n - 1][0] != '\0')
					{
					if (colCount)
						{
						collect[colCount] = 0;
						mFormat(collect);
						colCount = 0;
						}

					for (++Format; Format < Code; ++Format)
						{
						if (!Brent(&Format, &colCount, collect))
							{
							break;
							}
						}

					if (*Format)
						{
						Format++;
						}
					}
				else
					{
					Format = Code + 1;
					}
				}
			}
		else
			{
			if ((n = strpos(*(Code + 1), Codes)) != 0)
				{
				if (Vals[n - 1][0] != '\0')
					{
					if (colCount)
						{
						collect[colCount] = 0;
						mFormat(collect);
						colCount = 0;
						}

					for (++Format; Format < Include; ++Format)
						{
						if (!Brent(&Format, &colCount, collect))
							{
							break;
							}
						}

					if (colCount)
						{
						collect[colCount] = 0;
						mFormat(collect);
						colCount = 0;
						}

					mFormat(Vals[n - 1]);

					for (++Format; Format < Code; ++Format)
						{
						if (!Brent(&Format, &colCount, collect))
							{
							break;
							}
						}

					if (*Format)
						{
						Format++;
						}
					}
				else
					{
					Format = Code + 1;
					}
				}
			}
		}

	if (colCount)
		{
		collect[colCount] = 0;
		mFormat(collect);
		}
	}
