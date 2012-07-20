/*=============================================================================
    Name    : Attack.c
    Purpose : General routines for ships attacking

    Created 10/7/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include "Types.h"
#include "FastMath.h"
#include "SpaceObj.h"
#include "Physics.h"
#include "Gun.h"
#include "AIShip.h"
#include "AITrack.h"
#include "Collision.h"
#include "Attack.h"
#include "Universe.h"
#include "Tweak.h"
#include "CommandLayer.h"
#include "Randy.h"

//#define DEBUG_ATTACK

static AttackSideStepParameters sampleSideStepParameters;

scriptStructEntry AttackSideStepParametersScriptTable[] =
{
    { "repositionTime", scriptSetReal32CB, (udword) &(sampleSideStepParameters.repositionTime), (udword) &(sampleSideStepParameters) },
    { "circleRange",    scriptSetReal32CB, (udword) &(sampleSideStepParameters.circleRange), (udword) &(sampleSideStepParameters) },
    { "fullMovementFreedom", scriptSetBool,(udword) &(sampleSideStepParameters.fullMovementFreedom), (udword) &(sampleSideStepParameters) },
    { NULL,NULL,0,0 }
};

/*-----------------------------------------------------------------------------
    Name        : needToGoToSameVerticalPlane
    Description : returns TRUE if ship, is within TOLERANCE of its target's
                  vertical height so the captial ship doesn't start rotating
                  up and down.
    Inputs      : ship, target, tolerance
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool needToGoToSameVerticalPlane(Ship *ship,SpaceObjRotImpTarg *target, real32 tolerance, real32 speedToleranceSqr)
{
    real32 ydist,speedSqr;

    if ((ship->shiptype == Carrier) || (ship->shiptype == MissileDestroyer) || (ship->shiptype == P1Mothership))
    {
        return TRUE;
    }

    speedSqr = vecMagnitudeSquared(target->posinfo.velocity);

    if(speedSqr > speedToleranceSqr)
    {
        ydist = ship->posinfo.position.z - target->posinfo.position.z;

        if(ydist < 0.0f)
        {
            ydist = -ydist;
        }

        if(ydist <= tolerance)
            return FALSE;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : attackPassiveRotate
    Description : does a passive attack with rotation, which means it may rotate in place and aim it guns in enemy's
                  direction, without moving the ship.
    Inputs      : ship, target
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackPassiveRotate(Ship *ship,Ship *target)
{
    vector trajectory;
    real32 range;
    real32 dist;
    real32 temp;
//    CommandToDo *command;
//    sdword i;

    if (ship->specialFlags & SPECIAL_Hyperspacing)
        return;

    aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);

    if ((ship->shiptype == Carrier) || (ship->shiptype == MissileDestroyer))
    {
        trajectory.z = 0.0f;        // don't pirouette EVER for Carrier, Missile Destroyer
    }
    else if (isCapitalShip(ship))
    {
        if (vecMagnitudeSquared(target->posinfo.velocity) > capShipLieFlatInGeneralSpeedSqrTolerance[ship->staticinfo->shipclass])
        {
            trajectory.z = 0.0f;
        }
    }

    dist = fsqrt(vecMagnitudeSquared(trajectory));
    vecDivideByScalar(trajectory,dist,temp);

    aitrackHeading(ship,&trajectory,FLYSHIP_ATTACKACCURACY);

    if (ship->attackvars.multipleAttackTargets)
    {
        gunShootGunsAtMultipleTargets(ship);
    }
    else
    {
        range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);

        gunShootGunsAtTarget(ship,(SpaceObjRotImpTarg *)target,range,&trajectory);
    }

    /*
    //check to see if we should back up!
    if(ship->tacticstype == Evasive)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
        if(command != NULL)
        {
            for(i=0;i<command->selection->numShips;i++)
            {
                if(!isCapitalShip(command->selection->ShipPtr[i]))
                {
                    //returns if a NON capital ship is in the selection.
                    return;
                }
            }
            if(command->ordertype.order == COMMAND_NULL)
            {
                //command group is doint nothing but passive attacking
                if(command->ordertype.attributes & COMMAND_IS_FORMATION)
                {
                    if(command->selection->ShipPtr[0] != ship)
                        return; //not leader..so return! so we only move leader
                }
                if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN ||
                   command->ordertype.attributes & COMMAND_IS_PROTECTING)
                {
                    return;  //if doing either of these things...we don't want to back up
                }
            }
            else
            {
                return; //if doing something else, we have to return
            }
        }
        vecScalarMultiply(trajectory,trajectory,-5000.0f);
        vecAddTo(trajectory,ship->posinfo.position);
        aishipFlyToPointAvoidingObjs(ship,&trajectory,AISHIP_FastAsPossible,0.0f);
    }
    */
}

