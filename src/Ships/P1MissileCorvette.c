/*=============================================================================
    Name    : P1MissileCorvette.c
    Purpose : Specifics for the P1MissileCorvette

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "P1MissileCorvette.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "Universe.h"

typedef struct
{
    real32 missilecorvetteGunRange[NUM_TACTICS_TYPES];
    real32 missilecorvetteTooCloseRange[NUM_TACTICS_TYPES];
    real32 missileRegenerateTime;
    real32 missileVolleyTime;
    real32 missileLagVolleyTime;
    AttackSideStepParameters sidestepParameters;
} P1MissileCorvetteStatics;

P1MissileCorvetteStatics P1MissileCorvetteStatic;

scriptStructEntry P1MissileCorvetteScriptTable[] =
{
    { "MissileRegenerateTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStatic.missileRegenerateTime),(udword) &(P1MissileCorvetteStatic) },
    { "MissileVolleyTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStatic.missileVolleyTime),(udword) &(P1MissileCorvetteStatic) },
    { "MissileLagVolleyTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStatic.missileLagVolleyTime),(udword) &(P1MissileCorvetteStatic) },
    { NULL,NULL,0,0 }
};

void P1MissileCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P1MissileCorvetteStatics *corvstat = &P1MissileCorvetteStatic;

    memset(corvstat,sizeof(*corvstat),0);

    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);
    scriptSetStruct(directory,filename,P1MissileCorvetteScriptTable,(ubyte *)corvstat);

    statinfo->custstatinfo = corvstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        corvstat->missilecorvetteGunRange[i] = statinfo->bulletRange[i];
        corvstat->missilecorvetteTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void P1MissileCorvetteInit(Ship *ship)
{
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);

    spec->lasttimeRegeneratedMissiles = 0.0f;
    spec->lasttimeFiredVolley = 0.0f;
    spec->lasttimeDidSpecialTargeting = -100000.0f;
    spec->curTargetIndex = 0;
}

void P1MissileCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    P1MissileCorvetteStatics *corvstat = (P1MissileCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void P1MissileCorvetteAttackPassive(Ship *ship,Ship *target,bool rotate)
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

void P1MissileCorvetteHousekeep(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    P1MissileCorvetteStatics *mcorvettestat = (P1MissileCorvetteStatics *)shipstaticinfo->custstatinfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;

    if ((universe.totaltimeelapsed - spec->lasttimeDidSpecialTargeting) > mcorvettestat->missileLagVolleyTime)
    {
        if ((universe.totaltimeelapsed - spec->lasttimeRegeneratedMissiles) > mcorvettestat->missileRegenerateTime)
        {
            spec->lasttimeRegeneratedMissiles = universe.totaltimeelapsed;
            numGuns = ship->gunInfo->numGuns;

            // find missile launcher with least missiles, and give it a missile
            for (i=0,minIndex=-1,minMissiles=100000;i<numGuns;i++)
            {
                gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
                if (gunstatic->guntype == GUN_MissileLauncher)
                {
                    gun = &ship->gunInfo->guns[i];
                    if (gun->numMissiles < gunstatic->maxMissiles)
                    {
                        if (gun->numMissiles < minMissiles)
                        {
                            minMissiles = gun->numMissiles;
                            minIndex = i;
                        }
                    }
                }
            }

            if (minIndex != -1)
            {
                ship->gunInfo->guns[minIndex].numMissiles++;
            }
        }
    }
}

bool P1MissileCorvetteSpecialTarget(Ship *ship,void *custom)
{
    SelectAnyCommand *targets;
    sdword i;
    sdword numShipsToTarget;
    sdword numGuns;
    Gun *gun;
    GunStatic *gunstatic;
    SpaceObjRotImpTarg *target;
    ShipStaticInfo *shipstaticinfo;
    P1MissileCorvetteSpec *spec;
    P1MissileCorvetteStatics *mcorvettestat;
    bool firedSomeMissiles;

    spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    mcorvettestat = (P1MissileCorvetteStatics *)shipstaticinfo->custstatinfo;

    spec->lasttimeDidSpecialTargeting = universe.totaltimeelapsed;

    if ((universe.totaltimeelapsed - spec->lasttimeFiredVolley) > mcorvettestat->missileVolleyTime)
    {
        spec->lasttimeFiredVolley = universe.totaltimeelapsed;
    }
    else
    {
        return FALSE;
    }

    targets = (SelectAnyCommand *)custom;
    numShipsToTarget = targets->numTargets;

    if (numShipsToTarget == 0)
    {
        // have destroyed targets, so we are done
        spec->curTargetIndex = 0;
        return TRUE;
    }

    if (spec->curTargetIndex >= numShipsToTarget)
    {
        spec->curTargetIndex = 0;
    }

    numGuns = ship->gunInfo->numGuns;
    firedSomeMissiles = FALSE;
    for (i=0;i<numGuns;i++)
    {
        gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
        if (gunstatic->guntype == GUN_MissileLauncher)
        {
            gun = &ship->gunInfo->guns[i];
            if (gunHasMissiles(gun))
            {
                firedSomeMissiles = TRUE;

                target = targets->TargetPtr[spec->curTargetIndex];

                missileShoot(ship,gun,target);

                spec->curTargetIndex++;
                if (spec->curTargetIndex >= numShipsToTarget)
                {
                    spec->curTargetIndex = 0;
                }
            }
        }
    }

    if (firedSomeMissiles)
    {
        return FALSE;
    }
    else
    {
        // all empty of missiles, so we are done
        spec->curTargetIndex = 0;
        return TRUE;
    }
}

CustShipHeader P1MissileCorvetteHeader =
{
    P1MissileCorvette,
    sizeof(P1MissileCorvetteSpec),
    P1MissileCorvetteStaticInit,
    NULL,
    P1MissileCorvetteInit,
    NULL,
    P1MissileCorvetteAttack,
    DefaultShipFire,
    P1MissileCorvetteAttackPassive,
    NULL,
    P1MissileCorvetteSpecialTarget,
    P1MissileCorvetteHousekeep,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

