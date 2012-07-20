/*=============================================================================
    Name    : Dock.c
    Purpose : Contains code for docking, and refueling/repairing

    Created 11/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "maths.h"
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "Vector.h"
#include "FastMath.h"
#include "LinkedList.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "AITrack.h"
#include "AIShip.h"
#include "CommandLayer.h"
#include "prim3d.h"
#include "render.h"
#include "Select.h"
#include "SoundEvent.h"
#include "Dock.h"
#include "UnivUpdate.h"
#include "Drone.h"
#include "ResearchShip.h"
#include "Physics.h"
#include "kgl.h"
#include "Collision.h"
#include "LaunchMgr.h"
#include "Tactics.h"
#include "SinglePlayer.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "Mothership.h"
#include "Clamp.h"
#include "SalCapCorvette.h"
#include "Tutor.h"
#include "Randy.h"
#include "AIVar.h"
#include "Alliance.h"
#include "Battle.h"
#include "DDDFrigate.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

//#define DEBUG_DOCKING

#define NOT_FINISHED_DOCKING 0
#define FINISHED_DOCKING_FLYING_BACK 1
#define FINISHED_DOCKING 2
#define PERMANENTLY_DOCKED 3
#define FINISHED_LAUNCHING_FROM_DOCKING 4
#define FINISHED_RETIREMENT             5

bool isCapitalShipStaticOrBig(ShipStaticInfo *shipstatic);

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 LATCH_TOLERANCE = 25.0f;

real32 CONEORIGIN_MINFLYSPEED = -40.0f;
real32 CONEORIGIN_TOLERANCE = 50.0f;

real32 DRONE_CONEORIGIN_MINFLYSPEED = -40.0f;
real32 DRONE_CONEORIGIN_TOLERANCE = 25.0f;

real32 BACKOUT_MINFLYSPEED = -80.0f;
real32 BACKOUT_TOLERANCE = 50.0f;

real32 FLYOUTCONE_MINFLYSPEED = -120.0f;
real32 FLYOUTCONE_TOLERANCE = 65.0f;

real32 CONEINSIDE_MINFLYSPEED = -80.0f;
real32 CONEINSIDE_TOLERANCE = 50.0f;

real32 BACKTODESTINATION_MINFLYSPEED = -40.0f;
real32 BACKTODESTINATION_TOLERANCE = 50.0f;

real32 BUSY_ADD_TO_DISTANCE = 2000.0f;
real32 BUSY_ADD_TO_DISTANCE_DEPOSITRU = 5000.0f;

real32 R1RESCONTROLLER_FIGHTERLATCHDIST = 0.0f;
real32 R1RESCONTROLLER_CORVETTELATCHDIST = 0.0f;
real32 R1RESCONTROLLER_RESOURCERLATCHDIST = 0.0f;
real32 R1RESCONTROLLER_LATCHMINTIME = 1.0f;

real32 R2RESCONTROLLER_FIGHTERLATCHDIST = 0.0f;
real32 R2RESCONTROLLER_CORVETTELATCHDIST = 0.0f;
real32 R2RESCONTROLLER_RESOURCERLATCHDIST = 0.0f;
real32 R2RESCONTROLLER_LATCHMINTIME = 1.0f;

real32 P2FUELPOD_FIGHTERLATCHDIST = 0.0f;
real32 P2FUELPOD_LATCHMINTIME = 1.0f;

real32 R1ASF_FIGHTERLATCHDIST = 0.0f;
real32 R1ASF_CORVETTELATCHDIST = 0.0f;
real32 R1ASF_LATCHMINTIME = 1.0f;

real32 R2ASF_FIGHTERLATCHDIST = 0.0f;
real32 R2ASF_CORVETTELATCHDIST = 0.0f;
real32 R2ASF_LATCHMINTIME = 1.0f;

real32 WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW = 3333.0f;

real32 R1CARRIER_LATCHMINTIME = 1.0f;
real32 R1CARRIER_REFUELREPAIRMINTIME = 1.0f;

real32 R2CARRIER_LATCHMINTIME = 1.0f;
real32 R2CARRIER_REFUELREPAIRMINTIME = 1.0f;

real32 R1MOTHERSHIP_LATCHMINTIME = 1.0f;
real32 R1MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 R2MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 P1MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 P2MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 R1REPCORV_FIGHTERLATCHDIST = 150.0f;
real32 R1REPCORV_CORVETTELATCHDIST = 300.0f;
real32 R1REPCORV_LATCHMINTIME = 3.0f;

real32 R2REPCORV_FIGHTERLATCHDIST = 150.0f;
real32 R2REPCORV_CORVETTELATCHDIST = 300.0f;
real32 R2REPCORV_LATCHMINTIME = 3.0f;

real32 R1RESCOL_FIGHTERLATCHDIST = 150.0f;
real32 R1RESCOL_LATCHDIST = 300.0f;
real32 R1RESCOL_LATCHMINTIME = 3.0f;

real32 R2RESCOL_FIGHTERLATCHDIST = 150.0f;
real32 R2RESCOL_LATCHDIST = 300.0f;
real32 R2RESCOL_LATCHMINTIME = 3.0f;

real32 JUNKYARDHQ_LATCHDIST = 300.0f;
real32 JUNKYARDHQ_LATCHMINTIME = 3.0f;

real32 JUNKYARDHQ_MUSTBENEAR_TO_DOCK = 12000.0f;

real32 DRONE_LAUNCH_VELOCITY = 300.0f;

real32 CARRIERMOTHERDOCK_DELAY_FLYINSIDE = 10.0f;
real32 CARRIERMOTHERDOCK_DELAY_FLYINSIDE_CORVETTE = 15.0f;

real32 SHIP_NEED_REFUEL_PERCENT = 0.7f;
real32 SHIP_NEED_REPAIR_PERCENT = 0.7f;

scriptEntry DockTweaks[] =
{
    makeEntry(LATCH_TOLERANCE,scriptSetReal32CB),
    makeEntry(CONEORIGIN_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(CONEORIGIN_TOLERANCE,scriptSetReal32CB),
    makeEntry(DRONE_CONEORIGIN_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(DRONE_CONEORIGIN_TOLERANCE,scriptSetReal32CB),
    makeEntry(BACKOUT_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(BACKOUT_TOLERANCE,scriptSetReal32CB),
    makeEntry(FLYOUTCONE_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(FLYOUTCONE_TOLERANCE,scriptSetReal32CB),
    makeEntry(CONEINSIDE_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(CONEINSIDE_TOLERANCE,scriptSetReal32CB),
    makeEntry(BACKTODESTINATION_MINFLYSPEED,scriptSetReal32CB),
    makeEntry(BACKTODESTINATION_TOLERANCE,scriptSetReal32CB),
    makeEntry(BUSY_ADD_TO_DISTANCE,scriptSetReal32CB),
    makeEntry(BUSY_ADD_TO_DISTANCE_DEPOSITRU,scriptSetReal32CB),

    makeEntry(R1RESCONTROLLER_FIGHTERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1RESCONTROLLER_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1RESCONTROLLER_RESOURCERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1RESCONTROLLER_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(R2RESCONTROLLER_FIGHTERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2RESCONTROLLER_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2RESCONTROLLER_RESOURCERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2RESCONTROLLER_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(R1ASF_FIGHTERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1ASF_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1ASF_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(R2ASF_FIGHTERLATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2ASF_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2ASF_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(R1REPCORV_FIGHTERLATCHDIST ,scriptSetReal32SqrCB),
    makeEntry(R1REPCORV_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R1REPCORV_LATCHMINTIME     ,scriptSetReal32CB),

    makeEntry(R2REPCORV_FIGHTERLATCHDIST ,scriptSetReal32SqrCB),
    makeEntry(R2REPCORV_CORVETTELATCHDIST,scriptSetReal32SqrCB),
    makeEntry(R2REPCORV_LATCHMINTIME     ,scriptSetReal32CB),

    makeEntry(R1CARRIER_LATCHMINTIME,scriptSetReal32CB),
    makeEntry(R1CARRIER_REFUELREPAIRMINTIME,scriptSetReal32CB),

    makeEntry(R2CARRIER_LATCHMINTIME,scriptSetReal32CB),
    makeEntry(R2CARRIER_REFUELREPAIRMINTIME,scriptSetReal32CB),

    makeEntry(R1MOTHERSHIP_LATCHMINTIME,scriptSetReal32CB),
    makeEntry(R1MOTHERSHIP_REFUELREPAIRMINTIME,scriptSetReal32CB),

    makeEntry(R2MOTHERSHIP_REFUELREPAIRMINTIME,scriptSetReal32CB),

    makeEntry(R1RESCOL_FIGHTERLATCHDIST,scriptSetReal32CB),
    makeEntry(R1RESCOL_LATCHDIST,scriptSetReal32CB),
    makeEntry(R1RESCOL_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(R2RESCOL_FIGHTERLATCHDIST,scriptSetReal32CB),
    makeEntry(R2RESCOL_LATCHDIST,scriptSetReal32CB),
    makeEntry(R2RESCOL_LATCHMINTIME,scriptSetReal32CB),

    makeEntry(JUNKYARDHQ_LATCHDIST,scriptSetReal32CB),
    makeEntry(JUNKYARDHQ_LATCHMINTIME,scriptSetReal32CB),
    makeEntry(JUNKYARDHQ_MUSTBENEAR_TO_DOCK,scriptSetReal32CB),

    makeEntry(DRONE_LAUNCH_VELOCITY,scriptSetReal32CB),

    makeEntry(CARRIERMOTHERDOCK_DELAY_FLYINSIDE,scriptSetReal32CB),
    makeEntry(CARRIERMOTHERDOCK_DELAY_FLYINSIDE_CORVETTE,scriptSetReal32CB),

    makeEntry(SHIP_NEED_REFUEL_PERCENT,scriptSetReal32CB),
    makeEntry(SHIP_NEED_REPAIR_PERCENT,scriptSetReal32CB),

    endEntry
};

/*=============================================================================
    Private functions:
=============================================================================*/

static void dockReserveDockPoint(Ship *ship,Ship *dockwith,sdword dockpointindex);
static void dockFreeDockPoint(Ship *ship,Ship *dockwith);

static void dockBusyThisShip(Ship *ship,Ship *dockwith);
static void dockUnbusyThisShip(Ship *ship);
sdword dockNeedRotationOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused);
sdword dockNeedOffsetOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused);

CommandToDo *findFormationGuardingShip(Ship *ship);

/*-----------------------------------------------------------------------------
    Name        : GetDirectionVectorOfShip
    Description : gets a direction vector of ship, given a direction (0=up,1=right,2=forward,4=down,5=left,6=back)
    Inputs      : direction, ship
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void GetDirectionVectorOfShip(vector *result,udword direction,Ship *ship)
{
    switch (direction & 3)
    {
        case 0:
            matGetVectFromMatrixCol1(*result,ship->rotinfo.coordsys);
            break;

        case 1:
            matGetVectFromMatrixCol2(*result,ship->rotinfo.coordsys);
            break;

        case 2:
        case 3:
            matGetVectFromMatrixCol3(*result,ship->rotinfo.coordsys);
            break;
    }

    if (direction & 4)
    {
        vecNegate(*result);
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockFindDockIndex
    Description : finds a dock point with given name
    Inputs      : name, dockstaticinfo
    Outputs     :
    Return      : returns an index to the found dockpoint
----------------------------------------------------------------------------*/
sdword dockFindDockIndex(char *name,DockStaticInfo *dockstaticinfo)
{
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *curdockpoint;
    sdword i;

    for (i=0,curdockpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++)
    {
        if (strcasecmp(name,curdockpoint->name) == 0)
        {
            return i;
        }
    }
    dbgFatalf(DBG_Loc,"Dock name %s not found",name);
    return -1;
}

