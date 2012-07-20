/*=============================================================================
    Name    : shipselect.c
    Purpose : Routines for manipulating SelectCommand structures

    Created 10/19/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "SpaceObj.h"
#include "ShipSelect.h"
#include "Universe.h"
#include "Alliance.h"
#include "Attributes.h"
#include "ProximitySensor.h"
#include "SinglePlayer.h"
#include "SoundEvent.h"
#include "SpeechEvent.h"
#include "Battle.h"

/*-----------------------------------------------------------------------------
    Name        : AddSpaceObjToSelectionBeforeIndex
    Description : Adds a SpaceObj to selection before index
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddSpaceObjToSelectionBeforeIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index)
{
    sdword i;

    dbgAssert(index >= 0);
    dbgAssert(index < selection->numSpaceObjs);

    for (i=selection->numSpaceObjs;i>index;i--)
    {
        selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i-1];
    }
    selection->SpaceObjPtr[index] = obj;
    selection->numSpaceObjs++;
}

/*-----------------------------------------------------------------------------
    Name        : AddSpaceObjToSelectionAfterIndex
    Description : Adds a SpaceObj to selection after index
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddSpaceObjToSelectionAfterIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index)
{
    sdword i;

    dbgAssert(index >= 0);
    dbgAssert(index < selection->numSpaceObjs);

    index++;

    for (i=selection->numSpaceObjs;i>index;i--)
    {
        selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i-1];
    }
    selection->SpaceObjPtr[index] = obj;
    selection->numSpaceObjs++;
}

/*-----------------------------------------------------------------------------
    Name        : RemoveSpaceObjFromSelectionPreserveOrder
    Description : removes a space obj from the selection, while preserving order
    Inputs      : selection, obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool RemoveSpaceObjFromSelectionPreserveOrder(SpaceObjSelection *selection,SpaceObj *obj)
{
    sdword i;
    sdword numSpaceObjs = selection->numSpaceObjs;

    for (i=0;i<numSpaceObjs;i++)
    {
        if (obj == selection->SpaceObjPtr[i])
        {
            for ( ;i<numSpaceObjs-1;i++)
            {                                           //for the rest of the list
                selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i+1];
            }
            selection->numSpaceObjs--;

            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : RemoveTargetFromSelection
    Description : A useful utility to remove targets from selection
                  Does not delete entry - just removes reference to it
    Inputs      : removeTargetPtr
    Outputs     :
    Return      : Returns TRUE if any target was removed
----------------------------------------------------------------------------*/
bool clRemoveTargetFromSelection(SelectAnyCommand *selection,TargetPtr removeTargetPtr)
{
    sdword i;
    bool removedAny = FALSE;

    for (i=0;i<selection->numTargets; )
    {
        if (removeTargetPtr == selection->TargetPtr[i])
        {
            removedAny = TRUE;
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
                      // warning: leave this as continue because sometimes removeTargetPtr may be set to NULL to remove all null's
        }
        i++;
    }

    return removedAny;
}

/*-----------------------------------------------------------------------------
    Name        : ShipInSelection
    Description : returns TRUE if ship is in selection
    Inputs      : selection, ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool ShipInSelection(SelectCommand *selection,Ship *ship)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : ShiptypeInSelection
    Description : Returns TRUE if the selection contains a ship of shiptype "Type"
    Inputs      : selection - the selection to check, type - the type to check for
    Outputs     :
    Return      : TRUE if a ship of the specified type is found
----------------------------------------------------------------------------*/
udword ShiptypeInSelection(SelectCommand *selection, ShipType type)
{
    udword i, count;

    for (i=0,count=0;i<selection->numShips;i++)
    {
        if (selection->ShipPtr[i]->shiptype == type)
        {
            count++;
        }
    }
    return count;
}


/*-----------------------------------------------------------------------------
    Name        : FindFirstInstanceOfShipType
    Description : Returns the first ship in the selection of the specified type
    Inputs      : selection - the selection to check, type - the type to check for
    Outputs     :
    Return      : A pointer to the first instance of the specified type
----------------------------------------------------------------------------*/
ShipPtr FindFirstInstanceOfShipType(SelectCommand *selection, ShipType type)
{
    udword i;

    for (i = 0; i<selection->numShips; i++)
    {
        if (selection->ShipPtr[i]->shiptype == type)
        {
            return selection->ShipPtr[i];
        }
    }
    return NULL;
}




