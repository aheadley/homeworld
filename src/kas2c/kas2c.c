//
//  support code for kas2c.y
//
//  Created 1998/06/10 by Darren Stone
//

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "kas2c.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

//#define LOTS_OF_DEBUGS 1

extern char *yytname[];

extern FILE *yyin, *yyout, *yyhout, *yyfout;

extern unsigned int functionCount;

extern int yynerrs;

extern int lineNumGet();
extern char *curFilenameGet();
char stateHelp[1024];

extern int parseLevel;

extern int ifOnceIndex;

static int fsmCount = 0;
static char fsmName[MAX_FSM_COUNT][MAX_FSM_NAME_LENGTH+1];
static int fsmCurrent;

static int stateCount = 0;
static char stateName[MAX_STATE_COUNT][MAX_STATE_NAME_LENGTH+1];
static int stateCurrent;

static int lstringCount = 0;
static char lstringName[MAX_LSTRING_COUNT][MAX_LSTRING_NAME_LENGTH+1];
static int lstringValueCount;

// function call stack
static CallStack callStack[MAX_FUNC_DEPTH];
static int curFuncDepth = -1;

//
//  exposed functions and default params
//
//  ensure you add the appropriate #include statements to yyout if necessary
//
static FunctionCall functions[] =
{
    { "TimerCreate",      0, "kasfTimerCreate", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerSet",         0, "kasfTimerSet", 2, {
        { "name",         PARAM_CHARPTR},
        { "duration",     PARAM_SDWORD},
    } },
    { "TimerStart",       0, "kasfTimerStart", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerCreateSetStart", 0, "kasfTimerCreateSetStart", 2, {
        { "name",         PARAM_CHARPTR},
        { "duration",     PARAM_SDWORD},
    } },
    { "TimerStop",        0, "kasfTimerStop", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerRemaining",   1, "kasfTimerRemaining", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerExpired",     1, "kasfTimerExpired", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerExpiredDestroy", 1, "kasfTimerExpiredDestroy", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "TimerDestroy",     0, "kasfTimerDestroy", 1, {
        { "name",         PARAM_CHARPTR},
    } },

    { "VarCreate",        0, "kasfVarCreate", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "VarSet",           0, "kasfVarValueSet", 2, {
        { "name",         PARAM_CHARPTR},
        { "value",        PARAM_SDWORD},
    } },
    { "VarCreateSet",     0, "kasfVarCreateSet", 2, {
        { "name",         PARAM_CHARPTR},
        { "value",        PARAM_SDWORD},
    } },
    { "VarInc",           0, "kasfVarValueInc", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "VarDec",           0, "kasfVarValueDec", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "VarGet",           1, "kasfVarValueGet", 1, {
        { "name",         PARAM_CHARPTR},
    } },
    { "VarDestroy",       0, "kasfVarDestroy", 1, {
        { "name",         PARAM_CHARPTR},
    } },

    { "MsgSend",          0, "kasfMsgSend", 2, {
        { "team",         PARAM_AITEAMPTR},
        { "msg",          PARAM_CHARPTR},
    } },
    { "MsgSendAll",       0, "kasfMsgSendAll", 1, {
        { "msg",          PARAM_CHARPTR},
    } },
    { "MsgReceived",      1, "kasfMsgReceived", 1, {
        { "msg",          PARAM_CHARPTR},
    } },

    { "MoveTo",           0, "kasfMoveTo", 1, {
        { "destination",  PARAM_VECTORPTR},
    } },

    { "ShipsMoveTo",           0, "kasfShipsMoveTo", 2, {
        { "ships",      PARAM_GROWSELECTIONPTR},
        { "destination",  PARAM_VECTORPTR},
    } },

    { "Attack",           0, "kasfAttack", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "AttackSpecial",    0, "kasfAttackSpecial", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "AttackFlank",      0, "kasfAttackFlank", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "MoveAttack",       0, "kasfMoveAttack", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "AttackHarass",     0, "kasfAttackHarass", 0, {
        { "", 0 },
    } },
    { "AttackMothership", 0, "kasfAttackMothership", 0, {
        { "", 0 },
    } },
    { "BulgeAttack",    1, "kasfBulgeAttack", 4,{
        { "targets",      PARAM_GROWSELECTIONPTR },
        { "bulgeTargets", PARAM_GROWSELECTIONPTR },
        { "attackers",    PARAM_GROWSELECTIONPTR },
        { "radius",       PARAM_SDWORD },
    } },

    { "Intercept",      0, "kasfIntercept", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },

    { "SetSwarmTargets",  0, "kasfSetSwarmerTargets", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "SwarmMoveTo",      0, "kasfSwarmMoveTo", 1, {
        { "targets",      PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsAttack",      0, "kasfShipsAttack", 2, {
        { "targets",        PARAM_GROWSELECTIONPTR},
        { "attackers",      PARAM_GROWSELECTIONPTR},
    } },
    { "TargetDrop",       0, "kasfTargetDrop", 0, {
        { "", 0 },
    } },

    { "FormationDelta",   0, "kasfFormationDelta", 0, {
        { "", 0 },
    } },
    { "FormationBroad",   0, "kasfFormationBroad", 0, {
        { "", 0 },
    } },
    { "FormationDelta3D", 0, "kasfFormationDelta3D", 0, {
        { "", 0 },
    } },
    { "FormationClaw",    0, "kasfFormationClaw", 0, {
        { "", 0 },
    } },
    { "FormationWall",    0, "kasfFormationWall", 0, {
        { "", 0 },
    } },
    { "FormationSphere",  0, "kasfFormationSphere", 0, {
        { "", 0 },
    } },
    { "FormationCustom",  0, "kasfFormationCustom", 1, {
        { "ships",  PARAM_GROWSELECTIONPTR},
    } },

    { "Random",           1, "kasfRandom", 2, {
        { "lowestNum",    PARAM_SDWORD},
        { "highestNum",   PARAM_SDWORD},
    } },

    { "Guard",            0, "kasfGuardShips", 1, {
        { "ships",        PARAM_GROWSELECTIONPTR},
    } },
    { "GuardMothership",  0, "kasfGuardMothership", 0, {
        { "", 0 },
    } },

    { "Print",            0, "kasfCommandMessage", 1, {
        { "string",       PARAM_CHARPTR},
    } },
    { "Log",              0, "kasfLog", 1, {
        { "string",       PARAM_CHARPTR},
    } },
    { "LogInteger",       0, "kasfLogInteger", 2, {
        { "string",       PARAM_CHARPTR},
        { "integer",      PARAM_SDWORD},
    } },

    { "Popup",            0, "kasfPopup", 1, {
        { "string",       PARAM_CHARPTR},
    } },
    { "PopupInteger",     0, "kasfPopupInteger", 2, {
        { "string",       PARAM_CHARPTR},
        { "integer",      PARAM_SDWORD},
    } },

    { "MissionCompleted", 0, "kasfMissionCompleted", 0, {
        { "", 0 },
    } },
    { "MissionFailed",    0, "kasfMissionFailed", 0, {
        { "", 0 },
    } },

    { "FadeToWhite",    0, "kasfFadeToWhite", 0, {
        { "", 0 },
    } },

    { "Patrol",           0, "kasfPatrolActive", 0, {
        { "", 0 },
    } },
    { "PatrolPath",       0, "kasfPatrolPath", 1, {
        { "path",         PARAM_PATHPTR},
    } },
    { "PathNextPoint",       1, "kasfPathNextPoint", 0, {
        { "", 0 },
    } },

    { "PointInside",      1, "kasfPointInside", 2, {
        { "volume",       PARAM_VOLUMEPTR},
        { "point",        PARAM_VECTORPTR},
    } },

    { "ShipsClear",        0, "kasfGrowSelectionClear", 1, {
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsCount",        1, "kasfShipsCount", 1, {
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsCountType",    1, "kasfShipsCountType", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "shipType",      PARAM_CHARPTR},
    } },
    { "ShipsAdd",          1, "kasfShipsAdd", 2, {
        { "shipsA",        PARAM_GROWSELECTIONPTR},
        { "shipsB",        PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsRemove",       1, "kasfShipsRemove", 2, {
        { "shipsA",        PARAM_GROWSELECTIONPTR},
        { "shipsB",        PARAM_GROWSELECTIONPTR},
    } },

    { "ShipsDisabled",     1, "kasfShipsDisabled", 1, {
        { "ships",        PARAM_GROWSELECTIONPTR},
    } },

    { "ShipsSelectEnemy",  1, "kasfShipsSelectEnemy", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsSelectFriendly",  1, "kasfShipsSelectFriendly", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsSelectClass",  1, "kasfShipsSelectClass", 3, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "shipClass",     PARAM_CHARPTR},
    } },
    { "ShipsSelectType",  1, "kasfShipsSelectType", 3, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "shipType",     PARAM_CHARPTR},
    } },
    { "ShipsSelectDamaged", 1, "kasfShipsSelectDamaged", 3, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "maxHealthPercent",     PARAM_SDWORD},
    } },
    { "ShipsSelectMoving", 1, "kasfShipsSelectMoving", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsSelectCapital",  1, "kasfShipsSelectCapital", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsSelectNonCapital",  1, "kasfShipsSelectNonCapital", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },
    { "ShipsSelectNotDocked",   1, "kasfShipsSelectNotDocked", 2, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
    } },

    { "ShipsSelectIndex",   1, "kasfShipsSelectIndex", 3, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "Index",         PARAM_SDWORD},
    } },

    { "ShipsSelectNearby",   1, "kasfShipsSelectNearby", 4, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "location",      PARAM_VECTORPTR},
        { "Index",         PARAM_SDWORD},
    } },

    { "ShipsSelectSpecial",   1, "kasfShipsSelectSpecial", 3, {
        { "newShips",      PARAM_GROWSELECTIONPTR},
        { "originalShips", PARAM_GROWSELECTIONPTR},
        { "specialFlag",   PARAM_SDWORD},
    } },

    { "FindShipsInside",   1, "kasfFindShipsInside", 2, {
        { "volume",        PARAM_VOLUMEPTR},
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },
    { "FindEnemiesInside", 1, "kasfFindEnemiesInside", 3, {
        { "volume",        PARAM_VOLUMEPTR},
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "neighborRadius",PARAM_SDWORD},
    } },
    { "FindEnemiesNearby", 1, "kasfFindEnemiesNearby", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "radius",        PARAM_SDWORD},
    } },
    { "FindEnemiesNearTeam", 1, "kasfFindEnemiesNearTeam", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "radius",        PARAM_SDWORD},
    } },
    { "FindEnemyShipsOfType", 1, "kasfFindEnemyShipsOfType", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "shipType",      PARAM_CHARPTR},
    } },
    { "FindFriendlyShipsOfType", 1, "kasfFindFriendlyShipsOfType", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "shipType",      PARAM_CHARPTR},
    } },
    { "FindEnemyShipsOfClass", 1, "kasfFindEnemyShipsOfClass", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "shipClass",     PARAM_CHARPTR},
    } },
    { "FindFriendlyShipsOfClass", 1, "kasfFindFriendlyShipsOfClass", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "shipClass",     PARAM_CHARPTR},
    } },

    { "FindShipsNearPoint", 1, "kasfFindShipsNearPoint", 3, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "location",     PARAM_VECTORPTR},
        { "radius",       PARAM_SDWORD},
    } },

    { "TeamHealthAverage",  1, "kasfTeamHealthAverage", 0, {
        { "", 0 },
    } },
    { "TeamHealthLowest",   1, "kasfTeamHealthLowest", 0, {
        { "", 0 },
    } },
    { "TeamFuelAverage",    1, "kasfTeamFuelAverage", 0, {
        { "", 0 },
    } },
    { "TeamFuelLowest",     1, "kasfTeamFuelLowest", 0, {
        { "", 0 },
    } },
    { "TeamCount",          1, "kasfTeamCount", 0, {
        { "", 0 },
    } },
    { "TeamCountOriginal",  1, "kasfTeamCountOriginal", 0, {
        { "", 0 },
    } },

    { "NewShipsAdded",      1, "kasfNewShipsAdded", 0, {
        { "", 0 },
    } },

    { "ThisTeamIs",         1, "kasfThisTeamIs", 1, {
        { "team",           PARAM_AITEAMPTR},
    } },

    { "TeamSkillSet",       0, "kasfTeamSkillSet", 1, {
        { "skillLevel",     PARAM_SDWORD},
    } },
    { "TeamSkillGet",       1, "kasfTeamSkillGet", 0, {
        { "", 0 },
    } },

    { "TeamMakeCrazy",      0, "kasfTeamMakeCrazy", 1, {
        { "makecrazy",     PARAM_SDWORD},
    } },

    { "TeamAttributesBitSet",  0, "kasfTeamAttributesBitSet", 1, {
        { "attributes",     PARAM_SDWORD},
    } },
    { "TeamAttributesBitClear",0, "kasfTeamAttributesBitClear", 1, {
        { "attributes",     PARAM_SDWORD},
    } },
    { "TeamAttributesSet",  0, "kasfTeamAttributesSet", 1, {
        { "attributes",     PARAM_SDWORD},
    } },

    { "TeamVelocityMaxSet",  0, "kasfTeamSetMaxVelocity", 1, {
        { "maxVelocity",     PARAM_SDWORD},
    } },
    { "TeamVelocityMaxClear",0, "kasfTeamClearMaxVelocity", 0, {
        { "", 0 },
    } },
    { "ShipsVelocityMaxSet",  0, "kasfShipsSetMaxVelocity", 2, {
        { "ships",           PARAM_GROWSELECTIONPTR },
        { "maxVelocity",     PARAM_SDWORD},
    } },
    { "ShipsVelocityMaxClear",0, "kasfShipsClearMaxVelocity", 1, {
        { "ships",           PARAM_GROWSELECTIONPTR },
    } },
    { "ShipsDamageModifierSet",  0, "kasfShipsSetDamageFactor", 2, {
        { "ships",           PARAM_GROWSELECTIONPTR },
        { "damageModifier",     PARAM_SDWORD},
    } },
    { "ShipsDamageModifierClear",0, "kasfShipsClearDamageFactor", 1, {
        { "ships",           PARAM_GROWSELECTIONPTR },
    } },
    { "TeamHealthSet",       0, "kasfTeamSetPercentDamaged", 1, {
        { "percentHealth",     PARAM_SDWORD},
    } },
    { "TeamFuelSet",       0, "kasfTeamSetPercentFueled", 1, {
        { "percentFueled",     PARAM_SDWORD},
    } },
    { "DisablePlayerHyperspace",    0, "kasfDisablePlayerHyperspace", 0, {
        { "", 0},
    } },
    { "HoldHyperspaceWindow",    0, "kasfHoldHyperspaceWindow", 1, {
        { "Hold", PARAM_BOOL},
    } },
    { "TeamHyperspaceOut",   0, "kasfTeamHyperspaceOut", 0, {
      { "", 0 },
    } },
    { "TeamHyperspaceIn",    0, "kasfTeamHyperspaceIn", 1, {
        { "destination",  PARAM_VECTORPTR},
    } },

    { "TeamHyperspaceInNear",   0, "kasfTeamHyperspaceInNear", 2, {
        { "destination",  PARAM_VECTORPTR},
        { "distance",     PARAM_SDWORD},
    } },

    { "ShipsAttributesBitSet",  0, "kasfShipsAttributesBitSet", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR},
        { "attributes",     PARAM_SDWORD},
    } },
    { "ShipsAttributesBitClear",0, "kasfShipsAttributesBitClear", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR},
        { "attributes",     PARAM_SDWORD},
    } },
    { "ShipsAttributesSet",  0, "kasfShipsAttributesSet", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR},
        { "attributes",     PARAM_SDWORD},
    } },

    { "ShipsSetNonRetaliation",0,"kasfShipsSetNonRetaliation", 1, {
        { "ships",          PARAM_GROWSELECTIONPTR},
    } },

    { "ShipsSetRetaliation",0,"kasfShipsSetRetaliation", 1, {
        { "ships",          PARAM_GROWSELECTIONPTR},
    } },

    { "GateDestroy",      0, "kasfGateDestroy", 1, {
        { "gatePoint",        PARAM_VECTORPTR},
    } },
    { "GateShipsOut",       0, "kasfGateShipsOut", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "gatePoint",     PARAM_VECTORPTR},
    } },
    { "GateShipsIn",       0, "kasfGateShipsIn", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "gatePoint",     PARAM_VECTORPTR},
    } },
    { "GateMoveToNearest", 0, "kasfGateMoveToNearest", 0, {
        { "", 0 },
    } },
    { "GateShipsOutNearest", 1, "kasfGateShipsOutNearest", 1, {
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },

    { "MissionSkillSet",    0, "kasfMissionSkillSet", 1, {
        { "skillLevel",     PARAM_SDWORD},
    } },
    { "MissionSkillGet",    1, "kasfMissionSkillGet", 0, {
        { "", 0 },
    } },

    { "RequestShips",       0, "kasfRequestShips", 2, {
        { "shipType",       PARAM_CHARPTR},
        { "numShips",       PARAM_SDWORD},
    } },
    { "RequestShipsOriginal",   0, "kasfRequestShipsOriginal", 1, {
        { "percentOriginal",    PARAM_SDWORD},
    } },

    { "Reinforce",          0, "kasfReinforce", 1, {
        { "team",           PARAM_AITEAMPTR},
    } },

    { "ReinforceTeamWithShips",          0, "kasfReinforceTeamWithShips", 2, {
        { "teamtoreinforce",PARAM_AITEAMPTR},
        { "shipstoadd",    PARAM_GROWSELECTIONPTR},
    } },

    { "ForceCombatStatus", 0, "kasfForceCombatStatus", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR},
        { "on",             PARAM_SDWORD},
    } },

    { "TeamGiveToAI",        0, "kasfTeamGiveToAI", 0, {
        { "", 0 },
    } },

    { "DisableAIFeature",     0, "kasfDisableAIFeature", 2, {
        { "feature", PARAM_SDWORD },
        { "type",    PARAM_SDWORD },
    } },

    { "EnableAIFeature",      0, "kasfEnableAIFeature", 2, {
        { "feature", PARAM_SDWORD },
        { "type",    PARAM_SDWORD },
    } },

    { "DisableAllAIFeatures", 0, "kasfDisableAllAIFeatures", 0, {
        { "", 0 },
    } },

    { "EnableAllAIFeatures",  0, "kasfEnableAllAIFeatures", 0, {
        { "", 0 },
    } },

