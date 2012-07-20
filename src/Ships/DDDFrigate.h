/*=============================================================================
    Name    : DDDFrigate.h
    Purpose : Definitions for DDDFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DDD_FRIGATE_H
#define ___DDD_FRIGATE_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

#define MAX_NUM_DRONES      24

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    udword DDDstate;
    real32 lasttimeRegenerated;
    ShipPtr DronePtrs[MAX_NUM_DRONES];
} DDDFrigateSpec;

typedef struct
{
    real32 attackRange;
    real32 tooCloseRange;
    real32 internalRegenerateRate;
    real32 externalRegenerateRate;
    real32 droneDeploymentRange;
} DDDFrigateStatics;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader DDDFrigateHeader;

/*=============================================================================
    Public functions:
=============================================================================*/

//void DDDFrigateRemoveShipReferences(Ship *ship,Ship *shiptoremove);
//void DDDFrigateDied(Ship *ship);
void DDDFrigateMakeSureItCanGuard(Ship *ship);
void DDDFrigateMakeReadyForHyperspace(Ship *ship);
void DDDFrigateJustDisabled(Ship *ship);
void DDDFrigateSwitchSides(Ship *dddf,sdword player);
void DDDFrigateDockAllDronesInstantly(Ship *ship);

#endif //___DDD_FRIGATE_H

