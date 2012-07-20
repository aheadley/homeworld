/*=============================================================================
    Name    : B-Spline.c
    Purpose : Code for spline interpolation

    Created 12/1/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "Vector.h"
#include "Matrix.h"
#include "B-Spline.h"

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : bsStartup
    Description : Start the spline module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bsStartup(void)
{
#if BS_TEST
    char *fileName;
    FILE *fp;
    splinecurve *curve;
    static real32 points[] = {0.0f, 1.0f, -1.0f, -0.5f, 0.0f, -2.0f, -1.2f};
    static real32 times[] = {0.0f,  0.5f,  2.0f,  3.0f, 4.0f,  5.5f,  7.0f, 7.5f, 9.0f};
    static tcb tcbs[] = {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                        {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f}};
    real32 time, value = 0.0f;
    sdword point;

    fileName = filePathPrepend("b-spline.chart", FF_UserSettingsPath);
    fp = fopen(fileName, "wt");
    if (fp != NULL)
    {
        curve = bsCurveStart(7, points, times, tcbs, TRUE);
        for (time = 0.0f; value != REALlyBig; time += 0.02f)
        {
            point = curve->currentPoint;
            value = bsCurveUpdate(curve, 0.02f);
            fprintf(fp, "\nTime = %2.2f, value = %2.8f", time, value);
            if (point != curve->currentPoint)
            {
                fprintf(fp, ", point = %2.2f", curve->points[curve->currentPoint]);
            }
        }
        bsCurveDelete(curve);
        fclose(fp);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : bsShutdown
    Description : Shuts down the spline module
    Inputs      : void
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bsShutdown(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : bsCurveStart
    Description : Start a spline curve blending
    Inputs      : nPoints - number of points in the curve
                  points - array of floats for the points (P0 .. Pn)
                  times - array of times (between points)
                  params - array of parameters(tension, continuity, bias; currently unused)
                  bAlloc - flag indicating the times and params blocks need allocation.
                    The points block is always re-allocated.
    Outputs     : allocates memory for the data specified
    Return      : pointer to newly allocated spline block
    Note        : If the bAlloc flag is set and the points must be allocated,
                  the number of points allocated will actualle be 3 greater than
                  the number of points specified: 1 extra at the start and 2
                  extra at the end.  This is because these curves need an extra
                  control points at the extremes.  Otherwise, it is assumed that
                  this arrays passed are previously allocated by this function
                  and are of valid length.
----------------------------------------------------------------------------*/
splinecurve *bsCurveStart(sdword nPoints, real32 *points, real32 *times, tcb *params, bool bAlloc)
{
    splinecurve *curve;
    sdword size;

    size = sizeof(splinecurve);                             //base size
    if (bAlloc)
    {
        nPoints += 3;                                           //extra control points for blending the extremes
        size += nPoints * (sizeof(real32) * 2 + sizeof(tcb));//increase the size if we have to allocate
    }
    curve = memAlloc(size, "SplineCurve", 0);
    if (bAlloc)
    {                                                       //if arrays in this allocation
        curve->points = (real32 *)((ubyte *)curve + sizeof(splinecurve));
        curve->times = (real32 *)((ubyte *)curve->points + sizeof(real32) * nPoints);

        curve->params = (tcb *)((ubyte *)curve->times + sizeof(real32) * nPoints);

        memcpy(&curve->points[1], points, sizeof(real32) * nPoints - 3);//copy list
        curve->points[0] = curve->points[1];                //duplicate first control point
        curve->points[nPoints - 1] = curve->points[nPoints - 2] =
            curve->points[nPoints - 3];                     //and last points
        memcpy(&curve->times[1], times, sizeof(real32) * nPoints - 3);//copy list
        curve->times[nPoints - 2] = curve->times[nPoints - 2] + 1.0f;//say last segments are 1 second long
        curve->times[nPoints - 1] = curve->times[nPoints - 2] + 1.0f;

        memcpy(&curve->params[1], params, sizeof(tcb) * nPoints - 3);//copy list
        curve->params[0] = curve->params[1];                //duplicate the first and last 2 control parameter blocks
        curve->params[nPoints - 1] = curve->params[nPoints - 2] = curve->params[nPoints - 3];

    }
    else
    {                                                       //else already allocated
        curve->points = points;                             //setup pointers
        curve->times = times;
        curve->params = params;
    }
    curve->timeElapsed = 0.0f;
    curve->currentPoint = 1;                                //first two points are just the extras
    curve->nPoints = nPoints;
    return(curve);
}

/*-----------------------------------------------------------------------------
    Name        : bsCurveStartPrealloced
    Description : Start a spline curve from memory that has already been allocated.
    Inputs      : curve - pointer to curve to initialize
                  nPoints - number of points in animation
                  values - values at the keyframes
                  times - time value array
                  params - tension, continuity, bias parameters
    Outputs     : fills in curve
    Return      : void
    Note        : does not allocate or assume extra points at beginning or end of arrays.
----------------------------------------------------------------------------*/
void bsCurveStartPrealloced(splinecurve *curve, sdword nPoints, real32 *values, real32 *times, tcb *params)
{
    curve->points = values;
    curve->times = times;
    curve->params = params;
    curve->timeElapsed = 0.0f;
    curve->currentPoint = 1;                                //first two points are just the extras
    curve->nPoints = nPoints;
}

