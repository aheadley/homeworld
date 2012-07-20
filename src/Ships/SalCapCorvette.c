/*=============================================================================
    Name    : SalCapCorvette.c
    Purpose : Specifics for the Salvage Capture Corvette

    Created 11/5/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "SalCapCorvette.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "Select.h"
#include "ShipSelect.h"
#include "UnivUpdate.h"
#include "AIShip.h"
#include "AITrack.h"
#include "memory.h"
#include "Universe.h"
#include "FastMath.h"
#include "Dock.h"
#include "Vector.h"
#include "CommandLayer.h"
#include "SoundEvent.h"
#include "Physics.h"
#include "MultiplayerGame.h"
#include "SaveGame.h"
#include "AIVar.h"
#include "prim3d.h"
#include "render.h"
#include "Clamp.h"
#include "NIS.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "Mothership.h"
#include "Collision.h"
#include "Carrier.h"
#include "Alliance.h"
#include "Tactics.h"
#include "Ping.h"
#include "SinglePlayer.h"
#include "Battle.h"
#include "CommandWrap.h"
#include "GenericDefender.h"
#include "GravWellGenerator.h"
#include "Randy.h"

#define DEBUG_SALCAP

#define SALVAGE_BEGIN               0
#define SAL_FLYTOTARGET             1
#define SAL_DOCKINGSTATE1           3
#define SAL_DOCKWITHLIGHTS1         4
#define SAL_DOCKWITHOUTLIGHTS1      5
#define SAL_WL_FLYTOWITHINCONE      6
#define SAL_CLAMPED                 8
#define SAL_STRIPTECH               9
#define SAL_TECH_UNDOCK             10
#define SAL_SALVAGE1                11

#define SAL_UNDOCK_AND_QUIT         12
#define SAL_FLYBACK_TO_DOCKING_SHIP 13
#define SAL_DOCKINGSTAGE            14
#define SAL_TECH_TAKE_TECH_HOME     15
#define SAL_WAIT                    16
#define SAL_DO_DOCK                 17

#define SAL_backoff                 35
#define SAL_DO_DOCK2                18
#define SAL_DO_DOCK3                19
#define SAL_DONE_DOCKING            20
#define WAYPOINTHACK2               21
#define SAL_DO_DOCK25               22
#define DOORDEAL                    23
#define DO_HARVEST                  24
#define SAL_DO_DAMAGE               25

#define SAL_WAYPOINTHACK            26
#define SAL_JUST_DO_DAMAGE          27


#define NO_INDEX                    999192

#define SAL_GOTOPOINT_LIMITVEL      0.0f

//prototyped here to avoid warning!!!
void clLaunchShip(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr receiverShip);
void startTractorBeam(Ship *ship, SpaceObjRotImpTargGuidanceShipDerelict *target);

SalCapCorvetteStatics SalCapCorvetteStatic;
SalCapCorvetteStatics SalCapCorvetteStaticRace1;
SalCapCorvetteStatics SalCapCorvetteStaticRace2;
SalCapCorvetteStatics SalCapCorvetteStaticDawg;

scriptStructEntry SalCapCorvetteStaticTable[] =
{
    { "HealthThreshold",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.HealthThreshold), (udword) &(SalCapCorvetteStatic) },
    { "healthRemovedPerSecond",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.healthRemovedPerSecond), (udword) &(SalCapCorvetteStatic) },
    { "getTechTime",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.getTechTime), (udword) &(SalCapCorvetteStatic) },
    { "flyToDistance",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.flyToDistance), (udword) &(SalCapCorvetteStatic) },
    { "maxPushingVelocitySingle",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.maxPushingVelocitySingle), (udword) &(SalCapCorvetteStatic) },
    { "noLightClampingDistance",    scriptSetReal32CB, (udword) &(SalCapCorvetteStatic.noLightClampingDistance), (udword) &(SalCapCorvetteStatic) },

    { NULL,NULL,0,0 }
};


udword salvageNumNeeded(Ship *ship, SpaceObjRotImpTarg *target);
void salvageSetDockVector(Ship *ship, Ship *dockwith,vector *trackHeading, vector *trackUp,vector *coneheadingInWorldCoordSys);
bool salvageTargetTrackVector(Ship *ship, vector *trackvector);
bool handOffTargetToDockWith(Ship *ship);
void SalCapRemoveShipReferences(Ship *ship, Ship *shiptoremove);


void SalCapCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    SalCapCorvetteStatics *corvstat;
    if(statinfo->shiprace == Traders)
    {
        corvstat = &SalCapCorvetteStaticDawg;
    }
    else
    {
        corvstat = (statinfo->shiprace == R1) ? &SalCapCorvetteStaticRace1 : &SalCapCorvetteStaticRace2;
    }
    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);
    scriptSetStruct(directory,filename,SalCapCorvetteStaticTable,(ubyte *)corvstat);
    statinfo->custstatinfo = corvstat;
}
/*-----------------------------------------------------------------------------
    Name        : CleanSalCapSpec(ship)
    Description : initializes the sal/cap's special variables
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void CleanSalCapSpec(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    spec->target = NULL;
    if(!spec->groupme)
        spec->dockwith = NULL;
    spec->dockindex = 0;
    spec->salvageIndex = NO_INDEX;            //init it with this!
    spec->salvageState = SALVAGE_BEGIN;
    spec->salvageAttributes = 0;

    spec->timeCounter = 0.0f;
    spec->getTechTime=0.0f;
    spec->tractorBeam=FALSE;
    spec->noAvoid=FALSE;
    bitClear(ship->specialFlags,SPECIAL_BrokenFormation);
}


void SalCapCorvetteInit(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    attackSideStepInit(&spec->attacksidestep);
    spec->groupme = FALSE;
    CleanSalCapSpec(ship);
    spec->noDamageTarget = NULL;
    spec->noDamageTargetTime = 0.0f;
    ship->specialFlags |= SPECIAL_IsASalvager;       // set flag for the dawg and salcap ships
}

/*
void SalCapCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    SalCapCorvetteStatics *corvstat = (SalCapCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}
void SalCapCorvetteAttackPassive(Ship *ship,Ship *target,bool rotate)
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
*/
//temporary test to generate a derelict
bool SalCapCorvetteSpecialActivate(Ship *ship)
{
    vector place;
    matGetVectFromMatrixCol3(place,ship->rotinfo.coordsys);
    vecScalarMultiply(place,place, 2000.0f);

    vecAddTo(place, ship->posinfo.position);

    univAddDerelict(Ghostship,&place);
    return TRUE;

}

/*-----------------------------------------------------------------------------
    Name        : startTractorBeam
    Description : Starts the tractor beam effect from ship to target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//make tractor beam originate from 'tractor beam light'
void startTractorBeam(Ship *ship, SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    //ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    //TractorBeamStatic *tractorBeamStatic = shipstatic->tractorEmitterStatic;
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    //vector beamposition;
    //matrix beamcoordsys;
    //vector beamtrajectory;
    //real32 beamtrajectorydist;

    etgeffectstatic *stat;
    etglod *etgLOD;
    sdword LOD;

    spec->tractorBeam = TRUE;
    //get tractor beams world coords...not yet known if there is a guide light for that though
    //matMultiplyMatByVec(&beamposition,&ship->rotinfo.coordsys,&resNozzleStatic->position);

    //beamposition = ship->posinfo.position;

    //matMultiplyMatByVec(&beamposition,&ship->rotinfo.coordsys,&tractorBeamStatic->position);
    //vecAddTo(beamposition,ship->posinfo.position);
/*
    vecDivideByScalar(beamtrajectory,beamtrajectorydist,temp);

    matCreateCoordSysFromHeading(&beamcoordsys,&beamtrajectory);
*/
    //create an effect for bullet, if applicable
    etgLOD = etgTractorBeamEffectTable[ship->shiprace];
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
        ship->rceffect = etgEffectCreate(stat, ship, NULL, NULL, NULL, ship->magnitudeSquared, SOF_AttachPosition|SOF_AttachCoordsys|SOF_AttachNLips, 0);
        ship->rceffect->posinfo.velocity.x = ship->rceffect->posinfo.velocity.y = ship->rceffect->posinfo.velocity.z = 0.0f;
        ((SalCapCorvetteSpec *)ship->ShipSpecifics)->tractorBeam = TRUE;
        univUpdateObjRotInfo((SpaceObjRot *)ship->rceffect);
        ship->soundevent.specialHandle = soundEvent(ship, Ship_Salvage);
    }
    else
    {
        ship->rceffect = NULL;                              //play no effect
    }
}

