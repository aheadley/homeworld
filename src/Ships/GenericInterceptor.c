/*=============================================================================
    Name    : GenericInterceptor.c
    Purpose : Specifics for the Generic Interceptor

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Types.h"
#include "FastMath.h"
#include "Debug.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "Collision.h"
#include "Physics.h"
#include "Universe.h"
#include "GenericInterceptor.h"
#include "StatScript.h"
#include "Gun.h"
#include "AIShip.h"
#include "AITrack.h"
#include "MEX.h"
#include "SoundEvent.h"
#include "FlightMan.h"
#include "CommandLayer.h"
#include "UnivUpdate.h"
#include "Tactics.h"
#include "NIS.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "DefenseFighter.h"
#include "Randy.h"
#include "Battle.h"

#ifdef gshaw
//#define DEBUG_AIATTACK
#endif

#define FAKE_FLY_BY_DISTANCE_MUCH_BIGGER_THAN_NEEDED   15000.0f

sdword FIGHTER_BREAK_ANGLE_MIN = 25;
sdword FIGHTER_BREAK_ANGLE_MAX = 45;

sdword FIGHTER_BREAK_VERTICAL_ANGLE_MIN = 10;
sdword FIGHTER_BREAK_VERTICAL_ANGLE_MAX = 30;

bool BombersUseBombingRun = FALSE;

/*
typedef struct
{
    real32 firetime;
    real32 maxFlyAwayDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxFlyAwayDistSquared[NUM_TACTICS_TYPES][NUM_CLASSES+1];    // use NUM_CLASSES entry as DefaultClass (so array len = NUM_CLASSES+1)
    real32 breakRange[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 flyPastDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 triggerHappy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 faceTargetAccuracy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxAttackTime[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    FlightManProb flightmanProb[NUM_TACTICS_TYPES][NUM_CLASSES+1];
} GenericInterceptorStatics;
*/
typedef struct          //inherited Structure for Cloaked Fighter
{
    real32 firetime;
    real32 maxFlyAwayDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxFlyAwayDistSquared[NUM_TACTICS_TYPES][NUM_CLASSES+1];    // use NUM_CLASSES entry as DefaultClass (so array len = NUM_CLASSES+1)
    real32 breakRange[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 flyPastDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 triggerHappy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 faceTargetAccuracy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxAttackTime[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    FlightManProb flightmanProb[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 CloakFuelBurnRate;
    real32 ForceDeCloakAtFuelPercent;
    real32 DeCloakFuelLevel;
    real32 CloakingTime;
    real32 DeCloakingTime;
    real32 VisibleState;
    real32 battleReCloakTime;
} CloakedFighterStatics;

GenericInterceptorStatics GenericInterceptorStatic;
CloakedFighterStatics CloakedFighterStatic;

GenericInterceptorStatics LightInterceptorStaticRace1;
GenericInterceptorStatics LightInterceptorStaticRace2;

GenericInterceptorStatics HeavyInterceptorStaticRace1;
GenericInterceptorStatics HeavyInterceptorStaticRace2;

GenericInterceptorStatics AttackBomberStaticRace1;
GenericInterceptorStatics AttackBomberStaticRace2;

CloakedFighterStatics CloakedFighterStaticRace1;
CloakedFighterStatics CloakedFighterStaticRace2;

GenericInterceptorStatics P1FighterStatic;

GenericInterceptorStatics P2SwarmerStatic;

GenericInterceptorStatics P2AdvanceSwarmerStatic;

GenericInterceptorStatics TargetDroneStatic;

scriptStructEntry LIStaticScriptTable[] =
{
    { "gunsFireTime",           scriptSetReal32CB,                  (udword) &(GenericInterceptorStatic.firetime),          (udword) &(GenericInterceptorStatic) },

    { "flightmanTurnaround",    (void(*)(char *,char *,void*)) scriptSetFlightManTurnaroundCB,     (udword) &(GenericInterceptorStatic.flightmanProb),     (udword) &(GenericInterceptorStatic) },
    { "flightmanAIP",           (void(*)(char *,char *,void*))scriptSetFlightManAIPCB,            (udword) &(GenericInterceptorStatic.flightmanProb),     (udword) &(GenericInterceptorStatic) },
    { "flightmanEvasiveBehind", (void(*)(char *,char *,void*))scriptSetFlightManEvasiveBehindCB,  (udword) &(GenericInterceptorStatic.flightmanProb),     (udword) &(GenericInterceptorStatic) },
    { "flightmanEvasiveFront",  (void(*)(char *,char *,void*))scriptSetFlightManEvasiveFrontCB,   (udword) &(GenericInterceptorStatic.flightmanProb),     (udword) &(GenericInterceptorStatic) },
    { "flightmanEvasivePure",   (void(*)(char *,char *,void*))scriptSetFlightManEvasivePureCB,    (udword) &(GenericInterceptorStatic.flightmanProb),     (udword) &(GenericInterceptorStatic) },
    { "maxFlyAwayDist",         scriptSetReal32CB_ARRAY,            (udword) &(GenericInterceptorStatic.maxFlyAwayDist),    (udword) &(GenericInterceptorStatic) },
    { "breakRange",             scriptSetReal32CB_ARRAY,            (udword) &(GenericInterceptorStatic.breakRange),        (udword) &(GenericInterceptorStatic) },
    { "flyPastDist",            scriptSetReal32CB_ARRAY,            (udword) &(GenericInterceptorStatic.flyPastDist),       (udword) &(GenericInterceptorStatic) },
    { "triggerHappy",           scriptSetCosAngCB_ARRAY,            (udword) &(GenericInterceptorStatic.triggerHappy),      (udword) &(GenericInterceptorStatic) },
    { "faceTargetAccuracy",     scriptSetCosAngCB_ARRAY,            (udword) &(GenericInterceptorStatic.faceTargetAccuracy),(udword) &(GenericInterceptorStatic) },
    { "maxAttackTime",          scriptSetReal32CB_ARRAY,            (udword) &(GenericInterceptorStatic.maxAttackTime),     (udword) &(GenericInterceptorStatic) },

    { NULL,NULL,0,0 }
};

scriptStructEntry CloakedFighterStaticScriptTable[] =
{
    { "CloakFuelBurnRate",          scriptSetReal32CB, (udword) &(CloakedFighterStatic.CloakFuelBurnRate),          (udword) &(CloakedFighterStatic) },
    { "ForceDeCloakAtFuelPercent",  scriptSetReal32CB, (udword) &(CloakedFighterStatic.ForceDeCloakAtFuelPercent),  (udword) &(CloakedFighterStatic) },
    { "CloakingTime",               scriptSetReal32CB, (udword) &(CloakedFighterStatic.CloakingTime),               (udword) &(CloakedFighterStatic) },
    { "DeCloakingTime",             scriptSetReal32CB, (udword) &(CloakedFighterStatic.DeCloakingTime),             (udword) &(CloakedFighterStatic) },
    { "VisibleState",               scriptSetReal32CB, (udword) &(CloakedFighterStatic.VisibleState),               (udword) &(CloakedFighterStatic) },
    { "battleReCloakTime",          scriptSetReal32CB, (udword) &(CloakedFighterStatic.battleReCloakTime),          (udword) &(CloakedFighterStatic) },

    { NULL,NULL,0,0 }
};

void GenericInterceptorStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    GenericInterceptorStatics *interceptorstat = NULL;
    udword i;
    TacticsType t;

    switch (statinfo->shiptype)
    {
        case LightInterceptor:
           interceptorstat = (statinfo->shiprace == R1) ? &LightInterceptorStaticRace1 : &LightInterceptorStaticRace2;
           break;
        case HeavyInterceptor:
           interceptorstat = (statinfo->shiprace == R1) ? &HeavyInterceptorStaticRace1 : &HeavyInterceptorStaticRace2;
           break;
        case AttackBomber:
           interceptorstat = (statinfo->shiprace == R1) ? &AttackBomberStaticRace1 : &AttackBomberStaticRace2;
           break;
        case CloakedFighter:
           interceptorstat = (statinfo->shiprace == R1) ? (GenericInterceptorStatics *)&CloakedFighterStaticRace1 : (GenericInterceptorStatics *)&CloakedFighterStaticRace2;
           break;
        case P1Fighter:
           interceptorstat = &P1FighterStatic;
           break;
        case P2Swarmer:
           interceptorstat = &P2SwarmerStatic;
           break;
        case P2AdvanceSwarmer:
           interceptorstat = &P2AdvanceSwarmerStatic;
           break;
        case TargetDrone:
           interceptorstat = &TargetDroneStatic;
           break;
    }
    dbgAssert(interceptorstat != NULL);

    statinfo->custstatinfo = interceptorstat;

    memset(interceptorstat,0,sizeof(*interceptorstat));

    scriptSetStruct(directory,filename,LIStaticScriptTable,(ubyte *)interceptorstat);

    for(t = Evasive; t < NUM_TACTICS_TYPES; t++)
        for (i = 0; i < NUM_CLASSES + 1; i++)
        {
            interceptorstat->maxFlyAwayDistSquared[t][i] = interceptorstat->maxFlyAwayDist[t][i] * interceptorstat->maxFlyAwayDist[t][i];
        }

    for(t = Evasive; t < NUM_TACTICS_TYPES; t++)
        for (i = 0; i < NUM_CLASSES; i++)
        {
            if (interceptorstat->flyPastDist[t][i] == 0.0f)
            {
                interceptorstat->maxFlyAwayDist[t][i]        = interceptorstat->maxFlyAwayDist[t][NUM_CLASSES];
                interceptorstat->breakRange[t][i]            = interceptorstat->breakRange[t][NUM_CLASSES];
                interceptorstat->flyPastDist[t][i]           = interceptorstat->flyPastDist[t][NUM_CLASSES];
                interceptorstat->triggerHappy[t][i]          = interceptorstat->triggerHappy[t][NUM_CLASSES];
                interceptorstat->faceTargetAccuracy[t][i]    = interceptorstat->faceTargetAccuracy[t][NUM_CLASSES];
                interceptorstat->maxAttackTime[t][i]         = interceptorstat->maxAttackTime[t][NUM_CLASSES];
                interceptorstat->flightmanProb[t][i]         = interceptorstat->flightmanProb[t][NUM_CLASSES];
            }
        }

    if (statinfo->shiptype == TargetDrone)
        for (t = Evasive; t < NUM_TACTICS_TYPES; t++)
        {
            statinfo->bulletRange[t] = 2500.0f;
            statinfo->bulletRangeSquared[t] = 2500.0f * 2500.0f;
            statinfo->minBulletRange[t] = 2500.0f;
        }
}

void GenericInterceptorInit(Ship *ship)
{
    GenericInterceptorSpec *spec = (GenericInterceptorSpec *)ship->ShipSpecifics;

    spec->activeGun = 0;
}

bool GenericInterceptorCanFire(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory,real32 triggerHappy)
{
    GenericInterceptorSpec *spec = (GenericInterceptorSpec *)ship->ShipSpecifics;
    GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)(((ShipStaticInfo *)ship->staticinfo)->custstatinfo);
    Gun *curgun;
    Gun *gun;
    vector shipheading;
    real32 dotprod;
    GunInfo *gunInfo = ship->gunInfo;
    sdword i;
    bool returnval;
    bool canUseGun;
    sdword pickgun;

    if (ship->shiptype == TargetDrone)
    {
        //target drones attack like interceptors, but don't have guns
        return FALSE;
    }

    if(ship->aistateattack == STATE_WHIPSTRAFE1)
    {
        triggerHappy = 0.0f;
    }

    if (bitTest(ship->flags, SOF_NISShip))
    {
        triggerHappy = NIS_TriggerHappyAngle;
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
                return FALSE;
            }
            else if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
            {
                return FALSE;
            }
            //otherwise, we're open..try to fire away!
        }
    }

    if ((universe.totaltimeelapsed - spec->lasttimefired) < interceptorstat->firetime)
    {
        return FALSE;   // if time since last shot fired is less than the gun firetime, don't shoot.
    }

    for (i=0,canUseGun=FALSE,pickgun=spec->activeGun;i<gunInfo->numGuns;i++)
    {
        if (gunCanShoot(ship, &gunInfo->guns[pickgun]))
        {
            canUseGun = TRUE;
            break;
        }

        pickgun++;
        if (pickgun>=gunInfo->numGuns)
        {
            pickgun = 0;
        }
    }

    if (canUseGun)
    {
        spec->activeGun = pickgun;
        dbgAssert(spec->activeGun < gunInfo->numGuns);
        gun = &gunInfo->guns[spec->activeGun];

        switch (gun->gunstatic->guntype)
        {
            case GUN_Fixed:
                matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                dotprod = vecDotProduct(*trajectory,shipheading);

                if (dotprod >= triggerHappy)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }

            case GUN_Gimble:
                if (gunOrientGimbleGun(ship,gun,target))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }

            case GUN_NewGimble:
                // orient all other gimble guns as well
                for (i=0,curgun=&gunInfo->guns[0];i<gunInfo->numGuns;i++,curgun++)
                {
                    if (i == spec->activeGun)
                    {
                        dbgAssert(curgun == gun);
                        returnval = gunOrientGimbleGun(ship,curgun,target);
                    }
                    else
                    {
                        gunOrientGimbleGun(ship,curgun,target);
                    }
                }
                return returnval;

            default:
                dbgAssert(FALSE);
                return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

