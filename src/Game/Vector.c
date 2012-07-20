/*=============================================================================
    VECTOR.C: VECTOR functions

    Created June 1997 by Gary Shaw
=============================================================================*/

#include <stdio.h>
#include <math.h>
#include "Types.h"
#include "Vector.h"
#include "Debug.h"
#include "FastMath.h"
#include "Globals.h"

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : vecNormalize
    Description : normalizes a vector (makes it a unit vector)
    Inputs      : vector
    Outputs     : vector is normalized
    Return      :
----------------------------------------------------------------------------*/
void vecNormalize(vector *a)
{
    real32 mag = fsqrt(vecMagnitudeSquared(*a));
    real32 oneOverMag;

#ifdef OPTIMIZE_VERBOSE
   // vecNormalizeCounter++;
#endif
    if (mag == 0.0f)
    {
        return;
    }

    oneOverMag = 1.0f / mag;

    a->x *= oneOverMag;
    a->y *= oneOverMag;
    a->z *= oneOverMag;
}

/*-----------------------------------------------------------------------------
    Name        : vecHomogenize
    Description : homogenizes an hvector (to the w == 1 plane)
    Inputs      : dst - destination vector
                  src - source hvector
    Outputs     : dst is homogenized from src
    Return      :
----------------------------------------------------------------------------*/
void vecHomogenize(vector* dst, hvector* src)
{
    real32 oneOverW;

    vecGrabVecFromHVec(*dst, *src);
    if (src->w == 0.0f || src->w == 1.0f)
    {
        return;
    }

    oneOverW = 1.0f / src->w;
    dst->x *= oneOverW;
    dst->y *= oneOverW;
    dst->z *= oneOverW;
}

/*-----------------------------------------------------------------------------
    Name        : vecCopyAndNormalize
    Description : copies a source vector to destination vector, and then
                  normalizes the destination vector
    Inputs      : src, dst
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCopyAndNormalize(vector *src,vector *dst)
{
    real32 mag = fsqrt(vecMagnitudeSquared(*src));
    real32 oneOverMag = 1.0f / mag;

    dst->x = src->x * oneOverMag;
    dst->y = src->y * oneOverMag;
    dst->z = src->z * oneOverMag;
}

/*-----------------------------------------------------------------------------
    Name        : vecNormalizeToLength
    Description : Normalizes vector to given length.
    Inputs      : vector, length
    Outputs     : vector will now have length length.
    Return      :
----------------------------------------------------------------------------*/
void vecNormalizeToLength(vector *a,real32 length)
{
    real32 mag = fsqrt(vecMagnitudeSquared(*a));
    real32 ratio = length / mag;

    a->x *= ratio;
    a->y *= ratio;
    a->z *= ratio;
}

