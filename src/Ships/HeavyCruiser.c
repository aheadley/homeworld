/*=============================================================================
    Name    : HeavyCruiser.c
    Purpose : Specifics for the HeavyCruiser

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "HeavyCruiser.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    real32 heavycruiserGunRange[NUM_TACTICS_TYPES];
    real32 heavycruiserTooCloseRange[NUM_TACTICS_TYPES];
} HeavyCruiserStatics;

HeavyCruiserStatics HeavyCruiserStaticRace1;
HeavyCruiserStatics HeavyCruiserStaticRace2;

void HeavyCruiserStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    HeavyCruiserStatics *heavycruiserstat = (statinfo->shiprace == R1) ? &HeavyCruiserStaticRace1 : &HeavyCruiserStaticRace2;

    statinfo->custstatinfo = heavycruiserstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        heavycruiserstat->heavycruiserGunRange[i] = statinfo->bulletRange[i];
        heavycruiserstat->heavycruiserTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void HeavyCruiserAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    HeavyCruiserStatics *heavycruiserstat = (HeavyCruiserStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,heavycruiserstat->heavycruiserGunRange[ship->tacticstype],heavycruiserstat->heavycruiserTooCloseRange[ship->tacticstype]);
}

void HeavyCruiserAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader HeavyCruiserHeader =
{
    HeavyCruiser,
    sizeof(HeavyCruiserSpec),
    HeavyCruiserStaticInit,
    NULL,
    NULL,
    NULL,
    HeavyCruiserAttack,
    DefaultShipFire,
    HeavyCruiserAttackPassive,
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

