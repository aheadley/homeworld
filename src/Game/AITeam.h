/*=============================================================================
    Name    : AITeam.h
    Purpose : Definitions for AITeam.c

    Created 5/31/1998 by dstone
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef __AITEAM_H
#define __AITEAM_H

#include "Types.h"
#include "SpaceObj.h"
#include "ShipSelect.h"
#include "AIEvents.h"
#include "Formation.h"
#include "FormationDefs.h"
#include "AIVar.h"
#include "Vector.h"
#include "AIUtilities.h"
#include "KAS.h"
#include "Volume.h"

//team flags
// Mask 0xff00000000 to Special Team Flags
#define AIT_SpecialTeamMask     0xff000000
#define FAST_ROVING_GUARD       0x00000001
#define SLOW_ROVING_GUARD       0x00000002
#define HARASS_TEAM             0x00000004
#define RESOURCECOLLECT_TEAM    0x00000008

#define AIT_DestroyTeam         0x00000010      //flag set to destroy the team in a safe place
#define AIT_Reissue01			0x00000020		//flag set to keep certain orders from being reissued over and over again
#define AIT_Reissue10			0x00000040		//flag set to keep certain orders from being reissued over and over again

#define TEAM_HOMOGENEOUS        0x00000100
#define TEAM_HETEROGENEOUS      0x00000200

#define TEAM_NEEDS_SUPPORT      0x00001000      //team needs a support ship
#define TEAM_GETTING_SUPPORT    0x00002000      //a support ship is near the team
#define TEAM_DOCKING            0x00004000
#define TEAM_RETREATING         0x00008000
#define TEAM_ARMADA             0x00010000
#define TEAM_Hyperspaceable     0x00020000      //the team can hyperspace (no fighters guarding or anything)
#define TEAM_TempGuard			0x00040000		//the team is an armada team temp guarding

#define TEAM_SpecialOptimal     0x00100000      //denotes whether the special team has setup with optimal targets/partners
#define TEAM_AdvancedMoveAttack 0x00200000      //the MoveAttack move will pick out targets individually
#define TEAM_SwarmTarget        0x00400000      //set when a swarm team should get it's target from the Targets structure
#define TEAM_AmIBeingWatched    0x00800000      //set when a swarm team is checking if it's being watched

#define TEAM_CloakFighters      0x01000000
#define TEAM_CloakedFighters    0x02000000
#define TEAM_CloakGenerator     0x04000000
#define TEAM_CloakedGenerator   0x08000000

//#define TEAM_GravWellGenerator  0x10000000
//#define TEAM_GravWellGenerating 0x20000000
#define TEAM_CloakCoop          0x10000000      //is cooperating with a cloak generator team

//swarmer stuff
#define MAX_NEW_SWARMERS    15

#define MAX_SPLIT           4

typedef enum {
    MOVE_DONE,                   //0
    MOVE_GUARDSHIPS,             //1
    MOVE_DEFMOSHIP,              //2
    MOVE_TEMPGUARD,              //3
    MOVE_GETSHIPS,               //4
    MOVE_FORMATION,              //5
    MOVE_MOVETEAM,               //6
    MOVE_MOVETEAMINDEX,          //7
    MOVE_MOVETEAMSPLIT,          //8
    MOVE_INTERCEPT,              //9
    MOVE_MOVETO,                 //10
    MOVE_PATROLMOVE,             //11
    MOVE_ACTIVEPATROL,           //12
    MOVE_ACTIVERECON,            //13
    MOVE_SHIPRECON,              //14
    MOVE_COUNTSHIPS,             //15
    MOVE_SPECIAL,                //16
    MOVE_ATTACK,                 //17
    MOVE_ADVANCEDATTACK,         //18
    MOVE_FLANKATTACK,            //19
    MOVE_MOVEATTACK,             //20
    MOVE_HARASSATTACK,           //21
    MOVE_SWARMATTACK,            //22
    MOVE_SWARMDEFENSE,           //23
    MOVE_SWARMPOD,               //24
    MOVE_FANCYGETSHIPS,          //25
    MOVE_DOCK,                   //26
    MOVE_LAUNCH,                 //27
    MOVE_REINFORCE,              //28
    MOVE_VARSET,                 //29
    MOVE_VARINC,                 //30
    MOVE_VARDEC,                 //31
    MOVE_VARWAIT,                //32
    MOVE_VARDESTROY,             //33
    MOVE_GUARDCOOPTEAM,          //34
    MOVE_SUPPORT,                //35
    MOVE_ARMADA,                 //36
    MOVE_CONTROLRESOURCES,       //37
    MOVE_RESVOLUME,              //38
    MOVE_CAPTURE,                //39
    MOVE_ACTIVECAPTURE,          //40
    MOVE_ACTIVEMINE,             //41
    MOVE_MINEVOLUME,             //42
    MOVE_SPECIALDEFENSE,         //43
    MOVE_ACTIVERES,              //44
    MOVE_MOTHERSHIP,             //45
    MOVE_KAMIKAZE,               //46
    MOVE_HYPERSPACE,             //47
    MOVE_DELETETEAM,             //48
    NUMBER_OF_MOVETYPES    // must go last
} AIMoveTypes;

typedef struct {
    ShipType  shipType;
    sdword    numShips;
    AIVar     *doneVar;
    sdword    priority;
} paramsGetShips;

#define MAX_NUM_ALTERNATIVES            12      // make sure it's a multiple of 4 for alignment purposes
#define ALTERNATIVE_SHIP_BASE           10      // base value for equivalence comparisons between ships
#define ALTERNATIVE_RANDOM              1       // alternativeFlags - indicates alternatives should be picked randomly

#define AIT_FAST_PATROL                 0
#define AIT_SLOW_PATROL                 1

typedef struct AlternativeShips
{
    sdword  numNextPicks;
    sdword  alternativeFlags;
    sbyte   shipTypeNextPicks[MAX_NUM_ALTERNATIVES];
    sbyte   shipNumEquivNextPicks[MAX_NUM_ALTERNATIVES];
} AlternativeShips;

#define SetNumAlternatives(alt,num) \
    (alt).numNextPicks = (num);   \
    (alt).alternativeFlags = 0; \
    dbgAssert((num) <= MAX_NUM_ALTERNATIVES);

#define SetNumAlternativesFlags(alt,num,flags) \
    (alt).numNextPicks = (num);   \
    (alt).alternativeFlags = (flags); \
    dbgAssert((num) <= MAX_NUM_ALTERNATIVES);

#define SetAlternative(alt,index,stype,equivnum) \
    (alt).shipTypeNextPicks[index] = (sbyte)(stype);  \
    (alt).shipNumEquivNextPicks[index] = (equivnum);

typedef struct {
    char varName[AIVAR_LABEL_MAX_LENGTH+1];
    sdword value;
} paramsVarSet;

typedef struct {
    char varName[AIVAR_LABEL_MAX_LENGTH+1];
    sdword value;
} paramsVarWait;

typedef struct {
    char varName[AIVAR_LABEL_MAX_LENGTH+1];
} paramsVarInc;

typedef struct {
    char varName[AIVAR_LABEL_MAX_LENGTH+1];
} paramsVarDec;

typedef struct {
    char varName[AIVAR_LABEL_MAX_LENGTH+1];
} paramsVarDestroy;

typedef struct {
    ShipType shipType;
    sdword   numShips;
    AIVar    *doneVar;
    sdword   priority;
    AlternativeShips alternatives;
} paramsFancyGetShips;

typedef struct {
    SelectCommand *ships;
} paramsGuardShips;

typedef struct
{
    SelectCommand *targets;
} paramsDefMoship;

typedef struct
{
    //nothing yet
    udword placeholder;
} paramsTempGuard;

typedef struct {
    SelectCommand *ships;
} paramsKamikaze;

typedef struct {
    SelectCommand *ships;
} paramsAttackShips;

typedef struct
{
    SelectCommand *targets;
    Ship *target_ship;
} paramsAdvancedAttack;

typedef struct
{
    SelectCommand *targets;
    bool8          hyperspace;
} paramsFlankAttack;

typedef struct
{
    SelectCommand *targets;
    ShipPtr target_ship;
    vector  destination;
} paramsMoveAttack;

typedef struct
{
    vector destination;
    udword index;
} paramsMoveTeam;

typedef struct
{
    SelectCommand *ships;
    Path *destinations;
} paramsMoveSplit;

typedef struct
{
    ShipPtr ship;
    real32  interval;
    real32  next_int;
    vector  destination;
    bool    moving;
} paramsIntercept;

typedef struct
{
    vector destination;
    vector source;
    real32 limiter;
    real32 timer;
    udword type;
} paramsMoveTo;

typedef struct
{
    Path *path;
    struct AITeamMove *loopMove;
    udword startIndex;
} paramsPatrolMove;

typedef struct
{
    udword patroltype;
} paramsActivePatrol;

typedef struct
{
    bool outwards;
    bool enemyrecon;
} paramsActiveRecon;

typedef struct
{
    SelectCommand *ships;
    SelectCommand *foundships;
} paramsShipRecon;

typedef struct
{
    //nothing yet
    udword placeholder;
} paramsCountShips;

typedef struct {
    TypeOfFormation formationtype;
} paramsFormation;

typedef struct
{
    ShipPtr target;
} paramsHarassAttack;

typedef struct
{
    SelectCommand *targets;
    SelectCommand *othertargets;
    GrowSelection newSwarmers;
//    ShipType      targettype;
} paramsSwarmAttack;

typedef struct
{
    GrowSelection newSwarmers;
    SelectCommand *guarding;
    SelectCommand *Pods;
    bool           full_attack;
    bool           full_refuel;
} paramsSwarmDefense;

typedef struct
{
    bool first_attack;
    udword attack_delay;
} paramsSwarmPod;

#define dockmoveFlags_Normal            0
#define dockmoveFlags_AtShip            1
#define dockmoveFlags_Stay              2
#define dockmoveFlags_Instantaneously   4
typedef struct
{
    SelectCommand *shipsDocking;
    ShipPtr dockAt;
    sdword dockmoveFlags;
} paramsDock;

typedef struct
{
    struct AITeam *reinforceteam;
} paramsReinforce;

typedef struct
{
    SelectCommand *ships;
} paramsSupport;

typedef struct
{
    Volume  volume;
    ResourceSelection *volResources;
    ResourceSelection *takenResources;
    bool8 strictVolume;
} paramsResVol;


typedef struct
{
    ShipPtr ship;
} paramsCapture;

typedef struct
{
    Volume volume;
    bool   inposition;
} paramsMineVol;

//
//  teams
//
#define MSG_QUEUE_MAX_MSGS 32
typedef struct {
    sdword          head;
    char            *msgs[MSG_QUEUE_MAX_MSGS];
    struct AITeam   *msgSenders[MSG_QUEUE_MAX_MSGS];  // team that sent the corresponding msg above
                                                      // NOTE:  if a team dies, the message will be deleted from this queue
} MsgQueue;

typedef enum {
    AttackTeam,
    DefenseTeam,
    ResourceTeam,
    ScriptTeam,
    AnyTeam,        //neuter type
    NumTeamType
} TeamType;

typedef struct AITeam {
    TeamType          teamType;
    udword            teamFlags;
    udword            TeamFeatures;
    ubyte             teamDelay;
    udword            newships;
    struct AIPlayer   *aiplayerowner;
    GrowSelection     shipList;
    udword            teamStrength;
    udword            teamValue;
    ubyte             teamDifficultyLevel;
    LinkedList        moves;
    struct AITeamMove *curMove;
    void              *custTeamInfo;
    void (*TeamDiedCB)(struct AITeam *team);
    struct AITeam     *cooperatingTeam;
    void (*cooperatingTeamDiedCB)(struct AITeam *team);
    MsgQueue          *msgQueue;
    struct AITeam     *msgSender;

    // KAS attributes (if teamType == SCRIPT_TEAM)
    char                kasLabel[KAS_TEAM_NAME_MAX_LENGTH+1];
    char                kasFSMName[KAS_FSM_NAME_MAX_LENGTH+1];
    char                kasStateName[KAS_STATE_NAME_MAX_LENGTH+1];
    KASWatchFunction    kasFSMWatchFunction;
    KASWatchFunction    kasStateWatchFunction;
    ShipType            kasOrigShipsType;  // predominant ship type of this team (in mission layout file)
    sdword              kasOrigShipsCount; // original size of team (in mission layout file)
    TacticsType         kasTactics;  // current tactics setting, passed to certain moves
    TypeOfFormation     kasFormation; // current formation setting, passed to certain moves
} AITeam;

// all move processing functions look like this:
// (returning 1 when we this move has completed
//  and any required waiting is finished, 0 otherwise)
typedef sdword (*aimProcessFunction) (AITeam *team);
typedef void (*aimShipDied) (AITeam *team, struct AITeamMove *move, Ship *ship);
typedef void (*aimCloseFunction) (AITeam *team, struct AITeamMove *move);
typedef void (*aimResourceDied) (AITeam *team, struct AITeamMove *move, Resource *resource);

typedef struct AITeamMove {
    Node listNode;
    AIMoveTypes type;

    union {
        paramsGuardShips        guardShips;
        paramsDefMoship         defmoship;
        paramsTempGuard         tempguard;
        paramsGetShips          getShips;
        paramsFormation         formation;
        paramsMoveTeam          move;
        paramsMoveSplit         movesplit;
        paramsIntercept         intercept;
        paramsMoveTo            moveTo;
        paramsPatrolMove        patrolmove;
        paramsActivePatrol      activepatrol;
        paramsActiveRecon       activerecon;
        paramsShipRecon         shiprecon;
        paramsCountShips        countShips;
        paramsAttackShips       attack;
        paramsAdvancedAttack    advatt;
        paramsFlankAttack       flankatt;
        paramsMoveAttack        moveatt;
        paramsHarassAttack      harass;
        paramsSwarmAttack       swarmatt;
        paramsSwarmDefense      swarmdef;
        paramsSwarmPod          swarmpod;
        paramsFancyGetShips     fancyGetShips;
        paramsDock              dock;
        paramsReinforce         reinforce;
        paramsSupport           support;
        paramsSupport           rescontrol;
        paramsResVol            resvolume;
        paramsCapture           capture;
        paramsMineVol           minevolume;
        paramsVarSet            varSet;
        paramsVarInc            varInc;
        paramsVarDec            varDec;
        paramsVarWait           varWait;
        paramsVarDestroy        varDestroy;
        paramsKamikaze          kamikaze;

        // ...
    } params;

    AIEvents events;

    aimProcessFunction processFunction;
    aimShipDied moveShipDiedFunction;
    aimCloseFunction moveCloseFunction;

    //only for resource oriented moves
    aimResourceDied moveResourceDiedFunction;

    bool8 processing;   // if the process function has starting working on this move yet

    bool8 wait;    // non-zero = wait for the move to finish
    bool8 remove;  // non-zero = remove this move from the move list when it has finished
    TypeOfFormation formation;
    TacticsType     tactics;

} AITeamMove;

/*=============================================================================
    Macros:
=============================================================================*/
#define InitNewMove(newMove,movetype,waitflag,removeflag,form,tactic,moveprocessfunc,moveshipdiedfunc,moveclosefunc)    \
    (newMove)->type                     = (movetype);             \
    (newMove)->processing               = FALSE;                  \
    (newMove)->wait                     = (waitflag);             \
    (newMove)->remove                   = (removeflag);           \
    (newMove)->formation                = (form);                 \
    dbgAssert(tactic >= 0);                                       \
    dbgAssert(tactic < NUM_TACTICS_TYPES);                        \
    (newMove)->tactics                  = (tactic);               \
    (newMove)->processFunction          = (moveprocessfunc);      \
    (newMove)->moveShipDiedFunction     = (moveshipdiedfunc);     \
    (newMove)->moveCloseFunction        = (moveclosefunc);        \
    aieHandlersClear(newMove)

