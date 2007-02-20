// --------------------------------------------------------------------------
// Citadel: LLSup.CPP
//
// Code for dealing with linked lists.
//
// for this to work, the "next" field of the list has to be the first field
// in each node and be a far pointer.


#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// disposeLL()		disposes linked list								
// addLL()			adds a node at the end								
// getNextLL()		gets the next node in the list, or NULL 			
// getLLNum()		gets the nth entry in a list						
// deleteLLNode()	deletes the nth node in a list						
// insertLLNode()	instert before the nth node in a list				
// getLLCount() 	counts the nodes in the list						

typedef struct eLL
	{
	struct eLL *next;
	} emptyLL;


// --------------------------------------------------------------------------
// disposeLL(): Disposes linked list.

void disposeLL(void **list)
	{
	emptyLL *thisNode = (emptyLL *) *list;

	while (thisNode)
		{
		emptyLL *nxt = thisNode->next;
		delete [] thisNode;
		thisNode = nxt;
		}

	*list = NULL;
	}


// --------------------------------------------------------------------------
// addLL(): Adds a node at the end.

void *addLL(void **list, uint nodeSize)
	{
	emptyLL *newNode = (emptyLL *) new char[nodeSize];
	if (newNode)
		{
		memset(newNode, 0, nodeSize);
		}

	if (*list)
		{
		emptyLL *cur = (emptyLL *) *list;

		while (cur->next)
			{
			cur = cur->next;
			}

		cur->next = newNode;
		}
	else
		{
		// new list
		*list = newNode;
		}

	return (newNode);
	}


// --------------------------------------------------------------------------
// getNextLL(): Gets the next node in the list, or NULL.

void *getNextLL(const void *list)
	{
	if (list)
		{
		return (((emptyLL *)list)->next);
		}

	return (NULL);
	}


// --------------------------------------------------------------------------
// getLLNum(): Gets the nth entry in a list.

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


// --------------------------------------------------------------------------
// deleteLLNode(): Deletes the nth node in a list. (1-based)

void deleteLLNode(void **list, ulong n)
	{
	emptyLL *cur = NULL;

	if (*list && n)
		{
		if (n == 1)
			{
			cur = (emptyLL *) *list;
			*list = cur->next;
			}
		else
			{
			emptyLL *prev = (emptyLL *) getLLNum(*list, n - 1);

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


// --------------------------------------------------------------------------
// insertLLNode(): Instert before the nth node in a list.

void *insertLLNode(void **list, ulong n, int nodeSize)
	{
	if (n)
		{
		emptyLL *newNode = (emptyLL *) new char[nodeSize];

		if (newNode)
			{
			memset(newNode, 0, nodeSize);

			if (n == 1)
				{
				newNode->next = (emptyLL *) *list;
				*list = newNode;
				}
			else
				{
				emptyLL *cur = (emptyLL *) getLLNum(*list, n - 1);

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


// --------------------------------------------------------------------------
// getLLCount(): Counts the nodes in the list.

ulong getLLCount(const void *list)
	{
	ulong i = 0;

	while (list && ++i)
		{
		list = ((emptyLL *)list)->next;
		}

	return (i);
	}
