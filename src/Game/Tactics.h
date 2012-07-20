/*=============================================================================
    Tactics.c: Control Functions for Homeworld Tactics

    Created June 1998 Bryce
=============================================================================*/

#ifndef TACTICS_H
#define TACTICS_H

#include "SpaceObj.h"
#include "ObjTypes.h"
#include "Task.h"
#include "Region.h"
#include "Select.h"
#include "mainswitches.h"

// comment this bitch out and no more tacticsOn crap
//#define DEBUG_TACTICS

#define NO_ORDER_NO_ENEMY       0           //default if other conditions don't apply
#define NO_ORDER_ENEMY          1
#define MOVE_ORDERS_NO_ENEMY    2
#define MOVE_ORDERS_ENEMY       3
#define ATTACK_ORDERS           4
#define ATTACK_ORDERS_ENEMY     5
#define NUM_TACTICS_SITUATIONS  6

#define ORDER_TYPE_NO_ORDERS     0
#define ORDER_TYPE_MOVE_ORDERS   2
#define ORDER_TYPE_ATTACK_ORDERS 4
#define ORDER_TYPE_ENEMY_MASK    1  //can OR this two a order type to add enemy near bonuses and behavior

#define Tactics_Fighter     0
#define Tactics_Corvette    1
#define NUM_TACTICS_APPLICABLE 2

#define TightFormation  0               //default
#define LooseFormation  1               //for majority evasive,combat neutral
#define NUM_FORMATION_PADDINGS 2        //counter for blah

//dodge array references
#define DODGE_ONE_TIME      0
#define DODGE_TWO_TIME      1
#define DODGE_THRUST_MULT   2
#define DODGE_WAIT          3
#define NUM_DODGE_INFO      4

//ship object variable tacticsTalk flags
#define TACTALK_FLIPTURN    0x01
#define TACTALK_GEN_FLIGHMAN_NEED_LOCK  0x02
#define TACTALK_BARRELROLL  0x04

typedef struct
{
    Node retreatLink;
    struct Ship *retreater;         //ship that is retreating
    CommandToDo *fleeingfrom;       //attack command that ship is fleeing from
    vector retreatFrom;             //position of Attackers (average location)
    real32 distanceStartSqr;        //square of distance from attackers at time of retreat command
} RetreatAtom;

typedef struct
{
    Node attackLink;
    struct Ship *retreater;         //ship that originally retreated
    real32 expirytime;
    real32 retreatTime;             //univupdate time that retreat was registered

    //must be last!!!
    SelectCommand attackerList;    //list of ships on the lookout for retreater ship
} AttackAtom;

