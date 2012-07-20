/*=============================================================================
    Name    : AIFleetMan
    Purpose : Controls construction of ships requested from combatman, defenseman, resourceman

    Created 5/28/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Universe.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIResourceMan.h"
#include "AIAttackMan.h"
#include "AIDefenseMan.h"
#include "AITeam.h"
#include "AIUtilities.h"
#include "AIVar.h"
#include "CommandWrap.h"
#include "Select.h"
#include "ResearchAPI.h"
#include "SinglePlayer.h"
#include "MultiplayerGame.h"
#include "Alliance.h"
#include "Randy.h"

Player *aifFindEnemyOf(Player *player)
{
    uword i;
    Player *enemy;

    for (i=0;i<universe.numPlayers;i++)
    {
        enemy = &universe.players[i];
        if (enemy != player)
        {
            return enemy;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : aifInit
    Description : initialize resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aifInit(struct AIPlayer *aiplayer)
{
    aiplayer->firstTurn         = TRUE;
    aiplayer->recalculateAllies = TRUE;
    aiplayer->ResearchDelay     = 0;
    aiplayer->ScriptCreator     = aiplayer->player->PlayerMothership;
    aiplayer->AICreator         = aiplayer->player->PlayerMothership;

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            if (singlePlayerGame ||
                ((!singlePlayerGame) && bitTest(tpGameCreated.flag, MG_Hyperspace)))
            {
                aiuEnableResourceFeature(AIF_HYPERSPACING);
            }
        case AI_INT:
        case AI_BEG:
            break;
        default:
            dbgAssert(FALSE);
    }

}

/*-----------------------------------------------------------------------------
    Name        : airClose
    Description : Closes resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aifClose(struct AIPlayer *aiplayer)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : aifShipDied
    Description : Removes a dead ship from the Fleet Command's internal structures
    Inputs      : aiplayer, ship - the ship that died
    Outputs     :
    Return      : TRUE if the ship was found and removed
----------------------------------------------------------------------------*/
bool aifShipDied(struct AIPlayer *aiplayer,ShipPtr ship)
{
    bool return_value = FALSE;

    if (ship->playerowner == aiplayer->player &&
        (ship->shiptype == Carrier || ship->shiptype == Mothership))
    {
        RequestShips *request;
        Node *tempnode;

        //could be a creator ship
        if (ship == aiplayer->AICreator)
        {
            aiplayer->AICreator = NULL;
            return_value = TRUE;
        }
        else if (ship == aiplayer->ScriptCreator)
        {
            aiplayer->ScriptCreator = NULL;
            return_value = TRUE;
        }

        //now go through all the requests to make sure
        //this ship isn't referred to as a creator

        //check the resource requests
        if (aiplayer->ResourceManRequestShips.creator == ship)
        {
            aiplayer->ResourceManRequestShips.creator = NULL;
            return_value = TRUE;
        }

        //check the attack requests
        if (aiplayer->AttackManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->AttackManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
        //check the defense requests
        if (aiplayer->DefenseManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->DefenseManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
        //check the script requests
        if (aiplayer->ScriptManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->ScriptManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
    }

    return return_value;
}

void BuildShip(ShipStaticInfo *shipstatic,Player *player,sdword *incRUs,sdword *decRUs, ShipPtr creator)
{
#if(0)	
	Node *shipnode;
	Ship *ship;
	sdword shipcount,classcount;
	CommandToDo *command;
	ShipStaticInfo *tempstat;
#endif

    aiplayerLog((player->playerIndex,"Building ship %s; Total %i",ShipTypeToStr(shipstatic->shiptype),player->shiptotals[shipstatic->shiptype]));

	// unit caps checking code
#if(0)	
	shipnode = universe.ShipList.head;
	shipcount = 0;
	classcount = 0;
	while(shipnode != NULL)
	{
		ship = (Ship *)listGetStructOfNode(shipnode);

        if((ship->playerowner->playerIndex == player->playerIndex) &&
		   (!bitTest(ship->flags, SOF_Dead)))
		{
		  if(ship->shiptype == shipstatic->shiptype)
			{
				shipcount++;
			}
			if(ship->staticinfo->shipclass == shipstatic->shipclass)
			{
				classcount++;
			}
		}
		
		shipnode = shipnode->next;
	}

	shipnode = universe.mainCommandLayer.todolist.head;
	while(shipnode != NULL)
	{
	
		command = (CommandToDo *)listGetStructOfNode(shipnode);
	
		if(command->ordertype.order == COMMAND_BUILDINGSHIP)
		{
			if(command->buildingship.playerIndex == player->playerIndex)
			{
				if(command->buildingship.shipType == shipstatic->shiptype)
				{
					shipcount++;
				}

				tempstat = GetShipStaticInfo(command->buildingship.shipType, player->race);
				if(tempstat->shipclass == shipstatic->shipclass)
				{
					classcount++;
				}
			}
		}
		shipnode = shipnode->next;
	}
	if((cdLimitCaps[shipstatic->shiptype] != -1) && (shipcount+1 > cdLimitCaps[shipstatic->shiptype]))
	{
		_asm nop;
	}
	if(classcount+1 > cdClassCaps[shipstatic->shipclass])	
	{
		_asm nop;
	}
#endif

    if (shipstatic->shiptype == ResourceCollector)
    {
        aiCurrentAIPlayer->NumRCollectorsBeingBuilt++;
    }
    else if (shipstatic->shiptype == ResourceController)
    {
        aiCurrentAIPlayer->NumRControllersBeingBuilt++;
    }
    else if (shipstatic->shiptype == AdvanceSupportFrigate)
    {
        aiCurrentAIPlayer->NumASFBeingBuilt++;
    }
    else if (shipstatic->shiptype == ResearchShip)
    {
        aiCurrentAIPlayer->NumResearchShipsBeingBuilt++;
    }

    if (!creator)
    {
        creator = player->PlayerMothership;
    }

    clWrapBuildShip(&universe.mainCommandLayer, shipstatic->shiptype, creator->shiprace, player->playerIndex, creator);

    *incRUs += shipstatic->buildCost;
    *decRUs -= shipstatic->buildCost;
}

ShipStaticInfo *GetSubstituteShipStatic(ShipRace race)
{
    ShipStaticInfo *shipstatic = NULL;

    switch (race)
    {
        case R1:
        case R2:
            shipstatic = GetShipStaticInfo(LightInterceptor,race);
            break;

        case P1:
            shipstatic = GetShipStaticInfo(P1Fighter,race);
            break;
        case P2:
            shipstatic = GetShipStaticInfo(P2Swarmer,race);
            break;

        default:
            dbgAssert(FALSE);
    }

    if (shipstatic)
    {
        if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
        {
            shipstatic = NULL;
        }
    }

    return shipstatic;
}

/*-----------------------------------------------------------------------------
    Name        : aifBuildRequestedShips
    Description : Decides which build request to process depending on
                  which is more important - attack, defense, or resources
    Inputs      :
    Outputs     :
    Return      : returns the chosen request
----------------------------------------------------------------------------*/
void aifBuildRequestedShips(RequestShips *scriptrequest,RequestShips *attrequest,RequestShips *defrequest,RequestShips *resourcerequest)
{
    ShipStaticInfo *scriptshipstatic = NULL;
    sdword scriptnumships = 0;
    sdword scripttotalcost = 0;

    ShipStaticInfo *attshipstatic = NULL;
    sdword attnumships = 0;
    sdword atttotalcost = 0;

    ShipStaticInfo *defshipstatic = NULL;
    sdword defnumships = 0;
    sdword deftotalcost = 0;

    ShipStaticInfo *resourceshipstatic = NULL;
    sdword resourcenumships = 0;
    sdword resourcetotalcost = 0;

    Player *player = aiCurrentAIPlayer->player;
    Ship *playerMothership = player->PlayerMothership;
    sdword i;
    sdword RUsLeft = player->resourceUnits;
    ShipRace race;

    if (playerMothership == NULL)
    {
        return;     // can't build anything, return
    }

    race = playerMothership->shiprace;

    if ((scriptrequest != NULL) && (scriptrequest->num_ships > 0))
    {
        if ((RacesAllowedForGivenShip[scriptrequest->shiptype] & RaceToRaceBits(race)))
        {
            scriptshipstatic = GetShipStaticInfo(scriptrequest->shiptype,race);
            if (!bitTest(scriptshipstatic->staticheader.infoFlags, IF_InfoLoaded))
            {
                scriptshipstatic = NULL;
            }
        }
        if (scriptshipstatic)
        {
            scriptnumships = (sdword)scriptrequest->num_ships;
            scripttotalcost = scriptnumships * scriptshipstatic->buildCost;
        }
        else
        {
            aiplayerLog((player->playerIndex,"Warning Script could not build shiprace %d shiptype %d",race,scriptrequest->shiptype));
            scriptrequest->num_ships = -1;
        }
    }

    if ((attrequest != NULL) && (attrequest->num_ships > 0))
    {
        if ((RacesAllowedForGivenShip[attrequest->shiptype] & RaceToRaceBits(race)))
        {
            attshipstatic = GetShipStaticInfo(attrequest->shiptype,race);
            if (!bitTest(attshipstatic->staticheader.infoFlags, IF_InfoLoaded))
            {
                attshipstatic = NULL;
            }
        }
        if (attshipstatic)
        {
            attnumships = (sdword)attrequest->num_ships;
            atttotalcost = attnumships * attshipstatic->buildCost;
        }
        else
        {
            aiplayerLog((player->playerIndex,"Warning Attack could not build shiprace %d shiptype %d",race,attrequest->shiptype));
            attrequest->num_ships = -1;
        }
    }

    if ((defrequest != NULL) && (defrequest->num_ships > 0))
    {
        if ((RacesAllowedForGivenShip[defrequest->shiptype] & RaceToRaceBits(race)))
        {
            defshipstatic = GetShipStaticInfo(defrequest->shiptype,race);
            if (!bitTest(defshipstatic->staticheader.infoFlags, IF_InfoLoaded))
            {
                defshipstatic = NULL;
            }
        }
        if (defshipstatic)
        {
            defnumships = (sdword)defrequest->num_ships;
            deftotalcost = defnumships * defshipstatic->buildCost;
        }
        else
        {
            aiplayerLog((player->playerIndex,"Warning Defense could not build shiprace %d shiptype %d",race,defrequest->shiptype));
            defrequest->num_ships = -1;
        }
    }

    if ((resourcerequest != NULL) && (resourcerequest->num_ships > 0))
    {
        if ((RacesAllowedForGivenShip[resourcerequest->shiptype] & RaceToRaceBits(race)))
        {
            resourceshipstatic = GetShipStaticInfo(resourcerequest->shiptype,race);
            if (!bitTest(resourceshipstatic->staticheader.infoFlags, IF_InfoLoaded))
            {
                resourceshipstatic = NULL;
            }
        }
        if (resourceshipstatic)
        {
            resourcenumships = (sdword)resourcerequest->num_ships;
            resourcetotalcost = resourcenumships * resourceshipstatic->buildCost;
        }
        else
        {
            aiplayerLog((player->playerIndex,"Warning Resource could not build shiprace %d shiptype %d",race,resourcerequest->shiptype));
            resourcerequest->num_ships = -1;
        }
    }

    if ((scripttotalcost + resourcetotalcost + atttotalcost + deftotalcost) <= RUsLeft)
    {
        // we can build everything, so let's do that

        // build everything resourceman wants
        for (i=0;i<resourcenumships;i++)
        {
            BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);
        }

        // build everything scriptman wants
        for (i=0;i<scriptnumships;i++)
        {
            BuildShip(scriptshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnScriptman,&RUsLeft,scriptrequest->creator);
            scriptrequest->num_ships--;
        }

        // build everything attman wants
        for (i=0;i<attnumships;i++)
        {
            BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator);
            attrequest->num_ships--;
        }

        // build everything defman wants
        for (i=0;i<defnumships;i++)
        {
            BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator);
            defrequest->num_ships--;
        }

        dbgAssert(RUsLeft >= 0);
    }
    else
    {
        // now choose who gets to go first, second, and third

        real32 totalRUsSpent = (real32) (aiCurrentAIPlayer->NumRUsSpentOnAttman +
                                  aiCurrentAIPlayer->NumRUsSpentOnDefman +
                                  aiCurrentAIPlayer->NumRUsSpentOnResourceman);
        real32 attmanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnAttman) / totalRUsSpent;
        real32 defmanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnDefman) / totalRUsSpent;
        real32 resourcemanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnResourceman) / totalRUsSpent;

        real32 attmanbudgetoverflow = attmanbudgetratio / ATTMAN_BUILDPRIORITY_RATIO;
        real32 defmanbudgetoverflow = defmanbudgetratio / DEFMAN_BUILDPRIORITY_RATIO;
        real32 resourcemanbudgetoverflow = resourcemanbudgetratio / RESMAN_BUILDPRIORITY_RATIO;

        if ((resourcenumships) && (resourcerequest->priority & REQUESTSHIPS_HIPRI))
        {
            resourcemanbudgetoverflow = -1.0f;
        }

        if ((attnumships) && (attrequest->priority & REQUESTSHIPS_HIPRI))
        {
            attmanbudgetoverflow = -1.0f;
        }

        if ((defnumships) && (defrequest->priority & REQUESTSHIPS_HIPRI))
        {
            defmanbudgetoverflow = -1.0f;
        }

        if (attnumships == 0) attmanbudgetoverflow = REALlyBig;
        if (defnumships == 0) defmanbudgetoverflow = REALlyBig;
        if (resourcenumships == 0) resourcemanbudgetoverflow = REALlyBig;

