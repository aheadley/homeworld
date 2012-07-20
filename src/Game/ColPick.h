/*=============================================================================
    Name    : Colpick.h
    Purpose : Definitions for the color picker

    Created 10/12/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COLPICK_H
#define ___COLPICK_H

#include "Types.h"
#include "color.h"
#include "prim2d.h"
#include "FEFlow.h"
#include "texreg.h"

/*=============================================================================
    Switches:
=============================================================================*/

#define CP_TEST                     0           //test this module
#define CP_VISUALIZE_CURVES         0           //display the hue-specific scalar curves in the color picker

#ifndef HW_Release

#define CP_ERROR_CHECKING           1           //general error checking
#define CP_VERBOSE_LEVEL            1           //control specific output code

#else //HW_Debug

#define CP_ERROR_CHECKING           0           //general error checking
#define CP_VERBOSE_LEVEL            0           //control specific output code

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define CP_NumberMarginX            3
#define CP_FontFile                 "Arial_12.hff"
#define CP_HueSaturationCircleRadius 4
#define CP_HueSaturationCircleSegments 8
#define CP_HueSaturationCircleColor colWhite
#define CP_ValueArrowWidth          16
#define CP_ValueArrowHeight         16
#define CP_ValueArrowColor          colWhite
#define CP_ValueTextureWidth        32
#define CP_ValueTextureHeight       256
#define CP_HueSatTextureWidth       256
#define CP_HueSatTextureHeight      256
#define CPGLC_HueSatTextureWidth    224
#define CPGLC_HueSatTextureHeight   150
#ifdef _WIN32
#define CP_PreviewImageRace1        "ColorPicker\\Race1\\Race1.lif"
#define CP_PreviewImageRace2        "ColorPicker\\Race2\\Race2.lif"
#else
#define CP_PreviewImageRace1        "ColorPicker/Race1/Race1.lif"
#define CP_PreviewImageRace2        "ColorPicker/Race2/Race2.lif"
#endif
#define CP_PreviewWidth             256
#define CP_PreviewHeight            128
#define CP_ScreenName               "Select_colour"
#define CP_DarkestColor0            150
#define CP_DarkestColor1            0
#define CP_NumberPreviousColors     5
//NewMultiGameSelectColour
//NewGameSelectColour
//JoinMultiGameSelectColour

/*=============================================================================
    Type definitions:
=============================================================================*/
//information for coloring a texture
typedef struct
{
    color base;
    color detail;
}
colorinfo;

typedef struct
{
    sdword num;                                 //number of points in curve
    real32 *hue;                                //locations of points in curve
    real32 *val;                                //values of points in curve
}
colpickcurve;

/*=============================================================================
    Data:
=============================================================================*/
extern sdword cpDarkestColor0;
extern sdword cpDarkestColor1;
extern bool   cpColorsPicked;
extern trcolorinfo colPreviousColors[CP_NumberPreviousColors];

/*=============================================================================
    Functions:
=============================================================================*/

void cpStartup(color *base, color *stripe);
void cpShutdown(void);
void cpColorsPick(color *base, color *stripe, ShipRace *race);
void cpSetColorsToModify(color *base, color *stripe);
void cpAcceptColors(void);
void cpTexturesPurge(void);
void cpPreviewImageDelete(void);
void cpColorsUpdate(color base, color stripe);
void cpTeamEffectScalars(real32 *scalar0, real32 *scalar1, color c0, color c1, ShipRace race);
real32 cpScalarFromCurveGet(colpickcurve *curve, real32 hue, real32 sat, real32 val);
void cpColorsReset(color *base, color *stripe);
void cpResetRegions(void);

void cpColorsAddToPreviousList(color base, color stripe);

#endif //___COLPICK_H

