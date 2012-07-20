/*=============================================================================
    Name    : Gun.c
    Purpose : Handles gun firing

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "glinc.h"
#include "Types.h"
#include "FastMath.h"
#include "Debug.h"
#include "LinkedList.h"
#include "Memory.h"
#include "SpaceObj.h"
#include "Physics.h"
#include "Vector.h"
#include "Matrix.h"
#include "Universe.h"
#include "SoundEvent.h"
#include "UnivUpdate.h"
#include "prim3d.h"
#include "render.h"
#include "Tweak.h"
#include "AIShip.h"
#include "Gun.h"
#include "Ships.h"
#include "Collision.h"
#include "Tactics.h"
#include "MadLinkInDefs.h"
#include "MadLinkIn.h"
#include "Randy.h"
#include "ETG.h"

#include "mainrgn.h"
#include "MEX.h"
#include "NIS.h"

#define ENABLE_SHIPRECOIL       0

#define GUNBURST_STATE_READY        0
#define GUNBURST_STATE_BURST        1
#define GUNBURST_STATE_COOLOFF      2

/*=============================================================================
    Data:
=============================================================================*/
real32 gunRecoilTable[GUN_RecoilTableLength + 1];

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : gunCanShoot
    Description : returns TRUE if gun can shoot
    Inputs      : gun
    Outputs     :
    Return      : returns TRUE if gun can shoot
----------------------------------------------------------------------------*/
bool gunCanShoot(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);

    if(ship->staticinfo->maxfuel != 0.0f)
    {
        //ship has fuel///lkets check it
        if(ship->fuel <= 0.0f)
        {
            //ship is out of fuel
            if(ship->shiptype != MinelayerCorvette)
            {
                //out of fuel and not minelayer
                //so shouldn't shoot guns...out of energy
                return FALSE;
            }
        }
    }
    if (gunstatic->burstFireTime == 0.0f)  // this gun doesn't fire in bursts
    {
        if(command != NULL)
        {
            if(command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING &&
               !ship->staticinfo->passiveAttackPenaltyExempt)
            {
                //ship is passive attacking, so lets limit its gun fire time
                if(command->ordertype.attributes & COMMAND_IS_PROTECTING &&
                   command->ordertype.attributes & COMMAND_IS_FORMATION &&
                   command->formation.formationtype == SPHERE_FORMATION)
                {
                    goto dontdospecialpassivefix;
                }
                {
                    real32 firetime = gunstatic->firetime+tacticsInfo.PassiveRetaliationFireTimeModifierLow;
                    real32 compare = (universe.totaltimeelapsed - gun->lasttimefired);
                    if(compare > firetime)
                    {
                        //pass minimum requirement
                        firetime += frandombetween(tacticsInfo.PassiveRetaliationFireTimeModifierLow,tacticsInfo.PassiveRetaliationFireTimeModifierHi);
                        if(compare > firetime)
                        {
                            //gun can now fire
                            return TRUE;
                        }
                    }
                    //it isn't ready to fire
                    return FALSE;
                }
            }
        }
dontdospecialpassivefix:
        return ((universe.totaltimeelapsed - gun->lasttimefired) > gunstatic->firetime);
    }
    else                                   // this gun fires in bursts
    {
        switch (gun->burstState)
        {
            case GUNBURST_STATE_READY:
readystate:
                gun->burstStartTime = universe.totaltimeelapsed;
                gun->burstState = GUNBURST_STATE_BURST;
                gun->useBurstFireTime = gunstatic->burstFireTime * tacticsInfo.tacticsBurstFireTimeModifier[ship->tacticstype];
                gun->useBurstWaitTime = gunstatic->burstWaitTime * tacticsInfo.tacticsBurstWaitTimeModifier[ship->tacticstype];
                if (ship->formationcommand && ship->formationcommand->formation.formationtype == SPHERE_FORMATION && ship->tacticstype != Evasive && ship->formationcommand->ordertype.order == COMMAND_ATTACK)
                {
                    gun->useBurstFireTime *= tacticsInfo.tacticsBurstFireTimeSphereModifier;
                    gun->useBurstWaitTime *= tacticsInfo.tacticsBurstWaitTimeSphereModifier;
                }
                else if (ship->attackvars.myLeaderIs)
                {
                    gun->useBurstFireTime *= tacticsInfo.tacticsBurstFireTimeWingmanModifier;
                    gun->useBurstWaitTime *= tacticsInfo.tacticsBurstWaitTimeWingmanModifier;
                }
//                soundEventBurstFire(ship, gun);
                break;

            case GUNBURST_STATE_BURST:
                if ((universe.totaltimeelapsed - gun->burstStartTime) > gun->useBurstFireTime)
                {
                    gun->burstState = GUNBURST_STATE_COOLOFF;
                    gun->burstStartTime = universe.totaltimeelapsed;
                    // fall through to GUNBURST_STATE_COOLOFF
                    soundEventBurstStop(ship, gun);
                }
                else
                {
                    break;
                }

            case GUNBURST_STATE_COOLOFF:
                if ((universe.totaltimeelapsed - gun->burstStartTime) > gun->useBurstWaitTime)
                {
                    gun->burstState = GUNBURST_STATE_READY;
                    goto readystate;
                }
                else
                {
                    return FALSE;
                }
        }

        if(command != NULL)
        {
            if(command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING
                && !ship->staticinfo->passiveAttackPenaltyExempt)
            {
                //ship is passive attacking, so lets limit its gun fire time
                if(command->ordertype.attributes & COMMAND_IS_PROTECTING &&
                   command->ordertype.attributes & COMMAND_IS_FORMATION &&
                   command->formation.formationtype == SPHERE_FORMATION)
                {
                    goto dontdospecialpassivefix2;
                }

                {
                    real32 firetime = gunstatic->firetime+tacticsInfo.PassiveRetaliationFireTimeModifierLow;
                    real32 compare = (universe.totaltimeelapsed - gun->lasttimefired);
                    if(compare > firetime)
                    {
                        //pass minimum requirement
                        firetime += frandombetween(tacticsInfo.PassiveRetaliationFireTimeModifierLow,tacticsInfo.PassiveRetaliationFireTimeModifierHi);
                        if(compare > firetime)
                        {
                            //gun can now fire
                            return TRUE;
                        }
                    }
                    //it isn't ready to fire
                    return FALSE;
                }
            }
        }
dontdospecialpassivefix2:
        return ((universe.totaltimeelapsed - gun->lasttimefired) > gunstatic->firetime);
    }
}

