/*=============================================================================
    Name    : Colpick.c
    Purpose : Logic for the color picker

    Created 10/12/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define PREVIEW_IMAGE 1
#define CP_SCALE_HUESAT 0

#ifndef SW_Render
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif
#include <stdio.h>
#include "glinc.h"
#include "Types.h"
#include "color.h"
#include "prim2d.h"
#include "font.h"
#include "FontReg.h"
#include "mouse.h"
#include "Memory.h"
#include "texreg.h"
#include "mainrgn.h"
#include "utility.h"
#include "StatScript.h"
#include "ObjTypes.h"
#include "render.h"
#include "Teams.h"
#include "ShipView.h"
#include "ColPick.h"
#include "glcompat.h"

/*=============================================================================
    Data:
=============================================================================*/
static regionhandle cpRegion[15];

//color picker process callbacks
void cpBaseColor(char *name, featom *atom);
void cpStripeColor(char *name, featom *atom);
void cpDefault(char *name, featom *atom);
void cpPreviousColor(char *name, featom *atom);
void cpNewRace(char *name, featom *atom);
fecallback cpProcessCallbacks[] =
{
    {cpBaseColor,               "CP_BaseColor"},
    {cpStripeColor,             "CP_StripeColor"},
    {cpDefault,                 "CP_Default"},
    {cpPreviousColor,           "CP_Previous0"},
    {cpPreviousColor,           "CP_Previous1"},
    {cpPreviousColor,           "CP_Previous2"},
    {cpPreviousColor,           "CP_Previous3"},
    {cpPreviousColor,           "CP_Previous4"},
    {cpNewRace,                 "CP_RaceOne"},
    {cpNewRace,                 "CP_RaceTwo"},
    {NULL, NULL}
};

//color picker user region draw callbacks
void cpBaseRedDraw(featom *atom, regionhandle region);
void cpBaseGreenDraw(featom *atom, regionhandle region);
void cpBaseBlueDraw(featom *atom, regionhandle region);
void cpBaseHueDraw(featom *atom, regionhandle region);
void cpBaseSaturationDraw(featom *atom, regionhandle region);
void cpBaseValueDraw(featom *atom, regionhandle region);
void cpStripeRedDraw(featom *atom, regionhandle region);
void cpStripeGreenDraw(featom *atom, regionhandle region);
void cpStripeBlueDraw(featom *atom, regionhandle region);
void cpStripeHueDraw(featom *atom, regionhandle region);
void cpStripeSaturationDraw(featom *atom, regionhandle region);
void cpStripeValueDraw(featom *atom, regionhandle region);
void cpBaseColorPreviewDraw(featom *atom, regionhandle region);
void cpStripeColorPreviewDraw(featom *atom, regionhandle region);
void cpPreviewImageDraw(featom *atom, regionhandle region);
void cpHueSaturationDraw(featom *atom, regionhandle region);
void cpValueDraw(featom *atom, regionhandle region);
void cpPreviousDraw(featom *atom, regionhandle region);

void svShipViewRender(featom *atom, regionhandle region);

fedrawcallback cpDrawCallbacks[] =
{
    {cpBaseRedDraw,             "CP_BaseRed"},
    {cpBaseGreenDraw,           "CP_BaseGreen"},
    {cpBaseBlueDraw,            "CP_BaseBlue"},
    {cpBaseHueDraw,             "CP_BaseHue"},
    {cpBaseSaturationDraw,      "CP_BaseSaturation"},
    {cpBaseValueDraw,           "CP_BaseValue"},
    {cpStripeRedDraw,           "CP_StripeRed"},
    {cpStripeGreenDraw,         "CP_StripeGreen"},
    {cpStripeBlueDraw,          "CP_StripeBlue"},
    {cpStripeHueDraw,           "CP_StripeHue"},
    {cpStripeSaturationDraw,    "CP_StripeSaturation"},
    {cpStripeValueDraw,         "CP_StripeValue"},
    {cpBaseColorPreviewDraw,    "CP_BaseColorPreview"},
    {cpStripeColorPreviewDraw,  "CP_StripeColorPreview"},
    {cpPreviewImageDraw,        "CP_PreviewImage"},
    //{svShipViewRender,          "SV_ShipView"},
    {cpHueSaturationDraw,       "CP_HueSaturation"},
    {cpValueDraw,               "CP_Value"},
    {cpPreviousDraw,            "CP_PreviousDraw0"},
    {cpPreviousDraw,            "CP_PreviousDraw1"},
    {cpPreviousDraw,            "CP_PreviousDraw2"},
    {cpPreviousDraw,            "CP_PreviousDraw3"},
    {cpPreviousDraw,            "CP_PreviousDraw4"},
    {NULL, NULL},
};

//current colors under adjustment
sdword cpBaseRed = 255, cpBaseGreen = 99, cpBaseBlue = 3;
sdword cpBaseHue = 35, cpBaseSaturation = 172, cpBaseValue = 205;
sdword cpStripeRed, cpStripeGreen, cpStripeBlue;
sdword cpStripeHue, cpStripeSaturation, cpStripeValue;
color *cpPBaseColor = &utyBaseColor;
color *cpPStripeColor = &utyStripeColor;
//ShipRace *cpPRace;
bool   cpColorsPicked = FALSE;

//region handles for the interactive user regions
regionhandle cpHueSaturationRegion = NULL, cpValueRegion = NULL;

//are we adjusting base or stripe color?
sdword cpColorMode;                             //0 = base, nonzero = stripe

//font used for printf RBGHSV numbers
fonthandle cpValuesFont;

//ship preview image
//layerimage *cpPreviewImage;
lifheader *cpPreviewImage = NULL;
color *cpPreviewTexturePalette = NULL;
udword cpPreviewTexture = TR_InvalidInternalHandle;

//texture handles
udword cpHueSatTexture = TR_InvalidInternalHandle;
sdword cpValueTexture = TR_InvalidInternalHandle;
color* cpHueSatData = NULL;
#if CP_SCALE_HUESAT
color* cpglcHueSatData = NULL;
#endif

void cpHueSatImageDelete(void);

//tweakables
sdword cpDarkestColor0 = CP_DarkestColor0;
sdword cpDarkestColor1 = CP_DarkestColor1;
sdword cpValueArrowWidth = CP_ValueArrowWidth;
sdword cpValueArrowHeight = CP_ValueArrowHeight;
color  cpValueArrowColor = CP_ValueArrowColor;

