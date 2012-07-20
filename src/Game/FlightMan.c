/*=============================================================================
    Name    : flightman.c
    Purpose : This module handles all of the flight maneuvers.

    Created 11/4/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Physics.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "StatScript.h"
#include "AITrack.h"
#include "AIShip.h"
#include "SoundEvent.h"
#include "FlightMan.h"
#include "Randy.h"

#ifndef HW_Release
//#define FLIGHTMAN_DEBUG
#endif

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 FLIPTURN_MAXROTSPEEDFLIP = 1.0f;
real32 FLIPTURN_PAUSEAFTERFLIPTIME = 1.0f;
real32 FLIPTURN_FLIPACCELMODIFIER = 0.25f;
real32 FLIPTURN_AFTERFLIPTHRUSTMOD = 1.0f;
real32 FLIPTURN_ROLLTHRUSTMOD = 0.5f;
real32 FLIPTURN_ROLLACCELMODIFIER = 0.5f;
real32 FLIPTURN_MAXROLLSPEED = 1.0f;

real32 IMMELMAN_THRUSTMOD = 1.0f;
real32 IMMELMAN_ACCELMODIFIERFLIP = 0.25f;
real32 IMMELMAN_MAXROTSPEEDFLIP = 1.0f;
real32 IMMELMAN_ROLLACCELMODIFIER = 0.25f;
real32 IMMELMAN_MAXROLLSPEED = 1.0f;
real32 IMMELMAN_ROLLTHRUSTMOD = 0.5f;

real32 SPLIT_S_THRUSTMOD = 1.0f;
real32 SPLIT_S_ACCELMODIFIERFLIP = 0.25f;
real32 SPLIT_S_MAXROTSPEEDPITCH = 1.0f;
real32 SPLIT_S_ACCELMODIFIERROLL = 0.25f;
real32 SPLIT_S_MAXROTSPEEDROLL = 1.0f;

real32 ENDOVER_ACCELMODIFIER = 0.25f;
real32 ENDOVER_MAXROTSPEED = 8.0f;

real32 HARDBANK_THRUSTMOD = 1.0f;
real32 HARDBANK_ACCELMODIFIERFLIP = 0.25f;
real32 HARDBANK_MAXROTSPEEDPITCH = 1.0f;
real32 HARDBANK_ACCELMODIFIERROLL = 0.25f;
real32 HARDBANK_MAXROTSPEEDROLL = 1.0f;

real32 SOFTBANK_THRUSTMOD = 1.0f;
real32 SOFTBANK_ACCELMODIFIERFLIP = 0.25f;
real32 SOFTBANK_MAXROTSPEEDPITCH = 1.0f;
real32 SOFTBANK_ACCELMODIFIERROLL = 0.25f;
real32 SOFTBANK_MAXROTSPEEDROLL = 1.0f;

real32 VREVERSE_ACCELMODIFIERYAW = 0.25f;
real32 VREVERSE_MAXROTSPEEDYAW = 1.0f;
real32 VREVERSE_THRUSTMOD = 1.0f;
real32 VREVERSE_ACCELMODIFIERPITCH = 0.25f;
real32 VREVERSE_MAXROTSPEEDPITCH = 1.0f;

real32 BARREL_ROLL_ACCELMODIFIERROLL = 0.25f;
real32 BARREL_ROLL_MAXROTSPEEDROLL = 1.0f;

real32 CELEB_FLIP_ACCELMODIFIERROLL = 0.25f;
real32 CELEB_FLIP_MAXROTSPEEDROLL = 1.5f;

real32 SWARMER_BARRELROLL_ACCELMODIFIERROLL = 0.25f;
real32 SWARMER_BARRELROLL_MAXROTSPEEDROLL = 2.0f;

real32 ROLL180_ACCELMODIFIERROLL = 0.25f;
real32 ROLL180_MAXROTSPEEDROLL = 2.0f;

real32 WHIPSTRAFE_ESCAPEDIST_Z = 500.0f;
real32 WHIPSTRAFE_BULLETRANGEOPTIMUM = 0.3f;
real32 WHIPSTRAFE_MAXANGCOMPAREFACTOR = 0.25f;
real32 WHIPSTRAFE_FLYBYOVERSHOOT = 1.3f;
real32 WHIPSTRAFE_MINVELOCITY = -200.0f;
real32 WHIPSTRAFE_REACHDESTINATIONRANGE = 110.0f;

real32 WHIPSTRAFE_FACEDOWNUP_ACCURACY   = 0.98f;
real32 WHIPSTRAFE_WHIPTHRUSTMOD         = 1.0f;
real32 WHIPSTRAFE_WHIPTIME              = 3.0f;
//real32 WHIPSTRAFE_LEVEL_ACCELMOD        = 0.5f;
//real32 WHIPSTRAFE_LEVEL_MAXROT          = 1.0f;
//real32 WHIPSTRAFE_LEVELOUTTHRUSTMOD     = 0.5f;

real32 ROLLAWAY_THRUSTMOD               = 1.0f;
real32 ROLLAWAY_ACCELMODIFIERROLL       = 1.0f;
real32 ROLLAWAY_MAXROTSPEEDROLL         = 1.0f;
//real32 ROLLAWAY_THRUSTMOD2              = 0.5f;
real32 ROLLAWAY_THRUSTDOWNTIME          = 0.5f;

real32 SPLITSEVASIVE_ACCELMODIFIERPITCH = 0.5f;
real32 SPLITSEVASIVE_MAXROTSPEEDPITCH = 1.0f;
real32 SPLITSEVASIVE_THRUSTMOD = 1.0f;
real32 SPLITSEVASIVE_ACCELMODIFIERROLL = 0.5f;
real32 SPLITSEVASIVE_MAXROTSPEEDROLL = 1.0f;

real32 LOYOYO_THRUSTMOD0 = 0.5f;
real32 LOYOYO_THRUSTMOD1 = 0.5f;
real32 LOYOYO_THRUSTMOD2 = 0.5f;
real32 LOYOYO_THRUSTMOD3 = 1.0f;
real32 LOYOYO_THRUSTMOD4 = 0.5f;
real32 LOYOYO_ACCELMODIFIERPITCH = 0.5f;
real32 LOYOYO_MAXROTSPEEDPITCH = 1.0f;
real32 LOYOYO_GODOWN_TIME = 0.5f;
real32 LOYOYO_GOUP_TIME = 2.0f;

real32 HIYOYO_THRUSTMOD0 = 0.5f;
real32 HIYOYO_THRUSTMOD1 = 0.5f;
real32 HIYOYO_THRUSTMOD2 = 0.5f;
real32 HIYOYO_THRUSTMOD3 = 1.0f;
real32 HIYOYO_THRUSTMOD4 = 0.5f;
real32 HIYOYO_ACCELMODIFIERPITCH = 0.5f;
real32 HIYOYO_MAXROTSPEEDPITCH = 1.0f;
real32 HIYOYO_GOUP_TIME = 0.5f;
real32 HIYOYO_GODOWN_TIME = 2.0f;

real32 SIDESTEP_THRUSTMOD = 1.0f;
real32 SIDESTEP_THRUSTMODTURN = 0.2f;
real32 SIDESTEP_ACCELMODIFIERYAW = 0.5f;
real32 SIDESTEP_MAXROTSPEEDYAW = 1.0f;
real32 SIDESTEP_ACCELMODIFIERPITCH = 0.25f;
real32 SIDESTEP_MAXROTSPEEDPITCH = 0.5f;
real32 SIDESTEP_ACCELMODIFIERROLL = 0.5f;
real32 SIDESTEP_MAXROTSPEEDROLL = 1.0f;
real32 SIDESTEP_PAUSETIME = 0.5f;
real32 SIDESTEP_ROLLROTATE = DEG_TO_RAD(100.0f);
real32 SIDESTEP_PITCHROTATE = DEG_TO_RAD(100.0f);
real32 SIDESTEP_BOOTITTIME = 2.0f;

#if 0
real32 SLALOM_THRUSTMOD = 0.5f;
real32 SLALOM_THRUSTMODTURN = 1.0f;
real32 SLALOM_ACCELMODIFIERPITCH = 0.5f;
real32 SLALOM_MAXROTSPEEDPITCH  = 1.0f;
real32 SLALOM_ACCELMODIFIERROLL = 0.5f;
real32 SLALOM_MAXROTSPEEDROLL   = 1.0f;
sdword SLALOM_BASEITERATIONS = 2;
sdword SLALOM_RANDOMITERATIONS = 3;  // must be 1,3,7,15, etc.;
#endif

#define SLALOM_MAXITERATIONS    15          // later don't hardcode to maximum

real32 SLALOM_MINVELOCITY = -300.0f;
real32 SLALOM_REACHDESTINATIONRANGE = 160.0f;
real32 SLALOM_MAXWAYPOINTTIME = 10.0f;
real32 SLALOM_RUN_LENGTH = 1000.0f;
sdword SLALOM_RANDOMITERATIONS = 3;
sdword SLALOM_BASEITERATIONS = 2;

real32 SANDWICH_ANGLE = DEG_TO_RAD(45.0f);
real32 SANDWICH_MAXTIME = 10.0f;
real32 SANDWICH_MINVELOCITY = -300.0f;
real32 SANDWICH_REACHDESTINATIONRANGE = 160.0f;
real32 SANDWICH_PINCHER_SCALE = 0.5f;

sdword FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKRATE = 63;
sdword FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKFRAME = 47;
sdword FLIGHTMAN_TOKEN_EVASIVEPURE_PROB = 50;

sdword FLIGHTMAN_EVASIVEPURE_PROB[NUM_TACTICS_TYPES] = { 50, 50 , 50 };
sdword FLIGHTMAN_SPLIT_PROB[NUM_TACTICS_TYPES] = { 255, 255, 255 };
sdword FLIGHTMAN_SANDWICH_PROB[NUM_TACTICS_TYPES] = { 255, 255, 255 };

uword FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[NUM_TACTICS_TYPES] = { 2, 2, 2 };

scriptEntry FlightmanTweaks[] =
{
    makeEntry(FLIPTURN_MAXROTSPEEDFLIP,scriptSetReal32CB),
    makeEntry(FLIPTURN_PAUSEAFTERFLIPTIME,scriptSetReal32CB),
    makeEntry(FLIPTURN_FLIPACCELMODIFIER,scriptSetReal32CB),
    makeEntry(FLIPTURN_AFTERFLIPTHRUSTMOD,scriptSetReal32CB),
    makeEntry(FLIPTURN_ROLLTHRUSTMOD,scriptSetReal32CB),
    makeEntry(FLIPTURN_ROLLACCELMODIFIER,scriptSetReal32CB),
    makeEntry(FLIPTURN_MAXROLLSPEED,scriptSetReal32CB),
    makeEntry(IMMELMAN_THRUSTMOD,scriptSetReal32CB),
    makeEntry(IMMELMAN_ACCELMODIFIERFLIP,scriptSetReal32CB),
    makeEntry(IMMELMAN_MAXROTSPEEDFLIP,scriptSetReal32CB),
    makeEntry(IMMELMAN_ROLLACCELMODIFIER,scriptSetReal32CB),
    makeEntry(IMMELMAN_MAXROLLSPEED,scriptSetReal32CB),
    makeEntry(IMMELMAN_ROLLTHRUSTMOD,scriptSetReal32CB),
    makeEntry(SPLIT_S_THRUSTMOD,scriptSetReal32CB),
    makeEntry(SPLIT_S_ACCELMODIFIERFLIP,scriptSetReal32CB),
    makeEntry(SPLIT_S_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(SPLIT_S_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(SPLIT_S_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(HARDBANK_THRUSTMOD,scriptSetReal32CB),
    makeEntry(HARDBANK_ACCELMODIFIERFLIP,scriptSetReal32CB),
    makeEntry(HARDBANK_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(HARDBANK_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(HARDBANK_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(SOFTBANK_THRUSTMOD,scriptSetReal32CB),
    makeEntry(SOFTBANK_ACCELMODIFIERFLIP,scriptSetReal32CB),
    makeEntry(SOFTBANK_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(SOFTBANK_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(SOFTBANK_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(VREVERSE_ACCELMODIFIERYAW,scriptSetReal32CB),
    makeEntry(VREVERSE_MAXROTSPEEDYAW,scriptSetReal32CB),
    makeEntry(VREVERSE_THRUSTMOD,scriptSetReal32CB),
    makeEntry(VREVERSE_ACCELMODIFIERPITCH,scriptSetReal32CB),
    makeEntry(VREVERSE_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(BARREL_ROLL_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(BARREL_ROLL_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_ESCAPEDIST_Z,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_BULLETRANGEOPTIMUM,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_MAXANGCOMPAREFACTOR,scriptSetReal32SqrCB),
    makeEntry(WHIPSTRAFE_FLYBYOVERSHOOT,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_MINVELOCITY,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_REACHDESTINATIONRANGE,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_FACEDOWNUP_ACCURACY,scriptSetCosAngCB),
    makeEntry(WHIPSTRAFE_WHIPTHRUSTMOD,scriptSetReal32CB),
    makeEntry(WHIPSTRAFE_WHIPTIME,scriptSetReal32CB),
//    makeEntry(WHIPSTRAFE_LEVEL_ACCELMOD,scriptSetReal32CB),
//    makeEntry(WHIPSTRAFE_LEVEL_MAXROT,scriptSetReal32CB),
//    makeEntry(WHIPSTRAFE_LEVELOUTTHRUSTMOD,scriptSetReal32CB),
    makeEntry(ROLLAWAY_THRUSTMOD,scriptSetReal32CB),
//    makeEntry(ROLLAWAY_THRUSTMOD2,scriptSetReal32CB),
    makeEntry(ROLLAWAY_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(ROLLAWAY_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(ROLLAWAY_THRUSTDOWNTIME,scriptSetReal32CB),
    makeEntry(SPLITSEVASIVE_ACCELMODIFIERPITCH,scriptSetReal32CB),
    makeEntry(SPLITSEVASIVE_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(SPLITSEVASIVE_THRUSTMOD,scriptSetReal32CB),
    makeEntry(SPLITSEVASIVE_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(SPLITSEVASIVE_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(LOYOYO_THRUSTMOD0,scriptSetReal32CB),
    makeEntry(LOYOYO_THRUSTMOD1,scriptSetReal32CB),
    makeEntry(LOYOYO_THRUSTMOD2,scriptSetReal32CB),
    makeEntry(LOYOYO_THRUSTMOD3,scriptSetReal32CB),
    makeEntry(LOYOYO_THRUSTMOD4,scriptSetReal32CB),
    makeEntry(LOYOYO_ACCELMODIFIERPITCH,scriptSetReal32CB),
    makeEntry(LOYOYO_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(LOYOYO_GOUP_TIME,scriptSetReal32CB),
    makeEntry(LOYOYO_GODOWN_TIME,scriptSetReal32CB),
    makeEntry(HIYOYO_THRUSTMOD0,scriptSetReal32CB),
    makeEntry(HIYOYO_THRUSTMOD1,scriptSetReal32CB),
    makeEntry(HIYOYO_THRUSTMOD2,scriptSetReal32CB),
    makeEntry(HIYOYO_THRUSTMOD3,scriptSetReal32CB),
    makeEntry(HIYOYO_THRUSTMOD4,scriptSetReal32CB),
    makeEntry(HIYOYO_ACCELMODIFIERPITCH,scriptSetReal32CB),
    makeEntry(HIYOYO_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(HIYOYO_GOUP_TIME,scriptSetReal32CB),
    makeEntry(HIYOYO_GODOWN_TIME,scriptSetReal32CB),
    makeEntry(SIDESTEP_THRUSTMOD,scriptSetReal32CB),
    makeEntry(SIDESTEP_THRUSTMODTURN,scriptSetReal32CB),
    makeEntry(SIDESTEP_ACCELMODIFIERYAW,scriptSetReal32CB),
    makeEntry(SIDESTEP_MAXROTSPEEDYAW,scriptSetReal32CB),
    makeEntry(SIDESTEP_ACCELMODIFIERPITCH,scriptSetReal32CB),
    makeEntry(SIDESTEP_MAXROTSPEEDPITCH,scriptSetReal32CB),
    makeEntry(SIDESTEP_ACCELMODIFIERROLL,scriptSetReal32CB),
    makeEntry(SIDESTEP_MAXROTSPEEDROLL,scriptSetReal32CB),
    makeEntry(SIDESTEP_PAUSETIME,scriptSetReal32CB),
    makeEntry(SIDESTEP_BOOTITTIME,scriptSetReal32CB),
    makeEntry(SIDESTEP_ROLLROTATE,scriptSetAngCB),
    makeEntry(SIDESTEP_PITCHROTATE,scriptSetAngCB),

    makeEntry(SLALOM_MINVELOCITY,scriptSetReal32CB),
    makeEntry(SLALOM_REACHDESTINATIONRANGE,scriptSetReal32CB),
    makeEntry(SLALOM_MAXWAYPOINTTIME,scriptSetReal32CB),
    makeEntry(SLALOM_RUN_LENGTH,scriptSetReal32CB),
    makeEntry(SLALOM_RANDOMITERATIONS,scriptSetSdwordCB),
    makeEntry(SLALOM_BASEITERATIONS,scriptSetSdwordCB),

    makeEntry(SANDWICH_ANGLE,scriptSetAngCB),
    makeEntry(SANDWICH_MAXTIME,scriptSetReal32CB),
    makeEntry(SANDWICH_REACHDESTINATIONRANGE,scriptSetReal32CB),
    makeEntry(SANDWICH_MINVELOCITY,scriptSetReal32CB),
    makeEntry(SANDWICH_PINCHER_SCALE,scriptSetReal32CB),

    makeEntry(FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKRATE,scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKFRAME,scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_TOKEN_EVASIVEPURE_PROB,scriptSetSdwordCB),

    makeEntry(FLIGHTMAN_EVASIVEPURE_PROB[Evasive],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_EVASIVEPURE_PROB[Neutral],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_EVASIVEPURE_PROB[Aggressive],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SPLIT_PROB[Evasive],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SPLIT_PROB[Neutral],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SPLIT_PROB[Aggressive],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SANDWICH_PROB[Evasive],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SANDWICH_PROB[Neutral],scriptSetSdwordCB),
    makeEntry(FLIGHTMAN_SANDWICH_PROB[Aggressive],scriptSetSdwordCB),

    makeEntry(FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[Evasive],scriptSetUwordCB),
    makeEntry(FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[Neutral],scriptSetUwordCB),
    makeEntry(FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[Aggressive],scriptSetUwordCB),

    endEntry
};

/*=============================================================================
    Private defines:
=============================================================================*/

