//
//  implementations of KAS functions
//

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "ShipSelect.h"
#include "AITeam.h"
#include "KASFunc.h"
#include "AIPlayer.h"
#include "AIUtilities.h"
#include "Vector.h"
#include "KAS.h"
#include "Volume.h"
#include "Timer.h"
#include "CommandWrap.h"
#include "AIVar.h"
#include "AIMoves.h"
#include "FormationDefs.h"
#include "Randy.h"
#include "SinglePlayer.h"
#include "font.h"
#include "render.h"
#include "prim2d.h"
#include "Ping.h"
#include "Objectives.h"
#include "UnivUpdate.h"
#include "Tutor.h"
#include "SoundEvent.h"
#include "SoundEventDefs.h"
#include "SpeechEvent.h"
#include "SalCapCorvette.h"
#include "HS.h"
#include "SaveGame.h"
#include "Collision.h"
#include "mainrgn.h"
#include "NIS.h"
#include "texreg.h"
#include "Sensors.h"
#include "Subtitle.h"
#include "BTG.h"
#include "AITrack.h"
#include "utility.h"
#include "FastMath.h"
#include "Animatic.h"
#include "TradeMgr.h"
#include "StringsOnly.h"
#include "TaskBar.h"
#include "DDDFrigate.h"
#include "ConsMgr.h"
#include "Universe.h"

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif

extern char SinglePlayerSavedGamesPath[];
extern sdword CurrentMissionSkillLevel;
//extern char *getWord(char *dest, char *source); // Found in researchgui.c

//
//  these are for the placeholder for a real fleet intelligence message window
//
extern fonthandle selGroupFont2;
#define POPUPTEXT_TITLE_COLOR      colRGB(255, 255, 255)
#define POPUPTEXT_COLOR            colRGB(200, 200, 0)
#define POPUPTEXT_BACKGROUND       colRGB(0, 0, 0)
#define POPUPTEXT_BORDER_COLOR     colRGB(100, 100, 100)
sdword popupTextNumLines = 0;
char popupTextLines[POPUPTEXT_MAX_LINES][POPUPTEXT_MAX_LINE_LENGTH+1];
real32 popupTextEndTime;
sdword popupTextWidth;

// this can help eliminate the need to pass the current team ptr to some functions
extern AITeam *CurrentTeamP;
extern AITeam *kasUnpausedTeam;

// run-time scoping for variables, timers, etc.
extern sdword CurrentMissionScope;  // what level we're currently executing at
extern char CurrentMissionScopeName[];

extern LabelledVector **LabelledVectors;
extern sdword LabelledVectorsUsed;

extern bool ccCameraTimeoutOverride;

nisstatic stat1 = {0, 50, 25,  5,  3, 240, 173, 0.5f, 0.5f, 0.9f, 0.1f, 0.2f, 0.1f, 0.6f, 0.2f, TRUE, FALSE, 0};

//current sensors manager zoom length
extern real32 smInitialDistance;
//stores the current sensors manager zoom length
sdword kasStoreSMInitialDistance = 0;

extern real32 SUBMESSAGE_SAFETY_TIMEOUT;
extern real32 subMessageReturnedFalseTime;

//for fading to black at the end of the game
extern real32 nisBlackFadeDest;
extern real32 nisBlackFadeRate;

void kasfMissionCompleted(void)
{
    if (tutorial == TUTORIAL_ONLY)
    {
        //put the student out of it's misery
        tutorialdone = TRUE;
    }
    else
    {
        clCommandMessage("MISSION COMPLETED");
        // play the end of Mission animatic
        // if you want to finish a level without hyperspacing, you put the stuff that happens here
        speechEventCleanup();
        singlePlayerMissionCompleteCB();
        animBinkPlay(singlePlayerGameInfo.currentMission, singlePlayerGameInfo.currentMission + 1);
    }
}

void kasfMissionFailed(void)
{
    //clCommandMessage("MISSION FAILED");
    singlePlayerMissionFailedCB();
}

void kasfFadeToWhite(void)
{
    soundEventStopSFX(3.0);
    nisBlackFadeDest = 1.0f;
    nisBlackFadeRate = 3.0f;
    nisTaskResume();
}

//
//  for scoping symbol names
//
static void kasfScopeName(char *scopedName, char *name)
{
    if (!name || strlen(name) < 2)
        scopedName[0] = 0;
    else if (toupper(name[0]) == 'G' && name[1] == '_')
        // user is requesting global scope explicitly
        strcpy(scopedName, name+2);
    else if (CurrentMissionScope != KAS_SCOPE_MISSION)
        // local scope
        sprintf(scopedName, "%s.%s", CurrentMissionScopeName, name);
    else
        // user is in global scope
        strcpy(scopedName, name);
}

void kasfVarCreate(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarCreate(scopedName);
}

void kasfVarCreateSet(char *name, sdword value)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarCreate(scopedName);
    aivarValueSet(aivarFind(scopedName), value);
}

void kasfVarValueSet(char *name, sdword value)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarValueSet(aivarFind(scopedName), value);
}

void kasfVarValueInc(char *name)
{
    AIVar *var;
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    var =  aivarFind(scopedName);
    aivarValueSet(var, aivarValueGet(var) + 1);
}

void kasfVarValueDec(char *name)
{
    AIVar *var;
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    var = aivarFind(scopedName);
    aivarValueSet(var, aivarValueGet(var) - 1);
}

sdword kasfVarValueGet(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return aivarValueGet(aivarFind(scopedName));
}

void kasfVarDestroy(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarDestroy(aivarFind(scopedName));
}

void kasfCommandMessage(char *message)
{
    clCommandMessage(message);
}

void kasfTimerCreate(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerCreate(scopedName);
}

void kasfTimerSet(char *name, sdword duration)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerSet(scopedName, duration);
}

void kasfTimerStart(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerStart(scopedName);
}

void kasfTimerCreateSetStart(char *name, sdword duration)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerCreateSetStart(scopedName, duration);
}

void kasfTimerStop(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerStart(scopedName);
}

sdword kasfTimerRemaining(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerRemaining(scopedName);
}

sdword kasfTimerExpired(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerExpired(scopedName);
}

sdword kasfTimerExpiredDestroy(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerExpiredDestroy(scopedName);
}

void kasfTimerDestroy(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerDestroy(scopedName);
}

sdword kasfMsgReceived(char *msg)
{
    // remove the need to pass a team ptr
    return aitMsgReceived(CurrentTeamP, msg);
}

void kasfAttackMothership(void)
{
    SelectCommand *target;
    target = (SelectCommand *)memAlloc(sizeofSelectCommand(1), "targetmoship", 0);
    // assumes human is player 0
    target->ShipPtr[0] = aiuFindEnemyMothership(&universe.players[1]);
    if (target->ShipPtr[0] != NULL)
    {
        target->numShips = 1;
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateAttack(CurrentTeamP, target, SAME_FORMATION, TRUE, TRUE);
    }
}

void kasfAttack(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
//*** Note changed to Advanced attack!
        aimCreateAdvancedAttack(CurrentTeamP, dupeTargets, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}

// gives a set of ships the attack command
// this allows individual ships from a team to attack a target
void kasfShipsAttack(GrowSelection *targets, GrowSelection *attackers)
{
    aiuWrapAttack(attackers->selection, targets->selection);
}

void kasfAttackSpecial(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateSpecial(CurrentTeamP, dupeTargets, SAME_FORMATION, CurrentTeamP->kasTactics, FALSE, TRUE);
    }
}

void kasfMoveAttack(GrowSelection *targets)
{
    //alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateMoveAttack(CurrentTeamP, dupeTargets, TRUE, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}



/*-----------------------------------------------------------------------------
    Name        : kasfBulgeAttack
    Description : Finds ships in the current team that are near the enemy ships,
                  and only uses those to attack the enemy.  This function is an
                  optimization for Mission 14 (though it could be used anywhere)
    Inputs      : targets      - the ships to attack,
                  bulgetargets - the ships that end up being attacked,
                  attackers    - the ships in the team that split off to attack
                  radius       - the maximum distance the team members have to be in
                                 order to attack
    Outputs     : Various ships in the current team get attack orders
    Return      : the number of ships chosen to take out
----------------------------------------------------------------------------*/
sdword kasfBulgeAttack(GrowSelection *targets, GrowSelection *bulgetargets,
                       GrowSelection *attackers, sdword radius)
{
    ShipPtr teamship;
    SelectCommand *teamships   = CurrentTeamP->shipList.selection,
                  *targetships = targets->selection,
                  onesel;
    MaxSelection tempsel;
    udword i, j, num_teamships = teamships->numShips,
                 num_targets   = targetships->numShips;
    real32 radiussq = (real32)radius*(real32)radius, distsq;

    // initialize variables
    bulgetargets->selection->numShips = 0;
    attackers->selection->numShips    = 0;
    tempsel.numShips = 0;
    onesel.numShips  = 1;

    //go through each individual team ship
    for (i=0; i < num_teamships;i++)
    {
        teamship = teamships->ShipPtr[i];

        // only use ships that aren't attacking already
        if (!teamship->command ||
            (teamship->command->ordertype.order != COMMAND_ATTACK))
        {
            // find target ships within the alloted radius
            for (j=0;j < num_targets;j++)
            {
                distsq = aiuFindDistanceSquared(targets->selection->ShipPtr[j]->posinfo.position,
                                                teamship->posinfo.position);

                if (distsq < radiussq)
                {
                    selSelectionAddSingleShip(&tempsel, targets->selection->ShipPtr[j]);
                }
            }

            // if any targets close enough were found, attack them with this ship
            if (tempsel.numShips)
            {
                onesel.ShipPtr[0] = teamship;
                aiuWrapAttack(&onesel, (SelectCommand *)&tempsel);

                // add the ship to the attackers grow selection
                growSelectAddShip(attackers, teamship);

                // add each new target to the bulgetargets grow selection
                for (j=0;j<tempsel.numShips;j++)
                {
                    growSelectAddShipNoDuplication(bulgetargets, tempsel.ShipPtr[j]);
                }
                tempsel.numShips = 0;
            }
        }
    }
    return bulgetargets->selection->numShips;
}



void kasfIntercept(GrowSelection *targets)
{
    //alloc memory that the move can later free itself
    if (targets->selection->numShips && targets->selection->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateIntercept(CurrentTeamP, targets->selection->ShipPtr[0], 2.0, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}



// for salvage/capture ships
void kasfTargetDrop(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    for (i=0; i<selection->numShips; i++)
        SalCapDropTarget(selection->ShipPtr[i]);
}

void kasfAttackFlank(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateFlankAttack(CurrentTeamP, dupeTargets, FALSE, FALSE, TRUE);
    }
}

void kasfAttackHarass(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateHarassAttack(CurrentTeamP, FALSE, TRUE);
}

//sets the swarmers to target mode and adds ships to the target list
void kasfSetSwarmerTargets(GrowSelection *targets)
{
    aiumemFree(aiCurrentAIPlayer->Targets);
    aiCurrentAIPlayer->Targets = selectDupSelection(targets->selection);
    bitSet(aiCurrentAIPlayer->AlertStatus, ALERT_SWARMER_TARGETS);
}

void kasfSwarmMoveTo(GrowSelection *targets)
{
    MaxSelection swarmers, Pod;
    udword i;
    real32 temp;
    vector destination;
    ShipPtr ship;

    swarmers.numShips = 0;
    Pod.numShips = 0;

    //split the teams into swarmers and pods
    for (i=0;i<CurrentTeamP->shipList.selection->numShips;i++)
    {
        ship = CurrentTeamP->shipList.selection->ShipPtr[i];
        if ((ship->shiptype == P2Swarmer) ||
            (ship->shiptype == P2AdvanceSwarmer))
        {
            selSelectionAddSingleShip(&swarmers, ship);
        }
        else
        {
            selSelectionAddSingleShip(&Pod, ship);
        }
    }
    destination = selCentrePointComputeGeneral((MaxSelection *)targets->selection, &temp);

    if (Pod.numShips && swarmers.numShips)
    {
        aiuWrapProtect((SelectCommand *)&swarmers, (SelectCommand *)&Pod);
        aiuWrapMove((SelectCommand *)&Pod, destination);
    }
    else if (Pod.numShips)
    {
        aiuWrapMove((SelectCommand *)&Pod, destination);
    }
    else if (swarmers.numShips)
    {
        aiuWrapMove((SelectCommand *)&swarmers, destination);
    }
}

void kasfFormationDelta(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, DELTA_FORMATION);
    CurrentTeamP->kasFormation = DELTA_FORMATION;
}
void kasfFormationBroad(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, BROAD_FORMATION);
    CurrentTeamP->kasFormation = BROAD_FORMATION;
}
void kasfFormationDelta3D(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, DELTA3D_FORMATION);
    CurrentTeamP->kasFormation = DELTA3D_FORMATION;
}
void kasfFormationClaw(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, CLAW_FORMATION);
    CurrentTeamP->kasFormation = CLAW_FORMATION;
}
void kasfFormationWall(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, WALL_FORMATION);
    CurrentTeamP->kasFormation = WALL_FORMATION;
}
void kasfFormationSphere(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, SPHERE_FORMATION);
    CurrentTeamP->kasFormation = SPHERE_FORMATION;
}
void kasfFormationCustom(GrowSelection *ships)
{
    aiuKasWrapFormation(ships->selection, CUSTOM_FORMATION);
}