#define FixMoveFuncPtrs(move,moveprocessfunc,moveshipdiedfunc,moveclosefunc)     \
    (move)->processFunction          = (moveprocessfunc);      \
    (move)->moveShipDiedFunction     = (moveshipdiedfunc);     \
    (move)->moveCloseFunction        = (moveclosefunc)

#define aitApproxTeamPos(team)          ((team)->shipList.selection->ShipPtr[0]->posinfo.position)
#define aitNumTeamShips(team)           ((team)->shipList.selection->numShips)
#define aitTeamShipTypeIs(type, team)   (((team)->shipList.selection->numShips) && ((team)->shipList.selection->ShipPtr[0]->shiptype == (type)))
#define aitTeamShipClassIs(cls, team)   (((team)->shipList.selection->numShips) && ((team)->shipList.selection->ShipPtr[0]->staticinfo->shipclass == (cls)))

/*=============================================================================
    Function Prototypes:
=============================================================================*/
//teams initialization and closing routines
void aitInit(struct AIPlayer *aiplayer);
void aitClose(struct AIPlayer *aiplayer);


/*-----------------------------------------------------------------------------
    General Utility Functions:
-----------------------------------------------------------------------------*/
void aitAddmoveBeforeAndMakeCurrent(AITeam *team, AITeamMove *newMove, AITeamMove *thisMove);
void aitAddmoveBeforeAndMakeCurrentNoSpecial(AITeam *team, AITeamMove *newMove, AITeamMove *thisMove);