/*-----------------------------------------------------------------------------
    Name        : dockFindDockIndex
    Description : finds a dock point with given name
    Inputs      : name, dockstaticinfo
    Outputs     :
    Return      : returns an index to the found dockstaticpoint
----------------------------------------------------------------------------*/
DockStaticPoint *dockFindDockStaticPoint(char *name,DockStaticInfo *dockstaticinfo)
{
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *curdockpoint;
    sdword i;

    for (i=0,curdockpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++)
    {
        if (strcasecmp(name,curdockpoint->name) == 0)
        {
            return curdockpoint;
        }
    }
    dbgAssert(FALSE);
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : ShipWithinDockRange
    Description : returns TRUE if ship is within docking range of target
    Inputs      : ship, target
    Outputs     :
    Return      : returns TRUE if ship is within docking range of target
----------------------------------------------------------------------------*/
bool ShipWithinDockRange(Ship *ship,Ship *target)
{
    vector diff;
    real32 distsqr;

    vecSub(diff,ship->posinfo.position,target->posinfo.position);
    distsqr = vecMagnitudeSquared(diff);

    if (distsqr < ((ShipStaticInfo *)target->staticinfo)->dockShipRange)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

Ship *FindAnotherResearchShiptoDockWith(Ship *ship)
{
    Node *objnode = universe.ShipList.head;
    Ship *dockat;
    Ship *closestDockat = NULL;
    ShipStaticInfo *dockatstatic;
    ResearchShipSpec *spec  = (ResearchShipSpec *)ship->ShipSpecifics;
    real32 timeTest = REALlyBig;
    if(spec->seed == TRUE)
    {    //Don't let seed dock with anything!
        return(NULL);
    }
    if(spec->master == TRUE)
    {
        return(NULL);  //master don't do docking
    }

    while (objnode != NULL)
    {
        dockat = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(dockat->objtype == OBJ_ShipType);

        if ((dockat->flags & SOF_Dead) == 0)
        {
            if (ship->playerowner == dockat->playerowner)
            {
                if(ship != dockat)
                {   //don't dock with same ship...its not cool
                    dockatstatic = (ShipStaticInfo *)dockat->staticinfo;
                    if(dockatstatic->shiptype == ResearchShip && dockat->shiprace == ship->shiprace)
                    {
                        spec = (ResearchShipSpec *)dockat->ShipSpecifics;
                        if(dockat->shiprace == R1)
                        {
                            if(spec->done != TRUE)
                            {
                                if(spec->seed == TRUE)
                                {
                                    spec->done = TRUE;
                                    spec->seed = FALSE;
                                    if(((ResearchShipSpec *)dockat->ShipSpecifics)->pie_plate_num == 5)
                                    {    //we need next ship to dock on TOP of this one
                                        ((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num = 0;
                                    }
                                    else
                                    {   //just add 1 to the docking ships position
                                        ((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num =
                                                                            spec->pie_plate_num + 1;
                                    }
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->seed = TRUE;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->busy_docking = TRUE;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->masterptr = ((ResearchShipSpec *)dockat->ShipSpecifics)->masterptr;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->dockordernumber = ((ResearchShipSpec *)dockat->ShipSpecifics)->dockordernumber + 1;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->dockwith = dockat;
                                    return(dockat);
                                }
                                else
                                {
                                    if(closestDockat != NULL)
                                    {
                                        if(dockat->timeCreated < timeTest)
                                        {
                                            //dock with oldest ship!
                                            closestDockat = dockat;
                                        }
                                        //otherwise, not oldest, so fegetabout it
                                    }
                                    else
                                    {
                                        closestDockat = dockat;
                                        timeTest = closestDockat->timeCreated;
                                    }
                                }

                            }
                        }
                        else
                        {
                            if(spec->done != TRUE)
                            {
                                if(spec->seed == TRUE)
                                 {
                                   if(((ResearchShipSpec *)dockat->ShipSpecifics)->pie_plate_num == 3)
                                    {    //we need next ship to dock on TOP of this one
                                        ((ResearchShipSpec *)dockat->ShipSpecifics)->done = TRUE;
                                        ((ResearchShipSpec *)dockat->ShipSpecifics)->seed = FALSE;
                                        ((ResearchShipSpec *)ship->ShipSpecifics)->seed = TRUE;
                                        ((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num = 0;
                                    }
                                    else
                                    {   //just add 1 to the docking ships position
                                        ((ResearchShipSpec *)dockat->ShipSpecifics)->pie_plate_num++;
                                    }
                                    if(dockat->flags & SOF_Slaveable)
                                    {
                                        ((ResearchShipSpec *)(dockat->slaveinfo->Master)->ShipSpecifics)->prepshipforanother++;
                                    }
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->busy_docking = TRUE;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->masterptr = ((ResearchShipSpec *)dockat->ShipSpecifics)->masterptr;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->dockordernumber = ((ResearchShipSpec *)dockat->ShipSpecifics)->dockordernumber + 1;
                                    ((ResearchShipSpec *)ship->ShipSpecifics)->dockwith = dockat;
                                    return(dockat);
                                }
                                else
                                {
                                    if(closestDockat != NULL)
                                    {
                                        if(dockat->timeCreated < timeTest)
                                        {
                                            //dock with oldest ship!
                                            closestDockat = dockat;
                                        }
                                        //otherwise, not oldest, so fegetabout it
                                    }
                                    else
                                    {
                                        closestDockat = dockat;
                                        timeTest = closestDockat->timeCreated;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        objnode = objnode->next;
    }
    if(closestDockat != NULL && ((ResearchShipSpec *)closestDockat->ShipSpecifics)->done == FALSE)
    {
        if(closestDockat->shiprace == R1)
        {
            spec = (ResearchShipSpec *)ship->ShipSpecifics;
            spec->seed = TRUE;                                  //set this one to being the main seed!
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->master = TRUE;
            bitSet(closestDockat->flags,SOF_Selectable);
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->masterptr = closestDockat;
            ((ResearchShipSpec *)ship->ShipSpecifics)->masterptr = closestDockat;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->pie_plate_num = 0;
            ((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num = 1;
            ((ResearchShipSpec *)ship->ShipSpecifics)->busy_docking = TRUE;
            ((ResearchShipSpec *)ship->ShipSpecifics)->dockordernumber = 1;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->dockordernumber = 0;
            ((ResearchShipSpec *)ship->ShipSpecifics)->dockwith = closestDockat;
            return(closestDockat);
        }
        else
        {
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->master = TRUE;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->masterptr = closestDockat;
            bitSet(closestDockat->flags,SOF_Selectable);
            ((ResearchShipSpec *)ship->ShipSpecifics)->masterptr = closestDockat;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->pie_plate_num = 1;
            ((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num = 1;
            ((ResearchShipSpec *)ship->ShipSpecifics)->seed = FALSE;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->seed = TRUE;
            ((ResearchShipSpec *)ship->ShipSpecifics)->busy_docking = TRUE;
            ((ResearchShipSpec *)ship->ShipSpecifics)->dockordernumber = 1;
            ((ResearchShipSpec *)closestDockat->ShipSpecifics)->dockordernumber = 0;
            ((ResearchShipSpec *)ship->ShipSpecifics)->dockwith = closestDockat;
            return(closestDockat);
        }
    }
    //assume that ship is on its own..hence we should make it selectable
    bitSet(ship->flags,SOF_Selectable);
    return(NULL);
}

sdword ThisShipIs(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shipclass == CLASS_Fighter || shipstatic->shiptype == ProximitySensor)
    {
        return THISSHIPIS_FIGHTER;
    }
    else if (shipstatic->shipclass == CLASS_Corvette)
    {
        return THISSHIPIS_CORVETTE;
    }
    else if (shipstatic->shiptype == ResourceCollector)
    {
        return THISSHIPIS_RESOURCER;
    }
    else
    {
        return THISSHIPIS_OTHERNONCAPITALSHIP;
    }
}

//returns TRUE if the shiptype requires a mothership in order to retire
bool shipRetireNeedsMothership(Ship *ship)
{
    if(ship->staticinfo->salvageStaticInfo != NULL)
    {
        if(ship->staticinfo->salvageStaticInfo->willFitCarrier)
        {
            //salvaging says it can fit in a carrier...so retiring can fit too
            return FALSE;
        }
        else
        {
            //needs mothership big door!
            return TRUE;
        }
    }
    else
    {
        //no salvage lights
        if(isCapitalShipStatic(ship->staticinfo))
        {
            if(ship->staticinfo->shipclass == CLASS_Mothership   ||
               ship->staticinfo->shipclass == CLASS_HeavyCruiser ||
               ship->staticinfo->shipclass == CLASS_Carrier ||
               ship->staticinfo->shipclass == CLASS_Destroyer ||
               ship->shiptype == ResourceController)
            {
                //too big
                return TRUE;
            }
        }
    }
    //lets allow it otherwise
    return FALSE;
}
bool ThisShipCanDockWith(Ship *ship,Ship *dockwith,DockType dockType)
{
    sdword thisShip;

    if (dockwith->staticinfo->canReceiveSomething)
    {
        if ((dockwith->flags & (SOF_Dead|SOF_Disabled|SOF_Crazy|SOF_Hyperspace)) == 0)
        {
            if (allianceIsShipAlly(ship,dockwith->playerowner) || bitTest(dockwith->specialFlags, SPECIAL_FriendlyStatus))
            {
                thisShip = ThisShipIs(ship->staticinfo);
                if(dockType & DOCK_FOR_RETIRE)
                {
                    if(dockwith->staticinfo->canReceiveShipsForRetire)
                    {
                        if((dockwith->shiptype == ship->shiptype))
                            return FALSE;

                        if(shipRetireNeedsMothership(ship))
                        {
                            //need a 'big' docking bay
                            if(dockwith->shiptype == Mothership)
                            {
                                return TRUE;
                            }
                        }
                        else
                        {
                            return TRUE;
                        }
                    }
                }
                else
                {
                    if (dockwith->shiptype == P2FuelPod)
                    {
                        if ((ship->shiptype == P2AdvanceSwarmer) || (ship->shiptype == P2Swarmer))
                        {
                            return TRUE;
                        }
                        return FALSE;
                    }

                    if ((dockwith->staticinfo->canReceiveTheseShips[thisShip]) &&
                        (dockwith->shiptype != ship->shiptype))   // ships of same type cannot dock
                        return TRUE;
                }
            }
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsAbleToDockWithThisShip
    Description : Makes sure that the ships in this selection are able to dock with
                  the ship dockwith.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsAbleToDockWithThisShip(SelectCommand *selection,Ship *dockwith,DockType dockType)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (!ThisShipCanDockWith(selection->ShipPtr[i],dockwith,dockType))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : FindNearestShipToDockAt
    Description : finds nearest ship to dock at
    Inputs      : ship (which wants to dock), dockType (type of docking desired)
    Outputs     :
    Return      : returns nearest ship to dock at
----------------------------------------------------------------------------*/
Ship *FindNearestShipToDockAt(Ship *ship,DockType dockType)
{
    Node *objnode = universe.ShipList.head;
    Ship *dockat;
    vector diff;
    real32 dist;
    real32 mindist = (real32)1e20;
    Ship *closestDockat = NULL;
    ShipStaticInfo *dockatstatic;
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    sdword canHandleNumShips;
    real32 busyadddist;

    if ((shipstatic->shiptype == ResearchShip) && (dockType & DOCK_FOR_RESEARCH))
    {
        return(FindAnotherResearchShiptoDockWith(ship));
    }

    while (objnode != NULL)
    {
        dockat = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(dockat->objtype == OBJ_ShipType);

        if (ThisShipCanDockWith(ship,dockat,dockType))
        {
            dockatstatic = (ShipStaticInfo *)dockat->staticinfo;

            if(!(dockat->flags & SOF_Disabled))
            {
                //ship mustn't be disabled
                CommandToDo *dockatcommand = getShipAndItsCommand(&universe.mainCommandLayer,dockat);
                if(dockatcommand != NULL)
                {
                    if(dockatcommand->ordertype.order == COMMAND_DOCK)
                    {
                        goto dockatbusydontdock;
                    }
                }
                if (((dockType & DOCK_TO_DEPOSIT_RESOURCES) && (dockatstatic->canReceiveResources)) ||
                   ((dockType & DOCK_FOR_REFUEL) && (dockatstatic->canReceiveShips) && (dockatstatic->pumpFuelRate > 0.0f)) ||
                   ((dockType & DOCK_FOR_REPAIR) && (dockatstatic->canReceiveShips) && (dockatstatic->repairOtherShipRate > 0)) ||
                   ((dockType & DOCK_FOR_BOTHREFUELANDREPAIR) && (dockatstatic->canReceiveShips) && (dockatstatic->pumpFuelRate > 0.0f) && (dockatstatic->repairOtherShipRate > 0)) ||
                   ((dockType & DOCK_PERMANENTLY) && (dockatstatic->canReceiveShipsPermanently)) ||
                   ((dockType & DOCK_FOR_RETIRE) && (dockatstatic->canReceiveShipsForRetire)))
                {
                    vecSub(diff,dockat->posinfo.position,ship->posinfo.position);
                    dist = fsqrt(vecMagnitudeSquared(diff));

                    if (dockType & DOCK_TO_DEPOSIT_RESOURCES)
                    {
                        canHandleNumShips = dockatstatic->canHandleNumShipsDepositingRU;
                        busyadddist = BUSY_ADD_TO_DISTANCE_DEPOSITRU;
                    }
                    else
                    {
                        canHandleNumShips = dockatstatic->canHandleNumShipsDocking;
                        busyadddist = BUSY_ADD_TO_DISTANCE;
                    }

                    if ((canHandleNumShips > 0) && (dockat->dockInfo->busyness >= canHandleNumShips))
                    {
                        dist += busyadddist * (dockat->dockInfo->busyness+1 - canHandleNumShips);
                           // if this ship is busy, make it seem like it is further away
                    }

                    if (dockatstatic->shiptype == JunkYardHQ)
                    {
                        if (dist > JUNKYARDHQ_MUSTBENEAR_TO_DOCK)
                        {
                            dist += REALlyBig;      // don't want to dock at junk yard HQ unless close
                        }
                    }

                    if (dockatstatic->shiptype == ResourceCollector)
                    {
                        dist += REALlyBig;      // don't want to dock at resource collectors
                    }

                    if (dist < mindist)
                    {
                        mindist = dist;
                        closestDockat = dockat;
                    }
                }
            }
        }
dockatbusydontdock:
        objnode = objnode->next;
    }

    return closestDockat;
}

/*-----------------------------------------------------------------------------
    Name        : allShipsDockingWithSameShip
    Description : returns TRUE if all ships in selection are docking with same ship
    Inputs      : selectcom
    Outputs     :
    Return      : returns TRUE if all ships in selection are docking with same ship
----------------------------------------------------------------------------*/
bool allShipsDockingWithSameShip(SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    Ship *firstship;
    Ship *ship;
    sdword i;

    dbgAssert(numShips > 0);

    firstship = (selectcom->ShipPtr[0])->dockvars.dockship;

    for (i=1;i<numShips;i++)
    {
        ship = (selectcom->ShipPtr[i])->dockvars.dockship;

        if (ship != firstship)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : InitializeDockVars
    Description : Initializes the ship->dockvars based on ship->dockvars.dockShip
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitializeDockVars(Ship *ship,Ship *dockship,DockType dockType)
{
    if (dockType & DOCK_STAY_TILL_EXPLICITLAUNCH)
    {
        ship->specialFlags |= SPECIAL_STAY_TILL_EXPLICITLAUNCH;
    }
    ship->dockvars.dockship = dockship;
    if (ship->dockvars.dockship != NULL)
    {
        dockBusyThisShip(ship,ship->dockvars.dockship);
    }
    else
    {
        ship->dockvars.busyingThisShip = NULL;
    }
    ship->dockvars.dockstate = 0;
    ship->dockvars.reserveddocking = -1;
    ship->dockvars.dockstaticpoint = NULL;
    ship->dockvars.customdockinfo = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : dockGetAppropriateTypeOfDocking
    Description : returns type of dockType based on repair/refuel state of selection of ships
    Inputs      : selectcom
    Outputs     :
    Return      : dockType to use
----------------------------------------------------------------------------*/
DockType dockGetAppropriateTypeOfDocking(SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    DockType dockType = DOCK_FOR_BOTHREFUELANDREPAIR;
    sdword shipsConsidered = 0;
    sdword shipsFuelConsidered = 0;
    bool shipsNeedRepair = FALSE;
    bool shipsNeedRefuel = FALSE;
    real32 avgfuel = 0.0f;
    real32 avghealth = 0.0f;

    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[0];
        shipstatic = ship->staticinfo;
        if (ship->shiptype == ResourceCollector)
        {
            return DOCK_FOR_REFUELREPAIR;
        }
        else if (!isCapitalShipStatic(shipstatic))
        {
            shipsConsidered++;
            avghealth += (ship->health / shipstatic->maxhealth);

            if (shipstatic->maxfuel != 0.0f)
            {
                avgfuel += (ship->fuel / shipstatic->maxfuel);
                shipsFuelConsidered++;
            }
        }
    }

    if (shipsConsidered > 0)
    {
        avghealth /= shipsConsidered;
        if (avghealth < SHIP_NEED_REPAIR_PERCENT)
        {
            shipsNeedRepair = TRUE;
        }
        if (shipsFuelConsidered > 0)
        {
            avgfuel /= shipsFuelConsidered;
            if (avgfuel < SHIP_NEED_REFUEL_PERCENT)
            {
                shipsNeedRefuel = TRUE;
            }
        }

        if (shipsNeedRepair && shipsNeedRefuel)
        {
            dockType = DOCK_FOR_BOTHREFUELANDREPAIR;
        }
        else if (shipsNeedRepair)
        {
            dockType = DOCK_FOR_REPAIR;
        }
        else if (shipsNeedRefuel)
        {
            dockType = DOCK_FOR_REFUEL;
        }
        else
        {
            dockType = DOCK_FOR_REFUELREPAIR;
        }
    }

    return dockType;
}

bool ShipIsntResearchShip(Ship *ship)
{
    if (ship->shiptype != ResearchShip)
    {
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : clDock
    Description : Command to dock
    Inputs      : selectcom, dockType
    Outputs     :
    Return      : the number of ships that are actually docking
----------------------------------------------------------------------------*/
sdword clDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    bool8 wasHarvesting = FALSE;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips;
    sdword i;
    Ship *ship;
    CollectResourceCommand collect;

    if(!(dockType & DOCK_FOR_RETIRE))
    {
        MakeShipsNonCapital(selectcom);
    }
    else
    {
        //docktype is for RETIREMENT

        MakeShipsOnlyFollowConstraints(selectcom,ShipIsntResearchShip);

        for(i=0;i<selectcom->numShips;i++)
        {
            if(selectcom->ShipPtr[i]->shiptype == ResearchShip)
            {
                //research ship
                //set it up for docking...doable only this way!
                ResearchShipMakeReadyForHyperspace(selectcom->ShipPtr[i]);
            }
            else if (selectcom->ShipPtr[i]->shiptype == DDDFrigate)
            {
                DDDFrigateMakeReadyForHyperspace(selectcom->ShipPtr[i]);
            }
        }
    }

    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_DOCK,dockwith,NULL);

    if(dockwith !=NULL)
    {
        CommandToDo *dockwithcommand = getShipAndItsCommand(&universe.mainCommandLayer,dockwith);
        if(dockwithcommand != NULL)
        {
            if(dockwithcommand->ordertype.order == COMMAND_DOCK)
            {
                if(dockType & DOCK_AT_SPECIFIC_SHIP)
                {
                    return 0;
                }
            }
        }
    }

    numShips = selectcom->numShips;
    if (numShips == 0)
    {
#ifdef DEBUG_DOCKING
        dbgMessage("\nNo Ships To Dock");
#endif
        return (numShips);
    }

    //reset forceCancelDock Parameter
    for(i=0;i<numShips;i++)
    {
        selectcom->ShipPtr[i]->forceCancelDock = FALSE;
        removeShipsFromDockingWithThisShip(selectcom->ShipPtr[i]);
    }
    if (dockType & DOCK_AT_SPECIFIC_SHIP)
    {
        if (dockwith == NULL)
        {
    #ifdef DEBUG_DOCKING
            dbgMessage("\nNo Ship to dock with");
    #endif
            return (numShips);
        }

        // Filter out ships that can't dock with dockwith
        MakeShipsAbleToDockWithThisShip(selectcom,dockwith,dockType);

        numShips = selectcom->numShips;

        if (numShips == 0)
        {
    #ifdef DEBUG_DOCKING
            dbgMessage("\nNo Ships To Dock");
    #endif
            return (numShips);
        }
    }
//report the docking order to the tactics retreat layer so
//as to register the retreat
    tacticsReportMove(comlayer,selectcom);

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            ClearProtecting(alreadycommand);
        }

        if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            ClearPassiveAttacking(alreadycommand);
        }

        if (alreadycommand->ordertype.order == COMMAND_COLLECTRESOURCE)
        {
            dbgAssert(alreadycommand->selection->numShips == 1);
            wasHarvesting = TRUE;
            collect = alreadycommand->collect;
        }
        else
        {
            FreeLastOrder(alreadycommand);

            alreadycommand->ordertype.order = COMMAND_DOCK;

            alreadycommand->dock.wasHarvesting = FALSE;
            alreadycommand->dock.formationToGoIntoAfterDockingID = 0;
            alreadycommand->dock.dockType = dockType;
            alreadycommand->dock.allNearShip = FALSE;
            alreadycommand->dock.haveFlyedFarEnoughAway = FALSE;
            for (i=0;i<numShips;i++)
            {
                ship = selectcom->ShipPtr[i];
                InitializeDockVars(ship,(dockType & DOCK_AT_SPECIFIC_SHIP) ? dockwith : FindNearestShipToDockAt(ship,dockType),dockType);
            }
            alreadycommand->dock.allDockingWithSameShip = (bool8)allShipsDockingWithSameShip(selectcom);

            InitShipsForAI(alreadycommand->selection,TRUE);

            PrepareShipsForCommand(alreadycommand,TRUE);

            return (numShips);
        }
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",0);
    InitCommandToDo(newcommand);

    if (wasHarvesting)
    {
        // copy collect information
        newcommand->collect = collect;
    }

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",0);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_DOCK;
    newcommand->ordertype.attributes = 0;

    newcommand->dock.wasHarvesting = wasHarvesting;
    newcommand->dock.formationToGoIntoAfterDockingID = 0;
    newcommand->dock.dockType = dockType;
    newcommand->dock.allNearShip = FALSE;
    newcommand->dock.haveFlyedFarEnoughAway = FALSE;
    newcommand->dock.allDockingWithSameShip = (bool8)allShipsDockingWithSameShip(selectcom);

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);

    //find ship at end so the find function filters the list properly...
    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        InitializeDockVars(ship,(dockType & DOCK_AT_SPECIFIC_SHIP) ? dockwith : FindNearestShipToDockAt(ship,dockType),dockType);
    }

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    return (numShips);
}

/*-----------------------------------------------------------------------------
    Name        : dockChangeSingleShipToDock
    Description : changes a command for a single ship to COMMAND_DOCK
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dockChangeSingleShipToDock(struct CommandToDo *command,Ship *ship,Ship *dockship,bool wasHarvesting,DockType dockType)
{
    dbgAssert(command->selection->numShips == 1);

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(command);
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(command);
    }

    command->ordertype.order = COMMAND_DOCK;

    command->dock.wasHarvesting = (bool8)wasHarvesting;
    command->dock.formationToGoIntoAfterDockingID = 0;
    command->dock.dockType = dockType;
    command->dock.allDockingWithSameShip = TRUE;
    command->dock.allNearShip = FALSE;
    command->dock.haveFlyedFarEnoughAway = FALSE;

    InitializeDockVars(ship,dockship,dockType);

    PrepareShipsForCommand(command,TRUE);
    InitShipsForAI(command->selection,TRUE);
}

void dockPrepareDroneForDocking(Ship *ship,Ship *dockship)
{
    ship->dockvars.dockship = dockship;
    ship->dockvars.busyingThisShip = NULL;
    ship->dockvars.dockstate = 0;
    ship->dockvars.dockstate2 = 0;
    ship->dockvars.dockstate3 = 0;
    ship->dockvars.reserveddocking = -1;
    ship->dockvars.dockstaticpoint = NULL;
    ship->dockvars.customdockinfo = NULL;
    InitShipAI(ship,TRUE);
}

void dockPrepareSingleShipForLaunch(Ship *ship,Ship *dockship)
{
    ship->specialFlags |= SPECIAL_Launching;
    ship->dockingship = dockship;
    ship->dockvars.dockship = dockship;
    ship->dockvars.busyingThisShip = NULL;
    ship->dockvars.reserveddocking = -1;
    ship->dockvars.dockstaticpoint = NULL;
    ship->dockvars.customdockinfo = NULL;
}

void RemoveShipFromDocking(Ship *ship)
{
    bitClear(ship->specialFlags,SPECIAL_STAY_TILL_EXPLICITLAUNCH|SPECIAL_KasCheckDoneLaunching);
    if(ship->clampInfo != NULL)
    {
        if(ship->dockingship == (Ship *)ship->clampInfo->host)
        {
            //host is docking ships...so unclamp
            //won't unclamp unless it is the right ship
            unClampObj((SpaceObjRotImpTargGuidance *)ship);
        }
    }

    if (ship->dockvars.busyingThisShip != NULL)
    {
        dockUnbusyThisShip(ship);
    }
    ship->dockingship = NULL;
    if (ship->dockvars.reserveddocking >= 0)
    {
        dockFreeDockPoint(ship,ship->dockvars.dockship);
    }
    ship->dockvars.dockship = NULL;
    ship->dockvars.dockstaticpoint = NULL;
    if (ship->dockvars.customdockinfo)
    {
        memFree(ship->dockvars.customdockinfo);
        ship->dockvars.customdockinfo = NULL;
    }
    madLinkInPostDockingShip(ship);
}

void RemoveShipFromLaunching(Ship *ship)
{
    bitClear(ship->specialFlags,SPECIAL_STAY_TILL_EXPLICITLAUNCH|SPECIAL_KasCheckDoneLaunching);
    bitClear(ship->specialFlags,SPECIAL_Launching);
    ship->dockingship = NULL;
    if (ship->dockvars.reserveddocking >= 0)
    {
        dockFreeDockPoint(ship,ship->dockvars.dockship);
    }
    ship->dockvars.dockship = NULL;
    ship->dockvars.dockstaticpoint = NULL;
    dbgAssert(ship->dockvars.customdockinfo == NULL);
    madLinkInPostDockingShip(ship);

    battleLaunchWelcomeExchange(ship);
}

void LaunchCleanup(struct CommandToDo *launchtodo)
{
    Ship *ship;

    if (launchtodo->selection->numShips == 0)
    {
        return;     // don't need to do cleanup if there are no ships (e.g. they have been removed)
    }

    ship = launchtodo->selection->ShipPtr[0];

    dbgAssert(launchtodo->selection->numShips == 1);
    RemoveShipFromLaunching(ship);
}

sdword dockNeedRotationOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused)
{
    sdword i;
    if(dockwith->staticinfo->dockOverideInfo != NULL)
    {
        for(i=0;i<dockwith->staticinfo->dockOverideInfo->numDockOverides;i++)
        {
            if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiptype == ship->shiptype)
            {
                //this is the overide slot for the ship
                if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiprace == ship->shiprace)
                {
                    //correct race?
                    if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].useNewOrientation==1)
                    {
                        //use the offset overide for this one
                        if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightNameUsed)
                        {
                            //light specific
                            if(strcmp(dockpointused->name,dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightName)==0)
                            {
                                return i;
                            }
                        }
                        else
                        {
                            //light name NOT used so this is good enough
                            return i;
                        }
                    }
                }
            }
        }
    }
    return(-1);
}
sdword dockNeedOffsetOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused)
{
    sdword i;
    if(dockwith->staticinfo->dockOverideInfo != NULL)
    {
        for(i=0;i<dockwith->staticinfo->dockOverideInfo->numDockOverides;i++)
        {
            if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiptype == ship->shiptype)
            {
                //this is the overide slot for the ship
                if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiprace == ship->shiprace)
                {
                    //correct race
                    if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].useNewOffset==1)
                    {
                        //use overide slot
                        if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightNameUsed)
                        {
                            //light specific
                            if(strcmp(dockpointused->name,dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightName)==0)
                            {
                                //light name is used and this is the one
                                return i;
                            }
                        }
                        else
                        {
                            //don't need light name
                            return i;
                        }
                    }
                }
            }
        }
    }
    return(-1);
}


/*-----------------------------------------------------------------------------
    Name        : DockCleanup
    Description : cleans up refueling state information
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DockCleanup(struct CommandToDo *docktodo)
{
    sdword i;
    SelectCommand *selection = docktodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromDocking(selection->ShipPtr[i]);
    }
    madLinkInPostDocking(docktodo,1);
}

/*-----------------------------------------------------------------------------
    Name        : dockShipRefuelsAtShip
    Description : ship ship refuels at ship dockwith
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool dockShipRefuelsAtShip(Ship *ship,Ship *dockwith)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    ShipStaticInfo *dockwithstaticinfo = (ShipStaticInfo *)dockwith->staticinfo;
    bool alldone = TRUE;

    dbgAssert(dockwithstaticinfo->canReceiveShips | dockwithstaticinfo->canReceiveResources);

    // Dump off any resource units we have
    if ((dockwithstaticinfo->canReceiveResources) && (ship->resources > 0))
    {
#ifdef DEBUG_DOCKING
        dbgMessagef("\nDepositing %d RU",ship->resources);
#endif
        ship->playerowner->resourceUnits += ship->resources;
        universe.gameStats.playerStats[ship->playerowner->playerIndex].totalResourceUnitsCollected += ship->resources;
        universe.gameStats.playerStats[ship->playerowner->playerIndex].totalResourceUnits += ship->resources;
        universe.gameStats.totalResourceUnitsCollected+=ship->resources;
        if ((singlePlayerGame) && (ship->playerowner->playerIndex == 0))
        {
            singlePlayerGameInfo.resourceUnitsCollected += ship->resources;
        }
        ship->resources = 0;
        soundEvent(ship, Ship_ResourceTransfer);
        /* is this said by the collector or the ship it docked with? */
        speechEvent(ship, STAT_ResCol_ResourcesTransferred, 0);
    }

    if (dockwithstaticinfo->repairOtherShipRate != 0.0f)
    {
        if (ship->health < shipstaticinfo->maxhealth)
        {
            if(ship->shiptype == ResourceCollector &&
               dockwithstaticinfo->repairResourceCollectorRate != 0.0f)
            {
                ship->health += dockwithstaticinfo->repairResourceCollectorRate * shipstaticinfo->maxhealth * 0.01f;
            }
            else
            {
                ship->health += dockwithstaticinfo->repairOtherShipRate * shipstaticinfo->maxhealth * 0.01f;
            }
            if (ship->health > shipstaticinfo->maxhealth)
            {
                ship->health = shipstaticinfo->maxhealth;
            }
            else
            {
                alldone = FALSE;
            }
        }
    }

    if (dockwithstaticinfo->pumpFuelRate != 0.0f)
    {
        if (ship->fuel < shipstaticinfo->maxfuel)
        {
            ship->fuel += dockwithstaticinfo->pumpFuelRate * universe.phystimeelapsed;
            if (ship->fuel > shipstaticinfo->maxfuel)
            {
                ship->fuel = shipstaticinfo->maxfuel;
            }
            else
            {
                alldone = FALSE;
            }
        }
    }
    if(ship->forceCancelDock)
    {
        //if we've been told to cancel our docking business, then
        //force this variable TRUE!
        alldone = TRUE;
    }
    return alldone;
}
bool dockFlyToConeInsideAndAlign(Ship *ship,Ship *dockwith,real32 headingTolerance,real32 dockDistance,ShipRace dockwithRace)
{
     DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
     vector coneheadingInWorldCoordSys;
     vector conepositionInWorldCoordSys;
     vector heading;
     vector destination;
     bool ok=FALSE;

     matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);
     matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
     vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);


     vecScalarMultiply(destination,coneheadingInWorldCoordSys,dockDistance);
     vecAddTo(destination,conepositionInWorldCoordSys);



     aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_DontFlyToObscuredPoints|AISHIP_IgnoreDockWithObscuredPoints|AISHIP_ReturnImmedIfPointObscured, CONEINSIDE_MINFLYSPEED,&dockwith->posinfo.velocity);

     if (MoveReachedDestinationVariable(ship,&destination,100.0f))
     {
         ok = TRUE;
     }
     if(dockwithRace==R2 && dockwith->shiptype == Mothership)
     {
        matGetVectFromMatrixCol3(heading,dockwith->rotinfo.coordsys);
     }
     else
     {
        vecScalarMultiply(heading,coneheadingInWorldCoordSys,-1.0f);
     }
     if(aitrackHeading(ship,&heading,headingTolerance))
     {
         //faceing correctly
         if(ok)
         {
             //only if we've reached the correct destination as well.
             return TRUE;
         }
     }

     return FALSE;
}
bool dockFlyToConeOrigin(Ship *ship,Ship *dockwith,real32 tolerance,real32 minflyspeed,bool pointAlongConeVector, bool faceWithDockwith)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    sdword flags = 0;
    sdword overide;

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    //matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,tolerance))
    {
        return TRUE;
    }
    else
    {
        if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
        {
            vector desiredHeading,desiredUp;
            GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,dockwith);
            GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,dockwith);
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else if (pointAlongConeVector)
        {
            vector heading;
            vecSub(heading,conepositionInWorldCoordSys,ship->posinfo.position);
            vecNormalize(&heading);
            aitrackHeading(ship,&heading,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else if(faceWithDockwith)
        {
            vector heading;
            matGetVectFromMatrixCol3(heading,dockwith->rotinfo.coordsys);
            aitrackHeading(ship,&heading,0.999f);
            flags = 0;
        }
        else
        {
            flags = AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying;
        }
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,flags,minflyspeed,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool dockFlyToConeOriginLatchIfWithin(Ship *ship,Ship *dockwith,real32 withindistsqr)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector desiredHeading;
    vector desiredUp;
    vector lefttogo;
    real32 distsqr;
    sdword overide;

    dbgAssert(dockstaticpoint->type == LatchPoint);

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        //matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);
        //vecScalarMultiply(coneheadingInWorldCoordSys,coneheadingInWorldCoordSys,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //vecAddTo(conepositionInWorldCoordSys,coneheadingInWorldCoordSys);
        //vecAddTo(conepositionInWorldCoordSys,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    if (withindistsqr == 0.0f)
    {
        goto latchimmediately;
    }


    vecSub(lefttogo,ship->posinfo.position,conepositionInWorldCoordSys);
    distsqr = vecMagnitudeSquared(lefttogo);

    if (distsqr > withindistsqr)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,AISHIP_PointInDirectionFlying,-50.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
    else
    {
latchimmediately:
        if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,LATCH_TOLERANCE))
        {
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            return TRUE;
        }
        else
        {
            if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
            {
                GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,dockwith);
                GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,dockwith);
            }
            else
            {
                GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,dockwith);
                GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,dockwith);
            }
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,0,-50.0f,&dockwith->posinfo.velocity);
            return FALSE;
        }
    }
}

bool dockFlyToConeOriginLatch(Ship *ship,Ship *dockwith)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector desiredHeading;
    vector desiredUp;
    sdword overide;

    dbgAssert(dockstaticpoint->type == LatchPoint);

    if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,dockwith);
        GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,dockwith);
    }
    else
    {
        GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,dockwith);
        GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,dockwith);
    }

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        //matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);
        //vecScalarMultiply(coneheadingInWorldCoordSys,coneheadingInWorldCoordSys,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //vecAddTo(conepositionInWorldCoordSys,coneheadingInWorldCoordSys);
        //vecAddTo(conepositionInWorldCoordSys,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset);
        //vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);


    if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,LATCH_TOLERANCE))
    {
        ship->posinfo.velocity = dockwith->posinfo.velocity;
        return TRUE;
    }
    else
    {
        aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,0,-50.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool dockBackoutOfConeLatch(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,backupDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    if ((MoveReachedDestinationVariable(ship,&destination,BACKOUT_TOLERANCE)))
    {
        return TRUE;
    }
    else
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
        return FALSE;
    }
}
bool dockBackoutOfConeLatchFacingProperly(Ship *ship,Ship *dockwith,real32 withindistsqr)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;
    vector lefttogo;
    real32 distsqr;
    vector desiredHeading;
    vector desiredUp;
    sdword overide;


    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,backupDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    vecSub(lefttogo,ship->posinfo.position,conepositionInWorldCoordSys);
    distsqr = vecMagnitudeSquared(lefttogo);

    if (distsqr > withindistsqr)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
        return FALSE;
    }
    else
    {
        if (MoveReachedDestinationVariable(ship,&destination,BACKOUT_TOLERANCE))
        {
            return TRUE;
        }
        else
        {
            if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
            {
                GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,dockwith);
                GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,dockwith);
            }
            else
            {
                GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,dockwith);
                GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,dockwith);
            }
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
            return FALSE;
        }
    }
    return FALSE;
}

