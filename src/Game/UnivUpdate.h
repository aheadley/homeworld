/*=============================================================================
    Name    : univupdate.h
    Purpose : Definitions for univupdate.c

    Created 10/4/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___UNIVERSEUPDATE_H
#define ___UNIVERSEUPDATE_H

#include "Types.h"
#include "Vector.h"
#include "LinkedList.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "ShipSelect.h"
#include "Mesh.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct DeleteShip
{
    Node objlink;
    Ship *ship;
    sdword deathBy;
} DeleteShip;

typedef struct DeleteResource
{
    Node objlink;
    Resource *resource;
} DeleteResource;

typedef struct DeleteMissile
{
    Node objlink;
    Missile *missile;
    sdword deathBy;
} DeleteMissile;

typedef struct DeleteDerelict
{
    Node objlink;
    Derelict *derelict;
    sdword deathBy;
} DeleteDerelict;

// For Ship ID Decoding:
typedef struct IDToPtrTable
{
    sdword numEntries;
    SpaceObjPtr *objptrs;
} IDToPtrTable;

/*=============================================================================
    Functions:
=============================================================================*/

void univupdateReset();
void univupdateInit();
void univupdateClose();

void univupdateCloseAllObjectsAndMissionSpheres();

bool univUpdate(real32 phystimeelapsed);

void univRotateObjYaw(SpaceObjRot *robj,real32 rot);
void univRotateObjPitch(SpaceObjRot *robj,real32 rot);
void univRotateObjRoll(SpaceObjRot *robj,real32 rot);

Ship *univCreateShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built);
Ship *univAddShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built);
Asteroid *univAddAsteroid(AsteroidType asteroidtype,vector *astpos);
DustCloud *univAddDustCloud(DustCloudType dustcloudtype,vector *cloudpos);
GasCloud *univAddGasCloud(GasCloudType gascloudtype,vector *cloudpos);
Nebula *univAddNebula(NebulaType nebulatype, vector *nebpos, void *stub);
Effect *univAddEffect(Ship *ship, etgeffectstatic *stat);
//Effect *univAddEffect(Ship *ship, EffectType type);

void univFreeShipContents(Ship *ship);
void univFreeBulletContents(Bullet *bullet);
void univFreeMissileContents(Missile *missile);
void univFreeEffectContents(Effect *effect);

void univFreeResourceContents(Resource *resource);
void univFreeAsteroidContents(Asteroid *asteroid);
void univFreeNebulaContents(Nebula *nebula);
void univFreeGasCloudContents(GasCloud *gascloud);
void univFreeDustCloudContents(DustCloud *dustcloud);

void univFreeDerelictContents(Derelict *derelict);

void MakeNewGasCloudStaticInfo(GasCloud *gascloud);
void MakeNewAsteroidStaticInfo(Asteroid *asteroid);
void MakeNewDustCloudStaticInfo(DustCloud *dustcloud);

void univDeleteEffect(Effect *effect);
void univRemoveShipFromOutside(Ship *ship);
void univDeleteWipeInsideShipOutOfExistence(Ship *ship);
void univDeleteDeadShip(Ship *ship, sdword deathBy);// deletes ship, but lets part of it hang around for explosion
void univWipeShipOutOfExistence(Ship *ship);                // totally deletes ship, frees memory
void univWipeDerelictOutOfExistance(Derelict *derelict);    //totally deletes the derelict, frees memory and spawns no new effects
void univReallyDeleteThisShipRightNow(Ship *ship);      //totally deletes it immediately
void univReallyDeleteThisResourceRightNow(Resource *resource);
void univReallyDeleteThisDerelictRightNow(Derelict *derelict);
bool8 univRemoveShipFromHotkeyGroup(Ship *ship, bool8 PlayDefeat); // removes ship from hotkey group
void univHideShipFromSpheres(Ship *ship);           //hides from all mission spheres
void univUnhideJustAboutEverything(void);
void univHideJustAboutEverything(void);

void univGetResourceStatistics(sdword *resourceValue,sdword *numHarvestableResources,sdword *numAsteroid0s);
Resource *univFindNearestResource(Ship *ship,real32 volumeRadius,vector *volumePosition);

void univRemoveObjFromRenderList(SpaceObj *spaceobj);
void univAddObjToRenderListIf(SpaceObj *newobj,SpaceObj *ifobj);
void univAddObjToRenderList(SpaceObj *newobj);
bool univSpaceObjInRenderList(SpaceObj *spaceobj);