/*-----------------------------------------------------------------------------
    Name        :   StopTractorBeam
    Description :   Stops the tractor beam effect...
                    Assumes a definite tractor beam existance
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void StopTractorBeam(Ship *ship)
{
    etgEffectDelete(ship->rceffect);
    univRemoveObjFromRenderList((SpaceObj *)ship->rceffect);
    listDeleteNode(&ship->rceffect->objlink);
    ship->rceffect = NULL;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapGetMaxPushingVelocity
    Description :   Determines the maximum velocity a salcap
                    target should fly at (assumes lights are attached)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 salCapGetMaxPushingVelocity(Ship *ship,Ship *target)
{
    real32 ratio;
    if(target->salvageInfo == NULL)
    {
        if(target->objtype == OBJ_ShipType)
            return(tacticsGetShipsMaxVelocity(target));
        else
            return(target->staticinfo->staticheader.maxvelocity);
    }
    ratio = (1-(((real32)target->salvageInfo->numSalvagePointsFree)/((real32)target->salvageInfo->numSalvagePoints)));
    if(target->objtype == OBJ_ShipType)
        ratio*=tacticsGetShipsMaxVelocity(target);
    else
        ratio*=target->staticinfo->staticheader.maxvelocity;
    return ratio;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToWithInCone
    Description :   Flys ship to within the docking cone
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_DOCK1_DISTANCE   1000.0f
#define DOCK1_INSIDE_TOLERANCE         200.0f

sdword salCapFlyToWithInCone(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMag;
    real32 dotprod;
    vector destination;

    SalvageStaticPoint *salvagestaticpoint = target->salvageInfo->salvagePoints[spec->salvageIndex].salvageStaticPoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,target->posinfo.position);

    vecSub(coneToShip,ship->posinfo.position,conepositionInWorldCoordSys);
    coneToShipMag = fsqrt(vecMagnitudeSquared(coneToShip));

    dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

    if (dotprod >= (salvagestaticpoint->coneangle*coneToShipMag))
    {
        return TRUE;
    }

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,SALCAP_DOCK1_DISTANCE);
    vecAddTo(destination,conepositionInWorldCoordSys);

    if ((MoveReachedDestinationVariable(ship,&destination,DOCK1_INSIDE_TOLERANCE)))
    {
        return TRUE;
    }
    else
    {
        aishipFlyToPointAvoidingObjs(ship,&destination,AISHIP_PointInDirectionFlying, 0.0f);
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToConeOrigin
    Description :   Flys ship to docking cone origin
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_FINAL_DOCK_TOLERANCE 25.0f
sdword salCapFlyToConeOrigin(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector desiredHeading;

    SalvageStaticPoint *salvagestaticpoint = target->salvageInfo->salvagePoints[spec->salvageIndex].salvageStaticPoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,target->posinfo.position);


    if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,SALCAP_FINAL_DOCK_TOLERANCE))
    {
        ship->posinfo.velocity = target->posinfo.velocity;
        return TRUE;
    }
    else
    {
        vecScalarMultiply(desiredHeading,coneheadingInWorldCoordSys,-1.0f);
        aitrackHeading(ship,&desiredHeading,1.0);
        aishipFlyToPointAvoidingObjs(ship,&conepositionInWorldCoordSys,0,0.0f);
        return FALSE;
    }
}
/*-----------------------------------------------------------------------------
    Name        :   salCapFlyOutOfCone
    Description :   Flys ship out of cone in an orderly way
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_BACKOUT_TOLERANCE 250.0f
#define SALCAP_BACKOUT_DISTANCE  1300.0f
sdword salCapFlyOutOfCone(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target,sdword trackheading)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;
    real32 backoutdist = SALCAP_BACKOUT_DISTANCE;

    SalvageStaticPoint *salvagestaticpoint = target->salvageInfo->salvagePoints[spec->salvageIndex].salvageStaticPoint;

    if (target->staticinfo->staticheader.staticCollInfo.collspheresize > backoutdist)
    {
        backoutdist = target->staticinfo->staticheader.staticCollInfo.collspheresize + SALCAP_BACKOUT_TOLERANCE;   // make sure we clear it
    }

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,target->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,backoutdist);
    vecAddTo(destination,target->posinfo.position);

    if (MoveReachedDestinationVariable(ship,&destination,SALCAP_BACKOUT_TOLERANCE))
    {
        ship->posinfo.velocity = target->posinfo.velocity;
        return TRUE;
    }
    else
    {
        if(trackheading)
            aitrackHeading(ship,&coneheadingInWorldCoordSys,1.0);
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,-50.0f,&target->posinfo.velocity);
        return FALSE;
    }


}

bool isThereAnotherTargetForMe(Ship *ship,SelectAnyCommand *targets)
{
    sdword i,index;
    SpaceObjRotImpTarg *t;
    sdword numFree;

    index = targets->numTargets;
    for(i=0;i<targets->numTargets;i++)
    {
        t = targets->TargetPtr[i];
        if(t->objtype == OBJ_ShipType)
        {
            if(bitTest(((Ship *)t)->specialFlags,SPECIAL_SalvagedTargetGoingIntoDockWithShip))
            {
                //ship is docking having been salvaged...
                //let it dock and don't target it!
                continue;
            }
            switch( ((Ship *)t)->shiptype)
            {
            case Mothership:
            case HeadShotAsteroid:
            case Drone:
            case P1Mothership:
            case P2Mothership:
            case P3Megaship:
            case FloatingCity:
            case CargoBarge:
            case MiningBase:
            case ResearchStation:
            case JunkYardDawg:
            case JunkYardHQ:
            //case Ghostship:       Ghostship is now salvageable!
            case ResearchShip:
            case Junk_LGun:
            case Junk_SGun:
                //fix later! negatory speech event!
                continue;
            case SalCapCorvette:
                {
                    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)((Ship *)t)->ShipSpecifics;
                    if(spec->salvageState > SAL_WL_FLYTOCONEORIGIN)
                    {
                        continue;
                    }
                }
                break;
            default:
                //valid ship
                break;
            }
        }
        else if(t->objtype == OBJ_DerelictType)
        {
            if(!multiPlayerGame)
            {
                //don't salavage derelicts in single player game!
                if(!((Derelict *)t)->staticinfo->salvageable)
                {
                    continue;
                }
                if(t->objtype == OBJ_DerelictType)
                {
                    if(((Derelict *)t)->attributes & ATTRIBUTES_StripTechnology)
                    {
                        //derelict is to be stripped of TECH !!
                        //so only let ONE ship attach to it lest we
                        //have to add other code...
                        if(((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex] > 0)
                        {
                            //no good....its been targetted already by this player
                            continue;
                        }
                    }
                }
            }
        }
        else
        {
            continue;   //bad target in list..ok though
        }
        if(((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageInfo != NULL)
        {
            numFree =((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageInfo->numSalvagePoints - ((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex];
        }
        else
        {
            //no salvage lights, so only 1 possible spot
            numFree = 1 - ((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex];
        }
        if(numFree == 0)
        {
            //no spots...so shouldn't head to ship
            continue;
        }
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapGetTarget
    Description :   returns a valid (best) target from list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

SpaceObjRotImpTargGuidanceShipDerelict *salCapGetTarget(Ship *ship,SelectAnyCommand *targets, sdword worry, sdword *flyandwait)
{
    sdword i,index;
    SpaceObjRotImpTarg *t;
    real32 dist = REALlyBig;
    real32 tempdist;
    vector sep;
    sdword numFree;
    sdword flyandwaitvar=-1;

    //variable set to true if target is 'busyied' by other salcaps...salcap will fly close and just wait until
    //the target is no longer worthless.
    *flyandwait = FALSE;

    index = targets->numTargets;
    for(i=0;i<targets->numTargets;i++)
    {
        t = targets->TargetPtr[i];
        if(t->objtype == OBJ_ShipType)
        {
            if(bitTest(((Ship *)t)->specialFlags,SPECIAL_SalvagedTargetGoingIntoDockWithShip))
            {
                //ship is docking having been salvaged...
                //let it dock and don't target it!
                continue;
            }
            switch( ((Ship *)t)->shiptype)
            {
            case Mothership:
            case HeadShotAsteroid:
            case Drone:
            case P2Mothership:
            case P3Megaship:
            case FloatingCity:
            case CargoBarge:
            case MiningBase:
            case ResearchStation:
            case JunkYardDawg:
            case JunkYardHQ:
            //case Ghostship:       Ghostship is now salvageable!
            case ResearchShip:
            case Junk_LGun:
            case Junk_SGun:
                //fix later! negatory speech event!
                continue;
            case GravWellGenerator:
                if(((GravWellGeneratorSpec *)((Ship *)t)->ShipSpecifics)->GravDerelict)
                {
                    //gravwell is a derelict
                    //don't salavage it

                    //maybe put speech event?
                    continue;
                }
            case SalCapCorvette:
                {
                    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)((Ship *)t)->ShipSpecifics;
                    if(spec->salvageState > SAL_WL_FLYTOCONEORIGIN)
                    {
                        continue;
                    }
                }
                break;
            default:
                //valid ship
                break;
            }
        }
        else if(t->objtype == OBJ_DerelictType)
        {
            if(!multiPlayerGame)
            {
                //don't salavage derelicts in single player game!
                if(!((Derelict *)t)->staticinfo->salvageable)
                {
                    continue;
                }
                if(t->objtype == OBJ_DerelictType)
                {
                    if(((Derelict *)t)->attributes & ATTRIBUTES_StripTechnology)
                    {
                        //derelict is to be stripped of TECH !!
                        //so only let ONE ship attach to it lest we
                        //have to add other code...
                        if(((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex] > 0)
                        {
                            //no good....its been targetted already by this player
                            continue;
                        }
                    }
                }
            }
        }
        else
        {
            //shouldn't be in list...
            targets->numTargets--;
            targets->TargetPtr[i]=targets->TargetPtr[targets->numTargets];
            continue;
        }
        if(((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageInfo != NULL)
        {
            numFree =((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageInfo->numSalvagePoints - ((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex];
        }
        else
        {
            //no salvage lights, so only 1 possible spot
            numFree = 1 - ((SpaceObjRotImpTargGuidanceShipDerelict *)t)->salvageNumTagged[ship->playerowner->playerIndex];
        }
        if(numFree == 0)
        {
            //no spots...so shouldn't head to ship
            if((!*flyandwait))
            {
                flyandwaitvar = i;
                *flyandwait=TRUE;
            }
            continue;
        }
        vecSub(sep,t->posinfo.position,ship->posinfo.position);
        tempdist = vecMagnitudeSquared(sep);
        if(tempdist < dist)
        {
            index = i;
            dist = tempdist;
        }

    }

    if(index == targets->numTargets)
    {
        //if (battleCanChatterAtThisTime(BCE_CannotComply, ship))
        //{
        //    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, ship, SOUND_EVENT_DEFAULT);
        //}
        if((*flyandwait) && worry)
        {
            return((SpaceObjRotImpTargGuidanceShipDerelict *)targets->TargetPtr[flyandwaitvar]);
        }
        else
        {
            return NULL;
        }
    }
    *flyandwait = FALSE;    //reset to FALSE here incase multiple targets in list.
    ((SpaceObjRotImpTargGuidanceShipDerelict *)targets->TargetPtr[index])->salvageNumTagged[ship->playerowner->playerIndex]++;

    return((SpaceObjRotImpTargGuidanceShipDerelict *)targets->TargetPtr[index]);
}

void docapture(Ship *ship,Ship *target)
{
    if(target->shiptype == CryoTray)
    {
        //don't capture a cryotray
        return;
    }

    if(universe.curPlayerIndex == ship->playerowner->playerIndex)
    {
        speechEventFleetSpec(ship, STAT_F_EnemyShipCaptured,target->shiptype,ship->playerowner->playerIndex);
    }
    if(ship->shiptype == Mothership)
    {
        MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
        if(spec->CAP_NumInBuildCue < (SALVAGE_MAX_CAPTURABLE-1))
        {
            spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue] = max(SALVAGE_MIN_RETROFIT_TIME,SALVAGE_MAX_RETROFIT_TIME*(1.0f - target->health/target->staticinfo->maxhealth));
            spec->CAPshiptype[spec->CAP_NumInBuildCue] = target->shiptype;
            spec->CAPshiprace[spec->CAP_NumInBuildCue] = target->staticinfo->shiprace;
            spec->CAPcolorScheme[spec->CAP_NumInBuildCue] = target->colorScheme;
            spec->CAP_NumInBuildCue++;
        }
    }
    else if(ship->shiptype == Carrier)
    {
        CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
        if(spec->CAP_NumInBuildCue < (SALVAGE_MAX_CAPTURABLE-1))
        {
            spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue] = max(SALVAGE_MIN_RETROFIT_TIME,SALVAGE_MAX_RETROFIT_TIME*(1.0f - target->health/target->staticinfo->maxhealth));
            spec->CAPshiptype[spec->CAP_NumInBuildCue] = target->shiptype;
            spec->CAPshiprace[spec->CAP_NumInBuildCue] = target->staticinfo->shiprace;
            spec->CAPcolorScheme[spec->CAP_NumInBuildCue] = target->colorScheme;
            spec->CAP_NumInBuildCue++;
        }
    }
    else
    {
        dbgFatalf(DBG_Loc,"\nUnknown type of ship receiveing captured ship.  NOT a carrier or mothership");
    }

}

/*-----------------------------------------------------------------------------
    Name        :   salCapHarvestTarget
    Description :   name says it all!
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void salCapHarvestTarget(SpaceObjRotImpTargGuidanceShipDerelict *target,Ship *dockwith)
{
    bool spm3AlreadySalvaged = FALSE;   // TRUE if the player has already salvaged a frigate in Mission 3

    switch(target->objtype)
    {
    case OBJ_ShipType:
        if (!singlePlayerGame)
        {
            if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
            {
                if ((isCapitalShip((Ship *)target)) &&
                    (((Ship *)target)->playerowner->playerIndex!=dockwith->playerowner->playerIndex) &&
                    (!allianceIsShipAlly((Ship *)target,dockwith->playerowner)) )
                {
                    universe.CapitalShipCaptured |= (1 << ((Ship *)target)->playerowner->playerIndex);
                    universe.PlayerWhoWon |= (1 << dockwith->playerowner->playerIndex);

                    if (((Ship *)target)->playerowner->playerState == PLAYER_ALIVE)
                    {
                        univKillPlayer(((Ship *)target)->playerowner->playerIndex,PLAYERKILLED_CAPTUREDSHIP);
                        CheckPlayerWin();
                    }
                }
            }
        }
        else
        {
            AIVar *var;

            //more specific aivars created for capturing cryotrays and assault frigates
            if (((Ship *)target)->shiptype == CryoTray)
            {
                if ((var = aivarFind("SalvageCryoTray")) == NULL)
                {
                    var = aivarCreate("SalvageCryoTray");
                }
                aivarValueSet(var, TRUE);
            }
            else if (((Ship *)target)->shiptype == StandardFrigate)
            {
                if ((var = aivarFind("SalvageFrigate")) == NULL)
                {
                    var = aivarCreate("SalvageFrigate");
                }
                else
                {
                    spm3AlreadySalvaged = TRUE;
                }
                aivarValueSet(var, TRUE);
            }
            else
            {
                if ((var = aivarFind("SalvageTS")) == NULL)
                {
                    var = aivarCreate("SalvageTS");
                }
                aivarValueSet(var, TRUE);
            }
        }

        //!!!Single Player Game Mission Specific Code!!!
        if((((Ship *)target)->shiptype == ResearchShip) ||
           (((Ship *)target)->shiptype == TargetDrone) ||
           (singlePlayerGame &&
            (((Ship *)target)->shiptype == StandardFrigate) &&
            (singlePlayerGameInfo.currentMission == 3) &&
            (!spm3AlreadySalvaged)))
        {
            //don't capture research ships
            //don't capture targetdrones
            //don't capture the first frigate in singleplayer game mission 3
            dockwith->playerowner->resourceUnits = (sdword)( dockwith->playerowner->resourceUnits + ((Ship *)target)->staticinfo->buildCost*0.7f);
            universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected = (sdword) (universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected + ((Ship *)target)->staticinfo->buildCost*0.7f);
        }
        else
        {
            docapture(dockwith,(Ship *)target);
        }

        //check if we just brutalized the player
        univKillOtherPlayersIfDead((Ship*)target);

        //ditch ship from universe
        bitSet(target->specialFlags,SPECIAL_Harvested);
        univRemoveShipFromOutside((Ship *)target);
        ((Ship *)target)->howDidIDie = DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED;
        ((Ship *)target)->whoKilledMe = dockwith->playerowner->playerIndex;
        univDeleteWipeInsideShipOutOfExistence((Ship *)target);
        break;
    case OBJ_DerelictType:
        //later make own univRemoveDerelictFromOutside)
        dockwith->playerowner->resourceUnits += 250;
        universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected+= 250;
        bitSet(target->specialFlags,SPECIAL_Harvested);
        AddTargetToDeleteList((SpaceObjRotImpTarg *)target,-1);
        break;
#ifdef DEBUG_SALCAP
    default:
        dbgFatalf(DBG_Loc,"\nSal/Cap: Trying to convert an unknown object type to RU's.");
        break;
#endif
    }
    //thanks lukina de pooina
    //ship->playerowner->resourceUnits -= ship->playerowner->resourceUnits / 3;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapGetAndBusySalvageIndex
    Description :   finds the best docking index to use
                    and busy's it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword salCapGetAndBusySalvageIndex(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    sdword i;
    SalvagePoint *salvagePoint;
    if(target->salvageInfo != NULL)
    {
        for (i = 0;i<target->salvageInfo->numSalvagePoints;i++)
        {
            salvagePoint = &target->salvageInfo->salvagePoints[i];
            if(!salvagePoint->busy)
            {
                //not busy
                salvagePoint->busy = TRUE;
                target->salvageInfo->numSalvagePointsFree--;
                return i;
            }
        }
    return NO_INDEX;
    }
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nShips salvage static info doesn't exist!.");
#endif
    return NO_INDEX;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapUnBusySalvagePoint
    Description :   unbusy's targets docking point
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void salCapUnBusySalvagePoint(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    if(spec->target->salvageInfo != NULL)
    {
        target->salvageInfo->salvagePoints[spec->salvageIndex].busy = 0;
        target->salvageInfo->numSalvagePointsFree++;
        if(ship->clampInfo != NULL)
        {
            //don't decrease this quantity unless we're clamped to the ship!
            dbgAssert(ship->clampInfo->host == ((SpaceObjRotImpTarg *)target));
            target->salvageInfo->numNeededForSalvage++;
            if(spec->target->salvageInfo->numNeededForSalvage > spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage)
            {
                spec->target->salvageInfo->numNeededForSalvage = spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage;
            }
        }
     }
    spec->salvageIndex = NO_INDEX;
}

sdword salCapFitCarrier(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    sdword needBig;

    if(target->salvageInfo != NULL)
    {
       needBig = target->staticinfo->salvageStaticInfo->willFitCarrier;
    }
    else
    {
        if(target->objtype==OBJ_DerelictType)
        {
            needBig = FALSE;
        }
        else if(((Ship *)target)->staticinfo->shipclass == CLASS_HeavyCruiser ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Carrier ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Destroyer ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Frigate ||
                ((Ship *)target)->staticinfo->shiptype == ResourceController ||
                ((Ship *)target)->staticinfo->shiptype == CryoTray)
        {
            needBig = FALSE;
        }
        else
        {
            needBig = TRUE;
        }
    }
    return needBig;
}
/*-----------------------------------------------------------------------------
    Name        :   salCapNeedBig
    Description :   returns needbig status..:) whether or not
                    a mothership bay door is required for salvage deposit
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword salCapNeedBig(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    sdword needBig;

    if(target->salvageInfo != NULL)
    {
        if(ship->shiprace == R1)
        {
            needBig = target->staticinfo->salvageStaticInfo->needBigR1;
        }
        else
        {
            needBig = target->staticinfo->salvageStaticInfo->needBigR2;
        }
    }
    else
    {
        if(target->objtype==OBJ_DerelictType)
        {
            needBig = TRUE;
        }
        else if(((Ship *)target)->staticinfo->shipclass == CLASS_HeavyCruiser ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Carrier ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Destroyer ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Frigate ||
                ((Ship *)target)->staticinfo->shiptype == CryoTray)
        {
            needBig = TRUE;
        }
        else
        {
            needBig = FALSE;
        }
    }
    return needBig;
}
/*-----------------------------------------------------------------------------
    Name        :   salCapFindDockWith
    Description :   finds a viable ship for the target to dockwith
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Ship *salCapFindDockWith(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    Node *node = universe.ShipList.head;
    Ship *dockwith,*dockwithpos;
    sdword needBig;
    real32 mindist,distsqr;
    vector distvec;

    //do we need a mothership?
    needBig = !salCapFitCarrier(ship,target);

    if(needBig)
    {
        if(ship->playerowner != NULL)
        {
            if(ship->playerowner->PlayerMothership != NULL)
            {
                if(ship->playerowner->PlayerMothership->shiptype == Mothership)
                {
                    return(ship->playerowner->PlayerMothership);
                }
            }
        }
        return NULL;    //motherships 'big' bay not found
    }

    dockwith = NULL;
    mindist = REALlyBig;

    //searching for mothership or carrier here! or other dockable ship

    while(node != NULL)
    {
        dockwithpos = (Ship *)listGetStructOfNode(node);
        if(dockwithpos->shiptype != Carrier && dockwithpos->shiptype != Mothership )
            goto nextnode;
        else if(dockwithpos->playerowner != ship->playerowner)
            goto nextnode;

        //found players carrier

        vecSub(distvec,dockwithpos->posinfo.position,spec->target->posinfo.position);
        distsqr = vecMagnitudeSquared(distvec);
        if(distsqr < mindist)
        {
            mindist = distsqr;
            dockwith = dockwithpos;
        }

nextnode:
        node = node->next;
    }
    return dockwith;
}


/*-----------------------------------------------------------------------------
    Name        :   salCapSetNewCoordSys
    Description :   Sets up targets coordinate system based on specified salvage lights
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void salCapSetNewCoordSys(SpaceObjRotImpTargGuidanceShipDerelict *target,vector *heading,vector *velocityN)
{
    sdword i;
    sdword good = FALSE;

    *velocityN = target->posinfo.velocity;
    vecNormalize(velocityN);

    for(i=0;i<target->salvageInfo->numSalvagePoints;i++)
    {
        if(target->salvageInfo->salvagePoints[i].salvageStaticPoint->type == Heading)
        {
            matMultiplyMatByVec(heading,&target->rotinfo.coordsys,&target->salvageInfo->salvagePoints[i].salvageStaticPoint->conenormal);
            good = TRUE;
            break;
        }
    }

    if(!good)
    {
        #ifdef DEBUG_SAL
              dbgMessagef("\nNO HEADING VECTOR FOUND! in SALVAGEINFO");
        #endif
        *heading = *velocityN;
        return;
    }
}
/*-----------------------------------------------------------------------------
    Name        :   salCapTrackHeadingAndUp
    Description :   tracks the docking heading if the lights are placed

    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword salCapTrackHeadingAndUp(SpaceObjRotImpTargGuidanceShipDerelict *target,real32 withinAngle)
{
    matrix rotMatrix;
    vector fakeHeading,velocityN;
    vector realHeading,neededHeading,planePerp;
    real32 cosAngle,Angle;

    //create coordsys from salvage heading lights...
    //salCapSetNewCoordSys(target,&fakeHeading,&velocityN);
    velocityN = target->posinfo.velocity;
    vecNormalize(&velocityN);

    aitrackHeadingWithFlags((Ship *)target,&velocityN,0.999f,0);
    return FALSE;

    matGetVectFromMatrixCol1(realHeading,target->rotinfo.coordsys);

    cosAngle = vecDotProduct(fakeHeading,velocityN);
    if(cosAngle > 1.0f)
        cosAngle = 1.0f;
    else if(cosAngle < -1.0f)
        cosAngle = -1.0f;

    Angle = acos(cosAngle);

    if(Angle <= withinAngle)
    {
        //no need to adjust
        return TRUE;
    }

    vecCrossProduct(planePerp,velocityN,fakeHeading);
    vecNormalize(&planePerp);

    nisRotateAboutVector((real32 *) &rotMatrix, &planePerp, Angle);

    matMultiplyVecByMat(&neededHeading,&realHeading,&rotMatrix);
    //matMultiplyMatByVec(&neededHeading,&rotMatrix,&realHeading);

    aitrackHeadingWithFlags((Ship *)target,&neededHeading,0.999f,AITRACKHEADING_IGNOREUPVEC);

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToWithinDockingRange
    Description :   fly's the target to within docking range
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SAL_WITHIN_DOCKING_RANGE    6000.0f

sdword salCapFlyToWithinDockingRange(Ship *ship)
{
    real32 maxpushvelocity;

    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    if(MoveReachedDestinationVariable((Ship *)spec->target,&spec->dockwith->posinfo.position,SAL_WITHIN_DOCKING_RANGE))
    {
        return TRUE;
    }


    //fly to point

    maxpushvelocity = salCapGetMaxPushingVelocity(ship,(Ship *)spec->target);
    if(spec->target->objtype == OBJ_ShipType)
    {
        if(((Ship *)spec->target)->specialFlags2 & SPECIAL_2_SalvageTargetAlreadyDriven)
        {
            goto dontflyme1;
        }
    }
    aishipFlyToPointAvoidingObjs((Ship *)spec->target,&spec->dockwith->posinfo.position,AISHIP_PointInDirectionFlying,maxpushvelocity);
    salCapTrackHeadingAndUp(spec->target,0.999f);
    if(spec->target->objtype == OBJ_ShipType)
    {
        ((Ship *)spec->target)->specialFlags2 |= SPECIAL_2_SalvageTargetAlreadyDriven;
    }
dontflyme1:;

    return FALSE;
}
sdword getDockIndex(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target,Ship *dockwith)
{
    sdword i;
    sdword needBig;
    sdword willfitcarrier;
    //determines if need a LARGE door
    needBig = salCapNeedBig(ship,target);
    willfitcarrier = salCapFitCarrier(ship,target);
    if(needBig && !willfitcarrier)
    {
        //needs a mothership BIG door
        //and WON'T fit in the carrier
        for(i=0;dockwith->dockInfo->numDockPoints;i++)
        {
            if(strcmp(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name,"Big") == 0)
            {
                return(i);
            }
        }
    }
    else
    {
        if(dockwith->shiprace == R1 || dockwith->shiptype == Carrier)
        {
            for(i=0;dockwith->dockInfo->numDockPoints;i++)
            {
                if(strcmp(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name,"Frigate") == 0)
                {
                    return(i);
                }
            }
        }
        else
        {
            //ship race is R2 and not a carrier!
            for(i=0;dockwith->dockInfo->numDockPoints;i++)
            {
                if(strcmp(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name,"Big") == 0)
                {
                    return(i);
                }
            }
        }
    }

    //for now die later, error handle by returning an informative flag that
    //indicates to deattach and go home with tail between legs
    dbgAssert(FALSE);

    return 0;

}
/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToDockingPoint1
    Description :   Performs a stage in the salvage Docking process
    inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_1over2Tolerance  800.0f
#define SALCAP_1over2HEADINGSCALAR 3500.0f

sdword salCapFlyToDockingPoint1over2(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    vector coneheadingInWorldCoordSys,conepositionInWorldCoordSys;
    vector destination,dockwithHeading;

    matGetVectFromMatrixCol3(dockwithHeading,spec->dockwith->rotinfo.coordsys);
    vecScalarMultiply(dockwithHeading,dockwithHeading,SALCAP_1over2HEADINGSCALAR);

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&spec->dockwith->rotinfo.coordsys,&spec->dockwith->dockInfo->dockpoints[spec->dockindex].dockstaticpoint->conenormal);
    matMultiplyMatByVec(&conepositionInWorldCoordSys,&spec->dockwith->rotinfo.coordsys,&spec->dockwith->dockInfo->dockpoints[spec->dockindex].dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,spec->dockwith->posinfo.position);

    vecAdd(destination,dockwithHeading,conepositionInWorldCoordSys);

    if (MoveReachedDestinationVariable((Ship *)target,&destination,SALCAP_1over2Tolerance))
    {
        return TRUE;
    }
    else
    {
        aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,AISHIP_PointInDirectionFlying,SAL_GOTOPOINT_LIMITVEL,&spec->dockwith->posinfo.velocity);
        return FALSE;
    }

}
/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToDockingPoint1
    Description :   Performs a stage in the salvage Docking process
    inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_ReverseDockTolerance1    1500.0f
#define SALCAP_HeadingTrackTolerance    2000.0f
//#define DOOR_DOCKING_NAV1               2500.0f
//#define DOOR_DOCKING_NAV2               575.0f
#define HEADING_SCALAR                  2500.0f
#define SALCAP_DOCKCONE_SCALAR_BIG_DOOR          1000.0f
#define SALCAP_DOCKCONE_SCALAR_OTHER          2000.0f

sdword salCapFlyToDockingPoint1(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    //SalCapCorvetteStatics *salcapcorvettestatics = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    vector coneheadingInWorldCoordSys,conepositionInWorldCoordSys;
    vector dockwithUp,dockwithHeading,dockwithRight,navpoint;
    vector desiredHeading,desiredUp,destination;
    vector positionWS,doorRight,doorUp;
    matrix coordsysWS;
    sdword needDest;
    sdword NO;

    NO = FALSE;
    if(spec->dockwith->shiptype == Mothership)
    {
        if(spec->dockwith->shiprace == R1)
        {
            if(salCapNeedBig(ship,target))
            {
                madLinkInGetDoorInfo(spec->dockwith,&coordsysWS,&positionWS);
                matGetVectFromMatrixCol1(doorUp,coordsysWS);
                matGetVectFromMatrixCol2(doorRight,coordsysWS);

                //vecScalarMultiply(doorLeft,doorRight,-1.0f);

                desiredHeading = doorUp;
                desiredUp = doorRight;

                NO = TRUE;
            }
        }
    }

        matGetVectFromMatrixCol1(dockwithUp,spec->dockwith->rotinfo.coordsys);
        matGetVectFromMatrixCol2(dockwithRight,spec->dockwith->rotinfo.coordsys);
        matGetVectFromMatrixCol3(dockwithHeading,spec->dockwith->rotinfo.coordsys);

    if(!NO)
    {

        if(spec->dockwith->shiprace == R1)
        {
            desiredHeading = dockwithUp;
            vecScalarMultiply(desiredUp,dockwithRight,-1.0f);
        }
        else
        {
            desiredHeading = dockwithHeading;
            desiredUp = dockwithUp;
        }
    }
    
    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&spec->dockwith->rotinfo.coordsys,&spec->dockwith->dockInfo->dockpoints[spec->dockindex].dockstaticpoint->conenormal);
    matMultiplyMatByVec(&conepositionInWorldCoordSys,&spec->dockwith->rotinfo.coordsys,&spec->dockwith->dockInfo->dockpoints[spec->dockindex].dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,spec->dockwith->posinfo.position);

    needDest = TRUE;
    if(spec->dockwith->shiptype == Mothership)
    {
        if(spec->dockwith->shiprace == R1)
        {
            if(salCapNeedBig(ship,target))
            {
                vecScalarMultiply(destination,coneheadingInWorldCoordSys,SALCAP_DOCKCONE_SCALAR_BIG_DOOR);
                vecAddTo(destination,conepositionInWorldCoordSys);
                vecScalarMultiply(navpoint,dockwithHeading,HEADING_SCALAR);
                vecAddTo(destination,navpoint);
                needDest=FALSE;
            }
        }
    }
    if(needDest)
    {
        vecScalarMultiply(destination,coneheadingInWorldCoordSys,SALCAP_DOCKCONE_SCALAR_OTHER);
        vecAddTo(destination,conepositionInWorldCoordSys);
    }


    if (MoveReachedDestinationVariable((Ship *)target,&destination,SALCAP_ReverseDockTolerance1))
    {
        return TRUE;
    }
    else
    {
        if (MoveReachedDestinationVariable((Ship *)target,&destination,SALCAP_HeadingTrackTolerance))
        {
            if(salCapNeedBig(ship,target))
            {
                aitrackHeadingAndUp((Ship *)target,&desiredHeading,&desiredUp,1.0);
            }
            else
            {
                vector smallHeading;
                vecScalarMultiply(smallHeading,coneheadingInWorldCoordSys,-1.0f);
                aitrackHeadingAndUp((Ship *)target,&smallHeading,&dockwithUp,1.0);
            }
            //aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,0,-salcapcorvettestatics->maxPushingVelocitySingle,&spec->dockwith->posinfo.velocity);
            aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,0,SAL_GOTOPOINT_LIMITVEL,&spec->dockwith->posinfo.velocity);
        }
        else
        {
            //aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,AISHIP_PointInDirectionFlying,-salcapcorvettestatics->maxPushingVelocitySingle,&spec->dockwith->posinfo.velocity);
            aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,AISHIP_PointInDirectionFlying,SAL_GOTOPOINT_LIMITVEL,&spec->dockwith->posinfo.velocity);
        }
        return FALSE;
    }
}
/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToDockingPoint2
    Description :   Performs a stage in the salvage Docking process
    inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define SALCAP_ReverseDockTolerance3 200.0f
#define SALCAP_STAGE2_DOCKWITHUP_SCALAR 0.0f
#define SALCAP_STAGE2_DOCKWITHRIGHT_SCALAR 1000.0f
#define SALCAP_STAGE2_DOCKWITHHEADING_SCALAR 1500.0f
#define SALCAP_WEIRD_DOCKING_DISTANCE2  2000.0f
sdword salCapFlyToDockingPoint2(Ship *dockwith,SpaceObjRotImpTargGuidanceShipDerelict *target,sdword dockindex)
{
    vector coneheadingInWorldCoordSys,conepositionInWorldCoordSys;
    vector dockwithUp,dockwithHeading,dockwithRight,navpoint;
    vector desiredHeading,desiredUp,desiredRight,destination;
    vector positionWS,doorRight,doorUp;
    matrix coordsysWS;
    sdword NO;

    NO = FALSE;
    if(dockwith->shiptype == Mothership)
    {
        if(dockwith->shiprace == R1)
        {
            if(salCapNeedBig(dockwith,target))
            {
                //madLinkInGetDoorInfo(dockwith,&coordsysWS,&positionWS);
                mothershipGetCargoPosition(dockwith,target,&positionWS,&coordsysWS,&desiredHeading,&desiredUp,&desiredRight);
                matGetVectFromMatrixCol1(doorUp,coordsysWS);
                matGetVectFromMatrixCol2(doorRight,coordsysWS);

                //vecScalarMultiply(doorLeft,doorRight,-1.0f);

                //desiredHeading = doorUp;
                //desiredUp = doorRight;

                NO = TRUE;
            }
        }
    }
    matGetVectFromMatrixCol1(dockwithUp,dockwith->rotinfo.coordsys);
    matGetVectFromMatrixCol2(dockwithRight,dockwith->rotinfo.coordsys);
    matGetVectFromMatrixCol3(dockwithHeading,dockwith->rotinfo.coordsys);

    if(!NO)
    {

        if(dockwith->shiprace == R1)
        {
            desiredHeading = dockwithUp;
            vecScalarMultiply(desiredUp,dockwithRight,-1.0f);
        }
        else
        {
            desiredHeading = dockwithHeading;
            desiredUp = dockwithUp;
        }
    }
    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->dockInfo->dockpoints[dockindex].dockstaticpoint->conenormal);
    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->dockInfo->dockpoints[dockindex].dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    destination = conepositionInWorldCoordSys;
    NO = FALSE;
    if(dockwith->shiptype == Mothership)
    {
        if(dockwith->shiprace == R1)
        {
            //if using big!
            if(salCapNeedBig(dockwith,target))
            {
                vecScalarMultiply(navpoint,dockwithUp,SALCAP_STAGE2_DOCKWITHUP_SCALAR);
                vecAddTo(destination,navpoint);
                vecScalarMultiply(navpoint,dockwithRight,-SALCAP_STAGE2_DOCKWITHRIGHT_SCALAR);
                vecAddTo(destination,navpoint);
                vecScalarMultiply(navpoint,dockwithHeading,SALCAP_STAGE2_DOCKWITHHEADING_SCALAR);
                vecAddTo(destination,navpoint);
                NO = TRUE;
            }
        }
    }
    if(!NO)
    {
        vecScalarMultiply(navpoint,coneheadingInWorldCoordSys,SALCAP_WEIRD_DOCKING_DISTANCE2);
        vecAddTo(destination,navpoint);
    }
    if (MoveReachedDestinationVariable((Ship *)target,&destination,SALCAP_ReverseDockTolerance3))
    {
        return TRUE;
    }
    else
    {
        if(salCapNeedBig(dockwith,target))
        {
            aitrackHeadingAndUp((Ship *)target,&desiredHeading,&desiredUp,1.0);
        }
        else
        {
            vector smallHeading;
            vecScalarMultiply(smallHeading,coneheadingInWorldCoordSys,-1.0f);
            aitrackHeadingAndUp((Ship *)target,&smallHeading,&dockwithUp,1.0);
        }
        //aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,0,-salcapcorvettestatics->maxPushingVelocitySingle,&spec->dockwith->posinfo.velocity);
        aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,0,SAL_GOTOPOINT_LIMITVEL,&dockwith->posinfo.velocity);
        return FALSE;
    }


    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        :   salCapFlyToDockingPoint3
    Description :   Performs a stage in the salvage Docking process
    inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

sdword salCapFlyToDockingPoint3(Ship *dockwith,SpaceObjRotImpTargGuidanceShipDerelict *target,sdword dockindex)
{
    vector coneheadingInWorldCoordSys,conepositionInWorldCoordSys;
    vector dockwithUp,dockwithHeading,dockwithRight;
    vector desiredHeading,desiredUp,desiredRight,destination;

    vector doorRight,doorUp;
    matrix coordsysWS;
    sdword NO;

    NO = FALSE;
    if(dockwith->shiptype == Mothership)
    {
        if(dockwith->shiprace == R1)
        {
            if(salCapNeedBig(dockwith,target))
            {
                mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)target, &destination,&coordsysWS,&desiredHeading,&desiredUp,&desiredRight);
                //madLinkInGetDoorInfo(spec->dockwith,&coordsysWS,&positionWS);
                matGetVectFromMatrixCol1(doorUp,coordsysWS);
                matGetVectFromMatrixCol2(doorRight,coordsysWS);

                //vecScalarMultiply(doorLeft,doorRight,-1.0f);

                //desiredHeading = doorUp;
                //desiredUp = doorRight;

                NO = TRUE;
            }
        }
    }
        matGetVectFromMatrixCol1(dockwithUp,dockwith->rotinfo.coordsys);
        matGetVectFromMatrixCol2(dockwithRight,dockwith->rotinfo.coordsys);
        matGetVectFromMatrixCol3(dockwithHeading,dockwith->rotinfo.coordsys);

    if(!NO)
    {
        if(dockwith->shiprace == R1)
        {
            desiredHeading = dockwithUp;
            vecScalarMultiply(desiredUp,dockwithRight,-1.0f);
        }
        else
        {
            desiredHeading = dockwithHeading;
            desiredUp = dockwithUp;
        }
        matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->dockInfo->dockpoints[dockindex].dockstaticpoint->conenormal);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->dockInfo->dockpoints[dockindex].dockstaticpoint->position);
        vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

        destination = conepositionInWorldCoordSys;
    }

    if (MoveReachedDestinationVariable((Ship *)target,&destination,TW_R1_DOOR_DOCK_TOLERANCE))
    {
        return TRUE;
    }
    else
    {
        if(salCapNeedBig(dockwith,target))
        {
            //if using big!
            /*
            if(spec->dockwith->shiprace == R1)
            {
                //do final door matrix tracking
                matrix doormatSS,doormatWS;
                vector doorposSS,doorposWS;
                madLinkInGetDoorInfo(spec->dockwith,&doormatSS,&doorposSS);
                matMultiplyMatByMat(&doormatWS,&spec->dockwith->rotinfo.coordsys,&doormatSS);
                matMultiplyVecByMat(&doorposWS,&doorposSS,&spec->dockwith->rotinfo.coordsys);
                vecAddTo(doorposWS,spec->dockwith->posinfo.position);


                matGetVectFromMatrixCol1(desiredHeading,doormatWS);
                matGetVectFromMatrixCol2(dockwithRight,doormatWS);
                vecScalarMultiply(desiredUp,dockwithRight,-1.0f);

            }
            */
            aitrackHeadingAndUp((Ship *)target,&desiredHeading,&desiredUp,1.0);

        }
        else
        {
            vector smallHeading;
            vecScalarMultiply(smallHeading,coneheadingInWorldCoordSys,-1.0f);
            aitrackHeadingAndUp((Ship *)target,&smallHeading,&dockwithUp,1.0);
        }
        aishipFlyToPointAvoidingObjsWithVel((Ship *)target,&destination,0,SAL_GOTOPOINT_LIMITVEL,&dockwith->posinfo.velocity);
        return FALSE;
    }


    return FALSE;
}

