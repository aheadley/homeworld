/*=============================================================================
    Name    : NavLights.h
    Purpose : Header file for NavLights.c

    Created 6/21/1997 by agarden
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef __NAVLIGHTS_H
#define __NAVLIGHTS_H

#include "SpaceObj.h"

void RenderNAVLights(Ship *ship);
void navLightStaticInfoDelete(NAVLightStaticInfo *staticInfo);

#endif

