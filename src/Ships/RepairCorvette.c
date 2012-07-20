/*=============================================================================
    Name    : RepairCorvette.c
    Purpose : Specifics for the RepairCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "RepairCorvette.h"
#include "StatScript.h"
#include "FastMath.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "ShipSelect.h"
#include "AIShip.h"
#include "AITrack.h"
#include "CommandLayer.h"
#include "UnivUpdate.h"
#include "Universe.h"
#include "Collision.h"
#include "SaveGame.h"
#include "SoundEvent.h"

void stopRepairEffect(Ship *ship);


#define flytoRepairDest(scalar,tol) \
        matGetVectFromMatrixCol1(targetup,spec->target->rotinfo.coordsys);                  \
        matGetVectFromMatrixCol3(targetheading,spec->target->rotinfo.coordsys);             \
                                                                                            \
        trackflag = TRUE;                                                                   \
        vecScalarMultiply(targetdown,targetup,-1.0f);                                       \
                                                                                            \
        if(ship->shiprace == R1)                                                            \
        {                                                                                   \
            vecScalarMultiply(trackup,targetup,-1.0f);                                      \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            vecScalarMultiply(trackup,targetup, 1.0f);                                      \
        }                                                                                   \
                                                                                            \
        if(!aitrackHeadingAndUp(ship,&targetheading,&trackup,0.999f))                       \
        {                                                                                   \
            trackflag = FALSE;                                                              \
        }                                                                                   \
                                                                                            \
        vecScalarMultiply(destination,targetdown,scalar);                                   \
        vecAddTo(destination,spec->target->posinfo.position);                               \
                                                                                            \
        if(!MoveReachedDestinationVariable(ship,&destination,tol))                          \
        {                                                                                   \
            aishipFlyToPointAvoidingObjs(ship,&destination,0,0.0f);                         \
            trackflag = FALSE;                                                              \
        }                                                                                   \
        else                                                                                \
        {                                                                                       \
            aitrackZeroVelocity(ship);                                                          \
        }
/*
        aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)spec->target,&trajectory);           \
        range = RangeToTarget(ship,(SpaceObjRotImpTarg *)spec->target,&trajectory);         \
*/                                                                                          




#ifdef bpasechn
            #define DEBUG_REPAIR
#endif

RepairCorvetteStatics RepairCorvetteStaticRace1;
RepairCorvetteStatics RepairCorvetteStaticRace2;

RepairCorvetteStatics RepairCorvetteStatic;


scriptStructEntry RepairCorvetteStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.repairApproachDistance), (udword) &(RepairCorvetteStatic) },

    { "approachAndWaitDistance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.approachAndWaitDistance), (udword) &(RepairCorvetteStatic) },
    { "rotationStopDistance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.rotationStopDistance), (udword) &(RepairCorvetteStatic) },
    { "stopRotMultiplier",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.stopRotMultiplier), (udword) &(RepairCorvetteStatic) },
    { "sloppyRotThreshold",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.sloppyRotThreshold), (udword) &(RepairCorvetteStatic) },
    { "dockWithRotationSpeed",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.dockWithRotationSpeed), (udword) &(RepairCorvetteStatic) },
    { "targetStartDockDistance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.targetStartDockDistance), (udword) &(RepairCorvetteStatic) },
    { "startdockTolerance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.startdockTolerance), (udword) &(RepairCorvetteStatic) },
    { "finaldockDistance",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.finaldockDistance), (udword) &(RepairCorvetteStatic) },

    { "CapitalDistanceRepairStart",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.CapitalDistanceRepairStart), (udword) &(RepairCorvetteStatic) },
    { "capitalShipHealthPerSecond",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.capitalShipHealthPerSecond), (udword) &(RepairCorvetteStatic) },
    { "AngleDotProdThreshold",    scriptSetReal32CB, (udword) &(RepairCorvetteStatic.AngleDotProdThreshold), (udword) &(RepairCorvetteStatic) },


    { NULL,NULL,0,0 }
};

void RepairCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    RepairCorvetteStatics *corvstat = (statinfo->shiprace == R1) ? &RepairCorvetteStaticRace1 : &RepairCorvetteStaticRace2;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);
    scriptSetStruct(directory,filename,RepairCorvetteStaticScriptTable,(ubyte *)corvstat);

    statinfo->custstatinfo = corvstat;
}

void RepairCorvetteInit(Ship *ship)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;
    spec->repairState = REPAIR_Begin;
    spec->target = NULL;
    spec->hyst = FALSE;
    attackSideStepInit(&spec->attacksidestep);
}

void RepairCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;
    RepairCorvetteStatics *corvstat = (RepairCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void RepairCorvetteAttackPassive(Ship *ship,Ship *target,bool rotate)
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

void removeShipFromSelection(Ship *ship,SelectCommand *targets)
{
    sdword i;
    for(i=0;i<targets->numShips;i++)
    {
        if(ship == targets->ShipPtr[i])
        {
            targets->numShips--;
            targets->ShipPtr[i] = targets->ShipPtr[targets->numShips];
            break;
        }
    }
}

bool rotSpeedLessThanSloppy(vector *rotspeed, real32 thresh,real32 max)
{
    //check if rotspeed is less than the threshold
    real32 rotx,roty,rotz;
    real32 mx=1.0f;
    real32 my=1.0f;
    real32 mz=1.0f;


    if(rotspeed->x < 0.0f)
    {rotx = -rotspeed->x;mx=-1.0f;}
    else
    {rotx = rotspeed->x;}

    if(rotspeed->y < 0.0f)
    {roty = -rotspeed->y;my=-1.0f;}
    else
    {roty = rotspeed->y;}

    if(rotspeed->z < 0.0f)
    {rotz = -rotspeed->z;mz=-1.0f;}
    else
    {rotz = rotspeed->z;}


    if(rotx > thresh)
        return FALSE;
    if(roty > thresh)
        return FALSE;
    if(rotz > thresh)
        return FALSE;

    //check if rotspeed is above max rotation speed
    //and if so set it to the max rotation speed
    if(max < rotx)
        rotspeed->x = max*mx;
    if(max < roty)
        rotspeed->y = max*my;
    if(max < rotz)
        rotspeed->z = max*mz;

    return TRUE;
}


bool RepairCorvetteSpecialOps(Ship *ship, void *custom)
{
    SelectAnyCommand *targets;
    CommandToDo *command;
    SelectCommand oneship;
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;
    RepairCorvetteStatics *repaircorvettestatics;
    sdword i,trackflag,fixflag;
    vector targetheading,targetup,destination,targetdown,trackup;

    targets = (SelectAnyCommand *)custom;

    repaircorvettestatics = (RepairCorvetteStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(targets->numTargets == 0)
    {
        oneship.numShips = 1;
        oneship.ShipPtr[0] = ship;
        clHalt(&universe.mainCommandLayer, &oneship);
        return TRUE;
    }

    switch(spec->repairState)
    {
    case REPAIR_Begin:
        //remove non ships and enemy ships and non strike craft
        for(i=0;i<targets->numTargets;i++)
        {
            if(targets->TargetPtr[i]->objtype != OBJ_ShipType)
            {
                targets->numTargets--;
                targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            }
            //object is a ship
            else if( ((Ship *)targets->TargetPtr[i])->staticinfo->shipclass != CLASS_Fighter &&
                ((Ship *)targets->TargetPtr[i])->staticinfo->shipclass != CLASS_Corvette ||
                ((Ship *)targets->TargetPtr[i])->playerowner != ship->playerowner)
            {
                targets->numTargets--;
                targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            }
        }
        spec->target = (Ship *)targets->TargetPtr[0];
        spec->repairState = REPAIR_Approach;
#ifdef DEBUG_REPAIR
        dbgMessagef("\nAquired Target for Repair.");
#endif

        break;
    case REPAIR_Approach:
        //fly to target (flag it so we are the only ones going after it
        if(!MoveReachedDestinationVariable(ship,&spec->target->posinfo.position,repaircorvettestatics->approachAndWaitDistance))
        {
            //not within range yet, so fly to it
            aishipFlyToPointAvoidingObjs(ship,&spec->target->posinfo.position,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
            break;
        }
        //we are within range so lets see if we can fix it, or if we need
        //to wait
        if(!(spec->target->posinfo.isMoving & ISMOVING_MOVING))
        {
            //ship is stopped, so it is ready
            spec->repairState = REPAIR_Nearing;
#ifdef DEBUG_REPAIR
        dbgMessagef("\nREPAIR_Approach: %f distance reached",repaircorvettestatics->approachAndWaitDistance);
#endif

            break;
        }
        else
        {
            command = getShipAndItsCommand(&universe.mainCommandLayer,spec->target);
            if(command == NULL
               || command->ordertype.order == COMMAND_NULL)
            {
                //ship isn't really doing anything so we can
                //move in
                spec->repairState = REPAIR_Nearing;
#ifdef DEBUG_REPAIR
dbgMessagef("\nREPAIR_Approach: %f distance reached",repaircorvettestatics->approachAndWaitDistance);
#endif

            }
        }
        aitrackStabilizeShip(ship);
        break;
    case REPAIR_Nearing:
        //Orient Self with docking bay facing target
        if(!MoveReachedDestinationVariable(ship,&spec->target->posinfo.position,repaircorvettestatics->rotationStopDistance))
        {
            //not within range yet, so fly to it
            aishipFlyToPointAvoidingObjs(ship,&spec->target->posinfo.position,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
            break;
        }
        //we are within range so lets see if we can fix it, or if we need
        //to wait
        if(!(spec->target->posinfo.isMoving & ISMOVING_MOVING))
        {
            //ship is stopped, so it is ready

            spec->repairState = REPAIR_StopRot;
            //spec->repairState = REPAIR_Hackwork;

#ifdef DEBUG_REPAIR
        dbgMessagef("\nREPAIR_Nearing: %f distance reached, proceeding to stop rotation.",repaircorvettestatics->rotationStopDistance);
#endif
            break;
        }
        else
        {
            command = getShipAndItsCommand(&universe.mainCommandLayer,spec->target);
            if(command == NULL
               || command->ordertype.order == COMMAND_NULL)
            {
                //ship isn't really doing anything so we can
                //move in
            spec->repairState = REPAIR_StopRot;
            }
        }
        aitrackStabilizeShip(ship);
        break;
    case REPAIR_StopRot:
        //stop the ships rotation and translation...if any and
        //rotates itself into position
               //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);

        vecScalarMultiply(spec->target->rotinfo.rotspeed,spec->target->rotinfo.rotspeed,repaircorvettestatics->stopRotMultiplier);
        if(rotSpeedLessThanSloppy(&spec->target->rotinfo.rotspeed, repaircorvettestatics->sloppyRotThreshold, repaircorvettestatics->dockWithRotationSpeed))
        {
            //rotation below special threshold,
            //set it to our rotation threshold (should be pretty dang close
#ifdef DEBUG_REPAIR
        dbgMessagef("\nREPAIR_StopRot, rotation stoped, now getting into dock pos.");
#endif
            spec->repairState = REPAIR_Dock1;
        }

        aitrackStabilizeShip(ship);
        break;
    case REPAIR_Dock1:

        //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);

        //bring in the ship and connect it to our docking bay
        flytoRepairDest(repaircorvettestatics->targetStartDockDistance,repaircorvettestatics->startdockTolerance);

        if(trackflag)
        {
#ifdef DEBUG_REPAIR
    dbgMessagef("\nREPAIR_Dock1: Docking Position 1 reached.");
#endif
            spec->repairState = REPAIR_Dock2;
            //set docking ship var to avoid collisions...
            //ship->dockingship = spec->target;
            spec->target->dockingship = ship;
        }
        break;
    case REPAIR_Dock2:
        //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);

        //bring in the ship and connect it to our docking bay
        flytoRepairDest(repaircorvettestatics->finaldockDistance,6.0f);

        //if(range <= repaircorvettestatics->finaldockDistance)
        //if(MoveReachedDestinationVariable(ship,&destination,repaircorvettestatics->finaldockDistance))
        if(trackflag)
        {
#ifdef DEBUG_REPAIR
            dbgMessagef("\nREPAIR_Dock2: Docking Position 2 reached.");
#endif
			ship->soundevent.specialHandle = soundEvent(ship, Ship_RepairLoop);
            spec->repairState = REPAIR_Repair;
        }

        break;
    case REPAIR_Repair:
        //transfer fuel, and repair ships health
        //probably should latch ship hear by stopping all forces
        //and such...or by actually latching it on using
        //salcap latch code (modified to become general...)

        //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);

        flytoRepairDest(repaircorvettestatics->finaldockDistance,10.0f);

        fixflag = TRUE;
        if(spec->target->health < spec->target->staticinfo->maxhealth)
        {
            spec->target->health += ship->staticinfo->repairOtherShipRate*universe.phystimeelapsed;
            if(spec->target->health >= spec->target->staticinfo->maxhealth)
            {
                spec->target->health = spec->target->staticinfo->maxhealth;
            }
            else
                fixflag = FALSE;

        }

        if(spec->target->fuel < spec->target->staticinfo->maxfuel)
        {
            spec->target->fuel += ship->staticinfo->pumpFuelRate*universe.phystimeelapsed;
            if(spec->target->fuel >= spec->target->staticinfo->maxfuel)
            {
                spec->target->fuel = spec->target->staticinfo->maxfuel;
            }
            else
                fixflag = FALSE;
        }

        if(fixflag)
        {
            //we're done...
            spec->repairState = REPAIR_Disengage1;
        }
        break;
    case REPAIR_Disengage1:
        //De Dock...
        //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);
        spec->target->dockingship = NULL;

        /*
        oneship.numShips = 1;
        oneship.ShipPtr[0] = spec->target;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&oneship);
        */

        spec->repairState = REPAIR_Disengage2;
        spec->disengagetime = 0.0f;
        break;
    case REPAIR_Disengage2:
        //De Dock...
        //stop ship from takign off instantly
        bitSet(spec->target->dontrotateever,1);
        bitSet(spec->target->dontapplyforceever,1);
        matGetVectFromMatrixCol1(targetup,ship->rotinfo.coordsys);
        vecScalarMultiply(targetup,targetup,100.0f);
        vecAdd(destination,targetup,ship->posinfo.position);

        aishipFlyToPointAvoidingObjs(ship,&destination,0,0.0f);

        spec->disengagetime += universe.phystimeelapsed;
        if(spec->disengagetime > 1.5f)
            spec->repairState = REPAIR_Done;
        break;
    case REPAIR_Done:
        //move on to next ship
               //stop ship from takign off instantly
        bitClear(spec->target->dontrotateever,1);
        bitClear(spec->target->dontapplyforceever,1);
        removeShipFromSelection(spec->target,(SelectCommand *)targets);
        spec->repairState = REPAIR_Begin;
        spec->target = NULL;
        return FALSE;
        break;
    case REPAIR_Hackwork:
        oneship.numShips = 1;
        oneship.ShipPtr[0] = spec->target;
        spec->target->fuel += 100.0f;       //minimal ammount to allow docking
        clDock(&universe.mainCommandLayer, &oneship,DOCK_FOR_BOTHREFUELANDREPAIR,ship);
        return TRUE;
    }

    return FALSE;
}

void RepairCorvetteRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;
    if(shiptoremove == spec->target)
    {
        spec->repairState = REPAIR_Begin;
        spec->target = NULL;
    }
}

void RepairCorvetteDied(Ship *ship)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;
    if(spec->target != NULL)
    {
        bitClear(spec->target->dontrotateever,1);
        bitClear(spec->target->dontapplyforceever,1);

        spec->target = NULL;
    }

}

void RepairCorvetteOrderChanged(Ship *ship)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;

    //for super duper cool intelligence, we could check if the order is the
    //same thing as what we're doing currently!  Then we don't stop if it
    //is.

    spec->repairState = REPAIR_Begin;
    if(spec->target != NULL)
    {
        bitClear(spec->target->dontrotateever,1);
        bitClear(spec->target->dontapplyforceever,1);

        spec->target = NULL;
    }

    if(ship->rceffect != NULL)
    {
        stopRepairEffect(ship);
    }
    spec->hyst = FALSE;
}
void startRepairEffect(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory,real32 distance)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    RepairNozzleStatic *repnozzlestatic = shipstatic->repairNozzleStatic;
    //TURN ON EFFECT HERE
    //
    //Use ship->rceffect, since ship isn't a resource collector,
    //the pointer isn't being used
    etglod *etgLOD = etgSpecialPurposeEffectTable[EGT_REPAIR_BEAM];

    etgeffectstatic *stat;
    sdword LOD;
    udword intLength;
    udword intWidth;
    matrix coordsys;
    real32 targetRadius = target->staticinfo->staticheader.staticCollInfo.collspheresize;
    vector repairBeamPosition;

    matMultiplyMatByVec(&repairBeamPosition,&ship->rotinfo.coordsys,&repnozzlestatic->position);

    matCreateCoordSysFromHeading(&coordsys,trajectory);

    intLength = TreatAsUdword(distance);
    intWidth = TreatAsUdword(targetRadius);

    vecAddTo(repairBeamPosition,ship->posinfo.position);

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
        ship->rceffect = etgEffectCreate(stat, ship, &repairBeamPosition, &ship->posinfo.velocity, &coordsys, 1.0f, EAF_Velocity | EAF_NLips, 2, intLength, intWidth);
    }
    else
    {
        ship->rceffect = NULL;                              //play no effect
    }


}
void stopRepairEffect(Ship *ship)
{

#ifdef HW_DEBUG
    //make sure we only entered here if effect was playing!
    dbgAssert(ship->rceffect != NULL);
#endif

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

    //soundEventStop(ship->soundevent.specialHandle);

    ship->rceffect = NULL;
}