//script processing table for color picker stuff
void colPickScalarScan(char *directory,char *field,void *dataToFillIn);
static scriptEntry colPickScriptTable[] =
{
    {"baseScalar", colPickScalarScan, NULL},
    {"stripeScalar", colPickScalarScan, (void *)0xffffffff},
    {"cpValueArrowWidth", scriptSetSdwordCB, &cpValueArrowWidth},
    {"cpValueArrowHeight", scriptSetSdwordCB, &cpValueArrowHeight},
    {"cpValueArrowColor", scriptSetRGBCB, &cpValueArrowColor},
    {NULL, NULL, NULL},
};
//hue-specific scalar tables
colpickcurve colPickScalarCurve[NUM_RACES * 2] =//two for each race
{
    {0, NULL, NULL},
    {0, NULL, NULL},
    {0, NULL, NULL},
    {0, NULL, NULL}
};

//previously chosen colors
trcolorinfo colPreviousColors[CP_NumberPreviousColors] =
{
    {colBlack, colBlack},
    {colBlack, colBlack},
    {colBlack, colBlack},
    {colBlack, colBlack},
    {colBlack, colBlack}
};

/*=============================================================================
    Private functions:
=============================================================================*/
//scan in a new
void colPickScalarScan(char *directory,char *field,void *dataToFillIn)
{
    char raceString[32];
    sdword race, index;
    real32 hue, val;
    sdword add = 0;

    if (dataToFillIn != NULL)
    {
        add = 1;
    }
    sscanf(field, "%2s,%f,%f", raceString, &hue, &val);
    race = StrToShipRace(raceString);
    dbgAssert(race >= 0 && race < NUM_RACES);
    dbgAssert(hue >= 0.0f && hue <= 1.0f);
    dbgAssert(val >= 0.0f && val <= 2.0f);
    index = race * 2 + add;
    colPickScalarCurve[index].hue = memRealloc(colPickScalarCurve[index].hue,
            (colPickScalarCurve[index].num + 1) * sizeof(real32), "huescalar0", NonVolatile);
    colPickScalarCurve[index].val = memRealloc(colPickScalarCurve[index].val,
            (colPickScalarCurve[index].num + 1) * sizeof(real32), "valscalar0", NonVolatile);
    if (colPickScalarCurve[index].num > 1)
    {
        dbgAssert(hue > colPickScalarCurve[index].hue[colPickScalarCurve[index].num - 1]);
    }
    colPickScalarCurve[index].hue[colPickScalarCurve[index].num] = hue;
    colPickScalarCurve[index].val[colPickScalarCurve[index].num] = val;
    colPickScalarCurve[index].num++;
}

/*-----------------------------------------------------------------------------
    Name        : cpHueSatImageDelete
    Description : Delete the hue/saturation blend image
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpHueSatImageDelete(void)
{
    if (cpHueSatData != NULL)
    {
        memFree(cpHueSatData);
        cpHueSatData = NULL;
    }
#if CP_SCALE_HUESAT
    if (cpglcHueSatData != NULL)
    {
        memFree(cpglcHueSatData);
        cpglcHueSatData = NULL;
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : cpHueSatImageCreate
    Description : Create a hue/saturation blend image
    Inputs      :
    Outputs     :
    Return      : temporary buffer that holds texture data
----------------------------------------------------------------------------*/
color* cpHueSatImageCreate(void)
{
    sdword hue, sat;
    real32 red, green, blue;
    color* pBuffer;

    if (cpHueSatTexture != TR_InvalidInternalHandle)
    {
        if (glcActive())
        {
            trRGBTextureDelete(cpHueSatTexture);
            cpHueSatTexture = TR_InvalidInternalHandle;
        }
        else
        {
            return NULL;
        }
    }
    cpHueSatImageDelete();
    cpHueSatData = pBuffer = memAlloc(CP_HueSatTextureWidth * CP_HueSatTextureHeight * sizeof(color), "ColorPickerHueSat", 0);
    for (sat = 0; sat < CP_HueSatTextureHeight; sat++)
    {
        for (hue = 0; hue < CP_HueSatTextureWidth; hue++, pBuffer++)
        {
            colHSVToRGB(&red, &green, &blue, colUbyteToReal(hue), colUbyteToReal(CP_HueSatTextureHeight - 1 - sat), colUbyteToReal(CP_HueSatTextureHeight - 1 - sat / 4));
            *pBuffer = colRGB(colRealToUbyte(red), colRealToUbyte(green), colRealToUbyte(blue));
        }
    }
    cpHueSatTexture = trRGBTextureCreate(cpHueSatData, CP_HueSatTextureWidth, CP_HueSatTextureHeight, FALSE);

#if CP_SCALE_HUESAT
    //create scaled version for glcompat module
    cpglcHueSatData = trImageScale(cpHueSatData,
                                   CP_HueSatTextureWidth, CP_HueSatTextureHeight,
                                   CPGLC_HueSatTextureWidth, CPGLC_HueSatTextureHeight,
                                   FALSE);
#endif

    return cpHueSatData;
}

color* cpValueTextureGradientCreateScaled(sdword width, sdword height)
{
    color* buffer;
    sdword hue, sat, i;
    real32 realRed, realGreen, realBlue, value, valStep;

    valStep = 1.0f / (real32)height;
    value   = 1.0f;

    if (cpColorMode == 0)
    {
        hue = cpBaseHue;
        sat = cpBaseSaturation;
    }
    else
    {
        hue = cpStripeHue;
        sat = cpStripeSaturation;
    }

    buffer = memAlloc(height * width * sizeof(color), "ColorPickerValue", 0);

    for (i = 0; i < height; i++)
    {
        colHSVToRGB(&realRed, &realGreen, &realBlue,
                    colUbyteToReal(hue), colUbyteToReal(sat), value);

        memClearDword(buffer + i * width,
                      colRGB(colRealToUbyte(realRed), colRealToUbyte(realGreen), colRealToUbyte(realBlue)),
                      width);

        value -= valStep;
    }

    return(buffer);
}

