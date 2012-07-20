/*=============================================================================
    Name    : collision.h
    Purpose : Definitions for collision.c

    Created 9/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COLLISION_H
#define ___COLLISION_H

#include "Types.h"
#include "SpaceObj.h"
#include "Blobs.h"

real32 RangeToTarget(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory);
real32 RangeToTargetGivenDist(Ship *ship,SpaceObjRotImpTarg *target,real32 dist);
void collUpdateCollRectangle(SpaceObjRotImp *irobj);
void collDrawCollisionInfo(SpaceObjRotImp *irobj);
void collZeroRectInfo(StaticCollInfo *staticCollInfo);
real32 collCheckRectLine(SpaceObjRotImp *obj1,vector *univpoint,vector *univdir,real32 linelength,sdword *collSide);
bool collCheckRectPoint(SpaceObjRotImp *obj1,vector *point);
bool collCheckRectInRect(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2);
bool collCheckLineOfSight(Ship* source, Ship* target, vector* sourcePosition, vector* direction);

#define collAddSpaceObjToCollBlobs(obj) bobAddObjToNearestBlob(&universe.collBlobList,obj)
#define collAddSpaceObjToSpecificBlob(tblob,tobj) bobAddObjToSpecificBlob(tblob,tobj)
//void collAddSpaceObjToCollBlobs(SpaceObj *obj);

void collUpdateCollBlobs(void);
void collUpdateObjsInCollBlobs(void);

void collCheckAllBumpCollisions(void);
void collCheckAllBulletMissileCollisions(void);

#define COLLISION_CHECK_STATS

/*=============================================================================
    Data:
=============================================================================*/
#ifdef COLLISION_CHECK_STATS

extern udword shipsavoidingstuff;
extern udword shipsavoidedwalks;
extern udword shipsavoidedchecks;

extern udword shipshipwalks;
extern udword shipresourcewalks;
extern udword shipderelictwalks;
extern udword shipshipchecks;
extern udword shipresourcechecks;
extern udword shipderelictchecks;

extern udword bulletwalks;
extern udword bulletchecks;

extern udword missilewalks;
extern udword missilechecks;

extern udword minewalks;
extern udword minechecks;

#endif

#endif

