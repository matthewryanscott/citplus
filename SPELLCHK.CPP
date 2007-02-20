// --------------------------------------------------------------------------
// Citadel: SpellChk.CPP
//
// Spell checker.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "net.h"
#include "room.h"
#include "domenu.h"


// --------------------------------------------------------------------------
// Contents
//
// spellCheck		The spell checker.

static unsigned int PowersOf2Minus1[] =
	{
	0u, 	1u, 	3u, 	7u, 	15u,	31u,	63u,	127u,
	255u,	511u,	1023u,	2047u,	4095u,	8191u,	16383u, 32767u,
	65535u,
	};

class DictionaryFile
	{
	long offsets[27];
	long FPos;
	FILE *DictFile;

	char ByteBuffer;
	int BitsInByteBuffer;

	char ReadBits(int BitsToRead);
	void Seek(long Position);

#ifdef WINCIT
	TermWindowC *TW;
#endif

public:
#ifdef WINCIT
	DictionaryFile(TermWindowC *NewTW)
#else
	DictionaryFile(void)
#endif
		{
		memset(offsets, 0, sizeof(long) * 27);
		FPos = 0;
		BitsInByteBuffer = 0;
		ByteBuffer = 0;

		DictFile = NULL;

#ifdef WINCIT
		TW = NewTW;
#endif
		}

	~DictionaryFile(void)
		{
		Close();
		}

	Bool Open(const char *FileName);
	void Close(void)
		{
		if (DictFile)
			{
			fclose(DictFile);
			DictFile = NULL;
			}
		}

	Bool Find(char *word);
	void Lookup(char *word, Bool pmt);
	Bool GetWord(char *str, long Position, char First);
	};

Bool DictionaryFile::Open(const char *FileName)
	{
	DictFile = fopen(FileName, FO_RB);

	if (DictFile)
		{
		fseek(DictFile, sizeof(long), SEEK_SET);

		offsets[0] = 0;

		for (int i = 1; i < 26; i++)
			{
			fread(&(offsets[i]), sizeof(long), 1, DictFile);
			}

		const long filesize = filelength(fileno(DictFile)) - (26 * sizeof(long));
		offsets[26] = filesize * 8L / 5L;

		if (offsets[25] < filesize)
			{
			tw()doCR();
			tw()mPrintf(getmsg(MENU_EDIT, 40), FileName);
			return (FALSE);
			}

		return (TRUE);
		}
	else
		{
		tw()doCR();
		tw()mPrintf(getmsg(15), FileName);
		return (FALSE);
		}
	}


// a is word being corrected, b is possible match
static Bool match(char *a, const char *b)
	{
	int lena = strlen(a);
	int lenb = strlen(b);
	int lenmax = max(lena, lenb);
	int lenmin = min(lena, lenb);
	int i, j;

	// no more than three chars diff
	if (lenmin + 3 < lenmax)
		{
		return (FALSE);
		}

	// either a substring of the other
	if (strstr(a, b) || strstr(b, a))
		{
		return (TRUE);
		}

	// insertion/ommision of (a few) letters
	if (/*lena > 4 &&*/ lenmin > 3 && lenmin > lenmax - 3)
		{
		for (i = 0; i < lenmin; i++)
			{
			if (a[i] != b[i])
				{
				break;
				}
			}

		for (j = 0; j < lenmin; j++)
			{
			if (a[lena - j - 1] != b[lenb - j - 1])
				{
				break;
				}
			}

		if (i + 2 > lenmin || (j && i + j > 2 && (i + j + 2 >= lenmin)))
			{
			return (TRUE);
			}
		}

	// transposition of adjacent two letters if same length and too short
	// for above test
	if (lenmin <= 3 && lena == lenb)
		{
		for (i = 0; i < lena - 1; i++)
			{
			int test = FALSE;
			char ch = a[i + 1];
			a[i + 1] = a[i];
			a[i] = ch;

			if (strcmp(a, b) == SAMESTRING)
				{
				test = TRUE;
				}

			a[i] = a[i + 1];
			a[i + 1] = ch;

			if (test)
				{
				return (TRUE);
				}
			}
		}

	// else - just say no
	return (FALSE);
	}


