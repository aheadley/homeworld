/*=============================================================================
    Name    : CommandLayer.c
    Purpose : This is the ship command layer of homeworld.  All commands to
              ship go through this layer, whether the commands originate from
              across a network, a computer player, a human player, etc.

    Created 7/5/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "Types.h"
#include "FastMath.h"
#include "Debug.h"
#include "Memory.h"
#include "Formation.h"
#include "CommandLayer.h"
#include "AIShip.h"
#include "AITrack.h"
#include "StatScript.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "SoundEvent.h"
#include "Select.h"
#include "Collision.h"
#include "FlightMan.h"
#include "Attack.h"
#include "AIPlayer.h"
#include "GenericInterceptor.h"
#include "DDDFrigate.h"
#include "SalCapCorvette.h"
#include "MinelayerCorvette.h"
#include "GravWellGenerator.h"
#include "CloakGenerator.h"
#include "ResearchShip.h"
#include "ResearchAPI.h"
#include "Tactics.h"
#include "SinglePlayer.h"
#include "RepairCorvette.h"
#include "GenericDefender.h"
#include "Alliance.h"
#include "Probe.h"
#include "HeavyCorvette.h"
#include "MadLinkIn.h"
#include "ProximitySensor.h"
#include "ConsMgr.h"
#include "ResCollect.h"
#include "Tutor.h"
#include "Randy.h"
#include "Battle.h"
#include "Ping.h"
#include "NetCheck.h"
#include "CommandNetwork.h"

#ifdef gshaw
#ifndef HW_Release
//#define DEBUG_FORMATIONS

//#define DEBUG_LAUNCHSHIP

//#define DEBUG_COLLECTRESOURCES

//#define DEBUG_COMMANDLAYER

#endif
#endif

/*=============================================================================
    Private Functions:
=============================================================================*/

static void FormationCleanup(struct CommandToDo *formationtodo);
static void MoveCleanup(struct CommandToDo *movetodo);
static void MpHyperspaceCleanup(struct CommandToDo *movetodo);
static void RemoveShipFromMoving(Ship *ship);
//static void RemoveShipFromAttacking(Ship *ship);
//static void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo);
//static void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo);
static SpaceObjRotImpTarg *AttackClosestOpponent(Ship *ship,AttackCommand *attack);
static void AttackCleanup(struct CommandToDo *attacktodo);
static AttackTargets *allocateMultipleAttackTargets(Ship *ship);
static void pickMultipleAttackTargets(Ship *ship,AttackCommand *attack);
void ChangeOrderToHalt(CommandToDo *alreadycommand);
bool processMoveToDo(CommandToDo *movetodo,bool passiveAttacked);
bool processMpHyperspaceingToDo(CommandToDo *movetodo);
bool processMoveLeaderToDo(CommandToDo *movetodo,bool passiveAttacked);
void clMoveThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clMpHyperspaceThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
bool isSelectionExclusivlyStrikeCraft(SelectCommand *selection);
bool isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(SelectCommand *selection);
bool selectionHasSwarmers(SelectCommand *selection);
void removeShipFromMpHyperspaceing(Ship *ship);

/*=============================================================================
    Tweakables:
=============================================================================*/

#define DISTMINRATIO        1.0f
real32 DISTMAXRATIO = 5.0f;
real32 SHRINKMIN = 0.1f;
real32 SHRINKMAX = 1.0f;

real32 MAX_DIST_JOIN_GROUP_SQR = (2000.0f*2000.0f);
real32 PROTECT_GET_THIS_CLOSE_CAP = 2000.0f;
real32 PROTECT_GET_THIS_CLOSE_STRIKE = 2000.0f;
real32 PROTECT_STILL_VELOCITY = 20.0f;

real32 WINGMAN_JOIN_MAX_DIST     = (200.0f*200.0f);

real32 FIGHTER_MAX_THREAT_DIST[NUM_TACTICS_TYPES] = { (2000.0f*2000.0f),(2000.0f*2000.0f),(2000.0f*2000.0f) };

udword FIGHTER_PAIRUP_PROB       = 255;

udword FIGHTER_PAIR_CHECK_RATE[NUM_TACTICS_TYPES] = { 7,7,7 };
udword FIGHTER_SINGLE_CHECK_RATE[NUM_TACTICS_TYPES] = { 31,31,31 };

udword MULTIPLETARGETS_RETARGET_RATE = 31;

bool8 HYPERSPACE = FALSE;

real32 GLOBAL_SHIP_HEALTH_MODIFIER = 1.0f;

real32 RESCONTROLLER_FOLLOW_BEHIND = 1.5f;

extern sdword FIGHTER_BREAK_ANGLE_MIN;
extern sdword FIGHTER_BREAK_ANGLE_MAX;

extern sdword FIGHTER_BREAK_VERTICAL_ANGLE_MIN;
extern sdword FIGHTER_BREAK_VERTICAL_ANGLE_MAX;

extern bool BombersUseBombingRun;

extern real32 FLY_INTO_WORLD_PERCENT_DIST;

scriptEntry CommandLayerTweaks[] =
{
    makeEntry(MAX_DIST_JOIN_GROUP_SQR,scriptSetReal32SqrCB),
    makeEntry(PROTECT_GET_THIS_CLOSE_CAP,scriptSetReal32CB),
    makeEntry(PROTECT_GET_THIS_CLOSE_STRIKE,scriptSetReal32CB),
    makeEntry(PROTECT_STILL_VELOCITY,scriptSetReal32CB),
    makeEntry(HYPERSPACE,scriptSetBool8),
    makeEntry(WINGMAN_JOIN_MAX_DIST,scriptSetReal32SqrCB),
    makeEntry(FIGHTER_PAIRUP_PROB,scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Evasive],scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Neutral],scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Aggressive],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Evasive],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Neutral],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Aggressive],scriptSetUdwordCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Evasive],scriptSetReal32SqrCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Neutral],scriptSetReal32SqrCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Aggressive],scriptSetReal32SqrCB),
    makeEntry(MULTIPLETARGETS_RETARGET_RATE,scriptSetUdwordCB),
    makeEntry(GLOBAL_SHIP_HEALTH_MODIFIER,scriptSetReal32CB),
    makeEntry(FIGHTER_BREAK_ANGLE_MIN,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_ANGLE_MAX,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_VERTICAL_ANGLE_MIN,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_VERTICAL_ANGLE_MAX,scriptSetSdwordCB),
    makeEntry(BombersUseBombingRun,scriptSetBool),
    makeEntry(DISTMAXRATIO,scriptSetReal32CB),
    makeEntry(SHRINKMIN,scriptSetReal32CB),
    makeEntry(SHRINKMAX,scriptSetReal32CB),
    makeEntry(FLY_INTO_WORLD_PERCENT_DIST,scriptSetReal32CB),
    makeEntry(RESCONTROLLER_FOLLOW_BEHIND,scriptSetReal32CB),
    endEntry
};

/*-----------------------------------------------------------------------------
    Name        : RemoveShipsFromCommand
    Description : removes ships from command
    Inputs      : command
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipsFromCommand(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;

    for (i=0;i<selection->numShips;i++)
    {
        selection->ShipPtr[i]->command = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromCommand
    Description : removes a ship from command
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromCommand(Ship *ship)
{
    ship->command = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : PrepareShipsForCommand
    Description : prepares ships for executing a new command
    Inputs      : command, rowClear - flag indicating if rightOfWay should be removed
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PrepareShipsForCommand(CommandToDo *command,bool rowClear)
{
    sdword i;
    SelectCommand *selection = command->selection;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->command = command;
        if ((rowClear) && (ship->specialFlags & SPECIAL_rowGettingOutOfWay))
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
    }
    tacticsMakeShipsNotLookForOtherShips(command->selection);
}

/*-----------------------------------------------------------------------------
    Name        : PrepareOneShipForCommand
    Description : prepares ship for executing a new command
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PrepareOneShipForCommand(Ship *ship,CommandToDo *command,bool rowClear)
{
    ship->command = command;
    if ((rowClear) && (ship->specialFlags & SPECIAL_rowGettingOutOfWay))
    {
        rowRemoveShipFromGettingOutOfWay(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitShipsForAI
    Description : Initializes selection ships AI state variables
    Inputs      : fresh (if we're starting a new command, this should be TRUE
                  otherwise, it should be false
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitShipsForAI(SelectCommand *selection,bool fresh)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->aistate = 0;
        ship->aistateattack = 0;
        if(fresh)
            ship->aistatecommand = 0;
    }
}

void InitShipsForAIDontResetAttack(SelectCommand *selection,bool fresh)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->aistate = 0;
        if(fresh)
            ship->aistatecommand = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getRangeToClosestTarget
    Description : REturns the square of the range from command selection to its closest target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 getRangeToClosestTarget(CommandToDo *command)
{
    vector commandPosition;
    real32 tempSqr,minDistanceSqr = REALlyBig;
    vector distvec;
    sdword i;

    dbgAssert(command->selection->numShips > 0);
    dbgAssert(command->ordertype.order == COMMAND_ATTACK);

    commandPosition = command->selection->ShipPtr[0]->posinfo.position;

    for(i=0;i<command->attack->numTargets;i++)
    {
        vecSub(distvec,commandPosition,command->attack->TargetPtr[0]->posinfo.position);
        tempSqr = vecMagnitudeSquared(distvec);
        if(tempSqr < minDistanceSqr)
        {
            minDistanceSqr = tempSqr;
        }
    }
    return minDistanceSqr;
}

/*-----------------------------------------------------------------------------
    Name        : InitShipAI
    Description : Initializes ship's AI state variables
    Inputs      : if fresh is TRUE, aicommand will be set (should only be true if
                  this is called from a new command generation function
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitShipAI(Ship *ship,bool fresh)
{
    ship->aistate = 0;
    ship->aistateattack = 0;
    if(fresh)
        ship->aistatecommand = 0;
}

/*-----------------------------------------------------------------------------
    Name        : protectShip
    Description : ship "protects" or flys nearby protectThisShip
    Inputs      : ship, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define COLLECTOR_STEADY_DIST   3000.0f
void protectShip(Ship *ship,Ship *protectThisShip,bool passiveAttacked)
{
    vector diff;
    real32 dist;
    real32 desiredrange;
    real32 shipSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
    real32 protectThisShipSize = ((ShipStaticInfo *)(protectThisShip->staticinfo))->staticheader.staticCollInfo.collspheresize;
    bool dontrotate;
    vector protectPoint = protectThisShip->posinfo.position;
    vector heading;

    if (protectThisShipSize < shipSize)
    {
        protectThisShipSize = shipSize;     // if guarding a smaller ship, give more room
    }

    if(isCapitalShip(ship) || ship->shiptype == GravWellGenerator || ship->shiptype == CloakGenerator)
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_CAP + shipSize + protectThisShipSize;
    }
    else
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_STRIKE + shipSize + protectThisShipSize;
    }

    // Let's have some ships like the ResourceController follow behind the ship so we don't accidentally block them.
    if (ship->shiptype == ResourceController)
    {
        real32 followbehind = desiredrange * RESCONTROLLER_FOLLOW_BEHIND;      // follow behind 1.5 radii
        matGetVectFromMatrixCol3(heading,protectThisShip->rotinfo.coordsys);
        vecSubFromScalarMultiply(protectPoint,heading,followbehind);
    }

    vecSub(diff,ship->posinfo.position,protectPoint);
    dist = getVectDistSloppy(diff);

//    ship->shipidle = FALSE;

    dontrotate = ((passiveAttacked) & (bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate);

    if (ship->shiptype == DDDFrigate)
    {
        DDDFrigateMakeSureItCanGuard(ship);
    }

    if (protectThisShip->dockvars.dockship == ship)     // if docking with me
    {
        if(MoveReachedDestinationVariable(ship,&protectThisShip->posinfo.position,COLLECTOR_STEADY_DIST))
        {
            //within range of steadyness
            goto juststeady;
        }
    }

    if (dist < desiredrange)
    {
        if ((ship->specialFlags2 & SPECIAL_2_CircularGuard) ||
            (getVectDistSloppy(protectThisShip->posinfo.velocity) < PROTECT_STILL_VELOCITY) )
        {
juststeady:
            if (dontrotate)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
            else
            {
                aitrackSteadyShip(ship);
            }
        }
        else
        {
            // just maintain velocity
            if (dontrotate)
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShipDriftOnly(ship);
                }
            }
            else
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShip(ship);
                }
            }
        }
    }
    else
    {
        // fly to protectThisShip
        if (dontrotate)
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&protectPoint,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                   0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
        }
        else
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&protectPoint,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShip(ship);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : protectShipsAvg
    Description : ship "protects" or flys nearby median position of protectships
    Inputs      : ship, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void protectShipsAvg(Ship *ship,ProtectCommand *protectShips,bool passiveAttacked)
{
    vector diff;
    real32 dist;
    real32 desiredrange;
    real32 shipSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
    bool dontrotate;
    sdword numShips = protectShips->numShips;
    vector temp,guardposition;
    sdword i;
    vector protectShipsAvgVel;
    real32 realtemp;
    real32 minx,maxx,miny,maxy,minz,maxz;
    bool anyshipdockingwithme;

    dbgAssert(numShips > 1);

    temp = protectShips->ShipPtr[0]->posinfo.position;
    minx = maxx = temp.x;
    miny = maxy = temp.y;
    minz = maxz = temp.z;

    protectShipsAvgVel = protectShips->ShipPtr[0]->posinfo.velocity;

    for (i=1;i<numShips;i++)
    {
        temp = protectShips->ShipPtr[i]->posinfo.position;
        vecAddTo(protectShipsAvgVel,protectShips->ShipPtr[i]->posinfo.velocity);

        if (temp.x < minx) minx = temp.x;
        else if (temp.x > maxx) maxx = temp.x;

        if (temp.y < miny) miny = temp.y;
        else if (temp.y > maxy) maxy = temp.y;

        if (temp.z < minz) minz = temp.z;
        else if (temp.z > maxz) maxz = temp.z;
    }

    guardposition.x = (minx + maxx) * 0.5f;
    guardposition.y = (miny + maxy) * 0.5f;
    guardposition.z = (minz + maxz) * 0.5f;

    vecDivideByScalar(protectShipsAvgVel,(real32)numShips,realtemp);

    // we now have the optimum guardposition and protectShipsAvgVel

    vecSub(diff,ship->posinfo.position,guardposition);
    dist = getVectDistSloppy(diff);

    if(isCapitalShip(ship) || ship->shiptype == GravWellGenerator || ship->shiptype == CloakGenerator)
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_CAP + shipSize;
    }
    else
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_STRIKE + shipSize;
    }

//    ship->shipidle = FALSE;

    dontrotate = ((passiveAttacked) & (bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate);

    if (ship->shiptype == DDDFrigate)
    {
        DDDFrigateMakeSureItCanGuard(ship);
    }

    anyshipdockingwithme = FALSE;

    if (ship->staticinfo->canReceiveSomething)
    {
        for (i=0;i<numShips;i++)
        {
            if (protectShips->ShipPtr[i]->dockvars.dockship == ship)
            {
                anyshipdockingwithme = TRUE;
                break;
            }
        }
    }

    if (anyshipdockingwithme)     // if docking with me
    {
        goto juststeady;
    }

    if (dist < desiredrange)
    {
        if ((ship->specialFlags2 & SPECIAL_2_CircularGuard) ||
            (getVectDistSloppy(protectShipsAvgVel) < PROTECT_STILL_VELOCITY))
        {
juststeady:
            if (dontrotate)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
            else
            {
                aitrackSteadyShip(ship);
            }
        }
        else
        {
            // just maintain velocity
            if (dontrotate)
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShipDriftOnly(ship);
                }
            }
            else
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShip(ship);
                }
            }
        }
    }
    else
    {
        // fly to protectThisShip
        if (dontrotate)
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&guardposition,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                   0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
        }
        else
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&guardposition,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShip(ship);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : processLaunchShipToDo
    Description : processes a launch ship command
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship has finished launching
----------------------------------------------------------------------------*/
bool processLaunchShipToDo(CommandToDo *launchtodo)
{
    Ship *ship = launchtodo->selection->ShipPtr[0];
    Ship *receiverShip = launchtodo->launchship.receiverShip;
    ShipStaticInfo *receiverShipStatic = (ShipStaticInfo *)receiverShip->staticinfo;

    dbgAssert(launchtodo->selection->numShips == 1);
    dbgAssert(receiverShip);

    /*
    if(launchtodo->launchship.receiverShip->shipSinglePlayerGameInfo->shipHyperspaceState != SHIPHYPERSPACE_NONE)
    {
        if(ship->collMyBlob == NULL)
        {
            //carrier/mothership is going into hyperspace...
            //so halt launching ships!
            //unless its already outside..then finish it up but quick!
            return FALSE;
        }
    }*/
    if(!singlePlayerGame)
    {
        CommandToDo *shipcommand=getShipAndItsCommand(&universe.mainCommandLayer,launchtodo->launchship.receiverShip);
        if(shipcommand != NULL)
        {
            if(shipcommand->ordertype.order == COMMAND_MP_HYPERSPACEING)
            {
                //launching ship is hyperspaceing...
                if(ship->collMyBlob == NULL)
                {
                    //wait
                    return FALSE;
                }
            }
        }

    }
    else if(singlePlayerGame && singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
    {
        if(ship->collMyBlob == NULL)
        {
            //carrier/mothership is going into hyperspace...
            //so halt launching ships!
            //unless its already outside..then finish it up but quick!
            return FALSE;
        }
    }
    if (receiverShipStatic->LaunchShipXFromShipY(ship,receiverShip))
    {
        RemoveShipFromLaunching(ship);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool isThisShipThreateningMe(SpaceObjRotImpTarg *target,Ship *me)
{
    SpecificAttackVars *targetattackvars;
    Ship *targetsLeader;
    Ship *targetsWingman;

    if (target->objtype != OBJ_ShipType)
    {
        return FALSE;       // non-ship objects are not considered threatening
    }

    targetattackvars = &((Ship *)target)->attackvars;

    // check if target is attacking me
    if (targetattackvars->attacktarget == (SpaceObjRotImpTarg *)me)
    {
        if (targetattackvars->attacksituation == 0) return TRUE; else return FALSE;
    }

    // check if target's leader or wingman is attacking me
    if ((targetsLeader = targetattackvars->myLeaderIs) != NULL)
    {
        dbgAssert(targetattackvars->myWingmanIs == NULL);
        if (targetsLeader->attackvars.attacktarget == (SpaceObjRotImpTarg *)me)
        {
            if (targetattackvars->attacksituation == 0) return TRUE; else return FALSE;
        }
    }
    else if ((targetsWingman = targetattackvars->myWingmanIs) != NULL)
    {
        dbgAssert(targetattackvars->myLeaderIs == NULL);
        if (targetsWingman->attackvars.attacktarget == (SpaceObjRotImpTarg *)me)
        {
            if (targetattackvars->attacksituation == 0) return TRUE; else return FALSE;
        }
    }

    return FALSE;
}

SpaceObjRotImpTarg *FindNewThreateningTarget(Ship *ship,AttackCommand *attack,SpaceObjRotImpTarg *oldtarget)
{
    sdword numShipsToAttack = attack->numTargets;
    SpaceObjRotImpTarg *newtarget;
    vector dist;
    real32 distsqr;
    real32 mindist = REALlyBig;
    sdword mindistj = -1;
    sdword j;
    real32 fighterMaxThreatDist = FIGHTER_MAX_THREAT_DIST[ship->tacticstype];

    dbgAssert(ship->tacticstype < NUM_TACTICS_TYPES);

    // for all ships/targets we are attacking
    for (j=0;j<numShipsToAttack;j++)
    {
        newtarget = attack->TargetPtr[j];
        if (newtarget == oldtarget)
        {
            continue;
        }
        if (isThisShipThreateningMe(newtarget,ship))
        {
            vecSub(dist,newtarget->posinfo.position,ship->posinfo.position);
            distsqr = vecMagnitudeSquared(dist);

            if (distsqr < fighterMaxThreatDist)
            {
                if (distsqr < mindist)
                {
                    mindist = distsqr;
                    mindistj = j;
                }
            }
        }
    }

    if (mindistj != -1)
    {
        return attack->TargetPtr[mindistj];
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : EnemyIsBehindMe
    Description : returns TRUE if enemy is behind me
    Inputs      : enemy, ship
    Outputs     :
    Return      : returns TRUE if enemy is behind me
----------------------------------------------------------------------------*/
bool EnemyIsBehindMe(SpaceObjRotImpTarg *enemy,Ship *me)
{
    vector myheading;
    vector enemytome;

    matGetVectFromMatrixCol3(myheading,me->rotinfo.coordsys);
    vecSub(enemytome,me->posinfo.position,enemy->posinfo.position);

    if (vecDotProduct(myheading,enemytome) > 0.0f)
    {
        // enemy is behind us
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : processSpecialToDo
    Description : processes a special
    Inputs      :
    Outputs     :
    Return      : returns TRUE if special has been completed
----------------------------------------------------------------------------*/
bool processSpecialToDo(CommandToDo *todo)
{
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    Ship *ship;
    ShipStaticInfo *shipstaticinfo;
    bool alldone = TRUE;
    bool shipdone;
    sdword i;

    dbgAssert(numShips > 0);

    if (todo->specialtargets)
    {
        // do the special target command

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            shipdone = TRUE;

            if (shipstaticinfo->custshipheader.CustShipSpecialTarget)
            {
                shipdone = shipstaticinfo->custshipheader.CustShipSpecialTarget(ship,todo->specialtargets);
            }

            alldone &= shipdone;
        }
        return alldone;
    }
    else
    {
        // do a continuous special activate command

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            shipdone = TRUE;

            dbgAssert(shipstaticinfo->specialActivateIsContinuous);

            if (shipstaticinfo->custshipheader.CustShipSpecialActivate)
            {
                shipdone = shipstaticinfo->custshipheader.CustShipSpecialActivate(ship);
            }

            alldone &= shipdone;
        }
        return alldone;
    }
}

/*-----------------------------------------------------------------------------
    Name        : processHaltToDo
    Description : returns TRUE if halt command is finished
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool processHaltToDo(CommandToDo *command,bool passiveAttacked)
{
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;
    bool done = TRUE;
    sdword i;
    Ship *ship;

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if ((passiveAttacked) & (bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
        {
            done &= aitrackSteadyShipDriftOnly(ship);
        }
        else
        {
            done &= aitrackSteadyShip(ship);
        }
    }

    return done;
}

/*-----------------------------------------------------------------------------
    Name        : ShipsOutOfPassiveAttackRange
    Description : returns TRUE if ships being attacked are out of selection's passive attack range
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ships being attacked are out of selection's passive attack range
----------------------------------------------------------------------------*/
bool ShipsInPassiveAttackRange(SelectCommand *selection,AttackCommand *attack)
{
    sdword numShips = selection->numShips;
    sdword numShipsToAttack = attack->numTargets;
    sdword i;
    vector avgposition = { 0.0f, 0.0f, 0.0f };
    Ship *ship;
    vector diff;
    real32 retaliateZone;
    real32 negRetaliateZone;
    bool aShipInRange = FALSE;
    real32 temp;

    dbgAssert(numShips > 0);
    dbgAssert(numShipsToAttack > 0);

    for (i=0;i<numShipsToAttack;i++)
    {
        vecAddTo(avgposition,attack->TargetPtr[i]->posinfo.position);
    }

    vecDivideByScalar(avgposition,(real32)numShipsToAttack,temp);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        retaliateZone = ship->staticinfo->passiveRetaliateZone;
        negRetaliateZone = -retaliateZone;

        vecSub(diff,ship->posinfo.position,avgposition);

        if (isBetweenExclusive(diff.x,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.y,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.z,negRetaliateZone,retaliateZone) )
        {
            aShipInRange = TRUE;
        }
    }

    return aShipInRange;
}

/*-----------------------------------------------------------------------------
    Name        : processPassiveAttackToDo
    Description : processes a passive attack command (attack without moving)
    Inputs      :
    Outputs     :
    Return      : returns TRUE if passive attack has been completed
----------------------------------------------------------------------------*/
bool processPassiveAttackToDo(CommandToDo *attacktodo)
{
    SelectCommand *selection = attacktodo->selection;
    AttackCommand *attack = attacktodo->attack;
    sdword numShips = selection->numShips;
    sdword numShipsToAttack = attack->numTargets;
    Ship *ship;
    ShipStaticInfo *shipstaticinfo;
    SpecificAttackVars *attackvars;
    SpaceObjRotImpTarg *target;
    sdword i;
    bool rotate;

    dbgAssert(numShips > 0);

    if ((numShipsToAttack == 0) || (!ShipsInPassiveAttackRange(selection,attack))
        || ((singlePlayerGame) && (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE ) ))
    {
#ifndef HW_Release
#ifdef gshaw
        if (numShipsToAttack != 0)
        {
            dbgMessagef("\nDropping out of passive attack");
        }
#endif
#endif

        for (i=0;i<numShips;i++)
        {
            soundEventBurstStop(selection->ShipPtr[i],NULL);
        }

        return TRUE;
    }

//fix later and optimize by passing in a rotate flag
    if(attacktodo->ordertype.order == COMMAND_MOVE ||
        attacktodo->ordertype.order == COMMAND_DOCK)
        rotate = FALSE;
    else
        rotate = TRUE;

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];

        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if (target == NULL)
        {
            soundEventBurstStop(ship,NULL);
            attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            InitShipAI(ship,FALSE);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & MULTIPLETARGETS_RETARGET_RATE) == (ship->shipID.shipNumber & MULTIPLETARGETS_RETARGET_RATE))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }
        if(ship->tacticstype == Evasive)
        {
            //ships in evasive don't passive retaliate
            if(rotate == TRUE)
            {
                vector dir;
                vecSub(dir,target->posinfo.position,ship->posinfo.position);
                vecNormalize(&dir);
                aitrackHeading(ship,&dir,0.999f);
            }
            continue;
        }

        if (shipstaticinfo->custshipheader.CustShipAttackPassive)
        {
            //dbgAssert(target->objtype == OBJ_ShipType);     // only target SHIPs should be passive attacked
            shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,rotate);
        }
        attackvars->attackevents = 0;       // clear any events we received
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : processAttackToDo
    Description : processes an attack command
    Inputs      :
    Outputs     :
    Return      : returns TRUE if attack has been completed
----------------------------------------------------------------------------*/
bool processAttackToDo(CommandToDo *attacktodo)
{
    sdword i;
    SelectCommand *selection = attacktodo->selection;
    AttackCommand *attack = attacktodo->attack;
    sdword numShips = selection->numShips;
    sdword numShipsToAttack = attack->numTargets;
    Ship *ship;
    ShipStaticInfo *shipstaticinfo;
    SpecificAttackVars *attackvars;
    bool alldone;
    SpaceObjRotImpTarg *target;
    udword checkbogeyrate;
    SpaceObjRotImpTarg *newtarget;
    bool fighterpair;
    udword flightman;
    Ship *myLeader;
    sdword attackAndMoving;


    dbgAssert(numShips > 0);

    if (numShipsToAttack == 0)
    {                                                               // don't do holding pattern for swarmers (don't want them to go into delta formation)
        if(isSelectionExclusivlyStrikeCraft(attacktodo->selection) && (!selectionHasSwarmers(attacktodo->selection)))
        {
            attacktodo->holdingStartTime = universe.totaltimeelapsed;
            //use leaders heading for holding pattern
            attacktodo->turnAroundTimes = 0;
            //use leaders heading for holding pattern
            matGetVectFromMatrixCol3(attacktodo->holdingPatternVec,selection->ShipPtr[0]->rotinfo.coordsys);
            vecScalarMultiply(attacktodo->holdingPatternVec,attacktodo->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);
            for (i=0;i<selection->numShips;i++)
            {
                //save individual positions
                attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
                soundEventBurstStop(selection->ShipPtr[i],NULL);
            }
            ////////////////////
            //Speech Event for end of attack
            //event num:  STAT_Strike_AttackComplete
            //use battlechatter
            if(attacktodo->selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
            {
                if (battleCanChatterAtThisTime(BCE_STAT_Strike_AttackComplete, attacktodo->selection->ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Strike_AttackComplete, attacktodo->selection->ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
            }
            ///////////////
            attacktodo->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
            return TRUE;    //return to do the holding pattern
        }
        alldone = TRUE;

        // we've killed all the ships, so let's stop movement and wait for next command
        for (i=0;i<selection->numShips;i++)
        {
            //save individual positions
            attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
            soundEventBurstStop(selection->ShipPtr[i],NULL);
        }
        if (attacktodo->ordertype.attributes)
        {
            // something else to do afterwards, so return TRUE without bothering to stabilize ships
            return TRUE;
        }

        for (i=0;i<selection->numShips;i++)
        {
            ship = selection->ShipPtr[i];
            if (!aitrackSteadyShip(ship))
            {
                alldone = FALSE;
            }
        }

        return alldone;

    }

    //check attack conditions as to whether or not we need to
    //go back to guarding a ship...

    if((tacticsInfo.tacticsGuardConditionCheckRate & universe.univUpdateCounter) == tacticsInfo.tacticsGuardConditionCheckFrame)
    {
        if(attacktodo->ordertype.attributes & COMMAND_IS_PROTECTING)
        if(tacticsCheckGuardConditionsDuringAttack(attacktodo))
        {
            for (i=0;i<numShips;i++)
            {
                soundEventBurstStop(selection->ShipPtr[i],NULL);
            }
            return TRUE;
        }
    }

    attackAndMoving = FALSE;
    if(attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        //specialized movement + combat
        attackAndMoving = TRUE;             //set flag for future movement reference
        processMoveToDo(attacktodo,TRUE);   //move the group to the new attack location

    }

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if(!isShipStaticInterceptor(shipstaticinfo) && ship->shiptype != DefenseFighter)
        {
            if (target == NULL)
            {
                soundEventBurstStop(ship,NULL);
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
                InitShipAI(ship,FALSE);
            }

            if (attackvars->multipleAttackTargets)
            {
                dbgAssert(shipstaticinfo->canTargetMultipleTargets);

                if ((universe.univUpdateCounter & MULTIPLETARGETS_RETARGET_RATE) == (ship->shipID.shipNumber & MULTIPLETARGETS_RETARGET_RATE))
                {
                    pickMultipleAttackTargets(ship,attack);
                }
            }

            goto doattack;
        }
        else
        {
            // if we are an interceptor then things get more complicated...

            // execute any flight maneuvers
            if (attackvars->flightmansLeft)
            {
                if (flightmanExecute(ship))
                {
                    attackvars->flightmansLeft--;
                    if (attackvars->flightmansLeft >= 1)
                    {
                        // select another evading maneuver
                        flightmanInit(ship,flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,target),FLIGHTMAN_EVASIVE_PURE));
                    }
                    else
                    {
                        attackvars->attacksituation = 0;
                    }
                }
                goto skipattack;
            }
            else
            {
                dbgAssert(attackvars->attacksituation == 0);
            }

            // if we have a leader
            if(ship->tacticstype != Aggressive) //only do wingman stuff for evasive
            if ((myLeader = attackvars->myLeaderIs) != NULL)
            {
                SpaceObjRotImpTarg *myLeaderTarget;
                bool shouldrotate = TRUE;

                dbgAssert(attackvars->myWingmanIs == NULL);

                if (attackvars->attacktarget != NULL)
                {
                    goto doattack;
                }

                if(ship->shiptype != DefenseFighter)
                {
                    if ((myLeaderTarget = myLeader->attackvars.attacktarget) != NULL)
                    {
                        if (shipstaticinfo->custshipheader.CustShipAttackPassive)
                        {
                            vector leadhead;
                            vector othervec;

                            matGetVectFromMatrixCol3(leadhead,myLeader->rotinfo.coordsys);
                            vecSub(othervec,ship->posinfo.position,myLeaderTarget->posinfo.position);
                            if(vecDotProduct(othervec,leadhead) < 0)
                            {
                                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)myLeaderTarget,TRUE);
                                shouldrotate = FALSE;
                            }
                        }
                    }
                    else
                    {
                        soundEventBurstStop(ship,NULL);
                    }
                }

                // track leader position
                formationWingmanTrackLeader(ship,myLeader,shouldrotate);

                goto skipattack;
            }

            if (((attackvars->myLeaderIs) || (attackvars->myWingmanIs)) && ship->tacticstype != Aggressive)
            {
                fighterpair = TRUE;
                checkbogeyrate = FIGHTER_PAIR_CHECK_RATE[ship->tacticstype];
            }
            else
            {
                fighterpair = FALSE;
                checkbogeyrate = FIGHTER_SINGLE_CHECK_RATE[ship->tacticstype];
            }

            if (target != NULL)
            {
                if ((universe.univUpdateCounter & checkbogeyrate) == (CHECKBOGEY_FRAME & checkbogeyrate))
                {
                    if (!isThisShipThreateningMe(target,ship))
                    {
                        // we should switch targets to a target that is threatening me, if there is one
                        newtarget = FindNewThreateningTarget(ship,attack,target);
                        if (newtarget != NULL)
                        {
                            soundEventBurstStop(ship,NULL);
                            if ((fighterpair) && ((gamerand() & 255) < FLIGHTMAN_SPLIT_PROB[ship->tacticstype]))
                            {
                                Ship *wingman = attackvars->myWingmanIs;

                                dbgAssert(wingman);

                                wingman->attackvars.flightmansLeft = 1;
                                wingman->attackvars.attacksituation = ATTACKSITUATION_SPLITTING;
                                if (gamerand() & 1)
                                {
                                    flightmanInitWithFlags(wingman,FLIGHTMAN_IMMELMAN,IMMELMAN_INVERT);
                                }
                                else
                                {
                                    flightmanInit(wingman,FLIGHTMAN_HARDBANK);
                                }

                                attackvars->flightmansLeft = 1;
                                attackvars->attacksituation = ATTACKSITUATION_SPLITTING;
                                flightmanInitWithFlags(ship,FLIGHTMAN_IMMELMAN,0);
                            }
                            else
                            {
                                if ((gamerand() & 255) < FLIGHTMAN_EVASIVEPURE_PROB[ship->tacticstype])
                                {
                                    flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_PURE);

                                    attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[ship->tacticstype];
                                    attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;
                                }
                                else
                                {
                                    // sensed threatening enemy attack
                                    if (EnemyIsBehindMe(newtarget,ship))
                                    {
                                        flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_BEHIND);
                                    }
                                    else
                                    {
                                        flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_FRONT);
                                    }
                                    attackvars->flightmansLeft = 1;
                                    attackvars->attacksituation = ATTACKSITUATION_EVADINGREPOS;
                                }
                                flightmanInit(ship,flightman);
                            }

                            attackvars->attacktarget = target = newtarget;
                            InitShipAI(ship,FALSE);
                            goto skipattack;
                        }
                        else
                        {
                            // no new threatening target, so keep attacking old one.
                            goto doattack;
                        }
                    }
                    else
                    {
                        // ship is threatening me, perhaps a token pure evasive maneuver if we are in Evasive tactics?
                        if ((ship->tacticstype == Evasive) && ((universe.univUpdateCounter & FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKRATE) == FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKFRAME))
                        {
                            if ((gamerand() & 255) < FLIGHTMAN_TOKEN_EVASIVEPURE_PROB)
                            {
                                flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,target),FLIGHTMAN_EVASIVE_PURE);

                                attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[ship->tacticstype];
                                attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;

                                flightmanInit(ship,flightman);

                                goto skipattack;
                            }
                        }

                        goto doattack;
                    }
                }
                else
                {
                    // just attack our target
                    goto doattack;
                }
            }
            else
            {
                // we don't currently have a target.  Let's pick one.
                soundEventBurstStop(ship,NULL);
                newtarget = FindNewThreateningTarget(ship,attack,target);
                if (newtarget != NULL)
                {
                    if ((gamerand() & 255) < FLIGHTMAN_EVASIVEPURE_PROB[ship->tacticstype])
                    {
                        flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_PURE);

                        attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[ship->tacticstype];
                        attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;
                    }
                    else
                    {
                        // sensed threatening enemy attack
                        if (EnemyIsBehindMe(newtarget,ship))
                        {
                            flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_BEHIND);
                        }
                        else
                        {
                            flightman = flightmanGetRandom(GenericInterceptorGetFlightManProb(ship,newtarget),FLIGHTMAN_EVASIVE_FRONT);
                        }
                        attackvars->flightmansLeft = 1;
                        attackvars->attacksituation = ATTACKSITUATION_EVADINGREPOS;
                    }

                    flightmanInit(ship,flightman);
                    attackvars->attacktarget = target = newtarget;
                    InitShipAI(ship,FALSE);

                    goto skipattack;
                }
                else
                {
                    // find a new target to pick on
                    attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
                    InitShipAI(ship,FALSE);

                    if ((fighterpair) && (target->objtype == OBJ_ShipType) &&
                        ((((Ship *)target)->attackvars.myWingmanIs == NULL) && (((Ship *)target)->attackvars.myLeaderIs == NULL)) &&
                        ((gamerand() & 255) < FLIGHTMAN_SANDWICH_PROB[ship->tacticstype]) )
                    {
                        Ship *wingman = attackvars->myWingmanIs;

                        dbgAssert(wingman);
                        wingman->attackvars.attacktarget = target;
                        InitShipAI(wingman,FALSE);

                        wingman->flightmanState2 = SANDWICH_RIGHT;
                        flightmanInitWithFlags(wingman,FLIGHTMAN_SANDWICH,(udword)target);
                        wingman->attackvars.flightmansLeft = 1;
                        wingman->attackvars.attacksituation = ATTACKSITUATION_SANDWICH;

                        ship->flightmanState2 = SANDWICH_LEFT;
                        flightmanInitWithFlags(ship,FLIGHTMAN_SANDWICH,(udword)target);
                        attackvars->flightmansLeft = 1;
                        attackvars->attacksituation = ATTACKSITUATION_SANDWICH;
                        goto skipattack;
                    }
                    else    // we didn't find a threatening enemy attack. Perhaps a celebratory barrel roll?
                    if (attackvars->attackevents & ATTACKEVENT_ENEMYTARGET_DIED)
                    {
                        flightmanInit(ship,FLIGHTMAN_BARREL_ROLL);
                        attackvars->flightmansLeft = 1;
//                        speechEvent(ship, BattleEnemyKilled, 0);
                        goto skipattack;
                    }

                    goto doattack;
                }

            } // end if target != NULL

        } // end if ship is Interceptor

