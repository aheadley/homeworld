/*=============================================================================
    Name    : damage.h
    Purpose : things necessary for showing ship damage

    Created 7/30/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _DAMAGE_H
#define _DAMAGE_H

#include "SpaceObj.h"

#ifndef HW_Release

#define DMG_VERBOSE_LEVEL   1

#else

#define DMG_VERBOSE_LEVEL   0

#endif

void dmgStartup(void);
void dmgShutdown(void);
void dmgShipThink(Ship* ship);
void dmgStopEffect(Ship* ship, sdword level);
void dmgStopSingleEffect(Effect* effect);
void dmgForgetEffects(Ship* ship);
void dmgGetLights(Ship* ship);
void dmgClearLights(Ship* ship);

#endif
