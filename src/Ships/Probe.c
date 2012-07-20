/*=============================================================================
    Name    : Probe.c
    Purpose : Specifics for the Probe

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <math.h>
#include "Types.h"
#include "SpaceObj.h"
#include "AITrack.h"
#include "Probe.h"
#include "StatScript.h"
#include "SoundEvent.h"
#include "Debug.h"
#include "Universe.h"
#include "Vector.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"

#define PROBE_WAIT_FOR_SAILS_TO_OPEN    3.0f

ProbeStatics ProbeStatic;

ProbeStatics ProbeStaticRace1;
ProbeStatics ProbeStaticRace2;

scriptStructEntry ProbeStaticScriptTable[] =
{
    { "ProbeDispatchMaxVelocity",     scriptSetReal32CB, (udword) &(ProbeStatic.ProbeDispatchMaxVelocity), (udword) &(ProbeStatic) },
    { NULL,NULL,0,0 }
};

real32 ProbeGetMaxVelocity(Ship *ship)
{
    ProbeSpec *spec = (ProbeSpec *) ship->ShipSpecifics;

    dbgAssert(ship->shiptype == Probe);

    if (spec->HaveMoved)
    {
        ProbeStatics *probestatics = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
        return probestatics->ProbeDispatchMaxVelocity;
    }

    return ship->staticinfo->staticheader.maxvelocity;
}

void ProbeStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    ProbeStatics *probestat = (statinfo->shiprace == R1) ? &ProbeStaticRace1 : &ProbeStaticRace2;

    statinfo->custstatinfo = probestat;

    scriptSetStruct(directory,filename,ProbeStaticScriptTable,(ubyte *)probestat);
}
void ProbeInit(Ship *ship)
{
    ProbeSpec *spec = (ProbeSpec *) ship->ShipSpecifics;
    spec->HaveMoved = FALSE;            //set Having Moved Flag to False
}

void ProbeHouseKeep(Ship *ship)
{
    ProbeSpec *spec = (ProbeSpec *) ship->ShipSpecifics;
    //ProbeStatics *probestatics = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    vector right,heading,testvec;


    if(spec->HaveMoved)
    {   //probe has moved...
        //fix later
        //could precalc the movetime to be deathtime + totaltime, then we aren't
        //doing a subtraction
        CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);

        //if we are docking...we should probably close the fins...
        if(ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPEN &&
           ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPENING)
        {
            //open wings after being given a move command!
            if(command != NULL && command->ordertype.order == COMMAND_DOCK)
            {
                // don't open sails as we're docking at the end of a mission having not been used during it
            }
            else
            {
                if(universe.totaltimeelapsed - spec->moveTime > PROBE_WAIT_FOR_SAILS_TO_OPEN)
                {
                    //wait a couple secs before opening sails
                    madLinkInOpenSpecialShip(ship);
                }
            }
        }

        if(command != NULL && command->ordertype.order == COMMAND_DOCK)
        {
            //probe is docking
            if(ship->dockingship != NULL)
            {
                vecSub(testvec,ship->dockingship->posinfo.position,ship->posinfo.position);
                if(vecMagnitudeSquared(testvec) < 25000000)
                {
                    if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPEN)
                    {
                        //close the fins...if docking
                        madLinkInCloseSpecialShip(ship);
                    }
                }
            }
        }

        /*
        if((universe.totaltimeelapsed - spec->movetime) > probestatics->ExpireyLife)
        {   //probe has exceeded lifetime.
            ship->deathtime = 1.0f;     //self destruct
            speechEventFleet(STAT_F_Probe_Expiring, 0, ship->playerowner->playerIndex);
        }
        */
        bitClear(ship->dontrotateever,1);
        if(ship->posinfo.isMoving & ISMOVING_MOVING)
        {
            //if above a certain velocity lets rotate like its 1492...
            if(vecMagnitudeSquared(ship->posinfo.velocity) > 10000.0f)      //later optimize by normalizing ourselves below...
            {
                //matGetVectFromMatrixCol1(up,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(right,ship->rotinfo.coordsys);
                vecCopyAndNormalize(&ship->posinfo.velocity,&heading);
                aitrackHeadingAndUp(ship,&heading,&right,0.99f);
                bitSet(ship->dontrotateever,1);
            }
        }
    }
}


CustShipHeader ProbeHeader =
{
    Probe,
    sizeof(ProbeSpec),
    ProbeStaticInit,
    NULL,
    ProbeInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ProbeHouseKeep,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


