#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>
#include <bios.h>
#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <sys\stat.h>


#define FALSE 0
#define TRUE 1

typedef unsigned int uint;
typedef unsigned long ulong;

int parse_it(char *words[], char input[]);
int qtext(char *buf, char *line, char end);
void disposeLL(void **list);
void *addLL(void **list, uint nodeSize);
void *getNextLL(void *list);
void *getLLNum(void *list, ulong n);
void deleteLLNode(void **list, ulong n);
ulong getLLCount(void *list);
void NetWith(char *node1, char *node2);
int getNode(char *path, char *node, int which);
void FixupLinks(void);

typedef enum
	{
	NT_AUTO, NT_MANUAL, NT_MANADDR
	} nodeTypes;

typedef struct sl
	{
	struct sl *next;
	char str[1];
	} strList;

typedef struct ni
	{
	struct ni *next;
	nodeTypes type;
	char name[31];
	char address[31];
	long pos;
	int touched;
	strList *netwith;
	} nodeInfo;

typedef struct dn
	{
	struct dn *next;

	char node1[31];
	char node2[31];
	} deadNet;

nodeInfo *findNode(char *name);

nodeInfo *nodes;
strList *deadNodes;
deadNet *deadNets;

void main(int argc, char *argv[])
	{
	FILE *inDat, *outMcr;
	char inFileDat[80], outFileMcr[80];
	int counter;

	printf("GremCit/064á Network Mapmaker Pass 2\n"
			"By Anticlimactic Teleservices");

	if (argc > 2)
		{
		strcpy(inFileDat, argv[1]);
		}
	else
		{
		strcpy(inFileDat, "map.dat");
		}

	if (argc > 3)
		{
		strcpy(outFileMcr, argv[2]);
		}
	else
		{
		strcpy(outFileMcr, "makemap.mcr");
		}

	printf("\n\nUsing input map file: %s.", inFileDat);
	printf("\nUsing output macro file: %s.", outFileMcr);
	printf("\nFree memory: %ld\n", farcoreleft());

	if ((inDat = fopen(inFileDat, "rt")) != NULL)
		{
		char line[512], line2[512];
		char *words[256];
		int lnum = 0, bad, count;
		long l;

		printf("\nReading input map file...");

		while (fgets(line, 512, inDat) != NULL)
			{
			lnum++;

			if (line[0] != '#')
				{
				continue;
				}

			if (strchr(line, '\n'))
				{
				*strchr(line, '\n') = 0;
				}
			strcpy(line2, line);

			count = parse_it(words, line);

			if (count)
				{
				nodeInfo *curNode = NULL;
				bad = FALSE;

				if (!strcmpi(words[0], "#AUTO"))
					{
					curNode = (nodeInfo *) addLL((void *) &nodes,
							sizeof(*curNode));

					if (curNode)
						{
						curNode->type = NT_AUTO;
						}
					}
				else if (!strcmpi(words[0], "#MANUAL"))
					{
					curNode = (nodeInfo *) addLL((void *) &nodes,
							sizeof(*curNode));

					if (curNode)
						{
						curNode->type = NT_MANUAL;
						}
					}
				else if (!strcmpi(words[0], "#MANUAL_ADDRESS"))
					{
					curNode = (nodeInfo *) addLL((void *) &nodes,
							sizeof(*curNode));

					if (curNode)
						{
						curNode->type = NT_MANADDR;
						}
					}
				else
					{
					bad = TRUE;
					}

				if (bad)
					{
					if (!strcmpi(words[0], "#DEAD_NODE"))
						{
						}
					else if (!strcmpi(words[0], "#DEAD_NET"))
						{
						}
					else
						{
						printf("\nBad line (#%d) ignored: %s", lnum, line2);
						}
					}
				else
					{
					if (curNode)
						{
						int i;

						strcpy(curNode->name, words[1]);
						strcpy(curNode->address, words[2]);

						for (i = 3; i < count; i++)
							{
							strList *sl;

							sl = (strList *) addLL((void *) &curNode->netwith,
									sizeof(*sl) + strlen(words[i]));

							if (sl)
								{
								strcpy(sl->str, words[i]);
								}
							else
								{
								printf("\nOut of memory line #%d", lnum);
								exit(1);
								}
							}
						}
					else
						{
						printf("\nOut of memory line #%d", lnum);
						exit(1);
						}
					}
				}
			}
		fclose(inDat);
		}
	else
		{
		printf("\nInput map file not found");
		exit(1);
		}

	printf("\nFree memory: %ld\n", farcoreleft());

	if ((outMcr = fopen(outFileMcr, "wt")) != NULL)
		{
		nodeInfo *theNode;
		long pos = 0;

		fprintf(outMcr, "TC,15,LT,0,TZ,1;\n");

		for (theNode = nodes; theNode; theNode = getNextLL(theNode))
			{
			theNode->pos = pos--;
			fprintf(outMcr, "TL,0,%ld,%s,#,TL,20,%ld,%s,#;\n", theNode->pos,
					theNode->address, theNode->pos, theNode->name);
			}

		fprintf(outMcr, "LC,14,LT,9;\n");

		for (theNode = nodes; theNode; theNode = getNextLL(theNode))
			{
			strList *sl;

			for (sl = theNode->netwith; sl; sl = getNextLL(sl))
				{
				nodeInfo *theOtherNode = findNode(sl->str);

				if (theOtherNode)
					{
					int test;

					fprintf(outMcr, "LI,0,%ld,0,%ld,#;\n", theNode->pos,
							theOtherNode->pos);

					do
						{
						strList *s;
						long i;

						test = FALSE;

						for (i = 1, s = theOtherNode->netwith; s; s = getNextLL(s), i++)
							{
							if (!strcmpi(s->str, theNode->name) ||
									!strcmpi(s->str, theNode->address))
								{
								deleteLLNode((void *) &theOtherNode->netwith, i);
								test = TRUE;
								break;
								}
							}
						} while (test);
					}
				}
			}

		fprintf(outMcr, "ZA;");
		fclose(outMcr);
		}
	else
		{
		printf("\nCould not write output macro file");
		}
	}