bool dockFlyOutOfCone(Ship *ship,Ship *dockwith,sdword headingdirection,sdword updirection)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;
    vector distanceawayVec;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,10000.0f);
    vecAddTo(destination,conepositionInWorldCoordSys);

    vecSub(distanceawayVec,ship->posinfo.position,conepositionInWorldCoordSys);
    if (vecMagnitudeSquared(distanceawayVec) >= (backupDistance*backupDistance))
    {
        return TRUE;
    }
    else
    {
        udword flags;
        vector heading;
        vector up;

        if ((headingdirection >= 0) && (updirection >= 0))
    {
            dbgAssert(headingdirection != updirection);
            GetDirectionVectorOfShip(&heading,headingdirection,dockwith);
            GetDirectionVectorOfShip(&up,updirection,dockwith);
            aitrackHeadingAndUp(ship,&heading,&up,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else
        {
            flags = AISHIP_PointInDirectionFlying;
        }

        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,flags,0.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool dockFlyAboveMothershipDoor(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    //madLinkInGetDoorInfo(dockwith,&coordsysWS,&dest1);
    //matGetVectFromMatrixCol1(heading,coordsysWS);
    //get tracking up vector
   // matGetVectFromMatrixCol2(up,coordsysWS);
    matGetVectFromMatrixCol3(dest1,coordsysWS);
    vecScalarMultiply(dest1,dest1,-1.0f);

    vecScalarMultiply(dest1,dest1,-1800.0f);
    vecAddTo(destination,dest1);

    if(MoveReachedDestinationVariable(ship,&destination,250.0f))
    {
        return TRUE;
    }
    else
    {
        aitrackHeadingAndUp(ship,&heading,&up,1.0f);
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

#define DOOR_Stage1_Right   -3700.0f
#define DOOR_Stage1_Up  -1550.0f

bool dockFlyToBottomOfDoor1(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector dest1;
    vector heading,up,right,doorheading,doorup;
    matrix coordsysWS;
    //bool track = FALSE;
    bool dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    //madLinkInGetDoorInfo(dockwith,&coordsysWS,&dest1);
    matGetVectFromMatrixCol1(doorheading,coordsysWS);
    //get tracking up vector
    matGetVectFromMatrixCol2(doorup,coordsysWS);
    //vecScalarMultiply(up,up,-1.0f);

    vecScalarMultiply(dest1,doorheading,DOOR_Stage1_Right);
    vecAddTo(destination,dest1);
    vecScalarMultiply(dest1,up,DOOR_Stage1_Up);
    vecAddTo(destination,dest1);

    //track = aitrackHeadingAndUp(ship,&heading,&up,0.98f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);


    if(MoveReachedDestinationVariable(ship,&destination,400.0f))
    {
        dest = TRUE;
    }
    if(dest)// && track)
        return TRUE;

    return FALSE;
}

#define DOOR_Stage2_UpOffset  -2700.0f

bool dockFlyToBottomOfDoor2(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;
    bool track = FALSE;
    bool dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    //madLinkInGetDoorInfo(dockwith,&coordsysWS,&dest1);
    //matGetVectFromMatrixCol1(heading,coordsysWS);
    //get tracking up vector
    //matGetVectFromMatrixCol2(up,coordsysWS);
    //vecScalarMultiply(up,up,-1.0f);

    vecScalarMultiply(dest1,heading,DOOR_Stage2_UpOffset);
    vecAddTo(destination,dest1);

    track = aitrackHeadingAndUp(ship,&heading,&up,0.98f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);


    if(MoveReachedDestinationVariable(ship,&destination,100.0f))
    {
        dest = TRUE;
    }
    if(dest && track)
        return TRUE;

    return FALSE;
}
bool dockFlyToBottomOfDoor3(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;
    bool track = FALSE;
    bool dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    //madLinkInGetDoorInfo(dockwith,&coordsysWS,&dest1);
    //matGetVectFromMatrixCol1(heading,coordsysWS);
    //get tracking up vector
    //matGetVectFromMatrixCol2(up,coordsysWS);
    //vecScalarMultiply(up,up,-1.0f);

    vecScalarMultiply(dest1,heading,-2700.0f);
    vecAddTo(destination,dest1);

    track = aitrackHeadingAndUp(ship,&heading,&up,0.98f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);


    if(MoveReachedDestinationVariable(ship,&destination,100.0f))
    {
        dest = TRUE;
    }
    if(dest && track)
        return TRUE;

    return FALSE;
}

bool dockFlyToDoor(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector heading,up,right;
    matrix coordsysWS;
    bool track = FALSE;
    bool dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    //madLinkInGetDoorInfo(dockwith,&coordsysWS,&dest1);
    //matGetVectFromMatrixCol1(heading,coordsysWS);
    //get tracking up vector
    //matGetVectFromMatrixCol2(up,coordsysWS);
    //vecScalarMultiply(up,up,-1.0f);

    track = aitrackHeadingAndUp(ship,&heading,&up,1.0f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);


    if(MoveReachedDestinationVariable(ship,&destination,TW_R1_DOOR_DOCK_TOLERANCE))
    {
        dest = TRUE;
    }
    if(dest)
        return TRUE;

    return FALSE;
}

bool dockFlyToConeInside(Ship *ship,Ship *dockwith,real32 dockDistance)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMag;
    real32 dotprod;
    vector destination;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecSub(coneToShip,ship->posinfo.position,conepositionInWorldCoordSys);
    coneToShipMag = fsqrt(vecMagnitudeSquared(coneToShip));

    dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

    if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
    {
        return TRUE;
    }

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,dockDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    if ((MoveReachedDestinationVariable(ship,&destination,CONEINSIDE_TOLERANCE)))
    {
rinside:
        return TRUE;
    }
    else
    {
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&destination,
            AISHIP_DontFlyToObscuredPoints|AISHIP_IgnoreDockWithObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying, CONEINSIDE_MINFLYSPEED,&dockwith->posinfo.velocity)
            & AISHIP_FLY_OBJECT_IN_WAY)
        {
            goto rinside;
        }
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ClampToWithin
    Description : returns number clamped to within min and max
    Inputs      : number, min, max
    Outputs     :
    Return      : returns number clamped to within min and max
----------------------------------------------------------------------------*/
real32 ClampToWithin(real32 number,real32 min,real32 max)
{
    if (number < min)
    {
        return min;
    }
    else if (number > max)
    {
        return max;
    }
    else
    {
        return number;
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockFindNearestDockPoint
    Description : out of a selection of dock points given by dockpointindices, finds the nearest docking point for
                  ship to dockwith.  Also outputs the intermediate variables in calculation.
    Inputs      : dockpointindices, ship, dockwith
    Outputs     : outconepositionInWorldCoordSys, outconeToShip, outconeToShipMagSqr (only valid if docking point was found)
    Return      : index of nearest dock point, or -1 if not found
----------------------------------------------------------------------------*/
sdword dockFindNearestDockPoint(sdword *dockpointindices[],Ship *ship,Ship *dockwith,
                                vector *outconepositionInWorldCoordSys,vector *outconeToShip,real32 *outconeToShipMagSqr)
{
    sdword *curdockpoint = dockpointindices[0];
    sdword curdockindex;
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockInfo *dockInfo = dockwith->dockInfo;
    DockStaticInfo *dockStaticInfo = dockwithstatic->dockStaticInfo;
    real32 mindistsqr = REALlyBig;
    sdword minindex = -1;
    vector conepositionInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    DockStaticPoint *dockstaticpoint;

    dbgAssert(dockInfo);
    dbgAssert(dockStaticInfo);
    dbgAssert(dockInfo->numDockPoints == dockStaticInfo->numDockPoints);

    while (curdockpoint != NULL)
    {
        curdockindex = *curdockpoint;
        dbgAssert(curdockindex < dockStaticInfo->numDockPoints);

        if (!dockInfo->dockpoints[curdockindex].thisDockBusy)
        {
            dockstaticpoint = &dockStaticInfo->dockstaticpoints[curdockindex];

            matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
            vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

            vecSub(coneToShip,ship->posinfo.position,conepositionInWorldCoordSys);

            coneToShipMagSqr = vecMagnitudeSquared(coneToShip);

            if (coneToShipMagSqr < mindistsqr)
            {
                mindistsqr = coneToShipMagSqr;
                minindex = curdockindex;
                *outconepositionInWorldCoordSys = conepositionInWorldCoordSys;
                *outconeToShip = coneToShip;
                *outconeToShipMagSqr = coneToShipMagSqr;
            }
        }

        dockpointindices++;
        curdockpoint = *dockpointindices;
    }

    return minindex;
}

/*-----------------------------------------------------------------------------
    Name        : dockFindLaunchPoint
    Description : out of a selection of dock points given by dockpointindices, finds a launch point for
                  ship to launch from.
    Inputs      : dockpointindices, ship, dockwith
    Outputs     :
    Return      : dockpointindex if found, -1 otherwise
----------------------------------------------------------------------------*/
sdword dockFindLaunchPoint(sdword *dockpointindices[],Ship *dockwith)
{
    sdword *curlaunchpoint = dockpointindices[0];
    sdword curlaunchindex;
    DockInfo *dockInfo = dockwith->dockInfo;

    while (curlaunchpoint != NULL)
    {
        curlaunchindex = *curlaunchpoint;
        dbgAssert(curlaunchindex < dockInfo->numDockPoints);

        if (!dockInfo->dockpoints[curlaunchindex].thisDockBusy)
        {
            return curlaunchindex;
        }

        dockpointindices++;
        curlaunchpoint = *dockpointindices;
    }

    return -1;
}

/*-----------------------------------------------------------------------------
    Name        : dockFindLaunchPointRandom
    Description : out of a selection of dock points given by dockpointindices, finds a random point for
                  ship to launch from.
    Inputs      : dockpointindices, ship, dockwith
    Outputs     :
    Return      : dockpointindex if found, -1 otherwise
----------------------------------------------------------------------------*/
sdword dockFindLaunchPointRandom(sdword *dockpointindices[],Ship *dockwith,Ship *ship)
{
    sdword *curlaunchpoint = dockpointindices[0];
    sdword curlaunchindex;
    DockInfo *dockInfo = dockwith->dockInfo;
    sdword numDockPoints = dockInfo->numDockPoints;
    sdword *PotentialIndices;
    sdword foundNumberIndices = 0;
    sdword retval;

    PotentialIndices = memAlloc(numDockPoints<<2,"pi(potentialindxs)",Pyrophoric);

    while (curlaunchpoint != NULL)
    {
        curlaunchindex = *curlaunchpoint;
        dbgAssert(curlaunchindex < numDockPoints);

        if (!dockInfo->dockpoints[curlaunchindex].thisDockBusy)
        {
            //door check here... maybe check if ship has animation first?
            if(dockwith->shiprace == R1)
            {
                if(dockwith->shiptype == Mothership)
                {
                    if(strcasecmp(dockInfo->dockpoints[curlaunchindex].dockstaticpoint->name,"Big") == 0)
                    {
                        if(dockwith->madDoorStatus != MAD_STATUS_DOOR_CLOSED)
                        {
                            goto busyDockPoint;
                        }
                        else
                        {
                            ship->specialFlags |= SPECIAL_EXITING_DOOR;
                        }
                    }
                }
            }

            PotentialIndices[foundNumberIndices] = curlaunchindex;
            foundNumberIndices++;
        }

    busyDockPoint:

        dockpointindices++;
        curlaunchpoint = *dockpointindices;
    }

    if (foundNumberIndices == 0)
    {
        retval = -1;
    }
    else if (foundNumberIndices == 1)
    {
        retval = PotentialIndices[0];
    }
    else
    {
        retval = PotentialIndices[randomG(foundNumberIndices)];
    }

    memFree(PotentialIndices);

    return retval;
}

void dockUnbusyThisShip(Ship *ship)
{
    Ship *busyShip = ship->dockvars.busyingThisShip;

    busyShip->dockInfo->busyness--;
    dbgAssert(busyShip->dockInfo->busyness >= 0);
    ship->dockvars.busyingThisShip = NULL;
}

void dockBusyThisShip(Ship *ship,Ship *dockwith)
{
    ship->dockvars.busyingThisShip = dockwith;
    dockwith->dockInfo->busyness++;
}

/*-----------------------------------------------------------------------------
    Name        : dockReserveDockPoint
    Description : reserves a docking point for use
    Inputs      : ship, dockwith, dockpointindex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void dockReserveDockPoint(Ship *ship,Ship *dockwith,sdword dockpointindex)
{
    dbgAssert(dockpointindex != -1);
    dbgAssert(ship->dockvars.reserveddocking < 0);    // make sure we already haven't reserved something

    dbgAssert(ship->dockvars.dockship == dockwith);
    dbgAssert(ship->dockvars.dockstaticpoint->dockindex == dockpointindex);

    ship->dockvars.reserveddocking = (sbyte)dockpointindex;
    dbgAssert(dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy == 0);
    dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy++;

    //special case advance support frigate docking for corvettes
    if(dockwith->shiptype == AdvanceSupportFrigate)
    {
        if(dockwith->shiprace == R1)
        {
            sdword i;
            for(i = 0;i<dockwith->dockInfo->numDockPoints;i++)
            {
                if(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name[0] == 'C')
                {
                    //assume this is a corvette docking point!
                    if(dockwith->dockInfo->dockpoints[i].thisDockBusy)
                    {
                        madLinkInOpenSpecialShip(dockwith);
                        break;
                    }
                }

            }
        }
    }
    if(dockwith->shiptype == ResourceController)
    {
      if(dockwith->shiprace == R1)
      {
          sdword i;
          for(i = 0;i<dockwith->dockInfo->numDockPoints;i++)
          {
              if(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name[0] == 'C')
              {
                  //assume this is a corvette docking point!
                  if(dockwith->dockInfo->dockpoints[i].thisDockBusy)
                  {
                      madLinkInOpenSpecialShip(dockwith);
                      break;
                  }
              }

          }
      }
    }
    else if(dockwith->shiptype == P2FuelPod)
    {
        madLinkInCloseSpecialShip(dockwith);
    }

}

/*-----------------------------------------------------------------------------
    Name        : dockFreeDockPoint
    Description : frees a docking point so it can be used by other ships
    Inputs      : ship, dockwith
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void dockFreeDockPoint(Ship *ship,Ship *dockwith)
{
    sdword dockpointindex;

    dbgAssert(ship->dockvars.dockship == dockwith);

    dockpointindex = ship->dockvars.dockstaticpoint->dockindex;

    if(strcasecmp(dockwith->dockInfo->dockpoints[dockpointindex].dockstaticpoint->name,"Big") == 0)
    {
        if(dockwith->shiptype == Mothership)
        {
            if(dockwith->shiprace == R1)
            {
                madLinkInCloseDoor(dockwith);
            }
        }
    }

    dbgAssert(ship->dockvars.reserveddocking == (sbyte)dockpointindex);
    ship->dockvars.reserveddocking = -1;

    dbgAssert(dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy > 0);
    dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy--;

    if(dockwith->shiptype == AdvanceSupportFrigate)
    {
        if(dockwith->shiprace == R1)
        {
            sdword i,animate;
            animate = TRUE;
            for(i = 0;i<dockwith->dockInfo->numDockPoints;i++)
            {
                if(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name[0] == 'C')
                {
                    //assume this is a corvette docking point!
                    if(dockwith->dockInfo->dockpoints[i].thisDockBusy)
                    {
                        //one is busy
                        animate = FALSE;
                        break;
                    }
                }
            }
            if(animate)
            {
                madLinkInCloseSpecialShip(dockwith);
            }
        }
    }
    else if(dockwith->shiptype == P2FuelPod)
    {
        sdword i,animate;
        animate = TRUE;
        for(i = 0;i<dockwith->dockInfo->numDockPoints;i++)
        {
            if(dockwith->dockInfo->dockpoints[i].thisDockBusy)
            {
                //one is busy
                animate = FALSE;
                break;
            }
        }
        if(animate)
        {
            madLinkInOpenSpecialShip(dockwith);
        }
    }
    if(dockwith->shiptype == ResourceController)
    {
        if(dockwith->shiprace == R1)
        {
            sdword i,animate;
            animate = TRUE;
            for(i = 0;i<dockwith->dockInfo->numDockPoints;i++)
            {
                if(dockwith->dockInfo->dockpoints[i].dockstaticpoint->name[0] == 'C')
                {
                    //assume this is a corvette docking point!
                    if(dockwith->dockInfo->dockpoints[i].thisDockBusy)
                    {
                        //one is busy
                        animate = FALSE;
                        break;
                    }
                }
            }
            if(animate)
            {
                madLinkInCloseSpecialShip(dockwith);
            }
        }
    }


}

void dockPutShipInside(Ship *ship,Ship *dockwith)
{
    ShipsInsideMe *shipsInsideMe = dockwith->shipsInsideMe;
    InsideShip *insideShip;

    dbgAssert(shipsInsideMe);

    insideShip = memAlloc(sizeof(InsideShip),"InsideShip",0);
    insideShip->ship = ship;
    listAddNode(&shipsInsideMe->insideList,&insideShip->node,insideShip);
    lmUpdateShipsInside();
    if (ship->staticinfo->shipclass==CLASS_Fighter)
        dockwith->shipsInsideMe->FightersInsideme++;
    else if (ship->staticinfo->shipclass==CLASS_Corvette)
        dockwith->shipsInsideMe->CorvettesInsideme++;

    //check if ship is salcap
    if(ship->shiptype == SalCapCorvette)
    {
        //check if it has a target
        if(((SalCapCorvetteSpec *)ship->ShipSpecifics)->target != NULL)
        {
            SpaceObjRotImpTargGuidanceShipDerelict *temp = ((SalCapCorvetteSpec *)ship->ShipSpecifics)->target;

            //cleanup the salcap
            bitClear(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
            SalCapOrderChangedCleanUp(ship);

            //harvest the target
            salCapHarvestTarget(temp,dockwith);  //hopefully harvest the target properly!
        }
    }
}

void dockInitShipForLaunch(Ship *ship)
{
    ship->dockvars.dockstate3 = 0;
}

void dockRemoveShipFromInside(Ship *ship,Ship *dockwith)
{
    Node *node = dockwith->shipsInsideMe->insideList.head;
    InsideShip *insideShip;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);

        if (insideShip->ship == ship)
        {
            listDeleteNode(node);
            lmUpdateShipsInside();
            if (ship->staticinfo->shipclass==CLASS_Fighter)
                dockwith->shipsInsideMe->FightersInsideme--;
            else if (ship->staticinfo->shipclass==CLASS_Corvette)
                dockwith->shipsInsideMe->CorvettesInsideme--;

            return;
        }

        node = node->next;
    }
    //dbgAssert(FALSE);
    dbgMessagef("\nWarning: ship %d not inside ship %d but expected",ship->shipID.shipNumber,dockwith->shipID.shipNumber);
}

void dockPutShipOutside(Ship *ship,Ship *creator,vector *createat,udword headingdirection,udword updirection)
{
    vector heading;
    vector up;
    vector right;
    sdword i;

    dbgAssert(creator);
    if (creator->collMyBlob != NULL)
    {
        collAddSpaceObjToSpecificBlob(creator->collMyBlob,(SpaceObj *)ship);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)ship);
    }
    listAddNode(&universe.SpaceObjList,&(ship->objlink),ship);
    listAddNode(&universe.ShipList,&(ship->shiplink),ship);
    listAddNode(&universe.ImpactableList,&(ship->impactablelink),ship);

    if (creator->flags & SOF_Hide)      // if creator is hidden, ship coming out should be hidden too
        bitSet(ship->flags, SOF_Hide);
    else
        bitClear(ship->flags, SOF_Hide);

    univAddObjToRenderListIf((SpaceObj *)ship,(SpaceObj *)creator);     // add to render list if parent ship is in render list
    ship->posinfo.position = *createat;
    ship->posinfo.velocity = creator->posinfo.velocity;

    dbgAssert(headingdirection != updirection);
    GetDirectionVectorOfShip(&heading,headingdirection,creator);
    GetDirectionVectorOfShip(&up,updirection,creator);
    vecCrossProduct(right,heading,up);   // should not have to normalize because heading, up are unit vectors and angle between them is 90 degrees.
    matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&right,&heading);

    univUpdateObjRotInfo((SpaceObjRot *)ship);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i])
        {
            trailZeroLength(ship->trail[i]);
        }
    }
}

void dockMakeNewStaticInfoForMaster(Ship *master)
{
    dbgAssert(!(master->flags & SOF_StaticInfoIsDynamic));


    master->staticinfo = memAlloc(sizeof(ShipStaticInfo),"ShipStaticInfo",0);
    *((ShipStaticInfo *)(master->staticinfo)) = *GetShipStaticInfo(master->shiptype,master->shiprace);
        //asteroidStaticInfos[asteroid->asteroidtype];  // structure copy
    master->flags |= SOF_StaticInfoIsDynamic;

}

void dockFreeMasterStaticMem(Ship *master)
{
    if (master->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(master->staticinfo);
        bitClear(master->flags, SOF_StaticInfoIsDynamic);
    }
    //here regenerate static info...not yet though
    master->staticinfo = GetShipStaticInfo(master->shiptype,master->shiprace);
    //should probably disperse damages to all slaves...but not now...
    //don't envision the need to break ships apart
}

/*=============================================================================
    Docking SLAVERY code
    Global Functions for use by anyone anywhere anytime
=============================================================================*/

//#define DEBUG_SLAVE
/*-----------------------------------------------------------------------------
    Name        : dockMakeMaster
    Description : Turns the ship, master, into a slave receivable ship
    Inputs      : ship to become a master
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void dockMakeMaster(Ship *master)
{
    //Only make masters who aren't anything
    dbgAssert(master->slaveinfo == NULL);

    master->slaveinfo = memAlloc(sizeof(SlaveInfo),"SlaveInfo",0);          //MEMALLOC
    master->slaveinfo->flags = 0;
    master->slaveinfo->Master = master;             //own master
    bitSet(master->slaveinfo->flags,SF_MASTER);
    listInit(&master->slaveinfo->slaves);
    bitSet(master->flags,SOF_Slaveable);            //set to indicate involved in the slave trade
#ifdef DEBUG_SLAVE
dbgMessagef("\nSLAVE_TRADE: Just made master: %x",master);
#endif
    dockMakeNewStaticInfoForMaster(master);
}
/*-----------------------------------------------------------------------------
    Name        : dockAddSlave
    Description : Slaves 'slavetoadd' to 'master', will take slavetoadd's current
                    position, and orientation and BIND it permanently to that place relative
                    to its new 'master'
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void dockAddSlave(Ship *master, Ship *slave)
{
    vector tmpvec;
    matrix tmpmat;
    Ship *slaveslave;
    Node *slavenode;
//    udword i,num;
    //Only accept fresh non slaves ...later expand to inherit slaves and fellow slaves

    dbgAssert(bitTest(master->slaveinfo->flags,SF_MASTER));
#ifdef DEBUG_SLAVE
    dbgMessagef("\nSLAVE_TRADE: Adding Slave to Master.");
#endif

    if(slave->slaveinfo != NULL)
    {   //must be trying to add a master...if not???
        dbgAssert(bitTest(slave->slaveinfo->flags,SF_MASTER));
        slavenode = slave->slaveinfo->slaves.head;
        while(slavenode != NULL)
        {
            slaveslave = (Ship *) listGetStructOfNode(slavenode);

            slaveslave->slaveinfo->flags = 0;
            bitSet(slaveslave->slaveinfo->flags,SF_SLAVE);
            slaveslave->slaveinfo->Master = master;
            bitSet(slaveslave->flags,SOF_Slaveable);

            bitClear(slaveslave->flags,SOF_Selectable);  //make non selectable
            //bitClear(slave->flags,SOF_Targetable);

            listAddNode(&master->slaveinfo->slaves,&slaveslave->slavelink,slave);

            //Store offset with respect to MASTER's object space
            vecSub(tmpvec,slaveslave->posinfo.position,master->posinfo.position);
            matMultiplyVecByMat(&slaveslave->slaveinfo->offset,&tmpvec,&master->rotinfo.coordsys);

            //Store slaves coordsys with respect to MASTER's object space
            tmpmat = slaveslave->rotinfo.coordsys;
            matTranspose(&tmpmat);
            matMultiplyMatByMat(&slaveslave->slaveinfo->coordsys,&tmpmat,&master->rotinfo.coordsys);
            matTranspose(&slaveslave->slaveinfo->coordsys);

            slavenode = slavenode->next;
        }
    }
    else
    {
        slave->slaveinfo = memAlloc(sizeof(SlaveInfo),"SlaveInfo",0);      //MEMALLOC
    }
    slave->slaveinfo->flags = 0;
    bitSet(slave->slaveinfo->flags,SF_SLAVE);
    slave->slaveinfo->Master = master;
    bitSet(slave->flags,SOF_Slaveable);

    bitClear(slave->flags,SOF_Selectable);  //make non selectable
    //bitClear(slave->flags,SOF_Targetable);

    listAddNode(&master->slaveinfo->slaves,&slave->slavelink,slave);

    //Store offset with respect to MASTER's object space
    vecSub(tmpvec,slave->posinfo.position,master->posinfo.position);
    matMultiplyVecByMat(&slave->slaveinfo->offset,&tmpvec,&master->rotinfo.coordsys);

    //Store slaves coordsys with respect to MASTER's object space
    tmpmat = slave->rotinfo.coordsys;
    matTranspose(&tmpmat);
    matMultiplyMatByMat(&slave->slaveinfo->coordsys,&tmpmat,&master->rotinfo.coordsys);
    matTranspose(&slave->slaveinfo->coordsys);

    //MOD HEALTH...MAYBE ONLY FOR RESEARCH!
    //((ShipStaticInfo *)master->staticinfo)->staticheader.mass += ((ShipStaticInfo *)slave->staticinfo)->staticheader.mass;
    ((ShipStaticInfo *)master->staticinfo)->maxhealth += ((ShipStaticInfo *)slave->staticinfo)->maxhealth;

    ((ShipStaticInfo *)master->staticinfo)->oneOverMaxHealth = 1.0f/((ShipStaticInfo *)master->staticinfo)->maxhealth;



    master->health += slave->health;
    /*
    for(i=0;i<NUM_TURN_TYPES;i++)
    {
        if(master->slaveinfo->slaves.num > 4)
        {
            num = 4;
        }
        else
        {
            num = master->slaveinfo->slaves.num;
        }

        ((ShipStaticInfo *)master->staticinfo)->turnspeed[i] = ((ShipStaticInfo *)slave->staticinfo)->turnspeed[i]/num;
    }
    */
    //ccRemoveShip(&universe.mainCameraCommand,slave);

}

/*-----------------------------------------------------------------------------
    Name        : dockRemoveSlave
    Description : Takes a given slave and removes it from 'masters' control and
                  allows slave to go on its merry business
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void dockRemoveSlave(Ship *master, Ship *slavetoremove)
{
    dbgAssert(bitTest(master->slaveinfo->flags,SF_MASTER));   //is master a master?
    dbgAssert(slavetoremove->slaveinfo->Master == master);    //this better be a slave of this master

    listRemoveNode(&slavetoremove->slavelink);
    memFree(slavetoremove->slaveinfo);                        //MEMFREE
    slavetoremove->slaveinfo = NULL;
    bitClear(slavetoremove->flags,SOF_Slaveable);
#ifdef DEBUG_SLAVE
    dbgMessagef("\nSLAVE_TRADE: Removing Slave");
#endif
    //bitSet(slavetoremove->flags,SOF_Selectable);
    master->health-=slavetoremove->health;
}

void dockLiberateSlave(Ship *freedomSlave)
{
    dbgAssert(bitTest(freedomSlave->slaveinfo->flags,SF_SLAVE));
    dockRemoveSlave(freedomSlave->slaveinfo->Master,freedomSlave);
}

void dockCrushMaster(Ship *master)
{
    Ship *slave;
    Node *slavenode;

#ifdef DEBUG_SLAVE
    dbgMessagef("\nSLAVE_TRADE: Deleteing Master");
#endif

    dbgAssert(bitTest(master->slaveinfo->flags, SF_MASTER));

    slavenode = master->slaveinfo->slaves.head;

    while(slavenode != NULL)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);
        slavenode = slavenode->next;    //move to next since we are deleting slave next line
        dockRemoveSlave(master,slave);
    }
    dbgAssert(master->slaveinfo->slaves.num == 0);      //makes sure nothing has been left
    bitClear(master->flags, SOF_Slaveable);
    memFree(master->slaveinfo);                         //MEMFREE
    master->slaveinfo = NULL;
    dockFreeMasterStaticMem(master);
}

void dockKillSlaves(Ship *master)
{
    Ship *slave;
    Node *slavenode;

#ifdef DEBUG_SLAVE
    dbgMessagef("\nSLAVE_TRADE: Deleteing Master");
#endif

    dbgAssert(bitTest(master->slaveinfo->flags, SF_MASTER));

    slavenode = master->slaveinfo->slaves.head;

    while(slavenode != NULL)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);

        slavenode = slavenode->next;    //move to next since we are deleting slave next line
    }
}

void dockDealWithDeadSlaveable(Ship *ship)
{
#ifdef DEBUG_SLAVE
    dbgMessagef("\nSLAVE_TRADE: cleaning up dead slave/master");
#endif

    dbgAssert(bitTest(ship->flags,SOF_Slaveable));
    dbgAssert(ship->slaveinfo != NULL);
    if(bitTest(ship->slaveinfo->flags,SF_MASTER))
    {    //dead ship was a master....
        //for now, destroy hiearchy...but later reassign MAstery to first slave
        //will prove complicated!
        dockKillSlaves(ship);
        dockCrushMaster(ship);
    }
    else
    {
        dbgAssert(bitTest(ship->slaveinfo->flags,SF_SLAVE));
        dbgMessage("\nSlave Died.");
        dockRemoveSlave(ship->slaveinfo->Master, ship);
    }
}

void dockUpdateSlaves(Ship *master)
{
    Ship *slave;
    Node *slavenode;
    vector tmpvec;

    dbgAssert(bitTest(master->slaveinfo->flags, SF_MASTER));

    slavenode = master->slaveinfo->slaves.head;

    while(slavenode != NULL)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);
        //slave->posinfo.force = master->posinfo.force;
        //slave->rotinfo.torque = master->rotinfo.torque;
        //maybe set velocity, and rot speed here..but not yet...actually...yet
        //slave->posinfo.velocity = master->posinfo.velocity;
        //slave->rotinfo.rotspeed = master->rotinfo.rotspeed;
        slave->posinfo.isMoving = master->posinfo.isMoving;

        physUpdateObjPosVelShip(slave,universe.phystimeelapsed);   //Update slave finally
        vecSub(tmpvec,slave->posinfo.position,master->posinfo.position);

        //'decode' slaves location in world space with respect to masters location
        matMultiplyMatByVec(&tmpvec,&master->rotinfo.coordsys,&slave->slaveinfo->offset);
        vecAdd(slave->posinfo.position, master->posinfo.position, tmpvec);

        //'decode' slaves orientation in world space with respect to Masters.
        matMultiplyMatByMat(&slave->rotinfo.coordsys,&master->rotinfo.coordsys, &slave->slaveinfo->coordsys);

        univUpdateObjRotInfo((SpaceObjRot *)slave);

        slavenode = slavenode->next;    //move to next since we are deleting slave next line
    }
}

/*=============================================================================
    Traders JunkYardHQ Docking
=============================================================================*/

#define JUNKYARDHQ_WAITLATCHPOINT     1
#define JUNKYARDHQ_FLYTOINSIDECONE    2
#define JUNKYARDHQ_FLYTOCONEORIGIN    3
#define JUNKYARDHQ_BACKUP             4
#define JUNKYARDHQ_WAIT               5

typedef struct
{
    sdword size;
    real32 dockmisc;
    real32 turnlatchdistsqr;
    real32 latchmintime;
    real32 timestamp;
} AtJunkYardHQDockInfo;

sdword JUNKYARDHQ_Latch;

sdword *JunkYardHQLatchPoints[] = { &JUNKYARDHQ_Latch, NULL };

void JunkYardHQStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    JUNKYARDHQ_Latch = dockFindDockIndex("dawg",dockStaticInfo);
}

bool ShipDocksAtJunkYardHQ(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtJunkYardHQDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtJunkYardHQDockInfo),"AtJunkYardHQ",0);
            customdockinfo->size = sizeof(AtJunkYardHQDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            customdockinfo->latchmintime = JUNKYARDHQ_LATCHMINTIME;
            customdockinfo->turnlatchdistsqr = JUNKYARDHQ_LATCHDIST;

            ship->dockvars.dockstate2 = JUNKYARDHQ_WAITLATCHPOINT;

            // deliberately fall through to state REPCORV_WAITLATCHPOINT

        case JUNKYARDHQ_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            //maybe need to do check here to see if another ship is docking
            //with this ship if the variable (numshipscan recieve) doesn't
            //work.

            if (dockwith->specialFlags2 & SPECIAL_2_NoMoreDockingAllowed)
            {
                aitrackSteadyShip(ship);
                return FALSE;
            }

            dockpointindex = dockFindNearestDockPoint(JunkYardHQLatchPoints,ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                aitrackSteadyShip(ship);
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = JUNKYARDHQ_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = JUNKYARDHQ_FLYTOINSIDECONE;
            }
            return FALSE;

        case JUNKYARDHQ_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = JUNKYARDHQ_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case JUNKYARDHQ_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = JUNKYARDHQ_WAIT;
                if (singlePlayerGame)
                {
                    AIVar *var;
                    var = aivarCreate("DockedWJydHQ");
                    aivarValueSet(var,TRUE);
                    bitSet(dockwith->specialFlags2,SPECIAL_2_NoMoreDockingAllowed);
                }
            }
            return FALSE;

        case JUNKYARDHQ_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            ship->rotinfo.rotspeed = dockwith->rotinfo.rotspeed;
            customdockinfo = ship->dockvars.customdockinfo;
            if ( (universe.totaltimeelapsed > customdockinfo->timestamp) &&
                 ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) == 0))
            {
                ship->dockvars.dockstate2 = JUNKYARDHQ_BACKUP;
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case JUNKYARDHQ_BACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    R1/R2 Repair Corvette Docking
=============================================================================*/

#define REPCORV_WAITLATCHPOINT     1
#define REPCORV_FLYTOINSIDECONE    2
#define REPCORV_FLYTOCONEORIGIN    3
#define REPCORV_REFUEL             4
#define REPCORV_BACKUP             5
#define REPCORV_WAIT               6

typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword latchpointsindex;
    real32 turnlatchdistsqr;
    real32 latchmintime;
    real32 timestamp;
} AtRepairCorvetteDockInfo;

sdword R1REPCORV_Latch;
sdword R2REPCORV_Latch;

sdword *R1RepairCorvetteLatchPoints[] = { &R1REPCORV_Latch, NULL };
sdword *R2RepairCorvetteLatchPoints[] = { &R2REPCORV_Latch, NULL };

sdword **RepairCorvettePoints[] = { R1RepairCorvetteLatchPoints, R2RepairCorvetteLatchPoints };

void R1RepairCorvetteStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1REPCORV_Latch = dockFindDockIndex("Latch",dockStaticInfo);
}