doattack:
        if(attackAndMoving)
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttack)
            {
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,ship->staticinfo->bulletRange[ship->tacticstype]);
            }
        }
skipattack:
        ship->shipidle = FALSE;
        attackvars->attackevents = 0;       // clear any events we received

    } // end for

    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : fixWallFlipTurnSelection
    Description : Fixes selection when ships in WALL only formation attempt a flipturn
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void fixWallFlipTurnSelection(SelectCommand *selection)
{
    sdword i;

    for(i=1;i<selection->numShips;i++)
    {
        selection->ShipPtr[i]->formationOffset.y = -selection->ShipPtr[i]->formationOffset.y;
    }

}
/*-----------------------------------------------------------------------------
    Name        : fixFlipTurnSelection
    Description : Fixes selection when ships in formation attempt a flipturn
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void fixFlipTurnSelection(SelectCommand *selection,SelectCommand *global)
{
    sdword i;//,j,switcher;
    Ship *tempship;
    //vector tempvec,tempvec2; //temporary formation vector
    //j=0;
    for(i=1;i<selection->numShips;i++)
    {
        //save temporary ship
        tempship = selection->ShipPtr[i];

        /*
        tempvec = selection->ShipPtr[i]->formationOffset;

        i++;
        if(i >= selection->numShips)
        {
            */
            tempship->formationOffset.y = -tempship->formationOffset.y;
        /*    break;              //done
        }

        while(tempship != global->ShipPtr[j])
            j++;
        //while loop exits when j points to same ship as temp ship

        switcher = j;
        while(global->ShipPtr[j] != selection->ShipPtr[i])
            j++;
        //while loop exits when j points to other ship to switch

        //swap ships and continue;
        selection->ShipPtr[i-1] = selection->ShipPtr[i];
        tempvec2 = selection->ShipPtr[i]->formationOffset;
        selection->ShipPtr[i-1]->formationOffset = tempvec;

        selection->ShipPtr[i] = tempship;
        selection->ShipPtr[i]->formationOffset = tempvec2;

        global->ShipPtr[switcher] = selection->ShipPtr[i-1];
        global->ShipPtr[j] = tempship;
        //global->ShipPtr[switcher]->formationOffset = selection->ShipPtr[i-1]->formationOffset;
        //global->ShipPtr[j]->formationOffset = tempvec;
        */
    }
}
void formGroups(SelectCommand *selection,SelectCommand *selectionDefenders,SelectCommand *selectionCorvettes,SelectCommand *selectionCapitalShips,SelectCommand *selectionInterceptors)
{
    sdword a,i,j,k,l;
    selectionDefenders->numShips = 0;       //i
    selectionCorvettes->numShips = 0;       //j
    selectionCapitalShips->numShips = 0;    //k
    selectionInterceptors->numShips = 0;    //l
    i = j = k = l = 0;

    for(a=0;a<selection->numShips;a++)
    {
        if((selection->ShipPtr[a]->staticinfo->shipclass == CLASS_Fighter) ||
           (selection->ShipPtr[a]->staticinfo->shiptype == TargetDrone))
        {
            //check if defender, else fighter
            if(selection->ShipPtr[a]->staticinfo->shiptype == LightDefender ||
                selection->ShipPtr[a]->staticinfo->shiptype == HeavyDefender)
            {
                selectionDefenders->ShipPtr[i] = selection->ShipPtr[a];
                i++;
                selectionDefenders->timeLastStatus = selection->timeLastStatus;
                selectionDefenders->numShips++;
            }
            else
            {
                selectionInterceptors->ShipPtr[l] = selection->ShipPtr[a];
                l++;
                selectionInterceptors->timeLastStatus = selection->timeLastStatus;
                selectionInterceptors->numShips++;
            }
        }
        else if(selection->ShipPtr[a]->staticinfo->shipclass == CLASS_Corvette)
        {
            selectionCorvettes->ShipPtr[j] = selection->ShipPtr[a];
            j++;
            selectionCorvettes->timeLastStatus = selection->timeLastStatus;
            selectionCorvettes->numShips++;

        }
        else        // capital ship and miscellaneous ships
        {
            //captital ship
            selectionCapitalShips->ShipPtr[k] = selection->ShipPtr[a];
            k++;
            selectionCapitalShips->timeLastStatus = selection->timeLastStatus;
            selectionCapitalShips->numShips++;
        }
    }

}
//group types
#define DEFENDERS       10
#define INTERCEPTORS    11
#define CAPITALS        12
#define CORVETTES       13

bool delegateCommand(CommandToDo *attacktodo,sdword group,sdword doform, SelectCommand *globalselection)
{
    sdword i;
    SelectCommand *selection = attacktodo->selection;
    AttackCommand *attack = attacktodo->attack;
    sdword numShips = selection->numShips;
    sdword allNotAttacking;
    sdword formationoverride = FALSE;
    Ship *ship;
    sdword pee;
    Ship *tempship;
    ShipStaticInfo *shipstaticinfo;
    SpecificAttackVars *attackvars;
    SpaceObjRotImpTarg *target;
    Ship *formationLeader;
    vector leadhead,othervec;
    real32 maxdist;
    bool attackAndMoving;

    formationLeader = selection->ShipPtr[0];   //set leader ship

    attackAndMoving = FALSE;
    if(attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        //specialized movement + combat
        attackAndMoving = TRUE;             //set flag for future movement reference
        processMoveLeaderToDo(attacktodo,TRUE);   //move the group to the new attack location

    }

    //perform fancy pants reordering of selection if we leader does a flip turn
    if(group == INTERCEPTORS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttack)
            {
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,ship->staticinfo->bulletRange[ship->tacticstype]);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if(shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        }
skipattack:
        if(ship->tacticsTalk != 0)
        {
            if(ship->formationcommand->formation.flipselectionfixed != TRUE)
            {
                //lock the leaders formation!
                //fix later:  this is bad because this 'formation' is
                //only a pseudo formation...it will fuck up other sub formations
                //FUCK!
                if(!bitTest(ship->tacticsTalk,TACTALK_BARRELROLL))
                {
                    //if(ship->flightman == FLIGHTMAN_IMMELMAN)
                    //{
                        //for immelman, lets not fix the formation, it should look good maybe
                    //    ship->formationcommand->formation.needFix = FALSE;
                    //    lockFormation(ship->formationcommand,0);
                    //}
                    //else
                    //{
                        ship->formationcommand->formation.needFix = TRUE;
                    //do special heading reversal in formation lock so
                    //as to begin forming the new formation rightaway
                    lockFormation(ship->formationcommand,1);
                    //}
                }
                else
                {
                    ship->formationcommand->formation.needFix = FALSE;
                    lockFormation(ship->formationcommand,0);
                }

                ship->formationcommand->formation.flipselectionfixed = TRUE;
                for(i=1;i<numShips;i++)
                {
                    flightmanInitFunc(selection->ShipPtr[i],ship->flightman,0);

#ifdef bpasechn
    #ifdef DEBUG_TACTICS
                    if(ship->flightman == FLIGHTMAN_NULL)
                        _asm int 3
    #endif
#endif

                    selection->ShipPtr[i]->tacticsNeedToFlightMan=TRUE;
                }
            }
        }
        else
        {
            if(ship->formationcommand->formation.flipselectionfixed == TRUE)
            {
                //only if did flipturn;
                unlockFormation(ship->formationcommand);
                if(ship->formationcommand->formation.needFix == TRUE)
                {
                    if(ship->formationcommand->formation.formationtype != WALL_FORMATION)
                    {
                        fixFlipTurnSelection(attacktodo->selection,globalselection);
                    }
                    else
                    {
                        fixWallFlipTurnSelection(attacktodo->selection);
                    }
                    ship->formationcommand->formation.needFix = FALSE;

                    //negate this bool value so the formation status is uptodate.
                    ship->formationcommand->formation.flipState = !ship->formationcommand->formation.flipState;
                }
            }
            ship->formationcommand->formation.flipselectionfixed = FALSE;
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;
             attackvars->attackevents = 0;       // clear any events we received

            //perform ship flight maneuvers here
            if(ship->tacticsNeedToFlightMan)
            {
#ifdef bpasechn
    #ifdef DEBUG_TACTICS
                if(ship->flightman == FLIGHTMAN_NULL)
                    _asm int 3
#endif
                if(ship->flightman != FLIGHTMAN_NULL)
#endif
                {
                    if(flightmanExecute(ship))
                    {
                        ship->tacticsNeedToFlightMan = FALSE;
                    }
                    else
                    {
                        formationoverride = TRUE;
                    }
                    continue;
                }
            }
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                //only do the attack if in proper attack state...
                //quick fix
                if(attacktodo->formation.formationtype == SPHERE_FORMATION)
                {
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                    bitSet(ship->dontrotateever,4);
                }
                else
                {
                    matGetVectFromMatrixCol3(leadhead,formationLeader->rotinfo.coordsys);
                    vecSub(othervec,ship->posinfo.position,target->posinfo.position);
                    if(vecDotProduct(othervec,leadhead) < 0)
                    {
                        shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                        bitSet(ship->dontrotateever,4);
                    }
                    else
                    {
                        bitClear(ship->dontrotateever,4);
                    }
                }
            }

            /*
            if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                //only do the attack if in proper attack state...
                if(!doform)
                {
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                    continue;
                }
                if(ship->tacticsFormationVar1 == TRUE)
                    if(selection->ShipPtr[0]->aistateattack != 4)
                    {
                        //set flag so we can try to attack because leader has finished attack pass
                        continue;
                    }
                    else
                    {
                        ship->tacticsFormationVar1 = FALSE;
                    }


                if(InterceptorInRange(ship,target))
                {
                    //set a state flag here so when it tries to passive attack again later, it won't
                    //unless the leader is again attacking...
                    ship->tacticsFormationVar1 = FALSE;
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                }
                else
                {
                    //ship is too close
                    ship->tacticsFormationVar1 = TRUE;
                    allNotAttacking++;
                }
            }
            */
        }
        if(doform)
        {
            if(!formationoverride)
            {
                processFormationToDo(attacktodo,FALSE,FALSE);
            }
            else
            {
                //do formation, but loosish...
                processFormationToDo(attacktodo,FALSE,TRUE);
            }
        }

        for(i=0;i<selection->numShips;i++)
        {
            bitClear(selection->ShipPtr[i]->dontrotateever,4);
        }
    }
    else if(group == DEFENDERS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;


        if (target == NULL)
        {
            soundEventBurstStop(ship,NULL);
            attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            InitShipAI(ship,FALSE);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & MULTIPLETARGETS_RETARGET_RATE) == (ship->shipID.shipNumber & MULTIPLETARGETS_RETARGET_RATE))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }

        maxdist = tacticsMaxDistToTarget(selection,target);
        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttack)
            {
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,maxdist);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                //only do the attack if in proper attack state...
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        attackvars->attackevents = 0;       // clear any events we received
        }
        //these need to be moved up...

        if(doform)
        {
            processFormationToDo(attacktodo,FALSE,TRUE);
        }

    }
    else if(group == CORVETTES)
    {
        if(selection->ShipPtr[0]->shiptype == MinelayerCorvette)
        {
            for(pee=1;pee<selection->numShips;pee++)
            {
                if(selection->ShipPtr[pee]->shiptype != MinelayerCorvette)
                {
                    tempship = selection->ShipPtr[0];
                    selection->ShipPtr[0] = selection->ShipPtr[pee];
                    selection->ShipPtr[pee] = tempship;
                }
            }
        }

        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttack)
            {
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,ship->staticinfo->bulletRange[ship->tacticstype]);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        }

        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttackPassive)

            {
                //only do the attack if in proper attack state...
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
            else if(shipstaticinfo->shiptype == MinelayerCorvette)
            {
                bitSet(ship->specialFlags,SPECIAL_BrokenFormation);
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,ship->staticinfo->bulletRange[ship->tacticstype]);
            }
            attackvars->attackevents = 0;       // clear any events we received
        }

        if(doform)
        {
            if(selection->ShipPtr[0]->aistateattack == 3 || selection->ShipPtr[0]->aistateattack == 4)
            {
            //leader of formation is turning around or reaproaching, so lets do a real formation...
                processFormationToDo(attacktodo,FALSE,FALSE);
            }
            else
            {
                processFormationToDo(attacktodo,FALSE,TRUE);
            }
        }
    }
    else if(group == CAPITALS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;


        if (target == NULL)
        {
            soundEventBurstStop(ship,NULL);
            attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            InitShipAI(ship,FALSE);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & MULTIPLETARGETS_RETARGET_RATE) == (ship->shipID.shipNumber & MULTIPLETARGETS_RETARGET_RATE))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }

        maxdist = tacticsMaxDistToTarget(selection,target);
        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipAttack)
            {
                shipstaticinfo->custshipheader.CustShipAttack(ship,target,maxdist);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else if(shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
            }
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                if (shipstaticinfo->custshipheader.CustShipAttackPassive)
                {
                    //only do the attack if in proper attack state...
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                }
            }
            ship->shipidle = FALSE;
            attackvars->attackevents = 0;       // clear any events we received
        }

        if(doform)
        {
            processFormationToDo(attacktodo,FALSE,TRUE);
        }


    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : targetIsntNearShip
    Description : determines if ships target is in or out of gun range

    Inputs      :
    Outputs     :
    Return      : returns TRUE if ships target is out of gun range
----------------------------------------------------------------------------*/

bool targetIsntNearShip(Ship *ship,SpaceObjRotImpTarg *target)
{
    vector distvect;
    real32 dist;
    vecSub(distvect,target->posinfo.position,ship->posinfo.position);
    dist = vecMagnitudeSquared(distvect);
    dist = fsqrt(dist);

    return(dist > ship->staticinfo->bulletRange[ship->tacticstype]);
}