/*-----------------------------------------------------------------------------
    Name        : attackSimple
    Description : simplest attack possible, attacks target without turning or anything
    Inputs      : ship, target
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackSimple(Ship *ship,SpaceObjRotImpTarg *target)
{
    vector trajectory;
    real32 range;
    real32 dist;
    real32 temp;

    aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);

    dist = fsqrt(vecMagnitudeSquared(trajectory));
    vecDivideByScalar(trajectory,dist,temp);

    range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);
    gunShootGunsAtTarget(ship,(SpaceObjRotImpTarg *)target,range,&trajectory);
    ship->shipisattacking = FALSE;      // we're not attacking, just blowing stuff up in our way
}

/*-----------------------------------------------------------------------------
    Name        : attackPassive
    Description : does a passive attack (shoots at enemy if possible, but does not try to face in enemy's direction or
                  to move in enemy's direction)
    Inputs      : ship, target
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackPassive(Ship *ship,Ship *target)
{
    vector trajectory;
    real32 range;
    real32 dist;
    real32 temp;
//    CommandToDo *command;
//    sdword i;

    if (ship->specialFlags & SPECIAL_Hyperspacing)
        return;

    if (ship->attackvars.multipleAttackTargets)
    {
        gunShootGunsAtMultipleTargets(ship);
    }
    else
    {
        aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);

        dist = fsqrt(vecMagnitudeSquared(trajectory));
        vecDivideByScalar(trajectory,dist,temp);

        range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);
        gunShootGunsAtTarget(ship,(SpaceObjRotImpTarg *)target,range,&trajectory);
    }

    /*
        //check to see if we should back up!
    if(ship->tacticstype == Evasive)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
        if(command != NULL)
        {
            for(i=0;i<command->selection->numShips;i++)
            {
                if(!isCapitalShip(command->selection->ShipPtr[i]))
                {
                    //returns if a NON capital ship is in the selection.
                    return;
                }
            }
            if(command->ordertype.order == COMMAND_NULL)
            {
                //command group is doint nothing but passive attacking
                if(command->ordertype.attributes & COMMAND_IS_FORMATION)
                {
                    if(command->selection->ShipPtr[0] != ship)
                        return; //not leader..so return! so we only move leader
                }
                if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN ||
                   command->ordertype.attributes & COMMAND_IS_PROTECTING)
                {
                    return;  //if doing either of these things...we don't want to back up
                }
            }
            else
            {
                return; //if doing something else, we have to return
            }
        }
        vecScalarMultiply(trajectory,trajectory,-5000.0f);
        vecAddTo(trajectory,ship->posinfo.position);
        aishipFlyToPointAvoidingObjs(ship,&trajectory,AISHIP_FastAsPossible,0.0f);
    }
    */
}