void R2RepairCorvetteStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2REPCORV_Latch = dockFindDockIndex("Latch",dockStaticInfo);
}

bool ShipDocksAtRepairCorvette(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtRepairCorvetteDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtRepairCorvetteDockInfo),"AtRCorvDock",0);
            customdockinfo->size = sizeof(AtRepairCorvetteDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            if (dockwithstatic->shiprace == R1)
            {
                customdockinfo->latchpointsindex = 0;
                customdockinfo->latchmintime = R1REPCORV_LATCHMINTIME;
                customdockinfo->turnlatchdistsqr = (shipstatic->shipclass == CLASS_Corvette) ? R1REPCORV_CORVETTELATCHDIST : R1REPCORV_FIGHTERLATCHDIST;
            }
            else
            {
                dbgAssert(dockwithstatic->shiprace == R2);
                customdockinfo->latchpointsindex = 1;
                customdockinfo->latchmintime = R2REPCORV_LATCHMINTIME;
                customdockinfo->turnlatchdistsqr = (shipstatic->shipclass == CLASS_Corvette) ? R2REPCORV_CORVETTELATCHDIST : R2REPCORV_FIGHTERLATCHDIST;
            }

            ship->dockvars.dockstate2 = REPCORV_WAITLATCHPOINT;

            // deliberately fall through to state REPCORV_WAITLATCHPOINT

        case REPCORV_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            dockpointindex = dockFindNearestDockPoint(RepairCorvettePoints[customdockinfo->latchpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
                //aitrackSteadyShip(ship);
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = REPCORV_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = REPCORV_FLYTOINSIDECONE;
            }
            return FALSE;

        case REPCORV_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = REPCORV_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case REPCORV_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = REPCORV_REFUEL;
//                speechEvent(dockwith, STAT_RepVette_StartedRepairs, 0);
                if (battleCanChatterAtThisTime(BCE_RepairStarted, dockwith))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_RepairStarted, dockwith, SOUND_EVENT_DEFAULT);
                }
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;

        case REPCORV_REFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = REPCORV_WAIT;
            }
            return FALSE;

        case REPCORV_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (universe.totaltimeelapsed > customdockinfo->timestamp)
            {
//                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = REPCORV_BACKUP;
//                speechEvent(dockwith, STAT_RepVette_FinishedRepairs, 0);
                if (battleCanChatterAtThisTime(BCE_RepairFinished, dockwith))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_RepairFinished, dockwith, SOUND_EVENT_DEFAULT);
                }
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case REPCORV_BACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}


/*=============================================================================
    R1/R2 Resource Collector Docking
=============================================================================*/

#define RESCOL_WAITLATCHPOINT     1
#define RESCOL_FLYTOINSIDECONE    2
#define RESCOL_FLYTOCONEORIGIN    3
#define RESCOL_REFUEL             4
#define RESCOL_BACKUP             5
#define RESCOL_WAIT               6

typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword latchpointsindex;
    real32 turnlatchdistsqr;
    real32 latchmintime;
    real32 timestamp;
} AtResourceCollectorDockInfo;

sdword R1RESCOLFIGHT_Latch;
sdword R1RESCOLCORV_Latch;
sdword R2RESCOLFIGHT_Latch;
sdword R2RESCOLCORV_Latch;

sdword *R1ResourceCollectorLatchPointsFIGH[] = { &R1RESCOLFIGHT_Latch, NULL };
sdword *R1ResourceCollectorLatchPointsCORV[] = { &R1RESCOLCORV_Latch,  NULL };
sdword *R2ResourceCollectorLatchPointsFIGH[] = { &R2RESCOLFIGHT_Latch, NULL };
sdword *R2ResourceCollectorLatchPointsCORV[] = { &R1RESCOLCORV_Latch,  NULL };

sdword **ResourceCollectorPoints[] = { R1ResourceCollectorLatchPointsFIGH, R1ResourceCollectorLatchPointsCORV,R2ResourceCollectorLatchPointsFIGH,R2ResourceCollectorLatchPointsCORV };

void R1ResourceCollectorStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1RESCOLFIGHT_Latch = dockFindDockIndex("Fight0",dockStaticInfo);
    R1RESCOLCORV_Latch  = dockFindDockIndex("Corv0",dockStaticInfo);
}

void R2ResourceCollectorStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2RESCOLFIGHT_Latch = dockFindDockIndex("Fight0",dockStaticInfo);
    R2RESCOLCORV_Latch  = dockFindDockIndex("Corv0",dockStaticInfo);
}

bool ShipDocksAtResourceCollector(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtResourceCollectorDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtResourceCollectorDockInfo),"AtRColDock",0);
            customdockinfo->size = sizeof(AtResourceCollectorDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            if (dockwithstatic->shiprace == R1)
            {
                if (shipstatic->shipclass == CLASS_Corvette)
                {
                    customdockinfo->latchpointsindex = 0;
                    customdockinfo->latchmintime = R1RESCOL_LATCHMINTIME;
                    customdockinfo->turnlatchdistsqr = R1RESCOL_LATCHDIST*R1RESCOL_LATCHDIST;
                }
                else
                {
                    customdockinfo->latchpointsindex = 1;
                    customdockinfo->latchmintime = R1RESCOL_LATCHMINTIME;
                    customdockinfo->turnlatchdistsqr = R1RESCOL_LATCHDIST*R1RESCOL_LATCHDIST;
                }
            }
            else
            {
                dbgAssert(dockwithstatic->shiprace == R2);
                if (shipstatic->shipclass == CLASS_Corvette)
                {
                    customdockinfo->latchpointsindex = 2;
                    customdockinfo->latchmintime = R2RESCOL_LATCHMINTIME;
                    customdockinfo->turnlatchdistsqr = R2RESCOL_LATCHDIST*R2RESCOL_LATCHDIST;
                }
                else
                {
                    customdockinfo->latchpointsindex = 3;
                    customdockinfo->latchmintime = R2RESCOL_LATCHMINTIME;
                    customdockinfo->turnlatchdistsqr = R2RESCOL_LATCHDIST*R2RESCOL_LATCHDIST;
                }
            }

            ship->dockvars.dockstate2 = RESCOL_WAITLATCHPOINT;

            // deliberately fall through to state REPCORV_WAITLATCHPOINT

        case RESCOL_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            //maybe need to do check here to see if another ship is docking
            //with this ship if the variable (numshipscan recieve) doesn't
            //work.

            dockpointindex = dockFindNearestDockPoint(ResourceCollectorPoints[customdockinfo->latchpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
#if 0
                if(!MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW))
                {
                    //if not very close, lets keep within WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW,
                    aishipFlyToShipAvoidingObjs(ship,dockwith,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
                }
                else
                {
                    //close enough, slow down
                    aitrackSteadyShip(ship);
                }
#endif
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = RESCOL_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = RESCOL_FLYTOINSIDECONE;
            }
            return FALSE;

        case RESCOL_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = RESCOL_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case RESCOL_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = RESCOL_REFUEL;
                //speechEvent(dockwith, STAT_RepVette_StartedRepairs, 0);
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;

        case RESCOL_REFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            ship->rotinfo.rotspeed = dockwith->rotinfo.rotspeed;
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = RESCOL_WAIT;
            }
            return FALSE;

        case RESCOL_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            ship->rotinfo.rotspeed = dockwith->rotinfo.rotspeed;
            customdockinfo = ship->dockvars.customdockinfo;
            if (universe.totaltimeelapsed > customdockinfo->timestamp)
            {
//                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = RESCOL_BACKUP;
                //speechEvent(dockwith, STAT_RepVette_FinishedRepairs, 0);
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case RESCOL_BACKUP:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockBackoutOfConeLatchFacingProperly(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    R1/R2 Advance Support Frigate Docking
=============================================================================*/

/*  -defines moved to dock.h
#define ASF_WAITLATCHPOINT     1
#define ASF_FLYTOINSIDECONE    2
#define ASF_FLYTOCONEORIGIN    3
#define ASF_REFUEL             4
#define ASF_BACKUP             5
#define ASF_WAIT               6
*/
typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword latchpointsindex;
    real32 turnlatchdistsqr;
    real32 latchmintime;
    real32 timestamp;
} AtASFDockInfo;

sdword R2ASF_Fight0;
sdword R2ASF_Fight1;
sdword R2ASF_Fight2;
sdword R2ASF_Fight3;
sdword R2ASF_Fight4;
sdword R2ASF_Fight5;
sdword R2ASF_Fight6;
sdword R2ASF_Fight7;
sdword R2ASF_Fight8;
sdword R2ASF_Fight9;
sdword R2ASF_Corv0;
sdword R2ASF_Corv1;
sdword R2ASF_Corv2;
sdword R2ASF_Corv3;

sdword R1ASF_FightL0;
sdword R1ASF_FightL1;
sdword R1ASF_FightL2;
sdword R1ASF_FightL3;
sdword R1ASF_FightL4;
sdword R1ASF_FightR0;
sdword R1ASF_FightR1;
sdword R1ASF_FightR2;
sdword R1ASF_FightR3;
sdword R1ASF_FightR4;
sdword R1ASF_Corv0;
sdword R1ASF_Corv1;
sdword R1ASF_Corv2;
sdword R1ASF_Corv3;

sdword *R1ASFCorvettePoints[] = { &R1ASF_Corv0, &R1ASF_Corv1, &R1ASF_Corv2, &R1ASF_Corv3, NULL };
sdword *R1ASFFighterPoints[] = { &R1ASF_FightL0, &R1ASF_FightL1, &R1ASF_FightL2, &R1ASF_FightL3, &R1ASF_FightL4,
                                 &R1ASF_FightR0, &R1ASF_FightR1, &R1ASF_FightR2, &R1ASF_FightR3, &R1ASF_FightR4, NULL };

sdword *R2ASFCorvettePoints[] = { &R2ASF_Corv0, &R2ASF_Corv1, &R2ASF_Corv2, &R2ASF_Corv3, NULL };
sdword *R2ASFFighterPoints[] = { &R2ASF_Fight0, &R2ASF_Fight1, &R2ASF_Fight2, &R2ASF_Fight3, &R2ASF_Fight4,
                                 &R2ASF_Fight5, &R2ASF_Fight6, &R2ASF_Fight7, &R2ASF_Fight8, &R2ASF_Fight9, NULL };

sdword **ASFPoints[] = { R1ASFCorvettePoints, R1ASFFighterPoints, R2ASFCorvettePoints, R2ASFFighterPoints };

void R1ASFStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1ASF_FightL0 = dockFindDockIndex("FightL0",dockStaticInfo);
    R1ASF_FightL1 = dockFindDockIndex("FightL1",dockStaticInfo);
    R1ASF_FightL2 = dockFindDockIndex("FightL2",dockStaticInfo);
    R1ASF_FightL3 = dockFindDockIndex("FightL3",dockStaticInfo);
    R1ASF_FightL4 = dockFindDockIndex("FightL4",dockStaticInfo);
    R1ASF_FightR0 = dockFindDockIndex("FightR0",dockStaticInfo);
    R1ASF_FightR1 = dockFindDockIndex("FightR1",dockStaticInfo);
    R1ASF_FightR2 = dockFindDockIndex("FightR2",dockStaticInfo);
    R1ASF_FightR3 = dockFindDockIndex("FightR3",dockStaticInfo);
    R1ASF_FightR4 = dockFindDockIndex("FightR4",dockStaticInfo);
    R1ASF_Corv0 = dockFindDockIndex("Corv0",dockStaticInfo);
    R1ASF_Corv1 = dockFindDockIndex("Corv1",dockStaticInfo);
    R1ASF_Corv2 = dockFindDockIndex("Corv2",dockStaticInfo);
    R1ASF_Corv3 = dockFindDockIndex("Corv3",dockStaticInfo);
}

void R2ASFStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2ASF_Fight0 = dockFindDockIndex("Fight0",dockStaticInfo);
    R2ASF_Fight1 = dockFindDockIndex("Fight1",dockStaticInfo);
    R2ASF_Fight2 = dockFindDockIndex("Fight2",dockStaticInfo);
    R2ASF_Fight3 = dockFindDockIndex("Fight3",dockStaticInfo);
    R2ASF_Fight4 = dockFindDockIndex("Fight4",dockStaticInfo);
    R2ASF_Fight5 = dockFindDockIndex("Fight5",dockStaticInfo);
    R2ASF_Fight6 = dockFindDockIndex("Fight6",dockStaticInfo);
    R2ASF_Fight7 = dockFindDockIndex("Fight7",dockStaticInfo);
    R2ASF_Fight8 = dockFindDockIndex("Fight8",dockStaticInfo);
    R2ASF_Fight9 = dockFindDockIndex("Fight9",dockStaticInfo);
    R2ASF_Corv0 = dockFindDockIndex("Corv0",dockStaticInfo);
    R2ASF_Corv1 = dockFindDockIndex("Corv1",dockStaticInfo);
    R2ASF_Corv2 = dockFindDockIndex("Corv2",dockStaticInfo);
    R2ASF_Corv3 = dockFindDockIndex("Corv3",dockStaticInfo);
}

bool ShipDocksAtASF(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtASFDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtASFDockInfo),"AtASFDock",0);
            customdockinfo->size = sizeof(AtASFDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            if (dockwithstatic->shiprace == R1)
            {
                if (shipstatic->shipclass == CLASS_Corvette)
                {
                    customdockinfo->latchpointsindex = 0;
                    customdockinfo->turnlatchdistsqr = R1ASF_CORVETTELATCHDIST;
                    customdockinfo->latchmintime = R1ASF_LATCHMINTIME;
                }
                else
                {
                    dbgAssert(shipstatic->shipclass == CLASS_Fighter || shipstatic->shiptype == ProximitySensor);
                    customdockinfo->latchpointsindex = 1;
                    customdockinfo->turnlatchdistsqr = R1ASF_FIGHTERLATCHDIST;
                    customdockinfo->latchmintime = R1ASF_LATCHMINTIME;
                }
            }
            else
            {
                dbgAssert(dockwithstatic->shiprace == R2);
                if (shipstatic->shipclass == CLASS_Corvette)
                {
                    customdockinfo->latchpointsindex = 2;
                    customdockinfo->turnlatchdistsqr = R2ASF_CORVETTELATCHDIST;
                    customdockinfo->latchmintime = R2ASF_LATCHMINTIME;
                }
                else
                {
                    dbgAssert(shipstatic->shipclass == CLASS_Fighter|| shipstatic->shiptype == ProximitySensor);
                    customdockinfo->latchpointsindex = 3;
                    customdockinfo->turnlatchdistsqr = R2ASF_FIGHTERLATCHDIST;
                    customdockinfo->latchmintime = R2ASF_LATCHMINTIME;
                }
            }

            ship->dockvars.dockstate2 = ASF_WAITLATCHPOINT;

            // deliberately fall through to state ASF_WAITLATCHPOINT

        case ASF_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            dockpointindex = dockFindNearestDockPoint(ASFPoints[customdockinfo->latchpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
#if 0
                if(!MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW))
                {
                    //if not very close, lets keep within WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW,
                    aishipFlyToShipAvoidingObjs(ship,dockwith,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
                }
                else
                {
                    //close enough, slow down
                    aitrackSteadyShip(ship);
                }
#endif
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (docktodo->dock.dockType & DOCK_INSTANTANEOUSLY)
            {
                vector desiredHeading,desiredUp,desiredRight;
                vector conepositionInWorldCoordSys;

                matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
                vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

                GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,dockwith);
                GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,dockwith);
                vecCrossProduct(desiredRight,desiredHeading,desiredUp);

                ship->posinfo.position = conepositionInWorldCoordSys;
                ship->posinfo.velocity = dockwith->posinfo.velocity;
                matCreateMatFromVecs(&ship->rotinfo.coordsys,&desiredUp,&desiredRight,&desiredHeading);

                univUpdateObjRotInfo((SpaceObjRot *)ship);

                ship->dockingship = dockwith;
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = ASF_WAIT;
                return FALSE;
            }

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = ASF_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = ASF_FLYTOINSIDECONE;
            }
            return FALSE;

        case ASF_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = ASF_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case ASF_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = ASF_REFUEL;
            if (battleCanChatterAtThisTime(BCE_CapitalShipDocked, dockwith))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDocked, dockwith, SOUND_EVENT_DEFAULT);
            }
//                speechEvent(dockwith, STAT_Cap_ShipDocked, 0);
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;

        case ASF_REFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = ASF_WAIT;
            }
            return FALSE;

        case ASF_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if ((universe.totaltimeelapsed > customdockinfo->timestamp) &&
                ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) == 0))
            {
                CommandToDo *command;
                command = getShipAndItsCommand(&universe.mainCommandLayer,dockwith);
                if(command != NULL)
                {
                    if(command->ordertype.order == COMMAND_MP_HYPERSPACEING)
                    {
                        return(FALSE);
                    }
                }

                //                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = ASF_BACKUP;
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case ASF_BACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    R1/R2 Resource Controller Docking
=============================================================================*/

#define RESCON_WAITLATCHPOINT     1
#define RESCON_FLYTOINSIDECONE    2
#define RESCON_FLYTOCONEORIGIN    3
#define RESCON_REFUEL             4
#define RESCON_BACKUP             5
#define RESCON_WAIT               6

typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword latchpointsindex;
    real32 turnlatchdistsqr;
    real32 latchmintime;
    real32 timestamp;
} AtResControllerDockInfo;

sdword R1RESCONTROLLER_Res;
sdword R1RESCONTROLLER_CorvL;
sdword R1RESCONTROLLER_CorvR;
sdword R1RESCONTROLLER_FightL0;
sdword R1RESCONTROLLER_FightL1;
sdword R1RESCONTROLLER_FightL2;
sdword R1RESCONTROLLER_FightR0;
sdword R1RESCONTROLLER_FightR1;
sdword R1RESCONTROLLER_FightR2;

sdword R2RESCONTROLLER_Res;
sdword R2RESCONTROLLER_Corv0;
sdword R2RESCONTROLLER_Corv1;
sdword R2RESCONTROLLER_Fight0;
sdword R2RESCONTROLLER_Fight1;
sdword R2RESCONTROLLER_Fight2;
sdword R2RESCONTROLLER_Fight3;
sdword R2RESCONTROLLER_Fight4;
sdword R2RESCONTROLLER_Fight5;

sdword *R1ResConCorvettePoints[] = { &R1RESCONTROLLER_CorvL, &R1RESCONTROLLER_CorvR, NULL };
sdword *R1ResConFighterPoints[] = { &R1RESCONTROLLER_FightL0, &R1RESCONTROLLER_FightL1, &R1RESCONTROLLER_FightL2,
                                    &R1RESCONTROLLER_FightR0, &R1RESCONTROLLER_FightR1, &R1RESCONTROLLER_FightR2, NULL };
sdword *R1ResConResourcePoints[] = { &R1RESCONTROLLER_Res, NULL };

sdword *R2ResConCorvettePoints[] = { &R2RESCONTROLLER_Corv0, &R2RESCONTROLLER_Corv1, NULL };
sdword *R2ResConFighterPoints[] = { &R2RESCONTROLLER_Fight0, &R2RESCONTROLLER_Fight1, &R2RESCONTROLLER_Fight2,
                                    &R2RESCONTROLLER_Fight3, &R2RESCONTROLLER_Fight4, &R2RESCONTROLLER_Fight5, NULL };
sdword *R2ResConResourcePoints[] = { &R2RESCONTROLLER_Res, NULL };

sdword **ResConPoints[] = { R1ResConResourcePoints, R1ResConCorvettePoints, R1ResConFighterPoints,
                            R2ResConResourcePoints, R2ResConCorvettePoints, R2ResConFighterPoints };


real32 *R1LatchDistances[] = { &R1RESCONTROLLER_RESOURCERLATCHDIST, &R1RESCONTROLLER_CORVETTELATCHDIST, &R1RESCONTROLLER_FIGHTERLATCHDIST };
real32 *R2LatchDistances[] = { &R2RESCONTROLLER_RESOURCERLATCHDIST, &R2RESCONTROLLER_CORVETTELATCHDIST, &R2RESCONTROLLER_FIGHTERLATCHDIST };

