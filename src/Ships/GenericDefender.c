/*=============================================================================
    Name    : GenericDefender.c
    Purpose : Specifics for the Generic Defender

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "GenericDefender.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "ShipSelect.h"
#include "AIShip.h"
#include "FastMath.h"
#include "Gun.h"
#include "Collision.h"
#include "AITrack.h"
#include "Battle.h"
#include "MinelayerCorvette.h"
#include "SoundEvent.h"
#include "Physics.h"
#include "GenericInterceptor.h"

#ifdef gshaw
#define DEBUG_DEFENDER
#endif

//local prototypes
bool kamikazeAttack(Ship *ship,SelectAnyCommand *targets);

typedef struct
{
    real32 gunRange[NUM_TACTICS_TYPES];
    real32 tooCloseRange[NUM_TACTICS_TYPES];
    real32 CIRCLE_RIGHT_VELOCITY;
    real32 CIRCLE_RIGHT_THRUST;
} GenericDefenderStatics;

GenericDefenderStatics LightDefenderStaticRace1;
GenericDefenderStatics LightDefenderStaticRace2;

GenericDefenderStatics HeavyDefenderStaticRace1;
GenericDefenderStatics HeavyDefenderStaticRace2;

scriptStructEntry DefenderStaticScriptTable[] =
{
    { "CIRCLE_RIGHT_VELOCITY",    scriptSetReal32CB, (udword) &(HeavyDefenderStaticRace1.CIRCLE_RIGHT_VELOCITY), (udword) &(HeavyDefenderStaticRace1) },
    { "CIRCLE_RIGHT_THRUST",      scriptSetReal32CB, (udword) &(HeavyDefenderStaticRace1.CIRCLE_RIGHT_THRUST), (udword) &(HeavyDefenderStaticRace1) },
    { NULL,NULL,0,0 }
};

void GenericDefenderStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    GenericDefenderStatics *defenderstat;

    switch (statinfo->shiptype)
    {
        case LightDefender:
           defenderstat = (statinfo->shiprace == R1) ? &LightDefenderStaticRace1 : &LightDefenderStaticRace2;
           break;
        case HeavyDefender:
           defenderstat = (statinfo->shiprace == R1) ? &HeavyDefenderStaticRace1 : &HeavyDefenderStaticRace2;
           break;
        default:
            dbgAssert(FALSE);
    }

    statinfo->custstatinfo = defenderstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        defenderstat->gunRange[i] = statinfo->bulletRange[i];
        defenderstat->tooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }

    defenderstat->CIRCLE_RIGHT_VELOCITY = 20.0f;
    defenderstat->CIRCLE_RIGHT_THRUST = 0.1f;

    scriptSetStruct(directory,filename,DefenderStaticScriptTable,(ubyte *)defenderstat);
}

#define DEFENDER_APPROACH        1
#define DEFENDER_CIRCLE          2

void GenericDefenderAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    GenericDefenderStatics *defenderstat = (GenericDefenderStatics *)shipstaticinfo->custstatinfo;
    real32 newrange;
    //newrange = maxdist - defenderstat->gunRange;
    //newrange = defenderstat->gunRange - newrange;
    //newrange = defenderstat->gunRange - maxdist + defenderstat->gunRange;
    //could do check if maxdist == gunRange, then don't do calcs, just use
    //max dist

    vector trajectory;
    real32 dist;
    real32 range;
    real32 temp;
    real32 rightvelocity;
    vector right;

    newrange = (defenderstat->gunRange[ship->tacticstype]*2) - maxdist;
    if(newrange < 0.0f)
        newrange = defenderstat->tooCloseRange[ship->tacticstype] + 100.0f;
    else if(newrange > defenderstat->gunRange[ship->tacticstype])
    {
        newrange = defenderstat->gunRange[ship->tacticstype];
    }

    switch (ship->aistateattack)
    {
        case 0:
            ship->aistateattack = DEFENDER_APPROACH;
            // fall through to DEFENDER_APPROACH

        case DEFENDER_APPROACH:
            aishipGetTrajectory(ship,target,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,target,dist);
            if (range > newrange)
            {
                aishipFlyToShipAvoidingObjsWithVel(ship,target,AISHIP_FastAsPossible|AISHIP_PointInDirectionFlying,0.0f,&target->posinfo.velocity);
                break;
            }
#ifdef DEBUG_DEFENDER
            dbgMessagef("\n%x Changing to circle",ship);
#endif
            ship->aistateattack = DEFENDER_CIRCLE;
            goto circleAlreadyCalculatedTrajectory;

        case DEFENDER_CIRCLE:
            aishipGetTrajectory(ship,target,&trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship,target,dist);
            if (range > newrange)
            {
#ifdef DEBUG_DEFENDER
            dbgMessagef("\n%x Changing back to approach",ship);
#endif
                ship->aistateattack = DEFENDER_APPROACH;
                break;
            }
circleAlreadyCalculatedTrajectory:

            vecDivideByScalar(trajectory,dist,temp);

            aitrackHeading(ship,&trajectory,FLYSHIP_ATTACKACCURACY);

            matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);
            rightvelocity = vecDotProduct(right,ship->posinfo.velocity);

            if (rightvelocity < defenderstat->CIRCLE_RIGHT_VELOCITY)
            {
                physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_RIGHT]*defenderstat->CIRCLE_RIGHT_THRUST,TRANS_RIGHT);
            }

            if (range < defenderstat->tooCloseRange[ship->tacticstype])
            {
                aitrackZeroForwardVelocity(ship);
            }

            if (ship->attackvars.multipleAttackTargets)
            {
                gunShootGunsAtMultipleTargets(ship);
            }
            else
            {
                gunShootGunsAtTarget(ship,target,range,&trajectory);
            }

            break;

        default:
            dbgAssert(FALSE);
            break;
    }

//    attackStraightForward(ship,target,newrange,defenderstat->tooCloseRange[ship->tacticstype]);
}

void GenericDefenderAttackPassive(Ship *ship,Ship *target,bool rotate)
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

/*=============================================================================
    Name    : kamikazeAttack
    Purpose : Flys ship into one of the targets in the target list

=============================================================================*/
//if ship misses a ship, this is the so called wind up distance

