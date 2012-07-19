/*=============================================================================
    Name    : color.h
    Purpose : Definitions for color definition.

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COLOR_H
#define ___COLOR_H

#include "types.h"

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef udword rgbquad;
typedef udword rgbaquad;
typedef rgbquad color;

/*=============================================================================
    Macros:
=============================================================================*/
//create RGB/RGBA quads
#define colRGB(r,g,b)       (0xff000000 | (((ubyte)(b)) << 16) | (((ubyte)(g)) << 8) | ((ubyte)(r)))
#define colRGBA(r,g,b,a)    ((((udword)(a)) << 24) | (((udword)(b)) << 16) | (((udword)(g)) << 8) | (udword)(r))

//extract elements from RGB/RGBA quads
#define colRed(rgb)         ((ubyte)((rgb) & 0x000000ff))
#define colGreen(rgb)       ((ubyte)(((rgb) & 0x0000ff00) >> 8))
#define colBlue(rgb)        ((ubyte)(((rgb) & 0x00ff0000) >> 16))
#define colAlpha(rgba)      ((ubyte)(((rgba) & 0xff000000) >> 24))
#define colClampRed(rgb)    colClamp256(colRed(rgb))
#define colClampGreen(rgb)  colClamp256(colGreen(rgb))
#define colClampBlue(rgb)   colClamp256(colBlue(rgb))
#define colClampAlpha(rgba) colClamp256(colAlpha(rgb))
#define colReal32(c)        ((real32)(c) / 256.0f)

//stock colors
#define colWhite            colRGB(255, 255, 255)
#define colBlack            colRGB(0, 0, 0)
#define colFuscia           colRGB(73, 98, 100)
#define colReddish          colRGB(239, 61, 46)

//convert between ubyte colors (0..255) and floating point colors (0..1)
#define colUbyteToReal(b)   ((real32)(b) / 255.0f)
#define colRealToUbyte(r)   ((ubyte)((r) * 255.0f))

#define colClamp256(n)      ((n) < 0 ? 0 : ((n) > 255 ? 255 : (n)))

/*=============================================================================
    Functions:
=============================================================================*/

//color-space conversions
void colRGBToHSV(real32 *H, real32 *S, real32 *V, real32 R, real32 G, real32 B);
void colHSVToRGB(real32 *R, real32 *G, real32 *B, real32 H, real32 S, real32 V);
void colRGBToHLS(real32 *H, real32 *L, real32 *S, real32 R, real32 G, real32 B);
void colHLSToRGB(real32 *R, real32 *G, real32 *B, real32 H, real32 L, real32 S);
udword colIntensityNTSC(color c);

//palette mapping crap
color colBestFitFindRGB(color *palette, color colorToMatch, sdword length);

color colMultiply(color c, real32 factor);
color colMultiplyClamped(color c, real32 factor);
sdword colRGBCompare(color *p0, color *p1, sdword nPixels);
color colBlend(color c0, color c1, real32 factor);


#endif //___COLOR_H

