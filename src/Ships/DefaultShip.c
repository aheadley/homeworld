/*=============================================================================
    Name    : DefaultShip.c
    Purpose : Specifics for the DefaultShip (placeholder ship)

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "Gun.h"
#include "Attack.h"
#include "Universe.h"
#include "DefaultShip.h"
#include "GenericInterceptor.h"
#include "UnivUpdate.h"

typedef struct
{
    udword dummy;
} DefaultShipStatics;

typedef struct
{
    bool dontRecurseKill;
} MiningBaseSpec;

void DefaultShipFire(Ship *ship,SpaceObjRotImpTarg *target)
{
    GunInfo *guninfo = ship->gunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;

    if (guninfo == NULL)
    {
        return;
    }

    numGuns = guninfo->numGuns;

    for (i=0;i<numGuns;i++)
    {
        gun = &guninfo->guns[i];
        if (gunCanShoot(ship, gun))
        {
            switch (gun->gunstatic->guntype)
            {
                case GUN_MissileLauncher:
                case GUN_MineLauncher:
                    if (gunHasMissiles(gun))
                    {
                        missileShoot(ship,gun,target);
                    }
                    break;

                case GUN_NewGimble:
                case GUN_Gimble:
                case GUN_Fixed:
                    gunShoot(ship,gun,target);
                    break;

                default:
                    dbgAssert(FALSE);

            }
        }
    }
}

void DefaultShipAttack(Ship *ship,SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    if (ship->gunInfo == NULL)
    {
        dbgMessagef("\nWARNING: %s tried to attack but has no guns",ShipTypeToStr(ship->shiptype));
        return;
    }

    attackStraightForward(ship,target,shipstaticinfo->bulletRange[ship->tacticstype],shipstaticinfo->minBulletRange[ship->tacticstype]*0.9f);
}

void DefaultShipAttackPassive(Ship *ship,Ship *target,bool rotate)
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

void DefaultShipAttackStationary(Ship *ship,SpaceObjRotImpTarg *target, real32 maxdist)
{
    DefaultShipAttackPassive(ship,(Ship *)target,TRUE);
}

void MiningBaseInit(Ship *ship)
{
    MiningBaseSpec *spec = (MiningBaseSpec *)ship->ShipSpecifics;

    spec->dontRecurseKill = FALSE;
}

void MiningBaseDied(Ship *ship)
{
    Ship *deleteme;
    Node *node;
    MiningBaseSpec *spec = (MiningBaseSpec *)ship->ShipSpecifics;

    dbgAssert(ship->shiptype == MiningBase);
    if (ship->flags & SOF_NISShip)
    {
        return;
    }

    if (spec->dontRecurseKill)
    {
        return;
    }

    // first pass: Set all other MiningBases to dontRecurseKill so we don't get weird recursion

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        deleteme = (Ship *)listGetStructOfNode(node);
        dbgAssert(deleteme->objtype == OBJ_ShipType);

        if ((deleteme->shiptype == MiningBase) && (deleteme != ship) && ((deleteme->flags & (SOF_Dead|SOF_NISShip)) == 0))
        {
            spec = (MiningBaseSpec *)deleteme->ShipSpecifics;
            spec->dontRecurseKill = TRUE;
        }
    }

    // delete all other mining bases

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        deleteme = (Ship *)listGetStructOfNode(node);
        dbgAssert(deleteme->objtype == OBJ_ShipType);

        if ((deleteme->shiptype == MiningBase) && (deleteme != ship) && ((deleteme->flags & (SOF_Dead|SOF_NISShip)) == 0))
        {
            deleteme->howDidIDie = ship->howDidIDie;
            deleteme->whoKilledMe = ship->whoKilledMe;
            univDeleteDeadShip(deleteme,EDT_AccumDamage);
            listRemoveNode(&deleteme->shiplink);
            listRemoveNode(&deleteme->impactablelink);

#if ETG_DISABLEABLE
            if (!etgEffectsEnabled)
            {
                univWipeShipOutOfExistence(deleteme);
            }
#endif
        }
    }
}

CustShipHeader DefaultShipHeader =
{
    (ShipType)-1,
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    DefaultShipAttack,
    DefaultShipFire,
    DefaultShipAttackPassive,
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

CustShipHeader DoNothingShipHeader =
{
    (ShipType)-1,
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
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

CustShipHeader MiningBaseHeader =
{
    (ShipType)-1,
    sizeof(MiningBaseSpec),
    NULL,
    NULL,
    MiningBaseInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MiningBaseDied,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader StationaryGunHeader =
{
    (ShipType)-1,
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    DefaultShipAttackStationary,
    DefaultShipFire,
    DefaultShipAttackPassive,
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

