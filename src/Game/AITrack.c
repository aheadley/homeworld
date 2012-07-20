/*=============================================================================
    Name    : AITrack.c
    Purpose : Given a desired velocity, heading, etc. this AI layer will
              make the ship track the desired velocity, heading, etc.

    Created 6/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include "Types.h"
#include "Debug.h"
#include "Vector.h"
#include "Matrix.h"
#include "FastMath.h"
#include "Physics.h"
#include "SpaceObj.h"
#include "AITrack.h"
#include "StatScript.h"
#include "Universe.h"
#include "AIShip.h"

#ifdef gshaw
#define DEBUG_AITRACK 0
#else
#define DEBUG_AITRACK 0
#endif

/*=============================================================================
    Tweakables
=============================================================================*/

real32 ERROR_VELOCITY_SQUARED   = 0.05f;
real32 STILL_VELOCITY_LO        = -3.0f;
real32 STILL_VELOCITY_HI        = 3.0f;
real32 STILL_ROT_LO             = -0.3f;
real32 STILL_ROT_HI             = 0.3f;
real32 STABILIZE_Z_LO           = -0.1f;
real32 STABILIZE_Z_HI           = 0.1f;

real32 AITRACKRIGHT_UPACCURACY = 0.985f;

scriptEntry AITrackTweaks[] =
{
    { "ErrorVelocitySquared",   scriptSetReal32CB, &ERROR_VELOCITY_SQUARED },
    { "StillVelocityLo",        scriptSetReal32CB, &STILL_VELOCITY_LO },
    { "StillVelocityHi",        scriptSetReal32CB, &STILL_VELOCITY_HI },
    { "StillRotLo",             scriptSetReal32CB, &STILL_ROT_LO },
    { "StillRotHi",             scriptSetReal32CB, &STILL_ROT_HI },
    makeEntry(STABILIZE_Z_LO,scriptSetReal32CB),
    makeEntry(STABILIZE_Z_HI,scriptSetReal32CB),
    makeEntry(AITRACKRIGHT_UPACCURACY,scriptSetCosAngCB),
    { NULL, NULL, 0 }
};

/*-----------------------------------------------------------------------------
    Name        : aitrackIsStabilizeShip
    Description : Returns TRUE if ship is stablized
    Inputs      : ship
    Outputs     :
    Return      : Returns TRUE if ship is stablized
----------------------------------------------------------------------------*/
bool aitrackIsStabilizeShip(Ship *ship)
{
    vector heading,up,right;
    bool stabilizedpitch;
    real32 rotx = ship->rotinfo.rotspeed.x;

    matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(up,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);

    stabilizedpitch = isBetweenExclusive(heading.z,STABILIZE_Z_LO,STABILIZE_Z_HI);
    if (stabilizedpitch)
    {
        if (isBetweenExclusive(rotx,STILL_ROT_LO,STILL_ROT_HI) &&
    //        isBetweenExclusive(roty,STILL_ROT_LO,STILL_ROT_HI) &&
    //        isBetweenExclusive(rotz,STILL_ROT_LO,STILL_ROT_HI) &&
    //          (stabilizedpitch) &&
            isBetweenExclusive(right.z,STABILIZE_Z_LO,STABILIZE_Z_HI) &&
            (up.z > 0) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackStabilizeShip
    Description : this ai will stabilize the ship (e.g. stop all pitch, roll,
                  and yaw, although it will not change the velocity of the ship)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackStabilizeGuidance(SpaceObjRotImpTargGuidance *ship)
{
    real32 rotx = ship->rotinfo.rotspeed.x;
    real32 roty = ship->rotinfo.rotspeed.y;
    real32 rotz = ship->rotinfo.rotspeed.z;
    vector heading;
    vector up;
    vector right;
    real32 rotstr;
    real32 oneovertime;
    bool stabilizedpitch;

    StaticInfoHealthGuidance *shipstaticinfo;

    matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(up,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);

    stabilizedpitch = isBetweenExclusive(heading.z,STABILIZE_Z_LO,STABILIZE_Z_HI);
    if (stabilizedpitch)
    {
        roty = ship->rotinfo.rotspeed.y = 0.0f;
    }

    if (isBetweenExclusive(rotx,STILL_ROT_LO,STILL_ROT_HI) &&
//        isBetweenExclusive(roty,STILL_ROT_LO,STILL_ROT_HI) &&
//        isBetweenExclusive(rotz,STILL_ROT_LO,STILL_ROT_HI) &&
          (stabilizedpitch) &&
        isBetweenExclusive(right.z,STABILIZE_Z_LO,STABILIZE_Z_HI) &&
        (up.z > 0) )
    {
        return TRUE;
    }

#ifdef gshaw
//    dbgMessagef("\nZeroing Rot %f %f %f %x",rotx,roty,rotz,(udword)ship);
#endif

    shipstaticinfo = ship->staticinfo;
    oneovertime = 1.0f / universe.phystimeelapsed;

    if (rotx > 0)
    {
        rotstr = rotx * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
    }
    else
    {
        rotstr = -rotx * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
    }

    if (stabilizedpitch)
    {
        if (rotz > 1.0f)
        {
            rotstr = rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        }
        else if (rotz < -1.0f)
        {
            rotstr = -rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        }
        else
        {
            if (right.z > 0)
            {
                physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ROLLRIGHT]*right.z,ROT_ROLLRIGHT);
            }
            else
            {
                physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ROLLLEFT]*(-right.z),ROT_ROLLLEFT);
            }
        }
    }
    else
    {
        if (rotz > 0.0f)
        {
            rotstr = rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        }
        else
        {
            rotstr = -rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        }

        if (roty > 1.0f)
        {
            rotstr = roty * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
        }
        else if (roty < -1.0f)
        {
            rotstr = -roty * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
        }
        else
        {
            if (heading.z < 0)
            {
                physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_PITCHUP]*(-heading.z),ROT_PITCHUP);
            }
            else
            {
                physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_PITCHDOWN]*heading.z,ROT_PITCHDOWN);
            }
        }
    }