/*-----------------------------------------------------------------------------
    Name        : processAttackToDoInFormation
    Description : processes an attack command for a group of ships in
                  formation...maintains formation
    Inputs      :
    Outputs     :
    Return      : returns TRUE if attack has been completed
----------------------------------------------------------------------------*/
bool processAttackToDoInFormation(CommandToDo *attacktodo)
{
    sdword donness = 0;
    CommandToDo fakeCommand;
    MaxSelection selectionDefenders, selectionCorvettes,selectionCaptialShips,selectionInterceptors;
    AttackCommand *attack = attacktodo->attack;
    sdword numShipsToAttack = attack->numTargets;
    SelectCommand *selection = attacktodo->selection;
    bool alldone;
    ShipStaticInfo *shipstaticinfo;
    Ship *ship;
    SpaceObjRotImpTarg *target;
    SpaceObjRotImpTarg *leadertarget;
    sdword i;
    sdword should = TRUE;
    bool focusfire = FALSE;

    SpecificAttackVars *attackvars;

    dbgAssert(selection->numShips > 0);

    if (selection->ShipPtr[0]->tacticstype == Aggressive)
    {
        focusfire = TRUE;
    }

    if (numShipsToAttack == 0)
    {                                                               // don't do holding pattern for swarmers (don't want them to go into delta formation)
        if(isSelectionExclusivlyStrikeCraft(attacktodo->selection) && (!selectionHasSwarmers(attacktodo->selection)))
        {
            attacktodo->holdingStartTime = universe.totaltimeelapsed;
            //use leaders heading for holding pattern
            attacktodo->turnAroundTimes = 0;
            //use leaders heading for holding pattern
            matGetVectFromMatrixCol3(attacktodo->holdingPatternVec,selection->ShipPtr[0]->rotinfo.coordsys);
            vecScalarMultiply(attacktodo->holdingPatternVec,attacktodo->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);
            for (i=0;i<selection->numShips;i++)
            {
                //save individual positions
                attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
                soundEventBurstStop(selection->ShipPtr[i],NULL);
            }
            ////////////////////
            //Speech Event for end of attack
            //event num:  STAT_Strike_AttackComplete
            //use battlechatter
            if(attacktodo->selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
            {
                if (battleCanChatterAtThisTime(BCE_STAT_Strike_AttackComplete, attacktodo->selection->ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Strike_AttackComplete, attacktodo->selection->ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
            }

            ///////////////

            attacktodo->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
            return TRUE;    //return to do the holding pattern
        }
        alldone = TRUE;

        // we've killed all the ships, so let's stop movement and wait for next command
        for (i=0;i<selection->numShips;i++)
        {
            //save individual positions
            attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
            soundEventBurstStop(selection->ShipPtr[i],NULL);
        }
        if (attacktodo->ordertype.attributes)
        {
            // something else to do afterwards, so return TRUE without bothering to stabilize ships
            return TRUE;
        }

        for (i=0;i<selection->numShips;i++)
        {
            ship = selection->ShipPtr[i];
            if (!aitrackSteadyShip(ship))
            {
                alldone = FALSE;
            }
        }

        return alldone;

    }


    //check attack conditions as to whether or not we need to
    //go back to guarding a ship...

    if((tacticsInfo.tacticsGuardConditionCheckRate & universe.univUpdateCounter) == tacticsInfo.tacticsGuardConditionCheckFrame)
    {
        if(attacktodo->ordertype.attributes & COMMAND_IS_PROTECTING)
        if(tacticsCheckGuardConditionsDuringAttack(attacktodo))
        {
            for (i=0;i<selection->numShips;i++)
            {
                soundEventBurstStop(selection->ShipPtr[i],NULL);
            }
            return TRUE;
        }
    }

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        ship->shipidle = FALSE;
        target = attackvars->attacktarget;
        if(i==0)
            leadertarget = target;

        if (target == NULL)
        {
            if(i == 0)
            {
                soundEventBurstStop(ship,NULL);
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
                InitShipAI(ship,FALSE);
                leadertarget = target;
            }
            else
            {
                soundEventBurstStop(ship,NULL);

                if ( leadertarget &&
                     (
                     ( ((leadertarget->objtype != OBJ_ShipType) || (((Ship *)leadertarget)->formationcommand == NULL)) ) ||
                     (focusfire)
                     )
                   )
                {
                    attackvars->attacktarget = leadertarget;
                }
                else
                {
                    attackvars->attacktarget = AttackClosestOpponent(ship,attack);
                }
                InitShipAI(ship,FALSE);
            }
        }

        if(i >= 1)
        {
            if ( leadertarget &&
                 ((leadertarget->objtype != OBJ_ShipType) || (((Ship *)leadertarget)->formationcommand == NULL)) )
            {
                attackvars->attacktarget = leadertarget;    // override with leadertarget if we are attacking a ship that isn't in formation
            }

            if(attacktodo->formation.formationtype == SPHERE_FORMATION)
            {
                //do this check infrequently
                if ((universe.univUpdateCounter & SPHERE_CHECK_TARGET_MASK) == SPHERE_CHECK_TARGET_FRAME)
                {
                    if(targetIsntNearShip(ship,ship->attackvars.attacktarget))
                    {
                        soundEventBurstStop(ship,NULL);
                        attackvars->attacktarget = AttackClosestOpponent(ship,attack);//leadertarget;
                        InitShipAI(ship,FALSE);
                    }
                }
            }
        }
        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & MULTIPLETARGETS_RETARGET_RATE) == (ship->shipID.shipNumber & MULTIPLETARGETS_RETARGET_RATE))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }
    }

    //ships should each be guaranteed to have their proper targets now!


    //GET TARGET HERE!

    //then, if in sphere formation, and target doesn't = center ship,
    //    make it the center ship, otherwise, center ship is target, so keep
    //    pumpleing it


    fakeCommand = *attacktodo;
    formGroups(selection,(SelectCommand *)&selectionDefenders,(SelectCommand *)&selectionCorvettes,(SelectCommand *)&selectionCaptialShips,(SelectCommand *)&selectionInterceptors);
    if(attacktodo->formation.formationtype == SPHERE_FORMATION)
    {
        //if(attacktodo->formation.enders != TRUE || attacktodo->formation.endership != (Ship *)leadertarget)
        if(attacktodo->formation.enders != TRUE)
        {
            //FormationSphereInfo *sphereInfo = (FormationSphereInfo *)attacktodo->formation.formationSpecificInfo;
            //SphereVar *sphereVar;
            attacktodo->formation.enders = TRUE;
            //attacktodo->formation.endership = (Ship *)leadertarget;
            //sphere formation...lets do enders game..
            //
            //first we need to save old formation type, so we can change it back

            //attacktodo->selection->ShipPtr[attacktodo->selection->numShips] = attacktodo->selection->ShipPtr[0];
            //attacktodo->selection->numShips++;
            //attacktodo->selection->ShipPtr[0] = (Ship *)target;
            formationContentHasChanged(attacktodo);
            should = FALSE;
        }
        else
        {
            should = FALSE;
        }
        processFormationToDo(attacktodo,FALSE,TRUE);
    }
    else
    {
        should = TRUE;
    }

    if(selectionInterceptors.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionInterceptors;
        donness |= delegateCommand(&fakeCommand,INTERCEPTORS,should,attacktodo->selection);
    }

    if(selectionDefenders.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionDefenders;
        donness |= delegateCommand(&fakeCommand,DEFENDERS,should,attacktodo->selection);
    }
    if(selectionCorvettes.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionCorvettes;
        donness |= delegateCommand(&fakeCommand,CORVETTES,should,attacktodo->selection);
    }

    if(selectionCaptialShips.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionCaptialShips;
        donness |= delegateCommand(&fakeCommand,CAPITALS,should,attacktodo->selection);
        //processAttackToDo(&fakeCommand);
    }

    return(donness);

}

/*-----------------------------------------------------------------------------
    Name        : hyperspaceOutPingTimeOut
    Description : timeout cb function that determines if the ping should time out
    Inputs      : hellaPing - the ping we're evaluating
                  userID - cast to ship * for pointer to proximity sensor.
                  userData - ignore
                  bRemoveReferneces - if TRUE, remove references to pingDyingObject
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
bool hyperspaceOutPingTimeOut(struct ping *hellaPing, udword userID, ubyte *userData, bool bRemoveReferences)
{
    if (bRemoveReferences)
    {
        if ((SpaceObj *)userID == pingDyingObject)
        {
            return(TRUE);
        }
        return(FALSE);
    }

    if(((Ship *)userID)->flags & SOF_Hyperspace)
    {
        return(TRUE);
    }
    return(FALSE);
}
/*-----------------------------------------------------------------------------
    Name        : hyperspaceInPingTimeOut
    Description : timeout cb function that determines if the ping should time out
    Inputs      : hellaPing - the ping we're evaluating
                  userID - cast to ship * for pointer to proximity sensor.
                  userData - ignore
                  bRemoveReferneces - if TRUE, remove references to pingDyingObject
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
 bool hyperspaceInPingTimeOut(struct ping *hellaPing, udword userID, ubyte *userData, bool bRemoveReferences)
{
    if (bRemoveReferences)
    {
        if ((SpaceObj *)userID == pingDyingObject)
        {
            return(TRUE);
        }
        return(FALSE);
    }
    if(((Ship *)userID)->shipSinglePlayerGameInfo->shipHyperspaceState == SHIPHYPERSPACE_NONE)
    {
        return(TRUE);
    }
    return(FALSE);
}

bool needToWaitForThisShip(Ship *ship)
{
    Node *node;
    CommandToDo *command;
    SelectCommand *selection;
    sdword i;

    if(ship->shiptype == AdvanceSupportFrigate)
    {
        //ship is an ASF
        //lets do a todo list walk!
        node = universe.mainCommandLayer.todolist.head;

        while(node != NULL)
        {

            command = (CommandToDo *)listGetStructOfNode(node);
            selection = command->selection;
            if(command->ordertype.order == COMMAND_DOCK)
            {
                for(i=0;i<selection->numShips;i++)
                {
                    if(selection->ShipPtr[i]->dockvars.dockship == ship)
                    {
                        if(selection->ShipPtr[i]->dockvars.dockstate2 == ASF_WAITLATCHPOINT ||
                           selection->ShipPtr[i]->dockvars.dockstate2 == ASF_BACKUP ||
                           selection->ShipPtr[i]->dockvars.dockstate2 == ASF_WAIT ||
                           selection->ShipPtr[i]->dockvars.dockstate2 == ASF_REFUEL)
                        {
                            continue;
                        }
                        return TRUE;
                    }
                }
            }
            else if(command->ordertype.order == COMMAND_LAUNCHSHIP)
            {
                if(command->launchship.receiverShip == ship)
                {
                    if(ship->collMyBlob != NULL)
                    {
                        //ship hsing is launchiong a ship and that ship is outside...so finish it off.
                        return TRUE;        //need to wait for this ship!
                    }
                }
            }
            node = node->next;
        }
    }
    else if(ship->shiptype == Mothership)
    {
        //ship is an ASF
        //lets do a todo list walk!
        node = universe.mainCommandLayer.todolist.head;

        while(node != NULL)
        {

            command = (CommandToDo *)listGetStructOfNode(node);
            selection = command->selection;
            if(command->ordertype.order == COMMAND_LAUNCHSHIP)
            {
                if(command->launchship.receiverShip == ship)
                {
                    if(command->selection->ShipPtr[0]->collMyBlob != NULL)
                    {
                        //ship hsing is launchiong a ship and that ship is outside...so finish it off.
                        return TRUE;        //need to wait for this ship!
                    }
                }
            }
            node = node->next;
        }
    }

    //don't need to wait for anything else
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : processMpHyperspaceingToDo
    Description : processes the hyperspaceing command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define HW_ETHER_WAIT       TW_MULTIPLAYER_HYPERSPACE_WAIT_IN_ETHER_TIME

bool processMpHyperspaceingToDo(CommandToDo *movetodo)
{
    sdword i,j,donehsflag;
    vector shipheading,othervector;
    real32 dotproduct;
    real32 timefraction;
    sdword shipslice;
    bool dontgoyet=FALSE;
    for(i=0;i<movetodo->selection->numShips;i++)
    {
       movetodo->selection->ShipPtr[i]->posinfo.isMoving = TRUE;
    }
    switch(movetodo->hyperspaceState)
    {
    case COM_HYPERSPACE_START:
        //check if any ASF's in selection
        //if yes, we won't hs out until all slots on asf are full
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            if(aitrackZeroRotationAnywhere(movetodo->selection->ShipPtr[i])
               && aitrackZeroVelocity(movetodo->selection->ShipPtr[i]))
            {
                movetodo->selection->ShipPtr[i]->rotinfo.rotspeed.x=0.0f;
                movetodo->selection->ShipPtr[i]->rotinfo.rotspeed.y=0.0f;
                movetodo->selection->ShipPtr[i]->rotinfo.rotspeed.z=0.0f;
                movetodo->selection->ShipPtr[i]->posinfo.velocity.x=0.0f;
                movetodo->selection->ShipPtr[i]->posinfo.velocity.y=0.0f;
                movetodo->selection->ShipPtr[i]->posinfo.velocity.z=0.0f;
                continue;
            }
            dontgoyet=TRUE;

        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            if(needToWaitForThisShip(movetodo->selection->ShipPtr[i]))
            {
                //ship needs us to wait!
                return FALSE;
            }
        }
        if(dontgoyet)
        {
            return FALSE;
        }
        //set to hyperspace out!
        spHyperspaceSelectionOut(movetodo->selection);
        //setup timer to count down...
        movetodo->hyperSpaceingTime = universe.totaltimeelapsed;
        movetodo->hyperspaceState=COM_HYPERSPACE_OUT;

        //reset ping flag status
        for(j=0;j<movetodo->selection->numShips;j++)
        {
            movetodo->selection->ShipPtr[j]->hyperspacePing = FALSE;
        }
        return FALSE;

    case COM_HYPERSPACE_OUT:
        //check if all hyperspaced out...
        timefraction = universe.totaltimeelapsed - movetodo->hyperSpaceingTime;
        shipslice = (sdword) (timefraction/TW_PING_MAX_DISPERSAL)*movetodo->selection->numShips;
        if(shipslice-movetodo->selection->numShips == 1 ||
            shipslice > movetodo->selection->numShips )
        {
            shipslice = movetodo->selection->numShips;
        }
        else if(shipslice < 1)
        {
            shipslice = 1;
        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            for(j=0;j<shipslice;j++)
            {
                if(!movetodo->selection->ShipPtr[j]->hyperspacePing)
                {
                    ping *newPing;

                    movetodo->selection->ShipPtr[j]->hyperspacePing=TRUE;
                    //create ping HERE!
                    newPing = pingCreate(NULL, (SpaceObj *)movetodo->selection->ShipPtr[j], (pingeval)hyperspaceOutPingTimeOut, NULL, 0, (udword)movetodo->selection->ShipPtr[j]);
                    newPing->c = TW_HW_PING_COLOUR_OUT;
                    newPing->size = TW_HW_PING_MAX_SIZE_OUT;
                    newPing->minScreenSize = primScreenToGLScaleX(2);
                    newPing->minSize = TW_HW_PING_MIN_SIZE_OUT;
                    newPing->pingDuration = 1.0f / TW_HW_PING_RATE_OUT;
                    newPing->interPingPause = 0.25;
                    newPing->TOMask         = PTOM_Hyperspace;

                    ///////////////////////////////
                    //PUT SPEECH EVENTS HERE!
                    ///////////////////////////////
                }
            }
            if(!(movetodo->selection->ShipPtr[0]->flags & SOF_Hyperspace))
            {
                return(FALSE);
            }
        }

        //tell code that all ships have no pings...so we can create them for hyping in..
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            movetodo->selection->ShipPtr[i]->hyperspacePing=FALSE;
        }

        movetodo->hyperSpaceingTime = universe.totaltimeelapsed+HW_ETHER_WAIT;
        movetodo->hyperspaceState=COM_HYPERSPACE_WAIT;
        break;
    case COM_HYPERSPACE_WAIT:
        //check if done in the ether
        if(movetodo->hyperSpaceingTime<= universe.totaltimeelapsed)
        {
            //hyperspaced out!
            hvector movetoplace;
            movetoplace.x = movetodo->move.destination.x;
            movetoplace.y = movetodo->move.destination.y;
            movetoplace.z = movetodo->move.destination.z;

            matGetVectFromMatrixCol3(shipheading,movetodo->selection->ShipPtr[0]->rotinfo.coordsys);
            vecSet(othervector,1.0f,0.0f,0.0f);

            dotproduct = vecDotProduct(shipheading,othervector);
            if(dotproduct >= 1.0)
                dotproduct = 1.0f;
            else if(dotproduct <=-1.0f)
                dotproduct = -1.0f;

            movetoplace.w = RAD_TO_DEG(acos(dotproduct));

            vecSet(othervector,0.0f,1.0f,0.0f);
            dotproduct = vecDotProduct(shipheading,othervector);
            if(dotproduct < 0)
            {
                movetoplace.w = 360-movetoplace.w;
            }

            spHyperspaceSelectionIn(movetodo->selection,&movetoplace);
            movetodo->hyperspaceState=COM_HYPERSPACE_IN;
            movetodo->hyperSpaceingTime = universe.totaltimeelapsed;
        }
        break;
    case COM_HYPERSPACE_IN:
        //check if all hyperspaced in...
        donehsflag = TRUE;
        timefraction = universe.totaltimeelapsed - movetodo->hyperSpaceingTime;
        shipslice = (sdword) (timefraction/TW_PING_MAX_DISPERSAL)*movetodo->selection->numShips;
        if(shipslice-movetodo->selection->numShips == 1 ||
            shipslice > movetodo->selection->numShips )
        {
            shipslice = movetodo->selection->numShips;
        }
        else if(shipslice < 1)
        {
            shipslice = 1;
        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            if(movetodo->selection->ShipPtr[i]->playerowner != universe.curPlayerPtr)
            {
                //ship isn't a local players ship...
                if(battleCanChatterAtThisTime(BCE_COMM_F_HyperspaceDetected, movetodo->selection->ShipPtr[i]))
                {
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_HyperspaceDetected, SOUND_EVENT_DEFAULT, &movetodo->selection->ShipPtr[i]->posinfo.position);
                }
            }


            //do hs ping for ALL ships
            for(j=0;j<shipslice;j++)
            {
                if(!movetodo->selection->ShipPtr[j]->hyperspacePing)
                {
                    ping *newPing;
                    movetodo->selection->ShipPtr[j]->hyperspacePing=TRUE;
                    //create ping HERE!
                    newPing = pingCreate(NULL, (SpaceObj *)movetodo->selection->ShipPtr[j], (pingeval)hyperspaceInPingTimeOut, NULL, 0, (udword)movetodo->selection->ShipPtr[j]);
                    newPing->c = TW_HW_PING_COLOUR_IN;
                    newPing->size = TW_HW_PING_MAX_SIZE_IN;
                    newPing->minScreenSize = primScreenToGLScaleX(2);
                    newPing->minSize = TW_HW_PING_MIN_SIZE_IN;
                    newPing->pingDuration = 1.0f / TW_HW_PING_RATE_IN;
                    newPing->interPingPause = 0.25;
                    newPing->TOMask         = PTOM_Hyperspace;

                    ///////////////////////////////
                    //PUT SPEECH EVENTS HERE!
                    ///////////////////////////////

                }
            }
            if((movetodo->selection->ShipPtr[i]->shipSinglePlayerGameInfo->shipHyperspaceState != SHIPHYPERSPACE_NONE))
            {
                donehsflag = FALSE;
            }
            else
            {
                //individual ship is out of hyperspace...make it selectable!
                bitSet(movetodo->selection->ShipPtr[i]->flags,SOF_Selectable);
            }
        }
        if(donehsflag == FALSE)
        {
            return FALSE;
        }
        for(j=0;j<movetodo->selection->numShips;j++)
        {
            movetodo->selection->ShipPtr[j]->hyperspacePing=FALSE;
        }
        //all out..
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            //make selectable again!
            bitSet(movetodo->selection->ShipPtr[i]->flags,SOF_Selectable);
        }
        return(TRUE);
        break;
    }

    return(FALSE);
}

real32 hyperspaceCost(real32 distance,SelectCommand *selection)
{
    sdword i;
    Ship *ship;
    real32 tempcost;
    real32 cost=0.0f;
    for(i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if(distance < TW_HyperSpaceCostStruct[ship->shiptype].minDistance)
        {
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].min;
        }
        else
        {
            distance-= TW_HyperSpaceCostStruct[ship->shiptype].minDistance;
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].min +
                TW_HyperSpaceCostStruct[ship->shiptype].distanceSlope*distance;
        }
        if(tempcost > TW_HyperSpaceCostStruct[ship->shiptype].max)
        {
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].max;
        }
        cost+= tempcost;
    }
    return(cost);
}


/*-----------------------------------------------------------------------------
    Name        : processMoveToDo
    Description : processes the move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool processMoveToDo(CommandToDo *movetodo,bool passiveAttacked)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;
    Ship *ship;
    bool thisShipDone;
    bool allDone = TRUE;
    ShipStaticInfo *shipstatic;
    udword aiflags;
    sdword j;
    bool dontrotate;
    real32 MOVE_ARRIVE_TOLERANCE;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        shipstatic = (ShipStaticInfo *)ship->staticinfo;

        if(ship->shiptype == ResearchShip)
        {
            if(ship->flags & SOF_Slaveable)
            {
                dontrotate = TRUE;
            }
            else
            {
                dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
            }
        }
        else
        {
            if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
            {
                dontrotate = TRUE;
            }
            else
            {
                dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
            }
        }

        thisShipDone = FALSE;

        switch (ship->aistatecommand)
        {
        case 0:
                if ((ship->isDodging) || (universe.totaltimeelapsed < ship->DodgeTime))
                {
                    //ship is dodging...be a little more forgiving
                    MOVE_ARRIVE_TOLERANCE = 250.0f;
                }
                else
                {
                    MOVE_ARRIVE_TOLERANCE = 50.0f;
                }
                if (MoveReachedDestinationVariable(ship,&ship->moveTo,MOVE_ARRIVE_TOLERANCE))
                {
                    ship->aistatecommand = 1;
                    vecZeroVector(ship->moveTo);
                    vecZeroVector(ship->moveFrom);
                    if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                    {
                        for(j=1;j<movetodo->selection->numShips;j++)
                        {
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                        }
                    }

                    goto state1;
                }
                else
                {
                    aiflags = (dontrotate) ? (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured) : (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying|AISHIP_CarTurn);
                    if (aishipFlyToPointAvoidingObjs(ship,&ship->moveTo,aiflags,(ship->shiptype == HeadShotAsteroid) ? (-tacticsGetShipsMaxVelocity(ship)) : -20.0f) & AISHIP_FLY_OBJECT_IN_WAY)
                    {
                        ship->aistatecommand = 1;
                        vecZeroVector(ship->moveTo);
                        vecZeroVector(ship->moveFrom);
                        if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                        {
                            for(j=1;j<movetodo->selection->numShips;j++)
                            {
                                vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                                vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                            }
                        }

                        goto state1;
                    }
                }
                break;
state1:
            case 1:
                if ((dontrotate) ||
                    ((shipstatic->pitchdescend != 0.0f) && (ship->aidescend < 0.0f)))   // -ve aidescend indicates ship is primarily moving down
                {
                    thisShipDone = aitrackSteadyShipDriftOnly(ship);
                }
                else
                {
                    vector trackthisheading;
                    vecSet(trackthisheading,movetodo->move.heading.x,movetodo->move.heading.y,0.0f);
                    vecNormalize(&trackthisheading);
                    thisShipDone = (aitrackSteadyShipDriftOnly(ship) & aitrackHeadingWithBank(ship,&trackthisheading,FLYSHIP_HEADINGACCURACY,shipstatic->sinbank));
                }
                break;
        }

        allDone &= thisShipDone;
    }

    return allDone;
}

/*-----------------------------------------------------------------------------
    Name        : processMoveLeaderToDo
    Description : processes the move command, but only for the leader (this routine
                  is only used for ships in formation moving, and the other ships maintain
                  formation around the leader)
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool processMoveLeaderToDo(CommandToDo *movetodo,bool passiveAttacked)
{
    Ship *ship;
    SelectCommand *selection = movetodo->selection;
    ShipStaticInfo *shipstatic;
    bool dontrotate;
    udword aiflags;
    sdword j;
    udword moveresult;
    real32 MOVE_ARRIVE_TOLERANCE;

    dbgAssert(selection->numShips >= 1);

    ship = selection->ShipPtr[0];
    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    if(ship->shiptype == ResearchShip)
    {
        if(ship->flags & SOF_Slaveable)
        {
            dontrotate = TRUE;
        }
        else
        {
            dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
        }
    }
    else
    {
        dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
    }

    switch (ship->aistatecommand)
    {
        case 0:
            if ((ship->isDodging) || (universe.totaltimeelapsed < ship->DodgeTime))
            {
                //ship is dodging...be a little more forgiving
                MOVE_ARRIVE_TOLERANCE = 250.0f;
            }
            else
            {
                MOVE_ARRIVE_TOLERANCE = 50.0f;
            }
            if (MoveReachedDestinationVariable(ship,&movetodo->move.destination,MOVE_ARRIVE_TOLERANCE))
            {
                ship->aistatecommand = 1;
                vecZeroVector(ship->moveTo);
                vecZeroVector(ship->moveFrom);

                if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                {
                    for(j=1;j<movetodo->selection->numShips;j++)
                    {
                        vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                        vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                    }
                }

                goto state1;
            }
            else
            {
                aiflags = (dontrotate) ? (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured) : (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying|AISHIP_CarTurn);
                moveresult = aishipFlyToPointAvoidingObjs(ship,&movetodo->move.destination,aiflags,movetodo->formation.travelvel);
                if (moveresult & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    vecZeroVector(ship->moveTo);
                    vecZeroVector(ship->moveFrom);
                    ship->aistatecommand = 1;
                    if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                    {
                        for(j=1;j<movetodo->selection->numShips;j++)
                        {
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                        }
                    }

                    goto state1;
                }
            }
            break;
state1:
        case 1:
            if ((dontrotate) ||
                ((shipstatic->pitchdescend != 0.0f) && (ship->aidescend < 0.0f)))   // -ve aidescend indicates ship is primarily moving down
            {
                return aitrackSteadyShipDriftOnly(ship);
            }
            else
            {
                vector trackthisheading;
                vecSet(trackthisheading,movetodo->move.heading.x,movetodo->move.heading.y,0.0f);
                vecNormalize(&trackthisheading);
                return (aitrackSteadyShipDriftOnly(ship) & aitrackHeadingWithBank(ship,&trackthisheading,FLYSHIP_HEADINGACCURACY,shipstatic->sinbank));
            }
    }
    return FALSE;
}

void SalCapCancelOrder(Ship *ship);

void RemoveShipFromSpecial(Ship *shiptoremove)
{
    if (shiptoremove->specialFlags & SPECIAL_IsASalvager)
    {
        SalCapCancelOrder(shiptoremove);
        SalCapOrderChangedCleanUp(shiptoremove);
    }
    else if (shiptoremove->shiptype == MinelayerCorvette)
    {
        MinelayerCorvetteOrderChangedCleanUp(shiptoremove);
    }
    else if(shiptoremove->shiptype == RepairCorvette)
    {
        RepairCorvetteOrderChanged(shiptoremove);
    }
    else if(shiptoremove->shiptype == HeavyCorvette)
    {
        heavyCorvetteOrderChanged(shiptoremove);
    }
    else if(shiptoremove->shiptype == AdvanceSupportFrigate)
    {
        if(shiptoremove->rceffect != NULL)
        {
            //need to turn off effect
            stopRepairEffect(shiptoremove);
        }
    }
}

void SpecialCleanup(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;

    //for all special ships using the repair effect
    for (i=0;i<numShips;i++)
    {
        if(selection->ShipPtr[i]->rceffect != NULL)
        {
            if(selection->ShipPtr[i]->staticinfo->repairBeamCapable)
            {
                //ship must be using the repair effect
                stopRepairEffect(selection->ShipPtr[i]);
            }
        }
        RemoveShipFromSpecial(selection->ShipPtr[i]);
    }

    if (command->specialtargets)
    {
        memFree(command->specialtargets);
        command->specialtargets = NULL;
    }
}

void RemoveShipFromResourceCollecting(Ship *shiptoremove)
{
    if (shiptoremove->rceffect != NULL)
    {
        TurnHarvestEffectOff(shiptoremove);
    }
    shiptoremove->rcstate1 = 0;
    shiptoremove->rcstate2 = 0;
}

void CollectResourceCleanup(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromResourceCollecting(selection->ShipPtr[i]);
    }
}

void FreeCommandToDoContents(CommandToDo *command)
{
    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        memFree(command->protect);
        bitClear(command->ordertype.attributes,COMMAND_IS_PROTECTING);
    }

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        FormationCleanup(command);
        bitClear(command->ordertype.attributes,COMMAND_IS_FORMATION);
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        AttackCleanup(command);
        dbgAssert(command->attack);
        memFree(command->attack);
        bitClear(command->ordertype.attributes,COMMAND_IS_PASSIVEATTACKING);
    }

    //report dead command to tactics layer
    tacticsAttackCommandVoided(command);

    RemoveShipsFromCommand(command);

    switch (command->ordertype.order)
    {
        case COMMAND_ATTACK:
            AttackCleanup(command);
            dbgAssert(command->attack);
            memFree(command->attack);
            break;

        case COMMAND_MOVE:
            MoveCleanup(command);
            break;
        case COMMAND_MP_HYPERSPACEING:
            MpHyperspaceCleanup(command);
            break;

        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_BUILDINGSHIP:
            break;

        case COMMAND_COLLECTRESOURCE:
            CollectResourceCleanup(command);
            break;

        case COMMAND_MILITARYPARADE:
            dbgAssert(command->militaryParade);
            FreeMilitaryParadeContents(command->militaryParade);
            memFree(command->militaryParade);
            break;

        case COMMAND_LAUNCHSHIP:
            LaunchCleanup(command);
            break;

        case COMMAND_DOCK:
            DockCleanup(command);
            break;

        case COMMAND_SPECIAL:
            SpecialCleanup(command);
            break;

        default:
            dbgAssert(FALSE);   // should never get here
            break;
    }

    memFree(command->selection);
}

void FreeLastOrder(CommandToDo *command)
{
    sdword dontClearHoldingPattern = FALSE;
    //report dead command to tactics layer
    tacticsAttackCommandVoided(command);

    // deliberately do not call
    // RemoveShipsFromCommand(command)
    // because last order is being freed in order to be changed, the actual command is not freed.

    switch (command->ordertype.order)
    {
        case COMMAND_ATTACK:
            AttackCleanup(command);
            dbgAssert(command->attack);
            memFree(command->attack);
            command->attack = NULL;
            dontClearHoldingPattern = TRUE;
            break;

        case COMMAND_MOVE:
            MoveCleanup(command);
            break;
        case COMMAND_MP_HYPERSPACEING:
            MpHyperspaceCleanup(command);
            break;

        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_BUILDINGSHIP:
            break;

        case COMMAND_COLLECTRESOURCE:
            CollectResourceCleanup(command);
            break;

        case COMMAND_MILITARYPARADE:
            dbgAssert(command->militaryParade);
            FreeMilitaryParadeContents(command->militaryParade);
            memFree(command->militaryParade);
            command->militaryParade = NULL;
            break;

        case COMMAND_LAUNCHSHIP:
            LaunchCleanup(command);
            break;

        case COMMAND_DOCK:
            DockCleanup(command);
            break;

        case COMMAND_SPECIAL:
            SpecialCleanup(command);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
    if(!dontClearHoldingPattern)
    {
        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
    }
}

void ClearPassiveAttacking(CommandToDo *command)
{
    bitClear(command->ordertype.attributes,COMMAND_IS_PASSIVEATTACKING);
    AttackCleanup(command);
    dbgAssert(command->attack);
    memFree(command->attack);
    command->attack = NULL;

/*
    Locked formation cleanup not needed because a check is done in AttackCleanup to
    see if a formation is locked and unlocks it if need be...
*/