void DictionaryFile::Lookup(char *word, Bool pmt)
	{
	char check[MAXWORDLEN + 1];
	Bool found = FALSE;

	if (pmt)
		{
		tw()mPrintf(getmsg(MENU_EDIT, 41), word);
		}

	if (strlen(word) > 2)
		{
		long NextLetterPos = offsets[word[0] - '@'];

		Seek(offsets[word[0] - 'A']);

		while (GetWord(check, -1, word[0]) && tw()OC.User.CanOutput())
			{
			if (FPos > NextLetterPos)
				{
				break;
				}

			if (match(word, check))
				{
				if (!found)
					{
					found = TRUE;
					tw()prtList(LIST_START);
					}

				tw()prtList(check);
				}

#ifdef WINCIT
			TW->OC.CheckInput(FALSE, TW);
#else
			OC.CheckInput(FALSE);
#endif
			}
		}

	if (found)
		{
		tw()prtList(LIST_END);
		}
	else
		{
		tw()mPrintfCR(getmsg(MENU_EDIT, 42));
		}
	}


char DictionaryFile::ReadBits(int NumBits)
	{
	int ReadValue = 0;

	for (int BitsRead = 0; BitsRead < NumBits;)
		{
		if (!BitsInByteBuffer)
			{
			fread(&ByteBuffer, 1, sizeof(char), DictFile);
			BitsInByteBuffer = 8;
			}

		int BitsToRead = BitsInByteBuffer;

		if (BitsToRead > NumBits - BitsRead)
			{
			BitsToRead = NumBits - BitsRead;
			}

		ReadValue |= (ByteBuffer & (PowersOf2Minus1[BitsToRead])) << BitsRead;

		BitsInByteBuffer -= BitsToRead;
		ByteBuffer = (char) (ByteBuffer >> BitsToRead);

		BitsRead += BitsToRead;
		}

	return ((char) ReadValue);
	}


void DictionaryFile::Seek(long Position)
	{
	fseek(DictFile, sizeof(long) * 26 + (Position * 5 / 8), SEEK_SET);
	BitsInByteBuffer = 0;
	ReadBits((int) ((Position * 5) % 8));
	FPos = Position;
	}


Bool DictionaryFile::GetWord(char *str, long Position, char First)
	{
	char c;

	if (Position != -1)
		{
		Seek(Position);

		do
			{
			c = ReadBits(5);
			FPos++;
			} while (c);
		}

	*(str++) = First;

	do
		{
		c = ReadBits(5);
		FPos++;

		if (c == 27)
			{
			*(str++) = '\'';
			}
		else if (c == 0)
			{
			*(str++) = 0;
			}
		else
			{
			*(str++) = (char) (c + '@');
			}
		} while (c);

	return (TRUE);
	}


Bool DictionaryFile::Find(char *word)
	{
	Bool found = TRUE;

	normalizeString(word);
	stripansi(word);

	if (word[1] && word[0] >= 'A' && word[0] <= 'Z')
		{
		int i = word[0] - 'A';
		char check[MAXWORDLEN + 1];

		if (tw()CurrentUser->IsWordInDictionary(word))
			{
			return (TRUE);
			}

		long low = offsets[i];
		long high = offsets[i + 1];
		found = FALSE;
		long cur, cur2 = 2;

		while (low < high)
			{
			cur = (low + high) / 2;

			GetWord(check, cur - 1, word[0]);

			i = strcmp(word, check);
			if (i < 0)
				{
				high = cur;
				}
			else if (i > 0)
				{
				low = cur;
				}
			else
				{
				found = TRUE;
				low = high;
				}

			if (cur == cur2)
				{
				low = high;
				}

			cur2 = cur;
			}
		}

	return (found);
	}


// a few binary tree routines
typedef struct st
	{
	struct st *left, *right, *parent;
	int Order;							// for balancing. see below.
	char string[1];
	} strTree;

// On balancing: Order is used only in the root node while adding to the
// tree; starting at 1 and incrementing for each new node. When this count
// exceeds BALANCEINTERVAL (below), the tree is balanced.
//
// Order is used internally by the balancing code. It first assigns a count
// starting at 0 to each node's logical order in the tree. We then build a
// new tree, adding in the optimal order. After we are done, the old tree
// is no more, and we have a balanced tree and all is good.
//
// After balancing, Order in the root node is set back to 0, and the counting
// resumes.

