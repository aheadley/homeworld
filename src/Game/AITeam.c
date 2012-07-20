/*=============================================================================
    Name    : AITeam.c
    Purpose : Team stuff for AIPlayer

    Created 5/31/1998 by dstone
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "AITeam.h"
#include "AIAttackMan.h"
#include "AIDefenseMan.h"
#include "AIHandler.h"
#include "CommandWrap.h"
#include "Memory.h"
#include "Universe.h"
#include "Types.h"
#include "ShipSelect.h"
#include "SpaceObj.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIEvents.h"
#include "SaveGame.h"
#include "AIMoves.h"
#include "KAS.h"
#include "Tactics.h"
#include "GravWellGenerator.h"

/*-----------------------------------------------------------------------------
    Name        : aitCreate
    Description : Creates a new team instance
    Inputs      : teamType - the type of team to create
    Outputs     : Allocates memory for a team
    Return      : the new team created
----------------------------------------------------------------------------*/
AITeam *aitCreate(TeamType teamType)
{
    AITeam *team;

    team           = memAlloc(sizeof(AITeam), "aiteam", 0);
    team->teamType = teamType;
#if 0
    if (team->teamType == ScriptTeam)
        // assign team to computer player (#1 in single player game)
        team->aiplayerowner = &universe.players[1];
    else
#endif
    team->aiplayerowner = aiCurrentAIPlayer;
    growSelectInit(&team->shipList);
    listInit(&team->moves);

    team->teamFlags             = 0x00000000;
    team->newships              = 0;
    team->curMove               = NULL;
    team->custTeamInfo          = NULL;
    team->TeamDiedCB            = NULL;
    team->teamStrength          = 0;
    team->teamValue             = 0;
    team->teamDifficultyLevel   = (AITeamLevel)2*aiCurrentAIPlayer->aiplayerDifficultyLevel;
    team->cooperatingTeam       = NULL;
    team->cooperatingTeamDiedCB = NULL;
    team->msgQueue              = NULL;
    team->msgSender             = NULL;

    team->kasLabel[0]           = 0;
    team->kasFSMName[0]         = 0;
    team->kasStateName[0]       = 0;
    team->kasFSMWatchFunction   = NULL;
    team->kasStateWatchFunction = NULL;
    team->kasTactics            = Neutral;
    team->kasFormation          = NO_FORMATION;

    // keep track of all allocations
    if (team->aiplayerowner->teamsUsed >= team->aiplayerowner->teamsAllocated)
    {
        // allocate more if necessary
        team->aiplayerowner->teams = memRealloc(team->aiplayerowner->teams, sizeof(AITeam *) * (team->aiplayerowner->teamsAllocated + AITEAM_ALLOC_INCREMENT), "aiteamlist", 0);
        team->aiplayerowner->teamsAllocated += AITEAM_ALLOC_INCREMENT;
    }
    team->aiplayerowner->teams[team->aiplayerowner->teamsUsed++] = team;

    switch (team->teamDifficultyLevel)
    {
        case TEAM_ADVANCED:
            aiuEnableTeamFeature(team, AIT_ADVANCED_ATTACK);
        case TEAM_INTERMEDIATE_ADVANCED:
            aiuEnableTeamFeature(team, AIT_FLANK_ATTACK);
            aiuEnableTeamFeature(team, AIT_CLOAKING);
            aiuEnableTeamFeature(team, AIT_GRAVWELL);
        case TEAM_INTERMEDIATE:
            aiuEnableTeamFeature(team, AIT_TACTICS);
        case TEAM_BEGINNER_INTERMEDIATE:
        case TEAM_BEGINNER:
            break;
        default:
            break;
    }

    team->teamDelay = AIT_TEAM_MOVE_DELAY[team->teamDifficultyLevel];

    return team;
}


