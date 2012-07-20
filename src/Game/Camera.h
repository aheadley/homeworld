/*=============================================================================
    Name    : Camera.h
    Purpose : Definitions for Camera.c

    Created 6/20/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___CAMERA_H
#define ___CAMERA_H

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define CAM_ERROR_CHECKING      1               //general error checking
#define CAM_VERBOSE_LEVEL       2               //print extra info
#define CAM_CHECKSUM            1               //camera checksum function

#else //HW_Debug

#define CAM_ERROR_CHECKING      0               //general error checking
#define CAM_VERBOSE_LEVEL       0               //print extra info
#define CAM_CHECKSUM            0               //camera checksum function

#endif //HW_Debug


/*=============================================================================
    Types:
=============================================================================*/

#define CAM_USER_ZOOMED 0x02
#define CAM_USER_MOVED 0x01

typedef struct Camera
{
    struct Player *playerowner;
    real32 angle;
    real32 declination;
    real32 distance;
    vector eyeposition;
    vector lookatpoint;
    vector oldlookatpoint;
    vector upvector;
    real32 fieldofview;         // in radians
    real32 clipPlaneNear;
    real32 clipPlaneFar;
    real32 closestZoom;
    real32 farthestZoom;
    bool8 ignoreZoom;
} Camera;

/*=============================================================================
    Functions:
=============================================================================*/

void cameraInit(Camera *camera,real32 distance);
void cameraRotAngle(Camera *camera,real32 angle);
void cameraRotDeclination(Camera *camera,real32 declination);
void cameraZoom(Camera *camera,real32 zoomfactor, bool EnforceShipDistances);
void cameraSetEyePosition(Camera *camera);
sdword cameraControl(Camera *camera,bool EnforceShipDistances);
void cameraCopyPositionInfo(Camera *dst,Camera *src);
void cameraChangeLookatpoint(Camera *camera,vector *newlookatpoint);
void cameraRotateAbout(Camera *camera,vector about,real32 deg);
void cameraSensitivitySet(sdword sens100);
void cameraRayCast(vector *dest, Camera *cam, sdword screenX, sdword screenY, sdword screenWidth, sdword screenHeight);
#if CAM_CHECKSUM
udword cameraChecksum(Camera *cam);
#endif

struct Ship;

void cameraSetEyePositionBasedOnShip(Camera *camera,struct Ship *ship);

/*=============================================================================
    Public tweakable constants:
=============================================================================*/

extern real32 CAMERA_ZOOM_IN;
extern real32 CAMERA_ZOOM_OUT;
extern real32 CAMERA_KEY_ZOOM_IN;
extern real32 CAMERA_KEY_ZOOM_OUT;
extern real32 CAMERA_AUTO_ZOOM_IN;
extern real32 CAMERA_AUTO_ZOOM_OUT;
extern real32 CAMERA_CLIP_FAR;
extern real32 CAMERA_CLIP_FAR_PLANET;

extern real32 CAMERA_FLOAT_SCALAR0;
extern real32 CAMERA_FLOAT_SCALAR1;
extern real32 CAMERA_FLOAT_DIST_SCALAR;
extern real32 CAMERA_FLOAT_OFFSET_SCALAR0;
extern real32 CAMERA_FLOAT_OFFSET_SCALAR1;
extern real32 CAMERA_FLOAT_OFFSET_SCALAR2;
extern real32 CAMERA_MOUSE_SENS_MIN;
extern real32 CAMERA_MOUSE_SENS;
extern real32 CAMERA_MOUSE_SENS_MAX;

extern real32 CAMERA_MAX_ZOOMOUT_DISTANCE;
extern real32 CAMERA_MIN_ZOOMOUT_DISTANCE;

/*=============================================================================
    Public Variables:
=============================================================================*/

extern sdword camMouseX;
extern sdword camMouseY;
extern bool8  wheel_up;
extern bool8  wheel_down;
extern bool8  useSlowWheelZoomIn;
extern bool   zoomOutNow;
extern bool   zoomInNow;


#endif //___CAMERA_H