#define BALANCEINTERVAL 50

static strTree *doAdd(char *str, strTree *start)
	{
	strTree *thisNode;
	int i = strcmp(str, start->string);

	if (i == 0)
		{
		return (NULL);
		}
	else if (i < 0)
		{
		if (start->left)
			{
			thisNode = doAdd(str, start->left);
			return (thisNode);
			}
		else
			{
			start->left = (strTree *) getMemG(1, sizeof(*start->left) + strlen(str));
			thisNode = start->left;
			}
		}
	else
		{
		if (start->right)
			{
			thisNode = doAdd(str, start->right);
			return (thisNode);
			}
		else
			{
			start->right = (strTree *) getMemG(1, sizeof(*start->right) + strlen(str));
			thisNode = start->right;
			}
		}

	thisNode->parent = start;
	return (thisNode);
	}


static void doAddNode(strTree *New, strTree *Start)
	{
	int i = New->Order - Start->Order;

	if (i == 0)
		{
		return;
		}
	else if (i < 0)
		{
		if (Start->left)
			{
			doAddNode(New, Start->left);
			}
		else
			{
			Start->left = New;
			New->parent = Start;
			}
		}
	else
		{
		if (Start->right)
			{
			doAddNode(New, Start->right);
			}
		else
			{
			Start->right = New;
			New->parent = Start;
			}
		}
	}


static strTree *getFirst(strTree *start)
	{
	strTree *cur;

	if ((cur = start) != NULL)
		{
		while (cur->parent)
			{
			cur = cur->parent;
			}
		while (cur->left)
			{
			cur = cur->left;
			}
		}

	return (cur);
	}


static strTree *getNext(strTree *startNode)
	{
	strTree *cur = NULL, *cur2;

	if (startNode)
		{
		if (startNode->right)
			{
			for (cur = startNode->right; cur->left; cur = cur->left);
			}
		else
			{
			for (cur = startNode->parent, cur2 = startNode; cur; cur = cur->parent)
				{
				if (cur2 == cur->left)
					{
					break;
					}

				cur2 = cur;
				}
			}
		}

	return (cur);
	}


static int TwoToTheNth(int N)
	{
	int Val = 1;

	while (N--)
		{
		Val *= 2;
		}

	return (Val);
	}


static void RemoveFromTree(strTree **Root, strTree *Node)
	{
	if (Node && Root && *Root)
		{
		if (Node->parent)
			{
			if (Node->left && Node->right)
				{
				// bring right leg up to parent. Attach left leg to bottom
				// of right leg.

				Node->right->parent = Node->parent;

				if (Node->parent->left == Node)
					{
					Node->parent->left = Node->right;
					}
				else
					{
					Node->parent->right = Node->right;
					}

				strTree *cur = Node->right;

				while (cur->left)
					{
					cur = cur->left;
					}

				cur->left = Node->left;
				Node->left->parent = cur;
				}
			else if (Node->left)
				{
				// just a tail to the left: point the parent at it.
				if (Node->parent->left == Node)
					{
					Node->parent->left = Node->left;
					}
				else
					{
					Node->parent->right = Node->left;
					}

				Node->left->parent = Node->parent;
				}
			else
				{
				// just a tail to the right; point the parent at it.
				if (Node->parent->left == Node)
					{
					Node->parent->left = Node->right;
					}
				else
					{
					Node->parent->right = Node->right;
					}

				if (Node->right)
					{
					Node->right->parent = Node->parent;
					}
				}
			}
		else
			{
			if (Node->left && Node->right)
				{
				// bring right leg up to root. Attach left leg to bottom
				// of right leg.

				Node->right->parent = NULL;
				*Root = Node->right;

				strTree *cur = Node->right;

				while (cur->left)
					{
					cur = cur->left;
					}

				cur->left = Node->left;
				Node->left->parent = cur;
				}
			else if (Node->left)
				{
				// just a tail to the left: new root.
				*Root = Node->left;
				Node->left->parent = NULL;
				}
			else
				{
				// just a tail to the right: new root.
				*Root = Node->right;

				if (Node->right)
					{
					Node->right->parent = NULL;
					}
				}
			}

		Node->parent = Node->left = Node->right = NULL;
		}
	}