void GenericInterceptorFire(Ship *ship,SpaceObjRotImpTarg *target)
{
    GenericInterceptorSpec *spec = (GenericInterceptorSpec *)ship->ShipSpecifics;
    GunInfo *gunInfo = ship->gunInfo;

    spec->lasttimefired = universe.totaltimeelapsed;

    dbgAssert(spec->activeGun < gunInfo->numGuns);

    gunShoot(ship,&gunInfo->guns[spec->activeGun],target);

    spec->activeGun++;
    if (spec->activeGun >= gunInfo->numGuns)
    {
        spec->activeGun = 0;
    }
}

void setupWhipStrafe(Ship *ship,GenericInterceptorSpec *spec,Ship *target)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    vector targetshipvec;
    vector tmpvec2;
    real32 zsqr,rsqr;
    real32 temp;
    uword whipstrafeflags;

    targetshipvec = target->posinfo.position;
    temp = target->staticinfo->staticheader.staticCollInfo.approxcollspheresize + shipstaticinfo->bulletRange[ship->tacticstype] * WHIPSTRAFE_BULLETRANGEOPTIMUM;

    if (ship->posinfo.position.z > targetshipvec.z)
    {
        whipstrafeflags = WHIPSTRAFE_ABOVE;
        targetshipvec.z += temp;
    }
    else
    {
        whipstrafeflags = 0;
        targetshipvec.z -= temp;
    }

    vecSub(tmpvec2,targetshipvec,ship->posinfo.position);

    zsqr = tmpvec2.z * tmpvec2.z;
    rsqr = tmpvec2.x * tmpvec2.x + tmpvec2.y * tmpvec2.y;

    if (zsqr <= rsqr*WHIPSTRAFE_MAXANGCOMPAREFACTOR)
    {
        // angle is appropriatley low, so continue
        vecMultiplyByScalar(tmpvec2,WHIPSTRAFE_FLYBYOVERSHOOT);
        vecAdd(spec->aivec,ship->posinfo.position,tmpvec2);

        spec->aiAIPflightman = TRUE;
        flightmanInit(ship,FLIGHTMAN_WHIP_STRAFE);
        dbgAssert(ship->flightman == FLIGHTMAN_WHIP_STRAFE);
        ship->flightmanState2 = (ubyte) whipstrafeflags;
        ship->aistateattack = STATE_WHIPSTRAFE1;
        spec->aitimekill = universe.totaltimeelapsed;
    }
}

