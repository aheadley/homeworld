
#include <stdlib.h>
#include "Types.h"
#include "FastMath.h"
#include "Memory.h"
#include "Debug.h"
#include "Vector.h"
#include "SpaceObj.h"
#include "AITrack.h"
#include "AIShip.h"
#include "ResCollect.h"
#include "CommandLayer.h"
#include "StatScript.h"
#include "UnivUpdate.h"
#include "Universe.h"
#include "SoundEvent.h"
#include "Collision.h"
#include "Physics.h"
#include "Tutor.h"
#include "Randy.h"
#include "Clouds.h"
#include "Nebulae.h"
#include "SinglePlayer.h"

#ifndef HW_Release

#ifdef gshaw
#define DEBUG_COLLECTRESOURCES
#endif

#endif

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 COLLECTRESOURCE_DIST = 400.0f;
real32 MAX_RESOURCE_VELOCITY_TO_CHASE = 50.0f;
real32 MAX_RESOURCE_VELOCITY_TO_CHASE_LEVEL6 = 50.0f;
real32 ASTEROID_HARVEST_RANGE = 400.0f;
real32 ASTEROID_MAXHARVEST_RANGE = 600.0f;
udword CIRCLE_HARVEST_ASTEROIDS_BIGGEREQUAL_THAN = 2;
real32 CIRCLE_RIGHT_VELOCITY = 20.0f;
real32 CIRCLE_RIGHT_THRUST = 0.1f;
real32 DUSTCLOUD_RU_DAMAGE_RATIO;
real32 ASTEROID_RU_DAMAGE_RATIO[NUM_ASTEROIDTYPES] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
real32 ASTEROIDBREAK_LO_VELOCITY = -200.0f;
real32 ASTEROIDBREAK_HI_VELOCITY = 200.0f;
real32 ASTEROIDBREAK_RADIUS_SCALE = 0.7f;

real32 ASTEROIDBREAK_LO_ROTATE = -1.0f;
real32 ASTEROIDBREAK_HI_ROTATE = 1.0f;

real32 ASTEROID_LO_ROTATE[NUM_ASTEROIDTYPES] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
real32 ASTEROID_HI_ROTATE[NUM_ASTEROIDTYPES] = { 1.0f , 1.0f , 1.0f , 1.0f ,  1.0f };

udword ASTEROID_ROTATE_PROB = 150;

real32 DUSTCLOUD_RCOLLECTOR_BLOWUP_SCALE = 1.0f;

real32 COLLECTRESOURCE_ZONE = 2000.0f;

// regrow resources stuff
udword REGROW_RESOURCES_CHECK_RATE = 255;
udword REGROW_RESOURCES_CHECK_FRAME = 86;
real32 REGROW_RATE_MODIFIER[REGROWRATE_Max+1] = { 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };

sdword regenerateRUrate = 255;

sdword regenerateRUsIfUnder = 400;
sdword regenerateRUs = 25;

udword CHECK_HARV_OUTOFWORLD_RESOURCE_RATE = 127;
udword CHECK_SHIP_OUTOFWORLD_RATE = 127;

scriptEntry ResCollectTweaks[] =
{
    makeEntry(COLLECTRESOURCE_DIST,scriptSetReal32CB),
    makeEntry(MAX_RESOURCE_VELOCITY_TO_CHASE,scriptSetReal32CB),
    makeEntry(MAX_RESOURCE_VELOCITY_TO_CHASE_LEVEL6,scriptSetReal32CB),
    makeEntry(ASTEROID_HARVEST_RANGE,scriptSetReal32CB),
    makeEntry(ASTEROID_MAXHARVEST_RANGE,scriptSetReal32CB),
    makeEntry(CIRCLE_HARVEST_ASTEROIDS_BIGGEREQUAL_THAN,scriptSetUdwordCB),
    makeEntry(CIRCLE_RIGHT_VELOCITY,scriptSetReal32CB),
    makeEntry(CIRCLE_RIGHT_THRUST,scriptSetReal32CB),
    makeEntry(ASTEROID_RU_DAMAGE_RATIO[Asteroid0],scriptSetReal32CB),
    makeEntry(ASTEROID_RU_DAMAGE_RATIO[Asteroid1],scriptSetReal32CB),
    makeEntry(ASTEROID_RU_DAMAGE_RATIO[Asteroid2],scriptSetReal32CB),
    makeEntry(ASTEROID_RU_DAMAGE_RATIO[Asteroid3],scriptSetReal32CB),
    makeEntry(ASTEROID_RU_DAMAGE_RATIO[Asteroid4],scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RU_DAMAGE_RATIO,scriptSetReal32CB),
    makeEntry(ASTEROIDBREAK_LO_VELOCITY,scriptSetReal32CB),
    makeEntry(ASTEROIDBREAK_HI_VELOCITY,scriptSetReal32CB),
    makeEntry(ASTEROIDBREAK_LO_ROTATE,scriptSetReal32CB),
    makeEntry(ASTEROIDBREAK_HI_ROTATE,scriptSetReal32CB),
    makeEntry(ASTEROIDBREAK_RADIUS_SCALE,scriptSetReal32CB),
    makeEntry(ASTEROID_LO_ROTATE[Asteroid0],scriptSetReal32CB),
    makeEntry(ASTEROID_HI_ROTATE[Asteroid0],scriptSetReal32CB),
    makeEntry(ASTEROID_LO_ROTATE[Asteroid1],scriptSetReal32CB),
    makeEntry(ASTEROID_HI_ROTATE[Asteroid1],scriptSetReal32CB),
    makeEntry(ASTEROID_LO_ROTATE[Asteroid2],scriptSetReal32CB),
    makeEntry(ASTEROID_HI_ROTATE[Asteroid2],scriptSetReal32CB),
    makeEntry(ASTEROID_LO_ROTATE[Asteroid3],scriptSetReal32CB),
    makeEntry(ASTEROID_HI_ROTATE[Asteroid3],scriptSetReal32CB),
    makeEntry(ASTEROID_LO_ROTATE[Asteroid4],scriptSetReal32CB),
    makeEntry(ASTEROID_HI_ROTATE[Asteroid4],scriptSetReal32CB),
    makeEntry(ASTEROID_ROTATE_PROB,scriptSetUdwordCB),
    makeEntry(DUSTCLOUD_RCOLLECTOR_BLOWUP_SCALE,scriptSetReal32CB),
    makeEntry(COLLECTRESOURCE_ZONE,scriptSetReal32CB),
    makeEntry(REGROW_RESOURCES_CHECK_RATE,scriptSetUdwordCB),
    makeEntry(REGROW_RESOURCES_CHECK_FRAME,scriptSetUdwordCB),
    makeEntry(REGROW_RATE_MODIFIER[0],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[1],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[2],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[3],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[4],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[5],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[6],scriptSetReal32CB),
    makeEntry(REGROW_RATE_MODIFIER[7],scriptSetReal32CB),
    makeEntry(regenerateRUrate,scriptSetSdwordCB),
    makeEntry(regenerateRUsIfUnder,scriptSetSdwordCB),
    makeEntry(regenerateRUs,scriptSetSdwordCB),
    makeEntry(CHECK_HARV_OUTOFWORLD_RESOURCE_RATE,scriptSetUdwordCB),
    makeEntry(CHECK_SHIP_OUTOFWORLD_RATE,scriptSetUdwordCB),
    endEntry
};