void NetWith(char *node1, char *node2)
	{
	nodeInfo *theNode;
	strList *sl;
	int newNet = FALSE;

	if (strcmp(node1, ".."))
		{
		theNode = findNode(node1);
		if (!theNode)
			{
			printf("\nNew node: \"%s\"", node1);

			// create a new one
			theNode = (nodeInfo *) addLL((void *) &nodes, sizeof(*theNode));

			if (theNode)
				{
				strcpy(theNode->name, node1);
				strcpy(theNode->address, node1);
				}
			else
				{
				printf("\nOut of memory");
				exit(1);
				}
			}

		for (sl = theNode->netwith; sl; sl = getNextLL(sl))
			{
			if (!strcmpi(sl->str, node2))
				{
				break;
				}
			}

		if (!sl)
			{
			sl = (strList *) addLL((void *) &theNode->netwith,
					sizeof(*sl) + strlen(node2));

			if (sl)
				{
				strcpy(sl->str, node2);
				}
			else
				{
				printf("\nOut of memory");
				exit(1);
				}

			newNet = TRUE;
			}
		}

	if (strcmp(node2, ".."))
		{
		theNode = findNode(node2);
		if (!theNode)
			{
			printf("\nNew node: \"%s\"", node2);

			// create a new one
			theNode = (nodeInfo *) addLL((void *) &nodes, sizeof(*theNode));

			if (theNode)
				{
				strcpy(theNode->name, node2);
				strcpy(theNode->address, node2);
				}
			else
				{
				printf("\nOut of memory");
				exit(1);
				}
			}

		for (sl = theNode->netwith; sl; sl = getNextLL(sl))
			{
			if (!strcmpi(sl->str, node1))
				{
				break;
				}
			}

		if (!sl)
			{
			sl = (strList *) addLL((void *) &theNode->netwith,
					sizeof(*sl) + strlen(node1));

			if (sl)
				{
				strcpy(sl->str, node1);
				}
			else
				{
				printf("\nOut of memory");
				exit(1);
				}

			newNet = TRUE;
			}
		}

	if (newNet)
		{
		printf("\nNew netlink: \"%s\" to \"%s\"", node1, node2);
		}
	}

int getNode(char *path, char *node, int which)
	{
	int i;
	char *ptr = path;

	node[0] = 0;

	*(strchr(path, 0) + 1) = 0;

	for (i = 0; *ptr && i < which; i++)
		{
		while (*ptr && !(*ptr == '!' && *ptr + 1 != '!' && *ptr + 2 != '!'))
			{
			ptr++;
			}

		if (*ptr)
			{
			ptr++;
			}
		}

	for (i = 0; i < 30 && *ptr && *ptr != '!'; ptr++, i++)
		{
		node[i] = *ptr;
		}

	node[i] = 0;

	return (node[0]);
	}