#define FLIGHTPROB_VALID 0xcc

/*=============================================================================
    Private types:
=============================================================================*/

typedef void (*FlightmanSpecificInit) (Ship *ship,sdword flags);
typedef bool (*FlightmanSpecificExecute) (Ship *ship);
typedef void (*FlightmanSpecificClose) (Ship *ship);

/*=============================================================================
    Private data:
=============================================================================*/

static FlightmanSpecificInit flightmanSpecificInitTable[NUM_FLIGHTMANEUVERS];
static FlightmanSpecificExecute flightmanSpecificExecuteTable[NUM_FLIGHTMANEUVERS];
static FlightmanSpecificClose flightmanSpecificCloseTable[NUM_FLIGHTMANEUVERS];

/*=============================================================================
    Debug:
=============================================================================*/

#ifndef HW_Release

#define FLIGHTMAN_TESTNUM       1
Ship *testflightmanship = NULL;
udword testflightmans[FLIGHTMAN_TESTNUM] = { FLIGHTMAN_SLALOM };
udword testcurflightman = 0;

void flightmanTest()
{
    if (testflightmanship == NULL)
    {
        return;
    }

    if (testflightmanship->flightman != testflightmans[testcurflightman])
    {
        flightmanInit(testflightmanship,testflightmans[testcurflightman]);
    }
    else
    {
        if (flightmanExecute(testflightmanship))
        {
            testcurflightman++;
            if (testcurflightman >= FLIGHTMAN_TESTNUM)
            {
                testcurflightman = 0;
            }
            aitrackForceSteadyShip(testflightmanship);
            vecZeroVector(testflightmanship->posinfo.force);
            vecZeroVector(testflightmanship->rotinfo.torque);
            testflightmanship = NULL;
        }
    }
}

