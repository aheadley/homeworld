/*=============================================================================
    Name    : Tracking.c
    Purpose : Functions and data to visually track numbers on-screen.

    Created 7/21/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Tracking.h"

#if NUMBER_TRACKING         //if anything in this module is enabled,
#include <string.h>
#include "main.h"
#include "Debug.h"
#include "Memory.h"
#include "prim2d.h"
#include "font.h"
#include "Key.h"

/*=============================================================================
    Data:
=============================================================================*/
trackvalue trkValue[TRK_NumberTracks];
sdword trkTrackIndex = 0;
bool trkTrackingVisual = FALSE;

struct
{
    char *minusString, *plusString;
    real32 base;
}
trkRangeString[] =
{
    {"-100M",   "+100M",    100000000.0f},
    {"-10M",    "+10M",     10000000.0f},
    {"-1M",     "+1M",      1000000.0f},
    {"-100K",   "+100K",    100000.0f},
    {"-10K",    "+10K",     10000.0f},
    {"-1K",     "+1K",      1000.0f},
    {"-100",    "+100",     100.0f},
    {"-10",     "+10",      10.0f},
    {"-1",      "+1",       1.0f},
    {"-100m",   "+100m",    0.1f},
    {"-10m",    "+10m",     0.01f},
    {"-1m",     "+1m",      0.001f},
    {"-100u",   "+100u",    0.0001f},
    {"-10u",    "+10u",     0.00001f},
    {"-1u",     "+1u",      0.000001f},
    {"-100n",   "+100n",    0.0000001f},
    {"-10n",    "+10n",     0.00000001f},
    {"-1n",     "+1n",      0.000000001f},
    {NULL, NULL, 0.0f}
};


/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : trkTrackValueAddFn
    Description : Add a value to track
    Inputs      : name - name of the value to track - to be displayed on-screen
                  number - pointer number to track
                  timer - pointer to timer number, such as taskTimeElaped
                  c - color to render the bar in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trkTrackValueAddFn(char *name, real32 *number, real32 *timer, color c)
{
    dbgAssert(trkTrackIndex < TRK_NumberTracks - 1);
    trkValue[trkTrackIndex].value = number;
    trkValue[trkTrackIndex].lastValue = *number;
    trkValue[trkTrackIndex].name = memStringDupeNV(name);
    trkValue[trkTrackIndex].timer = timer;
    trkValue[trkTrackIndex].lastTime = *timer;
    trkValue[trkTrackIndex].c = c;
    trkTrackIndex++;
}

/*-----------------------------------------------------------------------------
    Name        : trkTrackValueRemoveFn
    Description : Remove a track value by name
    Inputs      : name - name of track value to remove.  All values of matching
                    name will be removed.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trkTrackValueRemoveFn(char *name)
{
    sdword index, j;

    for (index = 0; index < trkTrackIndex; index++)
    {
        if (!strcmp(name, trkValue[index].name))
        {
            for (j = index + 1; j < trkTrackIndex; j++)
            {
                trkValue[j - 1] = trkValue[j];
            }
            index--;
            trkTrackIndex--;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trkTrackValueRemoveAllFn
    Description : Remove all tracking values
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trkTrackValueRemoveAllFn(void)
{
    sdword index;

    for (index = 0; index < trkTrackIndex; index++)
    {
        memFree(trkValue[index].name);
    }
    trkTrackIndex = 0;
}

/*-----------------------------------------------------------------------------
    Name        : trkTrackValuesDisplayFn
    Description : Renders all the track values
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trkTrackValuesDisplayFn(void)
{
    sdword index, range;
    sdword x, y, xMin, xMax, xMed, width, height;
    real32 delta, timeElapsed;
    rectangle rect;

    if (keyIsStuck(TRK_ToggleKey))
    {
        keyClearSticky(TRK_ToggleKey);
        trkTrackingVisual ^= TRUE;
    }

    if (!trkTrackingVisual || trkTrackIndex == 0)
    {
        return;
    }

    /*
    if (timeElapsed == 0.0f)
    {
        return;
    }
    */

    height = fontHeight(" ") + 1;
    y = (MAIN_WindowHeight - (height * trkTrackIndex)) / 2;
    width = fontWidth("+100M") + 1;
    x = MAIN_WindowWidth - width * 2 - TRK_TrackWidth;
    xMin = x + width;
    xMax = xMin + TRK_TrackWidth;
    xMed = (xMin + xMax) / 2;
    for (index = 0; index < trkTrackIndex; index++, y += height)
    {
        timeElapsed = *trkValue[index].timer - trkValue[index].lastTime;
        trkValue[index].lastTime = *trkValue[index].timer;
        if (timeElapsed == 0.0f)
        {
            continue;
        }
        //print the name of the value
        fontPrint(x - fontWidth(trkValue[index].name), y, trkValue[index].c, trkValue[index].name);
        delta = ABS(*trkValue[index].value - trkValue[index].lastValue) / timeElapsed;//see how much it has changed
        if (delta == 0.0f)
        {                                                   //don't do anything else if it has not changed
            continue;
        }
        //find what range to print in
        if (delta / trkRangeString[0].base > 10.0f)
        {                                                   //if it's bigger than the biggest range
            continue;
        }
        for (range = 0; trkRangeString[range].minusString; range++)
        {
            if (delta / trkRangeString[range].base >= 1.0f)
            {                                               //if this is the right range
                //get the real delta and bias it against the base
                delta = (trkValue[index].lastValue - *trkValue[index].value) / timeElapsed / trkRangeString[range].base;
                rect.y0 = y;
                rect.y1 = y + height - 1;
                if (delta < 0.0f)
                {
                    rect.x0 = xMed + (sdword)(delta * (real32)TRK_TrackWidth / 20.0f);
                    rect.x1 = xMed;
                }
                else
                {
                    rect.x0 = xMed;
                    rect.x1 = xMed + (sdword)(delta * (real32)TRK_TrackWidth / 20.0f);
                }
                dbgAssert(rect.x0 != rect.x1);
                primRectSolid2(&rect, trkValue[index].c);
                fontPrint(xMin - fontWidth(trkRangeString[range].minusString) - 1, y, colWhite, trkRangeString[range].minusString);
                fontPrint(xMax + 1, y, colWhite, trkRangeString[range].plusString);
                break;
            }
        }
        trkValue[index].lastValue = *trkValue[index].value;
    }
}
#endif //NUMBER_TRACKING