sdword kasfRandom(sdword lowestNum, sdword highestNum)
{
    if (lowestNum > highestNum)
    {
        sdword temp = lowestNum;
        lowestNum = highestNum;
        highestNum = temp;
    }
    return lowestNum + (ranRandom(RAN_AIPlayer) % (highestNum - lowestNum + 1));
}

void kasfGuardMothership(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDefendMothership(CurrentTeamP, FALSE, TRUE);
}

void kasfGuardShips(GrowSelection *ships)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeShips = selectDupSelection(ships->selection);
    if (dupeShips->numShips && dupeShips->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateGuardShips(CurrentTeamP, dupeShips, FALSE, TRUE);
    }
}

void kasfPatrolPath(Path *path)
{
    Path *dupePath = aiuPathDupe(path);
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreatePatrolMove(CurrentTeamP, dupePath, 0, SAME_FORMATION, CurrentTeamP->kasTactics, FALSE, TRUE);
}

void kasfPatrolActive(void)
{
    // an intelligent patrol that chooses its own path
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateActivePatrol(CurrentTeamP, AIT_SLOW_PATROL, FALSE, TRUE);
}

void kasfLog(char *string)
{
    aiplayerLog((0,"KAS: %s", string));
}

void kasfLogInteger(char *string, sdword integer)
{
    char tempstr[256];
    char tempstr2[256];

    // ensure not too long and that there is an integer field
    if (strlen(string) > 200 || !strstr(string, "%d"))
        return;

    sprintf(tempstr, "KAS: %s", string);
    sprintf(tempstr2, tempstr, integer);
    aiplayerLog((0, tempstr2));
}

#if 0       // this function is obsolete now
sdword kasfTeamMemberHandle(sdword Index)
{
    sdword numShips;

    numShips = CurrentTeamP->shipList.selection->numShips;
    if(Index >= numShips)
        return 0;

    return (sdword)(CurrentTeamP->shipList.selection->ShipPtr[Index]);
}
#endif

sdword kasfShipsOrder(GrowSelection *ships)
{
    Ship *ship;

    if (ships->selection->numShips > 0)
    {
        ship = ships->selection->ShipPtr[0];
        if (ship->command)
        {
            return ship->command->ordertype.order;
        }
    }

    return 0;
}

sdword kasfShipsOrderAttributes(GrowSelection *ships)
{
    Ship *ship;

    if (ships->selection->numShips > 0)
    {
        ship = ships->selection->ShipPtr[0];
        if (ship->command)
        {
            return ship->command->ordertype.attributes;
        }
    }

    return 0;
}

//  returns percentage
sdword kasfTeamHealthAverage(void)
{
    sdword i, numShips;
    real32 maxHealth = 0.0, actualHealth = 0.0;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        maxHealth += CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxhealth;
        actualHealth += CurrentTeamP->shipList.selection->ShipPtr[i]->health;
    }
    return (sdword) (100 * actualHealth / maxHealth);
}

//  returns percentage
sdword kasfTeamHealthLowest(void)
{
    sdword i, numShips;
    sdword thisOne = 0, lowestOne = 110;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        thisOne = (sdword) (100 * CurrentTeamP->shipList.selection->ShipPtr[i]->health / CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxhealth);
        if (thisOne < lowestOne)
            lowestOne = thisOne;
    }
    return lowestOne;
}

//  returns percentage
sdword kasfTeamFuelAverage(void)
{
    sdword i, numShips;
    real32 maxFuel = 0.0, actualFuel = 0.0;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
            maxFuel += CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxfuel;
            actualFuel += CurrentTeamP->shipList.selection->ShipPtr[i]->fuel;
    }
    return (sdword) (100 * actualFuel / maxFuel);
}

//  returns percentage
sdword kasfTeamFuelLowest(void)
{
    sdword i, numShips;
    sdword thisOne = 0, lowestOne = 110;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        thisOne = (sdword) (100 * CurrentTeamP->shipList.selection->ShipPtr[i]->fuel / CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxfuel);
        if (thisOne < lowestOne)
            lowestOne = thisOne;
    }
    return lowestOne;
}

sdword kasfTeamCount(void)
{
    return CurrentTeamP->shipList.selection->numShips;
}

sdword kasfNewShipsAdded(void)
{
    sdword return_value = (sdword)CurrentTeamP->newships;

    CurrentTeamP->newships = 0;

    return return_value;
}

void kasfGrowSelectionClear(GrowSelection *ships)
{
    kasGrowSelectionClear(ships);
}

sdword kasfShipsCount(GrowSelection *ships)
{
    return ships->selection->numShips;
}

sdword kasfPointInside(Volume *volume, hvector *location)
{
    vector position;

    vecGrabVecFromHVec(position, *location);

    return volPointInside(volume, &position);
}

