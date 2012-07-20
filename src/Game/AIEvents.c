#include "AIEvents.h"
#include "AITeam.h"
#include "Memory.h"
#include "AIPlayer.h"
#include "Blobs.h"

//
//  handle any events applicable for the
//  curMove of the given team
//
//  these handlers can do pretty much anything,
//  including changing curMove, etc.
//
void aieExecute(struct AITeam *team)
{
    AITeamMove *curMove = team->curMove;
    SelectCommand *ships = NULL;
    ShipPtr ship;

    if (!curMove || curMove->type == MOVE_DONE)
        return;

    // we're just going to allow one event to fire per cycle through here
    // for now

    // IMPORTANT: a handler can do pretty much anything it wants to,
    // so after calling a handler, don't assume that the previous
    // curMove is still the current one or even if it still exists

    // team died
    if (curMove->events.teamDied.handler &&
        aieCheckTeamDied(team))
    {
        aiplayerLog((aiIndex,"eventhandler: teamDied"));
        curMove->events.teamDied.handler(team);
    }
    // ship died
    else if (curMove->events.shipDied.handler &&
             (!curMove->events.shipDied.oneShot ||
              !curMove->events.shipDied.triggered) &&
             aieCheckShipDied(team, &ship))
    {
        aiplayerLog((aiIndex,"eventhandler: shipDied"));
        curMove->events.shipDied.triggered = TRUE;
        curMove->events.shipDied.handler(team, ship);
    }
    // gettingRocked
    else if (curMove->events.gettingRocked.handler &&
             (!curMove->events.gettingRocked.oneShot ||
              !curMove->events.gettingRocked.triggered) &&
             aieCheckGettingRocked(team, &ships)) // any of the team's ships are getting rocked
    {
        aiplayerLog((aiIndex,"eventhandler: gettingRocked"));
        curMove->events.gettingRocked.triggered = TRUE;
        curMove->events.gettingRocked.handler(team, ships);
    }
    // enemyNearby
    else if (curMove->events.enemyNearby.handler &&
             (!curMove->events.enemyNearby.oneShot ||
              !curMove->events.enemyNearby.triggered) &&
             aieCheckEnemyNearby(team, &ships))
    {
        aiplayerLog((aiIndex,"eventhandler: enemyNearby"));
        curMove->events.enemyNearby.triggered = TRUE;
        curMove->events.enemyNearby.handler(team, ships);
    }
    // enemy not nearby
    else if (curMove->events.enemyNotNearby.handler &&
             (!curMove->events.enemyNotNearby.oneShot ||
              !curMove->events.enemyNotNearby.triggered) &&
             aieCheckEnemyNotNearby(team))
    {
        aiplayerLog((aiIndex,"eventhandler: enemyNotNearby"));
        curMove->events.enemyNotNearby.triggered = TRUE;
        curMove->events.enemyNotNearby.handler(team);
    }
    // firing
    else if (curMove->events.firing.handler &&
             (!curMove->events.firing.oneShot ||
              !curMove->events.firing.triggered) &&
             aieCheckFiring(team))
    {
        aiplayerLog((aiIndex,"eventhandler: firing"));
        curMove->events.firing.triggered = TRUE;
        curMove->events.firing.handler(team);
    }
    // disengaging
    else if (curMove->events.disengage.handler &&
             curMove->events.firing.triggered &&     //<----- note that this event is only triggered
             (!curMove->events.disengage.oneShot ||       //if the "firing" event has already been triggered
              !curMove->events.disengage.triggered) &&
             aieCheckDisengage(team))
    {
        aiplayerLog((aiIndex,"eventhandler: disengaging"));
        curMove->events.disengage.triggered = TRUE;
        curMove->events.disengage.handler(team);
    }
    // health low
    else if (curMove->events.healthLow.handler &&
             (!curMove->events.healthLow.oneShot ||
              !curMove->events.healthLow.triggered) &&
             aieCheckHealthLow(team))
    {
        aiplayerLog((aiIndex,"eventhandler: healthLow"));
        curMove->events.healthLow.triggered = TRUE;
        curMove->events.healthLow.handler(team);
    }
    // health high
    else if (curMove->events.healthHigh.handler &&
             (!curMove->events.healthHigh.oneShot ||
              !curMove->events.healthHigh.triggered) &&
             aieCheckHealthHigh(team))
    {
        aiplayerLog((aiIndex,"eventhandler: healthHigh"));
        curMove->events.healthHigh.triggered = TRUE;
        curMove->events.healthHigh.handler(team);
    }
    // numbers low
    else if (curMove->events.numbersLow.handler &&
             (!curMove->events.numbersLow.oneShot ||
              !curMove->events.numbersLow.triggered) &&
             aieCheckNumbersLow(team))
    {
        aiplayerLog((aiIndex,"eventhandler: numbersLow"));
        curMove->events.numbersLow.triggered = TRUE;
        curMove->events.numbersLow.handler(team);
    }
    // numbers high
    else if (curMove->events.numbersHigh.handler &&
             (!curMove->events.numbersHigh.oneShot ||
              !curMove->events.numbersHigh.triggered) &&
             aieCheckNumbersHigh(team))
    {
        aiplayerLog((aiIndex,"eventhandler: numbersHigh"));
        curMove->events.numbersHigh.triggered = TRUE;
        curMove->events.numbersHigh.handler(team);
    }
    // fuel low
    else if (curMove->events.fuelLow.handler &&
             (!curMove->events.fuelLow.oneShot ||
              !curMove->events.fuelLow.triggered) &&
             aieCheckFuelLow(team))
    {
        aiplayerLog((aiIndex,"eventhandler: fuelLow"));
        curMove->events.fuelLow.triggered = TRUE;
        curMove->events.fuelLow.handler(team);
    }
    // fuel high
    else if (curMove->events.fuelHigh.handler &&
             (!curMove->events.fuelHigh.oneShot ||
              !curMove->events.fuelHigh.triggered) &&
             aieCheckFuelHigh(team))
    {
        aiplayerLog((aiIndex,"eventhandler: fuelHigh"));
        curMove->events.fuelHigh.triggered = TRUE;
        curMove->events.fuelHigh.handler(team);
    }
    //interrupt
    else if (curMove->events.interrupt.handler &&
             (!curMove->events.interrupt.oneShot ||
              !curMove->events.interrupt.triggered) &&
             aieCheckInterrupt(team))
    {
        if ((!curMove->events.interrupt.interval) ||
            ((curMove->events.interrupt.interval) && (!curMove->events.interrupt.intervalcnt)))
        {
//            aiplayerLog((aiIndex,"eventhandler: interrupt"));
            curMove->events.interrupt.triggered = TRUE;
            curMove->events.interrupt.handler(team, curMove->events.interrupt.intvar);
            curMove->events.interrupt.intervalcnt = curMove->events.interrupt.interval;
        }
        else
        {
            curMove->events.interrupt.intervalcnt--;
        }
    }

    if (ships)  // may have been allocated in some checks
        memFree(ships);
}

