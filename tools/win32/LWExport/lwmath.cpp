/*
** LWMATH.CPP : Code for various math routines.
*/

#include "math.h"

#include "lwmath.h"
#include "lwoparse.h"

void vecNormalize(LWONormal *a)
{
    float mag = (float)sqrt(vecMagnitudeSquared(*a));

    a->x /= mag;
    a->y /= mag;
    a->z /= mag;
}

#define FUDGE	(0.001)
char CompareNormals(LWONormal *n1, LWONormal *n2)
{
	return(((n1->x >= n2->x - FUDGE) && (n1->x <= n2->x + FUDGE)) && 
		   ((n1->y >= n2->y - FUDGE) && (n1->y <= n2->y + FUDGE)) && 
		   ((n1->z >= n2->z - FUDGE) && (n1->z <= n2->z + FUDGE)));
}