/*-----------------------------------------------------------------------------
    Name        : MakeShipsNotIncludeTheseShips
    Description : Makes sure that ships in selection do not include theseships
    Inputs      : selection, theseships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsNotIncludeTheseShips(SelectCommand *selection,SelectCommand *theseships)
{
    sdword numTheseShips = theseships->numShips;
    sdword i;

    for (i=0;i<numTheseShips;i++)
    {
        clRemoveShipFromSelection(selection,theseships->ShipPtr[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : AnyOfTheseShipsAreInSelection
    Description : returns true if any ships in theseShips are in selection
    Inputs      : theseShips,selection
    Outputs     :
    Return      : returns true if any ships in theseShips are in selection
----------------------------------------------------------------------------*/
bool AnyOfTheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection)
{
    sdword i,j;
    ShipPtr thisShip;

    for (i=0;i<theseShips->numShips;i++)
    {
        thisShip = theseShips->ShipPtr[i];

        for (j=0;j<selection->numShips;j++)
        {
            if (thisShip == selection->ShipPtr[j])
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : TheseShipsAreInSelection
    Description : returns true if all of these ships are in selection
    Inputs      : theseShips, selection
    Outputs     :
    Return      : returns true if all of these ships are in selection
----------------------------------------------------------------------------*/
bool TheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection)
{
    sdword i;
    sdword numShips = theseShips->numShips;

    if (numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        if (!ShipInSelection(selection,theseShips->ShipPtr[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : SelectionsAreEquivalent
    Description : tells whether selection1 and selection2 are focusing on the same
                  ships (ignores order of objects in lists).
    Inputs      : selection1, selection2
    Outputs     :
    Return      : true if selection1 and selection2 are focusing on the same ships
----------------------------------------------------------------------------*/
bool SelectionsAreEquivalent(SelectCommand *selection1,SelectCommand *selection2)
{
    sdword i,j;
    sdword numShips = selection1->numShips;
    ShipPtr matchship;
    bool matched;

    if (numShips != selection2->numShips)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        matched = FALSE;
        matchship = selection1->ShipPtr[i];

        for (j=0;j<numShips;j++)
        {
            if (selection2->ShipPtr[j] == matchship)
            {
                matched = TRUE;
                break;
            }
        }

        if (!matched)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : SelectionsAreTotallyEquivalent
    Description : tells whether selection1 and selection2 are the same selections
                  (including order).
    Inputs      : selection1, selection2
    Outputs     :
    Return      : true if selection1 and selection2 are the same
----------------------------------------------------------------------------*/
bool SelectionsAreTotallyEquivalent(SelectCommand *selection1,SelectCommand *selection2)
{
    sdword numShips = selection1->numShips;
    sdword i;

    if (numShips != selection2->numShips)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        if (selection1->ShipPtr[i] != selection2->ShipPtr[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : isShipAttackCapable
    Description : returns TRUE if ship is attack capable
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is attack capable
----------------------------------------------------------------------------*/
bool isShipAttackCapable(Ship *ship)
{
    if (ship->staticinfo->custshipheader.CustShipAttack == NULL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : AreAllShipsAttackCapable
    Description : returns TRUE if all ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if all ships are attack capable
----------------------------------------------------------------------------*/
bool AreAllShipsAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipAttack == NULL)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : AreAnyShipsAttackCapable
    Description : returns TRUE if any ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if any ships are attack capable
----------------------------------------------------------------------------*/
bool AreAnyShipsAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipAttack != NULL)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : isShipPassiveAttackCapable
    Description : returns TRUE if ship is attack capable
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is attack capable
----------------------------------------------------------------------------*/
bool isShipPassiveAttackCapable(Ship *ship)
{
    if (ship->staticinfo->custshipheader.CustShipAttackPassive == NULL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : AreAllShipsPassiveAttackCapable
    Description : returns TRUE if all ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if all ships are attack capable
----------------------------------------------------------------------------*/
bool AreAllShipsPassiveAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipAttackPassive == NULL)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : AreAnyShipsPassiveAttackCapable
    Description : returns TRUE if any ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if any ships are attack capable
----------------------------------------------------------------------------*/
bool AreAnyShipsPassiveAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipAttackPassive != NULL)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsAttackCapable
    Description : Makes sure that the ships in this selection are capable of
                  attack.  Removes any ships from this selection that are not
                  capable of attacking.
    Inputs      : source - selection to cull
    Outputs     : dest - culled selection
    Return      : number of ships in culled selection
    Note        : source and dest can safely be the same selection.
----------------------------------------------------------------------------*/
sdword MakeShipsAttackCapable(SelectCommand *dest, SelectCommand *source)
{
/*
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipAttack == NULL)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
*/
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        if (source->ShipPtr[index]->staticinfo->custshipheader.CustShipAttack != NULL)
        {
            dest->ShipPtr[numShips] = source->ShipPtr[index];
            numShips++;
        }
    }
    dest->numShips = numShips;
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : ShipCanHarvest
    Description : returns TRUE if ship can harvest
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship can harvest
----------------------------------------------------------------------------*/
bool ShipCanHarvest(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shiptype == ResourceCollector)
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsHarvestCapable
    Description : Makes a selection of just ships that are harvesters.
    Inputs      : source - selection to search through
    Outputs     : dest - where to store the culled selection
    Return      : number of ships in culled selection
----------------------------------------------------------------------------*/
sdword MakeShipsHarvestCapable(SelectCommand *dest, SelectCommand *source)
{
/*
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (!ShipCanHarvest(shipstatic))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
*/
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        if (ShipCanHarvest(source->ShipPtr[index]->staticinfo))
        {
            dest->ShipPtr[numShips] = source->ShipPtr[index];
            numShips++;
        }
    }
    dest->numShips = numShips;
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : ShipCanGuard
    Description : returns TRUE if ship is capable of guarding
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship is capable of guarding
----------------------------------------------------------------------------*/
bool ShipCanGuard(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shiptype == Mothership)
    {
        return FALSE;
    }

    if (shipstatic->shiptype == Probe)
    {
        return FALSE;
    }

    /*
    if (shipstatic->shiptype == ProximitySensor)
    {
        return FALSE;
    }
    */
#if 0
    if (shipstatic->shiptype == ResearchShip)
    {
        return FALSE;
    }

    if (shipstatic->shiptype == SensorArray)
    {
        return FALSE;
    }
#endif
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsGuardCapable
    Description : Makes sure that the ships in this selection are capable of
                  guarding.  Removes any ships from this selection that are not
                  capable of guarding.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsGuardCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (!ShipCanGuard(shipstatic))
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
    Name        : makeShipsDockCapable
    Description : removes ships that shouldn't be docking from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsDockCapable(SelectCommand *selection)
{
    sdword i;
    Ship *ship;

    if (playPackets)
    {
        return;
    }

    for (i=0;i<selection->numShips;)
    {
        //remove probe from selection..shouldn't dock!
        ship = selection->ShipPtr[i];
        if ((ship->shiptype == Probe) || (ship->shiptype == TargetDrone) || (ship->shiptype == SensorArray))
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
    Name        : makeShipsRetireable
    Description : removes ships that shouldn't be retiring from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsRetireable(SelectCommand *selection)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;)
    {
        //remove probe from selection..shouldn't dock!
        ship = selection->ShipPtr[i];
        if ((ship->shiptype == Probe) || (ship->shiptype == TargetDrone))
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
    Name        : makeShipsNotIncludeSinglePlayerMotherships
    Description : If in single player, removes motherships from the selection
    Inputs      : selection - selection to cull
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsNotIncludeSinglePlayerMotherships(SelectCommand *selection)
{
    MakeShipsMobile(selection);
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsControllable
    Description : Removes ships from the selection such that all ships left in the selction
                  aren't docking and can be freely ordered
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsControllable(SelectCommand *selection,sdword newCommand)
{
    sdword i;
    CommandToDo *command;

    if (playPackets)
    {
        // playing back game, user should be able to do NOTHING
        //selection->numShips = 0;
        return;
    }

    if(newCommand == COMMAND_HALT)
    {
        return;
    }
    for (i=0;i<selection->numShips;)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,selection->ShipPtr[i]);
        if(command != NULL)
        {
            if(command->ordertype.order == COMMAND_DOCK ||
                command->ordertype.order == COMMAND_LAUNCHSHIP)
            {
                //if docking ship is NULL, we can assume some stuff...
                if (selection->ShipPtr[i]->dockingship != NULL)
                {
                    // to remove ShipPtr from list, decrement numShips and put last member
                    // of array into slot being deleted.

                    ///////////////////////
                    //Cannot Comply speech event!
                    //Use Battle Chatter to filter
                    //if SHOULDn't be called for move's that are cancelled..
                    //then we'll add a flag that gets passed it turning this on or off
                    //event num: COMM_CannotComply
                    //BCE_CannotComply
                    ///////////////////////
                    if (battleCanChatterAtThisTime(BCE_CannotComply, selection->ShipPtr[i]->dockingship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selection->ShipPtr[i]->dockingship, SOUND_EVENT_DEFAULT);
                    }

                    if(selection->ShipPtr[i]->shiptype != ResearchShip)
                    {
                        selection->numShips--;
                        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
                        continue; // we must check same index again because we put last array entry here
                    }
                }
            }
        }
        i++;
    }
}

void makeSelectionHyperspaceCapable(SelectCommand *selection)
{
    sdword i;

    //remove all NON-hyperspaceable ships!

    for(i=0;i<selection->numShips;)
    {
        if(!TW_HyperSpaceCostStruct[selection->ShipPtr[i]->shiptype].canMpHyperspace)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : MakeShipsSpecialActivateCapable
    Description : Makes sure that the ships in this selection are capable of
                  the special activate.  Removes any ships from this selection that are not
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSpecialActivateCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipSpecialActivate == NULL)
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
    Name        : MakeShipsSpecialTargetCapable
    Description : Makes sure that the ships in this selection are capable of
                  special targeting.  Removes any ships from this selection that are not
    Inputs      : selection - selection to neuter
                  bFriendlies - if TRUE, only select ships that can z-bandbox
                    friendlies, otherwise only ships that can z-bandbox enemies
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSpecialTargetCapable(SelectCommand *selection, bool bFriendlies)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
//        if (shipstatic->custshipheader.CustShipSpecialTarget == NULL)
//        {
        if (shipstatic->custshipheader.CustShipSpecialTarget != NULL)
        {
            if (bFriendlies)
            {                                               //if we're targeting friendlies and ship can do so
                if (shipstatic->canSpecialBandBoxFriendlies)
                {
                    i++;                                    //leave this ship as is
                    continue;
                }
            }
            else
            {                                               //if we're targeting enemies and ship can do so
                if (!shipstatic->canSpecialBandBoxFriendlies)
                {
                    i++;                                    //leave this ship as is
                    continue;
                }
            }
        }
        // to remove ShipPtr from list, decrement numShips and put last member
        // of array into slot being deleted.
        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

        continue; // we must check same index again because we put last array entry here
//        }
//        i++;
    }
}

bool AreShipsMobile(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->staticheader.immobile)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsMobile
    Description : Makes sure that the ships in this selection are mobile
                  Removes any ships from this selection that are not mobile.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsMobile(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->staticheader.immobile)
        {
            if ((shipstatic->shiptype == Mothership) && (selection->ShipPtr[i]->playerowner == universe.curPlayerPtr))
            {
                speechEventFleet(COMM_F_MoShip_CantMove, 0, universe.curPlayerIndex);
            }

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
    Name        : makeShipsFormationCapable
    Description : removes ships that can't go into formation
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsFormationCapable(SelectCommand *selection)
{

    sdword i;
    Ship *ship;
    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ship->shiptype == Probe)
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
    Name        : MakeShipsNonCapital
    Description : Makes sure that the ships in this selection are not capital
                  ships.  Removes any capital ships from this selection.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsNonCapital(SelectCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (isCapitalShip(selection->ShipPtr[i]))
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
    Name        : MakeShipsOnlyCapital
    Description : Makes sure that the ships in this selection are only capital ships
    Inputs      : selection - the selection to lookat
    Outputs     : Removes non capital ships from the selection
    Return      : void
----------------------------------------------------------------------------*/
void MakeShipsOnlyCapital(SelectCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (isCapitalShip(selection->ShipPtr[i]))
        {
            i++;
        }
        else
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : MakeShipsOnlyFollowConstraints
    Description : General routine that makes all ships in selection follow constraints set
                  out by shipConstraintsCB callback function.
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsOnlyFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (!shipConstraintsCB(selection->ShipPtr[i]))
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
    Name        : DoAllShipsFollowConstraints
    Description : Returns TRUE if all ships in selection follow shipConstraintsCB
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      : Returns TRUE if all ships in selection follow shipConstraintsCB
----------------------------------------------------------------------------*/
bool DoAllShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (!shipConstraintsCB(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : DoAnyShipsFollowConstraints
    Description : Returns TRUE if any ships in selection follow shipConstraintsCB
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      : Returns TRUE if any ships in selection follow shipConstraintsCB
----------------------------------------------------------------------------*/
bool DoAnyShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (shipConstraintsCB(selection->ShipPtr[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsNotIncludeMissiles
    Description : Makes sure that the targets in selection are not missiles.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsNotIncludeMissiles(SelectAnyCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numTargets;)
    {
        if ((selection->TargetPtr[i])->objtype == OBJ_MissileType)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyEnemyShips
    Description : Makes sure that the targets in this selection are ships and are enemies (not curplayer's ships)
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyEnemyShips(SelectAnyCommand *selection,Player *curplayer)
{
    sdword i;
    SpaceObjRotImpTarg *target;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];

        if ((target->objtype != OBJ_ShipType) ||
            (((Ship *)target)->shiptype == Drone) ||   // Drones not considered ships in this case
            (((Ship *)target)->playerowner == curplayer) ||
            allianceIsShipAlly((Ship *)target, curplayer))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyNonForceAttackTargets
    Description : Makes sure that the targets in this selection are ships and are enemies (not curplayer's ships)
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyNonForceAttackTargets(SelectAnyCommand *selection,Player *curplayer)
{
    sdword i;
    SpaceObjRotImpTarg *target;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];
        if ((target->objtype == OBJ_DerelictType) && (((Derelict *)target)->derelicttype == HyperspaceGate))
        {
            goto dontremove;
        }

        if (target->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
        {
            goto dontremove;
        }

        if ((target->objtype != OBJ_ShipType) ||
            (((Ship *)target)->shiptype == Drone) ||   // Drones not considered ships in this case
			(((Ship *)target)->shiptype == JunkYardHQ) ||
            (((Ship *)target)->playerowner == curplayer) ||
            allianceIsShipAlly((Ship *)target, curplayer))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
dontremove:
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsSalvageable
    Description : Filters the targets for any ship or derelict that's salvageable
    Inputs      : selection - the targets to filter,
                  curplayer - determines while player is enemy
    Outputs     : Changes selection
    Return      : void
----------------------------------------------------------------------------*/
void MakeTargetsSalvageable(SelectAnyCommand *selection, Player *curplayer)
{
    udword i;
    SpaceObjRotImpTarg *target;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];

        if (target->objtype == OBJ_DerelictType)
        {
            /*switch (((Derelict *)target)->derelicttype)
            {
                case AngelMoon:
                case AngelMoon_clean:
                case Homeworld:
                case PlanetOfOrigin:
                case PlanetOfOrigin_scarred:
                case Scaffold:
                case Scaffold_scarred:
                case ScaffoldFingerA_scarred:
                case ScaffoldFingerB_scarred:
                    //Don't salvage these
                    selection->numTargets--;
                    selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
                    continue;
                    break;
                default:
                    break;
            }*/
            if (!((DerelictStaticInfo *)target->staticinfo)->salvageable)
            {
                selection->numTargets--;
                selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
                continue;
            }
        }
        //!!!Single Player Game Mission Specific Code!!!
        //here's what's happening:
        //if it's a ship (but not a drone)
        //and it's not the players or an allies
        //exceptions to the rule are CryoTrays - they can always be salvaged
        //except for single player Mission 1
        else if (target->objtype == OBJ_ShipType)
        {
            if  ((((Ship *)target)->shiptype == Drone) ||     // Drones not considered ships in this case
                ( ( (((Ship *)target)->shiptype != CryoTray) ||
                    (singlePlayerGameInfo.currentMission == 1) ) && // Don't salvage player's or ally's shis, unless they are CryoTrays
                (( (((Ship *)target)->playerowner == curplayer) ||
                   allianceIsShipAlly((Ship *)target, curplayer) )
                 && !bitTest(((Ship *)target)->flags, SOF_Disabled))    ))
            {
                selection->numTargets--;
                selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
                continue;
            }
        }
        else
        {
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
            continue;
        }

        i++;
    }
}



/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNotHaveCloakedEnemyShips
    Description : Removes enemy cloaked ships from the selection
    Inputs      : selection - the selection being filtered,
                  curplayer - the current player
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void MakeSelectionNotHaveCloakedEnemyShips(SelectCommand *selection, Player *curplayer)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if ((ship->playerowner != curplayer) && (bitTest(ship->flags, SOF_Cloaked)))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsNotHaveNonCombatShipsForGuardAttack
    Description : Removes CloakGen...and other related ships
                    so that selection can be given an ATATCK order
                    while guarding and such that the cloak gen and other
                    ships are left guarding
    Inputs      : selection - the selection being filtered,
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/

void makeShipsNotHaveNonCombatShipsForGuardAttack(SelectCommand *selection)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if (ship->shiptype == CloakGenerator ||
            ship->shiptype == GravWellGenerator ||
            ship->shiptype == DFGFrigate)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNotHaveNonVisibleEnemyShips
    Description : Removes non-visible enemy ships from the selection
                  Note: takes into account Proximity Sensors detecting cloaked ships
    Inputs      : selection - the selection being filtered,
                  curplayer - the current player
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void MakeSelectionNotHaveNonVisibleEnemyShips(SelectCommand *selection, Player *curplayer)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if ((ship->playerowner != curplayer) &&
            (bitTest(ship->flags, SOF_Cloaked)) &&
            (!proximityCanPlayerSeeShip(curplayer, ship)))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
}



/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyBeWithinRangeAndNotIncludeMe
    Description : Makes targets all be within range of comparewith, but don't include comparewith
    Inputs      : selection, comparewith, range
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyBeWithinRangeAndNotIncludeMe(SelectAnyCommand *selection,SpaceObjRotImpTarg *comparewith,real32 range)
{
    sdword i;
    SpaceObjRotImpTarg *target;
    real32 negrange = -range;
    vector diff;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];

        vecSub(diff,target->posinfo.position,comparewith->posinfo.position);
        if ((target == comparewith) ||
            !isBetweenExclusive(diff.x,negrange,range) ||
            !isBetweenExclusive(diff.y,negrange,range) ||
            !isBetweenExclusive(diff.z,negrange,range) )
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsFriendlyShips
    Description : Makes sure that the ships in this selection are enemies (not curplayer's ships)
                  Removes any friendly ships from the selection
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsFriendlyShips(SelectCommand *selection,Player *curplayer)
{
    ShipPtr ship;
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if ((ship->objtype != OBJ_ShipType) ||
            (ship->playerowner != curplayer) )
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
    Name        : MakeShipsFriendlyAndAlliesShips
    Description : Makes sure that the ships in this selection are curplayer's and allies  ships
                  Removes any enemy ships from the selection
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsFriendlyAndAlliesShips(SelectCommand *selection,struct Player *curplayer)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if  ((selection->ShipPtr[i]->objtype != OBJ_ShipType)
            || ( ((selection->ShipPtr[i])->playerowner != curplayer) &&
               (!allianceIsShipAlly(selection->ShipPtr[i], curplayer)) ) )
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
    Name        : makeshipMastersincludeslaves(SelectCommand *selection)
    Description : Checks the list of selected ships and if it finds a Master in the list,
                  it adds the slaves aswell.
    Inputs      : selection
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
void MakeShipMastersIncludeSlaves(SelectCommand *selection)
{
    sdword i,count;
    Node *slavenode;
    count = selection->numShips;

    for (i=0;i < count;)
    {
        if ((selection->ShipPtr[i])->flags & SOF_Slaveable)
        {   //ship is a most likely a master...
            if ((selection->ShipPtr[i])->slaveinfo->flags & SF_MASTER)
            {
                slavenode = (selection->ShipPtr[i])->slaveinfo->slaves.head;
                while(slavenode != NULL)
                {
                    if(selection->numShips < COMMAND_MAX_SHIPS)
                    {   //haven't reached maximum  so add slave onto end
                        selection->ShipPtr[selection->numShips] = (Ship *)listGetStructOfNode(slavenode);
                        selection->numShips++;
                    }
                    else
                    {   //reached max so break.
                        break;
                    }
                    slavenode = slavenode->next;
                }
            }
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selectAllCurrentPlayersNonHyperspacingShips
    Description : Selects all of the current player's non-hyperspacing ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllCurrentPlayersNonHyperspacingShips(void)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            if(!(ship->flags & SOF_Disabled))
            {
                if ((!ShipCanHyperspace(ship)) && (ship->shiptype != Drone) && (ship->shiptype != CryoTray) && (ship->shiptype != Probe))
                {
                    selection->ShipPtr[i++] = ship;
                }
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = i;

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : selectAllCurrentPlayersHyperspacingShips
    Description : Selects all of the current player's hyperspacing ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllCurrentPlayersHyperspacingShips(void)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            if(!(ship->flags & SOF_Disabled))
            {
                if (ShipCanHyperspace(ship))
                {
                    selection->ShipPtr[i++] = ship;
                }
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = i;

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : selectAllPlayersShips
    Description : selects all of a player's ships
    Inputs      : player
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllPlayersShips(struct Player *player)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            selection->ShipPtr[i++] = ship;
        }

        objnode = objnode->next;
    }

    selection->numShips = i;

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNonResearchShips(SelectCommand *selectcom)
    Description : Removes all research ship types from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void MakeSelectionNonResearchShips(SelectCommand *selectcom)
{
    ubyte i;

    for (i = 0; i < selectcom->numShips;)
    {
        if(selectcom->ShipPtr[i]->shiptype == ResearchShip)
        {
            selectcom->numShips--;                  //decrease # of ships by 1
            selectcom->ShipPtr[i] = selectcom->ShipPtr[selectcom->numShips];  //copy last ship ptr to recently removed shipptr index
            continue;
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsSingleClickSpecialCapable
    Description : Makes a selection of just ships that are have the
                    canSingleClickSpecialActivate flag
    Inputs      : source - selection to search through
    Outputs     : dest - where to store the culled selection
    Return      : number of ships in culled selection
----------------------------------------------------------------------------*/
sdword MakeShipsSingleClickSpecialCapable(SelectCommand *dest, SelectCommand *source)
{
/*
    sdword index;

    dest->numShips = 0;
    for (index = 0; index < source->numShips; index++)
    {
        if (source->ShipPtr[index]->staticinfo->canSingleClickSpecialActivate)
        {
            dest->ShipPtr[dest->numShips] = source->ShipPtr[index];
            dest->numShips++;
        }
    }
    return(dest->numShips);
*/
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        if (source->ShipPtr[index]->staticinfo->canSingleClickSpecialActivate)
        {
            dest->ShipPtr[numShips] = source->ShipPtr[index];
            numShips++;
        }
    }
    dest->numShips = numShips;
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsNotDocked
    Description : Makes a selection of ships that are not hiddel
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword MakeShipsNotHidden(SelectCommand *dest, SelectCommand *source)
{
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        if (!bitTest(source->ShipPtr[index]->flags, SOF_Hide))
        {
            dest->ShipPtr[numShips] = source->ShipPtr[index];
            numShips++;
        }
    }
    dest->numShips = numShips;
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : selectSelectionInterception
    Description : Returns a selection consisting of ships in both selection1 and selection2
    Inputs      : selection1, selection2
    Outputs     : Creates a new selection
    Return      : the new selection, NULL if there's no intersection
----------------------------------------------------------------------------*/
SelectCommand *selectSelectionIntersection(SelectCommand *selection1, SelectCommand *selection2)
{
    SelectCommand *smallerSelection, *largerSelection;
    SelectCommand *intersection = NULL;
    ShipPtr ship;
    sdword i;

    if (selection1->numShips < selection2->numShips)
    {
        smallerSelection = selection1;
        largerSelection  = selection2;
    }
    else
    {
        smallerSelection = selection2;
        largerSelection  = selection1;
    }

    intersection = selectDupSelection(smallerSelection);

    for (i=intersection->numShips-1; i >= 0; i--)
    {
        ship = intersection->ShipPtr[i];

        if (!ShipInSelection(largerSelection, ship))
        {
            clRemoveShipFromSelection(intersection, ship);
        }
    }

    if (!intersection->numShips)
    {
        memFree(intersection);
        intersection = NULL;
    }

    return intersection;
}




/*-----------------------------------------------------------------------------
    Name        : selectMergeTwoSelections
    Description : Takes two selections and makes one big one. Note that a new selection
                  is allocated in memory.
    Inputs      : selection1, selection2 - the selections to merge
    Outputs     : Allocates memory space for a new selection, frees the other two spaces
    Return      : The new selection
----------------------------------------------------------------------------*/
SelectCommand *selectMergeTwoSelections(SelectCommand *selection1, SelectCommand *selection2, udword dealloc)
{
    SelectCommand *new_selection;
    udword i, j;

    if (selection1 == NULL)
    {
        //if selection1 is NULL, copy selection2 into the new selection
        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection2->numShips), "merge_sel2", 0);
        new_selection->numShips = selection2->numShips;

        for (i = 0; i < selection2->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection2->ShipPtr[i];
        }
        if ((dealloc == DEALLOC2) || (dealloc == DEALLOC_BOTH))
        {
            memFree(selection2);
        }
    }
    else if (selection2 == NULL)
    {
        //if selection2 is NULL, copy selection1 into the new selection
        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection1->numShips), "merge_sel1", 0);
        new_selection->numShips = selection1->numShips;

        for (i = 0; i < selection1->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection1->ShipPtr[i];
        }
        if ((dealloc == DEALLOC1) || (dealloc == DEALLOC_BOTH))
        {
            memFree(selection1);
        }
    }
    else
    {
        MakeShipsNotIncludeTheseShips(selection1, selection2);

        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection1->numShips + selection2->numShips), "merge_sel", 0);

        for (i = 0; i < selection1->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection1->ShipPtr[i];
        }
        for (j = 0; j < selection2->numShips; j++, i++)
        {
            new_selection->ShipPtr[i] = selection2->ShipPtr[j];
        }

        new_selection->numShips = i;

        switch (dealloc)
        {
            case DEALLOC1:
                memFree(selection1);
                break;
            case DEALLOC2:
                memFree(selection2);
                break;
            case DEALLOC_BOTH:
                memFree(selection1);
                memFree(selection2);
                break;
            default:
                break;
        }
    }

    return new_selection;
}


/*-----------------------------------------------------------------------------
    Name        : selectMemDupSelection
    Description : Creates a new selection that is an exact duplicate of another selection
    Inputs      : selection - the selection to duplicate
                  flag - the memory allocation flag
    Outputs     : Allocates memory for the new selection
    Return      : The new selection
----------------------------------------------------------------------------*/
SelectCommand *selectMemDupSelection(SelectCommand *selection, char *str, udword mmflag)
{
    SelectCommand *new_selection;

    new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection->numShips), str, mmflag);

    memcpy(new_selection,selection,sizeofSelectCommand(selection->numShips));

    return new_selection;
}

/*-----------------------------------------------------------------------------
    Name        : MothershipOrCarrierIndexInSelection
    Description : returns index where a mothership or carrier is in selection.  -1 if none found.
                  Mothership has higher priority
    Inputs      :
    Outputs     :
    Return      : returns index where a mothership or carrier is in selection.  -1 if none found
----------------------------------------------------------------------------*/
sdword MothershipOrCarrierIndexInSelection(SelectCommand *selection)
{
    sdword numShips = selection->numShips;
    sdword index = -1;
    sdword i;
    ShipType type;

    // go through selection to find index where a mothership or carrier is.  Give higher priority to motherships.
    for (i=0;i<numShips;i++)
    {
        type = selection->ShipPtr[i]->shiptype;
        if ((type == Mothership) || (type == P1Mothership) || (type == P2Mothership))
        {
            return i;
        }
        else if (type == Carrier)
        {
            index = i;
        }
    }

    return index;
}

/*=============================================================================
    Grow select functions
=============================================================================*/

void growSelectAddShip(GrowSelection *growSelect,Ship *ship)
{
    SelectCommand *shipList;

    // reserve more space if required
    if (growSelect->selection->numShips >= growSelect->maxNumShips)
    {
        growSelect->maxNumShips += GROWSELECT_ADDBATCH;
        shipList = memAlloc(sizeofSelectCommand(growSelect->maxNumShips), "regrowships", 0);
        memcpy(shipList, growSelect->selection, sizeofSelectCommand(growSelect->selection->numShips));
        memFree(growSelect->selection);
        growSelect->selection = shipList;
    }

    growSelect->selection->ShipPtr[growSelect->selection->numShips] = ship;
    growSelect->selection->numShips += 1;
}

bool growSelectRemoveShip(GrowSelection *growSelect,Ship *ship)
{
    return clRemoveShipFromSelection(growSelect->selection,ship);
}

/*-----------------------------------------------------------------------------
    Name        : growSelectRemoveShipBySettingNULL
    Description : Remove ship references from a grow selection
    Inputs      :
    Outputs     :
    Return      : TRUE if a ship reference was removed
----------------------------------------------------------------------------*/
bool growSelectRemoveShipBySettingNULL(GrowSelection *growSelect,Ship *ship)
{
    SelectCommand *selection = growSelect->selection;
    sdword numShips;
    sdword i;

    if (selection == NULL)
    {
        return FALSE;
    }
    numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            selection->ShipPtr[i] = NULL;
            return TRUE;
        }
    }

    return FALSE;
}

void growSelectRemoveShipIndex(GrowSelection *growSelect,sdword index)
{
    SelectCommand *selection = growSelect->selection;
    dbgAssert(index < selection->numShips);
    selection->numShips--;
    selection->ShipPtr[index] = selection->ShipPtr[selection->numShips];
}

void growSelectInit(GrowSelection *growSelect)
{
    growSelect->maxNumShips = GROWSELECT_INITIALBATCH;
    growSelect->selection = memAlloc(sizeofSelectCommand(GROWSELECT_INITIALBATCH), "growships", 0);
    growSelect->selection->numShips = 0;
}

void growSelectClose(GrowSelection *growSelect)
{
    if (growSelect->selection)
    {
        memFree(growSelect->selection);
        growSelect->selection = NULL;
    }
    growSelect->maxNumShips = 0;
}

void growSelectReset(GrowSelection *growSelect)
{
    growSelect->selection->numShips = 0;
}

void growSelectAddShipNoDuplication(GrowSelection *growSelect, Ship *ship)
{
    if (!ShipInSelection(growSelect->selection, ship))
    {
        growSelectAddShip(growSelect, ship);
    }
}


bool MakeSelectionKamikazeCapable(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        if(selection->ShipPtr[i]->staticinfo->shipclass != CLASS_Fighter &&
           selection->ShipPtr[i]->staticinfo->shipclass != CLASS_Corvette &&
           selection->ShipPtr[i]->staticinfo->shiptype != ResourceCollector)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }
    if(selection->numShips <= 0)
        return FALSE;
    return TRUE;
}


