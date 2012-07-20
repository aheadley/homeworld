/*=============================================================================
    Name    : SalCapCorvette.h
    Purpose : Definitions for Salvage Capture Corvette

    Created 11/5/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SAL_CAP_CORVETTE_H
#define ___SAL_CAP_CORVETTE_H

#include "Types.h"
#include "SpaceObj.h"
#include "Attack.h"
#include "ShipSelect.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
    sdword salvageState;               //sal/cap corv aistate for salvaging
    SpaceObjRotImpTargGuidanceShipDerelict *target;
    Ship *dockwith;
    sdword dockindex;
    sdword salvageIndex;
    sdword salvageAttributes;
    real32 timeCounter;
    real32 getTechTime;
    sdword tractorBeam;
    sdword noAvoid;
    real32 noDamageTargetTime;
    sdword groupme;
	SpaceObjRotImpTargGuidanceShipDerelict *noDamageTarget;

} SalCapCorvetteSpec;

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    real32 HealthThreshold;
    real32 healthRemovedPerSecond;
    real32 getTechTime;
    real32 flyToDistance;
    real32 maxPushingVelocitySingle;
    real32 noLightClampingDistance;
} SalCapCorvetteStatics;
extern SalCapCorvetteStatics SalCapCorvetteStatic;

//Defs for Salvaging State...
#define SALVAGE_AT_GET_TECH 0x01

#define SAL_WL_FLYTOCONEORIGIN      7
/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader SalCapCorvetteHeader;
extern CustShipHeader JunkYardDawgHeader;

void SalCapCorvette_RegisterExtraSpaceObjs(Ship *ship);

void SalCapOrderChangedCleanUp(Ship *ship);
void salcapUpdateClampedShip(Ship *ship);
void salCapRemoveDerelictReferences(Ship *ship,Derelict *d);
void SalCapDropTarget(Ship *ship);
void salCapHarvestTarget(SpaceObjRotImpTargGuidanceShipDerelict *target,Ship *dockwith);

void salCapExtraSpecialOrderCleanUp(SelectCommand *selection,udword ordertype,Ship *dockwith,SelectCommand *targets);
bool DropTargetInShip(Ship *dockwith,sdword *targetDepotState, SpaceObjRotImpTargGuidanceShipDerelict *target,sdword *dockindex);

#if RND_VISUALIZATION
void dockDrawSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *obj);
#endif
void salCapCleanUpCloakingTarget(Ship *ship, Ship *shiptoremove);
bool salCapAreEnoughSalvagersTargettingThisTarget(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target);
void salCapClearTechBool();
bool isThereAnotherTargetForMe(Ship *ship,SelectAnyCommand *targets);
sdword salCapNeedBig(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target);
#endif
//___SAL_CAP_CORVETTE_H

