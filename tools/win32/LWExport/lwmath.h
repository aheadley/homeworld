/*
** LWMATH.H : Header file for LWMATH.CPP.
*/

#ifndef __LWMATH_H
#define __LWMATH_H

#include "lwoparse.h"

//#define PI				((float)3.14159)
//#define TWOPI			(PI * 2)
#define HALFPI			(PI / 2)

#define vecCrossProduct(result,a,b) \
    (result)##.x = ((a)##.y * (b)##.z) - ((a)##.z * (b)##.y); \
    (result)##.y = ((a)##.z * (b)##.x) - ((a)##.x * (b)##.z); \
    (result)##.z = ((a)##.x * (b)##.y) - ((a)##.y * (b)##.x)

#define vecMagnitudeSquared(a) \
    ( ((a)##.x * (a)##.x) + ((a)##.y * (a)##.y) + ((a)##.z * (a)##.z) )


	void vecNormalize(LWONormal *a);

	char CompareNormals(LWONormal *n1, LWONormal *n2);

#endif