/*-----------------------------------------------------------------------------
    Name        : vecCapVectorSloppy
    Description : limits a vector's magnitude from growing beyond a maximum
                  magnitude by chopping x,y,z separately.
    Inputs      : vectorToCap, maxMagnitude
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCapVectorSloppy(vector *vectorToCap,real32 maxMagnitude)
{
    real32 negMaxMagnitude = -maxMagnitude;

    if (vectorToCap->x > maxMagnitude)
    {
        vectorToCap->x = maxMagnitude;
    }
    else if (vectorToCap->x < negMaxMagnitude)
    {
        vectorToCap->x = negMaxMagnitude;
    }

    if (vectorToCap->y > maxMagnitude)
    {
        vectorToCap->y = maxMagnitude;
    }
    else if (vectorToCap->y < negMaxMagnitude)
    {
        vectorToCap->y = negMaxMagnitude;
    }

    if (vectorToCap->z > maxMagnitude)
    {
        vectorToCap->z = maxMagnitude;
    }
    else if (vectorToCap->z < negMaxMagnitude)
    {
        vectorToCap->z = negMaxMagnitude;
    }
}

/*-----------------------------------------------------------------------------
    Name        : vecCapVector
    Description : limits a vector's magnitude from growing beyond a maximum
                  magnitude
    Inputs      : vectorToCap, maxMagnitude
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCapVector(vector *vectorToCap,real32 maxMagnitude)
{
    real32 actualMag = fsqrt(vecMagnitudeSquared(*vectorToCap));
    real32 ratio;

    if (actualMag > maxMagnitude)
    {
        ratio = maxMagnitude / actualMag;
        vectorToCap->x *= ratio;
        vectorToCap->y *= ratio;
        vectorToCap->z *= ratio;
    }
}

/*-----------------------------------------------------------------------------
    Name        : vecCapVectorWithMag
    Description : limits a vector's magnitude from growing beyond a maximum
                  magnitude
    Inputs      : vectorToCap, maxMagnitude, actualMag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCapVectorWithMag(vector *vectorToCap,real32 maxMagnitude,real32 actualMag)
{
    real32 ratio;

    if (actualMag > maxMagnitude)
    {
        ratio = maxMagnitude / actualMag;
        vectorToCap->x *= ratio;
        vectorToCap->y *= ratio;
        vectorToCap->z *= ratio;
    }
}

/*-----------------------------------------------------------------------------
    Name        : vecCapMinVector
    Description : limits a vector's minimum magnitude from growing below a maximum
                  magnitude
    Inputs      : vectorToCap, minMagnitude
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCapMinVector(vector *vectorToCap,real32 minMagnitude)
{
    real32 actualMag = fsqrt(vecMagnitudeSquared(*vectorToCap));
    real32 ratio;

    if (actualMag < minMagnitude)
    {
        ratio = minMagnitude / actualMag;
        vectorToCap->x *= ratio;
        vectorToCap->y *= ratio;
        vectorToCap->z *= ratio;
    }
}

/*-----------------------------------------------------------------------------
    Name        : vecCapMinMaxVector
    Description : limits both the vector's minimum and maximum magnitude
    Inputs      : vectorToCap, minMagnitude, maxMagnitude
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void vecCapMinMaxVector(vector *vectorToCap,real32 minMagnitude,real32 maxMagnitude)
{
    real32 actualMag = fsqrt(vecMagnitudeSquared(*vectorToCap));
    real32 ratio;

    if (actualMag < minMagnitude)
    {
        ratio = minMagnitude / actualMag;
        vectorToCap->x *= ratio;
        vectorToCap->y *= ratio;
        vectorToCap->z *= ratio;
    }

    if (actualMag > maxMagnitude)
    {
        ratio = maxMagnitude / actualMag;
        vectorToCap->x *= ratio;
        vectorToCap->y *= ratio;
        vectorToCap->z *= ratio;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getVectDistSloppy
    Description : given a distance vector, gives a sloppy approximation of the distance
    Inputs      : diff
    Outputs     :
    Return      : sloppy approximation of the distance
----------------------------------------------------------------------------*/
real32 getVectDistSloppy(vector diff)
{
    diff.x = ABS(diff.x);
    diff.y = ABS(diff.y);
    diff.z = ABS(diff.z);

    if (diff.x > diff.y)
    {
        if (diff.x > diff.z)
        {
            return diff.x;
        }
        else
        {
            return diff.z;
        }
    }
    else
    {
        if (diff.y > diff.z)
        {
            return diff.y;
        }
        else
        {
            return diff.z;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : vecLineIntersectWithXYPlane
    Description : finds the point of intersection between a line and the
                  XY plane at z=zp.  The line is defined with the two points
                  linepoint1 and linepoint2.
    Inputs      : linepoint1, linepoint2, zp
    Outputs     : result, point of intersection
    Return      :
----------------------------------------------------------------------------*/
void vecLineIntersectWithXYPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 zp)
{
    real32 k;

    k = (zp - linepoint1->z) / (linepoint2->z - linepoint1->z);

    result->x = (linepoint2->x - linepoint1->x)*k + linepoint1->x;
    result->y = (linepoint2->y - linepoint1->y)*k + linepoint1->y;
    result->z = zp;
}

/*-----------------------------------------------------------------------------
    Name        : vecLineIntersectWithYZPlane
    Description : finds the point of intersection between a line and the
                  YZ plane at x=xp.  The line is defined with the two points
                  linepoint1 and linepoint2.
    Inputs      : linepoint1, linepoint2, xp
    Outputs     : result, point of intersection
    Return      :
----------------------------------------------------------------------------*/
void vecLineIntersectWithYZPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 xp)
{
    real32 k;

    k = (xp - linepoint1->x) / (linepoint2->x - linepoint1->x);

    result->y = (linepoint2->y - linepoint1->y)*k + linepoint1->y;
    result->z = (linepoint2->z - linepoint1->z)*k + linepoint1->z;
    result->x = xp;
}

/*-----------------------------------------------------------------------------
    Name        : vecLineIntersectWithXZPlane
    Description : finds the point of intersection between a line and the
                  XZ plane at y=yp.  The line is defined with the two points
                  linepoint1 and linepoint2.
    Inputs      : linepoint1, linepoint2, yp
    Outputs     : result, point of intersection
    Return      :
----------------------------------------------------------------------------*/
void vecLineIntersectWithXZPlane(vector *result,vector *linepoint1,vector *linepoint2,real32 yp)
{
    real32 k;

    k = (yp - linepoint1->y) / (linepoint2->y - linepoint1->y);

    result->x = (linepoint2->x - linepoint1->x)*k + linepoint1->x;
    result->z = (linepoint2->z - linepoint1->z)*k + linepoint1->z;
    result->y = yp;
}

/*-----------------------------------------------------------------------------
    Name        : vecLineIntersectWithPlane
    Description : Compute the intersection of an arbitrary plane and an
                    arbitrary ray.
    Inputs      : Vplane - point on plane
                  Vnormal - normal to plane
                  Vline - point on line
                  Vdirection - direction of line
    Outputs     : dest - intersection point
    Return      : will generate error if there is no intersection (line is
                    paralell to plane)
----------------------------------------------------------------------------*/
void vecLineIntersectWithPlane(vector *dest, vector *Vplane, vector *Vnormal, vector *Vline, vector *Vdirection)
{
    real32 tScalar, value;
    vector temp;

    value = vecDotProduct(*Vdirection, *Vnormal);
    dbgAssert(value != 0.0f);

    vecSub(temp, *Vline, *Vplane);

    //t = _-(Rl-Rp).N_
    //         V.N
    tScalar = -vecDotProduct(temp, *Vnormal) / value;

    //R = Rl + tV
    dest->x = Vline->x + tScalar * Vdirection->x;
    dest->y = Vline->y + tScalar * Vdirection->y;
    dest->z = Vline->z + tScalar * Vdirection->z;
}

/*-----------------------------------------------------------------------------
    Name        : vecVectorsBlend
    Description : Linearly blend between two vectors.
    Inputs      : start, end - endpoints of the interpolation
                  factor - amount of end to use
    Outputs     : result - where the interpolated result is stored.
    Return      :
----------------------------------------------------------------------------*/
void vecVectorsBlend(vector *result, vector *start, vector *end, real32 factor)
{
    real32 oneMinusFactor = 1.0f - factor;
    result->x = start->x * oneMinusFactor + end->x * factor;
    result->y = start->y * oneMinusFactor + end->y * factor;
    result->z = start->z * oneMinusFactor + end->z * factor;
}

