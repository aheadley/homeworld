#include <stdlib.h>
#include <string.h>
#include "AITeam.h"
#include "AIEvents.h"
#include "AIVar.h"
#include "AIMoves.h"
#include "Memory.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "CommandWrap.h"
#include "Randy.h"
#include "AIUtilities.h"
#include "SaveGame.h"

sdword aimProcessGuardShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;

    if (team->shipList.selection->numShips == 0)
    {
        return FALSE;
    }

    if (!thisMove->processing)
    {
        if ((selection->numShips > 0) && (thisMove->params.guardShips.ships->numShips > 0))
        {
            if (aiuWrapProtect(selection, thisMove->params.guardShips.ships))
            {
                thisMove->processing = TRUE;        // only consider move processed if was successful issuing it
            }
        }
        else
        {
            aiplayerLog((aiIndex,"Warning: no ships to guard"));
            thisMove->processing = TRUE;
        }
        return FALSE;
    }
    else
    {
        if ((selection->numShips == 0) || (thisMove->params.guardShips.ships->numShips == 0))
        {
            memFree(thisMove->params.guardShips.ships);
            thisMove->params.guardShips.ships = NULL;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void aimShipDiedGuardShips(AITeam *team,AITeamMove *move,Ship *ship)
{
    SelectCommand *ships = move->params.guardShips.ships;
    if (ships != NULL)
    {
        if (clRemoveShipFromSelection(ships,ship))
        {
            // note! one of the ships we were guarding died!
            ;
        }
    }
}

void aimCloseGuardShips(AITeam *team,AITeamMove *move)
{
    if (move->params.guardShips.ships != NULL)
    {
        memFree(move->params.guardShips.ships);
        move->params.guardShips.ships = NULL;
    }
}

AITeamMove *aimCreateGuardShips(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "guardshipsmove", 0);

    InitNewMove(newMove,MOVE_GUARDSHIPS,wait,remove,formation,Neutral,aimProcessGuardShips,aimShipDiedGuardShips,aimCloseGuardShips);

    newMove->params.guardShips.ships  = ships;

//    aiplayerLog((aiIndex,"Created GuardShips Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_GuardShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessGuardShips,aimShipDiedGuardShips,aimCloseGuardShips);
}

void aimSave_GuardShips(AITeamMove *move)
{
    if (move->params.guardShips.ships) SaveSelection((SpaceObjSelection *)move->params.guardShips.ships);
}

void aimLoad_GuardShips(AITeamMove *move)
{
    if (move->params.guardShips.ships) move->params.guardShips.ships = (SelectCommand *)LoadSelectionAndFix();
}

sdword aimProcessGetShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    sdword numShipsRequested, typeRequested,numShipsToBuild;
    AIVar *doneVar;
    char label[AIVAR_LABEL_MAX_LENGTH+1];

    if (!thisMove->processing)
    {
        typeRequested = thisMove->params.getShips.shipType;
        numShipsRequested = thisMove->params.getShips.numShips;
        doneVar = aivarCreate(aivarLabelGenerate(label));
        aivarValueSet(doneVar, 0);
        thisMove->params.getShips.doneVar = doneVar;

        numShipsToBuild = aiuUnitCapCanBuildShip(aiCurrentAIPlayer, typeRequested, numShipsRequested);

        //if the ships are already built
        if (aiuAlreadyHasShipType(typeRequested, numShipsRequested))
        {
            aifTeamRequestsShipsCB(typeRequested,
                                   numShipsRequested,
                                   team,
                                   aivarLabelGet(doneVar),
                                   thisMove->params.getShips.priority);
        }
        //if the shiptype can be built
        //and if the numships unit caps will let us build is more than the number requested (with a bit of leeway)
        else if (aiuCanBuildShipType(typeRequested,(team->teamType == ScriptTeam)) &&
                 (numShipsToBuild > (numShipsRequested*0.75)))
        {
            //build only the number of ships unit caps will let us build
            aifTeamRequestsShipsCB(typeRequested,
                                  numShipsToBuild,
                                  team,
                                  aivarLabelGet(doneVar),
                                  thisMove->params.getShips.priority);
        }
        else
        {
            aiplayerLog((aiIndex, "Get Ships - can't build shiptype %i", thisMove->params.getShips.shipType));
        }

        thisMove->processing = TRUE;
    }

    // normally in a move processing function, we'd check the
    // thisMove->wait flag, but...
    //
    // always wait for the ships to be built (we could loosen this a bit later, maybe)
    // but consider what happens when the rest of the ships are actually built if we
    // leave early
    return aivarValueGet(thisMove->params.getShips.doneVar);
}

void aimCloseGetShips(AITeam *team,AITeamMove *move)
{
    if (move->params.getShips.doneVar != NULL)
    {
        aivarDestroy(move->params.getShips.doneVar);
        move->params.getShips.doneVar = NULL;
    }
}

