/*=============================================================================
    VECTOR.C: VECTOR functions

    Created June 1997 by Gary Shaw
=============================================================================*/

#include <stdio.h>
#include <math.h>
#include "types.h"
#include "vector.h"
#include "fastmath.h"

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : vecPrintVector
    Description : prints a vector
    Inputs      : vector
    Outputs     :
    Return      :		
----------------------------------------------------------------------------*/
void vecPrintVector(vector *a)
{
    printf("(%f,%f,%f)\n",a->x,a->y,a->z);
}

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
    real32 oneOverMag = 1.0f / mag;

    a->x *= oneOverMag;
    a->y *= oneOverMag;
    a->z *= oneOverMag;
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
    diff.x = abs(diff.x);
    diff.y = abs(diff.y);
    diff.z = abs(diff.z);

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

