/*=============================================================================
    Name    : ResearchShip.c
    Purpose : Specifics for the ResearchShip

    Created 01/06/98  Bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include "Types.h"
#include "SpaceObj.h"
#include "ResearchShip.h"
#include "UnivUpdate.h"
#include "SoundEvent.h"
#include "StatScript.h"
#include "AITrack.h"
#include "Matrix.h"
#include "AIShip.h"
#include "Debug.h"
#include "FastMath.h"
#include "Select.h"
#include "Universe.h"
#include "LinkedList.h"
#include "SaveGame.h"
#include "Dock.h"

#define ROTATE_WAIT 0
#define ROTATE_DO   1
#define ROTATE_STOP 2
#define ROTATE_STOP_QUICK 3
#define ANGLE_ESTABLISH   4

//Constants to determine if a ship is moving or whether it isn't moving
#define NOTMOVINGNEG    0.0f
#define NOTMOVINGPOS    0.0f

ResearchShipStatics ResearchShipStatic;

ResearchShipStatics ResearchShipStaticRace1;
ResearchShipStatics ResearchShipStaticRace2;

scriptStructEntry ResearchShipStaticScriptTable[] =
{
    { "R1final_dock_distance",    scriptSetReal32CB, (udword) &(ResearchShipStatic.R1final_dock_distance), (udword) &(ResearchShipStatic) },
    { "R1parallel_dock_distance",    scriptSetReal32CB, (udword) &(ResearchShipStatic.R1parallel_dock_distance), (udword) &(ResearchShipStatic) },
    { "R1VerticalDockDistance",    scriptSetReal32CB, (udword) &(ResearchShipStatic.R1VerticalDockDistance), (udword) &(ResearchShipStatic) },

    { "max_rotate",    scriptSetReal32CB, (udword) &(ResearchShipStatic.max_rotate), (udword) &(ResearchShipStatic) },
    { "rotate_acelleration",    scriptSetReal32CB, (udword) &(ResearchShipStatic.rotate_acelleration), (udword) &(ResearchShipStatic) },
    { "rotate_slow",    scriptSetReal32CB, (udword) &(ResearchShipStatic.rotate_slow), (udword) &(ResearchShipStatic) },
    { "R2DockFinalDistance",    scriptSetReal32CB, (udword) &(ResearchShipStatic.R2DockFinalDistance), (udword) &(ResearchShipStatic) },
    { "RotationAngle",    scriptSetReal32CB, (udword) &(ResearchShipStatic.RotationAngle), (udword) &(ResearchShipStatic) },


    { NULL,NULL,0,0 }
};

void ResearchShipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    ResearchShipStatics *researchstat = (statinfo->shiprace == R1) ? &ResearchShipStaticRace1 : &ResearchShipStaticRace2;

    statinfo->custstatinfo = researchstat;
    scriptSetStruct(directory,filename,ResearchShipStaticScriptTable,(ubyte *)researchstat);
}

void ResearchShipInit(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;
    spec->seed = FALSE;
    spec->dockers = 0;              //this variable is questionable
    spec->done = FALSE;
    spec->master = FALSE;
    spec->prepshipforanother = 0;
    spec->rotate_state = 0;
    spec->theta = 0.0f;
    spec->busy_docking = FALSE;
    spec->have_removed_from_parade = FALSE;
    spec->masterptr = NULL;
    spec->dockwith = NULL;
    spec->dockordernumber = 0;
    ship->dockvars.reserveddocking = -1;            //initialize since doesn't seem to be initialized

    /////////////////
    //research station online message

    speechEvent(ship,STAT_Research_StationOnline,0);

}

bool ship_is_moving(Ship *ship)
{
    CommandToDo *commy = getShipAndItsCommand(&universe.mainCommandLayer,ship);
    if(commy != NULL)
    {
        if(commy->ordertype.order == COMMAND_MOVE)
        {
            return TRUE;
        }
    }
    return FALSE;
    /*
    if(isBetweenInclusive(ship->posinfo.velocity.x,NOTMOVINGNEG,NOTMOVINGPOS))
    {
        if(isBetweenInclusive(ship->posinfo.velocity.y,NOTMOVINGNEG,NOTMOVINGPOS))
        {
            if(isBetweenInclusive(ship->posinfo.velocity.z,NOTMOVINGNEG,NOTMOVINGPOS))
            {
                return(FALSE);
            }
        }
    }
    return(TRUE);
    */
}

