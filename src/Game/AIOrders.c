
#include "Types.h"
#include "FastMath.h"
#include "AIOrders.h"
#include "AITeam.h"
#include "AIMoves.h"
#include "ShipSelect.h"
#include "Debug.h"
#include "AIUtilities.h"
#include "AIPlayer.h"
#include "AIEvents.h"
#include "AIHandler.h"
#include "CommandWrap.h"
#include "Randy.h"
#include "GravWellGenerator.h"

/*=============================================================================
    Orders Constant Definitions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aioCreateGuardShips
    Description : Creates an order to get a team to guard ships
    Inputs      : team - the team that will execute the order, ships - the ships to guard
    Outputs     : Creates a whole bunch of orders
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateGuardShips(AITeam *team, SelectCommand *ships)
{
    AlternativeShips alternatives;
    sbyte numShipsToUse;
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Guard Ships Order", team));

    SetNumAlternativesFlags(alternatives,5, ALTERNATIVE_RANDOM)
//    SetAlternative(alternatives,0,DefenseFighter,10);
    SetAlternative(alternatives,0,HeavyInterceptor,15);
    SetAlternative(alternatives,1,LightInterceptor,10);
    SetAlternative(alternatives,2,LightCorvette,18);
    SetAlternative(alternatives,3,HeavyCorvette,24);
    SetAlternative(alternatives,4,MultiGunCorvette,24);

    if (aiuAnyShipsAreCapitalShips(ships))
    {
        numShipsToUse = AIO_GUARD_SHIPS_NUM_CAPITAL;
    }
    else
    {
        numShipsToUse = AIO_GUARD_SHIPS_NUM_NONCAPITAL;
    }

    aimCreateFancyGetShips(team, HeavyDefender, numShipsToUse, &alternatives, 0, TRUE, FALSE);

#if 0
    if (ships->numShips > 2)
    {
        numShipsToUse = ships->numShips * 3;
    }
    else
    {
        numShipsToUse = ships->numShips * 4;
    }
#endif
#if 0
    if (ships->numShips > 2)
    {
        aimCreateGetShips(team, HeavyDefender, 6, 0, TRUE, FALSE);
        aimCreateGetShips(team, MultiGunCorvette, (sbyte)(ships->numShips-2), 0, TRUE, FALSE);
    }
    else
        aimCreateGetShips(team, HeavyDefender, (sbyte)(ships->numShips*3), 0, TRUE, FALSE);
#endif

    aimCreateFormation(team, SPHERE_FORMATION, FALSE, FALSE);  // vary the formation?
    move = aimCreateGuardShips(team, ships, FALSE, FALSE);
    aieHandlerSetFuelLow(move, AIO_GUARD_SHIPS_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetTeamDied(move, aihGuardShipsTeamDiedHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}



/*-----------------------------------------------------------------------------
    Name        : aioCreateReconaissance
    Description : Creates a the moves for a team to do a reconaissance mission
    Inputs      : team - the team to do the reconaissancing
    Outputs     : Creates a whole bunch of moves for the team
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateReconaissance(AITeam *team, ReconType type)
{
    vector destination, origin = ORIGIN_VECTOR;
    AlternativeShips alternatives;
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Reconaissance Order", team));

    switch (type)
    {
        case RECON_MOTHERSHIP:
            //later if there's more than one enemy, need to find both of their mo'ships
            destination = aiuFindEnemyMothershipCoords(aiCurrentAIPlayer->player);
            destination = aiuGenerateRandomStandoffPoint(destination, RECON_STANDOFF_DISTANCE, origin, RSP_NEAR);

            SetNumAlternatives(alternatives, 2);
            SetAlternative(alternatives,0,CloakedFighter,10);
            SetAlternative(alternatives,1,LightInterceptor,10);

            aimCreateFancyGetShips(team, Probe, 1, &alternatives, REQUESTSHIPS_HIPRI, TRUE, FALSE);
            aimCreateMoveTeam(team, destination, NO_FORMATION, TRUE, FALSE);
            aimCreateCountShips(team, FALSE, FALSE);
            move = aimCreateActiveRecon(team, TRUE, NO_FORMATION, Evasive, TRUE, FALSE);
            aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);
            aieHandlerSetTeamDied(move, aihReconaissanceTeamDiedHandler);
            aimCreateMoveDone(team, FALSE, FALSE);
            break;

        case RECON_ACTIVE_GENERAL:
            SetNumAlternatives(alternatives, 1);
            SetAlternative(alternatives,0,CloakedFighter,10);

            aimCreateFancyGetShips(team, LightInterceptor, 1, &alternatives, REQUESTSHIPS_HIPRI, TRUE, FALSE);
            move = aimCreateActiveRecon(team, FALSE, NO_FORMATION, Evasive, TRUE, FALSE);
            aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);
            aieHandlerSetTeamDied(move, aihReconaissanceTeamDiedHandler);
            aimCreateMoveDone(team, FALSE, FALSE);
            break;

        case RECON_ACTIVE_ENEMY:
            SetNumAlternatives(alternatives, 1);
            SetAlternative(alternatives,0,CloakedFighter,10);

            aimCreateFancyGetShips(team, LightInterceptor, 1, &alternatives, REQUESTSHIPS_HIPRI, TRUE, FALSE);
            move = aimCreateActiveRecon(team, TRUE, NO_FORMATION, Evasive, TRUE, FALSE);
            aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);
            aieHandlerSetTeamDied(move, aihReconaissanceTeamDiedHandler);
            aimCreateMoveDone(team, FALSE, FALSE);
            break;

        default:
            break;
    }


}



/*-----------------------------------------------------------------------------
    Name        : aioCreateHarass
    Description : Creates the moves for a team to perform the harass order
    Inputs      : team - the team to do the harassing
    Outputs     : Creates a whole bunch of moves for the team
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateHarass(AITeam *team)
{
    vector destination, origin = ORIGIN_VECTOR;
    AlternativeShips alternatives;
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Harass Order", team));

    bitSet(team->teamFlags, HARASS_TEAM);

    destination = aiuFindEnemyMothershipCoords(aiCurrentAIPlayer->player);
    destination = aiuFindRangeStandoffPoint(destination, origin, HARASS_STANDOFF_DISTANCE);

    SetNumAlternativesFlags(alternatives,6,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,HeavyInterceptor,HARASS_HEAVYINT_EQUIVNUM);
    SetAlternative(alternatives,1,HeavyInterceptor,HARASS_HEAVYINT_EQUIVNUM);
    SetAlternative(alternatives,2,AttackBomber,HARASS_BOMBER_EQUIVNUM);
    SetAlternative(alternatives,3,AttackBomber,HARASS_BOMBER_EQUIVNUM);
    SetAlternative(alternatives,4,CloakedFighter,10);
    SetAlternative(alternatives,5,CloakedFighter,10);

    aimCreateFancyGetShips(team, LightInterceptor, HARASS_LIGHTINT_INITNUM, &alternatives, REQUESTSHIPS_HIPRI, TRUE, FALSE);

    move = aimCreateMoveTeam(team, destination, AIO_HARASS_INITIAL_MOVE_FORMATION, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_HARASS_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(move, AIO_HARASS_NUMBERS_LOW, 0, TRUE, aihHarassNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetTeamDied(move, aihHarassTeamDiedHandler);

//    aimCreateFormation(team, CLAW_FORMATION, FALSE, FALSE);

    move = aimCreateHarassAttack(team, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_HARASS_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(move, AIO_HARASS_NUMBERS_LOW, 0, TRUE, aihHarassNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetTeamDied(move, aihHarassTeamDiedHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateReconHarass
    Description : Creates the moves for a team to do a recon/harass mission
    Inputs      : team - the team to do the recon/harassing
    Outputs     : Creates a whole bunch of moves for the team
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateReconHarass(AITeam *team)
{
    vector destination, origin = {0,0,0};
    AITeamMove *move;
    AlternativeShips alternatives;

    aiplayerLog((aiIndex, "%x Issuing Recon Harass Order", team));

    bitSet(team->teamFlags, HARASS_TEAM);

    destination = aiuFindEnemyMothershipCoords(aiCurrentAIPlayer->player);
    destination = aiuFindRangeStandoffPoint(destination, origin, HARASS_STANDOFF_DISTANCE);

    SetNumAlternatives(alternatives,2);
    SetAlternative(alternatives,0,HeavyInterceptor,HARASS_HEAVYINT_EQUIVNUM);
    SetAlternative(alternatives,1,AttackBomber,HARASS_BOMBER_EQUIVNUM);

    aimCreateFancyGetShips(team, LightInterceptor, HARASS_LIGHTINT_INITNUM, &alternatives, 0, TRUE, FALSE);

    move = aimCreateMoveTeam(team, destination, AIO_HARASS_INITIAL_MOVE_FORMATION, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_HARASS_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(move, AIO_HARASS_NUMBERS_LOW, 0, TRUE, aihHarassNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetTeamDied(move, aihHarassTeamDiedHandler);

    aimCreateCountShips(team, FALSE, FALSE);
//    aimCreateFormation(team, CLAW_FORMATION, FALSE, FALSE);

    move = aimCreateHarassAttack(team, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_HARASS_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(move, AIO_HARASS_NUMBERS_LOW, 0, TRUE, aihHarassNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetTeamDied(move, aihHarassTeamDiedHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : aioCreateDefendMothership
    Description : Creates the moves for a team to defend the mothership
                  - this order is not a long term one, it is there for
                    immediate defense of the mothership when it's
                    under attack
    Inputs      : team - the team the order is meant for
                  enemy - the dirty rotten scum sucking low lifes attacking
                  the poor mothership
    Outputs     : Creates a few moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateDefendMothership(AITeam *team)
{
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Defend Mothership Order", team));

    //note: ships are assigned to these teams by the defense manager
    //      so there is no getships move
    move = aimCreateDefendMothership(team, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_DEFMOTHERSHIP_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreatePatrol
    Description : Creates the moves for a team to patrol a certain path
    Inputs      : team - the team to do the patrolling
                  path - the path to follow
    Outputs     : Creates a bunch of moves for the team
    Return      : void
----------------------------------------------------------------------------*/
//note: this function isn't really used.
//      instead, use fast roving or slow roving defense
void aioCreatePatrol(AITeam *team, Path *path)
{
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Patrol Order", team));

    aimCreateGetShips(team, HeavyCorvette, 3, 0, TRUE, FALSE);
//    aimCreateFormation(team, DELTA_FORMATION, FALSE, FALSE);
    move = aimCreatePatrolMove(team, path, 0, DELTA_FORMATION, Aggressive, TRUE, FALSE);
    //create a enemynearby handler as well...
    aieHandlerSetFuelLow(move, AIO_PATROL_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(move, AIO_PATROL_NUMBERS_LOW, 3, TRUE, aihHarassNumbersLowHandler);
//    aieHandlerSetTeamDied(move, aihPatrolTeamDied);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateFastRovingDefense
    Description : Creates a team of roving ships that guard against baddies
    Inputs      : team - the team to do the roving
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateFastRovingDefense(AITeam *team)
{
    AlternativeShips alternatives;
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Fast Roving Defense Order", team));

    SetNumAlternativesFlags(alternatives,6, ALTERNATIVE_RANDOM)
    SetAlternative(alternatives,0,HeavyDefender,14);
    SetAlternative(alternatives,1,HeavyDefender,14);
    SetAlternative(alternatives,2,HeavyDefender,14);
    SetAlternative(alternatives,3,HeavyInterceptor,14);
    SetAlternative(alternatives,4,AttackBomber,15);
    SetAlternative(alternatives,5,AttackBomber,15);

    aimCreateFancyGetShips(team, LightInterceptor, 15, &alternatives, 0, TRUE, FALSE);
//    aimCreateFormation(team, WALL_FORMATION, FALSE, FALSE);
    move = aimCreateActivePatrol(team, AIT_FAST_PATROL, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_FASTROVING_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetTeamDied(move, aihFastDefenseTeamDiedHandler);
    aieHandlerSetNumbersLow(move, AIO_FASTROVING_NUMBERS_LOW, 0, FALSE, aihFastDefenseNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetInterrupt(move, (udword *)&(aiCurrentAIPlayer->aidDefenseTargets), 0, FALSE, aihFastDefenseDistressHandler);

//    aieHandlerSetEnemyNearby(move, 2000, FALSE, aihPatrolEnemyNearbyHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateSlowRovingDefense
    Description : Creates a team of slowly roving ships that guard against baddies
    Inputs      : team - the team to the roving
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateSlowRovingDefense(AITeam *team)
{
    AlternativeShips alternatives;
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Slow Roving Defense Order", team));

    SetNumAlternativesFlags(alternatives,7,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,HeavyCorvette,10);
    SetAlternative(alternatives,1,HeavyCorvette,10);
    SetAlternative(alternatives,2,HeavyCorvette,10);
    SetAlternative(alternatives,3,MultiGunCorvette,10);
    SetAlternative(alternatives,4,LightCorvette,7);
    SetAlternative(alternatives,5,StandardFrigate,30);
    SetAlternative(alternatives,6,DDDFrigate,30);

    aimCreateFancyGetShips(team, MultiGunCorvette, 3, &alternatives, 0, TRUE, FALSE);
//    aimCreateFormation(team, DELTA_FORMATION, FALSE, FALSE);
    move = aimCreateActivePatrol(team, AIT_SLOW_PATROL, TRUE, FALSE);
    aieHandlerSetFuelLow(move, AIO_SLOWROVING_FUEL_LOW, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetTeamDied(move, aihSlowDefenseTeamDiedHandler);
    aieHandlerSetNumbersLow(move, AIO_SLOWROVING_NUMBERS_LOW, 0, FALSE, aihSlowDefenseNumbersLowHandler);
    aieHandlerSetGettingRocked(move, FALSE, aihGenericGettingRockedHandler);
    aieHandlerSetInterrupt(move, (udword *)&(aiCurrentAIPlayer->aidDefenseTargets), 3, FALSE, aihSlowDefenseDistressHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : aioCreateReinforcements
    Description : Creates the moves to get the team to reinforce the other team
    Inputs      : team - the team that will be reinforcing,
                  reinforceteam - the team to reinforce
                  shiptype - the type of ships to reinforce with
                  num - the number of ships to reinforce with
                    NOTE: if num == 0, the team is considered to already have ships
                          and none are requested
                  alternatives - alternative ships for reinforcement
                    NOTE: if alternatives == NULL, no alternatives are requested
                  priority - the priority of the build request
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateReinforcements(AITeam *team, AITeam *reinforceteam, ShipType shiptype,
                             sbyte num, AlternativeShips *alternatives, sdword priority)
{
    aiplayerLog((aiIndex, "%x Issuing Reinforcements Order", team));

    if (num)
    {
        if (alternatives)
        {
            aimCreateFancyGetShips(team, shiptype, num, alternatives, priority, TRUE, FALSE);
        }
        else
        {
            aimCreateGetShips(team, shiptype, num, priority, TRUE, FALSE);
        }
    }

    aimCreateReinforce(team, reinforceteam, BROAD_FORMATION, Evasive, TRUE, FALSE);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateActiveSupport
    Description : Creates the moves to get a team to support another team (i.e. AdvanceSupportFrigate and Repair Corvette)
    Inputs      : team - the team doing the supporting,
                  ships - the ships to support
                    NOTE: ships can be NULL.
    Outputs     : Creates a whole whack of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateActiveSupport(AITeam *team, SelectCommand *ships, SupportType type)
{
    AlternativeShips alternatives;

    aiplayerLog((aiIndex, "%x Issuing Active Support Order", team));

    if (type == SUPPORT_STRIKECRAFT)
    {
        if (!team->shipList.selection->numShips)
        {
            SetNumAlternatives(alternatives, 1);
            SetAlternative(alternatives,0,RepairCorvette,3);

            aimCreateFancyGetShips(team, AdvanceSupportFrigate, 1, &alternatives, 0, TRUE, FALSE);
        }

        aimCreateSupport(team, ships, BROAD_FORMATION, Evasive, TRUE, FALSE);
    }
    else if (type == SUPPORT_RESOURCE)
    {
        if (!team->shipList.selection->numShips)
        {
            aimCreateGetShips(team, ResourceController, 1, 0, TRUE, FALSE);
        }

        aimCreateControlResources(team, ships, TRUE, FALSE);
    }
    else if (type == SUPPORT_MOTHERSHIP)
    {
        dbgAssert(team->shipList.selection->numShips);

        aimCreateMothershipMove(team, TRUE, FALSE);
    }
    else
    {
        aiplayerLog((aiIndex, "Error - unknown support type issued"));
        dbgAssert(FALSE);
    }

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateFighterStrike
    Description : Creates the moves to create and do stuff with a fighter
                  strike force
    Inputs      : team
    Outputs     : Creates a bunch'a moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateFighterStrike(AITeam *team)
{
    AlternativeShips alternatives;
    AITeamMove *newmove;

    aiplayerLog((aiIndex, "%x Issuing Fighter Strike Order", team));

    SetNumAlternativesFlags(alternatives,9,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,LightInterceptor,12);
    SetAlternative(alternatives,1,HeavyInterceptor,7);
    SetAlternative(alternatives,2,HeavyInterceptor,7);
    SetAlternative(alternatives,3,HeavyInterceptor,7);
    SetAlternative(alternatives,4,AttackBomber,10);
    SetAlternative(alternatives,5,AttackBomber,10);
    SetAlternative(alternatives,6,AttackBomber,10);
    SetAlternative(alternatives,7,HeavyDefender,8);
    SetAlternative(alternatives,8,CloakedFighter,10);

    aimCreateFancyGetShips(team, AttackBomber, 15, &alternatives, 0, TRUE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//later put formation in aiscript
    newmove = aimCreateTempGuard(team, DELTA_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(newmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    newmove = aimCreateArmada(team, CLAW_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(newmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateCorvetteStrike
    Description : Creates the moves to create and do stuff with a fighter
                  strike force
    Inputs      : team
    Outputs     : Creates a bunch'a moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateCorvetteStrike(AITeam *team)
{
    AlternativeShips alternatives;
    AITeamMove *newmove;

    aiplayerLog((aiIndex, "%x Issuing Corvette Strike Order", team));

    SetNumAlternativesFlags(alternatives,7,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,LightCorvette,13);
    SetAlternative(alternatives,1,HeavyCorvette,10);
    SetAlternative(alternatives,2,HeavyCorvette,10);
    SetAlternative(alternatives,3,HeavyCorvette,10);
    SetAlternative(alternatives,4,HeavyCorvette,10);
    SetAlternative(alternatives,5,MultiGunCorvette,7);
    SetAlternative(alternatives,6,MinelayerCorvette,7);

    aimCreateFancyGetShips(team, HeavyCorvette, 9, &alternatives, 0, TRUE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//later put formation in aiscript
    newmove = aimCreateTempGuard(team, DELTA3D_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(newmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    newmove = aimCreateArmada(team, WALL_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(newmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateFrigateStrike
    Description : Creates the moves to create and do stuff with a fighter
                  strike force
    Inputs      : team
    Outputs     : Creates a bunch'a moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateFrigateStrike(AITeam *team)
{
    AlternativeShips alternatives;
    AITeamMove *newmove;

    aiplayerLog((aiIndex, "%x Issuing Frigate Strike Order", team));

    SetNumAlternativesFlags(alternatives,1,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,IonCannonFrigate,10);
//    SetAlternative(alternatives,1,DDDFrigate,10);

    aimCreateFancyGetShips(team, StandardFrigate, 5, &alternatives, 0, TRUE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//later put formation in aiscript
    newmove = aimCreateTempGuard(team, WALL_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    newmove = aimCreateArmada(team, CLAW_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(newmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetNumbersLow(newmove, 20, 0, TRUE, aihHarassNumbersLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);

    //this team can hyperspace attack
    bitSet(team->teamFlags, TEAM_Hyperspaceable);

}


/*-----------------------------------------------------------------------------
    Name        : aioCreateResourcer
    Description : Creats the moves to have a team collect resources autonomously
    Inputs      : team - the team
    Outputs     : Creates sum moooooves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateResourcer(AITeam *team)
{
    aiplayerLog((aiIndex, "%x Issuing Resourcer Order", team));

    //assumes that ships will be added, therefore does not request them
    aimCreateActiveResource(team, TRUE, FALSE);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateCapture
    Description : Creates a team to do some capturin'
    Inputs      : team - the team to do some capturin'
    Outputs     : Creates some new moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateCapture(AITeam *team)
{
    aiplayerLog((aiIndex, "%x Issuing Capture Order", team));

    aimCreateGetShips(team, SalCapCorvette, (sbyte)(randyrandombetween(RAN_AIPlayer, 3, 7)), 0, TRUE, FALSE);

    aimCreateActiveCapture(team, TRUE, FALSE);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateMine
    Description : Creates a team to do some minin'
    Inputs      : team - the minin' team
    Outputs     : Creates some new moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateMine(AITeam *team)
{
    aiplayerLog((aiIndex, "%x Issuing Mine Order", team));

    aimCreateGetShips(team, MinelayerCorvette, (sbyte)(randyrandombetween(RAN_AIPlayer, 2, 5)), 0, TRUE, FALSE);

    aimCreateActiveMine(team, TRUE, FALSE);
    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateSpecialDefense
    Description : Creates a team to do some cloaking
    Inputs      : team - the team to do the cloaking
    Outputs     : Creates some new moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateSpecialDefense(AITeam *team, ShipType type)
{
    AITeamMove *move;
    ShipStaticInfo *gravstatics;

    aiplayerLog((aiIndex, "%x Issuing Special Defense Order", team));

    aimCreateGetShips(team, type, 1, 0, TRUE, FALSE);

    move = aimCreateSpecialDefense(team, TRUE, FALSE);

    if (type == GravWellGenerator)
    {
        gravstatics = GetShipStaticInfo(GravWellGenerator, R1);

        if (gravstatics)
        {
            aieHandlerSetEnemyNearby(move,
                                     (((GravWellGeneratorStatics *) gravstatics->custstatinfo)->GravWellRadius)*0.8,
                                     FALSE, aihGravWellEnemyNearbyHandler);
            aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);
        }
        else
        {
            dbgAssert(FALSE);
        }
    }

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateSwarmAttack
    Description : Creates a team of swarmers (P2 Pirates)
    Inputs      : team - the team to do the swarming
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateSwarmAttack(AITeam *team)
{
    AITeamMove *newmove;

    aiplayerLog((aiIndex, "Issuing Swarm Attack Order"));

    newmove = aimCreateSwarmAttack(team, TRUE, FALSE);
    aieHandlerSetFuelLow(newmove, 0, TRUE, FALSE, aihSwarmerEmptyFuelHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : aioCreateSwarmDefense
    Description : Creates a team of swarmers (P2 Pirates)
    Inputs      : team - the team to do the swarming
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateSwarmDefense(AITeam *team, SelectCommand *Pod)
{
    AITeamMove *newmove;

    aiplayerLog((aiIndex, "Issuing Swarm Defense Order"));

    newmove = aimCreateSwarmDefense(team, Pod, TRUE, FALSE);
    aieHandlerSetFuelLow(newmove, 0, TRUE, FALSE, aihSwarmerEmptyFuelHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateSwarmSupport
    Description : Creates a team of swarmers (P2 Pirates)
    Inputs      : team - the team to do the swarming
    Outputs     : Creates a bunch of moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateSwarmSupport(AITeam *team)
{
    aiplayerLog((aiIndex, "Issuing Swarm Support Order"));

    aimCreateSwarmPod(team, TRUE, FALSE);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateMultiBeamAttack
    Description : Creates the moves to make the P2 Multi Beam Frigates pulverize the enemy
    Inputs      : team - the multibeamFrigate team
    Outputs     : Creates a bunch'a moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateMultiBeamAttack(AITeam *team)
{
    SelectCommand *enemy;

    aiuNewPyroSelection(enemy, 1, "mbam");
    selSelectionAddSingleShip((MaxSelection *)enemy, aiuFindEnemyMothership(aiCurrentAIPlayer->player));

    aimCreateFlankAttack(team, enemy, FALSE, TRUE, TRUE);

    aimCreateMoveDone(team, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : aioCreateP2MothershipAttack
    Description : Creates the moves to make the P2 Mothership pulverize the enemy
    Inputs      : team - the multibeamFrigate team
    Outputs     : Creates a bunch'a moves
    Return      : void
----------------------------------------------------------------------------*/
void aioCreateP2MothershipAttack(AITeam *team)
{
    SelectCommand *enemy;

    aiuNewPyroSelection(enemy, 1, "p2am");
    selSelectionAddSingleShip((MaxSelection *)enemy, aiuFindEnemyMothership(aiCurrentAIPlayer->player));

    aimCreateAttack(team, enemy, NO_FORMATION, TRUE, TRUE);

    aimCreateMoveDone(team, FALSE, FALSE);
}





//
//  temporary contention relief
//
#include "AIOrders2.c.h"  // Gary


