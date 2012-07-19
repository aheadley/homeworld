/*
** CCLIST.CPP : Code to manage C++ double linked lists.
*/

#include "stdafx.h"

#include "cclist.h"

ccList::ccList(void)
{
	head = 0;
	tail = 0;

	numElements = 0;
}

ccList::~ccList(void)
{
	// Automatically purge the contents of the list.  If an application wants
	// to preserve the contents of the list, it should call FlushList() before
	// deleting the list object.  This will release the contents of the list.
	PurgeList();
}

void ccList::AddNode(ccNode *n)
{
	// AddNode just adds to the tail.
	AddTail(n);
}

void ccList::AddHead(ccNode *n)
{
	// New list?
	if(numElements == 0)
	{
		n->prev = 0;
		n->next = 0;
		
		head = n;
		tail = n;
	}
	else
	{
		n->prev = 0;
		n->next = head;

		head->prev = n;

		head = n;
	}

	numElements ++;
}

void ccList::AddTail(ccNode *n)
{
	// New list?
	if(numElements == 0)
	{
		n->prev = 0;
		n->next = 0;
		
		head = n;
		tail = n;
	}
	else
	{
		n->prev = tail;
		n->next = 0;

		tail->next = n;

		tail = n;
	}

	numElements ++;
}

void ccList::FlushNode(ccNode *n)
{
	if(n->prev)
	{
		n->prev->next = n->next;
	}
	else
	{
		// Head node.
		head = n->next;
	}

	if(n->next)
	{
		n->next->prev = n->prev;
	}
	else
	{
		// Tail node.
		tail = n->prev;
	}

	n->prev = 0;
	n->next = 0;

	numElements --;
}

void ccList::PurgeNode(ccNode *n)
{
	FlushNode(n);

	delete n;
}

void ccList::FlushList(void)
{
	ccNode *workNode, *flushNode;

	workNode = head;

	while(workNode)
	{
		flushNode = workNode;

		workNode = workNode->next;

		FlushNode(flushNode);
	}
}

void ccList::PurgeList(void)
{
	ccNode *workNode, *purgeNode;

	workNode = head;

	while(workNode)
	{
		purgeNode = workNode;

		workNode = workNode->next;

		PurgeNode(purgeNode);
	}
}

unsigned long ccList::GetElementIndex(ccNode *n)
{
	unsigned long index = 0;
	ccNode *workNode;

	workNode = head;

	while(workNode)
	{
		if(workNode == n)
			break;

		index ++;
		workNode = workNode->next;
	}

	return(index);
}

ccNode *ccList::GetNthElement(unsigned long index)
{
	ccNode *n;
	unsigned long i = 0;

	n = head;

	while(n)
	{
		if(i == index)
			break;

		i++;
		n = n->next;
	}

	return(n);
}