void make_all_slaves_moving(Ship *ship)
{
    Node *slavenode;
    Ship *slave;

    dbgAssert(ship->flags & SOF_Slaveable);     //ship should be slaveable

    slavenode = ship->slaveinfo->slaves.head;
    while (slavenode != NULL)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);
        slave->posinfo.isMoving = ISMOVING_MOVING | ISMOVING_ROTATING;
        slavenode = slavenode->next;
    }

}
void getRotatePoint(Ship *ship, vector *point, real32 *distance)
{
    Node *slavenode;
    Ship *slave;
    udword count;
    vector dist;

    dbgAssert(ship->flags & SOF_Slaveable);     //ship should be slaveable
    dbgAssert(ship->slaveinfo->slaves.num >= 5);    //need enough for a pie plate!

    slavenode = ship->slaveinfo->slaves.head;
    point->x = ship->posinfo.position.x;
    point->y = ship->posinfo.position.y;
    point->z = ship->posinfo.position.z;

    count = 0;
    while (count < 5)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);
        vecAdd(*point, *point, slave->posinfo.position);
        slavenode = slavenode->next;
        count++;
    }
    point->x /= 6.0f;
    point->y /= 6.0f;
    point->z /= 6.0f;

    vecSub(dist,ship->posinfo.position, *point);
    *distance = vecMagnitudeSquared(dist);
    *distance = fsqrt(*distance);
}

