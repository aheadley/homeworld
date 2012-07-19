/*
** BTGPolygon.h : Header file for BTGPolygon.cpp.
*/

#ifndef __BTGPolygon_H
#define __BTGPolygon_H

#include "cclist.h"
#include "btgvertex.h"

class BTGPolygon : public ccNode
{
private:
protected:
public:
	BOOL bVisible;
	unsigned long flags;		// flags.
	BTGVertex *v0, *v1, *v2;	// Three vertex pointers for this tri.
	
	enum btgpFlags
	{
		btgpFlags_Selected = 1,

		btgpFlags_Last			// Range checking.
	};

	BTGPolygon(void);
	BTGPolygon(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2);	// Create a polygon from 3 vertices;
	~BTGPolygon(void);

	inline BTGPolygon *GetNext(void)		{ return((BTGPolygon *)ccNode::GetNext()); }
};

#endif