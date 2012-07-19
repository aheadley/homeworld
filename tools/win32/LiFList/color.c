/*=============================================================================
    Name    : Color .c
    Purpose : Color-related utility functions.

    Created 10/13/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include "types.h"
#include "color.h"

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : colRGBToHSV
    Description : Converts a RGB color to an HSV color.
    Inputs      : RGB - color to convert (0..1)
    Outputs     : HSV - color to convert (0..1)
    Return      : void
----------------------------------------------------------------------------*/
#define LI_UndefinedHue         -1.0f
void colRGBToHSV(real32 *H, real32 *S, real32 *V, real32 R, real32 G, real32 B)
{
    real32 maxVal = max(R, max(G, B));
    real32 minVal = min(R, min(G, B));
    real32 delta;

    *V = maxVal;                                            //V is real simple
    if (maxVal != 0.0f)
    {
        *S = (maxVal - minVal) / maxVal;                    //this is the saturation
    }
    else
    {
        *S = 0.0f;                                          //saturation is zero if RGB are all zero
    }
    if (*S == 0.0f)
    {                                                       //if saturation zero
        *H = LI_UndefinedHue;                               //hue unknown
    }
    else
    {                                                       //else there is color
        delta = maxVal - minVal;
        if (R == maxVal)
        {
            *H = (G - B) / delta;                           //between yellow and Magenta
        }
        else if (G == maxVal)
        {
            *H = 2.0f + (B - R) / delta;                    //between cyan and yellow
        }
        else
        {
            *H = 4.0f + (R - G) / delta;                    //between magenta and cyan
        }
        *H /= 6.0f;                                         //convert from color hextants to 0..1
        if (*H < 0.0f)
        {                                                   //make sure it's positive
            *H += 1.0f;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : colHSVToRGB
    Description : Convert a HSV color to RGB space
    Inputs      : H,S,V - color to convert
    Outputs     : R,G,B - converted color
    Return      : void
----------------------------------------------------------------------------*/
void colHSVToRGB(real32 *R, real32 *G, real32 *B, real32 H, real32 S, real32 V)
{
    real32 integer, fraction;
    real32 p, q, t;
    if (S == 0)                                             //if no saturation
    {                                                       //hue cannot be solved not known
        *R = *G = *B = V;
    }
    else
    {
        if (H == 1.0f)
        {                                                   //0 and 1 the same
            H = 0.0f;
        }
        H *= 6.0f;                                          //convert to color hextants
        integer = (real32)floor((double)H);                 //integer part
        fraction = H - integer;                             //fractional part
        p = V * (1 - S);
        q = V * (1 - S * fraction);
        t = V * (1 - (S * (1 - fraction)));
#define colAssign(r, g, b)   *R = (r);*G = (g);*B = (b)
        switch ((sdword)integer)
        {
            case 0:
                colAssign(V, t, p);
                break;
            case 1:
                colAssign(q, V, p);
                break;
            case 2:
                colAssign(p, V, t);
                break;
            case 3:
                colAssign(p, q, V);
                break;
            case 4:
                colAssign(t, p, V);
                break;
            case 5:
                colAssign(V, p, q);
                break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : colRGBToHLS
    Description : Converts a RGB color to an HLS color.
    Inputs      : RGB - color to convert (0..1)
    Outputs     : HLS - color to convert (0..1)
    Return      : void
----------------------------------------------------------------------------*/
void colRGBToHLS(real32 *H, real32 *L, real32 *S, real32 R, real32 G, real32 B)
{
    real32 maxVal = max(R, max(G, B));
    real32 minVal = min(R, min(G, B));
    real32 delta;

    *L = (maxVal + minVal) / 2.0f;                          //L is real simple
    if (maxVal == minVal)
    {                                                       //if saturation zero
        *S = 0.0f;
        *H = LI_UndefinedHue;                               //hue unknown
    }
    else
    {                                                       //else there is color
        if (*L < 0.5f)
        {
            *S = (maxVal - minVal) / (maxVal + minVal);
        }
        else
        {
            *S = (maxVal - minVal) / (2.0f - maxVal - minVal);
        }
        delta = maxVal - minVal;
        if (R == maxVal)
        {
            *H = (G - B) / delta;                           //between yellow and Magenta
        }
        else if (G == maxVal)
        {
            *H = 2.0f + (B - R) / delta;                    //between cyan and yellow
        }
        else
        {
            *H = 4.0f + (R - G) / delta;                    //between magenta and cyan
        }
        *H /= 6.0f;                                         //convert from color hextants to 0..1
        if (*H < 0.0f)
        {                                                   //make sure it's positive
            *H += 1.0f;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : colHLSValue
    Description : A helper function for colHLSToRGB
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define hue60       (60.0f / 360.0f)
#define hue120      (120.0f / 360.0f)
#define hue180      (180.0f / 360.0f)
#define hue240      (240.0f / 360.0f)
#define hue360      (1.0f)
real32 colHLSValue(real32 n1, real32 n2, real32 hue)
{
    if (hue > hue360)
    {
        hue -= hue360;
    }
    else if (hue < 0.0f)
    {
        hue += hue360;
    }
    //now 0<=hue<=1
    if (hue < hue60)
    {       //red-yellow
        return(n1 + (n2 - n1) * hue / hue60);
    }
    else if (hue < hue180)
    {       //yellow-cyan
        return(n2);
    }
    else if (hue < hue240)
    {       //cyan-blue
        return(n1 + (n2 - n1) * (hue240 - hue) / hue60);
    }
            //blue back to red
    return(n1);
}

/*-----------------------------------------------------------------------------
    Name        : colHLSToRGB
    Description : Convert a HLS color to RGB space
    Inputs      : H,S,L - color to convert
    Outputs     : R,G,B - converted color
    Return      : void
----------------------------------------------------------------------------*/
void colHLSToRGB(real32 *R, real32 *G, real32 *B, real32 H, real32 L, real32 S)
{
    real32 m1, m2;

    if (L < 0.5f)
    {
        m2 = L * (1.0f + S);
    }
    else
    {
        m2 = L + S - L * S;
    }
    m1 = 2.0f * L - m2;
    if (S == 0)                                             //if no saturation
    {                                                       //hue cannot be solved not known
        *R = *G = *B = L;
    }
    else
    {                                                       //else hue can be solved for
        *R = colHLSValue(m1, m2, H + hue120);
        *G = colHLSValue(m1, m2, H);
        *B = colHLSValue(m1, m2, H - hue120);
    }
}

/*-----------------------------------------------------------------------------
    Name        : colBestFitFindRGB
    Description : Find a color in a palette, or a reasonable match, using
                    a manhattan distance in RGB space.
    Inputs      : palette - palette to search in.
                  colorToMatch - color to match as closly as possible.
                  length - length of palette to search in.
    Outputs     :
    Return      : index into palette of closest match of colorToMatch
----------------------------------------------------------------------------*/
color colBestFitFindRGB(color *palette, color colorToMatch, sdword length)
{
    sdword index, error, bestError = SDWORD_Max, bestIndex;//, bestColor;

    for (index = 0; index < length; index++, palette++)
    {
        error = abs(colRed(*palette) - colRed(colorToMatch)) +
            abs(colGreen(*palette) - colGreen(colorToMatch)) +
            abs(colBlue(*palette) - colBlue(colorToMatch));
        if (error < bestError)
        {
            if (error == 0)
            {
                return(index);
            }
            bestError = error;
            bestIndex = index;
//            bestColor = *palette;
        }
    }
    return(bestIndex);
}

/*-----------------------------------------------------------------------------
    Name        : colIntensityNTSC
    Description : Return the intensity of a color based upon the NTSC standard
                    color triple weightings.
    Inputs      : c - color to get intensity of
    Outputs     :
    Return      : intensity of c
----------------------------------------------------------------------------*/
#define Float2Fixed(x)      ((udword)((x) * 65536.0f))
#define Fixed2Int(f)        ((f) >> 16)
udword colIntensityNTSC(color c)
{
    udword red, green, blue, returnValue;

    red = colRed(c);
    green = colGreen(c);
    blue = colBlue(c);
    returnValue = Fixed2Int(red * Float2Fixed(0.229f) + green * Float2Fixed(0.587f) + blue * Float2Fixed(0.114));
    return(returnValue);
}

/*-----------------------------------------------------------------------------
    Name        : colMultiply
    Description : multiply a color by a floating-point factor
    Inputs      :
    Outputs     :
    Return      : multiplied color
----------------------------------------------------------------------------*/
color colMultiply(color c, real32 factor)
{
    udword red, green, blue;
    udword intFactor;

    intFactor = (udword)colRealToUbyte(factor);
    red = colRed(c);
    green = colGreen(c);
    blue = colBlue(c);
    return(colRGB((red * intFactor) >> 8, (green * intFactor) >> 8, (blue * intFactor) >> 8));
}

/*-----------------------------------------------------------------------------
    Name        : colBlend
    Description : Blend 2 colors together into a single color.
    Inputs      : c0 - color multiplied by factor
                  c1 - color multiplied by (1 - factor)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
color colBlend(color c0, color c1, real32 factor)
{
    udword red, green, blue, red1, green1, blue1;
    udword intFactor, oneFactor;

    intFactor = (udword)colRealToUbyte(factor);
    oneFactor = 255 - intFactor;
    red = colRed(c0);
    green = colGreen(c0);
    blue = colBlue(c0);
    red1 = colRed(c1);
    green1 = colGreen(c1);
    blue1 = colBlue(c1);
    return(colRGB(((red * intFactor) + (red1 * oneFactor)) >> 8,
                  ((green * intFactor) + (green1 * oneFactor)) >> 8,
                  ((blue * intFactor) + (blue1 * oneFactor)) >> 8));
}

/*-----------------------------------------------------------------------------
    Name        : colBlend
    Description : blends 2 colors
    Inputs      : ca - color a, cb - color b, t - lerp parameter
    Outputs     :
    Return      : blended color
----------------------------------------------------------------------------*/
/*
color colBlend(color ca, color cb, real32 t)
{
    uword ra, ga, ba;
    uword rb, gb, bb;
    uword r, g, b;
    ra = colRed(ca);
    ga = colGreen(ca);
    ba = colBlue(ca);
    rb = colRed(cb);
    gb = colGreen(cb);
    bb = colBlue(cb);
    r = cparam(ra, rb, t);
    g = cparam(ga, gb, t);
    b = cparam(ba, bb, t);
    return colRGB(r,g,b);
}
*/
/*-----------------------------------------------------------------------------
    Name        : colMultiplyClamped
    Description : multiply a color by a floating-point factor clamping to prevent overflow
    Inputs      :
    Outputs     :
    Return      : multiplied color
----------------------------------------------------------------------------*/
color colMultiplyClamped(color c, real32 factor)
{
    udword red, green, blue;
    udword intFactor;

    if (factor < 0.0f)
    {
        intFactor = 0;
    }
    else if (factor > 1.0f)
    {
        intFactor = 255;
    }
    else
    {
        intFactor = (udword)(factor * 255.0f);
    }
    red = (colRed(c) * intFactor) >> 8;
    green = (colGreen(c) * intFactor) >> 8;
    blue = (colBlue(c) * intFactor) >> 8;
    return(colRGB(red, green, blue));
}


