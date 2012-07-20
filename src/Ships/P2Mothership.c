/*=============================================================================
    Name    : P2Mothership.c
    Purpose : Specifics for the P2Mothership

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "P2Mothership.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"

typedef struct
{
    real32 p2mothershipGunRange[NUM_TACTICS_TYPES];
    real32 p2mothershipTooCloseRange[NUM_TACTICS_TYPES];
} P2MothershipStatics;

P2MothershipStatics P2MothershipStatic;

void P2MothershipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P2MothershipStatics *mothershipstat = &P2MothershipStatic;

    statinfo->custstatinfo = mothershipstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        mothershipstat->p2mothershipGunRange[i] = statinfo->bulletRange[i];
        mothershipstat->p2mothershipTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void P2MothershipAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P2MothershipStatics *motherstat = (P2MothershipStatics *)shipstaticinfo->custstatinfo;
    real32 toocloserange = motherstat->p2mothershipTooCloseRange[ship->tacticstype];

    // currently only correct if we are attacking a mothership class ship - otherwise we don't mind it ramming it
    if ((target->objtype == OBJ_ShipType) && (((Ship *)target)->staticinfo->shipclass == CLASS_Mothership))
    {
        // correct for P2 Mothership size - the laser is at the end of the ship, which is really long so we should
        // add the radius of the P2 Mothership to correct

        toocloserange += shipstaticinfo->staticheader.staticCollInfo.collspheresize;
    }

    attackStraightForward(ship,target,motherstat->p2mothershipGunRange[ship->tacticstype],toocloserange);
}

void P2MothershipAttackPassive(Ship *ship,Ship *target,bool rotate)
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

CustShipHeader P2MothershipHeader =
{
    P2Mothership,
    sizeof(P2MothershipSpec),
    P2MothershipStaticInit,
    NULL,
    NULL,
    NULL,
    P2MothershipAttack,
    DefaultShipFire,
    P2MothershipAttackPassive,
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

