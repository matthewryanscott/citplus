// --------------------------------------------------------------------------
// Citadel: FmtOvl.CPP
//
// Contains string handling stuff.

#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// sformat()	Configurable format
// parse_it()	routines to parse strings separated by white space
// qtext()		Consumes quoted strings and expands escape chars
// u_match()	Unix wildcarding
// cclass() 	Used with u_match()
// stripdoubleasterisk()	Strips multiple asterisks from a string.

static int qtext(char *buf, char *line, char end);

// --------------------------------------------------------------------------
// sformat(): Configurable format
/*
 *	sformat, a 10 minute project
 *	by Peter Torkelson
 *
 *	parses a number of arguments this lets you make configurable things..
 *
 *	call it with:
 *		str 	a string buffer
 *		fmt 	the format string
 *		val 	valid % escapes
 *
 *	here is an example:
 *
 *		sformat(str, "Hello %n, I am %w. (%%, %c)", "nw", "you", "me");
 *
 *	gives you:
 *		"Hello you, I am me. (%, (NULL))"
 *
 */
void cdecl sformat(char *str, const char *fmt, const char *val, ... )
	{
	int i;

	char s[2];
	va_list ap;

	s[1] = 0;
	*str = 0;

	while (*fmt)
		{
		if (*fmt != '%')
			{
			*s = *fmt;
			strcat(str, s);
			}
		else
			{
			fmt++;
			if (*fmt == 0)			// "somthing %", not nice
				{
				return;
				}

			if (*fmt == '%')        // %% = %
				{
				*s = *fmt;
				strcat(str, s);
				}
			else					// it must be a % something
				{
				i = strpos(*fmt, val) - 1;
				if (i != -1)
					{
					va_start(ap, val);
					while (i--)
						{
						(void) va_arg(ap, char *);
						}

					strcat(str, va_arg(ap, char *));
					va_end(ap);
					}
				else
					{
					strcat(str, getmsg(647));
					}
				}
			}

		fmt++;
		}
	}

// --------------------------------------------------------------------------
// parse_it(): routines to parse strings separated by white space
//
// Input:
//	char input[]: Actual input string to be parsed
//
// Output:
//	char *words[]: list of parsed words
//
// Return value:
//	int: number of words parsed

// states of machine
#define INWORD		0
#define OUTWORD 	1
#define INQUOTES	2

// characters
#define QUOTE	'"'
#define QUOTE2	'\''
#define MXWORD	128