//
//  given a volume, find all ships inside it.  make this the selection.
//  return the number of ships in this selection.
//
sdword kasfFindShipsInside(Volume *volume, GrowSelection *ships)
{
    Ship *ship;
    Node *node;

    if (ships == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            if (volPointInside(volume, &(ship->posinfo.position)))
                // add it
                growSelectAddShip(ships, ship);
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  given a volume, find all enemy ships inside it.  make this the selection.
//  return the number of ships in this selection.
//
sdword kasfFindEnemyShipsInside(Volume *volume, GrowSelection *ships)
{
    Ship *ship;
    Node *node;

    if (ships == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            if (!ship->playerowner->playerIndex && (!bitTest(ship->flags,SOF_Crazy|SOF_Hide|SOF_Hyperspace|SOF_Disabled)) && volPointInside(volume, &(ship->posinfo.position)))
                // add it
                growSelectAddShip(ships, ship);
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  given a selection of ships, find all enemies within a radius
//  of those ships.  make this the new selection.  return the number
//  of ships in the new selection.
//
sdword kasfFindEnemiesNearby(GrowSelection *ships, sdword radius)
{
    Ship *medianShip, *ship;
    real32 rangeSquared = (real32)radius*(real32)radius;
    Node *node;
    sdword i;

    if (ships == NULL || ships->selection == NULL || !(ships->selection->numShips))
        return 0;

    //  we should find the median ship, but since we're dealing with a selection
    //  and we probably don't intend the radius to exclude any existing ships,
    //  we'll assume the radius is large enough to consume the whole group from
    //  any (enemy) ship within the group (and then some, since the radius is supposed
    //  to add to the group if anything).  this is heavy-duty cheating, but it
    //  saves mucho CPU time if this routine were to be in many regular watch
    //  sections of script code.
    i = 0;
    do
    {
        medianShip = ships->selection->ShipPtr[i];
        ++i;
    } while (medianShip->playerowner->playerIndex && i < ships->selection->numShips); // not enemy yet

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            // is it an enemy?  (simplifying assumption: "is it the human"?)
            if ((!ship->playerowner->playerIndex) && (!bitTest(ship->flags,SOF_Crazy|SOF_Hide|SOF_Hyperspace|SOF_Disabled)))
            {
                // within the radius?
                if (medianShip == ship || aiuFindDistanceSquared(medianShip->posinfo.position, ship->posinfo.position) <= rangeSquared)
                {
                    // add it
                    growSelectAddShip(ships, ship);
                }
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  like FindEnemiesNearby, only check the radius relative to the current team,
//  ignoring what might be in the initial ships list
//
sdword kasfFindEnemiesNearTeam(GrowSelection *ships, sdword radius)
{
    Ship *medianShip, *ship;
    real32 rangeSquared = (real32)radius*(real32)radius;
    Node *node;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    if (!CurrentTeamP->shipList.selection->numShips)
        return 0;

    //
    //  TBD: could try to select a better median ship among the team
    //
    medianShip = CurrentTeamP->shipList.selection->ShipPtr[0];

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            // is it an enemy?  (simplifying assumption: "is it the human"?)
            if (!ship->playerowner->playerIndex && (!bitTest(ship->flags,SOF_Crazy|SOF_Hide|SOF_Hyperspace|SOF_Disabled)))
            {
                // within the radius?
                if (medianShip == ship || aiuFindDistanceSquared(medianShip->posinfo.position, ship->posinfo.position) <= rangeSquared)
                {
                    // add it
                    growSelectAddShip(ships, ship);
                }
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  Find all the ships near a point
//
sdword kasfFindShipsNearPoint(GrowSelection *ships, hvector *location, sdword radius)
{
    Ship *ship;
    real32 rangeSquared = (real32)radius*(real32)radius;
    vector position;
    Node *node;

    if (ships == NULL || ships->selection == NULL || location == NULL)
        return 0;

    vecGrabVecFromHVec(position, *location);

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            // is it an ship of some kind?
            if (!bitTest(ship->flags,SOF_Crazy|SOF_Hide|SOF_Hyperspace|SOF_Disabled))
            {
                // within the radius?
                if (aiuFindDistanceSquared(position, ship->posinfo.position) <= rangeSquared)
                {
                    // add it
                    growSelectAddShip(ships, ship);
                }
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  combine FindShipsInside and FindEnemiesNearby because they commonly go together
//
sdword kasfFindEnemiesInside(Volume *volume, GrowSelection *ships, sdword neighborRadius)
{
    return kasfFindEnemyShipsInside(volume, ships);
    //    return kasfFindEnemiesNearby(ships, neighborRadius);
    //else
    //    return 0;
}

void kasfTeamSkillSet(sdword skillLevel)
{
    // Falko, please implement this!

    //  CurrentTeamP->skillLevel = skillLevel;
}

sdword kasfTeamSkillGet(void)
{
    // Falko, please implement this!

    //  return CurrentTeamP->skillLevel;

    return 0;
}

void kasfShipsAttributesBitSet(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        bitSet(selection->ShipPtr[i]->attributes,attributes);
    }
}

void kasfShipsAttributesBitClear(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        bitClear(selection->ShipPtr[i]->attributes,attributes);
    }
}

void kasfShipsAttributesSet(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        selection->ShipPtr[i]->attributes = attributes;
    }
}

void kasfTeamAttributesBitSet(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;

    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        bitSet(shipP->attributes,attributes);
    }
}

void kasfTeamAttributesBitClear(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;

    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        bitClear(shipP->attributes,attributes);
    }
}

void kasfTeamAttributesSet(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;

    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        shipP->attributes = attributes;
    }
}

void kasfTeamMakeCrazy(sdword makecrazy)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;

    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        if (makecrazy)
        {
            ApplyCareenRotationDirectly(shipP);
            bitSet(shipP->flags,SOF_Crazy);
        }
        else
        {
            bitClear(shipP->flags,SOF_Crazy);
        }
    }
}

void kasfDisablePlayerHyperspace(void)
{
    singlePlayerGameInfo.hyperspaceFails = TRUE;
}

void kasfHoldHyperspaceWindow(bool hold)
{
    spHoldHyperspaceWindow = hold;
}

void kasfTeamHyperspaceIn(hvector *destination)
{
    if (!CurrentTeamP)
        return;

    spHyperspaceSelectionIn(CurrentTeamP->shipList.selection,destination);
    spHyperspaceDelay = 0.0f;
}

void kasfTeamHyperspaceInNear(hvector *destination, sdword distance)
{
    vector origin = {0.0,0.0,0.0};
    vector dest, hypepoint;
    hvector finalHyperspacePoint;

    if (!CurrentTeamP)
        return;

    vecGrabVecFromHVec(dest, *destination);
    hypepoint = aiuGenerateRandomStandoffPoint(dest, (real32)distance, origin, RSP_NORMAL);
    vecMakeHVecFromVec(finalHyperspacePoint, hypepoint);

    spHyperspaceSelectionIn(CurrentTeamP->shipList.selection, &finalHyperspacePoint);
    spHyperspaceDelay = 0.0f;
}

void kasfHyperspaceDelay(sdword milliseconds)
{
    spHyperspaceDelay = ((real32)milliseconds) / 1000.0f;
}

void kasfTeamHyperspaceOut(void)
{
    MaxSelection filtered;

    if (!CurrentTeamP)
        return;

    filtered.numShips = 0;

    aiuFilterDisabledShips(CurrentTeamP->shipList.selection, &filtered);

    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&filtered);
    spHyperspaceSelectionOut((SelectCommand *)&filtered);
    spHyperspaceDelay = 0.0f;
}

void kasfTeamSetMaxVelocity(sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;
    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        shipP->singlePlayerSpeedLimiter = (real32)maxVelocity;
        bitSet(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

void kasfTeamClearMaxVelocity(void)
{
    sdword i, numShips;
    Ship *shipP;

    if (!CurrentTeamP)
        return;
    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = CurrentTeamP->shipList.selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsSetMaxVelocity
    Description : Same as TeamSetMaxVelocity, except that it works on arbitrary
                    ship selections.
    Inputs      : ships - ships to cap velocity of, be they friendly or enemy.
                  maxVelocity - velocity to cap them at.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsSetMaxVelocity(GrowSelection *ships, sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        shipP->singlePlayerSpeedLimiter = (real32)maxVelocity;
        bitSet(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsClearMaxVelocity
    Description : Similar to TeamClearMaxVelocity, except that it works on an
                    arbitrary selection of ships.
    Inputs      : ships - ships to cap velocity of, be they friendly or enemy.
                  maxVelocity - velocity to cap them at.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsClearMaxVelocity(GrowSelection *ships, sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsSetDamageFactor
    Description : Sets a modifier in the ship structure to
                    modify the ammount of damage a ship does.
    Inputs      : ships - ships to modify damage of.
                  damagePercent (0 to 100) - divided by 100 then multiplied by ships damage
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsSetDamageFactor(GrowSelection *ships, sdword damagePercent)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        shipP->damageModifier = ((real32)damagePercent)/100.0f;
        bitSet(shipP->specialFlags,SPECIAL_KasSpecialDamageModifier);
    }
}
/*-----------------------------------------------------------------------------
    Name        : kasfShipsClearDamageFactor
    Description : Sets a modifier in the ship structure to
                    modify the ammount of damage a ship does.
    Inputs      : ships - ships to modify damage of.
                  damagePercent (0 to 100) - divided by 100 then multiplied by ships damage
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsClearDamageFactor(GrowSelection *ships)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_KasSpecialDamageModifier);
    }
}


// this function is a bit of a misnomer: it actually sets the percentage of health
// therefore 0 "percentDamaged", actually means the health is at 0 and the ship is dead
void kasfTeamSetPercentDamaged(sdword percentDamaged)
{
    sdword i, numShips;

    if (!CurrentTeamP)
        return;
    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
        CurrentTeamP->shipList.selection->ShipPtr[i]->health =
            CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxhealth * percentDamaged / 100;
}

void kasfTeamSetPercentFueled(sdword percentFueled)
{
    sdword i, numShips;

    if (!CurrentTeamP)
        return;
    numShips = CurrentTeamP->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
        CurrentTeamP->shipList.selection->ShipPtr[i]->fuel =
            CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxfuel * percentFueled / 100;
}

void kasfMissionSkillSet(sdword skillLevel)
{
    CurrentMissionSkillLevel = skillLevel;
}

sdword kasfMissionSkillGet(void)
{
    return CurrentMissionSkillLevel;
}

void kasfRequestShips(char *shipType, sdword numShips)
{
    AITeam *teamp;
    ShipType st = StrToShipType(shipType);
//    if (st < 0)
//        st = LightInterceptor;

    // make a temp team and get them to reinforce the first team
    teamp = aitCreate(ScriptTeam);
    strcpy(teamp->kasLabel, "R");
    strncat(teamp->kasLabel, CurrentTeamP->kasLabel, KAS_TEAM_NAME_MAX_LENGTH-1);
    teamp->kasOrigShipsCount = 0;
    aimCreateGetShips(teamp, st, (sbyte)numShips, (sdword)REQUESTSHIPS_HIPRI, TRUE, TRUE);
    aimCreateReinforce(teamp, CurrentTeamP, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    aimCreateDeleteTeam(teamp);
}

void kasfRequestShipsOriginal(sdword percentOriginal)
{
    AITeam *teamp;
    sbyte numShips = (sbyte)((CurrentTeamP->kasOrigShipsCount * (real32)percentOriginal * 0.01f) + 0.5f);
    if (numShips <= 0)
    {
        numShips = 1;
    }

    // make a temp team and get them to reinforce the first team
    teamp = aitCreate(ScriptTeam);
    strcpy(teamp->kasLabel, "R");
    strncat(teamp->kasLabel, CurrentTeamP->kasLabel, KAS_TEAM_NAME_MAX_LENGTH);
    teamp->kasOrigShipsCount = 0;
    aimCreateGetShips(teamp, CurrentTeamP->kasOrigShipsType, numShips, (sdword)REQUESTSHIPS_HIPRI, TRUE, TRUE);
    aimCreateReinforce(teamp, CurrentTeamP, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    aimCreateDeleteTeam(teamp);
}

void kasfReinforceTeamWithShips(struct AITeam *teamtoreinforce,GrowSelection *shipstoadd)
{
    sdword i;
    sdword numShips = shipstoadd->selection->numShips;
    AITeamMove *reinforceMove = teamtoreinforce->curMove;

    for (i=0;i<numShips;i++)
    {
        growSelectAddShip(&teamtoreinforce->shipList, shipstoadd->selection->ShipPtr[i]);
    }

    if (reinforceMove)
    {
        reinforceMove->processing = FALSE;
    }
}

void kasfReinforce(AITeam *team)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateReinforce(CurrentTeamP, team, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
}

void kasfForceCombatStatus(GrowSelection *ships, sdword on)
{
    udword i;

    if (on)
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            bitSet(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_ForcedAttackStatus);
        }
    }
    else
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            bitClear(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_ForcedAttackStatus);
        }
    }
}

sdword kasfThisTeamIs(AITeam *team)
{
    return (team == CurrentTeamP);
}

sdword kasfTeamCountOriginal(void)
{
    return CurrentTeamP->kasOrigShipsCount;
}

void kasfTeamGiveToAI(void)
{
    Ship *ship;
    udword i;

    if (!CurrentTeamP)
        return;

    for (i=0;i<CurrentTeamP->shipList.selection->numShips;)
    {
        ship = CurrentTeamP->shipList.selection->ShipPtr[i];
        aitRemoveShip(CurrentTeamP, ship);
        growSelectAddShip(&aiCurrentAIPlayer->newships, ship);

        if (ship->shiptype == Carrier || ship->shiptype == Mothership)
        {
            aiCurrentAIPlayer->AICreator = ship;
        }
    }
}

void kasfDisableAIFeature(sdword feature, sdword type)
{
    AIPlayer *aiplayer = aiCurrentAIPlayer;

    switch (type)
    {
        case ResourceFeature:
            aiuDisableResourceFeature(feature);
            break;
        case DefenseFeature:
            aiuDisableDefenseFeature(feature);
            break;
        case AttackFeature:
            aiuDisableAttackFeature(feature);
            break;
        case TeamFeature:
            aiuDisableTeamFeature(CurrentTeamP,feature);
            break;
        default:
            aiplayerLog((0, "Unknown Feature type being enabled %i, %i", feature, type));
            break;
    }
}
void kasfEnableAIFeature(sdword feature, sdword type)
{
    AIPlayer *aiplayer = aiCurrentAIPlayer;

    switch (type)
    {
        case ResourceFeature:
            aiuEnableResourceFeature(feature);
            break;
        case DefenseFeature:
            aiuEnableDefenseFeature(feature);
            break;
        case AttackFeature:
            aiuEnableAttackFeature(feature);
            break;
        case TeamFeature:
            aiuEnableTeamFeature(CurrentTeamP,feature);
            break;
        default:
            aiplayerLog((0, "Unknown Feature type being enabled %i, %i", feature, type));
            break;
    }
}

void kasfDisableAllAIFeatures(void)
{
    aiCurrentAIPlayer->ResourceFeatures = 0;
    aiCurrentAIPlayer->DefenseFeatures = 0;
    aiCurrentAIPlayer->AttackFeatures = 0;
//    aiCurrentAIPlayer->TeamFeatures = 0;
}
void kasfEnableAllAIFeatures(void)
{
    aiCurrentAIPlayer->ResourceFeatures = 0xFFFFFFFF;
    aiCurrentAIPlayer->DefenseFeatures = 0xFFFFFFFF;
    aiCurrentAIPlayer->AttackFeatures = 0xFFFFFFFF;
//    aiCurrentAIPlayer->TeamFeatures = 0xFFFFFFFF;
}


#if 0
void kasfTeamGiveToPlayer(void)
{
    Ship *ship;

    if (!CurrentTeamP)
        return;

    while (CurrentTeamP->shipList.selection->numShips)
    {
        ship = CurrentTeamP->shipList.selection->ShipPtr[0];
        aitRemoveShip(CurrentTeamP, ship);        // remove ship from team - and give to no one so it is free
        bitSet(ship->flags,SOF_Selectable);         // make sure player can select it
    }
}
#endif

void SelectionSwitchPlayerOwner(SelectCommand *selection)
{
    sdword i;
    sdword numShips = selection->numShips;
    Ship *ship;

    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,selection);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_PROTECT);      // remove ships from being targeted when switching sides
        if (ship->playerowner == &universe.players[0])
        {
            // going from human's ships to kas ships

            // Remove ships that are destroyed from any current selection
            clRemoveShipFromSelection((SelectCommand *)&selSelected,ship);

            // Remove ships that are destroyed from any hot key groups
            univRemoveShipFromHotkeyGroup(ship,FALSE);

            ship->playerowner = &universe.players[1];
            ship->attributes |= ATTRIBUTES_Defector;
            bitClear(ship->flags,SOF_Selectable);

            if (ship->shiptype == DDDFrigate)
            {
                DDDFrigateSwitchSides(ship,1);
            }
        }
        else
        {
            // going from kas ships to human ships
            aiplayerShipDied(ship);
            ship->playerowner = &universe.players[0];
            bitClear(ship->attributes,ATTRIBUTES_Defector);
			bitClear(ship->flags,SOF_Disabled);
            bitSet(ship->flags,SOF_Selectable);

            if (ship->shiptype == DDDFrigate)
            {
                DDDFrigateSwitchSides(ship,0);
            }
        }
    }
}

void kasfTeamSwitchPlayerOwner(void)
{
    if (!CurrentTeamP)
        return;

    SelectionSwitchPlayerOwner(CurrentTeamP->shipList.selection);
}

void kasfShipsSwitchPlayerOwner(GrowSelection *ships)
{
    SelectionSwitchPlayerOwner(ships->selection);
}

void kasfMoveTo(hvector *destination)
{
    vector dest;
    vecGrabVecFromHVec(dest,*destination);
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateMoveTeam(CurrentTeamP, dest, SAME_FORMATION, FALSE, TRUE);
}

void kasfShipsMoveTo(GrowSelection *ships, hvector *destination)
{
    vector dest;
    vecGrabVecFromHVec(dest,*destination);
    aiuWrapMove(ships->selection, dest);
}

void kasfTacticsAggressive(void)
{
    CurrentTeamP->kasTactics = Aggressive;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Aggressive);
}

void kasfTacticsNeutral(void)
{
    CurrentTeamP->kasTactics = Neutral;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Neutral);
}

void kasfTacticsEvasive(void)
{
    CurrentTeamP->kasTactics = Evasive;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Evasive);
}

//
//  returns True if any ship in the team is a given distance from the given location
//
sdword kasfNearby(hvector *location, sdword distance)
{
    real32 rangeSquared = (real32)distance*(real32)distance;
    sdword i, numShips;
    vector loc;
    vecGrabVecFromHVec(loc,*location);

    numShips = CurrentTeamP->shipList.selection->numShips;
    for (i = 0; i < numShips; ++i)
    {
        if (aiuFindDistanceSquared(CurrentTeamP->shipList.selection->ShipPtr[i]->posinfo.position, loc) <= rangeSquared)
            return TRUE;
    }
    return FALSE;
}

//
// returns the distance between two points
//
sdword kasfFindDistance(hvector *location1, hvector *location2)
{
    vector loc1, loc2;
    real32 distsq;

    vecGrabVecFromHVec(loc1, *location1);
    vecGrabVecFromHVec(loc2, *location2);

    distsq = aiuFindDistanceSquared(loc1, loc2);

    return (sdword)fsqrt(distsq);
}


sdword kasfUnderAttackElsewhere(struct AITeam *otherTeam, GrowSelection *attackers)
{
    sdword i;
    SelectCommand *teamSelection = otherTeam->shipList.selection;
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    Ship *ship;
    vector leaderpos;

    if (attackers == NULL)
        return 0;

    //  wipe the selection clean
    attackers->selection->numShips = 0;

    if (teamSelection->numShips == 0)
        return 0;

    // if some of the team is hidden, try other members of the team
    for (i=0; i < teamSelection->numShips; i++)
    {
        if (!bitTest(teamSelection->ShipPtr[i]->flags, SOF_Hide))
        {
            leaderpos = teamSelection->ShipPtr[i]->posinfo.position;
            break;
        }
    }

    if (i== teamSelection->numShips)
    {
        // all the team members are hidden... no attacking happening here.
        return 0;
    }

    // lets walk the command layer and see who's attacking us:
    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if ((command->ordertype.order == COMMAND_ATTACK) ||
            (command->ordertype.attributes & (COMMAND_IS_PASSIVEATTACKING|COMMAND_IS_ATTACKINGANDMOVING)))
        {
            if (command->attack)
            {
                if (AnyOfTheseShipsAreInSelection(teamSelection,(SelectCommand *)command->attack))
                {
                    SelectCommand *attackedby = command->selection;
                    for (i = 0;i < attackedby->numShips; i++)
                    {
                        ship = attackedby->ShipPtr[i];
                        if (MoveReachedDestinationVariable(ship,&leaderpos,10000.0f))
                            growSelectAddShipNoDuplication(attackers,ship);
                    }
                }
            }
        }

        curnode = curnode->next;
    }

    return attackers->selection->numShips;
}

sdword kasfUnderAttack(GrowSelection *attackers)
{
    return kasfUnderAttackElsewhere(CurrentTeamP, attackers);
}

sdword kasfShipsCountType(GrowSelection *ships, char *shipType)
{
    sdword i, count = 0, numShips = ships->selection->numShips;
    ShipType st = StrToShipType(shipType);
//    if (st < 0)
//        return 0;
    for (i = 0; i < numShips; ++i)
        if (ships->selection->ShipPtr[i]->shiptype == st)
            ++count;
    return count;
}

void kasfDock(struct AITeam *withTeam)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, dockmoveFlags_Normal, NULL, FALSE, TRUE);
}

void kasfDockSupport()
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, dockmoveFlags_Normal, NULL, FALSE, TRUE);
}

void DockSupportWith(struct AITeam * withTeam,sdword flags)
{
    Ship *dockwith = NULL;
    if (withTeam->shipList.selection->numShips >= 1)
    {
        dockwith = withTeam->shipList.selection->ShipPtr[0];
        dbgAssert(dockwith);
        if (aiuShipIsntSelectable(dockwith))
        {
            dockwith = NULL;
        }
        else
        {
            flags |= dockmoveFlags_AtShip;
        }
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, flags, dockwith, FALSE, TRUE);
}

void kasfDockSupportWith(struct AITeam * withTeam)
{
    DockSupportWith(withTeam,0);
}

void kasfShipsDockSupportWith(GrowSelection *ships, GrowSelection *withShips)
{
    aiuWrapDock(ships->selection, DOCK_AT_SPECIFIC_SHIP, withShips->selection->ShipPtr[0]);
}

void kasfDockStay(struct AITeam * withTeam)
{
    DockSupportWith(withTeam,dockmoveFlags_Stay);
}

void kasfShipsDockStay(GrowSelection *ships, GrowSelection *withShips)
{
    aiuWrapDock(ships->selection, DOCK_AT_SPECIFIC_SHIP|DOCK_STAY_TILL_EXPLICITLAUNCH, withShips->selection->ShipPtr[0]);
}

void kasfDockStayMothership(void)
{
    Ship *dockwith;

    if (CurrentTeamP->shipList.selection->numShips >= 1)
    {
        dockwith = CurrentTeamP->shipList.selection->ShipPtr[0]->playerowner->PlayerMothership;
        if ((dockwith) && (!aiuShipIsntSelectable(dockwith)))
        {
            aitDeleteAllTeamMoves(CurrentTeamP);
            aimCreateDock(CurrentTeamP, dockmoveFlags_AtShip|dockmoveFlags_Stay, dockwith, FALSE, TRUE);
        }
    }
}

void kasfDockInstant(struct AITeam * withTeam)
{
    //put in print command
    udword i;

    for (i=0;i<CurrentTeamP->shipList.selection->numShips;i++)
    {
        aiplayerLog((0, "Docking Instant Ship Type %i", CurrentTeamP->shipList.selection->ShipPtr[i]->shiptype));
    }
    DockSupportWith(withTeam,dockmoveFlags_Stay|dockmoveFlags_Instantaneously);
}

void kasfLaunch()
{
    udword i;

    for (i=0;i<CurrentTeamP->shipList.selection->numShips;i++)
    {
        bitSet(CurrentTeamP->shipList.selection->ShipPtr[i]->specialFlags, SPECIAL_LaunchedFromKas|SPECIAL_KasCheckDoneLaunching);
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateLaunch(CurrentTeamP, FALSE, TRUE);
}

sdword kasfTeamDocking(void)
{
    //return bitTest(CurrentTeamP->teamFlags, TEAM_DOCKING);
    return ((CurrentTeamP->teamFlags & TEAM_DOCKING) != 0);  // ensure 0/1 return value
}

bool ShipReadyForLaunch(Ship *ship)
{
    if ((ship->flags & (SOF_Hide | SOF_Hyperspace)) == SOF_Hide)
    {
        if (ShipIsRefuelingAtCarrierMother(ship))
            return FALSE;       // wait, refueling, we're not done yet!
        else
            return TRUE;        // ship definitely ready for launch, it is inside another ship
    }
    else if ((ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH) && (ShipIsWaitingForSoftLaunch(ship)))
    {
        return TRUE;
    }
    return FALSE;
}

bool ShipFinishedLaunching(Ship *ship)
{
    if (ship->specialFlags & SPECIAL_KasCheckDoneLaunching)
    {
        return FALSE;
    }

    return TRUE;
}

sdword kasfTeamFinishedLaunching(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    if (selection->numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<selection->numShips;i++)
    {
        if (!ShipFinishedLaunching(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

sdword kasfTeamDockedReadyForLaunch(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    if (selection->numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<selection->numShips;i++)
    {
        if (!ShipReadyForLaunch(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//
//  send to all script teams
//
void kasfMsgSendAll(char *msg)
{
    AITeam *teamp;
    sdword i;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamp = aiCurrentAIPlayer->teams[i];
          //not really sure we need this...  CPUTeams deserve to get messages too.
//        if ((teamp->teamType == ScriptTeam) || (teamp->teamType == AnyTeam))
            aitMsgSend(CurrentTeamP, teamp, msg);
    }
}

void kasfMsgSend(struct AITeam *team, char *msg)
{
    aitMsgSend(CurrentTeamP, team, msg);
}

sdword kasfRUsEnemyCollected(void)
{
    return singlePlayerGameInfo.resourceUnitsCollected;
}

//  serves the "friendly" and "enemy" functions below
static sdword kasfFindPlayersShipsOfType(GrowSelection *ships, char *shipType, sdword playerIndex)
{
    Node *node;
    ShipType st = StrToShipType(shipType);
    Ship *ship;

//    if (st < 0)
//        return 0;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            //  1. is it an enemy?  (simplifying assumption: "is it the human"?)
            //  2. is the right type?
            if (ship->playerowner->playerIndex == playerIndex && ship->shiptype == st)
                // add it
                growSelectAddShip(ships, ship);
        }
        node = node->next;
    }

    return ships->selection->numShips;
}

//  (enemy == human)
sdword kasfFindEnemyShipsOfType(GrowSelection *ships, char *shipType)
{
    return kasfFindPlayersShipsOfType(ships, shipType, 0);
}

//  (friendly == not human)
sdword kasfFindFriendlyShipsOfType(GrowSelection *ships, char *shipType)
{
    return kasfFindPlayersShipsOfType(ships, shipType, 1);
}

//  serves the "friendly" and "enemy" functions below
static sdword kasfFindPlayersShipsOfClass(GrowSelection *ships, char *shipClass, sdword playerIndex)
{
    Node *node;
    ShipClass sc = StrToShipClass(shipClass);
    Ship *ship;

//    if (sc < 0)
//        return 0;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            //  1. is it an enemy?  (simplifying assumption: "is it the human"?)
            //  2. is the right class?
            if (ship->playerowner->playerIndex == playerIndex && ship->staticinfo->shipclass == sc)
                // add it
                growSelectAddShip(ships, ship);
        }
        node = node->next;
    }

    return ships->selection->numShips;
}

//  (enemy == human)
sdword kasfFindEnemyShipsOfClass(GrowSelection *ships, char *shipClass)
{
    return kasfFindPlayersShipsOfClass(ships, shipClass, 0);
}

//  (friendly == not human)
sdword kasfFindFriendlyShipsOfClass(GrowSelection *ships, char *shipClass)
{
    return kasfFindPlayersShipsOfClass(ships, shipClass, 1);
}

sdword kasfPathNextPoint(void)
{
    AITeamMove *thisMove = CurrentTeamP->curMove;

    if (CurrentTeamP->shipList.selection->numShips <= 0)
        return -1;

    if (!thisMove)
    {
        if (CurrentTeamP->moves.num > 0)
            thisMove = (AITeamMove *)listGetStructOfNode(CurrentTeamP->moves.tail);
        if (!thisMove)
            return -1;
    }

    if (thisMove->type == MOVE_PATROLMOVE)
    {
        if (thisMove->processing)
            // end of path or loop
            return 0;
        else
            return 1;  // first point
    }
    if (thisMove->type != MOVE_MOVETEAMINDEX)
        return -1;  // not patrolling a path

    return thisMove->params.move.index + 1;
}

//
//  PETER: I think we can nuke this now, right?
//

//
//  this is a placeholder for a real fleet intelligence message window
//
void kasfPopupTextDraw(void)
{
#if 1
    sdword rowHeight, x, y, i, popupTextBorder;
    rectangle rect;
    fonthandle fhSave;
    //char buf[256];

    if (!gameIsRunning || !popupTextNumLines)
        return;

    if (nisIsRunning)
        popupTextNumLines = 0;

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    // to center it:
    x = 320 - popupTextWidth/2;
    //x = 640 - popupTextWidth - rowHeight*2;

    rowHeight = fontHeight("M") + 3; // used to space the text
    // to center it:
    y = (480 - (popupTextNumLines+2) * rowHeight) / 2;
    //y = rowHeight*2;
    popupTextBorder = rowHeight + 6;

    // border
    //primLineLoopStart2(1, POPUPTEXT_COLOR);
    //primLineLoopPoint3F(x - popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopPoint3F(x - popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopEnd2();

    rect.x0 = x - popupTextBorder;
    rect.x1 = x + popupTextWidth + popupTextBorder;
    rect.y0 = y - popupTextBorder;
    rect.y1 = y + rowHeight * (popupTextNumLines+2) + popupTextBorder;
    primBeveledRectSolid(&rect, POPUPTEXT_BACKGROUND, 4, 4);
    primSeriesOfRoundRects(&rect, 1, POPUPTEXT_COLOR, POPUPTEXT_BORDER_COLOR, 5, 4, 4);

    // title
    fontPrint(x, y, POPUPTEXT_TITLE_COLOR, "FLEET INTELLIGENCE");
    y += rowHeight*2;

    // each line of text
    for (i = 0; i < popupTextNumLines; i++)
    {
        fontPrint(x, y, POPUPTEXT_COLOR, popupTextLines[i]);
        y += rowHeight;
    }

    // stop displaying when expired
    if (universe.totaltimeelapsed > popupTextEndTime)
        popupTextNumLines = 0;

    fontMakeCurrent(fhSave);
#endif
#if 0
       //sdword rowHeight, x, y, i, popupTextBorder;
    sdword i, width, x, y;
    bool done, justified;
    //rectangle rect;
    fonthandle fhSave;
    char *pos, *oldpos;
    char oldline[256], line[256];
    //char buf[256];

    if (!gameIsRunning || !popupTextNumLines)
        return;

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    //x = 320 - popupTextWidth/2;

    //rowHeight = fontHeight("M") + 3; // used to space the text
    //y = (480 - (popupTextNumLines+2) * rowHeight) / 2;
    //popupTextBorder = rowHeight + 6;

    // border
    //primLineLoopStart2(1, POPUPTEXT_COLOR);
    //primLineLoopPoint3F(x - popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopPoint3F(x - popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopEnd2();

    //rect.x0 = x - popupTextBorder;
    //rect.x1 = x + popupTextWidth + popupTextBorder;
    //rect.y0 = y - popupTextBorder;
    //rect.y1 = y + rowHeight * (popupTextNumLines+2) + popupTextBorder;
    //primBeveledRectSolid(&rect, POPUPTEXT_BACKGROUND, 4, 4);
    //primSeriesOfRoundRects(&rect, 1, POPUPTEXT_COLOR, POPUPTEXT_BORDER_COLOR, 5, 4, 4);

    // title
    //fontPrint(x, y, POPUPTEXT_TITLE_COLOR, "FLEET INTELLIGENCE");
    //y += rowHeight*2;

    x = rect->x0;
    y = rect->y0;

    // each line of text
    for (i = 0; i < popupTextNumLines; i++)
    {
        //fontPrint(x, y, POPUPTEXT_COLOR, popupTextLines[i]);
        pos = popupTextLines[i];

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' ) pos++;
                }
                else
                {
                    if ( (width=fontWidth(line)) > (rect->x1 - rect->x0))
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while ( pos[0] == ' ' ) pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,POPUPTEXT_COLOR,"%s",line);
            y += fontHeight(" ");
            if (y > rect->y1 + fontHeight(" ")) done=TRUE;
        }
        //y += rowHeight;
    }

    // stop displaying when expired
    //if (universe.totaltimeelapsed > popupTextEndTime)
    //    popupTextNumLines = 0;

    fontMakeCurrent(fhSave);
#endif
}


//
//  PETER: I think we can nuke this now, right?
//
void kasfPopupText(char *line)
{
    sdword width;
    fonthandle fhSave;

    if (popupTextNumLines >= POPUPTEXT_MAX_LINES)
    {
        strcpy(popupTextLines[POPUPTEXT_MAX_LINES-1], " - - message truncated - -");
        return;
    }

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    strncpy(popupTextLines[popupTextNumLines++], line, POPUPTEXT_MAX_LINE_LENGTH);
    width = fontWidth(popupTextLines[popupTextNumLines-1]);

    // set duration relative to number of lines
    if (popupTextNumLines == 1)
    {
        popupTextEndTime = universe.totaltimeelapsed;
        popupTextWidth = POPUPTEXT_BOX_WIDTH;
    }

    if (width > popupTextWidth)
        popupTextWidth = width;

    popupTextEndTime += POPUPTEXT_DURATION_PER_LINE;


    fontMakeCurrent(fhSave);
}

//
//  PETER: I think we can nuke this now, right?
//
void kasfPopupTextClear(void)
{
    popupTextNumLines = 0;
}

//
//  new fleet-intelligence style popup
//
void kasfPopup(char *text)
{
    fleetIntelligenceCreate(text, 1);
}

void kasfPopupInteger(char *text, sdword integer)
{
    char tempstr[256];

    // ensure not too long and that there is an integer field
    if (strlen(text) > 200 || !strstr(text, "%d"))
        return;

    sprintf(tempstr, text, integer);
    fleetIntelligenceCreate(tempstr, 1);
}

//
//  new fleet-intelligence style objectives & popup
//
void kasfObjectiveCreate(char *label, char *briefText, char *fullText)
{
    if ((fullText == NULL) || (fullText[0] == 0))
    {
        objectiveAndFleetIntelligenceCreate(label, briefText, briefText, 0, TRUE);
    }
    else
    {
        objectiveAndFleetIntelligenceCreate(label, briefText, fullText, 0, TRUE);
    }
}

void kasfObjectiveCreateSecondary(char* label, char* briefText, char* fullText)
{
    if ((fullText == NULL) || (fullText[0] == 0))
    {
    objectiveAndFleetIntelligenceCreate(label, briefText, briefText, 0, FALSE);
    }
    else
    {
    objectiveAndFleetIntelligenceCreate(label, briefText, fullText, 0, FALSE);
    }
}

void kasfObjectiveSet(char *label, sdword status)
{
    objectiveSet(label, status);
}

//void kasfObjectivePopupAll(void)
//{
//    objectivePopupAll();
//}

sdword kasfObjectiveGet(char *label)
{
    return objectiveGet(label);
}

sdword kasfObjectiveGetAll(void)
{
    return objectiveGetAll();
}

void kasfObjectiveDestroy(char *label)
{
    objectiveDestroy(label);
}

void kasfObjectiveDestroyAll(void)
{
    objectiveDestroyAll();
}


sdword kasfNISRunning(void)
{
    return nisIsRunning;
}

sdword kasfShipsSelectEnemy(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = !ship->playerowner->playerIndex;  // enemy
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectFriendly(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = ship->playerowner->playerIndex;  // friendly
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectClass(GrowSelection *newShips, GrowSelection *originalShips, char *shipClass)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;
    ShipClass sc = StrToShipClass(shipClass);

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = isShipOfClass(ship, sc);  // class matches
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectType(GrowSelection *newShips, GrowSelection *originalShips, char *shipType)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;
    ShipType st = StrToShipType(shipType);

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = (ship->shiptype == st);  // class matches
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectCapital(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = isCapitalShip(ship);  // capital ship
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectNonCapital(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = !isCapitalShip(ship);  // non-capital ship
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectNotDocked(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = ((ship->flags & (SOF_Hide | SOF_Hyperspace)) != SOF_Hide);  // not docked
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectDamaged(GrowSelection *newShips, GrowSelection *originalShips, sdword maxHealthPercent)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = (ship->health <= maxHealthPercent * ship->staticinfo->maxhealth / 100);  // low health
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectMoving(GrowSelection *newShips, GrowSelection *originalShips)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = ship->posinfo.isMoving & ISMOVING_MOVING;  // moving
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectIndex(GrowSelection *newShips, GrowSelection *originalShips, sdword Index)
{
    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    // start with empty new selection
    while (newShips->selection->numShips)
         clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // if index is larger than the numships, or if numships is zero
    // return 0
    if ((originalShips->selection->numShips <= Index) ||
        (!originalShips->selection->numShips))
    {
        return 0;
    }

    newShips->selection->ShipPtr[0] = originalShips->selection->ShipPtr[Index];
    newShips->selection->numShips   = 1;

    return 1;
}


// selects the ships in originalShips that are within distance of location
sdword kasfShipsSelectNearby(GrowSelection *newShips, GrowSelection *originalShips, hvector *location, sdword distance)
{
    Ship *ship;
    real32 rangeSquared = (real32)distance*(real32)distance;
    sdword i;
    vector loc;
    vecGrabVecFromHVec(loc,*location);

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    //  wipe the selection clean
    newShips->selection->numShips = 0;

    // add any ships close to the location to the newShips shiplist
    for (i=0;i<originalShips->selection->numShips;i++)
    {
        ship = originalShips->selection->ShipPtr[i];

        if (aiuFindDistanceSquared(ship->posinfo.position,loc) < rangeSquared)
        {
            growSelectAddShip(newShips, ship);
        }
    }

    return newShips->selection->numShips;
}


//local function
/*-----------------------------------------------------------------------------
    Name        : kasfShipsMeetsCriteria
    Description : Returns TRUE if the ship meets the criteria determined by the CriteriaFlag
    Inputs      : ship - the ship being tested, CriteriaFlag - the criteria to test
    Outputs     :
    Return      : TRUE if the ship meets the criteria
----------------------------------------------------------------------------*/
bool kasfShipMeetsCriteria(ShipPtr ship, sdword CriteriaFlag)
{
    switch (CriteriaFlag)
    {
        case kasSpecial_ReturningTechnology:
            if ((ship->shiptype == SalCapCorvette) &&
                (bitTest(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology)))
                return TRUE;
            break;
        case kasSpecial_ShipsDisabled:
            if (bitTest(ship->flags, SOF_Disabled))
                return TRUE;
            break;
        case kasSpecial_ShipsAreArmed:
            switch (ship->staticinfo->shipclass)
            {
                case CLASS_Fighter:
                case CLASS_HeavyCruiser:
                case CLASS_Carrier:
                case CLASS_Destroyer:
                    return TRUE;
                    break;
                case CLASS_Frigate:
                    switch (ship->shiptype)
                    {
                        case CloakGenerator:
                        case ResourceController:
                            return FALSE;
                        default:
                            return TRUE;
                    }
                    break;
                case CLASS_Corvette:
                    switch (ship->shiptype)
                    {
                        case RepairCorvette:
                        case SalCapCorvette:
                            return FALSE;
                        default:
                            return TRUE;
                    }
                    break;
                default:
                    return FALSE;
            }
        case kasSpecial_ShipsDefected:
            if (bitTest(ship->attributes, ATTRIBUTES_Defector))
                return TRUE;
            break;
        default:
            return FALSE;
    }
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : kasfShipsSelectSpecial
    Description : Selects the ships using different criteria depending on the
                  SpecialFlag setting
    Inputs      : newShips - pointer to growselection (pass by reference)
                  originalShips - the ships to filter
                  SpecialFlag - indicates the criteria
    Outputs     :
    Return      : The number of ships selected
----------------------------------------------------------------------------*/
sdword kasfShipsSelectSpecial(GrowSelection *newShips, GrowSelection *originalShips, sdword SpecialFlag)
{
    ShipPtr ship;
    bool selected = FALSE, sourceIsDest = FALSE;
    udword i;

    if ((newShips == NULL) ||
        (newShips->selection == NULL) ||
        (originalShips == NULL) ||
        (originalShips->selection == NULL))
    {
        return 0;
    }

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        //start with an empty new selection
        newShips->selection->numShips = 0;

    for (i=0; i < originalShips->selection->numShips; i++)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = kasfShipMeetsCriteria(ship, SpecialFlag);

        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }
    return newShips->selection->numShips;
}


//
//  A = A + B
//
sdword kasfShipsAdd(GrowSelection *shipsA, GrowSelection *shipsB)
{
    sdword i;

    if (shipsA == NULL || shipsA->selection == NULL ||
        shipsB == NULL || shipsB->selection == NULL)
        return 0;

    if (shipsA == shipsB)
        return shipsA->selection->numShips;

    for (i = 0; i < shipsB->selection->numShips; ++i)
        growSelectAddShipNoDuplication(shipsA, shipsB->selection->ShipPtr[i]);

    return shipsA->selection->numShips;
}

//
//  A = A - B
//
sdword kasfShipsRemove(GrowSelection *shipsA, GrowSelection *shipsB)
{
    sdword i;

    if (shipsA == NULL || shipsA->selection == NULL ||
        shipsB == NULL || shipsB->selection == NULL)
        return 0;

    if (shipsA == shipsB)
    {
        while (shipsA->selection->numShips)
            clRemoveShipFromSelection(shipsA->selection, shipsA->selection->ShipPtr[0]);
        return 0;
    }

    for (i = 0; i < shipsB->selection->numShips; ++i)
        growSelectRemoveShip(shipsA, shipsB->selection->ShipPtr[i]);
    return shipsA->selection->numShips;
}


//returns TRUE if all the selected ships are in the specified formation
sdword kasfSelectedShipsInFormation(sdword formation)
{
    udword i;

    if (!selSelected.numShips)
    {
        return FALSE;
    }

    for (i=0;i<selSelected.numShips;i++)
    {
        if (!selSelected.ShipPtr[i]->formationcommand ||
            (selSelected.ShipPtr[i]->formationcommand->formation.formationtype != (TypeOfFormation)formation))
        {
            return FALSE;
        }
    }

    return TRUE;
}

//returns TRUE if any of the ships are in the specific formation
sdword kasfShipsInFormation(GrowSelection *ships, sdword formation)
{
    udword i;

    if (formation == -1)
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            if (ships->selection->ShipPtr[i]->formationcommand)
            {
                return TRUE;
            }
        }
    }
    else
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            if (ships->selection->ShipPtr[i]->formationcommand &&
                (ships->selection->ShipPtr[i]->formationcommand->formation.formationtype == (TypeOfFormation)formation))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//
// sets the ships so that the player's ships don't autoretaliate against them
//
void kasfShipsSetNonRetaliation(GrowSelection *ships)
{
    udword i;

    for (i=0;i<ships->selection->numShips;i++)
    {
        bitSet(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_FriendlyStatus);
    }
}

//
// sets the ships so that the player's ships don't autoretaliate against them
//
void kasfShipsSetRetaliation(GrowSelection *ships)
{
    udword i;

    for (i=0;i<ships->selection->numShips;i++)
    {
        bitClear(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_FriendlyStatus);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfPingAddSingleShip
    Description : Add a ping for a single ship, the first in the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfPingAddSingleShip(GrowSelection *ships, char *label)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->ShipPtr[0] == NULL)
        return;

    pingAnomalyObjectPingAdd(label, (SpaceObj *)(ships->selection->ShipPtr[0]));
}

/*-----------------------------------------------------------------------------
    Name        : kasfPingAddShips
    Description : Add a ping for a group of ships.  The ping will be centred
                    about the entire group of ships.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfPingAddShips(GrowSelection *ships, char *label)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->ShipPtr[0] == NULL)
        return;

    pingAnomalySelectionPingAdd(label, ships->selection);
}

void kasfPingAddPoint(hvector *point, char *label)
{
    pingAnomalyPositionPingAdd(label, (vector *)point);
}

void kasfPingRemove(char *label)
{
    pingAnomalyPingRemove(label);
}

void kasfStop(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    clWrapHalt(&universe.mainCommandLayer, CurrentTeamP->shipList.selection);
}

void kasfHarvest(void)
{
    MaxSelection tempSelection;
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    //aitDeleteAllTeamMoves(CurrentTeamP);

    if (selection->numShips > 0)
    {
        if (MakeShipsHarvestCapable((SelectCommand *)&tempSelection, selection))
        {
            clWrapCollectResource(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,NULL);
        }
    }
}

void kasfBuildControl(sdword on)
{
    singlePlayerGameInfo.giveComputerFleetControl = on;
}

// sets the team as a building team for the KAS script
void kasfBuildingTeam(struct AITeam *builder)
{
    //for now, just set the first ship on the team (assume it only has one ship)
    aiCurrentAIPlayer->ScriptCreator = builder->shipList.selection->ShipPtr[0];
}

void kasfKamikaze(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateKamikaze(CurrentTeamP, dupeTargets, SAME_FORMATION, FALSE, TRUE);
    }
}

void kasfKamikazeEveryone(GrowSelection *targets)
{
    AITeam *teamp;
    udword i;

    if (targets->selection->numShips && targets->selection->ShipPtr[0] != NULL)
    {
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            teamp = aiCurrentAIPlayer->teams[i];

            if (teamp->shipList.selection->numShips)
            {
                aitDeleteAllTeamMoves(teamp);
                aimCreateKamikaze(teamp, selectDupSelection(targets->selection), SAME_FORMATION, FALSE, TRUE);
            }
        }
    }

}

void kasfSpecialToggle(void)
{
    if (!CurrentTeamP || !CurrentTeamP->shipList.selection->numShips)
        return;
    clWrapSpecial(&universe.mainCommandLayer, CurrentTeamP->shipList.selection, NULL);
}

void kasfShipsDamage(GrowSelection *ships, sdword points)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
        return;

    for (i = 0; i < ships->selection->numShips; ++i)
        ApplyDamageToTarget((struct SpaceObjRotImpTarg *)ships->selection->ShipPtr[i], (real32)points, 0, 0, 0);
}


void kasfForceTaskbar()
{
    tbForceTaskbar(TRUE);
}



/*
----------------------------------------------------------------------------
 The following functions have been added to facilitate the tutorial mission.
----------------------------------------------------------------------------
*/

void kasfBuilderRestrictShipTypes(char *shipTypes)
{
    tutBuilderSetRestrictions(shipTypes, 1);
}

void kasfBuilderUnrestrictShipTypes(char *shipTypes)
{
    tutBuilderSetRestrictions(shipTypes, 0);
}

void kasfBuilderRestrictAll(void)
{
    tutBuilderRestrictAll();
}

void kasfBuilderRestrictNone(void)
{
    tutBuilderRestrictNone();
}

void kasfBuilderCloseIfOpen(void)
{
    cmCloseIfOpen();
}

// figure out how to do this someday....
void kasfForceBuildShipType(char *shipType)
{
    ShipType st;

    st = StrToShipType(shipType);
    cmForceBuildShipType(st);
//    clWrapBuildShip(&universe.mainCommandLayer,st,universe.curPlayerPtr->race,0,universe.curPlayerPtr->PlayerMothership);
}

// Range from 0 to 359 as universe moves from right to left
sdword kasfCameraGetAngleDeg(void)
{
    return (sdword)RAD_TO_DEG(currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle);
}

// Range from -90 (Above) to +90 (Below)
sdword kasfCameraGetDeclinationDeg(void)
{
    return (sdword)RAD_TO_DEG(currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.declination);
}

sdword kasfCameraGetDistance(void)
{
    return (sdword)currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.distance;
}

sdword kasfSelectNumSelected(void)
{
    return selSelected.numShips;
}

sdword kasfSelectIsSelectionShipType(sdword Index, char *shipType)
{
ShipType    st;

    st = StrToShipType(shipType);
    return ((selSelected.numShips > Index) && selSelected.ShipPtr[Index]->shiptype == st);
}

sdword kasfSelectContainsShipTypes(char *shipTypes)
{
    return tutSelectedContainsShipTypes(shipTypes);
}

void kasfTutSetPointerTargetXY(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, x,y);
}

void kasfTutSetPointerTargetXYRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYRight(name, x,y);
}

void kasfTutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYBottomRight(name, x,y);
}

void kasfTutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYTaskbar(name, x,y);
}

