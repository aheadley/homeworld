/*=============================================================================
    Name    : Tactical.h
    Purpose : Definitions for the tactical overlay

    Created 8/8/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___TACTICAL_H
#define ___TACTICAL_H

#include "Types.h"
#include "color.h"
#include "prim2d.h"
#include "utility.h"
#include "ObjTypes.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define TO_ERROR_CHECKING       1               //general error checking
#define TO_VERBOSE_LEVEL        2               //print extra info

#else //HW_Debug

#define TO_ERROR_CHECKING       0               //general error checking
#define TO_VERBOSE_LEVEL        0               //print extra info

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define TO_MinimumScreenSize     4
#define TO_MineMinimumScreenSize 0.5
#define TO_VertexScanFactorX    2.0f
#define TO_VertexScanFactorY    (-2.0f * 640.0f / 480.0f)
#define TO_NumPlayers           8

#define TO_MinFadeSize          0.06f                 //size of ship where fadeout starts (in fractions of the size of screen)
#define TO_MaxFadeSize          0.12f                 //size of ship where fadeout ends
#define TO_FadeStart            TO_MinFadeSize + 0.01f//where the fading actually begins
#define TO_FadeRate             (255.0f / (TO_MaxFadeSize - TO_MinFadeSize))//rate at which the fading happens

#define TO_TextColor            colRGB(64, 64, 255)
#define TO_IconColorFade        2

/*=============================================================================
    Type definitions:
=============================================================================*/
//single point in a TO icon
typedef struct
{
    real32 x, y;
}
toiconpoint;
//structure for a tactical overlay graphic
typedef struct
{
    sdword nPoints;
    toiconpoint loc[1];
}
toicon;

/*=============================================================================
    Data:
=============================================================================*/
extern color toPlayerColor[TO_NumPlayers];
extern toicon *toClassIcon[NUM_CLASSES+1];
extern bool8 toClassUsed[NUM_CLASSES+1][TO_NumPlayers];

/*=============================================================================
    Macros:
=============================================================================*/
#define toIconSize(n)       (sizeof(toicon) + sizeof(toiconpoint) * ((n) - 1))

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown
void toStartup(void);
void toShutdown(void);

//draw overlays for all ships
void toAllShipsDraw(void);

//draw legend for overlay
void toLegendDraw(void);


#endif //___TACTICAL_H