sdword RESOURCE_NOT_ACCESSIBLE_SECONDS = 60;

/*=============================================================================
    Private functions prototypes:
=============================================================================*/

void InitShipForResourceCollection(Ship *ship,Resource *resource);

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : ResourceMovingTooFast
    Description : returns TRUE if resource is moving too fast to be harvested
    Inputs      : resource
    Outputs     :
    Return      : returns TRUE if resource is moving too fast to be harvested
----------------------------------------------------------------------------*/
bool ResourceMovingTooFast(Resource *resource)
{
    real32 maxvelocitychase = MAX_RESOURCE_VELOCITY_TO_CHASE;

    if (singlePlayerGame && (singlePlayerGameInfo.currentMission == 6))
    {
        maxvelocitychase = MAX_RESOURCE_VELOCITY_TO_CHASE_LEVEL6;
    }

    if ((ABS(resource->posinfo.velocity.x) > maxvelocitychase) ||
        (ABS(resource->posinfo.velocity.y) > maxvelocitychase) ||
        (ABS(resource->posinfo.velocity.z) > maxvelocitychase) )
    {
#ifdef DEBUG_COLLECTRESOURCES
            dbgMessage("\nResource moving too fast. Picking different one...");
#endif
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : ResourceAlreadyBeingHarvested
    Description : returns TRUE if resource is already being targeted for harvesting
    Inputs      :
    Outputs     :
    Return      : returns TRUE if resource is already being targeted for harvesting
----------------------------------------------------------------------------*/
bool ResourceAlreadyBeingHarvested(struct CommandLayer *comlayer,struct CommandToDo *IAmThisCommand,Resource *resource)
{
    Node *curnode = comlayer->todolist.head;
    ResourceStaticInfo *resourcestaticinfo = resource->staticinfo;
    CommandToDo *command;

    if (!resourcestaticinfo->harvestableByMultipleShips)
    {
        while (curnode != NULL)
        {
            command = (CommandToDo *)listGetStructOfNode(curnode);

            if ((command->ordertype.order == COMMAND_COLLECTRESOURCE) && (command != IAmThisCommand))
            {
                if (command->collect.resource == resource)
                {
    #ifdef DEBUG_COLLECTRESOURCES
                dbgMessage("\nResource already targeted. Picking different one...");
    #endif
                    return TRUE;
                }
            }

            curnode = curnode->next;
        }
        return FALSE;
    }
    else
    {
        // harvestable my multiple ships, so find how many ships are targeting it
        udword targetedby = 0;

        while (curnode != NULL)
        {
            command = (CommandToDo *)listGetStructOfNode(curnode);

            if ((command->ordertype.order == COMMAND_COLLECTRESOURCE) && (command != IAmThisCommand))
            {
                if (command->collect.resource == resource)
                {
                    targetedby++;
                }
            }

            curnode = curnode->next;
        }

        if (targetedby >= resourcestaticinfo->harvestableByMultipleShips)
        {
    #ifdef DEBUG_COLLECTRESOURCES
            dbgMessage("\nResource being targeted by too many. Picking different one...");
    #endif
            return TRUE;
        }
        return FALSE;
    }
}

Resource *rescollectFindNearestResource(Ship *ship,struct CommandToDo *command)
{
    Resource *resource;

    if (command->collect.resourceVolumeSize)
    {
        resource = univFindNearestResource(ship,command->collect.resourceVolumeSize,&command->collect.resourceVolumePosition);
        if (resource == NULL)
        {
            command->collect.resourceVolumeSize = 0.0f;     // no more resources in this volume, so don't keep looking in it
            goto novolumelook;
    }
}
    else
    {
novolumelook:
        resource = univFindNearestResource(ship,0,NULL);
    }

    return resource;
}

void ChangeSingleShipToCollectResource(struct CommandToDo *command)
{
    Ship *ship = command->selection->ShipPtr[0];
    Resource *resource;
    dbgAssert(command->selection->numShips == 1);

    resource = rescollectFindNearestResource(ship,command);
    InitShipForResourceCollection(ship,resource);

    command->ordertype.order = COMMAND_COLLECTRESOURCE;
    command->ordertype.attributes = 0;
    command->collect.resource = resource;

    PrepareOneShipForCommand(ship,command,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : clCollectResource
    Description : command for a ship to collect a resource
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource)
{
    CommandToDo *newcommand;
    SelectCommand *selection;
    sdword numShips;
    Ship *ship;
    sdword i;

    numShips = selectcom->numShips;

    if (numShips == 0)
    {
        dbgMessage("\nNo ships to get resources");
        return;
    }

#ifdef DEBUG_COLLECTRESOURCES
    dbgMessage("\nReceived command to harvest");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    // add COMMAND_COLLECTRESOURCE commands for each resource collector:
    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        InitShipForResourceCollection(ship,resource);

        newcommand = memAlloc(sizeof(CommandToDo),"ToDo",0);
        InitCommandToDo(newcommand);

        selection = memAlloc(sizeof(SelectCommand),"ToDoSelection",0);
        selection->numShips = 1;
        selection->ShipPtr[0] = ship;

        newcommand->selection = selection;
        newcommand->ordertype.order = COMMAND_COLLECTRESOURCE;
        newcommand->ordertype.attributes = 0;

        newcommand->collect.resource = resource;
        newcommand->collect.resourceVolumeSize = 0.0f;
        vecZeroVector(newcommand->collect.resourceVolumePosition);
        if (resource)
        {
            newcommand->collect.resourceVolumeSize = COLLECTRESOURCE_ZONE;
            newcommand->collect.resourceVolumePosition = resource->posinfo.position;
        }

        PrepareShipsForCommand(newcommand,TRUE);

        listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    }
}

void HandleAsteroidScaling(Asteroid *asteroid)
{
    AsteroidStaticInfo *asteroidstatic = (AsteroidStaticInfo *)asteroid->staticinfo;
    if (asteroidstatic->asteroidCanShrink)
    {
        real32 oldscaling = asteroid->scaling;

        asteroid->scaling = (real32)asteroid->resourcevalue / (real32)asteroidstatic->resourcevalue;
        if (asteroid->scaling < asteroidstatic->asteroidMinShrink)
        {
            asteroid->scaling = asteroidstatic->asteroidMinShrink;
        }

        if (asteroid->scaling != oldscaling)
        {
            MakeNewAsteroidStaticInfo(asteroid);
        }
    }
}

void HandleDustCloudScaling(DustCloud* dustcloud)
{
    cloudSystem* csys;
    DustCloudStaticInfo* dustcloudstatic = (DustCloudStaticInfo*)dustcloud->staticinfo;

    if (dustcloudstatic->dustcloudCanShrink)
    {
        real32 oldscaling = dustcloud->scaling;

        dustcloud->scaling = (real32)dustcloud->resourcevalue / (real32)dustcloudstatic->resourcevalue;
        if (dustcloud->scaling > 1.0f)
        {
            dustcloud->scaling = 1.0f;
        }

        if (dustcloud->scaling < dustcloudstatic->dustcloudMinShrink)
        {
            dustcloud->scaling = dustcloudstatic->dustcloudMinShrink;
        }

        if (dustcloud->scaling != oldscaling)
        {
            MakeNewDustCloudStaticInfo(dustcloud);
        }

        csys = (cloudSystem*)dustcloud->stub;
        if (csys != NULL)
        {
            csys->healthFactor = dustcloud->scaling;
        }
    }
}

void HandleGasCloudScaling(GasCloud *gascloud)
{
    GasCloudStaticInfo *gascloudstatic = (GasCloudStaticInfo *)gascloud->staticinfo;
    if (gascloudstatic->gascloudCanShrink)
    {
        real32 oldscaling = gascloud->scaling;

        gascloud->scaling = (real32)gascloud->resourcevalue / (real32)gascloudstatic->resourcevalue;
        if (gascloud->scaling < gascloudstatic->gascloudMinShrink)
        {
            gascloud->scaling = gascloudstatic->gascloudMinShrink;
        }

        if (gascloud->scaling != oldscaling)
        {
            MakeNewGasCloudStaticInfo(gascloud);
        }
    }
}

void HandleNebulaScaling(Nebula* nebula)
{
    NebulaStaticInfo* nebulastatic;
    real32 factor;

    nebulastatic = (NebulaStaticInfo*)nebula->staticinfo;
    factor = (real32)nebula->resourcevalue / (real32)nebulastatic->resourcevalue;

    nebFadeAttachedTendrils((nebChunk*)nebula->stub, factor);
}

/*-----------------------------------------------------------------------------
    Name        : DustCloudTakesDamage
    Description : applies damage to dustcloud, causing it to lose RU's and possibly
                  die and wreak havok as it does so.  charging takes place here, too
    Inputs      : asteroid, damagetaken, targetWasAlive
    Outputs     :
    Return      : returns TRUE if should delete dustcloud
----------------------------------------------------------------------------*/
bool DustCloudTakesDamage(DustCloud* dustcloud, sdword damagetaken, bool targetWasAlive)
{
    sdword RUdamage;

    if (!targetWasAlive)
    {
        return FALSE;       // target already dead, so ignore
    }

    RUdamage = (sdword) (damagetaken * DUSTCLOUD_RU_DAMAGE_RATIO);
    dustcloud->resourcevalue -= RUdamage;

    if (dustcloud->resourcevalue <= 0)
    {
        return TRUE;        // all RU's have been sapped out.  Just kill asteroid without splitting it up.
    }

    if (dustcloud->health <= 0)
    {
        //clouds can't break.  instead they do something cool upon losing all health
        return TRUE;
    }
    else
    {
        HandleDustCloudScaling(dustcloud);
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : DustCloudChargesUp
    Description : ion cannon fire charges up a dustcloud; this fn performs the charging
    Inputs      : dustcloud, damagetaken, targetWasAlive
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DustCloudChargesUp(DustCloud* dustcloud, sdword damagetaken, bool targetWasAlive)
{
    cloudSystem* csys = (cloudSystem*)dustcloud->stub;

    if (!targetWasAlive || csys == NULL)
    {
        return;
    }

    csys->charge += 0.5f * ((real32)damagetaken / (real32)csys->maxhealth);
    if (csys->charge > 1.0f)
    {
        bitSet(csys->flags, LIGHTNING_DISCHARGING);
        csys->charge = 1.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : BreakAsteroidUp
    Description : Breaks the asteroid up into little fragments
    Inputs      : asteroid
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void BreakAsteroidUp(Asteroid *asteroid)
{
    AsteroidStaticInfo *asteroidstatic = (AsteroidStaticInfo *)asteroid->staticinfo;
    Asteroid *newasteroid;
    AsteroidType asteroidtype;
    udword j;
    udword numAsteroidsToMake;
    vector randpos;
    real32 randposrange;
    vector originalvelocity;
    vector randomvelocity;

    dbgAssert(asteroid->objtype == OBJ_AsteroidType);

    // asteroid has been destroyed.  Split it up into multiple asteroids
    if (asteroidstatic->asteroidCanBreak)
    {
        randposrange = asteroidstatic->staticheader.staticCollInfo.collspheresize * ASTEROIDBREAK_RADIUS_SCALE;
        originalvelocity = asteroid->posinfo.velocity;

        for (asteroidtype=Asteroid0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
        {
            numAsteroidsToMake = asteroidstatic->breakinto[asteroidtype];
            for (j=0;j<numAsteroidsToMake;j++)
            {
                randpos.x = frandombetween(-randposrange,randposrange);
                randpos.y = frandombetween(-randposrange,randposrange);
                randpos.z = frandombetween(-randposrange,randposrange);
                vecAddTo(randpos,asteroid->collInfo.collPosition);
                newasteroid = univAddAsteroid(asteroidtype,&randpos);
                randomvelocity.x = frandombetween(ASTEROIDBREAK_LO_VELOCITY,ASTEROIDBREAK_HI_VELOCITY);
                randomvelocity.y = frandombetween(ASTEROIDBREAK_LO_VELOCITY,ASTEROIDBREAK_HI_VELOCITY);
                randomvelocity.z = frandombetween(ASTEROIDBREAK_LO_VELOCITY,ASTEROIDBREAK_HI_VELOCITY);
                vecAdd(newasteroid->posinfo.velocity,randomvelocity,originalvelocity);
                if (asteroid->asteroidtype >= Asteroid2)
                if (((udword)(gamerand() & 255)) <= ASTEROID_ROTATE_PROB)
                {
                    newasteroid->rotinfo.rotspeed.x = frandombetween(ASTEROIDBREAK_LO_ROTATE,ASTEROIDBREAK_HI_ROTATE);
                    newasteroid->rotinfo.rotspeed.y = frandombetween(ASTEROIDBREAK_LO_ROTATE,ASTEROIDBREAK_HI_ROTATE);
                    newasteroid->rotinfo.rotspeed.z = frandombetween(ASTEROIDBREAK_LO_ROTATE,ASTEROIDBREAK_HI_ROTATE);
                }
                newasteroid->attributes = asteroid->attributes;     // inherit attributes
                bitClear(newasteroid->attributes,ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage);
                if (asteroidtype != Asteroid0) univAddObjToRenderListIf((SpaceObj *)newasteroid,(SpaceObj *)asteroid);     // add to render list if parent is in render list
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : AsteroidTakesDamage
    Description : applies damage to asteroid, causing it to use RU's, and possibly
                  die and split into multiple chunks.
    Inputs      : asteroid, damagetaken, targetWasAlive
    Outputs     :
    Return      : returns TRUE if should delete asteroid
----------------------------------------------------------------------------*/
bool AsteroidTakesDamage(Asteroid *asteroid,sdword damagetaken,bool targetWasAlive)
{
    sdword RUdamage;

    if (!targetWasAlive)
    {
        return FALSE;       // target already dead, so ignore
    }

    dbgAssert(asteroid->asteroidtype < NUM_ASTEROIDTYPES);

    RUdamage = (sdword) (damagetaken * ASTEROID_RU_DAMAGE_RATIO[asteroid->asteroidtype]);
    asteroid->resourcevalue -= RUdamage;

    if (asteroid->resourcevalue <= 0)
    {
        return TRUE;        // all RU's have been sapped out.  Just kill asteroid
    }

    if (asteroid->health <= 0)
    {
        return TRUE;
    }

    HandleAsteroidScaling(asteroid);
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : CollectResources
    Description : if it is a ship-resource collision, the resource will be collected
                  assuming there is room in the ship for the resource.
    Inputs      :
    Outputs     :
    Return      : TRUE if ship collected resource
----------------------------------------------------------------------------*/
bool CollectResources(Ship *ship,Resource *resource)
{
    ShipStaticInfo *shipstatic;
    sdword eat;

    bool resourceWasAlive;

    dbgAssert(ship->objtype == OBJ_ShipType);

    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    if (shipstatic->maxresources == 0)
    {
        return FALSE;       // not able to collect resources, so return FALSE
    }

    if (ship->resources >= shipstatic->maxresources)
    {
        return FALSE;       // ship full, can't collect resources
    }

    if ((universe.univUpdateCounter & shipstatic->harvestRate) == 0)
    {
        resourceWasAlive = ((resource->flags & SOF_Dead) == 0);

        eat = (shipstatic->harvestAmount <= resource->resourcevalue) ? shipstatic->harvestAmount : resource->resourcevalue;
        ship->resources += eat;

        if (ship->resources > shipstatic->maxresources)
        {                                                          // over eated
            eat -= (ship->resources - shipstatic->maxresources);
            ship->resources = shipstatic->maxresources;
            speechEvent(ship, STAT_ResCol_Full, 0);
        }

        resource->resourcevalue -= eat;

        if ((resource->resourcevalue <= 0) && (resourceWasAlive))
        {
            AddResourceToDeleteResourceList(resource);
            bitSet(ship->specialFlags, SPECIAL_Finished_Resource);
            bitClear(ship->specialFlags, SPECIAL_Resourcing);
        }
        else
        {
            if (resource->objtype == OBJ_AsteroidType)
            {
                HandleAsteroidScaling((Asteroid *)resource);
            }
            else if (resource->objtype == OBJ_DustType)
            {
                HandleDustCloudScaling((DustCloud *)resource);
            }
            else if (resource->objtype == OBJ_GasType)
            {
                HandleGasCloudScaling((GasCloud *)resource);
            }
            else if (resource->objtype == OBJ_NebulaType)
            {
                HandleNebulaScaling((Nebula*)resource);
            }
        }

    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : TurnHarvestEffectOff
    Description : turns the harvest effect off
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void TurnHarvestEffectOff(Ship *ship)
{
    dbgAssert(ship->rceffect != NULL);

    if (bitTest(((etgeffectstatic *)ship->rceffect->staticinfo)->specialOps, ESO_SelfDeleting))
    {                                                       //if the effect will delete itself
        ((real32 *)ship->rceffect->variable)[ETG_ResourceDurationParam] =
            ship->rceffect->timeElapsed;                            //time it out
    }
    else
    {                                                       //else it's a free-running effect... delete it
        etgEffectDelete(ship->rceffect);
        univRemoveObjFromRenderList((SpaceObj *)ship->rceffect);
        listDeleteNode(&ship->rceffect->objlink);
    }

    soundEventStop(ship->soundevent.specialHandle);

    ship->rceffect = NULL;
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void TurnOffAnyResourcingEffects(struct CommandToDo *todo)
{
    Ship *ship = todo->selection->ShipPtr[0];
    dbgAssert(todo->selection->numShips == 1);

    if (ship->rceffect != NULL)
    {
        TurnHarvestEffectOff(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : TurnHarvestEffectOn
    Description : turns the harvest effect on
    Inputs      : ship, resource, heading, dist
                  etgLOD - which harvesting effect to use
                  resourceRadius - radius of the resource we are harvesting
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void TurnHarvestEffectOn(Ship *ship,Resource *resource,vector *trajectory, real32 resourceRadius, etglod *etgLOD)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    ResNozzleStatic *resNozzleStatic = shipstatic->resNozzleStatic;
    vector nozzleposition;
    matrix nozzlecoordsys;
    vector nozzletrajectory;
    real32 nozzletrajectorydist;
    etgeffectstatic *stat;
    sdword LOD;
    udword intLength;
    udword intWidth;
    real32 temp;

    dbgAssert(resNozzleStatic);

    matMultiplyMatByVec(&nozzleposition,&ship->rotinfo.coordsys,&resNozzleStatic->position);

    vecSub(nozzletrajectory,*trajectory,nozzleposition);

    nozzletrajectorydist = fsqrt(vecMagnitudeSquared(nozzletrajectory));
    vecDivideByScalar(nozzletrajectory,nozzletrajectorydist,temp);

    matCreateCoordSysFromHeading(&nozzlecoordsys,&nozzletrajectory);

    intLength = TreatAsUdword(nozzletrajectorydist);
    intWidth = TreatAsUdword(resourceRadius);

    vecAddTo(nozzleposition,ship->posinfo.position);

    //create an effect for bullet, if applicable
    if (etgLOD != NULL)
    {
        LOD = ship->currentLOD;
        if (LOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[LOD];
        }
    }
    else
    {
        stat = NULL;
    }
#if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
    if (stat != NULL && !etgFrequencyExceeded(stat))
#endif
    {
        if (resource->objtype == OBJ_AsteroidType)
            ship->soundevent.specialHandle = soundEvent(ship, Ship_ResourceAsteroid);
        else //if (resource->objtype == OBJ_DustType)
            ship->soundevent.specialHandle = soundEvent(ship, Ship_ResourceDustCloud);
        /* should have other resource types too? */
        ship->rceffect = etgEffectCreate(stat, ship, &nozzleposition, &ship->posinfo.velocity, &nozzlecoordsys, 1.0f, EAF_Velocity | EAF_NLips, 2, intLength, intWidth);
    }
    else
    {
        ship->rceffect = NULL;                              //play no effect
    }
}

/*-----------------------------------------------------------------------------
    Name        : ModifyHarvestEffect
    Description : Modify the coordinate system, length and width of a
                    resourcing effect.
    Inputs      : resourceRadius - radius of the resource we are harvesting
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ModifyHarvestEffect(Effect *effect,Ship *ship,vector *trajectory, real32 resourceRadius)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    ResNozzleStatic *resNozzleStatic = shipstatic->resNozzleStatic;
    vector nozzleposition;
    vector nozzletrajectory;
    real32 nozzletrajectorydist;
    real32 temp;

    dbgAssert(resNozzleStatic);
    dbgAssert(effect);

    matMultiplyMatByVec(&nozzleposition,&ship->rotinfo.coordsys,&resNozzleStatic->position);

    vecSub(nozzletrajectory,*trajectory,nozzleposition);

    vecAdd(effect->posinfo.position,nozzleposition,ship->posinfo.position);

    nozzletrajectorydist = fsqrt(vecMagnitudeSquared(nozzletrajectory));
    vecDivideByScalar(nozzletrajectory,nozzletrajectorydist,temp);

    matCreateCoordSysFromHeading(&effect->rotinfo.coordsys,&nozzletrajectory);

    ((real32 *)effect->variable)[ETG_ResourceLengthParam] = nozzletrajectorydist;
    ((real32 *)effect->variable)[ETG_ResourceRadiusParam] = resourceRadius;
}

void DefaultShipHarvestsResource(struct Ship *ship,struct Resource *resource)
{
    if (!MoveReachedDestinationVariable(ship,&resource->posinfo.position,COLLECTRESOURCE_DIST))
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&resource->posinfo.position,AISHIP_FastAsPossible + AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,0.0f,&resource->posinfo.velocity);
    }
    else
    {
        aitrackSteadyShip(ship);
        CollectResources(ship,resource);
    }
}

#define R1RCASTEROID_APPROACHASTEROID       1
#define R1RCASTEROID_HARVEST_ON             2
#define R1RCASTEROID_HARVEST                3

/*-----------------------------------------------------------------------------
    Name        : R1ResourcerHarvestsAsteroid
    Description : Used to be only for asteroids, now generalized
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void R1ResourcerHarvestsAsteroid(struct Ship *ship,struct Resource *resource)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    vector trajectory;
    real32 range;
    real32 dist;
    real32 temp;
    etglod *lod;
    real32 resourceRadius;

//    dbgAssert(resource->objtype == OBJ_AsteroidType);

    switch (ship->rcstate1)
    {
        case 0:
            // initialize any data
            ship->rcstate1 = R1RCASTEROID_APPROACHASTEROID;
            // deliberately fall through to R1RCASTEROID_APPROACHASTEROID

        case R1RCASTEROID_APPROACHASTEROID:
            aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)resource,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)resource,dist);
            if (range > ASTEROID_HARVEST_RANGE)
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,&resource->posinfo.position,AISHIP_FastAsPossible + AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying + AISHIP_DontFlyToObscuredPoints,0.0f,&resource->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
                    resource->resourceNotAccessible = (UNIVERSE_UPDATE_RATE * RESOURCE_NOT_ACCESSIBLE_SECONDS);
                break;
            }
#ifdef DEBUG_COLLECTRESOURCES
            dbgMessagef("\n%x Changing to harvest",ship);
#endif
            ship->rcstate1 = R1RCASTEROID_HARVEST_ON;
            // deliberately fall through to R1RCASTEROID_HARVEST_ON

        case R1RCASTEROID_HARVEST_ON:
            // turn on harvest effect here
            if (resource->objtype == OBJ_AsteroidType)
            {
                lod = etgResourceEffectTable[ship->shiprace][ETG_AsteroidEffect];
            }
            else
            {
                lod = etgResourceEffectTable[ship->shiprace][ETG_GaseousEffect];
            }
            resourceRadius = resource->staticinfo->staticheader.staticCollInfo.collspheresize;
            TurnHarvestEffectOn(ship,resource,&trajectory, resourceRadius, lod);

            ship->rcstate1 = R1RCASTEROID_HARVEST;
            break;

        case R1RCASTEROID_HARVEST:
            aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)resource,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)resource,dist);
            if(ship->rceffect == NULL)
            {
                //if the effect isn't playing...lets play it...
                if (resource->objtype == OBJ_AsteroidType)
                {
                    lod = etgResourceEffectTable[ship->shiprace][ETG_AsteroidEffect];
                }
                else
                {
                    lod = etgResourceEffectTable[ship->shiprace][ETG_GaseousEffect];
                }
                resourceRadius = resource->staticinfo->staticheader.staticCollInfo.collspheresize;
                TurnHarvestEffectOn(ship,resource,&trajectory, resourceRadius, lod);
            }
            if (range > ASTEROID_MAXHARVEST_RANGE)
            {
                // turn off effect here
                if (ship->rceffect != NULL)
                {
                    TurnHarvestEffectOff(ship);
                }
                dbgAssert(ship->rceffect == NULL);

                ship->rcstate1 = R1RCASTEROID_APPROACHASTEROID;
                break;
            }

            // recalculate effect coordsys and length of effect
            if (ship->rceffect != NULL)
            {
                resourceRadius = resource->staticinfo->staticheader.staticCollInfo.collspheresize;
                ModifyHarvestEffect(ship->rceffect,ship,&trajectory, resourceRadius);
            }

            // harvest RU's

            CollectResources(ship,resource);

            vecDivideByScalar(trajectory,dist,temp);

            // circle the asteroid if harvesting big asteroids
            if ((resource->objtype == OBJ_AsteroidType) &&
                (((Asteroid *)resource)->asteroidtype < (AsteroidType)CIRCLE_HARVEST_ASTEROIDS_BIGGEREQUAL_THAN))
            {
                aitrackHeading(ship,&trajectory,FLYSHIP_HEADINGACCURACY);
                aitrackZeroVelocity(ship);
            }
            else
            {
#if 0
                real32 forwardvelocity;
                vector heading;

                aitrackRightVector(ship,&trajectory,FLYSHIP_HEADINGACCURACY);

                matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
                forwardvelocity = vecDotProduct(heading,ship->posinfo.velocity);

                if (forwardvelocity < CIRCLE_FORWARD_VELOCITY)
                {
                    physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*CIRCLE_FORWARD_THRUST,TRANS_FORWARD);
                }

                if (range > ASTEROID_HARVEST_RANGE)
                {
                    aishipFlyToPointAvoidingObjsWithVel(ship,&resource->posinfo.position,0,0.0f,&resource->posinfo.velocity);
                }
#else
                real32 rightvelocity;
                vector right;

                aitrackHeading(ship,&trajectory,FLYSHIP_HEADINGACCURACY);

                matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);
                rightvelocity = vecDotProduct(right,ship->posinfo.velocity);

                if (rightvelocity < CIRCLE_RIGHT_VELOCITY)
                {
//                    dbgMessagef("\nApplying right thrust %f",rightvelocity);
                    physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_RIGHT]*CIRCLE_RIGHT_THRUST,TRANS_RIGHT);
                }

                if (range > ASTEROID_HARVEST_RANGE)
                {
//                    dbgMessage("\nGoing back towards");
                    aishipFlyToPointAvoidingObjsWithVel(ship,&resource->posinfo.position,0,0.0f,&resource->posinfo.velocity);
                }
                else
                {
                    aitrackZeroForwardVelocity(ship);
                }
#endif
            }
            break;
    }
    return;
}

/*-----------------------------------------------------------------------------
    Name        : InitShipForResourceCollection
    Description : initializes ship ai variables for resource collection
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitShipForResourceCollection(Ship *ship,Resource *resource)
{
    InitShipAI(ship,TRUE);

    ship->rcstate1 = 0;
    ship->rcstate2 = 0;

    ship->ShipXHarvestsResourceY = R1ResourcerHarvestsAsteroid;//DefaultShipHarvestsResource;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void PreFix_ShipXHarvestsResourceY(Ship *ship)
{
    if (ship->ShipXHarvestsResourceY == DefaultShipHarvestsResource)
    {
        ship->ShipXHarvestsResourceY = 0;
    }
    else if (ship->ShipXHarvestsResourceY == R1ResourcerHarvestsAsteroid)
    {
        ship->ShipXHarvestsResourceY = 1;
    }
    else
    {
        ship->ShipXHarvestsResourceY = -1;
    }
}

void Fix_ShipXHarvestsResourceY(Ship *ship)
{
    switch ((sdword)ship->ShipXHarvestsResourceY)
    {
        case 0:
            ship->ShipXHarvestsResourceY = DefaultShipHarvestsResource;
            break;

        case 1:
            ship->ShipXHarvestsResourceY = R1ResourcerHarvestsAsteroid;
            break;

        default:
            ship->ShipXHarvestsResourceY = NULL;
            break;
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*-----------------------------------------------------------------------------
    Name        : processCollectResource
    Description : processes a collect resource command
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool processCollectResource(struct CommandToDo *collecttodo)
{
    sdword numShips = collecttodo->selection->numShips;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    Resource *resource = collecttodo->collect.resource;
    Ship *dockship;

    dbgAssert(numShips > 0);
    dbgAssert(numShips == 1);

    ship = collecttodo->selection->ShipPtr[0];
    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    if (ship->resources >= shipstatic->maxresources)
    {
        TurnOffAnyResourcingEffects(collecttodo);
        collecttodo->collect.resource = NULL;           // override - don't collect resource if full
        resource = NULL;
        goto letsdock;
    }

    if (resource == NULL)
    {
        if ((ship->resources < shipstatic->resourcesAtOneTime) && ((resource = rescollectFindNearestResource(ship,collecttodo)) != NULL))
        {
#ifdef DEBUG_COLLECTRESOURCES
            dbgMessage("\nCollecting next resource...");
#endif
            InitShipForResourceCollection(ship,resource);
            collecttodo->collect.resource = resource;
            return FALSE;
        }
        else
        {
letsdock:
            if ((ship->resources > 0) && ((dockship = FindNearestShipToDockAt(ship,DOCK_TO_DEPOSIT_RESOURCES)) != NULL))
            {
#ifdef DEBUG_COLLECTRESOURCES
                dbgMessage("\nFlying back to deposit resources...");
#endif
                // changing COMMAND_COLLECTRESOURCE to COMMAND_DOCK
                FreeLastOrder(collecttodo); // free COMMAND_COLLECTRESOURCE
                dockChangeSingleShipToDock(collecttodo,ship,dockship,TRUE,DOCK_TO_DEPOSIT_RESOURCES);

                return FALSE;
            }
            else
            {
                if (ship->playerowner->playerIndex)
                {
                    tutGameMessage("Game_NonPlayerHarvesterDone");
                }
                else
                {
                    tutGameMessage("Game_PlayerHarvesterDone");
                }

                speechEventFleet(STAT_F_ResourcesAllHarvested, 0, ship->playerowner->playerIndex);

                return TRUE;
            }
        }
    }
    else
    {
        if (ResourceMovingTooFast(resource))
        {
            // resource moving too fast, so pick a different one.
            TurnOffAnyResourcingEffects(collecttodo);
            collecttodo->collect.resource = NULL;
            return FALSE;
        }

        if (ResourceAlreadyBeingHarvested(&universe.mainCommandLayer,collecttodo,resource))
        {
            // resource already being harvested by someone else, so pick a different one.
            TurnOffAnyResourcingEffects(collecttodo);
            collecttodo->collect.resource = NULL;
            return FALSE;
        }

        if ((universe.univUpdateCounter & CHECK_HARV_OUTOFWORLD_RESOURCE_RATE) == (resource->resourceID.resourceNumber & CHECK_HARV_OUTOFWORLD_RESOURCE_RATE))
        {
            if (univObjectOutsideWorld((SpaceObj *)resource))
            {
                TurnOffAnyResourcingEffects(collecttodo);
                collecttodo->collect.resource = NULL;
                return FALSE;
            }
        }

        ship->ShipXHarvestsResourceY(ship,resource);

        if (resource->resourceNotAccessible)
        {
            TurnOffAnyResourcingEffects(collecttodo);
            collecttodo->collect.resource = NULL;
            return FALSE;
        }

        return FALSE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : R1ResourcerAttacksShip
    Description : used for resourcers attacking ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void R1ResourcerAttacksShip(struct Ship *ship,struct SpaceObjRotImpTarg *target,bool passiveAttacking)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    vector trajectory;
    real32 range;
    real32 dist;
    real32 temp;
    etglod *lod;
    real32 targetRadius;

//    dbgAssert(resource->objtype == OBJ_AsteroidType);

    switch (ship->rcstate1)
    {
        case 0:
            // initialize any data
            ship->rcstate1 = R1RCASTEROID_APPROACHASTEROID;
            // deliberately fall through to R1RCASTEROID_APPROACHASTEROID

        case R1RCASTEROID_APPROACHASTEROID:
            aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);
            if (range > ASTEROID_HARVEST_RANGE)
            {
                aishipFlyToPointAvoidingObjsWithVel(ship,&target->posinfo.position,AISHIP_FastAsPossible + AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,0.0f,&target->posinfo.velocity);
                break;
            }

            dbgMessagef("\nShip Changing to Harvest/attack mode.");

            ship->rcstate1 = R1RCASTEROID_HARVEST_ON;
            // deliberately fall through to R1RCASTEROID_HARVEST_ON

        case R1RCASTEROID_HARVEST_ON:
            lod = etgResourceEffectTable[ship->shiprace][ETG_AsteroidEffect];

            targetRadius = target->staticinfo->staticheader.staticCollInfo.collspheresize;
            TurnHarvestEffectOn(ship,(Resource *)target,&trajectory, targetRadius, lod);

            ship->rcstate1 = R1RCASTEROID_HARVEST;
            break;

        case R1RCASTEROID_HARVEST:
            aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);

            if (range > ASTEROID_MAXHARVEST_RANGE)
            {
                // turn off effect here
                if (ship->rceffect != NULL)
                {
                    TurnHarvestEffectOff(ship);
                }
                dbgAssert(ship->rceffect == NULL);

                ship->rcstate1 = R1RCASTEROID_APPROACHASTEROID;
                break;
            }
            if(ship->rceffect == NULL)
            {
                lod = etgResourceEffectTable[ship->shiprace][ETG_AsteroidEffect];

                targetRadius = target->staticinfo->staticheader.staticCollInfo.collspheresize;
                TurnHarvestEffectOn(ship,(Resource *)target,&trajectory, targetRadius, lod);
            }


            // recalculate effect coordsys and length
            if (ship->rceffect != NULL)
            {
                targetRadius = target->staticinfo->staticheader.staticCollInfo.collspheresize;
                ModifyHarvestEffect(ship->rceffect,ship,&trajectory, targetRadius);
            }

            // harvest RU's

            //CollectResources(ship,resource);

            ApplyDamageToTarget(target,TW_HARVESTER_DAMAGE_PER_SECOND*universe.phystimeelapsed,-1,DEATH_Killed_By_Player,ship->playerowner->playerIndex);

            vecDivideByScalar(trajectory,dist,temp);

            // circle the asteroid if harvesting big asteroids
            //if ((resource->objtype == OBJ_AsteroidType) &&
            //    (((Asteroid *)resource)->asteroidtype < (AsteroidType)CIRCLE_HARVEST_ASTEROIDS_BIGGEREQUAL_THAN))
            //{
            //    aitrackHeading(ship,&trajectory,FLYSHIP_HEADINGACCURACY);
            //    aitrackZeroVelocity(ship);
            //}
            //
            //else
            {
#if 0
                real32 forwardvelocity;
                vector heading;

                aitrackRightVector(ship,&trajectory,FLYSHIP_HEADINGACCURACY);

                matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
                forwardvelocity = vecDotProduct(heading,ship->posinfo.velocity);

                if (forwardvelocity < CIRCLE_FORWARD_VELOCITY)
                {
                    physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*CIRCLE_FORWARD_THRUST,TRANS_FORWARD);
                }

                if (range > ASTEROID_HARVEST_RANGE)
                {
                    aishipFlyToPointAvoidingObjsWithVel(ship,&target->posinfo.position,0,0.0f,&target->posinfo.velocity);
                }
#else
                real32 rightvelocity;
                vector right;

                aitrackHeading(ship,&trajectory,FLYSHIP_HEADINGACCURACY);

                matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);
                rightvelocity = vecDotProduct(right,ship->posinfo.velocity);

                if (rightvelocity < CIRCLE_RIGHT_VELOCITY)
                {
//                    dbgMessagef("\nApplying right thrust %f",rightvelocity);
                    physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_RIGHT]*CIRCLE_RIGHT_THRUST,TRANS_RIGHT);
                }

                if (range > ASTEROID_HARVEST_RANGE)
                {
//                    dbgMessage("\nGoing back towards");
                    aishipFlyToPointAvoidingObjsWithVel(ship,&target->posinfo.position,0,0.0f,&target->posinfo.velocity);
                }
                else
                {
                    aitrackZeroForwardVelocity(ship);
                }
#endif
            }

            break;
    }
    return;
}

void removeResourcerFromAttacking(Ship *ship)
{
    if (ship->rceffect != NULL)
    {
        TurnHarvestEffectOff(ship);
    }
}


