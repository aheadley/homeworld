/*=============================================================================
    Name    : MultiGunCorvette.c
    Purpose : Specifics for the MultiGunCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "MultiGunCorvette.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    AttackSideStepParameters sidestepParameters;
} MultiGunCorvetteStatics;

MultiGunCorvetteStatics MultiGunCorvetteStaticRace1;
MultiGunCorvetteStatics MultiGunCorvetteStaticRace2;

void MultiGunCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    MultiGunCorvetteStatics *corvstat = (statinfo->shiprace == R1) ? &MultiGunCorvetteStaticRace1 : &MultiGunCorvetteStaticRace2;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;
}

void MultiGunCorvetteInit(Ship *ship)
{
    MultiGunCorvetteSpec *spec = (MultiGunCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);
}

void MultiGunCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    MultiGunCorvetteSpec *spec = (MultiGunCorvetteSpec *)ship->ShipSpecifics;
    MultiGunCorvetteStatics *corvstat = (MultiGunCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void MultiGunCorvetteAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader MultiGunCorvetteHeader =
{
    MultiGunCorvette,
    sizeof(MultiGunCorvetteSpec),
    MultiGunCorvetteStaticInit,
    NULL,
    MultiGunCorvetteInit,
    NULL,
    MultiGunCorvetteAttack,
    DefaultShipFire,
    MultiGunCorvetteAttackPassive,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