#endif

/*=============================================================================
    Flight maneuver utilities
=============================================================================*/

bool flightmanPitchDown(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedy = ship->rotinfo.rotspeed.y;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedy;

    if (*totalAngleRotated <= -degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedy > -maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_PITCHDOWN] * rotaccelmodifier,ROT_PITCHDOWN);
        }
        return FALSE;
    }
}

bool flightmanPitchUp(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedy = ship->rotinfo.rotspeed.y;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedy;

    if (*totalAngleRotated >= degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedy < maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_PITCHUP] * rotaccelmodifier,ROT_PITCHUP);
        }
        return FALSE;
    }
}

bool flightmanRollRight(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedz = ship->rotinfo.rotspeed.z;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedz;

    if (*totalAngleRotated >= degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedz < maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ROLLRIGHT] * rotaccelmodifier,ROT_ROLLRIGHT);
        }
        return FALSE;
    }
}

bool flightmanRollLeft(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedz = ship->rotinfo.rotspeed.z;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedz;

    if (*totalAngleRotated <= -degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedz > -maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ROLLLEFT] * rotaccelmodifier,ROT_ROLLLEFT);
        }
        return FALSE;
    }
}

bool flightmanYawRight(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedx = ship->rotinfo.rotspeed.x;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedx;

    if (*totalAngleRotated <= -degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedx > -maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_YAWRIGHT] * rotaccelmodifier,ROT_YAWRIGHT);
        }
        return FALSE;
    }
}

bool flightmanYawLeft(Ship *ship,real32 *totalAngleRotated,real32 degreesToRotate,real32 maxrotspeed,real32 rotaccelmodifier)
{
    real32 rotspeedx = ship->rotinfo.rotspeed.x;
    *totalAngleRotated += universe.phystimeelapsed * rotspeedx;

    if (*totalAngleRotated >= degreesToRotate)
    {
        return TRUE;
    }
    else
    {
        if (rotspeedx < maxrotspeed)
        {
            physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_YAWLEFT] * rotaccelmodifier,ROT_YAWLEFT);
        }
        return FALSE;
    }
}

bool flightmanStabilizePitch(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    real32 rotspeedy = ship->rotinfo.rotspeed.y;
    real32 rotstr;

    if (rotspeedy > STILL_ROT_HI)
    {
        rotstr = rotspeedy * shipstaticinfo->staticheader.momentOfInertiaY / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
        return FALSE;
    }
    else if (rotspeedy < STILL_ROT_LO)
    {
        rotstr = -rotspeedy * shipstaticinfo->staticheader.momentOfInertiaY / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
        return FALSE;
    }

    return TRUE;
}

bool flightmanStabilizeRoll(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    real32 rotspeedz = ship->rotinfo.rotspeed.z;
    real32 rotstr;

    if (rotspeedz > STILL_ROT_HI)
    {
        rotstr = rotspeedz * shipstaticinfo->staticheader.momentOfInertiaZ / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        return FALSE;
    }
    else if (rotspeedz < STILL_ROT_LO)
    {
        rotstr = -rotspeedz * shipstaticinfo->staticheader.momentOfInertiaZ / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        return FALSE;
    }

    return TRUE;
}

bool flightmanStabilizeYaw(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    real32 rotspeedx = ship->rotinfo.rotspeed.x;
    real32 rotstr;

    if (rotspeedx > STILL_ROT_HI)
    {
        rotstr = rotspeedx * shipstaticinfo->staticheader.momentOfInertiaX / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
        return FALSE;
    }
    else if (rotspeedx < STILL_ROT_LO)
    {
        rotstr = -rotspeedx * shipstaticinfo->staticheader.momentOfInertiaX / universe.phystimeelapsed;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
        return FALSE;
    }

    return TRUE;
}

/*=============================================================================
    Specific flight maneuvers:
=============================================================================*/

/*=============================================================================
    Do nothing flight maneuver
=============================================================================*/

bool flightmanDoNothingExecute(Ship *ship)
{
    dbgAssert(ship->flightman == FLIGHTMAN_DONOTHING);
    return TRUE;
}

/*=============================================================================
    Flip turn:
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalanglerotated;
    real32 totalrollrotated;
    real32 finishedfliptimestamp;
    udword rollleft;
} FlipTurnInfo;

void flightmanFlipTurnInit(Ship *ship,sdword flags)
{
    FlipTurnInfo *flipturninfo;

    dbgAssert(ship->flightman == FLIGHTMAN_FLIPTURN);
    flipturninfo = ship->flightmanInfo = memAlloc(sizeof(FlipTurnInfo),"FlipTurnInfo",0);
    flipturninfo->size = sizeof(FlipTurnInfo);

    if (flags < 0)
    {
        flags = gamerand();
    }

    flipturninfo->totalanglerotated = 0.0f;
    flipturninfo->totalrollrotated = 0.0f;
    flipturninfo->finishedfliptimestamp = 0.0f;
    flipturninfo->rollleft = flags & 1;
}

bool flightmanFlipTurnExecute(Ship *ship)
{
    FlipTurnInfo *flipturninfo;

    dbgAssert(ship->flightman == FLIGHTMAN_FLIPTURN);
    flipturninfo = (FlipTurnInfo *)ship->flightmanInfo;
    dbgAssert(flipturninfo->size == sizeof(FlipTurnInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (flightmanPitchDown(ship,&flipturninfo->totalanglerotated,DEG_TO_RAD(180.0f),FLIPTURN_MAXROTSPEEDFLIP,FLIPTURN_FLIPACCELMODIFIER))
            {
                ship->flightmanState1 = 1;
                flipturninfo->finishedfliptimestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 1:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*FLIPTURN_AFTERFLIPTHRUSTMOD,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - flipturninfo->finishedfliptimestamp) > FLIPTURN_PAUSEAFTERFLIPTIME)
            {
                ship->flightmanState1 = 2;
            }
            return FALSE;

        case 2:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*FLIPTURN_ROLLTHRUSTMOD,TRANS_FORWARD);

            if (flipturninfo->rollleft)
            {
                return flightmanRollLeft(ship,&flipturninfo->totalrollrotated,DEG_TO_RAD(180.0f),FLIPTURN_MAXROLLSPEED,FLIPTURN_ROLLACCELMODIFIER);
            }
            else
            {
                return flightmanRollRight(ship,&flipturninfo->totalrollrotated,DEG_TO_RAD(180.0f),FLIPTURN_MAXROLLSPEED,FLIPTURN_ROLLACCELMODIFIER);
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Corkscrew flip turn:
=============================================================================*/

typedef struct
{
    sdword size;
    real32 total_yaw_angle;
    real32 total_roll_angle;
    real32 angle_increment;
    real32 force;
    real32 finishedfliptimestamp;
    udword rollleft;
} CorkscrewInfo;

void flightmanCorkscrewInit(Ship *ship,sdword flags)
{
    CorkscrewInfo *corkscrewinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_CORKSCREW);
    corkscrewinfo = ship->flightmanInfo = memAlloc(sizeof(CorkscrewInfo),"CorkscrewInfo",0);
    corkscrewinfo->size = sizeof(CorkscrewInfo);

    if (flags < 0)
    {
        flags = gamerand();
    }

    corkscrewinfo->total_yaw_angle = 0.0f;
    corkscrewinfo->total_roll_angle = 0.0f;
    corkscrewinfo->angle_increment = 4.0f;
    corkscrewinfo->force = 0.5f;
    corkscrewinfo->rollleft = flags & 1;
    corkscrewinfo->finishedfliptimestamp = 0.0f;
}

bool flightmanCorkscrewExecute(Ship *ship)
{
    CorkscrewInfo *corkscrewinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_CORKSCREW);
    corkscrewinfo = (CorkscrewInfo *)ship->flightmanInfo;
    dbgAssert(corkscrewinfo->size == sizeof(CorkscrewInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            flightmanYawLeft(ship,&corkscrewinfo->total_yaw_angle,DEG_TO_RAD(720.0f), 2.0f, 0.25f);
            corkscrewinfo->angle_increment -= 0.05f;
            return(flightmanRollRight(ship,&corkscrewinfo->total_roll_angle,DEG_TO_RAD(360.0f), corkscrewinfo->angle_increment, 0.25f));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Immelman
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 totalrollrotated;
    bool inverted;
    bool rollleft;
} ImmelmanInfo;

void flightmanImmelmanInit(Ship *ship,sdword flags)
{
    ImmelmanInfo *immelmaninfo;

    dbgAssert(ship->flightman == FLIGHTMAN_IMMELMAN);
    immelmaninfo = ship->flightmanInfo = memAlloc(sizeof(ImmelmanInfo),"ImmelmanInfo",0);
    immelmaninfo->size = sizeof(ImmelmanInfo);

    if (flags < 0)
    {
        flags = gamerand();
    }

    immelmaninfo->inverted = flags & IMMELMAN_INVERT;
    immelmaninfo->rollleft = flags & IMMELMAN_ROLLLEFT;
    immelmaninfo->totalpitchrotated = 0.0f;
    immelmaninfo->totalrollrotated = 0.0f;
}

bool flightmanImmelmanExecute(Ship *ship)
{
    ImmelmanInfo *immelmaninfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_IMMELMAN);
    immelmaninfo = (ImmelmanInfo *)ship->flightmanInfo;
    dbgAssert(immelmaninfo->size == sizeof(ImmelmanInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (immelmaninfo->inverted)
            {
                result = flightmanPitchDown(ship,&immelmaninfo->totalpitchrotated,DEG_TO_RAD(180.0f),IMMELMAN_MAXROTSPEEDFLIP,IMMELMAN_ACCELMODIFIERFLIP);
            }
            else
            {
                result = flightmanPitchUp(ship,&immelmaninfo->totalpitchrotated,DEG_TO_RAD(180.0f),IMMELMAN_MAXROTSPEEDFLIP,IMMELMAN_ACCELMODIFIERFLIP);
            }

            if (result)
            {
                ship->flightmanState1 = 1;
            }
            else
            {
                physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * IMMELMAN_THRUSTMOD,TRANS_FORWARD);
            }
            return FALSE;

        case 1:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD]*IMMELMAN_ROLLTHRUSTMOD,TRANS_FORWARD);

            if (immelmaninfo->rollleft)
            {
                return flightmanRollLeft(ship,&immelmaninfo->totalrollrotated,DEG_TO_RAD(180.0f),IMMELMAN_MAXROLLSPEED,IMMELMAN_ROLLACCELMODIFIER);
            }
            else
            {
                return flightmanRollRight(ship,&immelmaninfo->totalrollrotated,DEG_TO_RAD(180.0f),IMMELMAN_MAXROLLSPEED,IMMELMAN_ROLLACCELMODIFIER);
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }

}

