/*=============================================================================
    Name    : AIDefenseMan
    Purpose : Defense Manager

    Created 1998/05/28 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIHandler.h"
#include "AITeam.h"
#include "AIOrders.h"
#include "AIUtilities.h"
#include "Select.h"
#include "Stats.h"
#include "AIMoves.h"
#include "FastMath.h"
#include "Randy.h"

bool aitAnyTeamOfPlayerGuardingThisShip(struct AIPlayer *aiplayer,Ship *ship)
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

            if (move->type == MOVE_GUARDSHIPS)
            {
                if (move->params.guardShips.ships)
                {
                    if (ShipInSelection(move->params.guardShips.ships,ship))
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
    Name        : aidSendDistressSignal
    Description : Adds ships to the list of ships in distress
    Inputs      : ships - the ships in distress
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidSendDistressSignal(SelectCommand *ships)
{
    aiCurrentAIPlayer->aidDefenseTargets = selectMergeTwoSelections(aiCurrentAIPlayer->aidDefenseTargets, ships, DEALLOC1);
    aiCurrentAIPlayer->aidDistressShips  = selectMergeTwoSelections(aiCurrentAIPlayer->aidDistressShips, ships, DEALLOC1);
}


/*-----------------------------------------------------------------------------
    Name        : aidSendInvasionSignal
    Description : Adds ships to the list of ships invading the computer player's space
    Inputs      : ships - the ships invading
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidSendInvasionSignal(SelectCommand *ships)
{
    aiCurrentAIPlayer->aidDefenseTargets = selectMergeTwoSelections(aiCurrentAIPlayer->aidDefenseTargets, ships, DEALLOC1);
    aiCurrentAIPlayer->aidInvadingShips  = selectMergeTwoSelections(aiCurrentAIPlayer->aidInvadingShips, ships, DEALLOC1);
}


/*-----------------------------------------------------------------------------
    Name        : aidClearDistressSignal
    Description : Clears the shipsindistress variable (note - name has changed
                  to aidDefenseTargets
    Inputs      : aiplayer - the player to do the clearing for
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidClearDistressSignal(AIPlayer *aiplayer)
{
    if (aiplayer->aidDefenseTargets)
    {
        memFree(aiplayer->aidDefenseTargets);
        aiplayer->aidDefenseTargets = NULL;
    }
    if (aiplayer->aidInvadingShips)
    {
        memFree(aiplayer->aidInvadingShips);
        aiplayer->aidInvadingShips = NULL;
    }
    if (aiplayer->aidDistressShips)
    {
        memFree(aiplayer->aidDistressShips);
        aiplayer->aidDistressShips = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aidCountDefendableShips
    Description : Counts the number of unarmed or lightly armed ships in the computer player's fleet
    Inputs      :
    Outputs     :
    Return      : The number of defendable ships
----------------------------------------------------------------------------*/
udword aidCountDefendableShips(void)
{
    udword i, numdefships = 0;
    ShipPtr ship;

    for (i=0;i<aiCurrentAIPlayer->airResourceReserves.selection->numShips;i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];
        if ((ship->shiptype == ResourceCollector) || (ship->shiptype == RepairCorvette)
            || (ship->shiptype == SalCapCorvette))
        {
            numdefships++;
        }
        else if ((ship->shiptype == ResourceController) ||
                 (ship->shiptype == AdvanceSupportFrigate) ||
                 (ship->shiptype == Carrier))
        {
            //these ships count as two
            numdefships += AID_RESCON_ASF_DEFENDABLE_MULTIPLIER;
        }
    }
    return numdefships;
}