//#define buildscriptships   \

//#define buildattships   \

//#define builddefships   \

//#define buildresships   \

        if (scriptnumships)
        {
			for (i=0;(i<scriptnumships) && (RUsLeft >= scriptshipstatic->buildCost);i++)                              \
			{                                                                                                   \
				BuildShip(scriptshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnScriptman,&RUsLeft,scriptrequest->creator);\
				scriptrequest->num_ships--;                                                                        \
			}
//            buildscriptships
            return;
        }

        if (attmanbudgetoverflow <= defmanbudgetoverflow)
        {
            // attackman goes before defenseman
            if (attmanbudgetoverflow <= resourcemanbudgetoverflow)
            {
                if (defmanbudgetoverflow <= resourcemanbudgetoverflow)
                {
//                    buildattships
					for (i=0;(i<attnumships) && (RUsLeft >= attshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator); \
						attrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildresships
                }
                else
                {
//                    buildattships
					for (i=0;(i<attnumships) && (RUsLeft >= attshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator); \
						attrequest->num_ships--;                                                                        \
					}
//                    buildresships
//                    builddefships
                }
            }
            else
            {
				for (i=0;(i<resourcenumships) && (RUsLeft >= resourceshipstatic->buildCost);i++)                    \
				{                                                                                                   \
					BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);\
				}
//                buildresships
//                buildattships
//                builddefships
            }
        }
        else
        {
            // defenseman goes before attackman
            if (defmanbudgetoverflow <= resourcemanbudgetoverflow)
            {
                if (attmanbudgetoverflow <= resourcemanbudgetoverflow)
                {
					for (i=0;(i<defnumships) && (RUsLeft >= defshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator); \
						defrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildattships
//                    buildresships
                }
                else
                {
					for (i=0;(i<defnumships) && (RUsLeft >= defshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator); \
						defrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildresships
//                    buildattships
                }
            }
            else
            {
				for (i=0;(i<resourcenumships) && (RUsLeft >= resourceshipstatic->buildCost);i++)                    \
				{                                                                                                   \
					BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);\
				}
//                buildresships
//                builddefships
//                buildattships
            }
        }
    }

    return;
}

void aifAttackManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"attackreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->AICreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->AttackManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->AttackManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifDefenseManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"defreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->AICreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->DefenseManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->DefenseManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifScriptManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"defreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->ScriptCreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->ScriptManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->ScriptManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifResourceManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    dbgAssert(number > 0);

    aiCurrentAIPlayer->ResourceManRequestShips.shiptype = shiptype;
    aiCurrentAIPlayer->ResourceManRequestShips.num_ships = number;
    aiCurrentAIPlayer->ResourceManRequestShips.priority = priority;
    aiCurrentAIPlayer->ResourceManRequestShips.creator  = aiCurrentAIPlayer->AICreator;
}

#define constructTeamWaiting(twaiting,stype,nships,tm,dSetVar)   \
            (twaiting) = memAlloc(sizeof(TeamWaitingForTheseShips),"teamwaitingships",0);   \
            (twaiting)->shiptype = (stype);                                                 \
            (twaiting)->num_ships = (nships);                                               \
            (twaiting)->team = (tm);                                                        \
            dbgAssert(strlen(dSetVar) <= AIVAR_LABEL_MAX_LENGTH);                           \
            strcpy((twaiting)->doneSetVarStr,(dSetVar))

void aifTeamRequestsShipsCB(ShipType shiptype,sdword number,struct AITeam *team,char *doneSetVar, sdword priority)
{
    TeamWaitingForTheseShips *teamWaiting;

	aiplayerLog((aiIndex, "%x Requesting %i %ss", team, number, ShipTypeToStr(shiptype)));

    switch (team->teamType)
    {
        case AttackTeam:
            // put request in Q under attackman
            aifAttackManRequestsShipsCB(shiptype,number,priority);
            // add to AttackManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case DefenseTeam:
            // put request in Q under defenseman
            aifDefenseManRequestsShipsCB(shiptype,number,priority);
            // add to DefenseManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case ScriptTeam:
            // put request in Q under scriptman
            aifScriptManRequestsShipsCB(shiptype,number,priority);
            // add to DefenseManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case ResourceTeam:
        default:
            dbgAssert(FALSE);       // not supported yet
            break;
    }
}