/*-----------------------------------------------------------------------------
    Name        : aitFindMoveOfType
    Description : Returns a move of the type specified
    Inputs      : team - the team to find the move for, type - the movetype to stop at
    Outputs     : Deletes a bunch of moves
    Return      : The move or NULL
----------------------------------------------------------------------------*/
AITeamMove *aitFindMoveOfType(AITeam *team, AIMoveTypes type)
{
    AITeamMove *move = team->curMove;
    Node *nextnode = move->listNode.next;

    while ((move->type != type) && (nextnode != NULL))
    {
        move     = (AITeamMove *)listGetStructOfNode(nextnode);
        nextnode = move->listNode.next;
    }

    if (move->type != type)
    {
        return NULL;
    }
    else
    {
        return move;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitDeleteCurrentMove
    Description : Delete's the team's current move
    Inputs      : team - the team
    Outputs     : delete's the team's current move
    Return      : nothing
----------------------------------------------------------------------------*/
void aitDeleteCurrentMove(AITeam *team)
{
    AITeamMove *move = team->curMove;
    Node *nextnode = move->listNode.next;

    if (move->moveCloseFunction != NULL)
    {
        move->moveCloseFunction(team,move);
//        aiplayerLog((aiIndex, "%x Deleting Move in aitDeleteCurrentMove", team));
    }

    listDeleteNode(&move->listNode);

    team->curMove = (AITeamMove *)listGetStructOfNode(nextnode);
}


/*-----------------------------------------------------------------------------
    Name        : aitDeleteMovesUntilMoveType
    Description : Deletes all moves starting at current move until a move of type "type" is found
    Inputs      : team - the team to delete the moves from, type - the movetype to stop at
    Outputs     : Deletes a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aitDeleteMovesUntilMoveType(AITeam *team, AIMoveTypes type)
{
    AITeamMove *move = team->curMove;
    Node *nextnode = move->listNode.next;

    while ((move->type != type) && (nextnode != NULL))
    {
        listDeleteNode(&move->listNode);
        move     = (AITeamMove *)listGetStructOfNode(nextnode);
        nextnode = move->listNode.next;
    }

    if (move->type != type)
    {
        //nextnode == NULL
        team->curMove = NULL;
    }
    else
    {
        team->curMove = move;
    }
}



void aitDeleteAllTeamMoves(AITeam *team)
{
    Node *node;
    Node *nextnode;
    AITeamMove *move;

    node = team->moves.head;
    while (node != NULL)
    {
        move = (AITeamMove *)listGetStructOfNode(node);
        nextnode = node->next;

        if (move->moveCloseFunction != NULL)
        {
            move->moveCloseFunction(team,move);
//            aiplayerLog((aiIndex, "%x Deleting Move in aitDeleteAllTeamMoves", team));
        }

        listDeleteNode(node);
        node = nextnode;
    }
    dbgAssert(team->moves.num == 0);

    team->curMove = NULL;
}


void aitSpecialDefenseCoopTeamDiedCB(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;

    if (team->shipList.selection->numShips)
        team->cooperatingTeam = aitFindGoodCoopTeam(team->shipList.selection->ShipPtr[0]->shiptype);
    else
        team->cooperatingTeam = NULL;

    if (team->cooperatingTeam)
    {
        if (aitTeamShipTypeIs(CloakGenerator, team))
        {
            bitSet(team->cooperatingTeam->teamFlags, TEAM_CloakCoop);
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


void aitTakeoutMothershipGuardCoopTeamDiedCB(AITeam *team)
{
    //small chance of going bezerk
    //large chance of going to find another big ship and guarding it.
}

void GenericCooperatingTeamDiedCB(AITeam *team)
{
    team->cooperatingTeam = NULL;
}

/*=============================================================================
    Some Save Game here
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void PreFixCooperatingTeamDiedCB(AITeam *team)
{
    if (team->cooperatingTeamDiedCB == aitSpecialDefenseCoopTeamDiedCB)
    {
        team->cooperatingTeamDiedCB = 1;
    }
    else if (team->cooperatingTeamDiedCB == GenericCooperatingTeamDiedCB)
    {
        team->cooperatingTeamDiedCB = 2;
    }
    else
    {
        team->cooperatingTeamDiedCB = 0;
    }
}

void FixCooperatingTeamDiedCB(AITeam *team)
{
    switch ((sdword)team->cooperatingTeamDiedCB)
    {
        case 1:
            team->cooperatingTeamDiedCB = aitSpecialDefenseCoopTeamDiedCB;
            break;

        case 2:
            team->cooperatingTeamDiedCB = GenericCooperatingTeamDiedCB;
            break;

        case 0:
        default:
            team->cooperatingTeamDiedCB = NULL;
            break;
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*=============================================================================
    End of Some Save Game stuff
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : aitDestroy
    Description : Destroys a team instance
    Inputs      : aiplayer - the player the team belongs to
                  team - the team to destroy
    Outputs     : Deallocates memory being used by the team, modifies a few structures
    Return      : void
----------------------------------------------------------------------------*/
void aitDestroy(struct AIPlayer *aiplayer, AITeam *team, bool removeAllReferencesToTeam)
{
    sdword i = 0;
    sdword foundteam = -1;

    aiplayerLog((aiIndex, "%x destroying team", team));

    //find the array slot for the current team
    for (i = 0; i < aiplayer->teamsUsed; i++)
    {
        if (aiplayer->teams[i] == team)
        {
            foundteam = i;
        }
        else
        {
            if (aiplayer->teams[i]->cooperatingTeam == team)
            {
                if (aiplayer->teams[i]->cooperatingTeamDiedCB)
                {
                    aiplayer->teams[i]->cooperatingTeamDiedCB(aiplayer->teams[i]);
                }
                aiplayer->teams[i]->cooperatingTeam = NULL;
            }
        }
    }

    //make sure we found the array slot
    dbgAssert(foundteam != -1);

     //if there are still ship in the team, put them back in the newships list
    if (removeAllReferencesToTeam && team->shipList.selection && team->shipList.selection->numShips)
    {
          while (team->shipList.selection->numShips)
          {
               growSelectAddShipNoDuplication(&aiplayer->newships, team->shipList.selection->ShipPtr[0]);
                growSelectRemoveShipIndex(&team->shipList, 0);
          }
     }

    //delete the team
    growSelectClose(&team->shipList);

    aitDeleteAllTeamMoves(team);

    aitMsgQueueFree(team);

    if (team->TeamDiedCB)
        team->TeamDiedCB(team);

    aifTeamDied(aiplayer, team, removeAllReferencesToTeam);

    //insert a valid team into the now empty slot
    if (aiplayer->teamsUsed > 1)
    {
        aiplayer->teams[foundteam] = aiplayer->teams[aiplayer->teamsUsed - 1];
    }

    if (singlePlayerGame)
    {
        kasTeamDied(team);
    }

    memFree(team);

    aiplayer->teamsUsed--;
}


/*-----------------------------------------------------------------------------
    Name        : aitInit
    Description : Initializes the team related structures in the aiplayer structure
    Inputs      : aiplayer - the computer player to do the initializing in
    Outputs     : sets some pointers, sets some variables to zero.  You know, the usual stuff
    Return      : void
----------------------------------------------------------------------------*/
void aitInit(struct AIPlayer *aiplayer)
{
    aiplayer->teams          = memAlloc(sizeof(AITeam*)*AITEAM_ALLOC_INITIAL, "aiteamlist", 0);
    aiplayer->teamsAllocated = AITEAM_ALLOC_INITIAL;
    aiplayer->teamsUsed      = 0;
}



/*-----------------------------------------------------------------------------
    Name        : aitClose
    Description : Removes all references of teams from the game.
                  Full deallocation type thingy
    Inputs      : aiplayer - the player who's teams this function annihilate
    Outputs     : Deallocates lots and lots of memory
    Return      : void
----------------------------------------------------------------------------*/
void aitClose(struct AIPlayer *aiplayer)
{
    AITeam *team;
    udword i;

    //deallocate any team that's been created
    for (i = 0; i < aiplayer->teamsUsed; i++)
    {
        team = aiplayer->teams[i];

        growSelectClose(&team->shipList);   //deallocate the team's ships
        listDeleteAll(&team->moves);        //deallocate the team's moves
        memFree(team);                      //deallocate the team structure
    }
    memFree(aiplayer->teams);
}


void aitShipDied(struct AIPlayer *aiplayer,ShipPtr ship)
{
    udword i;
    AITeam *team;
    Node *node;
    AITeamMove *move;

//    if ((ship->playerowner == aiplayer->player) || (ship->attributes & ATTRIBUTES_Defector))  // do all checks for defector
//    {
        // go through all teams & ships
        for (i = 0; i < aiplayer->teamsUsed; i++)
        {
            team = aiplayer->teams[i];
            if (growSelectRemoveShip(&team->shipList, ship))
            {
                //later maybe add callback checking
                //later if team is dead, what do we do?
                //Note: aievents has a teamdied handler as well for each move

                //recalculate the strength and value of the team
                if ((aiplayer->numLeaders) && (bitTest(ship->attributes, ATTRIBUTES_TeamLeader)) &&
                    (team->teamDifficultyLevel > 0))
                {
                    if (!aitCheckForLeaderAndMoveToFront(team))
                    {
                        team->teamDifficultyLevel--;
                    }
                    aiplayer->numLeaders--;
                }
                aitSetTeamStrengthValue(team);
//                aiplayerLog((aiplayer->player->playerIndex,"Dead ship removed from team %x shiplist", team));
//                break;      // assuming a ship can only be in 1 team
            }
        }
//    }

    // second pass: go through all team's moves:
    for (i = 0; i < aiplayer->teamsUsed; i++)
    {
        team = aiplayer->teams[i];

        node = team->moves.head;
        while (node != NULL)
        {
            move = (AITeamMove *)listGetStructOfNode(node);

            if (move->moveShipDiedFunction)
            {
                move->moveShipDiedFunction(team,move,ship);
            }

            node = node->next;
        }
    }
}


void aitResourceDied(struct AIPlayer *aiplayer, Resource *resource)
{
    udword i;
    AITeam *team;
    Node *node;
    AITeamMove *move;

    for (i=0; i < aiplayer->teamsUsed; i++)
    {
        team = aiplayer->teams[i];

        node = team->moves.head;
        while (node != NULL)
        {
            move = (AITeamMove *)listGetStructOfNode(node);

            if (move->type == MOVE_RESVOLUME)
            {
                move->moveResourceDiedFunction(team, move, resource);
            }
            node = node->next;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : aitCheckForLeaderAndMoveToFront
    Description : Checks to see if the team has a leader, and moves it to the front of the selection of there is one
    Inputs      : team - the team to check
    Outputs     : Move some ships around the selection
    Return      : TRUE if a leader is found
----------------------------------------------------------------------------*/
bool aitCheckForLeaderAndMoveToFront(AITeam *team)
{
    ShipPtr ship;
    udword i;

    for (i=0;i<team->shipList.selection->numShips;i++)
    {
        ship = team->shipList.selection->ShipPtr[i];

        if (bitTest(ship->attributes, ATTRIBUTES_TeamLeader))
        {
            //found a leader, now move him to the front of the selection
            team->shipList.selection->ShipPtr[i] = team->shipList.selection->ShipPtr[0];
            team->shipList.selection->ShipPtr[0] = ship;
            return TRUE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aitCheckAmIBeingWatched
    Description : Checks to see if the player is watching this team
    Inputs      : team - the team being checked, sel - the selection being checked explicitly
    Outputs     :
    Return      : TRUE if the team is being watched
----------------------------------------------------------------------------*/
bool aitCheckAmIBeingWatched(AITeam *team, SelectCommand *sel)
{
    if (bitTest(team->teamFlags, TEAM_AmIBeingWatched))
    {
        bitClear(team->teamFlags, TEAM_AmIBeingWatched);
        if (sel->numShips && bitTest(sel->ShipPtr[0]->renderedLODs, 0x7))
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aitSetAmIBeingWatched
    Description : Sets up the team for  checking if it's being watched
    Inputs      : team - the team being setup,
                  sel - the selection being checked explicitly
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aitSetAmIBeingWatched(AITeam *team, SelectCommand *sel)
{
    if (sel->numShips)
    {
        //flag the team as one checking if it's being watched
        bitSet(team->teamFlags, TEAM_AmIBeingWatched);

        //clear the RenderedLODs bitfield in one of the selections ships.
        bitClear(sel->ShipPtr[0]->renderedLODs, 0x7);
    }
}



/*-----------------------------------------------------------------------------
    Name        : aitAddShip
    Description : Adds a ship to the team
    Inputs      : team - the team to add the ship to,
                  ship - the ship to add
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aitAddShip(AITeam *team, ShipPtr ship)
{
    growSelectAddShip(&team->shipList,ship);
    aitSetTeamHomoHetero(team);
    aitSetTeamSpecialFlags(team);
    aitSetTeamStrengthValue(team);
    team->newships++;

    //leader code
    //if there is a leader in the computer player's list of ships
    if (aiCurrentAIPlayer->numLeaders)
    {
        //if this new ship is a leader, increase the level of difficulty
        //of the team
        if (bitTest(ship->attributes, ATTRIBUTES_TeamLeader) &&
            (team->teamDifficultyLevel != TEAM_ADVANCED))
        {
           team->teamDifficultyLevel++;
        }

        //make sure that the leader is actually in front of the formation
        aitCheckForLeaderAndMoveToFront(team);
    }
}

void aitCheckShips(AITeam *team, ShipPtr ship)
{
    udword i;

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        if ((aiCurrentAIPlayer->teams[i] != team) && ShipInSelection(aiCurrentAIPlayer->teams[i]->shipList.selection, ship))
        {
            dbgAssert(FALSE);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitMoveSwarmShipDefenseToAttack
    Description : Special function for swarm teams - moves a ship from the defense swarm to the offense swarm
    Inputs      : attackSwarm, defenseSwarm, ship
    Outputs     : Removes the ship from defense swarm and adds it to the attacks swarm
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveSwarmShipDefenseToAttack(AITeam *attackSwarm, AITeam *defenseSwarm, ShipPtr ship)
{
    dbgAssert(ship->playerowner->playerIndex);
    dbgAssert(ship->objtype == OBJ_ShipType);

    growSelectAddShip(&attackSwarm->shipList, ship);

    //add the ship to the attack team's newSwarmers structure
    growSelectAddShip(&attackSwarm->curMove->params.swarmatt.newSwarmers, ship);

    //remove the ship from the current team
    growSelectRemoveShip(&defenseSwarm->shipList, ship);
    growSelectRemoveShip(&defenseSwarm->curMove->params.swarmdef.newSwarmers, ship);
    clRemoveShipFromSelection(defenseSwarm->curMove->params.swarmdef.guarding, ship);
}


/*-----------------------------------------------------------------------------
    Name        : aitMoveAllSwarmShipsAttackToDefense
    Description : Moves every swarm ship in the attack swarm to the defense swarm
    Inputs      : attackSwarm, defenseSwarm
    Outputs     : Removes all the ships in the attack swarm and puts them into the defense swarm
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveAllSwarmShipsAttackToDefense(AITeam *attackSwarm, AITeam *defenseSwarm)
{
    ShipPtr ship;

    while(attackSwarm->shipList.selection->numShips)
    {
        ship = attackSwarm->shipList.selection->ShipPtr[0];

        dbgAssert(ship->playerowner->playerIndex);
        dbgAssert(ship->objtype == OBJ_ShipType);

        //add the ship to the defense swarm's shiplist
        growSelectAddShip(&defenseSwarm->shipList, ship);

        //add the ship to the defense swarm's newSwarmer structure
        growSelectAddShip(&defenseSwarm->curMove->params.swarmdef.newSwarmers, ship);

        //remove the ship from the current team
        growSelectRemoveShip(&attackSwarm->shipList, ship);
    }

    if (attackSwarm->curMove->params.swarmatt.newSwarmers.selection->numShips)
    {
        attackSwarm->curMove->params.swarmatt.newSwarmers.selection->numShips = 0;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aitMoveAllSwarmShipsDefenseToAttack
    Description : Moves every swarm ship in the attack swarm to the defense swarm
    Inputs      : attackSwarm, defenseSwarm
    Outputs     : Removes all the ships in the attack swarm and puts them into the defense swarm
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveAllSwarmShipsDefenseToAttack(AITeam *defenseSwarm, AITeam *attackSwarm)
{
    ShipPtr ship;

    while (defenseSwarm->shipList.selection->numShips)
    {
        ship = defenseSwarm->shipList.selection->ShipPtr[0];

        dbgAssert(ship->playerowner->playerIndex);
        dbgAssert(ship->objtype == OBJ_ShipType);

        //add the ship to the defense swarm's shiplist
        growSelectAddShip(&attackSwarm->shipList, ship);

        //add the ship to the defense swarm's newSwarmer structure
        growSelectAddShip(&attackSwarm->curMove->params.swarmatt.newSwarmers, ship);

        //remove the ship from the current team
        growSelectRemoveShip(&defenseSwarm->shipList, ship);
    }

    if (defenseSwarm->curMove->params.swarmdef.newSwarmers.selection->numShips)
    {
        defenseSwarm->curMove->params.swarmdef.newSwarmers.selection->numShips = 0;
    }

    if ((defenseSwarm->curMove->params.swarmdef.guarding) &&
        (defenseSwarm->curMove->params.swarmdef.guarding->numShips))
    {
        defenseSwarm->curMove->params.swarmdef.guarding->numShips = 0;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aitMoveAllSwarmShipsDefense
    Description : Moves all the swarmers from one defense team to another
    Inputs      : destTeam - the destination team, sourceTeam - the source team
    Outputs     : Moves ships from team to team
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveAllSwarmShipsDefense(AITeam *destTeam, AITeam *sourceTeam)
{
    ShipPtr ship;

    while (sourceTeam->shipList.selection->numShips)
    {
        ship = sourceTeam->shipList.selection->ShipPtr[0];

        dbgAssert(ship->playerowner->playerIndex);
        dbgAssert(ship->objtype == OBJ_ShipType);

        growSelectAddShip(&destTeam->shipList, ship);
        growSelectAddShip(&destTeam->curMove->params.swarmdef.newSwarmers, ship);

        growSelectRemoveShip(&sourceTeam->shipList, ship);
    }

    sourceTeam->curMove->params.swarmdef.newSwarmers.selection->numShips = 0;
    sourceTeam->curMove->params.swarmdef.guarding->numShips = 0;
}


/*-----------------------------------------------------------------------------
    Name        : aitMoveAllSwarmShipsAttack
    Description : Moves all the swarmers from one attack team to another
    Inputs      : destTeam - the destination team, sourceTeam - the source team
    Outputs     : Moves ships from team to team
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveAllSwarmShipsAttack(AITeam *destTeam, AITeam *sourceTeam)
{
    ShipPtr ship;

    while (sourceTeam->shipList.selection->numShips)
    {
        ship = sourceTeam->shipList.selection->ShipPtr[0];

        dbgAssert(ship->playerowner->playerIndex);
        dbgAssert(ship->objtype == OBJ_ShipType);

        growSelectAddShip(&destTeam->shipList, ship);
        growSelectAddShip(&destTeam->curMove->params.swarmatt.newSwarmers, ship);

        growSelectRemoveShip(&sourceTeam->shipList, ship);
    }

    sourceTeam->curMove->params.swarmatt.newSwarmers.selection->numShips = 0;
}


/*-----------------------------------------------------------------------------
    Name        : aitFindNewPod
    Description : Finds a new swarm pod for the defense team
    Inputs      : defenseTeam - the swarmer defense team
    Outputs     :
    Return      : a pod team that has a pod and isn't the defense team's pod
----------------------------------------------------------------------------*/
AITeam *aitFindNewPod(AITeam *defenseTeam)
{
    udword i, lowestcnt, swarmcnt;
    AITeam *podteam, *lowestteam;

    for (i=0; i<aiCurrentAIPlayer->numSupportTeams;i++)
    {   // first try to find pods with no swarmers
        podteam = aiCurrentAIPlayer->supportTeam[i];

        if ((podteam->shipList.selection->numShips) &&
            (aiCurrentAIPlayer->supportTeam[i]->cooperatingTeam->cooperatingTeam != defenseTeam) &&
            ((!podteam->cooperatingTeam->shipList.selection->numShips) &&
             (!podteam->cooperatingTeam->cooperatingTeam->shipList.selection->numShips)))
        {
            return podteam;
        }
    }

    lowestcnt  = 15000;
    lowestteam = NULL;
    for (i=0; i<aiCurrentAIPlayer->numSupportTeams;i++)
    {   // next try to find pods with least swarmers
        podteam  = aiCurrentAIPlayer->supportTeam[i];
        swarmcnt = podteam->cooperatingTeam->shipList.selection->numShips +
                   podteam->cooperatingTeam->cooperatingTeam->shipList.selection->numShips;

        if ((podteam->shipList.selection->numShips) &&
            (aiCurrentAIPlayer->supportTeam[i]->cooperatingTeam->cooperatingTeam != defenseTeam) &&
            (swarmcnt < lowestcnt))
        {
            lowestteam = podteam;
        }
    }
    return lowestteam;
}


/*-----------------------------------------------------------------------------
    Name        : aitMoveSwarmersToNewPod
    Description : Moves the defense and attack swarmers to the new pod's defense and attack teams
    Inputs      : defenseTeam - the defense team, podTeam - the pod team
    Outputs     : Moves ships from team to team
    Return      : void
----------------------------------------------------------------------------*/
void aitMoveSwarmersToNewPod(AITeam *defenseTeam, AITeam *podTeam)
{
    AITeam *oldAttackTeam  = defenseTeam->cooperatingTeam;
    AITeam *newAttackTeam  = podTeam->cooperatingTeam;
    AITeam *newDefenseTeam = newAttackTeam->cooperatingTeam;

    aitMoveAllSwarmShipsDefense(newDefenseTeam, defenseTeam);
    aitMoveAllSwarmShipsAttack(newAttackTeam, oldAttackTeam);
}

/*-----------------------------------------------------------------------------
    Name        : aitAllDefenseSwarmersFull
    Description : Returns true if all defense swarmers have filled their tanks
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
bool aitAllDefenseSwarmersFull(void)
{
    udword i;

    for (i=0;i<aiCurrentAIPlayer->numGuardTeams;i++)
    {
        //later figure out what's going on here....
        if ((aiCurrentAIPlayer->guardTeams[i]->curMove) &&
            (aiCurrentAIPlayer->guardTeams[i]->curMove->params.swarmdef.full_refuel))
        {
            return FALSE;
        }
    }
    return TRUE;
}




//
// returns TRUE if removed ship
//
bool aitRemoveShip(AITeam *team, ShipPtr ship)
{
    return growSelectRemoveShip(&team->shipList,ship);
}


/*=============================================================================
    Special Team Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aitSetTeamSpecialFlags
    Description : Analyses the team and sets the team's flags -
                  TEAM_CloakFighters, TEAM_CloakGenerator, TEAM_GravWellGenerator
    Inputs      : team - the team
    Outputs     : Sets some flags
    Return      : void
----------------------------------------------------------------------------*/
void aitSetTeamSpecialFlags(AITeam *team)
{
    //set special flags
    if (!team->shipList.selection->numShips)
    {
        return;
    }

    if ((!bitTest(team->teamFlags, TEAM_CloakFighters)) &&
        (team->shipList.selection->ShipPtr[(team->shipList.selection->numShips-1)]->shiptype == CloakedFighter))
    {
        bitSet(team->teamFlags, TEAM_CloakFighters);
    }
    else if ((!bitTest(team->teamFlags, TEAM_CloakGenerator)) &&
             (team->shipList.selection->ShipPtr[(team->shipList.selection->numShips-1)]->shiptype == CloakGenerator))
    {
        bitSet(team->teamFlags, TEAM_CloakGenerator);
    }
/*    else if ((!bitTest(team->teamFlags, TEAM_GravWellGenerator)) &&
             (team->shipList.selection->ShipPtr[(team->shipList.selection->numShips-1)]->shiptype == GravWellGenerator))
    {
        bitSet(team->teamFlags, TEAM_GravWellGenerator);
    }
    */
}


/*-----------------------------------------------------------------------------
    Name        : aitProcessSpecialTeamCloakFighters
    Description : Processes cloak fighters
    Inputs      : team, curMove
    Outputs     : May do some stuff... it may even create some other stuff as well
    Return      : void
----------------------------------------------------------------------------*/
void aitProcessSpecialTeamCloakFighters(AITeam *team, AITeamMove *curMove)
{
    switch (curMove->type)
    {
        case MOVE_MOVETEAM:
        case MOVE_MOVETEAMINDEX:
            //maybe have a special function here to determine if the
            //move is dangerous or not
        case MOVE_SHIPRECON:
        case MOVE_ATTACK:
        case MOVE_ADVANCEDATTACK:
            aiuWrapSpecial(team->shipList.selection, NULL);

            bitClear(team->teamFlags, TEAM_CloakFighters);
            bitSet(team->teamFlags, TEAM_CloakedFighters);
            break;
        default:
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitProcessSpecialTeamCloakedFighters
    Description : Processes cloak fighters that are already cloaked
    Inputs      : team, curMove
    Outputs     : May do some stuff... it may even create some other stuff as well
    Return      : void
----------------------------------------------------------------------------*/
void aitProcessSpecialTeamCloakedFighters(AITeam *team, AITeamMove *curMove)
{
    switch (curMove->type)
    {
        case MOVE_TEMPGUARD:
            aiuWrapSpecial(team->shipList.selection, NULL);

            bitClear(team->teamFlags, TEAM_CloakedFighters);
            bitSet(team->teamFlags,  TEAM_CloakFighters);
            break;
        default:
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitProcessSpecialTeamCloakGenerator
    Description : Cloaks the cloak generator when appropriate
    Inputs      : team, curCoopMove - the move of the team the cloak generator is cloaking
    Outputs     : Stuff
    Return      : void
----------------------------------------------------------------------------*/
void aitProcessSpecialTeamCloakGenerator(AITeam *team, AITeamMove *curCoopMove)
{
    switch (curCoopMove->type)
    {
        case MOVE_MOVETEAM:
        case MOVE_MOVETEAMINDEX:
            //if the ship is moving near the mothership, conserve energy
            //and not cloak
            if (aiuPointWithinSphereOfInfluence(curCoopMove->params.move.destination))
            {
                break;
            }
        case MOVE_SHIPRECON:
        case MOVE_CONTROLRESOURCES:
        case MOVE_ATTACK:
        case MOVE_ADVANCEDATTACK:
            //later - before doing this, ensure the cloak generator is near
            //a capital ship to cloak, otherwise we're wasting energy
            aiuWrapSpecial(team->shipList.selection, NULL);

            bitClear(team->teamFlags, TEAM_CloakGenerator);
            bitSet(team->teamFlags, TEAM_CloakedGenerator);
            break;
        default:
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitProcessSpecialTeamCloakingGenerator
    Description : decloaks the cloak generator when appropriate
    Inputs      : team, curCoopMove - the move of the team the cloak generator is cloaking
    Outputs     : Stuff
    Return      : void
----------------------------------------------------------------------------*/
void aitProcessSpecialTeamCloakingGenerator(AITeam *team, AITeamMove *curCoopMove)
{
    switch (curCoopMove->type)
    {
        case MOVE_MOVETEAM:
        case MOVE_MOVETEAMINDEX:
            //if the ship is moving near the mothership, conserve energy
            //and not cloak
            if (!aiuPointWithinSphereOfInfluence(curCoopMove->params.move.destination))
            {
                break;
            }
        case MOVE_TEMPGUARD:
            aiuWrapSpecial(team->shipList.selection, NULL);

            bitClear(team->teamFlags, TEAM_CloakedGenerator);
            bitSet(team->teamFlags,  TEAM_CloakGenerator);
            break;
        default:
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitOptimizeGravwellGenerator
    Description : Makes sure the Gravwell generator team is guarding the right kind of team and adds some events
    Inputs      : team - the gravwell generator team
    Outputs     : Adds some events to the team move structure
    Return      : void
----------------------------------------------------------------------------*/
void aitOptimizeGravWellGenerator(AITeam *team)
{
    AITeamMove *guardMove;
    AITeam *coopTeam = team->cooperatingTeam;

    //set the guard move event to turn on wheneven an enemy fighter is nearby
    //and off when there isn't or when goodguy fighter is nearby (within range)

    if (!team->shipList.selection->numShips)
    {
        return;
    }

    if (coopTeam->shipList.selection->numShips)
    {
        if (!(aitTeamShipClassIs(CLASS_Frigate, coopTeam) || aitTeamShipClassIs(CLASS_Carrier, coopTeam) ||
              aitTeamShipClassIs(CLASS_Destroyer, coopTeam) || aitTeamShipClassIs(CLASS_HeavyCruiser, coopTeam)))
        {
//            dbgAssert(TRUE);
            //find a new team to guard
        }
    }
    else
    {
        //I dunno...  let's see how things work out...
    }


    guardMove = aitFindMoveOfType(team, MOVE_GUARDCOOPTEAM);

    dbgAssert(guardMove!=NULL);

    aieHandlerSetEnemyNearby(guardMove,
                             (((GravWellGeneratorStatics *) ((ShipStaticInfo *)(team->shipList.selection->ShipPtr[0]->staticinfo))->custstatinfo)->GravWellRadius)*0.8,
                             FALSE, aihGravWellEnemyNearbyHandler);
    bitSet(team->teamFlags, TEAM_SpecialOptimal);

}


/*-----------------------------------------------------------------------------
    Name        : aitOptimizeCloakGenerator
    Description : Makes sure the Cloak generator team is guarding the right kind of team and adds some events
    Inputs      : team - the cloak generator team
    Outputs     : Adds some events to the team move structure
    Return      : void
----------------------------------------------------------------------------*/
void aitOptimizeCloakGenerator(AITeam *team)
{
//    AITeamMove *guardMove;
    AITeam *coopTeam = team->cooperatingTeam;

    //make sure the team's cooperating team is slow
    //set the guard move event to turn on wheneven an enemy fighter is nearby
    //and off when there isn't or when goodguy fighter is nearby (within range)

    if (!(aitTeamShipClassIs(CLASS_Frigate, coopTeam) || aitTeamShipTypeIs(ResourceCollector, coopTeam)))
    {
        dbgAssert(TRUE);
        //find a new team to guard
    }

//    guardMove = aitFindMoveOfType(MOVE_GUARDSHIPS);
//    aieHandlerSetEnemyNearby(guardMove,
//                             (((GravWellGeneratorStatics *) ((ShipStaticInfo *)(team->shipList.selection->ShipPtr[0]->staticinfo))->custstatinfo)->GravWellRadius)*0.8,
//                             FALSE, aihGravWellEnemyNearbyHandler);

}



/*-----------------------------------------------------------------------------
    Name        : aitProcessSpecialTeam
    Description : Processes the special teams, i.e. cloaking team
    Inputs      : team, curMove
    Outputs     : May do some stuff...  May even create moves
    Return      : void
----------------------------------------------------------------------------*/
void aitProcessSpecialTeam(AITeam *team, AITeamMove *curMove)
{
    if ((aiuTeamFeatureEnabled(team, AIT_CLOAKING)) &&
        (bitTest(team->teamFlags, TEAM_CloakFighters)))
    {
        aitProcessSpecialTeamCloakFighters(team, curMove);
    }
    else if ((aiuTeamFeatureEnabled(team, AIT_CLOAKING)) &&
             (bitTest(team->teamFlags, TEAM_CloakedFighters)))
    {
        aitProcessSpecialTeamCloakedFighters(team, curMove);
    }

    //later pull all this crap out completely
    else if ((aiuTeamFeatureEnabled(team, AIT_CLOAKING)) &&
             (bitTest(team->teamFlags, TEAM_CloakGenerator)))
    {
//        if (!bitTest(team->teamFlags, TEAM_SpecialOptimal))
//        {
//            aitOptimizeCloakGenerator(team);
//        }
        if (team->cooperatingTeam)
        {
            aitProcessSpecialTeamCloakGenerator(team, team->cooperatingTeam->curMove);
        }
    }
    else if ((aiuTeamFeatureEnabled(team, AIT_CLOAKING)) &&
             (bitTest(team->teamFlags, TEAM_CloakedGenerator)))
    {
        if (team->cooperatingTeam)
        {
            aitProcessSpecialTeamCloakingGenerator(team, team->cooperatingTeam->curMove);
        }
    }
    else if ((aiuTeamFeatureEnabled(team, AIT_CLOAKING)) &&
             (bitTest(team->teamFlags, TEAM_CloakCoop)))
    {
        if (team->cooperatingTeam)
        {
            if (bitTest(team->cooperatingTeam->teamFlags, TEAM_CloakGenerator))
            {
                aitProcessSpecialTeamCloakGenerator(team->cooperatingTeam, team->curMove);
            }
            else if (bitTest(team->cooperatingTeam->teamFlags, TEAM_CloakedGenerator))
            {
                aitProcessSpecialTeamCloakingGenerator(team->cooperatingTeam, team->curMove);
            }
            else
            {
                dbgAssert(TRUE);
            }
        }
    }

/*    else if ((aiuTeamFeatureEnabled(team, AIT_GRAVWELL)) &&
             (bitTest(team->teamFlags, TEAM_GravWellGenerator)))
    {
        if (!bitTest(team->teamFlags, TEAM_SpecialOptimal))
        {
            aitOptimizeGravWellGenerator(team);
        }
//        aitProcessSpecialTeamGravWellGenerator(team, curMove);
    }
    else if ((aiuTeamFeatureEnabled(team, AIT_GRAVWELL)) &&
             (bitTest(team->teamFlags, TEAM_GravWellGenerating)))
    {
//        aitProcessSpecialTeamGravWellGenerating(team, curMove);
    }
 */
}


/*=============================================================================
    General Utility Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aitAddmoveBeforeAndMakeCurrent
    Description : Adds a move before the current one and makes the new move current
    Inputs      : team - the team to add the new move to,
                  newMove - the new move,
                  thisMove - the current move
    Outputs     : Some linked list manipulation and may change the formation of
                  the team
    Return      : void
----------------------------------------------------------------------------*/
void aitAddmoveBeforeAndMakeCurrentNoSpecial(AITeam *team, AITeamMove *newMove, AITeamMove *thisMove)
{
    listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);
    team->curMove = newMove;
}

void aitAddmoveBeforeAndMakeCurrent(AITeam *team, AITeamMove *newMove, AITeamMove *thisMove)
{
    //if the tactics setting of the new move is different from the
    //old move, set tactics
    if ((newMove->tactics != thisMove->tactics)&&(team->shipList.selection->numShips))
    {
        aiuWrapSetTactics(team->shipList.selection,newMove->tactics);
    }

    if (newMove->formation == SAME_FORMATION)
    {
//        if (thisMove->formation < NO_FORMATION)
//        {
            newMove->formation = thisMove->formation;
//        }
//        else
//        {
//            newMove->formation = AIT_DEFAULT_FORMATION;
//        }
    }
    else if ((newMove->formation != NO_FORMATION) &&
             (newMove->formation != thisMove->formation) &&
             (team->shipList.selection->numShips > 1))
    {
        aiuWrapFormation(team->shipList.selection,newMove->formation);
    }
    aitAddmoveBeforeAndMakeCurrentNoSpecial(team, newMove, thisMove);
}


/*-----------------------------------------------------------------------------
    Name        : aitFindNumTeamsWithFlag
    Description : Returns the number of teams with a certain flag set
    Inputs      : flag - the flag referred to in description
    Outputs     : none
    Return      : number of teams with the flag set
----------------------------------------------------------------------------*/
udword aitFindNumTeamsWithFlag(udword flag)
{
    udword i, num_teams_with_flag = 0;

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        if (bitTest(aiCurrentAIPlayer->teams[i]->teamFlags,flag))
        {
            num_teams_with_flag++;
        }
    }
    return num_teams_with_flag;
}


/*-----------------------------------------------------------------------------
    Name        : aitFindNextTeamWithFlag
    Description : Looks through the team list (starting with the
                  passed team) for the next team with the passed flag set
    Inputs      : team - the starting team, flag - the flag to look for
    Outputs     :
    Return      : the next team
----------------------------------------------------------------------------*/
AITeam *aitFindNextTeamWithFlag(AITeam *team, udword flag)
{
    udword i=0;

    //find "team"
    if (team)
    {
        for (i=0;(i<aiCurrentAIPlayer->teamsUsed) && (aiCurrentAIPlayer->teams[i] != team); i++);
        i++;
    }

    for (;(i<aiCurrentAIPlayer->teamsUsed) && (!bitTest(aiCurrentAIPlayer->teams[i]->teamFlags, flag)); i++);

    if (i==aiCurrentAIPlayer->teamsUsed)
    {
        return NULL;
    }
    else
    {
        return aiCurrentAIPlayer->teams[i];
    }

}


/*-----------------------------------------------------------------------------
    Name        : aitCountTeamsWaitingForShips
    Description : Counts the number of teams with the Getships and FancyGetships move
    Inputs      :
    Outputs     :
    Return      : the number of teams waiting for ships
----------------------------------------------------------------------------*/
sdword aitCountTeamsWaitingForShips(TeamType type)
{
    sdword i, num_teams_waiting = 0;
    AITeam *team;

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        //if type is AnyTeam or the team's type is type
        //and
        //if the team doesn't have a move
        //or if the team's move is "MOVE_GETSHIPS" or "MOVE_FANCYGETSHIPS"
        //then increment num_teams_waiting
        if (((type == AnyTeam) ||
             (team->teamType == type)) &&
            ((!team->curMove)||
             (team->curMove->type == MOVE_GETSHIPS)||
             (team->curMove->type == MOVE_FANCYGETSHIPS)))
        {
            num_teams_waiting++;
        }
    }
    return num_teams_waiting;
}




bool aitTeamIsDone(AITeam *team)
{
    if ((team->curMove == NULL) || (team->curMove->type == MOVE_DONE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aitTeamIsIdle
    Description : Returns TRUE if the team is waiting for ships, done or sitting
                  around doing nothing
    Inputs      : team - the team to check
    Outputs     :
    Return      : TRUE if team is being a slacker
----------------------------------------------------------------------------*/
bool aitTeamIsIdle(AITeam *team)
{
    if ((team->curMove == NULL) ||
        (team->curMove->type == MOVE_DONE) ||
        (team->curMove->type == MOVE_GETSHIPS) ||
        (team->curMove->type == MOVE_FANCYGETSHIPS))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsGuarding
    Description : Returns true if the team is guarding
    Inputs      : team - the team to check
    Outputs     :
    Return      : TRUE if the team is guarding
----------------------------------------------------------------------------*/
bool aitTeamIsGuarding(AITeam *team)
{
    AIMoveTypes moveType;

    if (team->curMove == NULL)
        return FALSE;

    moveType = team->curMove->type;

    if ((moveType == MOVE_GUARDSHIPS) ||
        (moveType == MOVE_GUARDCOOPTEAM) ||
        (moveType == MOVE_PATROLMOVE) ||
        (moveType == MOVE_ACTIVEPATROL))
    {
        return TRUE;
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsAttacking
    Description : Returns TRUE if the team is attacking
    Inputs      : team - the team to check
    Outputs     :
    Return      : TRUE if the team is attacking
----------------------------------------------------------------------------*/
bool aitTeamIsAttacking(AITeam *team)
{
    AIMoveTypes moveType;

    if (team->curMove == NULL)
        return FALSE;

    moveType = team->curMove->type;

    if ((moveType == MOVE_ATTACK) ||
        (moveType == MOVE_ADVANCEDATTACK) ||
        (moveType == MOVE_FLANKATTACK))
    {
        return TRUE;
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aitTeamIsntDefendingMothership
    Description : Returns TRUE if the team is doing anything other than defending the mothership
    Inputs      : team - the team to check, enemyships - the ships attacking the mothership
    Outputs     :
    Return      : TRUE if the team isn't defending the mothership
----------------------------------------------------------------------------*/
bool aitTeamIsntDefendingMothership(AITeam *team, SelectCommand *enemyships)
{
    if (team->curMove != NULL)
    {
        if ((team->curMove->type == MOVE_DEFMOSHIP) ||
            (team->curMove->type == MOVE_DOCK))
        {
            return FALSE;
        }
        else if (aitTeamIsAttacking(team))
        {
            if ((team->curMove->params.attack.ships) &&
                TheseShipsAreInSelection(team->curMove->params.attack.ships, enemyships))
            {
                return FALSE;
            }
        }
    }
    else
    {
        //reject teams who have no moves or no ships
        return FALSE;
    }
    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsFinishedMoving
    Description : Checks to see if the team has completed a move
    Inputs      : team - the moving team,
                  destination - where the team is moving to,
                  range - the distance to the destination that determines
                          the end of the move
    Outputs     :
    Return      : TRUE if the team has finished moving
----------------------------------------------------------------------------*/
bool aitTeamIsFinishedMoving(AITeam *team, vector destination, real32 range)
{
    vector current_location;
    real32 avg_size;

    if (!team->shipList.selection->numShips)
    {
        //if the team is dead, it's finished moving
        return TRUE;
    }

    current_location = selCentrePointComputeGeneral((MaxSelection *)team->shipList.selection, &avg_size);

    if ((aiuFindDistanceSquared(current_location, destination) < (range*range)) ||
        (vecIsZero(team->shipList.selection->ShipPtr[0]->moveTo)))
    {
        return TRUE;
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsDoingSpecialOp
    Description : Returns true if the team is preforming a special operation
    Inputs      : team - the team to check
    Outputs     :
    Return      : TRUE if the team is special op'ing
----------------------------------------------------------------------------*/
bool aitTeamIsDoingSpecialOp(AITeam *team)
{
//    dbgAssert(aitTeamShipTypeIs(MinelayerCorvette, team));

    if (team->shipList.selection->numShips &&
        team->shipList.selection->ShipPtr[0]->command &&
        (team->shipList.selection->ShipPtr[0]->command->ordertype.order == COMMAND_SPECIAL))
    {
        return TRUE;
    }
    return FALSE;
}





bool aitAnyTeamOfPlayerAttackingThisShip(struct AIPlayer *aiplayer,Ship *ship)
{
    sdword i;
    Node *node;
    AITeamMove *move;
    AITeam *team;

    for (i = 0; i < aiplayer->teamsUsed; ++i)
    {
        team = aiplayer->teams[i];

        node = team->moves.head;
        while (node != NULL)
        {
            move = (AITeamMove *)listGetStructOfNode(node);

            if ((move->type == MOVE_ATTACK) ||
                (move->type == MOVE_ADVANCEDATTACK) ||
                (move->type == MOVE_FLANKATTACK))
            {
                if (move->params.attack.ships)
                {
                    if (ShipInSelection(move->params.attack.ships,ship))
                    {
                        return TRUE;
                    }
                }
            }

            node = node->next;
        }
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsInRange
    Description : Returns True if the team can make it to the ship within "time"
    Inputs      : team - the team
                  ship - the ship to check if the team is in range
                  time - how much time determine's "in range"
    Outputs     :
    Return      : True if the team is in range
----------------------------------------------------------------------------*/
bool aitTeamIsInRange(AITeam *team, ShipPtr ship, real32 time)
{
    real32 timesq = time*time, distsq, teamvelsq;

    if (!team->shipList.selection->numShips)
    {
        return FALSE;
    }

    if (!aitTeamHomogenous(team))
    {
        teamvelsq = aiuFindSlowestShipMaxSpeed(team->shipList.selection);
    }
    else
    {
        teamvelsq = tacticsGetShipsMaxVelocity(team->shipList.selection->ShipPtr[0]);
    }

    distsq     = aiuFindDistanceSquared(team->shipList.selection->ShipPtr[0]->posinfo.position, ship->posinfo.position);
    teamvelsq *= teamvelsq;

    if ((teamvelsq * timesq) > distsq)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsInMothershipRange
    Description : Returns True if the ship is within a certain range of the mothership
    Inputs      : team - the team to check
    Outputs     :
    Return      : TRUE if the team is in range
----------------------------------------------------------------------------*/
bool aitTeamIsInMothershipRange(AITeam *team)
{
    ShipPtr mothership = team->aiplayerowner->player->PlayerMothership;

    if (!mothership)
    {
        return FALSE;
    }

    if (mothership->shiptype == Mothership)
    {
        return aitTeamIsInRange(team, mothership, 40.0);
    }
    else //it must be a carrier
    {
        return aitTeamIsInRange(team, mothership, 30.0);
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitRecallGuardTeam
    Description : Changes the moves of a guard team so that it returns to it's patrol
    Inputs      : team - the team to recall
    Outputs     : removes a few moves
    Return      : void
----------------------------------------------------------------------------*/
void aitRecallGuardTeam(AITeam *team)
{
    while (aitTeamIsAttacking(team))
    {
        aitDeleteCurrentMove(team);
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitNeedStrikeSupport
    Description : Returns TRUE if there are enough Strike Craft teams to
                  justify building support craft
    Inputs      : minstr - minimum strike craft for TRUE
    Outputs     :
    Return      : TRUE or FALSE (see description)
----------------------------------------------------------------------------*/
bool aitNeedStrikeSupport(udword minstr)
{
    udword tally = 0, i;
    AITeam *team;

    for (i=0; i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        if (aitNumTeamShips(team) &&
            ((aitTeamShipClassIs(CLASS_Fighter, team) ||
             (aitTeamShipClassIs(CLASS_Corvette, team)))))
        {
            if (!aitTeamIsInMothershipRange(team))
            {
                tally += aitNumTeamShips(team);
            }
        }
    }

    if (tally > minstr)
    {
        return TRUE;
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aitMakeTeamSupportShips
    Description : Sets the team to support ships... or something like that
    Inputs      : team - the team to do the supporting, ships - the ships to support
    Outputs     : Duplicates ships selection into the support move structure
    Return      : void
----------------------------------------------------------------------------*/
void aitMakeTeamSupportShips(AITeam *team, SelectCommand *ships)
{
    AITeamMove *move;

    move = team->curMove;

    if (!move)
    {
        return;
    }

    while ((move->type != MOVE_SUPPORT) && (move->type != MOVE_DONE))
    {
        move = listGetStructOfNode(move->listNode.next);
        if (move == NULL)
        {
            return;
        }
    }

    if (move->type == MOVE_DONE)
    {
        return;
    }
    dbgAssert(move->type == MOVE_SUPPORT);

    aiumemFree(move->params.support.ships);

    move->params.support.ships = selectMemDupSelection(ships, "dupmtss", 0);
}



/*-----------------------------------------------------------------------------
    Name        : aitCheckIfOtherDefTeamAnsweringSignalNeedsHelp
    Description : Look... the name of the function is pretty explicit.  What are you, dumb or something?
    Inputs      : team - the team doing the check, ships - the threat being responded to
    Outputs     :
    Return      : TRUE if the team needs help
----------------------------------------------------------------------------*/
bool aitCheckIfOtherDefTeamAnsweringSignalNeedsHelp(AITeam *team, SelectCommand *ships)
{
    udword i;
    AITeamMove *move;
    SelectCommand *teams = NULL;
    bool return_value = TRUE;

    for (i = 0; i < aiCurrentAIPlayer->numGuardTeams; i++)
    {
        if (aiCurrentAIPlayer->guardTeams[i] != team)
        {
            move = aiCurrentAIPlayer->guardTeams[i]->curMove;

            if (!move)
            {
                continue;
            }

            if (((move->type == MOVE_ATTACK) &&
                 (SelectionsAreEquivalent(move->params.attack.ships, ships))) ||
                ((move->type == MOVE_ADVANCEDATTACK) &&
                 (SelectionsAreEquivalent(move->params.advatt.targets, ships))) ||
                ((move->type == MOVE_FLANKATTACK) &&
                 (SelectionsAreEquivalent(move->params.flankatt.targets, ships))))
            {
                teams = selectMergeTwoSelections(teams, aiCurrentAIPlayer->guardTeams[i]->shipList.selection, DEALLOC1);
            }
        }
    }

    if (teams != NULL)
    {
        if (teams->numShips)
        {
            return_value = aiuSelectionNotGoodAtKillingTheseTargets(teams, ships, AIT_DEFTEAM_NEEDS_HELP_STRENGTH);
        }
        memFree(teams);
    }
    return return_value;
}


/*-----------------------------------------------------------------------------
    Name        : aitSetTeamHomoHetero
    Description : Sets the team's Homogenous or Heterogenous flags
    Inputs      : team - the team to set
    Outputs     : sets some flags
    Return      : void
----------------------------------------------------------------------------*/
void aitSetTeamHomoHetero(AITeam *team)
{
    SelectCommand *teamShips = team->shipList.selection;
    ShipType firstShipType   = teamShips->ShipPtr[0]->shiptype;
    udword i;

    bitClear(team->teamFlags, TEAM_HETEROGENEOUS);
    bitClear(team->teamFlags, TEAM_HOMOGENEOUS);

    for (i=1;i<teamShips->numShips;i++)
    {
        if (teamShips->ShipPtr[i]->shiptype != firstShipType)
        {
            bitSet(team->teamFlags, TEAM_HETEROGENEOUS);
        }
    }
    bitSet(team->teamFlags, TEAM_HOMOGENEOUS);
}



/*-----------------------------------------------------------------------------
    Name        : aitCheckTeamHomogenous
    Description : Checks if the team is homogenous by checking for the homogenous flag.
                  If the flag isn't set, check shiplist and set flag
    Inputs      : team - the team to check
    Outputs     : Sets some flags
    Return      : TRUE if the team is homogenous
----------------------------------------------------------------------------*/
bool aitTeamHomogenous(AITeam *team)
{
    if (bitTest(team->teamFlags, TEAM_HOMOGENEOUS))
    {
        return TRUE;
    }
    else if (bitTest(team->teamFlags, TEAM_HETEROGENEOUS))
    {
        return FALSE;
    }
    else
    {
        aitSetTeamHomoHetero(team);
        return aitTeamHomogenous(team);
    }

}


/*-----------------------------------------------------------------------------
    Name        : aitSetTeamStrengthValue
    Description : Sets the team's strength and value variables
    Inputs      : team - the team to rate
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aitSetTeamStrengthValue(AITeam *team)
{
    udword i, shipstrength, shipvalue;
    SelectCommand *teamShips = team->shipList.selection;

    team->teamStrength = 0;
    team->teamValue    = 0;

    for (i=0;i<teamShips->numShips;i++)
    {
        aiuRateShip(&shipstrength, &shipvalue, teamShips->ShipPtr[i]);
        team->teamStrength += shipstrength;
        team->teamValue    += shipvalue;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitSetTeamDifficultyLevel
    Description : Sets the difficulty level of the team.
                  Note: team difficulty level stored as the overall difficulty
                        level added to the team's personal difficulty level.
                        i.e. Overall difficulty 2 (advanced),
                             Team difficulty 1 (intermediate),
                             Actual team difficulty 3 (intermediate-advanced)
                        Actual team levels go from 0 (beginner) to 4 (advanced)
                        Whereas setting team and player difficulty, the settings
                        are only between 0 (beginner) to 2 (advanced). I'll stop
                        explaining now...

    Inputs      : team - the team to set the difficulty level for,
                  playerDL - the overall player difficulty level,
                  teamDL - the team's specific difficulty level
    Outputs     : Changes a the difficulty level number in the team structure
    Return      : the team's actual difficulty level
----------------------------------------------------------------------------*/
ubyte aitSetTeamDifficultyLevel(AITeam *team, udword playerDL, udword  teamDL)
{
    team->teamDifficultyLevel = (ubyte)(playerDL + teamDL);

    return team->teamDifficultyLevel;
}



/*-----------------------------------------------------------------------------
    Name        : aitFindMostValuable
    Description : Finds the most valueable team
    Inputs      : valueness - if > 1, find the nth most valueable team,
                              where n is valueness
                  note: valueness cannot be bigger than 10
    Outputs     :
    Return      : The nth most valueable team
----------------------------------------------------------------------------*/
AITeam *aitFindMostValuable(udword valueness)
{
    AITeam *valueArray[10];
    udword i, j, k;

    dbgAssert(valueness <= 10);

    for (i=0;i<10;i++)
    {
        valueArray[i] = NULL;
    }

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        if (aiCurrentAIPlayer->teams[i])
        {
            for (j=0;(valueArray[j])&&(j<10);j++)
            {
                //insertion sort
                if (valueArray[j]->teamValue < aiCurrentAIPlayer->teams[i]->teamValue)
                {
                    for (k=(10-1);k > j;k--)
                    {
                        valueArray[k] = valueArray[k-1];
                    }
                    valueArray[j] = aiCurrentAIPlayer->teams[i];
                    break;
                }
            }

            if ((j<10) && (!valueArray[j]))
            {
                valueArray[j] = aiCurrentAIPlayer->teams[i];
            }
        }
    }

    if (valueness)
    {
        return valueArray[valueness-1];
    }
    else
    {
        return valueArray[0];
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitTeamIsntCoopWithAnotherTeam_Cloak
    Description : Makes sure the team doesn't have another cloakgenerator cooperating team
    Inputs      : testteam - the team to check
    Outputs     :
    Return      : TRUE if the team isn't cooperating with another cloak team
----------------------------------------------------------------------------*/
bool aitTeamIsntCoopWithAnotherTeam_Cloak(AITeam *testteam)
{
    udword i;
    AITeam *team;

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        if (team->cooperatingTeam == testteam)
        {
            if (aitTeamShipTypeIs(CloakGenerator, team))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : aitTeamIsntCoopWithAnotherTeam_GravWell
    Description : Makes sure the team doesn't have another cooperating team with
                  strikecraft or with a gravwell generator
    Inputs      : testteam - the team to check
    Outputs     :
    Return      : TRUE if the team isn't cooperating with another cloak team
----------------------------------------------------------------------------*/
bool aitTeamIsntCoopWithAnotherTeam_GravWell(AITeam *testteam)
{
    udword i;
    AITeam *team;

    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        if ((team) && (team->cooperatingTeam == testteam))
        {
            if (aitTeamShipClassIs(CLASS_Fighter, team) ||
                aitTeamShipClassIs(CLASS_Corvette, team) ||
                aitTeamShipTypeIs(GravWellGenerator, team))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}



/*-----------------------------------------------------------------------------
    Name        : aitFindCloakGeneratorCoopTeam
    Description : Finds an appropriate team for the cloak generator to cooperate with
    Inputs      : none
    Outputs     :
    Return      : returns a pointer to a team
----------------------------------------------------------------------------*/
AITeam *aitFindCloakGeneratorCoopTeam(void)
{
    AITeam *resconTeam = NULL, *frigTeam = NULL, *team;
    udword i;

    //cloaking the resource controller ROCKS
    if (aiCurrentAIPlayer->airNumRControllers)
    {
        for (i=0;i<aiCurrentAIPlayer->numSupportTeams;i++)
        {
            team = aiCurrentAIPlayer->supportTeam[i];

            if (team &&
                team->shipList.selection->numShips &&
                aitTeamShipTypeIs(ResourceController, team))
            {
                resconTeam = aiCurrentAIPlayer->supportTeam[i];
                break;
            }
        }
    }

    //frigates are also good to cloak
    for (i=0;i<aiCurrentAIPlayer->numAttackTeams;i++)
    {
        team = aiCurrentAIPlayer->attackTeam[i];

        if (team &&
            team->shipList.selection->numShips &&
            aitTeamShipClassIs(CLASS_Frigate, team) &&
            (!aitTeamShipTypeIs(GravWellGenerator, team) &&
             !aitTeamShipTypeIs(CloakGenerator, team)) &&
            (aitTeamIsntCoopWithAnotherTeam_Cloak(team)))
        {
            frigTeam = aiCurrentAIPlayer->attackTeam[i];
            break;
        }
    }

    //later may need to check if the chosen team is being defended by someone already
    //and probably should have a nicer method of deciding between these two options
    //but for a first pass this should work out.
    if (frigTeam == NULL)
    {
        return resconTeam;
    }
    else
    {
        return frigTeam;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aitFindGravWellGeneratorCoopTeam
    Description : Finds an appropriate team for the Gravwell generator to cooperate with
    Inputs      : none
    Outputs     :
    Return      : returns a pointer to a team
----------------------------------------------------------------------------*/
AITeam *aitFindGravWellGeneratorCoopTeam(void)
{
    AITeam *capTeam = NULL, *team;
    udword i;

    //frigates are also good to cloak
    for (i=0;i<aiCurrentAIPlayer->numAttackTeams;i++)
    {
        team = aiCurrentAIPlayer->attackTeam[i];

        if (team &&
            team->shipList.selection->numShips &&
            (aitTeamShipClassIs(CLASS_Frigate, team) ||
             aitTeamShipClassIs(CLASS_Destroyer, team) ||
             aitTeamShipClassIs(CLASS_HeavyCruiser, team)) &&
            (!aitTeamShipTypeIs(GravWellGenerator, team) &&
             !aitTeamShipTypeIs(CloakGenerator, team)) &&
            (aitTeamIsntCoopWithAnotherTeam_GravWell))
        {
            capTeam = team;
            break;
        }
    }

    //later may need to check if the chosen team is being defended by someone already (particularily fighters)
    //and probably should have a nicer method of deciding between these two options
    //but for a first pass this should work out.
    return capTeam;
}


/*-----------------------------------------------------------------------------
    Name        : aitFindGoodCoopTeam
    Description : Finds a good team to cooperate with based on the shiptype
    Inputs      : type - the type of ship we'll be cooperating with
    Outputs     :
    Return      : the good coop team
----------------------------------------------------------------------------*/
AITeam *aitFindGoodCoopTeam(ShipType type)
{
    switch (type)
    {
        case CloakGenerator:
            //frigate class team (resource controller as well)
            return aitFindCloakGeneratorCoopTeam();
            break;
        case GravWellGenerator:
            //frigate or larger team with no fighters guarding
            return aitFindGravWellGeneratorCoopTeam();
            break;
        default:
//            dbgAssert(FALSE);
            break;
    }
    return NULL;
}





//
//  process current move for each team
//
void aitExecute(void)
{
    sdword i;
    AITeam *team;
    AITeamMove *lastMove, *thisMove;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; ++i)
    {
        team = aiCurrentAIPlayer->teams[i];

        // handle any events, if applicable first
        // (they may alter curMove)
        aieExecute(team);

        //if the handler set the team to be destroyed
        if (bitTest(team->teamFlags, AIT_DestroyTeam))
        {
            aitDestroy(aiCurrentAIPlayer, team, TRUE);
            i--;
            continue;
        }

        if (team->curMove == NULL)
        {
            if (team->moves.num > 0)
            {
                team->curMove = (AITeamMove *)listGetStructOfNode(team->moves.head);
                dbgAssert(team->curMove);
            }
            else
                continue;
        }

        //if the series of moves have been completed, don't do anything
        if (team->curMove->type == MOVE_DONE)
            continue;
        if (team->curMove->type == MOVE_DELETETEAM)
        {
            aitDestroy(aiCurrentAIPlayer, team, TRUE);
            i--;
            continue;
        }

        if (team->teamDelay)
        {
            team->teamDelay--;
        }
        else if (team->curMove->processFunction(team))
        {
            //if the process function set the team to be destroyed
            if (bitTest(team->teamFlags, AIT_DestroyTeam))
            {
                aitDestroy(aiCurrentAIPlayer, team, TRUE);
                i--;
                continue;
            }

            lastMove = team->curMove;
            if (!team->curMove->listNode.next)
            {
                team->curMove = NULL;
                // skip the tactics & formation stuff
            }
            else
            {
                team->curMove = (AITeamMove *)listGetStructOfNode(team->curMove->listNode.next);
                thisMove      = team->curMove;

                //if the tactics setting of the new move is different from the
                //old move, set tactics
                if ((aiuTeamFeatureEnabled(team, AIT_TACTICS)) &&
                    (thisMove->tactics != lastMove->tactics) && (team->shipList.selection->numShips))
                {
                    aiuWrapSetTactics(team->shipList.selection,thisMove->tactics);
                }

                //set the formation for this move
                //if the formation doesn't change
                if (thisMove->formation == SAME_FORMATION)
                {
                    //set the current move formation to the last move's formation
                    //or to a default
                    thisMove->formation = lastMove->formation;
                }
                else if ((thisMove->formation != NO_FORMATION) &&
                         (thisMove->formation != lastMove->formation) &&
                         (team->shipList.selection->numShips > 1))
                {
                    aiuWrapFormation(team->shipList.selection,thisMove->formation);
                }

                if (team->teamFlags & AIT_SpecialTeamMask)
                {
                    aitProcessSpecialTeam(team, thisMove);
                }

                team->teamDelay = AIT_TEAM_MOVE_DELAY[team->teamDifficultyLevel];
            }

            if (lastMove->remove)
            {
                if (lastMove->moveCloseFunction)
                {
                    lastMove->moveCloseFunction(team, lastMove);
//                    aiplayerLog((aiIndex, "%x Deleting Move in aitExecute due to Remove flag", team));
                }
                listDeleteNode(&lastMove->listNode);
            }
        }
    }
}

//
//  sends a message to a team
//
//  it will be stored in a FIFO queue until
//  explicitly received with aitMsgReceived()
//
//  (really old msgs will be forgotten)
//
void aitMsgSend(AITeam *fromTeamp, AITeam *teamp, char *msg)
{
    MsgQueue *msgQP;
    sdword mlen;

    if (!msg) return;
    mlen = strlen(msg);
    if (!mlen) return;

    if (!teamp->msgQueue)
    {
        // create new queue
        teamp->msgQueue = memAlloc(sizeof(MsgQueue), "msgqueue", 0);
        memset(teamp->msgQueue,0,sizeof(MsgQueue));        // initialize all to 0, including msgSenders
        //memset(teamp->msgQueue->msgs, 0, sizeof(char *) * MSG_QUEUE_MAX_MSGS);
        //teamp->msgQueue->head = 0;
    }

    msgQP = teamp->msgQueue;

    if (msgQP->msgs[msgQP->head])
        memFree(msgQP->msgs[msgQP->head]);
    msgQP->msgs[msgQP->head] = memAlloc(mlen+1, "msgqmsg",0);

    msgQP->msgSenders[msgQP->head] = fromTeamp;
    strcpy(msgQP->msgs[msgQP->head], msg);

    ++msgQP->head;

    if (msgQP->head == MSG_QUEUE_MAX_MSGS)
    {
        msgQP->head = 0;        // Darren, fix later properly
        aiplayerLog((aiIndex,"\nWarning: MsgQueue Overflow"));
    }
}

//
//  returns 1 if msg is in the received msgs queue
//  (msg will be removed from queue).
//  returns 0 otherwise.
//
sdword aitMsgReceived(AITeam *teamp, char *msg)
{
    MsgQueue *msgQP;
    sdword i;

    msgQP = teamp->msgQueue;
    if (!msgQP || !msg)
        return 0;

    for (i = 0; i < MSG_QUEUE_MAX_MSGS; ++i)
        if (msgQP->msgs[i] && !strcmp(msgQP->msgs[i], msg))
        {
            memFree(msgQP->msgs[i]);
            msgQP->msgs[i] = NULL;
            teamp->msgSender = msgQP->msgSenders[i];
            msgQP->msgSenders[i] = NULL;
            return 1;
        }

    return 0;
}

//
//  free up the entire msg queue (and all msgs)
//
void aitMsgQueueFree(AITeam *teamp)
{
    sdword i;

    if (!teamp->msgQueue)
        return;

    for (i = 0; i < MSG_QUEUE_MAX_MSGS; ++i)
        if (teamp->msgQueue->msgs[i])
            memFree(teamp->msgQueue->msgs[i]);

    memFree(teamp->msgQueue);
    teamp->msgQueue = NULL;
}

/*=============================================================================
    Save Game stuff here on
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

AITeam *savingThisAITeam = NULL;

typedef void (*PreFix_Move)(AITeamMove *move);
typedef void (*Save_Move)(AITeamMove *move);
typedef void (*Load_Move)(AITeamMove *move);
typedef void (*Fix_Move)(AITeamMove *move);

typedef struct
{
    AIMoveTypes movetype;
    PreFix_Move preFixMove;
    Save_Move saveMove;
    Load_Move loadMove;
    Fix_Move fixMove;
} SaveMoveTypeFuncTable;

SaveMoveTypeFuncTable theSaveMoveTypeFuncTable[NUMBER_OF_MOVETYPES] =
{
//    move ID                 Pre_fix move           Save move              load move              fix move
    { MOVE_DONE,              NULL,                  NULL,                  NULL,                  aimFix_MoveDone },
    { MOVE_GUARDSHIPS,        NULL,                  aimSave_GuardShips,    aimLoad_GuardShips,    aimFix_GuardShips },
    { MOVE_DEFMOSHIP,         NULL,                  aimSave_DefMoship,     aimLoad_DefMoship,     aimFix_DefMoship },
    { MOVE_TEMPGUARD,         NULL,                  NULL,                  NULL,                  aimFix_TempGuard },
    { MOVE_GETSHIPS,          aimPreFix_GetShips,    NULL,                  NULL,                  aimFix_GetShips },
    { MOVE_FORMATION,         NULL,                  NULL,                  NULL,                  aimFix_Formation },
    { MOVE_MOVETEAM,          NULL,                  NULL,                  NULL,                  aimFix_MoveTeam },
    { MOVE_MOVETEAMINDEX,     NULL,                  NULL,                  NULL,                  aimFix_MoveTeamIndex },
    { MOVE_MOVETEAMSPLIT,     NULL,                  aimSave_MoveTeamSplit, aimLoad_MoveTeamSplit, aimFix_MoveTeamSplit },
    { MOVE_INTERCEPT,         aimPreFix_Intercept,   NULL,                  NULL,                  aimFix_Intercept },
    { MOVE_MOVETO,            NULL,                  NULL,                  NULL,                  aimFix_MoveTo },
    { MOVE_PATROLMOVE,        aimPreFix_PatrolMove,  aimSave_PatrolMove,    aimLoad_PatrolMove,    aimFix_PatrolMove },
    { MOVE_ACTIVEPATROL,      NULL,                  NULL,                  NULL,                  aimFix_ActivePatrol },
    { MOVE_ACTIVERECON,       NULL,                  NULL,                  NULL,                  aimFix_ActiveRecon },
    { MOVE_SHIPRECON,         NULL,                  aimSave_ShipRecon,     aimLoad_ShipRecon,     aimFix_ShipRecon },
    { MOVE_COUNTSHIPS,        NULL,                  NULL,                  NULL,                  aimFix_CountShips },
    { MOVE_SPECIAL,           NULL,                  aimSave_Special,       aimLoad_Special,       aimFix_Special },
    { MOVE_ATTACK,            NULL,                  aimSave_Attack,        aimLoad_Attack,        aimFix_Attack },
    { MOVE_ADVANCEDATTACK,    aimPreFix_AdvancedAttack,aimSave_AdvancedAttack,aimLoad_AdvancedAttack,aimFix_AdvancedAttack },
    { MOVE_FLANKATTACK,       NULL,                  aimSave_FlankAttack,   aimLoad_FlankAttack,   aimFix_FlankAttack },
    { MOVE_MOVEATTACK,        aimPreFix_MoveAttack,  aimSave_MoveAttack,    aimLoad_MoveAttack,    aimFix_MoveAttack },
    { MOVE_HARASSATTACK,      aimPreFix_HarassAttack,NULL,                  NULL,                  aimFix_HarassAttack },
    { MOVE_SWARMATTACK,       NULL,                  aimSave_SwarmAttack,   aimLoad_SwarmAttack,   aimFix_SwarmAttack },
    { MOVE_SWARMDEFENSE,      NULL,                  aimSave_SwarmDefense,  aimLoad_SwarmDefense,  aimFix_SwarmDefense },
    { MOVE_SWARMPOD,          NULL,                  NULL,                  NULL,                  aimFix_SwarmPod },
    { MOVE_FANCYGETSHIPS,     aimPreFix_FancyGetShips,NULL,                 NULL,                  aimFix_FancyGetShips },
    { MOVE_DOCK,              aimPreFix_Dock,        aimSave_Dock,          aimLoad_Dock,          aimFix_Dock },
    { MOVE_LAUNCH,            NULL,                  NULL,                  NULL,                  aimFix_Launch },
    { MOVE_REINFORCE,         aimPreFix_Reinforce,   NULL,                  NULL,                  aimFix_Reinforce },
    { MOVE_VARSET,            NULL,                  NULL,                  NULL,                  aimFix_VarSet },
    { MOVE_VARINC,            NULL,                  NULL,                  NULL,                  aimFix_VarInc },
    { MOVE_VARDEC,            NULL,                  NULL,                  NULL,                  aimFix_VarDec },
    { MOVE_VARWAIT,           NULL,                  NULL,                  NULL,                  aimFix_VarWait },
    { MOVE_VARDESTROY,        NULL,                  NULL,                  NULL,                  aimFix_VarDestroy },
    { MOVE_GUARDCOOPTEAM,     NULL,                  NULL,                  NULL,                  aimFix_GuardCoopTeam },
    { MOVE_SUPPORT,           NULL,                  aimSave_Support,       aimLoad_Support,       aimFix_Support },
    { MOVE_ARMADA,            NULL,                  NULL,                  NULL,                  aimFix_Armada },
    { MOVE_CONTROLRESOURCES,  NULL,                  aimSave_ControlResources,aimLoad_ControlResources,aimFix_ControlResources },
    { MOVE_RESVOLUME,         NULL,                  aimSave_ResourceVolume,aimLoad_ResourceVolume,aimFix_ResourceVolume },
    { MOVE_CAPTURE,           NULL,                  NULL,                  NULL,                  aimFix_Capture },
    { MOVE_ACTIVECAPTURE,     NULL,                  NULL,                  NULL,                  aimFix_ActiveCapture },
    { MOVE_ACTIVEMINE,        NULL,                  NULL,                  NULL,                  aimFix_ActiveMine },
    { MOVE_MINEVOLUME,        NULL,                  NULL,                  NULL,                  aimFix_MineVolume },
    { MOVE_SPECIALDEFENSE,    NULL,                  NULL,                  NULL,                  aimFix_SpecialDefense },
    { MOVE_ACTIVERES,         NULL,                  NULL,                  NULL,                  aimFix_ActiveResource },
    { MOVE_MOTHERSHIP,        NULL,                  NULL,                  NULL,                  aimFix_MothershipMove },
    { MOVE_KAMIKAZE,          NULL,                  aimSave_Kamikaze,      aimLoad_Kamikaze,      aimFix_Kamikaze },
    { MOVE_HYPERSPACE,        NULL,                  NULL,                  NULL,                  aimFix_Hyperspace },
    { MOVE_DELETETEAM,        NULL,                  NULL,                  NULL,                  aimFix_DeleteTeam }
};


void SaveMoveCB(void *stuff)
{
#define move ((AITeamMove *)stuff)
    SaveChunk *chunk;
    AITeamMove *sc;
    SaveMoveTypeFuncTable *movetypefunctable;

    chunk = CreateChunk(BASIC_STRUCTURE|AITEAMMOVE,sizeof(AITeamMove),move);
    sc = chunkContents(chunk);

    dbgAssert(move->type < NUMBER_OF_MOVETYPES);
    movetypefunctable = &theSaveMoveTypeFuncTable[move->type];
    dbgAssert(move->type == movetypefunctable->movetype);

    if (movetypefunctable->preFixMove)
    {
        movetypefunctable->preFixMove(sc);
    }

    aiePreFixAIEvents(sc);

    SaveThisChunk(chunk);
    memFree(chunk);
    sc = NULL;

    if (movetypefunctable->saveMove)
    {
        dbgAssert(movetypefunctable->loadMove);     // should have corresponding load function
        movetypefunctable->saveMove(move);
    }
#undef move
}

void FixMoveCB(void *stuff)
{
#define move ((AITeamMove *)stuff)
    SaveMoveTypeFuncTable *movetypefunctable;

    dbgAssert(move->type < NUMBER_OF_MOVETYPES);
    movetypefunctable = &theSaveMoveTypeFuncTable[move->type];
    dbgAssert(move->type == movetypefunctable->movetype);

    if (movetypefunctable->fixMove)
    {
        movetypefunctable->fixMove(move);
    }

    aieFixAIEvents(move);
#undef move
}

void LoadMoveCB(LinkedList *list)
{
    AITeamMove *move;
    SaveChunk *chunk;
    SaveMoveTypeFuncTable *movetypefunctable;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|AITEAMMOVE,sizeof(AITeamMove));
    move = memAlloc(sizeof(AITeamMove), "loadedmove", 0);
    memcpy(move,chunkContents(chunk),sizeof(AITeamMove));

    memFree(chunk);

    dbgAssert(move->type < NUMBER_OF_MOVETYPES);
    movetypefunctable = &theSaveMoveTypeFuncTable[move->type];
    dbgAssert(move->type == movetypefunctable->movetype);

    if (movetypefunctable->loadMove)
    {
        movetypefunctable->loadMove(move);
    }

    listAddNode(list,&move->listNode,move);
}

void SaveMsgQueue(MsgQueue *msgQueue)
{
    sdword i;

    SaveInfoNumber(msgQueue->head);

    for (i=0;i<MSG_QUEUE_MAX_MSGS;i++)
    {
        if (msgQueue->msgs[i])
        {
            Save_String(msgQueue->msgs[i]);
        }
        else
        {
            Save_String("");
        }
    }
}

MsgQueue *LoadMsgQueue(void)
{
    sdword i;
    MsgQueue *msgQueue = memAlloc(sizeof(MsgQueue),"msgqueue",0);

    msgQueue->head = LoadInfoNumber();

    for (i=0;i<MSG_QUEUE_MAX_MSGS;i++)
    {
        msgQueue->msgs[i] = Load_String();
        if (msgQueue->msgs[i][0] == 0)
        {
            memFree(msgQueue->msgs[i]);
            msgQueue->msgs[i] = NULL;
        }
        msgQueue->msgSenders[i] = NULL;
    }

    return msgQueue;
}

void SaveThisAITeam(AITeam *team)
{
    SaveChunk *chunk;
    AITeam *sc;

    chunk = CreateChunk(BASIC_STRUCTURE|AITEAM,sizeof(AITeam),team);
    sc = chunkContents(chunk);

    sc->aiplayerowner = AIPlayerToNumber(team->aiplayerowner);

    sc->curMove = ConvertPointerInListToNum(&team->moves,team->curMove);
    dbgAssert(team->custTeamInfo == NULL);      // not supported yet, will not support if no one uses it, fix later
    dbgAssert(team->TeamDiedCB == NULL);        // not supported yet, will not support if no one uses it, fix later
    PreFixCooperatingTeamDiedCB(sc);

    sc->cooperatingTeam = AITeamToTeamIndex(team->cooperatingTeam);
    sc->msgSender = AITeamToTeamIndex(team->msgSender);

    sc->kasFSMWatchFunction = kasConvertFuncPtrToOffset(team->kasFSMWatchFunction);
    sc->kasStateWatchFunction = kasConvertFuncPtrToOffset(team->kasStateWatchFunction);

    SaveThisChunk(chunk);
    memFree(chunk);
    sc = NULL;

    SaveGrowSelection(&team->shipList);
    SaveLinkedListOfStuff(&team->moves,SaveMoveCB);

    if (team->msgQueue) SaveMsgQueue(team->msgQueue);
}

AITeam *LoadThisAITeam()
{
    SaveChunk *chunk;
    AITeam *team;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|AITEAM,sizeof(AITeam));

    team = memAlloc(sizeof(AITeam), "aiteam", 0);
    memcpy(team,chunkContents(chunk),sizeof(AITeam));
    memFree(chunk);

    LoadGrowSelectionAndFix(&team->shipList);
    LoadLinkedListOfStuff(&team->moves,LoadMoveCB);

    if (team->msgQueue) team->msgQueue = LoadMsgQueue();

    return team;
}

void FixThisAITeam(AITeam *team)
{
    team->aiplayerowner = NumberToAIPlayer((sdword)team->aiplayerowner);

    team->curMove = ConvertNumToPointerInList(&team->moves,(sdword)team->curMove);

    team->cooperatingTeam = AITeamIndexToTeam(team->aiplayerowner,(sdword)team->cooperatingTeam);
    team->msgSender = AITeamIndexToTeam(team->aiplayerowner,(sdword)team->msgSender);

    FixCooperatingTeamDiedCB(team);

    team->kasFSMWatchFunction = kasConvertOffsetToFuncPtr((sdword)team->kasFSMWatchFunction);
    team->kasStateWatchFunction = kasConvertOffsetToFuncPtr((sdword)team->kasStateWatchFunction);

    FixLinkedListOfStuff(&team->moves,FixMoveCB);
}

void aitSave(struct AIPlayer *aiplayer)
{
    sdword i;

    SaveInfoNumber(aiplayer->teamsAllocated);
    SaveInfoNumber(aiplayer->teamsUsed);

    for (i=0;i<aiplayer->teamsUsed;i++)
    {
        savingThisAITeam = aiplayer->teams[i];
        SaveThisAITeam(savingThisAITeam);
        savingThisAITeam = NULL;
    }
}

void aitLoad(struct AIPlayer *aiplayer)
{
    sdword i;

    aiplayer->teamsAllocated = LoadInfoNumber();
    aiplayer->teamsUsed = LoadInfoNumber();

    aiplayer->teams = memAlloc(sizeof(AITeam*)*aiplayer->teamsAllocated, "aiteamlist", 0);

    for (i=0;i<aiplayer->teamsUsed;i++)
    {
        aiplayer->teams[i] = LoadThisAITeam();
    }
}

void aitFix(struct AIPlayer *aiplayer)
{
    sdword i;

    for (i=0;i<aiplayer->teamsUsed;i++)
    {
        savingThisAITeam = aiplayer->teams[i];
        FixThisAITeam(savingThisAITeam);
        savingThisAITeam = NULL;
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