void R1ResControllerStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1RESCONTROLLER_Res = dockFindDockIndex("Res",dockStaticInfo);
    R1RESCONTROLLER_CorvL = dockFindDockIndex("CorvL",dockStaticInfo);
    R1RESCONTROLLER_CorvR = dockFindDockIndex("CorvR",dockStaticInfo);
    R1RESCONTROLLER_FightL0 = dockFindDockIndex("FightL0",dockStaticInfo);
    R1RESCONTROLLER_FightL1 = dockFindDockIndex("FightL1",dockStaticInfo);
    R1RESCONTROLLER_FightL2 = dockFindDockIndex("FightL2",dockStaticInfo);
    R1RESCONTROLLER_FightR0 = dockFindDockIndex("FightR0",dockStaticInfo);
    R1RESCONTROLLER_FightR1 = dockFindDockIndex("FightR1",dockStaticInfo);
    R1RESCONTROLLER_FightR2 = dockFindDockIndex("FightR2",dockStaticInfo);
}

void R2ResControllerStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2RESCONTROLLER_Res = dockFindDockIndex("Res",dockStaticInfo);
    R2RESCONTROLLER_Corv0 = dockFindDockIndex("Corv0",dockStaticInfo);
    R2RESCONTROLLER_Corv1 = dockFindDockIndex("Corv1",dockStaticInfo);
    R2RESCONTROLLER_Fight0 = dockFindDockIndex("Fight0",dockStaticInfo);
    R2RESCONTROLLER_Fight1 = dockFindDockIndex("Fight1",dockStaticInfo);
    R2RESCONTROLLER_Fight2 = dockFindDockIndex("Fight2",dockStaticInfo);
    R2RESCONTROLLER_Fight3 = dockFindDockIndex("Fight3",dockStaticInfo);
    R2RESCONTROLLER_Fight4 = dockFindDockIndex("Fight4",dockStaticInfo);
    R2RESCONTROLLER_Fight5 = dockFindDockIndex("Fight5",dockStaticInfo);
}

bool ShipDocksAtResController(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtResControllerDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;
    sdword index;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtResControllerDockInfo),"AtResConDock",0);
            customdockinfo->size = sizeof(AtResControllerDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            if (shipstatic->shiptype == ResourceCollector)
            {
                index = 0;
            }
            else
            {
                if (shipstatic->shipclass == CLASS_Corvette)
                {
                    index = 1;
                }
                else if (shipstatic->shipclass == CLASS_Fighter || shipstatic->shiptype == ProximitySensor)
                {
                    index = 2;
                }
                else
                {
                    dbgAssert(FALSE);
                }
            }

            if (dockwithstatic->shiprace == R1)
            {
                customdockinfo->latchpointsindex = index;
                customdockinfo->turnlatchdistsqr = *(R1LatchDistances[index]);
                customdockinfo->latchmintime = R1RESCONTROLLER_LATCHMINTIME;
            }
            else
            {
                dbgAssert(dockwithstatic->shiprace == R2);
                customdockinfo->latchpointsindex = index + 3;
                customdockinfo->turnlatchdistsqr = *(R2LatchDistances[index]);
                customdockinfo->latchmintime = R2RESCONTROLLER_LATCHMINTIME;
            }

            ship->dockvars.dockstate2 = RESCON_WAITLATCHPOINT;

            // deliberately fall through to state RESCON_WAITLATCHPOINT

        case RESCON_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            dockpointindex = dockFindNearestDockPoint(ResConPoints[customdockinfo->latchpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
#if 0
                if(!MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW))
                {
                    //if not very close, lets keep within WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW,
                    aishipFlyToShipAvoidingObjs(ship,dockwith,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
                }
                else
                {
                    //close enough, slow down
                    aitrackSteadyShip(ship);
                }
#endif
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = RESCON_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = RESCON_FLYTOINSIDECONE;
            }
            return FALSE;

        case RESCON_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = RESCON_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case RESCON_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith,customdockinfo->turnlatchdistsqr))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = RESCON_REFUEL;
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;

        case RESCON_REFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = RESCON_WAIT;
            }
            return FALSE;

        case RESCON_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (universe.totaltimeelapsed > customdockinfo->timestamp)
            {
//                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = RESCON_BACKUP;
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case RESCON_BACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    Carrier/Mothership Docking
=============================================================================*/

typedef struct
{
    sdword size;
    real32 dockmisc;
    real32 timestamp;
    real32 giveupdockpointat;
    sdword inpointsindex;
    sdword resourcelatchpointsindex;
    real32 latchmintime;
    real32 refuelrepairmintime;
} AtCarrierDockInfo;

#define CARRIERMOTHERDOCK_FLYTOINSIDECONE       1
#define CARRIERMOTHERDOCK_FLYTOCONEORIGIN       2
#define CARRIERMOTHERDOCK_LATCHFLYTOCONEORIGIN  3
#define CARRIERMOTHERDOCK_LATCHFLYTOINSIDECONE  4
#define CARRIERMOTHERDOCK_REFUEL                5
#define CARRIERMOTHERDOCK_LATCHREFUEL           6
#define CARRIERMOTHERDOCK_LATCHBACKUP           7
#define CARRIERMOTHERDOCK_WAITFORLATCH          8
#define CARRIERMOTHERDOCK_FINDCONE              9
#define CARRIERMOTHERDOCK_LAUNCH                10
#define CARRIERMOTHERDOCK_WAIT                  11
#define CARRIERMOTHERDOCK_LATCHWAIT             12
#define CARRIERMOTHERDOCK_FOR_RETIRE            13
#define CARRIERMOTHERDOCK_IN_DOOR               14
#define CARRIERMOTHERDOCK_FLYTODOOR2            15
#define CARRIERMOTHERDOCK_FLYTODOOR3            16
#define CARRIERMOTHERDOCK_FLYTODOOR             17
#define CARRIERMOTHERDOCK_WAIT_FOR_DOOR_TO_CLOSE 18
#define CARRIERMOTHERDOCK_FLYTOCONEORIGIN_AND_ALIGN 19



sdword R1CARRIER_In0;
sdword R1CARRIER_In1;
sdword R1CARRIER_Res0;
sdword R1CARRIER_Res1;
sdword R1CARRIER_Out0;
sdword R1CARRIER_Out1;
sdword R1CARRIER_Frigate;

sdword *R1CarrierResourceLatchPoints[] = { &R1CARRIER_Res0, &R1CARRIER_Res1, NULL };
sdword *R1CarrierInConePoints[] = { &R1CARRIER_In0, &R1CARRIER_In1, NULL };
sdword *R1CarrierSmallLaunchPoints[] = { &R1CARRIER_Out0, &R1CARRIER_Out1, NULL };
sdword *R1CarrierFrigateLaunchPoints[] = { &R1CARRIER_Frigate, NULL };
sdword *R1CarrierInFrigate[] = { &R1CARRIER_Frigate, NULL };

sdword R2CARRIER_In1;
sdword R2CARRIER_In2;
sdword R2CARRIER_Out1;
sdword R2CARRIER_Out2;
sdword R2CARRIER_Frigate;
sdword R2CARRIER_Res0;
sdword R2CARRIER_Res1;

sdword *R2CarrierResourceLatchPoints[] = { &R2CARRIER_Res0, &R2CARRIER_Res1, NULL };
sdword *R2CarrierInConePoints[] = { &R2CARRIER_In1, &R2CARRIER_In2, NULL };
sdword *R2CarrierSmallLaunchPoints[] = { &R2CARRIER_Out1, &R2CARRIER_Out2, NULL };
sdword *R2CarrierFrigateLaunchPoints[] = { &R2CARRIER_Frigate, NULL };
sdword *R2CarrierInFrigate[] = { &R2CARRIER_Frigate, NULL };

sdword R1MOTHERSHIP_In;
sdword R1MOTHERSHIP_In1;
sdword R1MOTHERSHIP_Out;
sdword R1MOTHERSHIP_Out1;
sdword R1MOTHERSHIP_Frigate;
sdword R1MOTHERSHIP_Big;
sdword R1MOTHERSHIP_ResU;
sdword R1MOTHERSHIP_ResD;

sdword *R1MothershipInConePoints[] = { &R1MOTHERSHIP_In, &R1MOTHERSHIP_In1, NULL };
sdword *R1MothershipSmallLaunchPoints[] = { &R1MOTHERSHIP_Out, &R1MOTHERSHIP_Out1, NULL };
sdword *R1MothershipFrigateLaunchPoints[] = { &R1MOTHERSHIP_Frigate, NULL };
sdword *R1MothershipBigLaunchPoints[] = { &R1MOTHERSHIP_Big, NULL };
sdword *R1MothershipResourceLatchPoints[] = { &R1MOTHERSHIP_ResU, &R1MOTHERSHIP_ResD, NULL };
sdword *R1MothershipInBig[] = { &R1MOTHERSHIP_Big, NULL };
sdword *R1MothershipInResearch[] = { &R1MOTHERSHIP_Frigate, NULL };

sdword R2MOTHERSHIP_In0;
sdword R2MOTHERSHIP_In1;
sdword R2MOTHERSHIP_Out0;
sdword R2MOTHERSHIP_Out1;
sdword R2MOTHERSHIP_Big;
sdword R2MOTHERSHIP_Res0;
sdword R2MOTHERSHIP_Res1;

sdword *R2MothershipInConePoints[] = { &R2MOTHERSHIP_In0, &R2MOTHERSHIP_In1, NULL };
sdword *R2MothershipSmallLaunchPoints[] = { &R2MOTHERSHIP_Out0, &R2MOTHERSHIP_Out1, NULL };
sdword *R2MothershipCapitalLaunchPoints[] = { &R2MOTHERSHIP_Big, NULL };
sdword *R2MothershipResourceLatchPoints[] = { &R2MOTHERSHIP_Res0, &R2MOTHERSHIP_Res1, NULL };
sdword *R2MothershipInBig[] = { &R2MOTHERSHIP_Big, NULL };
sdword *R2MothershipInResearch[] = { &R2MOTHERSHIP_Big, NULL };

sdword P1MOTHERSHIP_In;
sdword P1MOTHERSHIP_Out;
sdword P1MOTHERSHIP_Out1;

sdword *P1MothershipInConePoints[] = { &P1MOTHERSHIP_In, NULL };
sdword *P1MothershipSmallLaunchPoints[] = { &P1MOTHERSHIP_Out, &P1MOTHERSHIP_Out1, NULL };

sdword P2MOTHERSHIP_In;
sdword P2MOTHERSHIP_In1;
sdword P2MOTHERSHIP_In2;
sdword P2MOTHERSHIP_In3;
sdword P2MOTHERSHIP_In4;
sdword P2MOTHERSHIP_In5;
sdword P2MOTHERSHIP_In6;
sdword P2MOTHERSHIP_In7;
sdword P2MOTHERSHIP_In8;
sdword P2MOTHERSHIP_In9;
sdword P2MOTHERSHIP_In10;
sdword P2MOTHERSHIP_In11;
sdword P2MOTHERSHIP_Out;
sdword P2MOTHERSHIP_Out1;
sdword P2MOTHERSHIP_Out2;
sdword P2MOTHERSHIP_Out3;
sdword P2MOTHERSHIP_Out4;
sdword P2MOTHERSHIP_Out5;
sdword P2MOTHERSHIP_Out6;
sdword P2MOTHERSHIP_Out7;
sdword P2MOTHERSHIP_Out8;
sdword P2MOTHERSHIP_Out9;
sdword P2MOTHERSHIP_Out10;
sdword P2MOTHERSHIP_Out11;

sdword *P2MothershipInConePoints[] = { &P2MOTHERSHIP_In, &P2MOTHERSHIP_In1, &P2MOTHERSHIP_In2, &P2MOTHERSHIP_In3, &P2MOTHERSHIP_In4, &P2MOTHERSHIP_In5,
                                        &P2MOTHERSHIP_In6, &P2MOTHERSHIP_In7, &P2MOTHERSHIP_In8, &P2MOTHERSHIP_In9, &P2MOTHERSHIP_In10, &P2MOTHERSHIP_In11, NULL };
sdword *P2MothershipSmallLaunchPoints[] = { &P2MOTHERSHIP_Out, &P2MOTHERSHIP_Out1, &P2MOTHERSHIP_Out2, &P2MOTHERSHIP_Out3, &P2MOTHERSHIP_Out4, &P2MOTHERSHIP_Out5,
                                        &P2MOTHERSHIP_Out6, &P2MOTHERSHIP_Out7, &P2MOTHERSHIP_Out8, &P2MOTHERSHIP_Out9, &P2MOTHERSHIP_Out10, &P2MOTHERSHIP_Out11, NULL };

void R1CarrierStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1CARRIER_In0 = dockFindDockIndex("In0",dockStaticInfo);
    R1CARRIER_In1 = dockFindDockIndex("In1",dockStaticInfo);
    R1CARRIER_Res0 = dockFindDockIndex("Res0",dockStaticInfo);
    R1CARRIER_Res1 = dockFindDockIndex("Res1",dockStaticInfo);
    R1CARRIER_Out0 = dockFindDockIndex("Out0",dockStaticInfo);
    R1CARRIER_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
    R1CARRIER_Frigate = dockFindDockIndex("Frigate",dockStaticInfo);
}

void R2CarrierStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2CARRIER_In1 = dockFindDockIndex("In1",dockStaticInfo);
    R2CARRIER_In2 = dockFindDockIndex("In2",dockStaticInfo);
    R2CARRIER_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
    R2CARRIER_Out2 = dockFindDockIndex("Out2",dockStaticInfo);
    R2CARRIER_Frigate = dockFindDockIndex("Frigate",dockStaticInfo);
    R2CARRIER_Res0 = dockFindDockIndex("Res0",dockStaticInfo);
    R2CARRIER_Res1 = dockFindDockIndex("Res1",dockStaticInfo);
}

void R1MothershipStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R1MOTHERSHIP_In = dockFindDockIndex("In",dockStaticInfo);
    R1MOTHERSHIP_In1 = dockFindDockIndex("In1",dockStaticInfo);
    R1MOTHERSHIP_Out = dockFindDockIndex("Out",dockStaticInfo);
    R1MOTHERSHIP_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
    R1MOTHERSHIP_Frigate = dockFindDockIndex("Frigate",dockStaticInfo);
    R1MOTHERSHIP_Big = dockFindDockIndex("Big",dockStaticInfo);
    R1MOTHERSHIP_ResU = dockFindDockIndex("ResU",dockStaticInfo);
    R1MOTHERSHIP_ResD = dockFindDockIndex("ResD",dockStaticInfo);
}

void R2MothershipStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    R2MOTHERSHIP_In0 = dockFindDockIndex("In0",dockStaticInfo);
    R2MOTHERSHIP_In1 = dockFindDockIndex("In1",dockStaticInfo);
    R2MOTHERSHIP_Out0 = dockFindDockIndex("Out0",dockStaticInfo);
    R2MOTHERSHIP_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
    R2MOTHERSHIP_Big = dockFindDockIndex("Big",dockStaticInfo);
    R2MOTHERSHIP_Res0 = dockFindDockIndex("Res0",dockStaticInfo);
    R2MOTHERSHIP_Res1 = dockFindDockIndex("Res1",dockStaticInfo);
}

void P1MothershipStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    P1MOTHERSHIP_In = dockFindDockIndex("In",dockStaticInfo);
    P1MOTHERSHIP_Out = dockFindDockIndex("Out",dockStaticInfo);
    P1MOTHERSHIP_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
}

void P2MothershipStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    P2MOTHERSHIP_In = dockFindDockIndex("In",dockStaticInfo);
    P2MOTHERSHIP_In1 = dockFindDockIndex("In1",dockStaticInfo);
    P2MOTHERSHIP_In2 = dockFindDockIndex("In2",dockStaticInfo);
    P2MOTHERSHIP_In3 = dockFindDockIndex("In3",dockStaticInfo);
    P2MOTHERSHIP_In4 = dockFindDockIndex("In4",dockStaticInfo);
    P2MOTHERSHIP_In5 = dockFindDockIndex("In5",dockStaticInfo);
    P2MOTHERSHIP_In6 = dockFindDockIndex("In6",dockStaticInfo);
    P2MOTHERSHIP_In7 = dockFindDockIndex("In7",dockStaticInfo);
    P2MOTHERSHIP_In8 = dockFindDockIndex("In8",dockStaticInfo);
    P2MOTHERSHIP_In9 = dockFindDockIndex("In9",dockStaticInfo);
    P2MOTHERSHIP_In10 = dockFindDockIndex("In10",dockStaticInfo);
    P2MOTHERSHIP_In11 = dockFindDockIndex("In11",dockStaticInfo);
    P2MOTHERSHIP_Out = dockFindDockIndex("Out",dockStaticInfo);
    P2MOTHERSHIP_Out1 = dockFindDockIndex("Out1",dockStaticInfo);
    P2MOTHERSHIP_Out2 = dockFindDockIndex("Out2",dockStaticInfo);
    P2MOTHERSHIP_Out3 = dockFindDockIndex("Out3",dockStaticInfo);
    P2MOTHERSHIP_Out4 = dockFindDockIndex("Out4",dockStaticInfo);
    P2MOTHERSHIP_Out5 = dockFindDockIndex("Out5",dockStaticInfo);
    P2MOTHERSHIP_Out6 = dockFindDockIndex("Out6",dockStaticInfo);
    P2MOTHERSHIP_Out7 = dockFindDockIndex("Out7",dockStaticInfo);
    P2MOTHERSHIP_Out8 = dockFindDockIndex("Out8",dockStaticInfo);
    P2MOTHERSHIP_Out9 = dockFindDockIndex("Out9",dockStaticInfo);
    P2MOTHERSHIP_Out10 = dockFindDockIndex("Out10",dockStaticInfo);
    P2MOTHERSHIP_Out11 = dockFindDockIndex("Out11",dockStaticInfo);
}

sdword **InPoints[] = { R1CarrierInConePoints, R2CarrierInConePoints,
                        R1MothershipInConePoints, R2MothershipInConePoints,
                        P1MothershipInConePoints, P2MothershipInConePoints,
                        R1CarrierInFrigate, R2CarrierInFrigate,
                        R1MothershipInBig, R2MothershipInBig,
                        R1MothershipInResearch, R2MothershipInResearch };

sdword **ResourceLatchPoints[] = { R1CarrierResourceLatchPoints, R2CarrierResourceLatchPoints, R1MothershipResourceLatchPoints, R2MothershipResourceLatchPoints };

#define FINDFREELAUNCHPOINT             1
#define LAUNCH_FROMLAUNCHPOINT          2
#define LAUNCH_FROMMOTHERSHIPR1DOOR1    3
#define LAUNCH_FROMMOTHERSHIPR1DOOR2    4

sdword **FindR1CarrierLaunchPoints(ShipStaticInfo *shipstatic)
{

    if(isCapitalShipStaticOrBig(shipstatic)
       ||  (shipstatic->shiptype == ResourceCollector))
    {
        return R1CarrierFrigateLaunchPoints;
    }
    else
    {
        return R1CarrierSmallLaunchPoints;
    }
    /*
    if ((shipstatic->shipclass == CLASS_Frigate)
        || (shipstatic->shiptype == SensorArray)
        || (shipstac->shiptype == ResearchShip)
        || (shipstatic->shiptype == ResourceCollector))
    {
        return R1CarrierFrigateLaunchPoints;
    }
    else
    {
        return R1CarrierSmallLaunchPoints;
    }
    */
}

sdword **FindR2CarrierLaunchPoints(ShipStaticInfo *shipstatic)
{
    if(isCapitalShipStaticOrBig(shipstatic)
       ||  (shipstatic->shiptype == ResourceCollector))
    {
        return R2CarrierFrigateLaunchPoints;
    }
    else
    {
        return R2CarrierSmallLaunchPoints;
    }
    /*if (shipstatic->shipclass == CLASS_Frigate || shipstatic->shiptype == SensorArray
        || shipstatic->shiptype == ResourceCollector
        || (shipstac->shiptype == ResearchShip)
        || (shipstatic->shiptype == ResourceCollector)
        || (shipstatic->shiptype == GravWellGenerator)
        || (shipstatic->shiptype == CloakGenerator))
    {
        return R2CarrierFrigateLaunchPoints;
    }
    else
    {
        return R2CarrierSmallLaunchPoints;
    }
    */
}

sdword **FindR1MothershipLaunchPoints(ShipStaticInfo *shipstatic)
{
    /*
    switch (shipstatic->shipclass)
    {
        case CLASS_HeavyCruiser:
        case CLASS_Carrier:
        case CLASS_Destroyer:
            return R1MothershipBigLaunchPoints;

        case CLASS_Frigate:
            if (shipstatic->shiptype == ResourceController)
            {
                return R1MothershipBigLaunchPoints;
            }
            else
            {
                return R1MothershipFrigateLaunchPoints;
            }

        default:
            return R1MothershipSmallLaunchPoints;
    }
    */
    if(isCapitalShipStatic(shipstatic))
    {
        if(shipstatic->shipclass == CLASS_Frigate ||
            shipstatic->shiptype == ResearchShip)
        {
            return R1MothershipFrigateLaunchPoints;
        }
        else
        {
            return R1MothershipBigLaunchPoints;
        }
    }
    else if(shipstatic->shiptype == SensorArray)
    {
        return R1MothershipFrigateLaunchPoints;
    }
    return R1MothershipSmallLaunchPoints;

}

sdword **FindR2MothershipLaunchPoints(ShipStaticInfo *shipstatic)
{
    if (isCapitalShipStaticOrBig(shipstatic))
    {
        return R2MothershipCapitalLaunchPoints;
    }
    else
    {
        return R2MothershipSmallLaunchPoints;
    }
}

sdword **FindP1MothershipLaunchPoints(ShipStaticInfo *shipstatic)
{
//  no capital ships can dock with Pirate 1 Mothership -- July 10/98 - Michael G.
//    if (isCapitalShipStatic(shipstatic))
//    {
//        return P1MothershipSmallLaunchPoints;
//    }
//    else
//    {
        return P1MothershipSmallLaunchPoints;
//    }
}

sdword **FindP2MothershipLaunchPoints(ShipStaticInfo *shipstatic)
{
//  no capital ships can dock with Pirate 2 Mothership -- July 10/98 - Michael G.
//    if (isCapitalShipStatic(shipstatic))
//    {
//        return P1MothershipSmallLaunchPoints;
//    }
//    else
//    {
        return P2MothershipSmallLaunchPoints;
//    }
}

//function used to determine use of big door vs small entrances...
//isn't always called for retiring..hence case where I check if the ship is retiring
bool dockRetireShipNeedsBig(Ship *ship,Ship *dockwith)
{
    if(dockwith->shiptype == Mothership)
    {
        if(dockwith->shiprace == R1)
        {
            CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
            //command is probably non-null guaranteed...but lets just be safe.
            if(command != NULL && command->dock.dockType & DOCK_FOR_RETIRE)
            {
                if(ship->staticinfo->shipclass == CLASS_Fighter ||
                   ship->staticinfo->shipclass == CLASS_Corvette ||
                   ship->staticinfo->shipclass == CLASS_Frigate ||
                   ship->shiptype == ResourceCollector)
                {
                    return FALSE;
                }
                else if(ship->shiptype == ProximitySensor ||
                        ship->shiptype == Probe ||
                        ship->shiptype == ResearchShip)
                {
                    return FALSE;
                }
                return TRUE;
            }
            else if(ship->staticinfo->shipclass == CLASS_Carrier ||
               ship->staticinfo->shipclass == CLASS_HeavyCruiser ||
               ship->staticinfo->shipclass == CLASS_Destroyer ||
               ship->shiptype == ResourceController)
            {
                //need big
                return TRUE;
            }
            return FALSE;
        }
        else if(dockwith->shiprace == R2)
        {
            if(ship->staticinfo->shipclass == CLASS_Fighter ||
               ship->staticinfo->shipclass == CLASS_Corvette)
            {
                return FALSE;
            }
            else if(ship->shiptype == ProximitySensor ||
                    ship->shiptype == Probe)
            {
                return FALSE;
            }
            /*
            if(ship->staticinfo->shipclass == CLASS_Carrier ||
               ship->staticinfo->shipclass == CLASS_HeavyCruiser ||
               ship->staticinfo->shipclass == CLASS_Destroyer ||
               ship->staticinfo->shipclass == CLASS_Frigate ||
               ship->shiptype == ResourceController ||
               ship->shiptype == SensorArray ||
               ship->shiptype == GravWellGenerator ||
               ship->shiptype == CloakGenerator)
            {
                //need big
                return TRUE;
            }*/

        }
        return TRUE;
    }
    else
    {
        //carrier
        if(ship->staticinfo->shipclass == CLASS_Fighter ||
           ship->staticinfo->shipclass == CLASS_Corvette)
        {
            return FALSE;
        }
        else if(ship->shiptype == ProximitySensor ||
                ship->shiptype == Probe)
        {
            return FALSE;
        }
    }
    return TRUE;
}

sdword **GetLaunchPoints(ShipStaticInfo *shipstatic,ShipStaticInfo *dockwithstatic)
{
    if (dockwithstatic->shiptype == Carrier)
    {
        if (dockwithstatic->shiprace == R1)
        {
            return FindR1CarrierLaunchPoints(shipstatic);
        }
        else if (dockwithstatic->shiprace == R2)
        {
            return FindR2CarrierLaunchPoints(shipstatic);
        }
    }
    else
    {
        if (dockwithstatic->shiprace == R1)
        {
            if (dockwithstatic->shiptype == Mothership);
                return FindR1MothershipLaunchPoints(shipstatic);
        }
        else
        if (dockwithstatic->shiprace == R2)
        {
            if (dockwithstatic->shiptype == Mothership);
                return FindR2MothershipLaunchPoints(shipstatic);
        }
        else
        if (dockwithstatic->shiprace == P1)
        {
            if (dockwithstatic->shiptype == P1Mothership);
                return FindP1MothershipLaunchPoints(shipstatic);
        }
        else if (dockwithstatic->shiprace == P2);
        {
            if (dockwithstatic->shiptype == P2Mothership);
                return FindP2MothershipLaunchPoints(shipstatic);
        }
    }
    return NULL;
}

