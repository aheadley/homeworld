/*=============================================================================
    Name    : nebulae.h
    Purpose : structures and functions for the management and display of nebulae

    Created 3/4/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _NEBULAE_H
#define _NEBULAE_H


// -----
// INCLUDES
// -----

#include "Types.h"
#include "Vector.h"
#include "Clouds.h"


// -----
// DEFINES
// -----

#define NUM_NEBTENDRIL_LODS 2

#define NEB_MAX_NEBULAE     8
#define NEB_MAX_TENDRILS    3
#define NEB_NUM_LODS        2

//pickin' mode
#define NEB_INITIAL_CHUNK   0
#define NEB_MEDIAN_CHUNK    1

//flags
#define NEB_CHUNK_INACTIVE  1
#define NEB_CHUNK_PICKED    2
#define NEB_CHUNK_DEAD      4

#define NEB_CHUNK_FORE      1
#define NEB_CHUNK_AFT       2

#define NEB_TENDRIL_INACTIVE 1
#define NEB_TENDRIL_LEADING  2
#define NEB_TENDRIL_TRAILING 4
#define NEB_TENDRIL_GEOMETRY 8


// -----
// STRUCTURES
// -----

/*
 * a chunk forms the core of a nebulae's tendrils.  chunks bob and weave.
 * tendrils dock with chunks
 */
typedef struct nebChunk_s
{
    udword flags;
    udword counter;
    udword numTendrils;

    real32 innerRadius;
    real32 outerRadius;
    vector position;

    vector velocity;
    vector dPos;

    void*  tendrils[NEB_MAX_TENDRILS];
    udword tFlags[NEB_MAX_TENDRILS];

    void* spaceobj;
    void* nebulae;
} nebChunk;

typedef struct
{
    udword  numVerts;
    udword  numFaces;
    real32* vertices;       // actual number of real32's is numVerts*3
    real32* normals;        // actual number of real32's is numVerts*3
    udword* faces;
    real32  hmat[16];
    udword  slices, stacks;
} nebTendrilLOD;

/*
 * rendering a tendril results in both its endpoints being rendered as well, if
 * they haven't already been, and the tendril itself
 */
typedef struct tendril_s
{
    udword flags;
    udword counter;

    nebChunk* a;
    nebChunk* b;

    vector direction;

    //bounding sphere
    real32 radius;
    vector midpoint;

    struct tendril_s* prev;
    struct tendril_s* next;

    nebTendrilLOD lod[NUM_NEBTENDRIL_LODS];
    color colour, realColour;
    real32 fadeFactor;
    ubyte maxAlpha;
} nebTendril;

typedef struct nebulae_s
{
    udword numChunks;
    udword numTendrils;
    nebChunk* chunkTable;
    nebTendril* tendrilTable;
} nebulae_t;

extern nebulae_t nebNebulae[NEB_MAX_NEBULAE];
extern udword numNebulae;


// -----
// VARIABLES
// -----

extern real32 nebFogSum;

extern real32 NEB_RADIUS;

// -----
// PROTOS
// -----

struct Nebula;

void nebStartup(void);
void nebShutdown(void);
void nebReset(void);
void nebAllocateChunks(nebulae_t* neb, udword num);
void nebAllocateTendrils(nebulae_t* neb, udword num);
void nebFreeChunks(nebulae_t* neb);
void nebFreeTendrils(nebulae_t* neb);
void nebDeleteChunk(nebChunk* chunk);
void nebInitChunk(nebChunk* chunk);
void nebInitTendril(nebTendril* tendril);
void nebAddTendril(nebChunk* chunk, nebTendril* tendril, udword flags);
void nebCopyEllipse(ellipseObject* dest, ellipseObject* src, vector* pos, real32 radius);
nebTendril* nebGetFreshTendril(nebulae_t* neb);
nebChunk* nebPickChunk(nebulae_t* neb, sdword type, nebChunk* constrainta, nebChunk* constraintb);
void nebDistributeChunks(nebulae_t* neb, vector* origin, vector* dimension);
void nebAttachTendrils(nebulae_t* neb);
void nebDeactivateEmptyChunksAndFixup(nebulae_t* neb);
void nebGo(vector* origin, vector* dimension, udword numOfChunks, udword numOfTendrils);
void nebRenderChunk(nebChunk* chunk, sdword lod);
void nebRenderTendril(nebTendril* tendril, sdword lod);
void nebRender(void);
real32 nebAngularDeviation(nebChunk* newChunk, nebChunk* oldChunka, nebChunk* oldChunkb);
real32 nebChunkDistance(nebChunk* a, nebChunk* b);
void nebChunkPicked(nebChunk* chunk, bool8 v);
void nebUnpickAllChunks(nebulae_t* neb);
void nebSetFog(void);
void nebNewNeb(struct Nebula* neb);
void nebFadeAttachedTendrils(nebChunk* chunk, real32 factor);

void nebSave_Nebula(void);
void nebLoad_Nebula(void);

nebTendril *nebNumToTendrilPtr(nebulae_t* neb, sdword num);
sdword nebTendrilPtrToNum(nebulae_t* neb, nebTendril *nebTendril);

nebChunk *nebNumToChunkPtr(nebulae_t* neb, sdword num);
sdword nebChunkPtrToNum(nebulae_t* neb, nebChunk *nebChunk);

nebulae_t* nebNumToNebulaePtr(sdword num);
sdword nebNebulaePtrToNum(nebulae_t* neb);

#endif