#define univRemoveObjFromMinorRenderList univRemoveObjFromRenderList
#define univSpaceObjInMinorRenderList univSpaceObjInRenderList

bool univResourceMovingTooFast(Resource *resource);

real32 univGetChecksum(sdword *numShipsInChecksum);
udword univCalcShipChecksum();


void univUpdateRenderList();
void univUpdateMinorRenderList();

Ship *univFindShipIAmInside(Ship *me);
bool univAmIInsideThisShip(Ship *me,Ship *ship);

Ship *ShipIDtoShip(ShipID shipID,bool considerInsideShips);
ResourcePtr ResourceIDtoResourcePtr(ResourceID resourceID);
MissilePtr MissileIDtoMissilePtr(MissileID missileID);
DerelictPtr DerelictIDToDerelictPtr(DerelictID derelictID);

void univSortObjects(void);

void univInitSpaceObjPosRot(SpaceObj *obj,vector *position,bool randomOrientation);
void univUpdateObjRotInfo(SpaceObjRot *robj);

void AddResourceToDeleteResourceList(Resource *resource);
void AddDerelictToDeleteDerelictList(Derelict *derelict,GunSoundType damageType);
void AddMissileToDeleteMissileList(Missile *missile,GunSoundType damageType);
void AddShipToDeleteShipList(Ship *ship,GunSoundType damageType);
void AddTargetToDeleteList(SpaceObjRotImpTarg *target,GunSoundType soundType);
void univAddToWorldList(Derelict *world);
void univRemoveFromWorldList(Derelict *world);

Missile *univAddMissile(ShipRace race);

Derelict *univAddDerelict(DerelictType derelicttype,vector *pos);
Derelict *univAddDerelictByStatInfo(DerelictType derelictType, DerelictStaticInfo *stat, vector *pos);
Derelict *univAddHyperspaceGateAsDerelict(hvector *posAndRotation);

void ObjectsCollided(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,real32 colldist,vector *distvector,real32 dist,real32 distsquared);
void ApplyDamageToCollidingObjects(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,vector *distvector,real32 dist);
bool ApplyDamageToTarget(SpaceObjRotImpTarg *target,real32 damagetaken,GunSoundType soundType,sdword damageHow,sdword playerIndex);

void univBulletCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Bullet *bullet,real32 collideLineDist,sdword collSide);
void univMissileCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Missile *missile,real32 collideLineDist,sdword collSide);

SelectCommand *getEnemiesWithinProximity(Ship *thisship,real32 retaliateZone);

void univDeleteBullet(Bullet *bullet);

#define PLAYERKILLED_DROPPEDOUT         -1
#define PLAYERKILLED_STANDARDDEATH      0
#define PLAYERKILLED_CAPTUREDSHIP       1
#define PLAYERKILLED_LOSTMISSION        2

void univKillPlayer(sdword i,sdword playerdeathtype);

void CheckPlayerWin(void);

void InitializeEngineTrails(Ship *newship);

shipbindings *univMeshBindingsDupe(ShipStaticInfo *shipstaticinfo, Ship *newShip);
void univResetNewGimbleGun(Gun *gun);

void InitializeNavLights(Ship *newship);

bool univFindBackupMothership(struct Player *player);

// fast id table lookup stuff
void univInitFastNetworkIDLookups(void);
void univCloseFastNetworkIDLookups(void);
void univResetFastNetworkIDLookups(void);
void IDToPtrTableAdd(IDToPtrTable *table,uword ID,SpaceObj *obj);
// fast id table save stuff
void LoadIDToPtrTable(IDToPtrTable *table);
void SaveIDToPtrTable(IDToPtrTable *table);

void ApplyCareenRotationDirectly(Ship *ship);

bool univObjectOutsideWorld(SpaceObj *obj);

void univKillOtherPlayersIfDead(Ship *ship);


/*=============================================================================
    Data:
=============================================================================*/

extern udword NUM_STARS;
extern udword NUM_BIG_STARS;

extern vector defaultshipupvector;
extern vector defaultshiprightvector;
extern vector defaultshipheadingvector;
extern matrix defaultshipmatrix;

extern IDToPtrTable ShipIDToPtr;
extern IDToPtrTable ResourceIDToPtr;
extern IDToPtrTable DerelictIDToPtr;
extern IDToPtrTable MissileIDToPtr;

#endif