bool LaunchShipFromCarrierMother(Ship *ship,Ship *dockwith)
{
    sdword launchpointindex,overide;
    DockStaticPoint *dockstaticpoint;
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    ShipStaticInfo *shipstatic;
    vector conepositionInWorldCoordSys;
    vector heading,up,right;
    matrix coordsysWS;

    switch (ship->dockvars.dockstate3)
    {
        case 0:
            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            ship->dockvars.launchpoints = GetLaunchPoints(shipstatic,dockwithstatic);
            dbgAssert(ship->dockvars.launchpoints);
            ship->dockvars.dockstate3 = FINDFREELAUNCHPOINT;
            // deliberately fall through to FINDFREELAUNCHPOINT

        case FINDFREELAUNCHPOINT:
            dbgAssert(ship->dockvars.launchpoints);
            launchpointindex = dockFindLaunchPointRandom(ship->dockvars.launchpoints,dockwith,ship);

            if (launchpointindex == -1)
            {
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[launchpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,launchpointindex);

            matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
            vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

            dockRemoveShipFromInside(ship,dockwith);

            if (dockwithstatic->shiptype == Mothership
                && dockwithstatic->shiprace == R1
                && ship->specialFlags & SPECIAL_EXITING_DOOR)
            {
                //need special R1 Mothership door docking procedures..sigh

                //at this point the big docking port is free...
                //I'm assuming the door is closed!
                #ifndef HW_Release
                dbgAssert(dockwith->madDoorStatus == MAD_STATUS_DOOR_CLOSED);
                #endif
                //request door open and attach object to door
                madLinkInOpenDoor(dockwith);

                //need to start and stop here so a) works with current animation scheme, and b)
                //lets us get teh door position
                madAnimationStart(dockwith, dockwith->cuedAnimationIndex);  //start 0th cued animation
                mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship, &conepositionInWorldCoordSys,&coordsysWS,&heading,&up,&right);
                madAnimationStop(dockwith);

                //put ship outside at wrong orientation..but it will instantly be re-ordered with the attach command
                //before it is rendered
                dockPutShipOutside(ship,dockwith,&conepositionInWorldCoordSys,dockstaticpoint->headingdirection,dockstaticpoint->updirection);
                MothershipAttachObjectToDoor(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship);
                ship->dockvars.dockstate3 = LAUNCH_FROMMOTHERSHIPR1DOOR1;
            }
            else
            {
                //old school docking procedures
                vector outpos;
                if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
                {
                    vector tmpvec2;
                    vecAdd(tmpvec2,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
                    matMultiplyMatByVec(&outpos,&dockwith->rotinfo.coordsys,&tmpvec2);
                    vecAddTo(outpos,dockwith->posinfo.position);
                }
                else
                {
                    outpos = conepositionInWorldCoordSys;
                }
                if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
                {
                    dockPutShipOutside(ship,dockwith,&outpos,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up);
                }
                else
                {
                    dockPutShipOutside(ship,dockwith,&outpos,dockstaticpoint->headingdirection,dockstaticpoint->updirection);
                }

                ship->dockvars.dockstate3 = LAUNCH_FROMLAUNCHPOINT;
            }

            return FALSE;
        case LAUNCH_FROMMOTHERSHIPR1DOOR1:
            #ifndef HW_Release
            dbgAssert(dockwith->shiprace == R1);
            dbgAssert(dockwith->shiptype == Mothership);
            #endif

            if(dockwith->madDoorStatus == MAD_STATUS_DOOR_OPEN)
            {
                //door is open
                MothershipDettachObjectFromDoor(dockwith);
                ship->dockvars.dockstate3 = LAUNCH_FROMMOTHERSHIPR1DOOR2;
            }
            else
            {
                //door still opening
                #ifndef HW_Release
                dbgAssert(dockwith->madDoorStatus == MAD_STATUS_DOOR_OPENING);
                #endif
            }
            return FALSE;
        case LAUNCH_FROMMOTHERSHIPR1DOOR2:
            if (dockFlyAboveMothershipDoor(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
            break;
        case LAUNCH_FROMLAUNCHPOINT:
            dockstaticpoint = ship->dockvars.dockstaticpoint;
            dbgAssert(dockstaticpoint);
            //heading,up
            {
                //variables to define launch heading
                sdword h,u;
                if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
                {
                    h = dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading;
                    u = dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up;
                }
                else if(dockRetireShipNeedsBig(ship,dockwith) && dockwith->shiptype == Mothership && dockwith->shiprace == R2)
                {
                    h = 2;
                    u=0;
                }
                else
                {
                    h=-1;
                    u=-1;
                }
                if (dockFlyOutOfCone(ship,dockwith,h,u))
                {
                    ship->dockingship = NULL;
                    dockFreeDockPoint(ship,dockwith);
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
    }

    return FALSE;
}

#define CARRIERMOTHER_CONE_ALIGN_TOLERANCE  0.99f

bool ShipDocksAtCarrierMother(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtCarrierDockInfo *customdockinfo;
    sdword dockpointindex;
    bool resourcercanlatch,flag2;


    Player *player = ship->playerowner;
    Ship *mothership;
    Ship *carriers[4];
    udword launchmask = 0;

    FillInCarrierMothershipInfo(player,&mothership,carriers);

    if (dockwith == mothership)
    {
        launchmask = BIT0;
    }
    else if (dockwith == carriers[0])
    {
        launchmask = BIT1;
    }
    else if (dockwith == carriers[1])
    {
        launchmask = BIT2;
    }
    else if (dockwith == carriers[2])
    {
        launchmask = BIT3;
    }
    else if (dockwith == carriers[3])
    {
        launchmask = BIT4;
    }

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtCarrierDockInfo),"AtCarrierDock",0);
            customdockinfo->size = sizeof(AtCarrierDockInfo);

            customdockinfo->giveupdockpointat = 0.0f;

            if (dockwithstatic->shiptype == Carrier)
            {
                if (dockwithstatic->shiprace == R1)
                {
                    if((docktodo->dock.dockType & DOCK_FOR_RETIRE) &&
                       dockRetireShipNeedsBig(ship,dockwith))
                    {
                        customdockinfo->inpointsindex = 6;
                    }
                    else
                    {
                        customdockinfo->inpointsindex = 0;
                    }
                    customdockinfo->resourcelatchpointsindex = 0;
                    customdockinfo->latchmintime = R1CARRIER_LATCHMINTIME;
                    customdockinfo->refuelrepairmintime = R1CARRIER_REFUELREPAIRMINTIME;
                }
                else
                {
                    if((docktodo->dock.dockType & DOCK_FOR_RETIRE) &&
                       dockRetireShipNeedsBig(ship,dockwith))
                    {
                        customdockinfo->inpointsindex = 7;
                    }
                    else
                    {
                        customdockinfo->inpointsindex = 1;
                    }
                    customdockinfo->resourcelatchpointsindex = 1;
                    customdockinfo->latchmintime = R2CARRIER_LATCHMINTIME;
                    customdockinfo->refuelrepairmintime = R2CARRIER_REFUELREPAIRMINTIME;
                }
                resourcercanlatch = TRUE;
            }
            else
            {
                if (dockwithstatic->shiprace == R1)
                {
                    dbgAssert(dockwithstatic->shiptype == Mothership);

                    if((docktodo->dock.dockType & DOCK_FOR_RETIRE) &&
                       dockRetireShipNeedsBig(ship,dockwith))
                    {
                        customdockinfo->inpointsindex = 8;
                    }
                    else if (ship->staticinfo->shipclass == CLASS_Frigate ||
                        ship->shiptype == ResearchShip)
                    {
                        customdockinfo->inpointsindex = 10;
                    }
                    else
                    {
                        customdockinfo->inpointsindex = 2;
                    }

                    customdockinfo->resourcelatchpointsindex = 2;
                    customdockinfo->latchmintime = R1MOTHERSHIP_LATCHMINTIME;
                    customdockinfo->refuelrepairmintime = R1MOTHERSHIP_REFUELREPAIRMINTIME;
                    resourcercanlatch = TRUE;
                }
                else
                if (dockwithstatic->shiprace == R2)
                {
                    dbgAssert(dockwithstatic->shiptype == Mothership);
                    if (ship->shiptype == ResearchShip)
                    {
                        customdockinfo->inpointsindex = 11;
                    }
                    else if((docktodo->dock.dockType & DOCK_FOR_RETIRE) &&
                       dockRetireShipNeedsBig(ship,dockwith))
                    {
                        customdockinfo->inpointsindex = 9;
                    }
                    else
                    {
                        customdockinfo->inpointsindex = 3;
                    }

                    customdockinfo->resourcelatchpointsindex = 3;
                    customdockinfo->latchmintime = 0.0f;
                    customdockinfo->refuelrepairmintime = R2MOTHERSHIP_REFUELREPAIRMINTIME;
                    resourcercanlatch = TRUE;
                }
                else
                if (dockwithstatic->shiprace == P1)
                {
                    dbgAssert(dockwithstatic->shiptype == P1Mothership);
                    customdockinfo->inpointsindex = 4;
                    customdockinfo->resourcelatchpointsindex = -1;
                    customdockinfo->latchmintime = 0.0f;
                    customdockinfo->refuelrepairmintime = P1MOTHERSHIP_REFUELREPAIRMINTIME;
                    resourcercanlatch = FALSE;
                }
                else
                {
                    dbgAssert(dockwithstatic->shiptype == P2Mothership);
                    dbgAssert(dockwithstatic->shiprace == P2);
                    customdockinfo->inpointsindex = 5;
                    customdockinfo->resourcelatchpointsindex = -1;
                    customdockinfo->latchmintime = 0.0f;
                    customdockinfo->refuelrepairmintime = P2MOTHERSHIP_REFUELREPAIRMINTIME;
                    resourcercanlatch = FALSE;
                }

            }

            if ((resourcercanlatch) && (ship->shiptype == ResourceCollector) && ((docktodo->dock.dockType & (DOCK_PERMANENTLY|DOCK_FOR_RETIRE)) == 0))
            {
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_WAITFORLATCH;
                goto waitforlatch;
            }
            else
            {
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FINDCONE;
            }
            // deliberately fall through to CARRIERMOTHERDOCK_FINDCONE

        case CARRIERMOTHERDOCK_FINDCONE:
            if (docktodo->dock.dockType & DOCK_INSTANTANEOUSLY)
            {
                // just directly put ship inside:
                univRemoveShipFromOutside(ship);
                dockPutShipInside(ship,dockwith);
                return PERMANENTLY_DOCKED;
            }

            customdockinfo = ship->dockvars.customdockinfo;
            dockpointindex = dockFindNearestDockPoint(InPoints[customdockinfo->inpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
#if 0
                if(!MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW))
                {
                    //if not very close, lets keep within WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW,
                    aishipFlyToShipAvoidingObjs(ship,dockwith,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
                }
                else
                {
                    //close enough, slow down
                    aitrackSteadyShip(ship);
                }
#endif
                ship->dockwaitforNorm = FALSE;
                return FALSE;
            }
            if(docktodo->dock.dockType & DOCK_FOR_RETIRE)
            {
                if(!ship->dockwaitforNorm)
                {
                    //wait a sec...and let normal launching ships grab the docking point
                    ship->dockwaitforNorm = TRUE;
                    return FALSE;
                }
            }
            ship->dockwaitforNorm = FALSE;
            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);
            if (ship->staticinfo->shipclass == CLASS_Fighter)
                customdockinfo->giveupdockpointat = universe.totaltimeelapsed + CARRIERMOTHERDOCK_DELAY_FLYINSIDE;
            else if (ship->staticinfo->shipclass == CLASS_Corvette || ship->shiptype == ProximitySensor)
                customdockinfo->giveupdockpointat = universe.totaltimeelapsed + CARRIERMOTHERDOCK_DELAY_FLYINSIDE_CORVETTE;

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOINSIDECONE;
            }

            if(docktodo->dock.dockType & DOCK_FOR_RETIRE)
            {
                //enter our own state for retirement
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FOR_RETIRE;
            }
            return FALSE;

waitforlatch:
        case CARRIERMOTHERDOCK_WAITFORLATCH:
            customdockinfo = ship->dockvars.customdockinfo;
            dockpointindex = dockFindNearestDockPoint(ResourceLatchPoints[customdockinfo->resourcelatchpointsindex],ship,dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
#if 0
                if(!MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW))
                {
                    //if not very close, lets keep within WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW,
                    aishipFlyToShipAvoidingObjs(ship,dockwith,AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying,0.0f);
                }
                else
                {
                    //close enough, slow down
                    aitrackSteadyShip(ship);
                }
#endif
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHFLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHFLYTOINSIDECONE;
            }
            return FALSE;

        case CARRIERMOTHERDOCK_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if ((customdockinfo->giveupdockpointat > 0.0f) && (customdockinfo->giveupdockpointat <= universe.totaltimeelapsed))
            {
                dockFreeDockPoint(ship,dockwith);
                ship->dockvars.reserveddocking = -2;            // -2 indicates it was given up for speed reasons, not because docking is totally complete
                customdockinfo->giveupdockpointat = -1.0f;
            }
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockingship = dockwith;
                if(docktodo->dock.dockType & DOCK_FOR_RETIRE)
                {
                    //docking for retiring...
                    if(dockRetireShipNeedsBig(ship,dockwith))
                    {
                        //ship is a capital ship so we need to add an
                        //extra state to let it align properly to the docking line
                        ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOCONEORIGIN_AND_ALIGN;
                        return FALSE;
                    }
                }
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOCONEORIGIN;

            }
            return FALSE;
        case CARRIERMOTHERDOCK_FLYTOCONEORIGIN_AND_ALIGN:
            customdockinfo = ship->dockvars.customdockinfo;
             if ((customdockinfo->giveupdockpointat > 0.0f) && (customdockinfo->giveupdockpointat <= universe.totaltimeelapsed))
             {
                 dockFreeDockPoint(ship,dockwith);
                 customdockinfo->giveupdockpointat = -1.0f;
             }
             if (dockFlyToConeInsideAndAlign(ship,dockwith,CARRIERMOTHER_CONE_ALIGN_TOLERANCE,1000.0f,dockwith->shiprace))
             {
                 ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOCONEORIGIN;
             }
             return FALSE;
        case CARRIERMOTHERDOCK_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if ((customdockinfo->giveupdockpointat > 0.0f) && (customdockinfo->giveupdockpointat <= universe.totaltimeelapsed))
            {
                dockFreeDockPoint(ship,dockwith);
                customdockinfo->giveupdockpointat = -1.0f;
            }
            flag2 = (docktodo->dock.dockType & DOCK_FOR_RETIRE) && dockwith->shiptype == Mothership && dockRetireShipNeedsBig(ship,dockwith);
            if (dockFlyToConeOrigin(ship,dockwith,CONEORIGIN_TOLERANCE,CONEORIGIN_MINFLYSPEED,FALSE,flag2))
            {
                if(ship->shiptype == CloakedFighter)
                {
                    if(bitTest(ship->flags,SOF_Cloaked)  || bitTest(ship->flags,SOF_Cloaking))
                    {
                        bitClear(ship->flags,SOF_Cloaking);
                        bitSet(ship->flags,SOF_DeCloaking);
                    }
                }
                if (customdockinfo->giveupdockpointat > 0.0f)
                {
                    dockFreeDockPoint(ship,dockwith);
                    ship->dockvars.reserveddocking = -2;        // -2 indicates it was given up for speed reasons, not because docking is totally complete
                    customdockinfo->giveupdockpointat = -1.0f;
                }
//                soundEvent(ship, Docking);
                univRemoveShipFromOutside(ship);
                if(docktodo->dock.dockType & DOCK_FOR_RETIRE)
                {
                    //no speech event I thinks...or special one for retirement here!
                    return TRUE;
                }
                dockPutShipInside(ship,dockwith);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_REFUEL;
                if (battleCanChatterAtThisTime(BCE_CapitalShipDocked, dockwith))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDocked, dockwith, SOUND_EVENT_DEFAULT);
                }
//                speechEvent(dockwith, STAT_Cap_ShipDocked, 0);

                if ((singlePlayerGame)  &&
                    (ship->shiptype == SalCapCorvette) &&
                    (bitTest(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology)))
                {
                    tutGameMessage("Game_TechnologyReturned");
                    bitClear(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology);
                    salCapClearTechBool();
                }

            }
            return FALSE;

        case CARRIERMOTHERDOCK_LATCHFLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHFLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case CARRIERMOTHERDOCK_LATCHFLYTOCONEORIGIN:
            if (dockFlyToConeOriginLatch(ship,dockwith))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHREFUEL;
                if (battleCanChatterAtThisTime(BCE_CapitalShipDocked, dockwith))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDocked, dockwith, SOUND_EVENT_DEFAULT);
                }
//                speechEvent(dockwith, STAT_Cap_ShipDocked, 0);
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;
        case CARRIERMOTHERDOCK_FOR_RETIRE:
            if(dockRetireShipNeedsBig(ship,dockwith))
            {
                if(dockwith->shiprace == R1)
                {
                    if(dockwith->shiptype == Mothership)
                    {
                        ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_IN_DOOR;
                        return FALSE;
                    }
                }
            }
            ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTOINSIDECONE;
            return FALSE;;
        case CARRIERMOTHERDOCK_IN_DOOR:
            if(dockFlyToBottomOfDoor1(ship,dockwith))
            {
                madLinkInOpenDoor(dockwith);
                ship->dockingship = dockwith;
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTODOOR2;
            }
            return FALSE;
        case CARRIERMOTHERDOCK_FLYTODOOR2:
            if(dockFlyToBottomOfDoor2(ship,dockwith))
            {
                madLinkInOpenDoor(dockwith);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTODOOR3;
            }
            return FALSE;
        case CARRIERMOTHERDOCK_FLYTODOOR3:
            if(dockFlyToBottomOfDoor3(ship,dockwith))
            {
                madLinkInOpenDoor(dockwith);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_FLYTODOOR;
            }
            return FALSE;
        case CARRIERMOTHERDOCK_FLYTODOOR:
            if(dockwith->madDoorStatus != MAD_STATUS_DOOR_OPEN)
            {
                //wait for door to be open!
                return FALSE;;
            }
            if(dockFlyToDoor(ship,dockwith))
            {
                //in position to be clamped to door!
                MothershipAttachObjectToDoor(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship);
                madLinkInCloseDoor(dockwith);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_WAIT_FOR_DOOR_TO_CLOSE;
            }
            return FALSE;;
        case CARRIERMOTHERDOCK_WAIT_FOR_DOOR_TO_CLOSE:
            if(dockwith->madDoorStatus == MAD_STATUS_DOOR_CLOSED)
            {
                //door is closed...so retire ship!
                univRemoveShipFromOutside(ship);
                return TRUE;
            }
            return FALSE;;
        case CARRIERMOTHERDOCK_REFUEL:
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->refuelrepairmintime;
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_WAIT;
            }
            return FALSE;

        case CARRIERMOTHERDOCK_WAIT:
            customdockinfo = ship->dockvars.customdockinfo;
            if (universe.totaltimeelapsed > customdockinfo->timestamp)
            {
                if ( ( (dockwith->playerowner->autoLaunch & launchmask) || (ShipHasToLaunch(dockwith,ship)) ||
                       (dockwith->playerowner != ship->playerowner) ) &&
                           ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) == 0)
                   )
                {
//                    soundEvent(ship, Launch);
                    dockInitShipForLaunch(ship);
                    ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LAUNCH;
                }
                else
                {
                    return PERMANENTLY_DOCKED;
                }
            }
            return FALSE;

        case CARRIERMOTHERDOCK_LAUNCH:
            if (LaunchShipFromCarrierMother(ship,dockwith))
            {
                return FINISHED_LAUNCHING_FROM_DOCKING;
            }
            return FALSE;

        case CARRIERMOTHERDOCK_LATCHREFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHWAIT;
            }
            return FALSE;

        case CARRIERMOTHERDOCK_LATCHWAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if ((universe.totaltimeelapsed > customdockinfo->timestamp) &&
                ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) == 0))
            {
//                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = CARRIERMOTHERDOCK_LATCHBACKUP;
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case CARRIERMOTHERDOCK_LATCHBACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*=============================================================================
    DDDFrigate Launching/docking
=============================================================================*/

bool LaunchShipFromDDDF(Ship *ship,Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    DroneSpec *dronespec = (DroneSpec *)ship->ShipSpecifics;

    dbgAssert(ship->shiptype == Drone);
    dbgAssert(dockwith->shiptype == DDDFrigate);

    switch (ship->dockvars.dockstate3)
    {
        case 0:
            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dronespec->droneNumber];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
            vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

            dockRemoveShipFromInside(ship,dockwith);
            dockPutShipOutside(ship,dockwith,&conepositionInWorldCoordSys,dockstaticpoint->headingdirection,dockstaticpoint->updirection);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);
            vecMultiplyByScalar(coneheadingInWorldCoordSys,DRONE_LAUNCH_VELOCITY);
            vecAddTo(ship->posinfo.velocity,coneheadingInWorldCoordSys);

            ship->dockvars.dockstate3 = 1;
            return FALSE;

        case 1:
            if (dockFlyOutOfCone(ship,dockwith,-1,-1))
            {
                ship->dockingship = NULL;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
    }

    return FALSE;
}

typedef struct
{
    sdword size;
    real32 dockmisc;
} AtDDDFDockInfo;

#define DRONEDOCK_FINDCONE          1
#define DRONEDOCK_FLYTOINSIDECONE   2
#define DRONEDOCK_FLYTOCONEORIGIN   3

bool DroneDocksAtDDDF(struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    DroneSpec *dronespec = (DroneSpec *)ship->ShipSpecifics;
    AtDDDFDockInfo *customdockinfo;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;

    dbgAssert(ship->shiptype == Drone);
    dbgAssert(dockwith->shiptype == DDDFrigate);

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtDDDFDockInfo),"AtDDDFDockInfo",0);
            customdockinfo->size = sizeof(AtDDDFDockInfo);

            ship->dockvars.dockstate2 = DRONEDOCK_FINDCONE;
            // deliberately fall through to next state

        case DRONEDOCK_FINDCONE:
            customdockinfo = ship->dockvars.customdockinfo;

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dronespec->droneNumber];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
            vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

            vecSub(coneToShip,ship->posinfo.position,conepositionInWorldCoordSys);

            coneToShipMagSqr = vecMagnitudeSquared(coneToShip);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = DRONEDOCK_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = DRONEDOCK_FLYTOINSIDECONE;
            }
            return FALSE;

        case DRONEDOCK_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                ship->dockvars.dockstate2 = DRONEDOCK_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case DRONEDOCK_FLYTOCONEORIGIN:
            if (dockFlyToConeOrigin(ship,dockwith,DRONE_CONEORIGIN_TOLERANCE,DRONE_CONEORIGIN_MINFLYSPEED,TRUE,FALSE))
            {
                univRemoveShipFromOutside(ship);
                dockPutShipInside(ship,dockwith);
                return TRUE;
            }
            return FALSE;
    }

    return FALSE;
}

/*=============================================================================
    Research ship docking stuff...race 1 for now
=============================================================================*/

#define RESEARCH_DECIDE_ON_LATCH_POINT  1
#define WAIT_FOR_SHIP_TO_BE_DONE        2
#define RESEARCH_DOCK_FLYTOPIEPOINT     3
#define RESEARCH_FLYTOPIVOT_POINT       4
#define RESEARCH_STEADYSHIP             5
#define RESEARCH_FINAL_SQUEEZE          6
#define RESEARCH_WAIT                   7
#define RESEARCH_SLAVE                  8

//#define R1_DEBUG_RESEARCH_DOCKING

#define PIE_POINT_TOLR2   255.0f           //INCREASE THIS...AND MAKE TUNABLE!
#define PIE_POINT_TOLR1   75.0f           //INCREASE THIS...AND MAKE TUNABLE!
#define PARA_POINT_TOL  10.0f            //SAME COOKIE
#define FINAL_POINT_TOL 2.0f
#define ANGLE_TOL1       0.99f
#define ANGLE_TOLAC      0.999f

void bitSetAllSlaves(Ship *ship)
{
    if(ship->flags & SOF_Slaveable)
    {
        bitSet(ship->slaveinfo->Master->specialFlags,SPECIAL_StopForResearchDocking);
    }
}
void bitClearAllSlaves(Ship *ship)
{
    if(ship->flags & SOF_Slaveable)
    {
        bitClear(ship->slaveinfo->Master->specialFlags,SPECIAL_StopForResearchDocking);
    }
}
bool isThereAMasterForMe(Ship *ship)
{
    Ship *m;
    Node *shipnode = universe.ShipList.head;
    udword x = 0;

    while(shipnode != NULL)
    {
        if(x >= 6)
            break;  //no masters in list of ships
        m = (Ship *) listGetStructOfNode(shipnode);
        if(m->staticinfo->shiptype == ResearchShip)
        {
            if(m->playerowner== ship->playerowner)
            {
                if(m->flags & SOF_Slaveable)
                    return TRUE;
                x++;
            }
        }
        shipnode = shipnode->next;
    }
    return FALSE;
}

