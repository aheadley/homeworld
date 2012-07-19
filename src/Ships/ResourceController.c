/*=============================================================================
    Name    : ResourceController.c
    Purpose : Specifics for the ResourceController ship

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "ResourceController.h"
#include "statscript.h"
#include "shipselect.h"
#include "aiship.h"
#include "collision.h"
#include "dock.h"
#include "universe.h"
#include "commandlayer.h"
#include "repaircorvette.h"

typedef struct
{
    real32 repairApproachDistance;
} ResourceControllerStatics;

ResourceControllerStatics ResourceControllerStatic;

ResourceControllerStatics ResourceControllerStaticRace1;
ResourceControllerStatics ResourceControllerStaticRace2;


scriptStructEntry RCStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(ResourceControllerStatic.repairApproachDistance), (udword) &(ResourceControllerStatic) },

    { NULL,NULL,0,0 }
};


void ResourceControllerStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    ResourceControllerStatics *resourcestat = (statinfo->shiprace == R1) ? &ResourceControllerStaticRace1 : &ResourceControllerStaticRace2;

    statinfo->custstatinfo = resourcestat;
    scriptSetStruct(directory,filename,RCStaticScriptTable,(ubyte *)resourcestat);
}

bool ResourceControllerSpecialTarget(Ship *ship, void *custom)
{
    ResourceControllerStatics *rcstat = (ResourceControllerStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets;

    targets = (SelectAnyCommand *)custom;
    if(targets->numTargets == 0)
            return TRUE;
    //fix later: sort from closest to furthest in command layer once!
    //also need to remove non-ships from selection

    return(refuelRepairShips(ship, targets,rcstat->repairApproachDistance));

}

CustShipHeader ResourceControllerHeader =
{
    ResourceController,
    sizeof(ResourceControllerSpec),
    ResourceControllerStaticInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ResourceControllerSpecialTarget,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