#define TransitionToSTATE_REAPPROACH           \
    ship->aistateattack = STATE_REAPPROACH;                                                                                \
    spec->aiAIPflightman = FALSE;                                                                                          \
    ship->aistate = 0;                                                                                                     \
                                                                                                                           \
    if ((ship->flightman == FLIGHTMAN_NULL) && (target->objtype == OBJ_ShipType) && (isCapitalShip((Ship *)target)) && (((ShipStaticInfo *)(target->staticinfo))->shipclass != CLASS_Mothership)) \
    {                                                                                                                      \
        if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_WHIP_STRAFE))         \
        {                                                                                                                  \
            setupWhipStrafe(ship,spec,(Ship *)target);                                                                             \
        }                                                                                                                  \
    }

void GenericInterceptorPassiveAttack(Ship *ship,Ship *target,bool rotate)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)shipstaticinfo->custstatinfo;
    uword targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
    vector trajectory;
    real32 range;
    real32 dist,temp;

    //Commented out for now...was causing crashwith CloakedFighter
    //dbgAssert(shipstaticinfo->custshipheader.CustShipFire == GenericInterceptorFire);

    if (shipstaticinfo->gunStaticInfo)
        aishipGetTrajectoryWithVelPrediction(ship,(SpaceObjRotImpTarg *)target,shipstaticinfo->gunStaticInfo->gunstatics[0].bulletspeed,&trajectory);
    else
        aishipGetTrajectory(ship,(SpaceObjRotImpTarg *)target,&trajectory);

    dist = fsqrt(vecMagnitudeSquared(trajectory));
    vecDivideByScalar(trajectory,dist,temp);

    range = RangeToTargetGivenDist(ship,(SpaceObjRotImpTarg *)target,dist);

    if ((rotate) & ((bool)shipstaticinfo->rotateToRetaliate))
    {
        aitrackHeading(ship,&trajectory,FLYSHIP_ATTACKACCURACY);
    }

    if (range < shipstaticinfo->bulletRange[ship->tacticstype])
    {
        if (GenericInterceptorCanFire(ship,(SpaceObjRotImpTarg *)target,&trajectory,interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
        {
            //GenericInterceptorFire(ship);
            shipstaticinfo->custshipheader.CustShipFire(ship,(SpaceObjRotImpTarg *)target);
        }
    }
    /*
    //check to see if we should back up!
    if(ship->tacticstype == Evasive)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
        if(command != NULL)
        {
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
        aishipFlyToPointAvoidingObjs(ship,&trajectory,AISHIP_FastAsPossible,0.0f);
    }
    */
}

// works with DefenseFighter as well, even though it is not a generic interceptor
FlightManProb *GenericInterceptorGetFlightManProb(Ship *ship,SpaceObjRotImpTarg *target)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    uword targetIndex;

    if ((target != NULL) && (target->objtype == OBJ_ShipType))
    {
        targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
    }
    else
    {
        targetIndex = (uword)NUM_CLASSES;
    }

    if (ship->shiptype == DefenseFighter)
        return &((DefenseFighterStatics *)shipstaticinfo->custstatinfo)->flightmanProb[ship->tacticstype][targetIndex];
    else
        return &((GenericInterceptorStatics *)shipstaticinfo->custstatinfo)->flightmanProb[ship->tacticstype][targetIndex];
}