/*=============================================================================
    Split-S
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 totalrollrotated;
    bool16 rollleft;
    bool16 inverted;
} SplitSInfo;

void flightmanSplitSInit(Ship *ship,sdword flags)
{
    SplitSInfo *splitsinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SPLIT_S);
    splitsinfo = ship->flightmanInfo = memAlloc(sizeof(SplitSInfo),"SplitSInfo",0);
    splitsinfo->size = sizeof(SplitSInfo);

    splitsinfo->totalpitchrotated = 0.0f;
    splitsinfo->totalrollrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    splitsinfo->rollleft = (bool16)(flags & 1);
    splitsinfo->inverted = (bool16)(flags & 2);
}

bool flightmanSplitSExecute(Ship *ship)
{
    SplitSInfo *splitsinfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_SPLIT_S);
    splitsinfo = (SplitSInfo *)ship->flightmanInfo;
    dbgAssert(splitsinfo->size == sizeof(SplitSInfo));

    switch (ship->flightmanState1)
    {
        case 0:

            if (splitsinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&splitsinfo->totalrollrotated,DEG_TO_RAD(180.0f),SPLIT_S_MAXROTSPEEDROLL,SPLIT_S_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&splitsinfo->totalrollrotated,DEG_TO_RAD(180.0f),SPLIT_S_MAXROTSPEEDROLL,SPLIT_S_ACCELMODIFIERROLL);
            }

            if (result)
            {
                ship->flightmanState1 = 1;
            }
            return FALSE;

        case 1:
            flightmanStabilizeRoll(ship);

            if (splitsinfo->inverted)
            {
                result = flightmanPitchDown(ship,&splitsinfo->totalpitchrotated,DEG_TO_RAD(180.0f),SPLIT_S_MAXROTSPEEDPITCH,SPLIT_S_ACCELMODIFIERFLIP);
            }
            else
            {
                result = flightmanPitchUp(ship,&splitsinfo->totalpitchrotated,DEG_TO_RAD(180.0f),SPLIT_S_MAXROTSPEEDPITCH,SPLIT_S_ACCELMODIFIERFLIP);
            }

            if (result)
            {
                return TRUE;
            }
            else
            {
                physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SPLIT_S_THRUSTMOD,TRANS_FORWARD);
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            return FALSE;

    }
}

/*=============================================================================
    Hardbank
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 totalrollrotated;
    bool rollleft;
} HardBankInfo;

void flightmanHardBankInit(Ship *ship,sdword flags)
{
    HardBankInfo *hardbankinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_HARDBANK);
    hardbankinfo = ship->flightmanInfo = memAlloc(sizeof(HardBankInfo),"HardBankInfo",0);
    hardbankinfo->size = sizeof(HardBankInfo);

    hardbankinfo->totalpitchrotated = 0.0f;
    hardbankinfo->totalrollrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    hardbankinfo->rollleft = flags & 1;
}

bool flightmanHardBankExecute(Ship *ship)
{
    HardBankInfo *hardbankinfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_HARDBANK);
    hardbankinfo = (HardBankInfo *)ship->flightmanInfo;
    dbgAssert(hardbankinfo->size == sizeof(HardBankInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (hardbankinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&hardbankinfo->totalrollrotated,DEG_TO_RAD(90.0f),HARDBANK_MAXROTSPEEDROLL,HARDBANK_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&hardbankinfo->totalrollrotated,DEG_TO_RAD(90.0f),HARDBANK_MAXROTSPEEDROLL,HARDBANK_ACCELMODIFIERROLL);
            }

            if (result)
            {
                ship->flightmanState1 = 1;
            }
            return FALSE;

        case 1:
            flightmanStabilizeRoll(ship);

            if (flightmanPitchUp(ship,&hardbankinfo->totalpitchrotated,DEG_TO_RAD(180.0f),HARDBANK_MAXROTSPEEDPITCH,HARDBANK_ACCELMODIFIERFLIP))
            {
                return TRUE;
            }
            else
            {
                physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HARDBANK_THRUSTMOD,TRANS_FORWARD);
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Softbank
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 totalrollrotated;
    bool rollleft;
} SoftBankInfo;

void flightmanSoftBankInit(Ship *ship,sdword flags)
{
    SoftBankInfo *softbankinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SOFTBANK);
    softbankinfo = ship->flightmanInfo = memAlloc(sizeof(SoftBankInfo),"SoftBankInfo",0);
    softbankinfo->size = sizeof(SoftBankInfo);

    softbankinfo->totalpitchrotated = 0.0f;
    softbankinfo->totalrollrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    softbankinfo->rollleft = flags & SOFTBANK_LEFT;
}

bool flightmanSoftBankExecute(Ship *ship)
{
    SoftBankInfo *softbankinfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_SOFTBANK);
    softbankinfo = (SoftBankInfo *)ship->flightmanInfo;
    dbgAssert(softbankinfo->size == sizeof(SoftBankInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (softbankinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&softbankinfo->totalrollrotated,DEG_TO_RAD(90.0f),SOFTBANK_MAXROTSPEEDROLL,SOFTBANK_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&softbankinfo->totalrollrotated,DEG_TO_RAD(90.0f),SOFTBANK_MAXROTSPEEDROLL,SOFTBANK_ACCELMODIFIERROLL);
            }

            if (result)
            {
                ship->flightmanState1 = 1;
            }
            return FALSE;

        case 1:
            flightmanStabilizeRoll(ship);

            if (flightmanPitchUp(ship,&softbankinfo->totalpitchrotated,DEG_TO_RAD(90.0f),SOFTBANK_MAXROTSPEEDPITCH,SOFTBANK_ACCELMODIFIERFLIP))
            {
                return TRUE;
            }
            else
            {
                physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SOFTBANK_THRUSTMOD,TRANS_FORWARD);
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Slalom
=============================================================================*/

typedef struct
{
    sdword size;
    sdword numwaypoints;
    vector waypoints[SLALOM_MAXITERATIONS];
    real32 timestamp;
} SlalomInfo;

void flightmanSlalomInit(Ship *ship,sdword flags)
{
    SlalomInfo *slalominfo;
    bool rightdirection;
    sdword i;
    vector position;
    vector heading;
    vector right;
    vector forwardright;
    vector forwardleft;

    dbgAssert(ship->flightman == FLIGHTMAN_SLALOM);
    slalominfo = ship->flightmanInfo = memAlloc(sizeof(SlalomInfo),"SlalomInfo",0);
    slalominfo->size = sizeof(SlalomInfo);

    if (flags < 0)
    {
        flags = gamerand();
    }

    slalominfo->numwaypoints = SLALOM_BASEITERATIONS + (flags & SLALOM_RANDOMITERATIONS);

    rightdirection = flags & 256;

    dbgAssert(slalominfo->numwaypoints >= 1);
    dbgAssert(SLALOM_BASEITERATIONS >= 1);

    dbgAssert(slalominfo->numwaypoints < SLALOM_MAXITERATIONS);
    dbgAssert((SLALOM_BASEITERATIONS + SLALOM_RANDOMITERATIONS - 1) < SLALOM_MAXITERATIONS);

    matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);

    vecAdd(forwardright,heading,right);
    vecSub(forwardleft,heading,right);

    vecMultiplyByScalar(forwardright,SLALOM_RUN_LENGTH);
    vecMultiplyByScalar(forwardleft,SLALOM_RUN_LENGTH);

    position = ship->posinfo.position;

    if (rightdirection)
    {
        vecAddTo(position,forwardright);
    }
    else
    {
        vecAddTo(position,forwardleft);
    }
    rightdirection = !rightdirection;

    slalominfo->waypoints[0] = position;

    for (i=1;i<slalominfo->numwaypoints;i++)
    {
        if (rightdirection)
        {
            vecAddTo(position,forwardright);
        }
        else
        {
            vecAddTo(position,forwardleft);
        }
        rightdirection = !rightdirection;

        slalominfo->waypoints[i] = position;
    }

    slalominfo->timestamp = universe.totaltimeelapsed;
}

bool flightmanSlalomExecute(Ship *ship)
{
    SlalomInfo *slalominfo;
    vector *curwaypoint;

    dbgAssert(ship->flightman == FLIGHTMAN_SLALOM);
    slalominfo = (SlalomInfo *)ship->flightmanInfo;
    dbgAssert(slalominfo->size == sizeof(SlalomInfo));

    dbgAssert(ship->flightmanState1 < slalominfo->numwaypoints);

    if ((universe.totaltimeelapsed - slalominfo->timestamp) > SLALOM_MAXWAYPOINTTIME)
    {
        goto nextstate;
    }

    curwaypoint = &slalominfo->waypoints[ship->flightmanState1];

    if (MoveReachedDestinationVariable(ship,curwaypoint,SLALOM_REACHDESTINATIONRANGE))
    {
        goto nextstate;
    }

    aishipFlyToPointAvoidingObjs(ship,curwaypoint,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,SLALOM_MINVELOCITY);
    return FALSE;

nextstate:
    ship->flightmanState1++;
    if (ship->flightmanState1 >= slalominfo->numwaypoints)
    {
        return TRUE;
    }
    else
    {
        slalominfo->timestamp = universe.totaltimeelapsed;
        return FALSE;
    }
}