#if 0
    { "TeamGiveToPlayer",    0, "kasfTeamGiveToPlayer", 0, {
        { "", 0 },
    } },
#endif
    { "TeamSwitchPlayerOwner",    0, "kasfTeamSwitchPlayerOwner", 0, {
        { "", 0 },
    } },

    { "ShipsSwitchPlayerOwner",  0, "kasfShipsSwitchPlayerOwner", 1, {
        { "ships",          PARAM_GROWSELECTIONPTR},
    } },

    { "TacticsAggressive",   0, "kasfTacticsAggressive", 0, {
        { "", 0 },
    } },
    { "TacticsNeutral",      0, "kasfTacticsNeutral", 0, {
        { "", 0 },
    } },
    { "TacticsEvasive",      0, "kasfTacticsEvasive", 0, {
        { "", 0 },
    } },

    { "Nearby",             1, "kasfNearby", 2, {
        { "location",       PARAM_VECTORPTR},
        { "distance",       PARAM_SDWORD},
    } },

    { "FindDistance",       1, "kasfFindDistance", 2, {
        { "location1",      PARAM_VECTORPTR},
        { "location2",      PARAM_VECTORPTR},
    } },

    { "UnderAttack",        1, "kasfUnderAttack", 1, {
        { "attackers",      PARAM_GROWSELECTIONPTR},
    } },
    { "UnderAttackElsewhere", 1, "kasfUnderAttackElsewhere", 2, {
        { "otherTeam",      PARAM_AITEAMPTR},
        { "attackers",      PARAM_GROWSELECTIONPTR},
    } },

    { "Dock",               0, "kasfDock", 1, {     // later remove, replaced with DockSupport and DockSupportWith
        { "withTeam",       PARAM_AITEAMPTR},
    } },
    { "DockSupport",        0, "kasfDockSupport", 0, {
        { "", 0 },
    } },
    { "DockSupportWith",    0, "kasfDockSupportWith", 1, {
        { "withTeam",       PARAM_AITEAMPTR},
    } },
    { "ShipsDockSupportWith",    0, "kasfShipsDockSupportWith", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR},
        { "withShips",       PARAM_GROWSELECTIONPTR},
    } },
    { "DockStay",           0, "kasfDockStay", 1, {
        { "withTeam",       PARAM_AITEAMPTR},
    } },
    { "ShipsDockStay",    0, "kasfShipsDockStay", 2, {
        { "ships",           PARAM_GROWSELECTIONPTR},
        { "withShips",       PARAM_GROWSELECTIONPTR},
    } },
    { "DockInstant",        0, "kasfDockInstant", 1, {
        { "withTeam",       PARAM_AITEAMPTR},
    } },
    { "DockStayMothership", 0, "kasfDockStayMothership", 0, {
        { "", 0 },
    } },
    { "Launch",             0, "kasfLaunch", 0, {
        { "", 0 },
    } },
    { "TeamDocking",        1, "kasfTeamDocking", 0, {
        { "", 0 },
    } },
    { "TeamDockedReadyForLaunch", 1, "kasfTeamDockedReadyForLaunch", 0, {
        { "", 0 },
    } },

    { "TeamFinishedLaunching", 1, "kasfTeamFinishedLaunching", 0, {
        { "", 0 },
    } },

    { "RUsEnemyCollected",  1, "kasfRUsEnemyCollected", 0, {
        { "", 0 },
    } },

    { "RaceOfHuman",        1, "kasfRaceOfHuman", 0, {
        { "", 0 },
    } },

    { "NISRunning",         1, "kasfNISRunning", 0, {
        { "", 0 },
    } },

    { "ObjectiveCreateSecondary", 0, "kasfObjectiveCreateSecondary", 3, {
      { "label",            PARAM_CHARPTR},
      { "briefText",        PARAM_CHARPTR},
      { "fullText",         PARAM_CHARPTR},
    } },
    { "ObjectiveCreate",    0, "kasfObjectiveCreate", 3, {
        { "label",          PARAM_CHARPTR},
        { "briefText",        PARAM_CHARPTR},
        { "fullText",         PARAM_CHARPTR},
    } },
    { "ObjectiveSet",       0, "kasfObjectiveSet", 2, {
        { "label",          PARAM_CHARPTR},
        { "status",         PARAM_SDWORD},
    } },
    { "ObjectiveGet",       1, "kasfObjectiveGet", 1, {
        { "label",          PARAM_CHARPTR},
    } },
    { "ObjectivesGetAll",    1, "kasfObjectiveGetAll", 0, {
        { "", 0 },
    } },
    { "ObjectiveDestroy",   0, "kasfObjectiveDestroy", 1, {
        { "label",          PARAM_CHARPTR},
    } },
    { "ObjectivesDestroyAll",0, "kasfObjectiveDestroyAll", 0, {
        { "", 0 },
    } },

    { "SaveLevel",         0, "kasfSaveLevel", 2, {
        { "LevelNumber",      PARAM_SDWORD},
        { "LevelName",        PARAM_CHARPTR},
    } },

    { "PingAddSingleShip", 0, "kasfPingAddSingleShip", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "label",         PARAM_CHARPTR},
    } },
    { "PingAddShips",      0, "kasfPingAddShips", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "label",         PARAM_CHARPTR},
    } },
    { "PingAddPoint",      0, "kasfPingAddPoint", 2, {
        { "vector",        PARAM_VECTORPTR},
        { "label",         PARAM_CHARPTR},
    } },
    { "PingRemove",        0, "kasfPingRemove", 1, {
        { "label",         PARAM_CHARPTR},
    } },

    { "BuildControl",      0, "kasfBuildControl", 1, {
        { "on",            PARAM_SDWORD},
    } },

    { "BuildingTeam",      0, "kasfBuildingTeam", 1, {
        { "Builder",       PARAM_AITEAMPTR},
    } },

    { "Kamikaze",          0, "kasfKamikaze", 1, {
        { "targets",       PARAM_GROWSELECTIONPTR},
    } },

    { "KamikazeEveryone",  0, "kasfKamikazeEveryone", 1, {
        { "targets",       PARAM_GROWSELECTIONPTR},
    } },

    { "SpecialToggle",     0, "kasfSpecialToggle", 0, {
        { "", 0 },
    } },

    { "ShipsDamage",       0, "kasfShipsDamage", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR},
        { "points",        PARAM_SDWORD},
    } },

    { "ShipsOrder",        1, "kasfShipsOrder", 1, {
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },

    { "ShipsOrderAttributes", 1, "kasfShipsOrderAttributes", 1, {
        { "ships",         PARAM_GROWSELECTIONPTR},
    } },

    { "ForceTaskbar",     0, "kasfForceTaskbar", 0, {
        { "", 0 },
    } },


// Functions added for the tutorial -------------
    { "BuilderRestrictShipTypes", 0, "kasfBuilderRestrictShipTypes", 1, {
        { "shipTypes",      PARAM_CHARPTR },
    } },

    { "BuilderUnrestrictShipTypes", 0, "kasfBuilderUnrestrictShipTypes", 1, {
        { "shipTypes",      PARAM_CHARPTR },
    } },

    { "BuilderRestrictAll", 0, "kasfBuilderRestrictAll", 0, {
        { "", 0 },
    } },

    { "BuilderRestrictNone", 0, "kasfBuilderRestrictNone", 0, {
        { "", 0 },
    } },

    { "BuilderCloseIfOpen", 0, "kasfBuilderCloseIfOpen", 0, {
        { "", 0 },
    } },

    { "ForceBuildShipType", 0, "kasfForceBuildShipType", 1, {
        { "shipType", PARAM_CHARPTR },
    } },

    { "CameraGetAngleDeg", 1, "kasfCameraGetAngleDeg", 0, {
        { "", 0 },
    } },

    { "CameraGetDeclinationDeg", 1, "kasfCameraGetDeclinationDeg", 0, {
        { "", 0 },
    } },

    { "CameraGetDistance", 1, "kasfCameraGetDistance", 0, {
        { "", 0 },
    } },

    { "SelectNumSelected", 1, "kasfSelectNumSelected", 0, {
        { "", 0 },
    } },

    { "SelectIsSelectionShipType", 1, "kasfSelectIsSelectionShipType", 2, {
        { "index",         PARAM_SDWORD },
        { "shipType",      PARAM_CHARPTR },
    } },

    { "SelectContainsShipTypes", 1, "kasfSelectContainsShipTypes", 1, {
        { "shipTypes",     PARAM_CHARPTR },
    } },

    { "SelectedShipsInFormation", 1, "kasfSelectedShipsInFormation", 1, {
        { "formation", PARAM_SDWORD },
    } },
    { "ShipsInFormation", 1, "kasfShipsInFormation", 2, {
        { "ships",     PARAM_GROWSELECTIONPTR },
        { "formation", PARAM_SDWORD },
    } },
    { "TutSetPointerTargetXY", 0, "kasfTutSetPointerTargetXY", 3, {
        { "name",          PARAM_CHARPTR },
        { "pointerX",      PARAM_SDWORD },
        { "pointerY",      PARAM_SDWORD },
    } },

    { "TutSetPointerTargetXYRight", 0, "kasfTutSetPointerTargetXYRight", 3, {
        { "name",          PARAM_CHARPTR },
        { "pointerX",      PARAM_SDWORD },
        { "pointerY",      PARAM_SDWORD },
    } },

    { "TutSetPointerTargetXYBottomRight", 0, "kasfTutSetPointerTargetXYBottomRight", 3, {
        { "name",          PARAM_CHARPTR },
        { "pointerX",      PARAM_SDWORD },
        { "pointerY",      PARAM_SDWORD },
    } },

    { "TutSetPointerTargetXYTaskbar", 0, "kasfTutSetPointerTargetXYTaskbar", 3, {
        { "name",          PARAM_CHARPTR },
        { "pointerX",      PARAM_SDWORD },
        { "pointerY",      PARAM_SDWORD },
    } },

    { "TutSetPointerTargetXYFE", 0, "kasfTutSetPointerTargetXYFE", 3, {
        { "name",          PARAM_CHARPTR },
        { "pointerX",      PARAM_SDWORD },
        { "pointerY",      PARAM_SDWORD },
    } },

    { "TutSetPointerTargetShip", 0, "kasfTutSetPointerTargetShip", 2, {
        { "name",          PARAM_CHARPTR },
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutSetPointerTargetShipSelection", 0, "kasfTutSetPointerTargetShipSelection", 2, {
        { "name",          PARAM_CHARPTR },
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutSetPointerTargetShipHealth", 0, "kasfTutSetPointerTargetShipHealth", 2, {
        { "name",          PARAM_CHARPTR },
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutSetPointerTargetShipGroup", 0, "kasfTutSetPointerTargetShipGroup", 2, {
        { "name",          PARAM_CHARPTR },
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutSetPointerTargetFERegion", 0, "kasfTutSetPointerTargetFERegion", 2, {
        { "name",          PARAM_CHARPTR },
        { "pAtomName",     PARAM_CHARPTR },
    } },

    { "TutSetPointerTargetRect", 0, "kasfTutSetPointerTargetRect", 5, {
        { "name",          PARAM_CHARPTR },
        { "x0",            PARAM_SDWORD },
        { "y0",            PARAM_SDWORD },
        { "x1",            PARAM_SDWORD },
        { "y1",            PARAM_SDWORD },
    } },

    { "TutSetPointerTargetAIVolume", 0, "kasfTutSetPointerTargetAIVolume", 2, {
        { "name",          PARAM_CHARPTR },
        { "volume",        PARAM_VOLUMEPTR },
    } },

    { "TutRemovePointer",   0, "kasfTutRemovePointer", 1, {
        { "name",          PARAM_CHARPTR },
    } },

    { "TutRemoveAllPointers",   0, "kasfTutRemoveAllPointers", 0, {
        { "",          0 },
    } },

    { "TutSetTextDisplayBoxGame", 0, "kasfTutSetTextDisplayBoxGame", 4, {
        { "posX",          PARAM_SDWORD },
        { "posY",          PARAM_SDWORD },
        { "width",         PARAM_SDWORD },
        { "height",        PARAM_SDWORD },
    } },

    { "TutSetTextDisplayBoxToSubtitleRegion", 0, "kasfTutSetTextDisplayBoxToSubtitleRegion", 0, {
        { "", 0 },
    } },

    { "TutSetTextDisplayBoxFE", 0, "kasfTutSetTextDisplayBoxFE", 4, {
        { "posX",          PARAM_SDWORD },
        { "posY",          PARAM_SDWORD },
        { "width",         PARAM_SDWORD },
        { "height",        PARAM_SDWORD },
    } },

    { "TutShowText",       0, "kasfTutShowText", 1, {
        { "text",          PARAM_CHARPTR },
    } },

    { "TutHideText",       0, "kasfTutHideText", 0, {
        { "", 0 },
    } },

    { "TutShowNextButton", 0, "kasfTutShowNextButton", 0, {
        { "", 0 },
    } },

    { "TutHideNextButton", 0, "kasfTutHideNextButton", 0, {
        { "", 0 },
    } },

    { "TutNextButtonClicked", 1, "kasfTutNextButtonClicked", 0, {
        { "", 0 },
    } },

    { "TutShowBackButton", 0, "kasfTutShowBackButton", 0, {
        { "", 0 },
    } },

    { "TutHideBackButton", 0, "kasfTutHideBackButton", 0, {
        { "", 0 },
    } },

    { "TutShowPrevButton", 0, "kasfTutShowPrevButton", 0, {
        { "", 0 },
    } },

    { "TutSaveLesson",     0, "kasfTutSaveLesson", 2, {
        { "LessonNumber",  PARAM_SDWORD },
        { "LessonName",    PARAM_CHARPTR },
    } },

    { "TutShowImages",     0, "kasfTutShowImages", 1, {
        { "imageList",     PARAM_CHARPTR },
    } },

    { "TutHideImages",     0, "kasfTutHideImages", 0, {
        { "", 0 },
    } },

    { "TutEnableEverything", 0, "kasfTutEnableEverything", 0, {
        { "", 0 },
    } },

    { "TutDisableEverything", 0, "kasfTutDisableEverything", 0, {
        { "", 0 },
    } },

    { "TutEnableFlags",    0, "kasfTutEnableFlags", 1, {
        { "flagsList",     PARAM_CHARPTR },
    } },

    { "TutDisableFlags",   0, "kasfTutDisableFlags", 1, {
        { "flagsList",     PARAM_CHARPTR },
    } },

    { "TutForceUnpaused", 0, "kasfTutForceUnpaused", 0, {
        { "", 0 },
    } },

    { "TutGameSentMessage",  1, "kasfTutGameSentMessage", 1, {
        { "commandNames",   PARAM_CHARPTR },
    } },

    { "TutResetGameMessageQueue", 0, "kasfTutResetGameMessageQueue", 0, {
        { "", 0 },
    } },

    { "TutContextMenuDisplayedForShipType",  1, "kasfTutContextMenuDisplayedForShipType", 1, {
        { "shipType",   PARAM_CHARPTR },
    } },

    { "TutResetContextMenuShipTypeTest", 0, "kasfTutResetContextMenuShipTypeTest", 0, {
        { "", 0 },
    } },

    { "TutRedrawEverything", 0, "kasfTutRedrawEverything", 0, {
        { "", 0 },
    } },

    { "BuildManagerShipTypeInBatchQueue",  1, "kasfBuildManagerShipTypeInBatchQueue", 1, {
        { "shipType",   PARAM_CHARPTR },
    } },

    { "BuildManagerShipTypeInBuildQueue",  1, "kasfBuildManagerShipTypeInBuildQueue", 1, {
        { "shipType",   PARAM_CHARPTR },
    } },

    { "BuildManagerShipTypeSelected",  1, "kasfBuildManagerShipTypeSelected", 1, {
        { "shipType",   PARAM_CHARPTR },
    } },

    { "TutCameraFocus",    0, "kasfTutCameraFocus", 1, {
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutCameraFocusDerelictType",    0, "kasfTutCameraFocusDerelictType", 1, {
        { "derelictType",     PARAM_CHARPTR },
    } },

    { "TutCameraFocusFar",    0, "kasfTutCameraFocusFar", 1, {
        { "pShipList",     PARAM_GROWSELECTIONPTR },
    } },

    { "TutCameraFocusCancel", 0, "kasfTutCameraFocusCancel", 0, {
        { "", 0 },
    } },

    { "TutCameraFocusedOnShipType", 1, "kasfTutCameraFocusedOnShipType", 1, {
        { "shipType", PARAM_CHARPTR },
    } },

    { "DisablePlayer", 0, "kasfDisablePlayer", 1, {
        { "toggle", PARAM_BOOL },
    } },


    { "TutShipsInView", 1, "kasfTutShipsInView", 1, {
        { "ships", PARAM_GROWSELECTIONPTR},
    } },

    { "TutShipsTactics", 1, "kasfTutShipsTactics", 1, {
        { "ships", PARAM_GROWSELECTIONPTR},
    } },

    { "FindSelectedShips", 1, "kasfFindSelectedShips", 1, {
    { "ships",         PARAM_GROWSELECTIONPTR},
    } },

    { "TutPieDistance", 1, "kasfTutPieDistance", 0, {
        { "", 0},
    } },

    { "TutPieHeight", 1, "kasfTutPieHeight", 0, {
        { "", 0},
    } },

    { "ForceFISensors", 0, "kasfForceFISensors", 0, {
        { "", 0},
    } },

    { "OpenSensors", 0, "kasfOpenSensors", 1, {
        { "flags", PARAM_SDWORD },
    } },

    { "CloseSensors", 0, "kasfCloseSensors", 1, {
        { "flags", PARAM_SDWORD },
    } },

    { "SensorsIsOpen", 1, "kasfSensorsIsOpen", 1, {
        { "flags", PARAM_SDWORD },
    } },

    { "SensorsWeirdness", 0, "kasfSensorsWeirdness", 1, {
        { "flags", PARAM_SDWORD },
    } },

// ----------------------------------------------

    { "TechSetResearch",    0, "kasfAllowPlayerToResearch", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechSetPurchase",    0, "kasfAllowPlayerToPurchase", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechSet",    0, "kasfPlayerAcquiredTechnology", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechGetResearch",    1, "kasfCanPlayerResearch", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechGetPurchase",    1, "kasfCanPlayerPurchase", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechGet",    1, "kasfDoesPlayerHave", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechSetCost",        0, "kasfSetBaseTechnologyCost", 2, {
        { "techName",     PARAM_CHARPTR },
        { "cost",         PARAM_SDWORD },
    } },
    { "TechGetCost",        1, "kasfGetBaseTechnologyCost", 1, {
        { "techName",     PARAM_CHARPTR },
    } },
    { "TechIsResearching",    1, "kasfTechIsResearching", 0, {
        { "", 0 },
    } },
    { "TraderGUIDisplay",    0, "kasfEnableTraderGUI", 0, {
        { "", 0 },
    } },
    { "TraderGUIIsDisplayed",    1, "kasfTraderGUIActive", 0, {
        { "", 0 },
    } },
    { "TraderGUIDialogSet",    0, "kasfSetTraderDialog", 2, {
        { "dialogNum",      PARAM_SDWORD },
        { "dialogText",     PARAM_CHARPTR },
    } },
    { "TraderGUIPriceScaleSet",    0, "kasfSetTraderPriceScale", 1, {
        { "scalePercent",     PARAM_SDWORD },
    } },
    { "TraderGUIPriceScaleGet",    1, "kasfGetTraderPriceScale", 0, {
        { "", 0 },
    } },
    { "TraderGUIDisable",    0, "kasfSetTraderDisabled", 1, {
        { "disable",     PARAM_SDWORD },
    } },

    { "RUsGet",    1, "kasfRUsGet", 1, {
        { "player",         PARAM_SDWORD },
    } },
    { "RUsSet",    0, "kasfRUsSet", 2, {
        { "player",         PARAM_SDWORD },
        { "RUs",            PARAM_SDWORD },
    } },
    { "GetWorldResources",    1, "kasfGetWorldResources", 0, {
        { "", 0 },
    } },

    { "SoundEvent",     0, "kasfSoundEvent", 1, {
        { "event",         PARAM_SDWORD },
    } },
    { "SoundEventShips",   0, "kasfSoundEventShips", 2, {
        { "ships",         PARAM_GROWSELECTIONPTR },
        { "event",         PARAM_SDWORD },
    } },

    { "SpeechEvent",       0, "kasfSpeechEvent", 2, {
        { "event",         PARAM_SDWORD },
        { "variable",      PARAM_SDWORD },
    } },
    { "SpeechEventShips",  0, "kasfSpeechEventShips", 3, {
        { "ships",         PARAM_GROWSELECTIONPTR },
        { "event",         PARAM_SDWORD },
        { "variable",      PARAM_SDWORD },
    } },
    { "ToggleActor",       0, "kasfToggleActor", 2, {
        { "Actor",         PARAM_SDWORD },
        { "on",      PARAM_SDWORD },
    } },
    { "MusicPlay",          0, "kasfMusicPlay", 1, {
        { "trackNum",       PARAM_SDWORD },
    } },
    { "MusicStop",          0, "kasfMusicStop", 1, {
        { "fadeTime",       PARAM_SDWORD },
    } },

    { "HyperspaceDelay",    0, "kasfHyperspaceDelay", 1, {
        { "milliseconds",      PARAM_SDWORD },
    } },

    { "RenderedShips",      1, "kasfRenderedShips", 2, {
        { "ships",          PARAM_GROWSELECTIONPTR },
        { "LOD",            PARAM_SDWORD },
    } },

    { "ResetShipRenderFlags",  0, "kasfResetShipRenderFlags", 1, {
        { "ships",          PARAM_GROWSELECTIONPTR },
    } },

    { "RenderedDerelictType", 1, "kasfRenderedDerelicts", 2, {
        { "derelicttype",       PARAM_CHARPTR },
        { "LOD",                PARAM_SDWORD },
    } },

    { "ResetDerelictRenderFlags",  0, "kasfResetDerelictRenderFlags", 1, {
        { "derelicttype",      PARAM_CHARPTR },
    } },

    { "ClearScreen",       0, "kasfClearScreen", 0, {
        { "", 0 },
    } },

    { "wideScreenIn",       0, "kasfWideScreenIn", 1, {
        { "frames",         PARAM_SDWORD },
    } },

    { "wideScreenOut",      0, "kasfWideScreenOut", 1, {
        { "frames",         PARAM_SDWORD },
    } },

    { "SubtitleSimulate",   0, "kasfSubtitleSimulate", 3, {
        { "actor",          PARAM_SDWORD },
        { "milliseconds",   PARAM_SDWORD },
        { "text",           PARAM_CHARPTR },
    } },

    { "LocationCard",       0, "kasfLocationCard", 2, {
        { "milliseconds",   PARAM_SDWORD },
        { "location",       PARAM_CHARPTR },
    } },

    { "HideShips",          0, "kasfHideShips", 1, {
        { "shipstohide",    PARAM_GROWSELECTIONPTR },
    } },

    { "UnhideShips",        0, "kasfUnhideShips", 1, {
        { "shipstounhide",  PARAM_GROWSELECTIONPTR },
    } },

    { "DeleteShips",        0, "kasfDeleteShips", 1, {
        { "shipstodelete",  PARAM_GROWSELECTIONPTR },
    } },

    { "RotateDerelictType", 0, "kasfRotateDerelictType", 5, {
        { "derelictType",  PARAM_CHARPTR },
        { "rot_x",         PARAM_SDWORD },
        { "rot_y",         PARAM_SDWORD },
        { "rot_z",         PARAM_SDWORD },
        { "variation",     PARAM_SDWORD },
    } },

    { "PauseUniverse",      0, "kasfUniversePause", 0, {
        { "", 0 },
    } },

    { "UnpauseUniverse",    0, "kasfUniverseUnpause", 0, {
        { "", 0 },
    } },

    { "PauseShipBuilding",  0, "kasfPauseBuildShips", 0, {
        { "", 0 },
    } },

    { "UnpauseShipBuilding",0, "kasfUnpauseBuildShips", 0, {
        { "", 0 },
    } },

    { "PauseOtherKAS",      0, "kasfOtherKASPause", 0, {
        { "", 0 },
    } },

    { "UnpauseOtherKAS",    0, "kasfOtherKASUnpause", 0, {
        { "", 0 },
    } },

    { "IntelEventEnded",    1, "kasfIntelEventEnded", 0, {
        { "", 0 },
    } },

    { "IntelEventNotEnded", 0, "kasfIntelEventNotEnded", 0, {
        { "", 0 },
    } },

    { "ForceIntelEventEnded", 0, "kasfForceIntelEventEnded", 0, {
        { "", 0 },
    } },

    { "Stop",               0, "kasfStop", 0, {
        { "", 0 },
    } },

    { "Harvest",            0, "kasfHarvest", 0, {
        { "", 0 },
    } },

    { "SensorsStaticOn",    0, "kasfSensorsStaticOn", 0, {
        { "", 0 },
    } },

    { "SensorsStaticOff",   0, "kasfSensorsStaticOff", 0, {
        { "", 0 },
    } },

    { "GameEnd",            0, "kasfGameEnd", 0, {
        { "", 0 },
    } },

    { "SpawnEffect", 0, "kasfSpawnEffect", 3, {
        { "ships",          PARAM_GROWSELECTIONPTR },
        { "effect",         PARAM_CHARPTR },
        { "effectParam",    PARAM_SDWORD },
    } },

    { "*", 0, "", 0, { { "", 0 } } } // end-marker
};