void CalcRallyPoint(vector s, vector t, AttackBomberSpec *diveinfo)
{
    s.z = t.z;

    vecSub(diveinfo->rallypoint, s, t);
    vecNormalize(&diveinfo->rallypoint);
    vecMultiplyByScalar(diveinfo->rallypoint, 700.0f);

    if (t.z < 0.0f)
    {
        diveinfo->rallypoint.z += 1000.0f;
    }
    else
    {
        diveinfo->rallypoint.z -= 1000.0f;
    }

    diveinfo->angle = -1.0f;
}

void GenericInterceptorAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    GenericInterceptorSpec *spec = (GenericInterceptorSpec *)ship->ShipSpecifics;
    vector trajectory;
    //real32 tempreal;
    real32 dist;
    real32 range;
    real32 temp;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)shipstaticinfo->custstatinfo;
    uword targetIndex;
    udword flighttodo;
    vector tmpvec;
    matrix tmpmat;
    sdword randeg;
    real32 randegf;
    udword flags;
    sdword curdistonly;
    vector rallypoint,dest;
    AttackBomberSpec *diveinfo;
    real32 sintheta, costheta;
    TypeOfFormation formtype = NO_FORMATION;

    if (target->objtype == OBJ_ShipType)
    {
        targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
    }
    else
    {
        targetIndex = (uword)NUM_CLASSES;
    }

    switch (ship->aistateattack)
    {
        case STATE_INIT:
            spec->aiAIPflightman = FALSE;
            if((ship->shiptype != AttackBomber) || (!BombersUseBombingRun) || (targetIndex > CLASS_Frigate) ||
               ((target->objtype == OBJ_ShipType) && (((Ship *)target)->shiptype == Mothership) && (((Ship *)target)->shiprace == R1)))
            {
                ship->aistateattack = STATE_APPROACH;
                aishipGetTrajectory(ship,target,&trajectory);
                aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,0.0f);
                range = RangeToTarget(ship,target,&trajectory);

                if (range < shipstaticinfo->bulletRange[ship->tacticstype])
                {
                    ship->aistateattack = STATE_KILL;
                    spec->aitimekill = universe.totaltimeelapsed;
                }
                break;
            }
            ship->aistateattack = STATE_START_BOMBING_RUN;
            CalcRallyPoint(ship->posinfo.position, target->posinfo.position, (AttackBomberSpec *)ship->ShipSpecifics);

            // fall right into bombing run code

        case STATE_START_BOMBING_RUN:
            diveinfo = (AttackBomberSpec *)ship->ShipSpecifics;
            if(diveinfo->angle == -1.0f)
            {
                memcpy(&diveinfo->orig_point, &diveinfo->rallypoint, sizeof(vector));
                diveinfo->angle = DEG_TO_RAD(45);
                sintheta = sin(diveinfo->angle);
                costheta = cos(diveinfo->angle);
                diveinfo->rallypoint.x = (diveinfo->orig_point.x * costheta) + (diveinfo->orig_point.y * sintheta);   // rotate rallypoint X degrees clockwise
                diveinfo->rallypoint.y = (diveinfo->orig_point.y * costheta) - (diveinfo->orig_point.x * sintheta);
            }
            vecAdd(rallypoint, target->posinfo.position, diveinfo->rallypoint);
            vecSub(tmpvec, ship->posinfo.position, rallypoint);
            if(vecMagnitudeSquared(tmpvec) < 5625)  // within 75 metres
            {
                if((gamerand() & 15) == 0)      // 1 in 16 chance of starting bombing dive
                {
                    ship->aistateattack = STATE_BOMBING_DIVE;
                    diveinfo->rallypoint.x = -(diveinfo->rallypoint.x);
                    diveinfo->rallypoint.y = -(diveinfo->rallypoint.y);
                    diveinfo->rallypoint.z = -200.0f;
                    diveinfo->bombingdelay = 15;
                }
                else
                {
                    diveinfo->angle += DEG_TO_RAD(10);
                    if(diveinfo->angle >= DEG_TO_RAD(360))
                        diveinfo->angle -= DEG_TO_RAD(360);
                    sintheta = sin(diveinfo->angle);
                    costheta = cos(diveinfo->angle);
                    diveinfo->rallypoint.x = (diveinfo->orig_point.x * costheta) + (diveinfo->orig_point.y * sintheta);   // rotate rallypoint X degrees clockwise
                    diveinfo->rallypoint.y = (diveinfo->orig_point.y * costheta) - (diveinfo->orig_point.x * sintheta);
                }
            }
            else
                aishipFlyToPointAvoidingObjsWithVel(ship, &rallypoint, AISHIP_PointInDirectionFlying | AISHIP_DontFlyToObscuredPoints | AISHIP_FastAsPossible, -400.0f, &target->posinfo.velocity);

            break;

        case STATE_BOMBING_DIVE:
            diveinfo = (AttackBomberSpec *)ship->ShipSpecifics;
            vecAdd(rallypoint, target->posinfo.position, diveinfo->rallypoint);
            vecSub(tmpvec, ship->posinfo.position, rallypoint);
            if(vecMagnitudeSquared(tmpvec) < 5625)  // within 75 metres
                ship->aistateattack = STATE_INIT;
            else
                aishipFlyToPointAvoidingObjsWithVel(ship, &rallypoint, AISHIP_PointInDirectionFlying | AISHIP_DontFlyToObscuredPoints | AISHIP_FastAsPossible, -400.0f, &target->posinfo.velocity);

            if(diveinfo->bombingdelay)
                diveinfo->bombingdelay--;
            else if (GenericInterceptorCanFire(ship,target,&trajectory,interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
                shipstaticinfo->custshipheader.CustShipFire(ship,target);
            break;

        case STATE_APPROACH:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_APPROACH",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,0.0f);
            range = RangeToTarget(ship,target,&trajectory);

            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                ship->aistateattack = STATE_KILL;
                spec->aitimekill = universe.totaltimeelapsed;
            }
            break;

        case STATE_TURNAROUND:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_TURNAROUND",(udword)ship);
#endif
            if (flightmanExecute(ship))
            {
                ship->tacticsTalk = 0;          //reset var
                TransitionToSTATE_REAPPROACH
            }
            break;

        case STATE_REAPPROACH:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_REAPPROACH",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying,0.0f);
            range = RangeToTarget(ship,target,&trajectory);

            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                ship->aistateattack = STATE_KILL;
                spec->aitimekill = universe.totaltimeelapsed;
            }
            break;

        case STATE_WHIPSTRAFE1:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_WHIPSTRAFE1",(udword)ship);