void salCapPointAtTarget(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    vector conepositionInWorldCoordSys,heading;

    SalvageStaticPoint *salvagestaticpoint;
    if(target->salvageInfo != NULL)
    {
        salvagestaticpoint = target->salvageInfo->salvagePoints[spec->salvageIndex].salvageStaticPoint;
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&target->rotinfo.coordsys,&salvagestaticpoint->position);
        vecAddTo(conepositionInWorldCoordSys,target->posinfo.position);

        vecSub(heading,conepositionInWorldCoordSys,ship->posinfo.position);
        vecNormalize(&heading);
    }
    else
    {
        conepositionInWorldCoordSys = target->posinfo.position;
        vecSub(heading,conepositionInWorldCoordSys,ship->posinfo.position);
        vecNormalize(&heading);
    }
    aitrackHeading(ship,&heading,0.999f);
}

void removeTargetFromSelection(SpaceObjRotImpTarg *target,SelectAnyCommand *targets)
{
    sdword i=0;
    for(i=0;i<targets->numTargets;)
    {
        if(target == targets->TargetPtr[i])
        {
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            continue;
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        :   SalCapCorvetteSpecialTarget
    Description :   Performs salvageing!  Hopefully
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool SalCapCorvetteSpecialTarget(Ship *ship, void *custom)
{
    SelectAnyCommand *targets;
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    SalCapCorvetteStatics *salcapcorvettestatics = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    SelectCommand selection;
    sdword waittarget;
    targets = (SelectAnyCommand *)custom;
    if(targets->numTargets == 0
        && spec->target == NULL)
    {
        //no targets left, so cleanup and finish
        SelectCommand selection;

        spec->tractorBeam = FALSE;
        //if(spec->target != NULL)
        //{
        //    bitClear(((Ship *)spec->target)->dontrotateever,1);
        //    bitClear(((Ship *)spec->target)->dontapplyforceever,1);
       // }
        //make sure clampinfo doesn't get fucked here!
        if(ship->rceffect)
            StopTractorBeam(ship);
        CleanSalCapSpec(ship);
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        clHalt(&universe.mainCommandLayer,&selection);
        return TRUE;
    }

    if(spec->target == NULL)
    {
        if(spec->salvageState != SALVAGE_BEGIN )
        {
            if(ship->rceffect != NULL)
                StopTractorBeam(ship);
            CleanSalCapSpec(ship);
            return FALSE;           //let it re-enter and do normal stuff.
        }
    }

    if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
    {
        ship->attackvars.attacktarget=targets->TargetPtr[0];
        doKamikazeAttack(ship,targets->TargetPtr[0]);
        return FALSE;
    }

    //check if target has been harvested and not had references removed this
    //univupdate <will become obsolete soon when harvesting code is outside the salcap stuff>
    if(spec->target != NULL)
    {
        if(spec->target->specialFlags & SPECIAL_Harvested)
        {
            //target JUST harvested this univupdate...so ditch it..
            spec->tractorBeam = FALSE;
            if(ship->rceffect)
                StopTractorBeam(ship);
            CleanSalCapSpec(ship);
            return TRUE;
        }
    }

    //begin state machine
    switch(spec->salvageState)
    {
    case SALVAGE_BEGIN:
        //get target from target list using some sort of criteria :)
        if(spec->target != NULL)
        {
            SelectCommand *targetShips = (SelectCommand *)targets;
            if(ShipInSelection(targetShips,(Ship *)spec->target))
            {
                //assume already clamped!
                spec->salvageState = SAL_SALVAGE1;
                break;
            }
        }

        if((spec->target = salCapGetTarget(ship,targets,TRUE,&waittarget))==NULL)
        {
            //no viable target in list
            aitrackStabilizeShip(ship); //if ship is in a command, this will get
                                        //called over and over until all ships are done!
            aitrackZeroVelocity(ship);
            CleanSalCapSpec(ship);
            return TRUE;
        }

        if(waittarget)
        {
            //target is special...we need to wait for it to be free...

            if(!MoveReachedDestinationVariable(ship,&spec->target->posinfo.position,salcapcorvettestatics->flyToDistance*4))
            {
                //not close enough to begin proper docking sequence, so fly closer to target
                if (aishipFlyToPointAvoidingObjs(ship,&spec->target->posinfo.position,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,0.0f) & AISHIP_FLY_OBJECT_IN_WAY)
                    goto reachedit;
            }
            else
            {
reachedit:
                aitrackZeroVelocity(ship);
                aitrackStabilizeShip(ship);
            }

            spec->target=NULL;  //reset to NULL because it isn't really going after it.
            break;
        }

        spec->salvageState = SAL_FLYTOTARGET;

        //don't let a salcap salvage a derelict twice if tech is what be gotten
        if(singlePlayerGame)
        {
            if(spec->target->objtype == OBJ_DerelictType)
            {
                if(((Derelict *)spec->target)->attributes & ATTRIBUTES_StripTechnology)
                {
                    if(universe.DerelictTech)
                    {
                        //Already Been salvaged...don't allow again
                        //no viable target in list
                        ((SpaceObjRotImpTargGuidanceShipDerelict *)spec->target)->salvageNumTagged[ship->playerowner->playerIndex]--;
                        aitrackStabilizeShip(ship); //if ship is in a command, this will get
                                                    //called over and over until all ships are done!
                        aitrackZeroVelocity(ship);
                        CleanSalCapSpec(ship);
                        return TRUE;
                    }
                }
            }
        }

        break;
    case SAL_FLYTOTARGET:
        //fly to within a certain range of the target
        if(spec->target->clampInfo != NULL)
        {
            //someone is salvaging this target already
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                //shouldn't go below zero!
                //so force to zero...these functions can be called twice..unfortunatly
                spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        if(!MoveReachedDestinationVariable(ship,&spec->target->posinfo.position,salcapcorvettestatics->flyToDistance))
        {
            //not close enough to begin proper docking sequence, so fly closer to target
            if ((aishipFlyToPointAvoidingObjs(ship,&spec->target->posinfo.position,AISHIP_DontFlyToObscuredPoints|AISHIP_PointInDirectionFlying,0.0f)
                & AISHIP_FLY_OBJECT_IN_WAY) == 0)
                break;
        }
        if(spec->target->objtype != OBJ_DerelictType)
        {
            if(!salCapAreEnoughSalvagersTargettingThisTarget(ship,spec->target))
            {
                //not enough doods targetting this bizitch...play sound event
                //with ye'old battler chatter of course
                ;
            }
        }
        spec->salvageState = SAL_DOCKINGSTATE1;
        break;
    case SAL_DOCKINGSTATE1:
        //choose salvaging docking method
        //1-with salvage lights on target
        //2-without salvage lights
        if(spec->target->clampInfo != NULL)
        {
            //someone is salvaging this target already
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                //so force to zero...these functions can be called twice..unfortunatly
                spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        if( spec->target->salvageInfo != NULL)
        {
            spec->salvageState = SAL_DOCKWITHLIGHTS1;
        }
        else
        {
            spec->salvageState = SAL_DOCKWITHOUTLIGHTS1;
        }

        //fix formation anoyances!
        ship->specialFlags |= SPECIAL_BrokenFormation;
        break;
    case SAL_DOCKWITHOUTLIGHTS1:
        //default harvest method with no Salvage Lights on target
        if(spec->target->clampInfo != NULL)
        {
            //someone is salvaging this target already
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        {
            vector trajectory,heading;
            sdword or;
            real32 range;

            spec->noAvoid = TRUE;   //set noAvoid variable so collision avoidence will not avoid the salcaps target!
            aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)spec->target,&trajectory);
            range = RangeToTarget(ship,(SpaceObjRotImpTarg *)spec->target,&trajectory);
            aishipFlyToPointAvoidingObjs(ship,&spec->target->posinfo.position,0,0.0f);
            vecSub(heading,spec->target->posinfo.position,ship->posinfo.position);
            vecNormalize(&heading);
            or = aitrackHeadingWithFlags(ship,&heading,0.98f,AITRACKHEADING_IGNOREUPVEC);
            if(range <= salcapcorvettestatics->noLightClampingDistance && or)
            {
                //Salcap is theoretically on the hull of the other ship at this point
                //couldn't use colboxindirection function because turned out to be
                //unreliable
                //
                //now clamp object to salcap and tell it to dock for targetDeposit
                if(ship->shiptype == JunkYardDawg)
                {
                    clampObjToObj((SpaceObjRotImpTargGuidance *)spec->target,(SpaceObjRotImpTargGuidance *)ship);
                }
                else
                {
                    //clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)spec->target);
                    clampObjToObj((SpaceObjRotImpTargGuidance *)spec->target,(SpaceObjRotImpTargGuidance *)ship);
                }
                dbgAssert(ship->rceffect == NULL);
                startTractorBeam(ship,spec->target);
                spec->salvageState = SAL_CLAMPED;
            }
        }
        break;
    case SAL_DOCKWITHLIGHTS1:
        if(spec->target->clampInfo != NULL)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        if(spec->target->salvageInfo->numSalvagePointsFree == 0)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        //Free salvaging Point!
        if((spec->salvageIndex = salCapGetAndBusySalvageIndex(ship,spec->target)) == NO_INDEX)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        spec->salvageState = SAL_WL_FLYTOWITHINCONE;
        break;
    case SAL_WL_FLYTOWITHINCONE:
        if(spec->target->clampInfo != NULL)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        if(salCapFlyToWithInCone(ship,spec->target))
        {
            spec->noAvoid = TRUE;
            spec->salvageState = SAL_WL_FLYTOCONEORIGIN;
        }
        if(spec->target->objtype == OBJ_ShipType)
        {
            bitSet(((Ship *)spec->target)->dontrotateever,1);
            bitSet(((Ship *)spec->target)->dontapplyforceever,1);
        }
        vecScalarMultiply(spec->target->posinfo.velocity,spec->target->posinfo.velocity,0.90f);
        vecScalarMultiply(spec->target->rotinfo.rotspeed,spec->target->rotinfo.rotspeed,0.90f);
        break;
    case SAL_WL_FLYTOCONEORIGIN:
        if(spec->target->clampInfo != NULL)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
            removeTargetFromSelection((SpaceObjRotImpTarg *)spec->target,targets);
            CleanSalCapSpec(ship);
            return FALSE;
        }
        if(salCapFlyToConeOrigin(ship,spec->target))
        {
            //we are now in clamp teritory...
            if(ship->shiptype == JunkYardDawg)
            {
                clampObjToObj((SpaceObjRotImpTargGuidance *)spec->target,(SpaceObjRotImpTargGuidance *)ship);
            }
            else
            {
                //do clamp
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)spec->target);

            }
            //declare that we are latched on
            spec->target->salvageInfo->numNeededForSalvage--;

            spec->salvageState = SAL_CLAMPED;
            if(ship->shiptype == JunkYardDawg)
            {
                //grip ship...
                madLinkInCloseSpecialShip(ship);
            }
        }
        if(spec->target->objtype == OBJ_ShipType)
        {
            bitSet(((Ship *)spec->target)->dontrotateever,1);
            bitSet(((Ship *)spec->target)->dontapplyforceever,1);
        }
        vecScalarMultiply(spec->target->posinfo.velocity,spec->target->posinfo.velocity,0.90f);
        vecScalarMultiply(spec->target->rotinfo.rotspeed,spec->target->rotinfo.rotspeed,0.90f);

        break;
    case SAL_CLAMPED:
        //check if we need to rip tech from the object instead
//        speechEvent(ship, STAT_SCVette_TargetAcquired, 0);

        //close all animations if a ship
        if (battleCanChatterAtThisTime(BCE_SalvageTargetAcquired, ship))
        {
            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_SalvageTargetAcquired, ship, ranRandom(RAN_Sound) % 4);
        }

        if(spec->target->objtype == OBJ_DerelictType)
        {
            if(((Derelict *)spec->target)->attributes & ATTRIBUTES_StripTechnology)
            {

                spec->salvageState = SAL_STRIPTECH;
                spec->salvageAttributes |= SALVAGE_AT_GET_TECH;
                break;
            }
            spec->salvageState = SAL_SALVAGE1;
        }
        else if(spec->target->objtype == OBJ_ShipType)
        {
            if(((Ship *)spec->target)->attributes & ATTRIBUTES_StripTechnology)
            {
                spec->salvageState = SAL_STRIPTECH;
                spec->salvageAttributes |= SALVAGE_AT_GET_TECH;
                break;
            }
            bitClear(((Ship *)spec->target)->dontrotateever,1);
            bitClear(((Ship *)spec->target)->dontapplyforceever,1);
            spec->salvageState = SAL_DO_DAMAGE;
            if(spec->target->objtype != OBJ_DerelictType)
            {
                if(!salCapAreEnoughSalvagersTargettingThisTarget(ship,spec->target))
                {
                    //not enough doods targetting this bizitch...play sound event
                    ;
                }
            }
        }
        break;
    case SAL_DO_DAMAGE:
        if(spec->target->salvageInfo == NULL)
        {
            //don't do damage for these, just give a docking order...
            spec->salvageState = SAL_SALVAGE1;
            break;
        }

        //go through list and DON'T do damage to special cases!
        if(spec->target->objtype == OBJ_ShipType)
        {
            switch(((Ship *)spec->target)->shiptype)
            {
            case CryoTray:
                spec->salvageState = SAL_SALVAGE1;
                break;
            default:
                break;
            }
            if(spec->salvageState == SAL_SALVAGE1)
            {
                //we don'twant to do damage...so special case
                //jump out of case statement!
                break;
            }
        }
        if(spec->target->health*spec->target->staticinfo->oneOverMaxHealth < salcapcorvettestatics->HealthThreshold)
        {
            spec->salvageState = SAL_SALVAGE1;
        }
        else
        {
            if(ship->playerowner != NULL)
            {
                ApplyDamageToTarget((SpaceObjRotImpTarg *)spec->target,salcapcorvettestatics->healthRemovedPerSecond*universe.phystimeelapsed,-1,DEATH_Killed_By_Player,ship->playerowner->playerIndex);
            }
            else
            {
                ApplyDamageToTarget((SpaceObjRotImpTarg *)spec->target,salcapcorvettestatics->healthRemovedPerSecond*universe.phystimeelapsed,-1,DEATH_Killed_By_Dead_Player,99);
            }
            if(spec->target->objtype == OBJ_ShipType)
            {
                ((Ship *)spec->target)->gettingrocked = ship;

                ((Ship *)spec->target)->recentAttacker = ship;
                ((Ship *)spec->target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
            }

            //spec->target->health -= salcapcorvettestatics->healthRemovedPerSecond*universe.phystimeelapsed;
        }
        break;
    case SAL_JUST_DO_DAMAGE:
        //if theres no place to dock with, ships will go to this state and just
        //do damage to a ship
        if(ship->playerowner != NULL)
        {
            ApplyDamageToTarget((SpaceObjRotImpTarg *)spec->target,salcapcorvettestatics->healthRemovedPerSecond*universe.phystimeelapsed,-1,DEATH_Killed_By_Player,ship->playerowner->playerIndex);
        }
        else
        {
            ApplyDamageToTarget((SpaceObjRotImpTarg *)spec->target,salcapcorvettestatics->healthRemovedPerSecond*universe.phystimeelapsed,-1,DEATH_Killed_By_Dead_Player,99);
        }
        if(spec->target->objtype == OBJ_ShipType)
        {
            ((Ship *)spec->target)->gettingrocked = ship;

            ((Ship *)spec->target)->recentAttacker = ship;
            ((Ship *)spec->target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
        }

        break;
    case SAL_SALVAGE1:
        //don't do dockwith check if JunkYardDawg
        if(ship->shiptype != JunkYardDawg)
        {
            if((spec->dockwith = salCapFindDockWith(ship,spec->target)) == NULL)
            {
                spec->salvageState = SAL_JUST_DO_DAMAGE;
                break;
            }
            if(spec->target->salvageInfo == NULL)
            {
                bitSet(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
                selection.numShips = 1;
                selection.ShipPtr[0] = ship;
                clDock(&universe.mainCommandLayer,&selection,DOCK_FOR_BOTHREFUELANDREPAIR|DOCK_AT_SPECIFIC_SHIP,spec->dockwith);
            }
            else
            {
                spec->salvageState = SAL_FLYBACK_TO_DOCKING_SHIP;

                if(ship->shiptype == SalCapCorvette)    //the dawg only needs 1!
                {
                    //check if we still need people to attach
                    if(spec->target->salvageInfo->numNeededForSalvage > 0)
                        break;
                }
            }
        }
        if(spec->target->objtype == OBJ_ShipType)
        {

            if(!bitTest(spec->target->flags,SOF_Disabled))
            {
                if(universe.curPlayerIndex ==
                   ((Ship *)spec->target)->playerowner->playerIndex)
                {
                    ///////////////////////////
                    //Ship has been stolen...
                    //event num: STAT_F_ShipStolen
                    //battle chatter maybe...
                    if (ship->playerowner->playerIndex != universe.curPlayerIndex)
                    {
                        speechEventFleetSpec((Ship *)spec->target, STAT_F_ShipStolen, ((Ship *)spec->target)->shiptype, universe.curPlayerIndex);
                    }
                }
            }
            //disable ship here
            selection.numShips = 1;
            selection.ShipPtr[0] = (Ship *)spec->target;
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selection);
            bitClear(spec->target->flags, SOF_Selectable);
            bitSet(spec->target->flags,SOF_Disabled);
            bitSet(spec->target->flags,SOF_DontDrawTrails);
            shipHasJustBeenDisabled((Ship *)spec->target);

            if(spec->target->objtype == OBJ_ShipType)
            {
                bitClear(spec->target->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
            }
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected,(SpaceObjRotImpTarg *)spec->target);
            if(ship->shiptype == JunkYardDawg)
            {
                AIVar *var;
                var = aivarCreate("DawgHasBone");
                aivarValueSet(var,TRUE);

                //disable this ship FOREVER!
                bitSet(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever);

                //Allow movement for junkyarddawg by settinbg special flag
                bitSet(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
                return TRUE;
            }
            if(singlePlayerGame)
            {
                AIVar *var;
                var = aivarCreate("SalvageTD");
                aivarValueSet(var, TRUE);
            }
        }
        break;
    case SAL_FLYBACK_TO_DOCKING_SHIP:
        if(spec->target->salvageInfo != NULL)
        {
            if(spec->target->salvageInfo->numNeededForSalvage > 0)
                break;
        }
        bitClear(spec->target->flags, SOF_Selectable);
        if(salCapFlyToWithinDockingRange(ship))
        {
            //we are within range
            spec->salvageState = SAL_DOCKINGSTAGE;
        }
        break;
    case SAL_DOCKINGSTAGE:
        bitClear(spec->target->flags, SOF_Selectable);
        spec->dockindex = getDockIndex(ship,spec->target,spec->dockwith);
        spec->salvageState = SAL_WAIT;
        break;
    case SAL_WAIT:
        bitClear(spec->target->flags, SOF_Selectable);
        if(!spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy)
        {
            bitSet(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint);
            spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy++;   //busy port
            bitSet(spec->target->specialFlags,SPECIAL_BUSY_BECAUSE_SALVAGE_DOCKING);
            spec->salvageState = SAL_WAYPOINTHACK;
        }
        //docking point busy, so lets wait:
        if(spec->target->specialFlags & SPECIAL_BUSY_BECAUSE_SALVAGE_DOCKING)
        {
            spec->salvageState = SAL_WAYPOINTHACK;
        }
        aitrackZeroVelocity((Ship *)spec->target);
        aitrackSteadyShip((Ship *)spec->target);
        break;
    case SAL_WAYPOINTHACK:
        {
            vector between,dockwithRight;

            if(spec->dockwith->shiprace == R1)
            {
                if(spec->dockwith->shiptype == Mothership)
                {
                    vecSub(between,spec->target->posinfo.position,spec->dockwith->posinfo.position);
                    matGetVectFromMatrixCol2(dockwithRight,spec->dockwith->rotinfo.coordsys);

                    if(vecDotProduct(between,dockwithRight) > 0)
                    {
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock 0.5");
#endif
                        spec->salvageState = WAYPOINTHACK2;
                    }
                    else
                    {
#ifdef DEBUG_SALCAP
                        dbgMessagef("\nSAL:dock 1");
#endif

                        spec->salvageState = SAL_DO_DOCK;
                    }
                }
                else
                {
#ifdef DEBUG_SALCAP
                    dbgMessagef("\nSAL:dock 1");
#endif

                    spec->salvageState = SAL_DO_DOCK;
                }
            }
            else
            {
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock 1");
#endif
                spec->salvageState = SAL_DO_DOCK;
            }
        }
        break;
    case WAYPOINTHACK2:
        if(salCapFlyToDockingPoint1over2(ship,spec->target))
        {
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock 1");
            spec->salvageState = SAL_DO_DOCK;
        }
        break;

#endif
    case SAL_DO_DOCK:
        bitClear(spec->target->flags, SOF_Selectable);
        if(salCapFlyToDockingPoint1(ship,spec->target))
        {
            //check to see if we need to opendoors
            if(spec->dockwith->shiprace == R1)
            {
                if(spec->dockwith->shiptype == Mothership)
                {
                    if(salCapNeedBig(ship,spec->target))
                    {
                        //request, nay, demand mothership door ship opens!
                        madLinkInOpenDoor(spec->dockwith);
                    }
                }
            }
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock 2");
#endif

            spec->salvageState = SAL_DO_DOCK2;
            spec->target->dockingship = spec->dockwith;
            unClampObj((SpaceObjRotImpTargGuidance *)ship);
            spec->noDamageTargetTime = universe.totaltimeelapsed;
            spec->noDamageTarget = spec->target;
            if(spec->target->salvageInfo != NULL)
            {
                spec->target->salvageInfo->numSalvagePointsFree++;

                //its ok to decrease this here..we were just clmaped
                spec->target->salvageInfo->numNeededForSalvage++;
                if(spec->target->salvageInfo->numNeededForSalvage > spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage)
                {
                    spec->target->salvageInfo->numNeededForSalvage = spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage;
                }

            }
        }
        break;
    case SAL_DO_DOCK2:
        {
            //needto restore selectabled and disabled flags to
            //proper values after call to cleanup function
            SpaceObjRotImpTargGuidanceShipDerelict *targ;
            Ship *dockwith;
            sdword groupme = TRUE;

            //this could potentially screw up...
            //if we have say a valid target left, but multiple people to go after it, they will all
            //register a valid target, and not get grouped..then they will
            //have there order terminated shortly after this bit of code and end up dead
            //in space not grouped.  Very insignificant...
            if(targets->numTargets > 1 && isThereAnotherTargetForMe(ship,targets))
            {
                //we aren't done
                groupme = FALSE;
            }
            dockwith = spec->dockwith;
            targ = spec->target;
            if(!handOffTargetToDockWith(ship))
            {
                //docking ship is BUSY...so we must wait here...
                return FALSE;
            }

            if (singlePlayerGame)
            {
                AIVar *var;

                if ((var = aivarFind("SalvageHandOff")) == NULL)
                {
                    var = aivarCreate("SalvageHandOff");
                }
                aivarValueSet(var, TRUE);
            }
            //clear just in case...only be handing off if we were shure...
            bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
            SalCapOrderChangedCleanUp(ship);
            bitClear(targ->flags,SOF_Selectable);
            bitSet(targ->flags,SOF_Disabled);
            bitSet(targ->flags,SOF_DontDrawTrails);

            if(groupme)
            {
                //set flag to give grouporder in housekeep function
                spec->groupme = TRUE;
                spec->dockwith = dockwith;
            }
        }
        //return FALSE and let end code settle things
        return FALSE;
    case SAL_STRIPTECH:
        spec->timeCounter += universe.phystimeelapsed;
        if(spec->timeCounter > salcapcorvettestatics->getTechTime)
        {
            //tech aquired
            //autodocking...
            if(singlePlayerGame)
            {
                AIVar *var;

                var = aivarFind("TechRemoved");

                if (var == NULL)
                {
                    var = aivarCreate("TechRemoved");
                }
                aivarValueSet(var, 100);
                bitSet(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology);
                universe.DerelictTech = TRUE;    //indicate that derelict is not to be salvaged again...
            }

            // strip any pings associated with it
            if (spec->target) pingRemovePingByOwner((SpaceObj *)spec->target);

            // enable appropriate tech
            //enable(aivarValueGet(aivarFind("TechToEnable")));

            //unclamp ship
            unClampObj((SpaceObjRotImpTargGuidance *)ship);
            spec->noDamageTargetTime = universe.totaltimeelapsed;
            spec->noDamageTarget = spec->target;

            spec->tractorBeam = FALSE;
            if(ship->rceffect)
            {
                StopTractorBeam(ship);
            }
            if(spec->target->salvageInfo != NULL)
            {
                spec->target->salvageInfo->numSalvagePointsFree++;

                //its ok to do this here, we we just clamped
                spec->target->salvageInfo->numNeededForSalvage++;
                if(spec->target->salvageInfo->numNeededForSalvage > spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage)
                {
                    spec->target->salvageInfo->numNeededForSalvage = spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage;
                }
            }
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            spec->salvageState = SAL_TECH_UNDOCK;
        }
        else
        {
            //set the aivar to the precentage of stripping that has been done
            if (singlePlayerGame)
            {
                AIVar *var;

                var = aivarFind("TechRemoved");

                if (var == NULL)
                {
                    var = aivarCreate("TechRemoved");
                }
                aivarValueSet(var, (sdword)((100 * spec->timeCounter) / salcapcorvettestatics->getTechTime));
            }
        }
        //kas function returns striptech timeCounter

        break;
    case SAL_UNDOCK_AND_QUIT:
        if(spec->target->salvageInfo != NULL)
        {
            if(salCapFlyOutOfCone(ship,spec->target,TRUE))
            {
                return TRUE;
            }
        }
        else
        {
            return TRUE;
        }
        break;
    case SAL_TECH_UNDOCK:
        if(spec->target->salvageInfo != NULL)
        {
            if(salCapFlyOutOfCone(ship,spec->target,TRUE))
            {
                spec->salvageState = SAL_TECH_TAKE_TECH_HOME;
            }
        }
        else
        {
            spec->salvageState = SAL_TECH_TAKE_TECH_HOME;
        }
        break;
    case SAL_TECH_TAKE_TECH_HOME:
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        //later..change to dock for tech deposit later...
        bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);

        SalCapOrderChangedCleanUp(ship);

        if (ship->playerowner->PlayerMothership)
        {
            clDock(&universe.mainCommandLayer,&selection,DOCK_AT_SPECIFIC_SHIP,ship->playerowner->PlayerMothership);
        }
        else
        {
            clDock(&universe.mainCommandLayer,&selection,DOCK_FOR_BOTHREFUELANDREPAIR,NULL);
        }
        break;
    default:
        dbgFatalf(DBG_Loc,"\nUnknown SalCap AI State %d",spec->salvageState);

    }
    return FALSE;
}


#define DROP_BEGIN 0
#define DROP_DOCK2 1
#define DROP_DONEDOCKING 2
#define DROP_DOORDEAL 3
#define DROP_DO_HARVEST 4

bool handOffTargetToDockWith(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    sdword i;
    ///////////////////
    //salcap drop off target speech event
    //event num: STAT_SCVette_DroppingOff
    //battle chatter needed
    if(ship->playerowner->playerIndex == universe.curPlayerIndex)
    {
        if (battleCanChatterAtThisTime(BCE_STAT_SCVette_DroppingOff, ship))
        {
            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_SCVette_DroppingOff, ship, ship->shiptype);
        }
    }

    ///////

    if(spec->dockwith->shiptype == Mothership)
    {
        MothershipSpec *mspec = (MothershipSpec *)spec->dockwith->ShipSpecifics;
        for(i=0;i<MAX_NUM_DROP;i++)
        {
            if(mspec->droptarget[i] == spec->target)
            {
                //already handed off..don't do.
                goto handed;
            }
        }

        for(i=0;i<MAX_NUM_DROP;i++)
        {
            if(mspec->droptarget[i] == NULL)
            {
                mspec->dropstate[i] = DROP_BEGIN;
                mspec->droptarget[i] = spec->target;
                mspec->dockindex[i]  = spec->dockindex;
                goto handed;
            }
        }

    }
    else
    {
        //assumed to be carrier
        CarrierSpec *cspec = (CarrierSpec *)spec->dockwith->ShipSpecifics;
        for(i=0;i<MAX_NUM_DROP;i++)
        {
            if(cspec->droptarget[i] == spec->target)
            {
                //already handed off..don't do.
                goto handed;
            }
        }
        for(i=0;i<MAX_NUM_DROP;i++)
        {
            if(cspec->droptarget[i] == NULL)
            {
                cspec->dropstate[i] = DROP_BEGIN;
                cspec->droptarget[i] = spec->target;
                cspec->dockindex[i] = spec->dockindex;
                goto handed;
            }
        }
    }
    //if we get here...we didn't find an attachment point...so we should wait!!!
    return FALSE;

handed:
    bitSet(spec->target->specialFlags,SPECIAL_SalvagedTargetGoingIntoDockWithShip);
    return TRUE;
}



bool DropTargetInShip(Ship *dockwith,sdword *targetDepotState, SpaceObjRotImpTargGuidanceShipDerelict *target,sdword *dockindex)
{
    switch(*targetDepotState)
    {
    case DROP_BEGIN:

        bitClear(target->flags, SOF_Selectable);
        bitSet(target->flags,SOF_Disabled);
        bitSet(target->flags,SOF_DontDrawTrails);
        if(target->objtype == OBJ_ShipType)
        {
            SelectCommand selection;
            selection.numShips = 1;
            selection.ShipPtr[0] = (Ship *)target;
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selection);
            shipHasJustBeenDisabled((Ship *)target);
        }
        if(salCapFlyToDockingPoint2(dockwith,target,*dockindex))
        {
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock 2");
#endif

            *targetDepotState = DROP_DOCK2;
        }
        break;
    case DROP_DOCK2:
        bitClear(target->flags, SOF_Selectable);
        if(salCapFlyToDockingPoint3(dockwith,target,*dockindex))
        {
#ifdef DEBUG_SALCAP
                dbgMessagef("\nSAL:dock DONE");
#endif

            *targetDepotState = DROP_DONEDOCKING;
        }
        break;
    case DROP_DONEDOCKING:
        bitClear(target->flags, SOF_Selectable);
        if(dockwith->shiptype == Mothership)
        {
            if(dockwith->shiprace == R1)
            {
                //if big dock
                if(salCapNeedBig(dockwith,target))
                {
                    MothershipAttachObjectToDoor(dockwith,target);
                    madLinkInCloseDoor(dockwith);
                    *targetDepotState = DROP_DOORDEAL;
                    break;
                }
            }
        }
        *targetDepotState = DROP_DO_HARVEST;
        break;
    case DROP_DOORDEAL:
        if(dockwith->madDoorStatus != MAD_STATUS_DOOR_CLOSED)
        {
            //not done yet
            break;
        }
        //done door closeing
        //fall through to harvesting...
    case DROP_DO_HARVEST:
        //GroupShip(&universe.mainCommandLayer,ship,spec->dockwith);  //have ship groupup outside dockwith ship
        if(bitTest(target->specialFlags2,SPECIAL_2_BusiedDockPoint))
        {
            if(dockwith->dockInfo->dockpoints[*dockindex].thisDockBusy)
            {
                bitClear(target->specialFlags2,SPECIAL_2_BusiedDockPoint);
                dockwith->dockInfo->dockpoints[*dockindex].thisDockBusy--;
            }
        }
        bitClear(target->specialFlags,SPECIAL_BUSY_BECAUSE_SALVAGE_DOCKING);
        salCapHarvestTarget(target,dockwith);
        //unbusy dockpoint
        return TRUE;

      }
    return FALSE;
}