/*
    if(command->ordertype.order == COMMAND_NULL)
    {
        //command is a NULL command
        if(command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            //command is a NULL command and in FORMATION
            //There for the formation had to be locked
            dbgAssert(command->formation.formationLocked);

            //unlock the formation
            unlockFormation(command);
        }
    }
*/
}

void ClearProtecting(CommandToDo *command)
{
    bitClear(command->ordertype.attributes,COMMAND_IS_PROTECTING);
    dbgAssert(command->protect);
    memFree(command->protect);
    command->protect = NULL;

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
//        command->formation.formAroundProtectedShip = FALSE;
        if ((command->formation.formationtype == SPHERE_FORMATION) && (command->selection->numShips > 0))
        {
            // sphere formation was surrounding protected ship, so let's undo this
            formationContentHasChanged(command);
        }
    }
}

void RemoveShipsFromDoingStuff(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword i;
    ShipPtr shiptoremove;
    Node *todonode;
    CommandToDo *todo;
    SelectCommand *todoselection;

    for (i=0;i<selectcom->numShips;i++)
    {
        shiptoremove = selectcom->ShipPtr[i];
        if (shiptoremove->specialFlags & SPECIAL_rowGettingOutOfWay)
        {
            rowRemoveShipFromGettingOutOfWay(shiptoremove);
        }
    }

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);
        todoselection = todo->selection;

        for (i=0;i<selectcom->numShips;i++)
        {
            shiptoremove = selectcom->ShipPtr[i];

            if (todo->ordertype.attributes & COMMAND_IS_FORMATION)
            {
                sdword removedShipFromFormation;
                if ((removedShipFromFormation = formationRemoveShipFromSelection(todo,shiptoremove)) >= 0)
                {
                    RemoveShipFromCommand(shiptoremove);
                    RemoveShipFromFormation(shiptoremove);

                    if ((todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) || (todo->ordertype.order == COMMAND_ATTACK))
                    {
                        RemoveShipFromAttacking(shiptoremove);
                        RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                    }

                    if (todo->ordertype.order == COMMAND_DOCK)
                    {
                        RemoveShipFromDocking(shiptoremove);
                    }

                    if (todo->ordertype.order == COMMAND_MOVE)
                    {
                        RemoveShipFromMoving(shiptoremove);
                    }

                    if(todo->ordertype.order == COMMAND_SPECIAL)
                    {
                        RemoveShipFromSpecial(shiptoremove);
                    }

                    if(todo->ordertype.order == COMMAND_MP_HYPERSPACEING)
                    {
                        removeShipFromMpHyperspaceing(shiptoremove);
                    }

                    if (todoselection->numShips < ABSOLUTE_MIN_SHIPS_IN_FORMATION)
                    {
                        Node *nextnode = todonode->next;
                        FreeCommandToDoContents(todo);
                        listDeleteNode(todonode);
                        todonode = nextnode;
                        goto readyfornextnode;
                    }
                    else
                    {
                        setFormationTravelVelocity(todo);
                        //formationArrangeOptimum(todo);
                        if (todo->formation.formationtype == WALL_FORMATION)    // leave holes in WALL and SPHERE formations
                        {
                            if ((removedShipFromFormation == 0) && (shiptoremove->shiptype == TargetDrone))     // always preserve wall formation order if consists of Target Drones
                            {
                                todo->formation.formationtype = CUSTOM_FORMATION;
                                formationContentHasChanged(todo);   // leave holes in wall by making it custom
                            }
                            else
                            {
                                ;       // deliberately do not call formationContentHasChanged for sphere formation
                            }
                        }
                        else if (todo->formation.formationtype == SPHERE_FORMATION)
                        {
                            ;       // deliberately do not call formationContentHasChanged for sphere formation
                        }
                        else if (todo->formation.formationtype == CUSTOM_FORMATION)
                        {
                            if (removedShipFromFormation == 0)
                            {
                                formationContentHasChanged(todo);       // only update custom formation if leader died
                            }
                        }
                        else
                        {
                            formationContentHasChanged(todo);
                        }
                    }
                }
            }
            else
            {
                switch (todo->ordertype.order)
                {
                    case COMMAND_ATTACK:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            RemoveShipFromAttacking(shiptoremove);
                            RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_COLLECTRESOURCE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            RemoveShipFromResourceCollecting(shiptoremove);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_MOVE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromMoving(shiptoremove);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_NULL:
                    case COMMAND_HALT:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_SPECIAL:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            dbgAssert((todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == 0);

                            RemoveShipFromCommand(shiptoremove);
                            RemoveShipFromSpecial(shiptoremove);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_MILITARYPARADE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromMilitaryParade(shiptoremove,todo->militaryParade);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_LAUNCHSHIP:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromLaunching(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_DOCK:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromDocking(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_BUILDINGSHIP:
                        break;

                    case COMMAND_MP_HYPERSPACEING:
#ifdef HW_DEBUG
                        dbgMessage("\nHyperspaceing ship removed from doing stuff!");
#endif
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            removeShipFromMpHyperspaceing(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }

                        break;
                    default:
                        dbgAssert(FALSE);
                        break;
                }

            }
        }

        todonode = todonode->next;
readyfornextnode:
        ;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clDerelictDied
    Description : Removes derelict from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clDerelictDied(CommandLayer *comlayer,DerelictPtr derelict)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)derelict))
            {
                RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)derelict,todo);
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
                break;
            case COMMAND_SPECIAL:
                if (todo->specialtargets)
                {
                    clRemoveShipFromSelection(todo->specialtargets,(SpaceObjRotImpTarg *)derelict);
                }
                break;
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_MILITARYPARADE:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)derelict))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)derelict,todo);
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clResourceDied
    Description : Removes resource from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clResourceDied(CommandLayer *comlayer,ResourcePtr resource)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)resource))
            {
                RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)resource,todo);
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
            case COMMAND_SPECIAL:
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_MILITARYPARADE:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)resource))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)resource,todo);
                }
                break;

            case COMMAND_COLLECTRESOURCE:
                if (resource == todo->collect.resource)
                {
                    TurnOffAnyResourcingEffects(todo);
                    todo->collect.resource = NULL;
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clMissileDied
    Description : Removes missile from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMissileDied(CommandLayer *comlayer,MissilePtr missile)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
            case COMMAND_SPECIAL:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_MILITARYPARADE:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)missile))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)missile,todo);
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : shipHasJustCloaked
    Description : cleans up te games such that the ship that just cloaked
                  isn';t referenced by ships n' such that it shouldn't be.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shipHasJustCloaked(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_CLOAKED);
    if(ship->playerowner != universe.curPlayerPtr &&
       !proximityCanPlayerSeeShip(universe.curPlayerPtr,ship))
    {           //if ship isn't players...remove it from camera stack
        ccRemoveShip(&universe.mainCameraCommand,ship);
    }
}

void shipHasJustDisappeared(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_DISAPPEARED);
}

/*-----------------------------------------------------------------------------
    Name        : shipHasJustBeenDisabled
    Description : cleans up the game such that the ship that has just been disabled isn't
                  being attacked by ships, etc.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shipHasJustBeenDisabled(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_DISABLED);
    ccRemoveShip(&universe.mainCameraCommand,ship);
    switch (ship->shiptype)
    {
        case DDDFrigate:
            DDDFrigateJustDisabled(ship);
            break;

        case CloakGenerator:
            CloakGeneratorJustDisabled(ship);
            break;

        case GravWellGenerator:
            GravWellGeneratorJustDisabled(ship);
            break;
    }
    //if ship doesn't have the animation it won't matter
    madLinkInCloseGunsShip(ship);

    if(ship->shiptype != SensorArray)
    {
        madLinkInPreDockingShip(ship);
    }

    if(ship->shiptype != SensorArray)
    {
        //we don't want to do this for the sensor array
        madLinkInCloseSpecialShip(ship);
    }
}

// removeShipsFromDockingWithThisShip
//
// If there are ships docking with this ship, they will be cancelled
void removeShipsFromDockingWithThisShip(Ship *shiptoremove)
{
    Node *todonode;
    CommandToDo *todo;

    if(!shiptoremove->staticinfo->canReceiveShips)
    {
        //ship can't receive ships therefore don't do check
        return;
    }

    todonode = universe.mainCommandLayer.todolist.head;
    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        switch (todo->ordertype.order)
        {
            case COMMAND_DOCK:
                {
                    SelectCommand *selection = todo->selection;
                    sdword numShips = selection->numShips;
                    Ship *ship;
                    sdword i;
                    for (i=0;i<numShips;i++)
                    {
                        ship = selection->ShipPtr[i];
                        if ((ship->dockvars.dockship == shiptoremove) || (ship->dockvars.busyingThisShip == shiptoremove))
                        {
                            RemoveShipFromDocking(ship);
                        }
                    }
                }
                break;
        }
        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromBeingTargeted
    Description : removes shiptoremove from command layer, such that it is
                  not the target of any other ship
    Inputs      : shiptoremove, removeFlag which can be:
                  REMOVE_PROTECT - should remove ship from being protected
                  REMOVE_CLOAKED - ship was just cloaked.  If this flag is true, when the ship is removed,
                  it is only removed if the reference that is being considered CAN'T see the ship
                  REMOVE_DISABLED - ship was SOF_Disable'd (e.g. Salvaged)
                  REMOVE_DISAPPEARED - ship was hidden (SOF_Hide'd)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromBeingTargeted(CommandLayer *comlayer,ShipPtr shiptoremove,udword removeFlag)
{
    Node *todonode;
    Node *nextnode;
    CommandToDo *todo;
    udword REMOVE_PROTECT_FLAG = removeFlag & REMOVE_PROTECT;
    udword CLOAKED_REMOVAL_FLAG = removeFlag & REMOVE_CLOAKED;
    udword SHIP_JUST_DISABLED_FLAG = removeFlag & REMOVE_DISABLED;
    udword SHIP_DISAPPEARED_FLAG = removeFlag & REMOVE_DISAPPEARED;
    udword SHIP_HYPERSPACEING_FLAG = removeFlag & REMOVE_HYPERSPACING;
    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        //if ship is hyperspaceing out...via mp hp move
        if(SHIP_HYPERSPACEING_FLAG)
        {
            //prevent ships from docking with it!

            if(todo->ordertype.order == COMMAND_DOCK)
            {
                SelectCommand *selection = todo->selection;
                sdword numShips = selection->numShips;
                bool removefromdocking;
                Ship *ship;
                sdword i;
                for (i=0;i<numShips;i++)
                {
                    ship = selection->ShipPtr[i];
                    removefromdocking = FALSE;
                    if (ship->dockvars.dockship == shiptoremove)
                    {
                        removefromdocking = TRUE;
                        if(ship->dockvars.dockship->shiptype == AdvanceSupportFrigate)
                        {
                            if(ship->dockvars.dockstate2 != ASF_WAITLATCHPOINT)
                            {
                                removefromdocking = FALSE;
                            }
                        }
                    }
                    else if(ship->dockvars.busyingThisShip == shiptoremove)
                    {
                        removefromdocking = TRUE;
                        if(ship->dockvars.busyingThisShip->shiptype == AdvanceSupportFrigate)
                        {
                            if(ship->dockvars.dockstate2 != ASF_WAITLATCHPOINT)
                            {
                                removefromdocking = FALSE;
                            }
                        }
                    }
                    if (removefromdocking)
                        RemoveShipFromDocking(ship);
                }
            }
            else if(todo->ordertype.order == COMMAND_SPECIAL)
            {
                sdword i;
                for(i=0;i<todo->selection->numShips;i++)
                {
                    if(todo->selection->ShipPtr[i]->shiptype == SalCapCorvette)
                    {
                        if((((SalCapCorvetteSpec *)(todo->selection->ShipPtr[i])->ShipSpecifics))->target != NULL &&
                           ((Ship *)(((SalCapCorvetteSpec *)(todo->selection->ShipPtr[i])->ShipSpecifics))->target) == shiptoremove)
                        {
                            if(((ShipStaticInfo *)todo->selection->ShipPtr[i]->staticinfo)->custshipheader.CustShipRemoveShipReferences != NULL)
                            {
                                ((ShipStaticInfo *)todo->selection->ShipPtr[i]->staticinfo)->custshipheader.CustShipRemoveShipReferences(todo->selection->ShipPtr[i], shiptoremove);
                            }
                        }
                    }
                }
                if ( SHIP_DISAPPEARED_FLAG ||
                (!(CLOAKED_REMOVAL_FLAG &&
                proximityCanPlayerSeeShip(todo->selection->ShipPtr[0]->playerowner,shiptoremove))) )
            {
                if (!SHIP_JUST_DISABLED_FLAG)
                {
                    if (todo->specialtargets)
                    {
                        clRemoveShipFromSelection(todo->specialtargets,shiptoremove);
                    }
                }
            }

            }
            todonode = todonode->next;
            goto readyfornextnode;
        }

        if(REMOVE_PROTECT_FLAG)
        {
            if (todo->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                if ( SHIP_DISAPPEARED_FLAG ||
                   (!(CLOAKED_REMOVAL_FLAG &&
                    proximityCanPlayerSeeShip(todo->selection->ShipPtr[0]->playerowner,shiptoremove))) )
                {
                    clRemoveShipFromSelection((SelectCommand *)todo->protect,shiptoremove);

                    if (todo->protect->numShips == 0)
                    {
                        ClearProtecting(todo);

                        if (todo->ordertype.attributes == 0)
                        {
                            if (todo->ordertype.order == COMMAND_NULL)
                            {
                                nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                    }
                }
            }
        }
        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if ( SHIP_DISAPPEARED_FLAG ||
                (!(CLOAKED_REMOVAL_FLAG &&
                    proximityCanPlayerSeeShip(todo->selection->ShipPtr[0]->playerowner,shiptoremove))) )
            {
                if (clRemoveShipFromSelection((SelectCommand *)todo->attack,shiptoremove))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)shiptoremove,todo);
                }
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_ATTACK:
                if ( SHIP_DISAPPEARED_FLAG ||
                    (!(CLOAKED_REMOVAL_FLAG &&
                    proximityCanPlayerSeeShip(todo->selection->ShipPtr[0]->playerowner,shiptoremove))) )
                {
                    if (clRemoveShipFromSelection((SelectCommand *)todo->attack,shiptoremove))
                    {
                        RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)shiptoremove,todo);
                    }
                }
                break;

            case COMMAND_SPECIAL:
                if ( SHIP_DISAPPEARED_FLAG ||
                    (!(CLOAKED_REMOVAL_FLAG &&
                    proximityCanPlayerSeeShip(todo->selection->ShipPtr[0]->playerowner,shiptoremove))) )
                {
                    if (!SHIP_JUST_DISABLED_FLAG)
                    {
                        if (todo->specialtargets)
                        {
                            sdword h;
                            clRemoveShipFromSelection(todo->specialtargets,shiptoremove);
                            for(h=0;h<todo->selection->numShips;h++)
                            {
                                if(todo->selection->ShipPtr[h]->shiptype == SalCapCorvette ||
                                    todo->selection->ShipPtr[h]->shiptype == JunkYardDawg)
                                {
                                    salCapCleanUpCloakingTarget(todo->selection->ShipPtr[h],shiptoremove);
                                }
                            }
                        }
                    }
                }
                break;

            case COMMAND_DOCK:
                if (!SHIP_DISAPPEARED_FLAG)
                {
                    SelectCommand *selection = todo->selection;
                    sdword numShips = selection->numShips;
                    Ship *ship;
                    sdword i;
                    for (i=0;i<numShips;i++)
                    {
                        ship = selection->ShipPtr[i];
                        if ((ship->dockvars.dockship == shiptoremove) || (ship->dockvars.busyingThisShip == shiptoremove))
                        {
                            RemoveShipFromDocking(ship);
                        }
                    }
                }
                break;

            case COMMAND_LAUNCHSHIP:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->launchship.receiverShip)
                {
                    Ship *ship = todo->selection->ShipPtr[0];

                    dbgAssert(todo->selection->numShips == 1);
                    RemoveShipFromLaunching(ship);

                    nextnode = todonode->next;
                    FreeCommandToDoContents(todo);
                    listDeleteNode(todonode);
                    todonode = nextnode;
                    goto readyfornextnode;
                }
                break;

            case COMMAND_BUILDINGSHIP:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->buildingship.creator)
                {
                    nextnode = todonode->next;
                    FreeCommandToDoContents(todo);
                    listDeleteNode(todonode);
                    todonode = nextnode;
                    goto readyfornextnode;
                }
                break;

            case COMMAND_MILITARYPARADE:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->militaryParade->aroundShip)
                {
                    ChangeOrderToHalt(todo);
                }
                break;

            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_HALT:
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        todonode = todonode->next;
readyfornextnode:
        ;
    }
}

/*=============================================================================
    Public Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : clInit
    Description : Initializes the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clInit(CommandLayer *comlayer)
{
    listInit(&comlayer->todolist);
}

/*-----------------------------------------------------------------------------
    Name        : clClose
    Description : Closes the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clClose(CommandLayer *comlayer)
{
    Node *nextnode;
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    // free contents of each node of list
    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;

        if (singlePlayerGameLoadNewLevelFlag)
        {
            // preserve parade formation throughout hyperspace
            if (command->ordertype.order == COMMAND_MILITARYPARADE)
            {
                Ship *aroundship = command->militaryParade->aroundShip;
                if ((aroundship) && (aroundship->playerowner == universe.curPlayerPtr))
                {
                    goto dontfreeit;
                }
            }
        }

        FreeCommandToDoContents(command);
        listDeleteNode(curnode);

dontfreeit:
        curnode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clCancelAllLaunchOrdersFromPlayer
    Description : Cancels all launch orders so ships in process of launching
                  will be available to dock and to fix a fatal bug with the single player game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void clCancelAllLaunchOrdersFromPlayer(struct Player *player)
{
    Node *nextnode;
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    sdword i;

    dbgAssert(singlePlayerGame);

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;


        if (command->ordertype.order == COMMAND_LAUNCHSHIP)
        {
            for(i=0;i<command->selection->numShips;i++)
            {
                if(command->selection->ShipPtr[i]->playerowner == player)
                {
                    if(!ShipCanHyperspace(command->selection->ShipPtr[i]) )
                    {
                        RemoveShipFromLaunching(command->selection->ShipPtr[i]);
                        if(clRemoveShipFromSelection(command->selection,command->selection->ShipPtr[i]))
                        {
                            if(command->selection->numShips == 0)
                            {
                                FreeCommandToDoContents(command);
                                listDeleteNode(curnode);
                                break;
                            }
                        }
                    }
                }
            }
        }

        curnode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clReset
    Description : Resets the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clReset(CommandLayer *comlayer)
{
    clClose(comlayer);
}

/*-----------------------------------------------------------------------------
    Name        : IsSelectionAlreadyDoingSomething
    Description : sees if the selection specified is alread in the command layer
    Inputs      :
    Outputs     :
    Return      : CommandToDo pointer if the selection specified is already
                  present, NULL otherwise
----------------------------------------------------------------------------*/
CommandToDo *IsSelectionAlreadyDoingSomething(CommandLayer *comlayer,SelectCommand *selectcom)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (SelectionsAreEquivalent(selectcom,command->selection))
        {
            return command;
        }

        curnode = curnode->next;
    }

    return NULL;
}

//returns TRUE if a strike craft is in the selection <corvette or fighter>
bool selectionHasStrikeCraft(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette)
        {
            return TRUE;
        }
    }
    return FALSE;
}

bool isSelectionExclusivlyStrikeCraft(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(!(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette))
        {
            return FALSE;
        }
    }
    return TRUE;
}

bool isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(!(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette ||
             selection->ShipPtr[i]->staticinfo->cantMoveAndAttack))
        {
            return FALSE;
        }
    }
    return TRUE;
}

bool selectionHasSwarmers(SelectCommand *selection)
{
    sdword i;
    Ship *ship;

    for(i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if ((ship->shiptype == P2Swarmer) || (ship->shiptype == P2AdvanceSwarmer))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
;=============================================================================
; Solution for ships moving to single point nastiness:
;=============================================================================

; If it looks like a bunch of ships in different parts of the world are moving to a "central location" then
; their relative positions will be scaled near to a single point using the factor SHRINKMIN.
; If it looks like a bunch of ships are moving to a relatively far location, they will preserve their
; relative positions (SHRINKMAX, which is typically 1.0)

; Here is the technical description:

; consider a distance d, which is the average position the ships travel.
; consider a radius r, within which all ships ordered to move are within

; Take the ratio d/r.
; if d/r is less than DISTMINRATIO, the relative scaling factor will be SHRINKMIN
; if d/r is greater than DISTMAXRATIO, the relative scaling factor will be SHRINKMAX
; in between,
; scales from a     d/r ratio of DISTMINRATIO      to      a scale factor of SHRINKMIN
;                                DISTMAXRATIO                                SHRINKMAX
*/

typedef struct
{
    vector fromtoship;
    vector totoship;
} ShipMoveCalcs;

#define NUM_STACK_MOVECALCS 100

/*-----------------------------------------------------------------------------
    Name        : CalculateMoveToPoints
    Description :
    Inputs      : selection, from, to
    Outputs     : moveTo vectors are filled in
    Return      :
----------------------------------------------------------------------------*/
void CalculateMoveToPoints(SelectCommand *selection,vector from,vector to)
{
    sdword numShips = selection->numShips;
    ShipMoveCalcs stackMoveCalcs[NUM_STACK_MOVECALCS];
    bool moveCalcsAllocated = FALSE;
    ShipMoveCalcs *movecalcs;
    sdword i;
    vector d,r;
    real32 dsqr,rsqr,dmag;
    real32 maxrsqr,maxrmag;
    real32 scale,doverr;

    dbgAssert(numShips > 0);

    if (numShips == 1)
    {
        selection->ShipPtr[0]->moveTo = to;
        return;
    }

    vecSub(d,to,from);
    dsqr = vecMagnitudeSquared(d);
    dmag = fsqrt(dsqr);

    if (numShips > NUM_STACK_MOVECALCS)
    {
        moveCalcsAllocated = TRUE;
        movecalcs = memAlloc(sizeof(ShipMoveCalcs)*numShips,"movecalcs",0);
    }
    else
    {
        movecalcs = stackMoveCalcs;
    }

    maxrsqr = -1.0f;

    for (i=0;i<numShips;i++)
    {
        vecSub(r,selection->ShipPtr[i]->posinfo.position,from);
        movecalcs[i].fromtoship = r;
        movecalcs[i].totoship = r;

        rsqr = vecMagnitudeSquared(r);
        if (rsqr > maxrsqr)
        {
            maxrsqr = rsqr;
        }
    }

    dbgAssert(maxrsqr >= 0.0f);
    maxrmag = fsqrt(maxrsqr);

    // scales from a d/r ratio of DISTMINRATIO      to      SHRINKMIN
    //                            DISTMAXRATIO              SHRINKMAX

    if (dmag < maxrmag)      // assumes DISTMINRATIO = 1.0
    {
        // shrink to a point:
        scale = SHRINKMIN;
    }
    else
    {
        // preserve relative movement
        doverr = dmag / maxrmag;

        if (doverr >= DISTMAXRATIO)
        {
            // skip scaling
            scale = SHRINKMAX;
        }
        else
        {
            // somewhere between moving to a point and relative movement

            scale = ((doverr - DISTMINRATIO) * (SHRINKMAX - SHRINKMIN) / (DISTMAXRATIO - DISTMINRATIO)) + SHRINKMIN;
        }
    }

    if (scale != 1.0f)
    for (i=0;i<numShips;i++)
    {
        vecMultiplyByScalar(movecalcs[i].totoship,scale);
    }

    for (i=0;i<numShips;i++)
    {
        vecAdd(selection->ShipPtr[i]->moveTo,to,movecalcs[i].totoship);
    }

    if (moveCalcsAllocated)
    {
        memFree(movecalcs);
    }
}
bool selectionHasOtherForceNoAttackAndMoveShips(SelectCommand *selection)
{
    sdword i;
    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->staticinfo->cantMoveAndAttack)
        {
            return TRUE;
        }
    }
    return FALSE;
}
void ChangeOrderToMove(CommandToDo *alreadycommand,vector from,vector to)
{
    SelectCommand *selectcom = alreadycommand->selection;
    sdword i,j;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if(alreadycommand->ordertype.order == COMMAND_ATTACK)
    {
        //selection is actually attacking...so this should be a move attack
        if(selectionHasStrikeCraft(alreadycommand->selection) ||
           selectionHasOtherForceNoAttackAndMoveShips(alreadycommand->selection))
        {
            //filter strike craft...give strike craft a move order,
            //and give give capital ships a majik command
            if(!isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(alreadycommand->selection))
            {
                //some capitals some strike craft
                MaxSelection tempSelection;
                tempSelection.numShips = 0;
                j = 0;

                dbgAssert(alreadycommand->selection->numShips < COMMAND_MAX_SHIPS);

                for(i=0;i<alreadycommand->selection->numShips;i++)
                {
                    if(alreadycommand->selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
                       alreadycommand->selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette ||
                       alreadycommand->selection->ShipPtr[i]->staticinfo->cantMoveAndAttack)
                    {
                        tempSelection.numShips++;
                        tempSelection.ShipPtr[j] = alreadycommand->selection->ShipPtr[i];
                        j++;
                    }
                }
                RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&tempSelection);
                clMoveThese(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,from,to);
                if(alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
                {
                    clFormation(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,alreadycommand->formation.formationtype);
                }
                goto StillDoAttackMoveWithCapitals;
            }
        }
        else
        {
StillDoAttackMoveWithCapitals:
            //just capital ships in selection
            if(!bitTest(alreadycommand->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL))
            {
                alreadycommand->ordertype.attributes |= COMMAND_IS_ATTACKINGANDMOVING;
                alreadycommand->move.destination = to;
                vecSub(alreadycommand->move.heading,to,from);
                vecNormalize(&alreadycommand->move.heading);

                InitShipsForAI(selectcom,TRUE);

                CalculateMoveToPoints(selectcom,from,to);

                for (i=0;i<selectcom->numShips;i++)
                {
                    //selectcom->ShipPtr[i]->moveTo = to;
                    selectcom->ShipPtr[i]->moveFrom = selectcom->ShipPtr[i]->posinfo.position;
                    //soundEventStartEngine(selectcom->ShipPtr[i]);
                }
                PrepareShipsForCommand(alreadycommand,TRUE);
                return;
            }
            bitClear(alreadycommand->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
        }
    }

    if (alreadycommand->ordertype.order == COMMAND_MOVE)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging movement direction");
#endif

    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to move");
#endif
        FreeLastOrder(alreadycommand);

        alreadycommand->ordertype.order = COMMAND_MOVE;
    }

    alreadycommand->move.destination = to;
    vecSub(alreadycommand->move.heading,to,from);
    vecNormalize(&alreadycommand->move.heading);

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        InitShipAI(selectcom->ShipPtr[0],TRUE);
    }
    else
    {
        InitShipsForAI(selectcom,TRUE);
    }

    CalculateMoveToPoints(selectcom,from,to);

    for (i=0;i<selectcom->numShips;i++)
    {
        //selectcom->ShipPtr[i]->moveTo   = to;
        selectcom->ShipPtr[i]->moveFrom = selectcom->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }

    PrepareShipsForCommand(alreadycommand,TRUE);
    if(selectcom->numShips == 1)
    {
        if(selectcom->ShipPtr[0]->shiptype == Probe)
        {
            ((ProbeSpec *) selectcom->ShipPtr[0]->ShipSpecifics)->HaveMoved = TRUE;
            ((ProbeSpec *) selectcom->ShipPtr[0]->ShipSpecifics)->moveTime = universe.totaltimeelapsed;
        }
    }

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clMoveThese
    Description : Command to move ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMoveThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;
    sdword i;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships To Move");
