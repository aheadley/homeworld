
#include "Types.h"
#include "SpaceObj.h"
#include "Stats.h"
#include "AIPlayer.h"
#include "AIUtilities.h"
#include "CommandLayer.h"

void aioCreateTakeoutMothershipFast(struct AITeam *team,Ship *mothership)
{
    AlternativeShips alternatives;
//    SelectCommand *selectone;
    AITeamMove *attackmove;

    aiplayerLog((aiIndex, "%x Issuing Takeout Mothership Fast Order", team));

    SetNumAlternativesFlags(alternatives,11,ALTERNATIVE_RANDOM);
    SetAlternative(alternatives,0,IonCannonFrigate,10);
    SetAlternative(alternatives,1,IonCannonFrigate,10);
    SetAlternative(alternatives,2,IonCannonFrigate,10);
    SetAlternative(alternatives,3,IonCannonFrigate,30);
    SetAlternative(alternatives,4,MissileDestroyer,30);
    SetAlternative(alternatives,5,StandardDestroyer,30);
    SetAlternative(alternatives,6,StandardFrigate,10);
    SetAlternative(alternatives,7,StandardFrigate,10);
    SetAlternative(alternatives,8,MinelayerCorvette,7);
    SetAlternative(alternatives,9,HeavyCorvette,3);
    SetAlternative(alternatives,10,MultiGunCorvette,3);

    aimCreateFancyGetShips(team, IonCannonFrigate, 3, &alternatives, 0, TRUE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//    aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
    aimCreateTempGuard(team, AIO_TOUT_MSHIP_FAST_TGRD_FORMATION, AIO_TOUT_MSHIP_FAST_TGRD_TACTICS, TRUE, FALSE);

    // aimCreateRequestGuardTeam();

//    selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
//    selectone->numShips = 1;
//    selectone->ShipPtr[0] = mothership;
//    attackmove = aimCreateFlankAttack(team, selectone, FALSE, TRUE, FALSE);
    attackmove = aimCreateArmada(team, BROAD_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(attackmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);

    //this team can hyperspace attack
    bitSet(team->teamFlags, TEAM_Hyperspaceable);
}

void aioCreateTakeoutMothershipBig(struct AITeam *team,Ship *mothership, bool ForceBig)
{
    AlternativeShips alternatives1,alternatives2;
//    SelectCommand *selectone;
    AITeamMove *attackmove;

    if (!aiCurrentAIPlayer->player->PlayerMothership)
    {
        return;
    }

    aiplayerLog((aiIndex, "%x Issuing Takeout Mothership Big Order", team));

    if ((aiCurrentAIPlayer->player->PlayerMothership->shiptype == Mothership) || ForceBig)
    {
        SetNumAlternativesFlags(alternatives1,1,ALTERNATIVE_RANDOM);
        SetAlternative(alternatives1,0,StandardDestroyer,30);

        SetNumAlternativesFlags(alternatives2,2,ALTERNATIVE_RANDOM);
        SetAlternative(alternatives2,0,MissileDestroyer,30);
        SetAlternative(alternatives2,1,MissileDestroyer,30);
    }
    else if (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Carrier)
    {
        SetNumAlternativesFlags(alternatives1,0,ALTERNATIVE_RANDOM);

        SetNumAlternativesFlags(alternatives2,0,ALTERNATIVE_RANDOM);
//        SetAlternative(alternatives2,0,DDDFrigate,10);
    }
    else
    {
        aiplayerLog((aiIndex, "Unknown Mothership Type"));
        return;
    }

    aimCreateFancyGetShips(team, IonCannonFrigate, 2, &alternatives1, 0, TRUE, FALSE);
    aimCreateFancyGetShips(team, StandardFrigate,  2, &alternatives2, REQUESTSHIPS_HIPRI, TRUE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//    aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
    aimCreateTempGuard(team, AIO_TOUT_MSHIP_BIG_TGRD_FORMATION, AIO_TOUT_MSHIP_BIG_TGRD_TACTICS, TRUE, FALSE);

//    selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
//    selectone->numShips = 1;
//    selectone->ShipPtr[0] = mothership;
//    attackmove = aimCreateAttack(team, selectone, WALL_FORMATION, TRUE, FALSE);
    attackmove = aimCreateArmada(team, WALL_FORMATION, Aggressive, TRUE, FALSE);
    aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(attackmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);

    //this team can hyperspace attack
    bitSet(team->teamFlags, TEAM_Hyperspaceable);
}

void aioCreateTakeoutMothershipHuge(struct AITeam *team,Ship *mothership)
{
    AlternativeShips guardalternatives;
    AlternativeShips alternatives;
    AITeam *secondaryteam;
//    SelectCommand *selectone;
    AIVar *Var0;
    AIVar *Var1;
    char label[AIVAR_LABEL_MAX_LENGTH+1];
    AITeamMove *move;

    aiplayerLog((aiIndex, "%x Issuing Takeout Mothership Huge Order", team));

    secondaryteam = aitCreate(team->teamType);
    secondaryteam->cooperatingTeam = team;

    Var0 = aivarCreate(aivarLabelGenerate(label));
    aivarValueSet(Var0, 0);

    Var1 = aivarCreate(aivarLabelGenerate(label));
    aivarValueSet(Var1, 0);

    SetNumAlternativesFlags(guardalternatives,6,ALTERNATIVE_RANDOM);
    SetAlternative(guardalternatives,0,HeavyInterceptor,10);
    SetAlternative(guardalternatives,1,DefenseFighter,10);
    SetAlternative(guardalternatives,2,LightCorvette,15);
    SetAlternative(guardalternatives,3,HeavyCorvette,20);
    SetAlternative(guardalternatives,4,MultiGunCorvette,25);
//    SetAlternative(guardalternatives,5,DFGFrigate,50);
//    SetAlternative(guardalternatives,6,DDDFrigate,50);
    SetAlternative(guardalternatives,5,MissileDestroyer,100);

    aimCreateFancyGetShips(secondaryteam, HeavyDefender, (sbyte)10, &guardalternatives, 0, TRUE, FALSE);

    aimCreateVarSet(secondaryteam, aivarLabelGet(Var1), TRUE, FALSE, FALSE);

    move = aimCreateVarWait(secondaryteam, aivarLabelGet(Var0), TRUE, TRUE, FALSE);
    aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

    aimCreateVarDestroy(secondaryteam, aivarLabelGet(Var0), FALSE, FALSE);

    aimCreateFormation(secondaryteam, SPHERE_FORMATION, FALSE, FALSE);

    move = aimCreateGuardCooperatingTeam(secondaryteam, FALSE, FALSE);
    aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);

    aimCreateMoveDone(secondaryteam, FALSE, FALSE);

    SetNumAlternatives(alternatives,0);
    aimCreateFancyGetShips(team, HeavyCruiser, 1, &alternatives, 0, TRUE, FALSE);

    aimCreateVarSet(team, aivarLabelGet(Var0), TRUE, FALSE, FALSE);

    move = aimCreateVarWait(team, aivarLabelGet(Var1), TRUE, TRUE, FALSE);
    aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

    aimCreateVarDestroy(team, aivarLabelGet(Var1), FALSE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//    aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
    aimCreateTempGuard(team, NO_FORMATION, AIO_TOUT_MSHIP_GUARD_TGRD_TACTICS, TRUE, FALSE);

    aimCreateArmada(team, NO_FORMATION, Aggressive, TRUE, FALSE);

    aimCreateMoveDone(team, FALSE, FALSE);

    //this team can hyperspace attack
    bitSet(team->teamFlags, TEAM_Hyperspaceable);
}

void aioCreateTakeoutMothershipGuard(struct AITeam *team,Ship *mothership)
{
    AlternativeShips alternatives;
    AlternativeShips guardalternatives;
    AITeam *secondaryteam;
//    SelectCommand *selectone;
    AIVar *Var0;
    AIVar *Var1;
    char label[AIVAR_LABEL_MAX_LENGTH+1];
    AITeamMove *move;

    if (!aiCurrentAIPlayer->player->PlayerMothership)
    {
        return;
    }

    aiplayerLog((aiIndex, "%x Issuing Takeout Mothership Guard Order", team));

    secondaryteam = aitCreate(team->teamType);
    secondaryteam->cooperatingTeam = team;

    Var0 = aivarCreate(aivarLabelGenerate(label));
    aivarValueSet(Var0, 0);

    Var1 = aivarCreate(aivarLabelGenerate(label));
    aivarValueSet(Var1, 0);

    SetNumAlternativesFlags(guardalternatives,6,ALTERNATIVE_RANDOM);
    SetAlternative(guardalternatives,0,HeavyInterceptor,10);
    SetAlternative(guardalternatives,1,LightInterceptor,5);
    SetAlternative(guardalternatives,2,DefenseFighter,10);
    SetAlternative(guardalternatives,3,LightCorvette,15);
    SetAlternative(guardalternatives,4,HeavyCorvette,20);
    SetAlternative(guardalternatives,5,MultiGunCorvette,25);
//    SetAlternative(guardalternatives,6,DFGFrigate,100);
//    SetAlternative(guardalternatives,7,DDDFrigate,100);

    aimCreateFancyGetShips(secondaryteam, HeavyDefender, (sbyte)10, &guardalternatives, 0, TRUE, FALSE);

    aimCreateVarSet(secondaryteam, aivarLabelGet(Var1), TRUE, FALSE, FALSE);

    move = aimCreateVarWait(secondaryteam, aivarLabelGet(Var0), TRUE, TRUE, FALSE);
    aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

    aimCreateVarDestroy(secondaryteam, aivarLabelGet(Var0), FALSE, FALSE);

    aimCreateFormation(secondaryteam, SPHERE_FORMATION, FALSE, FALSE);

    aimCreateGuardCooperatingTeam(secondaryteam, FALSE, FALSE);

    aimCreateMoveDone(secondaryteam, FALSE, FALSE);

    if (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Mothership)
    {
        SetNumAlternativesFlags(alternatives,5,ALTERNATIVE_RANDOM);
        SetAlternative(alternatives,0,MissileDestroyer,30);
        SetAlternative(alternatives,1,StandardDestroyer,30);
        SetAlternative(alternatives,2,StandardFrigate,10);
        SetAlternative(alternatives,3,MinelayerCorvette,7);
        SetAlternative(alternatives,4,HeavyCorvette,7);
    }
    else if (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Carrier)
    {
        SetNumAlternativesFlags(alternatives,3,ALTERNATIVE_RANDOM);
        SetAlternative(alternatives,0,StandardFrigate,10);
        SetAlternative(alternatives,1,MinelayerCorvette,7);
        SetAlternative(alternatives,2,HeavyCorvette,7);
    }
    else
    {
        aiplayerLog((aiIndex, "Unknown Mothership Type"));
        return;
    }

    aimCreateFancyGetShips(team, IonCannonFrigate, 3, &alternatives, 0, TRUE, FALSE);

//    aimCreateFormation(team, WALL_FORMATION, FALSE, FALSE);

    aimCreateVarSet(team, aivarLabelGet(Var0), TRUE, FALSE, FALSE);

    move = aimCreateVarWait(team, aivarLabelGet(Var1), TRUE, TRUE, FALSE);
    aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

    aimCreateVarDestroy(team, aivarLabelGet(Var1), FALSE, FALSE);

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//    aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
    aimCreateTempGuard(team, AIO_TOUT_MSHIP_GUARD_TGRD_FORMATION, AIO_TOUT_MSHIP_GUARD_TGRD_TACTICS, TRUE, FALSE);

//    aimCreateFormation(team, WALL_FORMATION, FALSE, FALSE);

//    selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
//    selectone->numShips = 1;
//    selectone->ShipPtr[0] = mothership;
    move = aimCreateArmada(team, WALL_FORMATION, Aggressive, TRUE, FALSE);
//    move = aimCreateAttack(team, selectone, WALL_FORMATION, TRUE, FALSE);
    aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);

    aimCreateMoveDone(team, FALSE, FALSE);
}

void aioCreateTakeoutTargetsWithCurrentTeam(struct AITeam *team,SelectCommand *targets)
{
    AITeamMove *attackmove;

    dbgAssert(team->shipList.selection->numShips > 0);

    aiplayerLog((aiIndex, "%x Issuing Takeout Targets With Current Team Order", team));

    attackmove = aimCreateAttack(team, targets, AIO_TOUT_TARG_WCUR_FORMATION, /*AIO_TOUT_TARG_WCUR_TACTICS*/  TRUE, FALSE);
    aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(attackmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}

void aioCreateTakeoutTargetWithCurrentTeam(struct AITeam *team,Ship *ship)
{
    SelectCommand *selectone;
    AITeamMove *attackmove;

    dbgAssert(team->shipList.selection->numShips > 0);

    aiplayerLog((aiIndex, "%x Issuing Takeout Target With Current Team Order", team));

    selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
    selectone->numShips = 1;
    selectone->ShipPtr[0] = ship;

    attackmove = aimCreateAttack(team,selectone, AIO_TOUT_TARG_WCUR_FORMATION, /*AIO_TOUT_TARG_WCUR_TACTICS*/ TRUE, FALSE);
    aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}

void aioCreateTakeoutTarget(struct AITeam *team,Ship *target)
{
    ShipStaticInfo *shipsToBuy;
    sdword numShipsToBuy;
    SelectCommand *selectone;
    bool goodEnough;
    AITeamMove *attackmove;

    // check reserves first:
    SelectCommand *useTheseShips = statsBestShipsToUseToKillTarget(aiCurrentAIPlayer->newships.selection,target->staticinfo,&goodEnough);

    aiplayerLog((aiIndex, "%x Issuing Takeout Target Order - Target Type %i", target->shiptype, team));

    if (goodEnough)
    {
treatasgoodenough:;
        if (useTheseShips)
        {
            // let's put useTheseShips into the team from reserve
            sdword i;
            Ship *ship;
            sdword numShips = useTheseShips->numShips;

            for (i=0;i<numShips;i++)
            {
                ship = useTheseShips->ShipPtr[i];

                growSelectRemoveShip(&aiCurrentAIPlayer->newships,ship);
                aitAddShip(team,ship);
            }
        }
    }
    else
    {
        Ship *playerMothership = aiCurrentAIPlayer->player->PlayerMothership;
        ShipRace race;
        if (playerMothership == NULL)
        {
            aiplayerLog((aiIndex, "Warning: could not build ships to takeout target"));
            goto treatasgoodenough;
        }

        race = playerMothership->shiprace;
        if ((race == P3) || (race == Traders))
        {
            aiplayerLog((aiIndex, "Warning: Pirates3Traders could not build ships to takeout target"));
            goto treatasgoodenough;
        }

        // we have to build them
        if (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Mothership)
        {
            shipsToBuy = statsBestShipToBuyToKillShip(race,statShipConstraintsFightingShipsCB,target->staticinfo);
        }
        else
        {
            shipsToBuy = statsBestShipToBuyToKillShip(race,statShipConstraintsCarrierFightingShipsCB,target->staticinfo);
        }
        dbgAssert(shipsToBuy);
        numShipsToBuy = statsNumShipsNeededToKillTarget(shipsToBuy,target->staticinfo);

        if (numShipsToBuy == 0)
        {
            // we don't know what ships to buy, so just arbitrarily pick some.
            switch (race)
            {
                case R1:
                case R2:
                    shipsToBuy = GetShipStaticInfo(StandardFrigate,race);
                    break;

                case P1:
                    shipsToBuy = GetShipStaticInfo(P1MissileCorvette,race);
                    break;
                case P2:
                    shipsToBuy = GetShipStaticInfo(P2AdvanceSwarmer,race);
                    break;

                default:
                    dbgAssert(FALSE);
            }
            numShipsToBuy = 1;
            aiplayerLog((aiIndex,"Taking out unknown target %d.  Guessing on ship to use",target->shiptype));
        }

        aiplayerLog((aiIndex,"Taking out with %i of shiptype %i", numShipsToBuy, shipsToBuy->shiptype));
        aimCreateGetShips(team, shipsToBuy->shiptype, (sbyte)numShipsToBuy, 0, TRUE, FALSE);
    }

    if (useTheseShips != NULL)
    {
        memFree(useTheseShips);
    }

    aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//    aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
    aimCreateTempGuard(team, AIO_TOUT_TARG_TGUARD_FORMATION, AIO_TOUT_TARG_TGUARD_TACTICS, TRUE, FALSE);

    selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
    selectone->numShips = 1;
    selectone->ShipPtr[0] = target;
    attackmove = aimCreateAttack(team, selectone, BROAD_FORMATION, TRUE, FALSE);
    aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
    aieHandlerSetFuelLow(attackmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);
    aimCreateMoveDone(team, FALSE, FALSE);
}

void aioCreateFancyTakeoutTarget(struct AITeam *team,Ship *target)
{
    ShipStaticInfo *shipsToBuy;
    sdword numShipsToBuy;
    SelectCommand *selectone;
    bool goodEnough;
    AITeam *secondaryteam;
    SelectCommand *nearbydangerousships;
    AITeamMove *move;

    // check reserves first:
    SelectCommand *useTheseShips = statsBestShipsToUseToKillTarget(aiCurrentAIPlayer->newships.selection,target->staticinfo,&goodEnough);

    aiplayerLog((aiIndex, "%x Issuing Fancy Takeout Target Order - Target %i", target->shiptype, team));

    if (goodEnough)
    {
treatasgoodenoughfancy:;
        if (useTheseShips)
        {
            // let's put useTheseShips into the team from reserve
            sdword i;
            Ship *ship;
            sdword numShips = useTheseShips->numShips;

            for (i=0;i<numShips;i++)
            {
                ship = useTheseShips->ShipPtr[i];

                growSelectRemoveShip(&aiCurrentAIPlayer->newships,ship);
                aitAddShip(team,ship);
            }
        }
    }
    else
    {
        Ship *playerMothership = aiCurrentAIPlayer->player->PlayerMothership;
        ShipRace race;
        if (playerMothership == NULL)
        {
            aiplayerLog((aiIndex, "Warning: could not build ships to takeout target"));
            goto treatasgoodenoughfancy;
        }

        race = playerMothership->shiprace;
        if ((race == P3) || (race == Traders))
        {
            aiplayerLog((aiIndex, "Warning: Pirates3Traders could not build ships to takeout target"));
            goto treatasgoodenoughfancy;
        }

        // we have to build them
        if (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Mothership)
        {
            shipsToBuy = statsBestShipToBuyToKillShip(race,statShipConstraintsFightingShipsCB,target->staticinfo);
        }
        else
        {
            shipsToBuy = statsBestShipToBuyToKillShip(race,statShipConstraintsCarrierFightingShipsCB,target->staticinfo);
        }
        dbgAssert(shipsToBuy);
        numShipsToBuy = statsNumShipsNeededToKillTarget(shipsToBuy,target->staticinfo);

        if (numShipsToBuy == 0)
        {
            // we don't know what ships to buy, so just arbitrarily pick some.
            switch (race)
            {
                case R1:
                case R2:
                    shipsToBuy = GetShipStaticInfo(StandardFrigate,race);
                    break;

                case P1:
                    shipsToBuy = GetShipStaticInfo(P1MissileCorvette,race);
                    break;
                case P2:
                    shipsToBuy = GetShipStaticInfo(P2AdvanceSwarmer,race);
                    break;

                default:
                    dbgAssert(FALSE);
            }
            numShipsToBuy = 1;
            aiplayerLog((aiIndex,"Taking out unknown target %d.  Guessing on ship to use",target->shiptype));
        }

        aiplayerLog((aiIndex,"Taking out with %i of shiptype %i", numShipsToBuy, shipsToBuy->shiptype));
        aimCreateGetShips(team, shipsToBuy->shiptype, (sbyte)numShipsToBuy, 0, TRUE, FALSE);
    }

    if (useTheseShips != NULL)
    {
        memFree(useTheseShips);
    }

    // we now have primary team.  Do we need a secondary team?

    nearbydangerousships = aiuFindNearbyDangerousEnemyShips(target,1600.0f);

    if (nearbydangerousships->numShips == 0)
    {
nosecondaryteamnecessary:
        {
        AITeamMove *attackmove;

//        aimCreateFormation(team, BROAD_FORMATION, FALSE, FALSE);

        aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//        aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
        aimCreateTempGuard(team, AIO_TOUT_TARG_FANCY_TGRD_FORMATION, AIO_TOUT_TARG_FANCY_TGRD_TACTICS, TRUE, FALSE);

//        aimCreateFormation(team, BROAD_FORMATION, FALSE, FALSE);

        selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
        selectone->numShips = 1;
        selectone->ShipPtr[0] = target;
        attackmove = aimCreateAttack(team, selectone, BROAD_FORMATION, TRUE, FALSE);
        aieHandlerSetGettingRocked(attackmove, TRUE, aihGenericGettingRockedHandler);
        aieHandlerSetFuelLow(attackmove, 15, TRUE, TRUE, aihGenericFuelLowHandler);

        aimCreateMoveDone(team, FALSE, FALSE);
        }
    }
    else
    {
        AIVar *Var0;
        AIVar *Var1;
        sdword secondary_num_ships_to_buy;
        ShipStaticInfo *secondary_ships_to_buy;
        char label[AIVAR_LABEL_MAX_LENGTH+1];

        if (aiCurrentAIPlayer->player->PlayerMothership && (aiCurrentAIPlayer->player->PlayerMothership->shiptype == Mothership))
        {
            secondary_ships_to_buy = statsBestShipToBuyToKillFleet(aiCurrentAIPlayer->player->race,statShipConstraintsFightingShipsCB,nearbydangerousships);
        }
        else
        {
            secondary_ships_to_buy = statsBestShipToBuyToKillFleet(aiCurrentAIPlayer->player->race,statShipConstraintsCarrierFightingShipsCB,nearbydangerousships);
        }

        secondary_num_ships_to_buy = statsNumShipsNeededToKillFleet(secondary_ships_to_buy,nearbydangerousships);

        if (secondary_num_ships_to_buy == 0)
        {
            goto nosecondaryteamnecessary;
        }

        if (secondary_num_ships_to_buy > 20)
        {
            secondary_ships_to_buy = statsBestShipToBuyToKillFleet(aiCurrentAIPlayer->player->race,statShipConstraintsFrigatesOrBetterCB,
                                                                   nearbydangerousships);

            secondary_num_ships_to_buy = statsNumShipsNeededToKillFleet(secondary_ships_to_buy,nearbydangerousships);

            if (secondary_num_ships_to_buy == 0)
            {
                goto nosecondaryteamnecessary;
            }

            if (secondary_num_ships_to_buy > 20)
            {
                secondary_num_ships_to_buy = 20;
            }
        }

        secondaryteam = aitCreate(team->teamType);
        secondaryteam->cooperatingTeam = team;

        Var0 = aivarCreate(aivarLabelGenerate(label));
        aivarValueSet(Var0, 0);

        Var1 = aivarCreate(aivarLabelGenerate(label));
        aivarValueSet(Var1, 0);

        aimCreateGetShips(secondaryteam, secondary_ships_to_buy->shiptype, (sbyte)secondary_num_ships_to_buy, 0, TRUE, FALSE);

        aimCreateVarSet(secondaryteam, aivarLabelGet(Var1), TRUE, FALSE, FALSE);

        move = aimCreateVarWait(secondaryteam, aivarLabelGet(Var0), TRUE, TRUE, FALSE);
        aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

        aimCreateVarDestroy(secondaryteam, aivarLabelGet(Var0), FALSE, FALSE);

//        aimCreateFormation(secondaryteam, SPHERE_FORMATION, FALSE, FALSE);

        move = aimCreateGuardCooperatingTeam(secondaryteam, FALSE, FALSE);
        aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);

        aimCreateMoveDone(secondaryteam, FALSE, FALSE);


//        aimCreateFormation(team, BROAD_FORMATION, FALSE, FALSE);

        aimCreateVarSet(team, aivarLabelGet(Var0), TRUE, FALSE, FALSE);

        move = aimCreateVarWait(team, aivarLabelGet(Var1), TRUE, TRUE, FALSE);
        aieHandlerSetTeamDied(move, aihRemoveTeamDiedHandler);

        aimCreateVarDestroy(team, aivarLabelGet(Var1), FALSE, FALSE);

        aimCreateVarDec(team, aiCurrentAIPlayer->attackVarLabel, TRUE, FALSE);
//        aimCreateVarWait(team, aiCurrentAIPlayer->attackVarLabel, -1, TRUE, FALSE);
        aimCreateTempGuard(team, AIO_TOUT_TARG_FANCY_TGRD_FORMATION, AIO_TOUT_TARG_FANCY_TGRD_TACTICS, TRUE, FALSE);

//        aimCreateFormation(team, BROAD_FORMATION, FALSE, FALSE);

        selectone = memAlloc(sizeofSelectCommand(1),"takeoutsel",0);
        selectone->numShips = 1;
        selectone->ShipPtr[0] = target;
        move = aimCreateAttack(team, selectone, BROAD_FORMATION, TRUE, FALSE);
        aieHandlerSetFuelLow(move, 15, TRUE, TRUE, aihGenericFuelLowHandler);

        aimCreateMoveDone(team, FALSE, FALSE);
    }

    aiumemFree(nearbydangerousships);
}

