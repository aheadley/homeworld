/*=============================================================================
    Name    : P1Mothership.c
    Purpose : Specifics for the P1Mothership

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "P1Mothership.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    real32 p1mothershipGunRange[NUM_TACTICS_TYPES];
    real32 p1mothershipTooCloseRange[NUM_TACTICS_TYPES];
} P1MothershipStatics;

P1MothershipStatics P1MothershipStatic;

void P1MothershipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P1MothershipStatics *mothershipstat = &P1MothershipStatic;

    statinfo->custstatinfo = mothershipstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        mothershipstat->p1mothershipGunRange[i] = statinfo->bulletRange[i];
        mothershipstat->p1mothershipTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void P1MothershipAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P1MothershipStatics *motherstat = (P1MothershipStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,motherstat->p1mothershipGunRange[ship->tacticstype],motherstat->p1mothershipTooCloseRange[ship->tacticstype]);
}

void P1MothershipAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader P1MothershipHeader =
{
    P1Mothership,
    sizeof(P1MothershipSpec),
    P1MothershipStaticInit,
    NULL,
    NULL,
    NULL,
    P1MothershipAttack,
    DefaultShipFire,
    P1MothershipAttackPassive,
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

