/* -------------------------------------------------------------------- */
/*	EXT2PROT.CPP				Citadel 								*/
/* -------------------------------------------------------------------- */
/*	Convert program. 64 to 65: #PROTOCOLs out of EXTERNAL.CIT, into 	*/
/*	PROTOCOL.CIT.														*/
/* -------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SAMESTRING		0		/* value for strcmp() & friend			*/
#define SameString(a,b) (strcmpi((a), (b)) == SAMESTRING)

#define LABELSIZE		30		/* length of room names 				*/
typedef char			label[LABELSIZE+1];
typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned char	uchar;
typedef uint			Bool;

typedef struct pr
	{
	struct pr *next;
	label name; 				/* The name of the protocol.			*/
	label MenuName;
	char CommandKey;
	label autoDown; 			/* An auto-download initiation string.	*/
	char respDown[128]; 		/* Command send with response file. 	*/
	char rcv[128];				/* Command line to receive files.		*/
	char snd[128];				/* Command line to send files.			*/
	int block;					/* Size of block. (0 for variable.) 	*/
	Bool batch; 				/* If it is a batch protocol.			*/
	Bool NetOnly;				/* Only for networking. 				*/
	} protocols;

protocols *protList;

const char *externalCit = "EXTERNAL.CIT";
const char *protocolCit = "PROTOCOL.CIT";

void ReadExternalCit(void);
void WriteProtocolCit(void);


const char *mkDblBck(const char *str, char *newstr)
	{
	char *wow = newstr;

	for (; *str; str++)
		{
		if (*str == '\n')
			{
			*(newstr++) = '\\';
			*newstr = 'n';
			}
		else if (*str == '\t')
			{
			*(newstr++) = '\\';
			*newstr = 't';
			}
		else if (*str == '\r')
			{
			*(newstr++) = '\\';
			*newstr = 'r';
			}
		else if (*str == '\f')
			{
			*(newstr++) = '\\';
			*newstr = 'f';
			}
		else if (*str == '\b')
			{
			*(newstr++) = '\\';
			*newstr = 'b';
			}
		else if (*str == 127)
			{
			*(newstr++) = '^';
			*newstr = '?';
			}
		else
			{
			if (*str < 32)
				{
				*(newstr++) = '^';
				*newstr = *str + '@';
				}
			else
				{
				*newstr = *str;

				if (*newstr == '\\')
					{
					*(++newstr) = '\\';
					}
				}
			}

		newstr++;
		}

	*newstr = 0;
	return (wow);
	}

/* -------------------------------------------------------------------- */
/*	parse_it()		routines to parse strings separated by white space	*/
/* -------------------------------------------------------------------- */
/*																		*/
/* strategy: called as: count = parse_it(workspace, input); 			*/
/*																		*/
/* where workspace is a two-d char array of the form:					*/
/*																		*/
/* char *workspace[MAXWORD];											*/
/*																		*/
/* and input is the input string to be parsed.	it returns				*/
/* the actual number of things parsed.									*/
/*																		*/
/* -------------------------------------------------------------------- */
/* states of machine... */
#define INWORD		0
#define OUTWORD 	1
#define INQUOTES	2

/* characters */
#undef	TAB
#define TAB 	9
#define BLANK	' '
#define QUOTE	'\"'
#define QUOTE2	'\''
#define MXWORD	128

