/*=============================================================================
    Name    : Carrier.c
    Purpose : Specifics for the Carrier

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "Carrier.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "ShipSelect.h"
#include "RepairCorvette.h"
#include "SalCapCorvette.h"
#include "SaveGame.h"
#include "Universe.h"
#include "CommandLayer.h"
#include "ConsMgr.h"

typedef struct
{
    real32 carrierGunRange[NUM_TACTICS_TYPES];
    real32 carrierTooCloseRange[NUM_TACTICS_TYPES];
    real32 repairApproachDistance;
} CarrierStatics;

CarrierStatics CarrierStatic;

CarrierStatics CarrierStaticRace1;
CarrierStatics CarrierStaticRace2;

scriptStructEntry CStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(CarrierStatic.repairApproachDistance), (udword) &(CarrierStatic) },

    { NULL,NULL,0,0 }
};

void CarrierStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    CarrierStatics *carrstat = (statinfo->shiprace == R1) ? &CarrierStaticRace1 : &CarrierStaticRace2;

    statinfo->custstatinfo = carrstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        carrstat->carrierGunRange[i] = statinfo->bulletRange[i];
        carrstat->carrierTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    scriptSetStruct(directory,filename,CStaticScriptTable,(ubyte *)carrstat);

}
void CarrierInit(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;

    for(i=0;i<MAX_NUM_DROP;i++)
    {
        spec->dropstate[i] = 0;
        spec->droptarget[i] = NULL;
        spec->dockindex[i]=0;
    }

}
void CarrierAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    CarrierStatics *carrstat = (CarrierStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,carrstat->carrierGunRange[ship->tacticstype],carrstat->carrierTooCloseRange[ship->tacticstype]);
}

void CarrierAttackPassive(Ship *ship,Ship *target,bool rotate)
{
#if 0
    if ((rotate) & ((bool)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
#endif
        attackPassive(ship,target);
}

bool CarrierSpecialTarget(Ship *ship,void *custom)
{
    CarrierStatics *cstat = (CarrierStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets;
    targets = (SelectAnyCommand *)custom;
    return(refuelRepairShips(ship, targets,cstat->repairApproachDistance));
}

void CarrierHouseKeep(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;

    for(i=0;i<MAX_NUM_DROP;i++)
    {
        if(spec->droptarget[i] != NULL)
        {
            if(DropTargetInShip(ship,&spec->dropstate[i],spec->droptarget[i],&spec->dockindex[i]))
            {
                spec->dropstate[i] = 0;
                spec->droptarget[i] = NULL;
            }
        }
    }

    for(i=0;i<spec->CAP_NumInBuildCue;)
    {
        spec->CAPTimeToBuildShip[i]-= universe.phystimeelapsed;
        if(spec->CAPTimeToBuildShip[i] < 0.0f)
        {
            Ship *newship;

            if (GetShipStaticInfoSafe(spec->CAPshiptype[i],spec->CAPshiprace[i]))
            {
                //time for a captured ship to pop out
                if ((multiPlayerGame) && (ship->playerowner->playerIndex==sigsPlayerIndex))
                {
                    shiplagtotals[spec->CAPshiptype[i]]++;
                }

                newship = clCreateShip(&universe.mainCommandLayer,
                        spec->CAPshiptype[i],
                        spec->CAPshiprace[i],
                        ship->playerowner->playerIndex,
                        ship);
                newship->colorScheme = spec->CAPcolorScheme[i];
            }

            spec->CAP_NumInBuildCue--;
            spec->CAPTimeToBuildShip[i] = spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue];
            spec->CAPshiptype[i] = spec->CAPshiptype[spec->CAP_NumInBuildCue];
            spec->CAPshiprace[i] = spec->CAPshiprace[spec->CAP_NumInBuildCue];
            spec->CAPcolorScheme[i] = spec->CAPcolorScheme[spec->CAP_NumInBuildCue];
            continue;
        }
        i++;
    }

}

void CarrierRemoveShipReferences(Ship *ship,Ship *shiptoremove)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        if(shiptoremove == (Ship *) spec->droptarget[i])
        {
            if(ship->dockInfo != NULL)
            {
                if(ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy)
                    ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy--;
            }
            spec->droptarget[i] = NULL;
            break;
        }
    }
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"
void Carrier_PreFix(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        spec->droptarget[i] = (SpaceObjRotImpTargGuidanceShipDerelict*)SpaceObjRegistryGetID((SpaceObj *)spec->droptarget[i]);
    }
}

void Carrier_Fix(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        spec->droptarget[i] = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetObj((sdword)spec->droptarget[i]);
    }
}
#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"


void CarrierDied(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {

        if(spec->droptarget[i] != NULL)
        {
            if(spec->droptarget[i]->objtype == OBJ_ShipType)
            {
                //undisabled and make selectable
                if(!bitTest(((Ship *)spec->droptarget[i])->specialFlags2,SPECIAL_2_DisabledForever))
                {
                    bitClear(spec->droptarget[i]->flags,SOF_Disabled);
                    bitClear(spec->droptarget[i]->specialFlags,SPECIAL_SalvagedTargetGoingIntoDockWithShip);
                    bitSet(spec->droptarget[i]->flags,SOF_Selectable);
                }
            }

        }
    }
}

CustShipHeader CarrierHeader =
{
    Carrier,
    sizeof(CarrierSpec),
    CarrierStaticInit,
    NULL,
    CarrierInit,
    NULL,
    CarrierAttack,
    DefaultShipFire,
    CarrierAttackPassive,
    NULL,
    CarrierSpecialTarget,
    CarrierHouseKeep,
    CarrierRemoveShipReferences,
    CarrierDied,
    Carrier_PreFix,
    NULL,
    NULL,
    Carrier_Fix
};