/*=============================================================================
    Sandwich
=============================================================================*/

typedef struct
{
    sdword size;
    vector waypoint;
    real32 timestamp;
} SandwichInfo;

void flightmanSandwichInit(Ship *ship,sdword flags)
{
    Ship *target = (Ship *)flags;
    SandwichInfo *sandwichinfo;
    vector diff;
    real32 rotangle;
    matrix rotmatz;

    dbgAssert(target->objtype == OBJ_ShipType);

    dbgAssert(ship->flightman == FLIGHTMAN_SANDWICH);
    sandwichinfo = ship->flightmanInfo = memAlloc(sizeof(SandwichInfo),"SandwichInfo",0);
    sandwichinfo->size = sizeof(SandwichInfo);

    vecSub(diff,target->posinfo.position,ship->posinfo.position);
    vecMultiplyByScalar(diff,SANDWICH_PINCHER_SCALE);

    if (ship->flightmanState2 == SANDWICH_RIGHT)
    {
        rotangle = -SANDWICH_ANGLE;
    }
    else
    {
        rotangle = SANDWICH_ANGLE;
    }

    matMakeRotAboutZ(&rotmatz,(real32)cos(rotangle),(real32)sin(rotangle));
    matMultiplyMatByVec(&sandwichinfo->waypoint,&rotmatz,&diff);

    vecAddTo(sandwichinfo->waypoint,ship->posinfo.position);

    sandwichinfo->timestamp = universe.totaltimeelapsed;
}

bool flightmanSandwichExecute(Ship *ship)
{
    SandwichInfo *sandwichinfo = (SandwichInfo *)ship->flightmanInfo;
    dbgAssert(sandwichinfo->size == sizeof(SandwichInfo));

    dbgAssert(ship->flightman == FLIGHTMAN_SANDWICH);

    if ((universe.totaltimeelapsed - sandwichinfo->timestamp) > SANDWICH_MAXTIME)
    {
        return TRUE;
    }

    if (MoveReachedDestinationVariable(ship,&sandwichinfo->waypoint,SANDWICH_REACHDESTINATIONRANGE))
    {
        return TRUE;
    }

    aishipFlyToPointAvoidingObjs(ship,&sandwichinfo->waypoint,AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying,SANDWICH_MINVELOCITY);
    return FALSE;
}

/*=============================================================================
    Barrel roll
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalrotated;
    bool rollleft;
} RollInfo;

void flightmanBarrelRollInit(Ship *ship,sdword flags)
{
    RollInfo *barrelrollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_BARREL_ROLL);
    barrelrollinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"BRollInfo",0);
    barrelrollinfo->size = sizeof(RollInfo);

    barrelrollinfo->totalrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    barrelrollinfo->rollleft = flags & 1;
}

bool flightmanBarrelRollExecute(Ship *ship)
{
    RollInfo *barrelrollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_BARREL_ROLL);
    barrelrollinfo = (RollInfo *)ship->flightmanInfo;
    dbgAssert(barrelrollinfo->size == sizeof(RollInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (barrelrollinfo->rollleft)
                return(flightmanRollLeft(ship,&barrelrollinfo->totalrotated,DEG_TO_RAD(360.0f),BARREL_ROLL_MAXROTSPEEDROLL,BARREL_ROLL_ACCELMODIFIERROLL));
            else
                return(flightmanRollRight(ship,&barrelrollinfo->totalrotated,DEG_TO_RAD(360.0f),BARREL_ROLL_MAXROTSPEEDROLL,BARREL_ROLL_ACCELMODIFIERROLL));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Swarmer Barrel roll
=============================================================================*/

void flightmanSwarmerBRollInit(Ship *ship,sdword flags)
{
    RollInfo *sbrollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SWARMER_BARRELROLL);
    sbrollinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"SBRollInfo",0);
    sbrollinfo->size = sizeof(RollInfo);

    sbrollinfo->totalrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    sbrollinfo->rollleft = flags & 1;
}

bool flightmanSwarmerBRollExecute(Ship *ship)
{
    RollInfo *sbrollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SWARMER_BARRELROLL);
    sbrollinfo = (RollInfo *)ship->flightmanInfo;
    dbgAssert(sbrollinfo->size == sizeof(RollInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (sbrollinfo->rollleft)
                return(flightmanRollLeft(ship,&sbrollinfo->totalrotated,DEG_TO_RAD(720.0f),SWARMER_BARRELROLL_MAXROTSPEEDROLL,SWARMER_BARRELROLL_ACCELMODIFIERROLL));
            else
                return(flightmanRollRight(ship,&sbrollinfo->totalrotated,DEG_TO_RAD(720.0f),SWARMER_BARRELROLL_MAXROTSPEEDROLL,SWARMER_BARRELROLL_ACCELMODIFIERROLL));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Roll 180
=============================================================================*/

void flightmanRoll180Init(Ship *ship,sdword flags)
{
    RollInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_ROLL180);
    rollinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"Roll180Info",0);
    rollinfo->size = sizeof(RollInfo);

    rollinfo->totalrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    rollinfo->rollleft = flags & 1;
}

bool flightmanRoll180Execute(Ship *ship)
{
    RollInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_ROLL180);
    rollinfo = (RollInfo *)ship->flightmanInfo;
    dbgAssert(rollinfo->size == sizeof(RollInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (rollinfo->rollleft)
                return(flightmanRollLeft(ship,&rollinfo->totalrotated,DEG_TO_RAD(180.0f),ROLL180_MAXROTSPEEDROLL,ROLL180_ACCELMODIFIERROLL));
            else
                return(flightmanRollRight(ship,&rollinfo->totalrotated,DEG_TO_RAD(180.0f),ROLL180_MAXROTSPEEDROLL,ROLL180_ACCELMODIFIERROLL));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Barrelroll Out
=============================================================================*/

void flightmanBRollOutInit(Ship *ship,sdword flags)
{
    RollInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_BARRELROLL_OUT);
    rollinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"BRollOutInfo",0);
    rollinfo->size = sizeof(RollInfo);

    rollinfo->totalrotated = 0.0f;
    rollinfo->rollleft = FALSE;
}

bool flightmanBRollOutExecute(Ship *ship)
{
    RollInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_BARRELROLL_OUT);
    rollinfo = (RollInfo *)ship->flightmanInfo;
    dbgAssert(rollinfo->size == sizeof(RollInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * 0.50f, TRANS_UP);
            return(flightmanRollLeft(ship,&rollinfo->totalrotated,DEG_TO_RAD(180.0f), 1.70f, 0.25f));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Endover (for swarmers)
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalrotated;
    real32 amount_to_rotate;
    real32 increment;
    bool rollleft;
} EndoverInfo;

void flightmanEndoverInit(Ship *ship,sdword flags)
{
    EndoverInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_ENDOVER);
    rollinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"EndoverInfo",0);
    rollinfo->size = sizeof(EndoverInfo);

    rollinfo->totalrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }
    rollinfo->rollleft = flags & 1;

    rollinfo->amount_to_rotate = DEG_TO_RAD(540.0f + (((gamerand() & 3) - 1.5f) * 60.0f));  // can be 90 degrees off
    rollinfo->increment = ENDOVER_MAXROTSPEED;
}

bool flightmanEndoverExecute(Ship *ship)
{
    EndoverInfo *rollinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_ENDOVER);
    rollinfo = (EndoverInfo *)ship->flightmanInfo;
    dbgAssert(rollinfo->size == sizeof(EndoverInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            rollinfo->increment -= 0.2f;

            if (rollinfo->rollleft)
                return(flightmanYawLeft(ship,&rollinfo->totalrotated, rollinfo->amount_to_rotate, rollinfo->increment, ENDOVER_ACCELMODIFIER));
            else
                return(flightmanYawRight(ship,&rollinfo->totalrotated, rollinfo->amount_to_rotate, rollinfo->increment, ENDOVER_ACCELMODIFIER));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Celebration flip
=============================================================================*/

void flightmanCelebFlipInit(Ship *ship,sdword flags)
{
    RollInfo *cfinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_CELEB_FLIP);
    cfinfo = ship->flightmanInfo = memAlloc(sizeof(RollInfo),"CelebFlipInfo",0);
    cfinfo->size = sizeof(RollInfo);

    cfinfo->totalrotated = 0.0f;
    cfinfo->rollleft = FALSE;
}

bool flightmanCelebFlipExecute(Ship *ship)
{
    RollInfo *cfinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_CELEB_FLIP);
    cfinfo = (RollInfo *)ship->flightmanInfo;
    dbgAssert(cfinfo->size == sizeof(RollInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            return(flightmanPitchUp(ship,&cfinfo->totalrotated,DEG_TO_RAD(360.0f), CELEB_FLIP_MAXROTSPEEDROLL, CELEB_FLIP_ACCELMODIFIERROLL));

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Whip strafe flight maneuver
=============================================================================*/

typedef struct
{
    sdword size;
    real32 whiptimestamp;
//    real32 totalpitchrotated;
} WhipStrafeInfo;

void flightmanWhipStrafeInit(Ship *ship,sdword flags)
{
    WhipStrafeInfo *whipstrafeinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_WHIP_STRAFE);
    whipstrafeinfo = ship->flightmanInfo = memAlloc(sizeof(WhipStrafeInfo),"WhipStrafeInfo",0);
    whipstrafeinfo->size = sizeof(WhipStrafeInfo);

    whipstrafeinfo->whiptimestamp = 0.0f;

//    whipstrafeinfo->totalpitchrotated = 0.0f;
}

bool flightmanWhipStrafeExecute(Ship *ship)
{
    WhipStrafeInfo *whipstrafeinfo;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    vector updownvector = { 0.0f, 0.0f, 1.0f };

    dbgAssert(ship->flightman == FLIGHTMAN_WHIP_STRAFE);
    whipstrafeinfo = (WhipStrafeInfo *)ship->flightmanInfo;
    dbgAssert(whipstrafeinfo->size == sizeof(WhipStrafeInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (ship->flightmanState2 & WHIPSTRAFE_ABOVE)
            {
                // coming from above, so we want to face down
                updownvector.z = -1.0f;
            }

            if (aitrackHeadingWithBankFlags(ship,&updownvector,WHIPSTRAFE_FACEDOWNUP_ACCURACY,shipstaticinfo->sinbank,AITRACKHEADING_IGNOREUPVEC))
            {
                ship->flightmanState1 = 1;
                whipstrafeinfo->whiptimestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 1:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * WHIPSTRAFE_WHIPTHRUSTMOD,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - whipstrafeinfo->whiptimestamp) > WHIPSTRAFE_WHIPTIME)
            {
                ship->flightmanState1 = 2;
            }
            return FALSE;

        case 2:
            return TRUE;
#if 0
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * WHIPSTRAFE_LEVELOUTTHRUSTMOD,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&whipstrafeinfo->totalpitchrotated,DEG_TO_RAD(90.0f),WHIPSTRAFE_LEVEL_MAXROT,WHIPSTRAFE_LEVEL_ACCELMOD))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
#endif
        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Roll away
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalrollrotated;
    real32 upsidedowntimestamp;
    bool rollleft;
} RollAwayInfo;

void flightmanRollAwayInit(Ship *ship,sdword flags)
{
    RollAwayInfo *rollawayinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_ROLLAWAY);
    rollawayinfo = ship->flightmanInfo = memAlloc(sizeof(RollAwayInfo),"RollAwayInfo",0);
    rollawayinfo->size = sizeof(RollAwayInfo);

    if (flags < 0)
    {
        flags = gamerand();
    }

    rollawayinfo->totalrollrotated = 0.0f;
    rollawayinfo->upsidedowntimestamp = 0.0f;

    rollawayinfo->rollleft = flags & 1;
}

