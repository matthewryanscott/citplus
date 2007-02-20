// --------------------------------------------------------------------------
// Citadel: Format.CPP
//
// Contains string handling stuff.

#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// normalizeString()	deletes leading & trailing blanks etc.
// strpos()				find a character in a string
// substr()				is string 1 in string 2?
// deansi()				Removes Ctrl-A codes from strings


char THREAD fmtBuf[MAXWORDLEN + 1];

// --------------------------------------------------------------------------
// normalizeString(): Deletes leading & trailing blanks etc.

void normalizeString(char *s)
	{
	char *pc;

	pc = s;

	if (!cfg.colors)
		{
		// find end of string
		while (*pc)
			{
			if (*pc < ' ')
				{
				*pc = ' ';
				}
			pc++;
			}
		}
	else
		{
		// find end of string
		while (*pc)
			{
			if (*pc < ' ' && (*pc != CTRL_A))
				{
				*pc = ' ';
				}
			pc++;
			}
		}

	// no trailing spaces
	while (pc > s && isspace(*(pc - 1)))
		{
		pc--;
		}
	*pc = '\0';

	// no leading spaces
	while (*s == ' ')
		{
		for (pc = s; *pc; pc++)
			{
			*pc = *(pc + 1);
			}
		}

	// no double blanks
	for (; *s;)
		{
		if (*s == ' ' && *(s + 1) == ' ')
			{
			for (pc = s; *pc; pc++)
				{
				*pc = *(pc + 1);
				}
			}
		else
			{
			s++;
			}
		}
	}


// --------------------------------------------------------------------------
// strpos(): Find a character in a string.

int strpos(char ch, const char *str)
	{
	int i;
	int len = strlen(str);

	for (i = 0; i < len; i++)
		{
		if (toupper(ch) == toupper(str[i]))
			{
			return (i + 1);
			}
		}

	return (0);
	}


// --------------------------------------------------------------------------
// substr(): Is string 2 in string 1?
// returns: -1 on no match
//			nonnegative integer is location of s2 in s1

int substr(const char *str1, const char *str2, Bool StripAnsi)
	{
	char *tmp1, *tmp2;
	int toreturn;

	tmp1 = strdup(str1);
	tmp2 = strdup(str2);

	if (tmp1 == NULL || tmp2 == NULL)
		{
		delete [] tmp1;
		delete [] tmp2;

		OutOfMemory(6);
		return (CERROR);
		}

	strlwr(tmp1);
	strlwr(tmp2);

	if (StripAnsi)
		{
		stripansi(tmp1);
		stripansi(tmp2);
		}

	const char *result = strstr(tmp1, tmp2);
	if (result == NULL)
		{
		toreturn = CERROR;
		}
	else
		{
		toreturn = (int) (result - tmp1);
		}

	delete [] tmp1;
	delete [] tmp2;

	return (toreturn);
	}


#undef strcmp
#undef stricmp
#undef strcmpi

int deansi_str_cmp(const char *string1, const char *string2)
	{
	char buf1[MAXWORDLEN + 1];
	char buf2[MAXWORDLEN + 1];

	if ((strlen(string1) > MAXWORDLEN) || (strlen(string2) > MAXWORDLEN) )
		{
#ifndef WINCIT
		cPrintf(getmsg(649));
		doccr();
#endif
		return (1);
		}

	strcpy(buf1, string1);
	strcpy(buf2, string2);

	stripansi(buf1);
	stripansi(buf2);

	return (strcmp(buf1, buf2));
	}

int deansi_str_cmpi(const char *string1, const char *string2)
	{
	char buf1[MAXWORDLEN + 1];
	char buf2[MAXWORDLEN + 1];

	if ((strlen(string1) > MAXWORDLEN) || (strlen(string2) > MAXWORDLEN))
		{
#ifndef WINCIT
		cPrintf(getmsg(650));
		doccr();
#endif
		return (1);
		}

	strcpy(buf1, string1);
	strcpy(buf2, string2);

	stripansi(buf1);
	stripansi(buf2);

	return (stricmp(buf1, buf2));
	}

