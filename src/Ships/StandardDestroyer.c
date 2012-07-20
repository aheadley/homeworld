/*=============================================================================
    Name    : StandardDestroyer.c
    Purpose : Specifics for the StandardDestroyer

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "StandardDestroyer.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    real32 standarddestroyerGunRange[NUM_TACTICS_TYPES];
    real32 standarddestroyerTooCloseRange[NUM_TACTICS_TYPES];
} StandardDestroyerStatics;

StandardDestroyerStatics StandardDestroyerStaticRace1;
StandardDestroyerStatics StandardDestroyerStaticRace2;

void StandardDestroyerStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    StandardDestroyerStatics *sdestroyerstat = (statinfo->shiprace == R1) ? &StandardDestroyerStaticRace1 : &StandardDestroyerStaticRace2;

    statinfo->custstatinfo = sdestroyerstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        sdestroyerstat->standarddestroyerGunRange[i] = statinfo->bulletRange[i];
        sdestroyerstat->standarddestroyerTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void StandardDestroyerAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    StandardDestroyerStatics *sdestroyerstat = (StandardDestroyerStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,sdestroyerstat->standarddestroyerGunRange[ship->tacticstype],sdestroyerstat->standarddestroyerTooCloseRange[ship->tacticstype]);
}

void StandardDestroyerAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader StandardDestroyerHeader =
{
    StandardDestroyer,
    sizeof(StandardDestroyerSpec),
    StandardDestroyerStaticInit,
    NULL,
    NULL,
    NULL,
    StandardDestroyerAttack,
    DefaultShipFire,
    StandardDestroyerAttackPassive,
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

