/*=============================================================================
    Name    : ProximitySensor.c
    Purpose : Specifics for the ProximitySensor

    Created 01/06/1998 by bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "ProximitySensor.h"
#include "SoundEvent.h"
#include "Universe.h"
#include "memory.h"
#include "UnivUpdate.h"
#include "LinkedList.h"
#include "FastMath.h"
#include "Vector.h"
#include "Ping.h"
#include "Blobs.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "Battle.h"
#include "Alliance.h"


/*=============================================================================
    Data:
=============================================================================*/
color ProximitySensorBlipColor = colWhite;

ProximitySensorStatics ProximitySensorStatic;

ProximitySensorStatics ProximitySensorStaticRace1;
ProximitySensorStatics ProximitySensorStaticRace2;

scriptStructEntry ProximitySensorStaticScriptTable[] =
{
    { "SearchRate",    scriptSetUdwordCB, (udword) &(ProximitySensorStatic.SearchRate), (udword) &(ProximitySensorStatic) },
    { "SearchRadius",    scriptSetReal32CB, (udword) &(ProximitySensorStatic.SearchRadius), (udword) &(ProximitySensorStatic) },
    { "SensorCircleRadius",    scriptSetSdwordCB, (udword) &(ProximitySensorStatic.SensorCircleRadius), (udword) &(ProximitySensorStatic) },
    { "SensorBlinkRate",    scriptSetReal32CB, (udword) &(ProximitySensorStatic.SensorBlinkRate), (udword) &(ProximitySensorStatic) },
    { "SearchRateAfterFound",    scriptSetUdwordCB, (udword) &(ProximitySensorStatic.SearchRateAfterFound), (udword) &(ProximitySensorStatic) },
    { "TriggerSpeed",    scriptSetReal32CB, (udword) &(ProximitySensorStatic.TriggerSpeed), (udword) &(ProximitySensorStatic) },

    { "blipColor",    scriptSetRGBACB, (udword) &(ProximitySensorStatic.blipColor), (udword) &(ProximitySensorStatic) },
    { "blipThickness",    scriptSetSdwordCB, (udword) &(ProximitySensorStatic.blipThickness), (udword) &(ProximitySensorStatic) },

    { NULL,NULL,0,0 }
};


void ProximitySensorStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    ProximitySensorStatics *ProximitySensorstat = (statinfo->shiprace == R1) ? &ProximitySensorStaticRace1 : &ProximitySensorStaticRace2;

    statinfo->custstatinfo = ProximitySensorstat;
    scriptSetStruct(directory,filename,ProximitySensorStaticScriptTable,(ubyte *)ProximitySensorstat);

    ProximitySensorstat->SearchRadiusSqr = ProximitySensorstat->SearchRadius*ProximitySensorstat->SearchRadius;
    ProximitySensorstat->TriggerSpeedSqr = ProximitySensorstat->TriggerSpeed*ProximitySensorstat->TriggerSpeed;
}

void ProximitySensorInit(Ship *ship)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;
    spec->sensorState = SENSOR_BEGIN;
    spec->blipRadius = 0.0f;
    spec->TAGGED = FALSE;
}

void proxInFocusSelection(SelectCommand *focus)
{
    sdword count;
    for(count = 0; count < focus->numShips; count++)
    {
        if(focus->ShipPtr[count]->shiptype == ProximitySensor)
        {   //proximity sensor is in the selection
            ((ProximitySensorSpec *)((focus->ShipPtr[count])->ShipSpecifics))->TAGGED = TRUE;
        }
    }
}