void RemoveTeamFromTeamWaitingQ(LinkedList *TeamWaitingQ,AITeam *team)
{
    Node *node = TeamWaitingQ->head;
    Node *nextnode;
    TeamWaitingForTheseShips *teamWaiting;

    while (node != NULL)
    {
        teamWaiting = (TeamWaitingForTheseShips *)listGetStructOfNode(node);
        nextnode = node->next;

        if (teamWaiting->team == team)
        {
            listDeleteNode(node);
        }

        node = nextnode;
    }
}

void aifTeamDied(struct AIPlayer *aiplayer,struct AITeam *team, bool removeAllReferencesToTeam)
{
    sdword i, j;
    MsgQueue *msgQP;

    RemoveTeamFromTeamWaitingQ(&aiplayer->AttackManTeamsWaitingForShipsQ,team);
    RemoveTeamFromTeamWaitingQ(&aiplayer->DefenseManTeamsWaitingForShipsQ,team);
    RemoveTeamFromTeamWaitingQ(&aiplayer->ScriptManTeamsWaitingForShipsQ,team);

    if (!removeAllReferencesToTeam)
    {
        //don't do anything else
        return;
    }

    // remove any messages from team that may still be queued up
    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; ++i)
    {
        msgQP = aiCurrentAIPlayer->teams[i]->msgQueue;
        if (!msgQP) continue;
        for (j = 0; j < MSG_QUEUE_MAX_MSGS; ++j)
            if (msgQP->msgSenders[j] == team)
            {
                if (msgQP->msgs[j])
                    memFree(msgQP->msgs[j]);
                msgQP->msgs[j] = NULL;
                msgQP->msgSenders[j] = NULL;
            }
    }

    if (team->teamType == AttackTeam)
    {
        aiaTeamDied(aiplayer,team);
    }
    else if (team->teamType == DefenseTeam)
    {
        aidTeamDied(aiplayer,team);
    }
    else if (team->teamType == ResourceTeam)
    {
        airTeamDied(aiplayer,team);
    }
}

