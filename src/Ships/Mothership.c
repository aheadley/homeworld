/*=============================================================================
    Name    : Mothership.c
    Purpose : Specifics for the Mothership

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Debug.h"
#include "SpaceObj.h"
#include "Mothership.h"
#include "StatScript.h"
#include "Gun.h"
#include "Attack.h"
#include "DefaultShip.h"
#include "MadLinkIn.h"
#include "SaveGame.h"
#include "Universe.h"
#include "AITrack.h"
#include "SalCapCorvette.h"
#include "MadLinkInDefs.h"
#include "CommandLayer.h"

extern sdword R1MOTHERSHIP_Big;
extern sdword R2MOTHERSHIP_Big;

MothershipStatics MothershipStatic;

MothershipStatics MothershipStaticRace1;
MothershipStatics MothershipStaticRace2;

//I don't envision any more ships attaching themselves to the mothership door...should they then hooray!
scriptStructEntry MothershipStaticTable[] =
{
    { "specialDoorOffset[R1][AdvanceSupportFrigate]",   scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][AdvanceSupportFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][Carrier]",                 scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][Carrier]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][CloakGenerator]",          scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][CloakGenerator]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][DDDFrigate]",              scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][DDDFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][DFGFrigate]",              scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][DFGFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][GravWellGenerator]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][GravWellGenerator]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][HeavyCruiser]",            scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][HeavyCruiser]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][IonCannonFrigate]",        scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][IonCannonFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][MissileDestroyer]",        scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][MissileDestroyer]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][ResourceController]",      scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][ResourceController]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][StandardDestroyer]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][StandardDestroyer]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][StandardFrigate]",         scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][StandardFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][P1IonArrayFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][P1IonArrayFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][P2MultiBeamFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][P2MultiBeamFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][P2FuelPod]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][P2FuelPod]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R1][CryoTray]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R1][CryoTray]), (udword) &(MothershipStatic) },

    { "specialDoorOffset[R2][AdvanceSupportFrigate]",   scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][AdvanceSupportFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][Carrier]",                 scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][Carrier]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][CloakGenerator]",          scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][CloakGenerator]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][DDDFrigate]",              scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][DDDFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][DFGFrigate]",              scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][DFGFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][GravWellGenerator]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][GravWellGenerator]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][HeavyCruiser]",            scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][HeavyCruiser]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][IonCannonFrigate]",        scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][IonCannonFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][MissileDestroyer]",        scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][MissileDestroyer]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][ResourceController]",      scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][ResourceController]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][StandardDestroyer]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][StandardDestroyer]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][StandardFrigate]",         scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][StandardFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][P1IonArrayFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][P1IonArrayFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][P2MultiBeamFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][P2MultiBeamFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][P2FuelPod]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][P2FuelPod]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[R2][CryoTray]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[R2][CryoTray]), (udword) &(MothershipStatic) },

    { "specialDoorOffset[P1][P1IonArrayFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[P1][P1IonArrayFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[P2][P2MultiBeamFrigate]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[P2][P2MultiBeamFrigate]), (udword) &(MothershipStatic) },
    { "specialDoorOffset[P2][P2FuelPod]",       scriptSetLWToHWMonkeyVectorCB, (udword) &(MothershipStatic.specialDoorOffset[P2][P2FuelPod]), (udword) &(MothershipStatic) },

    { "specialDoorOffset", scriptSetSpecialDoorOffsetCB,(udword) &(MothershipStatic), (udword) &(MothershipStatic)},




    { "specialDoorInterpolationPerSecond",              scriptSetReal32CB, (udword) &(MothershipStatic.specialDoorInterpolationPerSecond), (udword) &(MothershipStatic) },

    { NULL,NULL,0,0 }
};

real32 DOCKPOINT_BUSY_TOO_LONG = UNIVERSE_UPDATE_RATE * 60;

void MothershipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    MothershipStatics *mothershipstat = (statinfo->shiprace == R1) ? &MothershipStaticRace1 : &MothershipStaticRace2;
    sdword i,j;
    //init static info
    for(i=0;i<NUM_RACES;i++)
    {
        for(j=0;j<TOTAL_NUM_SHIPS;j++)
        {
            mothershipstat->specialDoorOffset[i][j].x = 0.0f;
            mothershipstat->specialDoorOffset[i][j].y = 0.0f;
            mothershipstat->specialDoorOffset[i][j].z = 0.0f;
            mothershipstat->specialDoorOrientationHeading[i][j]=-1;
            mothershipstat->specialDoorOrientationUp[i][j]=-1;
        }
    }

    scriptSetStruct(directory,filename,MothershipStaticTable,(ubyte *)mothershipstat);

    statinfo->custstatinfo = mothershipstat;
}
void MothershipInit(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;

    for(i=0;i<MAX_NUM_DROP;i++)
    {
       spec->dropstate[i] = 0;
       spec->droptarget[i] = NULL;
       spec->dockindex[i] = 0;
    }
    spec->doorCargo = NULL;

    spec->CAP_NumInBuildCue=0;
}
void MothershipAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    attackPassive(ship,(Ship *)target);  // Typecast to prevent warning, attackPassive will handle SpaceObjRotImpTarg
}

void MothershipAttackPassive(Ship *ship,Ship *target,bool rotate)
{
    attackPassive(ship,target);
}

//single player game...called when hs button is pushed for quick
//docking + progression to HS..this will ensure it partly works when
//launching big ships...
void mothershipCleanDoorForHSInstant(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;

    if(spec->doorCargo != NULL)
    {
        MothershipDettachObjectFromDoor(ship);
        spec->doorCargo = NULL;
    }

    if( ship->madDoorStatus != MAD_STATUS_DOOR_CLOSED)
    {
        //if door isn't closed
        if(ship->madBindings->bPaused)
        {
            //animation is paused...well,unpause it
            //so as to fix a bug in lukes code!
            madAnimationPause(ship,!ship->madBindings->bPaused);
        }
        madAnimationStop(ship);

        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorOpenIndexes[0];
        ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
        ship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
        ship->madAnimationFlags=0;
        ship->nextAnim = 0;
    }
}

void MothershipDoorUpKeep(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    vector positionWS,doorHeading,doorRight,doorUp;
    vector heading,up,right;
    matrix coordsysWS;

    if(spec->doorCargo != NULL)
    {
            //get info
        mothershipGetCargoPosition(ship,spec->doorCargo,&positionWS,&coordsysWS,&heading,&up,&right);

        matGetVectFromMatrixCol1(doorUp,coordsysWS);
        matGetVectFromMatrixCol2(doorRight,coordsysWS);
        matGetVectFromMatrixCol3(doorHeading,coordsysWS);

        //vecScalarMultiply(doorLeft,doorRight,-1.0f);
        matPutVectIntoMatrixCol1(up,spec->doorCargo->rotinfo.coordsys);
        //vecScalarMultiply(doorHeading,doorHeading,-1.0f);
        matPutVectIntoMatrixCol2(right,spec->doorCargo->rotinfo.coordsys);
        matPutVectIntoMatrixCol3(heading,spec->doorCargo->rotinfo.coordsys);

        if(spec->doorCargo->putOnDoor==TRUE || MoveReachedDestinationVariable((Ship *)spec->doorCargo,&positionWS,1.0f))
        {
            spec->doorCargo->putOnDoor=TRUE;
            spec->doorCargo->posinfo.position = positionWS;
        }
        else
        {
            vector adder;
            vecSub(adder,positionWS,spec->doorCargo->posinfo.position);
            vecScalarMultiply(adder,adder,0.3f);
            vecAddTo(spec->doorCargo->posinfo.position,adder);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : GetDirectionVectorFromMat
    Description : gets a direction vector from a matrix (0=up,1=right,2=forward,4=down,5=left,6=back)
    Inputs      : direction, ship
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void GetDirectionVectorFromMat(vector *result,udword direction,matrix *coordsys)
{
    switch (direction & 3)
    {
        case 0:
            matGetVectFromMatrixCol1(*result,*coordsys);
            break;

        case 1:
            matGetVectFromMatrixCol2(*result,*coordsys);
            break;

        case 2:
        case 3:
            matGetVectFromMatrixCol3(*result,*coordsys);
            break;
    }

    if (direction & 4)
    {
        vecNegate(*result);
    }
}
void mothershipGetCargoPosition(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *cargo, vector *position, matrix *coordsys, vector *heading, vector *up, vector *right)
{
    MothershipStatics *mothershipstatics = (MothershipStatics *)((ship->staticinfo))->custstatinfo;
    vector worldRelative;//doorUp,doorRight,doorHeading;
    sdword headingdir,updir;


    if(madLinkInGetDoorInfo(ship,coordsys,position))
    {
        position->x += TW_R1_MOTHERSHIP_DOOR_OFFSET_MODIFIER;

        headingdir = mothershipstatics->specialDoorOrientationHeading[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype];
        if(headingdir == -1)
            headingdir = 0;
        updir = mothershipstatics->specialDoorOrientationUp[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype];
        if(updir == -1)
            updir = 1;
        if(cargo->objtype == OBJ_ShipType)
        {
            matMultiplyMatByVec(&worldRelative,coordsys,&mothershipstatics->specialDoorOffset[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype]);
            GetDirectionVectorFromMat(heading,headingdir,coordsys);
            GetDirectionVectorFromMat(up,updir,coordsys);
            vecCrossProduct(*right,*heading,*up);
        }
        else
        {
            //probably never going to get here normally...derelicts will not be salvaged or docked.
            //use default values
            matMultiplyMatByVec(&worldRelative,coordsys,&mothershipstatics->specialDoorOffset[R1][HeavyCruiser]);
            GetDirectionVectorFromMat(heading,0,&ship->rotinfo.coordsys);
            GetDirectionVectorFromMat(up,1,&ship->rotinfo.coordsys);
            vecCrossProduct(*right,*heading,*up);

        }
        vecAddTo(*position,worldRelative);
    }
    else
    {
        //no postion attainable use default values
        *position = ship->posinfo.position;
        GetDirectionVectorFromMat(heading,0,&ship->rotinfo.coordsys);
        GetDirectionVectorFromMat(up,1,&ship->rotinfo.coordsys);
        vecCrossProduct(*right,*heading,*up);
    }

}
void MothershipRemoveShipReferences(Ship *ship,Ship *shiptoremove)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;
    if(shiptoremove == (Ship *) spec->doorCargo)
    {
        spec->doorCargo = NULL;
    }

    for(i=0;i<MAX_NUM_DROP;i++)
    {
        if(shiptoremove == (Ship *) spec->droptarget[i])
        {
            if(ship->dockInfo != NULL)
            {
                if(ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy)
                    ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy--;
            }
            if(ship->shiprace == R1)
            {
                if(ship->shiptype == Mothership)
                {
                    if(salCapNeedBig(ship,spec->droptarget[i]))
                    {
                        //we will have told the door too open...so lets check
                        if(ship->madDoorStatus == MAD_STATUS_DOOR_OPEN ||
                         ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
                        {
                          madLinkInCloseDoor(ship);   //close door if open!
                        }
                    }
                }
            }
            spec->droptarget[i] = NULL;
        }
    }
}

void MothershipAttachObjectToDoor(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *object)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;

#ifndef HW_Release
    dbgAssert(ship->shiptype == Mothership);
    dbgAssert(ship->shiprace == R1);
#endif
    spec->doorCargo = object;
    //in future, need to calculate offset from offset from offset!
}
void MothershipDettachObjectFromDoor(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;

#ifndef HW_Release
    dbgAssert(ship->shiptype == Mothership);
    dbgAssert(ship->shiprace == R1);
#endif
    spec->doorCargo = NULL;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"
void Mothership_PreFix(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        spec->droptarget[i] = SpaceObjRegistryGetID((SpaceObj *)spec->droptarget[i]);
    }
    spec->doorCargo = SpaceObjRegistryGetID((SpaceObj *)spec->doorCargo);

}

void Mothership_Fix(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        spec->droptarget[i] = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetObj((sdword)spec->droptarget[i]);
    }
    spec->doorCargo = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetObj((sdword)spec->doorCargo);
}
#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

udword ReadDockTimer(ubyte *docktimer)
{
    return (docktimer[0] | (docktimer[1]<<8) | (docktimer[2]<<16));
}

void WriteDockTimer(ubyte *docktimer,udword newvalue)
{
    docktimer[0] = newvalue & 255;
    docktimer[1] = (newvalue>>8) & 255;
    docktimer[2] = (newvalue>>16) & 255;
}

bool NoShipUsingThisDockPoint(Ship *ship,DockStaticPoint *dockpoint)
{
    // walk the command layer to make sure no one is using this dockpoint

    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    SelectCommand *selection;
    Ship *dship;
    sdword i;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        switch (command->ordertype.order)
        {
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
                selection = command->selection;
                for (i=0;i<selection->numShips;i++)
                {
                    dship = selection->ShipPtr[i];
                    if (dship->dockvars.dockship == ship)
                    {
                        if (dship->dockvars.dockstaticpoint == dockpoint)
                        {
                            // we're docking/launching with this ship,
                            return FALSE;
                        }
                    }
                }
                break;
        }

        curnode = curnode->next;
    }

    return TRUE;
}

void MothershipHouseKeep(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;
    bool droppingTargets = FALSE;

    for(i=0;i<MAX_NUM_DROP;i++)
    {
        if(spec->droptarget[i] != NULL)
        {
            droppingTargets = TRUE;
            if(DropTargetInShip(ship,&spec->dropstate[i],spec->droptarget[i],&spec->dockindex[i]))
            {
                spec->dropstate[i] = 0;
                spec->droptarget[i] = NULL;
            }
        }
    }

    for(i=0;i<spec->CAP_NumInBuildCue;)
    {
        spec->CAPTimeToBuildShip[i]-= universe.phystimeelapsed;
        if(spec->CAPTimeToBuildShip[i] < 0.0f)
        {
            //time for a captured ship to pop out
            Ship *newship;

            if (GetShipStaticInfoSafe(spec->CAPshiptype[i],spec->CAPshiprace[i]))
            {
                if ((multiPlayerGame) && (ship->playerowner->playerIndex==sigsPlayerIndex))
                {
                    shiplagtotals[spec->CAPshiptype[i]]++;
                }

                newship = clCreateShip(&universe.mainCommandLayer,
                        spec->CAPshiptype[i],
                        spec->CAPshiprace[i],
                        ship->playerowner->playerIndex,
                        ship);
                newship->colorScheme = spec->CAPcolorScheme[i];
            }

            spec->CAP_NumInBuildCue--;
            spec->CAPTimeToBuildShip[i] = spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue];
            spec->CAPshiptype[i] = spec->CAPshiptype[spec->CAP_NumInBuildCue];
            spec->CAPshiprace[i] = spec->CAPshiprace[spec->CAP_NumInBuildCue];
            spec->CAPcolorScheme[i] = spec->CAPcolorScheme[spec->CAP_NumInBuildCue];
            continue;
        }
        i++;
    }

    // check dock point and unbusy if it's busy for really long
    if (!droppingTargets)
    {
        DockPoint *dockpoint = NULL;

        if (ship->shiprace == R1)
        {
            dbgAssert((R1MOTHERSHIP_Big >= 0) && (R1MOTHERSHIP_Big < ship->dockInfo->numDockPoints));
            dockpoint = &ship->dockInfo->dockpoints[R1MOTHERSHIP_Big];
        }
        else if (ship->shiprace == R2)
        {
            dbgAssert((R2MOTHERSHIP_Big >= 0) && (R2MOTHERSHIP_Big < ship->dockInfo->numDockPoints));
            dockpoint = &ship->dockInfo->dockpoints[R2MOTHERSHIP_Big];
        }

        if (dockpoint)
        {
            udword docktimer = ReadDockTimer(spec->freedocktimer);

            if (dockpoint->thisDockBusy)
            {
                if (docktimer >= DOCKPOINT_BUSY_TOO_LONG)
                {
                    if (NoShipUsingThisDockPoint(ship,dockpoint->dockstaticpoint))
                    {
                        dockpoint->thisDockBusy = 0;
                    }
                    docktimer = 0;
                }
                else
                {
                    docktimer++;
                }
            }
            else
            {
                docktimer = 0;
            }

            WriteDockTimer(spec->freedocktimer,docktimer);
        }
    }
}

void MothershipDied(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    sdword i;
    for(i=0;i<MAX_NUM_DROP;i++)
    {
        if(spec->droptarget[i] != NULL)
        {
            if(spec->droptarget[i]->objtype == OBJ_ShipType)
            {
                //undisabled and make selectable
                if(!bitTest(((Ship *)spec->droptarget[i])->specialFlags2,SPECIAL_2_DisabledForever))
                {
                    bitClear(spec->droptarget[i]->flags,SOF_Disabled);
                    bitClear(spec->droptarget[i]->specialFlags,SPECIAL_SalvagedTargetGoingIntoDockWithShip);
                    bitSet(spec->droptarget[i]->flags,SOF_Selectable);
                }
            }

        }
    }
}
CustShipHeader MothershipHeader =
{
    Mothership,
    sizeof(MothershipSpec),
    MothershipStaticInit,
    NULL,
    MothershipInit,
    NULL,
    MothershipAttack,
    DefaultShipFire,
    MothershipAttackPassive,
    NULL,
    NULL,
    MothershipHouseKeep,
    MothershipRemoveShipReferences,
    MothershipDied,
    Mothership_PreFix,
    NULL,
    NULL,
    Mothership_Fix
};

