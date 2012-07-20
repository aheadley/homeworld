/*=============================================================================
    Name    : AIResourceMan.c
    Purpose : Code for the Resource Manager of the computer player

    Created 5/27/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "ResCollect.h"
#include "CommandWrap.h"
#include "AIResourceMan.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIOrders.h"
#include "AITeam.h"
#include "AIUtilities.h"
#include "AIDefenseMan.h"
#include "Select.h"
#include "Blobs.h"
#include "ConsMgr.h"
#include "MultiplayerGame.h"
#include "Randy.h"

udword UPDATE_RU_COUNT_RATE = 3;

sdword BUILD_RC_CASH_TABLE_BASE = 400;
sdword BUILD_RC_CASH_TABLE_INC = 200;

sdword MAX_RCOLLECTORS_PER_DOCK_POINT = 3;

sdword MAX_RCOLLECTORS_TO_BUILD = 5;

sdword MIN_WORTHWHILE_RUs = 400;

sdword RUs_PER_RCOLLECTOR = 1000;


sdword MIN_WORTHWHILE_RUs_FOR_CONTROLLER = 3000;

sdword MIN_RCOLLECTORS_FOR_CONTROLLER = 4;

sdword BUILD_RCONTROLLER_CASH_BASE = 1500;

sdword BUILD_ASF_CASH_BASE = 2000;

real32 ASF_POSITION_TO_ENEMY_MOTHERSHIP = 0.4f;

real32 RCONTROLLER_POS_FACTOR_FRIENDLIES_PRESENT = 1.1f;
real32 RCONTROLLER_POS_FACTOR_ENEMIESPRESENT_BUT_OUTNUMBERED = 0.6f;
real32 RCONTROLLER_POS_FACTOR_ENEMIESPRESENT = 0.3f;

real32 RCONTROLLER_POS_FACTOR_INBETWEEN = 1.0f;
real32 RCONTROLLER_POS_FACTOR_NOTINBETWEEN = 0.1f;

scriptEntry AIResourceManTweaks[] =
{
    makeEntry(UPDATE_RU_COUNT_RATE,scriptSetUdwordCB),
    makeEntry(BUILD_RC_CASH_TABLE_BASE,scriptSetSdwordCB),
    makeEntry(BUILD_RC_CASH_TABLE_INC,scriptSetSdwordCB),
    makeEntry(MAX_RCOLLECTORS_PER_DOCK_POINT,scriptSetSdwordCB),
    makeEntry(MAX_RCOLLECTORS_TO_BUILD,scriptSetSdwordCB),
    makeEntry(MIN_WORTHWHILE_RUs,scriptSetSdwordCB),
    makeEntry(RUs_PER_RCOLLECTOR,scriptSetSdwordCB),
    makeEntry(MIN_WORTHWHILE_RUs_FOR_CONTROLLER,scriptSetSdwordCB),
    makeEntry(MIN_RCOLLECTORS_FOR_CONTROLLER,scriptSetSdwordCB),
    makeEntry(BUILD_RCONTROLLER_CASH_BASE,scriptSetSdwordCB),
    makeEntry(ASF_POSITION_TO_ENEMY_MOTHERSHIP,scriptSetReal32CB),
    makeEntry(BUILD_ASF_CASH_BASE,scriptSetSdwordCB),
    makeEntry(RCONTROLLER_POS_FACTOR_FRIENDLIES_PRESENT,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_ENEMIESPRESENT_BUT_OUTNUMBERED,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_ENEMIESPRESENT,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_INBETWEEN,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_NOTINBETWEEN,scriptSetReal32CB),
    endEntry
};

/*-----------------------------------------------------------------------------
    Name        : NumberOfEasilyAccesibleRUs
    Description : returns total number of easily accesible RU's to player
    Inputs      :
    Outputs     :
    Return      : returns total number of easily accesible RU's to player
----------------------------------------------------------------------------*/
sdword NumberOfEasilyAccesibleRUs(Player *player)
{
    // for now, just return the total number of RU's.
    Node *objnode = universe.ResourceList.head;
    Resource *resource;
    sdword numRUs = 0;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->flags & SOF_Resource);

        numRUs += resource->resourcevalue;

        objnode = objnode->next;
    }

    return numRUs;
}