void ModifyRepairEffect(Effect *effect,Ship *ship,vector *trajectory,real32 distance, SpaceObjRotImpTarg *target)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    RepairNozzleStatic *repnozzlestatic = shipstatic->repairNozzleStatic;
    vector repairBeamPosition;

    dbgAssert(effect);

    matMultiplyMatByVec(&repairBeamPosition,&ship->rotinfo.coordsys,&repnozzlestatic->position);

    vecAdd(effect->posinfo.position,repairBeamPosition,ship->posinfo.position);

    matCreateCoordSysFromHeading(&effect->rotinfo.coordsys,trajectory);

	distance = max(200.0f,distance);
	if(target->objtype == OBJ_ShipType)
	{
		if(((Ship *)target)->shiptype == Mothership)
		{
			distance = max(800.0f,distance);
		}
	}


    ((real32 *)effect->variable)[ETG_ResourceLengthParam] = distance;
    ((real32 *)effect->variable)[ETG_ResourceRadiusParam] = target->staticinfo->staticheader.staticCollInfo.collspheresize;
}

//function will return the most heavily damaged target in the list, OR
//if all are fully healthy, it will return the FIRST target in the list
bool areAllHealthy(Ship *ship,SelectAnyCommand *targets)
{
    sdword i;
    for(i=0;i<targets->numTargets;i++)
    {
        if(targets->TargetPtr[i]->health != ((ShipStaticInfo *)targets->TargetPtr[i]->staticinfo)->maxhealth)
        {
            return FALSE;
        }
    }
    return (TRUE);
}
bool refuelRepairShips(Ship *ship, SelectAnyCommand *targets,real32 rangetoRefuel)
{
    //remove unwantedships from this selection
    //optimize by doing only once for this ship
    SelectCommand selectOne;
    sdword i;
    SpaceObjRotImpTarg *target,*targettemp;
    vector trajectory,curheading;
    real32 range,dotprod;
    CommandToDo *targetCommand;

    //filter target list for unrepairable ships!

    for(i=0;i<targets->numTargets;)
    {
        if(targets->TargetPtr[i]->objtype != OBJ_ShipType)
        {
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            continue;
        }
        targetCommand = getShipAndItsCommand(&universe.mainCommandLayer,(Ship *)targets->TargetPtr[i]);
        if(targetCommand != NULL)
        {
            if(targetCommand->ordertype.order == COMMAND_DOCK)
            {
                //remove docking ships from the list
                if(((Ship *)targets->TargetPtr[i])->dockingship != NULL)
                {
                    //only if in final stages
                    targets->numTargets--;
                    targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
                    continue;
                }
            }
        }

        //object is a ship
        if( ((Ship *)targets->TargetPtr[i])->staticinfo->shipclass != CLASS_Fighter &&
            ((Ship *)targets->TargetPtr[i])->staticinfo->shipclass != CLASS_Corvette ||
            ((Ship *)targets->TargetPtr[i])->playerowner != ship->playerowner)
        {
            if(ship->staticinfo->repairBeamCapable)
            {
                //repair corvette can repair capital ships!
                //class doesn't matter
                if(((Ship *)targets->TargetPtr[i])->playerowner == ship->playerowner)
                {
                    //ship is not a fighter or a corvette, but is ours, so we can fix it!
                    i++;
                    continue;
                }
            }
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
        }
        else
        {
            i++;
        }
    }

    if (targets->numTargets <= 0)
    {
        return TRUE;
    }



    target = targets->TargetPtr[0];

//needto add
//repairBeamCapable
//    real32 healthPerSecond,CapitalDistanceRepairStart;
//    real32 CapitalDistanceRepairStart2;

    if(ship->staticinfo->repairBeamCapable)
    {
        //assume target is a ship!!!!

        //targets might be a capital ship
        if(((Ship *)target)->staticinfo->shipclass != CLASS_Fighter)
        {
            //target is such that it should be BEAM repaired!
            //so lets fly upto it and ZAP repair it!
            real32 range1,range2;
            aishipGetTrajectory(ship,target,&trajectory);
            range = RangeToTarget(ship,target,&trajectory);

            vecNormalize(&trajectory);
            aitrackHeading(ship,&trajectory,0.999f);

            if(((Ship *)target)->shiptype == Mothership)
            {
                range1 = 2*ship->staticinfo->CapitalDistanceRepairStart;
                range2 = 2*ship->staticinfo->CapitalDistanceRepairStart2;
            }
            else
            {
                range1 = ship->staticinfo->CapitalDistanceRepairStart;
                range2 = ship->staticinfo->CapitalDistanceRepairStart2;
            }
            //we add 300 to the range checker for a sort of hysterysis!
            if(range > range1)
            {
                aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_FastAsPossible,0.0f);
                if(range > range2)
                {
                    //far enough away to stop the effect and ONLY fly towards ship
                    if(ship->rceffect != NULL)
                    {
                        //turn off ships effect
                        stopRepairEffect(ship);
                    }
                    return FALSE;
                }
            }
            else
            {
                aitrackZeroVelocity(ship);
            }

            //in range to repair this li'l ship


            matGetVectFromMatrixCol3(curheading,ship->rotinfo.coordsys);
            dotprod = vecDotProduct(trajectory,curheading);

            if(!areAllHealthy(ship,targets))
            {
                if(dotprod > ship->staticinfo->AngleDotProdThreshold)
                {
                    //within angle of 'repair beam'
                    if(ship->rceffect == NULL)
                    {
                        //turn on effect if we should!
                        startRepairEffect(ship,target,&trajectory,range);
                    }
                    else
                    {
                        //fix up effect
                        ModifyRepairEffect(ship->rceffect,ship,&trajectory,range, target);
                    }
                    target->health+=ship->staticinfo->healthPerSecond*universe.phystimeelapsed;
                    if(((Ship *)target)->health > ((Ship *)target)->staticinfo->maxhealth)
                    {
                        if(ship->rceffect != NULL)
                        {
                            //turn off ships effect
                            stopRepairEffect(ship);
                        }
                        target->health = ((Ship *)target)->staticinfo->maxhealth;

                        //fix this in a bit
                        targettemp = targets->TargetPtr[0];
                        for(i=0;i<(targets->numTargets-1);i++)
                        {
                            targets->TargetPtr[i]=targets->TargetPtr[i+1];
                        }
                        targets->TargetPtr[targets->numTargets-1] = targettemp;

                        //if(targets->numTargets <= 0)
                        //    return TRUE;
                        //targets->TargetPtr[0] = targets->TargetPtr[targets->numTargets];
                        return FALSE;
                    }
                }
                else
                {
                    //not pointing at ship, turn off effect and other
                    //needed things!
                    if(ship->rceffect != NULL)
                    {
                        //turn off ships effect
                        stopRepairEffect(ship);
                    }
                }
            }
            else
            {
                if(ship->rceffect != NULL)
                {
                    //turn off ships effect
                    stopRepairEffect(ship);
                }
            }
            return FALSE;
        }
    }

    aishipGetTrajectory(ship,target,&trajectory);
    aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_FastAsPossible +AISHIP_FirstPointInDirectionFlying,0.0f);
    range = RangeToTarget(ship,target,&trajectory);

    if(range <= rangetoRefuel)
    {
        //within repair range
        selectOne.numShips = 1;
        selectOne.ShipPtr[0] = (Ship *) target;
        if(target->objtype == OBJ_ShipType)
        {
            ((Ship *)target)->fuel += 300.0f;   //??? appropriate ammount?
            if(((Ship *)target)->fuel > ((Ship *)target)->staticinfo->maxfuel)
                ((Ship *)target)->fuel = ((Ship *)target)->staticinfo->maxfuel;

            //"speech event?  Switching to Emergency Docking Fuel Supply?
            //cldock
            clDock(&universe.mainCommandLayer,&selectOne,DOCK_AT_SPECIFIC_SHIP,ship);
        }
        targets->numTargets--;
        if(targets->numTargets <= 0)
            return TRUE;
        targets->TargetPtr[0] = targets->TargetPtr[targets->numTargets];
    }

    return FALSE;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void RepairCorvette_PreFix(Ship *ship)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (Ship *) SpaceObjRegistryGetID((SpaceObj *)spec->target);
}

void RepairCorvette_Fix(Ship *ship)
{
    RepairCorvetteSpec *spec = (RepairCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (Ship *) SpaceObjRegistryGetShip((sdword)spec->target);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"


bool RepairCorvetteSpecialTarget(Ship *ship,void *custom)
{
    RepairCorvetteStatics *stat = (RepairCorvetteStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets;
    targets = (SelectAnyCommand *)custom;
    return(refuelRepairShips(ship, targets,stat->repairApproachDistance));
}
CustShipHeader RepairCorvetteHeader =
{
    RepairCorvette,
    sizeof(RepairCorvetteSpec),
    RepairCorvetteStaticInit,
    NULL,
    RepairCorvetteInit,
    NULL,
    RepairCorvetteAttack,
    DefaultShipFire,
    RepairCorvetteAttackPassive,
    NULL,
    RepairCorvetteSpecialTarget,
    NULL,
    RepairCorvetteRemoveShipReferences,
    RepairCorvetteDied,
    RepairCorvette_PreFix,
    NULL,
    NULL,
    RepairCorvette_Fix
};