#endif
//            dbgAssert(ship->flightman == FLIGHTMAN_WHIP_STRAFE);
            if (ship->flightman != FLIGHTMAN_WHIP_STRAFE)
            {
                dbgMessagef("\nWarning: Whip strafe cancelled");
                ship->aistateattack = STATE_INIT;
                break;
            }

            aishipGetTrajectoryWithAngleCorrection(ship,target,&trajectory);

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory,dist,temp);

            range = RangeToTargetGivenDist(ship,target,dist);

            if (range < (shipstaticinfo->bulletRange[ship->tacticstype]*0.7f))
            {
                aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible,WHIPSTRAFE_MINVELOCITY);
                aitrackHeadingWithBank(ship,&trajectory,0.99999f /*interceptorstat->faceTargetAccuracy[targetIndex]*/,shipstaticinfo->sinbank);

                if (GenericInterceptorCanFire(ship,target,&trajectory,interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
                {
                    //GenericInterceptorFire(ship);
                    shipstaticinfo->custshipheader.CustShipFire(ship,target);
                }
            }
            else
            {
                aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,WHIPSTRAFE_MINVELOCITY);
            }

            if ((MoveReachedDestinationVariable(ship,&spec->aivec,WHIPSTRAFE_REACHDESTINATIONRANGE) || ((universe.totaltimeelapsed - spec->aitimekill) > interceptorstat->maxAttackTime[ship->tacticstype][targetIndex])))
            {
                ship->aistateattack = STATE_WHIPSTRAFE2;
            }
            break;

        case STATE_WHIPSTRAFE2:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_WHIPSTRAFE2",(udword)ship);