bool proxShouldDrawOverlay(Ship *ship)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;
    if(ship->playerowner == universe.curPlayerPtr)      //draw because it is local players...
        if(spec->sensorState == SENSOR_SENSED || spec->sensorState == SENSOR_SENSED2)
            return(TRUE);

    return(FALSE);
}
udword proxBlipColor(Ship *ship)
{
    ProximitySensorStatics *proximitysensorstatics;
    proximitysensorstatics = (ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    ProximitySensorBlipColor = proximitysensorstatics->blipColor;
    return(ProximitySensorBlipColor);
}
sdword proxBlipThickness(Ship *ship)
{
    ProximitySensorStatics *proximitysensorstatics;
    proximitysensorstatics = (ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
    return(proximitysensorstatics->blipThickness);
}

//function tags ship so that ai knows player can see the ship.  If ship is cloaked,

/*-----------------------------------------------------------------------------
    Name        : proximityPlayerSeesShip
    Description : Tags 'enemy' ship such that the owner of 'ship'
                  can see the enemy ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void proximityPlayerSeesShip(Ship *ship,Ship *enemy)
{
    sdword mask = 0x01;
    if(ship->playerowner == NULL)
        return;
    mask = mask << ship->playerowner->playerIndex;
    bitSet(enemy->visibleToWho,mask);

    if(ship->playerowner->playerIndex == universe.curPlayerIndex)
    {
        //local machines ship
        if(enemy->flags & SOF_Cloaked)
        {
            //enemy is cloaked
            ////////////////////
            //speech event indicating such
            //event num: STAT_F_CloakedShipsDetected
            //battle chatter!
            if (battleCanChatterAtThisTime(BCE_STAT_F_CloakedShipsDetected, ship))
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_F_CloakedShipsDetected, SOUND_EVENT_DEFAULT, &enemy->posinfo.position);
            }

            ///////////////
        }
    }

}

/*-----------------------------------------------------------------------------
    Name        : proximityPlayerCantSeeShip
    Description : Untags Tags 'enemy' ship such that the owner of 'ship'
                  can't see the enemy ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void proximityPlayerCantSeeShip(Ship *ship,Ship *enemy)
{
    sdword mask = 0x01;
    if(ship->playerowner == NULL)
        return;
    mask = mask << ship->playerowner->playerIndex;
    bitClear(enemy->visibleToWho,mask);
}


/*-----------------------------------------------------------------------------
    Name        : proximityCanPlayerSeeShip
    Description : determins if player can see shipInQuestion
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool proximityCanPlayerSeeShip(Player *player,Ship *shipInQuestion)
{
    sdword mask = 0x01;
    if(player == NULL)
        return FALSE;

    mask = mask << player->playerIndex;
    if(bitTest(shipInQuestion->visibleToWho,mask))
    {
        //player can see ship
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : DetectShips
    Description : searchs ships blob for enemy and cloaked ships nearby
    Inputs      :
    Outputs     :
    Return      : TRUE if enemy ships nearby
----------------------------------------------------------------------------*/

bool DetectShips(Ship *ship)
{
    Ship *enemy;
    vector dist;
    bool returnval;
    ProximitySensorStatics *proximitysensorstatics;
    SelectCommand *selection;
    sdword numShips;
    sdword i;

    proximitysensorstatics = (ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    //Optimize detection...
    //Use collision avoidence to 'tell' proximity sensors of ships near it???

    if(ship->collMyBlob == NULL)
    {
        return FALSE;   //not outside...so don't detect anything
    }

    selection = ship->collMyBlob->blobShips;
    numShips = selection->numShips;

    i=0;

    returnval = FALSE;
    while (i < numShips)
    {
        enemy = selection->ShipPtr[i];

        if(!allianceIsShipAlly(enemy, ship->playerowner))
        {    //Enemy ship...later add check here for ALLIANCE
            vecSub(dist,enemy->posinfo.position,ship->posinfo.position);
            if(vecMagnitudeSquared(dist) < proximitysensorstatics->SearchRadiusSqr)
            {   //enemy is within detection radius
                proximityPlayerSeesShip(ship,enemy);
                returnval = TRUE;
            }
        }
        i++;
    }
    return returnval;
}

sdword proxGetBlipRadius(Ship *ship)
{
    return((sdword) ((ProximitySensorSpec *)ship->ShipSpecifics)->blipRadius);
}

/*-----------------------------------------------------------------------------
    Name        : ProximitySensorPingTimeout
    Description : Ping callback to evaluate if the ping has timed out.
    Inputs      : hellaPing - the ping we're evaluating
                  userID - cast to ship * for pointer to proximity sensor.
                  userData - ignore
                  bRemoveReferneces - if TRUE, remove references to pingDyingObject
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
bool ProximitySensorPingTimeout(struct ping *hellaPing, udword userID, char *userData, bool bRemoveReferences)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)((Ship *)userID)->ShipSpecifics;

    if (bRemoveReferences)
    {
        if ((SpaceObj *)userID == pingDyingObject)
        {
            return(TRUE);
        }
        return(FALSE);
    }
    if(!(spec->sensorState == SENSOR_SENSED || spec->sensorState == SENSOR_SENSED2))
    {
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : ProximitySensorHouseKeep
    Description : Housekeeping function for proximity sensor
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ProximitySensorHouseKeep(Ship *ship)
{
    ping *newPing;

    ProximitySensorStatics *proximitysensorstatics;
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;
    proximitysensorstatics = (ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    //HERE WE SHOULD PROBABLY EXTEND THE STUPID WINGS....!  IF INDEED THEY ARE TO BE EXTENDED


    //right now we are returning if this prox isn't the current players so we don't waste time in its loops
    //maybe later only return if it is an enemy current player because proximity sensor info would be allied shared?  Nahh

    //no longer can we make this optimization since it needs to be deterministic

    //if(ship->playerowner != universe.curPlayerPtr)
    //    return;                                         //if this prox sensor isn't the local users
                                                        // don't do anything
    //deal with the mesh animations cleverlike!
    if(ship->posinfo.isMoving & ISMOVING_MOVING)
    {
        //if prox is going above a reasonable speed..and not just being moved a bit!
        if(vecMagnitudeSquared(ship->posinfo.velocity) > 40000.0f)
        {
            if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPEN)
            {
                //close the fins...if docking
                madLinkInCloseSpecialShip(ship);
            }
        }
        else
        {
            if(ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPEN &&
               ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPENING &&
               ship->madSpecialStatus != MAD_STATUS_SPECIAL_CLOSING)
            {
               madLinkInOpenSpecialShip(ship);
            }
        }
    }
    else
    {
        if(ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPEN &&
           ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPENING)
        {
           madLinkInOpenSpecialShip(ship);
        }
    }


    switch(spec->sensorState)
    {
    case SENSOR_BEGIN:  //Sensor has just been built
        spec->sensorState = SENSOR_SENSE;
        break;
    case SENSOR_SENSE:
        if((universe.univUpdateCounter & TW_ProximitySensorSearchRate) == TW_ProximitySensorSearchFrame)
        {   //its time to search for ships in vicinity
           if(DetectShips(ship))
           {    //enemy detected in vicinity....notifiy user..later through in # of Enemies?
                //CueUserAudio(ship);                     //turn on the audio        -- one time call
//                soundEvent(ship, Sensor);
                spec->sensorState = SENSOR_SENSED;
                spec->blipRadius = 0.0f;
                spec->TAGGED = FALSE;

                if (ship->playerowner->playerIndex == universe.curPlayerIndex)
                {
                    //only do this stuff for the local player...
                    //speechEventFleetSpec(ship, STAT_F_ProxSensor_Detection, 0, ship->playerowner->playerIndex);
                    if(battleCanChatterAtThisTime(BCE_STAT_F_ProxSensor_Detection, ship))
                    {
                        battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_F_ProxSensor_Detection, SOUND_EVENT_DEFAULT, &ship->posinfo.position);
                    }
                    newPing = pingCreate(NULL, (SpaceObj *)ship, ProximitySensorPingTimeout, NULL, 0, (udword)ship);
                    newPing->c = proxBlipColor(ship);
                    newPing->size = proximitysensorstatics->SearchRadius;
                    newPing->minScreenSize = primScreenToGLScaleX(2);
                    newPing->pingDuration = 1.0f / proximitysensorstatics->SensorBlinkRate;
                    newPing->interPingPause = 0.25;
                    newPing->TOMask = PTOM_Proximity;
                }
            }
        }
        break;
    case SENSOR_SENSED:
        if(!spec->TAGGED)
        {   //We have been tagged..so don't annoy user anyfurther
            //CueUserAudio(ship);     //turn off user audio here:
            spec->sensorState = SENSOR_SENSED2;
        }
    case SENSOR_SENSED2:
        //need to determine whether to beep, or to blink here...
        //
        spec->blipRadius+= proximitysensorstatics->SensorBlinkRate;
        if(spec->blipRadius >= (real32) proximitysensorstatics->SensorCircleRadius)
            spec->blipRadius = 0.0f;

        if((universe.univUpdateCounter & TW_ProximitySensorSearchRate) == TW_ProximitySensorSearchFrame)
        {   //its time to search and see if Enemies STILL in vicinity...or if relief has come?
           if(!DetectShips(ship))
           {   //Enemies No longer being Detected
               //de TAG us and go to search mode....
                spec->sensorState = SENSOR_SENSE;
                //turn off sound here too ...
                spec->TAGGED = FALSE;
           }
        }
        break;
    default:
        break;
    }
}

CustShipHeader ProximitySensorHeader =
{
    ProximitySensor,
    sizeof(ProximitySensorSpec),
    ProximitySensorStaticInit,
    NULL,
    ProximitySensorInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ProximitySensorHouseKeep,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};



