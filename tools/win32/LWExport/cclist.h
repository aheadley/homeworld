/*
** CCLIST.H : Header file for CCLIST.CPP.
*/

#ifndef __CCLIST_H
#define __CCLIST_H

#include "string.h"

	class ccNode
	{
		friend class ccList;
	private:
		class ccNode *prev;
		class ccNode *next;
	protected:
	public:
		inline ccNode *GetNext(void)			{ return(next); }
		inline void SetNext(ccNode *n)			{ next = n; }

		inline ccNode *GetPrev(void)				{ return(prev); }
		inline void SetPrev(ccNode *n)			{ prev = n; }

		inline ccNode(void)						{ ; }
		inline virtual ~ccNode(void) = 0		{ ; }
	};

	class ccString : public ccNode
	{
	private:
	protected:
	public:
		char *s;

		inline ccString(void)					{ s = 0; }
		inline ccString(char *q)				{ s = new char [strlen(q) + 1];
													strcpy(s, q); }
		inline ~ccString(void)					{ if(s) delete [] s; }

		inline ccString *GetNext(void)					{ return((ccString *)ccNode::GetNext()); }
	};

	class ccList
	{
	private:
		ccNode *head;
		ccNode *tail;

		unsigned long numElements;

	protected:
	public:

		inline ccNode *GetHead(void)					{ return(head); }
		inline void SetHead(ccNode *n)					{ head = n; }

		inline ccNode *GetTail(void)					{ return(tail); }
		inline void SetTail(ccNode *n)					{ tail = n; }

		inline unsigned long GetNumElements(void)		{ return(numElements); }
		inline void SetNumElements(unsigned long ne)	{ numElements = ne; }
		
		ccList(void);
		~ccList(void);

		ccNode *GetNthElement(unsigned long index);

		void AddNode(ccNode *n);
		void AddHead(ccNode *n);
		void AddTail(ccNode *n);
		
		void FlushNode(ccNode *n);
		void PurgeNode(ccNode *n);

		void SwapNode(ccNode *n0, ccNode *n1);

		void FlushList(void);
		void PurgeList(void);
	};

#endif