#endif
//            dbgAssert(ship->flightman == FLIGHTMAN_WHIP_STRAFE);
            if (ship->flightman != FLIGHTMAN_WHIP_STRAFE)
            {
                dbgMessagef("\nWarning: Whip strafe cancelled");
                ship->aistateattack = STATE_INIT;
                break;
            }

            if (flightmanExecute(ship))
            {
                ship->aistateattack = STATE_BREAK;

                vecSub(tmpvec,ship->posinfo.position,target->posinfo.position);
                tmpvec.z = 0.0f;
                vecNormalizeToLength(&tmpvec,interceptorstat->flyPastDist[ship->tacticstype][targetIndex]);

                randeg = randombetween(FIGHTER_BREAK_ANGLE_MIN,FIGHTER_BREAK_ANGLE_MAX);
                randegf = (real32) ((randeg & 1) ? randeg : -randeg);
                randegf = DEG_TO_RAD(randegf);

                matMakeRotAboutZ(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
                matMultiplyMatByVec(&spec->aivec,&tmpmat,&tmpvec);

                vecAddTo(spec->aivec,target->posinfo.position);
            }
            break;

        case STATE_KILL:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_KILL",(udword)ship);
#endif
            if (shipstaticinfo->gunStaticInfo)
                aishipGetTrajectoryWithVelPrediction(ship,target,shipstaticinfo->gunStaticInfo->gunstatics[0].bulletspeed,&trajectory);
            else
                aishipGetTrajectory(ship,target,&trajectory);

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory,dist,temp);

            if (vecDotProduct(trajectory,target->posinfo.velocity) < 0)     // never track a velocity of ship moving towards us,
            {                                                               // because we don't want to back up or slow down.
                aishipFlyToShipAvoidingObjs(ship,target,AISHIP_FastAsPossible,-shipstaticinfo->staticheader.maxvelocity);
            }
            else
            {
                aishipFlyToShipAvoidingObjsWithVel(ship,target,AISHIP_FastAsPossible,-shipstaticinfo->staticheader.maxvelocity,&target->posinfo.velocity);
            }

            if (ship->flightman == FLIGHTMAN_BARREL_ROLL || ship->flightman == FLIGHTMAN_SWARMER_BARRELROLL || ship->flightman == FLIGHTMAN_ROLL180)
            {
                flags = AITRACKHEADING_DONTROLL;
                if(flightmanExecute(ship))
                    ship->tacticsTalk = 0;  //reset variable
            }
            else
            {
                flags = 0;
                if(ship->flightman == FLIGHTMAN_CELEB_FLIP || ship->flightman == FLIGHTMAN_BARRELROLL_OUT)
                {
                    flightmanExecute(ship);
                    break;
                }
            }

            aitrackHeadingWithBankFlags(ship,&trajectory,interceptorstat->faceTargetAccuracy[ship->tacticstype][targetIndex],shipstaticinfo->sinbank,flags);

            range = RangeToTargetGivenDist(ship,target,dist);

            if (range < shipstaticinfo->bulletRange[ship->tacticstype])
            {
                if (GenericInterceptorCanFire(ship,target,&trajectory,interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
                {
                    //GenericInterceptorFire(ship);
                    shipstaticinfo->custshipheader.CustShipFire(ship,target);

                    if ((ship->flightman == FLIGHTMAN_NULL) && (!spec->aiAIPflightman))
                    {
                        spec->aiAIPflightman = TRUE;  // only try to do barrel roll once, don't keep repeating random check
                        if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_BARREL_ROLL))
                        {
                            bitSet(ship->tacticsTalk,TACTALK_BARRELROLL);
                            flightmanInit(ship,FLIGHTMAN_BARREL_ROLL);
                        }
                        else if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_CELEB_FLIP))
                        {
                            flightmanInit(ship,FLIGHTMAN_CELEB_FLIP);
                        }
                        else if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_SWARMER_BARRELROLL))
                        {
                            flightmanInit(ship,FLIGHTMAN_SWARMER_BARRELROLL);
                        }
                        else if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_ROLL180))
                        {
                            flightmanInit(ship,FLIGHTMAN_ROLL180);
                        }
                    }
                }

                if ((range < interceptorstat->breakRange[ship->tacticstype][targetIndex])) // || ((universe.totaltimeelapsed - spec->aitimekill) > interceptorstat->maxAttackTime[ship->tacticstype][targetIndex]))
                {
                    randeg = randombetween(FIGHTER_BREAK_ANGLE_MIN,FIGHTER_BREAK_ANGLE_MAX);
                    if (flightmanTestRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_AIP,FLIGHTMAN_BARRELROLL_OUT))
                    {
                        ship->aistateattack = STATE_BARRELROLL_OUT;
                        flightmanInit(ship,FLIGHTMAN_BARRELROLL_OUT);
                        randegf = (real32) randeg;  // always break left during barrelroll out
                    }
                    else
                    {
                        ship->aistateattack = STATE_BREAK;
                        ship->tacticsTalk = 0;  //reset variable

                        if (targetIndex == CLASS_Fighter)
                            randegf = (real32) randeg;  // always break left for fighters so they avoid each other better
                        else
                            randegf = (real32) ((randeg & 1) ? randeg : -randeg);
                    }
                    randegf = DEG_TO_RAD(randegf);

                    if ((ship->formationcommand) && (ship->tacticstype != Evasive))
                        formtype = ship->formationcommand->formation.formationtype;
                    else
                        formtype = NO_FORMATION;

                    if ((target->objtype == OBJ_ShipType) && (((Ship *)target)->shiptype == Mothership) && (((Ship *)target)->shiprace == R1) )
                    {
                        formtype = NO_FORMATION;            // yet another R1 Mothership special consideration
                    }

                    if ( (ABS(trajectory.z) >= 0.75) )
                    {
                        //vecScalarMultiply(tmpvec,trajectory,interceptorstat->flyPastDist[ship->tacticstype][targetIndex]);
                        vecScalarMultiply(tmpvec,trajectory,FAKE_FLY_BY_DISTANCE_MUCH_BIGGER_THAN_NEEDED);
                        matMakeRotAboutY(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
                        matMultiplyMatByVec(&spec->aivec,&tmpmat,&tmpvec);
                    }
                    else if ( (formtype == DELTA_FORMATION) || (formtype == BROAD_FORMATION) )
                    {
                        randeg = randombetween(FIGHTER_BREAK_VERTICAL_ANGLE_MIN,FIGHTER_BREAK_VERTICAL_ANGLE_MAX);
                        randegf = (real32) randeg;
                        randegf = DEG_TO_RAD(randegf);
                        tmpvec.x = trajectory.x;
                        tmpvec.y = trajectory.y;
                        tmpvec.z = 0.0f;
                        //vecNormalizeToLength(&tmpvec,interceptorstat->flyPastDist[ship->tacticstype][targetIndex]);
                        vecNormalizeToLength(&tmpvec,FAKE_FLY_BY_DISTANCE_MUCH_BIGGER_THAN_NEEDED);

                        if (ABS(trajectory.x) > ABS(trajectory.y))
                        {
                            matMakeRotAboutY(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
                        }
                        else
                        {
                            matMakeRotAboutX(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
                        }
                        matMultiplyMatByVec(&spec->aivec,&tmpmat,&tmpvec);
                        if (trajectory.z > 0.0f)
                        {
                            // attacking from below, so stay below
                            if ((spec->aivec.z) > 0.0f) spec->aivec.z = -spec->aivec.z;
                        }
                        else
                        {
                            // attacking from above, so stay above
                            if ((spec->aivec.z) < 0.0f) spec->aivec.z = -spec->aivec.z;
                        }
                    }
                    else
                    {
                        //vecScalarMultiply(tmpvec,trajectory,interceptorstat->flyPastDist[ship->tacticstype][targetIndex]);
                        vecScalarMultiply(tmpvec,trajectory,FAKE_FLY_BY_DISTANCE_MUCH_BIGGER_THAN_NEEDED);
                        matMakeRotAboutZ(&tmpmat,(real32)cos(randegf),(real32)sin(randegf));
                        matMultiplyMatByVec(&spec->aivec,&tmpmat,&tmpvec);
                    }


#ifdef DEBUG_TACTICS
                    if(tacticsOn)
#endif
                    {
                        //slowly reduce verticle component so as to organize
                        //things and reduce deadlyness of verticle attacks
                        //tempreal = vecMagnitudeSquared(spec->aivec);
                        //tempreal = fsqrt(tempreal);
                        //spec->aivec.x *= tacticsInfo.InterceptorVerticalMultiplier;
                        //vecNormalize(&spec->aivec);
                        //vecScalarMultiply(spec->aivec,spec->aivec,tempreal);
                    }

                    vecAddTo(spec->aivec,target->posinfo.position);
                }
            }
            break;

        case STATE_BARRELROLL_OUT:
            if(flightmanExecute(ship))
                ship->aistateattack = STATE_BREAK;
            break;

        case STATE_BREAK:
#ifdef DEBUG_AIATTACK
            dbgMessagef("\nShip %x STATE_BREAK",(udword)ship);
#endif
            aishipGetTrajectory(ship,target,&trajectory);
            dist = vecMagnitudeSquared(trajectory);

            curdistonly = TRUE;
            if(target->objtype == OBJ_ShipType)
            {
                if( ((ShipStaticInfo *)target->staticinfo)->shipclass <= CLASS_Frigate)
                {
                    //ship is a frigate..so lets base our fly away distance on current distance ONLY!
                    curdistonly = FALSE;
                }
            }
            if ((dist > interceptorstat->maxFlyAwayDistSquared[ship->tacticstype][targetIndex]) || ((MoveReachedDestinationVariable(ship,&spec->aivec,INTERCEPTORBREAK_TOLERANCE) && curdistonly)) )
            {
abortbreakafterall:
                matGetVectFromMatrixCol3(tmpvec,ship->rotinfo.coordsys);
                if (vecDotProduct(trajectory,tmpvec) > 0.0f)
                {
                    flighttodo = FLIGHTMAN_DONOTHING;
                }
                else
                {
                    flighttodo = flightmanGetRandom(&interceptorstat->flightmanProb[ship->tacticstype][targetIndex],FLIGHTMAN_TURNAROUND);
                    bitSet(ship->tacticsTalk,TACTALK_FLIPTURN);
                }
                //flightmanInit(ship,flighttodo);
                flightmanInitFunc(ship,flighttodo,0);
                ship->aistateattack = STATE_TURNAROUND;
            }
            else
            {
                if(!curdistonly)
				{
					vecSub(dest,spec->aivec,target->posinfo.position);
					vecNormalize(&dest);
					vecScalarMultiply(dest,dest,20000.0f);
					vecAddTo(dest,target->posinfo.position);
					if(aishipFlyToPointAvoidingObjs(ship,&dest,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FastAsPossible|AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY) & AISHIP_FLY_OBJECT_IN_WAY)
					{
						goto abortbreakafterall;
					}
				}
				else if (aishipFlyToPointAvoidingObjs(ship,&spec->aivec,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FastAsPossible|AISHIP_PointInDirectionFlying,INTERCEPTORBREAK_MINVELOCITY) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    goto abortbreakafterall;
                }
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

}

void CloakedFighterStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    CloakedFighterStatics *interceptorstat;

    GenericInterceptorStaticInit(directory,filename,statinfo);

    interceptorstat = (CloakedFighterStatics *)statinfo->custstatinfo;

    scriptSetStruct(directory,filename,CloakedFighterStaticScriptTable,(ubyte *)interceptorstat);

    //Set the fuel level at which ship decloaks and can no longer cloak
    interceptorstat->DeCloakFuelLevel = (interceptorstat->ForceDeCloakAtFuelPercent/100)*statinfo->maxfuel;
    interceptorstat->CloakingTime = 1 / interceptorstat->CloakingTime;
}