#if 0
    desiredrotspeed = -heading.z
    if (ship->rotinfo.rotspeed.z < desiredrotspeed)
    {
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW],ROT_ABOUTYCCW);
    }
    else
    {
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTYCW],ROT_ABOUTYCW);
    }
#endif

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackIsZeroRotation
    Description : returns TRUE if rotation is zero
    Inputs      :
    Outputs     :
    Return      : returns TRUE if rotation is zero
----------------------------------------------------------------------------*/
bool aitrackIsZeroRotation(Ship *ship)
{
    real32 rotx = ship->rotinfo.rotspeed.x;
    real32 roty = ship->rotinfo.rotspeed.y;
    real32 rotz = ship->rotinfo.rotspeed.z;

    if (isBetweenExclusive(rotx,STILL_ROT_LO,STILL_ROT_HI) &&
        isBetweenExclusive(roty,STILL_ROT_LO,STILL_ROT_HI) &&
        isBetweenExclusive(rotz,STILL_ROT_LO,STILL_ROT_HI))
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackZeroRotationAnywhere
    Description : this ai will stabilize the ship (e.g. stop all pitch, roll,
                  and yaw, although it will not change the velocity of the ship)
                  It does not necessarily stop the ship oriented up
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackZeroRotationAnywhere(Ship *ship)
{
    real32 rotx = ship->rotinfo.rotspeed.x;
    real32 roty = ship->rotinfo.rotspeed.y;
    real32 rotz = ship->rotinfo.rotspeed.z;
    vector heading;
    vector up;
    vector right;
    real32 rotstr;
    real32 oneovertime;

    ShipStaticInfo *shipstaticinfo;

    matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(up,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);

    if (isBetweenExclusive(rotx,STILL_ROT_LO,STILL_ROT_HI) &&
        isBetweenExclusive(roty,STILL_ROT_LO,STILL_ROT_HI) &&
        isBetweenExclusive(rotz,STILL_ROT_LO,STILL_ROT_HI))
    {
        return TRUE;
    }

    shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    oneovertime = 1.0f / universe.phystimeelapsed;

    if (rotx > 0)
    {
        rotstr = rotx * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
    }
    else
    {
        rotstr = -rotx * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
    }

    if (roty > 0)
    {
        rotstr = roty * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
    }
    else
    {
        rotstr = -roty * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
    }
    if (rotz > 0)
    {
        rotstr = rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
    }
    else
    {
        rotstr = -rotz * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackHeading
    Description : given a desired heading vector, this ai will orient the
                  ship in the correct heading as well as stopping any roll.
    Inputs      : desiredHeading
    Outputs     :
    Return      : TRUE if heading has been successfully tracked.
----------------------------------------------------------------------------*/
bool aitrackHeadingAndUp(Ship *ship,vector *desiredHeading,vector *desiredUp,real32 accuracy)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    vector curheading;
    vector curup;
    real32 dotProdCheckHeading;
    real32 dotProdCheckUp;

    real32 desiredrotspeed;
    real32 rotspeed;

    vector desiredHeadingInShipCoordSys;

    vector desiredUpInShipCoordSys;

    real32 rotstr;
    real32 oneovertime;

    if(ship->objtype == OBJ_ShipType)
    {
        if(ship->dontrotateever)
        {
            return TRUE;
        }
    }
    matGetVectFromMatrixCol3(curheading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(curup,ship->rotinfo.coordsys);

    dotProdCheckHeading = vecDotProduct(*desiredHeading,curheading);
    dotProdCheckUp = vecDotProduct(*desiredUp,curup);

#if DEBUG_AITRACK
    dbgMessagef("\nHeading accuracy: %f",dotProdCheckHeading);
#endif

    if (dotProdCheckHeading <= -1.0f)   // use <= in case inaccuraces cause number slightly less than 1.0
    {
        // Exactly opposite direction, so arbitrarily chose a direction and rotate that way
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTXCW],ROT_ABOUTXCW);
        return FALSE;
    }

    if (dotProdCheckUp <= -1.0f)   // use <= in case inaccuraces cause number slightly less than 1.0
    {
        // Exactly opposite direction, so arbitrarily chose a direction and rotate that way
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTZCW],ROT_ABOUTZCW);
        return FALSE;
    }

    if ((dotProdCheckHeading >= accuracy) && (dotProdCheckUp >= accuracy))
    {
#if DEBUG_AITRACK
        dbgMessage("\nOn correct heading...");
#endif
        aitrackForceShipZeroRotation(ship);
        return TRUE;     // heading close enough to desired heading, so return
    }

    // Our heading is not close to the desired heading, so we have
    // to do more work...

    // transform desiredHeading into desiredHeading in Ship co-ordinate system

    matMultiplyVecByMat(&desiredHeadingInShipCoordSys,desiredHeading,&ship->rotinfo.coordsys);

    // transform desiredUp into desiredUp in Ship co-ordinate system

    matMultiplyVecByMat(&desiredUpInShipCoordSys,desiredUp,&ship->rotinfo.coordsys);

    oneovertime = 1.0f / universe.phystimeelapsed;

    // Check if we need to rotate about x axis (yaw)
    if (desiredHeadingInShipCoordSys.y != 0)
    {
        if (desiredHeadingInShipCoordSys.z < 0.0f)
        {
            if (desiredHeadingInShipCoordSys.y < 0.0f)
            {
                desiredrotspeed = ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
            else
            {
                desiredrotspeed = -ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
        }
        else
        {
            desiredrotspeed = -desiredHeadingInShipCoordSys.y * ship->nonstatvars.turnspeed[TURN_ABOUTX];
        }

        rotspeed = ship->rotinfo.rotspeed.x;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
        }
    }

    // Check if we need to rotate about y axis (pitch)
    if (desiredHeadingInShipCoordSys.x != 0)
    {
        desiredrotspeed = desiredHeadingInShipCoordSys.x * ship->nonstatvars.turnspeed[TURN_ABOUTY];
        rotspeed = ship->rotinfo.rotspeed.y;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
        }
    }

    // Check if we need to rotate about z axis (roll) to make up-vector of ship
    // align with world z axis.

    if (desiredUpInShipCoordSys.y != 0.0f)
    {
        desiredrotspeed = desiredUpInShipCoordSys.y * ship->nonstatvars.turnspeed[TURN_ABOUTZ];
        rotspeed = ship->rotinfo.rotspeed.z;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackHeading
    Description : given a desired heading vector, this ai will orient the
                  ship in the correct heading as well as stopping any roll.
    Inputs      : desiredHeading
    Outputs     :
    Return      : TRUE if heading has been successfully tracked.
----------------------------------------------------------------------------*/
bool aitrackHeadingFunc(SpaceObjRotImpTargGuidance *ship,vector *desiredHeading,real32 accuracy,real32 upaccuracy,real32 sinbank,udword flags)
{
    StaticInfoHealthGuidance *shipstaticinfo = ship->staticinfo;

    vector curheading;
    vector curup;
    real32 dotProdCheckHeading;
    real32 dotProdCheckUp;

    real32 desiredrotspeed;
    real32 rotspeed;

    vector desiredHeadingInShipCoordSys;

    vector desiredUp = { 0.0f,0.0f,1.0f };
    vector desiredUpInShipCoordSys;

    real32 rotstr;
    real32 oneovertime;

    real32 yawcloseness = 0.0f;

    if(ship->objtype == OBJ_ShipType)
    {
        if(((Ship *)ship)->dontrotateever)    //if this flag is set we don't want to rotate the ship at all
        {
            return TRUE;
        }
    }

    if (ship->nonstatvars.rotstrength[ROT_PITCHUP] == 0)
    {
        desiredHeading->z = 0.0f;
        vecNormalize(desiredHeading);       // renormalize
    }

    matGetVectFromMatrixCol3(curheading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(curup,ship->rotinfo.coordsys);

    aishipStatsDesiredHeading(*desiredHeading);
    aishipStatsActualHeading(curheading);

    dotProdCheckHeading = vecDotProduct(*desiredHeading,curheading);
    dotProdCheckUp = curup.z; //vecDotProduct(desiredUp,curup);

#if DEBUG_AITRACK
    dbgMessagef("\nHeading accuracy: %f",dotProdCheckHeading);
#endif

    if (dotProdCheckHeading <= -1.0f)
    {
        // Exactly opposite direction, so arbitrarily chose a direction and rotate that way
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTXCW],ROT_ABOUTXCW);
        return FALSE;
    }

    if ((dotProdCheckHeading >= accuracy) &&
        ((flags & (AITRACKHEADING_IGNOREUPVEC+AITRACKHEADING_DONTROLL)) || (dotProdCheckUp >= upaccuracy)) )
    {
#if DEBUG_AITRACK
        dbgMessage("\nOn correct heading...");
#endif
		if(!(flags & AITRACK_DONT_ZERO_ME))
		{
			aitrackForceGuidanceZeroRotation(ship);
		}
        return TRUE;     // heading close enough to desired heading, so return
    }

    // Our heading is not close to the desired heading, so we have
    // to do more work...

    // transform desiredHeading into desiredHeading in Ship co-ordinate system

    matMultiplyVecByMat(&desiredHeadingInShipCoordSys,desiredHeading,&ship->rotinfo.coordsys);

    // transform desiredUp into desiredUp in Ship co-ordinate system

    matMultiplyVecByMat(&desiredUpInShipCoordSys,&desiredUp,&ship->rotinfo.coordsys);

    oneovertime = 1.0f / universe.phystimeelapsed;

    // Check if we need to rotate about x axis (yaw)
    if (desiredHeadingInShipCoordSys.y != 0)
    {
        if (desiredHeadingInShipCoordSys.z < 0.0f)
        {
            if (desiredHeadingInShipCoordSys.y < 0.0f)
            {
                desiredrotspeed = ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
            else
            {
                desiredrotspeed = -ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
        }
        else
        {
            desiredrotspeed = -desiredHeadingInShipCoordSys.y * ship->nonstatvars.turnspeed[TURN_ABOUTX];
        }

        if (sinbank != 0.0f)
        {
            if (desiredHeadingInShipCoordSys.z < 0.0f)
            {
                if (desiredHeadingInShipCoordSys.y < 0.0f)
                {
                    yawcloseness = sinbank;
                }
                else
                {
                    yawcloseness = -sinbank;
                }
            }
            else
            {
                yawcloseness = -desiredHeadingInShipCoordSys.y * sinbank;
            }
        }
        rotspeed = ship->rotinfo.rotspeed.x;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
        }
    }

    // Check if we need to rotate about y axis (pitch)
    if (desiredHeadingInShipCoordSys.x != 0)
    {
        desiredrotspeed = desiredHeadingInShipCoordSys.x * ship->nonstatvars.turnspeed[TURN_ABOUTY];
        rotspeed = ship->rotinfo.rotspeed.y;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
        }
    }

    if (flags & AITRACKHEADING_DONTROLL)
    {
        return FALSE;
    }

    // Check if we need to rotate about z axis (roll) to make up-vector of ship
    // align with world z axis.

    if (desiredUpInShipCoordSys.y != yawcloseness)
    {
        desiredrotspeed = (desiredUpInShipCoordSys.y - yawcloseness) * ship->nonstatvars.turnspeed[TURN_ABOUTZ];
        rotspeed = ship->rotinfo.rotspeed.z;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        }
    }

    return FALSE;
}

