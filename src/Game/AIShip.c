/*=============================================================================
    Name    : AIShip.c
    Purpose : This AI layer controls where a ship's velocity vector should be
              and what heading the ship should face.  It determines this based
              on commands received (parameters passed), and on where enemy
              ships are, etc.
              Note that it is the responsibility of the AItrack layer below
              to actually cause the ship to follow the desired velocity vector
              and heading.

    Created 6/26/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "FastMath.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "Physics.h"
#include "AITrack.h"
#include "AIShip.h"
#include "StatScript.h"
#include "Universe.h"
#include "Ships.h"
#include "UnivUpdate.h"
#include "Collision.h"
#include "Blobs.h"
#include "Tactics.h"
#include "Alliance.h"
#include "Randy.h"
#include "ProfileTimers.h"

#ifdef gshaw
#define DEBUG_AISHIP    0
#else
#define DEBUG_AISHIP    0
#endif

#define PAD_AROUND_SMALLEST_SHIP    1

#define VELFACTOR 0

bool isCapitalShipStaticOrBig(ShipStaticInfo *shipstatic);

/*=============================================================================
    AIShip Stats (optional)
=============================================================================*/

#ifdef AISHIP_STATS
AIshipStats aishipStats = { 0,0,0,0,0,0.0f, 0, {0.0f, 0.0f, 0.0f } };
#endif

/*=============================================================================
    Data
=============================================================================*/

bool aishipTempDisableAvoiding = FALSE;

/*=============================================================================
    Private Function declarations:
=============================================================================*/

static void scriptAIShipPrecalculate(char *directory,char *field,void *dataToFillIn);
static void rowSetDetails(char *directory,char *field,void *dataToFillIn);

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 VELOCITY_SCALE_FACTOR = 0.1f;
real32 URGENT_SCALE_FACTOR   = 1.0f;
real32 MISSILE_SCALE_FACTOR  = 1.0f;
real32 MISSILE_MIN_VELOCITY = 500.0f;
real32 MISSILE_USE_VELOCITYPRED_DISTANCE = 250.0f*250.0f;
//real32 MINE_MIN_VELOCITY = 75.0f;
real32 AVOID_OBJ_SCALE_FACTOR = (real32)1e-4;
real32 COMBAT_AVOID_OBJ_SCALE_FACTOR = (real32)1e-5;
real32 PROBE_AVOID_OBJ_SCALE_FACTOR = (real32)1e-5;
real32 AVOID_PANIC_FACTOR = 0.2f;
real32 AVOID_OBJ_PADDING_SCALE = 1.6f;
real32 AVOID_OBJ_PADDING_SCALE_BIG = 2.0f;
real32 AVOID_OBJ_ROW_PADDING_SCALE = 3.0f;
real32 AVOID_OBJ_FORMATION_PADDING_SCALE = 2.55f;

real32 AVOID_OBJ_MSHIP_MILPARADE_PADDING_SCALE = 1.55f;
real32 AVOID_OBJ_R2MSHIP_MILPARADE_PADDING_SCALE = 0.75f;

real32 AVOID_OBJ_OBSCUREDPOINT_SCALE = 1.55f;
real32 AVOID_SPREAD_OUT_DIST = 100.0f;
real32 CAR_TURN_POWER = 0.1f;
real32 FLYSHIP_HEADINGACCURACY = 0.999f;
real32 FLYSHIP_ATTACKACCURACY = 0.999f;
real32 FIGHTER_BANK = 0.99f;
real32 CORVETTE_BANK = 0.707f;
real32 FRIGATE_BANK = 0.5f;
real32 DESTROYER_BANK = 0.35f;
real32 MASSIVESHIP_BANK = 0.17f;
real32 NOSHIP_BANK = 0.01f;
real32 FRIGATE_TURNPITCH = DEG_TO_RAD(30.0f);
real32 DESTROYER_TURNPITCH = DEG_TO_RAD(25.0f);
real32 MASSIVESHIP_TURNPITCH = DEG_TO_RAD(20.0f);
real32 NOSHIP_TURNPITCH = DEG_TO_RAD(0.01f);
real32 FRIGATE_DESCENDPITCH = DEG_TO_RAD(20.0f);
real32 DESTROYER_DESCENDPITCH = DEG_TO_RAD(15.0f);
real32 MASSIVESHIP_DESCENDPITCH = DEG_TO_RAD(10.0f);
real32 NOSHIP_DESCENDPITCH = DEG_TO_RAD(0.01f);

real32 INTERCEPTORBREAK_TOLERANCE = 110.0f;
real32 INTERCEPTORBREAK_MINVELOCITY = -200.0f;
real32 INTERCEPTORKILL_MINVELOCITY = -200.0f;

real32 DESCEND_PITCH4_DIST = 0.25f;
real32 DESCEND_PITCH3_DIST = 0.50f;
real32 DESCEND_PITCH2_DIST = 0.75f;
real32 MIN_DIST_FOR_FANCY_DESCEND = 100.0f;
real32 MIN_ANGLE_FOR_FANCY_DESCEND = 0.577f;      // tan(30)

real32 MIN_DIST_TO_TRACK_HEADING = 20.0f;
real32 MAX_ANGLE_FOR_TRACK_HEADING = 2.747f;   // tan(70)

real32 ROW_GOING_MOSTLY_UPDOWN_ANGLE = 0.707f;
real32 AVOID_ROW_SHIP_BY_SCALE = 4.0f;
real32 AVOID_ROW_SHIP_BY_ADD = 200.0f;
real32 AVOID_ROW_R1MOTHERSHIP_BY = 1000.0f;
real32 ROW_GO_DOWN_INSTEAD_OF_UP_RATIO = 0.3f;

real32 PASS_RIGHT_FACTOR = 1.0f;
real32 USE_PASS_RIGHT_ANGLE = 0.97f;

real32 NEG_AVOID_MIN_VEL = -1.0f;
real32 AVOID_MIN_VEL = 1.0f;

real32 DONTUSEVELOCITYPRED_IFBELOW = 1000.0f;

bool DO_AVOID_OBJS = TRUE;


// pre-calculated constants:
real32 oneOverDESCEND_PITCH4_DIST;
real32 oneOverDESCEND_3_MINUS_4_DIST;
real32 oneOverDESCEND_2_MINUS_3_DIST;