#endif
        return;
    }
    if(numShips == 1)
    {
        if(selectcom->ShipPtr[0]->shiptype == Probe)
        {
            //need probe 'solution' again
            //so set specific probe variables
            ((ProbeSpec *) selectcom->ShipPtr[0]->ShipSpecifics)->HaveMoved = TRUE;
            ((ProbeSpec *) selectcom->ShipPtr[0]->ShipSpecifics)->moveTime = universe.totaltimeelapsed;
        }
    }
    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        ChangeOrderToMove(alreadycommand,from,to);

        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to move");
#endif


    {
        sdword sizeofcopycom;
        SelectCommand *copycom;
        CommandToDo *currentShipCommand;
        sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
        copycom = memAlloc(sizeofcopycom,"cc1(copycom)", Pyrophoric);
        memcpy(copycom,selectcom,sizeofcopycom);


        for(i=0;i<copycom->numShips;i++)
        {

            currentShipCommand = getShipAndItsCommand(&universe.mainCommandLayer,copycom->ShipPtr[i]);
            if(currentShipCommand != NULL)
            {
                //ship is doing something
                if(currentShipCommand->ordertype.order == COMMAND_ATTACK)
                {
                    //ship is attacking
                    MaxSelection tempSelection;
                    MaxAnySelection targetSelection;
                    TypeOfFormation formation;
                    bool needFormation;
                    sdword j;

                    tempSelection.numShips = 0;


                    for(j=0;j<currentShipCommand->selection->numShips;j++)
                    {
                        if(ShipInSelection(copycom,currentShipCommand->selection->ShipPtr[j]))
                        {
                            tempSelection.ShipPtr[tempSelection.numShips] = currentShipCommand->selection->ShipPtr[j];
                            tempSelection.numShips++;
                            clRemoveShipFromSelection(copycom,currentShipCommand->selection->ShipPtr[j]);
                        }
                    }
                    dbgAssert(tempSelection.numShips > 0);
                    for(j=0;j<currentShipCommand->attack->numTargets;j++)
                    {
                        targetSelection.TargetPtr[j] = currentShipCommand->attack->TargetPtr[j];
                    }
                    targetSelection.numTargets = currentShipCommand->attack->numTargets;

                    if(currentShipCommand->ordertype.attributes & COMMAND_IS_FORMATION)
                    {
                        formation = currentShipCommand->formation.formationtype;
                        needFormation = TRUE;
                    }
                    else
                    {
                        needFormation = FALSE;
                    }
                    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&tempSelection);
                    if(needFormation)
                        clFormation(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,formation);
                    //need to reissue attack because the formation command cancels the attack!
                    clAttackThese(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SelectAnyCommand *)&targetSelection);

                    clMoveThese(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,from,to);
                }
            }
        }
        sizeofcopycom = sizeofSelectCommand(copycom->numShips);
        memcpy(selectcom,copycom,sizeofcopycom);
        memFree(copycom);
        numShips = selectcom->numShips;
        if(numShips == 0)
            return;
    }


    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->move.destination = to;
    vecSub(newcommand->move.heading,to,from);
    vecNormalize(&newcommand->move.heading);
    newcommand->ordertype.order = COMMAND_MOVE;
    newcommand->ordertype.attributes = 0;

    InitShipsForAI(selection,TRUE);

    CalculateMoveToPoints(selection,from,to);

    for (i=0;i<numShips;i++)
    {
        //selection->ShipPtr[i]->moveTo = to;
        selection->ShipPtr[i]->moveFrom = selection->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : clMove
    Description : Command to move ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;

    MakeShipsMobile(selectcom);
    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_MOVE,NULL,NULL);

    if (selectcom->numShips < 1)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to move");
#endif
        return;
    }

#ifdef DEBUG_TACTICS
    if(tacticsOn)
#endif
    {
        tacticsReportMove(comlayer,selectcom);
    }

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);
            ChangeOrderToMove(command,from,to);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clMoveThese(comlayer,copycom,from,to);
    }

    memFree(copycom);
}


void TurnAllGravwellsOff(SelectCommand *selectcom)
{
    sdword i;

    for(i=0;i<selectcom->numShips;i++)
    {
        if(selectcom->ShipPtr[i]->shiptype == GravWellGenerator)
        {
            if( ((GravWellGeneratorSpec *)(selectcom->ShipPtr[i]->ShipSpecifics))->GravFieldOn)
            {
                turnoffGravwell(selectcom->ShipPtr[i]);
            }
        }
    }
}

void ChangeOrderToHyperSpace(CommandToDo *alreadycommand,vector from,vector to)
{
    SelectCommand *selectcom = alreadycommand->selection;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.order == COMMAND_MP_HYPERSPACEING)
    {
        dbgMessagef("\nA little wierd:  Ships that were hyperspaceing have been given an order to hyperspace again.  Ignoreing...");
        return;
        dbgFatalf(DBG_Loc,"Shouldn't Be able to Order these guys...they're hyperspaceing!");
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to Hyperspace");
#endif
        FreeLastOrder(alreadycommand);

        alreadycommand->ordertype.order = COMMAND_MP_HYPERSPACEING;
    }

    alreadycommand->move.destination = to;
    vecSub(alreadycommand->move.heading,to,from);
    vecNormalize(&alreadycommand->move.heading);

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        InitShipAI(selectcom->ShipPtr[0],TRUE);
    }
    else
    {
        InitShipsForAI(selectcom,TRUE);
    }

    //CalculateMoveToPoints(selectcom,from,to);

    //for (i=0;i<selectcom->numShips;i++)
    //{
    //    //selectcom->ShipPtr[i]->moveTo   = to;
    //    selectcom->ShipPtr[i]->moveFrom = selectcom->ShipPtr[i]->posinfo.position;
    //    //soundEventStartEngine(selectcom->ShipPtr[i]);
    //}
    alreadycommand->hyperSpaceingTime = 0.0f;
    alreadycommand->hyperspaceState = COM_HYPERSPACE_START;
    PrepareShipsForCommand(alreadycommand,TRUE);

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clMpHyperspaceThese
    Description : Command to mp hyperspace ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMpHyperspaceThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships To hyperspace");
#endif
        return;
    }
    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        ChangeOrderToHyperSpace(alreadycommand,from,to);
        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to hyperspace");
#endif

    TurnAllGravwellsOff(selectcom);

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->move.destination = to;
    vecSub(newcommand->move.heading,to,from);
    vecNormalize(&newcommand->move.heading);
    newcommand->ordertype.order = COMMAND_MP_HYPERSPACEING;
    newcommand->ordertype.attributes = 0;
    newcommand->hyperSpaceingTime = 0.0f;
    newcommand->hyperspaceState = COM_HYPERSPACE_START;

    InitShipsForAI(selection,TRUE);

    //CalculateMoveToPoints(selection,from,to);

    /*
      for (i=0;i<numShips;i++)
    {
        //selection->ShipPtr[i]->moveTo = to;
        selection->ShipPtr[i]->moveFrom = selection->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }
    */
    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : getNewHyperspaceingDestination
    Description : calculates the correct destination point for a group of ships hyperspacing
    Inputs      : selection, from, to,newTo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void getNewHyperspaceingDestination(vector *newTo,vector *from,vector *to,SelectCommand *selection)
{
    //sdword i;
//    vector groupPos;
    //real32 temp;

    //groupPos.x=0.0f;
    //groupPos.y=0.0f;
    //groupPos.z=0.0f;
    //for(i=0;i<selection->numShips;i++)
    //{
    //    vecAddTo(groupPos,selection->ShipPtr[i]->posinfo.position);
    //}
    //vecDivideByScalar(groupPos,((float)selection->numShips),temp);
    //grouppos is the average posiiton of the group
    vecSub(*newTo,selection->ShipPtr[0]->posinfo.position,*from);
    //add the offset to the desired desitination
    vecAddTo(*newTo,*to);
}

