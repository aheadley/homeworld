/*=============================================================================
    Name    : autolod.c
    Purpose : routines for maintaining a more even framerate

    Created 9/8/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "glcaps.h"
#include "AutoLOD.h"
#include "LOD.h"
#include "StatScript.h"
#include "Options.h"


/*=============================================================================
    Data
=============================================================================*/

static udword alodNumPolys;
static udword alodTargetPolys = 2000;
static udword alodPolyDelta;
static real32 alodIdealScaleFactor;
static real32 alodMaxScaleFactor;
static real32 alodMinScaleFactor;
static real32 alodScaleFactorDelta = 0.008f;
static bool   alodAmPanicking;

static real32 alodFastMinScale = 0.2f;
static real32 alodFastMaxScale = 1.2f;
static udword alodFastTargetPolys = 2000;
static udword alodFastTargetDelta = 350;
static real32 alodSlowMinScale = 0.1f;
static real32 alodSlowMaxScale = 0.7f;
static udword alodSlowTargetPolys = 1400;
static udword alodSlowTargetDelta = 300;

static udword alodActivePolyDelta;

typedef enum
{
    alodOK,
    alodGoingDown,
    alodGoingUp,
    alodGotDown
} alodState_t;

static alodState_t alodState;
static bool alodEnabled;

sdword alodDownDelta = 0;

scriptEntry AutoLODTweaks[] =
{
    makeEntry(alodScaleFactorDelta, scriptSetReal32CB),
    makeEntry(alodFastMinScale, scriptSetReal32CB),
    makeEntry(alodFastMaxScale, scriptSetReal32CB),
    makeEntry(alodFastTargetPolys, scriptSetUdwordCB),
    makeEntry(alodFastTargetDelta, scriptSetUdwordCB),
    makeEntry(alodSlowMinScale, scriptSetReal32CB),
    makeEntry(alodSlowMaxScale, scriptSetReal32CB),
    makeEntry(alodSlowTargetPolys, scriptSetUdwordCB),
    makeEntry(alodSlowTargetDelta, scriptSetUdwordCB),
    endEntry
};


/*=============================================================================
    Code
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : alodStartup
    Description : initialize the autolod module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodStartup(void)
{
    alodIdealScaleFactor = lodScaleFactor;

    alodReset();
}

/*-----------------------------------------------------------------------------
    Name        : alodReset
    Description : reset the autolod module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodReset(void)
{
    lodScaleFactor = alodIdealScaleFactor;

    alodState = alodOK;
    alodAmPanicking = FALSE;
    alodEnabled = TRUE;

    if (RGL && (RGLtype == SWtype))
    {
        alodSetMinMax(alodSlowMinScale, alodSlowMaxScale);
        alodSetTargetPolys(0/*alodSlowTargetPolys*/, alodSlowTargetDelta);
    }
    else
    {
        alodSetMinMax(alodFastMinScale, alodFastMaxScale);
        alodSetTargetPolys(0/*alodFastTargetPolys*/, alodFastTargetDelta);
    }
}

/*-----------------------------------------------------------------------------
    Name        : alodShutdown
    Description : shutdown the autolod module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodShutdown(void)
{
    lodScaleFactor = alodIdealScaleFactor;
    alodState = alodOK;
    alodAmPanicking = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : alodSetMinMax
    Description : set the lodScaleFactor limits
    Inputs      : minScale - minimum scale factor
                  maxScale - maximum scale factor
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodSetMinMax(real32 minScale, real32 maxScale)
{
    alodMinScaleFactor = minScale;
    alodMaxScaleFactor = maxScale;
}

/*-----------------------------------------------------------------------------
    Name        : alodGetMin
    Description : return minScale
    Inputs      :
    Outputs     :
    Return      : minScale
----------------------------------------------------------------------------*/
real32 alodGetMin(void)
{
    return alodMinScaleFactor;
}

/*-----------------------------------------------------------------------------
    Name        : alodGetMax
    Description : return maxScale
    Inputs      :
    Outputs     :
    Return      : maxScale
----------------------------------------------------------------------------*/
real32 alodGetMax(void)
{
    return alodMaxScaleFactor;
}

/*-----------------------------------------------------------------------------
    Name        : alodSetTargetPolys
    Description : set the acceptable onscreen polygon limits
    Inputs      : targetPolys - ideal number of polys onscreen
                  polyDelta - acceptable delta, used to prevent hysteresis
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodSetTargetPolys(udword targetPolys, udword polyDelta)
{
    if (targetPolys != 0)
    {
        alodTargetPolys = targetPolys;
    }
    if (polyDelta != 0)
    {
        alodActivePolyDelta = alodPolyDelta = polyDelta;
    }
}

/*-----------------------------------------------------------------------------
    Name        : alodGetTargetPolys
    Description : get the acceptable onscreen polygon limits
    Inputs      : targetPolys, polyDelta - where poly limits go
    Outputs     : targetPolys, polyDelta are set
    Return      :
----------------------------------------------------------------------------*/
void alodGetTargetPolys(udword* targetPolys, udword* polyDelta)
{
    *targetPolys = alodTargetPolys;
    *polyDelta = alodPolyDelta;
}

