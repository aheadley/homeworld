/*=============================================================================
    Name    : Star3d.h
    Purpose : Definitions for Star3d.c

    Created 6/21/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___STAR3D_H
#define ___STAR3D_H

#include "types.h"
#include "vector.h"
#include "color.h"

/*=============================================================================
    TYPES:
=============================================================================*/

typedef struct
{
    vector position;
    real32 brightness;
    color c;
    ubyte r,g,b,pad;
} Star3d;

typedef struct
{
    udword Num3dStars;
    real32 innerlimit;
    real32 outerlimit;
    Star3d Stars[1];        // will dynamically allocate
} Star3dInfo;

/*=============================================================================
    Data:
=============================================================================*/
extern sdword starMaxColor;
extern sdword starMinColor;
extern sdword starRedVariance;
extern sdword starGreenVariance;
extern sdword starBlueVariance;

/*=============================================================================
    Functions:
=============================================================================*/

Star3dInfo *star3dInit(udword numStars,real32 innerlimit,real32 outerlimit);
void star3dClose(Star3dInfo *star3dinfo);

#endif //___STAR3D_H