void ResearchShipHouseKeep(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;
    SelectCommand selection;
    vector up,destination,desiredheading,newup;
    vector univup = {0.0f,0.0f,1.0f};
    bool InParadeandMoving = FALSE;
    matrix rot_matrix,tmpmat,rotmat;
    real32 radangle;
    ResearchShipStatics *researchshipstatics;
    researchshipstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(bitTest(ship->specialFlags,SPECIAL_StopForResearchDocking))
    {
        vecScalarMultiply(ship->posinfo.velocity,ship->posinfo.velocity,0.94);
        vecScalarMultiply(ship->rotinfo.rotspeed,ship->rotinfo.rotspeed,0.94);
        bitSet(ship->dontapplyforceever,1);
        bitSet(ship->dontrotateever,1);
    }
    else
    {
        bitClear(ship->dontapplyforceever,1);
        bitClear(ship->dontrotateever,1);
    }

    if(ship->flags & SOF_Slaveable)
    {
        if(ship->slaveinfo->flags & SF_MASTER)
        {
            CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
            if(command != NULL)
            {
                //ship may be in parade...
                if(command->ordertype.order == COMMAND_MILITARYPARADE)
                {
                    //ship is in parade
                    CommandToDo *ashipCom = getShipAndItsCommand(&universe.mainCommandLayer,command->militaryParade->aroundShip);
                    if(ashipCom != NULL)
                    {
                        //probably shouldn't be null regardless..should be equal to command..but ohwell
                        if(ashipCom->ordertype.order == COMMAND_MOVE ||
                           ashipCom->ordertype.order == COMMAND_ATTACK ||
                           ashipCom->ordertype.order == COMMAND_SPECIAL ||
                           ashipCom->ordertype.order == COMMAND_MP_HYPERSPACEING)
                        {
                            InParadeandMoving = TRUE;
                            spec->rotate_state = ROTATE_WAIT;
                        }
                    }
                //in parade
                    if(!InParadeandMoving)
                    {
                        if(bitTest(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser))
                        {
                            InParadeandMoving = TRUE;
                            spec->rotate_state = ROTATE_WAIT;
                        }
                    }
                }
            }
        }
    }
    if(ship->flags & SOF_Slaveable)
    {    //ship is slaveable
        bitSet(ship->flags,SOF_DontDrawTrails);
        ship->autostabilizeship = FALSE;        // never have univupdate autostabilize research ships
        if(ship->slaveinfo->flags & SF_MASTER)
        {    //ship is a master
            if(!InParadeandMoving)
            {
                if(ship->shiprace == R1)
                {   //ship is a race 1 MASTER research ship...
                    //i.e. it controls the motion
                    switch(spec->rotate_state)
                    {
                    case ANGLE_ESTABLISH:       //establishes the angle at which the station is supposed to rotate at
                        break;
                    case ROTATE_WAIT:           //wait to rotate
                        if(ship->slaveinfo->slaves.num >= 5)
                        {   //correct # of ships...
                            if(!ship_is_moving(ship))
                            {
                                if(!spec->prepshipforanother)
                                {   //no ships are coming to dock
                                    //do up vector tracking!
                                    //Later Calculate this vector ONCE at code start up...Let Daly do it
                                    //and get as a static value

                                    radangle = DEG_TO_RAD(researchshipstatics->RotationAngle);
                                    matMakeRotAboutX(&rotmat,(real32) cos(radangle),(real32) sin(radangle));
                                    matMultiplyMatByVec(&newup, &rotmat, &univup);
                                    //matGetVectFromMatrixCol3(desiredHeading,ship->rotinfo.coordsys);
                                    if(aitrackHeadingWithFlags(ship,&newup,0.96f,AITRACKHEADING_IGNOREUPVEC))
                                    {
                                        spec->rotate_state = ROTATE_DO;
                                        getRotatePoint(ship, &spec->rotate_point,&spec->rotate_distance);
                                    }
                                }
                            }
                            if(!spec->have_removed_from_parade)
                            {
                                spec->have_removed_from_parade = TRUE;
                                selection.numShips = 1;
                                selection.ShipPtr[0] = ship;
                                //RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selection);
                                //clHalt(&universe.mainCommandLayer,&selection);
                            }
                        }
                        break;
                    case ROTATE_STOP:
                    case ROTATE_STOP_QUICK:
                    case ROTATE_DO:
                        ship->posinfo.isMoving = ISMOVING_MOVING | ISMOVING_ROTATING;
                        make_all_slaves_moving(ship);
                        if(ship->slaveinfo->slaves.num < 5)
                        {
                            //don't rotate anymore because slaves dropped below threhold for whatever reasons!
                            spec->rotate_state = ROTATE_STOP;
                            break;
                        }
                        if(spec->prepshipforanother)
                        {   //need to prep ship for a docking ship..
                            spec->rotate_state = ROTATE_STOP;
                        }
                        else if(ship_is_moving(ship))
                        {   //ship is being moved
                            spec->rotate_state = ROTATE_STOP_QUICK;
                        }
                        else
                        {
                            spec->rotate_state = ROTATE_DO;
                        }

                        //matGetVectFromMatrixCol3(heading,ship->rotinfo.coordsys);
                        matGetVectFromMatrixCol1(up,ship->rotinfo.coordsys);
                        //vecScalarMultiply(rotate_point, heading, -250.0f);
                        //vecAdd(rotate_point,rotate_point, ship->posinfo.position);

                        switch(spec->rotate_state)
                        {
                        case ROTATE_DO:
                            if(spec->theta >= researchshipstatics->max_rotate)
                                spec->theta = researchshipstatics->max_rotate;
                            else
                                spec->theta += researchshipstatics->rotate_acelleration;
                            break;
                        case ROTATE_STOP_QUICK:
                            spec->theta *= researchshipstatics->rotate_slow;
                            spec->theta *= researchshipstatics->rotate_slow;
                        case ROTATE_STOP:
                            if(spec->theta <= 0.00001f)
                            {
                                spec->theta = 0.0f;
                                spec->rotate_state = ROTATE_WAIT;
                                break;
                            }
                            else
                                spec->theta *= researchshipstatics->rotate_slow;
                            break;
                        default:
                            dbgAssert(FALSE);
                            break;
                        }
                        matMakeRotAboutX(&rot_matrix,(real32) cos(spec->theta),(real32) sin(spec->theta));
                        //matMultiplyMatByMat(&tmpmat, &rot_matrix, &ship->rotinfo.coordsys);
                        matMultiplyMatByMat(&tmpmat, &ship->rotinfo.coordsys, &rot_matrix);
                        //vecSub(tmpvec, ship->posinfo.position, spec->rotate_point);
                        //dist = vecMagnitudeSquared(tmpvec);
                        //dist = fsqrt(dist);

                        matGetVectFromMatrixCol3(desiredheading,tmpmat);
                        vecScalarMultiply(destination, desiredheading, spec->rotate_distance);      //old was dist
                        vecAdd(destination,destination,spec->rotate_point);

                        ship->posinfo.position = destination;
                        ship->rotinfo.coordsys = tmpmat;
                        univUpdateObjRotInfo((SpaceObjRot *)ship);
                        break;
                    default:
                        dbgMessagef("\nShouldn't Get Here...unknown Research Ship Rotate State");
                        dbgAssert(FALSE);
                        break;
                    }


                }
                else
                {    //Ship is an R2 Master...so rotate differently
                    switch(spec->rotate_state)
                    {
                    case ANGLE_ESTABLISH:
                        break;
                    case ROTATE_WAIT:           //wait to rotate
                        if(ship->slaveinfo->slaves.num >= 3)
                        {   //correct # of ships...
                            if(!spec->prepshipforanother && !ship_is_moving(ship))
                            {   //no ships are coming to dock
                                radangle = DEG_TO_RAD(researchshipstatics->RotationAngle);
                                matMakeRotAboutX(&rotmat,(real32) cos(radangle),(real32) sin(radangle));
                                matMultiplyMatByVec(&newup, &rotmat, &univup);
                                if(aitrackHeadingWithFlags(ship,&newup,0.96f,AITRACKHEADING_IGNOREUPVEC))
                                {
                                    spec->rotate_state = ROTATE_DO;
                                }
                            }
                            else
                            {
                                vecSet(ship->rotinfo.rotspeed,0.0f,0.0f,0.0f);
                                vecSet(ship->rotinfo.torque,0.0f,0.0f,0.0f);
                            }
                        }
                        if(ship->slaveinfo->slaves.num >= 1)
                        {   //correct # of ships...
                            if(!spec->have_removed_from_parade)
                            {
                                spec->have_removed_from_parade = TRUE;
                                selection.numShips = 1;
                                selection.ShipPtr[0] = ship;
                                //RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selection);
                                //clHalt(&universe.mainCommandLayer,&selection);
                            }
                        }
                        break;
                    case ROTATE_STOP:
                    case ROTATE_STOP_QUICK:
                    case ROTATE_DO:
                        ship->posinfo.isMoving = ISMOVING_MOVING | ISMOVING_ROTATING;
                        make_all_slaves_moving(ship);
                        if(ship->slaveinfo->slaves.num < 3)
                        {
                            //don't rotate anymore because slaves dropped below threhold for whatever reasons!
                            spec->rotate_state = ROTATE_STOP;
                            break;
                        }
                        if(spec->prepshipforanother)
                        {   //need to prep ship for a docking ship..
                            spec->rotate_state = ROTATE_STOP;
                        }
                        if(ship_is_moving(ship))
                        {   //ship is being moved
                            spec->rotate_state = ROTATE_STOP_QUICK;
                        }
                        switch(spec->rotate_state)
                        {
                        case ROTATE_DO:
                            if(spec->theta >= researchshipstatics->max_rotate)
                                spec->theta = researchshipstatics->max_rotate;
                            else
                                spec->theta += researchshipstatics->rotate_acelleration;
                            break;
                        case ROTATE_STOP_QUICK:
                            spec->theta *= researchshipstatics->rotate_slow;
                            spec->theta *= researchshipstatics->rotate_slow;
                        case ROTATE_STOP:
                            if(spec->theta <= 0.00001f)
                                {
                                spec->theta = 0.0f;
                                spec->rotate_state = ROTATE_WAIT;
                                break;
                                }
                            else
                                spec->theta *= researchshipstatics->rotate_slow;
                            break;
                        default:
                            dbgAssert(FALSE);
                            break;
                        }
                        matMakeRotAboutZ(&rot_matrix,(real32) cos(spec->theta),(real32) sin(spec->theta));
                        tmpmat = ship->rotinfo.coordsys;
                        matMultiplyMatByMat(&ship->rotinfo.coordsys,&tmpmat,&rot_matrix);
                        univUpdateObjRotInfo((SpaceObjRot *)ship);
                        break;
                    default:
                        dbgMessagef("\nShouldn't Get Here...unknown Research Ship Rotate State");
                        dbgAssert(FALSE);
                        break;
                    }
                }
            }
        }
        else
        {
            vecScalarMultiply(ship->posinfo.velocity,ship->posinfo.velocity, 0.0f);
        }

    }
}
void CleanResearchShip(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;
    udword count;

    spec->seed = FALSE;
    spec->dockers = 0;
    spec->done = FALSE;
    spec->master = FALSE;
    spec->prepshipforanother = 0;
    spec->rotate_state = 0;
    spec->theta = 0.0f;
    spec->busy_docking = FALSE;
    spec->masterptr = NULL;
    spec->dockwith = NULL;
    spec->have_removed_from_parade = FALSE;
    if(ship->dockInfo != NULL)
    {
        for(count = 0;count < ship->dockInfo->numDockPoints; count++)
        {
            ship->dockInfo->dockpoints[count].thisDockBusy = 0;   //debusy dockpoints
        }
    }
    ship->dockvars.reserveddocking = -1;
}

