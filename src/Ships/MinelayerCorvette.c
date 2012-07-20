/*=============================================================================
    Name    : MinelayerCorvette.c
    Purpose : Specifics for the Minelayer Corvette

    Created 11/5/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


//#define DEBUG_ATTACK

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "Types.h"
#include "Debug.h"
#include "UnivUpdate.h"
#include "SpaceObj.h"
#include "MinelayerCorvette.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "Universe.h"
#include "Gun.h"
#include "AITrack.h"
#include "AIShip.h"
#include "Attack.h"
#include "Collision.h"
#include "FastMath.h"
#include "Physics.h"
#include "SaveGame.h"
#include "Randy.h"
#include "SoundEvent.h"
#include "Battle.h"

MinelayerCorvetteStatics MinelayerCorvetteStatic;

MinelayerCorvetteStatics MinelayerCorvetteStaticRace1;
MinelayerCorvetteStatics MinelayerCorvetteStaticRace2;

#define FIRST_OFF                       99
#define FIRST_OFF2                      100
#define BEGIN_WALL_DROP_RIGHT           101
#define BEGIN_WALL_DROP_UP              102
#define BEGIN_WALL_DROP_DOWN            103
#define BEGIN_WALL_DROP_LEFT            104
#define DONE_WAIT                       105

real32 mothershipDistSqr;
real32 mothershipDistSqr2;

#define ATTACK_INIT                 0
#define APPROACH                    1
#define BREAK1                      2
#define BREAKPOSITION               3
#define KILL                        4
#define BREAK2                      5

#define DROP_MOTHERSHIP				7

#define DIST_FROM_HALL_BREAKPOS     600

scriptStructEntry MinelayerCorvetteStaticScriptTable[] =
{
    { "NumMinesInSide",    scriptSetUdwordCB, (udword) &(MinelayerCorvetteStatic.NumMinesInSide), (udword) &(MinelayerCorvetteStatic) },
    { "MINE_STOP_FRICTION",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.MINE_STOP_FRICTION), (udword) &(MinelayerCorvetteStatic) },
    { "MineSpacing",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.MineSpacing), (udword) &(MinelayerCorvetteStatic) },
    { "MineDropDistance",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.MineDropDistance), (udword) &(MinelayerCorvetteStatic) },

    { "breakInAwayDist",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.breakInAwayDist), (udword) &(MinelayerCorvetteStatic) },
    { "DropRange",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.DropRange), (udword) &(MinelayerCorvetteStatic) },
    { "DropStopRange",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.DropStopRange), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Mothership]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Mothership]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_HeavyCruiser]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_HeavyCruiser]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Carrier]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Carrier]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Destroyer]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Destroyer]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Frigate]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Frigate]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Corvette]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Corvette]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Fighter]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Fighter]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_Resource]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_Resource]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayDist[CLASS_NonCombat]",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayDist[CLASS_NonCombat]), (udword) &(MinelayerCorvetteStatic) },
    { "FlyAwayTolerance",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.FlyAwayTolerance), (udword) &(MinelayerCorvetteStatic) },
    { "Break2SphereizeFreq",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.Break2SphereizeFreq), (udword) &(MinelayerCorvetteStatic) },
    { "MineClearDistance",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.MineClearDistance), (udword) &(MinelayerCorvetteStatic) },
    { "forced_drop_damage_lo",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.forced_drop_damage_lo), (udword) &(MinelayerCorvetteStatic) },
    { "forced_drop_damage_hi",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.forced_drop_damage_hi), (udword) &(MinelayerCorvetteStatic) },
    { "forced_drop_lifetime",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.forced_drop_lifetime), (udword) &(MinelayerCorvetteStatic) },
    { "gunReFireTime",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.gunReFireTime), (udword) &(MinelayerCorvetteStatic) },
    { "mineRegenerateTime",    scriptSetReal32CB, (udword) &(MinelayerCorvetteStatic.mineRegenerateTime), (udword) &(MinelayerCorvetteStatic) },
    { NULL,NULL,0,0 }
};

bool MinelayerCorvetteStaticMineDrop(Ship *ship,SpaceObjRotImpTarg *target);


void MinelayerCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    MinelayerCorvetteStatics *corvstat = (statinfo->shiprace == R1) ? &MinelayerCorvetteStaticRace1 : &MinelayerCorvetteStaticRace2;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;

    scriptSetStruct(directory,filename,MinelayerCorvetteStaticScriptTable,(ubyte *)corvstat);
    corvstat->DropStopRadiusSqr = corvstat->MineDropDistance * corvstat->MineDropDistance;
    corvstat->NumMinesInSideSqr = corvstat->NumMinesInSide*corvstat->NumMinesInSide;
    corvstat->MineClearDistanceSQR = corvstat->MineClearDistance*corvstat->MineClearDistance;
}

void MinelayerCorvetteInit(Ship *ship)
{
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);
    spec->MiningStatus = FIRST_OFF;
    ship->aistateattack =  ATTACK_INIT;
    spec->MineDropNumber = 0;
    spec->lasttimeRegeneratedMines = 0.0f;

	//special case motherships stuff...day of gold yadda yadda...
	mothershipDistSqr = 6000.0f*6000.0f;
	mothershipDistSqr2 = 8000.0f*8000.0f;
}

void MinelayerCorvetteFire(Ship *ship,SpaceObjRotImpTarg *target)
{
    GunInfo *guninfo = ship->gunInfo;
    Gun *gun0,*gun1;

    if (guninfo == NULL)
    {
        dbgAssert(FALSE);       //should have gun info...something is teribly wrong...
    }
    gun0 = ((Gun *) &(ship->gunInfo->guns[0]));

    if(ship->gunInfo->numGuns > 1)
    {    //ship has 2 guns (race 1)
        gun1 = ((Gun *) &(ship->gunInfo->guns[1]));
        if(gun0->lasttimefired <= gun1->lasttimefired)
        {    //Gun 0's turn to fire
            if (gun0->numMissiles >0)
            {   //if there is ammo
                if (gunCanShoot(ship, gun0))
                {
                    missileShoot(ship,gun0, target);
                }
            }
            else
            {
                if(gun1->numMissiles >0)
                {
                    if (gunCanShoot(ship, gun1))
                    {
                        missileShoot(ship,gun1, target);
                    }
                }
            }
        }
        else
        {
            if (gun1->numMissiles >0)
            {   //if there is ammo
                if (gunCanShoot(ship, gun1))
                {
                    missileShoot(ship,gun1, target);
                }
            }
            else
            {
                if(gun0->numMissiles >0)
                {
                    if (gunCanShoot(ship, gun0))
                    {
                        missileShoot(ship,gun0, target);
                    }
                }
            }
        }
    }
    else
    {    //ship is race 2 wiht 1 gun only
        if (gun0->numMissiles >0)
        {   //if there is ammo
            if (gunCanShoot(ship, gun0))
            {
            missileShoot(ship,gun0, target);
            }
        }
    }
}

void SetAIVecHeading(Ship *ship, SpaceObjRotImpTarg *target, vector *trajectory)
{
    //real32 range;
    vector tmpvec;
    udword target_class;
    real32 randegf;
    sdword randeg;
    matrix tmpmat;
    vector targetheading;
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    MinelayerCorvetteStatics *minelayercorvettestatics;
    minelayercorvettestatics = (MinelayerCorvetteStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    matGetVectFromMatrixCol3(targetheading,target->rotinfo.coordsys)

    if(target != NULL)
    {
        if(target->objtype == OBJ_ShipType)
            target_class = ((Ship *)target)->staticinfo->shipclass;
        else
            target_class = CLASS_NonCombat;
    }

    if (vecDotProduct(targetheading,*trajectory) > 0)
    {   //Attack accross front of ship :)
        vecAddTo(spec->aivec,targetheading);
        vecAddTo(spec->aivec,*trajectory);
        vecScalarMultiply(spec->aivec,spec->aivec,minelayercorvettestatics->FlyAwayDist[target_class]);
    }
    else
    {    //test
        if(randombetween(1,2) & 1)
        {
            vecCrossProduct(spec->aivec,*trajectory, targetheading);
        }
        else
        {
            vecCrossProduct(spec->aivec, targetheading, *trajectory);
        }
        vecAddTo(spec->aivec, *trajectory);
        vecScalarMultiply(spec->aivec,spec->aivec,minelayercorvettestatics->FlyAwayDist[target_class]);
    }
    if(!(target->posinfo.isMoving & ISMOVING_MOVING))
    {    //if target isn't moving, randomize trajectory
        randeg = randombetween(5,30);
        randegf = (real32) ((randeg & 1) ? randeg : -randeg);
        randegf = (real32) randeg;
        randegf = DEG_TO_RAD(randegf);

        matMakeRotAboutZ(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
        matMultiplyMatByVec(&tmpvec,&tmpmat,&spec->aivec);
        spec->aivec = tmpvec;
    }
    vecAddTo(spec->aivec,target->posinfo.position);

    //need change
    aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY);

}



/*-----------------------------------------------------------------------------
    Name        : MineLayerAttackRun
    Description : Does a "sidestep" attack.  The ship will shoot at target, and then at random either slide left or
                  right before doing another attack.
    Inputs      : ship, target, attacksidestep (contains internal state variables), parameters (tunable constants)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void MineLayerAttackRun(Ship *ship,SpaceObjRotImpTarg *target,AttackSideStep *attacksidestep,AttackSideStepParameters *parameters)
{
    vector trajectory;
    real32 range;
    Gun *gun;
    udword numGuns,target_class;
    GunInfo *guninfo = ship->gunInfo;
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    MinelayerCorvetteStatics *minelayercorvettestatics;
    minelayercorvettestatics = (MinelayerCorvetteStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    numGuns = guninfo->numGuns;

    gun = &guninfo->guns[0];
    if(target != NULL)
    {
        if(target->objtype == OBJ_ShipType)
            target_class = ((Ship *)target)->staticinfo->shipclass;
        else
            target_class = CLASS_NonCombat;
    }

    switch (ship->aistateattack)
    {
        case ATTACK_INIT:
        case APPROACH:
#ifdef DEBUG_ATTACK
            dbgMessagef("\nShip %x MINELAYER_ATTACK_APPROACH",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying,0.0f);
            range = RangeToTarget(ship,target,&trajectory);

			//lets check if we want to force drop...
			if(target->objtype == OBJ_ShipType)
			{
				if(((Ship *)target)->shiptype == Mothership)
				{
					//its a mothership
					vector tempvec;
					real32 tempreal;
					vecSub(tempvec,target->collInfo.collPosition,ship->collInfo.collPosition);
					tempreal = vecMagnitudeSquared(tempvec);
					if(tempreal < mothershipDistSqr)
					{
						//we're within range of force dropping!
						ship->aistateattack = DROP_MOTHERSHIP;
					}
					break;
				}

			}


            if (range < minelayercorvettestatics->breakInAwayDist)
            {
                ship->aistateattack = BREAKPOSITION;
                spec->aivec.x = 0.0f;
                spec->aivec.y = 0.0f;
                spec->aivec.z = 0.0f;
            }

            break;
		case DROP_MOTHERSHIP:
			{
				vector tempvec;
				real32 tempreal;
				vecSub(tempvec,ship->collInfo.collPosition,target->collInfo.collPosition);
				tempreal = vecMagnitudeSquared(tempvec);
				vecNormalize(&tempvec);
				if(tempreal > mothershipDistSqr2)
				{
					ship->aistateattack = ATTACK_INIT;
				}
				if(aitrackHeadingWithFlags(ship,&tempvec,0.97f,AITRACKHEADING_IGNOREUPVEC))
				{
					if(MinelayerCorvetteStaticMineDrop(ship,target))		//problem...there will be other targets...bah..lets see..
					{
						MinelayerCorvetteOrderChangedCleanUp(ship);
					}
				}
				break;
			}
        case BREAKPOSITION:
#ifdef DEBUG_ATTACK
        dbgMessagef("\nShip %x BREAKPOSITION",(udword)ship);
#endif

            aishipGetTrajectory(ship,target,&trajectory);
            range = RangeToTarget(ship,target,&trajectory);
            vecNormalize(&trajectory);

            SetAIVecHeading(ship,target,&trajectory);

            ship->aistateattack = BREAK1;

        case BREAK1:
#ifdef DEBUG_ATTACK
    dbgMessagef("\nShip %x BREAK1",(udword)ship);
#endif

            aishipGetTrajectory(ship,target,&trajectory);
            range = RangeToTarget(ship,target,&trajectory);


            //aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY);
            aishipFlyToPointAvoidingObjs(ship,&target->posinfo.position,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY);
            if(range < minelayercorvettestatics->DropRange)
            {
                //temp
                vecNormalize(&trajectory);
                SetAIVecHeading(ship,target,&trajectory);
                //temp

                ship->aistateattack = KILL;    //within mining range so start dropping
                spec->aispheretime=0.0f;            //reset time;
            }

            break;
        case KILL:
#ifdef DEBUG_ATTACK
    dbgMessagef("\nShip %x KILL",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            range = RangeToTarget(ship,target,&trajectory);

            spec->aispheretime += universe.phystimeelapsed;

            if(gunCanShoot(ship, gun))
            {
                if(gun->numMissiles > 0)
                {
                   spec->mineaistate = MINE_DROP_ATTACK;
                   MinelayerCorvetteFire(ship,target);
                 }
            }
            if(range > minelayercorvettestatics->DropStopRange)
            {   //out of range....
                ship->aistateattack = BREAK2;
            }
            if(spec->aispheretime > minelayercorvettestatics->Break2SphereizeFreq)
            {    //time to sphereize;
                spec->aivec.x =0.0f;
                spec->aivec.y =0.0f;
                spec->aivec.z =0.0f;
                spec->aispheretime = -1000000.0f;     // Reset, and never do again till next attack pass...
                vecNormalize(&trajectory);
                SetAIVecHeading(ship,target,&trajectory);

#ifdef DEBUG_ATTACK
    dbgMessagef("\nShip %x KILL: Adjust for Break2 Sphereizing Godliness Maneuver :)",(udword)ship);
#endif

            }

            aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY);
            break;
        case BREAK2:
#ifdef DEBUG_ATTACK
    dbgMessagef("\nShip %x BREAK2",(udword)ship);
#endif

            aishipGetTrajectory(ship,target,&trajectory);
            range = RangeToTarget(ship,target,&trajectory);
            aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY);
            if(range > minelayercorvettestatics->FlyAwayDist[target_class] ||
               (MoveReachedDestinationVariable(ship,&spec->aivec,minelayercorvettestatics->FlyAwayTolerance)))
            {    //turn around and start over
                ship->aistateattack = APPROACH;
            }
            break;
        default:
            dbgAssert(FALSE);
            break;
    }

}
void MinelayerCorvetteOrderChangedCleanUp(Ship *ship)
{
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    spec->MiningStatus = FIRST_OFF;
    if(spec->mineforminfo != NULL)
    {
        spec->mineforminfo->FULL = TRUE;
        spec->mineforminfo = NULL;
    }

}
void MinelayerCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    MinelayerCorvetteStatics *corvstat = (MinelayerCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    MineLayerAttackRun(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

bool MinelayerCorvetteStaticMineDrop(Ship *ship,SpaceObjRotImpTarg *target)
{
    MinelayerCorvetteStatics *minelayercorvettestatics;
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    sdword flag;
    Gun *gun0,*gun1;
    real32 time;
    sdword maxmis;

    minelayercorvettestatics = (MinelayerCorvetteStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    gun0 = ((Gun *) &(ship->gunInfo->guns[0]));

    maxmis = gun0->gunstatic->maxMissiles;

    if(ship->gunInfo->numGuns > 1)
    {    //ship has 2 guns (race 1)

        gun1 = ((Gun *) &(ship->gunInfo->guns[1]));

        if(gun0->numMissiles == 0 && gun1->numMissiles == 0)
            return FALSE;

        if((universe.totaltimeelapsed - gun1->lasttimefired) < minelayercorvettestatics->gunReFireTime
           && (universe.totaltimeelapsed - gun0->lasttimefired) < minelayercorvettestatics->gunReFireTime)
        {
            return(FALSE);
        }
        maxmis += gun1->gunstatic->maxMissiles;
    }
    else
    {
        if(gun0->numMissiles == 0)
            return FALSE;

        if((universe.totaltimeelapsed - gun0->lasttimefired) < minelayercorvettestatics->gunReFireTime)
            return(FALSE);
    }


    switch(spec->MiningStatus)
    {
    case FIRST_OFF:
        //////////////////////
        //speech event for forcedropped mines
        //event num: COMM_MLVette_ForceDrop
        //use battle chatter
        if(ship->playerowner->playerIndex == universe.curPlayerIndex)
        {
            if (battleCanChatterAtThisTime(BCE_COMM_MLVette_ForceDrop, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_MLVette_ForceDrop, ship, SOUND_EVENT_DEFAULT);
            }
        }
        /////////////////////////

        spec->MiningStatus = FIRST_OFF2;
    case FIRST_OFF2:
        if(aitrackZeroRotationAnywhere(ship))
        {
            flag = 2;
        }

        if(aitrackZeroVelocity(ship))
        {
            if(flag == 2)
            {   //we're ready for next step
                MineFormationInfo *mineformationinfo;
                aitrackForceSteadyShip(ship);           //stop movement, stop rotation
                spec->MiningStatus = BEGIN_WALL_DROP_RIGHT;
                spec->MiningSideMax = 1;
                spec->MiningSideCount = 0;
                matGetVectFromMatrixCol1(spec->formation_up,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(spec->formation_right,ship->rotinfo.coordsys);
                vecScalarMultiply(spec->formation_up, spec->formation_up, minelayercorvettestatics->MineSpacing);
                vecScalarMultiply(spec->formation_down, spec->formation_up, -1.0f);
                vecScalarMultiply(spec->formation_right,spec->formation_right,minelayercorvettestatics->MineSpacing);
                vecScalarMultiply(spec->formation_left,spec->formation_right,-1.0f);
                matGetVectFromMatrixCol3(spec->formation_heading,ship->rotinfo.coordsys);
                vecScalarMultiply(spec->formation_heading,spec->formation_heading,-minelayercorvettestatics->MineDropDistance);

                //Create Formation Entity Here

                mineformationinfo = memAlloc(sizeof(MineFormationInfo),"MineFormationInfo",NonVolatile);
                listAddNode(&universe.MineFormationList,&(mineformationinfo->FormLink),mineformationinfo);
                listInit(&mineformationinfo->MineList);
                mineformationinfo->playerowner = ship->playerowner;
                mineformationinfo->FULL = FALSE;
                mineformationinfo->wallstate = PULSE_START;
                mineformationinfo->effect = NULL;
                spec->mineforminfo = mineformationinfo;
                time = 29.0;
                spec->mineforminfo->waittime = time;     //set wait time for each pulse

                spec->formation_number_X = 0;
                spec->formation_number_Y = 0;

                vecAdd(spec->formation_position,ship->posinfo.position,spec->formation_heading);
                spec->mineaistate = MINE_DROP_FORMATION;
                MinelayerCorvetteFire(ship,target);
                spec->MineDropNumber = 1;
            }
        }
        break;
    case BEGIN_WALL_DROP_RIGHT:
        vecAddTo(spec->formation_position,spec->formation_right);
        spec->mineaistate = MINE_DROP_FORMATION;

        MinelayerCorvetteFire(ship,target);
        spec->MineDropNumber++;
        spec->formation_number_X++;
        spec->MiningSideCount++;
        if(spec->MiningSideCount == spec->MiningSideMax)
        {
            spec->MiningSideCount = 0;
            spec->MiningStatus = BEGIN_WALL_DROP_UP;
        }
        break;
    case BEGIN_WALL_DROP_UP:
        vecAddTo(spec->formation_position,spec->formation_up);
        spec->mineaistate = MINE_DROP_FORMATION;
        MinelayerCorvetteFire(ship,target);
        spec->MineDropNumber++;
        spec->formation_number_Y++;
        spec->MiningSideCount++;
        if(spec->MiningSideCount == spec->MiningSideMax)
        {
            spec->MiningSideMax++;
            spec->MiningSideCount = 0;
            spec->MiningStatus = BEGIN_WALL_DROP_LEFT;
        }
        break;
    case BEGIN_WALL_DROP_LEFT:
        vecAddTo(spec->formation_position,spec->formation_left);
        spec->mineaistate = MINE_DROP_FORMATION;
        MinelayerCorvetteFire(ship,target);
        spec->MineDropNumber++;
        spec->formation_number_X--;
        spec->MiningSideCount++;
        if(spec->MiningSideCount == spec->MiningSideMax)
        {
            spec->MiningSideCount = 0;
            spec->MiningStatus = BEGIN_WALL_DROP_DOWN;
        }
        break;
    case BEGIN_WALL_DROP_DOWN:
        vecAddTo(spec->formation_position,spec->formation_down);
        spec->mineaistate = MINE_DROP_FORMATION;
         MinelayerCorvetteFire(ship,target);
        spec->MineDropNumber++;
        spec->formation_number_Y--;
        spec->MiningSideCount++;
        if(spec->MiningSideCount == spec->MiningSideMax)
        {
            spec->MiningSideMax++;
            spec->MiningSideCount = 0;
            spec->MiningStatus = BEGIN_WALL_DROP_RIGHT;
        }
        break;
    case DONE_WAIT:
        spec->mineaistate = MINE_DROP_ATTACK;
        return TRUE;
        break;
    }
    if(spec->MineDropNumber == minelayercorvettestatics->NumMinesInSide*minelayercorvettestatics->NumMinesInSide)
    {
        if(spec->mineforminfo != NULL)
        {
            spec->mineforminfo->FULL = TRUE;
            spec->mineforminfo = NULL;
        }
        spec->mineaistate=MINE_DROP_ATTACK;
        spec->MiningStatus = DONE_WAIT;
        spec->MineDropNumber = 0;
        return(TRUE);                               //finished Wall
    }

    return(FALSE);
}


bool MinelayerCorvetteSpecialActivate(Ship *ship)
{
    return(MinelayerCorvetteStaticMineDrop(ship,NULL));
}

void MineLayerCorvetteDied(Ship *ship)
{
    MinelayerCorvetteStatics *minelayercorvettestatics;
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    minelayercorvettestatics = (MinelayerCorvetteStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(spec->mineforminfo != NULL)
    {
        spec->mineforminfo->FULL = TRUE;
        spec->mineforminfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MinelayerCorvetteHousekeep
    Description : Regenerates mines in Minlayer corvette
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MinelayerCorvetteHousekeep(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
    MinelayerCorvetteStatics *mstat = (MinelayerCorvetteStatics *)shipstaticinfo->custstatinfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;

    if ((universe.totaltimeelapsed - spec->lasttimeRegeneratedMines) > mstat->mineRegenerateTime)
    {
        spec->lasttimeRegeneratedMines = universe.totaltimeelapsed;
        numGuns = ship->gunInfo->numGuns;

        // find missile launcher with least missiles, and give it a missile
        for (i=0,minIndex=-1,minMissiles=100000;i<numGuns;i++)
        {
            gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
            if (gunstatic->guntype == GUN_MineLauncher)
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

/*-----------------------------------------------------------------------------
    Name        : univUpdateMineWallFormations
    Description : updates the wall formations (mainly for there effect...right now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define PULSE_SPEED 2000.0f
#define PULSE_LENGTH 45.0f
#define MINE_GONE_TOO_FAR 4000.0f
#define MINE_GONE_TOO_FAR_SQR 16000000.0f

void univUpdateMineWallFormations()
{
    Node *node,*minenode,*tempnode;
    //Missile *mine, *mineend = NULL, *minestart, *mineendtemp;
    MineFormationInfo *mineformationinfo;
    Missile *minestart;
    vector tempvec;
    node = universe.MineFormationList.head;
    while(node != NULL)
    {
        mineformationinfo = (MineFormationInfo *) listGetStructOfNode(node);

        if(mineformationinfo->MineList.num == 9)             //use another number other than 0 later..maybe 8? Tunable
        {
            //no more mines in formation so destroy this here formation

            //destroy any effects that are still going

            //here re-task mines (make not Force Dropped...)
            if(mineformationinfo->FULL)
            {
                //no more mines are going to be added...

                minenode = mineformationinfo->MineList.head;
                //set formationinfo to NULL
                while(minenode != NULL)
                {
                    minestart = (Missile *) listGetStructOfNode(minenode);

                    minestart->formationinfo = NULL;
                    minenode = minenode->next;
                }
                listRemoveAll(&mineformationinfo->MineList);    //removes all mine links
                tempnode = node->next;
                listDeleteNode(node);       //destroys mineformation
                node = tempnode;
                continue;
            }
        }
        else
        {
            //mines exist in formation
            if(mineformationinfo->FULL)
            {

                minenode = mineformationinfo->MineList.head;

                while(minenode != NULL)
                {
                    minestart = (Missile *) listGetStructOfNode(minenode);

                    vecSub(tempvec, minestart->posinfo.position, minestart->formation_position);
                    if(vecMagnitudeSquared(tempvec) > MINE_GONE_TOO_FAR_SQR)
                    {
                        tempnode = minenode->next;
                        minestart->formationinfo = NULL;
                        listRemoveNode(&minestart->formationLink);
                        minenode = tempnode;
                        continue;
                    }
                    minenode = minenode->next;
                }
            }
        }

        node = node->next;
    }

}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void MineLayerCorvette_PreFix(Ship *ship)
{
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;

    spec->mineforminfo = ConvertPointerInListToNum(&universe.MineFormationList,spec->mineforminfo);
}

void MineLayerCorvette_Fix(Ship *ship)
{
    MinelayerCorvetteSpec *spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;

    spec->mineforminfo = ConvertNumToPointerInList(&universe.MineFormationList,(sdword)spec->mineforminfo);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

CustShipHeader MinelayerCorvetteHeader =
{
    MinelayerCorvette,
    sizeof(MinelayerCorvetteSpec),
    MinelayerCorvetteStaticInit,
    NULL,
    MinelayerCorvetteInit,
    NULL,
    MinelayerCorvetteAttack,
    MinelayerCorvetteFire,
    NULL,
    MinelayerCorvetteSpecialActivate,
    NULL,
    MinelayerCorvetteHousekeep,
    NULL,
    MineLayerCorvetteDied,
    MineLayerCorvette_PreFix,
    NULL,
    NULL,
    MineLayerCorvette_Fix
};

