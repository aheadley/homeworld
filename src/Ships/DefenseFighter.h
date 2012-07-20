/*=============================================================================
    Name    : DefenseFighter.h
    Purpose : Definitions for DefenseFighter

    Created 1/27/1998 by Bryce Pasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DEFENSE_FIGHTER_H
#define ___DEFENSE_FIGHTER_H

#include "Types.h"
#include "SpaceObj.h"
#include "FlightMan.h"

/*=============================================================================
    Types:
=============================================================================*/
typedef struct
{
    udword NumTargetsCanAttack;
    real32 CoolDownTimePerLaser;
    real32 DamageReductionLow;
    real32 DamageReductionHigh;
    udword DamageRate;
    udword RangeCheckRate;
    sdword TargetOwnBullets;
    sdword MultipleTargettingofSingleBullet;
    real32 max_rot_speed;
    real32 rotate_time;
    real32 rotate_recover_time;
    FlightManProb flightmanProb[NUM_TACTICS_TYPES][NUM_CLASSES+1];
} DefenseFighterStatics;

typedef struct
{
    Node bulletnode;
    Bullet *bullet;
    Bullet *laser;
    real32 CoolDownTime;
    bool CoolDown;
    bool LaserDead;
} DefenseStruct;

typedef struct
{
    LinkedList DefenseList;
    sdword DefenseFighterCanNowRotate;
    real32 rotate_time_counter;
} DefenseFighterSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader DefenseFighterHeader;

void DefenseFighterReportBullet(Ship *ship, Bullet *bullet);
void defenseFighterAdjustLaser(Bullet *bullet);
void DefenseFighterBulletRemoval(Bullet *bullettogoByeBye);
//void DefenseFighterDied(Ship *ship);

#endif //___DEFENSE_FIGHTER_H

