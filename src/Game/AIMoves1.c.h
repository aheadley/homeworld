#include "FastMath.h"
#include "Select.h"
#include "AIUtilities.h"
#include "AIHandler.h"
#include "AIAttackMan.h"
#include "Dock.h"
#include "SalCapCorvette.h"
#include "SinglePlayer.h"
#include "Tutor.h"

/*=============================================================================
    Special Move functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aimInsertMove
    Description : Inserts a new move into the team's movelist and sets the
                  move as the team's current move
    Inputs      : move - the move to add, team - the team to add it to
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimInsertMove(AITeam *team, struct AITeamMove *newMove)
{
    if (team->curMove == NULL)
    {
        listAddNode(&(team->moves), &(newMove->listNode), newMove);
        listMoveNodeToHead(&(newMove->listNode));
        team->curMove = (AITeamMove *)listGetStructOfNode(team->moves.head);
    }
    else
    {
        if ((team->curMove->type != MOVE_GETSHIPS) &&
            (team->curMove->type != MOVE_FANCYGETSHIPS))
        {
            team->curMove->processing = FALSE;
        }

        aitAddmoveBeforeAndMakeCurrent(team, newMove, team->curMove);
    }
}


/*=============================================================================
    Move Callback functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aimMoveSplitShipDied
    Description : Removes the reference to the dead ship, if there is one
    Inputs      : team - split team, move - thismove, ship - the ship that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimMoveSplitShipDied(AITeam *team, AITeamMove *move, ShipPtr ship)
{
    udword i;
    bool found = FALSE;

    if (move->params.movesplit.ships)
    {
        for (i=0;i<move->params.movesplit.ships->numShips;i++)
        {
            if (found == TRUE)
            {
                move->params.movesplit.destinations->point[i-1] = move->params.movesplit.destinations->point[i];
                move->params.movesplit.ships->ShipPtr[i-1] = move->params.movesplit.ships->ShipPtr[i];
            }

            if (ship == move->params.movesplit.ships->ShipPtr[i])
            {
                found = TRUE;
                aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from move split"));
            }
        }

        if (found)
        {
            move->params.movesplit.ships->numShips--;
            aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from move split"));
        }
    }
    else
    {
        dbgAssert(FALSE);
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimMoveSplitClose
    Description : Deallocates the move split stuff
    Inputs      : team, move
    Outputs     : Deallocates the ships structure
    Return      : void
----------------------------------------------------------------------------*/
void aimMoveSplitClose(AITeam *team, AITeamMove *move)
{
    aiumemFree(move->params.movesplit.ships);
    aiumemFree(move->params.movesplit.destinations);
    aiplayerLog((team->aiplayerowner->player->playerIndex, "%x Closed Move Split Move", team));
}




/*-----------------------------------------------------------------------------
    Name        : aimInterceptShipDied
    Description : Checks to see if the ship that died is the ship being intercepted.
                  Removes references if this is true
    Inputs      : team - the team intercepting,
                  move - the intercept move,
                  ship - the ship that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimInterceptShipDied(AITeam *team, AITeamMove *move, ShipPtr ship)
{
    if (ship == move->params.intercept.ship)
    {
        move->params.intercept.ship = NULL;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aimFlankAttackClose
    Description : Cleanly closes the Flank Attack move
    Inputs      : team - the team the move belongs to
                  move - the move to close
    Outputs     : Deallocates the targets structure
    Return      : void
----------------------------------------------------------------------------*/
void aimFlankAttackClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.flankatt.targets);
    aiplayerLog((team->aiplayerowner->player->playerIndex,"%x Closed Flank Attack Move", team));
}


/*-----------------------------------------------------------------------------
    Name        : aimFlankAttackShipDied
    Description : Deals with a ship dying while a team is has a flanking attack in its move list
    Inputs      : team - the team with the flanking attack move
                  move - the flanking attack move
                  ship - the ship that died
    Outputs     : Removes the ship from any structure it might be a part of
    Return      : void
----------------------------------------------------------------------------*/
void aimFlankAttackShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if (move->params.flankatt.targets != NULL)
    {
        if (clRemoveShipFromSelection(move->params.advatt.targets, ship))
        {
            aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from Flanking Attack Target"));
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimAdvancedAttackClose
    Description : Cleanly closes the advanced attack move
    Inputs      : team - the team the move belongs to, move - the move to close
    Outputs     : Deallocates the targets structure
    Return      : void
----------------------------------------------------------------------------*/
void aimAdvancedAttackClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.advatt.targets);
    aiplayerLog((team->aiplayerowner->player->playerIndex,"%x Closed Advanced Attack Move", team));
}


/*-----------------------------------------------------------------------------
    Name        : aimAdvancedAttackShipDied
    Description : Deals with a ship dying while a team is performing an advanced attack.
                  The target selection and target_ship pointer need to be checked to see
                  if the dead ship is part of that selection
    Inputs      : team - the attacking team
                  move - the advanced attack move
                  ship - the dead ship
    Outputs     : Removes the ship from any structure it might be a part of
    Return      : void
----------------------------------------------------------------------------*/
void aimAdvancedAttackShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if (move->params.advatt.targets != NULL)
    {
        if (clRemoveShipFromSelection(move->params.advatt.targets, ship))
        {
    // postfinal: below commented out to try reissuing the attack after a target dies
//            if (move->params.advatt.target_ship == ship)
//            {
//                move->params.advatt.target_ship = NULL;
//                move->processing = FALSE;

                if (team->curMove == move)
                {
                    move->processing = FALSE;
//                  aimProcessAdvancedAttack(team)
                }
//            }
            aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from Advanced Attack Target"));
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimMoveAttackShipDied
    Description : Ship dying while a move attack is happening
    Inputs      : team, move, ship
    Outputs     : removes ship from structure
    Return      : void
----------------------------------------------------------------------------*/
void aimMoveAttackShipDied(AITeam *team, struct AITeamMove *move, ShipPtr ship)
{
    if (move->params.moveatt.targets != NULL)
    {
        if (clRemoveShipFromSelection(move->params.moveatt.targets, ship))
        {
            if (move->params.moveatt.target_ship == ship)
            {
                move->params.moveatt.target_ship = NULL;
            }
            aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from MoveAttack move"));
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : aimMoveAttackClose
    Description : Closes the move attack move
    Inputs      : team, move
    Outputs     : deallocates the targets structure
    Return      : void
----------------------------------------------------------------------------*/
void aimMoveAttackClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.moveatt.targets);
    aiplayerLog((team->aiplayerowner->player->playerIndex,"%x Closed Move Attack Move", team));
}



/*-----------------------------------------------------------------------------
    Name        : aimHarassAttackShipDied
    Description : Deals with a ship dying while a team is doing a harass attack
                  - if the ship that died is the ship the team was harassing,
                    the reference to that ship is removed
    Inputs      : team - the harassing team
                  move - the harass move
                  ship - the ship that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimHarassAttackShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if ((move->params.harass.target != NULL) && (move->params.harass.target == ship))
    {
        //removes the target ship from the structure
        move->params.harass.target = NULL;

        if (move->events.firing.triggered)
        {
            move->events.firing.triggered = FALSE;
            if (team->shipList.selection->numShips)
            {
                aiuWrapFormation(team->shipList.selection,DELTA3D_FORMATION);
            }
        }

        //if the ship was being harassed, it is removed
        if (move->processing)
        {
            move->processing = FALSE;
        }
        aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from harass target"));
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimDockClose
    Description : Cleanly closes the dock move
    Inputs      : team - the team the move belongs to
                  move - the move to close
    Outputs     : Deallocates the shipsDocking structure
    Return      : void
----------------------------------------------------------------------------*/
void aimDockClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.dock.shipsDocking);
    move->params.dock.shipsDocking = NULL;
    move->params.dock.dockAt = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : aimDockShipDied
    Description : Deals with a ship dying while a team is doing a dock command
    Inputs      : team - the docking team
                  move - the dock move
                  ship - the ship that died
    Outputs     : removes references of the ship from the dock structure
    Return      : void
----------------------------------------------------------------------------*/
void aimDockShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if (move->params.dock.shipsDocking != NULL)
    {
        if (clRemoveShipFromSelection(move->params.dock.shipsDocking, ship))
        {
            aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from dock structure"));
        }
    }
    if (move->params.dock.dockAt == ship)
    {
        move->params.dock.dockAt = NULL;
        aiplayerLog((team->aiplayerowner->player->playerIndex,"Cleared dockAt Ship from dock structure"));
    }
}

/*-----------------------------------------------------------------------------
    Name        : aimDefendMothershipClose
    Description : Cleanly closes the defend mothership move
    Inputs      : team - the team defending the mothership,
                  move - the move to close
    Outputs     : Deallocates the defend mothership structure
    Return      : void
----------------------------------------------------------------------------*/
void aimDefendMothershipClose(AITeam *team, struct AITeamMove *move)
{
    //blah blah blah
}


/*-----------------------------------------------------------------------------
    Name        : aimDefendMothershipShipDied
    Description : Deals with a ship dying while a team is defending the mothership
    Inputs      : team - the team defending the mothership
                  move - the defend mothership move
                  ship - the ship that died
    Outputs     : removes references of the ship from the move structure
    Return      : void
----------------------------------------------------------------------------*/
void aimDefendMothershipShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if ((move->params.defmoship.targets) && (clRemoveShipFromSelection(move->params.defmoship.targets, ship)))
    {
        memFree(move->params.defmoship.targets);
        move->params.defmoship.targets = NULL;
        aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from defend mothership structure"));
        move->processing = FALSE;
        aimProcessDefendMothership(team);
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimPatrolMoveClose
    Description : Deallocates the path structure
    Inputs      : team - the team that is executing the move, move - the move
    Outputs     : deallocates the path structure in the patrol move
    Return      : void
----------------------------------------------------------------------------*/
void aimPatrolMoveClose(AITeam *team, struct AITeamMove *move)
{
    memFree(move->params.patrolmove.path);
}


/*-----------------------------------------------------------------------------
    Name        : aimSupportShipDied
    Description : Deals with a ship dying while a team is supporting a set of ships
    Inputs      : team - the team doing the supporting,
                  move - the support move,
                  ship - the ship that died
    Outputs     : removes references of the ship from the move structure
    Return      : void
----------------------------------------------------------------------------*/
void aimSupportShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if ((move->params.support.ships) && (clRemoveShipFromSelection(move->params.support.ships, ship)))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex,"Removed Ship from Support move structure"));
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimSupportClose
    Description : Deallocates the ships structure in the support move
    Inputs      : team - the team that is executing the move,
                  move - the move being executed
    Outputs     : Deallocates a selection
    Return      : void
----------------------------------------------------------------------------*/
void aimSupportClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.support.ships);
}


/*-----------------------------------------------------------------------------
    Name        : aimShipReconShipDied
    Description : Deals with a ship dying while a team is doing a ship reconaissance
    Inputs      : team - the team doing the reconaissance,
                  move - the ship recon move,
                  ship - the ship that died
    Outputs     : removes references of the ship from the move structure
    Return      : void
----------------------------------------------------------------------------*/
void aimShipReconShipDied(AITeam *team, struct AITeamMove *move, Ship *ship)
{
    if (clRemoveShipFromSelection(move->params.shiprecon.ships, ship))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from ReconShip ships"));
    }

    if ((move->params.shiprecon.foundships) &&
        (clRemoveShipFromSelection(move->params.shiprecon.foundships, ship)))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from ReconShips shipsfound"));

        if (!move->params.shiprecon.foundships->numShips)
        {
            memFree(move->params.shiprecon.foundships);
            move->params.shiprecon.foundships = NULL;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimShipReconClose
    Description : Closes the shiprecon move
    Inputs      : move - the shiprecon move
    Outputs     : deallocs a few structures
    Return      : void
----------------------------------------------------------------------------*/
void aimShipReconClose(AITeam *team, struct AITeamMove *move)
{
    aiumemFree(move->params.shiprecon.foundships);
    aiumemFree(move->params.shiprecon.ships);
}


/*-----------------------------------------------------------------------------
    Name        : aimControlResourcesShipDied
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimControlResourcesShipDied(AITeam *team, struct AITeamMove *move, ShipPtr ship)
{
    //already taken care of because the ships pointer points to
    //aiplayer->airResourceCollectors, which removes ships on its own
}


/*-----------------------------------------------------------------------------
    Name        : aimSwarmAttackShipDied
    Description : Deals with a ship dying while a swarm attack is happening
    Inputs      : team, move ship
    Outputs     : removes the ship from one of the swarm attack structures
    Return      : void
----------------------------------------------------------------------------*/
void aimSwarmAttackShipDied(AITeam *team, struct AITeamMove *move, ShipPtr ship)
{
    SelectCommand *targets = move->params.swarmatt.targets;
    SelectCommand *othertargets = move->params.swarmatt.othertargets;
    SelectCommand *teamShips = team->shipList.selection;

    if (growSelectRemoveShip(&move->params.swarmatt.newSwarmers, ship))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from swarm attack new swarmers"));
        return;
    }

    if ((othertargets) && clRemoveShipFromSelection(othertargets, ship))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from swarm attack othertargets"));
        if (!othertargets->numShips)
        {
            aiumemFree(move->params.swarmatt.othertargets);
        }
    }

    if ((targets) &&
        (clRemoveShipFromSelection(targets, ship)))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from swarm attack targets"));
        if (!targets->numShips)
        {
//            aiumemFree(move->params.swarmatt.targets);
//            targets = move->params.swarmatt.targets = aiuFindNearbyEnemyShips(ship, 1000);
//            if (!targets->numShips)
//            {
                aiumemFree(move->params.swarmatt.targets);
                return;
//            }
        }

        if (teamShips->numShips)
        {
            //give attack command to all swarmers
            if ((teamShips->numShips > 2*targets->numShips) &&
                ((isShipOfClass(targets->ShipPtr[0], CLASS_Corvette)) ||
                 (isShipOfClass(targets->ShipPtr[0], CLASS_Fighter))))
            {
                //split them if possible
                aiuSplitAttack(teamShips, targets);
            }
            else
            {
                //group attack otherwise
                aiuWrapAttack(teamShips, targets);
            }
        }
        return;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimSwarmAttackClose
    Description : Closes swarm attack move
    Inputs      : team, move
    Outputs     : frees a few structures
    Return      : void
----------------------------------------------------------------------------*/
void aimSwarmAttackClose(AITeam *team, AITeamMove *move)
{
    growSelectClose(&move->params.swarmatt.newSwarmers);
    aiumemFree(move->params.swarmatt.targets);
    aiumemFree(move->params.swarmatt.othertargets);
}


/*-----------------------------------------------------------------------------
    Name        : aimSwarmDefenseShipDied
    Description : Deals with a ship dying while a swarm defense is happening
    Inputs      : team, move ship
    Outputs     : removes the ship from one of the swarm defense structures
    Return      : void
----------------------------------------------------------------------------*/
void aimSwarmDefenseShipDied(AITeam *team, struct AITeamMove *move, ShipPtr ship)
{
    if (growSelectRemoveShip(&move->params.swarmdef.newSwarmers, ship))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from swarm defense new swarmers"));
        return;
    }

    if (move->params.swarmdef.guarding)
    {
        if (ship == move->params.swarmdef.guarding->ShipPtr[0])
        {
            bitClear(team->teamFlags, TEAM_AmIBeingWatched);
        }
        if (clRemoveShipFromSelection(move->params.swarmdef.guarding, ship))
        {
            aiplayerLog((team->aiplayerowner->player->playerIndex, "Removed Ship from swarm defense guarding"));
            return;
        }
    }

    if ((move->params.swarmdef.Pods) &&
        (clRemoveShipFromSelection(move->params.swarmdef.Pods, ship)))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Swarmer Pod killed"));
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimSwarmDefenseClose
    Description : Closes swarm defense move
    Inputs      : team, move
    Outputs     : frees a few structures
    Return      : void
----------------------------------------------------------------------------*/
void aimSwarmDefenseClose(AITeam *team, AITeamMove *move)
{
    growSelectClose(&move->params.swarmdef.newSwarmers);
    aiumemFree(move->params.swarmdef.guarding);
    aiumemFree(move->params.swarmdef.Pods);
}