void kasfTutSetPointerTargetXYFE(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYFE(name, x,y);
}

void kasfTutSetPointerTargetShip(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShip(name, ships->selection->ShipPtr[0]);
    }
}

void kasfTutSetPointerTargetShipSelection(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShipSelection(name, ships->selection);
    }
}

void kasfTutSetPointerTargetShipHealth(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
        tutSetPointerTargetShipHealth(name, ships->selection->ShipPtr[0]);
}

void kasfTutSetPointerTargetShipGroup(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShipGroup(name, ships->selection->ShipPtr[0]);
    }
}

void kasfTutSetPointerTargetFERegion(char *name, char *pAtomName)
{
    tutSetPointerTargetFERegion(name, pAtomName);
}

void kasfTutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1)
{
    tutSetPointerTargetRect(name, x0, y0, x1, y1);
}

void kasfTutSetPointerTargetAIVolume(char *name, Volume *volume)
{
    tutSetPointerTargetAIVolume(name, volume);
}

void kasfTutRemovePointer(char *name)
{
    tutRemovePointerByName(name);
}

void kasfTutRemoveAllPointers(void)
{
    tutRemoveAllPointers();
}

void kasfTutSetTextDisplayBoxGame(sdword x, sdword y, sdword width, sdword height)
{
    tutSetTextDisplayBox(x, y, width, height, TRUE);
}