/*-----------------------------------------------------------------------------
    Name        : clMpHyperspace
    Description : Command to hyperspace ships in multiplayer game ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;
    sdword cost;
    vector distsqr;
    real32 tempdist,distance;
    sdword i;
    vector newTo;
    sdword playerindex;

    makeSelectionHyperspaceCapable(selectcom);

    //should probably leave in this check...you never know
    if (selectcom->numShips < 1)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to hyperspace");
#endif

        return;
    }

    vecSub(distsqr,from,to);
    tempdist = vecMagnitudeSquared(distsqr);
    distance = fsqrt(tempdist);
    cost = (sdword)hyperspaceCost(distance,selectcom);

    playerindex = selectcom->ShipPtr[0]->playerowner->playerIndex;
    if ( (playerindex >= tpGameCreated.numPlayers) ||    // computer player RU's arene't deterministic, always let it spend RUs (computer RUs can go negative)
         (universe.players[playerindex].resourceUnits >= cost) )
    {
        universe.players[playerindex].resourceUnits -= cost;
        universe.gameStats.playerStats[playerindex].totalResourceUnitsSpent += cost;
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nCan't afford HS - cancelling");
#endif
        return;
    }
#ifdef DEBUG_TACTICS
    if(tacticsOn)
#endif
    {
        tacticsReportMove(comlayer,selectcom);
    }

    //remove ships from being targeted...
    for(i=0;i<selectcom->numShips;i++)
    {
        RemoveShipFromBeingTargeted(comlayer,selectcom->ShipPtr[i],REMOVE_PROTECT|REMOVE_HYPERSPACING);
    }
    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc3(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to hyperspace (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {

            MakeShipsNotIncludeTheseShips(copycom,command->selection);
            getNewHyperspaceingDestination(&newTo,&from,&to,command->selection);
            ChangeOrderToHyperSpace(command,from,newTo);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        getNewHyperspaceingDestination(&newTo,&from,&to,copycom);
        clMpHyperspaceThese(comlayer,copycom,from,newTo);
    }

    memFree(copycom);
}


/*-----------------------------------------------------------------------------
    Name        : AttackClosestOpponent
    Description : finds and returns nearest opponent to ship to attack
    Inputs      : ship, todo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SpaceObjRotImpTarg *AttackClosestOpponent(Ship *ship,AttackCommand *attack)
{
    sdword numTargetsToAttack = attack->numTargets;
    sdword i;
    vector dist;
    real32 distsqr;
    real32 mindist;
    sdword attacki;
    SpaceObjRotImpTarg *attackme;

    if (numTargetsToAttack <= 0)
    {
        return NULL;
    }

    mindist = REALlyBig;
    attacki = -1;
    for (i=0;i<numTargetsToAttack;i++)
    {
        attackme = attack->TargetPtr[i];

        vecSub(dist,ship->posinfo.position,attackme->posinfo.position);
        distsqr = vecMagnitudeSquared(dist);

        if (distsqr < mindist)
        {
            mindist = distsqr;
            attacki = i;
        }
    }

    dbgAssert(attacki != -1);

    return attack->TargetPtr[attacki];
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromAttacking
    Description : Removes ship from attacking
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromAttacking(Ship *ship)
{
    if(ship->shiptype == ResourceCollector)
    {
        removeResourcerFromAttacking(ship);
    }
    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        memFree(ship->attackvars.multipleAttackTargets);
        ship->attackvars.multipleAttackTargets = NULL;
    }

    ship->attackvars.attacktarget = NULL;
    ship->attackvars.myLeaderIs = NULL;
    ship->attackvars.myWingmanIs = NULL;
    ship->shipisattacking = FALSE;
    bitClear(ship->specialFlags,SPECIAL_Kamikaze|SPECIAL_BrokenFormation|SPECIAL_AttackFromAbove);
    if (ship->flightman != FLIGHTMAN_NULL)
    {
        flightmanClose(ship);
    }
    if(ship->formationcommand != NULL)
    {
        if(ship->formationcommand->formation.formationLocked)
            unlockFormation(ship->formationcommand);

        ship->formationcommand->formation.flipselectionfixed = FALSE;
        ship->formationcommand->formation.needFix = FALSE;
    }
    //dbgMessagef("\nShip %i is no longer attacking", ship->shipID.shipNumber);
}

/*-----------------------------------------------------------------------------
    Name        : AttackCleanup
    Description : cleans up anything from a attack command
    Inputs      : attacktodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AttackCleanup(struct CommandToDo *attacktodo)
{
    sdword i;
    SelectCommand *selection = attacktodo->selection;
    sdword numShips = selection->numShips;
    bool clearmove = TRUE;

    if (bitTest(attacktodo->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING))
    {
        bitClear(attacktodo->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
    }

    if(attacktodo->ordertype.order == COMMAND_MOVE)
    {
        clearmove = FALSE;
    }


    if(attacktodo->ordertype.attributes & COMMAND_IS_FORMATION)
    {
#ifdef DEBUG_TACTICS
        if(tacticsOn)
#endif
        {
            if ((attacktodo->formation.formationtype == SPHERE_FORMATION) && (attacktodo->selection->numShips > 0))
            {
                attacktodo->formation.enders = FALSE;
                //attacktodo->formation.endership = NULL;
                formationContentHasChanged(attacktodo);
            }
        }
    }

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromAttacking(selection->ShipPtr[i]);
        // don't clear unless clearing a move otherwise some moves will get erroneously cancelled
        if(clearmove)
        {
            vecZeroVector(selection->ShipPtr[i]->moveTo);
            vecZeroVector(selection->ShipPtr[i]->moveFrom);
        }
        bitClear(selection->ShipPtr[i]->specialFlags,SPECIAL_BrokenFormation);


    }

    //attack has been cancelled, lets cue up a gun closing animation
    //need a delay item here...
    madLinkInCloseGuns(attacktodo,1);
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromFormation
    Description : cleans up ship stuff that was in a formation
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromFormation(Ship *ship)
{
    ship->formationcommand = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : FormationCleanup
    Description : cleans up anything from a formation
    Inputs      : formationtodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FormationCleanup(struct CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromFormation(selection->ShipPtr[i]);
    }

    if (formationtodo->formation.formationSpecificInfo)
    {
        memFree(formationtodo->formation.formationSpecificInfo);
        formationtodo->formation.formationSpecificInfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromMoving
    Description : removes ship from moving
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromMoving(Ship *ship)
{
    vecZeroVector(ship->moveTo);
    vecZeroVector(ship->moveFrom);
}

/*-----------------------------------------------------------------------------
    Name        : MoveCleanup
    Description : cleans up anything from a move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MoveCleanup(struct CommandToDo *movetodo)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromMoving(selection->ShipPtr[i]);
    }
}

void removeShipFromMpHyperspaceing(Ship *ship)
{
    vecZeroVector(ship->moveTo);
    vecZeroVector(ship->moveFrom);
    bitSet(ship->flags,SOF_Selectable);
}

/*-----------------------------------------------------------------------------
    Name        : MpHyperspaceCleanup
    Description : cleans up anything from a hyperspaceing move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MpHyperspaceCleanup(struct CommandToDo *movetodo)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        removeShipFromMpHyperspaceing(selection->ShipPtr[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveAttackTargetFromExtraAttackInfo
    Description : removes shiptoremove from being targeted for attack. Whatever ship was attacking it
                  will choose a new target.
    Inputs      : shiptoremove, extraattackinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo)
{
    sdword i;
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    Ship *ship;
    AttackTargets *multipleAttackTargets;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];

        if (ship->attackvars.attacktarget == targettoremove)
        {
            ship->attackvars.attacktarget = NULL;
            ship->attackvars.attackevents |= ATTACKEVENT_ENEMYTARGET_DIED;
        }

        if ((multipleAttackTargets = ship->attackvars.multipleAttackTargets) != NULL)
        {
            sdword j;
            sdword numAttackTargets = multipleAttackTargets->numAttackTargets;

            for (j=0;j<numAttackTargets;j++)
            {
                if (multipleAttackTargets->TargetPtr[j] == targettoremove)
                {
                    multipleAttackTargets->TargetPtr[j] = NULL;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipReferencesFromExtraAttackInfo
    Description : removes shiptoremove from being reference by extraattackinfo.  For example, if a wingman ship
                  was removed, the reference to the wingman would be removed.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo)
{
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    SpecificAttackVars *attackvars;
    sdword i;
    Ship *ship;

    dbgAssert(shiptoremove);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        attackvars = &ship->attackvars;

        if (ship == shiptoremove)
        {
            continue;       // don't process one that is going to be removed
        }

        if (attackvars->myLeaderIs == shiptoremove)
        {
            // this ship's leader has fallen, so we should not be his wingman anymore.
            attackvars->myLeaderIs = NULL;
            dbgAssert(attackvars->myWingmanIs == NULL);

            attackvars->attackevents |= ATTACKEVENT_LEADER_DIED;
        }

        if (attackvars->myWingmanIs == shiptoremove)
        {
            // lost wingman
            dbgAssert(attackvars->myLeaderIs == NULL);
            attackvars->myWingmanIs = NULL;

            // we lost our wingman
            attackvars->attackevents |= ATTACKEVENT_WINGMAN_DIED;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : allocateMultipleAttackTargets
    Description : allocates a AttackTargets structure based on the number of guns on ship
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
AttackTargets *allocateMultipleAttackTargets(Ship *ship)
{
    sdword numGuns = ship->gunInfo->numGuns;
    AttackTargets *attacktargets;
    sdword sizeofAttackTargets;

    dbgAssert(numGuns > 0);
    sizeofAttackTargets = sizeofAttackTargets(numGuns);
    attacktargets = memAlloc(sizeofAttackTargets,"AttackTargets",0);
    memset(attacktargets,0,sizeofAttackTargets);

    attacktargets->numAttackTargets = numGuns;

    return attacktargets;
}

typedef struct
{
    vector targetInShipCoordSys;
    real32 pickedMe;
} PickMultipleTargetsInfo;

/*-----------------------------------------------------------------------------
    Name        : pickMultipleAttackTargets
    Description : picks best attack target for each gun on a ship
    Inputs      : ship, attack
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pickMultipleAttackTargets(Ship *ship,AttackCommand *attack)
{
    sdword numShipsToAttack = attack->numTargets;
    AttackTargets *attacktargets = ship->attackvars.multipleAttackTargets;
    sdword numAttackTargets = attacktargets->numAttackTargets;
    PickMultipleTargetsInfo *multipleTargetsInfo;
    PickMultipleTargetsInfo *thisTargetInfo;
    SpaceObjRotImpTarg *target;
    SpaceObjRotImpTarg *target1 = NULL;
    SpaceObjRotImpTarg *target2 = NULL;
    sdword i;
    sdword j;
    GunInfo *gunInfo = ship->gunInfo;
    Gun *gun;
    GunStatic *gunstatic;
    sdword bestIndex;
    real32 bestDist;
    vector targetInWorldCoordSys;
    vector targetInShipCoordSys;
    real32 dist;
    real32 range;
    real32 temp;
    real32 dotprod,bonus,gunRange;

    dbgAssert(numShipsToAttack > 0);
    dbgAssert(numAttackTargets > 0);
    dbgAssert(numAttackTargets == gunInfo->numGuns);

    multipleTargetsInfo = memAlloc(sizeof(PickMultipleTargetsInfo)*numShipsToAttack,"mTI(multTargInfo)",Pyrophoric);

    for (i=0,thisTargetInfo=multipleTargetsInfo;i<numShipsToAttack;i++,thisTargetInfo++)
    {
        target = attack->TargetPtr[i];

        aishipGetTrajectory(ship,target,&targetInWorldCoordSys);
        matMultiplyVecByMat(&thisTargetInfo->targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);
        thisTargetInfo->pickedMe = 1.0f;
    }
    if(ship->staticinfo->shipclass == CLASS_Fighter)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Fighter][ship->tacticstype];
    }
    else if(ship->staticinfo->shipclass == CLASS_Corvette)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Corvette][ship->tacticstype];
    }
    else
    {
        bonus = 1.0f;
    }

    if (ship->staticinfo->shipclass == CLASS_Frigate)
    {
        target = ship->attackvars.attacktarget;
        if (target->objtype == OBJ_ShipType)
        {
           if (((Ship *)target)->staticinfo->shipclass == CLASS_Corvette)
           {
               target1 = target;
               for (i=0;i<numShipsToAttack;i++)
               {
                   if (attack->TargetPtr[i] != target1)
                   {
                       target2 = attack->TargetPtr[i];
                       break;
                   }
               }
           }
        }
    }

    for (i=0;i<numAttackTargets;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;

        gunRange = gunstatic->bulletrange*bonus;
        bestDist = REALlyBig;
        bestIndex = -1;

        for (j=0,thisTargetInfo=multipleTargetsInfo;j<numShipsToAttack;j++,thisTargetInfo++)
        {
            target = attack->TargetPtr[j];
            if (target1)
            {
                // concentrate firepower: skip this loop for other ships than target1, target2
                if ((target != target1) && (target != target2))
                {
                    continue;
                }
            }

            vecSub(targetInShipCoordSys,thisTargetInfo->targetInShipCoordSys,gunstatic->position);

            dist = fsqrt(vecMagnitudeSquared(targetInShipCoordSys));

            range = RangeToTargetGivenDist(ship,target,dist);

            if (range < gunRange)
            {
                vecDivideByScalar(targetInShipCoordSys,dist,temp);

                dotprod = vecDotProduct(targetInShipCoordSys,gunstatic->gunnormal);

                dist *= (2.0f - dotprod);
                dist *= thisTargetInfo->pickedMe;   // treat distance as if it were bigger for targets already picked
                if (dist < bestDist)
                {
                    bestDist = dist;
                    bestIndex = j;
                }
            }
        }

        if (bestIndex != -1)
        {
            attacktargets->TargetPtr[i] = attack->TargetPtr[bestIndex];
            multipleTargetsInfo[bestIndex].pickedMe *= 1.2f;
        }
        else
        {
            attacktargets->TargetPtr[i] = NULL;
        }
    }

    memFree(multipleTargetsInfo);
}

#define WINGMANSHIP_USED       1
#define WINGMANSHIP_IGNORE     2

typedef struct
{
    sdword numShips;
    sdword flags[1];
} FindWingmenInfo;

#define sizeofFindWingmenInfo(n) (sizeof(FindWingmenInfo) + (((n)-1)*sizeof(sdword)))

/*-----------------------------------------------------------------------------
    Name        : BreakupIntoLeaderWingman
    Description : breaks up the selection of ships into leader/wingman pairs
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void BreakupIntoLeaderWingman(SelectCommand *selection)
{
    sdword numShips = selection->numShips;
    udword sizeofFindwingmen;
    FindWingmenInfo *findwingmen;
    sdword *flags;
    vector dist;
    real32 distsqr;
    Ship *compareto;
    Ship *wingman;
    sdword wingmanj;
    sdword i,j;
    Ship *leader;
    real32 mindist;
    Ship *ship;

    sizeofFindwingmen = sizeofFindWingmenInfo(numShips);
    findwingmen = memAlloc(sizeofFindwingmen,"findwingmen",0);

    findwingmen->numShips = numShips;
    for (i=0,flags=&findwingmen->flags[0];i<numShips;i++,flags++)
    {
        ship = selection->ShipPtr[i];

        if ((isShipInterceptor(ship) || ship->shiptype == DefenseFighter) &&
            (ship->shiprace != P1) && (ship->shiprace != P2) &&
            ((gamerand() & 255) <= FIGHTER_PAIRUP_PROB)
           )
        {
            *flags = 0;
        }
        else
        {
            *flags = WINGMANSHIP_IGNORE;
        }
    }

    for (i=0;i<numShips;i++)
    {
        if (findwingmen->flags[i])
        {
            continue;
        }

        leader = selection->ShipPtr[i];

        if(leader->shiptype == DefenseFighter)
        {
            sdword cont=FALSE;
            for(j=i+1;j<numShips;j++)
            {
                if (findwingmen->flags[j])
                {
                    continue;
                }
                if(selection->ShipPtr[j]->shiptype != DefenseFighter)
                {
                    Ship *temp = selection->ShipPtr[j];
                    selection->ShipPtr[j] = leader;
                    selection->ShipPtr[i] = temp;
                    cont = TRUE;
                    break;
                }
            }
            if(cont)
            {
                i--;
            }
            continue;
        }

        mindist = REALlyBig;
        wingmanj = 0;
        for (j=i+1;j<numShips;j++)
        {
            if (findwingmen->flags[j])
            {
                continue;
            }

            compareto = selection->ShipPtr[j];

            if(compareto->shiptype == DefenseFighter)
            {
                //autoselect defenseFighter
                wingmanj = j;
                break;
            }

            vecSub(dist,leader->posinfo.position,compareto->posinfo.position);
            distsqr = vecMagnitudeSquared(dist);

            if (distsqr >= WINGMAN_JOIN_MAX_DIST)
            {
                continue;
            }

            if (distsqr < mindist)
            {
                mindist = distsqr;
                wingmanj = j;
            }
        }

        if (wingmanj != 0)
        {
            // found a wingman
            wingman = selection->ShipPtr[wingmanj];
            findwingmen->flags[wingmanj] |= WINGMANSHIP_USED;

            leader->attackvars.myLeaderIs = NULL;
            leader->attackvars.myWingmanIs = wingman;
            wingman->attackvars.myLeaderIs = leader;
            wingman->attackvars.myWingmanIs = NULL;
        }
    }

    memFree(findwingmen);
}

typedef struct
{
    sdword numFlaggedShips;
    sdword flags[1];
} AssignTargetsInfo;

#define sizeofAssignTargetsInfo(n) (sizeof(AssignTargetsInfo) + (((n)-1)*sizeof(sdword)))

/*-----------------------------------------------------------------------------
    Name        : InitExtraAttackInfo
    Description : intializes the extra attack information in the ships based on attackers (selection)
                  and ships getting attacked (attack)
    Inputs      : selection, attack
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitExtraAttackInfo(SelectCommand *selection,AttackCommand *attack,bool findwingmen)
{
    sdword numShips = selection->numShips;
    sdword numTargetsToAttack = attack->numTargets;
    sdword i,j,k;
    SpecificAttackVars *attackvars;
    sdword sizeofAssigntargets;
    AssignTargetsInfo *assigntargets;
    sdword closestshipj;
    vector dist;
    real32 distsqr;
    real32 mindist;
    SpaceObjRotImpTarg *target;
    Ship *ship;
    sdword *flags;
    vector avgposSelection = { 0.0f, 0.0f, 0.0f };
    vector avgposTargets = { 0.0f, 0.0f, 0.0f };
    vector diff;
    real32 temp;
    Ship *leader = selection->ShipPtr[0];
    bool focusfire = FALSE;

    dbgAssert(numShips > 0);
    dbgAssert(numTargetsToAttack > 0);

    if (leader->tacticstype == Aggressive)
    {
        focusfire = TRUE;
    }

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        //dbgAssert(ship->attackvars.multipleAttackTargets == NULL);
        ship->attackvars.multipleAttackTargets = NULL;
        ship->attackvars.attacktarget = NULL;
        ship->attackvars.myLeaderIs = NULL;
        ship->attackvars.myWingmanIs = NULL;
        ship->attackvars.flightmansLeft = 0;
        ship->attackvars.attacksituation = 0;
        ship->attackvars.attackevents = 0;
        bitClear(ship->specialFlags,SPECIAL_Kamikaze|SPECIAL_BrokenFormation|SPECIAL_AttackFromAbove);

        ship->tacticsNeedToFlightMan = FALSE;
        ship->tacticsTalk = 0;

        vecAddTo(avgposSelection,ship->posinfo.position);
    }

    for (i=0;i<numTargetsToAttack;i++)
    {
        target = attack->TargetPtr[i];
        vecAddTo(avgposTargets,target->posinfo.position);
    }

    vecDivideByScalar(avgposSelection,((real32)numShips),temp);
    vecDivideByScalar(avgposTargets,((real32)numTargetsToAttack),temp);
    vecSub(diff,avgposSelection,avgposTargets);
    diff.z = ABS(diff.z);

    if ((diff.z >= ATTACKING_FROM_ABOVE_MIN_DIST) &&
        (diff.z >= (fsqrt(diff.x*diff.x + diff.y*diff.y)*ATTACKING_FROM_ABOVE_MIN_RATIO)))
    {
        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            bitSet(ship->specialFlags,SPECIAL_AttackFromAbove);
        }
    }

    // break fighters up into leader/wingman.
    if (findwingmen)
    {
        BreakupIntoLeaderWingman(selection);
    }

    if (numTargetsToAttack == 1)
    {
        for (i=0;i<numShips;i++)
        {
            attackvars = &selection->ShipPtr[i]->attackvars;
            if (!IAmAWingman(attackvars))
            {
                attackvars->attacktarget = attack->TargetPtr[0];
            }
        }
    }
    else
    {
        sizeofAssigntargets = sizeofAssignTargetsInfo(numTargetsToAttack);
        assigntargets = memAlloc(sizeofAssigntargets,"assigntargets",Pyrophoric);
        memset(assigntargets,0,sizeofAssigntargets);

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            attackvars = &ship->attackvars;
            if (IAmAWingman(attackvars))
            {
                continue;
            }

            closestshipj = -1;
            mindist = REALlyBig;
            for (j=0,flags=&assigntargets->flags[0];j<numTargetsToAttack;j++,flags++)
            {
                if (*flags != 0)
                {
                    continue;
                }

                target = attack->TargetPtr[j];

                vecSub(dist,ship->posinfo.position,target->posinfo.position);
                distsqr = vecMagnitudeSquared(dist);

                if (distsqr < mindist)
                {
                    mindist = distsqr;
                    closestshipj = j;
                }
            }

            dbgAssert(closestshipj != -1);

            dbgAssert(assigntargets->flags[closestshipj] == 0);
            assigntargets->flags[closestshipj] = 1;
            assigntargets->numFlaggedShips++;
            dbgAssert(assigntargets->numFlaggedShips <= numTargetsToAttack);
            if (assigntargets->numFlaggedShips == numTargetsToAttack)
            {
                for (k=0,flags=&assigntargets->flags[0];k<numTargetsToAttack;k++,flags++)
                {
                    *flags = 0;
                }
                assigntargets->numFlaggedShips = 0;
            }

            attackvars->attacktarget = attack->TargetPtr[closestshipj];

            if (focusfire && (i > 0) && leader->attackvars.attacktarget)
            {
                attackvars->attacktarget = leader->attackvars.attacktarget;     // override for focus fire
            }

            if (((ShipStaticInfo *)(ship->staticinfo))->canTargetMultipleTargets)
            {
                dbgAssert(attackvars->multipleAttackTargets == NULL);
                attackvars->multipleAttackTargets = allocateMultipleAttackTargets(ship);
                pickMultipleAttackTargets(ship,attack);
            }

        }

        memFree(assigntargets);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToAttack
    Description : changes order to attack
    Inputs      : alreadycommand, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToAttack(CommandToDo *alreadycommand,AttackCommand *attackcom)
{
    AttackCommand *attack;
    udword sizeofattack;
    SelectCommand *selection = alreadycommand->selection;
    bool dontresetattack = FALSE;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.order == COMMAND_MP_HYPERSPACEING)
    {
        return;     // don't want to attack under any circumstances if hyperspacing!
    }

    if (alreadycommand->ordertype.order == COMMAND_ATTACK)
    {
        if (SelectionsAreTotallyEquivalent((SelectCommand *)alreadycommand->attack,(SelectCommand *)attackcom))
        {
            if(alreadycommand->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
            {
                bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
            }
            return;
        }
        dontresetattack = TRUE;
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nReceived Order to change target of attack");
#endif
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to attack");
#endif
    }

    dbgAssert(AreAllShipsAttackCapable(alreadycommand->selection));

    FreeLastOrder(alreadycommand);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,TRUE);

    alreadycommand->ordertype.order = COMMAND_ATTACK;
    alreadycommand->attack = attack;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        alreadycommand->formation.doneInitialAttack = FALSE;
    }

    PrepareShipsForCommand(alreadycommand,TRUE);

    if (dontresetattack)
    {
        InitShipsForAIDontResetAttack(alreadycommand->selection,TRUE);
    }
    else
    {
        InitShipsForAI(alreadycommand->selection,TRUE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clAttackThese
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *clAttackThese(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    AttackCommand *attack;
    udword sizeofselection;
    udword sizeofattack;

    // for safety
    if (!AreAllShipsAttackCapable(selectcom))
    {
        MakeShipsAttackCapable(selectcom,selectcom);
        dbgMessage("\nWARNING: ships not capable of attacking told to attack");
    }

    if (selectcom->numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships As Attackers");
#endif
        return NULL;
    }

    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO ATTACK.");
        return NULL;
    }

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            ClearProtecting(alreadycommand);
        }


        ChangeOrderToAttack(alreadycommand,attackcom);

        //link in animation to open guns here:
        //madLinkInOpenGuns(alreadycommand,1);

        return alreadycommand;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to attack");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,TRUE);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_ATTACK;
    newcommand->ordertype.attributes = 0;
    newcommand->attack = attack;

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    //link in animation to open guns here:
    //madLinkInOpenGuns(newcommand,1);


    return newcommand;
}

/*-----------------------------------------------------------------------------
    Name        : clAttack
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;

    // for safety
    if (!AreAllShipsAttackCapable(selectcom))
    {
        MakeShipsAttackCapable(selectcom,selectcom);
        dbgMessage("\nWARNING: ships not capable of attacking told to attack");
    }

    if (selectcom->numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships As Attackers");
#endif
        return;
    }

    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO ATTACK.");
        return;
    }
    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_ATTACK,NULL,NULL);

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);

            if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                ClearProtecting(command);
            }

            ChangeOrderToAttack(command,attackcom);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clAttackThese(comlayer,copycom,attackcom);
    }

    memFree(copycom);
}
/*-----------------------------------------------------------------------------
    Name        : canChangeOrderToPassiveAttack
    Description : returns TRUE if alreadycommand can passive attack
    Inputs      :
    Outputs     :
    Return      : returns TRUE if alreadycommand can passive attack
----------------------------------------------------------------------------*/
bool canChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attack)
{
    if (alreadycommand->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
    {
        return FALSE;
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        if ((alreadycommand->attack == NULL) || (attack == NULL))
        {
            return FALSE;
        }

        if (!SelectionsAreEquivalent((SelectCommand *)alreadycommand->attack,(SelectCommand *)attack))
        {
            return TRUE;
        }
    }

    switch (alreadycommand->ordertype.order)
    {
        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_MILITARYPARADE:
            return TRUE;

        case COMMAND_MOVE:
        case COMMAND_DOCK:
#ifdef DEBUG_TACTICS
            if(tacticsOn)
#endif
            {
                if (!tacticsAreStrikeCraftInSelection(alreadycommand->selection))
                    return TRUE;        //this is a new exceptions
                else
                    return FALSE;
            }
        case COMMAND_ATTACK:
        case COMMAND_MP_HYPERSPACEING:
            return FALSE;
        case COMMAND_LAUNCHSHIP:
        case COMMAND_COLLECTRESOURCE:
        case COMMAND_SPECIAL:
        case COMMAND_BUILDINGSHIP:
            return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToPassiveAttack
    Description : Activates passive attack for command
    Inputs      : alreadycommand, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attackcom)
{
    AttackCommand *attack;
    udword sizeofattack;
    SelectCommand *selection = alreadycommand->selection;
    sdword count,i;

    // should check canChangeOrderToPassiveAttack before calling this routine
    dbgAssert(canChangeOrderToPassiveAttack(alreadycommand,attackcom));

    count=0;
    for(i=0;i<alreadycommand->selection->numShips;i++)
    {
        if(alreadycommand->selection->ShipPtr[i]->passiveAttackCancelTimer > universe.totaltimeelapsed)
        {
            //count a bad ship
            count++;
        }
    }
    if( (((real32) count)/((real32) alreadycommand->selection->numShips)) > 0.5f)
    {
        //more than 50% in group bad, so don't passive attack
        return;
    }

#ifdef gshaw
    dbgMessage("\nChanging Order to passive attack");
#endif

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,FALSE);

    alreadycommand->ordertype.attributes |= COMMAND_IS_PASSIVEATTACKING;
    bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);      // don't do holding pattern if passive attacking
    alreadycommand->attack = attack;

    InitShipsForAI(alreadycommand->selection,FALSE);

    PrepareShipsForCommand(alreadycommand,FALSE);

    //setup formation properly here
    if(alreadycommand->ordertype.order == COMMAND_NULL)
    {
        if(alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            //NULL command and in formation
            lockFormation(alreadycommand,0);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clPassiveAttack
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPassiveAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    AttackCommand *attack;
    udword sizeofselection;
    udword sizeofattack;
    sdword count,i;

    if (selectcom->numShips == 0)
    {
#ifdef gshaw
        dbgMessage("\nNo Ships For Passive attack");
#endif
        return;
    }

    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO PASSIVE ATTACK.");
        return;
    }

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (canChangeOrderToPassiveAttack(alreadycommand,attackcom))
        {
            ChangeOrderToPassiveAttack(alreadycommand,attackcom);
        }
        else
        {
            dbgMessage("\nWARNING: Couldn't change order to passive attack");
        }
        return;
    }

#ifdef gshaw
    dbgMessage("\nReceived Order to passive attack");
#endif

    count=0;
    for(i=0;i<selectcom->numShips;i++)
    {
        if(selectcom->ShipPtr[i]->passiveAttackCancelTimer > universe.totaltimeelapsed)
        {
            //count a bad ship
            count++;
        }
    }
    if( (((real32) count)/((real32) selectcom->numShips)) > 0.5f)
    {
        //more than 50% in group bad, so don't passive attack
        return;
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,FALSE);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_PASSIVEATTACKING;
    newcommand->attack = attack;

    InitShipsForAI(selection,FALSE);

    PrepareShipsForCommand(newcommand,FALSE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToSpecial
    Description : changes order to special
    Inputs      : alreadycommand, targets
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToSpecial(CommandToDo *alreadycommand,SpecialCommand *targets)
{
    SpecialCommand *specialtargets;
    udword sizeofspecialtargets;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    FreeLastOrder(alreadycommand);

    if (targets != NULL)
    {
        sizeofspecialtargets = sizeofSelectCommand(targets->numTargets);
        specialtargets = memAlloc(sizeofspecialtargets,"ToDoSpecial",0);
        memcpy(specialtargets,targets,sizeofspecialtargets);
        alreadycommand->specialtargets = specialtargets;
    }
    else
    {
        alreadycommand->specialtargets = NULL;
    }

    alreadycommand->ordertype.order = COMMAND_SPECIAL;

    PrepareShipsForCommand(alreadycommand,TRUE);
}

bool ShipsAreSalCapCorvettes(Ship *ship)
{
    if (ship->shiptype == SalCapCorvette)
    {
        return TRUE;
    }
    return FALSE;
}

void clPlaySpeechEventsForSupportCraft(SelectCommand *selection)
{
    sdword i;

    if(selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
    {
        //local machines ships...
        for(i=0;i<selection->numShips;i++)
        {
            switch(selection->ShipPtr[i]->shiptype)
            {
            case Carrier:
            case ResourceController:
            case ResourceCollector:
            case AdvanceSupportFrigate:
            case RepairCorvette:
                //I don't think we need battle chatter limiting...
                speechEvent(selection->ShipPtr[0],COMM_Support,0);
                return;     //return after first one...all we need!
            }
        }

    }
}

/*-----------------------------------------------------------------------------
    Name        : clSpecial
    Description : Command to do special
    Inputs      : selectcom, targets
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    SpecialCommand *specialtargets;
    udword sizeofselection;
    udword sizeofspecialtargets;

    if (selectcom->numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships for special");
#endif
        return;
    }

    if(selectcom->numShips == 1)
    {
        if(selectcom->ShipPtr[0]->shiptype == SalCapCorvette)
        {
            CommandToDo *oldcommand=getShipAndItsCommand(&universe.mainCommandLayer,selectcom->ShipPtr[0]);
            if(oldcommand != NULL)
            {
                if(oldcommand->ordertype.order == COMMAND_SPECIAL)
                {
                    if((((SalCapCorvetteSpec *)selectcom->ShipPtr[0]->ShipSpecifics)->target) != NULL)
                    {
                        if(ShipInSelection((SelectCommand *)targets,(Ship *)(((SalCapCorvetteSpec *)selectcom->ShipPtr[0]->ShipSpecifics)->target)))
                        {
                            //ship is targetting a ship in the list already...lets ignore the command

                            //if we get bugs..we could only do this if the target list is the same exactly
                            return;
                        }
                    }
                }
            }

        }
    }

    clPlaySpeechEventsForSupportCraft(selectcom);
    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_SPECIAL,NULL,(SelectCommand *)targets);


    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    if (targets == NULL)
    {
        // this is just a special activate command, so activate it for each ship and return without modifying
        // the command layer as long as the specialActivateIsContinuous flag isn't set.
        sdword numShips = selectcom->numShips;
        sdword i;
        Ship *ship;
        ShipStaticInfo *shipstaticinfo;

        for (i=0;i<numShips;i++)
        {
            ship = selectcom->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);

            if (!shipstaticinfo->specialActivateIsContinuous)
            {
                clRemoveShipFromSelection(selection,ship);

                if (shipstaticinfo->custshipheader.CustShipSpecialActivate)
                {
                    shipstaticinfo->custshipheader.CustShipSpecialActivate(ship);
                }

            }
        }

        if (selection->numShips == 0)
        {
            // all ships have been activated, so return
            memFree(selection);
            return;
        }
    }

    if (!DoAnyShipsFollowConstraints(selection,ShipsAreSalCapCorvettes))        // Bryce later make salcap corvettes be able to salvage in formation fix later
    {
        if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selection)) != NULL)
        {
            memFree(selection);
            ChangeOrderToSpecial(alreadycommand,targets);
            return;
        }
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to do special");
#endif

    RemoveShipsFromDoingStuff(comlayer,selection);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    if (targets != NULL)
    {
        sizeofspecialtargets = sizeofSelectCommand(targets->numTargets);
        specialtargets = memAlloc(sizeofspecialtargets,"ToDoSpecial",0);
        memcpy(specialtargets,targets,sizeofspecialtargets);
        newcommand->specialtargets = specialtargets;
    }
    else
    {
        newcommand->specialtargets = NULL;
    }

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_SPECIAL;
    newcommand->ordertype.attributes = 0;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

void tacticsChangedForShipCB(Ship *ship,TacticsType oldtactics,TacticsType newtactics)
{
#if 0               // take this out due to micromanagement abuse
    if(newtactics == Evasive)
    {
        tacticsSetShipToDoDodge(ship);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : clSetTactics
    Description : Sets tactics for a group of ships
    Inputs      : selectcom, tacticstype
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    TacticsType oldtactics;

#ifdef DEBUG_TACTICS
    if(!tacticsOn) return;  //don't change tactics settings if not enabled
#endif

    dbgAssert(tacticstype >= 0);
    dbgAssert(tacticstype < NUM_TACTICS_TYPES);

    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        //adjust ships maximum bullet range (don't forget to adjust
        //individual guns ranges on the fly
        /*
        if(ship->staticinfo->shipclass == CLASS_Fighter)
        {
            ship->bulletrage*=tacticsInfo.BulletRangeBonus[CLASS_Fighter][ship->tacticstype];
        }
        else if (ship->staticinfo->shipclass == CLASS_Corvette)
        {
            ship->bulletrage*=tacticsInfo.BulletRangeBonus[Tactics_Corvette][ship->tacticstype];
        }
        */

        oldtactics = ship->tacticstype;

        //if ship wasn't in evassive, make sure its attack target is set to
        //NULL if switching to evassive so that the wingman leader AI will work properly...

        if(tacticstype == Evasive)
            if(oldtactics != Evasive)
                if (ship->attackvars.myLeaderIs)
                {
                    ship->attackvars.attacktarget = NULL;   //set target to
                }

        ship->tacticstype = tacticstype;
        tacticsChangedForShipCB(ship,oldtactics,tacticstype);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clRUTransfer
    Description : Transfers RU's from selectcom.numShips to selectcom.timeLastStatus
    Inputs      : selectcom,resourceUntis
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clRUTransfer(CommandLayer *comlayer, sdword toIndex, sdword fromIndex, sdword resourceUnits,ubyte flags)
{
    //Place sound event here bitch (thats right...I'm talking to you Shane)
    sdword from,to;
    from = fromIndex;
    to = toIndex;

    if (resourceUnits <= 0)     // prevents cheating client side even if they hack it
        return;

    switch(flags)
    {
    case RUTRANS_GENERATEDRUS:
    case RUTRANS_BUILTSHIP:
    case RUTRANS_BUILDCANCEL:
        // obsolete now
        break;
    default:
        if (universe.players[to].playerState == PLAYER_DEAD)
            return;

        if (universe.players[from].playerState == PLAYER_DEAD)
            return;

        universe.players[from].resourceUnits -= resourceUnits;
        universe.players[to].resourceUnits += resourceUnits;

        universe.gameStats.playerStats[from].totalResourceUnits -= resourceUnits;
        universe.gameStats.playerStats[from].totalResourceUnitsGiven += resourceUnits;

        universe.gameStats.playerStats[to].totalResourceUnits += resourceUnits;
        universe.gameStats.playerStats[to].totalResourceUnitsRecieved += resourceUnits;

        if (from == universe.curPlayerIndex)
        {                                                   //if we've given some away
            speechEventFleet(COMM_F_ResourcesTransferred, 0, from);
        }
        else if (to == universe.curPlayerIndex)
        {                                                   //if we just got some
            soundEvent(NULL, UI_ReceivingRUs);
            speechEventFleet(COMM_F_ResourcesReceived, 0, to);
        }
        break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSetKamikaze
    Description : Sets the kamikaze bit for all ships that are kamikaze capable
                  provided they are doing an attack...ideally, we do that check before we transmitt
    Inputs      : selectcom,
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom)
{
     sdword numShips = selectcom->numShips;
     sdword i;
     Ship *ship;
     CommandToDo *command;

     for (i=0;i<numShips;i++)
     {
         ship = selectcom->ShipPtr[i];
         command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
         if(command == NULL)
         {

         }
         else if(command->ordertype.order != COMMAND_ATTACK)
         {
            if(command->ordertype.order == COMMAND_SPECIAL)
            {
                if(ship->shiptype == SalCapCorvette)
                {
                    bitSet(ship->specialFlags,SPECIAL_Kamikaze|SPECIAL_BrokenFormation);
                    continue;
                }
            }
             if(ship->playerowner->playerIndex ==
                universe.curPlayerIndex)
             {
                 //shouldn't set  the kamikazee stuff...
                /////////////////////
                //no target kamikazee speech event
                 //num: COMM_Kamikaze_NoTargets
                 //battle chatter

                 //////////////
             }
         }
         else
         {
            bitSet(ship->specialFlags,SPECIAL_Kamikaze|SPECIAL_BrokenFormation);
         }
     }
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToHalt
    Description : changes order to Halt
    Inputs      : alreadycommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToHalt(CommandToDo *alreadycommand)
{
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
    }

    FreeLastOrder(alreadycommand);
    alreadycommand->ordertype.order = COMMAND_HALT;

    PrepareShipsForCommand(alreadycommand,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : clHaltThese
    Description : command for ships to halt
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clHaltThese(CommandLayer *comlayer,SelectCommand *selectcom)
{
    CommandToDo *command;
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;
    sdword i;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to halt!");
#endif
        return;
    }

    for (i=0;i<selectcom->numShips;)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,selectcom->ShipPtr[i]);
        if(command != NULL)
        {
            if(command->ordertype.order == COMMAND_DOCK ||
                command->ordertype.order == COMMAND_LAUNCHSHIP)
            {
                //if docking ship is NULL, we can assume some stuff...
                if (selectcom->ShipPtr[i]->dockingship != NULL)
                {
                    if(selectcom->ShipPtr[i]->shiptype != ResearchShip)
                    {
                        //signal docking ship to cancel
                        selectcom->ShipPtr[i]->forceCancelDock=TRUE;
                        selectcom->numShips--;
                        selectcom->ShipPtr[i] = selectcom->ShipPtr[selectcom->numShips];
                        continue; // we must check same index again because we put last array entry here
                    }
                }
            }
        }
        i++;
    }

    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_HALT,NULL,NULL);

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadycommand->ordertype.order == COMMAND_MOVE)
        {
            speechEvent(selectcom->ShipPtr[0], COMM_MoveCancelled, 0);
        }
        else if (alreadycommand->ordertype.order == COMMAND_ATTACK)
        {
            speechEvent(selectcom->ShipPtr[0], COMM_AttackCancelled, 0);
        }

        #ifdef HW_DEBUG
              dbgAssert(alreadycommand->ordertype.order != COMMAND_MP_HYPERSPACEING);
        #endif


        ChangeOrderToHalt(alreadycommand);
        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to halt");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_HALT;
    newcommand->ordertype.attributes = 0;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : clHalt
    Description : command for ships to halt
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clHalt(CommandLayer *comlayer,SelectCommand *selectcom)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    bool stoppassiveattacking;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to halt!");
#endif
        return;
    }

    for(i=0;i<selectcom->numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        stoppassiveattacking = FALSE;

        if (ship->command == NULL)
            stoppassiveattacking = TRUE;
        else
        {
            switch (ship->command->ordertype.order)
            {
                case COMMAND_NULL:
                case COMMAND_HALT:
                case COMMAND_MILITARYPARADE:
                    stoppassiveattacking = TRUE;
            }
        }

        if (stoppassiveattacking)
            ship->passiveAttackCancelTimer=universe.totaltimeelapsed+TW_GLOBAL_PASSIVE_ATTACK_CANCEL_MODIFIER;
        else
            ship->passiveAttackCancelTimer=0.0f;
    }

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);

            ChangeOrderToHalt(command);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clHaltThese(comlayer,copycom);
    }

    memFree(copycom);
}

/*-----------------------------------------------------------------------------
    Name        : clScuttle
    Description : command for ship to scuttle itself
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clScuttle(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    sdword i;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bScuttle)
        return;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to scuttle!");
#endif
        return;
    }

    for (i=0;i<numShips;i++)
    {
        selectcom->ShipPtr[i]->deathtime = universe.totaltimeelapsed + frandombetween(SCUTTLE_SHIPDEATHTIMEMIN,SCUTTLE_SHIPDEATHTIMEMAX);
    }

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clLaunchShip
    Description : command for a ship to launch from a ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clLaunchShip(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr receiverShip)
{
    CommandToDo *newcommand;
    SelectCommand *selection;
    Ship *ship = selectcom->ShipPtr[0];
    ShipStaticInfo *receiverShipStaticInfo = (ShipStaticInfo *)receiverShip->staticinfo;

    dbgAssert(selectcom->numShips == 1);
    dbgAssert(receiverShip != NULL);
    dbgAssert(receiverShipStaticInfo->canReceiveShips|receiverShipStaticInfo->canReceiveShipsPermanently);

    if (ship->specialFlags & SPECIAL_Launching)
    {
        return;         // already launching
    }

    if (ship->shiplink.belongto)
    {
        return;         // already outside
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    selection = memAlloc(sizeof(SelectCommand),"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeof(SelectCommand));

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_LAUNCHSHIP;
    newcommand->ordertype.attributes = 0;

    newcommand->launchship.receiverShip = receiverShip;

    dockPrepareSingleShipForLaunch(ship,receiverShip);
    dockInitShipForLaunch(ship);

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);

    //listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    //insert at begining to take priority over other commands that will be occurring
    //i.e. retiring
    listAddNodeBeginning(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : clLaunchMultipleShips
    Description : command for multiple ships to launch from a ship
    Inputs      : selectcom, receiverShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    SelectCommand selectone;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to launch");
#endif
        return;
    }

    if (launchFrom == NULL)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ship to launch from");
#endif
        return;
    }

    selectone.numShips = 1;
    for (i=0;i<numShips;i++)
    {
        if (!(selectcom->ShipPtr[i]->attributes & ATTRIBUTES_Defector))      // don't let the defector launch
        {
            selectone.ShipPtr[0] = selectcom->ShipPtr[i];
            clLaunchShip(comlayer,&selectone,launchFrom);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : LaunchAllInternalShipsOfPlayerThatMustBeLaunched
    Description : launches all internal ships of given player that must be launched
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void LaunchAllInternalShipsOfPlayerThatMustBeLaunched(struct Player *player)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if ((ship->shipsInsideMe) && (ship->shiptype != DDDFrigate))
            {
                Node *insidenode = ship->shipsInsideMe->insideList.head;
                InsideShip *insideship;
                SelectCommand selectone;

                while (insidenode != NULL)
                {
                    insideship = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                        // make defector launch
                    if ((insideship->ship->attributes & ATTRIBUTES_Defector) || (ShipHasToLaunch(ship,insideship->ship)))
                    {
                        selectone.numShips = 1;
                        selectone.ShipPtr[0] = insideship->ship;

                        clLaunchShip(&universe.mainCommandLayer,&selectone,ship);
                    }

                    insidenode = insidenode->next;
                }
            }
        }

        objnode = objnode->next;
    }
}

void FillInCarrierMothershipInfo(struct Player *player,Ship **mothership,Ship *carrierX[])
{
    sdword i,j;

    *mothership = NULL;
    for (i=0;i<4;i++)
    {
         carrierX[i] = NULL;
    }

    *mothership = player->PlayerMothership;
    if ((*mothership) && ((*mothership)->shiptype == Carrier))
    {
         carrierX[0] = *mothership;
         *mothership = NULL;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].owner == player)
        {
            for (j=0;j<4;j++)       // find first free slot and put it in
            {
                if (carrierX[j] == NULL)
                {
                    carrierX[j] = cmCarriers[i].ship;
                    break;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : LaunchAllInternalShipsOfPlayer
    Description : launches all internal ships of given player
    Inputs      : carriermask: 0=momship, 1=carrier1, 2=carrier2
    Outputs     :
    Return      : the number of ships launched
----------------------------------------------------------------------------*/
sdword LaunchAllInternalShipsOfPlayer(struct Player *player, udword carriermask)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    Ship *mothership;
    Ship *carriers[4];
    sdword numShips = 0;

    FillInCarrierMothershipInfo(player,&mothership,carriers);

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if ((ship->shipsInsideMe) && (ship->shiptype != DDDFrigate))
            {
                if ( ((carriermask & BIT0) && (ship == mothership)) ||
                     ((carriermask & BIT1) && (ship == carriers[0])) ||
                     ((carriermask & BIT2) && (ship == carriers[1])) ||
                     ((carriermask & BIT3) && (ship == carriers[2])) ||
                     ((carriermask & BIT4) && (ship == carriers[3])) )
                {
                    Node *insidenode = ship->shipsInsideMe->insideList.head;
                    InsideShip *insideship;
                    SelectCommand selectone;

                    while (insidenode != NULL)
                    {
                        insideship = (InsideShip *)listGetStructOfNode(insidenode);
                        dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                        if (!(insideship->ship->attributes & ATTRIBUTES_Defector))      // don't let the defector launch
                        {
                            selectone.numShips = 1;
                            selectone.ShipPtr[0] = insideship->ship;

                            clLaunchShip(&universe.mainCommandLayer,&selectone,ship);
                            numShips++;
                        }

                        insidenode = insidenode->next;
                    }

                }

            }
        }

        objnode = objnode->next;
    }

    return (numShips);
}

