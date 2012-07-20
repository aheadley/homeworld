/*=============================================================================
    Name    : autolod.h
    Purpose : routines for maintaining a more even framerate

    Created 9/8/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _AUTOLOD_H
#define _AUTOLOD_H

#include "Types.h"

void alodStartup(void);
void alodReset(void);
void alodShutdown(void);
void alodSetMinMax(real32 minScale, real32 maxScale);
real32 alodGetMin(void);
real32 alodGetMax(void);
void alodSetTargetPolys(udword targetPolys, udword polyDelta);
void alodGetTargetPolys(udword* targetPolys, udword* polyDelta);
void alodSetPolys(udword polys);
void alodIncPolys(udword polys);
void alodEnable(bool enable);
void alodAdjustScaleFactor(void);
udword alodGetPolys(void);
bool alodGetPanic(void);
void alodSetPanic(bool panic);

#endif