void FixupLinks(void)
	{
	strList *sl;
	deadNet *dn;
	nodeInfo *ni;

	printf("\nVerifying data...");

	for (sl = deadNodes; sl; sl = getNextLL(sl))
		{
		int test;

		do
			{
			long i;

			test = FALSE;

			for (i = 1, ni = nodes; ni; ni = getNextLL(ni), i++)
				{
				if (!strcmpi(sl->str, ni->name) || !strcmpi(sl->str, ni->address))
					{
					printf("\nRemoving dead node \"%s\"", sl->str);
					deleteLLNode((void *) &nodes, i);
					test = TRUE;
					break;
					}
				}
			} while (test);

		for (ni = nodes; ni; ni = getNextLL(ni))
			{
			strList *s;

			do
				{
				long i;

				test = FALSE;

				for (i = 1, s = ni->netwith; s; s = getNextLL(s), i++)
					{
					if (!strcmpi(s->str, sl->str))
						{
						printf("\nRemoving netlink between \"%s\" and dead node \"%s\"", ni->name, sl->str);
						deleteLLNode((void *) &ni->netwith, i);
						test = TRUE;
						break;
						}
					}
				} while (test);
			}
		}

	for (dn = deadNets; dn; dn = getNextLL(dn))
		{
		for (ni = nodes; ni; ni = getNextLL(ni))
			{
			if (!strcmpi(ni->name, dn->node1) || !strcmpi(ni->address, dn->node1))
				{
				int test;
				strList *s;

				do
					{
					long i;

					test = FALSE;

					for (i = 1, s = ni->netwith; s; s = getNextLL(s), i++)
						{
						if (!strcmpi(s->str, dn->node2))
							{
							printf("\nRemoving dead netlink between \"%s\" and \"%s\"", dn->node1, dn->node2);
							deleteLLNode((void *) &ni->netwith, i);
							test = TRUE;
							break;
							}
						}
					} while (test);
				}

			if (!strcmpi(ni->name, dn->node2) || !strcmpi(ni->address, dn->node2))
				{
				int test;
				strList *s;

				do
					{
					long i;

					test = FALSE;

					for (i = 1, s = ni->netwith; s; s = getNextLL(s), i++)
						{
						if (!strcmpi(s->str, dn->node1))
							{
							printf("\nRemoving dead netlink between \"%s\" and \"%s\"", dn->node2, dn->node1);
							deleteLLNode((void *) &ni->netwith, i);
							test = TRUE;
							break;
							}
						}
					} while (test);
				}
			}
		}

	for (ni = nodes; ni; ni = getNextLL(ni))
		{
		strList *sl;

		for (sl = ni->netwith; sl; sl = getNextLL(sl))
			{
			NetWith(ni->name, sl->str);
			}
		}
	}

nodeInfo *findNode(char *name)
	{
	nodeInfo *theNode;

	for (theNode = nodes; theNode; theNode = getNextLL(theNode))
		{
		if (!strcmpi(theNode->name, name) || !strcmpi(theNode->address, name))
			{
			break;
			}
		}

	return (theNode);
	}

/* -------------------------------------------------------------------- */
/*	parse_it()		routines to parse strings separated by white space	*/
/* -------------------------------------------------------------------- */
/*																		*/
/* strategy: called as: count = parse_it(workspace, input);				*/
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

int parse_it(char *words[], char input[])
	{
	int i, state, thisword;

	input[strlen(input) + 1] = 0; 		/* double-null */

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
/*	qtext() 		Consumes quoted strings and expands escape chars	*/
/* -------------------------------------------------------------------- */
int qtext(char *buf, char *line, char end)
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

				case 'n':						/* newline */
					{
					*(buf++) = '\n';
					break;
					}

				case 't':						/* tab */
					{
					*(buf++) = '\t';
					break;
					}

				case 'r':						/* carriage return */
					{
					*(buf++) = '\r';
					break;
					}

				case 'f':						/* formfeed */
					{
					*(buf++) = '\f';
					break;
					}

				case 'b':						/* backspace */
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
	return line[index] == end ? index + 1 : index;
	}

struct emptyLL
	{
	struct emptyLL	*next;
	};

/* -------------------------------------------------------------------- */
/*	disposeLL() 	disposes linked list								*/
/* -------------------------------------------------------------------- */
void disposeLL(void **list)
	{
	struct emptyLL *nxt, *this;

	this = *list;

	while (this)
		{
		nxt = this->next;
		free(this);
		this = nxt;
		}

	*list = NULL;
	}

/* -------------------------------------------------------------------- */
/*	addLL() 		adds a node at the end								*/
/* -------------------------------------------------------------------- */
void *addLL(void **list, uint nodeSize)
	{
	struct emptyLL *new, *cur;

	new = calloc(1, nodeSize);

	if (*list)
		{
		cur = *list;
		while (cur->next)
			{
			cur = cur->next;
			}
		cur->next = new;
		}
	else
		{
		/* new list */
		*list = new;
		}
	return (new);
	}

/* -------------------------------------------------------------------- */
/*	getNextLL() 	gets the next node in the list, or NULL 			*/
/* -------------------------------------------------------------------- */
void *getNextLL(void *list)
	{
	if ((struct emptyLL *)list)
		{
		return (((struct emptyLL *)list)->next);
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
			list = ((struct emptyLL *) list)->next;
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
	struct emptyLL *cur, *prev;

	if (*list && n)
		{
		if (n == 1)
			{
			cur = *list;
			*list = cur->next;
			}
		else
			{
			prev = getLLNum(*list, n - 1);
			if (prev)
				{
				cur = prev->next;

				if (cur)
					{
					prev->next = cur->next;
					}
				}
			}

		free(cur);
		}
	}

/* -------------------------------------------------------------------- */
/*	getLLCount()	counts the nodes in the list						*/
/* -------------------------------------------------------------------- */
ulong getLLCount(void *list)
	{
	ulong i = 0;

	while (list && ++i)
		{
		list = ((struct emptyLL *)list)->next;
		}
	return (i);
	}