void kasfTutSetTextDisplayBoxToSubtitleRegion(void)
{
    tutSetTextDisplayBoxToSubtitleRegion();
}
void kasfTutSetTextDisplayBoxFE(sdword x, sdword y, sdword width, sdword height)
{
    tutSetTextDisplayBox(x, y, width, height, FALSE);
}

void kasfTutShowText(char *szText)
{
    tutShowText(szText);
}

void kasfTutHideText(void)
{
    tutHideText();
}

void kasfTutShowNextButton(void)
{
    tutShowNextButton();
}

void kasfTutHideNextButton(void)
{
    tutHideNextButton();
}

sdword kasfTutNextButtonClicked(void)
{
    return tutNextButtonClicked();
}

void kasfTutShowBackButton(void)
{
    tutShowBackButton();
}

void kasfTutHideBackButton(void)
{
    tutHideBackButton();
}

void kasfTutShowPrevButton(void)
{
    tutShowPrevButton();
}

void kasfTutSaveLesson(sdword Num, char *pName)
{
    tutSaveLesson(Num, pName);
}

void kasfTutShowImages(char *szImages)
{
    tutShowImages(szImages);
}

void kasfTutHideImages(void)
{
    tutHideImages();
}

void kasfTutEnableEverything(void)
{
    tutEnableEverything();
}