//  verify uniqueness of FSM name & add it to the lookup table
void kasFSMAdd(char *name)
{
    int i;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFSMAdd(\"%s\")\n", name);
#endif

    if (strlen(name) > MAX_FSM_NAME_LENGTH)
    {
        fprintf(stderr, "%s(%d) warning : FSM name too long: %s\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
        return;
    }
    for (i = 0; i < fsmCount; ++i)
        if (!strcmp(fsmName[i], name))
        {
            fprintf(stderr, "%s(%d) warning : redefinition of %s FSM\n", curFilenameGet(), lineNumGet(), name);
            ++yynerrs;
            break;
        }
    if (i >= fsmCount)
    {
        strcpy(fsmName[fsmCount], name);
        ++fsmCount;
    }
}

void kasFSMStart(char *name)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFSMStart(\"%s\")\n", name);
#endif

    kasFSMAdd(name);
    fsmCurrent = fsmCount-1;
    parseLevel = LEVEL_FSM;
    sprintf(stateHelp, "expecting state list in %s FSM", name);
}

void kasFSMEnd(char *name)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFSMEnd(\"%s\")\n", name);
#endif

    parseLevel = LEVEL_LEVEL;
    if (name[0] && strcmp(name, fsmName[fsmCurrent]))
    {
        fprintf(stderr, "%s(%d) warning : %s FSM ended with different name: %s\n", curFilenameGet(), lineNumGet(), fsmName[fsmCurrent], name);
        ++yynerrs;
    }
    sprintf(stateHelp, "expecting initialize section for mission after FSM definition(s)");
}