bool flightmanRollAwayExecute(Ship *ship)
{
    RollAwayInfo *rollawayinfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_ROLLAWAY);
    rollawayinfo = (RollAwayInfo *)ship->flightmanInfo;
    dbgAssert(rollawayinfo->size == sizeof(RollAwayInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (rollawayinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&rollawayinfo->totalrollrotated,DEG_TO_RAD(180.0f),ROLLAWAY_MAXROTSPEEDROLL,ROLLAWAY_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&rollawayinfo->totalrollrotated,DEG_TO_RAD(180.0f),ROLLAWAY_MAXROTSPEEDROLL,ROLLAWAY_ACCELMODIFIERROLL);
            }

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_UP] * ROLLAWAY_THRUSTMOD,TRANS_UP);

            if (result)
            {
                ship->flightmanState1 = 1;
                rollawayinfo->upsidedowntimestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 1:
            flightmanStabilizeRoll(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_UP] * ROLLAWAY_THRUSTMOD,TRANS_UP);

            if ((universe.totaltimeelapsed - rollawayinfo->upsidedowntimestamp) > ROLLAWAY_THRUSTDOWNTIME)
            {
                ship->flightmanState1 = 2;
            }
            return FALSE;

        case 2:
            if (rollawayinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&rollawayinfo->totalrollrotated,DEG_TO_RAD(360.0f),ROLLAWAY_MAXROTSPEEDROLL,ROLLAWAY_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&rollawayinfo->totalrollrotated,DEG_TO_RAD(360.0f),ROLLAWAY_MAXROTSPEEDROLL,ROLLAWAY_ACCELMODIFIERROLL);
            }

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_UP] * ROLLAWAY_THRUSTMOD,TRANS_UP);

            if (result)
            {
                return TRUE;
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    SplitS Evasive
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 totalrollrotated;
    bool rollleft;
} SplitSEvasiveInfo;

void flightmanSplitSEvasiveInit(Ship *ship,sdword flags)
{
    SplitSEvasiveInfo *splitsevasiveinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SPLITS_EVASIVE);
    splitsevasiveinfo = ship->flightmanInfo = memAlloc(sizeof(SplitSEvasiveInfo),"SplitSEvasiveInfo",0);
    splitsevasiveinfo->size = sizeof(SplitSEvasiveInfo);

    splitsevasiveinfo->totalpitchrotated = 0.0f;
    splitsevasiveinfo->totalrollrotated = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    splitsevasiveinfo->rollleft = flags & 1;
}

bool flightmanSplitSEvasiveExecute(Ship *ship)
{
    SplitSEvasiveInfo *splitsevasiveinfo;
    bool result;

    dbgAssert(ship->flightman == FLIGHTMAN_SPLITS_EVASIVE);
    splitsevasiveinfo = (SplitSEvasiveInfo *)ship->flightmanInfo;
    dbgAssert(splitsevasiveinfo->size == sizeof(SplitSEvasiveInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (splitsevasiveinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&splitsevasiveinfo->totalrollrotated,DEG_TO_RAD(90.0f),SPLITSEVASIVE_MAXROTSPEEDROLL,SPLITSEVASIVE_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&splitsevasiveinfo->totalrollrotated,DEG_TO_RAD(90.0f),SPLITSEVASIVE_MAXROTSPEEDROLL,SPLITSEVASIVE_ACCELMODIFIERROLL);
            }

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SPLITSEVASIVE_THRUSTMOD,TRANS_FORWARD);

            if (result)
            {
                ship->flightmanState1 = 1;
            }
            return FALSE;

        case 1:
            flightmanStabilizeRoll(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SPLITSEVASIVE_THRUSTMOD,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&splitsevasiveinfo->totalpitchrotated,DEG_TO_RAD(90.0f),SPLITSEVASIVE_MAXROTSPEEDPITCH,SPLITSEVASIVE_ACCELMODIFIERPITCH))
            {
                ship->flightmanState1 = 2;
            }
            return FALSE;

        case 2:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SPLITSEVASIVE_THRUSTMOD,TRANS_FORWARD);

            if (splitsevasiveinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&splitsevasiveinfo->totalrollrotated,DEG_TO_RAD(180.0f),SPLITSEVASIVE_MAXROTSPEEDROLL,SPLITSEVASIVE_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&splitsevasiveinfo->totalrollrotated,DEG_TO_RAD(180.0f),SPLITSEVASIVE_MAXROTSPEEDROLL,SPLITSEVASIVE_ACCELMODIFIERROLL);
            }

            if (result)
            {
                ship->flightmanState1 = 3;
                splitsevasiveinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 3:
            flightmanStabilizeRoll(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SPLITSEVASIVE_THRUSTMOD,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&splitsevasiveinfo->totalpitchrotated,DEG_TO_RAD(180.0f),SPLITSEVASIVE_MAXROTSPEEDPITCH,SPLITSEVASIVE_ACCELMODIFIERPITCH))
            {
                return TRUE;
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    High yo-yo
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 timestamp;
} HiYoYoInfo;

void flightmanHiYoYoInit(Ship *ship,sdword flags)
{
    HiYoYoInfo *hiyoyoinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_HIYOYO);
    hiyoyoinfo = ship->flightmanInfo = memAlloc(sizeof(HiYoYoInfo),"HiYoYoInfo",0);
    hiyoyoinfo->size = sizeof(HiYoYoInfo);

    hiyoyoinfo->timestamp = 0.0f;
    hiyoyoinfo->totalpitchrotated = 0.0f;
}

bool flightmanHiYoYoExecute(Ship *ship)
{
    HiYoYoInfo *hiyoyoinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_HIYOYO);
    hiyoyoinfo = (HiYoYoInfo *)ship->flightmanInfo;
    dbgAssert(hiyoyoinfo->size == sizeof(HiYoYoInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HIYOYO_THRUSTMOD0,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&hiyoyoinfo->totalpitchrotated,DEG_TO_RAD(90.0f),HIYOYO_MAXROTSPEEDPITCH,HIYOYO_ACCELMODIFIERPITCH))
            {
                ship->flightmanState1 = 1;
                hiyoyoinfo->timestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 1:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HIYOYO_THRUSTMOD1,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - hiyoyoinfo->timestamp) > HIYOYO_GOUP_TIME)
            {
                ship->flightmanState1 = 2;
                hiyoyoinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 2:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HIYOYO_THRUSTMOD2,TRANS_FORWARD);

            if (flightmanPitchDown(ship,&hiyoyoinfo->totalpitchrotated,DEG_TO_RAD(180.0f),HIYOYO_MAXROTSPEEDPITCH,HIYOYO_ACCELMODIFIERPITCH))
            {
                ship->flightmanState1 = 3;
                hiyoyoinfo->timestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 3:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HIYOYO_THRUSTMOD3,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - hiyoyoinfo->timestamp) > HIYOYO_GODOWN_TIME)
            {
                ship->flightmanState1 = 4;
                hiyoyoinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 4:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * HIYOYO_THRUSTMOD4,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&hiyoyoinfo->totalpitchrotated,DEG_TO_RAD(90.0f),HIYOYO_MAXROTSPEEDPITCH,HIYOYO_ACCELMODIFIERPITCH))
            {
                return TRUE;
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            break;
    }

    return FALSE;
}

