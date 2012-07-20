#include "UnivUpdate.h"

sdword aimProcessGuardCooperatingTeam(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;
    SelectCommand *guardselection = NULL;

    dbgAssert(!thisMove->processing);

    if (team->shipList.selection->numShips == 0)
    {
        return FALSE;
    }

    if (team->cooperatingTeam != NULL)
    {
        guardselection = team->cooperatingTeam->shipList.selection;
    }

    if ((selection->numShips > 0) && (guardselection != NULL) && (guardselection->numShips > 0))
    {
        aiuWrapProtect(selection,guardselection);
    }
    else
    {
        aiplayerLog((aiIndex,"Warning: no ships to guard"));
    }
    thisMove->processing = TRUE;

    return TRUE;
}

AITeamMove *aimCreateGuardCooperatingTeamNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "guardcoopmove", 0);

    InitNewMove(newMove,MOVE_GUARDCOOPTEAM,wait,remove,formation,Neutral,aimProcessGuardCooperatingTeam,NULL,NULL);

//    aiplayerLog((aiIndex,"Created GuardCoop Move"));

    return newMove;
}

AITeamMove *aimCreateGuardCooperatingTeam(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = aimCreateGuardCooperatingTeamNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_GuardCoopTeam(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessGuardCooperatingTeam,NULL,NULL);
}

sdword aimProcessLaunch(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;
    Ship *ship;
    Ship *shipIAmInside;
    sdword i;

    if (selection->numShips == 0)
    {
        return FALSE;
    }

    ship = selection->ShipPtr[0];
    shipIAmInside = univFindShipIAmInside(ship);
    if (shipIAmInside != NULL)
    {
        SelectCommand *selectcopy = memAlloc(sizeofSelectCommand(selection->numShips),"selectcopy",0);
        selectcopy->numShips = 0;

        dbgAssert(!thisMove->processing);

        for (i=0;i<selection->numShips;i++)
        {
            if (univAmIInsideThisShip(selection->ShipPtr[i],shipIAmInside))
            {
                selectcopy->ShipPtr[selectcopy->numShips++] = selection->ShipPtr[i];
            }
        }
        dbgAssert(selectcopy->numShips > 0);

        // Falko, don't replace this with aiuWrap because many ships in selectcopy are hidden, but this is okay in this case
        clWrapLaunchMultipleShips(&universe.mainCommandLayer,selectcopy,shipIAmInside);
        memFree(selectcopy);

        thisMove->processing = TRUE;
        return TRUE;
    }
    else if ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) && (ShipIsWaitingForSoftLaunch(ship)))
    {
        // we're on the ASF, so let's do a "soft" launch
        for (i=0;i<selection->numShips;i++)
        {
            bitClear(selection->ShipPtr[i]->specialFlags,SPECIAL_STAY_TILL_EXPLICITLAUNCH);
        }

        thisMove->processing = TRUE;
        return TRUE;
    }

    return FALSE;
}

AITeamMove *aimCreateLaunchNoAdd(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "launchmove", 0);

    InitNewMove(newMove,MOVE_LAUNCH,wait,remove,SAME_FORMATION,Neutral,aimProcessLaunch,NULL,NULL);

//    aiplayerLog((aiIndex,"Created Launch Move"));

    return newMove;
}

AITeamMove *aimCreateLaunch(AITeam *team, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateLaunchNoAdd(team, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Launch(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessLaunch,NULL,NULL);
}

sdword aimProcessFormation(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;

    dbgAssert(!thisMove->processing);

    if (team->shipList.selection->numShips == 0)
    {
        return FALSE;
    }

    if (selection->numShips > 0)
    {
        if (!aiuWrapFormation(selection,thisMove->params.formation.formationtype))
        {
            return FALSE;
        }
    }
    else
    {
        aiplayerLog((aiIndex,"Warning: no ships to put in formation"));
    }
    thisMove->processing = TRUE;

    //
    //  future enhancement:
    //  it might be nice to pay attention to the thisMove->wait flag here,
    //  and not return TRUE until the ships were actually in their formation
    //
    return TRUE;
}

AITeamMove *aimCreateFormationNoAdd(AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "formationmove", 0);

    InitNewMove(newMove,MOVE_FORMATION,wait,remove,formation,Neutral,aimProcessFormation,NULL,NULL);

    newMove->params.formation.formationtype = formationtype;

//    aiplayerLog((aiIndex,"Created Formation Move"));

    return newMove;
}