/*-----------------------------------------------------------------------------
    Name        : bsHermiteCompute
    Description : Compute some hermite coeffients.
    Inputs      : time - time within key, 0..1
    Outputs     : dest - where to store the results
    Return      :
----------------------------------------------------------------------------*/
void bsHermiteCompute(real32 time, hvector *dest)
{
   real32 t2, t3, z;

   t2 = time * time;
   t3 = time * t2;
   z = 3.0f * t2 - t3 - t3;

   dest->x = 1.0f - z;
   dest->y = z;
   dest->z = t3 - t2 - t2 + time;
   dest->w = t3 - t2;
}

/*-----------------------------------------------------------------------------
    Name        : bsCurveUpdate
    Description : Update a spline curve for a given time elapsed.
    Inputs      : curve - curve to update
                  timeElapsed - time since curve created or last updated.
    Outputs     : May modify internal vector or matrix if a control point passed
    Return      : New value of the spline function, or REALlyBig if the spline
                  has ended.
----------------------------------------------------------------------------*/
real32 bsCurveUpdate(splinecurve *curve, real32 timeElapsed)
{
    static hvector hermiteVector;
    static real32 lastTime = REALlyNegative;
    real32 unscaledTime, scaledTime, keyLength, value;
    sdword currentPoint;
    tcb *key0, *key1;
    static tcb *oldKey0 = NULL;
    static real32 dd0a, dd0b, ds1a, ds1b;
    real32 adj0, adj1, delta;

    dbgAssert(timeElapsed >= 0.0f);
    curve->timeElapsed += timeElapsed;
    //find next point.  Could conceivably pass multiple control points in one frame.
    while (curve->timeElapsed > curve->times[curve->currentPoint + 1])
    {                                                       //if current control point ended
        curve->currentPoint++;                              //update the current point index
        if (curve->currentPoint >= curve->nPoints - 1)
        {                                                   //if all points used up
            return(REALlyBig);                              //this curve all used up
        }
    }
    currentPoint = curve->currentPoint;                     //faster access to point index
    if (currentPoint >= curve->nPoints)
    {
        return(REALlyBig);
    }
    unscaledTime = curve->timeElapsed - curve->times[currentPoint];//get time into current control point
    keyLength = curve->times[currentPoint + 1] - curve->times[currentPoint];
    scaledTime = unscaledTime / (keyLength);                //scale time to 1 second range

    if (lastTime != scaledTime)
    {                                                       //if we need to recompute Hermite coefficients
        bsHermiteCompute(scaledTime, &hermiteVector);       //compute Hermite coefficients
        lastTime = scaledTime;
    }
    key0 = &curve->params[currentPoint];
    key1 = &curve->params[currentPoint + 1];
    adj0 = keyLength / ( curve->times[currentPoint + 1] - curve->times[currentPoint - 1] );
    adj1 = keyLength / ( curve->times[currentPoint + 2] - curve->times[currentPoint] );
    if (key0 != oldKey0)
    {
        dd0a = ( 1.0f - key0->tension )                     //compute numerators for the tangent vector functions later on
             * ( 1.0f + key0->continuity )
             * ( 1.0f + key0->bias );
        dd0b = ( 1.0f - key0->tension )
             * ( 1.0f - key0->continuity )
             * ( 1.0f - key0->bias );
        ds1a = ( 1.0f - key1->tension )
             * ( 1.0f - key1->continuity )
             * ( 1.0f + key1->bias );
        ds1b = ( 1.0f - key1->tension )
             * ( 1.0f + key1->continuity )
             * ( 1.0f - key1->bias );
        oldKey0 = key0;
    }
    delta = curve->points[currentPoint + 1] - curve->points[currentPoint];

    value = hermiteVector.x * curve->points[currentPoint] +
            hermiteVector.y * curve->points[currentPoint + 1] +
            hermiteVector.z * (adj0 * (dd0a * (curve->points[currentPoint] - curve->points[currentPoint - 1])
               + dd0b * delta)) +
            hermiteVector.w * (adj1 * ( ds1a * delta + ds1b *
               ( curve->points[currentPoint + 2] - curve->points[currentPoint + 1])));
    return(value);
}

/*-----------------------------------------------------------------------------
    Name        : bsCurveDelete
    Description : Delete a spline and all it's lists, if applicable
    Inputs      : curve - curve to delete
    Outputs     :
    Return      :
    Note        : If other curves are using this one's control point lists,
                    you'd better free them appropriately.
----------------------------------------------------------------------------*/
void bsCurveDelete(splinecurve *curve)
{
    memFree(curve);
}

/*-----------------------------------------------------------------------------
    Name        : bsCurveRestart
    Description : Restarts a curve at time = 0
    Inputs      : curve - curve to restart
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void bsCurveRestart(splinecurve *curve)
{
    curve->timeElapsed = 0.0f;                              //reset time to zero
    curve->currentPoint = 1;                                //first point is just an extra
}