/*-----------------------------------------------------------------------------
    Name        : aidCleanupUnusedTeams
    Description : Puts any ships from teams that don't do anything into the
                  combat reserves
    Inputs      : None
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void aidCleanupUnusedTeams(void)
{
    sdword i;

    for (i=0;i<aiCurrentAIPlayer->numGuardTeams;)
    {
        if (aitTeamIsDone(aiCurrentAIPlayer->guardTeams[i]))
        {
            // return any members of team to reserves:
            AITeam *team = aiCurrentAIPlayer->guardTeams[i];
            sdword j;

            for (j=0;j<team->shipList.selection->numShips;j++)
            {
                growSelectAddShip(&aiCurrentAIPlayer->newships, team->shipList.selection->ShipPtr[j]);
            }

            // delete this team
            aitDestroy(aiCurrentAIPlayer,aiCurrentAIPlayer->guardTeams[i],TRUE);

//            aiCurrentAIPlayer->numGuardTeams--;
//            aiCurrentAIPlayer->guardTeams[i] = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams];
//            aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = NULL;
            continue;       // check same index again
        }
        i++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidCheckSphereOfInfluence
    Description : Checks the Mothership's sphere of influence for invasions
                  and KILLS THEM
                  Note that sphere of influence measures in distance squared for
                  efficiency.
    Inputs      : None
    Outputs     : Gets the defense teams to annihilate anything that crosses
                  the sphere of influence line`
    Return      : void
----------------------------------------------------------------------------*/
void aidCheckSphereOfInfluence(void)
{
    ShipPtr enemy_mothership;
    real32 distsq, spherewithextra;
    vector mothership_pos;
    SelectCommand *enemy_invaders;
    ShipPtr mothership = aiCurrentAIPlayer->player->PlayerMothership;
    udword i;

    if (!mothership)
    {
        return;
    }
    mothership_pos = mothership->posinfo.position;

    //calculates sphere of influence if there isn't one,
    //or every 15 seconds
    if ((!aiCurrentAIPlayer->sphereofinfluence) ||
        (!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)))
    {
        enemy_mothership = aiuFindEnemyMothership(aiCurrentAIPlayer->player);
        if (enemy_mothership == NULL)
        {
            return;
        }
        distsq = aiuFindDistanceSquared(enemy_mothership->posinfo.position, mothership_pos);
        aiCurrentAIPlayer->sphereofinfluence = distsq/4;
    }

    if (aiuDefenseFeatureEnabled(AID_SPHERE_OF_INFLUENCE_INVADERS))
    {
        enemy_invaders = aiuFindLeadShipInSphereOfInfluence(mothership_pos, aiCurrentAIPlayer->sphereofinfluence);

        //if there invaders
        if ((enemy_invaders))
        {
            aidSendInvasionSignal(enemy_invaders);
            memFree(enemy_invaders);
        }
    }

    //check for guard teams outside of the sphere of influence and
    //recall them
    //later have a chance of no recall depending on "personality"
    spherewithextra = aiCurrentAIPlayer->sphereofinfluence * 1.4;
    for (i=0;i<aiCurrentAIPlayer->numGuardTeams;i++)
    {
        if (aitNumTeamShips(aiCurrentAIPlayer->guardTeams[i]))
        {
            distsq = aiuFindDistanceSquared(mothership_pos, aitApproxTeamPos(aiCurrentAIPlayer->guardTeams[i]));

            if (distsq > spherewithextra)
            {
                aitRecallGuardTeam(aiCurrentAIPlayer->guardTeams[i]);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aidSetupResourceDefenseTeams
    Description : Sets up defenses for resource collectors
    Inputs      : None
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidSetupResourceDefenseTeams(void)
{
    ShipPtr ship;
    sdword i;
    SelectCommand *selectone;

    //if the difficulty level is high enough, when a certain number of resource
    //defense teams have been setup, they change to patrolling defense teams that
    //actively defend ships rather than passively guard them
    for (i=0;i<aiCurrentAIPlayer->airResourceReserves.selection->numShips;i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];
        if ((ship->shiptype == ResourceCollector) || (ship->shiptype == ResourceController) ||
            (ship->shiptype == AdvanceSupportFrigate))
        {
            if (!aitAnyTeamOfPlayerGuardingThisShip(aiCurrentAIPlayer,ship))
            {
                if (aiCurrentAIPlayer->numGuardTeams < AIPLAYER_MAX_NUM_GUARDTEAMS)
                {
                    selectone = memAlloc(sizeofSelectCommand(1),"guardreser",0);
                    selectone->numShips = 1;
                    selectone->ShipPtr[0] = ship;

                    // create new defense team to guard ship
                    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);
                    aioCreateGuardShips(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams],selectone);
                    aiCurrentAIPlayer->numGuardTeams++;
                }
                else
                {
                    aiplayerLog((aiIndex,"Warning - Defenseman can't guard resource ship"));
                }
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidCreateFastGuardTeam
    Description : Creates an active guard team consisting of fast ships (fighters)
    Inputs      : none
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void aidCreateFastGuardTeam(void)
{
    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);

    //identify team as fast roving team
    bitSet(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]->teamFlags, FAST_ROVING_GUARD);
    aioCreateFastRovingDefense(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]);
    aiCurrentAIPlayer->numGuardTeams++;
}


/*-----------------------------------------------------------------------------
    Name        : aidCreateSlowGuardTeam
    Description : Creates an active guard team consisting of slow ships (corvettes
                  and frigates)
    Inputs      : none
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void aidCreateSlowGuardTeam(void)
{
    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);

    //identify team as slow roving team
    bitSet(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]->teamFlags, SLOW_ROVING_GUARD);
    aioCreateSlowRovingDefense(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]);
    aiCurrentAIPlayer->numGuardTeams++;
}


/*-----------------------------------------------------------------------------
    Name        : aidSetupRovingDefenseTeams
    Description : Sets up and manages the roving defense teams
    Inputs      :
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidSetupRovingDefenseTeams(void)
{
    udword numDefendableShips = aidCountDefendableShips();

    if (aiCurrentAIPlayer->numGuardTeams >= AID_ROVING_DEFENSE_LOW_COUNT_CUTOFF)
    {
        if (numDefendableShips/AID_ROVING_DEFENSE_LOW_COUNT_DIVISOR > aiCurrentAIPlayer->numGuardTeams)
        {
            if (num_is_odd(aiCurrentAIPlayer->numGuardTeams))
            {
                aidCreateSlowGuardTeam();
            }
            else
            {
                aidCreateFastGuardTeam();
            }
        }
    }
    else if (numDefendableShips/AID_ROVING_DEFENSE_HIGH_COUNT_DIVISOR > aiCurrentAIPlayer->numGuardTeams)
    {
        if (num_is_odd(aiCurrentAIPlayer->numGuardTeams))
        {
            aidCreateSlowGuardTeam();
        }
        else
        {
            aidCreateFastGuardTeam();
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidPositionProximitySensorRoving
    Description : Hooks up a proximity sensor to a moving team
    Inputs      : Index of the proximity sensor (i.e. location in the aiCurrentAIPlayer->aidProximitySensors array
    Outputs     : Does that guard command thing with the proximity sensor
    Return      : void
----------------------------------------------------------------------------*/
void aidPositionProximitySensorRoving(udword index)
{
    AITeam *team;
    SelectCommand selone;

    selone.numShips   = 1;
    selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[index];

    if (index > 1)
    {
        team = aitFindMostValuable(index-1);
    }
    else
    {
        team = aitFindMostValuable(index);
    }

    aiuWrapProtect(&selone, team->shipList.selection);
}


/*-----------------------------------------------------------------------------
    Name        : aidPositionProximitySensorGuard
    Description : Positions the proximity sensor(s) in a guard position close to the mothership
    Inputs      : index - the index of the proximity sensor (i.e. location in the aiCurrentAIPlayer->aidProximitySensors structure
    Outputs     : Moves some ships around
    Return      : void
----------------------------------------------------------------------------*/
void aidPositionProximitySensorGuard(udword index)
{
    vector moveto0, moveto2;
    vector originvect = {0,0,0};
    SelectCommand selone;

    if ((!vecAreEqual(aiCurrentAIPlayer->aidProximitySensors->ShipPtr[0]->moveTo, originvect))||(index==2))
    {
        //the ship for index 2 is moved when index 0 is moved
        //ships are only moved when they've done moving
        return;
    }

    selone.numShips   = 1;
    selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[index];

    if (index == 0)
    {
        moveto0 = aiuGenerateRandomStandoffPoint(aiuPlayerMothershipCoords(aiCurrentAIPlayer->player), 6000, originvect, RSP_NEAR);
        aiuWrapMove(&selone, moveto0);
    }

    if (aiCurrentAIPlayer->aidProximitySensors->numShips > 2)
    {
        selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[2];

        moveto2 = aiuGenerateFlankCoordinates(aiuPlayerMothershipCoords(aiCurrentAIPlayer->player), moveto0, originvect, 6000);
        aiuWrapMove(&selone, moveto2);
    }


}




/*-----------------------------------------------------------------------------
    Name        : aidCloakerDefense
    Description : Looks for cloakers, and reacts to their presence in a
                  paranoid delusional way
                  note: due to the way the build queues work, it is simpler
                        to have the proximity sensors be moved and positioned
                        by ResourceMan, hence the ResMan callback instead of
                        defenseman callback.
    Inputs      : Nahtink
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidCloakDefense(void)
{
    udword num_sensors, i;

    if (aiuFindCloakersInEnemyShipsIAmAwareOf(aiuDefenseFeatureEnabled(AID_CLOAK_DEFENSE_RED)))
    {
        if (bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED))
        {
            num_sensors = randyrandombetween(RAN_AIPlayer, 3, 6);

            if (aiCurrentAIPlayer->NumProxSensorsRequested < num_sensors)
            {
                if (aiCurrentAIPlayer->aidProximitySensors &&
                    (aiCurrentAIPlayer->aidProximitySensors->numShips < 6))
                {
                    aifResourceManRequestsShipsCB(ProximitySensor, num_sensors - aiCurrentAIPlayer->NumProxSensorsRequested, 0);
                    aiCurrentAIPlayer->NumProxSensorsRequested = num_sensors;
                }
            }
            //make sure we have a few proximity sensors
            //send them to where the cloaked ships were sighted
        }
        else if (bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_YELLOW))
        {
            if (!aiCurrentAIPlayer->NumProxSensorsRequested)
            {
                if (aiCurrentAIPlayer->aidProximitySensors &&
                    (aiCurrentAIPlayer->aidProximitySensors->numShips < 6))
                {
                    aiCurrentAIPlayer->NumProxSensorsRequested = randyrandombetween(RAN_AIPlayer, 1,3);
                    aifResourceManRequestsShipsCB(ProximitySensor, aiCurrentAIPlayer->NumProxSensorsRequested, 0);
                }
            }
            //make sure we have 1 or 2 proximity sensors
            //position them in guard positions - 1 close to mothership,
            //another roving with roving guard
        }
    }
    else
    {
        if ((!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)) &&
            (aiCurrentAIPlayer->aidProximitySensors) &&
            ((aiCurrentAIPlayer->NumProxSensorsRequested > aiCurrentAIPlayer->aidProximitySensors->numShips)) &&
            (aiCurrentAIPlayer->aidProximitySensors->numShips < 3))
        {
            aifResourceManRequestsShipsCB(ProximitySensor, (aiCurrentAIPlayer->NumProxSensorsRequested - aiCurrentAIPlayer->aidProximitySensors->numShips), 0);
        }

        //reset Red alert status every every 15 seconds to check if it's still valid
        if ((bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED)) &&
            (!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)))
        {
            bitClear(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED);
        }
    }

    if (aiCurrentAIPlayer->aidProximitySensors)
    {
        for (i=0;i < aiCurrentAIPlayer->aidProximitySensors->numShips;i++)
        {
            //if i is odd or larger than three
            if ((i > 3) || (i & 0x1))
            {
                aidPositionProximitySensorRoving(i);
            }
            else
            {
                aidPositionProximitySensorGuard(i);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aidAddSlackerShips
    Description : Gets ships that are idle to defend the mothership
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
SelectCommand *aidAddSlackerShips(void)
{
    udword i;
    AITeam *slackteam;
    struct AITeamMove *newMove;
    SelectCommand *slackerships = NULL;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        slackteam = aiCurrentAIPlayer->teams[i];

        //if team isn't doing anything, and the team has ships and
        //the first ship is dangerous (presumably the rest of the
        //team is dangerous if the first ship is
        if ((slackteam->teamType != ScriptTeam) &&
            (aitTeamIsIdle(slackteam)) &&
            (slackteam->shipList.selection->numShips) &&
            (aiuIsShipDangerous(slackteam->shipList.selection->ShipPtr[0])))
        {

            //add a defend mothership move
            newMove = aimCreateDefendMothershipNoAdd(slackteam, TRUE, TRUE);
            aieHandlerSetFuelLow(newMove, AID_DEFEND_MOTHERSHIP_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
            aimInsertMove(slackteam, newMove);

            slackerships = selectMergeTwoSelections(slackerships, aiCurrentAIPlayer->teams[i]->shipList.selection, DEALLOC1);
        }
    }
    return slackerships;
}


/*-----------------------------------------------------------------------------
    Name        : aidAddGuardingShips
    Description : Finds teams that are guarding and adds them to the mothership defense
    Inputs      :
    Outputs     : Inserts a move into the guarding team's movelist
    Return      : The selection of ships that were added
----------------------------------------------------------------------------*/
SelectCommand *aidAddGuardingShips(void)
{
    udword i;
    AITeam *guardteam;
    SelectCommand *guardships = NULL;
    AITeamMove *newMove;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        guardteam = aiCurrentAIPlayer->teams[i];

        if ((guardteam->teamType != ScriptTeam) &&
            (aitTeamIsGuarding(guardteam)) &&
            (guardteam->shipList.selection->numShips) &&
            (aiuIsShipDangerous(guardteam->shipList.selection->ShipPtr[0])))
        {
            //add a defend mothership move
            newMove = aimCreateDefendMothershipNoAdd(guardteam, TRUE, TRUE);
            aieHandlerSetFuelLow(newMove, AID_DEFEND_MOTHERSHIP_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
            aimInsertMove(guardteam, newMove);

            guardships = selectMergeTwoSelections(guardships, guardteam->shipList.selection, DEALLOC1);
        }
    }
    return guardships;
}


/*-----------------------------------------------------------------------------
    Name        : aidAddAllShips
    Description : Finds all teams and adds them to the mothership defense
    Inputs      : enemyships - the ships attacking the mothership
    Outputs     : Inserts a move into the guarding team's movelist
    Return      : The selection of ships that were added
----------------------------------------------------------------------------*/
SelectCommand *aidAddAllShips(SelectCommand *enemyships)
{
    udword i;
    AITeam *team;
    SelectCommand *newships = NULL;
    AITeamMove *newMove;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        if ((team->teamType != ScriptTeam) &&
            (team->shipList.selection->numShips) &&
            (team->shipList.selection->ShipPtr[0]->shiptype != Mothership) &&
            (team->shipList.selection->ShipPtr[0]->shiptype != Carrier) &&
            (aiuIsShipDangerous(team->shipList.selection->ShipPtr[0])) &&
            (aitTeamIsntDefendingMothership(team, enemyships)) &&
            (aitTeamIsInMothershipRange(team)))
        {
            //add a defend mothership move
            newMove = aimCreateDefendMothershipNoAdd(team, TRUE, TRUE);
            aieHandlerSetFuelLow(newMove, 5, TRUE, TRUE, aihGenericFuelLowHandler);
            aimInsertMove(team, newMove);

            newships = selectMergeTwoSelections(newships, team->shipList.selection, DEALLOC1);
        }
    }
    return newships;
}