/*
    case SALVAGE_GET_TECH:
        spec->time_counter += universe.phystimeelapsed;
        if(spec->time_counter salcapcorvettestatics->getTechTime)
        {
            //tech aquired
            //autodocking...
            AIVar *var;
            var = aivarCreate("TechRemoved");
            aivarValueSet(var, TRUE);

            // enable appropriate tech
            //enable(aivarValueGet(aivarFind("TechToEnable")));

            selection.numShips = 1;
            selection.ShipPtr[0] = ship;
            clDock(&universe.mainCommandLayer,&selection,DOCK_FOR_BOTHREFUELANDREPAIR,NULL);
        }
        break;
    */

//housekeep function...we don't really need this I think...it is out of the header file
void SalCapCorvetteHouseKeep(Ship *ship)
{
//was going to slowly rotate small ships..
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    if((universe.totaltimeelapsed - spec->noDamageTargetTime) > 5.0f)
    {
        //turn off no damage vars
        spec->noDamageTargetTime = 0.0f;
        spec->noDamageTarget = NULL;
    }

    //if we're latched on, and effect ISN'T player;..start playing it.
    if(!ship->rceffect)
    {
        if(spec->target != NULL)
        {
            if(spec->target->clampInfo != NULL && spec->target->clampInfo->host == (SpaceObjRotImpTarg *)ship)
            {
                startTractorBeam(ship, (SpaceObjRotImpTargGuidanceShipDerelict *)spec->target);
            }
            else if(ship->clampInfo != NULL && ship->clampInfo->host == (SpaceObjRotImpTarg *)spec->target)
            {
                startTractorBeam(ship, (SpaceObjRotImpTargGuidanceShipDerelict *)spec->target);
            }
        }
    }
    if(spec->groupme)
    {
        Ship *dockwith;
        SelectCommand selectone;
        selectone.numShips = 1;
        selectone.ShipPtr[0] = ship;
        dockwith = spec->dockwith;
        spec->dockwith = NULL;
        spec->groupme = FALSE;
        //clHalt(&universe.mainCommandLayer,&selectone);
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selectone);
        if(dockwith != NULL)
        {
            GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,dockwith);
        }
    }

