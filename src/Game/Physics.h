/*=============================================================================
    Name    : Physics.h
    Purpose : Definitions for physics.c

    Created 6/18/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PHYSICS_H
#define ___PHYSICS_H

#include "Types.h"
#include "Vector.h"
#include "SpaceObj.h"

/*=============================================================================
    Macros:
=============================================================================*/

#define physApplyForceVectorToObj(obj,forcevector) vecAddTo((obj)->posinfo.force,(forcevector))

/*=============================================================================
    Functions:
=============================================================================*/

void physApplyArbitraryForceToObj(SpaceObj *obj,vector *force);
void physApplyForceToObj(SpaceObj *obj,real32 force,uword transdir);
void physApplyRotToObj(SpaceObjRot *obj,real32 torque,uword rotdir);
void physUpdateObjPosVel(SpaceObj *obj,real32 phystimeelapsed);
bool physUpdateBulletPosVel(Bullet *bullet,real32 phystimeelapsed);

void physUpdateObjPosVelShip(Ship *obj,real32 phystimeelapsed);
void physUpdateObjPosVelDerelicts(Derelict *obj,real32 phystimeelapsed);
void physUpdateObjPosVelMissile(Missile *obj,real32 phystimeelapsed);
void physUpdateObjPosVelBasic(SpaceObj *obj,real32 phystimeelapsed);

#endif //___PHYSICS_H