/*-----------------------------------------------------------------------------
    Name        : clAutoLaunch
    Description : command to turn the autoLaunch flag on or off for a given player
    Inputs      : OnOff - bit0=Mothership, bit1=Carrier 1, bit2=Carrier2, bit3=Carrier3, bit4=Carrier4
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clAutoLaunch(udword OnOff,udword playerIndex)
{
    Player *player = &universe.players[playerIndex];
    bool speechevent = FALSE;
    sdword i;
    udword bit = BIT0;

    for (i=0;i<(4+1);i++,bit<<=1)       // 4 Carriers + 1 Mothership        BIT0..BIT4 are valid
    {
        if ((player->autoLaunch & bit) != (OnOff & bit))
        {
            if (OnOff & bit)
            {
                // changing autoLaunch from Off to On
                player->autoLaunch |= bit;
                // tell all internal ships to launch:
                if (LaunchAllInternalShipsOfPlayer(player, bit) > 0)
                {
                    speechevent = TRUE;
                }
            }
            else
            {
                // changing autoLaunch from On to Off
                player->autoLaunch &= ~bit;
            }
        }
    }

    if (speechevent && (singlePlayerGameInfo.currentMission != 16))
    {                  //fleet command is out of commission in Mission 16, so don't play this
        speechEventFleet(COMM_F_AutolaunchOn, 0, playerIndex);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSetAlliance
    Description : sets an alliance based on a player bitmask
    Inputs      : player bitmask
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void clSetAlliance(udword AllianceType, uword curalliance, uword newalliance)
{
    allianceSetAlliance(AllianceType, curalliance, newalliance);
}

void clSetResearch(udword type, udword playernum, udword labnum, udword tech)
{
    if (type == RESEARCH_SUBCOMMAND_START)
    {
        rmAssignPlayersLabToResearch(&universe.players[playernum], labnum, tech);
    }
    else if (type == RESEARCH_SUBCOMMAND_STOP)
    {
        rmClearResearchlab(&universe.players[playernum], labnum);
    }
}

bool ShipHasToLaunch(Ship *InsideShip, Ship *ship)
{
    if (singlePlayerGame)
    {
        if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
        {
            return FALSE;
        }
    }

    if (ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH)
    {
        return FALSE;
    }

    if ( (ship->staticinfo->shipclass==CLASS_Fighter) &&
         (InsideShip->shipsInsideMe->FightersInsideme <=
          InsideShip->staticinfo->maxDockableFighters) )
        return (FALSE);

    if ( (ship->staticinfo->shipclass==CLASS_Corvette) &&
         (InsideShip->shipsInsideMe->CorvettesInsideme <=
          InsideShip->staticinfo->maxDockableCorvettes) )
        return (FALSE);

    return (TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : clCreateShip
    Description : creates a new ship
    Inputs      : shipType, shipRace, playerIndex, creator
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Ship *clCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    Ship *ship;
    SelectCommand select;


    Player *player = &universe.players[playerIndex];
    Ship *mothership, *carrier1=NULL, *carrier2=NULL;
    sword i;
    udword launchmask;


    mothership = player->PlayerMothership;
    if ((mothership) && (mothership->shiptype == Carrier))
    {
        carrier1 = mothership;
        mothership = NULL;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].owner == player)
        {
            if (carrier1)
            {
                carrier2 = cmCarriers[i].ship;
                break;
            }
            else
            {
                carrier1 = cmCarriers[i].ship;
            }
        }
    }

    if (creator == mothership)
    {
        launchmask = BIT0;
    }
    else if (creator == carrier1)
    {
        launchmask = BIT1;
    }
    else if (creator == carrier2)
    {
        launchmask = BIT2;
    }





    if (creator == NULL)
    {
        return NULL;         // if creator died, too bad for ships inside
    }

    if (universe.players[playerIndex].playerState != PLAYER_ALIVE)
    {
        return NULL;         // if player is not alive, don't allow ship creation
    }

    dbgAssert(playerIndex < universe.numPlayers);

    // create ship
    ship = univCreateShip(shipType,shipRace,&creator->posinfo.position,&universe.players[playerIndex],1);

    gameStatsAddShip(ship,playerIndex);

    // update unit cap variables
    unitCapCreateShip(ship,&universe.players[playerIndex]);

    // update the special internet ghost totals, because of lag.
    if ((multiPlayerGame) && (playerIndex==sigsPlayerIndex))
    {
        shiplagtotals[shipType]--;
    }

    // if ship is a research ship and player is curplayer or aiplayer

    // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
    if (ship->staticinfo->shiptype==ResearchShip)
    {
/*        if ( (&universe.players[playerIndex]==universe.curPlayerPtr) ||
             (universe.players[playerIndex].aiPlayer!=NULL) )
        {*/
            rmActivateFreeLab(&universe.players[playerIndex]);
//        }
    }

    // put ship inside creator
    dockPutShipInside(ship,creator);

    if ( (universe.players[playerIndex].autoLaunch & launchmask) ||
         (ShipHasToLaunch(creator, ship)) )
    {
        select.numShips = 1;
        select.ShipPtr[0] = ship;


        clLaunchShip(comlayer,&select,(ShipPtr)creator);
    }
    return(ship);
}

/*-----------------------------------------------------------------------------
    Name        : processBuildingShipToDo
    Description : processes ship building command
    Inputs      :
    Outputs     :
    Return      : TRUE if command is done
----------------------------------------------------------------------------*/
bool processBuildingShipToDo(CommandToDo *command)
{
    Player *player;
    Ship *creator;

    if (universe.univUpdateCounter > command->buildingship.frameAtWhichToCreate)
    {
        creator = command->buildingship.creator;
        dbgAssert(creator != NULL);
        player = creator->playerowner;

        clCreateShip(&universe.mainCommandLayer,command->buildingship.shipType,command->buildingship.shipRace,
                     player->playerIndex,creator);

        return TRUE;
    }

    return FALSE;
}

udword NumberOfOtherShipsOfSameTypeBeingBuilt(ShipType shipType,ShipPtr creator,udword playerIndex)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    udword numOtherBeingBuilt = 0;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        if ((command->ordertype.order == COMMAND_BUILDINGSHIP) && (command->buildingship.playerIndex == playerIndex))
        {
            if (command->buildingship.shipType == shipType)
            {
                // if they don't have a creator or both have the same creator
                if ( ((creator == NULL) || (command->buildingship.creator == NULL)) ||
                     (creator == command->buildingship.creator))
                {
                    numOtherBeingBuilt++;
                }
            }
        }

        curnode = curnode->next;
    }

    return numOtherBeingBuilt;
}

void clDeterministicBuild(udword command, CommandLayer* comLayer,
                          sdword numShips,
                          ShipType shipType, ShipRace shipRace,
                          uword playerIndex, ShipPtr creator)
{
    Player* player = &universe.players[playerIndex];

    dbgAssert(playerIndex < universe.numPlayers);

    if (creator == NULL)
    {
        //creator died, ships die too
        return;
    }

    if (player->playerState != PLAYER_ALIVE)
    {
        //!alive players can't build ships
        return;
    }

    cmDeterministicBuild(command, numShips, shipType, shipRace, playerIndex, creator);
}