static void AddToTree(strTree **Root, strTree *Node)
	{
	if (*Root)
		{
		doAddNode(Node, *Root);
		}
	else
		{
		*Root = Node;
		Node->parent = Node->left = Node->right = NULL;
		}
	}


static void AddToBalancedTree(strTree **OldRoot, strTree **NewRoot, int Node, int Space)
	{
	strTree *cur = getFirst(*OldRoot);

	while (cur && cur->Order < Node)
		{
		cur = getNext(cur);
		}

	if (cur)
		{
		RemoveFromTree(OldRoot, cur);
		AddToTree(NewRoot, cur);
		}

	if (Space > 1)
		{
		Space /= 2;

		AddToBalancedTree(OldRoot, NewRoot, Node - Space, Space);
		AddToBalancedTree(OldRoot, NewRoot, Node + Space, Space);
		}
	}


static void BalanceTree(strTree **root)
	{
	strTree *cur;
	int NodeCounter;

	for (NodeCounter = 1, cur = getFirst(*root); cur;
			cur = getNext(cur), NodeCounter++)
		{
		cur->Order = NodeCounter;
		}
	NodeCounter--;

	int Depth;
	for (Depth = 0; Depth < INT_MAX; Depth++)
		{
		if (TwoToTheNth(Depth) - 1 >= NodeCounter)
			{
			break;
			}
		}

	// Overengineering, really.
	if (Depth != INT_MAX)
		{
		strTree *NewRoot = NULL;

		AddToBalancedTree(root, &NewRoot, TwoToTheNth(Depth) / 2,
				TwoToTheNth(Depth) / 2);

		assert(!*root);

		*root = NewRoot;
		}

	(*root)->Order = 0;
	}


static void addToTree(char *str, strTree **root)
	{
	strTree *cur;

	if (!*root)
		{
		*root = (strTree *) getMemG(1, sizeof(**root) + strlen(str));
		cur = *root;
		}
	else
		{
		cur = doAdd(str, *root);
		}

	if (cur)
		{
		strcpy(cur->string, str);

		if (++(*root)->Order > BALANCEINTERVAL)
			{
			BalanceTree(root);
			}
		}
	}


static void disposeTree(strTree *cur)
	{
	if (cur)
		{
		disposeTree(cur->left);
		disposeTree(cur->right);
		freeMemG(cur);
		}
	}


// --------------------------------------------------------------------------
// spellCheck(): The spell checker.
//
// Notes:
//	Assumes MENU_EDIT is locked.