int parse_it(char *words[], char input[])
	{
	int i, state, thisword;

	input[strlen(input) + 1] = 0;		// double-null

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

/* -------------------------------------------------------------------- */
/*	qtext() 		Consumes quoted strings and expands escape chars	*/
/* -------------------------------------------------------------------- */
static int qtext(char *buf, char *line, char end)
	{
	int index = 0;
	int slash = 0;

	while (line[index] != 0 && (line[index] != end || slash != 0))
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

				case 'a':                       // alarm
					{
					*(buf++) = BELL;
					break;
					}

				case 'n':                       // newline
					{
					*(buf++) = '\n';
					break;
					}

				case 't':                       // tab (horizontal)
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
		else // if (slash == 2 )
			{
			char chr;

			if (line[index] == '?')
				{
				chr = 127;
				}
			else if (line[index] >= '@' && line[index] <= '_')
				{
				chr = (char) (line[index] - '@');
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

/* -------------------------------------------------------------------- */
/*	u_match()		Unix wildcarding									*/
/* -------------------------------------------------------------------- */
/*
 * Bool u_match(string, pattern)
 * char *string, *pattern;
 *
 * Match a pattern as in sh(1).
 */

#define CMASK	0377
#undef	QUOTE
#define QUOTE	0200
#define QMASK	(CMASK&~QUOTE)
#define NOT '!' // might use ^

static const char *cclass(const register char *p, register int sub);

Bool u_match(const register char *s, const register char *p)
	{
	register int sc, pc;

	if (s == NULL || p == NULL)
		{
		return (FALSE);
		}

	while ((pc = *p++ & CMASK) != 0)
		{
		sc = *s++ & QMASK;

		switch (pc)
			{
			case '[':
				{
				if ((p = cclass(p, sc)) == NULL)
					{
					return (FALSE);
					}

				break;
				}

			case '?':
				{
				if (sc == 0)
					{
					return (FALSE);
					}

				break;
				}

			case '*':
				{
				s--;

				do
					{
					if (*p == 0 || u_match(s, p))
						{
						return (TRUE);
						}
					} while (*s++ != 0);

				return (FALSE);
				}

			default:
				{
				if (tolower(sc) != (tolower(pc&~QUOTE)))
					{
					return (FALSE);
					}
				}
			}
		}
	return (*s == 0);
	}

/* -------------------------------------------------------------------- */
/*	cclass()		Used with u_match() 								*/
/* -------------------------------------------------------------------- */
static const char *cclass(const register char *p, register int sub)
	{
	register int c, d, not, found;

	sub = tolower(sub);

	if ((not = *p == NOT) != 0) 	// so naughty
		{
		p++;
		}

	found = not;

	do
		{
		if (*p == 0)
			{
			return (NULL);
			}

		c = tolower(*p & CMASK);

		if (p[1] == '-' && p[2] != ']')
			{
			d = tolower(p[2] & CMASK);
			p++;
			}
		else
			{
			d = c;
			}

		if (c == sub || c <= sub && sub <= d)
			{
			found = !not;
			}

		} while (*++p != ']');

	return (found ? p + 1 : NULL);
	}

// --------------------------------------------------------------------------
// stripdoubleasterisk(): Strips multiple asterisks from a string.
//
// Input:
//	char *str: String to be stripped of multiple consecutive asterisks
//
// Output:
//	char *str: Modified to new value
//
// Return value:
//	None

void stripdoubleasterisk(char *str)
	{
	int i = 0;
	char *pc = str;

	while (*pc)
		{
		if (*pc == '*' && (*(pc+1) == '*'))
			{
			pc++;		// Zap extra asterisk
			}
		else
			{
			str[i] = *pc;
			i++;
			pc++;
			}
		}
	str[i] = 0;
	}

char *MakeItLower(const char *Source, char *Dest)
	{
	int i;
	Bool Special = FALSE, SpecialX = FALSE, First = TRUE;

	for (i = 0; Source[i]; i++)
		{
		Bool ChangeIt = FALSE;

		if (First)
			{
			if (Source[i] == CTRL_A || Source[i] == CTRL_B)
				{
				Special = TRUE;
				}
			else
				{
				if (SpecialX)
					{
					if (toupper(Source[i]) == 'X')
						{
						SpecialX = FALSE;
						}
					}
				else if (Special)
					{
					SpecialX = (toupper(Source[i + 1]) == 'X');
					Special = FALSE;
					}
				else
					{
					ChangeIt = TRUE;
					First = FALSE;
					}
				}
			}

		if (ChangeIt)
			{
			Dest[i] = (char) tolower(Source[i]);
			}
		else
			{
			Dest[i] = Source[i];
			}
		}

	Dest[i] = 0;

	return (Dest);
	}

char *MakeItUpper(const char *Source, char *Dest)
	{
	int i;
	Bool Special = FALSE, SpecialX = FALSE, First = TRUE;

	for (i = 0; Source[i]; i++)
		{
		Bool ChangeIt = FALSE;

		if (First)
			{
			if (Source[i] == CTRL_A || Source[i] == CTRL_B)
				{
				Special = TRUE;
				}
			else
				{
				if (SpecialX)
					{
					if (toupper(Source[i]) == 'X')
						{
						SpecialX = FALSE;
						}
					}
				else if (Special)
					{
					SpecialX = (toupper(Source[i + 1]) == 'X');
					Special = FALSE;
					}
				else
					{
					ChangeIt = TRUE;
					First = FALSE;
					}
				}
			}

		if (ChangeIt)
			{
			Dest[i] = (char) toupper(Source[i]);
			}
		else
			{
			Dest[i] = Source[i];
			}
		}

	Dest[i] = 0;

	return (Dest);
	}
