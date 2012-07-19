/*=============================================================================
    Name    : CameraCommand.h
    Purpose : Definitions for CameraCommand.c
              Handles the "camera stack"

    Created 7/3/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___CAMERA_COMMAND_H
#define ___CAMERA_COMMAND_H

#include "types.h"
#include "linkedlist.h"
#include "spaceobj.h"
#include "camera.h"
#include "shipselect.h"
#include "statscript.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef SelectCommand FocusCommand;

// each camera stack entry contains information about the camera state,
// and what ships it should focus on.
typedef struct
{
    Node stacklink;
    Camera remembercam;
    FocusCommand focus;
} CameraStackEntry;

typedef enum
{
    CAMERA_SET,
    CAMERA_FOCUSORIENT,
    CAMERA_FOCUSMOVE,
    CAMERA_FOCUSPOP
} TransitionType;

typedef enum
{
    CAMACTION_FOCUS,
    CAMACTION_FOCUSFAR,
    CAMACTION_CANCELFOCUS,
    CAMACTION_FORWARDFOCUS,
    CAMACTION_VIEWMISSPHERE
} ActionType;

#define CCMODE_LOCK_OUT_USER_INPUT              1
#define CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY   2
#define CCMODE_FOCUS_CHANGES_NOTHING            4

#define CCMODE_PILOT_SHIP                       8

// The CameraCommand object contains a camera stack, for focusing on ships
// and canceling focus, a toggle to ViewEntireMissphere, and also an
// actual camera which always tries to "chase" the currently active
// camera on the stack.
typedef struct
{
    LinkedList camerastack;
    CameraStackEntry *currentCameraStack;
    bool ViewEntireMissphere;
    Camera actualcamera;
    TransitionType transition;
    ActionType action;
    real32 transTimeStamp;
    sdword UserControlled;
    bool zoomInCloseAsPossible;
    real32 lockCameraTimeStamp;
    bool dontUseVelocityPredInChase;
    udword ccMode;
    CameraStackEntry *dontFocusOnMe;
} CameraCommand;

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofFocusCommand sizeofSelectCommand
#define sizeofCameraStackEntry(n) (sizeof(CameraStackEntry) + ((n-1)*sizeof(ShipPtr)))

// provides the currently active camera stack entry, where x is pointer to CameraCommand
#define currentCameraStackEntry(x) ((x)->currentCameraStack)

// provides the currently active focus, where x is pointer to CameraCommand
#define currentFocus(x) (&(currentCameraStackEntry(x))->focus)

#define ccFocusSize(n)  (sizeof(FocusCommand) + sizeof(ShipPtr) * ((n) - 1))

/*=============================================================================
    Data:
=============================================================================*/
extern real32 CAMERA_DEFAULT_DISTANCE;
extern real32 CAMERA_FAR_DISTANCE;
extern real32 CAMERA_FOCUSFAR_DISTANCE;

/*=============================================================================
    Functions:
=============================================================================*/

// Camera Command Layer

udword NewSetFocusPoint(CameraStackEntry *curentry, real32 *targetDistance);

void ccInit(CameraCommand *cameracommand);
void ccClose(CameraCommand *cameracommand);
void ccReset(CameraCommand *cameracommand);
void ccFocus(CameraCommand *cameracommand,FocusCommand *focuscom);
void ccFocusClose(CameraCommand *cameracommand, FocusCommand *focuscom);
void ccFocusFar(CameraCommand *cameracommand,FocusCommand *focuscom, Camera *currentCamera);
void ccFocusOnMyMothership(CameraCommand *cameracommand);
void ccFocusOnPlayersMothership(CameraCommand *cameracommand,uword playerindex);
bool ccFocusOnFleet(CameraCommand *cameracommand);
bool ccFocusExact(CameraCommand *cameracommand,CameraCommand *focustocopy);
void ccFocusOnFleetSmooth(CameraCommand *cameracommand);
void ccFocusGeneral(CameraCommand *cameracommand,FocusCommand *focuscom, bool bCloseUp);
void ccForwardFocus(CameraCommand *cameracommand);
void ccCancelFocus(CameraCommand *cameracommand);
void ccViewToggleMissionSphere(CameraCommand *cameracommand);
void ccControl(CameraCommand *cameracommand);
void ccLockOnTargetNow(CameraCommand *cameracommand);

void ccSetModeFlag(CameraCommand *cameracommand,udword ccModeFlag);     // use CCMODE_whatever flags
void ccClearModeFlag(CameraCommand *cameracommand,udword ccModeFlag);   // use CCMODE_whatever flags

void ccChangeAngleDeclination(CameraCommand *cameracommand,real32 angle,real32 declination);



//void ccFocusSideways(CameraCommand *cameracommand,FocusCommand *focuscom);


void ccFreezeLookAtPoint(CameraCommand *cameracommand,SelectCommand *selectcom);

void ccCopyCamera(CameraCommand *cameracommand,Camera *cameraToCopy);

// call this routine to update camera command layer to remove ship
void ccRemoveShip(CameraCommand *cameracommand,Ship *ship);

// call this routine to remove references to theseships from cameracommand
void ccRemoveTheseShips(CameraCommand *cameracommand,SelectCommand *theseships);
void GetDistanceAngleDeclination(Camera *camera,vector *distvec);

//cubic curve evaluation:
void EvalCubic( float *y0, float *m0, float y1, float t );
void ccGetShipCollCenter(Ship *ship, vector *pos, real32 *rad);

//cull a focus selection to a certain radius
sdword ccFocusCullRadiusMean(FocusCommand *selection, real32 radiusSqr, vector *centre);
sdword ccFocusCullRadiusGeneral(FocusCommand *out, FocusCommand *in, real32 radiusSqr, vector *centre);

#endif //___CAMERA_COMMAND_H

