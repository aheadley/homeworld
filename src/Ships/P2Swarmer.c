/*=============================================================================
    Name    : P2Swarmer.c
    Purpose : Specifics for the P2Swarmer

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "P2Swarmer.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    AttackSideStepParameters sidestepParameters;
} P2SwarmerStatics;

P2SwarmerStatics P2SwarmerStaticRace1;
P2SwarmerStatics P2SwarmerStaticRace2;


void P2SwarmerStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    P2SwarmerStatics *corvstat = (statinfo->shiprace == R1) ? &P2SwarmerStaticRace1 : &P2SwarmerStaticRace2;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;
}

void P2SwarmerInit(Ship *ship)
{
    P2SwarmerSpec *spec = (P2SwarmerSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);

}

void P2SwarmerAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    P2SwarmerSpec *spec = (P2SwarmerSpec *)ship->ShipSpecifics;
    P2SwarmerStatics *corvstat = (P2SwarmerStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void P2SwarmerAttackPassive(Ship *ship,Ship *target,bool rotate)
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


CustShipHeader P2SwarmerHeader =
{
    P2Swarmer,
    sizeof(P2SwarmerSpec),
    P2SwarmerStaticInit,
    NULL,
    P2SwarmerInit,
    NULL,
    P2SwarmerAttack,
    DefaultShipFire,
    P2SwarmerAttackPassive,
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

