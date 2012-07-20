/*=============================================================================
    Name    : AdvanceSupportFrigate.c
    Purpose : Specifics for the AdvanceSupportFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "AdvanceSupportFrigate.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "ShipSelect.h"
#include "AIShip.h"
#include "Collision.h"
#include "Dock.h"
#include "Universe.h"
#include "CommandLayer.h"
#include "RepairCorvette.h"


typedef struct
{
    real32 asfGunRange[NUM_TACTICS_TYPES];
    real32 asfTooCloseRange[NUM_TACTICS_TYPES];
    real32 repairApproachDistance;
} AdvanceSupportFrigateStatics;

AdvanceSupportFrigateStatics    AdvanceSupportFrigateStatic;

AdvanceSupportFrigateStatics AdvanceSupportFrigateStaticRace1;
AdvanceSupportFrigateStatics AdvanceSupportFrigateStaticRace2;

scriptStructEntry ASFStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(AdvanceSupportFrigateStatic.repairApproachDistance), (udword) &(AdvanceSupportFrigateStatic) },

    { NULL,NULL,0,0 }
};


void AdvanceSupportFrigateStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    AdvanceSupportFrigateStatics *asfstat = (statinfo->shiprace == R1) ? &AdvanceSupportFrigateStaticRace1 : &AdvanceSupportFrigateStaticRace2;

    statinfo->custstatinfo = asfstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        asfstat->asfGunRange[i] = statinfo->bulletRange[i];
        asfstat->asfTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    scriptSetStruct(directory,filename,ASFStaticScriptTable,(ubyte *)asfstat);

}

void AdvanceSupportFrigateAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    AdvanceSupportFrigateStatics *asfstat = (AdvanceSupportFrigateStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,asfstat->asfGunRange[ship->tacticstype],asfstat->asfTooCloseRange[ship->tacticstype]);
}

void AdvanceSupportFrigateAttackPassive(Ship *ship,Ship *target,bool rotate)
{
    if ((rotate) & ((bool)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
    {
        attackPassive(ship,target);
    }
}

bool AdvacedSupportFrigateSpecialTarget(Ship *ship,void *custom)
{
    AdvanceSupportFrigateStatics *asfstat = (AdvanceSupportFrigateStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets;
    targets = (SelectAnyCommand *)custom;
    return(refuelRepairShips(ship, targets,asfstat->repairApproachDistance));
}

CustShipHeader AdvanceSupportFrigateHeader =
{
    AdvanceSupportFrigate,
    sizeof(AdvanceSupportFrigateSpec),
    AdvanceSupportFrigateStaticInit,
    NULL,
    NULL,
    NULL,
    AdvanceSupportFrigateAttack,
    DefaultShipFire,
    AdvanceSupportFrigateAttackPassive,
    NULL,
    AdvacedSupportFrigateSpecialTarget,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