void kasfTutDisableEverything(void)
{
    tutDisableEverything();
}

void kasfTutEnableFlags(char *pFlags)
{
    tutSetEnableFlags(pFlags, 1);
}

void kasfTutDisableFlags(char *pFlags)
{
    tutSetEnableFlags(pFlags, 0);
}

void kasfTutForceUnpaused(void)
{
    universePause = FALSE;
}

sdword kasfTutGameSentMessage(char *commandNames)
{
    return tutGameSentMessage(commandNames);
}

void kasfTutResetGameMessageQueue(void)
{
    tutResetGameMessageQueue();
}

sdword kasfTutContextMenuDisplayedForShipType(char *shipType)
{
    return tutContextMenuDisplayedForShipType(shipType);
}

void kasfTutResetContextMenuShipTypeTest(void)
{
    tutResetContextMenuShipTypeTest();
}

void kasfTutRedrawEverything(void)
{
    bitSet(regRootRegion.status, RSF_ReallyDirty);
}

sdword kasfBuildManagerShipTypeInBatchQueue(char *shipType)
{
    return tutBuildManagerShipTypeInBatchQueue(shipType);
}

sdword kasfBuildManagerShipTypeInBuildQueue(char *shipType)
{
    return tutBuildManagerShipTypeInBuildQueue(shipType);
}

sdword kasfBuildManagerShipTypeSelected(char *shipType)
{
    return tutBuildManagerShipTypeSelected(shipType);
}

sdword kasfTutCameraFocusedOnShipType(char *shipTypes)
{
    return tutCameraFocusedOnShipType(shipTypes);
}

void kasfTutCameraFocus(GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        ccCameraTimeoutOverride = TRUE;

        // go to a clean main screen
        if (piePointSpecMode != PSM_Idle)
        {
            piePointModeOnOff();
        }
        spMainScreen();
        ccFocus(&universe.mainCameraCommand, (FocusCommand *)ships->selection);
    }
    else
    {
        //if ships has no ships, focus on selselected
        MaxSelection tempselected;

        tutGameMessage("KB_Focus");

        tempselected = selSelected;     //copy structure
        MakeShipMastersIncludeSlaves((SelectCommand *)&tempselected);
        ccFocus(&universe.mainCameraCommand,(FocusCommand *)&tempselected);
    }

}

void kasfTutCameraFocusDerelictType(char *derelictType)
{
    Derelict *derelict;
    Node *node;
    DerelictSelection focus;
    DerelictType dt = StrToDerelictType(derelictType);

    focus.numDerelicts = 1;

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            ccCameraTimeoutOverride = TRUE;

            // go to a clean main screen
            if (piePointSpecMode != PSM_Idle)
            {
                piePointModeOnOff();
            }
            spMainScreen();

            focus.DerelictPtr[0] = derelict;
            ccFocus(&universe.mainCameraCommand, (FocusCommand *)&focus);
            return;
        }
        node = node->next;
    }
}

void kasfTutCameraFocusFar(GrowSelection *ships)
{
    if (ships->selection->numShips > 0)
    {
        spMainScreen();
        ccCameraTimeoutOverride = TRUE;
        ccFocusFar(&universe.mainCameraCommand, (FocusCommand *)ships->selection, &universe.mainCameraCommand.actualcamera);
    }
    else
    {
        //if ships has no ships, focus on selselected
        MaxSelection tempselected;

        tutGameMessage("KB_Focus");

        tempselected = selSelected;     //copy structure
        MakeShipMastersIncludeSlaves((SelectCommand *)&tempselected);
        ccFocusFar(&universe.mainCameraCommand,(FocusCommand *)&tempselected, &universe.mainCameraCommand.actualcamera);
    }

}


void kasfTutCameraFocusCancel(void)
{
    ccCameraTimeoutOverride = FALSE;
    ccCancelFocus(&universe.mainCameraCommand);
}


