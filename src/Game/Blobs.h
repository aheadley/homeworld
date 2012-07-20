/*=============================================================================
    Name    : Blobs.h
    Purpose : Creates 'blobs' or spheres as part of the fog-of-war in the
                sensors manager.

    Created 2/9/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___BLOBS_H
#define ___BLOBS_H              1

#include "Types.h"
#include "Vector.h"
#include "LinkedList.h"
#include "SpaceObj.h"
#include "ShipSelect.h"

/*=============================================================================
    Switches:
=============================================================================*/

#ifndef HW_Release
#ifdef gshaw
#define BOB_STATS
#endif
#endif

#define BOB_TEST                    1

#ifndef HW_Release

#define BOB_ERROR_CHECKING          1           //general error checking
#define BOB_VERBOSE_LEVEL           4           //control specific output code
#define BOB_ANAL_CHECKING           0           //super-anal blob validation control

#else //HW_Debug

#define BOB_ERROR_CHECKING          0           //general error checking
#define BOB_VERBOSE_LEVEL           0           //control specific output code
#define BOB_ANAL_CHECKING           0           //super-anal blob validation control

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/

//resource type flags
#define BTF_Asteroid                0x00000001
#define BTF_Nebula                  0x00000002
#define BTF_GasCloud                0x00000004
#define BTF_DustCloud               0x00000008
#define BTF_Explored                0x00000010  //player has ships in blob
#define BTF_ProbeDroid              0x00000020  //player has an active probe in the blob
#define BTF_GettingAttacked         0x00000040  //you have ships in this blob that are being attacked
#define BTF_Attacking               0x00000080  //you have ships in this blob which are attacking other blobs
#define BTM_PieceOfTheAction        0x000000c0  //battles involving your ships
#define BTF_DontUpdate              0x00000100  //this blob is just fine as it is; don't update it
#define BTF_ClearObjectFlags        0x00000200  //clear object dontupdate flag when complete
#define BTF_Carrier                 0x00000400  //There's a player carrier in this blob
#define BTF_Mothership              0x00000800  //The mothership is in this blob
#define BTF_RecentDeath             0x00001000  // an object recently died and the list will need itemizing
#define BTF_UncloakedEnemies        0x00002000  //there are uncloaked enemies in the blob
#define BTF_FreeThisBlob            0x00004000  //This blob needs 'a freein
#define BTF_FreeBlobObjects         0x00008000  //Free the objects in this blob
#define BTM_ItemizeFlags            0x00003cff  //these bits are cleared and re-assigned during itemization

#define BOB_UpdateRadiusThreshold   1.00f       //can grow by 2%
#define BOB_UpdateCentreThreshold   0.10f       //can move by 10% of radius

/*=============================================================================
    Type definitions:
=============================================================================*/

// size for which a target will be put into blobBigTargets instead of blobSmallTargets
#define BIG_TARGET_SIZE     500.0f
#define BIG_SHIP_SIZE       500.0f