void TERMWINDOWMEMBER spellCheck(char *buf, int lim)
	{
	strTree *words = NULL, *cur;
	label dictfile;
	char FullPath[128];

	CurrentRoom->GetDictionary(dictfile, sizeof(dictfile));

	if (!*dictfile)
		{
		CopyStringToBuffer(dictfile, cfg.dictionary);
		}

	sprintf(FullPath, sbs, cfg.lexpath, dictfile);

#ifdef WINCIT
	DictionaryFile Dictionary(this);
#else
	DictionaryFile Dictionary;
#endif

	if (Dictionary.Open(FullPath))
		{
		Bool found = FALSE, Checking = TRUE;
		char *ptr = buf;

		do
			{
			char word[MAXWORDLEN + 1];
			Bool HasLower = FALSE, HasDigit = FALSE, IsAnsi = FALSE;
			Bool IsExtendedMCI = FALSE;

			int i;
			for (i = 0; *ptr && i < MAXWORDLEN && !isspace(*ptr) &&
					(!ispunct(*ptr) || *ptr == '\'');
					i++, ptr++)
				{
				if (IsAnsi)
					{
					if (i == 1)
						{
						if (toupper(*ptr) == 'X')
							{
							IsExtendedMCI = TRUE;
							}
						else if (*ptr == TERM_SPLCHK_OFF[0])
							{
							Checking = FALSE;
							}
						else if (*ptr == TERM_SPLCHK_ON[0])
							{
							Checking = TRUE;
							}
						}
					else if (toupper(*ptr) == 'X')
						{
						IsExtendedMCI = FALSE;
						}

					if (!IsExtendedMCI)
						{
						break;
						}
					}

				if (!i && ((*ptr == CTRL_A) || (*ptr == CTRL_B)))
					{
					IsAnsi = TRUE;
					}

				if (islower(*ptr))
					{
					HasLower = TRUE;
					}

				if (isdigit(*ptr))
					{
					HasDigit = TRUE;
					}

				word[i] = (char) toupper(*ptr);
				}
			ptr++;

			if (!IsAnsi && Checking &&
					(HasLower || CurrentUser->IsCheckAllCaps()) &&
					(!HasDigit || CurrentUser->IsCheckDigits()))
				{
				word[i] = 0;

				if (word[1] && word[0] >= 'A' && word[0] <= 'Z')
					{
					if (word[i - 2] == '\'' && word[i - 1] == 'S' &&
							!CurrentUser->IsCheckApostropheS())
						{
						word[i - 2] = 0;
						}

					addToTree(word, &words);
					}
				}
			} while (*ptr);

		int TotalWords = 0;
		for (cur = getFirst(words); cur; cur = getNext(cur), TotalWords++);

		OC.SetOutFlag(OUTOK);

		CRmPrintfCR(getmsg(MENU_EDIT, 43), ltoac(TotalWords),
				TotalWords == 1 ? ns : justs);

		time_t LastUpdate = time(NULL);
		int UpdateLen = 0, WordsChecked = 0;
		#define UPDATETIME 1

		for (cur = getFirst(words); cur && OC.User.CanOutput();
				cur = getNext(cur))
			{
			WordsChecked++;

			if (!Dictionary.Find(cur->string))
				{
				while (UpdateLen--)
					{
					doBS();
					}
				UpdateLen = 0;

				if (CurrentUser->GetSpellCheckMode() == 0 ||
						CurrentUser->GetSpellCheckMode() == 3)
					{
					char str[80];

					found = TRUE;

					sprintf(str, getmsg(MENU_EDIT, 44), cur->string);

					const int a = getYesNo(str, 4);

					if (a == 0)
						{
						if (getYesNo(getmsg(MENU_EDIT, 45), 0))
							{
							if (!CurrentUser->AddWordToDictionary(cur->string))
								{
								OutOfMemory(43);
								}
							}
						}
					else if (a == 1)
						{
						CRmPrintf(getmsg(MENU_EDIT, 47));
						Dictionary.Lookup(cur->string, FALSE);

						replaceString(buf, lim, TRUE, cur->string);
						}
					else
						{
						// abort...
						break;
						}
					}
				else if (CurrentUser->GetSpellCheckMode() == 1) // regular
					{
					if (!found)
						{
						CRmPrintfCR(getmsg(MENU_EDIT, 48));
						found = TRUE;
						}

					Dictionary.Lookup(cur->string, TRUE);
					}
				else											// terse
					{
					if (!found)
						{
						CRmPrintfCR(getmsg(MENU_EDIT, 49));
						found = TRUE;
						prtList(LIST_START);
						}

					prtList(cur->string);
					}

				LastUpdate = time(NULL);
				}
			else
				{
				if (time(NULL) >= LastUpdate + UPDATETIME)
					{
					while (UpdateLen--)
						{
						doBS();
						}

					label String;
					sprintf(String, getmsg(347), ltoac(WordsChecked));

					mPrintf(String);
					UpdateLen = strlen(String);

					LastUpdate = time(NULL);
					}
				}
			}

		while (UpdateLen--)
			{
			doBS();
			}

		if (!found)
			{
			CRmPrintfCR(getmsg(MENU_EDIT, 50));
			}
		else
			{
			if (CurrentUser->GetSpellCheckMode() == 0 ||
					CurrentUser->GetSpellCheckMode() == 3)	// verbose
				{

				}
			else if (CurrentUser->GetSpellCheckMode() == 1)	// regular
				{

				}
			else											// terse
				{
				prtList(LIST_END);
				}
			}

		disposeTree(words);
		}
	else
		{
		CRmPrintfCR(getmsg(MENU_EDIT, 51), dictfile);
		}
	}