/*-----------------------------------------------------------------------------
    Name        : attackStraightForward
    Description : attacks target by going straight for it (but not too close).
    Inputs      : ship, target, gunRange, tooCloseRange
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackStraightForward(Ship *ship,SpaceObjRotImpTarg *target,real32 gunRange,real32 tooCloseRange)
{
    vector trajectory;
    vector destination,newHeading;
    real32 dist;
    real32 range;
    real32 temp;
    bool shootguns = FALSE;
    CommandToDo *targetCommand;


    aishipGetTrajectory(ship,target,&trajectory);

    dist = fsqrt(vecMagnitudeSquared(trajectory));
    vecDivideByScalar(trajectory,dist,temp);

    range = RangeToTargetGivenDist(ship,target,dist);

    if(capShipLieFlatInGeneralSpeedSqrTolerance[ship->staticinfo->shipclass] != 0.0f
       && needToGoToSameVerticalPlane(ship,target,capShipLieFlatInGeneralDistanceTolerance[ship->staticinfo->shipclass],capShipLieFlatInGeneralSpeedSqrTolerance[ship->staticinfo->shipclass]))
    {
        destination.x = ship->posinfo.position.x;
        destination.z = target->posinfo.position.z;
        destination.y = ship->posinfo.position.y;

        //aishipFlyToPointAvoidingObjs(ship,&destination,AISHIP_FastAsPossible,0.0f);

        //also fly towards the ship
        if (range > tooCloseRange)
        {
            // about right range
            vecAddTo(destination,target->posinfo.position);
            vecMultiplyByScalar(destination,0.5f);      // avg of target position and previous destination
        }

        if(target->objtype == OBJ_ShipType)
        {
            if( ((Ship *)target)->staticinfo->shipclass == CLASS_Fighter ||
                ((Ship *)target)->staticinfo->shipclass == CLASS_Corvette)
            {
                targetCommand = getShipAndItsCommand(&universe.mainCommandLayer,(Ship *)target);
                if(targetCommand != NULL)
                {
                    //target is a ship and is doing something
                    if(targetCommand->ordertype.order == COMMAND_ATTACK)
                    {
                        //target is attacking something
                        if( ((Ship *)((Ship *)target)->attackvars.attacktarget) == ship)
                        {
                            //target is attacking ship
                            //don't move to same plane because the ship is coming back
                            goto dontgotosameplane;
                        }
                    }
                }
            }
        }

        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&target->posinfo.velocity);
dontgotosameplane:

        newHeading.y = trajectory.y;
        newHeading.x = trajectory.x;
        newHeading.z = 0.0f;
        vecNormalize(&newHeading);
        aitrackHeading(ship,&newHeading,FLYSHIP_ATTACKACCURACY);

        if (range < gunRange)
        {
            shootguns = TRUE;
        }
    }
    else
    {
        if (range > gunRange)
        {
            // too far away, so fly in

            aishipFlyToShipAvoidingObjsWithVel(ship,target,0,0.0f,&target->posinfo.velocity);
        }
        else if (range > tooCloseRange)
        {
            // about right range
            aishipFlyToShipAvoidingObjsWithVel(ship,target,0,0.0f,&target->posinfo.velocity);
            shootguns = TRUE;
        }
        else
        {
            // too close
            aitrackZeroVelocity(ship);
            shootguns = TRUE;
        }

        aitrackHeading(ship,&trajectory,FLYSHIP_ATTACKACCURACY);
    }

    if (shootguns)
    {
        if (ship->attackvars.multipleAttackTargets)
        {
            gunShootGunsAtMultipleTargets(ship);
        }
        else
        {
            gunShootGunsAtTarget(ship,target,range,&trajectory);
        }
    }
}

#define SIDESTEP_APPROACH      0
#define SIDESTEP_KILL          1
#define SIDESTEP_REPOSITION    2
#define SIDESTEP_APPROACHREPOSITION 3

/*-----------------------------------------------------------------------------
    Name        : attackSideStepInit
    Description : initializes variables for attacksidestep
    Inputs      : attacksidestep
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackSideStepInit(AttackSideStep *attacksidestep)
{
    attacksidestep->aitime = 0.0f;
    attacksidestep->aidirection = 0;
}

/*-----------------------------------------------------------------------------
    Name        : attackSideStep
    Description : Does a "sidestep" attack.  The ship will shoot at target, and then at random either slide left or
                  right before doing another attack.
    Inputs      : ship, target, attacksidestep (contains internal state variables), parameters (tunable constants)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void attackSideStep(Ship *ship,SpaceObjRotImpTarg *target,AttackSideStep *attacksidestep,AttackSideStepParameters *parameters)
{
    vector trajectory;
    real32 dist;
    real32 range;
    real32 temp;
    bool didshoot;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    switch (ship->aistateattack)
    {
        case SIDESTEP_APPROACH:
#ifdef DEBUG_ATTACK
            dbgMessagef("\nShip %x SIDESTEP_APPROACH",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,0.0f);
            range = RangeToTarget(ship,target,&trajectory);

            didshoot = FALSE;
            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                if (ship->attackvars.multipleAttackTargets)
                {
                    didshoot = gunShootGunsAtMultipleTargets(ship);
                }
                else
                {
                    didshoot = gunShootGunsAtTarget(ship,target,range,&trajectory);
                }
            }

            if (didshoot)
            {
                ship->aistateattack = SIDESTEP_APPROACHREPOSITION;
                attacksidestep->aitime = universe.totaltimeelapsed;
                if (parameters->fullMovementFreedom)
                {
                    attacksidestep->aidirection = (gamerand() & 3);
                }
                else
                {
                    if (ship->specialFlags & SPECIAL_AttackFromAbove)
                        attacksidestep->aidirection = (gamerand() & 3) ? TRANS_UP : TRANS_DOWN;
                    else
                        attacksidestep->aidirection = (gamerand() & 3) ? TRANS_RIGHT : TRANS_LEFT;
                }
                break;
            }

            if (range < parameters->circleRange)
            {
                ship->aistateattack = SIDESTEP_KILL;
            }
            break;

        case SIDESTEP_APPROACHREPOSITION:
#ifdef DEBUG_ATTACK
            dbgMessagef("\nShip %x SIDESTEP_APPROACHREPOSITION",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory,dist,temp);
            aitrackHeadingWithBank(ship,&trajectory,FLYSHIP_HEADINGACCURACY,shipstaticinfo->sinbank);

            range = RangeToTargetGivenDist(ship,target,dist);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*0.5f,TRANS_FORWARD);
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[attacksidestep->aidirection],(uword)attacksidestep->aidirection);

            aishipFlyToPointAvoidingObjsWithVel(ship,NULL,0,0.0f,&ship->posinfo.velocity);     // just avoid objects

            if (ship->attackvars.multipleAttackTargets)
            {
                gunShootGunsAtMultipleTargets(ship);
            }

            if ((universe.totaltimeelapsed - attacksidestep->aitime) > parameters->repositionTime)
            {
                if (range < parameters->circleRange)
                {
                    ship->aistateattack = SIDESTEP_KILL;
                }
                else
                {
                    ship->aistateattack = SIDESTEP_APPROACH;
                }
            }
            break;

        case SIDESTEP_KILL:
#ifdef DEBUG_ATTACK
            dbgMessagef("\nShip %x SIDESTEP_KILL",(udword)ship);
#endif

            aishipGetTrajectory(ship,target,&trajectory);

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory,dist,temp);

            aishipFlyToPointAvoidingObjsWithVel(ship,NULL,0,0.0f,&ship->posinfo.velocity);     // just avoid objects

            aitrackHeadingWithBank(ship,&trajectory,FLYSHIP_ATTACKACCURACY,shipstaticinfo->sinbank);

            range = RangeToTargetGivenDist(ship,target,dist);

            didshoot = FALSE;
            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                if (ship->attackvars.multipleAttackTargets)
                {
                    didshoot = gunShootGunsAtMultipleTargets(ship);
                }
                else
                {
                    didshoot = gunShootGunsAtTarget(ship,target,range,&trajectory);
                }
            }

            if (didshoot)
            {
                ship->aistateattack = SIDESTEP_REPOSITION;
                attacksidestep->aitime = universe.totaltimeelapsed;
                if (parameters->fullMovementFreedom)
                {
                    attacksidestep->aidirection = (gamerand() & 3);
                }
                else
                {
                    if (ship->specialFlags & SPECIAL_AttackFromAbove)
                        attacksidestep->aidirection = (gamerand() & 3) ? TRANS_UP : TRANS_DOWN;
                    else
                        attacksidestep->aidirection = (gamerand() & 3) ? TRANS_RIGHT : TRANS_LEFT;
                }
                break;
            }

            if (range > parameters->circleRange)
            {
                ship->aistateattack = SIDESTEP_APPROACH;
            }

            break;

        case SIDESTEP_REPOSITION:
#ifdef DEBUG_ATTACK
            dbgMessagef("\nShip %x SIDESTEP_REPOSITION",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory,dist,temp);
            aitrackHeadingWithBank(ship,&trajectory,FLYSHIP_HEADINGACCURACY,shipstaticinfo->sinbank);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[attacksidestep->aidirection],(uword)attacksidestep->aidirection);
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*0.5f,TRANS_FORWARD);

            aishipFlyToPointAvoidingObjsWithVel(ship,NULL,0,0.0f,&ship->posinfo.velocity);     // just avoid objects

            range = RangeToTargetGivenDist(ship,target,dist);

            if (ship->attackvars.multipleAttackTargets)
            {
                gunShootGunsAtMultipleTargets(ship);
            }

            if (range > parameters->circleRange)
            {
                ship->aistateattack = SIDESTEP_APPROACH;
                break;
            }

            if ((universe.totaltimeelapsed - attacksidestep->aitime) > parameters->repositionTime)
            {
                if (range < parameters->circleRange)
                {
                    ship->aistateattack = SIDESTEP_KILL;
                }
                else
                {
                    ship->aistateattack = SIDESTEP_APPROACH;
                }
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

}