/*    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    vector ShipRight,ShipHeading;
    vector newHeading,newUp,CurrentUp,CurrentHeading,rotateAxis;
    matrix rotmat;

    if(spec->target != NULL)
    {
        if(spec->target->clampInfo != NULL &&
            spec->target->clamInfo->host == (SpaceObjRotImpTarg *)ship)
        {
            //we have a small ship attached to the salcap corvette
            //lets rotate it SLOWLY to face a certain direction!
            if((universe.univUpdateCounter & 7) == 0)
            {
                matGetVectFromMatrixCol1(ShipRight,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol3(ShipHeading,ship->rotinfo.coordsys);
                newHeading = ShipRight;
                newUp = ShipHeding;
                matGetVectFromMatrixCol2(CurrentUp,spec->target->clamInfo->clampCoordsys);
                matGetVectFromMatrixCol3(CurrentHeading,spec->target->clamInfo->clampCoordsys);

                if(acos(vecDotProduct(newHeding,CurrentHeading)) < 10.0f



            }
        }
    }
*/
}

//Don't forget to clear tractorbeaminfo in ship and derelict structures.!
void SalCapClose(Ship *ship)
{
}

void salCapClearTechBool()
{
    universe.DerelictTech = FALSE;
}
void SalCapDied(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    spec->tractorBeam = FALSE;

    if(bitTest(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology))
    {
        universe.DerelictTech = FALSE;    //indicate that derelict is not to be salvaged again...
    }


    if(ship->rceffect != NULL)
        StopTractorBeam(ship);
    if(spec->target != NULL)
    {
        if(spec->salvageState > SALVAGE_BEGIN)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                //shouldn't go below zero!
                //so force to zero...these functions can be called twice..unfortunatly
                spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
        }
        unClampObj((SpaceObjRotImpTargGuidance *)spec->target);
        //if(ship->shiptype != JunkYardDawg)
        //{
            if(spec->target->objtype == OBJ_ShipType)
            {
                bitClear(((Ship *)spec->target)->dontrotateever,1);
                bitClear(((Ship *)spec->target)->dontapplyforceever,1);

                if(!bitTest(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever))
                {
                    //if it isn't disabled forever
                    bitClear(spec->target->flags,SOF_Disabled);
                    if (((Ship *)spec->target)->shiptype != CryoTray)
                    {
                        if(((Ship *)spec->target)->playerowner->playerState == PLAYER_ALIVE)
                        {
                            bitSet(spec->target->flags,SOF_Selectable);
                        }
                    }
                }

                if(bitTest(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint))
                {
                    bitClear(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint);
                    if(spec->dockwith != NULL)
                    {
                        if(spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy)
                            spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy--;
                    }
                }

            }
        //}
    }
}