/*-----------------------------------------------------------------------------
    Name        : cpValueTextureGradientCreate
    Description : Create the gradient bitmap for the value slider.  Gradient built off the
                : current hue/saturation.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
color *cpValueTextureGradientCreate(void)
{
    color *buffer;
    sdword hue, sat, i;
    real32 realRed, realGreen, realBlue, value, valStep;

    valStep = 1.0f / (real32)CP_ValueTextureHeight;
    value = (real32)1.0;

    if (cpColorMode == 0)
    {
        hue = cpBaseHue;
        sat = cpBaseSaturation;
    }
    else
    {
        hue = cpStripeHue;
        sat = cpStripeSaturation;
    }

    buffer = memAlloc(CP_ValueTextureHeight * CP_ValueTextureWidth * sizeof(color), "ColorPickerValue", 0);

    for (i = 0; i < CP_ValueTextureHeight; i++)
    {
        colHSVToRGB(&realRed, &realGreen, &realBlue,
                    colUbyteToReal(hue), colUbyteToReal(sat), value);

        memClearDword(buffer + i * CP_ValueTextureWidth,
                      colRGB(colRealToUbyte(realRed), colRealToUbyte(realGreen), colRealToUbyte(realBlue)),
                      CP_ValueTextureWidth);

        value -= valStep;
    }

    return(buffer);
}

/*-----------------------------------------------------------------------------
    Name        : cpValueTextureCreate
    Description : Create a value texture
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpValueTextureCreate(void)
{
    color *buffer;

    if (cpValueTexture != TR_InvalidInternalHandle)
    {
        trRGBTextureDelete(cpValueTexture);
        cpValueTexture = TR_InvalidInternalHandle;
//        return;
    }

    buffer = cpValueTextureGradientCreate();
    cpValueTexture = trRGBTextureCreate(buffer, CP_ValueTextureWidth, CP_ValueTextureHeight, FALSE);
    memFree(buffer);
}

/*-----------------------------------------------------------------------------
    Name        : cpPreviewTexturePrepare
    Description : Prepares the texture which will draw the preview image.
    Inputs      : void
    Outputs     : Colorizes and creates a new texture.  Frees the old one if
                    needed.
    Return      : void
----------------------------------------------------------------------------*/
void cpPreviewTexturePrepare(void)
{
#if PREVIEW_IMAGE
    real32 scalar0, scalar1;

    scalar0 = cpScalarFromCurveGet(&colPickScalarCurve[whichRaceSelected * 2], colUbyteToReal(cpBaseHue), colUbyteToReal(cpBaseSaturation), colUbyteToReal(cpBaseValue));
    scalar1 = cpScalarFromCurveGet(&colPickScalarCurve[whichRaceSelected * 2 + 1], colUbyteToReal(cpStripeHue), colUbyteToReal(cpStripeSaturation), colUbyteToReal(cpStripeValue));
    //colorize the palette for the preview image
    dbgAssert(cpPreviewTexturePalette != NULL);
    trBufferColorRGB(cpPreviewTexturePalette, (color *)cpPreviewImage->palette,
                     cpPreviewImage->teamEffect0, cpPreviewImage->teamEffect1,
                     colRGB(cpBaseRed, cpBaseGreen, cpBaseBlue), colRGB(cpStripeRed, cpStripeGreen, cpStripeBlue),
                     256, cpPreviewImage->flags, scalar0, scalar1);
    if (cpPreviewTexture == TR_InvalidInternalHandle)
    {
        cpPreviewTexture = trPalettedTextureCreate(cpPreviewImage->data, cpPreviewTexturePalette, CP_PreviewWidth, CP_PreviewHeight);
    }
    else
    {
        if (trNoPalettes)
        {
            trRGBTextureDelete(cpPreviewTexture);
            cpPreviewTexture = trPalettedTextureCreate(cpPreviewImage->data, cpPreviewTexturePalette, CP_PreviewWidth, CP_PreviewHeight);
        }
        else
        {
            trPalettedTextureMakeCurrent(cpPreviewTexture, cpPreviewTexturePalette);
        }
    }
#endif //PREVIEW_IMAGE
}

/*-----------------------------------------------------------------------------
    Name        : cpPreviewImagePrepare
    Description : Loads in, decompresses and prepares for composition the preview image
    Inputs      : void
    Outputs     : loads in cpPreviewImage and creates cpPreviewTexture
    Return      : void
----------------------------------------------------------------------------*/
void cpPreviewImagePrepare(void)
{
#if PREVIEW_IMAGE
    char *fileName;

    if (whichRaceSelected == R1)
    {
        fileName = CP_PreviewImageRace1;
    }
    else
    {
        fileName = CP_PreviewImageRace2;
    }
    cpPreviewImage = trLIFFileLoad(fileName, NonVolatile);
    dbgAssert(cpPreviewImage->flags & TRF_Paletted);
    if (cpPreviewTexturePalette == NULL)
    {
        cpPreviewTexturePalette = memAlloc(TR_PaletteSize, "PreviewPalette", 0);
    }

    cpPreviewTexturePrepare();
#endif
}

/*-----------------------------------------------------------------------------
    Name        : cpDirtyPreviewImage
    Description : dirties (marks for update) the preview image region
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpDirtyPreviewImage()
{
    sdword i;

    for (i = 0; i < 15; i++)
    {
        if (cpRegion[i] != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(cpRegion[i]);
#endif
            regRecursiveSetDirty(cpRegion[i]);
        }
    }

    if (cpValueRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cpValueRegion);
#endif
        bitSet(cpValueRegion->status, RSF_DrawThisFrame);
    }
    if (cpHueSaturationRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cpHueSaturationRegion);
#endif
        bitSet(cpHueSaturationRegion->status, RSF_DrawThisFrame);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cpHueSaturationProcess/cpValueProcess
    Description : Process messages to the color picker regions
    Inputs      : standard callbacks
    Outputs     : updates the current colors (cpBaseRed etc.)
    Return      : 0
----------------------------------------------------------------------------*/
udword cpValueProcess(regionhandle region, sdword ID, udword event, udword data)
{
    sdword *hue, *sat, *val, *red, *green, *blue;
    real32 realRed, realGreen, realBlue;
    udword mask = 0;

    if (event == RPE_PressLeft)
    {
        mouseClipToRect(&region->rect);
        mask |= RPR_Redraw;
    }
    if (event == RPE_HoldLeft || event == RPE_PressLeft)
    {
        mask |= RPR_Redraw;
        if (cpColorMode == 0)
        {
            hue = &cpBaseHue;
            sat = &cpBaseSaturation;
            val = &cpBaseValue;
            red = &cpBaseRed;
            green = &cpBaseGreen;
            blue = &cpBaseBlue;
        }
        else
        {
            hue = &cpStripeHue;
            sat = &cpStripeSaturation;
            val = &cpStripeValue;
            red = &cpStripeRed;
            green = &cpStripeGreen;
            blue = &cpStripeBlue;
        }

        *val = (region->rect.y1 - 1 - mouseCursorY()) * CP_ValueTextureHeight / (region->rect.y1 - region->rect.y0);
        if (cpColorMode == 0)
        {
            *val = max(*val, cpDarkestColor0);
        }
        else
        {
            *val = max(*val, cpDarkestColor1);
        }
        colHSVToRGB(&realRed, &realGreen, &realBlue,
                   colUbyteToReal(*hue), colUbyteToReal(*sat), colUbyteToReal(*val));
        *red = (sdword)colRealToUbyte(realRed);
        *green = (sdword)colRealToUbyte(realGreen);
        *blue = (sdword)colRealToUbyte(realBlue);
    }
    else
    {
        cpValueTextureCreate();
        mouseClipToRect(NULL);
    }
//    cpColorsPicked = TRUE;
    cpPreviewTexturePrepare();
#if CP_VERBOSE_LEVEL >= 1
    dbgMessagef("\ncpValueProcess: ID: 0x%x, event 0x%x, data 0x%x", ID, event, data);
#endif

    if (mask != 0)
    {
        cpDirtyPreviewImage();
    }

    return(mask);
}

