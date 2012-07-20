/*=============================================================================
    Name    : Sensors.h
    Purpose : Definitions for the sensors manager

    Created 10/8/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SENSORS_H
#define ___SENSORS_H

#include "Types.h"
#include "FEFlow.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define SM_TEST                     1           //test this module
#ifndef HW_Release

#define SM_ERROR_CHECKING           1           //general error checking
#define SM_VERBOSE_LEVEL            2           //control specific output code
#define SM_TOGGLE_SENSORLEVEL       1           //enable toggling the sensprs level with the L key

#else //HW_Debug

#define SM_ERROR_CHECKING           0           //general error checking
#define SM_VERBOSE_LEVEL            0           //control specific output code
#define SM_TOGGLE_SENSORLEVEL       0           //enable toggling the sensprs level with the L key

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define SM_ScreenName               "Sensors_manager"
#define SM_FleetIntelScreenName     "FleetIntel"
#define SM_ViewportName             "SM_ViewPort"
#define SM_Dispatch                 "SM_Dispatch"
#define SM_Hyperspace               "SM_Hyperspace"
#define SM_TacticalOverlay          "SM_TacticalOverlay"
#define SM_NonResource              "SM_NonResource"
#define SM_Resource                 "SM_Resource"
#define SM_Close                    "SM_Close"
#define SM_Skip                     "SM_Skip"
#define SM_CancelDispatch           "SM_CancelDispatch"
#define SM_ToggleSensorsLevel       "SM_ToggleSensorsLevel"
#define SM_Pan                      "SM_Pan"
#define SM_CancelMoveOrClose        "SM_CancelMoveOrClose"

#define SM_ViewportFilter           RPE_LeftClickButton | RPE_HoldLeft | RPE_RightClickButton | RPE_HoldRight | RPE_WheelUp | RPE_WheelDown

//defaults for the sensors camera
#define SM_ClipNear                 1.0e+3f
#define SM_ClipFar                  1.0e+6f
#define SM_InitialCameraDist        90000.0f
#define SM_CameraMinDistance        65000.0f
#define SM_CameraMaxDistance        150000.0f
#define SM_NearClipMultipier        1.7f

//sensors manager point specification stuffs
#define SPM_Idle                    0
#define SPM_Pause                   1
#define SPM_Active                  2
#define SM_CrosshairSize            32

//default colors of differing mission spheres
#define SPC_Normal                  colRGB(40, 40, 40)
#define SPC_Selected                colRGB(255, 255, 255)
#define SPC_TeamColor               colRGB(32, 176, 64)
#define SM_BlobColor                colRGB(32, 5, 182)
#define SM_BlobColorHigh            colRGB(5, 182, 32)
#define SM_BlobColorLow             colRGB(182, 5, 32)

#define SM_BlobUpdateRate           4
#define SM_OriginDepthCueAmount     0.4f
#define SM_DepthCueRadius           50000.0f
#define SM_CircleBorder             1.373291015625f
#define SM_DepthCueStartRadius      20000.0f
#define SM_ZoomMin                  50000.0f
#define SM_ZoomMax                  250000.0f
#define SM_ZoomMinFactor            8.0f
#define SM_ZoomMaxFactor            4.0f
#define SM_InitialDistance          ((SM_ZoomMax + SM_ZoomMin) / 2.0f)
#define SM_UniverseSizeX            100000.0f
#define SM_UniverseSizeY            100000.0f
#define SM_UniverseSizeZ            100000.0f
#define SM_FocusScalar              4.0f
#define SM_FocusRadius              5000.0f
#define SM_SelectedFlashSpeed       0.8f
#define SM_TickLength               0.1f
#define SM_ClosestDistance          2000.0f

#define SM_PlayerListTextColor      colWhite
#define SM_CursorTextColor          colWhite
#define SM_PlayerListTextMargin     4
#define SM_PlayerListMarginX        4
#define SM_PlayerListMarginY        3
#define SM_TOBottomCornerX          0
#define SM_TOBottomCornerY          0
#define SM_TOLineSpacing            1
#define SM_TOColor                  colWhite

#define SM_ZoomLength               10
#define SM_MainViewZoomLength       5
#define SM_SelectionWidth           10
#define SM_SelectionHeight          10

#define SM_ProjectionScale          0.5f

#define SM_FarthestMargin           750.0f
#define SM_ClosestMargin            750.0f

#define SEL_SegmentsMin             8
#define SEL_SegmentsMax             48

#define SM_LargeResourceSize        45.0f

#define SM_BlurryArraySize          128

#define SM_BlobClosenessFactor      2.0f

#define SM_NumberTOs                64

//for world plane
#define SM_WorldPlaneColor          colRGB(223, 115, 25)
#define SM_HeightCircleColor        colRGB(223, 220, 10)
#define SM_WorldPlaneDistanceFactor 0.6f
#define SM_WorldPlaneSegments       48
#define SM_MovementWorldPlaneDim    0.6f
#define SM_MovePlaneColor           colRGB(6,191,113)
#define SM_RadialTickStart          (PI / 12)
#define SM_RadialTickInc            (PI / 12)
#define SM_TickInnerMult            0.93f
#define SM_TickOuterMult            1.00f
#define SM_TickExtentFactor         1.3f
#define SM_BlobCircleSize           2500.0f
#define SM_ShortFootLength          2500.0f
#define SM_BackgroundDim            0.5f
#define SM_MaxTicksOnScreen         24
#define SM_TickTextChars            8

//for horizon ticks
#define SM_HorizTickAngle           PI / 12.0f
#define SM_HorizTickVerticalFactor  0.01f
#define SM_HorizonLineDistanceFactor 0.99f
#define SM_HorizonTickHorizFactor   0.75f
#define SM_TickTextSpacing          2

//for panning the world plane about whilley-nilley
#define SM_PanSpeedMultiplier       0.001f
#define SM_CursorPanX               300
#define SM_CursorPanY               300
#define SM_MaxFrameTicks            64
#define SM_PanTrack                 0.05f
#define SM_PanEvalThreshold         1.0f
#define SM_PanUnivExtentMult        1.00f
#define SM_NumberBigDots            256

#define SM_SkipFadeoutTime          0.5f

//these flags control ship-type specific sensors and ping things
#define SM_TO                       1           //draw a TO for this ship type
#define SM_Mesh                     2           //draw this ship using a mesh in the SM
#define SM_Exclude                  4           //exclude this ship from battle pings/blobs

#define SM_BlobRadiusMax            5.0f

#define SM_FOWBlobUpdateTime        0.75        //how long the FOW sub-blobs last

/*=============================================================================
    Typedefs:
=============================================================================*/
//for logging objects to render as blurry
typedef struct
{
    sdword x, y;
    color c;
}
smblurry;