/*-----------------------------------------------------------------------------
    Team State Query Functions:
-----------------------------------------------------------------------------*/
bool aitAnyTeamOfPlayerAttackingThisShip(struct AIPlayer *aiplayer,Ship *ship);

udword aitFindNumTeamsWithFlag(udword flag);
AITeam *aitFindNextTeamWithFlag(AITeam *team, udword flag);

sdword aitCountTeamsWaitingForShips(TeamType type);

//team status functions
bool aitTeamIsDone(AITeam *team);
bool aitTeamIsIdle(AITeam *team);
bool aitTeamIsGuarding(AITeam *team);
bool aitTeamIsAttacking(AITeam *team);
bool aitTeamIsntDefendingMothership(AITeam *team, SelectCommand *enemyships);
bool aitTeamIsFinishedMoving(AITeam *team, vector destination, real32 range);
bool aitTeamIsDoingSpecialOp(AITeam *team);
bool aitCheckIfOtherDefTeamAnsweringSignalNeedsHelp(AITeam *team, SelectCommand *ships);
bool aitTeamIsInRange(AITeam *team, ShipPtr ship, real32 time);
bool aitTeamIsInMothershipRange(AITeam *team);

void aitRecallGuardTeam(AITeam *team);
bool aitNeedStrikeSupport(udword minstr);