AITeamMove *aimCreateGetShipsNoAdd(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove)
{
//    TypeOfFormation formation = SAME_FORMATION;
    AlternativeShips alternatives;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "getshipsmove", 0);

    SetNumAlternatives(alternatives, 0);
    newMove = aimCreateFancyGetShipsNoAdd(team, shiptype, num_ships, &alternatives, priority, wait, remove);

/*    dbgAssert(num_ships > 0);

    InitNewMove(newMove,MOVE_GETSHIPS,wait,remove,formation,Neutral,aimProcessGetShips,NULL,aimCloseGetShips);

    newMove->params.getShips.shipType = shiptype;
    newMove->params.getShips.numShips = num_ships;
    newMove->params.getShips.priority = priority;
    newMove->params.getShips.doneVar = NULL;

    aiplayerLog((aiIndex,"Created GetShips Move"));
*/
    return newMove;
}

AITeamMove *aimCreateGetShips(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    dbgAssert(num_ships > 0);

    newMove = aimCreateGetShipsNoAdd(team, shiptype, num_ships, priority, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_GetShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessGetShips,NULL,aimCloseGetShips);
    move->params.getShips.doneVar = NumberToAIVar((sdword)move->params.getShips.doneVar);
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_GetShips(AITeamMove *move)
{
    move->params.getShips.doneVar = (AIVar*)AIVarToNumber(move->params.getShips.doneVar);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

sdword aimProcessVarWait(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varWait.varName);

    thisMove->processing = TRUE;
    if (var &&
        thisMove->params.varWait.value == aivarValueGet(var))
        return TRUE;

    // variable hasn't reached its target value, so pay attention to the wait flag
    return !(thisMove->wait);
}


AITeamMove *aimCreateVarWait(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varwaitmove", 0);

    InitNewMove(newMove,MOVE_VARWAIT,wait,remove,formation,Neutral,aimProcessVarWait,NULL,NULL);

    memStrncpy(newMove->params.varWait.varName, varName, AIVAR_LABEL_MAX_LENGTH);
/*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varWait.varName[strlen(varName)] = 0;
    else
        newMove->params.varWait.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
*/
    newMove->params.varWait.value     = value;

//    aiplayerLog((aiIndex,"Created VarWait Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarWait(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarWait,NULL,NULL);
}

sdword aimProcessVarDec(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varDec.varName);

    if (!thisMove->processing)
    {
        if (var)
            aivarValueSet(var, aivarValueGet(var)-1);
        thisMove->processing = TRUE;
    }

    // nothing to wait on
    return TRUE;
}


AITeamMove *aimCreateVarDec(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "vardecmove", 0);

    InitNewMove(newMove,MOVE_VARDEC,wait,remove,formation,Neutral,aimProcessVarDec,NULL,NULL);

    memStrncpy(newMove->params.varDec.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varDec.varName[strlen(varName)] = 0;
    else
        newMove->params.varDec.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarDec Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarDec(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarDec,NULL,NULL);
}

sdword aimProcessVarInc(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varInc.varName);

    if (!thisMove->processing)
    {
        if (var)
            aivarValueSet(var, aivarValueGet(var)+1);
        thisMove->processing = TRUE;
    }

    // nothing to wait on
    return TRUE;
}


AITeamMove *aimCreateVarInc(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varincmove", 0);

    InitNewMove(newMove,MOVE_VARINC,wait,remove,formation,Neutral,aimProcessVarInc,NULL,NULL);

    memStrncpy(newMove->params.varInc.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varInc.varName[strlen(varName)] = 0;
    else
        newMove->params.varInc.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarInc Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarInc(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarInc,NULL,NULL);
}

sdword aimProcessVarSet(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varSet.varName);

    if (!thisMove->processing)  // probably not necessary to check for this simple command
    {
        aivarValueSet(var, thisMove->params.varSet.value);
        thisMove->processing = TRUE;
    }

    return TRUE;
}


AITeamMove *aimCreateVarSet(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varsetmove", 0);

    InitNewMove(newMove,MOVE_VARSET,wait,remove,formation,Neutral,aimProcessVarSet,NULL,NULL);

    memStrncpy(newMove->params.varSet.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varSet.varName[strlen(varName)] = 0;
    else
        newMove->params.varSet.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
*/
    newMove->params.varSet.value = value;

//    aiplayerLog((aiIndex,"Created VarSet Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarSet(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarSet,NULL,NULL);
}

sdword aimProcessVarDestroy(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varDestroy.varName);

    if (!thisMove->processing)  // probably not necessary to check for this simple command
    {
        aivarDestroy(var);
        thisMove->processing = TRUE;
    }

    return TRUE;
}

AITeamMove *aimCreateVarDestroy(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varsetmove", 0);

    InitNewMove(newMove,MOVE_VARDESTROY,wait,remove,formation,Neutral,aimProcessVarDestroy,NULL,NULL);

    memStrncpy(newMove->params.varDestroy.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varDestroy.varName[strlen(varName)] = 0;
    else
        newMove->params.varDestroy.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarDestroy Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarDestroy(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarDestroy,NULL,NULL);
}

//
//  temporary contention relief
//
#include "AIMoves1.c.h"  // Falko
#include "AIMoves2.c.h"  // Gary