void CloakedFighterInit(Ship *ship)
{
    CloakedFighterSpec *spec = (CloakedFighterSpec *)ship->ShipSpecifics;
    GenericInterceptorInit(ship);
    spec->CloakingStatus = 1.0f;        //ship isn't cloaked
    spec->CloakLowWarning = FALSE;
    spec->ReCloak = FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : SpawnCloakingEffect
    Description : Create an effect for cloaking or decloaking
    Inputs      : ship - ship that is (de)cloaking
                  etgLOD - effect LOD
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SpawnCloakingEffect(Ship *ship, etglod *etgLOD)
{
    etgeffectstatic *stat;
    sdword LOD;

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
    if (stat != NULL && etgEffectsEnabled)
#else
    if (stat != NULL)
#endif
    {
        etgEffectCreate(stat, ship, NULL, NULL, NULL, ship->magnitudeSquared, EAF_Full, 3,
            ship->shiprace, ship->shiptype, ship->staticinfo->staticheader.staticCollInfo.collspheresize);
    }
}

bool CloakedFighterSpecialActivate(Ship *ship)
{
    CloakedFighterSpec *spec = (CloakedFighterSpec *)ship->ShipSpecifics;
    CloakedFighterStatics *cloakedfighterstatics;
    cloakedfighterstatics = (CloakedFighterStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    spec->ReCloak = FALSE;
    if(ship->fuel <= cloakedfighterstatics->DeCloakFuelLevel)
    {           //ships fuel is below critical level, so don't let it cloak
//        speechEvent(ship, COMM_Cloak_InsufficientPower, 0);
		if (battleCanChatterAtThisTime(BCE_CloakingInsufficientPower, ship))
		{
			battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingInsufficientPower, ship, SOUND_EVENT_DEFAULT);
		}
        return TRUE;
    }
/*
    if(bitTest(ship->flags,SOF_Cloaked) || bitTest(ship->flags,SOF_Cloaking))
    {
        bitSet(ship->flags,SOF_DeCloaking);     //begin DeCloaking Process;
        bitClear(ship->flags,SOF_Cloaking);     //clear in case we are cloaking right now
    }
    else
    {
        bitSet(ship->flags,SOF_Cloaking);       //begin Cloaking Process
        bitClear(ship->flags,SOF_DeCloaking);   //clear in case we are decloaking right now
    }
*/
    if(bitTest(ship->flags,SOF_Cloaked))
    {

        if(universe.curPlayerIndex != ship->playerowner->playerIndex)
        {
            //////////////////
            //speechevent ENEMY fighters decloaking
            //needs to be dependant on camera n' stuff!
            //event num: COMM_F_Grp_Enemy_Fighters_Decloaking
            //battle chatter
            // if (battleCanChatterAtThisTime(BCE_COMM_F_Grp_Enemy_Fighters_Decloaking, ship))
            //{
            //    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_Grp_Enemy_Fighters_Decloaking, ship, SOUND_EVENT_DEFAULT);
            //}
            //
        }
        bitSet(ship->flags,SOF_DeCloaking);     //begin DeCloaking Process;
        SpawnCloakingEffect(ship, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);
		soundEvent(ship, Ship_CloakOff);
    }
    else
    {
        bitSet(ship->flags,SOF_Cloaking);       //begin Cloaking Process
        SpawnCloakingEffect(ship, etgSpecialPurposeEffectTable[EGT_CLOAK_ON]);
		soundEvent(ship, Ship_CloakOn);
//        speechEvent(ship, COMM_Cloak_CloakingOn, 0);
		if (battleCanChatterAtThisTime(BCE_CloakingOn, ship))
		{
			battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingOn, ship, SOUND_EVENT_DEFAULT);
		}
    }
    return TRUE;
}