bool aitrackHeadingWithBankPitchFunc(Ship *ship,vector *desiredHeading,real32 accuracy,real32 sinbank,real32 pitchdescend,real32 pitchturn,udword flags)
{
    vector curheading;
    vector desheading;
    real32 curheadingsqr,curheadingmag;
    real32 desheadingsqr,desheadingmag;

    vector curup;

    real32 dotprodheading;
    real32 dotProdCheckUp;

    real32 actualpitchturn;
    real32 finalpitch;

    real32 temp1,temp2;

    if(ship->objtype == OBJ_ShipType)
    {
        if(ship->dontrotateever)
        {
            //don't rotate this ship
            return TRUE;
        }
    }
    matGetVectFromMatrixCol3(curheading,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(curup,ship->rotinfo.coordsys);

    // I will treat curheading.z as = 0, but will not set it.

    desheading.x = desiredHeading->x;
    desheading.y = desiredHeading->y;
    // I will treat desheading.z as = 0, but will not set it.

    desheadingsqr = desheading.x * desheading.x + desheading.y * desheading.y;
    curheadingsqr = curheading.x * curheading.x + curheading.y * curheading.y;
    if (desheadingsqr == 0.0f)       // extreme case where heading directly up
    {
        desheading.x = 1.0f;
        desheadingsqr = 1.0f;
    }
    if (curheadingsqr == 0.0f)      // extreme case where heading directly up
    {
        curheading.x = 1.0f;
        curheadingsqr = 1.0f;
    }

    curheadingmag = fsqrt(curheadingsqr);
    desheadingmag = fsqrt(desheadingsqr);
    temp1 = 1.0f / curheadingmag;
    temp2 = 1.0f / desheadingmag;

    curheading.x *= temp1;
    curheading.y *= temp1;
    desheading.x *= temp2;
    desheading.y *= temp2;

    dotprodheading = (curheading.x * desheading.x) + (curheading.y * desheading.y); // no need to mulitply z components because it is 0

    dotProdCheckUp = curup.z;   //vecDotProduct({0,0,1},curup);

    if ((dotprodheading >= accuracy) && (pitchdescend == 0.0f) &&
        isBetweenExclusive(curheading.z,STABILIZE_Z_LO,STABILIZE_Z_HI) &&
        ((flags & AITRACKHEADING_IGNOREUPVEC) || (dotProdCheckUp >= accuracy)) )
    {
#if DEBUG_AITRACK
        dbgMessage("\nCapital ship on correct heading...");
#endif
        ship->rotinfo.rotspeed.x = 0.0f;
        ship->rotinfo.rotspeed.y = 0.0f;
        ship->rotinfo.rotspeed.z = 0.0f;
        return TRUE;     // heading close enough to desired heading, so return
    }

    actualpitchturn = (1.0f - ABS(dotprodheading)) * pitchturn;

    if (pitchdescend < 0.0f)
    {
        actualpitchturn = -actualpitchturn;
        if (pitchdescend < actualpitchturn)
        {
            finalpitch = pitchdescend;
        }
        else
        {
            finalpitch = actualpitchturn;
        }
    }
    else
    {
        if (pitchdescend > actualpitchturn)
        {
            finalpitch = pitchdescend;
        }
        else
        {
            finalpitch = actualpitchturn;
        }
    }

    // we now have the final desired pitch, finalpitch.

    desheading.z = (real32)tan(finalpitch);     // z = tan(theta) * 1 (because sqrt(desheading.x^2 + desheading.y^2) == 1)

    temp1 = 1.0f / fsqrt(desheading.z * desheading.z + 1.0f);

    desheading.x *= temp1;       // normalize vector
    desheading.y *= temp1;
    desheading.z *= temp1;

    return aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship,&desheading,accuracy,accuracy, sinbank,flags);
}

