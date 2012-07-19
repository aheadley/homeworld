/*=============================================================================
    Name    : Quantize.h
    Purpose : Interface to module quantize.c

    Created 9/30/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___QUANTIZE_H
#define ___QUANTIZE_H

#include "types.h"
#include "color.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifdef HW_Debug

#define Q_ERROR_CHECKING           1           //general error checking
#define Q_VERBOSE_LEVEL            0           //control specific output code
#define Q_PRINT_COLOR_ALLOCS       0

#else //HW_Debug

#define Q_ERROR_CHECKING           0           //general error checking
#define Q_VERBOSE_LEVEL            0           //control specific output code
#define Q_PRINT_COLOR_ALLOCS       0

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define Q_QueueLength               1024

#define Q_SkipIndices               1

#define Q_TeamColorSteps            16
#define Q_TeamColorDivisor          (256 / Q_TeamColorSteps)

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for passing parameters to the image quantization
typedef struct
{
    sdword width, height;
    color *RGBSource;
    ubyte *indexDest;
    ubyte *teamColor;
}
quantdata;

/*=============================================================================
    Functions:
=============================================================================*/
//quantize a single image
sdword qRGBImageQuantize(ubyte *destIndex, color *destPalette, color *source, sdword width, sdword height, sdword flags);

//quantize a set of images into a single palette:
void qQuantizeReset(void);
void qRGBImageAdd(ubyte *destIndex, color *source, sdword width, sdword height, ubyte *teamColor);
sdword qRGBQuantizeQueue(color *destPalette);

#endif //___QUANTIZE_H
