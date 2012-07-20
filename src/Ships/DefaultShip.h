/*=============================================================================
    Name    : DefaultShip.h
    Purpose : Definitions for DefaultShip

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DEFAULT_SHIP_H
#define ___DEFAULT_SHIP_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    sdword dummy;
} DefaultShipSpec;

/*=============================================================================
    Public data:
=============================================================================*/

void DefaultShipFire(Ship *ship,SpaceObjRotImpTarg *target);
void DefaultShipAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist);

extern CustShipHeader DefaultShipHeader;
extern CustShipHeader DoNothingShipHeader;
extern CustShipHeader StationaryGunHeader;
extern CustShipHeader MiningBaseHeader;

#endif //___DEFAULT_SHIP_H

