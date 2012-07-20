/*=============================================================================
    Name    : GenericInterceptor.h
    Purpose : Definitions for Generic Interceptor

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GENERIC_INTERCEPTOR_H
#define ___GENERIC_INTERCEPTOR_H

#include "Types.h"
#include "SpaceObj.h"
#include "FlightMan.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;              // AI timestamp for when we went into kill mode
    vector aivec;                   // AI vector when breaking
    bool16 aiAIPflightman;          // AI boolean indicating if attack in progress (AIP) flight maneuver has been chosen
    uword activeGun;
} GenericInterceptorSpec;

typedef struct                      //Inherited From GenericInterceptorSpec
{
    real32 lasttimefired;
    real32 aitimekill;              // AI timestamp for when we went into kill mode
    vector aivec;                   // AI vector when breaking
    bool16 aiAIPflightman;          // AI boolean indicating if attack in progress (AIP) flight maneuver has been chosen
    uword activeGun;
    real32 CloakingStatus;          // Gradient value for cloaking and decloaking state
    bool CloakLowWarning;
    bool ReCloak;
    real32 ReCloakTime;
} CloakedFighterSpec;

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;              // AI timestamp for when we went into kill mode
    vector aivec;                   // AI vector when breaking
    bool16 aiAIPflightman;          // AI boolean indicating if attack in progress (AIP) flight maneuver has been chosen
    uword activeGun;
    vector rallypoint;
    vector orig_point;
    real32 angle;
    sdword bombingdelay;
} AttackBomberSpec;

typedef struct
{
    real32 firetime;
    real32 maxFlyAwayDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxFlyAwayDistSquared[NUM_TACTICS_TYPES][NUM_CLASSES+1];    // use NUM_CLASSES entry as DefaultClass (so array len = NUM_CLASSES+1)
    real32 breakRange[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 flyPastDist[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 triggerHappy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 faceTargetAccuracy[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    real32 maxAttackTime[NUM_TACTICS_TYPES][NUM_CLASSES+1];
    FlightManProb flightmanProb[NUM_TACTICS_TYPES][NUM_CLASSES+1];
} GenericInterceptorStatics;


/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader GenericInterceptorHeader;
extern CustShipHeader CloakedFighterHeader;
extern CustShipHeader AttackBomberHeader;
extern CustShipHeader TargetDroneHeader;

// public GenericInterceptor functions used by flight maneuvers for interceptors
void GenericInterceptorStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo);
void GenericInterceptorInit(Ship *ship);
void GenericInterceptorFire(Ship *ship,SpaceObjRotImpTarg *target);
void GenericInterceptorPassiveAttack(Ship *ship,Ship *target,bool rotate);
void GenericInterceptorAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist);
void GenericInterceptorPassiveAttack(Ship *ship,Ship *target,bool rotate);
FlightManProb *GenericInterceptorGetFlightManProb(Ship *ship,SpaceObjRotImpTarg *target);
void SpawnCloakingEffect(Ship *ship, etglod *etgLOD);

bool InterceptorInRange(Ship *ship,SpaceObjRotImpTarg *target);
bool GenericInterceptorCanFire(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory,real32 triggerHappy);

//need access to this variables globally to determine ships attackstates

#define STATE_INIT              0
#define STATE_APPROACH          1
#define STATE_KILL              2
#define STATE_BREAK             3
#define STATE_TURNAROUND        4
#define STATE_REAPPROACH        5

#define STATE_WHIPSTRAFE1       10
#define STATE_WHIPSTRAFE2       11
#define STATE_BARRELROLL_OUT    12
#define STATE_START_BOMBING_RUN 13
#define STATE_BOMBING_DIVE      14


#define isShipInterceptor(ship) (((ShipStaticInfo *)((ship)->staticinfo))->custshipheader.CustShipAttack == GenericInterceptorAttack)
#define isShipStaticInterceptor(shipstatic) ((shipstatic)->custshipheader.CustShipAttack == GenericInterceptorAttack)
//#define isShipInterceptor(ship) (((ShipStaticInfo *)((ship)->staticinfo))->shipclass == CLASS_Fighter)
//#define isShipStaticInterceptor(shipstatic) ((shipstatic)->shipclass == CLASS_Fighter)

#endif //___GENERIC_INTERCEPTOR_H