//
//  clear all event handlers
//  (ie, don't handle events)
//
void aieHandlersClear (AITeamMove *move)
{
    move->events.gettingRocked.handler  = NULL;
    move->events.enemyNearby.handler    = NULL;
    move->events.enemyNotNearby.handler = NULL;
    move->events.firing.handler         = NULL;
    move->events.disengage.handler      = NULL;
    move->events.healthLow.handler      = NULL;
    move->events.healthHigh.handler     = NULL;
    move->events.numbersLow.handler     = NULL;
    move->events.numbersHigh.handler    = NULL;
    move->events.fuelLow.handler        = NULL;
    move->events.fuelHigh.handler       = NULL;
    move->events.shipDied.handler       = NULL;
    move->events.teamDied.handler       = NULL;
    move->events.interrupt.handler      = NULL;
}

void aieHandlerSetGettingRocked (struct AITeamMove *move, bool8 oneShot, aieHandlerShips handler)
{
    move->events.gettingRocked.oneShot   = oneShot;
    move->events.gettingRocked.triggered = 0;
    move->events.gettingRocked.handler   = handler;
}

void aieHandlerSetEnemyNearby (AITeamMove *move, real32 watchRadius, bool8 oneShot, aieHandlerShips handler)
{
    move->events.enemyNearby.watchRadius = watchRadius;
    move->events.enemyNearby.oneShot     = oneShot;
    move->events.enemyNearby.triggered   = 0;
    move->events.enemyNearby.handler     = handler;
}

void aieHandlerSetEnemyNotNearby (AITeamMove *move, real32 watchRadius, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.enemyNotNearby.watchRadius = watchRadius;
    move->events.enemyNotNearby.oneShot     = oneShot;
    move->events.enemyNotNearby.triggered   = 0;
    move->events.enemyNotNearby.handler     = handler;
}