/*-----------------------------------------------------------------------------
    Name        : gunNewGimbleUpdateCoordSys
    Description : update a New_Gimble gun's coordinate system to reflect gun->angle, gun->declination
    Inputs      : gun, gunstatic
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunNewGimbleUpdateCoordSys(Gun *gun,GunStatic *gunstatic)
{
    matrix newmat,rotxmatrix,rotymatrix;

    matMakeRotAboutY(&rotymatrix,(real32)cos(gun->declination),(real32)sin(gun->declination));
    matMakeRotAboutX(&rotxmatrix,(real32)cos(gun->angle),(real32)sin(gun->angle));

    matMultiplyMatByMat(&newmat,&gunstatic->defaultgunorientation,&rotxmatrix);
    matMultiplyMatByMat(&gun->curgunorientation,&newmat,&rotymatrix);
    if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
    {
        matMultiplyMatByMat(&newmat,&gunstatic->defaultGunOrientationNonConcat,&rotymatrix);
        matMultiplyMatByMat(&gun->curGunOrientationNonConcat,&newmat,&rotxmatrix);
    }

    matGetVectFromMatrixCol3(gun->gunheading,gun->curgunorientation);
}

/*-----------------------------------------------------------------------------
    Name        : gunUpdateSlave
    Description : Update a slave gun's status from it's slave driver
    Inputs      : gun - slave gun
                  guns - list of gunst
                  iSlaveDriver - index of slaveDriver gun
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunUpdateSlave(Gun *gun, GunInfo *guns, sdword iSlaveDriver)
{
    Gun *slaveDriver;

    dbgAssert(iSlaveDriver >= 0 && iSlaveDriver < guns->numGuns);
    slaveDriver = &guns->guns[iSlaveDriver];
    gun->gunheading = slaveDriver->gunheading;              //copy the essentials from the slave driver
    gun->curgunorientation = slaveDriver->curgunorientation;
    gun->declination = slaveDriver->declination;
    gun->angle = slaveDriver->angle;
}

/*-----------------------------------------------------------------------------
    Name        : gunOrientGimbleGun
    Description : if possible, orients the gimble gun to point at target
    Inputs      : gun,target
    Outputs     :
    Return      : TRUE if gun oriented as desired
----------------------------------------------------------------------------*/
bool gunOrientGimbleGun(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target)
{
    GunStatic *gunstatic = gun->gunstatic;
    vector targetInWorldCoordSys;
    vector targetInShipCoordSys;
    vector targetInGunCoordSys;
    real32 dotprod,bulletspeed;
    real32 triggerHappy, maxAngle, maxAngleSpeed;

    real32 tryangle;
    real32 turndeclination;
    real32 desireddeclinationspeed;
    real32 turnangle;
    real32 desiredanglespeed;

    bool updateguncoordsys;

    if (bitTest(ship->flags, SOF_NISShip))
    {
        triggerHappy = NIS_TriggerHappyAngle;
        maxAngle = NIS_TriggerHappyAngle;
        maxAngleSpeed = gunstatic->maxanglespeed;
    }
    else
    {
        triggerHappy = gunstatic->triggerHappy;
        maxAngle = gunstatic->cosmaxAngleFromNorm;
        maxAngleSpeed = gunstatic->maxanglespeed;
        if ((gunstatic->bulletType == BULLET_Beam) && (target->objtype == OBJ_ShipType))
        {
            ShipClass sclass = ((Ship *)target)->staticinfo->shipclass;
            if (sclass == CLASS_Fighter)
            {
                maxAngle = IONCANNON_TARGETS_FIGHTER_ANGLE;
                maxAngleSpeed = IONCANNON_TARGETS_FIGHTER_MAXANGLESPEED;
                triggerHappy = IONCANNON_TARGETS_FIGHTER_TRIGGERHAPPY;
            }
            else if (sclass == CLASS_Corvette)
            {
                maxAngle = IONCANNON_TARGETS_CORVETTE_ANGLE;
                maxAngleSpeed = IONCANNON_TARGETS_CORVETTE_MAXANGLESPEED;
                triggerHappy = IONCANNON_TARGETS_CORVETTE_TRIGGERHAPPY;
            }
        }
    }
    if(ship->staticinfo->shipclass == CLASS_Fighter)
    {
        bulletspeed = gunstatic->bulletspeed*tacticsInfo.BulletSpeedBonus[Tactics_Fighter][ship->tacticstype];
    }
    else if(ship->staticinfo->shipclass == CLASS_Corvette)
    {
        bulletspeed = gunstatic->bulletspeed*tacticsInfo.BulletSpeedBonus[Tactics_Corvette][ship->tacticstype];
    }
    else
    {
        bulletspeed = gunstatic->bulletspeed;
    }


    if (gunstatic->guntype == GUN_Gimble)
    {
        aishipGetTrajectoryWithVelPrediction(ship,target,bulletspeed,&targetInWorldCoordSys);
//        vecSub(targetInWorldCoordSys,target->posinfo.position,ship->posinfo.position);

        matMultiplyVecByMat(&targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);

        vecSubFrom(targetInShipCoordSys,gunstatic->position);

        vecNormalize(&targetInShipCoordSys);

        dotprod = vecDotProduct(targetInShipCoordSys,gunstatic->gunnormal);
        if (dotprod > 1.0f)
        {
            dotprod = 1.0f;     // do this because vecNormalize does not normalize perfectly
        }
        if (isBetweenInclusive(dotprod,maxAngle,gunstatic->cosminAngleFromNorm))
        {
            gun->gunheading = targetInShipCoordSys;

            soundEventStop(gun->gimblehandle);
            gun->gimblehandle = -1;

            return TRUE;
        }

        if (gun->gimblehandle < 0)
        {
            gun->gimblehandle = soundEventPlay(ship, Gun_WeaponMove, gun);
        }

        return FALSE;
    }
    else if (gunstatic->guntype == GUN_NewGimble)
    {
        if (gunstatic->slaveDriver >= 0)
        {                                                   //if this gun has a slavedriver
            gunUpdateSlave(gun, ship->gunInfo, gunstatic->slaveDriver);
        }

        aishipGetTrajectoryWithVelPrediction(ship,target,bulletspeed,&targetInWorldCoordSys);
//        vecSub(targetInWorldCoordSys,target->posinfo.position,ship->posinfo.position);

        matMultiplyVecByMat(&targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);

        // we have targetInShipCoordSys.  Now change to gun's coordinate system

        vecSubFrom(targetInShipCoordSys,gunstatic->position);

        vecNormalize(&targetInShipCoordSys);

        matMultiplyVecByMat(&targetInGunCoordSys,&targetInShipCoordSys,&gun->curgunorientation);

        dotprod = targetInGunCoordSys.z;        // (0,0,1) dot (x,y,z) == z

        gun->anglespeed = 0.0f;
        gun->declinationspeed = 0.0f;

        if ((dotprod >= triggerHappy) && (gunstatic->bulletType != BULLET_Beam))
        {
            goto onTargetExitEarly;
        }

        updateguncoordsys = FALSE;

        if (gunstatic->slaveDriver < 0)
        {                                                   //only do updates for free guns
            // always want target to be on positive z axis in gun coordinate system (i.e. gun barrel on z should point to target)
            if (targetInGunCoordSys.y != 0)
            {
                desiredanglespeed = -targetInGunCoordSys.y * gunstatic->angletracking;
                if (desiredanglespeed > maxAngleSpeed)
                {
                    desiredanglespeed = maxAngleSpeed;
                }
                else if (desiredanglespeed < -maxAngleSpeed)
                {
                    desiredanglespeed = -maxAngleSpeed;
                }

                turnangle = desiredanglespeed * universe.phystimeelapsed;
                tryangle = turnangle + gun->angle;
                if ((tryangle < gunstatic->maxturnangle) && (tryangle > gunstatic->minturnangle))
                {
                    gun->angle = tryangle;
                    gun->anglespeed = desiredanglespeed;
                    updateguncoordsys = TRUE;
                }

            }

            if (targetInGunCoordSys.x != 0)
            {
                desireddeclinationspeed = targetInGunCoordSys.x * gunstatic->declinationtracking;
                if (desireddeclinationspeed > maxAngleSpeed)
                {
                    desireddeclinationspeed = maxAngleSpeed;
                }
                else if (desireddeclinationspeed < -maxAngleSpeed)
                {
                    desireddeclinationspeed = -maxAngleSpeed;
                }

                turndeclination = desireddeclinationspeed * universe.phystimeelapsed;
                tryangle = turndeclination + gun->declination;
                if ((tryangle < gunstatic->maxdeclination) && (tryangle > gunstatic->mindeclination))
                {
                    gun->declination = tryangle;
                    gun->declinationspeed = desireddeclinationspeed;
                    updateguncoordsys = TRUE;
                }
            }

            if (updateguncoordsys)
            {
                gunNewGimbleUpdateCoordSys(gun,gunstatic);
            }
        }

        if (dotprod >= triggerHappy)
        {
onTargetExitEarly:
            gun->gunheading = targetInShipCoordSys;

            soundEventStop(gun->gimblehandle);
            gun->gimblehandle = -1;

            return TRUE;
        }
        else
        {
            if (gun->gimblehandle < 0)
            {
                gun->gimblehandle = soundEventPlay(ship, Gun_WeaponMove, gun);
            }

            return FALSE;
        }
    }
    else
    {
        dbgAssert(FALSE);
        return FALSE;
    }
    return(FALSE);
}

