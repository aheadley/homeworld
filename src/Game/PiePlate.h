/*=============================================================================
    Name    : Pieplate.h
    Purpose : Definitions for the movement pie-plate

    Created 4/9/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PIEPLATE_H
#define ___PIEPLATE_H   PI

#include "Types.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define PIE_MOVE_NEARTO             0           //enables the 'move near to' variant of the move to command

#ifndef HW_Release

#define PIE_ERROR_CHECKING          1           //general error checking
#define PIE_VERBOSE_LEVEL           2           //control specific output code
#define PIE_VISUALIZE_VERTICAL      0           //draw the vertical through the specified point
#define PIE_VISUALIZE_EXTENTS       1           //visualize universe extents

#else //HW_Debug

#define PIE_ERROR_CHECKING          0           //general error checking
#define PIE_VERBOSE_LEVEL           0           //control specific output code
#define PIE_VISUALIZE_VERTICAL      0           //draw the vertical through the specified point
#define PIE_VISUALIZE_EXTENTS       0           //visualize universe extents

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//default point spec attributes
#define PIE_PizzaDishRadius         0.75f
#define PIE_PizzaSlices             32

#define PIE_PlanePointFudge         0.00001f

#define PIE_LineStipple             0xff00
#define PIE_StippleSpeed            1.5f

#define PIE_OriginSizeCentre        0.1f
#define PIE_OriginSizeDish          0.08f
#define PIE_OriginSizeHeight        0.1f
#define PIE_ShipLineTickSize        0.03f
#define PIE_ClosestDistance         200.0f
#define PIE_DottedDistance          2
#define PIE_VerticalVectorHeight    100000.0f
#define PIE_MaxMoveHorizontal       15000.0f
#define PIE_MaxMoveVertical         15000.0f
#define PIE_PlaneScreenPointIndex   256
#define PIE_NumberCircleLODs        7

#define PSM_Idle                    0
#define PSM_Waiting                 1
#define PSM_XY                      2
#define PSM_Z                       4

#define PIE_UnprojectDepth          0.99

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    real32 screenRadius;
    sdword nSegments;
}
pieplanecirclesegments;

/*=============================================================================
    Data:
=============================================================================*/
//parameters of the point spec overlays
extern sdword piePointSpecMode;
extern sdword pieOldPointSpecMode;
extern sdword piePointSpecMouseReset;
extern real32 piePointSpecZ;
extern vector piePlanePoint;
extern vector pieHeightPoint;
extern udword pieStipplePattern;
extern uword pieLineStipple;
extern real32 pieLineStippleCounter, pieLineStippleSpeed;
extern color piePointLineColor;
extern color pieOriginColor;
extern void (*pieLineDrawCallback)(real32 distance);
extern void (*pieMovementCursorDrawCallback)(real32 distance);
extern void (*piePlaneDrawCallback)(real32 distance);

#if PIE_VISUALIZE_VERTICAL
extern vector pieScreen0, pieScreen1;
extern bool pieOnScreen;
#endif //PIE_VISUALIZE_VERTICAL

#if PIE_VISUALIZE_EXTENTS
extern bool pieVisualizeExtents;
#endif


/*=============================================================================
    Functions
=============================================================================*/
//movement mechanism functions
void piePointModeOnOff(void);
void piePointSpecDraw(void);
void piePointModeToggle(sdword bOn);
void piePointModePause(sdword bPause);
void pieShipDied(struct Ship *ship);
void pieAllShipsToPiePlateDraw(real32 distance);
void piePlaneDraw(real32 distance);
void pieMovementCursorDraw(real32 distance);
void piePointModeToggle(sdword bOn);

void pieStartup(void);

bool pieMovePointClipToLimits(real32 sizeX, real32 sizeY, real32 sizeZ, vector *pointA, vector *pointB);
sdword pieCircleSegmentsCompute(real32 screenRadius);

#endif //___PIEPLATE_H

