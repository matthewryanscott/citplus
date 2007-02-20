// --------------------------------------------------------------------------
// Citadel: FileRdWr.CPP
//
// General-purpose file read/write functions


#include "ctdl.h"
#pragma hdrstop

#include "filerdwr.h"

// --------------------------------------------------------------------------
// Contents
//
// PutStr() 			puts a null-terminated string to a file
// GetStr() 			gets a null-terminated string from a file

// Get null-terminated string from file
Bool GetStr(FILE *fl, char *str, int mlen)
	{
	if (feof(fl))
		{
		return (FALSE);
		}

	uchar ch = 1;
	int l = 0;
	while (!feof(fl) && ch)
		{
		ch = (uchar) fgetc(fl);

		if ((ch != 0xFF && ch != '\r') && l < mlen)
			{
			str[l++] = ch;
			}
		}

	str[l] = 0;

	while (l-- > 0 && str[l] == CTRL_A)
		{
		str[l] = 0;
		}

	if (*str)
		{
		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

/* -------------------------------------------------------------------- */
/*	net69_GetStr()	gets a null-or-CRLF-terminated string from a file	*/
/* -------------------------------------------------------------------- */
Bool net69_GetStr(FILE *fl, char *str, int mlen)
	{
	uchar ch;

	ch = (uchar) fgetc(fl);

	while (!feof(fl) && (!ch || (ch == '\r') || (ch == '\n') || (ch == 0xFF)))
		{
		ch = (uchar) fgetc(fl);
		}

	if (feof(fl))
		{
		*str = 0;
		return (FALSE);
		}

	int l = 0;
	while (!feof(fl) && ch && (ch != '\r') && (ch != '\n') && (ch != 0xFF))
		{
		if (l < mlen)
			{
			str[l++] = ch;
			}

		ch = (uchar) fgetc(fl);
		}

	str[l] = 0;

	while (l-- > 0 && str[l] == CTRL_A)
		{
		str[l] = 0;
		}

	if (*str)
		{
		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}

/* -------------------------------------------------------------------- */
/*	net69_PutStr()		puts a null-terminated string to a file after CRLF*/
/* -------------------------------------------------------------------- */
void net69_PutStr(FILE *fl, const char *str)
	{
	fputc(13, fl);
	fputc(10, fl);

	fwrite(str, sizeof(char), strlen(str), fl);
	}

/* -------------------------------------------------------------------- */
/*	PutStr()			puts a null-terminated string to a file 		*/
/* -------------------------------------------------------------------- */
Bool PutStr(FILE *fl, const char *str)
	{
	const size_t Bytes = strlen(str) + 1;

	return (fwrite(str, sizeof(char), Bytes, fl) == Bytes);
	}