void aieHandlerSetFiring (AITeamMove *move, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.firing.oneShot   = oneShot;
    move->events.firing.triggered = 0;
    move->events.firing.handler   = handler;
}

void aieHandlerSetDisengage(AITeamMove *move, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.disengage.oneShot   = oneShot;
    move->events.disengage.triggered = 0;
    move->events.disengage.handler   = handler;
}

void aieHandlerSetHealthLow (AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.healthLow.watchPercentage = watchPercentage;
    move->events.healthLow.watchIndividual = watchIndividual;
    move->events.healthLow.oneShot = oneShot;
    move->events.healthLow.triggered = 0;
    move->events.healthLow.handler = handler;
}

void aieHandlerSetHealthHigh (AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.healthHigh.watchPercentage = watchPercentage;
    move->events.healthHigh.watchIndividual = watchIndividual;
    move->events.healthHigh.oneShot = oneShot;
    move->events.healthHigh.triggered = 0;
    move->events.healthHigh.handler = handler;
}

void aieHandlerSetNumbersLow (AITeamMove *move, sdword watchPercentage, sdword watchBaseCount, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.numbersLow.watchPercentage = watchPercentage;
    move->events.numbersLow.watchBaseCount = watchBaseCount;
    move->events.numbersLow.oneShot = oneShot;
    move->events.numbersLow.triggered = 0;
    move->events.numbersLow.handler = handler;
}

void aieHandlerSetNumbersHigh (AITeamMove *move, sdword watchPercentage, sdword watchBaseCount, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.numbersHigh.watchPercentage = watchPercentage;
    move->events.numbersHigh.watchBaseCount = watchBaseCount;
    move->events.numbersHigh.oneShot = oneShot;
    move->events.numbersHigh.triggered = 0;
    move->events.numbersHigh.handler = handler;
}

void aieHandlerSetFuelLow (AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.fuelLow.watchPercentage = watchPercentage;
    move->events.fuelLow.watchIndividual = watchIndividual;
    move->events.fuelLow.oneShot = oneShot;
    move->events.fuelLow.triggered = 0;
    move->events.fuelLow.handler = handler;
}

void aieHandlerSetFuelHigh (AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler)
{
    move->events.fuelHigh.watchPercentage = watchPercentage;
    move->events.fuelHigh.watchIndividual = watchIndividual;
    move->events.fuelHigh.oneShot         = oneShot;
    move->events.fuelHigh.triggered       = 0;
    move->events.fuelHigh.handler         = handler;
}

void aieHandlerSetShipDied (AITeamMove *move, bool8 oneShot, aieHandlerShip handler)
{
    move->events.shipDied.oneShot = oneShot;
    move->events.shipDied.triggered = 0;
    move->events.shipDied.handler = handler;
}

void aieHandlerSetTeamDied (AITeamMove *move, aieHandlerSimple handler)
{
    move->events.teamDied.handler = handler;
}

void aieHandlerSetInterrupt (AITeamMove *move, udword *intvar, udword interval, bool8 oneShot, aieHandlerInt handler)
{
    move->events.interrupt.intvar      = intvar;
    move->events.interrupt.interval    = interval;
    move->events.interrupt.intervalcnt = interval;
    move->events.interrupt.oneShot     = oneShot;
    move->events.interrupt.triggered   = 0;
    move->events.interrupt.handler     = handler;
}

// -------------------------------------------
//  "CHECK" functions for events
//

//
//  return true iff someone in the team is getting rocked.
//  NOTE: this will allocate memory for ships iff true.
//
sdword aieCheckGettingRocked(AITeam *team, SelectCommand **ships)
{
    sdword i;
    SelectCommand *selection = team->shipList.selection;

    for (i = 0; i < selection->numShips; ++i)
        if (selection->ShipPtr[i]->gettingrocked &&
            (selection->ShipPtr[i]->gettingrocked->playerowner != selection->ShipPtr[i]->playerowner))
        {
            *ships = memAlloc(sizeofSelectCommand(1), "rockedby", 0);
            (*ships)->numShips = 1;
            (*ships)->ShipPtr[0] = selection->ShipPtr[i]->gettingrocked;
            // an enhancement would keep checking subsequent ships
            // in the team, adding their rockers to this selection
            return TRUE;
        }
    return FALSE;
}