/*-----------------------------------------------------------------------------
    Name        : clBuildShip
    Description : builds a new ship (including waiting time for it to be built)
    Inputs      : shipType, shipRace, playerIndex, creator
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    Player *player = &universe.players[playerIndex];
    ShipStaticInfo *shipstaticinfo = GetShipStaticInfo(shipType,shipRace);
    CommandToDo *newcommand;

    dbgAssert(playerIndex < universe.numPlayers);

    if (creator == NULL)
    {
        return;         // if creator died, too bad for ships inside
    }

    if (player->playerState != PLAYER_ALIVE)
    {
        return;         // if player is not alive, don't allow ship creation
    }

    //if (shipstaticinfo->buildCost <= player->resourceUnits)
    {
        player->resourceUnits -= shipstaticinfo->buildCost;     // subtract cost right away
        universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsSpent += shipstaticinfo->buildCost;
        // make new command indicating delayed build:

        newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
        InitCommandToDo(newcommand);

        newcommand->selection = memAlloc(sizeofSelectCommand(1),"NoSelection",0);
        newcommand->selection->numShips = 0;
        newcommand->ordertype.order = COMMAND_BUILDINGSHIP;
        newcommand->ordertype.attributes = 0;
        newcommand->buildingship.shipType = shipType;
        newcommand->buildingship.shipRace = shipRace;
        newcommand->buildingship.creator = creator;
        newcommand->buildingship.playerIndex = (udword)playerIndex;
        newcommand->buildingship.frameAtWhichToCreate = universe.univUpdateCounter + ((NumberOfOtherShipsOfSameTypeBeingBuilt(shipType,creator,playerIndex)+1) * shipstaticinfo->buildTime);

        PrepareShipsForCommand(newcommand,TRUE);

        listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    }
}

/*-----------------------------------------------------------------------------
    Name        : FindShipsWereGuarding
    Description : finds ships we're guarding, either directly or indirectly
    Inputs      :
    Outputs     :
    Return      : returns TRUE if terminating condition was finding that shipsWereGuarding are directly or
                  indirectly guarding us - returns FALSE if terminating condition is we can't find any more ships we're guarding
----------------------------------------------------------------------------*/
bool FindShipsWereGuarding(CommandToDo *originalus,SelectCommand *us,GrowSelection *shipsWereGuarding,sdword iteration)
{
    sdword i,j;
    sdword usnumShips = us->numShips;
    Ship *ship;
    CommandToDo *shipcommand;
    GrowSelection newshipsWereGuarding;
    bool result;

    if (iteration > 10)
    {
        return FALSE;       // safety for recursive function
    }

    for (i=0;i<usnumShips;i++)
    {
        ship = us->ShipPtr[i];
        shipcommand = ship->command;
        if (shipcommand && (shipcommand->ordertype.attributes & COMMAND_IS_PROTECTING))
        {
            for (j=0;j<shipcommand->protect->numShips;j++)
            {
                growSelectAddShipNoDuplication(shipsWereGuarding,shipcommand->protect->ShipPtr[j]);
            }
        }
    }

    if (shipsWereGuarding->selection->numShips == 0)
    {
        return FALSE;
    }

    if (AnyOfTheseShipsAreInSelection(shipsWereGuarding->selection,originalus->selection))
    {
        return TRUE;
    }

    growSelectInit(&newshipsWereGuarding);

    result = FindShipsWereGuarding(originalus,shipsWereGuarding->selection,&newshipsWereGuarding,iteration+1);

    growSelectClose(&newshipsWereGuarding);

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : CheckCircularGuard
    Description : Checks for circular guard, and if it is sets the bit SPECIAL_2_CircularGuard for each ship otherwise clears it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CheckCircularGuard(CommandToDo *command)
{
    sdword i;
    GrowSelection shipsWereGuarding;

    growSelectInit(&shipsWereGuarding);

    if (FindShipsWereGuarding(command,command->selection,&shipsWereGuarding,0))
    {
        // ships we're guarding are directly or indirectly guarding us:
        SelectCommand *selection = command->selection;

        for (i=0;i<selection->numShips;i++)
        {
            bitSet(selection->ShipPtr[i]->specialFlags2,SPECIAL_2_CircularGuard);
        }
    }
    else
    {
        // ships we're guarding are NOT directly or indirectly guarding us:
        SelectCommand *selection = command->selection;

        for (i=0;i<selection->numShips;i++)
        {
            bitClear(selection->ShipPtr[i]->specialFlags2,SPECIAL_2_CircularGuard);
        }
    }

    growSelectClose(&shipsWereGuarding);
}

void ChangeOrderToProtect(CommandToDo *alreadycommand,ProtectCommand *protectcom)
{
    ProtectCommand *protect;
    udword sizeofprotect;

    alreadycommand->protectFlags = 0;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        if (SelectionsAreTotallyEquivalent(alreadycommand->protect,protectcom))
        {
            return;
        }
        ClearProtecting(alreadycommand);
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nChanged Order to protect");
#endif

    FreeLastOrder(alreadycommand);

    alreadycommand->ordertype.order = COMMAND_NULL;
    alreadycommand->ordertype.attributes |= COMMAND_IS_PROTECTING;

    sizeofprotect = sizeofProtectCommand(protectcom->numShips);
    protect = memAlloc(sizeofprotect,"ToDoProtect",0);
    memcpy(protect,protectcom,sizeofprotect);

    alreadycommand->protect = protect;

    PrepareShipsForCommand(alreadycommand,TRUE);
    CheckCircularGuard(alreadycommand);

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        if (alreadycommand->formation.formationtype == SPHERE_FORMATION)
        {
            // we are now protecting in sphere formation, and we want to surround
            // a ship that we protect when go in sphere formation.  This is different
            // than the regular sphere formation, so call to tell formationContentHasChanged
            formationContentHasChanged(alreadycommand);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clProtect
    Description : command to protect a ship
    Inputs      : selectcom, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *clProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    ProtectCommand *protect;
    udword sizeofselection;
    udword sizeofprotect;
    sdword numShips = selectcom->numShips;
    sdword numShipsToProtect = protectcom->numShips;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships selected as protectors");
#endif
        return NULL;
    }
    //cleanup like halt command
    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_HALT,NULL,NULL);

    if (numShipsToProtect == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to protect");
#endif
        return NULL;
    }

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        ChangeOrderToProtect(alreadycommand,protectcom);
        return alreadycommand;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to protect");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofprotect = sizeofProtectCommand(numShipsToProtect);
    protect = memAlloc(sizeofprotect,"ToDoProtect",0);
    memcpy(protect,protectcom,sizeofprotect);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_PROTECTING;
    newcommand->protect = protect;
    newcommand->protectFlags = 0;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    CheckCircularGuard(newcommand);
    return newcommand;
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToGroup
    Description : Adds ship to CommandToDo group
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToGroup(ShipPtr ship,CommandToDo *group)
{
    SelectCommand *newselection;
    sdword sizeofnewselection;
    sdword sizeofselection;
    sdword numShips;

    numShips = group->selection->numShips;
    dbgAssert(numShips >= 1);

    sizeofnewselection = sizeofSelectCommand(numShips+1);
    sizeofselection = sizeofSelectCommand(numShips);

    newselection = memAlloc(sizeofnewselection,"ToDoSelection",NonVolatile);
    memcpy(newselection,group->selection,sizeofselection);

    memFree(group->selection);
    group->selection = newselection;

    newselection->ShipPtr[numShips] = ship;
    newselection->numShips = numShips + 1;

    PrepareOneShipForCommand(ship,group,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToFormationGroup
    Description : adds a ship to group
    Inputs      : ship, group
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToFormationGroup(ShipPtr ship,CommandToDo *group)
{
#ifdef DEBUG_LAUNCHSHIP
    dbgMessage("\nAdding ship to group");
#endif

    AddShipToGroup(ship,group);
    FillInShipFormationStuff(ship,group);

    dbgAssert(group->ordertype.attributes & COMMAND_IS_FORMATION);
    formationContentHasChanged(group);

    InitShipAI(ship,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : thereareothercompatibleresearchships(ship)
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool thereareothercompatibleresearchships(Ship *ship)
{
    Node *resnode;
    Ship *resship;

    dbgAssert(ship->shiptype == ResearchShip);

    resnode = universe.ShipList.head;

    if(((ResearchShipSpec *)ship->ShipSpecifics)->master)
        return FALSE;

    while(resnode != NULL)
    {
        resship = (Ship *) listGetStructOfNode(resnode);
        if(resship->shiptype == ResearchShip && resship->playerowner == ship->playerowner)
        {    //there exists a ship to dock with...
            if(resship != ship)
            {
                return TRUE;
            }
        }
        resnode = resnode->next;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : GroupShipIntoMilitaryParade
    Description : groups ship into military parade around aroundShip
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GroupShipIntoMilitaryParade(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
    CommandToDo *militaryGroup = GetMilitaryGroupAroundShip(comlayer,aroundShip);
    if (militaryGroup != NULL)
    {
        AddShipToMilitaryGroup(ship,militaryGroup);
    }
    else
    {
        CreateMilitaryGroupAroundShip(comlayer,ship,aroundShip);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSetMilitaryParade
    Description : sets ships into a military parade
    Inputs      : selectcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword index;
    sdword i;
    Ship *aroundShip;
    Ship *ship;
    CommandToDo *command;

    index = MothershipOrCarrierIndexInSelection(selectcom);
    if (index < 0)
    {
        dbgMessagef("\nWarning: No ship to form parade around");
        return;
    }

    dbgAssert(index < selectcom->numShips);
    aroundShip = selectcom->ShipPtr[index];

    for (i=0;i<selectcom->numShips;i++)
    {
        if (i != index)
        {
            ship = selectcom->ShipPtr[i];
            command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
            if ((command != NULL) && (command->ordertype.order == COMMAND_MILITARYPARADE) &&
                (command->militaryParade->aroundShip  == aroundShip))
            {
                // already in military Parade around aroundShip
                // do nothing
            }
            else
            {
                SelectCommand selectone;
                selectone.ShipPtr[0] = ship;
                selectone.numShips = 1;
                RemoveShipsFromDoingStuff(comlayer,&selectone);
                GroupShipIntoMilitaryParade(comlayer,selectcom->ShipPtr[i],aroundShip);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : GroupShip
    Description : puts a ship into an appropriate group around the aroundShip
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GroupShip(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
//    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    CommandToDo *shipGroup;
//    sdword groupSize;
//    vector destination;
    SelectCommand selection;


//    groupSize = shipstaticinfo->groupSize;
//    dbgAssert(groupSize > 0);
    if((ship->shiptype == ResearchShip) && (thereareothercompatibleresearchships(ship)))
    {
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        bitClear(ship->flags,SOF_Selectable);
        clDock(comlayer, &selection, DOCK_FOR_RESEARCH, NULL);
        return;
    }
    shipGroup = getShipAndItsFormationCommand(comlayer,aroundShip);
    if (shipGroup != NULL)
    {
        // the aroundShip is in formation, so let's just add this ship to aroundShip's formation
        AddShipToFormationGroup(ship,shipGroup);
        return;
    }

    GroupShipIntoMilitaryParade(comlayer,ship,aroundShip);

#if 0
    if (groupSize == 1)
    {
        // just position at random location
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        GetRandomLocationAroundShip(&destination,aroundShip);
        clMove(comlayer,&selection,ship->posinfo.position,destination);
    }
    else
    {
        shipGroup = FindGroupForShip(comlayer,ship,groupSize);

        if (shipGroup == NULL)
        {
            // make new group, and position ship at random location
            selection.numShips = 1;
            selection.ShipPtr[0] = ship;
            GetRandomLocationAroundShip(&destination,aroundShip);
            clFormation(comlayer,&selection,DELTA_FORMATION);
            clMove(comlayer,&selection,ship->posinfo.position,destination);
        }
        else
        {
            // put ship in that group
            AddShipToFormationGroup(ship,shipGroup);
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : clShipDied
    Description : Call this function to update ship command layer when ship dies
    Inputs      : deadship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clShipDied(CommandLayer *comlayer,ShipPtr deadship)
{
    SelectCommand selection;

    selection.numShips = 1;
    selection.ShipPtr[0] = deadship;

    RemoveShipsFromDoingStuff(comlayer,&selection);

    RemoveShipFromBeingTargeted(comlayer,deadship,REMOVE_PROTECT);

    // Update player unit caps info
    unitCapDeleteShip(deadship,deadship->playerowner);

    // if ship is a research ship and player is curplayer or aiplayer

    // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
    if (deadship->staticinfo->shiptype==ResearchShip)
    {
/*        if ( (deadship->playerowner==universe.curPlayerPtr) ||
             (deadship->playerowner->aiPlayer!=NULL) )
        {*/
            rmDeactivateLab(deadship->playerowner);
//        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : TargetWithinAttackRange
    Description : returns TRUE if ship within attack range of target
    Inputs      : ship,target
    Outputs     :
    Return      : returns TRUE if ship within attack range of target
----------------------------------------------------------------------------*/
bool TargetWithinAttackRange(Ship *ship,SpaceObjRotImpTarg *target)
{
    vector trajectory;
    real32 range;

    aishipGetTrajectory(ship,target,&trajectory);
    range = RangeToTarget(ship,target,&trajectory);

    if (range < 10000.0f)
    {
        return TRUE;        // consider within attack range sooner
    }
#if 0
    if(((ShipStaticInfo *)(ship->staticinfo))->bulletRange[ship->tacticstype] == 0.0f)
    {
        if(ship->shiptype == CloakGenerator ||
            ship->shiptype == DFGFrigate ||
            ship->shiptype == GravWellGenerator)
        {
            if(range < 7000.0f)
            {
                return TRUE;
            }
        }
    }
#endif
    if (range < ((ShipStaticInfo *)(ship->staticinfo))->bulletRange[ship->tacticstype])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void putFightersInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}
void putCorvettesInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}
void putCapitalsInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass != CLASS_Fighter
           && src->ShipPtr[i]->staticinfo->shipclass != CLASS_Corvette)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}



/*-----------------------------------------------------------------------------
    Name        : clHoldingPattern
    Description : Puts command into a holding pattern by
                  breaking into small formations n' such.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define MAX_UNORDERED_GROUP_SIZE    11  //maximum ships in a group after
                                        //attacking {ships ungrouped previously)

void clHoldingPattern(CommandLayer *comlayer,CommandToDo *command)
{
    bool needToPassiveAttack = FALSE;
    bool needToProtect = FALSE;
    MaxSelection passiveAttackSelection;
    MaxSelection protectSelection;
    MaxSelection fighters,corvettes,capitals;
    MaxSelection working;
    MaxSelection *curselection;
    sdword i,j,k;

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        return;         // already in formation, don't need to split them up into deltas
    }

    if(command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        if(command->attack->numTargets > 0)
        {
            //if group was passive attacking, set a flag so we remember that that was the case
            needToPassiveAttack = TRUE;
            //store the target list acordingly
            selSelectionCopy((MaxAnySelection *)&passiveAttackSelection,(MaxAnySelection *)command->attack);
        }
    }
    if(command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        if(command->protect->numShips > 0)
        {
            //group was protecting a ship, set the protection list and
            //set a flag accordingly
            selSelectionCopy((MaxAnySelection *)&protectSelection,(MaxAnySelection *)command->protect);
            needToProtect = TRUE;
        }
    }

    putFightersInSelection(&fighters,(MaxSelection *)command->selection);
    putCorvettesInSelection(&corvettes,(MaxSelection *)command->selection);
    putCapitalsInSelection(&capitals,(MaxSelection *)command->selection);

    // want to make sure all ships go into either fighters, corvettes, or frigates
    dbgAssert(fighters.numShips + corvettes.numShips + capitals.numShips == command->selection->numShips);

    //loop through all types of ships {capitals,fighters and corvetters}
    for(j = 0; j < 3; j++)
    {
        working.numShips = 0;
        if(j == 0)
            curselection = &fighters;
        else if (j==1)
            curselection = &corvettes;
        else
            curselection = &capitals;
        k =0;

        //loop through the current selection and divide type into groups
        //of MAX_UNORDERED_GROUP_SIZE
        for(i=0;i<curselection->numShips;i++)
        {
            working.ShipPtr[k] = curselection->ShipPtr[i];
            k++;
            if(k == MAX_UNORDERED_GROUP_SIZE)
            {
                //maximum # reached for this group
                //set their formation,targets,and protection flags accordingly
                working.numShips = k;
                clFormation(comlayer,(SelectCommand *)&working,DELTA_FORMATION);

                //if group was previously passiveattacking or
                //protecting set them to doing the same thing again with the
                //same target list
                if(needToPassiveAttack)
                    clPassiveAttack(comlayer,(SelectCommand *)&working,(SelectAnyCommand *)&passiveAttackSelection);
                if(needToProtect)
                    clProtect(comlayer,(SelectCommand *)&working,(SelectCommand *)&protectSelection);
                working.numShips = 0;
                k = 0;
            }
        }
        //done looping through groups
        working.numShips = k;
        if(k > 0)
        {
            //set remaining ships to formation + other tasks
            clFormation(comlayer,(SelectCommand *)&working,DELTA_FORMATION);
            //if group was previously passiveattacking or
            //protecting set them to doing the same thing again with the
            //same target list
            if(needToPassiveAttack)
                clPassiveAttack(comlayer,(SelectCommand *)&working,(SelectAnyCommand *)&passiveAttackSelection);
            if(needToProtect)
                clProtect(comlayer,(SelectCommand *)&working,(SelectCommand *)&protectSelection);
            working.numShips = 0;
            k = 0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : salCapCorvettesInSelectionCantDoFormation
    Description : returns true if salcaps in selection can't dealwith
                  a formation at the present time
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool salCapCorvettesInSelectionCantDoFormation(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->shiptype == SalCapCorvette)
        {
            if(selection->ShipPtr[i]->specialFlags & SPECIAL_BrokenFormation)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : processHoldingPattern
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool processHoldingPattern(CommandToDo *command,SelectCommand *selection)
{
    sdword i,count;
    vector destination,heading,right;

    if(selection->numShips == 0)
        return TRUE;

    //flight forward for 2 secs...then turn
    if((universe.totaltimeelapsed - command->holdingStartTime) > TW_HOLDING_PATTERN_PATHTIME)

    {
        //turn right
        command->holdingStartTime = universe.totaltimeelapsed;
        matGetVectFromMatrixCol2(right,selection->ShipPtr[0]->rotinfo.coordsys);
        matGetVectFromMatrixCol3(heading,selection->ShipPtr[0]->rotinfo.coordsys);
        vecScalarMultiply(right,right,TW_HOLDING_PATTERN_RIGHT_COEFFICIENT);
        vecScalarMultiply(heading,heading,TW_HOLDING_PATTERN_HEADING_COEFFICIENT);
        vecAdd(command->holdingPatternVec,right,heading);
        vecScalarMultiply(command->holdingPatternVec,command->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);

        command->turnAroundTimes++;
        if(command->turnAroundTimes > TW_HOLDING_PATTERN_NUM_TURNS)
            return TRUE;
    }
    count = selection->numShips;
    if(command->ordertype.attributes & COMMAND_IS_FORMATION)
        count = 1;  //only process leader if command is in formation!
    for(i=0;i<count;i++)
    {
        vecAdd(destination,command->holdingPatternVec,selection->ShipPtr[i]->holdingPatternPos);
        aishipFlyToPointAvoidingObjs(selection->ShipPtr[i],&destination,AISHIP_PointInDirectionFlying|AISHIP_FastAsPossible,TW_HOLDING_PATTERN_PATH_MAX_SPEED);
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : clProcess
    Description : processes the command layer (executes all commands)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clProcess(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;
    Node *nextnode;
    bool passiveAttacked;
    bool done;

    Ship *ship;
    Ship *target;
    SpaceObjRotImpTarg *attacktarget;
    sdword i;
    SelectCommand *commandselection;

#ifndef HW_Release
    flightmanTest();
#endif

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

#ifdef DEBUG_COMMANDLAYER
//        dbgMessagef("\nclProcessing %d %d",command->ordertype.order,command->ordertype.attributes);
#endif

        commandselection = command->selection;

        if(singlePlayerGame)
        {
            for (i=0;i<commandselection->numShips;i++)
            {
                ship = commandselection->ShipPtr[i];
                if (ship->flags & (SOF_Hyperspace|SOF_Crazy))
                {
                    goto nextcommand;       // don't process command if any ships in hyperspace ether or Crazy
                }
                ship->autostabilizeship = FALSE; // don't autostabilize ship if ship has a command to do
            }
        }
        else
        {
            //multiplayer version...allowing hyperspace!
            for (i=0;i<commandselection->numShips;i++)
            {
                ship = commandselection->ShipPtr[i];
                if (ship->flags & (SOF_Crazy))
                {
                    goto nextcommand;       // don't process command if any ships  Crazy
                }
                ship->autostabilizeship = FALSE; // don't autostabilize ship if ship has a command to do
            }
        }
        if (command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            switch (command->ordertype.order)
            {
                case COMMAND_ATTACK:
                    ship = command->selection->ShipPtr[0];
                    attacktarget = command->attack->TargetPtr[0];
#ifdef DEBUG_TACTICS
                    if(tacticsOn)
#endif
                    {
                        if(!tacticsInfo.holdFormationDuringBattle[ship->tacticstype] || command->selection->numShips == 1)
                        {   //evasive tactics, so attack as per olden times

                            if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                            {
                                command->formation.doneInitialAttack = TRUE;
                                if (processAttackToDo(command))
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING &&
                                        command->selection->ShipPtr[0]->aistatecommand == 0)
                                    {
                                        bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                        ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                                    }
                                    else
                                    {
                                        // free COMMAND_ATTACK stuff
                                        FreeLastOrder(command);
                                        // attack has finished so convert it back to a formation
                                        command->ordertype.order = COMMAND_NULL;
                                    }
                                }
                                else
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                                    {
                                        if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                                        if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                                        {
                                            if(command->selection->ShipPtr[0]->aistatecommand == 0)
                                            {
                                                //it is at this distance that the group of ships turns and hitails it out of there...
                                                bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                                ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                                                break;
                                            }
                                            else
                                            {
                                                ChangeOrderToHalt(command);
                                            }
                                        }
                                    }

                                }
                            }
                            else
                            {
                                aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                            command->formation.travelvel);
                                ship->shipidle = FALSE;
                                processFormationToDo(command,FALSE,FALSE);
                            }
                        }
                        else
                        {  // neutral or aggressive leader tactics,
                            //formation type is held during combat, be it loose or tight
                            //command->formation.doneInitialAttack = TRUE;    // we attack in formation anyway, so let's just attack
                            if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                            {
                                command->formation.doneInitialAttack = TRUE;
                                if (processAttackToDoInFormation(command))
                                {
                                    // free COMMAND_ATTACK stuff
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING &&
                                        command->selection->ShipPtr[0]->aistatecommand == 0)
                                    {
                                        //attacking and moving and HAVEN'T reached destination!
                                        bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                        ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                                    }
                                    else
                                    {
                                        // free COMMAND_ATTACK stuff
                                        FreeLastOrder(command);
                                        // attack has finished so convert it back to a formation
                                        command->ordertype.order = COMMAND_NULL;
                                    }                                     //command->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
                                }
                                else
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                                    {
                                        if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                                        if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                                        {
                                            if(command->selection->ShipPtr[0]->aistatecommand == 0)
                                            {
                                                //it is at this distance that the group of ships turns and hitails it out of there...
                                                bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                                ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                                                break;
                                            }
                                            else
                                            {
                                                ChangeOrderToHalt(command);
                                            }
                                        }
                                    }

                                }
                            }
                            else
                            {
                                aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                            command->formation.travelvel);
                                ship->shipidle = FALSE;
                                processFormationToDo(command,FALSE,FALSE);
                            }
                        }
                    }
#ifdef DEBUG_TACTICS
                    else
                    {
                        if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                        {
                            command->formation.doneInitialAttack = TRUE;
                            if (processAttackToDo(command))
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                        else
                        {
                            aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                    command->formation.travelvel);
                            ship->shipidle = FALSE;
                            processFormationToDo(command,FALSE,FALSE);
                        }
                    }
#endif
                    break;
                case COMMAND_DOCK:
                    if (command->dock.dockType & DOCK_INSTANTANEOUSLY)
                    {
                        goto processdock;
                    }
                    if ((command->dock.allDockingWithSameShip) && (!command->dock.allNearShip))
                    {
                        ship = command->selection->ShipPtr[0];
                        target = ship->dockvars.dockship;

                        if ((target == NULL) || ShipWithinDockRange(ship,target))
                        {
                            command->dock.allNearShip = TRUE;
                            goto processdock;
                        }

                        if (aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_CarTurn + AISHIP_DontFlyToObscuredPoints + AISHIP_ReturnImmedIfPointObscured,
                                command->formation.travelvel) & AISHIP_FLY_OBJECT_IN_WAY)
                        {
                            command->dock.allNearShip = TRUE;
                            goto processdock;
                        }

                        ship->shipidle = FALSE;
                        processFormationToDo(command,FALSE,FALSE);
                    }
                    else
                    {
processdock:
                        if (processDockToDo(command))
                        {
                            if (command->selection->numShips == 0)
                            {
                                // All ships have been removed from docking.  Therefore totally wipe out command
                                nextnode = curnode->next;         // dock command has finished
                                FreeCommandToDoContents(command);
                                listDeleteNode(curnode);
                                curnode = nextnode;
                                goto readyfornextnode;
                            }
                            else
                            {
                                FreeLastOrder(command);  // free COMMAND_DOCK
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                    }
                    break;

                case COMMAND_HALT:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }

                    ship = command->selection->ShipPtr[0];
                    if ((passiveAttacked) & (bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                    {
                        done = aitrackSteadyShipDriftOnly(ship);
                    }
                    else
                    {
                        done = aitrackSteadyShip(ship);
                    }
                    processFormationToDo(command,TRUE,passiveAttacked);

                    if (done)
                    {
                        FreeLastOrder(command);
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_NULL:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

//                        if (command->formation.formAroundProtectedShip)
                        if ((command->formation.formationtype == SPHERE_FORMATION) && (command->selection->numShips > 1))   // for Sphere formation, formation will
                        {                                                           // surround protected ship.
                            processFormationToDo(command,FALSE,passiveAttacked);
                        }
                        else
                        {
                            if (command->protect->numShips > 1)
                            {
                                protectShipsAvg(command->selection->ShipPtr[0],command->protect,passiveAttacked);
                                processFormationToDo(command,FALSE,passiveAttacked);
                            }
                            else
                            {
                                protectShip(command->selection->ShipPtr[0],command->protect->ShipPtr[0],passiveAttacked);
                                processFormationToDo(command,!command->protect->ShipPtr[0]->posinfo.isMoving,passiveAttacked);
                            }
                        }
                    }
                    else
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

                        if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
                        {
                            if(processHoldingPattern(command,command->selection))
                            {
                                bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                            }
                        }
                        else
                        {
                            ship = command->selection->ShipPtr[0];
                            if ((passiveAttacked) & (bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                            {
                                //if(ship->tacticstype != Evasive)
                                    aitrackSteadyShipDriftOnly(ship);
                            }
                            else
                            {
                                aitrackSteadyShip(ship);
                            }
                        }
                        if(passiveAttacked)// && command->selection->ShipPtr[0]->tacticstype == Evasive)
                            processFormationToDo(command,FALSE,passiveAttacked);
                        else
                            processFormationToDo(command,bitTest(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN) ? FALSE : TRUE,passiveAttacked);
                    }
                    break;
                case COMMAND_MP_HYPERSPACEING:
                    if(processMpHyperspaceingToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_MP_HYPERSPACEING stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;
                case COMMAND_MOVE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        //don't set to true as it controls whether or not a ship
                        //faces in movement direction
                        //passiveAttacked = TRUE;
                    }

                    if (processMoveLeaderToDo(command,passiveAttacked))
                    {
                        // reached destination, so change COMMAND_MOVE | COMMAND_IS_FORMATION to COMMAND_NULL | COMMAND_IS_FORMATION
#ifdef DEBUG_FORMATIONS
                        dbgMessage("\nReached Destination in Formation.");
#endif
                        if (universe.curPlayerPtr->PlayerMothership != NULL)
                        {
                            // Mothership can be carrier, so make sure it is really a mothership
                            if ((universe.curPlayerPtr->PlayerMothership->shiptype == Mothership) && (selShipInSelection(command->selection->ShipPtr, command->selection->numShips, universe.curPlayerPtr->PlayerMothership)))
                            {
                                speechEventFleetSpec(universe.curPlayerPtr->PlayerMothership, COMM_F_MoShip_Arrived, 0, universe.curPlayerIndex);
                            }
                        }

                        FreeLastOrder(command); // free COMMAND_MOVE stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    processFormationToDo(command,FALSE,passiveAttacked);
                    break;

                case COMMAND_SPECIAL:
                    if (processSpecialToDo(command))
                    {
                        FreeLastOrder(command);  // free COMMAND_SPECIAL
                        command->ordertype.order = COMMAND_NULL;
                    }
                    if(salCapCorvettesInSelectionCantDoFormation(command->selection))
                    {
                        processFormationToDo(command,FALSE,FALSE);
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
        else
        {
            switch (command->ordertype.order)
            {
                case COMMAND_ATTACK:
                    //open guns if in range or if we need to
                    if (processAttackToDo(command))
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                        {
                            if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                            {
                                bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                            }
                            else
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                        else
                        {
                            /*
                            nextnode = curnode->next;         // attack has finished
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                            */
                            if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                            {
                                bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                            }
                            else
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                            //command->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
                        }
                    }
                    else
                    {
                        if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                        {
                            if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                            if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                            {
                                //it is at this distance that the group of ships turns and hitails it out of there...
                                bitSet(command->selection->ShipPtr[0]->specialFlags,SPECIAL_ATTACKMOVECANCEL);
                                ChangeOrderToMove(command,command->selection->ShipPtr[0]->moveFrom,command->selection->ShipPtr[0]->moveTo);
                                break;
                            }
                        }

                    }
                    break;
                case COMMAND_MP_HYPERSPACEING:
                    if(processMpHyperspaceingToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_MOVE stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_MOVE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        //don't set to true so we face direction we're
                        //moving
                        //passiveAttacked = TRUE;
                    }

                    if (processMoveToDo(command,passiveAttacked))
                    {
                        if (universe.curPlayerPtr->PlayerMothership != NULL)
                        {
                            // Mothership can be carrier, so make sure it is really a mothership
                            if ((universe.curPlayerPtr->PlayerMothership->shiptype == Mothership) && (selShipInSelection(command->selection->ShipPtr, command->selection->numShips, universe.curPlayerPtr->PlayerMothership)))
                            {
                                speechEventFleetSpec(universe.curPlayerPtr->PlayerMothership, COMM_F_MoShip_Arrived, 0, universe.curPlayerIndex);
                            }
                        }

                        // has the probe arrived?
                        for (i = 0; i < command->selection->numShips; i++)
                        {
                            if (command->selection->ShipPtr[i]->shiptype == Probe)
                            {
                                if (command->selection->ShipPtr[i]->playerowner == universe.curPlayerPtr)
                                {
                                    speechEventFleetSpec(command->selection->ShipPtr[i], STAT_F_Probe_Arrived, 0, universe.curPlayerIndex);
                                    break;
                                }
                            }
                        }

                        if (command->ordertype.attributes & (COMMAND_IS_PROTECTING|COMMAND_IS_PASSIVEATTACKING))
                        {
                            FreeLastOrder(command);   // free COMMAND_MOVE stuff
                            command->ordertype.order = COMMAND_NULL;
                        }
                        else
                        {
                            nextnode = curnode->next;
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_DOCK:
                    if (processDockToDo(command))
                    {
                        if (command->dock.wasHarvesting)
                        {
                            dbgAssert(command->selection->numShips == 1);

                            // changing COMMAND_DOCK to COMMAND_COLLECTRESOURCE
                            FreeLastOrder(command);
                            ChangeSingleShipToCollectResource(command);
                        }
                        else
                        {
                            nextnode = curnode->next;         // dock command has finished
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_SPECIAL:
                    if (processSpecialToDo(command))
                    {
                        nextnode = curnode->next;         // special command has finished
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_COLLECTRESOURCE:
                    if (processCollectResource(command))
                    {
                        Ship *ship = command->selection->ShipPtr[0];
                        dbgAssert(command->selection->numShips >= 1);

                        nextnode = curnode->next;         // collect resource command has finished
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;

                        if ((ship->playerowner) && (ship->playerowner->PlayerMothership))
                        {
                            GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,ship->playerowner->PlayerMothership);
                        }

                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_LAUNCHSHIP:
                    if (processLaunchShipToDo(command))
                    {
                        Ship *ship;
                        Ship *receiverShip;

                        dbgAssert(command->selection->numShips == 1);

                        ship = command->selection->ShipPtr[0];
                        receiverShip = command->launchship.receiverShip;

                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;

                        GroupShip(comlayer,ship,receiverShip);

                        aiplayerShipLaunchedCallback(ship);

                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_HALT:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }

                    if (processHaltToDo(command,passiveAttacked))
                    {
                        if (command->ordertype.attributes)
                        {
                            FreeLastOrder(command);
                            command->ordertype.order = COMMAND_NULL;
                        }
                        else
                        {
                            nextnode = curnode->next;
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_MILITARYPARADE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }
                    processMilitaryParadeToDo(command,passiveAttacked);
                    break;

                case COMMAND_BUILDINGSHIP:
                    dbgAssert(command->ordertype.attributes == 0);
                    // don't allow COMMAND_IS_PASSIVEATTACKING, COMMAND_IS_PROTECTING, COMMAND_IS_FORMATION
                    if (processBuildingShipToDo(command))
                    {
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_NULL:
                    if (command->ordertype.attributes == 0)
                    {
                        // NULL command, and no attributes, so let's remove it from the command layer
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
                    {
                        //put ships in holding pattern...maintain all relevant info though (protecting and passiveattacking!)
                        nextnode = curnode->next;

                        //this function will result in command being turned into a formation flying a holding pattern
                        clHoldingPattern(comlayer,command);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }


                    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                    {
                        ProtectCommand *commandprotect = command->protect;
                        SelectCommand *commandselect = command->selection;
                        sdword numShipsToProtect = commandprotect->numShips;
                        sdword numShips = commandselect->numShips;
                        Ship *protectShipPtr;
                        sdword protectThisShip;
                        sdword i;

                        passiveAttacked = FALSE;
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

                        dbgAssert(numShipsToProtect > 0);
                        if (numShipsToProtect == 1)
                        {
                            protectShipPtr = commandprotect->ShipPtr[0];
                            for (i=0;i<numShips;i++)
                            {
                                protectShip(commandselect->ShipPtr[i],protectShipPtr,passiveAttacked);
                            }
                        }
                        else
                        {
                            if (numShips == 1)
                            {
                                // we only have 1 ship, and > 1 ships to protect, so try to stay in middle
                                protectShipsAvg(commandselect->ShipPtr[0],commandprotect,passiveAttacked);
                            }
                            else
                            {
                                protectThisShip = 0;

                                for (i=0;i<numShips;i++)
                                {
                                    protectShip(commandselect->ShipPtr[i],commandprotect->ShipPtr[protectThisShip],passiveAttacked);

                                    protectThisShip++;
                                    if (protectThisShip >= numShipsToProtect)
                                    {
                                        protectThisShip = 0;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);

                            ship = command->selection->ShipPtr[0];
                            if ((bool)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                            {
                                if(ship->tacticstype != Evasive)
                                    aitrackSteadyShipDriftOnly(ship);
                            }
                            else
                            {
                                aitrackSteadyShip(ship);
                            }
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }

        }
nextcommand:
        curnode = curnode->next;
readyfornextnode:
        ;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clPostProcess
    Description : processes all of the command layer (2nd pass), for actions such as protecting
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPostProcess(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    Node *endnode = comlayer->todolist.tail;
    CommandToDo *command;
    Node *nextnode;
    AttackCommand *attack;
    ShipPtr protectMe;
    sdword i;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;

        if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            dbgAssert(command->protect->numShips > 0);

            if (bitTest(command->protectFlags,PROTECTFLAGS_JUST_FOLLOW))
            {
                goto dontretaliate;
            }

            for (i=0;i<command->protect->numShips;i++)
            {
                protectMe = command->protect->ShipPtr[i];

                if ((protectMe->gettingrocked != NULL) && (!allianceIsShipAlly(command->selection->ShipPtr[0],protectMe->gettingrocked->playerowner)))
                {
                    if ((command->ordertype.order == COMMAND_NULL) || (command->ordertype.order == COMMAND_MOVE) || (command->ordertype.order == COMMAND_ATTACK))
                    {
                        if ((command->ordertype.attributes & COMMAND_IS_FORMATION) && (command->formation.formationtype == SPHERE_FORMATION))
                        {
                            if (i == 0)     // 0th ship is always protected by a surrounded sphere of ships, and these ships
                            {               // should stay nearby (not attack, just passive attack).
                                goto dontretaliate;
                            }
                        }

                       /*
                       if (AreAllShipsAttackCapable(command->selection))
                        {
                            attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,protectMe->gettingrocked);
                            //filter gravwell and other related ships for attack during guarding
                            makeShipsNotHaveNonCombatShipsForGuardAttack(command->selection);
                            //ChangeOrderToAttack(command,attack);
                            clAttack(&universe.mainCommandLayer,command->selection,attack);
                            memFree(attack);
                        }
                        else
                        */
                          {

//                            if (AreAnyShipsAttackCapable(command->selection))
                            MaxSelection attackingships;
                            if (MakeShipsAttackCapable((SelectCommand *)&attackingships, command->selection))
                            {
                                // just tell the ships that are attack capable to attack
                                CommandToDo *newcommand;
//                                SelectCommand *attackingships = selectDupSelection(command->selection);
                                attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,protectMe->gettingrocked);
//                                MakeShipsAttackCapable(attackingships);
                                dbgAssert(attackingships.numShips > 0);
                                makeShipsNotHaveNonCombatShipsForGuardAttack((SelectCommand *)&attackingships);
                                if(attackingships.numShips > 0)
                                {
                                    newcommand = clProtect(comlayer,(SelectCommand *)&attackingships,command->protect);
                                    //filter gravwell and other related ships for attack during guarding
                                    ChangeOrderToAttack(newcommand,attack);
                                }
                                memFree(attack);
//                                memFree(attackingships);
                            }
                        }
                        goto dontretaliate;
                    }
                }

            }
        }
dontretaliate:

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsCommand
    Description : Returns allocated command containing the ship
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship
----------------------------------------------------------------------------*/
CommandToDo *getShipAndItsCommand(CommandLayer *comlayer,ShipPtr ship)
{
#if 0
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (selShipInSelection(command->selection->ShipPtr,command->selection->numShips,ship))
        {
            dbgAssert(ship->command == command);
            return command;
        }

        curnode = curnode->next;
    }

    dbgAssert(ship->command == NULL);
    return NULL;
#else
    return ship->command;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsCommandSelection
    Description : Returns selection for allocated command containing the ship.
                  If there's no command for the ship, it'll return the formation
                  of the ship.
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship
----------------------------------------------------------------------------*/
SelectCommand *getShipAndItsCommandSelection(CommandLayer *comlayer,ShipPtr ship,bool *parade)
{
    CommandToDo *command = getShipAndItsCommand(comlayer,ship);
    SelectCommand *returnselect;
    udword sizeofselect;

    if (command != NULL)
    {
        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            *parade = TRUE;
        }
        else
        {
            *parade = FALSE;
        }
        sizeofselect = sizeofSelectCommand(command->selection->numShips);
        returnselect = memAlloc(sizeofselect,"commandselection",Pyrophoric);
        memcpy(returnselect,command->selection,sizeofselect);
    }
    else
    {
        //no command found containing this ship; return just the single ship
        returnselect = memAlloc(sizeofSelectCommand(1),"sf(shipandform)", Pyrophoric);
        returnselect->numShips = 1;
        returnselect->ShipPtr[0] = ship;
        *parade = FALSE;
    }
    return returnselect;
}

/*-----------------------------------------------------------------------------
    Name        : GetMilitaryGroupAroundShip
    Description : gets the
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *GetMilitaryGroupAroundShip(CommandLayer *comlayer,Ship *aroundShip)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            if (command->militaryParade->aroundShip == aroundShip)
            {
                return command;
            }
        }

        curnode = curnode->next;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : clPresetShipsToPosition
    Description : sets ships to correct position according to command layer
    Inputs      : comlayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPresetShipsToPosition(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            setMilitaryParade(command);
        }
        else if (command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            setFormationToDo(command);
        }

        curnode = curnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsFormationCommand
    Description : Returns allocated command containing the ship and its formation
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship and its formation
----------------------------------------------------------------------------*/
CommandToDo *getShipAndItsFormationCommand(CommandLayer *comlayer,ShipPtr ship)
{
    return ship->formationcommand;
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsFormation
    Description : Returns allocated selectcommand containing the ship and its formation
    Inputs      :
    Outputs     :
    Return      : Returns allocated selectcommand containing the ship and its formation
----------------------------------------------------------------------------*/
SelectCommand *getShipAndItsFormation(CommandLayer *comlayer,ShipPtr ship)
{
    CommandToDo *command = getShipAndItsFormationCommand(comlayer,ship);
    SelectCommand *returnselect;
    udword sizeofselect;

    if (command != NULL)
    {
        sizeofselect = sizeofSelectCommand(command->selection->numShips);
        returnselect = memAlloc(sizeofselect,"sf(shipandform)",Pyrophoric);
        memcpy(returnselect,command->selection,sizeofselect);
    }
    else
    {
        returnselect = memAlloc(sizeofSelectCommand(1),"sf(shipandform)", Pyrophoric);
        returnselect->numShips = 1;
        returnselect->ShipPtr[0] = ship;
    }

    return returnselect;
}

/*-----------------------------------------------------------------------------
    Name        : CommandInCommandLayer
    Description : returns TRUE if searchfor is in comlayer
    Inputs      : comlayer, searchfor
    Outputs     :
    Return      : returns TRUE if searchfor is in comlayer
----------------------------------------------------------------------------*/
bool CommandInCommandLayer(CommandLayer *comlayer,CommandToDo *searchfor)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        if (command == searchfor)
        {
            return TRUE;
        }

        curnode = curnode->next;
    }

    return FALSE;
}

#if BINNETLOG

void binwriteselection(udword header,SelectCommand *selection)
{
    binnetselectionMax sel;
    sdword i;

    sel.header = header;
    if (selection == NULL)
    {
        sel.numShips = 0;
    }
    else
    {
        sel.numShips = selection->numShips;
        for (i=0;i<sel.numShips;i++)
        {
            sel.ShipID[i] = selection->ShipPtr[i]->shipID.shipNumber;
        }
    }

    fwrite(&sel,sizeofbinnetselection(sel.numShips),1,netlogfile);
}

void binwriteship(Ship *ship)
{
    uword shipid = (uword)-1;

    if (ship != NULL)
    {
        shipid = ship->shipID.shipNumber;
    }

    fwrite(&shipid,sizeof(uword),1,netlogfile);
}

void binwriteanyselection(udword header,SelectAnyCommand *selection)
{
    binnetanyselectionMax sel;
    SpaceObjRotImpTarg *target;
    uword objtype;
    uword id;
    sdword i;

    sel.header = header;
    if (selection == NULL)
    {
        sel.numTargets = 0;
    }
    else
    {
        sel.numTargets = selection->numTargets;
        for (i=0;i<sel.numTargets;i++)
        {
            target = selection->TargetPtr[i];
            objtype = target->objtype;
            id = 0;

            switch (objtype)
            {
                case OBJ_ShipType:
                    id = ((Ship *)target)->shipID.shipNumber;
                    break;

                case OBJ_MissileType:
                    id = ((Missile *)target)->missileID.missileNumber;
                    break;

                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    id = ((Resource *)target)->resourceID.resourceNumber;
                    break;

                case OBJ_DerelictType:
                    id = ((Derelict *)target)->derelictID.derelictNumber;
                    break;
            }

            sel.TargetID[i] = objtype | (id << 16);
        }
    }

    fwrite(&sel,sizeofbinnetanyselection(sel.numTargets),1,netlogfile);
}

void clChecksum(void)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    SelectCommand *selection;
    binnetCmdLayerInfoMax cmd;
    udword header;
    sdword i;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        selection = command->selection;

        cmd.header = makenetcheckHeader('C','C','C','C');
        cmd.order = command->ordertype.order;
        cmd.attributes = command->ordertype.attributes;
        cmd.numShips = selection->numShips;

        for (i=0;i<cmd.numShips;i++)
        {
            cmd.ShipID[i] = selection->ShipPtr[i]->shipID.shipNumber;
        }

        fwrite(&cmd,sizeofbinnetCmdLayerInfo(cmd.numShips),1,netlogfile);

        if (cmd.attributes & COMMAND_IS_FORMATION)
        {
            header = makenetcheckHeader('F','O','R','M');
            fwrite(&header,sizeof(header),1,netlogfile);
            fwrite(&command->formation,sizeof(TypeOfFormation)+sizeof(bool),1,netlogfile);
            fwrite(&command->formation.tacticalState,44,1,netlogfile);
        }

        if (cmd.attributes & COMMAND_IS_PROTECTING)
        {
            binwriteselection(makenetcheckHeader('S','E','P','T'),command->protect);
        }

        if (cmd.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            binwriteanyselection(makenetcheckHeader('S','E','P','A'),command->attack);
        }
#if 0
        if (cmd.attributes & COMMAND_IS_ATTACKINGANDMOVING)
        {
            header = makenetcheckHeader('A','T','M','V');
            fwrite(&header,sizeof(header),1,netlogfile);
            fwrite(&command->move,sizeof(MoveCommand),1,netlogfile);
        }
#endif
        switch (cmd.order)
        {
            case COMMAND_MOVE:
                header = makenetcheckHeader('M','O','V','E');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->move,sizeof(MoveCommand),1,netlogfile);
                break;

            case COMMAND_ATTACK:
                binwriteanyselection(makenetcheckHeader('S','E','A','T'),command->attack);
                break;

            case COMMAND_DOCK:
                header = makenetcheckHeader('D','O','C','K');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->dock,sizeof(DockCommand),1,netlogfile);
                break;

            case COMMAND_LAUNCHSHIP:
                header = makenetcheckHeader('L','A','U','N');
                fwrite(&header,sizeof(header),1,netlogfile);
                binwriteship(command->launchship.receiverShip);
                break;

            case COMMAND_COLLECTRESOURCE:
                break;      // don't bother - ship already has this info

            case COMMAND_BUILDINGSHIP:
                header = makenetcheckHeader('B','U','L','D');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->buildingship,sizeof(ShipType)+sizeof(ShipRace),1,netlogfile);
                break;

            case COMMAND_SPECIAL:
                binwriteanyselection(makenetcheckHeader('S','P','E','C'),command->specialtargets);
                break;

            case COMMAND_MILITARYPARADE:
                header = makenetcheckHeader('M','I','L','P');
                fwrite(&header,sizeof(header),1,netlogfile);
                binwriteship(command->militaryParade->aroundShip);
                fwrite(&command->militaryParade->paradeType,sizeof(sdword),1,netlogfile);
                break;

            case COMMAND_MP_HYPERSPACEING:
                header = makenetcheckHeader('M','P','H','P');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->hyperspaceState,sizeof(sdword),1,netlogfile);
                break;
        }

        curnode = curnode->next;
    }
}

#endif