void salCapCleanUpCloakingTarget(Ship *ship, Ship *shiptoremove)
{
    SalCapRemoveShipReferences(ship,shiptoremove);
}

void SalCapRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    if(shiptoremove == (Ship *)spec->target)
    {
        //clear variable just for the heck of it...
        bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
        if(spec->salvageIndex != NO_INDEX)
        {
            salCapUnBusySalvagePoint(ship,spec->target);
        }

        if(ship->shiptype == JunkYardDawg)
        {
            //release grip on ship...
            madLinkInOpenSpecialShip(ship);
        }
        if(spec->salvageState > SALVAGE_BEGIN)
        {
            spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
            if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
            {
                //shouldn't go below zero!
                //so force to zero...these functions can be called twice..unfortunatly
                spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
            }
        }
        if(bitTest(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint))
        {
            bitClear(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint);
            if(spec->dockwith != NULL)
            {
                if(spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy)
                    spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy--;
            }
        }

        spec->tractorBeam = FALSE;
        if(ship->rceffect)
            StopTractorBeam(ship);
        if(ship->clampInfo != NULL)
        {
            #ifdef DEBUG_SALCAP
            dbgAssert(ship->clampInfo->host == (SpaceObjRotImpTarg *)shiptoremove);
            #endif
            unClampObj((SpaceObjRotImpTargGuidance *)ship);
        }
        else if(spec->target->clampInfo != NULL)
        {
            if(spec->target->clampInfo->host == ((SpaceObjRotImpTarg *)ship))
            {
                unClampObj((SpaceObjRotImpTargGuidance *)spec->target);
            }
        }
        if(spec->target->salvageInfo == NULL)
        {
            CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
            if(command != NULL)
            if(command->ordertype.order == COMMAND_DOCK)
            {
                //ship is docking AND
                //shiptoremove is a 'no light' ship...so lets halt this salcap
                SelectCommand selection;
                selection.numShips = 1;
                selection.ShipPtr[0] = ship;
                clHalt(&universe.mainCommandLayer,&selection);
            }
        }
        CleanSalCapSpec(ship);
    }

    if(shiptoremove == spec->dockwith)
    {
        //We are screwed here depending on what state we're in...ideally, lets just
        //reset a few variables and then set the state back to just
        //after clamping!
        if(spec->salvageState == SAL_DO_DOCK2 ||
           spec->salvageState == SAL_backoff)
        {
            //don't do any real cleanup...since already cleanish
            SalCapOrderChangedCleanUp(ship);
            return;
        }
        if(spec->target->salvageInfo != NULL)
        {
            //this is a no light ship
            //so we're no longer executing the special command!
            SelectCommand selection;
            selection.numShips = 1;
            selection.ShipPtr[0] = ship;
            if((spec->dockwith = salCapFindDockWith(ship,spec->target)) == NULL)
            {
                bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
                if(spec->salvageState > SALVAGE_BEGIN)
                {
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
                    if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
                    {
                        //shouldn't go below zero!
                        //so force to zero...these functions can be called twice..unfortunatly
                        spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
                    }
                }
                clHalt(&universe.mainCommandLayer,&selection);
                return;
            }
            else
            {
                clDock(&universe.mainCommandLayer,&selection,DOCK_FOR_BOTHREFUELANDREPAIR,spec->dockwith);
            }
        }
        spec->dockwith = NULL;
        spec->salvageState = SAL_SALVAGE1;
    }
    if(shiptoremove == (Ship *)spec->noDamageTarget)
    {
        spec->noDamageTargetTime = 0.0f;
        spec->noDamageTarget = NULL;
    }
}
void salCapRemoveDerelictReferences(Ship *ship,Derelict *d)
{
     SalCapRemoveShipReferences(ship, (Ship *)d);
}