//
//  return FSM index in lookup table if it exists
//  return -1 if not found
//
int kasFSMValid(char *name)
{
    int i = 0;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateValid(\"%s\")\n", name);
#endif

    while (i < fsmCount)
    {
        if (!strcmp(fsmName[i],name))
            return i;
        ++i;
    }
    return -1;
}

void kasFSMCreateStart(char *name)
{
    int fsmNum;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFSMCreateStart(\"%s\")\n", name);
#endif

    if ((fsmNum = kasFSMValid(name)) < 0)
    {
        fprintf(stderr, "%s(%d) warning : undefined FSM: %s\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
        return;
    }
    fprintf(yyout, "kasFSMCreate(\"%s\", Init_%s_%s, Watch_%s_%s, ",
            fsmName[fsmNum],
            levelNameGet(), fsmName[fsmNum],
            levelNameGet(), fsmName[fsmNum]);
}

void kasFSMCreateEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFSMCreateEnd()\n");
#endif

    fprintf(yyout, ");\n\t");
}

//  wipe state list for this FSM
void kasStateListClear(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateListClear()\n");
#endif

    stateCount = 0;
}

//  verify uniqueness of state name (for this FSM) & add it to the lookup table
void kasStateListAdd(char *name)
{
    int i;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateListAdd(\"%s\")\n", name);
#endif

    if (strlen(name) > MAX_STATE_NAME_LENGTH)
    {
        fprintf(stderr, "%s(%d) warning : state name too long: %s\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
        return;
    }
    for (i = 0; i < stateCount; ++i)
        if (!strcmp(stateName[i], name))
        {
            fprintf(stderr, "%s(%d) warning : duplicate state name listed: %s\n", curFilenameGet(), lineNumGet(), name);
            ++yynerrs;
            break;
        }
    if (i >= stateCount)
    {
        strcpy(stateName[stateCount], name);
        ++stateCount;
    }
}

void kasStateListEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateListEnd()\n");
#endif

    sprintf(stateHelp, "expecting initialize section in %s FSM", fsmName[fsmCurrent]);
}

