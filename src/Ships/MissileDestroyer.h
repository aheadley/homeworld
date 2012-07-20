/*=============================================================================
    Name    : MissileDestroyer.h
    Purpose : Definitions for MissileDestroyer

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MISSILE_DESTROYER_H
#define ___MISSILE_DESTROYER_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    real32 lasttimeRegeneratedMissiles;
    real32 lasttimeFiredVolley;
    real32 lasttimeDidSpecialTargeting;
    sdword curTargetIndex;
    sdword volleyState;
} MissileDestroyerSpec;

#define VOLLEY_BEGIN        10
#define VOLLEY_NOT_BEGIN    20
/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader MissileDestroyerHeader;

#endif //___MISSILE_DESTROYER_H