//
//  return true iff there are enemy ships within a certain radius of any of
//  the given team's ships.
//  NOTE: this will allocate memory for ships iff true.
//
sdword aieCheckEnemyNearby(AITeam *team, SelectCommand **ships)
{
//    blob *myblob;

    if (team->shipList.selection->numShips)
    {
        // must use less efficient, but guaranteed, enemy nearby
        *ships = aiuFindNearbyEnemyShips(team->shipList.selection->ShipPtr[0],team->curMove->events.enemyNearby.watchRadius);

        // for efficiency, we assume all ships in the team and all
        // potential enemies are in the same blob -- an enhancement
        // later could be to select the most "central" ship instead
        // of the first one
            // NOTE: this method didn't work because blobs were too small.
//        myblob = aiuWrapGetCollBlob(team->shipList.selection);
//        if (!myblob) goto end;
//
//        *ships = selectDupSelection(myblob->blobShips);
//        aiuMakeShipsOnlyPrimaryEnemyShips((*ships));
//        MakeTargetsOnlyBeWithinRangeAndNotIncludeMe((SelectAnyCommand *)(*ships), (SpaceObjRotImpTarg *)(*ships)->ShipPtr[0], team->curMove->events.enemyNearby.watchRadius);
        if ((*ships)->numShips)
        {
            if (((*ships)->numShips <= 2) &&
                ((*ships)->ShipPtr[0]->shiptype == LightInterceptor))
            {
                goto end;
            }
            return TRUE;
        }
    }

  end:
    aiumemFree(*ships);
    return FALSE;
}

//
//  return true iff there are no enemy ships within a certain radius of any of
//  the given team's ships.
//
sdword aieCheckEnemyNotNearby(AITeam *team)
{
//    blob *myblob;
    SelectCommand *ships;

    if (team->shipList.selection->numShips)
    {
        // must use less efficient, but guaranteed, enemy nearby
        ships = aiuFindNearbyEnemyShips(team->shipList.selection->ShipPtr[0],team->curMove->events.enemyNearby.watchRadius);

        // for efficiency, we assume all ships in the team and all
        // potential enemies are in the same blob -- an enhancement
        // later could be to select the most "central" ship instead
        // of the first one
//        myblob = aiuWrapGetCollBlob(team->shipList.selection);
//        if (!myblob) goto end;
//
//        ships.numShips = 0;
//        selSelectionCopy((MaxAnySelection *)&ships, (MaxAnySelection *)myblob->blobShips);
//        MakeTargetsOnlyEnemyShips((SelectAnyCommand *)&ships, aiCurrentAIPlayer->player);
//        MakeTargetsOnlyBeWithinRangeAndNotIncludeMe((SelectAnyCommand *)&ships, (SpaceObjRotImpTarg *)team->shipList.selection->ShipPtr[0], team->curMove->events.enemyNotNearby.watchRadius);
        if (ships->numShips)
        {
            aiumemFree(ships);
            return FALSE;
        }
    }

//  end:
    aiumemFree(ships);
    return TRUE;
}

//
// return true iff one of the ships is firing (indicated by the shipisattacking
// boolean variable
//
sdword aieCheckFiring(AITeam *team)
{
    sdword i;
    SelectCommand *selection = team->shipList.selection;

    for (i=0;i < selection->numShips; ++i)
    {
        if (selection->ShipPtr[i]->shipisattacking)
        {
            return TRUE;
        }
    }
    return FALSE;
}


//
// returns true iff all the ships stop firing (indicated by the shipisattacking
// boolean variable
//
sdword aieCheckDisengage(AITeam *team)
{
    sdword i;
    SelectCommand *selection = team->shipList.selection;

    for (i=0;i < selection->numShips; ++i)
    {
        if (selection->ShipPtr[i]->shipisattacking)
        {
            return FALSE;
        }
    }
    return TRUE;
}

//
//  return true iff the entire group (or one individual, if specified)
//  is below a certain level of health
//
sdword aieCheckHealthLow(AITeam *team)
{
    sdword i;
    real32 maxHealth = 0.0;
    real32 actualHealth = 0.0;
    bool watchIndividual = team->curMove->events.healthLow.watchIndividual;

    for (i = 0; i < team->shipList.selection->numShips; ++i)
    {
        if (watchIndividual)
        {
            if ((100 * team->shipList.selection->ShipPtr[i]->health)
                < (team->curMove->events.healthLow.watchPercentage *
                   team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth))
                return TRUE;
        }
        else // accummulate
        {
            maxHealth += team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth;
            actualHealth += team->shipList.selection->ShipPtr[i]->health;
        }
    }
    if (!watchIndividual)
    {
        if ((100 * actualHealth) < (team->curMove->events.healthLow.watchPercentage * maxHealth))
            return TRUE;
    }
    return FALSE;
}