typedef struct
{
    sdword x;
    sdword y;
    char text[SM_TickTextChars];
}
smticktext;

/*=============================================================================
    Data:
=============================================================================*/

extern bool smSensorsDisable;
extern bool8 smZoomingIn, smZoomingOut, smFocus;
extern bool smInstantTransition;

//tweakables in the level files
extern real32 smDepthCueRadius;
extern real32 smDepthCueStartRadius;
extern real32 smCircleBorder;
extern real32 smZoomMax;
extern real32 smZoomMin;
extern real32 smInitialDistance;
extern real32 smUniverseSizeX;
extern real32 smUniverseSizeY;
extern real32 smUniverseSizeZ;
extern real32 smClosestDistance;

extern sdword  smFuzzyBlobs;

extern sdword smTacticalOverlay;
extern bool smFocusOnMothershipOnClose;
extern bool smCentreWorldPlane;

extern color smWorldPlaneColor;
extern real32 smMovementWorldPlaneDim;
extern color smCurrentWorldPlaneColor;
extern sdword smTickTextIndex;

extern bool smFleetIntel;

extern ubyte smShipTypeRenderFlags[TOTAL_NUM_SHIPS];

extern udword smSensorWeirdness;

extern sdword MP_HyperSpaceFlag;

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown
void smStartup(void);
void smShutdown(void);
void smUpdateParameters(void);

// used in the tactical overlay to draw the players names
sdword smClickedOnPlayer(rectangle *viewportRect);
void smPlayerNamesDraw(rectangle *viewportRect);

//activate the sensors manager.  It will stop itself
void smSensorsBegin(char *name, featom *atom);

//explicitly close senors manager
void smSensorsClose(char *name, featom *atom);
void smSensorsCloseForGood(void);

//kill an object if you're in the sensors manager
void smObjectDied(void *object);

//initializes the sensors manager weirdness for
//single player missions 7 and 8
void smSensorWeirdnessInit(void);

//misc functions
void *smStrchr(char *string, char character);
void smHorizonLineDraw(void *cam, hmatrix *modelView, hmatrix *projection, real32 distance);
void smTickTextDraw(void);
void smUpdateHyperspaceStatus(bool goForLaunch);

//logic handlers for main region
void smNULL(void);
extern void (*smHoldLeft)(void);
extern void (*smHoldRight)(void);

/*=============================================================================
    Save Game stuff:
=============================================================================*/

void smSave(void);
void smLoad(void);

#endif //___SENSORS_H