/*-----------------------------------------------------------------------------
    Name        : aitrackRightVector
    Description : given a desired right vector, this ai will orient the
                  ship in the correct heading as well as stopping any roll.
    Inputs      : desiredRightVector
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackRightVector(Ship *ship,vector *desiredRightVector,real32 accuracy)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    vector curright;
    vector curup;
    real32 dotProdCheckRight;
    real32 dotProdCheckUp;

    vector desiredRightInShipCoordSys;

    vector desiredUp = { 0.0f,0.0f,1.0f };
    vector desiredUpInShipCoordSys;

    real32 desiredrotspeed;
    real32 rotspeed;

    real32 rotstr;
    real32 oneovertime;

    if(ship->objtype == OBJ_ShipType)
    {
        if(ship->dontrotateever)
        {
            //don't rotate this ship
            return TRUE;
        }
    }
    matGetVectFromMatrixCol2(curright,ship->rotinfo.coordsys);
    matGetVectFromMatrixCol1(curup,ship->rotinfo.coordsys);

    dotProdCheckRight = vecDotProduct(*desiredRightVector,curright);
    dotProdCheckUp = curup.z; //vecDotProduct(desiredUp,curup)

#if DEBUG_AITRACK
    dbgMessagef("\nHeading accuracy: %f",dotProdCheckRight);
#endif

    if (dotProdCheckRight <= -1.0f)
    {
        // Exactly opposite direction, so arbitrarily chose a direction and rotate that way
        physApplyRotToObj((SpaceObjRot *)ship,ship->nonstatvars.rotstrength[ROT_ABOUTXCW],ROT_ABOUTXCW);
        return FALSE;
    }

    if ((dotProdCheckRight >= accuracy) && (dotProdCheckUp >= AITRACKRIGHT_UPACCURACY))
    {
#if DEBUG_AITRACK
        dbgMessage("\nOn correct heading...");
#endif
        aitrackForceShipZeroRotation(ship);
        return TRUE;     // heading close enough to desired heading, so return
    }

    // Our heading is not close to the desired heading, so we have
    // to do more work...

    // transform desiredRightVector into desiredRightVector in Ship co-ordinate system

    matMultiplyVecByMat(&desiredRightInShipCoordSys,desiredRightVector,&ship->rotinfo.coordsys);

    // transform desiredUp into desiredUp in Ship co-ordinate system

    matMultiplyVecByMat(&desiredUpInShipCoordSys,&desiredUp,&ship->rotinfo.coordsys);

    oneovertime = 1.0f / universe.phystimeelapsed;

    // Check if we need to rotate about x axis (yaw)
    if (desiredRightInShipCoordSys.z != 0)
    {
        if (desiredRightInShipCoordSys.y < 0.0f)
        {
            if (desiredRightInShipCoordSys.z > 0.0f)
            {
                desiredrotspeed = ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
            else
            {
                desiredrotspeed = -ship->nonstatvars.turnspeed[TURN_ABOUTX];
            }
        }
        else
        {
            desiredrotspeed = desiredRightInShipCoordSys.z * ship->nonstatvars.turnspeed[TURN_ABOUTX];
        }

        rotspeed = ship->rotinfo.rotspeed.x;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCCW]),ROT_ABOUTXCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaX * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTXCW]),ROT_ABOUTXCW);
        }
    }

    // Check if we need to rotate about y axis (pitch) to make up-vector of ship
    // align with world z axis.

    if (desiredUpInShipCoordSys.z != 0)
    {
        if (desiredUpInShipCoordSys.x < 0.0f)
        {
            if (desiredUpInShipCoordSys.z < 0.0f)
            {
                desiredrotspeed = ship->nonstatvars.turnspeed[TURN_ABOUTY];
            }
            else
            {
                desiredrotspeed = -ship->nonstatvars.turnspeed[TURN_ABOUTY];
            }
        }
        else
        {
            desiredrotspeed = -desiredUpInShipCoordSys.z * ship->nonstatvars.turnspeed[TURN_ABOUTY];
        }

        rotspeed = ship->rotinfo.rotspeed.y;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCCW]),ROT_ABOUTYCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaY * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTYCW]),ROT_ABOUTYCW);
        }
    }

    // Check if we need to rotate about z axis (roll)
    if (desiredRightInShipCoordSys.x != 0)
    {
        if (desiredRightInShipCoordSys.y < 0.0f)
        {
            if (desiredRightInShipCoordSys.x < 0.0f)
            {
                desiredrotspeed = ship->nonstatvars.turnspeed[TURN_ABOUTZ];
            }
            else
            {
                desiredrotspeed = -ship->nonstatvars.turnspeed[TURN_ABOUTZ];
            }
        }
        else
        {
            desiredrotspeed = -desiredRightInShipCoordSys.x * ship->nonstatvars.turnspeed[TURN_ABOUTZ];
        }

        rotspeed = ship->rotinfo.rotspeed.z;

        if (rotspeed < desiredrotspeed)
        {
            rotstr = (desiredrotspeed - rotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCCW]),ROT_ABOUTZCCW);
        }
        else
        {
            rotstr = (rotspeed - desiredrotspeed) * shipstaticinfo->staticheader.momentOfInertiaZ * oneovertime;
            physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]),ROT_ABOUTZCW);
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackVelocityVector
    Description : given a velocity vector, makes the ship track it.  Note that
                  it does not change the heading, only the velocity of the ship.
    Inputs      : desiredVelocity
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aitrackVelocityVectorGuidance(SpaceObjRotImpTargGuidance *ship,vector *desiredVelocity)
{
    StaticInfoHealthGuidance *shipstaticinfo = ship->staticinfo;

    vector errorDifference;
    real32 errorMagSquared;
    real32 thruststr;
    real32 massOverTime;

    vector desiredVelocityInShipCoordSys;
    vector shipVelocityInShipCoordSys;

    // check if desiredVelocity is approximately the same as current velocity

    vecSub(errorDifference,*desiredVelocity,ship->posinfo.velocity); // later is there a faster way to do this?
    errorMagSquared = vecMagnitudeSquared(errorDifference);

#if DEBUG_AITRACK
    dbgMessagef("\nVelocity Error = %f",errorMagSquared);
#endif

    if (errorMagSquared < ERROR_VELOCITY_SQUARED)
    {
#if DEBUG_AITRACK
        dbgMessage("\nVelocity tracking locked...");
#endif
        return;     // ship velocity close enought to desired velocity, so return
    }

    // current velocity is not close enought to desiredVelocity, so
    // we have to do more work.

    // transfrom desiredVelocity into desiredVelocity in ship co-ordinate system
    // transfrom shipVelocity into shipVelocity in ship co-ordinate system

    matMultiplyVecByMat(&desiredVelocityInShipCoordSys,desiredVelocity,&ship->rotinfo.coordsys);
    matMultiplyVecByMat(&shipVelocityInShipCoordSys,&ship->posinfo.velocity,&ship->rotinfo.coordsys);

    massOverTime = shipstaticinfo->staticheader.mass / universe.phystimeelapsed;

    if (shipVelocityInShipCoordSys.x < desiredVelocityInShipCoordSys.x)
    {
        thruststr = (desiredVelocityInShipCoordSys.x - shipVelocityInShipCoordSys.x) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_UP]),TRANS_UP);
    }
    else if (shipVelocityInShipCoordSys.x > desiredVelocityInShipCoordSys.x)
    {
        thruststr = (shipVelocityInShipCoordSys.x - desiredVelocityInShipCoordSys.x) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_DOWN]),TRANS_DOWN);
    }

    if (shipVelocityInShipCoordSys.y < desiredVelocityInShipCoordSys.y)
    {
        thruststr = (desiredVelocityInShipCoordSys.y - shipVelocityInShipCoordSys.y) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_RIGHT]),TRANS_RIGHT);
    }
    else if (shipVelocityInShipCoordSys.y > desiredVelocityInShipCoordSys.y)
    {
        thruststr = (shipVelocityInShipCoordSys.y - desiredVelocityInShipCoordSys.y) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_LEFT]),TRANS_LEFT);
    }

    if (shipVelocityInShipCoordSys.z < desiredVelocityInShipCoordSys.z)
    {
        thruststr = (desiredVelocityInShipCoordSys.z - shipVelocityInShipCoordSys.z) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_FORWARD]),TRANS_FORWARD);
    }
    else if (shipVelocityInShipCoordSys.z > desiredVelocityInShipCoordSys.z)
    {
        thruststr = (shipVelocityInShipCoordSys.z - desiredVelocityInShipCoordSys.z) * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_BACKWARD]),TRANS_BACKWARD);
    }

}

/*-----------------------------------------------------------------------------
    Name        : aitrackIsZeroVelocity
    Description : returns TRUE if ship has stopped drifting
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship has stopped drifting
----------------------------------------------------------------------------*/
bool aitrackIsZeroVelocity(Ship *ship)
{
    vector shipVelocity;

    shipVelocity = ship->posinfo.velocity;

    if (isBetweenExclusive(shipVelocity.x,STILL_VELOCITY_LO,STILL_VELOCITY_HI) &&
        isBetweenExclusive(shipVelocity.y,STILL_VELOCITY_LO,STILL_VELOCITY_HI) &&
        isBetweenExclusive(shipVelocity.z,STILL_VELOCITY_LO,STILL_VELOCITY_HI) )
    {
        return TRUE;     // ship velocity close enought to desired velocity, so return
    }
    else
    {
        return FALSE;
    }
}

