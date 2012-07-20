/*=============================================================================
    Name    : Attack.h
    Purpose : Definitions for Attack.c

    Created 10/7/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___ATTACK_H
#define ___ATTACK_H

#include "Types.h"
#include "SpaceObj.h"
#include "StatScript.h"

typedef struct
{
    real32 aitime;           // AI timestamp variable
    udword aidirection;      // AI miscellaneous variable
} AttackSideStep;

typedef struct
{
    real32 repositionTime;
    real32 circleRange;
    bool fullMovementFreedom;
} AttackSideStepParameters;

void attackPassiveRotate(Ship *ship,Ship *target);

void attackSideStepInit(AttackSideStep *attacksidestep);
void attackSideStep(Ship *ship,SpaceObjRotImpTarg *target,AttackSideStep *attacksidestep,AttackSideStepParameters *parameters);

void attackStraightForward(Ship *ship,SpaceObjRotImpTarg *target,real32 gunRange,real32 tooCloseRange);

void attackPassive(Ship *ship,Ship *target);

void attackSimple(Ship *ship,SpaceObjRotImpTarg *target);

/*=============================================================================
    Data:
=============================================================================*/

extern scriptStructEntry AttackSideStepParametersScriptTable[];

#endif