//structure for a single blob
typedef struct blob
{
    Node node;                                  //blobs are arranged in a linked list
    LinkedList subBlobs;                        //list of all sub-blobs
    real32 subBlobTime;                         //time the sub-blob list was last updated for this blob
    vector centre;                              //location of the sphere
    real32 radius;                              //radius of the bloc
    real32 volume;                              //updated with the radius
    real32 oneOverVolume;
    real32 totalMass;                           //total mass of this sphere
    real32 screenX, screenY;                    //on-screen location
    real32 screenRadius;                        //on-screen size
    real32 sortDistance;                        //square of distance from mothership
    real32 sqrtSortDistance;
    real32 cameraSortDistance;                  //sort distance from camera (non-deterministic)
    sdword RUs;                                 //number of resource units
    sdword nRockRUs, nDustRUs;                  //number of different types of RUs
    sdword nDustClouds, nRocks;                 //number of different types of resource objects
    udword flags;                               //types of resource units and whatnot
    sdword nCapitalShips, nAttackShips, nNonCombat;//ship-type breakdown
    sdword nHiddenShips;                        //number of ships hidden by surrounding resources (omputed by the SM)
    sdword nFriendlyShips;                      //number of player's ships in the blob
    real32 shipMass;                            //mass of just ships in this blob
    color lastColor;                            //last color rendered with

    SelectCommand *blobShips;                   // only used for collision blobs
    SelectCommand *blobSmallShips;
    SelectCommand *blobBigShips;
    SelectAnyCommand *blobSmallTargets;
    SelectAnyCommand *blobBigTargets;
    ResourceSelection *blobResources;
    DerelictSelection *blobDerelicts;
    BulletSelection *blobBullets;
    MissileSelection *blobMissileMissiles;
    MissileSelection *blobMissileMines;

    real32 blobMaxShipCollSphereSize;           // only used for collision blobs
    real32 blobMaxSmallShipCollSphereSize;
    real32 blobMaxBigShipCollSphereSize;
    real32 blobMaxBTargetCollSphereSize;
    real32 blobMaxSTargetCollSphereSize;
    real32 blobMaxResourceCollSphereSize;
    real32 blobMaxDerelictCollSphereSize;

    sdword maxNumShips;                         // only used for collision blobs
    sdword maxNumSmallShips;
    sdword maxNumBigShips;
    sdword maxNumSmallTargets;
    sdword maxNumBigTargets;
    sdword maxNumResources;
    sdword maxNumDerelicts;
    sdword maxNumBullets;
    sdword maxNumMissileMissiles;
    sdword maxNumMissileMines;
    sdword maxNumObjects;

    SpaceObjSelection *blobObjects;             // used for all blobs

    udword pad0;                                //round up to 256 bytes
}
blob;

typedef blob *blobPtr;

typedef struct BlobProperties
{
    real32 bobDensityLow;
    real32 bobDensityHigh;
    real32 bobStartSphereSize;
    real32 bobSmallestRadius;
    real32 bobBiggestRadius;
    real32 bobBiggestRadiusDefault;
    real32 bobOverlapFactor;
    real32 bobSqrtOverlapFactor;
    real32 bobRadiusCombineMargin;
} BlobProperties;

typedef bool (*subblobcallback)(blob *superblob, SpaceObj *obj);

/*=============================================================================
    Macros:
=============================================================================*/
#define bobBlobSize(n)          (sizeof(blob))
#if BOB_ANAL_CHECKING
#define blobAnalVerify(thisBlob) blobAnalVerifyFn(thisBlob)
#else
#define blobAnalVerify(thisBlob)
#endif

/*=============================================================================
    Functions:
=============================================================================*/

void bobListCreate(BlobProperties *blobProperties, LinkedList *list, udword playerIndex);
void bobSubBlobListCreate(BlobProperties *blobProperties, LinkedList *list, blob *superBlob, subblobcallback criteria);
void bobListUpdate(LinkedList *list);
void bobListDelete(LinkedList *list);
bool bobListSortCallback(void *firststruct,void *secondstruct);

blob *bobFindNearestBlobToObject(LinkedList *list,SpaceObj *obj,real32 *returndistsqr);
void bobAddObjToNearestBlob(LinkedList *list,SpaceObj *obj);

void bobAddObjToSpecificBlob(blob *putInBlob,SpaceObj *obj);

void bobObjectDied(SpaceObj *object,LinkedList *list);
void bobRemoveMineFromSpecificBlob(blob *thisBlob,Missile *mine);

void bobUpdateObjsInBlobCollInfo(blob *thisBlob);

real32 bobGetChecksum(LinkedList *list,sdword *numBlobsInChecksum);
void bobObjectListMedian(vector *dest, real32 *destRadius, sdword nObjects, SpaceObj **objects);

void bobInitProperties();
void bobResetProperties();

#ifdef BOB_STATS
typedef struct BobStats
{
    bool statsValid;
    bool subBlobs;
    sqword timeStart;
    sqword timeStop;
    sqword timeDuration;
    sdword numPasses;
    sdword numWalks;
    sdword numChecks;
    sdword trivialRejects;
    sdword initialBlobs;
    sdword finalBlobs;
    sdword combine;
} BobStats;

extern BobStats bobStats;
#endif

#if BOB_ANAL_CHECKING
void blobAnalVerifyFn(blob *thisBlob);
#endif

#endif //___BLOBS_H