//
//  return true iff the entire group (or one individual, if specified)
//  is above a certain level of health
//
sdword aieCheckHealthHigh(AITeam *team)
{
    sdword i;
    real32 maxHealth = 0;
    real32 actualHealth = 0;
    bool watchIndividual = team->curMove->events.healthHigh.watchIndividual;

    for (i = 0; i < team->shipList.selection->numShips; ++i)
    {
        if (watchIndividual)
        {
            if ((100 * team->shipList.selection->ShipPtr[i]->health) >
                (team->curMove->events.healthHigh.watchPercentage *
                 team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth))
                return TRUE;
        }
        else // accummulate
        {
            maxHealth += team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth;
            actualHealth += team->shipList.selection->ShipPtr[i]->health;
        }
    }
    if (!watchIndividual)
    {
        if ((100 * actualHealth) > (team->curMove->events.healthHigh.watchPercentage * maxHealth))
            return TRUE;
    }
    return FALSE;
}

//
//  return true iff the size of the team is below a certain number of ships
//  (expressed as a % of original team size)
//
sdword aieCheckNumbersLow(AITeam *team)
{
    if (!team->curMove->events.numbersLow.watchBaseCount)
    {
        team->curMove->events.numbersLow.watchBaseCount = team->shipList.selection->numShips;
        return FALSE;
    }
    return ((100 * team->shipList.selection->numShips)
             < (team->curMove->events.numbersLow.watchPercentage *
            team->curMove->events.numbersLow.watchBaseCount));
}

//
//  return true iff the size of the team is above a certain number of ships
//  (expressed as a % of original team size)
//
sdword aieCheckNumbersHigh(AITeam *team)
{
    if (!team->curMove->events.numbersHigh.watchBaseCount)
    {
        team->curMove->events.numbersHigh.watchBaseCount = team->shipList.selection->numShips;
        return FALSE;
    }
    return ((100 * team->shipList.selection->numShips)
             > (team->curMove->events.numbersHigh.watchPercentage *
                team->curMove->events.numbersHigh.watchBaseCount));
}

//
//  return true iff the entire group (or one individual, if specified)
//  is below a certain level of fuel
//
sdword aieCheckFuelLow(AITeam *team)
{
    sdword i;
    real32 maxFuel = 0.0;
    real32 actualFuel = 0.0;
    bool watchIndividual = team->curMove->events.fuelLow.watchIndividual;

    for (i = 0; i < team->shipList.selection->numShips; ++i)
    {
        if (watchIndividual)
        {
            if ((100.0 * team->shipList.selection->ShipPtr[i]->fuel) <
                (team->curMove->events.fuelLow.watchPercentage *
                 team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel))
                return TRUE;
        }
        else // accummulate
        {
            maxFuel += team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel;
            actualFuel += team->shipList.selection->ShipPtr[i]->fuel;
        }
    }
    if (!watchIndividual)
    {
        if ((100.0 * actualFuel) < (team->curMove->events.fuelLow.watchPercentage * maxFuel))
            return TRUE;
    }
    return FALSE;
}

//
//  return true iff the entire group (or one individual, if specified)
//  is below a certain level of fuel
//
sdword aieCheckFuelHigh(AITeam *team)
{
    sdword i;
    real32 maxFuel = 0.0;
    real32 actualFuel = 0.0;
    bool watchIndividual = team->curMove->events.fuelHigh.watchIndividual;

    for (i = 0; i < team->shipList.selection->numShips; ++i)
    {
        if (watchIndividual)
        {
            if ((100.0 * team->shipList.selection->ShipPtr[i]->fuel) >
                (team->curMove->events.fuelHigh.watchPercentage *
                 team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel))
                return TRUE;
        }
        else // accummulate
        {
            maxFuel += team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel;
            actualFuel += team->shipList.selection->ShipPtr[i]->fuel;
        }
    }
    if (!watchIndividual)
    {
        if ((100.0 * actualFuel) > (team->curMove->events.fuelHigh.watchPercentage * maxFuel))
            return TRUE;
    }
    return FALSE;
}

sdword aieCheckShipDied(AITeam *team, ShipPtr *ship)
{
    return FALSE;
}

sdword aieCheckTeamDied(AITeam *team)
{
    if (team->shipList.selection->numShips)
    {
        return FALSE;
    }
    return TRUE;
}