const char *deansi(const char *str)
	{
	int i = 0;
	const char *pc;

	if (strlen(str) > ((LABELSIZE * 4) + 1))
		{
#ifndef WINCIT
		cPrintf(getmsg(651));
#endif
		return (str);
		}

	pc = str;

	while (*pc)
		{
		if (*pc == CTRL_A || *pc == CTRL_B)
			{
			if (*(pc + 1))
				{
				pc += 2;

				if (toupper(*(pc - 1)) == 'X')
					{
					while (*pc > CTRL_B && !isspace(*pc) &&
							toupper(*pc) != 'X' && *pc != '\r' && *pc != '\n')
						{
						pc++;
						}

					if (toupper(*pc) == 'X')
						{
						pc++;
						}
					}
				}
			else
				{
				pc++;
				}
			}
		else
			{
			fmtBuf[i] = *pc;
			i++;
			pc++;
			}
		}

	fmtBuf[i] = 0;

	return (fmtBuf);
	}


const char *special_deansi(const char *str, const char *strip)
	{
	int i = 0;
	const char *pc;
	int len = strlen(strip);

	if (strlen(str) > ((LABELSIZE * 4) + 1))
		{
#ifndef WINCIT
		cPrintf(getmsg(651));
		doccr();
#endif
		return (str);
		}

	pc = str;

	while (*pc)
		{
		if (*pc == CTRL_A && ((toupper(*(pc + 1)) == 'X' &&
				strnicmp(pc + 2, strip, len)) ||
				(*(pc + 1) == strip[0] && len == 1)))
			{
			if (*(pc + 1))
				{
				pc += 2;

				if (toupper(*(pc - 1)) == 'X')
					{
					while (*pc > CTRL_B && !isspace(*pc) &&
							toupper(*pc) != 'X' && *pc != '\r' && *pc != '\n')
						{
						pc++;
						}

					if (toupper(*pc) == 'X')
						{
						pc++;
						}
					}
				}
			else
				{
				pc++;
				}
			}
		else
			{
			fmtBuf[i] = *pc;
			i++;
			pc++;
			}
		}

	fmtBuf[i] = 0;

	return (fmtBuf);
	}


int hasansi(const char *str)
	{
	return (strchr(str, CTRL_A) != NULL);
	}

// changes string being sent, does not return a pointer like deansi
void stripansi(char *str)
	{
	int i = 0;
	char *pc;

	pc = str;

	while (*pc)
		{
		if (*pc == CTRL_A || *pc == CTRL_B)
			{
			if (*(pc + 1))
				{
				pc += 2;

				if (toupper(*(pc - 1)) == 'X')
					{
					while (*pc > CTRL_B && !isspace(*pc) &&
							toupper(*pc) != 'X' && *pc != '\r' && *pc != '\n')
						{
						pc++;
						}

					if (toupper(*pc) == 'X')
						{
						pc++;
						}
					}
				}
			else
				{
				pc++;
				}
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

// changes string being sent, does not return a pointer like deansi
void strip(char *str, char ch)
	{
	int i = 0;
	char *pc;

	pc = str;

	while (*pc)
		{
		if (*pc == ch)
			{
			pc++;		// Zap char
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

const char *strip_punct(const char *str)
	{
	if (strlen(str) > MAXWORDLEN)
		{
#ifndef WINCIT
		cPrintf(getmsg(655));
		doccr();
#endif
		return (str);
		}

	const char *pc;
	for (pc = str; *pc && ispunct(*pc); pc++);

	strcpy(fmtBuf, pc);

	Bool done = FALSE;
	for (int i = strlen(fmtBuf); i && !done; i--)
		{
		if (ispunct(fmtBuf[i-1]))
			{
			fmtBuf[i-1] = 0;
			}
		else
			{
			done = TRUE;
			}
		}

	return (fmtBuf);
	}

// changes string being sent, does not return a pointer like deansi
// used for names
void strip_special(char *str)
	{
	int i = 0;
	char *pc;

	pc = str;

	while (*pc)
		{
		if (!(isalpha(*pc) || isdigit(*pc) || *pc == 1 || *pc == ' ' || *pc == '.' || *pc == '-' || *pc == '\''))
			{
			pc++;
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