//set team attributes
void aitMakeTeamSupportShips(AITeam *team, SelectCommand *ships);
void aitSetTeamHomoHetero(AITeam *team);
void aitSetTeamSpecialFlags(AITeam *team);
bool aitTeamHomogenous(AITeam *team);
void aitSetTeamStrengthValue(AITeam *team);
ubyte aitSetTeamDifficultyLevel(AITeam *team, udword playerDL, udword teamDL);

AITeam *aitFindMostValuable(udword valueness);

AITeam *aitFindGoodCoopTeam(ShipType type);

AITeam *aitCreate(TeamType teamType);
void aitDestroy(struct AIPlayer *aiplayer, AITeam *team, bool removeAllReferencesToTeam);

void aitDeleteCurrentMove(AITeam *team);
void aitDeleteMovesUntilMoveType(AITeam *team, AIMoveTypes type);
void aitDeleteAllTeamMoves(AITeam *team);

void aitAddShip(AITeam *team, ShipPtr ship);

//swarm specific code
void aitMoveSwarmShipDefenseToAttack(AITeam *attackSwarm, AITeam *defenseSwarm, ShipPtr ship);
void aitMoveAllSwarmShipsDefenseToAttack(AITeam *defenseSwarm, AITeam *attackSwarm);
void aitMoveAllSwarmShipsAttackToDefense(AITeam *attackSwarm, AITeam *defenseSwarm);
void aitMoveAllSwarmShipsDefense(AITeam *destTeam, AITeam *sourceTeam);
void aitMoveAllSwarmShipsAttack(AITeam *destTeam, AITeam *sourceTeam);
AITeam *aitFindNewPod(AITeam *defenseTeam);
void aitMoveSwarmersToNewPod(AITeam *defenseTeam, AITeam *podTeam);
bool aitAllDefenseSwarmersFull(void);