/*=============================================================================
    Low yo-yo
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalpitchrotated;
    real32 timestamp;
} LoYoYoInfo;

void flightmanLoYoYoInit(Ship *ship,sdword flags)
{
    LoYoYoInfo *loyoyoinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_LOYOYO);
    loyoyoinfo = ship->flightmanInfo = memAlloc(sizeof(LoYoYoInfo),"LoYoYoInfo",0);
    loyoyoinfo->size = sizeof(LoYoYoInfo);

    loyoyoinfo->timestamp = 0.0f;
    loyoyoinfo->totalpitchrotated = 0.0f;
}

bool flightmanLoYoYoExecute(Ship *ship)
{
    LoYoYoInfo *loyoyoinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_LOYOYO);
    loyoyoinfo = (LoYoYoInfo *)ship->flightmanInfo;
    dbgAssert(loyoyoinfo->size == sizeof(LoYoYoInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * LOYOYO_THRUSTMOD0,TRANS_FORWARD);

            if (flightmanPitchDown(ship,&loyoyoinfo->totalpitchrotated,DEG_TO_RAD(90.0f),LOYOYO_MAXROTSPEEDPITCH,LOYOYO_ACCELMODIFIERPITCH))
            {
                ship->flightmanState1 = 1;
                loyoyoinfo->timestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 1:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * LOYOYO_THRUSTMOD1,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - loyoyoinfo->timestamp) > LOYOYO_GODOWN_TIME)
            {
                ship->flightmanState1 = 2;
                loyoyoinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 2:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * LOYOYO_THRUSTMOD2,TRANS_FORWARD);

            if (flightmanPitchUp(ship,&loyoyoinfo->totalpitchrotated,DEG_TO_RAD(180.0f),LOYOYO_MAXROTSPEEDPITCH,LOYOYO_ACCELMODIFIERPITCH))
            {
                ship->flightmanState1 = 3;
                loyoyoinfo->timestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 3:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * LOYOYO_THRUSTMOD3,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - loyoyoinfo->timestamp) > LOYOYO_GOUP_TIME)
            {
                ship->flightmanState1 = 4;
                loyoyoinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 4:
            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * LOYOYO_THRUSTMOD4,TRANS_FORWARD);

            if (flightmanPitchDown(ship,&loyoyoinfo->totalpitchrotated,DEG_TO_RAD(90.0f),LOYOYO_MAXROTSPEEDPITCH,LOYOYO_ACCELMODIFIERPITCH))
            {
                return TRUE;
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            break;
    }

    return FALSE;
}

/*=============================================================================
    Sidestep
=============================================================================*/

typedef struct
{
    sdword size;
    real32 totalrollrotated;
    real32 totalpitchrotated;
    real32 totalyawrotated;
    real32 timestamp;
    bool rollleft;
    bool yawleft;
} SideStepInfo;

void flightmanSideStepInit(Ship *ship,sdword flags)
{
    SideStepInfo *sidestepinfo;

    dbgAssert(ship->flightman == FLIGHTMAN_SIDESTEP);
    sidestepinfo = ship->flightmanInfo = memAlloc(sizeof(SideStepInfo),"SideStepInfo",0);
    sidestepinfo->size = sizeof(SideStepInfo);

    sidestepinfo->totalrollrotated = 0.0f;
    sidestepinfo->totalpitchrotated = 0.0f;
    sidestepinfo->totalyawrotated = 0.0f;
    sidestepinfo->timestamp = 0.0f;

    if (flags < 0)
    {
        flags = gamerand();
    }

    sidestepinfo->rollleft = flags & 1;
    sidestepinfo->yawleft = flags & 2;
}

bool flightmanSideStepExecute(Ship *ship)
{
    SideStepInfo *sidestepinfo;
    bool result;
    bool pitchresult;

    dbgAssert(ship->flightman == FLIGHTMAN_SIDESTEP);
    sidestepinfo = (SideStepInfo *)ship->flightmanInfo;
    dbgAssert(sidestepinfo->size == sizeof(SideStepInfo));

    switch (ship->flightmanState1)
    {
        case 0:
            if (sidestepinfo->rollleft)
            {
                result = flightmanRollLeft(ship,&sidestepinfo->totalrollrotated,SIDESTEP_ROLLROTATE,SIDESTEP_MAXROTSPEEDROLL,SIDESTEP_ACCELMODIFIERROLL);
            }
            else
            {
                result = flightmanRollRight(ship,&sidestepinfo->totalrollrotated,SIDESTEP_ROLLROTATE,SIDESTEP_MAXROTSPEEDROLL,SIDESTEP_ACCELMODIFIERROLL);
            }

            if (result)
            {
                flightmanStabilizeRoll(ship);
            }

            pitchresult = flightmanPitchUp(ship,&sidestepinfo->totalpitchrotated,SIDESTEP_PITCHROTATE,SIDESTEP_MAXROTSPEEDPITCH,SIDESTEP_ACCELMODIFIERPITCH);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SIDESTEP_THRUSTMODTURN,TRANS_FORWARD);

            if (result & pitchresult)
            {
                ship->flightmanState1 = 2;
                sidestepinfo->timestamp = universe.totaltimeelapsed;
//                sidestepinfo->totalpitchrotated = 0.0f;
            }
            return FALSE;

        case 2:
            flightmanStabilizePitch(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SIDESTEP_THRUSTMOD,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - sidestepinfo->timestamp) > SIDESTEP_PAUSETIME)
            {
                ship->flightmanState1 = 3;
            }
            return FALSE;

        case 3:
            if (sidestepinfo->yawleft)
            {
                result = flightmanYawLeft(ship,&sidestepinfo->totalyawrotated,DEG_TO_RAD(180.0f),SIDESTEP_MAXROTSPEEDYAW,SIDESTEP_ACCELMODIFIERYAW);
            }
            else
            {
                result = flightmanYawRight(ship,&sidestepinfo->totalyawrotated,DEG_TO_RAD(180.0f),SIDESTEP_MAXROTSPEEDYAW,SIDESTEP_ACCELMODIFIERYAW);
            }

            if (result)
            {
                ship->flightmanState1 = 4;
                sidestepinfo->timestamp = universe.totaltimeelapsed;
            }
            return FALSE;

        case 4:
            flightmanStabilizeYaw(ship);

            physApplyForceToObj((SpaceObj *)ship,ship->nonstatvars.thruststrength[TRANS_FORWARD] * SIDESTEP_THRUSTMOD,TRANS_FORWARD);

            if ((universe.totaltimeelapsed - sidestepinfo->timestamp) > SIDESTEP_BOOTITTIME)
            {
                return TRUE;
            }
            return FALSE;

        default:
            dbgAssert(FALSE);
            break;
    }

    return FALSE;
}

/*=============================================================================
    Flight maneuver intialization, execution, close commands
=============================================================================*/

void flightmanInitFunc(Ship *ship,udword flightman,sdword flags)
{
    FlightmanSpecificInit flightmanSpecificInit;

    dbgAssert(flightman < NUM_FLIGHTMANEUVERS);
    dbgAssert(flightman != FLIGHTMAN_NULL);

    if (ship->flightman != FLIGHTMAN_NULL)
    {
        sdword temp = ship->tacticsTalk;
        flightmanClose(ship);
        ship->tacticsTalk = temp;
    }

#ifdef FLIGHTMAN_DEBUG
    dbgMessagef("\nStarting flightman %d for Ship %x",flightman,(udword)ship);
#endif

//    speechEvent(ship,ManeuverSet,flightman);

    ship->flightman = flightman;
    ship->flightmanState1 = 0;
    // deliberately do not intialize ship->flightmanState2 because it can be set as a parameter
    dbgAssert(ship->flightmanInfo == NULL);

    flightmanSpecificInit = flightmanSpecificInitTable[flightman];
    if (flightmanSpecificInit != NULL)
    {
        flightmanSpecificInit(ship,flags);
    }

    if (!ship->shipisattacking)
    {
        ship->shipisattacking = TRUE;
        //dbgMessagef("\nShip %i is attacking (flightman)", ship->shipID.shipNumber);
    }
    if ((ship->attackvars.myWingmanIs)&&(!ship->attackvars.myWingmanIs->shipisattacking))
    {
        ship->attackvars.myWingmanIs->shipisattacking = TRUE;
        //dbgMessagef("\nShip %i is attacking (flightman - wingman)", ship->attackvars.myWingmanIs->shipID.shipNumber);
    }
}

void flightmanClose(Ship *ship)
{
    FlightmanSpecificClose flightmanSpecificClose;

    dbgAssert(ship->flightman < NUM_FLIGHTMANEUVERS);
    dbgAssert(ship->flightman != FLIGHTMAN_NULL);

#ifdef FLIGHTMAN_DEBUG
    dbgMessagef("\nEnding flightman %d for Ship %x",ship->flightman,(udword)ship);
#endif

    flightmanSpecificClose = flightmanSpecificCloseTable[ship->flightman];
    if (flightmanSpecificClose != NULL)
    {
        flightmanSpecificClose(ship);
    }
    if (ship->flightmanInfo)
    {
        memFree(ship->flightmanInfo);
        ship->flightmanInfo = NULL;
    }
    ship->flightman = FLIGHTMAN_NULL;
    ship->tacticsNeedToFlightMan = FALSE;
    ship->tacticsTalk = 0;
}

