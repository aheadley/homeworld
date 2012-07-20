/*=============================================================================
    Name    : P3StandardShip.c
    Purpose : Specifics for the P3 Ship's

    Created 5/07/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "P3StandardShip.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    real32 shipGunRange[NUM_TACTICS_TYPES];
    real32 shipTooCloseRange[NUM_TACTICS_TYPES];
} P3StandardShipStatics;

P3StandardShipStatics P3MegashipStatic;
P3StandardShipStatics P3DestroyerStatic;
P3StandardShipStatics P3FrigateStatic;

void P3StandardShipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P3StandardShipStatics *shipstat=NULL;

    switch (statinfo->shiptype)
    {
        case P3Destroyer :
            shipstat = &P3DestroyerStatic;
        break;
        case P3Frigate :
            shipstat = &P3FrigateStatic;
        break;
        case P3Megaship :
            shipstat = &P3MegashipStatic;
        break;
    }

    dbgAssert(shipstat != NULL);

    statinfo->custstatinfo = shipstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        shipstat->shipGunRange[i] = statinfo->bulletRange[i] * 0.9f;
        shipstat->shipTooCloseRange[i] = statinfo->minBulletRange[i] * 0.8f;
    }
}

void P3StandardShipAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P3StandardShipStatics *shipstat = (P3StandardShipStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,shipstat->shipGunRange[ship->tacticstype],shipstat->shipTooCloseRange[ship->tacticstype]);
}

void P3StandardShipAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader P3StandardShipHeader =
{
    (ShipType)-1,
    sizeof(P3StandardShipSpec),
    P3StandardShipStaticInit,
    NULL,
    NULL,
    NULL,
    P3StandardShipAttack,
    DefaultShipFire,
    P3StandardShipAttackPassive,
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