#define K_Start         0
#define K_Ram           1
#define K_Ram2          2
#define K_TurnAround    3
#define K_TurnAround2   4
#define K_Yell          5

#define kamikazeFlyByDist 1200.0f

void doKamikazeAttack(Ship *ship,SpaceObjRotImpTarg *target)
{
    real32 mag,dist,range,shipvel;
    vector destination,trajectory,heading,shipheading,EinsteinVelocity;
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;

    vecSub(trajectory,target->collInfo.collPosition,ship->posinfo.position);
    mag = vecMagnitudeSquared(trajectory);
    dist = fsqrt(mag);

    destination = trajectory;
    vecNormalize(&destination);
    heading = destination;

    //this hokey fudge factor is needed so the kamikaze ship doesn't
    //slow down as it reaches its destination
    mag = dist + 500.0f;
    vecScalarMultiply(destination,destination,mag);
    vecAddTo(destination,target->collInfo.collPosition);

    range = RangeToTargetGivenDist(ship,target,dist);

    switch(ship->kamikazeState)
    {
    case K_Start:
        //if within a threshold range
        bitClear(ship->specialFlags,SPECIAL_KamikazeCrazyFast);
        if(range < 500.0f)
        {
            //calculate flyby dist
            matGetVectFromMatrixCol3(shipheading, ship->rotinfo.coordsys);
            vecScalarMultiply(shipheading,shipheading,kamikazeFlyByDist);
            vecAdd(ship->kamikazeVector,shipheading,target->collInfo.collPosition);
            ship->kamikazeState = K_TurnAround;
        }
        ship->kamikazeState = K_Ram;

        break;

    case K_Ram:
        if (range < 400.0f)
        {
            ship->kamikazeState = K_Yell;
        }
    case K_Ram2:
        //lets use...mmmmm..twister theor...no...relativity
        vecSub(EinsteinVelocity,ship->posinfo.velocity,target->posinfo.velocity);
        shipvel = fsqrt(vecMagnitudeSquared(EinsteinVelocity));
        if(ship->shiptype == MinelayerCorvette)
        {
            ((MinelayerCorvetteSpec *)(ship->ShipSpecifics))->mineaistate = MINE_DROP_ATTACK;
        }
        else if (ship->shiptype == DefenseFighter)
        {
            goto dontshoot;
        }

        if (isShipStaticInterceptor(shipstaticinfo))
        {
            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)shipstaticinfo->custstatinfo;
                uword targetIndex;

                if (target->objtype == OBJ_ShipType)
                {
                    targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
                }
                else
                {
                    targetIndex = (uword)NUM_CLASSES;
                }

                if (GenericInterceptorCanFire(ship,target,&trajectory,interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
                {
                    ship->staticinfo->custshipheader.CustShipFire(ship,target);
                }
            }
        }
        else
        {
            gunShootGunsAtTarget(ship,target,range,&trajectory);
        }
dontshoot:
        aishipFlyToPointAvoidingObjs(ship,&destination,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying | AISHIP_FastAsPossible,0.0f);
        if(range < 1500.0f)
        {
            bitSet(ship->specialFlags,SPECIAL_KamikazeCrazyFast);
        }
        else
        {
            bitClear(ship->specialFlags,SPECIAL_KamikazeCrazyFast);
        }

/*
        if(range < 400.0f && (shipvel < ship->staticinfo->staticheader.maxvelocity*0.7f ||
                   vecDotProduct(trajectory,heading) < 0))
        {
            //ship is inside 400.0f, and isn't faceing ship, or isn't
            //going fast enough

            //calculate flyby dist
            matGetVectFromMatrixCol3(shipheading, ship->rotinfo.coordsys);
            vecScalarMultiply(shipheading,shipheading,kamikazeFlyByDist);
            vecAdd(ship->kamikazeVector,shipheading,target->posinfo.position);
            ship->kamikazeState = K_TurnAround;
        }
*/
        break;
    case K_TurnAround:
        bitClear(ship->specialFlags,SPECIAL_KamikazeCrazyFast);
        aishipFlyToPointAvoidingObjs(ship,&ship->kamikazeVector,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying | AISHIP_FastAsPossible,0.0f);
        if(MoveReachedDestinationVariable(ship,&ship->kamikazeVector,300.0f))
            ship->kamikazeState = K_TurnAround2;
        break;
    case K_TurnAround2:
        bitClear(ship->specialFlags,SPECIAL_KamikazeCrazyFast);
        if(aitrackHeadingWithFlags(ship,&heading,0.90f,AITRACKHEADING_IGNOREUPVEC))
            ship->kamikazeState = K_Ram;
        break;
    case K_Yell:
//      speechEvent(ship, COMM_LInt_Kamikaze, 0);
        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Kamikaze, ship, SOUND_EVENT_DEFAULT);
        ship->kamikazeState = K_Ram2;
        break;
    }

        //fire guns here...
    //
    //
    //Sound note:   Could put in kamikaze state change here after
    //              getting to within a certain distance of target
    //
    //          ask bryce how to...
}

CustShipHeader GenericDefenderHeader =
{
    (ShipType)-1,
    sizeof(GenericDefenderSpec),
    GenericDefenderStaticInit,
    NULL,
    NULL,
    NULL,
    GenericDefenderAttack,
    DefaultShipFire,
    GenericDefenderAttackPassive,
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

