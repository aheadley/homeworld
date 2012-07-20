/*=============================================================================
    Name    : DDDFrigate.c
    Purpose : Specifics for the DDDFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "Drone.h"
#include "DDDFrigate.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "Formation.h"
#include "CommandLayer.h"
#include "Dock.h"
#include "AIShip.h"
#include "Collision.h"
#include "AITrack.h"
#include "SoundEvent.h"
#include "SaveGame.h"
#include "Randy.h"

#define DDDSTATE_ALLINSIDE  0
#define DDDSTATE_LAUNCHTHEM 1
#define DDDSTATE_LAUNCHING  2
#define DDDSTATE_ALLOUTSIDE 3
#define DDDSTATE_DOCKTHEM   4
#define DDDSTATE_DOCKING    5

DDDFrigateStatics DDDFrigateStaticRace1;
DDDFrigateStatics DDDFrigateStaticRace2;

scriptStructEntry DDDStaticScriptTable[] =
{
    { "attackRange",    scriptSetReal32CB, (udword) &(DDDFrigateStaticRace1.attackRange), (udword) &(DDDFrigateStaticRace1) },
    { "tooCloseRange",  scriptSetReal32CB, (udword) &(DDDFrigateStaticRace1.tooCloseRange), (udword) &(DDDFrigateStaticRace1) },
    { "internalRegenerateRate",  scriptSetReal32CB, (udword) &(DDDFrigateStaticRace1.internalRegenerateRate), (udword) &(DDDFrigateStaticRace1) },
    { "externalRegenerateRate",  scriptSetReal32CB, (udword) &(DDDFrigateStaticRace1.externalRegenerateRate), (udword) &(DDDFrigateStaticRace1) },
    { "droneDeploymentRange",  scriptSetReal32CB, (udword) &(DDDFrigateStaticRace1.droneDeploymentRange), (udword) &(DDDFrigateStaticRace1) },

    { NULL,NULL,0,0 }
};

void DDDFrigateStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    DDDFrigateStatics *dddstat = (statinfo->shiprace == R1) ? &DDDFrigateStaticRace1 : &DDDFrigateStaticRace2;

    statinfo->custstatinfo = dddstat;

    scriptSetStruct(directory,filename,DDDStaticScriptTable,(ubyte *)dddstat);
}

Ship *CreateDroneInside(Ship *ship,udword droneNumber)
{
    Ship *droneship;
    DroneSpec *dronespec;

    dbgAssert(ship->shiptype == DDDFrigate);

    droneship = univCreateShip(Drone,ship->shiprace,&ship->posinfo.position,ship->playerowner,1);

    gameStatsAddShip(droneship,ship->playerowner->playerIndex);

    bitClear(droneship->flags,SOF_Selectable);
    dronespec = (DroneSpec *)droneship->ShipSpecifics;
    dronespec->droneNumber = droneNumber;
    dronespec->droneState = DRONESTATE_DORMANT;

    dockPutShipInside(droneship,ship);

    return droneship;
}

void DDDFrigateInit(Ship *ship)
{
    // Create its Drones when the DDDFrigate is created
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;
    sdword i;

    spec->DDDstate = DDDSTATE_ALLINSIDE;
    spec->lasttimeRegenerated = 0.0f;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        spec->DronePtrs[i] = CreateDroneInside(ship,i);
    }
}

// later put this in the custshipheader?
void DDDFrigateRemoveShipReferences(Ship *ship,Ship *shiptoremove)
{
    sdword i;
    DDDFrigateSpec *spec;

    dbgAssert(ship->shiptype == DDDFrigate);

    if (shiptoremove->shiptype != Drone)
    {
        return;
    }

    spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        if (spec->DronePtrs[i] == shiptoremove)
        {
            spec->DronePtrs[i] = NULL;
        }
    }
}

// later put this in the custshipheader?
void DDDFrigateDied(Ship *ship)
{
    sdword i;
    DDDFrigateSpec *spec;
    Ship *deleteme;

    dbgAssert(ship->shiptype == DDDFrigate);

    spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        deleteme = spec->DronePtrs[i];

        if ((deleteme != NULL) && ((deleteme->flags & SOF_Dead) == 0))
        {
            if (deleteme->shiplink.belongto != NULL)
            {
                // ship is outside, so delete it
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
}

void DDDFrigateSwitchSides(Ship *dddf,sdword player)
{
    sdword i;
    DDDFrigateSpec *spec;
    Ship *ship;

    dbgAssert(dddf->shiptype == DDDFrigate);

    spec = (DDDFrigateSpec *)dddf->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        ship = spec->DronePtrs[i];

        if (ship != NULL)
        {
            ship->playerowner = &universe.players[player];
            if (player)
                ship->attributes |= ATTRIBUTES_Defector;
            else
                bitClear(ship->attributes,ATTRIBUTES_Defector);
        }
    }
}

bool DDDFrigateSpecialActivate(Ship *ship)
{
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    switch (spec->DDDstate)
    {
        case DDDSTATE_ALLINSIDE:
            spec->DDDstate = DDDSTATE_LAUNCHTHEM;
            speechEvent(ship, COMM_DDF_LaunchDrones, 0);
            break;

        case DDDSTATE_ALLOUTSIDE:
            spec->DDDstate = DDDSTATE_DOCKTHEM;
            speechEvent(ship, COMM_DDF_RetractDrones, 0);
            break;
    }

    return TRUE;
}

void LaunchDrone(Ship *ship,Ship *drone)
{
    DroneSpec *dronespec = (DroneSpec *)drone->ShipSpecifics;

    dbgAssert(ship->shiptype == DDDFrigate);
    dbgAssert(drone->shiptype == Drone);

    dockPrepareSingleShipForLaunch(drone,ship);
    dockInitShipForLaunch(drone);

    dronespec->droneState = DRONESTATE_LAUNCHING;

    InitShipAI(drone,TRUE);
}

void DDDFrigateHousekeep(Ship *ship)
{
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;
    DroneSpec *dronespec;
    DDDFrigateStatics *dddstat = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    real32 regeneraterate = (spec->DDDstate == DDDSTATE_ALLINSIDE) ? dddstat->internalRegenerateRate : dddstat->externalRegenerateRate;

    if ((universe.totaltimeelapsed - spec->lasttimeRegenerated) > regeneraterate)
    {
        udword i;

        spec->lasttimeRegenerated = universe.totaltimeelapsed;

        // regenerate a drone, if one has been destroyed
        for (i=0;i<MAX_NUM_DRONES;i++)
        {
            if (spec->DronePtrs[i] == NULL)
            {
                // found a dead drone, so let's create a new one.
                spec->DronePtrs[i] = CreateDroneInside(ship,i);

                switch (spec->DDDstate)
                {
                    case DDDSTATE_ALLINSIDE:    // all inside, so don't have to do anything
                    case DDDSTATE_DOCKTHEM:     // docking, so no need to launch and then dock
                    case DDDSTATE_DOCKING:      // docking, so no need to launch and then dock
                    case DDDSTATE_LAUNCHTHEM:   // going to launch everything, so no action needed
                        break;

                    case DDDSTATE_LAUNCHING:
                        // we just created a drone, which is inside.
                        // we need to prepare it so it can launch too.
                        LaunchDrone(ship,spec->DronePtrs[i]);
                        break;

                    case DDDSTATE_ALLOUTSIDE:
                        spec->DDDstate = DDDSTATE_LAUNCHTHEM;   // launch regenerated drone
                        break;
                }
                break;
            }
        }
    }

    switch (spec->DDDstate)
    {
        case DDDSTATE_ALLINSIDE:
            break;

        case DDDSTATE_LAUNCHTHEM:
        {
            // for all the drones inside,
            ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;
            Node *node = shipsInsideMe->insideList.head;
            InsideShip *insideShip;

            if (node == NULL)
            {
                // no Drones, so just return
                spec->DDDstate = DDDSTATE_ALLINSIDE;
                return;
            }

            soundEvent(ship, Ship_DroneLaunch);

            do
            {
                insideShip = (InsideShip *)listGetStructOfNode(node);

                LaunchDrone(ship,insideShip->ship);

                node = node->next;
            } while (node != NULL);

            spec->DDDstate = DDDSTATE_LAUNCHING;
        }
        break;

        case DDDSTATE_LAUNCHING:
        {
            // for all of the DDD's Drones,
            udword i;
            Ship *drone;
            bool alllaunched = TRUE;

            for (i=0;i<MAX_NUM_DRONES;i++)
            {
                drone = spec->DronePtrs[i];
                if (drone != NULL)
                {
                    dronespec = (DroneSpec *)drone->ShipSpecifics;

                    dbgAssert(dronespec->droneNumber == i);
                    switch (dronespec->droneState)
                    {
                        case DRONESTATE_LAUNCHING:
                            alllaunched = FALSE;
                            if (LaunchShipFromDDDF(drone,ship))
                            {
                                RemoveShipFromLaunching(drone);
                                dronespec->droneState = DRONESTATE_LAUNCHED;
                            }
                            break;

                        case DRONESTATE_LAUNCHED:
                            break;
                    }
                }
            }

            if (alllaunched)
            {
                SelectCommand *droneselect;
                SelectCommand selectone;
                sdword numShipsToLaunch = 0;

                droneselect = memAlloc(sizeofSelectCommand(MAX_NUM_DRONES),"DroneSelLaunch",0);

                for (i=0;i<MAX_NUM_DRONES;i++)
                {
                    drone = spec->DronePtrs[i];
                    if (drone != NULL)
                    {
                        dbgAssert(((DroneSpec *)drone->ShipSpecifics)->droneState == DRONESTATE_LAUNCHED);
                        droneselect->ShipPtr[numShipsToLaunch++] = drone;
                    }
                }

                if (numShipsToLaunch == 0)
                {
                    memFree(droneselect);       // all ships are launched, but there are none so consider them inside
                    spec->DDDstate = DDDSTATE_ALLINSIDE;
                    return;
                }

                droneselect->numShips = numShipsToLaunch;
                dbgAssert(numShipsToLaunch <= MAX_NUM_DRONES);

                selectone.ShipPtr[0] = ship;
                selectone.numShips = 1;

                // put all ships in formation, and guarding DDDF
                clFormation(&universe.mainCommandLayer,droneselect,SPHERE_FORMATION);
                clProtect(&universe.mainCommandLayer,droneselect,&selectone);

                memFree(droneselect);
                spec->DDDstate = DDDSTATE_ALLOUTSIDE;
            }
        }
        break;

        case DDDSTATE_ALLOUTSIDE:
            break;

        case DDDSTATE_DOCKTHEM:
        {
            sdword i;
            Ship *drone;
            SelectCommand *droneselect;
            sdword numShipsToDock = 0;
            DroneSpec *dronespec;

            droneselect = memAlloc(sizeofSelectCommand(MAX_NUM_DRONES),"DroneSelection",0);

            // remove drones from doing other stuff

            for (i=0;i<MAX_NUM_DRONES;i++)
            {
                drone = spec->DronePtrs[i];
                if (drone != NULL)
                {
                    dbgAssert(((DroneSpec *)drone->ShipSpecifics)->droneState == DRONESTATE_LAUNCHED);
                    droneselect->ShipPtr[numShipsToDock++] = drone;
                }
            }

            if (numShipsToDock == 0)
            {
                memFree(droneselect);
                spec->DDDstate = DDDSTATE_ALLINSIDE;
                return;
            }

            droneselect->numShips = numShipsToDock;
            dbgAssert(numShipsToDock <= MAX_NUM_DRONES);

            // remove ships from sphere formation and protecting
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,droneselect);

            // initialize each drone for docking
            for (i=0;i<numShipsToDock;i++)
            {
                drone = droneselect->ShipPtr[i];
                dronespec = (DroneSpec *)drone->ShipSpecifics;
                dockPrepareDroneForDocking(drone,ship);
                dronespec->droneState = DRONESTATE_DOCKING;
            }

            memFree(droneselect);
            spec->DDDstate = DDDSTATE_DOCKING;
        }
        break;

        case DDDSTATE_DOCKING:
        {
            sdword i;
            Ship *drone;
            DroneSpec *dronespec;
            bool allDocked = TRUE;

            // for each drone in DRONESTATE_Docking, call DroneDocksAtDDDF
            for (i=0;i<MAX_NUM_DRONES;i++)
            {
                drone = spec->DronePtrs[i];
                if (drone != NULL)
                {
                    dronespec = (DroneSpec *)drone->ShipSpecifics;
                    if (dronespec->droneState == DRONESTATE_DOCKING)
                    {
                        allDocked = FALSE;
                        if (DroneDocksAtDDDF(drone,ship))
                        {
                            soundEvent(ship, Ship_DroneAcquire);
                            RemoveShipFromDocking(drone);
                            dronespec->droneState = DRONESTATE_DORMANT;
                        }
                    }
                }
            }

            if (allDocked)
            {
                spec->DDDstate = DDDSTATE_ALLINSIDE;
            }
        }
        break;
    }
}

void DDDFrigateDockAllDronesInstantly(Ship *ship)
{
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;
    DroneSpec *dronespec;
    Ship *drone;
    sdword i;

    dbgAssert(ship->shiptype == DDDFrigate);

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        drone = spec->DronePtrs[i];
        if (drone != NULL)
        {
            dronespec = (DroneSpec *)drone->ShipSpecifics;

            switch (dronespec->droneState)
            {
                case DRONESTATE_DOCKING:
                    RemoveShipFromDocking(drone);
                    break;

                case DRONESTATE_LAUNCHING:
                    RemoveShipFromLaunching(drone);
                    break;
            }

            if (dronespec->droneState != DRONESTATE_DORMANT)
            {
                if (ship->shiplink.belongto)
                {
                    univRemoveShipFromOutside(drone);
                    dockPutShipInside(drone,ship);
                }
                dronespec->droneState = DRONESTATE_DORMANT;
            }
        }
    }

    spec->DDDstate = DDDSTATE_ALLINSIDE;
}

void DDDFrigateAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    DDDFrigateStatics *dddstatics = (DDDFrigateStatics *)shipstatic->custstatinfo;
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;
    vector trajectory;
    real32 range;
    bool inRange = FALSE;

    aishipGetTrajectory(ship,target,&trajectory);
    range = RangeToTarget(ship,target,&trajectory);

    if (range > dddstatics->attackRange)
    {
        // too far away, so fly in
        aishipFlyToShipAvoidingObjsWithVel(ship,target,AISHIP_FirstPointInDirectionFlying+AISHIP_PointInDirectionFlying+AISHIP_CarTurn+AISHIP_FastAsPossible,0.0f,&target->posinfo.velocity);
    }
    else if (range > dddstatics->tooCloseRange)
    {
        // about right range
        aishipFlyToShipAvoidingObjsWithVel(ship,target,AISHIP_FirstPointInDirectionFlying+AISHIP_PointInDirectionFlying+AISHIP_CarTurn+AISHIP_FastAsPossible,0.0f,&target->posinfo.velocity);
        inRange = TRUE;
    }
    else
    {
        // too close
        aitrackZeroVelocity(ship);
        inRange = TRUE;
    }

    if ((inRange) && (spec->DDDstate == DDDSTATE_ALLINSIDE))
    {
        DDDFrigateSpecialActivate(ship);
    }
}

void DDDFrigatePassiveAttack(Ship *ship,Ship *target,bool rotate)
{
    DDDFrigateStatics *dddstatics = (DDDFrigateStatics *)ship->staticinfo->custstatinfo;
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;
    vector trajectory;
    real32 range;

    if (spec->DDDstate == DDDSTATE_ALLINSIDE)
    {
        aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);
        range = RangeToTarget(ship,(SpaceObjRotImpTarg *)target,&trajectory);

        if (range <= dddstatics->attackRange)
        {
            DDDFrigateSpecialActivate(ship);
        }
    }
}

void DDDFrigateMakeSureItCanGuard(Ship *ship)
{
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    dbgAssert(ship->shiptype == DDDFrigate);
    if (spec->DDDstate == DDDSTATE_ALLINSIDE)
    {
        DDDFrigateSpecialActivate(ship);
    }
}

void DDDFrigateMakeReadyForHyperspace(Ship *ship)
{
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    dbgAssert(ship->shiptype == DDDFrigate);
    if (spec->DDDstate == DDDSTATE_ALLOUTSIDE)
    {
        DDDFrigateSpecialActivate(ship);
    }
}

#define DRONE_DISABLED_DEATHTIME_LOW    2.0f
#define DRONE_DISABLED_DEATHTIME_HIGH   5.0f
void DDDFrigateJustDisabled(Ship *ship)
{
    //DDDFrigateMakeReadyForHyperspace(ship);
    sdword i;
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        if(spec->DronePtrs[i] != NULL)
        {
            spec->DronePtrs[i]->deathtime = universe.totaltimeelapsed += frandombetween(DRONE_DISABLED_DEATHTIME_LOW,DRONE_DISABLED_DEATHTIME_HIGH);
        }
    }

}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void DDDFrigate_PreFix(Ship *ship)
{
    sdword i;
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        spec->DronePtrs[i] = SpaceObjRegistryGetID((SpaceObj *)spec->DronePtrs[i]);
    }
}

void DDDFrigate_Fix(Ship *ship)
{
    sdword i;
    DDDFrigateSpec *spec = (DDDFrigateSpec *)ship->ShipSpecifics;

    for (i=0;i<MAX_NUM_DRONES;i++)
    {
        spec->DronePtrs[i] = SpaceObjRegistryGetShip((sdword)spec->DronePtrs[i]);
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

CustShipHeader DDDFrigateHeader =
{
    DDDFrigate,
    sizeof(DDDFrigateSpec),
    DDDFrigateStaticInit,
    NULL,
    DDDFrigateInit,
    NULL,
    DDDFrigateAttack,
    NULL,
    DDDFrigatePassiveAttack,
    DDDFrigateSpecialActivate,
    NULL,
    DDDFrigateHousekeep,
    DDDFrigateRemoveShipReferences,
    DDDFrigateDied,
    DDDFrigate_PreFix,
    NULL,
    NULL,
    DDDFrigate_Fix
};

