/*
** BTGLight.h : Header file for BTGLight.cpp.
*/

#ifndef __BTGLIGHT_H
#define __BTGLIGHT_H

#include "cclist.h"

class BTGLight : public ccNode
{
private:
protected:
public:
	unsigned long flags;		// flags.
	double	x, y;				// Light locations.
    signed long dred, dgreen, dblue; // Diffuse colour.
    signed long ared, agreen, ablue; // Ambient colour.
    signed long dinten, ainten;      // Diffuse, Ambient intensities.

	BOOL bVisible;

	enum btglFlags
	{
		btglFlags_Selected = 1,

		btglFlags_Last			// Range checking.
	};

	BTGLight(void);
	BTGLight(CPoint *pPoint);	// Create a light at this point;
	~BTGLight(void);

	inline BTGLight *GetNext(void)		{ return((BTGLight *)ccNode::GetNext()); }
};

#endif