//
//  return state index in lookup table if it exists
//  return -1 if not found
//
int kasStateValid(char *name)
{
    int i = 0;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateValid(\"%s\")\n", name);
#endif

    while (i < stateCount)
    {
        if (!strcmp(stateName[i],name))
            return i;
        ++i;
    }
    return -1;
}

void kasStateStart(char *name)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateStart(\"%s\")\n", name);
#endif

    stateCurrent = kasStateValid(name);
    if (stateCurrent < 0)
    {
        fprintf(stderr, "%s(%d) warning : undeclared state: %s\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
    }
    parseLevel = LEVEL_STATE;
    sprintf(stateHelp, "expecting initialize section in %s state", name);
}

void kasStateEnd(char *name)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasStateEnd(\"%s\")\n", name);
#endif

    parseLevel = LEVEL_FSM;
    if (name[0] && strcmp(name, stateName[stateCurrent]))
    {
        fprintf(stderr, "%s(%d) warning : %s state ended with different name: %s\n", curFilenameGet(), lineNumGet(), stateName[stateCurrent], name);
        ++yynerrs;
    }
    sprintf(stateHelp, "expecting end of %s FSM after state definition(s)", fsmName[fsmCurrent]);
}

//
//  set run-time scope for variables, timers, etc.
//
static void kasScopeSet(void)
{
    switch (parseLevel)
    {
        // local symbols visible at state & fsm levels
        case LEVEL_STATE:
            fprintf(yyout, "CurrentMissionScope = KAS_SCOPE_STATE;\n\t");
            fprintf(yyout, "strcpy(CurrentMissionScopeName, kasThisTeamPtr->kasLabel);\n\t");
            break;
        case LEVEL_FSM:
            fprintf(yyout, "CurrentMissionScope = KAS_SCOPE_FSM;\n\t");
            fprintf(yyout, "strcpy(CurrentMissionScopeName, kasThisTeamPtr->kasLabel);\n\t");
            //fprintf(yyout, "strcpy(CurrentMissionScopeName, \"%s\");\n\t", fsmName[fsmCurrent]);
            break;
        // globals for mission
        case LEVEL_LEVEL:
        default:
            fprintf(yyout, "CurrentMissionScope = KAS_SCOPE_MISSION;\n\t");
            fprintf(yyout, "strcpy(CurrentMissionScopeName, \"%s\");\n\t", levelNameGet());
            break;
    }
}

