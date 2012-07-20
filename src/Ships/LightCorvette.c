/*=============================================================================
    Name    : LightCorvette.c
    Purpose : Specifics for the Light Corvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "LightCorvette.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    AttackSideStepParameters sidestepParameters;
} LightCorvetteStatics;

LightCorvetteStatics LightCorvetteStaticRace1;
LightCorvetteStatics LightCorvetteStaticRace2;


void LightCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    LightCorvetteStatics *corvstat = (statinfo->shiprace == R1) ? &LightCorvetteStaticRace1 : &LightCorvetteStaticRace2;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;
}

void LightCorvetteInit(Ship *ship)
{
    LightCorvetteSpec *spec = (LightCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);

}

void LightCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    LightCorvetteSpec *spec = (LightCorvetteSpec *)ship->ShipSpecifics;
    LightCorvetteStatics *corvstat = (LightCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void LightCorvetteAttackPassive(Ship *ship,Ship *target,bool rotate)
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


CustShipHeader LightCorvetteHeader =
{
    LightCorvette,
    sizeof(LightCorvetteSpec),
    LightCorvetteStaticInit,
    NULL,
    LightCorvetteInit,
    NULL,
    LightCorvetteAttack,
    DefaultShipFire,
    LightCorvetteAttackPassive,
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

