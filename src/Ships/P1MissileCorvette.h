/*=============================================================================
    Name    : P1MissileCorvette.h
    Purpose : Definitions for Carrier

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___P1_MISSILE_CORVETTE_H
#define ___P1_MISSILE_CORVETTE_H

#include "Types.h"
#include "SpaceObj.h"
#include "Attack.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
    real32 lasttimeRegeneratedMissiles;
    real32 lasttimeFiredVolley;
    real32 lasttimeDidSpecialTargeting;
    sdword curTargetIndex;
} P1MissileCorvetteSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader P1MissileCorvetteHeader;

#endif //___P1_MISSILE_CORVETTE_H