/* -------------------------------------------------------------------- */
/*	qtext() 		Consumes quoted strings and expands escape chars	*/
/* -------------------------------------------------------------------- */
static int qtext(char *buf, char *line, char end)
	{
	int index = 0;
	int slash = 0;
	char chr;

	while (line[index] != '\0' && (line[index] != end || slash != 0))
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

				case 'n':                       /* newline */
					{
					*(buf++) = '\n';
					break;
					}

				case 't':                       /* tab */
					{
					*(buf++) = '\t';
					break;
					}

				case 'r':                       /* carriage return */
					{
					*(buf++) = '\r';
					break;
					}

				case 'f':                       /* formfeed */
					{
					*(buf++) = '\f';
					break;
					}

				case 'b':                       /* backspace */
					{
					*(buf++) = '\b';
					break;
					}
				}
			slash = 0;
			}
		else /* if (slash == 2 ) */
			{
			if (line[index] == '?')
				{
				chr = 127;
				}
			else if (line[index] >= 'A' && line[index] <= 'Z')
				{
				chr = (char) (line[index] - 'A' + 1);
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

int parse_it(char *words[], char input[])
	{
	int i, state, thisword;

	input[strlen(input) + 1] = 0;		/* double-null */

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

				/* if we are now in a string, setup, otherwise, break */
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
/* for this to work, the "next" field of the list has to be the first   */
/* data in each node													*/
/* -------------------------------------------------------------------- */
/*								Contents								*/
/*																		*/
/*	disposeLL() 	disposes linked list								*/
/*	addLL() 		adds a node at the end								*/
/*	getNextLL() 	gets the next node in the list, or NULL 			*/
/*	getLLNum()		gets the nth entry in a list						*/
/*	deleteLLNode()	deletes the nth node in a list						*/
/*	insertLLNode()	instert before the nth node in a list				*/
/*	getLLCount()	counts the nodes in the list						*/
/* -------------------------------------------------------------------- */

typedef struct eLL
	{
	struct eLL *next;
	} emptyLL;

/* -------------------------------------------------------------------- */
/*	disposeLL() 	disposes linked list								*/
/* -------------------------------------------------------------------- */
void disposeLL(void **list)
	{
	emptyLL *thisNode;

	thisNode = (emptyLL *) *list;

	while (thisNode)
		{
		emptyLL *nxt;

		nxt = thisNode->next;
		delete [] thisNode;
		thisNode = nxt;
		}

	*list = NULL;
	}

/* -------------------------------------------------------------------- */
/*	addLL() 		adds a node at the end								*/
/* -------------------------------------------------------------------- */
void *addLL(void **list, uint nodeSize)
	{
	emptyLL *newNode, *cur;

	newNode = (emptyLL *) new char[nodeSize];
	if (newNode)
		{
		memset(newNode, 0, nodeSize);
		}

	if (*list)
		{
		cur = (emptyLL *) *list;

		while (cur->next)
			{
			cur = cur->next;
			}

		cur->next = newNode;
		}
	else
		{
		/* new list */
		*list = newNode;
		}

	return (newNode);
	}

/* -------------------------------------------------------------------- */
/*	getNextLL() 	gets the next node in the list, or NULL 			*/
/* -------------------------------------------------------------------- */
void *getNextLL(const void *list)
	{
	if (list)
		{
		return (((emptyLL *)list)->next);
		}

	return (NULL);
	}

/* -------------------------------------------------------------------- */
/*	getLLNum()		gets the nth entry in a list						*/
/* -------------------------------------------------------------------- */
void *getLLNum(void *list, ulong n)
	{
	if (n)
		{
		while (--n && list)
			{
			list = ((emptyLL *) list)->next;
			}

		return (list);
		}

	return (NULL);
	}

/* -------------------------------------------------------------------- */
/*	deleteLLNode()	deletes the nth node in a list						*/
/* -------------------------------------------------------------------- */
void deleteLLNode(void **list, ulong n)
	{
	emptyLL *cur = NULL, *prev;

	if (*list && n)
		{
		if (n == 1)
			{
			cur = (emptyLL *) *list;
			*list = cur->next;
			}
		else
			{
			prev = (emptyLL *) getLLNum(*list, n - 1);

			if (prev)
				{
				cur = prev->next;

				if (cur)
					{
					prev->next = cur->next;
					}
				}
			}

		delete [] cur;
		}
	}

/* -------------------------------------------------------------------- */
/*	insertLLNode()	instert before the nth node in a list				*/
/* -------------------------------------------------------------------- */
void *insertLLNode(void **list, ulong n, int nodeSize)
	{
	if (n)
		{
		emptyLL *newNode, *cur;

		newNode = (emptyLL *) new char[nodeSize];
		if (newNode)
			{
			memset(newNode, 0, nodeSize);
			}

		if (newNode)
			{
			if (n == 1)
				{
				newNode->next = (emptyLL *) *list;
				*list = newNode;
				}
			else
				{
				cur = (emptyLL *) getLLNum(*list, n - 1);

				if (cur && cur->next)
					{
					newNode->next = cur->next;
					cur->next = newNode;
					}
				else
					{
					// list not that big - just stick at end
					cur = (emptyLL *) *list;

					while (cur->next)
						{
						cur = cur->next;
						}

					cur->next = newNode;
					}
				}
			}

		return (newNode);
		}
	else
		{
		return (NULL);
		}
	}

/* -------------------------------------------------------------------- */
/*	getLLCount()	counts the nodes in the list						*/
/* -------------------------------------------------------------------- */
ulong getLLCount(const void *list)
	{
	ulong i = 0;

	while (list && ++i)
		{
		list = ((emptyLL *)list)->next;
		}

	return (i);
	}




// copy a string into a buffer of a known length
inline void CopyStringToBuffer(char *dest, const char *src, int Length)
	{
	strncpy(dest, src, Length);
	dest[Length] = 0;
	}

void main(void)
	{
	printf("EXTERNAL to PROTOCOL\n"
			"This program reads EXTERNAL.CIT and copies #PROTOCOL information into\n"
			"PROTOCOL.CIT, which Citadel+/065 uses to define external file transfer\n"
			"protocols.\n\nReading EXTERNAL.CIT...\n");

	ReadExternalCit();

	printf("Writing PROTOCOL.CIT...\n");

	WriteProtocolCit();

	printf("\nDone. Thank you for using Citadel+.\n");
	}

/* -------------------------------------------------------------------- */
/*	ReadExternalCit()	reads external.cit values into lists			*/
/* -------------------------------------------------------------------- */
void ReadExternalCit(void)
	{
	FILE *fBuf;
	char line[257];
	char *words[256];
	int count;

	if ((fBuf = fopen(externalCit, "r")) == NULL)       /* ASCII mode */
		{
		printf("Cannot find %s!", externalCit);
		exit(69);
		}
	else
		{
		int i, lineNo = 0;

		while (fgets(line, 256, fBuf) != NULL)
			{
			lineNo++;

			if (line[0] != '#')
				{
				continue;
				}

			count = parse_it(words, line);

			if (SameString(words[0], "#PROTOCOL"))
				{
				protocols *theProt;

				theProt = (protocols *) addLL((void **) &protList,
							sizeof(*theProt));

				if (theProt)
					{
					CopyStringToBuffer(theProt->name, words[1], LABELSIZE);
					theProt->batch = !!atoi(words[2]);
					theProt->block = atoi(words[3]);
					CopyStringToBuffer(theProt->rcv, words[4], 127);
					CopyStringToBuffer(theProt->snd, words[5], 127);

					if (count <= 6 || words[6][0] == '\0')
						{
						theProt->autoDown[0] = '\0';
						}
					else
						{
						CopyStringToBuffer(theProt->autoDown, words[6], LABELSIZE);
						}

					if (count > 7)
						{
						CopyStringToBuffer(theProt->respDown, words[7], 127);
						}
					}
				else
					{
					printf("Not enough memory for #PROTOCOL \"%s\"",
							words[1]);
					printf("\n");
					}
				}
			}

		fclose(fBuf);
		}
	}

/* -------------------------------------------------------------------- */
/*	ReadProtocolCit()	reads PROTOCOL.CIT values into list 			*/
/* -------------------------------------------------------------------- */
void WriteProtocolCit(void)
	{
	FILE *fBuf;
	char line[257];
	char *words[256];
	int count;

	protocols *theProt;

	if ((fBuf = fopen(protocolCit, "a")) == NULL)   /* ASCII mode */
		{
		printf("Cannot open %s!", protocolCit);
		printf("\n");
		}
	else
		{
		protocols *theProt;
		char wow[300];

		for (theProt = protList; theProt;
				theProt = (protocols *) getNextLL(theProt))
			{
			fprintf(fBuf, "%s\t\"%s\"\n", "#PROTOCOL", mkDblBck(theProt->name, wow));
			fprintf(fBuf, "%s\t\"%s\"\n", "#RECEIVE", mkDblBck(theProt->rcv, wow));
			fprintf(fBuf, "%s\t\t\"%s\"\n", "#SEND", mkDblBck(theProt->snd, wow));
			fprintf(fBuf, "%s\t\"%s\"\n", "#RESPONSE_SEND", mkDblBck(theProt->respDown, wow));
			if (theProt->autoDown[0] != 7)
				{
				fprintf(fBuf, "%s\t\"%s\"\n", "#AUTO_UPLOAD", mkDblBck(theProt->autoDown, wow));
				}
			fprintf(fBuf, "%s\t\t%d\n", "#BATCH", theProt->batch);
			fprintf(fBuf, "%s\t%d\n", "#BLOCK_SIZE", theProt->block);
			fprintf(fBuf, "\n");
			}

		fclose(fBuf);
		}
	}
