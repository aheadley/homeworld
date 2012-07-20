/*=============================================================================
    Name    : Drone.c
    Purpose : Specifics for the Drone (part of DDDF)

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "DefaultShip.h"
#include "Drone.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"

typedef struct
{
    udword dummy;
} DroneStatics;

DroneStatics DroneStaticRace1;
DroneStatics DroneStaticRace2;

void DroneStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    DroneStatics *dronestat = (statinfo->shiprace == R1) ? &DroneStaticRace1 : &DroneStaticRace2;

    statinfo->custstatinfo = dronestat;
}

void DroneAttackPassive(Ship *ship,Ship *target,bool rotate)
{
    DroneSpec *spec = (DroneSpec *)ship->ShipSpecifics;

    if (spec->droneState != DRONESTATE_LAUNCHED)
    {
        return;     // only allow fully launched drones to shoot
    }

    if ((rotate) & ((bool)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
    {
        attackPassive(ship,target);
    }
}

CustShipHeader DroneHeader =
{
    Drone,
    sizeof(DroneSpec),
    DroneStaticInit,
    NULL,
    NULL,
    NULL,
    NULL,
    DefaultShipFire,
    DroneAttackPassive,
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

