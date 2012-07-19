/*=============================================================================
    Name    : render.c
    Purpose : Render module initialization and render a specific scene.

    Created 6/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef gshaw
//#define DEBUG_COLLISIONS
#endif

#define RND_DOCKLIGHT_TWEAK 0
#define VISIBLE_POLYS 0

#ifdef khentschel
#ifndef HW_Release
#define VERBOSE_SHIP_STATS  1
#else
#define VERBOSE_SHIP_STATS  0
#endif
#define DISPLAY_LOD_SCALE   1
#else
#define VERBOSE_SHIP_STATS  0
#define DISPLAY_LOD_SCALE   1
#endif

#ifdef ddunlop
#ifndef HW_Release
#define FONT_CHECKSPECIAL   0       // special define for testing extended characters
#else
#define FONT_CHECKSPECIAL   0       // turn off this function
#endif
#endif

#define SHOW_TRAIL_STATS    0
#define RND_WILL_PANIC      0
#define USE_RND_HINT        0
#define WILL_TWO_PASS       0

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "glinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "debug.h"
#include "switches.h"
#include "netcheck.h"
#include "spaceobj.h"
#include "mouse.h"
#include "prim2d.h"
#include "region.h"
#include "task.h"
#include "key.h"
#include "universe.h"
#include "nis.h"
#include "camera.h"
#include "cameracommand.h"
#include "prim3d.h"
#include "font.h"
#include "select.h"
#include "texreg.h"
#include "utility.h"
#include "globals.h"
#include "light.h"
#include "shader.h"
#include "tactical.h"
#include "mesh.h"
#include "render.h"
#include "collision.h"
#include "univupdate.h"
#include "soundevent.h"
#include "gun.h"
#include "dock.h"
#include "navlights.h"
#include "main.h"
#include "tweak.h"
#include "defensefighter.h"
#include "clouds.h"
#include "nebulae.h"
#include "fastmath.h"
#include "btg.h"
#include "demo.h"
#include "file.h"
#include "clipper.h"
#include "glcaps.h"
#include "teams.h"
#include "singleplayer.h"
#include "hs.h"
#include "screenshot.h"
#include "salcapcorvette.h"
#include "autolod.h"
#include "proximitysensor.h"
#include "objectives.h"
#include "subtitle.h"
#include "tutor.h"
#include "animatic.h"
#include "bink.h"
#include "StringsOnly.h"
#include "mainrgn.h"
#include "aiship.h"
#include "commandnetwork.h"
#include "ProfileTimers.h"
#include "LagPrint.h"
#include "glcompat.h"
#include "tracking.h"
#include "launchmgr.h"
#include "devstats.h"

bool8 rndFogOn = FALSE;

#if RND_VISUALIZATION
extern BOOL dockLines;
extern BOOL gunLines;
extern bool8 RENDER_BOXES;
extern bool8 RENDER_LIGHTLINES;
#endif

extern bool8 g_ReplaceHack;
extern bool8 g_WireframeHack;
extern bool8 g_HiddenRemoval;

extern color HorseRaceDropoutColor;

vector g_RndPosition;

bool rndTakeScreenshot = FALSE;

/*=============================================================================
    Private Types:
=============================================================================*/

/*=============================================================================
    Data:
=============================================================================*/
//this function pointer is what to call to render the main view
renderfunction rndMainViewRender = rndMainViewRenderFunction;

static sdword rndHint = 0;

HGLRC hGLRenderContext;
HDC hGLDeviceContext;
HWND hGLWindow;

udword meshRenders;

real32 rndAspectRatio;                                      //aspect ratio of rendering context

static sdword rndFillCounter = 0;
static color rndFillColour = 0;

//callback functions optionally called during rendering a mission sphere
rendercallback rndPreObjectCallback = NULL, rndPostObjectCallback = NULL;

sdword rndPerspectiveCorrect = FALSE;
sdword rndTextureEnabled = FALSE;
sdword rndTextureEnviron = RTE_Modulate;
sdword rndNormalization = FALSE;
sdword rndAdditiveBlending = FALSE;
sdword rndLightingEnabled = TRUE;
sdword rndScissorEnabled = FALSE;

//data for billboard geometry
hmatrix rndCameraMatrix;
hmatrix rndProjectionMatrix;
hvector rndBillboardCentre;
#if RND_ERROR_CHECKING
sdword rndBillboardEnabled = FALSE;
#endif


typedef struct c4ub_v3f_s
{
    ubyte  c[4];
    real32 v[3];
} c4ub_v3f;
c4ub_v3f* stararray = NULL;
c4ub_v3f* bigstararray = NULL;


//asteroid0 stuff
typedef struct asteroid0data
{
    color c;
    vector* position;
} asteroid0data;

#define ASTEROID0DATA_SIZE 80
asteroid0data asteroid0Data[ASTEROID0DATA_SIZE];


//frame counter
udword rndFrameCount;

//frame rate data and definitions
#if RND_FRAME_RATE
#define RND_FrameRatePeriod         2.0f
#define RND_FrameRateSeconds        2.0f
#define RND_FrameRateMaxRate        10
#define RND_FrameRateStack          1024
#define RND_FrameRateKey            NUMPAD7
udword rndFrameRateStart;
udword rndFrameRate;
udword rndFrameCountMax, rndFrameCountMin;
udword rndPrintCount;
sdword rndDisplayFrameRate = 0;
taskhandle rndFrameRateTaskHandle;
extern regionhandle ghMainRegion;
#endif //RND_FRAME_RATE
#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
sdword rndDisplayCollStats = 0;
#define RND_CollStatsKey            NUMPADSLASH
#endif

//polygon statistics measurments
#if RND_POLY_STATS
#define RND_PolyStatsPeriod         2.0f
#define RND_PolyStatsSeconds        2.0f
#define RND_PolyStatsStack          1024
#define RND_PolyStatsKey            NUMPAD8
sdword rndDisplayPolyStats = 0;
sdword rndNumberPolys;
sdword rndNumberTextured;
sdword rndNumberSmoothed;
sdword rndNumberDots;
sdword rndNumberLines;
taskhandle rndPolyStatsTaskHandle;
char rndPolyStatsString[256];
color rndPolyStatsColor = colReddish;
sdword rndPolyStatFrameCounter = 0;
#endif //RND_POLY_STATS

#if RND_XYZ
#define RND_XYZKey                  NUMPAD9
bool rndXYZPrint = FALSE;
#endif

//scaling minimum cap crap
static sdword RND_CAPSCALECAP_STATS = 0;
char rndCapScaleCapStatsString[256];
#if RND_SCALECAP_TWEAK
#define scaleCapSlopeDelta  0.000001f                       //adjust the scaling cap
static char scaleCapString[256] = "";
#endif //RND_SCALECAP_TWEAK

#define RND_FOV_TWEAK 0

real32 btgFieldOfView = 50.0f;

#if RND_FOV_TWEAK
#define btgFieldOfViewDelta 1.0f
static char fovString[256] = "";
#endif

static sdword shipsAtLOD[5];
static sdword polysAtLOD[5];