void kasInitializeStart(void)
{
    char funcName[MAX_FUNC_NAME_LENGTH+1];

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasInitializeStart()\n");
#endif

    fprintf(yyout, "//\n");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            fprintf(yyout, "//  \"initialize\" code for %s mission\n", levelNameGet());
            break;
        case LEVEL_FSM:
            fprintf(yyout, "//  \"initialize\" code for %s %s FSM\n", levelNameGet(), fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            fprintf(yyout, "//  \"initialize\" code for %s %s %s state\n", levelNameGet(), fsmName[fsmCurrent], stateName[stateCurrent]);
            break;
        default:
            // should never happen!
            fprintf(yyout, "//  unknown \"initialize\" code\n");
    }
    fprintf(yyout, "//\n");

    fprintf(yyout, "void ");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            sprintf(funcName, "Init_%s", levelNameGet());
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in initialize section of mission");
            break;
        case LEVEL_FSM:
            sprintf(funcName, "Init_%s_%s", levelNameGet(), fsmName[fsmCurrent]);
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in initialize section of %s FSM", fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            sprintf(funcName, "Init_%s_%s_%s", levelNameGet(), fsmName[fsmCurrent], stateName[stateCurrent]);
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in initialize section of %s state", stateName[stateCurrent]);
            break;
        default:
            sprintf(funcName, "Init_UNKNOWN");
            fprintf(yyout, "%s", funcName);
            fprintf(stderr, "%s(%d) warning : initialize section in wrong place\n", curFilenameGet(), lineNumGet()); // should never happen!
            ++yynerrs;
            sprintf(stateHelp, "in unexpected initialize section");
    }
    fprintf(yyout, "(void)\n{\n\t");
    //fprintf(yyout, "static int ifOnce = 0; // for \"ifonce\" checking\n\t");  //we use AIVars now
    //ifOnceIndex = 0;

    kasScopeSet();

    // prototype
    fprintf(yyhout, "void %s(void);\n", funcName);

    // function pointer
    if (yyfout)
        fprintf(yyfout, "\t(void*)%s,\n", funcName);
    functionCount++;
}

void kasInitializeEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasInitializeEnd()\n");
#endif

    fprintf(yyout, "\n}\n\n\n");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            sprintf(stateHelp, "expecting watch section for mission");
            break;
        case LEVEL_FSM:
            sprintf(stateHelp, "expecting watch section in %s FSM", fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            sprintf(stateHelp, "expecting watch section in %s state", stateName[stateCurrent]);
            break;
        default:
            // shouldn't ever happen!
            sprintf(stateHelp, "expecting watch section in wrong place");
            break;
    }

}

void kasWatchStart(void)
{
    char funcName[MAX_FUNC_NAME_LENGTH+1];

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasWatchStart()\n");
#endif

    fprintf(yyout, "//\n");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            fprintf(yyout, "//  \"watch\" code for %s mission\n", levelNameGet());
            break;
        case LEVEL_FSM:
            fprintf(yyout, "//  \"watch\" code for %s %s FSM\n", levelNameGet(), fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            fprintf(yyout, "//  \"watch\" code for %s %s %s state\n", levelNameGet(), fsmName[fsmCurrent], stateName[stateCurrent]);
            break;
        default:
            // should never happen!
            fprintf(yyout, "//  unknown \"initialize\" code\n");
    }
    fprintf(yyout, "//\n");

    fprintf(yyout, "void ");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            sprintf(funcName, "Watch_%s", levelNameGet());
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in watch section of mission");
            break;
        case LEVEL_FSM:
            sprintf(funcName, "Watch_%s_%s", levelNameGet(), fsmName[fsmCurrent]);
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in watch section of %s FSM", fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            sprintf(funcName, "Watch_%s_%s_%s", levelNameGet(), fsmName[fsmCurrent], stateName[stateCurrent]);
            fprintf(yyout, "%s", funcName);
            sprintf(stateHelp, "in watch section of %s state", stateName[stateCurrent]);
            break;
        default:
            sprintf(funcName, "Watch_UNKNOWN");
            fprintf(yyout, "%s", funcName);
            fprintf(stderr, "%s(%d) warning : watch section in wrong place\n", curFilenameGet(), lineNumGet());  // should never happen!
            ++yynerrs;
            sprintf(stateHelp, "in unexpected watch section");
    }
    fprintf(yyout, "(void)\n{\n\t");
    //fprintf(yyout, "static int ifOnce = 0; // for \"ifonce\" checking\n\t");  //we use AIVars now
    //ifOnceIndex = 0;

    kasScopeSet();

    // prototype
    fprintf(yyhout, "void %s(void);\n", funcName);

    // function pointer
    if (yyfout)
        fprintf(yyfout, "\t(void*)%s,\n", funcName);
    functionCount++;
}

void kasWatchEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasWatchEnd()\n");
#endif

    fprintf(yyout, "\n}\n\n\n");
    switch (parseLevel)
    {
        case LEVEL_LEVEL:
            sprintf(stateHelp, "expecting end of file");
            break;
        case LEVEL_FSM:
            sprintf(stateHelp, "expecting state definitions in %s FSM", fsmName[fsmCurrent]);
            break;
        case LEVEL_STATE:
            sprintf(stateHelp, "expecting end of %s state", stateName[stateCurrent]);
            break;
        default:
            // shouldn't ever happen!
            sprintf(stateHelp, "expecting the unknown in wrong place");
            break;
    }
}

char *stateHelpGet(void)
{
    return stateHelp;
}

void kasLocalizationStart(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLocalizationStart()\n");
#endif

    kasLStringClearAll();
    sprintf(stateHelp, "expecting LSTRING_ definition(s)");
}

void kasLocalizationEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLocalizationEnd()\n");
#endif

    sprintf(stateHelp, "");
}

//  wipe list of definied lstrings
void kasLStringClearAll(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringClearAll()\n");
#endif

    lstringCount = 0;
}

//
//  return lstring name index in lookup table if it exists
//  return -1 if not found
//
int kasLStringValid(char *name)
{
    int i = 0;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringValid(\"%s\")\n", name);
#endif

    while (i < lstringCount)
    {
        if (!strcmp(lstringName[i],name))
            return i;
        ++i;
    }
    return -1;
}

void kasLStringReference(char *name)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringReference(\"%s\")\n", name);
#endif

    if (kasLStringValid(name) != -1)
        fprintf(yyout, "LSTRING_%s[strCurLanguage]", name);
    else
    {
        fprintf(stderr, "%s(%d) warning : LSTRING_%s not defined\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
    }
}
//
//  validate lstring name and start definition
//
void kasLStringDefineStart(char *name)
{
    int i;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringDefineStart(\"%s\")\n", name);
#endif

    lstringValueCount = 0;
    if (strlen(name) > MAX_LSTRING_NAME_LENGTH)
    {
        fprintf(stderr, "%s(%d) warning : name too long: LSTRING_%s\n", curFilenameGet(), lineNumGet(), name);
        ++yynerrs;
        return;
    }
    for (i = 0; i < lstringCount; ++i)
        if (!strcmp(lstringName[i], name))
        {
            fprintf(stderr, "%s(%d) warning : duplicate definition: LSTRING_%s\n", curFilenameGet(), lineNumGet(), name);
            ++yynerrs;
            return;
        }
    if (i >= lstringCount)
    {
        strcpy(lstringName[lstringCount], name);
        ++lstringCount;
    }

    fprintf(yyout, "static char *LSTRING_%s[] = {  // multilingual strings", name);
}