typedef struct
{
    udword EnemyNearByCheckRate;
    real32 EnemyNearZone;
    real32 PassiveRetaliationFireTimeModifierLow;
    real32 PassiveRetaliationFireTimeModifierHi;
    udword MenuFeedBack_X,MenuFeedBack_Y;
    real32 menuPopupTime;
    real32 RetreatDistanceSqr[NUM_CLASSES];
    real32 RetreatGetAwayDistSqr[NUM_CLASSES];
    udword RetreatCheckRate;
    real32 freeRetreatTime;
    real32 AttackMemoryTimeOut;
    real32 capitalTransMult[NUM_TRANS_DEGOFFREEDOM];
    real32 capitalRotMult[NUM_ROT_DEGOFFREEDOM];
    real32 capitalTurnMult[NUM_TURN_TYPES];
    real32 InterceptorVerticalMultiplier;
    real32 MaxVelocityBonus[NUM_TACTICS_SITUATIONS][NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    real32 FuelBurnBonus[NUM_TACTICS_SITUATIONS][NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    real32 formationPadding[NUM_FORMATION_PADDINGS];
    real32 DodgeInfo[NUM_DODGE_INFO][TOTAL_NUM_SHIPS];          //waste of memory but easiest to reference
    real32 DodgeLowAddTime,DodgeHighAddTime;
    real32 DamageBonus[NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    real32 BulletRangeBonus[NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    real32 BulletSpeedBonus[NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    real32 ManeuvBonus[NUM_TACTICS_SITUATIONS][NUM_TACTICS_APPLICABLE][NUM_TACTICS_TYPES];
    sdword tacticsGuardConditionCheckRate;
    sdword tacticsGuardConditionCheckFrame;
    real32 tacticsGuardReturnToGuardingDistance;
    real32 tacticsGuardReturnToGuardingDistanceSqr;
    sdword holdFormationDuringBattle[NUM_TACTICS_TYPES];
    real32 tacticsDamageMultiplierInFormation;
    real32 tacticsDamageMultiplierInSphereFormation;
    real32 tacticsBurstFireTimeModifier[NUM_TACTICS_TYPES];
    real32 tacticsBurstWaitTimeModifier[NUM_TACTICS_TYPES];
    real32 tacticsBurstFireTimeSphereModifier;
    real32 tacticsBurstWaitTimeSphereModifier;
    real32 tacticsBurstFireTimeWingmanModifier;
    real32 tacticsBurstWaitTimeWingmanModifier;
    real32 movingAttackTurnsIntoMoveCommandDistanceSqr;
    real32 tacticsDamageModifierRed;
    real32 tacticsDamageModifierYellow;
} TacticsInfo;

extern TacticsInfo tacticsInfo;

//special operation tweakables
extern real32 kamikazeDamage[TOTAL_NUM_SHIPS];
extern real32 kamikazeSpeedBurst;
extern real32 kamikazeFuelBurnMult;
extern real32 speedBurstMaxVelocityMultiplier;
extern real32 speedBurstThrustMult;
extern real32 speedBurstDuration;
extern real32 speedBurstFuelBurnMult;
extern real32 speedBurstCoolDown;
extern real32 burstRange;
extern real32 burstChargeTime;
extern real32 burstCoolDownTime;
extern real32 burstDamageLo[NUM_RACES];
extern real32 burstDamageHi[NUM_RACES];
extern real32 burstSpeed;
extern real32 oneOverburstSpeed;
extern real32 burstRadius;

extern real32 ATTACKING_FROM_ABOVE_MIN_DIST;
extern real32 ATTACKING_FROM_ABOVE_MIN_RATIO;

//data

extern BabyCallBack    *tacticsFlashMenuBaby;
extern regionhandle    tacticsMenuRegion;
//ProtoTypes

void tacticsStartUp();
void tacticsShutDown();
void tacticsPopUpSetUp(TacticsType tacticstype);
void tacticsSetSelectionToTactics(SelectCommand *selection,udword orderflag);
void tacticsSetShipToDoDodge(Ship *ship);
void tacticsDoDodge(Ship *ship);
//sdword tacticsGetFormationOptimalState(SelectCommand *selection);
void tacticsUpdate(Ship *ship);

void tacticsDelegateAttackCommand(Ship *ship,CommandToDo *command, AttackCommand *attack, bool gettingRocked,bool doingSomething);
void tacticsDelegateSingleAttack(Ship *ship,CommandLayer *comlayer,SelectCommand *selectone,AttackCommand *attack, bool gettingRocked);
void tacticsReportMove(CommandLayer *comlayer,SelectCommand *selection);


void tacticsShipDied(Ship *ship);
void tacticsGlobalUpdate();
void tacticsAttackCommandVoided(CommandToDo *command);

void tacticsRearrangeFormationSuperDuper(SelectCommand *selection,TypeOfFormation formationtype);
real32 tacticsMaxDistToTarget(SelectCommand *selection,SpaceObjRotImpTarg *target);
bool tacticsCheckGuardConditionsDuringAttack(struct CommandToDo *command);
real32 tacticsGetShipsMaxVelocity(Ship *ship);

bool tacticsAreStrikeCraftInSelection(SelectCommand *selection);

//TRUE if shipA has retreated from shipB, only valid for time that
//shipsB is on lookout for shipA (maybe 90seconds?  tunable)
bool tacticsHasShipRetreatedFromShip(Ship *shipA, Ship *shipB);

void tacticsMakeShipsNotLookForOtherShips(SelectCommand *selection);

extern real32 IONCANNON_TARGETS_FIGHTER_ANGLE;
extern real32 IONCANNON_TARGETS_CORVETTE_ANGLE;

extern real32 IONCANNON_TARGETS_FIGHTER_TRIGGERHAPPY;
extern real32 IONCANNON_TARGETS_CORVETTE_TRIGGERHAPPY;

extern real32 IONCANNON_TARGETS_FIGHTER_MAXANGLESPEED;
extern real32 IONCANNON_TARGETS_CORVETTE_MAXANGLESPEED;

#endif