udword cpHueSaturationProcess(regionhandle region, sdword ID, udword event, udword data)
{
    sdword *hue, *sat, *val, *red, *green, *blue;
    real32 realRed, realGreen, realBlue;
    color *buffer;
    udword mask = 0;

    if (event == RPE_PressLeft)
    {
        mask |= RPR_Redraw;
        mouseClipToRect(&region->rect);
    }
    if (event == RPE_HoldLeft || event == RPE_PressLeft)
    {
        mask |= RPR_Redraw;
        if (cpColorMode == 0)
        {
            hue = &cpBaseHue;
            sat = &cpBaseSaturation;
            val = &cpBaseValue;
            red = &cpBaseRed;
            green = &cpBaseGreen;
            blue = &cpBaseBlue;
        }
        else
        {
            hue = &cpStripeHue;
            sat = &cpStripeSaturation;
            val = &cpStripeValue;
            red = &cpStripeRed;
            green = &cpStripeGreen;
            blue = &cpStripeBlue;
        }

        *hue = (mouseCursorX() - region->rect.x0) * CP_HueSatTextureWidth / (region->rect.x1 - region->rect.x0);
        *sat = (region->rect.y1 - 1 - mouseCursorY()) * CP_HueSatTextureHeight / (region->rect.y1 - region->rect.y0);
        colHSVToRGB(&realRed, &realGreen, &realBlue,
                   colUbyteToReal(*hue), colUbyteToReal(*sat), colUbyteToReal(*val));
        *red = (sdword)colRealToUbyte(realRed);
        *green = (sdword)colRealToUbyte(realGreen);
        *blue = (sdword)colRealToUbyte(realBlue);
    }
    else
    {
        mask |= RPR_Redraw;
        // Update the gradient slider.
        buffer = cpValueTextureGradientCreate();
        cpValueTextureCreate();
        trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
        memFree(buffer);
        mouseClipToRect(NULL);
    }
//    cpColorsPicked = TRUE;
    cpPreviewTexturePrepare();
#if CP_VERBOSE_LEVEL >= 1
    dbgMessagef("\ncpHueSaturationProcess: ID: 0x%x, event 0x%x, data 0x%x", ID, event, data);
#endif

    if (mask != 0)
    {
        cpDirtyPreviewImage();
    }

    return(mask);
}

/*-----------------------------------------------------------------------------
    Name        : Various
    Description : Process callbacks for various controls
    Inputs      : standard callbacks
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpBaseColor(char *name, featom *atom)
{
    color* buffer;

    if (FEFIRSTCALL(atom))
    {                                                       //CallOnCreate()
#if CP_VERBOSE_LEVEL >= 1
        dbgMessage("\ncpBaseColor: CallOnCreate");
#endif                                                      //set up the two process user regions
        cpHueSaturationRegion = feRegionFindByFunction("CP_HueSaturation");
        regFunctionSet(cpHueSaturationRegion, cpHueSaturationProcess);
        regFilterSet(cpHueSaturationRegion, RPE_PressLeft | RPE_HoldLeft | RPE_ReleaseLeft | RPE_ExitHoldLeft);
        cpValueRegion = feRegionFindByFunction("CP_Value");
        regFunctionSet(cpValueRegion, cpValueProcess);
        regFilterSet(cpValueRegion, RPE_PressLeft | RPE_HoldLeft | RPE_ReleaseLeft | RPE_ExitHoldLeft);
                                                            //set toggle state of the base/strip color selector buttons
        feToggleButtonSet("CP_BaseColor", TRUE);
        feToggleButtonSet("CP_StripeColor", FALSE);
        cpColorMode = 0;
        //... create textures for the user regions
        (void)cpHueSatImageCreate();
        cpValueTextureCreate();
        cpPreviewImagePrepare();
    }
    else if (FELASTCALL(atom))
    {
        //screen is dying... set the color for good
        cpAcceptColors();

        cpPreviewImageDelete();
        trRGBTextureDelete(cpHueSatTexture);                    //delete the textures
        cpHueSatTexture = TR_InvalidInternalHandle;
        cpHueSatImageDelete();
        trRGBTextureDelete(cpValueTexture);
        cpValueTexture = TR_InvalidInternalHandle;

        cpHueSaturationRegion = NULL;
        cpValueRegion = NULL;
    }
    else
    {
        feToggleButtonSet("CP_BaseColor", TRUE);
        feToggleButtonSet("CP_StripeColor", FALSE);
        cpColorMode = 0;
#if CP_VERBOSE_LEVEL >= 1
        dbgMessagef("\ncpBaseColor: color mode = 0x%x", cpColorMode);
#endif
        // Update the gradient slider.
        buffer = cpValueTextureGradientCreate();
        cpValueTextureCreate();
        trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
        cpDirtyPreviewImage();
        memFree(buffer);
    }
}
void cpStripeColor(char *name, featom *atom)
{
    color *buffer;
    feToggleButtonSet("CP_BaseColor", FALSE);
    feToggleButtonSet("CP_StripeColor", TRUE);
    cpColorMode = 1;
#if CP_VERBOSE_LEVEL >= 1
    dbgMessagef("\ncpStripeColor: color mode = 0x%x", cpColorMode);
#endif
    // Update the gradient slider.
    buffer = cpValueTextureGradientCreate();
    cpValueTextureCreate();
    trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
    cpDirtyPreviewImage();
    memFree(buffer);
}
/*-----------------------------------------------------------------------------
    Name        : cpPreviewImageDelete
    Description : Deletes the preview image, palette and texture
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cpPreviewImageDelete(void)
{
    cpDirtyPreviewImage();
#if PREVIEW_IMAGE
    if (cpPreviewTexture != TR_InvalidInternalHandle)
    {
        trRGBTextureDelete(cpPreviewTexture);
        cpPreviewTexture = TR_InvalidInternalHandle;
    }
    if (cpPreviewImage != NULL)
    {
        memFree(cpPreviewImage);                                //free the preview image and it's colorized palette
        cpPreviewImage = NULL;
    }
    if (cpPreviewTexturePalette != NULL)
    {
        memFree(cpPreviewTexturePalette);
        cpPreviewTexturePalette = NULL;
    }
#endif
}
//previous color buttons: choose some previously chosen colors
void cpPreviousColor(char *name, featom *atom)
{
    sdword index = -1;
    color *buffer;

    sscanf(name, "CP_Previous%d", &index);
    dbgAssert(index >= 0 && index < CP_NumberPreviousColors);
    if (colPreviousColors[index].base != colBlack)
    {                                                       //if this color was set properly
        cpColorsUpdate(colPreviousColors[index].base, colPreviousColors[index].detail);
        cpDirtyPreviewImage();
        //regRecursiveSetDirty(feStack[feStackIndex].baseRegion);
        buffer = cpValueTextureGradientCreate();
        cpValueTextureCreate();
        trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
        memFree(buffer);
//        cpDirtyPreviewImage();
        cpPreviewTexturePrepare();
    }
    cpColorsPicked = TRUE;
}

//default color buttons: reset to your race defaults
void cpDefault(char *name, featom *atom)
{
    color *buffer;
    //set default color schemes
    teReset();
    cpColorsUpdate(teColorSchemes[0].textureColor.base, teColorSchemes[0].textureColor.detail);
    cpDirtyPreviewImage();
    cpPreviewImageDelete();
    cpColorsPicked = TRUE;      // we deliberately pressed this button, so treat as if colors are picked!
    buffer = cpValueTextureGradientCreate();
    cpValueTextureCreate();
    trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
    memFree(buffer);
    cpDirtyPreviewImage();
}

void cpResetRegions(void)
{
    sdword index;

    for (index = 0; index < 15; index++)
    {
        cpRegion[index] = NULL;
    }

    cpHueSaturationRegion = NULL;

    cpValueRegion = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : cpNewRace
    Description : Race select callback from within the color picker
    Inputs      : name - name of entry to envoke this function
    Outputs     : modifies whichRaceSelected (== R1 or R2)
    Return      :
----------------------------------------------------------------------------*/
void cpNewRace(char *name, featom *atom)
{
    udword raceOne, raceTwo;
    ShipRace oldRace = whichRaceSelected;
    tecolorscheme tempScheme;
    color base, stripe;
    color *buffer;

#if defined(Downloadable)
    if (FEFIRSTCALL(atom) && (strcmp(name, "CP_RaceOne")))
    {                                                       //disable the Race 2 region right away
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((regionhandle)atom->region)->status, RSF_RegionDisabled);
        whichRaceSelected = R1;
        goto forcedToRace1;
    }