/*-----------------------------------------------------------------------------
    Name        : alodSetPolys
    Description : directly set the number of polys rendered
    Inputs      : polys - number of polys
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodSetPolys(udword polys)
{
    alodNumPolys = polys;
}

/*-----------------------------------------------------------------------------
    Name        : alodIncPolys
    Description : add to the number of polys rendered
    Inputs      : polys - number of polys to add to total
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodIncPolys(udword polys)
{
    if (alodEnabled)
    {
        alodNumPolys += polys;
    }
}

/*-----------------------------------------------------------------------------
    Name        : alodEnable
    Description : enable or disable the effect of calling alodIncPolys
    Inputs      : enable - TRUE or FALSE (enable or disable)
    Outputs     : alodEnabled is set to enable
    Return      :
----------------------------------------------------------------------------*/
void alodEnable(bool enable)
{
    alodEnabled = enable;
}

/*-----------------------------------------------------------------------------
    Name        : alodGetPolys
    Description : returns the number of polys rendered
    Inputs      :
    Outputs     : number of polys
    Return      :
----------------------------------------------------------------------------*/
udword alodGetPolys(void)
{
    return alodNumPolys;
}

/*-----------------------------------------------------------------------------
    Name        : alodGetPanic
    Description : returns a boolean indicating whether lodScaleFactor adjustment
                  didn't do the trick by itself
    Inputs      :
    Outputs     : TRUE if panicking, FALSE otherwise
    Return      :
----------------------------------------------------------------------------*/
bool alodGetPanic(void)
{
    return alodAmPanicking;
}

/*-----------------------------------------------------------------------------
    Name        : alodSetPanic
    Description : control whether framerate panic is ensuing
    Inputs      : panic - TRUE or FALSE
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodSetPanic(bool panic)
{
    alodAmPanicking = panic;
}

/*-----------------------------------------------------------------------------
    Name        : alodAdjustScaleFactor
    Description : applies auto lodScaleFactor tweaking
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void alodAdjustScaleFactor(void)
{
    real32 scaleDelta;
    udword delta;

    if (alodTargetPolys == 0 ||
        alodPolyDelta == 0 ||
        opNoLODVal)
    {
        //we're not active
        alodSetPanic(FALSE);
        lodScaleFactor = alodMaxScaleFactor;
        return;
    }

    //distance from ideal poly count
    delta = ABS((sdword)(alodTargetPolys - alodNumPolys));

    alodSetPanic(FALSE);

    if (alodNumPolys > alodTargetPolys)
    {
        //may want to reduce detail

        if (alodState == alodGoingUp)
        {
            //avoid hysteresis
            if (alodNumPolys < (alodTargetPolys + alodPolyDelta))
            {
                return;
            }
        }

        alodState = alodGoingDown;
        alodActivePolyDelta = alodPolyDelta;//delta;

        //adjust scalefactor decrement wrt distance
        scaleDelta = alodScaleFactorDelta;
        if (delta > alodPolyDelta)
        {
            scaleDelta *= 2.0f;
            if (delta > 2*alodPolyDelta)
            {
                scaleDelta *= 2.0f;
            }
        }
        lodScaleFactor -= scaleDelta;   //reduce scalefactor
        if (lodScaleFactor < alodMinScaleFactor)
        {
            //cap scalefactor
            lodScaleFactor = alodMinScaleFactor;
            alodState = alodGotDown;

            if (alodNumPolys > (alodTargetPolys + alodPolyDelta))
            {
                //still too many polys, set PANIC mode
                alodSetPanic(TRUE);
            }
        }
    }
    else
    {
        //FIXME: this ALWAYS overshoots
        //may want to increase detail

        if (delta > alodPolyDelta)
        {
            if ((alodNumPolys < (alodTargetPolys - alodPolyDelta)) ||
                (alodState != alodGotDown))
            {
                alodState = alodGoingUp;

                //adjust scalefactor increment wrt distance
                scaleDelta = alodScaleFactorDelta;
                if (delta > alodPolyDelta)
                {
                    if (delta > 2*alodPolyDelta)
                    {
                        scaleDelta *= 4.0f;
                    }
                    else
                    {
                        scaleDelta *= 2.0f;
                    }
                }
                lodScaleFactor += scaleDelta;   //increase scalefactor
                if (lodScaleFactor > alodMaxScaleFactor)
                {
                    //cap scalefactor
                    lodScaleFactor = alodMaxScaleFactor;
                }
            }
        }
    }
#if LOD_SCALE_DEBUG
    if (lodDebugScaleFactor != 0.0f)
    {
        lodScaleFactor = lodDebugScaleFactor;
    }
#endif
}
