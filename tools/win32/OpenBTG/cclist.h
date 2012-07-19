/*
** CCLIST.H : Header file for CCLIST.CPP.
*/

#ifndef __CCLIST_H
#define __CCLIST_H

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

	class ccList
	{
	private:
		ccNode *head;
		ccNode *tail;

		signed long numElements;

	protected:
	public:

		inline ccNode *GetHead(void)					{ return(head); }
		inline void SetHead(ccNode *n)					{ head = n; }

		inline ccNode *GetTail(void)					{ return(tail); }
		inline void GetTail(ccNode *n)					{ tail = n; }

		inline unsigned long GetNumElements(void)		{ return(numElements); }
		inline void SetNumElements(unsigned long ne)	{ numElements = ne; }
		
		ccList(void);
		~ccList(void);

		void AddNode(ccNode *n);
		void AddHead(ccNode *n);
		void AddTail(ccNode *n);

		void FlushNode(ccNode *n);
		void PurgeNode(ccNode *n);

		void FlushList(void);
		void PurgeList(void);

		ccNode *GetNthElement(unsigned long index);
		unsigned long GetElementIndex(ccNode *n);
	};

#endif