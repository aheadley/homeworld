/*
** BTGStar.h : Header file for BTGStar.cpp.
*/

#ifndef __BTGSTAR_H
#define __BTGSTAR_H

#include "cclist.h"
#include "tgacontainer.h"

class BTGStar : public ccNode
{
private:
protected:
public:
	unsigned long flags;		// flags.
	double	x, y;				// Vertex locations.
	signed long red, green, blue, alpha;		// Color of vertex.
	CTGAContainer *pMyStar;

	BOOL bVisible;

	enum btgvFlags
	{
		btgsFlags_Selected = 1,

		btgsFlags_Last			// Range checking.
	};

	BTGStar(char *fileName);
	BTGStar(CPoint *pPoint, char *fileName);	// Create a vertex at this point;
	~BTGStar(void);

	inline BTGStar *GetNext(void)		{ return((BTGStar *)ccNode::GetNext()); }
};

#endif