void ResearchShipMakeReadyForHyperspace(Ship *ship)
{
//    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;
    if(ship->flags & SOF_Slaveable)
    {
        bitClear(ship->slaveinfo->Master->specialFlags,SPECIAL_StopForResearchDocking);
        dockCrushMaster(ship->slaveinfo->Master);
    }
/*
    if(ship->dockvars.reserveddocking != -1)
    {
        sdword dockpointindex;

        dockpointindex = ship->dockvars.dockstaticpoint->dockindex;

        dbgAssert(ship->dockvars.reserveddocking == (sbyte)dockpointindex);
        ship->dockvars.reserveddocking = -1;

        if(spec->dockwith != NULL)
        {
            dbgAssert(spec->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy > 0);
            spec->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy--;
        }
    }
*/
    CleanResearchShip(ship);
}

void ResearchShipDied(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;
    Node *shipnode = universe.ShipList.head;
    Ship *obj;
    MaxSelection selection;

    //need to slow down/stop first research ship and turn it into a new master!
    if(ship->dockvars.reserveddocking != -1)
    {
        sdword dockpointindex;
        CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
        if(command != NULL)
        {
            if(command->ordertype.order == COMMAND_DOCK)
            {
                if(command->dock.dockType != DOCK_FOR_RETIRE)
                {
                    dockpointindex = ship->dockvars.dockstaticpoint->dockindex;

                    dbgAssert(ship->dockvars.reserveddocking == (sbyte)dockpointindex);
                    ship->dockvars.reserveddocking = -1;

                    if(spec->dockwith != NULL)
                    {
                        dbgAssert(spec->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy > 0);
                        spec->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy--;
                    }
                }
            }
        }
    }
    if(spec->dockwith != NULL)
    {
        if(ship->shiprace == R1)
        {
            ((ResearchShipSpec *)spec->dockwith->ShipSpecifics)->seed = TRUE;    //set correct seeding
            ((ResearchShipSpec *)spec->dockwith->ShipSpecifics)->done = FALSE;
        }
        else
        {
            ((ResearchShipSpec *)spec->dockwith->ShipSpecifics)->seed = TRUE;    //set correct seeding
            ((ResearchShipSpec *)spec->dockwith->ShipSpecifics)->done = FALSE;
        }
    }
    selection.numShips = 0;
    if(ship->flags & SOF_Slaveable)
    {   //ship is slaved
        if(ship->slaveinfo->flags & SF_MASTER)
        {    //ship is a master
            //ship that died is a master!  no worries
            while(shipnode != NULL)
            {
                obj = (Ship *) listGetStructOfNode(shipnode);
                if(obj->shiptype == ResearchShip)
                {
                    if(obj->playerowner == ship->playerowner)
                    {
                        if(!bitTest(obj->flags,SOF_Slaveable))
                        {
                            if(obj != ship)
                            {
                                CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,obj);
                                if(command != NULL
                                   && command->ordertype.order == COMMAND_DOCK
                                   && (command->dock.dockType & DOCK_FOR_RETIRE||command->dock.dockType & DOCK_PERMANENTLY))
                                {
                                    //don't reorder a dock for research
                                }
                                else if(obj->flags & SOF_Dead)
                                {
                                }
                                else if(!bitTest(obj->flags,SOF_Dead))
                                {   //as long as ship isn't considered dead
                                    //need better method
                                    CleanResearchShip(obj);
                                    selection.ShipPtr[selection.numShips] = obj;
                                    selection.numShips++;
                                }
                            }
                        }
                    }
                }
                shipnode = shipnode->next;
            }
        }
        else
        {
            //ship that died isn't a master
        }
    }
    else
    {
        while(shipnode != NULL)
        {
            obj = (Ship *) listGetStructOfNode(shipnode);
            if(obj->shiptype == ResearchShip)
            {
                if(obj->playerowner == ship->playerowner)
                {
                    if(obj != ship)
                    {
                        if(((ResearchShipSpec *) obj->ShipSpecifics)->dockordernumber > spec->dockordernumber)
                        {
                            CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,obj);
                            if(command != NULL
                               && command->ordertype.order == COMMAND_DOCK
                               && (command->dock.dockType & DOCK_FOR_RETIRE||command->dock.dockType & DOCK_PERMANENTLY))
                            {
                                //don't reorder a dock for research
                            }
                            else if(obj->flags & SOF_Dead)
                            {
                            }
                            else
                            {
                                if(obj->dockvars.reserveddocking != -1)
                                {
                                    sdword dockpointindex;

                                    dockpointindex = obj->dockvars.dockstaticpoint->dockindex;

                                    dbgAssert(obj->dockvars.reserveddocking == (sbyte)dockpointindex);
                                    obj->dockvars.reserveddocking = -1;

                                    //dbgAssert(((ResearchShipSpec *) obj->ShipSpecifics)->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy > 0);

                                    if (((ResearchShipSpec *) obj->ShipSpecifics)->dockwith) // Bryce did I fix this right by putting in this check dockwith != NULL
                                        ((ResearchShipSpec *) obj->ShipSpecifics)->dockwith->dockInfo->dockpoints[dockpointindex].thisDockBusy = 0;

                                }
                                CleanResearchShip(obj);

                                dbgAssert(selection.numShips < COMMAND_MAX_SHIPS);
                                selection.ShipPtr[selection.numShips] = obj;
                                selection.numShips++;
                                //ship->dockInfo->dockpoints[shippointindex].thisDockBusy = 0;    //artificially busy point
                                //selection.ShipPtr[selection.numShips] = obj;
                                //selection.numShips++;
                            }
                        }
                    }
                }
            }
            shipnode = shipnode->next;
        }
    }
    if(selection.numShips >= 1)
    {
        clDock(&universe.mainCommandLayer, (SelectCommand *)&selection, DOCK_FOR_RESEARCH, NULL);
    }

}