AITeamMove *aimCreateFormation(AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateFormationNoAdd(team, formationtype, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_Formation(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessFormation,NULL,NULL);
}


sdword aimProcessMoveDone(AITeam *team)
{
    return FALSE;
}

AITeamMove *aimCreateMoveDone(AITeam *team, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "movedone", 0);

    InitNewMove(newMove,MOVE_DONE,wait,remove,formation,Neutral,aimProcessMoveDone,NULL,NULL);

//    aiplayerLog((aiIndex,"Created Move Done Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_MoveDone(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessMoveDone,NULL,NULL);
}

//generic bandbox attack move
sdword aimProcessAttack(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection/*, *targets*/;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Attack Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if ((selection->numShips > 0) && (thisMove->params.attack.ships->numShips > 0))
        {
            aiuWrapAttack(team->shipList.selection, thisMove->params.attack.ships);
//            targets = aiuWrapAttack(team, thisMove->params.attack.ships);
//            aiumemFree(targets);
            thisMove->processing = TRUE;
        }
        else
        {
            aiplayerLog((aiIndex,"Warning: no ships to attack"));
            thisMove->processing = TRUE;
        }

        return !thisMove->wait;
    }
    else
    {
        if ((selection->numShips == 0) || (thisMove->params.attack.ships->numShips == 0) ||
            (aiuShipsNoLongerAttacking(selection)))
        {
            // we're done
            memFree(thisMove->params.attack.ships);
            thisMove->params.attack.ships = NULL;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void aimShipDiedAttack(AITeam *team,AITeamMove *move,Ship *ship)
{
    SelectCommand *ships = move->params.attack.ships;
    if (ships != NULL)
    {
        if (clRemoveShipFromSelection(ships,ship))
        {
            // note! one of the ships we were attacking died!
            ;
        }
    }
}

void aimCloseAttack(AITeam *team,AITeamMove *move)
{
    if (move->params.attack.ships != NULL)
    {
        memFree(move->params.attack.ships);
        move->params.attack.ships = NULL;
    }
}

AITeamMove *aimCreateAttackNoAdd(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "attackmove", 0);

    InitNewMove(newMove,MOVE_ATTACK,wait,remove,formation,Neutral,aimProcessAttack,aimShipDiedAttack,aimCloseAttack);
    newMove->params.attack.ships      = targets;

//    aiplayerLog((aiIndex,"Created Attack Move"));

    return newMove;
}

AITeamMove *aimCreateAttack(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateAttackNoAdd(team, targets, formation, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;

}

void aimFix_Attack(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessAttack,aimShipDiedAttack,aimCloseAttack);
}

void aimSave_Attack(AITeamMove *move)
{
    if (move->params.attack.ships) SaveSelection((SpaceObjSelection *)move->params.attack.ships);
}

void aimLoad_Attack(AITeamMove *move)
{
    if (move->params.attack.ships) move->params.attack.ships = (SelectCommand *)LoadSelectionAndFix();
}


//special move
sdword aimProcessSpecial(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;

    if (team->shipList.selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Special Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if ((selection->numShips > 0) &&
            ((thisMove->params.attack.ships == NULL) ||
             (thisMove->params.attack.ships->numShips > 0)))
        {
            aiuWrapSpecial(team->shipList.selection, thisMove->params.attack.ships);
            thisMove->processing = TRUE;
        }
        else
        {
            aiplayerLog((aiIndex,"Warning: no ships to special stuff with"));
            thisMove->processing = TRUE;
        }

        return FALSE;
    }
    else
    {
        if ((selection->numShips == 0) ||
            (!aitTeamIsDoingSpecialOp(team)))
        {
            // we're done
            aiumemFree(thisMove->params.attack.ships);

            return TRUE;
        }
        else
        {
            return (!thisMove->wait);
        }
    }
}

void aimSpecialShipDied(AITeam *team,AITeamMove *move,Ship *ship)
{
    SelectCommand *ships = move->params.attack.ships;
    if (ships != NULL)
    {
        if (clRemoveShipFromSelection(ships,ship))
        {
            // note! one of the ships we were attacking died!
            ;
        }
    }
}

void aimSpecialClose(AITeam *team,AITeamMove *move)
{
    aiumemFree(move->params.attack.ships);
}

AITeamMove *aimCreateSpecialNoAdd(AITeam *team, SelectCommand *targets,TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "special", 0);

    InitNewMove(newMove,MOVE_SPECIAL,wait,remove,formation,tactics,aimProcessSpecial,aimSpecialShipDied,aimSpecialClose);
    newMove->params.attack.ships = targets;

    aiplayerLog((aiIndex,"Created Special Move"));

    return newMove;
}