bool checkAddToTeam(LinkedList *TeamWaitingQ,Ship *ship)
{
    Node *node = TeamWaitingQ->head;
    TeamWaitingForTheseShips *teamWaiting;

    while (node != NULL)
    {
        teamWaiting = (TeamWaitingForTheseShips *)listGetStructOfNode(node);

        dbgAssert(teamWaiting->num_ships > 0);
        if (teamWaiting->shiptype == ship->shiptype)
        {
            teamWaiting->num_ships--;
            aitAddShip(teamWaiting->team,ship);
            if (teamWaiting->num_ships == 0)
            {
                aivarValueSet(aivarFind(teamWaiting->doneSetVarStr),TRUE);
                listDeleteNode(node);
            }
            return TRUE;
        }

        node = node->next;
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aifProcessShipBuildRequests
    Description : Goes through the attack, defense, resource and script ship build queues
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifProcessShipBuildRequests(void)
{
    RequestShips *attrequest = NULL,*defrequest = NULL,*resourcerequest = NULL,*scriptrequest = NULL, *temprequest = NULL;
    sdword olddefnumships;
    sdword oldattnumships;
    sdword oldscriptnumships;
    Player *player = aiCurrentAIPlayer->player;
    ubyte research_delay;
    Node *tempnode;
    udword tech;        //amount of technology that needs to be researched

    if (aiCurrentAIPlayer->ResearchDelay)
    {
        research_delay = --aiCurrentAIPlayer->ResearchDelay;
    }
    else
    {
        research_delay = 0;
        aiCurrentAIPlayer->ResearchDelay = AIF_RESEARCH_DELAY[aiCurrentAIPlayer->aiplayerDifficultyLevel];
    }

    aiCurrentAIPlayer->TechnologyDeficit = 0;

    if (aiCurrentAIPlayer->ResourceManRequestShips.num_ships > 0)
    {
        resourcerequest = &aiCurrentAIPlayer->ResourceManRequestShips;

        tech = rmTechRequiredForShip(player, resourcerequest->shiptype);
        if (tech)
        {
            aiCurrentAIPlayer->TechnologyDeficit += tech;
            if ((!research_delay) && (rmResearchTechForShip(player, resourcerequest->shiptype)))
            {
                aiplayerLog((player->playerIndex, "Researching %s for Resource", ShipTypeToStr(resourcerequest->shiptype)));
            }
            resourcerequest = NULL;
        }
    }

    if (aiCurrentAIPlayer->AttackManRequestShipsQ.head != NULL)
    {
        tempnode   = aiCurrentAIPlayer->AttackManRequestShipsQ.head;
        attrequest = (RequestShips *)listGetStructOfNode(tempnode);

        tech = rmTechRequiredForShip(player, attrequest->shiptype);
        if (tech)
        {
            aiCurrentAIPlayer->TechnologyDeficit += tech;
            if ((!research_delay) && (rmResearchTechForShip(player, attrequest->shiptype)))
                aiplayerLog((player->playerIndex, "Researching %s for Offense", ShipTypeToStr(attrequest->shiptype)));

            //temporary
            attrequest = NULL;

/*            tempnode = tempnode->next;

            while (tempnode)
            {
                attrequest = (RequestShips *)listGetStructOfNode(tempnode);

                if ((attrequest) && (!rmTechRequiredForShip(player, attrequest->shiptype)))
                {
                    break;
                }
                tempnode = tempnode->next;
            }
            if (!tempnode)
            {
                attrequest = NULL;
            }
*/        }

        if (attrequest)
        {
            oldattnumships = attrequest->num_ships;
            dbgAssert(oldattnumships > 0);
        }
    }

    if (aiCurrentAIPlayer->DefenseManRequestShipsQ.head != NULL)
    {
        tempnode   = aiCurrentAIPlayer->DefenseManRequestShipsQ.head;
        defrequest = (RequestShips *)listGetStructOfNode(tempnode);

        tech = rmTechRequiredForShip(player, defrequest->shiptype);
        if (tech)
        {
            aiCurrentAIPlayer->TechnologyDeficit += tech;
            if ((!research_delay) && (rmResearchTechForShip(player, defrequest->shiptype)))
                 aiplayerLog((player->playerIndex, "Researching %s for Defense", ShipTypeToStr(defrequest->shiptype)));
            //temporary
            defrequest = NULL;
/*            tempnode = tempnode->next;

            while (tempnode)
            {
                defrequest = (RequestShips *)listGetStructOfNode(tempnode);

                //go through list of defense requests to find something we can build
                //will this screw up priorities?
                if ((defrequest) && (!rmTechRequiredForShip(player, defrequest->shiptype)))
                {
                    break;
                }
                tempnode = tempnode->next;
            }
            if (!tempnode)
            {
                defrequest = NULL;
            }
*/        }

        if (defrequest)
        {
            olddefnumships = defrequest->num_ships;
            dbgAssert(olddefnumships > 0);
        }
    }

    //it is assumed that any ship requested by the script is researched
    //therefore no research is done for the script system and no checking is done either
    if (aiCurrentAIPlayer->ScriptManRequestShipsQ.head != NULL)
    {
        scriptrequest     = (RequestShips *)listGetStructOfNode(aiCurrentAIPlayer->ScriptManRequestShipsQ.head);
        oldscriptnumships = scriptrequest->num_ships;
        dbgAssert(oldscriptnumships > 0);
    }

    //go through the queues to find more stuff to research if needed
    if (rmFindFreeLab(player) != -1)
    {
        tempnode = aiCurrentAIPlayer->AttackManRequestShipsQ.head;

        while (tempnode != NULL)
        {
            temprequest = (RequestShips *)listGetStructOfNode(tempnode);

            if (rmTechRequiredForShip(player, temprequest->shiptype))
            {
                if ((!research_delay) && (rmResearchTechForShip(player, temprequest->shiptype)))
                {
                    aiplayerLog((player->playerIndex, "Researching %s later in Attack queue", ShipTypeToStr(temprequest->shiptype)));
                    break;
                }
            }
            tempnode = tempnode->next;
        }
    }

    aifBuildRequestedShips(scriptrequest,attrequest,defrequest,resourcerequest);

    if (scriptrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(scriptrequest->num_ships >= 0);
        if (oldscriptnumships != scriptrequest->num_ships)
        {
            dbgAssert((oldscriptnumships - scriptrequest->num_ships) > 0);
            aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[scriptrequest->shiptype] += (ubyte)(oldscriptnumships - scriptrequest->num_ships);
        }

        if (scriptrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&scriptrequest->node);
        }
    }

    if (attrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(attrequest->num_ships >= 0);
        if (oldattnumships != attrequest->num_ships)
        {
            dbgAssert((oldattnumships - attrequest->num_ships) > 0);
            aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[attrequest->shiptype] += (ubyte)(oldattnumships - attrequest->num_ships);
        }

        if (attrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&attrequest->node);
        }
    }

    if (defrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(defrequest->num_ships >= 0);
        if (olddefnumships != defrequest->num_ships)
        {
            dbgAssert((olddefnumships - defrequest->num_ships) > 0);
            aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[defrequest->shiptype] += (ubyte)(olddefnumships - defrequest->num_ships);
        }

        if (defrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&defrequest->node);
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aifAssignNewShips
    Description : Checks who is requesting new ships and assigns them to the proper manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifAssignNewShips(void)
{
    GrowSelection *newships = &aiCurrentAIPlayer->newships;
    udword i;
    ShipPtr ship;

    for (i=0;i<newships->selection->numShips;)
    {
        ship = newships->selection->ShipPtr[i];

        if ( (aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
             checkAddToTeam(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,ship) )

        {
            // script manager requested this ship to be built, so give it to scriptman.
            aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ( (aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
                  (checkAddToTeam(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,ship)) )
        {
            // attack manager requested this ship to be built, so give it to attackman.
            aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ( (aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
                  (checkAddToTeam(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,ship)) )
        {
            // defense manager requested this ship to be built, so give it to defenseman.
            aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ((ship->staticinfo->shipclass == CLASS_Resource) ||
            (ship->staticinfo->shipclass == CLASS_NonCombat) ||
            (ship->shiptype == ResourceController) ||
            (ship->shiptype == AdvanceSupportFrigate) ||
            (ship->shiptype == Mothership) ||
            (ship->shiptype == Carrier))
        {
            growSelectRemoveShip(newships,ship);
            airAddNewShip(ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else
        {
            // leave in reserves
        }
        i++;
    }
}


/*=============================================================================
    Hyperspacing Logic Code:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifHyperspaceInit
    Description : Initializes any hyperspacing stuff
    Inputs      : aiplayer - the CPU to do hyperspacing initialization for
    Outputs     : Whatever's needed to initialize hyperspacing
    Return      : void
----------------------------------------------------------------------------*/
void aifHyperspaceInit(AIPlayer *aiplayer)
{
    //determine what percentage of RUs to skim for hyperspacing
    //for now, just take off 8%
    aiplayer->aifHyperSkimming = 0.08f;
}


/*-----------------------------------------------------------------------------
    Name        : aifSkimHyperspaceRUs
    Description : Banks RUs for Hyperspacing
    Inputs      : none
    Outputs     : transfers some new RUs to the aiplayer structure
    Return      : void
----------------------------------------------------------------------------*/
void aifSkimHyperspaceRUs(void)
{
    udword newResources, skimResources;
    Player *player = aiCurrentAIPlayer->player;

    if (aiCurrentAIPlayer->aifHyperSavings < 0)
    {   // if the CPU is in debt, all new resources go to pay off the debt
        if (player->resourceUnits)
        {
            if (player->resourceUnits < (-(aiCurrentAIPlayer->aifHyperSavings)))
            {   // the players resources do not exceed the debt.  Pay off as much as possible
                aiplayerLog((aiIndex, "Paying off the debt.  Savings = %i, deposit = %i", aiCurrentAIPlayer->aifHyperSavings, player->resourceUnits));
                aiCurrentAIPlayer->aifHyperSavings += player->resourceUnits;
                player->resourceUnits = 0;
            }
            else
            {   // the players resources exceed the debt.  Pay off everything.
                aiplayerLog((aiIndex, "Paying off the entire debt."));
                player->resourceUnits += aiCurrentAIPlayer->aifHyperSavings;
                aiCurrentAIPlayer->aifHyperSavings = 0;
            }
        }
    }
    else if (player->resourceUnits > aiCurrentAIPlayer->aifLastRUCount)
    {   // if the CPU has acquired new resources, skim some off
        newResources  = player->resourceUnits - aiCurrentAIPlayer->aifLastRUCount;
        skimResources = (udword)((real32)newResources * aiCurrentAIPlayer->aifHyperSkimming);
        aiCurrentAIPlayer->aifHyperSavings += skimResources;
        player->resourceUnits              -= skimResources;

        aiplayerLog((aiIndex, "Skimming %i RUs for Hyperspacing.  Savings = %i, RUs = %i", skimResources, aiCurrentAIPlayer->aifHyperSavings, player->resourceUnits));
    }
    aiCurrentAIPlayer->aifLastRUCount = player->resourceUnits;
}


/*=============================================================================
    Ally Logic:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifFindAllies
    Description : Looks at the other CPUs and allies with those that
                  have the same primary enemy
    Inputs      :
    Outputs     : Sets up some alliances
    Return      : void
----------------------------------------------------------------------------*/
extern udword AIPLAYER_BIGOTRY[];

void aifFindAllies(void)
{
    udword i;
    AIPlayer *OtherCPU;
    udword HowMuchDoILoveMyFriends;

    HowMuchDoILoveMyFriends = randyrandom(RAN_AIPlayer, 100);

    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].aiPlayer &&
            (universe.players[i].playerState == PLAYER_ALIVE) &&
            (universe.players[i].aiPlayer != aiCurrentAIPlayer))
        {                   // if the player is a living CPU but not this CPU
            OtherCPU = universe.players[i].aiPlayer;

            if (OtherCPU->primaryEnemyPlayer == aiCurrentAIPlayer->primaryEnemyPlayer)
            {               // if their primary enemies are the same
                if ((HowMuchDoILoveMyFriends < AIPLAYER_BIGOTRY[i]) &&
                    !allianceArePlayersAllied(OtherCPU->player, aiCurrentAIPlayer->player))
                {           // if the CPU wants to be allied if they aren't allied
                    //create an alliance
                    clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE,
                                      OtherCPU->player->playerIndex,
                                      aiCurrentAIPlayer->player->playerIndex);
                }
            }
            else
            {               // their primary enemies aren't the same
                if (allianceArePlayersAllied(OtherCPU->player, aiCurrentAIPlayer->player))
                {           // if they are already allied
                    //break the alliance
                    clWrapSetAlliance(ALLIANCE_BREAKALLIANCE,
                                      OtherCPU->player->playerIndex,
                                      aiCurrentAIPlayer->player->playerIndex);
                }
            }
        }
    }
    aiCurrentAIPlayer->recalculateAllies = FALSE;
}


/*=============================================================================
    Fleet Commands:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifP2FleetCommand
    Description : Logic for fleet command for the P2 pirates
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifP2FleetCommand(void)
{
    Player *player = aiCurrentAIPlayer->player;

    aiuCreateBlobArrays(player);
    aiuUpdateKnowledgeOfEnemyShips(aiCurrentAIPlayer);
    aidClearDistressSignal(aiCurrentAIPlayer);

    aifAssignNewShips();

    aiaP2AttackManager();
    aitExecute();

    aifProcessShipBuildRequests();

    aiuDeleteBlobArrays();
}




/*-----------------------------------------------------------------------------
    Name        : aifFleetCommand
    Description : Logic for the top of the computer player command structure
    Inputs      : aiplayer - the computer player structure
    Outputs     : lotsa stuff
    Return      : void
----------------------------------------------------------------------------*/
void aifFleetCommand(void)
{
    Player *player = aiCurrentAIPlayer->player;
    bool hasFleetControl = (singlePlayerGame) ? singlePlayerGameInfo.giveComputerFleetControl : TRUE;

    aiIndex = aiCurrentAIPlayer->player->playerIndex;

    //temporary
    if (player->race == P2)
    {
        aifP2FleetCommand();
        return;
    }

    if (aiCurrentAIPlayer->recalculateAllies)
    {
        aifFindAllies();
    }

    aiuCreateBlobArrays(player);
    aiuUpdateKnowledgeOfEnemyShips(aiCurrentAIPlayer);
    aidClearDistressSignal(aiCurrentAIPlayer);

    aifAssignNewShips();

    if (aiCurrentAIPlayer->firstTurn)
    {
        if (hasFleetControl)
        {
            aiaProcessSpecialTeams();
//        airProcessSpecialTeams();
        }
        aiCurrentAIPlayer->firstTurn = FALSE;
    }
    else
    {
        if (hasFleetControl)
        {
            if (aiuResourceFeatureEnabled(AIF_HYPERSPACING))
            {
                aifSkimHyperspaceRUs();
            }

            aiCurrentAIPlayer->ResourceManRequestShips.num_ships = 0;

            //call AttackMan - any ships requested will result in call to aifAttackManRequestsShipsCB
            aiaAttackManager();

            //call DefenseMan - any ships requested will result in call to aifDefenseManRequestsShipsCB
            aidDefenseManager();

            //call Resource Manager - any ships requested will result in call to aifResourceManRequestsShipsCB
            airResourceManager();
        }
    }

    // call Team Manager - any ships requested will result in call to aifAttackManRequestsShipsCB or aifDefenseManRequestsShipsCB
    aitExecute();

    aifProcessShipBuildRequests();

    aiuDeleteBlobArrays();
}




