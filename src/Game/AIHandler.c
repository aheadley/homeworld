/*=============================================================================
    Name    : AIHandler.c
    Purpose : Contains the event handler functions for the computer player

    Created 6/19/1998 by fpoiker
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "AITeam.h"
#include "AIPlayer.h"
#include "AIMoves.h"
#include "AIHandler.h"
#include "AIOrders.h"
#include "CommandWrap.h"
#include "Select.h"
#include "ShipSelect.h"
#include "GravWellGenerator.h"
#include "Randy.h"
#include "UnivUpdate.h"

aieHandlerSimple handlerTable[] =
{
    aihGenericEmptyFuelHandler,
    aihSwarmerEmptyFuelHandler,
    aihGenericFuelLowHandler,
    aihHarassNumbersLowHandler,
    aihHarassFiringSingleShipHandler,
    aihHarassDisengageSingleShipHandler,
    aihKamikazeHealthLowHandler,
    aihFastDefenseNumbersLowHandler,
    aihSlowDefenseNumbersLowHandler,
    (aieHandlerSimple)aihGenericGettingRockedHandler,
    (aieHandlerSimple)aihPatrolEnemyNearbyHandler,
    (aieHandlerSimple)aihGravWellEnemyNearbyHandler,
    aihGravWellEnemyNotNearbyHandler,
    (aieHandlerSimple)aihFastDefenseDistressHandler,
    (aieHandlerSimple)aihSlowDefenseDistressHandler,
    aihFastDefenseTeamDiedHandler,
    aihSlowDefenseTeamDiedHandler,
    aihGuardShipsTeamDiedHandler,
    aihReconaissanceTeamDiedHandler,
    aihHarassTeamDiedHandler,
    aihPatrolTeamDiedHandler,
    aihRemoveTeamDiedHandler,
    NULL
};

sdword aieHandlerToNum(aieHandlerSimple handler)
{
    if (handler == NULL)
    {
        return -1;
    }
    else
    {
        sdword i = 0;
        while (handlerTable[i] != NULL)
        {
            if (handlerTable[i] == handler)
            {
                return i;
            }

            i++;
        }

        dbgFatalf(DBG_Loc,"Save Game: Unknown Handler");
        return -1;
    }
}

aieHandlerSimple aieNumToHandler(sdword num)
{
    if (num == -1)
    {
        return NULL;
    }
    else
    {
        dbgAssert(num < sizeof(handlerTable)/sizeof(handlerTable[0]));
        return handlerTable[num];
    }
}

/*-----------------------------------------------------------------------------
    Name        : aihGenericEmptyFuelHandler
    Description : Scuttles any ship that runs out of fuel - this handler
                  assumes it was called with the "watchIndividual" flag set to TRUE
    Inputs      : team - the team with the ship that has no fuel
    Outputs     : Kills a ship
    Return      : void
----------------------------------------------------------------------------*/
void aihGenericEmptyFuelHandler(AITeam *team)
{
    sdword i;
    SelectCommand scuttleship;

    scuttleship.numShips = 1;

    for (i = (team->shipList.selection->numShips-1); i >= 0; i--)
    {
        if (100.0 * team->shipList.selection->ShipPtr[i]->fuel /
                team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel
                  < team->curMove->events.fuelLow.watchPercentage)
        {
            scuttleship.ShipPtr[0] = team->shipList.selection->ShipPtr[i];
            aiuWrapScuttle(&scuttleship);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aihSwarmerEmptyFuelHandler
    Description : Sets the swarmers to SOF_Crazy when they are out of fuel.
                  NOTE: Need to somehow blow those ships up as well...
    Inputs      : team - the team with the ship that has no fuel
    Outputs     : Kills a ship
    Return      : void
----------------------------------------------------------------------------*/
void aihSwarmerEmptyFuelHandler(AITeam *team)
{
    sdword i, numShips = team->shipList.selection->numShips;
    ShipPtr ship;

    for (i = 0; i < numShips; i++)
    {
        ship = team->shipList.selection->ShipPtr[i];

        if (!bitTest(ship->flags, SOF_Crazy) &&
            (100.0 * ship->fuel / ship->staticinfo->maxfuel
                  < team->curMove->events.fuelLow.watchPercentage))
        {
            bitSet(ship->flags, SOF_Crazy);
            ship->deathtime = universe.totaltimeelapsed + frandyrandombetween(RAN_AIPlayer, 14.0, 20.0);
            ApplyCareenRotationDirectly(ship);
            ship->rotinfo.rotspeed.x *= 1.5;
            ship->rotinfo.rotspeed.y *= 1.5;
            ship->rotinfo.rotspeed.z *= 1.5;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aihGenericFuelLowHandler
    Description : A generic handler function for the fuel low event
    Inputs      : team - the team who's fuel is too low
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihGenericFuelLowHandler(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;

    thisMove->processing = FALSE;
    thisMove->events.fuelLow.triggered = FALSE;

    //add a dock move before - if the ship runs out of fuel on the way, it's toast
    newMove = aimCreateDockNoAdd(team, dockmoveFlags_Normal, NULL, TRUE, TRUE);
    aieHandlerSetFuelLow(newMove, 0, TRUE, FALSE, aihGenericEmptyFuelHandler);
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
}



/*-----------------------------------------------------------------------------
    Name        : aihHarassNumbersLowHandler
    Description : Handles the harass team being decimated
    Inputs      : team - the team being decimated
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihHarassNumbersLowHandler(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    AlternativeShips alternatives;
    vector gathering_point;
    vector origin = ORIGIN_VECTOR;

//commented code is the reinforce type numbers low handler
/*    udword i;

    for (i = 0; i < aiCurrentAIPlayer->numGuardTeams; i++)
    {
        //later need to actually remove team from guardTeams array
        if (bitTest(aiCurrentAIPlayer->guardTeams[i]->teamFlags, FAST_ROVING_GUARD) &&
            (aiCurrentAIPlayer->guardTeams[i]->shipList.selection->numShips))
        {
            bitClear(aiCurrentAIPlayer->guardTeams[i]->teamFlags, FAST_ROVING_GUARD);
            aiCurrentAIPlayer->guardTeams[i]->curMove = aimCreateReinforce(aiCurrentAIPlayer->guardTeams[i], team, TRUE, FALSE);
            aimCreateMoveDone(aiCurrentAIPlayer->guardTeams[i], FALSE, FALSE);
            return;
        }
    }
*/
    //later maybe change to "reinforce" move, maybe along with harass team getting out
    //of there to attack someone else.
    //later - very important to get the same ships that were requested in the first place
    //        this isn't all that "generic"
    if (aiCurrentAIPlayer->player->PlayerMothership)
    {
        gathering_point = aiCurrentAIPlayer->player->PlayerMothership->posinfo.position;
    }
    else
    {
        vecZeroVector(gathering_point);
    }

    while ((thisMove->type == MOVE_ATTACK) || (thisMove->type == MOVE_ADVANCEDATTACK))
    {
        if (thisMove->remove)
        {
            aitDeleteCurrentMove(team);
        }
        else
        {
            team->curMove = (AITeamMove *)listGetStructOfNode(team->curMove->listNode.next);
        }

        thisMove = team->curMove;

        dbgAssert(thisMove);
    }

    gathering_point = aiuFindRangeStandoffPoint(gathering_point,origin, AIH_HARASS_NUMLOW_STANDOFF_DIST);

    thisMove->processing = FALSE;
    thisMove->events.numbersLow.triggered = FALSE;

    //add a move, getships, formation
    newMove = aimCreateMoveTeamNoAdd(team, gathering_point, AIH_HARASS_NUMLOW_FORMATION, FALSE, TRUE);
//band-aid solution... change moveteam move so that tactics are there...
    newMove->tactics = Evasive;
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

    SetNumAlternatives(alternatives,2);
    SetAlternative(alternatives,0,HeavyInterceptor,HARASS_HEAVYINT_EQUIVNUM);
    SetAlternative(alternatives,1,AttackBomber, HARASS_BOMBER_EQUIVNUM);

    newMove = aimCreateFancyGetShipsNoAdd(team, LightInterceptor, 16, &alternatives, REQUESTSHIPS_HIPRI, TRUE, TRUE);
    listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

    bitClear(team->teamFlags, TEAM_NEEDS_SUPPORT);
}


/*-----------------------------------------------------------------------------
    Name        : aihHarassFiringSingleShipHandler
    Description : Puts the harass team into a sphere formation around the ship it's attacking
    Inputs      : team - the team attacking
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihHarassFiringSingleShipHandler(AITeam *team)
{
    aiuWrapFormation(team->shipList.selection,AIH_HARASS_SINGLEATTACK_FORMATION);
    aieHandlerSetDisengage(team->curMove, TRUE, aihHarassDisengageSingleShipHandler);
}


/*-----------------------------------------------------------------------------
    Name        : aihHarassDisengageSingleShipHandler
    Description : Puts the harass team back into the default formation
    Inputs      : team - the team disengaging
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihHarassDisengageSingleShipHandler(AITeam *team)
{
    aiuWrapFormation(team->shipList.selection, team->curMove->formation);
    team->curMove->events.firing.triggered = FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : aihKamikazeHealthLowHandler
    Description : Finds the ship who's health is low and kamikaze's it into the enemy
    Inputs      : team - the team with the suicide bombers
    Outputs     : Forces pilots to kill themselves in the name of greater good
    Return      : void
----------------------------------------------------------------------------*/
void aihKamikazeHealthLowHandler(AITeam *team)
{
    MaxSelection shipsToDie;
    udword i;
    ShipPtr ship;

    dbgAssert(team->shipList.selection->numShips);

    shipsToDie.numShips = 0;

    for (i=0;i<team->shipList.selection->numShips;i++)
    {
        ship = team->shipList.selection->ShipPtr[i];

        if ((isShipOfClass(ship, CLASS_Fighter) || isShipOfClass(ship, CLASS_Corvette)) &&
            ((100 * ship->health) <
             (team->curMove->events.healthLow.watchPercentage * ship->staticinfo->maxhealth)))
        {
            selSelectionAddSingleShip(&shipsToDie, ship);
        }
    }

    dbgAssert(shipsToDie.numShips);

    aiuWrapSetKamikaze((SelectCommand *)&shipsToDie);
}


/*-----------------------------------------------------------------------------
    Name        : aihFastDefenseNumbersLowHandler
    Description : Handles the fast roving defense team being decimated
    Inputs      : team - the team being decimated
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihFastDefenseNumbersLowHandler(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    AlternativeShips alternatives;
    vector gathering_point;
    vector origin = ORIGIN_VECTOR;
    SelectCommand *teamsel = team->shipList.selection;
    sdword original_number = thisMove->events.numbersLow.watchBaseCount;

    //later maybe change to "reinforce" move, maybe along with harass team getting out
    //of there to attack someone else.
    if (aiCurrentAIPlayer->player->PlayerMothership)
    {
        gathering_point = aiCurrentAIPlayer->player->PlayerMothership->posinfo.position;
    }
    else
    {
        vecZeroVector(gathering_point);
    }

    gathering_point = aiuFindRangeStandoffPoint(gathering_point,origin, AIH_FASTDEF_NUMLOW_STANDOFF_DIST);

    thisMove->processing = FALSE;
    thisMove->events.numbersLow.triggered = FALSE;

    //add a move, getships, formation
    newMove = aimCreateMoveTeamNoAdd(team, gathering_point, AIH_FASTDEF_NUMLOW_FORMATION, FALSE, TRUE);
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

    if (teamsel->numShips)
    {
        if (teamsel->numShips < original_number)
        {
            newMove = aimCreateGetShipsNoAdd(team, teamsel->ShipPtr[0]->shiptype, (sbyte)(original_number - teamsel->numShips), 0, TRUE, TRUE);
        }
        else
        {
            //just add one ship (to prevent infinite loops)
            newMove = aimCreateGetShipsNoAdd(team, teamsel->ShipPtr[0]->shiptype, 1, 0, TRUE, TRUE);
        }

    }
    else
    {
        SetNumAlternativesFlags(alternatives,3, ALTERNATIVE_RANDOM)
        SetAlternative(alternatives,0,HeavyDefender,14);
        SetAlternative(alternatives,1,HeavyInterceptor,14);
        SetAlternative(alternatives,2,AttackBomber,15);

        newMove = aimCreateFancyGetShipsNoAdd(team, LightInterceptor, 12, &alternatives, 0, TRUE, FALSE);
    }

    thisMove->events.numbersLow.watchBaseCount = 0;
    thisMove->processing = FALSE;

    listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);

//    newMove = aimCreateFormationNoAdd(team, WALL_FORMATION, FALSE, TRUE);
//    listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);
}


/*-----------------------------------------------------------------------------
    Name        : aihSlowDefenseNumbersLowHandler
    Description : Handles the slow roving defense team being decimated
    Inputs      : team - the team being decimated
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihSlowDefenseNumbersLowHandler(AITeam *team)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    AlternativeShips alternatives;
    vector gathering_point;
    vector origin = ORIGIN_VECTOR;
    SelectCommand *teamsel = team->shipList.selection;
    sdword original_number = thisMove->events.numbersLow.watchBaseCount;

    //later maybe change to "reinforce" move, maybe along with harass team getting out
    //of there to attack someone else.
    if (aiCurrentAIPlayer->player->PlayerMothership)
    {
        gathering_point = aiCurrentAIPlayer->player->PlayerMothership->posinfo.position;
    }
    else
    {
        vecZeroVector(gathering_point);
    }

    gathering_point = aiuFindRangeStandoffPoint(gathering_point,origin, AIH_SLOWDEF_NUMLOW_STANDOFF_DIST);

    thisMove->processing = FALSE;
    thisMove->events.numbersLow.triggered = FALSE;

    //add a move, getships, formation
    newMove = aimCreateMoveTeamNoAdd(team, gathering_point,AIH_SLOWDEF_NUMLOW_FORMATION, FALSE, TRUE);
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);

    if (teamsel->numShips)
    {
        if (teamsel->numShips < original_number)
        {
            newMove = aimCreateGetShipsNoAdd(team, teamsel->ShipPtr[0]->shiptype, (sbyte)(original_number - teamsel->numShips), 0, TRUE, TRUE);
        }
        else
        {
            newMove = aimCreateGetShipsNoAdd(team, teamsel->ShipPtr[0]->shiptype, 1, 0, TRUE, TRUE);
        }
    }
    else
    {
        SetNumAlternativesFlags(alternatives,4,ALTERNATIVE_RANDOM);
        SetAlternative(alternatives,0,HeavyCorvette,12);
        SetAlternative(alternatives,1,LightCorvette,7);
        SetAlternative(alternatives,2,StandardFrigate,30);
        SetAlternative(alternatives,3,DDDFrigate,30);

        newMove = aimCreateFancyGetShipsNoAdd(team, MultiGunCorvette, 3, &alternatives, 0, TRUE, FALSE);
    }

    thisMove->events.numbersLow.watchBaseCount = 0;
    thisMove->processing = FALSE;

    listAddNodeBefore(&(thisMove->listNode), &(newMove->listNode), newMove);
}




/*-----------------------------------------------------------------------------
    Name        : aihGenericGettingRockedHandler
    Description : A generic event handler for the "getting rocked" event
    Inputs      : team - the team that's getting rocked,
                  ships - the ships that are attacking the team
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihGenericGettingRockedHandler(struct AITeam *team, SelectCommand *ships)
{
    AITeamMove *thisMove = team->curMove;
    AITeamMove *newMove;
    SelectCommand *attacktargets;

    dbgAssert(ships->numShips > 0);

    if (thisMove->type == MOVE_ATTACK)
    {
        dbgAssert(team->shipList.selection->numShips > 0);
        if (thisMove->params.attack.ships->numShips > 0)
        {
            if (team->shipList.selection->ShipPtr[0]->collMyBlob == thisMove->params.attack.ships->ShipPtr[0]->collMyBlob)
            {
                // we're very close to target, so soldier on without worrying about other attacking ships
                return;
            }
        }
    }

    thisMove->events.gettingRocked.triggered = FALSE;

    attacktargets = getShipAndItsFormation(&universe.mainCommandLayer,ships->ShipPtr[0]);

// later add:
//    if (TargetsAreJustPesky(team->shipList.selection,attacktargets))
//    {
//        memFree(attacktargets);
//        return;
//    }

    if (aiuSelectionNotGoodAtKillingTheseTargets(team->shipList.selection,attacktargets,AIH_GENERIC_GETTINGROCKED_STRENGTH))
    {
        memFree(attacktargets);
        return;
    }

    thisMove->processing = FALSE;

    //add a attack move before
    newMove = aimCreateAttackNoAdd(team, attacktargets,AIH_GENERIC_GETTINGROCKED_FORMATION, TRUE, TRUE);
    newMove->events = thisMove->events;
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
}


/*-----------------------------------------------------------------------------
    Name        : aihPatrolEnemyNearbyHandler
    Description : Attacks enemies nearby
    Inputs      : team - the team who the event belongs to, ships - the ships nearby
    Outputs     : Creates new moves
    Return      : void
----------------------------------------------------------------------------*/
void aihPatrolEnemyNearbyHandler(struct AITeam *team, SelectCommand *ships)
{
    AITeamMove *newMove, *thisMove = team->curMove;

    dbgAssert(ships->numShips > 0);

    if (aiuSelectionNotGoodAtKillingTheseTargets(team->shipList.selection,ships,AIH_PATROL_ENEMYNEARBY_STRENGTH))
    {
        return;
    }

    newMove = aimCreateAttackNoAdd(team, selectMemDupSelection(ships, "duppenh", 0), AIH_PATROL_ENEMYNEARBY_FORMATION, TRUE, TRUE);
    newMove->events = thisMove->events;
    newMove->events.enemyNearby.handler = NULL;
    team->curMove->processing = FALSE;
    aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
}



/*-----------------------------------------------------------------------------
    Name        : aihGravWellEnemyNearbyHandler
    Description : Turns on the grav well generator if fighters are nearby
    Inputs      : team - the grav well generator team, ships - the ships near the gravwell generator
    Outputs     : Turns on the special ability of the gravwell generator
    Return      : void
----------------------------------------------------------------------------*/
void aihGravWellEnemyNearbyHandler(AITeam *team, SelectCommand *ships)
{
    AITeamMove *thisMove = team->curMove, *newMove;
    udword i, numEnemyFighters = 0, numEnemyCorvettes = 0;
//    ShipPtr ship;

    dbgAssert(ships->numShips > 0);

    for (i=0;i<ships->numShips;i++)
    {
        if (isShipOfClass(ships->ShipPtr[i],CLASS_Fighter))
        {
            ++numEnemyFighters;
        }
        else if (isShipOfClass(ships->ShipPtr[i], CLASS_Corvette))
        {
            ++numEnemyCorvettes;
        }

        //if there are enough strikecraft
        if ((numEnemyFighters > 2) || numEnemyCorvettes)
        {
            //if the event hasn't happened yet
            if (!thisMove->events.enemyNotNearby.handler)
            {
                //turn on the gravwell
                aiuWrapSpecial(team->shipList.selection, NULL);
                aieHandlerSetEnemyNotNearby(thisMove, ((GravWellGeneratorStatics *) ((ShipStaticInfo *)(team->shipList.selection->ShipPtr[0]->staticinfo))->custstatinfo)->GravWellRadius,
                                            TRUE, aihGravWellEnemyNotNearbyHandler);
                //set to cooperating team to destroy the trapped ships
                if (team->cooperatingTeam)
                {
                    //later maybe decide if the current move is more important or not
                    newMove = aimCreateAdvancedAttackNoAdd(team->cooperatingTeam, selectMemDupSelection(ships, "gwenh", 0), SAME_FORMATION, Neutral /*later.. Evasive?*/, TRUE, TRUE);
                    aimInsertMove(team->cooperatingTeam, newMove);
                }

            }
            //if the event has happened, new ships may have been captured,
            //add those to the advanced attack move
            else
            {
                if (team->cooperatingTeam)
                {
                    //if the other team has an advanced attack move
                    if (team->cooperatingTeam->curMove->type == MOVE_ADVANCEDATTACK)
                    {
                        if (!SelectionsAreEquivalent(ships, team->cooperatingTeam->curMove->params.advatt.targets))
                        {
                            team->cooperatingTeam->curMove->params.advatt.targets = selectMergeTwoSelections(ships, team->cooperatingTeam->curMove->params.advatt.targets, DEALLOC2);
                        }
                    }
                    else
                    {
                        newMove = aimCreateAdvancedAttackNoAdd(team->cooperatingTeam, selectMemDupSelection(ships, "gwenh", 0), SAME_FORMATION, Neutral /*later.. Evasive?*/, TRUE, TRUE);
                        aimInsertMove(team, newMove);
                    }
                }
            }
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : aihGravWellEnemyNotNearbyHandler
    Description : Turns off the grav well generator no more enemy ships are nearby
    Inputs      : team - the grav well generator team
    Outputs     : Turns off the special ability of the gravwell generator
    Return      : void
----------------------------------------------------------------------------*/
void aihGravWellEnemyNotNearbyHandler(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;

    aiuWrapSpecial(team->shipList.selection, NULL);
    thisMove->events.enemyNotNearby.handler = NULL;
    aieHandlerSetEnemyNearby(thisMove,
                             (((GravWellGeneratorStatics *) ((ShipStaticInfo *)(team->shipList.selection->ShipPtr[0]->staticinfo))->custstatinfo)->GravWellRadius)*0.8,
                             FALSE, aihGravWellEnemyNearbyHandler);
    //later cancel attack move on cooperating team???
}


/*-----------------------------------------------------------------------------
    Name        : aihFastDefenseDistressHandler
    Description : Handles distress calls for fast defense teams
    Inputs      : team - the team handling the distress call,
                  intvar - the variable that is being watched,
                  in this case the aiplayer->aidDefenseTargets
    Outputs     : Creates new moves to deal with the poopyhead attacking the defensless ship
    Return      : void
----------------------------------------------------------------------------*/
void aihFastDefenseDistressHandler(struct AITeam *team, udword *intvar)
{
    AITeamMove *newMove, *thisMove = team->curMove;
    SelectCommand *enemyShips;
    MaxSelection invadingShips, distressShips;
    udword i;
    TypeOfFormation formation;

    if (aiCurrentAIPlayer->aidDistressShips)
    {
        selSelectionCopy((MaxAnySelection *)&distressShips, (MaxAnySelection *)aiCurrentAIPlayer->aidDistressShips);

        if (aiuRescueShipType((SelectCommand *)&distressShips, team, AdvanceSupportFrigate))
        {
            return;
        }
        if (aiuRescueShipType((SelectCommand *)&distressShips, team, ResourceController))
        {
            return;
        }
    }

    if (aiCurrentAIPlayer->aidInvadingShips)
    {
        selSelectionCopy((MaxAnySelection *)&invadingShips, (MaxAnySelection *)aiCurrentAIPlayer->aidInvadingShips);

//    if (aihTakeoutSmallCombatInvaders(invaderShips, team))
//        return;
//    if (aihTakeoutBigCombatInvaders(invaderShips, team))
//        return;

        //later choose between ships on the distress ships list
        //closest/most important/etc.
        for (i = 0; i < invadingShips.numShips; i++)
        {
            enemyShips = aiuFindNearbyDangerousEnemyShips(invadingShips.ShipPtr[i], AIH_FASTDEF_INVADER_ENEMYDIST);
            selSelectionAddSingleShip((MaxSelection *)enemyShips, invadingShips.ShipPtr[i]);

            if (aitCheckIfOtherDefTeamAnsweringSignalNeedsHelp(team, enemyShips))
            {
                if (enemyShips->numShips > AIH_FASTDEF_INVADER_LOTSFEW_LIMIT)
                {
                    formation = AIH_FASTDEF_INVADER_LOTS_FORMATION;
                }
                else
                {
                    formation = AIH_FASTDEF_INVADER_FEW_FORMATION;
                }

                newMove         = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection(enemyShips, "fddh", 0), formation,Aggressive,TRUE, TRUE);
                newMove->events = thisMove->events;
                if (aiuAttackFeatureEnabled(AIA_KAMIKAZE))
                {
                    aieHandlerSetHealthLow(newMove, AIO_FIGHTER_KAMIKAZE_HEALTH, TRUE, FALSE, aihKamikazeHealthLowHandler);
                }
                newMove->events.interrupt.handler  = NULL;
                newMove->events.numbersLow.handler = NULL;
                team->curMove->processing = FALSE;
                aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
                memFree(enemyShips);
                return;
            }
            memFree(enemyShips);
        }
    }

    if ((aiCurrentAIPlayer->aidDistressShips) && (distressShips.numShips))
    {
        if (aiuRescueShipType((SelectCommand *)&distressShips, team, ResourceCollector))
        {
            return;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aihSlowDefenseDistressHandler
    Description : Handles distress calls for slow defense teams
    Inputs      : team - the team handling the distress call,
                  intvar - the variable that is being watched,
                  in this case the aiplayer->aidDefenseTargets
    Outputs     : Creates new moves to deal with the poopyhead attacking the defensless ship
    Return      : void
----------------------------------------------------------------------------*/
void aihSlowDefenseDistressHandler(struct AITeam *team, udword *intvar)
{
    AITeamMove *newMove, *thisMove = team->curMove;
    SelectCommand *enemyShips;
    MaxSelection distressShips, invadingShips;
    udword i;
    real32 distsq_to_ships;
    TypeOfFormation formation;

    if (!team->shipList.selection->numShips)
    {
        // just in case
        return;
    }

    if (aiCurrentAIPlayer->aidDistressShips)
    {
        selSelectionCopy((MaxAnySelection *)&distressShips, (MaxAnySelection *)aiCurrentAIPlayer->aidDistressShips);

        if (aiuRescueShipType((SelectCommand *)&distressShips, team, AdvanceSupportFrigate))
        {
            return;
        }
        if (aiuRescueShipType((SelectCommand *)&distressShips, team, ResourceController))
        {
            return;
        }
    }

    if (aiCurrentAIPlayer->aidInvadingShips)
    {
        selSelectionCopy((MaxAnySelection *)&invadingShips, (MaxAnySelection *)aiCurrentAIPlayer->aidInvadingShips);

        //later choose between ships on the distress ships list
        //closest/most important/etc.
        for (i = 0; i < invadingShips.numShips; i++)
        {
            enemyShips = aiuFindNearbyDangerousEnemyShips(invadingShips.ShipPtr[i], AIH_SLOWDEF_INVADER_ENEMYDIST);
            selSelectionAddSingleShip((MaxSelection *)enemyShips, invadingShips.ShipPtr[i]);
            distsq_to_ships = aiuFindDistanceSquared(team->shipList.selection->ShipPtr[0]->posinfo.position,
                                                     invadingShips.ShipPtr[i]->posinfo.position);

            //if other teams need help and if the enemy is less than 10 km away...
            if ((aitCheckIfOtherDefTeamAnsweringSignalNeedsHelp(team, enemyShips)) &&
                (distsq_to_ships < AIH_SLOWDEF_RESPONSE_MAXDISTSQ))
            {
                if (enemyShips->numShips > AIH_SLOWDEF_INVADER_LOTSFEW_LIMIT)
                {
                    formation = AIH_SLOWDEF_INVADER_LOTS_FORMATION;
                }
                else
                {
                    formation = AIH_SLOWDEF_INVADER_FEW_FORMATION;
                }

                newMove         = aimCreateAdvancedAttackNoAdd(team, selectMemDupSelection(enemyShips, "sddh", 0), formation,Aggressive,TRUE, TRUE);
                newMove->events = thisMove->events;
                if (aiuAttackFeatureEnabled(AIA_KAMIKAZE))
                {
                    if (aitTeamShipClassIs(CLASS_Corvette, team))
                    {
                        aieHandlerSetHealthLow(newMove, AIO_CORVETTE_KAMIKAZE_HEALTH, TRUE, FALSE, aihKamikazeHealthLowHandler);
                    }
                }
                newMove->events.interrupt.handler  = NULL;
                newMove->events.numbersLow.handler = NULL;
                team->curMove->processing = FALSE;
                aitAddmoveBeforeAndMakeCurrent(team, newMove, thisMove);
                memFree(enemyShips);
                return;
            }
            memFree(enemyShips);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aihFastDefenseTeamDiedHandler
    Description : Restarts the fast defense team
    Inputs      : team - the team that died
    Outputs     : Deletes all the previous moves and creates new ones
    Return      : void
----------------------------------------------------------------------------*/
void aihFastDefenseTeamDiedHandler(struct AITeam *team)
{
    aitDeleteAllTeamMoves(team);
    aioCreateFastRovingDefense(team);
}


/*-----------------------------------------------------------------------------
    Name        : aihSlowDefenseTeamDiedHandler
    Description : Restarts the slow defense team
    Inputs      : team - the team that died
    Outputs     : Deletes all the previous moves and creates new ones
    Return      : void
----------------------------------------------------------------------------*/
void aihSlowDefenseTeamDiedHandler(struct AITeam *team)
{
    aitDeleteAllTeamMoves(team);
    aioCreateSlowRovingDefense(team);
}


/*-----------------------------------------------------------------------------
    Name        : aihGuardShipsTeamDiedHandler
    Description : Restarts the guard ships team
    Inputs      : team - the team that died
    Outputs     : Deletes all the previous moves and create new ones
    Return      : void
----------------------------------------------------------------------------*/
void aihGuardShipsTeamDiedHandler(struct AITeam *team)
{
    MaxSelection tempsel;

    selSelectionCopy((MaxAnySelection *)&tempsel, (MaxAnySelection *)team->curMove->params.guardShips.ships);

    aitDeleteAllTeamMoves(team);
    aioCreateGuardShips(team, selectMemDupSelection((SelectCommand *)&tempsel, "gustdh", 0));
}


/*-----------------------------------------------------------------------------
    Name        : aihReconaissanceTeamDiedHandler
    Description : Nothing yet
    Inputs      : team - the team that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aihReconaissanceTeamDiedHandler(struct AITeam *team)
{
    aitDeleteAllTeamMoves(team);

    //later maybe make more distinction between general recon and enemy recon
    aioCreateReconaissance(team, RECON_ACTIVE_ENEMY);
}


/*-----------------------------------------------------------------------------
    Name        : aihReconShipTeamDiedHandler
    Description : Restarts the recon team
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void  aihReconShipTeamDiedHandler(struct AITeam *team)
{
    aitDeleteAllTeamMoves(team);

    //later somehow continue the recon for ships
    aioCreateReconaissance(team, RECON_ACTIVE_ENEMY);
}



/*-----------------------------------------------------------------------------
    Name        : aihHarassTeamDiedHandler
    Description : Restarts the harass team
    Inputs      : team - the team that died
    Outputs     : Deletes all the previous moves and creates new ones
    Return      : void
----------------------------------------------------------------------------*/
void aihHarassTeamDiedHandler(struct AITeam *team)
{
	if (!bitTest(team->teamFlags, AIT_Reissue01))
	{
		bitSet(team->teamFlags, AIT_Reissue01);
	}
	else
	{
		if (!bitTest(team->teamFlags, AIT_Reissue10))
		{
			bitSet(team->teamFlags, AIT_Reissue10);
		}
		else
		{
			// if we've tried harass teams twice, destroy the team
			bitClear(team->teamFlags, AIT_Reissue10|AIT_Reissue01);
			bitSet(team->teamFlags, AIT_DestroyTeam);
			return;
		}
	}
	if (randyrandom(RAN_AIPlayer, 4))
	{
		// retry this puppy
		aitDeleteAllTeamMoves(team);
		aioCreateHarass(team);
	}
	else
	{
		bitSet(team->teamFlags, AIT_DestroyTeam);
	}
}


/*-----------------------------------------------------------------------------
    Name        : aihPatrolTeamDiedHandler
    Description : Restarts the patrol team
    Inputs      : team - the team that died
    Outputs     : Flags the team to be destroyed
    Return      : void
----------------------------------------------------------------------------*/
void aihPatrolTeamDiedHandler(struct AITeam *team)
{
    bitSet(team->teamFlags, AIT_DestroyTeam);
//    aitDestroy(aiCurrentAIPlayer, team, TRUE);
}


/*-----------------------------------------------------------------------------
    Name        : aihRemoveTeamDiedHandler
    Description : Removes the team that died
    Inputs      : team - the team that died
    Outputs     : Flags the team to be destroyed
    Return      : void
----------------------------------------------------------------------------*/
void aihRemoveTeamDiedHandler(struct AITeam *team)
{
    bitSet(team->teamFlags, AIT_DestroyTeam);
//    aitDestroy(aiCurrentAIPlayer, team, TRUE);
}










