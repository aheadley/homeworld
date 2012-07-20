/*=============================================================================
    Tactics.c: Control Functions for Homeworld Tactics

    Created June 1998 Bryce
=============================================================================*/
#include <stdlib.h>

#include "Tactics.h"
#include "UnivUpdate.h"
#include "StatScript.h"
#include "Region.h"
#include "FEFlow.h"
#include "mainrgn.h"
#include "Task.h"
#include "Universe.h"
#include "Physics.h"
#include "ClassDefs.h"
#include "Types.h"
#include "CommandLayer.h"
#include "Tweak.h"
#include "Blobs.h"
#include "ProximitySensor.h"
#include "Formation.h"
#include "FastMath.h"
#include "RepairCorvette.h"
#include "FlightMan.h"
#include "mouse.h"
#include "SalCapCorvette.h"
#include "Alliance.h"
#include "Probe.h"
#include "Randy.h"
#include "SoundEvent.h"
#include "Battle.h"

//global data
BabyCallBack    *tacticsFlashMenuBaby;
regionhandle    tacticsMenuRegion;

//special ops tweakables
real32 kamikazeDamage[TOTAL_NUM_SHIPS];
real32 kamikazeSpeedBurst;
real32 kamikazeFuelBurnMult;
real32 speedBurstMaxVelocityMultiplier;
real32 speedBurstThrustMult;
real32 speedBurstDuration;
real32 speedBurstFuelBurnMult;
real32 speedBurstCoolDown;
real32 burstRange;
real32 burstChargeTime;
real32 burstCoolDownTime;
real32 burstDamageLo[NUM_RACES];
real32 burstDamageHi[NUM_RACES];
real32 burstSpeed;
real32 oneOverburstSpeed;
real32 burstRadius;

real32 ATTACKING_FROM_ABOVE_MIN_DIST = 250.0f;
real32 ATTACKING_FROM_ABOVE_MIN_RATIO = 2.0f;

real32 OUT_OF_FUEL_VELOCITY_SLOWDOWN = 0.125f;

real32 IONCANNON_TARGETS_FIGHTER_ANGLE = 0.99939f;
real32 IONCANNON_TARGETS_CORVETTE_ANGLE = 0.99756f;

real32 IONCANNON_TARGETS_FIGHTER_TRIGGERHAPPY = 0.99939f;
real32 IONCANNON_TARGETS_CORVETTE_TRIGGERHAPPY = 0.99756f;

real32 IONCANNON_TARGETS_FIGHTER_MAXANGLESPEED = 3.0f;
real32 IONCANNON_TARGETS_CORVETTE_MAXANGLESPEED = 3.0f;

real32 GLOBAL_SHIP_SPEED_MODIFIER = 1.0f;

real32 DELAY_FORMTIGHTNESS_CHANGE = 5.0f;

//local prototypes
bool tacticsShipCanDodge(Ship *ship);
bool tacticsShipIsAffectedByTactcis(Ship *ship);
bool tacticsIsShipLookingForAnyOfThese(Ship *ship,SelectCommand *selection);
bool tacticsAreEnemiesNearby(Ship *leadership, Ship *thisship,real32 retaliateZone);

//special ops prototype
void speedBurstUpdate(Ship *ship);

TacticsInfo tacticsInfo;