/*-----------------------------------------------------------------------------
    Name        : NumRUDockPointsOnThisShip
    Description : returns number of RU dock points on ship
    Inputs      :
    Outputs     :
    Return      : returns number of RU dock points on ship
----------------------------------------------------------------------------*/
sdword NumRUDockPointsOnThisShip(Ship *ship)
{
    if ((ship->shiptype == Carrier) || (ship->shiptype == Mothership))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : airInit
    Description : initialize resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airInit(struct AIPlayer *aiplayer)
{
    udword i;

    growSelectInit(&aiplayer->airResourceReserves);
    growSelectInit(&aiplayer->airResourceCollectors);
    aiplayer->airEasilyAccesibleRUsInWorld = -1;
    aiplayer->airNumRCollectors            = 0;
    aiplayer->airNumRControllers           = 0;
    aiplayer->airNumASF                    = 0;
    aiplayer->airNumResearchShips          = 0;
    aiplayer->numSupportTeams              = 0;
    aiplayer->ResourcersToBuild            = 0;

    for (i=0;i<AIPLAYER_NUM_SUPPORTTEAMS;i++)
    {
        aiplayer->supportTeam[i] = NULL;
    }

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            aiuEnableResourceFeature(AIR_ACTIVE_MOTHERSHIP);
            aiuEnableResourceFeature(AIR_ACTIVE_RESOURCE_COLLECTION);
            aiuEnableResourceFeature(AIR_ACTIVE_SUPPORT_FRIGATE);
            aiuEnableResourceFeature(AIR_RESOURCE_DISTRESS_SIGNALS);
            aiuEnableResourceFeature(AIR_SMART_COLLECTOR_REQUESTS);
            aiuEnableResourceFeature(AIR_SMART_RESEARCH_SHIP_REQUESTS);
        case AI_INT:
            aiuEnableResourceFeature(AIR_AGGRESSIVE_RESOURCING);
            aiuEnableResourceFeature(AIR_RESOURCE_CONTROLLER_REQUESTS);
            aiuEnableResourceFeature(AIR_SUPPORT_FRIGATE_REQUESTS);
        case AI_BEG:
            aiuEnableResourceFeature(AIR_ACTIVE_RESOURCE_CONTROLLER);
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
void airClose(struct AIPlayer *aiplayer)
{
    udword i;

    growSelectClose(&aiplayer->airResourceReserves);
    growSelectClose(&aiplayer->airResourceCollectors);

    for (i=0;i<AIPLAYER_NUM_SUPPORTTEAMS;i++)
    {
        if (aiplayer->supportTeam[i])
        {
            aitDestroy(aiplayer, aiplayer->supportTeam[i], FALSE);
            aiplayer->supportTeam[i] = NULL;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : airShipDied
    Description : Removes a dead ship from the Combat Manager's internal structures
    Inputs      : aiplayer, ship - the ship that died
    Outputs     :
    Return      : TRUE if the ship was found and removed
----------------------------------------------------------------------------*/
bool airShipDied(struct AIPlayer *aiplayer,ShipPtr ship)
{
//    if ((ship->playerowner == aiplayer->player) || (ship->attributes & ATTRIBUTES_Defector)) // do all checks for defector
//    {
        if (growSelectRemoveShip(&aiplayer->airResourceReserves, ship))
        {
            if (ship->shiptype == ResourceCollector)
            {
                growSelectRemoveShip(&aiplayer->airResourceCollectors, ship);
                aiplayer->airNumRCollectors--;
                dbgAssert(aiplayer->airNumRCollectors >= 0);
            }
            else if (ship->shiptype == ResourceController)
            {
                aiplayer->airNumRControllers--;
                dbgAssert(aiplayer->airNumRControllers >= 0);
            }
            else if (ship->shiptype == AdvanceSupportFrigate)
            {
                aiplayer->airNumASF--;
                dbgAssert(aiplayer->airNumASF >= 0);
            }
            else if (ship->shiptype == ResearchShip)
            {
                aiplayer->airNumResearchShips--;
                dbgAssert(aiplayer->airNumResearchShips >= 0);
                aiCurrentAIPlayer->TechnologyDeficit = 0;       //recalculate technology deficit
            }
            return TRUE;
        }
//    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : airTeamDied
    Description : Removes a team from the resource stuff and all that jazz
    Inputs      : aiplayer, team
    Outputs     : does some things
    Return      : void
----------------------------------------------------------------------------*/
void airTeamDied(struct AIPlayer *aiplayer, struct AITeam *team)
{
    udword i;

    for (i=0;i<aiplayer->numSupportTeams;i++)
    {
        if (aiplayer->supportTeam[i] == team)
        {
            aiplayer->numSupportTeams--;
            aiplayer->supportTeam[i] = aiplayer->supportTeam[aiplayer->numSupportTeams];
            aiplayer->supportTeam[aiplayer->numSupportTeams] = NULL;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : airAddedResourceCollector
    Description : called when a resource collector is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedResourceCollector(Ship *ship)
{
    Resource *resource;
    SelectCommand selectone;
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;
    AITeam *team;

    growSelectAddShip(&aiCurrentAIPlayer->airResourceCollectors, ship);

    aiCurrentAIPlayer->airNumRCollectors++;
    if (aiCurrentAIPlayer->NumRCollectorsBeingBuilt > 0) aiCurrentAIPlayer->NumRCollectorsBeingBuilt--;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_RESOURCE_COLLECTION))
    {
        //add the resource collector to an existing team, or make a new team
        team = aitFindNextTeamWithFlag(NULL, RESOURCECOLLECT_TEAM);

        while (team)
        {
            if (team->shipList.selection->numShips < 3)
            {
                aitAddShip(team, ship);
                break;
            }
            team = aitFindNextTeamWithFlag(team, RESOURCECOLLECT_TEAM);
        }

        if (!team)
        {
            team = aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
            bitSet(aiCurrentAIPlayer->supportTeam[new_team_num]->teamFlags, RESOURCECOLLECT_TEAM);
            growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
            aioCreateResourcer(aiCurrentAIPlayer->supportTeam[new_team_num]);
            aiCurrentAIPlayer->numSupportTeams++;
        }
        if (aiuResourceFeatureEnabled(AIR_AGGRESSIVE_RESOURCING))
        {
            aiuWrapSetTactics(team->shipList.selection, Aggressive);
        }
    }
    else
    {
        resource = univFindNearestResource(ship, 0, NULL);

        selectone.numShips = 1;
        selectone.ShipPtr[0] = ship;

        if (aiuResourceFeatureEnabled(AIR_AGGRESSIVE_RESOURCING))
        {
            aiuWrapSetTactics(&selectone, Aggressive);
        }
        aiuWrapCollectResource(&selectone,resource);
    }
}

bool FindBestPlaceForAdvanceSupportFrigate(vector *destination)
{
    Ship *myMothership = aiCurrentAIPlayer->player->PlayerMothership;
    Ship *enemyMothership = aiuFindEnemyMothership(aiCurrentAIPlayer->player);
    vector diff;

    if (myMothership == NULL)
    {
        return FALSE;
    }

    if (enemyMothership == NULL)
    {
        return FALSE;
    }

    vecSub(diff,enemyMothership->posinfo.position,myMothership->posinfo.position);

    vecMultiplyByScalar(diff,ASF_POSITION_TO_ENEMY_MOTHERSHIP);

    vecAdd(*destination,myMothership->posinfo.position,diff);

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : airAddedResourceController
    Description : called when a resource controller is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedResourceController(Ship *ship)
{
    SelectCommand selectone;
    vector destination;
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_RESOURCE_CONTROLLER))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_RESOURCE);
        aiCurrentAIPlayer->numSupportTeams++;
    }
    else
    {
        if (aiuFindBestResourceBlob(&destination))
        {
            selectone.numShips = 1;
            selectone.ShipPtr[0] = ship;

            aiuWrapMove(&selectone,destination);
        }
    }

    aiCurrentAIPlayer->airNumRControllers++;
    if (aiCurrentAIPlayer->NumRControllersBeingBuilt > 0) aiCurrentAIPlayer->NumRControllersBeingBuilt--;

    // tell new resource controller to do something here

}

/*-----------------------------------------------------------------------------
    Name        : airAddedAdvanceSupportFrigate
    Description : called when a Advance Support Frigate is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedAdvanceSupportFrigate(Ship *ship)
{
    vector destination;
    SelectCommand selectone;

    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_SUPPORT_FRIGATE))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_STRIKECRAFT);
        aiCurrentAIPlayer->numSupportTeams++;
    }
    else
    {
        if (FindBestPlaceForAdvanceSupportFrigate(&destination))
        {
            selectone.numShips = 1;
            selectone.ShipPtr[0] = ship;

            aiuWrapMove(&selectone,destination);
        }
    }

    aiCurrentAIPlayer->airNumASF++;
    if (aiCurrentAIPlayer->NumASFBeingBuilt > 0) aiCurrentAIPlayer->NumASFBeingBuilt--;
}


/*-----------------------------------------------------------------------------
    Name        : airAddedResearchShip
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddedResearchShip(Ship *ship)
{
    aiCurrentAIPlayer->airNumResearchShips++;
    if (aiCurrentAIPlayer->NumResearchShipsBeingBuilt > 0) aiCurrentAIPlayer->NumResearchShipsBeingBuilt--;

    // tell new research ship to do something here

}


/*-----------------------------------------------------------------------------
    Name        : airAddedProximitySensor
    Description : Does that proximity sensor adding stuff that we all read about
                  in Newsweek.  Proximity Sensor positioning and stuff is taken care
                  of by the defense manager
    Inputs      : ship - the new ship
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddedProximitySensor(ShipPtr ship)
{
    if (!aiCurrentAIPlayer->aidProximitySensors)
    {
        aiuNewSelection(aiCurrentAIPlayer->aidProximitySensors, 10, "dproxsens");
    }
    selSelectionAddSingleShip((MaxSelection *)aiCurrentAIPlayer->aidProximitySensors, ship);
}


/*-----------------------------------------------------------------------------
    Name        : airAddedMothership
    Description : Adds a mothership team to the support teams
    Inputs      : ship - the new mothership
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void airAddedMothership(ShipPtr ship)
{
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_MOTHERSHIP))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_MOTHERSHIP);
        aiCurrentAIPlayer->numSupportTeams++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : airAddNewShip
    Description : Adds newly assigned ships to the shipreserve structure
    Inputs      : newsel - selection of the new ships
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddNewShip(Ship *ship)
{
    growSelectAddShip(&aiCurrentAIPlayer->airResourceReserves, ship);

    if (ship->shiptype == ResourceCollector)
    {
        airAddedResourceCollector(ship);
    }
    else if (ship->shiptype == ResourceController)
    {
        airAddedResourceController(ship);
    }
    else if (ship->shiptype == AdvanceSupportFrigate)
    {
        airAddedAdvanceSupportFrigate(ship);
    }
    else if (ship->shiptype == ResearchShip)
    {
        airAddedResearchShip(ship);
    }
    else if (ship->shiptype == ProximitySensor)
    {
        airAddedProximitySensor(ship);
    }
    else if ((ship->shiptype == Mothership) ||
             (ship->shiptype == Carrier))
    {
        airAddedMothership(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : aiuCanBuildShipType
    Description : Returns TRUE if current AI player can build ships of shiptype
    Inputs      : shiptype
    Outputs     :
    Return      : Returns TRUE if current AI player can build ships of shiptype
----------------------------------------------------------------------------*/
bool aiuCanBuildShipType(ShipType shiptype,bool scriptteam)
{
    Ship *playerMothership = aiCurrentAIPlayer->player->PlayerMothership;
    ShipStaticInfo *teststatic;
    ShipRace race;

    if (playerMothership == NULL)
    {
        return FALSE;
    }

    race = playerMothership->shiprace;

    if ((RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(race)) == 0)
    {
        // Can't build this shiptype for this race
        return FALSE;
    }

    teststatic = GetShipStaticInfo(shiptype,race);
    if (!bitTest(teststatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return FALSE;       // this ship static info not loaded, so can't build
    }

    if ((shiptype == ResourceCollector) && (!singlePlayerGame) && (!bitTest(tpGameCreated.flag,MG_HarvestinEnabled)))
    {
        return FALSE;
    }

    if (scriptteam)
    {
        return TRUE;
    }
    else
    {
        return cmCanBuildShipType(playerMothership,shiptype,FALSE);     // later set to TRUE to take into account research
    }


}


/*-----------------------------------------------------------------------------
    Name        : airCheckForDistressSignals
    Description : Checks all resource ships to make sure they aren't being attacked.
                  If they are, a distress signal is sent to the defense manager
    Inputs      : None
    Outputs     : Creates a selection of distressed ships, if there are any
    Return      : void
----------------------------------------------------------------------------*/
void airCheckForDistressSignals(void)
{
    udword i, numShips = aiCurrentAIPlayer->airResourceReserves.selection->numShips;
    ShipPtr ship;
    MaxSelection distressed_ships;

    distressed_ships.numShips = 0;

    for (i = 0; i < numShips; i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];

        if (ship->gettingrocked)
        {
            selSelectionAddSingleShip(&distressed_ships, ship);
        }
    }

    if (distressed_ships.numShips)
    {
        aidSendDistressSignal((SelectCommand *)&distressed_ships);
    }
}


/*-----------------------------------------------------------------------------
    Name        : airDivideSupportTasks
    Description : Divides the support tasks among the support teams
                  Looks through the team array to find any ship with
    Inputs      : none
    Outputs     : Fiddles with the support team structures
    Return      : void
----------------------------------------------------------------------------*/
void airDivideSupportTasks(void)
{
    udword num_teams, num_support_teams, num_teams_per_support, num_teams_leftover,i;
    udword num_ASF_teams = 0;
    AITeam *teams[AIPLAYER_NUM_SUPPORTTEAMS];
    AITeam *team;

    num_teams         = aitFindNumTeamsWithFlag(TEAM_NEEDS_SUPPORT);
    num_support_teams = aiCurrentAIPlayer->numSupportTeams;

    if (!num_teams)
    {
        return;
    }

    //only get non-resourcecontroller support teams
    //later fix this to support more than one team needing support
    //also make AdvanceSupportFrigate support roving guarding teams
    for (i=0; i<num_support_teams;i++)
    {
        team = aiCurrentAIPlayer->supportTeam[i];
        if ((team->shipList.selection->numShips) && (aitTeamShipTypeIs(AdvanceSupportFrigate, team)))
        {
            teams[num_ASF_teams++] = team;
        }
    }

    if (!num_ASF_teams)
    {
        return;
    }

    num_teams_per_support = num_teams / num_support_teams;
    num_teams_leftover    = num_teams % num_support_teams;

    team = aitFindNextTeamWithFlag(NULL, TEAM_NEEDS_SUPPORT);

    i = 0;

    while (team && (i < num_ASF_teams))
    {
        aitMakeTeamSupportShips(teams[i++], team->shipList.selection);
        team = aitFindNextTeamWithFlag(team, TEAM_NEEDS_SUPPORT);
    }

/*    if (!num_teams_leftover)
    {
        for (i=0;i<num_support_teams;i++)
        {
            for (j=0;j<num_teams_per_support;j++)
            {

            }
        }
    }*/
}


/*-----------------------------------------------------------------------------
    Name        : airProcessSpecialTeams
    Description : Processes any early ships to be built by the resource manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airProcessSpecialTeams(void)
{
    if (!aiCurrentAIPlayer->airNumResearchShips)
    {
        if (aiuCanBuildShipType(ResearchShip,FALSE))
        {
            aifResourceManRequestsShipsCB(ResearchShip, 1, REQUESTSHIPS_HIPRI);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : airDumbResourcerRequests
    Description : Uses the ResourcersToBuild variable to determine if a new resourcer needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new resourcer has been requested at high priority
----------------------------------------------------------------------------*/
bool airDumbResourcerRequests(void)
{
    if (aiCurrentAIPlayer->ResourcersToBuild)
    {
        sdword totalRCollectors = aiCurrentAIPlayer->airNumRCollectors + aiCurrentAIPlayer->NumRCollectorsBeingBuilt;

        if (aiCurrentAIPlayer->ResourcersToBuild > totalRCollectors)
        {
            if (aiuCanBuildShipType(ResourceCollector,FALSE))
            {
                aifResourceManRequestsShipsCB(ResourceCollector, aiCurrentAIPlayer->ResourcersToBuild - totalRCollectors, REQUESTSHIPS_HIPRI);
                return TRUE;
            }
        }
    }
    else
    {
        aiCurrentAIPlayer->ResourcersToBuild = randyrandombetween(RAN_AIPlayer, (2 + (udword)aiCurrentAIPlayer->aiplayerDifficultyLevel), (4 + (udword)aiCurrentAIPlayer->aiplayerDifficultyLevel));
        aiplayerLog((aiIndex, "Using Dumb Resourcing - %i Resource Ships", aiCurrentAIPlayer->ResourcersToBuild));
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : airDumbResearchRequests
    Description : Uses the Researchers to build variable to determine of a new
                  research ship needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new research ship has been requested at high priority
----------------------------------------------------------------------------*/
bool airDumbResearchRequests(bool *requestedResearch)
{
    if (aiCurrentAIPlayer->ResearchersToBuild)
    {
        sdword totalResearch = aiCurrentAIPlayer->airNumResearchShips + aiCurrentAIPlayer->NumResearchShipsBeingBuilt;

        if (totalResearch < aiCurrentAIPlayer->ResearchersToBuild)
        {
            if (aiuCanBuildShipType(ResearchShip,FALSE))
            {
                if (!totalResearch)
                {
                    aifResourceManRequestsShipsCB(ResearchShip, 1, REQUESTSHIPS_HIPRI);
                    *requestedResearch = TRUE;
                    return TRUE;
                }
                else
                {
                    aifResourceManRequestsShipsCB(ResearchShip, 1, 0);
                    *requestedResearch = TRUE;
                }
            }
        }
    }
    else
    {
        //later maybe add a bonehead move variable as well to build up to 6
        aiCurrentAIPlayer->ResearchersToBuild = randyrandombetween(RAN_AIPlayer, 1, 3);
        aiplayerLog((aiIndex, "Using Dumb Research - %i Research Ships", aiCurrentAIPlayer->ResearchersToBuild));
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : airSmartResearchRequests
    Description : Uses the technology deficit variable to determine if a new
                  research ship needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new research ship has been requested at high priority
----------------------------------------------------------------------------*/
bool airSmartResearchRequests(bool *requestedResearch)
{
    if (aiCurrentAIPlayer->TechnologyDeficit > 0)
    {
        sdword totalResearch = aiCurrentAIPlayer->airNumResearchShips + aiCurrentAIPlayer->NumResearchShipsBeingBuilt;

        if ((real32)(totalResearch) < (((real32)(aiCurrentAIPlayer->TechnologyDeficit))/2.0))
        {
            if (aiuCanBuildShipType(ResearchShip,FALSE))
            {
                if (!totalResearch)
                {
                    aifResourceManRequestsShipsCB(ResearchShip, 1, REQUESTSHIPS_HIPRI);
                    *requestedResearch = TRUE;
                    return TRUE;
                }
                else
                {
                    aifResourceManRequestsShipsCB(ResearchShip, 1, 0);
                    *requestedResearch = TRUE;
                    return FALSE;
                }
            }
        }
        else
        {
            //recalculate technology deficit every time now.
            aiCurrentAIPlayer->TechnologyDeficit = 0;
        }
    }
    *requestedResearch = FALSE;
    return FALSE;
}




/*-----------------------------------------------------------------------------
    Name        : airResourceManager
    Description : Manages all the resources and non-combat ships for the computer player
    Inputs      : orders - orders from the fleet manager
    Outputs     : sets resource collectors to collect resources and optimizes resource
                  controller and other non-combat ships
    Return      : requests for new ships
----------------------------------------------------------------------------*/
void airResourceManager(void)
{
    sdword cashGenerated;
    bool requestedSomething = FALSE, requestedResearch = FALSE;

    // Update continuously updated variables

    if (((aiCurrentAIPlayer->aiplayerFrameCount & UPDATE_RU_COUNT_RATE) == 0)||
        (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld == -1))
    {
        aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld = NumberOfEasilyAccesibleRUs(aiCurrentAIPlayer->player);
    }

    cashGenerated = aiCurrentAIPlayer->player->resourceUnits + aiCurrentAIPlayer->NumRUsSpentOnAttman + aiCurrentAIPlayer->NumRUsSpentOnDefman + aiCurrentAIPlayer->NumRUsSpentOnScriptman;

    //determine if we should build any research ships
    if (singlePlayerGame || (!singlePlayerGame && bitTest(tpGameCreated.flag,MG_ResearchEnabled)))
    {
        if (aiuResourceFeatureEnabled(AIR_SMART_RESEARCH_SHIP_REQUESTS))
        {
            if (airSmartResearchRequests(&requestedResearch))
            {
                //a high priority request for a research ship has been entered
                return;
            }
        }
        else
        {
            if (airDumbResearchRequests(&requestedResearch))
            {
                //a high priority request for a research ship has been entered
                return;
            }
        }
    }

    //determine if we should build any resource collectors
    if ((singlePlayerGame || (!singlePlayerGame && bitTest(tpGameCreated.flag,MG_HarvestinEnabled))) &&
        (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > MIN_WORTHWHILE_RUs))
    {
        sdword totalRCollectors = aiCurrentAIPlayer->airNumRCollectors + aiCurrentAIPlayer->NumRCollectorsBeingBuilt;

        if (aiuResourceFeatureEnabled(AIR_SMART_COLLECTOR_REQUESTS))
        {
            if (totalRCollectors < MAX_RCOLLECTORS_TO_BUILD)
            {
                if (totalRCollectors < aiCurrentAIPlayer->NumRUDockPoints*MAX_RCOLLECTORS_PER_DOCK_POINT)
                {
                    if ((totalRCollectors == 0) || (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > (totalRCollectors+1)*RUs_PER_RCOLLECTOR))
                    {
                        // see if we have enough cash to make it worthwhile.  (The more resource collectors we
                        // already have, the more cash we want before building a new one because the Resource
                        // Manager should always be providing an increasing amount of RU's).

                        // Note that when we say cash, we mean cashGenerated
                        if ((totalRCollectors == 0) ||      // if this is our first RCollector, we always want to build one
                            (cashGenerated > BUILD_RC_CASH_TABLE_BASE + BUILD_RC_CASH_TABLE_INC * (totalRCollectors-1)))
                        {
                            if (totalRCollectors == 0)
                            {
                                if (aiuCanBuildShipType(ResourceCollector,FALSE))
                                {
                                    aifResourceManRequestsShipsCB(ResourceCollector, 1, REQUESTSHIPS_HIPRI);
                                    return;
                                }
                            }
                            else
                            {
                                if (aiuCanBuildShipType(ResourceCollector,FALSE))
                                {
                                    aifResourceManRequestsShipsCB(ResourceCollector, 1, 0);
                                    requestedSomething = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (airDumbResourcerRequests())
            {
                //a resource collector has been requested at high priority
                return;
            }
        }

        //determine if we should build any resource controllers
        if (aiuResourceFeatureEnabled(AIR_RESOURCE_CONTROLLER_REQUESTS) &&
            (totalRCollectors >= MIN_RCOLLECTORS_FOR_CONTROLLER))
        {
            sdword totalRControllers = aiCurrentAIPlayer->airNumRControllers + aiCurrentAIPlayer->NumRControllersBeingBuilt;
            if ((totalRControllers == 0) && (!requestedResearch))
            {
                if (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > (MIN_WORTHWHILE_RUs_FOR_CONTROLLER * universe.numPlayers))
                {
                    if (cashGenerated > BUILD_RCONTROLLER_CASH_BASE)
                    {
                        if (aiuCanBuildShipType(ResourceController,FALSE))
                        {
                            aifResourceManRequestsShipsCB(ResourceController, 1, 0);
                            return;
                        }
                    }
                }
            }
        }
    }

    if (!requestedSomething)
    {
        //determine if we should build any AdvanceSupportFrigates
        if (aiuResourceFeatureEnabled(AIR_SUPPORT_FRIGATE_REQUESTS) &&
            ((!singlePlayerGame) && bitTest(tpGameCreated.flag, MG_FuelBurnEnabled)) )
        {
            if (cashGenerated > BUILD_ASF_CASH_BASE)
            {
                sdword totalASF = aiCurrentAIPlayer->airNumASF + aiCurrentAIPlayer->NumASFBeingBuilt;

                if ((totalASF == 0) &&
                    aitNeedStrikeSupport(randyrandombetween(RAN_AIPlayer, 10, 15)))
                {
                    if (aiuCanBuildShipType(AdvanceSupportFrigate,FALSE) &&
                        aiuUnitCapCanBuildShip(aiCurrentAIPlayer, AdvanceSupportFrigate, 1))
                    {
                        aifResourceManRequestsShipsCB(AdvanceSupportFrigate, 1, 0);
                    }
                }
            }
        }
    }

    if (aiCurrentAIPlayer->numSupportTeams)
    {
        airDivideSupportTasks();
    }

    //Distress signals only work with active guard,
    //so that feature has to be activated as well
    if (aiuResourceFeatureEnabled(AIR_RESOURCE_DISTRESS_SIGNALS))
    {
        airCheckForDistressSignals();
    }
}

