/*=============================================================================
    Name    : b-spline.h
    Purpose : Code for generic interpolation of beta-spline curves.

    Created 11/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___B_SPLINE_H
#define ___B_SPLINE_H    444

/*=============================================================================
    Switches:
=============================================================================*/

#define BS_TEST                     0           //test the spline module

#ifndef HW_Release

#define BS_ERROR_CHECKING           1           //general error checking
#define BS_VERBOSE_LEVEL            1           //control specific output code

#else //HW_Debug

#define BS_ERROR_CHECKING           0           //general error checking
#define BS_VERBOSE_LEVEL            0           //control specific output code

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//flags for spline curves
#define BS_NoPoint                  -1

/*=============================================================================
    Type definitions:
=============================================================================*/

//parameters for a control point
typedef struct
{
    real32 tension;
    real32 continuity;
    real32 bias;
}
tcb;

//info for a single spline curve
typedef struct
{
    real32 timeElapsed;
    sdword currentPoint;
    sdword nPoints;
//    hmatrix *currentM;
//    hvector *currentG;
    real32 *points;
    real32 *times;
    tcb *params;
}
splinecurve;

/*=============================================================================
    Functions:
=============================================================================*/

void bsStartup();
void bsShutdown();

splinecurve *bsCurveStart(sdword nPoints, real32 *points, real32 *times, tcb *params, bool bAlloc);
void bsCurveStartPrealloced(splinecurve *curve, sdword nPoints, real32 *values, real32 *times, tcb *params);
void bsCurveRestart(splinecurve *curve);
real32 bsCurveUpdate(splinecurve *curve, real32 timeElapsed);
void bsCurveDelete(splinecurve *curve);

#endif //___B_SPLINE_H