/*-----------------------------------------------------------------------------
    Name        : aidMothershipDefense
    Description : Checks to see if the mothership is being attacked.
                  If it is, ships around the mothership are given defend
                  mothership orders
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidMothershipDefense(void)
{
    SelectCommand *enemyships = NULL, *newships = NULL, *goodguyships = NULL, *combatreserves;
    ShipPtr Ship;
    udword i, newteam;
    AITeam *mdteam = NULL;
    ShipPtr mothership = aiCurrentAIPlayer->player->PlayerMothership;

    enemyships = aiuEnemyShipsInMothershipBlob();

    if ((!enemyships) && (mothership) && (mothership->gettingrocked))
    {
        enemyships = aiuFindNearbyEnemyShips(mothership->gettingrocked, AID_MOTHERSHIP_ATTACKER_FLEET_RADIUS);
        selSelectionAddSingleShip((MaxSelection *)enemyships, mothership->gettingrocked);
    }

    //if there are enemy ships in moship blob
    //they aren't just a single fighter scout or
    //a random (small chance) that it'll consider the enemy anyway,
    //despite the fact that it's a single fighter scout
    if ((enemyships) &&
        ((!((enemyships->numShips == 1) && (isShipOfClass(enemyships->ShipPtr[0], CLASS_Fighter)))) ||
        (ranRandom(RAN_AIPlayer)&255 < 3)))

    {
        aiumemFree(aiCurrentAIPlayer->shipsattackingmothership);
        aiCurrentAIPlayer->shipsattackingmothership = selectMemDupSelection(enemyships, "msdp", Pyrophoric);
        goodguyships = (SelectCommand *)memAlloc(sizeofSelectCommand(aiCurrentAIPlayer->newships.selection->numShips), "moshipdefships", 0);
        goodguyships->numShips = 0;

        //if there are ships in the combat reserves
        if (aiCurrentAIPlayer->newships.selection->numShips)
        {
            combatreserves = aiCurrentAIPlayer->newships.selection;

            //find all reserve ships and add them to the new defense team
            for (i = 0; i < combatreserves->numShips; i++)
            {
                //only select dangerous ships, not puny girly ships
                if (aiuIsShipDangerous(combatreserves->ShipPtr[i]))
                {
                    //if a mothership defense team doesn't exist yet
                    //later maybe have more than one mothership defense team
                    if (aiCurrentAIPlayer->mothershipdefteam == -1)
                    {
                        //create a mothership defense team
                        newteam = aiCurrentAIPlayer->numGuardTeams;
                        aiCurrentAIPlayer->guardTeams[newteam] = aitCreate(DefenseTeam);
                        mdteam = aiCurrentAIPlayer->guardTeams[newteam];
                        aioCreateDefendMothership(mdteam);

                        aiCurrentAIPlayer->mothershipdefteam = newteam;
                        aiCurrentAIPlayer->numGuardTeams++;
                    }
                    else
                    {
                        mdteam = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam];
                    }

                    Ship = combatreserves->ShipPtr[i];
                    growSelectRemoveShip(&aiCurrentAIPlayer->newships, Ship);
                    combatreserves = aiCurrentAIPlayer->newships.selection;
                    aitAddShip(mdteam, Ship);
                }
            }
        }

        if (mdteam)
        {
            dbgAssert(mdteam->shipList.selection->numShips);
        }

        newships = aidAddSlackerShips();

        goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);

        aiuMakeShipsOnlyDangerousToMothership(enemyships);

        if (enemyships->numShips > 0)
        {
            //if there are no defending ships or the defending ships are weaker than the attacking ships
            if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE_MEDIUM) &&
                ((!goodguyships->numShips) ||
                 (statsGetRelativeFleetStrengths(enemyships, goodguyships) > AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH)))
            {
                //get all ships that are currently guarding to join in the fray
                newships = aidAddGuardingShips();
                goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);

                //if there are still no defending ships or the defending ships are still weaker than the attacking ships
                if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE_HARDCORE) &&
                    ((!goodguyships->numShips) ||
                     (statsGetRelativeFleetStrengths(enemyships, goodguyships) > AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH)))
                {
                    //get every available ship to join in the fray
                    newships = aidAddAllShips(enemyships);
                    goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);
                }
            }
        }
    }
    else
    {
        aiumemFree(aiCurrentAIPlayer->shipsattackingmothership);

        if (aiCurrentAIPlayer->mothershipdefteam != -1)
        {
            mdteam = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam];

            for (i=0;i<mdteam->shipList.selection->numShips;i++)
            {
                growSelectAddShip(&aiCurrentAIPlayer->newships, mdteam->shipList.selection->ShipPtr[i]);
            }

            aitDestroy(aiCurrentAIPlayer,mdteam,TRUE);

            if (aiCurrentAIPlayer->mothershipdefteam != aiCurrentAIPlayer->numGuardTeams)
            {
                aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam] = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams];
            }
            aiCurrentAIPlayer->mothershipdefteam = -1;
        }
    }

    aiumemFree(enemyships);
    aiumemFree(goodguyships);

}




/*-----------------------------------------------------------------------------
    Name        : aidDefenseManager
    Description : Manages defense of the computer player's fleet.
    Inputs      : None
    Outputs     : Lots of intricate calculations
    Return      : void
----------------------------------------------------------------------------*/
void aidDefenseManager(void)
{
    //different levels of defense:
    //1. defend individual resource collectors and support ships
    //2. when enough ships have been built to guard, change to an active defense
    //   which keeps ships out of the computer player's sphere of influence
    //3. If sphere of influence broken, a large enough fleet of ships is sent to
    //   intercept it.
    //4. Second sphere of influence is immediate mothership area - already dealt with
    //
    //Note: sphere of influence does not include blobs created by harass ships or ships
    //      in a frontal assault.

    aidCleanupUnusedTeams();

    if (aiuDefenseFeatureEnabled(AID_GUARDING))
    {
        if (aiuDefenseFeatureEnabled(AID_ACTIVE_GUARD))
        {
            aidSetupRovingDefenseTeams();
        }
        else
        {
            aidSetupResourceDefenseTeams();
        }
    }

    aidCheckSphereOfInfluence();

    if (aiuDefenseFeatureEnabled(AID_CLOAK_DEFENSE))
    {
        aidCloakDefense();
    }

    //sets up defense teams for support ships (ASF, Repair Corvette, etc.)
    //aidSetupSupportDefenseTeams();

    //sets up defense for mothership - takes priority over all if deemed necessary
    if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE))
    {
        aidMothershipDefense();
    }
}