sdword aieCheckInterrupt(AITeam *team)
{
    if (*(team->curMove->events.interrupt.intvar))
    {
        return TRUE;
    }
    return FALSE;
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aiePreFixAIEvents(struct AITeamMove *move)
{
    if (move->events.interrupt.handler)
    {
        // convert pointer to offset into AIPlayer structure
        dbgAssert(move->events.interrupt.intvar);
        move->events.interrupt.intvar = ((ubyte *)move->events.interrupt.intvar) - ((ubyte *)fixingThisAIPlayer);
        dbgAssert(move->events.interrupt.intvar < sizeof(AIPlayer));
    }

    move->events.gettingRocked.handler  = (aieHandlerShips)aieHandlerToNum((aieHandlerSimple)move->events.gettingRocked.handler);
    move->events.enemyNearby.handler    = (aieHandlerShips)aieHandlerToNum((aieHandlerSimple)move->events.enemyNearby.handler);
    move->events.enemyNotNearby.handler = (aieHandlerSimple)aieHandlerToNum(move->events.enemyNotNearby.handler);
    move->events.firing.handler         = (aieHandlerSimple)aieHandlerToNum(move->events.firing.handler);
    move->events.disengage.handler      = (aieHandlerSimple)aieHandlerToNum(move->events.disengage.handler);
    move->events.healthLow.handler      = (aieHandlerSimple)aieHandlerToNum(move->events.healthLow.handler);
    move->events.healthHigh.handler     = (aieHandlerSimple)aieHandlerToNum(move->events.healthHigh.handler);
    move->events.numbersLow.handler     = (aieHandlerSimple)aieHandlerToNum(move->events.numbersLow.handler);
    move->events.numbersHigh.handler    = (aieHandlerSimple)aieHandlerToNum(move->events.numbersHigh.handler);
    move->events.fuelLow.handler        = (aieHandlerSimple)aieHandlerToNum(move->events.fuelLow.handler);
    move->events.fuelHigh.handler       = (aieHandlerSimple)aieHandlerToNum(move->events.fuelHigh.handler);
    move->events.shipDied.handler       = (aieHandlerShip)aieHandlerToNum((aieHandlerSimple)move->events.shipDied.handler);
    move->events.teamDied.handler       = (aieHandlerSimple)aieHandlerToNum(move->events.teamDied.handler);
    move->events.interrupt.handler      = (aieHandlerInt)aieHandlerToNum((aieHandlerSimple)move->events.interrupt.handler);
}

void aieFixAIEvents(struct AITeamMove *move)
{
    move->events.gettingRocked.handler  = (aieHandlerShips)aieNumToHandler((sdword)move->events.gettingRocked.handler);
    move->events.enemyNearby.handler    = (aieHandlerShips)aieNumToHandler((sdword)move->events.enemyNearby.handler);
    move->events.enemyNotNearby.handler = aieNumToHandler((sdword)move->events.enemyNotNearby.handler);
    move->events.firing.handler         = aieNumToHandler((sdword)move->events.firing.handler);
    move->events.disengage.handler      = aieNumToHandler((sdword)move->events.disengage.handler);
    move->events.healthLow.handler      = aieNumToHandler((sdword)move->events.healthLow.handler);
    move->events.healthHigh.handler     = aieNumToHandler((sdword)move->events.healthHigh.handler);
    move->events.numbersLow.handler     = aieNumToHandler((sdword)move->events.numbersLow.handler);
    move->events.numbersHigh.handler    = aieNumToHandler((sdword)move->events.numbersHigh.handler);
    move->events.fuelLow.handler        = aieNumToHandler((sdword)move->events.fuelLow.handler);
    move->events.fuelHigh.handler       = aieNumToHandler((sdword)move->events.fuelHigh.handler);
    move->events.shipDied.handler       = (aieHandlerShip)aieNumToHandler((sdword)move->events.shipDied.handler);
    move->events.teamDied.handler       = aieNumToHandler((sdword)move->events.teamDied.handler);
    move->events.interrupt.handler      = (aieHandlerInt)aieNumToHandler((sdword)move->events.interrupt.handler);

    if (move->events.interrupt.handler)
    {
        dbgAssert(move->events.interrupt.intvar < sizeof(AIPlayer));
        move->events.interrupt.intvar = (udword *)( ((ubyte *)fixingThisAIPlayer) + ((sdword)move->events.interrupt.intvar) );
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

