/*=============================================================================
    Name    : Gun.h
    Purpose : Definitions for Gun.c

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GUN_H
#define ___GUN_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

#ifdef HW_Release

#define GUN_TUNE_MODE       0
#define GUN_VERBOSE_LEVEL   0

#else

#define GUN_TUNE_MODE       1
#define GUN_VERBOSE_LEVEL   1

#endif

#define GUN_RecoilTableLength   255
#define GUN_RecoilTime    0.05f
#define GUN_OneOverTime   20.0f
#define ONEPLUS_HIGH_TIME   1.05f

/*=============================================================================
    Functions:
=============================================================================*/

bool gunCanShoot(Ship *ship, Gun *gun);
bool gunOrientGimbleGun(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target);
void missileShoot(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target);
void gunShoot(Ship *ship,Gun *gun, SpaceObjRotImpTarg *target);
bool gunShootGunsAtTarget(Ship *ship,SpaceObjRotImpTarg *target,real32 range,vector *trajectory);
bool gunShootGunsAtMultipleTargets(Ship *ship);

bool gunMatrixUpdate(udword flags, hmatrix *startMatrix, hmatrix *matrix, void *data, sdword ID);

#if RND_VISUALIZATION
void gunDrawGunInfo(Ship *ship);
#endif

//gun location in world code for silly sound boyez
void gunGetGunPositionInWorld(vector *positionInWorldCoordSys,matrix *coordsys,Gun *gun);

//compute firepower
real32 gunFirePower(GunStatic *gunStatic, TacticsType tactics, real32 *fireTime);
real32 gunShipFirePower(ShipStaticInfo *info, TacticsType tactics);

void gunStartup(void);
void gunShutdown(void);
#if GUN_TUNE_MODE
void gunTuneGun(Ship *ship);
#endif

/*=============================================================================
    Macros:
=============================================================================*/
// only valid for GUN_MissileLauncher
#define gunHasMissiles(gun) ((gun)->numMissiles > 0)

/*=============================================================================
    Variables:
=============================================================================*/

#if GUN_TUNE_MODE
extern bool gunTuningMode;
extern sdword tuningGun;
#endif

#endif //___GUN_H