/*-----------------------------------------------------------------------------
    Name        : kasfDisablePlayer
    Description : Disables player commands
    Inputs      : toggle - if TRUE the player's commands gets disabled,
                           if FALSE, the player's commands get enabled
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfDisablePlayer(bool toggle)
{
    if (toggle)
    {
        mrDisable();
    }
    else
    {
        mrEnable();
    }
}


sdword kasfTutShipsInView(GrowSelection *ships)
{
    udword i, inview = 0;

    for (i=0;i<ships->selection->numShips;i++)
    {
        if (rndShipVisible((SpaceObj *)ships->selection->ShipPtr[i], mrCamera))
        {
            inview++;
        }
    }

    return inview;
}

sdword kasfTutShipsTactics(GrowSelection *ships)
{
    if (ships->selection->numShips)
        return ships->selection->ShipPtr[0]->tacticstype;
    else
        return -1;
}

sdword kasfTutPieDistance(void)
{
    extern real32 pieDistance;

    return (sdword)pieDistance;
}

sdword kasfTutPieHeight(void)
{
    extern real32 pieHeight;

    return (sdword)pieHeight;
}

void kasfForceFISensors(void)
{
    smFleetIntel = TRUE;
}

void kasfOpenSensors(sdword flag)
{
#if UNIVERSE_TURBOPAUSE_DEBUG
    universeTurbo = FALSE;
#endif
    //if flag isn't a TRUE/FALSE value
    //note: opening the sensors manager at a given distance only
    //      requires the "flag" variable to be larger than 1000 or
    //      so.  If need be, values above 1 can be used to flag
    //      different cases
    if (flag > 1)
    {
        //store the sensors manager zoom length
        kasStoreSMInitialDistance = (sdword)smInitialDistance;
        //set the sensors manager zoom length
        smInitialDistance         = (real32)flag;
        smFleetIntel              = TRUE;
    }
    else
    {
        smFleetIntel = flag;
    }

    spMainScreen();
    smSensorsBegin(NULL, NULL);
}

void kasfCloseSensors(sdword flag)
{
    /*
#ifdef gshaw
    if (smFleetIntel != TRUE)
    {
        return;
    }
#endif
    dbgAssert(smFleetIntel==TRUE);
    */
    if ((smSensorsActive) && (!smZoomingIn) && (!smZoomingOut))
    {
        if (kasStoreSMInitialDistance)
        {
            //restore the sensors manager zoom length
            smInitialDistance    = (real32)kasStoreSMInitialDistance;
            kasStoreSMInitialDistance = 0;
        }

        //flags don't do anything yet
        smSensorsClose(NULL, NULL);
        smFleetIntel = FALSE;
    }

}

sdword kasfSensorsIsOpen(sdword flag)
{
    if (flag)
    {
        //returns true if the fleet intel window is open
        return smFleetIntel;
    }
    else
    {
        //returns true if the player is in the sensors manager but not fleet intel
        return ((smSensorsActive || smZoomingIn || smZoomingOut) && !smFleetIntel);
    }
}

void kasfSensorsWeirdness(sdword value)
{
//    static nisstatic stat2;
//    static nisstatic stat3;

    smSensorWeirdness = value;

/*    if (flag)
    {
        stat1.index = 0;

        nisStaticOnExp(&stat1);
//        nisStaticOnExp(&stat2);
//        nisStaticOnExp(&stat3);
        smSensorWeirdnessInit();
    }
    else
    {
        nisStaticOffExp(0);
//        nisStaticOffExp(1);
//        nisStaticOffExp(2);
    }
*/
}

void kasfAllowPlayerToResearch(char *name)
{
    AllowPlayerToResearch(name);
}

void kasfAllowPlayerToPurchase(char *name)
{
    AllowPlayerToPurchase(name);
}

void kasfPlayerAcquiredTechnology(char *name)
{
    PlayerAcquiredTechnology(name);
}

sdword kasfCanPlayerResearch(char *name)
{
    return CanPlayerResearch(name);
}

sdword kasfCanPlayerPurchase(char *name)
{
    return CanPlayerPurchase(name);
}

sdword kasfDoesPlayerHave(char *name)
{
    return DoesPlayerHave(name);
}

void kasfSetBaseTechnologyCost(char *name, sdword cost)
{
    SetBaseTechnologyCost(name, cost);
}

sdword kasfGetBaseTechnologyCost(char *name)
{
    return GetBaseTechnologyCost(name);
}

sdword kasfTechIsResearching(void)
{
    return rmResearchingAnything(&universe.players[0]);
}

void kasfEnableTraderGUI(void)
{
    tmEnableTraderGUI();
}

sdword kasfTraderGUIActive(void)
{
    return tmTraderGUIActive();
}

void kasfSetTraderDialog(sdword dialogNum, char *text)
{
    tmSetDialog(dialogNum, text);
}

void kasfSetTraderPriceScale(sdword percent)
{
    tmSetPriceScale(percent);
}

void kasfSetTraderDisabled(sdword disable)
{
    tmSetTradeDisabled(disable);
}

sdword kasfGetTraderPriceScale(void)
{
    return tmGetPriceScale();
}

sdword kasfRUsGet(sdword player)
{
    return universe.players[player].resourceUnits;
}

void kasfRUsSet(sdword player, sdword RUs)
{
    universe.players[player].resourceUnits = RUs;
}

sdword kasfGetWorldResources(void)
{
    sdword total, numHarvestables, numAsteroid0s;

    univGetResourceStatistics(&total, &numHarvestables, &numAsteroid0s);

    return total;
}

void kasfSoundEvent(sdword event)
{
    soundEventMisc(event);
}

void kasfSoundEventShips(GrowSelection *ships, sdword event)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->numShips == 0)
        return;

    soundEvent(ships->selection->ShipPtr[0], event);
}

void kasfSpeechEvent(sdword event, sdword variable)
{
    subMessageEnded = 0;
#ifndef _MACOSX_FIX_ME
    speechEventFleet(event, variable, universe.curPlayerIndex);
#endif
}

void kasfSpeechEventShips(GrowSelection *ships, sdword event, sdword variable)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->numShips == 0)
        return;

    speechEvent(ships->selection->ShipPtr[0], event, variable);
}

void kasfToggleActor(sdword Actor, sdword on)
{
    soundEventSetActorFlag(Actor, on);
}

void kasfMusicPlay(sdword trackNum)
{
    soundEventPlayMusic((udword)trackNum);
}

void kasfMusicStop(sdword fadeTime)
{
    soundEventStopMusic((real32)fadeTime);
}

sdword kasfRenderedShips(GrowSelection *ships, sdword LOD)
{
    sdword i;
    ubyte LODmask;

    for (LODmask = 0, i = 8; i > 0; i--)
    {                                                       //prepare a mask for the given LOD
        if (i <= LOD)
        {
            LODmask |= 1;
        }
        LODmask <<= 1;
    }

    if (ships == NULL || ships->selection == NULL)
        return 0;

    for (i = 0; i < ships->selection->numShips; ++i)        //see if ship rendered at the specified LOD or lower
    {
        if (bitTest(ships->selection->ShipPtr[i]->renderedLODs, LODmask))
        {
            return 1;
        }
    }

    return 0;
}

void kasfResetShipRenderFlags(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
        return;

    for (i=0; i < ships->selection->numShips; i++)
        ships->selection->ShipPtr[i]->renderedLODs = 0;
}

sdword kasfRenderedDerelicts(char *derelictType, sdword LOD)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if ((derelict->derelicttype == dt) &&
            bitTest(derelict->renderedLODs, 1 << LOD))
            return 1;
        node = node->next;
    }
    return 0;
}

void kasfResetDerelictRenderFlags(char *derelictType)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            derelict->renderedLODs = 0;
            break;
        }
        node = node->next;
    }
}

void kasfGateMoveToNearest(void)
{
    real32 temp;
    real32 closestDistSquared = -1;
    sdword i;
    vector loc;
    vector shippos;
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    Derelict *gate = NULL;
    Derelict *closest = NULL;

    if (selection == NULL || !selection->numShips)
        return;

    shippos = selection->ShipPtr[0]->posinfo.position;

    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        if (!strncasecmp(LabelledVectors[i]->label, "GATE", 4))
        {
            vecGrabVecFromHVec(loc, *(LabelledVectors[i]->hvector));
            temp = aiuFindDistanceSquared(shippos,loc);

            if (closestDistSquared == -1 || temp <= closestDistSquared)
            {
                if ((gate = GetHyperspaceGateFromVector(&loc)) != NULL)
                {
                    closestDistSquared = temp;
                    closest = gate;
                }
            }
        }
    }

    if (closestDistSquared != -1 && closest)
    {
        // we now have a pointer to the hyperspace gate derelict
        vector shipToGate;
        vector gateNormal;
        vector waypoint;
        vector gatePosition;

        gatePosition = closest->posinfo.position;

        vecSub(shipToGate,gatePosition,shippos);
        matGetVectFromMatrixCol3(gateNormal,closest->rotinfo.coordsys);

        if (vecDotProduct(shipToGate,gateNormal) > 0.0f)
        {
            // shipToGate and gateNormal are pointing in same direction...pick back direction;
            vecNegate(gateNormal);
        }

        vecMultiplyByScalar(gateNormal,HYPERSPACEGATE_WAYPOINTDIST);
        vecAdd(waypoint,gateNormal,gatePosition);

        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateMoveTeam(CurrentTeamP, waypoint, SAME_FORMATION, TRUE, TRUE);
        aimCreateMoveTeam(CurrentTeamP, gatePosition, SAME_FORMATION, TRUE, TRUE);
    }
}

void kasfGateDestroy(hvector *gatePoint)
{
    //hyperspace
    hsStaticDestroy(gatePoint);
}

void kasfGateShipsIn(GrowSelection *ships, hvector *gatePoint)
{
    //hyperspace
    spHyperspaceSelectionInStatic(ships->selection, gatePoint);
}

void kasfGateShipsOut(GrowSelection *ships, hvector *gatePoint)
{
    //hyperspace
    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,ships->selection);
    spHyperspaceSelectionOutStatic(ships->selection);
}


sdword kasfGateShipsOutNearest(GrowSelection *ships)
{
    hvector *closest = NULL;
    real32 temp;
    real32 closestDistSquared = -1;
    sdword i;
    vector loc;
    SelectCommand *selection;

    selection = ships->selection;

    if (selection == NULL || !selection->numShips)
        return 0;

    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        if (!strncasecmp(LabelledVectors[i]->label, "GATE", 4))
        {
            vecGrabVecFromHVec(loc, *(LabelledVectors[i]->hvector));
            temp = aiuFindDistanceSquared(
                        selection->ShipPtr[0]->posinfo.position,
                        loc);
            if (closestDistSquared == -1 || temp <= closestDistSquared)
            {
                closestDistSquared = temp;
                closest = LabelledVectors[i]->hvector;
            }
        }
    }

    if (closestDistSquared != -1 && closest)
    {
        if (closestDistSquared < 4000000)
        {
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,selection);
            spHyperspaceSelectionOutStatic(selection);
            return TRUE;
        }
    }
    return FALSE;
}

//creates a list of the player's currently selected ships
//added for the tutorial
udword kasfFindSelectedShips(GrowSelection *ships)
{
    udword i;

    if (ships == NULL)
        return 0;

    while (ships->selection->numShips)
        clRemoveShipFromSelection(ships->selection, ships->selection->ShipPtr[0]);

    for (i=0;i<selSelected.numShips;i++)
        growSelectAddShip(ships, selSelected.ShipPtr[i]);

    return selSelected.numShips;
}

//
//  returns how many ships in the selection are disabled
//
sdword kasfShipsDisabled(GrowSelection *ships)
{
    udword i;
    sdword count = 0;

    if (ships == NULL)
        return 0;

    for (i = 0; i < ships->selection->numShips; ++i)
    {
        if (bitTest(ships->selection->ShipPtr[i]->flags, SOF_Disabled))
            ++count;
    }

    return count;
}

//saves a single player level
void kasfSaveLevel(sdword LevelNum, char *LevelName)
{
//    char levelName[256];

//LM:Why is this here?  It trips an assertion...    btgSetColourMultiplier(-1.0f);

    collUpdateCollBlobs();
    collUpdateObjsInCollBlobs();

    sprintf(CurrentLevelName, "%s%s", SinglePlayerSavedGamesPath, LevelName);
    if (SaveGame(CurrentLevelName))
        clCommandMessage(strGetString(strSavedGame));
    else
        clCommandMessage(strGetString(strPatchUnableWriteFile));
}

