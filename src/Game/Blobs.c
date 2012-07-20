/*=============================================================================
    Name    : Blobs.c
    Purpose : Code to create and maintain lists of 'blobs' which are spheres
                encapsulating objects in the game world.

    Created 2/9/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Debug.h"
#include "Memory.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "FastMath.h"
#include "Sensors.h"
#include "NetCheck.h"
#include "Battle.h"
#include "SoundEvent.h"
#include "GravWellGenerator.h"
#include "Blobs.h"
#include "Alliance.h"
#include "TimeoutTimer.h"
#include "SinglePlayer.h"

#ifdef BOB_STATS

BobStats bobStats = { 0,0,0,0,0,0,0,0,0,0,0 };

void bobListUpdateStatsInitFunc(bool subBlobs)
{
    memset(&bobStats,0,sizeof(struct BobStats));
    bobStats.subBlobs = subBlobs;
    GetRawTime(&bobStats.timeStart);
}

void bobListUpdateStatsCloseFunc()
{
    GetRawTime(&bobStats.timeStop);
    bobStats.timeDuration = bobStats.timeStop - bobStats.timeStart;
    bobStats.statsValid = TRUE;
}

#define bobListUpdateStatsInit(x) bobListUpdateStatsInitFunc(x)
#define bobListUpdateStatsClose() bobListUpdateStatsCloseFunc()

#define bobStatsPass()    bobStats.numPasses++
#define bobStatsWalk()    bobStats.numWalks++;
#define bobStatsCheck()   bobStats.numChecks++;
#define bobStatsTrivialReject() bobStats.trivialRejects++;
#define bobStatsInitialBlobs(x) bobStats.initialBlobs = (x)
#define bobStatsFinalBlobs(x)   bobStats.finalBlobs = (x)
#define bobStatsCombine()       bobStats.combine++;

#else

#define bobListUpdateStatsInit(x)
#define bobListUpdateStatsClose()

#define bobStatsPass()
#define bobStatsWalk()
#define bobStatsCheck()
#define bobStatsTrivialReject()
#define bobStatsInitialBlobs(x)
#define bobStatsFinalBlobs(x)
#define bobStatsCombine()

#endif

/*=============================================================================
    Data:
=============================================================================*/

static BlobProperties *BlobPropertiesPtr;

real32 bobUpdateRadiusThreshold = BOB_UpdateRadiusThreshold;
real32 bobUpdateCentreThreshold = BOB_UpdateCentreThreshold;
//vector bobOriginVector = {0.0f, 0.0f, 0.0f};

/*=============================================================================
    Functions:
=============================================================================*/

#define BLOBOBJ_BATCH       10

void blobFree(blob *thisBlob);
void blobFreeContents(blob *thisBlob);

extern real32 SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6;
extern BlobProperties collBlobProperties;

/*-----------------------------------------------------------------------------
    Name        : bobResetProperties
    Description : resets blob properties to their defaults
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobResetProperties()
{
    collBlobProperties.bobBiggestRadius = collBlobProperties.bobBiggestRadiusDefault;
}

/*-----------------------------------------------------------------------------
    Name        : bobInitProperties
    Description : initializes blob properties to allow for special case properties for certain single player levels
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobInitProperties()
{
    bobResetProperties();

    if (singlePlayerGame)
    {
        if (singlePlayerGameInfo.currentMission == 6)
        {
            collBlobProperties.bobBiggestRadius = SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : bobFindNearestBlobToObject
    Description : finds the nearest blob to the obj
    Inputs      : list of blobs to search, obj
    Outputs     :
    Return      : returns nearest blob to the obj
----------------------------------------------------------------------------*/
blob *bobFindNearestBlobToObject(LinkedList *list,SpaceObj *obj,real32 *returndistsqr)
{
    vector distvec;
    real32 distsqr;
    Node *blobnode = list->head;
    blob *thisBlob;
    real32 mindistsqr = REALlyBig;
    blob *closestBlob = NULL;

    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        vecSub(distvec,obj->posinfo.position,thisBlob->centre);
        distsqr = vecMagnitudeSquared(distvec);

        if (distsqr < mindistsqr)
        {
            closestBlob = thisBlob;
            mindistsqr = distsqr;
        }

        blobnode = blobnode->next;
    }

    *returndistsqr = mindistsqr;
    return closestBlob;
}

