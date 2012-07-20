/*=============================================================================
    Name    : SensorArray.c
    Purpose : Specifics for the ProximitySensor

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "SensorArray.h"
#include "SoundEvent.h"
#include "Universe.h"

typedef struct
{
    udword dummy;
} SensorArrayStatics;

SensorArrayStatics SensorArrayStaticRace1;
SensorArrayStatics SensorArrayStaticRace2;

void SensorArrayStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    SensorArrayStatics *SensorArraystat = (statinfo->shiprace == R1) ? &SensorArrayStaticRace1 : &SensorArrayStaticRace2;

    statinfo->custstatinfo = SensorArraystat;
}

void SensorArrayInit(Ship *ship)
{
//Sensor Array Just created...
    ship->playerowner->sensorLevel = 2;
}

bool noMoreLocalSensorArrays(Ship *ship)
{
    Node *shipnode = universe.ShipList.head;
    Ship *obj;

    while(shipnode != NULL)
    {
        obj = (Ship *) listGetStructOfNode(shipnode);
        if(obj == ship)                                     //ship is same as ship dieing
            goto nextnode;                                  //so don't count it
        if(obj->playerowner == ship->playerowner)       //ship is owned by same person as dieing ship
        {
            if(obj->shiptype == SensorArray)                //ship is a sensor array
            {
                return(FALSE);                  //Sensor level should remain the same
            }
        }
    nextnode:
            shipnode = shipnode->next;
    }

    return(TRUE);
}
void SensorArrayDied(Ship *ship)
{
    if(noMoreLocalSensorArrays(ship))
    {    //if this was the last sensor array belonging to the player
        ship->playerowner->sensorLevel = 0;         //???  is 0 the right one?
    }
}

CustShipHeader SensorArrayHeader =
{
    SensorArray,
    sizeof(SensorArraySpec),
    SensorArrayStaticInit,
    NULL,
    SensorArrayInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SensorArrayDied,
    NULL,
    NULL,
    NULL,
    NULL
};