bool aitrackZeroForwardVelocity(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;
    real32 thruststr;
    real32 massOverTime;
    real32 forwardvelocity;
    vector heading;

    matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
    forwardvelocity = vecDotProduct(heading,ship->posinfo.velocity);

    if (isBetweenExclusive(forwardvelocity,STILL_VELOCITY_LO,STILL_VELOCITY_HI))
    {
        return TRUE;    // ship velocity close enought to desired velocity, so return
    }

    massOverTime = shipstaticinfo->staticheader.mass / universe.phystimeelapsed;

    if (forwardvelocity < 0)
    {
        thruststr = -forwardvelocity * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_FORWARD]),TRANS_FORWARD);
    }
    else
    {
        thruststr = forwardvelocity * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_BACKWARD]),TRANS_BACKWARD);
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackZeroVelocity
    Description : stops ship from drifting
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackZeroVelocity(Ship *ship)
{
    vector shipVelocity;
    vector shipVelocityInShipCoordSys;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    real32 thruststr;
    real32 massOverTime;

    shipVelocity = ship->posinfo.velocity;

    if (isBetweenExclusive(shipVelocity.x,STILL_VELOCITY_LO,STILL_VELOCITY_HI) &&
        isBetweenExclusive(shipVelocity.y,STILL_VELOCITY_LO,STILL_VELOCITY_HI) &&
        isBetweenExclusive(shipVelocity.z,STILL_VELOCITY_LO,STILL_VELOCITY_HI) )
    {
        return TRUE;     // ship velocity close enought to desired velocity, so return
    }

#ifdef gshaw
//    dbgMessagef("\nZeroing velocity %f %f %f %x",hshipVelocity.x,hshipVelocity.y,hshipVelocity.z,(udword)ship);
#endif

    // transfrom shipVelocity into shipVelocity in ship co-ordinate system
    matMultiplyVecByMat(&shipVelocityInShipCoordSys,&shipVelocity,&ship->rotinfo.coordsys);

    massOverTime = shipstaticinfo->staticheader.mass / universe.phystimeelapsed;
    if (shipVelocityInShipCoordSys.x < 0)
    {
        thruststr = -shipVelocityInShipCoordSys.x * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_UP]),TRANS_UP);
    }
    else
    {
        thruststr = shipVelocityInShipCoordSys.x * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_DOWN]),TRANS_DOWN);
    }

    if (shipVelocityInShipCoordSys.y < 0)
    {
        thruststr = -shipVelocityInShipCoordSys.y * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_RIGHT]),TRANS_RIGHT);
    }
    else
    {
        thruststr = shipVelocityInShipCoordSys.y * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_LEFT]),TRANS_LEFT);
    }

    if (shipVelocityInShipCoordSys.z < 0)
    {
        thruststr = -shipVelocityInShipCoordSys.z * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_FORWARD]),TRANS_FORWARD);
    }
    else
    {
        thruststr = shipVelocityInShipCoordSys.z * massOverTime;
        physApplyForceToObj((SpaceObj *)ship,capNumber(thruststr,ship->nonstatvars.thruststrength[TRANS_BACKWARD]),TRANS_BACKWARD);
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackSteadyShipDriftOnly
    Description : stops ship from drifting only
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackSteadyShipDriftOnly(Ship *ship)
{
    if (aitrackZeroVelocity(ship))
    {
        aitrackForceZeroVelocity(ship);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aitrackIsShipSteadyAnywhere
    Description : returns TRUE if ship is steady anywhere (not necessarily oriented up)
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is steady anywhere (not necessarily oriented up)
----------------------------------------------------------------------------*/
bool aitrackIsShipSteadyAnywhere(Ship *ship)
{
    return (aitrackIsZeroRotation(ship) & aitrackIsZeroVelocity(ship));
}

/*-----------------------------------------------------------------------------
    Name        : aitrackSteadyShipAnywhere
    Description : stops ship from drifting, and stops ship from rotating (not necessarily oriented up)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackSteadyShipAnywhere(Ship *ship)
{
    bool stabilized = aitrackZeroRotationAnywhere(ship);
    bool zerovel = aitrackZeroVelocity(ship);

    if (stabilized && zerovel)
    {
        aitrackForceSteadyShip(ship);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aitrackIsShipSteady
    Description : returns TRUE if ship is steady
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is steady
----------------------------------------------------------------------------*/
bool aitrackIsShipSteady(Ship *ship)
{
    return (aitrackIsStabilizeShip(ship) & aitrackIsZeroVelocity(ship));
}

/*-----------------------------------------------------------------------------
    Name        : aitrackSteadyShip
    Description : stops ship from drifting, and stops ship from rotating
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool aitrackSteadyShip(Ship *ship)
{
    bool stabilized = aitrackStabilizeShip(ship);
    bool zerovel = aitrackZeroVelocity(ship);

    if (stabilized && zerovel)
    {
        aitrackForceSteadyShip(ship);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aitrackForceSteadyShip
    Description : stops ship from drifting/rotating, perfectly.
                  Use as an approximation to aitrackSteadyShip, if ship is
                  moving very slow.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aitrackForceSteadyShip(Ship *ship)
{
    if (ship->flags & SOF_Crazy)
    {
        return;     // ship is crazy, so return
    }

    ship->posinfo.velocity.x = 0.0f;
    ship->posinfo.velocity.y = 0.0f;
    ship->posinfo.velocity.z = 0.0f;

    ship->rotinfo.rotspeed.x = 0.0f;
    ship->rotinfo.rotspeed.y = 0.0f;
    ship->rotinfo.rotspeed.z = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackForceShipZeroRotation
    Description : stops ships from rotating, perfectly.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aitrackForceGuidanceZeroRotation(SpaceObjRotImpTargGuidance *ship)
{
    if (ship->flags & SOF_Crazy)
    {
        return;     // ship is crazy, so return
    }

    ship->rotinfo.rotspeed.x = 0.0f;
    ship->rotinfo.rotspeed.y = 0.0f;
    ship->rotinfo.rotspeed.z = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackForceZeroVelocity
    Description : stops ship from moving, perfectly
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aitrackForceZeroVelocity(Ship *ship)
{
    if (ship->flags & SOF_Crazy)
    {
        return;     // ship is crazy, so return
    }

    ship->posinfo.velocity.x = 0.0f;
    ship->posinfo.velocity.y = 0.0f;
    ship->posinfo.velocity.z = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : aitrackForceHeading
    Description : points ship in a particular direction
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aitrackForceHeading(Ship *ship, vector *heading, vector *upvector)
{
    vector rightvec;
    if (ship->flags & SOF_Crazy)
    {
        return;     // ship is crazy, so return
    }

    vecCrossProduct(rightvec,*heading,*upvector);
    // set up vector
    ship->rotinfo.coordsys.m11 = upvector->x;
    ship->rotinfo.coordsys.m21 = upvector->y;
    ship->rotinfo.coordsys.m31 = upvector->z;
    // set right vector
    ship->rotinfo.coordsys.m12 = rightvec.x;
    ship->rotinfo.coordsys.m22 = rightvec.y;
    ship->rotinfo.coordsys.m32 = rightvec.z;
    // set heading vector
    ship->rotinfo.coordsys.m13 = heading->x;
    ship->rotinfo.coordsys.m23 = heading->y;
    ship->rotinfo.coordsys.m33 = heading->z;
}


/*-----------------------------------------------------------------------------
    Name        : MoveReachedDestinationVariable
    Description : returns TRUE if ship has reached destination (or is close enough)
    Inputs      : ship,destination,bounds
    Outputs     :
    Return      : TRUE if ship has reached destination (or is close enough)
----------------------------------------------------------------------------*/
bool MoveReachedDestinationVariable(Ship *ship,vector *destination,real32 bounds)
{
    vector lefttogo;
    real32 negbounds = -bounds;

    vecSub(lefttogo,*destination,ship->posinfo.position);

    if ((isBetweenExclusive(lefttogo.x,negbounds,bounds)) &&
        (isBetweenExclusive(lefttogo.y,negbounds,bounds)) &&
        (isBetweenExclusive(lefttogo.z,negbounds,bounds)))
    {
#ifdef gshaw
//        dbgMessagef("\nShip %x reached destination!",(udword)ship);
#endif
        return TRUE;
    }
    else
    {
#ifdef gshaw
//        dbgMessagef("\nLeft to go: %f %f %f",lefttogo.x,lefttogo.y,lefttogo.z);
#endif
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MoveLeftToGo
    Description : returns the maximum distance along a axis which the ship has left to go
    Inputs      : ship, destination
    Outputs     :
    Return      : returns the maximum distance along a axis which the ship has left to go
----------------------------------------------------------------------------*/
real32 MoveLeftToGo(Ship *ship,vector *destination)
{
    real32 absx = ABS(destination->x - ship->posinfo.position.x);
    real32 absy = ABS(destination->y - ship->posinfo.position.y);
    real32 absz = ABS(destination->z - ship->posinfo.position.z);

    return max(max(absx,absy),absz);
}


/*-----------------------------------------------------------------------------
    Name        : aitrackRotationSpeed
    Description : make's a ship track a desired speed
    Inputs      : ship, rotation speed, track
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
bool aitrackRotationSpeed(Ship *ship, real32 desiredrotspeed, uword track)
{
    real32 rotstr;
    real32 oneovertime;
    real32 rotspeed;

    oneovertime = 1.0f / universe.phystimeelapsed;

    switch (track)
    {
        case ROT_ABOUTXCCW :
        case ROT_ABOUTXCW  :
            if (isBetweenExclusive(ship->rotinfo.rotspeed.x,
                desiredrotspeed+STILL_ROT_LO,
                desiredrotspeed+STILL_ROT_HI))
                return(TRUE);
            rotspeed = ship->rotinfo.rotspeed.x;
            rotstr = (desiredrotspeed - rotspeed) * ship->staticinfo->staticheader.momentOfInertiaX * oneovertime;
        break;
        case ROT_ABOUTYCCW :
        case ROT_ABOUTYCW  :
            if (isBetweenExclusive(ship->rotinfo.rotspeed.y,
                desiredrotspeed+STILL_ROT_LO,
                desiredrotspeed+STILL_ROT_HI))
                return(TRUE);
            rotspeed = ship->rotinfo.rotspeed.y;
            rotstr = (desiredrotspeed - rotspeed) * ship->staticinfo->staticheader.momentOfInertiaY * oneovertime;
        break;
        case ROT_ABOUTZCCW :
        case ROT_ABOUTZCW  :
            if (isBetweenExclusive(ship->rotinfo.rotspeed.z,
                desiredrotspeed+STILL_ROT_LO,
                desiredrotspeed+STILL_ROT_HI))
                return(TRUE);
            rotspeed = ship->rotinfo.rotspeed.z;
            rotstr = (desiredrotspeed - rotspeed) * ship->staticinfo->staticheader.momentOfInertiaZ * oneovertime;
        break;
    }

    if (rotspeed < desiredrotspeed)
    {
        physApplyRotToObj((SpaceObjRot *)ship,capNumber(rotstr,ship->nonstatvars.rotstrength[track]),track);
        return(FALSE);
    }
    else
    {
        rotstr = -capNumber(-rotstr,ship->nonstatvars.rotstrength[ROT_ABOUTZCW]);
        physApplyRotToObj((SpaceObjRot *)ship,rotstr,track);
        return(FALSE);
    }
}