bool parallelParkStabilize(Ship *ship,Ship *dockwith, real32 POS_TOL, real32 ANG_TOL, sdword FIT)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    vector coneheadingInWorldCoordSysDockWith;
    vector DockWithHeading,DockWithUp;

    AtResearchShipDockInfo *customdockinfo;
    vector desiredHeading,desiredUp;
    matrix tmpmat,rotmatrix;
    real32 theta;
    udword flag = 0;

    vector destination;
    vector destinationoffset;
    ResearchShipStatics *resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;


    customdockinfo = ship->dockvars.customdockinfo;

    dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[customdockinfo->docktoindex];
    ship->dockvars.dockstaticpoint = dockwithstaticpoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
    matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

    vecScalarMultiply(destinationoffset,DockWithHeading,500.0f);
    destinationoffset.x = destinationoffset.x + coneheadingInWorldCoordSysDockWith.x*550.0f;
    destinationoffset.y = destinationoffset.y + coneheadingInWorldCoordSysDockWith.y*550.0f;
    destinationoffset.z = destinationoffset.z + coneheadingInWorldCoordSysDockWith.z*550.0f;
    vecAdd(destination,dockwith->posinfo.position, destinationoffset);
    if(((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num == 0)
    {    //ship is docking on'a'top so add upwards factor
        matGetVectFromMatrixCol1(DockWithUp,dockwith->rotinfo.coordsys);
        vecScalarMultiply(DockWithUp,DockWithUp,resstatics->R1VerticalDockDistance);
        vecAdd(destination,destination,DockWithUp);
    }

    if(customdockinfo->docktoindex == 0)
    {
        theta = DEG_TO_RAD(60);
    }
    else
    {
        theta = DEG_TO_RAD(-60);
    }

    matMakeRotAboutX(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
    matMultiplyMatByMat(&tmpmat, &dockwith->rotinfo.coordsys, &rotmatrix);
    flag = 0;

    matGetVectFromMatrixCol3(desiredHeading , tmpmat);
    matGetVectFromMatrixCol1(desiredUp , tmpmat);

    if(aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,ANG_TOL))
    {
        if(FIT)
            ship->rotinfo.coordsys = tmpmat;
        flag = 1;
    }

    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FastAsPossible,0.0f,&dockwith->posinfo.velocity);
    if (MoveReachedDestinationVariable(ship,&destination,POS_TOL))
    {
        if(FIT)
            ship->posinfo.position = destination;
        flag += 1;
    }
    if(flag == 2)
        return TRUE;

    return FALSE;

}

bool parallelParkPiePlate(Ship *ship,Ship *dockwith, real32 POS_DIS, real32 POS_TOL, real32 ANG_TOL, sdword FIT)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    vector coneheadingInWorldCoordSysDockWith;
    vector DockWithHeading,DockWithUp;

    AtResearchShipDockInfo *customdockinfo;
    vector destination;
    vector destinationoffset;

    ResearchShipStatics *resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    vector desiredHeading,desiredUp;
    matrix tmpmat,rotmatrix;
    real32 theta;
    udword flag = 0;

    customdockinfo = ship->dockvars.customdockinfo;

    dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[customdockinfo->docktoindex];
    ship->dockvars.dockstaticpoint = dockwithstaticpoint;

    //get stationary research ships position and docking point heading
    matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
    matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

    destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*POS_DIS;
    destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*POS_DIS;
    destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*POS_DIS;
    vecAdd(destination,dockwith->posinfo.position, destinationoffset);
    if(((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num == 0)
    {    //ship is docking on'a'top so add upwards factor
        matGetVectFromMatrixCol1(DockWithUp,dockwith->rotinfo.coordsys);
        vecScalarMultiply(DockWithUp,DockWithUp,resstatics->R1VerticalDockDistance);
        vecAdd(destination,destination,DockWithUp);
    }
    if(customdockinfo->docktoindex == 0)
    {
        theta = DEG_TO_RAD(60);
    }
    else
    {
        theta = DEG_TO_RAD(-60);
    }

    matMakeRotAboutX(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
    matMultiplyMatByMat(&tmpmat, &dockwith->rotinfo.coordsys, &rotmatrix);
    flag = 0;

    matGetVectFromMatrixCol3(desiredHeading , tmpmat);
    matGetVectFromMatrixCol1(desiredUp , tmpmat);

    if(aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,ANG_TOL))
    {
        if(FIT)
            ship->rotinfo.coordsys = tmpmat;
        flag = 1;
    }

    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FastAsPossible,0.0f,&dockwith->posinfo.velocity);
    if (MoveReachedDestinationVariable(ship,&destination,POS_TOL))
    {
        if(FIT)
            ship->posinfo.position = destination;
        flag++;
    }
    if(flag == 2)
        return TRUE;

    return FALSE;

}
bool dockFlyToPiePoint(Ship *ship,Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    vector coneheadingInWorldCoordSysDockWith;
    vector DockWithHeading,DockWithUp;

    AtResearchShipDockInfo *customdockinfo;

    vector destination;
    vector destinationoffset;
    ResearchShipStatics *resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;


    customdockinfo = ship->dockvars.customdockinfo;

    dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[customdockinfo->docktoindex];
    ship->dockvars.dockstaticpoint = dockwithstaticpoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
    matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

    vecScalarMultiply(destinationoffset,DockWithHeading,900.0f);
    destinationoffset.x = destinationoffset.x + coneheadingInWorldCoordSysDockWith.x*550.0f;
    destinationoffset.y = destinationoffset.y + coneheadingInWorldCoordSysDockWith.y*550.0f;
    destinationoffset.z = destinationoffset.z + coneheadingInWorldCoordSysDockWith.z*550.0f;
    vecAdd(destination,dockwith->posinfo.position, destinationoffset);
    if(((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num == 0)
    {    //ship is docking on'a'top so add upwards factor
        matGetVectFromMatrixCol1(DockWithUp,dockwith->rotinfo.coordsys);
        vecScalarMultiply(DockWithUp,DockWithUp,resstatics->R1VerticalDockDistance);
        vecAdd(destination,destination,DockWithUp);
    }

    if(dockwith->posinfo.isMoving & ISMOVING_MOVING)
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying + AISHIP_FastAsPossible,((ShipStaticInfo *)ship->staticinfo)->staticheader.maxvelocity/2,&dockwith->posinfo.velocity);
    else
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying + AISHIP_FastAsPossible,0.0f,&dockwith->posinfo.velocity);

    return(MoveReachedDestinationVariable(ship,&destination,PIE_POINT_TOLR1));
}
void FlyToResearchWaitPoint(Ship *ship,Ship *dockwith)
{
    vector waitpoint;

    if( ((ResearchShipSpec *) ship->ShipSpecifics)->masterptr != NULL)
        waitpoint = ((ResearchShipSpec *) ship->ShipSpecifics)->masterptr->posinfo.position;
    else
        waitpoint = dockwith->posinfo.position;

    if(!MoveReachedDestinationVariable(ship,&waitpoint,1500.0f))
        aishipFlyToPointAvoidingObjsWithVel(ship,&waitpoint,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);
    else
        aitrackZeroVelocity(ship);
}
/*-----------------------------------------------------------------------------
    Name        : R1ResearchShipDocksAtResearchShip
    Description : custom code for docking researchships to researchships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool R1ResearchShipDocksAtResearchShip(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    Ship *tmpmaster;
    AtResearchShipDockInfo *customdockinfo;
    sdword dockwithpointindex;
    DockStaticPoint *shipstaticpoint;
    sword shippointindex;
    ResearchShipStatics *resstatics;

    resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    dbgAssert(ship->shiptype == ResearchShip);          //both should only be research
    dbgAssert(dockwith->shiptype == ResearchShip);  //ships otherwise poo on you

    dbgAssert(ship->shiprace == R1);
    dbgAssert(dockwith->shiprace == R1);      //both must be race 1 ships...

    switch (ship->dockvars.dockstate2)
    {
    case 0:
        dbgAssert(ship->dockvars.customdockinfo == NULL);
        customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtResearchShipDockInfo),"ResearchAtResearchCustomDockInfo",0);
        customdockinfo->size = sizeof(AtResearchShipDockInfo);
        ship->dockvars.dockstate2 = WAIT_FOR_SHIP_TO_BE_DONE;
    case WAIT_FOR_SHIP_TO_BE_DONE:
        if(((ResearchShipSpec *)dockwith->ShipSpecifics)->busy_docking == FALSE)
        {
            ship->dockvars.dockstate2 = RESEARCH_DECIDE_ON_LATCH_POINT;
            if(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY)
            {
                goto dockin_findlatch;
            }
            break;
        }
        else if(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY)
        {
            ship->dockvars.dockstate2 = RESEARCH_DECIDE_ON_LATCH_POINT;
            goto dockin_findlatch;
        }
        FlyToResearchWaitPoint(ship,dockwith);
        break;
    case RESEARCH_DECIDE_ON_LATCH_POINT:
dockin_findlatch:
        customdockinfo = ship->dockvars.customdockinfo;     //set customdockinfo...
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        if(dockwith->dockInfo->dockpoints[0].thisDockBusy == 0)
        {
            customdockinfo->docktoindex = dockwithpointindex = 0;   //choose IN side to dock on
            customdockinfo->dockingindex = shippointindex = 1;
        }
        /*
        else if(dockwith->dockInfo->dockpoints[1].thisDockBusy == 0)
        {
            customdockinfo->docktoindex = dockwithpointindex = 1;   //choose Out side to dock on
            customdockinfo->dockingindex = shippointindex = 0;
        }
        */
        else
        {
        #ifdef R1_DEBUG_RESEARCH_DOCKING
            dbgFatalf(DBG_Loc, "\nResearch Ship Race 1 docking error...No free docking point found");
        #endif
        }

#ifdef R1_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 1: DOCKWITH    : point %s chosen.",dockwithstatic->dockStaticInfo->dockstaticpoints[dockwithpointindex].name);
        dbgMessagef("\nRace 1: DOCKING SHIP: point %s chosen.",shipstatic->dockStaticInfo->dockstaticpoints[shippointindex].name);
        dbgMessagef("\nNow flying to PiePoint.");
#endif
        dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockwithpointindex];
        shipstaticpoint = &shipstatic->dockStaticInfo->dockstaticpoints[shippointindex];

        ship->dockvars.dockstaticpoint = dockwithstaticpoint;

        /* speechEvent: Guidance Lock Responding */
        if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
        {
            speechEventVar(ship, STAT_Research_Docking, 0, 0);
        }

        ship->dockvars.dockstate2 = RESEARCH_DOCK_FLYTOPIEPOINT;

        dockReserveDockPoint(ship,dockwith,dockwithpointindex);     //steal docking ports, but don't signify docking...yet
        ship->dockInfo->dockpoints[shippointindex].thisDockBusy = 1;    //artificially busy point
        if(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY)
        {
            goto nextdockr1res;
        }
        return FALSE;       //not done yet!
    case RESEARCH_DOCK_FLYTOPIEPOINT:
        customdockinfo = ship->dockvars.customdockinfo;
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        if (dockFlyToPiePoint(ship,dockwith))
        {
nextdockr1res:
            ((ResearchShipSpec *)(ship->ShipSpecifics))->busy_docking = FALSE;    //signifiy that another ship can start docking
            if(bitTest(dockwith->flags, SOF_Slaveable))
            {   //docked ship is a salve or master
                ship->dockingship = dockwith->slaveinfo->Master;
            }
            else if(dockwith->dockingship != NULL)
            {
                ship->dockingship = dockwith->dockingship;
            }
            else
            {    //first time...so dockwith 'will be a master' but isn't yet
                ship->dockingship = dockwith;
            }

            /* speechEvent: Guidance lock confirmed */
            if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                speechEventVar(ship, STAT_Research_Docking, 0, 1);
            }

            ship->dockvars.dockstate2 = RESEARCH_STEADYSHIP;
            if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                goto morer1resdock;
            }

#ifdef R1_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 1: Reached Point, now parallel parking.");
#endif
        }
        return FALSE;
    case RESEARCH_STEADYSHIP:           //aligns ship to fit in pie plate
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        if(parallelParkStabilize(ship,dockwith,100.0f,0.99f,FALSE))
        {
            /* speechEvent: coming together... */
            speechEventVar(ship, STAT_Research_Docking, 0, 2);

            ship->dockvars.dockstate2 = RESEARCH_FINAL_SQUEEZE;
#ifdef R1_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 1: Parallel Parking");
        dbgMessagef("\nAllowing Next Research Ship to start docking!");
#endif
       }
        return FALSE;
    case RESEARCH_FLYTOPIVOT_POINT:
        customdockinfo = ship->dockvars.customdockinfo;
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        bitSetAllSlaves(dockwith);
        if(parallelParkPiePlate(ship,dockwith, resstatics->R1parallel_dock_distance,PARA_POINT_TOL,ANGLE_TOL1,FALSE))
        {
            ship->posinfo.velocity.x=0.0f;
            ship->posinfo.velocity.y=0.0f;
            ship->posinfo.velocity.z=0.0f;
            ship->dockvars.dockstate2 = RESEARCH_FINAL_SQUEEZE;
#ifdef R1_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 1: Final Docking Phaze....");
#endif
        }
        return FALSE;
    case RESEARCH_FINAL_SQUEEZE:
        customdockinfo = ship->dockvars.customdockinfo;
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        bitSetAllSlaves(dockwith);

        if(parallelParkPiePlate(ship,dockwith, resstatics->R1final_dock_distance,FINAL_POINT_TOL,ANGLE_TOLAC, TRUE))
        {
morer1resdock:
            ship->posinfo.velocity.x=0.0f;
            ship->posinfo.velocity.y=0.0f;
            ship->posinfo.velocity.z=0.0f;
            if(!bitTest(dockwith->flags,SOF_Slaveable))
            {
            //object isn't slaved, might not be a master
                if(dockwith->playerowner->shiptotals[ResearchShip] > 2)
                {
                    //there are more than 2 reserach ships, so wait until our dockwith is docked...
                    if(isThereAMasterForMe(ship))
                    {
                        //there exists a master somewheres, so hold horse
                        if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
                        {
                            break;
                        }
                    }
                    //no master so let this guy dock...
                }
            }
            if(bitTest(dockwith->flags,SOF_Slaveable))
            {    //object is a master or slave
                if(bitTest(dockwith->slaveinfo->flags,SF_SLAVE))
                {    //object is a slave
                    tmpmaster = dockwith->slaveinfo->Master;
                }
                else
                {   // object is a master
                    tmpmaster = dockwith;
                }
            }
            else
            {    //object is 'the first'
                dockMakeMaster(dockwith);
                tmpmaster = dockwith;
            }
            if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                addMonkeyResearchShipChangePosition(dockwith,ship,0);
            }
            dockAddSlave(tmpmaster,ship);       //enslaved!
            ((ResearchShipSpec *)(ship->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = FALSE;
            bitClearAllSlaves(dockwith);

            /* finished docking */
            if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                speechEvent(ship, STAT_Research_Docked, 0);
                soundEvent(ship, Ship_ResearchLockOn);
            }
            ship->dockvars.dockstate2 = RESEARCH_WAIT;
#ifdef R1_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 1: Docked...Slaveing ship, extending docking port.");
#endif
        }
        return FALSE;
    case  RESEARCH_SLAVE:

        //maybe should put this in loop above

                break;
    case RESEARCH_WAIT:
        break;
    default:
        dbgAssert(FALSE);
        break;

    }

    return FALSE;
}

//#define R2_DEBUG_RESEARCH_DOCKING

#define R2_FIND_LATCH                   1
#define R2_WAIT_FOR_DOCK_POINT          2
#define R2_FLY_TO_DOCK_PATH_START       3
#define R2_ROTATE_AND_SPIN_INTO_PLACE   4
#define R2_DOCKED_AND_WAITING           5
#define R2_DOCKED_AND_SLAVING           6

#define R2_FINAL_ANGLE_TOL          0.999f

bool R2atdockpathstart(Ship *ship, Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    vector conepositionInWorldCoordSysDockWith;
    vector coneheadingInWorldCoordSysDockWith;
    vector DockWithHeading;

    AtResearchShipDockInfo *customdockinfo;

    vector destination;
    vector destinationoffset;

    customdockinfo = ship->dockvars.customdockinfo;

    dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[customdockinfo->docktoindex];
    ship->dockvars.dockstaticpoint = dockwithstaticpoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
    matMultiplyMatByVec(&conepositionInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSysDockWith,dockwith->posinfo.position);
    matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

    destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*600.0f;
    destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*600.0f;
    destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*600.0f;
    vecAdd(destination,conepositionInWorldCoordSysDockWith, destinationoffset);

    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);

    return(MoveReachedDestinationVariable(ship,&destination,PIE_POINT_TOLR2));
}

bool R2spinintoplace(Ship *ship, Ship *dockwith, real32 POS_TOL, real32 ANGLE_TOL, sdword FIT)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;
    vector conepositionInWorldCoordSysDockWith;
    vector coneheadingInWorldCoordSysDockWith;
    vector DockWithHeading;

    AtResearchShipDockInfo *customdockinfo;

    vector destination;
    vector destinationoffset;
    vector desiredHeading;
    vector tmpvec;
    matrix rotmatrix,tmpmat;
    vector desiredUp;
    real32 theta;

    udword flag = 0;

    ResearchShipStatics *resstatics;
    resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    customdockinfo = ship->dockvars.customdockinfo;

    dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[customdockinfo->docktoindex];
    ship->dockvars.dockstaticpoint = dockwithstaticpoint;

    matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
    matMultiplyMatByVec(&conepositionInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSysDockWith,dockwith->posinfo.position);
    matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

    destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*resstatics->R2DockFinalDistance;
    destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*resstatics->R2DockFinalDistance;
    destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*resstatics->R2DockFinalDistance;
    vecAdd(destination,conepositionInWorldCoordSysDockWith, destinationoffset);

    desiredHeading = coneheadingInWorldCoordSysDockWith;
    if(customdockinfo->docktoindex == 0)
    {
        //theta = DEG_TO_RAD(30);
        //matMakeRotAboutX(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
        matGetVectFromMatrixCol3(desiredUp,dockwith->rotinfo.coordsys);
        //matMultiplyMatByVec(&tmpcross, &dockwith->rotinfo.coordsys, &tmpvec);
        //vecCrossProduct(desiredUp,desiredHeading,tmpcross);
        //desiredUp = tmpcross;

    }
    else if(customdockinfo->docktoindex == 1)
    {
        matGetVectFromMatrixCol3(desiredUp,dockwith->rotinfo.coordsys);
        //theta = DEG_TO_RAD(-30);
        //matMakeRotAboutX(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
        //matGetVectFromMatrixCol1(tmpvec,dockwith->rotinfo.coordsys);
        //matMultiplyMatByVec(&tmpcross, &dockwith->rotinfo.coordsys, &tmpvec);
        //vecCrossProduct(desiredUp,desiredHeading,tmpcross);
        //desiredUp = tmpcross;
    }
    else if(customdockinfo->docktoindex == 2)
    {
        desiredUp = DockWithHeading;
    }
    else if(customdockinfo->docktoindex == 3)
    {
        theta = DEG_TO_RAD(60);
        matMakeRotAboutZ(&rotmatrix,(real32) cos(theta),(real32) sin(theta));

        //matGetVectFromMatrixCol1(tmpvec,dockwith->rotinfo.coordsys);
        //matMultiplyVecByMat(&desiredUp, &tmpvec,&rotmatrix);

        //USE THIS CODE FOR ALTERNATING ROTATIONS...

        matMultiplyMatByMat(&tmpmat, &dockwith->rotinfo.coordsys, &rotmatrix);
        matGetVectFromMatrixCol1(desiredUp,tmpmat);
    }
    else if(customdockinfo->docktoindex == 4)
    {
        destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*100;
        destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*100;
        destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*100;
        vecAdd(destination,conepositionInWorldCoordSysDockWith, destinationoffset);

        matGetVectFromMatrixCol1(desiredUp,dockwith->rotinfo.coordsys);
        vecScalarMultiply(desiredHeading,desiredHeading,-1.0f);
        dbgMessagef("\n\nTRIED TO DOCK WITH SHIPS ASS!! REPORT TO BRYCE.");
    }
    else
    {
        dbgAssert(FALSE);       //shouldget here.
    }

    vecNormalize(&desiredUp);
    if(aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,ANGLE_TOL))
    {
        if(FIT)
        {
            matPutVectIntoMatrixCol1(desiredUp,ship->rotinfo.coordsys);
            vecCrossProduct(tmpvec,desiredHeading,desiredUp);
            matPutVectIntoMatrixCol2(tmpvec,ship->rotinfo.coordsys);
            matPutVectIntoMatrixCol3(desiredHeading,ship->rotinfo.coordsys);
        }

        flag = 1;
    }

    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);
    if (MoveReachedDestinationVariable(ship,&destination,POS_TOL))
    {
        if(FIT)
            ship->posinfo.position = destination;
        flag += 1;
        if(flag == 2)
        {
            ship->posinfo.velocity.x = 0.0f;
            ship->posinfo.velocity.y = 0.0f;
            ship->posinfo.velocity.z = 0.0f;
            return TRUE;
        }
    }
    return FALSE;
}



bool R2ResearchShipDocksAtResearchShip(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockwithstaticpoint;

    AtResearchShipDockInfo *customdockinfo;
    sdword dockwithpointindex;

    DockStaticPoint *shipstaticpoint;
    sword shippointindex;

    Ship* tmpmaster;

    ResearchShipStatics *resstatics;
    resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    switch (ship->dockvars.dockstate2)
    {
    case 0:
        dbgAssert(ship->dockvars.customdockinfo == NULL);
        customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtResearchShipDockInfo),"AtResearchShipDockInfo",0);
        customdockinfo->size = sizeof(AtResearchShipDockInfo);
#ifdef  R2_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nResearch Docking code Started.  RACE 2");
#endif
        ship->dockvars.dockstate2 = R2_WAIT_FOR_DOCK_POINT;
    case R2_WAIT_FOR_DOCK_POINT:
        if(((ResearchShipSpec *)dockwith->ShipSpecifics)->busy_docking == FALSE)
        {
            ship->dockvars.dockstate2 = R2_FIND_LATCH;
            if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                goto r2resinstant1;
            }
            break;
        }
        if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
        {
            goto r2resinstant1;
        }

        FlyToResearchWaitPoint(ship,dockwith);
        break;
    case R2_FIND_LATCH:
r2resinstant1:
        customdockinfo = ship->dockvars.customdockinfo;     //set customdockinfo...
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
         if(((ResearchShipSpec *)ship->ShipSpecifics)->seed == TRUE)
         {  //if new seed, put on end
            customdockinfo->docktoindex = dockwithpointindex = 3;   //choose Out side to dock on
            customdockinfo->dockingindex = shippointindex = 4;
         }
         else if(dockwith->dockInfo->dockpoints[0].thisDockBusy == 0)         //head left
         {
             customdockinfo->docktoindex = dockwithpointindex = 0;   //choose IN side to dock on
             customdockinfo->dockingindex = shippointindex = 4;
         }
         else if(dockwith->dockInfo->dockpoints[1].thisDockBusy == 0)   //head right
         {
             customdockinfo->docktoindex = dockwithpointindex = 1;   //choose Out side to dock on
             customdockinfo->dockingindex = shippointindex = 4;
         }
         else if(dockwith->dockInfo->dockpoints[2].thisDockBusy == 0)   //head bottom
         {
             customdockinfo->docktoindex = dockwithpointindex = 2;   //choose Out side to dock on
             customdockinfo->dockingindex = shippointindex = 4;
         }
         else if(dockwith->dockInfo->dockpoints[3].thisDockBusy == 0)   //head front
         {
             customdockinfo->docktoindex = dockwithpointindex = 3;   //choose Out side to dock on
             customdockinfo->dockingindex = shippointindex = 4;
            dbgMessagef("\nERROR..shouldn't get to this point!!!");
         }
         else
         {

#ifdef R2_DEBUG_RESEARCH_DOCKING
         dbgMessagef("\nRACE 2:  ARRIVED AT SELECTED SHIP, But nothing available.  Return true;");
#endif       //maybe reselect!
             return(TRUE);  //nothing available so poo!
         }

#ifdef R2_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 2: Research dockwith point %s chosen.",dockwithstatic->dockStaticInfo->dockstaticpoints[dockwithpointindex].name);
        dbgMessagef("\nRace 2: Research ship dock point %s chosen.",shipstatic->dockStaticInfo->dockstaticpoints[shippointindex].name);
#endif

        dockwithstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockwithpointindex];
        shipstaticpoint = &shipstatic->dockStaticInfo->dockstaticpoints[shippointindex];

        ship->dockvars.dockstaticpoint = dockwithstaticpoint;

        ship->dockvars.dockstate2 = RESEARCH_DOCK_FLYTOPIEPOINT;

        /* speechEvent: Guidance Lock Responding */
        if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
        {
            speechEventVar(ship, STAT_Research_Docking, 0, 0);
        }

        dockReserveDockPoint(ship,dockwith,dockwithpointindex);
        //this could have huge reprocussions!
        ship->dockInfo->dockpoints[shippointindex].thisDockBusy = 1;    //artificially busy point
        ship->dockvars.dockstate2 = R2_FLY_TO_DOCK_PATH_START;
        if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
        {
            goto dockinstantr2yeah1;
        }
        break;
    case R2_FLY_TO_DOCK_PATH_START:
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        bitSetAllSlaves(dockwith);
        if(R2atdockpathstart(ship,dockwith))
        {
dockinstantr2yeah1:
            ((ResearchShipSpec *)ship->ShipSpecifics)->busy_docking = FALSE;
            if(bitTest(dockwith->flags, SOF_Slaveable))
            {   //docked ship is a salve or master
                ship->dockingship = dockwith->slaveinfo->Master;
            }
            else if(dockwith->dockingship != NULL)
            {
                ship->dockingship = dockwith->dockingship;
            }
            else
            {    //first time...so dockwith 'will be a master' but isn't yet
                ship->dockingship = dockwith;
            }
            /* speechEvent: coming together... */
            ship->dockvars.dockstate2 = R2_ROTATE_AND_SPIN_INTO_PLACE;

            if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                goto r2dockinstantfinish;
            }

            speechEventVar(ship, STAT_Research_Docking, 0, 2);