scriptEntry AIShipTweaks[] =
{
    makeEntry(VELOCITY_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(URGENT_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(MISSILE_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(MISSILE_MIN_VELOCITY,scriptSetReal32CB),
    makeEntry(MISSILE_USE_VELOCITYPRED_DISTANCE,scriptSetReal32SqrCB),
    //makeEntry(MINE_MIN_VELOCITY,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(COMBAT_AVOID_OBJ_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(PROBE_AVOID_OBJ_SCALE_FACTOR,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_PADDING_SCALE_BIG,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_MSHIP_MILPARADE_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_R2MSHIP_MILPARADE_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_OBSCUREDPOINT_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_OBJ_ROW_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_PANIC_FACTOR,scriptSetReal32CB),
    makeEntry(AVOID_SPREAD_OUT_DIST,scriptSetReal32CB),
    makeEntry(CAR_TURN_POWER,scriptSetReal32CB),
    makeEntry(FLYSHIP_HEADINGACCURACY,scriptSetCosAngCB),
    makeEntry(FLYSHIP_ATTACKACCURACY,scriptSetCosAngCB),
    makeEntry(DO_AVOID_OBJS,scriptSetBool),
    makeEntry(CORVETTE_BANK,scriptSetSinAngCB),
    makeEntry(FRIGATE_BANK,scriptSetSinAngCB),
    makeEntry(DESTROYER_BANK,scriptSetSinAngCB),
    makeEntry(MASSIVESHIP_BANK,scriptSetSinAngCB),
    makeEntry(NOSHIP_BANK,scriptSetSinAngCB),
    makeEntry(FRIGATE_TURNPITCH,scriptSetAngCB),
    makeEntry(DESTROYER_TURNPITCH,scriptSetAngCB),
    makeEntry(MASSIVESHIP_TURNPITCH,scriptSetAngCB),
    makeEntry(NOSHIP_TURNPITCH,scriptSetAngCB),
    makeEntry(FRIGATE_DESCENDPITCH,scriptSetAngCB),
    makeEntry(DESTROYER_DESCENDPITCH,scriptSetAngCB),
    makeEntry(MASSIVESHIP_DESCENDPITCH,scriptSetAngCB),
    makeEntry(NOSHIP_DESCENDPITCH,scriptSetAngCB),
    makeEntry(DESCEND_PITCH4_DIST,scriptSetReal32CB),
    makeEntry(DESCEND_PITCH3_DIST,scriptSetReal32CB),
    makeEntry(DESCEND_PITCH2_DIST,scriptSetReal32CB),
    makeEntry(MIN_DIST_FOR_FANCY_DESCEND,scriptSetReal32CB),
    makeEntry(MIN_ANGLE_FOR_FANCY_DESCEND,scriptSetTanAngCB),
    makeEntry(MIN_DIST_TO_TRACK_HEADING,scriptSetReal32CB),
    makeEntry(MAX_ANGLE_FOR_TRACK_HEADING,scriptSetTanAngCB),
    makeEntry(INTERCEPTORBREAK_TOLERANCE,scriptSetReal32CB),
    makeEntry(INTERCEPTORBREAK_MINVELOCITY,scriptSetReal32CB),
    makeEntry(INTERCEPTORKILL_MINVELOCITY,scriptSetReal32CB),
    makeEntry(ROW_GOING_MOSTLY_UPDOWN_ANGLE,scriptSetCosAngCB),
    makeEntry(AVOID_ROW_SHIP_BY_SCALE,scriptSetReal32CB),
    makeEntry(AVOID_ROW_SHIP_BY_ADD,scriptSetReal32CB),
    makeEntry(AVOID_ROW_R1MOTHERSHIP_BY,scriptSetReal32CB),
    makeEntry(ROW_GO_DOWN_INSTEAD_OF_UP_RATIO,scriptSetReal32CB),
    makeEntry(PASS_RIGHT_FACTOR,scriptSetReal32CB),
    makeEntry(USE_PASS_RIGHT_ANGLE,scriptSetCosAngSqrCB),
    makeEntry(AVOID_MIN_VEL,scriptSetReal32CB),
    makeEntry(DONTUSEVELOCITYPRED_IFBELOW,scriptSetReal32CB),
    { "rowDetails", rowSetDetails, NULL },
    { "AISHIP_OTHER_CALCULATIONS", scriptAIShipPrecalculate, NULL },        // should go last
    endEntry
};

static void scriptAIShipPrecalculate(char *directory,char *field,void *dataToFillIn)
{
    oneOverDESCEND_PITCH4_DIST = 1.0f / DESCEND_PITCH4_DIST;
    oneOverDESCEND_3_MINUS_4_DIST = 1.0f / (DESCEND_PITCH3_DIST - DESCEND_PITCH4_DIST);
    oneOverDESCEND_2_MINUS_3_DIST = 1.0f / (DESCEND_PITCH2_DIST - DESCEND_PITCH3_DIST);
    dbgAssert(oneOverDESCEND_3_MINUS_4_DIST > 0.0f);
    dbgAssert(oneOverDESCEND_2_MINUS_3_DIST > 0.0f);
    NEG_AVOID_MIN_VEL = -AVOID_MIN_VEL;
}

#ifdef AISHIP_STATS
void aishipStatsInitFunc(Ship *ship)
{
    if (selSelected.numShips > 0 && (ship == selSelected.ShipPtr[0]))
    {
        memset(&aishipStats,0,sizeof(aishipStats));
        aishipStats.isValid = TRUE;
        aishipStats.isDisplayable = TRUE;
    }
}

void aishipStatsCloseFunc()
{
    aishipStats.isValid = FALSE;
}

void aishipStatsPrint(sdword *y)
{
    if (aishipStats.isDisplayable)
    {
        fontPrintf(0,*y += 20,colWhite, "AIShip state:%d walks:%d considered:%d",
                   aishipStats.state, aishipStats.avoidedWalks, aishipStats.avoidedConsidered);

        *y += 20;
        if (aishipStats.passingOnRight)
        fontPrintf(0,*y,colWhite, "AIShip passingonright:(%f %f %f)",
           aishipStats.passingOnRightVec.x,aishipStats.passingOnRightVec.y,aishipStats.passingOnRightVec.z);

        *y += 20;
        if (aishipStats.repulsenum)
        fontPrintf(0,*y,colWhite, "AIShip Repulse %d:(%f %f %f)",
           aishipStats.repulsenum, aishipStats.repulse.x, aishipStats.repulse.y, aishipStats.repulse.z);

        fontPrintf(0,*y += 20,colWhite,"distToColl:%f desiredVel:(%f %f %f)",
                   aishipStats.distanceToCollision,aishipStats.desiredVel.x,aishipStats.desiredVel.y,aishipStats.desiredVel.z);

        fontPrintf(0,*y += 20,colWhite,"desiredHeading:(%f %f %f) actualHeading:(%f %f %f)",
                   aishipStats.desiredHeading.x,aishipStats.desiredHeading.y,aishipStats.desiredHeading.z,aishipStats.actualHeading.x,aishipStats.actualHeading.y,aishipStats.actualHeading.z);
    }
}

#endif

#if 0       // obsolete
void aishipFlyToPoint(Ship *ship,vector *destination,udword aishipflags)
{
    vector desiredVel;
    vector desiredHead;

    vecSub(desiredVel,*destination,ship->posinfo.position);

    vecMultiplyByScalar(desiredVel,VELOCITY_SCALE_FACTOR);

    if (bitTest(aishipflags,AISHIP_PointInDirectionFlying))
    {
        vecCopyAndNormalize(&desiredVel,&desiredHead);
        aitrackHeading(ship,&desiredHead,FLYSHIP_HEADINGACCURACY);
    }
    aitrackVelocityVector(ship,&desiredVel);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : aishipPrecalcInfo
    Description : precalculates information used by the AIShip routines and puts
                  it in the ShipStaticInfo structure.
    Inputs      : shipstatinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aishipPrecalcInfo(ShipStaticInfo *shipstatinfo)
{
    shipstatinfo->sinbank = 0.0f;
    shipstatinfo->pitchturn = 0.0f;
    shipstatinfo->pitchdescend = 0.0f;

    switch (shipstatinfo->shipclass)
    {
        case CLASS_Fighter:
            shipstatinfo->sinbank = FIGHTER_BANK;
            break;

        case CLASS_Corvette:
            shipstatinfo->sinbank = CORVETTE_BANK;
            break;

        case CLASS_Frigate:
            shipstatinfo->sinbank = FRIGATE_BANK;
            shipstatinfo->pitchturn = FRIGATE_TURNPITCH;
            shipstatinfo->pitchdescend = FRIGATE_DESCENDPITCH;
            break;

        case CLASS_Destroyer:
            shipstatinfo->sinbank = DESTROYER_BANK;
            shipstatinfo->pitchturn = DESTROYER_TURNPITCH;
            shipstatinfo->pitchdescend = DESTROYER_DESCENDPITCH;
            break;

        case CLASS_Carrier:
        case CLASS_HeavyCruiser:
            shipstatinfo->sinbank = MASSIVESHIP_BANK;
            shipstatinfo->pitchturn = MASSIVESHIP_TURNPITCH;
            shipstatinfo->pitchdescend = MASSIVESHIP_DESCENDPITCH;
            break;

        default:
            switch (shipstatinfo->shiptype)
            {
                case SensorArray:
                    shipstatinfo->sinbank = NOSHIP_BANK;
                    shipstatinfo->pitchturn = NOSHIP_TURNPITCH;
                    shipstatinfo->pitchdescend = NOSHIP_DESCENDPITCH;
                    break;
                case CryoTray:
                case P1Mothership:
                case P2Mothership:
                case P3Megaship:
                case FloatingCity:
                case ResearchStation:
                case Ghostship:
                    shipstatinfo->sinbank = MASSIVESHIP_BANK;
                    shipstatinfo->pitchturn = MASSIVESHIP_TURNPITCH;
                    shipstatinfo->pitchdescend = MASSIVESHIP_DESCENDPITCH;
                    break;
                case ResourceController:
                    shipstatinfo->sinbank = FRIGATE_BANK;
                    shipstatinfo->pitchturn = FRIGATE_TURNPITCH;
                    shipstatinfo->pitchdescend = FRIGATE_DESCENDPITCH;
                    break;
            }
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getPitchToDescend
    Description : returns the pitch in radians the ship should be at, given the
                  normalized distance left zdistanceleft, and pitchdescend (maximum
                  pitch change)
    Inputs      : zdistanceleft, pitchdescend
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 getPitchToDescend(real32 zdistanceleft,real32 pitchdescend)
{
    if (zdistanceleft < 0.0f)
    {
        zdistanceleft = -zdistanceleft;

        if (zdistanceleft < 0.0f)
        {
            return 0.0f;
        }
        else if (zdistanceleft < DESCEND_PITCH4_DIST)       // 0 to DESCEND_PITCH4_DIST
        {
            return pitchdescend * zdistanceleft * oneOverDESCEND_PITCH4_DIST;
        }
        else if (zdistanceleft < DESCEND_PITCH3_DIST)       // DESCEND_PITCH4_DIST to DESCEND_PITCH3_DIST
        {
            return pitchdescend * (DESCEND_PITCH3_DIST - zdistanceleft) * oneOverDESCEND_3_MINUS_4_DIST;
        }
        else if (zdistanceleft < DESCEND_PITCH2_DIST)       // DESCEND_PITCH3_DIST to DESCEND_PITCH2_DIST
        {
            return -pitchdescend * (zdistanceleft - DESCEND_PITCH3_DIST) * oneOverDESCEND_2_MINUS_3_DIST;
        }
        else                                                // DESCEND_PITCH2_DIST +
        {
            return -pitchdescend;
        }
    }
    else
    {
        if (zdistanceleft < 0.0f)
        {
            return 0.0f;
        }
        else if (zdistanceleft < DESCEND_PITCH4_DIST)       // 0 to DESCEND_PITCH4_DIST
        {
            return -pitchdescend * zdistanceleft * oneOverDESCEND_PITCH4_DIST;
        }
        else if (zdistanceleft < DESCEND_PITCH3_DIST)       // DESCEND_PITCH4_DIST to DESCEND_PITCH3_DIST
        {
            return -pitchdescend * (DESCEND_PITCH3_DIST - zdistanceleft) * oneOverDESCEND_3_MINUS_4_DIST;
        }
        else if (zdistanceleft < DESCEND_PITCH2_DIST)       // DESCEND_PITCH3_DIST to DESCEND_PITCH2_DIST
        {
            return pitchdescend * (zdistanceleft - DESCEND_PITCH3_DIST) * oneOverDESCEND_2_MINUS_3_DIST;
        }
        else                                                // DESCEND_PITCH2_DIST +
        {
            return pitchdescend;
        }
    }
}

void aishipGetTrajectoryWithVelPrediction(Ship *ship,SpaceObjRotImpTarg *target,real32 bulletspeed,vector *trajectory)
{
    vector distToTargetVec;
    real32 distToTargetMag;
    real32 bullettraveltime;
    vector deltatarget;

    vecSub(distToTargetVec,target->collInfo.collPosition,ship->posinfo.position); // MUST be relative to ship->posinfo.position, because
                                                                                  // later it is modified into the ship's coordinate system
                                                                                  // for gun firing, which is based on ship->posinfo.position
    if (bulletspeed <= DONTUSEVELOCITYPRED_IFBELOW)
    {
        // don't bother with velocity prediction for 0 bulletspeed
        goto settrajectory;
    }

    distToTargetMag = fsqrt(vecMagnitudeSquared(distToTargetVec));

    bullettraveltime = distToTargetMag / bulletspeed;

    vecScalarMultiply(deltatarget,target->posinfo.velocity,bullettraveltime);

    vecAddTo(distToTargetVec,deltatarget);

settrajectory:
    *trajectory = distToTargetVec;
}

void aishipGetTrajectory(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory)
{
    vecSub(*trajectory,target->collInfo.collPosition,ship->posinfo.position);
}

void aishipGetTrajectoryWithAngleCorrection(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory)
{
    vector futurepos = ship->posinfo.position;

    futurepos.x += (ship->posinfo.velocity.x * 0.25f);
    futurepos.y += (ship->posinfo.velocity.y * 0.25f);
    futurepos.z += (ship->posinfo.velocity.z * 0.25f);

    vecSub(*trajectory,target->collInfo.collPosition,futurepos);
}

#define STATE_POINT_IN_DIRECTION    1
#define STATE_START_FLYING          2

real32 GetCollSizeInDirection(SpaceObjRotImp *obj,vector dir)
{
    vector up;
    vector right;
    vector heading;
    real32 upcomp,rightcomp,headingcomp;
    StaticCollInfo *collInfo = &obj->staticinfo->staticheader.staticCollInfo;

    matGetVectFromMatrixCol1(up,obj->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,obj->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,obj->rotinfo.coordsys);

    upcomp = ABS(vecDotProduct(up,dir));
    rightcomp = ABS(vecDotProduct(right,dir));
    headingcomp = ABS(vecDotProduct(heading,dir));

    return (upcomp*collInfo->uplength + rightcomp*collInfo->rightlength + headingcomp*collInfo->forwardlength)*0.5f;
}

/*-----------------------------------------------------------------------------
    Name        : rowSetDetails
    Description : sets right of way details for a ship (callback function for statscript)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void rowSetDetails(char *directory,char *field,void *dataToFillIn)
{
    char shipstr[50];
    sdword rowpri;
    ShipType shiptype;
    real32 rowAvoidByVal;

    RemoveCommasFromString(field);

    sscanf(field,"%s %d %f",shipstr,&rowpri,&rowAvoidByVal);

    shiptype = StrToShipType(shipstr);
    if ((shiptype < 0) || (shiptype >= TOTAL_NUM_SHIPS))
    {
        dbgFatalf(DBG_Loc,"Invalid shiptype read at line %s in aiship.script",field);
    }

    rightOfWays[shiptype] = (sbyte)rowpri;
    rowAvoidBy[shiptype] = rowAvoidByVal;
}

/*-----------------------------------------------------------------------------
    Name        : GetDirectionOfTravel
    Description : gets the direction of travel of ship
                  bases direction of travel based on where it's moving to if it's got a move command, otherwise
                  bases direction of travel on velocity of ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GetDirectionOfTravel(Ship *ship,vector *dirOfTravel)
{
    if (!vecIsZero(ship->moveTo))
    {
        vecSub(*dirOfTravel,ship->moveTo,ship->moveFrom);
    }
    else
    {
        *dirOfTravel = ship->posinfo.velocity;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rowRemoveShipFromGettingOutOfWay
    Description : removes getting out of way status from ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rowRemoveShipFromGettingOutOfWay(Ship *ship)
{
    if (!(ship->specialFlags & SPECIAL_rowGettingOutOfWay))
    {
        return;
    }

    bitClear(ship->specialFlags,SPECIAL_rowGettingOutOfWay);
    //bitClear(ship->dontrotateever,2);
    bitClear(ship->dontapplyforceever,2);

    ship->rowGetOutOfWay = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : rowGetOutOfWayShipDiedCB
    Description : callback function notify for when rowGetOutOfWay ship dies
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rowGetOutOfWayShipDiedCB(Ship *ship)
{
    ;
}

#define ROW_CAN_GET_OUT_OF_WAY      1
#define ROW_CANNOT_GET_OUT_OF_WAY   0
#define ROW_YOU_STILL_AVOID_ME      -1

/*-----------------------------------------------------------------------------
    Name        : rowShipCanGetOutOfWayOfMe
    Description : returns TRUE if the ship can get out of the way of me
    Inputs      : ship, me
    Outputs     :
    Return      : returns TRUE if the ship can get out of the way of me
----------------------------------------------------------------------------*/
sdword rowShipCanGetOutOfWayOfMe(Ship *ship,Ship *me)
{
    CommandToDo *shipcommand = getShipAndItsCommand(&universe.mainCommandLayer,ship);
    CommandToDo *mecommand;

    if (ship->shiptype == Drone)
    {
        return ROW_CANNOT_GET_OUT_OF_WAY;
    }

    if (shipcommand == NULL)
    {                   // ship not doing anything, so it can get out of way
        return ROW_CAN_GET_OUT_OF_WAY;
    }

    mecommand = getShipAndItsCommand(&universe.mainCommandLayer,me);

    if (shipcommand == mecommand)
    {
        return ROW_YOU_STILL_AVOID_ME;       // if ship and me are in the same command, we shouldn't use right of way
    }

    if (shipcommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        return ROW_CANNOT_GET_OUT_OF_WAY;       // ships that are protecting are avoiding stuff anyway, shouldn't use right of way
    }

    if (mecommand && (mecommand->ordertype.attributes & COMMAND_IS_PROTECTING))
    {
        return ROW_CANNOT_GET_OUT_OF_WAY;       // ships that are protecting are avoiding stuff anyway, shouldn't use right of way
    }

    switch (shipcommand->ordertype.order)
    {
        case COMMAND_NULL:
        case COMMAND_HALT:
            return ROW_CAN_GET_OUT_OF_WAY;

        case COMMAND_MILITARYPARADE:
            if (shipcommand->militaryParade->aroundShip == me)
            {
                return ROW_CANNOT_GET_OUT_OF_WAY;       // ships in my parade shouldn't try to avoid me!
            }
            return ROW_CAN_GET_OUT_OF_WAY;

        case COMMAND_MOVE:
        case COMMAND_DOCK:
        case COMMAND_ATTACK:
        case COMMAND_SPECIAL:
            return ROW_CANNOT_GET_OUT_OF_WAY;

        default:
            return ROW_CANNOT_GET_OUT_OF_WAY;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rowOriginalPointIsClear
    Description : returns TRUE if original point is clear
                  (right-of-way ship we're getting out way of is not near it and
                   right-of-way ship is not heading in our direction)
    Inputs      :
    Outputs     :
    Return      : returns TRUE if original point is clear
----------------------------------------------------------------------------*/
bool rowOriginalPointIsClear(Ship *ship)
{
    real32 avoidcollsize;
    real32 avoidcollpad;
    vector destToObj;
    real32 destToObjMag2;
    Ship *avoidme;

    vector meToShip;
    vector meDirOfTravel;

    dbgAssert(ship->specialFlags & SPECIAL_rowGettingOutOfWay);

    avoidme = ship->rowGetOutOfWay;
    if (avoidme == NULL)
    {
        return TRUE;
    }

    GetDirectionOfTravel(avoidme,&meDirOfTravel);

    vecSub(meToShip,ship->collInfo.collPosition,avoidme->collInfo.collPosition);

    if (vecDotProduct(meToShip,meDirOfTravel) > 0.0f)
    {
        // right-of-way ship (avoidme) is still heading in our direction, must wait till it is not
        return FALSE;
    }

    avoidcollsize = avoidme->staticinfo->staticheader.staticCollInfo.avoidcollspheresize;
    avoidcollpad = avoidme->staticinfo->staticheader.staticCollInfo.avoidcollspherepad;

    vecSub(destToObj,avoidme->collInfo.collPosition,ship->rowOriginalPoint);
    destToObjMag2 = vecMagnitudeSquared(destToObj);

    if (destToObjMag2 < (avoidcollsize*avoidcollsize))
    {
        return FALSE;
    }

    // later check all ships in nearest blob too?

    return TRUE;
}

#define rowHANG_AROUND_TILL_SHIP_OUT_OF_WAY     1
#define rowFLY_BACK_TO_ORIGINAL_POINT           2

/*-----------------------------------------------------------------------------
    Name        : rowFlyShipOutOfWay
    Description : flys ship out of way
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rowFlyShipOutOfWay(Ship *ship)
{
    dbgAssert(ship->specialFlags & SPECIAL_rowGettingOutOfWay);

    //bitClear(ship->dontrotateever,2);
    bitClear(ship->dontapplyforceever,2);

    switch (ship->rowState)
    {
        case 0:     // fly to rowOutOfWayPoint
            if (ship->rowGetOutOfWay == NULL)
            {
                ship->rowState = rowFLY_BACK_TO_ORIGINAL_POINT;
                break;
            }
            if (rowOriginalPointIsClear(ship))
            {
                ship->rowState = rowFLY_BACK_TO_ORIGINAL_POINT;
                break;
            }
            if (MoveReachedDestinationVariable(ship,&ship->rowOutOfWayPoint,50.0f))
            {
                ship->rowState = rowHANG_AROUND_TILL_SHIP_OUT_OF_WAY;
                break;
            }
            aishipFlyToPointAvoidingObjs(ship,&ship->rowOutOfWayPoint,0,0.0f);
            break;

        case rowHANG_AROUND_TILL_SHIP_OUT_OF_WAY:
            if (ship->rowGetOutOfWay == NULL)
            {
                ship->rowState = rowFLY_BACK_TO_ORIGINAL_POINT;
                break;
            }
            if (rowOriginalPointIsClear(ship))
            {
                ship->rowState = rowFLY_BACK_TO_ORIGINAL_POINT;
                break;
            }
            // just hover around here
            aishipFlyToPointAvoidingObjs(ship,NULL,0,0.0f);
            break;

        case rowFLY_BACK_TO_ORIGINAL_POINT:
            if (MoveReachedDestinationVariable(ship,&ship->rowOriginalPoint,50.0f))
            {
                rowRemoveShipFromGettingOutOfWay(ship);     // we're done!
                return;
            }
            if (aishipFlyToPointAvoidingObjs(ship,&ship->rowOriginalPoint,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,0.0f) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                rowRemoveShipFromGettingOutOfWay(ship);     // we're done!
                return;
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    //bitSet(ship->dontrotateever,2);
    bitSet(ship->dontapplyforceever,2);
}

/*-----------------------------------------------------------------------------
    Name        : rowSignalShipToGetOutOfWayOfMe
    Description : tells ship to get out of the way of me
    Inputs      : ship, me
                  me is the right of way ship (get out of way of me)
                  ship is avoiding ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rowSignalShipToGetOutOfWayOfMe(Ship *ship,Ship *me,vector *meToShip,vector *meDirOfTravel)
{
    vector delta;

    ship->specialFlags |= SPECIAL_rowGettingOutOfWay;
    //bitSet(ship->dontrotateever,2);
    bitSet(ship->dontapplyforceever,2);

    ship->rowState = 0;
    ship->rowGetOutOfWay = me;
    ship->rowOriginalPoint = ship->posinfo.position;

    // calculate outOfWayPoint;

    if ((me->shiprace == R1) && (me->shiptype == Mothership))
    {
        // special case solution for R1 Mothership
        // under no circumstances do we want ships going up or down to avoid the R1 Mothership!

        vector meToShipNorm = *meToShip;
        vector meDirOfTravelNorm = *meDirOfTravel;
        vector directionOfParting;
        vector up = { 0.0f, 0.0f, 1.0f };
        vector test;

        meToShipNorm.z = 0.0f;
        meDirOfTravelNorm.z = 0.0f;
        vecNormalize(&meToShipNorm);
        vecNormalize(&meDirOfTravelNorm);

        // find direction ships should part in (should be on x-y plane and perpendicular to direction of movement of mothership)
        vecCrossProduct(directionOfParting,meDirOfTravelNorm,up);

        // now find out if we should go in directionOfParting or -directionOfParting
        vecCrossProduct(test,meDirOfTravelNorm,meToShipNorm);
        if (test.z > 0.0f)
        {
            vecNegate(directionOfParting);
        }

        vecScalarMultiply(delta,directionOfParting,AVOID_ROW_R1MOTHERSHIP_BY);
    }
    else
    {
        vector meDirOfTravelNorm = *meDirOfTravel;
        vector meToShipNorm;
        real32 avoidby = rowAvoidBy[me->shiptype];

        if (avoidby == 0.0f) avoidby = me->staticinfo->staticheader.staticCollInfo.avoidcollspheresize*AVOID_ROW_SHIP_BY_SCALE + AVOID_ROW_SHIP_BY_ADD;

        vecNormalize(&meDirOfTravelNorm);

        if ((meDirOfTravelNorm.z > ROW_GOING_MOSTLY_UPDOWN_ANGLE) || (meDirOfTravelNorm.z < -ROW_GOING_MOSTLY_UPDOWN_ANGLE))
        {
            // ship we are trying to avoid is going mainly up/down, so we should move on x-y plane.
            vector directionOfParting;
            vector up = { 0.0f, 0.0f, 1.0f };
            vector test;

            meToShipNorm = *meToShip;
            meDirOfTravelNorm = *meDirOfTravel;

            meToShipNorm.z = 0.0f;
            meDirOfTravelNorm.z = 0.0f;
            vecNormalize(&meToShipNorm);
            vecNormalize(&meDirOfTravelNorm);

            // find direction ships should part in (should be on x-y plane and perpendicular to direction of movement of me)
            vecCrossProduct(directionOfParting,meDirOfTravelNorm,up);

            // now find out if we should go in directionOfParting or -directionOfParting
            vecCrossProduct(test,meDirOfTravelNorm,meToShipNorm);
            if (test.z > 0.0f)
            {
                vecNegate(directionOfParting);
            }

            vecScalarMultiply(delta,directionOfParting,avoidby);
        }
        else
        {
            // move up or down to get out of way

            if ((-meToShip->z) > (avoidby*ROW_GO_DOWN_INSTEAD_OF_UP_RATIO)) // if me is above ship by this let's go down
            {
                // go down
                vecSet(delta,0.0f,0.0f,-avoidby);
            }
            else
            {
                // go up
                vecSet(delta,0.0f,0.0f,avoidby);
            }
        }
    }

    vecAdd(ship->rowOutOfWayPoint,ship->posinfo.position,delta);
}

/*-----------------------------------------------------------------------------
    Name        : rowSignalLeaderInFormationToGetOutOfWayOfMe
    Description : tells other ships in ship's formation to get out of the way of me
    Inputs      : ship, me
                  me is the right of way ship (get out of way of me)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rowSignalLeaderInFormationToGetOutOfWayOfMe(Ship *ship,Ship *me,vector *meToShip,vector *meDirOfTravel)
{
    SelectCommand *selection = ship->formationcommand->selection;
    //sdword numShips;
    //sdword i;
    Ship *shipi;

    dbgAssert(selection);
    dbgAssert(selection->numShips > 0);
    //numShips = selection->numShips;

    shipi = selection->ShipPtr[0];
    if (!(shipi->specialFlags & SPECIAL_rowGettingOutOfWay))    // not already getting out of way
    {
        rowSignalShipToGetOutOfWayOfMe(shipi,me,meToShip,meDirOfTravel);
    }

#if 0
    for (i=0;i<numShips;i++)
    {
        shipi = selection->ShipPtr[i];
        if (shipi != ship)
        {
            if (!(shipi->specialFlags & SPECIAL_rowGettingOutOfWay))    // not already getting out of way
            {
                rowSignalShipToGetOutOfWayOfMe(shipi,me,meToShip,meDirOfTravel);
            }
        }
    }
#endif
}

udword aishipFlyToPointAvoidingObjsFunc(Ship *ship,vector *destination,udword aishipflags,real32 limitvel,vector *withVel)
{
    vector desiredVel;
    vector desiredHead;
    vector targetVec;
    sdword objindex;
    SpaceObjRotImp *avoidobj = NULL;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    blob *thisBlob = NULL;
    SelectAnyCommand *targetsAvoid = NULL;

    vector repulse;
    vector repulsenorm;
    real32 repulsedist;
    real32 repulsemag2;
#if VELFACTOR
    real32 velfactor;
#endif
    real32 collidedist;
    real32 distanceToCollision;

    real32 testExactOpposite;
    real32 shipcollsize,shipcollpad,shiprowcollpad;
    real32 avoidcollsize = 0,avoidcollpad = 0;

    real32 maxavoidcollsize,maxavoidcollpad;

    real32 maxdistconsider;
    real32 temp;

    real32 zdistanceleft;
    real32 pitchtodescend;

    udword returnflag = 0;

#ifndef HW_Release
    udword considered = 0;
#endif

    sdword pass;
    sdword bottom,top;
    sdword numTargets;

    real32 distcheck;
    real32 maxpossibledist;

    real32 obscureaddcollsize = 0;

    real32 destToObjDist;
    vector destToObjNorm;

    bool rowEnabled;

    CommandToDo *shipcommand = ship->command;

    real32 useAVOID_OBJ_SCALE_FACTOR = AVOID_OBJ_SCALE_FACTOR;

    SpaceObjRotImpTarg *takeOutInMyWay = NULL;

    if (ship->dontapplyforceever)
    {
        return 0;     // no point wasting our time
    }

    if (ship->staticinfo->staticheader.immobile)
    {
        return 0;     // no point wasting our time
    }

    PTSTART(2);

    if (isStrikeCraft(ship->shiptype))
    {
        if ((shipcommand) && (shipcommand->ordertype.order == COMMAND_ATTACK))
        {
            useAVOID_OBJ_SCALE_FACTOR = COMBAT_AVOID_OBJ_SCALE_FACTOR;
        }
    }
    else if (ship->shiptype == Probe)
    {
        if (((ProbeSpec *)ship->ShipSpecifics)->HaveMoved)
        {
            useAVOID_OBJ_SCALE_FACTOR = PROBE_AVOID_OBJ_SCALE_FACTOR;
        }
    }

    aishipStatsInit(ship);

    aishipStatsState(ship->aistate);

    switch (ship->aistate)
    {
/*=============================================================================
    State 0
=============================================================================*/
        case 0:
            if (shipstaticinfo->pitchdescend != 0.0f)
            {
                if (destination)
                {
                    real32 xdistanceleft = (destination->x - ship->posinfo.position.x);
                    real32 ydistanceleft = (destination->y - ship->posinfo.position.y);
                    real32 r;
                    zdistanceleft = ABS(destination->z - ship->posinfo.position.z);
                    r = fsqrt(xdistanceleft*xdistanceleft + ydistanceleft*ydistanceleft);
                    if ((zdistanceleft < MIN_DIST_FOR_FANCY_DESCEND) ||
                        (zdistanceleft < (r*MIN_ANGLE_FOR_FANCY_DESCEND)))
                    {
                        ship->aidescend = 0.0f;                     // set to 0 to indicate no fancy descend
                    }
                    else
                    {
                        ship->aidescend = 1.0f / zdistanceleft;     // used to normalize distance left to go in descend

                        if ((r < MIN_DIST_TO_TRACK_HEADING) ||
                            (zdistanceleft > (r*MAX_ANGLE_FOR_TRACK_HEADING)))
                        {
                            ship->aidescend = -ship->aidescend;     // -ve aidescend indicates don't track heading as well
                        }
                    }
                }
                else
                {
                    ship->aidescend = 0.0f;                         // set to 0 to indicate no fancy descend
                }
            }
            ship->aistate = STATE_POINT_IN_DIRECTION;
            // deliberately fall through to STATE_POINT_IN_DIRECTION

        case STATE_POINT_IN_DIRECTION:
            if (bitTest(aishipflags,(AISHIP_FirstPointInDirectionFlying+AISHIP_CarTurn)))
            {
        #if DEBUG_AISHIP
                 dbgMessagef("\nOrienting ship for flight");
        #endif
                dbgAssert(destination);     // must have destination for AISHIP_FirstPointInDirectionFlying or AISHIP_CarTurn to have any meaning
                vecSub(targetVec,*destination,ship->posinfo.position);

                vecNormalize(&targetVec);

                if (shipstaticinfo->pitchdescend == 0.0f)
                {
                    if (aitrackHeadingWithBankFlags(ship,&targetVec,0.96f,shipstaticinfo->sinbank,AITRACKHEADING_IGNOREUPVEC))
                    {
                        ship->aistate = STATE_START_FLYING;
                        aishipStatsClose();
                        PTEND(2);
                        return 0;
                    }
                }
                else
                {
                    if ((ship->aidescend < 0.0f) || (aitrackHeadingWithBankPitchFlags(ship,&targetVec,0.96f,shipstaticinfo->sinbank,0.0f,shipstaticinfo->pitchturn,AITRACKHEADING_IGNOREUPVEC)))
                    {
                        ship->aistate = STATE_START_FLYING;
                        aishipStatsClose();
                        PTEND(2);
                        return 0;
                    }
                }

                if (bitTest(aishipflags,AISHIP_CarTurn) && (!isCapitalShipStaticOrBig(shipstaticinfo)))
                {
                    physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*CAR_TURN_POWER,TRANS_FORWARD);
                }
                else
                {
                    aitrackSteadyShipDriftOnly(ship);
                }
            }
            else
            {
                ship->aistate = STATE_START_FLYING;
                goto startflying;
            }
            aishipStatsClose();
            PTEND(2);
            return 0;

/*=============================================================================
    State START_FLYING
=============================================================================*/
startflying:
        case STATE_START_FLYING:

            if (destination)
            {
                vecSub(targetVec,*destination,ship->posinfo.position);

                if ((aishipflags & AISHIP_FastAsPossible) == 0)
                {
                    vecMultiplyByScalar(targetVec,VELOCITY_SCALE_FACTOR);
                }
                // else assumes URGENT_SCALE_FACTOR is 1!
            }
            else
            {
                vecZeroVector(targetVec);
            }

            if ((limitvel == 0.0f) && ship->formationcommand && (ship == shipcommand->selection->ShipPtr[0]))
            {
                // ship is leader of formation, it may potentially travel slower so other ships can maintain formation

                if ((shipcommand->formation.formationtype != SPHERE_FORMATION) ||
                    (!((shipcommand->ordertype.attributes & COMMAND_IS_PROTECTING) || (shipcommand->formation.enders)))
                   )
                {
                    limitvel = shipcommand->formation.travelvel;
                }
            }

            if (limitvel < 0.0f)
            {
                vecCapMinVector(&targetVec,-limitvel);
            }

            if (withVel != NULL)
            {
                vecAddTo(targetVec,*withVel);
            }

            if (limitvel > 0.0f)
            {
                vecCapVector(&targetVec,limitvel);
            }
            else
            {
                real32 velMult = tacticsGetShipsMaxVelocity(ship);
                vecCapVector(&targetVec,velMult);
            }

        #if DEBUG_AISHIP
            dbgMessagef("\nFlying to target vector: (%f %f %f)",targetVec.x,targetVec.y,targetVec.z);
        #endif

            desiredVel = targetVec;

            if (aishipTempDisableAvoiding)
            {
                goto noavoid;
            }

#ifndef HW_Release
            if (!DO_AVOID_OBJS)
            {
                goto noavoid;
            }
#endif

            if (ship->dockingship != NULL)       // don't avoid any objects if docking
            {
                goto noavoid;
            }

            //don't avoid our target if we are in kamikaze mode
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                goto noavoid;
            }

#ifdef COLLISION_CHECK_STATS
            shipsavoidingstuff++;
#endif

            shipcollsize = shipstaticinfo->staticheader.staticCollInfo.avoidcollspheresize;
            shipcollpad = shipstaticinfo->staticheader.staticCollInfo.avoidcollspherepad;

            rowEnabled = FALSE;

            if (shipstaticinfo->staticheader.rightOfWay > 0)    // if ship has any right of way
            {
                shiprowcollpad = shipcollsize * AVOID_OBJ_ROW_PADDING_SCALE;
                rowEnabled = TRUE;
            }
            else
                shiprowcollpad = shipcollpad;

            thisBlob = ship->collMyBlob;
            if (thisBlob == NULL)
            {
                dbgMessagef("\nWarning: flags %x ship %s %s flying with no blob",ship->flags,ShipRaceToStr(ship->shiprace),ShipTypeToStr(ship->shiptype));
                goto noavoid;
            }

            pass = 0;

passagain:
            if (pass == 0)
            {
                targetsAvoid = thisBlob->blobSmallTargets;
                dbgAssert(targetsAvoid);
                numTargets = targetsAvoid->numTargets;
                if (numTargets == 0)
                {
                    goto nextpass;
                }
                maxavoidcollsize = thisBlob->blobMaxSTargetCollSphereSize;
                maxavoidcollpad = maxavoidcollsize * AVOID_OBJ_PADDING_SCALE;
                maxpossibledist = shiprowcollpad + maxavoidcollpad;

                // binary search targetindex such that
                // (ship->collOptimizeDist - avoidobj->collOptimizeDist) <= maxpossibledist
                bottom = 0;
                top = numTargets-1;
                while (top >= bottom)
                {
                    objindex = (bottom+top)>>1;
                    if ((ship->collOptimizeDist - targetsAvoid->TargetPtr[objindex]->collOptimizeDist) > maxpossibledist)
                    {
                        // targetindex is too small
                        bottom = objindex+1;
                    }
                    else
                    {
                        top = objindex-1;
                    }
                }

                dbgAssert(objindex >= 0);
                dbgAssert(objindex < numTargets);
            }
            else
            {
                dbgAssert(pass == 1);
                targetsAvoid = thisBlob->blobBigTargets;
                dbgAssert(targetsAvoid);
                numTargets = targetsAvoid->numTargets;
                if (numTargets == 0)
                {
                    goto nextpass;
                }
                maxavoidcollsize = thisBlob->blobMaxBTargetCollSphereSize;
                maxavoidcollpad = maxavoidcollsize * ((maxavoidcollsize > AISHIP_BIGSHIP) ? AVOID_OBJ_PADDING_SCALE_BIG : AVOID_OBJ_PADDING_SCALE);
                maxpossibledist = shiprowcollpad + maxavoidcollpad;
                objindex = 0;
            }

            while (objindex < numTargets)
            {
                avoidobj = (SpaceObjRotImp *) targetsAvoid->TargetPtr[objindex];

#ifdef COLLISION_CHECK_STATS
                shipsavoidedwalks++;
#endif
                aishipStatsAvoidedWalks();

                if ((distcheck = (avoidobj->collOptimizeDist - ship->collOptimizeDist)) > maxpossibledist)
                {
                    goto nextpass;
                }

                // use more trivial rejections?

                avoidcollpad = avoidobj->staticinfo->staticheader.staticCollInfo.avoidcollspherepad;

                maxdistconsider = avoidcollpad + shiprowcollpad;

                if (ABS(distcheck) > maxdistconsider)
                {
                    goto nextnode;
                }

                vecSub(repulse,ship->collInfo.collPosition,avoidobj->collInfo.collPosition);

#ifdef COLLISION_CHECK_STATS
                shipsavoidedchecks++;
#endif

                if (!rowEnabled)
                {
                    goto norowcheck;
                }

                if (!isBetweenInclusive(repulse.x,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(repulse.y,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(repulse.z,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }

                if (shipstaticinfo->staticheader.rightOfWay > avoidobj->staticinfo->staticheader.rightOfWay)
                {
                    // We have right of way:
                    bool dontBotherAvoiding = TRUE;

                    if (avoidobj->flags & SOF_Hide)
                    {
                        goto nextnode;
                    }

                    if (avoidobj->objtype == OBJ_ShipType)
                    {
#define avoidship ((Ship *)avoidobj)
                        // signal ship avoidobj to get out of way here (assuming it's static and our ship)
                        if (!(avoidship->specialFlags & SPECIAL_rowGettingOutOfWay))    // not already getting out of way
                        {
                            if (allianceIsShipAlly(avoidship,ship->playerowner))
                            {
                                sdword canGetOutOfWay = rowShipCanGetOutOfWayOfMe(avoidship,ship);      // ship is me
                                if (canGetOutOfWay == ROW_CAN_GET_OUT_OF_WAY)
                                {
                                    vector meToShip;
                                    vector meDirOfTravel;

                                    GetDirectionOfTravel(ship,&meDirOfTravel);

                                    vecSub(meToShip,avoidship->collInfo.collPosition,ship->collInfo.collPosition);

                                    if (vecDotProduct(meToShip,meDirOfTravel) > 0.0f)
                                    {
                                        if (avoidship->formationcommand)
                                        {
                                            rowSignalLeaderInFormationToGetOutOfWayOfMe(avoidship,ship,&meToShip,&meDirOfTravel);
                                        }
                                        else
                                        {
                                            rowSignalShipToGetOutOfWayOfMe(avoidship,ship,&meToShip,&meDirOfTravel);
                                        }
                                    }
                                }
                                else if (canGetOutOfWay == ROW_YOU_STILL_AVOID_ME)
                                {
                                    dontBotherAvoiding = FALSE;
                                }
                            }
                            else if (ship->specialFlags2 & SPECIAL_2_DontPlowThroughEnemyShips)
                            {
                                dontBotherAvoiding = FALSE;
                            }
                        }
#undef avoidship
                    }
                    else if (avoidobj->objtype == OBJ_AsteroidType)
                    {
                        bool takingItOut = FALSE;

                        // later tell ship to blow up asteroid in way?
                        if (isCapitalShipStatic(shipstaticinfo) && (ship->shiptype != Carrier) && (ship->shiptype != Mothership))
                        {
                            if ((shipcommand == NULL) ||
                                   ((shipcommand->ordertype.order != COMMAND_ATTACK) &&
                                   ((shipcommand->ordertype.attributes & (COMMAND_IS_ATTACKINGANDMOVING|COMMAND_IS_PASSIVEATTACKING)) == 0))
                                )
                            {
                                // ship isn't devoting its guns to anywhere else, so let's take a potshot at the asteroid in our way:
                                if (ship->gunInfo && (ship->gunInfo->numGuns > 0) && (avoidobj->flags & SOF_Targetable))
                                {
                                    vector meToShip;
                                    vector meDirOfTravel;

                                    GetDirectionOfTravel(ship,&meDirOfTravel);

                                    vecSub(meToShip,avoidobj->collInfo.collPosition,ship->collInfo.collPosition);

                                    if ((!takeOutInMyWay) && (vecDotProduct(meToShip,meDirOfTravel) > 0.0f))
                                    {
                                        takingItOut = TRUE;
                                        takeOutInMyWay = (SpaceObjRotImpTarg *)avoidobj;
                                    }
                                }
                            }
                        }

                        if (!takingItOut)
                        {
                            // perhaps we should be avoiding it if we're not taking it out?
                            sbyte asteroidrow = avoidobj->staticinfo->staticheader.rightOfWay;
                            if (((Asteroid *)avoidobj)->asteroidtype == Asteroid3)
                                asteroidrow++;      // give +1 row to asteroid3 since it's pretty big

                            if (shipstaticinfo->staticheader.rightOfWay <= asteroidrow)
                            {
                                dontBotherAvoiding = FALSE;     // we should avoid it after all
                            }
                        }
                    }

                    if (dontBotherAvoiding)
                    {
                        goto nextnode;
                    }
                }

                maxdistconsider = avoidcollpad + shipcollpad;

                if (ABS(distcheck) > maxdistconsider)
                {
                    goto nextnode;
                }
norowcheck:

                if (!isBetweenInclusive(repulse.x,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(repulse.y,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(repulse.z,-maxdistconsider,maxdistconsider))
                {
                    goto nextnode;
                }


                if ((avoidobj->flags & (SOF_Hide|SOF_Impactable|SOF_NotBumpable)) != (SOF_Impactable))
                {
                    goto nextnode;      // object must be impactable, bumpable
                }

                if (avoidobj == (SpaceObjRotImp *) ship)
                {
                    goto nextnode;  // cannot collide with itself
                }

                avoidcollsize = avoidobj->staticinfo->staticheader.staticCollInfo.avoidcollspheresize;

                if (avoidobj->objtype == OBJ_ShipType)
                {
#define avoidship ((Ship *)avoidobj)

                    if (shipcommand)
                    {
                        if (avoidship->command == shipcommand)
                        {
                            // ship and avoidobj are in the same formation, so we should effectively lower
                            // AVOID_OBJ_PADDING_SCALE to AVOID_OBJ_FORMATION_PADDING_SCALE

                            avoidcollpad = avoidcollsize * AVOID_OBJ_FORMATION_PADDING_SCALE;
                            shipcollpad = shipcollsize * AVOID_OBJ_FORMATION_PADDING_SCALE;
                            goto getmaxdist;
                        }

                        if ( (shipcommand->ordertype.order == COMMAND_MILITARYPARADE) && (shipcommand->militaryParade->aroundShip == avoidship))
                        {
                            if (avoidship->shiptype == Mothership)
                            {
                                avoidcollpad = avoidcollsize * ((avoidship->shiprace == R2) ? AVOID_OBJ_R2MSHIP_MILPARADE_PADDING_SCALE : AVOID_OBJ_MSHIP_MILPARADE_PADDING_SCALE);
                                shipcollpad = shipcollsize * AVOID_OBJ_FORMATION_PADDING_SCALE;
                            }
                            else
                            {
                                avoidcollpad = avoidcollsize * AVOID_OBJ_FORMATION_PADDING_SCALE;
                                shipcollpad = shipcollsize * AVOID_OBJ_FORMATION_PADDING_SCALE;
                            }
                       getmaxdist:
                            maxdistconsider = avoidcollpad + shipcollpad;

                            if (!isBetweenInclusive(repulse.x,-maxdistconsider,maxdistconsider))
                            {
                                goto nextnode;
                            }

                            if (!isBetweenInclusive(repulse.y,-maxdistconsider,maxdistconsider))
                            {
                                goto nextnode;
                            }

                            if (!isBetweenInclusive(repulse.z,-maxdistconsider,maxdistconsider))
                            {
                                goto nextnode;
                            }
                        }
                    }

                    if (((Ship *)avoidobj)->dockingship == ship)
                    {
                        goto nextnode;  // don't check if docking with ship
                    }

                    if((ship->flags & SOF_Slaveable) && (((Ship *) avoidobj)->flags & SOF_Slaveable))
                    {   //ships are slaveable
                        // two slaved ships SHOULD NOT try to avoid eachother
                        if(ship->slaveinfo->Master == ((Ship *) avoidobj)->slaveinfo->Master)
                        {   //if slaved to same thing, don't avoid
                            goto nextnode;
                        }
                    }
#undef avoidship
                }

                if(ship->specialFlags & SPECIAL_IsASalvager)
                {
                    if(((SalCapCorvetteSpec *)ship->ShipSpecifics)->salvageState == SAL_WL_FLYTOCONEORIGIN)
                    {
                        //don't want salvager in this state to avoid...
                        goto noavoid;
                    }
                    if( ((SalCapCorvetteSpec *)ship->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)avoidobj)
                    {
                        //avoid target?
                        if(((SalCapCorvetteSpec *)ship->ShipSpecifics)->noAvoid)
                        {
                            //want to avoid target
                            goto nextnode;
                        }
                    }
                    else if(avoidobj->objtype == OBJ_ShipType)
                    {
                        if( ((Ship *)avoidobj)->specialFlags & SPECIAL_IsASalvager)
                        {
                            if (((SalCapCorvetteSpec *)((Ship *)avoidobj)->ShipSpecifics)->target)
                            if( ((SalCapCorvetteSpec *)((Ship *)avoidobj)->ShipSpecifics)->target == ((SalCapCorvetteSpec *)ship->ShipSpecifics)->target)
                            {
                                //don't avoid other salcap corvettes
                                goto noavoid;
                            }
                        }
                    }
                }

                if(avoidobj->flags & SOF_Clamped)
                {
                    if(((Ship*)((Ship *)avoidobj)->clampInfo->host) == ship)
                    {
                        goto nextnode;
                    }
                }

                if (bitTest(avoidobj->flags,SOF_Cloaked))    //add cloakfield owner check here!
                {           //if object being considered is cloaked
                    if(avoidobj->objtype == OBJ_ShipType)
                    {
                        if(!proximityCanPlayerSeeShip(ship->playerowner,(Ship *)avoidobj))
                        {
                            goto nextnode;      //don't avoid it, but ignore it
                        }
                    }
                    else
                    {
                        goto nextnode;      //don't avoid it, but ignore it
                    }
                }

                if (shipcommand && shipcommand->ordertype.order == COMMAND_COLLECTRESOURCE && shipcommand->collect.resource == ((Resource *)avoidobj))
                {
                    goto nextnode;      // don't avoid asteroids you're harvesting
                }

                repulsemag2 = vecMagnitudeSquared(repulse);
                repulsedist = fsqrt(repulsemag2);

                if (repulsedist > maxdistconsider)
                {
                    goto nextnode;
                }

#ifndef HW_Release
                considered++;
#endif
                aishipStatsAvoidedConsidered();

                if (repulsedist < 0.1f)         // special case
                {
                    repulse.x = 0.1f;
                    repulsedist = 0.1f;
                    repulsemag2 = 0.01f;
                }

                vecScalarDivide(repulsenorm,repulse,repulsedist,temp);

                // very close, so figure out collidedist based on collision cubes
                collidedist = GetCollSizeInDirection((SpaceObjRotImp *)ship,repulsenorm) + GetCollSizeInDirection(avoidobj,repulsenorm);
#if VELFACTOR
                velfactor = -vecDotProduct(ship->posinfo.velocity,repulsenorm);
#endif
                distanceToCollision = repulsedist - collidedist;
                if (distanceToCollision < 20.0f)
                {
                    distanceToCollision = 20.0f;
                }
                repulse = repulsenorm;

                aishipStatsDistanceToCollision(distanceToCollision);

                if (avoidobj->objtype == OBJ_ShipType)
                {
                    if ((((Ship *)avoidobj)->shiptype == Mothership) && (((Ship *)avoidobj)->shiprace == R1))
                    {
                        repulse.z = 0.0f;
                        vecNormalize(&repulse);
                    }
                }

                distanceToCollision *= useAVOID_OBJ_SCALE_FACTOR;
                vecDivideByScalar(repulse,distanceToCollision,temp);

#if VELFACTOR
                if (velfactor > 1)
                {
                    velfactor *= AVOID_PANIC_FACTOR;
                    vecMultiplyByScalar(repulse,velfactor);
                }
#endif
//#define DEST_OBSCURED_BY_OBJ_PADDING    50.0f

                if (bitTest(aishipflags,AISHIP_DontFlyToObscuredPoints))
                {
                    vector destToObj;
                    real32 destToObjMag2;
                    real32 obscurecollsize;

                    if (avoidobj->objtype == OBJ_ShipType)
                    {
                        if (bitTest(aishipflags,AISHIP_IgnoreFormationObscuredPoints))
                        {
                            if (ship->formationcommand == ((Ship *)avoidobj)->formationcommand)
                            {
                                goto skipobscurecheck;
                            }
                        }

                        if (bitTest(aishipflags,AISHIP_IgnoreDockWithObscuredPoints))
                        {
                            if (ship->dockvars.dockship == ((Ship *)avoidobj))
                            {
                                goto skipobscurecheck;
                            }
                        }
                    }

                    obscurecollsize = avoidcollsize*AVOID_OBJ_OBSCUREDPOINT_SCALE + obscureaddcollsize; // + DEST_OBSCURED_BY_OBJ_PADDING;

                    obscureaddcollsize += AVOID_SPREAD_OUT_DIST;

                    // test if trying to move to a point that is being obscured by avoidobj
                    if (destination)
                    {
                        vecSub(destToObj,avoidobj->collInfo.collPosition,*destination);
                    }
                    else
                    {
                        // no destination provided, so use current ship position
                        vecSub(destToObj,avoidobj->collInfo.collPosition,ship->posinfo.position);
                    }

                    destToObjMag2 = vecMagnitudeSquared(destToObj);

                    if (destToObjMag2 < (obscurecollsize*obscurecollsize))
                    {
                        if (avoidobj->staticinfo->staticheader.staticCollInfo.avoidcollmodifier == 0.0f)
                        {
                            goto skipobscurecollsizedirectioncheck;
                        }

                        destToObjDist = fsqrt(destToObjMag2);
                        vecScalarDivide(destToObjNorm,destToObj,destToObjDist,temp);

                        if (destToObjDist < (GetCollSizeInDirection(avoidobj,destToObjNorm)))
                        {
skipobscurecollsizedirectioncheck:
                            if (bitTest(aishipflags,AISHIP_ReturnImmedIfPointObscured))
                            {
                                aishipStatsClose();
                                PTEND(2);
                                return AISHIP_FLY_OBJECT_IN_WAY;
                            }
                            else
                            {
                                returnflag |= AISHIP_FLY_OBJECT_IN_WAY;
                            }
                        }
                    }
skipobscurecheck:;
                }

                // test if repulsion vector is opposite of desired velocity by USE_PASS_RIGHT_ANGLE
                // so that we can add pass on right to repulsion vector so objects
                // will be able to go around each other
                testExactOpposite = vecDotProduct(repulsenorm,targetVec);
                if (testExactOpposite < 0)
                {
                    real32 targetVecMag2 = vecMagnitudeSquared(targetVec);
                    if (targetVecMag2 >= (20.0f*20.0f))     // must be at least 20m away before we consider passing on right (avoid jitter)
                    {
                        if ((testExactOpposite*testExactOpposite) >= (targetVecMag2 * USE_PASS_RIGHT_ANGLE))
                        {
                            vector passright;
                            vector upvector = { 0.0f,0.0f,1.0f };
                            real32 passrightmag;

                            vecCrossProduct(passright,targetVec,upvector);
                            vecNormalize(&passright);

                            passrightmag = repulsedist * PASS_RIGHT_FACTOR;
                            vecMultiplyByScalar(passright,passrightmag);

                            aishipStatsPassingOnRight(passright);

                            vecAddTo(repulse,passright);
                        }
                    }
                }

    #if DEBUG_AISHIP
                dbgMessagef("\nAVOID %x: (%f %f %f)",avoidobj,repulse.x,repulse.y,repulse.z);
    #endif
                aishipStatsRepulse(repulse);

                vecAddTo(desiredVel,repulse);

        nextnode:
                objindex++;
            }

        nextpass:
            pass++;
            if (pass < 2)
            {
                goto passagain;
            }


#ifndef HW_Release
            //dbgMessagef("\nAVOID considered: %d",considered);
#endif

noavoid:

        #if DEBUG_AISHIP
            dbgMessagef("\nFinal Desired Velocity: (%f %f %f)",desiredVel.x,desiredVel.y,desiredVel.z);
        #endif

            aishipStatsDesiredVel(desiredVel);

            if ((aishipflags & (AISHIP_StopIfVelocityTooLow|AISHIP_DontFlyToObscuredPoints)) &&
                isBetweenExclusive(desiredVel.x,NEG_AVOID_MIN_VEL,AVOID_MIN_VEL) &&
                isBetweenExclusive(desiredVel.y,NEG_AVOID_MIN_VEL,AVOID_MIN_VEL) &&
                isBetweenExclusive(desiredVel.z,NEG_AVOID_MIN_VEL,AVOID_MIN_VEL))
            {
                if (bitTest(aishipflags,AISHIP_ReturnImmedIfPointObscured))
                {
                    aishipStatsClose();
                    PTEND(2);
                    return AISHIP_FLY_OBJECT_IN_WAY;
                }
                else
                {
                    returnflag |= AISHIP_FLY_OBJECT_IN_WAY;
                }
                vecZeroVector(desiredVel);
            }
            else if (bitTest(aishipflags,AISHIP_PointInDirectionFlying))
            {
                if (shipstaticinfo->pitchdescend != 0.0f)
                {
                    // calculate normalize distance left
                    if ((destination) && (ship->aidescend != 0.0f))
                    {
                        zdistanceleft = (destination->z - ship->posinfo.position.z) * ABS(ship->aidescend);
                        pitchtodescend = getPitchToDescend(zdistanceleft,shipstaticinfo->pitchdescend);
#if DEBUG_AISHIP
                        dbgMessagef("\nzdistanceleft: %f pitchtodescend: %f",zdistanceleft,pitchtodescend);
#endif
                    }
                    else
                    {
                        pitchtodescend = 0.0f;
                    }
                    // use desiredVel for heading, because the heading does not have to be noramlized for this function

                    if ((destination) && (ship->aidescend < 0.0f))
                    {
                        matGetVectFromMatrixCol3(desiredHead,ship->rotinfo.coordsys);
                        aitrackHeadingWithBankPitch(ship,&desiredHead,FLYSHIP_HEADINGACCURACY,shipstaticinfo->sinbank,pitchtodescend,shipstaticinfo->pitchturn);
                    }
                    else
                        aitrackHeadingWithBankPitch(ship,&desiredVel,FLYSHIP_HEADINGACCURACY,shipstaticinfo->sinbank,pitchtodescend,shipstaticinfo->pitchturn);
                }
                else
                {
                    vecCopyAndNormalize(&desiredVel,&desiredHead);
                    aitrackHeadingWithBank(ship,&desiredHead,FLYSHIP_HEADINGACCURACY,shipstaticinfo->sinbank);
                }
            }

            if (takeOutInMyWay)
                attackSimple(ship,takeOutInMyWay);

            aitrackVelocityVector(ship,&desiredVel);

            aishipStatsClose();
            PTEND(2);
            return returnflag;

/*=============================================================================
    Unknown State:
=============================================================================*/

        default:
            dbgAssert(FALSE);
            aishipStatsClose();
            PTEND(2);
            return 0;
    }
}

void aishipFlyMissileToTarget(Missile *missile,SpaceObjRotImpTarg *target)
{
    MissileStaticInfo *missilestaticinfo = (MissileStaticInfo *)missile->staticinfo;
    //vector desiredHead;
    real32 missilevel;
    real32 missiletraveltime;
    vector distToTargetVec;
    real32 distToTargetMagSqr, distToTargetMag;
    vector deltatarget;

    vecSub(distToTargetVec,target->collInfo.collPosition,missile->posinfo.position);

    distToTargetMagSqr = vecMagnitudeSquared(distToTargetVec);

    if (distToTargetMagSqr > MISSILE_USE_VELOCITYPRED_DISTANCE)
    {
        goto gottargettrajectory;
    }

    missilevel = fsqrt(vecMagnitudeSquared(missile->posinfo.velocity));

    if (missilevel < 200.0f)
    {
        goto gottargettrajectory;
    }

    distToTargetMag = fsqrt(distToTargetMagSqr);

    missiletraveltime = distToTargetMag / missilevel;

    vecScalarMultiply(deltatarget,target->posinfo.velocity,missiletraveltime);

    vecAddTo(distToTargetVec,deltatarget);

gottargettrajectory:
    //vecMultiplyByScalar(distToTargetVec,MISSILE_SCALE_FACTOR);

    vecCapMinMaxVector(&distToTargetVec,missilestaticinfo->staticheader.maxvelocity,missilestaticinfo->staticheader.maxvelocity);

    //vecCopyAndNormalize(&desiredVel,&desiredHead);
    //aitrackHeadingWithFlags(missile,&desiredHead,FLYSHIP_ATTACKACCURACY,AITRACKHEADING_DONTROLL+AITRACKHEADING_IGNOREUPVEC);
    aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)missile,&distToTargetVec);
}


/*-----------------------------------------------------------------------------
    Name        : shouldMissileLoseTarget
    Description : checks certain criteria and sees if missile should lose its target
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool shouldMissileLoseTarget(Missile *missile)
{
    real32 rannum;
    real32 prob;
    vector roughDist;
    ObjType objtype = missile->target->objtype;

    if(objtype == OBJ_ShipType)
    {
        //if captial class we can't miss unless we're stupid
        if(isCapitalShip(((Ship *)missile->target)))
            return FALSE;

        vecSub(roughDist,missile->target->posinfo.position,missile->posinfo.position);

        if(isBetweenInclusive(roughDist.x,-tweakMinDistBeforeTargetLoss,tweakMinDistBeforeTargetLoss) &&
            isBetweenInclusive(roughDist.y,-tweakMinDistBeforeTargetLoss,tweakMinDistBeforeTargetLoss) &&
            isBetweenInclusive(roughDist.z,-tweakMinDistBeforeTargetLoss,tweakMinDistBeforeTargetLoss))
        {
            //target is within range to be considered for target loss
            missile->haveCheckedForLoss++;   //indicate check

         prob = 0.0f;

            prob = tweakRandomNess[((Ship *)missile->target)->tacticstype];
            if( ((Ship *)missile->target)->isDodging == TRUE)
            {
                prob+= tweakBoostIfDodging;
            }
            if( ((Ship *)missile->target)->flightman != FLIGHTMAN_NULL)
            {
                prob+=tweakBoostIfFlightMan;
            }

            rannum = frandombetween(0.0,1.0f);

            if(rannum < prob)
            {
                //satisfied: Target aquisition lost!
                return TRUE;
            }
        }
    }
    else if ((objtype == OBJ_DustType) || (objtype == OBJ_NebulaType))
    {
        vecSub(roughDist,missile->target->posinfo.position,missile->posinfo.position);

        if(isBetweenInclusive(roughDist.x,-200.0f,200.0f) &&
            isBetweenInclusive(roughDist.y,-200.0f,200.0f) &&
            isBetweenInclusive(roughDist.z,-200.0f,200.0f))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aishipGuideMissile
    Description : flys missile
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aishipGuideMissile(Missile *missile)
{
    SpaceObjRotImpTarg *target = missile->target;
    vector heading;

    if (missile->flags & SOF_Dead)
    {
        return FALSE;
    }

    if (target == NULL)
    {
        ;   //let missile just keep flying
    }
    else
    {
        aishipFlyMissileToTarget(missile,target);
        if(missile->target->flags & SOF_Cloaked)
        {
            missile->target = NULL;
        }
        else if(missile->haveCheckedForLoss < tweakNumTimesCheck)
        {
            if((universe.univUpdateCounter & tweakCheckMissileTargetLossRate) == (universe.univUpdateCounter & missile->missileID.missileNumber))
            {
                if(shouldMissileLoseTarget(missile))
                {
                    missile->target = NULL;
                }
            }
        }
    }

    heading = missile->posinfo.velocity;
    if ((heading.x != 0.0f) || (heading.y != 0.0f) || (heading.z != 0.0f))
    {
        vecNormalize(&heading);
        matCreateCoordSysFromHeading(&missile->rotinfo.coordsys,&heading);
        vecZeroVector(missile->rotinfo.rotspeed);
        vecZeroVector(missile->rotinfo.torque);
    }

    missile->posinfo.isMoving = TRUE;

    missile->timelived += universe.phystimeelapsed;
    if (missile->timelived > missile->totallifetime)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool aishipFlyMineToTarget(Missile *mine,SpaceObjRotImpTarg *target)
{
    MissileStaticInfo *minestaticinfo = (MissileStaticInfo *)mine->staticinfo;
    vector desiredVel;
    //vector desiredHead;
    real32 mine_rangesqr;
    real32 max_velocity;
    real32 max_velocitySQR;
    if(mine->FORCE_DROPPED)
    {
        mine_rangesqr = minestaticinfo->MINE_RANGESQR_FORCED;
        max_velocity = minestaticinfo->maxvelocity_FORCED;
        max_velocitySQR = minestaticinfo->maxvelocity_FORCED*minestaticinfo->maxvelocity_FORCED;
    }
    else
    {
        mine_rangesqr = minestaticinfo->MINE_RANGESQR;
        max_velocity = minestaticinfo->staticheader.maxvelocity;
        max_velocitySQR =minestaticinfo->staticheader.maxvelocity*minestaticinfo->staticheader.maxvelocity;
    }

    vecSub(desiredVel,target->collInfo.collPosition,mine->posinfo.position);
    //if(vecMagnitudeSquared(desiredVel) > mine_rangesqr)
    if(!MoveReachedDestinationVariable((Ship *)mine,&target->collInfo.collPosition,minestaticinfo->MINE_RANGESQR_FORCED))

    {
        //target flew out of range.
        mine->target = NULL;
        return(FALSE);
    }
    else if(mine->target != NULL)
    {
        if(mine->target->flags & SOF_Cloaked || mine->target->flags &SOF_Disabled)
        {
            mine->target = NULL;
            return FALSE;
        }
    }
    //checks targets speed and only goes after it if it can
    //have a chance of reaching it
    if(vecDotProduct(desiredVel,target->posinfo.velocity) > 0 )
    {
        //target is flying away from mine...check velocity now
        if(vecMagnitudeSquared(target->posinfo.velocity) > max_velocitySQR)
        {
            //target is flying faster than mine can go so don't follow it!
            mine->target = NULL;
            return(FALSE);
        }
    }

    if ((target->objtype != -1) && (target->collMyBlob))
    {
        // we've got a "real" target here
        if (mine->collMyBlob != target->collMyBlob)
        {
            if (mine->collMyBlob)
            {
                // let's take it out of our blob
                bobRemoveMineFromSpecificBlob(mine->collMyBlob,mine);
            }

            // put it in targets blob
            collAddSpaceObjToSpecificBlob(target->collMyBlob,(SpaceObj *)mine);
        }
    }

    vecNormalizeToLength(&desiredVel,max_velocity);
    //vecCapMinMaxVector(&desiredVel,MINE_MIN_VELOCITY,max_velocity);

    //vecCopyAndNormalize(&desiredVel,&desiredHead);

    // TAKE out track mines with right vector to spin them - save cpu
    //matGetVectFromMatrixCol1(desiredHead,mine->rotinfo.coordsys);       //track mines right vector always...spins then.
    //aitrackHeadingWithFlags(mine,&desiredHead,FLYSHIP_ATTACKACCURACY,AITRACKHEADING_IGNOREUPVEC);

    aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)mine,&desiredVel);
    return(TRUE);
}

bool aishipslowminewithFriction(Missile *mine, real32 fric)
{
    if(isBetweenInclusive(mine->posinfo.velocity.x,-1.0f,1.0f) &&
        isBetweenInclusive(mine->posinfo.velocity.y,-1.0f,1.0f) &&
        isBetweenInclusive(mine->posinfo.velocity.z,-1.0f,1.0f))
    {
        mine->posinfo.velocity.x = 0.0f;
        mine->posinfo.velocity.y = 0.0f;
        mine->posinfo.velocity.z = 0.0f;
        mine->rotinfo.rotspeed.x = 0.0f;
        mine->rotinfo.rotspeed.y = 0.0f;
        mine->rotinfo.rotspeed.z = 0.0f;
        return(TRUE);
    }
    else
    {
        mine->posinfo.velocity.x *= fric;
        mine->posinfo.velocity.y *= fric;
        mine->posinfo.velocity.z *= fric;
        mine->rotinfo.rotspeed.x *= fric;
        mine->rotinfo.rotspeed.y *= fric;
        mine->rotinfo.rotspeed.z *= fric;
    }
    return(FALSE);
}


#define blobSearchRight  1
#define blobSearchLeft  2

SpaceObjRotImpTarg *baseObj;
blob *baseBlob;
sdword blobDirection;
sdword blobIndex;
extern BlobProperties collBlobProperties;


//SEARCH Right...then
//SEARCH Left
Ship *getShipNearObjTok(SpaceObjRotImpTarg *obj, real32 range)
{

    Ship *blobShip;
    vector difvec;
    real32 dif;

    if(obj != NULL)
    {
        //starting
        if(obj->collMyBlob == NULL)
            return NULL;

        blobDirection = blobSearchRight;
        baseBlob = obj->collMyBlob;
        baseObj = obj;
        blobIndex = 0;
    }
    //continue or starting...doesn't matter
newBlob:
    for(;blobIndex < baseBlob->blobShips->numShips;blobIndex++)
    {
        //scan blob list
        blobShip = baseBlob->blobShips->ShipPtr[blobIndex];
        if(MoveReachedDestinationVariable((Ship *)blobShip,&baseObj->collInfo.collPosition,range))
        {
            //found ship within range
            blobIndex++;
            return blobShip;
        }
    }

    //blob expired...no more ships
    if(blobDirection == blobSearchRight)
    {
needanotherblob:
        if(baseBlob->node.next == NULL)
        {
            //done going right...go left
            //first reset blob though
startleft:
            blobDirection = blobSearchLeft;
            baseBlob=baseObj->collMyBlob;
            goto leftsearch;
        }

        baseBlob = (blob *)listGetStructOfNode(baseBlob->node.next);
        blobIndex = 0;
        //check if in range

        //this blob is further than normal blob
        //dif = baseBlob->sqrtSortDistance-baseObj->collMyBlob->sqrtSortDistance;
        //dif = dif + baseObj->collMyBlob->radius - collBlobProperties.bobBiggestRadius;
        dif = (baseBlob->sqrtSortDistance-collBlobProperties.bobBiggestRadius)-
               (baseObj->collMyBlob->sqrtSortDistance+baseObj->collOptimizeDist);
        if(dif > range)
        {
            //blob list exhausted to right
            goto startleft;
        }

        //blob is possible
        vecSub(difvec,baseObj->collInfo.collPosition,baseBlob->centre);
        if(!MoveReachedDestinationVariable((Ship *)baseObj,&baseBlob->centre,range+collBlobProperties.bobBiggestRadius))
        {
            //blob edge not in range...consider another
            goto needanotherblob;
        }
        //good
        goto newBlob;

    }
    else
    {
leftsearch:
        if(baseBlob->node.prev == NULL)
        {
            //done going left..return NULL
            return NULL;
        }

        //get next left blob
        baseBlob = (blob *)listGetStructOfNode(baseBlob->node.prev);

        //consider if in range
                //this blob is further than normal blob
        //dif = baseObj->collMyBlob->sqrtSortDistance - baseBlob->sqrtSortDistance;
        //dif = dif + collBlobProperties.bobBiggestRadius - baseObj->collMyBlob->radius;
        dif = (baseObj->collOptimizeDist+baseObj->collMyBlob->sqrtSortDistance)-
               (baseBlob->sqrtSortDistance+collBlobProperties.bobBiggestRadius);
        if(dif > range)
        {
            //blob list exhausted to right
            return NULL;
        }

        //blob is possible
        vecSub(difvec,baseObj->collInfo.collPosition,baseBlob->centre);
        if(!MoveReachedDestinationVariable((Ship *)baseObj,&baseBlob->centre,range+baseBlob->radius))
        {
            //blob edge not in range...consider another
            goto leftsearch;
        }
        //good
        goto newBlob;
    }
    return NULL;
}

Ship *aishipmineaquiretarget(Missile *mine)
{
    blob *MyBlob;
    SelectCommand *shipselection;
    //sdword shipindex = 0;
    Ship *ship;
    Ship *ShiptoRecieveShitPounding = NULL;
    //real32 distanceSqr;
    //vector diff;
    real32 mine_range;

    MissileStaticInfo *minestaticinfo = (MissileStaticInfo *)mine->staticinfo;

    MyBlob = mine->collMyBlob;
    shipselection = MyBlob->blobShips;
    if(mine->FORCE_DROPPED)
    {
        mine_range = minestaticinfo->mineRangeForced;
    }
    else
    {
        mine_range = minestaticinfo->mineRange;
    }

    ship = getShipNearObjTok((SpaceObjRotImpTarg *)mine,mine_range);

    while (ship != NULL)
    {
        //ship = shipselection->ShipPtr[shipindex];

        //ship = (Ship *)listGetStructOfNode(shipnode);

        if ( (ship->playerowner == mine->playerowner) ||
             (allianceIsShipAlly(ship,mine->playerowner)) )
        {   //don't target are ships....
            goto nextnode;
        }
        if(ship->flags & SOF_Cloaked)
        {
            if(!proximityCanPlayerSeeShip(mine->playerowner,ship))
            {
                goto nextnode;
            }
        }
        if(ship->flags & SOF_Disabled)
        {
            goto nextnode;
        }
        //here we decide not to attack allies as well :)

        //add more rejections to speed it up

        /*
        vecSub(diff,ship->posinfo.position,mine->posinfo.position);
        distanceSqr = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;

        if(distanceSqr < minestaticinfo->MINE_RANGESQR)
        */
        {
            if(ShiptoRecieveShitPounding == NULL)
            {
                ShiptoRecieveShitPounding = ship;
            }
            else if(ship->staticinfo->staticheader.mass > ShiptoRecieveShitPounding->staticinfo->staticheader.mass)
            {
                ShiptoRecieveShitPounding = ship;
            }
            else if (ship->staticinfo->staticheader.mass == ShiptoRecieveShitPounding->staticinfo->staticheader.mass)
            {   //if masses are the same
                if(randombetween(1,2) & 1)
                {   //randomly decide one over the other (that way, a group of fighters will
                   ShiptoRecieveShitPounding = ship;   // be attacked equally)
                }
            }
        }

    nextnode:
        //shipindex++;
        ship = getShipNearObjTok(NULL, mine_range);
    }
    return(ShiptoRecieveShitPounding);
}

// If it is time, it orders a search for a target
bool mine_do_search(Missile *mine)
{
    if((universe.univUpdateCounter & MINE_DO_SEARCH_MASK) == (mine->missileID.missileNumber & MINE_DO_SEARCH_MASK))
    {   //only do a check ever MINE_DO_SEARCH_MASKth frame...much quicker!
        if(mine->target == NULL)
        {
            mine->target = (SpaceObjRotImpTarg *)aishipmineaquiretarget(mine);
        }
        if(mine->target != NULL)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : aishipGuideMine
    Description : controls mine
    Inputs      : missile of type mine
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aishipGuideMine(Missile *mine)
{
    //Ship *ship;
    MissileStaticInfo *minestaticinfo = (MissileStaticInfo *)mine->staticinfo;
    vector dist;
    real32 distsqr;
    MinelayerCorvetteStatics *minelayercorvettestatics;

    if (mine->flags & SOF_Dead)
    {
        return FALSE;
    }

    dbgAssert(mine->missileType == MISSILE_Mine);

    if (mine->target)
    {
        switch (mine->target->objtype)
        {
            case OBJ_MissileType:
            case OBJ_AsteroidType:
            case OBJ_NebulaType:
            case OBJ_DustType:
                mine->target = NULL;
            default:
                break;
        }
    }

    switch(mine->mineAIState)
    {
    case MINE_DROP_ATTACK:                      //Fix exit path of mines here
        mine->mineAIState = MINE_ATTACK_RUN;
        //this is the state where mines are being dropped during an attack
        break;
    case MINE_DROP_FORMATION:     //starting of wall formation
        if(mine->owner != NULL)
        {
            vecSub(dist,mine->posinfo.position,mine->owner->posinfo.position);
            minelayercorvettestatics = (MinelayerCorvetteStatics *)((ShipStaticInfo *)mine->owner->staticinfo)->custstatinfo;
            distsqr = vecMagnitudeSquared(dist);
            if(distsqr > minelayercorvettestatics->MineClearDistanceSQR)
            {
                mine->mineAIState = DO_WALL_FORMATION;
            }
        }
        else
        {
            mine->mineAIState = DO_WALL_FORMATION;
        }
        break;
    case DO_WALL_FORMATION:
        if(mine_do_search(mine))        //always search for enemy after launching is cleared
        {
            mine->mineAIState = MINE_ATTACK_RUN;
        }
        if(!MoveReachedDestinationVariable((Ship *)mine,&mine->formation_position,10.0f))
        {
            SpaceObjRotImpTarg dest_faker;
            dest_faker.objtype = -1;
            dest_faker.collInfo.collPosition = mine->formation_position;
            vecSet(dest_faker.posinfo.velocity, 0.0f,0.0f,0.0f);
            dest_faker.flags = 0;
            aishipFlyMineToTarget(mine,&dest_faker);
        }
        else
        {
            //we've reached the destination so Slow down...
            aishipslowminewithFriction(mine, minestaticinfo->MINE_STOP_FRICTION);
        }

        break;
    case MINE_SEARCH_AND_STOP:
        aishipslowminewithFriction(mine, minestaticinfo->MINE_STOP_FRICTION);
        if(mine_do_search(mine))        //always search for enemy after launching is cleared
        {
            mine->mineAIState = MINE_ATTACK_RUN;
        }
        break;
    case MINE_ATTACK_RUN:
        if(mine->target == NULL)
        {   //Lost target...so stop, reform wall and search for new target
            //check if too far from formation location
            if(mine->formationinfo == NULL)
            {
                mine->mineAIState = MINE_SEARCH_AND_STOP;
            }
            else
            {
                mine->mineAIState = DO_WALL_FORMATION;
            }
                break;
        }
        else
        {
            if(!aishipFlyMineToTarget(mine,mine->target))
            {
                //lost target, so reform wall
                mine->mineAIState = DO_WALL_FORMATION;
            }
        }
        break;
    default:
        dbgAssert(FALSE);
        break;
    }

    mine->posinfo.isMoving = TRUE;

    // all mines, including force dropped ones, now expire
    mine->timelived += universe.phystimeelapsed;
    if (mine->timelived > mine->totallifetime)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}