void gunGetGunPositionInWorld(vector *positionInWorldCoordSys,matrix *coordsys,Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    matMultiplyMatByVec(positionInWorldCoordSys,coordsys,&gunstatic->position);
}

/*-----------------------------------------------------------------------------
    Name        : missileShoot
    Description : shoots a missile
    Inputs      : ship which owns gun, gun to fire missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void missileShoot(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target)
{
    Missile *missile;
    GunStatic *gunstatic = gun->gunstatic;
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    etgeffectstatic *stat;
    etglod *etgLOD;
    sdword LOD,i;
    real32 floatDamage;
    udword intDamage;
    ShipRace shiprace = ship->shiprace;
    matrix newCoordsys;

    MinelayerCorvetteSpec *spec;

    vector gunheadingInWorldCoordSys;
    vector positionInWorldCoordSys,mineup,mineright,minehead;

#if ENABLE_SHIPRECOIL
    vector recoil;
    real32 massovertime;
#endif

    dbgAssert(gunstatic->guntype == GUN_MissileLauncher || gunstatic->guntype == GUN_MineLauncher);
    dbgAssert(gunHasMissiles(gun));

    //line-of-sight check
    if (target != NULL && gunstatic->guntype == GUN_MissileLauncher)
    {
        vector gunDirection, gunPosition;

        matMultiplyMatByVec(&gunDirection, &ship->rotinfo.coordsys, &gun->gunheading);
        matMultiplyMatByVec(&gunPosition, &ship->rotinfo.coordsys, &gunstatic->position);
        vecAddTo(gunPosition, ship->posinfo.position);
        if (!collCheckLineOfSight(ship, (Ship*)target, &gunPosition, &gunDirection))
        {
#if GUN_VERBOSE_LEVEL > 1
            dbgMessage("\nline-of-sight obscured, not shooting");
#endif
            return;
        }
    }

    gun->numMissiles--;
    gun->lasttimefired = universe.totaltimeelapsed;

    missile = memAlloc(sizeof(Missile),"Missile",0);
    memset(missile,0,sizeof(Missile));

    missile->objtype = OBJ_MissileType;
    missile->flags = SOF_Rotatable + SOF_Impactable + SOF_NotBumpable + SOF_Targetable;

    missile->enginePosition.x = 0.0f;
    missile->enginePosition.y = 0.0f;
    missile->enginePosition.z = 0.0f;
    missile->trail = NULL;

    if(gunstatic->guntype == GUN_MissileLauncher)
    {   //specific definitions for MISSILES
        missile->staticinfo = &missileStaticInfos[shiprace];
        missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.phystimeelapsed;
        missile->missileType = MISSILE_Regular;
        missile->FORCE_DROPPED = FALSE;
        missile->formationinfo = NULL;
        missile->trail = mistrailNew(missile->staticinfo->trailStatic, missile);
    }   //specific definitions for MINES
    else
    {   //must be a mine launcher
        missile->staticinfo = &mineStaticInfos[shiprace];
        dbgAssert(ship->shiptype == MinelayerCorvette);
        spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
        missile->missileType = MISSILE_Mine;
        missile->mineAIState = spec->mineaistate;       //initialize the Mines AI state
        if(spec->mineaistate == MINE_DROP_FORMATION)
        {
            listAddNode(&spec->mineforminfo->MineList,&(missile->formationLink),missile);
            missile->formationinfo = spec->mineforminfo;
            missile->formation_position = spec->formation_position;         //only way to communicate this info
            missile->formation_number_X = spec->formation_number_X;
            missile->formation_number_Y = spec->formation_number_Y;
            missile->maxtraveldist = ((MissileStaticInfo *)missile->staticinfo)->maxvelocity_FORCED * universe.phystimeelapsed;
            missile->FORCE_DROPPED = TRUE;                                  //set flag
        }
        else
        {
            missile->FORCE_DROPPED = FALSE;                                  //set flag
            missile->formationinfo = NULL;
            missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.phystimeelapsed;
        }
    }
    missile->haveCheckedForLoss = 0;    //init target loss variable

    missile->colorScheme = ship->colorScheme;
    missile->health = ((MissileStaticInfo *)missile->staticinfo)->maxhealth;

    ClearNode(missile->renderlink);
    missile->currentLOD = ship->currentLOD;
    missile->collMyBlob = NULL;
    missile->cameraDistanceSquared = ship->cameraDistanceSquared;
    missile->flashtimer = 0;
    missile->lasttimecollided = 0;
    missile->attributes = 0;
    missile->attributesParam = 0;

    vecZeroVector(missile->posinfo.force);
    vecZeroVector(missile->rotinfo.torque);

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.thruststrength[i] = missile->staticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.rotstrength[i] = missile->staticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
       missile->nonstatvars.turnspeed[i] = missile->staticinfo->turnspeedstat[i];
    }


    missile->soundType = gunstatic->gunsoundtype;
    missile->race = shiprace;
    missile->timelived = 0.0f;
    missile->playerowner = ship->playerowner;
    missile->owner = ship;
    missile->target = target;

    if(missile->FORCE_DROPPED == TRUE)
    {   //force dropped mine only!
        MinelayerCorvetteStatics *staticinfo = (MinelayerCorvetteStatics *) ship->staticinfo->custstatinfo;
        missile->damage = (frandombetween(staticinfo->forced_drop_damage_lo,staticinfo->forced_drop_damage_hi));
        missile->totallifetime = staticinfo->forced_drop_lifetime;
    }
    else
    {   //normal
        missile->damage = (frandombetween(gunstatic->gunDamageLo[ship->tacticstype],gunstatic->gunDamageHi[ship->tacticstype]));
        missile->totallifetime = gunstatic->bulletlifetime;
    }

    //test to see if needs damage modifier
    if(bitTest(ship->specialFlags,SPECIAL_KasSpecialDamageModifier))
    {
        //ship is being 'beefed' up by KAS...add value to damagemultiplier
        missile->damage*=ship->damageModifier;
    }

    missile->missileID.missileNumber = universe.missileNumber;
    if (multiPlayerGame) IDToPtrTableAdd(&MissileIDToPtr,universe.missileNumber,(SpaceObj *)missile);
    universe.missileNumber++;

    // now fill in missile->posinfo

    // heading
    matMultiplyMatByVec(&gunheadingInWorldCoordSys,&ship->rotinfo.coordsys,&gun->gunheading);

    // position
    matMultiplyMatByVec(&positionInWorldCoordSys,&ship->rotinfo.coordsys,&gunstatic->position);

    vecAdd(missile->posinfo.position,positionInWorldCoordSys,ship->posinfo.position);
    missile->posinfo.position.x += gunheadingInWorldCoordSys.x * gunstatic->barrelLength;
    missile->posinfo.position.y += gunheadingInWorldCoordSys.y * gunstatic->barrelLength;
    missile->posinfo.position.z += gunheadingInWorldCoordSys.z * gunstatic->barrelLength;

    missile->posinfo.isMoving = TRUE;
    missile->posinfo.haventCalculatedDist = TRUE;

    vecScalarMultiply(missile->posinfo.velocity,gunheadingInWorldCoordSys,gunstatic->bulletspeed);

    // now fill in missile->rotinfo
    if (gunstatic->guntype == GUN_MineLauncher)
    {   //mine launcher so fix mine rotation depending on gun launched from...and race?
        //hard coded fix to a special exception :)

        if(ship->gunInfo->numGuns > 1)
        {    //race 1 ship..do rotation..later make global LOD on this..since its a waste of time..really
            //build mine coordinate system from the ships...BIG savings...
            if(((Gun *) &(ship->gunInfo->guns[0])) == gun)
            {
                matGetVectFromMatrixCol1(mineright,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(mineup,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
                vecScalarMultiply(mineup,mineup,-1.0f);
            }
            else
            {
                matGetVectFromMatrixCol1(mineright,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(mineup,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
                vecScalarMultiply(mineright,mineright, -1.0f);
            }
            matPutVectIntoMatrixCol1(mineup,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol2(mineright,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol3(minehead,missile->rotinfo.coordsys);
        }
        else
        {
            // LM: what is this stranded code doing?
            matGetVectFromMatrixCol1(mineup,ship->rotinfo.coordsys);
            matGetVectFromMatrixCol2(mineright,ship->rotinfo.coordsys);
            matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
            matPutVectIntoMatrixCol1(mineup,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol2(mineright,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol3(minehead,missile->rotinfo.coordsys);
            //            matCreateCoordSysFromHeading(&missile->rotinfo.coordsys,&gunheadingInWorldCoordSys);
        }
    }
    else
    {
        matCreateCoordSysFromHeading(&missile->rotinfo.coordsys,&gunheadingInWorldCoordSys);
    }

    univUpdateObjRotInfo((SpaceObjRot *)missile);

    if ((target != NULL) && (target->collMyBlob != NULL))
    {
        collAddSpaceObjToSpecificBlob(target->collMyBlob,(SpaceObj *)missile);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)missile);
    }
    listAddNode(&universe.SpaceObjList,&(missile->objlink),missile);
    listAddNode(&universe.ImpactableList,&(missile->impactablelink),missile);
    listAddNode(&universe.MissileList,&(missile->missilelink),missile);

    univAddObjToRenderListIf((SpaceObj *)missile,(SpaceObj *)ship);     // add missile to render list if parent ship is in render list

//    if (gunstatic->burstFireTime == 0.0f)
    {
        soundEventPlay(ship, Gun_WeaponShot, gun);
    }

    //muzle flash effect trigger and set collision event
    missile->hitEffect = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunHit];//get pointer to bullet effect
//    missile->hitEffect = etgGunEventTable[shiprace][gunstatic->gunsoundtype][EGT_GunHit];//just get pointer from parent gun
    etgLOD = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get pointer to bullet effect
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
//    stat = etgGunEventTable[shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get gun effect

    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgFireEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgFireEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {                                                       //if there is a gun fire effect
            matCreateCoordSysFromHeading(&newCoordsys,&gunheadingInWorldCoordSys);
            floatDamage = (real32)missile->damage;
            if (RGLtype == SWtype)
            {                                                   //smaller muzzle flashes in software
                floatDamage *= etgSoftwareScalarFire;
            }
            floatDamage *= ship->magnitudeSquared;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, ship, &missile->posinfo.position, NULL, &newCoordsys, 1.0f, EAF_Velocity | EAF_NLips, 1, intDamage);
        }
    }
    if(bitTest(ship->flags,SOF_CloakGenField))
    {   //dang decloakage...
        bitSet(ship->flags,SOF_DeCloaking);
        if(bitTest(ship->flags,SOF_Cloaked))
        {
            bitClear(ship->flags,SOF_Cloaked);
            ship->shipDeCloakTime = universe.totaltimeelapsed;
        }
        bitClear(ship->flags,SOF_Cloaking);
    }
    if (target != NULL && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
}


/*-----------------------------------------------------------------------------
    Name        : gunRecoilTableCompute
    Description : Computes and fills in the gun recoil motion table
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunRecoilTableCompute(void)
{
    sdword index;
    real32 timeIndex, value;

    for (index = 0, timeIndex = 0.0f; timeIndex < GUN_RecoilTime;
         index++, timeIndex += 1.0f / (real32)GUN_RecoilTableLength)
    {
        //y = 1 - ((x - .05) * 20)^2
        value = (timeIndex - GUN_RecoilTime) * GUN_OneOverTime;
        gunRecoilTable[index] = 1.0f - (value * value);
    }
    for (; index < GUN_RecoilTableLength; index++, timeIndex += 1.0f / (real32)GUN_RecoilTableLength)
    {
        //y = ((-atan((x-.4)*8)/PI+.5-.06538)/.825433
        gunRecoilTable[index] = (((real32)-atan((timeIndex-0.4f)*8.0f)/PI+0.5f)-0.06538f)/0.825433f;
    }
    gunRecoilTable[GUN_RecoilTableLength] = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : gunRecoilVectorCompute
    Description : Compute a vector representing a gun's recoil as a function
                    of time between shots.
    Inputs      : dest - heading of gun mesh
                  recoilLength - length of the gun's recoil, in meters
                  lastFired - absolute time of the last gun shot
                  rateOfFire - time between shots
    Outputs     : dest - recoil vector to be added to position
    Return      :
----------------------------------------------------------------------------*/
void gunRecoilVectorCompute(vector *dest, real32 recoilLength, real32 lastFired, real32 rateOfFire)
{
    real32 timeElapsed, offset;
    sdword index;

    timeElapsed = universe.totaltimeelapsed - lastFired;
    if (timeElapsed > rateOfFire)
    {                                                       //if the gun is sitting idle
        dest->x = dest->y = dest->z = 0.0f;                 //no recoil at all
        return;
    }
    timeElapsed /= rateOfFire;                              //normalize time elapsed from 0 to 1

    index = (sdword)(timeElapsed * (real32)GUN_RecoilTableLength);
    offset = gunRecoilTable[index];                         //table-lookup for gun recoil curve
    offset *= -recoilLength;
    dest->x *= offset;
    dest->y *= offset;
    dest->z *= offset;
}