/*-----------------------------------------------------------------------------
    Name        : aimResourceVolumeResourceDied
    Description : Deals with a resource dying while the resource volume move exists
    Inputs      : team - the team doing the resourcing,
                  move - the resource volume move,
                  resource - the resource that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimResourceVolumeResourceDied(AITeam *team, AITeamMove *move, Resource *resource)
{
    udword i;

    if ((move->params.resvolume.volResources) &&
        (clRemoveTargetFromSelection((SelectAnyCommand *)move->params.resvolume.volResources, (TargetPtr)resource)))
    {
//        aiplayerLog((team->aiplayerowner->player->playerIndex, "Resource removed from volResources"));

        if (!move->params.resvolume.volResources->numResources)
        {
            aiumemFree(move->params.resvolume.volResources);
        }
    }
    if ((move->params.resvolume.takenResources) &&
        (clRemoveTargetFromSelection((SelectAnyCommand *)move->params.resvolume.takenResources, (TargetPtr)resource)))
    {
        aiplayerLog((team->aiplayerowner->player->playerIndex, "Resource removed from takenResources"));

//        if (!move->params.resvolume.takenResources->numResources)
//        {
//            aiumemFree(move->params.resvolume.takenResources);
//        }
    }


    if (move->params.resvolume.strictVolume)
    {
        for (i=0;i<team->shipList.selection->numShips;i++)
        {
            if (bitTest(team->shipList.selection->ShipPtr[i]->specialFlags, SPECIAL_Finished_Resource))
            {
                SelectCommand selone;
                selone.numShips = 1;
                selone.ShipPtr[0] = team->shipList.selection->ShipPtr[i];
                bitClear(selone.ShipPtr[0]->specialFlags, SPECIAL_Finished_Resource);
                aiuWrapHalt(&selone);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimResourceVolumeShipDied
    Description : If a resource collector dies, the resources of that volume
                  need to be recalculated
    Inputs      : team, move, ship
    Outputs     : Deallocates some structures
    Return      : void
----------------------------------------------------------------------------*/
void aimResourceVolumeShipDied(AITeam *team, AITeamMove *move, ShipPtr ship)
{
    if (ShipInSelection(team->shipList.selection, ship))
    {
        aiumemFree(move->params.resvolume.volResources);
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimResourceVolumeClose
    Description : Deallocates the resource volume structures
    Inputs      : team, move
    Outputs     : Deallocates some structures
    Return      : void
----------------------------------------------------------------------------*/
void aimResourceVolumeClose(AITeam *team, AITeamMove *move)
{
    aiumemFree(move->params.resvolume.volResources);
    aiumemFree(move->params.resvolume.takenResources);
}

/*-----------------------------------------------------------------------------
    Name        : aimCaptureShipDied
    Description : Deals with a ship dying while the capture move is being executed
    Inputs      : team, move, ship
    Outputs     : Deallocates some pointers
    Return      : void
----------------------------------------------------------------------------*/
void aimCaptureShipDied(AITeam *team, AITeamMove *move, ShipPtr ship)
{
    if (ship == move->params.capture.ship)
    {
        move->params.capture.ship = NULL;

        aiplayerLog((aiIndex, "Captured ship died"));
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimMothershipMoveShipDied
    Description : If a mothership dies, a new one must be found.  If there isn't one, we lost anyway, so there's no point
    Inputs      : team, move, ship
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aimMothershipMoveShipDied(AITeam *team, AITeamMove *move, ShipPtr ship)
{
    //do something here... make sure it's really cool.
}





/*=============================================================================
    Move Processing Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aimProcessMove
    Description : Moves the team if it isn't moving yet, detects when the team is done moving
    Inputs      : team - the team doing the moving
    Outputs     :
    Return      : Returns true when the team is done moving
----------------------------------------------------------------------------*/
sdword aimProcessMove(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Move Move, Zero Sized Team"));
        return TRUE;
    }


    if (!thisMove->processing)
    {
        //give command layer move command
        if (aiuWrapMove(team->shipList.selection, thisMove->params.move.destination))
        {
            thisMove->processing = TRUE;
        }
        else
        {
            aiplayerLog((aiIndex, "Can't Move!!!"));
            return FALSE;
        }
    }
    else
    {
        if (aitTeamIsFinishedMoving(team, thisMove->params.move.destination, AIM_DIST_MOVE_END))
        {
//            aiplayerLog((aiIndex,"Moving team reached destination"));
            return TRUE;
        }
    }

    if (thisMove->wait)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessMoveSplit
    Description : Moves each ship to their corresponding destination
    Inputs      : team - the team that is splitting
    Outputs     : Moves ships around
    Return      : Returns TRUE when the team is done moving
----------------------------------------------------------------------------*/
sdword aimProcessMoveSplit(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand selone;
    udword i;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex, "Move Split Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        selone.numShips = 1;

        for (i=0;i<thisMove->params.movesplit.ships->numShips;i++)
        {
            selone.ShipPtr[0] = thisMove->params.movesplit.ships->ShipPtr[i];
            aiuWrapMove(&selone, thisMove->params.movesplit.destinations->point[i]);
        }
        thisMove->processing = TRUE;
    }
    else
    {
        for (i=0;i<thisMove->params.movesplit.ships->numShips;i++)
        {
            if (!vecIsZero(thisMove->params.movesplit.ships->ShipPtr[i]->moveTo))
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    if (thisMove->wait)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessHyperspace
    Description : Hyperspaces the team to a certain destination
    Inputs      : team - the team that's hyperspacing
    Outputs     : See "description"
    Return      : Returns TRUE when the team is done hyperspacing
----------------------------------------------------------------------------*/
sdword aimProcessHyperspace(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    MaxSelection hyperspacingShips;
    real32 cost, avg_size;
    vector current_location;
    udword i;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex, "Hyperspace Move, Zero Sized Team"));
        return TRUE;
    }

    hyperspacingShips.numShips = 0;

    if (!thisMove->processing)
    {
        aiuFilterSelectableShips(team->shipList.selection,&hyperspacingShips);
        makeSelectionHyperspaceCapable((SelectCommand *)&hyperspacingShips);

        if (hyperspacingShips.numShips)
        {
            current_location = selCentrePointComputeGeneral((MaxSelection *)&hyperspacingShips, &avg_size);
            cost = hyperspaceCost(fsqrt(aiuFindDistanceSquared(current_location, thisMove->params.move.destination)), (SelectCommand *)&hyperspacingShips);

            aiCurrentAIPlayer->aifHyperSavings -= (sdword)cost;
            clWrapMpHyperspace(&universe.mainCommandLayer, (SelectCommand *)&hyperspacingShips, current_location, thisMove->params.move.destination);

            aiplayerLog((aiIndex, "%x Hyperspacing NOW!  Cost = %i  Savings = %i", team, (sdword)cost, aiCurrentAIPlayer->aifHyperSavings));
        }

        thisMove->processing = TRUE;

        return FALSE;
    }
    else
    {
        for (i=0;i<team->shipList.selection->numShips;i++)
        {
            if (!bitTest(team->shipList.selection->ShipPtr[0]->flags, SOF_Selectable))
            {
                return FALSE;
            }
        }
        return TRUE;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessIntercept
    Description : Checks the ship to see if it's moving, if it is,
                  calculate an intercept point, create a move to it's location
    Inputs      : team - the intercepting team
    Outputs     : May create some moveteam moves
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessIntercept(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    ShipPtr ship = thisMove->params.intercept.ship;

    if (ship == NULL)
    {
        return TRUE;
    }

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Intercept Move, Zero Sized Team"));
        return TRUE;
    }


    if (!thisMove->processing)
    {
        if (!vecIsZero(ship->moveTo))
        {
            //make destination halfway point between target ship and it's destination
            //later add code to deal with when the intercepting team is directly
            // in the path of the ship it's intercepting
            vecAdd(thisMove->params.intercept.destination, ship->moveTo, ship->posinfo.position);
            vecMultiplyByScalar(thisMove->params.intercept.destination, 0.5);
            thisMove->params.intercept.moving = TRUE;
        }
        else
        {
            thisMove->params.intercept.destination = ship->posinfo.position;
            thisMove->params.intercept.moving = FALSE;
        }

        newMove = aimCreateMoveTeamNoAdd(team, thisMove->params.intercept.destination, thisMove->formation, /*thisMove->tactics,*/ FALSE, TRUE);
        newMove->events = thisMove->events;
        thisMove->processing  = TRUE;
        thisMove->params.intercept.next_int = universe.totaltimeelapsed + thisMove->params.intercept.interval;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
    }
    else
    {
        //if the team has arrived
        if (aiuFindDistanceSquared(ship->posinfo.position, team->shipList.selection->ShipPtr[0]->posinfo.position) < AIM_INTERCEPT_FINISH_DISTSQ)
        {
            return TRUE;
        }

        //only do checks during certain intervals
        if (thisMove->params.intercept.next_int > universe.totaltimeelapsed)
        {
            return FALSE;
        }

        //if the intercept target has started moving or changed course, recalculate destination
        if ((!thisMove->params.intercept.moving) &&
            (aiuFindDistanceSquared(ship->posinfo.position, thisMove->params.intercept.destination) > AIM_INTERCEPT_RECALC_DISTSQ))
        {
            thisMove->processing = FALSE;
        }
        else if ((thisMove->params.intercept.moving) && (!vecAreEqual(ship->moveTo, thisMove->params.intercept.destination)))
        {
            thisMove->processing = FALSE;
        }
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessMoveTo
    Description : Moves a team for the amount of time or distance specified in limiter
    Inputs      : team - the team that's going to move
    Outputs     :
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessMoveTo(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    vector current_location;
    real32 avg_size;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Move To Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if (!aiuShipsAreHidden(team->shipList.selection))
        {
            //give command layer move command
            aiuWrapMove(team->shipList.selection, thisMove->params.moveTo.destination);

            thisMove->params.moveTo.timer  = universe.totaltimeelapsed;
            thisMove->params.moveTo.source = selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &avg_size);
            thisMove->processing = TRUE;
        }
    }
    else
    {
        switch (thisMove->params.moveTo.type)
        {
            case TIME_LIMITED:
                if (universe.totaltimeelapsed - thisMove->params.moveTo.timer > thisMove->params.moveTo.limiter)
                {
                    aiuWrapHalt(team->shipList.selection);
//                    aiplayerLog((aiIndex,"Move To team reached timed destination"));
                    return TRUE;
                }
                break;
            case DISTANCE_LIMITED:
                current_location = selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &avg_size);
                if (fsqrt(aiuFindDistanceSquared(current_location, thisMove->params.moveTo.source)) > thisMove->params.moveTo.limiter)
                {
                    aiuWrapHalt(team->shipList.selection);
//                    aiplayerLog((aiIndex,"Move To team reached distance destination"));
                    return TRUE;
                }
                break;
            default:
                aiplayerLog((aiIndex,"Unknown Move To type"));
        }
    }
    return FALSE;
}




/*-----------------------------------------------------------------------------
    Name        : aimProcessCountShips
    Description : Counts ships within visual range of the team
    Inputs      : team - the team doing the counting
    Outputs     :
    Return      : Returns TRUE when the team is done counting
----------------------------------------------------------------------------*/
sdword aimProcessCountShips(AITeam *team)
{
    //just a placeholder for now
    //later maybe actually count ships nearby
    aiplayerLog((aiIndex,"Counting Ships... 1, 2, 3, 4..."));
    return TRUE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessFlankAttack
    Description : Moves the ship to the flanking position, then issues an attack order
    Inputs      : team - the team doing the attacking
    Outputs     : A neato flanking attack
    Return      : Returns TRUE when the team is done attacking
----------------------------------------------------------------------------*/
sdword aimProcessFlankAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    vector current_location, target_location, flank_location, zerovec = {0,0,0};
    real32 avg_size;

    if (team->shipList.selection->numShips == 0)
    {
        //no ships to do anything with, finish move
        aiplayerLog((aiIndex,"Flank Attack Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        current_location = selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &avg_size);
        target_location = selCentrePointComputeGeneral((MaxSelection *)thisMove->params.flankatt.targets, &avg_size);

        //find the flanking coordinates
        flank_location = aiuGenerateFlankCoordinates(target_location, current_location, zerovec, AIM_FLANK_ATTACK_RADIUS);

        if (thisMove->params.flankatt.hyperspace)
        {
            //create a move to hyperspace to those coordinates (with a wait)
            newMove = aimCreateHyperspaceNoAdd(team, flank_location, AIM_FLANK_MOVE_FORMATION, TRUE, TRUE);
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
        else
        {
            //create a move to move to those coordinates (with a wait)
            newMove = aimCreateMoveTeamNoAdd(team, flank_location, AIM_FLANK_MOVE_FORMATION, TRUE, TRUE);
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }

        //create an attack move to target ships
        //previously normal attack later change to general attack
        newMove = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection(thisMove->params.flankatt.targets, "dupfla", 0), AIM_FLANK_ATTACK_FORMATION,Aggressive, TRUE, TRUE);
        listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

        //remove shipdied and close callbacks
        //the attack move will take care of these
//        thisMove->moveShipDiedFunction = NULL;
//        thisMove->moveCloseFunction = NULL;

        //leave this function
        thisMove->processing = TRUE;
        return FALSE;
    }

    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessMoveAttack
    Description : Gives the attack command then if in range and not moving, find a random flank point and move to it
    Inputs      : team
    Outputs     : Creates some new move
    Return      : TRUE when all the targets are dead
----------------------------------------------------------------------------*/
sdword aimProcessMoveAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    SelectCommand *victims = thisMove->params.moveatt.targets, *teamShips = team->shipList.selection;
    vector victims_center, teams_center, destination, origin = {0,0,0};
    real32 dummy,mingunrangesq;

    if (teamShips->numShips == 0)
    {
        //team is dead
        aiplayerLog((aiIndex,"Move Attack Move, Zero size team"));
        return TRUE;
    }

    if ((victims == NULL) || (victims->numShips == 0))
    {
        //we're done
        return TRUE;
    }

    if (!thisMove->processing)
    {
        newMove = aimCreateAttackNoAdd(team, selectMemDupSelection(victims, "moveatt", 0), thisMove->formation, FALSE, TRUE);
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

        thisMove->processing = TRUE;
        return FALSE;
    }
    else
    {
        if ((!team->shipList.selection->ShipPtr[0]->command) ||
            (team->shipList.selection->ShipPtr[0]->command->ordertype.order != COMMAND_ATTACK))
        {
            //somehow the attack command has been lost
            //reissue the attack command
            newMove = aimCreateAttackNoAdd(team, selectMemDupSelection(victims, "moveatt2",0), thisMove->formation, FALSE, TRUE);
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
            vecZeroVector(thisMove->params.moveatt.destination);

            return FALSE;
        }
        else if ((vecIsZero(thisMove->params.moveatt.destination) && aiuShipsInGunRangeOfTargets(team->shipList.selection)) ||
                 (!vecIsZero(thisMove->params.moveatt.destination) && vecIsZero(teamShips->ShipPtr[0]->moveTo)) ||
                 (aiuFindDistanceSquared(selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &dummy), thisMove->params.moveatt.destination) < 10000)) //10000 == range squared
        //if the team is in gun range and hasn't started circling around the enemy
        //or it has started circling around, but the move stopped for some reason (too close to the target, for example)
        //or if the team has finished a half circle
        {

            //may need to tweak mingunrange
            mingunrangesq  = aiuFindMinimumGunRangeSquared(teamShips);
            victims_center = selCentrePointComputeGeneral((MaxSelection *)victims, &dummy);
            teams_center   = selCentrePointComputeGeneral((MaxSelection *)teamShips, &dummy);
            destination    = aiuGenerateFlankCoordinates(victims_center, teams_center, origin, fsqrt(mingunrangesq));

            newMove = aimCreateMoveTeamNoAdd(team, destination, thisMove->formation, FALSE, TRUE);
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

            thisMove->params.moveatt.destination = destination;

            return FALSE;
        }
    }
    return FALSE;
}




/*-----------------------------------------------------------------------------
    Name        : aimProcessAdvancedAttack
    Description : Move advanced attacks - sequentially attacks individual
                  targets instead of all targets at once
    Inputs      : team - the team doing the attacking
    Outputs     : Kills lotsa ships
    Return      : Returns TRUE when the team is done attacking
----------------------------------------------------------------------------*/
sdword aimProcessAdvancedAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *moveTargets = thisMove->params.advatt.targets;
    SelectCommand *teamShips = team->shipList.selection;
    SelectCommand *target;

    if (team->shipList.selection->numShips == 0)
    {
        //no ships to do anything with, move done
        aiplayerLog((aiIndex,"Advanced Attack Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if ((teamShips->numShips) && (moveTargets) && (moveTargets->numShips))
            //^^^^^^^^^^^^^ why do I need this?
        {
            //if a non-fighter team is attacking fighters
/*            if ((aiuShipIsFighter(moveTargets->ShipPtr[0])) && (!aiuShipIsFighter(teamShips->ShipPtr[0])))
            {
*/                thisMove->params.advatt.target_ship = NULL;
                target = /*(AttackCommand *)*/selectMemDupSelection(moveTargets, "duppaa", 0);
/*            }
            else if ((moveTargets->numShips < AIM_SQUADRON_NUM_SHIPS) ||
                     (teamShips->numShips < AIM_SQUADRON_NUM_SHIPS))
            {
                thisMove->params.advatt.target_ship = thisMove->params.advatt.targets->ShipPtr[0];
                target = (AttackCommand *)memAlloc(sizeofAttackCommand(1), "advatt1", 0);
                target->numTargets = 1;
                target->TargetPtr[0] = (TargetPtr)thisMove->params.advatt.target_ship;
            }
            else
            {
                thisMove->params.advatt.target_ship = thisMove->params.advatt.targets->ShipPtr[0];
                target = (AttackCommand *)memAlloc(sizeofAttackCommand(2), "advatt2", 0);
                target->numTargets = 2;
                target->TargetPtr[0] = (TargetPtr)thisMove->params.advatt.target_ship;
                target->TargetPtr[1] = (TargetPtr)thisMove->params.advatt.targets->ShipPtr[1];
            }
*/

            thisMove->params.advatt.targets = aiuAttack(team, target);
            thisMove->processing = TRUE;

            //postfinal - removed because targets now part of the advatt structure
            //            memFree(target);
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        if ((!thisMove->params.advatt.targets) || (!thisMove->params.advatt.targets->numShips) ||
            (aiuShipsNoLongerAttacking(teamShips)))
        {
            aiumemFree(thisMove->params.advatt.targets);
            return TRUE;
        }
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessHarassAttack
    Description : Attacks wimpy ships and generally causes havoc
    Inputs      : team - the team doing the havoc causing
    Outputs     :
    Return      : Returns TRUE when the team is done attacking
----------------------------------------------------------------------------*/
sdword aimProcessHarassAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    real32 target_distance_sq;
    vector target_standoff_pt, approx_team_pos, target_pos;
    SelectCommand target, *protection;
    TypeOfFormation formation;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Harass Attack Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        bitClear(team->teamFlags, TEAM_RETREATING);

        //find the nearest harass target
        target.ShipPtr[0] = aiuFindHarassTarget(team->shipList.selection);

        if (target.ShipPtr[0] != NULL)
        {
            //find distance to target
            approx_team_pos    = team->shipList.selection->ShipPtr[0]->posinfo.position;
            target_pos         = target.ShipPtr[0]->posinfo.position;
            target_distance_sq = aiuFindDistanceSquared(approx_team_pos, target_pos);

            if (target_distance_sq > AIM_MIN_HARASS_ATTACK_DIST_SQ)
            {
                //find a standoff point to target
                target_standoff_pt = aiuFindRangeStandoffPoint(target_pos, approx_team_pos, AIM_HARASS_STANDOFF_DIST);

                //insert move move in front of this move and move the move pointer back one move
                // bonus points if you can tell me how many times I wrote "move" in that comment
                newMove = aimCreateMoveTeamNoAdd(team, target_standoff_pt, AIM_HARASS_MOVE_FORMATION, TRUE, TRUE);
                newMove->events = thisMove->events;
                aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

                return FALSE;
            }
            else
            {
                bitSet(team->teamFlags, TEAM_NEEDS_SUPPORT);
                protection = aiuFindNearbyDangerousEnemyShips(target.ShipPtr[0], AIM_HARASS_PROTECTION_RADIUS);

                //if the target is protected, attack the protection first, then the target
                if (protection->numShips > 0)
                {
                    if (protection->numShips > 1)
                    {
                        formation = AIM_HARASS_MULTITARGET_FORMATION;
                    }
                    else
                    {
                        formation = AIM_HARASS_SINGLETARGET_FORMATION;
                    }

                    newMove = aimCreateAdvancedAttackNoAdd(team, protection, formation,Aggressive,TRUE, TRUE);
                    newMove->events = thisMove->events;
                    if (aiuAttackFeatureEnabled(AIA_KAMIKAZE))
                    {
                        aieHandlerSetHealthLow(newMove, AIO_FIGHTER_KAMIKAZE_HEALTH, TRUE, FALSE, aihKamikazeHealthLowHandler);
                    }


                    if (protection->numShips == 1)
                    {
                        aieHandlerSetFiring(newMove, TRUE, aihHarassFiringSingleShipHandler);
                    }

                    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

//                    memFree(protection);
                    return FALSE;
                }

                memFree(protection);
                target.numShips = 1;
                //later change back to normal attack???
                newMove = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection(&target, "duppha", 0), AIM_HARASS_SINGLETARGET_FORMATION,Aggressive, TRUE, TRUE);
                newMove->events = thisMove->events;
                if (aiuAttackFeatureEnabled(AIA_KAMIKAZE))
                {
                    aieHandlerSetHealthLow(newMove, AIO_FIGHTER_KAMIKAZE_HEALTH, TRUE, FALSE, aihKamikazeHealthLowHandler);
                }

                aieHandlerSetFiring(newMove, TRUE, aihHarassFiringSingleShipHandler);
                thisMove->params.harass.target = target.ShipPtr[0];
                thisMove->processing = FALSE;
                aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
                aiplayerLog((aiIndex,"Harassing!  Boom! Boom! Bang! Bang!"));
            }
        }
        return FALSE;
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessDock
    Description : Moves the team to dock with the nearest dockable ship
    Inputs      : team - the team doing the docking
    Outputs     : Issues a dock command
    Return      : void
----------------------------------------------------------------------------*/
sdword aimProcessDock(AITeam *team)
{
    AITeamMove *thisMove = team->curMove/*, *newMove*/;
    sdword i = 0, numShips = team->shipList.selection->numShips;
    real32 maxHealth = 0, actualHealth = 0;
    ShipPtr ship;
    SelectCommand *teamselection;
    sdword dockMoveFlags = thisMove->params.dock.dockmoveFlags;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Dock Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        Ship *dockAt = NULL;
        DockType dockType;

        //determine if the ships' health is low
        for (i = 0; i < team->shipList.selection->numShips; i++)
        {
            maxHealth += team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth;
            actualHealth += team->shipList.selection->ShipPtr[i]->health;
        }

        //if the ships' health is low, dock for refuel and repair
        if (100 *actualHealth/maxHealth < AIM_DOCK_REPAIRHEALTH_TRIGGER)
        {
            dockType = DOCK_FOR_BOTHREFUELANDREPAIR;
        }
        else
        {
            dockType = DOCK_FOR_REFUEL;
        }

        if ((dockMoveFlags & dockmoveFlags_AtShip) && (thisMove->params.dock.dockAt))
        {
            dockType = DOCK_AT_SPECIFIC_SHIP;       // override all previous decisions
            dockAt = thisMove->params.dock.dockAt;
        }

        if (dockMoveFlags & dockmoveFlags_Stay)
        {
            dockType |= DOCK_STAY_TILL_EXPLICITLAUNCH;
        }

        if (dockMoveFlags & dockmoveFlags_Instantaneously)
        {
            dockType |= DOCK_INSTANTANEOUSLY;
        }

        aiuWrapDock(team->shipList.selection, dockType, dockAt);
        aiplayerLog((aiIndex,"Docking for dockType %d",dockType));

        thisMove->params.dock.shipsDocking = selectMemDupSelection(team->shipList.selection, "dockdup", 0);
        thisMove->processing = TRUE;
        bitSet(team->teamFlags, TEAM_DOCKING);
        return FALSE;
    }
    else
    {
        teamselection = team->shipList.selection;
        for (i = 0; i < numShips; i++)
        {
            ship = teamselection->ShipPtr[i];

            if ( (((ship->dockvars.dockship == NULL) && (!aiuShipIsHidden(ship)))) ||
                 ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) && ShipIsWaitingForSoftLaunch(ship)) )
            {
                clRemoveShipFromSelection(thisMove->params.dock.shipsDocking, ship);
            }
#if 0       // OLD LOGIC for checking when ships are done docking.  Kept for historical reference.
            //check to see if all the ships have been refueled.
            if ((ship->fuel/ship->staticinfo->maxfuel > AIM_DOCK_FINISH_FUEL_THRESHOLD) && (!aiuShipIsHidden(ship)))
            {
                clRemoveShipFromSelection(thisMove->params.dock.shipsDocking, ship);
            }
#endif
        }
        if (!thisMove->params.dock.shipsDocking->numShips)
        {
            bitClear(team->teamFlags, TEAM_DOCKING);
            return TRUE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessDefendMothership
    Description : The team looks for the nearest ship attacking the mothership
                  and attacks it
    Inputs      : team - the team doing the defending
    Outputs     : Dead enemy ships.  Lots of 'em
    Return      : TRUE if the all clear command has been given
----------------------------------------------------------------------------*/
sdword aimProcessDefendMothership(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    SelectCommand *target;

    //if all the ships in the team are gone, let the defense manager clean it up
    //(later)  I hope!  :-)
    if (team->shipList.selection->numShips == 0)
    {
//        aiplayerLog((aiIndex,"Defend Mothership Move, Zero size team"));
        return FALSE;
    }

    target = (SelectCommand *)memAlloc(sizeofSelectCommand(1), "defmoshiptarget", 0);
    target->ShipPtr[0] = aiuFindNearestMothershipAttacker(team->shipList.selection);

    if (target->ShipPtr[0] != NULL)
    {
            target->numShips = 1;
            newMove = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection(target, "duppdm", 0), AIM_DEF_MOTHERSHIP_ATTACK_FORMATION,Aggressive,TRUE, TRUE);
            aieHandlerSetFuelLow(newMove, AID_DEFEND_MOTHERSHIP_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

            memFree(target);
            return FALSE;
    }
    else
    {
        memFree(target);
        return TRUE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessPatrolMove
    Description : A series of moves is setup and executed.
    Inputs      : team - the team doing the patrol moving
    Outputs     : Creates a whole bunch of moveteam moves
    Return      : TRUE if the patrol move has completed
----------------------------------------------------------------------------*/
sdword aimProcessPatrolMove(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove, *tempMove;
    udword i, num_points = thisMove->params.patrolmove.path->numPoints,
              startIndex = thisMove->params.patrolmove.startIndex;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Patrol Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {

        if (thisMove->params.patrolmove.path->closed)
        {
            //in a closed loop path, create all the moves then point the curMove pointer to
            //the startindex

            //create the first move
            newMove = aimCreateMoveTeamIndexNoAdd(team, thisMove->params.patrolmove.path->point[0],0,
                                                  thisMove->formation,thisMove->tactics,TRUE, FALSE);
            thisMove->params.patrolmove.loopMove = newMove;
            newMove->events = thisMove->events;
            listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

            if (startIndex == 0)
            {
                tempMove = newMove;
            }

            //create all the rest of the moves
            for (i = 1; i < num_points; i++)
            {
                newMove = aimCreateMoveTeamIndexNoAdd(team, thisMove->params.patrolmove.path->point[i],i,
                                                      thisMove->formation,thisMove->tactics,TRUE,FALSE);
                newMove->events = thisMove->events;
                listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

                if (startIndex == i)
                {
                    tempMove = newMove;
                }
            }
        }
        else
        {
            //in open loop paths, ignore previous moves and just start with the
            //startindex move
            for (i = startIndex; i < num_points; i++)
            {
                newMove = aimCreateMoveTeamIndexNoAdd(team, thisMove->params.patrolmove.path->point[i],i,
                                                      thisMove->formation,thisMove->tactics,TRUE,TRUE);
                newMove->events = thisMove->events;
                listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

                if (startIndex == i)
                {
                    tempMove = newMove;
                }
            }
        }

        thisMove->processing = TRUE;
        team->curMove = tempMove;
        return FALSE;
    }
    else
    {
        if (thisMove->params.patrolmove.path->closed)
        {
            //go through the moveteam moves and set the processing flag to false
            tempMove = thisMove->params.patrolmove.loopMove;
            while (tempMove != thisMove)
            {
                tempMove->processing = FALSE;
                tempMove = (AITeamMove *)listGetStructOfNode(tempMove->listNode.next);
            }
            team->curMove = thisMove->params.patrolmove.loopMove;
            return FALSE;
        }
        return TRUE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessActivePatrol
    Description : Creates a new patrol path for the team and creates the moves to do them
    Inputs      : team - the team doing the active patrol
    Outputs     : Creates a bunch of moves
    Return      : TRUE if the move is done
----------------------------------------------------------------------------*/
sdword aimProcessActivePatrol(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    Path *patrolpath;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Active Patrol Move, Zero Sized Team"));
        return TRUE;
    }

    patrolpath = aiuGeneratePatrolPath(thisMove->params.activepatrol.patroltype);

    if (patrolpath->numPoints > 0)
    {
        newMove = aimCreatePatrolMoveNoAdd(team, patrolpath, 0,
                                           thisMove->formation, Aggressive, TRUE, TRUE);
        newMove->events = thisMove->events;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
    }
    else
    {
        memFree(patrolpath);
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessTempGuard
    Description : Moves the ships into a defensive position and guards against attacks.
                  This move is meant for attack ships that are sitting around waiting
                  for their attack order
    Inputs      : team - the team doing the temp guard
    Outputs     : Creates a whack of moves
    Return      : TRUE if the move is done
----------------------------------------------------------------------------*/
sdword aimProcessTempGuard(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    MaxSelection invadingShips, distressShips;
    vector origin = {0,0,0};
    real32 distance;

    if (team->shipList.selection->numShips == 0)
    {
        bitClear(team->teamFlags, TEAM_TempGuard);
        aiplayerLog((aiIndex,"Temp Guard Move, Zero Sized Team"));
        return TRUE;
    }

    if (!bitTest(team->teamFlags, TEAM_TempGuard))
    {
        bitSet(team->teamFlags, TEAM_TempGuard);
    }

    // if the attack order has been given
    if (aivarValueGet(aivarFind(aiCurrentAIPlayer->attackVarLabel)) <= -1)
    {
        aiplayerLog((aiIndex,"%x, TempGuard: attackVarLabel set to -1, leaving guard", team));
        bitClear(team->teamFlags, TEAM_TempGuard);
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if (aiCurrentAIPlayer->player->PlayerMothership)
        {
            distance = AIM_TEMP_GUARD_MOVE_LOCATION_MULT*(fsqrt(aiCurrentAIPlayer->sphereofinfluence));

/*            guardLocation = aiuFindRangeStandoffPoint(aiCurrentAIPlayer->player->PlayerMothership->posinfo.position,
                                                      origin, AIM_TEMP_GUARD_MOVE_LOCATION_MULT*(fsqrt(aiCurrentAIPlayer->sphereofinfluence)));

            newMove = aimCreateMoveTeamNoAdd(team, guardLocation, AIM_TEMP_GUARD_MOVE_FORMATION, FALSE, TRUE);
*/
            //  **** origin here... get rid of that...//
            newMove = aimCreateMoveToNoAdd(team, origin, distance,DISTANCE_LIMITED,AIM_TEMP_GUARD_MOVE_FORMATION, AIM_TEMP_GUARD_MOVE_TACTICS, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
        thisMove->processing = TRUE;
        return FALSE;
    }

    //this keeps on repeating!  do something about it!!!
    if (aiCurrentAIPlayer->aidInvadingShips)
    {
        selSelectionCopy((MaxAnySelection *)&invadingShips, (MaxAnySelection *)aiCurrentAIPlayer->aidInvadingShips);

        //later also consider corvette attacks
        MakeShipsOnlyCapital((SelectCommand *)&invadingShips);

        if (invadingShips.numShips)
        {
            newMove = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection((SelectCommand *)&invadingShips, "dupptg", 0), AIM_TEMP_GUARD_INVADER_FORMATION,Neutral, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
            thisMove->processing = FALSE;
            return FALSE;
        }
    }

    if (aiCurrentAIPlayer->aidDistressShips)
    {
        selSelectionCopy((MaxAnySelection *)&distressShips, (MaxAnySelection *)aiCurrentAIPlayer->aidDistressShips);

        if (aiuRescueShipType((SelectCommand *)&distressShips, team, AdvanceSupportFrigate))
        {
            return FALSE;
        }
        if (aiuRescueShipType((SelectCommand *)&distressShips, team, ResourceController))
        {
            return FALSE;
        }
    }

    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessReinforce
    Description : Moves the team close to the team it's supposed to reinforce,
                  gives the reinforcing ships the same order as the team,
                  and copies the team into the other
    Inputs      : team - the team doing the move
    Outputs     :
    Return      : TRUE if the move is completed
----------------------------------------------------------------------------*/
sdword aimProcessReinforce(AITeam *team)
{
    AITeam *reinforceTeam = team->curMove->params.reinforce.reinforceteam;
    AITeamMove *thisMove = team->curMove, *reinforceMove = reinforceTeam->curMove, *newMove;
    sdword i;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Dock Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        //first move the reinforcing team to the location of the team to be reinforced
        if ((reinforceTeam->shipList.selection->numShips) &&
            (aiuFindDistanceSquared(team->shipList.selection->ShipPtr[0]->posinfo.position,
                                   reinforceTeam->shipList.selection->ShipPtr[0]->posinfo.position) > AIM_INTERCEPT_FINISH_DISTSQ))
        {
            newMove = aimCreateInterceptNoAdd(team, reinforceTeam->shipList.selection->ShipPtr[0], 5.0,DELTA_FORMATION,Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
            return FALSE;
        }

        //then copy the reinforcing team ships to the team to be reinforced
        for (i=team->shipList.selection->numShips-1;i>=0;i--)
        {
            aitAddShip(reinforceTeam, team->shipList.selection->ShipPtr[i]);
            growSelectRemoveShipIndex(&team->shipList, i);
        }

        dbgAssert(team->shipList.selection->numShips == 0);

        if (reinforceMove)
        {
            reinforceMove->processing = FALSE;
            if (reinforceTeam->shipList.selection->numShips > 1)
            {
                if ((reinforceMove->formation != NO_FORMATION) &&
                    (reinforceMove->formation != SAME_FORMATION))
                {
                    aiuWrapFormation(reinforceTeam->shipList.selection, reinforceMove->formation);
                }
                else if ((reinforceTeam->kasFormation != NO_FORMATION) &&
                         (reinforceTeam->kasFormation != SAME_FORMATION))
                {
                    aiuWrapFormation(reinforceTeam->shipList.selection, reinforceTeam->kasFormation);
                }
            }
            aiplayerLog((aiIndex,"Reinforcing move #%i", reinforceMove->type));
        }

        //note: the reinforce move does not deal with special casees yet.
        //code below is for that purpose

/*        switch (reinforceMove->type)
        {
            case MOVE_DONE:
                //if the team to reinforce is done, this team is done as well...
                return TRUE;
            case MOVE_GUARDSHIPS:
                //add movetoship move
                //then guard target
            case MOVE_DEFMOSHIP:
                //add movetoship move
                //then recheck.  If close enough, add to team
            case MOVE_TEMPGUARD:
                //add movetoship move
                //then add to team
            case MOVE_GETSHIPS:
                //add ships to team (check if adding proper ships to get out of
                //  getships move?)
            case MOVE_FORMATION:
                //add moveto ship move
                //command should be different by the time the ships arrive
            case MOVE_MOVETEAM:
            case MOVE_PATROLMOVE:
            case MOVE_ACTIVEPATROL:
            case MOVE_COUNTSHIPS:
            case MOVE_ATTACK:
            case MOVE_ADVANCEDATTACK:
            case MOVE_FLANKATTACK:
            case MOVE_HARASSATTACK:
            case MOVE_FANCYGETSHIPS:
            case MOVE_DOCK:
            case MOVE_REINFORCE:
            case MOVE_VARSET:
            case MOVE_VARINC:
            case MOVE_VARDEC:
            case MOVE_VARWAIT:
            case MOVE_VARDESTROY:
            case MOVE_GUARDCOOPTEAM:
            default:
                break;
        }*/
    }
    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessSupport
    Description : Moves the support ship into the vicinity of the ships it is supporting
    Inputs      : team - the supporting team
    Outputs     : May create some new moves if needed
    Return      : TRUE if the move is completed
----------------------------------------------------------------------------*/
sdword aimProcessSupport(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    SelectCommand *ships = thisMove->params.support.ships;
    vector support_location;
    real32 dist;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Support Move, Zero Sized Team"));
        return TRUE;
    }

    if ((!ships) || (!ships->numShips))
    {
        //move back to the mothership
        dist = aiuFindDistanceSquared(team->shipList.selection->ShipPtr[0]->posinfo.position,
                                      aiuPlayerMothershipCoords(aiCurrentAIPlayer->player));
        if (dist > 100000000)
        {
            newMove = aimCreateMoveToNoAdd(team, aiuPlayerMothershipCoords(aiCurrentAIPlayer->player),
                                           8.0, TIME_LIMITED, SAME_FORMATION, Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
        return FALSE;
    }

    support_location = aiuFindSafestStandoffPoint(ships, AIM_SUPPORT_SAFE_DISTANCE);

    if (!vecIsZero(support_location))
    {
        //support location becomes move location
        newMove = aimCreateMoveToNoAdd(team, support_location, 8.0, TIME_LIMITED, SAME_FORMATION, Evasive, TRUE, TRUE);
        newMove->events = thisMove->events;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
    }
    else
    {
        aiplayerLog((aiIndex,"AI: Random Support Standoff Point"));

        //generate a random standoff point and store the angle (?) to keep consistency
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessActiveRecon
    Description : Moves the team to the next nearest blob, counts ships, repeat
    Inputs      : team - the team doing the recon mission
    Outputs     : Creates a few new moves if needed
    Return      : TRUE if the move is done
----------------------------------------------------------------------------*/
sdword aimProcessActiveRecon(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    vector destination;
    udword chance_of_change_enemyrecon, num_enemy_blobs, tactics_chance;
    TacticsType tactic;
    ShipPtr playerMothership = aiCurrentAIPlayer->player->PlayerMothership;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Active Recon Move, Zero Sized Team"));
        return FALSE;
    }

    if (team->shipList.selection->ShipPtr[0]->shiptype == Probe)
    {
        //if a probe is farther than 5km from the mothership,
        //it is considered launched and can't move again.
        if (playerMothership && (aiuFindDistanceSquared(team->shipList.selection->ShipPtr[0]->posinfo.position, playerMothership->posinfo.position) > 25000000))
        {
            return TRUE;
        }
    }

    if (!thisMove->processing)
    {
         tactics_chance = (udword)randyrandom(RAN_AIPlayer, 100);

        if (tactics_chance < 2)
        {
            tactic = Aggressive;
        }
        else if (tactics_chance < 9)
        {
            tactic = Neutral;
        }
        else
        {
            tactic = Evasive;
        }

        newMove = aimCreateMoveToNoAdd(team, aiuFindEnemyMothershipCoords(aiCurrentAIPlayer->player),
                                       5.0, TIME_LIMITED, SAME_FORMATION, tactic, TRUE, TRUE);
        newMove->events = thisMove->events;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        thisMove->processing = TRUE;
        return FALSE;
    }

    if (playerMothership)
    {
        if (thisMove->params.activerecon.outwards)
        {
            if (thisMove->params.activerecon.enemyrecon)
            {
                destination = aiuFindStandoffOfNextNearestEnemyBlobOut(playerMothership, team->shipList.selection->ShipPtr[0]);
            }
            else
            {
                destination = aiuFindStandoffOfNextNearestBlobOut(playerMothership, team->shipList.selection->ShipPtr[0]);
            }

            if (vecIsZero(destination))
            {
                thisMove->params.activerecon.outwards = FALSE;
                return FALSE;
            }

            newMove = aimCreateMoveTeamNoAdd(team, destination, SAME_FORMATION, /*Evasive,*/ TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
        else
        {
            if (thisMove->params.activerecon.enemyrecon)
            {
                destination = aiuFindStandoffOfNextNearestEnemyBlobIn(playerMothership, team->shipList.selection->ShipPtr[0]);
            }
            else
            {
                destination = aiuFindStandoffOfNextNearestBlobIn(playerMothership,team->shipList.selection->ShipPtr[0]);
            }

            if (vecIsZero(destination))
            {
                thisMove->params.activerecon.outwards = TRUE;
                return FALSE;
            }

            newMove = aimCreateMoveTeamNoAdd(team, destination, SAME_FORMATION, /*Evasive,*/ TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }

        num_enemy_blobs = aiuGetNumEnemyBlobs();

        if (thisMove->params.activerecon.enemyrecon)
        {
            chance_of_change_enemyrecon = 100 - 20*num_enemy_blobs;
            thisMove->params.activerecon.enemyrecon = !(randyrandombetween(RAN_AIPlayer, 0, 100) > chance_of_change_enemyrecon);
        }
        else
        {
            chance_of_change_enemyrecon = 15*num_enemy_blobs;
            thisMove->params.activerecon.enemyrecon = randyrandombetween(RAN_AIPlayer, 0, 100) > chance_of_change_enemyrecon;
        }



//        newMove = aimCreateCountShipsNoAdd(team, TRUE, TRUE);
//        newMove->events = thisMove->events;
//        listAddNodeAfter(&(team->curMove->listNode), &(newMove->listNode), newMove);

        return FALSE;
    }
    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessShipRecon
    Description : Makes the team intercept and peek at a selection of ships
    Inputs      : team - the team doing the peeking
    Outputs     : Creates a few moves - mainly intercept and KILL!!!! Oh... sorry... only intercept...
    Return      : TRUE if the move is done
----------------------------------------------------------------------------*/
sdword aimProcessShipRecon(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    ShipPtr target;
    SelectCommand *teamships = team->shipList.selection,
                  *moveships = thisMove->params.shiprecon.ships,
                  *nearbyships;
    udword i;

    if (!teamships->numShips)
    {
        aiplayerLog((aiIndex,"Ship Recon Move, Zero Sized Team"));
        return TRUE;
    }

    //if the ships have died, return TRUE
    if (!moveships->numShips)
    {
        return TRUE;
    }

    //first check to see if any of the ships are nearby
    nearbyships = selectSelectionIntersection(teamships->ShipPtr[0]->collMyBlob->blobShips, moveships);
    if (nearbyships)
    {
        for (i=0;i<nearbyships->numShips;i++)
        {
            clRemoveShipFromSelection(moveships, nearbyships->ShipPtr[i]);
        }
        selectMergeTwoSelections(thisMove->params.shiprecon.foundships, nearbyships, DEALLOC2);
        thisMove->processing = FALSE;
    }

    //if the ships have been found, return TRUE
    if (!moveships->numShips)
    {
        return TRUE;
    }

    if (!thisMove->processing)
    {
        target = aiuGetClosestShip(moveships, teamships->ShipPtr[0]);

        newMove = aimCreateInterceptNoAdd(team, target, 5, NO_FORMATION, Evasive, TRUE, TRUE);
        newMove->events = thisMove->events;
        listAddNodeAfter(&(team->curMove->listNode), &(newMove->listNode), newMove);

        thisMove->processing = TRUE;

        return FALSE;
    }

    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessArmada
    Description : Moves those cool armada ships into the thick of things and kills lots of stuff
    Inputs      : team - the team doing the armada-ing
    Outputs     : Lots of shooting and stuff - creates attack moves and move moves
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessArmada(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    MaxSelection hyperspace;
//    vector dest_target;
//    SelectCommand *sel_target;
//    bool visibility;

    //choose good target
    //if unknown, send recon mission to check out
    //  wait for recon to finish
    //if known, choose attack type (flank, straight, takeout)
    //  attack!
                    //we want to outflank the mothership fleet.... maybe have a fleet analysis or something... hmmm...
                    //maybe add a small fleet analysis in Flank attack?
                    //find weak side...

    //later - add coordination with recon/support/guard/cooperating team

    if (!team->shipList.selection->numShips)
    {
        //all ships are dead.  R.I.P.
        aiplayerLog((aiIndex,"Armada Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if (!bitTest(team->teamFlags, TEAM_ARMADA))
        {
            bitSet(team->teamFlags, TEAM_ARMADA);
        }

        if (aiCurrentAIPlayer->aiaArmada.targets)
        {
            selSelectionCopy((MaxAnySelection *)&hyperspace, (MaxAnySelection *)team->shipList.selection);
            makeSelectionHyperspaceCapable((SelectCommand *)&hyperspace);

            // hyperspace if we can
            if (bitTest(team->teamFlags, TEAM_Hyperspaceable) &&
                (hyperspace.numShips) &&
                (aiCurrentAIPlayer->aifHyperSavings > 0))
            {
                aiplayerLog((aiIndex, "%x, Armada: Creating Hyperspace Flank Attack", team));
                newMove = aimCreateFlankAttackNoAdd(team, selectMemDupSelection(aiCurrentAIPlayer->aiaArmada.targets, "duppfah", 0), TRUE, /*SAME_FORMATION, */TRUE, TRUE);
            }
            else
            {
                aiplayerLog((aiIndex, "%x, Armada: Creating Normal Flank Attack", team));
                newMove = aimCreateFlankAttackNoAdd(team, selectMemDupSelection(aiCurrentAIPlayer->aiaArmada.targets, "duppfan", 0), FALSE, /*SAME_FORMATION, */TRUE, TRUE);
            }

            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
            thisMove->processing = TRUE;
        }
    }
    else
    {
         //later must flesh this one out!!!
        if (aiCurrentAIPlayer->aiaArmada.targets)
        {
            aiplayerLog((aiIndex, "%x, Armada: Still targets %i left, redoing processing", team, aiCurrentAIPlayer->aiaArmada.targets->numShips));
            thisMove->processing = FALSE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessControlResources
    Description : Moves the resource controller (or team... which will
                  most likely only have a resource controller) to optimize resourcer
                  docking
    Inputs      : team - the team that's controlling resourcers
    Outputs     : Creates a few moves
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessControlResources(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    vector destination;
    real32 dist;

    if (!team->shipList.selection->numShips)
    {
        aiplayerLog((aiIndex, "Control Resources Move, Zero ships"));
        return TRUE;
    }

    if (thisMove->params.rescontrol.ships)
    {
//        thisMove->params.rescontrol.ships = aiuFindResourceCollectors();
        destination = selCentrePointComputeGeneral((MaxSelection *)thisMove->params.rescontrol.ships, &dist);
    }
    else
    {
        destination = aiuFindResourceControllerDestination();
    }

    dist = aiuFindDistanceSquared(destination, team->shipList.selection->ShipPtr[0]->posinfo.position);

    if (dist > 100000000) //10 km
    {
        newMove = aimCreateMoveTeamNoAdd(team, destination, NO_FORMATION, TRUE, TRUE);
        newMove->events = thisMove->events;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessSwarmAttack
    Description : Controls the attack portion of the P2 swarm attack
    Inputs      : team - the team that's swarming
    Outputs     : Moves ships around
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessSwarmAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *targets = thisMove->params.swarmatt.targets;
    SelectCommand *teamShips = team->shipList.selection;
    udword i, fuellow;

    for (i=0;i<teamShips->numShips;)
    {
        ShipPtr ship;

        if (aitMsgReceived(team, "DockNow"))
        {
            if (team->msgSender)
            if (team->msgSender->shipList.selection->numShips)
            {
                aitDeleteAllTeamMoves(team);
                team->curMove = aimCreateDock(team, dockmoveFlags_Stay|dockmoveFlags_AtShip, team->msgSender->shipList.selection->ShipPtr[0], TRUE, TRUE);
                return FALSE;
            }
        }

        //if we're already fighting,
        if (targets)
        {
            //lower chance of refuelling
            fuellow = randyrandombetween(RAN_AIPlayer, 14, 18);
        }
        else
        {
            //high chance of refuelling
            fuellow = randyrandombetween(RAN_AIPlayer, 18, 25);
        }
        if (!aiuFuelAbove(teamShips->ShipPtr[i], fuellow) || !aiuHealthAbove(teamShips->ShipPtr[i], 50))
        {
            ship = teamShips->ShipPtr[i];

            //put ship into defense team
            aitAddShip(team->cooperatingTeam, teamShips->ShipPtr[i]);
            //add the ship to the cooperating team's newSwarmers structure
            growSelectAddShip(&team->cooperatingTeam->curMove->params.swarmdef.newSwarmers, teamShips->ShipPtr[i]);
            //remove the ship from the current team
            growSelectRemoveShipIndex(&team->shipList, i);
            growSelectRemoveShip(&thisMove->params.swarmatt.newSwarmers, ship);
            //aiplayerLog((1, "Attack %x: Ship moved from offense to defense", team));

//            aitCheckShips(team->cooperatingTeam, ship);
        }
        else
        {
            i++;
        }
    }

    if (!teamShips->numShips)
    {
        //we're either waiting for ships, or all ships are dead
        return FALSE;
    }

    if (targets && targets->numShips &&
        aiuShipsCloseToEnemyMothership(aiCurrentAIPlayer->player, targets, 4400))
    {
        aiumemFree(targets);
    }

    //if targets are dead, get the next targets
    if (!targets || !targets->numShips)
    {
        targets = aiuFindSwarmerTarget(team);

        if (targets)
        {
            //if there are globally assigned targets
            if (bitTest(team->teamFlags, TEAM_SwarmTarget))
            {
                //copy them locally, delete them and clear the swarmtargets flag
                thisMove->params.swarmatt.othertargets = selectMemDupSelection(aiCurrentAIPlayer->Targets, "othertarg", 0);
                aiumemFree(aiCurrentAIPlayer->Targets);
                bitClear(team->teamFlags, TEAM_SwarmTarget);
            }
        }
        else if (singlePlayerGameInfo.currentMission == 8)
        {
            //uses normal code to find swarm targets in mission 8
            targets = aiuFindSwarmTargets(teamShips, randyrandombetween(RAN_AIPlayer, 1, 3), FALSE);
        }
        else
        {
            //uses harass code to find target in mission 7
            targets = aiuFindSwarmTargets(teamShips, 1, TRUE);
        }


        if (targets && targets->numShips)
        {
            //set the target type
//            thisMove->params.swarmatt.targettype = targets->ShipPtr[0]->shiptype;

            //give attack command to all swarmers
            if ((isShipOfClass(targets->ShipPtr[0], CLASS_Corvette)) ||
                (isShipOfClass(targets->ShipPtr[0], CLASS_Fighter)))
            {
                aiuSplitAttack(teamShips, targets);
            }
            else
            {
                aiuWrapAttack(teamShips, targets);
            }
            thisMove->params.swarmatt.targets = targets;
        }
        else
        {
            //temporary... maybe move the ships back to the guards
            thisMove->params.swarmatt.targets = NULL;

        }
    }
    else if (thisMove->params.swarmatt.newSwarmers.selection->numShips)
    {
        //give attack move to new swarmers
        //old swarmers continue what they're doing
        aiuWrapAttack(thisMove->params.swarmatt.newSwarmers.selection, targets);
    }
    thisMove->params.swarmatt.newSwarmers.selection->numShips = 0;

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessSwarmDefense
    Description : Controls the defense portion of the swarm attack
    Inputs      : team - the team that's defense swarming
    Outputs     : Moves ships around
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessSwarmDefense(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *teamShips = team->shipList.selection;
    MaxSelection dockships, guardships;
    paramsSwarmDefense *params = &thisMove->params.swarmdef;
    AITeam *newPodTeam;
    ShipPtr ship;
    udword i, fuellow;

    if (!teamShips->numShips)
    {
        //we're either waiting for ships, or all ships are dead
        return FALSE;
    }

    if (aitMsgReceived(team, "DockNow"))
    {
        if (team->msgSender)
        if (team->msgSender->shipList.selection->numShips)
        {
            aitDeleteAllTeamMoves(team);
            team->curMove = aimCreateDock(team, dockmoveFlags_Stay|dockmoveFlags_AtShip, team->msgSender->shipList.selection->ShipPtr[0], TRUE, TRUE);
            return FALSE;
        }
    }

    if (!params->Pods->numShips)
    {
        //pod has been destroyed, find another
        newPodTeam = aitFindNewPod(team);

        if (newPodTeam)
        {
              selSelectionCopy((MaxAnySelection *)params->Pods, (MaxAnySelection *)newPodTeam->shipList.selection);
//            aitMoveSwarmersToNewPod(team, newPodTeam);
//            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    dockships.numShips  = 0;
    guardships.numShips = 0;

    for (i=0; i<teamShips->numShips;i++)
    {
        ship = teamShips->ShipPtr[i];

        fuellow = randyrandombetween(RAN_AIPlayer, 45, 65);

        //if the swarmer isn't already docking and it's fuel is below fuellow
        //                                          or it's health is below 50
        //                                          or the defense swarm is in full refuel mode
        //                                              and the swarmer's fuel is below 85
        if ((!ship->dockvars.dockship) &&
            ((!aiuFuelAbove(ship, fuellow) ||
              !aiuHealthAbove(teamShips->ShipPtr[i], 50)  ||
              (params->full_refuel && !aiuFuelAbove(ship, 85)))))
        {
            //add the swarmer to a list of to be dockers
            selSelectionAddSingleShip(&dockships, ship);

            //if the ship was guarding, remove it from the guarding list
            if (params->guarding) clRemoveShipFromSelection(params->guarding, ship);
        }
        //else if the swarmer isn't already guarding the pod
        else if ((!ship->dockvars.dockship) && ((!params->guarding) || (!ShipInSelection(params->guarding, ship))))
        {
            selSelectionAddSingleShip(&guardships, ship);
        }
    }

    if (dockships.numShips)
    {
        if (dockships.numShips > 7)
        {
            tutGameMessage("Swarm_BigDock");
        }
        aiuSwarmDock((SelectCommand *)&dockships, params->Pods);
    }
    if (guardships.numShips)
    {
        params->guarding = selectMergeTwoSelections(params->guarding, (SelectCommand *)&guardships, DEALLOC1);
        aiuWrapProtect((SelectCommand *)&guardships, params->Pods);
    }

    // if there are guarding ships and
    //if swarmers are on full attack or new ships have arrived or the attacking team has targets
    if (params->guarding &&
        ((params->full_attack) ||
         (params->newSwarmers.selection->numShips) ||
         (team->cooperatingTeam->curMove->params.swarmatt.targets) ||
         (team->cooperatingTeam->curMove->params.swarmatt.othertargets)))
    {
        //give attackers all the guarding ships
        for (i=0;/*(i<params->newSwarmers.selection->numShips)&&*/(params->guarding->numShips);i++)
        {
            aitMoveSwarmShipDefenseToAttack(team->cooperatingTeam, team, params->guarding->ShipPtr[0]);
        }
        params->newSwarmers.selection->numShips = 0;
    }

    if (params->guarding && ((params->full_attack)||
        ((team->cooperatingTeam->curMove->params.swarmatt.targets)&&(!team->cooperatingTeam->shipList.selection->numShips))))
    {
        //just put the remaining guarding ships into attack team
        for (i=0;i<params->guarding->numShips;)
        {
            aitMoveSwarmShipDefenseToAttack(team->cooperatingTeam, team, params->guarding->ShipPtr[0]);
        }
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessSwarmPod
    Description : Moves the P2 Pod around and acts as a control center for the
                  other swarmer teams
    Inputs      : team - the pod team
    Outputs     : Moving and silly things like that
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessSwarmPod(AITeam *team)
{
    AITeam *attackSwarm = team->cooperatingTeam, *defenseSwarm = team->cooperatingTeam->cooperatingTeam;
//    paramsSwarmDefense *defParams = &defenseSwarm->curMove->params.swarmdef;
    AITeamMove *thisMove = team->curMove, *newMove;
    SelectCommand /**enemyShips,*/ *targets;
//    udword numShips, i;
    vector current_location, target_location, destination; //, zerovec = {0,0,0};
    real32 avg_size, dist;

    aiuFindArmadaTarget(&target_location, &targets, team->shipList.selection);
    current_location = selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &avg_size);

    if (team->shipList.selection->numShips)
    {
        if (aitMsgReceived(team, "P2MothershipBlowin"))
        {
            MaxSelection filtered;

            filtered.numShips = 0;
            aiuFilterDisabledShips(team->shipList.selection, &filtered);
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&filtered);
            spHyperspaceSelectionOut((SelectCommand *)&filtered);
            spHyperspaceDelay = 0.0f;
            return TRUE;
        }

        // mothership assault code
/*        if (singlePlayerGameInfo.currentMission == 8)
        {
            if (!thisMove->processing)
            {
                //first move - move the ship towards the armada (mothership or powerbase)

                //find the flanking coordinates
                destination = aiuGenerateFlankCoordinates(target_location, current_location, zerovec, 10000);

                //create a move to move to those coordinates (with a wait)
                newMove = aimCreateMoveTeamNoAdd(team, destination, NO_FORMATION, FALSE, TRUE);
                aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

                thisMove->processing = TRUE;
            }
            else if (!vecIsZero(team->shipList.selection->ShipPtr[0]->moveTo))
            {
                //the pod is moving

                //when near the armada, stop the pod and refuel all the swarmers
                if (aiuFindDistanceSquared(target_location, current_location) < 225000000)
                {
                    aiuWrapHalt(team->shipList.selection);

                    defParams->full_refuel = TRUE;
                    thisMove->params.swarmpod.first_attack = TRUE;
                }
            }
            else if (thisMove->params.swarmpod.first_attack)
            {
                if (defParams->guarding)
                {
                    numShips = defParams->guarding->numShips;
                }
                else
                {
                    numShips = 0;
                }

                //if all swarmers have fueled up
                if (numShips == defenseSwarm->shipList.selection->numShips)
                {
                    defParams->full_refuel = FALSE;

                    //if all other defense swarmers are ready to attack
                    if (aitAllDefenseSwarmersFull())
                    {
                        thisMove->params.swarmpod.first_attack    = FALSE;
                        defParams->full_attack = TRUE;

                        //move the swarmers to the attack team
                        for (i=0; i<numShips; i++)
                        {
                            aitMoveSwarmShipDefenseToAttack(attackSwarm, defenseSwarm, defParams->guarding->ShipPtr[0]);
                        }

                        //move the attack swarmers into the battle area.  Boom boom bang bang!
                        if (attackSwarm->shipList.selection->numShips)
                        {
                            newMove = aimCreateMoveTeamNoAdd(attackSwarm, target_location, NO_FORMATION, FALSE, TRUE);
                            aitAddmoveBeforeAndMakeCurrent(attackSwarm, newMove, attackSwarm->curMove);
                        }
                    }
                }
                //the enemy is too close for comfort
                else if (aiuFindDistanceSquared(target_location, current_location) < 80000000)
                {
                    defParams->full_refuel = FALSE;
                    thisMove->params.swarmpod.first_attack = FALSE;
                    defParams->full_attack = TRUE;

                    aitMoveAllSwarmShipsDefenseToAttack(defenseSwarm, attackSwarm);

                    newMove = aimCreateMoveTeamNoAdd(attackSwarm, target_location, NO_FORMATION, FALSE, TRUE);
                    aitAddmoveBeforeAndMakeCurrent(attackSwarm, newMove, attackSwarm->curMove);
                }
            }
        }
        else
        {*/

        // we treat the pod as a simple support ship
        if (!thisMove->processing)
        {
            aitMoveAllSwarmShipsDefenseToAttack(defenseSwarm, attackSwarm);
            thisMove->processing = TRUE;
        }
        destination = selCentrePointComputeGeneral((MaxSelection *)attackSwarm->shipList.selection, &avg_size);
        dist = aiuFindDistanceSquared(destination, team->shipList.selection->ShipPtr[0]->posinfo.position);

        if (dist > 225000000)
        {
            newMove = aimCreateMoveToNoAdd(team, destination, 5000.0, DISTANCE_LIMITED, NO_FORMATION, Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
//            if ((aiCurrentAIPlayer->aiplayerFrameCount & 31) == 0)
//            {
//                thisMove->processing = FALSE;
//            }
/*        }*/



/*        enemyShips = aiuFindNearbyEnemyShips(team->shipList.selection->ShipPtr[0], 8000);

        if (enemyShips->numShips)
        {
            if (!attackSwarm->shipList.selection->numShips && defenseSwarm->shipList.selection->numShips)
            {
                sdword numGuardingShips;
                if (defParams->guarding)
                {
                    numGuardingShips = defParams->guarding->numShips;
                }
                else
                {
                    numGuardingShips = 0;
                }

                //temporary... later do something more secure
                //obviously numShips will always be equal to numships
                if ((numGuardingShips > 0) && (numShips <= numGuardingShips))
                {
                    for (i=0; i<numShips; i++)
                    {
                        aitMoveSwarmShipDefenseToAttack(attackSwarm, defenseSwarm, defParams->guarding->ShipPtr[0]);
                    }
                }
            }
        }
        //if no enemies around at all, reset to the beginning
        else if ((attackSwarm->shipList.selection->numShips) &&
                 (attackSwarm->curMove->type == MOVE_SWARMATTACK) &&
                 (!attackSwarm->curMove->params.swarmatt.targets))
        {
            ++thisMove->params.swarmpod.attack_delay;
            if (thisMove->params.swarmpod.attack_delay > 5)
            {
                aitMoveAllSwarmShipsAttackToDefense(attackSwarm, defenseSwarm);
                defParams->full_attack = FALSE;
                thisMove->params.swarmpod.attack_delay = 0;
                thisMove->processing = FALSE;
            }
        }*/
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessResourceVolume
    Description : Moves the resource collectors around to optimize resource
                  collection within a volume
    Inputs      : team - the resource collecting team
    Outputs     : Moves ships around in a most productive way
    Return      : TRUE if the move is complete (the volume is completely resourced)
----------------------------------------------------------------------------*/
sdword aimProcessResourceVolume(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    Resource *resource, *biggestResource;
    SelectCommand selone;
    ShipPtr ship;
    udword i;

    if (team->shipList.selection->numShips == 0)
    {
        return TRUE;
    }

    selone.numShips = 1;

    if ((!thisMove->params.resvolume.volResources) &&
        (!thisMove->params.resvolume.takenResources))
//        !thisMove->processing)
    {
        thisMove->params.resvolume.volResources = aiuFindResourcesInVolume(thisMove->params.resvolume.volume);
        thisMove->params.resvolume.takenResources = (ResourceSelection *)memAlloc(sizeofSelectCommand(10), "takeRes", 0);
        thisMove->params.resvolume.takenResources->numResources = 0;
        aiplayerLog((aiIndex, "Resetting volResources and takenResources"));
//        thisMove->processing = TRUE;
    }

    if ((thisMove->params.resvolume.volResources) && (thisMove->params.resvolume.volResources->numResources))
    {
        for (i=0; i < team->shipList.selection->numShips;i++)
        {
            ship = team->shipList.selection->ShipPtr[i];

            if (!aiuShipIsResourcingOrSomething(ship))
            {
                //find the best fit asteroid in the volume
                resource = aiuFindBestResource(&biggestResource, ship, thisMove->params.resvolume.volResources);

                if (resource)
                {
                    //send the resourcer to resource it
                    selone.ShipPtr[0] = ship;
                    aiuWrapCollectResource(&selone, resource);
                    clRemoveTargetFromSelection((SelectAnyCommand *)thisMove->params.resvolume.volResources, (TargetPtr)resource);

                    //maybe this isn't the best way to do it... what if the resource collector gets killed?
                    selSelectionAddSingleShip((MaxSelection *)thisMove->params.resvolume.takenResources, (ShipPtr)resource);
                    bitSet(ship->specialFlags, SPECIAL_Resourcing);
//                    aiplayerLog((aiIndex, "Resourcer %i harvesting best fit", i));
                }
                else
                {
                                                                       //make tweakable
                    if (100*ship->resources/ship->staticinfo->maxresources > 90)
                    {
                        selone.ShipPtr[0] = ship;
                        aiuWrapDock(&selone, DOCK_TO_DEPOSIT_RESOURCES, NULL);
//                        aiplayerLog((aiIndex, "Resourcer %i docking", i));
                    }
                    else if (biggestResource)
                    {
                        selone.ShipPtr[0] = ship;
                        aiuWrapCollectResource(&selone, biggestResource);
                        bitSet(ship->specialFlags, SPECIAL_Resourcing);
//                        aiplayerLog((aiIndex, "Resourcer %i harvesting biggest", i));
                    }
                }

                //if no best fit is found and resourcer has > 0 resources, dock!
                //later, when resource ships have cleaned out the volume, we'll
                //want to avoid having the ships go dock - so only do that when they're
                //reasonably full
            }
            else if (bitTest(ship->specialFlags, SPECIAL_Finished_Resource))
            {
                if (100*ship->resources/ship->staticinfo->maxresources > 90)
                {
                    selone.ShipPtr[0] = ship;
                    aiuWrapDock(&selone, DOCK_TO_DEPOSIT_RESOURCES, NULL);
//                    aiplayerLog((aiIndex, "Resourcer %i docking", i));
                }
                bitClear(ship->specialFlags, SPECIAL_Finished_Resource);
            }
        }
    }
    else
    {
//        aiplayerLog((aiIndex, "Finished Resourcing Blob"));
        return TRUE;
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessActiveResource
    Description : Activly looks for the best resource pocket and mines the
                  hell out of it
    Inputs      : team - the team doing the resourcing
    Outputs     : Creates some new moves every now and then
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessActiveResource(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    Volume volume;
    blob *resblob;

    if (!team->shipList.selection->numShips)
    {
        return TRUE;
    }

    resblob = aiuFindNearestResourceBlob(team->shipList.selection);

    if (!resblob)
    {
        //***if resources regenerate, return FALSE and just wait...
        //assume that resources regenerate, just wait.
        return FALSE;
    }

    volume.type = VOLUME_SPHERE;
    volume.attribs.sphere.center = resblob->centre;
    volume.attribs.sphere.radius = resblob->radius;

    newMove = aimCreateResourceVolumeNoAdd(team, volume, FALSE, TRUE, TRUE);
    newMove->events = thisMove->events;
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessMothershipMove
    Description : Moves tha mothership around
    Inputs      : team - the mothership team
    Outputs     : Creates a timed move
    Return      : TRUE if the move is finished
----------------------------------------------------------------------------*/
sdword aimProcessMothershipMove(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    vector destination;
    real32 dist, time;

    if (!aitNumTeamShips(team))
    {
        aiplayerLog((aiIndex, "Closing Mothership Move"));
        return TRUE;
    }

    if (!aiCurrentAIPlayer->airNumRControllers)
    {
        destination = aiuFindResourceControllerDestination();

        dist = aiuFindDistanceSquared(destination, aitApproxTeamPos(team));

        if ((((aitTeamShipTypeIs(Mothership, team)) && (dist > 200000000)) ||  //sqrt(20) km
             ((aitTeamShipTypeIs(Carrier, team)) && (dist > 50000000))) &&     //sqrt(10) km
            (aiuDestinationNotNearOtherMothership(destination, 800000000)))

        {
            time = frandyrandombetween(RAN_AIPlayer, 20.0, 35.0);
            newMove = aimCreateMoveToNoAdd(team, destination, time, TIME_LIMITED, NO_FORMATION, Neutral, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
    }
    else if (!thisMove->processing)
    {
        destination = aiuFindResourceControllerDestination();

        dist = aiuFindDistanceSquared(destination, aitApproxTeamPos(team));

        if ((((aitTeamShipTypeIs(Mothership, team)) && (dist > 400000000)) ||  //20 km
             ((aitTeamShipTypeIs(Carrier, team)) && (dist > 100000000))) &&     //10 km
            (aiuDestinationNotNearOtherMothership(destination, 1600000000)))
        {
            newMove = aimCreateMoveToNoAdd(team, destination, 40, TIME_LIMITED, NO_FORMATION, Neutral, TRUE, TRUE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
//        thisMove->processing = TRUE;
    }

    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessCapture
    Description : Captures a ship and brings it back to the mothership
    Inputs      : team - the team doing the capturing
    Outputs     : Moves some ships around and stuff...
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessCapture(AITeam *team)
{
    //basically just latch onto the target.
    //The capture function takes care of the rest.
    //this move will probably not even be needed....
    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessActiveCapture
    Description : Roves the salvage capture corvette around until a nice juicy
                  target is found, then creates a capture move for that target
    Inputs      : team - the team doing the capturing
    Outputs     : Moves some ships around, then creates a new move
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessActiveCapture(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *target;
    SelectCommand onesel;

    if (!team->shipList.selection->numShips)
    {
        return TRUE;
    }

    if (!thisMove->processing)
    {
        //note: later need to add visibility checking for this one...
        target = aiuFindCaptureTarget(team->shipList.selection);
        //now check what ships are protecting it

        onesel.numShips = 1;

        if (target->ShipPtr[0])
        {
            onesel.ShipPtr[0] = target->ShipPtr[0];
        }
        else if (target->ShipPtr[1])
        {
            onesel.ShipPtr[0] = target->ShipPtr[1];
        }
        else if (target->ShipPtr[2])
        {
            onesel.ShipPtr[0] = target->ShipPtr[2];
        }
        else
        {
            //don't bother doing anything
            return FALSE;
        }

        aiuWrapSpecial(team->shipList.selection, &onesel);

        thisMove->processing = TRUE;

        //later move towards ships...  use support moveto code...
        //maybe redo aiuFindCaptureTarget
        //capture best ships...

        //may also need
    }
    else if (!(((udword)universe.totaltimeelapsed) & 15))
    {
        if (((SalCapCorvetteSpec *)(team->shipList.selection->ShipPtr[0]->ShipSpecifics))->salvageState < 3) //dockingstate1
        {
            thisMove->processing = FALSE;
        }
    }

    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aimProcessActiveMine
    Description : Da minin' move - finds a nice juicy area to put a minefield in, then puts it in
    Inputs      : team - the minin' team
    Outputs     : Creates a few new moves
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessActiveMine(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
//    Path *destinations;
//    vector vec;
//    SelectCommand *ships;
    Volume *volume = NULL;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex, "Active Mine Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        //choose type of minelaying - mothership protect, resource patch, enemy path
        //find the minelaying locations
        //setup the minevolume thingy
        //do this a few times, then return TRUE

        volume = aiuFindBestVolumeToMine(team->shipList.selection);

        if (!volume)
        {
            //if the function couldn't find anything, then mine the center
            volume = (Volume *)memAlloc(sizeof(Volume), "voltomine", 0);

            volume->type = VOLUME_SPHERE;
            vecSet(volume->attribs.sphere.center, 0,0,0);
            volume->attribs.sphere.radius = 6000;
        }

        newMove = aimCreateMineVolumeNoAdd(team, *volume, TRUE, TRUE);
        newMove->events = thisMove->events;
        aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

        aiumemFree(volume);

        thisMove->processing = TRUE;
        return FALSE;
    }



    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessMineVolume
    Description : Moves the minelayer corvettes into position to mine a volume and then makes them mine
    Inputs      : team
    Outputs     : Creates a few new moves
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessMineVolume(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove, *tempMove;
    Path *minepoints;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex, "Mine Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if (thisMove->params.minevolume.volume.type == VOLUME_SPHERE)
        {
            //add two moves - one with the circle slightly wider than the next, so that all the
            //minelayer corvettes are facing inwards

            //move to slightly outside the circle
            minepoints = aiuGenerateCircularPath(team->shipList.selection->numShips,
                                                 thisMove->params.minevolume.volume.attribs.sphere.center,
                                                 (thisMove->params.minevolume.volume.attribs.sphere.radius+500), FALSE);
            newMove = aimCreateMoveTeamSplitNoAdd(team, selectMemDupSelection(team->shipList.selection, "mv1", 0),
                                                  minepoints, Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);
            tempMove = newMove;

            //move in a little - points the minelayers in the correct direction
            minepoints = aiuGenerateCircularPath(team->shipList.selection->numShips,
                                                 thisMove->params.minevolume.volume.attribs.sphere.center,
                                                 thisMove->params.minevolume.volume.attribs.sphere.radius, FALSE);
            newMove = aimCreateMoveTeamSplitNoAdd(team, selectMemDupSelection(team->shipList.selection, "mv1", 0),
                                                  minepoints, Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);


            //actual mine move
            newMove = aimCreateSpecialNoAdd(team, NULL, NO_FORMATION, Evasive, TRUE, TRUE);
            newMove->events = thisMove->events;
            listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

            team->curMove = tempMove;
        }
        thisMove->processing = TRUE;
    }
    else
    {
//        if (!thisMove->params.minevolume.inposition)
//        {
//            thisMove->params.minevolume.inposition = TRUE;
//            aiuWrapSpecial(team->shipList.selection, NULL);
//        }
//        else
//        {
//            if (aitTeamIsDoingSpecialOp(team) && thisMove->wait)
//            {
//                return FALSE;
//            }
//            else
//            {
//                return TRUE;
//            }
//        }
        return TRUE;
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aimProcessSpecialDefense
    Description : Processes teams with special defense ships - cloak generators, gravwell generators
    Inputs      : team - the special team
    Outputs     : May create some new moves
    Return      : TRUE if the move is complete
----------------------------------------------------------------------------*/
sdword aimProcessSpecialDefense(AITeam *team)
{
    AITeam *coopTeam = team->cooperatingTeam;
    AITeamMove *thisMove = team->curMove, *newMove;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex, "Special Defense Move - Zero sized team"));
        return TRUE;
    }



//    if (!thisMove->processing)
//    {
        //find a good team to do special defense with

            //gravwell - frigate and larger teams with no fighters guarding
            //cloakgen - frigate or slower teams
    if (coopTeam == NULL)
    {
        team->cooperatingTeam = aitFindGoodCoopTeam(team->shipList.selection->ShipPtr[0]->shiptype);

        if (team->cooperatingTeam)
        {
            team->cooperatingTeamDiedCB = aitSpecialDefenseCoopTeamDiedCB;

            if (aitTeamShipTypeIs(CloakGenerator, team))
            {
                bitSet(team->cooperatingTeam->teamFlags, TEAM_CloakCoop);
//                dbgAssert(!team->cooperatingTeam->cooperatingTeam);
                team->cooperatingTeam->cooperatingTeam = team;
                team->cooperatingTeam->cooperatingTeamDiedCB = GenericCooperatingTeamDiedCB;
            }

            //later - maybe set wait to FALSE, and have this function scan for nearby
            //        cloakables (for the cloak generator) and cloak when appropriate
            newMove = aimCreateGuardCooperatingTeamNoAdd(team, TRUE, FALSE);
            newMove->events = thisMove->events;
            aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
        }
    }
    return FALSE;

//    }
}




/*=============================================================================
    Move Creation Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aimCreateMoveTeam & aimCreateMoveTeamNoAdd
    Description : Creates a move that moves a team to a location
    Inputs      : team - the team doing the move
                  location - the location to move to
                  wait - whether the move processing waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Calls the command layer move command
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMoveTeamNoAdd(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "moveteammove", 0);

    InitNewMove(newMove,MOVE_MOVETEAM,wait,remove,formation,Neutral,aimProcessMove,NULL,NULL);
    newMove->params.move.destination = destination;

//    aiplayerLog((aiIndex,"Created Move Team Move"));

    return newMove;
}

AITeamMove *aimCreateMoveTeam(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMoveTeamNoAdd(team, destination, formation, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MoveTeam(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessMove,NULL,NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateMoveTeamIndex
    Description : Creates an indexed move that moves a team to a location
    Inputs      : team - the team doing the move
                  destination - where to move to
                  index - the move index
                  formation - the formation to use while moving
                  tactics - the type of tactics to use while moving
                  wait - whether or not to wait for the move to complete before moving on
                  remove - whether to remove the move structure after the move has completed
    Outputs     : Creates a new move
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMoveTeamIndexNoAdd(AITeam *team, vector destination, udword index,
                                        TypeOfFormation formation, TacticsType tactics,
                                        bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "moveteamindex", 0);

    InitNewMove(newMove,MOVE_MOVETEAMINDEX,wait,remove,formation,tactics,aimProcessMove,NULL,NULL);
    newMove->params.move.destination = destination;
    newMove->params.move.index       = index;

//    aiplayerLog((aiIndex,"Created Move Team Index Move"));

    return newMove;
}

AITeamMove *aimCreateMoveTeamIndex(AITeam *team, vector destination, udword index,
                                   TypeOfFormation formation, TacticsType tactics,
                                   bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMoveTeamIndexNoAdd(team, destination, index, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MoveTeamIndex(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessMove,NULL,NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateMoveTeamSplit
    Description : Creates a move to split the team's ships
    Inputs      : team - the team doing the split move, ships - the ships to split, destinations - the destinations (corresponding to the ship)
    Outputs     : Creates a new move
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMoveTeamSplitNoAdd(AITeam *team, SelectCommand *ships,
                                        Path *destinations, TacticsType tactics,
                                        bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "moveteamsplit", 0);

    dbgAssert(destinations->numPoints == ships->numShips);

    InitNewMove(newMove,MOVE_MOVETEAMSPLIT, wait, remove, NO_FORMATION, tactics, aimProcessMoveSplit, aimMoveSplitShipDied, aimMoveSplitClose);
    newMove->params.movesplit.destinations = destinations;
    newMove->params.movesplit.ships = ships;

    aiplayerLog((aiIndex,"Created Move Split Move"));

    return newMove;
}

AITeamMove *aimCreateMoveTeamSplit(AITeam *team, SelectCommand *ships,
                                   Path *destinations, TacticsType tactics,
                                   bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMoveTeamSplitNoAdd(team, ships, destinations, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

Path *LoadPath(void);       // fix later
void SavePath(Path *path);

void aimFix_MoveTeamSplit(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessMoveSplit, aimMoveSplitShipDied, aimMoveSplitClose);
}

void aimSave_MoveTeamSplit(AITeamMove *move)
{
    if (move->params.movesplit.ships) SaveSelection((SpaceObjSelection *)move->params.movesplit.ships);
    if (move->params.movesplit.destinations) SavePath(move->params.movesplit.destinations);
}


void aimLoad_MoveTeamSplit(AITeamMove *move)
{
    if (move->params.movesplit.ships) move->params.movesplit.ships = (SelectCommand *)LoadSelectionAndFix();
    if (move->params.movesplit.destinations) move->params.movesplit.destinations = LoadPath();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateHyperspace & aimCreateHyperspaceNoAdd
    Description : Creates a move that hyperspaces a team to a location
    Inputs      : team - the team doing the move
                  location - the location to move to
                  wait - whether the move processing waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Calls the command layer move command
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateHyperspaceNoAdd(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "movehyperspace", 0);

    InitNewMove(newMove,MOVE_HYPERSPACE,wait,remove,formation,Neutral,aimProcessHyperspace,NULL,NULL);
    newMove->params.move.destination = destination;

    aiplayerLog((aiIndex,"%x Created Hyperspace Move", team));

    return newMove;
}

AITeamMove *aimCreateHyperspace(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateHyperspaceNoAdd(team, destination, formation, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Hyperspace(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessHyperspace,NULL,NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateIntercept & aimCreateInterceptNoAdd
    Description : Creates a move that moves a team to the location of a ship
                  -attempts to intercept the ship by predicting it's location based on previous checks
    Inputs      : team      - the team doing the move
                  ship      - the ship to intercept
                  interval  - how many seconds to wait until checking the move again
                  formation - the formation to do the move in
                  tactics   - the tactics to use while doing the move
                  wait      - whether the move processing waits for this move to complete
                  remove    - whether the move processor removes this move when it is done
    Outputs     : Calls the command layer move command
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateInterceptNoAdd(AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "moveintercept", 0);

    InitNewMove(newMove,MOVE_INTERCEPT,wait,remove,formation,tactics,aimProcessIntercept,aimInterceptShipDied,NULL);
    newMove->params.intercept.ship     = ship;
    newMove->params.intercept.interval = interval;
    newMove->params.intercept.next_int = 0.0;

//    aiplayerLog((aiIndex,"Created Intercept Move"));

    return newMove;
}

AITeamMove *aimCreateIntercept(AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateInterceptNoAdd(team, ship, interval, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Intercept(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessIntercept,aimInterceptShipDied,NULL);
    move->params.intercept.ship = SpaceObjRegistryGetShip((sdword)move->params.intercept.ship);
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_Intercept(AITeamMove *move)
{
    move->params.intercept.ship = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)move->params.intercept.ship);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*-----------------------------------------------------------------------------
    Name        : aimCreateMoveTo
    Description : Creates a mve that moves the team toward a location for a certain distance or duration
    Inputs      : team - the team to move,
                  destination - the direction to move the team,
                    Note: destination is always considered to be further than the team would go
                          if the destination is reached before the limiter is reached, the team stops
                  limiter - determines when the move stops,
                  type - what type of move this is,
                  formation - the formation to do the move in
                  tactics - the tactics to use while doing the move
                  wait - whether the move processor waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMoveToNoAdd(AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "movemoveto", 0);

    InitNewMove(newMove, MOVE_MOVETO,wait,remove,formation,tactics,aimProcessMoveTo,NULL,NULL);
    newMove->params.moveTo.destination = destination;
    newMove->params.moveTo.limiter     = limiter;
    newMove->params.moveTo.type        = type;

//    aiplayerLog((aiIndex,"Created Moveto Move"));

    return newMove;
}

AITeamMove *aimCreateMoveTo(AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMoveToNoAdd(team, destination, limiter, type, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MoveTo(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessMoveTo,NULL,NULL);
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateCountShips
    Description : Creates a move that makes the team count ships within visual range
    Inputs      : team - the team doing the counting
                  wait - whether the move processing waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateCountShipsNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "countshipsmove", 0);

    InitNewMove(newMove,MOVE_COUNTSHIPS,wait,remove,formation,Neutral,aimProcessCountShips,NULL,NULL);

//    aiplayerLog((aiIndex,"Created Count Ships Move"));

    return newMove;
}

AITeamMove *aimCreateCountShips(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateCountShipsNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_CountShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessCountShips,NULL,NULL);
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateFlankAttack
    Description : Creates a move that makes a team attack from a flanking angle, rather than straight on
    Inputs      : team - the team doing the attacking
                  targets - the selection of attackees
                  hyperspace - if TRUE the flanking is done by hyperspacing
                  wait - whether the move processor waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateFlankAttackNoAdd(AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove;

    if (!aiuTeamFeatureEnabled(team, AIT_FLANK_ATTACK))
    {
        return aimCreateAttackNoAdd(team, targets, formation, wait, remove);
    }

    newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "FlankAttackmove", 0);

    InitNewMove(newMove,MOVE_FLANKATTACK,wait,remove,formation,Neutral,aimProcessFlankAttack,aimFlankAttackShipDied,aimFlankAttackClose);
    newMove->params.flankatt.targets    = targets;
    newMove->params.flankatt.hyperspace = hyperspace;

    aiplayerLog((aiIndex,"%x Created Flanking Attack Move", team));

    return newMove;
}

AITeamMove *aimCreateFlankAttack(AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateFlankAttackNoAdd(team, targets, hyperspace, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_FlankAttack(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessFlankAttack,aimFlankAttackShipDied,aimFlankAttackClose);
}

void aimSave_FlankAttack(AITeamMove *move)
{
    if (move->params.flankatt.targets) SaveSelection((SpaceObjSelection *)move->params.flankatt.targets);
}

void aimLoad_FlankAttack(AITeamMove *move)
{
    if (move->params.flankatt.targets) move->params.flankatt.targets = (SelectCommand *)LoadSelectionAndFix();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateAdvancedAttack
    Description : Creates a move that makes the team attack an enemy more efficiently
    Inputs      : team - the team doing the attacking
                  targets - the attackees
                  wait - whether the move processor waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : the newly created  move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateAdvancedAttackNoAdd(AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    if (!aiuTeamFeatureEnabled(team, AIT_ADVANCED_ATTACK))
    {
        return aimCreateAttackNoAdd(team, targets, formation, wait, remove);
    }

    newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "advancedattackmove", 0);

    InitNewMove(newMove,MOVE_ADVANCEDATTACK,wait,remove,formation,tactics,aimProcessAdvancedAttack,aimAdvancedAttackShipDied,aimAdvancedAttackClose);
    newMove->params.advatt.targets     = targets;
    newMove->params.advatt.target_ship = NULL;

//    aiplayerLog((aiIndex,"Created Advanced Attack Move"));

    return newMove;
}

AITeamMove *aimCreateAdvancedAttack(AITeam *team, SelectCommand *target, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateAdvancedAttackNoAdd(team,target,formation,tactics,wait,remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_AdvancedAttack(AITeamMove *move)
{
    move->params.advatt.target_ship = (Ship*)SpaceObjRegistryGetID((SpaceObj *)move->params.advatt.target_ship);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

void aimSave_AdvancedAttack(AITeamMove *move)
{
    if (move->params.advatt.targets) SaveSelection((SpaceObjSelection *)move->params.advatt.targets);
}

void aimLoad_AdvancedAttack(AITeamMove *move)
{
    if (move->params.advatt.targets) move->params.advatt.targets = (SelectCommand *)LoadSelectionAndFix();
}

void aimFix_AdvancedAttack(AITeamMove *move)
{
    move->params.advatt.target_ship = SpaceObjRegistryGetShip((sdword)move->params.advatt.target_ship);
    FixMoveFuncPtrs(move,aimProcessAdvancedAttack,aimAdvancedAttackShipDied,aimAdvancedAttackClose);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateMoveAttack
    Description : Creates a move that attacks a ship, then moves around to flank it.
    Inputs      : team, targets, formation, tactics, wait, remove
                  Advanced boolean variable determines whether the attack is an advanced attack or not.
    Outputs     : Creates a new move
    Return      : The new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMoveAttackNoAdd(AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "moveattackmove", 0);

    InitNewMove(newMove,MOVE_MOVEATTACK,wait,remove,formation,tactics,aimProcessMoveAttack,aimMoveAttackShipDied,aimMoveAttackClose);
    newMove->params.moveatt.targets     = targets;
    newMove->params.moveatt.target_ship = NULL;
    vecZeroVector(newMove->params.moveatt.destination);

    if (Advanced)
    {
        bitSet(team->teamFlags, TEAM_AdvancedMoveAttack);
    }
    return newMove;
}
AITeamMove *aimCreateMoveAttack(AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMoveAttackNoAdd(team, targets, Advanced, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_MoveAttack(AITeamMove *move)
{
    move->params.moveatt.target_ship = (Ship*)SpaceObjRegistryGetID((SpaceObj *)move->params.moveatt.target_ship);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"


void aimFix_MoveAttack(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessMoveAttack,aimMoveAttackShipDied,aimMoveAttackClose);
}

void aimSave_MoveAttack(AITeamMove *move)
{
    if (move->params.moveatt.targets) SaveSelection((SpaceObjSelection *)move->params.moveatt.targets);
}

void aimLoad_MoveAttack(AITeamMove *move)
{
    move->params.moveatt.target_ship = SpaceObjRegistryGetShip((sdword)move->params.moveatt.target_ship);
    if (move->params.moveatt.targets) move->params.moveatt.targets = (SelectCommand *)LoadSelectionAndFix();
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateHarassAttack
    Description : Creates a move that makes the team harass an enemy
    Inputs      : team - the team doing the harassing
                  wait - whether the move processing waits for this move to complete
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateHarassAttack(AITeam *team, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "harassattackmove", 0);

    InitNewMove(newMove,MOVE_HARASSATTACK,wait,remove,formation,Neutral,aimProcessHarassAttack,aimHarassAttackShipDied,NULL);
    newMove->params.harass.target = NULL;

//    aiplayerLog((aiIndex,"Created Harass Attack Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_HarassAttack(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessHarassAttack,aimHarassAttackShipDied,NULL);
    move->params.harass.target = SpaceObjRegistryGetShip((sdword)move->params.harass.target);
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_HarassAttack(AITeamMove *move)
{
    move->params.harass.target = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)move->params.harass.target);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*-----------------------------------------------------------------------------
    Name        : aimCreateDock
    Description : Creates a move that makes the team dock with the nearest dockable ship
    Inputs      : team - the team doing the docking,
                  wait - whether the move processing waits for this move to complete,
                  remove - whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateDockNoAdd(AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = AIM_DOCK_DEFAULT_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "dockmove", 0);

    InitNewMove(newMove, MOVE_DOCK, wait, remove,formation,Neutral, aimProcessDock, aimDockShipDied, aimDockClose);
    newMove->params.dock.shipsDocking = NULL;
    newMove->params.dock.dockmoveFlags = dockmoveFlags;
    newMove->params.dock.dockAt = dockAt;

//    aiplayerLog((aiIndex,"Created Dock Move"));

    return newMove;
}

AITeamMove *aimCreateDock(AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateDockNoAdd(team, dockmoveFlags, dockAt, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_Dock(AITeamMove *move)
{
    move->params.dock.dockAt = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)move->params.dock.dockAt);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

void aimFix_Dock(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessDock, aimDockShipDied, aimDockClose);
    move->params.dock.dockAt = SpaceObjRegistryGetShip((sdword)move->params.dock.dockAt);
}

void aimSave_Dock(AITeamMove *move)
{
    if (move->params.dock.shipsDocking) SaveSelection((SpaceObjSelection *)move->params.dock.shipsDocking);
}

void aimLoad_Dock(AITeamMove *move)
{
    if (move->params.dock.shipsDocking) move->params.dock.shipsDocking = (SelectCommand *)LoadSelectionAndFix();
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateDefendMothership
    Description : Creates a move that makes a team defend the mothership
    Inputs      : team - the team to do the defending
                  wait - whether the move processing waits for this move to complete
                  remove -whether the move processor removes this move when it is done
    Outputs     : Creates a new move structure
    Return      : The newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateDefendMothershipNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "defendmoshipmove", 0);

    InitNewMove(newMove, MOVE_DEFMOSHIP, wait, remove, formation,Neutral,aimProcessDefendMothership, aimDefendMothershipShipDied, aimDefendMothershipClose);
    newMove->params.defmoship.targets = NULL;

//    aiplayerLog((aiIndex,"Created Defend Mothership Move"));

    return newMove;
}

AITeamMove *aimCreateDefendMothership(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateDefendMothershipNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_DefMoship(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessDefendMothership, aimDefendMothershipShipDied, aimDefendMothershipClose);
}

void aimSave_DefMoship(AITeamMove *move)
{
    if (move->params.defmoship.targets) SaveSelection((SpaceObjSelection *)move->params.defmoship.targets);
}

void aimLoad_DefMoship(AITeamMove *move)
{
    if (move->params.defmoship.targets) move->params.defmoship.targets = (SelectCommand *)LoadSelectionAndFix();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreatePatrolMove
    Description : Creates a move that makes a team patrol a pathway
    Inputs      : team - the team to do the patrolling
                  wait - whether the move processing waits for this move to complete
                  remove - whether the move gets deleted when it is done
    Outputs     : Creates a new move structure
    Return      : The newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreatePatrolMoveNoAdd(AITeam *team, Path *path, udword startIndex,
                                     TypeOfFormation formation, TacticsType tactics,
                                     bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "patrolmove", 0);

    InitNewMove(newMove, MOVE_PATROLMOVE, wait, remove,formation,tactics,aimProcessPatrolMove, NULL, aimPatrolMoveClose);
    newMove->params.patrolmove.path       = path;
    newMove->params.patrolmove.loopMove   = NULL;
    newMove->params.patrolmove.startIndex = startIndex;

//    aiplayerLog((aiIndex,"Created Patrol Move Move"));

    return newMove;
}

AITeamMove *aimCreatePatrolMove(AITeam *team, Path *path, udword startIndex,
                                TypeOfFormation formation, TacticsType tactics,
                                bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreatePatrolMoveNoAdd(team, path, startIndex, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimFix_PatrolMove(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessPatrolMove, NULL, aimPatrolMoveClose);
    move->params.patrolmove.loopMove = ConvertNumToPointerInList(&savingThisAITeam->moves,(sdword)move->params.patrolmove.loopMove);
}

void aimPreFix_PatrolMove(AITeamMove *move)
{
    move->params.patrolmove.loopMove = (AITeamMove*)ConvertPointerInListToNum(&savingThisAITeam->moves,move->params.patrolmove.loopMove);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

Path *LoadPath(void);       // fix later
void SavePath(Path *path);

void aimSave_PatrolMove(AITeamMove *move)
{
    if (move->params.patrolmove.path) SavePath(move->params.patrolmove.path);
}

void aimLoad_PatrolMove(AITeamMove *move)
{
    if (move->params.patrolmove.path) move->params.patrolmove.path = LoadPath();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateActivePatrol
    Description : Creates a move that sends the team on a patrol that patrols
                  the continuously changing fleet
    Inputs      : team - the team to add the move to,
                  wait - whether to wait for this move to complete,
                  remove - whether to remove the move after it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateActivePatrolNoAdd(AITeam *team, udword patroltype, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = DELTA3D_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "activepatrol", 0);

    InitNewMove(newMove, MOVE_ACTIVEPATROL, wait, remove, formation,Neutral,aimProcessActivePatrol, NULL, NULL);
    newMove->params.activepatrol.patroltype = patroltype;

//    aiplayerLog((aiIndex,"Created Active Patrol Move"));

    return newMove;
}

AITeamMove *aimCreateActivePatrol(AITeam *team, udword patroltype, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateActivePatrolNoAdd(team, patroltype, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ActivePatrol(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessActivePatrol, NULL, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateTempGuard
    Description : Creates the move required for an attack team to become a temporary
                  guard team
    Inputs      : team - the team to do the temp guard,
                  wait - whether to wait for this move to complete,
                  remove - whether to remove the move after it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateTempGuardNoAdd(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "tempguard", 0);

    InitNewMove(newMove, MOVE_TEMPGUARD, wait, remove, formation,tactics,aimProcessTempGuard, NULL, NULL);

//    aiplayerLog((aiIndex,"Created Temp Guard Move"));

    return newMove;
}

AITeamMove *aimCreateTempGuard(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateTempGuardNoAdd(team, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_TempGuard(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessTempGuard, NULL, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateReinforce
    Description : Creates a move to make a team reinforce another team
    Inputs      : team - the team to do the reinforcing,
                  reinforceteam - the team to reinforce
                  wait - whether to wait for this move to complete,
                  remove - whether to remove the move after it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateReinforceNoAdd(AITeam *team, AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "reinforce", 0);

    InitNewMove(newMove, MOVE_REINFORCE, wait, remove, formation,tactics,aimProcessReinforce, NULL, NULL);
    newMove->params.reinforce.reinforceteam = reinforceteam;

//    aiplayerLog((aiIndex,"Created Reinforce Move"));

    return newMove;
}

AITeamMove *aimCreateReinforce(AITeam *team, AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateReinforceNoAdd(team, reinforceteam, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimFix_Reinforce(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessReinforce, NULL, NULL);
    move->params.reinforce.reinforceteam = AITeamIndexToTeam(savingThisAITeam->aiplayerowner,(sdword)move->params.reinforce.reinforceteam);
}

void aimPreFix_Reinforce(AITeamMove *move)
{
    move->params.reinforce.reinforceteam = (AITeam *)AITeamToTeamIndex(move->params.reinforce.reinforceteam);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*-----------------------------------------------------------------------------
    Name        : aimCreateSupport
    Description : Creates a move to make a team support another team (only for
                  AdvanceSupportFrigates, Repair Corvettes and Resource Controllers)
    Inputs      : team - the team doing the supporting
                  ships - the ships to support
                  formation - the formation to use
                  tactics - the tactics to use
                  wait - whether to wait for this move to complete
                  remove - whether to remove the move after it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateSupportNoAdd(AITeam *team, SelectCommand *ships,
                                  TypeOfFormation formation, TacticsType tactics,
                                  bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "support", 0);

    InitNewMove(newMove, MOVE_SUPPORT, wait, remove, formation, tactics,
                aimProcessSupport, aimSupportShipDied, aimSupportClose);
    newMove->params.support.ships = ships;

//    aiplayerLog((aiIndex,"Created Support Move"));

    return newMove;
}

AITeamMove *aimCreateSupport(AITeam *team, SelectCommand *ships,
                             TypeOfFormation formation, TacticsType tactics,
                             bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSupportNoAdd(team, ships, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Support(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessSupport, aimSupportShipDied, aimSupportClose);
}

void aimSave_Support(AITeamMove *move)
{
    if (move->params.support.ships) SaveSelection((SpaceObjSelection *)move->params.support.ships);
}

void aimLoad_Support(AITeamMove *move)
{
    if (move->params.support.ships) move->params.support.ships = (SelectCommand *)LoadSelectionAndFix();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateActiveRecon
    Description : Creates a move to make a team do active reconaissance
    Inputs      : team - the team doing the reconaissance
                  formation - the formation to use
                  tactics - the tactics to use
                  wait - whether to wait for this move to complete
                  remove - whether to remove the move after it is done
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateActiveReconNoAdd(AITeam *team, bool EnemyRecon, TypeOfFormation formation,
                                      TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "actrecon", 0);

    InitNewMove(newMove, MOVE_ACTIVERECON, wait, remove, formation, tactics,
                aimProcessActiveRecon, NULL, NULL);
    newMove->params.activerecon.outwards   = TRUE;
    newMove->params.activerecon.enemyrecon = EnemyRecon;

//    aiplayerLog((aiIndex,"Created Active Recon Move"));

    return newMove;
}

AITeamMove *aimCreateActiveRecon(AITeam *team, bool EnemyRecon, TypeOfFormation formation,
                                 TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateActiveReconNoAdd(team, EnemyRecon, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ActiveRecon(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessActiveRecon, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateShipRecon
    Description : Creates a move to make a team recon a set of ships
    Inputs      : team, ships, formation, tactics, wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateShipReconNoAdd(AITeam *team, SelectCommand *ships, TypeOfFormation formation,
                                    TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "shiprecon", 0);

    InitNewMove(newMove, MOVE_SHIPRECON, wait, remove, formation, tactics,
                aimProcessShipRecon, aimShipReconShipDied, aimShipReconClose);
    newMove->params.shiprecon.ships      = ships;
    newMove->params.shiprecon.foundships = NULL;

//    aiplayerLog((aiIndex, "Created Ship Recon Move"));

    return newMove;
}

AITeamMove *aimCreateShipRecon(AITeam *team, SelectCommand *ships, TypeOfFormation formation,
                               TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateShipReconNoAdd(team, ships, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ShipRecon(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessShipRecon, aimShipReconShipDied, aimShipReconClose);
}

void aimSave_ShipRecon(AITeamMove *move)
{
    if (move->params.shiprecon.ships) SaveSelection((SpaceObjSelection *)move->params.shiprecon.ships);
}

void aimLoad_ShipRecon(AITeamMove *move)
{
    if (move->params.shiprecon.ships) move->params.shiprecon.ships = (SelectCommand *)LoadSelectionAndFix();
}



/*-----------------------------------------------------------------------------
    Name        : aimCreateArmada
    Description : Creates an active move that takes care of large groups of capital ships
    Inputs      : team - the team doing the armadaing
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateArmadaNoAdd(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "armada", 0);

    InitNewMove(newMove, MOVE_ARMADA, wait, remove, formation, tactics, aimProcessArmada, NULL, NULL);

    aiplayerLog((aiIndex, "%x Created Armada Move", team));

    return newMove;
}

AITeamMove *aimCreateArmada(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateArmadaNoAdd(team, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Armada(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessArmada, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateControlResources
    Description : Creates a move that gets a resource controller to control resources
    Inputs      : team - the Resource Controller team,
                  ships - the ships to control
                        Note: if ships == NULL, the move will control all resourcers
                  wait - whether to wait for the move to finish,
                  remove - whether to delete the move after it's done1
    Outputs     : Creates a new move
    Return      : The new move that was created as per "Outputs"
----------------------------------------------------------------------------*/
AITeamMove *aimCreateControlResourcesNoAdd(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "rescontrol", 0);

    InitNewMove(newMove, MOVE_CONTROLRESOURCES, wait, remove, NO_FORMATION, Neutral,
                aimProcessControlResources, aimControlResourcesShipDied, NULL/*aimControlResourcesClose*/);

    newMove->params.rescontrol.ships = ships;

//    aiplayerLog((aiIndex, "Created Control Resources Move"));

    return newMove;
}

AITeamMove *aimCreateControlResources(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateControlResourcesNoAdd(team, ships, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ControlResources(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessControlResources, aimControlResourcesShipDied, NULL/*aimControlResourcesClose*/);
}

void aimSave_ControlResources(AITeamMove *move)
{
    if (move->params.rescontrol.ships) SaveSelection((SpaceObjSelection *)move->params.rescontrol.ships);
}

void aimLoad_ControlResources(AITeamMove *move)
{
    if (move->params.rescontrol.ships) move->params.rescontrol.ships = (SelectCommand *)LoadSelectionAndFix();
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateSwarmAttack
    Description : Creates a swarm attack of ships - special code for P2 pirates
    Inputs      : team - the team swarm
                  wait - whether to wait for the move to finish
                  remove - whether the move gets deleted after it's done
    Outputs     : Creates a new move
    Return      : void
----------------------------------------------------------------------------*/
AITeamMove *aimCreateSwarmAttackNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "swarmattack", 0);

    InitNewMove(newMove, MOVE_SWARMATTACK, wait, remove, NO_FORMATION, Neutral,
                aimProcessSwarmAttack, aimSwarmAttackShipDied, aimSwarmAttackClose);
    growSelectInit(&newMove->params.swarmatt.newSwarmers);
    newMove->params.swarmatt.targets      = NULL;
    newMove->params.swarmatt.othertargets = NULL;

//    aiplayerLog((aiIndex, "Created Swarm Attack Move"));

    return newMove;
}

AITeamMove *aimCreateSwarmAttack(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSwarmAttackNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_SwarmAttack(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessSwarmAttack, aimSwarmAttackShipDied, aimSwarmAttackClose);
}

void aimSave_SwarmAttack(AITeamMove *move)
{
    SaveGrowSelection(&move->params.swarmatt.newSwarmers);
    if (move->params.swarmatt.targets) SaveSelection((SpaceObjSelection *)move->params.swarmatt.targets);
    if (move->params.swarmatt.othertargets) SaveSelection((SpaceObjSelection *)move->params.swarmatt.othertargets);
}

void aimLoad_SwarmAttack(AITeamMove *move)
{
    LoadGrowSelectionAndFix(&move->params.swarmatt.newSwarmers);
    if (move->params.swarmatt.targets) move->params.swarmatt.targets = (SelectCommand *)LoadSelectionAndFix();
    if (move->params.swarmatt.othertargets) move->params.swarmatt.othertargets = (SelectCommand *)LoadSelectionAndFix();
}

/*-----------------------------------------------------------------------------
    Name        : aimCreateSwarmDefense
    Description : Creates a swarm defense - special code for P2 pirates to defend
                  fuel pods
    Inputs      : team - the team swarm
                  pod - pointer to the fuel pod
                  wait - whether to wait for the move to finish
                  remove - whether the move gets deleted after it's done
    Outputs     : Creates a new move
    Return      : void
----------------------------------------------------------------------------*/
AITeamMove *aimCreateSwarmDefenseNoAdd(AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "swarmdefense", 0);

    InitNewMove(newMove, MOVE_SWARMDEFENSE, wait, remove, NO_FORMATION, Neutral,
                aimProcessSwarmDefense, aimSwarmDefenseShipDied, aimSwarmDefenseClose);
    growSelectInit(&newMove->params.swarmdef.newSwarmers);
    newMove->params.swarmdef.guarding    = NULL;
    newMove->params.swarmdef.Pods        = pods;
    newMove->params.swarmdef.full_refuel = FALSE;
    newMove->params.swarmdef.full_attack = FALSE;

//    aiplayerLog((aiIndex, "Created Swarm Defense Move"));

    return newMove;
}

AITeamMove *aimCreateSwarmDefense(AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSwarmDefenseNoAdd(team, pods, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_SwarmDefense(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessSwarmDefense, aimSwarmDefenseShipDied, aimSwarmDefenseClose);
}

void aimSave_SwarmDefense(AITeamMove *move)
{
    SaveGrowSelection(&move->params.swarmdef.newSwarmers);
    if (move->params.swarmdef.guarding) SaveSelection((SpaceObjSelection *)move->params.swarmdef.guarding);
    if (move->params.swarmdef.Pods)     SaveSelection((SpaceObjSelection *)move->params.swarmdef.Pods);
}

void aimLoad_SwarmDefense(AITeamMove *move)
{
    LoadGrowSelectionAndFix(&move->params.swarmdef.newSwarmers);
    if (move->params.swarmdef.guarding) move->params.swarmdef.guarding = (SelectCommand *)LoadSelectionAndFix();
    if (move->params.swarmdef.Pods)     move->params.swarmdef.Pods     = (SelectCommand *)LoadSelectionAndFix();
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateSwarmPod
    Description : Creates a move to control the swarm pod
    Inputs      : team, wait, remove
    Outputs     : A new move is created
    Return      : The new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateSwarmPodNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "swarmpod", 0);

    InitNewMove(newMove, MOVE_SWARMPOD, wait, remove, NO_FORMATION, Neutral,
                aimProcessSwarmPod, NULL, NULL);
    newMove->params.swarmpod.first_attack = FALSE;
    newMove->params.swarmpod.attack_delay = 0;

//    aiplayerLog((aiIndex, "Created Swarm Pod Move"));

    return newMove;
}
AITeamMove *aimCreateSwarmPod(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSwarmPodNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_SwarmPod(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessSwarmPod, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateResourceVolume
    Description : Creates a move that makes a team collect resources in a spherical volume
    Inputs      : team - the team doing the resourcing,
                  center - the center of the sphere,
                  radius - the radius of the sphere,
                  strictVolume - whether the volume is strictly adhered to, or if
                                 the resourcer is allowed to wander a bit
                  wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateResourceVolumeNoAdd(AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "resvolume", 0);

    InitNewMove(newMove, MOVE_RESVOLUME, wait, remove, NO_FORMATION, Neutral,
                aimProcessResourceVolume, aimResourceVolumeShipDied, aimResourceVolumeClose);
    newMove->moveResourceDiedFunction        = aimResourceVolumeResourceDied;
    newMove->params.resvolume.volume         = volume;
    newMove->params.resvolume.volResources   = NULL;
    newMove->params.resvolume.takenResources = NULL;
    newMove->params.resvolume.strictVolume   = strictVolume;

//    aiplayerLog((aiIndex, "Created Resource Volume Move"));

    return newMove;
}
AITeamMove *aimCreateResourceVolume(AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateResourceVolumeNoAdd(team, volume, strictVolume, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ResourceVolume(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessResourceVolume, aimResourceVolumeShipDied, aimResourceVolumeClose);
    move->moveResourceDiedFunction = aimResourceVolumeResourceDied;
}

void aimSave_ResourceVolume(AITeamMove *move)
{
    if (move->params.resvolume.volResources) SaveSelection((SpaceObjSelection *)move->params.resvolume.volResources);
    if (move->params.resvolume.takenResources) SaveSelection((SpaceObjSelection *)move->params.resvolume.takenResources);
}

void aimLoad_ResourceVolume(AITeamMove *move)
{
    if (move->params.resvolume.volResources) move->params.resvolume.volResources = (ResourceSelection *)LoadSelectionAndFix();
    if (move->params.resvolume.takenResources) move->params.resvolume.takenResources = (ResourceSelection *)LoadSelectionAndFix();
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateActiveResource
    Description : Creates a move to continuously look for the best resource pockets and strip them dry
    Inputs      : team, wait, remove
    Outputs     : Creates a new move
    Return      : The new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateActiveResourceNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "activeres", 0);

    InitNewMove(newMove, MOVE_ACTIVERES, wait, remove, NO_FORMATION, Neutral,
                aimProcessActiveResource, NULL, NULL);

//    aiplayerLog((aiIndex, "Created Active Resource Move"));

    return newMove;
}

AITeamMove *aimCreateActiveResource(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateActiveResourceNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ActiveResource(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessActiveResource, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateMothershipMove
    Description : Creates a new move that gets the mothership to move in productive ways
    Inputs      : team, wait, remove
    Outputs     : Creates a new move
    Return      : The new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMothershipMoveNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "mothership", 0);

    InitNewMove(newMove, MOVE_MOTHERSHIP, wait, remove, NO_FORMATION, Neutral,
                aimProcessMothershipMove, aimMothershipMoveShipDied, NULL);

//    aiplayerLog((aiIndex, "Created Mothership Move"));

    return newMove;
}

AITeamMove *aimCreateMothershipMove(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMothershipMoveNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MothershipMove(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessMothershipMove, aimMothershipMoveShipDied, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateCapture
    Description : Creates a move to capture an enemy ship
    Inputs      : team - the team to capture the ship,
                  ship - the ship to capture,
                  wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateCaptureNoAdd(AITeam *team, ShipPtr ship, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "capture", 0);

    InitNewMove(newMove, MOVE_CAPTURE, wait, remove, NO_FORMATION, Neutral,
                aimProcessCapture, aimCaptureShipDied, NULL);
    newMove->params.capture.ship = ship;

//    aiplayerLog((aiIndex, "Created Capture Move"));

    return newMove;
}

AITeamMove *aimCreateCapture(AITeam *team, ShipPtr ship, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateCaptureNoAdd(team, ship, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Capture(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessCapture, aimCaptureShipDied, NULL);
}



/*-----------------------------------------------------------------------------
    Name        : aimCreateActiveCapture
    Description : Moves the salvage capture ships around until a nice meaty target is found
    Inputs      : team - the salvage capture team, wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateActiveCaptureNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "activecapture", 0);

    InitNewMove(newMove, MOVE_ACTIVECAPTURE, wait, remove, NO_FORMATION, Neutral,
                aimProcessActiveCapture, NULL, NULL);

//    aiplayerLog((aiIndex, "Created Active Capture"));

    return newMove;
}

AITeamMove *aimCreateActiveCapture(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateActiveCaptureNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ActiveCapture(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessActiveCapture, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateActiveMine
    Description : Moves the minelayer corvettes to a nice place for mining, then covers the area with a bunch of mines
    Inputs      : team, wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateActiveMineNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "activemine", 0);

    InitNewMove(newMove, MOVE_ACTIVEMINE, wait, remove, NO_FORMATION, Neutral,
                aimProcessActiveMine, NULL, NULL);

//    aiplayerLog((aiIndex, "Created Active Mine"));

    return newMove;
}

AITeamMove *aimCreateActiveMine(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateActiveMineNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_ActiveMine(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessActiveMine, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateMineVolume
    Description : Creates a move to make the team mine a volume in space
    Inputs      : team, volume - the volume to mine, wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateMineVolumeNoAdd(AITeam *team, Volume volume, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "minevol", 0);

    InitNewMove(newMove, MOVE_MINEVOLUME, wait, remove, NO_FORMATION, Neutral,
                aimProcessMineVolume, NULL, NULL);
    newMove->params.minevolume.volume = volume;

//    aiplayerLog((aiIndex, "Created Mine Volume"));

    return newMove;
}
AITeamMove *aimCreateMineVolume(AITeam *team, Volume volume, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateMineVolumeNoAdd(team, volume, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MineVolume(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessMineVolume, NULL, NULL);
}


/*-----------------------------------------------------------------------------
    Name        : aimCreateSpecialDefense
    Description : Active Move to properly deal with special ships (cloak generators, gravwell generators)
    Inputs      : team, wait, remove
    Outputs     : Creates a new move
    Return      : the new move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateSpecialDefenseNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "specdef", 0);

    InitNewMove(newMove, MOVE_SPECIALDEFENSE, wait, remove, NO_FORMATION, Neutral,
                aimProcessSpecialDefense, NULL, NULL);

//    aiplayerLog((aiIndex, "Created Special Defense"));

    return newMove;
}
AITeamMove *aimCreateSpecialDefense(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSpecialDefenseNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_SpecialDefense(AITeamMove *move)
{
    FixMoveFuncPtrs(move, aimProcessSpecialDefense, NULL, NULL);
}



/*-----------------------------------------------------------------------------
    Name        : aimCreateDeleteTeam
    Description : Creates a move to delete the current team
    Inputs      : team - the team to delete
    Outputs     : Removes the team and it's moves from memory
    Return      : the newly created move
----------------------------------------------------------------------------*/
AITeamMove *aimCreateDeleteTeamNoAdd(AITeam *team)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "delete", 0);

    InitNewMove(newMove, MOVE_DELETETEAM, TRUE, TRUE, SAME_FORMATION, Neutral, NULL, NULL, NULL);

//    aiplayerLog((aiIndex, "Created Delete Team Move"));

    return newMove;
}

AITeamMove *aimCreateDeleteTeam(AITeam *team)
{
    AITeamMove *newMove;

    newMove = aimCreateDeleteTeamNoAdd(team);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_DeleteTeam(AITeamMove *move)
{
    FixMoveFuncPtrs(move,NULL, NULL, NULL);
}