AITeamMove *aimCreateSpecial(AITeam *team, SelectCommand *targets,TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateSpecialNoAdd(team, targets, formation, tactics, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;

}

void aimFix_Special(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessSpecial,aimSpecialShipDied,aimSpecialClose);
}

void aimSave_Special(AITeamMove *move)
{
    if (move->params.attack.ships) SaveSelection((SpaceObjSelection *)move->params.attack.ships);
}

void aimLoad_Special(AITeamMove *move)
{
    if (move->params.attack.ships) move->params.attack.ships = (SelectCommand *)LoadSelectionAndFix();
}


sdword aimProcessKamikaze(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;

    if (selection->numShips == 0)
    {
        aiplayerLog((aiIndex,"Kamikaze Move, Zero Sized Team"));
        return TRUE;
    }

    if (!thisMove->processing)
    {
        if ((selection->numShips > 0) && (thisMove->params.kamikaze.ships->numShips > 0))
        {
            if (aiuWrapSetTactics(selection,Aggressive))
            {
                if (aiuWrapAttack(selection, thisMove->params.kamikaze.ships))
                {
                    aiuWrapSetKamikaze(selection);
                    thisMove->processing = TRUE;
                }
            }
        }
        else
        {
            aiplayerLog((aiIndex,"Warning: no ships to kamikaze into"));
            thisMove->processing = TRUE;
        }

        return FALSE;
    }
    else
    {
        if ((selection->numShips == 0) || (thisMove->params.kamikaze.ships->numShips == 0) ||
            (aiuShipsNoLongerAttacking(selection)))
        {
            // we're done
            memFree(thisMove->params.kamikaze.ships);
            thisMove->params.kamikaze.ships = NULL;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void aimShipDiedKamikaze(AITeam *team,AITeamMove *move,Ship *ship)
{
    SelectCommand *ships = move->params.kamikaze.ships;
    if (ships != NULL)
    {
        if (clRemoveShipFromSelection(ships,ship))
        {
            // note! one of the ships we were kamikazeing died!
            ;
        }
    }
}

void aimCloseKamikaze(AITeam *team,AITeamMove *move)
{
    if (move->params.kamikaze.ships != NULL)
    {
        memFree(move->params.kamikaze.ships);
        move->params.kamikaze.ships = NULL;
    }
}

AITeamMove *aimCreateKamikazeNoAdd(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "kamikazemove", 0);

    InitNewMove(newMove,MOVE_KAMIKAZE,wait,remove,formation,Aggressive,aimProcessKamikaze,aimShipDiedKamikaze,aimCloseKamikaze);
    newMove->params.kamikaze.ships      = targets;

//    aiplayerLog((aiIndex,"Created Kamikaze Move"));

    return newMove;
}

AITeamMove *aimCreateKamikaze(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    newMove = aimCreateKamikazeNoAdd(team, targets, formation, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;

}

void aimFix_Kamikaze(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessKamikaze,aimShipDiedKamikaze,aimCloseKamikaze);
}

void aimSave_Kamikaze(AITeamMove *move)
{
    if (move->params.kamikaze.ships) SaveSelection((SpaceObjSelection *)move->params.kamikaze.ships);
}

void aimLoad_Kamikaze(AITeamMove *move)
{
    if (move->params.kamikaze.ships) move->params.kamikaze.ships = (SelectCommand *)LoadSelectionAndFix();
}


static void GetFromReservesShipsOfShipType(AITeam *team,ShipType shiptype,sdword equivvalue,sdword *numPointsLeftToGet)
{
    SelectCommand *selection;
    sdword i;
    Ship *ship;

    while (*numPointsLeftToGet > 0)
    {
        selection = aiCurrentAIPlayer->newships.selection;
        for (i=0;i<selection->numShips;i++)
        {
            ship = selection->ShipPtr[i];
            if (ship->staticinfo->shiptype == shiptype)
            {
                growSelectRemoveShipIndex(&aiCurrentAIPlayer->newships,i);
                aitAddShip(team,ship);
                *numPointsLeftToGet -= equivvalue;
                goto foundship;
            }
        }
        // no ships found, so return
        return;
foundship:;
    }
}

void aimCloseFancyGetShips(AITeam *team,AITeamMove *move)
{
    if (move->params.fancyGetShips.doneVar != NULL)
    {
        aivarDestroy(move->params.fancyGetShips.doneVar);
        move->params.fancyGetShips.doneVar = NULL;
    }
}

sdword aimProcessFancyGetShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *doneVar;
    char label[AIVAR_LABEL_MAX_LENGTH+1];
    sdword numShipsToBuild, numShipsToGet;

    if (!thisMove->processing)
    {
        ShipType shiptype = thisMove->params.fancyGetShips.shipType;
        sdword numPointsLeftToGet = thisMove->params.fancyGetShips.numShips * ALTERNATIVE_SHIP_BASE;
        AlternativeShips *alternativeShips = &thisMove->params.fancyGetShips.alternatives;
        sdword numAlternatives = alternativeShips->numNextPicks;
        sdword i;

        doneVar = aivarCreate(aivarLabelGenerate(label));
        aivarValueSet(doneVar, 0);
        thisMove->params.fancyGetShips.doneVar = doneVar;

        // try to get as many of the requested ships as possible:
        // if any types are unavailable due to research, add check here later
        GetFromReservesShipsOfShipType(team,shiptype,ALTERNATIVE_SHIP_BASE,&numPointsLeftToGet);

        // now try alternatives
        for (i=0;i<numAlternatives;i++)
        {
            if (numPointsLeftToGet <= 0)
            {
                break;
            }

            GetFromReservesShipsOfShipType(team,alternativeShips->shipTypeNextPicks[i],alternativeShips->shipNumEquivNextPicks[i],&numPointsLeftToGet);
        }

        if (numPointsLeftToGet > 0)
        {
            // we couldn't get enough from reserves, so we have to build the rest:
            if (alternativeShips->alternativeFlags & ALTERNATIVE_RANDOM)
            {
                // randomly pick the rest

                sdword j;
                udword randomnum = ranRandom(RAN_AIPlayer) % (numAlternatives+1);
                if (randomnum == 0)
                {
                    goto buildbaseshiporfirst;
                }

                randomnum--;
                dbgAssert(randomnum < numAlternatives);
                for (i=randomnum,j=0;;)
                {
                    if (aiuCanBuildShipType(alternativeShips->shipTypeNextPicks[i],(team->teamType == ScriptTeam)))
                    {
                        // found the ship we can build.  Let's build them.
                        numShipsToGet = numPointsLeftToGet / alternativeShips->shipNumEquivNextPicks[i];
                        if (numShipsToGet == 0)
                        {
                            numShipsToGet++;
                        }

                        //determine the number of ships that unit caps will let us build
                        numShipsToBuild = aiuUnitCapCanBuildShip(aiCurrentAIPlayer, alternativeShips->shipTypeNextPicks[i], numShipsToGet);

                        //if we can build all the ships
                        if (numShipsToBuild == numShipsToGet)
                        {
                            aifTeamRequestsShipsCB(alternativeShips->shipTypeNextPicks[i],numShipsToGet,team,aivarLabelGet(doneVar),
                                                   thisMove->params.fancyGetShips.priority);
                            break;
                        }
                        //if we can only build a few
                        else if (numShipsToBuild > numShipsToGet*0.25)
                        {
                            numPointsLeftToGet = (numShipsToGet - numShipsToBuild) * alternativeShips->shipNumEquivNextPicks[i];

                            dbgAssert(numPointsLeftToGet > 0);

                            aifTeamRequestsShipsCB(alternativeShips->shipTypeNextPicks[i],numShipsToBuild,team,aivarLabelGet(doneVar),
                                                   thisMove->params.fancyGetShips.priority);
                        }
                    }

                    j++;
                    if (j >= numAlternatives)
                    {
//                        aiplayerLog((aiIndex,"Warning - couldn't build any secondary ships"));
                        goto buildbaseshiporfirst;
                    }

                    i++;
                    if (i >= numAlternatives)
                    {
                        i = 0;
                    }
                }
            }
            else
            {
buildbaseshiporfirst:
                if (aiuCanBuildShipType(shiptype,(team->teamType == ScriptTeam)))
                {
                    numShipsToGet = numPointsLeftToGet / ALTERNATIVE_SHIP_BASE;
                    if (numShipsToGet == 0)
                    {
                        numShipsToGet++;
                    }

                    //determine the number of ships that unit caps will let us build
                    numShipsToBuild = aiuUnitCapCanBuildShip(aiCurrentAIPlayer, shiptype, numShipsToGet);

                    //if we can build all the ships
                    if (numShipsToBuild == numShipsToGet)
                    {
                        aifTeamRequestsShipsCB(shiptype,numShipsToGet,team,aivarLabelGet(doneVar),thisMove->params.fancyGetShips.priority);
                    }
                    //if we can only build some
                    else if (numShipsToBuild > numShipsToGet*0.75)
                    {
                        numPointsLeftToGet = (numShipsToBuild - numShipsToGet) * ALTERNATIVE_SHIP_BASE;
                        aifTeamRequestsShipsCB(shiptype,numShipsToBuild,team,aivarLabelGet(doneVar),thisMove->params.fancyGetShips.priority);
                    }
                    else
                    {
						aivarDestroy(doneVar);	
						thisMove->params.fancyGetShips.doneVar = NULL;
						
						//postfinal: found that the fancygetships move gets stuck here,
						//			 even though the team already has all the ships,
						//			 added if and while loop
						
						// if the current team *already* has the ships we need,
						// the move is done
						if (team->shipList.selection->numShips)
						{
							if ((team->shipList.selection->ShipPtr[0]->shiptype ==
								 thisMove->params.fancyGetShips.shipType) &&
								(team->shipList.selection->numShips >=
								 thisMove->params.fancyGetShips.numShips))
							{
								return TRUE;
							}
						
							// same as above, but goes through all the alternatives
							for (i=0;i<numAlternatives;i++)
							{
								if ((team->shipList.selection->ShipPtr[0]->shiptype ==
									 alternativeShips->shipTypeNextPicks[i]) &&
									(team->shipList.selection->numShips >=
									 (thisMove->params.fancyGetShips.numShips * 10)/alternativeShips->shipNumEquivNextPicks[i]))
								{
									return TRUE;
								}
							}
						}
                        return FALSE;
                    }
                }
                else
                {
                    for (i=0;i<numAlternatives;i++)
                    {
                        if (aiuCanBuildShipType(alternativeShips->shipTypeNextPicks[i],(team->teamType == ScriptTeam)))
                        {
                            numShipsToGet = numPointsLeftToGet / alternativeShips->shipNumEquivNextPicks[i];
                            if (numShipsToGet == 0)
                            {
                                numShipsToGet++;
                            }


                            //determine the number of ships that unit caps will let us build
                            numShipsToBuild = aiuUnitCapCanBuildShip(aiCurrentAIPlayer, alternativeShips->shipTypeNextPicks[i], numShipsToGet);

                            //if we can build all the ships
                            if (numShipsToBuild == numShipsToGet)
                            {
                                aifTeamRequestsShipsCB(alternativeShips->shipTypeNextPicks[i],numShipsToGet,team,aivarLabelGet(doneVar),
                                                       thisMove->params.fancyGetShips.priority);
                                goto havebuiltbaseshiporfirst;
                            }
                            //if we can only build some
                            else if (numShipsToBuild > numShipsToGet*0.75)
                            {
                                numPointsLeftToGet = (numShipsToBuild - numShipsToGet) * ALTERNATIVE_SHIP_BASE;
                                aifTeamRequestsShipsCB(alternativeShips->shipTypeNextPicks[i],numShipsToBuild,team,aivarLabelGet(doneVar),
                                                       thisMove->params.fancyGetShips.priority);
                            }

                        }
                    }

                    // we couldn't build base ship, or any alternatives,
                    // destroy the team and try again.
                    aiplayerLog((aiIndex,"Warning - couldn't build base or secondary ship %i, deleting team", shiptype));
					aivarDestroy(doneVar);	
					thisMove->params.fancyGetShips.doneVar = NULL;
                    bitSet(team->teamFlags, AIT_DestroyTeam);
                    return TRUE;
                }
havebuiltbaseshiporfirst:;
            }
        }
        else
        {
            aivarValueSet(doneVar, TRUE);
        }

        thisMove->processing = TRUE;
    }

    // normally in a move processing function, we'd check the
    // thisMove->wait flag, but...
    //
    // always wait for the ships to be built (we could loosen this a bit later, maybe)
    // but consider what happens when the rest of the ships are actually built if we
    // leave early
    //
    // Falko's note later: leaving early is too much trouble - formations become incomplete,
    //                     the ships built after the team executes a move sit around for
    //                     an undefined period of time and the current Numbers Low
    //                     event handler gets confused
    return aivarValueGet(thisMove->params.getShips.doneVar);
}

AITeamMove *aimCreateFancyGetShipsNoAdd(AITeam *team, ShipType shiptype, sbyte num_ships, AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "getshipsmove", 0);

    dbgAssert(num_ships > 0);
#ifndef HW_Release
    {
        sdword i;

        for (i=0;i<alternatives->numNextPicks;i++)
        {
            dbgAssert(alternatives->shipNumEquivNextPicks[i] > 0);
        }
    }
#endif

    InitNewMove(newMove,MOVE_FANCYGETSHIPS,wait,remove,formation,Neutral,aimProcessFancyGetShips,NULL,aimCloseFancyGetShips);

    newMove->params.fancyGetShips.shipType = shiptype;
    newMove->params.fancyGetShips.numShips = num_ships;
    newMove->params.fancyGetShips.priority = priority;
    newMove->params.fancyGetShips.alternatives = *alternatives;
    newMove->params.fancyGetShips.doneVar = NULL;

//    aiplayerLog((aiIndex,"Created Fancy Getships Move"));

    return newMove;
}

AITeamMove *aimCreateFancyGetShips(AITeam *team, ShipType shiptype, sbyte num_ships, AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    dbgAssert(num_ships > 0);

    newMove = aimCreateFancyGetShipsNoAdd(team, shiptype, num_ships, alternatives, priority, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimFix_FancyGetShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessFancyGetShips,NULL,aimCloseFancyGetShips);
    move->params.fancyGetShips.doneVar = NumberToAIVar((sdword)move->params.fancyGetShips.doneVar);
}

void aimPreFix_FancyGetShips(AITeamMove *move)
{
    move->params.fancyGetShips.doneVar = (AIVar *)AIVarToNumber(move->params.fancyGetShips.doneVar);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