#endif

    base = colRGB(cpBaseRed, cpBaseGreen, cpBaseBlue);
    stripe = colRGB(cpStripeRed, cpStripeGreen, cpStripeBlue);
    if (base != teColorSchemes[0].textureColor.base || stripe != teColorSchemes[0].textureColor.detail)// ||
//        base != *cpPBaseColor || stripe != *cpPStripeColor)
    {                                                       //if this is not the default team colors or it has been changed at all
        cpColorsPicked = TRUE;                              //player has chosen some colors
    }
    else
    {
        cpColorsPicked = FALSE;
    }
    if (!FEFIRSTCALL(atom))
    {
        whichRaceSelected = (!strcmp(name, "CP_RaceOne")) ? R1 : R2;
        if (whichRaceSelected != oldRace && (!cpColorsPicked))
        {                                                   //if new race selected and no colors picked
            swap(teColorSchemes[0], teColorSchemes[1], tempScheme);//swap default R1/R2 color schemes
            cpColorsUpdate(teColorSchemes[0].textureColor.base, teColorSchemes[0].textureColor.detail);
            cpDirtyPreviewImage();
        }
    }
#if defined(Downloadable)
forcedToRace1:;
#endif
    // ensure non-selected button Off, selected On
    raceOne = (whichRaceSelected == R1) ? TRUE : FALSE;
    raceTwo = (whichRaceSelected == R2) ? TRUE : FALSE;
    feToggleButtonSet("CP_RaceOne", raceOne);
    feToggleButtonSet("CP_RaceTwo", raceTwo);
    cpPreviewImageDelete();
    buffer = cpValueTextureGradientCreate();
    cpValueTextureCreate();
    trRGBTextureUpdate(cpValueTexture, buffer, CP_ValueTextureWidth, CP_ValueTextureHeight);
    memFree(buffer);
    cpDirtyPreviewImage();
}


