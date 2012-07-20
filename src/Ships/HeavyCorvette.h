/*=============================================================================
    Name    : HeavyCorvette.h
    Purpose : Definitions for HeavyCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___HEAVY_CORVETTE_H
#define ___HEAVY_CORVETTE_H

#include "Types.h"
#include "SpaceObj.h"
#include "Attack.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
    sdword burstState;
    sdword cooldown;
    vector burstFireVector;
    real32 burstChargeState;
    real32 burstChargeState2;
    real32 bulletLifeTime;
    Effect *chargeEffect;
} HeavyCorvetteSpec;

/*=============================================================================
    Public data:
=============================================================================*/
void heavyCorvetteOrderChanged(Ship *ship);

extern CustShipHeader HeavyCorvetteHeader;

#endif //___HEAVY_CORVETTE_H