#ifdef R2_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 2: Reached Docking outer point.  Begin backing in. ");
#endif
        }
        break;
    case R2_ROTATE_AND_SPIN_INTO_PLACE:
        if(dockwith->flags & SOF_Slaveable)
        {
            ((ResearchShipSpec *)(dockwith->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = TRUE;
        }
        bitSetAllSlaves(dockwith);

        if(R2spinintoplace(ship,dockwith,FINAL_POINT_TOL,R2_FINAL_ANGLE_TOL, TRUE))
        {
            //should do locking in here
r2dockinstantfinish:

            if(!bitTest(dockwith->flags,SOF_Slaveable))
            {
            //object isn't slaved, might not be a master
                if(dockwith->playerowner->shiptotals[ResearchShip] > 2)
                {
                    if(isThereAMasterForMe(ship))
                    {
                        //there exists a master somewheres, so hold horse
                        break;
                    }
                    //no master so let this guy dock...
                }
            }
           if(bitTest(dockwith->flags,SOF_Slaveable))
            {    //object is a master or slave
                if(bitTest(dockwith->slaveinfo->flags,SF_SLAVE))
                {    //object is a slave
                    tmpmaster = dockwith->slaveinfo->Master;
                }
                else
                {   // object is a master
                    tmpmaster = dockwith;
                }
            }
            else
            {    //object is 'the first'
                dockMakeMaster(dockwith);
                tmpmaster = dockwith;
            }
            bitClearAllSlaves(dockwith);

            if((docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                addMonkeyResearchShipChangePosition(dockwith,ship,customdockinfo->docktoindex);
            }
            dockAddSlave(tmpmaster,ship);       //enslaved!
            ((ResearchShipSpec *)(ship->slaveinfo->Master)->ShipSpecifics)->prepshipforanother = FALSE;

            /* finished docking */
            if(!(docktodo->dock.dockType & DOCK_INSTANTANEOUSLY))
            {
                speechEvent(ship, STAT_Research_Docked, 0);
                soundEvent(ship, Ship_ResearchLockOn);
            }

            ship->dockvars.dockstate2 = R2_DOCKED_AND_WAITING;
 #ifdef R2_DEBUG_RESEARCH_DOCKING
        dbgMessagef("\nRace 2: Successfully Parked Docking Ship...Now slaveing");
#endif
        }
        break;
    case R2_DOCKED_AND_SLAVING:
        break;
    case R2_DOCKED_AND_WAITING:
        break;
    default:
        dbgAssert(FALSE);
        break;

    }
    return FALSE;   //return that we aren't done docking...YET
}

/*=============================================================================
    General docking:
=============================================================================*/

// these routines will eventually not be used

bool LaunchShipFromDefaultShip(Ship *ship,Ship *dockwith)
{
    vector destination;
    vector heading;

    switch (ship->dockvars.dockstate3)
    {
        case 0:
            dockRemoveShipFromInside(ship,dockwith);
            dockPutShipOutside(ship,dockwith,&dockwith->posinfo.position,2,0);

            ship->dockvars.dockstate3 = 1;

            return FALSE;

        case 1:
            matGetVectFromMatrixCol3(heading,dockwith->rotinfo.coordsys);
            vecMultiplyByScalar(heading,1000.0f);

            vecAdd(destination,dockwith->posinfo.position,heading);

            if (MoveReachedDestinationVariable(ship,&destination,50.0f))
            {
                ship->dockingship = NULL;
                return TRUE;
            }
            else
            {
                aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,-40.0f,&dockwith->posinfo.velocity);
                return FALSE;
            }
    }

    return FALSE;
}

bool ShipDocksAtDefaultShip(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ship->dockingship = dockwith;
    if (MoveReachedDestinationVariable(ship,&dockwith->posinfo.position,50.0f))
    {
        return TRUE;
    }
    else
    {
        aishipFlyToShipAvoidingObjsWithVel(ship,dockwith,AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying,-40.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

/*=============================================================================
    Ship docking, at a high level (ignoring individual ship details)
=============================================================================*/

#define DOCKSHIP_NEARSHIP   1
#define DOCKSHIP_FLYBACK    2
#define DOCKSHIP_DONE       3

/*-----------------------------------------------------------------------------
    Name        : ShipDocksAtShip
    Description : code to handle a ship docking at another ship
    Inputs      : ship, dockwith
    Outputs     :
    Return      : 1 if ship is finished docking, 0 otherwise
----------------------------------------------------------------------------*/
udword ShipDocksAtShip(struct CommandToDo *docktodo,Ship *ship,Ship *dockwith)
{
    sdword returnval;
    vector desireddest;

    if (dockwith != NULL)
    {
        switch (ship->dockvars.dockstate)
        {
            case 0:
                if ( (docktodo->dock.dockType & DOCK_INSTANTANEOUSLY) ||
                     ((docktodo->dock.allNearShip) || ShipWithinDockRange(ship,dockwith)) )
                {
rdest:
                    ship->dockvars.dockstate = DOCKSHIP_NEARSHIP;
                    ship->dockvars.dockstate2 = 0;
                    vecSub(ship->dockvars.destination,ship->posinfo.position,dockwith->posinfo.position);
                    goto nearship;
                }
                else
                {
                    if (aishipFlyToShipAvoidingObjsWithVel(ship,dockwith,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_PointInDirectionFlying|AISHIP_CarTurn,0.0f,&dockwith->posinfo.velocity)
                        & AISHIP_FLY_OBJECT_IN_WAY)
                    {
                        goto rdest;
                    }
                }
                ship->shipidle = FALSE;
                return NOT_FINISHED_DOCKING;
nearship:
            case DOCKSHIP_NEARSHIP:
                returnval = dockwith->staticinfo->ShipXDocksAtShipY(docktodo,ship,dockwith);
                ship->shipidle = FALSE;
                if (returnval)
                {
                    ship->dockingship = NULL;
                    if (ship->dockvars.busyingThisShip != NULL)
                    {
                        dockUnbusyThisShip(ship);
                    }
                    if (returnval == PERMANENTLY_DOCKED)
                    {
                        ship->dockvars.dockstate = DOCKSHIP_DONE;
                        return PERMANENTLY_DOCKED;
                    }
                    else if (returnval == FINISHED_LAUNCHING_FROM_DOCKING)
                    {
                        ship->dockvars.dockstate = DOCKSHIP_DONE;
                        return FINISHED_LAUNCHING_FROM_DOCKING;
                    }
                    else if(docktodo->dock.dockType & DOCK_FOR_RETIRE)
                    {
                        return FINISHED_RETIREMENT;
                    }
                    else
                    {
                        ship->dockvars.dockstate = DOCKSHIP_FLYBACK;
                    }
                }
                return NOT_FINISHED_DOCKING;

            case DOCKSHIP_FLYBACK:
                if ((docktodo->dock.wasHarvesting) ||
                    ((ship->shiptype == ResourceCollector) &&
                        ((dockwith->shiptype == Mothership) || (dockwith->shiptype == Carrier)))
                   )

                {
                    ship->dockvars.dockstate = DOCKSHIP_DONE;
                    return FINISHED_DOCKING;        // return right away without flying back if harvesting
                }

                vecAdd(desireddest,dockwith->posinfo.position,ship->dockvars.destination);

                if (MoveReachedDestinationVariable(ship,&desireddest,BACKTODESTINATION_TOLERANCE))
                {
reacheddest:
//                    if (aitrackSteadyShip(ship))
//                    {
                        ship->dockvars.dockstate = DOCKSHIP_DONE;
                        return FINISHED_DOCKING;
//                    }
                }
                else
                {
#ifdef DEBUG_DOCKING
                    dbgMessagef("\nReturning from docking %x",(udword)ship);
#endif
                    if (aishipFlyToPointAvoidingObjsWithVel(ship,&desireddest,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_PointInDirectionFlying,BACKTODESTINATION_MINFLYSPEED,&dockwith->posinfo.velocity)
                        & AISHIP_FLY_OBJECT_IN_WAY)
                    {
                        goto reacheddest;
                    }
                }
                return FINISHED_DOCKING_FLYING_BACK;

            case DOCKSHIP_DONE:
                return FINISHED_DOCKING;

            default:
                dbgAssert(FALSE);
                return NOT_FINISHED_DOCKING;
        }
    }
    else
    {
        aitrackSteadyShip(ship);
        return FINISHED_DOCKING;
    }
}
/*=============================================================================
    P2 Fuel Pod Docking
=============================================================================*/

#define FUELPOD_WAITLATCHPOINT     1
#define FUELPOD_FLYTOINSIDECONE    2
#define FUELPOD_FLYTOCONEORIGIN    3
#define FUELPOD_REFUEL             4
#define FUELPOD_BACKUP             5
#define FUELPOD_WAIT               6

typedef struct
{
    sdword size;
    real32 dockmisc;
    real32 latchmintime;
    real32 timestamp;
} AtFuelPodDockInfo;

sdword P2FUELPOD_FightL0;
sdword P2FUELPOD_FightL1;
sdword P2FUELPOD_FightL2;
sdword P2FUELPOD_FightR0;
sdword P2FUELPOD_FightR1;
sdword P2FUELPOD_FightR2;

sdword *P2FuelPodFighterPoints[] = { &P2FUELPOD_FightL0, &P2FUELPOD_FightL1, &P2FUELPOD_FightL2,
                                    &P2FUELPOD_FightR0, &P2FUELPOD_FightR1, &P2FUELPOD_FightR2, NULL };

//sdword *P2FuelPodPoints = &P2FuelPodFighterPoints;

real32 *P2LatchDistance = &P2FUELPOD_FIGHTERLATCHDIST;

void P2FuelPodStaticDockInit(ShipStaticInfo *staticinfo)
{
    DockStaticInfo *dockStaticInfo = staticinfo->dockStaticInfo;

    P2FUELPOD_FightL0 = dockFindDockIndex("FightL0",dockStaticInfo);
    P2FUELPOD_FightL1 = dockFindDockIndex("FightL1",dockStaticInfo);
    P2FUELPOD_FightL2 = dockFindDockIndex("FightL2",dockStaticInfo);
    P2FUELPOD_FightR0 = dockFindDockIndex("FightR0",dockStaticInfo);
    P2FUELPOD_FightR1 = dockFindDockIndex("FightR1",dockStaticInfo);
    P2FUELPOD_FightR2 = dockFindDockIndex("FightR2",dockStaticInfo);
}

bool ShipDocksAtFuelPod(struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith)
{
    ShipStaticInfo *dockwithstatic = (ShipStaticInfo *)dockwith->staticinfo;
    DockStaticPoint *dockstaticpoint;
    vector conepositionInWorldCoordSys;
    vector coneheadingInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMagSqr;
    real32 coneToShipMag;
    real32 dotprod;
    AtFuelPodDockInfo *customdockinfo;
    sdword dockpointindex;
    ShipStaticInfo *shipstatic;

    switch (ship->dockvars.dockstate2)
    {
        case 0:
            dbgAssert(ship->dockvars.customdockinfo == NULL);
            customdockinfo = ship->dockvars.customdockinfo = memAlloc(sizeof(AtFuelPodDockInfo),"AtFuelPodDock",0);
            customdockinfo->size = sizeof(AtFuelPodDockInfo);

            shipstatic = ((ShipStaticInfo *)ship->staticinfo);

            if (shipstatic->shiptype != P2Swarmer && shipstatic->shiptype != P2AdvanceSwarmer)
                dbgAssert(FALSE);

            dbgAssert(dockwithstatic->shiprace == P2);
            customdockinfo->latchmintime = P2FUELPOD_LATCHMINTIME;

            ship->dockvars.dockstate2 = FUELPOD_WAITLATCHPOINT;

            // deliberately fall through to state FUELPOD_WAITLATCHPOINT

        case FUELPOD_WAITLATCHPOINT:
            customdockinfo = ship->dockvars.customdockinfo;

            dockpointindex = dockFindNearestDockPoint(&(P2FuelPodFighterPoints[0]), ship, dockwith,
                                                      &conepositionInWorldCoordSys, &coneToShip, &coneToShipMagSqr);

            if (dockpointindex == -1)
            {
                protectShip(ship,dockwith,FALSE);
                //aitrackSteadyShip(ship);
                return FALSE;
            }

            dockstaticpoint = &dockwithstatic->dockStaticInfo->dockstaticpoints[dockpointindex];

            ship->dockvars.dockstaticpoint = dockstaticpoint;

            dockReserveDockPoint(ship,dockwith,dockpointindex);

            matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

            coneToShipMag = fsqrt(coneToShipMagSqr);
            dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);  // 1.0*coneToShipMag*cos(theta)

            if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
            {
                // we are in the cone, so fly to centre of cone.
                ship->dockvars.dockstate2 = FUELPOD_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            else
            {
                customdockinfo->dockmisc = ClampToWithin(coneToShipMag,dockstaticpoint->mindist,dockstaticpoint->maxdist);
                ship->dockvars.dockstate2 = FUELPOD_FLYTOINSIDECONE;
            }
            return FALSE;

        case FUELPOD_FLYTOINSIDECONE:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeInside(ship,dockwith,customdockinfo->dockmisc))
            {
                madLinkInPreDockingShip(ship);
                ship->dockvars.dockstate2 = FUELPOD_FLYTOCONEORIGIN;
                ship->dockingship = dockwith;
            }
            return FALSE;

        case FUELPOD_FLYTOCONEORIGIN:
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockFlyToConeOriginLatchIfWithin(ship,dockwith, *P2LatchDistance))
            {
//                soundEvent(ship, Docking);
                ship->dockvars.dockstate2 = FUELPOD_REFUEL;
                clampObjToObj((SpaceObjRotImpTargGuidance *)ship,(SpaceObjRotImpTargGuidance *)dockwith);
            }
            return FALSE;

        case FUELPOD_REFUEL:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (dockShipRefuelsAtShip(ship,dockwith))
            {
//                soundEvent(dockwith, Acquire);
                customdockinfo = ship->dockvars.customdockinfo;
                customdockinfo->timestamp = universe.totaltimeelapsed + customdockinfo->latchmintime;
                ship->dockvars.dockstate2 = FUELPOD_WAIT;
            }
            return FALSE;

        case FUELPOD_WAIT:
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            customdockinfo = ship->dockvars.customdockinfo;
            if (universe.totaltimeelapsed > customdockinfo->timestamp)
            {
//                soundEvent(ship, Launch);
                ship->dockvars.dockstate2 = FUELPOD_BACKUP;
                unClampObj((SpaceObjRotImpTargGuidance *)ship);
            }
            return FALSE;

        case FUELPOD_BACKUP:
            if (dockBackoutOfConeLatch(ship,dockwith))
            {
                ship->dockingship = NULL;
                dockFreeDockPoint(ship,dockwith);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockInitializeCustomFunctions
    Description : intializees the custom docking function pointers in statinfo
    Inputs      : statinfo, type, race
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dockInitializeCustomFunctions(ShipStaticInfo *statinfo,ShipType type,ShipRace race)
{
    statinfo->ShipXDocksAtShipY = ShipDocksAtDefaultShip;
    statinfo->LaunchShipXFromShipY = LaunchShipFromDefaultShip;

    if (race == R1)
    {
        switch (type)
        {
            case AdvanceSupportFrigate:
                statinfo->ShipXDocksAtShipY = ShipDocksAtASF;
                statinfo->LaunchShipXFromShipY = NULL;
                R1ASFStaticDockInit(statinfo);
                break;

            case Carrier:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                R1CarrierStaticDockInit(statinfo);
                break;

            case Mothership:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                R1MothershipStaticDockInit(statinfo);
                break;

            case RepairCorvette:
                statinfo->ShipXDocksAtShipY = ShipDocksAtRepairCorvette;
                statinfo->LaunchShipXFromShipY = NULL;
                R1RepairCorvetteStaticDockInit(statinfo);
                break;

            case ResourceController:
                statinfo->ShipXDocksAtShipY = ShipDocksAtResController;
                statinfo->LaunchShipXFromShipY = NULL;
                R1ResControllerStaticDockInit(statinfo);
                break;
            case ResearchShip:
                statinfo->ShipXDocksAtShipY = R1ResearchShipDocksAtResearchShip;
                statinfo->LaunchShipXFromShipY = NULL;
                break;
            case ResourceCollector:
                statinfo->ShipXDocksAtShipY = ShipDocksAtResourceCollector;
                statinfo->LaunchShipXFromShipY = NULL;
                R1ResourceCollectorStaticDockInit(statinfo);
                break;
        }
    }
    else if (race == R2)
    {
        switch (type)
        {
            case AdvanceSupportFrigate:
                statinfo->ShipXDocksAtShipY = ShipDocksAtASF;
                statinfo->LaunchShipXFromShipY = NULL;
                R2ASFStaticDockInit(statinfo);
                break;
            case Carrier:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                R2CarrierStaticDockInit(statinfo);
                break;
            case Mothership:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                R2MothershipStaticDockInit(statinfo);
                break;

            case RepairCorvette:
                statinfo->ShipXDocksAtShipY = ShipDocksAtRepairCorvette;
                statinfo->LaunchShipXFromShipY = NULL;
                R2RepairCorvetteStaticDockInit(statinfo);
                break;
            case ResourceController:
                statinfo->ShipXDocksAtShipY = ShipDocksAtResController;
                statinfo->LaunchShipXFromShipY = NULL;
                R2ResControllerStaticDockInit(statinfo);
                break;
            case ResearchShip:
                statinfo->ShipXDocksAtShipY = R2ResearchShipDocksAtResearchShip;
                statinfo->LaunchShipXFromShipY = NULL;
                break;
            case ResourceCollector:
                statinfo->ShipXDocksAtShipY = ShipDocksAtResourceCollector;
                statinfo->LaunchShipXFromShipY = NULL;
                R2ResourceCollectorStaticDockInit(statinfo);
                break;
        }
    }
    else if (race == P1)
    {
        switch (type)
        {
            case P1Mothership:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                P1MothershipStaticDockInit(statinfo);
                break;
        }
    }
    else if (race == P2)
    {
        switch (type)
        {
            case P2Mothership:
                statinfo->ShipXDocksAtShipY = ShipDocksAtCarrierMother;
                statinfo->LaunchShipXFromShipY = LaunchShipFromCarrierMother;
                P2MothershipStaticDockInit(statinfo);
                break;
            case P2FuelPod:
                statinfo->ShipXDocksAtShipY = ShipDocksAtFuelPod;
                statinfo->LaunchShipXFromShipY = NULL;
                P2FuelPodStaticDockInit(statinfo);
                break;
        }
    }
    else if (race == Traders)
    {
        switch (type)
        {
            case JunkYardHQ:
                statinfo->ShipXDocksAtShipY = ShipDocksAtJunkYardHQ;
                statinfo->LaunchShipXFromShipY = NULL;
                JunkYardHQStaticDockInit(statinfo);
                break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : processDockToDo
    Description : processes a dock command
    Inputs      :
    Outputs     : resourceShip, resource
    Return      : returns TRUE if dock has completed
----------------------------------------------------------------------------*/
bool processDockToDo(CommandToDo *docktodo)
{
    sdword i;
    SelectCommand *selection = docktodo->selection;
    Ship *ship;
    Ship *dockwith;
    bool alldone = TRUE;
    udword returnval;

    for (i=0;i<selection->numShips; )
    {
        ship = selection->ShipPtr[i];
        dockwith = ship->dockvars.dockship;
        if (dockwith == NULL)
        {
            // ship died, let's potentially find a new one:
            DockType dockType = docktodo->dock.dockType;

            if (dockType & (DOCK_FOR_BOTHREFUELANDREPAIR|DOCK_FOR_REFUELREPAIR|DOCK_TO_DEPOSIT_RESOURCES|DOCK_PERMANENTLY|DOCK_FOR_RETIRE))
            {
                dockwith = FindNearestShipToDockAt(ship,dockType);
            }

            if (dockwith != NULL)
            {
                InitializeDockVars(ship,dockwith,dockType);
                InitShipAI(ship,TRUE);
                alldone = FALSE;
            }
        }
        else
        {
            returnval = ShipDocksAtShip(docktodo,ship,dockwith);
            if ((returnval == NOT_FINISHED_DOCKING) ||
                ((returnval == FINISHED_DOCKING_FLYING_BACK) && (!docktodo->dock.haveFlyedFarEnoughAway)))
            {
                alldone = FALSE;
            }
            else if(returnval == FINISHED_RETIREMENT)
            {
                bool removed;
                RemoveShipFromDocking(ship);
                if (docktodo->dock.wasHarvesting)
                {
                    //clear this flag so when returning from this loop, ship doesn't try to start harvesting again
                    docktodo->dock.wasHarvesting = FALSE;
                }
                removed = clRemoveShipFromSelection(selection,ship);
                dbgAssert(removed);
                RemoveShipFromCommand(ship);
                dockwith->playerowner->resourceUnits += (sdword) (TW_RETIRE_MONEYBACK_RATIO*ship->staticinfo->buildCost);
                universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalRegeneratedResourceUnits += (sdword) (TW_RETIRE_MONEYBACK_RATIO*ship->staticinfo->buildCost);
                universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnits += (sdword) (TW_RETIRE_MONEYBACK_RATIO*ship->staticinfo->buildCost);

                ship->howDidIDie = DEATH_BY_RETIREMENT;

                if(isCapitalShip(ship))
                {
                    //speech event for capital ship retired
                    if(ship->playerowner->playerIndex ==
                       universe.curPlayerIndex)
                    {
                        speechEventFleet(STAT_F_Cap_Retired, ship->shiptype, universe.curPlayerIndex);
                    }
                }
                univDeleteWipeInsideShipOutOfExistence(ship);
                continue;
            }
            else
            {
                RemoveShipFromDocking(ship);
                if (!docktodo->dock.wasHarvesting)
                {
                    // this ship is permanently docked, so we should remove it from docktodo->selection
                    bool removed = clRemoveShipFromSelection(selection,ship);
                    dbgAssert(removed);

                    RemoveShipFromCommand(ship);

                    if (docktodo->ordertype.attributes & COMMAND_IS_FORMATION)
                    {
                        // this ship was in formation so we have to do extra work
                        RemoveShipFromFormation(ship);
                        if (selection->numShips > 0)
                        {
                            formationContentHasChanged(docktodo);
                        }
                    }

                    if (docktodo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        RemoveShipFromAttacking(ship);
                        RemoveShipReferencesFromExtraAttackInfo(ship,docktodo);
                    }

                    if ((returnval == FINISHED_DOCKING) ||
                        ((returnval == FINISHED_DOCKING_FLYING_BACK) && (docktodo->dock.haveFlyedFarEnoughAway)))
                    {
                        // Let's put into formation and have them follow dockwith ship
                        CommandToDo *formationgroup = findFormationGuardingShip(dockwith);
                        if (formationgroup != NULL)
                        {
                            AddShipToFormationGroup(ship,formationgroup);
                        }
                        else
                        {
                            // create new formationgroup
                            SelectCommand selectone;
                            SelectCommand protectone;
                            TypeOfFormation formationtype;

                            docktodo->dock.haveFlyedFarEnoughAway = TRUE;

                            if ((ship->shiptype == ResourceCollector) &&
                                ((dockwith->shiptype == Mothership) || (dockwith->shiptype == Carrier)) )
                            {
                                GroupShip(&universe.mainCommandLayer,ship,dockwith);
                            }
                            else
                            {
                                if (docktodo->ordertype.attributes & COMMAND_IS_FORMATION)
                                {
                                    formationtype = docktodo->formation.formationtype;
                                }
                                else
                                {
                                    formationtype = DELTA_FORMATION;
                                }

                                selectone.numShips = 1;
                                selectone.ShipPtr[0] = ship;
                                protectone.numShips = 1;
                                protectone.ShipPtr[0] = dockwith;
                                clFormation(&universe.mainCommandLayer,&selectone,formationtype);
                                formationgroup = clProtect(&universe.mainCommandLayer,&selectone,&protectone);
                                formationgroup->protectFlags |= PROTECTFLAGS_JUST_FOLLOW;
                            }
                        }
                    }
                    else if (returnval == FINISHED_LAUNCHING_FROM_DOCKING)
                    {
                        GroupShip(&universe.mainCommandLayer,ship,dockwith);
                    }
                    // else if (returnval == PERMANENTLY_DOCKED) THEN DO NOTHING

                    continue;       // don't increment counter because we removed ShipPtr[i] and a new one
                                    // is now in its place
                }
            }
        }
        i++;
    }

    if (alldone)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockDrawDockInfo
    Description : draws docking info for debugging
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VISUALIZATION
void dockDrawDockInfo(Ship *ship)
{
    sdword i;
    DockStaticInfo *dockStaticInfo = ((ShipStaticInfo *)(ship->staticinfo))->dockStaticInfo;
    sdword numDockPoints;
    DockStaticPoint *dockstaticpoint;
    vector to;

    if (dockStaticInfo == NULL)
    {
        return;
    }

    numDockPoints = dockStaticInfo->numDockPoints;

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    for (i=0,dockstaticpoint=&dockStaticInfo->dockstaticpoints[0];i<numDockPoints;i++,dockstaticpoint++)
    {
        vecScalarMultiply(to,dockstaticpoint->conenormal,1000.0f);
        vecAddTo(to,dockstaticpoint->position);
        primLine3(&dockstaticpoint->position,&to,colWhite);
    }

    rndLightingEnable(TRUE);
}
#endif

bool ShipIsRefuelingAtCarrierMother(struct Ship *ship)
{
    if (ship->dockvars.dockship)
    {
        ShipStaticInfo *dockshipstatic = ship->dockvars.dockship->staticinfo;
        if (dockshipstatic->ShipXDocksAtShipY == ShipDocksAtCarrierMother)
        {
            if (ship->dockvars.dockstate2 == CARRIERMOTHERDOCK_REFUEL)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool ShipIsWaitingForSoftLaunch(struct Ship *ship)
{
    if (ship->dockvars.dockship)
    {
        ShipStaticInfo *dockshipstatic = ship->dockvars.dockship->staticinfo;
        if (dockshipstatic->ShipXDocksAtShipY == ShipDocksAtASF)
        {
            if (ship->dockvars.dockstate2 == ASF_WAIT)
            {
                return TRUE;
            }
        }
        else if (dockshipstatic->ShipXDocksAtShipY == ShipDocksAtCarrierMother)
        {
            if (ship->dockvars.dockstate2 == CARRIERMOTHERDOCK_LATCHWAIT)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

CommandToDo *findFormationGuardingShip(Ship *ship)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if ((command->ordertype.attributes & (COMMAND_IS_PROTECTING|COMMAND_IS_FORMATION)) == (COMMAND_IS_PROTECTING|COMMAND_IS_FORMATION))
        {
            if (ShipInSelection(command->protect,ship))
            {
                return command;
            }
        }

        curnode = curnode->next;
    }

    return NULL;
}