/*-----------------------------------------------------------------------------
    Name        : toFakeOneship
    Description : Used to 'correct' movement circles for a Slaved ship (which should be a master)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//to OPTIMIZE THIS:  ONLY CALCULATE SLAVED SHIPS AVERAGE POSITION when slaves are added!
void toFakeOneShip(Ship *ship, vector *oldpos, real32 *oldradius)
{
    Node *slavenode;
    Ship *slave;
    vector newcenter;
    real32 newradius,tempreal;

    if(!(ship->flags & SOF_Slaveable))
        return;

    dbgAssert(ship->slaveinfo->flags & SF_MASTER);      //must be a master
    slavenode = ship->slaveinfo->slaves.head;

    newcenter = ship->collInfo.collPosition;

    while(slavenode != NULL)
    {   //add all slaves positions together
        slave = (Ship *) listGetStructOfNode(slavenode);
        vecAddTo(newcenter,slave->collInfo.collPosition);
        slavenode = slavenode->next;
    }
    vecDivideByScalar(newcenter,(float) ship->slaveinfo->slaves.num+1,tempreal);   //take average of slavemaster position

    if(ship->shiprace == R1)
    {
        newradius = ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize*2.5f;
    }
    else
    {   //Must be R2
        newradius = (float) ship->slaveinfo->slaves.num+1;
        newradius = ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize*(newradius/4.0f)*1.25f;

    }

    *oldpos = ship->collInfo.collPosition;
    *oldradius = ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize = newradius;

    ship->collInfo.collPosition = newcenter;

}

void toUnFakeOneShip(Ship *ship, vector *oldpos,real32 *oldradius)
{
    if(!(ship->flags & SOF_Slaveable))
        return;

    ship->collInfo.collPosition = *oldpos;
    ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize = *oldradius;

}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void ResearchShip_PreFix(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;

    spec->masterptr = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->masterptr);
    spec->dockwith = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->dockwith);
}

void ResearchShip_Fix(Ship *ship)
{
    ResearchShipSpec *spec = (ResearchShipSpec *)ship->ShipSpecifics;

    spec->masterptr = SpaceObjRegistryGetShip((sdword)spec->masterptr);
    spec->dockwith = SpaceObjRegistryGetShip((sdword)spec->dockwith);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

void addMonkeyResearchShipChangePosition(Ship *dockwith, Ship *ship,sdword dockindex)
{
    DockStaticPoint *dockwithstaticpoint;
    vector coneheadingInWorldCoordSysDockWith,DockWithHeading,destination,DockWithUp,destinationoffset;
    vector desiredHeading,desiredUp,conepositionInWorldCoordSysDockWith,tmpvec;
    real32 theta;
    matrix rotmatrix,tmpmat;

    ResearchShipStatics *resstatics;
    resstatics = (ResearchShipStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;


    dbgAssert(dockwith != NULL);    //if we're calling this there should be another reserach station available somewhere

    if(ship->shiprace == R1)
    {
        dockwithstaticpoint = &dockwith->staticinfo->dockStaticInfo->dockstaticpoints[dockindex];

        matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
        matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)


        destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*resstatics->R1final_dock_distance;
        destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*resstatics->R1final_dock_distance;
        destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*resstatics->R1final_dock_distance;
        vecAdd(destination,dockwith->posinfo.position, destinationoffset);
        if(((ResearchShipSpec *)ship->ShipSpecifics)->pie_plate_num == 0)
        {    //ship is docking on'a'top so add upwards factor
            matGetVectFromMatrixCol1(DockWithUp,dockwith->rotinfo.coordsys);
            vecScalarMultiply(DockWithUp,DockWithUp,resstatics->R1VerticalDockDistance);
            vecAdd(destination,destination,DockWithUp);
        }

        theta = DEG_TO_RAD(60);

        matMakeRotAboutX(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
        matMultiplyMatByMat(&tmpmat, &dockwith->rotinfo.coordsys, &rotmatrix);

        //share a lot of these things...later...
        ship->rotinfo.coordsys = tmpmat;
        ship->posinfo.position = destination;
    }
    else
    {
        //r2 positioning
        dockwithstaticpoint = &dockwith->staticinfo->dockStaticInfo->dockstaticpoints[dockindex];

        matMultiplyMatByVec(&coneheadingInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->conenormal);
        matMultiplyMatByVec(&conepositionInWorldCoordSysDockWith,&dockwith->rotinfo.coordsys,&dockwithstaticpoint->position);
        vecAddTo(conepositionInWorldCoordSysDockWith,dockwith->posinfo.position);
        matGetVectFromMatrixCol3(DockWithHeading,dockwith->rotinfo.coordsys)

        destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*resstatics->R2DockFinalDistance;
        destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*resstatics->R2DockFinalDistance;
        destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*resstatics->R2DockFinalDistance;
        vecAdd(destination,conepositionInWorldCoordSysDockWith, destinationoffset);

        desiredHeading = coneheadingInWorldCoordSysDockWith;
        if(dockindex == 0)
        {
            matGetVectFromMatrixCol3(desiredUp,dockwith->rotinfo.coordsys);
        }
        else if(dockindex == 1)
        {
            matGetVectFromMatrixCol3(desiredUp,dockwith->rotinfo.coordsys);
        }
        else if(dockindex == 2)
        {
            desiredUp = DockWithHeading;
        }
        else if(dockindex == 3)
        {
            theta = DEG_TO_RAD(60);
            matMakeRotAboutZ(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
            matMultiplyMatByMat(&tmpmat, &dockwith->rotinfo.coordsys, &rotmatrix);
            matGetVectFromMatrixCol1(desiredUp,tmpmat);
        }
        else if(dockindex == 4)
        {
            destinationoffset.x = coneheadingInWorldCoordSysDockWith.x*100;
            destinationoffset.y = coneheadingInWorldCoordSysDockWith.y*100;
            destinationoffset.z = coneheadingInWorldCoordSysDockWith.z*100;
            vecAdd(destination,conepositionInWorldCoordSysDockWith, destinationoffset);

            matGetVectFromMatrixCol1(desiredUp,dockwith->rotinfo.coordsys);
            vecScalarMultiply(desiredHeading,desiredHeading,-1.0f);
        }
        else
        {
            dbgAssert(FALSE);       //shouldget here.
        }

        vecNormalize(&desiredUp);

        matPutVectIntoMatrixCol1(desiredUp,ship->rotinfo.coordsys);
        vecCrossProduct(tmpvec,desiredHeading,desiredUp);
        matPutVectIntoMatrixCol2(tmpvec,ship->rotinfo.coordsys);
        matPutVectIntoMatrixCol3(desiredHeading,ship->rotinfo.coordsys);

        ship->posinfo.position = destination;
        ship->posinfo.velocity.x = 0.0f;
        ship->posinfo.velocity.y = 0.0f;
        ship->posinfo.velocity.z = 0.0f;
    }
}


//custom monkey function to add res ships to worldand make them orient themselves correctly
void addMonkeyResearchShip(Ship *ship)
{
    SelectCommand selectOne;
    selectOne.numShips=1;
    selectOne.ShipPtr[0]=ship;

    clDock(&universe.mainCommandLayer,&selectOne,DOCK_FOR_RESEARCH|DOCK_INSTANTANEOUSLY,NULL);


}

CustShipHeader ResearchShipHeader =
{
    ResearchShip,
    sizeof(ResearchShipSpec),
    ResearchShipStaticInit,
    NULL,
    ResearchShipInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ResearchShipHouseKeep,
    NULL,
    ResearchShipDied,
    ResearchShip_PreFix,
    NULL,
    NULL,
    ResearchShip_Fix
};