bool flightmanExecute(Ship *ship)
{
    dbgAssert(ship->flightman < NUM_FLIGHTMANEUVERS);
    if (ship->flightman == FLIGHTMAN_NULL)
    {
        // should this be happening?
        dbgMessage("\nWarning: tried to execute NULL flightman");
//        dbgAssert(FALSE);
        return TRUE;
    }
    dbgAssert(flightmanSpecificExecuteTable[ship->flightman] != NULL);
    if ((flightmanSpecificExecuteTable[ship->flightman])(ship))
    {
        flightmanClose(ship);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

udword flightmanGetRandom(FlightManProb *prob,udword flightmanSubtype)
{
    udword randnum;
    udword i;

    dbgAssert(prob->valid == FLIGHTPROB_VALID);

    switch (flightmanSubtype)
    {
        case FLIGHTMAN_EVASIVE_BEHIND:
            if (prob->sumtotalEvasiveBehind != 0)
            {
                randnum = randomG(prob->sumtotalEvasiveBehind);

                for (i=0;i<FLIGHTMAN_TYPE_EVASIVE_NUM;i++)
                {
                    if (randnum < prob->flightcumEvasiveBehind[i])
                    {
                        return (i+FLIGHTMAN_TYPE_EVASIVE_START);
                    }
                }
                dbgAssert(FALSE);
            }
            break;

        case FLIGHTMAN_EVASIVE_FRONT:
            if (prob->sumtotalEvasiveFront != 0)
            {
                randnum = randomG(prob->sumtotalEvasiveFront);

                for (i=0;i<FLIGHTMAN_TYPE_EVASIVE_NUM;i++)
                {
                    if (randnum < prob->flightcumEvasiveFront[i])
                    {
                        return (i+FLIGHTMAN_TYPE_EVASIVE_START);
                    }
                }
                dbgAssert(FALSE);
            }
            break;

        case FLIGHTMAN_EVASIVE_PURE:
            if (prob->sumtotalEvasivePure != 0)
            {
                randnum = randomG(prob->sumtotalEvasivePure);

                for (i=0;i<FLIGHTMAN_TYPE_EVASIVE_NUM;i++)
                {
                    if (randnum < prob->flightcumEvasivePure[i])
                    {
                        return (i+FLIGHTMAN_TYPE_EVASIVE_START);
                    }
                }
                dbgAssert(FALSE);
            }
            break;

        case FLIGHTMAN_TURNAROUND:
            if (prob->sumtotalTurnaround != 0)
            {
                randnum = randomG(prob->sumtotalTurnaround);

                for (i=0;i<FLIGHTMAN_TYPE_TURNAROUND_NUM;i++)
                {
                    if (randnum < prob->flightcumTurnaround[i])
                    {
                        return (i+FLIGHTMAN_TYPE_TURNAROUND_START);
                    }
                }
                dbgAssert(FALSE);
            }
            break;

        case FLIGHTMAN_AIP:
            if (prob->sumtotalAIP != 0)
            {
                randnum = randomG(prob->sumtotalAIP);

                for (i=0;i<FLIGHTMAN_TYPE_AIP_NUM;i++)
                {
                    if (randnum < prob->flightcumAIP[i])
                    {
                        return (i+FLIGHTMAN_TYPE_AIP_START);
                    }
                }
                dbgAssert(FALSE);
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    return FLIGHTMAN_DONOTHING;
}

bool flightmanTestRandom(FlightManProb *prob,udword flightmanSubtype,udword flightman)
{
    udword sumtotal;
    udword randnum;

    dbgAssert(prob->valid == FLIGHTPROB_VALID);

    switch (flightmanSubtype)
    {
        case FLIGHTMAN_TURNAROUND:
            dbgAssert(flightman >= FLIGHTMAN_TYPE_TURNAROUND_START);
            dbgAssert(flightman <= FLIGHTMAN_TYPE_TURNAROUND_END);
            sumtotal = prob->sumtotalTurnaround;
            if (sumtotal == 0) return FALSE;

            randnum = randomG(sumtotal);

            if (randnum < prob->flightprobTurnaround[flightman-FLIGHTMAN_TYPE_TURNAROUND_START])
            {
                return TRUE;
            }
            break;

        case FLIGHTMAN_AIP:
            dbgAssert(flightman >= FLIGHTMAN_TYPE_AIP_START);
            dbgAssert(flightman <= FLIGHTMAN_TYPE_AIP_END);
            sumtotal = prob->sumtotalAIP;
            if (sumtotal == 0) return FALSE;

			randnum = randomG(sumtotal);

            if (randnum < prob->flightprobAIP[flightman-FLIGHTMAN_TYPE_AIP_START])
            {
                return TRUE;
            }
            break;

        case FLIGHTMAN_EVASIVE_BEHIND:
            dbgAssert(flightman >= FLIGHTMAN_TYPE_EVASIVE_START);
            dbgAssert(flightman <= FLIGHTMAN_TYPE_EVASIVE_END);
            sumtotal = prob->sumtotalEvasiveBehind;
            if (sumtotal == 0) return FALSE;

            randnum = randomG(sumtotal);

            if (randnum < prob->flightprobEvasiveBehind[flightman-FLIGHTMAN_TYPE_EVASIVE_START])
            {
                return TRUE;
            }
            break;

        case FLIGHTMAN_EVASIVE_FRONT:
            dbgAssert(flightman >= FLIGHTMAN_TYPE_EVASIVE_START);
            dbgAssert(flightman <= FLIGHTMAN_TYPE_EVASIVE_END);
            sumtotal = prob->sumtotalEvasiveFront;
            if (sumtotal == 0) return FALSE;

            randnum = randomG(sumtotal);

            if (randnum < prob->flightprobEvasiveFront[flightman-FLIGHTMAN_TYPE_EVASIVE_START])
            {
                return TRUE;
            }
            break;

        case FLIGHTMAN_EVASIVE_PURE:
            dbgAssert(flightman >= FLIGHTMAN_TYPE_EVASIVE_START);
            dbgAssert(flightman <= FLIGHTMAN_TYPE_EVASIVE_END);
            sumtotal = prob->sumtotalEvasivePure;
            if (sumtotal == 0) return FALSE;

            randnum = randomG(sumtotal);

            if (randnum < prob->flightprobEvasivePure[flightman-FLIGHTMAN_TYPE_EVASIVE_START])
            {
                return TRUE;
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    return FALSE;
}

char *strGetNextValue(char *field)
{
    while ((*field != '\0') && (*field != '\n'))
    {
        if ((*field == ' ') || (*field == '\t'))
        {
            goto foundwhitespace;
        }
        field++;
    }
    return NULL;

foundwhitespace:
    while ((*field != '\0') && (*field != '\n'))
    {
        if ((*field != ' ') && (*field != '\t'))
        {
            return field;
        }
        field++;
    }
    return NULL;
}

void setFlightManProb(char *field,udword *flightprobs,udword *flightcums,udword *sumtotal,udword num)
{
    udword i;
    udword *thisprob;
    udword *thiscum;
    udword cum;

    RemoveCommasFromString(field);

    for (i=0,thisprob=flightprobs;i<num;i++,thisprob++)
    {
        sscanf(field,"%d",thisprob);
        field = strGetNextValue(field);
        if (field == NULL)
        {
            break;
        }
    }

    for (i=0,cum=0,thisprob=flightprobs,thiscum=flightcums;i<num;i++,thisprob++,thiscum++)
    {
        cum += *thisprob;
        *thiscum = cum;
    }
    *sumtotal = cum;
}

void scriptSetFlightManTurnaroundCB(char *directory,char *field, FlightManProb *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    char field2[32];
    FlightManProb *prob;

    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %s",tactic_buffer, class_buffer, field2);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    prob = dataToFillIn;
    setFlightManProb(field2,prob->flightprobTurnaround,prob->flightcumTurnaround,&prob->sumtotalTurnaround,FLIGHTMAN_TYPE_TURNAROUND_NUM);
    prob->valid = FLIGHTPROB_VALID;
}

void scriptSetFlightManAIPCB(char *directory,char *field,FlightManProb *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    char field2[32];
    FlightManProb *prob;

    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %s",tactic_buffer, class_buffer, field2);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    prob = dataToFillIn;
    setFlightManProb(field2,prob->flightprobAIP,prob->flightcumAIP,&prob->sumtotalAIP,FLIGHTMAN_TYPE_AIP_NUM);
    prob->valid = FLIGHTPROB_VALID;
}

void scriptSetFlightManEvasiveBehindCB(char *directory,char *field,FlightManProb *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    char field2[32];
    FlightManProb *prob;

    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %s",tactic_buffer, class_buffer, field2);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    prob = dataToFillIn;
    setFlightManProb(field2,prob->flightprobEvasiveBehind,prob->flightcumEvasiveBehind,&prob->sumtotalEvasiveBehind,FLIGHTMAN_TYPE_TURNAROUND_NUM);
    prob->valid = FLIGHTPROB_VALID;
}

void scriptSetFlightManEvasiveFrontCB(char *directory,char *field,FlightManProb *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    char field2[32];
    FlightManProb *prob;

    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %s",tactic_buffer, class_buffer, field2);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    prob = dataToFillIn;
    setFlightManProb(field2,prob->flightprobEvasiveFront,prob->flightcumEvasiveFront,&prob->sumtotalEvasiveFront,FLIGHTMAN_TYPE_TURNAROUND_NUM);
    prob->valid = FLIGHTPROB_VALID;
}

void scriptSetFlightManEvasivePureCB(char *directory,char *field,FlightManProb *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    char field2[32];
    FlightManProb *prob;

    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %s",tactic_buffer, class_buffer, field2);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    prob = dataToFillIn;
    setFlightManProb(field2,prob->flightprobEvasivePure,prob->flightcumEvasivePure,&prob->sumtotalEvasivePure,FLIGHTMAN_TYPE_TURNAROUND_NUM);
    prob->valid = FLIGHTPROB_VALID;
}

/*=============================================================================
    Function table of flight maneuvers
=============================================================================*/

static FlightmanSpecificInit flightmanSpecificInitTable[NUM_FLIGHTMANEUVERS] =
{
  NULL,
  NULL,
  flightmanFlipTurnInit,
  flightmanHardBankInit,
  flightmanImmelmanInit,
  flightmanSplitSInit,
  flightmanCorkscrewInit,
  flightmanEndoverInit,
  NULL,
  flightmanBarrelRollInit,
  flightmanWhipStrafeInit,
  flightmanCelebFlipInit,
  flightmanBRollOutInit,
  flightmanSwarmerBRollInit,
  flightmanRoll180Init,
  flightmanRollAwayInit,
  flightmanSoftBankInit,
  flightmanSplitSEvasiveInit,
  flightmanSlalomInit,
  flightmanSideStepInit,
  flightmanLoYoYoInit,
  flightmanHiYoYoInit,
  NULL,
  flightmanSandwichInit,
  NULL,
  NULL
};

static FlightmanSpecificExecute flightmanSpecificExecuteTable[NUM_FLIGHTMANEUVERS] =
{
  NULL,
  flightmanDoNothingExecute,
  flightmanFlipTurnExecute,
  flightmanHardBankExecute,
  flightmanImmelmanExecute,
  flightmanSplitSExecute,
  flightmanCorkscrewExecute,
  flightmanEndoverExecute,
  NULL,
  flightmanBarrelRollExecute,
  flightmanWhipStrafeExecute,
  flightmanCelebFlipExecute,
  flightmanBRollOutExecute,
  flightmanSwarmerBRollExecute,
  flightmanRoll180Execute,
  flightmanRollAwayExecute,
  flightmanSoftBankExecute,
  flightmanSplitSEvasiveExecute,
  flightmanSlalomExecute,
  flightmanSideStepExecute,
  flightmanLoYoYoExecute,
  flightmanHiYoYoExecute,
  NULL,
  flightmanSandwichExecute,
  NULL,
  NULL
};

static FlightmanSpecificClose flightmanSpecificCloseTable[NUM_FLIGHTMANEUVERS] =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
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

