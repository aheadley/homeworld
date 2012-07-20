/*=============================================================================
    Name    : render.h
    Purpose : Initialise rendering system and provide a scene render entry point.

    Created 6/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RENDER_H
#define ___RENDER_H

#include "Types.h"
#include "Camera.h"
#include "Matrix.h"
#include "color.h"
#include "SpaceObj.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define RND_ERROR_CHECKING      1               //basic error checking
#define RND_VERBOSE_LEVEL       1               //control verbose printing
#define RND_FRAME_RATE          1               //display frame rate
#define RND_CAMERA_OFFSET       1               //offset camera by a certain amount to avoid lighting problems
#define RND_BACKGROUND_CHECK    1               //check if we should display background
#define RND_BACKGROUND_STATIC   1               //attach background to camera so it doesn't move at all
#define RND_POLY_STATS          1               //display polygon stats
#define RND_XYZ                 1               //display position of selected ship
#define RND_SCALECAP_TWEAK      0               //allow adjusting (and printing) of the scaling cap crap
#define RND_GL_STATE_DEBUG      1               //dump the GL state to a file
#define RND_GL_STATE_WINDOW     1               //dump the GL state to debug window
#define RND_PLUG_DISABLEABLE    1               //can disable the shameless plug
#define RND_VISUALIZATION       1               //enable special visualization options

#else //HW_Debug

#define RND_ERROR_CHECKING      0               //no error ckecking in retail
#define RND_VERBOSE_LEVEL       0               //don't print any verbose strings in retail
#define RND_FRAME_RATE          0               //display frame rate
#define RND_CAMERA_OFFSET       1               //offset camera by a certain amount to avoid lighting problems
#define RND_BACKGROUND_CHECK    1               //check if we should display background
#define RND_BACKGROUND_STATIC   1               //attach background to camera so it doesn't move at all
#define RND_POLY_STATS          0               //display polygon stats
#define RND_XYZ                 0               //display position of selected ship
#define RND_SCALECAP_TWEAK      0               //allow adjusting (and printing) of the scaling cap crap
#define RND_GL_STATE_DEBUG      0               //dump the GL state to a file
#define RND_GL_STATE_WINDOW     0               //dump the GL state to debug window
#define RND_PLUG_DISABLEABLE    0               //can disable the shameless plug
#define RND_VISUALIZATION       0               //enable special visualization options

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define RND_StackSize           (1024 * 10)

#define RND_StarColor           colRGB(180, 180, 200)
#define RND_Race1BulletColor    colRGB(254, 189, 39)
#define RND_Race2BulletColor    colRGB(4, 251, 66)

#define RND_CameraOffsetX       3.0f

#define RTE_Modulate        0
#define RTE_Replace         1
#define RTE_Decal           2
#define RTE_Blend           3

//scale cap debug stuff
#define RND_CapScaleCapKey          NUMPAD4
#define RND_CapScaleCapKeyDec       NUMPAD2
#define RND_CapScaleCapKeyInc       NUMPAD3
#define rndCapScaleCapSlopeDelta    0.000001f

/*=============================================================================
    Type definitions:
=============================================================================*/

//structure for info passed to the rndInit() functions
typedef struct
{
    sdword width;
    sdword height;
    void   *hWnd;    // don't want to include windows.h just for this!
}
rndinitdata;

//user-defined render callback functions
typedef void (*rendercallback)(void);
typedef void (*renderfunction)(Camera *camera);

/*=============================================================================
    Data:
=============================================================================*/
extern rendercallback rndPreObjectCallback, rndPostObjectCallback;
extern udword rndFrameCount;
extern real32 rndAspectRatio;                                      //aspect ratio of rendering context
extern bool8  rndFogOn;
extern hmatrix rndCameraMatrix;
extern hmatrix rndProjectionMatrix;
extern sdword rndNormalization;
extern bool rndTakeScreenshot;
extern sdword rndLightingEnabled;
extern sdword rndScissorEnabled;

#if RND_POLY_STATS
sdword rndDisplayPolyStats;
sdword rndNumberPolys;
sdword rndNumberTextured;
sdword rndNumberSmoothed;
sdword rndNumberDots;
sdword rndNumberLines;
#endif

#if RND_PLUG_DISABLEABLE
extern bool8 rndShamelessPlugEnabled;
#endif

extern renderfunction rndMainViewRender;

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown the rendering module.
sdword rndInit(rndinitdata *initData);
sdword rndSmallInit(rndinitdata* initData, bool GL);
void rndClose(void);

//render a mission sphere using a specific camera.  Or don't render; it's your call.
void rndMainViewRenderFunction(Camera *camera);             //normal rendering mode
void rndMainViewRenderNothingFunction(Camera *camera);      //don't do anything; just return
void rndMainViewAllButRenderFunction(Camera *camera);       //compute selection info but don't render
void rndBackgroundRender(real32 radius, Camera *camera, bool bDrawStars);

//main render task
void rndRenderTask(void);

//enable certain renderer options
void rndHintInc(void);
void rndBackFaceCullEnable(sdword bEnable);
sdword rndLightingEnable(sdword bEnable);
sdword rndTextureEnable(sdword bEnable);
void rndBillboardEnable(vector *centre);
void rndBillboardDisable(void);
sdword rndTextureEnvironment(sdword mode);
sdword rndPerspectiveCorrection(sdword bEnable);
sdword rndNormalizeEnable(sdword bEnable);
sdword rndAdditiveBlends(sdword bAdditive);
sdword rndMaterialfv(sdword face, sdword pname, real32* params);
void rndSetClearColor(color c);

real32 rndComputeOverlap(Ship* ship, real32 scalar);

void rndShamelessPlug(void);
void rndEnvironmentMap(vector* camera, vector* A, vector* B, vector* C,
                       vector* U, vector* V, vector* W);
void rndEnvironmentMapConvex(vector* camera, vector* A, vector* B, vector* C,
                       real32 convex, vector* U, vector* V, vector* W);

void rndSetScreenFill(sdword count, color c);

void rndLoadShamelessPlug(bool on);     //load / free shameless plug texture
void rndShamelessPlug(void);

void rndResetGLState(void);

void rndClearToBlack(void);
void rndAllClearToBlack(void);
void rndClear(void);
void rndFlush(void);

//render utility functions
void rndRenderAHomeworld(void* camera, void *world);
bool rndShipVisible(SpaceObj* spaceobj, Camera* camera);
bool rndShipVisibleUsingCoordSys(SpaceObj* spaceobj, Camera* camera);
void rndDrawScissorBars(bool scissorEnabled);

#if RND_GL_STATE_DEBUG
extern bool rndGLStateSaving;
void rndGLStateLogFunction(char *location);
#define rndGLStateLog(s) if (rndGLStateSaving) rndGLStateLogFunction(s);
#else
#define rndGLStateLog(s)
#endif
#endif //___RENDER_H
