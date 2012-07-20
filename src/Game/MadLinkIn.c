/*=============================================================================
    Name    : madLinkIn.c
    Purpose : contains all code used to bind mesh animations to
              actual events.  Esentially a hard coded nightmare.

    Created 31/08/1998 by bryce pasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

//#include "MeshAnim.h"
#include <stdlib.h>

#include "Debug.h"
#include "CommandLayer.h"
#include "Universe.h"
#include "SpaceObj.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "Types.h"
#include "Mothership.h"
#include "Randy.h"
#include "SoundEvent.h"

#ifdef bpasechn
    #define DEBUG_MESH_ANIMATIONS
#endif



//Variables in ship structure to support mesh animations (amongst others)
//
//    udword madAnimationFlags;        //general purpose flags for mad animations
//    real32 madAnimationTime;        //general purpose variable for counting
//    udword cuedAnimation[MAD_MAX_CUEABLE];            //Animation index waiting in the wings to be played

/*-----------------------------------------------------------------------------
    Name        : madLinkInInit
    Description : Loads in mad script file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInInit()
{

}


//    udword madAnimationFlags;        //general purpose flags for mad animations
//    real32 madAnimationTime;        //general purpose variable for counting
//    udword cuedAnimation[MAD_MAX_CUEABLE];            //Animation index waiting in the wings to be played


/*-----------------------------------------------------------------------------
    Name        : madLinkInUpdateMeshAnimations
    Description : starts,stops and does whatever needs to be done
                  for mesh animations based on code given cues.
    Inputs      : Ship to have animation updated
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInUpdateMeshAnimations(Ship *ship)
{
    if(bitTest(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION))
    {
        //we need to switch to a new animation for some odd reason
        if(ship->madBindings->nCurrentAnim != MAD_NoAnimation)
        {
            if(ship->madBindings->bPaused)
            {
                //animation is paused...well,unpause it
                //so as to fix a bug in lukes code!
                madAnimationPause(ship,!ship->madBindings->bPaused);
            }
            madAnimationStop(ship);
        }
        madAnimationStart(ship, ship->cuedAnimationIndex);  //start 0th cued animation
		/* play special animation sound here */
		soundEvent(ship, ship->shaneyPooSoundEventAnimationTypeFlag);
        bitClear(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
        if(ship->shiptype == Mothership)
        {
            MothershipDoorUpKeep(ship);
        }
        return;
    }



    //time to start animation

    if(madAnimationUpdate(ship, universe.phystimeelapsed))
    {
#ifdef DEBUG_MESH_ANIMATIONS
        dbgMessagef("\nAnimation Finished.");
#endif
        //start and pause poo here for pooeyness...(wings just closed, gotta stay closed...etc..

        //cleanup ended animation
        switch(ship->cuedAnimationType)
        {
        case MAD_ANIMATION_GUN_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madGunStatus = MAD_STATUS_GUNS_OPEN;
            break;
        case MAD_ANIMATION_GUN_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madGunStatus = MAD_STATUS_GUNS_CLOSED;
            //setup closed gun hack...
            madAnimationStart(ship, ship->staticinfo->madStatic->gunOpenIndexes[0]);    //start 0th animation
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_WINGS_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madWingStatus = MAD_STATUS_WINGS_OPEN;
            break;
        case MAD_ANIMATION_WINGS_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madWingStatus = MAD_STATUS_WINGS_CLOSED;
            //setup closed gun hack...
            madAnimationStart(ship, ship->staticinfo->madStatic->PostDockIndexes[0]);   //start 0th animation
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_DOOR_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madDoorStatus = MAD_STATUS_DOOR_OPEN;
            madAnimationStart(ship, ship->staticinfo->madStatic->DoorCloseIndexes[0]);  //start 0th animaiton
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_DOOR_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
			// play the mothership door closed sound (big Ka-chunk)
			// this is in addition to the closing sound which is triggered at the start of closing
			soundEvent(ship, Ship_MoshipDoorClosed);
            break;
        case MAD_ANIMATION_SPECIAL_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madSpecialStatus = MAD_STATUS_SPECIAL_OPEN;
            break;
        case MAD_ANIMATION_SPECIAL_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madSpecialStatus = MAD_STATUS_SPECIAL_CLOSED;
            madAnimationStart(ship, ship->staticinfo->madStatic->specialOpenIndexes[0]);    //start 0th animaiton
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        }

        bitClear(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        //play animations that are waiting...
        if(ship->nextAnim != 0)
        {
            switch(ship->nextAnim)
            {
            case MAD_ANIMATION_GUN_OPENING:
                madOpenGunsShip(ship);
                break;
            case MAD_ANIMATION_GUN_CLOSING:
                madLinkInCloseGunsShip(ship);
                break;
            case MAD_ANIMATION_WINGS_OPENING:
                madLinkInPostDockingShip(ship);
                break;
            case MAD_ANIMATION_WINGS_CLOSING:
                madLinkInPreDockingShip(ship);
                break;
            case MAD_ANIMATION_DOOR_OPENING:
                madLinkInOpenDoor(ship);
                break;
            case MAD_ANIMATION_DOOR_CLOSING:
                madLinkInCloseDoor(ship);
                break;
            case MAD_ANIMATION_SPECIAL_OPENING:
                madLinkInOpenSpecialShip(ship);
                break;
            case MAD_ANIMATION_SPECIAL_CLOSING:
                madLinkInCloseSpecialShip(ship);
                break;
            default:
                dbgFatalf(DBG_Loc,"\nUnknown animation in nextAnim variable.");
                break;
            }
            ship->nextAnim = 0;
        }
    }
    if(ship->shiptype == Mothership)
    {
        MothershipDoorUpKeep(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInSetUpInitialBuiltMadState
    Description : For ships like the defender, probe, etc...
                  they need to have animations started then paused
                  upon creation.  This function does that.
    Inputs      : Ship to have animation set up
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInSetUpInitialBuiltMadState(Ship *ship)
{
#ifdef DEBUG_MESH_ANIMATIONS
    dbgAssert(ship->madBindings != NULL);
#endif

    if(ship->staticinfo->madStatic->needStartBuiltAnimation)
    {
        //if ship has need to be set to a specific starting animation
        madAnimationStart(ship, ship->staticinfo->madStatic->startBuiltAnimationIndex);
        madAnimationPause(ship, !ship->madBindings->bPaused);
        switch(ship->shiprace)
        {
        case R1:
            switch(ship->shiptype)
            {
            case HeavyDefender:
                ship->madGunStatus = MAD_STATUS_GUNS_CLOSED;
                break;
            case Probe:
                ship->madWingStatus = MAD_STATUS_WINGS_CLOSED;
                break;
            }
            break;
        case R2:
            switch(ship->shiptype)
            {
            case HeavyCorvette:
                ship->madWingStatus = MAD_STATUS_WINGS_CLOSED;
                break;
            }
            break;
        case P1:
        case P2:
        case P3:
        case Traders:
            break;
        default:
            dbgFatalf(DBG_Loc,"\nUnknown race type with a start up mesh animation (1).");
        }
    }

}
/*-----------------------------------------------------------------------------
    Name        : madLinkInSetUpInitialBuiltMadState
    Description : For ships like the defender, probe, etc...
                  they need to have animations started then paused
                  upon creation.  This function does that.
    Inputs      : Ship to have animation set up
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInSetUpInitialPlacedMadState(Ship *ship)
{
#ifdef DEBUG_MESH_ANIMATIONS
    dbgAssert(ship->madBindings != NULL);
#endif

    if(ship->staticinfo->madStatic->needStartPlacedAnimation)
    {
        //if ship has need to be set to a specific starting animation
        madAnimationStart(ship, ship->staticinfo->madStatic->startPlacedAnimationIndex);
        madAnimationPause(ship, !ship->madBindings->bPaused);
        switch(ship->shiprace)
        {
        case R1:
            switch(ship->shiptype)
            {
            case HeavyDefender:
                ship->madGunStatus = MAD_STATUS_GUNS_CLOSED;
                break;
            }
            break;
        case R2:
            switch(ship->shiptype)
            {
            }
            break;
        case P1:
        case P2:
        case P3:
        case Traders:
            break;
        default:
            dbgFatalf(DBG_Loc,"\nUnknown race type with a start up mesh animation (1).");
        }
    }

}

//General Comment:
//
//priority below:  1 - standard  (opens based on standard timings)
//                 2 - immediate (cues animation to play immediatly)

void madOpenGunsShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numGunOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madGunStatus == MAD_STATUS_GUNS_OPEN ||
           ship->madGunStatus == MAD_STATUS_GUNS_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madGunStatus == MAD_STATUS_GUNS_CLOSING)
        {
            ship->nextAnim = MAD_ANIMATION_GUN_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numGunOpenDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunOpenDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunOpenDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;

        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunOpenIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunOpenIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_GUN_OPENING;
        ship->madGunStatus = MAD_STATUS_GUNS_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenGuns
    Description : Sets up ships with gun animations to open their guns imediatly (regardless of certain other things).
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenGuns(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madOpenGunsShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseGuns
    Description : Sets the ship to imediatly begin a gun closing animation
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseGuns(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madLinkInCloseGunsShip(ship);
    }
}

void madLinkInCloseGunsShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numGunCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }
        if(ship->madGunStatus == MAD_STATUS_GUNS_CLOSED ||
           ship->madGunStatus == MAD_STATUS_GUNS_CLOSING)
        {
            //guns are currently open/opening...so lets ferget about it
            return;
        }
        else if(ship->madGunStatus == MAD_STATUS_GUNS_OPENING)
        {
            ship->nextAnim = MAD_ANIMATION_GUN_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numGunCloseDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunCloseDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunCloseDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunCloseIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunCloseIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
		}
        ship->cuedAnimationType = MAD_ANIMATION_GUN_CLOSING;
        ship->madGunStatus = MAD_STATUS_GUNS_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPostDocking
    Description : Cues up animations for ships that have finished
                  docking (opens fins, etc...)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPostDocking(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madLinkInPostDockingShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPostDockingShip
    Description : utility funciton for above function
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPostDockingShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDockIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madWingStatus == MAD_STATUS_WINGS_OPEN ||
           ship->madWingStatus == MAD_STATUS_WINGS_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madWingStatus == MAD_STATUS_WINGS_CLOSING)
        {
            ship->nextAnim = MAD_ANIMATION_WINGS_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madWingOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numPostDockDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numPostDockDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->PostDockDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;
		}
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numPostDockIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->PostDockIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_WINGS_OPENING;
        ship->madWingStatus = MAD_STATUS_WINGS_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPreDocking
    Description : Cues up animations for ships that are docking (closes fins, etc...)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPreDocking(CommandToDo *command,udword priority)
{

    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madLinkInPreDockingShip(ship);
    }
}
/*-----------------------------------------------------------------------------
    Name        : madLinkInPreDockingShip
    Description : utility function for above function
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPreDockingShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numPostDockIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madWingStatus == MAD_STATUS_WINGS_CLOSED ||
           ship->madWingStatus == MAD_STATUS_WINGS_CLOSING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madWingStatus == MAD_STATUS_WINGS_OPENING)
        {
            ship->nextAnim = MAD_ANIMATION_WINGS_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madWingOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numDockDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numDockDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->DockDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numDockIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->DockIndexes[animIndex];
            ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
        }
        ship->cuedAnimationType = MAD_ANIMATION_WINGS_CLOSING;
        ship->madWingStatus = MAD_STATUS_WINGS_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenDoor
    Description : opens door on R1 mothership (for now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenDoor(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDoorOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madDoorStatus == MAD_STATUS_DOOR_OPEN ||
           ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
        {
            //ALWAYS return;
            ship->nextAnim = MAD_ANIMATION_DOOR_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        animIndex = randombetween(0,(ship->staticinfo->madStatic->numDoorOpenIndexes-1));
        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorOpenIndexes[animIndex];

        ship->cuedAnimationType = MAD_ANIMATION_DOOR_OPENING;
        ship->madDoorStatus = MAD_STATUS_DOOR_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
		ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseDoor
    Description : opens door on R1 mothership (for now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseDoor(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDoorCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madDoorStatus == MAD_STATUS_DOOR_CLOSED ||
           ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
        {
            //ALWAYS return;
            ship->nextAnim = MAD_ANIMATION_DOOR_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        animIndex = randombetween(0,(ship->staticinfo->madStatic->numDoorCloseIndexes-1));
        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorCloseIndexes[animIndex];

        ship->cuedAnimationType = MAD_ANIMATION_DOOR_CLOSING;
        ship->madDoorStatus = MAD_STATUS_DOOR_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
		ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getDoorInfo
    Description : fills in coordsys and position with the doors information for this ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool madLinkInGetDoorInfo(Ship *ship, matrix *coordsys, vector *position)
{
    sdword madIndex, gunIndex;
    vector positiont;
    matrix coordsyst;

    madIndex = madBindingIndexFindByName(ship->staticinfo->madStatic->header, "Hangardoor");
    gunIndex = madGunBindingIndexFindByName(ship->staticinfo, "Hangardoor");
    dbgAssert(madIndex != -1);
    dbgAssert(gunIndex != -1);
    madAnimBindingMatrix(&coordsyst,&positiont,ship,gunIndex, madIndex);

    matMultiplyMatByMat(coordsys,&ship->rotinfo.coordsys,&coordsyst);
    //matMultiplyVecByMat(position,&positionSS,&ship->rotinfo.coordsys);
    matMultiplyMatByVec(position,&ship->rotinfo.coordsys,&positiont);
    vecAddTo(*position,ship->posinfo.position);
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenSpecialShip
    Description : runs the 'open' special animation of the ships.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenSpecialShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numSpecialOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPEN ||
           ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSING)
        {
            //toggle Reverse here!
            //Luke...Make it so.
            //for now, overwrite animation
            ship->nextAnim = MAD_ANIMATION_SPECIAL_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numSpecialOpenDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialOpenDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialOpenDamagedIndexes[animIndex];
            ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialOpenIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialOpenIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_SPECIAL_OPENING;
        ship->madSpecialStatus = MAD_STATUS_SPECIAL_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseSpecialShip
    Description : sets the 'close' special animation to play
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseSpecialShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numSpecialCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }
        if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSED ||
           ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSING)
        {
            //guns are currently open/opening...so lets ferget about it
            return;
        }
        else if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPENING)

        {
            //cue this anim for play right after the current one finishings
            ship->nextAnim = MAD_ANIMATION_SPECIAL_CLOSING;
            return;

        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health*ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numSpecialCloseDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialCloseDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialCloseDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialCloseIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialCloseIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
        }
        ship->cuedAnimationType = MAD_ANIMATION_SPECIAL_CLOSING;
        ship->madSpecialStatus = MAD_STATUS_SPECIAL_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