void CloakedFighterFire(Ship *ship,SpaceObjRotImpTarg *target)
{
    if(ship->shiptype == CloakedFighter)
    {
        if (bitTest(ship->flags,SOF_Cloaked) || bitTest(ship->flags,SOF_Cloaking))
        {             //ship is cloaked...make it decloak...
            bitClear(ship->flags,SOF_Cloaking);         //if bastard is trying to cloak,
            bitSet(ship->flags,SOF_DeCloaking);         //put an end to it pronto!
            ((CloakedFighterSpec *) ship->ShipSpecifics)->ReCloak = TRUE;
            ((CloakedFighterSpec *) ship->ShipSpecifics)->ReCloakTime = universe.totaltimeelapsed +
                ((CloakedFighterStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->battleReCloakTime;
        }
    }

    GenericInterceptorFire(ship,target);
}

void CloakedFighterHouseKeep(Ship *ship)
{
    CloakedFighterSpec *spec = (CloakedFighterSpec *)ship->ShipSpecifics;
    CloakedFighterStatics *cloakedfighterstatics;
    cloakedfighterstatics = (CloakedFighterStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(bitTest(ship->flags,SOF_Cloaking))           //ship is cloaking
    {
        //decrement Cloaking Status counter based on time elapsed
/***** calculated the inverse of CloakingTime in CloakedFighterStaticInit and multiply instead of divide *****/
        spec->CloakingStatus -= universe.phystimeelapsed*cloakedfighterstatics->CloakingTime;
        if(spec->CloakingStatus <= cloakedfighterstatics->VisibleState)
        {                                       //if ship is now past the vissible threshold
            bitSet(ship->flags,SOF_Cloaked);    //make it invisible to everything
            if(spec->CloakingStatus <= 0.0f)    //if it is completly invisible stop decloaking
            {
                spec->CloakingStatus = 0.0f;    //reset to 0.0
                bitClear(ship->flags,SOF_Cloaking); //stop ship from 'cloaking'
                //Since ship is cloaked, must remove it from being targeted
                //Note:  This probably will not let a person attack their own cloaked ships...

                shipHasJustCloaked(ship);
                //speechEvent(ship, COMM_Cloak_CloakingOn, 0);
                /*

                RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,FALSE);
                if(ship->playerowner != universe.curPlayerPtr)
                {           //if ship isn't players...remove it from camera stack
                    ccRemoveShip(&universe.mainCameraCommand,ship);
                }
                speechEvent(ship, COMM_Cloak_CloakingOn, 0);
                */
            }
        }
    }
    else if(bitTest(ship->flags,SOF_DeCloaking))    //Ship is decloaking
    {
        //Increment Cloaking Status counter based on time elapsed
/***** calculated the inverse of CloakingTime in CloakedFighterStaticInit and multiply instead of divide *****/
        spec->CloakingStatus += universe.phystimeelapsed*cloakedfighterstatics->CloakingTime;
        if(spec->CloakingStatus >= cloakedfighterstatics->VisibleState)
        {    //ship is 'visible' since it is past visible threshold
            if(ship->flags & SOF_Cloaked)
            {
                bitClear(ship->flags, SOF_Cloaked);
                ship->shipDeCloakTime = universe.totaltimeelapsed;
            }
            if(spec->CloakingStatus >= 1.0f)        //done decloaking
            {
                spec->CloakingStatus = 1.0f;
                bitClear(ship->flags,SOF_DeCloaking);
                spec->CloakLowWarning = FALSE;  //reset flag
                if(spec->ReCloak == FALSE)
                {   //only play speech when really decloaking for good
//                    speechEvent(ship, COMM_Cloak_Decloak, 0);
					if (battleCanChatterAtThisTime(BCE_Decloaking, ship))
					{
						battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Decloaking, ship, SOUND_EVENT_DEFAULT);
					}
                }
            }
        }
    }

    //Now, lets charge the guy/gal fuel for using the cloaking device
    if(bitTest(ship->flags,SOF_Cloaked) || bitTest(ship->flags,SOF_Cloaking))
    {               //Ship is cloaked or cloaking subtract fuel
        ship->fuel -= cloakedfighterstatics->CloakFuelBurnRate*universe.phystimeelapsed;
        if(ship->fuel <= cloakedfighterstatics->DeCloakFuelLevel)
        {                                           //if fuel reaches a certain level, DeCloak
            bitSet(ship->flags,SOF_DeCloaking);     //begin DeCloaking Process;
            bitClear(ship->flags,SOF_Cloaking);     //clear in case we are cloaking right now
            SpawnCloakingEffect(ship, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);
			soundEvent(ship, Ship_CloakOff);
        }
        if ((ship->fuel <= cloakedfighterstatics->CloakFuelBurnRate * 10.0f) && !spec->CloakLowWarning)
        {
            spec->CloakLowWarning = TRUE;
//            speechEvent(ship, STAT_Cloak_CloakPowerLow, (sdword)(ship->fuel / cloakedfighterstatics->CloakFuelBurnRate));
			if (battleCanChatterAtThisTime(BCE_CloakingPowerLow, ship))
			{
				battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingPowerLow, ship, (sdword)(ship->fuel / cloakedfighterstatics->CloakFuelBurnRate));
			}
        }
    }

    if(spec->ReCloak == TRUE)
    {
        if(universe.totaltimeelapsed > spec->ReCloakTime)
        {
            spec->ReCloak = FALSE;
            bitSet(ship->flags,SOF_Cloaking);
			soundEvent(ship, Ship_CloakOn);
        }
    }
}

bool InterceptorInRange(Ship *ship,SpaceObjRotImpTarg *target)
{
    GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)ship->staticinfo->custstatinfo;
    real32 range;
    uword targetIndex;
    vector trajectory;
    aishipGetTrajectory(ship,target,&trajectory);

    range = RangeToTarget(ship,target,&trajectory);

    if (target->objtype == OBJ_ShipType)
    {
        targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
    }
    else
    {
        targetIndex = (uword)NUM_CLASSES;
    }


    if ((range < interceptorstat->breakRange[ship->tacticstype][targetIndex]))
        return FALSE;
    //if((range > ship->staticinfo->bulletRange))
    //    return FALSE;
    return TRUE;
}

bool GenericInterceptorSpecialActivate(Ship *ship)
{
    if(ship->shiptype == LightInterceptor)
    {
        if(!bitTest(ship->specialFlags,SPECIAL_SpeedBurst))
        {
            bitSet(ship->specialFlags,SPECIAL_SpeedBurst);
            ship->speedBurstTime = universe.totaltimeelapsed+speedBurstDuration;
        }

    }
    return TRUE;
}

CustShipHeader GenericInterceptorHeader =
{
    (ShipType)-1,
    sizeof(GenericInterceptorSpec),
    GenericInterceptorStaticInit,
    NULL,
    GenericInterceptorInit,
    NULL,
    GenericInterceptorAttack,
    GenericInterceptorFire,
    GenericInterceptorPassiveAttack,
    GenericInterceptorSpecialActivate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader CloakedFighterHeader =
{
    (ShipType)-1,
    sizeof(CloakedFighterSpec),
    CloakedFighterStaticInit,
    NULL,
    CloakedFighterInit,
    NULL,
    GenericInterceptorAttack,
    CloakedFighterFire,
    GenericInterceptorPassiveAttack,
    CloakedFighterSpecialActivate,
    NULL,
    CloakedFighterHouseKeep,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader AttackBomberHeader =
{
    (ShipType)-1,
    sizeof(AttackBomberSpec),
    GenericInterceptorStaticInit,
    NULL,
    GenericInterceptorInit,
    NULL,
    GenericInterceptorAttack,
    GenericInterceptorFire,
    GenericInterceptorPassiveAttack,
    GenericInterceptorSpecialActivate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader TargetDroneHeader =
{
    (ShipType)-1,
    sizeof(GenericInterceptorSpec),
    GenericInterceptorStaticInit,
    NULL,
    GenericInterceptorInit,
    NULL,
    GenericInterceptorAttack,
    GenericInterceptorFire,
    NULL,
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