/*-----------------------------------------------------------------------------
    Name        : bobListSortCallback
    Description : Compare two
    Inputs      :
    Outputs     :
    Return      : TRUE if firststruct > secondstruct (ascending order)
----------------------------------------------------------------------------*/
bool bobListSortCallback(void *firststruct,void *secondstruct)
{
    if (((blob *)(firststruct))->sortDistance > ((blob *)(secondstruct))->sortDistance)
    {
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : CompareSpaceObjCollOptimizeDistCB
    Description : Callback function for comparing collOptimzeDist of two spaceobj's
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int CompareSpaceObjCollOptimizeDistCB(const void *arg1,const void *arg2)
{
    SpaceObj *sobj1 = *((SpaceObj **)arg1);
    SpaceObj *sobj2 = *((SpaceObj **)arg2);

    if (sobj1->collOptimizeDist > sobj2->collOptimizeDist)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

#define AssignIfBigger(var,val) \
    if ((val) > (var))          \
    {                           \
        (var) = (val);          \
    }



/*-----------------------------------------------------------------------------
    Name        : FillInCollisionBlobSpecificLists
    Description : Fills in collision blob specific lists (Ships, Targets, etc.) based on blobObjects list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FillInCollisionBlobSpecificLists(blob *thisBlob)
{
    sdword numShips = 0, numResources = 0, numDerelicts = 0, numBullets = 0;
    sdword numMissileMissiles = 0, numMissileMines = 0, numSmallTargets = 0, numBigTargets = 0;
    sdword numBigShips = 0, numSmallShips = 0;
    sdword i;
    SpaceObjSelection *blobObjects = thisBlob->blobObjects;
    SpaceObj *spaceobj;

    thisBlob->blobMaxShipCollSphereSize = 0.0f;
    thisBlob->blobMaxBigShipCollSphereSize = 0.0f;
    thisBlob->blobMaxSmallShipCollSphereSize = 0.0f;
    thisBlob->blobMaxSTargetCollSphereSize = 0.0f;
    thisBlob->blobMaxBTargetCollSphereSize = 0.0f;
    thisBlob->blobMaxResourceCollSphereSize = 0.0f;
    thisBlob->blobMaxDerelictCollSphereSize = 0.0f;

    for (i=0;i<blobObjects->numSpaceObjs;i++)
    {
        spaceobj = blobObjects->SpaceObjPtr[i];

        if (spaceobj->flags & SOF_Targetable)
        {
            if(spaceobj->objtype != OBJ_MissileType)
            {
                real32 originalcollspheresize = ((TargetPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize;

                if (originalcollspheresize >= BIG_TARGET_SIZE)
                {
                    thisBlob->blobBigTargets->TargetPtr[numBigTargets++] = (TargetPtr)spaceobj;
                    AssignIfBigger(thisBlob->blobMaxBTargetCollSphereSize,originalcollspheresize)
                }
                else
                {
                    //don't add missiles because they're bitches
                    thisBlob->blobSmallTargets->TargetPtr[numSmallTargets++] = (TargetPtr)spaceobj;
                    AssignIfBigger(thisBlob->blobMaxSTargetCollSphereSize,originalcollspheresize)
                }
            }
        }

        switch (spaceobj->objtype)
        {
            case OBJ_ShipType:
            {
                real32 originalcollspheresize = ((ShipPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize;

                if (originalcollspheresize >= BIG_SHIP_SIZE)
                {
                    thisBlob->blobBigShips->ShipPtr[numBigShips++] = (ShipPtr)spaceobj;
                    AssignIfBigger(thisBlob->blobMaxBigShipCollSphereSize,originalcollspheresize)
                }
                else
                {
                    thisBlob->blobSmallShips->ShipPtr[numSmallShips++] = (ShipPtr)spaceobj;
                    AssignIfBigger(thisBlob->blobMaxSmallShipCollSphereSize,originalcollspheresize)
                }

                thisBlob->blobShips->ShipPtr[numShips++] = (ShipPtr)spaceobj;
                AssignIfBigger(thisBlob->blobMaxShipCollSphereSize,((ShipPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.collspheresize)
            }
                break;

            case OBJ_BulletType:
                thisBlob->blobBullets->BulletPtr[numBullets++] = (BulletPtr)spaceobj;
                break;

            case OBJ_AsteroidType:
            case OBJ_NebulaType:
            case OBJ_GasType:
            case OBJ_DustType:
                thisBlob->blobResources->ResourcePtr[numResources++] = (ResourcePtr)spaceobj;
                AssignIfBigger(thisBlob->blobMaxResourceCollSphereSize,((ResourcePtr)spaceobj)->staticinfo->staticheader.staticCollInfo.collspheresize)
                break;

            case OBJ_DerelictType:
                thisBlob->blobDerelicts->DerelictPtr[numDerelicts++] = (DerelictPtr)spaceobj;
                AssignIfBigger(thisBlob->blobMaxDerelictCollSphereSize,((DerelictPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.collspheresize)
                break;

            case OBJ_MissileType:
                if (((MissilePtr)spaceobj)->missileType == MISSILE_Mine)
                {
                    thisBlob->blobMissileMines->MissilePtr[numMissileMines++] = (MissilePtr)spaceobj;
                }
                else
                {
                    thisBlob->blobMissileMissiles->MissilePtr[numMissileMissiles++] = (MissilePtr)spaceobj;
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
    }

    dbgAssert(thisBlob->blobShips->numShips == numShips);
    dbgAssert(thisBlob->blobBigShips->numShips == numBigShips);
    dbgAssert(thisBlob->blobSmallShips->numShips == numSmallShips);
    dbgAssert(thisBlob->blobBigTargets->numTargets == numBigTargets);
    dbgAssert(thisBlob->blobSmallTargets->numTargets == numSmallTargets);
    dbgAssert(thisBlob->blobResources->numResources == numResources);
    dbgAssert(thisBlob->blobDerelicts->numDerelicts == numDerelicts);
    dbgAssert(thisBlob->blobBullets->numBullets == numBullets);
    dbgAssert(thisBlob->blobMissileMissiles->numMissiles == numMissileMissiles);
    dbgAssert(thisBlob->blobMissileMines->numMissiles == numMissileMines);
}

/*-----------------------------------------------------------------------------
    Name        : bobUpdateObjsInBlobCollInfo
    Description : updates "objects in blob" collision info
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobUpdateObjsInBlobCollInfo(blob *thisBlob)
{
    SpaceObjSelection *blobObjects = thisBlob->blobObjects;
    sdword i;
    SpaceObj *spaceobj;
    vector distvec;

    // calculate collOptimizeDist for each spaceobj relative to blob centre
    thisBlob->flags &= ~(BTM_PieceOfTheAction);             //clear out the "action flags"
    for (i=0;i<blobObjects->numSpaceObjs;i++)
    {
        spaceobj = blobObjects->SpaceObjPtr[i];

        dbgAssert((spaceobj->flags & SOF_Dead) == 0);
#if BOB_ERROR_CHECKING
        if (spaceobj->objtype == OBJ_AsteroidType)
        {
            dbgAssert(((Asteroid *)spaceobj)->asteroidtype != Asteroid0);    // don't put Asteroid0 in for speed
        }
        else
#endif
        if (spaceobj->objtype == OBJ_ShipType)
        {                                                   //keep the "action flags" up-to-date
            if (((Ship *)spaceobj)->recentlyAttacked != 0)
            {
                bitSet(thisBlob->flags, BTF_GettingAttacked);
            }
            if (((Ship *)spaceobj)->shipisattacking)
            {
                bitSet(thisBlob->flags, BTF_Attacking);
            }
        }

        vecSub(distvec,spaceobj->posinfo.position,thisBlob->centre);
        spaceobj->collOptimizeDist = fsqrt(vecMagnitudeSquared(distvec));
    }

    qsort(&blobObjects->SpaceObjPtr[0],blobObjects->numSpaceObjs,sizeof(SpaceObj *),CompareSpaceObjCollOptimizeDistCB);

    FillInCollisionBlobSpecificLists(thisBlob);
    blobAnalVerify(thisBlob);
}

/*-----------------------------------------------------------------------------
    Name        : bobUpdateExtraCollBobInfo
    Description : updates extra collision blob info for collision blobs only
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobUpdateExtraCollBobInfo(LinkedList *list)
{
    SpaceObj *spaceobj;
    vector distvec;
    sdword i;
    Node *blobnode = list->head;
    blob *thisBlob;
    sdword numShips,numResources,numDerelicts,numBullets,numMissileMissiles,numMissileMines,numSmallTargets,numBigTargets;
    sdword numBigShips,numSmallShips;
    SpaceObjSelection *blobObjects;

    // for each blob,
    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);
        blobnode = blobnode->next;

        blobObjects = thisBlob->blobObjects;

        // calculate collOptimizeDist for each spaceobj relative to blob centre
        for (i=0;i<blobObjects->numSpaceObjs;i++)
        {
            spaceobj = blobObjects->SpaceObjPtr[i];

            vecSub(distvec,spaceobj->posinfo.position,thisBlob->centre);
            spaceobj->collOptimizeDist = fsqrt(vecMagnitudeSquared(distvec));
            spaceobj->collMyBlob = thisBlob;
        }

        // sort blob spaceobjs based on distance from centre of blob (collOptimizeDist)
        qsort(&blobObjects->SpaceObjPtr[0],blobObjects->numSpaceObjs,sizeof(SpaceObj *),CompareSpaceObjCollOptimizeDistCB);


        if (bitTest(thisBlob->flags, BTF_DontUpdate))
        {                                                   //don't re-process this blob if it's good enough
            bitClear(thisBlob->flags, BTF_DontUpdate);
            continue;
        }

        // create blob ship, resource, derelict, bullet, missile, and target lists for efficiency

        // 1st pass: figure out # of ship, resource, derelict, bullet, missile, and targets
        numShips = 0, numResources = 0, numDerelicts = 0, numBullets = 0;
        numMissileMissiles = 0, numMissileMines = 0, numSmallTargets = 0, numBigTargets = 0;
        numBigShips = 0, numSmallShips = 0;
        for (i=0;i<blobObjects->numSpaceObjs;i++)
        {
            spaceobj = blobObjects->SpaceObjPtr[i];

            if (spaceobj->flags & SOF_Targetable)
            {
                if(spaceobj->objtype != OBJ_MissileType)
                {
                    if (((TargetPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_TARGET_SIZE)
                    {
                        numBigTargets++;
                    }
                    else
                    {
                        numSmallTargets++;
                    }
                }
            }

            switch (spaceobj->objtype)
            {
                case OBJ_ShipType:
                    if (((ShipPtr)spaceobj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_SHIP_SIZE)
                    {
                        numBigShips++;
                    }
                    else
                    {
                        numSmallShips++;
                    }
                    numShips++;
                    break;

                case OBJ_BulletType:
                    numBullets++;
                    break;

                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    numResources++;
                    break;

                case OBJ_DerelictType:
                    numDerelicts++;
                    break;

                case OBJ_MissileType:
                    if (((MissilePtr)spaceobj)->missileType == MISSILE_Mine)
                    {
                        numMissileMines++;
                    }
                    else
                    {
                        numMissileMissiles++;
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }

        // 2nd pass: construct ship, resource, derelict, bullet, missile, and target lists

        thisBlob->maxNumShips = numShips + BLOBOBJ_BATCH;
        thisBlob->blobShips = memAlloc(sizeofSelectCommand(thisBlob->maxNumShips),"bs(blobships)",Pyrophoric);
        thisBlob->blobShips->numShips = numShips;

        thisBlob->maxNumBigShips = numBigShips + BLOBOBJ_BATCH;
        thisBlob->blobBigShips = memAlloc(sizeofSelectCommand(thisBlob->maxNumBigShips),"bsb(blobbships)",Pyrophoric);
        thisBlob->blobBigShips->numShips = numBigShips;

        thisBlob->maxNumSmallShips = numSmallShips + BLOBOBJ_BATCH;
        thisBlob->blobSmallShips = memAlloc(sizeofSelectCommand(thisBlob->maxNumSmallShips),"bss(blobsships)",Pyrophoric);
        thisBlob->blobSmallShips->numShips = numSmallShips;

        thisBlob->maxNumSmallTargets = numSmallTargets + BLOBOBJ_BATCH;
        thisBlob->blobSmallTargets = memAlloc(sizeofSelectCommand(thisBlob->maxNumSmallTargets),"bt(blobstargets)",Pyrophoric);
        thisBlob->blobSmallTargets->numTargets = numSmallTargets;

        thisBlob->maxNumBigTargets = numBigTargets + BLOBOBJ_BATCH;
        thisBlob->blobBigTargets = memAlloc(sizeofSelectCommand(thisBlob->maxNumBigTargets),"bt(blobbtargets)",Pyrophoric);
        thisBlob->blobBigTargets->numTargets = numBigTargets;

        thisBlob->maxNumResources = numResources + BLOBOBJ_BATCH;
        thisBlob->blobResources = memAlloc(sizeofSelectCommand(thisBlob->maxNumResources),"br(blobresources)",Pyrophoric);
        thisBlob->blobResources->numResources = numResources;

        thisBlob->maxNumDerelicts = numDerelicts + BLOBOBJ_BATCH;
        thisBlob->blobDerelicts = memAlloc(sizeofSelectCommand(thisBlob->maxNumDerelicts),"bd(blobderelicts)",Pyrophoric);
        thisBlob->blobDerelicts->numDerelicts = numDerelicts;

        thisBlob->maxNumBullets = numBullets + BLOBOBJ_BATCH;
        thisBlob->blobBullets = memAlloc(sizeofSelectCommand(thisBlob->maxNumBullets),"bb(blobbullets)",Pyrophoric);
        thisBlob->blobBullets->numBullets = numBullets;

        thisBlob->maxNumMissileMissiles = numMissileMissiles + BLOBOBJ_BATCH;
        thisBlob->blobMissileMissiles = memAlloc(sizeofSelectCommand(thisBlob->maxNumMissileMissiles),"bm(blobmissiles)",Pyrophoric);
        thisBlob->blobMissileMissiles->numMissiles = numMissileMissiles;

        thisBlob->maxNumMissileMines = numMissileMines + BLOBOBJ_BATCH;
        thisBlob->blobMissileMines = memAlloc(sizeofSelectCommand(thisBlob->maxNumMissileMines),"bm(blobmines)",Pyrophoric);
        thisBlob->blobMissileMines->numMissiles = numMissileMines;

        thisBlob->maxNumObjects = blobObjects->numSpaceObjs;

        FillInCollisionBlobSpecificLists(thisBlob);

        blobAnalVerify(thisBlob);
    }
}

/*-----------------------------------------------------------------------------
    Name        : bobSubBlobListCreate
    Description : Create a 'sub-blob' list inside a bigger blob.
    Inputs      : blobProperties - same as for bobListCreate
                  list - same as for bobListCreate
                  superBlob - the blob which contains a list of objects to use
                  criteria - callback function to see if an object is suitable
                    for the mini-blob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobSubBlobListCreate(BlobProperties *blobProperties, LinkedList *list, blob *superBlob, subblobcallback criteria)
{
    blob *preallocBlobs;
    SpaceObjSelection *preallocObjSelection;
    SpaceObjSelection *newObjSelection;
    udword usePreallocBlob = 0;
    SpaceObj *spaceobj;
    blob *newBlob;
    blob *tblob;
    Node *node, *nextnode;
    sdword index;
    real32 mass;
    vector distance;

    dbgAssert(blobProperties);
    BlobPropertiesPtr = blobProperties;
    listInit(list);                                         //init the linked list

    if (superBlob->blobObjects->numSpaceObjs == 0)
    {                                                       //if there's nothing in the blob
        return;                                             //all done
    }

    //first pass: create a sphere structure for each ship
    preallocBlobs = memAlloc(bobBlobSize(1)*superBlob->blobObjects->numSpaceObjs,"MassAllocBlobs",0);
    preallocObjSelection = memAlloc(sizeof(SelectCommand)*superBlob->blobObjects->numSpaceObjs,"MassAllocSel",0);

    for (index = 0; index < superBlob->blobObjects->numSpaceObjs; index++)
    {                                                       //for all blobs in this big fat blob
        spaceobj = superBlob->blobObjects->SpaceObjPtr[index];
        if (criteria(superBlob, spaceobj))
        {                                                   //if this object satisfies some criteria
            switch (spaceobj->objtype)
            {
                case OBJ_ShipType:
                    if (((Ship *)spaceobj)->shiptype == Drone)
                    {
                        continue;
                    }
                    mass = ((Ship *)spaceobj)->staticinfo->staticheader.mass;
                    break;
                case OBJ_BulletType:
                    continue;                                   //no sphere for you!
                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    mass = ((Resource *)spaceobj)->staticinfo->staticheader.mass;
                    break;
                case OBJ_DerelictType:
                    if (((Derelict *)spaceobj)->staticinfo->worldRender)
                    {                                           //don't include worlds in the blobs.  They wouldn't fit anyhow.
                        continue;
                    }
                    mass = ((Derelict *)spaceobj)->staticinfo->staticheader.mass;
                    break;
                case OBJ_EffectType:
                case OBJ_MissileType:
                    continue;                                   //no sphere for you!
            }
            newBlob = &preallocBlobs[usePreallocBlob];
            newBlob->flags = 0;
            bitClear(newBlob->flags, BTF_FreeThisBlob);
            bitClear(newBlob->flags, BTF_FreeBlobObjects);
            newBlob->centre = spaceobj->posinfo.position;
            newBlob->radius = BlobPropertiesPtr->bobStartSphereSize;
            newBlob->volume = sphereVolume(newBlob->radius);
            newBlob->oneOverVolume = 1.0f / newBlob->volume;

            newBlob->totalMass = mass;

            newBlob->RUs = 0;

    #ifndef HW_Release
//            newBlob->debugFlag = 0;
    #endif
            newBlob->blobShips = NULL;
            newBlob->blobBigShips = NULL;
            newBlob->blobSmallShips = NULL;
            newBlob->blobSmallTargets = NULL;
            newBlob->blobBigTargets = NULL;
            newBlob->blobResources = NULL;
            newBlob->blobDerelicts = NULL;
            newBlob->blobBullets = NULL;
            newBlob->blobMissileMissiles = NULL;
            newBlob->blobMissileMines = NULL;

            newBlob->blobObjects = &preallocObjSelection[usePreallocBlob++];
            newBlob->blobObjects->numSpaceObjs = 1;
            newBlob->blobObjects->SpaceObjPtr[0] = spaceobj;
            newBlob->subBlobs.num = BIT31;                  //flag the sub-blob list not yet created

            vecSub(distance, superBlob->centre, spaceobj->posinfo.position);
            newBlob->sortDistance = vecMagnitudeSquared(distance);
            newBlob->sqrtSortDistance = 0.0f;
            listAddNode(list, &newBlob->node, newBlob);
            newBlob->subBlobs.num = BIT31;                  //flag the sub-blob list not yet created

        }
    }
    //second pass: sort the spheres based on distance from the mothership
    listMergeSortGeneral(list, bobListSortCallback);

    //final pass: combine the spheres into a neat, maageable list
    bobListUpdateStatsInit(TRUE);
    bobListUpdate(list);
    bobListUpdateStatsClose();

    // at this point we have a list of all the blobs we want.  But they may be in the preallocBlobs chunk.
    // Reallocate any that are in the preallocBlobs chunk.
    // Same goes for blobObjects which may be in the preallocSelectCommands chunk.

    node = list->head;
    while (node != NULL)
    {
        tblob = (blob *)listGetStructOfNode(node);
        nextnode = node->next;

        if (!bitTest(tblob->flags, BTF_FreeBlobObjects))
        {
            if (tblob->blobObjects != NULL)
            {
                dbgAssert(tblob->blobObjects->numSpaceObjs == 1);
                newObjSelection = memAlloc(sizeofSelectCommand(1),"nbo(newblobobj)",Pyrophoric);
                memcpy(newObjSelection,tblob->blobObjects,sizeofSelectCommand(1));
                tblob->blobObjects = newObjSelection;
            }
            bitSet(tblob->flags, BTF_FreeBlobObjects);
        }

        if (!bitTest(tblob->flags, BTF_FreeThisBlob))
        {
            newBlob = memAlloc(sizeof(blob),"nb(newblob)",Pyrophoric);
            memcpy(newBlob,tblob,sizeof(blob));
            bitSet(newBlob->flags, BTF_FreeThisBlob);

            listAddNodeAfter(node,&newBlob->node,newBlob);
            listRemoveNode(node);
        }

        node = nextnode;
    }

    memFree(preallocBlobs);
    memFree(preallocObjSelection);
}

/*-----------------------------------------------------------------------------
    Name        : bobAllObjectsSatisfyBlob
    Description : Checks to see if the position of all objects is still within
                    the bounds of the blob.
    Inputs      : thisBlob - blob to examine.
    Outputs     :
    Return      : TRUE if all blob objects are still within the blob
----------------------------------------------------------------------------*/
bool bobAllObjectsSatisfyBlob(blob *thisBlob)
{
    vector difference, blobPosition;
    real32 blobRadius, blobRadiusSq = thisBlob->radius * thisBlob->radius;
    real32 differenceSq;

    if (bitTest(thisBlob->flags, BTF_RecentDeath))
    {                                                       //if something in this blob just died
        return(FALSE);                                      //update this blob
    }
    bobObjectListMedian(&blobPosition, &blobRadius, thisBlob->blobObjects->numSpaceObjs, thisBlob->blobObjects->SpaceObjPtr);

    if (blobRadius > thisBlob->radius)
    {                                                       //if the blob has grown
        if (blobRadius / thisBlob->radius  > bobUpdateRadiusThreshold)
        {                                                   //limit how much it can grow
            return(FALSE);
        }
    }
    vecSub(difference, thisBlob->centre, blobPosition);     //inter-centre distance
    differenceSq = difference.x * difference.x + difference.y * difference.y + difference.z * difference.z;
    if (differenceSq / blobRadiusSq > bobUpdateCentreThreshold)
    {                                                       //if the centre has moved too much
        return(FALSE);
    }
    return(TRUE);                                           //all objects inside the blob cool like ice cubes baby
}

/*-----------------------------------------------------------------------------
    Name        : bobAllObjectsFlagDontUpdate
    Description : Flag all objects in the blob as SOF_DontCreateBlob and the
                    blob as BTF_DontUpdate and BTF_ClearObjectFlags
    Inputs      : thisBlob - blob to flag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobAllObjectsFlagDontUpdate(blob *thisBlob)
{
    sdword count;
    SpaceObj **spaceobjPointer;

    spaceobjPointer = thisBlob->blobObjects->SpaceObjPtr;

    for (count = thisBlob->blobObjects->numSpaceObjs; count > 0; count--, spaceobjPointer++)
    {                                                       //flag all objects
        if ((*spaceobjPointer)->objtype == OBJ_BulletType)  //remove all bullets and missile missiles (not mines) from the list

        {
            *spaceobjPointer = thisBlob->blobObjects->SpaceObjPtr[thisBlob->blobObjects->numSpaceObjs - 1];
            spaceobjPointer--;
            thisBlob->blobObjects->numSpaceObjs--;
            continue;
        }
        else if ((((*spaceobjPointer)->objtype == OBJ_MissileType)) &&
                 (((Missile*)(*spaceobjPointer))->missileType != MISSILE_Mine))
        {                                                   //else it's a missile missile
            // missile could be in small target list and also in missile missile list
            if (thisBlob->blobSmallTargets != NULL)
            {
                RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobSmallTargets,(*spaceobjPointer));
            }
            *spaceobjPointer = thisBlob->blobObjects->SpaceObjPtr[thisBlob->blobObjects->numSpaceObjs - 1];
            spaceobjPointer--;
            thisBlob->blobObjects->numSpaceObjs--;
            continue;
        }
        dbgAssert(!bitTest((*spaceobjPointer)->flags, SOF_DontCreateBlob));
        bitSet((*spaceobjPointer)->flags, SOF_DontCreateBlob);
    }

    thisBlob->sqrtSortDistance = 0.0f;
    thisBlob->blobBullets->numBullets = 0;                  //clear out the bullet and missile lists
    thisBlob->blobMissileMissiles->numMissiles = 0;         //they'll soon be re-created anyhow
    thisBlob->flags |= BTF_DontUpdate | BTF_ClearObjectFlags;//flag the blob
}

/*-----------------------------------------------------------------------------
    Name        : bobAllObjectsUnflagDontUpdate
    Description : Clears all objects in the blob of SOF_DontCreateBlob and the
                    blob of BTF_DontUpdate and BTF_ClearObjectFlags
    Inputs      : thisBlob - blob to flag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobAllObjectsUnflagDontUpdate(blob *thisBlob)
{
    sdword count;
    SpaceObj **spaceobjPointer;

    spaceobjPointer = thisBlob->blobObjects->SpaceObjPtr;

    for (count = thisBlob->blobObjects->numSpaceObjs; count > 0; count--, spaceobjPointer++)
    {                                                       //flag all objects
        bitClear((*spaceobjPointer)->flags, SOF_DontCreateBlob);
    }
    bitClear(thisBlob->flags, (BTF_ClearObjectFlags));//flag the blob
}

/*-----------------------------------------------------------------------------
    Name        : bobAllBlobsFlagObjects
    Description : Flag all objects in specified blobs as satisfying the current
                    state of the blob.  If not satisfying blob state, delete
                    the blob.
    Inputs      : list - list of blobs
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobAllBlobsFlagObjects(LinkedList *list)
{
    Node *node, *nextNode;
    blob *thisBlob;

    for (node = list->head; node != NULL; node = nextNode)
    {
        nextNode = node->next;
        thisBlob = (blob *)listGetStructOfNode(node);

        if (bobAllObjectsSatisfyBlob(thisBlob))
        {                                                   //if this blob works in it's present state
            bobAllObjectsFlagDontUpdate(thisBlob);
        }
        else
        {                                                   //this blob is no good, proceed to update it as usual
            listRemoveNode(node);
            if (thisBlob->subBlobs.num != BIT31)
            {                                               //if there is a sub-blob list
                bobListDelete(&thisBlob->subBlobs);
            }
            blobFree(thisBlob);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : bobListCreate
    Description : Create a list of blobs for the current state of the universe.
                  Typically called at the startup of the sensors manager.
    Inputs      : playerIndex - index of current player.
    Outputs     :
    Return      : The head of a list of blobs.
----------------------------------------------------------------------------*/
void bobListCreate(BlobProperties *blobProperties, LinkedList *list, udword playerIndex)
{
    SpaceObj *spaceobj;
    Node *node;
    blob *newBlob;
    real32 mass;
    blob *preallocBlobs;
    SpaceObjSelection *preallocObjSelection;
    SpaceObjSelection *newObjSelection;
    udword usePreallocBlob = 0;
    blob *tblob;
    Node *nextnode;
    TargetPtr target;

    dbgAssert(blobProperties);
    BlobPropertiesPtr = blobProperties;
    //listInit(list);                                         //init the linked list
    if (list->num > 0)
    {                                                       //if there are already blobs in the blob list
        bobAllBlobsFlagObjects(list);
    }

    if (universe.SpaceObjList.num == 0)
    {
        return;     // no SpaceObj's, therefore no blobs
    }

    //first pass: create a sphere structure for each ship

    // mass preallocate everything
    preallocBlobs = memAlloc(bobBlobSize(1)*universe.SpaceObjList.num,"MAB(MassAllocBlobs)",Pyrophoric);
    preallocObjSelection = memAlloc(sizeof(SelectCommand)*universe.SpaceObjList.num,"MAS(MassAllocSel)",Pyrophoric);

    node = universe.SpaceObjList.head;                      //get first node in universe list
    while (node != NULL)
    {
        spaceobj = (SpaceObj *)listGetStructOfNode(node);
        node = node->next;
        if (spaceobj->flags & (SOF_Dead | SOF_DontCreateBlob))
        {                                                   //don't process dead or satisfactory objects
            continue;
        }
        switch (spaceobj->objtype)
        {
            case OBJ_ShipType:
                mass = ((Ship *)spaceobj)->staticinfo->staticheader.mass;
                break;
            case OBJ_BulletType:
                spaceobj->collMyBlob = NULL;
                continue;
            case OBJ_AsteroidType:
                if (((Asteroid *)spaceobj)->asteroidtype == Asteroid0)
                {
                    spaceobj->collMyBlob = NULL;        // don't add Asteroid0 for speed
                    continue;
                }
                // deliberately fall through
            case OBJ_NebulaType:
            case OBJ_GasType:
            case OBJ_DustType:
                mass = ((Resource *)spaceobj)->staticinfo->staticheader.mass;
                break;
            case OBJ_DerelictType:
                if (((Derelict *)spaceobj)->staticinfo->worldRender)
                {                                           //don't include worlds in the blobs.  They wouldn't fit anyhow.
                    continue;
                }
                mass = ((Derelict *)spaceobj)->staticinfo->staticheader.mass;
                break;
            case OBJ_EffectType:
                continue;
            case OBJ_MissileType:
                if (((MissilePtr)spaceobj)->missileType == MISSILE_Mine)
                {
                    mass = 0.1f;
                    break;
                }
                else
                {
                    spaceobj->collMyBlob = NULL;
                    continue;
                }
        }
        newBlob = &preallocBlobs[usePreallocBlob];
        newBlob->flags = 0;
        bitClear(newBlob->flags, BTF_FreeThisBlob);
        bitClear(newBlob->flags, BTF_FreeBlobObjects);
        newBlob->centre = spaceobj->posinfo.position;
        newBlob->radius = BlobPropertiesPtr->bobStartSphereSize;
        newBlob->volume = sphereVolume(newBlob->radius);
        newBlob->oneOverVolume = 1.0f / newBlob->volume;

        newBlob->totalMass = mass;

        newBlob->RUs = 0;

#ifndef HW_Release
//        newBlob->debugFlag = 0;
#endif
        newBlob->blobShips = NULL;
        newBlob->blobBigShips = NULL;
        newBlob->blobSmallShips = NULL;
        newBlob->blobBigTargets = NULL;
        newBlob->blobSmallTargets = NULL;
        newBlob->blobResources = NULL;
        newBlob->blobDerelicts = NULL;
        newBlob->blobBullets = NULL;
        newBlob->blobMissileMissiles = NULL;
        newBlob->blobMissileMines = NULL;

        newBlob->blobObjects = &preallocObjSelection[usePreallocBlob++];
        newBlob->blobObjects->numSpaceObjs = 1;
        newBlob->blobObjects->SpaceObjPtr[0] = spaceobj;
        newBlob->subBlobs.num = BIT31;                  //flag the sub-blob list not yet created

        newBlob->sortDistance = vecMagnitudeSquared(spaceobj->posinfo.position);
        newBlob->sqrtSortDistance = 0.0f;

        listAddNode(list, &newBlob->node, newBlob);
    }
    //second pass: sort the spheres based on distance from the mothership
    listMergeSortGeneral(list, bobListSortCallback);

    //final pass: combine the spheres into a neat, maageable list
    bobListUpdateStatsInit(FALSE);
    bobListUpdate(list);
    bobListUpdateStatsClose();

    // at this point we have a list of all the blobs we want.  But they may be in the preallocBlobs chunk.
    // Reallocate any that are in the preallocBlobs chunk.
    // Same goes for blobObjects which may be in the preallocSelectCommands chunk.

    node = list->head;
    while (node != NULL)
    {
        tblob = (blob *)listGetStructOfNode(node);
        nextnode = node->next;

        if (!bitTest(tblob->flags, BTF_FreeBlobObjects))
        {
            if (tblob->blobObjects != NULL)
            {
                dbgAssert(tblob->blobObjects->numSpaceObjs == 1);
                newObjSelection = memAlloc(sizeofSelectCommand(1),"nbo(newblobobj)",Pyrophoric);
                memcpy(newObjSelection,tblob->blobObjects,sizeofSelectCommand(1));
                tblob->blobObjects = newObjSelection;
            }
            bitSet(tblob->flags, BTF_FreeBlobObjects);
        }

        if (!bitTest(tblob->flags, BTF_FreeThisBlob))
        {
            newBlob = memAlloc(sizeof(blob),"nb(newblob)",Pyrophoric);
            memcpy(newBlob,tblob,sizeof(blob));
            bitSet(newBlob->flags, BTF_FreeThisBlob);

            listAddNodeAfter(node,&newBlob->node,newBlob);
            listRemoveNode(node);
        }

        node = nextnode;
    }

    memFree(preallocBlobs);
    memFree(preallocObjSelection);

    //next pass: do blob itemizing needed only for collisions
    bobUpdateExtraCollBobInfo(list);

    //add bullets and missiles to the collision blobs of their targets
    // put in bullets/missiles to nearest blobs

    node = universe.BulletList.head;                      //get first node in bullet list
    while (node != NULL)
    {
        spaceobj = (SpaceObj *)listGetStructOfNode(node);
        node = node->next;

        target = ((Bullet *)spaceobj)->target;

        if ((target != NULL) && (target->collMyBlob != NULL))
        {
            bobAddObjToSpecificBlob(target->collMyBlob,spaceobj);
        }
        else
        {
            bobAddObjToNearestBlob(list,spaceobj);
        }
    }

    node = universe.MissileList.head;                      //get first node in missile list
    while (node != NULL)
    {
        spaceobj = (SpaceObj *)listGetStructOfNode(node);
        node = node->next;

        if (spaceobj->flags & SOF_Dead)
        {
            continue;
        }

        if (((MissilePtr)spaceobj)->missileType != MISSILE_Mine)
        {
            target = ((Missile *)spaceobj)->target;

            if ((target != NULL) && (target->collMyBlob != NULL))
            {
                bobAddObjToSpecificBlob(target->collMyBlob,spaceobj);
            }
            else
            {
                bobAddObjToNearestBlob(list,spaceobj);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : bobObjetListMedian
    Description : Compute the median of a set of objects (average of greates and lowest).
    Inputs      : nObjects - number of objects
                  objects - list of objects
    Outputs     : dest - where to store the mean
                  destRadius - where to store the newly computed radius
    Return      :
----------------------------------------------------------------------------*/
void bobObjectListMedian(vector *dest, real32 *destRadius, sdword nObjects, SpaceObj **objects)
{
    sdword index;
    vector minV = {REALlyBig, REALlyBig, REALlyBig};
    vector maxV = {REALlyNegative, REALlyNegative, REALlyNegative};
    real32 radius, val;

    for (index = 0; index < nObjects; index++, objects++)
    {
        if (bitTest((*objects)->flags, SOF_Impactable))
        {                                                   //if there's a collision sphere
            radius = (*objects)->staticinfo->staticheader.staticCollInfo.collspheresize;
            val = (*objects)->posinfo.position.x - radius;
            minV.x = min(minV.x, val);
            val = (*objects)->posinfo.position.y - radius;
            minV.y = min(minV.y, val);
            val = (*objects)->posinfo.position.z - radius;
            minV.z = min(minV.z, val);
            val = (*objects)->posinfo.position.x + radius;
            maxV.x = max(maxV.x, val);
            val = (*objects)->posinfo.position.y + radius;
            maxV.y = max(maxV.y, val);
            val = (*objects)->posinfo.position.z + radius;
            maxV.z = max(maxV.z, val);
        }
        else
        {                                                   //else there's no collision sphere
            minV.x = min(minV.x, (*objects)->posinfo.position.x);
            minV.y = min(minV.y, (*objects)->posinfo.position.y);
            minV.z = min(minV.z, (*objects)->posinfo.position.z);
            maxV.x = max(maxV.x, (*objects)->posinfo.position.x);
            maxV.y = max(maxV.y, (*objects)->posinfo.position.y);
            maxV.z = max(maxV.z, (*objects)->posinfo.position.z);
        }
    }
    dest->x = (minV.x + maxV.x) * 0.5f;
    dest->y = (minV.y + maxV.y) * 0.5f;
    dest->z = (minV.z + maxV.z) * 0.5f;
    *destRadius = max(max(maxV.x - minV.x, maxV.y - minV.y), maxV.z - minV.z) * 0.5f + BlobPropertiesPtr->bobRadiusCombineMargin;
}

/*-----------------------------------------------------------------------------
    Name        : bobBlobCombine
    Description : Combine two blobs determined to be overlapping.
    Inputs      : thisBlob - the blob to be retained
                  otherBlob - the blob to be absorbed
                  newRadius - the new combined radius already computed
                  newVolume - the new combined volume already computed
    Outputs     :
    Return      : the next node for continued walking through the linked list
----------------------------------------------------------------------------*/
Node *bobBlobCombine(blob *thisBlob, blob *otherBlob, real32 newRadius, real32 newVolume)
{
    blob tempBlob;
    Node *nextNode;
    SpaceObjSelection *newselection;
    sdword totalNumObjects;

    tempBlob = *thisBlob;       // structure copy of thisBlob, before merging takes place

    totalNumObjects = tempBlob.blobObjects->numSpaceObjs + otherBlob->blobObjects->numSpaceObjs;
    newselection = memAlloc(sizeofSelectCommand(totalNumObjects),"nbO(newblobObjects)",Pyrophoric);
    newselection->numSpaceObjs = totalNumObjects;
    thisBlob->blobObjects = newselection;

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif
    bitClear(thisBlob->flags, BTF_DontUpdate);              //make sure this blob gets itemized
    thisBlob->flags |= otherBlob->flags & BTF_ClearObjectFlags;//make sure objects get their SOF_DontCreateBlob bit cleared
    if (bitTest(tempBlob.flags, BTF_FreeThisBlob))
    {
        bitSet(thisBlob->flags, BTF_FreeThisBlob);
    }
    else
    {
        bitClear(thisBlob->flags, BTF_FreeThisBlob);
    }
    bitSet(thisBlob->flags, BTF_FreeBlobObjects);
    thisBlob->blobShips = NULL;
    thisBlob->blobBigShips = NULL;
    thisBlob->blobSmallShips = NULL;
    thisBlob->blobSmallTargets = NULL;
    thisBlob->blobBigTargets = NULL;
    thisBlob->blobResources = NULL;
    thisBlob->blobDerelicts = NULL;
    thisBlob->blobBullets = NULL;
    thisBlob->blobMissileMissiles = NULL;
    thisBlob->blobMissileMines = NULL;

    //join the object lists together
    memcpy(newselection->SpaceObjPtr,tempBlob.blobObjects->SpaceObjPtr,tempBlob.blobObjects->numSpaceObjs * sizeof(SpaceObj *));
    memcpy(&newselection->SpaceObjPtr[tempBlob.blobObjects->numSpaceObjs], otherBlob->blobObjects->SpaceObjPtr, otherBlob->blobObjects->numSpaceObjs * sizeof(SpaceObj *));

    bobObjectListMedian(&thisBlob->centre, &newRadius, newselection->numSpaceObjs, newselection->SpaceObjPtr);
    thisBlob->radius = newRadius;
    thisBlob->volume = newVolume;
    thisBlob->oneOverVolume = 1.0f / newVolume;
    thisBlob->totalMass = tempBlob.totalMass + otherBlob->totalMass;
    thisBlob->sortDistance = vecMagnitudeSquared(thisBlob->centre);
    thisBlob->sqrtSortDistance = 0.0f;
    thisBlob->RUs = tempBlob.RUs + otherBlob->RUs;
    //delete otherBlob, and thisBlob becomes newBlob
    nextNode = otherBlob->node.next;
    blobFreeContents(&tempBlob);
    listRemoveNode(&otherBlob->node);
    blobFree(otherBlob);
    return(nextNode);
}

/*-----------------------------------------------------------------------------
    Name        : bobBlobItemize
    Description : Perform analiysis of a blob for current sensors level.  This
                    includes counting capital and non-capital ships, number of
                    RU's and determining if the blob is visable.
    Inputs      : thisBlob - blob to scan through
                  sensorsLevel - current sensors level for player.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
char bobCapitalClass[] = {CLASS_Frigate, CLASS_Destroyer, CLASS_Carrier, CLASS_HeavyCruiser, CLASS_Mothership, 0};
char bobAttackClass[] = {CLASS_Fighter, CLASS_Corvette, 0};
void bobBlobItemize(blob *thisBlob, sdword sensorsLevel)
{
    sdword index;
    SpaceObj *object;
    ubyte shipClass;
    SpaceObjSelection *blobObjects = thisBlob->blobObjects;
    bool bGravWellDetected = FALSE, bProbeDetected = FALSE;
    Ship *gravWell, *probe;

    thisBlob->RUs = 0;
    thisBlob->nRockRUs = thisBlob->nDustRUs = 0;
    thisBlob->nDustClouds = thisBlob->nRocks = 0;
    thisBlob->flags &= ~(BTM_ItemizeFlags);
    thisBlob->nCapitalShips = thisBlob->nAttackShips = thisBlob->nNonCombat = 0;
    thisBlob->nFriendlyShips = 0;
    thisBlob->shipMass = 0.0f;

    if (thisBlob->radius < BlobPropertiesPtr->bobSmallestRadius)
    {                                                   //cap the radius to a minimum
        thisBlob->radius = BlobPropertiesPtr->bobSmallestRadius;
    }
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {                                                       //for every object in this blob
        object = blobObjects->SpaceObjPtr[index];
        switch (object->objtype)
        {                                                   //see what type of ship it is
            case OBJ_ShipType:
                if (!allianceIsShipAlly((Ship *)object, universe.curPlayerPtr))
                {                                           //only count enemy units
                    shipClass = (ubyte)((ShipStaticInfo *)((Ship *)object)->staticinfo)->shipclass;
                    if (smStrchr(bobCapitalClass, shipClass) != NULL)
                    {
                        thisBlob->nCapitalShips++;
                    }
                    else if (smStrchr(bobAttackClass, shipClass) != NULL)
                    {
                        thisBlob->nAttackShips++;
                    }
                    else
                    {
                        thisBlob->nNonCombat++;
                    }
                    if (!bitTest(object->flags,SOF_Cloaked))
                    {                                       //if there is an uncloaked enemy ship here
                        bitSet(thisBlob->flags, BTF_UncloakedEnemies);
                    }
                }
                else
                {
                    thisBlob->nFriendlyShips++;
                }
                thisBlob->shipMass += ((Ship *)object)->staticinfo->staticheader.mass;
//!!! bryce: change this definition
#define probeActive(ship)       TRUE
                if (((Ship *)object)->playerowner == universe.curPlayerPtr &&
                    !(object->flags & SOF_Disabled))        //if shipis players and isn't disabled!
                {                                           //if this is one of the player's ships
                    bitSet(thisBlob->flags, BTF_Explored);
                    if (((Ship *)object)->shiptype == Probe && probeActive((Ship *)object))
                    {                                       //if player has an active probe in the blob
                        bitSet(thisBlob->flags, BTF_ProbeDroid);
                    }
                    else if (((Ship *)object)->shiptype == Carrier)
                    {
                        bitSet(thisBlob->flags, BTF_Carrier);
                    }
                    else if (((Ship *)object)->shiptype == Mothership)
                    {
                        bitSet(thisBlob->flags, BTF_Mothership);
                    }
                }
                else if (allianceIsShipAlly((Ship *)object,universe.curPlayerPtr) &&
                    !(object->flags & SOF_Disabled))
                {
                    bitSet(thisBlob->flags, BTF_Explored);
                }
                else
                {                                           //not friendly or allied
                    if (((Ship *)object)->shiptype == GravWellGenerator)
                    {                                       //enemy gravwell
                        if (((GravWellGeneratorSpec *)(((Ship *)object)->ShipSpecifics))->GravFieldOn)
                        {                                   //if it's active
                            bGravWellDetected = TRUE;
                            gravWell = (Ship *)object;
                        }
                    }
                    else if (((Ship *)object)->shiptype == Probe)
                    {                                       //enemy probe
                        bProbeDetected = TRUE;
                        probe = (Ship *)object;
                    }
                }
                if (((Ship *)object)->recentlyAttacked != 0)
                {
                    bitSet(thisBlob->flags, BTF_GettingAttacked);
                }
                if (((Ship *)object)->shipisattacking)
                {
                    bitSet(thisBlob->flags, BTF_Attacking);
                }
                break;
            case OBJ_AsteroidType:
                bitSet(thisBlob->flags, BTF_Asteroid);
                thisBlob->nRockRUs += ((Resource *)object)->staticinfo->resourcevalue;
                thisBlob->nRocks++;
                goto resourceCommon;
            case OBJ_GasType:
                goto resourceCommon;
            case OBJ_DustType:
                bitSet(thisBlob->flags, BTF_DustCloud);
                thisBlob->nDustRUs += ((Resource *)object)->staticinfo->resourcevalue;
                thisBlob->nDustClouds++;
                goto resourceCommon;
            case OBJ_NebulaType:
                bitSet(thisBlob->flags, BTF_Nebula);
                thisBlob->nDustRUs += ((Resource *)object)->staticinfo->resourcevalue;
resourceCommon:
                thisBlob->RUs += ((Resource *)object)->staticinfo->resourcevalue;
                break;
            default:
                break;
        }
    }

    if (bitTest(thisBlob->flags, BTF_Explored))
    {
        if (bProbeDetected)
        {
            battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_ProbeDetected, SOUND_EVENT_DEFAULT, &probe->posinfo.position);
        }
        if (bGravWellDetected)
        {
            battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_GravwellDetected, SOUND_EVENT_DEFAULT, &gravWell->posinfo.position);
        }
    }

    //set maximum sensors levels the same as having a probe in every blob if we are in ghost mode.
    if (smGhostMode)
    {
        bitSet(thisBlob->flags, BTF_ProbeDroid);
    }
    //explicitly say a player ship is in this blob if the correct sensors level
}

/*-----------------------------------------------------------------------------
    Name        : bobListUpdate
    Description : Updates the current blob list by combining spheres (as needed)
                    and splitting spheres (as needed).
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobListUpdate(LinkedList *list)
{
    Node *thisNode, *otherNode;
    blob *thisBlob, *otherBlob;
    sdword nHits;
    real32 distance, radius, checkradius, newRadius, newVolume, newDensity;
    vector difference;
    real32 thisblobSortDistance;
    real32 otherblobSortDistance;

    real32 bobOverlapFactor = BlobPropertiesPtr->bobOverlapFactor;
    real32 bobSqrtOverlapFactor = BlobPropertiesPtr->bobSqrtOverlapFactor;
    real32 bobDensityHigh = BlobPropertiesPtr->bobDensityHigh;
    real32 bobBiggestRadius = BlobPropertiesPtr->bobBiggestRadius;

    real32 checkdist;

    bobStatsInitialBlobs(list->num);

    //first pass: go through list repeatedly until there are no more spheres to combine
    do
    {
        nHits = 0;
        thisNode = list->head;
        bobStatsPass();
        while (thisNode != NULL)
        {
            thisBlob = (blob *)listGetStructOfNode(thisNode);
            if (thisBlob->sqrtSortDistance == 0.0f)
            {
                thisBlob->sqrtSortDistance = fsqrt(thisBlob->sortDistance);
            }
            thisblobSortDistance = thisBlob->sqrtSortDistance;
            //walk forward through list combining any spheres which could combine with this one
            otherNode = thisNode->next;
            while (otherNode != NULL)
            {
                //get distance bettween blobs and compare to sum of radii
                bobStatsWalk();
                otherBlob = (blob *)listGetStructOfNode(otherNode);//get distance bettween nodes

                if (otherBlob->sqrtSortDistance == 0.0f)
                {
                    otherBlob->sqrtSortDistance = fsqrt(otherBlob->sortDistance);
                }
                otherblobSortDistance = otherBlob->sqrtSortDistance;

                checkdist = otherblobSortDistance - thisblobSortDistance;

                //a test for early aborting of this comparison process
                if (checkdist > (thisBlob->radius + bobBiggestRadius))
                {
                    bobStatsTrivialReject();
                    break;
                }

                radius = thisBlob->radius + otherBlob->radius;
                checkradius = radius * bobSqrtOverlapFactor;
                if (checkdist > checkradius)
                {
                    goto nextnode;
                }

                bobStatsCheck();

                vecSub(difference, thisBlob->centre, otherBlob->centre);

                if (!isBetweenInclusive(difference.x,-checkradius,checkradius))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(difference.y,-checkradius,checkradius))
                {
                    goto nextnode;
                }

                if (!isBetweenInclusive(difference.z,-checkradius,checkradius))
                {
                    goto nextnode;
                }

                distance = vecMagnitudeSquared(difference);
                radius *= radius;                           //combined radii^2

                if (distance <= (radius*bobOverlapFactor))
                {                                           //if these blobs are sufficiently overlapping
                    //determine if the new sphere would match density criteria
                    bobStatsCombine();
                    newRadius = (((real32)fsqrt(distance)) + thisBlob->radius + otherBlob->radius) * 0.5f;
                    newVolume = sphereVolume(newRadius);
                    newDensity = (thisBlob->totalMass + otherBlob->totalMass) / newVolume;

                    if (newDensity > max(thisBlob->totalMass * thisBlob->oneOverVolume, otherBlob->totalMass * otherBlob->oneOverVolume))
                    {                                       //if new sphere is denser than the greater of the two spheres
                        otherNode = bobBlobCombine(thisBlob, otherBlob, newRadius, newVolume);
                        nHits++;
                    }
                    else if (newDensity > bobDensityHigh && newRadius < bobBiggestRadius)
                    {                                       //if combination will increase density
                        otherNode = bobBlobCombine(thisBlob, otherBlob, newRadius, newVolume);
                        nHits++;
                    }
                    else
                    {                                       //else we're not to combine them
                        otherNode = otherNode->next;
                    }
                }
                else
                {
nextnode:
                    otherNode = otherNode->next;
                }
            }
            thisNode = thisNode->next;
        }
    }
    while (nHits > 0);

    bobStatsFinalBlobs(list->num);

    //do a pass over all the final nodes to determine how to render them
    thisNode = list->head;
    while (thisNode != NULL)
    {
        thisBlob = listGetStructOfNode(thisNode);
        thisNode = thisNode->next;
        if (bitTest(thisBlob->flags, BTF_ClearObjectFlags))
        {
            bobAllObjectsUnflagDontUpdate(thisBlob);        //remove the update flag
        }
        if (!bitTest(thisBlob->flags, BTF_DontUpdate))
        {
            bobBlobItemize(thisBlob, universe.curPlayerPtr->sensorLevel);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : blobFreeContents
    Description : frees the blob's contents
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void blobFreeContents(blob *thisBlob)
{
#ifndef HW_Release
//    dbgAssert(thisBlob->debugFlag == 0);
#endif

    if (thisBlob->blobShips != NULL) memFree(thisBlob->blobShips);
    if (thisBlob->blobBigShips != NULL) memFree(thisBlob->blobBigShips);
    if (thisBlob->blobSmallShips != NULL) memFree(thisBlob->blobSmallShips);
    if (thisBlob->blobSmallTargets != NULL) memFree(thisBlob->blobSmallTargets);
    if (thisBlob->blobBigTargets != NULL) memFree(thisBlob->blobBigTargets);
    if (thisBlob->blobResources != NULL) memFree(thisBlob->blobResources);
    if (thisBlob->blobDerelicts != NULL) memFree(thisBlob->blobDerelicts);
    if (thisBlob->blobBullets != NULL) memFree(thisBlob->blobBullets);
    if (thisBlob->blobMissileMissiles != NULL) memFree(thisBlob->blobMissileMissiles);
    if (thisBlob->blobMissileMines != NULL) memFree(thisBlob->blobMissileMines);

    if (bitTest(thisBlob->flags, BTF_FreeBlobObjects))
    {
        if (thisBlob->blobObjects != NULL) memFree(thisBlob->blobObjects);
    }
}

/*-----------------------------------------------------------------------------
    Name        : blobFree
    Description : frees a blob
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void blobFree(blob *thisBlob)
{
    blobFreeContents(thisBlob);

    if (bitTest(thisBlob->flags, BTF_FreeThisBlob))
    {
        memFree(thisBlob);
    }
}

/*-----------------------------------------------------------------------------
    Name        : bobListDelete
    Description : Delete a list of blobs, along with any sub-blobs they might be hiding.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobListDelete(LinkedList *list)
{
    Node *blobnode = list->head;
    Node *nextnode;
    blob *tblob;

    while (blobnode != NULL)
    {
        tblob = (blob *)listGetStructOfNode(blobnode);
        nextnode = blobnode->next;
        listRemoveNode(blobnode);
        if (tblob->subBlobs.num != BIT31)
        {                                                   //if there is a sub-blob list
            bobListDelete(&tblob->subBlobs);
        }
        blobFree(tblob);
        blobnode = nextnode;
    }

    dbgAssert(list->num == 0);
}
#if 0
/*-----------------------------------------------------------------------------
    Name        : bobGetChecksum
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 bobGetChecksum(LinkedList *list,sdword *numBlobsInChecksum)
{
    Node *blobnode = list->head;
    blob *thisBlob;
    real32 x=0.0f;
    real32 y=0.0f;
    real32 z=0.0f;
    sdword countBlobs = 0;

    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        x += thisBlob->centre.x;
        y += thisBlob->centre.y;
        z += thisBlob->centre.z;

        if ((netlogfile) && (logEnable == LOG_VERBOSE))
        {
#if BINNETLOG
            binnetBlobInfo bi;
            bi.header = makenetcheckHeader('B','1','0','B');
            bi.collBlobSortDist = thisBlob->sortDistance;
            bi.x = thisBlob->centre.x;
            bi.y = thisBlob->centre.y;
            bi.z = thisBlob->centre.z;
            bi.r = thisBlob->radius;
            bi.numSpaceObjs = thisBlob->blobObjects->numSpaceObjs;
            fwrite(&bi, sizeof(bi), 1 ,netlogfile);
#else
            fprintf(netlogfile,"  Blob:%f %f %f %d\n",thisBlob->centre.x,thisBlob->centre.y,thisBlob->centre.z,thisBlob->blobObjects->numSpaceObjs);
#endif
        }

        countBlobs++;

        blobnode = blobnode->next;
    }

    *numBlobsInChecksum = countBlobs;

    return (x+y+z);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : bobRemoveMineFromSpecificBlob
    Description : removes a mine from thisBlob
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobRemoveMineFromSpecificBlob(blob *thisBlob,Missile *mine)
{
    RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobObjects,(SpaceObj *)mine);
    RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobMissileMines,(SpaceObj *)mine);
    mine->collMyBlob = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : bobObjectDied
    Description : Deletes a spaceobj out of the blobs
    Inputs      : object, list
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void bobObjectDied(SpaceObj *object,LinkedList *list)
{
    //remove from any blob lists it may be in
    Node *node = list->head;
    blob *thisBlob;
    sdword index;
    SpaceObjSelection *blobObjects;

    while (node != NULL)
    {                                                       //search every blob
        thisBlob = (blob *)listGetStructOfNode(node);
        if (thisBlob->subBlobs.num != BIT31)
        {                                                   //if there is a sub-blob list in this blob
            bobObjectDied(object, &thisBlob->subBlobs);
        }
        blobObjects = thisBlob->blobObjects;

        for (index = 0; index < blobObjects->numSpaceObjs; index++)
        {                                                   //search every object in blob
            if ((SpaceObj *)object == blobObjects->SpaceObjPtr[index])
            {                                               //if this is the blob
                for (; index < blobObjects->numSpaceObjs - 1; index++)
                {                                           //for the rest of the list
                    blobObjects->SpaceObjPtr[index] = blobObjects->SpaceObjPtr[index + 1];
                }
                blobObjects->numSpaceObjs--;                //mode one notch back in the array

                if (object->flags & SOF_Targetable)
                {
                    if(object->objtype != OBJ_MissileType)
                    {
                        if (((TargetPtr)object)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_TARGET_SIZE)
                        {
                            if (thisBlob->blobBigTargets != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobBigTargets,object);
                                dbgAssert(result);
                            }
                        }
                        else
                        {
                            //don't do for mines...they won't be in the list
                            if (thisBlob->blobSmallTargets != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobSmallTargets,object);
                                dbgAssert(result);
                            }
                        }
                    }
                }

                switch (object->objtype)
                {
                    case OBJ_ShipType:
                        if (((ShipPtr)object)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_SHIP_SIZE)
                        {
                            if (thisBlob->blobBigShips != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobBigShips,object);
                                dbgAssert(result);
                            }
                        }
                        else
                        {
                            if (thisBlob->blobSmallShips != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobSmallShips,object);
                                dbgAssert(result);
                            }
                        }

                        if (thisBlob->blobShips != NULL)
                        {
                            bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobShips,object);
                            dbgAssert(result);
                        }
                        bitSet(thisBlob->flags, BTF_RecentDeath);
                        break;

                    case OBJ_BulletType:
                        if (thisBlob->blobBullets != NULL)
                        {
                            bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobBullets,object);
                            dbgAssert(result);
                        }
                        break;

                    case OBJ_AsteroidType:
                    case OBJ_NebulaType:
                    case OBJ_GasType:
                    case OBJ_DustType:
                        if (thisBlob->blobResources != NULL)
                        {
                            bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobResources,object);
                            dbgAssert(result);
                        }
                        bitSet(thisBlob->flags, BTF_RecentDeath);
                        break;

                    case OBJ_DerelictType:
                        if (thisBlob->blobDerelicts != NULL)
                        {
                            bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobDerelicts,object);
                            dbgAssert(result);
                        }
                        break;

                    case OBJ_MissileType:
                        if (((MissilePtr)object)->missileType == MISSILE_Mine)
                        {
                            if (thisBlob->blobMissileMines != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobMissileMines,object);
                                dbgAssert(result);
                            }
                        }
                        else
                        {
                            if (thisBlob->blobMissileMissiles != NULL)
                            {
                                bool result = RemoveSpaceObjFromSelectionPreserveOrder((SpaceObjSelection *)thisBlob->blobMissileMissiles,object);
                                dbgAssert(result);
                            }
                        }
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }

#if 0      // don't do verify here - objects may be deleted from blob before being removed from lists resulting in inconsistancies
#if BOB_ANAL_CHECKING
                if (thisBlob->subBlobs.num == BIT31)
                {
                    blobAnalVerify(thisBlob);
                }
#endif
#endif
                goto foundInBlobs;
            }
        }
        node = node->next;
    }
foundInBlobs:;
}

/*-----------------------------------------------------------------------------
    Name        : AddSpaceObjToSelectionPreserveOrder
    Description : Adds obj to selection, while preserving sort order (based on collOptimizeDist)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddSpaceObjToSelectionPreserveOrder(SpaceObjSelection *selection,SpaceObj *obj)
{
    sdword bottom,top;
    sdword numObjects = selection->numSpaceObjs;
    sdword foundindex;

    if (numObjects == 0)
    {
        // just add new object:
        selection->SpaceObjPtr[0] = obj;
        selection->numSpaceObjs = 1;
        return;
    }

#if BOB_ANAL_CHECKING
    if (ShipInSelection((SelectCommand *)selection,(Ship *)obj))
    {
        dbgFatalf(DBG_Loc,"obj %d already in selection!",obj->objtype);
    }
#endif

    bottom = 0;
    top = numObjects-1;
    while (top >= bottom)
    {
        foundindex = (bottom+top)>>1;
        if (obj->collOptimizeDist > selection->SpaceObjPtr[foundindex]->collOptimizeDist)
        {
            // foundindex is too small
            bottom = foundindex+1;
        }
        else
        {
            top = foundindex-1;
        }
    }

    dbgAssert(foundindex >= 0);
    dbgAssert(foundindex < numObjects);

    if (obj->collOptimizeDist < selection->SpaceObjPtr[foundindex]->collOptimizeDist)
    {
        AddSpaceObjToSelectionBeforeIndex(obj,selection,foundindex);
    }
    else
    {
        AddSpaceObjToSelectionAfterIndex(obj,selection,foundindex);
    }
}

#if 0
void *myMemRealloc(void *currentPointer, sdword newSize, char *name, udword flags)
{
    ubyte *newptr;

    newptr = memAlloc(newSize,name,flags);
    memcpy(newptr,currentPointer,newSize-4);
    memFree(currentPointer);

    return newptr;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : AddObjToObjectListsOfBlob
    Description : Add an object to a specific blob
    Inputs      : obj - object to add to blob
                  putInBlob - blob to put obj in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddObjToObjectListsOfBlob(SpaceObj *obj,blob *putInBlob)
{
    if (obj->flags & SOF_Targetable)
    {
        if(obj->objtype != OBJ_MissileType)
        {
            //don't add missiles to big/small target lists.
            if (((TargetPtr)obj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_TARGET_SIZE)
            {
                if ((putInBlob->blobBigTargets->numTargets+1) > putInBlob->maxNumBigTargets)
                {
                    putInBlob->maxNumBigTargets += BLOBOBJ_BATCH;
                    putInBlob->blobBigTargets = memRealloc(putInBlob->blobBigTargets,sizeofSelectCommand(putInBlob->maxNumBigTargets),"nbbt(nblobbtargets)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobBigTargets,obj);
            }
            else
            {
                if ((putInBlob->blobSmallTargets->numTargets+1) > putInBlob->maxNumSmallTargets)
                {
                    putInBlob->maxNumSmallTargets += BLOBOBJ_BATCH;
                    putInBlob->blobSmallTargets = memRealloc(putInBlob->blobSmallTargets,sizeofSelectCommand(putInBlob->maxNumSmallTargets),"nbst(nblobstargets)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobSmallTargets,obj);
            }
        }
    }

    switch (obj->objtype)
    {
        case OBJ_ShipType:
            if (((ShipPtr)obj)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_SHIP_SIZE)
            {
                if ((putInBlob->blobBigShips->numShips+1) > putInBlob->maxNumBigShips)
                {
                    putInBlob->maxNumBigShips += BLOBOBJ_BATCH;
                    putInBlob->blobBigShips = memRealloc(putInBlob->blobBigShips,sizeofSelectCommand(putInBlob->maxNumBigShips),"nbbs(nblobbships)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobBigShips,obj);
            }
            else
            {
                if ((putInBlob->blobSmallShips->numShips+1) > putInBlob->maxNumSmallShips)
                {
                    putInBlob->maxNumSmallShips += BLOBOBJ_BATCH;
                    putInBlob->blobSmallShips = memRealloc(putInBlob->blobSmallShips,sizeofSelectCommand(putInBlob->maxNumSmallShips),"nbss(nblobsships)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobSmallShips,obj);
            }

            if ((putInBlob->blobShips->numShips+1) > putInBlob->maxNumShips)
            {
                putInBlob->maxNumShips += BLOBOBJ_BATCH;
                putInBlob->blobShips = memRealloc(putInBlob->blobShips,sizeofSelectCommand(putInBlob->maxNumShips),"nbs(nblobships)",Pyrophoric);
            }
            AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobShips,obj);
            break;

        case OBJ_BulletType:
            if ((putInBlob->blobBullets->numBullets+1) > putInBlob->maxNumBullets)
            {
                putInBlob->maxNumBullets += BLOBOBJ_BATCH;
                putInBlob->blobBullets = memRealloc(putInBlob->blobBullets,sizeofSelectCommand(putInBlob->maxNumBullets),"nbb(nblobbullets)",Pyrophoric);
            }
            AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobBullets,obj);
            break;

        case OBJ_AsteroidType:
        case OBJ_NebulaType:
        case OBJ_GasType:
        case OBJ_DustType:
            if ((putInBlob->blobResources->numResources+1) > putInBlob->maxNumResources)
            {
                putInBlob->maxNumResources += BLOBOBJ_BATCH;
                putInBlob->blobResources = memRealloc(putInBlob->blobResources,sizeofSelectCommand(putInBlob->maxNumResources),"nbr(nblobresources)",Pyrophoric);
            }
            AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobResources,obj);
            break;

        case OBJ_DerelictType:
            if ((putInBlob->blobDerelicts->numDerelicts+1) > putInBlob->maxNumDerelicts)
            {
                putInBlob->maxNumDerelicts += BLOBOBJ_BATCH;
                putInBlob->blobDerelicts = memRealloc(putInBlob->blobDerelicts,sizeofSelectCommand(putInBlob->maxNumDerelicts),"nbd(nblobderelicts)",Pyrophoric);
            }
            AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobDerelicts,obj);
            break;

        case OBJ_MissileType:
            if (((MissilePtr)obj)->missileType == MISSILE_Mine)
            {
                if ((putInBlob->blobMissileMines->numMissiles+1) > putInBlob->maxNumMissileMines)
                {
                    putInBlob->maxNumMissileMines += BLOBOBJ_BATCH;
                    putInBlob->blobMissileMines = memRealloc(putInBlob->blobMissileMines,sizeofSelectCommand(putInBlob->maxNumMissileMines),"nbm(nblobmines)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobMissileMines,obj);
            }
            else
            {
                if ((putInBlob->blobMissileMissiles->numMissiles+1) > putInBlob->maxNumMissileMissiles)
                {
                    putInBlob->maxNumMissileMissiles += BLOBOBJ_BATCH;
                    putInBlob->blobMissileMissiles = memRealloc(putInBlob->blobMissileMissiles,sizeofSelectCommand(putInBlob->maxNumMissileMissiles),"nbm(nblobmissiles)",Pyrophoric);
                }
                AddSpaceObjToSelectionPreserveOrder((SpaceObjSelection *)putInBlob->blobMissileMissiles,obj);
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    if ((putInBlob->blobObjects->numSpaceObjs+1) > putInBlob->maxNumObjects)
    {
        dbgAssert(bitTest(putInBlob->flags, BTF_FreeBlobObjects));
        putInBlob->maxNumObjects += BLOBOBJ_BATCH;
        putInBlob->blobObjects = memRealloc(putInBlob->blobObjects,sizeofSelectCommand(putInBlob->maxNumObjects),"nbo(nblobobjects)",Pyrophoric);
    }
    AddSpaceObjToSelectionPreserveOrder(putInBlob->blobObjects,obj);
    //blobAnalVerify(putInBlob);        INVALID TO PUT HERE: Missiles are added to blobs before the missile list, will fail!
}

/*-----------------------------------------------------------------------------
    Name        : bobAddObjToSpecificBlob
    Description : Adds obj to putInBlob, while preserving sort order
    Inputs      : putInBlob, obj
    Outputs     :
    Return      :
    Note        : Must be collision blobs, which is sorted
----------------------------------------------------------------------------*/
void bobAddObjToSpecificBlob(blob *putInBlob,SpaceObj *obj)
{
    vector distvec;

    dbgAssert((obj->flags & SOF_Dead) == 0);
    if (obj->objtype == OBJ_AsteroidType)
    {
        dbgAssert(((Asteroid *)obj)->asteroidtype != Asteroid0);    // don't put Asteroid0 in for speed
    }
    dbgAssert(putInBlob);

#ifndef HW_Release
//    dbgAssert(putInBlob->debugFlag == 0);
#endif

    vecSub(distvec,obj->posinfo.position,putInBlob->centre);
    obj->collOptimizeDist = fsqrt(vecMagnitudeSquared(distvec));
    obj->collMyBlob = putInBlob;

    AddObjToObjectListsOfBlob(obj,putInBlob);
}

/*-----------------------------------------------------------------------------
    Name        : bobAddObjToNearestBlob
    Description : Adds obj to nearest blob in list, while preserving sort order
    Inputs      : list, obj
    Outputs     :
    Return      :
    Note        : Must be collision blobs, which is sorted
----------------------------------------------------------------------------*/
void bobAddObjToNearestBlob(LinkedList *list,SpaceObj *obj)
{
    real32 distsqr;
    blob *putInBlob;

    dbgAssert((obj->flags & SOF_Dead) == 0);
    if (obj->objtype == OBJ_AsteroidType)
    {
        dbgAssert(((Asteroid *)obj)->asteroidtype != Asteroid0);    // don't put Asteroid0 in for speed
    }

    putInBlob = bobFindNearestBlobToObject(list,obj,&distsqr);
    if (putInBlob == NULL)
    {
        return;
    }

#ifndef HW_Release
//    dbgAssert(putInBlob->debugFlag == 0);
#endif

    obj->collOptimizeDist = fsqrt(distsqr);
    obj->collMyBlob = putInBlob;

    AddObjToObjectListsOfBlob(obj,putInBlob);
}

#if BOB_ANAL_CHECKING
/*-----------------------------------------------------------------------------
    Name        : bobAnalShipInSelection
    Description : returns TRUE if ship is in selection
    Inputs      : selection, ship
    Outputs     :
    Return      : number of times object is in selection
----------------------------------------------------------------------------*/
sdword bobAnalShipInSelection(SelectCommand *selection,Ship *ship)
{
    sdword i, nFound = 0;

    for (i=0;i<selection->numShips;i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            nFound++;
        }
    }
    return(nFound);
}

/*-----------------------------------------------------------------------------
    Name        : blobAnalVerifyFn
    Description : Anally check all lists in the specified blob.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void blobAnalVerifyFn(blob *thisBlob)
{
    sdword index;
    SpaceObjSelection *blobObjects;
    SpaceObj *object;
    bool result;
    sdword numShips, numSmallShips, numBigShips, numSmallTargets, numBigTargets, numResources, numDerelicts, numBullets, numMissileMissiles, numMissileMines, numObjects;

    //verify the main object list
    blobObjects = thisBlob->blobObjects;
    numObjects = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {                                                   //search every object in blob
        object = blobObjects->SpaceObjPtr[index];
        dbgAssert(object->collMyBlob == thisBlob);

        dbgAssert(1 == bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects,(Ship *)object));

        if (object->flags & SOF_Targetable)
        {
            if(obj->objtype != OBJ_MissileType)
            {
                if (((TargetPtr)object)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_TARGET_SIZE)
                {
                    dbgAssert(thisBlob->blobBigTargets != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobBigTargets, (Ship *)object);
                    dbgAssert(result == 1);
                }
                else
                {
                    dbgAssert(thisBlob->blobSmallTargets != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobSmallTargets, (Ship *)object);
                    dbgAssert(result == 1);
                }
            }
        }
        switch (object->objtype)
        {
            case OBJ_ShipType:
                if (((ShipPtr)object)->staticinfo->staticheader.staticCollInfo.originalcollspheresize >= BIG_SHIP_SIZE)
                {
                    dbgAssert(thisBlob->blobBigShips != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobBigShips, (Ship *)object);
                    dbgAssert(result == 1);
                }
                else
                {
                    dbgAssert(thisBlob->blobSmallShips != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobSmallShips, (Ship *)object);
                    dbgAssert(result == 1);
                }

                dbgAssert(thisBlob->blobShips != NULL);
                result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobShips, (Ship *)object);
                dbgAssert(result == 1);
                break;

            case OBJ_BulletType:
                dbgAssert(thisBlob->blobBullets != NULL);
                result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobBullets, (Ship *)object);
                dbgAssert(result == 1);
                break;

            case OBJ_AsteroidType:
            case OBJ_NebulaType:
            case OBJ_GasType:
            case OBJ_DustType:
                dbgAssert(thisBlob->blobResources != NULL);
                result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobResources, (Ship *)object);
                dbgAssert(result == 1);
                break;

            case OBJ_DerelictType:
                dbgAssert(thisBlob->blobDerelicts != NULL);
                result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobDerelicts, (Ship *)object);
                dbgAssert(result == 1);
                break;

            case OBJ_MissileType:
                if (((MissilePtr)object)->missileType == MISSILE_Mine)
                {
                    dbgAssert(thisBlob->blobMissileMines != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobMissileMines, (Ship *)object);
                    dbgAssert(result == 1);
                }
                else
                {
                    dbgAssert(thisBlob->blobMissileMissiles != NULL);
                    result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobMissileMissiles, (Ship *)object);
                    dbgAssert(result == 1);
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

    }
    //now verify the individually itemized lists
    //verify ship list
    blobObjects = (SpaceObjSelection *)thisBlob->blobShips;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ShipList.num);
    //dbgAssert(thisBlob->blobBigShips->numShips + thisBlob->blobSmallShips->numShips == thisBlob->blobShips->numShips);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumShips);
    numShips = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->collMyBlob == thisBlob);
        dbgAssert(object->objtype == OBJ_ShipType);
    }
    //verify small ship list
    blobObjects = (SpaceObjSelection *)thisBlob->blobSmallShips;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ShipList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumSmallShips);
    numSmallShips = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->collMyBlob == thisBlob);
        dbgAssert(object->objtype == OBJ_ShipType);
    }
    //verify big ship list
    blobObjects = (SpaceObjSelection *)thisBlob->blobBigShips;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ShipList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumBigShips);
    numBigShips = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->collMyBlob == thisBlob);
        dbgAssert(object->objtype == OBJ_ShipType);
    }
    //verify target lists
    blobObjects = (SpaceObjSelection *)thisBlob->blobSmallTargets;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ImpactableList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumSmallTargets);
    numSmallTargets = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(bitTest(object->flags, SOF_Targetable));
    }
    blobObjects = (SpaceObjSelection *)thisBlob->blobBigTargets;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ImpactableList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumBigTargets);
    numBigTargets = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(bitTest(object->flags, SOF_Targetable));
    }
    //verify resource list
    blobObjects = (SpaceObjSelection *)thisBlob->blobResources;
    dbgAssert(blobObjects->numSpaceObjs <= universe.ResourceList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumResources);
    numResources = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->objtype == OBJ_AsteroidType || object->objtype == OBJ_NebulaType || object->objtype == OBJ_DustType || object->objtype == OBJ_GasType);
    }
    //verify derelict list
    blobObjects = (SpaceObjSelection *)thisBlob->blobDerelicts;
    dbgAssert(blobObjects->numSpaceObjs <= universe.DerelictList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumDerelicts);
    numDerelicts = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->objtype == OBJ_DerelictType);
    }
    //verify bullet list
    blobObjects = (SpaceObjSelection *)thisBlob->blobBullets;
    dbgAssert(blobObjects->numSpaceObjs <= universe.BulletList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumBullets);
    numBullets = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->objtype == OBJ_BulletType);
    }
    //verify missile list
    blobObjects = (SpaceObjSelection *)thisBlob->blobMissileMissiles;
    dbgAssert(blobObjects->numSpaceObjs <= universe.MissileList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumMissileMissiles);
    numMissileMissiles = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->objtype == OBJ_MissileType);
        dbgAssert(((MissilePtr)object)->missileType == MISSILE_Regular);
    }
    //verify mine list
    blobObjects = (SpaceObjSelection *)thisBlob->blobMissileMines;
    dbgAssert(blobObjects->numSpaceObjs <= universe.MissileList.num);
    dbgAssert(blobObjects->numSpaceObjs <= thisBlob->maxNumMissileMines);
    numMissileMines = blobObjects->numSpaceObjs;
    for (index = 0; index < blobObjects->numSpaceObjs; index++)
    {
        object = blobObjects->SpaceObjPtr[index];
        result = bobAnalShipInSelection((SelectCommand *)thisBlob->blobObjects, (Ship *)object);
        dbgAssert(result == 1);
        dbgAssert(object->objtype == OBJ_MissileType);
        dbgAssert(((MissilePtr)object)->missileType == MISSILE_Mine);
    }

    //numeric verifications
    dbgAssert(numSmallShips + numBigShips == numShips);
    dbgAssert(numMissileMines + numMissileMissiles + numBullets + numDerelicts + numResources + numShips == numObjects);
}
#endif