/*-----------------------------------------------------------------------------
    Name        : gunShoot
    Description : shoots a given gun of the given ship
    Inputs      : ship  - that which owns gun
                  gun - gun to fire
                  target - object being shot at or NULL
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunShoot(Ship *ship, Gun *gun, SpaceObjRotImpTarg *target)
{
    Bullet *bullet;
    GunStatic *gunstatic = gun->gunstatic;
    ShipStaticInfo *shipstatic;
    etgeffectstatic *stat;
    etglod *etgLOD;
    sdword LOD;
    real32 floatDamage,bulletspeed,damagemult,healthFactor;
    udword intDamage, intVelocity, intLength;
    vector offset0, offset1;
    CommandToDo *shipFormationCommand;
    vector gunheadingInWorldCoordSys;
    vector positionInWorldCoordSys;

    vector gunDirection, gunPosition;

#if ENABLE_SHIPRECOIL
    vector recoil;
    real32 massovertime;
#endif

    dbgAssert(gunstatic->guntype != GUN_MissileLauncher);

    //line-of-sight check
    if (target != NULL)
    {
        switch (((ShipStaticInfo*)ship->staticinfo)->shipclass)
        {
        case CLASS_Mothership:
        case CLASS_HeavyCruiser:
        case CLASS_Destroyer:
        case CLASS_Carrier:
        case CLASS_Frigate:
            matMultiplyMatByVec(&gunDirection, &ship->rotinfo.coordsys, &gun->gunheading);
            matMultiplyMatByVec(&gunPosition, &ship->rotinfo.coordsys, &gunstatic->position);
            vecAddTo(gunPosition, ship->posinfo.position);
            if (!collCheckLineOfSight(ship, (Ship*)target, &gunPosition, &gunDirection))
            {
#if GUN_VERBOSE_LEVEL > 1
                dbgMessage("\nline-of-sight obscured, not shooting");
#endif
                return;
            }
        }
    }

    //if this is the first gunshot, set the shipisattacking variable
    //for the ship and his wingman as well (if the wingman exists)
    if (!ship->shipisattacking)
    {
        ship->shipisattacking = TRUE;
        //dbgMessagef("\nShip %i is attacking (gun)", ship->shipID.shipNumber);
    }
    if ((ship->attackvars.myWingmanIs)&&(!ship->attackvars.myWingmanIs->shipisattacking))
    {
        ship->attackvars.myWingmanIs->shipisattacking = TRUE;
        //dbgMessagef("\nShip %i is attacking (gun - wingman)", ship->attackvars.myWingmanIs->shipID.shipNumber);
    }
    if(ship->specialFlags2 & SPECIAL_2_ShipInGravwell)
    {
        //ship is in gravwell
        return; //don't let it fire.
    }


    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    gun->lasttimefired = universe.totaltimeelapsed;

    bullet = memAlloc(sizeof(Bullet),"Bu(Bullet)",Pyrophoric);

    bullet->objtype = OBJ_BulletType;
    bullet->flags = 0;
    bullet->staticinfo = NULL;
    ClearNode(bullet->renderlink);
    bullet->currentLOD = ship->currentLOD;
    bullet->collMyBlob = NULL;
    bullet->cameraDistanceSquared = ship->cameraDistanceSquared;


    //calculate a damage multipler based on several variables
    //including strike craft being in formation, and their damage
    switch(ship->staticinfo->shipclass)
    {
    case CLASS_Fighter:
    case CLASS_Corvette:
        //optimize 'healthfactor' for all game operations
        //by calculating it once a frame?
        healthFactor = ship->health*ship->staticinfo->oneOverMaxHealth;
        if (healthFactor < selShipHealthYellowFactor)
        {
            //in the red
            damagemult = tacticsInfo.tacticsDamageModifierRed;
        }
        else if (healthFactor < selShipHealthGreenFactor )
        {
            //in the yellow
            damagemult = tacticsInfo.tacticsDamageModifierYellow;
        }
        else
        {
            //in the green
            damagemult = 1.0f;
        }

        shipFormationCommand = ship->formationcommand;
        if(shipFormationCommand != NULL)
        {
            //in formation..concatenate the damage multipler from above with this new one
            damagemult *= (shipFormationCommand->formation.formationtype == SPHERE_FORMATION) ?
                tacticsInfo.tacticsDamageMultiplierInSphereFormation : tacticsInfo.tacticsDamageMultiplierInFormation;
            break;
        }
        break;
    default:
        damagemult = 1.0f;
        break;
    }

    if(bitTest(ship->specialFlags,SPECIAL_KasSpecialDamageModifier))
    {
        //ship is being 'beefed' up by KAS...add value to damagemultiplier
        damagemult*=ship->damageModifier;
    }
    //set gun damage based on ships tactical state.
    if(bitTest(ship->specialFlags,SPECIAL_BurstFiring))
    {
        bullet->damageFull = bullet->damage = frandombetween(burstDamageLo[ship->shiprace],burstDamageHi[ship->shiprace])*damagemult;
        bullet->bulletType = BULLET_SpecialBurst;
    }
    else
    {
        bullet->damageFull = bullet->damage = frandombetween(gunstatic->gunDamageLo[ship->tacticstype],gunstatic->gunDamageHi[ship->tacticstype])*damagemult;
        bullet->bulletType = gunstatic->bulletType;
    }

    bullet->soundType = gunstatic->gunsoundtype;

    bullet->owner = ship;
    bullet->playerowner = ship->playerowner;
    bullet->gunowner = gun;
    if ((target != NULL))// && (target->objtype != OBJ_MissileType))
    {
        if(bitTest(ship->specialFlags,SPECIAL_BurstFiring))
            bullet->target = NULL;
        else
            bullet->target = target;
    }
    else
    {
        bullet->target = NULL;
    }
    bullet->bulletColor = etgBulletColor[shipstatic->shiprace][bullet->soundType];
    bullet->bulletmass = gunstatic->bulletmass;
    bullet->lengthmag = gunstatic->bulletlength;

    bullet->timelived = 0.0f;
    if(bitTest(ship->specialFlags,SPECIAL_BurstFiring))
    {
        HeavyCorvetteSpec *spec = (HeavyCorvetteSpec *)ship->ShipSpecifics;
        bullet->totallifetime = spec->bulletLifeTime;
    }
    else
    {
        //set bullet life time based on range (and hence add tactics bonuses)
        if(ship->staticinfo->shipclass == CLASS_Fighter)
        {
            bullet->totallifetime = gunstatic->bulletlifetime*tacticsInfo.BulletRangeBonus[Tactics_Fighter][ship->tacticstype];
        }
        else if(ship->staticinfo->shipclass == CLASS_Corvette)
        {
            bullet->totallifetime = gunstatic->bulletlifetime*tacticsInfo.BulletRangeBonus[Tactics_Corvette][ship->tacticstype];
        }
        else
        {
            bullet->totallifetime = gunstatic->bulletlifetime;
        }
    }
    bullet->SpecialEffectFlag = 0;
    bullet->DFGFieldEntryTime = 0.0f;

    // now fill in bullet->posinfo

    memset(&(bullet->posinfo),0,sizeof(bullet->posinfo));

    // heading
    matMultiplyMatByVec(&gunheadingInWorldCoordSys,&ship->rotinfo.coordsys,&gun->gunheading);
    bullet->bulletheading = gunheadingInWorldCoordSys;

    // length of bullet
    vecScalarMultiply(bullet->lengthvec,gunheadingInWorldCoordSys,gunstatic->bulletlength);

    matCreateCoordSysFromHeading(&bullet->rotinfo.coordsys,&gunheadingInWorldCoordSys);

    switch (bullet->bulletType)
    {
        case BULLET_Beam:
            vecZeroVector(bullet->posinfo.velocity);
            bullet->traveldist = gunstatic->bulletlength;
            bullet->beamtraveldist = gunstatic->bulletlength;
            break;

        default:
            // velocity
            if(bitTest(ship->specialFlags,SPECIAL_BurstFiring))
            {
                //fix later
                bulletspeed = burstSpeed;
            }
            else
            {
                if(ship->staticinfo->shipclass == CLASS_Fighter)
                {
                    bulletspeed = gunstatic->bulletspeed*tacticsInfo.BulletSpeedBonus[Tactics_Fighter][ship->tacticstype];
                }
                else if(ship->staticinfo->shipclass == CLASS_Corvette)
                {
                    bulletspeed = gunstatic->bulletspeed*tacticsInfo.BulletSpeedBonus[Tactics_Corvette][ship->tacticstype];
                }
                else
                {
                    bulletspeed = gunstatic->bulletspeed;
                }
            }

            vecScalarMultiply(bullet->posinfo.velocity,gunheadingInWorldCoordSys,bulletspeed);
                // bullettravel
            bullet->traveldist = bulletspeed * universe.phystimeelapsed;
            break;
    }

#if ENABLE_SHIPRECOIL
    if (!shipstatic->staticheader.immobile)     // if ship isn't immobile, apply recoil (opposite force)
    {
        massovertime = -bullet->bulletmass / universe.phystimeelapsed;
        recoil = bullet->posinfo.velocity;
        vecMultiplyByScalar(recoil,massovertime);
        physApplyForceVectorToObj(ship,recoil);

    }
#endif

    // position
    matMultiplyMatByVec(&positionInWorldCoordSys,&ship->rotinfo.coordsys,&gunstatic->position);

    //add barrel length
    if (gunstatic->offset.x != 0.0f)
    {                                                       //if there is an offset vector
        matMultiplyMatByVec(&offset0, &gun->curgunorientation, &gunstatic->offset);//offset in gun space
        matMultiplyMatByVec(&offset1, &ship->rotinfo.coordsys, &offset0);//offset in world space
        vecAdd(positionInWorldCoordSys, positionInWorldCoordSys, offset1);//add to position
    }
    vecAdd(bullet->posinfo.position,positionInWorldCoordSys,ship->posinfo.position);
    bullet->posinfo.position.x += gunheadingInWorldCoordSys.x * gunstatic->barrelLength;
    bullet->posinfo.position.y += gunheadingInWorldCoordSys.y * gunstatic->barrelLength;
    bullet->posinfo.position.z += gunheadingInWorldCoordSys.z * gunstatic->barrelLength;

    bullet->posinfo.isMoving = TRUE;
    bullet->posinfo.haventCalculatedDist = TRUE;

    if ((target != NULL) && (target->collMyBlob != NULL))
    {
        collAddSpaceObjToSpecificBlob(target->collMyBlob,(SpaceObj *)bullet);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)bullet);
    }
    listAddNode(&universe.SpaceObjList,&(bullet->objlink),bullet);
    listAddNode(&universe.BulletList,&(bullet->bulletlink),bullet);

    univAddObjToRenderListIf((SpaceObj *)bullet,(SpaceObj *)ship);     // add to render list if parent ship is in render list

//    if (gunstatic->burstFireTime == 0.0f)
    {
        if (bullet->bulletType == BULLET_Beam)
        {
            soundEventPlay(ship, Gun_WeaponFireLooped, gun);
        }
        else
        {
            soundEventPlay(ship, Gun_WeaponShot, gun);
        }
    }

    //figure out some parameters for the effects we are about to spawn
    floatDamage = (real32)bullet->damage;
    floatDamage *= ship->magnitudeSquared;
    intDamage = TreatAsUdword(floatDamage);
    intVelocity = TreatAsUdword(bulletspeed);
    intLength = TreatAsUdword(gunstatic->bulletlength);
    //create an effect for bullet, if applicable
    if(bitTest(ship->specialFlags,SPECIAL_BurstFiring))
    {
        etgLOD = etgSpecialPurposeEffectTable[EGT_BURST_BULLET_EFFECT];
    }
    else
    {
        etgLOD = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunBullet];//get pointer to bullet effect
    }

    if (etgLOD != NULL)
    {
        if (target != NULL)
        {
            LOD = min(ship->currentLOD, target->currentLOD);
        }
        else
        {
            LOD = ship->currentLOD;
        }
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
    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgBulletEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgBulletEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {
            bullet->effect = etgEffectCreate(stat, (Ship *)bullet, NULL, &ship->posinfo.velocity, NULL, 1.0f, EAF_AllButNLips, 3, intDamage, intVelocity, intLength);
        }
        else
        {
            bullet->effect = NULL;                              //play no effect for this bullet
        }
    }
    else
    {
        bullet->effect = NULL;                              //play no effect for this bullet
    }

    //fire effect trigger and set collision event
    bullet->hitEffect = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunHit];//get pointer to bullet effect
//    bullet->hitEffect = etgGunEventTable[ship->shiprace][gunstatic->gunsoundtype][EGT_GunHit];//just get pointer from parent gun
    etgLOD = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get pointer to bullet effect
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
//    stat = etgGunEventTable[ship->shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get gun effect

    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgFireEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgFireEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {                                                       //if there is a gun fire effect
            if (RGLtype == SWtype)
            {                                                   //smaller muzzle flashes in software
                floatDamage *= etgSoftwareScalarFire;
            }
            etgEffectCreate(stat, ship, &bullet->posinfo.position, &ship->posinfo.velocity, &bullet->rotinfo.coordsys, 1.0f, EAF_Velocity, 1, intDamage);
        }
    }
    if(bitTest(ship->flags,SOF_CloakGenField))
    {
        bitSet(ship->flags,SOF_DeCloaking);
        if(bitTest(ship->flags,SOF_Cloaked))
        {
            bitClear(ship->flags,SOF_Cloaked);
            ship->shipDeCloakTime = universe.totaltimeelapsed;
        }
        bitClear(ship->flags,SOF_Cloaking);
    }
    if (target != NULL && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
}
/*-----------------------------------------------------------------------------
    Name        : gunShootGunsAtTarget
    Description : shoots a ship's guns at target
    Inputs      : ship, target, range, [trajectory] (trajectory required for ships with fixed guns)
    Outputs     :
    Return      :
    Note        : trajectory can be passed in as NULL for ships that don't have
                  any fixed guns.  trajectory is required only for ships that have
                  fixed guns.
----------------------------------------------------------------------------*/
bool gunShootGunsAtTarget(Ship *ship,SpaceObjRotImpTarg *target,real32 range,vector *trajectory)
{
    GunInfo *gunInfo = ship->gunInfo;
    sdword numGuns;
    Gun *gun;
    sdword i;
    vector shipheading;
    bool shotguns = FALSE;
    GunStatic *gunstatic;
    real32 dotprod;
    real32 bonus;
    real32 triggerHappy;

    if (gunInfo == NULL)
    {
//        dbgMessagef("\nWARNING: no guns on ship %s %s, tried to shoot",ShipRaceToStr(ship->shiprace),ShipTypeToStr(ship->shiptype));
        return shotguns;
    }

    numGuns = gunInfo->numGuns;

    if(ship->staticinfo->shipclass == CLASS_Fighter)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Fighter][ship->tacticstype];
    }
    else if (ship->staticinfo->shipclass == CLASS_Corvette)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Fighter][ship->tacticstype];
    }
    else
    {
        bonus = 1.0f;
    }

    for (i=0;i<numGuns;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;
        if (bitTest(ship->flags, SOF_NISShip))
        {
            triggerHappy = NIS_TriggerHappyAngle;
        }
        else
        {
            triggerHappy = gunstatic->triggerHappy;
        }

        if(ship->staticinfo->madStatic != NULL)
        {
            //ship has meshanimations
            if(ship->staticinfo->madStatic->numGunOpenIndexes > 0)
            {
                //has gun opening animations
                if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN &&
                    ship->madGunStatus != MAD_STATUS_GUNS_OPENING)
                {
                    madOpenGunsShip(ship);
                    return shotguns;
                }
                else if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
                {
                    return shotguns;
                }
                //otherwise, we're open..try to fire away!
            }
        }

        if (range < gunstatic->bulletrange*bonus)
        {
            if (gunCanShoot(ship, gun))
            {
                switch (gunstatic->guntype)
                {
                    case GUN_MissileLauncher:
                        if (gunHasMissiles(gun))
                        {
                            matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                            dotprod = vecDotProduct(*trajectory,shipheading);

                            if (dotprod >= triggerHappy)
                            {
                                missileShoot(ship,gun,target);
                                shotguns = TRUE;
                            }
                        }
                        break;
                    case GUN_MineLauncher:
                        if (gunHasMissiles(gun))
                        {
                            matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                            dotprod = vecDotProduct(*trajectory,shipheading);

                            if (dotprod >= triggerHappy)
                            {
                                missileShoot(ship,gun,target);
                                shotguns = TRUE;
                            }
                        }
                    break;
                    case GUN_Fixed:
                        matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                        dotprod = vecDotProduct(*trajectory,shipheading);

                        if (dotprod >= triggerHappy)
                        {
                            gunShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    case GUN_Gimble:
                    case GUN_NewGimble:
                        if (gunOrientGimbleGun(ship,gun,target))
                        {
                            gunShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
            }
        }
    }
/*
    if (shotguns && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
    */
    return shotguns;
}

/*-----------------------------------------------------------------------------
    Name        : gunShootGunsAtMultipleTargets
    Description : shoots a ship's guns at multiple targets
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool gunShootGunsAtMultipleTargets(Ship *ship)
{
    GunInfo *gunInfo = ship->gunInfo;
    sdword numGuns = gunInfo->numGuns;
    Gun *gun;
    sdword i;
    vector shipheading;
    bool shotguns = FALSE;
    GunStatic *gunstatic;
    real32 dotprod;
    AttackTargets *attackTargets = ship->attackvars.multipleAttackTargets;
    SpaceObjRotImpTarg *target;
    real32 dist;
    vector trajectory;
    real32 temp;
    real32 triggerHappy;

    dbgAssert(attackTargets->numAttackTargets == numGuns);      // one attack target for each gun

    for (i=0;i<numGuns;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;
        if (bitTest(ship->flags, SOF_NISShip))
        {
            triggerHappy = NIS_TriggerHappyAngle;
        }
        else
        {
            triggerHappy = gunstatic->triggerHappy;
        }
        if(ship->staticinfo->madStatic != NULL)
        {
            //ship has meshanimations
            if(ship->staticinfo->madStatic->numGunOpenIndexes > 0)
            {
                //has gun opening animations
                if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN &&
                    ship->madGunStatus != MAD_STATUS_GUNS_OPENING)
                {
                    madOpenGunsShip(ship);
                    return shotguns;
                }
                else if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
                {
                    return shotguns;
                }
                //otherwise, we're open..try to fire away!
            }
        }

        if (gunCanShoot(ship, gun))
        {
            target = attackTargets->TargetPtr[i];

            if (target != NULL)
            {
                switch (gunstatic->guntype)
                {
                    case GUN_MissileLauncher:
                        if (gunHasMissiles(gun))
                        {
                            missileShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;
                    case GUN_MineLauncher:
                        if (gunHasMissiles(gun))
                        {
                            missileShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;
                    case GUN_Fixed:
                        aishipGetTrajectory(ship,target,&trajectory);
                        dist = fsqrt(vecMagnitudeSquared(trajectory));
                        vecDivideByScalar(trajectory,dist,temp);

                        matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);

                        dotprod = vecDotProduct(trajectory,shipheading);

                        if (dotprod >= triggerHappy)
                        {
                            gunShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    case GUN_Gimble:
                    case GUN_NewGimble:
                        if (gunOrientGimbleGun(ship,gun,target))
                        {
                            gunShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
            }
        }
        else if (gunstatic->slaveDriver >= 0)
        {
            gunUpdateSlave(gun, gunInfo, gunstatic->slaveDriver);
        }
    }
/*
    if (shotguns && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
*/
    return shotguns;
}