/*=============================================================================
    Defense Startup, Shutdown and Other Stuff Functions:
=============================================================================*/
void aidTeamDied(struct AIPlayer *aiplayer,struct AITeam *team)
{
    sdword i;

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        if (team == aiplayer->guardTeams[i])
        {
			aiplayer->numGuardTeams--;
			
			//check if the mothership defense team will replace this dead team
			if (aiplayer->mothershipdefteam == aiplayer->numGuardTeams)
			{
				aiplayer->mothershipdefteam = i;
			}

            aiplayer->guardTeams[i] = aiplayer->guardTeams[aiplayer->numGuardTeams];
				aiplayer->guardTeams[aiplayer->numGuardTeams] = NULL;

        }
    }
}

bool aidShipDied(struct AIPlayer *aiplayer, ShipPtr ship)
{
    bool return_value = FALSE;

    if ((aiplayer->aidProximitySensors) &&
        (clRemoveShipFromSelection(aiplayer->aidProximitySensors, ship)))
    {
        if (aiplayer->NumProxSensorsRequested)
        {
            aiplayer->NumProxSensorsRequested--;
        }
        return_value = TRUE;
    }
    if ((aiplayer->aidDefenseTargets) &&
        (clRemoveShipFromSelection(aiplayer->aidDefenseTargets, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->aidDistressShips) &&
        (clRemoveShipFromSelection(aiplayer->aidDistressShips, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->aidInvadingShips) &&
        (clRemoveShipFromSelection(aiplayer->aidInvadingShips, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->shipsattackingmothership) &&
        (clRemoveShipFromSelection(aiplayer->shipsattackingmothership, ship)))
    {
        return_value = TRUE;
    }

    return return_value;
}

void aidInit(struct AIPlayer *aiplayer)
{
    aiplayer->numGuardTeams            = 0;
    aiplayer->shipsattackingmothership = NULL;
    aiplayer->mothershipdefteam        = -1;
    aiplayer->NumProxSensorsRequested  = 0;
    aiplayer->aidProximitySensors      = NULL;
    aiplayer->aidDefenseTargets        = NULL;
    aiplayer->aidInvadingShips         = NULL;
    aiplayer->aidDistressShips         = NULL;
    aiplayer->sphereofinfluence        = 0;
    aiplayer->AlertStatus              = 0;
//    growSelectInit(&aiplayer->newdefenseships);

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            aiuEnableDefenseFeature(AID_ACTIVE_GUARD);
            aiuEnableDefenseFeature(AID_SPHERE_OF_INFLUENCE_INVADERS);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE_HARDCORE);
            aiuEnableDefenseFeature(AID_CLOAK_DEFENSE_RED);
        case AI_INT:
            aiuEnableDefenseFeature(AID_GUARDING);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE_MEDIUM);
            aiuEnableDefenseFeature(AID_CLOAK_DEFENSE);
        case AI_BEG:
            break;
        default:
            dbgAssert(FALSE);
    }
}

void aidClose(struct AIPlayer *aiplayer)
{
    sdword i;

//    growSelectClose(&aiplayer->newdefenseships);

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        dbgAssert(aiplayer->guardTeams[i]);
        aitDestroy(aiplayer,aiplayer->guardTeams[i],FALSE);
    }

    aiumemFree(aiplayer->shipsattackingmothership);
    aiumemFree(aiplayer->aidProximitySensors);
    aidClearDistressSignal(aiplayer);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : aidAddNewShip
    Description : Adds new ship to defense manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidAddNewShip(Ship *ship)
{
    growSelectAddShip(&aiCurrentAIPlayer->newdefenseships, ship);
}
#endif


