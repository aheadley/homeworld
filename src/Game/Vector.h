/*=============================================================================
    VECTOR.H: Definitions for VECTOR.C

    Created June 1997 by Gary Shaw
=============================================================================*/

#ifndef ___VECTOR_H
#define ___VECTOR_H

#include "Types.h"

/*=============================================================================
    Type definitions:
=============================================================================*/

typedef struct
{
    real32 x,y,z;
} vector;

typedef struct
{
    real32 x,y,z,w;
} hvector;

/*=============================================================================
    Macros
=============================================================================*/

#define vecSet(v,xp,yp,zp)  \
    (v).x = (xp);         \
    (v).y = (yp);         \
    (v).z = (zp)

#define vecGrabVecFromHVec(v,h) \
    (v).x = (h).x;          \
    (v).y = (h).y;          \
    (v).z = (h).z

#define vecMakeHVecFromVec(h,v) \
    (h).x = (v).x;          \
    (h).y = (v).y;          \
    (h).z = (v).z;          \
    (h).w = 1.0f;

#define vecZeroVector(a) \
    (a).x = 0.0f;     \
    (a).y = 0.0f;     \
    (a).z = 0.0f

#define vecAdd(a,b,c) \
    (a).x = (b).x + (c).x; \
    (a).y = (b).y + (c).y; \
    (a).z = (b).z + (c).z

#define vecAddTo(a,b) \
    (a).x += (b).x; \
    (a).y += (b).y; \
    (a).z += (b).z

#define vecSub(a,b,c) \
    (a).x = (b).x - (c).x; \
    (a).y = (b).y - (c).y; \
    (a).z = (b).z - (c).z

#define vecSubFrom(a,b) \
    (a).x -= (b).x; \
    (a).y -= (b).y; \
    (a).z -= (b).z

#define vecDotProduct(a,b) \
    ( ((a).x * (b).x) + ((a).y * (b).y) + ((a).z * (b).z) )

#define vecCrossProduct(result,a,b) \
    (result).x = ((a).y * (b).z) - ((a).z * (b).y); \
    (result).y = ((a).z * (b).x) - ((a).x * (b).z); \
    (result).z = ((a).x * (b).y) - ((a).y * (b).x)

#define vecAddToScalarMultiply(dstvec,vec,k) \
    (dstvec).x += ((vec).x * (k));       \
    (dstvec).y += ((vec).y * (k));       \
    (dstvec).z += ((vec).z * (k))

#define vecSubFromScalarMultiply(dstvec,vec,k) \
    (dstvec).x -= ((vec).x * (k));       \
    (dstvec).y -= ((vec).y * (k));       \
    (dstvec).z -= ((vec).z * (k))

#define vecScalarMultiply(dstvec,vec,k) \
    (dstvec).x = (vec).x * (k);       \
    (dstvec).y = (vec).y * (k);       \
    (dstvec).z = (vec).z * (k)

#define vecMultiplyByScalar(vec,k) \
    (vec).x *= (k);                \
    (vec).y *= (k);                \
    (vec).z *= (k)

#define vecScalarDivide(dstvec,vec,k,tmp) \
    (tmp) = 1.0f / (k);                 \
    (dstvec).x = (vec).x * (tmp);     \
    (dstvec).y = (vec).y * (tmp);     \
    (dstvec).z = (vec).z * (tmp)

#define vecDivideByScalar(vec,k,tmp) \
    (tmp) = 1.0f / (k);              \
    (vec).x *= (tmp);              \
    (vec).y *= (tmp);              \
    (vec).z *= (tmp)

#define vecNegate(vec) \
    (vec).x = -(vec).x; \
    (vec).y = -(vec).y; \
    (vec).z = -(vec).z

#define vecCopyAndNegate(dstvec,vec) \
    (dstvec).x = -(vec).x; \
    (dstvec).y = -(vec).y; \
    (dstvec).z = -(vec).z

#define vecMagnitudeSquared(a) \
    ( ((a).x * (a).x) + ((a).y * (a).y) + ((a).z * (a).z) )

#define vecAreEqual(a,b) \
    ( ((a).x == (b).x) && ((a).y == (b).y) && ((a).z == (b).z) )

#define vecIsZero(a) \
    ( ((a).x == 0.0f) && ((a).y == 0.0f) && ((a).z == 0.0f) )

/*=============================================================================
    Functions:
=============================================================================*/

void vecPrintVector(vector *a);
void vecNormalize(vector *a);
void vecHomogenize(vector* dst, hvector* src);
void vecCopyAndNormalize(vector *src,vector *dst);
void vecNormalizeToLength(vector *a,real32 length);
void vecCapVectorSloppy(vector *vectorToCap,real32 maxMagnitude);
void vecCapVector(vector *vectorToCap,real32 maxMagnitude);
void vecCapVectorWithMag(vector *vectorToCap,real32 maxMagnitude,real32 actualMag);
void vecCapMinVector(vector *vectorToCap,real32 minMagnitude);
void vecCapMinMaxVector(vector *vectorToCap,real32 minMagnitude,real32 maxMagnitude);
real32 getVectDistSloppy(vector diff);
void vecLineIntersectWithXYPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 zp);
void vecLineIntersectWithYZPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 xp);
void vecLineIntersectWithXZPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 yp);
void vecLineIntersectWithPlane(vector *dest, vector *Vplane, vector *Vnormal, vector *Vline, vector *Vdirection);
void vecVectorsBlend(vector *result, vector *start, vector *end, real32 factor);

#endif //___VECTOR_H