// Called only when user hits HALT.  (clHalt).
void SalCapCancelOrder(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    if(spec->target != NULL)
    {
        if(bitTest(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint))
        {
            bitClear(spec->target->specialFlags2,SPECIAL_2_BusiedDockPoint);
            if(spec->dockwith != NULL)
            {
                if(spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy)
                    spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy--;
            }
        }
    }
}

//Called when a new order is given to the salvage capture corvette
void SalCapOrderChangedCleanUp(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    sdword unclamped = FALSE;


    bitClear(ship->specialFlags,SPECIAL_Kamikaze);
    ship->attackvars.attacktarget=NULL;
    ship->kamikazeState=0;

    if(!(ship->specialFlags & SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse))
    {
        if(spec->target != NULL)
        {
            if(spec->salvageState > SALVAGE_BEGIN)
            {
                spec->target->salvageNumTagged[ship->playerowner->playerIndex]--;
                if(spec->target->salvageNumTagged[ship->playerowner->playerIndex]<0)
                {
                    //shouldn't go below zero!
                    //so force to zero...these functions can be called twice..unfortunatly
                    spec->target->salvageNumTagged[ship->playerowner->playerIndex] = 0;
                }
            }
        }
        if(spec->salvageIndex != NO_INDEX)
        {
            //twas busy with index
    #ifdef DEBUG_SALCAP
            dbgAssert(spec->target != NULL);
    #endif
            salCapUnBusySalvagePoint(ship,spec->target);
            if(spec->target->salvageInfo->numNeededForSalvage > 0)
            {
                //ship isn't capable of being salvaged now...so undisable it
                if ((spec->target->objtype == OBJ_ShipType) &&
                    (((Ship *)spec->target)->shiptype != CryoTray) &&
                    !bitTest(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever))
                {
                    if(((Ship *)spec->target)->playerowner->playerState == PLAYER_ALIVE)
                    {
                        bitSet(spec->target->flags,SOF_Selectable);
                    }
                }
                bitClear(((Ship *)spec->target)->dontrotateever,1);
                bitClear(((Ship *)spec->target)->dontapplyforceever,1);

                if(!bitTest(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever))
                {
                    if(spec->target->salvageInfo->numSalvagePoints -spec->target->salvageInfo->numSalvagePointsFree > 0)
                    {
                        //ship ISN'T going into docking bay..
                        //so clear disabled flag
                        bitClear(spec->target->flags,SOF_Disabled);
                    }
                }
            }
        }

        if(ship->shiptype == JunkYardDawg)
        {
            if ((spec->target) && (spec->target->clampInfo != NULL))
            {
                unClampObj((SpaceObjRotImpTargGuidance *)spec->target);
                unclamped = TRUE;
            }
        }
        else if(ship->clampInfo != NULL && ship->clampInfo->host == (SpaceObjRotImpTarg *)spec->target)
        {
            unClampObj((SpaceObjRotImpTargGuidance *)ship);
            unclamped = TRUE;
        }
        else if(spec->target != NULL &&
            spec->target->clampInfo != NULL &&
            spec->target->clampInfo->host == (SpaceObjRotImpTarg *)ship)
        {
            //other method where target is attached to ship
            unClampObj((SpaceObjRotImpTargGuidance *)spec->target);
            unclamped = TRUE;
            if(ship->shiptype != JunkYardDawg)
            {
                if(!(spec->target->specialFlags & SPECIAL_SalvagedTargetGoingIntoDockWithShip))
                {
                    //ship ISN'T going into docking bay..
                    //so clear disabled flag
                    if(!bitTest(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever))
                    {
                        bitClear(spec->target->flags,SOF_Disabled);
                    }
                }
                if ((spec->target->objtype == OBJ_ShipType) &&
                    (((Ship *)spec->target)->shiptype != CryoTray))
                {
                    if(((Ship *)spec->target)->playerowner->playerState == PLAYER_ALIVE)
                    {
                        if(!bitTest(((Ship *)spec->target)->specialFlags2,SPECIAL_2_DisabledForever))
                        {
                            bitSet(spec->target->flags,SOF_Selectable);
                        }
                    }
                }
            }
        }
        spec->tractorBeam = FALSE;
        if(ship->rceffect)
            StopTractorBeam(ship);

        spec->noDamageTargetTime = universe.totaltimeelapsed;
        spec->noDamageTarget = spec->target;
        CleanSalCapSpec(ship);

    }
    bitClear(ship->specialFlags,SPECIAL_BrokenFormation);
}