//data for debugging the GL state by dumping it's contents
#if RND_GL_STATE_DEBUG
typedef struct
{
    char *name;
    GLenum enumeration;
    bool bDefault;
}
enumentry;
#define enumEntry(string, n)            {string, n, FALSE}
#define enumDefaultEntry(string, n)     {string, n, TRUE}
#define enumEnd                         {NULL, 0, FALSE}
#define enumError                       {"ERROR", 0xffffff8, TRUE}
enumentry rndBoolEnums[] =
{
    enumEntry("GL_TRUE", GL_TRUE),
    enumDefaultEntry("GL_FALSE", GL_FALSE),
    enumEnd
};
enumentry rndAlphaTestFuncEnum[] =
{
    enumEntry("GL_NEVER", GL_NEVER),
    enumEntry("GL_LESS", GL_LESS),
    enumEntry("GL_EQUAL", GL_EQUAL),
    enumEntry("GL_LEQUAL", GL_LEQUAL),
    enumEntry("GL_GREATER", GL_GREATER),
    enumEntry("GL_NOTEQUAL", GL_NOTEQUAL),
    enumEntry("GL_GEQUAL", GL_GEQUAL),
    enumEntry("GL_ALWAYS", GL_ALWAYS),
    enumError,
    enumEnd
};
enumentry rndBlendFuncEnums[] =
{
    enumEntry("GL_ZERO", GL_ZERO),
    enumEntry("GL_ONE", GL_ONE),
    enumEntry("GL_DST_COLOR", GL_DST_COLOR),
    enumEntry("GL_ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR),
    enumEntry("GL_SRC_ALPHA", GL_SRC_ALPHA),
    enumEntry("GL_ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA),
    enumEntry("GL_DST_ALPHA", GL_DST_ALPHA),
    enumEntry("GL_ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA),
    enumEntry("GL_SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE),
    enumError,
    enumEnd
};
enumentry rndFaceEnums[] =
{
    enumEntry("GL_FRONT", GL_FRONT),
    enumEntry("GL_BACK", GL_BACK),
    enumEntry("GL_FRONT_AND_BACK", GL_FRONT_AND_BACK),
    enumError,
    enumEnd
};
enumentry rndMatrixEnums[] =
{
    enumEntry("GL_MODELVIEW", GL_MODELVIEW),
    enumEntry("GL_PROJECTION", GL_PROJECTION),
    enumEntry("GL_TEXTURE", GL_TEXTURE),
    enumError,
    enumEnd
};
enumentry rndHintEnums[] =
{
    enumEntry("GL_FASTEST", GL_FASTEST),
    enumEntry("GL_NICEST", GL_NICEST),
    enumEntry("GL_DONT_CARE", GL_DONT_CARE),
    enumError,
    enumEnd
};
enumentry rndShadeModelEnums[] =
{
    enumEntry("GL_FLAT", GL_FLAT),
    enumEntry("GL_SMOOTH", GL_SMOOTH),
    enumError,
    enumEnd
};

enumentry rndTextureEnvEnums[] =
{
    enumEntry("GL_MODULATE", GL_MODULATE),
    enumEntry("GL_DECAL", GL_DECAL),
    enumEntry("GL_BLEND", GL_BLEND),
    enumEntry("GL_REPLACE", GL_REPLACE),
    enumError,
    enumEnd
};

typedef struct
{
    char *heading;
    GLenum enumeration;
    sdword type;
    sdword nValues;
    enumentry *enumTable;
}
glstateentry;

#define stateEntry(string, enumb, type, nValues, table)  {string, enumb, type, nValues, table}
#define G_Bool                  0
#define G_GetBool               1
#define G_Integer               2
#define G_Float                 3
#define G_FloatByte             4
#define G_IntFunc               5

//definitions for special-case functions
#define G_TextureEnv            0

glstateentry rndStateSaveTable[] =
{
    stateEntry("GL_ALPHA_TEST", GL_ALPHA_TEST,                                  G_Bool, 1, rndBoolEnums),
    stateEntry("GL_BLEND", GL_BLEND,                                            G_Bool, 1, rndBoolEnums),
    stateEntry("GL_CULL_FACE", GL_CULL_FACE,                                    G_Bool, 1, rndBoolEnums),
    stateEntry("GL_DEPTH_TEST", GL_DEPTH_TEST,                                  G_Bool, 1, rndBoolEnums),
    stateEntry("GL_FOG", GL_FOG,                                                G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHT0", GL_LIGHT0,                                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHT1", GL_LIGHT1,                                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHTING", GL_LIGHTING,                                      G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LINE_SMOOTH", GL_LINE_SMOOTH,                                G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LINE_STIPPLE", GL_LINE_STIPPLE,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_NORMALIZE", GL_NORMALIZE,                                    G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POINT_SMOOTH", GL_POINT_SMOOTH,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POLYGON_SMOOTH", GL_POLYGON_SMOOTH,                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POLYGON_STIPPLE", GL_POLYGON_STIPPLE,                        G_Bool, 1, rndBoolEnums),
    stateEntry("GL_SCISSOR_TEST", GL_SCISSOR_TEST,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_TEXTURE_2D", GL_TEXTURE_2D,                                  G_Bool, 1, rndBoolEnums),

    //stateEntry("GL_TEXTURE_ENV", G_TextureEnv,                                  G_IntFunc,   1, rndTextureEnvEnums),
    //stateEntry("GL_TEXTURE_2D_BINDING", GL_TEXTURE_2D_BINDING,                  G_Integer,   1, NULL),
    stateEntry("GL_ALPHA_TEST_FUNC", GL_ALPHA_TEST_FUNC,                        G_Integer,   1, rndAlphaTestFuncEnum),
    stateEntry("GL_ALPHA_TEST_REF", GL_ALPHA_TEST_REF,                          G_Float,     1, NULL),
    stateEntry("GL_BLEND_DST", GL_BLEND_DST,                                    G_Integer,   1, rndBlendFuncEnums),
    stateEntry("GL_BLEND_SRC", GL_BLEND_SRC,                                    G_Integer,   1, rndBlendFuncEnums),
    stateEntry("GL_BLUE_BIAS", GL_BLUE_BIAS,                                    G_FloatByte, 1, NULL),
    stateEntry("GL_COLOR_CLEAR_VALUE", GL_COLOR_CLEAR_VALUE,                    G_Float,     4, NULL),
    stateEntry("GL_COLOR_MATERIAL_FACE", GL_COLOR_MATERIAL_FACE,                G_Integer,   1, rndFaceEnums),
    stateEntry("GL_CULL_FACE_MODE", GL_CULL_FACE_MODE,                          G_Integer,   1, rndFaceEnums),
    stateEntry("GL_CURRENT_COLOR", GL_CURRENT_COLOR,                            G_FloatByte, 4, NULL),
    stateEntry("GL_CURRENT_RASTER_COLOR", GL_CURRENT_RASTER_COLOR,              G_FloatByte, 4, NULL),
    stateEntry("GL_CURRENT_RASTER_POSITION", GL_CURRENT_RASTER_POSITION,        G_Float,     4, NULL),
    stateEntry("GL_CURRENT_TEXTURE_COORDS", GL_CURRENT_TEXTURE_COORDS,          G_Float,     4, NULL),
    stateEntry("GL_DEPTH_BITS", GL_DEPTH_BITS,                                  G_Integer,   1, NULL),
    stateEntry("GL_DEPTH_WRITEMASK", GL_DEPTH_WRITEMASK,                        G_GetBool,   1, rndBoolEnums),
    stateEntry("GL_FOG_COLOR", GL_FOG_COLOR,                                    G_FloatByte, 4, NULL),
    stateEntry("GL_FOG_DENSITY", GL_FOG_DENSITY,                                G_Float,     1, NULL),
    stateEntry("GL_GREEN_BIAS", GL_GREEN_BIAS,                                  G_FloatByte, 1, NULL),
    stateEntry("GL_LIGHT_MODEL_AMBIENT", GL_LIGHT_MODEL_AMBIENT,                G_FloatByte, 4, NULL),
    stateEntry("GL_LIGHT_MODEL_TWO_SIDE", GL_LIGHT_MODEL_TWO_SIDE,              G_GetBool,   1, rndBoolEnums),
    stateEntry("GL_LINE_WIDTH", GL_LINE_WIDTH,                                  G_Float,     1, NULL),
    stateEntry("GL_LINE_WIDTH_GRANULARITY", GL_LINE_WIDTH_GRANULARITY,          G_Float,     1, NULL),
    stateEntry("GL_MATRIX_MODE", GL_MATRIX_MODE,                                G_Integer,   1, rndMatrixEnums),
    stateEntry("GL_MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE,                      G_Integer,   1, NULL),
    stateEntry("GL_MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS,                    G_Integer,   2, NULL),
    stateEntry("GL_MODELVIEW_MATRIX", GL_MODELVIEW_MATRIX,                      G_Float,     16, NULL),
    stateEntry("GL_PERSPECTIVE_CORRECTION_HINT", GL_PERSPECTIVE_CORRECTION_HINT,G_Integer,   1, rndHintEnums),
    stateEntry("GL_POINT_SIZE", GL_POINT_SIZE,                                  G_Float,     1, NULL),
    stateEntry("GL_POINT_SIZE_GRANULARITY", GL_POINT_SIZE_GRANULARITY,          G_Float,     1, NULL),
    stateEntry("GL_POLYGON_MODE", GL_POLYGON_MODE,                              G_Integer,   1, rndFaceEnums),
    stateEntry("GL_PROJECTION_MATRIX", GL_PROJECTION_MATRIX,                    G_Float,     16, NULL),
    stateEntry("GL_RED_BIAS", GL_RED_BIAS,                                      G_FloatByte, 1, NULL),
    stateEntry("GL_SCISSOR_BOX", GL_SCISSOR_BOX,                                G_Integer,   4, NULL),
    stateEntry("GL_SHADE_MODEL", GL_SHADE_MODEL,                                G_Integer,   1, rndShadeModelEnums),
    stateEntry("GL_SUBPIXEL_BITS", GL_SUBPIXEL_BITS,                            G_Integer,   1, NULL),
    stateEntry("GL_VIEWPORT", GL_VIEWPORT,                                      G_Integer,   1, NULL),
    {NULL, 0, 0, 0, NULL}
};
bool rndGLStateSaving = FALSE;
char rndGLStateLogFileName[128];
sdword rndGLStateLogIndex = 0;
#endif //RND_GL_STATE_DEBUG

#if RND_PLUG_DISABLEABLE
bool8 rndShamelessPlugEnabled = TRUE;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
#if RND_GL_STATE_DEBUG

/*-----------------------------------------------------------------------------
    Name        : rndIntToString
    Description : Convert a GL enumeration to a string
    Inputs      : enumb - enumeration to convert
                  table - table of enumerations/strings
    Outputs     :
    Return      : name of enumeration
----------------------------------------------------------------------------*/
char *rndIntToString(GLenum enumb, enumentry *table)
{
    sdword index;

    for (index = 0; table[index].name != NULL; index++)
    {
        if (enumb == table[index].enumeration || table[index].bDefault)
        {
            return(table[index].name);
        }
    }
    dbgAssert(FALSE);
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : rndGLStateLog
    Description : Log the state of the GL machine.
    Inputs      : location - user-specified name for where this function is called from
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#define MAX_FLOATS          16
#define MAX_INTS            4
#define MAX_BOOLS           1
void rndGLStateLogFunction(char *location)
{
    sdword index, j;
    GLfloat floats[MAX_FLOATS];
    GLint ints[MAX_INTS];
    GLboolean bools[MAX_BOOLS];
    char totalString[256];
    char valueString[128];
    FILE *f;

    f = fopen(rndGLStateLogFileName, "at");
    if (f == NULL)
    {
        dbgMessagef("\nError opening '%s' for GL state logging.", rndGLStateLogFileName);
        return;
    }

#if RND_GL_STATE_WINDOW
    dbgMessagef("\n******************** %s", location);
#endif
    fprintf(f, "******************** %s\n", location);
    for (index = 0; rndStateSaveTable[index].heading != NULL; index++)
    {
        sprintf(totalString, "%32s:", rndStateSaveTable[index].heading);
        switch (rndStateSaveTable[index].type)
        {
            case G_Bool:
                bools[0] = glIsEnabled(rndStateSaveTable[index].enumeration);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(bools[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", bools[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_GetBool:
                glGetBooleanv(rndStateSaveTable[index].enumeration, bools);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(bools[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", bools[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_Integer:
                glGetIntegerv(rndStateSaveTable[index].enumeration, ints);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(ints[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", ints[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_Float:
                glGetFloatv(rndStateSaveTable[index].enumeration, floats);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    sprintf(valueString, "%.2f", floats[j]);
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_FloatByte:
                glGetFloatv(rndStateSaveTable[index].enumeration, floats);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    sprintf(valueString, "%d", colRealToUbyte(floats[j]));
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_IntFunc:
                switch (rndStateSaveTable[index].enumeration)
                {
                    case G_TextureEnv:
//                        glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, ints);
                        break;
                    default:
                        dbgAssert(FALSE);
                }
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(ints[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", ints[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            default:
                dbgAssert(FALSE);
        }
#if RND_GL_STATE_WINDOW
        dbgMessagef("\n%s", totalString);
#endif
        fprintf(f, "%s\n", totalString);
    }
    fclose(f);
}

#endif //RND_GL_STATE_DEBUG
#if RND_FRAME_RATE
/*-----------------------------------------------------------------------------
    Name        : rndFrameRateTaskFunction
    Description : Task function for the frame rate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndFrameRateTaskFunction(void)
{
    rndFrameRateStart = rndFrameRate = rndPrintCount = 0;   //it counters
    rndFrameCountMax = 0;                                   //set min/max counters
    rndFrameCountMin = SDWORD_Max;                          //to rediculous values

    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);
        rndFrameRate = rndFrameCount - rndFrameRateStart;   //number of frames since last printed
        rndFrameRateStart = rndFrameCount;                  //reset start frame
        rndFrameCountMax = max(rndFrameCountMax, rndFrameRate);//update min/max
        rndFrameCountMin = min(rndFrameCountMin, rndFrameRate);

        if (rndDisplayFrameRate)
        {                                                   //display frame rate
            dbgMessagef("\nFrame rate: %.2f <= %.2f <= %.2f",
                       (real32)(rndFrameCountMin) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameCountMax) / RND_FrameRateSeconds);
        }
        rndPrintCount++;
        if (rndPrintCount >= RND_FrameRateMaxRate)
        {                                                   //if time to reset min/max
            rndPrintCount = 0;
            rndFrameCountMax = 0;                           //set min/max counters
            rndFrameCountMin = SDWORD_Max;                  //to rediculous values
        }
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)
//toggle frame rate on/off
sdword rndFrameRateToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayFrameRate ^= TRUE;
    dbgMessagef("\nFrame rate printing %s", rndDisplayFrameRate ? "ON" : "OFF");
    return(0);
}

#endif //RND_FRAME_RATE

#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
udword rndCollStatsToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayCollStats ^= TRUE;
    dbgMessagef("\nCollision stats printing %s", rndDisplayCollStats ? "ON" : "OFF");
    return(0);
}
#endif //COLLISION_CHECK_STATS

#if RND_POLY_STATS
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndPolyStatsTaskFunction(void)
{
    rndNumberPolys    = 0;
    rndNumberTextured = 0;
    rndNumberSmoothed = 0;
    rndNumberDots     = 0;
    rndNumberLines    = 0;

    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);

        if (rndDisplayPolyStats && (rndPolyStatFrameCounter != 0 && rndNumberPolys != 0))
        {                                                   //display frame rate
            sprintf(rndPolyStatsString, "\n%d, (%.2f, %.2f), %d, %d",
                    rndNumberPolys / rndPolyStatFrameCounter,
                    100.0f * rndNumberTextured / rndNumberPolys,
                    100.0f * rndNumberSmoothed / rndNumberPolys, 10, 11);
            //print the poly count of the current LOD
            if (selSelected.numShips == 1)
            {
                static char polyCountString[256];
                static Ship *ship;
                static lod *lodInfo;
                static sdword index, polyCount, vertexCount, lodIndex;
                static meshdata *mesh;

                ship = selSelected.ShipPtr[0];
                if (lodTuningMode)
                {
                    lodIndex = rndLOD;
                }
                else
                {
                    lodIndex = ship->currentLOD;
                }
                if (lodIndex < ship->staticinfo->staticheader.LOD->nLevels)
                {
                    lodInfo = &ship->staticinfo->staticheader.LOD->level[lodIndex];
                    if ((lodInfo->flags & LM_LODType) == LT_Mesh)
                    {                                       //if it's a mesh type lod
                        mesh = (meshdata *)lodInfo->pData;  //get the mesh pointer
                        for (index = vertexCount = polyCount = 0; index < mesh->nPolygonObjects; index++)
                        {                                   //count vertices and polygons
                            polyCount += mesh->object[index].nPolygons;
                            vertexCount += mesh->object[index].nVertices;
                        }
                        sprintf(polyCountString, ", %d, %d, %d", lodIndex, vertexCount, polyCount);
                        strcat(rndPolyStatsString, polyCountString);
                    }
                }
            }
            dbgMessage(rndPolyStatsString);
#if MESH_MATERIAL_STATS
            sprintf(meshMaterialStatsString, "\n%.2f / %.2f / %.2f",
                    (real32)meshMaterialChanges / (real32)rndPolyStatFrameCounter,
                    (real32)meshTotalMaterials / (real32)rndPolyStatFrameCounter,
                    (real32)meshMaterialChanges / (real32)meshTotalMaterials);
            dbgMessage(meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
        }
        rndPrintCount++;
        rndPolyStatsColor = colReddish;
        rndNumberPolys    = 0;
        rndNumberTextured = 0;
        rndNumberSmoothed = 0;
        rndNumberDots     = 0;
        rndNumberLines    = 0;
        rndPolyStatFrameCounter = 0;
#if MESH_MATERIAL_STATS
        meshMaterialChanges = 0;
        meshTotalMaterials = 0;
#endif //MESH_MATERIAL_STATS
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)
//toggle frame rate on/off
sdword rndPolyStatsToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayPolyStats ^= TRUE;
    dbgMessagef("\nFrame rate printing %s", rndDisplayPolyStats ? "ON" : "OFF");
    if (rndDisplayPolyStats)
    {
        strcpy(rndPolyStatsString, "\nnPolys (%textured, %smoothed), nDots, nLines, shipLod, nVertices, nPolys");
        dbgMessage(rndPolyStatsString);
        rndPolyStatsColor = colWhite;
#if MESH_MATERIAL_STATS
        strcpy(meshMaterialStatsString, "\nnMaterialChanges / nMaterials / factor");
        dbgMessage(meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
    }
    return(0);
}

#endif //RND_POLY_STATS

static real32 scalar;
void rndCapScaleCapStatsTaskFunction(void)
{
    if (selSelected.numShips == 1)
    {
        Ship* ship = selSelected.ShipPtr[0];
        ShipStaticInfo* shipstaticinfo = (ShipStaticInfo*)ship->staticinfo;

        if (ship->trail[0] == NULL)
        {
            scalar = 0.0f;
        }
        else
        {
            scalar = ship->trail[0]->scalecap;
            if (scalar == -1.0f)
            {
                scalar = 0.0f;
            }
        }
    }
    else
    {
        scalar = 0.0f;
    }

    sprintf(rndCapScaleCapStatsString, "\n trailScaleCap %f", scalar);
}

bool setupPixelFormat(HDC hDC)
{
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,       /* support double-buffering */
        PFD_TYPE_RGBA,          /* color type */
//        16,             /* prefered color depth */
        MAIN_WindowDepth,
        0, 0, 0, 0, 0, 0,       /* color bits (ignored) */
        0,              /* no alpha buffer */
        0,              /* alpha bits (ignored) */
        0,              /* no accumulation buffer */
        0, 0, 0, 0,         /* accum bits (ignored) */
        16,             /* depth buffer */
        0,              /* no stencil buffer */
        0,              /* no auxiliary buffers */
        PFD_MAIN_PLANE,         /* main layer */
        0,              /* reserved */
        0, 0, 0,            /* no layer, visible, damage masks */
    };

    if (rwglChoosePixelFormat == NULL || glNT)
    {
        pixelFormat = ChoosePixelFormat(hDC, &pfd);
    }
    else
    {
        pixelFormat = rwglChoosePixelFormat(hDC, &pfd);
    }
    if (pixelFormat == 0)
    {
        dbgMessagef("\nChoosePixelFormat failed (%d).", GetLastError());
        return FALSE;
    }

    if (rwglSetPixelFormat == NULL || glNT)
    {
        if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
        {
            dbgMessagef("\nSetPixelFormat failed (%d).", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (rwglSetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
        {
            dbgMessagef("\nSetPixelFormat failed (%d).", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

bool setupPalette(HDC hDC)
{
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;

    if (rwglGetPixelFormat == NULL || glNT)
    {
        pixelFormat = GetPixelFormat(hDC);
    }
    else
    {
        pixelFormat = rwglGetPixelFormat(hDC);
    }

    if (rwglDescribePixelFormat == NULL || glNT)
    {
        DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    }
    else
    {
        rwglDescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        dbgMessage("rndInit: needs paletted display");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

typedef int (*AUXINITPOSITIONPROC)(GLuint, GLuint, GLuint, GLuint, GLuint);

sdword rndSmallInit(rndinitdata* initData, bool GL)
{
    if (GL)
    {
        hGLWindow = initData->hWnd;
        hGLDeviceContext = GetDC(initData->hWnd);
        if (!setupPixelFormat(hGLDeviceContext))
        {
            return FALSE;
        }
        if (!setupPalette(hGLDeviceContext))
        {
            return FALSE;
        }
        hGLRenderContext = (HGLRC)rwglCreateContext((unsigned int)hGLDeviceContext);
        rwglMakeCurrent((int)hGLDeviceContext, (int)hGLRenderContext);
    }
    else
    {
        AUXINITPOSITIONPROC initPositionProc;

        initPositionProc = (AUXINITPOSITIONPROC)rwglGetProcAddress("rauxInitPosition");
        dbgAssert(initPositionProc != NULL);

        hGLWindow = initData->hWnd;
        rglCreateWindow((GLint)hGLWindow, (GLint)MAIN_WindowWidth, (GLint)MAIN_WindowDepth);

        if (!initPositionProc(0, 0, initData->width, initData->height, MAIN_WindowDepth))
        {
            return FALSE;
        }
    }

    glLockArraysEXT = (LOCKARRAYSEXTproc)rwglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (UNLOCKARRAYSEXTproc)rwglGetProcAddress("glUnlockArraysEXT");

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : rndInit
    Description : Initialize the rendering system.
    Inputs      : initData - pointer to a structure containing initialization
                    info.
    Outputs     : ??
    Return      : ERROR if an error occurred.
----------------------------------------------------------------------------*/
static GLfloat lightPosition0[] = {10000.0f, 1.0f, 1.0f, 0.1f};

sdword rndInit(rndinitdata *initData)
{
    static GLfloat  ambientProperties[] = {0.9f, 0.5f, 0.5f, 1.0f};
    static GLfloat  diffuseProperties[] = {0.8f, 0.8f, 0.8f, 1.0f};
    static GLfloat  specularProperties[] = {1.0f, 1.0f, 1.0f, 1.0f};

    if (!RGL)
    {
        hGLWindow = initData->hWnd;
        hGLDeviceContext = GetDC(initData->hWnd);
        if (!setupPixelFormat(hGLDeviceContext))
        {
            dbgMessage("\nrndInit: GL couldn't setupPixelFormat");
            return !OKAY;
        }
        if (!setupPalette(hGLDeviceContext))
        {
            dbgMessage("\nrndInit: GL couldn't setupPalette");
            return !OKAY;
        }
        hGLRenderContext = (HGLRC)rwglCreateContext((unsigned int)hGLDeviceContext);  //greate GL render context and select into window
        rwglMakeCurrent((int)hGLDeviceContext, (int)hGLRenderContext);

//        auxInitPosition(0, 0, initData->width, initData->height);   //create the viewport for rendering
//        auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);       //set mode (direct color, Z-buffered, double buffered)
    }
    else
    {
        AUXINITPOSITIONPROC initPositionProc;

        initPositionProc = (AUXINITPOSITIONPROC)rwglGetProcAddress("rauxInitPosition");
        dbgAssert(initPositionProc != NULL);

        hGLWindow = initData->hWnd;
        rglCreateWindow((GLint)hGLWindow, (GLint)MAIN_WindowWidth, (GLint)MAIN_WindowDepth);

        if (!initPositionProc(0, 0, initData->width, initData->height, MAIN_WindowDepth))
        {
            dbgMessage("\nrndInit: RGL couldn't initPositionProc");
            return !OKAY;
        }
    }

    glCapStartup();
    if (!glCapFeatureExists(GL_SHARED_TEXTURE_PALETTE_EXT))
    {
        trNoPalettes = TRUE;
    }

#if RND_VERBOSE_LEVEL >= 1
    dbgMessagef("\nrndInit: OpenGL Vendor     :%s", glGetString(GL_VENDOR));
    dbgMessagef("\nrndInit: OpenGL Renderer   :%s", glGetString(GL_RENDERER));
    dbgMessagef("\nrndInit: OpenGL Version    :%s", glGetString(GL_VERSION));
    dbgMessagef("\nrndInit: OpenGL Extensions :%s", glGetString(GL_EXTENSIONS));
#endif
    rndSetClearColor(colRGBA(0,0,0,255));
    glClearDepth( 1.0 );

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rndAdditiveBlending = FALSE;

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientProperties);
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularProperties);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0f);
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientProperties);

    glEnable( GL_LIGHT0 );

//    glShadeModel(GL_FLAT);

    glCullFace(GL_BACK);                                    //enable culling of back-facing polys
    glEnable(GL_CULL_FACE);

    rndPreObjectCallback = rndPostObjectCallback = NULL;    //no render callbacks yet

    rndAspectRatio = (GLfloat)initData->width / (GLfloat)initData->height;

    rndFrameCount = 0;

#if RND_FRAME_RATE
    rndFrameRateTaskHandle = taskStart(rndFrameRateTaskFunction, RND_FrameRatePeriod, 0);//start frame rate task
    regKeyChildAlloc(ghMainRegion, RND_FrameRateKey, RPE_KeyDown, rndFrameRateToggle, 1, RND_FrameRateKey);
#endif//RND_FRAME_RATE
#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
    regKeyChildAlloc(ghMainRegion, RND_CollStatsKey, RPE_KeyDown, rndCollStatsToggle, 1, RND_CollStatsKey);
#endif
#if RND_POLY_STATS
    rndPolyStatsTaskHandle = taskStart(rndPolyStatsTaskFunction, RND_PolyStatsPeriod, 0);//start frame rate task
    regKeyChildAlloc(ghMainRegion, RND_PolyStatsKey, RPE_KeyDown, rndPolyStatsToggle, 1, RND_PolyStatsKey);
#endif //RND_POLY_STATS

#if TEST_SPAN_PERFORMANCE
    testHook();
#endif

    glDepthFunc(glCapDepthFunc);

    glLockArraysEXT = (LOCKARRAYSEXTproc)rwglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (UNLOCKARRAYSEXTproc)rwglGetProcAddress("glUnlockArraysEXT");

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : rndClose
    Description : Closes the rendering module.
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void rndClose(void)
{
    if (RGL)
    {
        rglDeleteWindow((GLint)hGLWindow);
    }
    else
    {
        if (hGLRenderContext)
        {
            rwglMakeCurrent(0, 0);                              //release GL render context
            rwglDeleteContext((int)hGLRenderContext);
        }
        ReleaseDC(hGLWindow, hGLDeviceContext);                 //release the Device context
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndBillboardEnable
    Description : Enable billboard mode
    Inputs      : centre - location of the centre or 'pivot' point of the
                    billboard geometry.
    Outputs     : Loads identity for modelview matrix and translates by
                    (cameramatrix * centre) which is stored in rndBillboardCentre.
    Return      : void
    Note        : Billboard geometry should be drawn on the z = 0 plane
                    about x = y = 0.
----------------------------------------------------------------------------*/
void rndBillboardEnable(vector *centre)
{
    hvector hCentre;

#if RND_ERROR_CHECKING
    if (rndBillboardEnabled)
    {
        dbgFatal(DBG_Loc, "Billboarding already enabled");
    }
    rndBillboardEnabled = TRUE;
#endif
    glLoadIdentity();
    hCentre.x = centre->x;
    hCentre.y = centre->y;
    hCentre.z = centre->z;
    hCentre.w = 1.0f;
    hmatMultiplyHMatByHVec(&rndBillboardCentre, &rndCameraMatrix, &hCentre);
    glTranslatef(rndBillboardCentre.x, rndBillboardCentre.y, rndBillboardCentre.z);
}

/*-----------------------------------------------------------------------------
    Name        : rndBillboardDisable
    Description : Disable billboard mode.
    Inputs      : void
    Outputs     : Loads rndCameraMatrix into modelview (should only be 1
                    level on the model-view matrix stack).
    Return      : void
----------------------------------------------------------------------------*/
void rndBillboardDisable(void)
{
#if RND_ERROR_CHECKING
    if (rndBillboardEnabled == FALSE)
    {
        dbgFatal(DBG_Loc, "Billboarding already disabled.");
    }
    rndBillboardEnabled = FALSE;
#endif
    glLoadMatrixf((GLfloat *)(&rndCameraMatrix));    //restore previous camera matrix
}

void rndFilter(bool on)
{
    if (!RGL)
    {
        return;
    }
    if (on)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

static real64 rndNear(real64 in)
{
    return (RGLtype == D3Dtype) ? 0.5 * in : in;
}

/*-----------------------------------------------------------------------------
    Name        : rndBackgroundRender
    Description : Render the background of a mission sphere
    Inputs      : radius - radius of mission sphere
                  galaxy - pointer to galaxyinfo structure describing background
    Outputs     : renders each segment as a quadrangle about the mission sphere
    Return      : void
----------------------------------------------------------------------------*/
void rndBackgroundRender(real32 radius, Camera* camera, bool bDrawStars)
{
    real32 projection[16];
    sdword index;

    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio,     //set projection matrix
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);
    glMatrixMode(GL_MODELVIEW);

    if (showBackgrounds && gameIsRunning)
    {
        btgRender();
    }

    //draw the stars in the mission sphere
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, 0);
    rndTextureEnable(FALSE);
    if (bDrawStars && gameIsRunning)
    {
        bool blends, pointSize;

        blends = glCapFeatureExists(GL_POINT_SMOOTH);
        pointSize = glCapFeatureExists(GL_POINT_SIZE);

        glPointSize(1.0f);

        if (blends)
        {
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_BLEND);
            //additively blend only if background are enabled
            rndAdditiveBlends(showBackgrounds);
            if (pointSize && (MAIN_WindowWidth > 1024))
            {
                glPointSize(1.2f);
            }
        }

        //need to reset the projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluPerspective(btgFieldOfView, rndAspectRatio,
                        rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

        //draw small stars
        if (glCapFeatureExists(GL_VERTEX_ARRAY))
        {
            glInterleavedArrays(GL_C4UB_V3F, 0, (GLvoid*)stararray);
            glDrawArrays(GL_POINTS, 0, universe.star3dinfo->Num3dStars - NUM_BIG_STARS);
        }
        else
        {
            glBegin(GL_POINTS);
            for (index = 0; index < (universe.star3dinfo->Num3dStars - NUM_BIG_STARS); index++)
            {
                glColor4ub(stararray[index].c[0], stararray[index].c[1],
                           stararray[index].c[2], stararray[index].c[3]);
                glVertex3fv((GLfloat*)&stararray[index].v);
            }
            glEnd();
        }

        if (blends && pointSize)
        {
            switch (MAIN_WindowWidth)
            {
            case 640:
            case 800:
                glPointSize(2.1f);
                break;
            case 1024:
                glPointSize(2.5f);
                break;
            case 1280:
                glPointSize(2.65f);
                break;
            default:
                glPointSize(2.8f);
            }
        }

        //draw big stars
        if (glCapFeatureExists(GL_VERTEX_ARRAY))
        {
            glInterleavedArrays(GL_C4UB_V3F, 0, (GLvoid*)bigstararray);
            glDrawArrays(GL_POINTS, 0, NUM_BIG_STARS);
        }
        else
        {
            glBegin(GL_POINTS);
            for (index = 0; index < NUM_BIG_STARS; index++)
            {
                glColor4ub(bigstararray[index].c[0], bigstararray[index].c[1],
                           bigstararray[index].c[2], bigstararray[index].c[3]);
                glVertex3fv((GLfloat*)&bigstararray[index].v);
            }
            glEnd();
        }

        if (blends)
        {
            glDisable(GL_BLEND);
            glDisable(GL_POINT_SMOOTH);
            if (pointSize)
            {
                glPointSize(1.0f);
            }
        }
    }

    rndAdditiveBlends(FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    glMatrixMode(GL_MODELVIEW);
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraInside
    Description : decides whether the camera is inside a ship or not
    Inputs      : spaceobj - the object to test
                  camera - the current camera object
    Outputs     :
    Return      : TRUE if camera is inside, FALSE otherwise
----------------------------------------------------------------------------*/
bool rndCameraInside(SpaceObj* spaceobj, Camera* camera)
{
    if ((spaceobj->flags & SOF_Rotatable) && (spaceobj->flags & SOF_Impactable))
    {
        vector dist;
        real32 collspheresize;

        collspheresize = spaceobj->staticinfo->staticheader.staticCollInfo.collspheresize;
        collspheresize *= collspheresize;

        //sphere test
        vecSub(dist, spaceobj->posinfo.position, camera->eyeposition);
        if (vecMagnitudeSquared(dist) > collspheresize)
        {
            //further away than the bounding sphere, trivially reject
            return FALSE;
        }

        //rect test (final)
        return collCheckRectPoint((SpaceObjRotImp*)spaceobj, &camera->eyeposition);
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndInsideShip
    Description : determines whether the camera is inside a ship
    Inputs      : spaceobj - the object to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if camera inside, else FALSE
----------------------------------------------------------------------------*/
bool rndInsideShip(SpaceObj* spaceobj, Camera* camera)
{
    if (nisIsRunning)
    {
        return FALSE;
    }
    if (bitTest(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP))
    {
        return FALSE;
    }
    return rndCameraInside(spaceobj, camera);
}

/*-----------------------------------------------------------------------------
    Name        : rndShipVisible
    Description : determines if a SpaceObj is within the viewing frustum via
                  a few different means
    Inputs      : spaceobj - the spaceobject to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if ship is visible, else FALSE
----------------------------------------------------------------------------*/
bool rndShipVisible(SpaceObj* spaceobj, Camera* camera)
{
    vector veye, vobj;

    vecSub(veye, camera->eyeposition, camera->lookatpoint);
    vecSub(vobj, camera->eyeposition, spaceobj->posinfo.position);

    if ((spaceobj->flags & SOF_Rotatable) && (spaceobj->flags & SOF_Impactable))
    {
        //bbox frustum culling
        StaticCollInfo* sinfo = &((SpaceObjRotImp*)spaceobj)->staticinfo->staticheader.staticCollInfo;

#if 0 //GLcompat
        return !rglIsClipped((GLfloat*)&sinfo->collrectoffset,
                             sinfo->uplength, sinfo->rightlength,
                             sinfo->forwardlength);
#else
        return !clipBBoxIsClipped((real32*)&sinfo->collrectoffset,
                                  sinfo->uplength, sinfo->rightlength,
                                  sinfo->forwardlength);
#endif
    }
    else
    {
        //sphere test

        if (spaceobj->objtype == OBJ_ShipType)
        {
            //account for ship's radius
            real32 radius = ((Ship*)spaceobj)->staticinfo->staticheader.staticCollInfo.approxcollspheresize;
            return (vecDotProduct(veye, vobj) > -radius);
        }
        else
        {
            return (vecDotProduct(veye, vobj) > 0.0f);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndShipVisibleUsingCoordSys
    Description : calls rndShipVisible after setting up appropriate matrices in the GL
    Inputs      : spaceobj - the spaceobject to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if visible, FALSE otherwise
----------------------------------------------------------------------------*/
bool rndShipVisibleUsingCoordSys(SpaceObj* spaceobj, Camera* camera)
{
    GLfloat projection[16], modelview[16];
    hmatrix coordMatrixForGL;
    bool    result;

    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX,  modelview);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(camera->fieldofview, rndAspectRatio,
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    rgluLookAt(camera->eyeposition.x, camera->eyeposition.y, camera->eyeposition.z,
               camera->lookatpoint.x, camera->lookatpoint.y, camera->lookatpoint.z,
               camera->upvector.x, camera->upvector.y, camera->upvector.z);

    hmatMakeHMatFromMat(&coordMatrixForGL, &((SpaceObjRot*)spaceobj)->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position, coordMatrixForGL);
    glMultMatrixf((GLfloat*)&coordMatrixForGL);

    result = rndShipVisible(spaceobj, camera);

    glMatrixMode(GL_PROJECTION_MATRIX);
    glLoadMatrixf(projection);

    glMatrixMode(GL_MODELVIEW_MATRIX);
    glLoadMatrixf(modelview);

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : rndFade
    Description : configures the rendering pipeline for fading ships out of existence
                  at the edge of the renderlist
    Inputs      : spaceobj - the object
                  camera - the camera
    Outputs     :
    Return      : TRUE if the ship is not visible (totally faded), FALSE otherwise
----------------------------------------------------------------------------*/
bool rndFade(SpaceObj* spaceobj, Camera* camera)
{
    vector distvec;
    real32 distsqr0, distsqr1, distsqr;
    real32 fadedist, maxdist, mindist;

    real32 mult = 1.3f;

    vecSub(distvec, camera->lookatpoint, spaceobj->posinfo.position);
    distsqr0 = vecMagnitudeSquared(distvec);

    vecSub(distvec, camera->eyeposition, spaceobj->posinfo.position);
    distsqr1 = vecMagnitudeSquared(distvec);

    distsqr = (distsqr0 < distsqr1) ? distsqr0 : distsqr1;

    if (spaceobj->objtype == OBJ_ShipType)
    {
        switch (((ShipStaticInfo*)spaceobj->staticinfo)->shipclass)
        {
        case CLASS_Mothership:
            fadedist = RENDER_FADE_MOTHERSHIP;
            maxdist  = RENDER_LIMIT_MOTHERSHIP;
            break;
        case CLASS_HeavyCruiser:
            fadedist = RENDER_FADE_HEAVYCRUISER;
            maxdist  = RENDER_LIMIT_HEAVYCRUISER;
            break;
        case CLASS_Carrier:
            fadedist = RENDER_FADE_CARRIER;
            maxdist  = RENDER_LIMIT_CARRIER;
            break;
        case CLASS_Destroyer:
            fadedist = RENDER_FADE_DESTROYER;
            maxdist  = RENDER_LIMIT_DESTROYER;
            break;
        case CLASS_Frigate:
            fadedist = RENDER_FADE_FRIGATE;
            maxdist  = RENDER_LIMIT_FRIGATE;
            break;
        case CLASS_Corvette:
            fadedist = RENDER_FADE_CORVETTE;
            maxdist  = RENDER_LIMIT_CORVETTE;
            break;
        case CLASS_Fighter:
            fadedist = RENDER_FADE_FIGHTER;
            maxdist  = RENDER_LIMIT_FIGHTER;
            break;
        case CLASS_Resource:
            fadedist = RENDER_FADE_RESOURCE;
            maxdist  = RENDER_LIMIT_RESOURCE;
            break;
        case CLASS_NonCombat:
            fadedist = RENDER_FADE_NONCOMBAT;
            maxdist  = RENDER_LIMIT_NONCOMBAT;
            break;
        default:
            fadedist = RENDER_FADE_FRIGATE;     //happy medium
            maxdist  = RENDER_LIMIT_FRIGATE;
        }

        if (((ShipStaticInfo*)spaceobj->staticinfo)->renderlistFade != 0.0f)
        {
            fadedist = ((ShipStaticInfo*)spaceobj->staticinfo)->renderlistFade;
            maxdist  = ((ShipStaticInfo*)spaceobj->staticinfo)->renderlistLimitSqr;
        }

        fadedist *= mult;
        maxdist = mult * fsqrt(maxdist);
    }
    else if (spaceobj->objtype == OBJ_DerelictType)
    {
        if (((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistFade != 0.0f)
        {
            fadedist = ((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistFade;
            maxdist  = ((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistLimitSqr;
        }
        else if (spaceobj->flags & SOF_BigObject)
        {
            fadedist = RENDER_MAXVIEWABLE_FADE;
            maxdist  = RENDER_MAXVIEWABLE_DISTANCE_SQR;
        }
        else
        {
            fadedist = RENDER_VIEWABLE_FADE;
            maxdist  = RENDER_VIEWABLE_DISTANCE_SQR;
        }

        fadedist *= mult;
        maxdist = mult * fsqrt(maxdist);
    }
    else
    {
        if (spaceobj->flags & SOF_BigObject)
        {
            fadedist = mult * RENDER_MAXVIEWABLE_FADE;
            maxdist  = mult * fsqrt(RENDER_MAXVIEWABLE_DISTANCE_SQR);
        }
        else
        {
            fadedist = mult * RENDER_VIEWABLE_FADE;
            maxdist  = mult * fsqrt(RENDER_VIEWABLE_DISTANCE_SQR);
        }
    }

    mindist = maxdist - fadedist;
    fadedist *= fadedist;
    mindist *= mindist;

    if (distsqr < mindist || g_ReplaceHack)
    {
        if (RGL && !usingShader)
            rglLightingAdjust(0.0f);
        else
            meshSetFade(0.0f);
    }
    else if (distsqr < mindist+fadedist)
    {
        real32 amount = (distsqr - mindist) / fadedist;
        if (RGL && !usingShader)
            rglLightingAdjust(amount);
        else
            meshSetFade(amount);
        rndAdditiveBlends(FALSE);
    }
    else
    {
        if (RGL && !usingShader)
            rglLightingAdjust(1.0f);
        else
            meshSetFade(1.0f);
        return TRUE;
    }

    return FALSE;
}

void rndUnFade()
{
    if (RGL && !usingShader)
        rglLightingAdjust(0.0f);
    else
        meshSetFade(0.0f);
}

/*-----------------------------------------------------------------------------
    Name        : rndRenderAHomeworld
    Description : Special 'world-render' code for drawing planets
    Inputs      : camera - what camera we're rendering from
                  world - what planet we're drawing
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define camera ((Camera *)voidCamera)
#define world  ((Derelict *)voidWorld)
void rndRenderAHomeworld(void* voidCamera, void *voidWorld)
{
    real32 planetScale;
    vector *position;
    meshdata *worldMesh;
    hmatrix coordMatrixForGL;
    sdword colorScheme = world->colorScheme;

    planetScale = world->staticinfo->scaleFactor;
    position = &world->posinfo.position;
    worldMesh = (meshdata *)world->staticinfo->staticheader.LOD->level[0].pData;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio, rndNear(camera->clipPlaneNear), CAMERA_CLIP_FAR_PLANET);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    hmatMakeHMatFromMat(&coordMatrixForGL,&world->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(*position, coordMatrixForGL);
    glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
    glScalef(planetScale, planetScale, planetScale);

    rndNormalizeEnable(TRUE);
    glDisable(GL_DEPTH_TEST);
    meshObjectRender(&worldMesh->object[0], worldMesh->localMaterial, colorScheme);
    glEnable(GL_DEPTH_TEST);
    rndNormalizeEnable(FALSE);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
}
#undef world
#undef camera

/*-----------------------------------------------------------------------------
    Name        : rndRenderAWorldEffect
    Description : Render an effect in the manner of a homeworld.
    Inputs      : camera - camera we're looking at it from
                  effect - effect we are to render
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndRenderAWorldEffect(Camera *camera, Effect *effect)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio, rndNear(camera->clipPlaneNear), CAMERA_CLIP_FAR_PLANET);

    glMatrixMode(GL_MODELVIEW);

    rndNormalizeEnable(TRUE);
    glDisable(GL_DEPTH_TEST);

    etgEffectDraw(effect);

    glEnable(GL_DEPTH_TEST);
    rndNormalizeEnable(FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
}

static real32 lastTime0 = 0.0f;
static real32 lastTime1 = 0.0f;

void rndResetFloatTime(void)
{
    lastTime0 = taskTimeElapsed;
    lastTime1 = taskTimeElapsed;
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraOffset0
    Description : camera offset fn 0
    Inputs      :
    Outputs     :
    Return      : a value to be added to a camera coordinate
----------------------------------------------------------------------------*/
real32 rndCameraOffset0(void)
{
    real32 et;
    static real32 t = 0.0f;

    t += taskTimeElapsed - lastTime0;
    lastTime0 = taskTimeElapsed;

    et = t * CAMERA_FLOAT_SCALAR0;

    return (real32)(sin((real64)et) - cos(0.5f * (real64)et));
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraOffset1
    Description : camera offset fn 1
    Inputs      :
    Outputs     :
    Return      : a value to be added to a camera coordinate
----------------------------------------------------------------------------*/
real32 rndCameraOffset1(void)
{
    real32 et;
    static real32 t = 0.0f;

    t += taskTimeElapsed - lastTime1;
    lastTime1 = taskTimeElapsed;

    et = t * CAMERA_FLOAT_SCALAR1;

    return (real32)(sin((real64)et) + cos(0.5f * (real64)et));
}

/*-----------------------------------------------------------------------------
    Name        : rndScaleCameraOffsets
    Description : apply scaling to the camera float offsets to fudge a slightly
                  more linear falloff with distance
    Inputs      : camera - the camera
                  offset - offsets, 2 of 'em
    Outputs     : scaledOffset - scaled versions of the offsets
    Return      :
----------------------------------------------------------------------------*/
void rndScaleCameraOffsets(real32 *scaledOffset, Camera* camera, real32* offset)
{
    real32 dist;

    dist = camera->distance * CAMERA_FLOAT_DIST_SCALAR;
    if (dist > 1.0f)
    {
        scaledOffset[0] = offset[0] * dist;
        scaledOffset[1] = offset[1] * dist;
        scaledOffset[2] = offset[2] * dist;
    }
    else
    {
        scaledOffset[0] = offset[0];
        scaledOffset[1] = offset[1];
        scaledOffset[2] = offset[2];
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndDockScalar
    Description : returns a value [0..1] indicating the amount of docking light
                  being received
    Inputs      : ship - ship to light
                  dockship - the ship that 'ship' is docking with
                  nearVal, farVal - range of light
    Outputs     :
    Return      : scalar [0..1] indicating amount of light being received
----------------------------------------------------------------------------*/
real32 rndDockScalar(Ship* ship, Ship* dockship, real32 nearVal, real32 farVal)
{
    vector veclen;
    real32 dist;

    vecSub(veclen, dockship->posinfo.position, ship->posinfo.position);
    dist = fsqrt(vecMagnitudeSquared(veclen));
    if (dist < nearVal) dist = nearVal;
    if (dist > farVal)  dist = farVal;
    dist -= nearVal;
    dist /= (farVal - nearVal);
    return 1.0f - dist;
}

/*-----------------------------------------------------------------------------
    Name        : rndDrawAsteroid0
    Description : renders asteroid0's (small asteroids that provide visible
                  structure to a level)
    Inputs      : n - number of points to render
    Outputs     : displays the asteroid0Data list
    Return      :
----------------------------------------------------------------------------*/
void rndDrawAsteroid0(sdword n)
{
    sdword  index;
    color   c;
    vector* v;

    glPointSize(glCapFeatureExists(GL_POINT_SIZE) ? 2.0f : 1.0f);
    glBegin(GL_POINTS);
    for (index = 0; index < n; index++)
    {
        c = asteroid0Data[index].c;
        v = asteroid0Data[index].position;
        glColor3ub(colRed(c), colGreen(c), colBlue(c));
        glVertex3fv((GLfloat*)v);
    }
    glEnd();
    glPointSize(1.0f);
}

/*-----------------------------------------------------------------------------
    Name        : rndPreRenderDebugStuff
    Description : A bunch of debug stuff from the beginning of the render
                    function.
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndPreRenderDebugStuff(Camera *camera)
{
#if GUN_TUNE_MODE
    if (gunTuningMode && (selSelected.numShips == 1))
    {
        if (keyIsStuck(INSERTKEY))
        {
            tuningGun++;

            keyClearSticky(INSERTKEY);
        }
    }
#endif

#if LOD_PRINT_DISTANCE
    if (keyIsStuck(INSERTKEY))
    {
        if (rndLOD < 10)
        {
            rndLOD++;
            dbgMessagef("\nLevel of detail = %d", rndLOD);
        }
        keyClearSticky(INSERTKEY);
    }
    if (keyIsStuck(HOMEKEY))
    {
        if (rndLOD > 0)
        {
            rndLOD--;
            dbgMessagef("\nLevel of detail = %d", rndLOD);
        }
        keyClearSticky(HOMEKEY);
    }
    if (keyIsStuck(DELETEKEY))
    {
        lodDrawingMode ^= TRUE;
        if (selSelected.numShips == 1)
        {
            vector length;

            length.x = camera->eyeposition.x - selSelected.ShipPtr[0]->posinfo.position.x;
            length.y = camera->eyeposition.y - selSelected.ShipPtr[0]->posinfo.position.y;
            length.z = camera->eyeposition.z - selSelected.ShipPtr[0]->posinfo.position.z;
            dbgMessagef("\nCamera distance = %f", vecMagnitudeSquared(length));
        }
        else
        {
            dbgMessagef("\nPlease select one and only one object for LOD tuning.");
        }
        keyClearSticky(DELETEKEY);
    }
    if (keyIsStuck(ENDKEY))
    {
        lodTuningMode ^= TRUE;
        dbgMessagef("\nLOD tuning %s", lodTuningMode ? "ON" : "OFF");
        keyClearSticky(ENDKEY);
    }
#endif
#if RND_XYZ
    if (keyIsStuck(RND_XYZKey))
    {
        keyClearSticky(RND_XYZKey);
        rndXYZPrint ^= TRUE;
    }
#endif //RND_XYZ
}

/*-----------------------------------------------------------------------------
    Name        : rndPostRender3DStuff
    Description : Post-object rendering of 3D debugging stuff
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndPostRenderDebug3DStuff(Camera *camera)
{
#if NIS_DRAW_CAMERA
    if (nisIsRunning && nisCamera != NULL && nisCamera != camera)
    {                                                       //if we should draw the NIS camera
        vector upVector = nisCamera->eyeposition;
        vector cameraVector;
        static bool flashFlag;

        rndLightingEnable(FALSE);
        vecSub(cameraVector, nisCamera->lookatpoint, nisCamera->eyeposition);
        vecMultiplyByScalar(cameraVector, 200);
        vecAdd(cameraVector, cameraVector, nisCamera->lookatpoint);
        primLine3(&nisCamera->eyeposition, &cameraVector, flashFlag ? colWhite : colFuscia);
        vecScalarMultiply(upVector, nisCamera->upvector, 200);
        vecAdd(upVector, nisCamera->eyeposition, upVector);
//        upVector.z -= abs(nisCamera->eyeposition.z - cameraVector.z) +
//            abs(nisCamera->eyeposition.y - cameraVector.y);
        primLine3(&nisCamera->eyeposition, &upVector, colWhite);
//      if (selSelected.numShips > 0)
//      {
//          primLine3(&nisCamera->eyeposition, &selSelected.ShipPtr[0]->posinfo.position, flashFlag ? colWhite : colFuscia);
//      }
        flashFlag ^= TRUE;
    }
#endif
}


#if FONT_CHECKSPECIAL
fonthandle testing=1;
extern fontregistry frFontRegistry[FR_NumberFonts];
#endif

/*-----------------------------------------------------------------------------
    Name        : rndPostRenderDebug2DStuff
    Description : Post-object rendering of 2D debug stuff
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndPostRenderDebug2DStuff(Camera *camera)
{
#if LOD_PRINT_DISTANCE
    if (lodDrawingMode && selSelected.numShips == 1)
    {
        vector length;
        vector shippos = selSelected.ShipPtr[0]->collInfo.collPosition;
        sdword nCurrentLOD;

        if (!lodTuningMode)
        {
            nCurrentLOD = selSelected.ShipPtr[0]->currentLOD;
        }
        else
        {
            nCurrentLOD = rndLOD;
        }

        vecSub(length,camera->eyeposition,shippos);

        fontPrintf(0, MAIN_WindowHeight - fontHeight(" ") - 1, colWhite,
                   "Current LOD = %d, distance = %f, x = %.1f y = %.1f z = %.1f",
                   nCurrentLOD, vecMagnitudeSquared(length),shippos.x,shippos.y,shippos.z);
    }
#endif
#if GUN_TUNE_MODE
    if (gunTuningMode && (selSelected.numShips == 1))
    {
        gunTuneGun(selSelected.ShipPtr[0]);
    }
#endif

#if RND_SCALECAP_TWEAK
#ifndef khentschel
    fontPrint(0, 0, colWhite, scaleCapString);
#endif
#endif

#if SHOW_TRAIL_STATS
    {
        char buf[256];
        static lastUpdated;
        static lastNotUpdated;

        if (trailsUpdated == 0)
        {
            sprintf(buf, "ren %3d up %3d not %3d",
                    trailsRendered, lastUpdated, lastNotUpdated);
        }
        else
        {
            sprintf(buf, "ren %3d up %3d not %3d",
                    trailsRendered, trailsUpdated, trailsNotUpdated);
            lastUpdated = trailsUpdated;
            lastNotUpdated = trailsNotUpdated;
        }

        fontPrint(0, 32, colWhite, buf);
        trailsUpdated = trailsNotUpdated = 0;

        {
            extern sdword bTrailRender, bTrailDraw;

            sprintf(buf, "render %d draw %d", bTrailRender, bTrailDraw);
            fontPrint(0, 48, colWhite, buf);

            if (keyIsHit(SHIFTKEY))
            {
                if (keyIsHit(NINEKEY)) bTrailRender = 1;
                if (keyIsHit(ZEROKEY)) bTrailDraw = 1;
            }
            else
            {
                if (keyIsHit(NINEKEY)) bTrailRender = 0;
                if (keyIsHit(ZEROKEY)) bTrailDraw = 0;
            }
        }
    }
#endif

#if USE_RND_HINT
    {
        char buf[32];
        switch (rndHint)
        {
        case 0:
            sprintf(buf, "hint [default]");
            break;
        case 1:
            sprintf(buf, "hint [on]");
            break;
        case 2:
            sprintf(buf, "hint [off]");
            break;
        default:
            sprintf(buf, "hint [?]");
        }
        fontPrint(MAIN_WindowWidth - fontWidth(buf), fontHeight(buf), colWhite, buf);
    }
#endif

#if RND_FOV_TWEAK
    if (keyIsHit(NINEKEY))
    {
        btgFieldOfView -= btgFieldOfViewDelta;
        sprintf(fovString, "fov = %f", btgFieldOfView);
    }
    if (keyIsHit(ZEROKEY))
    {
        btgFieldOfView += btgFieldOfViewDelta;
        sprintf(fovString, "fov = %f", btgFieldOfView);
    }
    fontPrint(MAIN_WindowWidth - fontWidth(fovString), 0, colWhite, fovString);
#endif //RND_FOV_TWEAK

#if RND_SCALECAP_TWEAK
    if (keyIsHit(RND_CapScaleCapKey))
    {
        if (keyIsHit(CONTROLKEY))
        {
            RND_CAPSCALECAP_STATS = TRUE;
        }
        else
        {
            RND_CAPSCALECAP_STATS = FALSE;
        }
    }
    if (RND_CAPSCALECAP_STATS)
    {
        rndCapScaleCapStatsTaskFunction();
    }
#endif //RND_SCALECAP_TWEAK
#if RND_DOCKLIGHT_TWEAK
    if (selSelected.numShips == 1)
    {
        Ship* ship = selSelected.ShipPtr[0];
        if (ship->dockvars.reserveddocking != -1 &&
            ship->dockvars.dockship != NULL)
        {
            vector veclen;
            real32 dist;
            Ship* dockship;
            char string[64];

            dockship = ship->dockvars.dockship;
            vecSub(veclen, ship->posinfo.position, dockship->posinfo.position);
            dist = fsqrt(vecMagnitudeSquared(veclen));
            sprintf(string, "%0.2f m", dist);
            fontPrint(0, 0, colWhite, string);
        }
    }
#endif

#if RND_FRAME_RATE
    if (rndDisplayFrameRate)
    {
        char   string[256];
#if VERBOSE_SHIP_STATS
        sdword i;
#endif
        extern sdword trTextureChanges, trAvoidedChanges;

        if (RGL)
        {
            sprintf(string, "(%d . %d) (%u) %u . %u . %u",
                    trTextureChanges, trAvoidedChanges,
                    alodGetPolys(), rglNumPolys(),
                    rglCulledPolys(), meshRenders);
        }
        else
        {
            sprintf(string, "(%d . %d) (%u) . %u",
                    trTextureChanges, trAvoidedChanges,
                    alodGetPolys(), meshRenders);
        }

        trTextureChanges = 0;
        trAvoidedChanges = 0;

        meshRenders = 0;
        fontPrint(MAIN_WindowWidth - fontWidth(string), 0, colWhite, string);
#if VERBOSE_SHIP_STATS
        for (i = 0; i < 5; i++)
        {
            sprintf(string, "%d: %d / %d", i, shipsAtLOD[i], polysAtLOD[i]);
            shipsAtLOD[i] = 0;
            polysAtLOD[i] = 0;
            fontPrint(MAIN_WindowWidth - fontWidth(string), (i+1)*fontHeight(string), colWhite, string);
        }
#endif
#if DISPLAY_LOD_SCALE
        sprintf(string, "scale %f", lodScaleFactor);
        fontPrintf(0, fontHeight(string), colWhite, string);
#endif
        if (alodGetPanic())
        {
            sprintf(string, "PANIC!");
            fontPrint((MAIN_WindowWidth - fontWidth(string)) / 2, 0, colRGB(255,0,0), string);
        }

        if (universe.mainCameraCommand.currentCameraStack->focus.numShips == 1)
        {
            Ship* ship = universe.mainCameraCommand.currentCameraStack->focus.ShipPtr[0];
            vector veclen;
            real32 dist;

//            vecSub(veclen, ship->posinfo.position, camera->eyeposition);
            vecSub(veclen, ship->collInfo.collPosition, camera->eyeposition);
            dist = fsqrt(vecMagnitudeSquared(veclen));
            sprintf(string, "dist %0.2f m", dist);
            fontPrint((MAIN_WindowWidth - fontWidth(string)) / 2,
                      fontHeight(" "),
                      colWhite, string);
        }
    }
#endif
#if MEM_STATISTICS
        if (memStatsLogging)
        {
            sdword y = 0, x = 0, maxWidth = 0, width, index;
            fontPrint(0, y, colWhite, memStatString);
            for (index = 0; index < MS_NumberCookieNames; index++)
            {
                if (memStatsCookieNames[index].outputString[0] == 0)
                {
                    continue;
                }
                y += fontHeight(" ") + 1;
                width = fontWidth(memStatsCookieNames[index].outputString);
                maxWidth = max(maxWidth, width);
                if (y >= MAIN_WindowHeight)
                {
                    y = fontHeight(" ") + 1;
                    x += maxWidth + 1;
                    maxWidth = 0;
                    if (x > MAIN_WindowWidth)
                    {
                        break;
                    }
                }
                fontPrint(x, y, colWhite, memStatsCookieNames[index].outputString);
            }
        }
#endif
#if DEM_CHECKSUM
    if (demChecksumError)
    {
        fontPrint(2, MAIN_WindowHeight - 60, colWhite, demChecksumString);
    }
#endif

#if FONT_CHECKSPECIAL
    fontMakeCurrent(testing);
    fontPrint(10,50,colWhite,"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ");

    if ((keyIsStuck(CONTROLKEY)) && (keyIsHit(SHIFTKEY)))
    {
        testing--;
        if (testing < 1) testing = 1;
        keyClearSticky(CONTROLKEY);
    }
    else if (keyIsStuck(CONTROLKEY))
    {
        testing++;
        if (frFontRegistry[testing].name == NULL)
        {
            testing--;
        }
        keyClearSticky(CONTROLKEY);
    }

#endif

    trkTrackValuesDisplay();
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewRenderNothingFunction
    Description : Don't do anything; just return.  For when something else is
                    already rendering the screen.
    Inputs      : camera - it's not even used
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndMainViewRenderNothingFunction(Camera *camera)
{
    return;
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewAllButRenderFunction
    Description : Compute selection info for space obj's but don't render them
    Inputs      :
    Outputs     :
    Return      :
    Note        : Because this function mirrors much of the functionality of
                    rndMainViewRenderFunction, any drastic changes to
                    rndMainViewRenderFunction will have to be made this function.
----------------------------------------------------------------------------*/
void rndMainViewAllButRenderFunction(Camera *camera)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewRenderFunction
    Description : Render a mission sphere as referenced by a specific camera.
    Inputs      : camera - pointer to a camera structure which includes a
                    mission sphere.
    Outputs     : ??
    Return      : void
    Note        : Because much of the functionality of this function is mirrored
                    in rndMainViewAllButRenderFunction, any drastic changes to
                    rndMainViewRenderFunction will have to be made to
                    rndMainViewAllButRenderFunction.
----------------------------------------------------------------------------*/
void rndMainViewRenderFunction(Camera *camera)
{
    Node *objnode;
    bool twopass;
    SpaceObj *spaceobj;
    udword i,numstars;
    Star3d *star;
    static sdword init = FALSE;
//    hvector cameraSpace, eyeSpace;
    vector to;
    lod *level;
    ShipStaticInfo *shipStaticInfo;
    sdword playerIndex;
    hmatrix coordMatrixForGL;
//    matrix scaledMatrix;
//    real32 scaleFactor;
//    meshdata *worldMesh;
    Effect *effect;
    static sdword shipTrails;
    extern sdword trailsUpdated, trailsNotUpdated, trailsRendered;
    sdword colorScheme;
    bool displayEffect;

    sdword asteroid0Count;

    real32 scaledOffset[3];
    static real32 cameraOffset[3] = {0.0f, 0.0f, 0.0f};
    static bool   cameraFloating = FALSE;

    mouseCursorObjPtr  = NULL;               //Falko: got an obscure crash where mouseCursorObjPtr was mangled, will this work?

    dbgAssert(camera != NULL); //verify parameters

    if (!binkDonePlaying)
    {                               //!!! can be cleaned up using the render function pointer
        return;
    }

    if (RGL)
    {
        rglFeature(RGL_LOCK);                               //exclusive lock on the framebuffer
    }

    primModeClear2();                                       //go to 3D rendering mode

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(camera->fieldofview, rndAspectRatio,    //set projection matrix
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gameIsRunning && !nisIsRunning && piePointSpecMode == PSM_Idle && !universePause)
    {
        if (!cameraFloating)
        {
            rndResetFloatTime();
        }
        cameraFloating = TRUE;
    }
    else
    {
        cameraFloating = FALSE;
        if (nisIsRunning)
        {
            cameraOffset[0] = cameraOffset[1] = cameraOffset[2] = 0.0f;
        }
    }

    if (bitTest(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP))
    {
        cameraFloating = FALSE;
        cameraOffset[0] = cameraOffset[1] = cameraOffset[2] = 0.0f;
    }

    if (cameraFloating)
    {
        cameraOffset[0] = CAMERA_FLOAT_OFFSET_SCALAR0 * rndCameraOffset0();
        cameraOffset[1] = CAMERA_FLOAT_OFFSET_SCALAR1 * rndCameraOffset1();
        cameraOffset[2] = (0.5f * cameraOffset[0]) + (0.5f * cameraOffset[1]);
    }
    rndScaleCameraOffsets(scaledOffset, camera, cameraOffset);
    rgluLookAt(camera->eyeposition.x + scaledOffset[2],
               camera->eyeposition.y + scaledOffset[0],
               camera->eyeposition.z + scaledOffset[1],
               camera->lookatpoint.x, camera->lookatpoint.y, camera->lookatpoint.z,
               camera->upvector.x, camera->upvector.y, camera->upvector.z);
    //get local copy of matrices
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&rndCameraMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&rndProjectionMatrix));

    //position light(s) in world
    lightPositionSet();

    rndPreRenderDebugStuff(camera);

    //!!! should this be here?  It does not sound like it.
    if (feRenderEverything)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        feRenderEverything = FALSE;
    }

//draw the texture-mapped background
#if RND_BACKGROUND_STATIC
    glPushMatrix();
        glTranslatef(camera->eyeposition.x, camera->eyeposition.y, camera->eyeposition.z);
#endif
        primErrorMessagePrint();

        numstars=universe.star3dinfo->Num3dStars;
        if (stararray == NULL)
        {
            c4ub_v3f* ptr;

            stararray = (c4ub_v3f*)memAlloc((numstars - NUM_BIG_STARS) * sizeof(c4ub_v3f), "stararray", NonVolatile);
            bigstararray = (c4ub_v3f*)memAlloc(NUM_BIG_STARS * sizeof(c4ub_v3f), "bigstararray", NonVolatile);

            ptr = stararray;
            for (i = 0, star = universe.star3dinfo->Stars; i < (numstars - NUM_BIG_STARS); i++, star++, ptr++)
            {
                ptr->c[0] = star->r;
                ptr->c[1] = star->g;
                ptr->c[2] = star->b;
                ptr->c[3] = 200;
                ptr->v[0] = star->position.x;
                ptr->v[1] = star->position.y;
                ptr->v[2] = star->position.z;
            }

            for (ptr = bigstararray; i < numstars; i++, star++, ptr++)
            {
                ptr->c[0] = star->r;
                ptr->c[1] = star->g;
                ptr->c[2] = star->b;
                ptr->c[3] = 200;
                ptr->v[0] = star->position.x;
                ptr->v[1] = star->position.y;
                ptr->v[2] = star->position.z;
            }
        }

        rndLightingEnable(FALSE);                           //stars are self-illuminated
        primErrorMessagePrint();
        glDisable(GL_DEPTH_TEST);                           //and infinitely far away
        primErrorMessagePrint();
        if (singlePlayerGame && singlePlayerGameInfo.currentMission == 13)
        {
            rndBackgroundRender(BG_RADIUS, camera, !showBackgrounds);
        }
        else
        {
            rndBackgroundRender(BG_RADIUS, camera, TRUE);
        }

        rndTextureEnvironment(RTE_Modulate);
        glEnable(GL_DEPTH_TEST);                            //re-enable depth test
        primErrorMessagePrint();
        rndLightingEnable(TRUE);                            //and lighting
        primErrorMessagePrint();
#if RND_BACKGROUND_STATIC
    glPopMatrix();
#endif
#if !LIGHT_PLAYER_COLORS
//    lightDefaultLightSet();
    lightSetLighting();
#endif

    //render all worlds in the universe
    for (i = 0; i < UNIV_NUMBER_WORLDS; i++)
    {
        if (universe.world[i] != NULL)
        {                                                   //if this one defined
            if (universe.world[i]->objtype == OBJ_DerelictType)
            {                                               //if it's a derelict
                rndRenderAHomeworld(camera, universe.world[i]);
            }
            else
            {                                               //else it must be a world effect
                dbgAssert(universe.world[i]->objtype == OBJ_EffectType);
                rndRenderAWorldEffect(camera, (Effect *)universe.world[i]);
            }
        }
        else
        {                                                   //stop processing worlds
            break;
        }
    }
    if (rndPreObjectCallback != NULL)
    {                                                       //render the pre-object callback if needed
        rendercallback rc = rndPreObjectCallback;
        rndPreObjectCallback = NULL;
        rc();
    }

    if (RGL && rndFogOn)
    {
        glEnable(GL_FOG);
    }

#if PIE_MOVE_NEARTO
    //clear out the 'move near to' info
    selClosestTarget = NULL;
    selClosestDistance = SDWORD_Max;
#endif //PIE_MOVE_NEARTO

    trailsRendered = shipTrails = 0;
    alodSetPolys(0);

#if WILL_TWO_PASS
    if (RGL && (RGLtype == SWtype))
    {
        twopass = TRUE;
    }
    else
    {
        twopass = FALSE;
    }
#else
    twopass = FALSE;
#endif
    if (twopass)
    {
        objnode = universe.RenderList.tail;
    }
    else
    {
        objnode = universe.RenderList.head;
    }

    while (objnode != NULL)
    {
        spaceobj = (SpaceObj *)listGetStructOfNode(objnode);

        g_WireframeHack = FALSE;
        rndPerspectiveCorrection(FALSE);

        switch (spaceobj->objtype)
        {
            case OBJ_BulletType:                            //type bullet
                if (((Bullet *)spaceobj)->effect == NULL)
                {                                           //if there is no effect for this bullet
                    rndLightingEnable(FALSE);               //draw default line
                    rndTextureEnable(FALSE);
                    if(((Bullet *)spaceobj)->bulletType == BULLET_Laser)
                    {   //bullet update 'feature' to draw laser to bullet
                        defenseFighterAdjustLaser(((Bullet *)spaceobj));
                    }
                    else if (((Bullet *)spaceobj)->bulletType == BULLET_Beam)
                    {
                        if(((Bullet *)spaceobj)->timelived <= UNIVERSE_UPDATE_PERIOD)
                        {
                            goto dontdraw;
                        }
                    }
                    vecAdd(to,spaceobj->posinfo.position,((Bullet *)spaceobj)->lengthvec);
                    primLine3(&spaceobj->posinfo.position, &to, ((Bullet *)spaceobj)->bulletColor);
dontdraw:
                    rndLightingEnable(TRUE);
                }
                break;
            case OBJ_EffectType:                            //if type effect
                if (!twopass)
                {
                    effect = (Effect *)spaceobj;                    //get effect pointer
                    if(effect->owner != NULL)
                    {
                       SpaceObj *effectowner = (SpaceObj *)effect->owner;
                       if(effectowner->objtype == OBJ_BulletType)
                       {
                         //effect is owned by a bullet
                         if(((Bullet *) effectowner)->bulletType == BULLET_Laser)
                         {
                            //effect is owned by a defense fighter laser beam
                            defenseFighterAdjustLaser(((Bullet *) effectowner));
                            etgEffectUpdate(effect, 0.0f);                        //luke suggested fix to laser length problem
                         }
                         else if(((Bullet *) effectowner)->bulletType == BULLET_Beam)
                         {
                             if(((Bullet *)effectowner)->timelived <= UNIVERSE_UPDATE_PERIOD)
                             {
                                goto dontdraw2;
                             }
                         }
                       }
                    }
                    etgEffectDraw(effect);
dontdraw2:;
                }
                break;
            case OBJ_ShipType:
                if (spaceobj->staticinfo->staticheader.LOD != NULL)
                {
                    if(!bitTest(spaceobj->flags,SOF_Cloaked) ||
                       ((Ship *)spaceobj)->playerowner == universe.curPlayerPtr ||
                       (proximityCanPlayerSeeShip(universe.curPlayerPtr,((Ship *)spaceobj))))       //if ship isn't cloaked or it is the players
                    {
                        //put the ship into the mission map array
//                        mmAddShipToMapArray((ShipPtr)spaceobj);
                        shipStaticInfo = (ShipStaticInfo *)spaceobj->staticinfo;
                        playerIndex = (sdword)((Ship *)spaceobj)->playerowner->playerIndex;
                        dbgAssert((spaceobj->flags & SOF_Rotatable) != 0);
                        selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

#if RND_VISUALIZATION
    #ifdef DEBUG_COLLISIONS
                        collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
    #else
                        if (RENDER_BOXES)
                        {
                            collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                        }
    #endif
#endif

                        glPushMatrix();
                        level = lodLevelGet((void *)spaceobj, &camera->eyeposition, &((SpaceObjRotImp *)spaceobj)->collInfo.collPosition);

                        if (taskTimeElapsed-((Ship *)spaceobj)->flashtimer < FLASH_TIMER)
                        {
                            g_ReplaceHack = TRUE;
                            if (RGL)
                            {
                                glPixelTransferf(GL_RED_BIAS, 0.3f);
                                glPixelTransferf(GL_GREEN_BIAS, 0.3f);
                                glPixelTransferf(GL_BLUE_BIAS, 0.3f);
                            }
                        }

                        switch (level->flags & LM_LODType)
                        {
                            case LT_Invalid:
                                break;
                            case LT_Mesh:
                                displayEffect = hsShouldDisplayEffect((Ship*)spaceobj);
#if LIGHT_PLAYER_COLORS
                                lightPlayerColorLightSet(playerIndex);
#endif //LIGHT_PLAYER_COLORS
                                hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)spaceobj)->rotinfo.coordsys);
                                hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position,coordMatrixForGL);
                                glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                                shPushLightMatrix(&coordMatrixForGL);
                                if (selCameraSpace.z < 0.0f)
                                {
                                    //((Ship *)spaceobj)->magnitudeSquared = 1.0f - selCameraSpace.z * shipStaticInfo->NLIPS;
#if RND_SCALECAP_TWEAK
                                    if (selSelected.numShips == 1 &&
                                        selSelected.ShipPtr[0]->staticinfo == shipStaticInfo)
                                    {
                                        if (keyIsHit(ONEKEY))
                                        {
                                            shipStaticInfo->scaleCap -= scaleCapSlopeDelta;
                                            dbgMessagef(scaleCapString, "\nN-LIPS = %f", shipStaticInfo->scaleCap);
                                            sprintf(scaleCapString, "N-LIPS = %f", shipStaticInfo->scaleCap);
                                        }
                                        if (keyIsHit(TWOKEY))
                                        {
                                            shipStaticInfo->scaleCap += scaleCapSlopeDelta;
                                            dbgMessagef(scaleCapString, "\nN-LIPS = %f", shipStaticInfo->scaleCap);
                                            sprintf(scaleCapString, "N-LIPS = %f", shipStaticInfo->scaleCap);
                                        }
                                    }
#endif //RND_SCALECAP_TWEAK

                                    if (shipStaticInfo->scaleCap != 0.0f)
                                    {                   //if there's a scaling cap
                                        real32 scaleFactor;
                                        if (glCapFeatureExists(GL_RESCALE_NORMAL))
                                        {
                                            glEnable(GL_RESCALE_NORMAL);
                                            rndNormalization = TRUE;
                                        }
                                        else
                                        {
                                            rndNormalizeEnable(TRUE);
                                        }
                                        scaleFactor = 1.0f - selCameraSpace.z * shipStaticInfo->scaleCap;
#if RND_VERBOSE_LEVEL >= 1
                                        if (_isnan((double)scaleFactor))
                                        {
                                            dbgMessage("\n-- scaleFactor is NaN --");
                                        }
#endif
                                        ((Ship *)spaceobj)->magnitudeSquared = scaleFactor;
                                        glScalef(scaleFactor, scaleFactor, scaleFactor);
                                        ((Ship *)spaceobj)->collInfo.selCircleRadius *= scaleFactor;
                                    }
                                    else
                                    {
                                        ((Ship *)spaceobj)->magnitudeSquared = 1.0f;
                                    }
                                }
                                else                        //don't even bother rendering if behind camera
                                {
                                    ((Ship *)spaceobj)->magnitudeSquared = 1.0f;
//!!!                                    break;
                                }
#if SO_CLOOGE_SCALE
                                if (shipStaticInfo->scaleFactor != 1.0f)
                                {
                                    glScalef(shipStaticInfo->scaleFactor,
                                             shipStaticInfo->scaleFactor,
                                             shipStaticInfo->scaleFactor);
                                }
#endif
#if RND_VISUALIZATION
                                if (dockLines) dockDrawDockInfo((Ship *)spaceobj);
                                if (gunLines) gunDrawGunInfo((Ship *)spaceobj);
                                if (dockLines) dockDrawSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)spaceobj);
#endif

 /*
                                switch (shipStaticInfo->shiprace)
                                {
                                    case R1:
                                    case R2:
                                        i = ((Ship *)spaceobj)->playerowner->playerIndex;
                                        dbgAssert(i >= 0 && i < 8);//!!!
                                        break;
                                    default:
                                        i = 0;
                                        break;
                                }
*/
                                i = ((Ship *)spaceobj)->colorScheme;
                                dbgAssert(i >= 0 && i < TE_NumberPlayers);
                                if (bitTest(spaceobj->flags,SOF_Cloaked))
                                {
                                    g_WireframeHack = (bool8)((((Ship *)spaceobj)->playerowner == universe.curPlayerPtr) || proximityCanPlayerSeeShip(universe.curPlayerPtr,((Ship *)spaceobj)));
                                }

                                if (rndShipVisible(spaceobj, camera))
                                {
                                    bool result = rndFade(spaceobj, camera);
                                    Ship* ship = (Ship*)spaceobj;
                                    ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;

                                    if (ssinfo != NULL)
                                    {
                                        if (ssinfo->shipHyperspaceState == SHIPHYPERSPACE_NONE)
                                        {
                                            ssinfo = NULL;
                                        }
                                    }

                                    if (!result)
                                    {
                                        if (spaceobj->currentLOD == 0)
                                        {
                                            rndPerspectiveCorrection(TRUE);
                                        }

                                        meshRenders++;

                                        if (rndInsideShip(spaceobj, camera))
                                        {
                                            g_WireframeHack = TRUE;
                                            g_HiddenRemoval = FALSE;
                                        }

                                        if (ssinfo != NULL &&
                                            ssinfo->hsState != HS_INACTIVE &&
                                            ssinfo->hsState != HS_FINISHED)
                                        {
                                            if (ssinfo->staticHyperspaceGate)
                                            {
                                                hsNoGate(TRUE);
                                            }
                                            hsContinue((Ship*)spaceobj, displayEffect);
                                            if (ssinfo->staticHyperspaceGate)
                                            {
                                                hsNoGate(FALSE);
                                            }
                                        }

                                        //
                                        // lighting while docking
                                        //
                                        if (((Ship*)spaceobj)->dockvars.reserveddocking != -1 &&
                                            ((Ship*)spaceobj)->dockvars.dockship != NULL)
                                        {
                                            Ship* dockship;
                                            real32 t, nearVal, farVal;

                                            dockship = ((Ship*)spaceobj)->dockvars.dockship;
                                            nearVal = dockship->staticinfo->dockLightNear;
                                            farVal  = dockship->staticinfo->dockLightFar;
                                            t = rndDockScalar((Ship*)spaceobj,
                                                              ((Ship*)spaceobj)->dockvars.dockship,
                                                              nearVal, farVal);
                                            shDockLightColor(dockship->staticinfo->dockLightColor);
                                            shDockLight(t);
                                        }

                                        if ((ssinfo != NULL &&
                                             (ssinfo->hsState != HS_FINISHED || singlePlayerGameInfo.hyperspaceFails)) ||
                                            ssinfo == NULL)
                                        {
                                            extern sdword visiblePoly;
                                            extern bool g_SpecificPoly;
                                            extern bool g_Points;
                                            if (((Ship *)spaceobj)->bindings != NULL)
                                            {
                                                meshRenderShipHierarchy(((Ship *)spaceobj)->bindings,
                                                        ((Ship *)spaceobj)->currentLOD,
                                                        (meshdata *)level->pData, i);
#if VISIBLE_POLYS
                                                if (visiblePoly >= 0)
                                                {
                                                    g_SpecificPoly = TRUE;
                                                    g_WireframeHack = TRUE;
                                                    glScalef(1.02f, 1.02f, 1.02f);
                                                    meshRenderShipHierarchy(((Ship*)spaceobj)->bindings,
                                                                            ((Ship*)spaceobj)->currentLOD,
                                                                            (meshdata*)level->pData, i);
                                                    g_WireframeHack = FALSE;
                                                    glScalef(1.01f, 1.01f, 1.01f);
                                                    g_Points = TRUE;
                                                    meshRenderShipHierarchy(((Ship*)spaceobj)->bindings,
                                                                            ((Ship*)spaceobj)->currentLOD,
                                                                            (meshdata*)level->pData, i);
                                                    g_Points = FALSE;
                                                    g_SpecificPoly = FALSE;
                                                }
#endif
                                            }
                                            else
                                            {
                                                meshRender((meshdata *)level->pData, i);
#if VISIBLE_POLYS
                                                if (visiblePoly >= 0)
                                                {
                                                    g_SpecificPoly = TRUE;
                                                    g_WireframeHack = TRUE;
                                                    glScalef(1.02f, 1.02f, 1.02f);
                                                    meshRender((meshdata *)level->pData, i);
                                                    g_WireframeHack = FALSE;
                                                    glScalef(1.01f, 1.01f, 1.01f);
                                                    g_Points = TRUE;
                                                    meshRender((meshdata *)level->pData, i);
                                                    g_Points = FALSE;
                                                    g_SpecificPoly = FALSE;
                                                }
#endif
                                            }
                                            spaceobj->renderedLODs |= (1 << spaceobj->currentLOD);

                                            rndPerspectiveCorrection(FALSE);

                                            //navlights
                                            if (!bitTest(spaceobj->flags, SOF_Cloaked))
                                            {
                                                RenderNAVLights((Ship*)spaceobj);
                                            }
                                        }

                                        g_WireframeHack = FALSE;
                                        g_HiddenRemoval = TRUE;

                                        shDockLight(0.0f);

#if VERBOSE_SHIP_STATS
                                        if (rndDisplayFrameRate)
                                        {
                                            shipsAtLOD[spaceobj->currentLOD]++;
                                            {
                                                meshdata* mesh = (meshdata*)level->pData;
                                                polysAtLOD[spaceobj->currentLOD] += mesh->object[0].nPolygons;
                                            }
                                        }
#endif
                                    }

                                    rndUnFade();
                                }
                                shPopLightMatrix();

                                if (glCapFeatureExists(GL_RESCALE_NORMAL))
                                {
                                    glDisable(GL_RESCALE_NORMAL);
                                    rndNormalization = FALSE;
                                }
                                else
                                {
                                    rndNormalizeEnable(FALSE);
                                }
                                break;
                            case LT_TinySprite:
                                dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                                break;
                            case LT_SubPixel:
                                rndTextureEnable(FALSE);
                                rndLightingEnable(FALSE);
                                primPointSize3(&spaceobj->posinfo.position,
                                    ((Ship *)spaceobj)->collInfo.selCircleRadius,
                                    toPlayerColor[playerIndex]);
                                rndLightingEnable(TRUE);
                                break;
                            case LT_Function:
                                dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                                break;
                            case LT_NULL:
                                break;
                            default:
                                dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                        }

                        if (g_ReplaceHack)
                        {
                            g_ReplaceHack = FALSE;
                            if (RGL)
                            {
                                glPixelTransferf(GL_RED_BIAS, 0.0f);
                                glPixelTransferf(GL_GREEN_BIAS, 0.0f);
                                glPixelTransferf(GL_BLUE_BIAS, 0.0f);
                            }
                        }

                        {
                            udword index;
                            for (index = 0; index < 2; index++)
                            {
                                if (((Ship*)spaceobj)->lightning[index] != NULL)
                                {
                                    lightning* l = (lightning*)((Ship*)spaceobj)->lightning[index];
                                    cloudRenderAndUpdateLightning(l, spaceobj->currentLOD);

                                    l->countdown--;
                                    if (l->countdown == 0)
                                    {
                                        cloudKillLightning(l);
                                        ((Ship*)spaceobj)->lightning[index] = NULL;
                                    }
                                }
                            }
                        }

                        glPopMatrix();

                        {
                            Ship* ship = (Ship*)spaceobj;
                            ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;

                            if (ssinfo != NULL &&
                                ssinfo->hsState != HS_INACTIVE &&
                                ssinfo->hsState != HS_FINISHED)
                            {
                                if (ssinfo->staticHyperspaceGate)
                                {
                                    hsNoGate(TRUE);
                                }
                                hsEnd((Ship*)spaceobj, displayEffect);
                                if (ssinfo->staticHyperspaceGate)
                                {
                                    hsNoGate(FALSE);
                                }
                            }
                        }

                        rndPerspectiveCorrection(FALSE);

                        rndFade(spaceobj, camera);
                        if (!bitTest(spaceobj->flags, SOF_Cloaked) || (((Ship*)spaceobj)->playerowner == universe.curPlayerPtr) || proximityCanPlayerSeeShip(universe.curPlayerPtr,(Ship*)spaceobj))

                        {
                            udword idx;
                            for (idx = 0; idx < MAX_NUM_TRAILS; idx++)
                            {
                                if (((Ship*)spaceobj)->trail[idx] != NULL)
                                {
                                    trailDraw(&((Ship*)spaceobj)->enginePosition,
                                              ((Ship*)spaceobj)->trail[idx],
                                              spaceobj->currentLOD,
                                              ((Ship *)spaceobj)->colorScheme);
                                }
                            }
                        }
                        rndUnFade();
                    }
                }
                else
                {                                           //else behind camera
                    ((Ship *)spaceobj)->collInfo.selCircleRadius = -1.0f;
                }
                break;
            case OBJ_DerelictType:
                if (((Derelict *)spaceobj)->staticinfo->worldRender)
                {                                           //if it's a world derelict
                    break;                                  //render as normal
                }

                if (((Derelict *)spaceobj)->derelicttype == HyperspaceGate)
                {
                    selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

#if RND_VISUALIZATION
                    if (RENDER_BOXES)
                    {
                        collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                    }
#endif
                    break;
                }

                //else fall through
            case OBJ_AsteroidType:
            case OBJ_DustType:
            case OBJ_GasType:
            case OBJ_MissileType:
                //cloaked effect hack...change later when implement 'stages maybe'
                if(bitTest(spaceobj->flags,SOF_Cloaked))
                    break;   //if it isn't cloaked, or it is and it is the local players field draw it

                if (spaceobj->staticinfo->staticheader.LOD != NULL)
                {
                    dbgAssert((spaceobj->flags & (SOF_Rotatable+SOF_Impactable+SOF_Targetable)) != 0);

                    //add this space object to the mission map array
//                    mmAddObjToMapArray(spaceobj);

                    selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

#if RND_VISUALIZATION
#ifdef DEBUG_COLLISIONS
                    collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
#else
                    if (RENDER_BOXES)
                    {
                        collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                    }
#endif
#endif

                    glPushMatrix();
                    level = lodLevelGet((void *)spaceobj, &camera->eyeposition, &((SpaceObjRotImp *)spaceobj)->collInfo.collPosition);

                    if (taskTimeElapsed-((Ship *)spaceobj)->flashtimer < FLASH_TIMER)
                    {
                        g_ReplaceHack = TRUE;
                        if (RGL)
                        {
                            glPixelTransferf(GL_RED_BIAS, 0.3f);
                            glPixelTransferf(GL_GREEN_BIAS, 0.3f);
                            glPixelTransferf(GL_BLUE_BIAS, 0.3f);
                        }
                    }

                    switch (level->flags & LM_LODType)
                    {
                        case LT_Invalid:
                            break;
                        case LT_Mesh:
#if LIGHT_PLAYER_COLORS
//                            lightDefaultLightSet();
//!!!                            lightSetLighting();
#endif
                            g_RndPosition = spaceobj->posinfo.position;
                            hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)spaceobj)->rotinfo.coordsys);
                            hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position,coordMatrixForGL);
                            glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                            shPushLightMatrix(&coordMatrixForGL);

                            if (spaceobj->objtype == OBJ_AsteroidType)
                            {
                                real32 scaling = ((Asteroid *)spaceobj)->scaling;
                                if (scaling != 1.0f)
                                {
                                    glScalef(scaling,scaling,scaling);
                                }
                            }

#ifndef HW_Release
                            if(spaceobj->objtype == OBJ_DerelictType)
                            if(((Derelict *)spaceobj)->derelicttype < NUM_DERELICTTYPES)
                            if (dockLines) dockDrawSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)spaceobj);
#endif
                            colorScheme = 0;
                            switch (spaceobj->objtype)
                            {
                            case OBJ_GasType:
                            case OBJ_DustType:
                                rndFade(spaceobj, camera);
                                meshRenders++;
                                cloudRenderSystem((void*)level->pData, ((DustCloud*)spaceobj)->stub, spaceobj->currentLOD);
//                                shipsAtLOD[spaceobj->currentLOD]++;
                                rndUnFade();
                                break;
                            case OBJ_MissileType:
                                colorScheme = ((Missile *)spaceobj)->colorScheme;
                                goto renderDefault;
                            case OBJ_DerelictType:
                                colorScheme = ((Derelict *)spaceobj)->colorScheme;
                                //fall through
renderDefault:
                            default:
                                if (rndShipVisible(spaceobj, camera))
                                {
                                    rndFade(spaceobj, camera);

                                    if (spaceobj->currentLOD == 0)
                                    {
                                        rndPerspectiveCorrection(TRUE);
                                    }
                                    rndGLStateLog("Before Derelict");
                                    meshRenders++;
                                    meshRender((meshdata *)level->pData,colorScheme);
                                    spaceobj->renderedLODs |= (1 << spaceobj->currentLOD);
                                    rndGLStateLog("After Derelict");
                                    //navlights
                                    if (spaceobj->objtype == OBJ_DerelictType && !bitTest(spaceobj->flags, SOF_Cloaked))
                                    {
                                        RenderNAVLights((Ship*)spaceobj);
                                    }

                                    rndPerspectiveCorrection(FALSE);
#if VERBOSE_SHIP_STATS
                                    if (rndDisplayFrameRate)
                                    {
                                        shipsAtLOD[spaceobj->currentLOD]++;
                                        {
                                            meshdata* mesh = (meshdata*)level->pData;
                                            polysAtLOD[spaceobj->currentLOD] += mesh->object[0].nPolygons;
                                        }
                                    }
#endif
                                    rndUnFade();
                                }
                            }
                            shPopLightMatrix();
                            break;

                        case LT_TinySprite:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                            break;
                        case LT_SubPixel:
                            rndTextureEnable(FALSE);
                            rndLightingEnable(FALSE);
                            primPointSize3(&spaceobj->posinfo.position,
                                           ((Ship *)spaceobj)->collInfo.selCircleRadius,
                                           spaceobj->staticinfo->staticheader.LOD->pointColor);
                            rndLightingEnable(TRUE);
                            break;
                        case LT_Function:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                            break;
                        case LT_NULL:
                            break;
                        default:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                    }

                    if (g_ReplaceHack)
                    {
                        g_ReplaceHack = FALSE;
                        if (RGL)
                        {
                            glPixelTransferf(GL_RED_BIAS, 0.0f);
                            glPixelTransferf(GL_GREEN_BIAS, 0.0f);
                            glPixelTransferf(GL_BLUE_BIAS, 0.0f);
                        }
                    }

                    glPopMatrix();

                    if (spaceobj->objtype == OBJ_MissileType)
                    {
                        Missile* mis = (Missile*)spaceobj;

                        if (mis->trail != NULL)
                        {
                            playerIndex = (sdword)mis->playerowner->playerIndex;
                            mistrailDraw(&mis->enginePosition, mis->trail,
                                         spaceobj->currentLOD, mis->colorScheme);
                        }
                    }
                }
                break;
            case OBJ_NebulaType:
                //do nothing
                break;
            default:
                dbgFatalf(DBG_Loc, "Undefined object type %d", spaceobj->objtype);
        }
        if (twopass)
        {
            objnode = objnode->prev;
        }
        else
        {
            objnode = objnode->next;
        }
    }

    if (twopass)
    {
        objnode = universe.RenderList.head;
        while (objnode != NULL)
        {
            spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
            g_WireframeHack = FALSE;
            rndPerspectiveCorrection(FALSE);
            switch (spaceobj->objtype)
            {
            case OBJ_EffectType:
                effect = (Effect *)spaceobj;                    //get effect pointer
                if(effect->owner != NULL)
                {
                   SpaceObj *effectowner = (SpaceObj *)effect->owner;
                   if(effectowner->objtype == OBJ_BulletType)
                   {
                     //effect is owned by a bullet
                     if(((Bullet *) effectowner)->bulletType == BULLET_Laser)
                     {
                        //effect is owned by a defense fighter laser beam
                        defenseFighterAdjustLaser(((Bullet *) effectowner));
                        etgEffectUpdate(effect, 0.0f);                        //luke suggested fix to laser length problem
                     }
                     else if(((Bullet *) effectowner)->bulletType == BULLET_Beam)
                     {
                         if(((Bullet *)effectowner)->timelived <= UNIVERSE_UPDATE_PERIOD)
                         {
                            goto dontdraw3;
                         }
                     }
                   }
                }
                etgEffectDraw(effect);
dontdraw3:;
            }
            objnode = objnode->next;
        }
    }

    //
    // minor renderlist (asteroid0 list)
    //
    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);
    g_WireframeHack = FALSE;
    glPointSize(1.0f);

    asteroid0Count = 0;

    objnode = universe.MinorRenderList.head;
    while (objnode != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(objnode);

        switch (spaceobj->objtype)
        {
        case OBJ_AsteroidType:
            asteroid0Data[asteroid0Count].c = spaceobj->staticinfo->staticheader.LOD->pointColor;
            asteroid0Data[asteroid0Count].position = &spaceobj->posinfo.position;
            asteroid0Count++;
            if (asteroid0Count == ASTEROID0DATA_SIZE)
            {
                rndDrawAsteroid0(asteroid0Count);
                asteroid0Count = 0;
            }
            break;

        default:
            dbgFatalf(DBG_Loc, "MinorRenderList contains invalid object type %d", spaceobj->objtype);
        }

        objnode = objnode->next;
    }
    if (asteroid0Count != 0)
    {
        rndDrawAsteroid0(asteroid0Count);
    }
    rndLightingEnable(TRUE);

    //hyperspace
    hsStaticRender();

    alodAdjustScaleFactor();

    rndPerspectiveCorrection(FALSE);

    if (RGL)
    {
        glDisable(GL_FOG);
    }

    nebRender();

    if (rndPostObjectCallback != NULL)
    {                                                       //render the post-object callback if needed
        rndPostObjectCallback();
        rndPostObjectCallback = NULL;
    }
    rndPostRenderDebug3DStuff(camera);
    primModeSet2();
    rndPostRenderDebug2DStuff(camera);
    if (RGL)
    {
        rglFeature(RGL_UNLOCK);         //remove our exclusive lock
    }
}

GLuint plug_handle = 0;
sdword plug_width, plug_height;

typedef struct tagTGAHeader
{
    unsigned char idLength;
    unsigned char colorMapType;
    unsigned char imageType;
    // Color Map information.
    unsigned short colorMapStartIndex;
    unsigned short colorMapNumEntries;
    unsigned char colorMapBitsPerEntry;
    // Image Map information.
    signed short imageOffsetX;
    signed short imageOffsetY;
    unsigned short imageWidth;
    unsigned short imageHeight;
    unsigned char pixelDepth;
    unsigned char imageDescriptor;
} TGAHeader;

//rndLoadTarga
udword rndLoadTarga(char* filename, sdword* width, sdword* height)
{
    TGAHeader head;
    ubyte* data;
    ubyte* pdata;
    ubyte* bp;
    uword* psdata;
    sdword i;
    GLuint handle;

    fileLoadAlloc(filename, (void**)&data, 0);

    pdata = data;
    head.idLength = *pdata++;
    head.colorMapType = *pdata++;
    head.imageType = *pdata++;
    psdata = (uword*)pdata;
    head.colorMapStartIndex = *psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.colorMapNumEntries = *psdata;
    pdata += 2;
    head.colorMapBitsPerEntry = *pdata++;
    psdata = (uword*)pdata;
    head.imageOffsetX = (sword)*psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageOffsetY = (sword)*psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageWidth = *psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageHeight = *psdata;
    pdata += 2;
    head.pixelDepth = *pdata++;
    head.imageDescriptor = *pdata++;

    dbgAssert(head.pixelDepth == 32);

    pdata += head.idLength;

    *width  = (sdword)head.imageWidth;
    *height = (sdword)head.imageHeight;

    for (i = 0, bp = pdata; i < 4*(*width)*(*height); i += 4, bp += 4)
    {
        ubyte r, b;
        r = bp[0];
        b = bp[2];
        bp[0] = b;
        bp[2] = r;
        bp[3] = (ubyte)(255 - (int)bp[3]);
    }

    glGenTextures(1, &handle);
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pdata);

    memFree(data);

    return handle;
}

/*-----------------------------------------------------------------------------
    Name        : rndLoadShamelessPlug
    Description : loads or frees the Relic shameless plug texture
    Inputs      : on - TRUE or FALSE (load or free)
    Outputs     : plug_handle is either valid or invalid
    Return      :
----------------------------------------------------------------------------*/
void rndLoadShamelessPlug(bool on)
{
    if (on)
    {
        plug_handle = rndLoadTarga("plug.tga", &plug_width, &plug_height);
    }
    else
    {
        if (plug_handle != 0)
        {
            glDeleteTextures(1, &plug_handle);
            plug_handle = 0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndShamelessPlug
    Description : displays the Relic shameless plug in bottom right corner of screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndShamelessPlug()
{
    GLboolean blendOn, depthOn;
    GLfloat   fwidth, fheight;
    GLint     matrixMode;
    sdword    lightOn;

    GLfloat   projection[16];
    GLfloat   winWidth, winHeight;

#ifdef fpoiker
    return;
#endif

#if RND_PLUG_DISABLEABLE
    if (!rndShamelessPlugEnabled)
    {
        return;
    }
#endif

    if (plug_handle == 0)
    {
        rndLoadShamelessPlug(TRUE);
    }

    fwidth  = (GLfloat)plug_width;
    fheight = (GLfloat)plug_height;
    winWidth  = (GLfloat)MAIN_WindowWidth;
    winHeight = (GLfloat)MAIN_WindowHeight;

    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluOrtho2D(0.0f, winWidth, 0.0f, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    blendOn = glIsEnabled(GL_BLEND);
    lightOn = rndLightingEnable(FALSE);
    depthOn = glIsEnabled(GL_DEPTH_TEST);
    if (!blendOn) glEnable(GL_BLEND);
    if (depthOn)  glDisable(GL_DEPTH_TEST);

    winWidth  -= 1.0f;
    winHeight -= 1.0f;

    if (RGLtype == SWtype)
    {
        glTexCoord2f(winWidth - fwidth, 0);
        trClearCurrent();
        glBindTexture(GL_TEXTURE_2D, plug_handle);
        glDrawPixels(plug_width, plug_height, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
        rndTextureEnable(TRUE);
        rndAdditiveBlends(FALSE);
        trClearCurrent();
        glBindTexture(GL_TEXTURE_2D, plug_handle);

        glColor4ub(255,255,255,255);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(winWidth - fwidth, fheight);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(winWidth - fwidth, 0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(winWidth, 0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(winWidth, fheight);
        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    if (matrixMode == GL_MODELVIEW)
    {
        glMatrixMode(GL_MODELVIEW);
    }

    rndLightingEnable(lightOn);
    if (!blendOn) glDisable(GL_BLEND);
    if (depthOn)  glEnable(GL_DEPTH_TEST);
}

extern udword receivedPacketNumber;

static udword printLagMinFrames = 0;

extern udword PRINTLAG_IFGREATERTHAN;
extern udword PRINTLAG_MINFRAMES;

/*-----------------------------------------------------------------------------
    Name        : rndDrawOnScreenSyncStatus
    Description : Prints sync status to screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndDrawOnScreenSyncStatus(void)
{
#if SYNC_CHECK
    if ((multiPlayerGame|playPackets) && (gameIsRunning))
    {
        sdword y;
        udword rsyncpkts = queueNumberEntries(ProcessSyncPktQ);

        if (randSyncErr)
        {
            fontPrintf(400,10,colWhite,"RAND SYNC ERR: Frame %d",randSyncErrFrame);
        }
        if (univSyncErr)
        {
            fontPrintf(400,20,colWhite,"UNIV SYNC ERR: Frame %d",univSyncErrFrame);
        }
        if ((blobSyncErr) && (multiPlayerGame)) // only print cheat detect in multiPlayerGame
        {
            fontPrintf(400,30,colWhite,strGetString(strCheatDetect),blobSyncErrFrame);
        }
        if (pktSyncErr)
        {
            fontPrint(400,40,colWhite,"PKT SYNC ERR");
        }

        y = 50;

        if ((printLagMinFrames) || (rsyncpkts > PRINTLAG_IFGREATERTHAN))
        {
            if (rsyncpkts > PRINTLAG_IFGREATERTHAN)
                printLagMinFrames = PRINTLAG_MINFRAMES;
            else if (printLagMinFrames)
                printLagMinFrames--;

            //fontPrintf(400,y,colWhite,"Lag >= %3.3f",(real32)(rsyncpkts * CAPTAINSERVER_PERIOD));
            lagSlowComputerIcon();
        }

        lagSlowInternetIcon();

        if ((playPackets) && (!recordplayPacketsInGame))
        {
            fontPrintf(400,y,colWhite,"%d %f",receivedPacketNumber,universe.totaltimeelapsed);
            y += 10;
        }

/*        if (multiPlayerGame)
        {
            sdword i;

            for (i=0;i<sigsNumPlayers;i++)
            {
                if (playersReadyToGo[i] == PLAYER_DROPPED_OUT)
                {
                    fontPrintf(400,y,HorseRaceDropoutColor,"%s %s",playerNames[i],strGetString(strDroppedOut));
                    y += 10;
                }
            }
        }*/
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : rndDrawOnScreenDebugInfo
    Description : Handle render-time debugging info to print on the screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VERBOSE_LEVEL >= 1
extern sdword numchans[];
extern sdword channelsinuse;

void rndDrawOnScreenDebugInfo(void)
{
    static sdword counter = 0;
    static color c;
    sdword y = 0;
#if MR_TEST_HPB
    if (mrTestHPBMode)
    {
        if (selSelected.numShips == 1)
        {
            sdword width = fontWidth("H:  -9999.00");
            fontPrintf(MAIN_WindowWidth - width, 0, colWhite, "H:  %.2f", mrHeading * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, fontHeight(" ") + 1, colWhite, "P:  %.2f", mrPitch * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, (fontHeight(" ") + 1) * 2, colWhite, "B:  %.2f", mrBank * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, (fontHeight(" ") + 1) * 3, colWhite, "I: %d", madTestHPBIndex);
            if (keyIsHit(ALTKEY))
            {
                mrHeading = mrPitch = mrBank = 0.0f;
            }
        }
        mouseCursorHide();
    }
    mrTestHPBMode = FALSE;
#endif
#if PIE_VISUALIZE_VERTICAL
    if (pieOnScreen)
    {
        static bool pieFlashFlag = FALSE;
        pieFlashFlag ^= TRUE;
        if (pieFlashFlag)
        {
            primLine2((sdword)pieScreen0.x, 479 - (sdword)pieScreen0.y, (sdword)pieScreen1.x, 479 - (sdword)pieScreen1.y, colWhite);
        }
    }
#endif //PIE_VISUALIZE_VERTICAL

    y = 0;

#if VISIBLE_POLYS
    if (selSelected.numShips == 1)
    {
        extern sdword visiblePoly, visibleSegment, visibleDirection, visibleWhich;
        extern real32 visibleUV[3][2];
        if (counter > 0)
        {
            counter--;
        }
        else
        {
            counter = 4;
            if (keyIsHit(NINEKEY))
            {
                //dec
                if (visiblePoly >= 0)
                {
                    if (keyIsHit(SHIFTKEY))
                    {
                        visiblePoly -= 10;
                        if (visiblePoly < -1)
                        {
                            visiblePoly = -1;
                        }
                    }
                    else
                    {
                        visiblePoly--;
                    }
                }
            }
            else if (keyIsHit(ZEROKEY))
            {
                //inc
                if (keyIsHit(SHIFTKEY))
                {
                    visiblePoly += 10;
                }
                else
                {
                    visiblePoly++;
                }
            }
            else if (keyIsHit(ONEKEY))
            {
                visibleDirection = -1;
                visibleWhich = 0;
            }
            else if (keyIsHit(TWOKEY))
            {
                visibleDirection = +1;
                visibleWhich = 0;
            }
            else if (keyIsHit(THREEKEY))
            {
                visibleDirection = -1;
                visibleWhich = 1;
            }
            else if (keyIsHit(FOURKEY))
            {
                visibleDirection = +1;
                visibleWhich = 1;
            }
            else if (keyIsHit(FIVEKEY))
            {
                visibleSegment++;
                if (visibleSegment > 2)
                {
                    visibleSegment = 0;
                }
            }
            else if (keyIsHit(SEVENKEY))
            {
                extern bool g_Output;
                g_Output = TRUE;
            }
        }
        fontPrintf(0, 0, colWhite, "%d poly %d", visibleSegment, visiblePoly);
        c = (visibleSegment == 0) ? colRGB(255,0,0) : colWhite;
        fontPrintf(0, 16, c, "%0.4f %0.4f", visibleUV[0][0], visibleUV[0][1]);
        c = (visibleSegment == 1) ? colRGB(0,255,0) : colWhite;
        fontPrintf(0, 32, c, "%0.4f %0.4f", visibleUV[1][0], visibleUV[1][1]);
        c = (visibleSegment == 2) ? colRGB(0,0,255) : colWhite;
        fontPrintf(0, 48, c, "%0.4f %0.4f", visibleUV[2][0], visibleUV[2][1]);
    }
#endif

#if RND_FRAME_RATE
    if (rndDisplayFrameRate)
    {                                                   //display frame rate
        cmDeterministicBuildDisplay();
        if (rndFrameCountMin == SDWORD_Max)
        {
            fontPrintf(0, 0, colReddish, "%.2f <= %.2f <= %.2f",
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds);
        }
        else
        {
            fontPrintf(0, 0, colWhite, "%.2f <= %.2f <= %.2f",
                       (real32)(rndFrameCountMin) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameCountMax) / RND_FrameRateSeconds);
        }
        y += 10;

        fontPrintf(0,y += 20,colRGB(255,255,0),"Channels in use:%d Guns:%d Ships:%d SFX:%d UI:%d",channelsinuse,numchans[0],numchans[1],numchans[2],numchans[3]);

    }
#endif//RND_FRAME_RATE

#ifdef COLLISION_CHECK_STATS
    if (rndDisplayCollStats)
    {
        fontPrintf(0,y += 20,colWhite, "Ship:%d  %d  Resources:%d  %d  Derelicts:%d %d  Total:%d  %d",shipshipwalks,shipshipchecks,
                   shipresourcewalks,shipresourcechecks,shipderelictwalks,shipderelictchecks,
                   shipshipwalks+shipresourcewalks+shipderelictwalks,shipshipchecks+shipresourcechecks+shipderelictchecks);

        fontPrintf(0,y += 20,colWhite, "Bullet:%d  %d  Missile:%d  %d  Mine:%d  %d  Total:%d  %d",
                   bulletwalks,bulletchecks,missilewalks,missilechecks,minewalks,minechecks,
                   bulletwalks+missilewalks+minewalks,bulletchecks+missilechecks+minechecks);

        fontPrintf(0,y += 20,colWhite, "Ships Avoiding Stuff:%d   Avoided Walks:%d   Avoided Checks:%d",
                   shipsavoidingstuff,shipsavoidedwalks,shipsavoidedchecks);
#ifdef BOB_STATS
        if (bobStats.statsValid)
        {
            sdword timeDuration = bobStats.timeDuration / (1000L);
            fontPrintf(0,y += 20,colWhite, "Blobtype %d Time: %d numPasses:%d numWalks: %d",
                       bobStats.subBlobs, timeDuration, bobStats.numPasses, bobStats.numWalks);
            fontPrintf(0,y += 20,colWhite,"numChecks:%d trivialRejects:%d InitialBlobs: %d FinalBlobs: %d",
                       bobStats.numChecks, bobStats.trivialRejects, bobStats.initialBlobs, bobStats.finalBlobs);
        }
#endif
#ifdef AISHIP_STATS
        aishipStatsPrint(&y);
#endif
    }
#endif
#if RND_POLY_STATS
    if (rndDisplayPolyStats)
    {                                                   //display frame rate
        fontPrint(0, MAIN_WindowHeight - fontHeight(rndPolyStatsString),
                  rndPolyStatsColor, rndPolyStatsString);
        rndPolyStatFrameCounter++;
#if MESH_MATERIAL_STATS
        fontPrint(MAIN_WindowWidth - fontWidth(meshMaterialStatsString),
                  MAIN_WindowHeight - fontHeight(rndPolyStatsString),
                  rndPolyStatsColor, meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
    }
#endif//RND_POLY_STATS
#if RND_XYZ
    if (rndXYZPrint && selSelected.numShips == 1)
    {
        static char string[256];
        if (selSelecting.numTargets == 1)
        {
            sprintf(string, "deltaXYZ = (%.2f, %.2f, %.2f)", selSelecting.TargetPtr[0]->posinfo.position.x - selSelected.ShipPtr[0]->posinfo.position.x, selSelecting.TargetPtr[0]->posinfo.position.y - selSelected.ShipPtr[0]->posinfo.position.y, selSelecting.TargetPtr[0]->posinfo.position.z - selSelected.ShipPtr[0]->posinfo.position.z);
        }
        else
        {
            sprintf(string, "XYZ = (%.2f, %.2f, %.2f)", selSelected.ShipPtr[0]->posinfo.position.x, selSelected.ShipPtr[0]->posinfo.position.y, selSelected.ShipPtr[0]->posinfo.position.z);
        }
        fontPrint(MAIN_WindowWidth - fontWidth(string), 0, colWhite, string);
    }
#endif
#if NIS_PRINT_INFO
    if (nisPrintInfo)
    {
        fontPrintf(MAIN_WindowWidth - fontWidth(nisInfoString) - 10,
                   MAIN_WindowHeight - fontHeight(nisInfoString) - 1,
                   colWhite, nisInfoString);
    }
#endif

#if 0
    if (selSelected.numShips == 1)
    {
        real32 overlap = rndComputeOverlap(selSelected.ShipPtr[0], 2.0f);
        fontPrintf(0, 0, colWhite, "overlap %0.3f     ", overlap);
    }
#endif
#if RND_SCALECAP_TWEAK
    if (RND_CAPSCALECAP_STATS)
    {
        if (selSelected.numShips == 1)
        {
            static Ship* ship;
            static shiptrail* trail;

            ship = selSelected.ShipPtr[0];
            trail = ship->trail[0];

            if (trail != NULL)
            {
                if (keyIsHit(RND_CapScaleCapKeyDec))
                {
                    //decrement
                    if (trail->scalecap == -1.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                    trail->scalecap -= rndCapScaleCapSlopeDelta;
                    if (trail->scalecap < 0.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                }
                if (keyIsHit(RND_CapScaleCapKeyInc))
                {
                    //increment
                    if (trail->scalecap == -1.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                    trail->scalecap += rndCapScaleCapSlopeDelta;
                }
            }
            fontPrint(0, MAIN_WindowHeight - fontHeight(rndCapScaleCapStatsString),
                      colWhite, rndCapScaleCapStatsString);
        }
    }
#endif
}
#endif

/*-----------------------------------------------------------------------------
    Name        : rndDrawScissorBars
    Description : Draw the scissor window bars top and bottom for when
                    scissoring is only partial or not supported.
    Inputs      : scissorEnabled - does the renderer support scissor windows?
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndDrawScissorBars(bool scissorEnabled)
{
    sdword y;
    real32 GLy, oneGLy;

    if (nisScissorFade == 0.0f && !scissorEnabled)
    {
        y = NIS_LetterHeight;
        GLy = primScreenToGLY(y);
        oneGLy = primScreenToGLY(MAIN_WindowHeight - y);
        glColor3f(0.0f, 0.0f, 0.0f);
    }
    else
    {
        y = (sdword)((real32)NIS_LetterHeight * nisScissorFade);
        GLy = primScreenToGLY(y);
        oneGLy = primScreenToGLY(MAIN_WindowHeight - y);
        dbgAssert(!nisFullyScissored || !scissorEnabled);
        dbgAssert((nisScissorFade > 0.0f && nisScissorFade <= 1.0f) || !scissorEnabled);
        glColor3f(MR_LetterboxGrey);
        glBegin(GL_QUADS);
        //top grey part
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(0));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(0));
        glVertex2f(primScreenToGLX(-1), GLy);
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), GLy);
        //bottom scissor part
        glVertex2f(primScreenToGLX(-1), oneGLy);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), oneGLy);
        glEnd();

        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, nisScissorFade);
    }
    glBegin(GL_QUADS);
    //top scissor part
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(0));
    glVertex2f(primScreenToGLX(-1), GLy);
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), GLy);
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(0));
    //bottom scissor part
    glVertex2f(primScreenToGLX(-1), oneGLy);
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight));
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight));
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), oneGLy);
    glEnd();
    glDisable(GL_BLEND);
}

/*-----------------------------------------------------------------------------
    Name        : rndRenderTask
    Description : Main render task
    Inputs      : void
    Outputs     : Clears frame buffer, renders regions, draws mouse, swaps buffers.
    Return      : void
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndRenderTask(void)
{
    static bool shouldSwap;
    static sdword index;
#ifdef PROFILE_TIMERS
    static sdword y;
#endif
    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);
        primErrorMessagePrint();

        speechEventUpdate();
        subTitlesUpdate();

#if RND_GL_STATE_DEBUG
        if (keyIsHit(GKEY) && keyIsStuck(LKEY))
        {                                                   //if starting a new round of state saving
            static FILE *f;
            keyClearSticky(LKEY);
            rndGLStateSaving = TRUE;
            if (rndGLStateLogIndex < 10)
            {
                sprintf(rndGLStateLogFileName, "gl0%d.state", rndGLStateLogIndex);
            }
            else
            {
                sprintf(rndGLStateLogFileName, "gl%d.state", rndGLStateLogIndex);
            }
            rndGLStateLogIndex++;
            f = fopen(rndGLStateLogFileName, "wt");         //open the file to clean it out
            if (f != NULL)
            {
                fclose(f);                                  //close the file
            }
        }
#endif
        glColor3ub(colRed(RND_StarColor), colGreen(RND_StarColor), colBlue(RND_StarColor));
        shouldSwap = feShouldSaveMouseCursor();
        if (shouldSwap)
        {
            if (RGL)
            {
                rglFeature(RGL_SAVEBUFFER_ON);
                if (mainRasterSkip)
                {
                    rglFeature(RGL_NOSKIP_RASTER);
                }
            }
        }
        else
        {
            if (RGL)
            {
                rglFeature(RGL_SAVEBUFFER_OFF);
            }
            if (binkDonePlaying)
            {
                if (RGL)
                {
                    if (mainRasterSkip && gameIsRunning)
                    {
                        rglFeature(RGL_SKIP_RASTER);
                    }
                    else
                    {
                        rglFeature(RGL_NOSKIP_RASTER);
                    }
                }
                if (lmActive)
                {
                    rndClearToBlack();
                    glClear(GL_DEPTH_BUFFER_BIT);
                }
                else
                {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the buffers
                }
            }
        }

        rndScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);   //can we do scissoring?
        primErrorMessagePrint();
        //default rendering scheme is primitives on. any
        //functions which want it off should set it back on when done
        regFunctionsDraw();                                 //render all regions
        primErrorMessagePrint();
        rndFrameCount++;                                    //update frame count
        //draw partial scissor window, if applicable
        if (nisScissorFade != 0.0f || (nisFullyScissored && !rndScissorEnabled))
        {
            rndDrawScissorBars(rndScissorEnabled);
        }
        glDisable(GL_SCISSOR_TEST);                         //in case the scene was rendered with scissoring, turn it off properlu.


        /* need to update audio event layer */
        soundEventUpdate();

        //draw the subtitle text.  It's done here because it has to draw over everything
        for (index = 0; index < SUB_NumberRegions; index++)
        {
            if ((!tutPointersDrawnThisFrame) && index == STR_LetterboxBar)
            {                                           //if this is the subtitle region
                tutDrawTextPointers(&subRegion[STR_LetterboxBar].rect);//draw any active pointers there may be
            }
            if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
            {
                if (index == STR_NIS)
                {
                    subTimeElapsed = &thisNisPlaying->timeElapsed;
                }
                else
                {
                    subTimeElapsed = &universe.totaltimeelapsed;
                }
                subTitlesDraw(&subRegion[index]);
            }
        }

        rndDrawOnScreenSyncStatus();        // always print sync status, even in HW_Release mode

#if RND_VERBOSE_LEVEL >= 1
        rndDrawOnScreenDebugInfo();                         //draw a bunch of debugging crapola
#endif
#ifdef PROFILE_TIMERS
        y = 300;
        if (rndDisplayCollStats) profTimerStatsPrint(&y);        // compiled out automatically if PROFILE_TIMERS not defined
#endif
        // set the cursor type, reset the variables then draw the mouse cursor
        mouseSelectCursorSetting();
        mouseSetCursorSetting();
        shouldSwap = feShouldSaveMouseCursor();
        if (shouldSwap)
        {
            mouseStoreCursorUnder();
        }
        if (!glcActive())
        {
            mouseDraw();                                        //draw mouse atop everything
        }

        if (universePause)
        {
#if 0
            if (singlePlayerGame)
            {
                static sdword intelliCount;
                static Node*  intelliNode;

                //count the number of popped up popups
                intelliCount = 0;
                intelliNode  = poFleetIntelligence.head;
                while (intelliNode != NULL)
                {
                    FleetIntelligence* inode = (FleetIntelligence*)listGetStructOfNode(intelliNode);
                    if (inode->showOnce)
                    {
                        intelliCount++;
                    }
                    intelliNode = intelliNode->next;
                }
                //don't display plug if a popup has paused the game
                if (intelliCount == 0)
                {
                    rndShamelessPlug();
                }
            }
            else
#endif
            {
                rndShamelessPlug();
            }
        }

        //take a screenshot or sequence thereof
#if SS_SCREENSHOTS
        if (keyIsStuck(SCROLLKEY))
        {
            keyClearSticky(SCROLLKEY);
#if MAIN_Password
            if (mainScreenShotsEnabled)
#endif //MAIN_Password
            {
                rndTakeScreenshot = TRUE;
            }
        }
        else if (keyIsStuck(PAUSEKEY))
        {
            keyClearSticky(PAUSEKEY);
            if (RGL)
            {
#if SS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nMulti-shot end.");
#endif
                rglFeature(RGL_MULTISHOT_END);
            }
        }
        if (rndTakeScreenshot)
        {
            static ubyte* buf;

            rndTakeScreenshot = FALSE;
            //buf = (ubyte*)malloc(3*MAIN_WindowWidth*MAIN_WindowHeight);
            buf = (void *)VirtualAlloc(NULL, 3*MAIN_WindowWidth*MAIN_WindowHeight, MEM_COMMIT, PAGE_READWRITE);
            if (buf != NULL)
            {
                BOOL result;
                glReadPixels(0, 0, MAIN_WindowWidth, MAIN_WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, buf);
                ssSaveScreenshot(buf);
                //free(buf);
                result = VirtualFree(buf, 0, MEM_RELEASE);
                dbgAssert(result);
            }
        }
#endif //SS_SCREENSHOTS

        if (rndFillCounter)
        {
            static rectangle r;
            r.x0 = 0;
            r.y0 = 0;
            r.x1 = MAIN_WindowWidth - 1;
            r.y1 = MAIN_WindowHeight - 1;
            primRectSolid2(&r, rndFillColour);
            rndFillCounter--;
        }
#if 0
#if defined (Downloadable) || defined(DLPublicBeta)
        ;
#else
        {
            extern bool mainPlayAVIs;

            if (mainPlayAVIs)
            {
                mainPlayAVIs = FALSE;
                animSmackPlay(-1, (sdword)"Movies\\sierra.smk");
                animSmackPlay(-1, (sdword)"Movies\\relicintro.smk");
            }
        }
#endif
#endif
        //demo-specific update code
        if (demDemoPlaying && gameIsRunning)
        {                                                   //if a demo is playing and we're in the actual game
            if (rndMainViewRender != rndMainViewRenderFunction)
            {                                               //this was a fake render; don't do a swap or flush
                goto afterTheSwap;
            }
        }
        if (mainReinitRenderer > 0)
        {
            mainReinitRenderer--;
            if (mainReinitRenderer == 0)
            {
                mainReinitRGL();
            }
        }
        else
        {
            if (animaticJustPlayed > 0)
            {
                animaticJustPlayed--;
            }
            else if (!feDontFlush)
            {
                if (glCapFeatureExists(GL_SWAPFRIENDLY))
                {
                    if (glcActive())
                    {
                        if (RGLtype == SWtype)
                        {
                            (void)glcActivate(FALSE);
                        }
                        else
                        {
                            glcPageFlip(FALSE);
                        }
                    }
                }
                rndFlush();
            }
            feDontFlush = FALSE;
        }
        primErrorMessagePrint();

        if (shouldSwap)
        {
            mouseRestoreCursorUnder();
        }
        primErrorMessagePrint();
afterTheSwap:

#if RND_GL_STATE_DEBUG
        rndGLStateSaving = FALSE;                           //done saving the file for now
#endif //RND_GL_STATE_DEBUG
        tutPointersDrawnThisFrame = FALSE;
        taskStackRestoreCond();
        taskYield(0);                                       //hold off to next frame
    }
    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : rndLightingEnable
    Description : Enable or disable back-face culling
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : just calls glEnable/glDisable
    Return      : void
----------------------------------------------------------------------------*/
sdword rndLightingEnable(sdword bEnable)
{
    sdword oldStatus = rndLightingEnabled;

    if (bEnable)
    {
        if (!rndLightingEnabled)
        {
            glEnable(GL_LIGHTING);
            rndLightingEnabled = TRUE;
        }
    }
    else
    {
        if (rndLightingEnabled)
        {
            glDisable(GL_LIGHTING);
            rndLightingEnabled = FALSE;
        }
    }

    return oldStatus;
}

/*-----------------------------------------------------------------------------
    Name        : rndBackFaceCullEnable
    Description : Enable or disable back-face culling
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : just calls glEnable/glDisable
    Return      : void
----------------------------------------------------------------------------*/
void rndBackFaceCullEnable(sdword bEnable)
{
    if (bEnable)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndTextureEnable
    Description : Enable or disable texturing
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : toggles rndTextureEnabled
    Return      : previous enabled status
----------------------------------------------------------------------------*/
sdword rndTextureEnable(sdword bEnable)
{
    sdword enabled = rndTextureEnabled;
    if (bEnable)
    {
        if (!rndTextureEnabled)
        {
            glEnable(GL_TEXTURE_2D);
            rndTextureEnabled = TRUE;
        }
    }
    else
    {
        if (rndTextureEnabled)
        {
            glDisable(GL_TEXTURE_2D);
            rndTextureEnabled = FALSE;
        }
    }
    return(enabled);
}

/*-----------------------------------------------------------------------------
    Name        : rndTextureEnvironment
    Description : changes the current texture environment
    Inputs      : mode - one of RTE_Modulate, RTE_Replace, RTE_Decal, RTE_Blend
    Outputs     : changes the current texture environment mode
    Return      : old mode
----------------------------------------------------------------------------*/
sdword rndTextureModeTable[] =
{
    GL_MODULATE, GL_REPLACE, GL_DECAL, GL_BLEND
};
sdword rndTextureEnvironment(sdword mode)
{
    sdword oldMode = rndTextureEnviron;
    if (rndTextureEnviron != mode)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, rndTextureModeTable[mode]);
        rndTextureEnviron = mode;
    }
    return(oldMode);
}

/*-----------------------------------------------------------------------------
    Name        : rndPerspectiveCorrection
    Description : provides a hint to the GL as to whether to perspectively correct textures
    Inputs      : bEnable - TRUE enables (NICEST), FALSE disables (FASTEST)
    Outputs     : may toggle rndPerspectiveCorrect
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndPerspectiveCorrection(sdword bEnable)
{
    sdword oldStatus = rndPerspectiveCorrect;
#if USE_RND_HINT
    if (rndHint > 0)
    {
        if (rndHint == 1)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
            rndPerspectiveCorrect = TRUE;
        }
        else
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            rndPerspectiveCorrect = FALSE;
        }
        return(oldStatus);
    }
#endif
    if (mainNoPerspective)
    {
        if (RGL) glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        return(oldStatus);
    }
    if (bEnable)
    {
        if (!rndPerspectiveCorrect)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
            rndPerspectiveCorrect = TRUE;
        }
    }
    else
    {
        if (rndPerspectiveCorrect)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            rndPerspectiveCorrect = FALSE;
        }
    }
    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndNormalizeEnable
    Description : enables or disables normalization
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : toggles rndNormalization
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndNormalizeEnable(sdword bEnable)
{
    sdword oldStatus = rndNormalization;
    if (bEnable)
    {
        glEnable(GL_NORMALIZE);
        rndNormalization = TRUE;
    }
    else
    {
        glDisable(GL_NORMALIZE);
        rndNormalization = FALSE;
    }
    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndHintInc
    Description : changes the glHint mode of operation
    Inputs      :
    Outputs     : alters rndHint
    Return      :
----------------------------------------------------------------------------*/
void rndHintInc()
{
#if USE_RND_HINT
    rndHint++;
    if (rndHint >= 3) rndHint = 0;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : rndAdditiveBlends
    Description : enables or disables additive blending mode
    Inputs      : bAdditive - TRUE or FALSE
    Outputs     :
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndAdditiveBlends(sdword bAdditive)
{
    sdword oldStatus;
    extern udword gDevcaps2;

    oldStatus = rndAdditiveBlending;
    if (bAdditive != rndAdditiveBlending)
    {
        rndAdditiveBlending = bAdditive;
        if (bAdditive)
        {
            if (bitTest(gDevcaps2, DEVSTAT2_NO_ADDITIVE))
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
        }
        else
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndMaterialfv
    Description : set GL material params
    Inputs      : face - GL_FRONT or GL_FRONT_AND_BACK
                  pname - GL_AMBIENT or GL_DIFFUSE
                  params - RGBA [0..1]
    Outputs     :
    Return      : TRUE if material was updated, FALSE if same as last
----------------------------------------------------------------------------*/
#define V4_EQUAL(A, B) (A[0] != B[0] || A[1] != B[1] || A[2] != B[2] || A[3] != B[3]) ? FALSE : TRUE
#define V4_SET(D, S) \
    { \
        D[0] = S[0]; \
        D[1] = S[1]; \
        D[2] = S[2]; \
        D[3] = S[3]; \
    }
sdword rndMaterialfv(sdword face, sdword pname, real32* params)
{
    static real32 ambient[2][4] = {{-1.0f, -1.0f, -1.0f, -1.0f},{-1.0f, -1.0f, -1.0f, -1.0f}};
    static real32 diffuse[2][4] = {{-1.0f, -1.0f, -1.0f, -1.0f},{-1.0f, -1.0f, -1.0f, -1.0f}};

    if (face == -1)
    {
        real32 reset[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
        V4_SET(ambient[0], reset);
        V4_SET(ambient[1], reset);
        V4_SET(diffuse[0], reset);
        V4_SET(diffuse[1], reset);
        return TRUE;
    }

    if (face == GL_FRONT)
    {
        if (pname == GL_AMBIENT)
        {
            if (!V4_EQUAL(ambient[0], params))
            {
                V4_SET(ambient[0], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
        else
        {
            if (!V4_EQUAL(diffuse[0], params))
            {
                V4_SET(diffuse[0], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
    }
    else
    {
        if (pname == GL_AMBIENT)
        {
            if (!V4_EQUAL(ambient[0], params) || !V4_EQUAL(ambient[1], params))
            {
                V4_SET(ambient[0], params);
                V4_SET(ambient[1], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
        else
        {
            if (!V4_EQUAL(diffuse[0], params) || !V4_EQUAL(ambient[1], params))
            {
                V4_SET(diffuse[0], params);
                V4_SET(diffuse[1], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
    }

    return FALSE;
}


#define vcSub(a,b,c) \
    (a)##->x = (b)##->x - (c)##->x; \
    (a)##->y = (b)##->y - (c)##->y; \
    (a)##->z = (b)##->z - (c)##->z

#define vcCrossProduct(result,a,b) \
    (result)##->x = ((a)##->y * (b)##->z) - ((a)##->z * (b)##->y); \
    (result)##->y = ((a)##->z * (b)##->x) - ((a)##->x * (b)##->z); \
    (result)##->z = ((a)##->x * (b)##->y) - ((a)##->y * (b)##->x)

#define vcAddToScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x += ((vec)##->x * (k));       \
    (dstvec)##->y += ((vec)##->y * (k));       \
    (dstvec)##->z += ((vec)##->z * (k))

#define vcAddToScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x += ((vec)##->x * (k));       \
    (dstvec)##->y += ((vec)##->y * (k));       \
    (dstvec)##->z += ((vec)##->z * (k))

#define vcScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x = (vec)##->x * (k);       \
    (dstvec)##->y = (vec)##->y * (k);       \
    (dstvec)##->z = (vec)##->z * (k)



#define vcDotProduct(a,b) \
    ( ((a)##->x * (b)##->x) + ((a)##->y * (b)##->y) + ((a)##->z * (b)##->z) )


/*-----------------------------------------------------------------------------
    Name        : rndEnvironmentMap
    Description : calculates parameters for rendering a reflective poly
    Inputs      : vector camera : location of camera
                  vectors A,B,C : vertices of poly


    Outputs     : vectors U,V,W : corners on texture map (.x and .y only!)
    Return      :
----------------------------------------------------------------------------*/

void rndEnvironmentMap(vector* camera, vector* A, vector* B, vector* C,
                       vector* U, vector* V, vector* W)
{
    vector v1,v2,normal;
    real32 length;

    //get normal of triangle
    vcSub(&v1,B,A);
    vcSub(&v2,C,A);
    vcCrossProduct(&normal,&v1,&v2);
    vecNormalizeToLength(&normal, (real32)1.0f);

    //vertex A:

    vcSub(&v1,A,camera);        //v1 is incoming ray
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    //v1 is now reflected ray
    U->x = atan(v1.y/v1.x);                             //azimuth
    U->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex B:

    vcSub(&v1,B,camera);
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    V->x = atan(v1.y/v1.x);                             //azimuth
    V->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex C:

    vcSub(&v1,C,camera);
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    W->x = atan(v1.y/v1.x);                             //azimuth
    W->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;

}

/*-----------------------------------------------------------------------------
    Name        : rndEnvironmentMapConvex
    Description : calculates parameters for rendering a reflective poly with
                    a convex or concave surface
    Inputs      : vector camera : location of camera
                  vectors A,B,C : vertices of poly
                  convex: curvature of surface
                        convex < 1 : concave
                        convex = 1 : flat
                        convex > 1 : convex


    Outputs     : vectors U,V,W : corners on texture map (.x and .y only!)
    Return      :
----------------------------------------------------------------------------*/

void rndEnvironmentMapConvex(vector* camera, vector* A, vector* B, vector* C,
                       real32 convex, vector* U, vector* V, vector* W)
{
    vector v1,v2,normal;
    real32 length;

    //get normal of triangle
    vcSub(&v1,B,A);
    vcSub(&v2,C,A);
    vcCrossProduct(&normal,&v1,&v2);
    vecNormalizeToLength(&normal, (real32)1.0f);

    //vertex A:

    vcSub(&v1,A,camera);        //v1 is incoming ray
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    //v1 is now reflected ray
    U->x = atan(v1.y/v1.x);                             //azimuth
    U->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex B:

    vcSub(&v1,B,camera);
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,2*length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    V->x = atan(v1.y/v1.x);                             //azimuth
    V->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex C:

    vcSub(&v1,C,camera);
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,2*length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    W->x = atan(v1.y/v1.x);                             //azimuth
    W->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;



}

/*-----------------------------------------------------------------------------
    Name        : rndSetScreenFill
    Description : set a flag to fill the screen entirely with a rectangle of
                  specified colour immediately before the next Flush
    Inputs      : count - number of frames for fill to occur
                  c - color to fill screen with
    Outputs     : globals are set
    Return      :
-----------------------------------------------------------------------------*/
void rndSetScreenFill(sdword count, color c)
{
    rndFillCounter = count;
    rndFillColour = c;
}

/*-----------------------------------------------------------------------------
    Name        : rndSetClearColor
    Description : set the background clear color, if such a thing is supported by
                  underlying GL
    Inputs      : c - the color to set
    Outputs     : GL_COLOR_CLEAR_VALUE is perhaps modified
    Return      :
----------------------------------------------------------------------------*/
void rndSetClearColor(color c)
{
    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        glClearColor(colReal32(colRed(c)),
                     colReal32(colGreen(c)),
                     colReal32(colBlue(c)),
                     1.0f);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndResetGLState
    Description : reset the GL's state after switching renderers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndResetGLState(void)
{
    trClearCurrent();

    rndMaterialfv(-1, 0, NULL);

    if (rndAdditiveBlending)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    else
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    rndAdditiveBlends(rndAdditiveBlending);

    if (rndNormalization)
    {
        glEnable(GL_NORMALIZE);
    }
    else
    {
        glDisable(GL_NORMALIZE);
    }
    rndNormalizeEnable(rndNormalization);

    if (rndPerspectiveCorrect)
    {
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    else
    {
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    }
    rndPerspectiveCorrection(rndPerspectiveCorrect);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, RTE_Modulate);
    rndTextureEnviron = RTE_Modulate;

    if (rndTextureEnabled)
    {
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
    rndTextureEnable(rndTextureEnabled);

    rndBackFaceCullEnable(TRUE);

    if (rndLightingEnabled)
    {
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }
    rndLightingEnable(rndLightingEnabled);

    rndSetClearColor(colBlack);
}

/*-----------------------------------------------------------------------------
    Name        : rndComputeOverlap
    Description : compute amount of overlap of collision spheres wrt ships in renderlist
    Inputs      : ship - ship to test
                  scalar - multiplier for strike craft collision radii
    Outputs     :
    Return      : [0..1] amount of obscuring that's going on
----------------------------------------------------------------------------*/
real32 rndComputeOverlap(Ship* ship, real32 scalar)
{
    Node* objnode;
    SpaceObj* spaceobj;
    ShipStaticInfo* shipStaticInfo;
    Ship* testship;
    vector vobj;
    real32 overlap;
    CollInfo* acoll;
    CollInfo* bcoll;
    rectangle arect, brect, crect;
    real32 aarea, carea;
    real32 ratio;
    real32 radius;

    //looking for our ship
    objnode = universe.RenderList.head;
    while (objnode != NULL)
    {
        testship = (Ship*)listGetStructOfNode(objnode);
        if (testship == ship)
        {
            break;
        }
        objnode = objnode->next;
    }

    if (objnode == NULL)
    {
        //couldn't find it
        return 0.0f;
    }

    //move backwards computing overlap
//    objnode = objnode->prev;
    objnode = objnode->next;
    overlap = 0.0f;
    //make screenspace rectangle
    acoll = &ship->collInfo;
    radius = scalar * acoll->selCircleRadius;
    arect.x0 = primGLToScreenX(acoll->selCircleX - radius);
    arect.x1 = primGLToScreenX(acoll->selCircleX + radius);
    arect.y0 = primGLToScreenY(acoll->selCircleY + radius);
    arect.y1 = primGLToScreenY(acoll->selCircleY - radius);
    aarea = (real32)(2 * (arect.x1 - arect.x0) +
                     2 * (arect.y1 - arect.y0));
    while (objnode != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
        if (spaceobj->objtype == OBJ_ShipType)
        {
            //useful stuff
            testship = (Ship*)spaceobj;
            shipStaticInfo = (ShipStaticInfo*)spaceobj->staticinfo;

            if (ship->staticinfo->shipclass <= CLASS_Frigate)
            {
                if (testship->staticinfo->shipclass > CLASS_Frigate)
                {
                    // don't calculate strikecraft overlapping capital ships
                    goto nextobj;
                }
            }
#if 0
            //not sure what to do about this yet
            if (ship->collInfo.precise != NULL)
            {
                __asm int 3 ;
            }
#endif

            //position in cameraspace
            vecSub(vobj, mrCamera->eyeposition, spaceobj->posinfo.position);

            //make screenspace rectangle
            bcoll = &testship->collInfo;
            radius = scalar * bcoll->selCircleRadius;
            brect.x0 = primGLToScreenX(bcoll->selCircleX - radius);
            brect.x1 = primGLToScreenX(bcoll->selCircleX + radius);
            brect.y0 = primGLToScreenY(bcoll->selCircleY + radius);
            brect.y1 = primGLToScreenY(bcoll->selCircleY - radius);

            //find union of ab
            primRectUnion2(&crect, &arect, &brect);
            if (crect.x0 == crect.x1 || crect.y0 == crect.y1)
            {
                carea = 0.0f;
            }
            else
            {
                carea = (real32)(2 * (crect.x1 - crect.x0) +
                                 2 * (crect.y1 - crect.y0));
            }

            //compute overlap
            ratio = carea / aarea;
            overlap += ratio;
            if (overlap >= 1.0f)
            {
                overlap = 1.0f;
                break;
            }
        }

//        objnode = objnode->prev;
nextobj:
        objnode = objnode->next;
    }

    return overlap;
}

/*-----------------------------------------------------------------------------
    Name        : rndClearToBlack
    Description : clear the screen to black, guaranteed
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndClearToBlack(void)
{
    real32 bgcolor[4];

    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        //get current clearcolor
        glGetFloatv(GL_COLOR_CLEAR_VALUE, (GLfloat*)bgcolor);
        //set clearcolor to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        //reset clearcolor
        glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
    }
    else
    {
        //clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndAllClearToBlack
    Description : clear all buffers to black
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndAllClearToBlack(void)
{
    real32 bgcolor[4];

    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        //get current clearcolor
        glGetFloatv(GL_COLOR_CLEAR_VALUE, (GLfloat*)bgcolor);
        //set clearcolor to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //clear the screen
        rndClear();
        //reset clearcolor
        glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
    }
    else
    {
        //clear the screen
        rndClear();
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndClear
    Description : clear front and back buffers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndClear(void)
{
    sdword i;

    for (i = 0; i < 3; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        rndFlush();
    }

    if (RGLtype == SWtype)
    {
        rglFeature(RGL_SPEEDY);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndFlush
    Description : flush the GL and swap buffers if necessary
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndFlush(void)
{
    glFlush();
    primErrorMessagePrint();
    if (!RGL)
    {
        if (rwglSwapBuffers == NULL || glNT)
        {
            SwapBuffers(hGLDeviceContext);
        }
        else
        {
            rwglSwapBuffers(hGLDeviceContext);
        }
    }
    Sleep(1);
}