//
//  complete the definition of an lstring
//
void kasLStringDefineEnd(void)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringDefineEnd()\n");
#endif

    fprintf(yyout, " };\n\n");

    if (lstringValueCount != LSTRING_COUNT_REQUIRED)
    {
        fprintf(stderr, "%s(%d) warning : %d values defined for LSTRING_%s; %d values required\n", curFilenameGet(), lineNumGet(),
                lstringValueCount,
                lstringName[lstringCount-1],
                LSTRING_COUNT_REQUIRED);
        ++yynerrs;
        return;
    }
}

//
//  one value of a multilingual string definition
//
void kasLStringValue(char *value)
{
#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasLStringValue(\"%s\")\n", value);
#endif

    fprintf(yyout, "\n\t\"%s\",", value);
    ++lstringValueCount;
}

//
//  validate function and start call stack info
//
void kasFunctionStart(char *funcName)
{
    int funcNum;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionStart(\"%s\")\n", funcName);
#endif

    funcNum = kasFunction(funcName);
    if (funcNum < 0)
    {
        fprintf(stderr, "%s(%d) warning : unknown function: %s\n", curFilenameGet(), lineNumGet(), funcName);
        ++yynerrs;
        return;
    }

    if (++curFuncDepth >= MAX_FUNC_DEPTH)
    {
        fprintf(stderr, "%s(%d) warning : %s function nested too deep\n", curFilenameGet(), lineNumGet(), funcName);
        ++yynerrs;
        return;
    }

    // init call info so far
    callStack[curFuncDepth].functionNum = funcNum;
    callStack[curFuncDepth].paramsSoFar = 0;

    if (functions[funcNum].returnsNumber)
        fprintf(yyout, "(sdword)");   // to avoid type-casting warnings during later compilation
    else
        fprintf(yyout, "(sdword)(");  // start the (xxx, 0) expression

    fprintf(yyout, "%s(", functions[funcNum].realName);

    sprintf(stateHelp, "expecting parameter list for %s", funcName);
}

void kasFunctionEnd(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    FunctionCall *fcallp;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionEnd()\n");
#endif

    if (curFuncDepth < 0)
    {
        // should never happen!
        fprintf(stderr, "%s(%d) warning : invalid function call\n", curFilenameGet(), lineNumGet());
        ++yynerrs;
        return;
    }

    if (callStack[curFuncDepth].paramsSoFar < functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : not enough parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }

    fprintf(yyout, ")");

    // if function doesn't return a number, have it "return" 0, for expression compatibility
    if (!functions[funcNum].returnsNumber)
        fprintf(yyout, ", 0)");

    --curFuncDepth;

    sprintf(stateHelp, "");
}


void kasJump(char *newStateName)
{
    int stateNum;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasJump(\"%s\")\n", newStateName);
#endif

    if (parseLevel != LEVEL_FSM && parseLevel != LEVEL_STATE)
    {
        // can't jump from top level
        fprintf(stderr, "%s(%d) warning : jumps only permitted from within FSMs and states\n", curFilenameGet(), lineNumGet());
        ++yynerrs;
        return;
    }

    stateNum = kasStateValid(newStateName);
    if (stateNum >= 0)
    {
        fprintf(yyout, "kasJump(\"%s\", Init_%s_%s_%s, Watch_%s_%s_%s)",
                stateName[stateNum],
                levelNameGet(), fsmName[fsmCurrent], stateName[stateNum],
                levelNameGet(), fsmName[fsmCurrent], stateName[stateNum]);
    }
    else
    {
        fprintf(stderr, "%s(%d) warning : undeclared state: %s\n", curFilenameGet(), lineNumGet(), newStateName);
        ++yynerrs;
    }
}

//  validate next param type
void kasFunctionParamNumber(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamNumber()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_SBYTE &&
            type != PARAM_UBYTE &&
            type != PARAM_SWORD &&
            type != PARAM_UWORD &&
            type != PARAM_SDWORD &&
            type != PARAM_UDWORD &&
            type != PARAM_REAL32 &&
            type != PARAM_REAL64 &&
            type != PARAM_BOOL &&
            type != PARAM_BOOL8 &&
            type != PARAM_BOOL16)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (number passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name);
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamCharPtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamCharPtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_CHARPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_CHARPTR));
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamAITeamPtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamAITeamPtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_AITEAMPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_AITEAMPTR));
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamSelectCommandPtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamSelectCommandPtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_GROWSELECTIONPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_GROWSELECTIONPTR));
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamPathPtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamPathPtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_PATHPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_PATHPTR));
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamVectorPtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamVectorPtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_VECTORPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_VECTORPTR));
            ++yynerrs;
        }
    }
}

//  validate next param type
void kasFunctionParamVolumePtr(void)
{
    int funcNum = callStack[curFuncDepth].functionNum;
    int type;

#ifdef LOTS_OF_DEBUGS
    fprintf(stderr, "-=*> kasFunctionParamVolumePtr()\n");
#endif

    if (curFuncDepth < 0)
        return;

    if (++(callStack[curFuncDepth].paramsSoFar) > functions[funcNum].numParams)
    {
        fprintf(stderr, "%s(%d) warning : too many parameters passed to %s function\n", curFilenameGet(), lineNumGet(), functions[funcNum].name);
        ++yynerrs;
    }
    else
    {
        type = functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].type;
        if (type != PARAM_VOLUMEPTR)
        {
            fprintf(stderr, "%s(%d) warning : expected %s for %s parameter of %s function (%s passed instead)\n", curFilenameGet(), lineNumGet(), kasParamTypeToString(type), functions[funcNum].params[callStack[curFuncDepth].paramsSoFar-1].name, functions[funcNum].name, kasParamTypeToString(PARAM_VOLUMEPTR));
            ++yynerrs;
        }
    }
}

// friendly, english type strings
char *kasParamTypeToString(int type)
{
    static char *s[] = {
        "integer",
        "integer",
        "integer",
        "integer",
        "integer",
        "integer",
        "integer",
        "integer",
        "boolean",
        "boolean",
        "boolean",
        "ship",
        "ships",
        "team",
        "string",
        "path",
        "vector",
        "volume",
    };

    return s[type];
}

// friendly, english type strings
char *kasParamTypeToC(int type)
{
    static char *s[] = {
       "sbyte",
       "ubyte",
       "sword",
       "uword",
       "sdword",
       "udword",
       "real32",
       "real64",
       "bool",
       "bool8",
       "bool16",
       "Ship *",
       "SelectCommand *",
       "AITeam *",
       "char *",
       "Path *",
       "Vector *",
       "Volume *",
    };

    return s[type];
}

//
//  display headers of all available function
//  0 = english
//  1 = C-style
//
void kasHeaders(int ctype)
{
    int i = 0;
    int p;

    fprintf(stdout, "\nKAS2C Version %s\n", KAS2C_VERSION);
    fprintf(stdout, "Copyright (C) 1998 Relic Entertainment Inc.  All rights reserved.\n");
    fprintf(stdout, "\nFunction Summary:\n\n");

    while (functions[i].name[0] != '*')
    {
        if (ctype)
        {
            fprintf(stdout, "%s %s(",
                functions[i].returnsNumber ? "sdword" : "void",
                functions[i].realName);
            p = 0;
            while (p < functions[i].numParams)
            {
                fprintf(stdout, "%s %s%s",
                        kasParamTypeToC(functions[i].params[p].type),
                        functions[i].params[p].name,
                        p == functions[i].numParams-1 ? "" : ", ");
                ++p;
            }
            fprintf(stdout, ");\n");
        }
        else
        {
            fprintf(stdout, "%s (", functions[i].name);
            p = 0;
            while (p < functions[i].numParams)
            {
                fprintf(stdout, "%s : %s%s",
                        functions[i].params[p].name,
                        kasParamTypeToString(functions[i].params[p].type),
                        p == functions[i].numParams-1 ? "" : ", ");
                ++p;
            }
            if (functions[i].returnsNumber)
                fprintf(stdout, ") : %s;\n", kasParamTypeToString(PARAM_SDWORD));
            else
                fprintf(stdout, ");\n");
        }
        ++i;
    }
}


//
//  returns function number or -1 if it isn't available
//
int kasFunction(char *name)
{
    int i = 0;

    while (1)
    {
        if (functions[i].name[0] == '*')
            return -1;
        else if (!strcasecmp(name, functions[i].name))
            return i;
        ++i;
    }
}