scriptEntry TacticsInfotable[] =
{
    { "EnemyNearZone" ,scriptSetReal32CB, &tacticsInfo.EnemyNearZone},
    { "PassiveRetaliationFireTimeModifierLow" ,scriptSetReal32CB, &tacticsInfo.PassiveRetaliationFireTimeModifierLow},
    { "PassiveRetaliationFireTimeModifierHi" ,scriptSetReal32CB, &tacticsInfo.PassiveRetaliationFireTimeModifierHi},
    { "MenuFeedBack_X" ,scriptSetUdwordCB, &tacticsInfo.MenuFeedBack_X},
    { "MenuFeedBack_Y" ,scriptSetUdwordCB, &tacticsInfo.MenuFeedBack_Y},
    { "menuPopupTime" ,scriptSetReal32CB, &tacticsInfo.menuPopupTime},

    makeEntry(ATTACKING_FROM_ABOVE_MIN_DIST,scriptSetReal32CB),
    makeEntry(ATTACKING_FROM_ABOVE_MIN_RATIO,scriptSetReal32CB),

    { "RetreatDistance[CLASS_Mothership]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Mothership]},
    { "RetreatDistance[CLASS_HeavyCruiser]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_HeavyCruiser]},
    { "RetreatDistance[CLASS_Carrier]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Carrier]},
    { "RetreatDistance[CLASS_Destroyer]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Destroyer]},
    { "RetreatDistance[CLASS_Frigate]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Frigate]},
    { "RetreatDistance[CLASS_Corvette]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Corvette]},
    { "RetreatDistance[CLASS_Fighter]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Fighter]},
    { "RetreatDistance[CLASS_Resource]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_Resource]},
    { "RetreatDistance[CLASS_NonCombat]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatDistanceSqr[CLASS_NonCombat]},

    { "RetreatGetAwayDistance[CLASS_Mothership]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Mothership]},
    { "RetreatGetAwayDistance[CLASS_HeavyCruiser]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_HeavyCruiser]},
    { "RetreatGetAwayDistance[CLASS_Carrier]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Carrier]},
    { "RetreatGetAwayDistance[CLASS_Destroyer]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Destroyer]},
    { "RetreatGetAwayDistance[CLASS_Frigate]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Frigate]},
    { "RetreatGetAwayDistance[CLASS_Corvette]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Corvette]},
    { "RetreatGetAwayDistance[CLASS_Fighter]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Fighter]},
    { "RetreatGetAwayDistance[CLASS_Resource]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_Resource]},
    { "RetreatGetAwayDistance[CLASS_NonCombat]" ,scriptSetReal32SqrCB, &tacticsInfo.RetreatGetAwayDistSqr[CLASS_NonCombat]},

    { "freeRetreatTime" ,scriptSetReal32CB, &tacticsInfo.freeRetreatTime},

    { "AttackMemoryTimeOut" ,scriptSetReal32CB, &tacticsInfo.AttackMemoryTimeOut},

    { "formationPadding[LooseFormation]",scriptSetReal32CB, &tacticsInfo.formationPadding[LooseFormation]},
    { "formationPadding[TightFormation]",scriptSetReal32CB, &tacticsInfo.formationPadding[TightFormation]},

    { "InterceptorVerticalMultiplier" ,scriptSetReal32CB, &tacticsInfo.InterceptorVerticalMultiplier},

    { "DodgeLowAddTime" ,scriptSetReal32CB, &tacticsInfo.DodgeLowAddTime},
    { "DodgeHighAddTime" ,scriptSetReal32CB, &tacticsInfo.DodgeHighAddTime},

    { "DodgeInfo[DODGE_ONE_TIME][AttackBomber]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][AttackBomber]},
    { "DodgeInfo[DODGE_TWO_TIME][AttackBomber]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][AttackBomber]},
    { "DodgeInfo[DODGE_THRUST_MULT][AttackBomber]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][AttackBomber]},
    { "DodgeInfo[DODGE_WAIT][AttackBomber]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][AttackBomber]},

    { "DodgeInfo[DODGE_ONE_TIME][CloakedFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][CloakedFighter]},
    { "DodgeInfo[DODGE_TWO_TIME][CloakedFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][CloakedFighter]},
    { "DodgeInfo[DODGE_THRUST_MULT][CloakedFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][CloakedFighter]},
    { "DodgeInfo[DODGE_WAIT][CloakedFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][CloakedFighter]},

    { "DodgeInfo[DODGE_ONE_TIME][DefenseFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][DefenseFighter]},
    { "DodgeInfo[DODGE_TWO_TIME][DefenseFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][DefenseFighter]},
    { "DodgeInfo[DODGE_THRUST_MULT][DefenseFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][DefenseFighter]},
    { "DodgeInfo[DODGE_WAIT][DefenseFighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][DefenseFighter]},


    { "DodgeInfo[DODGE_ONE_TIME][HeavyCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][HeavyCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][HeavyCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][HeavyCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][HeavyCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][HeavyCorvette]},
    { "DodgeInfo[DODGE_WAIT][HeavyCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][HeavyCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][HeavyDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][HeavyDefender]},
    { "DodgeInfo[DODGE_TWO_TIME][HeavyDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][HeavyDefender]},
    { "DodgeInfo[DODGE_THRUST_MULT][HeavyDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][HeavyDefender]},
    { "DodgeInfo[DODGE_WAIT][HeavyDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][HeavyDefender]},

    { "DodgeInfo[DODGE_ONE_TIME][HeavyInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][HeavyInterceptor]},
    { "DodgeInfo[DODGE_TWO_TIME][HeavyInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][HeavyInterceptor]},
    { "DodgeInfo[DODGE_THRUST_MULT][HeavyInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][HeavyInterceptor]},
    { "DodgeInfo[DODGE_WAIT][HeavyInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][HeavyInterceptor]},

    { "DodgeInfo[DODGE_ONE_TIME][LightCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][LightCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][LightCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][LightCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][LightCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][LightCorvette]},
    { "DodgeInfo[DODGE_WAIT][LightCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][LightCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][LightDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][LightDefender]},
    { "DodgeInfo[DODGE_TWO_TIME][LightDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][LightDefender]},
    { "DodgeInfo[DODGE_THRUST_MULT][LightDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][LightDefender]},
    { "DodgeInfo[DODGE_WAIT][LightDefender]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][LightDefender]},

    { "DodgeInfo[DODGE_ONE_TIME][LightInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][LightInterceptor]},
    { "DodgeInfo[DODGE_TWO_TIME][LightInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][LightInterceptor]},
    { "DodgeInfo[DODGE_THRUST_MULT][LightInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][LightInterceptor]},
    { "DodgeInfo[DODGE_WAIT][LightInterceptor]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][LightInterceptor]},

    { "DodgeInfo[DODGE_ONE_TIME][MinelayerCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][MinelayerCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][MinelayerCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][MinelayerCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][MinelayerCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][MinelayerCorvette]},
    { "DodgeInfo[DODGE_WAIT][MinelayerCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][MinelayerCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][MultiGunCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][MultiGunCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][MultiGunCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][MultiGunCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][MultiGunCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][MultiGunCorvette]},
    { "DodgeInfo[DODGE_WAIT][MultiGunCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][MultiGunCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][RepairCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][RepairCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][RepairCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][RepairCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][RepairCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][RepairCorvette]},
    { "DodgeInfo[DODGE_WAIT][RepairCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][RepairCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][SalCapCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][SalCapCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][SalCapCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][SalCapCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][SalCapCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][SalCapCorvette]},
    { "DodgeInfo[DODGE_WAIT][SalCapCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][SalCapCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][P1Fighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][P1Fighter]},
    { "DodgeInfo[DODGE_TWO_TIME][P1Fighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][P1Fighter]},
    { "DodgeInfo[DODGE_THRUST_MULT][P1Fighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][P1Fighter]},
    { "DodgeInfo[DODGE_WAIT][P1Fighter]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][P1Fighter]},

    { "DodgeInfo[DODGE_ONE_TIME][P1MissileCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][P1MissileCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][P1MissileCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][P1MissileCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][P1MissileCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][P1MissileCorvette]},
    { "DodgeInfo[DODGE_WAIT][P1MissileCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][P1MissileCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][P1StandardCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][P1StandardCorvette]},
    { "DodgeInfo[DODGE_TWO_TIME][P1StandardCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][P1StandardCorvette]},
    { "DodgeInfo[DODGE_THRUST_MULT][P1StandardCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][P1StandardCorvette]},
    { "DodgeInfo[DODGE_WAIT][P1StandardCorvette]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][P1StandardCorvette]},

    { "DodgeInfo[DODGE_ONE_TIME][P2AdvanceSwarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][P2AdvanceSwarmer]},
    { "DodgeInfo[DODGE_TWO_TIME][P2AdvanceSwarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][P2AdvanceSwarmer]},
    { "DodgeInfo[DODGE_THRUST_MULT][P2AdvanceSwarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][P2AdvanceSwarmer]},
    { "DodgeInfo[DODGE_WAIT][P2AdvanceSwarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][P2AdvanceSwarmer]},

    { "DodgeInfo[DODGE_ONE_TIME][P2Swarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_ONE_TIME][P2Swarmer]},
    { "DodgeInfo[DODGE_TWO_TIME][P2Swarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_TWO_TIME][P2Swarmer]},
    { "DodgeInfo[DODGE_THRUST_MULT][P2Swarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][P2Swarmer]},
    { "DodgeInfo[DODGE_WAIT][P2Swarmer]",scriptSetReal32CB, &tacticsInfo.DodgeInfo[DODGE_WAIT][P2Swarmer]},

    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},

    { "DamageBonus[Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Fighter][Evasive]},
    { "DamageBonus[Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Fighter][Neutral]},
    { "DamageBonus[Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Fighter][Aggressive]},

    { "BulletRangeBonus[Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Fighter][Evasive]},
    { "BulletRangeBonus[Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Fighter][Neutral]},
    { "BulletRangeBonus[Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Fighter][Aggressive]},

    { "BulletSpeedBonus[Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Fighter][Evasive]},
    { "BulletSpeedBonus[Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Fighter][Neutral]},
    { "BulletSpeedBonus[Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Fighter][Aggressive]},

    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]},

    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]},

    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.MaxVelocityBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "DamageBonus[Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Corvette][Evasive]},
    { "DamageBonus[Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Corvette][Neutral]},
    { "DamageBonus[Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.DamageBonus[Tactics_Corvette][Aggressive]},

    { "BulletRangeBonus[Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Corvette][Evasive]},
    { "BulletRangeBonus[Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Corvette][Neutral]},
    { "BulletRangeBonus[Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.BulletRangeBonus[Tactics_Corvette][Aggressive]},

    { "BulletSpeedBonus[Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Corvette][Evasive]},
    { "BulletSpeedBonus[Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Corvette][Neutral]},
    { "BulletSpeedBonus[Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.BulletSpeedBonus[Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]},

    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},

    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]},

    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.FuelBurnBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},



    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Evasive]},
    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Neutral]},
    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Corvette][Aggressive]},

    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[NO_ORDER_ENEMY][Tactics_Fighter][Aggressive]},

    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_NO_ENEMY][Tactics_Fighter][Aggressive]},

    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[MOVE_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},

    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS][Tactics_Fighter][Aggressive]},

    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Evasive]},
    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Neutral]},
    { "ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]", scriptSetReal32CB, &tacticsInfo.ManeuvBonus[ATTACK_ORDERS_ENEMY][Tactics_Fighter][Aggressive]},
    { "tacticsGuardReturnToGuardingDistance", scriptSetReal32CB, &tacticsInfo.tacticsGuardReturnToGuardingDistance},
    { "tacticsGuardConditionCheckRate", scriptSetReal32CB, &tacticsInfo.tacticsGuardConditionCheckRate},
    { "tacticsGuardConditionCheckFrame", scriptSetReal32CB, &tacticsInfo.tacticsGuardConditionCheckFrame},

    { "kamikazeDamage[AttackBomber]", scriptSetReal32CB, &kamikazeDamage[AttackBomber]},
    { "kamikazeDamage[CloakedFighter]", scriptSetReal32CB, &kamikazeDamage[CloakedFighter]},
    { "kamikazeDamage[DefenseFighter]", scriptSetReal32CB, &kamikazeDamage[DefenseFighter]},
    { "kamikazeDamage[HeavyCorvette]", scriptSetReal32CB, &kamikazeDamage[HeavyCorvette]},
    { "kamikazeDamage[HeavyDefender]", scriptSetReal32CB, &kamikazeDamage[HeavyDefender]},
    { "kamikazeDamage[HeavyInterceptor]", scriptSetReal32CB, &kamikazeDamage[HeavyInterceptor]},
    { "kamikazeDamage[LightCorvette]", scriptSetReal32CB, &kamikazeDamage[LightCorvette]},
    { "kamikazeDamage[LightDefender]", scriptSetReal32CB, &kamikazeDamage[LightDefender]},
    { "kamikazeDamage[LightInterceptor]", scriptSetReal32CB, &kamikazeDamage[LightInterceptor]},
    { "kamikazeDamage[MinelayerCorvette]", scriptSetReal32CB, &kamikazeDamage[MinelayerCorvette]},
    { "kamikazeDamage[MultiGunCorvette]", scriptSetReal32CB, &kamikazeDamage[MultiGunCorvette]},
    { "kamikazeDamage[RepairCorvette]", scriptSetReal32CB, &kamikazeDamage[RepairCorvette]},
    { "kamikazeDamage[ResourceCollector]", scriptSetReal32CB, &kamikazeDamage[ResourceCollector]},
    { "kamikazeDamage[SalCapCorvette]", scriptSetReal32CB, &kamikazeDamage[SalCapCorvette]},
    { "kamikazeDamage[P1Fighter]", scriptSetReal32CB, &kamikazeDamage[P1Fighter]},
    { "kamikazeDamage[P2AdvanceSwarmer]", scriptSetReal32CB, &kamikazeDamage[P2AdvanceSwarmer]},
    { "kamikazeDamage[P2FuelPod]", scriptSetReal32CB, &kamikazeDamage[P2FuelPod]},
    { "kamikazeDamage[P2Swarmer]", scriptSetReal32CB, &kamikazeDamage[P2Swarmer]},

    { "kamikazeSpeedBurst", scriptSetReal32CB, &kamikazeSpeedBurst},
    { "kamikazeFuelBurnMult", scriptSetReal32CB, &kamikazeFuelBurnMult},
    { "speedBurstMaxVelocityMultiplier", scriptSetReal32CB, &speedBurstMaxVelocityMultiplier},
    { "speedBurstThrustMult", scriptSetReal32CB, &speedBurstThrustMult},
    { "speedBurstDuration", scriptSetReal32CB, &speedBurstDuration},
    { "speedBurstFuelBurnMult", scriptSetReal32CB, &speedBurstFuelBurnMult},
    { "speedBurstCoolDown", scriptSetReal32CB, &speedBurstCoolDown},

    { "burstRange", scriptSetReal32CB, &burstRange},
    { "burstChargeTime", scriptSetReal32CB, &burstChargeTime},
    { "burstCoolDownTime", scriptSetReal32CB, &burstCoolDownTime},

    { "burstDamageLo[R1]", scriptSetReal32CB, &burstDamageLo[R1]},
    { "burstDamageHi[R1]", scriptSetReal32CB, &burstDamageHi[R1]},
    { "burstDamageLo[R2]", scriptSetReal32CB, &burstDamageLo[R2]},
    { "burstDamageHi[R2]", scriptSetReal32CB, &burstDamageHi[R2]},
    { "burstSpeed", scriptSetReal32CB, &burstSpeed},
    { "burstRadius", scriptSetReal32CB, &burstRadius},

    { "holdFormationDuringBattle[Evasive]", scriptSetSdwordCB, &tacticsInfo.holdFormationDuringBattle[Evasive]},
    { "holdFormationDuringBattle[Neutral]", scriptSetSdwordCB, &tacticsInfo.holdFormationDuringBattle[Neutral]},
    { "holdFormationDuringBattle[Aggressive]", scriptSetSdwordCB, &tacticsInfo.holdFormationDuringBattle[Aggressive]},

    { "tacticsBurstFireTimeModifier[Evasive]", scriptSetReal32CB, &tacticsInfo.tacticsBurstFireTimeModifier[Evasive]},
    { "tacticsBurstFireTimeModifier[Neutral]", scriptSetReal32CB, &tacticsInfo.tacticsBurstFireTimeModifier[Neutral]},
    { "tacticsBurstFireTimeModifier[Aggressive]", scriptSetReal32CB, &tacticsInfo.tacticsBurstFireTimeModifier[Aggressive]},
    { "tacticsBurstWaitTimeModifier[Evasive]", scriptSetReal32CB, &tacticsInfo.tacticsBurstWaitTimeModifier[Evasive]},
    { "tacticsBurstWaitTimeModifier[Neutral]", scriptSetReal32CB, &tacticsInfo.tacticsBurstWaitTimeModifier[Neutral]},
    { "tacticsBurstWaitTimeModifier[Aggressive]", scriptSetReal32CB, &tacticsInfo.tacticsBurstWaitTimeModifier[Aggressive]},
    { "tacticsBurstFireTimeSphereModifier", scriptSetReal32CB, &tacticsInfo.tacticsBurstFireTimeSphereModifier},
    { "tacticsBurstWaitTimeSphereModifier", scriptSetReal32CB, &tacticsInfo.tacticsBurstWaitTimeSphereModifier},
    { "tacticsBurstFireTimeWingmanModifier", scriptSetReal32CB, &tacticsInfo.tacticsBurstFireTimeWingmanModifier},
    { "tacticsBurstWaitTimeWingmanModifier", scriptSetReal32CB, &tacticsInfo.tacticsBurstWaitTimeWingmanModifier},

    { "tacticsDamageMultiplierInFormation", scriptSetReal32CB, &tacticsInfo.tacticsDamageMultiplierInFormation},
    { "tacticsDamageMultiplierInSphereFormation", scriptSetReal32CB, &tacticsInfo.tacticsDamageMultiplierInSphereFormation},
    { "movingAttackTurnsIntoMoveCommandDistance", scriptSetReal32SqrCB, &tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr},

    { "tacticsDamageModifierRed", scriptSetReal32CB, &tacticsInfo.tacticsDamageModifierRed},
    { "tacticsDamageModifierYellow", scriptSetReal32CB, &tacticsInfo.tacticsDamageModifierYellow},

    makeEntry(OUT_OF_FUEL_VELOCITY_SLOWDOWN,scriptSetReal32CB),
    makeEntry(IONCANNON_TARGETS_FIGHTER_ANGLE,scriptSetCosAngCB),
    makeEntry(IONCANNON_TARGETS_CORVETTE_ANGLE,scriptSetCosAngCB),
    makeEntry(IONCANNON_TARGETS_FIGHTER_TRIGGERHAPPY,scriptSetCosAngCB),
    makeEntry(IONCANNON_TARGETS_CORVETTE_TRIGGERHAPPY,scriptSetCosAngCB),
    makeEntry(IONCANNON_TARGETS_FIGHTER_MAXANGLESPEED,scriptSetAngCB),
    makeEntry(IONCANNON_TARGETS_CORVETTE_MAXANGLESPEED,scriptSetAngCB),

    makeEntry(GLOBAL_SHIP_SPEED_MODIFIER,scriptSetReal32CB),

    makeEntry(DELAY_FORMTIGHTNESS_CHANGE,scriptSetReal32CB),

    endEntry
};

void tacticsStartUp()
{
    sdword x,y,z;
    tacticsInfo.MenuFeedBack_X = 0;
    tacticsInfo.MenuFeedBack_Y = 0;
    tacticsInfo.EnemyNearZone = 0;
    for(x=0;x< NUM_TACTICS_SITUATIONS;x++)
        for(y=0;y< NUM_TACTICS_APPLICABLE;y++)
            for(z=0;z< NUM_TACTICS_TYPES;z++)
                  tacticsInfo.MaxVelocityBonus[x][y][z] = 1.0f;

    for(x=0;x< NUM_TACTICS_SITUATIONS;x++)
        for(y=0;y< NUM_TACTICS_APPLICABLE;y++)
            for(z=0;z< NUM_TACTICS_TYPES;z++)
                  tacticsInfo.FuelBurnBonus[x][y][z] = 1.0f;

    for(x=0;x< NUM_TACTICS_SITUATIONS;x++)
        for(y=0;y< NUM_TACTICS_APPLICABLE;y++)
            for(z=0;z< NUM_TACTICS_TYPES;z++)
                  tacticsInfo.ManeuvBonus[x][y][z] = 1.0f;

    for(x=0;x< NUM_DODGE_INFO;x++)
        for(y=0;y< TOTAL_NUM_SHIPS;y++)
           tacticsInfo.DodgeInfo[x][y] = 0.0f;


    for(x=0;x< NUM_FORMATION_PADDINGS;x++)
        tacticsInfo.formationPadding[x] = 1.0f;


    for(x=0;x< NUM_TACTICS_APPLICABLE;x++)
        for(y=0;y< NUM_TACTICS_TYPES;y++)
        {
            tacticsInfo.DamageBonus[x][y] = 1.0f;
            tacticsInfo.BulletRangeBonus[x][y] = 1.0f;
            tacticsInfo.BulletSpeedBonus[x][y] = 1.0f;
        }
#ifdef DEBUG_TACTICS
    if(!tacticsOn)
        return;
#endif


    scriptSet(NULL,"Tactics.script",TacticsInfotable);

    //precalculate certain variables
    tacticsInfo.tacticsGuardReturnToGuardingDistanceSqr = tacticsInfo.tacticsGuardReturnToGuardingDistance*tacticsInfo.tacticsGuardReturnToGuardingDistance;
    oneOverburstSpeed = 1.0f/burstSpeed;
    tacticsFlashMenuBaby = NULL;
    tacticsMenuRegion = NULL;
    listInit(&universe.RetreatList);
    listInit(&universe.AttackMemory);
}

void tacticsShutDown()
{
    if(tacticsFlashMenuBaby != NULL)
    {
        taskCallBackRemove(tacticsFlashMenuBaby);
        tacticsFlashMenuBaby = NULL;
    }
    if(tacticsMenuRegion != NULL)
    {
        feMenuDisappear(NULL,NULL);
        tacticsMenuRegion = NULL;
    }
    listDeleteAll(&universe.RetreatList);
    listDeleteAll(&universe.AttackMemory);
}
extern regionhandle ghMainRegion;

bool tacticsDeleteMenu(udword num, void *data, struct BabyCallBack *baby)
{
#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif

    if(tacticsMenuRegion != NULL)
    {
        //only remove menu if it is up...
        feMenuDisappear(NULL,NULL);
    }
    tacticsFlashMenuBaby = NULL;
    return(TRUE);
}
void tacticsPopUpSetUp(TacticsType tacticstype)
{
    fescreen *TacticsScreen = feScreenFind("Group_Tactics_submenu");
    regionhandle regionptr;
    featom *babyspice, *atom;
    sdword index;

#ifdef DEBUG_TACTICS
    if(!tacticsOn) return;
#endif

    if(TacticsScreen == NULL)
    {
        dbgFatalf(DBG_Loc, "\nCouldn't find menu item for tactics craptics");
    }
    //set the correct checked bit for tactics
    for (index = 0; index < TacticsScreen->nAtoms; index++)
    {                                                       //clear all atoms of the selected bit
        bitClear(TacticsScreen->atoms[index].status, FAS_Checked);
    }
    atom = feAtomFindInScreen(TacticsScreen, mrMenuItemByTactic[tacticstype]);
    dbgAssert(atom != NULL);
    bitSet(atom->status, FAS_Checked);

    if(tacticsFlashMenuBaby != NULL)
    {
        taskCallBackRemove(tacticsFlashMenuBaby);
        if(tacticsMenuRegion != NULL)
        {
            feMenuDisappear(NULL,NULL);
            tacticsMenuRegion = NULL;
        }
        tacticsFlashMenuBaby = NULL;
    }
    //store this pointer to set another pointer to delete it later...:)
    tacticsMenuRegion = feMenuStart(ghMainRegion,TacticsScreen, mouseCursorX(), mouseCursorY());

    //disable proccessing functions
    regionptr = tacticsMenuRegion->child;
    while(regionptr != NULL)
    {
        babyspice = (featom *) regionptr->userID;
        regionptr->processFunction = &regNULLProcessFunction;
        regionptr = regionptr->next;
    }
    tacticsFlashMenuBaby = taskCallBackRegister(tacticsDeleteMenu, 0, NULL, tacticsInfo.menuPopupTime);
/*
    switch(tacticstype)
    {
    case Evasive:

        break;
    case Neutral:
        break;
    case Aggressive:
        break;
    default:
        dbgAssert(FALSE);
    }
*/
}

/*********************************************************************************************/
//  tacticsSetSelectionToTactics(SelectCommand *selection,sdword order_type)
//
//  Changes selections tactics_order type to order_type
/********************************************************************************************/

void tacticsSetSelectionToTactics(SelectCommand *selection,udword orderFlag)
{
    sdword i;

    switch(orderFlag)
    {
    case ORDER_TYPE_MOVE_ORDERS:
        break;

    }
    for(i=0;i<selection->numShips;i++)
    {
        selection->ShipPtr[i]->tactics_ordertype = orderFlag;
    }
}

void tacticsSetShipToDoDodge(Ship *ship)
{
#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif

    if(tacticsShipCanDodge(ship))
    {
        CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
        if (command != NULL)
        {
            if(ship->shiptype == SalCapCorvette)
            {
                {
                    if(command->ordertype.order == COMMAND_SPECIAL ||
                       command->ordertype.order == COMMAND_DOCK)
                    {
                        //don't let salcap dodge if this is the case
                        return;
                    }
                }
            }
        }
        if(ship->dockingship != NULL)
        {
            //don't dock if in this stage of docking!
            return;
        }
        if (ship->flags & SOF_Cloaked)
        {
            return;     // cloaked ships don't dodge!
        }

        if (ship->specialFlags & SPECIAL_Kamikaze)
        {
            return;     // kamikaze ships don't dodge!
        }

        if(ship->tacticstype != Evasive)
        {
            CommandToDo *command=getShipAndItsCommand(&universe.mainCommandLayer,ship);
            if(ship->shiptype != HeavyDefender)
            {
                if(command != NULL)
                {
                    if(command->ordertype.attributes & COMMAND_IS_FORMATION)
                    {
                        //ship is in sphere..but is the sphere formed?
                        if(command->formation.formationtype == SPHERE_FORMATION)
                        {
                            //as long as we aren't evassive, we shouldn't dodge
                            return;
                        }
                    }
                }
            }
        }
        if(ship->isDodging == TRUE)
        {
            return;
        }
        else
        {
            //ship isn't currently dodging
            if(universe.totaltimeelapsed < ship->DodgeTime)
                return; //ship hasn't cooled down from dodge yet
        }

        if(ship->flightman != FLIGHTMAN_NULL)
        {
            //ship is performing a flightmaneuver...don't dodge
            if(ship->flightman != FLIGHTMAN_BARREL_ROLL)
               return;
        }
        ship->isDodging = TRUE;
        ship->DodgeTime = universe.totaltimeelapsed + tacticsInfo.DodgeInfo[DODGE_ONE_TIME][ship->staticinfo->shiptype];
        ship->DodgeDir = (uword) randombetween(0,TRANS_LEFT);
        ship->DodgeFlag = 0;
    }
}

void tacticsDoDodge(Ship *ship)
{
#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif

    physApplyForceToObj((SpaceObj *)ship,ship->staticinfo->thruststrengthstat[ship->DodgeDir]*tacticsInfo.DodgeInfo[DODGE_THRUST_MULT][ship->staticinfo->shiptype],ship->DodgeDir);
    if(universe.totaltimeelapsed > ship->DodgeTime)
    {
        //dodge time elapsed...stop dodging
        if(ship->DodgeFlag == 1)
        {
            ship->isDodging = FALSE;
            ship->DodgeTime = universe.totaltimeelapsed + tacticsInfo.DodgeInfo[DODGE_WAIT][ship->staticinfo->shiptype]+frandombetween(tacticsInfo.DodgeLowAddTime,tacticsInfo.DodgeHighAddTime);
            return;
        }
        switch(ship->DodgeDir)
        {                                //toggle dodge direction to oposite way
        case TRANS_UP:
            ship->DodgeDir = TRANS_DOWN;
            break;
        case TRANS_DOWN:
            ship->DodgeDir = TRANS_UP;
            break;
        case TRANS_RIGHT:
            ship->DodgeDir = TRANS_LEFT;
            break;
        case TRANS_LEFT:
            ship->DodgeDir = TRANS_RIGHT;
            break;
        case TRANS_FORWARD:
            ship->DodgeDir = TRANS_BACKWARD;
            break;
        case TRANS_BACKWARD:
            ship->DodgeDir = TRANS_FORWARD;
            break;
        }
        ship->DodgeTime = universe.totaltimeelapsed + tacticsInfo.DodgeInfo[DODGE_TWO_TIME][ship->staticinfo->shiptype];
        ship->DodgeFlag = 1;
    }
}

bool tacticsShipCanDodge(Ship *ship)
{
#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif
    if(tacticsInfo.DodgeInfo[DODGE_ONE_TIME][ship->staticinfo->shiptype] == 0.0f)
    {
        //ship has dodging turned off
        return FALSE;
    }

    if(ship->staticinfo->maxfuel != 0.0f)
    {
        //ship has fuel///lkets check it
        if(ship->fuel <= 0.0f)
        {
            //out of fuel...can't dodge
            return FALSE;
        }
    }
    return(tacticsShipIsAffectedByTactcis(ship));
}

bool tacticsShipIsAffectedByTactcis(Ship *ship)
{
#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif

    if(ship->staticinfo->shipclass == CLASS_Fighter ||
        ship->staticinfo->shipclass == CLASS_Corvette)
    {
        return(TRUE);
    }
    return(FALSE);
}
/*******************************************************************************************
sdword tacticsGetFormationOptimalState(SelectCommand *selection);
    returns a type (formationLoose or formationTight) depending on rules for formation formation :)
    if any agressive pressent, go tight.  That way slowest setting will not screw up formation
**********************************************************************************************/

sdword tacticsGetFormationOptimalState(SelectCommand *selection)
{
    sdword i;
    //sdword cA,cN,cE;    //counters for different tactics settings in formation

    TacticsType grouptactics = selection->ShipPtr[0]->tacticstype;

#ifdef DEBUG_TACTICS
    dbgAssert(tacticsOn);
#endif

    //base formation on leaders tactical settings
    if(grouptactics != Aggressive)
    {
        //check ships for evvasive and neutral settings
        for(i=0;i<selection->numShips;i++)
        {
            //if non-strike craft in formation, maintain tight formation
            if(!tacticsShipIsAffectedByTactcis(selection->ShipPtr[i]))
                return(TightFormation);

            //for now, loose formations ONLY if ALL evasive...later we'll fix
            // if(selection->ShipPtr[i]->tacticstype == Aggressive ||
            //   selection->ShipPtr[i]->tacticstype == Neutral)
            //   return(TightFormation);
            switch(selection->ShipPtr[i]->tactics_ordertype)
            {
            case MOVE_ORDERS_NO_ENEMY:
            case NO_ORDER_NO_ENEMY:
                //no enemy near this ship, so tightness is favourved
                break;
            case NO_ORDER_ENEMY:
            case MOVE_ORDERS_ENEMY:
            case ATTACK_ORDERS:
            case ATTACK_ORDERS_ENEMY:
                //if atleast 1 ship in formation has been told to attack
                //or is near an enemy, we want loose formations...
                return(LooseFormation);
            default:
                dbgFatalf(DBG_Loc,"\nUnkown tactical order state in ship.");
            }
        }
    }
    return(TightFormation);
}
void tacticsBeFairToRetreaters(Ship *ship, SelectCommand *targets)
{
    AttackAtom  *attackatom;
    Node *node;
    bool flagFreeBe;
    sdword i,j;

    node = universe.AttackMemory.head;

    //fix later: optimize by having a ship flag set whether or not
    //a ship is on the look out!!!!

    while(node != NULL)
    {
        attackatom = (AttackAtom *)listGetStructOfNode(node);
        //if a retreat occurred very soon, we won't look for them
        flagFreeBe = FALSE;
        if(attackatom->retreatTime > universe.totaltimeelapsed)
        {
            for(i=0;i<attackatom->attackerList.numShips;i++)
            {
                if(ship == attackatom->attackerList.ShipPtr[i])
                {
                    //ship is looking for something
                    for(j=0;j<targets->numShips;j++)
                    {
                        if(targets->ShipPtr[j] == attackatom->retreater)
                        {
                            clRemoveShipFromSelection((SelectCommand *)targets,attackatom->retreater);
                            //fix later...might be able to
                            //goto next node here, can a ship be in multiple attacklists?
                            break;
                        }
                    }
                }
            }
        }

        node = node->next;
    }
}

void BreakUpShipsIntoAttackCapable(SelectCommand *selection,
                                   MaxSelection *attackSelection,
                                   MaxSelection *nonAttackSelection)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if (isShipAttackCapable(ship))
        {
            attackSelection->ShipPtr[attackSelection->numShips++] = ship;
        }
        else
        {
            nonAttackSelection->ShipPtr[nonAttackSelection->numShips++] = ship;
        }
    }
#if 0       // Bryce, this wasn't consistent with the isShipAttackCapable code in shipselect.h, and caused bugs
    sdword i,j,k;
    j=k=0;
    for(i = 0;i<selection->numShips;i++)
    {
        switch(selection->ShipPtr[i]->staticinfo->shipclass)
        {
        case CLASS_HeavyCruiser:
        case CLASS_Carrier:
        case CLASS_Destroyer:
        case CLASS_Frigate:
        case CLASS_Corvette:
        case CLASS_Fighter:
            attackSelection->ShipPtr[j] = selection->ShipPtr[i];
            attackSelection->numShips++;
            j++;
            break;
        default:
            nonAttackSelection->ShipPtr[k] = selection->ShipPtr[k];
            nonAttackSelection->numShips++;
            k++;
            break;
        }
    }
#endif
}

#define SPEECH_RETALIATE_REPEAT 15.0f

void tacticsSpeechForRetaliation(Ship *ship)
{
    if(ship->retaliateSpeechTime <= universe.totaltimeelapsed)
    {
        if(ship->playerowner == universe.curPlayerPtr)
        {
            if(!selAnyHotKeyTest(ship))
            {
                if(ship->retaliateSpeechVar == FALSE)
                {
                    if (battleCanChatterAtThisTime(BCE_STAT_Grp_AttackRetaliate, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_AttackRetaliate, ship, SOUND_EVENT_DEFAULT);
                        ship->retaliateSpeechVar = TRUE;
                        ship->retaliateSpeechTime = universe.totaltimeelapsed + SPEECH_RETALIATE_REPEAT;
                    }
                }
                else
                {
                    if (battleCanChatterAtThisTime(BCE_STAT_Grp_AttackRetaliate_Repeat, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_AttackRetaliate_Repeat, ship, SOUND_EVENT_DEFAULT);
                        ship->retaliateSpeechVar = FALSE;
                        ship->retaliateSpeechTime = universe.totaltimeelapsed + SPEECH_RETALIATE_REPEAT;
                    }
                }
            }
            else
            {
                if(ship->retaliateSpeechVar == FALSE)
                {
                    if (battleCanChatterAtThisTime(BCE_STAT_AssGrp_AttackRetaliate, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_AssGrp_AttackRetaliate, ship, selHotKeyGroupNumberTest(ship));
                        ship->retaliateSpeechVar = TRUE;
                        ship->retaliateSpeechTime = universe.totaltimeelapsed + SPEECH_RETALIATE_REPEAT;
                    }
                }
                else
                {
                    if (battleCanChatterAtThisTime(BCE_STAT_AssGrp_AttackRetaliate_Repeat, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_AssGrp_AttackRetaliate_Repeat, ship, selHotKeyGroupNumberTest(ship));
                        ship->retaliateSpeechVar = FALSE;
                        ship->retaliateSpeechTime = universe.totaltimeelapsed + SPEECH_RETALIATE_REPEAT;
                    }
                }
            }
        }
    }

}


void tacticsDelegateAttackCommand(Ship *ship,CommandToDo *command, AttackCommand *attack, bool gettingRocked,bool doingSomething)
{
    //ship is in retreat list...so attack it...
    //
    //******************
    //should probably delete that attackmemory item...or at least
    //certain ships from it...will lookinto that
    //*******************************
    MaxSelection attackSelection,nonAttackSelection;
    attackSelection.numShips = 0;
    nonAttackSelection.numShips = 0;

    //As long as we're not getting hit right now, we'll
    //give retreaters a chance to getaway.
    if(!gettingRocked)
    {
#ifdef DEBUG_TACTICS
//        dbgMessagef("\nBeing Fair to possible Retreaters.");
#endif
        tacticsBeFairToRetreaters(ship,(SelectCommand *)attack);
        if(attack->numTargets < 1)
            return;
    }

    //if we're busy, can only passive attack
    if(doingSomething)
    {
        //ship is doing something (moving,docking)
        //so only passive attack
        if(command->selection->ShipPtr[0]->tacticstype != Evasive)
        {
            if (canChangeOrderToPassiveAttack(command,attack))
            {
                ChangeOrderToPassiveAttack(command,attack);
            }
        }
        return;
    }

    //if ships aren't doing something
    //AND ships are actually 'on the lookout' for some of these target
    //we'll full on attack those beeyatches...otherwise..we'll go through
    //normal passive attacking procedures.
    if(!doingSomething && tacticsIsShipLookingForAnyOfThese(ship,(SelectCommand *)attack)
       && ship->shiptype != Mothership)     // don't want to ChangeOrderToAttack for Mothership
    {
#ifdef DEBUG_TACTICS
        dbgMessagef("\nAttacking Previous Retreaters");
#endif
        BreakUpShipsIntoAttackCapable(command->selection,&attackSelection,&nonAttackSelection);
        if(attackSelection.numShips == command->selection->numShips)
        {
            ChangeOrderToAttack(command,attack);
        }
        else if(attackSelection.numShips > 0)
        {
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&nonAttackSelection);
            dbgAssert(command->selection->numShips > 0);
            ChangeOrderToAttack(command,attack);
        }
        return;
    }

    switch(ship->tacticstype)
    {
    case Aggressive:
        //give counter attack order


#ifdef DEBUG_TACTICS
        if(doingSomething)
        {
            dbgFatalf(DBG_Loc,"\ndoingsomething == TRUE, allowed to retask to full attack!  BAD!");
        }
#endif
        if (ship->shiptype == Mothership)
        {
            goto neutralstuff;      // don't want to ChangeOrderToAttack for Mothership
        }

        /*  //not needed anymore..ships can't fullout attack during movements,docks
        if(command->ordertype.order == COMMAND_DOCK)
        {
            //ships are docking, so only passive attack...
            if (canChangeOrderToPassiveAttack(command,attack))
            {
                ChangeOrderToPassiveAttack(command,attack);
            }
            break;
        }
        */
        BreakUpShipsIntoAttackCapable(command->selection,&attackSelection,&nonAttackSelection);
        if(attackSelection.numShips == command->selection->numShips)
        {
            ChangeOrderToAttack(command,attack);
        }
        else if(attackSelection.numShips > 0)
        {
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&nonAttackSelection);
            dbgAssert(command->selection->numShips > 0);
            ChangeOrderToAttack(command,attack);
        }
        break;

    case Neutral:
neutralstuff:
        //passive attack
        if (canChangeOrderToPassiveAttack(command,attack))
        {
            ChangeOrderToPassiveAttack(command,attack);
        }
        tacticsSpeechForRetaliation(ship);
        break;
    case Evasive:
        //doc de la design says no passive retaliation at ALL for
        //evassive tacticized ships

        //not doing this anymore

//        if (canChangeOrderToPassiveAttack(command,attack))
//        {
 //           ChangeOrderToPassiveAttack(command,attack);
  //      }
  //      tacticsSpeechForRetaliation(ship);
        break;
    default:
        break;
    }

}


void tacticsDelegateSingleAttack(Ship *ship,CommandLayer *comlayer,SelectCommand *selectone,AttackCommand *attack, bool gettingRocked)
{
    CommandToDo *cmtest;

    if(!gettingRocked)
    {
#ifdef DEBUG_TACTICS
//        dbgMessagef("\nBeing Fair to possible Retreaters.");
#endif
        tacticsBeFairToRetreaters(ship,(SelectCommand *)attack);
        if(attack->numTargets < 1)
            return;
    }

    if(tacticsIsShipLookingForAnyOfThese(ship,(SelectCommand *)attack))
    {
        clAttack(comlayer,selectone,attack);
#ifdef DEBUG_TACTICS
        dbgMessagef("\nAttacking Previous Retreaters");
#endif
        return;
    }

    if(isCapitalShip(ship))
    {
        //capital ships only passive attack
        if(ship->tacticstype != Evasive)
        {
            //don't passive retaliate in evassive
            clPassiveAttack(comlayer,selectone,attack);
        }
        return;
    }

    switch(ship->tacticstype)
    {
    case Aggressive:
        //give counter attack order
        cmtest = getShipAndItsCommand(comlayer,ship);
        if(cmtest != NULL)
        {
            if(cmtest->ordertype.order == COMMAND_DOCK)
            {
                //passive attack
                clPassiveAttack(comlayer,selectone,attack);
                break;
            }
        }
        clAttack(comlayer,selectone,attack);
        break;
    case Neutral:
        //passive attack
        clPassiveAttack(comlayer,selectone,attack);
        tacticsSpeechForRetaliation(ship);
        break;
    case Evasive:
        //don't passive retaliate any more

        //later don't passive retaliate...simply dodge for now, do it though
        //clPassiveAttack(comlayer,selectone,attack);
        //tacticsSpeechForRetaliation(ship);
        break;
    default:
        break;
    }

}

//name: tacticsRemoveRetreat
//
//desc: Removes and deletes the retreat order 'r' from the tactics retreat list

void tacticsRemoveRetreat(RetreatAtom *r)
{
    listDeleteNode(&r->retreatLink);
}

//name: tacticsShipIsRetreatingFromThisCommand
//
//desc: checks to see if this ship is retreating from this command

RetreatAtom *tacticsShipIsRetreatingFromThisCommand(Ship *ship, CommandToDo *command)
{
    Node *link;
    RetreatAtom *r;

    link = universe.RetreatList.head;

    while(link != NULL)
    {
        r = (RetreatAtom *)listGetStructOfNode(link);


        if(r->retreater == ship)
        {
            //found ship
            if(r->fleeingfrom == command)
            {
                //found command...
                return(r);
            }
        }

        link = link->next;
    }
    return(NULL);
}

//Name: tacticsSetRetreatFrom
//
//Description:  Calculates the average location of a group of ships
//

void setAveragePosition(vector *vec, SelectCommand *selection)
{
    udword i;
    real32 oneOverShips;
    vec->x = 0.0f; vec->y = 0.0f; vec->z = 0.0f;

    for(i = 0; i < selection->numShips; i++)
    {
        vecAddTo(*vec,selection->ShipPtr[i]->posinfo.position);
    }
    oneOverShips = 1.0f/selection->numShips;
    vecScalarMultiply(*vec,*vec,oneOverShips);
}


//Name: tacticsReportMove
//
//Description:  Function scans todo list and deals with any commands with attackinfo
//              dealing with the ships in selection.

void tacticsReportMove(CommandLayer *comlayer,SelectCommand *selection)
{
    Node *curnode;
    CommandToDo *command;
    sdword i,j;
    vector  testdistvec;


    curnode = comlayer->todolist.head;

#ifdef DEBUG_TACTICS
    if(noRetreat)
        return;
#endif

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if(command->ordertype.order == COMMAND_ATTACK)
        {
            if(command->selection->ShipPtr[0]->tacticstype == Aggressive)
            {
                //command is set to agressive <general simplicity
                //assumption that all ships in the command have the same
                //tactics setting> so ships can't retreat.
                goto nextnode;
            }
            if (command->ordertype.attributes & COMMAND_IS_FORMATION)
            {
                if(!command->formation.doneInitialAttack)
                {
                    //group of ships hasn't attacked yet...so
                    //don't register a retreat yet!
                    goto nextnode;
                }
            }
            else
            {
                //ship(s) aren't in formation...
                //mustn't let ships retreat if not close enough!
                //!!! Ships must be within RetreatGetAwayDistSqr
                vecSub(testdistvec,command->selection->ShipPtr[0]->posinfo.position,selection->ShipPtr[0]->posinfo.position);
                if(vecMagnitudeSquared(testdistvec) > tacticsInfo.RetreatGetAwayDistSqr[selection->ShipPtr[0]->staticinfo->shipclass])
                {
                    //too far away to be considered for retreating
                    goto nextnode;
                }
            }

            for(i=0;i<((SelectCommand *)command->attack)->numShips;i++)
            {
                for(j=0;j<selection->numShips;j++)
                {
                    if(selection->ShipPtr[j] == ((SelectCommand *)command->attack)->ShipPtr[i])
                    {
                        //ship that has been given move command is also
                        //being attacked by this command.  So
                        //set retreat conditions and do check somewheres
                        vector tempvec,tempRetFrom;//,oldvec;
                        real32 tempdistanceStartSqr;//,olddistanceStartSqr;
                        RetreatAtom *newRetreat,*oldretreat;

                        setAveragePosition(&tempRetFrom, (SelectCommand *)command->attack);

                        vecSub(tempvec,selection->ShipPtr[j]->posinfo.position,tempRetFrom);
                        tempdistanceStartSqr = vecMagnitudeSquared(tempvec);

                        oldretreat = tacticsShipIsRetreatingFromThisCommand(selection->ShipPtr[j], command);
                        if(oldretreat != NULL)
                        {
                            //ship is currently retreating from this command, so don't do anything
                            continue;

                            //ship is currently retreating from this command (amongst others possibly)
                            //lets choose the more advantagous retreat plan.

                            /*
                            vecSub(oldvec,selection->ShipPtr[j]->posinfo.position,oldretreat->retreatFrom);
                            olddistanceStartSqr = vecMagnitudeSquared(tempvec);
                            if(olddistanceStartSqr < oldretreat->distanceStartSqr)
                            {
                                continue;
                            }
                            else
                            {
                                //cancel current ships retreat from this
                                //command, and lets create a new one
#ifdef DEBUG_TACTICS
                                dbgMessagef("\nremoving old retreat in favour of a better one");
#endif
                                tacticsRemoveRetreat(oldretreat);
                            }
                            */
                        }

                        newRetreat = (RetreatAtom *) memAlloc(sizeof(RetreatAtom),"Retreat Data", NonVolatile);


                        newRetreat->retreater = selection->ShipPtr[j];
                        newRetreat->fleeingfrom = command;

                        newRetreat->retreatFrom = tempRetFrom;
                        newRetreat->distanceStartSqr = tempdistanceStartSqr;
                        listAddNode(&universe.RetreatList,&newRetreat->retreatLink,newRetreat);
#ifdef bpasechn
    #ifdef DEBUG_TACTICS
                        dbgMessagef("\nRetreat Registered.");
    #endif
#endif
                        //maybe make a retreat list that we update
                        //in tactics update every Xth frame
                        //we also remove references as ships die n such.
                        //pain in ass, but efficient

                        //when retreat conditions are met, ship
                        //is added to a ship 'memory' list.  This
                        //list will contain ships that must be attacked
                        //if ships come within range.  Checked in
                        //tacticsDelegateAttack.
                        //ship memory lists must be checked and
                        //cleaned up as ships die
                        //and must also have expiry times on entries.

                    }

                }
            }
        }
nextnode:
        curnode = curnode->next;
    }

}

//Name: tacticsShipDied
//
//Desc: A ship has died, so we need to remove certain references
//      from our tactics data structures
void tacticsShipDied(Ship *ship)
{
    sdword i;
    Node *link,*templink;
    RetreatAtom *r;
    AttackAtom *a;

#ifdef DEBUG_TACTICS
    if(!tacticsOn)
        return;
#endif

    link = universe.RetreatList.head;
    while(link != NULL)
    {
        r = (RetreatAtom *) listGetStructOfNode(link);
        if(r->retreater == ship)
        {
            templink = link->next;

            listDeleteNode(link);

            link = templink;
            continue;
        }
        link = link->next;
    }

    link = universe.AttackMemory.head;
    while(link != NULL)
    {
        a = (AttackAtom *) listGetStructOfNode(link);
        if(a->retreater == ship)
        {
            goto deleteNode;
        }
        for(i=0;i<a->attackerList.numShips;i++)
        {
            if(ship == a->attackerList.ShipPtr[i])
            {
                a->attackerList.numShips--;
                if(a->attackerList.numShips == 0)
                    goto deleteNode;
                a->attackerList.ShipPtr[i] = a->attackerList.ShipPtr[a->attackerList.numShips];
                return;
            }
        }

        link = link->next;
        continue;

deleteNode:
        templink = link->next;

        listDeleteNode(link);

        link = templink;
        return;

    }



}
//name tacticsSelCopy(&newattackatom->attackList,lookers);
//
//desc: copies contents of one selection to the other
void tacticsSelCopy(SelectCommand *other,SelectCommand *one)
{
    sdword i;
    for(i=0;i<one->numShips;i++)
    {
        other->ShipPtr[i] = one->ShipPtr[i];
    }
    other->numShips = one->numShips;
}

//Name:     tacticsAttackCommandVoided
//
//desc:     An attack command has been cleared, we need to remove
//          references to it
void tacticsAttackCommandVoided(CommandToDo *command)
{
    Node *link,*templink;
    RetreatAtom *r;

#ifdef DEBUG_TACTICS
    if(!tacticsOn)
        return;
#endif

    link = universe.RetreatList.head;
    while(link != NULL)
    {
        r = (RetreatAtom *) listGetStructOfNode(link);
        if(r->fleeingfrom == command)
        {
            templink = link->next;

            listDeleteNode(link);

            link = templink;
            continue;
        }

        link = link->next;
    }
}

//Name: tacticsPutOnLookOutfor(Ship *looker, Ship *retreater)
//
//desc: Sets the 'looker' ship to be on attack lookout for
//      retreater.  If, after tunable time (in tactics.script)
//      retreater comes within looker's passive retaliation zone
//      and looker isn't doing anything (except waiting to passvie attack
//      looker will recieve full attack orders to attack retreater.
void tacticsPutOnLookOutFor(SelectCommand *lookers, Ship *retreater)
{
    AttackAtom *newattackatom;
    udword sizeofmem;

    //calculate memory needs cleverly
    sizeofmem = sizeof(AttackAtom) + sizeof(ShipPtr)*(lookers->numShips-1);

    newattackatom = (AttackAtom *)memAlloc(sizeofmem,"tacticsattackmemory",NonVolatile);

    newattackatom->retreater = retreater;
    newattackatom->expirytime = universe.totaltimeelapsed+tacticsInfo.AttackMemoryTimeOut;
    newattackatom->retreatTime = universe.totaltimeelapsed+tacticsInfo.freeRetreatTime;

    tacticsSelCopy(&newattackatom->attackerList,lookers);

    listAddNode(&universe.AttackMemory,&newattackatom->attackLink,newattackatom);

}

//This function will make all ships in selection stop being on the
//lookout for otherships <if selection is given new order>
void tacticsMakeShipsNotLookForOtherShips(SelectCommand *selection)
{
    AttackAtom  *attackatom;
    Node *node,*tempnode;
    sdword i,j;

    node = universe.AttackMemory.head;

    while(node != NULL)
    {
        attackatom = (AttackAtom *)listGetStructOfNode(node);

        for(j=0;j<selection->numShips;j++)
        {
            Ship *ship = selection->ShipPtr[j];
            for(i=0;i<attackatom->attackerList.numShips;i++)
            {
                if(ship == attackatom->attackerList.ShipPtr[i])
                {
                    //ship is looking for something
                    //remove it from list
                    attackatom->attackerList.numShips--;
                    attackatom->attackerList.ShipPtr[i] = attackatom->attackerList.ShipPtr[attackatom->attackerList.numShips];
                    if(attackatom->attackerList.numShips == 0)
                    {
                        tempnode = node->next;
                        listDeleteNode(node);   //delete node+memory
                        node = tempnode;
                        goto nextnode;
                    }
                    break;
                }

            }
        }
        node = node->next;
nextnode:
    continue;
    }
}

//returns true if at least 1 of the ships in list is on the ships target list
bool tacticsIsShipLookingForAnyOfThese(Ship *ship,SelectCommand *selection)
{
    AttackAtom  *attackatom;
    Node *node;
    sdword i,j;

    node = universe.AttackMemory.head;

    while(node != NULL)
    {
        attackatom = (AttackAtom *)listGetStructOfNode(node);

        for(i=0;i<attackatom->attackerList.numShips;i++)
        {
            if(ship == attackatom->attackerList.ShipPtr[i])
            {
                //ship is looking for something
                for(j=0;j<selection->numShips;j++)
                {
                    if(selection->ShipPtr[j] == attackatom->retreater)
                        return TRUE;
                }
            }

        }
        node = node->next;
    }
    return FALSE;
}
bool tacticsAreStrikeCraftInSelection(SelectCommand *selection)
{
    sdword i;
    for(i = 0; i< selection->numShips; i++)
    {
        if(selection->ShipPtr[i]->objtype == OBJ_ShipType)
        {
            if(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
              selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
//name : tacticsMaxDistToTarget
//
//desc: returns the distance from the furthest ship, to the target
//      Used to determine flyin distance for defenders

real32 tacticsMaxDistToTarget(SelectCommand *selection,SpaceObjRotImpTarg *target)
{
    sdword i;
    real32 maxdistsqr,newdistsqr;
    vector tempvec;
    //fix later : could do optimally by analysing formation type, and determining the
    //furthest ship and looking at just it..but for now, we brute force it

    //could use camera distacne squared maybe?

    vecSub(tempvec,selection->ShipPtr[0]->posinfo.position,target->posinfo.position);
    maxdistsqr = vecMagnitudeSquared(tempvec);


    for(i=1;i<selection->numShips;i++)
    {
        vecSub(tempvec,selection->ShipPtr[i]->posinfo.position,target->posinfo.position);
        newdistsqr = vecMagnitudeSquared(tempvec);
        if(newdistsqr > maxdistsqr)
        {
            maxdistsqr = newdistsqr;
        }
    }
    return(fsqrt(maxdistsqr));
}

bool tacticsIsShipLookingForShip(Ship *ship,Ship *target)
{
    AttackAtom  *attackatom;
    Node *node;
    sdword i;

    node = universe.AttackMemory.head;

    while(node != NULL)
    {
        attackatom = (AttackAtom *)listGetStructOfNode(node);
        if(target == attackatom->retreater)
        {
            for(i=0;i<attackatom->attackerList.numShips;i++)
            {
                if(ship == attackatom->attackerList.ShipPtr[i])
                   return TRUE;
            }
        }
        node = node->next;
    }
    return FALSE;
}

//function to perform dodging perhaps bad place...
void tacticsDodgeUpdate(Ship *ship)
{
    if(ship->isDodging)
    {
        tacticsDoDodge(ship);
    }
    else
    {
        if(ship->tacticstype != Aggressive)
        {
            //do we need to dodge?
            switch(ship->tactics_ordertype)
            {
            case MOVE_ORDERS_NO_ENEMY:
            case NO_ORDER_NO_ENEMY:
            case ATTACK_ORDERS:
                //no enemy near this ship, so no dodging
                break;
            case MOVE_ORDERS_ENEMY:
            case NO_ORDER_ENEMY:
                tacticsSetShipToDoDodge(ship);
                break;
            case ATTACK_ORDERS_ENEMY:
                //both neutral and evasive tactics dodge during attacks
                tacticsSetShipToDoDodge(ship);
                break;
            }
        }
    }
}

//function to determine if enemy ships are nearby and to determine
//a ships order type in terms of tactics functions
void tacticsUpdateOrderStatus(Ship *ship)
{
    CommandToDo *command;
    udword enemyalready;
    sdword oldOrder = ship->tactics_ordertype;
    command = getShipAndItsCommand(&universe.mainCommandLayer,ship);

    //scan to see if enemies are near by

    if((universe.univUpdateCounter & EnemyNearByCheckRate) == EnemyNearByCheckFrame)
    {
        if(command != NULL)
        {
            if(command->tacticalupdatetime < universe.totaltimeelapsed)
            {
                command->tacticalupdatetime = universe.totaltimeelapsed;
                if(tacticsAreEnemiesNearby(command->selection->ShipPtr[0],ship,tacticsInfo.EnemyNearZone))
                {
                    //enemies near by..later scan them, but not now
                    enemyalready = ORDER_TYPE_ENEMY_MASK;
                }
                else
                {
                    enemyalready = 0;
                }
            }
            else
            {
                enemyalready = command->selection->ShipPtr[0]->tactics_ordertype & ORDER_TYPE_ENEMY_MASK;
            }
        }
        else
        {
            if(tacticsAreEnemiesNearby(ship,ship,tacticsInfo.EnemyNearZone))
            {
                    //enemies near by..later scan them, but not now
                enemyalready = ORDER_TYPE_ENEMY_MASK;
            }
            else
            {
                enemyalready = 0;
            }
        }
    }
    else
    {
        enemyalready = ship->tactics_ordertype & ORDER_TYPE_ENEMY_MASK;
    }

    //set tacitcs_ordertype variable utilizing enemyalready flag set above
    if (command != NULL)
    {
        switch(command->ordertype.order)
        {
        case COMMAND_NULL:
        case COMMAND_MILITARYPARADE:
            ship->tactics_ordertype = ORDER_TYPE_NO_ORDERS + enemyalready;
            break;
        case COMMAND_MOVE:
        case COMMAND_DOCK:
        case COMMAND_LAUNCHSHIP:
        case COMMAND_HALT:
            ship->tactics_ordertype = ORDER_TYPE_MOVE_ORDERS + enemyalready;
            break;
        case COMMAND_ATTACK:
                    //ship->tactics_ordertype = ORDER_TYPE_ATTACK_ORDERS;
            ship->tactics_ordertype = ORDER_TYPE_ATTACK_ORDERS + enemyalready;
            break;
        case COMMAND_SPECIAL:
            if(ship->shiptype == RepairCorvette)
            {
                if( ((RepairCorvetteSpec *)ship->ShipSpecifics)->repairState > REPAIR_Dock1)
                    ship->tactics_ordertype = ORDER_TYPE_MOVE_ORDERS;   //force not to dodge
                else
                    ship->tactics_ordertype = ORDER_TYPE_MOVE_ORDERS + enemyalready;
            }
            else
            {
                ship->tactics_ordertype = ORDER_TYPE_MOVE_ORDERS + enemyalready;
            }
            break;
        default:
            break;
        }
        //set command leaders tacticstype to the correct one so we can reference it later when we don't
        //update the command structure enemy nearby status.
        command->selection->ShipPtr[0]->tactics_ordertype = ship->tactics_ordertype;    //possibly redundant
        if(ship->tactics_ordertype != oldOrder)
        {
            //order has changed
            if(command->ordertype.attributes & COMMAND_IS_FORMATION)
            {
                //command is in formation
                setFormationTravelVelocity(command);
            }
        }
    }
    else
    {
        //ship isn't doing anything anymore...but are enemies near by?
        //use old info until new info is pumped in
        ship->tactics_ordertype = ORDER_TYPE_NO_ORDERS + enemyalready;
    }
}

//update ships thrusterinfo based on there order type
void tacticsManeuvUpdate(Ship *ship)
{
    sdword i;
    sdword classType;
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;
    real32 manMult;

    manMult = 1.0f;
    if(shipstaticinfo->shipclass == CLASS_Fighter)
    {
        if(bitTest(ship->specialFlags,SPECIAL_SpeedBurst))
            manMult = speedBurstThrustMult;
        classType = Tactics_Fighter;
    }
    else
    {
        classType = Tactics_Corvette;
    }

    for(i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        ship->nonstatvars.thruststrength[i] = shipstaticinfo->thruststrengthstat[i]*tacticsInfo.ManeuvBonus[ship->tactics_ordertype][classType][ship->tacticstype]*manMult;
    }
    for(i=0;i<NUM_ROT_DEGOFFREEDOM;i++)
    {
        ship->nonstatvars.rotstrength[i] = shipstaticinfo->rotstrengthstat[i]*tacticsInfo.ManeuvBonus[ship->tactics_ordertype][classType][ship->tacticstype]*manMult;
    }
    for(i=0;i<NUM_TURN_TYPES;i++)
    {
        ship->nonstatvars.turnspeed[i] = shipstaticinfo->turnspeedstat[i]*tacticsInfo.ManeuvBonus[ship->tactics_ordertype][classType][ship->tacticstype]*manMult;
    }
}


//Name: tacticsUpdate
//
//Description:  Called from univupdate.c, this function performs
//              any tactics related code for this ship

void tacticsUpdate(Ship *ship)
{
    sdword oldstate;


#ifdef DEBUG_TACTICS
    if(!tacticsOn)
        return;
#endif


    //maybe only do this check every so often...


    if(ship->staticinfo->shipclass != CLASS_Fighter &&
              ship->staticinfo->shipclass != CLASS_Corvette)
    {
        //ship is a NON strike craft...
        //don't bother updating crap for it then...
        return;
    }

/***********************************/
    //strike craft ONLY proccessed beyond this point!!!!
/***********************************/

    //Update status of special Ops speed Burst
    speedBurstUpdate(ship);

    //update ships thrusterinfo based on there order type
    tacticsManeuvUpdate(ship);

    //update strike crafts dodging
    tacticsDodgeUpdate(ship);

    //update ships enemy + order status
    tacticsUpdateOrderStatus(ship);

    if(ship->formationcommand != NULL)
    {
        //ship is in formation, so set formation
        //check if we haven't processed this formation yet
        if(universe.totaltimeelapsed > (ship->formationcommand->formation.tacticsUpdate + DELAY_FORMTIGHTNESS_CHANGE))
        {
            //set formation update time
            ship->formationcommand->formation.tacticsUpdate = universe.totaltimeelapsed;
            //set formation type based on contents...should add elsewhere later...so isn't called all the time
            oldstate = ship->formationcommand->formation.tacticalState;
            ship->formationcommand->formation.tacticalState = tacticsGetFormationOptimalState(ship->formationcommand->selection);
            if((oldstate != ship->formationcommand->formation.tacticalState) && (ship->formationcommand->formation.formationtype != CUSTOM_FORMATION))
                FormationCalculateOffsets(ship->formationcommand);
        }
    }
}



//Name: TacticsGlobalUpdate()
//
//Desc: Performs any global tactics functions, such as retreat monitoring
//

void tacticsGlobalUpdate()
{
    Node *rnode,*tempnode;
    AttackAtom *attackatom;
    RetreatAtom *ratom;
    Ship *retreater, *retreatedFrom;

#ifdef DEBUG_TACTICS
    if(!tacticsOn)
        return;
#endif
    //Deal with retreats here:

    if((universe.univUpdateCounter & RetreatCheckRate) == RetreatCheckFrame)
    {
        //time to do tactics update
        vector tempvec2;
        real32 distsqr;

        rnode = universe.RetreatList.head;
        while(rnode != NULL)
        {
            ratom = (RetreatAtom *) listGetStructOfNode(rnode);
            retreater = ratom->retreater;

            //tacticsSetRetreatFrom(&tempvec, (SelectCommand *)ratom->fleeingfrom->attack);
            //vecSub(tempvec2,tempvec,retreater->posinfo.position);
            vecSub(tempvec2,retreater->posinfo.position,ratom->retreatFrom);

            distsqr = vecMagnitudeSquared(tempvec2);

            if((distsqr - ratom->distanceStartSqr) > tacticsInfo.RetreatDistanceSqr[retreater->staticinfo->shipclass])
            {
                vector retreatFromAvgPos;
                vector tmpvec3;
                real32 gotawaydistsqr;

                setAveragePosition(&retreatFromAvgPos, (SelectCommand *)ratom->fleeingfrom->selection);
                vecSub(tmpvec3,retreatFromAvgPos,retreater->posinfo.position);
                gotawaydistsqr = vecMagnitudeSquared(tmpvec3);

                if (gotawaydistsqr > tacticsInfo.RetreatGetAwayDistSqr[retreater->staticinfo->shipclass])
                {
    #ifdef bpasechn
    #ifdef DEBUG_TACTICS
                    dbgMessagef("\nRetreat Satisfied...");
    #endif
    #endif
                    //////////////////////
                    //Retreat successful speech event!

                    if(retreater->playerowner->playerIndex == universe.curPlayerIndex)
                    {
                        //local machines ships RETREATED
                        //use event num:
                        //STAT_Grp_RetreatSuccessful
                        //or STAT_AssGrp_RetreatSuccessful
                        //battlechatter
                        if(selHotKeyGroupNumberTest(retreater) == SEL_InvalidHotKey)
                        {                                   //not in a group!
                            if (battleCanChatterAtThisTime(BCE_STAT_Grp_RetreatSuccessful, retreater))
                            {
                                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_RetreatSuccessful, retreater, SOUND_EVENT_DEFAULT);
                            }
                        }
                        else
                        {
                            if (battleCanChatterAtThisTime(BCE_STAT_AssGrp_RetreatSuccessful, retreater))
                            {
                                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_AssGrp_RetreatSuccessful, retreater, selHotKeyGroupNumberTest(retreater));
                            }
                        }
                        ////
                    }
                    else
                    {
                        //local machines enemy retreated FROM
                        //local player!
                        //use event number
                        //STAT_Grp_EnemyRetreated
                        //or STAT_AssGrp_EnemyRetreated
                        retreatedFrom = ((SelectCommand *)ratom->fleeingfrom->selection)->ShipPtr[0];
                        if(selHotKeyGroupNumberTest(retreatedFrom) == SEL_InvalidHotKey)
                         {                                  //not in a group!
                             if (battleCanChatterAtThisTime(BCE_STAT_Grp_EnemyRetreated, retreatedFrom))
                             {
                                 battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_EnemyRetreated, retreatedFrom, SOUND_EVENT_DEFAULT);
                             }
                         }
                         else
                         {
                             if (battleCanChatterAtThisTime(BCE_STAT_AssGrp_EnemyRetreated, retreatedFrom))
                             {
                                 battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_AssGrp_EnemyRetreated, retreatedFrom, selHotKeyGroupNumberTest(retreatedFrom));
                             }
                         }

                    }
                    if (clRemoveShipFromSelection((SelectCommand *)ratom->fleeingfrom->attack,retreater))
                    {
                        RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)retreater,ratom->fleeingfrom);
                    }

                    //put retreater into a ship memory list...and then, we check thta list for each ship passive attacking
                    //if the retreater is in the list along with the passive attacker, we issue an attack command...
                    //as long as this updata function hasn't killed the memory due to a time out.

                    tacticsPutOnLookOutFor(ratom->fleeingfrom->selection,retreater);

                    tempnode = rnode->next;
                    tacticsRemoveRetreat(ratom);
                    rnode = tempnode;
                    continue;
                }
            }

            rnode = rnode->next;
        }
    }

    if((universe.univUpdateCounter & ATTACKMEMORY_CLEANUP_RATE) == ATTACKMEMORY_CLEANUP_FRAME)
    {
        //perform attackmemory cleanups...
        rnode = universe.AttackMemory.head;
        while(rnode != NULL)
        {
            attackatom = (AttackAtom *)listGetStructOfNode(rnode);
            if(attackatom->expirytime < universe.totaltimeelapsed)
            {
#ifdef DEBUG_TACTICS
                dbgMessagef("\nAttack Lookout timed out");
#endif
                tempnode = rnode->next;
                listDeleteNode(rnode);
                rnode = tempnode;
                continue;
            }
            rnode = rnode->next;
        }
    }
}

bool tacticsAreEnemiesNearby(Ship *leadership,Ship *thisship, real32 retaliateZone)
{
    Player *playerowner = thisship->playerowner;
    blob *thisblob = thisship->collMyBlob;
    sdword objindex = 0;
    SelectCommand *blobships = thisblob->blobShips;
    udword num = blobships->numShips;
    Ship *ship;
    vector diff;
    real32 timesincecloak;
    real32 negRetaliateZone = -retaliateZone;

    if ((thisship->shiptype == FloatingCity) || (thisship->shiptype == TargetDrone) ||
        (bitTest(thisship->specialFlags, SPECIAL_FriendlyStatus|SPECIAL_Hyperspacing)))
    {
        return FALSE;      // traders get special status, not considered enemy to anyone
    }

    while (objindex < num)
    {
        ship = blobships->ShipPtr[objindex];

        if ((allianceIsShipAlly(ship, playerowner)))
        {
            goto nextnode;      // must be enemy ship
        }
        if ((ship->shiptype == FloatingCity) || (ship->shiptype == TargetDrone) ||
            (bitTest(ship->specialFlags, SPECIAL_FriendlyStatus|SPECIAL_Hyperspacing)))
        {
            goto nextnode;      // traders get special status, not considered enemy to anyone
        }
        if (bitTest(ship->flags,SOF_Cloaked))
        {
            if(!proximityCanPlayerSeeShip(thisship->playerowner,ship))
            {
                goto nextnode;      // if ship is cloaked, and can't be seen by that ships player..
            }
        }
        timesincecloak = universe.totaltimeelapsed - ship->shipDeCloakTime;
        if(timesincecloak <= TW_SECOND_SEE_TIME)
        {
            if(timesincecloak < TW_NOONE_SEE_TIME)
            {
                //ship being considered has decloaked so recently that 'thisship' couldn't
                //possibly see it yet
                goto nextnode;
            }
            else if(timesincecloak < TW_FIRST_SEE_TIME)
            {
                //ship has decloaked so long ago that only a small percentage could see it now
                if(randombetween(0,100) > TW_FIRST_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
            else if(timesincecloak < TW_SECOND_SEE_TIME)
            {
                //ship has decloaked a fair ammount of time ago, and some more ships are likely to see it now
                if(randombetween(0,100) > TW_SECOND_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
        }
        if(bitTest(ship->flags,SOF_Disabled|SOF_Hide))
        {
            //don't attack disabled ships.
            goto nextnode;
        }

        vecSub(diff,ship->posinfo.position,leadership->posinfo.position);

        if (isBetweenExclusive(diff.x,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.y,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.z,negRetaliateZone,retaliateZone) )
        {
            return TRUE;
        }
nextnode:
        objindex++;
    }

    return FALSE;
}


void speedBurstUpdate(Ship *ship)
{
    if(bitTest(ship->specialFlags,SPECIAL_SpeedBurst))
    {
        if(ship->speedBurstTime < universe.totaltimeelapsed)
        {
            bitClear(ship->specialFlags,SPECIAL_SpeedBurst);
            bitSet(ship->specialFlags,SPECIAL_SpeedBurstCooling);
            ship->speedBurstTime = universe.totaltimeelapsed+speedBurstCoolDown;
        }
    }
    else if(bitTest(ship->specialFlags,SPECIAL_SpeedBurstCooling))
    {
        if(ship->speedBurstTime < universe.totaltimeelapsed)
        {
            bitClear(ship->specialFlags,SPECIAL_SpeedBurstCooling);
        }
    }

}

/*-----------------------------------------------------------------------------
    Name        : tacticsCheckGuardConditionsDuringAttack
    Description : returns TRUE if command needs to cancel its attack and return to guarding ship
    Inputs      : command in question
    Outputs     :
    Return      : as above
----------------------------------------------------------------------------*/
bool tacticsCheckGuardConditionsDuringAttack(CommandToDo *command)
{
    sdword i;
    real32 distSqr,tempreal;
    vector distVec,protectAv,defendAv;

    dbgAssert(command->ordertype.attributes * COMMAND_IS_PROTECTING);

    //get average position of defenders
    vecSet(defendAv,0.0f,0.0f,0.0f);
    for(i=0;i<command->selection->numShips;i++)
    {
        vecAddTo(defendAv,command->selection->ShipPtr[i]->posinfo.position);
    }
    tempreal = 1.0f/((real32)i);
    vecScalarMultiply(defendAv,defendAv,tempreal);

    //get average position of ships needing protection...
    vecSet(protectAv,0.0f,0.0f,0.0f);
    for(i=0;i<command->protect->numShips;i++)
    {
        vecAddTo(protectAv,command->protect->ShipPtr[i]->posinfo.position);
    }
    tempreal = 1.0f/((real32)i);
    vecScalarMultiply(protectAv,protectAv,tempreal);


    vecSub(distVec, protectAv, defendAv);
    distSqr = vecMagnitudeSquared(distVec);

    if(distSqr > tacticsInfo.tacticsGuardReturnToGuardingDistanceSqr)
    {
        //guarding ship has
#ifdef DEBUG_TACTICS
        dbgMessagef("\nShips guarding a ship cancelling attack and returning to guard duty.");
#endif
        return TRUE;
    }
    return FALSE;
}

real32 tacticsGetShipsMaxVelocity(Ship *ship)
{
    real32 speedmult = 1.0f;

    if(ship->objtype != OBJ_ShipType)
    {
        return(ship->staticinfo->staticheader.maxvelocity);
    }

    if (ship->shiptype == Probe)
    {
        return ProbeGetMaxVelocity(ship);
    }

    if(bitTest(ship->specialFlags,SPECIAL_Kamikaze))
    {
        //ship is in kamikaze mode
        if(bitTest(ship->specialFlags,SPECIAL_KamikazeCrazyFast))
        {
            //in a state where extra speed boost is needed
            speedmult = kamikazeSpeedBurst;
        }
    }
    if(bitTest(ship->specialFlags,SPECIAL_SpeedBurst))
    {
        speedmult *= speedBurstMaxVelocityMultiplier;
    }
    else if(bitTest(ship->specialFlags,SPECIAL_SpeedBurstCooling))
    {
        //slow deceleration
        real32 temp;
        temp = speedBurstMaxVelocityMultiplier*(ship->speedBurstTime/universe.totaltimeelapsed);
        if(temp < 1.0f)
            temp = 1.0f;
        speedmult *= temp;
    }

    if (ship->staticinfo->maxfuel != 0.0f)
    {
        // ship burns fuel
        if (ship->fuel <= 0.0f)
        {
            speedmult = OUT_OF_FUEL_VELOCITY_SLOWDOWN;
        }
    }

    if (bitTest(ship->specialFlags,SPECIAL_SinglePlayerLimitSpeed))
    {
        //Falko: removed because we want to be able to increase the maximum speed
        //       of ships as well as decreasing it.
//        if (ship->singlePlayerSpeedLimiter < ship->staticinfo->staticheader.maxvelocity)
//        {
            return ship->singlePlayerSpeedLimiter;      // deliberately override other considerations - we want this to be the final word
//        }
    }
    if(bitTest(ship->specialFlags,SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse))
    {
        SalCapCorvetteStatics *salcapcorvettestatics = ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
        return(salcapcorvettestatics->maxPushingVelocitySingle);
    }

    switch(ship->staticinfo->shipclass)
    {
    case CLASS_Fighter:
        return(ship->staticinfo->staticheader.maxvelocity*tacticsInfo.MaxVelocityBonus[ship->tactics_ordertype][Tactics_Fighter][ship->tacticstype]*speedmult);
    case CLASS_Corvette:
        return(ship->staticinfo->staticheader.maxvelocity*tacticsInfo.MaxVelocityBonus[ship->tactics_ordertype][Tactics_Corvette][ship->tacticstype]*speedmult);
    default:
        return(ship->staticinfo->staticheader.maxvelocity*speedmult);
    }

}


/*-----------------------------------------------------------------------------
    Name        : tacticsHasShipRetreatedFromShip
    Description : returns TRUE if:
                1 -shipA isn't attacking shipB

                2 - ship is listed in the has retreated list
                Need two checks because ship can be in retreated list, but
                still be getting attack :(

                will return true if shipA has retreated and shipB
                is still on lookout for shipA...so time dependant
    Inputs      :
    Outputs     :
    Return      : as above
----------------------------------------------------------------------------*/
bool tacticsHasShipRetreatedFromShip(Ship *shipA, Ship *shipB)
{
    CommandToDo *attackerCommand;
    Node *node;
    AttackAtom *aAtom;

    attackerCommand = getShipAndItsCommand(&universe.mainCommandLayer,shipB);
    if(attackerCommand != NULL)
    {
        SelectCommand sel;
        sel.numShips = 1;
        sel.ShipPtr[0] = shipA;

        if(TheseShipsAreInSelection((SelectCommand *)attackerCommand->attack,&sel))
        {
            //ship is a target, so ship hasn't retreated
            return FALSE;
        }
    }

    //shipB isn't attacking shipA, so lets see
    //if ship is in retreat
    node = universe.AttackMemory.head;
    while(node != NULL)
    {
        aAtom = (AttackAtom *) listGetStructOfNode(node);

        if(aAtom->retreater == shipA)
        {
            //ship has retreated from something
            SelectCommand sel;
            sel.numShips = 1;
            sel.ShipPtr[0] = shipB;
            if(TheseShipsAreInSelection(&aAtom->attackerList,&sel))
            {
                //ship has retreated
                return TRUE;
            }
        }

        node = node->next;
    }
    return FALSE;
}