void aitSpecialDefenseCoopTeamDiedCB(AITeam *team);
void GenericCooperatingTeamDiedCB(AITeam *team);

// returns TRUE if removed ship
bool aitRemoveShip(AITeam *team, ShipPtr ship);

//process current move for each team
void aitExecute(void);


void aitShipDied(struct AIPlayer *aiplayer,ShipPtr ship);
void aitResourceDied(struct AIPlayer *aiplayer, Resource *resource);

bool aitCheckForLeaderAndMoveToFront(AITeam *team);

bool aitCheckAmIBeingWatched(AITeam *team, SelectCommand *sel);
void aitSetAmIBeingWatched(AITeam *team, SelectCommand *sel);


sdword aitMsgReceived(AITeam *teamp, char *msg);
void aitMsgSend(AITeam *fromTeamp, AITeam *teamp, char *msg);
void aitMsgQueueFree(AITeam *teamp);

void aitSave(struct AIPlayer *aiplayer);
void aitLoad(struct AIPlayer *aiplayer);
void aitFix(struct AIPlayer *aiplayer);

//
//  reserved pointers to all allocated AIVars
//
#define AITEAM_ALLOC_INITIAL   64
#define AITEAM_ALLOC_INCREMENT 32

/*=============================================================================
    Globals:
=============================================================================*/

extern AITeam *savingThisAITeam;

#endif