/*-----------------------------------------------------------------------------
    Name        : cpNumberDraw
    Description : Draws a number in a rectangle, then draws a rectangle
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpNumberDraw(featom *atom, regionhandle region, sdword value)
{
    fonthandle  fhSave;

    rectangle *r = &region->rect;

    if (bitTest(atom->flags, FAF_BorderVisible))
    {
        primRectOutline2(r, atom->borderWidth, atom->borderColor);
    }

    if (feShouldSaveMouseCursor())
    {
        rectangle rect = region->rect;
        rect.x0++;
        rect.y1 -= 2;
        primRectSolid2(&rect, colBlack);
    }

    fhSave = fontCurrentGet();                              //save the current font
    fontMakeCurrent(cpValuesFont);                          //select the appropriate font
    fontPrintf(r->x0 + CP_NumberMarginX, r->y0 + (r->y1 - r->y0 - fontHeight(" ")) / 2, atom->borderColor, "%d", value);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : Various
    Description : Draw callbacks for different parts of the color picker
    Inputs      : standard draw callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpBaseRedDraw(featom *atom, regionhandle region)
{
    cpRegion[0] = region;
    cpNumberDraw(atom, region, cpBaseRed);
}
void cpBaseGreenDraw(featom *atom, regionhandle region)
{
    cpRegion[1] = region;
    cpNumberDraw(atom, region, cpBaseGreen);
}
void cpBaseBlueDraw(featom *atom, regionhandle region)
{
    cpRegion[2] = region;
    cpNumberDraw(atom, region, cpBaseBlue);
}
void cpBaseHueDraw(featom *atom, regionhandle region)
{
    cpRegion[3] = region;
    cpNumberDraw(atom, region, cpBaseHue);
}
void cpBaseSaturationDraw(featom *atom, regionhandle region)
{
    cpRegion[4] = region;
    cpNumberDraw(atom, region, cpBaseSaturation);
}
void cpBaseValueDraw(featom *atom, regionhandle region)
{
    cpRegion[5] = region;
    cpNumberDraw(atom, region, cpBaseValue);
}
void cpStripeRedDraw(featom *atom, regionhandle region)
{
    cpRegion[6] = region;
    cpNumberDraw(atom, region, cpStripeRed);
}
void cpStripeGreenDraw(featom *atom, regionhandle region)
{
    cpRegion[7] = region;
    cpNumberDraw(atom, region, cpStripeGreen);
}
void cpStripeBlueDraw(featom *atom, regionhandle region)
{
    cpRegion[8] = region;
    cpNumberDraw(atom, region, cpStripeBlue);
}
void cpStripeHueDraw(featom *atom, regionhandle region)
{
    cpRegion[9] = region;
    cpNumberDraw(atom, region, cpStripeHue);
}
void cpStripeSaturationDraw(featom *atom, regionhandle region)
{
    cpRegion[10] = region;
    cpNumberDraw(atom, region, cpStripeSaturation);
}
void cpStripeValueDraw(featom *atom, regionhandle region)
{
    cpRegion[11] = region;
    cpNumberDraw(atom, region, cpStripeValue);
}
void cpBaseColorPreviewDraw(featom *atom, regionhandle region)
{
    if (FELASTCALL(atom))
    {
        cpRegion[12] = NULL;
        return;
    }

    cpRegion[12] = region;
    region->rect.y1--;
    primRectSolid2(&region->rect, colRGB(cpBaseRed, cpBaseGreen, cpBaseBlue));
    region->rect.y1++;
    primRectOutline2(&region->rect, atom->borderWidth, atom->borderColor);
}
void cpStripeColorPreviewDraw(featom *atom, regionhandle region)
{
    if (FELASTCALL(atom))
    {
        cpRegion[13] = NULL;
        cpHueSaturationRegion = NULL;
        cpValueRegion = NULL;
        return;
    }

    cpRegion[13] = region;
    region->rect.y1--;
    primRectSolid2(&region->rect, colRGB(cpStripeRed, cpStripeGreen, cpStripeBlue));
    region->rect.y1++;
    primRectOutline2(&region->rect, atom->borderWidth, atom->borderColor);
}
void cpPreviewImageDraw(featom *atom, regionhandle region)
{
    if (FELASTCALL(atom))
    {
        cpRegion[14] = NULL;
        return;
    }

#if PREVIEW_IMAGE
    cpRegion[14] = region;
    if (cpPreviewTexture == TR_InvalidInternalHandle)
    {
        cpPreviewImagePrepare();
    }
    trPalettedTextureMakeCurrent(cpPreviewTexture, cpPreviewTexturePalette);
    rndPerspectiveCorrection(FALSE);
    if (glcActive())
    {
        glcRectSolidTextured2(&region->rect,
                              region->rect.x1 - region->rect.x0,
                              region->rect.y1 - region->rect.y0,
                              cpPreviewImage->data,
                              cpPreviewTexturePalette,
                              TRUE);
    }
    else
    {
        primRectSolidTextured2(&region->rect);
    }
    primRectOutline2(&region->rect, atom->borderWidth, atom->borderColor);
#endif
}
void cpHueSaturationDraw(featom *atom, regionhandle region)
{
    oval o;
    sdword *hue, *sat;
    rectangle rect;

    if (FELASTCALL(atom))
    {
        cpHueSaturationRegion = NULL;
        return;
    }

    if (cpColorMode == 0)
    {
        hue = &cpBaseHue;
        sat = &cpBaseSaturation;
    }
    else
    {
        hue = &cpStripeHue;
        sat = &cpStripeSaturation;
    }
    //remove out-of-region ovals
    rect = region->rect;
    rect.x0 -= CP_HueSaturationCircleRadius;
    rect.x1 += CP_HueSaturationCircleRadius;
    rect.y0 -= CP_HueSaturationCircleRadius + 1;
    rect.y1 += CP_HueSaturationCircleRadius;
    primRectSolid2(&rect, atom->borderColor);
    //draw the hue/saturation map
    if (cpHueSatTexture == TR_InvalidInternalHandle)
    {
        (void)cpHueSatImageCreate();
    }
    if (glcActive())
    {
#if CP_SCALE_HUESAT
        glcRectSolidTextured2(&region->rect,
                              CP_HueSatTextureWidth,
                              CP_HueSatTextureHeight,
                              (ubyte*)cpglcHueSatData,
                              NULL, TRUE);
#else
        glcRectSolidTexturedScaled2(&region->rect,
                                    CP_HueSatTextureWidth,
                                    CP_HueSatTextureHeight,
                                    (ubyte*)cpHueSatData,
                                    NULL, TRUE);
#endif
    }
    else
    {
        trRGBTextureMakeCurrent(cpHueSatTexture);
        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&region->rect);
    }
    primLine2(region->rect.x0, region->rect.y1-1, region->rect.x1, region->rect.y1-1, atom->borderColor);
    primLine2(region->rect.x0, region->rect.y1, region->rect.x1, region->rect.y1, atom->borderColor);
    //draw the pointer indicating what color we have
    o.centreX = (region->rect.x0 + *hue * (region->rect.x1 - region->rect.x0) / CP_HueSatTextureWidth + 1);
    o.centreY = (region->rect.y1 - *sat * (region->rect.y1 - region->rect.y0) / CP_HueSatTextureHeight);
    o.radiusX = o.radiusY = CP_HueSaturationCircleRadius;
    primOvalArcOutline2(&o, 0.0f, TWOPI, 1,
                        CP_HueSaturationCircleSegments, colBlack);
    o.centreX--;
    o.centreY--;
    primOvalArcOutline2(&o, 0.0f, TWOPI, 1,
                        CP_HueSaturationCircleSegments, CP_HueSaturationCircleColor);
#if CP_VISUALIZE_CURVES
    {
        colpickcurve *curve = &colPickScalarCurve[whichRaceSelected * 2];
        sdword cIndex, index;
        real32 realX0, realY0, realWidth, realHeight;

        realX0 = primScreenToGLX(region->rect.x0);
        realY0 = primScreenToGLY(region->rect.y1);
        realWidth = (real32)(region->rect.x1 - region->rect.x0);
        realHeight = (real32)(region->rect.y1 - region->rect.y0);
        glColor3ub((ubyte)cpBaseRed, (ubyte)cpBaseGreen, (ubyte)cpBaseBlue);     //color for first line
        for (cIndex = 0; cIndex < 2; cIndex++, curve++)
        {
            glBegin(GL_LINE_STRIP);
            for (index = 0; index < curve->num; index++)
            {
                glVertex2f(realX0 + primScreenToGLScaleX(realWidth * curve->hue[index]),
                           realY0 + primScreenToGLScaleY(realHeight * curve->val[index]));
            }
            glEnd();
            glColor3ub((ubyte)cpStripeRed, (ubyte)cpStripeGreen, (ubyte)cpStripeBlue);//color for second line
        }
    }
#endif
}
void cpValueDraw(featom *atom, regionhandle region)
{
    triangle tri;
    sdword val;
    rectangle rect;

    if (FELASTCALL(atom))
    {
        cpValueRegion = NULL;
        return;
    }

    //draw the value pointer
    if (cpColorMode == 0)
    {
        val = cpBaseValue;
    }
    else
    {
        val = cpStripeValue;
    }

    if (feShouldSaveMouseCursor())
    {
        rectangle rect = region->rect;
        rect.x1++;
        rect.y0 -= cpValueArrowHeight / 2 + 1;
        rect.y1 += cpValueArrowHeight / 2 + 1;
        primRectSolid2(&rect, colBlack);
    }

    val = val * (region->rect.y1 - region->rect.y0) / CP_ValueTextureHeight;
    tri.x0 = region->rect.x1 - cpValueArrowWidth;
    tri.y0 = region->rect.y1 - val - 1;
    tri.x1 = tri.x2 = tri.x0 + cpValueArrowWidth;
    tri.y1 = tri.y0 - cpValueArrowHeight / 2;
    tri.y2 = tri.y0 + cpValueArrowHeight / 2;
    primTriOutline2(&tri, 1, cpValueArrowColor);
    //draw the gradient of value
    rect = region->rect;
    rect.x1 = tri.x0 - 1;

//    cpValueTextureCreate();
    if (glcActive())
    {
        color* buffer = cpValueTextureGradientCreateScaled(rect.x1 - rect.x0, rect.y1 - rect.y0);
        if (buffer != NULL)
        {
            glcRectSolidTexturedScaled2(&rect,
                                        rect.x1 - rect.x0,
                                        rect.y1 - rect.y0,
                                        (ubyte*)buffer,
                                        NULL, TRUE);
            memFree(buffer);
        }
    }
    else
    {
        trRGBTextureMakeCurrent(cpValueTexture);
        primRectSolidTextured2(&rect);
    }
    primRectOutline2(&rect, atom->borderWidth, atom->borderColor);
}
void cpPreviousDraw(featom *atom, regionhandle region)
{
    sdword index;
    rectangle rect;

    index = atom->name[15] - '0';                           //index of "CP_PreviousDraw0"
    primRectSolid2(&region->rect, colPreviousColors[index].base);
    rect.x0 = region->rect.x0;
    rect.x1 = region->rect.x1;
    rect.y0 = region->rect.y0 + (region->rect.y1 - region->rect.y0) * 1 / 3;
    rect.y1 = region->rect.y0 + (region->rect.y1 - region->rect.y0) * 3 / 5;
    primRectSolid2(&rect, colPreviousColors[index].detail);
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : cpStartup
    Description : Startup the color picker
    Inputs      : base, stripe - starting team colors
    Outputs     : Adds a bunch of callbacks.  That's about it.
    Return      : void
----------------------------------------------------------------------------*/
void cpStartup(color *base, color *stripe)
{
    sdword index;
    real32 *oldHues, *oldValues;

    for (index = 0; index < 15; index++)
    {
        cpRegion[index] = NULL;
    }

    cpColorsUpdate(*base, *stripe);
    feCallbackAddMultiple(cpProcessCallbacks);
    feDrawCallbackAddMultiple(cpDrawCallbacks);
    cpValuesFont = frFontRegister(CP_FontFile);

    //read in the hue-specific scalar curves
    scriptSet(NULL, "colPick.script", colPickScriptTable);

    //now scan the curves and make sure they have valid starting/ending points
    for (index = 0; index < NUM_RACES * 2; index++)
    {                                                       //for each curve
        if (colPickScalarCurve[index].hue == NULL || colPickScalarCurve[index].hue[0] != 0.0f)
        {                                                   //if this curve has no entry at zero
            colPickScalarCurve[index].num++;                //add one in at zero
            oldHues = colPickScalarCurve[index].hue;
            oldValues = colPickScalarCurve[index].val;
            colPickScalarCurve[index].hue = memAlloc(colPickScalarCurve[index].num * sizeof(real32), "huescalar1", NonVolatile);
            colPickScalarCurve[index].val = memAlloc(colPickScalarCurve[index].num * sizeof(real32), "valscalar1", NonVolatile);
            colPickScalarCurve[index].hue[0] = 0.0f;
            if (oldHues != NULL)
            {                                               //if there were previous values
                memcpy(&colPickScalarCurve[index].hue[1], oldHues, (colPickScalarCurve[index].num - 1) * sizeof(real32));
                memcpy(&colPickScalarCurve[index].val[1], oldValues, (colPickScalarCurve[index].num - 1) * sizeof(real32));
                colPickScalarCurve[index].val[0] = oldValues[0];
                memFree(oldHues);
                memFree(oldValues);
            }
            else
            {
                colPickScalarCurve[index].val[0] = 1.0f;
            }
        }
        if (colPickScalarCurve[index].hue[colPickScalarCurve[index].num - 1] != 1.0f)
        {                                                   //if this curve has no entry at 1
            colPickScalarCurve[index].hue = memRealloc(colPickScalarCurve[index].hue,
                    (colPickScalarCurve[index].num + 1) * sizeof(real32), "huescalar2", NonVolatile);
            colPickScalarCurve[index].val = memRealloc(colPickScalarCurve[index].val,
                    (colPickScalarCurve[index].num + 1) * sizeof(real32), "valscalar2", NonVolatile);
            colPickScalarCurve[index].hue[colPickScalarCurve[index].num] = 1.0f;
            colPickScalarCurve[index].val[colPickScalarCurve[index].num] =
                colPickScalarCurve[index].val[colPickScalarCurve[index].num - 1];
            colPickScalarCurve[index].num++;                //add one in at zero
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cpShutdown
    Description : Shut down the color picker for good
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpShutdown(void)
{
    sdword index;

    cpTexturesPurge();

    //free the hue-specific curves
    for (index = 0; index < NUM_RACES * 2; index++)
    {
        memFree(colPickScalarCurve[index].hue);
        memFree(colPickScalarCurve[index].val);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cpColorsUpdate
    Description : Re-load the color picker colors from the pointers.
    Inputs      : base, stripe - newly chosen colors
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpColorsUpdate(color base, color stripe)
{
    real32 hue, sat, val;
    cpStripeRed = colRed(stripe);
    cpStripeGreen = colGreen(stripe);
    cpStripeBlue = colBlue(stripe);
    colRGBToHSV(&hue, &sat, &val, colUbyteToReal(cpStripeRed), colUbyteToReal(cpStripeGreen), colUbyteToReal(cpStripeBlue));
    cpStripeHue = colRealToUbyte(hue);
    cpStripeSaturation = colRealToUbyte(sat);
    cpStripeValue = colRealToUbyte(val);
    cpBaseRed = colRed(base);
    cpBaseGreen = colGreen(base);
    cpBaseBlue = colBlue(base);
    colRGBToHSV(&hue, &sat, &val, colUbyteToReal(cpBaseRed), colUbyteToReal(cpBaseGreen), colUbyteToReal(cpBaseBlue));
    cpBaseHue = colRealToUbyte(hue);
    cpBaseSaturation = colRealToUbyte(sat);
    cpBaseValue = colRealToUbyte(val);
}

/*-----------------------------------------------------------------------------
    Name        : cpColorsPick
    Description : Start the color picker and register what colors are to be set.
    Inputs      : base,stripe - pointers to the colors to be set, plus the
                    initial colors.
    Outputs     : base,stripe - newly chosen colors, if the 'Done' button pressed.
                  race - race of newly chosen ship
    Return      : void
----------------------------------------------------------------------------*/
void cpColorsPick(color *base, color *stripe, ShipRace *race)
{
    cpPBaseColor = base;
    cpPStripeColor = stripe;
    feScreenStart(ghMainRegion, CP_ScreenName);
}

/*-----------------------------------------------------------------------------
    Name        : cpSetColorsToModify
    Description : sets which variables get modified by the color picker.
    Inputs      : variables
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void cpSetColorsToModify(color *base, color *stripe)
{
    cpPBaseColor = base;
    cpPStripeColor = stripe;
    cpColorsUpdate(*base, *stripe);
}

/*-----------------------------------------------------------------------------
    Name        : cpAcceptColors
    Description : copies the colors selected to the current variables
    Inputs      : none
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cpAcceptColors(void)
{
    color base, stripe;
//    tecolorscheme tempScheme;

    //set default color schemes, to ensure team0 has the correct colors
    teReset();
/*
    if (whichRaceSelected != R1)
    {                                                       //if new race selected and no colors picked
        swap(teColorSchemes[0], teColorSchemes[1], tempScheme);//swap default R1/R2 color schemes
    }
*/
    base = colRGB(cpBaseRed, cpBaseGreen, cpBaseBlue);
    stripe = colRGB(cpStripeRed, cpStripeGreen, cpStripeBlue);
    if (base != teColorSchemes[0].textureColor.base || stripe != teColorSchemes[0].textureColor.detail ||
        base != *cpPBaseColor || stripe != *cpPStripeColor)
    {                                                       //if this is not the default team colors or it has been changed at all
        cpColorsPicked = TRUE;                              //player has chosen some colors
    }
#if 0       // could have been set TRUE by other functions, don't reset
    else
    {
        cpColorsPicked = FALSE;
    }
#endif
    *cpPBaseColor = base;                                   //set the color
    *cpPStripeColor = stripe;
}

/*-----------------------------------------------------------------------------
    Name        : cpTexturesPurge
    Description : Purge textures used in the color picker, because they are no longer needed
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpTexturesPurge(void)
{
    cpHueSatImageDelete();
    if (cpHueSatTexture != TR_InvalidInternalHandle)
    {
        trRGBTextureDelete(cpHueSatTexture);
        cpHueSatTexture = TR_InvalidInternalHandle;
    }
    if (cpValueTexture != TR_InvalidInternalHandle)
    {
        trRGBTextureDelete(cpValueTexture);
        cpValueTexture = TR_InvalidInternalHandle;
    }
#if PREVIEW_IMAGE
    cpPreviewImageDelete();
#endif
}

/*-----------------------------------------------------------------------------
    Name        : cpScalarFromCurveGet
    Description : Linear interpolate a color modifier curve for a given hue
    Inputs      : curve - cure to scan in
                  hue - hue to find modifier for
    Outputs     :
    Return      : interpolated modifier value
----------------------------------------------------------------------------*/
real32 cpScalarFromCurveGet(colpickcurve *curve, real32 hue, real32 sat, real32 val)
{
    sdword index;
    real32 factor, oneMinusFactor;

    if (hue < 0.0f || hue > 1.0f)
    {                                                       //if undefined hue
        hue = 0.0f;                                         //use a default hue
    }

    for (index = 0; index < curve->num - 1; index++)
    {                                                       //scan through all points but the last
        if (hue >= curve->hue[index] && hue <= curve->hue[index + 1])
        {
            factor = (hue - curve->hue[index]) / (curve->hue[index + 1] - curve->hue[index]);
            oneMinusFactor = 1.0f - factor;

            return(factor * curve->val[index] + oneMinusFactor * curve->val[index + 1]);
        }
    }
    dbgAssert(FALSE);

    return 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : cpTeamEffectScalars
    Description : Compute team effect scalars from a color and a race
    Inputs      : c0 - team effect base color
                  c1 - team effect stripe color
                  race - race to determine factor of
    Outputs     : scalar0 - team effect base color scalar
                  scalar1 - team effect stripe color scalar
    Return      :
----------------------------------------------------------------------------*/
void cpTeamEffectScalars(real32 *scalar0, real32 *scalar1, color c0, color c1, ShipRace race)
{
    real32 red, green, blue, hue, sat, val;

    dbgAssert(race >= 0 && race < NUM_RACES);
    red = colUbyteToReal(colRed(c0));                       //get floating-point colors
    green = colUbyteToReal(colGreen(c0));
    blue = colUbyteToReal(colBlue(c0));
    colRGBToHSV(&hue, &sat, &val, red, green, blue);        //convert to HSV
    *scalar0 = cpScalarFromCurveGet(&colPickScalarCurve[race * 2], hue, sat, val);//get scalar from curve
    red = colUbyteToReal(colRed(c1));
    green = colUbyteToReal(colGreen(c1));
    blue = colUbyteToReal(colBlue(c1));
    colRGBToHSV(&hue, &sat, &val, red, green, blue);
    *scalar1 = cpScalarFromCurveGet(&colPickScalarCurve[race * 2 + 1], hue, sat, val);
}

/*-----------------------------------------------------------------------------
    Name        : cpColorsAddToPreviousList
    Description : Add a color pair to the chosen color history
    Inputs      : base, stripe - color scheme to choose
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cpColorsAddToPreviousList(color base, color stripe)
{
    sdword index;
    trcolorinfo tempInfo;

    for (index = 0; index < CP_NumberPreviousColors; index++)
    {
        if (colPreviousColors[index].base == base && colPreviousColors[index].detail == stripe)
        {                                                   //if this is the color chosen
            if (index != 0)
            {                                               //and it was not the first
                swap(colPreviousColors[index], colPreviousColors[0], tempInfo);//swap with the first
            }
            return;                                         //no need to add a new entry in
        }
    }
    //else it's a new color scheme; add it in at the start of the list
    for (index = CP_NumberPreviousColors - 1; index > 0; index--)
    {
        colPreviousColors[index] = colPreviousColors[index - 1];
    }
    colPreviousColors[index].base = base;
    colPreviousColors[index].detail = stripe;
}