/*-----------------------------------------------------------------------------
    Name        : gunMatrixUpdate
    Description : Binding matrix callback for updating gun mesh positions.
    Inputs      : flags - flags indicating certain attributes (see mesh.h)
                  startMatrix - starting matrix (from .geo file)
                  matrix - matrix to store result in.
                  data - pointer to Gun structure
                  ID - pointer to ship structure
    Outputs     : computes and fills in the gun matrix.
    Return      : true = matrix updated
----------------------------------------------------------------------------*/
bool gunMatrixUpdate(udword flags, hmatrix *startMatrix, hmatrix *matrix, void *data, sdword ID)
{
    Ship *ship;
    Gun *gun;
    GunStatic *gunstatic;
    bool bRecoil;
    vector recoilVector;

    ship = (Ship *)ID;
    dbgAssert(ship->objtype == OBJ_ShipType);
    gun = (Gun *)data;
    gunstatic = gun->gunstatic;

    bRecoil = bitTest(flags, HBF_Recoil);
    flags &= (HBF_Declination|HBF_Azimuth);

    switch (gunstatic->guntype)
    {
        case GUN_NewGimble:
            if (flags == (HBF_Declination|HBF_Azimuth))
            {
                if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
                {
                    hmatMakeHMatFromMat(matrix,&gun->curGunOrientationNonConcat);
                }
                else
                {
                    hmatMakeHMatFromMat(matrix,&gun->curgunorientation);
                }
                matrix->m14 = startMatrix->m14;
                matrix->m24 = startMatrix->m24;
                matrix->m34 = startMatrix->m34;
                break;
            }
            else if (flags == HBF_Azimuth)
            {
                hmatrix rotmatrix;
                real32 angle = gun->angle;
                hmatMakeRotAboutX(&rotmatrix,(real32)cos(angle),(real32)sin(angle));
                hmatMultiplyHMatByHMat(matrix, startMatrix, &rotmatrix);
                break;
            }
            else if (flags == HBF_Declination)
            {
                hmatrix rotmatrix;
                real32 declination = gun->declination;
                hmatMakeRotAboutY(&rotmatrix,(real32)cos(declination),(real32)sin(declination));
                hmatMultiplyHMatByHMat(matrix, startMatrix, &rotmatrix);
                break;
            }
            else
            {
//                dbgAssert(FALSE);
//                return FALSE;
                *matrix = IdentityHMatrix;
                matrix->m14 = startMatrix->m14;
                matrix->m24 = startMatrix->m24;
                matrix->m34 = startMatrix->m34;
                break;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
    //add recoil, if any
    if (bRecoil)
    {
        hmatGetVectFromHMatrixCol3(recoilVector, *matrix);
        gunRecoilVectorCompute(&recoilVector, gunstatic->recoilLength, gun->lasttimefired, gunstatic->firetime);
//        vecAddTo(bullet->posinfo.position, offset0);
        matrix->m14 += recoilVector.x;
        matrix->m24 += recoilVector.y;
        matrix->m34 += recoilVector.z;
    }


    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : gunDrawGunInfo
    Description : draws gun debug information
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VISUALIZATION
void gunDrawGunInfo(Ship *ship)
{
    sdword i;
    GunInfo *gunInfo = ship->gunInfo;
    sdword numGuns;
    Gun *gun;
    GunStatic *gunstatic;
    vector to, from, offset0;

    if (gunInfo == NULL)
    {
        return;
    }

    numGuns = gunInfo->numGuns;

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    for (i=0,gun=&gunInfo->guns[0];i<numGuns;i++,gun++)
    {
        gunstatic = gun->gunstatic;
        from = gun->gunstatic->position;
        vecScalarMultiply(to, gun->gunheading,500.0f);
        vecAddTo(to, from);
        // heading

        //gun offset
        if (gunstatic->offset.x != 0.0f)
        {                                                       //if there is an offset vector
            matMultiplyMatByVec(&offset0, &gun->curgunorientation, &gunstatic->offset);//offset in gun space
            vecAddTo(from, offset0);                            //add to position
            vecAddTo(to, offset0);                            //add to position
        }
        //add barrel length
        from.x += gun->gunheading.x * gunstatic->barrelLength;
        from.y += gun->gunheading.y * gunstatic->barrelLength;
        from.z += gun->gunheading.z * gunstatic->barrelLength;
        to.x += gun->gunheading.x * gunstatic->barrelLength;
        to.y += gun->gunheading.y * gunstatic->barrelLength;
        to.z += gun->gunheading.z * gunstatic->barrelLength;
        primLine3(&from, &to, colFuscia/*colRGB(10, 255, 100)*/);
    }

    rndLightingEnable(TRUE);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : gunFirePower
    Description : Compute the firepower of a gun for a given tactics setting.
    Inputs      : gunStatic - static info of the gun
                  tactic - tactics setting to determine damage for
    Outputs     : fireTime - the computed rate of fire.  Not set for guns of type BULLET_Beam.
    Return      :
----------------------------------------------------------------------------*/
real32 gunFirePower(GunStatic *gunStatic, TacticsType tactics, real32 *fireTime)
{
    dbgAssert(tactics == Evasive || tactics == Neutral || tactics == Aggressive);

    if (gunStatic->firetime == 0.0f)
    {
        *fireTime = UNIVERSE_UPDATE_PERIOD;
    }
    else
    {
        *fireTime = gunStatic->firetime;
    }
    if (gunStatic->bulletType == BULLET_Beam)
    {
        return((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f * (real32)UNIVERSE_UPDATE_RATE * gunStatic->bulletlifetime / *fireTime);
    }
    else if(gunStatic->burstFireTime != 0.0f)
    {
        //burst fire gun..add aditional factor of:
        //burstfiretime/(burstfiretime_burstwaittime)
        return(((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f / *fireTime)*(gunStatic->burstFireTime/(gunStatic->burstFireTime+gunStatic->burstWaitTime)));
    }
    return((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f / *fireTime);
}

/*-----------------------------------------------------------------------------
    Name        : gunShipFirePower
    Description : Compute the firepower for a given ship at some tactics setting
    Inputs      : info - staticinfo of ship
                  tactics - what tactic to compute damage for
    Outputs     :
    Return      : total damage of the ship
----------------------------------------------------------------------------*/
#define GUN_NumberInterceptorTypes      7
ubyte gunInterceptorTypes[GUN_NumberInterceptorTypes] = {LightInterceptor, HeavyInterceptor, CloakedFighter, P1Fighter, P2Swarmer, P2AdvanceSwarmer, TargetDrone};
real32 gunShipFirePower(ShipStaticInfo *info, TacticsType tactics)
{
    sdword index;
    real32 power = 0.0f, powerThisGun, totalFireRate, fireRate;
    GenericInterceptorStatics *interceptorstat;

    if (memchr(gunInterceptorTypes, info->shiptype, GUN_NumberInterceptorTypes))
    {                                                       //interceptors have a different rate of fire computation
        interceptorstat = (GenericInterceptorStatics *)info->custstatinfo;
        totalFireRate = interceptorstat->firetime;
        if (info->gunStaticInfo != NULL)
        {
            for (index = 0; index < info->gunStaticInfo->numGuns; index++)
            {
                powerThisGun = gunFirePower(&info->gunStaticInfo->gunstatics[index], tactics, &fireRate);
                powerThisGun *= fireRate / (totalFireRate * (real32)info->gunStaticInfo->numGuns);
                power += powerThisGun;
            }
        }
    }
    else
    {
        if (info->gunStaticInfo != NULL)
        {
            for (index = 0; index < info->gunStaticInfo->numGuns; index++)
            {
                power += gunFirePower(&info->gunStaticInfo->gunstatics[index], tactics, &fireRate);
            }
        }
    }
    return(power);
}

/*-----------------------------------------------------------------------------
    Name        : gunStartup
    Description : Start the gun module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunStartup(void)
{
    gunRecoilTableCompute();
}

/*-----------------------------------------------------------------------------
    Name        : gunShutdown
    Description : Shut down the gun module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunShutdown(void)
{
    ;
}

#if GUN_TUNE_MODE

bool gunTuningMode = FALSE;
sdword tuningGun = 0;

void gunTuneGun(Ship *ship)
{
    ShipStaticInfo *shipstatic = ship->staticinfo;
    GunInfo *gunInfo = ship->gunInfo;
    Gun *gun;
    GunStatic *gunstatic;
    bool updateguncoordsys = FALSE;

    if (gunInfo == NULL)
    {
        return;
    }

    if (gunInfo->numGuns == 0)
    {
        return;
    }

    if (tuningGun >= gunInfo->numGuns)
    {
        tuningGun = 0;
    }

    gun = &gunInfo->guns[tuningGun];
    gunstatic = gun->gunstatic;

    switch (gunstatic->guntype)
    {
        case GUN_NewGimble:
            if (keyIsHit(ARRLEFT))
            {
                gun->angle -= GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRRIGHT))
            {
                gun->angle += GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRUP))
            {
                gun->declination += GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRDOWN))
            {
                gun->declination -= GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }
            if (keyIsHit(NUMPAD0))
            {
                gun->angle = 0.0f;
                gun->declination = 0.0f;
                updateguncoordsys = TRUE;
            }

            if (updateguncoordsys)
            {
                if (gun->angle < gunstatic->minturnangle)
                {
                    gun->angle = gunstatic->minturnangle;
                }

                if (gun->angle > gunstatic->maxturnangle)
                {
                    gun->angle = gunstatic->maxturnangle;
                }

                if (gun->declination < gunstatic->mindeclination)
                {
                    gun->declination = gunstatic->mindeclination;
                }

                if (gun->declination > gunstatic->maxdeclination)
                {
                    gun->declination = gunstatic->maxdeclination;
                }

                gunNewGimbleUpdateCoordSys(gun,gunstatic);
            }

            {
                char buffer[300];
                sprintf(buffer,"Gun %d %s Angle:%4.2f Declin:%4.2f Heading:%2.3f %2.3f %2.3f %d",tuningGun,GunTypeToStr(gunstatic->guntype),
                           RAD_TO_DEG(gun->angle),RAD_TO_DEG(gun->declination),gun->gunheading.x,gun->gunheading.y,gun->gunheading.z,universe.univUpdateCounter);

                strcat(buffer,"\n");
                fontPrint(0, 400, colWhite, buffer);
                //dbgMessage(buffer);
            }
            break;

        default:
            fontPrintf(0, 400, colWhite,
                       "Gun %d %s",tuningGun,GunTypeToStr(gunstatic->guntype));
            break;
    }
}
#endif

