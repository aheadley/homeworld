/*
** BTGVertex.h : Header file for BTGVertex.cpp.
*/

#ifndef __BTGVertex_H
#define __BTGVertex_H

#include "cclist.h"

class BTGVertex : public ccNode
{
private:
protected:
public:
	unsigned long flags;		// flags.
	double	x, y;				// Vertex locations.
	signed long red, green, blue, alpha;		// Color of vertex.
	signed long brightness;

	BOOL bVisible;

	enum btgvFlags
	{
		btgvFlags_Selected = 1,

		btgvFlags_Last			// Range checking.
	};

	BTGVertex(void);
	BTGVertex(CPoint *pPoint);	// Create a vertex at this point;
	~BTGVertex(void);

	inline BTGVertex *GetNext(void)		{ return((BTGVertex *)ccNode::GetNext()); }
};

#endif