/*-----------------------------------------------------------------------------
    Name        : kasfClearScreen
    Description : Refreshes the screen
    Inputs      : none
    Outputs     : the player's monitor feels a clearing sensation
    Return      : void
----------------------------------------------------------------------------*/
void kasfClearScreen(void)
{
    rndClear();
}


//starts the NIS widescreen thing under KAS control
void kasfWideScreenIn(sdword frames)
{
#if UNIVERSE_TURBOPAUSE_DEBUG
    universeTurbo = FALSE;
#endif
    dbgAssert(frames > 1 && frames < 10000);
    if (nisScissorFadeOut != 0)
    {
        nisScissorFade = nisScissorFadeOut = nisScissorFadeTime = 0.0f;
    }
    soundEvent(NULL, UI_Letterbox);
    nisScissorFadeIn = (real32)frames / NIS_FrameRate;
    nisTaskResume();
}

//ends the NIS widescreen thing under KAS control
void kasfWideScreenOut(sdword frames)
{
    dbgAssert(frames > 1 && frames < 10000);
    if (nisScissorFadeIn != 0)
    {
        nisScissorFade = nisScissorFadeIn = nisScissorFadeTime = 0.0f;
    }
    soundEvent(NULL, UI_Unletterbox);
    nisScissorFadeOut = (real32)frames / NIS_FrameRate;
    nisTaskResume();
}

//simulate a subtitle
static sdword kasfDummyEventNumber = 0;         //incremented to ensure uniquie event numbers
void kasfSubtitleSimulate(sdword actor, sdword milliseconds, char *speech)
{
    dbgAssert(milliseconds > 10);
    subTitleAdd(actor, kasfDummyEventNumber, speech, strlen(speech), (real32)milliseconds / 1000.0f);
    subMessageEnded = 0;
    kasfDummyEventNumber++;
}

//display a location card
void kasfLocationCard(sdword milliseconds, char *location)
{
    char buffer[SUB_SubtitleLength];

    dbgAssert(milliseconds > 10);
    snprintf(buffer, SUB_SubtitleLength - 1, "#r%x#t%x%s", STR_LocationCard, STT_LocationCard, location);
    subTitleAdd(STA_LocationCard, kasfDummyEventNumber, buffer, strlen(buffer), (real32)milliseconds / 1000.0f);
    kasfDummyEventNumber++;
}

//display easter egg location card
void kasfLocationCardSpecial(sdword milliseconds, char *string)
{
//    char buffer[SUB_SubtitleLength];

    dbgAssert(milliseconds > 10);
    subTitleAdd(STA_LocationCard, kasfDummyEventNumber, string, strlen(string), (real32)milliseconds / 1000.0f);
    kasfDummyEventNumber++;
}

sdword kasfRaceOfHuman(void)
{
    return universe.players[0].race;
}

//hides some ships and sets the SOF_HideSave bit where appropriate so the NIS can unhide them
void kasfHideShips(GrowSelection *ships)
{
    sdword index;

    for (index = 0; index < ships->selection->numShips; index++)
    {
        if (bitTest(ships->selection->ShipPtr[index]->flags, SOF_Hide))
        {                                                   //if ship already hidden
            bitSet(ships->selection->ShipPtr[index]->flags, SOF_HideSave);
        }
        else
        {
            univRemoveObjFromRenderList((SpaceObj *)ships->selection->ShipPtr[index]);
            bitSet(ships->selection->ShipPtr[index]->flags, SOF_Hide);

        }
    }
}

//unhides some ships and sets the SOF_HideSave bit where appropriate so the NIS can unhide them
void kasfUnhideShips(GrowSelection *ships)
{
    sdword index;

    for (index = 0; index < ships->selection->numShips; index++)
    {
        if (bitTest(ships->selection->ShipPtr[index]->flags, SOF_HideSave))
        {                                                   //if ship was hidden to begin with
            bitClear(ships->selection->ShipPtr[index]->flags, SOF_HideSave);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[index]->flags, SOF_Hide);

        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : kasfDeleteShips
    Description : Deletes a select of ships from the game
    Inputs      : ships - the ships to delete
    Outputs     : wipes the ships from the game
    Return      : void
----------------------------------------------------------------------------*/
void kasfDeleteShips(GrowSelection *ships)
{
    while (ships->selection->numShips)
    {
        univWipeShipOutOfExistence(ships->selection->ShipPtr[0]);
    }
}



/*-----------------------------------------------------------------------------
    Name        : kasfRotateDerelictType
    Description : Rotates a type of derelict
    Inputs      : derelictType - string of derelict type
                  rot_x - rotation speed around x axis  (*1000)
                  rot_y - rotation speed around y axis  (*1000)
                  rot_z - rotation speed around z axis  (*1000)
                  variation - amount of random rotation variation
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfRotateDerelictType(char *derelictType, sdword rot_x, sdword rot_y, sdword rot_z, sdword variation)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);
    real32 realrot_x     = (real32)rot_x,
           realrot_y     = (real32)rot_y,
           realrot_z     = (real32)rot_z,
           realvariation = (real32)variation;

    // scale rotation values to their proper values
    realrot_x     /= 1000;
    realrot_y     /= 1000;
    realrot_z     /= 1000;
    realvariation /= 1000;

    //go through the entire list of derelicts and
    //give the appropriate ones a rotation
    node = universe.DerelictList.head;

    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            derelict->rotinfo.rotspeed.x = realrot_x + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
            derelict->rotinfo.rotspeed.y = realrot_y + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
            derelict->rotinfo.rotspeed.z = realrot_z + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
        }
        node = node->next;
    }
}



/*-----------------------------------------------------------------------------
    Name        : kasfUniversePause
    Description : Pause all ships in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfUniversePause(void)
{
    nisUniversePause = TRUE;
#if NIS_VERBOSE_LEVEL >= 1
    dbgMessage("\nKAS has PAUSED the universe.");
#endif
    //!!! pause KAS timers and FSM's
}

/*-----------------------------------------------------------------------------
    Name        : kasfUniverseUnpause
    Description : Pause all ships in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfUniverseUnpause(void)
{
    nisUniversePause = FALSE;
#if NIS_VERBOSE_LEVEL >= 1
    dbgMessage("\nKAS has UNPAUSED the universe.");
#endif
    //!!! unpause KAS timers and FSM's
}

// pause the build

void kasfPauseBuildShips(void)
{
    cmPauseAllJbos();
}

void kasfUnpauseBuildShips(void)
{
    cmUnPauseAllJobs();
}


/*-----------------------------------------------------------------------------
    Name        : kasfOtherKASPause/kasfOtherKASUnpause
    Description : Pause/unPause global KAS watches and timers not associated with
                    this FSM.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfOtherKASPause(void)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2];

    kasUnpausedTeam = CurrentTeamP;
    dbgAssert(CurrentMissionScope != KAS_SCOPE_MISSION);    //make sure we're operating at least at FSM level scope
    sprintf(scopedName, "%s.", CurrentMissionScopeName);
    timTimerPauseAllNotScoped(scopedName);                  //pause all the other timers
}
void kasfOtherKASUnpause(void)
{
    kasUnpausedTeam = NULL;
    timTimerUnpauseAll();                                   //unpause all paused timers
}

/*-----------------------------------------------------------------------------
    Name        : kasfIntelEventEnded
    Description : Returns TRUE only once if a fleet intel/command subtitled
                    speech event ends or the player hits SPACE to short-cut
                    the event.
    Inputs      :
    Outputs     :
    Return      : 1 if the current fleet intel subtitle expired, 2 if the user
                    hit space in the sensors manager.
----------------------------------------------------------------------------*/
sdword kasfIntelEventEnded(void)
{
    sdword returnValue = subMessageEnded;
    subMessageEnded = 0;                                //only return TRUE once for each event

    // safety to make sure eventualy intel event ends
    if (!returnValue)
    {
        if (subMessageReturnedFalseTime == 0.0f)
        {
            subMessageReturnedFalseTime = universe.totaltimeelapsed;
        }
        else
        {
            if ( ((universe.totaltimeelapsed - subMessageReturnedFalseTime) > SUBMESSAGE_SAFETY_TIMEOUT) &&
                 (!subAnyCardsOnScreen()) )
            {
                subMessageReturnedFalseTime = 0.0f;
                return 1;           // subtitle finished by timeout
            }
        }
    }
    else
    {
        subMessageReturnedFalseTime = 0.0f;
    }

    return(returnValue);
}

/*-----------------------------------------------------------------------------
    Name        : kasfIntelEventNotEnded
    Description : Clears the subMessageEnded flag.  Should be called at the
                    init of any FSM that will use "IntelEventEnded"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfIntelEventNotEnded(void)
{
    subMessageEnded = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : kasfForceIntelEventEnded
    Description : Clears the subMessageEnded flag.  Should be called at the
                    init of any FSM that will use "IntelEventEnded"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfForceIntelEventEnded(void)
{
    subMessageEnded = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : kasfSensorsStaticOn
    Description : Turns on sensors manager static
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfSensorsStaticOn(void)
{
    static nisstatic newStatic;                             //needs to be persistant

    newStatic.index = NIS_SMStaticIndex;
    //!!! make these values tweakable (see nisStaticOnSet for an example)
    newStatic.nLines = 50;
    newStatic.nLinesVariation = 5;
    newStatic.width = primScreenToGLScaleX(10);
    newStatic.widthVariation = primScreenToGLScaleX(1);
    newStatic.y = primScreenToGLY(MAIN_WindowHeight / 2);
    newStatic.yVariation = primScreenToGLScaleY(MAIN_WindowHeight / 2);
    newStatic.hue = 0.5f;
    newStatic.hueVariation = 0.5f;
    newStatic.lum = 0.9f;
    newStatic.lumVariation = 0.1f;
    newStatic.sat = 0.5f;
    newStatic.satVariation = 0.1f;
    newStatic.alpha = 0.6f;
    newStatic.alphaVariation = 0.1f;
    newStatic.bAlpha = TRUE;
    newStatic.bTextured = FALSE;
    newStatic.texture = TR_InvalidInternalHandle;

    nisStaticOnExp(&newStatic);
}

/*-----------------------------------------------------------------------------
    Name        : kasfSensorsStaticOff
    Description : Turns off static in the sensors manager.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfSensorsStaticOff(void)
{
    nisStaticOffExp(NIS_SMStaticIndex);
}

/*-----------------------------------------------------------------------------
    Name        : kasfGameEnd
    Description : Quit the game and play the exit plugscreens.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfGameEnd(void)
{
// this stuff only happens during demos
#if defined(CGW) || defined (Downloadable) || defined(OEM)
    // quit the game here
    universe.quittime = universe.totaltimeelapsed;
    utyPlugScreens = TRUE;
    //spMainScreen();
    //utyGameQuitToPlugScreens(NULL,NULL);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : kasfSpawnEffect
    Description : Spawns an effect with a single float parameter
    Inputs      : ships - ship(s) to spawn the effect on.
                  effectName - name of the effect
                  parameter - parameter to pass in (to be converted to a float)
    Outputs     :
    Return      :
    Note        : the first parameter to the effect will be the size of the ship.
----------------------------------------------------------------------------*/
void kasfSpawnEffect(GrowSelection *ships, char *effectName, sdword parameter)
{
    real32 floatParameter;
    udword intParameter, intSize, index, count;
    static char *lastName = NULL;
    static etgeffectstatic *lastEffect;
    Ship **shipPtr;

    //retain name so this lookup does not have ot happen every single time
    if (effectName != lastName)
    {
        lastName = effectName;
        lastEffect = etgEffectStaticFind(effectName, FALSE);
#if ETG_ERROR_CHECKING
        if (lastEffect == NULL)
        {
            dbgFatalf(DBG_Loc, "Effect '%s' not found for KAS execution", effectName);
        }
#endif
    }
    floatParameter = (real32)parameter;
    intParameter = TreatAsUdword(floatParameter);       //get the user parameter
    count = ships->selection->numShips;
    shipPtr = ships->selection->ShipPtr;
    for (index = 0; index < count; index++, shipPtr++)
    {
        floatParameter = ((ShipStaticInfo *)((*shipPtr)->staticinfo))->staticheader.staticCollInfo.collspheresize;
        intSize = TreatAsUdword(floatParameter);            //get the size

        etgEffectCreate(lastEffect, *shipPtr, NULL, NULL, NULL, 0.0f, EAF_Full, 2, intSize, intParameter);
    }
}