void SalCapDropTarget(Ship *ship)
{
    SelectCommand selection;
    bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
    selection.numShips = 1;
    selection.ShipPtr[0] = ship;
    SalCapOrderChangedCleanUp(ship);

    if(ship->shiptype == JunkYardDawg)
    {
        //release grip on ship...
        madLinkInOpenSpecialShip(ship);
    }

    clHalt(&universe.mainCommandLayer,&selection);
}

// Save Game stuff

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void SalCapCorvette_RegisterExtraSpaceObjs(Ship *ship)
{
}

void SalCapCorvette_PreFix(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetID((SpaceObj *)spec->target);
    spec->dockwith = (Ship *) SpaceObjRegistryGetID((SpaceObj *)spec->dockwith);
    spec->noDamageTarget = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetID((SpaceObj *)spec->noDamageTarget);
}

void SalCapCorvette_Fix(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetTarget((sdword)spec->target);
    spec->dockwith = SpaceObjRegistryGetShip((sdword)spec->dockwith);
    spec->noDamageTarget = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetTarget((sdword)spec->noDamageTarget);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

CustShipHeader SalCapCorvetteHeader =
{
    SalCapCorvette,
    sizeof(SalCapCorvetteSpec),
    SalCapCorvetteStaticInit,
    NULL,
    SalCapCorvetteInit,
    SalCapClose,
    NULL, // SalCapCorvetteAttack,
    NULL, // DefaultShipFire,
    NULL, // SalCapCorvetteAttackPassive,
    NULL,
    SalCapCorvetteSpecialTarget,
    SalCapCorvetteHouseKeep,
    SalCapRemoveShipReferences,
    SalCapDied,
    SalCapCorvette_PreFix,
    NULL,
    NULL,
    SalCapCorvette_Fix
};

CustShipHeader JunkYardDawgHeader =
{
    JunkYardDawg,
    sizeof(SalCapCorvetteSpec),
    SalCapCorvetteStaticInit,
    NULL,
    SalCapCorvetteInit,
    SalCapClose,
    NULL, // SalCapCorvetteAttack,
    NULL, // DefaultShipFire,
    NULL, // SalCapCorvetteAttackPassive,
    NULL,
    SalCapCorvetteSpecialTarget,
    SalCapCorvetteHouseKeep,
    SalCapRemoveShipReferences,
    SalCapDied,
    SalCapCorvette_PreFix,
    NULL,
    NULL,
    SalCapCorvette_Fix
};

/*-----------------------------------------------------------------------------
    Name        : dockDrawSalvageInfo
    Description : draws salvage docking lights
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VISUALIZATION
void dockDrawSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *obj)
{
    sdword i;
    vector to;
    StaticInfoHealthGuidanceShipDerelict *statinfo = obj->staticinfo;

    if(statinfo->salvageStaticInfo != NULL)
    {
        SalvageStaticInfo *salvageStaticInfo = statinfo->salvageStaticInfo;
        SalvageStaticPoint *salvagePoint;
        sdword salvageNumPoints = salvageStaticInfo->numSalvagePoints;

        rndLightingEnable(FALSE);
        rndTextureEnable(FALSE);



        for (i=0,salvagePoint=&salvageStaticInfo->salvageStaticPoints[0];i<salvageNumPoints;i++,salvagePoint++)
        {
            vecScalarMultiply(to,salvagePoint->conenormal,2000.0f);
            vecAddTo(to,salvagePoint->position);
            if(obj->salvageInfo->salvagePoints[i].busy)
            {
                //in use, so use RED instead of fuscia
                primLine3(&salvagePoint->position,&to,colReddish);
            }
            else
            {
                primLine3(&salvagePoint->position,&to,colFuscia);
            }
        }

        /*
        salCapSetNewCoordSys(obj,&fakeHeading,&velocityN);

        matGetVectFromMatrixCol1(realHeading,obj->rotinfo.coordsys);
        cosAngle = vecDotProduct(fakeHeading,velocityN);
        Angle = acos(cosAngle);

                matMultiplyVecByMat(&N,&fakeHeading,&obj->rotinfo.coordsys);
                vecScalarMultiply(to,N,6000.0f);
                primLine3(&o,&to,colRGB(255, 255, 255));

        vecCrossProduct(planePerp,velocityN,fakeHeading);
        vecNormalize(&planePerp);
        nisRotateAboutVector((real32 *) &rotMatrix, &planePerp, Angle);
        matMultiplyVecByMat(&neededHeading,&realHeading,&rotMatrix);

            matMultiplyVecByMat(&N,&planePerp,&obj->rotinfo.coordsys);
            vecScalarMultiply(to,N,6000.0f);
            primLine3(&o,&to,colRGB(255, 255, 0));


            matMultiplyVecByMat(&N,&neededHeading,&obj->rotinfo.coordsys);
            vecScalarMultiply(to,N,4000.0f);
            primLine3(&o,&to,colRGB(0, 255, 0));

            matMultiplyVecByMat(&N,&velocityN,&obj->rotinfo.coordsys);
            vecScalarMultiply(to,N,4000.0f);
            primLine3(&o,&to,colRGB(0, 0, 255));
        */

        rndLightingEnable(TRUE);
    }
}
#endif

void salCapExtraSpecialOrderCleanUp(SelectCommand *selection,udword ordertype,Ship *dockwith,SelectCommand *targets)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        //SHOULD WE DO A CHECK HERE FOR JUNKYARD DAWG!!??!?!?!
        if(selection->ShipPtr[i]->shiptype == SalCapCorvette)
        {
            if( ((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target != NULL)
            {
                if(((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target->clampInfo != NULL &&
                   ((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target->clampInfo->host == (SpaceObjRotImpTarg *)selection->ShipPtr[i])
                {
                    //salcap corvette is doing something new and has a target
                    //clamped on!
                    switch (ordertype)
                    {
                    case COMMAND_MOVE:
                    case COMMAND_NULL:
                        break;  //no cleanup
                    case COMMAND_SPECIAL:
                        if(ShipInSelection(targets,(Ship *)((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target))
                        {
                            //given command special with target already attached
                            break;
                        }
                        goto ditchTarget;
                    case COMMAND_DOCK:
                        if(dockwith != NULL)
                        {
                            if(dockwith->shiptype == Mothership ||
                               dockwith->shiptype == Carrier)
                            {
                                //keep it clamped and simply dock with the new ship!
                                break;
                            }
                        }
                        goto ditchTarget;
                    case COMMAND_ATTACK:
                    case COMMAND_LAUNCHSHIP:
                    case COMMAND_COLLECTRESOURCE:
                    case COMMAND_BUILDINGSHIP:
                    case COMMAND_HALT:
                    case COMMAND_MILITARYPARADE:
ditchTarget:
                        bitClear(selection->ShipPtr[i]->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
                        SalCapOrderChangedCleanUp(selection->ShipPtr[i]);
                        break;
                    }
                }
            }
        }
    }
}


//counts the number of salvage corvettes in the universe targetting this same ship
//must already be targetting it
//returns TRUE if enough are heading towards it to get it
bool salCapAreEnoughSalvagersTargettingThisTarget(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    sdword count;
    Node *node = universe.mainCommandLayer.todolist.head;
    CommandToDo *todo;
    sdword i;

    count = 0;
    while (node != NULL)
    {
        todo = (CommandToDo *) listGetStructOfNode(node);

        if(todo->ordertype.order == COMMAND_SPECIAL)
        {
            for(i=0;i<todo->selection->numShips;i++)
            {
                if(todo->selection->ShipPtr[i]->shiptype == SalCapCorvette)
                {
                    //this is a salvage corvette
                    if( ((SalCapCorvetteSpec *)todo->selection->ShipPtr[i]->ShipSpecifics)->target ==target)
                    {
                        //this salcorvette is targetting this target
                        count++;
                    }
                }
            }
        }
        node = node->next;
    }

    if(target->salvageInfo != NULL)
    {
        if(count < target->staticinfo->salvageStaticInfo->numNeededForSalvage)
        {
            return FALSE;
        }
        return TRUE;
    }
    if(count >= 1)
    {
        return TRUE;
    }
    return FALSE;
}

